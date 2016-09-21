#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_import_h
#define gameimage_import_h

#include <stdint.h>

uint8_t* gimg_import(const char* filepath, int* width, int* height, enum GIMG_PIXEL_FORMAT* format);

void gimg_format_pixels_alpha(uint8_t* pixels, int width, int height, int val);
void gimg_pre_muilti_alpha(uint8_t* pixels, int width, int height);
void gimg_remove_ghost_pixel(uint8_t* pixels, int width, int height);

#endif // gameimage_import_h

#ifdef __cplusplus
}
#endif