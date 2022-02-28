#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_tif_h
#define gameimage_tif_h

#include <stdint.h>

uint8_t* gimg_tiff_read_file(const char* filepath, int* width, int* height, int* format);
int gimg_tiff_write(const char* filepath, const uint8_t* pixels, int width, int height, int format, int reverse);

#endif // gameimage_hgt_h

#ifdef __cplusplus
}
#endif