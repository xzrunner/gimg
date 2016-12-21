#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gameimage_math_h
#define gameimage_math_h

#ifndef IS_POT
	#define IS_POT(x) ((x) > 0 && ((x) & ((x) -1)) == 0)
#endif // IS_POT

#ifndef MAX
	#define MAX(a, b) ( ((a)>(b))?(a):(b) )
#endif // MAX

#ifndef MIN
	#define MIN(a, b) ( ((a)<(b))?(a):(b) )
#endif // MIN

#endif // gameimage_math_h

#ifdef __cplusplus
}
#endif