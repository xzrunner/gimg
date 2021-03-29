#include "gimg_bmp.h"
#include "gimg_typedef.h"
#include "gimg_utility.h"

#include <fs_file.h>
#include <fault.h>
#include <logger.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BMP_HEADER_SIZE 54

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

#ifdef _WIN32
	#include <pshpack2.h>
#else
	#pragma pack(2)
#endif // _WIN32
typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;
#ifdef _WIN32
	#include <poppack.h>
#else
	#pragma pack()
#endif // _WIN32

typedef struct tagBITMAPINFOHEADER{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;	/* 0 - rgb, 1 - rle8, 2 - rle4, 3 - BITFEILDS */
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

static uint8_t*
read_pixels(struct fs_file* file, int w, int h, int c) {
	int line_sz;
	if (c == 3) {
		int padding = 0;
		while ((w * c + padding) % 4 != 0) {
			padding++;
		}
		line_sz = w * c + padding;
	} else {
		line_sz = w * c;
	}

	uint8_t* pixels = (uint8_t*)malloc(line_sz * h);
	if (pixels == NULL) {
		LOGW("OOM: read_pixels, w %d, h %d", w, h);
		return NULL;
	}

	int dst_ptr = 0;
	ARRAY(uint8_t, data, line_sz);
	for (int y = 0; y < h; ++y) {
		int src_ptr = 0;
		fs_read(file, data, line_sz);
		for (int x = 0; x < w; ++x) {
			const uint8_t* src = &data[src_ptr];
			uint8_t* dst = &pixels[dst_ptr];
			switch (c)
			{
			case 1:
				dst[0] = src[0];
				break;
			case 3:
				dst[0] = src[2];
				dst[1] = src[1];
				dst[2] = src[0];
				break;
			case 4:
				dst[0] = src[2];
				dst[1] = src[1];
				dst[2] = src[0];
				dst[3] = src[3];
				break;
			}
			src_ptr += c;
			dst_ptr += c;
		}
	}

	return pixels;
}

uint8_t*
gimg_bmp_read(const char* filepath, int* width, int* height, int* format) {
	struct fs_file* file = fs_open(filepath, "rb");
	if (file == NULL) {
        fprintf(stderr, "can't open %s\n", filepath);
        return NULL;
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
		*format = GPF_RGBA8;
	} else if (bmih.biBitCount == 24) {
		pixels = read_pixels(file, w, h, 3);
		*format = GPF_RGB;
	} else if (bmih.biBitCount == 16) {
		pixels = (uint8_t*)malloc(w * h * 2);
		fs_seek_from_cur(file, 12);	// skip rgbquad mask
		fs_read(file, (void*)pixels, w * h * 2);
		*format = GPF_RGB565;
	} else if (bmih.biBitCount == 8) {
		pixels = read_pixels(file, w, h, 4);
		*format = GPF_RGBA8;
	} else {
		fault("Invalid image file: %s\n", filepath);
	}

	fs_close(file);

	return pixels;
}

int
gimg_bmp_write(const char* filepath, const uint8_t* pixels, int width, int height, int format) {
	struct fs_file* file = fs_open(filepath, "wb");
	if (file == NULL) {
		fault("Can't open image file: %s\n", filepath);
	}

	uint8_t rgb_mask[12] = {// RGBQUAD MASK     
		0x00, 0xF8, 0x00, 0x00, //red mask     
 		0xE0, 0x07, 0x00, 0x00, //green mask    
		0x1F, 0x00, 0x00, 0x00  //blue mask
	};

	BITMAPFILEHEADER bmfh;
	memset(&bmfh, 0, sizeof(bmfh));
	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	if (format == GPF_RGB565) {
		bmfh.bfOffBits = 14 + 40 + sizeof(rgb_mask);
		bmfh.bfSize = bmfh.bfOffBits + width * height * 2;
		bmfh.bfSize = (bmfh.bfSize + 2) & ~2;
	} else {
		bmfh.bfOffBits = 14 + 40;
		bmfh.bfSize = bmfh.bfOffBits + width * height * 3;
		bmfh.bfSize = (bmfh.bfSize + 3) & ~3;
	}

	BITMAPINFOHEADER bmih;
	bmih.biSize = sizeof(bmih);
	bmih.biWidth = width;
	bmih.biHeight = height;
	bmih.biPlanes = 1;
	if (format == GPF_RGB565) {
		bmih.biBitCount = 16;
		bmih.biCompression = 3;
	} else {
		bmih.biBitCount = 24;
		bmih.biCompression = 0;
	}
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0x0ec4;
	bmih.biYPelsPerMeter = 0x0ec4;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	fs_write(file, &bmfh, sizeof(bmfh));
	fs_write(file, &bmih, sizeof(bmih));
	if (format == GPF_RGB565) 
	{
		fs_write(file, rgb_mask, sizeof(rgb_mask));
		fs_write(file, (void*)pixels, width * height * 2);
	}
	else 
	{
		ARRAY(uint8_t, buf, width * 3);
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
	}
	fs_close(file);

	return 0;
}