#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#ifndef gameimage_etc1_h
#define gameimage_etc1_h

uint8_t* gimg_etc1_decode(const uint8_t* buf, int width, int height);
uint8_t* gimg_etc1_encode(const uint8_t* buf, int width, int height);

uint8_t* gimg_etc1_read_file(const char* filepath, uint32_t* width, uint32_t* height);

#endif // gameimage_etc1_h

#ifdef __cplusplus
}
#endif