#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_import_h
#define gameimage_import_h

#include <stdint.h>
#include <stdbool.h>

uint8_t* gimg_import(const char* filepath, int* width, int* height, int* format);

bool gimg_read_header(const char* filepath, int* width, int* height);

void gimg_format_pixels_alpha(uint8_t* pixels, int width, int height, int val);
void gimg_pre_mul_alpha(uint8_t* pixels, int width, int height);
void gimg_remove_ghost_pixel(uint8_t* pixels, int width, int height);
void gimg_revert_y(uint8_t* pixels, int width, int height, int format);

uint8_t* gimg_rgba8_to_rgb8(const uint8_t* pixels, int width, int height);
uint8_t* gimg_rgb32f_to_rgb8(const uint8_t* pixels, int width, int height);

uint8_t* gimg_rgba8_to_rgba4(const uint8_t* pixels, int width, int height);
uint8_t* gimg_rgba8_to_rgba4_dither(uint8_t* pixels, int width, int height);

#endif // gameimage_import_h

#ifdef __cplusplus
}
#endif