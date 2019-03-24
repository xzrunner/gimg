#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_typedef_h
#define gameimage_typedef_h

enum GIMG_FILE {
	GIMG_FILE_INVALID = 0,
	FILE_PNG,
	FILE_JPG,
	FILE_BMP,
	FILE_PPM,
	FILE_PVR,
	FILE_PKM,
	FILE_DDS,
	FILE_TGA,
    FILE_HDR,
};

enum GIMG_IMAGE_TYPE {
	GIT_INVALID = 0,
	GIT_PNG,
	GIT_PVR,
	GIT_ETC1,
	GIT_ETC2,
	GIT_DDS,
};

enum GIMG_PIXEL_FORMAT {
	GPF_INVALID = 0,
	GPF_ALPHA,
	GPF_RGB,
	GPF_RGBA8,
	GPF_RGBA4,
	GPF_BGRA_EXT,
	GPF_BGR_EXT,
    GPF_RGB16F,
	GPF_LUMINANCE,
	GPF_LUMINANCE_ALPHA,
	GPF_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	GPF_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	GPF_COMPRESSED_RGBA_S3TC_DXT5_EXT,
};

#endif // gameimage_typedef_h

#ifdef __cplusplus
}
#endif