#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_ppm_h
#define gameimage_ppm_h

#include <stdint.h>

uint8_t* gimg_ppm_read(const char* filepath, int* width, int* height);
int      gimg_ppm_write(const char* filepath, const uint8_t* pixels, int width, int height);

#endif // gameimage_ppm_h

#ifdef __cplusplus
}
#endif