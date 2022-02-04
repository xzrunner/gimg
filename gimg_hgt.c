#include "gimg_hgt.h"

#include <fs_file.h>
#include <logger.h>

#include <stdio.h>
#include <stdlib.h>

#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )

uint16_t* 
gimg_hgt_read_file(const char* filepath, int* width, int* height) {
	struct fs_file* file = fs_open(filepath, "rb");
	if (file == NULL) {
		fprintf(stderr, "can't open %s\n", filepath);
		return NULL;
	}

    int w, h;

	size_t file_size = fs_size(file);
    if (file_size > 3000000) {
        w = 3601;
        h = 3601;
    } else {
        w = 1201;
        h = 1201;
    }

    *width = w;
    *height = h;

    size_t size = w * h * sizeof(uint16_t);
    uint16_t* pixels = (uint16_t*)malloc(size);
	if (pixels == NULL) {
		LOGW("OOM: gimg_hgt_read, filepath %s, w %d, h %d", filepath, *width, *height);
		return NULL;
	}

    size_t buf_sz = w * sizeof(uint16_t);
    uint16_t* buffer = (uint16_t*)malloc(buf_sz);
    for (int y = 0; y < h; ++y) {
        fs_read(file, buffer, buf_sz);
        for (int x = 0; x < w; ++x) {
            pixels[(h - 1 - y) * w + x] = SWAP_2(buffer[x]);
        }
    }
    free(buffer);

    fs_close(file);

    return pixels;
}