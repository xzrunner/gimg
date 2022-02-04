#include "gimg_utility.h"
#include "gimg_typedef.h"

#include <string.h>
#include <ctype.h>		// tolower
#include <assert.h>

enum GIMG_FILE
gimg_file_type(const char* filepath) {
	int n = strlen(filepath);
    if (filepath[n - 4] != '.') {
        return GIMG_FILE_INVALID;
    }
    char ext[4];
    strcpy(ext, &filepath[n - 3]);
    for (int i = 0; i < 3; ++i) {
        ext[i] = tolower(ext[i]);
    }
    if (strcmp(ext, "png") == 0) {
        return FILE_PNG;
    } else if (strcmp(ext, "jpg") == 0) {
        return FILE_JPG;
    } else if (strcmp(ext, "bmp") == 0) {
        return FILE_BMP;
    } else if (strcmp(ext, "ppm") == 0) {
        return FILE_PPM;
    } else if (strcmp(ext, "pvr") == 0) {
        return FILE_PVR;
    } else if (strcmp(ext, "pkm") == 0) {
        return FILE_PKM;
    } else if (strcmp(ext, "dds") == 0) {
        return FILE_DDS;
    } else if (strcmp(ext, "tga") == 0) {
        return FILE_TGA;
    } else if (strcmp(ext, "hdr") == 0) {
        return FILE_HDR;
    } else if (strcmp(ext, "hgt") == 0) {
        return FILE_HGT;
	} else {
		return GIMG_FILE_INVALID;
	}
}