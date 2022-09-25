#include "gimg_hgt.h"
#include "gimg_typedef.h"

#include <logger.h>
#include <tiffio.h>

#include <stdlib.h>
#include <string.h>

uint8_t* 
gimg_tiff_read_file(const char* filepath, int* width, int* height, int* format) {
	TIFF* tif = TIFFOpen(filepath, "r");
	if (tif == NULL) {
		fprintf(stderr, "can't open %s\n", filepath);
		return NULL;
	}

	int w = 0, h = 0, c = 0, bpp = 0;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &c);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp);
	bpp /= 8;

	*width = w;
	*height = h;

	size_t size = w * h * c * bpp;
	uint8_t* pixels = (uint8_t*)malloc(size);
	if (pixels == NULL) {
		LOGW("OOM: gimg_tiff_read_file, filepath %s, w %d, h %d", filepath, *width, *height);
		return NULL;
	}

	size_t line_size = TIFFScanlineSize(tif);
	tdata_t buf = _TIFFmalloc(line_size);
	uint8_t* ptr = pixels;
	for (int y = 0; y < h; y++) {
		TIFFReadScanline(tif, buf, y, 0);
		memcpy(ptr, buf, line_size);
		ptr += line_size;
	}

	_TIFFfree(buf);
	TIFFClose(tif);

	*format = GPF_INVALID;
	if (c == 1) {
		if (bpp == 1) {
			*format = GPF_RED;
		} else if (bpp == 2) {
			*format = GPF_R16;
		}
	} else if (c == 4) {
		if (bpp == 1) {
			*format = GPF_RGBA8;
		} else if (bpp == 2) {
			*format = GPF_RGBA16F;
		}
	}

	return (uint8_t*)pixels;
}

int 
gimg_tiff_write(const char* filepath, const uint8_t* pixels, int width, int height, int format, int reverse) {
	// fixme: only support r16 now
	if (format != GPF_R16) {
		return 0;
	}

	TIFF* tif = TIFFOpen(filepath, "w");
	if (tif == NULL) {
		return 0;
	}

	int channels = 0;
	int bits_per_pixel = 0;
	switch (format)
	{
	case GPF_RED:
		channels = 1;
		bits_per_pixel = 8;
		break;
	case GPF_R16:
		channels = 1;
		bits_per_pixel = 16;
		break;
	case GPF_RGBA8:
		channels = 4;
		bits_per_pixel = 8;
		break;
	case GPF_RGBA16F:
		channels = 4;
		bits_per_pixel = 16;
		break;
	default:
		LOGW("Unknown image format %d", format);
		return 0;
	}

	float refblackwhite[2 * 1];
	refblackwhite[0] = 0.0;
	refblackwhite[1] = (float)((1L << bits_per_pixel) - 1);

	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bits_per_pixel);
	TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_REFERENCEBLACKWHITE, refblackwhite);
//	TIFFSetField(tif, TIFFTAG_TRANSFERFUNCTION, gray);
	TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);

	int scan_line_sz = width * channels * (bits_per_pixel / 8);
	const uint8_t* ptr = pixels;
	if (reverse) {
		for (int i = 0; i < height; i++) {
			TIFFWriteScanline(tif, ptr, height - 1 - i, 0);
			ptr += scan_line_sz;
		}
	} else {
		for (int i = 0; i < height; i++) {
			TIFFWriteScanline(tif, ptr, i, 0);
			ptr += scan_line_sz;
		}
	}

    TIFFClose(tif);

	return 0;
}
