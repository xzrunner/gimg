#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_tif_h
#define gameimage_tif_h

#include <stdint.h>

uint8_t* gimg_tif_read_file(const char* filepath, int* width, int* height, int* format);

#endif // gameimage_hgt_h

#ifdef __cplusplus
}
#endif