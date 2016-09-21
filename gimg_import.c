#include "gimg_import.h"

#include "gimg_png.h"
#include "gimg_jpg.h"

#include <stdbool.h>
#include <string.h>
#include <ctype.h>		// tolower

static inline bool
_is_png(const char* filepath) {
	int n = strlen(filepath);
	return tolower(filepath[n-4]) == '.'
		&& tolower(filepath[n-3]) == 'p'
		&& tolower(filepath[n-2]) == 'n'
		&& tolower(filepath[n-1]) == 'g';
}

static inline bool
_is_jpg(const char* filepath) {
	int n = strlen(filepath);
	return tolower(filepath[n-4]) == '.'
		&& tolower(filepath[n-3]) == 'j'
		&& tolower(filepath[n-2]) == 'p'
		&& tolower(filepath[n-1]) == 'g';
}

uint8_t* 
gimg_import(const char* filepath, int* width, int* height, enum GIMG_PIXEL_FORMAT* format) {
	uint8_t* pixels = NULL;
	if (_is_png(filepath)) {
		pixels = gimg_png_read(filepath, width, height, format);
	} else if (_is_jpg(filepath)) {
		int channels;
		pixels = gimg_jpg_read(filepath, width, height, &channels);
		*format = GPF_RGB;
	}
	if (pixels && *format == GPF_RGBA) {
		gimg_remove_ghost_pixel(pixels, *width, *height);
		gimg_format_pixels_alpha(pixels, *width, *height, 0);
	}
	return pixels;
}

void 
gimg_format_pixels_alpha(uint8_t* pixels, int width, int height, int val) {
	int ptr = 0;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			uint8_t r = pixels[ptr],
				g = pixels[ptr+1],
				b = pixels[ptr+2],
				a = pixels[ptr+3];
			if (a == 0) {
				r = g = b = val;
			}
			pixels[ptr++] = r;
			pixels[ptr++] = g;
			pixels[ptr++] = b;
			pixels[ptr++] = a;
		}
	}
}

void 
gimg_pre_muilti_alpha(uint8_t* pixels, int width, int height) {
	int pos = 0;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			float alpha = pixels[pos + 3] / 255.0f;
			for (int i = 0; i < 3; ++i) {
				pixels[pos + i] = (uint8_t)(pixels[pos + i] * alpha);
			}
			pos += 4;
		}
	}
}

void 
gimg_remove_ghost_pixel(uint8_t* pixels, int width, int height) {
	int ptr = 0;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			uint8_t r = pixels[ptr],
				g = pixels[ptr+1],
				b = pixels[ptr+2],
				a = pixels[ptr+3];

// 			if (r <= 3 && g <= 3 && b <= 3 && a <= 3) {
// 				r = g = b = a = 0;
// 			}
			if (a <= 3) {
				a = 0;
			}

			pixels[ptr++] = r;
			pixels[ptr++] = g;
			pixels[ptr++] = b;
			pixels[ptr++] = a;
		}
	}
}