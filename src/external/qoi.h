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

// Load and decode a QOI image from the file system into a 32bbp RGBA buffer
int width, height;
void *rgba_pixels = qoi_read("image.qoi", &width, &height, 4);

// Encode and store an RGBA buffer to the file system
qoi_write("image_new.qoi", rgba_pixels, width, height, 4);


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

A QOI file has a 12 byte header, followed by any number of data "chunks".

struct qoi_header_t {
	char [4];              // magic bytes "qoif"
	unsigned short width;  // image width in pixels (BE)
	unsigned short height; // image height in pixels (BE)
	unsigned int size;     // number of data bytes following this header (BE)
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
	u8 dr   :  2;   // 2-bit   red channel difference: -1..2
	u8 dg   :  2;   // 2-bit green channel difference: -1..2
	u8 db   :  2;   // 2-bit  blue channel difference: -1..2
}

QOI_DIFF_16 {
	u8 tag  :  3;   // b110
	u8 dr   :  5;   // 5-bit   red channel difference: -15..16
	u8 dg   :  4;   // 4-bit green channel difference:  -7.. 8
	u8 db   :  4;   // 4-bit  blue channel difference:  -7.. 8
}

QOI_DIFF_24 {
	u8 tag  :  4;   // b1110
	u8 dr   :  5;   // 5-bit   red channel difference: -15..16
	u8 dg   :  5;   // 5-bit green channel difference: -15..16
	u8 db   :  5;   // 5-bit  blue channel difference: -15..16
	u8 da   :  5;   // 5-bit alpha channel difference: -15..16
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

#ifndef QOI_NO_STDIO

// Encode raw RGB or RGBA pixels into a QOI image write it to the file system.
// w and h denote the the width and height of the pixel data. channels must be 
// either 3 for RGB data or 4 for RGBA.
// The function returns 0 on failure (invalid parameters, or fopen or malloc 
// failed) or the number of bytes written on success.

int qoi_write(const char *filename, const void *data, int w, int h, int channels);


// Read and decode a QOI image from the file system into either raw RGB 
// (channels=3) or RGBA (channels=4) pixel data.
// The function either returns NULL on failure (invalid data, or malloc or fopen
// failed) or a pointer to the decoded pixels. On success out_w and out_h will 
// be set to the width and height of the decoded image.
// The returned pixel data should be free()d after use.

void *qoi_read(const char *filename, int *out_w, int *out_h, int channels);

#endif // QOI_NO_STDIO


// Encode raw RGB or RGBA pixels into a QOI image in memory. w and h denote the
// width and height of the pixel data. channels must be either 3 for RGB data 
// or 4 for RGBA.
// The function either returns NULL on failure (invalid parameters or malloc 
// failed) or a pointer to the encoded data on success. On success the out_len
// is set to the size in bytes of the encoded data.
// The returned qoi data should be free()d after user.

void *qoi_encode(const void *data, int w, int h, int channels, int *out_len);


// Decode a QOI image from memory into either raw RGB (channels=3) or RGBA 
// (channels=4) pixel data.
// The function either returns NULL on failure (invalid parameters or malloc 
// failed) or a pointer to the decoded pixels. On success out_w and out_h will
// be set to the width and height of the decoded image.
// The returned pixel data should be free()d after use.

void *qoi_decode(const void *data, int size, int *out_w, int *out_h, int channels);

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
#define QOI_HEADER_SIZE 12
#define QOI_PADDING 4

typedef union {
	struct { unsigned char r, g, b, a; } rgba;
	unsigned int v;
} qoi_rgba_t;

void qoi_write_16(unsigned char *bytes, int *p, unsigned short v) {
	bytes[(*p)++] = (0xff00 & v) >> 8;
	bytes[(*p)++] = (0xff & v);
}

void qoi_write_32(unsigned char *bytes, int *p, unsigned int v) {
	qoi_write_16(bytes, p, (v & 0xffff0000) >> 16);
	qoi_write_16(bytes, p, (v & 0xffff));
}

unsigned int qoi_read_16(const unsigned char *bytes, int *p) {
	unsigned int a = bytes[(*p)++];
	unsigned int b = bytes[(*p)++];
	return (a << 8) | b;
}

unsigned int qoi_read_32(const unsigned char *bytes, int *p) {
	unsigned int a = qoi_read_16(bytes, p);
	unsigned int b = qoi_read_16(bytes, p);
	return (a << 16) | b;
}

void *qoi_encode(const void *data, int w, int h, int channels, int *out_len) {
	if (
		data == NULL || out_len == NULL ||
		w <= 0 || w >= (1 << 16) ||
		h <= 0 || h >= (1 << 16) ||
		channels < 3 || channels > 4
	) {
		return NULL;
	}

	int max_size = w * h * (channels + 1) + QOI_HEADER_SIZE + QOI_PADDING;
	int p = 0;
	unsigned char *bytes = QOI_MALLOC(max_size);
	if (!bytes) {
		return NULL;
	}

	qoi_write_32(bytes, &p, QOI_MAGIC);
	qoi_write_16(bytes, &p, w);
	qoi_write_16(bytes, &p, h);
	qoi_write_32(bytes, &p, 0); // size, will be set later

	const unsigned char *pixels = (const unsigned char *)data;

	qoi_rgba_t index[64] = {0};

	int run = 0;
	qoi_rgba_t px_prev = {.rgba = {.r = 0, .g = 0, .b = 0, .a = 255}};
	qoi_rgba_t px = px_prev;

	int px_len = w * h * channels;
	int px_end = px_len - channels;
	for (int px_pos = 0; px_pos < px_len; px_pos += channels) {
		if (channels == 4) {
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
					vr > -16 && vr < 17 && vg > -16 && vg < 17 && 
					vb > -16 && vb < 17 && va > -16 && va < 17
				) {
					if (
						va == 0 && vr > -2 && vr < 3 &&
						vg > -2 && vg < 3 && vb > -2 && vb < 3
					) {
						bytes[p++] = QOI_DIFF_8 | ((vr + 1) << 4) | (vg + 1) << 2 | (vb + 1);
					}
					else if (
						va == 0 && vr > -16 && vr < 17 && 
						vg > -8 && vg < 9 && vb > -8 && vb < 9
					) {
						bytes[p++] = QOI_DIFF_16 | (vr + 15);
						bytes[p++] = ((vg + 7) << 4) | (vb + 7);
					}
					else {
						bytes[p++] = QOI_DIFF_24 | ((vr + 15) >> 1);
						bytes[p++] = ((vr + 15) << 7) | ((vg + 15) << 2) | ((vb + 15) >> 3);
						bytes[p++] = ((vb + 15) << 5) | (va + 15);
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

	int data_len = p - QOI_HEADER_SIZE;
	*out_len = p;

	p = 8;
	qoi_write_32(bytes, &p, data_len);
	return bytes;
}

void *qoi_decode(const void *data, int size, int *out_w, int *out_h, int channels) {
	if (channels < 3 || channels > 4 || size < QOI_HEADER_SIZE) {
		return NULL;
	}

	const unsigned char *bytes = (const unsigned char *)data;
	int p = 0;

	int magic = qoi_read_32(bytes, &p);
	int w = qoi_read_16(bytes, &p);
	int h = qoi_read_16(bytes, &p);
	int data_len = qoi_read_32(bytes, &p);

	if (
		w == 0 || h == 0 || magic != QOI_MAGIC || 
		size != data_len + QOI_HEADER_SIZE
	) {
		return NULL;
	}

	int px_len = w * h * channels;
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
				px.rgba.r += ((b1 >> 4) & 0x03) - 1;
				px.rgba.g += ((b1 >> 2) & 0x03) - 1;
				px.rgba.b += ( b1       & 0x03) - 1;
			}
			else if ((b1 & QOI_MASK_3) == QOI_DIFF_16) {
				int b2 = bytes[p++];
				px.rgba.r += (b1 & 0x1f) - 15;
				px.rgba.g += (b2 >> 4) - 7;
				px.rgba.b += (b2 & 0x0f) - 7;
			}
			else if ((b1 & QOI_MASK_4) == QOI_DIFF_24) {
				int b2 = bytes[p++];
				int b3 = bytes[p++];
				px.rgba.r += (((b1 & 0x0f) << 1) | (b2 >> 7)) - 15;
				px.rgba.g +=  ((b2 & 0x7c) >> 2) - 15;
				px.rgba.b += (((b2 & 0x03) << 3) | ((b3 & 0xe0) >> 5)) - 15;
				px.rgba.a +=   (b3 & 0x1f) - 15;
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

	*out_w = w;
	*out_h = h;
	return pixels;
}

#ifndef QOI_NO_STDIO
#include <stdio.h>

int qoi_write(const char *filename, const void *data, int w, int h, int channels) {
	int size;
	void *encoded = qoi_encode(data, w, h, channels, &size);
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

void *qoi_read(const char *filename, int *out_w, int *out_h, int channels) {
	FILE *f = fopen(filename, "rb");
	if (!f) {
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);

	void *data = QOI_MALLOC(size);
	if (!data) {
		return NULL;
	}

	int bytes_read = fread(data, 1, size, f);
	fclose(f);

	void *pixels = qoi_decode(data, bytes_read, out_w, out_h, channels);
	QOI_FREE(data);
	return pixels;
}

#endif // QOI_NO_STDIO
#endif // QOI_IMPLEMENTATION
