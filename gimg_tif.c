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

	// fixme: only support r16 now
	if (bpp == 2)
	{
		size_t size = w * h * bpp;
		uint16_t* pixels = (uint16_t*)malloc(size);
		if (pixels == NULL) {
			LOGW("OOM: gimg_tif_read_file, filepath %s, w %d, h %d", filepath, *width, *height);
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