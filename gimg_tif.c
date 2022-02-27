#include "gimg_hgt.h"
#include "gimg_typedef.h"

#include <logger.h>
#include <tiffio.h>

#include <stdlib.h>

uint8_t* 
gimg_tif_read_file(const char* filepath, int* width, int* height, int* format) {
	TIFF* tif = TIFFOpen(filepath, "r");

	int w = 0, h = 0, bpp = 0;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp);
	bpp /= 8;

	*width = w;
	*height = h;

	size_t size = w * h * 4 * 2;
	uint32_t* pixels = (uint32_t*)malloc(size);
	if (pixels == NULL) {
		LOGW("OOM: gimg_tif_read_file, filepath %s, w %d, h %d", filepath, *width, *height);
		return NULL;
	}

	TIFFReadRGBAImage(tif, w, h, pixels, 0);

	TIFFClose(tif);

	*format = GPF_RGBA8;

	return (uint8_t*)pixels;
}
