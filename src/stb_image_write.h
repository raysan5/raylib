/* stb_image_write - v0.98 - public domain - http://nothings.org/stb/stb_image_write.h
   writes out PNG/BMP/TGA images to C stdio - Sean Barrett 2010
                            no warranty implied; use at your own risk


   Before #including,

       #define STB_IMAGE_WRITE_IMPLEMENTATION

   in the file that you want to have the implementation.

   Will probably not work correctly with strict-aliasing optimizations.

ABOUT:

   This header file is a library for writing images to C stdio. It could be
   adapted to write to memory or a general streaming interface; let me know.

   The PNG output is not optimal; it is 20-50% larger than the file
   written by a decent optimizing implementation. This library is designed
   for source code compactness and simplicitly, not optimal image file size
   or run-time performance.

BUILDING:

   You can #define STBIW_ASSERT(x) before the #include to avoid using assert.h.
   You can #define STBIW_MALLOC(), STBIW_REALLOC(), and STBIW_FREE() to replace
   malloc,realloc,free.
   You can define STBIW_MEMMOVE() to replace memmove()

USAGE:

   There are four functions, one for each image file format:

     int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
     int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_hdr(char const *filename, int w, int h, int comp, const void *data);

   Each function returns 0 on failure and non-0 on success.

   The functions create an image file defined by the parameters. The image
   is a rectangle of pixels stored from left-to-right, top-to-bottom.
   Each pixel contains 'comp' channels of data stored interleaved with 8-bits
   per channel, in the following order: 1=Y, 2=YA, 3=RGB, 4=RGBA. (Y is
   monochrome color.) The rectangle is 'w' pixels wide and 'h' pixels tall.
   The *data pointer points to the first byte of the top-left-most pixel.
   For PNG, "stride_in_bytes" is the distance in bytes from the first byte of
   a row of pixels to the first byte of the next row of pixels.

   PNG creates output files with the same number of components as the input.
   The BMP format expands Y to RGB in the file format and does not
   output alpha.

   PNG supports writing rectangles of data even when the bytes storing rows of
   data are not consecutive in memory (e.g. sub-rectangles of a larger image),
   by supplying the stride between the beginning of adjacent rows. The other
   formats do not. (Thus you cannot write a native-format BMP through the BMP
   writer, both because it is in BGR order and because it may have padding
   at the end of the line.)

   HDR expects linear float data. Since the format is always 32-bit rgb(e)
   data, alpha (if provided) is discarded, and for monochrome data it is
   replicated across all three channels.

CREDITS:

   PNG/BMP/TGA
      Sean Barrett
   HDR
      Baldur Karlsson
   TGA monochrome:
      Jean-Sebastien Guay
   misc enhancements:
      Tim Kelsey
   bugfixes:
      github:Chribba
*/

#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

extern int stbi_write_png(char const *filename, int w, int h, int comp, const void  *data, int stride_in_bytes);
extern int stbi_write_bmp(char const *filename, int w, int h, int comp, const void  *data);
extern int stbi_write_tga(char const *filename, int w, int h, int comp, const void  *data);
extern int stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);

#ifdef __cplusplus
}
#endif

#endif//INCLUDE_STB_IMAGE_WRITE_H

#ifdef STB_IMAGE_WRITE_IMPLEMENTATION

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#if defined(STBIW_MALLOC) && defined(STBIW_FREE) && defined(STBIW_REALLOC)
// ok
#elif !defined(STBIW_MALLOC) && !defined(STBIW_FREE) && !defined(STBIW_REALLOC)
// ok
#else
#error "Must define all or none of STBIW_MALLOC, STBIW_FREE, and STBIW_REALLOC."
#endif

#ifndef STBIW_MALLOC
#define STBIW_MALLOC(sz)    malloc(sz)
#define STBIW_REALLOC(p,sz) realloc(p,sz)
#define STBIW_FREE(p)       free(p)
#endif
#ifndef STBIW_MEMMOVE
#define STBIW_MEMMOVE(a,b,sz) memmove(a,b,sz)
#endif


#ifndef STBIW_ASSERT
#include <assert.h>
#define STBIW_ASSERT(x) assert(x)
#endif

typedef unsigned int stbiw_uint32;
typedef int stb_image_write_test[sizeof(stbiw_uint32)==4 ? 1 : -1];

static void writefv(FILE *f, const char *fmt, va_list v)
{
   while (*fmt) {
      switch (*fmt++) {
         case ' ': break;
         case '1': { unsigned char x = (unsigned char) va_arg(v, int); fputc(x,f); break; }
         case '2': { int x = va_arg(v,int); unsigned char b[2];
                     b[0] = (unsigned char) x; b[1] = (unsigned char) (x>>8);
                     fwrite(b,2,1,f); break; }
         case '4': { stbiw_uint32 x = va_arg(v,int); unsigned char b[4];
                     b[0]=(unsigned char)x; b[1]=(unsigned char)(x>>8);
                     b[2]=(unsigned char)(x>>16); b[3]=(unsigned char)(x>>24);
                     fwrite(b,4,1,f); break; }
         default:
            STBIW_ASSERT(0);
            return;
      }
   }
}

static void write3(FILE *f, unsigned char a, unsigned char b, unsigned char c)
{
   unsigned char arr[3];
   arr[0] = a, arr[1] = b, arr[2] = c;
   fwrite(arr, 3, 1, f);
}

static void write_pixels(FILE *f, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad, int expand_mono)
{
   unsigned char bg[3] = { 255, 0, 255}, px[3];
   stbiw_uint32 zero = 0;
   int i,j,k, j_end;

   if (y <= 0)
      return;

   if (vdir < 0)
      j_end = -1, j = y-1;
   else
      j_end =  y, j = 0;

   for (; j != j_end; j += vdir) {
      for (i=0; i < x; ++i) {
         unsigned char *d = (unsigned char *) data + (j*x+i)*comp;
         if (write_alpha < 0)
            fwrite(&d[comp-1], 1, 1, f);
         switch (comp) {
            case 1: fwrite(d, 1, 1, f);
                    break;
            case 2: if (expand_mono)
                       write3(f, d[0],d[0],d[0]); // monochrome bmp
                    else
                       fwrite(d, 1, 1, f);  // monochrome TGA
                    break;
            case 4:
               if (!write_alpha) {
                  // composite against pink background
                  for (k=0; k < 3; ++k)
                     px[k] = bg[k] + ((d[k] - bg[k]) * d[3])/255;
                  write3(f, px[1-rgb_dir],px[1],px[1+rgb_dir]);
                  break;
               }
               /* FALLTHROUGH */
            case 3:
               write3(f, d[1-rgb_dir],d[1],d[1+rgb_dir]);
               break;
         }
         if (write_alpha > 0)
            fwrite(&d[comp-1], 1, 1, f);
      }
      fwrite(&zero,scanline_pad,1,f);
   }
}

static int outfile(char const *filename, int rgb_dir, int vdir, int x, int y, int comp, int expand_mono, void *data, int alpha, int pad, const char *fmt, ...)
{
   FILE *f;
   if (y < 0 || x < 0) return 0;
   f = fopen(filename, "wb");
   if (f) {
      va_list v;
      va_start(v, fmt);
      writefv(f, fmt, v);
      va_end(v);
      write_pixels(f,rgb_dir,vdir,x,y,comp,data,alpha,pad,expand_mono);
      fclose(f);
   }
   return f != NULL;
}

int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data)
{
   int pad = (-x*3) & 3;
   return outfile(filename,-1,-1,x,y,comp,1,(void *) data,0,pad,
           "11 4 22 4" "4 44 22 444444",
           'B', 'M', 14+40+(x*3+pad)*y, 0,0, 14+40,  // file header
            40, x,y, 1,24, 0,0,0,0,0,0);             // bitmap header
}

int stbi_write_tga(char const *filename, int x, int y, int comp, const void *data)
{
   int has_alpha = (comp == 2 || comp == 4);
   int colorbytes = has_alpha ? comp-1 : comp;
   int format = colorbytes < 2 ? 3 : 2; // 3 color channels (RGB/RGBA) = 2, 1 color channel (Y/YA) = 3
   return outfile(filename, -1,-1, x, y, comp, 0, (void *) data, has_alpha, 0,
                  "111 221 2222 11", 0,0,format, 0,0,0, 0,0,x,y, (colorbytes+has_alpha)*8, has_alpha*8);
}

// *************************************************************************************************
// Radiance RGBE HDR writer
// by Baldur Karlsson
#define stbiw__max(a, b)  ((a) > (b) ? (a) : (b))

void stbiw__linear_to_rgbe(unsigned char *rgbe, float *linear)
{
   int exponent;
   float maxcomp = stbiw__max(linear[0], stbiw__max(linear[1], linear[2]));

   if (maxcomp < 1e-32) {
      rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
   } else {
      float normalize = (float) frexp(maxcomp, &exponent) * 256.0f/maxcomp;

      rgbe[0] = (unsigned char)(linear[0] * normalize);
      rgbe[1] = (unsigned char)(linear[1] * normalize);
      rgbe[2] = (unsigned char)(linear[2] * normalize);
      rgbe[3] = (unsigned char)(exponent + 128);
   }
}

void stbiw__write_run_data(FILE *f, int length, unsigned char databyte)
{
   unsigned char lengthbyte = (unsigned char) (length+128);
   STBIW_ASSERT(length+128 <= 255);
   fwrite(&lengthbyte, 1, 1, f);
   fwrite(&databyte, 1, 1, f);
}

void stbiw__write_dump_data(FILE *f, int length, unsigned char *data)
{
   unsigned char lengthbyte = (unsigned char )(length & 0xff);
   STBIW_ASSERT(length <= 128); // inconsistent with spec but consistent with official code
   fwrite(&lengthbyte, 1, 1, f);
   fwrite(data, length, 1, f);
}

void stbiw__write_hdr_scanline(FILE *f, int width, int comp, unsigned char *scratch, const float *scanline)
{
   unsigned char scanlineheader[4] = { 2, 2, 0, 0 };
   unsigned char rgbe[4];
   float linear[3];
   int x;

   scanlineheader[2] = (width&0xff00)>>8;
   scanlineheader[3] = (width&0x00ff);

   /* skip RLE for images too small or large */
   if (width < 8 || width >= 32768) {
      for (x=0; x < width; x++) {
         switch (comp) {
            case 4: /* fallthrough */
            case 3: linear[2] = scanline[x*comp + 2];
                    linear[1] = scanline[x*comp + 1];
                    linear[0] = scanline[x*comp + 0];
                    break;
            case 2: /* fallthrough */
            case 1: linear[0] = linear[1] = linear[2] = scanline[x*comp + 0];
                    break;
         }
         stbiw__linear_to_rgbe(rgbe, linear);
         fwrite(rgbe, 4, 1, f);
      }
   } else {
      int c,r;
      /* encode into scratch buffer */
      for (x=0; x < width; x++) {
         switch(comp) {
            case 4: /* fallthrough */
            case 3: linear[2] = scanline[x*comp + 2];
                    linear[1] = scanline[x*comp + 1];
                    linear[0] = scanline[x*comp + 0];
                    break;
            case 2: /* fallthrough */
            case 1: linear[0] = linear[1] = linear[2] = scanline[x*comp + 0];
                    break;
         }
         stbiw__linear_to_rgbe(rgbe, linear);
         scratch[x + width*0] = rgbe[0];
         scratch[x + width*1] = rgbe[1];
         scratch[x + width*2] = rgbe[2];
         scratch[x + width*3] = rgbe[3];
      }

      fwrite(scanlineheader, 4, 1, f);

      /* RLE each component separately */
      for (c=0; c < 4; c++) {
         unsigned char *comp = &scratch[width*c];

         x = 0;
         while (x < width) {
            // find first run
            r = x;
            while (r+2 < width) {
               if (comp[r] == comp[r+1] && comp[r] == comp[r+2])
                  break;
               ++r;
            }
            if (r+2 >= width)
               r = width;
            // dump up to first run
            while (x < r) {
               int len = r-x;
               if (len > 128) len = 128;
               stbiw__write_dump_data(f, len, &comp[x]);
               x += len;
            }
            // if there's a run, output it
            if (r+2 < width) { // same test as what we break out of in search loop, so only true if we break'd
               // find next byte after run
               while (r < width && comp[r] == comp[x])
                  ++r;
               // output run up to r
               while (x < r) {
                  int len = r-x;
                  if (len > 127) len = 127;
                  stbiw__write_run_data(f, len, comp[x]);
                  x += len;
               }
            }
         }
      }
   }
}

int stbi_write_hdr(char const *filename, int x, int y, int comp, const float *data)
{
   int i;
   FILE *f;
   if (y <= 0 || x <= 0 || data == NULL) return 0;
   f = fopen(filename, "wb");
   if (f) {
      /* Each component is stored separately. Allocate scratch space for full output scanline. */
      unsigned char *scratch = (unsigned char *) STBIW_MALLOC(x*4);
      fprintf(f, "#?RADIANCE\n# Written by stb_image_write.h\nFORMAT=32-bit_rle_rgbe\n"      );
      fprintf(f, "EXPOSURE=          1.0000000000000\n\n-Y %d +X %d\n"                 , y, x);
      for(i=0; i < y; i++)
         stbiw__write_hdr_scanline(f, x, comp, scratch, data + comp*i*x);
      STBIW_FREE(scratch);
      fclose(f);
   }
   return f != NULL;
}

/////////////////////////////////////////////////////////
// PNG

// stretchy buffer; stbiw__sbpush() == vector<>::push_back() -- stbiw__sbcount() == vector<>::size()
#define stbiw__sbraw(a) ((int *) (a) - 2)
#define stbiw__sbm(a)   stbiw__sbraw(a)[0]
#define stbiw__sbn(a)   stbiw__sbraw(a)[1]

#define stbiw__sbneedgrow(a,n)  ((a)==0 || stbiw__sbn(a)+n >= stbiw__sbm(a))
#define stbiw__sbmaybegrow(a,n) (stbiw__sbneedgrow(a,(n)) ? stbiw__sbgrow(a,n) : 0)
#define stbiw__sbgrow(a,n)  stbiw__sbgrowf((void **) &(a), (n), sizeof(*(a)))

#define stbiw__sbpush(a, v)      (stbiw__sbmaybegrow(a,1), (a)[stbiw__sbn(a)++] = (v))
#define stbiw__sbcount(a)        ((a) ? stbiw__sbn(a) : 0)
#define stbiw__sbfree(a)         ((a) ? STBIW_FREE(stbiw__sbraw(a)),0 : 0)

static void *stbiw__sbgrowf(void **arr, int increment, int itemsize)
{
   int m = *arr ? 2*stbiw__sbm(*arr)+increment : increment+1;
   void *p = STBIW_REALLOC(*arr ? stbiw__sbraw(*arr) : 0, itemsize * m + sizeof(int)*2);
   STBIW_ASSERT(p);
   if (p) {
      if (!*arr) ((int *) p)[1] = 0;
      *arr = (void *) ((int *) p + 2);
      stbiw__sbm(*arr) = m;
   }
   return *arr;
}

static unsigned char *stbiw__zlib_flushf(unsigned char *data, unsigned int *bitbuffer, int *bitcount)
{
   while (*bitcount >= 8) {
      stbiw__sbpush(data, (unsigned char) *bitbuffer);
      *bitbuffer >>= 8;
      *bitcount -= 8;
   }
   return data;
}

static int stbiw__zlib_bitrev(int code, int codebits)
{
   int res=0;
   while (codebits--) {
      res = (res << 1) | (code & 1);
      code >>= 1;
   }
   return res;
}

static unsigned int stbiw__zlib_countm(unsigned char *a, unsigned char *b, int limit)
{
   int i;
   for (i=0; i < limit && i < 258; ++i)
      if (a[i] != b[i]) break;
   return i;
}

static unsigned int stbiw__zhash(unsigned char *data)
{
   stbiw_uint32 hash = data[0] + (data[1] << 8) + (data[2] << 16);
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;
   return hash;
}

#define stbiw__zlib_flush() (out = stbiw__zlib_flushf(out, &bitbuf, &bitcount))
#define stbiw__zlib_add(code,codebits) \
      (bitbuf |= (code) << bitcount, bitcount += (codebits), stbiw__zlib_flush())
#define stbiw__zlib_huffa(b,c)  stbiw__zlib_add(stbiw__zlib_bitrev(b,c),c)
// default huffman tables
#define stbiw__zlib_huff1(n)  stbiw__zlib_huffa(0x30 + (n), 8)
#define stbiw__zlib_huff2(n)  stbiw__zlib_huffa(0x190 + (n)-144, 9)
#define stbiw__zlib_huff3(n)  stbiw__zlib_huffa(0 + (n)-256,7)
#define stbiw__zlib_huff4(n)  stbiw__zlib_huffa(0xc0 + (n)-280,8)
#define stbiw__zlib_huff(n)  ((n) <= 143 ? stbiw__zlib_huff1(n) : (n) <= 255 ? stbiw__zlib_huff2(n) : (n) <= 279 ? stbiw__zlib_huff3(n) : stbiw__zlib_huff4(n))
#define stbiw__zlib_huffb(n) ((n) <= 143 ? stbiw__zlib_huff1(n) : stbiw__zlib_huff2(n))

#define stbiw__ZHASH   16384

unsigned char * stbi_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality)
{
   static unsigned short lengthc[] = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258, 259 };
   static unsigned char  lengtheb[]= { 0,0,0,0,0,0,0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,  4,  5,  5,  5,  5,  0 };
   static unsigned short distc[]   = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577, 32768 };
   static unsigned char  disteb[]  = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13 };
   unsigned int bitbuf=0;
   int i,j, bitcount=0;
   unsigned char *out = NULL;
   unsigned char **hash_table[stbiw__ZHASH]; // 64KB on the stack!
   if (quality < 5) quality = 5;

   stbiw__sbpush(out, 0x78);   // DEFLATE 32K window
   stbiw__sbpush(out, 0x5e);   // FLEVEL = 1
   stbiw__zlib_add(1,1);  // BFINAL = 1
   stbiw__zlib_add(1,2);  // BTYPE = 1 -- fixed huffman

   for (i=0; i < stbiw__ZHASH; ++i)
      hash_table[i] = NULL;

   i=0;
   while (i < data_len-3) {
      // hash next 3 bytes of data to be compressed
      int h = stbiw__zhash(data+i)&(stbiw__ZHASH-1), best=3;
      unsigned char *bestloc = 0;
      unsigned char **hlist = hash_table[h];
      int n = stbiw__sbcount(hlist);
      for (j=0; j < n; ++j) {
         if (hlist[j]-data > i-32768) { // if entry lies within window
            int d = stbiw__zlib_countm(hlist[j], data+i, data_len-i);
            if (d >= best) best=d,bestloc=hlist[j];
         }
      }
      // when hash table entry is too long, delete half the entries
      if (hash_table[h] && stbiw__sbn(hash_table[h]) == 2*quality) {
         STBIW_MEMMOVE(hash_table[h], hash_table[h]+quality, sizeof(hash_table[h][0])*quality);
         stbiw__sbn(hash_table[h]) = quality;
      }
      stbiw__sbpush(hash_table[h],data+i);

      if (bestloc) {
         // "lazy matching" - check match at *next* byte, and if it's better, do cur byte as literal
         h = stbiw__zhash(data+i+1)&(stbiw__ZHASH-1);
         hlist = hash_table[h];
         n = stbiw__sbcount(hlist);
         for (j=0; j < n; ++j) {
            if (hlist[j]-data > i-32767) {
               int e = stbiw__zlib_countm(hlist[j], data+i+1, data_len-i-1);
               if (e > best) { // if next match is better, bail on current match
                  bestloc = NULL;
                  break;
               }
            }
         }
      }

      if (bestloc) {
         int d = (int) (data+i - bestloc); // distance back
         STBIW_ASSERT(d <= 32767 && best <= 258);
         for (j=0; best > lengthc[j+1]-1; ++j);
         stbiw__zlib_huff(j+257);
         if (lengtheb[j]) stbiw__zlib_add(best - lengthc[j], lengtheb[j]);
         for (j=0; d > distc[j+1]-1; ++j);
         stbiw__zlib_add(stbiw__zlib_bitrev(j,5),5);
         if (disteb[j]) stbiw__zlib_add(d - distc[j], disteb[j]);
         i += best;
      } else {
         stbiw__zlib_huffb(data[i]);
         ++i;
      }
   }
   // write out final bytes
   for (;i < data_len; ++i)
      stbiw__zlib_huffb(data[i]);
   stbiw__zlib_huff(256); // end of block
   // pad with 0 bits to byte boundary
   while (bitcount)
      stbiw__zlib_add(0,1);

   for (i=0; i < stbiw__ZHASH; ++i)
      (void) stbiw__sbfree(hash_table[i]);

   {
      // compute adler32 on input
      unsigned int i=0, s1=1, s2=0, blocklen = data_len % 5552;
      int j=0;
      while (j < data_len) {
         for (i=0; i < blocklen; ++i) s1 += data[j+i], s2 += s1;
         s1 %= 65521, s2 %= 65521;
         j += blocklen;
         blocklen = 5552;
      }
      stbiw__sbpush(out, (unsigned char) (s2 >> 8));
      stbiw__sbpush(out, (unsigned char) s2);
      stbiw__sbpush(out, (unsigned char) (s1 >> 8));
      stbiw__sbpush(out, (unsigned char) s1);
   }
   *out_len = stbiw__sbn(out);
   // make returned pointer freeable
   STBIW_MEMMOVE(stbiw__sbraw(out), out, *out_len);
   return (unsigned char *) stbiw__sbraw(out);
}

unsigned int stbiw__crc32(unsigned char *buffer, int len)
{
   static unsigned int crc_table[256];
   unsigned int crc = ~0u;
   int i,j;
   if (crc_table[1] == 0)
      for(i=0; i < 256; i++)
         for (crc_table[i]=i, j=0; j < 8; ++j)
            crc_table[i] = (crc_table[i] >> 1) ^ (crc_table[i] & 1 ? 0xedb88320 : 0);
   for (i=0; i < len; ++i)
      crc = (crc >> 8) ^ crc_table[buffer[i] ^ (crc & 0xff)];
   return ~crc;
}

#define stbiw__wpng4(o,a,b,c,d) ((o)[0]=(unsigned char)(a),(o)[1]=(unsigned char)(b),(o)[2]=(unsigned char)(c),(o)[3]=(unsigned char)(d),(o)+=4)
#define stbiw__wp32(data,v) stbiw__wpng4(data, (v)>>24,(v)>>16,(v)>>8,(v));
#define stbiw__wptag(data,s) stbiw__wpng4(data, s[0],s[1],s[2],s[3])

static void stbiw__wpcrc(unsigned char **data, int len)
{
   unsigned int crc = stbiw__crc32(*data - len - 4, len+4);
   stbiw__wp32(*data, crc);
}

static unsigned char stbiw__paeth(int a, int b, int c)
{
   int p = a + b - c, pa = abs(p-a), pb = abs(p-b), pc = abs(p-c);
   if (pa <= pb && pa <= pc) return (unsigned char) a;
   if (pb <= pc) return (unsigned char) b;
   return (unsigned char) c;
}

unsigned char *stbi_write_png_to_mem(unsigned char *pixels, int stride_bytes, int x, int y, int n, int *out_len)
{
   int ctype[5] = { -1, 0, 4, 2, 6 };
   unsigned char sig[8] = { 137,80,78,71,13,10,26,10 };
   unsigned char *out,*o, *filt, *zlib;
   signed char *line_buffer;
   int i,j,k,p,zlen;

   if (stride_bytes == 0)
      stride_bytes = x * n;

   filt = (unsigned char *) STBIW_MALLOC((x*n+1) * y); if (!filt) return 0;
   line_buffer = (signed char *) STBIW_MALLOC(x * n); if (!line_buffer) { STBIW_FREE(filt); return 0; }
   for (j=0; j < y; ++j) {
      static int mapping[] = { 0,1,2,3,4 };
      static int firstmap[] = { 0,1,0,5,6 };
      int *mymap = j ? mapping : firstmap;
      int best = 0, bestval = 0x7fffffff;
      for (p=0; p < 2; ++p) {
         for (k= p?best:0; k < 5; ++k) {
            int type = mymap[k],est=0;
            unsigned char *z = pixels + stride_bytes*j;
            for (i=0; i < n; ++i)
               switch (type) {
                  case 0: line_buffer[i] = z[i]; break;
                  case 1: line_buffer[i] = z[i]; break;
                  case 2: line_buffer[i] = z[i] - z[i-stride_bytes]; break;
                  case 3: line_buffer[i] = z[i] - (z[i-stride_bytes]>>1); break;
                  case 4: line_buffer[i] = (signed char) (z[i] - stbiw__paeth(0,z[i-stride_bytes],0)); break;
                  case 5: line_buffer[i] = z[i]; break;
                  case 6: line_buffer[i] = z[i]; break;
               }
            for (i=n; i < x*n; ++i) {
               switch (type) {
                  case 0: line_buffer[i] = z[i]; break;
                  case 1: line_buffer[i] = z[i] - z[i-n]; break;
                  case 2: line_buffer[i] = z[i] - z[i-stride_bytes]; break;
                  case 3: line_buffer[i] = z[i] - ((z[i-n] + z[i-stride_bytes])>>1); break;
                  case 4: line_buffer[i] = z[i] - stbiw__paeth(z[i-n], z[i-stride_bytes], z[i-stride_bytes-n]); break;
                  case 5: line_buffer[i] = z[i] - (z[i-n]>>1); break;
                  case 6: line_buffer[i] = z[i] - stbiw__paeth(z[i-n], 0,0); break;
               }
            }
            if (p) break;
            for (i=0; i < x*n; ++i)
               est += abs((signed char) line_buffer[i]);
            if (est < bestval) { bestval = est; best = k; }
         }
      }
      // when we get here, best contains the filter type, and line_buffer contains the data
      filt[j*(x*n+1)] = (unsigned char) best;
      STBIW_MEMMOVE(filt+j*(x*n+1)+1, line_buffer, x*n);
   }
   STBIW_FREE(line_buffer);
   zlib = stbi_zlib_compress(filt, y*( x*n+1), &zlen, 8); // increase 8 to get smaller but use more memory
   STBIW_FREE(filt);
   if (!zlib) return 0;

   // each tag requires 12 bytes of overhead
   out = (unsigned char *) STBIW_MALLOC(8 + 12+13 + 12+zlen + 12);
   if (!out) return 0;
   *out_len = 8 + 12+13 + 12+zlen + 12;

   o=out;
   STBIW_MEMMOVE(o,sig,8); o+= 8;
   stbiw__wp32(o, 13); // header length
   stbiw__wptag(o, "IHDR");
   stbiw__wp32(o, x);
   stbiw__wp32(o, y);
   *o++ = 8;
   *o++ = (unsigned char) ctype[n];
   *o++ = 0;
   *o++ = 0;
   *o++ = 0;
   stbiw__wpcrc(&o,13);

   stbiw__wp32(o, zlen);
   stbiw__wptag(o, "IDAT");
   STBIW_MEMMOVE(o, zlib, zlen);
   o += zlen;
   STBIW_FREE(zlib);
   stbiw__wpcrc(&o, zlen);

   stbiw__wp32(o,0);
   stbiw__wptag(o, "IEND");
   stbiw__wpcrc(&o,0);

   STBIW_ASSERT(o == out + *out_len);

   return out;
}

int stbi_write_png(char const *filename, int x, int y, int comp, const void *data, int stride_bytes)
{
   FILE *f;
   int len;
   unsigned char *png = stbi_write_png_to_mem((unsigned char *) data, stride_bytes, x, y, comp, &len);
   if (!png) return 0;
   f = fopen(filename, "wb");
   if (!f) { STBIW_FREE(png); return 0; }
   fwrite(png, 1, len, f);
   fclose(f);
   STBIW_FREE(png);
   return 1;
}
#endif // STB_IMAGE_WRITE_IMPLEMENTATION

/* Revision history
      0.98 (2015-04-08)
             added STBIW_MALLOC, STBIW_ASSERT etc
      0.97 (2015-01-18)
             fixed HDR asserts, rewrote HDR rle logic
      0.96 (2015-01-17)
             add HDR output
             fix monochrome BMP
      0.95 (2014-08-17)
		       add monochrome TGA output
      0.94 (2014-05-31)
             rename private functions to avoid conflicts with stb_image.h
      0.93 (2014-05-27)
             warning fixes
      0.92 (2010-08-01)
             casts to unsigned char to fix warnings
      0.91 (2010-07-17)
             first public release
      0.90   first internal release
*/
