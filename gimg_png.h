#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_png_h
#define gameimage_png_h

#include <stdint.h>

uint8_t* gimg_png_read(const char* filepath, int* width, int* height, int* format);
int gimg_png_write(const char* filepath, const uint8_t* pixels, int width, int height, int format, int reverse);

void gimg_png_read_header(const char* filepath, int* width, int* height);

#endif // gameimage_png_h

#ifdef __cplusplus
}
#endif