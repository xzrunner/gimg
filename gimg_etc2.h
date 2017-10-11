#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#ifndef gameimage_etc2_h
#define gameimage_etc2_h

enum DTEX_ETC_TYPE
{
	ETC1_RGB_NO_MIPMAPS = 0,
	ETC2PACKAGE_RGB_NO_MIPMAPS,
	ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD,
	ETC2PACKAGE_RGBA_NO_MIPMAPS,
	ETC2PACKAGE_RGBA1_NO_MIPMAPS,
	ETC2PACKAGE_R_NO_MIPMAPS,
	ETC2PACKAGE_RG_NO_MIPMAPS,
	ETC2PACKAGE_R_SIGNED_NO_MIPMAPS,
	ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS
};

uint16_t* gimg_etc2_decode(const uint8_t* buf, int width, int height, int type);
uint8_t* gimg_etc2_encode(const uint8_t* buf, int width, int height);

uint8_t* gimg_etc2_read_file(const char* filepath, int* width, int* height, int* type);

uint8_t* gimg_etc2_init_blank(int edge);

#endif // gameimage_etc2_h

#ifdef __cplusplus
}
#endif