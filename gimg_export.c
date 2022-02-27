#include "gimg_export.h"
#include "gimg_utility.h"
#include "gimg_typedef.h"

#include "gimg_png.h"
#include "gimg_jpg.h"
#include "gimg_bmp.h"

int
gimg_export(const char* filepath, const uint8_t* pixels, int w, int h, int format, int reverse) {
	int ret = -1;

	int type = gimg_file_type(filepath);
	switch (type) {
	case FILE_PNG:
		ret = gimg_png_write(filepath, pixels, w, h, format, reverse);
		break;
	case FILE_JPG:
		ret = gimg_jpg_write(filepath, pixels, w, h, format, 80);
		break;
	case FILE_BMP:
		ret = gimg_bmp_write(filepath, pixels, w, h, format);
		break;
	case FILE_TIFF:
		ret = gimg_tiff_write(filepath, pixels, w, h, format, reverse);
		break;
	}

	return ret;
}