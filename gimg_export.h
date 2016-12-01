#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_export_h
#define gameimage_export_h

#include "gimg_typedef.h"

#include <stdint.h>

void gimg_export(const char* filepath, const uint8_t* pixels, int w, int h, enum GIMG_PIXEL_FORMAT fmt);

#endif // gameimage_export_h

#ifdef __cplusplus
}
#endif