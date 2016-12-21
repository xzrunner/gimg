#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_pvr_h
#define gameimage_pvr_h

#include <stdint.h>

// only support rgba 4bpp now
uint8_t* gimg_pvr_decode(const uint8_t* buf, int width, int height);
uint8_t* gimg_pvr_encode(const uint8_t* buf, int width, int height);

uint8_t* gimg_pvr_read_file(const char* filepath, uint32_t* width, uint32_t* height);
void gimg_pvr_write_file(const char* filepath, const uint8_t* buf, uint32_t width, uint32_t height);

unsigned gimg_pvr_get_morton_number(int x, int y);

uint8_t* gimg_pvr_init_blank(int edge);

#endif // gameimage_pvr_h

#ifdef __cplusplus
}
#endif