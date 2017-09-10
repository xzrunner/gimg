#include "gimg_import.h"
#include "gimg_utility.h"
#include "gimg_typedef.h"

#include "gimg_png.h"
#include "gimg_jpg.h"
#include "gimg_bmp.h"
#include "gimg_ppm.h"
#include "gimg_pvr.h"
#include "gimg_etc2.h"

#include <logger.h>

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
			*format = GPF_RGB;
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
			uint8_t* uncompressed = gimg_pvr_decode(compressed, *width, *height);
			free(compressed);
			pixels = uncompressed;
			*format = GPF_RGBA;
		}
		break;
	case FILE_PKM:
		{
			int type;
			uint8_t* compressed = gimg_etc2_read_file(filepath, width, height, &type);
			uint8_t* uncompressed = gimg_etc2_decode(compressed, *width, *height, type);
			free(compressed);
			pixels = uncompressed;
			*format = GPF_RGBA;
		}
		break;
	default:
		return pixels;
	}
	if (pixels && *format == GPF_RGBA) 
	{
		gimg_remove_ghost_pixel(pixels, *width, *height);
		gimg_format_pixels_alpha(pixels, *width, *height, 0);
	}
	return pixels;
}

int 
gimg_read_header(const char* filepath, int* width, int* height) {
	int ret = -1;
	int type = gimg_file_type(filepath);
	switch (type) {
	case FILE_PNG:
		gimg_png_read_header(filepath, width, height);
		break;
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
	case GPF_RGBA:
		channel = 4;
		break;
	}

	int line_sz = width * channel;
	ARRAY(uint8_t, buf, line_sz);
	int bpos = 0, epos = line_sz * (height - 1);
 	for (int i = 0, n = floor(height / 2); i < n; ++i) {
 		memcpy(buf, &pixels[bpos], line_sz);
 		memcpy(&pixels[bpos], &pixels[epos], line_sz);
 		memcpy(&pixels[epos], buf, line_sz);
 		bpos += line_sz;
 		epos -= line_sz;
 	}
}

uint8_t* 
gimg_rgba2rgb(const uint8_t* pixels, int width, int height) {
	uint8_t* rgb = (uint8_t*)malloc(width*height*3);
	if (rgb == NULL) {
		LOGW("OOM: gimg_rgba2rgb, w %d, h %d", width, height);
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