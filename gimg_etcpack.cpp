#include "gimg_etcpack.h"

#include <etcpack_lib.h>

extern "C"
void etcpack_init() {
	readCompressParams();
	setupAlphaTableAndValtab();
}

extern "C"
void etcpack_unpack_etc2c(unsigned int block_part1, unsigned int block_part2, uint8_t *img, int width, int height, int startx, int starty, int channels) {
	decompressBlockETC2c(block_part1, block_part2, img, width, height, startx, starty, channels);
}

extern "C"
void etcpack_unpack_alpha_c(uint8_t* data, uint8_t* img, int width, int height, int ix, int iy, int channels) {
	decompressBlockAlphaC(data, img, width, height, ix, iy, channels);
}

extern "C"
void etcpack_unpack_etc21bitalphac(unsigned int block_part1, unsigned int block_part2, uint8_t *img, uint8_t* alphaimg, int width, int height, int startx, int starty, int channels) {
	decompressBlockETC21BitAlphaC(block_part1, block_part2, img, alphaimg, width, height, startx, starty, channels);
}