#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gimg_rg_etc1_h
#define gimg_rg_etc1_h

#include <stdbool.h>

bool rg_etc1_unpack_block(const void *pETC1_block, 
						  unsigned int* pDst_pixels_rgba, 
						  bool preserve_alpha);

enum rg_etc1_quality {
	cLowQuality,
	cMediumQuality,
	cHighQuality,
};

void rg_etc1_pack_block_init();

unsigned int rg_etc1_pack_block(void* pETC1_block, 
								const unsigned int* pSrc_pixels_rgba, 
								enum rg_etc1_quality quality,
								bool dithering);

#endif // gimg_rg_etc1_h

#ifdef __cplusplus
}
#endif