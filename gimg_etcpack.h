#ifdef __cplusplus
extern "C"
{
#endif
#ifndef gimg_etcpack_h
#define gimg_etcpack_h

#include <stdint.h>

void etcpack_init();

void etcpack_unpack_etc2c(unsigned int block_part1, unsigned int block_part2, uint8_t *img, int width, int height, int startx, int starty, int channels);
void etcpack_unpack_alpha_c(uint8_t* data, uint8_t* img, int width, int height, int ix, int iy, int channels);
void etcpack_unpack_etc21bitalphac(unsigned int block_part1, unsigned int block_part2, uint8_t *img, uint8_t* alphaimg, int width, int height, int startx, int starty, int channels);

#endif // gimg_etcpack_h

#ifdef __cplusplus
}
#endif