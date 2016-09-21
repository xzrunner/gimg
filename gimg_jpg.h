#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_jpg_h
#define gameimage_jpg_h

#include <stdint.h>

uint8_t* gimg_jpg_read(const char* filepath, int* width, int* height, int* channels);
int gimg_jpg_write(const char* filepath, const uint8_t* pixels, int width, int height, int quality);

#endif // gameimage_jpg_h

#ifdef __cplusplus
}
#endif