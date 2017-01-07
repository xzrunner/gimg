#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_bmp_h
#define gameimage_bmp_h

#include <stdint.h>

uint8_t* gimg_bmp_read(const char* filepath, int* width, int* height, int* format);
int      gimg_bmp_write(const char* filepath, const uint8_t* pixels, int width, int height);

#endif // gameimage_bmp_h

#ifdef __cplusplus
}
#endif