#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_typedef_h
#define gameimage_typedef_h

enum GIMG_IMAGE_TYPE {
	GIT_INVALID = 0,
	GIT_PNG,
	GIT_PVR,
	GIT_ETC1,
	GIT_ETC2,
};

enum GIMG_PIXEL_FORMAT {
	GPF_INVALID = 0,
	GPF_ALPHA,
	GPF_RGB,
	GPF_RGBA,
	GPF_LUMINANCE,
	GPF_LUMINANCE_ALPHA
};

#endif // gameimage_typedef_h

#ifdef __cplusplus
}
#endif