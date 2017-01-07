#include "gimg_bmp.h"
#include "gimg_typedef.h"

#include <fs_file.h>
#include <fault.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BMP_HEADER_SIZE 54

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

#include <pshpack2.h>
typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;
#include <poppack.h>

typedef struct tagBITMAPINFOHEADER{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

static uint8_t*
read_pixels(struct fs_file* file, int w, int h, int c) {
	assert(c == 4 || c == 3);

	int line_sz;
	if (c == 3) {
		int padding = 0; 
		while ((w * 3 + padding) % 4 != 0) {
			padding++;
		}
		line_sz = w * 3 + padding;
	} else {
		line_sz = w * 4;
	}

	uint8_t* pixels = (uint8_t*)malloc(line_sz * h);
	if (!pixels) {
		fault("malloc fail: gimg_bmp_read\n");
	}

	if (c == 3) {
		int dst_ptr = 0;
		uint8_t data[line_sz];
		for (int y = 0; y < h; ++y) {
			int src_ptr= 0;
			fs_read(file, data, line_sz);
			for (int x = 0; x < w; ++x) {
				const uint8_t* src = &data[src_ptr];
				uint8_t* dst = &pixels[dst_ptr];
				dst[0] = src[2];
				dst[1] = src[1];
				dst[2] = src[0];
				src_ptr += c;
				dst_ptr += c;
			}
		}
	} else {
		int dst_ptr = 0;
		uint8_t data[line_sz];
		for (int y = 0; y < h; ++y) {
			int src_ptr= 0;
			fs_read(file, data, line_sz);
			for (int x = 0; x < w; ++x) {
				const uint8_t* src = &data[src_ptr];
				uint8_t* dst = &pixels[dst_ptr];
				dst[0] = src[2];
				dst[1] = src[1];
				dst[2] = src[0];
				dst[3] = src[3];
				src_ptr += c;
				dst_ptr += c;
			}
		}
	}

	return pixels;
}

uint8_t* 
gimg_bmp_read(const char* filepath, int* width, int* height, int* format) {
	struct fs_file* file = fs_open(filepath, "rb");
	if (file == NULL) {
		fault("Can't open image file: %s\n", filepath);
	}

	BITMAPFILEHEADER bmfh;
	if (fs_read(file, &bmfh, sizeof(bmfh)) != sizeof(bmfh)) {
		fault("Invalid image file: %s\n", filepath);
	}

	BITMAPINFOHEADER bmih;
	if (fs_read(file, &bmih, sizeof(bmih)) != sizeof(bmih)) {
		fault("Invalid image file: %s\n", filepath);
	}

	int w = bmih.biWidth,
		h = bmih.biHeight;
	*width = w;
	*height = h;

	uint8_t* pixels = NULL;
	if (bmih.biBitCount == 32) {
		pixels = read_pixels(file, w, h, 4);
		*format = GPF_RGBA;
	} else if (bmih.biBitCount == 24) {
		pixels = read_pixels(file, w, h, 3);
		*format = GPF_RGB;
	} else {
		fault("Invalid image file: %s\n", filepath);
	}

	fs_close(file);

	return pixels;
}

int 
gimg_bmp_write(const char* filepath, const uint8_t* pixels, int width, int height) {
	struct fs_file* file = fs_open(filepath, "wb");
	if (file == NULL) {
		fault("Can't open image file: %s\n", filepath);
	}

	BITMAPFILEHEADER bmfh;
	memset(&bmfh, 0, sizeof(bmfh));
	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = 14 + 40;
	bmfh.bfSize = bmfh.bfOffBits + width * height * 3;
	bmfh.bfSize = (bmfh.bfSize + 3) & ~3;

	BITMAPINFOHEADER bmih;
	bmih.biSize = sizeof(bmih);
	bmih.biWidth = width;
	bmih.biHeight = height;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = 0;      // BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0x0ec4;
	bmih.biYPelsPerMeter = 0x0ec4;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	fs_write(file, &bmfh, sizeof(bmfh));
	fs_write(file, &bmih, sizeof(bmih));

//	fs_write(file, (void*)pixels, width * height * 3);

	uint8_t buf[width * 3];
	int src_ptr = 0;
	for (int y = 0; y < height; ++y) {
		int dst_ptr = 0;
		for (int x = 0; x < width; ++x) {
			const uint8_t* src = &pixels[src_ptr];
			uint8_t* dst = &buf[dst_ptr];
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			src_ptr += 3;
			dst_ptr += 3;
		}
		fs_write(file, (void*)buf, width * 3);
	}

	fs_close(file);

	return 0;
}