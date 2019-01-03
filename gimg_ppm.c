#include "gimg_ppm.h"

#include <fs_file.h>
#include <fault.h>
#include <logger.h>

#include <ctype.h>
#include <stdlib.h>

static char
skip_comments(struct fs_file* file, char last) {
	while (isdigit(last) == 0){
		if (last == '#') {
			do {
				fs_read(file, &last, sizeof(last));
			} while(last != '\n');
		} else {
			fs_read(file, &last, sizeof(last));
		}
	}
	return last;
}

static char
read_int(struct fs_file* file, char last, int* ret) {
	int value = 0;
	if (last >= '0' && last <= '9') {
		value = value * 10 + (last - '0');
	}

	while (1) {
		fs_read(file, &last, sizeof(last));
		if (last >= '0' && last <= '9') {
			value = value * 10 + (last - '0');
		} else {
			break;
		}
	}

	*ret = value;

	return last;
}

uint8_t*
gimg_ppm_read(const char* filepath, int* width, int* height) {
	struct fs_file* file = fs_open(filepath, "rb");
	if (file == NULL) {
        fprintf(stderr, "can't open %s\n", filepath);
        return NULL;
	}

	char magic[2];
	fs_read(file, magic, sizeof(magic));
	if (magic[0] != 'P' || magic[1] != '6') {
		fs_close(file);
		return NULL;
	}

	int w, h, maxval;
	char last;
	fs_read(file, &last, sizeof(last));
	last = skip_comments(file, last);
	last = read_int(file, last, &w);
	last = skip_comments(file, last);
	last = read_int(file, last, &h);
	last = skip_comments(file, last);
	last = read_int(file, last, &maxval);
	if (maxval != 255) {
		fs_close(file);
		return NULL;
	}

	*width = w;
	*height = h;

	int size = w * h * 3;
	uint8_t* pixels = (uint8_t*)malloc(size);
	if (pixels == NULL) {
		LOGW("OOM: gimg_ppm_read, filepath %s, w %d, h %d", filepath, *width, *height);
		return NULL;
	}
	fs_read(file, pixels, size);

	fs_close(file);

	return pixels;
}

int
gimg_ppm_write(const char* filepath, const uint8_t* pixels, int width, int height) {
	struct fs_file* file = fs_open(filepath, "wb");
	if (file == NULL) {
		fault("Can't open image file: %s\n", filepath);
	}

	// todo

	return 0;
}