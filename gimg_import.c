#include "gimg_import.h"
#include "gimg_utility.h"
#include "gimg_typedef.h"
#include "gimg_math.h"

#include "gimg_png.h"
#include "gimg_jpg.h"
#include "gimg_bmp.h"
#include "gimg_ppm.h"
#include "gimg_pvr.h"
#include "gimg_etc2.h"
#include "gimg_dds.h"
#include "gimg_tga.h"

#include <logger.h>

#define STB_IMAGE_STATIC
#include <stb_image.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

uint8_t*
gimg_import(const char* filepath, int* width, int* height, int* format) {
	uint8_t* pixels = NULL;
	int type = gimg_file_type(filepath);
	switch (type) {
	case FILE_PNG:
		{
			pixels = gimg_png_read(filepath, width, height, format);
		}
		break;
	case FILE_JPG:
		{
			int channels;
			pixels = gimg_jpg_read(filepath, width, height, &channels);
			switch (channels)
			{
			case 1:
				*format = GPF_RED;
				break;
			case 3:
				*format = GPF_RGB;
				break;
			default:
				assert(0);
			}
		}
		break;
	case FILE_BMP:
		{
			pixels = gimg_bmp_read(filepath, width, height, format);
		}
		break;
	case FILE_PPM:
		{
			pixels = gimg_ppm_read(filepath, width, height);
			*format = GPF_RGB;
		}
		break;
	case FILE_PVR:
		{
			uint8_t* compressed = gimg_pvr_read_file(filepath, width, height);
			uint8_t* uncompressed = gimg_pvr_decode_rgba8(compressed, *width, *height);
			free(compressed);
			pixels = uncompressed;
			*format = GPF_RGBA8;
		}
		break;
	case FILE_PKM:
		{
			int type;
			uint8_t* compressed = gimg_etc2_read_file(filepath, width, height, &type);
			uint8_t* uncompressed = gimg_etc2_decode_rgba8(compressed, *width, *height, type);
			free(compressed);
			pixels = uncompressed;
			*format = GPF_RGBA8;
		}
		break;
	case FILE_DDS:
		pixels = gimg_dds_read_file(filepath, width, height, format);
		break;
	case FILE_TGA:
		pixels = gimg_tga_read_file(filepath, width, height, format);
		break;
    case FILE_HDR:
    {
        stbi_set_flip_vertically_on_load(true);
        int channels;
        pixels = (uint8_t*)stbi_loadf(filepath, width, height, &channels, 0);
		assert(channels == 3);
        *format = GPF_RGB16F;
    }
        break;
	default:
		return pixels;
	}
	if (pixels && *format == GPF_RGBA8)
	{
		gimg_remove_ghost_pixel(pixels, *width, *height);
		gimg_format_pixels_alpha(pixels, *width, *height, 0);
	}
	return pixels;
}

bool
gimg_read_header(const char* filepath, int* width, int* height) {
	bool ret = true;
	int type = gimg_file_type(filepath);
	switch (type) {
	case FILE_PNG:
		gimg_png_read_header(filepath, width, height);
		break;
	default:
		ret = false;
	}
	return ret;
}

void
gimg_format_pixels_alpha(uint8_t* pixels, int width, int height, int val) {
	int ptr = 0;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			uint8_t r = pixels[ptr],
				g = pixels[ptr+1],
				b = pixels[ptr+2],
				a = pixels[ptr+3];
			if (a == 0) {
				r = g = b = val;
			}
			pixels[ptr++] = r;
			pixels[ptr++] = g;
			pixels[ptr++] = b;
			pixels[ptr++] = a;
		}
	}
}

void
gimg_pre_mul_alpha(uint8_t* pixels, int width, int height) {
	int pos = 0;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			float alpha = pixels[pos + 3] / 255.0f;
			for (int i = 0; i < 3; ++i) {
				pixels[pos + i] = (uint8_t)(pixels[pos + i] * alpha);
			}
			pos += 4;
		}
	}
}

void
gimg_remove_ghost_pixel(uint8_t* pixels, int width, int height) {
	int ptr = 0;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			uint8_t r = pixels[ptr],
				g = pixels[ptr+1],
				b = pixels[ptr+2],
				a = pixels[ptr+3];

// 			if (r <= 3 && g <= 3 && b <= 3 && a <= 3) {
// 				r = g = b = a = 0;
// 			}
			if (a <= 3) {
				a = 0;
			}

			pixels[ptr++] = r;
			pixels[ptr++] = g;
			pixels[ptr++] = b;
			pixels[ptr++] = a;
		}
	}
}

void
gimg_revert_y(uint8_t* pixels, int width, int height, int format) {
	int channel = 0;
	switch (format)
	{
	case GPF_ALPHA: case GPF_LUMINANCE: case GPF_LUMINANCE_ALPHA:
		channel = 1;
		break;
	case GPF_RGB:
		channel = 3;
		break;
	case GPF_RGBA8:
		channel = 4;
		break;
	case GPF_RGBA4:
		channel = 2;
		break;
    case GPF_RGBA16F:
        channel = 8;
        break;
    case GPF_RGB16F:
        channel = 6;
        break;
    case GPF_RGB32F:
        channel = 12;
        break;
	}

	int line_sz = width * channel;
	ARRAY(uint8_t, buf, line_sz);
	int bpos = 0, epos = line_sz * (height - 1);
 	for (int i = 0, n = (int)(floorf(height / 2.0f)); i < n; ++i) {
 		memcpy(buf, &pixels[bpos], line_sz);
 		memcpy(&pixels[bpos], &pixels[epos], line_sz);
 		memcpy(&pixels[epos], buf, line_sz);
 		bpos += line_sz;
 		epos -= line_sz;
 	}
}

uint8_t*
gimg_rgba8_to_rgb8(const uint8_t* pixels, int width, int height) {
	uint8_t* rgb = (uint8_t*)malloc(width*height*3);
	if (rgb == NULL) {
		LOGW("OOM: gimg_rgba8_to_rgb8, w %d, h %d", width, height);
		return NULL;
	}
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			int src = (i*width+j)*4;
			int dst = (i*width+j)*3;
			memcpy(&rgb[dst], &pixels[src], 3);
		}
	}
	return rgb;
}

uint8_t*
gimg_rgb32f_to_rgb8(const uint8_t* pixels, int width, int height) {
	uint8_t* rgb = (uint8_t*)malloc(width*height*3);
	if (rgb == NULL) {
		LOGW("OOM: gimg_rgb16_to_rgb8, w %d, h %d", width, height);
		return NULL;
	}

    const float* pixels32 = (const float*)pixels;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			int src = (i*width+j)*3;
			int dst = (i*width+j)*3;
            for (int k = 0; k < 3; ++k) {
                float f = min(1.0f, pixels32[src + k]);
                rgb[dst + k] = (uint8_t)(255 * f);
            }
		}
	}
	return rgb;
}

uint8_t*
gimg_rgba8_to_rgba4(const uint8_t* pixels, int width, int height) {
	size_t sz = width * height * 2;
	uint16_t* dst = (uint16_t*)malloc(sz);
	if (!dst) {
		return NULL;
	}
	memset(dst, 0x00, sz);

	uint16_t*      dst_ptr = dst;
	const uint8_t* src_ptr = pixels;
	for (int i = 0, n = width * height; i < n; ++i)
	{
		*dst_ptr =
			((src_ptr[0] >> 4) << 12) |
			((src_ptr[1] >> 4) <<  8) |
			((src_ptr[2] >> 4) <<  4) |
			((src_ptr[3] >> 4) <<  0);
		++dst_ptr;
		src_ptr += 4;
	}
	return (uint8_t*)dst;
}

// Floyd-Steinberg
uint8_t*
gimg_rgba8_to_rgba4_dither(uint8_t* pixels, int width, int height) {
	size_t sz = width * height * 2;
	uint16_t* dst = (uint16_t*)malloc(sz);
	if (!dst) {
		return NULL;
	}
	memset(dst, 0x00, sz);

	int old_r, old_g, old_b, old_a;
	int new_r, new_g, new_b, new_a;
	int err_r, err_g, err_b, err_a;
	uint16_t* dst_ptr = dst;
	uint8_t*  src_ptr = pixels;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			old_r = src_ptr[0];
			old_g = src_ptr[1];
			old_b = src_ptr[2];
			old_a = src_ptr[3];
			new_r = (old_r >> 4) << 4;
			new_g = (old_g >> 4) << 4;
			new_b = (old_b >> 4) << 4;
			new_a = (old_a >> 4) << 4;
			err_r = old_r - new_r;
			err_g = old_g - new_g;
			err_b = old_b - new_b;
			err_a = old_a - new_a;

			// x + 1, y
			if (x != width - 1) {
				uint8_t* ptr = src_ptr + 4;
				float f = 7.0f / 16.0f;
				ptr[0] = MIN(255, MAX(0, ptr[0] + (int)(f * err_r)));
				ptr[1] = MIN(255, MAX(0, ptr[1] + (int)(f * err_g)));
				ptr[2] = MIN(255, MAX(0, ptr[2] + (int)(f * err_b)));
				ptr[3] = MIN(255, MAX(0, ptr[3] + (int)(f * err_a)));
			}

			// x - 1, y + 1
			if (x != 0 && y != height - 1) {
				uint8_t* ptr = src_ptr + 4 * (width - 1);
				float f = 3.0f / 16.0f;
				ptr[0] = MIN(255, MAX(0, ptr[0] + (int)(f * err_r)));
				ptr[1] = MIN(255, MAX(0, ptr[1] + (int)(f * err_g)));
				ptr[2] = MIN(255, MAX(0, ptr[2] + (int)(f * err_b)));
				ptr[3] = MIN(255, MAX(0, ptr[3] + (int)(f * err_a)));
			}

			// x, y + 1
			if (y != height - 1) {
				uint8_t* ptr = src_ptr + 4 * width;
				float f = 5.0f / 16.0f;
				ptr[0] = MIN(255, MAX(0, ptr[0] + (int)(f * err_r)));
				ptr[1] = MIN(255, MAX(0, ptr[1] + (int)(f * err_g)));
				ptr[2] = MIN(255, MAX(0, ptr[2] + (int)(f * err_b)));
				ptr[3] = MIN(255, MAX(0, ptr[3] + (int)(f * err_a)));
			}

			// x + 1, y + 1
			if (x != width - 1 && y != height - 1) {
				uint8_t* ptr = src_ptr + 4 * (width + 1);
				float f = 1.0f / 16.0f;
				ptr[0] = MIN(255, MAX(0, ptr[0] + (int)(f * err_r)));
				ptr[1] = MIN(255, MAX(0, ptr[1] + (int)(f * err_g)));
				ptr[2] = MIN(255, MAX(0, ptr[2] + (int)(f * err_b)));
				ptr[3] = MIN(255, MAX(0, ptr[3] + (int)(f * err_a)));
			}

			*dst_ptr =
				(((new_r & 0xff) >> 4) << 12) |
				(((new_g & 0xff) >> 4) << 8) |
				(((new_b & 0xff) >> 4) << 4) |
				(((new_a & 0xff) >> 4) << 0);

			++dst_ptr;
			src_ptr += 4;
		}
	}

	return (uint8_t*)dst;
}