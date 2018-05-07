// Code from nv_dds https://github.com/paroj/nv_dds

#include "gimg_dds.h"
#include "gimg_typedef.h"

#include <fs_file.h>
#include <fault.h>

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define DDS_REVERT_Y

enum TextureType {
	TextureNone = 0, TextureFlat,    // 1D, 2D textures
	Texture3D,
	TextureCubemap
};

// surface description flags
const uint32_t DDSF_CAPS = 0x00000001;
const uint32_t DDSF_HEIGHT = 0x00000002;
const uint32_t DDSF_WIDTH = 0x00000004;
const uint32_t DDSF_PITCH = 0x00000008;
const uint32_t DDSF_PIXELFORMAT = 0x00001000;
const uint32_t DDSF_MIPMAPCOUNT = 0x00020000;
const uint32_t DDSF_LINEARSIZE = 0x00080000;
const uint32_t DDSF_DEPTH = 0x00800000;

// pixel format flags
const uint32_t DDSF_ALPHAPIXELS = 0x00000001;
const uint32_t DDSF_FOURCC = 0x00000004;
const uint32_t DDSF_RGB = 0x00000040;
const uint32_t DDSF_RGBA = 0x00000041;

// dwCaps1 flags
const uint32_t DDSF_COMPLEX = 0x00000008;
const uint32_t DDSF_TEXTURE = 0x00001000;
const uint32_t DDSF_MIPMAP = 0x00400000;

// dwCaps2 flags
const uint32_t DDSF_CUBEMAP = 0x00000200;
const uint32_t DDSF_CUBEMAP_POSITIVEX = 0x00000400;
const uint32_t DDSF_CUBEMAP_NEGATIVEX = 0x00000800;
const uint32_t DDSF_CUBEMAP_POSITIVEY = 0x00001000;
const uint32_t DDSF_CUBEMAP_NEGATIVEY = 0x00002000;
const uint32_t DDSF_CUBEMAP_POSITIVEZ = 0x00004000;
const uint32_t DDSF_CUBEMAP_NEGATIVEZ = 0x00008000;
const uint32_t DDSF_CUBEMAP_ALL_FACES = 0x0000FC00;
const uint32_t DDSF_VOLUME = 0x00200000;

// compressed texture types
enum DXT_Type
{
	FOURCC_DXT1 = 0x31545844, //(MAKEFOURCC('D','X','T','1'))
	FOURCC_DXT3 = 0x33545844, //(MAKEFOURCC('D','X','T','3'))
	FOURCC_DXT5 = 0x35545844, //(MAKEFOURCC('D','X','T','5'))
};

struct DDS_PIXELFORMAT {
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwFourCC;
	uint32_t dwRGBBitCount;
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwABitMask;
};

struct DDS_HEADER {
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwHeight;
	uint32_t dwWidth;
	uint32_t dwPitchOrLinearSize;
	uint32_t dwDepth;
	uint32_t dwMipMapCount;
	uint32_t dwReserved1[11];
	struct DDS_PIXELFORMAT ddspf;
	uint32_t dwCaps1;
	uint32_t dwCaps2;
	uint32_t dwReserved2[3];
};

struct texture {
	unsigned int width;
	unsigned int height;
	unsigned int depth;
	unsigned int size;
	uint8_t* pixels;
};

static void
fourcc(uint32_t enc, char output[4]) {
	output[0] = enc >> 0 & 0xFF;
	output[1] = enc >> 8 & 0xFF;
	output[2] = enc >> 16 & 0xFF;
	output[3] = enc >> 24 & 0xFF;
}

// calculates size of DXTC texture in bytes
static unsigned int
size_dxtc(unsigned int width, unsigned int height, unsigned int format) {
	return ((width + 3) / 4) * ((height + 3) / 4) * (format == GPF_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16);
}

// calculates size of uncompressed RGB texture in bytes
static unsigned int
size_rgb(unsigned int width, unsigned int height, unsigned int components) {
	return width * height * components;
}

static bool
is_compressed(unsigned int format) {
	return (format == GPF_COMPRESSED_RGBA_S3TC_DXT1_EXT)
		|| (format == GPF_COMPRESSED_RGBA_S3TC_DXT3_EXT)
		|| (format == GPF_COMPRESSED_RGBA_S3TC_DXT5_EXT);
}

// clamps input size to [1-size]
static unsigned int
clamp_size(unsigned int size) {
	if (size <= 0)
		size = 1;

	return size;
}

struct DXTColBlock {
	uint16_t col0;
	uint16_t col1;

	uint8_t row[4];
};

struct DXT3AlphaBlock {
	uint16_t row[4];
};

struct DXT5AlphaBlock {
	uint8_t alpha0;
	uint8_t alpha1;

	uint8_t row[6];
};

#define GIMG_SWAP(a, b, type) do { type temp = a; a = b; b = temp; } while (0)

// flip a DXT1 color block
static void
flip_blocks_dxtc1(struct DXTColBlock* line, unsigned int numBlocks) {
	struct DXTColBlock* curblock = line;

	for (unsigned int i = 0; i < numBlocks; i++)
	{
		GIMG_SWAP(curblock->row[0], curblock->row[3], uint8_t);
		GIMG_SWAP(curblock->row[1], curblock->row[2], uint8_t);

		curblock++;
	}
}

// flip a DXT3 color block
static void
flip_blocks_dxtc3(struct DXTColBlock *line, unsigned int numBlocks) {
	struct DXTColBlock* curblock = line;
	struct DXT3AlphaBlock* alphablock;

	for (unsigned int i = 0; i < numBlocks; i++) {
		alphablock = (struct DXT3AlphaBlock*)curblock;

		GIMG_SWAP(alphablock->row[0], alphablock->row[3], uint16_t);
		GIMG_SWAP(alphablock->row[1], alphablock->row[2], uint16_t);

		curblock++;

		GIMG_SWAP(curblock->row[0], curblock->row[3], uint8_t);
		GIMG_SWAP(curblock->row[1], curblock->row[2], uint8_t);

		curblock++;
	}
}

///////////////////////////////////////////////////////////////////////////////
// flip a DXT5 alpha block
void flip_dxt5_alpha(struct DXT5AlphaBlock *block) {
	uint8_t gBits[4][4];

	const uint32_t mask = 0x00000007;          // bits = 00 00 01 11
	uint32_t bits = 0;
	memcpy(&bits, &block->row[0], sizeof(uint8_t) * 3);

	gBits[0][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[0][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[0][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[0][3] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][3] = (uint8_t)(bits & mask);

	bits = 0;
	memcpy(&bits, &block->row[3], sizeof(uint8_t) * 3);

	gBits[2][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[2][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[2][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[2][3] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][3] = (uint8_t)(bits & mask);

	uint32_t *pBits = ((uint32_t*) &(block->row[0]));

	*pBits = *pBits | (gBits[3][0] << 0);
	*pBits = *pBits | (gBits[3][1] << 3);
	*pBits = *pBits | (gBits[3][2] << 6);
	*pBits = *pBits | (gBits[3][3] << 9);

	*pBits = *pBits | (gBits[2][0] << 12);
	*pBits = *pBits | (gBits[2][1] << 15);
	*pBits = *pBits | (gBits[2][2] << 18);
	*pBits = *pBits | (gBits[2][3] << 21);

	pBits = ((uint32_t*) &(block->row[3]));

#ifdef MACOS
	*pBits &= 0x000000ff;
#else
	*pBits &= 0xff000000;
#endif

	*pBits = *pBits | (gBits[1][0] << 0);
	*pBits = *pBits | (gBits[1][1] << 3);
	*pBits = *pBits | (gBits[1][2] << 6);
	*pBits = *pBits | (gBits[1][3] << 9);

	*pBits = *pBits | (gBits[0][0] << 12);
	*pBits = *pBits | (gBits[0][1] << 15);
	*pBits = *pBits | (gBits[0][2] << 18);
	*pBits = *pBits | (gBits[0][3] << 21);
}

///////////////////////////////////////////////////////////////////////////////
// flip a DXT5 color block
void flip_blocks_dxtc5(struct DXTColBlock *line, unsigned int numBlocks) {
	struct DXTColBlock *curblock = line;
	struct DXT5AlphaBlock *alphablock;

	for (unsigned int i = 0; i < numBlocks; i++) {
		alphablock = (struct DXT5AlphaBlock*)curblock;

		flip_dxt5_alpha(alphablock);

		curblock++;

		GIMG_SWAP(curblock->row[0], curblock->row[3], uint8_t);
		GIMG_SWAP(curblock->row[1], curblock->row[2], uint8_t);

		curblock++;
	}
}

static void
_revert_y(unsigned int width, unsigned int height, unsigned int format, unsigned int depth, unsigned int size, uint8_t* pixels) {
	unsigned int linesize;
	unsigned int offset;

	if (!is_compressed(format)) {
		assert(depth > 0);

		unsigned int imagesize = size / depth;
		linesize = imagesize / height;

		uint8_t* tmp = (uint8_t*)malloc(linesize);

		for (unsigned int n = 0; n < depth; n++) {
			offset = imagesize * n;
			uint8_t *top = (uint8_t*)pixels + offset;
			uint8_t *bottom = top + (imagesize - linesize);

			for (unsigned int i = 0; i < (height >> 1); i++) {
				// swap
				memcpy(tmp, bottom, linesize);
				memcpy(bottom, top, linesize);
				memcpy(top, tmp, linesize);

				top += linesize;
				bottom -= linesize;
			}
		}

		free(tmp);
	}
	else {
		void(*flipblocks)(struct DXTColBlock*, unsigned int);
		unsigned int xblocks = width / 4;
		unsigned int yblocks = height / 4;
		unsigned int blocksize;

		switch (format) {
		case GPF_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			blocksize = 8;
			flipblocks = flip_blocks_dxtc1;
			break;
		case GPF_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			blocksize = 16;
			flipblocks = flip_blocks_dxtc3;
			break;
		case GPF_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			blocksize = 16;
			flipblocks = flip_blocks_dxtc5;
			break;
		default:
			return;
		}

		linesize = xblocks * blocksize;

		struct DXTColBlock *top;
		struct DXTColBlock *bottom;

		uint8_t *tmp = (uint8_t*)malloc(linesize);

		for (unsigned int j = 0; j < (yblocks >> 1); j++) {
			top = (struct DXTColBlock*)((uint8_t*)pixels + j * linesize);
			bottom = (struct DXTColBlock*)((uint8_t*)pixels + (((yblocks - j) - 1) * linesize));

			flipblocks(top, xblocks);
			flipblocks(bottom, xblocks);

			// swap
			memcpy(tmp, bottom, linesize);
			memcpy(bottom, top, linesize);
			memcpy(top, tmp, linesize);
		}

		free(tmp);
	}
}

uint8_t*
gimg_dds_read_file(const char* filepath, unsigned int* out_width, unsigned int* out_height, unsigned int* out_format) {
	struct fs_file* file = fs_open(filepath, "rb");
	if (file == NULL) {
		fault("Can't open dds file: %s\n", filepath);
	}

	// read in file marker, make sure its a DDS file
	char filecode[4];
	fs_read(file, filecode, sizeof(filecode));
	if (strncmp(filecode, "DDS ", 4) != 0) {
		fault("not a DDS file\n");
	}

	unsigned int format;
	unsigned int components;

	// read in DDS header
    struct DDS_HEADER ddsh;
	fs_read(file, &ddsh, sizeof(ddsh));

    // default to flat texture type (1D, 2D, or rectangle)
	enum TextureType type = TextureFlat;

    // check if image is a cubemap
	if (ddsh.dwCaps2 & DDSF_CUBEMAP) {
		type = TextureCubemap;
	}

    // check if image is a volume texture
	if ((ddsh.dwCaps2 & DDSF_VOLUME) && (ddsh.dwDepth > 0)) {
		type = Texture3D;
	}

    // figure out what the image format is
    if (ddsh.ddspf.dwFlags & DDSF_FOURCC) {
        switch (ddsh.ddspf.dwFourCC) {
        case FOURCC_DXT1:
            format = GPF_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            components = 3;
            break;
        case FOURCC_DXT3:
            format = GPF_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            components = 4;
            break;
        case FOURCC_DXT5:
            format = GPF_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            components = 4;
            break;
        default:
			{
				char str[4];
				fourcc(ddsh.ddspf.dwFourCC, str);
				fault("unknown texture compression '%s'", str);
			}
        }
    } else if (ddsh.ddspf.dwRGBBitCount == 32 &&
               ddsh.ddspf.dwRBitMask == 0x00FF0000 &&
               ddsh.ddspf.dwGBitMask == 0x0000FF00 &&
               ddsh.ddspf.dwBBitMask == 0x000000FF &&
               ddsh.ddspf.dwABitMask == 0xFF000000) {
        format = GPF_BGRA_EXT;
        components = 4;
    } else if (ddsh.ddspf.dwRGBBitCount == 32 &&
               ddsh.ddspf.dwRBitMask == 0x000000FF &&
               ddsh.ddspf.dwGBitMask == 0x0000FF00 &&
               ddsh.ddspf.dwBBitMask == 0x00FF0000 &&
               ddsh.ddspf.dwABitMask == 0xFF000000) {
        format = GPF_RGBA8;
        components = 4;
    } else if (ddsh.ddspf.dwRGBBitCount == 24 &&
               ddsh.ddspf.dwRBitMask == 0x000000FF &&
               ddsh.ddspf.dwGBitMask == 0x0000FF00 &&
               ddsh.ddspf.dwBBitMask == 0x00FF0000) {
        format = GPF_RGB;
        components = 3;
    } else if (ddsh.ddspf.dwRGBBitCount == 24 &&
               ddsh.ddspf.dwRBitMask == 0x00FF0000 &&
               ddsh.ddspf.dwGBitMask == 0x0000FF00 &&
               ddsh.ddspf.dwBBitMask == 0x000000FF) {
        format = GPF_BGR_EXT;
        components = 3;
    } else if (ddsh.ddspf.dwRGBBitCount == 8) {
        format = GPF_LUMINANCE;
        components = 1;
    } else {
		fault("unknow texture format");
    }

    // store primary surface width/height/depth
    unsigned int width, height, depth;
    width = ddsh.dwWidth;
    height = ddsh.dwHeight;
    depth = clamp_size(ddsh.dwDepth);   // set to 1 if 0

    // use correct size calculation function depending on whether image is
    // compressed
    unsigned int (*sizefunc)(unsigned int, unsigned int, unsigned int);
    sizefunc = (is_compressed(format) ? &size_dxtc : &size_rgb);

    // load all surfaces for the image (6 surfaces for cubemaps)
//	struct texture textures[6];
//	for (unsigned int n = 0; n < (unsigned int) (type == TextureCubemap ? 6 : 1); n++) {
        // get reference to newly added texture object

        // calculate surface size
        unsigned int size = is_compressed(format) ?
			sizefunc(width, height, format) * depth :
			sizefunc(width, height, components) * depth;

        // load surface
        uint8_t* pixels = (uint8_t*)malloc(size);
		fs_read(file, pixels, size);

#ifdef DDS_REVERT_Y
		_revert_y(width, height, format, depth, size, pixels);
#endif // DDS_REVERT_Y

		// fill tex
		*out_width  = width;
		*out_height = height;
		*out_format = format;
		return pixels;
		//tex->width  = width;
		//tex->height = height;
		//tex->depth  = depth;
		//tex->size   = size;
		//tex->pixels = pixels;

        //if (flipImage)
        //    flip(img);

        unsigned int w = clamp_size(width >> 1);
        unsigned int h = clamp_size(height >> 1);
        unsigned int d = clamp_size(depth >> 1);

        // store number of mipmaps
        unsigned int numMipmaps = ddsh.dwMipMapCount;

        // number of mipmaps in file includes main surface so decrease count
        // by one
        if (numMipmaps != 0)
            numMipmaps--;

		// todo
        //// load all mipmaps for current surface
        //for (unsigned int i = 0; i < numMipmaps && (w || h); i++) {
        //    // add empty surface
        //    img.add_mipmap(CSurface());

        //    // get reference to newly added mipmap
        //    CSurface &mipmap = img.get_mipmap(i);

        //    // calculate mipmap size
        //    size = (this->*sizefunc)(w, h) * d;

        //    uint8_t *pixels = new uint8_t[size];
        //    is.read((char*)pixels, size);

        //    mipmap.create(w, h, d, size, pixels);

        //    delete[] pixels;

        //    if (flipImage)
        //        flip(mipmap);

        //    // shrink to next power of 2
        //    w = clamp_size(w >> 1);
        //    h = clamp_size(h >> 1);
        //    d = clamp_size(d >> 1);
        //}
//	}

	// todo
    //// swap cubemaps on y axis (since image is flipped in OGL)
    //if (type == TextureCubemap && flipImage) {
    //    CTexture tmp;
    //    tmp = m_images[3];
    //    m_images[3] = m_images[2];
    //    m_images[2] = tmp;
    //}
}