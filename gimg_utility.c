#include "gimg_utility.h"
#include "gimg_typedef.h"

#include <string.h>
#include <ctype.h>		// tolower
#include <assert.h>

enum GIMG_FILE
gimg_file_type(const char* filepath) {
    if (filepath == NULL) {
        return GIMG_FILE_INVALID;
    }

	int n = strlen(filepath);
    if (n == 0) {
        return GIMG_FILE_INVALID;
    }

    int ptr = n - 1;
    while (ptr >= 0) {
        if (filepath[ptr] == '.') {
            break;
        }
        --ptr;
    }
    if (ptr < 0) {
        return GIMG_FILE_INVALID;
    }

    ++ptr;
    char ext[8];
    strcpy(ext, &filepath[ptr]);
    for (int i = 0; i < n - ptr; ++i) {
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
    } else if (strcmp(ext, "tif") == 0 || strcmp(ext, "tiff") == 0) {
        return FILE_TIFF;
	} else {
		return GIMG_FILE_INVALID;
	}
}