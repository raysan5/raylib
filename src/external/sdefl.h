/*# Small Deflate
`sdefl` is a small bare bone lossless compression library in ANSI C (ISO C90)
which implements the Deflate (RFC 1951) compressed data format specification standard.
It is mainly tuned to get as much speed and compression ratio from as little code
as needed to keep the implementation as concise as possible.

## Features
- Portable single header and source file duo written in ANSI C (ISO C90)
- Dual license with either MIT or public domain
- Small implementation
    - Deflate: 525 LoC
    - Inflate: 320 LoC
- Webassembly:
    - Deflate ~3.7 KB (~2.2KB compressed)
    - Inflate ~3.6 KB (~2.2KB compressed)

## Usage:
This file behaves differently depending on what symbols you define
before including it.

Header-File mode:
If you do not define `SDEFL_IMPLEMENTATION` before including this file, it
will operate in header only mode. In this mode it declares all used structs
and the API of the library without including the implementation of the library.

Implementation mode:
If you define `SDEFL_IMPLEMENTATION` before including this file, it will
compile the implementation . Make sure that you only include
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
| sdefl 1.0 -0            |   127 MB/s |   371 MB/s |    40004116 | 39.88 |
| sdefl 1.0 -1            |   111 MB/s |   398 MB/s |    38940674 | 38.82 |
| sdefl 1.0 -5            |    45 MB/s |   420 MB/s |    36577183 | 36.46 |
| sdefl 1.0 -7            |    38 MB/s |   423 MB/s |    36523781 | 36.41 |
| libdeflate 1.3 -1       |   147 MB/s |   667 MB/s |    39597378 | 39.60 |
| libdeflate 1.3 -6       |    69 MB/s |   689 MB/s |    36648318 | 36.65 |
| libdeflate 1.3 -9       |    13 MB/s |   672 MB/s |    35197141 | 35.20 |
| libdeflate 1.3 -12      |  8.13 MB/s |   670 MB/s |    35100568 | 35.10 |

### Compression
Results on the [Silesia compression corpus](http://sun.aei.polsl.pl/~sdeor/index.php?page=silesia):

| File    |   Original | `sdefl 0`  	| `sdefl 5` 	| `sdefl 7` |
| :------ | ---------: | -----------------: | ---------: | ----------: |
| dickens | 10.192.446 |  4,260,187|  3,845,261|   3,833,657 |
| mozilla | 51.220.480 | 20,774,706 | 19,607,009 |  19,565,867 |
| mr      |  9.970.564 | 3,860,531 |  3,673,460 |   3,665,627 |
| nci     | 33.553.445 | 4,030,283 |  3,094,526 |   3,006,075 |
| ooffice |  6.152.192 | 3,320,063 |  3,186,373 |   3,183,815 |
| osdb    | 10.085.684 | 3,919,646 |  3,649,510 |   3,649,477 |
| reymont |  6.627.202 | 2,263,378 |  1,857,588 |   1,827,237 |
| samba   | 21.606.400 | 6,121,797 |  5,462,670 |   5,450,762 |
| sao     |  7.251.944 | 5,612,421 |  5,485,380 |   5,481,765 |
| webster | 41.458.703 | 13,972,648 | 12,059,432 |  11,991,421 |
| xml     |  5.345.280 | 886,620|    674,009 |     662,141 |
| x-ray   |  8.474.240 | 6,304,655 |  6,244,779 |   6,244,779 |

## License
```
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Micha Mettke
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
#ifndef SDEFL_H_INCLUDED
#define SDEFL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define SDEFL_MAX_OFF   (1 << 15)
#define SDEFL_WIN_SIZ   SDEFL_MAX_OFF
#define SDEFL_WIN_MSK   (SDEFL_WIN_SIZ-1)

#define SDEFL_HASH_BITS 15
#define SDEFL_HASH_SIZ  (1 << SDEFL_HASH_BITS)
#define SDEFL_HASH_MSK  (SDEFL_HASH_SIZ-1)

#define SDEFL_MIN_MATCH 4
#define SDEFL_BLK_MAX   (256*1024)
#define SDEFL_SEQ_SIZ   ((SDEFL_BLK_MAX + SDEFL_MIN_MATCH)/SDEFL_MIN_MATCH)

#define SDEFL_SYM_MAX   (288)
#define SDEFL_OFF_MAX   (32)
#define SDEFL_PRE_MAX   (19)

#define SDEFL_LVL_MIN   0
#define SDEFL_LVL_DEF   5
#define SDEFL_LVL_MAX   8

struct sdefl_freq {
  unsigned lit[SDEFL_SYM_MAX];
  unsigned off[SDEFL_OFF_MAX];
};
struct sdefl_code_words {
  unsigned lit[SDEFL_SYM_MAX];
  unsigned off[SDEFL_OFF_MAX];
};
struct sdefl_lens {
  unsigned char lit[SDEFL_SYM_MAX];
  unsigned char off[SDEFL_OFF_MAX];
};
struct sdefl_codes {
  struct sdefl_code_words word;
  struct sdefl_lens len;
};
struct sdefl_seqt {
  int off, len;
};
struct sdefl {
  int bits, bitcnt;
  int tbl[SDEFL_HASH_SIZ];
  int prv[SDEFL_WIN_SIZ];

  int seq_cnt;
  struct sdefl_seqt seq[SDEFL_SEQ_SIZ];
  struct sdefl_freq freq;
  struct sdefl_codes cod;
};
extern int sdefl_bound(int in_len);
extern int sdeflate(struct sdefl *s, void *o, const void *i, int n, int lvl);
extern int zsdeflate(struct sdefl *s, void *o, const void *i, int n, int lvl);

#ifdef __cplusplus
}
#endif

#endif /* SDEFL_H_INCLUDED */

#ifdef SDEFL_IMPLEMENTATION

#include <assert.h> /* assert */
#include <string.h> /* memcpy */
#include <limits.h> /* CHAR_BIT */

#define SDEFL_NIL               (-1)
#define SDEFL_MAX_MATCH         258
#define SDEFL_MAX_CODE_LEN      (15)
#define SDEFL_SYM_BITS          (10u)
#define SDEFL_SYM_MSK           ((1u << SDEFL_SYM_BITS)-1u)
#define SDEFL_LIT_LEN_CODES     (14)
#define SDEFL_OFF_CODES         (15)
#define SDEFL_PRE_CODES         (7)
#define SDEFL_CNT_NUM(n)        ((((n)+3u/4u)+3u)&~3u)
#define SDEFL_EOB               (256)

#define sdefl_npow2(n) (1 << (sdefl_ilog2((n)-1) + 1))

static int
sdefl_ilog2(int n) {
  if (!n) return 0;
#ifdef _MSC_VER
  unsigned long msbp = 0;
  _BitScanReverse(&msbp, (unsigned long)n);
  return (int)msbp;
#elif defined(__GNUC__) || defined(__clang__)
  return (int)sizeof(unsigned long) * CHAR_BIT - 1 - __builtin_clzl((unsigned long)n);
#else
  #define lt(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
  static const char tbl[256] = {
    0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,lt(4), lt(5), lt(5), lt(6), lt(6), lt(6), lt(6),
    lt(7), lt(7), lt(7), lt(7), lt(7), lt(7), lt(7), lt(7)};
  int tt, t;
  if ((tt = (n >> 16))) {
    return (t = (tt >> 8)) ? 24 + tbl[t] : 16 + tbl[tt];
  } else {
    return (t = (n >> 8)) ? 8 + tbl[t] : tbl[n];
  }
  #undef lt
#endif
}
static unsigned
sdefl_uload32(const void *p) {
  /* hopefully will be optimized to an unaligned read */
  unsigned n = 0;
  memcpy(&n, p, sizeof(n));
  return n;
}
static unsigned
sdefl_hash32(const void *p) {
  unsigned n = sdefl_uload32(p);
  return (n * 0x9E377989) >> (32 - SDEFL_HASH_BITS);
}
static void
sdefl_put(unsigned char **dst, struct sdefl *s, int code, int bitcnt) {
  s->bits |= (code << s->bitcnt);
  s->bitcnt += bitcnt;
  while (s->bitcnt >= 8) {
    unsigned char *tar = *dst;
    *tar = (unsigned char)(s->bits & 0xFF);
    s->bits >>= 8;
    s->bitcnt -= 8;
    *dst = *dst + 1;
  }
}
static void
sdefl_heap_sub(unsigned A[], unsigned len, unsigned sub) {
  unsigned c, p = sub;
  unsigned v = A[sub];
  while ((c = p << 1) <= len) {
    if (c < len && A[c + 1] > A[c]) c++;
    if (v >= A[c]) break;
    A[p] = A[c], p = c;
  }
  A[p] = v;
}
static void
sdefl_heap_array(unsigned *A, unsigned len) {
  unsigned sub;
  for (sub = len >> 1; sub >= 1; sub--)
    sdefl_heap_sub(A, len, sub);
}
static void
sdefl_heap_sort(unsigned *A, unsigned n) {
  A--;
  sdefl_heap_array(A, n);
  while (n >= 2) {
    unsigned tmp = A[n];
    A[n--] = A[1];
    A[1] = tmp;
    sdefl_heap_sub(A, n, 1);
  }
}
static unsigned
sdefl_sort_sym(unsigned sym_cnt, unsigned *freqs,
               unsigned char *lens, unsigned *sym_out) {
  unsigned cnts[SDEFL_CNT_NUM(SDEFL_SYM_MAX)] = {0};
  unsigned cnt_num = SDEFL_CNT_NUM(sym_cnt);
  unsigned used_sym = 0;
  unsigned sym, i;
  for (sym = 0; sym < sym_cnt; sym++)
    cnts[freqs[sym] < cnt_num-1 ? freqs[sym]: cnt_num-1]++;
  for (i = 1; i < cnt_num; i++) {
    unsigned cnt = cnts[i];
    cnts[i] = used_sym;
    used_sym += cnt;
  }
  for (sym = 0; sym < sym_cnt; sym++) {
    unsigned freq = freqs[sym];
    if (freq) {
        unsigned idx = freq < cnt_num-1 ? freq : cnt_num-1;
        sym_out[cnts[idx]++] = sym | (freq << SDEFL_SYM_BITS);
    } else lens[sym] = 0;
  }
  sdefl_heap_sort(sym_out + cnts[cnt_num-2], cnts[cnt_num-1] - cnts[cnt_num-2]);
  return used_sym;
}
static void
sdefl_build_tree(unsigned *A, unsigned sym_cnt) {
  unsigned i = 0, b = 0, e = 0;
  do {
    unsigned m, n, freq_shift;
    if (i != sym_cnt && (b == e || (A[i] >> SDEFL_SYM_BITS) <= (A[b] >> SDEFL_SYM_BITS)))
      m = i++;
    else m = b++;
    if (i != sym_cnt && (b == e || (A[i] >> SDEFL_SYM_BITS) <= (A[b] >> SDEFL_SYM_BITS)))
      n = i++;
    else n = b++;

    freq_shift = (A[m] & ~SDEFL_SYM_MSK) + (A[n] & ~SDEFL_SYM_MSK);
    A[m] = (A[m] & SDEFL_SYM_MSK) | (e << SDEFL_SYM_BITS);
    A[n] = (A[n] & SDEFL_SYM_MSK) | (e << SDEFL_SYM_BITS);
    A[e] = (A[e] & SDEFL_SYM_MSK) | freq_shift;
  } while (sym_cnt - ++e > 1);
}
static void
sdefl_gen_len_cnt(unsigned *A, unsigned root, unsigned *len_cnt,
                  unsigned max_code_len) {
  int n;
  unsigned i;
  for (i = 0; i <= max_code_len; i++)
    len_cnt[i] = 0;
  len_cnt[1] = 2;

  A[root] &= SDEFL_SYM_MSK;
  for (n = (int)root - 1; n >= 0; n--) {
    unsigned p = A[n] >> SDEFL_SYM_BITS;
    unsigned pdepth = A[p] >> SDEFL_SYM_BITS;
    unsigned depth = pdepth + 1;
    unsigned len = depth;

    A[n] = (A[n] & SDEFL_SYM_MSK) | (depth << SDEFL_SYM_BITS);
    if (len >= max_code_len) {
      len = max_code_len;
      do len--; while (!len_cnt[len]);
    }
    len_cnt[len]--;
    len_cnt[len+1] += 2;
  }
}
static void
sdefl_gen_codes(unsigned *A, unsigned char *lens, const unsigned *len_cnt,
                unsigned max_code_word_len, unsigned sym_cnt) {
  unsigned i, sym, len, nxt[SDEFL_MAX_CODE_LEN + 1];
  for (i = 0, len = max_code_word_len; len >= 1; len--) {
    unsigned cnt = len_cnt[len];
    while (cnt--) lens[A[i++] & SDEFL_SYM_MSK] = (unsigned char)len;
  }
  nxt[0] = nxt[1] = 0;
  for (len = 2; len <= max_code_word_len; len++)
    nxt[len] = (nxt[len-1] + len_cnt[len-1]) << 1;
  for (sym = 0; sym < sym_cnt; sym++)
    A[sym] = nxt[lens[sym]]++;
}
static unsigned
sdefl_rev(unsigned c, unsigned char n) {
  c = ((c & 0x5555) << 1) | ((c & 0xAAAA) >> 1);
  c = ((c & 0x3333) << 2) | ((c & 0xCCCC) >> 2);
  c = ((c & 0x0F0F) << 4) | ((c & 0xF0F0) >> 4);
  c = ((c & 0x00FF) << 8) | ((c & 0xFF00) >> 8);
  return c >> (16-n);
}
static void
sdefl_huff(unsigned char *lens, unsigned *codes, unsigned *freqs,
           unsigned num_syms, unsigned max_code_len) {
  unsigned c, *A = codes;
  unsigned len_cnt[SDEFL_MAX_CODE_LEN + 1];
  unsigned used_syms = sdefl_sort_sym(num_syms, freqs, lens, A);
  if (!used_syms) return;
  if (used_syms == 1) {
    unsigned s = A[0] & SDEFL_SYM_MSK;
    unsigned i = s ? s : 1;
    codes[0] = 0, lens[0] = 1;
    codes[i] = 1, lens[i] = 1;
    return;
  }
  sdefl_build_tree(A, used_syms);
  sdefl_gen_len_cnt(A, used_syms-2, len_cnt, max_code_len);
  sdefl_gen_codes(A, lens, len_cnt, max_code_len, num_syms);
  for (c = 0; c < num_syms; c++) {
    codes[c] = sdefl_rev(codes[c], lens[c]);
  }
}
struct sdefl_symcnt {
  int items;
  int lit;
  int off;
};
static void
sdefl_precode(struct sdefl_symcnt *cnt, unsigned *freqs, unsigned *items,
              const unsigned char *litlen, const unsigned char *offlen) {
  unsigned *at = items;
  unsigned run_start = 0;

  unsigned total = 0;
  unsigned char lens[SDEFL_SYM_MAX + SDEFL_OFF_MAX];
  for (cnt->lit = SDEFL_SYM_MAX; cnt->lit > 257; cnt->lit--)
    if (litlen[cnt->lit - 1]) break;
  for (cnt->off = SDEFL_OFF_MAX; cnt->off > 1; cnt->off--)
    if (offlen[cnt->off - 1]) break;

  total = (unsigned)(cnt->lit + cnt->off);
  memcpy(lens, litlen, sizeof(unsigned char) * (size_t)cnt->lit);
  memcpy(lens + cnt->lit, offlen, sizeof(unsigned char) * (size_t)cnt->off);
  do {
    unsigned len = lens[run_start];
    unsigned run_end = run_start;
    do run_end++; while (run_end != total && len == lens[run_end]);
    if (!len) {
      while ((run_end - run_start) >= 11) {
        unsigned n = (run_end - run_start) - 11;
        unsigned xbits =  n < 0x7f ? n : 0x7f;
        freqs[18]++;
        *at++ = 18u | (xbits << 5u);
        run_start += 11 + xbits;
      }
      if ((run_end - run_start) >= 3) {
        unsigned n = (run_end - run_start) - 3;
        unsigned xbits =  n < 0x7 ? n : 0x7;
        freqs[17]++;
        *at++ = 17u | (xbits << 5u);
        run_start += 3 + xbits;
      }
    } else if ((run_end - run_start) >= 4) {
      freqs[len]++;
      *at++ = len;
      run_start++;
      do {
        unsigned xbits = (run_end - run_start) - 3;
        xbits = xbits < 0x03 ? xbits : 0x03;
        *at++ = 16 | (xbits << 5);
        run_start += 3 + xbits;
        freqs[16]++;
      } while ((run_end - run_start) >= 3);
    }
    while (run_start != run_end) {
      freqs[len]++;
      *at++ = len;
      run_start++;
    }
  } while (run_start != total);
  cnt->items = (int)(at - items);
}
struct sdefl_match_codes {
  int ls, lc;
  int dc, dx;
};
static void
sdefl_match_codes(struct sdefl_match_codes *cod, int dist, int len) {
  static const short dxmax[] = {0,6,12,24,48,96,192,384,768,1536,3072,6144,12288,24576};
  static const unsigned char lslot[258+1] = {
    0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 12,
    12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 16,
    16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18,
    18, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    27, 27, 28
  };
  cod->ls = lslot[len];
  cod->lc = 257 + cod->ls;
  cod->dx = sdefl_ilog2(sdefl_npow2(dist) >> 2);
  cod->dc = cod->dx ? ((cod->dx + 1) << 1) + (dist > dxmax[cod->dx]) : dist-1;
}
static void
sdefl_match(unsigned char **dst, struct sdefl *s, int dist, int len) {
  static const char lxn[] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
  static const short lmin[] = {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,
      51,59,67,83,99,115,131,163,195,227,258};
  static const short dmin[] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,
      385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};

  struct sdefl_match_codes cod;
  sdefl_match_codes(&cod, dist, len);
  sdefl_put(dst, s, (int)s->cod.word.lit[cod.lc], s->cod.len.lit[cod.lc]);
  sdefl_put(dst, s, len - lmin[cod.ls], lxn[cod.ls]);
  sdefl_put(dst, s, (int)s->cod.word.off[cod.dc], s->cod.len.off[cod.dc]);
  sdefl_put(dst, s, dist - dmin[cod.dc], cod.dx);
}
static void
sdefl_flush(unsigned char **dst, struct sdefl *s, int is_last,
            const unsigned char *in) {
  int j, i = 0, item_cnt = 0;
  struct sdefl_symcnt symcnt = {0};
  unsigned codes[SDEFL_PRE_MAX];
  unsigned char lens[SDEFL_PRE_MAX];
  unsigned freqs[SDEFL_PRE_MAX] = {0};
  unsigned items[SDEFL_SYM_MAX + SDEFL_OFF_MAX];
  static const unsigned char perm[SDEFL_PRE_MAX] = {16,17,18,0,8,7,9,6,10,5,11,
      4,12,3,13,2,14,1,15};

  /* huffman codes */
  s->freq.lit[SDEFL_EOB]++;
  sdefl_huff(s->cod.len.lit, s->cod.word.lit, s->freq.lit, SDEFL_SYM_MAX, SDEFL_LIT_LEN_CODES);
  sdefl_huff(s->cod.len.off, s->cod.word.off, s->freq.off, SDEFL_OFF_MAX, SDEFL_OFF_CODES);
  sdefl_precode(&symcnt, freqs, items, s->cod.len.lit, s->cod.len.off);
  sdefl_huff(lens, codes, freqs, SDEFL_PRE_MAX, SDEFL_PRE_CODES);
  for (item_cnt = SDEFL_PRE_MAX; item_cnt > 4; item_cnt--) {
    if (lens[perm[item_cnt - 1]]) break;
  }
  /* block header */
  sdefl_put(dst, s, is_last ? 0x01 : 0x00, 1); /* block */
  sdefl_put(dst, s, 0x02, 2); /* dynamic huffman */
  sdefl_put(dst, s, symcnt.lit - 257, 5);
  sdefl_put(dst, s, symcnt.off - 1, 5);
  sdefl_put(dst, s, item_cnt - 4, 4);
  for (i = 0; i < item_cnt; ++i)
    sdefl_put(dst, s, lens[perm[i]], 3);
  for (i = 0; i < symcnt.items; ++i) {
    unsigned sym = items[i] & 0x1F;
    sdefl_put(dst, s, (int)codes[sym], lens[sym]);
    if (sym < 16) continue;
    if (sym == 16) sdefl_put(dst, s, items[i] >> 5, 2);
    else if(sym == 17) sdefl_put(dst, s, items[i] >> 5, 3);
    else sdefl_put(dst, s, items[i] >> 5, 7);
  }
  /* block sequences */
  for (i = 0; i < s->seq_cnt; ++i) {
    if (s->seq[i].off >= 0)
      for (j = 0; j < s->seq[i].len; ++j) {
        int c = in[s->seq[i].off + j];
        sdefl_put(dst, s, (int)s->cod.word.lit[c], s->cod.len.lit[c]);
      }
    else sdefl_match(dst, s, -s->seq[i].off, s->seq[i].len);
  }
  sdefl_put(dst, s, (int)(s)->cod.word.lit[SDEFL_EOB], (s)->cod.len.lit[SDEFL_EOB]);
  memset(&s->freq, 0, sizeof(s->freq));
  s->seq_cnt = 0;
}
static void
sdefl_seq(struct sdefl *s, int off, int len) {
  assert(s->seq_cnt + 2 < SDEFL_SEQ_SIZ);
  s->seq[s->seq_cnt].off = off;
  s->seq[s->seq_cnt].len = len;
  s->seq_cnt++;
}
static void
sdefl_reg_match(struct sdefl *s, int off, int len) {
  struct sdefl_match_codes cod;
  sdefl_match_codes(&cod, off, len);
  s->freq.lit[cod.lc]++;
  s->freq.off[cod.dc]++;
}
struct sdefl_match {
  int off;
  int len;
};
static void
sdefl_fnd(struct sdefl_match *m, const struct sdefl *s,
          int chain_len, int max_match, const unsigned char *in, int p) {
  int i = s->tbl[sdefl_hash32(&in[p])];
  int limit = ((p-SDEFL_WIN_SIZ)<SDEFL_NIL)?SDEFL_NIL:(p-SDEFL_WIN_SIZ);
  while (i > limit) {
    if (in[i+m->len] == in[p+m->len] &&
        (sdefl_uload32(&in[i]) == sdefl_uload32(&in[p]))){
      int n = SDEFL_MIN_MATCH;
      while (n < max_match && in[i+n] == in[p+n]) n++;
      if (n > m->len) {
        m->len = n, m->off = p - i;
        if (n == max_match) break;
      }
    }
    if (!(--chain_len)) break;
    i = s->prv[i&SDEFL_WIN_MSK];
  }
}
static int
sdefl_compr(struct sdefl *s, unsigned char *out, const unsigned char *in,
            int in_len, int lvl) {
  unsigned char *q = out;
  static const unsigned char pref[] = {8,10,14,24,30,48,65,96,130};
  int max_chain = (lvl < 8) ? (1 << (lvl + 1)): (1 << 13);
  int n, i = 0, litlen = 0;
  for (n = 0; n < SDEFL_HASH_SIZ; ++n) {
    s->tbl[n] = SDEFL_NIL;
  }
  do {int blk_end = i + SDEFL_BLK_MAX < in_len ? i + SDEFL_BLK_MAX : in_len;
    while (i < blk_end) {
      struct sdefl_match m = {0};
      int max_match = ((in_len-i)>SDEFL_MAX_MATCH) ? SDEFL_MAX_MATCH:(in_len-i);
      int nice_match = pref[lvl] < max_match ? pref[lvl] : max_match;
      int run = 1, inc = 1, run_inc;
      if (max_match > SDEFL_MIN_MATCH) {
        sdefl_fnd(&m, s, max_chain, max_match, in, i);
      }
      if (lvl >= 5 && m.len >= SDEFL_MIN_MATCH && m.len < nice_match){
        struct sdefl_match m2 = {0};
        sdefl_fnd(&m2, s, max_chain, m.len+1, in, i+1);
        m.len = (m2.len > m.len) ? 0 : m.len;
      }
      if (m.len >= SDEFL_MIN_MATCH) {
        if (litlen) {
          sdefl_seq(s, i - litlen, litlen);
          litlen = 0;
        }
        sdefl_seq(s, -m.off, m.len);
        sdefl_reg_match(s, m.off, m.len);
        if (lvl < 2 && m.len >= nice_match) {
          inc = m.len;
        } else {
          run = m.len;
        }
      } else {
        s->freq.lit[in[i]]++;
        litlen++;
      }
      run_inc = run * inc;
      if (in_len - (i + run_inc) > SDEFL_MIN_MATCH) {
        while (run-- > 0) {
          unsigned h = sdefl_hash32(&in[i]);
          s->prv[i&SDEFL_WIN_MSK] = s->tbl[h];
          s->tbl[h] = i, i += inc;
        }
      } else {
        i += run_inc;
      }
    }
    if (litlen) {
      sdefl_seq(s, i - litlen, litlen);
      litlen = 0;
    }
    sdefl_flush(&q, s, blk_end == in_len, in);
  } while (i < in_len);

  if (s->bitcnt)
    sdefl_put(&q, s, 0x00, 8 - s->bitcnt);
  return (int)(q - out);
}
extern int
sdeflate(struct sdefl *s, void *out, const void *in, int n, int lvl) {
  s->bits = s->bitcnt = 0;
  return sdefl_compr(s, (unsigned char*)out, (const unsigned char*)in, n, lvl);
}
static unsigned
sdefl_adler32(unsigned adler32, const unsigned char *in, int in_len) {
  #define SDEFL_ADLER_INIT (1)
  const unsigned ADLER_MOD = 65521;
  unsigned s1 = adler32 & 0xffff;
  unsigned s2 = adler32 >> 16;
  unsigned blk_len, i;

  blk_len = in_len % 5552;
  while (in_len) {
    for (i = 0; i + 7 < blk_len; i += 8) {
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
    for (; i < blk_len; ++i) {
      s1 += *in++, s2 += s1;
    }
    s1 %= ADLER_MOD;
    s2 %= ADLER_MOD;
    in_len -= blk_len;
    blk_len = 5552;
  }
  return (unsigned)(s2 << 16) + (unsigned)s1;
}
extern int
zsdeflate(struct sdefl *s, void *out, const void *in, int n, int lvl) {
  int p = 0;
  unsigned a = 0;
  unsigned char *q = (unsigned char*)out;

  s->bits = s->bitcnt = 0;
  sdefl_put(&q, s, 0x78, 8); /* deflate, 32k window */
  sdefl_put(&q, s, 0x01, 8); /* fast compression */
  q += sdefl_compr(s, q, (const unsigned char*)in, n, lvl);

  /* append adler checksum */
  a = sdefl_adler32(SDEFL_ADLER_INIT, (const unsigned char*)in, n);
  for (p = 0; p < 4; ++p) {
    sdefl_put(&q, s, (a >> 24) & 0xFF, 8);
    a <<= 8;
  }
  return (int)(q - (unsigned char*)out);
}
extern int
sdefl_bound(int len) {
  int a = 128 + (len * 110) / 100;
  int b = 128 + len + ((len / (31 * 1024)) + 1) * 5;
  return (a > b) ? a : b;
}
#endif /* SDEFL_IMPLEMENTATION */

