#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_dds_h
#define gameimage_dds_h

#include <stdint.h>

uint8_t* gimg_dds_read_file(const char* filepath, unsigned int* out_width, unsigned int* out_height, unsigned int* out_format);

#endif // gameimage_dds_h

#ifdef __cplusplus
}
#endif