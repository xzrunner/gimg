#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_hgt_h
#define gameimage_hgt_h

#include <stdint.h>

uint16_t* gimg_hgt_read_file(const char* filepath, int* width, int* height);

#endif // gameimage_hgt_h

#ifdef __cplusplus
}
#endif