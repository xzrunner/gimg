#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_tga_h
#define gameimage_tga_h

#include <stdint.h>

uint8_t* gimg_tga_read_file(const char* filepath, int* width, int* height, int* format);

#endif // gameimage_tga_h

#ifdef __cplusplus
}
#endif