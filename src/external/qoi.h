/*

QOI - The "Quite OK Image" format for fast, lossless image compression

Dominic Szablewski - https://phoboslab.org


-- LICENSE: The MIT License(MIT)

Copyright(c) 2021 Dominic Szablewski

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files(the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


-- About

QOI encodes and decodes images in a lossless format. An encoded QOI image is
usually around 10--30% larger than a decently optimized PNG image.

QOI outperforms simpler PNG encoders in compression ratio and performance. QOI
images are typically 20% smaller than PNGs written with stbi_image but 10% 
larger than with libpng. Encoding is 25-50x faster and decoding is 3-4x faster 
than stbi_image or libpng.


-- Synopsis

// Define `QOI_IMPLEMENTATION` in *one* C/C++ file before including this
// library to create the implementation.

#define QOI_IMPLEMENTATION
#include "qoi.h"

// Encode and store an RGBA buffer to the file system. The qoi_desc describes
// the input pixel data.
qoi_write("image_new.qoi", rgba_pixels, &(qoi_desc){
	.width = 1920,
	.height = 1080, 
	.channels = 4,
	.colorspace = QOI_SRGB
});

// Load and decode a QOI image from the file system into a 32bbp RGBA buffer.
// The qoi_desc struct will be filled with the width, height, number of channels
// and colorspace read from the file header.
qoi_desc desc;
void *rgba_pixels = qoi_read("image.qoi", &desc, 4);



-- Documentation

This library provides the following functions;
- qoi_read    -- read and decode a QOI file
- qoi_decode  -- decode the raw bytes of a QOI image from memory
- qoi_write   -- encode and write a QOI file
- qoi_encode  -- encode an rgba buffer into a QOI image in memory

See the function declaration below for the signature and more information.

If you don't want/need the qoi_read and qoi_write functions, you can define
QOI_NO_STDIO before including this library.

This library uses malloc() and free(). To supply your own malloc implementation
you can define QOI_MALLOC and QOI_FREE before including this library.


-- Data Format

A QOI file has a 14 byte header, followed by any number of data "chunks".

struct qoi_header_t {
	char     magic[4];   // magic bytes "qoif"
	uint32_t width;      // image width in pixels (BE)
	uint32_t height;     // image height in pixels (BE)
	uint8_t  channels;   // must be 3 (RGB) or 4 (RGBA)
	uint8_t  colorspace; // a bitmap 0000rgba where
	                     //   - a zero bit indicates sRGBA, 
	                     //   - a one bit indicates linear (user interpreted)
	                     //   colorspace for each channel
};

The decoder and encoder start with {r: 0, g: 0, b: 0, a: 255} as the previous
pixel value. Pixels are either encoded as
 - a run of the previous pixel
 - an index into a previously seen pixel
 - a difference to the previous pixel value in r,g,b,a
 - full r,g,b,a values

A running array[64] of previously seen pixel values is maintained by the encoder
and decoder. Each pixel that is seen by the encoder and decoder is put into this
array at the position (r^g^b^a) % 64. In the encoder, if the pixel value at this
index matches the current pixel, this index position is written to the stream.

Each chunk starts with a 2, 3 or 4 bit tag, followed by a number of data bits. 
The bit length of chunks is divisible by 8 - i.e. all chunks are byte aligned.

QOI_INDEX {
	u8 tag  :  2;   // b00
	u8 idx  :  6;   // 6-bit index into the color index array: 0..63
}

QOI_RUN_8 {
	u8 tag  :  3;   // b010
	u8 run  :  5;   // 5-bit run-length repeating the previous pixel: 1..32
}

QOI_RUN_16 {
	u8 tag  :  3;   // b011
	u16 run : 13;   // 13-bit run-length repeating the previous pixel: 33..8224
}

QOI_DIFF_8 {
	u8 tag  :  2;   // b10
	u8 dr   :  2;   // 2-bit   red channel difference: -2..1
	u8 dg   :  2;   // 2-bit green channel difference: -2..1
	u8 db   :  2;   // 2-bit  blue channel difference: -2..1
}

QOI_DIFF_16 {
	u8 tag  :  3;   // b110
	u8 dr   :  5;   // 5-bit   red channel difference: -16..15
	u8 dg   :  4;   // 4-bit green channel difference:  -8.. 7
	u8 db   :  4;   // 4-bit  blue channel difference:  -8.. 7
}

QOI_DIFF_24 {
	u8 tag  :  4;   // b1110
	u8 dr   :  5;   // 5-bit   red channel difference: -16..15
	u8 dg   :  5;   // 5-bit green channel difference: -16..15
	u8 db   :  5;   // 5-bit  blue channel difference: -16..15
	u8 da   :  5;   // 5-bit alpha channel difference: -16..15
}

QOI_COLOR {
	u8 tag  :  4;   // b1111
	u8 has_r:  1;   //   red byte follows
	u8 has_g:  1;   // green byte follows
	u8 has_b:  1;   //  blue byte follows
	u8 has_a:  1;   // alpha byte follows
	u8 r;           //   red value if has_r == 1: 0..255
	u8 g;           // green value if has_g == 1: 0..255
	u8 b;           //  blue value if has_b == 1: 0..255
	u8 a;           // alpha value if has_a == 1: 0..255
}

The byte stream is padded with 4 zero bytes. Size the longest chunk we can
encounter is 5 bytes (QOI_COLOR with RGBA set), with this padding we just have 
to check for an overrun once per decode loop iteration.

*/


// -----------------------------------------------------------------------------
// Header - Public functions

#ifndef QOI_H
#define QOI_H

#ifdef __cplusplus
extern "C" {
#endif

// A pointer to qoi_desc struct has to be supplied to all of qoi's functions. It
// describes either the input format (for qoi_write, qoi_encode), or is filled
// with the description read from the file header (for qoi_read, qoi_decode).

// The colorspace in this qoi_desc is a bitmap with 0000rgba where a 0-bit 
// indicates sRGB and a 1-bit indicates linear colorspace for each channel. You 
// may use one of the predefined constants: QOI_SRGB, QOI_SRGB_LINEAR_ALPHA or 
// QOI_LINEAR. The colorspace is purely informative. It will be saved to the
// file header, but does not affect en-/decoding in any way.

#define QOI_SRGB 0x00
#define QOI_SRGB_LINEAR_ALPHA 0x01
#define QOI_LINEAR 0x0f

typedef struct {
	unsigned int width;
	unsigned int height;
	unsigned char channels;
	unsigned char colorspace;
} qoi_desc;

#ifndef QOI_NO_STDIO

// Encode raw RGB or RGBA pixels into a QOI image and write it to the file 
// system. The qoi_desc struct must be filled with the image width, height, 
// number of channels (3 = RGB, 4 = RGBA) and the colorspace. 

// The function returns 0 on failure (invalid parameters, or fopen or malloc 
// failed) or the number of bytes written on success.

int qoi_write(const char *filename, const void *data, const qoi_desc *desc);


// Read and decode a QOI image from the file system. If channels is 0, the
// number of channels from the file header is used. If channels is 3 or 4 the
// output format will be forced into this number of channels.

// The function either returns NULL on failure (invalid data, or malloc or fopen
// failed) or a pointer to the decoded pixels. On success, the qoi_desc struct 
// will be filled with the description from the file header.

// The returned pixel data should be free()d after use.

void *qoi_read(const char *filename, qoi_desc *desc, int channels);

#endif // QOI_NO_STDIO


// Encode raw RGB or RGBA pixels into a QOI image in memory.

// The function either returns NULL on failure (invalid parameters or malloc 
// failed) or a pointer to the encoded data on success. On success the out_len 
// is set to the size in bytes of the encoded data.

// The returned qoi data should be free()d after user.

void *qoi_encode(const void *data, const qoi_desc *desc, int *out_len);


// Decode a QOI image from memory.

// The function either returns NULL on failure (invalid parameters or malloc 
// failed) or a pointer to the decoded pixels. On success, the qoi_desc struct 
// is filled with the description from the file header.

// The returned pixel data should be free()d after use.

void *qoi_decode(const void *data, int size, qoi_desc *desc, int channels);


#ifdef __cplusplus
}
#endif
#endif // QOI_H


// -----------------------------------------------------------------------------
// Implementation

#ifdef QOI_IMPLEMENTATION
#include <stdlib.h>

#ifndef QOI_MALLOC
	#define QOI_MALLOC(sz) malloc(sz)
	#define QOI_FREE(p)    free(p)
#endif

#define QOI_INDEX   0x00 // 00xxxxxx
#define QOI_RUN_8   0x40 // 010xxxxx
#define QOI_RUN_16  0x60 // 011xxxxx
#define QOI_DIFF_8  0x80 // 10xxxxxx
#define QOI_DIFF_16 0xc0 // 110xxxxx
#define QOI_DIFF_24 0xe0 // 1110xxxx
#define QOI_COLOR   0xf0 // 1111xxxx

#define QOI_MASK_2  0xc0 // 11000000
#define QOI_MASK_3  0xe0 // 11100000
#define QOI_MASK_4  0xf0 // 11110000

#define QOI_COLOR_HASH(C) (C.rgba.r ^ C.rgba.g ^ C.rgba.b ^ C.rgba.a)
#define QOI_MAGIC \
	(((unsigned int)'q') << 24 | ((unsigned int)'o') << 16 | \
	 ((unsigned int)'i') <<  8 | ((unsigned int)'f'))
#define QOI_HEADER_SIZE 14
#define QOI_PADDING 4

typedef union {
	struct { unsigned char r, g, b, a; } rgba;
	unsigned int v;
} qoi_rgba_t;

void qoi_write_32(unsigned char *bytes, int *p, unsigned int v) {
	bytes[(*p)++] = (0xff000000 & v) >> 24;
	bytes[(*p)++] = (0x00ff0000 & v) >> 16;
	bytes[(*p)++] = (0x0000ff00 & v) >> 8;
	bytes[(*p)++] = (0x000000ff & v);
}

unsigned int qoi_read_32(const unsigned char *bytes, int *p) {
	unsigned int a = bytes[(*p)++];
	unsigned int b = bytes[(*p)++];
	unsigned int c = bytes[(*p)++];
	unsigned int d = bytes[(*p)++];
	return (a << 24) | (b << 16) | (c << 8) | d;
}

void *qoi_encode(const void *data, const qoi_desc *desc, int *out_len) {
	if (
		data == NULL || out_len == NULL || desc == NULL ||
		desc->width == 0 || desc->height == 0 ||
		desc->channels < 3 || desc->channels > 4 ||
		(desc->colorspace & 0xf0) != 0
	) {
		return NULL;
	}

	int max_size = 
		desc->width * desc->height * (desc->channels + 1) + 
		QOI_HEADER_SIZE + QOI_PADDING;

	int p = 0;
	unsigned char *bytes = QOI_MALLOC(max_size);
	if (!bytes) {
		return NULL;
	}

	qoi_write_32(bytes, &p, QOI_MAGIC);
	qoi_write_32(bytes, &p, desc->width);
	qoi_write_32(bytes, &p, desc->height);
	bytes[p++] = desc->channels;
	bytes[p++] = desc->colorspace;


	const unsigned char *pixels = (const unsigned char *)data;

	qoi_rgba_t index[64] = {0};

	int run = 0;
	qoi_rgba_t px_prev = {.rgba = {.r = 0, .g = 0, .b = 0, .a = 255}};
	qoi_rgba_t px = px_prev;
	
	int px_len = desc->width * desc->height * desc->channels;
	int px_end = px_len - desc->channels;
	for (int px_pos = 0; px_pos < px_len; px_pos += desc->channels) {
		if (desc->channels == 4) {
			px = *(qoi_rgba_t *)(pixels + px_pos);
		}
		else {
			px.rgba.r = pixels[px_pos];
			px.rgba.g = pixels[px_pos+1];
			px.rgba.b = pixels[px_pos+2];
		}

		if (px.v == px_prev.v) {
			run++;
		}

		if (run > 0 && (run == 0x2020 || px.v != px_prev.v || px_pos == px_end)) {
			if (run < 33) {
				run -= 1;
				bytes[p++] = QOI_RUN_8 | run;
			}
			else {
				run -= 33;
				bytes[p++] = QOI_RUN_16 | run >> 8;
				bytes[p++] = run;
			}
			run = 0;
		}

		if (px.v != px_prev.v) {
			int index_pos = QOI_COLOR_HASH(px) % 64;

			if (index[index_pos].v == px.v) {
				bytes[p++] = QOI_INDEX | index_pos;
			}
			else {
				index[index_pos] = px;

				int vr = px.rgba.r - px_prev.rgba.r;
				int vg = px.rgba.g - px_prev.rgba.g;
				int vb = px.rgba.b - px_prev.rgba.b;
				int va = px.rgba.a - px_prev.rgba.a;

				if (
					vr > -17 && vr < 16 && vg > -17 && vg < 16 && 
					vb > -17 && vb < 16 && va > -17 && va < 16
				) {
					if (
						va == 0 && vr > -3 && vr < 2 &&
						vg > -3 && vg < 2 && vb > -3 && vb < 2
					) {
						bytes[p++] = QOI_DIFF_8 | ((vr + 2) << 4) | (vg + 2) << 2 | (vb + 2);
					}
					else if (
						va == 0 && vr > -17 && vr < 16 && 
						vg > -9 && vg < 8 && vb > -9 && vb < 8
					) {
						bytes[p++] = QOI_DIFF_16 | (vr + 16);
						bytes[p++] = ((vg + 8) << 4) | (vb + 8);
					}
					else {
						bytes[p++] = QOI_DIFF_24 | ((vr + 16) >> 1);
						bytes[p++] = ((vr + 16) << 7) | ((vg + 16) << 2) | ((vb + 16) >> 3);
						bytes[p++] = ((vb + 16) << 5) | (va + 16);
					}
				}
				else {
					bytes[p++] = QOI_COLOR | (vr?8:0)|(vg?4:0)|(vb?2:0)|(va?1:0);
					if (vr) { bytes[p++] = px.rgba.r; }
					if (vg) { bytes[p++] = px.rgba.g; }
					if (vb) { bytes[p++] = px.rgba.b; }
					if (va) { bytes[p++] = px.rgba.a; }
				}
			}
		}
		px_prev = px;
	}

	for (int i = 0; i < QOI_PADDING; i++) {
		bytes[p++] = 0;
	}

	*out_len = p;
	return bytes;
}

void *qoi_decode(const void *data, int size, qoi_desc *desc, int channels) {
	if (
		data == NULL || desc == NULL ||
		(channels != 0 && channels != 3 && channels != 4) ||
		size < QOI_HEADER_SIZE + QOI_PADDING
	) {
		return NULL;
	}

	const unsigned char *bytes = (const unsigned char *)data;
	int p = 0;

	unsigned int header_magic = qoi_read_32(bytes, &p);
	desc->width = qoi_read_32(bytes, &p);
	desc->height = qoi_read_32(bytes, &p);
	desc->channels = bytes[p++];
	desc->colorspace = bytes[p++];

	if (
		desc->width == 0 || desc->height == 0 || 
		desc->channels < 3 || desc->channels > 4 ||
		header_magic != QOI_MAGIC
	) {
		return NULL;
	}

	if (channels == 0) {
		channels = desc->channels;
	}

	int px_len = desc->width * desc->height * channels;
	unsigned char *pixels = QOI_MALLOC(px_len);
	if (!pixels) {
		return NULL;
	}

	qoi_rgba_t px = {.rgba = {.r = 0, .g = 0, .b = 0, .a = 255}};
	qoi_rgba_t index[64] = {0};

	int run = 0;
	int chunks_len = size - QOI_PADDING;
	for (int px_pos = 0; px_pos < px_len; px_pos += channels) {
		if (run > 0) {
			run--;
		}
		else if (p < chunks_len) {
			int b1 = bytes[p++];

			if ((b1 & QOI_MASK_2) == QOI_INDEX) {
				px = index[b1 ^ QOI_INDEX];
			}
			else if ((b1 & QOI_MASK_3) == QOI_RUN_8) {
				run = (b1 & 0x1f);
			}
			else if ((b1 & QOI_MASK_3) == QOI_RUN_16) {
				int b2 = bytes[p++];
				run = (((b1 & 0x1f) << 8) | (b2)) + 32;
			}
			else if ((b1 & QOI_MASK_2) == QOI_DIFF_8) {
				px.rgba.r += ((b1 >> 4) & 0x03) - 2;
				px.rgba.g += ((b1 >> 2) & 0x03) - 2;
				px.rgba.b += ( b1       & 0x03) - 2;
			}
			else if ((b1 & QOI_MASK_3) == QOI_DIFF_16) {
				int b2 = bytes[p++];
				px.rgba.r += (b1 & 0x1f) - 16;
				px.rgba.g += (b2 >> 4) - 8;
				px.rgba.b += (b2 & 0x0f) - 8;
			}
			else if ((b1 & QOI_MASK_4) == QOI_DIFF_24) {
				int b2 = bytes[p++];
				int b3 = bytes[p++];
				px.rgba.r += (((b1 & 0x0f) << 1) | (b2 >> 7)) - 16;
				px.rgba.g +=  ((b2 & 0x7c) >> 2) - 16;
				px.rgba.b += (((b2 & 0x03) << 3) | ((b3 & 0xe0) >> 5)) - 16;
				px.rgba.a +=   (b3 & 0x1f) - 16;
			}
			else if ((b1 & QOI_MASK_4) == QOI_COLOR) {
				if (b1 & 8) { px.rgba.r = bytes[p++]; }
				if (b1 & 4) { px.rgba.g = bytes[p++]; }
				if (b1 & 2) { px.rgba.b = bytes[p++]; }
				if (b1 & 1) { px.rgba.a = bytes[p++]; }
			}

			index[QOI_COLOR_HASH(px) % 64] = px;
		}

		if (channels == 4) { 
			*(qoi_rgba_t*)(pixels + px_pos) = px;
		}
		else {
			pixels[px_pos] = px.rgba.r;
			pixels[px_pos+1] = px.rgba.g;
			pixels[px_pos+2] = px.rgba.b;
		}
	}

	return pixels;
}

#ifndef QOI_NO_STDIO
#include <stdio.h>

int qoi_write(const char *filename, const void *data, const qoi_desc *desc) {
	int size;
	void *encoded = qoi_encode(data, desc, &size);
	if (!encoded) {
		return 0;
	}

	FILE *f = fopen(filename, "wb");
	if (!f) {
		QOI_FREE(encoded);
		return 0;
	}
	
	fwrite(encoded, 1, size, f);
	fclose(f);
	QOI_FREE(encoded);
	return size;
}

void *qoi_read(const char *filename, qoi_desc *desc, int channels) {
	FILE *f = fopen(filename, "rb");
	if (!f) {
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);

	void *data = QOI_MALLOC(size);
	if (!data) {
		fclose(f);
		return NULL;
	}

	int bytes_read = fread(data, 1, size, f);
	fclose(f);

	void *pixels = qoi_decode(data, bytes_read, desc, channels);
	QOI_FREE(data);
	return pixels;
}

#endif // QOI_NO_STDIO
#endif // QOI_IMPLEMENTATION
