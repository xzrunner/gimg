#include "gimg_png.h"
#include "gimg_typedef.h"
#include "gimg_utility.h"

#include <fs_file.h>
#include <fault.h>
#include <logger.h>

#include <libpng16/png.h>

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

struct stream {
	char* data;
	int offset;
};

static inline void
_read_memory_cb(png_structp png, png_bytep data, png_size_t size)
{
	struct stream* ss = (struct stream*)png_get_io_ptr(png);
	memcpy(data, ss->data + ss->offset, size);
	ss->offset += size;
}

uint8_t*
gimg_png_read(const char* filepath, int* width, int* height, int* format) {
	struct fs_file* file = fs_open(filepath, "rb");
	if (file == NULL) {
		fault("Can't open png file: %s\n", filepath);
	}

	size_t sz = fs_size(file);
	uint8_t* buf = (uint8_t*)malloc(sz);
	if (buf == NULL) {
		LOGW("OOM: gimg_png_read, w %d, h %d", *width, *height);
		return NULL;
	}
	if (fs_read(file, buf, sz) != sz) {
		fault("Invalid uncompress data source\n");
	}
	fs_close(file);

	struct stream ss;
	ss.data = (char*)buf;
	ss.offset = 0;

	png_byte lHeader[8];
	png_structp lPngPtr = NULL; png_infop lInfoPtr = NULL;
	png_byte* lImageBuffer = NULL; png_bytep* lRowPtrs = NULL;
	png_int_32 lRowSize; bool lTransparency;
	do
	{
// 		if (m_resource.read(lHeader, sizeof(lHeader)) == 0)
// 			break;
		memcpy(lHeader, (char*)ss.data + ss.offset, sizeof(lHeader));
		ss.offset += sizeof(lHeader);
		if (png_sig_cmp(lHeader, 0, 8) != 0) break;

		lPngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!lPngPtr) break;
		lInfoPtr = png_create_info_struct(lPngPtr);
		if (!lInfoPtr) break;
		png_set_read_fn(lPngPtr, &ss, _read_memory_cb);
		if (setjmp(png_jmpbuf(lPngPtr))) break;

		png_set_sig_bytes(lPngPtr, 8);
		png_read_info(lPngPtr, lInfoPtr);
		png_int_32 lDepth, lColorType;
		png_uint_32 lWidth, lHeight;
		png_get_IHDR(lPngPtr, lInfoPtr, &lWidth, &lHeight, &lDepth, &lColorType, NULL, NULL, NULL);
		*width = lWidth; *height = lHeight;

		// Creates a full alpha channel if transparency is encoded as
		// an array of palette entries or a single transparent color.
		lTransparency = false;
		if (png_get_valid(lPngPtr, lInfoPtr, PNG_INFO_tRNS))
		{
			png_set_tRNS_to_alpha(lPngPtr);
			lTransparency = true;
		}
		// Expands PNG with less than 8bits per channel to 8bits.
		if (lDepth < 8)
		{
			png_set_packing (lPngPtr);
		}
		// Shrinks PNG with 16bits per color channel down to 8bits.
		else if (lDepth == 16)
		{
			png_set_strip_16(lPngPtr);
		}
		// Indicates that image needs conversion to RGBA if needed.
		switch (lColorType)
		{
		case PNG_COLOR_TYPE_PALETTE:
			png_set_palette_to_rgb(lPngPtr);
			*format = lTransparency ? GPF_RGBA8 : GPF_RGB;
			break;
		case PNG_COLOR_TYPE_RGB:
			*format = lTransparency ? GPF_RGBA8 : GPF_RGB;
			break;
		case PNG_COLOR_TYPE_RGBA:
			*format = GPF_RGBA8;
			break;
		case PNG_COLOR_TYPE_GRAY:
			png_set_expand_gray_1_2_4_to_8(lPngPtr);
			*format = lTransparency ? GPF_LUMINANCE_ALPHA : GPF_LUMINANCE;
			break;
		case PNG_COLOR_TYPE_GA:
			png_set_expand_gray_1_2_4_to_8(lPngPtr);
			*format = GPF_LUMINANCE_ALPHA;
			break;
		}
		png_read_update_info(lPngPtr, lInfoPtr);

		lRowSize = png_get_rowbytes(lPngPtr, lInfoPtr);
		if (lRowSize <= 0) break;
		lImageBuffer = (png_byte*)malloc(lRowSize * lHeight * sizeof(png_byte));
		if (!lImageBuffer) break;
		lRowPtrs = (png_bytep*)malloc(lHeight * sizeof(png_bytep));
		if (!lRowPtrs) break;
		for (unsigned int i = 0; i < lHeight; ++i)
		{
			lRowPtrs[lHeight - (i + 1)] = lImageBuffer + i * lRowSize;
		}
		png_read_image(lPngPtr, lRowPtrs);

		png_destroy_read_struct(&lPngPtr, &lInfoPtr, NULL);
		free(lRowPtrs);

		free(buf);

		return lImageBuffer;
	} while (0);

	// error
	//ERROR:
	//		Log::error("Error while reading PNG file");
	free(lRowPtrs); free(lImageBuffer);
	free(buf);
	if (lPngPtr != NULL)
	{
		png_infop* lInfoPtrP = lInfoPtr != NULL ? &lInfoPtr : NULL;
		png_destroy_read_struct(&lPngPtr, lInfoPtrP, NULL);
	}
	return NULL;

	free(buf);
}

int
gimg_png_write(const char* filepath, const uint8_t* pixels, int width, int height, int format, int reverse) {
	if (format != GPF_RGB && format != GPF_RGBA8) {
		return -1;
	}

	unsigned bands_per_pixel;
	png_byte color_type;
	if (format == GPF_RGB) {
		bands_per_pixel = 3;
		color_type = PNG_COLOR_TYPE_RGB;
	} else {
		bands_per_pixel = 4;
		color_type = PNG_COLOR_TYPE_RGBA;
	}

	int bit_depth = 8;

	FILE *out = fopen(filepath, "wb");
	if (!out) {
		return 1;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	assert(png_ptr);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	assert(info_ptr);
	png_init_io(png_ptr, out);
	png_set_IHDR(png_ptr, info_ptr, width, height,
		bit_depth, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);

	ARRAY(uint8_t*, row_pointers, height);
	if (reverse) {
		for (int y = 0; y < height; y++) {
			row_pointers[height - 1 - y] = (uint8_t*)pixels + y * (width * bands_per_pixel * bit_depth / 8);
		}
	} else {
		for (int y = 0; y < height; y++) {
			row_pointers[y] = (uint8_t*)pixels + y * (width * bands_per_pixel * bit_depth / 8);
		}
	}

	png_write_image(png_ptr, &row_pointers[0]);
	png_write_end(png_ptr, NULL);
	fclose(out);

	return 0;
}

static void
_read_file_cb(png_structp png_ptr, png_bytep out, png_size_t count)
{
	png_voidp io_ptr = png_get_io_ptr(png_ptr);
	if (io_ptr == 0) {
		return;
	}

	struct fs_file* file = (struct fs_file*)io_ptr;
	fs_read(file, (char*)out, count);
}

void
gimg_png_read_header(const char* filepath, int* width, int* height) {
	struct fs_file* file = fs_open(filepath, "rb");
	if (file == NULL) {
		fault("Can't open png file: %s\n", filepath);
	}

	png_byte header[8];
	fs_read(file, header, sizeof(header));
	if (png_sig_cmp(header, 0, 8) != 0) {
		fs_close(file);
		return;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fs_close(file);
		return;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		fs_close(file);
		return;
	}
	png_set_read_fn(png_ptr, (void*)file, _read_file_cb);
	if (setjmp(png_jmpbuf(png_ptr))) {
		fs_close(file);
		return;
	}

	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	png_int_32 _depth, _color_type;
	png_uint_32 _width, _height;
	png_get_IHDR(png_ptr, info_ptr, &_width, &_height, &_depth, &_color_type, NULL, NULL, NULL);

	*width = _width; *height = _height;

	if (png_ptr != NULL)
	{
		png_infop* info_ptr_ptr = info_ptr != NULL ? &info_ptr : NULL;
		png_destroy_read_struct(&png_ptr, info_ptr_ptr, NULL);
	}

	fs_close(file);
}