#include "gimg_export.h"
#include "gimg_utility.h"
#include "gimg_bmp.h"

void 
gimg_export(const char* filepath, const uint8_t* pixels, int w, int h, enum GIMG_PIXEL_FORMAT fmt) {
	switch (gimg_file_type(filepath)) {
	case FILE_BMP:
		if (fmt == GPF_RGB) {
			gimg_bmp_write(filepath, pixels, w, h);
		}
		break;
	}
}