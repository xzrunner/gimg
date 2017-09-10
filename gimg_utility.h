#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_utility_h
#define gameimage_utility_h

#ifdef _MSC_VER
#	include <malloc.h>
#	define ARRAY(type, name, size) type* name = (type*)_alloca((size) * sizeof(type))
#else
#	define ARRAY(type, name, size) type name[size]
#endif

enum GIMG_FILE gimg_file_type(const char* filepath);

#endif // gameimage_utility_h

#ifdef __cplusplus
}
#endif