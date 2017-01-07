#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_export_h
#define gameimage_export_h

#include <stdint.h>

int gimg_export(const char* filepath, const uint8_t* pixels, int w, int h, int format, int reverse);

#endif // gameimage_export_h

#ifdef __cplusplus
}
#endif