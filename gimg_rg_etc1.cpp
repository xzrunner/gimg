#include "gimg_rg_etc1.h"

#include <rg_etc1.h>

bool 
rg_etc1_unpack_block(const void *pETC1_block, unsigned int* pDst_pixels_rgba, bool preserve_alpha) {
	return rg_etc1::unpack_etc1_block(pETC1_block, pDst_pixels_rgba, preserve_alpha);	
}

void 
rg_etc1_pack_block_init() {
	rg_etc1::pack_etc1_block_init();
}

unsigned int 
rg_etc1_pack_block(void* pETC1_block, const unsigned int* pSrc_pixels_rgba, enum rg_etc1_quality quality, bool dithering) {
	rg_etc1::etc1_pack_params params;
	params.m_quality = (enum rg_etc1::etc1_quality)quality;
	params.m_dithering = dithering;
	return rg_etc1::pack_etc1_block(pETC1_block, pSrc_pixels_rgba, params);	
}