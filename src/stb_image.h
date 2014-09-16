/* stbi-1.33 - public domain JPEG/PNG reader - http://nothings.org/stb_image.c
   when you control the images you're loading
                                     no warranty implied; use at your own risk

   QUICK NOTES:
      Primarily of interest to game developers and other people who can
          avoid problematic images and only need the trivial interface

      JPEG baseline (no JPEG progressive)
      PNG 8-bit-per-channel only

      TGA (not sure what subset, if a subset)
      BMP non-1bpp, non-RLE
      PSD (composited view only, no extra channels)

      GIF (*comp always reports as 4-channel)
      HDR (radiance rgbE format)
      PIC (Softimage PIC)

      - decode from memory or through FILE (define STBI_NO_STDIO to remove code)
      - decode from arbitrary I/O callbacks
      - overridable dequantizing-IDCT, YCbCr-to-RGB conversion (define STBI_SIMD)

   Latest revisions:
      1.33 (2011-07-14) minor fixes suggested by Dave Moore
      1.32 (2011-07-13) info support for all filetypes (SpartanJ)
      1.31 (2011-06-19) a few more leak fixes, bug in PNG handling (SpartanJ)
      1.30 (2011-06-11) added ability to load files via io callbacks (Ben Wenger)
      1.29 (2010-08-16) various warning fixes from Aurelien Pocheville 
      1.28 (2010-08-01) fix bug in GIF palette transparency (SpartanJ)
      1.27 (2010-08-01) cast-to-uint8 to fix warnings (Laurent Gomila)
                        allow trailing 0s at end of image data (Laurent Gomila)
      1.26 (2010-07-24) fix bug in file buffering for PNG reported by SpartanJ

   See end of file for full revision history.

   TODO:
      stbi_info support for BMP,PSD,HDR,PIC


 ============================    Contributors    =========================
              
 Image formats                                Optimizations & bugfixes
    Sean Barrett (jpeg, png, bmp)                Fabian "ryg" Giesen
    Nicolas Schulz (hdr, psd)                                                 
    Jonathan Dummer (tga)                     Bug fixes & warning fixes           
    Jean-Marc Lienher (gif)                      Marc LeBlanc               
    Tom Seddon (pic)                             Christpher Lloyd           
    Thatcher Ulrich (psd)                        Dave Moore                 
                                                 Won Chun                   
                                                 the Horde3D community      
 Extensions, features                            Janez Zemva                
    Jetro Lauha (stbi_info)                      Jonathan Blow              
    James "moose2000" Brown (iPhone PNG)         Laurent Gomila                             
    Ben "Disch" Wenger (io callbacks)            Aruelien Pocheville
    Martin "SpartanJ" Golini                     Ryamond Barbiero
                                                 David Woo
                                                 

 If your name should be here but isn't, let Sean know.

*/

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

// To get a header file for this, either cut and paste the header,
// or create stb_image.h, #define STBI_HEADER_FILE_ONLY, and
// then include stb_image.c from it.

////   begin header file  ////////////////////////////////////////////////////
//
// Limitations:
//    - no jpeg progressive support
//    - non-HDR formats support 8-bit samples only (jpeg, png)
//    - no delayed line count (jpeg) -- IJG doesn't support either
//    - no 1-bit BMP
//    - GIF always returns *comp=4
//
// Basic usage (see HDR discussion below):
//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not NULL ... 
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
//    stbi_image_free(data)
//
// Standard parameters:
//    int *x       -- outputs image width in pixels
//    int *y       -- outputs image height in pixels
//    int *comp    -- outputs # of image components in image file
//    int req_comp -- if non-zero, # of image components requested in result
//
// The return value from an image loader is an 'unsigned char *' which points
// to the pixel data. The pixel data consists of *y scanlines of *x pixels,
// with each pixel consisting of N interleaved 8-bit components; the first
// pixel pointed to is top-left-most in the image. There is no padding between
// image scanlines or between pixels, regardless of format. The number of
// components N is 'req_comp' if req_comp is non-zero, or *comp otherwise.
// If req_comp is non-zero, *comp has the number of components that _would_
// have been output otherwise. E.g. if you set req_comp to 4, you will always
// get RGBA output, but you can check *comp to easily see if it's opaque.
//
// An output image with N components has the following components interleaved
// in this order in each pixel:
//
//     N=#comp     components
//       1           grey
//       2           grey, alpha
//       3           red, green, blue
//       4           red, green, blue, alpha
//
// If image loading fails for any reason, the return value will be NULL,
// and *x, *y, *comp will be unchanged. The function stbi_failure_reason()
// can be queried for an extremely brief, end-user unfriendly explanation
// of why the load failed. Define STBI_NO_FAILURE_STRINGS to avoid
// compiling these strings at all, and STBI_FAILURE_USERMSG to get slightly
// more user-friendly ones.
//
// Paletted PNG, BMP, GIF, and PIC images are automatically depalettized.
//
// ===========================================================================
//
// iPhone PNG support:
//
// By default we convert iphone-formatted PNGs back to RGB; nominally they
// would silently load as BGR, except the existing code should have just
// failed on such iPhone PNGs. But you can disable this conversion by
// by calling stbi_convert_iphone_png_to_rgb(0), in which case
// you will always just get the native iphone "format" through.
//
// Call stbi_set_unpremultiply_on_load(1) as well to force a divide per
// pixel to remove any premultiplied alpha *only* if the image file explicitly
// says there's premultiplied data (currently only happens in iPhone images,
// and only if iPhone convert-to-rgb processing is on).
//
// ===========================================================================
//
// HDR image support   (disable by defining STBI_NO_HDR)
//
// stb_image now supports loading HDR images in general, and currently
// the Radiance .HDR file format, although the support is provided
// generically. You can still load any file through the existing interface;
// if you attempt to load an HDR file, it will be automatically remapped to
// LDR, assuming gamma 2.2 and an arbitrary scale factor defaulting to 1;
// both of these constants can be reconfigured through this interface:
//
//     stbi_hdr_to_ldr_gamma(2.2f);
//     stbi_hdr_to_ldr_scale(1.0f);
//
// (note, do not use _inverse_ constants; stbi_image will invert them
// appropriately).
//
// Additionally, there is a new, parallel interface for loading files as
// (linear) floats to preserve the full dynamic range:
//
//    float *data = stbi_loadf(filename, &x, &y, &n, 0);
// 
// If you load LDR images through this interface, those images will
// be promoted to floating point values, run through the inverse of
// constants corresponding to the above:
//
//     stbi_ldr_to_hdr_scale(1.0f);
//     stbi_ldr_to_hdr_gamma(2.2f);
//
// Finally, given a filename (or an open file or memory block--see header
// file for details) containing image data, you can query for the "most
// appropriate" interface to use (that is, whether the image is HDR or
// not), using:
//
//     stbi_is_hdr(char *filename);
//
// ===========================================================================
//
// I/O callbacks
//
// I/O callbacks allow you to read from arbitrary sources, like packaged
// files or some other source. Data read from callbacks are processed
// through a small internal buffer (currently 128 bytes) to try to reduce
// overhead. 
//
// The three functions you must define are "read" (reads some bytes of data),
// "skip" (skips some bytes of data), "eof" (reports if the stream is at the end).

#define STBI_NO_HDR		// RaySan: not required by raylib

#ifndef STBI_NO_STDIO

#if defined(_MSC_VER) && _MSC_VER >= 0x1400
#define _CRT_SECURE_NO_WARNINGS // suppress bogus warnings about fopen()
#endif

#include <stdio.h>
#endif

// NOTE: Added to work with raylib on Android
#if defined(PLATFORM_ANDROID)
    #include "utils.h"  // Android fopen function map
#endif

#define STBI_VERSION 1

enum
{
   STBI_default = 0, // only used for req_comp

   STBI_grey       = 1,
   STBI_grey_alpha = 2,
   STBI_rgb        = 3,
   STBI_rgb_alpha  = 4
};

typedef unsigned char stbi_uc;

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API - works on images of any type
//

//
// load image by filename, open file, or memory buffer
//

extern stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);

#ifndef STBI_NO_STDIO
extern stbi_uc *stbi_load            (char const *filename,     int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi_load_from_file  (FILE *f,                  int *x, int *y, int *comp, int req_comp);
// for stbi_load_from_file, file pointer is left pointing immediately after image
#endif

typedef struct
{
   int      (*read)  (void *user,char *data,int size);   // fill 'data' with 'size' bytes.  return number of bytes actually read 
   void     (*skip)  (void *user,unsigned n);            // skip the next 'n' bytes
   int      (*eof)   (void *user);                       // returns nonzero if we are at end of file/data
} stbi_io_callbacks;

extern stbi_uc *stbi_load_from_callbacks  (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp);

#ifndef STBI_NO_HDR
   extern float *stbi_loadf_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);

   #ifndef STBI_NO_STDIO
   extern float *stbi_loadf            (char const *filename,   int *x, int *y, int *comp, int req_comp);
   extern float *stbi_loadf_from_file  (FILE *f,                int *x, int *y, int *comp, int req_comp);
   #endif
   
   extern float *stbi_loadf_from_callbacks  (stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp);

   extern void   stbi_hdr_to_ldr_gamma(float gamma);
   extern void   stbi_hdr_to_ldr_scale(float scale);

   extern void   stbi_ldr_to_hdr_gamma(float gamma);
   extern void   stbi_ldr_to_hdr_scale(float scale);
#endif // STBI_NO_HDR

// stbi_is_hdr is always defined
extern int    stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user);
extern int    stbi_is_hdr_from_memory(stbi_uc const *buffer, int len);
#ifndef STBI_NO_STDIO
extern int      stbi_is_hdr          (char const *filename);
extern int      stbi_is_hdr_from_file(FILE *f);
#endif // STBI_NO_STDIO


// get a VERY brief reason for failure
// NOT THREADSAFE
extern const char *stbi_failure_reason  (void); 

// free the loaded image -- this is just free()
extern void     stbi_image_free      (void *retval_from_stbi_load);

// get image dimensions & components without fully decoding
extern int      stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);
extern int      stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp);

#ifndef STBI_NO_STDIO
extern int      stbi_info            (char const *filename,     int *x, int *y, int *comp);
extern int      stbi_info_from_file  (FILE *f,                  int *x, int *y, int *comp);

#endif



// for image formats that explicitly notate that they have premultiplied alpha,
// we just return the colors as stored in the file. set this flag to force
// unpremultiplication. results are undefined if the unpremultiply overflow.
extern void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply);

// indicate whether we should process iphone images back to canonical format,
// or just pass them through "as-is"
extern void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert);


// ZLIB client - used by PNG, available for other purposes

extern char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
extern char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);
extern int   stbi_zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

extern char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
extern int   stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);


// define faster low-level operations (typically SIMD support)
#ifdef STBI_SIMD
typedef void (*stbi_idct_8x8)(stbi_uc *out, int out_stride, short data[64], unsigned short *dequantize);
// compute an integer IDCT on "input"
//     input[x] = data[x] * dequantize[x]
//     write results to 'out': 64 samples, each run of 8 spaced by 'out_stride'
//                             CLAMP results to 0..255
typedef void (*stbi_YCbCr_to_RGB_run)(stbi_uc *output, stbi_uc const  *y, stbi_uc const *cb, stbi_uc const *cr, int count, int step);
// compute a conversion from YCbCr to RGB
//     'count' pixels
//     write pixels to 'output'; each pixel is 'step' bytes (either 3 or 4; if 4, write '255' as 4th), order R,G,B
//     y: Y input channel
//     cb: Cb input channel; scale/biased to be 0..255
//     cr: Cr input channel; scale/biased to be 0..255

extern void stbi_install_idct(stbi_idct_8x8 func);
extern void stbi_install_YCbCr_to_RGB(stbi_YCbCr_to_RGB_run func);
#endif // STBI_SIMD


#ifdef __cplusplus
}
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // STBI_INCLUDE_STB_IMAGE_H