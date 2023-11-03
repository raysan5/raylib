/*
# Small Deflate
`sdefl` is a small bare bone lossless compression library in ANSI C (ISO C90)
which implements the Deflate (RFC 1951) compressed data format specification standard.
It is mainly tuned to get as much speed and compression ratio from as little code
as needed to keep the implementation as concise as possible.

## Features
- Portable single header and source file duo written in ANSI C (ISO C90)
- Dual license with either MIT or public domain
- Small implementation
    - Deflate: 525 LoC
    - Inflate: 500 LoC
- Webassembly:
    - Deflate ~3.7 KB (~2.2KB compressed)
    - Inflate ~3.6 KB (~2.2KB compressed)

## Usage:
This file behaves differently depending on what symbols you define
before including it.

Header-File mode:
If you do not define `SINFL_IMPLEMENTATION` before including this file, it
will operate in header only mode. In this mode it declares all used structs
and the API of the library without including the implementation of the library.

Implementation mode:
If you define `SINFL_IMPLEMENTATION` before including this file, it will
compile the implementation. Make sure that you only include
this file implementation in *one* C or C++ file to prevent collisions.

### Benchmark

| Compressor name         | Compression| Decompress.| Compr. size | Ratio |
| ------------------------| -----------| -----------| ----------- | ----- |
| miniz 1.0 -1            |   122 MB/s |   208 MB/s |    48510028 | 48.51 |
| miniz 1.0 -6            |    27 MB/s |   260 MB/s |    36513697 | 36.51 |
| miniz 1.0 -9            |    23 MB/s |   261 MB/s |    36460101 | 36.46 |
| zlib 1.2.11 -1          |    72 MB/s |   307 MB/s |    42298774 | 42.30 |
| zlib 1.2.11 -6          |    24 MB/s |   313 MB/s |    36548921 | 36.55 |
| zlib 1.2.11 -9          |    20 MB/s |   314 MB/s |    36475792 | 36.48 |
| sdefl 1.0 -0            |   127 MB/s |   355 MB/s |    40004116 | 39.88 |
| sdefl 1.0 -1            |   111 MB/s |   413 MB/s |    38940674 | 38.82 |
| sdefl 1.0 -5            |    45 MB/s |   436 MB/s |    36577183 | 36.46 |
| sdefl 1.0 -7            |    38 MB/s |   432 MB/s |    36523781 | 36.41 |
| libdeflate 1.3 -1       |   147 MB/s |   667 MB/s |    39597378 | 39.60 |
| libdeflate 1.3 -6       |    69 MB/s |   689 MB/s |    36648318 | 36.65 |
| libdeflate 1.3 -9       |    13 MB/s |   672 MB/s |    35197141 | 35.20 |
| libdeflate 1.3 -12      |  8.13 MB/s |   670 MB/s |    35100568 | 35.10 |

### Compression
Results on the [Silesia compression corpus](http://sun.aei.polsl.pl/~sdeor/index.php?page=silesia):

| File    |   Original | `sdefl 0`    | `sdefl 5`  | `sdefl 7`   |
| --------| -----------| -------------| ---------- | ------------|
| dickens | 10.192.446 | 4,260,187    |  3,845,261 |   3,833,657 |
| mozilla | 51.220.480 | 20,774,706   | 19,607,009 |  19,565,867 |
| mr      |  9.970.564 | 3,860,531    |  3,673,460 |   3,665,627 |
| nci     | 33.553.445 | 4,030,283    |  3,094,526 |   3,006,075 |
| ooffice |  6.152.192 | 3,320,063    |  3,186,373 |   3,183,815 |
| osdb    | 10.085.684 | 3,919,646    |  3,649,510 |   3,649,477 |
| reymont |  6.627.202 | 2,263,378    |  1,857,588 |   1,827,237 |
| samba   | 21.606.400 | 6,121,797    |  5,462,670 |   5,450,762 |
| sao     |  7.251.944 | 5,612,421    |  5,485,380 |   5,481,765 |
| webster | 41.458.703 | 13,972,648   | 12,059,432 |  11,991,421 |
| xml     |  5.345.280 | 886,620      |    674,009 |     662,141 |
| x-ray   |  8.474.240 | 6,304,655    |  6,244,779 |   6,244,779 |

## License
```
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020-2023 Micha Mettke
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
```
*/
#ifndef SINFL_H_INCLUDED
#define SINFL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define SINFL_PRE_TBL_SIZE 128
#define SINFL_LIT_TBL_SIZE 1334
#define SINFL_OFF_TBL_SIZE 402

struct sinfl {
  const unsigned char *bitptr;
  unsigned long long bitbuf;
  int bitcnt;

  unsigned lits[SINFL_LIT_TBL_SIZE];
  unsigned dsts[SINFL_OFF_TBL_SIZE];
};
extern int sinflate(void *out, int cap, const void *in, int size);
extern int zsinflate(void *out, int cap, const void *in, int size);

#ifdef __cplusplus
}
#endif

#endif /* SINFL_H_INCLUDED */

#ifdef SINFL_IMPLEMENTATION

#include <string.h> /* memcpy, memset */
#include <assert.h> /* assert */

#if defined(__GNUC__) || defined(__clang__)
#define sinfl_likely(x)       __builtin_expect((x),1)
#define sinfl_unlikely(x)     __builtin_expect((x),0)
#else
#define sinfl_likely(x)       (x)
#define sinfl_unlikely(x)     (x)
#endif

#ifndef SINFL_NO_SIMD
#if defined(__x86_64__) || defined(_WIN32) || defined(_WIN64)
  #include <emmintrin.h>
  #define sinfl_char16 __m128i
  #define sinfl_char16_ld(p) _mm_loadu_si128((const __m128i *)(void*)(p))
  #define sinfl_char16_str(d,v)  _mm_storeu_si128((__m128i*)(void*)(d), v)
  #define sinfl_char16_char(c) _mm_set1_epi8(c)
#elif defined(__arm__) || defined(__aarch64__)
  #include <arm_neon.h>
  #define sinfl_char16 uint8x16_t
  #define sinfl_char16_ld(p) vld1q_u8((const unsigned char*)(p))
  #define sinfl_char16_str(d,v) vst1q_u8((unsigned char*)(d), v)
  #define sinfl_char16_char(c) vdupq_n_u8(c)
#else
  #define SINFL_NO_SIMD
#endif
#endif

static int
sinfl_bsr(unsigned n) {
#ifdef _MSC_VER
  _BitScanReverse(&n, n);
  return n;
#elif defined(__GNUC__) || defined(__clang__)
  return 31 - __builtin_clz(n);
#endif
}
static unsigned long long
sinfl_read64(const void *p) {
  unsigned long long n;
  memcpy(&n, p, 8);
  return n;
}
static void
sinfl_copy64(unsigned char **dst, unsigned char **src) {
  unsigned long long n;
  memcpy(&n, *src, 8);
  memcpy(*dst, &n, 8);
  *dst += 8, *src += 8;
}
static unsigned char*
sinfl_write64(unsigned char *dst, unsigned long long w) {
  memcpy(dst, &w, 8);
  return dst + 8;
}
#ifndef SINFL_NO_SIMD
static unsigned char*
sinfl_write128(unsigned char *dst, sinfl_char16 w) {
  sinfl_char16_str(dst, w);
  return dst + 8;
}
static void
sinfl_copy128(unsigned char **dst, unsigned char **src) {
  sinfl_char16 n = sinfl_char16_ld(*src);
  sinfl_char16_str(*dst, n);
  *dst += 16, *src += 16;
}
#endif
static void
sinfl_refill(struct sinfl *s) {
  s->bitbuf |= sinfl_read64(s->bitptr) << s->bitcnt;
  s->bitptr += (63 - s->bitcnt) >> 3;
  s->bitcnt |= 56; /* bitcount in range [56,63] */
}
static int
sinfl_peek(struct sinfl *s, int cnt) {
  assert(cnt >= 0 && cnt <= 56);
  assert(cnt <= s->bitcnt);
  return s->bitbuf & ((1ull << cnt) - 1);
}
static void
sinfl_eat(struct sinfl *s, int cnt) {
  assert(cnt <= s->bitcnt);
  s->bitbuf >>= cnt;
  s->bitcnt -= cnt;
}
static int
sinfl__get(struct sinfl *s, int cnt) {
  int res = sinfl_peek(s, cnt);
  sinfl_eat(s, cnt);
  return res;
}
static int
sinfl_get(struct sinfl *s, int cnt) {
  sinfl_refill(s);
  return sinfl__get(s, cnt);
}
struct sinfl_gen {
  int len;
  int cnt;
  int word;
  short* sorted;
};
static int
sinfl_build_tbl(struct sinfl_gen *gen, unsigned *tbl, int tbl_bits,
                const int *cnt) {
  int tbl_end = 0;
  while (!(gen->cnt = cnt[gen->len])) {
    ++gen->len;
  }
  tbl_end = 1 << gen->len;
  while (gen->len <= tbl_bits) {
    do {unsigned bit = 0;
      tbl[gen->word] = (*gen->sorted++ << 16) | gen->len;
      if (gen->word == tbl_end - 1) {
        for (; gen->len < tbl_bits; gen->len++) {
          memcpy(&tbl[tbl_end], tbl, (size_t)tbl_end * sizeof(tbl[0]));
          tbl_end <<= 1;
        }
        return 1;
      }
      bit = 1 << sinfl_bsr((unsigned)(gen->word ^ (tbl_end - 1)));
      gen->word &= bit - 1;
      gen->word |= bit;
    } while (--gen->cnt);
    do {
      if (++gen->len <= tbl_bits) {
        memcpy(&tbl[tbl_end], tbl, (size_t)tbl_end * sizeof(tbl[0]));
        tbl_end <<= 1;
      }
    } while (!(gen->cnt = cnt[gen->len]));
  }
  return 0;
}
static void
sinfl_build_subtbl(struct sinfl_gen *gen, unsigned *tbl, int tbl_bits,
                   const int *cnt) {
  int sub_bits = 0;
  int sub_start = 0;
  int sub_prefix = -1;
  int tbl_end = 1 << tbl_bits;
  while (1) {
    unsigned entry;
    int bit, stride, i;
    /* start new sub-table */
    if ((gen->word & ((1 << tbl_bits)-1)) != sub_prefix) {
      int used = 0;
      sub_prefix = gen->word & ((1 << tbl_bits)-1);
      sub_start = tbl_end;
      sub_bits = gen->len - tbl_bits;
      used = gen->cnt;
      while (used < (1 << sub_bits)) {
        sub_bits++;
        used = (used << 1) + cnt[tbl_bits + sub_bits];
      }
      tbl_end = sub_start + (1 << sub_bits);
      tbl[sub_prefix] = (sub_start << 16) | 0x10 | (sub_bits & 0xf);
    }
    /* fill sub-table */
    entry = (*gen->sorted << 16) | ((gen->len - tbl_bits) & 0xf);
    gen->sorted++;
    i = sub_start + (gen->word >> tbl_bits);
    stride = 1 << (gen->len - tbl_bits);
    do {
      tbl[i] = entry;
      i += stride;
    } while (i < tbl_end);
    if (gen->word == (1 << gen->len)-1) {
      return;
    }
    bit = 1 << sinfl_bsr(gen->word ^ ((1 << gen->len) - 1));
    gen->word &= bit - 1;
    gen->word |= bit;
    gen->cnt--;
    while (!gen->cnt) {
      gen->cnt = cnt[++gen->len];
    }
  }
}
static void
sinfl_build(unsigned *tbl, unsigned char *lens, int tbl_bits, int maxlen,
            int symcnt) {
  int i, used = 0;
  short sort[288];
  int cnt[16] = {0}, off[16]= {0};
  struct sinfl_gen gen = {0};
  gen.sorted = sort;
  gen.len = 1;

  for (i = 0; i < symcnt; ++i)
    cnt[lens[i]]++;
  off[1] = cnt[0];
  for (i = 1; i < maxlen; ++i) {
    off[i + 1] = off[i] + cnt[i];
    used = (used << 1) + cnt[i];
  }
  used = (used << 1) + cnt[i];
  for (i = 0; i < symcnt; ++i)
    gen.sorted[off[lens[i]]++] = (short)i;
  gen.sorted += off[0];

  if (used < (1 << maxlen)){
    for (i = 0; i < 1 << tbl_bits; ++i)
      tbl[i] = (0 << 16u) | 1;
    return;
  }
  if (!sinfl_build_tbl(&gen, tbl, tbl_bits, cnt)){
    sinfl_build_subtbl(&gen, tbl, tbl_bits, cnt);
  }
}
static int
sinfl_decode(struct sinfl *s, const unsigned *tbl, int bit_len) {
  int idx = sinfl_peek(s, bit_len);
  unsigned key = tbl[idx];
  if (key & 0x10) {
    /* sub-table lookup */
    int len = key & 0x0f;
    sinfl_eat(s, bit_len);
    idx = sinfl_peek(s, len);
    key = tbl[((key >> 16) & 0xffff) + (unsigned)idx];
  }
  sinfl_eat(s, key & 0x0f);
  return (key >> 16) & 0x0fff;
}
static int
sinfl_decompress(unsigned char *out, int cap, const unsigned char *in, int size) {
  static const unsigned char order[] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
  static const short dbase[30+2] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
      257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
  static const unsigned char dbits[30+2] = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,
      10,10,11,11,12,12,13,13,0,0};
  static const short lbase[29+2] = {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,
      43,51,59,67,83,99,115,131,163,195,227,258,0,0};
  static const unsigned char lbits[29+2] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,
      4,4,4,5,5,5,5,0,0,0};

  const unsigned char *oe = out + cap;
  const unsigned char *e = in + size, *o = out;
  enum sinfl_states {hdr,stored,fixed,dyn,blk};
  enum sinfl_states state = hdr;
  struct sinfl s = {0};
  int last = 0;

  s.bitptr = in;
  while (1) {
    switch (state) {
    case hdr: {
      /* block header */
      int type = 0;
      sinfl_refill(&s);
      last = sinfl__get(&s,1);
      type = sinfl__get(&s,2);

      switch (type) {default: return (int)(out-o);
      case 0x00: state = stored; break;
      case 0x01: state = fixed; break;
      case 0x02: state = dyn; break;}
    } break;
    case stored: {
      /* uncompressed block */
      unsigned len, nlen;
      sinfl__get(&s,s.bitcnt & 7);
      len = (unsigned short)sinfl__get(&s,16);
      nlen = (unsigned short)sinfl__get(&s,16);
      s.bitptr -= s.bitcnt / 8;
      s.bitbuf = s.bitcnt = 0;

      if ((unsigned short)len != (unsigned short)~nlen)
        return (int)(out-o);
      if (len > (e - s.bitptr) || !len)
        return (int)(out-o);

      memcpy(out, s.bitptr, (size_t)len);
      s.bitptr += len, out += len;
      if (last) return (int)(out-o);
      state = hdr;
    } break;
    case fixed: {
      /* fixed huffman codes */
      int n; unsigned char lens[288+32];
      for (n = 0; n <= 143; n++) lens[n] = 8;
      for (n = 144; n <= 255; n++) lens[n] = 9;
      for (n = 256; n <= 279; n++) lens[n] = 7;
      for (n = 280; n <= 287; n++) lens[n] = 8;
      for (n = 0; n < 32; n++) lens[288+n] = 5;

      /* build lit/dist tables */
      sinfl_build(s.lits, lens, 10, 15, 288);
      sinfl_build(s.dsts, lens + 288, 8, 15, 32);
      state = blk;
    } break;
    case dyn: {
      /* dynamic huffman codes */
      int n, i;
      unsigned hlens[SINFL_PRE_TBL_SIZE];
      unsigned char nlens[19] = {0}, lens[288+32];

      sinfl_refill(&s);
      {int nlit = 257 + sinfl__get(&s,5);
      int ndist = 1 + sinfl__get(&s,5);
      int nlen = 4 + sinfl__get(&s,4);
      for (n = 0; n < nlen; n++)
        nlens[order[n]] = (unsigned char)sinfl_get(&s,3);
      sinfl_build(hlens, nlens, 7, 7, 19);

      /* decode code lengths */
      for (n = 0; n < nlit + ndist;) {
        int sym = 0;
        sinfl_refill(&s);
        sym = sinfl_decode(&s, hlens, 7);
        switch (sym) {default: lens[n++] = (unsigned char)sym; break;
        case 16: for (i=3+sinfl_get(&s,2);i;i--,n++) lens[n]=lens[n-1]; break;
        case 17: for (i=3+sinfl_get(&s,3);i;i--,n++) lens[n]=0; break;
        case 18: for (i=11+sinfl_get(&s,7);i;i--,n++) lens[n]=0; break;}
      }
      /* build lit/dist tables */
      sinfl_build(s.lits, lens, 10, 15, nlit);
      sinfl_build(s.dsts, lens + nlit, 8, 15, ndist);
      state = blk;}
    } break;
    case blk: {
      /* decompress block */
      while (1) {
        int sym;
        sinfl_refill(&s);
        sym = sinfl_decode(&s, s.lits, 10);
        if (sym < 256) {
          /* literal */
          if (sinfl_unlikely(out >= oe)) {
            return (int)(out-o);
          }
          *out++ = (unsigned char)sym;
          sym = sinfl_decode(&s, s.lits, 10);
          if (sym < 256) {
            *out++ = (unsigned char)sym;
            continue;
          }
        }
        if (sinfl_unlikely(sym == 256)) {
          /* end of block */
          if (last) return (int)(out-o);
          state = hdr;
          break;
        }
        /* match */
        if (sym >= 286) {
          /* length codes 286 and 287 must not appear in compressed data */
          return (int)(out-o);
        }
        sym -= 257;
        {int len = sinfl__get(&s, lbits[sym]) + lbase[sym];
        int dsym = sinfl_decode(&s, s.dsts, 8);
        int offs = sinfl__get(&s, dbits[dsym]) + dbase[dsym];
        unsigned char *dst = out, *src = out - offs;
        if (sinfl_unlikely(offs > (int)(out-o))) {
          return (int)(out-o);
        }
        out = out + len;

#ifndef SINFL_NO_SIMD
        if (sinfl_likely(oe - out >= 16 * 3)) {
          if (offs >= 16) {
            /* simd copy match */
            sinfl_copy128(&dst, &src);
            sinfl_copy128(&dst, &src);
            do sinfl_copy128(&dst, &src);
            while (dst < out);
          } else if (offs >= 8) {
            /* word copy match */
            sinfl_copy64(&dst, &src);
            sinfl_copy64(&dst, &src);
            do sinfl_copy64(&dst, &src);
            while (dst < out);
          } else if (offs == 1) {
            /* rle match copying */
            sinfl_char16 w = sinfl_char16_char(src[0]);
            dst = sinfl_write128(dst, w);
            dst = sinfl_write128(dst, w);
            do dst = sinfl_write128(dst, w);
            while (dst < out);
          } else {
            /* byte copy match */
            *dst++ = *src++;
            *dst++ = *src++;
            do *dst++ = *src++;
            while (dst < out);
          }
        }
#else
        if (sinfl_likely(oe - out >= 3 * 8 - 3)) {
          if (offs >= 8) {
            /* word copy match */
            sinfl_copy64(&dst, &src);
            sinfl_copy64(&dst, &src);
            do sinfl_copy64(&dst, &src);
            while (dst < out);
          } else if (offs == 1) {
            /* rle match copying */
            unsigned int c = src[0];
            unsigned int hw = (c << 24u) | (c << 16u) | (c << 8u) | (unsigned)c;
            unsigned long long w = (unsigned long long)hw << 32llu | hw;
            dst = sinfl_write64(dst, w);
            dst = sinfl_write64(dst, w);
            do dst = sinfl_write64(dst, w);
            while (dst < out);
          } else {
            /* byte copy match */
            *dst++ = *src++;
            *dst++ = *src++;
            do *dst++ = *src++;
            while (dst < out);
          }
        }
#endif
        else {
          *dst++ = *src++;
          *dst++ = *src++;
          do *dst++ = *src++;
          while (dst < out);
        }}
      }
    } break;}
  }
  return (int)(out-o);
}
extern int
sinflate(void *out, int cap, const void *in, int size) {
  return sinfl_decompress((unsigned char*)out, cap, (const unsigned char*)in, size);
}
static unsigned
sinfl_adler32(unsigned adler32, const unsigned char *in, int in_len) {
  const unsigned ADLER_MOD = 65521;
  unsigned s1 = adler32 & 0xffff;
  unsigned s2 = adler32 >> 16;
  unsigned blk_len, i;

  blk_len = in_len % 5552;
  while (in_len) {
    for (i=0; i + 7 < blk_len; i += 8) {
      s1 += in[0]; s2 += s1;
      s1 += in[1]; s2 += s1;
      s1 += in[2]; s2 += s1;
      s1 += in[3]; s2 += s1;
      s1 += in[4]; s2 += s1;
      s1 += in[5]; s2 += s1;
      s1 += in[6]; s2 += s1;
      s1 += in[7]; s2 += s1;
      in += 8;
    }
    for (; i < blk_len; ++i)
      s1 += *in++, s2 += s1;
    s1 %= ADLER_MOD; s2 %= ADLER_MOD;
    in_len -= blk_len;
    blk_len = 5552;
  } return (unsigned)(s2 << 16) + (unsigned)s1;
}
extern int
zsinflate(void *out, int cap, const void *mem, int size) {
  const unsigned char *in = (const unsigned char*)mem;
  if (size >= 6) {
    const unsigned char *eob = in + size - 4;
    int n = sinfl_decompress((unsigned char*)out, cap, in + 2u, size);
    unsigned a = sinfl_adler32(1u, (unsigned char*)out, n);
    unsigned h = eob[0] << 24 | eob[1] << 16 | eob[2] << 8 | eob[3] << 0;
    return a == h ? n : -1;
  } else {
    return -1;
  }
}
#endif

