// from NeHe-Tutorial's TGALoader.cpp

#include "gimg_tga.h"
#include "gimg_typedef.h"

#include <fs_file.h>
#include <fault.h>
#include <logger.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct
{
	char header[12];
} TGAHeader;

typedef struct
{
	unsigned char header[6];
	unsigned int  bytes_per_pixel;
	unsigned int  image_size;
	unsigned int  temp;
	unsigned int  type;
	unsigned int  height;
	unsigned int  width;
	unsigned int  bpp;
} TGA;

static char uTGAcompare[12] = { 0,0,2, 0,0,0,0,0,0,0,0,0 };
static char uAlphaTGAcompare[12] = { 0,0,3, 0,0,0,0,0,0,0,0,0 };
static char cTGAcompare[12] = { 0,0,10,0,0,0,0,0,0,0,0,0 };

static uint8_t*
_load_uncompressed_color(struct fs_file* file, int* width, int* height, int* format) {
	TGA tga;
	if (fs_read(file, tga.header, sizeof(tga.header)) != sizeof(tga.header)) {
		return NULL;
	}

	*width = tga.header[1] * 256 + tga.header[0];
	*height = tga.header[3] * 256 + tga.header[2];
	size_t bpp = tga.header[4];
	tga.width = *width;
	tga.height = *height;
	tga.bpp = bpp;

	if ((*width <= 0) || (*height <= 0) || ((bpp != 24) && (bpp != 32))) {
		return NULL;
	}

	if (bpp == 24) {
		*format = GPF_RGB;
	} else {
		*format = GPF_RGBA8;
	}

	tga.bytes_per_pixel = tga.bpp / 8;
	tga.image_size = tga.bytes_per_pixel * tga.width * tga.height;
	uint8_t* pixels = (uint8_t*)malloc(tga.image_size);
	if (pixels == NULL) {
		return pixels;
	}

	if (fs_read(file, pixels, tga.image_size) != tga.image_size) {
		return NULL;
	}


	for (int cswap = 0; cswap < (int)tga.image_size; cswap += tga.bytes_per_pixel) {
		pixels[cswap] ^= pixels[cswap + 2] ^= pixels[cswap] ^= pixels[cswap + 2];
	}

	return pixels;
}

static uint8_t*
_load_uncompressed_alpha(struct fs_file* file, int* width, int* height, int* format) {
	TGA tga;
	if (fs_read(file, tga.header, sizeof(tga.header)) != sizeof(tga.header)) {
		return NULL;
	}

	*width = tga.header[1] * 256 + tga.header[0];
	*height = tga.header[3] * 256 + tga.header[2];
	size_t bpp = tga.header[4];
	tga.width = *width;
	tga.height = *height;
	tga.bpp = bpp;

	if (*width <= 0 || *height <= 0 || (bpp != 8)) {
		return NULL;
	}

	*format = GPF_ALPHA;

	tga.bytes_per_pixel = tga.bpp / 8;
	tga.image_size = tga.bytes_per_pixel * tga.width * tga.height;
	uint8_t* pixels = (uint8_t*)malloc(tga.image_size);
	if (pixels == NULL) {
		return pixels;
	}

	if (fs_read(file, pixels, tga.image_size) != tga.image_size) {
		return NULL;
	}

	return pixels;
}

static uint8_t*
_load_compressed(struct fs_file* file, int* width, int* height, int* format) {
	TGA tga;
	if (fs_read(file, tga.header, sizeof(tga.header)) != sizeof(tga.header)) {
		return NULL;
	}

	*width = tga.header[1] * 256 + tga.header[0];
	*height = tga.header[3] * 256 + tga.header[2];
	size_t bpp = tga.header[4];
	tga.width = *width;
	tga.height = *height;
	tga.bpp = bpp;

	if (*width <= 0 || *height <= 0 || (bpp != 24 && bpp != 32)) {
		fs_close(file);
		return NULL;
	}


	if (bpp == 24) {
		*format = GPF_RGB;
	} else {
		*format = GPF_RGBA8;
	}

	tga.bytes_per_pixel = tga.bpp / 8;
	tga.image_size = tga.bytes_per_pixel * tga.width * tga.height;
	uint8_t* pixels = (uint8_t*)malloc(tga.image_size);
	if (pixels == NULL) {
		fs_close(file);
		return NULL;
	}

	size_t pixelcount = tga.height * tga.width;
	size_t currentpixel = 0;
	size_t currentbyte = 0;
	unsigned char* colorbuffer = (unsigned char*)malloc(tga.bytes_per_pixel);

	do
	{
		unsigned char chunkheader = 0;
		if (fs_read(file, &chunkheader, sizeof(chunkheader)) != sizeof(chunkheader)) {
			free(colorbuffer);
			fs_close(file);
			free(pixels);
			return NULL;
		}

		if (chunkheader < 128)
		{
			chunkheader++;
			for (short counter = 0; counter < chunkheader; counter++)
			{
				if (fs_read(file, colorbuffer, tga.bytes_per_pixel) != tga.bytes_per_pixel) {
					free(colorbuffer);
					fs_close(file);
					free(pixels);
					return NULL;
				}

				pixels[currentbyte] = colorbuffer[2];
				pixels[currentbyte + 1] = colorbuffer[1];
				pixels[currentbyte + 2] = colorbuffer[0];

				if (tga.bytes_per_pixel == 4)
				{
					pixels[currentbyte + 3] = colorbuffer[3];
				}

				currentbyte += tga.bytes_per_pixel;
				currentpixel++;

				if (currentpixel > pixelcount) {
					free(colorbuffer);
					fs_close(file);
					free(pixels);
					return NULL;
				}
			}
		}
		else
		{
			chunkheader -= 127;
			if (fs_read(file, colorbuffer, tga.bytes_per_pixel) != tga.bytes_per_pixel) {
				free(colorbuffer);
				fs_close(file);
				free(pixels);
				return NULL;
			}

			for (short counter = 0; counter < chunkheader; counter++)
			{
				pixels[currentbyte] = colorbuffer[2];
				pixels[currentbyte + 1] = colorbuffer[1];
				pixels[currentbyte + 2] = colorbuffer[0];

				if (tga.bytes_per_pixel == 4)
				{
					pixels[currentbyte + 3] = colorbuffer[3];
				}

				currentbyte += tga.bytes_per_pixel;
				currentpixel++;

				if (currentpixel > pixelcount) {
					free(colorbuffer);
					fs_close(file);
					free(pixels);
					return NULL;
				}
			}
		}
	} while (currentpixel < pixelcount);

	free(colorbuffer);
	fs_close(file);

	return pixels;
}

uint8_t*
gimg_tga_read_file(const char* filepath, int* width, int* height, int* format) {
	struct fs_file* file = fs_open(filepath, "rb");
	if (file == NULL) {
        fprintf(stderr, "can't open %s\n", filepath);
        return NULL;
	}

	TGAHeader tgaheader;
	if (fs_read(file, &tgaheader, sizeof(tgaheader)) != sizeof(tgaheader)) {
		fault("Invalid image file: %s\n", filepath);
	}

	if (memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0) {
		return _load_uncompressed_color(file, width, height, format);
	} else if (memcmp(uAlphaTGAcompare, &tgaheader, sizeof(tgaheader)) == 0) {
		return _load_uncompressed_alpha(file, width, height, format);
	} else if (memcmp(cTGAcompare, &tgaheader, sizeof(tgaheader)) == 0) {
		return _load_compressed(file, width, height, format);
	} else {
		fault("Unknown tga type: %s\n", filepath);
		return NULL;
	}
}