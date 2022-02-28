#include "gimg_hgt.h"
#include "gimg_typedef.h"

#include <logger.h>
#include <tiffio.h>

#include <stdlib.h>

uint8_t* 
gimg_tiff_read_file(const char* filepath, int* width, int* height, int* format) {
	TIFF* tif = TIFFOpen(filepath, "r");
	if (tif == NULL) {
		fprintf(stderr, "can't open %s\n", filepath);
		return NULL;
	}

	int w = 0, h = 0, bpp = 0;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp);
	bpp /= 8;

	*width = w;
	*height = h;

	// fixme: only support r16 now
	if (bpp == 2)
	{
		size_t size = w * h * bpp;
		uint16_t* pixels = (uint16_t*)malloc(size);
		if (pixels == NULL) {
			LOGW("OOM: gimg_tiff_read_file, filepath %s, w %d, h %d", filepath, *width, *height);
			return NULL;
		}

		size_t line_size = TIFFScanlineSize(tif);
		tdata_t buf = _TIFFmalloc(line_size);
		uint16_t* ptr = pixels;
		for (int y = 0; y < h; y++) {
			TIFFReadScanline(tif, buf, y, 0);
			memcpy(ptr, buf, line_size);
			ptr += w;
		}

		_TIFFfree(buf);
		TIFFClose(tif);

		*format = GPF_R16;

		return (uint8_t*)pixels;
	}
	else
	{
		return NULL;
	}
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

	int bits_per_pixel = 16;
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

	int scan_line_sz = width * (bits_per_pixel / 8);
	uint8_t* ptr = pixels;
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
