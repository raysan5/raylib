/* stb_image_resize2 - v2.50 - public domain image resizing

   by Jeff Roberts (v2) and Jorge L Rodriguez
   http://github.com/nothings/stb

   Can be threaded with the extended API. SSE2, AVX, Neon and WASM SIMD support. Only
   scaling and translation is supported, no rotations or shears.

   COMPILING & LINKING
      In one C/C++ file that #includes this file, do this:
         #define STB_IMAGE_RESIZE_IMPLEMENTATION
      before the #include. That will create the implementation in that file.

   EASY API CALLS:
     Easy API downsamples w/Mitchell filter, upsamples w/cubic interpolation, clamps to edge.

     stbir_resize_uint8_srgb( input_pixels,  input_w,  input_h,  input_stride_in_bytes,
                              output_pixels, output_w, output_h, output_stride_in_bytes,
                              pixel_layout_enum )

     stbir_resize_uint8_linear( input_pixels,  input_w,  input_h,  input_stride_in_bytes,
                                output_pixels, output_w, output_h, output_stride_in_bytes,
                                pixel_layout_enum )

     stbir_resize_float_linear( input_pixels,  input_w,  input_h,  input_stride_in_bytes,
                                output_pixels, output_w, output_h, output_stride_in_bytes,
                                pixel_layout_enum )

     If you pass NULL or zero for the output_pixels, we will allocate the output buffer
     for you and return it from the function (free with free() or STBIR_FREE).
     As a special case, XX_stride_in_bytes of 0 means packed continuously in memory.

   API LEVELS
      There are three levels of API - easy-to-use, medium-complexity and extended-complexity.

      See the "header file" section of the source for API documentation.

   ADDITIONAL DOCUMENTATION

      MEMORY ALLOCATION
         By default, we use malloc and free for memory allocation.  To override the
         memory allocation, before the implementation #include, add a:

            #define STBIR_MALLOC(size,user_data) ...
            #define STBIR_FREE(ptr,user_data)   ...

         Each resize makes exactly one call to malloc/free (unless you use the
         extended API where you can do one allocation for many resizes). Under
         address sanitizer, we do separate allocations to find overread/writes.

      PERFORMANCE
         This library was written with an emphasis on performance. When testing
         stb_image_resize with RGBA, the fastest mode is STBIR_4CHANNEL with
         STBIR_TYPE_UINT8 pixels and CLAMPed edges (which is what many other resize
         libs do by default). Also, make sure SIMD is turned on of course (default
         for 64-bit targets). Avoid WRAP edge mode if you want the fastest speed.

         This library also comes with profiling built-in. If you define STBIR_PROFILE,
         you can use the advanced API and get low-level profiling information by
         calling stbir_resize_extended_profile_info() or stbir_resize_split_profile_info()
         after a resize.

      SIMD
         Most of the routines have optimized SSE2, AVX, AVX-512, NEON and WASM versions.

         On Microsoft compilers, we automatically turn on SIMD for 64-bit x64 and
         ARM; for 32-bit x86 and ARM, you select SIMD mode by defining STBIR_SSE2 or
         STBIR_NEON. 

         For AVX and AVX2, we auto-select it by detecting the /arch:AVX
         or /arch:AVX2 switches. You can also always manually turn SSE2, AVX or AVX2
         support on by defining STBIR_SSE2, STBIR_AVX or STBIR_AVX2.

         For AVX-512, we turn it on by /arch:AVX512 (or -march=x86-64-v4, or the
         -mavx512f -mavx512bw -mavx512vl trio on clang and GCC), or by defining
         STBIR_AVX512. It's mostly useful on Zen 5 or modern Intel cores that don't
         downclock on use. Best on very large resizes.

         On Linux, SSE2 and Neon is on by default for 64-bit x64 or ARM64. For 32-bit,
         we select x86 SIMD mode by whether you have -msse2, -mavx or -mavx2 enabled
         on the command line. For 32-bit ARM, you must pass -mfpu=neon-vfpv4 for both
         clang and GCC, but GCC also requires an additional -mfp16-format=ieee to
         automatically enable NEON.

         On x86 platforms, you can also define STBIR_FP16C to turn on FP16C instructions
         for converting back and forth to half-floats. This is autoselected when we
         are using AVX2. Clang and GCC also require the -mf16c switch. ARM always uses
         the built-in half float hardware NEON instructions.

         You can also tell us to use multiply-add instructions with STBIR_USE_FMA.
         Because x86 doesn't always have fma, we turn it off by default to maintain
         determinism across all platforms. If you don't care about non-FMA determinism
         and are willing to restrict yourself to more recent x86 CPUs (around the AVX
         timeframe), then fma will give you around a 15% speedup.

         You can force off SIMD in all cases by defining STBIR_NO_SIMD. You can turn
         off AVX, AVX2 or AVX-512 specifically with STBIR_NO_AVX, STBIR_NO_AVX2 or
         STBIR_NO_AVX512. AVX is 10% to 40% faster, and AVX2 is generally another 12%.

      ALPHA CHANNEL
         Most of the resizing functions provide the ability to control how the alpha
         channel of an image is processed.

         When alpha represents transparency, it is important that when combining
         colors with filtering, the pixels should not be treated equally; they
         should use a weighted average based on their alpha values. For example,
         if a pixel is 1% opaque bright green and another pixel is 99% opaque
         black and you average them, the average will be 50% opaque, but the
         unweighted average and will be a middling green color, while the weighted
         average will be nearly black. This means the unweighted version introduced
         green energy that didn't exist in the source image.

         (If you want to know why this makes sense, you can work out the math for
         the following: consider what happens if you alpha composite a source image
         over a fixed color and then average the output, vs. if you average the
         source image pixels and then composite that over the same fixed color.
         Only the weighted average produces the same result as the ground truth
         composite-then-average result.)

         Therefore, it is in general best to "alpha weight" the pixels when applying
         filters to them. This essentially means multiplying the colors by the alpha
         values before combining them, and then dividing by the alpha value at the
         end.

         The computer graphics industry introduced a technique called "premultiplied
         alpha" or "associated alpha" in which image colors are stored in image files
         already multiplied by their alpha. This saves some math when compositing,
         and also avoids the need to divide by the alpha at the end (which is quite
         inefficient). However, while premultiplied alpha is common in the movie CGI
         industry, it is not commonplace in other industries like videogames, and most
         consumer file formats are generally expected to contain not-premultiplied
         colors. For example, Photoshop saves PNG files "unpremultiplied", and web
         browsers like Chrome and Firefox expect PNG images to be unpremultiplied.

         Note that there are three possibilities that might describe your image
         and resize expectation:

             1. images are not premultiplied, alpha weighting is desired
             2. images are not premultiplied, alpha weighting is not desired
             3. images are premultiplied

         Both case #2 and case #3 require the exact same math: no alpha weighting
         should be applied or removed. Only case 1 requires extra math operations;
         the other two cases can be handled identically.

         stb_image_resize expects case #1 by default, applying alpha weighting to
         images, expecting the input images to be unpremultiplied. This is what the
         COLOR+ALPHA buffer types tell the resizer to do.

         When you use the pixel layouts STBIR_RGBA, STBIR_BGRA, STBIR_ARGB,
         STBIR_ABGR, STBIR_RA, or STBIR_AR you are telling us that the pixels are
         non-premultiplied. In these cases, the resizer will alpha weight the colors
         (effectively creating the premultiplied image), do the filtering, and then
         convert back to non-premult on exit.

         When you use the pixel layouts STBIR_RGBA_PM, STBIR_BGRA_PM, STBIR_ARGB_PM,
         STBIR_ABGR_PM, STBIR_RA_PM or STBIR_AR_PM, you are telling that the pixels
         ARE premultiplied. In this case, the resizer doesn't have to do the
         premultipling - it can filter directly on the input. This about twice as
         fast as the non-premultiplied case, so it's the right option if your data is
         already setup correctly.

         When you use the pixel layout STBIR_4CHANNEL or STBIR_2CHANNEL, you are
         telling us that there is no channel that represents transparency; it may be
         RGB and some unrelated fourth channel that has been stored in the alpha
         channel, but it is actually not alpha. No special processing will be
         performed.

         The difference between the generic 4 or 2 channel layouts, and the
         specialized _PM versions is with the _PM versions you are telling us that
         the data *is* alpha, just don't premultiply it. That's important when
         using SRGB pixel formats, we need to know where the alpha is, because
         it is converted linearly (rather than with the SRGB converters).

         Because alpha weighting produces the same effect as premultiplying, you
         even have the option with non-premultiplied inputs to let the resizer
         produce a premultiplied output. Because the intially computed alpha-weighted
         output image is effectively premultiplied, this is actually more performant
         than the normal path which un-premultiplies the output image as a final step.

         Finally, when converting both in and out of non-premulitplied space (for
         example, when using STBIR_RGBA), we go to somewhat heroic measures to
         ensure that areas with zero alpha value pixels get something reasonable
         in the RGB values. If you don't care about the RGB values of zero alpha
         pixels, you can call the stbir_set_non_pm_alpha_speed_over_quality()
         function - this runs a premultiplied resize about 25% faster. That said,
         when you really care about speed, using premultiplied pixels for both in
         and out (STBIR_RGBA_PM, etc) much faster than both of these premultiplied
         options.

      PIXEL LAYOUT CONVERSION
         The resizer can convert from some pixel layouts to others. When using the
         stbir_set_pixel_layouts(), you can, for example, specify STBIR_RGBA
         on input, and STBIR_ARGB on output, and it will re-organize the channels
         during the resize. Currently, you can only convert between two pixel
         layouts with the same number of channels.

      DETERMINISM
         We commit to being deterministic (from x64 to ARM to scalar to SIMD, etc).
         This requires compiling with fast-math off (using at least /fp:precise).
         Also, you must turn off fp-contracting (which turns mult+adds into fmas)!
         We attempt to do this with pragmas, but with Clang, you usually want to add
         -ffp-contract=off to the command line as well.

         For 32-bit x86, you must use SSE and SSE2 codegen for determinism. That is,
         if the scalar x87 unit gets used at all, we immediately lose determinism.
         On Microsoft Visual Studio 2008 and earlier, from what we can tell there is
         no way to be deterministic in 32-bit x86 (some x87 always leaks in, even
         with fp:strict). On 32-bit x86 GCC, determinism requires both -msse2 and
         -fpmath=sse.

         Note that we will not be deterministic with float data containing NaNs -
         the NaNs will propagate differently on different SIMD and platforms.

         If you turn on STBIR_USE_FMA, then we will be deterministic with other
         fma targets, but we will differ from non-fma targets (this is unavoidable,
         because a fma isn't simply an add with a mult - it also introduces a
         rounding difference compared to non-fma instruction sequences.

      FLOAT PIXEL FORMAT RANGE
         Any range of values can be used for the non-alpha float data that you pass
         in (0 to 1, -1 to 1, whatever). However, if you are inputting float values
         but *outputting* bytes or shorts, you must use a range of 0 to 1 so that we
         scale back properly. The alpha channel must also be 0 to 1 for any format
         that does premultiplication prior to resizing.

         Note also that with float output, using filters with negative lobes, the
         output filtered values might go slightly out of range. You can define
         STBIR_FLOAT_LOW_CLAMP and/or STBIR_FLOAT_HIGH_CLAMP to specify the range
         to clamp to on output, if that's important.

      MAX/MIN SCALE FACTORS
         Images can be up to 2^30 pixels wide or high (dimensions are ints, and
         the internal pointer math is done in size_t).

         The positions of the filter taps are calculated in stbir_real (double by
         default), which is accurate all the way to that limit. When the scale
         works out to an exact fraction (which it does for most whole image
         resizes), the positions are calculated with 64-bit integer math and are
         exact at any size.

         If you are on a part without hardware doubles, you can define stbir_real
         to float before you #include. Positions are then accurate to about 2^20
         pixels and are only off by half a pixel at 2^24, so for extreme resizes
         on those parts, do it in multiple stages (using float intermediate
         buffers).

      HUGE IMAGES AND TILING
         Image strides are 64-bit, so the buffers themselves can be as large as
         memory allows; widths and heights are limited to 2^30 (ints). To resize
         an image larger than that in either direction, resize it in tiles: for
         each output tile, point the resize at a piece of the input (the 64-bit
         strides reach anywhere in the buffer) with the output tile's size, and
         set the input region the tile maps to, in the piece's own coordinates,
         with stbir_set_input_subrect. Two things keep the tiles seamless:

           - the piece must extend past the region by the filter's reach on every
             side that isn't a real image edge (the filter's support in input
             pixels: support x input / output when downsampling, support when
             upsampling, plus a pixel of slack), because the edge modes act at
             the piece's edges - a tap that runs off the piece is clamped or
             wrapped there, as if the image ended. WRAP is the awkward one: a
             tile at a real edge needs the pixels from the opposite edge of the
             whole image, which a piece of the buffer can't reach, so you would
             have to copy them into place next to the piece - it is simpler to
             use another edge mode when tiling.

           - the region must be exactly the tile's share of the whole mapping
             (tile size x whole input / whole output, at the tile's offset), so
             every tile has the whole resize's scale and phase - a region drawn
             from rounded numbers shifts that tile's samples against its
             neighbors.

         A region that reaches past the source is clipped to it (the output
         pixels that would map outside are not written).

      FLIPPED IMAGES
         Stride is just the delta from one scanline to the next. This means you can
         use a negative stride to handle inverted images (point to the final
         scanline and use a negative stride). You can invert the input or output,
         using negative strides.

      DEFAULT FILTERS
         For functions which don't provide explicit control over what filters to
         use, you can change the compile-time defaults with:

            #define STBIR_DEFAULT_FILTER_UPSAMPLE     STBIR_FILTER_something
            #define STBIR_DEFAULT_FILTER_DOWNSAMPLE   STBIR_FILTER_something

         See stbir_filter in the header-file section for the list of filters.

      NEW FILTERS
         A number of 1D filter kernels are supplied. For a list of supported
         filters, see the stbir_filter enum. You can install your own filters by
         using the stbir_set_filter_callbacks_new function.

      PROGRESS
         For interactive use with slow resize operations, you can use the 
         scanline callbacks in the extended API. It would have to be a *very* large
         image resample to need progress though - we're very fast.

      ASSERT
         Define STBIR_ASSERT(boolval) to override assert() and not use assert.h

      CACHE SIZE
         A resize is run in column slices sized so that its working memory fits
         the L2 cache. By default, the L2 size is read with cpuid on x86 (and a L2 
         of a megabyte or more is treated as 512 KB, to keep the internal memory
         down), and taken as 256 KB on other cpus. If that isn't right for you,
         set stbir_l2_cache_size_override (bytes) before resizing, and it is used
         as is; or define STBIR_ASK_WINDOWS_FOR_L2_SIZE, STBIR_ASK_LINUX_FOR_L2_SIZE
         or STBIR_ASK_MACOS_FOR_L2_SIZE before the implementation include to ask
         the OS instead of cpuid (see the comment above them for the headers each
         one needs). Usually the defaults will just work fine.

      THREADING
         To resize on several threads, call stbir_build_samplers_with_splits on
         one thread with the number of threads you'd like, then call
         stbir_resize_extended_split on each thread with its index (and a count
         of 1), and stbir_free_samplers when they're all done. The build returns
         the number of splits to actually use: it is limited to how many ways
         the resize can be split, and then to the count the build estimates
         will still speed things up (a small image or very narrow rows don't
         get faster with more threads, they get slower). The estimate's constants
         are statics in the implementation that you can override if you need to.

      FUTURE TODOS
        *  Add pixel layout conversions for sensible different channel counts
           (maybe, 1->3/4, 3->4, 4->1, 3->1).
         * Convert the reincludes to macros when we know they aren't changing.
         * Most of our code is internally function pointers, should we compile
           all the SIMD stuff always and dynamically dispatch?

   CONTRIBUTORS
      Jeff Roberts: 2.0 implementation, optimizations, SIMD
      Martins Mozeiko: NEON simd, WASM simd, clang and GCC whisperer
      Fabian Giesen: half float and srgb converters
      Sean Barrett: API design, optimizations
      Jorge L Rodriguez: Original 1.0 implementation
      Aras Pranckevicius: bugfixes
      Nathan Reed: warning fixes for 1.0

   REVISIONS
      2.50 (2026-10-07) Big performance release, especially for large and/or
                        threaded resizes! New stuff:
                          - Resizes now run in column slices sized to the L2 cache 
                            (auto-detected, or set stbir_l2_cache_size_override), 
                            which is 1.6-3x faster on large resizes (more when 
                            threaded) and uses far less temp memory.
                          - Coefficient sets for rational scales are now shared, 
                            using much less memory for these resizes (and faster).
                          - Rational downsamples now get faster dedicated kernels
                            (20-30% faster).
                          - Added horizontal gather kernels up to 17 taps, so 
                            4-tap filters stay on the fast gather path down to 
                            4:1 (20-30% faster than scatter path). Useful in mip
                            chain generation loops.
                          - For each resize, we adjust the offset of the internal
                            buffers to avoid 4K aliasing with the input buffer
                            (avoids random 40% slowdowns).
                          - New model for estimating vertical or horizontal first
                            mode. Much better at predicting best direction (lots 
                            of resizes faster 30%-40% with this change).
                          - Added a model of thread addition improvement - if you
                            ask for 32 threads for example, the model will only use 
                            the thread count that will improve performance (some 
                            users were estimating this themselves - this should 
                            be more accurate).
                          - 1 channel downsizes 30-40% faster (SIMD across samples).
                          - Added some new AVX kernels and optimizations.
                          - Added AVX-512 support - useful mostly just for Zen 5
                            and recent Intel cores and large resamples.
                          - Bunch of cleanup and exactness in coefficient building,
                            along with new higher precision filter callbacks - this
                            moves our image resize limits up to 2^30.
                          - Added several new filters - more Mitchell variants,
                            some Lanczos, and the Muratori filter.
                          - Point sampling now actually does point sampling when
                            downsampling (previously only upsamples were point
                            sampled, down was averaged).
                          - Added 64-bit strides so that you can resize over the 
                            2^30 with manual tiling (calling resize multiple
                            times over the 64-bit area).
                          - Windows ARM64EC support (thanks navvyswethgraphics)
                          - Lots of little bugfixes in unusual corner cases.
                        NOTE THAT 2.50 IS NOT deterministic with prior versions
                        (among other improvements: coefficients calculated more 
                        precisely, new v-first vs  h-first decisions, etc).

      2.18 (2026-03-25) fixed coefficient calculation when skipping a coefficient off 
                          the left side of the window, added non-aligned access safe 
                          memcpy mode for scalar path, fixed various typos, and fixed 
                          define error in the float clamp output mode. 
      2.17 (2025-10-25) silly format bug in easy-to-use APIs.
      2.16 (2025-10-21) fixed the easy-to-use APIs to allow inverted bitmaps (negative
                          strides), fix vertical filter kernel callback, fix threaded
                          gather buffer priming (and assert).
                          (thanks adipose, TainZerL, and Harrison Green)
      2.15 (2025-07-17) fixed an assert in debug mode when using floats with input
                          callbacks, work around GCC warning when adding to null ptr
                          (thanks Johannes Spohr and Pyry Kovanen).
      2.14 (2025-05-09) fixed a bug using downsampling gather horizontal first, and 
                          scatter with vertical first.
      2.13 (2025-02-27) fixed a bug when using input callbacks, turned off simd for 
                          tiny-c, fixed some variables that should have been static,
                          fixes a bug when calculating temp memory with resizes that
                          exceed 2GB of temp memory (very large resizes).
      2.12 (2024-10-18) fix incorrect use of user_data with STBIR_FREE
      2.11 (2024-09-08) fix harmless asan warnings in 2-channel and 3-channel mode
                          with AVX-2, fix some weird scaling edge conditions with
                          point sample mode.
      2.10 (2024-07-27) fix the defines GCC and mingw for loop unroll control,
                          fix MSVC 32-bit arm half float routines.
      2.09 (2024-06-19) fix the defines for 32-bit ARM GCC builds (was selecting
                          hardware half floats).
      2.08 (2024-06-10) fix for RGB->BGR three channel flips and add SIMD (thanks
                          to Ryan Salsbury), fix for sub-rect resizes, use the
                          pragmas to control unrolling when they are available.
      2.07 (2024-05-24) fix for slow final split during threaded conversions of very 
                          wide scanlines when downsampling (caused by extra input 
                          converting), fix for wide scanline resamples with many 
                          splits (int overflow), fix GCC warning.
      2.06 (2024-02-10) fix for identical width/height 3x or more down-scaling 
                          undersampling a single row on rare resize ratios (about 1%).
      2.05 (2024-02-07) fix for 2 pixel to 1 pixel resizes with wrap (thanks Aras),
                        fix for output callback (thanks Julien Koenen).
      2.04 (2023-11-17) fix for rare AVX bug, shadowed symbol (thanks Nikola Smiljanic).
      2.03 (2023-11-01) ASAN and TSAN warnings fixed, minor tweaks.
      2.00 (2023-10-10) mostly new source: new api, optimizations, simd, vertical-first, etc
                          2x-5x faster without simd, 4x-12x faster with simd,
                          in some cases, 20x to 40x faster esp resizing large to very small.
      0.96 (2019-03-04) fixed warnings
      0.95 (2017-07-23) fixed warnings
      0.94 (2017-03-18) fixed warnings
      0.93 (2017-03-03) fixed bug with certain combinations of heights
      0.92 (2017-01-02) fix integer overflow on large (>2GB) images
      0.91 (2016-04-02) fix warnings; fix handling of subpixel regions
      0.90 (2014-09-17) first released version

   LICENSE
     See end of file for license information.
*/

#if !defined(STB_IMAGE_RESIZE_DO_HORIZONTALS) && !defined(STB_IMAGE_RESIZE_DO_VERTICALS) && !defined(STB_IMAGE_RESIZE_DO_CODERS)   // for internal re-includes

#ifndef STBIR_INCLUDE_STB_IMAGE_RESIZE2_H
#define STBIR_INCLUDE_STB_IMAGE_RESIZE2_H

#include <stddef.h>
#ifdef _MSC_VER
typedef unsigned char    stbir_uint8;
typedef unsigned short   stbir_uint16;
typedef unsigned int     stbir_uint32;
typedef unsigned __int64 stbir_uint64;
typedef __int64          stbir_int64;
#else
#include <stdint.h>
typedef uint8_t  stbir_uint8;
typedef uint16_t stbir_uint16;
typedef uint32_t stbir_uint32;
typedef uint64_t stbir_uint64;
typedef int64_t  stbir_int64;
#endif

// we use this type for high precision floating point work - it's normally just a double
//   but you can define it to float, if you don't have double support (some old embedded
//   platforms). 
#ifndef stbir_real
#define stbir_real double
#endif

#ifndef STBIRDEF
#ifdef STB_IMAGE_RESIZE_STATIC
#define STBIRDEF static
#else
#ifdef __cplusplus
#define STBIRDEF extern "C"
#else
#define STBIRDEF extern
#endif
#endif
#endif

//////////////////////////////////////////////////////////////////////////////
////   start "header file" ///////////////////////////////////////////////////
//
// Easy-to-use API:
//
//     * stride is the offset between successive rows of image data
//        in memory, in bytes (64-bit, so a buffer can be larger than 2 GB).
//        specify 0 for packed continuously in memory
//     * colorspace is linear or sRGB as specified by function name
//     * Uses the default filters
//     * Uses edge mode clamped
//     * returned result is 1 for success or 0 in case of an error.


// stbir_pixel_layout specifies:
//   number of channels
//   order of channels
//   whether color is premultiplied by alpha
// for back compatibility, you can cast the old channel count to an stbir_pixel_layout
typedef enum
{
  STBIR_1CHANNEL = 1,
  STBIR_2CHANNEL = 2,
  STBIR_RGB      = 3,               // 3-chan, with order specified (for channel flipping)
  STBIR_BGR      = 0,               // 3-chan, with order specified (for channel flipping)
  STBIR_4CHANNEL = 5,

  STBIR_RGBA = 4,                   // alpha formats, where alpha is NOT premultiplied into color channels
  STBIR_BGRA = 6,
  STBIR_ARGB = 7,
  STBIR_ABGR = 8,
  STBIR_RA   = 9,
  STBIR_AR   = 10,

  STBIR_RGBA_PM = 11,               // alpha formats, where alpha is premultiplied into color channels
  STBIR_BGRA_PM = 12,
  STBIR_ARGB_PM = 13,
  STBIR_ABGR_PM = 14,
  STBIR_RA_PM   = 15,
  STBIR_AR_PM   = 16,

  STBIR_RGBA_NO_AW = 11,            // alpha formats, where NO alpha weighting is applied at all!
  STBIR_BGRA_NO_AW = 12,            //   these are just synonyms for the _PM flags (which also do
  STBIR_ARGB_NO_AW = 13,            //   no alpha weighting). These names just make it more clear
  STBIR_ABGR_NO_AW = 14,            //   for some folks).
  STBIR_RA_NO_AW   = 15,
  STBIR_AR_NO_AW   = 16,

} stbir_pixel_layout;

//===============================================================
//  Simple-complexity API
//
//    If output_pixels is NULL (0), then we will allocate the buffer and return it to you.
//--------------------------------

STBIRDEF unsigned char * stbir_resize_uint8_srgb( const unsigned char *input_pixels , int input_w , int input_h, stbir_int64 input_stride_in_bytes,
                                                        unsigned char *output_pixels, int output_w, int output_h, stbir_int64 output_stride_in_bytes,
                                                        stbir_pixel_layout pixel_type );

STBIRDEF unsigned char * stbir_resize_uint8_linear( const unsigned char *input_pixels , int input_w , int input_h, stbir_int64 input_stride_in_bytes,
                                                          unsigned char *output_pixels, int output_w, int output_h, stbir_int64 output_stride_in_bytes,
                                                          stbir_pixel_layout pixel_type );

STBIRDEF float * stbir_resize_float_linear( const float *input_pixels , int input_w , int input_h, stbir_int64 input_stride_in_bytes,
                                                  float *output_pixels, int output_w, int output_h, stbir_int64 output_stride_in_bytes,
                                                  stbir_pixel_layout pixel_type );
//===============================================================

//===============================================================
// Medium-complexity API
//
// This extends the easy-to-use API as follows:
//
//     * Can specify the datatype - U8, U8_SRGB, U16, FLOAT, HALF_FLOAT
//     * Edge wrap can selected explicitly
//     * Filter can be selected explicitly
//--------------------------------

typedef enum
{
  STBIR_EDGE_CLAMP   = 0,
  STBIR_EDGE_REFLECT = 1,
  STBIR_EDGE_WRAP    = 2,  // this edge mode is slower and uses more memory
  STBIR_EDGE_ZERO    = 3,
} stbir_edge;

typedef enum
{
  STBIR_FILTER_DEFAULT            = 0,  // use same filter type that easy-to-use API chooses
  STBIR_FILTER_BOX                = 1,  // A trapezoid w/1-pixel wide ramps, same result as box for integer scale ratios
  STBIR_FILTER_TRIANGLE           = 2,  // On upsampling, produces same results as bilinear texture filtering
  STBIR_FILTER_CUBICBSPLINE       = 3,  // The cubic b-spline (aka Mitchell-Netrevalli with B=1,C=0), gaussian-esque
  STBIR_FILTER_CATMULLROM         = 4,  // An interpolating cubic spline (the default when upsampling)
  STBIR_FILTER_MITCHELL           = 5,  // Mitchell-Netrevalli filter with B=1/3, C=1/3 (the default when downsampling)
  STBIR_FILTER_POINT_SAMPLE       = 6,  // Simple point sampling
  STBIR_FILTER_MITCHELL_QUARTER   = 7,  // Mitchell-Netrevalli with B=1/4, C=3/8 (Robidoux), a little sharper than Mitchell
  STBIR_FILTER_MITCHELL_SIXTH     = 8,  // Mitchell-Netrevalli with B=1/6, C=5/12, sharper still
  STBIR_FILTER_MITCHELL_NEG_SIXTH = 9,  // Mitchell-Netrevalli with B=-1/6, C=7/12, over-sharpening
  STBIR_FILTER_MITCHELL_NEG_THIRD = 10, // Mitchell-Netrevalli with B=-1/3, C=2/3, more over-sharpening
  STBIR_FILTER_LANCZOS4           = 11, // Lanczos with 2 lobes (4 taps at 1:1), sharp, some ringing on hard edges
  STBIR_FILTER_LANCZOS5           = 12, // Lanczos with 2.5 lobes (5 taps at 1:1), very sharp, often ringy
  STBIR_FILTER_MURATORI6          = 13, // Casey Muratori's Bink 2 kernel (1 -4 19 19 -4 1)/32: sharper than Catmull-Rom, stable under repeated half pixel shifts when using 8-bit pixels
  STBIR_FILTER_OTHER              = 14, // User callback specified (keep last - the built-ins are everything below it)
} stbir_filter;

typedef enum
{
  STBIR_TYPE_UINT8            = 0,
  STBIR_TYPE_UINT8_SRGB       = 1,
  STBIR_TYPE_UINT8_SRGB_ALPHA = 2,  // alpha channel, when present, should also be SRGB (this is very unusual)
  STBIR_TYPE_UINT16           = 3,
  STBIR_TYPE_FLOAT            = 4,
  STBIR_TYPE_HALF_FLOAT       = 5
} stbir_datatype;

// medium api
STBIRDEF void *  stbir_resize( const void *input_pixels , int input_w , int input_h, stbir_int64 input_stride_in_bytes,
                                     void *output_pixels, int output_w, int output_h, stbir_int64 output_stride_in_bytes,
                               stbir_pixel_layout pixel_layout, stbir_datatype data_type,
                               stbir_edge edge, stbir_filter filter );
//===============================================================


//===============================================================
// Extended-complexity API
//
// This API exposes all resize functionality.
//
//     * Separate filter types for each axis
//     * Separate edge modes for each axis
//     * Separate input and output data types
//     * Can specify regions with subpixel correctness
//     * Can specify alpha flags
//     * Can specify a memory callback
//     * Can specify a callback data type for pixel input and output
//     * Can be threaded for a single resize
//     * Can be used to resize many frames without recalculating the sampler info
//
//  Use this API as follows:
//     1) Call the stbir_resize_init function on a local STBIR_RESIZE structure
//     2) Call any of the stbir_set functions
//     3) Optionally call stbir_build_samplers() if you are going to resample multiple times
//        with the same input and output dimensions (like resizing video frames)
//     4) Resample by calling stbir_resize_extended().
//     5) Call stbir_free_samplers() if you called stbir_build_samplers()
//--------------------------------


// Types:

// INPUT CALLBACK: this callback is used for input scanlines
typedef void const * stbir_input_callback( void * optional_output, void const * input_ptr, int num_pixels, int x, int y, void * context );

// OUTPUT CALLBACK: this callback is used for output scanlines (or pieces of scanlines - see below)
typedef void stbir_output_callback( void const * output_ptr, int num_pixels, int x, int y, void * context );

// callbacks for user installed filters (there are old and new versions, please use the new versions from now on)
typedef float stbir__kernel_callback( float x, float scale, void * user_data );
typedef float stbir__support_callback( float scale, void * user_data );
typedef stbir_real stbir__kernel_callback_new( stbir_real x, stbir_real scale, void * user_data );
typedef stbir_real stbir__support_callback_new( stbir_real scale, void * user_data );
typedef stbir_real stbir__radius_callback_new( stbir_real scale, stbir_real inv_scale, int is_upsample, void * user_data );

// internal structure with precomputed scaling
typedef struct stbir__info stbir__info;

typedef struct STBIR_RESIZE  // use the stbir_resize_init and stbir_override functions to set these values for future compatibility
{
  void * user_data;
  void const * input_pixels;
  int input_w, input_h;
  stbir_real input_s0, input_t0, input_s1, input_t1;
  int input_s_n0, input_s_n1, input_s_d;   // the input region as fractions, when it was set in pixels: s0 = n0/d, s1 = n1/d (d is 0 when set as reals)
  int input_t_n0, input_t_n1, input_t_d;
  stbir_input_callback * input_cb;
  void * output_pixels;
  int output_w, output_h;
  int output_subx, output_suby, output_subw, output_subh;
  stbir_output_callback * output_cb;
  stbir_int64 input_stride_in_bytes;
  stbir_int64 output_stride_in_bytes;
  int splits;
  int fast_alpha;
  int needs_rebuild;
  int called_alloc;
  stbir_pixel_layout input_pixel_layout_public;
  stbir_pixel_layout output_pixel_layout_public;
  stbir_datatype input_data_type;
  stbir_datatype output_data_type;
  stbir_filter horizontal_filter, vertical_filter;
  stbir_edge horizontal_edge, vertical_edge;
  stbir__kernel_callback * horizontal_filter_kernel; stbir__support_callback * horizontal_filter_support;   // old callback form
  stbir__kernel_callback * vertical_filter_kernel; stbir__support_callback * vertical_filter_support;
  stbir__kernel_callback_new * horizontal_filter_kernel_new; stbir__support_callback_new * horizontal_filter_support_new; stbir__radius_callback_new * horizontal_filter_radius;
  stbir__kernel_callback_new * vertical_filter_kernel_new; stbir__support_callback_new * vertical_filter_support_new; stbir__radius_callback_new * vertical_filter_radius;
  stbir__info * samplers;
} STBIR_RESIZE;

// extended complexity api


// First off, you must ALWAYS call stbir_resize_init on your resize structure before any of the other calls!
STBIRDEF void stbir_resize_init( STBIR_RESIZE * resize,
                                 const void *input_pixels,  int input_w,  int input_h, stbir_int64 input_stride_in_bytes, // stride can be zero
                                       void *output_pixels, int output_w, int output_h, stbir_int64 output_stride_in_bytes, // stride can be zero
                                 stbir_pixel_layout pixel_layout, stbir_datatype data_type );

//===============================================================
// You can update these parameters any time after resize_init and there is no cost
//--------------------------------

STBIRDEF void stbir_set_datatypes( STBIR_RESIZE * resize, stbir_datatype input_type, stbir_datatype output_type );
STBIRDEF void stbir_set_pixel_callbacks( STBIR_RESIZE * resize, stbir_input_callback * input_cb, stbir_output_callback * output_cb );   // no callbacks by default
STBIRDEF void stbir_set_user_data( STBIR_RESIZE * resize, void * user_data );                                               // pass back STBIR_RESIZE* by default
STBIRDEF void stbir_set_buffer_ptrs( STBIR_RESIZE * resize, const void * input_pixels, stbir_int64 input_stride_in_bytes, void * output_pixels, stbir_int64 output_stride_in_bytes );

//===============================================================


//===============================================================
// If you call any of these functions, you will trigger a sampler rebuild!
//--------------------------------

STBIRDEF int stbir_set_pixel_layouts( STBIR_RESIZE * resize, stbir_pixel_layout input_pixel_layout, stbir_pixel_layout output_pixel_layout );  // sets new buffer layouts
STBIRDEF int stbir_set_edgemodes( STBIR_RESIZE * resize, stbir_edge horizontal_edge, stbir_edge vertical_edge );       // CLAMP by default

STBIRDEF int stbir_set_filters( STBIR_RESIZE * resize, stbir_filter horizontal_filter, stbir_filter vertical_filter ); // STBIR_DEFAULT_FILTER_UPSAMPLE/DOWNSAMPLE by default

// Your own filters: per axis, a kernel callback and one of two ways to give its extent (a support or a radius - both
//   on one axis returns 0; all null keeps the built-in filter). All in stbir_real (double unless user overridden to float).
//   the resizer places the taps in stbir_real and rounds each weight to float once, into the coefficient table.
//
//   The kernel returns the weight at x, in the filter's own pixels (output pixels when downsampling, input pixels when
//   upsampling), centered on zero. It is also passed the ratio <= 1 (output / input downsampling, input / output
//   upsampling), for kernels like the box whose shape depends on it.
//
//   The support returns the filter's radius in those same filter pixels, and is passed the same ratio. For most kernels
//   this is a constant (2 for the cubics, 1 for the triangle); the resizer converts it to input pixels itself.
//
//   The radius is the alternative for a kernel whose extent depends on the scale: it returns the radius in INPUT pixels,
//   from scale, inv_scale and the resizer's own up / down decision (is_upsample, 1:1 included), so an exact ratio gets
//   an exact tap count (the built-in box is 0.5 * inv_scale + 0.5).
//
//     static stbir_real my_kernel( stbir_real x, stbir_real scale, void * user ) { ... }
//     static stbir_real my_support( stbir_real scale, void * user ) { return 2.0; }
//     stbir_set_filter_callbacks_new( &resize, my_kernel, my_support, 0, my_kernel, my_support, 0 );
STBIRDEF int stbir_set_filter_callbacks_new( STBIR_RESIZE * resize, stbir__kernel_callback_new * horizontal_filter, stbir__support_callback_new * horizontal_support, stbir__radius_callback_new * horizontal_radius, stbir__kernel_callback_new * vertical_filter, stbir__support_callback_new * vertical_support, stbir__radius_callback_new * vertical_radius );

// Old version of the filter callbacks (for backwards compatibility) - please use stbir_set_filter_callbacks_new for new code.
STBIRDEF int stbir_set_filter_callbacks( STBIR_RESIZE * resize, stbir__kernel_callback * horizontal_filter, stbir__support_callback * horizontal_support, stbir__kernel_callback * vertical_filter, stbir__support_callback * vertical_support );

STBIRDEF int stbir_set_pixel_subrect( STBIR_RESIZE * resize, int subx, int suby, int subw, int subh );        // sets both sub-regions (full regions by default)
STBIRDEF int stbir_set_input_subrect( STBIR_RESIZE * resize, stbir_real s0, stbir_real t0, stbir_real s1, stbir_real t1 );    // sets input sub-region (full region by default). a region past the image is clipped to it: the output pixels mapping outside aren't written
STBIRDEF int stbir_set_output_pixel_subrect( STBIR_RESIZE * resize, int subx, int suby, int subw, int subh ); // sets output sub-region (full region by default)

// when inputting AND outputting non-premultiplied alpha pixels, we use a slower but higher quality technique
//   that fills the zero alpha pixel's RGB values with something plausible.  If you don't care about areas of
//   zero alpha, you can call this function to get about a 25% speed improvement for STBIR_RGBA to STBIR_RGBA
//   types of resizes.
STBIRDEF int stbir_set_non_pm_alpha_speed_over_quality( STBIR_RESIZE * resize, int non_pma_alpha_speed_over_quality );
//===============================================================


//===============================================================
// You can call build_samplers to prebuild all the internal data we need to resample.
//   Then, if you call resize_extended many times with the same resize, you only pay the
//   cost once.
// If you do call build_samplers, you MUST call free_samplers eventually.
//--------------------------------

// This builds the samplers and does one allocation
STBIRDEF int stbir_build_samplers( STBIR_RESIZE * resize );

// You MUST call this, if you call stbir_build_samplers or stbir_build_samplers_with_splits
STBIRDEF void stbir_free_samplers( STBIR_RESIZE * resize );
//===============================================================


// And this is the main function to perform the resize synchronously on one thread.
STBIRDEF int stbir_resize_extended( STBIR_RESIZE * resize );


//===============================================================
// Use these functions for multithreading.
//   1) You call stbir_build_samplers_with_splits first on the main thread
//   2) Then stbir_resize_extended_split on each thread
//   3) stbir_free_samplers when done on the main thread
//--------------------------------

// This will build samplers for threading.
//   You can pass in the number of threads you'd like to use (try_splits).
//   It returns the number of splits (threads) that you can call it with.
//   It might be less: when the image can't be split up that many ways, or when
//   the build's estimate says more threads wouldn't help.

STBIRDEF int stbir_build_samplers_with_splits( STBIR_RESIZE * resize, int try_splits );

// This function does a split of the resizing (you call this fuction for each
// split, on multiple threads). A split is a piece of the output resize pixel space.

// Note that you MUST call stbir_build_samplers_with_splits before stbir_resize_extended_split!

// Usually, you will always call stbir_resize_extended_split with split_start as the thread_index
//   and "1" for the split_count.
// But, if you have a weird situation where you MIGHT want 8 threads, but sometimes
//   only 4 threads, you can use 0,2,4,6 for the split_start's and use "2" for the
//   split_count each time to turn in into a 4 thread resize. (This is unusual).

STBIRDEF int stbir_resize_extended_split( STBIR_RESIZE * resize, int split_start, int split_count );
//===============================================================


//===============================================================
// Pixel Callbacks info:
//--------------------------------

//   The input callback is super flexible - it calls you with the input address
//   of the start of the input row (based on the stride and base pointer), and the
//   x position and pixel count that it needs from that row. It gives you an
//   optional_output pointer that you can fill, or you can just return your own
//   pointer into your own data.
//
//   You can also do conversion from non-supported data types if necessary - in
//   this case, you ignore the input_ptr and just use the x and y parameters to
//   calculate your own input_ptr based on the size of each non-supported pixel.
//   (Something like the third example below.)
//
//   You can also install just an input or just an output callback by setting the
//   callback that you don't want to zero.
//
//     First example, progress: (getting a callback that you can monitor the progress):
//        void const * my_callback( void * optional_output, void const * input_ptr, int num_pixels, int x, int y, void * context )
//        {
//           percentage_done = y / input_height;
//           return ( (char*) input_ptr ) + x * pixel_width_in_bytes;  // use buffer from call (input_ptr is the row start)
//        }
//
//     Next example, copying: (copy from some other buffer or stream):
//        void const * my_callback( void * optional_output, void const * input_ptr, int num_pixels, int x, int y, void * context )
//        {
//           CopyOrStreamData( optional_output, other_data_src, num_pixels * pixel_width_in_bytes );
//           return optional_output;  // return the optional buffer that we filled
//        }
//
//     Third example, input another buffer without copying: (zero-copy from other buffer):
//        void const * my_callback( void * optional_output, void const * input_ptr, int num_pixels, int x, int y, void * context )
//        {
//           void * pixels = ( (char*) other_image_base ) + ( y * other_image_stride ) + ( x * other_pixel_width_in_bytes );
//           return pixels;       // return pointer to your data without copying
//        }
//
//
//   The output callback is considerably simpler - it just calls you so that you can dump
//   out the output pixels. You can also convert to other output types here if you want.
//
//   Note that the resizer can process the output in column slices (to stay in cache), so
//   you are called with pieces of scanlines: num_pixels pixels starting at output pixel x
//   on output row y. x and y are always in the full output image's pixel space (so with an
//   output sub-rect, they include the sub-rect offset). The pieces of a scanline are not
//   delivered in order, so if you want whole scanlines, assemble them yourself with x and y.
//
//   Simple example:
//        void my_output( void const * output_ptr, int num_pixels, int x, int y, void * context )
//        {
//           memcpy( ( (char*) my_output_image ) + y * my_output_stride + x * pixel_width_in_bytes, output_ptr, num_pixels * pixel_width_in_bytes );
//        }
//===============================================================



//===============================================================
// optional built-in profiling API
//--------------------------------

#ifdef STBIR_PROFILE

typedef struct STBIR_PROFILE_INFO
{
  stbir_uint64 total_clocks;

  // how many clocks spent (of total_clocks) in the various resize routines, along with a string description
  //    there are "resize_count" number of zones
  stbir_uint64 clocks[ 8 ];
  char const ** descriptions;

  // count of clocks and descriptions
  stbir_uint32 count;
} STBIR_PROFILE_INFO;

// use after calling stbir_resize_extended (or stbir_build_samplers or stbir_build_samplers_with_splits)
STBIRDEF void stbir_resize_build_profile_info( STBIR_PROFILE_INFO * out_info, STBIR_RESIZE const * resize );

// use after calling stbir_resize_extended
STBIRDEF void stbir_resize_extended_profile_info( STBIR_PROFILE_INFO * out_info, STBIR_RESIZE const * resize );

// use after calling stbir_resize_extended_split
STBIRDEF void stbir_resize_split_profile_info( STBIR_PROFILE_INFO * out_info, STBIR_RESIZE const * resize, int split_start, int split_num );

//===============================================================

#endif


////   end header file   /////////////////////////////////////////////////////
#endif // STBIR_INCLUDE_STB_IMAGE_RESIZE2_H

#if defined(STB_IMAGE_RESIZE_IMPLEMENTATION) || defined(STB_IMAGE_RESIZE2_IMPLEMENTATION)

#ifndef STBIR_ASSERT
#include <assert.h>
#define STBIR_ASSERT(x) assert(x)
#endif

#ifndef STBIR_MALLOC
#include <stdlib.h>
#define STBIR_MALLOC(size,user_data) ((void)(user_data), malloc(size))
#define STBIR_FREE(ptr,user_data)    ((void)(user_data), free(ptr))
// (we used the comma operator to evaluate user_data, to avoid "unused parameter" warnings)
#endif

#ifdef _MSC_VER

#define stbir__inline __forceinline

#else

#define stbir__inline __inline__

// Clang address sanitizer
#if defined(__has_feature)
  #if __has_feature(address_sanitizer) || __has_feature(memory_sanitizer)
    #ifndef STBIR__SEPARATE_ALLOCATIONS
      #define STBIR__SEPARATE_ALLOCATIONS
    #endif
  #endif
#endif

#endif

// GCC and MSVC
#if defined(__SANITIZE_ADDRESS__)
  #ifndef STBIR__SEPARATE_ALLOCATIONS
    #define STBIR__SEPARATE_ALLOCATIONS
  #endif
#endif

// Always turn off automatic FMA use - use STBIR_USE_FMA if you want.
// Otherwise, this is a determinism disaster.
#ifndef STBIR_DONT_CHANGE_FP_CONTRACT  // override in case you don't want this behavior
#if defined(_MSC_VER) && !defined(__clang__)
#if _MSC_VER > 1200
#pragma fp_contract(off)
#endif
#elif defined(__GNUC__) &&  !defined(__clang__)
#pragma GCC optimize("fp-contract=off")
#else
#pragma STDC FP_CONTRACT OFF
#endif
#endif

#ifdef _MSC_VER
#define STBIR__UNUSED(v)  (void)(v)
#else
#define STBIR__UNUSED(v)  (void)sizeof(v)
#endif

#define STBIR__ARRAY_SIZE(a) (sizeof((a))/sizeof((a)[0]))


#ifndef STBIR_DEFAULT_FILTER_UPSAMPLE
#define STBIR_DEFAULT_FILTER_UPSAMPLE    STBIR_FILTER_CATMULLROM
#endif

#ifndef STBIR_DEFAULT_FILTER_DOWNSAMPLE
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE  STBIR_FILTER_MITCHELL
#endif


#ifndef STBIR__HEADER_FILENAME
#define STBIR__HEADER_FILENAME "stb_image_resize2.h"
#endif

// the internal pixel layout enums are in a different order, so we can easily do range comparisons of types
//   the public pixel layout is ordered in a way that if you cast num_channels (1-4) to the enum, you get something sensible
typedef enum
{
  STBIRI_1CHANNEL = 0,
  STBIRI_2CHANNEL = 1,
  STBIRI_RGB      = 2,
  STBIRI_BGR      = 3,
  STBIRI_4CHANNEL = 4,

  STBIRI_RGBA = 5,
  STBIRI_BGRA = 6,
  STBIRI_ARGB = 7,
  STBIRI_ABGR = 8,
  STBIRI_RA   = 9,
  STBIRI_AR   = 10,

  STBIRI_RGBA_PM = 11,
  STBIRI_BGRA_PM = 12,
  STBIRI_ARGB_PM = 13,
  STBIRI_ABGR_PM = 14,
  STBIRI_RA_PM   = 15,
  STBIRI_AR_PM   = 16,
} stbir_internal_pixel_layout;

// define the public pixel layouts to not compile inside the implementation (to avoid accidental use)
#define STBIR_BGR bad_dont_use_in_implementation
#define STBIR_1CHANNEL STBIR_BGR
#define STBIR_2CHANNEL STBIR_BGR
#define STBIR_RGB STBIR_BGR
#define STBIR_RGBA STBIR_BGR
#define STBIR_4CHANNEL STBIR_BGR
#define STBIR_BGRA STBIR_BGR
#define STBIR_ARGB STBIR_BGR
#define STBIR_ABGR STBIR_BGR
#define STBIR_RA STBIR_BGR
#define STBIR_AR STBIR_BGR
#define STBIR_RGBA_PM STBIR_BGR
#define STBIR_BGRA_PM STBIR_BGR
#define STBIR_ARGB_PM STBIR_BGR
#define STBIR_ABGR_PM STBIR_BGR
#define STBIR_RA_PM STBIR_BGR
#define STBIR_AR_PM STBIR_BGR

// must match stbir_datatype
static unsigned char stbir__type_size[] = {
  1,1,1,2,4,2 // STBIR_TYPE_UINT8,STBIR_TYPE_UINT8_SRGB,STBIR_TYPE_UINT8_SRGB_ALPHA,STBIR_TYPE_UINT16,STBIR_TYPE_FLOAT,STBIR_TYPE_HALF_FLOAT
};

// When gathering, the contributors are which source pixels contribute.
// When scattering, the contributors are which destination pixels are contributed to.
typedef struct
{
  int n0; // First contributing pixel
  int n1; // Last contributing pixel
} stbir__contributors;

typedef struct
{
  int lowest;    // First sample index for whole filter
  int highest;   // Last sample index for whole filter
  int widest;    // widest single set of samples for an output
} stbir__filter_extent_info;

typedef struct stbir__scale_info
{
  int input_full_size;
  int output_sub_size;
  stbir_real scale;
  stbir_real inv_scale;
  stbir_real pixel_shift; // starting shift in output pixel space (in pixels)
  int scale_is_rational;
  stbir_uint32 scale_numerator, scale_denominator;
  stbir_real tap_scale_in, tap_scale_out;   // 1 / 2N and 1 / 2D for stbir__tap_offset
  int positions_exact;   // the tap positions come from the exact fraction and an integer shift (stbir__tap_offset)
  int shift_pixels;      // pixel_shift as that integer
  int approximated_drift; // the fraction approximates the scale: how far its far edge can land from the float chain's, in input pixels (0: exact)
} stbir__scale_info;

typedef struct
{
  stbir__contributors * contributors;
  float* coefficients;
  stbir__scale_info scale_info;
  stbir_filter filter_enum;
  stbir__kernel_callback_new * filter_kernel_new;    // the kernel: the built-in's or the user's, or
  stbir__radius_callback_new * filter_radius;        // the filter's extent, one of the three: a radius in input pixels (the box) or
  stbir__support_callback_new * filter_support_new;  //   a support in filter pixels (the other built-ins); the user's, or the built-in's,
  stbir__kernel_callback * filter_kernel_old;        //   an old back compat float kernel
  stbir__support_callback * filter_support_old;      //   or an old back compat support function
  stbir_edge edge;
  int coefficient_width;
  int entry_floats;      // horizontal only: floats per packed column entry (STBIR__COLUMN_ENTRY_FLOATS of coefficient_width)

  // polyphase sharing (see stbir__plan_polyphase_sharing). The coefficient sets for the repeating part of a rational scale
  //   are stored once. 0 = every output has its own set.
  int shared_period;     // coefficient sets in the repeating block
  int shared_prologue;   // outputs with their own coefficient sets before it
  int shared_epilogue;   // outputs with their own coefficient sets after it
  int shared_block_n0;   // n0 of the block's first output (output P), and how far n0 moves per pass over the block
  int shared_block_shift;
  int shared_epilogue_n0; // n0 of the first epilogue output
  int filter_pixel_width;
  int filter_pixel_margin;
  int num_contributors;
  size_t contributors_size;
  size_t coefficients_size;
  stbir__filter_extent_info extent_info;
  int is_gather;  // 0 = scatter, 1 = gather with scale >= 1, 2 = gather with scale < 1
} stbir__sampler;

// horizontal routines combine the contribs and coeffs into one structure - coefficients is actually an array of width floats.
typedef struct
{
  int advance;              // how far the running decode pointer moves for the NEXT column, in floats (already times the channel count)
  int width;                // how many pixels this column reads (n1 - n0 + 1) - only the n-coefficient kernels look at it
  float coefficients[ 1 ];  // width floats long, with a zero pad after for the widths whose kernels load one float past
} stbir__column_entry;

// a column slice is a run of output columns that the whole vertical/horizontal loop is run over at once (instead of
//   full scanlines), so the ring buffer and the temp buffers stay in cache. the coefficients are shared by all the
//   slices, each slice just starts at a different offset into them.
typedef struct
{
  int x0;                                     // first output pixel of the slice
  int width;                                  // output pixels in the slice
  int in_n0, in_n1;                           // input pixels decoded for the slice (in the horizontal sampler's coordinate space - can hang off either edge with WRAP)
  int first_n0;                               // input pixel output x0 starts reading at
} stbir__column_slice;

typedef struct
{
#ifdef STBIR_PROFILE
  union
  {
    struct { stbir_uint64 total, looping, vertical, horizontal, decode, encode, alpha, unalpha; } named;
    stbir_uint64 array[8];
  } profile;
  stbir_uint64 * current_zone_excluded_ptr;
#endif
  float* decode_buffer;

  int ring_buffer_first_scanline;
  int ring_buffer_last_scanline;
  int ring_buffer_begin_index;    // first_scanline is at this index in the ring buffer
  int start_output_y, end_output_y;
  int start_input_y, end_input_y;  // used in scatter only
  int first_column_slice, num_column_slices;  // the column slices this split runs: all of them when the splits are bands of rows, its own range when they are columns

  #ifdef STBIR__SEPARATE_ALLOCATIONS
    float** ring_buffers; // one pointer for each ring buffer
  #else
    float* ring_buffer;  // one big buffer that we index into
  #endif

  float* vertical_buffer;

  // where the buffers above were allocated. they each have a page of slack, and get repositioned within it at
  //   resize time to keep them clear of 4K aliasing with the input rows (see stbir__4K_alias_skew_buffers)
  void* decode_buffer_alloc;
  void* ring_buffer_alloc;
  void* vertical_buffer_alloc;

  char no_cache_straddle[64];
} stbir__per_split_info;

typedef float * stbir__decode_pixels_func( float * decode, ptrdiff_t width_times_channels, void const * input );
typedef void stbir__alpha_weight_func( float * decode_buffer, ptrdiff_t width_times_channels );
typedef void stbir__horizontal_gather_channels_func( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer,
  stbir__column_entry const * column, int entry_stride );
typedef void stbir__alpha_unweight_func(float * encode_buffer, ptrdiff_t width_times_channels );
typedef void stbir__encode_pixels_func( void * output, ptrdiff_t width_times_channels, float const * encode );

struct stbir__info
{
#ifdef STBIR_PROFILE
  union
  {
    struct { stbir_uint64 total, build, alloc, horizontal, vertical, cleanup, scatter; } named;
    stbir_uint64 array[7];
  } profile;
  stbir_uint64 * current_zone_excluded_ptr;
#endif
  stbir__sampler horizontal;
  stbir__sampler vertical;

  void const * input_data;
  void * output_data;

  stbir_int64 input_stride_bytes;
  stbir_int64 output_stride_bytes;
  size_t ring_buffer_length_bytes;   // The length of an individual entry in the ring buffer (there are ring_buffer_num_entries of them)
  int ring_buffer_num_entries;    // Total number of entries in the ring buffer.

  stbir_datatype input_type;
  stbir_datatype output_type;

  stbir_input_callback * in_pixels_cb;
  void * user_data;
  stbir_output_callback * out_pixels_cb;

  stbir__contributors conservative_extents;  // input pixel range that covers everything the horizontal filter can touch (decode buffers are sized from this)

  void * alloced_mem;
  stbir__per_split_info * split_info;  // by default 1, but there will be N of these allocated based on the thread init you did
  stbir__column_slice * column_slices;   // the output columns, left to right (the last slice can be narrower)
  int num_column_slices;
  int split_cols, split_rows;      // the split grid: splits = split_cols column groups x split_rows row bands (see stbir__split_mode)

  stbir__decode_pixels_func * decode_pixels;
  stbir__alpha_weight_func * alpha_weight;
  stbir__horizontal_gather_channels_func * horizontal_gather_channels;
  stbir__horizontal_gather_channels_func * horizontal_gather_period1;   // for the run over a polyphase period 1 set (integer downsamples: one coefficient set for every output), or null
  stbir__alpha_unweight_func * alpha_unweight;
  stbir__encode_pixels_func * encode_pixels;

  int alloc_ring_buffer_num_entries;    // Number of entries in the ring buffer that will be allocated
  int splits; // count of splits

  stbir_internal_pixel_layout input_pixel_layout_internal;
  stbir_internal_pixel_layout output_pixel_layout_internal;

  int input_color_and_type;
  int input_callback_offset_bytes; // where the input callback's temp buffer sits relative to the decode target (see STBIR_INPUT_CALLBACK_PADDING)
  int offset_x, offset_y; // offset within output_data
  int vertical_first;
  int channels;
  int effective_channels; // same as channels, except on RGBA/ARGB (7), or XA/AX (3)
  size_t alloced_total;
};


#define stbir__max_uint8_as_float             255.0f
#define stbir__max_uint16_as_float            65535.0f
#define stbir__max_uint8_as_float_inverted    3.9215689e-03f     // (1.0f/255.0f)
#define stbir__max_uint16_as_float_inverted   1.5259022e-05f     // (1.0f/65535.0f)

// an alpha below this isn't divided by when un-premultiplying - alpha is effectively zero
#define stbir__alpha_too_small_to_divide ((float)1 / (1 << 20) / (1 << 20) / (1 << 20) / (1 << 20) / (1 << 20) / (1 << 20))

// a coefficient set whose sum is within this of 1 isn't renormalized: scaling by 1 / sum would move every float
//   coefficient by less than half an ulp ( 2^-25 of itself ), so each would round straight back - the loop is a no-op
#define stbir__renorm_tolerance ( (stbir_real)1.0 / ( 1 << 26 ) )

// a kernel value this small (either sign) is dropped before the coefficients are normalized
#define stbir__too_small_coefficient ( (stbir_real)1.0 / ( 1 << 26 ) )

// a coefficient set whose sum is under this (either sign) is zeroed instead of normalized
#define stbir__too_small_coefficient_sum ( (stbir_real)1.0 / ( 1 << 16 ) )

// min/max friendly
#define STBIR_CLAMP(x, xmin, xmax) for(;;) { \
  if ( (x) < (xmin) ) (x) = (xmin);     \
  if ( (x) > (xmax) ) (x) = (xmax);     \
  break;                                \
}

static stbir__inline int stbir__min(int a, int b)
{
  return a < b ? a : b;
}

static stbir__inline int stbir__max(int a, int b)
{
  return a > b ? a : b;
}

static float stbir__srgb_uchar_to_linear_float[256] = {
  0.000000f, 0.000304f, 0.000607f, 0.000911f, 0.001214f, 0.001518f, 0.001821f, 0.002125f, 0.002428f, 0.002732f, 0.003035f,
  0.003347f, 0.003677f, 0.004025f, 0.004391f, 0.004777f, 0.005182f, 0.005605f, 0.006049f, 0.006512f, 0.006995f, 0.007499f,
  0.008023f, 0.008568f, 0.009134f, 0.009721f, 0.010330f, 0.010960f, 0.011612f, 0.012286f, 0.012983f, 0.013702f, 0.014444f,
  0.015209f, 0.015996f, 0.016807f, 0.017642f, 0.018500f, 0.019382f, 0.020289f, 0.021219f, 0.022174f, 0.023153f, 0.024158f,
  0.025187f, 0.026241f, 0.027321f, 0.028426f, 0.029557f, 0.030713f, 0.031896f, 0.033105f, 0.034340f, 0.035601f, 0.036889f,
  0.038204f, 0.039546f, 0.040915f, 0.042311f, 0.043735f, 0.045186f, 0.046665f, 0.048172f, 0.049707f, 0.051269f, 0.052861f,
  0.054480f, 0.056128f, 0.057805f, 0.059511f, 0.061246f, 0.063010f, 0.064803f, 0.066626f, 0.068478f, 0.070360f, 0.072272f,
  0.074214f, 0.076185f, 0.078187f, 0.080220f, 0.082283f, 0.084376f, 0.086500f, 0.088656f, 0.090842f, 0.093059f, 0.095307f,
  0.097587f, 0.099899f, 0.102242f, 0.104616f, 0.107023f, 0.109462f, 0.111932f, 0.114435f, 0.116971f, 0.119538f, 0.122139f,
  0.124772f, 0.127438f, 0.130136f, 0.132868f, 0.135633f, 0.138432f, 0.141263f, 0.144128f, 0.147027f, 0.149960f, 0.152926f,
  0.155926f, 0.158961f, 0.162029f, 0.165132f, 0.168269f, 0.171441f, 0.174647f, 0.177888f, 0.181164f, 0.184475f, 0.187821f,
  0.191202f, 0.194618f, 0.198069f, 0.201556f, 0.205079f, 0.208637f, 0.212231f, 0.215861f, 0.219526f, 0.223228f, 0.226966f,
  0.230740f, 0.234551f, 0.238398f, 0.242281f, 0.246201f, 0.250158f, 0.254152f, 0.258183f, 0.262251f, 0.266356f, 0.270498f,
  0.274677f, 0.278894f, 0.283149f, 0.287441f, 0.291771f, 0.296138f, 0.300544f, 0.304987f, 0.309469f, 0.313989f, 0.318547f,
  0.323143f, 0.327778f, 0.332452f, 0.337164f, 0.341914f, 0.346704f, 0.351533f, 0.356400f, 0.361307f, 0.366253f, 0.371238f,
  0.376262f, 0.381326f, 0.386430f, 0.391573f, 0.396755f, 0.401978f, 0.407240f, 0.412543f, 0.417885f, 0.423268f, 0.428691f,
  0.434154f, 0.439657f, 0.445201f, 0.450786f, 0.456411f, 0.462077f, 0.467784f, 0.473532f, 0.479320f, 0.485150f, 0.491021f,
  0.496933f, 0.502887f, 0.508881f, 0.514918f, 0.520996f, 0.527115f, 0.533276f, 0.539480f, 0.545725f, 0.552011f, 0.558340f,
  0.564712f, 0.571125f, 0.577581f, 0.584078f, 0.590619f, 0.597202f, 0.603827f, 0.610496f, 0.617207f, 0.623960f, 0.630757f,
  0.637597f, 0.644480f, 0.651406f, 0.658375f, 0.665387f, 0.672443f, 0.679543f, 0.686685f, 0.693872f, 0.701102f, 0.708376f,
  0.715694f, 0.723055f, 0.730461f, 0.737911f, 0.745404f, 0.752942f, 0.760525f, 0.768151f, 0.775822f, 0.783538f, 0.791298f,
  0.799103f, 0.806952f, 0.814847f, 0.822786f, 0.830770f, 0.838799f, 0.846873f, 0.854993f, 0.863157f, 0.871367f, 0.879622f,
  0.887923f, 0.896269f, 0.904661f, 0.913099f, 0.921582f, 0.930111f, 0.938686f, 0.947307f, 0.955974f, 0.964686f, 0.973445f,
  0.982251f, 0.991102f, 1.0f
};

typedef union
{
  unsigned int u;
  float f;
} stbir__FP32;

// From https://gist.github.com/rygorous/2203834

// the SIMD sRGB encoders index this with a float's bits munged into 912..1015, so this biases it back to the table size
#define STBIR__SRGB8_TAB4_BIAS ((127-13)*8)
#define STBIR__SRGB8_TAB4( index ) fp32_to_srgb8_tab4[ (ptrdiff_t)(index) - STBIR__SRGB8_TAB4_BIAS ]
static const stbir_uint32 fp32_to_srgb8_tab4[104] = {
  0x0073000d, 0x007a000d, 0x0080000d, 0x0087000d, 0x008d000d, 0x0094000d, 0x009a000d, 0x00a1000d,
  0x00a7001a, 0x00b4001a, 0x00c1001a, 0x00ce001a, 0x00da001a, 0x00e7001a, 0x00f4001a, 0x0101001a,
  0x010e0033, 0x01280033, 0x01410033, 0x015b0033, 0x01750033, 0x018f0033, 0x01a80033, 0x01c20033,
  0x01dc0067, 0x020f0067, 0x02430067, 0x02760067, 0x02aa0067, 0x02dd0067, 0x03110067, 0x03440067,
  0x037800ce, 0x03df00ce, 0x044600ce, 0x04ad00ce, 0x051400ce, 0x057b00c5, 0x05dd00bc, 0x063b00b5,
  0x06970158, 0x07420142, 0x07e30130, 0x087b0120, 0x090b0112, 0x09940106, 0x0a1700fc, 0x0a9500f2,
  0x0b0f01cb, 0x0bf401ae, 0x0ccb0195, 0x0d950180, 0x0e56016e, 0x0f0d015e, 0x0fbc0150, 0x10630143,
  0x11070264, 0x1238023e, 0x1357021d, 0x14660201, 0x156601e9, 0x165a01d3, 0x174401c0, 0x182401af,
  0x18fe0331, 0x1a9602fe, 0x1c1502d2, 0x1d7e02ad, 0x1ed4028d, 0x201a0270, 0x21520256, 0x227d0240,
  0x239f0443, 0x25c003fe, 0x27bf03c4, 0x29a10392, 0x2b6a0367, 0x2d1d0341, 0x2ebe031f, 0x304d0300,
  0x31d105b0, 0x34a80555, 0x37520507, 0x39d504c5, 0x3c37048b, 0x3e7c0458, 0x40a8042a, 0x42bd0401,
  0x44c20798, 0x488e071e, 0x4c1c06b6, 0x4f76065d, 0x52a50610, 0x55ac05cc, 0x5892058f, 0x5b590559,
  0x5e0c0a23, 0x631c0980, 0x67db08f6, 0x6c55087f, 0x70940818, 0x74a007bd, 0x787d076c, 0x7c330723,
};

static stbir__inline stbir_uint8 stbir__linear_to_srgb_uchar(float in)
{
  static const stbir__FP32 almostone = { 0x3f7fffff }; // 1-eps
  static const stbir__FP32 minval = { (127-13) << 23 };
  stbir_uint32 tab,bias,scale,t;
  stbir__FP32 f;

  // Clamp to [2^(-13), 1-eps]; these two values map to 0 and 1, respectively.
  // The tests are carefully written so that NaNs map to 0, same as in the reference
  // implementation.
  if (!(in > minval.f)) // written this way to catch NaNs
      return 0;
  if (in > almostone.f)
      return 255;

  // Do the table lookup and unpack bias, scale
  f.f = in;
  tab = fp32_to_srgb8_tab4[(f.u - minval.u) >> 20];
  bias = (tab >> 16) << 9;
  scale = tab & 0xffff;

  // Grab next-highest mantissa bits and perform linear interpolation
  t = (f.u >> 12) & 0xff;
  return (unsigned char) ((bias + scale*t) >> 16);
}

#ifndef STBIR_FORCE_GATHER_FILTER_SCANLINES_AMOUNT
#define STBIR_FORCE_GATHER_FILTER_SCANLINES_AMOUNT 128 // when downsampling and <= 128 scanlines of buffering, use gather (scatter above that - 32 or more to 1 downsampling)
#endif


// every scanline buffer carries this many extra floats past the end. we right justify into the buffer
//   so the conversion can read ahead of the writes. also gives the horizontal kernels a one-float overread
#define STBIR_INPUT_CALLBACK_PADDING 16

#ifdef _M_IX86_FP
#if ( _M_IX86_FP >= 2 ) 
#ifndef STBIR_SSE
#define STBIR_SSE
#endif
#endif
#endif

#ifdef __TINYC__
  // tiny c has no intrinsics yet - this can become a version check if they add them
  #define STBIR_NO_SIMD
#endif

// (ARM64EC defines _M_X64 and _M_AMD64 as well as _M_ARM64EC, so the x64 tests have to leave it out - it is NEON below)
#if ( defined(_x86_64) || defined( __x86_64__ ) || defined( _M_X64 ) || defined(__x86_64) || defined(_M_AMD64) || defined(__SSE2__) || defined(STBIR_SSE) || defined(STBIR_SSE2) ) && !defined(_M_ARM64EC)
  #ifndef STBIR_SSE2
    #define STBIR_SSE2
  #endif
  // AVX-512: the foundation plus the byte/word (BW) and 128/256-bit (VL) extensions. /arch:AVX512 on microsoft, 
  //   -mavx512f -mavx512bw -mavx512vl (or -march=x86-64-v4) on clang and gcc. Also implies AVX2
  #if ( defined(__AVX512F__) && defined(__AVX512BW__) && defined(__AVX512VL__) ) || defined(STBIR_AVX512)
    #ifdef STBIR_NO_AVX512
      #undef STBIR_AVX512
    #else
      #ifndef STBIR_AVX512
        #define STBIR_AVX512
      #endif
      #ifndef STBIR_AVX2
        #define STBIR_AVX2
      #endif
    #endif
  #endif
  #if defined(__AVX__) || defined(STBIR_AVX2)
    #ifndef STBIR_AVX
      #ifndef STBIR_NO_AVX
        #define STBIR_AVX
      #endif
    #endif
  #endif
  #if defined(__AVX2__) || defined(STBIR_AVX2)
    #ifndef STBIR_NO_AVX2
      #ifndef STBIR_AVX2
        #define STBIR_AVX2
      #endif
      #if defined( _MSC_VER ) && !defined(__clang__)
        #ifndef STBIR_FP16C  // FP16C instructions are on all AVX2 cpus, so we can autoselect it here on microsoft - clang needs -mf16c
          #define STBIR_FP16C
        #endif
      #endif
    #endif
  #endif
  #ifdef __F16C__
    #ifndef STBIR_FP16C  // turn on FP16C instructions if the define is set (for clang and gcc)
      #define STBIR_FP16C
    #endif
  #endif
#endif

// 64-bit ARM (MSVC's ARM64EC included): the NEON code has a few AArch64 only forms
#if defined( _M_ARM64 ) || defined( _M_ARM64EC ) || defined( __aarch64__ ) || defined( __arm64__ )
#define STBIR__ARM64
#endif

#if defined( STBIR__ARM64 ) || ((__ARM_NEON_FP & 4) != 0) || defined(__ARM_NEON__)
#ifndef STBIR_NEON
#define STBIR_NEON
#endif
#endif

#if defined(_M_ARM) || defined(__arm__)
#ifdef STBIR_USE_FMA
#undef STBIR_USE_FMA // no FMA for 32-bit arm on MSVC
#endif
#endif

#if defined(__wasm__) && defined(__wasm_simd128__)
#ifndef STBIR_WASM
#define STBIR_WASM
#endif
#endif

// restrict pointers for the output pointers, other loop and unroll control
#if defined( _MSC_VER ) && !defined(__clang__)
  #define STBIR_STREAMOUT_PTR( star ) star __restrict
  #define STBIR_NO_UNROLL( ptr ) __assume(ptr) // this oddly keeps msvc from unrolling a loop
  #if _MSC_VER >= 1900
    #define STBIR_NO_UNROLL_LOOP_START __pragma(loop( no_vector )) 
  #else
    #define STBIR_NO_UNROLL_LOOP_START 
  #endif
#elif defined( __clang__ )
  #define STBIR_STREAMOUT_PTR( star ) star __restrict__
  #define STBIR_NO_UNROLL( ptr ) __asm__ (""::"r"(ptr)) 
  #if ( __clang_major__ >= 4 ) || ( ( __clang_major__ >= 3 ) && ( __clang_minor__ >= 5 ) )
    #define STBIR_NO_UNROLL_LOOP_START _Pragma("clang loop unroll(disable)") _Pragma("clang loop vectorize(disable)")
  #else
    #define STBIR_NO_UNROLL_LOOP_START
  #endif 
#elif defined( __GNUC__ )
  #define STBIR_STREAMOUT_PTR( star ) star __restrict__
  #define STBIR_NO_UNROLL( ptr ) __asm__ (""::"r"(ptr))
  #if __GNUC__ >= 14
    #define STBIR_NO_UNROLL_LOOP_START _Pragma("GCC unroll 0") _Pragma("GCC novector")
  #else
    #define STBIR_NO_UNROLL_LOOP_START
  #endif
  #define STBIR_NO_UNROLL_LOOP_START_INF_FOR
#else
  #define STBIR_STREAMOUT_PTR( star ) star
  #define STBIR_NO_UNROLL( ptr )
  #define STBIR_NO_UNROLL_LOOP_START
#endif

#ifndef STBIR_NO_UNROLL_LOOP_START_INF_FOR
#define STBIR_NO_UNROLL_LOOP_START_INF_FOR STBIR_NO_UNROLL_LOOP_START
#endif

#ifdef STBIR_NO_SIMD // force simd off for whatever reason

// force simd off overrides everything else, so clear it all

#ifdef STBIR_SSE2
#undef STBIR_SSE2
#endif

#ifdef STBIR_AVX
#undef STBIR_AVX
#endif

#ifdef STBIR_NEON
#undef STBIR_NEON
#endif

#ifdef STBIR_AVX2
#undef STBIR_AVX2
#endif

#ifdef STBIR_FP16C
#undef STBIR_FP16C
#endif

#ifdef STBIR_WASM
#undef STBIR_WASM
#endif

#ifdef STBIR_SIMD
#undef STBIR_SIMD
#endif

#else // STBIR_SIMD

#ifdef STBIR_SSE2
  #include <emmintrin.h>

  #define stbir__simdf __m128
  #define stbir__simdi __m128i

  #define stbir_simdi_castf( reg ) _mm_castps_si128(reg)
  #define stbir_simdf_casti( reg ) _mm_castsi128_ps(reg)

  #define stbir__simdf_load( reg, ptr ) (reg) = _mm_loadu_ps( (float const*)(ptr) )
  #define stbir__simdi_load( reg, ptr ) (reg) = _mm_loadu_si128 ( (stbir__simdi const*)(ptr) )
  #define stbir__simdf_load1( out, ptr ) (out) = _mm_load_ss( (float const*)(ptr) )  // top values can be random (not denormal or nan for perf)
  #define stbir__simdf_load1z( out, ptr ) (out) = _mm_load_ss( (float const*)(ptr) )  // top values must be zero
  #define stbir__simdf_frep4( fvar ) _mm_set_ps1( fvar )
  #define stbir__simdf_load1frep4( out, fvar ) (out) = _mm_set_ps1( fvar )
  #define stbir__simdf_load2( out, ptr ) (out) = _mm_castsi128_ps( _mm_loadl_epi64( (__m128i*)(ptr)) ) // top values can be random (not denormal or nan for perf)
  #define stbir__simdf_load2z( out, ptr ) (out) = _mm_castsi128_ps( _mm_loadl_epi64( (__m128i*)(ptr)) ) // top values must be zero
  #define stbir__simdf_load2hmerge( out, reg, ptr ) (out) = _mm_castpd_ps(_mm_loadh_pd( _mm_castps_pd(reg), (double*)(ptr) ))
  // pairwise add of two registers: out = ( a0+a1, a2+a3, b0+b1, b2+b3 ), and the halves version: ( a0+a2, a1+a3, b0+b2, b1+b3 )
  #define stbir__simdf_hadd2( out, a, b ) (out) = _mm_add_ps( _mm_shuffle_ps( a, b, (0<<0)+(2<<2)+(0<<4)+(2<<6) ), _mm_shuffle_ps( a, b, (1<<0)+(3<<2)+(1<<4)+(3<<6) ) )
  #define stbir__simdf_addhalves2( out, a, b ) (out) = _mm_add_ps( _mm_movelh_ps( a, b ), _mm_movehl_ps( b, a ) )

  #define stbir__simdf_zeroP() _mm_setzero_ps()
  #define stbir__simdf_zero( reg ) (reg) = _mm_setzero_ps()

  #define stbir__simdf_store( ptr, reg )  _mm_storeu_ps( (float*)(ptr), reg )
  #define stbir__simdf_store1( ptr, reg ) _mm_store_ss( (float*)(ptr), reg )
  #define stbir__simdf_store2( ptr, reg ) _mm_storel_epi64( (__m128i*)(ptr), _mm_castps_si128(reg) )
  #define stbir__simdf_store2h( ptr, reg ) _mm_storeh_pd( (double*)(ptr), _mm_castps_pd(reg) )

  #define stbir__simdi_store( ptr, reg )  _mm_storeu_si128( (__m128i*)(ptr), reg )
  #define stbir__simdi_store2( ptr, reg ) _mm_storel_epi64( (__m128i*)(ptr), (reg) )

  #define stbir__prefetch( ptr ) _mm_prefetch((char*)(ptr), _MM_HINT_T0 )

  #define stbir__simdi_expand_u8_to_u32(out0,out1,out2,out3,ireg) \
  { \
    stbir__simdi zero = _mm_setzero_si128(); \
    out2 = _mm_unpacklo_epi8( ireg, zero ); \
    out3 = _mm_unpackhi_epi8( ireg, zero ); \
    out0 = _mm_unpacklo_epi16( out2, zero ); \
    out1 = _mm_unpackhi_epi16( out2, zero ); \
    out2 = _mm_unpacklo_epi16( out3, zero ); \
    out3 = _mm_unpackhi_epi16( out3, zero ); \
  }


  #define stbir__simdi_expand_u16_to_u32(out0,out1,ireg) \
  { \
    stbir__simdi zero = _mm_setzero_si128(); \
    out0 = _mm_unpacklo_epi16( ireg, zero ); \
    out1 = _mm_unpackhi_epi16( ireg, zero ); \
  }

  #define stbir__simdf_convert_float_to_i32( i, f ) (i) = _mm_cvttps_epi32(f)
  #define stbir__simdf_convert_float_to_uint8( f ) ((unsigned char)_mm_cvtsi128_si32(_mm_cvttps_epi32(_mm_max_ps(_mm_min_ps(f,STBIR__CONSTF(STBIR_max_uint8_as_float)),_mm_setzero_ps()))))
  #define stbir__simdf_convert_float_to_short( f ) ((unsigned short)_mm_cvtsi128_si32(_mm_cvttps_epi32(_mm_max_ps(_mm_min_ps(f,STBIR__CONSTF(STBIR_max_uint16_as_float)),_mm_setzero_ps()))))

  #define stbir__simdi_to_int( i ) _mm_cvtsi128_si32(i)
  #define stbir__simdi_convert_i32_to_float(out, ireg) (out) = _mm_cvtepi32_ps( ireg )
  #define stbir__simdf_add( out, reg0, reg1 ) (out) = _mm_add_ps( reg0, reg1 )
  #define stbir__simdf_mult( out, reg0, reg1 ) (out) = _mm_mul_ps( reg0, reg1 )
  #define stbir__simdf_mult_mem( out, reg, ptr ) (out) = _mm_mul_ps( reg, _mm_loadu_ps( (float const*)(ptr) ) )
  #define stbir__simdf_mult1_mem( out, reg, ptr ) (out) = _mm_mul_ss( reg, _mm_load_ss( (float const*)(ptr) ) )
  #define stbir__simdf_add_mem( out, reg, ptr ) (out) = _mm_add_ps( reg, _mm_loadu_ps( (float const*)(ptr) ) )

  #ifdef STBIR_USE_FMA           // not on by default to maintain bit identical simd to non-simd
  #include <immintrin.h>
  #define stbir__simdf_madd( out, add, mul1, mul2 ) (out) = _mm_fmadd_ps( mul1, mul2, add )
  #define stbir__simdf_madd_mem( out, add, mul, ptr ) (out) = _mm_fmadd_ps( mul, _mm_loadu_ps( (float const*)(ptr) ), add )
  #define stbir__simdf_madd1_mem( out, add, mul, ptr ) (out) = _mm_fmadd_ss( mul, _mm_load_ss( (float const*)(ptr) ), add )
  #else
  #define stbir__simdf_madd( out, add, mul1, mul2 ) (out) = _mm_add_ps( add, _mm_mul_ps( mul1, mul2 ) )
  #define stbir__simdf_madd_mem( out, add, mul, ptr ) (out) = _mm_add_ps( add, _mm_mul_ps( mul, _mm_loadu_ps( (float const*)(ptr) ) ) )
  #define stbir__simdf_madd1_mem( out, add, mul, ptr ) (out) = _mm_add_ss( add, _mm_mul_ss( mul, _mm_load_ss( (float const*)(ptr) ) ) )
  #endif

  #define stbir__simdf_add1( out, reg0, reg1 ) (out) = _mm_add_ss( reg0, reg1 )

  #define stbir__simdf_and( out, reg0, reg1 ) (out) = _mm_and_ps( reg0, reg1 )

  #define stbir__simdf_min( out, reg0, reg1 ) (out) = _mm_min_ps( reg0, reg1 )
  #define stbir__simdf_max( out, reg0, reg1 ) (out) = _mm_max_ps( reg0, reg1 )

  #define stbir__simdf_0123ABCDto3ABx( out, reg0, reg1 ) (out)=_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( _mm_shuffle_ps( reg1,reg0, (0<<0) + (1<<2) + (2<<4) + (3<<6) )), (3<<0) + (0<<2) + (1<<4) + (2<<6) ) )
  #define stbir__simdf_0123ABCDto23Ax( out, reg0, reg1 ) (out)=_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( _mm_shuffle_ps( reg1,reg0, (0<<0) + (1<<2) + (2<<4) + (3<<6) )), (2<<0) + (3<<2) + (0<<4) + (1<<6) ) )

  static const stbir__simdf STBIR_zeroones = { 0.0f,1.0f,0.0f,1.0f };
  static const stbir__simdf STBIR_onezeros = { 1.0f,0.0f,1.0f,0.0f };
  #define stbir__simdf_aaa1( out, alp, ones ) (out)=_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( _mm_movehl_ps( ones, alp ) ), (1<<0) + (1<<2) + (1<<4) + (2<<6) ) )
  #define stbir__simdf_1aaa( out, alp, ones ) (out)=_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( _mm_movelh_ps( ones, alp ) ), (0<<0) + (2<<2) + (2<<4) + (2<<6) ) )
  #define stbir__simdf_a1a1( out, alp, ones) (out) = _mm_or_ps( _mm_castsi128_ps( _mm_srli_epi64( _mm_castps_si128(alp), 32 ) ), STBIR_zeroones )
  #define stbir__simdf_1a1a( out, alp, ones) (out) = _mm_or_ps( _mm_castsi128_ps( _mm_slli_epi64( _mm_castps_si128(alp), 32 ) ), STBIR_onezeros )

  #define stbir__simdf_swiz( reg, one, two, three, four ) _mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( reg ), (one<<0) + (two<<2) + (three<<4) + (four<<6) ) )

  #define stbir__simdi_and( out, reg0, reg1 ) (out) = _mm_and_si128( reg0, reg1 )
  #define stbir__simdi_or( out, reg0, reg1 ) (out) = _mm_or_si128( reg0, reg1 )
  #define stbir__simdi_16madd( out, reg0, reg1 ) (out) = _mm_madd_epi16( reg0, reg1 )

  #define stbir__simdf_pack_to_8bytes(out,aa,bb) \
  { \
    stbir__simdf af,bf; \
    stbir__simdi a,b; \
    af = _mm_min_ps( aa, STBIR_max_uint8_as_float ); \
    bf = _mm_min_ps( bb, STBIR_max_uint8_as_float ); \
    af = _mm_max_ps( af, _mm_setzero_ps() ); \
    bf = _mm_max_ps( bf, _mm_setzero_ps() ); \
    a = _mm_cvttps_epi32( af ); \
    b = _mm_cvttps_epi32( bf ); \
    a = _mm_packs_epi32( a, b ); \
    out = _mm_packus_epi16( a, a ); \
  }

  #define stbir__simdf_load4_transposed( o0, o1, o2, o3, ptr ) \
      stbir__simdf_load( o0, (ptr) );    \
      stbir__simdf_load( o1, (ptr)+4 );  \
      stbir__simdf_load( o2, (ptr)+8 );  \
      stbir__simdf_load( o3, (ptr)+12 ); \
      {                                  \
        __m128 tmp0, tmp1, tmp2, tmp3;   \
        tmp0 = _mm_unpacklo_ps(o0, o1);  \
        tmp2 = _mm_unpacklo_ps(o2, o3);  \
        tmp1 = _mm_unpackhi_ps(o0, o1);  \
        tmp3 = _mm_unpackhi_ps(o2, o3);  \
        o0 = _mm_movelh_ps(tmp0, tmp2);  \
        o1 = _mm_movehl_ps(tmp2, tmp0);  \
        o2 = _mm_movelh_ps(tmp1, tmp3);  \
        o3 = _mm_movehl_ps(tmp3, tmp1);  \
      }

  #define stbir__interleave_pack_and_store_16_u8( ptr, r0, r1, r2, r3 ) \
      r0 = _mm_packs_epi32( r0, r1 ); \
      r2 = _mm_packs_epi32( r2, r3 ); \
      r1 = _mm_unpacklo_epi16( r0, r2 ); \
      r3 = _mm_unpackhi_epi16( r0, r2 ); \
      r0 = _mm_unpacklo_epi16( r1, r3 ); \
      r2 = _mm_unpackhi_epi16( r1, r3 ); \
      r0 = _mm_packus_epi16( r0, r2 ); \
      stbir__simdi_store( ptr, r0 ); \

  #define stbir__simdi_32shr( out, reg, imm ) out = _mm_srli_epi32( reg, imm )

  #if defined(_MSC_VER) && !defined(__clang__)
    // msvc inits with 8 bytes
    #define STBIR__CONST_32_TO_8( v ) (char)(unsigned char)((v)&255),(char)(unsigned char)(((v)>>8)&255),(char)(unsigned char)(((v)>>16)&255),(char)(unsigned char)(((v)>>24)&255)
    #define STBIR__CONST_4_32i( v ) STBIR__CONST_32_TO_8( v ), STBIR__CONST_32_TO_8( v ), STBIR__CONST_32_TO_8( v ), STBIR__CONST_32_TO_8( v )
    #define STBIR__CONST_4d_32i( v0, v1, v2, v3 ) STBIR__CONST_32_TO_8( v0 ), STBIR__CONST_32_TO_8( v1 ), STBIR__CONST_32_TO_8( v2 ), STBIR__CONST_32_TO_8( v3 )
  #else
    // everything else inits with long long's
    #define STBIR__CONST_4_32i( v ) (long long)((((stbir_uint64)(stbir_uint32)(v))<<32)|((stbir_uint64)(stbir_uint32)(v))),(long long)((((stbir_uint64)(stbir_uint32)(v))<<32)|((stbir_uint64)(stbir_uint32)(v)))
    #define STBIR__CONST_4d_32i( v0, v1, v2, v3 ) (long long)((((stbir_uint64)(stbir_uint32)(v1))<<32)|((stbir_uint64)(stbir_uint32)(v0))),(long long)((((stbir_uint64)(stbir_uint32)(v3))<<32)|((stbir_uint64)(stbir_uint32)(v2)))
  #endif

  #define STBIR__SIMDF_CONST(var, x) stbir__simdf var = { x, x, x, x }
  #define STBIR__SIMDI_CONST(var, x) stbir__simdi var = { STBIR__CONST_4_32i(x) }
  #define STBIR__SIMDI_CONST4(var, a, b, c, d) stbir__simdi var = { STBIR__CONST_4d_32i(a, b, c, d) }
  #define STBIR__CONSTF(var) (var)
  #define STBIR__CONSTI(var) (var)

  // an arbitrary byte permute (pshufb) needs SSSE3
  #if defined(STBIR_AVX) || defined(__SSSE3__) || defined(__SSE4_1__)
    #include <tmmintrin.h>
    #define STBIR_SIMD_BYTE_SHUFFLE
    #define stbir__simdi_shuffle_bytes( out, reg, table ) (out) = _mm_shuffle_epi8( (reg), (table) )
  #endif

  #if defined(STBIR_AVX) || defined(__SSE4_1__)
    #include <smmintrin.h>
    #define stbir__simdf_pack_to_8words(out,reg0,reg1) out = _mm_packus_epi32(_mm_cvttps_epi32(_mm_max_ps(_mm_min_ps(reg0,STBIR__CONSTF(STBIR_max_uint16_as_float)),_mm_setzero_ps())), _mm_cvttps_epi32(_mm_max_ps(_mm_min_ps(reg1,STBIR__CONSTF(STBIR_max_uint16_as_float)),_mm_setzero_ps())))
  #else
    static STBIR__SIMDI_CONST(stbir__s32_32768, 32768);
    static STBIR__SIMDI_CONST(stbir__s16_32768, ((32768<<16)|32768));

    #define stbir__simdf_pack_to_8words(out,reg0,reg1) \
      { \
        stbir__simdi tmp0,tmp1; \
        tmp0 = _mm_cvttps_epi32(_mm_max_ps(_mm_min_ps(reg0,STBIR__CONSTF(STBIR_max_uint16_as_float)),_mm_setzero_ps())); \
        tmp1 = _mm_cvttps_epi32(_mm_max_ps(_mm_min_ps(reg1,STBIR__CONSTF(STBIR_max_uint16_as_float)),_mm_setzero_ps())); \
        tmp0 = _mm_sub_epi32( tmp0, stbir__s32_32768 ); \
        tmp1 = _mm_sub_epi32( tmp1, stbir__s32_32768 ); \
        out = _mm_packs_epi32( tmp0, tmp1 ); \
        out = _mm_sub_epi16( out, stbir__s16_32768 ); \
      }

  #endif

  #define STBIR_SIMD

  // if we detect AVX, set the simd8 defines
  #ifdef STBIR_AVX
    #include <immintrin.h>
    #define STBIR_SIMD8
    #define stbir__simdf8 __m256
    #define stbir__simdi8 __m256i
    #define stbir__simdf8_load( out, ptr ) (out) = _mm256_loadu_ps( (float const *)(ptr) )
    #define stbir__simdi8_load( out, ptr ) (out) = _mm256_loadu_si256( (__m256i const *)(ptr) )
    #define stbir__simdf8_mult( out, a, b ) (out) = _mm256_mul_ps( (a), (b) )
    #define stbir__simdf8_store( ptr, out ) _mm256_storeu_ps( (float*)(ptr), out )
    #define stbir__simdi8_store( ptr, reg )  _mm256_storeu_si256( (__m256i*)(ptr), reg )
    #define stbir__simdf8_frep8( fval ) _mm256_set1_ps( fval )

    #define stbir__simdf8_min( out, reg0, reg1 ) (out) = _mm256_min_ps( reg0, reg1 )
    #define stbir__simdf8_max( out, reg0, reg1 ) (out) = _mm256_max_ps( reg0, reg1 )

    #define stbir__simdf8_add4halves( out, bot4, top8 ) (out) = _mm_add_ps( bot4, _mm256_extractf128_ps( top8, 1 ) )
    #define stbir__simdf8_mult_mem( out, reg, ptr ) (out) = _mm256_mul_ps( reg, _mm256_loadu_ps( (float const*)(ptr) ) )
    #define stbir__simdf8_add_mem( out, reg, ptr ) (out) = _mm256_add_ps( reg, _mm256_loadu_ps( (float const*)(ptr) ) )
    #define stbir__simdf8_add( out, a, b ) (out) = _mm256_add_ps( a, b )
    #define stbir__simdf8_load1b( out, ptr ) (out) = _mm256_broadcast_ss( ptr )
    #define stbir__simdf_load1rep4( out, ptr ) (out) = _mm_broadcast_ss( ptr )  // avx load instruction

    #define stbir__simdi8_convert_i32_to_float(out, ireg) (out) = _mm256_cvtepi32_ps( ireg )
    #define stbir__simdf8_convert_float_to_i32( i, f ) (i) = _mm256_cvttps_epi32(f)

    #define stbir__simdf8_bot4s( out, a, b ) (out) = _mm256_permute2f128_ps(a,b, (0<<0)+(2<<4) )
    #define stbir__simdf8_top4s( out, a, b ) (out) = _mm256_permute2f128_ps(a,b, (1<<0)+(3<<4) )

    #define stbir__simdf8_gettop4( reg ) _mm256_extractf128_ps(reg,1)

    #ifdef STBIR_AVX2

    // vpmovzxbd takes eight bytes to eight dwords in one shuffle, so sixteen bytes are three (the high eight moved down first)
    #define stbir__simdi8_expand_u8_to_u32(out0,out1,ireg) \
    { \
      out0 = _mm256_cvtepu8_epi32( ireg ); \
      out1 = _mm256_cvtepu8_epi32( _mm_unpackhi_epi64( ireg, ireg ) ); \
    }

    #define stbir__simdf8_pack_to_16bytes(out,aa,bb) \
    { \
      stbir__simdi8 t; \
      stbir__simdf8 af,bf; \
      stbir__simdi8 a,b; \
      af = _mm256_min_ps( aa, STBIR_max_uint8_as_float8 ); \
      bf = _mm256_min_ps( bb, STBIR_max_uint8_as_float8 ); \
      af = _mm256_max_ps( af, _mm256_setzero_ps() ); \
      bf = _mm256_max_ps( bf, _mm256_setzero_ps() ); \
      a = _mm256_cvttps_epi32( af ); \
      b = _mm256_cvttps_epi32( bf ); \
      t = _mm256_permute4x64_epi64( _mm256_packs_epi32( a, b ), (0<<0)+(2<<2)+(1<<4)+(3<<6) ); \
      out = _mm256_castsi256_si128( _mm256_permute4x64_epi64( _mm256_packus_epi16( t, t ), (0<<0)+(2<<2)+(1<<4)+(3<<6) ) ); \
    }

    #define stbir__simdi8_expand_u16_to_u32(out,ireg) out = _mm256_cvtepu16_epi32( ireg );

    // a byte permute of 32 bytes by one 16 byte table, per 128-bit lane (vpshufb ymm)
    #define stbir__simdi8_shuffle_bytes( out, reg, table ) (out) = _mm256_shuffle_epi8( (reg), _mm256_broadcastsi128_si256( table ) )

    #define stbir__simdf8_pack_to_16words(out,aa,bb) \
      { \
        stbir__simdf8 af,bf; \
        stbir__simdi8 a,b; \
        af = _mm256_min_ps( aa, STBIR_max_uint16_as_float8 ); \
        bf = _mm256_min_ps( bb, STBIR_max_uint16_as_float8 ); \
        af = _mm256_max_ps( af, _mm256_setzero_ps() ); \
        bf = _mm256_max_ps( bf, _mm256_setzero_ps() ); \
        a = _mm256_cvttps_epi32( af ); \
        b = _mm256_cvttps_epi32( bf ); \
        (out) = _mm256_permute4x64_epi64( _mm256_packus_epi32(a, b), (0<<0)+(2<<2)+(1<<4)+(3<<6) ); \
      }

    #else

    #define stbir__simdi8_expand_u8_to_u32(out0,out1,ireg) \
    { \
      stbir__simdi a,zero = _mm_setzero_si128(); \
      a = _mm_unpacklo_epi8( ireg, zero ); \
      out0 = _mm256_setr_m128i( _mm_unpacklo_epi16( a, zero ), _mm_unpackhi_epi16( a, zero ) ); \
      a = _mm_unpackhi_epi8( ireg, zero ); \
      out1 = _mm256_setr_m128i( _mm_unpacklo_epi16( a, zero ), _mm_unpackhi_epi16( a, zero ) ); \
    }

    #define stbir__simdf8_pack_to_16bytes(out,aa,bb) \
    { \
      stbir__simdi t; \
      stbir__simdf8 af,bf; \
      stbir__simdi8 a,b; \
      af = _mm256_min_ps( aa, STBIR_max_uint8_as_float8 ); \
      bf = _mm256_min_ps( bb, STBIR_max_uint8_as_float8 ); \
      af = _mm256_max_ps( af, _mm256_setzero_ps() ); \
      bf = _mm256_max_ps( bf, _mm256_setzero_ps() ); \
      a = _mm256_cvttps_epi32( af ); \
      b = _mm256_cvttps_epi32( bf ); \
      out = _mm_packs_epi32( _mm256_castsi256_si128(a), _mm256_extractf128_si256( a, 1 ) ); \
      out = _mm_packus_epi16( out, out ); \
      t = _mm_packs_epi32( _mm256_castsi256_si128(b), _mm256_extractf128_si256( b, 1 ) ); \
      t = _mm_packus_epi16( t, t ); \
      out = _mm_castps_si128( _mm_shuffle_ps( _mm_castsi128_ps(out), _mm_castsi128_ps(t), (0<<0)+(1<<2)+(0<<4)+(1<<6) ) ); \
    }

    #define stbir__simdi8_expand_u16_to_u32(out,ireg) \
    { \
      stbir__simdi a,b,zero = _mm_setzero_si128(); \
      a = _mm_unpacklo_epi16( ireg, zero ); \
      b = _mm_unpackhi_epi16( ireg, zero ); \
      out = _mm256_insertf128_si256( _mm256_castsi128_si256( a ), b, 1 ); \
    }

    #define stbir__simdf8_pack_to_16words(out,aa,bb) \
      { \
        stbir__simdi t0,t1; \
        stbir__simdf8 af,bf; \
        stbir__simdi8 a,b; \
        af = _mm256_min_ps( aa, STBIR_max_uint16_as_float8 ); \
        bf = _mm256_min_ps( bb, STBIR_max_uint16_as_float8 ); \
        af = _mm256_max_ps( af, _mm256_setzero_ps() ); \
        bf = _mm256_max_ps( bf, _mm256_setzero_ps() ); \
        a = _mm256_cvttps_epi32( af ); \
        b = _mm256_cvttps_epi32( bf ); \
        t0 = _mm_packus_epi32( _mm256_castsi256_si128(a), _mm256_extractf128_si256( a, 1 ) ); \
        t1 = _mm_packus_epi32( _mm256_castsi256_si128(b), _mm256_extractf128_si256( b, 1 ) ); \
        out = _mm256_setr_m128i( t0, t1 ); \
      }

    #endif

    static __m256i stbir_00001111 = { STBIR__CONST_4d_32i( 0, 0, 0, 0 ), STBIR__CONST_4d_32i( 1, 1, 1, 1 ) };
    #define stbir__simdf8_0123to00001111( out, in ) (out) = _mm256_permutevar_ps ( in, stbir_00001111 )

    static __m256i stbir_22223333 = { STBIR__CONST_4d_32i( 2, 2, 2, 2 ), STBIR__CONST_4d_32i( 3, 3, 3, 3 ) };
    #define stbir__simdf8_0123to22223333( out, in ) (out) = _mm256_permutevar_ps ( in, stbir_22223333 )

    #define stbir__simdf8_0123to2222( out, in ) (out) = stbir__simdf_swiz(_mm256_castps256_ps128(in), 2,2,2,2 )

    #define stbir__simdf8_load4b( out, ptr ) (out) = _mm256_broadcast_ps( (__m128 const *)(ptr) )

    static __m256i stbir_00112233 = { STBIR__CONST_4d_32i( 0, 0, 1, 1 ), STBIR__CONST_4d_32i( 2, 2, 3, 3 ) };
    #define stbir__simdf8_0123to00112233( out, in ) (out) = _mm256_permutevar_ps ( in, stbir_00112233 )
    #define stbir__simdf8_add4( out, a8, b ) (out) = _mm256_add_ps( a8,  _mm256_insertf128_ps( _mm256_setzero_ps(), b, 0 ) )   

    static __m256i stbir_load6 = { STBIR__CONST_4_32i( 0x80000000 ), STBIR__CONST_4d_32i(  0x80000000,  0x80000000, 0, 0 ) };
    #define stbir__simdf8_load6z( out, ptr ) (out) = _mm256_maskload_ps( ptr, stbir_load6 )

    #define stbir__simdf8_0123to00000000( out, in ) (out) =  _mm256_shuffle_ps ( in, in, (0<<0)+(0<<2)+(0<<4)+(0<<6) )
    #define stbir__simdf8_0123to11111111( out, in ) (out) =  _mm256_shuffle_ps ( in, in, (1<<0)+(1<<2)+(1<<4)+(1<<6) )
    #define stbir__simdf8_0123to22222222( out, in ) (out) =  _mm256_shuffle_ps ( in, in, (2<<0)+(2<<2)+(2<<4)+(2<<6) )
    #define stbir__simdf8_0123to33333333( out, in ) (out) =  _mm256_shuffle_ps ( in, in, (3<<0)+(3<<2)+(3<<4)+(3<<6) )
    #define stbir__simdf8_0123to21032103( out, in ) (out) =  _mm256_shuffle_ps ( in, in, (2<<0)+(1<<2)+(0<<4)+(3<<6) )
    #define stbir__simdf8_0123to32103210( out, in ) (out) =  _mm256_shuffle_ps ( in, in, (3<<0)+(2<<2)+(1<<4)+(0<<6) )
    #define stbir__simdf8_0123to12301230( out, in ) (out) =  _mm256_shuffle_ps ( in, in, (1<<0)+(2<<2)+(3<<4)+(0<<6) )
    #define stbir__simdf8_0123to10321032( out, in ) (out) =  _mm256_shuffle_ps ( in, in, (1<<0)+(0<<2)+(3<<4)+(2<<6) )
    #define stbir__simdf8_0123to30123012( out, in ) (out) =  _mm256_shuffle_ps ( in, in, (3<<0)+(0<<2)+(1<<4)+(2<<6) )

    #define stbir__simdf8_0123to11331133( out, in ) (out) =  _mm256_shuffle_ps ( in, in, (1<<0)+(1<<2)+(3<<4)+(3<<6) )
    #define stbir__simdf8_0123to00220022( out, in ) (out) =  _mm256_shuffle_ps ( in, in, (0<<0)+(0<<2)+(2<<4)+(2<<6) )

    #define stbir__simdf8_aaa1( out, alp, ones ) do { (out) = _mm256_blend_ps( alp, ones, (1<<0)+(1<<1)+(1<<2)+(0<<3)+(1<<4)+(1<<5)+(1<<6)+(0<<7)); (out)=_mm256_shuffle_ps( out,out, (3<<0) + (3<<2) + (3<<4) + (0<<6) ); } while(0)
    #define stbir__simdf8_1aaa( out, alp, ones ) do { (out) = _mm256_blend_ps( alp, ones, (0<<0)+(1<<1)+(1<<2)+(1<<3)+(0<<4)+(1<<5)+(1<<6)+(1<<7)); (out)=_mm256_shuffle_ps( out,out, (1<<0) + (0<<2) + (0<<4) + (0<<6) ); } while(0)
    #define stbir__simdf8_a1a1( out, alp, ones) do { (out) = _mm256_blend_ps( alp, ones, (1<<0)+(0<<1)+(1<<2)+(0<<3)+(1<<4)+(0<<5)+(1<<6)+(0<<7)); (out)=_mm256_shuffle_ps( out,out, (1<<0) + (0<<2) + (3<<4) + (2<<6) ); } while(0)
    #define stbir__simdf8_1a1a( out, alp, ones) do { (out) = _mm256_blend_ps( alp, ones, (0<<0)+(1<<1)+(0<<2)+(1<<3)+(0<<4)+(1<<5)+(0<<6)+(1<<7)); (out)=_mm256_shuffle_ps( out,out, (1<<0) + (0<<2) + (3<<4) + (2<<6) ); } while(0)

    #define stbir__simdf8_zero( reg ) (reg) = _mm256_setzero_ps()

    #ifdef STBIR_USE_FMA           // not on by default to maintain bit identical simd to non-simd
    #define stbir__simdf8_madd( out, add, mul1, mul2 ) (out) = _mm256_fmadd_ps( mul1, mul2, add )
    #define stbir__simdf8_madd_mem( out, add, mul, ptr ) (out) = _mm256_fmadd_ps( mul, _mm256_loadu_ps( (float const*)(ptr) ), add )
    #define stbir__simdf8_madd_mem4( out, add, mul, ptr )(out) = _mm256_fmadd_ps( _mm256_setr_m128( mul, _mm_setzero_ps() ), _mm256_setr_m128( _mm_loadu_ps( (float const*)(ptr) ), _mm_setzero_ps() ), add )
    #else
    #define stbir__simdf8_madd( out, add, mul1, mul2 ) (out) = _mm256_add_ps( add, _mm256_mul_ps( mul1, mul2 ) )
    #define stbir__simdf8_madd_mem( out, add, mul, ptr ) (out) = _mm256_add_ps( add, _mm256_mul_ps( mul, _mm256_loadu_ps( (float const*)(ptr) ) ) )
    #define stbir__simdf8_madd_mem4( out, add, mul, ptr )  (out) = _mm256_add_ps( add, _mm256_setr_m128( _mm_mul_ps( mul, _mm_loadu_ps( (float const*)(ptr) ) ), _mm_setzero_ps() ) )
    #endif
    #define stbir__if_simdf8_cast_to_simdf4( val ) _mm256_castps256_ps128( val )

    static const stbir__simdf8 STBIR_max_uint16_as_float_inverted8 = { stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted };
    static const stbir__simdf8 STBIR_max_uint8_as_float_inverted8 = { stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted };
    static const stbir__simdf8 STBIR_ones8 = { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 };
    static const stbir__simdf8 STBIR_simd_point58 = { 0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5 };
    static const stbir__simdf8 STBIR_max_uint8_as_float8 = { stbir__max_uint8_as_float,stbir__max_uint8_as_float,stbir__max_uint8_as_float,stbir__max_uint8_as_float, stbir__max_uint8_as_float,stbir__max_uint8_as_float,stbir__max_uint8_as_float,stbir__max_uint8_as_float };
    static const stbir__simdf8 STBIR_max_uint16_as_float8 = { stbir__max_uint16_as_float,stbir__max_uint16_as_float,stbir__max_uint16_as_float,stbir__max_uint16_as_float, stbir__max_uint16_as_float,stbir__max_uint16_as_float,stbir__max_uint16_as_float,stbir__max_uint16_as_float };

    // the 16-wide set (AVX-512) for the X-wide code.
    #ifdef STBIR_AVX512
    #define STBIR_SIMD16
    #define stbir__simdf16 __m512
    #define stbir__simdi16 __m512i
    #define stbir__simdf16_load( out, ptr ) (out) = _mm512_loadu_ps( (float const *)(ptr) )
    #define stbir__simdi16_load( out, ptr ) (out) = _mm512_loadu_si512( (void const *)(ptr) )
    #define stbir__simdf16_store( ptr, reg ) _mm512_storeu_ps( (float*)(ptr), reg )
    #define stbir__simdi16_store( ptr, reg ) _mm512_storeu_si512( (void*)(ptr), reg )
    #define stbir__simdf16_mult( out, a, b ) (out) = _mm512_mul_ps( (a), (b) )
    #define stbir__simdf16_add_mem( out, reg, ptr ) (out) = _mm512_add_ps( reg, _mm512_loadu_ps( (float const*)(ptr) ) )
    #define stbir__simdf16_min( out, a, b ) (out) = _mm512_min_ps( a, b )
    #define stbir__simdf16_max( out, a, b ) (out) = _mm512_max_ps( a, b )
    #define stbir__simdf16_zero( reg ) (reg) = _mm512_setzero_ps()
    #define stbir__simdf16_frep16( fval ) _mm512_set1_ps( fval )
    #define stbir__simdi16_convert_i32_to_float( out, ireg ) (out) = _mm512_cvtepi32_ps( ireg )
    #define stbir__simdf16_convert_float_to_i32( i, f ) (i) = _mm512_cvttps_epi32( f )
    #define stbir__simdf16_cast_to_simdf4( val ) _mm512_castps512_ps128( val )

    #ifdef STBIR_USE_FMA           // not on by default to maintain bit identical simd to non-simd
    #define stbir__simdf16_madd( out, add, mul1, mul2 ) (out) = _mm512_fmadd_ps( mul1, mul2, add )
    #define stbir__simdf16_madd_mem( out, add, mul, ptr ) (out) = _mm512_fmadd_ps( mul, _mm512_loadu_ps( (float const*)(ptr) ), add )
    #else
    #define stbir__simdf16_madd( out, add, mul1, mul2 ) (out) = _mm512_add_ps( add, _mm512_mul_ps( mul1, mul2 ) )
    #define stbir__simdf16_madd_mem( out, add, mul, ptr ) (out) = _mm512_add_ps( add, _mm512_mul_ps( mul, _mm512_loadu_ps( (float const*)(ptr) ) ) )
    #endif

    // the per lane shuffles: one 4-wide order, applied in every lane
    #define stbir__simdf16_0123to0123( out, in ) (out) = (in)
    #define stbir__simdf16_0123to1230( out, in ) (out) = _mm512_shuffle_ps( in, in, (1<<0)+(2<<2)+(3<<4)+(0<<6) )
    #define stbir__simdf16_0123to2103( out, in ) (out) = _mm512_shuffle_ps( in, in, (2<<0)+(1<<2)+(0<<4)+(3<<6) )
    #define stbir__simdf16_0123to3210( out, in ) (out) = _mm512_shuffle_ps( in, in, (3<<0)+(2<<2)+(1<<4)+(0<<6) )
    #define stbir__simdf16_0123to1032( out, in ) (out) = _mm512_shuffle_ps( in, in, (1<<0)+(0<<2)+(3<<4)+(2<<6) )
    #define stbir__simdf16_0123to3012( out, in ) (out) = _mm512_shuffle_ps( in, in, (3<<0)+(0<<2)+(1<<4)+(2<<6) )

    // the alpha patterns: blend the ones in (the mask repeats per lane), then shuffle each lane
    #define stbir__simdf16_aaa1( out, alp, ones ) do { (out) = _mm512_mask_blend_ps( (__mmask16)0x7777, alp, ones ); (out) = _mm512_shuffle_ps( out, out, (3<<0)+(3<<2)+(3<<4)+(0<<6) ); } while(0)
    #define stbir__simdf16_1aaa( out, alp, ones ) do { (out) = _mm512_mask_blend_ps( (__mmask16)0xeeee, alp, ones ); (out) = _mm512_shuffle_ps( out, out, (1<<0)+(0<<2)+(0<<4)+(0<<6) ); } while(0)
    #define stbir__simdf16_a1a1( out, alp, ones ) do { (out) = _mm512_mask_blend_ps( (__mmask16)0x5555, alp, ones ); (out) = _mm512_shuffle_ps( out, out, (1<<0)+(0<<2)+(3<<4)+(2<<6) ); } while(0)
    #define stbir__simdf16_1a1a( out, alp, ones ) do { (out) = _mm512_mask_blend_ps( (__mmask16)0xaaaa, alp, ones ); (out) = _mm512_shuffle_ps( out, out, (1<<0)+(0<<2)+(3<<4)+(2<<6) ); } while(0)

    // integer conversions: sixteen bytes or words to sixteen dwords in one op, and the saturating narrows back
    #define stbir__simdi16_expand_u8_to_u32( out, ireg ) (out) = _mm512_cvtepu8_epi32( ireg )
    #define stbir__simdi16_expand_u16_to_u32( out, ireg ) (out) = _mm512_cvtepu16_epi32( ireg )

    #define stbir__simdf16_pack_to_32bytes( out, aa, bb ) \
    { \
      stbir__simdf16 af, bf; \
      af = _mm512_min_ps( aa, STBIR_max_uint8_as_float16 ); \
      bf = _mm512_min_ps( bb, STBIR_max_uint8_as_float16 ); \
      af = _mm512_max_ps( af, _mm512_setzero_ps() ); \
      bf = _mm512_max_ps( bf, _mm512_setzero_ps() ); \
      (out) = _mm256_inserti128_si256( _mm256_castsi128_si256( _mm512_cvtusepi32_epi8( _mm512_cvttps_epi32( af ) ) ), _mm512_cvtusepi32_epi8( _mm512_cvttps_epi32( bf ) ), 1 ); \
    }

    // the pack interleaves the two inputs per 128-bit lane, the 64-bit permute puts them back in order
    static const stbir__simdi16 stbir__pack_lanes_in_order16 = { STBIR__CONST_4d_32i( 0,0, 2,0 ), STBIR__CONST_4d_32i( 4,0, 6,0 ), STBIR__CONST_4d_32i( 1,0, 3,0 ), STBIR__CONST_4d_32i( 5,0, 7,0 ) };   // 64-bit indexes 0 2 4 6 1 3 5 7
    #define stbir__simdf16_pack_to_32words( out, aa, bb ) \
    { \
      stbir__simdf16 af, bf; \
      af = _mm512_min_ps( aa, STBIR_max_uint16_as_float16 ); \
      bf = _mm512_min_ps( bb, STBIR_max_uint16_as_float16 ); \
      af = _mm512_max_ps( af, _mm512_setzero_ps() ); \
      bf = _mm512_max_ps( bf, _mm512_setzero_ps() ); \
      (out) = _mm512_permutexvar_epi64( stbir__pack_lanes_in_order16, _mm512_packus_epi32( _mm512_cvttps_epi32( af ), _mm512_cvttps_epi32( bf ) ) ); \
    }

    static const stbir__simdf16 STBIR_ones16 = { 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f,1.0f };
    static const stbir__simdf16 STBIR_simd_point516 = { 0.5f,0.5f,0.5f,0.5f, 0.5f,0.5f,0.5f,0.5f, 0.5f,0.5f,0.5f,0.5f, 0.5f,0.5f,0.5f,0.5f };
    static const stbir__simdf16 STBIR_max_uint8_as_float16 = { 255.0f,255.0f,255.0f,255.0f, 255.0f,255.0f,255.0f,255.0f, 255.0f,255.0f,255.0f,255.0f, 255.0f,255.0f,255.0f,255.0f };
    static const stbir__simdf16 STBIR_max_uint16_as_float16 = { 65535.0f,65535.0f,65535.0f,65535.0f, 65535.0f,65535.0f,65535.0f,65535.0f, 65535.0f,65535.0f,65535.0f,65535.0f, 65535.0f,65535.0f,65535.0f,65535.0f };
    static const stbir__simdf16 STBIR_max_uint8_as_float_inverted16 = { stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted, stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted, stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted, stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted,stbir__max_uint8_as_float_inverted };
    static const stbir__simdf16 STBIR_max_uint16_as_float_inverted16 = { stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted, stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted, stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted, stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted,stbir__max_uint16_as_float_inverted };
    #endif  // STBIR_AVX512

  #endif

#elif defined(STBIR_NEON)

  #include <arm_neon.h>

  #define stbir__simdf float32x4_t
  #define stbir__simdi uint32x4_t

  #define stbir_simdi_castf( reg ) vreinterpretq_u32_f32(reg)
  #define stbir_simdf_casti( reg ) vreinterpretq_f32_u32(reg)

  #define stbir__simdf_load( reg, ptr ) (reg) = vld1q_f32( (float const*)(ptr) )
  #define stbir__simdi_load( reg, ptr ) (reg) = vld1q_u32( (uint32_t const*)(ptr) )
  #define stbir__simdf_load1( out, ptr ) (out) = vld1q_lane_f32( (float const*)(ptr), vdupq_n_f32(0), 0 ) // top values zero, like SSE (a replicated inf would turn into nan against the zero coefficient lanes)
  #define stbir__simdf_load1z( out, ptr ) (out) = vld1q_lane_f32( (float const*)(ptr), vdupq_n_f32(0), 0 ) // top values must be zero
  #define stbir__simdf_frep4( fvar ) vdupq_n_f32( fvar )
  #define stbir__simdf_load1frep4( out, fvar ) (out) = vdupq_n_f32( fvar )
  #define stbir__simdf_load2( out, ptr ) (out) = vcombine_f32( vld1_f32( (float const*)(ptr) ), vcreate_f32(0) ) // top values can be random (not denormal or nan for perf)
  #define stbir__simdf_load2z( out, ptr ) (out) = vcombine_f32( vld1_f32( (float const*)(ptr) ), vcreate_f32(0) )  // top values must be zero
  #define stbir__simdf_load2hmerge( out, reg, ptr ) (out) = vcombine_f32( vget_low_f32(reg), vld1_f32( (float const*)(ptr) ) )
  // pairwise add of two registers: out = ( a0+a1, a2+a3, b0+b1, b2+b3 ), and the halves version: ( a0+a2, a1+a3, b0+b2, b1+b3 )
  #define stbir__simdf_hadd2( out, a, b ) (out) = vcombine_f32( vpadd_f32( vget_low_f32( a ), vget_high_f32( a ) ), vpadd_f32( vget_low_f32( b ), vget_high_f32( b ) ) )
  #define stbir__simdf_addhalves2( out, a, b ) (out) = vaddq_f32( vcombine_f32( vget_low_f32( a ), vget_low_f32( b ) ), vcombine_f32( vget_high_f32( a ), vget_high_f32( b ) ) )

  #define stbir__simdf_zeroP() vdupq_n_f32(0)
  #define stbir__simdf_zero( reg ) (reg) = vdupq_n_f32(0)

  #define stbir__simdf_store( ptr, reg )  vst1q_f32( (float*)(ptr), reg )
  #define stbir__simdf_store1( ptr, reg ) vst1q_lane_f32( (float*)(ptr), reg, 0)
  #define stbir__simdf_store2( ptr, reg ) vst1_f32( (float*)(ptr), vget_low_f32(reg) )
  #define stbir__simdf_store2h( ptr, reg ) vst1_f32( (float*)(ptr), vget_high_f32(reg) )

  #define stbir__simdi_store( ptr, reg )  vst1q_u32( (uint32_t*)(ptr), reg )
  #define stbir__simdi_store2( ptr, reg ) vst1_u32( (uint32_t*)(ptr), vget_low_u32(reg) )

  #define stbir__prefetch( ptr )

  #define stbir__simdi_expand_u8_to_u32(out0,out1,out2,out3,ireg) \
  { \
    uint16x8_t l = vmovl_u8( vget_low_u8 ( vreinterpretq_u8_u32(ireg) ) ); \
    uint16x8_t h = vmovl_u8( vget_high_u8( vreinterpretq_u8_u32(ireg) ) ); \
    out0 = vmovl_u16( vget_low_u16 ( l ) ); \
    out1 = vmovl_u16( vget_high_u16( l ) ); \
    out2 = vmovl_u16( vget_low_u16 ( h ) ); \
    out3 = vmovl_u16( vget_high_u16( h ) ); \
  }


  #define stbir__simdi_expand_u16_to_u32(out0,out1,ireg) \
  { \
    uint16x8_t tmp = vreinterpretq_u16_u32(ireg); \
    out0 = vmovl_u16( vget_low_u16 ( tmp ) ); \
    out1 = vmovl_u16( vget_high_u16( tmp ) ); \
  }

  #define stbir__simdf_convert_float_to_i32( i, f ) (i) = vreinterpretq_u32_s32( vcvtq_s32_f32(f) )
  #define stbir__simdi_to_int( i ) (int)vgetq_lane_u32(i, 0)
  #define stbir__simdf_convert_float_to_uint8( f ) ((unsigned char)vgetq_lane_s32(vcvtq_s32_f32(vmaxq_f32(vminq_f32(f,STBIR__CONSTF(STBIR_max_uint8_as_float)),vdupq_n_f32(0))), 0))
  #define stbir__simdf_convert_float_to_short( f ) ((unsigned short)vgetq_lane_s32(vcvtq_s32_f32(vmaxq_f32(vminq_f32(f,STBIR__CONSTF(STBIR_max_uint16_as_float)),vdupq_n_f32(0))), 0))
  #define stbir__simdi_convert_i32_to_float(out, ireg) (out) = vcvtq_f32_s32( vreinterpretq_s32_u32(ireg) )
  #define stbir__simdf_add( out, reg0, reg1 ) (out) = vaddq_f32( reg0, reg1 )
  #define stbir__simdf_mult( out, reg0, reg1 ) (out) = vmulq_f32( reg0, reg1 )
  #define stbir__simdf_mult_mem( out, reg, ptr ) (out) = vmulq_f32( reg, vld1q_f32( (float const*)(ptr) ) )
  #define stbir__simdf_mult1_mem( out, reg, ptr ) (out) = vmulq_f32( reg, vld1q_dup_f32( (float const*)(ptr) ) )
  #define stbir__simdf_add_mem( out, reg, ptr ) (out) = vaddq_f32( reg, vld1q_f32( (float const*)(ptr) ) )

  #ifdef STBIR_USE_FMA           // not on by default to maintain bit identical simd to non-simd (and also x64 no madd to arm madd)
  #define stbir__simdf_madd( out, add, mul1, mul2 ) (out) = vfmaq_f32( add, mul1, mul2 )
  #define stbir__simdf_madd_mem( out, add, mul, ptr ) (out) = vfmaq_f32( add, mul, vld1q_f32( (float const*)(ptr) ) )
  #define stbir__simdf_madd1_mem( out, add, mul, ptr ) (out) = vfmaq_f32( add, mul, vld1q_dup_f32( (float const*)(ptr) ) )
  #else
  #define stbir__simdf_madd( out, add, mul1, mul2 ) (out) = vaddq_f32( add, vmulq_f32( mul1, mul2 ) )
  #define stbir__simdf_madd_mem( out, add, mul, ptr ) (out) = vaddq_f32( add, vmulq_f32( mul, vld1q_f32( (float const*)(ptr) ) ) )
  #define stbir__simdf_madd1_mem( out, add, mul, ptr ) (out) = vaddq_f32( add, vmulq_f32( mul, vld1q_dup_f32( (float const*)(ptr) ) ) )
  #endif

  #define stbir__simdf_add1( out, reg0, reg1 ) (out) = vaddq_f32( reg0, reg1 )

  #define stbir__simdf_and( out, reg0, reg1 ) (out) = vreinterpretq_f32_u32( vandq_u32( vreinterpretq_u32_f32(reg0), vreinterpretq_u32_f32(reg1) ) )

  // min / max with SSE's nan rule
  #if defined( STBIR__ARM64 )
  #define stbir__simdf_min( out, reg0, reg1 ) (out) = vminnmq_f32( reg0, reg1 )
  #define stbir__simdf_max( out, reg0, reg1 ) (out) = vmaxnmq_f32( reg0, reg1 )
  #else
  #define stbir__simdf_min( out, reg0, reg1 ) (out) = vbslq_f32( vcltq_f32( reg0, reg1 ), reg0, reg1 )
  #define stbir__simdf_max( out, reg0, reg1 ) (out) = vbslq_f32( vcgtq_f32( reg0, reg1 ), reg0, reg1 )
  #endif

  #define stbir__simdf_0123ABCDto3ABx( out, reg0, reg1 ) (out) = vextq_f32( reg0, reg1, 3 )
  #define stbir__simdf_0123ABCDto23Ax( out, reg0, reg1 ) (out) = vextq_f32( reg0, reg1, 2 )

  #define stbir__simdf_a1a1( out, alp, ones ) (out) = vzipq_f32(vuzpq_f32(alp, alp).val[1], ones).val[0]
  #define stbir__simdf_1a1a( out, alp, ones ) (out) = vzipq_f32(ones, vuzpq_f32(alp, alp).val[0]).val[0]

  #if defined( STBIR__ARM64 )

    #define stbir__simdf_aaa1( out, alp, ones ) (out) = vcopyq_laneq_f32(vdupq_n_f32(vgetq_lane_f32(alp, 3)), 3, ones, 3)
    #define stbir__simdf_1aaa( out, alp, ones ) (out) = vcopyq_laneq_f32(vdupq_n_f32(vgetq_lane_f32(alp, 0)), 0, ones, 0)

    #if defined( _MSC_VER ) && !defined(__clang__)
      #define stbir_make16(a,b,c,d) vcombine_u8( \
        vcreate_u8( (4*a+0) | ((4*a+1)<<8) | ((4*a+2)<<16) | ((4*a+3)<<24) | \
          ((stbir_uint64)(4*b+0)<<32) | ((stbir_uint64)(4*b+1)<<40) | ((stbir_uint64)(4*b+2)<<48) | ((stbir_uint64)(4*b+3)<<56)), \
        vcreate_u8( (4*c+0) | ((4*c+1)<<8) | ((4*c+2)<<16) | ((4*c+3)<<24) | \
          ((stbir_uint64)(4*d+0)<<32) | ((stbir_uint64)(4*d+1)<<40) | ((stbir_uint64)(4*d+2)<<48) | ((stbir_uint64)(4*d+3)<<56) ) )

      static stbir__inline uint8x16x2_t stbir_make16x2(float32x4_t rega,float32x4_t regb)
      {
        uint8x16x2_t r = { vreinterpretq_u8_f32(rega), vreinterpretq_u8_f32(regb) };
        return r;
      }
    #else
      #define stbir_make16(a,b,c,d) (uint8x16_t){4*a+0,4*a+1,4*a+2,4*a+3,4*b+0,4*b+1,4*b+2,4*b+3,4*c+0,4*c+1,4*c+2,4*c+3,4*d+0,4*d+1,4*d+2,4*d+3}
      #define stbir_make16x2(a,b) (uint8x16x2_t){{vreinterpretq_u8_f32(a),vreinterpretq_u8_f32(b)}}
    #endif

    #define stbir__simdf_swiz( reg, one, two, three, four ) vreinterpretq_f32_u8( vqtbl1q_u8( vreinterpretq_u8_f32(reg), stbir_make16(one, two, three, four) ) )
    #define stbir__simdf_swiz2( rega, regb, one, two, three, four ) vreinterpretq_f32_u8( vqtbl2q_u8( stbir_make16x2(rega,regb), stbir_make16(one, two, three, four) ) )

    // an arbitrary byte permute
    #define STBIR_SIMD_BYTE_SHUFFLE
    #define stbir__simdi_shuffle_bytes( out, reg, table ) (out) = vreinterpretq_u32_u8( vqtbl1q_u8( vreinterpretq_u8_u32( reg ), vreinterpretq_u8_u32( table ) ) )

    #define stbir__simdi_16madd( out, reg0, reg1 ) \
    { \
      int16x8_t r0 = vreinterpretq_s16_u32(reg0); \
      int16x8_t r1 = vreinterpretq_s16_u32(reg1); \
      int32x4_t tmp0 = vmull_s16( vget_low_s16(r0), vget_low_s16(r1) ); \
      int32x4_t tmp1 = vmull_s16( vget_high_s16(r0), vget_high_s16(r1) ); \
      (out) = vreinterpretq_u32_s32( vpaddq_s32(tmp0, tmp1) ); \
    }

  #else

    #define stbir__simdf_aaa1( out, alp, ones ) (out) = vsetq_lane_f32(1.0f, vdupq_n_f32(vgetq_lane_f32(alp, 3)), 3)
    #define stbir__simdf_1aaa( out, alp, ones ) (out) = vsetq_lane_f32(1.0f, vdupq_n_f32(vgetq_lane_f32(alp, 0)), 0)

    #if defined( _MSC_VER ) && !defined(__clang__)
      static stbir__inline uint8x8x2_t stbir_make8x2(float32x4_t reg)
      {
        uint8x8x2_t r = { { vget_low_u8(vreinterpretq_u8_f32(reg)), vget_high_u8(vreinterpretq_u8_f32(reg)) } };
        return r;
      }
      #define stbir_make8(a,b) vcreate_u8( \
        (4*a+0) | ((4*a+1)<<8) | ((4*a+2)<<16) | ((4*a+3)<<24) | \
        ((stbir_uint64)(4*b+0)<<32) | ((stbir_uint64)(4*b+1)<<40) | ((stbir_uint64)(4*b+2)<<48) | ((stbir_uint64)(4*b+3)<<56) )
    #else
      #define stbir_make8x2(reg) (uint8x8x2_t){ { vget_low_u8(vreinterpretq_u8_f32(reg)), vget_high_u8(vreinterpretq_u8_f32(reg)) } }
      #define stbir_make8(a,b) (uint8x8_t){4*a+0,4*a+1,4*a+2,4*a+3,4*b+0,4*b+1,4*b+2,4*b+3}
    #endif

    #define stbir__simdf_swiz( reg, one, two, three, four ) vreinterpretq_f32_u8( vcombine_u8( \
        vtbl2_u8( stbir_make8x2( reg ), stbir_make8( one, two ) ), \
        vtbl2_u8( stbir_make8x2( reg ), stbir_make8( three, four ) ) ) )

    #define stbir__simdi_16madd( out, reg0, reg1 ) \
    { \
      int16x8_t r0 = vreinterpretq_s16_u32(reg0); \
      int16x8_t r1 = vreinterpretq_s16_u32(reg1); \
      int32x4_t tmp0 = vmull_s16( vget_low_s16(r0), vget_low_s16(r1) ); \
      int32x4_t tmp1 = vmull_s16( vget_high_s16(r0), vget_high_s16(r1) ); \
      int32x2_t out0 = vpadd_s32( vget_low_s32(tmp0), vget_high_s32(tmp0) ); \
      int32x2_t out1 = vpadd_s32( vget_low_s32(tmp1), vget_high_s32(tmp1) ); \
      (out) = vreinterpretq_u32_s32( vcombine_s32(out0, out1) ); \
    }

  #endif

  #define stbir__simdi_and( out, reg0, reg1 ) (out) = vandq_u32( reg0, reg1 )
  #define stbir__simdi_or( out, reg0, reg1 ) (out) = vorrq_u32( reg0, reg1 )

  #define stbir__simdf_pack_to_8bytes(out,aa,bb) \
  { \
    float32x4_t af = vmaxq_f32( vminq_f32(aa,STBIR__CONSTF(STBIR_max_uint8_as_float) ), vdupq_n_f32(0) ); \
    float32x4_t bf = vmaxq_f32( vminq_f32(bb,STBIR__CONSTF(STBIR_max_uint8_as_float) ), vdupq_n_f32(0) ); \
    int16x4_t ai = vqmovn_s32( vcvtq_s32_f32( af ) ); \
    int16x4_t bi = vqmovn_s32( vcvtq_s32_f32( bf ) ); \
    uint8x8_t out8 = vqmovun_s16( vcombine_s16(ai, bi) ); \
    out = vreinterpretq_u32_u8( vcombine_u8(out8, out8) ); \
  }

  #define stbir__simdf_pack_to_8words(out,aa,bb) \
  { \
    float32x4_t af = vmaxq_f32( vminq_f32(aa,STBIR__CONSTF(STBIR_max_uint16_as_float) ), vdupq_n_f32(0) ); \
    float32x4_t bf = vmaxq_f32( vminq_f32(bb,STBIR__CONSTF(STBIR_max_uint16_as_float) ), vdupq_n_f32(0) ); \
    int32x4_t ai = vcvtq_s32_f32( af ); \
    int32x4_t bi = vcvtq_s32_f32( bf ); \
    out = vreinterpretq_u32_u16( vcombine_u16(vqmovun_s32(ai), vqmovun_s32(bi)) ); \
  }

  #define stbir__interleave_pack_and_store_16_u8( ptr, r0, r1, r2, r3 ) \
  { \
    int16x4x2_t tmp0 = vzip_s16( vqmovn_s32(vreinterpretq_s32_u32(r0)), vqmovn_s32(vreinterpretq_s32_u32(r2)) ); \
    int16x4x2_t tmp1 = vzip_s16( vqmovn_s32(vreinterpretq_s32_u32(r1)), vqmovn_s32(vreinterpretq_s32_u32(r3)) ); \
    uint8x8x2_t out = \
    { { \
      vqmovun_s16( vcombine_s16(tmp0.val[0], tmp0.val[1]) ), \
      vqmovun_s16( vcombine_s16(tmp1.val[0], tmp1.val[1]) ), \
    } }; \
    vst2_u8(ptr, out); \
  }

  #define stbir__simdf_load4_transposed( o0, o1, o2, o3, ptr ) \
  { \
    float32x4x4_t tmp = vld4q_f32(ptr); \
    o0 = tmp.val[0]; \
    o1 = tmp.val[1]; \
    o2 = tmp.val[2]; \
    o3 = tmp.val[3]; \
  }

  #define stbir__simdi_32shr( out, reg, imm ) out = vshrq_n_u32( reg, imm )

  #if defined( _MSC_VER ) && !defined(__clang__)
    #define STBIR__SIMDF_CONST(var, x) __declspec(align(8)) float var[] = { x, x, x, x }
    #define STBIR__SIMDI_CONST(var, x) __declspec(align(8)) uint32_t var[] = { x, x, x, x }
    #define STBIR__SIMDI_CONST4(var, a, b, c, d) __declspec(align(8)) uint32_t var[] = { a, b, c, d }
    #define STBIR__CONSTF(var) (*(const float32x4_t*)var)
    #define STBIR__CONSTI(var) (*(const uint32x4_t*)var)
  #else
    #define STBIR__SIMDF_CONST(var, x) stbir__simdf var = { x, x, x, x }
    #define STBIR__SIMDI_CONST(var, x) stbir__simdi var = { x, x, x, x }
    #define STBIR__SIMDI_CONST4(var, a, b, c, d) stbir__simdi var = { a, b, c, d }
    #define STBIR__CONSTF(var) (var)
    #define STBIR__CONSTI(var) (var)
  #endif

  #define STBIR_SIMD

#elif defined(STBIR_WASM)

  #include <wasm_simd128.h>

  #define stbir__simdf v128_t
  #define stbir__simdi v128_t

  #define stbir_simdi_castf( reg ) (reg)
  #define stbir_simdf_casti( reg ) (reg)

  #define stbir__simdf_load( reg, ptr )             (reg) = wasm_v128_load( (void const*)(ptr) )
  #define stbir__simdi_load( reg, ptr )             (reg) = wasm_v128_load( (void const*)(ptr) )
  #define stbir__simdf_load1( out, ptr )            (out) = wasm_v128_load32_zero( (void const*)(ptr) ) // top values zero, like SSE (a replicated inf would turn into nan against the zero coefficient lanes)
  #define stbir__simdf_load1z( out, ptr )           (out) = wasm_v128_load32_zero( (void const*)(ptr) ) // top values must be zero
  #define stbir__simdf_frep4( fvar )                wasm_f32x4_splat( fvar )
  #define stbir__simdf_load1frep4( out, fvar )      (out) = wasm_f32x4_splat( fvar )
  #define stbir__simdf_load2( out, ptr )            (out) = wasm_v128_load64_zero( (void const*)(ptr) ) // top values zero, like SSE
  #define stbir__simdf_load2z( out, ptr )           (out) = wasm_v128_load64_zero( (void const*)(ptr) ) // top values must be zero
  #define stbir__simdf_load2hmerge( out, reg, ptr ) (out) = wasm_v128_load64_lane( (void const*)(ptr), reg, 1 )
// pairwise add of two registers: out = ( a0+a1, a2+a3, b0+b1, b2+b3 ), and the halves version: ( a0+a2, a1+a3, b0+b2, b1+b3 )
#define stbir__simdf_hadd2( out, a, b )            (out) = wasm_f32x4_add( wasm_i32x4_shuffle( a, b, 0, 2, 4, 6 ), wasm_i32x4_shuffle( a, b, 1, 3, 5, 7 ) )
#define stbir__simdf_addhalves2( out, a, b )       (out) = wasm_f32x4_add( wasm_i32x4_shuffle( a, b, 0, 1, 4, 5 ), wasm_i32x4_shuffle( a, b, 2, 3, 6, 7 ) )

  #define stbir__simdf_zeroP() wasm_f32x4_const_splat(0)
  #define stbir__simdf_zero( reg ) (reg) = wasm_f32x4_const_splat(0)

  #define stbir__simdf_store( ptr, reg )   wasm_v128_store( (void*)(ptr), reg )
  #define stbir__simdf_store1( ptr, reg )  wasm_v128_store32_lane( (void*)(ptr), reg, 0 )
  #define stbir__simdf_store2( ptr, reg )  wasm_v128_store64_lane( (void*)(ptr), reg, 0 )
  #define stbir__simdf_store2h( ptr, reg ) wasm_v128_store64_lane( (void*)(ptr), reg, 1 )

  #define stbir__simdi_store( ptr, reg )  wasm_v128_store( (void*)(ptr), reg )
  #define stbir__simdi_store2( ptr, reg ) wasm_v128_store64_lane( (void*)(ptr), reg, 0 )

  #define stbir__prefetch( ptr )

  #define stbir__simdi_expand_u8_to_u32(out0,out1,out2,out3,ireg) \
  { \
    v128_t l = wasm_u16x8_extend_low_u8x16 ( ireg ); \
    v128_t h = wasm_u16x8_extend_high_u8x16( ireg ); \
    out0 = wasm_u32x4_extend_low_u16x8 ( l ); \
    out1 = wasm_u32x4_extend_high_u16x8( l ); \
    out2 = wasm_u32x4_extend_low_u16x8 ( h ); \
    out3 = wasm_u32x4_extend_high_u16x8( h ); \
  }


  #define stbir__simdi_expand_u16_to_u32(out0,out1,ireg) \
  { \
    out0 = wasm_u32x4_extend_low_u16x8 ( ireg ); \
    out1 = wasm_u32x4_extend_high_u16x8( ireg ); \
  }

  #define stbir__simdf_convert_float_to_i32( i, f )    (i) = wasm_i32x4_trunc_sat_f32x4(f)
  #define stbir__simdi_to_int( i )                     wasm_i32x4_extract_lane(i, 0)
  #define stbir__simdf_convert_float_to_uint8( f )     ((unsigned char)wasm_i32x4_extract_lane(wasm_i32x4_trunc_sat_f32x4(wasm_f32x4_max(wasm_f32x4_min(f,STBIR_max_uint8_as_float),wasm_f32x4_const_splat(0))), 0))
  #define stbir__simdf_convert_float_to_short( f )     ((unsigned short)wasm_i32x4_extract_lane(wasm_i32x4_trunc_sat_f32x4(wasm_f32x4_max(wasm_f32x4_min(f,STBIR_max_uint16_as_float),wasm_f32x4_const_splat(0))), 0))
  #define stbir__simdi_convert_i32_to_float(out, ireg) (out) = wasm_f32x4_convert_i32x4(ireg)
  #define stbir__simdf_add( out, reg0, reg1 )          (out) = wasm_f32x4_add( reg0, reg1 )
  #define stbir__simdf_mult( out, reg0, reg1 )         (out) = wasm_f32x4_mul( reg0, reg1 )
  #define stbir__simdf_mult_mem( out, reg, ptr )       (out) = wasm_f32x4_mul( reg, wasm_v128_load( (void const*)(ptr) ) )
  #define stbir__simdf_mult1_mem( out, reg, ptr )      (out) = wasm_f32x4_mul( reg, wasm_v128_load32_splat( (void const*)(ptr) ) )
  #define stbir__simdf_add_mem( out, reg, ptr )        (out) = wasm_f32x4_add( reg, wasm_v128_load( (void const*)(ptr) ) )

  #define stbir__simdf_madd( out, add, mul1, mul2 )    (out) = wasm_f32x4_add( add, wasm_f32x4_mul( mul1, mul2 ) )
  #define stbir__simdf_madd_mem( out, add, mul, ptr )  (out) = wasm_f32x4_add( add, wasm_f32x4_mul( mul, wasm_v128_load( (void const*)(ptr) ) ) )
  #define stbir__simdf_madd1_mem( out, add, mul, ptr ) (out) = wasm_f32x4_add( add, wasm_f32x4_mul( mul, wasm_v128_load32_splat( (void const*)(ptr) ) ) )

  #define stbir__simdf_add1( out, reg0, reg1 )  (out) = wasm_f32x4_add( reg0, reg1 )

  #define stbir__simdf_and( out, reg0, reg1 ) (out) = wasm_v128_and( reg0, reg1 )

  #define stbir__simdf_min( out, reg0, reg1 ) (out) = wasm_f32x4_min( reg0, reg1 )
  #define stbir__simdf_max( out, reg0, reg1 ) (out) = wasm_f32x4_max( reg0, reg1 )

  #define stbir__simdf_0123ABCDto3ABx( out, reg0, reg1 ) (out) = wasm_i32x4_shuffle( reg0, reg1, 3, 4, 5, -1 )
  #define stbir__simdf_0123ABCDto23Ax( out, reg0, reg1 ) (out) = wasm_i32x4_shuffle( reg0, reg1, 2, 3, 4, -1 )

  #define stbir__simdf_aaa1(out,alp,ones) (out) = wasm_i32x4_shuffle(alp, ones, 3, 3, 3, 4)
  #define stbir__simdf_1aaa(out,alp,ones) (out) = wasm_i32x4_shuffle(alp, ones, 4, 0, 0, 0)
  #define stbir__simdf_a1a1(out,alp,ones) (out) = wasm_i32x4_shuffle(alp, ones, 1, 4, 3, 4)
  #define stbir__simdf_1a1a(out,alp,ones) (out) = wasm_i32x4_shuffle(alp, ones, 4, 0, 4, 2)

  #define stbir__simdf_swiz( reg, one, two, three, four ) wasm_i32x4_shuffle(reg, reg, one, two, three, four)

  // an arbitrary byte permute
  #define STBIR_SIMD_BYTE_SHUFFLE
  #define stbir__simdi_shuffle_bytes( out, reg, table ) (out) = wasm_i8x16_swizzle( reg, table )

  #define stbir__simdi_and( out, reg0, reg1 )    (out) = wasm_v128_and( reg0, reg1 )
  #define stbir__simdi_or( out, reg0, reg1 )     (out) = wasm_v128_or( reg0, reg1 )
  #define stbir__simdi_16madd( out, reg0, reg1 ) (out) = wasm_i32x4_dot_i16x8( reg0, reg1 )

  #define stbir__simdf_pack_to_8bytes(out,aa,bb) \
  { \
    v128_t af = wasm_f32x4_max( wasm_f32x4_min(aa, STBIR_max_uint8_as_float), wasm_f32x4_const_splat(0) ); \
    v128_t bf = wasm_f32x4_max( wasm_f32x4_min(bb, STBIR_max_uint8_as_float), wasm_f32x4_const_splat(0) ); \
    v128_t ai = wasm_i32x4_trunc_sat_f32x4( af ); \
    v128_t bi = wasm_i32x4_trunc_sat_f32x4( bf ); \
    v128_t out16 = wasm_i16x8_narrow_i32x4( ai, bi ); \
    out = wasm_u8x16_narrow_i16x8( out16, out16 ); \
  }

  #define stbir__simdf_pack_to_8words(out,aa,bb) \
  { \
    v128_t af = wasm_f32x4_max( wasm_f32x4_min(aa, STBIR_max_uint16_as_float), wasm_f32x4_const_splat(0)); \
    v128_t bf = wasm_f32x4_max( wasm_f32x4_min(bb, STBIR_max_uint16_as_float), wasm_f32x4_const_splat(0)); \
    v128_t ai = wasm_i32x4_trunc_sat_f32x4( af ); \
    v128_t bi = wasm_i32x4_trunc_sat_f32x4( bf ); \
    out = wasm_u16x8_narrow_i32x4( ai, bi ); \
  }

  #define stbir__interleave_pack_and_store_16_u8( ptr, r0, r1, r2, r3 ) \
  { \
    v128_t tmp0 = wasm_i16x8_narrow_i32x4(r0, r1); \
    v128_t tmp1 = wasm_i16x8_narrow_i32x4(r2, r3); \
    v128_t tmp = wasm_u8x16_narrow_i16x8(tmp0, tmp1); \
    tmp = wasm_i8x16_shuffle(tmp, tmp, 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15); \
    wasm_v128_store( (void*)(ptr), tmp); \
  }

  #define stbir__simdf_load4_transposed( o0, o1, o2, o3, ptr ) \
  { \
    v128_t t0 = wasm_v128_load( ptr    ); \
    v128_t t1 = wasm_v128_load( ptr+4  ); \
    v128_t t2 = wasm_v128_load( ptr+8  ); \
    v128_t t3 = wasm_v128_load( ptr+12 ); \
    v128_t s0 = wasm_i32x4_shuffle(t0, t1, 0, 4, 2, 6); \
    v128_t s1 = wasm_i32x4_shuffle(t0, t1, 1, 5, 3, 7); \
    v128_t s2 = wasm_i32x4_shuffle(t2, t3, 0, 4, 2, 6); \
    v128_t s3 = wasm_i32x4_shuffle(t2, t3, 1, 5, 3, 7); \
    o0 = wasm_i32x4_shuffle(s0, s2, 0, 1, 4, 5); \
    o1 = wasm_i32x4_shuffle(s1, s3, 0, 1, 4, 5); \
    o2 = wasm_i32x4_shuffle(s0, s2, 2, 3, 6, 7); \
    o3 = wasm_i32x4_shuffle(s1, s3, 2, 3, 6, 7); \
  }

  #define stbir__simdi_32shr( out, reg, imm ) out = wasm_u32x4_shr( reg, imm )

  typedef float stbir__f32x4 __attribute__((__vector_size__(16), __aligned__(16)));
  #define STBIR__SIMDF_CONST(var, x) stbir__simdf var = (v128_t)(stbir__f32x4){ x, x, x, x }
  #define STBIR__SIMDI_CONST(var, x) stbir__simdi var = { x, x, x, x }
  #define STBIR__SIMDI_CONST4(var, a, b, c, d) stbir__simdi var = { a, b, c, d }
  #define STBIR__CONSTF(var) (var)
  #define STBIR__CONSTI(var) (var)

  #define STBIR_SIMD

#endif  // SSE2/NEON/WASM

#endif // NO SIMD

// the X-wide names
#if defined(STBIR_SIMD16)
  #define STBIR__U16_DECODE_BLOCK 16   // words per block in the u16 decoders (one 32 byte load)
  #define stbir__simdfX stbir__simdf16
  #define stbir__simdiX stbir__simdi16
  #define stbir__simdfX_load stbir__simdf16_load
  #define stbir__simdiX_load stbir__simdi16_load
  #define stbir__simdfX_mult stbir__simdf16_mult
  #define stbir__simdfX_add_mem stbir__simdf16_add_mem
  #define stbir__simdfX_madd_mem stbir__simdf16_madd_mem
  #define stbir__simdfX_store stbir__simdf16_store
  #define stbir__simdiX_store stbir__simdi16_store
  #define stbir__simdf_frepX  stbir__simdf16_frep16
  #define stbir__simdfX_madd stbir__simdf16_madd
  #define stbir__simdfX_min stbir__simdf16_min
  #define stbir__simdfX_max stbir__simdf16_max
  #define stbir__simdfX_aaa1 stbir__simdf16_aaa1
  #define stbir__simdfX_1aaa stbir__simdf16_1aaa
  #define stbir__simdfX_a1a1 stbir__simdf16_a1a1
  #define stbir__simdfX_1a1a stbir__simdf16_1a1a
  #define stbir__simdfX_convert_float_to_i32 stbir__simdf16_convert_float_to_i32
  #define stbir__simdfX_pack_to_words stbir__simdf16_pack_to_32words
  #define stbir__simdfX_zero stbir__simdf16_zero
  #define STBIR_onesX STBIR_ones16
  #define STBIR_max_uint8_as_floatX STBIR_max_uint8_as_float16
  #define STBIR_max_uint16_as_floatX STBIR_max_uint16_as_float16
  #define STBIR_simd_point5X STBIR_simd_point516
  #define stbir__simdfX_float_count 16
  #define stbir__simdfX_cast_to_simdf4( val ) stbir__simdf16_cast_to_simdf4( val )
  #define stbir__simdfX_0123to1230 stbir__simdf16_0123to1230
  #define stbir__simdfX_0123to2103 stbir__simdf16_0123to2103
#elif defined(STBIR_SIMD8)
  #define STBIR__U16_DECODE_BLOCK 8
  #define stbir__simdfX stbir__simdf8
  #define stbir__simdiX stbir__simdi8
  #define stbir__simdfX_load stbir__simdf8_load
  #define stbir__simdiX_load stbir__simdi8_load
  #define stbir__simdfX_mult stbir__simdf8_mult
  #define stbir__simdfX_add_mem stbir__simdf8_add_mem
  #define stbir__simdfX_madd_mem stbir__simdf8_madd_mem
  #define stbir__simdfX_store stbir__simdf8_store
  #define stbir__simdiX_store stbir__simdi8_store
  #define stbir__simdf_frepX  stbir__simdf8_frep8
  #define stbir__simdfX_madd stbir__simdf8_madd
  #define stbir__simdfX_min stbir__simdf8_min
  #define stbir__simdfX_max stbir__simdf8_max
  #define stbir__simdfX_aaa1 stbir__simdf8_aaa1
  #define stbir__simdfX_1aaa stbir__simdf8_1aaa
  #define stbir__simdfX_a1a1 stbir__simdf8_a1a1
  #define stbir__simdfX_1a1a stbir__simdf8_1a1a
  #define stbir__simdfX_convert_float_to_i32 stbir__simdf8_convert_float_to_i32
  #define stbir__simdfX_pack_to_words stbir__simdf8_pack_to_16words
  #define stbir__simdfX_zero stbir__simdf8_zero
  #define STBIR_onesX STBIR_ones8
  #define STBIR_max_uint8_as_floatX STBIR_max_uint8_as_float8
  #define STBIR_max_uint16_as_floatX STBIR_max_uint16_as_float8
  #define STBIR_simd_point5X STBIR_simd_point58
  #define stbir__simdfX_float_count 8
  #define stbir__simdfX_cast_to_simdf4( val ) stbir__if_simdf8_cast_to_simdf4( val )
  #define stbir__simdfX_0123to1230 stbir__simdf8_0123to12301230
  #define stbir__simdfX_0123to2103 stbir__simdf8_0123to21032103
#else
  #define STBIR__U16_DECODE_BLOCK 8
  #define stbir__simdfX stbir__simdf
  #define stbir__simdiX stbir__simdi
  #define stbir__simdfX_load stbir__simdf_load
  #define stbir__simdiX_load stbir__simdi_load
  #define stbir__simdfX_mult stbir__simdf_mult
  #define stbir__simdfX_add_mem stbir__simdf_add_mem
  #define stbir__simdfX_madd_mem stbir__simdf_madd_mem
  #define stbir__simdfX_store stbir__simdf_store
  #define stbir__simdiX_store stbir__simdi_store
  #define stbir__simdf_frepX  stbir__simdf_frep4
  #define stbir__simdfX_madd stbir__simdf_madd
  #define stbir__simdfX_min stbir__simdf_min
  #define stbir__simdfX_max stbir__simdf_max
  #define stbir__simdfX_aaa1 stbir__simdf_aaa1
  #define stbir__simdfX_1aaa stbir__simdf_1aaa
  #define stbir__simdfX_a1a1 stbir__simdf_a1a1
  #define stbir__simdfX_1a1a stbir__simdf_1a1a
  #define stbir__simdfX_convert_float_to_i32 stbir__simdf_convert_float_to_i32
  #define stbir__simdfX_pack_to_words stbir__simdf_pack_to_8words
  #define stbir__simdfX_zero stbir__simdf_zero
  #define STBIR_onesX STBIR__CONSTF(STBIR_ones)
  #define STBIR_simd_point5X STBIR__CONSTF(STBIR_simd_point5)
  #define STBIR_max_uint8_as_floatX STBIR__CONSTF(STBIR_max_uint8_as_float)
  #define STBIR_max_uint16_as_floatX STBIR__CONSTF(STBIR_max_uint16_as_float)
  #define stbir__simdfX_float_count 4
  #define stbir__simdfX_cast_to_simdf4( val ) ( val )
  #define stbir__if_simdf8_cast_to_simdf4( val ) ( val )
  #define stbir__simdfX_0123to1230 stbir__simdf_0123to1230
  #define stbir__simdfX_0123to2103 stbir__simdf_0123to2103
#endif


#if defined(STBIR_NEON) && !defined(_M_ARM) && !defined(__arm__)

  #if defined( _MSC_VER ) && !defined(__clang__)
  typedef __int16 stbir__FP16;
  #else
  typedef float16_t stbir__FP16;
  #endif

#else // no NEON, or 32-bit ARM for MSVC

  typedef union stbir__FP16
  {
    unsigned short u;
  } stbir__FP16;

#endif

#if (!defined(STBIR_NEON) && !defined(STBIR_FP16C)) || (defined(STBIR_NEON) && defined(_M_ARM)) || (defined(STBIR_NEON) && defined(__arm__))

  // Fabian's half float routines, see: https://gist.github.com/rygorous/2156668

  static stbir__inline float stbir__half_to_float( stbir__FP16 h )
  {
    static const stbir__FP32 magic = { (254 - 15) << 23 };
    static const stbir__FP32 was_infnan = { (127 + 16) << 23 };
    stbir__FP32 o;

    o.u = (h.u & 0x7fff) << 13;     // exponent/mantissa bits
    o.f *= magic.f;                 // exponent adjust
    if (o.f >= was_infnan.f)        // make sure Inf/NaN survive
      o.u |= 255 << 23;
    o.u |= (h.u & 0x8000) << 16;    // sign bit
    return o.f;
  }

  static stbir__inline stbir__FP16 stbir__float_to_half(float val)
  {
    stbir__FP32 f32infty = { 255 << 23 };
    stbir__FP32 f16max   = { (127 + 16) << 23 };
    stbir__FP32 denorm_magic = { ((127 - 15) + (23 - 10) + 1) << 23 };
    unsigned int sign_mask = 0x80000000u;
    stbir__FP16 o = { 0 };
    stbir__FP32 f;
    unsigned int sign;

    f.f = val;
    sign = f.u & sign_mask;
    f.u ^= sign;

    if (f.u >= f16max.u) // result is Inf or NaN (all exponent bits set)
      o.u = (f.u > f32infty.u) ? 0x7e00 : 0x7c00; // NaN->qNaN and Inf->Inf
    else // (De)normalized number or zero
    {
      if (f.u < (113 << 23)) // resulting FP16 is subnormal or zero
      {
        // use a magic value to align our 10 mantissa bits at the bottom of
        // the float. as long as FP addition is round-to-nearest-even this
        // just works.
        f.f += denorm_magic.f;
        // and one integer subtract of the bias later, we have our final float!
        o.u = (unsigned short) ( f.u - denorm_magic.u );
      }
      else
      {
        unsigned int mant_odd = (f.u >> 13) & 1; // resulting mantissa is odd
        // update exponent, rounding bias part 1
        f.u = f.u + ((15u - 127) << 23) + 0xfff;
        // rounding bias part 2
        f.u += mant_odd;
        // take the bits!
        o.u = (unsigned short) ( f.u >> 13 );
      }
    }

    o.u |= sign >> 16;
    return o;
  }

#endif


#if defined(STBIR_FP16C)

  #include <immintrin.h>

  static stbir__inline void stbir__half_to_float_SIMD(float * output, stbir__FP16 const * input)
  {
    _mm256_storeu_ps( (float*)output, _mm256_cvtph_ps( _mm_loadu_si128( (__m128i const* )input ) ) );
  }

  static stbir__inline void stbir__float_to_half_SIMD(stbir__FP16 * output, float const * input)
  {
    _mm_storeu_si128( (__m128i*)output, _mm256_cvtps_ph( _mm256_loadu_ps( input ), 0 ) );
  }

  static stbir__inline float stbir__half_to_float( stbir__FP16 h )
  {
    return _mm_cvtss_f32( _mm_cvtph_ps( _mm_cvtsi32_si128( (int)h.u ) ) );
  }

  static stbir__inline stbir__FP16 stbir__float_to_half( float f )
  {
    stbir__FP16 h;
    h.u = (unsigned short) _mm_cvtsi128_si32( _mm_cvtps_ph( _mm_set_ss( f ), 0 ) );
    return h;
  }

#elif defined(STBIR_SSE2)

  // Fabian's half float routines, see: https://gist.github.com/rygorous/2156668
  stbir__inline static void stbir__half_to_float_SIMD(float * output, void const * input)
  {
    static const STBIR__SIMDI_CONST(mask_nosign,      0x7fff);
    static const STBIR__SIMDI_CONST(smallest_normal,  0x0400);
    static const STBIR__SIMDI_CONST(infinity,         0x7c00);
    static const STBIR__SIMDI_CONST(expadjust_normal, (127 - 15) << 23);
    static const STBIR__SIMDI_CONST(magic_denorm,     113 << 23);

    __m128i i = _mm_loadu_si128 ( (__m128i const*)(input) );
    __m128i h = _mm_unpacklo_epi16 ( i, _mm_setzero_si128() );
    __m128i mnosign     = STBIR__CONSTI(mask_nosign);
    __m128i eadjust     = STBIR__CONSTI(expadjust_normal);
    __m128i smallest    = STBIR__CONSTI(smallest_normal);
    __m128i infty       = STBIR__CONSTI(infinity);
    __m128i expmant     = _mm_and_si128(mnosign, h);
    __m128i justsign    = _mm_xor_si128(h, expmant);
    __m128i b_notinfnan = _mm_cmpgt_epi32(infty, expmant);
    __m128i b_isdenorm  = _mm_cmpgt_epi32(smallest, expmant);
    __m128i shifted     = _mm_slli_epi32(expmant, 13);
    __m128i adj_infnan  = _mm_andnot_si128(b_notinfnan, eadjust);
    __m128i adjusted    = _mm_add_epi32(eadjust, shifted);
    __m128i den1        = _mm_add_epi32(shifted, STBIR__CONSTI(magic_denorm));
    __m128i adjusted2   = _mm_add_epi32(adjusted, adj_infnan);
    __m128  den2        = _mm_sub_ps(_mm_castsi128_ps(den1), *(const __m128 *)&magic_denorm);
    __m128  adjusted3   = _mm_and_ps(den2, _mm_castsi128_ps(b_isdenorm));
    __m128  adjusted4   = _mm_andnot_ps(_mm_castsi128_ps(b_isdenorm), _mm_castsi128_ps(adjusted2));
    __m128  adjusted5   = _mm_or_ps(adjusted3, adjusted4);
    __m128i sign        = _mm_slli_epi32(justsign, 16);
    __m128  final       = _mm_or_ps(adjusted5, _mm_castsi128_ps(sign));
    stbir__simdf_store( output + 0,  final );

    h = _mm_unpackhi_epi16 ( i, _mm_setzero_si128() );
    expmant     = _mm_and_si128(mnosign, h);
    justsign    = _mm_xor_si128(h, expmant);
    b_notinfnan = _mm_cmpgt_epi32(infty, expmant);
    b_isdenorm  = _mm_cmpgt_epi32(smallest, expmant);
    shifted     = _mm_slli_epi32(expmant, 13);
    adj_infnan  = _mm_andnot_si128(b_notinfnan, eadjust);
    adjusted    = _mm_add_epi32(eadjust, shifted);
    den1        = _mm_add_epi32(shifted, STBIR__CONSTI(magic_denorm));
    adjusted2   = _mm_add_epi32(adjusted, adj_infnan);
    den2        = _mm_sub_ps(_mm_castsi128_ps(den1), *(const __m128 *)&magic_denorm);
    adjusted3   = _mm_and_ps(den2, _mm_castsi128_ps(b_isdenorm));
    adjusted4   = _mm_andnot_ps(_mm_castsi128_ps(b_isdenorm), _mm_castsi128_ps(adjusted2));
    adjusted5   = _mm_or_ps(adjusted3, adjusted4);
    sign        = _mm_slli_epi32(justsign, 16);
    final       = _mm_or_ps(adjusted5, _mm_castsi128_ps(sign));
    stbir__simdf_store( output + 4,  final );

    // ~38 SSE2 ops for 8 values
  }

  // Fabian's round-to-nearest-even float to half
  // ~48 SSE2 ops for 8 output
  stbir__inline static void stbir__float_to_half_SIMD(void * output, float const * input)
  {
    static const STBIR__SIMDI_CONST(mask_sign,      0x80000000u);
    static const STBIR__SIMDI_CONST(c_f16max,       (127 + 16) << 23); // all FP32 values >=this round to +inf
    static const STBIR__SIMDI_CONST(c_nanbit,        0x200);
    static const STBIR__SIMDI_CONST(c_infty_as_fp16, 0x7c00);
    static const STBIR__SIMDI_CONST(c_min_normal,    (127 - 14) << 23); // smallest FP32 that yields a normalized FP16
    static const STBIR__SIMDI_CONST(c_subnorm_magic, ((127 - 15) + (23 - 10) + 1) << 23);
    static const STBIR__SIMDI_CONST(c_normal_bias,    0xfff - ((127 - 15) << 23)); // adjust exponent and add mantissa rounding

    __m128  f           =  _mm_loadu_ps(input);
    __m128  msign       = _mm_castsi128_ps(STBIR__CONSTI(mask_sign));
    __m128  justsign    = _mm_and_ps(msign, f);
    __m128  absf        = _mm_xor_ps(f, justsign);
    __m128i absf_int    = _mm_castps_si128(absf); // the cast is "free" (extra bypass latency, but no thruput hit)
    __m128i f16max      = STBIR__CONSTI(c_f16max);
    __m128  b_isnan     = _mm_cmpunord_ps(absf, absf); // is this a NaN?
    __m128i b_isregular = _mm_cmpgt_epi32(f16max, absf_int); // (sub)normalized or special?
    __m128i nanbit      = _mm_and_si128(_mm_castps_si128(b_isnan), STBIR__CONSTI(c_nanbit));
    __m128i inf_or_nan  = _mm_or_si128(nanbit, STBIR__CONSTI(c_infty_as_fp16)); // output for specials

    __m128i min_normal  = STBIR__CONSTI(c_min_normal);
    __m128i b_issub     = _mm_cmpgt_epi32(min_normal, absf_int);

    // "result is subnormal" path
    __m128  subnorm1    = _mm_add_ps(absf, _mm_castsi128_ps(STBIR__CONSTI(c_subnorm_magic))); // magic value to round output mantissa
    __m128i subnorm2    = _mm_sub_epi32(_mm_castps_si128(subnorm1), STBIR__CONSTI(c_subnorm_magic)); // subtract out bias

    // "result is normal" path
    __m128i mantoddbit  = _mm_slli_epi32(absf_int, 31 - 13); // shift bit 13 (mantissa LSB) to sign
    __m128i mantodd     = _mm_srai_epi32(mantoddbit, 31); // -1 if FP16 mantissa odd, else 0

    __m128i round1      = _mm_add_epi32(absf_int, STBIR__CONSTI(c_normal_bias));
    __m128i round2      = _mm_sub_epi32(round1, mantodd); // if mantissa LSB odd, bias towards rounding up (RTNE)
    __m128i normal      = _mm_srli_epi32(round2, 13); // rounded result

    // combine the two non-specials
    __m128i nonspecial  = _mm_or_si128(_mm_and_si128(subnorm2, b_issub), _mm_andnot_si128(b_issub, normal));

    // merge in specials as well
    __m128i joined      = _mm_or_si128(_mm_and_si128(nonspecial, b_isregular), _mm_andnot_si128(b_isregular, inf_or_nan));

    __m128i sign_shift  = _mm_srai_epi32(_mm_castps_si128(justsign), 16);
    __m128i final2, final= _mm_or_si128(joined, sign_shift);

    f           =  _mm_loadu_ps(input+4);
    justsign    = _mm_and_ps(msign, f);
    absf        = _mm_xor_ps(f, justsign);
    absf_int    = _mm_castps_si128(absf); // the cast is "free" (extra bypass latency, but no thruput hit)
    b_isnan     = _mm_cmpunord_ps(absf, absf); // is this a NaN?
    b_isregular = _mm_cmpgt_epi32(f16max, absf_int); // (sub)normalized or special?
    nanbit      = _mm_and_si128(_mm_castps_si128(b_isnan), STBIR__CONSTI(c_nanbit));
    inf_or_nan  = _mm_or_si128(nanbit, STBIR__CONSTI(c_infty_as_fp16)); // output for specials

    b_issub     = _mm_cmpgt_epi32(min_normal, absf_int);

    // "result is subnormal" path
    subnorm1    = _mm_add_ps(absf, _mm_castsi128_ps(STBIR__CONSTI(c_subnorm_magic))); // magic value to round output mantissa
    subnorm2    = _mm_sub_epi32(_mm_castps_si128(subnorm1), STBIR__CONSTI(c_subnorm_magic)); // subtract out bias

    // "result is normal" path
    mantoddbit  = _mm_slli_epi32(absf_int, 31 - 13); // shift bit 13 (mantissa LSB) to sign
    mantodd     = _mm_srai_epi32(mantoddbit, 31); // -1 if FP16 mantissa odd, else 0

    round1      = _mm_add_epi32(absf_int, STBIR__CONSTI(c_normal_bias));
    round2      = _mm_sub_epi32(round1, mantodd); // if mantissa LSB odd, bias towards rounding up (RTNE)
    normal      = _mm_srli_epi32(round2, 13); // rounded result

    // combine the two non-specials
    nonspecial  = _mm_or_si128(_mm_and_si128(subnorm2, b_issub), _mm_andnot_si128(b_issub, normal));

    // merge in specials as well
    joined      = _mm_or_si128(_mm_and_si128(nonspecial, b_isregular), _mm_andnot_si128(b_isregular, inf_or_nan));

    sign_shift  = _mm_srai_epi32(_mm_castps_si128(justsign), 16);
    final2      = _mm_or_si128(joined, sign_shift);
    final       = _mm_packs_epi32(final, final2);
    stbir__simdi_store( output,final );
  }

#elif defined(STBIR_NEON) && defined(_MSC_VER) && defined(STBIR__ARM64) && !defined(__clang__) // 64-bit ARM on MSVC (not clang)

  static stbir__inline void stbir__half_to_float_SIMD(float * output, stbir__FP16 const * input)
  {
    float16x4_t in0 = vld1_f16(input + 0);
    float16x4_t in1 = vld1_f16(input + 4);
    vst1q_f32(output + 0, vcvt_f32_f16(in0));
    vst1q_f32(output + 4, vcvt_f32_f16(in1));
  }

  static stbir__inline void stbir__float_to_half_SIMD(stbir__FP16 * output, float const * input)
  {
    float16x4_t out0 = vcvt_f16_f32(vld1q_f32(input + 0));
    float16x4_t out1 = vcvt_f16_f32(vld1q_f32(input + 4));
    vst1_f16(output+0, out0);
    vst1_f16(output+4, out1);
  }

  static stbir__inline float stbir__half_to_float( stbir__FP16 h )
  {
    return vgetq_lane_f32(vcvt_f32_f16(vld1_dup_f16(&h)), 0);
  }

  static stbir__inline stbir__FP16 stbir__float_to_half( float f )
  {
    return vget_lane_f16(vcvt_f16_f32(vdupq_n_f32(f)), 0).n16_u16[0];
  }

#elif defined(STBIR_NEON) && defined( STBIR__ARM64 ) // 64-bit ARM

  static stbir__inline void stbir__half_to_float_SIMD(float * output, stbir__FP16 const * input)
  {
    float16x8_t in = vld1q_f16(input);
    vst1q_f32(output + 0, vcvt_f32_f16(vget_low_f16(in)));
    vst1q_f32(output + 4, vcvt_f32_f16(vget_high_f16(in)));
  }

  static stbir__inline void stbir__float_to_half_SIMD(stbir__FP16 * output, float const * input)
  {
    float16x4_t out0 = vcvt_f16_f32(vld1q_f32(input + 0));
    float16x4_t out1 = vcvt_f16_f32(vld1q_f32(input + 4));
    vst1q_f16(output, vcombine_f16(out0, out1));
  }

  static stbir__inline float stbir__half_to_float( stbir__FP16 h )
  {
    return vgetq_lane_f32(vcvt_f32_f16(vdup_n_f16(h)), 0);
  }

  static stbir__inline stbir__FP16 stbir__float_to_half( float f )
  {
    return vget_lane_f16(vcvt_f16_f32(vdupq_n_f32(f)), 0);
  }

#elif defined(STBIR_WASM) || (defined(STBIR_NEON) && (defined(_MSC_VER) || defined(_M_ARM) || defined(__arm__))) // WASM or 32-bit ARM on MSVC/clang

  static stbir__inline void stbir__half_to_float_SIMD(float * output, stbir__FP16 const * input)
  {
    int i;
    for (i=0; i<8; i++)
    {
      output[i] = stbir__half_to_float(input[i]);
    }
  }
  static stbir__inline void stbir__float_to_half_SIMD(stbir__FP16 * output, float const * input)
  {
    int i;
    for (i=0; i<8; i++)
    {
      output[i] = stbir__float_to_half(input[i]);
    }
  }

#endif


#ifdef STBIR_SIMD

#define stbir__simdf_0123to3333( out, reg ) (out) = stbir__simdf_swiz( reg, 3,3,3,3 )
#define stbir__simdf_0123to2222( out, reg ) (out) = stbir__simdf_swiz( reg, 2,2,2,2 )
#define stbir__simdf_0123to1111( out, reg ) (out) = stbir__simdf_swiz( reg, 1,1,1,1 )
#define stbir__simdf_0123to0000( out, reg ) (out) = stbir__simdf_swiz( reg, 0,0,0,0 )
#define stbir__simdf_0123to0001( out, reg ) (out) = stbir__simdf_swiz( reg, 0,0,0,1 )
#define stbir__simdf_0123to1122( out, reg ) (out) = stbir__simdf_swiz( reg, 1,1,2,2 )
#define stbir__simdf_0123to2333( out, reg ) (out) = stbir__simdf_swiz( reg, 2,3,3,3 )
#define stbir__simdf_0123to1230( out, reg ) (out) = stbir__simdf_swiz( reg, 1,2,3,0 )
#define stbir__simdf_0123to2103( out, reg ) (out) = stbir__simdf_swiz( reg, 2,1,0,3 )
#define stbir__simdf_0123to3210( out, reg ) (out) = stbir__simdf_swiz( reg, 3,2,1,0 )
#define stbir__simdf_0123to2301( out, reg ) (out) = stbir__simdf_swiz( reg, 2,3,0,1 )
#define stbir__simdf_0123to3012( out, reg ) (out) = stbir__simdf_swiz( reg, 3,0,1,2 )
#define stbir__simdf_0123to0011( out, reg ) (out) = stbir__simdf_swiz( reg, 0,0,1,1 )
#define stbir__simdf_0123to2233( out, reg ) (out) = stbir__simdf_swiz( reg, 2,2,3,3 )
#define stbir__simdf_0123to1133( out, reg ) (out) = stbir__simdf_swiz( reg, 1,1,3,3 )
#define stbir__simdf_0123to0022( out, reg ) (out) = stbir__simdf_swiz( reg, 0,0,2,2 )
#define stbir__simdf_0123to1032( out, reg ) (out) = stbir__simdf_swiz( reg, 1,0,3,2 )

typedef union stbir__simdi_u32
{
  stbir_uint32 m128i_u32[4];
  int m128i_i32[4];
  stbir__simdi m128i_i128;
} stbir__simdi_u32;

static const int STBIR_mask[9] = { 0,0,0,-1,-1,-1,0,0,0 };

static const STBIR__SIMDF_CONST(STBIR_max_uint8_as_float,           stbir__max_uint8_as_float);
static const STBIR__SIMDF_CONST(STBIR_max_uint16_as_float,          stbir__max_uint16_as_float);
static const STBIR__SIMDF_CONST(STBIR_max_uint8_as_float_inverted,  stbir__max_uint8_as_float_inverted);
static const STBIR__SIMDF_CONST(STBIR_max_uint16_as_float_inverted, stbir__max_uint16_as_float_inverted);

static const STBIR__SIMDF_CONST(STBIR_simd_point5,   0.5f);
static const STBIR__SIMDF_CONST(STBIR_ones,          1.0f);
static const STBIR__SIMDI_CONST(STBIR_almost_zero,   (127 - 13) << 23);
static const STBIR__SIMDI_CONST(STBIR_almost_one,    0x3f7fffff);
static const STBIR__SIMDI_CONST(STBIR_mantissa_mask, 0xff);
static const STBIR__SIMDI_CONST(STBIR_topscale,      0x02000000);

//   Basically, in simd mode, we unroll the proper amount, and we don't want
//   the non-simd remnant loops to be unroll because they only run a few times
//   Adding this switch saves about 5K on clang which is Captain Unroll the 3rd.
#define STBIR_SIMD_STREAMOUT_PTR( star )  STBIR_STREAMOUT_PTR( star )
#define STBIR_SIMD_NO_UNROLL(ptr) STBIR_NO_UNROLL(ptr)
#define STBIR_SIMD_NO_UNROLL_LOOP_START STBIR_NO_UNROLL_LOOP_START
#define STBIR_SIMD_NO_UNROLL_LOOP_START_INF_FOR STBIR_NO_UNROLL_LOOP_START_INF_FOR

#ifdef STBIR_MEMCPY
#undef STBIR_MEMCPY
#endif
#define STBIR_MEMCPY stbir_simd_memcpy

// override normal use of memcpy with much simpler copy (faster and smaller with our sized copies)
static void stbir_simd_memcpy( void * dest, void const * src, size_t bytes )
{
  char STBIR_SIMD_STREAMOUT_PTR (*) d = (char*) dest;
  char STBIR_SIMD_STREAMOUT_PTR( * ) d_end = ((char*) dest) + bytes;
  char const * s = (char const*) src;

  // check overlaps
  STBIR_ASSERT( ( ( d >= ( (char*)src) + bytes ) ) || ( ( d + bytes ) <= (char*)src ) );

  if ( bytes < (16*stbir__simdfX_float_count) )
  {
    if ( bytes < 16 )
    {
      if ( bytes )
      {
        STBIR_SIMD_NO_UNROLL_LOOP_START
        do
        {
          STBIR_SIMD_NO_UNROLL(d);
          d[ 0 ] = s[ 0 ];
          ++d;
          ++s;
        } while ( d < d_end );
      }
    }
    else
    {
      stbir__simdf x;
      // do one unaligned to get us aligned for the stream out below
      stbir__simdf_load( x, s );
      stbir__simdf_store( d, x );
      s += ( ( ( (size_t)d ) + 16 ) & ~15 ) - (size_t)d;
      d = (char*)( ( ( (size_t)d ) + 16 ) & ~15 );

      STBIR_SIMD_NO_UNROLL_LOOP_START_INF_FOR
      for(;;)
      {
        STBIR_SIMD_NO_UNROLL(d);

        if ( d > ( d_end - 16 ) )
        {
          if ( d == d_end )
            return;
          s -= d - ( d_end - 16 );
          d = d_end - 16;
        }

        stbir__simdf_load( x, s );
        stbir__simdf_store( d, x );
        d += 16;
        s += 16;
      }
    }
  }
  else
  {
    stbir__simdfX x0,x1,x2,x3;

    // do one unaligned to get us aligned for the stream out below
    stbir__simdfX_load( x0, s +  0*stbir__simdfX_float_count );
    stbir__simdfX_load( x1, s +  4*stbir__simdfX_float_count );
    stbir__simdfX_load( x2, s +  8*stbir__simdfX_float_count );
    stbir__simdfX_load( x3, s + 12*stbir__simdfX_float_count );
    stbir__simdfX_store( d +  0*stbir__simdfX_float_count, x0 );
    stbir__simdfX_store( d +  4*stbir__simdfX_float_count, x1 );
    stbir__simdfX_store( d +  8*stbir__simdfX_float_count, x2 );
    stbir__simdfX_store( d + 12*stbir__simdfX_float_count, x3 );
    s += ( ( ( (size_t)d ) + (16*stbir__simdfX_float_count) ) & ~((16*stbir__simdfX_float_count)-1) ) - (size_t)d;
    d = (char*)( ( ( (size_t)d ) + (16*stbir__simdfX_float_count) ) & ~((16*stbir__simdfX_float_count)-1) );

    STBIR_SIMD_NO_UNROLL_LOOP_START_INF_FOR
    for(;;)
    {
      STBIR_SIMD_NO_UNROLL(d);

      if ( d > ( d_end - (16*stbir__simdfX_float_count) ) )
      {
        if ( d == d_end )
          return;
        s -= d - ( d_end - (16*stbir__simdfX_float_count) );
        d = d_end - (16*stbir__simdfX_float_count);
      }

      stbir__simdfX_load( x0, s +  0*stbir__simdfX_float_count );
      stbir__simdfX_load( x1, s +  4*stbir__simdfX_float_count );
      stbir__simdfX_load( x2, s +  8*stbir__simdfX_float_count );
      stbir__simdfX_load( x3, s + 12*stbir__simdfX_float_count );
      stbir__simdfX_store( d +  0*stbir__simdfX_float_count, x0 );
      stbir__simdfX_store( d +  4*stbir__simdfX_float_count, x1 );
      stbir__simdfX_store( d +  8*stbir__simdfX_float_count, x2 );
      stbir__simdfX_store( d + 12*stbir__simdfX_float_count, x3 );
      d += (16*stbir__simdfX_float_count);
      s += (16*stbir__simdfX_float_count);
    }
  }
}

// float copy that is specifically intentionally overlapping (src is below dest by less than the count, so a forward
//   copy replicates the first floats through the rest.
static void stbir_overlapping_memcpy( float * dest, float const * src, size_t count )
{
  float STBIR_SIMD_STREAMOUT_PTR (*) sd = (float*) src;
  float STBIR_SIMD_STREAMOUT_PTR( * ) s_end = ((float*) src) + count;
  ptrdiff_t ofs_to_dest = dest - src;

  if ( ofs_to_dest >= 4 ) // is the overlap more than 4 floats away?
  {
    float STBIR_SIMD_STREAMOUT_PTR( * ) s_end4 = ((float*) src) + (count&~(size_t)3);
    STBIR_SIMD_NO_UNROLL_LOOP_START
    do
    {
      stbir__simdf x;
      STBIR_SIMD_NO_UNROLL(sd);
      stbir__simdf_load( x, sd );
      stbir__simdf_store(  ( sd + ofs_to_dest ), x );
      sd += 4;
    } while ( sd < s_end4 );

    if ( sd == s_end )
      return;
  }

  do
  {
    STBIR_SIMD_NO_UNROLL(sd);
    sd[ ofs_to_dest ] = sd[ 0 ];
    ++sd;
  } while ( sd < s_end );
}

#else // no SSE2

// when in scalar mode, we let unrolling happen, so this macro just does the __restrict
#define STBIR_SIMD_STREAMOUT_PTR( star ) STBIR_STREAMOUT_PTR( star )
#define STBIR_SIMD_NO_UNROLL(ptr)
#define STBIR_SIMD_NO_UNROLL_LOOP_START
#define STBIR_SIMD_NO_UNROLL_LOOP_START_INF_FOR

#endif // SSE2


#ifdef STBIR_PROFILE

#ifndef STBIR_PROFILE_FUNC

#if ( defined(_x86_64) || defined( __x86_64__ ) || defined( _M_X64 ) || defined(__x86_64) || defined(__SSE2__) || defined(STBIR_SSE) || defined( _M_IX86_FP ) || defined(__i386) || defined( __i386__ ) || defined( _M_IX86 ) || defined( _X86_ ) ) && !defined(_M_ARM64EC)

#ifdef _MSC_VER

  STBIRDEF stbir_uint64 __rdtsc();
  #define STBIR_PROFILE_FUNC() __rdtsc()

#else // non msvc

  static stbir__inline stbir_uint64 STBIR_PROFILE_FUNC()
  {
    stbir_uint32 lo, hi;
    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi) );
    return ( ( (stbir_uint64) hi ) << 32 ) | ( (stbir_uint64) lo );
  }

#endif  // msvc

#elif defined( STBIR__ARM64 ) || defined(__ARM_NEON__)

#if defined( _MSC_VER ) && !defined(__clang__)

  #include <intrin.h>   // _ReadStatusReg, ARM64_SYSREG
  #ifndef ARM64_CNTVCT  // winnt.h defines it; without windows.h build the same encoding (CNTVCT_EL0, the generic timer counter)
  #define ARM64_CNTVCT ARM64_SYSREG(3,3,14,0,2)
  #endif
  #define STBIR_PROFILE_FUNC() _ReadStatusReg(ARM64_CNTVCT)

#else

  static stbir__inline stbir_uint64 STBIR_PROFILE_FUNC()
  {
    stbir_uint64 tsc;
    asm volatile("mrs %0, cntvct_el0" : "=r" (tsc));
    return tsc;
  }

#endif

#else // x64, arm

#error Unknown platform for profiling.

#endif  // x64, arm

#endif // STBIR_PROFILE_FUNC

#define STBIR_ONLY_PROFILE_GET_SPLIT_INFO ,stbir__per_split_info * split_info
#define STBIR_ONLY_PROFILE_SET_SPLIT_INFO ,split_info

#define STBIR_ONLY_PROFILE_BUILD_GET_INFO ,stbir__info * profile_info
#define STBIR_ONLY_PROFILE_BUILD_SET_INFO ,profile_info

// super light-weight micro profiler
#define STBIR_PROFILE_START_ll( info, wh ) { stbir_uint64 wh##thiszonetime = STBIR_PROFILE_FUNC(); stbir_uint64 * wh##save_parent_excluded_ptr = info->current_zone_excluded_ptr; stbir_uint64 wh##current_zone_excluded = 0; info->current_zone_excluded_ptr = &wh##current_zone_excluded;
#define STBIR_PROFILE_END_ll( info, wh ) wh##thiszonetime = STBIR_PROFILE_FUNC() - wh##thiszonetime; info->profile.named.wh += wh##thiszonetime - wh##current_zone_excluded; *wh##save_parent_excluded_ptr += wh##thiszonetime; info->current_zone_excluded_ptr = wh##save_parent_excluded_ptr; }
#define STBIR_PROFILE_FIRST_START_ll( info, wh ) { int zone; info->current_zone_excluded_ptr = &info->profile.named.total; for(zone=0;zone<(int)STBIR__ARRAY_SIZE(info->profile.array);zone++) info->profile.array[zone]=0; } STBIR_PROFILE_START_ll( info, wh );
#define STBIR_PROFILE_CLEAR_EXTRAS_ll( info, num ) { int extra; for(extra=1;extra<(num);extra++) { int zone; for(zone=0;zone<(int)STBIR__ARRAY_SIZE((info)->profile.array);zone++) (info)[extra].profile.array[zone]=0; } }

// for thread data
#define STBIR_PROFILE_START( wh ) STBIR_PROFILE_START_ll( split_info, wh )
#define STBIR_PROFILE_END( wh ) STBIR_PROFILE_END_ll( split_info, wh )
#define STBIR_PROFILE_FIRST_START( wh ) STBIR_PROFILE_FIRST_START_ll( split_info, wh )
#define STBIR_PROFILE_CLEAR_EXTRAS() STBIR_PROFILE_CLEAR_EXTRAS_ll( split_info, split_count )
#define STBIR_PROFILE_NEST_IN_FIRST( extra ) { if ( ( extra ) != split_info ) ( extra )->current_zone_excluded_ptr = split_info->current_zone_excluded_ptr; }   // an extra split's zones count against the first split's open zone

// for build data
#define STBIR_PROFILE_BUILD_START( wh ) STBIR_PROFILE_START_ll( profile_info, wh )
#define STBIR_PROFILE_BUILD_END( wh ) STBIR_PROFILE_END_ll( profile_info, wh )
#define STBIR_PROFILE_BUILD_FIRST_START( wh ) STBIR_PROFILE_FIRST_START_ll( profile_info, wh )
#define STBIR_PROFILE_BUILD_CLEAR( info ) { int zone; for(zone=0;zone<(int)STBIR__ARRAY_SIZE(info->profile.array);zone++) info->profile.array[zone]=0; }

#else  // no profile

#define STBIR_ONLY_PROFILE_GET_SPLIT_INFO
#define STBIR_ONLY_PROFILE_SET_SPLIT_INFO

#define STBIR_ONLY_PROFILE_BUILD_GET_INFO
#define STBIR_ONLY_PROFILE_BUILD_SET_INFO

#define STBIR_PROFILE_START( wh )
#define STBIR_PROFILE_END( wh )
#define STBIR_PROFILE_FIRST_START( wh )
#define STBIR_PROFILE_CLEAR_EXTRAS( )
#define STBIR_PROFILE_NEST_IN_FIRST( extra )

#define STBIR_PROFILE_BUILD_START( wh )
#define STBIR_PROFILE_BUILD_END( wh )
#define STBIR_PROFILE_BUILD_FIRST_START( wh )
#define STBIR_PROFILE_BUILD_CLEAR( info )

#endif  // stbir_profile

#ifndef STBIR_MEMCPY
// For memcpy
#include <string.h>
#define STBIR_MEMCPY( dest, src, len ) memcpy( dest, src, len )
#endif

#ifndef STBIR_SIMD

// float copy that is specifically intentionally overlapping (src is below dest by less than the count, so a forward
//   copy replicates the first floats through the rest.
static void stbir_overlapping_memcpy( float * dest, float const * src, size_t count )
{
  float STBIR_SIMD_STREAMOUT_PTR (*) sd = (float*) src;
  float STBIR_SIMD_STREAMOUT_PTR( * ) s_end = ((float*) src) + count;
  ptrdiff_t ofs_to_dest = dest - src;

  if ( ofs_to_dest >= 2 ) // is the overlap more than 2 floats away
  {
    float STBIR_SIMD_STREAMOUT_PTR( * ) s_end2 = ((float*) src) + (count&~(size_t)1);

    STBIR_NO_UNROLL_LOOP_START
    do
    {
      float a,b;
      STBIR_NO_UNROLL(sd);
      a = sd[0];
      b = sd[1];
      sd[ ofs_to_dest ] = a;
      sd[ ofs_to_dest + 1 ] = b;
      sd += 2;
    } while ( sd < s_end2 );

    if ( sd == s_end )
      return;
  }

  STBIR_NO_UNROLL_LOOP_START
  do
  {
    STBIR_NO_UNROLL(sd);
    sd[ ofs_to_dest ] = sd[ 0 ];
    ++sd;
  } while ( sd < s_end );
}

#endif

// the built-in kernels (all in stbir_real; the generator rounds the weight to float once, on the way into the
//   coefficient table). The box filter as the overlap of a pixel with the output pixel's footprint: 1 within
//   0.5 - scale / 2 of the center, 0 beyond 0.5 + scale / 2, a ramp between.
static stbir_real stbir__filter_trapezoid(stbir_real x, stbir_real scale, void * user_data)
{
  stbir_real halfscale = scale * (stbir_real)0.5;
  stbir_real t = (stbir_real)0.5 + halfscale;
  stbir_real r = (stbir_real)0.5 - halfscale;
  STBIR_ASSERT(scale <= (stbir_real)1.0);
  STBIR__UNUSED(user_data);

  if ( x < (stbir_real) 0.0 ) x = -x;

  if ( x >= t )
    return (stbir_real)0.0;
  if ( x <= r )
    return (stbir_real)1.0;
  return ( t - x ) / scale;
}

// the box's extent depends on the scale, so it is given as a radius (in input pixels): one multiply, so an integer
//   ratio's tap count is exact
static stbir_real stbir__radius_trapezoid(stbir_real scale, stbir_real inv_scale, int is_upsample, void * user_data)
{
  STBIR__UNUSED(scale);
  STBIR__UNUSED(is_upsample);
  STBIR__UNUSED(user_data);
  return (stbir_real)0.5 * inv_scale + (stbir_real)0.5;   // ( 0.5 + scale / 2 ) output pixels downsampling, 0.5 + inv_scale / 2 input pixels upsampling: the same number
}

static stbir_real stbir__filter_triangle(stbir_real x, stbir_real s, void * user_data)
{
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  if ( x < (stbir_real) 0.0 ) x = -x;

  if (x <= (stbir_real) 1.0)
    return (stbir_real) 1.0 - x;
  else
    return (stbir_real) 0.0;
}

static stbir_real stbir__filter_point(stbir_real x, stbir_real s, void * user_data)
{
  STBIR__UNUSED(x);
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  return (stbir_real) 1.0;
}

// the Mitchell-Netrevalli cubics with C = (1-B)/2: B=1 is the cubic b-spline, 1/3 Mitchell, 0 Catmull-Rom
static stbir__inline stbir_real stbir__mitchell_netrevalli(stbir_real x, stbir_real B)
{
  if ( x < (stbir_real) 0.0 ) x = -x;

  if (x < (stbir_real) 1.0)
    return (((stbir_real) 6.0 - (stbir_real) 2.0*B) + x*x*(((stbir_real) 9.0 - (stbir_real) 6.0*B)*x - ((stbir_real) 15.0 - (stbir_real) 9.0*B)))/(stbir_real) 6.0;
  else if (x < (stbir_real) 2.0)
    return (((stbir_real) 12.0 - (stbir_real) 4.0*B) + x*(((stbir_real) 12.0*B - (stbir_real) 24.0) + x*(((stbir_real) 15.0 - (stbir_real) 9.0*B) + ((stbir_real) 2.0*B - (stbir_real) 3.0)*x)))/(stbir_real) 6.0;

  return (stbir_real) 0.0;
}

static stbir_real stbir__filter_cubic(stbir_real x, stbir_real s, void * user_data)
{
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  return stbir__mitchell_netrevalli( x, (stbir_real) 1.0 );
}

// Catmull-Rom is the family at B=0, written out (it's the upsampling default)
static stbir_real stbir__filter_catmullrom(stbir_real x, stbir_real s, void * user_data)
{
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  if ( x < (stbir_real) 0.0 ) x = -x;

  if (x < (stbir_real) 1.0)
    return (stbir_real) 1.0 - x*x*((stbir_real) 2.5 - (stbir_real) 1.5*x);
  else if (x < (stbir_real) 2.0)
    return (stbir_real) 2.0 - x*((stbir_real) 4.0 + x*((stbir_real) 0.5*x - (stbir_real) 2.5));

  return (stbir_real) 0.0;
}

static stbir_real stbir__filter_mitchell(stbir_real x, stbir_real s, void * user_data)
{
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  return stbir__mitchell_netrevalli( x, (stbir_real) 1.0 / (stbir_real) 3.0 );
}

static stbir_real stbir__filter_mitchell_quarter(stbir_real x, stbir_real s, void * user_data)
{
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  return stbir__mitchell_netrevalli( x, (stbir_real) 0.25 );
}

static stbir_real stbir__filter_mitchell_sixth(stbir_real x, stbir_real s, void * user_data)
{
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  return stbir__mitchell_netrevalli( x, (stbir_real) 1.0 / (stbir_real) 6.0 );
}

static stbir_real stbir__filter_mitchell_neg_sixth(stbir_real x, stbir_real s, void * user_data)
{
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  return stbir__mitchell_netrevalli( x, (stbir_real) -1.0 / (stbir_real) 6.0 );
}

static stbir_real stbir__filter_mitchell_neg_third(stbir_real x, stbir_real s, void * user_data)
{
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  return stbir__mitchell_netrevalli( x, (stbir_real) -1.0 / (stbir_real) 3.0 );
}

// sin(pi x) / (pi x), done here so the coefficients match on every platform (no libm). sin(pi x) flips sign at every
//   whole x, so reduce to the nearest whole and run the series on [-pi/2, pi/2] (good to ~1e-9).
static stbir_real stbir__sinc_pi(stbir_real x)
{
  stbir_real const pi = (stbir_real) 3.14159265358979323846;
  stbir_real u, u2, s;
  int n;

  if ( x < (stbir_real) 0.0 ) x = -x;

  if (x < (stbir_real) 1e-4)
  {
    stbir_real px = pi * x;
    return (stbir_real) 1.0 - px*px / (stbir_real) 6.0;
  }

  n = (int) ( x + (stbir_real) 0.5 );
  u = pi * ( x - (stbir_real) n );
  u2 = u*u;
  s = (stbir_real) 1.0 - u2 / (stbir_real) 156.0;        // the series from the inside out: 1 - u^2 / ( 2k ( 2k + 1 ) )
  s = (stbir_real) 1.0 - u2 / (stbir_real) 110.0 * s;
  s = (stbir_real) 1.0 - u2 / (stbir_real) 72.0 * s;
  s = (stbir_real) 1.0 - u2 / (stbir_real) 42.0 * s;
  s = (stbir_real) 1.0 - u2 / (stbir_real) 20.0 * s;
  s = (stbir_real) 1.0 - u2 / (stbir_real) 6.0 * s;
  s = u * s;
  if ( n & 1 ) s = -s;

  return s / ( pi * x );
}

// Lanczos: a sinc windowed by a sinc stretched to the support
static stbir_real stbir__filter_lanczos(stbir_real x, stbir_real support)
{
  if ( x < (stbir_real) 0.0 ) x = -x;

  if (x < support)
    return stbir__sinc_pi( x ) * stbir__sinc_pi( x / support );

  return (stbir_real) 0.0;
}

static stbir_real stbir__filter_lanczos4(stbir_real x, stbir_real s, void * user_data)
{
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  return stbir__filter_lanczos( x, (stbir_real) 2.0 );
}

static stbir_real stbir__filter_lanczos5(stbir_real x, stbir_real s, void * user_data)
{
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  return stbir__filter_lanczos( x, (stbir_real) 2.5 );
}

// Casey Muratori's Bink 2 half pixel kernel ( 1 -4 19 19 -4 1 ) / 32: a Catmull-Rom spline through knots every half
//   pixel (1 at 0, the weights at the half pixels, 0 at the whole ones), padded at both ends so the four points stay in the table
static stbir_real stbir__filter_muratori6(stbir_real x, stbir_real s, void * user_data)
{
  static stbir_real const knots[9] = { (stbir_real) 19.0/(stbir_real) 32.0, (stbir_real) 1.0, (stbir_real) 19.0/(stbir_real) 32.0, (stbir_real) 0.0, (stbir_real) -4.0/(stbir_real) 32.0, (stbir_real) 0.0, (stbir_real) 1.0/(stbir_real) 32.0, (stbir_real) 0.0, (stbir_real) 0.0 };
  stbir_real t, p0, p1, p2, p3;
  int i;
  STBIR__UNUSED(s);
  STBIR__UNUSED(user_data);

  if ( x < (stbir_real) 0.0 ) x = -x;

  if ( x >= (stbir_real) 3.0 )
    return (stbir_real) 0.0;

  t = x * (stbir_real) 2.0;
  i = (int) t;
  t -= (stbir_real) i;

  p0 = knots[i];
  p1 = knots[i+1];
  p2 = knots[i+2];
  p3 = knots[i+3];

  return (stbir_real) 0.5*((stbir_real) 2.0*p1 + t*((p2 - p0) + t*(((stbir_real) 2.0*p0 - (stbir_real) 5.0*p1 + (stbir_real) 4.0*p2 - p3) + t*(((stbir_real) 3.0*p1 - (stbir_real) 3.0*p2) + (p3 - p0)))));
}

// point sampling takes the one nearest input pixel in both directions.
static stbir_real stbir__radius_point(stbir_real scale, stbir_real inv_scale, int is_upsample, void * user_data)
{
  STBIR__UNUSED(scale);
  STBIR__UNUSED(inv_scale);
  STBIR__UNUSED(is_upsample);
  STBIR__UNUSED(user_data);
  return (stbir_real)0.5;
}

// the other built-ins' supports: the kernel's radius in filter pixels

static stbir_real stbir__support_one(stbir_real scale, void * user_data)
{
  STBIR__UNUSED(scale);
  STBIR__UNUSED(user_data);
  return (stbir_real)1.0;
}

static stbir_real stbir__support_two(stbir_real scale, void * user_data)
{
  STBIR__UNUSED(scale);
  STBIR__UNUSED(user_data);
  return (stbir_real)2.0;
}

static stbir_real stbir__support_two_and_half(stbir_real scale, void * user_data)
{
  STBIR__UNUSED(scale);
  STBIR__UNUSED(user_data);
  return (stbir_real)2.5;
}

static stbir_real stbir__support_three(stbir_real scale, void * user_data)
{
  STBIR__UNUSED(scale);
  STBIR__UNUSED(user_data);
  return (stbir_real)3.0;
}

// the built-in filters, indexed by stbir_filter (everything below STBIR_FILTER_OTHER
static stbir__kernel_callback_new * stbir__builtin_kernels[STBIR_FILTER_OTHER] =
{
  0,                                 // STBIR_FILTER_DEFAULT (resolved before the lookup)
  stbir__filter_trapezoid,           // STBIR_FILTER_BOX
  stbir__filter_triangle,            // STBIR_FILTER_TRIANGLE
  stbir__filter_cubic,               // STBIR_FILTER_CUBICBSPLINE
  stbir__filter_catmullrom,          // STBIR_FILTER_CATMULLROM
  stbir__filter_mitchell,            // STBIR_FILTER_MITCHELL
  stbir__filter_point,               // STBIR_FILTER_POINT_SAMPLE
  stbir__filter_mitchell_quarter,    // STBIR_FILTER_MITCHELL_QUARTER
  stbir__filter_mitchell_sixth,      // STBIR_FILTER_MITCHELL_SIXTH
  stbir__filter_mitchell_neg_sixth,  // STBIR_FILTER_MITCHELL_NEG_SIXTH
  stbir__filter_mitchell_neg_third,  // STBIR_FILTER_MITCHELL_NEG_THIRD
  stbir__filter_lanczos4,            // STBIR_FILTER_LANCZOS4
  stbir__filter_lanczos5,            // STBIR_FILTER_LANCZOS5
  stbir__filter_muratori6,           // STBIR_FILTER_MURATORI6
};

static stbir__support_callback_new * stbir__builtin_supports[STBIR_FILTER_OTHER] =
{
  0,                                 // STBIR_FILTER_DEFAULT
  0,                                 // STBIR_FILTER_BOX (a radius)
  stbir__support_one,                // STBIR_FILTER_TRIANGLE
  stbir__support_two,                // STBIR_FILTER_CUBICBSPLINE
  stbir__support_two,                // STBIR_FILTER_CATMULLROM
  stbir__support_two,                // STBIR_FILTER_MITCHELL
  0,                                 // STBIR_FILTER_POINT_SAMPLE (a radius)
  stbir__support_two,                // STBIR_FILTER_MITCHELL_QUARTER
  stbir__support_two,                // STBIR_FILTER_MITCHELL_SIXTH
  stbir__support_two,                // STBIR_FILTER_MITCHELL_NEG_SIXTH
  stbir__support_two,                // STBIR_FILTER_MITCHELL_NEG_THIRD
  stbir__support_two,                // STBIR_FILTER_LANCZOS4
  stbir__support_two_and_half,       // STBIR_FILTER_LANCZOS5
  stbir__support_three,              // STBIR_FILTER_MURATORI6
};

static stbir__radius_callback_new * stbir__builtin_radii[STBIR_FILTER_OTHER] =
{
  0,                                 // STBIR_FILTER_DEFAULT
  stbir__radius_trapezoid,           // STBIR_FILTER_BOX
  0, 0, 0, 0,                        // STBIR_FILTER_TRIANGLE .. STBIR_FILTER_MITCHELL (supports)
  stbir__radius_point,               // STBIR_FILTER_POINT_SAMPLE
  0, 0, 0, 0, 0, 0, 0,               // STBIR_FILTER_MITCHELL_QUARTER .. STBIR_FILTER_MURATORI6 (supports)
};

// upsample (1:1 included), and a 1:1 with the output on the input's pixel grid (a copy)
static int stbir__scale_is_upsample( stbir__scale_info const * scale_info )
{
  return ( scale_info->scale >= (stbir_real) 1.0 );
}

static int stbir__scale_is_copy( stbir__scale_info const * scale_info )
{
  return ( scale_info->positions_exact ) && ( scale_info->scale_numerator == scale_info->scale_denominator );
}

// the sum under which a coefficient set is zeroed
static stbir_real stbir__too_small_coefficient_sum_for( stbir__scale_info const * scale_info )
{
  if ( stbir__scale_is_upsample( scale_info ) )
    return stbir__too_small_coefficient_sum;
  return stbir__too_small_coefficient_sum * scale_info->scale;
}

// floor and ceil without math.h (the header doesn't use it), either sign, for pixel coordinates
//   and tap counts - the range is known to fit an int
static int stbir__floor_to_int( stbir_real x )
{
  int i = (int) x;
  return ( (stbir_real)i > x ) ? i - 1 : i;
}

static int stbir__ceil_to_int( stbir_real x )
{
  int i = (int) x;
  return ( (stbir_real)i < x ) ? i + 1 : i;
}

// the filter's radius in input pixels. a radius callback gives it directly. a support callback gives the radius in
//   filter pixels (called with the ratio <= 1): input pixels when upsampling, output pixels when downsampling, so it
//   is scaled by inv_scale there. the old float form is converted the same way.
static stbir_real stbir__filter_radius_in_input_pixels( stbir__radius_callback_new * radius, stbir__support_callback_new * support_new, stbir__support_callback * support_old, stbir__scale_info const * scale_info, void * user_data )
{
  int is_upsample = stbir__scale_is_upsample( scale_info );
  stbir_real r;

  if ( radius )
    r = radius( scale_info->scale, scale_info->inv_scale, is_upsample, user_data );
  else if ( is_upsample )
  {
    if ( support_new )
      r = support_new( scale_info->inv_scale, user_data );
    else
      r = (stbir_real) support_old( (float) scale_info->inv_scale, user_data );
  }
  else
  {
    if ( support_new )
      r = support_new( scale_info->scale, user_data );
    else
      r = (stbir_real) support_old( (float) scale_info->scale, user_data );
    r *= scale_info->inv_scale;
  }

  // at least half an input pixel (point sampling's), so every output has a tap: the downsample generator walks the
  //   inputs and gives each output the ones within reach. a user filter's might be under; the built-ins never are
  if ( !( r >= (stbir_real)0.5 ) )
    r = (stbir_real)0.5;

  return r;
}

static stbir_real stbir__filter_radius_in_output_pixels( stbir__radius_callback_new * radius, stbir__support_callback_new * support_new, stbir__support_callback * support_old, stbir__scale_info const * scale_info, void * user_data )
{
  return stbir__filter_radius_in_input_pixels( radius, support_new, support_old, scale_info, user_data ) / scale_info->inv_scale;
}

// This is the maximum number of input samples that can affect an output sample
// with the given filter from the output pixel's perspective
static int stbir__get_filter_pixel_width( stbir__radius_callback_new * radius, stbir__support_callback_new * support_new, stbir__support_callback * support_old, stbir__scale_info const * scale_info, void * user_data)
{
  STBIR_ASSERT( ( radius != 0 ) || ( support_new != 0 ) || ( support_old != 0 ) );
  return stbir__ceil_to_int( (stbir_real)2.0 * stbir__filter_radius_in_input_pixels( radius, support_new, support_old, scale_info, user_data ) );
}

// this is how many coefficients per run of the filter (which is different from the filter_pixel_width depending on if we are scattering or gathering)
static int stbir__get_coefficient_width(stbir__sampler * samp, int is_gather, void * user_data)
{
  stbir__scale_info const * scale_info = &samp->scale_info;

  switch( is_gather )
  {
    case 1:
    case 2:
      // one of slack: a tap exactly on the far edge is in the range (stbir__exact_pixel_range), one more than ceil( 2 x radius )
      return stbir__ceil_to_int( (stbir_real)2.0 * stbir__filter_radius_in_input_pixels( samp->filter_radius, samp->filter_support_new, samp->filter_support_old, scale_info, user_data ) ) + 1;
    case 0:
      return stbir__ceil_to_int( (stbir_real)2.0 * stbir__filter_radius_in_output_pixels( samp->filter_radius, samp->filter_support_new, samp->filter_support_old, scale_info, user_data ) );
    default:
      STBIR_ASSERT( (is_gather >= 0 ) && (is_gather <= 2 ) );
      return 0;
  }
}

static int stbir__get_contributors(stbir__sampler * samp, int is_gather)
{
  if (is_gather)
      return samp->scale_info.output_sub_size;
  else
      return (samp->scale_info.input_full_size + samp->filter_pixel_margin * 2);
}

static int stbir__edge_zero_full( int n, int max )
{
  STBIR__UNUSED(n);
  STBIR__UNUSED(max);
  return 0; // NOTREACHED
}

static int stbir__edge_clamp_full( int n, int max )
{
  if (n < 0)
    return 0;

  if (n >= max)
    return max - 1;

  return n; // NOTREACHED
}

static int stbir__edge_reflect_full( int n, int max )
{
  if (n < 0)
  {
    if (n > -max)
      return -n;
    else
      return max - 1;
  }

  if (n >= max)
  {
    if ( ( n - max ) >= max )   // (as n >= 2 max, without the overflow)
      return 0;
    else
      return ( max - 1 ) - ( n - max );
  }

  return n; // NOTREACHED
}

static int stbir__edge_wrap_full( int n, int max )
{
  if (n >= 0)
    return (n % max);
  else
  {
    int m = (-n) % max;

    if (m != 0)
      m = max - m;

    return (m);
  }
}

typedef int stbir__edge_wrap_func( int n, int max );
static stbir__edge_wrap_func * stbir__edge_wrap_slow[] =
{
  stbir__edge_clamp_full,    // STBIR_EDGE_CLAMP
  stbir__edge_reflect_full,  // STBIR_EDGE_REFLECT
  stbir__edge_wrap_full,     // STBIR_EDGE_WRAP
  stbir__edge_zero_full,     // STBIR_EDGE_ZERO
};

stbir__inline static int stbir__edge_wrap(stbir_edge edge, int n, int max)
{
  // avoid per-pixel switch
  if (n >= 0 && n < max)
      return n;
  return stbir__edge_wrap_slow[edge]( n, max );
}

// the ranges' edge handling: in pixel ranges may hang off the image with WRAP (the decoder wraps them, within a width
//   each side), out pixel ranges are clipped to the output
static void stbir__clip_in_pixel_range( int first, int last, int input_size, stbir_edge edge, int * first_pixel, int * last_pixel )
{
  if ( last < first )
    last = first; // point sample mode can span a value *right* at 0.5, and cause these to cross
  if ( edge == STBIR_EDGE_WRAP )
  {
    if ( first < -input_size )
      first = -input_size;
    if ( ( last - input_size ) >= input_size )   // (as last >= 2 input_size, without the overflow)
      last = ( input_size - 1 ) + input_size;
  }
  *first_pixel = first;
  *last_pixel = last;
}

static void stbir__clip_out_pixel_range( int first, int last, int out_size, int * first_pixel, int * last_pixel )
{
  if ( first < 0 )
    first = 0;
  if ( last >= out_size )
    last = out_size - 1;
  *first_pixel = first;
  *last_pixel = last;
}

// The pixels within support of the center numer / den (quotient kept apart, only the remainder meets the real math).
//   On the far edge is in, on the near edge out; stbir__tie_epsilon lifts the tie above rounding noise (point sampling
//   hits both edges at every even ratio). The output range and the real chain ranges below use the same rule.
#define stbir__tie_epsilon ( ( sizeof( stbir_real ) == 4 ) ? (stbir_real)( 1.0 / ( 1 << 18 ) ) : (stbir_real)( 1.0 / ( (stbir_int64)1 << 36 ) ) )

// The input pixels within out_filter_radius (output pixels) of an output pixel's center.
static void stbir__calculate_in_pixel_range( int * first_pixel, int * last_pixel, stbir_real out_pixel_center, stbir_real out_filter_radius, stbir_real inv_scale, stbir_real out_shift, int input_size, stbir_edge edge )
{
  stbir_real in_pixel_influence_lowerbound = ( out_pixel_center - out_filter_radius + out_shift ) * inv_scale;
  stbir_real in_pixel_influence_upperbound = ( out_pixel_center + out_filter_radius + out_shift ) * inv_scale;
  stbir__clip_in_pixel_range( stbir__floor_to_int( in_pixel_influence_lowerbound + (stbir_real)0.5 + stbir__tie_epsilon ), stbir__floor_to_int( in_pixel_influence_upperbound - (stbir_real)0.5 + stbir__tie_epsilon ), input_size, edge, first_pixel, last_pixel );
}

// Output pixel o's center against input pixel i's (in input pixels for upsamples, output pixels for downsamples). With an
//   exact scale N/D and integer shift it is ( ( 2o + 1 + 2 shift ) D - ( 2i + 1 ) N ) / 2N (or 2D): integers until one
//   multiply by the precomputed 1 / 2N (or 1 / 2D), exact to float at any coordinate.
static stbir_real stbir__tap_offset( stbir__scale_info const * scale_info, int o, int i, int in_input_pixels )
{
  if ( scale_info->positions_exact )
  {
    // (the difference is at most the filter's width times 2N or 2D: exact in a double, a float ulp in a float)
    stbir_int64 numer = ( 2 * (stbir_int64)o + 1 + 2 * (stbir_int64)scale_info->shift_pixels ) * (stbir_int64)scale_info->scale_denominator
                      - ( 2 * (stbir_int64)i + 1 ) * (stbir_int64)scale_info->scale_numerator;
    return (stbir_real)numer * ( in_input_pixels ? scale_info->tap_scale_in : scale_info->tap_scale_out );
  }
  if ( in_input_pixels )
    return ( ( (stbir_real)o + (stbir_real)0.5 + scale_info->pixel_shift ) * scale_info->inv_scale ) - ( (stbir_real)i + (stbir_real)0.5 );
  return ( (stbir_real)o + (stbir_real)0.5 ) - ( ( (stbir_real)i + (stbir_real)0.5 ) * scale_info->scale - scale_info->pixel_shift );
}

// the kernel's weight at x (in filter pixels): through the new form directly, or an old float kernel's floats
static stbir_real stbir__kernel_weight( stbir__kernel_callback_new * kernel_new, stbir__kernel_callback * kernel_old, stbir_real x, stbir_real scale, void * user_data )
{
  if ( kernel_new )
    return kernel_new( x, scale, user_data );
  return (stbir_real) kernel_old( (float) x, (float) scale, user_data );
}

static void stbir__exact_pixel_range( stbir_int64 numer, stbir_int64 den, stbir_real den_reciprocal, stbir_real support, int * first, int * last )
{
  int q = (int)( numer / den );
  stbir_real f = (stbir_real)( numer % den ) * den_reciprocal;   // ( -1, 1 ): the same sign as numer
  *first = q + stbir__floor_to_int( f - support + (stbir_real)0.5 + stbir__tie_epsilon );
  *last  = q + stbir__floor_to_int( f + support - (stbir_real)0.5 + stbir__tie_epsilon );
}

static void stbir__exact_output_range( stbir_int64 numer, stbir_int64 den, stbir_real den_reciprocal, stbir_real support, int * first, int * last )
{
  int q = (int)( numer / den );
  stbir_real f = (stbir_real)( numer % den ) * den_reciprocal;
  *first = q + stbir__floor_to_int( f - support - (stbir_real)0.5 - stbir__tie_epsilon ) + 1;   // ceil, a tie exactly on it included
  *last  = q + stbir__floor_to_int( f + support - (stbir_real)0.5 - stbir__tie_epsilon );       // a tie exactly on it excluded
}

// The input pixels within the filter's support of output pixel o: out_filter_radius is the support in output pixels for
//   the real chain, support the same in input pixels (the filter's own, for upsamples) for the exact positions - then
//   the center is the one stbir__tap_offset weights from.
static void stbir__output_in_pixel_range( stbir__scale_info const * scale_info, int o, stbir_real out_filter_radius, stbir_real support, stbir_edge edge, int * first_pixel, int * last_pixel )
{
  if ( scale_info->positions_exact )
  {
    stbir_int64 numer = ( 2 * (stbir_int64)o + 1 + 2 * (stbir_int64)scale_info->shift_pixels ) * (stbir_int64)scale_info->scale_denominator;   // output o's center in input pixels, over 2N
    int first, last;
    stbir__exact_pixel_range( numer, 2 * (stbir_int64)scale_info->scale_numerator, scale_info->tap_scale_in, support, &first, &last );
    stbir__clip_in_pixel_range( first, last, scale_info->input_full_size, edge, first_pixel, last_pixel );
  }
  else
    stbir__calculate_in_pixel_range( first_pixel, last_pixel, (stbir_real)o + (stbir_real)0.5, out_filter_radius, scale_info->inv_scale, scale_info->pixel_shift, scale_info->input_full_size, edge );
}

static void stbir__calculate_coefficients_for_gather_upsample( stbir_real out_filter_radius, stbir_real support, stbir__kernel_callback_new * kernel_new, stbir__kernel_callback * kernel_old, stbir__scale_info * scale_info, int num_contributors, stbir__contributors* contributors, float* coefficient_group, int coefficient_width, stbir_edge edge, void * user_data )
{
  int n, end;
  int numerator = scale_info->scale_numerator;
  int polyphase = ( ( scale_info->scale_is_rational ) && ( numerator < num_contributors ) );

  // Looping through out pixels
  end = num_contributors; if ( polyphase ) end = numerator;
  for (n = 0; n < end; n++)
  {
    int i;
    int last_non_zero;
    int in_first_pixel, in_last_pixel;

    stbir__output_in_pixel_range( scale_info, n, out_filter_radius, support, edge, &in_first_pixel, &in_last_pixel );

    // make sure we never generate a range larger than our precalculated coeff width
    //   this only happens in point sample mode, but it's a good safe thing to do anyway
    if ( ( in_last_pixel - in_first_pixel + 1 ) > coefficient_width )
      in_last_pixel = in_first_pixel + coefficient_width - 1;

    last_non_zero = -1;
    for (i = 0; i <= in_last_pixel - in_first_pixel; i++)
    {
      stbir_real weight = stbir__kernel_weight( kernel_new, kernel_old, stbir__tap_offset( scale_info, n, i + in_first_pixel, 1 ), scale_info->inv_scale, user_data );
      float coeff = (float) weight;

      if ( ( weight < stbir__too_small_coefficient ) && ( weight > -stbir__too_small_coefficient ) )
      {
        if ( i == 0 )  // if we're at the front, just eat zero contributors
        {
          STBIR_ASSERT ( ( in_last_pixel - in_first_pixel ) != 0 ); // there should be at least one contrib
          ++in_first_pixel;
          i--;
          continue;
        }
        coeff = 0;  // make sure is fully zero (should keep denormals away)
      }
      else
        last_non_zero = i;

      coefficient_group[i] = coeff;
    }

    in_last_pixel = last_non_zero+in_first_pixel; // kills trailing zeros
    contributors->n0 = in_first_pixel;
    contributors->n1 = in_last_pixel;

    STBIR_ASSERT(contributors->n1 >= contributors->n0);

    ++contributors;
    coefficient_group += coefficient_width;
  }
}

static void stbir__insert_coeff( stbir__contributors * contribs, float * coeffs, int new_pixel, float new_coeff, int max_width )
{
  if ( contribs->n1 < contribs->n0 ) // this first clause should never happen, but handle in case
  {
    contribs->n0 = contribs->n1 = new_pixel;
    coeffs[0] = new_coeff;
  }
  else if ( new_pixel <= contribs->n1 )  // before the end
  {
    if ( new_pixel < contribs->n0 ) // before the front?
    {
      if ( ( contribs->n1 - new_pixel + 1 ) <= max_width )
      { 
        int j, o = contribs->n0 - new_pixel;
        for ( j = contribs->n1 - contribs->n0 ; j >= 0 ; j-- )
          coeffs[ j + o ] = coeffs[ j ];
        for ( j = 1 ; j < o ; j++ )
          coeffs[ j ] = 0;
        coeffs[ 0 ] = new_coeff;
        contribs->n0 = new_pixel;
      }
    }
    else
    {
      // add new weight to existing coeff if already there
      coeffs[ new_pixel - contribs->n0 ] += new_coeff;
    }
  }
  else
  {
    if ( ( new_pixel - contribs->n0 + 1 ) <= max_width )
    {
      int j, e = new_pixel - contribs->n0;
      for( j = ( contribs->n1 - contribs->n0 ) + 1 ; j < e ; j++ ) // clear in-betweens coeffs if there are any
        coeffs[j] = 0;

      coeffs[ e ] = new_coeff;
      contribs->n1 = new_pixel;
    }
  }
}

// The output pixels within in_pixels_radius (input pixels) of an input pixel's center.
static void stbir__calculate_out_pixel_range( int * first_pixel, int * last_pixel, stbir_real in_pixel_center, stbir_real in_pixels_radius, stbir_real scale, stbir_real out_shift, int out_size )
{
  stbir_real out_pixel_influence_lowerbound = ( in_pixel_center - in_pixels_radius ) * scale - out_shift;
  stbir_real out_pixel_influence_upperbound = ( in_pixel_center + in_pixels_radius ) * scale - out_shift;
  stbir__clip_out_pixel_range( stbir__floor_to_int( out_pixel_influence_lowerbound - (stbir_real)0.5 - stbir__tie_epsilon ) + 1, stbir__floor_to_int( out_pixel_influence_upperbound - (stbir_real)0.5 - stbir__tie_epsilon ), out_size, first_pixel, last_pixel );
}

// The output pixels within the filter's support of input pixel i, one pixel at a time - the same ways as
//   stbir__output_in_pixel_range: in_pixels_radius is the support in input pixels for the real chain, support the
//   same in output pixels for the exact positions.
static void stbir__input_out_pixel_range( stbir__scale_info const * scale_info, int i, stbir_real in_pixels_radius, stbir_real support, int * first_pixel, int * last_pixel )
{
  if ( scale_info->positions_exact )
  {
    stbir_int64 numer = ( 2 * (stbir_int64)i + 1 ) * (stbir_int64)scale_info->scale_numerator - 2 * (stbir_int64)scale_info->shift_pixels * (stbir_int64)scale_info->scale_denominator;   // input i's center in output pixels, over 2D
    int first, last;
    stbir__exact_output_range( numer, 2 * (stbir_int64)scale_info->scale_denominator, scale_info->tap_scale_out, support, &first, &last );
    stbir__clip_out_pixel_range( first, last, scale_info->output_sub_size, first_pixel, last_pixel );
  }
  else
    stbir__calculate_out_pixel_range( first_pixel, last_pixel, (stbir_real)i + (stbir_real)0.5, in_pixels_radius, scale_info->scale, scale_info->pixel_shift, scale_info->output_sub_size );
}

static void stbir__calculate_coefficients_for_gather_downsample( int start, int end, stbir_real in_pixels_radius, stbir_real support, stbir__kernel_callback_new * kernel_new, stbir__kernel_callback * kernel_old, stbir__scale_info * scale_info, int coefficient_width, int num_contributors, stbir__contributors * contributors, float * coefficient_group, void * user_data )
{
  int in_pixel;
  int i;
  int first_out_inited = -1;
  int out_size = scale_info->output_sub_size;
  int numerator = scale_info->scale_numerator;
  int polyphase = ( ( scale_info->scale_is_rational ) && ( numerator < out_size ) );

  STBIR__UNUSED(num_contributors);

  // Loop through the input pixels
  for (in_pixel = start; in_pixel < end; in_pixel++)
  {
    int out_first_pixel, out_last_pixel;

    stbir__input_out_pixel_range( scale_info, in_pixel, in_pixels_radius, support, &out_first_pixel, &out_last_pixel );

    if ( out_first_pixel > out_last_pixel )
      continue;

    // clamp or exit if we are using polyphase filtering, and the limit is up
    if ( polyphase )
    {
      // when polyphase, you only have to do coeffs up to the numerator count
      if ( out_first_pixel >= numerator )
        break;

      // don't do any extra work, clamp last pixel at numerator too
      if ( out_last_pixel >= numerator )
        out_last_pixel = numerator - 1;
    }

    for (i = 0; i <= out_last_pixel - out_first_pixel; i++)
    {
      stbir_real weight = stbir__kernel_weight( kernel_new, kernel_old, stbir__tap_offset( scale_info, i + out_first_pixel, in_pixel, 0 ), scale_info->scale, user_data );
      float coeff = 0.0f;

      if ( ( weight >= stbir__too_small_coefficient ) || ( weight <= -stbir__too_small_coefficient ) )
        coeff = (float) ( weight * scale_info->scale );

      {
        int out = i + out_first_pixel;
        float * coeffs = coefficient_group + (size_t)out * coefficient_width;
        stbir__contributors * contribs = contributors + out;

        // is this the first time this output pixel has been seen?  Init it.
        if ( out > first_out_inited )
        {
          STBIR_ASSERT( out == ( first_out_inited + 1 ) ); // ensure we have only advanced one at time
          first_out_inited = out;
          contribs->n0 = in_pixel;
          contribs->n1 = in_pixel;
          coeffs[0]  = coeff;
        }
        else
        {
          // insert on end (always in order)
          if ( coeffs[0] == 0.0f )  // if the first coefficient is zero, then zap it for this coeff
          {
            STBIR_ASSERT( ( in_pixel - contribs->n0 ) == 1 ); // ensure that when we zap, we're at the 2nd pos
            contribs->n0 = in_pixel;
          }
          contribs->n1 = in_pixel;
          STBIR_ASSERT( ( in_pixel - contribs->n0 ) < coefficient_width );
          coeffs[in_pixel - contribs->n0]  = coeff;
        }
      }
    }
  }
}

// fold a contributor's out of range taps back into the image for the edge mode, trim its trailing zero weights,
//   and fold its extents into lowest/highest/widest
static stbir__inline void stbir__fold_and_trim_contributor( stbir_edge edge, stbir__contributors * contributor, float * coefficient_group, int coefficient_width, int input_size, stbir__filter_extent_info * extents )
{
  int input_last_pixel = input_size - 1;
  int i;

  if ( edge == STBIR_EDGE_ZERO )
  {
    // In zero edge mode, just remove the out of bounds pixels completely (their weights are accounted for now).
    if ( contributor->n1 > input_last_pixel )
      contributor->n1 = input_last_pixel;

    if ( contributor->n0 < 0 )
    {
      int skip = -contributor->n0;
      int remaining;

      contributor->n0 = 0;

      // Move the weights down.
      remaining = contributor->n1 - contributor->n0 + 1;
      for (i = 0; i < remaining; i++)
        coefficient_group[i] = coefficient_group[i + skip];
    }
  }
  else if ( ( edge == STBIR_EDGE_CLAMP ) || ( edge == STBIR_EDGE_REFLECT ) )
  {
    // For clamp and reflect, calculate the true in bounds pixel (based on the edge type) and add the weight to it.

    // Right hand side first.
    if ( contributor->n1 > input_last_pixel )
    {
      int first = contributor->n0;
      int last = contributor->n1;
      contributor->n1 = input_last_pixel;
      for (i = input_size; i <= last; i++)
        stbir__insert_coeff( contributor, coefficient_group, stbir__edge_wrap_slow[edge]( i, input_size ), coefficient_group[i - first], coefficient_width );
    }

    // Then the left.
    if ( contributor->n0 < 0 )
    {
      int first = contributor->n0;
      float * coeff = coefficient_group - ( first + 1 );   // the weight of pixel -1
      float first_coeff;

      // Reinsert the weights at their reflected or clamped pixel (insert accumulates if the pixel is already there).
      for (i = -1; i > first; i--)
        stbir__insert_coeff( contributor, coefficient_group, stbir__edge_wrap_slow[edge]( i, input_size ), *coeff--, coefficient_width );

      // Save the first one, since we didn't do it yet (i == n0) - there might be too many coeffs to hold before we shrink.
      first_coeff = coeff[0];

      // Slide all the coeffs down (we accumulated them into the positive pixels) and reset the first pixel.
      contributor->n0 = 0;
      for (i = 0; i <= contributor->n1; i++)
        coefficient_group[i] = coefficient_group[i - first];

      // Now that we have shrunk the contributor, we can insert the first one safely.
      stbir__insert_coeff( contributor, coefficient_group, stbir__edge_wrap_slow[edge]( first, input_size ), first_coeff, coefficient_width );
    }
  }

  // Trim trailing zero coefficients, re-zero the unused tail, and widen the extents to this contributor.
  if ( contributor->n0 <= contributor->n1 )
  {
    int count = contributor->n1 - contributor->n0 + 1;
    while ( count && ( coefficient_group[count - 1] == 0.0f ) )
      --count;

    contributor->n1 = contributor->n0 + count - 1;

    if ( count > 0 )
    {
      if ( contributor->n0 < extents->lowest )
        extents->lowest = contributor->n0;
      if ( contributor->n1 > extents->highest )
        extents->highest = contributor->n1;
      if ( count > extents->widest )
        extents->widest = count;
    }

    for (i = count; i < coefficient_width; i++)
      coefficient_group[i] = 0.0f;
  }
}

static void stbir__cleanup_gathered_coefficients( stbir_edge edge, stbir__filter_extent_info* filter_info, stbir__scale_info * scale_info, int num_contributors, stbir__contributors* contributors, float * coefficients, int coefficient_width )
{
  int input_size = scale_info->input_full_size;
  int numerator = scale_info->scale_numerator;
  int denominator = scale_info->scale_denominator;
  int polyphase = ( ( scale_info->scale_is_rational ) && ( numerator < num_contributors ) );
  int calculated = polyphase ? numerator : num_contributors;   // when polyphase, only the first numerator outputs were calculated
  stbir_real too_small_sum = stbir__too_small_coefficient_sum_for( scale_info );
  int n, i;

  // Make the sum of each output's coefficients 1.
  for (n = 0; n < calculated; n++)
  {
    stbir__contributors * contributor = contributors + n;
    float * coefficient_group = coefficients + (size_t)n * coefficient_width;
    stbir_real total_filter = 0;
    int last = contributor->n1 - contributor->n0;

    for (i = 0; i <= last; i++)
    {
      total_filter += (stbir_real) coefficient_group[i];
      STBIR_ASSERT( ( coefficient_group[i] >= -2.0f ) && ( coefficient_group[i] <= 2.0f ) ); // check for wonky weights
    }

    if ( ( total_filter < too_small_sum ) && ( total_filter > -too_small_sum ) )
    {
      // All coeffs are extremely small, just zero it.
      contributor->n1 = contributor->n0;
      coefficient_group[0] = 0.0f;
    }
    else if ( ( total_filter < ( (stbir_real)1.0 - stbir__renorm_tolerance ) ) || ( total_filter > ( (stbir_real)1.0 + stbir__renorm_tolerance ) ) )
    {
      stbir_real filter_scale = (stbir_real)1.0 / total_filter;
      for (i = 0; i <= last; i++)
        coefficient_group[i] = (float) ( coefficient_group[i] * filter_scale );
    }
  }

  // Polyphase: the rest of the outputs are the first numerator's, moved along by the denominator once per period.
  if ( polyphase )
  {
    for (n = numerator; n < num_contributors; n++)
    {
      contributors[n].n0 = contributors[n - numerator].n0 + denominator;
      contributors[n].n1 = contributors[n - numerator].n1 + denominator;
    }
    stbir_overlapping_memcpy( coefficients + (size_t)numerator * coefficient_width, coefficients, (size_t)( num_contributors - numerator ) * coefficient_width );
  }

  filter_info->lowest = 0x7fffffff;
  filter_info->highest = -0x7fffffff;
  filter_info->widest = -1;
  for (n = 0; n < num_contributors; n++)
    stbir__fold_and_trim_contributor( edge, contributors + n, coefficients + (size_t)n * coefficient_width, coefficient_width, input_size, filter_info );
}

// how many input pixels the horizontal kernel that gets selected for this widest will read
static int stbir__horizontal_read_extent( stbir__contributors const * contribs, int widest )
{
  int mod, extent;

  if ( widest <= 17 )
    return widest;

  mod = widest & 3;
  extent = ( ( ( contribs->n1 - contribs->n0 + 1 ) - mod + 3 ) & ~3 ) + mod;
  if ( extent < ( 8 + mod ) )
    extent = 8 + mod;
  return extent;
}

// the SIMD kernels load a remnant of three coefficients as four, so widths of 4n+3 get a fourth, zero coefficient
#define STBIR__COEFFICIENT_PAD( widest ) ( ( (widest) & 3 ) == 3 )
#define STBIR__COLUMN_ENTRY_FLOATS( widest ) ( 2 + (widest) + STBIR__COEFFICIENT_PAD( widest ) )   // the entry's size in floats

// Repack the coefficient groups from coefficient_width floats each to the run time layout: a stbir__column_entry,
//   two ints (filled in by stbir__fill_column_entries) followed by widest coefficients.
static void stbir__pack_column_entries( int num_entries, float * coefficients, int coefficient_width, int widest )
{
  int entry_floats = STBIR__COLUMN_ENTRY_FLOATS( widest );
  int i, j;

  for (i = 0; i < num_entries; i++)
  {
    // When the entries grow they move up, so do the last one first; otherwise they move down (or stay), first one first.
    int n = ( entry_floats > coefficient_width ) ? ( num_entries - 1 - i ) : i;
    float * coefficient_group = coefficients + (size_t)n * coefficient_width;
    float * entry_coefficients = coefficients + (size_t)n * entry_floats + 2;

    if ( entry_coefficients > coefficient_group )
    {
      for (j = widest - 1; j >= 0; j--)
        entry_coefficients[j] = coefficient_group[j];
    }
    else
    {
      for (j = 0; j < widest; j++)
        entry_coefficients[j] = coefficient_group[j];
    }

    // The kernels that load one float past the coefficients see a zero.
    if ( STBIR__COEFFICIENT_PAD( widest ) )
      entry_coefficients[widest] = 0.0f;
  }
}

// the horizontal kernels read a fixed number of pixels from each contributor's n0 (see stbir__horizontal_read_extent),
//   so contributors near the right edge get moved back (zero weights in front) to keep those reads inside the
//   decoded range. 
static void stbir__clip_contributor_reads( int num_contributors, stbir__contributors* contributors, float * coefficients, int stride, int widest, int row0, int row1 )
{
  int row_end = row1 + 1;
  STBIR__UNUSED( row0 ); // only used in an assert

  // the fixed width kernels (up to 17 taps) read exactly widest pixels from n0 whatever a contributor's own width is,
  //   and the generic ones read in fours (stbir__horizontal_read_extent). So we make sure we never read outside the
  //   decode buffer, by possibly moving the sample area back into the scanline, and putting zero weights first.
  // we start on the right edge and check until we're well past the possible
  //   clip area (2*widest).
  {
    stbir__contributors * contribs = contributors + num_contributors - 1;
    float * coeffs = coefficients + (size_t)stride * ( num_contributors - 1 );

    // go until no chance of clipping (this is usually less than 8 lops)
    while ( ( contribs >= contributors ) && ( ( contribs->n0 + widest*2 ) >= row_end ) )
    {
      // might we clip??
      if ( ( contribs->n0 + widest ) > row_end )
      {
        // past 17 taps the generic loops run, and they read to this contributor's own width rounded up to a four,
        //   not a fixed widest amount - so calculate this
        int stop_range = stbir__horizontal_read_extent( contribs, widest );

        // now see if we still clip with the refined range
        if ( ( contribs->n0 + stop_range ) > row_end )
        {
          int new_n0 = row_end - stop_range;
          int num = contribs->n1 - contribs->n0 + 1;
          int backup = contribs->n0 - new_n0;
          float * from_co = coeffs + num - 1;
          float * to_co = from_co + backup;

          STBIR_ASSERT( ( new_n0 >= row0 ) && ( new_n0 < contribs->n0 ) );

          // move the coeffs over
          while( num )
          {
            *to_co-- = *from_co--;
            --num;
          }
          // zero new positions
          while ( to_co >= coeffs )
            *to_co-- = 0;
          // set new start point (backing up by less than the mod-4 slack keeps the read extent the same, so this can't clip again)
          contribs->n0 = new_n0;
          STBIR_ASSERT( ( contribs->n0 + stbir__horizontal_read_extent( contribs, widest ) ) <= row_end );
        }
      }
      --contribs;
      coeffs -= stride;
    }
  }
}

// pack the coefficient sets into column entries and clip the edge reads
static int stbir__pack_coefficients( int num_contributors, stbir__contributors* contributors, float * coefficients, int coefficient_width, int widest, int row0, int row1 )
{
  stbir__pack_column_entries( num_contributors, coefficients, coefficient_width, widest );
  stbir__clip_contributor_reads( num_contributors, contributors, coefficients + 2, STBIR__COLUMN_ENTRY_FLOATS( widest ), widest, row0, row1 );
  return widest;
}

// For polyphase resizing (rational scale resizing), there is a ton of coefficent sharing across the
//   horizontal scanline. We store the data like this (you need the pro and epi for clamp modes): 
//     [ prologue scanline coeffs ][ big run of repeating coeffs ][ epilogue scanline coeffs ]

// Bounds on how many outputs at each edge have taps outside the image (before folding), from the filter geometry.
static void stbir__polyphase_edge_bounds( stbir__sampler const * samp, void * user_data, int * out_prologue, int * out_epilogue )
{
  stbir__scale_info const * scale_info = &samp->scale_info;
  int num_contributors = samp->num_contributors;
  stbir_real in_pixels_radius;
  int prologue = 0;
  int epilogue = 0;
  int n;

  // the reach of an output's taps, plus how far an approximated fraction's copies can drift from these real positions
  in_pixels_radius = stbir__filter_radius_in_input_pixels( samp->filter_radius, samp->filter_support_new, samp->filter_support_old, scale_info, user_data ) + (stbir_real)scale_info->approximated_drift;

  // Nothing gets folded with wrap.
  if ( samp->edge != STBIR_EDGE_WRAP )
  {
    // Looping through out pixels from the left edge, until one reads nothing before the image
    for (n = 0; n < num_contributors; n++)
    {
      stbir_real in_first_pixel = ( (stbir_real)n + (stbir_real)0.5 + scale_info->pixel_shift ) * scale_info->inv_scale - in_pixels_radius;
      if ( stbir__floor_to_int( in_first_pixel + (stbir_real)0.5 ) >= 0 )
        break;
      ++prologue;
    }

    // And from the right edge, until one reads nothing past it
    for (n = num_contributors - 1; n >= 0; n--)
    {
      stbir_real in_last_pixel = ( (stbir_real)n + (stbir_real)0.5 + scale_info->pixel_shift ) * scale_info->inv_scale + in_pixels_radius;
      if ( stbir__floor_to_int( in_last_pixel - (stbir_real)0.5 ) < scale_info->input_full_size )
        break;
      ++epilogue;
    }

    // A pixel of slack each way: this is the real chain, the generators' positions can be exact.
    ++prologue;
    ++epilogue;
  }

  *out_prologue = prologue;
  *out_epilogue = epilogue;
}

// for polyphase (rational) scales, we want to unroll the sharing a bit just so we aren't
//   constantly breaking out of a tight loop to back up the coeff sharing. 
#define STBIR__POLYPHASE_MIN_BLOCK 256

// decide whether this (gather) sampler shares its polyphase coefficient sets, and size its tables accordingly.
//   called before the allocation pass, so it can only use the sampler's geometry.
static void stbir__plan_polyphase_sharing( stbir__sampler * samp, int reads_get_clipped, void * user_data )
{
  int num_contributors = samp->num_contributors;
  int numerator = (int) samp->scale_info.scale_numerator;
  int prologue, epilogue, period, repeating;

  samp->shared_period = 0;
  samp->shared_prologue = 0;
  samp->shared_epilogue = 0;

  if ( ( !samp->is_gather ) || ( !samp->scale_info.scale_is_rational ) || ( numerator >= num_contributors ) )
    return;

  stbir__polyphase_edge_bounds( samp, user_data, &prologue, &epilogue );

  // the prologue is whole periods, so an output's phase is its index mod numerator
  prologue = ( ( prologue + numerator - 1 ) / numerator ) * numerator;

  // the horizontal's clip (stbir__clip_contributor_reads) can also move outputs within 2*widest input pixels of the
  //   right edge: 2*widest*scale outputs, bounded with the coefficient width since widest isn't known yet
  if ( reads_get_clipped )
    epilogue += (int)( (stbir_real)2.0 * (stbir_real)samp->coefficient_width * samp->scale_info.scale ) + 2;

  // the block is one period of sets (one set at period 1, run with an entry stride of zero). the horizontal walks a
  //   row through it, so its block is padded out to keep the kernel calls long; the vertical fetches a set per row by index
  period = numerator;
  if ( ( numerator > 1 ) && ( reads_get_clipped ) )
  {
    while ( period < STBIR__POLYPHASE_MIN_BLOCK )
      period += numerator;
  }

  // not enough repeating outputs to be worth it
  repeating = num_contributors - prologue - epilogue;
  if ( repeating < period )
    return;

  samp->shared_period = period;
  samp->shared_prologue = prologue;
  samp->shared_epilogue = epilogue;
  samp->coefficients_size = (size_t)( prologue + period + epilogue ) * STBIR__COLUMN_ENTRY_FLOATS( samp->coefficient_width ) * sizeof(float) + sizeof(float)*STBIR_INPUT_CALLBACK_PADDING;
  samp->contributors_size = (size_t)( prologue + period + epilogue + 1 ) * sizeof(stbir__contributors);  // one per coefficient set (build time only)
}

// the shared layout keeps contributors for just the outputs that have their own coefficient sets, in set order:
//   [ P prologue outputs ][ block outputs ][ E epilogue outputs ]. 
static void stbir__shared_output_range( stbir__sampler const * samp, int x, int * n0, int * n1 )
{
  int prologue = samp->shared_prologue;
  int period = samp->shared_period;
  int epilogue = samp->shared_epilogue;
  int num_contributors = samp->num_contributors;
  stbir__contributors const * contributor;
  int shift = 0;

  if ( x >= num_contributors - epilogue )
    contributor = samp->contributors + prologue + period + ( x - ( num_contributors - epilogue ) );
  else if ( x >= prologue )
  {
    contributor = samp->contributors + prologue + ( ( x - prologue ) % period );
    shift = ( ( x - prologue ) / period ) * ( period / (int) samp->scale_info.scale_numerator ) * (int) samp->scale_info.scale_denominator;
  }
  else
    contributor = samp->contributors + x;

  *n0 = contributor->n0 + shift;
  *n1 = contributor->n1 + shift;
}

// n0/n1 of output x for either layout
static void stbir__output_range( stbir__sampler const * samp, int x, int * n0, int * n1 )
{
  if ( samp->shared_period )
    stbir__shared_output_range( samp, x, n0, n1 );
  else
  {
    *n0 = samp->contributors[ x ].n0;
    *n1 = samp->contributors[ x ].n1;
  }
}

// The shared layout's version of stbir__cleanup_gathered_coefficients.
static void stbir__cleanup_shared_coefficients( stbir__sampler * samp )
{
  stbir_edge edge = samp->edge;
  stbir__scale_info const * scale_info = &samp->scale_info;
  int input_size = scale_info->input_full_size;
  int num_contributors = samp->num_contributors;
  int coefficient_width = samp->coefficient_width;
  int numerator = (int) scale_info->scale_numerator;
  int denominator = (int) scale_info->scale_denominator;
  int prologue = samp->shared_prologue;
  int period = samp->shared_period;
  int epilogue = samp->shared_epilogue;
  stbir__contributors * contributors = samp->contributors;
  float * coefficients = samp->coefficients;
  float * phase_coefficients = coefficients + (size_t)prologue * coefficient_width;
  stbir__filter_extent_info * extents = &samp->extent_info;
  stbir_real too_small_sum = stbir__too_small_coefficient_sum_for( &samp->scale_info );
  int n, i;

  // Make the sum of each phase's coefficients 1 (as the plain path does for its first numerator outputs).
  for (n = 0; n < numerator; n++)
  {
    stbir__contributors * contributor = contributors + n;
    float * coefficient_group = phase_coefficients + (size_t)n * coefficient_width;
    stbir_real total_filter = 0;
    int last = contributor->n1 - contributor->n0;

    for (i = 0; i <= last; i++)
    {
      total_filter += (stbir_real) coefficient_group[i];
      STBIR_ASSERT( ( coefficient_group[i] >= -2.0f ) && ( coefficient_group[i] <= 2.0f ) ); // check for wonky weights
    }

    if ( ( total_filter < too_small_sum ) && ( total_filter > -too_small_sum ) )
    {
      // All coeffs are extremely small, just zero it.
      contributor->n1 = contributor->n0;
      coefficient_group[0] = 0.0f;
    }
    else if ( ( total_filter < ( (stbir_real)1.0 - stbir__renorm_tolerance ) ) || ( total_filter > ( (stbir_real)1.0 + stbir__renorm_tolerance ) ) )
    {
      stbir_real filter_scale = (stbir_real)1.0 / total_filter;
      for (i = 0; i <= last; i++)
        coefficient_group[i] = (float) ( coefficient_group[i] * filter_scale );
    }
  }

  // The contributors for the prologue and period outputs (0..prologue+period-1) are the phase's, moved along by the
  //   denominator once per period. The epilogue outputs (the last epilogue of the image) follow them.
  for (n = numerator; n < prologue + period; n++)
  {
    contributors[n].n0 = contributors[n - numerator].n0 + denominator;
    contributors[n].n1 = contributors[n - numerator].n1 + denominator;
  }
  for (n = 0; n < epilogue; n++)
  {
    int out_pixel = num_contributors - epilogue + n;
    contributors[prologue + period + n].n0 = contributors[out_pixel % numerator].n0 + ( out_pixel / numerator ) * denominator;
    contributors[prologue + period + n].n1 = contributors[out_pixel % numerator].n1 + ( out_pixel / numerator ) * denominator;
  }

  // The period is the phases repeated; the prologue and epilogue groups start out as copies of their phase.
  for (n = numerator; n < period; n++)
    STBIR_MEMCPY( phase_coefficients + (size_t)n * coefficient_width, phase_coefficients + (size_t)( n % numerator ) * coefficient_width, coefficient_width * sizeof(float) );
  for (n = 0; n < prologue; n++)
    STBIR_MEMCPY( coefficients + (size_t)n * coefficient_width, phase_coefficients + (size_t)( n % numerator ) * coefficient_width, coefficient_width * sizeof(float) );
  for (n = 0; n < epilogue; n++)
    STBIR_MEMCPY( coefficients + (size_t)( prologue + period + n ) * coefficient_width, phase_coefficients + (size_t)( ( num_contributors - epilogue + n ) % numerator ) * coefficient_width, coefficient_width * sizeof(float) );

  // Fold and trim every group. The period's outputs are never at an edge (that is what the prologue and epilogue are
  //   for), so for them this is just the trim, which is the same for every copy of a phase - the period stays periodic.
  extents->lowest = 0x7fffffff;
  extents->highest = -0x7fffffff;
  extents->widest = -1;
  for (n = 0; n < prologue + period + epilogue; n++)
  {
    STBIR_ASSERT( ( n < prologue ) || ( n >= prologue + period ) || ( edge == STBIR_EDGE_WRAP ) || ( ( contributors[n].n0 >= 0 ) && ( contributors[n].n1 < input_size ) ) );
    stbir__fold_and_trim_contributor( edge, contributors + n, coefficients + (size_t)n * coefficient_width, coefficient_width, input_size, extents );
  }

  // lowest and highest are over every output: the extremes are in the prologue and first period, and the last period
  //   and epilogue.
  {
    int n0, n1;
    stbir__shared_output_range( samp, num_contributors - epilogue - 1, &n0, &n1 );
    if ( n1 > extents->highest )
      extents->highest = n1;
  }
}

// the shared layout's version of stbir__pack_coefficients
static int stbir__pack_shared_coefficients( stbir__sampler * samp, int row0, int row1 )
{
  int num_contributors = samp->num_contributors;
  int widest = samp->extent_info.widest;
  int entry_floats = STBIR__COLUMN_ENTRY_FLOATS( widest );
  int prologue = samp->shared_prologue;
  int period = samp->shared_period;
  int epilogue = samp->shared_epilogue;
  int n0, n1;

  stbir__pack_column_entries( prologue + period + epilogue, samp->coefficients, samp->coefficient_width, widest );
  stbir__clip_contributor_reads( epilogue, samp->contributors + prologue + period, samp->coefficients + (size_t)( prologue + period ) * entry_floats + 2, entry_floats, widest, row0, row1 );

  // the clipping never needed to reach past the epilogue (the last periodic output is where it would have gone next)
  stbir__shared_output_range( samp, num_contributors - epilogue - 1, &n0, &n1 );
  STBIR_ASSERT( ( n0 + widest * 2 ) < row1 + 1 );
  return widest;
}

// the packed column entry at an index (the entries are entry_floats apart)
static stbir__column_entry * stbir__column_entry_at( stbir__sampler const * samp, int index )
{
  return (stbir__column_entry *)( samp->coefficients + (size_t)index * samp->entry_floats );
}

// fill in the column entries' advance and width from the build time contributors (after the entries have been
//   packed, and after the contributors have been read for the last time - the column slice table). returns 0 when
//   an advance doesn't fit the entry's int - a horizontal downsample of around 2^29:1 at four channels
static int stbir__fill_column_entries( stbir__sampler * samp, int channels )
{
  stbir__contributors const * contributors = samp->contributors;
  int num_entries = samp->shared_period ? ( samp->shared_prologue + samp->shared_period + samp->shared_epilogue ) : samp->num_contributors;
  int i;

  if ( samp->shared_period )
  {
    int prologue = samp->shared_prologue;
    int period = samp->shared_period;
    samp->shared_block_n0 = contributors[prologue].n0;
    samp->shared_block_shift = ( period / (int) samp->scale_info.scale_numerator ) * (int) samp->scale_info.scale_denominator;
    samp->shared_epilogue_n0 = contributors[prologue + period].n0;
  }

  // each entry says how far the decode pointer moves to the NEXT column. the block's last entry says how far to the
  //   block's first column one pass on (the wrap)
  for (i = 0; i < num_entries; i++)
  {
    stbir__column_entry * entry = stbir__column_entry_at( samp, i );
    int advance;

    if ( ( samp->shared_period ) && ( i == samp->shared_prologue + samp->shared_period - 1 ) )
      advance = ( samp->shared_block_n0 + samp->shared_block_shift ) - contributors[i].n0;
    else
      advance = ( i + 1 < num_entries ) ? ( contributors[i + 1].n0 - contributors[i].n0 ) : 0;

    if ( ( (stbir_int64)advance * channels <= -0x7fffffff ) || ( (stbir_int64)advance * channels >= 0x7fffffff ) )
      return 0; // error

    entry->advance = advance * channels;
    entry->width = contributors[i].n1 - contributors[i].n0 + 1;
  }
  return 1;
}

// which coefficient set (column entry) output x uses, for either layout
static int stbir__output_entry_index( stbir__sampler const * samp, int x )
{
  if ( samp->shared_period )
  {
    int prologue = samp->shared_prologue;
    int period = samp->shared_period;
    int epilogue = samp->shared_epilogue;
    int num_contributors = samp->num_contributors;

    if ( x >= num_contributors - epilogue )
      return prologue + period + ( x - ( num_contributors - epilogue ) );
    if ( x >= prologue )
      return prologue + ( ( x - prologue ) % period );
  }
  return x;
}

static float const * stbir__output_coefficients( stbir__sampler const * samp, int x )
{
  return samp->coefficients + (size_t)stbir__output_entry_index( samp, x ) * samp->coefficient_width;
}

// output x's packed column entry
static stbir__column_entry const * stbir__output_column_entry( stbir__sampler const * samp, int x )
{
  return stbir__column_entry_at( samp, stbir__output_entry_index( samp, x ) );
}

// Vertical scatter functions - used when we are downsampling around 32:1 or more (horizontal is always gather)
//   advice-wise, if you are downsampling this amount, then you'd better off doing the downsample in a couple steps
static void stbir__calculate_coefficients_scatter( stbir__kernel_callback_new * kernel_new, stbir__kernel_callback * kernel_old, stbir__scale_info const * scale_info, int in_pixel, int out_first_pixel, int out_last_pixel, stbir__contributors* contributor, float* coefficient_group, void * user_data )
{
  int i;

  contributor->n0 = out_first_pixel;
  contributor->n1 = out_last_pixel;

  for (i = 0; i <= out_last_pixel - out_first_pixel; i++)
  {
    stbir_real weight = stbir__kernel_weight( kernel_new, kernel_old, stbir__tap_offset( scale_info, i + out_first_pixel, in_pixel, 0 ), scale_info->scale, user_data );
    float coeff = 0.0f;

    if ( ( weight >= stbir__too_small_coefficient ) || ( weight <= -stbir__too_small_coefficient ) )
      coeff = (float) ( weight * scale_info->scale );

    coefficient_group[i] = coeff;
  }
}

// Normalize each output's coefficients: summed and scaled in stbir_real, each then rounded to float
static void stbir__normalize_scatter_coefficients( stbir__contributors* contributors, float* coefficients, int coefficient_width, int num_contributors, int output_size, stbir_real too_small_sum )
{
  int i, j, first = 0;

  for (i = 0; i < output_size; i++)
  {
    stbir_real total = 0;
    stbir_real filter_scale;

    while ( ( first < num_contributors ) && ( contributors[first].n1 < i ) )
      first++;

    for (j = first; j < num_contributors; j++)
    {
      if (i >= contributors[j].n0 && i <= contributors[j].n1)
        total += (stbir_real) coefficients[ (size_t)j * coefficient_width + i - contributors[j].n0 ];
      else if (i < contributors[j].n0)
        break;
    }

    if ( ( total < too_small_sum ) && ( total > -too_small_sum ) )
      filter_scale = 0;   // all coeffs are extremely small, just zero them
    else if ( ( total < ( (stbir_real)1.0 - stbir__renorm_tolerance ) ) || ( total > ( (stbir_real)1.0 + stbir__renorm_tolerance ) ) )
      filter_scale = (stbir_real)1.0 / total;
    else
      continue;

    for (j = first; j < num_contributors; j++)
    {
      if (i >= contributors[j].n0 && i <= contributors[j].n1)
        coefficients[ (size_t)j * coefficient_width + i - contributors[j].n0 ] = (float) ( coefficients[ (size_t)j * coefficient_width + i - contributors[j].n0 ] * filter_scale );
      else if (i < contributors[j].n0)
        break;
    }
  }
}

// Polyphase: only the first numerator outputs were calculated. From first_copy on, each contributor is the one a
//   denominator before it with its outputs moved along by the numerator (calculated ones keep their outputs, the copies follow).
static void stbir__replicate_scatter_coefficients( stbir__contributors* contributors, float* coefficients, int coefficient_width, int num_contributors, int first_copy, int calculated_end, int numerator, int denominator, int output_size )
{
  int j, i;

  for (j = first_copy; j < num_contributors; j++)
  {
    stbir__contributors* contributor = contributors + j;
    stbir__contributors* source = contributors + j - denominator;
    float* coefficient_group = coefficients + (size_t)j * coefficient_width;
    float* source_group = coefficients + (size_t)( j - denominator ) * coefficient_width;

    if ( j >= calculated_end )
    {
      contributor->n0 = 0;
      contributor->n1 = -1;

      // the whole source fits: copy it
      if ( source->n1 + numerator < output_size )
      {
        contributor->n0 = source->n0 + numerator;
        contributor->n1 = source->n1 + numerator;
        STBIR_MEMCPY( coefficient_group, source_group, (size_t)( source->n1 - source->n0 + 1 ) * sizeof(float) );
        continue;
      }
    }

    // append the source's outputs that are in range
    for (i = 0; i <= source->n1 - source->n0; i++)
    {
      int out = source->n0 + i + numerator;
      if ( out >= output_size )
        break;
      if ( contributor->n1 < contributor->n0 )
      {
        contributor->n0 = contributor->n1 = out;
        coefficient_group[0] = source_group[i];
      }
      else
        stbir__insert_coeff( contributor, coefficient_group, out, source_group[i], coefficient_width );
    }
  }
}

// The out of range input scanlines: clamp and reflect add their weights to the scanlines they map to (right edge
//   ascending, then left edge descending - the gather's order), zero drops them, wrap keeps them.
static void stbir__fold_scatter_edges( stbir__sampler * samp, int coefficient_width )
{
  stbir_edge edge = samp->edge;
  int input_size = samp->scale_info.input_full_size;
  int filter_pixel_margin = samp->filter_pixel_margin;
  stbir__contributors* contributors = samp->contributors;
  float* coefficients = samp->coefficients;
  int n, i;

  if ( ( edge == STBIR_EDGE_CLAMP ) || ( edge == STBIR_EDGE_REFLECT ) )
  {
    int pass;
    for (pass = 0; pass < 2; pass++)
    {
      int from = pass ? -1 : input_size;
      int to = pass ? -filter_pixel_margin - 1 : input_size + filter_pixel_margin;
      int step = pass ? -1 : 1;
      for (n = from; n != to; n += step)
      {
        stbir__contributors* contributor = contributors + n + filter_pixel_margin;
        float* coefficient_group = coefficients + (size_t)( n + filter_pixel_margin ) * coefficient_width;
        int folded = stbir__edge_wrap_slow[edge]( n, input_size ) + filter_pixel_margin;

        for (i = 0; i <= contributor->n1 - contributor->n0; i++)
          stbir__insert_coeff( contributors + folded, coefficients + (size_t)folded * coefficient_width, contributor->n0 + i, coefficient_group[i], coefficient_width );

        contributor->n0 = 0;
        contributor->n1 = -1;
      }
    }
  }
  else if ( edge == STBIR_EDGE_ZERO )
  {
    for (n = 0; n < filter_pixel_margin; n++)
    {
      contributors[n].n0 = 0;
      contributors[n].n1 = -1;
      contributors[input_size + 2 * filter_pixel_margin - 1 - n].n0 = 0;
      contributors[input_size + 2 * filter_pixel_margin - 1 - n].n1 = -1;
    }
  }
}

// Optimize step - skip zero coefficients at either end, and pack the groups from the build width down to the final width
//   (an output past the width is dropped). Also notes the first and last input scanlines with any weight.
static void stbir__pack_scatter_coefficients( stbir__sampler * samp, int build_width )
{
  int coefficient_width = samp->coefficient_width;
  int filter_pixel_margin = samp->filter_pixel_margin;
  int num_contributors = samp->num_contributors;
  stbir__contributors* contributors = samp->contributors;
  float* coefficients = samp->coefficients;
  int j, i;

  samp->extent_info.lowest = 0x7fffffff;
  samp->extent_info.highest = -0x7fffffff;
  samp->extent_info.widest = coefficient_width;

  for (j = 0; j < num_contributors; j++)
  {
    stbir__contributors* contributor = contributors + j;
    float const* source_group = coefficients + (size_t)j * build_width;
    float* coefficient_group = coefficients + (size_t)j * coefficient_width;   // at or before the source, so packing forward in place is safe
    int range = contributor->n1 - contributor->n0 + 1;
    int skip = 0;

    while ( ( skip < range ) && ( source_group[skip] == 0.0f ) )
      skip++;
    range -= skip;
    if ( range > coefficient_width )
      range = coefficient_width;
    while ( ( range > 0 ) && ( source_group[skip + range - 1] == 0.0f ) )
      range--;

    for (i = 0; i < range; i++)
      coefficient_group[i] = source_group[i + skip];

    if ( range <= 0 )
    {
      contributor->n0 = 0;
      contributor->n1 = -1;
      continue;
    }
    contributor->n0 += skip;
    contributor->n1 = contributor->n0 + range - 1;

    if ( j - filter_pixel_margin < samp->extent_info.lowest )
      samp->extent_info.lowest = j - filter_pixel_margin;
    samp->extent_info.highest = j - filter_pixel_margin;
  }
}

static void stbir__calculate_scatter_filters( stbir__sampler * samp, stbir_real in_pixels_radius, stbir_real support, void * user_data )
{
  stbir__scale_info const * scale_info = &samp->scale_info;
  int output_size = scale_info->output_sub_size;
  int filter_pixel_margin = samp->filter_pixel_margin;
  int num_contributors = samp->num_contributors;
  int build_width = samp->coefficient_width + 1;
  int polyphase = ( scale_info->scale_is_rational ) && ( (int)scale_info->scale_numerator < output_size );
  int numerator = polyphase ? (int)scale_info->scale_numerator : output_size;
  int calculated_end = num_contributors;   // the first contributor with nothing calculated
  int first_weighted = -1;                 // the first contributor with any weight
  int n;

  // Looping through in pixels
  for (n = 0; n < num_contributors; n++)
  {
    int out_first_pixel, out_last_pixel;

    stbir__input_out_pixel_range( scale_info, n - filter_pixel_margin, in_pixels_radius, support, &out_first_pixel, &out_last_pixel );

    // when polyphase, you only have to do coeffs up to the numerator count
    if ( out_first_pixel >= numerator )
    {
      calculated_end = n;
      break;
    }
    if ( out_last_pixel >= numerator )
      out_last_pixel = numerator - 1;

    stbir__calculate_coefficients_scatter( samp->filter_kernel_new, samp->filter_kernel_old, scale_info, n - filter_pixel_margin, out_first_pixel, out_last_pixel, samp->contributors + n, samp->coefficients + (size_t)n * build_width, user_data );

    if ( ( first_weighted < 0 ) && ( out_last_pixel >= out_first_pixel ) )
      first_weighted = n;
  }
  for (n = calculated_end; n < num_contributors; n++)
  {
    samp->contributors[n].n0 = 0;
    samp->contributors[n].n1 = -1;
  }

  stbir__normalize_scatter_coefficients( samp->contributors, samp->coefficients, build_width, calculated_end, numerator, stbir__too_small_coefficient_sum_for( scale_info ) );

  // the period is the rational's, which can be a little off the true one, so the copies start where it says
  if ( ( polyphase ) && ( first_weighted >= 0 ) )
    stbir__replicate_scatter_coefficients( samp->contributors, samp->coefficients, build_width, num_contributors, first_weighted + (int)scale_info->scale_denominator, calculated_end, numerator, (int)scale_info->scale_denominator, output_size );

  stbir__fold_scatter_edges( samp, build_width );
  stbir__pack_scatter_coefficients( samp, build_width );
}

static void stbir__calculate_filters( stbir__sampler * samp, void * user_data STBIR_ONLY_PROFILE_BUILD_GET_INFO )
{
  stbir_real scale = samp->scale_info.scale;
  stbir__kernel_callback_new * kernel_new = samp->filter_kernel_new;
  stbir__kernel_callback * kernel_old = samp->filter_kernel_old;
  stbir_real in_pixels_radius = stbir__filter_radius_in_input_pixels( samp->filter_radius, samp->filter_support_new, samp->filter_support_old, &samp->scale_info, user_data );
  stbir_real inv_scale = samp->scale_info.inv_scale;
  int input_full_size = samp->scale_info.input_full_size;
  int gather_num_contributors = samp->num_contributors;
  stbir__contributors* gather_contributors = samp->contributors;
  float * gather_coeffs = samp->coefficients;
  int gather_coefficient_width = samp->coefficient_width;

  // with shared polyphase sets the generated phases go after the prologue sets
  if ( samp->shared_period )
    gather_coeffs += (size_t)samp->shared_prologue * gather_coefficient_width;

  switch ( samp->is_gather )
  {
    case 1: // gather upsample
    {
      stbir_real filter_support = in_pixels_radius;   // the filter's own pixels are input pixels
      stbir_real out_pixels_radius = filter_support * scale;

      stbir__calculate_coefficients_for_gather_upsample( out_pixels_radius, filter_support, kernel_new, kernel_old, &samp->scale_info, gather_num_contributors, gather_contributors, gather_coeffs, gather_coefficient_width, samp->edge, user_data );

      STBIR_PROFILE_BUILD_START( cleanup );
      if ( samp->shared_period )
        stbir__cleanup_shared_coefficients( samp );
      else
        stbir__cleanup_gathered_coefficients( samp->edge, &samp->extent_info, &samp->scale_info, gather_num_contributors, gather_contributors, gather_coeffs, gather_coefficient_width );
      STBIR_PROFILE_BUILD_END( cleanup );
    }
    break;

    case 0: // scatter downsample (only on vertical)
    case 2: // gather downsample
    {
      stbir_real filter_support = in_pixels_radius / inv_scale;   // in output pixels
      int filter_pixel_margin = samp->filter_pixel_margin;
      int input_end = input_full_size + filter_pixel_margin;

      if ( !samp->is_gather )
      {
        STBIR_PROFILE_BUILD_START( scatter );
        stbir__calculate_scatter_filters( samp, in_pixels_radius, filter_support, user_data );
        STBIR_PROFILE_BUILD_END( scatter );
        break;
      }

      stbir__calculate_coefficients_for_gather_downsample( -filter_pixel_margin, input_end, in_pixels_radius, filter_support, kernel_new, kernel_old, &samp->scale_info, gather_coefficient_width, gather_num_contributors, gather_contributors, gather_coeffs, user_data );

      STBIR_PROFILE_BUILD_START( cleanup );
      if ( samp->shared_period )
        stbir__cleanup_shared_coefficients( samp );
      else
        stbir__cleanup_gathered_coefficients( samp->edge, &samp->extent_info, &samp->scale_info, gather_num_contributors, gather_contributors, gather_coeffs, gather_coefficient_width );
      STBIR_PROFILE_BUILD_END( cleanup );
    }
    break;
  }
}



//========================================================================================================
// scanline decoders and encoders

#define stbir__coder_min_num 1
#define STB_IMAGE_RESIZE_DO_CODERS
#include STBIR__HEADER_FILENAME

#define stbir__decode_suffix BGRA
#define stbir__decode_swizzle
#define stbir__decode_order0  2
#define stbir__decode_order1  1
#define stbir__decode_order2  0
#define stbir__decode_order3  3
#define stbir__encode_order0  2
#define stbir__encode_order1  1
#define stbir__encode_order2  0
#define stbir__encode_order3  3
#define stbir__coder_min_num 4
#define STB_IMAGE_RESIZE_DO_CODERS
#include STBIR__HEADER_FILENAME

#define stbir__decode_suffix ARGB
#define stbir__decode_swizzle
#define stbir__decode_order0  1
#define stbir__decode_order1  2
#define stbir__decode_order2  3
#define stbir__decode_order3  0
#define stbir__encode_order0  3
#define stbir__encode_order1  0
#define stbir__encode_order2  1
#define stbir__encode_order3  2
#define stbir__coder_min_num 4
#define STB_IMAGE_RESIZE_DO_CODERS
#include STBIR__HEADER_FILENAME

#define stbir__decode_suffix ABGR
#define stbir__decode_swizzle
#define stbir__decode_order0  3
#define stbir__decode_order1  2
#define stbir__decode_order2  1
#define stbir__decode_order3  0
#define stbir__encode_order0  3
#define stbir__encode_order1  2
#define stbir__encode_order2  1
#define stbir__encode_order3  0
#define stbir__coder_min_num 4
#define STB_IMAGE_RESIZE_DO_CODERS
#include STBIR__HEADER_FILENAME

#define stbir__decode_suffix AR
#define stbir__decode_swizzle
#define stbir__decode_order0  1
#define stbir__decode_order1  0
#define stbir__decode_order2  3
#define stbir__decode_order3  2
#define stbir__encode_order0  1
#define stbir__encode_order1  0
#define stbir__encode_order2  3
#define stbir__encode_order3  2
#define stbir__coder_min_num 2
#define STB_IMAGE_RESIZE_DO_CODERS
#include STBIR__HEADER_FILENAME


// fancy alpha means we expand to keep both premultipied and non-premultiplied color channels
static void stbir__fancy_alpha_weight_4ch( float * out_buffer, ptrdiff_t width_times_channels )
{
  float STBIR_STREAMOUT_PTR(*) out = out_buffer;
  float const * end_decode = out_buffer + ( width_times_channels / 4 ) * 7;  // decode buffer aligned to end of out_buffer
  float STBIR_STREAMOUT_PTR(*) decode = (float*)end_decode - width_times_channels;

  // fancy alpha is stored internally as R G B A Rpm Gpm Bpm

  #ifdef STBIR_SIMD

  #ifdef STBIR_SIMD8
  decode += 16;
  STBIR_NO_UNROLL_LOOP_START
  while ( decode <= end_decode )
  {
    stbir__simdf8 d0,d1,a0,a1,p0,p1;
    STBIR_NO_UNROLL(decode);
    stbir__simdf8_load( d0, decode-16 );
    stbir__simdf8_load( d1, decode-16+8 );
    stbir__simdf8_0123to33333333( a0, d0 );
    stbir__simdf8_0123to33333333( a1, d1 );
    stbir__simdf8_mult( p0, a0, d0 );
    stbir__simdf8_mult( p1, a1, d1 );
    stbir__simdf8_bot4s( a0, d0, p0 );
    stbir__simdf8_bot4s( a1, d1, p1 );
    stbir__simdf8_top4s( d0, d0, p0 );
    stbir__simdf8_top4s( d1, d1, p1 );
    stbir__simdf8_store ( out, a0 );
    stbir__simdf8_store ( out+7, d0 );
    stbir__simdf8_store ( out+14, a1 );
    stbir__simdf8_store ( out+21, d1 );
    decode += 16;
    out += 28;
  }
  decode -= 16;
  #else
  decode += 8;
  STBIR_NO_UNROLL_LOOP_START
  while ( decode <= end_decode )
  {
    stbir__simdf d0,a0,d1,a1,p0,p1;
    STBIR_NO_UNROLL(decode);
    stbir__simdf_load( d0, decode-8 );
    stbir__simdf_load( d1, decode-8+4 );
    stbir__simdf_0123to3333( a0, d0 );
    stbir__simdf_0123to3333( a1, d1 );
    stbir__simdf_mult( p0, a0, d0 );
    stbir__simdf_mult( p1, a1, d1 );
    stbir__simdf_store ( out, d0 );
    stbir__simdf_store ( out+4, p0 );
    stbir__simdf_store ( out+7, d1 );
    stbir__simdf_store ( out+7+4, p1 );
    decode += 8;
    out += 14;
  }
  decode -= 8;
  #endif

  // might be one last odd pixel
  #ifdef STBIR_SIMD8
  STBIR_NO_UNROLL_LOOP_START
  while ( decode < end_decode )
  #else
  if ( decode < end_decode )
  #endif
  {
    stbir__simdf d,a,p;
    STBIR_NO_UNROLL(decode);
    stbir__simdf_load( d, decode );
    stbir__simdf_0123to3333( a, d );
    stbir__simdf_mult( p, a, d );
    stbir__simdf_store ( out, d );
    stbir__simdf_store ( out+4, p );
    decode += 4;
    out += 7;
  }

  #else

  while( decode < end_decode )
  {
    float r = decode[0], g = decode[1], b = decode[2], alpha = decode[3];
    out[0] = r;
    out[1] = g;
    out[2] = b;
    out[3] = alpha;
    out[4] = r * alpha;
    out[5] = g * alpha;
    out[6] = b * alpha;
    out += 7;
    decode += 4;
  }

  #endif
}

static void stbir__fancy_alpha_weight_2ch( float * out_buffer, ptrdiff_t width_times_channels )
{
  float STBIR_STREAMOUT_PTR(*) out = out_buffer;
  float const * end_decode = out_buffer + ( width_times_channels / 2 ) * 3;
  float STBIR_STREAMOUT_PTR(*) decode = (float*)end_decode - width_times_channels;

  //  for fancy alpha, turns into: [X A Xpm][X A Xpm],etc

  #ifdef STBIR_SIMD

  decode += 8;
  if ( decode <= end_decode )
  {
    STBIR_NO_UNROLL_LOOP_START
    do {
      #ifdef STBIR_SIMD8
      stbir__simdf8 d0,a0,p0;
      STBIR_NO_UNROLL(decode);
      stbir__simdf8_load( d0, decode-8 );
      stbir__simdf8_0123to11331133( p0, d0 );
      stbir__simdf8_0123to00220022( a0, d0 );
      stbir__simdf8_mult( p0, p0, a0 );

      stbir__simdf_store2( out, stbir__if_simdf8_cast_to_simdf4( d0 ) );
      stbir__simdf_store( out+2, stbir__if_simdf8_cast_to_simdf4( p0 ) );
      stbir__simdf_store2h( out+3, stbir__if_simdf8_cast_to_simdf4( d0 ) );

      stbir__simdf_store2( out+6, stbir__simdf8_gettop4( d0 ) );
      stbir__simdf_store( out+8, stbir__simdf8_gettop4( p0 ) );
      stbir__simdf_store2h( out+9, stbir__simdf8_gettop4( d0 ) );
      #else
      stbir__simdf d0,a0,d1,a1,p0,p1;
      STBIR_NO_UNROLL(decode);
      stbir__simdf_load( d0, decode-8 );
      stbir__simdf_load( d1, decode-8+4 );
      stbir__simdf_0123to1133( p0, d0 );
      stbir__simdf_0123to1133( p1, d1 );
      stbir__simdf_0123to0022( a0, d0 );
      stbir__simdf_0123to0022( a1, d1 );
      stbir__simdf_mult( p0, p0, a0 );
      stbir__simdf_mult( p1, p1, a1 );

      stbir__simdf_store2( out, d0 );
      stbir__simdf_store( out+2, p0 );
      stbir__simdf_store2h( out+3, d0 );

      stbir__simdf_store2( out+6, d1 );
      stbir__simdf_store( out+8, p1 );
      stbir__simdf_store2h( out+9, d1 );
      #endif
      decode += 8;
      out += 12;
    } while ( decode <= end_decode );
  }
  decode -= 8;
  #endif

  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( decode < end_decode )
  {
    float x = decode[0], y = decode[1];
    STBIR_SIMD_NO_UNROLL(decode);
    out[0] = x;
    out[1] = y;
    out[2] = x * y;
    out += 3;
    decode += 2;
  }
}

static void stbir__fancy_alpha_unweight_4ch( float * encode_buffer, ptrdiff_t width_times_channels )
{
  float STBIR_SIMD_STREAMOUT_PTR(*) encode = encode_buffer;
  float STBIR_SIMD_STREAMOUT_PTR(*) input = encode_buffer;
  float const * end_output = encode_buffer + width_times_channels;

  // fancy RGBA is stored internally as R G B A Rpm Gpm Bpm

  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float alpha = input[3];
#ifdef STBIR_SIMD
    stbir__simdf i,ia;
    STBIR_SIMD_NO_UNROLL(encode);
    if ( alpha < stbir__alpha_too_small_to_divide )
    {
      stbir__simdf_load( i, input );
      stbir__simdf_store( encode, i );
    }
    else
    {
      stbir__simdf_load1frep4( ia, 1.0f / alpha );
      stbir__simdf_load( i, input+4 );
      stbir__simdf_mult( i, i, ia );
      stbir__simdf_store( encode, i );
      encode[3] = alpha;
    }
#else
    if ( alpha < stbir__alpha_too_small_to_divide )
    {
      encode[0] = input[0];
      encode[1] = input[1];
      encode[2] = input[2];
    }
    else
    {
      float ialpha = 1.0f / alpha;
      encode[0] = input[4] * ialpha;
      encode[1] = input[5] * ialpha;
      encode[2] = input[6] * ialpha;
    }
    encode[3] = alpha;
#endif

    input += 7;
    encode += 4;
  } while ( encode < end_output );
}

//  format: [X A Xpm][X A Xpm] etc
static void stbir__fancy_alpha_unweight_2ch( float * encode_buffer, ptrdiff_t width_times_channels )
{
  float STBIR_SIMD_STREAMOUT_PTR(*) encode = encode_buffer;
  float STBIR_SIMD_STREAMOUT_PTR(*) input = encode_buffer;
  float const * end_output = encode_buffer + width_times_channels;

  do {
    float alpha = input[1];
    encode[0] = input[0];
    if ( alpha >= stbir__alpha_too_small_to_divide )
      encode[0] = input[2] / alpha;
    encode[1] = alpha;

    input += 3;
    encode += 2;
  } while ( encode < end_output );
}

static void stbir__simple_alpha_weight_4ch( float * decode_buffer, ptrdiff_t width_times_channels )
{
  float STBIR_STREAMOUT_PTR(*) decode = decode_buffer;
  float const * end_decode = decode_buffer + width_times_channels;

  #ifdef STBIR_SIMD
  {
    decode += 2 * stbir__simdfX_float_count;
    STBIR_NO_UNROLL_LOOP_START
    while ( decode <= end_decode )
    {
      stbir__simdfX d0,a0,d1,a1;
      STBIR_NO_UNROLL(decode);
      stbir__simdfX_load( d0, decode-2*stbir__simdfX_float_count );
      stbir__simdfX_load( d1, decode-2*stbir__simdfX_float_count+stbir__simdfX_float_count );
      stbir__simdfX_aaa1( a0, d0, STBIR_onesX );
      stbir__simdfX_aaa1( a1, d1, STBIR_onesX );
      stbir__simdfX_mult( d0, d0, a0 );
      stbir__simdfX_mult( d1, d1, a1 );
      stbir__simdfX_store ( decode-2*stbir__simdfX_float_count, d0 );
      stbir__simdfX_store ( decode-2*stbir__simdfX_float_count+stbir__simdfX_float_count, d1 );
      decode += 2 * stbir__simdfX_float_count;
    }
    decode -= 2 * stbir__simdfX_float_count;

    // few last pixels remnants
    #ifdef STBIR_SIMD8
    STBIR_NO_UNROLL_LOOP_START
    while ( decode < end_decode )
    #else
    if ( decode < end_decode )
    #endif
    {
      stbir__simdf d,a;
      stbir__simdf_load( d, decode );
      stbir__simdf_aaa1( a, d, STBIR__CONSTF(STBIR_ones) );
      stbir__simdf_mult( d, d, a );
      stbir__simdf_store ( decode, d );
      decode += 4;
    }
  }

  #else

  while( decode < end_decode )
  {
    float alpha = decode[3];
    decode[0] *= alpha;
    decode[1] *= alpha;
    decode[2] *= alpha;
    decode += 4;
  }

  #endif
}

static void stbir__simple_alpha_weight_2ch( float * decode_buffer, ptrdiff_t width_times_channels )
{
  float STBIR_STREAMOUT_PTR(*) decode = decode_buffer;
  float const * end_decode = decode_buffer + width_times_channels;

  #ifdef STBIR_SIMD
  decode += 2 * stbir__simdfX_float_count;
  STBIR_NO_UNROLL_LOOP_START
  while ( decode <= end_decode )
  {
    stbir__simdfX d0,a0,d1,a1;
    STBIR_NO_UNROLL(decode);
    stbir__simdfX_load( d0, decode-2*stbir__simdfX_float_count );
    stbir__simdfX_load( d1, decode-2*stbir__simdfX_float_count+stbir__simdfX_float_count );
    stbir__simdfX_a1a1( a0, d0, STBIR_onesX );
    stbir__simdfX_a1a1( a1, d1, STBIR_onesX );
    stbir__simdfX_mult( d0, d0, a0 );
    stbir__simdfX_mult( d1, d1, a1 );
    stbir__simdfX_store ( decode-2*stbir__simdfX_float_count, d0 );
    stbir__simdfX_store ( decode-2*stbir__simdfX_float_count+stbir__simdfX_float_count, d1 );
    decode += 2 * stbir__simdfX_float_count;
  }
  decode -= 2 * stbir__simdfX_float_count;
  #endif

  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( decode < end_decode )
  {
    float alpha = decode[1];
    STBIR_SIMD_NO_UNROLL(decode);
    decode[0] *= alpha;
    decode += 2;
  }
}

static void stbir__simple_alpha_unweight_4ch( float * encode_buffer, ptrdiff_t width_times_channels )
{
  float STBIR_SIMD_STREAMOUT_PTR(*) encode = encode_buffer;
  float const * end_output = encode_buffer + width_times_channels;

  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float alpha = encode[3];

#ifdef STBIR_SIMD
    stbir__simdf i,ia;
    STBIR_SIMD_NO_UNROLL(encode);
    if ( alpha >= stbir__alpha_too_small_to_divide )
    {
      stbir__simdf_load1frep4( ia, 1.0f / alpha );
      stbir__simdf_load( i, encode );
      stbir__simdf_mult( i, i, ia );
      stbir__simdf_store( encode, i );
      encode[3] = alpha;
    }
#else
    if ( alpha >= stbir__alpha_too_small_to_divide )
    {
      float ialpha = 1.0f / alpha;
      encode[0] *= ialpha;
      encode[1] *= ialpha;
      encode[2] *= ialpha;
    }
#endif
    encode += 4;
  } while ( encode < end_output );
}

static void stbir__simple_alpha_unweight_2ch( float * encode_buffer, ptrdiff_t width_times_channels )
{
  float STBIR_SIMD_STREAMOUT_PTR(*) encode = encode_buffer;
  float const * end_output = encode_buffer + width_times_channels;

  do {
    float alpha = encode[1];
    if ( alpha >= stbir__alpha_too_small_to_divide )
      encode[0] /= alpha;
    encode += 2;
  } while ( encode < end_output );
}


// only used in RGB->BGR or BGR->RGB
static void stbir__simple_flip_3ch( float * decode_buffer, ptrdiff_t width_times_channels )
{
  float STBIR_STREAMOUT_PTR(*) decode = decode_buffer;
  float const * end_decode = decode_buffer + width_times_channels;

#ifdef STBIR_SIMD
    #ifdef stbir__simdf_swiz2 // do we have two argument swizzles?
      end_decode -= 12; 
      STBIR_NO_UNROLL_LOOP_START
      while( decode <= end_decode )
      {
        // on arm64 8 instructions, no overlapping stores
        stbir__simdf a,b,c,na,nb;
        STBIR_SIMD_NO_UNROLL(decode);
        stbir__simdf_load( a, decode );
        stbir__simdf_load( b, decode+4 );
        stbir__simdf_load( c, decode+8 );

        na = stbir__simdf_swiz2( a, b, 2, 1, 0, 5 );   
        b  = stbir__simdf_swiz2( a, b, 4, 3, 6, 7 );   
        nb = stbir__simdf_swiz2( b, c, 0, 1, 4, 3 );   
        c  = stbir__simdf_swiz2( b, c, 2, 7, 6, 5 );   

        stbir__simdf_store( decode, na );
        stbir__simdf_store( decode+4, nb ); 
        stbir__simdf_store( decode+8, c );
        decode += 12;
      }
      end_decode += 12;
    #else
      end_decode -= 24;
      STBIR_NO_UNROLL_LOOP_START
      while( decode <= end_decode )
      {
        // 26 instructions on x64
        stbir__simdf a,b,c,d,e,f,g;
        float i21, i23;
        STBIR_SIMD_NO_UNROLL(decode);
        stbir__simdf_load( a, decode );
        stbir__simdf_load( b, decode+3 );
        stbir__simdf_load( c, decode+6 );
        stbir__simdf_load( d, decode+9 );
        stbir__simdf_load( e, decode+12 );
        stbir__simdf_load( f, decode+15 );
        stbir__simdf_load( g, decode+18 );

        a = stbir__simdf_swiz( a, 2, 1, 0, 3 );   
        b = stbir__simdf_swiz( b, 2, 1, 0, 3 );   
        c = stbir__simdf_swiz( c, 2, 1, 0, 3 );   
        d = stbir__simdf_swiz( d, 2, 1, 0, 3 );   
        e = stbir__simdf_swiz( e, 2, 1, 0, 3 );   
        f = stbir__simdf_swiz( f, 2, 1, 0, 3 );   
        g = stbir__simdf_swiz( g, 2, 1, 0, 3 );   

        // stores overlap, need to be in order, 
        stbir__simdf_store( decode,    a );
        i21 = decode[21];
        stbir__simdf_store( decode+3,  b ); 
        i23 = decode[23];
        stbir__simdf_store( decode+6,  c );
        stbir__simdf_store( decode+9,  d );
        stbir__simdf_store( decode+12, e );
        stbir__simdf_store( decode+15, f );
        stbir__simdf_store( decode+18, g );
        decode[21] = i23;
        decode[23] = i21;
        decode += 24;
      }
      end_decode += 24;
    #endif
#else
  end_decode -= 12;
  STBIR_NO_UNROLL_LOOP_START
  while( decode <= end_decode )
  {
    // 16 instructions
    float t0,t1,t2,t3;
    STBIR_NO_UNROLL(decode);
    t0 = decode[0];
    t1 = decode[3];
    t2 = decode[6];
    t3 = decode[9];
    decode[0] = decode[2];
    decode[3] = decode[5];
    decode[6] = decode[8];
    decode[9] = decode[11];
    decode[2] = t0;
    decode[5] = t1;
    decode[8] = t2;
    decode[11] = t3;
    decode += 12;
  }
  end_decode += 12;
#endif

  STBIR_NO_UNROLL_LOOP_START
  while( decode < end_decode )
  {
    float t = decode[0];
    STBIR_NO_UNROLL(decode);
    decode[0] = decode[2];
    decode[2] = t;
    decode += 3;
  }
}


// decodes the input pixels that a column slice needs for input scanline n into output_buffer
//   most of the complexity in this function is for wrap mode, we need to grab from multiple
//   points in the input buffer
static void stbir__decode_scanline(stbir__info const * stbir_info, int n, float * output_buffer, stbir__column_slice const * slice STBIR_ONLY_PROFILE_GET_SPLIT_INFO )
{
  int channels = stbir_info->channels;
  int effective_channels = stbir_info->effective_channels;
  int input_sample_in_bytes = stbir__type_size[stbir_info->input_type] * channels;
  stbir_edge edge_horizontal = stbir_info->horizontal.edge;
  stbir_edge edge_vertical = stbir_info->vertical.edge;
  int input_full_size = stbir_info->horizontal.scale_info.input_full_size;
  int row = stbir__edge_wrap(edge_vertical, n, stbir_info->vertical.scale_info.input_full_size);
  const void* input_plane_data = ( (char *) stbir_info->input_data ) + (ptrdiff_t)( row * stbir_info->input_stride_bytes );  
  float * full_decode_buffer = output_buffer - (ptrdiff_t)slice->in_n0 * effective_channels;  // decode buffer is indexed in input pixel space, so back it up to input pixel zero (the products are 64-bit: a tile of a huge input can start past 2^31 / channels)
  float * last_decoded = output_buffer;
  int run_n0[3], run_n1[3], run_input_x[3];
  int num_runs = 0, r;
  int n0 = slice->in_n0, n1 = slice->in_n1;

  // if we are on edge_zero, and we get in here with an out of bounds n, then the calculate filters has failed
  STBIR_ASSERT( !(edge_vertical == STBIR_EDGE_ZERO && (n < 0 || n >= stbir_info->vertical.scale_info.input_full_size)) );

  if ( edge_horizontal == STBIR_EDGE_WRAP )
  {
    // the margins are never wider than the scanline (stbir__set_sampler clamps them), so each side wraps exactly once
    STBIR_ASSERT( n0 >= -input_full_size );
    if ( n1 > ( ( input_full_size - 1 ) + input_full_size ) )
      n1 = ( input_full_size - 1 ) + input_full_size;   // the conservative range can poke one past what the filter can reach, that pixel just gets zeroed below ( 2 x 2^30 - 1, written so it can't overflow )

    if ( n0 < 0 )
    {
      run_n0[num_runs] = n0;
      run_n1[num_runs] = ( n1 < -1 ) ? n1 : -1;
      run_input_x[num_runs] = n0 + input_full_size;
      ++num_runs;
    }
    if ( ( n1 >= 0 ) && ( n0 < input_full_size ) )
    {
      run_n0[num_runs] = ( n0 > 0 ) ? n0 : 0;
      run_n1[num_runs] = ( n1 < ( input_full_size - 1 ) ) ? n1 : ( input_full_size - 1 );
      run_input_x[num_runs] = run_n0[num_runs];
      ++num_runs;
    }
    if ( n1 >= input_full_size )
    {
      run_n0[num_runs] = ( n0 > input_full_size ) ? n0 : input_full_size;
      run_n1[num_runs] = n1;
      run_input_x[num_runs] = run_n0[num_runs] - input_full_size;
      ++num_runs;
    }
  }
  else
  {
    STBIR_ASSERT( ( n0 >= 0 ) && ( n1 < input_full_size ) );
    run_n0[0] = n0;
    run_n1[0] = n1;
    run_input_x[0] = n0;
    num_runs = 1;
  }

  for( r = 0 ; r < num_runs ; r++ )
  {
    int width = run_n1[r] + 1 - run_n0[r];
    ptrdiff_t width_times_channels = (ptrdiff_t)width * channels;
    float * decode_buffer = full_decode_buffer + (ptrdiff_t)run_n0[r] * effective_channels;
    float * end_decode = full_decode_buffer + (ptrdiff_t)( run_n1[r] + 1 ) * effective_channels;
    void const * input_data;

    // read directly out of input plane by default
    input_data = ( (char*)input_plane_data ) + (ptrdiff_t)run_input_x[r] * input_sample_in_bytes;

    // if we have an input callback, call it to get the input data
    if ( stbir_info->in_pixels_cb )
    {
      // call the callback with a temp buffer (that they can choose to use or not).  the temp is just right aligned memory in the decode_buffer itself
      input_data = stbir_info->in_pixels_cb( ( (char*) end_decode ) - ( (ptrdiff_t)width * input_sample_in_bytes ) + stbir_info->input_callback_offset_bytes, input_plane_data, width, run_input_x[r], row, stbir_info->user_data );
    }

    STBIR_PROFILE_START( decode );
    // convert the pixels info the float decode_buffer, (we index from end_decode, so that when channels<effective_channels, we are right justified in the buffer)
    last_decoded = stbir_info->decode_pixels( (float*)end_decode - width_times_channels, width_times_channels, input_data );
    STBIR_PROFILE_END( decode );

    if (stbir_info->alpha_weight)
    {
      STBIR_PROFILE_START( alpha );
      stbir_info->alpha_weight( decode_buffer, width_times_channels );
      STBIR_PROFILE_END( alpha );
    }
  }

  // the horizontal kernels sometimes read a few samples past the input end, so
  //   zero the extra to avoid NaN (has to happen after decoding, since the input
  //   buffer might overwrite them.  Should just be a couple clears.
  {
    float * range_end = full_decode_buffer + (ptrdiff_t)( slice->in_n1 + 1 ) * effective_channels + 2;
    while ( last_decoded < range_end )
      *last_decoded++ = 0.0f;
  }
}

// advance to the next set of coeffs and contrib info
#define stbir__next_contributor()                                                                \
    decode_buffer += column->advance;                                                            \
    column = ( (stbir__column_entry const *)( ( (char const *)(column) ) + (entry_stride) ) );


//=================
// Do 1 channel horizontal routines

#ifdef STBIR_SIMD

#define stbir__1_coeff_only()          \
    stbir__simdf tot,c;                \
    STBIR_SIMD_NO_UNROLL(decode);      \
    stbir__simdf_load1( c, hc );       \
    stbir__simdf_mult1_mem( tot, c, decode );

#define stbir__2_coeff_only()          \
    stbir__simdf tot,c,d;              \
    STBIR_SIMD_NO_UNROLL(decode);      \
    stbir__simdf_load2z( c, hc );      \
    stbir__simdf_load2( d, decode );   \
    stbir__simdf_mult( tot, c, d );    \
    stbir__simdf_0123to1230( c, tot ); \
    stbir__simdf_add1( tot, tot, c );

#define stbir__3_coeff_only()                  \
    stbir__simdf tot,c,t;                      \
    STBIR_SIMD_NO_UNROLL(decode);              \
    stbir__simdf_load( c, hc );                \
    stbir__simdf_mult_mem( tot, c, decode );   \
    stbir__simdf_0123to1230( c, tot );         \
    stbir__simdf_0123to2301( t, tot );         \
    stbir__simdf_add1( tot, tot, c );          \
    stbir__simdf_add1( tot, tot, t );

#define stbir__store_output_tiny()                \
    stbir__simdf_store1( output, tot );           \
    stbir__next_contributor();                    \
    output += 1;

#define stbir__4_coeff_start()                 \
    stbir__simdf tot,c;                        \
    STBIR_SIMD_NO_UNROLL(decode);              \
    stbir__simdf_load( c, hc );                \
    stbir__simdf_mult_mem( tot, c, decode );   \

#define stbir__4_coeff_continue_from_4( ofs )  \
    STBIR_SIMD_NO_UNROLL(decode);              \
    stbir__simdf_load( c, hc + (ofs) );        \
    stbir__simdf_madd_mem( tot, tot, c, decode+(ofs) );

#define stbir__1_coeff_remnant( ofs )          \
    { stbir__simdf d;                          \
    stbir__simdf_load1z( c, hc + (ofs) );      \
    stbir__simdf_load1( d, decode + (ofs) );   \
    stbir__simdf_madd( tot, tot, d, c ); }

#define stbir__2_coeff_remnant( ofs )          \
    { stbir__simdf d;                          \
    stbir__simdf_load2z( c, hc+(ofs) );        \
    stbir__simdf_load2( d, decode+(ofs) );     \
    stbir__simdf_madd( tot, tot, d, c ); }

#define stbir__3_coeff_setup()                 \
    stbir__simdf mask;                         \
    stbir__simdf_load( mask, STBIR_mask + 3 );

#define stbir__3_coeff_remnant( ofs )                  \
    stbir__simdf_load( c, hc+(ofs) );                  \
    stbir__simdf_and( c, c, mask );                    \
    stbir__simdf_madd_mem( tot, tot, c, decode+(ofs) );

// four outputs per iteration for one channel - do 4 1-channel versions
#define stbir__x4_setup()                                                                                               \
    stbir__column_entry const * e1 = (stbir__column_entry const *)( (char const *)column + entry_stride );              \
    stbir__column_entry const * e2 = (stbir__column_entry const *)( (char const *)e1 + entry_stride );                  \
    stbir__column_entry const * e3 = (stbir__column_entry const *)( (char const *)e2 + entry_stride );                  \
    float const * d0 = decode_buffer, * d1 = d0 + column->advance, * d2 = d1 + e1->advance, * d3 = d2 + e2->advance;     \
    float const * h0 = column->coefficients, * h1 = e1->coefficients, * h2 = e2->coefficients, * h3 = e3->coefficients;   \
    stbir__simdf a0, a1, a2, a3, c;

#define stbir__x4_start()                        \
  do {                                           \
    stbir__simdf_load( c, h0 );                  \
    stbir__simdf_mult_mem( a0, c, d0 );          \
    stbir__simdf_load( c, h1 );                  \
    stbir__simdf_mult_mem( a1, c, d1 );          \
    stbir__simdf_load( c, h2 );                  \
    stbir__simdf_mult_mem( a2, c, d2 );          \
    stbir__simdf_load( c, h3 );                  \
    stbir__simdf_mult_mem( a3, c, d3 );          \
  } while(0)

#define stbir__x4_cont( ofs )                                \
  do {                                                       \
    stbir__simdf_load( c, h0 + (ofs) );                      \
    stbir__simdf_madd_mem( a0, a0, c, d0 + (ofs) );          \
    stbir__simdf_load( c, h1 + (ofs) );                      \
    stbir__simdf_madd_mem( a1, a1, c, d1 + (ofs) );          \
    stbir__simdf_load( c, h2 + (ofs) );                      \
    stbir__simdf_madd_mem( a2, a2, c, d2 + (ofs) );          \
    stbir__simdf_load( c, h3 + (ofs) );                      \
    stbir__simdf_madd_mem( a3, a3, c, d3 + (ofs) );          \
  } while(0)

#define stbir__x4_rem1( ofs )                                \
  do {                                                       \
    stbir__simdf d;                                          \
    stbir__simdf_load1z( c, h0 + (ofs) );                    \
    stbir__simdf_load1( d, d0 + (ofs) );                     \
    stbir__simdf_madd( a0, a0, d, c );                       \
    stbir__simdf_load1z( c, h1 + (ofs) );                    \
    stbir__simdf_load1( d, d1 + (ofs) );                     \
    stbir__simdf_madd( a1, a1, d, c );                       \
    stbir__simdf_load1z( c, h2 + (ofs) );                    \
    stbir__simdf_load1( d, d2 + (ofs) );                     \
    stbir__simdf_madd( a2, a2, d, c );                       \
    stbir__simdf_load1z( c, h3 + (ofs) );                    \
    stbir__simdf_load1( d, d3 + (ofs) );                     \
    stbir__simdf_madd( a3, a3, d, c );                       \
  } while(0)

#define stbir__x4_rem2( ofs )                                \
  do {                                                       \
    stbir__simdf d;                                          \
    stbir__simdf_load2z( c, h0 + (ofs) );                    \
    stbir__simdf_load2( d, d0 + (ofs) );                     \
    stbir__simdf_madd( a0, a0, d, c );                       \
    stbir__simdf_load2z( c, h1 + (ofs) );                    \
    stbir__simdf_load2( d, d1 + (ofs) );                     \
    stbir__simdf_madd( a1, a1, d, c );                       \
    stbir__simdf_load2z( c, h2 + (ofs) );                    \
    stbir__simdf_load2( d, d2 + (ofs) );                     \
    stbir__simdf_madd( a2, a2, d, c );                       \
    stbir__simdf_load2z( c, h3 + (ofs) );                    \
    stbir__simdf_load2( d, d3 + (ofs) );                     \
    stbir__simdf_madd( a3, a3, d, c );                       \
  } while(0)

// widths of 4n+3 carry a zero after their last coefficient (STBIR__COEFFICIENT_PAD), so three is a full four
#define stbir__x4_rem3( ofs ) stbir__x4_cont( ofs )

// two coefficients: ( d0 d0+1 | d1 d1+1 ) * ( c0 c0+1 | c1 c1+1 ), twice, then one pairwise add gives the four outputs
#define stbir__x4_2taps()                                    \
  do {                                                       \
    stbir__simdf_load2( c, h0 );                             \
    stbir__simdf_load2hmerge( c, c, h1 );                    \
    stbir__simdf_load2( a0, d0 );                            \
    stbir__simdf_load2hmerge( a0, a0, d1 );                  \
    stbir__simdf_mult( a0, a0, c );                          \
    stbir__simdf_load2( a2, h2 );                            \
    stbir__simdf_load2hmerge( a2, a2, h3 );                  \
    stbir__simdf_load2( a3, d2 );                            \
    stbir__simdf_load2hmerge( a3, a3, d3 );                  \
    stbir__simdf_mult( a1, a3, a2 );                         \
    stbir__simdf_hadd2( a0, a0, a1 );                        \
  } while(0)

// the same association as the per pixel path's reduction, ( l0 + l2 ) + ( l1 + l3 ), so the results match it bit for bit
#define stbir__x4_finish()                                   \
  do {                                                       \
    stbir__simdf_addhalves2( a0, a0, a1 );                   \
    stbir__simdf_addhalves2( a2, a2, a3 );                   \
    stbir__simdf_hadd2( a0, a0, a2 );                        \
  } while(0)

// the three coefficient per pixel path reduces as ( l0 + l1 ) + l2, which the pairwise tree reproduces (lane 3 is the zero pad)
#define stbir__x4_finish3()                                  \
  do {                                                       \
    stbir__simdf_hadd2( a0, a0, a1 );                        \
    stbir__simdf_hadd2( a2, a2, a3 );                        \
    stbir__simdf_hadd2( a0, a0, a2 );                        \
  } while(0)

#define stbir__x4_store()                                                                 \
  do {                                                                                    \
    stbir__simdf_store( output, a0 );                                                     \
    decode_buffer = d3 + e3->advance;                                                     \
    column = (stbir__column_entry const *)( (char const *)e3 + entry_stride );            \
    output += 4;                                                                          \
  } while(0)

// polyphase period 1 for 1 channels (when have exact downsamples 2:1, 3:1, etc - common for mipmaps)
#define stbir__period1_setup_4()  stbir__simdf P0; stbir__simdf_load( P0, hc );
#define stbir__period1_setup_5()  stbir__simdf P0, PR; stbir__simdf_load( P0, hc ); stbir__simdf_load1z( PR, hc + 4 );
#define stbir__period1_setup_8()  stbir__simdf P0, P1; stbir__simdf_load( P0, hc ); stbir__simdf_load( P1, hc + 4 );
#define stbir__period1_setup_9()  stbir__simdf P0, P1, PR; stbir__simdf_load( P0, hc ); stbir__simdf_load( P1, hc + 4 ); stbir__simdf_load1z( PR, hc + 8 );
#define stbir__period1_setup_11() stbir__simdf P0, P1, PR3; stbir__simdf_load( P0, hc ); stbir__simdf_load( P1, hc + 4 ); stbir__simdf_load( PR3, hc + 8 );   // the fourth of PR3 is the 4n+3 zero pad
#define stbir__period1_setup_12() stbir__simdf P0, P1, P2; stbir__simdf_load( P0, hc ); stbir__simdf_load( P1, hc + 4 ); stbir__simdf_load( P2, hc + 8 );
#define stbir__period1_setup_13() stbir__simdf PR; stbir__period1_setup_12() stbir__simdf_load1z( PR, hc + 12 );
#define stbir__period1_setup_16() stbir__simdf P3; stbir__period1_setup_12() stbir__simdf_load( P3, hc + 12 );
#define stbir__period1_setup_17() stbir__simdf PR; stbir__period1_setup_16() stbir__simdf_load1z( PR, hc + 16 );

#define stbir__period1_4_coeff_start()                 \
    stbir__simdf tot, c;                               \
    STBIR_SIMD_NO_UNROLL(decode);                      \
    stbir__simdf_mult_mem( tot, P0, decode );

#define stbir__period1_4_coeff_continue()              \
    stbir__simdf_madd_mem( tot, tot, P1, decode + 4 );
#define stbir__period1_4_coeff_continue2()             \
    stbir__simdf_madd_mem( tot, tot, P2, decode + 8 );
#define stbir__period1_4_coeff_continue3()             \
    stbir__simdf_madd_mem( tot, tot, P3, decode + 12 );

#define stbir__period1_1_coeff_remnant( ofs )          \
  do {                                                 \
    stbir__simdf d;                                    \
    stbir__simdf_load1( d, decode + (ofs) );           \
    stbir__simdf_madd( tot, tot, d, PR );              \
  } while(0)
#define stbir__period1_3_coeff_remnant( ofs )          \
    stbir__simdf_madd_mem( tot, tot, PR3, decode + (ofs) );

#define stbir__period1_store_output()                  \
    stbir__simdf_0123to2301( c, tot );                 \
    stbir__simdf_add( tot, tot, c );                   \
    stbir__simdf_0123to1230( c, tot );                 \
    stbir__simdf_add1( tot, tot, c );                  \
    stbir__simdf_store1( output, tot );                \
    decode_buffer += advance;                          \
    output += 1;

// the four outputs' decode pointers are one base plus multiples of the (constant) advance
#define stbir__period1_x4_setup()                                                                      \
    float const * d0 = decode_buffer;                                                                  \
    int const dofs1 = advance, dofs2 = advance * 2, dofs3 = advance * 3;                               \
    stbir__simdf a0, a1, a2, a3;

#define stbir__period1_x4_start()                            \
  do {                                                       \
    stbir__simdf_mult_mem( a0, P0, d0 );                     \
    stbir__simdf_mult_mem( a1, P0, d0 + dofs1 );             \
    stbir__simdf_mult_mem( a2, P0, d0 + dofs2 );             \
    stbir__simdf_mult_mem( a3, P0, d0 + dofs3 );             \
  } while(0)

// the next four coefficients, for each of the four outputs
#define stbir__period1_x4_next( P, ofs )                     \
  do {                                                       \
    stbir__simdf_madd_mem( a0, a0, P, d0 + (ofs) );          \
    stbir__simdf_madd_mem( a1, a1, P, d0 + dofs1 + (ofs) );  \
    stbir__simdf_madd_mem( a2, a2, P, d0 + dofs2 + (ofs) );  \
    stbir__simdf_madd_mem( a3, a3, P, d0 + dofs3 + (ofs) );  \
  } while(0)
#define stbir__period1_x4_continue()        stbir__period1_x4_next( P1, 4 )
#define stbir__period1_x4_continue2()       stbir__period1_x4_next( P2, 8 )
#define stbir__period1_x4_continue3()       stbir__period1_x4_next( P3, 12 )
#define stbir__period1_x4_remnant3( ofs )   stbir__period1_x4_next( PR3, ofs )

#define stbir__period1_x4_remnant( ofs )                     \
  do {                                                       \
    stbir__simdf d;                                          \
    stbir__simdf_load1( d, d0 + (ofs) );                     \
    stbir__simdf_madd( a0, a0, d, PR );                      \
    stbir__simdf_load1( d, d0 + dofs1 + (ofs) );             \
    stbir__simdf_madd( a1, a1, d, PR );                      \
    stbir__simdf_load1( d, d0 + dofs2 + (ofs) );             \
    stbir__simdf_madd( a2, a2, d, PR );                      \
    stbir__simdf_load1( d, d0 + dofs3 + (ofs) );             \
    stbir__simdf_madd( a3, a3, d, PR );                      \
  } while(0)

#define stbir__period1_x4_store()                            \
  do {                                                       \
    stbir__simdf_addhalves2( a0, a0, a1 );                   \
    stbir__simdf_addhalves2( a2, a2, a3 );                   \
    stbir__simdf_hadd2( a0, a0, a2 );                        \
    stbir__simdf_store( output, a0 );                        \
    decode_buffer = d0 + dofs3 + advance;                    \
    output += 4;                                             \
  } while(0)

#define stbir__store_output()                     \
    stbir__simdf_0123to2301( c, tot );            \
    stbir__simdf_add( tot, tot, c );              \
    stbir__simdf_0123to1230( c, tot );            \
    stbir__simdf_add1( tot, tot, c );             \
    stbir__simdf_store1( output, tot );           \
    stbir__next_contributor();                    \
    output += 1;

#else

#define stbir__1_coeff_only()  \
    float tot;                 \
    tot = decode[0]*hc[0];

#define stbir__2_coeff_only()  \
    float tot;                 \
    tot = decode[0] * hc[0];   \
    tot += decode[1] * hc[1];

#define stbir__3_coeff_only()  \
    float tot;                 \
    tot = decode[0] * hc[0];   \
    tot += decode[1] * hc[1];  \
    tot += decode[2] * hc[2];

#define stbir__store_output_tiny()                \
    output[0] = tot;                              \
    stbir__next_contributor();                    \
    output += 1;

#define stbir__4_coeff_start()  \
    float tot0,tot1,tot2,tot3;  \
    tot0 = decode[0] * hc[0];   \
    tot1 = decode[1] * hc[1];   \
    tot2 = decode[2] * hc[2];   \
    tot3 = decode[3] * hc[3];

#define stbir__4_coeff_continue_from_4( ofs )  \
    tot0 += decode[0+(ofs)] * hc[0+(ofs)];     \
    tot1 += decode[1+(ofs)] * hc[1+(ofs)];     \
    tot2 += decode[2+(ofs)] * hc[2+(ofs)];     \
    tot3 += decode[3+(ofs)] * hc[3+(ofs)];

#define stbir__1_coeff_remnant( ofs )        \
    tot0 += decode[0+(ofs)] * hc[0+(ofs)];

#define stbir__2_coeff_remnant( ofs )        \
    tot0 += decode[0+(ofs)] * hc[0+(ofs)];   \
    tot1 += decode[1+(ofs)] * hc[1+(ofs)];   \

#define stbir__3_coeff_remnant( ofs )        \
    tot0 += decode[0+(ofs)] * hc[0+(ofs)];   \
    tot1 += decode[1+(ofs)] * hc[1+(ofs)];   \
    tot2 += decode[2+(ofs)] * hc[2+(ofs)];

#define stbir__store_output()                     \
    output[0] = (tot0+tot2)+(tot1+tot3);          \
    stbir__next_contributor();                    \
    output += 1;

#endif

#define STBIR__horizontal_channels 1
#define STB_IMAGE_RESIZE_DO_HORIZONTALS
#include STBIR__HEADER_FILENAME


//=================
// Do 2 channel horizontal routines

#ifdef STBIR_SIMD

#define stbir__1_coeff_only()         \
    stbir__simdf tot,c,d;             \
    STBIR_SIMD_NO_UNROLL(decode);     \
    stbir__simdf_load1z( c, hc );     \
    stbir__simdf_0123to0011( c, c );  \
    stbir__simdf_load2( d, decode );  \
    stbir__simdf_mult( tot, d, c );

#define stbir__2_coeff_only()         \
    stbir__simdf tot,c;               \
    STBIR_SIMD_NO_UNROLL(decode);     \
    stbir__simdf_load2( c, hc );      \
    stbir__simdf_0123to0011( c, c );  \
    stbir__simdf_mult_mem( tot, c, decode );

#define stbir__3_coeff_only()                \
    stbir__simdf tot,c,cs,d;                 \
    STBIR_SIMD_NO_UNROLL(decode);            \
    stbir__simdf_load( cs, hc );             \
    stbir__simdf_0123to0011( c, cs );        \
    stbir__simdf_mult_mem( tot, c, decode ); \
    stbir__simdf_0123to2222( c, cs );        \
    stbir__simdf_load2z( d, decode+4 );      \
    stbir__simdf_madd( tot, tot, d, c );

#define stbir__store_output_tiny()                \
    stbir__simdf_0123to2301( c, tot );            \
    stbir__simdf_add( tot, tot, c );              \
    stbir__simdf_store2( output, tot );           \
    stbir__next_contributor();                    \
    output += 2;

// polyphase period 1 for 2 channels (when have exact downsamples 2:1, 3:1, etc - common for mipmaps)
#define stbir__period1_setup_4()  stbir__simdf P0a, P0b, cs; stbir__simdf_load( cs, hc ); stbir__simdf_0123to0011( P0a, cs ); stbir__simdf_0123to2233( P0b, cs );
#define stbir__period1_setup_5()  stbir__simdf PR; stbir__period1_setup_4() stbir__simdf_load1z( cs, hc + 4 ); stbir__simdf_0123to0011( PR, cs );
#define stbir__period1_setup_8()  stbir__simdf P1a, P1b; stbir__period1_setup_4() stbir__simdf_load( cs, hc + 4 ); stbir__simdf_0123to0011( P1a, cs ); stbir__simdf_0123to2233( P1b, cs );
#define stbir__period1_setup_9()  stbir__simdf PR; stbir__period1_setup_8() stbir__simdf_load1z( cs, hc + 8 ); stbir__simdf_0123to0011( PR, cs );
#define stbir__period1_setup_11() stbir__simdf PR3a, PR3b; stbir__period1_setup_8() stbir__simdf_load( cs, hc + 8 ); stbir__simdf_0123to0011( PR3a, cs ); stbir__simdf_0123to2222( PR3b, cs );
#define stbir__period1_setup_12() stbir__simdf P2a, P2b; stbir__period1_setup_8() stbir__simdf_load( cs, hc + 8 ); stbir__simdf_0123to0011( P2a, cs ); stbir__simdf_0123to2233( P2b, cs );
#define stbir__period1_setup_13() stbir__simdf PR; stbir__period1_setup_12() stbir__simdf_load1z( cs, hc + 12 ); stbir__simdf_0123to0011( PR, cs );
#define stbir__period1_setup_16() stbir__simdf P3a, P3b; stbir__period1_setup_12() stbir__simdf_load( cs, hc + 12 ); stbir__simdf_0123to0011( P3a, cs ); stbir__simdf_0123to2233( P3b, cs );
#define stbir__period1_setup_17() stbir__simdf PR; stbir__period1_setup_16() stbir__simdf_load1z( cs, hc + 16 ); stbir__simdf_0123to0011( PR, cs );

#define stbir__period1_4_coeff_start()                 \
    stbir__simdf tot0, tot1;                           \
    STBIR_SIMD_NO_UNROLL(decode);                      \
    stbir__simdf_mult_mem( tot0, P0a, decode );        \
    stbir__simdf_mult_mem( tot1, P0b, decode + 4 );

#define stbir__period1_4_coeff_continue()                    \
    stbir__simdf_madd_mem( tot0, tot0, P1a, decode + 8 );    \
    stbir__simdf_madd_mem( tot1, tot1, P1b, decode + 12 );
#define stbir__period1_4_coeff_continue2()                   \
    stbir__simdf_madd_mem( tot0, tot0, P2a, decode + 16 );   \
    stbir__simdf_madd_mem( tot1, tot1, P2b, decode + 20 );
#define stbir__period1_4_coeff_continue3()                   \
    stbir__simdf_madd_mem( tot0, tot0, P3a, decode + 24 );   \
    stbir__simdf_madd_mem( tot1, tot1, P3b, decode + 28 );

#define stbir__period1_1_coeff_remnant( ofs )          \
    { stbir__simdf d;                                  \
    stbir__simdf_load2( d, decode + (ofs) * 2 );       \
    stbir__simdf_madd( tot0, tot0, d, PR ); }
#define stbir__period1_3_coeff_remnant( ofs )                      \
    { stbir__simdf d;                                              \
    stbir__simdf_madd_mem( tot0, tot0, PR3a, decode + (ofs) * 2 ); \
    stbir__simdf_load2z( d, decode + (ofs) * 2 + 4 );              \
    stbir__simdf_madd( tot1, tot1, d, PR3b ); }

#define stbir__period1_store_output()                  \
    stbir__simdf_add( tot0, tot0, tot1 );              \
    stbir__simdf_0123to2301( tot1, tot0 );             \
    stbir__simdf_add( tot0, tot0, tot1 );              \
    stbir__simdf_store2( output, tot0 );               \
    decode_buffer += advance;                          \
    output += 2;

#ifdef STBIR_SIMD8

#define stbir__4_coeff_start()                    \
    stbir__simdf8 tot0,c,cs;                      \
    STBIR_SIMD_NO_UNROLL(decode);                 \
    stbir__simdf8_load4b( cs, hc );               \
    stbir__simdf8_0123to00112233( c, cs );        \
    stbir__simdf8_mult_mem( tot0, c, decode );

#define stbir__4_coeff_continue_from_4( ofs )        \
    STBIR_SIMD_NO_UNROLL(decode);                    \
    stbir__simdf8_load4b( cs, hc + (ofs) );          \
    stbir__simdf8_0123to00112233( c, cs );           \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*2 );

#define stbir__1_coeff_remnant( ofs )                \
    { stbir__simdf t,d;                              \
    stbir__simdf_load1z( t, hc + (ofs) );            \
    stbir__simdf_load2( d, decode + (ofs) * 2 );     \
    stbir__simdf_0123to0011( t, t );                 \
    stbir__simdf_mult( t, t, d );                    \
    stbir__simdf8_add4( tot0, tot0, t ); }
 
#define stbir__2_coeff_remnant( ofs )                \
    { stbir__simdf t;                                \
    stbir__simdf_load2( t, hc + (ofs) );             \
    stbir__simdf_0123to0011( t, t );                 \
    stbir__simdf_mult_mem( t, t, decode+(ofs)*2 );   \
    stbir__simdf8_add4( tot0, tot0, t ); }

#define stbir__3_coeff_remnant( ofs )                \
    { stbir__simdf8 d;                               \
    stbir__simdf8_load4b( cs, hc + (ofs) );          \
    stbir__simdf8_0123to00112233( c, cs );           \
    stbir__simdf8_load6z( d, decode+(ofs)*2 );       \
    stbir__simdf8_madd( tot0, tot0, c, d ); }

#define stbir__store_output()                     \
    { stbir__simdf t,d;                           \
    stbir__simdf8_add4halves( t, stbir__if_simdf8_cast_to_simdf4(tot0), tot0 );    \
    stbir__simdf_0123to2301( d, t );              \
    stbir__simdf_add( t, t, d );                  \
    stbir__simdf_store2( output, t );             \
    stbir__next_contributor();                    \
    output += 2; }

#else

#define stbir__4_coeff_start()                   \
    stbir__simdf tot0,tot1,c,cs;                 \
    STBIR_SIMD_NO_UNROLL(decode);                \
    stbir__simdf_load( cs, hc );                 \
    stbir__simdf_0123to0011( c, cs );            \
    stbir__simdf_mult_mem( tot0, c, decode );    \
    stbir__simdf_0123to2233( c, cs );            \
    stbir__simdf_mult_mem( tot1, c, decode+4 );

#define stbir__4_coeff_continue_from_4( ofs )                \
    STBIR_SIMD_NO_UNROLL(decode);                            \
    stbir__simdf_load( cs, hc + (ofs) );                     \
    stbir__simdf_0123to0011( c, cs );                        \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*2 );  \
    stbir__simdf_0123to2233( c, cs );                        \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*2+4 );

#define stbir__1_coeff_remnant( ofs )            \
    { stbir__simdf d;                            \
    stbir__simdf_load1z( cs, hc + (ofs) );       \
    stbir__simdf_0123to0011( c, cs );            \
    stbir__simdf_load2( d, decode + (ofs) * 2 ); \
    stbir__simdf_madd( tot0, tot0, d, c ); }

#define stbir__2_coeff_remnant( ofs )                      \
    stbir__simdf_load2( cs, hc + (ofs) );                  \
    stbir__simdf_0123to0011( c, cs );                      \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*2 );

#define stbir__3_coeff_remnant( ofs )                       \
    { stbir__simdf d;                                       \
    stbir__simdf_load( cs, hc + (ofs) );                    \
    stbir__simdf_0123to0011( c, cs );                       \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*2 ); \
    stbir__simdf_0123to2222( c, cs );                       \
    stbir__simdf_load2z( d, decode + (ofs) * 2 + 4 );       \
    stbir__simdf_madd( tot1, tot1, d, c ); }

#define stbir__store_output()                     \
    stbir__simdf_add( tot0, tot0, tot1 );         \
    stbir__simdf_0123to2301( c, tot0 );           \
    stbir__simdf_add( tot0, tot0, c );            \
    stbir__simdf_store2( output, tot0 );          \
    stbir__next_contributor();                    \
    output += 2;

#endif

#else

#define stbir__1_coeff_only()  \
    float tota,totb,c;         \
    c = hc[0];                 \
    tota = decode[0]*c;        \
    totb = decode[1]*c;

#define stbir__2_coeff_only()  \
    float tota,totb,c;         \
    c = hc[0];                 \
    tota = decode[0]*c;        \
    totb = decode[1]*c;        \
    c = hc[1];                 \
    tota += decode[2]*c;       \
    totb += decode[3]*c;

// this weird order of add matches the simd
#define stbir__3_coeff_only()  \
    float tota,totb,c;         \
    c = hc[0];                 \
    tota = decode[0]*c;        \
    totb = decode[1]*c;        \
    c = hc[2];                 \
    tota += decode[4]*c;       \
    totb += decode[5]*c;       \
    c = hc[1];                 \
    tota += decode[2]*c;       \
    totb += decode[3]*c;

#define stbir__store_output_tiny()                \
    output[0] = tota;                             \
    output[1] = totb;                             \
    stbir__next_contributor();                    \
    output += 2;

#define stbir__4_coeff_start()      \
    float tota0,tota1,tota2,tota3,totb0,totb1,totb2,totb3,c;  \
    c = hc[0];                      \
    tota0 = decode[0]*c;            \
    totb0 = decode[1]*c;            \
    c = hc[1];                      \
    tota1 = decode[2]*c;            \
    totb1 = decode[3]*c;            \
    c = hc[2];                      \
    tota2 = decode[4]*c;            \
    totb2 = decode[5]*c;            \
    c = hc[3];                      \
    tota3 = decode[6]*c;            \
    totb3 = decode[7]*c;

#define stbir__4_coeff_continue_from_4( ofs )  \
    c = hc[0+(ofs)];                           \
    tota0 += decode[0+(ofs)*2]*c;              \
    totb0 += decode[1+(ofs)*2]*c;              \
    c = hc[1+(ofs)];                           \
    tota1 += decode[2+(ofs)*2]*c;              \
    totb1 += decode[3+(ofs)*2]*c;              \
    c = hc[2+(ofs)];                           \
    tota2 += decode[4+(ofs)*2]*c;              \
    totb2 += decode[5+(ofs)*2]*c;              \
    c = hc[3+(ofs)];                           \
    tota3 += decode[6+(ofs)*2]*c;              \
    totb3 += decode[7+(ofs)*2]*c;

#define stbir__1_coeff_remnant( ofs )  \
    c = hc[0+(ofs)];                   \
    tota0 += decode[0+(ofs)*2] * c;    \
    totb0 += decode[1+(ofs)*2] * c;

#define stbir__2_coeff_remnant( ofs )  \
    c = hc[0+(ofs)];                   \
    tota0 += decode[0+(ofs)*2] * c;    \
    totb0 += decode[1+(ofs)*2] * c;    \
    c = hc[1+(ofs)];                   \
    tota1 += decode[2+(ofs)*2] * c;    \
    totb1 += decode[3+(ofs)*2] * c;

#define stbir__3_coeff_remnant( ofs )  \
    c = hc[0+(ofs)];                   \
    tota0 += decode[0+(ofs)*2] * c;    \
    totb0 += decode[1+(ofs)*2] * c;    \
    c = hc[1+(ofs)];                   \
    tota1 += decode[2+(ofs)*2] * c;    \
    totb1 += decode[3+(ofs)*2] * c;    \
    c = hc[2+(ofs)];                   \
    tota2 += decode[4+(ofs)*2] * c;    \
    totb2 += decode[5+(ofs)*2] * c;

#define stbir__store_output()                     \
    output[0] = (tota0+tota2)+(tota1+tota3);      \
    output[1] = (totb0+totb2)+(totb1+totb3);      \
    stbir__next_contributor();                    \
    output += 2;

#endif

#define STBIR__horizontal_channels 2
#define STB_IMAGE_RESIZE_DO_HORIZONTALS
#include STBIR__HEADER_FILENAME


//=================
// Do 3 channel horizontal routines

#ifdef STBIR_SIMD

#define stbir__1_coeff_only()         \
    stbir__simdf tot,c,d;             \
    STBIR_SIMD_NO_UNROLL(decode);     \
    stbir__simdf_load1z( c, hc );     \
    stbir__simdf_0123to0001( c, c );  \
    stbir__simdf_load( d, decode );   \
    stbir__simdf_mult( tot, d, c );

#define stbir__2_coeff_only()         \
    stbir__simdf tot,c,cs,d;          \
    STBIR_SIMD_NO_UNROLL(decode);     \
    stbir__simdf_load2( cs, hc );     \
    stbir__simdf_0123to0000( c, cs ); \
    stbir__simdf_load( d, decode );   \
    stbir__simdf_mult( tot, d, c );   \
    stbir__simdf_0123to1111( c, cs ); \
    stbir__simdf_load( d, decode+3 ); \
    stbir__simdf_madd( tot, tot, d, c );

#define stbir__3_coeff_only()            \
    stbir__simdf tot,c,d,cs;             \
    STBIR_SIMD_NO_UNROLL(decode);        \
    stbir__simdf_load( cs, hc );         \
    stbir__simdf_0123to0000( c, cs );    \
    stbir__simdf_load( d, decode );      \
    stbir__simdf_mult( tot, d, c );      \
    stbir__simdf_0123to1111( c, cs );    \
    stbir__simdf_load( d, decode+3 );    \
    stbir__simdf_madd( tot, tot, d, c ); \
    stbir__simdf_0123to2222( c, cs );    \
    stbir__simdf_load( d, decode+6 );    \
    stbir__simdf_madd( tot, tot, d, c );

#define stbir__store_output_tiny()                \
    stbir__simdf_store2( output, tot );           \
    stbir__simdf_0123to2301( tot, tot );          \
    stbir__simdf_store1( output+2, tot );         \
    stbir__next_contributor();                    \
    output += 3;

#ifdef STBIR_SIMD8

// we're loading from the XXXYYY decode by -1 to get the XXXYYY into different halves of the AVX reg fyi
#define stbir__4_coeff_start()                     \
    stbir__simdf8 tot0,tot1,c,cs; stbir__simdf t;  \
    STBIR_SIMD_NO_UNROLL(decode);                  \
    stbir__simdf8_load4b( cs, hc );                \
    stbir__simdf8_0123to00001111( c, cs );         \
    stbir__simdf8_mult_mem( tot0, c, decode - 1 ); \
    stbir__simdf8_0123to22223333( c, cs );         \
    stbir__simdf8_mult_mem( tot1, c, decode+6 - 1 );

#define stbir__4_coeff_continue_from_4( ofs )      \
    STBIR_SIMD_NO_UNROLL(decode);                  \
    stbir__simdf8_load4b( cs, hc + (ofs) );        \
    stbir__simdf8_0123to00001111( c, cs );         \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*3 - 1 ); \
    stbir__simdf8_0123to22223333( c, cs );         \
    stbir__simdf8_madd_mem( tot1, tot1, c, decode+(ofs)*3 + 6 - 1 );

#define stbir__1_coeff_remnant( ofs )                          \
    STBIR_SIMD_NO_UNROLL(decode);                              \
    stbir__simdf_load1rep4( t, hc + (ofs) );                   \
    stbir__simdf8_madd_mem4( tot0, tot0, t, decode+(ofs)*3 - 1 );

#define stbir__2_coeff_remnant( ofs )                          \
    STBIR_SIMD_NO_UNROLL(decode);                              \
    stbir__simdf8_load4b( cs, hc + (ofs) - 2 );                \
    stbir__simdf8_0123to22223333( c, cs );                     \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*3 - 1 );

 #define stbir__3_coeff_remnant( ofs )                           \
    STBIR_SIMD_NO_UNROLL(decode);                                \
    stbir__simdf8_load4b( cs, hc + (ofs) );                      \
    stbir__simdf8_0123to00001111( c, cs );                       \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*3 - 1 ); \
    stbir__simdf8_0123to2222( t, cs );                           \
    stbir__simdf8_madd_mem4( tot1, tot1, t, decode+(ofs)*3 + 6 - 1 );

// polyphase period 1 for 3 channels (when have exact downsamples 2:1, 3:1, etc - common for mipmaps)
#define stbir__period1_setup_4()  stbir__simdf8 P01, P23, cs; stbir__simdf8_load4b( cs, hc ); stbir__simdf8_0123to00001111( P01, cs ); stbir__simdf8_0123to22223333( P23, cs );
#define stbir__period1_setup_5()  stbir__simdf PR; stbir__period1_setup_4() stbir__simdf_load1rep4( PR, hc + 4 );
#define stbir__period1_setup_8()  stbir__simdf8 P45, P67; stbir__period1_setup_4() stbir__simdf8_load4b( cs, hc + 4 ); stbir__simdf8_0123to00001111( P45, cs ); stbir__simdf8_0123to22223333( P67, cs );
#define stbir__period1_setup_9()  stbir__simdf PR; stbir__period1_setup_8() stbir__simdf_load1rep4( PR, hc + 8 );
#define stbir__period1_setup_11() stbir__simdf8 P89; stbir__simdf PR; stbir__period1_setup_8() stbir__simdf8_load4b( cs, hc + 8 ); stbir__simdf8_0123to00001111( P89, cs ); stbir__simdf8_0123to2222( PR, cs );
#define stbir__period1_setup_12() stbir__simdf8 P89, PAB; stbir__period1_setup_8() stbir__simdf8_load4b( cs, hc + 8 ); stbir__simdf8_0123to00001111( P89, cs ); stbir__simdf8_0123to22223333( PAB, cs );
#define stbir__period1_setup_13() stbir__simdf PR; stbir__period1_setup_12() stbir__simdf_load1rep4( PR, hc + 12 );
#define stbir__period1_setup_16() stbir__simdf8 PCD, PEF; stbir__period1_setup_12() stbir__simdf8_load4b( cs, hc + 12 ); stbir__simdf8_0123to00001111( PCD, cs ); stbir__simdf8_0123to22223333( PEF, cs );
#define stbir__period1_setup_17() stbir__simdf PR; stbir__period1_setup_16() stbir__simdf_load1rep4( PR, hc + 16 );

#define stbir__period1_4_coeff_start()                       \
    stbir__simdf8 tot0, tot1; stbir__simdf t;                \
    STBIR_SIMD_NO_UNROLL(decode);                            \
    stbir__simdf8_mult_mem( tot0, P01, decode - 1 );         \
    stbir__simdf8_mult_mem( tot1, P23, decode + 6 - 1 );

#define stbir__period1_4_coeff_continue()                          \
    stbir__simdf8_madd_mem( tot0, tot0, P45, decode + 12 - 1 );    \
    stbir__simdf8_madd_mem( tot1, tot1, P67, decode + 18 - 1 );
#define stbir__period1_4_coeff_continue2()                         \
    stbir__simdf8_madd_mem( tot0, tot0, P89, decode + 24 - 1 );    \
    stbir__simdf8_madd_mem( tot1, tot1, PAB, decode + 30 - 1 );
#define stbir__period1_4_coeff_continue3()                         \
    stbir__simdf8_madd_mem( tot0, tot0, PCD, decode + 36 - 1 );    \
    stbir__simdf8_madd_mem( tot1, tot1, PEF, decode + 42 - 1 );

#define stbir__period1_1_coeff_remnant( ofs )                      \
    stbir__simdf8_madd_mem4( tot0, tot0, PR, decode + (ofs) * 3 - 1 );
#define stbir__period1_3_coeff_remnant( ofs )                          \
    stbir__simdf8_madd_mem( tot0, tot0, P89, decode + (ofs) * 3 - 1 );  \
    stbir__simdf8_madd_mem4( tot1, tot1, PR, decode + (ofs) * 3 + 6 - 1 );

#define stbir__period1_store_output()                                       \
    stbir__simdf8_add( tot0, tot0, tot1 );                                  \
    stbir__simdf_0123to1230( t, stbir__if_simdf8_cast_to_simdf4( tot0 ) ); \
    stbir__simdf8_add4halves( t, t, tot0 );                                 \
    decode_buffer += advance;                                               \
    output += 3;                                                            \
    if ( output < output_end )                                              \
    {                                                                       \
      stbir__simdf_store( output-3, t );                                    \
      continue;                                                             \
    }                                                                       \
    { stbir__simdf tt; stbir__simdf_0123to2301( tt, t );                    \
    stbir__simdf_store2( output-3, t );                                     \
    stbir__simdf_store1( output+2-3, tt ); }                                \
    break;

#define stbir__store_output()                       \
    stbir__simdf8_add( tot0, tot0, tot1 );          \
    stbir__simdf_0123to1230( t, stbir__if_simdf8_cast_to_simdf4( tot0 ) ); \
    stbir__simdf8_add4halves( t, t, tot0 );         \
    stbir__next_contributor();                    \
    output += 3;                                    \
    if ( output < output_end )                      \
    {                                               \
      stbir__simdf_store( output-3, t );            \
      continue;                                     \
    }                                               \
    { stbir__simdf tt; stbir__simdf_0123to2301( tt, t ); \
    stbir__simdf_store2( output-3, t );             \
    stbir__simdf_store1( output+2-3, tt ); }        \
    break;


#else

#define stbir__4_coeff_start()                  \
    stbir__simdf tot0,tot1,tot2,c,cs;           \
    STBIR_SIMD_NO_UNROLL(decode);               \
    stbir__simdf_load( cs, hc );                \
    stbir__simdf_0123to0001( c, cs );           \
    stbir__simdf_mult_mem( tot0, c, decode );   \
    stbir__simdf_0123to1122( c, cs );           \
    stbir__simdf_mult_mem( tot1, c, decode+4 ); \
    stbir__simdf_0123to2333( c, cs );           \
    stbir__simdf_mult_mem( tot2, c, decode+8 );

#define stbir__4_coeff_continue_from_4( ofs )                 \
    STBIR_SIMD_NO_UNROLL(decode);                             \
    stbir__simdf_load( cs, hc + (ofs) );                      \
    stbir__simdf_0123to0001( c, cs );                         \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*3 );   \
    stbir__simdf_0123to1122( c, cs );                         \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*3+4 ); \
    stbir__simdf_0123to2333( c, cs );                         \
    stbir__simdf_madd_mem( tot2, tot2, c, decode+(ofs)*3+8 );

#define stbir__1_coeff_remnant( ofs )         \
    STBIR_SIMD_NO_UNROLL(decode);             \
    stbir__simdf_load1z( c, hc + (ofs) );     \
    stbir__simdf_0123to0001( c, c );          \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*3 );

#define stbir__2_coeff_remnant( ofs )                       \
    { stbir__simdf d;                                       \
    STBIR_SIMD_NO_UNROLL(decode);                           \
    stbir__simdf_load2z( cs, hc + (ofs) );                  \
    stbir__simdf_0123to0001( c, cs );                       \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*3 ); \
    stbir__simdf_0123to1122( c, cs );                       \
    stbir__simdf_load2z( d, decode+(ofs)*3+4 );             \
    stbir__simdf_madd( tot1, tot1, c, d ); }

#define stbir__3_coeff_remnant( ofs )                         \
    { stbir__simdf d;                                         \
    STBIR_SIMD_NO_UNROLL(decode);                             \
    stbir__simdf_load( cs, hc + (ofs) );                      \
    stbir__simdf_0123to0001( c, cs );                         \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*3 );   \
    stbir__simdf_0123to1122( c, cs );                         \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*3+4 ); \
    stbir__simdf_0123to2222( c, cs );                         \
    stbir__simdf_load1z( d, decode+(ofs)*3+8 );               \
    stbir__simdf_madd( tot2, tot2, c, d );  }

// polyphase period 1 for 3 channels (when have exact downsamples 2:1, 3:1, etc - common for mipmaps)
#define stbir__period1_setup_4()  stbir__simdf P0a, P0b, P0c, cs; stbir__simdf_load( cs, hc ); stbir__simdf_0123to0001( P0a, cs ); stbir__simdf_0123to1122( P0b, cs ); stbir__simdf_0123to2333( P0c, cs );
#define stbir__period1_setup_5()  stbir__simdf PR; stbir__period1_setup_4() stbir__simdf_load1z( cs, hc + 4 ); stbir__simdf_0123to0001( PR, cs );
#define stbir__period1_setup_8()  stbir__simdf P1a, P1b, P1c; stbir__period1_setup_4() stbir__simdf_load( cs, hc + 4 ); stbir__simdf_0123to0001( P1a, cs ); stbir__simdf_0123to1122( P1b, cs ); stbir__simdf_0123to2333( P1c, cs );
#define stbir__period1_setup_9()  stbir__simdf PR; stbir__period1_setup_8() stbir__simdf_load1z( cs, hc + 8 ); stbir__simdf_0123to0001( PR, cs );
#define stbir__period1_setup_11() stbir__simdf PR3a, PR3b, PR3c; stbir__period1_setup_8() stbir__simdf_load( cs, hc + 8 ); stbir__simdf_0123to0001( PR3a, cs ); stbir__simdf_0123to1122( PR3b, cs ); stbir__simdf_0123to2222( PR3c, cs );
#define stbir__period1_setup_12() stbir__simdf P2a, P2b, P2c; stbir__period1_setup_8() stbir__simdf_load( cs, hc + 8 ); stbir__simdf_0123to0001( P2a, cs ); stbir__simdf_0123to1122( P2b, cs ); stbir__simdf_0123to2333( P2c, cs );
#define stbir__period1_setup_13() stbir__simdf PR; stbir__period1_setup_12() stbir__simdf_load1z( cs, hc + 12 ); stbir__simdf_0123to0001( PR, cs );
#define stbir__period1_setup_16() stbir__simdf P3a, P3b, P3c; stbir__period1_setup_12() stbir__simdf_load( cs, hc + 12 ); stbir__simdf_0123to0001( P3a, cs ); stbir__simdf_0123to1122( P3b, cs ); stbir__simdf_0123to2333( P3c, cs );
#define stbir__period1_setup_17() stbir__simdf PR; stbir__period1_setup_16() stbir__simdf_load1z( cs, hc + 16 ); stbir__simdf_0123to0001( PR, cs );

#define stbir__period1_4_coeff_start()                 \
    stbir__simdf tot0, tot1, tot2, c;                  \
    STBIR_SIMD_NO_UNROLL(decode);                      \
    stbir__simdf_mult_mem( tot0, P0a, decode );        \
    stbir__simdf_mult_mem( tot1, P0b, decode + 4 );    \
    stbir__simdf_mult_mem( tot2, P0c, decode + 8 );

#define stbir__period1_4_coeff_continue()                    \
    stbir__simdf_madd_mem( tot0, tot0, P1a, decode + 12 );   \
    stbir__simdf_madd_mem( tot1, tot1, P1b, decode + 16 );   \
    stbir__simdf_madd_mem( tot2, tot2, P1c, decode + 20 );
#define stbir__period1_4_coeff_continue2()                   \
    stbir__simdf_madd_mem( tot0, tot0, P2a, decode + 24 );   \
    stbir__simdf_madd_mem( tot1, tot1, P2b, decode + 28 );   \
    stbir__simdf_madd_mem( tot2, tot2, P2c, decode + 32 );
#define stbir__period1_4_coeff_continue3()                   \
    stbir__simdf_madd_mem( tot0, tot0, P3a, decode + 36 );   \
    stbir__simdf_madd_mem( tot1, tot1, P3b, decode + 40 );   \
    stbir__simdf_madd_mem( tot2, tot2, P3c, decode + 44 );

#define stbir__period1_1_coeff_remnant( ofs )                \
    stbir__simdf_madd_mem( tot0, tot0, PR, decode + (ofs) * 3 );
#define stbir__period1_3_coeff_remnant( ofs )                          \
    { stbir__simdf d;                                                  \
    stbir__simdf_madd_mem( tot0, tot0, PR3a, decode + (ofs) * 3 );     \
    stbir__simdf_madd_mem( tot1, tot1, PR3b, decode + (ofs) * 3 + 4 ); \
    stbir__simdf_load1z( d, decode + (ofs) * 3 + 8 );                  \
    stbir__simdf_madd( tot2, tot2, PR3c, d ); }

#define stbir__period1_store_output()                  \
    stbir__simdf_0123ABCDto3ABx( c, tot0, tot1 );      \
    stbir__simdf_0123ABCDto23Ax( cs, tot1, tot2 );     \
    stbir__simdf_0123to1230( tot2, tot2 );             \
    stbir__simdf_add( tot0, tot0, cs );                \
    stbir__simdf_add( c, c, tot2 );                    \
    stbir__simdf_add( tot0, tot0, c );                 \
    decode_buffer += advance;                          \
    output += 3;                                       \
    if ( output < output_end )                         \
    {                                                  \
      stbir__simdf_store( output-3, tot0 );            \
      continue;                                        \
    }                                                  \
    stbir__simdf_0123to2301( tot1, tot0 );             \
    stbir__simdf_store2( output-3, tot0 );             \
    stbir__simdf_store1( output+2-3, tot1 );           \
    break;

#define stbir__store_output()                       \
    stbir__simdf_0123ABCDto3ABx( c, tot0, tot1 );   \
    stbir__simdf_0123ABCDto23Ax( cs, tot1, tot2 );  \
    stbir__simdf_0123to1230( tot2, tot2 );          \
    stbir__simdf_add( tot0, tot0, cs );             \
    stbir__simdf_add( c, c, tot2 );                 \
    stbir__simdf_add( tot0, tot0, c );              \
    stbir__next_contributor();                    \
    output += 3;                                    \
    if ( output < output_end )                      \
    {                                               \
      stbir__simdf_store( output-3, tot0 );         \
      continue;                                     \
    }                                               \
    stbir__simdf_0123to2301( tot1, tot0 );          \
    stbir__simdf_store2( output-3, tot0 );          \
    stbir__simdf_store1( output+2-3, tot1 );        \
    break;

#endif

#else

#define stbir__1_coeff_only()  \
    float tot0, tot1, tot2, c; \
    c = hc[0];                 \
    tot0 = decode[0]*c;        \
    tot1 = decode[1]*c;        \
    tot2 = decode[2]*c;

#define stbir__2_coeff_only()  \
    float tot0, tot1, tot2, c; \
    c = hc[0];                 \
    tot0 = decode[0]*c;        \
    tot1 = decode[1]*c;        \
    tot2 = decode[2]*c;        \
    c = hc[1];                 \
    tot0 += decode[3]*c;       \
    tot1 += decode[4]*c;       \
    tot2 += decode[5]*c;

#define stbir__3_coeff_only()  \
    float tot0, tot1, tot2, c; \
    c = hc[0];                 \
    tot0 = decode[0]*c;        \
    tot1 = decode[1]*c;        \
    tot2 = decode[2]*c;        \
    c = hc[1];                 \
    tot0 += decode[3]*c;       \
    tot1 += decode[4]*c;       \
    tot2 += decode[5]*c;       \
    c = hc[2];                 \
    tot0 += decode[6]*c;       \
    tot1 += decode[7]*c;       \
    tot2 += decode[8]*c;

#define stbir__store_output_tiny()                \
    output[0] = tot0;                             \
    output[1] = tot1;                             \
    output[2] = tot2;                             \
    stbir__next_contributor();                    \
    output += 3;

#define stbir__4_coeff_start()      \
    float tota0,tota1,tota2,totb0,totb1,totb2,totc0,totc1,totc2,totd0,totd1,totd2,c;  \
    c = hc[0];                      \
    tota0 = decode[0]*c;            \
    tota1 = decode[1]*c;            \
    tota2 = decode[2]*c;            \
    c = hc[1];                      \
    totb0 = decode[3]*c;            \
    totb1 = decode[4]*c;            \
    totb2 = decode[5]*c;            \
    c = hc[2];                      \
    totc0 = decode[6]*c;            \
    totc1 = decode[7]*c;            \
    totc2 = decode[8]*c;            \
    c = hc[3];                      \
    totd0 = decode[9]*c;            \
    totd1 = decode[10]*c;           \
    totd2 = decode[11]*c;

#define stbir__4_coeff_continue_from_4( ofs )  \
    c = hc[0+(ofs)];                           \
    tota0 += decode[0+(ofs)*3]*c;              \
    tota1 += decode[1+(ofs)*3]*c;              \
    tota2 += decode[2+(ofs)*3]*c;              \
    c = hc[1+(ofs)];                           \
    totb0 += decode[3+(ofs)*3]*c;              \
    totb1 += decode[4+(ofs)*3]*c;              \
    totb2 += decode[5+(ofs)*3]*c;              \
    c = hc[2+(ofs)];                           \
    totc0 += decode[6+(ofs)*3]*c;              \
    totc1 += decode[7+(ofs)*3]*c;              \
    totc2 += decode[8+(ofs)*3]*c;              \
    c = hc[3+(ofs)];                           \
    totd0 += decode[9+(ofs)*3]*c;              \
    totd1 += decode[10+(ofs)*3]*c;             \
    totd2 += decode[11+(ofs)*3]*c;

#define stbir__1_coeff_remnant( ofs )  \
    c = hc[0+(ofs)];                   \
    tota0 += decode[0+(ofs)*3]*c;      \
    tota1 += decode[1+(ofs)*3]*c;      \
    tota2 += decode[2+(ofs)*3]*c;

#define stbir__2_coeff_remnant( ofs )  \
    c = hc[0+(ofs)];                   \
    tota0 += decode[0+(ofs)*3]*c;      \
    tota1 += decode[1+(ofs)*3]*c;      \
    tota2 += decode[2+(ofs)*3]*c;      \
    c = hc[1+(ofs)];                   \
    totb0 += decode[3+(ofs)*3]*c;      \
    totb1 += decode[4+(ofs)*3]*c;      \
    totb2 += decode[5+(ofs)*3]*c;      \

#define stbir__3_coeff_remnant( ofs )  \
    c = hc[0+(ofs)];                   \
    tota0 += decode[0+(ofs)*3]*c;      \
    tota1 += decode[1+(ofs)*3]*c;      \
    tota2 += decode[2+(ofs)*3]*c;      \
    c = hc[1+(ofs)];                   \
    totb0 += decode[3+(ofs)*3]*c;      \
    totb1 += decode[4+(ofs)*3]*c;      \
    totb2 += decode[5+(ofs)*3]*c;      \
    c = hc[2+(ofs)];                   \
    totc0 += decode[6+(ofs)*3]*c;      \
    totc1 += decode[7+(ofs)*3]*c;      \
    totc2 += decode[8+(ofs)*3]*c;

#define stbir__store_output()                     \
    output[0] = (tota0+totc0)+(totb0+totd0);      \
    output[1] = (tota1+totc1)+(totb1+totd1);      \
    output[2] = (tota2+totc2)+(totb2+totd2);      \
    stbir__next_contributor();                    \
    output += 3;

#endif

#define STBIR__horizontal_channels 3
#define STB_IMAGE_RESIZE_DO_HORIZONTALS
#include STBIR__HEADER_FILENAME

//=================
// Do 4 channel horizontal routines

#ifdef STBIR_SIMD

#define stbir__1_coeff_only()             \
    stbir__simdf tot,c;                   \
    STBIR_SIMD_NO_UNROLL(decode);         \
    stbir__simdf_load1( c, hc );          \
    stbir__simdf_0123to0000( c, c );      \
    stbir__simdf_mult_mem( tot, c, decode );

#define stbir__2_coeff_only()                       \
    stbir__simdf tot,c,cs;                          \
    STBIR_SIMD_NO_UNROLL(decode);                   \
    stbir__simdf_load2( cs, hc );                   \
    stbir__simdf_0123to0000( c, cs );               \
    stbir__simdf_mult_mem( tot, c, decode );        \
    stbir__simdf_0123to1111( c, cs );               \
    stbir__simdf_madd_mem( tot, tot, c, decode+4 );

#define stbir__3_coeff_only()                       \
    stbir__simdf tot,c,cs;                          \
    STBIR_SIMD_NO_UNROLL(decode);                   \
    stbir__simdf_load( cs, hc );                    \
    stbir__simdf_0123to0000( c, cs );               \
    stbir__simdf_mult_mem( tot, c, decode );        \
    stbir__simdf_0123to1111( c, cs );               \
    stbir__simdf_madd_mem( tot, tot, c, decode+4 ); \
    stbir__simdf_0123to2222( c, cs );               \
    stbir__simdf_madd_mem( tot, tot, c, decode+8 );

#define stbir__store_output_tiny()                \
    stbir__simdf_store( output, tot );            \
    stbir__next_contributor();                    \
    output += 4;

#ifdef STBIR_SIMD8

#define stbir__4_coeff_start()                     \
    stbir__simdf8 tot0,c,cs; stbir__simdf t;  \
    STBIR_SIMD_NO_UNROLL(decode);                  \
    stbir__simdf8_load4b( cs, hc );                \
    stbir__simdf8_0123to00001111( c, cs );         \
    stbir__simdf8_mult_mem( tot0, c, decode );     \
    stbir__simdf8_0123to22223333( c, cs );         \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+8 );

#define stbir__4_coeff_continue_from_4( ofs )                  \
    STBIR_SIMD_NO_UNROLL(decode);                              \
    stbir__simdf8_load4b( cs, hc + (ofs) );                    \
    stbir__simdf8_0123to00001111( c, cs );                     \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*4 );   \
    stbir__simdf8_0123to22223333( c, cs );                     \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*4+8 );

#define stbir__1_coeff_remnant( ofs )                          \
    STBIR_SIMD_NO_UNROLL(decode);                              \
    stbir__simdf_load1rep4( t, hc + (ofs) );                   \
    stbir__simdf8_madd_mem4( tot0, tot0, t, decode+(ofs)*4 );

#define stbir__2_coeff_remnant( ofs )                          \
    STBIR_SIMD_NO_UNROLL(decode);                              \
    stbir__simdf8_load4b( cs, hc + (ofs) - 2 );                \
    stbir__simdf8_0123to22223333( c, cs );                     \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*4 );

 #define stbir__3_coeff_remnant( ofs )                         \
    STBIR_SIMD_NO_UNROLL(decode);                              \
    stbir__simdf8_load4b( cs, hc + (ofs) );                    \
    stbir__simdf8_0123to00001111( c, cs );                     \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*4 );   \
    stbir__simdf8_0123to2222( t, cs );                         \
    stbir__simdf8_madd_mem4( tot0, tot0, t, decode+(ofs)*4+8 );

// polyphase period 1 for 4 channels (when have exact downsamples 2:1, 3:1, etc - common for mipmaps)
#define stbir__period1_setup_4()  stbir__simdf8 P01, P23, cs; stbir__simdf8_load4b( cs, hc ); stbir__simdf8_0123to00001111( P01, cs ); stbir__simdf8_0123to22223333( P23, cs );
#define stbir__period1_setup_5()  stbir__simdf PR; stbir__period1_setup_4() stbir__simdf_load1rep4( PR, hc + 4 );
#define stbir__period1_setup_8()  stbir__simdf8 P45, P67; stbir__period1_setup_4() stbir__simdf8_load4b( cs, hc + 4 ); stbir__simdf8_0123to00001111( P45, cs ); stbir__simdf8_0123to22223333( P67, cs );
#define stbir__period1_setup_9()  stbir__simdf PR; stbir__period1_setup_8() stbir__simdf_load1rep4( PR, hc + 8 );
#define stbir__period1_setup_11() stbir__simdf8 P89; stbir__simdf PR; stbir__period1_setup_8() stbir__simdf8_load4b( cs, hc + 8 ); stbir__simdf8_0123to00001111( P89, cs ); stbir__simdf8_0123to2222( PR, cs );
#define stbir__period1_setup_12() stbir__simdf8 P89, PAB; stbir__period1_setup_8() stbir__simdf8_load4b( cs, hc + 8 ); stbir__simdf8_0123to00001111( P89, cs ); stbir__simdf8_0123to22223333( PAB, cs );
#define stbir__period1_setup_13() stbir__simdf PR; stbir__period1_setup_12() stbir__simdf_load1rep4( PR, hc + 12 );
#define stbir__period1_setup_16() stbir__simdf8 PCD, PEF; stbir__period1_setup_12() stbir__simdf8_load4b( cs, hc + 12 ); stbir__simdf8_0123to00001111( PCD, cs ); stbir__simdf8_0123to22223333( PEF, cs );
#define stbir__period1_setup_17() stbir__simdf PR; stbir__period1_setup_16() stbir__simdf_load1rep4( PR, hc + 16 );

#define stbir__period1_4_coeff_start()                    \
    stbir__simdf8 tot0; stbir__simdf t;                   \
    STBIR_SIMD_NO_UNROLL(decode);                         \
    stbir__simdf8_mult_mem( tot0, P01, decode );          \
    stbir__simdf8_madd_mem( tot0, tot0, P23, decode + 8 );

#define stbir__period1_4_coeff_continue()                      \
    stbir__simdf8_madd_mem( tot0, tot0, P45, decode + 16 );    \
    stbir__simdf8_madd_mem( tot0, tot0, P67, decode + 24 );
#define stbir__period1_4_coeff_continue2()                     \
    stbir__simdf8_madd_mem( tot0, tot0, P89, decode + 32 );    \
    stbir__simdf8_madd_mem( tot0, tot0, PAB, decode + 40 );
#define stbir__period1_4_coeff_continue3()                     \
    stbir__simdf8_madd_mem( tot0, tot0, PCD, decode + 48 );    \
    stbir__simdf8_madd_mem( tot0, tot0, PEF, decode + 56 );

#define stbir__period1_1_coeff_remnant( ofs )                  \
    stbir__simdf8_madd_mem4( tot0, tot0, PR, decode + (ofs) * 4 );
#define stbir__period1_3_coeff_remnant( ofs )                      \
    stbir__simdf8_madd_mem( tot0, tot0, P89, decode + (ofs) * 4 );  \
    stbir__simdf8_madd_mem4( tot0, tot0, PR, decode + (ofs) * 4 + 8 );

#define stbir__period1_store_output()                                                  \
    stbir__simdf8_add4halves( t, stbir__if_simdf8_cast_to_simdf4(tot0), tot0 );        \
    stbir__simdf_store( output, t );                                                   \
    decode_buffer += advance;                                                          \
    output += 4;

#define stbir__store_output()                      \
    stbir__simdf8_add4halves( t, stbir__if_simdf8_cast_to_simdf4(tot0), tot0 );     \
    stbir__simdf_store( output, t );               \
    stbir__next_contributor();                    \
    output += 4;

#else

#define stbir__4_coeff_start()                        \
    stbir__simdf tot0,tot1,c,cs;                      \
    STBIR_SIMD_NO_UNROLL(decode);                     \
    stbir__simdf_load( cs, hc );                      \
    stbir__simdf_0123to0000( c, cs );                 \
    stbir__simdf_mult_mem( tot0, c, decode );         \
    stbir__simdf_0123to1111( c, cs );                 \
    stbir__simdf_mult_mem( tot1, c, decode+4 );       \
    stbir__simdf_0123to2222( c, cs );                 \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+8 ); \
    stbir__simdf_0123to3333( c, cs );                 \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+12 );

#define stbir__4_coeff_continue_from_4( ofs )                  \
    STBIR_SIMD_NO_UNROLL(decode);                              \
    stbir__simdf_load( cs, hc + (ofs) );                       \
    stbir__simdf_0123to0000( c, cs );                          \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*4 );    \
    stbir__simdf_0123to1111( c, cs );                          \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*4+4 );  \
    stbir__simdf_0123to2222( c, cs );                          \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*4+8 );  \
    stbir__simdf_0123to3333( c, cs );                          \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*4+12 );

#define stbir__1_coeff_remnant( ofs )                       \
    STBIR_SIMD_NO_UNROLL(decode);                           \
    stbir__simdf_load1( c, hc + (ofs) );                    \
    stbir__simdf_0123to0000( c, c );                        \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*4 );

#define stbir__2_coeff_remnant( ofs )                         \
    STBIR_SIMD_NO_UNROLL(decode);                             \
    stbir__simdf_load2( cs, hc + (ofs) );                     \
    stbir__simdf_0123to0000( c, cs );                         \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*4 );   \
    stbir__simdf_0123to1111( c, cs );                         \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*4+4 );

#define stbir__3_coeff_remnant( ofs )                          \
    STBIR_SIMD_NO_UNROLL(decode);                              \
    stbir__simdf_load( cs, hc + (ofs) );                       \
    stbir__simdf_0123to0000( c, cs );                          \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*4 );    \
    stbir__simdf_0123to1111( c, cs );                          \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*4+4 );  \
    stbir__simdf_0123to2222( c, cs );                          \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*4+8 );

// polyphase period 1 for 4 channels (when have exact downsamples 2:1, 3:1, etc - common for mipmaps)
#define stbir__period1_setup_4()  stbir__simdf P0, P1, P2, P3; stbir__simdf_load1frep4( P0, hc[0] ); stbir__simdf_load1frep4( P1, hc[1] ); stbir__simdf_load1frep4( P2, hc[2] ); stbir__simdf_load1frep4( P3, hc[3] );
#define stbir__period1_setup_5()  stbir__simdf PR; stbir__period1_setup_4() stbir__simdf_load1frep4( PR, hc[4] );
#define stbir__period1_setup_8()  stbir__simdf P4, P5, P6, P7; stbir__period1_setup_4() stbir__simdf_load1frep4( P4, hc[4] ); stbir__simdf_load1frep4( P5, hc[5] ); stbir__simdf_load1frep4( P6, hc[6] ); stbir__simdf_load1frep4( P7, hc[7] );
#define stbir__period1_setup_9()  stbir__simdf PR; stbir__period1_setup_8() stbir__simdf_load1frep4( PR, hc[8] );
#define stbir__period1_setup_11() stbir__simdf P8, P9, P10; stbir__period1_setup_8() stbir__simdf_load1frep4( P8, hc[8] ); stbir__simdf_load1frep4( P9, hc[9] ); stbir__simdf_load1frep4( P10, hc[10] );
#define stbir__period1_setup_12() stbir__simdf P8, P9, P10, P11; stbir__period1_setup_8() stbir__simdf_load1frep4( P8, hc[8] ); stbir__simdf_load1frep4( P9, hc[9] ); stbir__simdf_load1frep4( P10, hc[10] ); stbir__simdf_load1frep4( P11, hc[11] );
#define stbir__period1_setup_13() stbir__simdf PR; stbir__period1_setup_12() stbir__simdf_load1frep4( PR, hc[12] );
#define stbir__period1_setup_16() stbir__simdf P12, P13, P14, P15; stbir__period1_setup_12() stbir__simdf_load1frep4( P12, hc[12] ); stbir__simdf_load1frep4( P13, hc[13] ); stbir__simdf_load1frep4( P14, hc[14] ); stbir__simdf_load1frep4( P15, hc[15] );
#define stbir__period1_setup_17() stbir__simdf PR; stbir__period1_setup_16() stbir__simdf_load1frep4( PR, hc[16] );

#define stbir__period1_4_coeff_start()                   \
    stbir__simdf tot0, tot1;                             \
    STBIR_SIMD_NO_UNROLL(decode);                        \
    stbir__simdf_mult_mem( tot0, P0, decode );           \
    stbir__simdf_mult_mem( tot1, P1, decode + 4 );       \
    stbir__simdf_madd_mem( tot0, tot0, P2, decode + 8 ); \
    stbir__simdf_madd_mem( tot1, tot1, P3, decode + 12 );

#define stbir__period1_4_coeff_continue()                  \
    stbir__simdf_madd_mem( tot0, tot0, P4, decode + 16 ); \
    stbir__simdf_madd_mem( tot1, tot1, P5, decode + 20 ); \
    stbir__simdf_madd_mem( tot0, tot0, P6, decode + 24 ); \
    stbir__simdf_madd_mem( tot1, tot1, P7, decode + 28 );
#define stbir__period1_4_coeff_continue2()                 \
    stbir__simdf_madd_mem( tot0, tot0, P8, decode + 32 ); \
    stbir__simdf_madd_mem( tot1, tot1, P9, decode + 36 ); \
    stbir__simdf_madd_mem( tot0, tot0, P10, decode + 40 ); \
    stbir__simdf_madd_mem( tot1, tot1, P11, decode + 44 );
#define stbir__period1_4_coeff_continue3()                 \
    stbir__simdf_madd_mem( tot0, tot0, P12, decode + 48 ); \
    stbir__simdf_madd_mem( tot1, tot1, P13, decode + 52 ); \
    stbir__simdf_madd_mem( tot0, tot0, P14, decode + 56 ); \
    stbir__simdf_madd_mem( tot1, tot1, P15, decode + 60 );

#define stbir__period1_1_coeff_remnant( ofs )              \
    stbir__simdf_madd_mem( tot0, tot0, PR, decode + (ofs) * 4 );
#define stbir__period1_3_coeff_remnant( ofs )                       \
    stbir__simdf_madd_mem( tot0, tot0, P8, decode + (ofs) * 4 );      \
    stbir__simdf_madd_mem( tot1, tot1, P9, decode + (ofs) * 4 + 4 );  \
    stbir__simdf_madd_mem( tot0, tot0, P10, decode + (ofs) * 4 + 8 );

#define stbir__period1_store_output()                  \
    stbir__simdf_add( tot0, tot0, tot1 );              \
    stbir__simdf_store( output, tot0 );                \
    decode_buffer += advance;                          \
    output += 4;

#define stbir__store_output()                     \
    stbir__simdf_add( tot0, tot0, tot1 );         \
    stbir__simdf_store( output, tot0 );           \
    stbir__next_contributor();                    \
    output += 4;

#endif

#else

#define stbir__1_coeff_only()         \
    float p0,p1,p2,p3,c;              \
    STBIR_SIMD_NO_UNROLL(decode);     \
    c = hc[0];                        \
    p0 = decode[0] * c;               \
    p1 = decode[1] * c;               \
    p2 = decode[2] * c;               \
    p3 = decode[3] * c;

#define stbir__2_coeff_only()         \
    float p0,p1,p2,p3,c;              \
    STBIR_SIMD_NO_UNROLL(decode);     \
    c = hc[0];                        \
    p0 = decode[0] * c;               \
    p1 = decode[1] * c;               \
    p2 = decode[2] * c;               \
    p3 = decode[3] * c;               \
    c = hc[1];                        \
    p0 += decode[4] * c;              \
    p1 += decode[5] * c;              \
    p2 += decode[6] * c;              \
    p3 += decode[7] * c;

#define stbir__3_coeff_only()         \
    float p0,p1,p2,p3,c;              \
    STBIR_SIMD_NO_UNROLL(decode);     \
    c = hc[0];                        \
    p0 = decode[0] * c;               \
    p1 = decode[1] * c;               \
    p2 = decode[2] * c;               \
    p3 = decode[3] * c;               \
    c = hc[1];                        \
    p0 += decode[4] * c;              \
    p1 += decode[5] * c;              \
    p2 += decode[6] * c;              \
    p3 += decode[7] * c;              \
    c = hc[2];                        \
    p0 += decode[8] * c;              \
    p1 += decode[9] * c;              \
    p2 += decode[10] * c;             \
    p3 += decode[11] * c;

#define stbir__store_output_tiny()                \
    output[0] = p0;                               \
    output[1] = p1;                               \
    output[2] = p2;                               \
    output[3] = p3;                               \
    stbir__next_contributor();                    \
    output += 4;

#define stbir__4_coeff_start()        \
    float x0,x1,x2,x3,y0,y1,y2,y3,c;  \
    STBIR_SIMD_NO_UNROLL(decode);     \
    c = hc[0];                        \
    x0 = decode[0] * c;               \
    x1 = decode[1] * c;               \
    x2 = decode[2] * c;               \
    x3 = decode[3] * c;               \
    c = hc[1];                        \
    y0 = decode[4] * c;               \
    y1 = decode[5] * c;               \
    y2 = decode[6] * c;               \
    y3 = decode[7] * c;               \
    c = hc[2];                        \
    x0 += decode[8] * c;              \
    x1 += decode[9] * c;              \
    x2 += decode[10] * c;             \
    x3 += decode[11] * c;             \
    c = hc[3];                        \
    y0 += decode[12] * c;             \
    y1 += decode[13] * c;             \
    y2 += decode[14] * c;             \
    y3 += decode[15] * c;

#define stbir__4_coeff_continue_from_4( ofs ) \
    STBIR_SIMD_NO_UNROLL(decode);     \
    c = hc[0+(ofs)];                  \
    x0 += decode[0+(ofs)*4] * c;      \
    x1 += decode[1+(ofs)*4] * c;      \
    x2 += decode[2+(ofs)*4] * c;      \
    x3 += decode[3+(ofs)*4] * c;      \
    c = hc[1+(ofs)];                  \
    y0 += decode[4+(ofs)*4] * c;      \
    y1 += decode[5+(ofs)*4] * c;      \
    y2 += decode[6+(ofs)*4] * c;      \
    y3 += decode[7+(ofs)*4] * c;      \
    c = hc[2+(ofs)];                  \
    x0 += decode[8+(ofs)*4] * c;      \
    x1 += decode[9+(ofs)*4] * c;      \
    x2 += decode[10+(ofs)*4] * c;     \
    x3 += decode[11+(ofs)*4] * c;     \
    c = hc[3+(ofs)];                  \
    y0 += decode[12+(ofs)*4] * c;     \
    y1 += decode[13+(ofs)*4] * c;     \
    y2 += decode[14+(ofs)*4] * c;     \
    y3 += decode[15+(ofs)*4] * c;

#define stbir__1_coeff_remnant( ofs ) \
    STBIR_SIMD_NO_UNROLL(decode);     \
    c = hc[0+(ofs)];                  \
    x0 += decode[0+(ofs)*4] * c;      \
    x1 += decode[1+(ofs)*4] * c;      \
    x2 += decode[2+(ofs)*4] * c;      \
    x3 += decode[3+(ofs)*4] * c;

#define stbir__2_coeff_remnant( ofs ) \
    STBIR_SIMD_NO_UNROLL(decode);     \
    c = hc[0+(ofs)];                  \
    x0 += decode[0+(ofs)*4] * c;      \
    x1 += decode[1+(ofs)*4] * c;      \
    x2 += decode[2+(ofs)*4] * c;      \
    x3 += decode[3+(ofs)*4] * c;      \
    c = hc[1+(ofs)];                  \
    y0 += decode[4+(ofs)*4] * c;      \
    y1 += decode[5+(ofs)*4] * c;      \
    y2 += decode[6+(ofs)*4] * c;      \
    y3 += decode[7+(ofs)*4] * c;

#define stbir__3_coeff_remnant( ofs ) \
    STBIR_SIMD_NO_UNROLL(decode);     \
    c = hc[0+(ofs)];                  \
    x0 += decode[0+(ofs)*4] * c;      \
    x1 += decode[1+(ofs)*4] * c;      \
    x2 += decode[2+(ofs)*4] * c;      \
    x3 += decode[3+(ofs)*4] * c;      \
    c = hc[1+(ofs)];                  \
    y0 += decode[4+(ofs)*4] * c;      \
    y1 += decode[5+(ofs)*4] * c;      \
    y2 += decode[6+(ofs)*4] * c;      \
    y3 += decode[7+(ofs)*4] * c;      \
    c = hc[2+(ofs)];                  \
    x0 += decode[8+(ofs)*4] * c;      \
    x1 += decode[9+(ofs)*4] * c;      \
    x2 += decode[10+(ofs)*4] * c;     \
    x3 += decode[11+(ofs)*4] * c;

#define stbir__store_output()                     \
    output[0] = x0 + y0;                          \
    output[1] = x1 + y1;                          \
    output[2] = x2 + y2;                          \
    output[3] = x3 + y3;                          \
    stbir__next_contributor();                    \
    output += 4;

#endif

#define STBIR__horizontal_channels 4
#define STB_IMAGE_RESIZE_DO_HORIZONTALS
#include STBIR__HEADER_FILENAME


//=================
// Do 7 channel horizontal routines

#ifdef STBIR_SIMD

#define stbir__1_coeff_only()                   \
    stbir__simdf tot0,tot1,c;                   \
    STBIR_SIMD_NO_UNROLL(decode);               \
    stbir__simdf_load1( c, hc );                \
    stbir__simdf_0123to0000( c, c );            \
    stbir__simdf_mult_mem( tot0, c, decode );   \
    stbir__simdf_mult_mem( tot1, c, decode+3 );

#define stbir__2_coeff_only()                         \
    stbir__simdf tot0,tot1,c,cs;                      \
    STBIR_SIMD_NO_UNROLL(decode);                     \
    stbir__simdf_load2( cs, hc );                     \
    stbir__simdf_0123to0000( c, cs );                 \
    stbir__simdf_mult_mem( tot0, c, decode );         \
    stbir__simdf_mult_mem( tot1, c, decode+3 );       \
    stbir__simdf_0123to1111( c, cs );                 \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+7 ); \
    stbir__simdf_madd_mem( tot1, tot1, c,decode+10 );

#define stbir__3_coeff_only()                           \
    stbir__simdf tot0,tot1,c,cs;                        \
    STBIR_SIMD_NO_UNROLL(decode);                       \
    stbir__simdf_load( cs, hc );                        \
    stbir__simdf_0123to0000( c, cs );                   \
    stbir__simdf_mult_mem( tot0, c, decode );           \
    stbir__simdf_mult_mem( tot1, c, decode+3 );         \
    stbir__simdf_0123to1111( c, cs );                   \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+7 );   \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+10 );  \
    stbir__simdf_0123to2222( c, cs );                   \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+14 );  \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+17 );

#define stbir__store_output_tiny()                \
    stbir__simdf_store( output+3, tot1 );         \
    stbir__simdf_store( output, tot0 );           \
    stbir__next_contributor();                    \
    output += 7;

#ifdef STBIR_SIMD8

#define stbir__4_coeff_start()                     \
    stbir__simdf8 tot0,tot1,c,cs;                  \
    STBIR_SIMD_NO_UNROLL(decode);                  \
    stbir__simdf8_load4b( cs, hc );                \
    stbir__simdf8_0123to00000000( c, cs );         \
    stbir__simdf8_mult_mem( tot0, c, decode );     \
    stbir__simdf8_0123to11111111( c, cs );         \
    stbir__simdf8_mult_mem( tot1, c, decode+7 );   \
    stbir__simdf8_0123to22222222( c, cs );         \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+14 );  \
    stbir__simdf8_0123to33333333( c, cs );         \
    stbir__simdf8_madd_mem( tot1, tot1, c, decode+21 );

#define stbir__4_coeff_continue_from_4( ofs )                   \
    STBIR_SIMD_NO_UNROLL(decode);                               \
    stbir__simdf8_load4b( cs, hc + (ofs) );                     \
    stbir__simdf8_0123to00000000( c, cs );                      \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*7 );    \
    stbir__simdf8_0123to11111111( c, cs );                      \
    stbir__simdf8_madd_mem( tot1, tot1, c, decode+(ofs)*7+7 );  \
    stbir__simdf8_0123to22222222( c, cs );                      \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*7+14 ); \
    stbir__simdf8_0123to33333333( c, cs );                      \
    stbir__simdf8_madd_mem( tot1, tot1, c, decode+(ofs)*7+21 );

#define stbir__1_coeff_remnant( ofs )                           \
    STBIR_SIMD_NO_UNROLL(decode);                               \
    stbir__simdf8_load1b( c, hc + (ofs) );                      \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*7 );

#define stbir__2_coeff_remnant( ofs )                           \
    STBIR_SIMD_NO_UNROLL(decode);                               \
    stbir__simdf8_load1b( c, hc + (ofs) );                      \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*7 );    \
    stbir__simdf8_load1b( c, hc + (ofs)+1 );                    \
    stbir__simdf8_madd_mem( tot1, tot1, c, decode+(ofs)*7+7 );

#define stbir__3_coeff_remnant( ofs )                           \
    STBIR_SIMD_NO_UNROLL(decode);                               \
    stbir__simdf8_load4b( cs, hc + (ofs) );                     \
    stbir__simdf8_0123to00000000( c, cs );                      \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*7 );    \
    stbir__simdf8_0123to11111111( c, cs );                      \
    stbir__simdf8_madd_mem( tot1, tot1, c, decode+(ofs)*7+7 );  \
    stbir__simdf8_0123to22222222( c, cs );                      \
    stbir__simdf8_madd_mem( tot0, tot0, c, decode+(ofs)*7+14 );

// polyphase period 1 for 7 channels (when have exact downsamples 2:1, 3:1, etc - common for mipmaps)
#define stbir__period1_setup_4()  stbir__simdf8 P0, P1, P2, P3; stbir__simdf8_load1b( P0, hc ); stbir__simdf8_load1b( P1, hc + 1 ); stbir__simdf8_load1b( P2, hc + 2 ); stbir__simdf8_load1b( P3, hc + 3 );
#define stbir__period1_setup_5()  stbir__simdf8 PR; stbir__period1_setup_4() stbir__simdf8_load1b( PR, hc + 4 );
#define stbir__period1_setup_8()  stbir__simdf8 P4, P5, P6, P7; stbir__period1_setup_4() stbir__simdf8_load1b( P4, hc + 4 ); stbir__simdf8_load1b( P5, hc + 5 ); stbir__simdf8_load1b( P6, hc + 6 ); stbir__simdf8_load1b( P7, hc + 7 );
#define stbir__period1_setup_9()  stbir__simdf8 PR; stbir__period1_setup_8() stbir__simdf8_load1b( PR, hc + 8 );
#define stbir__period1_setup_11() stbir__simdf8 P8, P9, P10; stbir__period1_setup_8() stbir__simdf8_load1b( P8, hc + 8 ); stbir__simdf8_load1b( P9, hc + 9 ); stbir__simdf8_load1b( P10, hc + 10 );
#define stbir__period1_setup_12() stbir__simdf8 P8, P9, P10, P11; stbir__period1_setup_8() stbir__simdf8_load1b( P8, hc + 8 ); stbir__simdf8_load1b( P9, hc + 9 ); stbir__simdf8_load1b( P10, hc + 10 ); stbir__simdf8_load1b( P11, hc + 11 );
#define stbir__period1_setup_13() stbir__simdf8 PR; stbir__period1_setup_12() stbir__simdf8_load1b( PR, hc + 12 );
#define stbir__period1_setup_16() stbir__simdf8 P12, P13, P14, P15; stbir__period1_setup_12() stbir__simdf8_load1b( P12, hc + 12 ); stbir__simdf8_load1b( P13, hc + 13 ); stbir__simdf8_load1b( P14, hc + 14 ); stbir__simdf8_load1b( P15, hc + 15 );
#define stbir__period1_setup_17() stbir__simdf8 PR; stbir__period1_setup_16() stbir__simdf8_load1b( PR, hc + 16 );

#define stbir__period1_4_coeff_start()                       \
    stbir__simdf8 tot0, tot1;                                \
    STBIR_SIMD_NO_UNROLL(decode);                            \
    stbir__simdf8_mult_mem( tot0, P0, decode );              \
    stbir__simdf8_mult_mem( tot1, P1, decode + 7 );          \
    stbir__simdf8_madd_mem( tot0, tot0, P2, decode + 14 );   \
    stbir__simdf8_madd_mem( tot1, tot1, P3, decode + 21 );

#define stbir__period1_4_coeff_continue()                    \
    stbir__simdf8_madd_mem( tot0, tot0, P4, decode + 28 );   \
    stbir__simdf8_madd_mem( tot1, tot1, P5, decode + 35 );   \
    stbir__simdf8_madd_mem( tot0, tot0, P6, decode + 42 );   \
    stbir__simdf8_madd_mem( tot1, tot1, P7, decode + 49 );
#define stbir__period1_4_coeff_continue2()                   \
    stbir__simdf8_madd_mem( tot0, tot0, P8, decode + 56 );   \
    stbir__simdf8_madd_mem( tot1, tot1, P9, decode + 63 );   \
    stbir__simdf8_madd_mem( tot0, tot0, P10, decode + 70 );  \
    stbir__simdf8_madd_mem( tot1, tot1, P11, decode + 77 );
#define stbir__period1_4_coeff_continue3()                   \
    stbir__simdf8_madd_mem( tot0, tot0, P12, decode + 84 );  \
    stbir__simdf8_madd_mem( tot1, tot1, P13, decode + 91 );  \
    stbir__simdf8_madd_mem( tot0, tot0, P14, decode + 98 );  \
    stbir__simdf8_madd_mem( tot1, tot1, P15, decode + 105 );

#define stbir__period1_1_coeff_remnant( ofs )                \
    stbir__simdf8_madd_mem( tot0, tot0, PR, decode + (ofs) * 7 );
#define stbir__period1_3_coeff_remnant( ofs )                          \
    stbir__simdf8_madd_mem( tot0, tot0, P8, decode + (ofs) * 7 );       \
    stbir__simdf8_madd_mem( tot1, tot1, P9, decode + (ofs) * 7 + 7 );   \
    stbir__simdf8_madd_mem( tot0, tot0, P10, decode + (ofs) * 7 + 14 );

#define stbir__period1_store_output()                 \
    stbir__simdf8_add( tot0, tot0, tot1 );            \
    decode_buffer += advance;                         \
    output += 7;                                      \
    if ( output < output_end )                        \
    {                                                 \
      stbir__simdf8_store( output-7, tot0 );          \
      continue;                                       \
    }                                                 \
    stbir__simdf_store( output-7+3, stbir__simdf_swiz(stbir__simdf8_gettop4(tot0),0,0,1,2) ); \
    stbir__simdf_store( output-7, stbir__if_simdf8_cast_to_simdf4(tot0) );           \
    break;

#define stbir__store_output()                     \
    stbir__simdf8_add( tot0, tot0, tot1 );        \
    stbir__next_contributor();                    \
    output += 7;                                  \
    if ( output < output_end )                    \
    {                                             \
      stbir__simdf8_store( output-7, tot0 );      \
      continue;                                   \
    }                                             \
    stbir__simdf_store( output-7+3, stbir__simdf_swiz(stbir__simdf8_gettop4(tot0),0,0,1,2) ); \
    stbir__simdf_store( output-7, stbir__if_simdf8_cast_to_simdf4(tot0) );           \
    break;

#else

#define stbir__4_coeff_start()                    \
    stbir__simdf tot0,tot1,tot2,tot3,c,cs;        \
    STBIR_SIMD_NO_UNROLL(decode);                 \
    stbir__simdf_load( cs, hc );                  \
    stbir__simdf_0123to0000( c, cs );             \
    stbir__simdf_mult_mem( tot0, c, decode );     \
    stbir__simdf_mult_mem( tot1, c, decode+3 );   \
    stbir__simdf_0123to1111( c, cs );             \
    stbir__simdf_mult_mem( tot2, c, decode+7 );   \
    stbir__simdf_mult_mem( tot3, c, decode+10 );  \
    stbir__simdf_0123to2222( c, cs );             \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+14 );  \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+17 );  \
    stbir__simdf_0123to3333( c, cs );                   \
    stbir__simdf_madd_mem( tot2, tot2, c, decode+21 );  \
    stbir__simdf_madd_mem( tot3, tot3, c, decode+24 );

#define stbir__4_coeff_continue_from_4( ofs )                   \
    STBIR_SIMD_NO_UNROLL(decode);                               \
    stbir__simdf_load( cs, hc + (ofs) );                        \
    stbir__simdf_0123to0000( c, cs );                           \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*7 );     \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*7+3 );   \
    stbir__simdf_0123to1111( c, cs );                           \
    stbir__simdf_madd_mem( tot2, tot2, c, decode+(ofs)*7+7 );   \
    stbir__simdf_madd_mem( tot3, tot3, c, decode+(ofs)*7+10 );  \
    stbir__simdf_0123to2222( c, cs );                           \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*7+14 );  \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*7+17 );  \
    stbir__simdf_0123to3333( c, cs );                           \
    stbir__simdf_madd_mem( tot2, tot2, c, decode+(ofs)*7+21 );  \
    stbir__simdf_madd_mem( tot3, tot3, c, decode+(ofs)*7+24 );

#define stbir__1_coeff_remnant( ofs )                           \
    STBIR_SIMD_NO_UNROLL(decode);                               \
    stbir__simdf_load1( c, hc + (ofs) );                        \
    stbir__simdf_0123to0000( c, c );                            \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*7 );     \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*7+3 );   \

#define stbir__2_coeff_remnant( ofs )                           \
    STBIR_SIMD_NO_UNROLL(decode);                               \
    stbir__simdf_load2( cs, hc + (ofs) );                       \
    stbir__simdf_0123to0000( c, cs );                           \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*7 );     \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*7+3 );   \
    stbir__simdf_0123to1111( c, cs );                           \
    stbir__simdf_madd_mem( tot2, tot2, c, decode+(ofs)*7+7 );   \
    stbir__simdf_madd_mem( tot3, tot3, c, decode+(ofs)*7+10 );

#define stbir__3_coeff_remnant( ofs )                           \
    STBIR_SIMD_NO_UNROLL(decode);                               \
    stbir__simdf_load( cs, hc + (ofs) );                        \
    stbir__simdf_0123to0000( c, cs );                           \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*7 );     \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*7+3 );   \
    stbir__simdf_0123to1111( c, cs );                           \
    stbir__simdf_madd_mem( tot2, tot2, c, decode+(ofs)*7+7 );   \
    stbir__simdf_madd_mem( tot3, tot3, c, decode+(ofs)*7+10 );  \
    stbir__simdf_0123to2222( c, cs );                           \
    stbir__simdf_madd_mem( tot0, tot0, c, decode+(ofs)*7+14 );  \
    stbir__simdf_madd_mem( tot1, tot1, c, decode+(ofs)*7+17 );

// polyphase period 1 for 7 channels (when have exact downsamples 2:1, 3:1, etc - common for mipmaps)
#define stbir__period1_setup_4()  stbir__simdf P0, P1, P2, P3; stbir__simdf_load1frep4( P0, hc[0] ); stbir__simdf_load1frep4( P1, hc[1] ); stbir__simdf_load1frep4( P2, hc[2] ); stbir__simdf_load1frep4( P3, hc[3] );
#define stbir__period1_setup_5()  stbir__simdf PR; stbir__period1_setup_4() stbir__simdf_load1frep4( PR, hc[4] );
#define stbir__period1_setup_8()  stbir__simdf P4, P5, P6, P7; stbir__period1_setup_4() stbir__simdf_load1frep4( P4, hc[4] ); stbir__simdf_load1frep4( P5, hc[5] ); stbir__simdf_load1frep4( P6, hc[6] ); stbir__simdf_load1frep4( P7, hc[7] );
#define stbir__period1_setup_9()  stbir__simdf PR; stbir__period1_setup_8() stbir__simdf_load1frep4( PR, hc[8] );
#define stbir__period1_setup_11() stbir__simdf P8, P9, P10; stbir__period1_setup_8() stbir__simdf_load1frep4( P8, hc[8] ); stbir__simdf_load1frep4( P9, hc[9] ); stbir__simdf_load1frep4( P10, hc[10] );
#define stbir__period1_setup_12() stbir__simdf P8, P9, P10, P11; stbir__period1_setup_8() stbir__simdf_load1frep4( P8, hc[8] ); stbir__simdf_load1frep4( P9, hc[9] ); stbir__simdf_load1frep4( P10, hc[10] ); stbir__simdf_load1frep4( P11, hc[11] );
#define stbir__period1_setup_13() stbir__simdf PR; stbir__period1_setup_12() stbir__simdf_load1frep4( PR, hc[12] );
#define stbir__period1_setup_16() stbir__simdf P12, P13, P14, P15; stbir__period1_setup_12() stbir__simdf_load1frep4( P12, hc[12] ); stbir__simdf_load1frep4( P13, hc[13] ); stbir__simdf_load1frep4( P14, hc[14] ); stbir__simdf_load1frep4( P15, hc[15] );
#define stbir__period1_setup_17() stbir__simdf PR; stbir__period1_setup_16() stbir__simdf_load1frep4( PR, hc[16] );

#define stbir__period1_4_coeff_start()                      \
    stbir__simdf tot0, tot1, tot2, tot3;                    \
    STBIR_SIMD_NO_UNROLL(decode);                           \
    stbir__simdf_mult_mem( tot0, P0, decode );              \
    stbir__simdf_mult_mem( tot1, P0, decode + 3 );          \
    stbir__simdf_mult_mem( tot2, P1, decode + 7 );          \
    stbir__simdf_mult_mem( tot3, P1, decode + 10 );         \
    stbir__simdf_madd_mem( tot0, tot0, P2, decode + 14 );   \
    stbir__simdf_madd_mem( tot1, tot1, P2, decode + 17 );   \
    stbir__simdf_madd_mem( tot2, tot2, P3, decode + 21 );   \
    stbir__simdf_madd_mem( tot3, tot3, P3, decode + 24 );

#define stbir__period1_4_coeff_continue()                   \
    stbir__simdf_madd_mem( tot0, tot0, P4, decode + 28 );   \
    stbir__simdf_madd_mem( tot1, tot1, P4, decode + 31 );   \
    stbir__simdf_madd_mem( tot2, tot2, P5, decode + 35 );   \
    stbir__simdf_madd_mem( tot3, tot3, P5, decode + 38 );   \
    stbir__simdf_madd_mem( tot0, tot0, P6, decode + 42 );   \
    stbir__simdf_madd_mem( tot1, tot1, P6, decode + 45 );   \
    stbir__simdf_madd_mem( tot2, tot2, P7, decode + 49 );   \
    stbir__simdf_madd_mem( tot3, tot3, P7, decode + 52 );
#define stbir__period1_4_coeff_continue2()                  \
    stbir__simdf_madd_mem( tot0, tot0, P8, decode + 56 );   \
    stbir__simdf_madd_mem( tot1, tot1, P8, decode + 59 );   \
    stbir__simdf_madd_mem( tot2, tot2, P9, decode + 63 );   \
    stbir__simdf_madd_mem( tot3, tot3, P9, decode + 66 );   \
    stbir__simdf_madd_mem( tot0, tot0, P10, decode + 70 );  \
    stbir__simdf_madd_mem( tot1, tot1, P10, decode + 73 );  \
    stbir__simdf_madd_mem( tot2, tot2, P11, decode + 77 );  \
    stbir__simdf_madd_mem( tot3, tot3, P11, decode + 80 );
#define stbir__period1_4_coeff_continue3()                  \
    stbir__simdf_madd_mem( tot0, tot0, P12, decode + 84 );  \
    stbir__simdf_madd_mem( tot1, tot1, P12, decode + 87 );  \
    stbir__simdf_madd_mem( tot2, tot2, P13, decode + 91 );  \
    stbir__simdf_madd_mem( tot3, tot3, P13, decode + 94 );  \
    stbir__simdf_madd_mem( tot0, tot0, P14, decode + 98 );  \
    stbir__simdf_madd_mem( tot1, tot1, P14, decode + 101 ); \
    stbir__simdf_madd_mem( tot2, tot2, P15, decode + 105 ); \
    stbir__simdf_madd_mem( tot3, tot3, P15, decode + 108 );

#define stbir__period1_1_coeff_remnant( ofs )                       \
    stbir__simdf_madd_mem( tot0, tot0, PR, decode + (ofs) * 7 );    \
    stbir__simdf_madd_mem( tot1, tot1, PR, decode + (ofs) * 7 + 3 );
#define stbir__period1_3_coeff_remnant( ofs )                          \
    stbir__simdf_madd_mem( tot0, tot0, P8, decode + (ofs) * 7 );       \
    stbir__simdf_madd_mem( tot1, tot1, P8, decode + (ofs) * 7 + 3 );   \
    stbir__simdf_madd_mem( tot2, tot2, P9, decode + (ofs) * 7 + 7 );   \
    stbir__simdf_madd_mem( tot3, tot3, P9, decode + (ofs) * 7 + 10 );  \
    stbir__simdf_madd_mem( tot0, tot0, P10, decode + (ofs) * 7 + 14 ); \
    stbir__simdf_madd_mem( tot1, tot1, P10, decode + (ofs) * 7 + 17 );

#define stbir__period1_store_output()                 \
    stbir__simdf_add( tot0, tot0, tot2 );             \
    stbir__simdf_add( tot1, tot1, tot3 );             \
    stbir__simdf_store( output+3, tot1 );             \
    stbir__simdf_store( output, tot0 );               \
    decode_buffer += advance;                         \
    output += 7;

#define stbir__store_output()                     \
    stbir__simdf_add( tot0, tot0, tot2 );         \
    stbir__simdf_add( tot1, tot1, tot3 );         \
    stbir__simdf_store( output+3, tot1 );         \
    stbir__simdf_store( output, tot0 );           \
    stbir__next_contributor();                    \
    output += 7;

#endif

#else

#define stbir__1_coeff_only()        \
    float tot0, tot1, tot2, tot3, tot4, tot5, tot6, c; \
    c = hc[0];                       \
    tot0 = decode[0]*c;              \
    tot1 = decode[1]*c;              \
    tot2 = decode[2]*c;              \
    tot3 = decode[3]*c;              \
    tot4 = decode[4]*c;              \
    tot5 = decode[5]*c;              \
    tot6 = decode[6]*c;

#define stbir__2_coeff_only()        \
    float tot0, tot1, tot2, tot3, tot4, tot5, tot6, c; \
    c = hc[0];                       \
    tot0 = decode[0]*c;              \
    tot1 = decode[1]*c;              \
    tot2 = decode[2]*c;              \
    tot3 = decode[3]*c;              \
    tot4 = decode[4]*c;              \
    tot5 = decode[5]*c;              \
    tot6 = decode[6]*c;              \
    c = hc[1];                       \
    tot0 += decode[7]*c;             \
    tot1 += decode[8]*c;             \
    tot2 += decode[9]*c;             \
    tot3 += decode[10]*c;            \
    tot4 += decode[11]*c;            \
    tot5 += decode[12]*c;            \
    tot6 += decode[13]*c;            \

#define stbir__3_coeff_only()        \
    float tot0, tot1, tot2, tot3, tot4, tot5, tot6, c; \
    c = hc[0];                       \
    tot0 = decode[0]*c;              \
    tot1 = decode[1]*c;              \
    tot2 = decode[2]*c;              \
    tot3 = decode[3]*c;              \
    tot4 = decode[4]*c;              \
    tot5 = decode[5]*c;              \
    tot6 = decode[6]*c;              \
    c = hc[1];                       \
    tot0 += decode[7]*c;             \
    tot1 += decode[8]*c;             \
    tot2 += decode[9]*c;             \
    tot3 += decode[10]*c;            \
    tot4 += decode[11]*c;            \
    tot5 += decode[12]*c;            \
    tot6 += decode[13]*c;            \
    c = hc[2];                       \
    tot0 += decode[14]*c;            \
    tot1 += decode[15]*c;            \
    tot2 += decode[16]*c;            \
    tot3 += decode[17]*c;            \
    tot4 += decode[18]*c;            \
    tot5 += decode[19]*c;            \
    tot6 += decode[20]*c;            \

#define stbir__store_output_tiny()                \
    output[0] = tot0;                             \
    output[1] = tot1;                             \
    output[2] = tot2;                             \
    output[3] = tot3;                             \
    output[4] = tot4;                             \
    output[5] = tot5;                             \
    output[6] = tot6;                             \
    stbir__next_contributor();                    \
    output += 7;

#define stbir__4_coeff_start()    \
    float x0,x1,x2,x3,x4,x5,x6,y0,y1,y2,y3,y4,y5,y6,c; \
    STBIR_SIMD_NO_UNROLL(decode); \
    c = hc[0];                    \
    x0 = decode[0] * c;           \
    x1 = decode[1] * c;           \
    x2 = decode[2] * c;           \
    x3 = decode[3] * c;           \
    x4 = decode[4] * c;           \
    x5 = decode[5] * c;           \
    x6 = decode[6] * c;           \
    c = hc[1];                    \
    y0 = decode[7] * c;           \
    y1 = decode[8] * c;           \
    y2 = decode[9] * c;           \
    y3 = decode[10] * c;          \
    y4 = decode[11] * c;          \
    y5 = decode[12] * c;          \
    y6 = decode[13] * c;          \
    c = hc[2];                    \
    x0 += decode[14] * c;         \
    x1 += decode[15] * c;         \
    x2 += decode[16] * c;         \
    x3 += decode[17] * c;         \
    x4 += decode[18] * c;         \
    x5 += decode[19] * c;         \
    x6 += decode[20] * c;         \
    c = hc[3];                    \
    y0 += decode[21] * c;         \
    y1 += decode[22] * c;         \
    y2 += decode[23] * c;         \
    y3 += decode[24] * c;         \
    y4 += decode[25] * c;         \
    y5 += decode[26] * c;         \
    y6 += decode[27] * c;

#define stbir__4_coeff_continue_from_4( ofs ) \
    STBIR_SIMD_NO_UNROLL(decode);  \
    c = hc[0+(ofs)];               \
    x0 += decode[0+(ofs)*7] * c;   \
    x1 += decode[1+(ofs)*7] * c;   \
    x2 += decode[2+(ofs)*7] * c;   \
    x3 += decode[3+(ofs)*7] * c;   \
    x4 += decode[4+(ofs)*7] * c;   \
    x5 += decode[5+(ofs)*7] * c;   \
    x6 += decode[6+(ofs)*7] * c;   \
    c = hc[1+(ofs)];               \
    y0 += decode[7+(ofs)*7] * c;   \
    y1 += decode[8+(ofs)*7] * c;   \
    y2 += decode[9+(ofs)*7] * c;   \
    y3 += decode[10+(ofs)*7] * c;  \
    y4 += decode[11+(ofs)*7] * c;  \
    y5 += decode[12+(ofs)*7] * c;  \
    y6 += decode[13+(ofs)*7] * c;  \
    c = hc[2+(ofs)];               \
    x0 += decode[14+(ofs)*7] * c;  \
    x1 += decode[15+(ofs)*7] * c;  \
    x2 += decode[16+(ofs)*7] * c;  \
    x3 += decode[17+(ofs)*7] * c;  \
    x4 += decode[18+(ofs)*7] * c;  \
    x5 += decode[19+(ofs)*7] * c;  \
    x6 += decode[20+(ofs)*7] * c;  \
    c = hc[3+(ofs)];               \
    y0 += decode[21+(ofs)*7] * c;  \
    y1 += decode[22+(ofs)*7] * c;  \
    y2 += decode[23+(ofs)*7] * c;  \
    y3 += decode[24+(ofs)*7] * c;  \
    y4 += decode[25+(ofs)*7] * c;  \
    y5 += decode[26+(ofs)*7] * c;  \
    y6 += decode[27+(ofs)*7] * c;

#define stbir__1_coeff_remnant( ofs ) \
    STBIR_SIMD_NO_UNROLL(decode);  \
    c = hc[0+(ofs)];               \
    x0 += decode[0+(ofs)*7] * c;   \
    x1 += decode[1+(ofs)*7] * c;   \
    x2 += decode[2+(ofs)*7] * c;   \
    x3 += decode[3+(ofs)*7] * c;   \
    x4 += decode[4+(ofs)*7] * c;   \
    x5 += decode[5+(ofs)*7] * c;   \
    x6 += decode[6+(ofs)*7] * c;   \

#define stbir__2_coeff_remnant( ofs ) \
    STBIR_SIMD_NO_UNROLL(decode);  \
    c = hc[0+(ofs)];               \
    x0 += decode[0+(ofs)*7] * c;   \
    x1 += decode[1+(ofs)*7] * c;   \
    x2 += decode[2+(ofs)*7] * c;   \
    x3 += decode[3+(ofs)*7] * c;   \
    x4 += decode[4+(ofs)*7] * c;   \
    x5 += decode[5+(ofs)*7] * c;   \
    x6 += decode[6+(ofs)*7] * c;   \
    c = hc[1+(ofs)];               \
    y0 += decode[7+(ofs)*7] * c;   \
    y1 += decode[8+(ofs)*7] * c;   \
    y2 += decode[9+(ofs)*7] * c;   \
    y3 += decode[10+(ofs)*7] * c;  \
    y4 += decode[11+(ofs)*7] * c;  \
    y5 += decode[12+(ofs)*7] * c;  \
    y6 += decode[13+(ofs)*7] * c;  \

#define stbir__3_coeff_remnant( ofs ) \
    STBIR_SIMD_NO_UNROLL(decode);  \
    c = hc[0+(ofs)];               \
    x0 += decode[0+(ofs)*7] * c;   \
    x1 += decode[1+(ofs)*7] * c;   \
    x2 += decode[2+(ofs)*7] * c;   \
    x3 += decode[3+(ofs)*7] * c;   \
    x4 += decode[4+(ofs)*7] * c;   \
    x5 += decode[5+(ofs)*7] * c;   \
    x6 += decode[6+(ofs)*7] * c;   \
    c = hc[1+(ofs)];               \
    y0 += decode[7+(ofs)*7] * c;   \
    y1 += decode[8+(ofs)*7] * c;   \
    y2 += decode[9+(ofs)*7] * c;   \
    y3 += decode[10+(ofs)*7] * c;  \
    y4 += decode[11+(ofs)*7] * c;  \
    y5 += decode[12+(ofs)*7] * c;  \
    y6 += decode[13+(ofs)*7] * c;  \
    c = hc[2+(ofs)];               \
    x0 += decode[14+(ofs)*7] * c;  \
    x1 += decode[15+(ofs)*7] * c;  \
    x2 += decode[16+(ofs)*7] * c;  \
    x3 += decode[17+(ofs)*7] * c;  \
    x4 += decode[18+(ofs)*7] * c;  \
    x5 += decode[19+(ofs)*7] * c;  \
    x6 += decode[20+(ofs)*7] * c;  \

#define stbir__store_output()                     \
    output[0] = x0 + y0;                          \
    output[1] = x1 + y1;                          \
    output[2] = x2 + y2;                          \
    output[3] = x3 + y3;                          \
    output[4] = x4 + y4;                          \
    output[5] = x5 + y5;                          \
    output[6] = x6 + y6;                          \
    stbir__next_contributor();                    \
    output += 7;

#endif

#define STBIR__horizontal_channels 7
#define STB_IMAGE_RESIZE_DO_HORIZONTALS
#include STBIR__HEADER_FILENAME


// include all of the vertical resamplers (both scatter and gather versions)

#define STBIR__vertical_channels 1
#define STB_IMAGE_RESIZE_DO_VERTICALS
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 1
#define STB_IMAGE_RESIZE_DO_VERTICALS
#define STB_IMAGE_RESIZE_VERTICAL_CONTINUE
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 2
#define STB_IMAGE_RESIZE_DO_VERTICALS
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 2
#define STB_IMAGE_RESIZE_DO_VERTICALS
#define STB_IMAGE_RESIZE_VERTICAL_CONTINUE
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 3
#define STB_IMAGE_RESIZE_DO_VERTICALS
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 3
#define STB_IMAGE_RESIZE_DO_VERTICALS
#define STB_IMAGE_RESIZE_VERTICAL_CONTINUE
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 4
#define STB_IMAGE_RESIZE_DO_VERTICALS
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 4
#define STB_IMAGE_RESIZE_DO_VERTICALS
#define STB_IMAGE_RESIZE_VERTICAL_CONTINUE
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 5
#define STB_IMAGE_RESIZE_DO_VERTICALS
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 5
#define STB_IMAGE_RESIZE_DO_VERTICALS
#define STB_IMAGE_RESIZE_VERTICAL_CONTINUE
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 6
#define STB_IMAGE_RESIZE_DO_VERTICALS
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 6
#define STB_IMAGE_RESIZE_DO_VERTICALS
#define STB_IMAGE_RESIZE_VERTICAL_CONTINUE
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 7
#define STB_IMAGE_RESIZE_DO_VERTICALS
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 7
#define STB_IMAGE_RESIZE_DO_VERTICALS
#define STB_IMAGE_RESIZE_VERTICAL_CONTINUE
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 8
#define STB_IMAGE_RESIZE_DO_VERTICALS
#include STBIR__HEADER_FILENAME

#define STBIR__vertical_channels 8
#define STB_IMAGE_RESIZE_DO_VERTICALS
#define STB_IMAGE_RESIZE_VERTICAL_CONTINUE
#include STBIR__HEADER_FILENAME

typedef void STBIR_VERTICAL_GATHERFUNC( float * output, float const * coeffs, float const ** inputs, float const * input0_end );

static STBIR_VERTICAL_GATHERFUNC * stbir__vertical_gathers[ 8 ] =
{
  stbir__vertical_gather_with_1_coeffs,stbir__vertical_gather_with_2_coeffs,stbir__vertical_gather_with_3_coeffs,stbir__vertical_gather_with_4_coeffs,stbir__vertical_gather_with_5_coeffs,stbir__vertical_gather_with_6_coeffs,stbir__vertical_gather_with_7_coeffs,stbir__vertical_gather_with_8_coeffs
};

static STBIR_VERTICAL_GATHERFUNC * stbir__vertical_gathers_continues[ 8 ] =
{
  stbir__vertical_gather_with_1_coeffs_cont,stbir__vertical_gather_with_2_coeffs_cont,stbir__vertical_gather_with_3_coeffs_cont,stbir__vertical_gather_with_4_coeffs_cont,stbir__vertical_gather_with_5_coeffs_cont,stbir__vertical_gather_with_6_coeffs_cont,stbir__vertical_gather_with_7_coeffs_cont,stbir__vertical_gather_with_8_coeffs_cont
};

typedef void STBIR_VERTICAL_SCATTERFUNC( float ** outputs, float const * coeffs, float const * input, float const * input_end );

static STBIR_VERTICAL_SCATTERFUNC * stbir__vertical_scatter_sets[ 8 ] =
{
  stbir__vertical_scatter_with_1_coeffs,stbir__vertical_scatter_with_2_coeffs,stbir__vertical_scatter_with_3_coeffs,stbir__vertical_scatter_with_4_coeffs,stbir__vertical_scatter_with_5_coeffs,stbir__vertical_scatter_with_6_coeffs,stbir__vertical_scatter_with_7_coeffs,stbir__vertical_scatter_with_8_coeffs
};

static STBIR_VERTICAL_SCATTERFUNC * stbir__vertical_scatter_blends[ 8 ] =
{
  stbir__vertical_scatter_with_1_coeffs_cont,stbir__vertical_scatter_with_2_coeffs_cont,stbir__vertical_scatter_with_3_coeffs_cont,stbir__vertical_scatter_with_4_coeffs_cont,stbir__vertical_scatter_with_5_coeffs_cont,stbir__vertical_scatter_with_6_coeffs_cont,stbir__vertical_scatter_with_7_coeffs_cont,stbir__vertical_scatter_with_8_coeffs_cont
};


// pointer to an output pixel, relative to the output region (offset_x/offset_y are already folded into output_data)
static void * stbir__output_pixel_ptr( stbir__info const * stbir_info, int x, int y )
{
  return ( (char*) stbir_info->output_data ) + (ptrdiff_t)( y * stbir_info->output_stride_bytes ) + ( (size_t)x * (size_t)stbir_info->channels * (size_t)stbir__type_size[ stbir_info->output_type ] );
}

static void stbir__encode_scanline( stbir__info const * stbir_info, float * encode_buffer, int row, stbir__column_slice const * slice  STBIR_ONLY_PROFILE_GET_SPLIT_INFO )
{
  int num_pixels = slice->width;
  int channels = stbir_info->channels;
  ptrdiff_t width_times_channels = (ptrdiff_t)num_pixels * channels;
  void * output_buffer_data = stbir__output_pixel_ptr( stbir_info, slice->x0, row );
  void * output_buffer;

  // un-alpha weight if we need to
  if ( stbir_info->alpha_unweight )
  {
    STBIR_PROFILE_START( unalpha );
    stbir_info->alpha_unweight( encode_buffer, width_times_channels );
    STBIR_PROFILE_END( unalpha );
  }

  // write directly into output by default
  output_buffer = output_buffer_data;

  // if we have an output callback, we first convert the decode buffer in place (and then hand that to the callback)
  if ( stbir_info->out_pixels_cb )
    output_buffer = encode_buffer;

  STBIR_PROFILE_START( encode );
  // convert into the output buffer
  stbir_info->encode_pixels( output_buffer, width_times_channels, encode_buffer );
  STBIR_PROFILE_END( encode );

  // if we have an output callback, call it to send the data (x and y are in full output image space, not region relative)
  if ( stbir_info->out_pixels_cb )
    stbir_info->out_pixels_cb( output_buffer, num_pixels, stbir_info->offset_x + slice->x0, stbir_info->offset_y + row, stbir_info->user_data );
}


// Get the ring buffer pointer for an index
static float* stbir__get_ring_buffer_entry(stbir__info const * stbir_info, stbir__per_split_info const * split_info, int index )
{
  STBIR_ASSERT( index < stbir_info->ring_buffer_num_entries );

  #ifdef STBIR__SEPARATE_ALLOCATIONS
    return split_info->ring_buffers[ index ];
  #else
    return (float*) ( ( (char*) split_info->ring_buffer ) + ( (size_t) index * stbir_info->ring_buffer_length_bytes ) );
  #endif
}

// Get the specified scan line from the ring buffer
static float* stbir__get_ring_buffer_scanline(stbir__info const * stbir_info, stbir__per_split_info const * split_info, int get_scanline)
{
  int ring_buffer_index = (split_info->ring_buffer_begin_index + (get_scanline - split_info->ring_buffer_first_scanline)) % stbir_info->ring_buffer_num_entries;
  return stbir__get_ring_buffer_entry( stbir_info, split_info, ring_buffer_index );
}

// the horizontal pass over one slice's output pixels. 
static void stbir__resample_horizontal_gather(stbir__info const * stbir_info, float* output_buffer, float const * input_buffer, stbir__column_slice const * slice STBIR_ONLY_PROFILE_GET_SPLIT_INFO )
{
  stbir__sampler const * horizontal = &stbir_info->horizontal;
  int channels = stbir_info->effective_channels;
  int point_copy = ( horizontal->filter_enum == STBIR_FILTER_POINT_SAMPLE ) && ( stbir__scale_is_copy( &horizontal->scale_info ) );
  int prologue = horizontal->shared_prologue;
  int period = horizontal->shared_period;
  int epilogue_start = horizontal->num_contributors - horizontal->shared_epilogue;
  int x = slice->x0, x_end = slice->x0 + slice->width;
  int n0 = slice->first_n0;

  STBIR_PROFILE_START( horizontal );
  while ( x < x_end )
  {
    float const * decode = input_buffer + (ptrdiff_t)( n0 - slice->in_n0 ) * channels;  // input_buffer holds input pixel in_n0 at index zero
    int run_end = x_end;
    int entry_stride = horizontal->entry_floats * (int) sizeof( float );

    if ( ( period ) && ( x < epilogue_start ) )
    {
      if ( period == 1 )
      {
        // one set for the whole periodic region: the prologue is its own run, then the region is one run over that set
        run_end = ( x < prologue ) ? prologue : epilogue_start;
        if ( x >= prologue )
          entry_stride = 0;
      }
      else
      {
        // to the end of this pass over the block (from the prologue, the first pass), but not into the epilogue
        int pass = ( x < prologue ) ? 0 : ( ( x - prologue ) / period );
        run_end = prologue + ( pass + 1 ) * period;
        if ( run_end > epilogue_start )
          run_end = epilogue_start;
      }
      if ( run_end > x_end )
        run_end = x_end;
    }

    if ( point_copy )
      STBIR_MEMCPY( output_buffer, decode, ( run_end - x ) * sizeof( float ) * channels );
    else if ( ( entry_stride == 0 ) && ( stbir_info->horizontal_gather_period1 ) )
      stbir_info->horizontal_gather_period1( output_buffer, run_end - x, decode, stbir__output_column_entry( horizontal, x ), 0 );
    else
      stbir_info->horizontal_gather_channels( output_buffer, run_end - x, decode, stbir__output_column_entry( horizontal, x ), entry_stride );
    output_buffer += (ptrdiff_t)( run_end - x ) * channels;

    // where the next run starts reading: the epilogue, or a block boundary
    if ( run_end == epilogue_start )
      n0 = horizontal->shared_epilogue_n0;
    else if ( period )
      n0 = horizontal->shared_block_n0 + ( ( run_end - prologue ) / period ) * horizontal->shared_block_shift;
    x = run_end;
  }
  STBIR_PROFILE_END( horizontal );
}

// how many floats wide the buffer that the vertical pass works on is (decoded input pixels when vertical
//   first, horizontally resampled output pixels otherwise)
static ptrdiff_t stbir__vertical_width_times_channels( stbir__info const * stbir_info, stbir__column_slice const * slice )
{
  int width = ( stbir_info->vertical_first ) ? ( slice->in_n1 - slice->in_n0 + 1 ) : slice->width;
  return (ptrdiff_t)stbir_info->effective_channels * width;
}

static void stbir__resample_vertical_gather(stbir__info const * stbir_info, stbir__per_split_info* split_info, int n, int contrib_n0, int contrib_n1, float const * vertical_coefficients, stbir__column_slice const * slice )
{
  float* encode_buffer = split_info->vertical_buffer;
  float* decode_buffer = split_info->decode_buffer;
  int vertical_first = stbir_info->vertical_first;
  ptrdiff_t width_times_channels = stbir__vertical_width_times_channels( stbir_info, slice );

  STBIR_ASSERT( stbir_info->vertical.is_gather );

  // loop over the contributing scanlines and scale into the buffer
  STBIR_PROFILE_START( vertical );
  {
    int k = 0, total = contrib_n1 - contrib_n0 + 1;
    STBIR_ASSERT( total > 0 );
    do {
      float const * inputs[8];
      int i, cnt = total;
      if ( cnt > 8 )
        cnt = 8;
      for( i = 0 ; i < cnt ; i++ )
        inputs[ i ] = stbir__get_ring_buffer_scanline(stbir_info, split_info, k+i+contrib_n0 );

      // call the N scanlines at a time function (up to 8 scanlines of blending at once)
      ((k==0)?stbir__vertical_gathers:stbir__vertical_gathers_continues)[cnt-1]( (vertical_first) ? decode_buffer : encode_buffer, vertical_coefficients + k, inputs, inputs[0] + width_times_channels );
      k += cnt;
      total -= cnt;
    } while ( total );
  }
  STBIR_PROFILE_END( vertical );

  if ( vertical_first )
  {
    // Now resample the gathered vertical data in the horizontal axis into the encode buffer
    decode_buffer[ width_times_channels ] = 0.0f; // clear two over for horizontals with a remnant of 3
    decode_buffer[ width_times_channels+1 ] = 0.0f; 
    stbir__resample_horizontal_gather(stbir_info, encode_buffer, decode_buffer, slice  STBIR_ONLY_PROFILE_SET_SPLIT_INFO );
  }

  stbir__encode_scanline( stbir_info, encode_buffer, n, slice  STBIR_ONLY_PROFILE_SET_SPLIT_INFO );
}

static void stbir__decode_and_resample_for_vertical_gather_loop(stbir__info const * stbir_info, stbir__per_split_info* split_info, int n, stbir__column_slice const * slice)
{
  int ring_buffer_index;
  float* ring_buffer;

  // Decode the nth scanline from the source image into the decode buffer.
  stbir__decode_scanline( stbir_info, n, split_info->decode_buffer, slice  STBIR_ONLY_PROFILE_SET_SPLIT_INFO );

  // update new end scanline
  split_info->ring_buffer_last_scanline = n;

  // get ring buffer
  ring_buffer_index = (split_info->ring_buffer_begin_index + (split_info->ring_buffer_last_scanline - split_info->ring_buffer_first_scanline)) % stbir_info->ring_buffer_num_entries;
  ring_buffer = stbir__get_ring_buffer_entry(stbir_info, split_info, ring_buffer_index);

  // Now resample it into the ring buffer.
  stbir__resample_horizontal_gather( stbir_info, ring_buffer, split_info->decode_buffer, slice  STBIR_ONLY_PROFILE_SET_SPLIT_INFO );

  // Now it's sitting in the ring buffer ready to be used as source for the vertical sampling.
}

// A trimmed zero coefficient can make a row's range start later than the next row's ((4,4) then (3,6)), and the ring only
//   loads forward. So look ahead up to max_look rows from y for the lowest first scanline; *out_row is which row had it.
static int stbir__lowest_scanline_needed( stbir__sampler const * vertical, int y, int max_look, int * out_row )
{
  int start_n0, lowest_n0, n0, n1, j;

  stbir__output_range( vertical, y, &start_n0, &n1 );
  lowest_n0 = start_n0;
  *out_row = 0;
  for (j = 1; j <= max_look; j++)
  {
    stbir__output_range( vertical, y + j, &n0, &n1 );
    if ( n0 > start_n0 )
      break;
    if ( n0 < lowest_n0 )
    {
      lowest_n0 = n0;
      *out_row = j;
    }
  }
  return lowest_n0;
}

static void stbir__vertical_gather_loop( stbir__info const * stbir_info, stbir__per_split_info* split_info, int split_count, stbir__column_slice const * slice )
{
  stbir__sampler const * vertical = &stbir_info->vertical;
  int y, start_output_y, end_output_y;

  STBIR_ASSERT( vertical->is_gather );

  start_output_y = split_info->start_output_y;
  end_output_y = split_info[split_count-1].end_output_y;

  // initialize the ring buffer for gathering: start it at the lowest scanline the first rows need
  {
    int look = vertical->filter_pixel_margin * 3, unused;
    if ( look > ( end_output_y - start_output_y - 1 ) )
      look = end_output_y - start_output_y - 1;
    split_info->ring_buffer_begin_index = 0;
    split_info->ring_buffer_first_scanline = stbir__lowest_scanline_needed( vertical, start_output_y, look, &unused );
    split_info->ring_buffer_last_scanline = split_info->ring_buffer_first_scanline - 1; // means "empty"
  }

  for (y = start_output_y; y < end_output_y; y++)
  {
    int in_first_scanline, in_last_scanline;
    float const * vertical_coefficients = stbir__output_coefficients( vertical, y );

    stbir__output_range( vertical, y, &in_first_scanline, &in_last_scanline );

    // rows with a zero weight at either end aren't worth decoding (the pack's shifted sets when the vertical shares the
    //   horizontal's tables, kernels that are exactly zero at their support's edge)
    while ( ( in_first_scanline < in_last_scanline ) && ( vertical_coefficients[0] == 0.0f ) )
    {
      ++in_first_scanline;
      ++vertical_coefficients;
    }
    while ( ( in_last_scanline > in_first_scanline ) && ( vertical_coefficients[ in_last_scanline - in_first_scanline ] == 0.0f ) )
      --in_last_scanline;

    // make sure the indexing hasn't broken
    STBIR_ASSERT( in_first_scanline >= split_info->ring_buffer_first_scanline );

    // Load in new scanlines
    while (in_last_scanline > split_info->ring_buffer_last_scanline)
    {
      STBIR_ASSERT( ( split_info->ring_buffer_last_scanline - split_info->ring_buffer_first_scanline + 1 ) <= stbir_info->ring_buffer_num_entries );

      // make sure there was room in the ring buffer when we add new scanlines
      if ( ( split_info->ring_buffer_last_scanline - split_info->ring_buffer_first_scanline + 1 ) == stbir_info->ring_buffer_num_entries )
      {
        split_info->ring_buffer_first_scanline++;
        split_info->ring_buffer_begin_index++;
      }

      if ( stbir_info->vertical_first )
      {
        float * ring_buffer = stbir__get_ring_buffer_scanline( stbir_info, split_info, ++split_info->ring_buffer_last_scanline );
        // Decode the nth scanline from the source image into the decode buffer.
        stbir__decode_scanline( stbir_info, split_info->ring_buffer_last_scanline, ring_buffer, slice  STBIR_ONLY_PROFILE_SET_SPLIT_INFO );
      }
      else
      {
        stbir__decode_and_resample_for_vertical_gather_loop(stbir_info, split_info, split_info->ring_buffer_last_scanline + 1, slice);
      }
    }

    // Now all buffers should be ready to write a row of vertical sampling, so do it.
    stbir__resample_vertical_gather(stbir_info, split_info, y, in_first_scanline, in_last_scanline, vertical_coefficients, slice );
  }
}

#define STBIR__FLOAT_EMPTY_MARKER 3.0e+38F
#define STBIR__FLOAT_BUFFER_IS_EMPTY(ptr) ((ptr)[0]==STBIR__FLOAT_EMPTY_MARKER)

static void stbir__encode_first_scanline_from_scatter(stbir__info const * stbir_info, stbir__per_split_info* split_info, stbir__column_slice const * slice)
{
  // evict a scanline out into the output buffer
  float* ring_buffer_entry = stbir__get_ring_buffer_entry(stbir_info, split_info, split_info->ring_buffer_begin_index );

  // dump the scanline out
  stbir__encode_scanline( stbir_info, ring_buffer_entry, split_info->ring_buffer_first_scanline, slice  STBIR_ONLY_PROFILE_SET_SPLIT_INFO );

  // mark it as empty
  ring_buffer_entry[ 0 ] = STBIR__FLOAT_EMPTY_MARKER;

  // advance the first scanline
  split_info->ring_buffer_first_scanline++;
  if ( ++split_info->ring_buffer_begin_index == stbir_info->ring_buffer_num_entries )
    split_info->ring_buffer_begin_index = 0;
}

static void stbir__horizontal_resample_and_encode_first_scanline_from_scatter(stbir__info const * stbir_info, stbir__per_split_info* split_info, stbir__column_slice const * slice)
{
  // evict a scanline out into the output buffer

  float* ring_buffer_entry = stbir__get_ring_buffer_entry(stbir_info, split_info, split_info->ring_buffer_begin_index );

  // Now resample it into the buffer.
  stbir__resample_horizontal_gather( stbir_info, split_info->vertical_buffer, ring_buffer_entry, slice  STBIR_ONLY_PROFILE_SET_SPLIT_INFO );

  // dump the scanline out
  stbir__encode_scanline( stbir_info, split_info->vertical_buffer, split_info->ring_buffer_first_scanline, slice  STBIR_ONLY_PROFILE_SET_SPLIT_INFO );

  // mark it as empty
  ring_buffer_entry[ 0 ] = STBIR__FLOAT_EMPTY_MARKER;

  // advance the first scanline
  split_info->ring_buffer_first_scanline++;
  if ( ++split_info->ring_buffer_begin_index == stbir_info->ring_buffer_num_entries )
    split_info->ring_buffer_begin_index = 0;
}

static void stbir__resample_vertical_scatter(stbir__info const * stbir_info, stbir__per_split_info* split_info, int n0, int n1, float const * vertical_coefficients, float const * vertical_buffer, float const * vertical_buffer_end )
{
  STBIR_ASSERT( !stbir_info->vertical.is_gather );

  STBIR_PROFILE_START( vertical );
  {
    int k = 0, total = n1 - n0 + 1;
    STBIR_ASSERT( total > 0 );
    do {
      float * outputs[8];
      int i, n = total; if ( n > 8 ) n = 8;
      for( i = 0 ; i < n ; i++ )
      {
        outputs[ i ] = stbir__get_ring_buffer_scanline(stbir_info, split_info, k+i+n0 );
        if ( ( i ) && ( STBIR__FLOAT_BUFFER_IS_EMPTY( outputs[i] ) != STBIR__FLOAT_BUFFER_IS_EMPTY( outputs[0] ) ) ) // make sure runs are of the same type
        {
          n = i;
          break;
        }
      }
      // call the scatter to N scanlines at a time function (up to 8 scanlines of scattering at once)
      ((STBIR__FLOAT_BUFFER_IS_EMPTY( outputs[0] ))?stbir__vertical_scatter_sets:stbir__vertical_scatter_blends)[n-1]( outputs, vertical_coefficients + k, vertical_buffer, vertical_buffer_end );
      k += n;
      total -= n;
    } while ( total );
  }

  STBIR_PROFILE_END( vertical );
}

typedef void stbir__handle_scanline_for_scatter_func(stbir__info const * stbir_info, stbir__per_split_info* split_info, stbir__column_slice const * slice);

static void stbir__vertical_scatter_loop( stbir__info const * stbir_info, stbir__per_split_info* split_info, int split_count, stbir__column_slice const * slice )
{
  int y, start_output_y, end_output_y, start_input_y, end_input_y;
  stbir__contributors* vertical_contributors = stbir_info->vertical.contributors;
  float const * vertical_coefficients = stbir_info->vertical.coefficients;
  stbir__handle_scanline_for_scatter_func * handle_scanline_for_scatter;
  void * scanline_scatter_buffer;
  void * scanline_scatter_buffer_end;
  int on_first_input_y, last_input_y;
  ptrdiff_t width_times_channels = stbir__vertical_width_times_channels( stbir_info, slice );

  STBIR_ASSERT( !stbir_info->vertical.is_gather );

  start_output_y = split_info->start_output_y;
  end_output_y = split_info[split_count-1].end_output_y;  // may do multiple split counts

  start_input_y = split_info->start_input_y;
  end_input_y = split_info[split_count-1].end_input_y;

  // adjust for starting offset start_input_y
  y = start_input_y + stbir_info->vertical.filter_pixel_margin;
  vertical_contributors += y ;
  vertical_coefficients += (size_t)stbir_info->vertical.coefficient_width * y;

  if ( stbir_info->vertical_first )
  {
    handle_scanline_for_scatter = stbir__horizontal_resample_and_encode_first_scanline_from_scatter;
    scanline_scatter_buffer = split_info->decode_buffer;
  }
  else
  {
    handle_scanline_for_scatter = stbir__encode_first_scanline_from_scatter;
    scanline_scatter_buffer = split_info->vertical_buffer;
  }
  scanline_scatter_buffer_end = ( (char*) scanline_scatter_buffer ) + sizeof( float ) * width_times_channels;

  // initialize the ring buffer for scattering
  split_info->ring_buffer_first_scanline = start_output_y;
  split_info->ring_buffer_last_scanline = -1;
  split_info->ring_buffer_begin_index = -1;

  // mark all the buffers as empty to start
  for( y = 0 ; y < stbir_info->ring_buffer_num_entries ; y++ )
  {
    float * decode_buffer = stbir__get_ring_buffer_entry( stbir_info, split_info, y );
    decode_buffer[ width_times_channels ] = 0.0f; // clear two over for horizontals with a remnant of 3
    decode_buffer[ width_times_channels+1 ] = 0.0f; 
    decode_buffer[0] = STBIR__FLOAT_EMPTY_MARKER; // only used on scatter
  }

  // do the loop in input space
  on_first_input_y = 1;
  last_input_y = start_input_y;
  for (y = start_input_y ; y < end_input_y; y++)
  {
    int out_first_scanline, out_last_scanline;

    out_first_scanline = vertical_contributors->n0;
    out_last_scanline = vertical_contributors->n1;

    // this input row is ours if the output rows it contributes to overlap the split's rows at all
    if ( ( out_last_scanline >= out_first_scanline ) && ( out_last_scanline >= start_output_y ) && ( out_first_scanline < end_output_y ) )
    {
      float const * coefficients = vertical_coefficients;

      // keep track of the range actually seen for the next resize
      last_input_y = y;
      if ( ( on_first_input_y ) && ( y > start_input_y ) )
        split_info->start_input_y = y;
      on_first_input_y = 0;

      // clip the region
      if ( out_first_scanline < start_output_y )
      {
        coefficients += start_output_y - out_first_scanline;
        out_first_scanline = start_output_y;
      }

      if ( out_last_scanline >= end_output_y )
        out_last_scanline = end_output_y - 1;

      STBIR_ASSERT( out_last_scanline - out_first_scanline + 1 <= stbir_info->ring_buffer_num_entries );   // (the ring is capped to the band's rows, so after the clip)

      // if very first scanline, init the index
      if (split_info->ring_buffer_begin_index < 0)
        split_info->ring_buffer_begin_index = out_first_scanline - start_output_y;

      STBIR_ASSERT( split_info->ring_buffer_begin_index <= out_first_scanline );

      // Decode the nth scanline from the source image into the decode buffer.
      stbir__decode_scanline( stbir_info, y, split_info->decode_buffer, slice  STBIR_ONLY_PROFILE_SET_SPLIT_INFO );

      // When horizontal first, we resample horizontally into the vertical buffer before we scatter it out
      if ( !stbir_info->vertical_first )
        stbir__resample_horizontal_gather( stbir_info, split_info->vertical_buffer, split_info->decode_buffer, slice  STBIR_ONLY_PROFILE_SET_SPLIT_INFO );

      // Now it's sitting in the buffer ready to be distributed into the ring buffers.

      // evict from the ring buffer if it is full and this row needs a new entry
      if ( ( ( split_info->ring_buffer_last_scanline - split_info->ring_buffer_first_scanline + 1 ) == stbir_info->ring_buffer_num_entries ) &&
           ( out_last_scanline > split_info->ring_buffer_last_scanline ) )
        handle_scanline_for_scatter( stbir_info, split_info, slice );

      // Now the horizontal buffer is ready to write to all ring buffer rows, so do it.
      stbir__resample_vertical_scatter(stbir_info, split_info, out_first_scanline, out_last_scanline, coefficients, (float*)scanline_scatter_buffer, (float*)scanline_scatter_buffer_end );

      // update the end of the buffer
      if ( out_last_scanline > split_info->ring_buffer_last_scanline )
        split_info->ring_buffer_last_scanline = out_last_scanline;
    }
    ++vertical_contributors;
    vertical_coefficients += stbir_info->vertical.coefficient_width;
  }

  // now evict the scanlines that are left over in the ring buffer
  while ( split_info->ring_buffer_first_scanline < end_output_y )
    handle_scanline_for_scatter(stbir_info, split_info, slice);

  // update the end_input_y if we do multiple resizes with the same data
  ++last_input_y;
  for( y = 0 ; y < split_count; y++ )
    if ( split_info[y].end_input_y > last_input_y )
      split_info[y].end_input_y = last_input_y;
}


static void stbir__set_sampler(stbir__sampler * samp, stbir_filter filter, stbir__kernel_callback_new * kernel_new, stbir__support_callback_new * support_new, stbir__radius_callback_new * radius, stbir__kernel_callback * kernel_old, stbir__support_callback * support_old, stbir_edge edge, stbir__scale_info * scale_info, int always_gather, void * user_data )
{
  // set filter
  if (filter == 0)
  {
    filter = STBIR_DEFAULT_FILTER_DOWNSAMPLE; // default to downsample
    if ( stbir__scale_is_upsample( scale_info ) )
    {
      if ( stbir__scale_is_copy( scale_info ) )
        filter = STBIR_FILTER_POINT_SAMPLE;
      else
        filter = STBIR_DEFAULT_FILTER_UPSAMPLE;
    }
  }
  samp->filter_enum = filter;

  STBIR_ASSERT(samp->filter_enum != 0);
  STBIR_ASSERT((unsigned)samp->filter_enum < STBIR_FILTER_OTHER);
  samp->filter_kernel_new = stbir__builtin_kernels[ filter ];
  samp->filter_kernel_old = 0;
  samp->filter_support_new = stbir__builtin_supports[ filter ];
  samp->filter_radius = stbir__builtin_radii[ filter ];
  samp->filter_support_old = 0;

  // a user filter is a kernel with a support or a radius (stbir_set_filter_callbacks_new refuses both), or an old
  //   float kernel with an old float support (stbir_set_filter_callbacks)
  if ( ( kernel_new && ( support_new || radius ) ) || ( kernel_old && support_old ) )
  {
    samp->filter_kernel_new = kernel_new;
    samp->filter_kernel_old = kernel_old;
    samp->filter_support_new = support_new;
    samp->filter_radius = radius;
    samp->filter_support_old = support_old;
    samp->filter_enum = STBIR_FILTER_OTHER;
  }

  samp->edge = edge;
  samp->filter_pixel_width  = stbir__get_filter_pixel_width( samp->filter_radius, samp->filter_support_new, samp->filter_support_old, scale_info, user_data );
  // Gather is always better, but in extreme downsamples, you have to most or all of the data in memory
  //    For horizontal, we always have all the pixels, so we always use gather here (always_gather==1).
  //    For vertical, we use gather if scaling up (which means we will have samp->filter_pixel_width
  //    scanlines in memory at once).
  samp->is_gather = 0;
  if ( stbir__scale_is_upsample( scale_info ) )
    samp->is_gather = 1;
  else if ( ( always_gather ) || ( samp->filter_pixel_width <= STBIR_FORCE_GATHER_FILTER_SCANLINES_AMOUNT ) )
    samp->is_gather = 2;

  // pre calculate stuff based on the above
  samp->coefficient_width = stbir__get_coefficient_width(samp, samp->is_gather, user_data);

  // filter_pixel_width is the conservative size in pixels of input that affect an output pixel.
  //   In rare cases (only with 2 pix to 1 pix with the default filters), it's possible that the 
  //   filter will extend before or after the scanline beyond just one extra entire copy of the 
  //   scanline (we would hit the edge twice). We don't let you do that, so we clamp the total 
  //   width to 3x the total of input pixel (once for the scanline, once for the left side 
  //   overhang, and once for the right side). We only do this for edge mode, since the other 
  //   modes can just re-edge clamp back in again.
  if ( edge == STBIR_EDGE_WRAP )
    if ( ( samp->filter_pixel_width / 3 ) > scale_info->input_full_size )  
      samp->filter_pixel_width = scale_info->input_full_size * 3;

  // This is how much to expand buffers to account for filters seeking outside
  // the image boundaries.
  samp->filter_pixel_margin = samp->filter_pixel_width / 2;
  
  // filter_pixel_margin is the amount that this filter can overhang on just one side of either 
  //   end of the scanline (left or the right). Since we only allow you to overhang 1 scanline's 
  //   worth of pixels, we clamp this one side of overhang to the input scanline size. Again, 
  //   this clamping only happens in rare cases with the default filters (2 pix to 1 pix). 
  if ( edge == STBIR_EDGE_WRAP )
    if ( samp->filter_pixel_margin > scale_info->input_full_size )
      samp->filter_pixel_margin = scale_info->input_full_size;

  samp->num_contributors = stbir__get_contributors(samp, samp->is_gather);

  samp->entry_floats = 0;
  samp->shared_period = 0;
  samp->shared_prologue = 0;
  samp->shared_epilogue = 0;
  samp->contributors_size = (size_t)samp->num_contributors * sizeof(stbir__contributors);
  samp->coefficients_size = (size_t)samp->num_contributors * STBIR__COLUMN_ENTRY_FLOATS( samp->coefficient_width ) * sizeof(float) + sizeof(float)*STBIR_INPUT_CALLBACK_PADDING; // room for the horizontal's column entry ints, plus padding
}

// Walks input pixels from start toward end (step is -1 or 1) and returns the last one that contributes to any output,
//   or fallback if none did. When the output region is only a pixel or two wide the starting guess can be past the
//   whole region, so a non contributing pixel only stops the walk once contributing ones have been seen.
static int stbir__last_contributing_input( int start, int end, int step, int fallback, stbir_real in_pixels_radius, stbir_real support, stbir__scale_info const * scale_info )
{
  int n = start;
  int last = fallback;
  int seen = 0;

  while ( ( step < 0 ) ? ( n >= end ) : ( n <= end ) )
  {
    int out_first_pixel, out_last_pixel;
    stbir__input_out_pixel_range( scale_info, n, in_pixels_radius, support, &out_first_pixel, &out_last_pixel );
    if ( out_first_pixel > out_last_pixel )
    {
      if ( seen )
        break;
    }
    else
    {
      last = n;
      seen = 1;
    }
    n += step;
  }
  return last;
}

static void stbir__get_conservative_extents( stbir__sampler * samp, stbir__contributors * range, void * user_data )
{
  stbir_real scale = samp->scale_info.scale;
  stbir_real in_pixels_radius = stbir__filter_radius_in_input_pixels( samp->filter_radius, samp->filter_support_new, samp->filter_support_old, &samp->scale_info, user_data );
  int input_full_size = samp->scale_info.input_full_size;
  stbir_edge edge = samp->edge;
  stbir_real inv_scale = samp->scale_info.inv_scale;

  STBIR_ASSERT( samp->is_gather != 0 );

  if ( samp->is_gather == 1 )
  {
    int in_first_pixel, in_last_pixel;
    stbir_real filter_support = in_pixels_radius;
    stbir_real out_filter_radius = filter_support * scale;

    stbir__output_in_pixel_range( &samp->scale_info, 0, out_filter_radius, filter_support, edge, &in_first_pixel, &in_last_pixel );
    range->n0 = in_first_pixel;
    stbir__output_in_pixel_range( &samp->scale_info, samp->scale_info.output_sub_size - 1, out_filter_radius, filter_support, edge, &in_first_pixel, &in_last_pixel );
    range->n1 = in_last_pixel;
  }
  else if ( samp->is_gather == 2 ) // downsample gather, refine
  {
    stbir_real filter_support = in_pixels_radius / inv_scale;   // in output pixels
    int filter_pixel_margin = samp->filter_pixel_margin;
    int output_sub_size = samp->scale_info.output_sub_size;
    int in_first_pixel, in_last_pixel;

    // get a conservative area of the input range
    stbir__calculate_in_pixel_range( &in_first_pixel, &in_last_pixel, (stbir_real)0.0, (stbir_real)0.0, samp->scale_info.inv_scale, samp->scale_info.pixel_shift, input_full_size, edge );
    range->n0 = in_first_pixel;
    stbir__calculate_in_pixel_range( &in_first_pixel, &in_last_pixel, (stbir_real)output_sub_size, (stbir_real)0.0, samp->scale_info.inv_scale, samp->scale_info.pixel_shift, input_full_size, edge );
    range->n1 = in_last_pixel;

    // walk down through the margin from the start of the area to find the bottom, and up from its end to find the top
    //   (the coefficient generator never goes past input_full_size + filter_pixel_margin - 1)
    range->n0 = stbir__last_contributing_input( range->n0 + 1, -filter_pixel_margin, -1, range->n0, in_pixels_radius, filter_support, &samp->scale_info );
    range->n1 = stbir__last_contributing_input( range->n1 - 1, input_full_size + filter_pixel_margin - 1, 1, range->n1, in_pixels_radius, filter_support, &samp->scale_info );
  }

  // when polyphase, the coefficient generator only calculates the first numerator outputs and then copies them
  //   forward shifted by denominator (exact integer math). the top of the range above was calculated directly at the
  //   far end of the image in floating point, which can round the other way, so widen by one - and when the fraction is
  //   an approximation of the scale, the copies drift from the real positions by up to approximated_drift input pixels.
  if ( ( samp->scale_info.scale_is_rational ) && ( (int)samp->scale_info.scale_numerator < samp->scale_info.output_sub_size ) )
    range->n1 += 1 + samp->scale_info.approximated_drift;

  // with WRAP the range is allowed to hang off either edge (those pixels get decoded from the other end of
  //   the scanline), the other modes fold everything back into the scanline so clamp them to it
  if ( samp->edge != STBIR_EDGE_WRAP )
  {
    // for reflect, the margin taps get folded back into the image, and on tiny inputs we
    //   land beyond, so widen to include them
    if ( samp->edge == STBIR_EDGE_REFLECT )
    {
      if ( range->n0 < 0 )
        range->n1 = stbir__max( range->n1, stbir__min( -range->n0, input_full_size - 1 ) );
      if ( range->n1 >= input_full_size )
        range->n0 = stbir__min( range->n0, stbir__max( 0, ( input_full_size - 1 ) - ( range->n1 - input_full_size ) ) );   // ( 2 x size - n1 - 1, written so it can't overflow at 2^30 )
    }

    // for non-edge-wrap modes, we never read over the edge, so clamp
    if ( range->n0 < 0 )
      range->n0 = 0;
    if ( range->n1 >= input_full_size )
      range->n1 = input_full_size - 1;
  }
}

static void stbir__get_split_info( stbir__per_split_info* split_info, int splits, int output_height, int vertical_pixel_margin, int input_full_height, int is_gather, stbir__sampler const * vertical )
{
  int start_y = 0;
  int remaining = output_height;
  int i;

  for (i = 0; i < splits; i++)
  {
    int rows = remaining / ( splits - i );

    split_info[i].start_output_y = start_y;
    split_info[i].end_output_y = start_y + rows;

    // Don't start a split on a row whose range starts later than a following row's (see stbir__lowest_scanline_needed):
    //   move the rows up to that one into the previous split, so it ends with the unusual row instead of this one
    //   starting on it. Only this split's rows are looked at (the next split's first row is its own problem).
    if ( ( is_gather ) && ( i ) && ( rows > 1 ) )
    {
      int look = vertical_pixel_margin * 3;
      int move;

      if ( look > ( rows - 1 ) )
        look = ( rows - 1 );
      stbir__lowest_scanline_needed( vertical, start_y, look, &move );
      split_info[i-1].end_output_y += move;
      split_info[i].start_output_y += move;
    }

    start_y += rows;
    remaining -= rows;

    // scatter range (updated to minimum as you run it)
    split_info[i].start_input_y = -vertical_pixel_margin;
    split_info[i].end_input_y = input_full_height + vertical_pixel_margin;
  }
}

static void stbir__free_internal_mem( stbir__info *info )
{
  #define STBIR__FREE_AND_CLEAR( ptr ) { if ( ptr ) { void * p = (ptr); (ptr) = 0; STBIR_FREE( p, info->user_data); } }

  if ( info )
  {
  #ifndef STBIR__SEPARATE_ALLOCATIONS
    STBIR__FREE_AND_CLEAR( info->alloced_mem );
  #else
    // a build whose allocation failed part way comes through here too: info and its pointer arrays are zeroed when
    //   allocated, so every pointer is either an allocation or null
    int i,j;

    if ( info->split_info )
    {
      for( i = 0 ; i < info->splits ; i++ )
      {
        stbir__per_split_info * split = info->split_info + i;

        if ( split->ring_buffers )
        {
          for( j = 0 ; j < info->alloc_ring_buffer_num_entries ; j++ )
          {
            float * ring_buffer = split->ring_buffers[j];
            if ( ring_buffer )
            {
              #ifdef STBIR_SIMD8
              if ( info->effective_channels == 3 )
                --ring_buffer; // avx in 3 channel mode needs one float at the start of the buffer
              #endif
              STBIR_FREE( ring_buffer, info->user_data );
            }
          }
          STBIR__FREE_AND_CLEAR( split->ring_buffers );
        }

        if ( split->decode_buffer_alloc )
        {
          #ifdef STBIR_SIMD8
          if ( info->effective_channels == 3 )
            split->decode_buffer_alloc = ( (float*)split->decode_buffer_alloc ) - 1; // avx in 3 channel mode needs one float at the start of the buffer
          #endif
          STBIR__FREE_AND_CLEAR( split->decode_buffer_alloc );
        }
        STBIR__FREE_AND_CLEAR( split->vertical_buffer_alloc );
      }
      STBIR__FREE_AND_CLEAR( info->split_info );
    }
    STBIR__FREE_AND_CLEAR( info->column_slices );
    if ( info->vertical.contributors != info->horizontal.contributors )  // (when shared, the vertical's coefficients point two ints into the horizontal's)
    {
      STBIR__FREE_AND_CLEAR( info->vertical.coefficients );
      STBIR__FREE_AND_CLEAR( info->vertical.contributors );
    }
    STBIR__FREE_AND_CLEAR( info->horizontal.coefficients );
    STBIR__FREE_AND_CLEAR( info->horizontal.contributors );
    STBIR__FREE_AND_CLEAR( info->alloced_mem );
    STBIR_FREE( info, info->user_data );
  #endif
  }

  #undef STBIR__FREE_AND_CLEAR
}

static int stbir__get_max_split( int splits, int height )
{
  int i;
  int max = 0;

  for( i = 0 ; i < splits ; i++ )
  {
    int each = height / ( splits - i );
    if ( each > max )
      max = each;
    height -= each;
  }
  return max;
}

static stbir__horizontal_gather_channels_func ** stbir__horizontal_gather_n_coeffs_funcs[8] =
{
  0, stbir__horizontal_gather_1_channels_with_n_coeffs_funcs, stbir__horizontal_gather_2_channels_with_n_coeffs_funcs, stbir__horizontal_gather_3_channels_with_n_coeffs_funcs, stbir__horizontal_gather_4_channels_with_n_coeffs_funcs, 0,0, stbir__horizontal_gather_7_channels_with_n_coeffs_funcs
};

static stbir__horizontal_gather_channels_func ** stbir__horizontal_gather_channels_funcs[8] =
{
  0, stbir__horizontal_gather_1_channels_funcs, stbir__horizontal_gather_2_channels_funcs, stbir__horizontal_gather_3_channels_funcs, stbir__horizontal_gather_4_channels_funcs, 0,0, stbir__horizontal_gather_7_channels_funcs
};

// the polyphase period 1 kernels, [channels][coefficient count] (see the horizontal kernel section)
static stbir__horizontal_gather_channels_func ** stbir__horizontal_gather_period1_funcs[8] =
{
  0, stbir__horizontal_gather_1_channels_period1_funcs, stbir__horizontal_gather_2_channels_period1_funcs, stbir__horizontal_gather_3_channels_period1_funcs, stbir__horizontal_gather_4_channels_period1_funcs, 0,0, stbir__horizontal_gather_7_channels_period1_funcs
};

// debug overrides (the tests set these; they are statics, so per translation unit): 0 = off
static int stbir__force_vertical_first = 0;    // 1 = horizontal first, 2 = vertical first (dotimings, refcheck -vfirst)
static int stbir__chose_vertical_first = 0;    // (read only) what the cost model picked for the last build
static int stbir__split_mode = 0;              // 1 = row bands only, 2 = column groups only (refcheck_threads -row_split / -colsplit)
static int stbir__split_force_cols = 0;        // force this many column groups, the bands are splits / this (refcheck_threads -grid)
static int stbir__column_slices = 0;           // force this many column slices instead of the L2 fit (refcheck_time cfg N:)
static int stbir__4K_vertical_dest_shift = 0;  // bytes added to the vertical gather destination's page offset (refcheck_time -vshift)

// Vertical first or horizontal first? It matters most when the two axes scale very differently (2x wider and half the
//   height: the vertical first order is around 3x faster), and 20-40% in ordinary cases. The decision is a kernel cost
//   model: both orders move the same decoded pixels, so the question is only how much each kernel charges per byte it
//   streams and the multiplies each direction does (vertical is more SIMDable than horizontal). We calculate the costs as:
//
//     vertical first:    (taps_v + 1) / s_h * rv  +  (1/s_h + 1) * rh      (vertical over input-width rows, horizontal over output rows)
//     horizontal first:  (1/s_v) * (1/s_h + 1) * rh  +  (taps_v + 1) * rv  (horizontal over every input row, vertical over output-width rows)
//   (1/s = input/output per axis, with WRAP's margin pixels and scanlines added to the input)
//
// rh and rv are the scale factors for the horizontal and vertical costs. We then measure a bunch of horizontal
//   first and vertical first resizes on a bunch of machines to train small tables to give us a relative speed of
//   each direction.

// horizontal tap count -> table column: 1, 2, 3, 4, 5-6, 7-8, 9-12, 13-17, 18+
static int stbir__horizontal_cost_bucket( int taps )
{
  if ( taps <= 4 ) return ( taps < 1 ) ? 0 : taps - 1;
  if ( taps <= 6 ) return 4;
  if ( taps <= 8 ) return 5;
  if ( taps <= 12 ) return 6;
  if ( taps <= 17 ) return 7;
  return 8;
}

// [1,2,3,4,7 channels][bucket]: horizontal kernel cost per byte streamed, relative to the vertical kernel's
static float stbir__horizontal_cost[5][9] =
{
  { 12.472f, 6.718f, 6.656f, 8.008f, 5.957f, 6.068f, 6.474f, 7.646f, 10.184f },
  {  8.067f, 4.192f, 5.091f, 4.906f, 4.816f, 5.584f, 5.741f, 6.971f, 6.182f },
  {  5.495f, 3.718f, 3.206f, 7.861f, 4.816f, 5.584f, 5.741f, 5.584f, 6.068f },
  {  3.651f, 2.135f, 2.791f, 3.583f, 2.950f, 3.583f, 4.154f, 4.192f, 4.861f },
  {  2.434f, 2.001f, 2.521f, 3.583f, 2.817f, 3.236f, 3.550f, 3.822f, 4.271f },
};

// the same when the horizontal is an integer downsample (the polyphase period 1 kernels)
static float stbir__horizontal_cost_period1[5][9] =
{
  { 12.406f, 6.718f, 6.534f, 5.186f, 5.114f, 5.138f, 6.656f, 5.431f, 10.374f },
  {  8.001f, 4.154f, 3.583f, 4.003f, 3.276f, 4.473f, 3.683f, 4.231f, 6.013f },
  {  5.024f, 3.033f, 3.583f, 3.683f, 3.268f, 4.432f, 3.650f, 4.231f, 5.283f },
  {  2.976f, 2.076f, 2.665f, 2.452f, 2.354f, 2.766f, 2.715f, 3.327f, 4.684f },
  {  2.184f, 1.928f, 2.342f, 1.983f, 2.233f, 3.005f, 2.715f, 3.421f, 3.858f },
};

// a x b as a signed 128 bit value, for the exact cost comparison below
static void stbir__mul_64x64( stbir_int64 a, stbir_int64 b, stbir_int64 * hi, stbir_uint64 * lo )
{
  stbir_uint64 ua = (stbir_uint64) a, ub = (stbir_uint64) b;
  stbir_uint64 a0 = ua & 0xffffffffu, a1 = ua >> 32, b0 = ub & 0xffffffffu, b1 = ub >> 32;
  stbir_uint64 p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
  stbir_uint64 mid = ( p00 >> 32 ) + ( p01 & 0xffffffffu ) + ( p10 & 0xffffffffu );
  stbir_uint64 uhi = p11 + ( p01 >> 32 ) + ( p10 >> 32 ) + ( mid >> 32 );
  *lo = ( mid << 32 ) | ( p00 & 0xffffffffu );
  if ( a < 0 ) uhi -= ub;   // the unsigned product of the two's complements, corrected to the signed one
  if ( b < 0 ) uhi -= ua;
  *hi = (stbir_int64) uhi;
}

// a x b <= c x d, exactly
static int stbir__products_le( stbir_int64 a, stbir_int64 b, stbir_int64 c, stbir_int64 d )
{
  stbir_int64 lhi, rhi;
  stbir_uint64 llo, rlo;
  stbir__mul_64x64( a, b, &lhi, &llo );
  stbir__mul_64x64( c, d, &rhi, &rlo );
  return ( lhi < rhi ) || ( ( lhi == rhi ) && ( llo <= rlo ) );
}

// WRAP is the one edge mode that changes the amount of work: the decoded rows carry the filter's margin pixels on both
//   ends and the vertical runs over margin scanlines, so the input is effectively taps wider / taller.
static int stbir__should_do_vertical_first( int channel_index, int horizontal_filter_pixel_width, int horizontal_input_size, int horizontal_output_size, stbir_edge horizontal_edge, int horizontal_is_period1,
                                            int vertical_filter_pixel_width, int vertical_input_size, int vertical_output_size, stbir_edge vertical_edge )
{
  stbir_int64 horizontal_input, vertical_input, horizontal_output, vertical_output, vertical_taps, cost_ratio;
  int bucket, vertical_first;

  // the horizontal kernel's cost relative to the vertical's, in 1/1000ths
  bucket = stbir__horizontal_cost_bucket( horizontal_filter_pixel_width );
  cost_ratio = (stbir_int64) ( ( horizontal_is_period1 ? stbir__horizontal_cost_period1 : stbir__horizontal_cost )[ channel_index ][ bucket ] * 1000.0f + 0.5f );

  // the inputs with their wrap margins
  horizontal_input = (stbir_int64) horizontal_input_size + ( ( horizontal_edge == STBIR_EDGE_WRAP ) ? horizontal_filter_pixel_width : 0 );
  vertical_input = (stbir_int64) vertical_input_size + ( ( vertical_edge == STBIR_EDGE_WRAP ) ? vertical_filter_pixel_width : 0 );
  horizontal_output = horizontal_output_size;
  vertical_output = vertical_output_size;
  vertical_taps = (stbir_int64) vertical_filter_pixel_width + 1;

  vertical_first = stbir__products_le( vertical_taps * vertical_output, ( horizontal_input - horizontal_output ) * 1000,
                                       cost_ratio * ( horizontal_input + horizontal_output ), vertical_input - vertical_output );

  // the test hook: report the choice, and let the tests force it
  stbir__chose_vertical_first = vertical_first;
  if ( stbir__force_vertical_first )
    vertical_first = ( stbir__force_vertical_first == 2 ) ? 1 : 0;

  return vertical_first;
}

// layout lookups - must match stbir_internal_pixel_layout
static unsigned char stbir__pixel_channels[] = {
  1,2,3,3,4,   // 1ch, 2ch, rgb, bgr, 4ch
  4,4,4,4,2,2, // RGBA,BGRA,ARGB,ABGR,RA,AR
  4,4,4,4,2,2, // RGBA_PM,BGRA_PM,ARGB_PM,ABGR_PM,RA_PM,AR_PM
};

// the internal pixel layout enums are in a different order, so we can easily do range comparisons of types
//   the public pixel layout is ordered in a way that if you cast num_channels (1-4) to the enum, you get something sensible
static stbir_internal_pixel_layout stbir__pixel_layout_convert_public_to_internal[] = {
  STBIRI_BGR, STBIRI_1CHANNEL, STBIRI_2CHANNEL, STBIRI_RGB, STBIRI_RGBA,
  STBIRI_4CHANNEL, STBIRI_BGRA, STBIRI_ARGB, STBIRI_ABGR, STBIRI_RA, STBIRI_AR,
  STBIRI_RGBA_PM, STBIRI_BGRA_PM, STBIRI_ARGB_PM, STBIRI_ABGR_PM, STBIRI_RA_PM, STBIRI_AR_PM,
};

// We need a good guess at the L2 size for the column sizing (CACHE SIZE in the docs). We get this size, in order of preference as:
//     - stbir_l2_cache_size_override, if you set it to something non-zero before resizing (static, starts at 0, bytes)
//     - the OS, if you define one of these before including this header (we don't include the headers for you):
//         STBIR_ASK_WINDOWS_FOR_L2_SIZE  - needs <windows.h> and <malloc.h>  (GetLogicalProcessorInformationEx into an _alloca buffer)
//         STBIR_ASK_LINUX_FOR_L2_SIZE    - needs <unistd.h>       (sysconf _SC_LEVEL2_CACHE_SIZE)
//         STBIR_ASK_MACOS_FOR_L2_SIZE    - needs <sys/sysctl.h>   (sysctlbyname hw.l2cachesize)
//     - cpuid on x86/x64 (leaf 0x80000006, the L2 size in KB - Intel and AMD both fill it in)
//     - 256 KB if none of that applies or answers
//   An L2 of 1 MB or more is clamped to 512 KB to keep total resize internal memory use down.

static int stbir_l2_cache_size_override = 0;   // bytes, 0 = detect

#if ( defined( _M_X64 ) || defined( _M_IX86 ) || defined( __x86_64__ ) || defined( __i386__ ) ) && !defined( _M_ARM64EC )
  #define STBIR__L2_FROM_CPUID
  #ifdef _MSC_VER
    #include <intrin.h>
  #else
    #include <cpuid.h>
  #endif
#endif

// (asked on every build - cpuid is a few hundred cycles, and no cache means nothing shared between threads building at once)
static int stbir__l2_cache_size( void )
{
  int bytes = 0;

  if ( stbir_l2_cache_size_override > 0 )
    return stbir_l2_cache_size_override;

  #if defined( STBIR_ASK_WINDOWS_FOR_L2_SIZE )
  {
    // the level 2 data or unified cache of the first processor group that reports one. the list is a few hundred
    //   bytes per cache level per core, so it goes on the stack (a 64 core machine's is ~30 KB; past 256 KB fall through)
    DWORD len = 0;
    GetLogicalProcessorInformationEx( RelationCache, 0, &len );
    if ( ( len ) && ( len <= 256 * 1024 ) )
    {
      char * buf = (char*) _alloca( len );
      if ( GetLogicalProcessorInformationEx( RelationCache, (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) buf, &len ) )
      {
        DWORD ofs = 0;
        while ( ofs < len )
        {
          SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX * info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) ( buf + ofs );
          if ( ( info->Relationship == RelationCache ) && ( info->Cache.Level == 2 ) && ( ( info->Cache.Type == CacheData ) || ( info->Cache.Type == CacheUnified ) ) )
          {
            bytes = (int) info->Cache.CacheSize;
            break;
          }
          ofs += info->Size;
        }
      }
    }
  }
  #elif defined( STBIR_ASK_LINUX_FOR_L2_SIZE )
  {
    long v = sysconf( _SC_LEVEL2_CACHE_SIZE );
    if ( v > 0 )
      bytes = (int) v;
  }
  #elif defined( STBIR_ASK_MACOS_FOR_L2_SIZE )
  {
    stbir_uint64 v = 0;
    size_t vlen = sizeof( v );
    if ( ( sysctlbyname( "hw.l2cachesize", &v, &vlen, 0, 0 ) == 0 ) && ( v > 0 ) )
      bytes = (int) v;
  }
  #endif

  #if defined( STBIR__L2_FROM_CPUID )
  if ( bytes <= 0 )   // no OS query, or it didn't answer
  {
    unsigned int regs[4] = { 0, 0, 0, 0 };
    #ifdef _MSC_VER
    __cpuid( (int*) regs, 0x80000000 );
    if ( regs[0] >= 0x80000006 )
    {
      __cpuid( (int*) regs, 0x80000006 );
      bytes = (int) ( regs[2] >> 16 ) * 1024;   // ECX[31:16] = L2 size in KB
    }
    #else
    if ( __get_cpuid_max( 0x80000000, 0 ) >= 0x80000006 )
    {
      __get_cpuid( 0x80000006, &regs[0], &regs[1], &regs[2], &regs[3] );
      bytes = (int) ( regs[2] >> 16 ) * 1024;
    }
    #endif
  }
  #endif

  if ( bytes <= 0 )
    bytes = 256 * 1024;
  else if ( bytes >= 1024 * 1024 )
    bytes = 512 * 1024;

  return bytes;
}

// The splits (what a caller threads over) are a grid of split_cols column groups x split_rows row bands. Any grid gives
//   the same pixels and much the same memory (a split holds one slice's buffers either way), so stbir__choose_split_grid
//   picks the fastest by estimate, for every count from the request down to one split:
//   - a row band re-decodes the vertical filter's taps rows before its first output row, so short bands cost more.
//   - column groups are free while they are the slices a single thread would run anyway (the L2 fit); each extra slice
//     costs stbir_split_slice_row_cost columns of work per row. A group streams at least stbir_split_min_group_bytes
//     of each input row; below stbir_split_group_full_bytes it pays stbir_split_group_cost_pct per (full / bytes - 1)
//     per 8 threads, and narrow rows cost (threads / 8)^2 x (64 KB / input row bytes) / stbir_split_stream_cost_div.
//   - a thread costs stbir_thread_start_cost of work to start and join, and the count is capped so that the work
//     (output pixels x taps x channels / 4) is at least stbir_split_min_work x splits^2.
//   - each count is scored by its largest split, and the last threads are dropped while they add less than
//     stbir_split_min_thread_gain_pct of a thread's speed-up each over the best smaller count.
//   So the build may return fewer splits than asked for (which was always allowed); stbir_split_extra_threads adds some back.

// the split estimate's tuning (statics so an app can override them before resizing, like stbir_l2_cache_size_override;
//   work units are output pixels x taps x channels / 4, roughly nanoseconds of resizing on a fast core)
static int stbir_split_min_group_bytes   = 1024;   // the least input row a column group streams (bytes)
static int stbir_split_group_full_bytes  = 4096;   // below this much input row a column group pays the narrow group cost:
static int stbir_split_group_cost_pct    = 30;     //   this percent x ( full / bytes - 1 ) x ( threads / 8 ) of the work
static int stbir_split_stream_cost_div   = 170;    // narrow rows under many threads: ( threads / 8 )^2 x ( 64 KB / input row bytes ) / this, whatever the grid
static int stbir_split_slice_row_cost    = 250;    // input columns' worth of work per row for each slice beyond the L2 fit
static int stbir_thread_start_cost       = 12000;  // starting and joining one thread, in work units (a spinning pool ~1000, a thread per job 50000+)
static int stbir_split_min_thread_gain_pct = 30;   // the last threads must each add this percent of a thread's worth of speed-up, or they are dropped
static int stbir_split_min_work          = 20000;  // the work must be at least this x splits^2
static int stbir_split_extra_threads     = 0;      // use this many more splits than the estimate settles on (never more than asked for)
#define STBIR__SPLIT_MAX_SPLITS 256                // the grid search's table


// slice boundaries are multiples of this many pixels (so the SIMD loops only run their remnants at the end of the row)
#define STBIR__SLICE_STEP 16

// the most slices a row can have (each at least a step wide)
static int stbir__slice_max_count( int output_columns )
{
  int n = output_columns / STBIR__SLICE_STEP;
  return ( n < 1 ) ? 1 : n;
}


// The split grid for a request of *splits: cols x rows, and *splits becomes cols * rows (never more than asked for).
static void stbir__choose_split_grid( int * splits, int * out_cols, int * out_rows, int mode, int output_w, int output_h, int input_w, int input_pixel_bytes, int taps_v, float inv_scale_v, int l2_slices, int effective_channels )
{
  int max_splits = *splits;
  int requested_splits;
  int max_cols = stbir__slice_max_count( output_w );   // a column group needs at least one slice
  int best_cols = 1;
  int best_rows = 1;

  if ( max_splits < 1 )
    max_splits = 1;
  if ( max_splits > STBIR__SPLIT_MAX_SPLITS )
    max_splits = STBIR__SPLIT_MAX_SPLITS;
  requested_splits = max_splits;

  if ( stbir__split_force_cols > 0 )
  {
    // Debug: a forced grid.
    best_cols = ( stbir__split_force_cols < max_cols ) ? stbir__split_force_cols : max_cols;
    if ( best_cols > max_splits )
      best_cols = max_splits;
    best_rows = max_splits / best_cols;
  }
  else if ( mode == 1 )
  {
    // Row bands only.
    best_rows = ( max_splits < output_h ) ? max_splits : output_h;
  }
  else if ( mode == 2 )
  {
    // Column groups only.
    best_cols = ( max_splits < max_cols ) ? max_splits : max_cols;
  }
  else
  {
    // Everything in fixed point: times are in 1/65536ths of the single threaded work.
    stbir_uint64 work = (stbir_uint64)output_w * (stbir_uint64)output_h * (stbir_uint64)taps_v * (stbir_uint64)effective_channels / 4;
    stbir_uint64 band_warmup = (stbir_uint64)( (float)taps_v * 65536.0f / ( ( inv_scale_v > 1.0f ) ? inv_scale_v : 1.0f ) );   // what a band re-decodes before its first row, in output rows' worth of work
    stbir_uint64 row_bytes = (stbir_uint64)input_w * (stbir_uint64)input_pixel_bytes;
    stbir_uint64 estimate_of[ STBIR__SPLIT_MAX_SPLITS + 1 ];
    int cols_of[ STBIR__SPLIT_MAX_SPLITS + 1 ];
    int rows_of[ STBIR__SPLIT_MAX_SPLITS + 1 ];
    int count, cols;

    if ( work < 1 )
      work = 1;
    if ( row_bytes < 1 )
      row_bytes = 1;

    // The thread start/join cost grows with the count: the work must be at least min_work x splits^2.
    while ( ( max_splits > 1 ) && ( work < (stbir_uint64)stbir_split_min_work * (stbir_uint64)max_splits * (stbir_uint64)max_splits ) )
      --max_splits;

    // The best grid for every count up to the request, by estimated time (the largest split's - it sets the pace).
    for (count = 1; count <= requested_splits; count++)
    {
      estimate_of[count] = ~(stbir_uint64)0;
      cols_of[count] = 1;
      rows_of[count] = 1;

      for (cols = 1; cols <= count; cols++)
      {
        int rows, rows_per_band, extra_slices;
        stbir_uint64 group_bytes, narrow_cost, stream_cost, per_row_cost, estimate;

        if ( count % cols )
          continue;
        if ( cols > max_cols )
          break;

        // A column group streams its share of each input row, and never less than min_group_bytes of it.
        group_bytes = row_bytes / (stbir_uint64)cols;
        if ( group_bytes < 1 )
          group_bytes = 1;
        if ( ( cols > 1 ) && ( group_bytes < (stbir_uint64)stbir_split_min_group_bytes ) )
          break;

        // Narrow groups cost group_cost_pct per ( full / bytes - 1 ) per 8 threads.
        narrow_cost = 0;
        if ( ( cols > 1 ) && ( group_bytes < (stbir_uint64)stbir_split_group_full_bytes ) )
          narrow_cost = ( ( (stbir_uint64)stbir_split_group_full_bytes * 65536 ) / group_bytes - 65536 ) * (stbir_uint64)stbir_split_group_cost_pct * (stbir_uint64)count / ( 100 * 8 );

        // Narrow rows under many threads cost the same whatever the grid (every row is a fresh page, latency bound, and
        //   the latency climbs with the thread count): the larger of this and the narrow group cost.
        stream_cost = ( (stbir_uint64)count * (stbir_uint64)count * 65536 * 65536 ) / ( (stbir_uint64)64 * (stbir_uint64)stbir_split_stream_cost_div * row_bytes );
        if ( stream_cost > narrow_cost )
          narrow_cost = stream_cost;

        rows = count / cols;
        if ( rows > output_h )
          continue;

        // The slowest split sets the time: the largest band (rows rounded up) doing the largest group's share of the
        //   slices (the slice count rounded up to a multiple of the groups), against an even share of the work.
        rows_per_band = ( output_h + rows - 1 ) / rows;
        extra_slices = ( ( l2_slices + cols - 1 ) / cols ) * cols - l2_slices;
        per_row_cost = 65536 + band_warmup / (stbir_uint64)rows_per_band + ( (stbir_uint64)extra_slices * (stbir_uint64)stbir_split_slice_row_cost * 65536 ) / (stbir_uint64)input_w + narrow_cost;
        estimate = ( (stbir_uint64)rows_per_band * (stbir_uint64)rows ) * per_row_cost / ( (stbir_uint64)output_h * (stbir_uint64)count );
        estimate += ( (stbir_uint64)count * (stbir_uint64)stbir_thread_start_cost * 65536 ) / work;   // plus the threads' own start/join cost

        if ( estimate < estimate_of[count] )
        {
          estimate_of[count] = estimate;
          cols_of[count] = cols;
          rows_of[count] = rows;
        }
      }
    }

    // From the request down, drop threads while they add less than min_thread_gain_pct of a thread's speed-up each
    //   over the best smaller count (not the next one down: a prime count only has bad grids, which makes the count
    //   above it look good).
    count = max_splits;
    while ( count > 1 )
    {
      stbir_uint64 speedup = ( estimate_of[1] * 256 ) / estimate_of[count];
      stbir_uint64 best_speedup = 0;
      int fewer, best_fewer = 1;

      for (fewer = 1; fewer < count; fewer++)
      {
        stbir_uint64 speedup_fewer = ( estimate_of[1] * 256 ) / estimate_of[fewer];
        if ( speedup_fewer > best_speedup )
        {
          best_speedup = speedup_fewer;
          best_fewer = fewer;
        }
      }

      if ( ( speedup > best_speedup ) && ( ( speedup - best_speedup ) * 100 >= (stbir_uint64)stbir_split_min_thread_gain_pct * 256 * (stbir_uint64)( count - best_fewer ) ) )
        break;

      // The extra threads over the best smaller count weren't worth it: fall back to it and check again.
      count = best_fewer;
    }

    // The app wants more than the estimate settled on.
    if ( stbir_split_extra_threads > 0 )
    {
      count += stbir_split_extra_threads;
      if ( count > requested_splits )
        count = requested_splits;
      // A count no grid fits (more bands than rows): the next one down.
      while ( ( count > 1 ) && ( estimate_of[count] == ~(stbir_uint64)0 ) )
        --count;
    }

    best_cols = cols_of[count];
    best_rows = rows_of[count];
  }

  if ( best_cols > max_cols )
    best_cols = max_cols;
  if ( best_rows > output_h )
    best_rows = output_h;
  if ( best_cols < 1 )
    best_cols = 1;
  if ( best_rows < 1 )
    best_rows = 1;

  *out_cols = best_cols;
  *out_rows = best_rows;
  *splits = best_cols * best_rows;
}

// where slice number slice of num_slices ends
static int stbir__slice_end( int output_columns, int num_slices, int slice )
{
  stbir_uint64 end;

  if ( slice >= num_slices - 1 )
    return output_columns;

  end = ( (stbir_uint64)( slice + 1 ) * (stbir_uint64)output_columns ) / (stbir_uint64)num_slices;
  return ( (int)end / STBIR__SLICE_STEP ) * STBIR__SLICE_STEP;
}

// the widest slice there can be for a count (for sizing the per slice buffers): the average plus a step of rounding
static int stbir__slice_columns_for( int output_columns, int slices )
{
  int columns = ( output_columns + slices - 1 ) / slices + STBIR__SLICE_STEP;
  if ( columns >= output_columns )
    columns = output_columns;
  return columns;
}

// the input pixels a slice decodes for that many columns (see the sizing in stbir__alloc_internal_mem_and_build_samplers)
static int stbir__slice_in_span_for( stbir__sampler const * horizontal, int slice_columns, int conservative_in_span )
{
  int span = (int)( slice_columns * horizontal->scale_info.inv_scale ) + 1 + horizontal->filter_pixel_width + 2 * horizontal->coefficient_width + 4;
  if ( ( slice_columns >= horizontal->scale_info.output_sub_size ) || ( span > conservative_in_span ) )
    span = conservative_in_span;
  return span;
}

// the ring buffer for one slice: its entries are horizontal output rows (slice_columns wide), or decoded input rows
//   (slice_in_span wide) when vertical first
static size_t stbir__ring_bytes_for( stbir__sampler const * horizontal, int slice_columns, int conservative_in_span, int effective_channels, int vertical_first, int ring_entries )
{
  int row_pixels = vertical_first ? stbir__slice_in_span_for( horizontal, slice_columns, conservative_in_span ) : slice_columns;
  return (size_t)ring_entries * ( (size_t)row_pixels * effective_channels * sizeof(float) + sizeof(float)*STBIR_INPUT_CALLBACK_PADDING );
}

// pick the slice count: the fewest slices whose ring buffer fits the L2 budget (allowing an eighth over - a little
//   spill costs less than another slice's re-decode), or the most slices possible (16 columns each) if none fit
static int stbir__choose_column_slices( stbir__sampler const * horizontal, int conservative_in_span, int effective_channels, int vertical_first, int ring_entries )
{
  int output_columns = horizontal->scale_info.output_sub_size;
  size_t budget = (size_t)stbir__l2_cache_size();
  size_t allowed = budget + budget / 8;
  int max_slices = stbir__slice_max_count( output_columns );
  int slices;

  for (slices = 1; slices < max_slices; slices++)
  {
    int slice_columns = stbir__slice_columns_for( output_columns, slices );
    if ( stbir__ring_bytes_for( horizontal, slice_columns, conservative_in_span, effective_channels, vertical_first, ring_entries ) <= allowed )
      return slices;
  }
  return max_slices;
}

// splits the output columns into num_column_slices balanced slices on step boundaries (stbir__slice_end), and works
//   out the input range each slice has to decode: the lowest n0 through the furthest pixel any of its kernels read
//   (stbir__pack_coefficients arranged for those reads to stay within the conservative range).
static void stbir__build_column_slices( stbir__info * info, int slice_columns, int in_span_bound )
{
  stbir__sampler const * horizontal = &info->horizontal;
  int output_columns = horizontal->scale_info.output_sub_size;
  int widest = horizontal->coefficient_width; // the coefficients are packed to widest by now
  int first_column, end_column;
  int slice_index = 0;

  STBIR__UNUSED( in_span_bound );   // both only checked in asserts
  STBIR__UNUSED( slice_columns );

  for (first_column = 0; first_column < output_columns; first_column = end_column, slice_index++)
  {
    stbir__column_slice * slice = info->column_slices + slice_index;
    int in_first = 0x7fffffff;
    int in_last = -0x7fffffff;
    int x, unused;

    end_column = stbir__slice_end( output_columns, info->num_column_slices, slice_index );
    STBIR_ASSERT( ( end_column > first_column ) && ( end_column <= output_columns ) && ( ( end_column - first_column ) <= slice_columns ) );

    // the lowest first pixel through the furthest pixel any of the slice's kernels read
    for (x = first_column; x < end_column; x++)
    {
      stbir__contributors contributor;
      int last_read;

      stbir__output_range( horizontal, x, &contributor.n0, &contributor.n1 );
      last_read = contributor.n0 + stbir__horizontal_read_extent( &contributor, widest ) - 1;
      if ( contributor.n0 < in_first )
        in_first = contributor.n0;
      if ( last_read > in_last )
        in_last = last_read;
    }

    STBIR_ASSERT( in_first >= info->conservative_extents.n0 );
    STBIR_ASSERT( in_last <= info->conservative_extents.n1 );
    STBIR_ASSERT( ( in_last - in_first + 1 ) <= in_span_bound ); // the buffers were sized from in_span_bound in the first allocation pass

    slice->x0 = first_column;
    slice->width = end_column - first_column;
    slice->in_n0 = in_first;
    slice->in_n1 = in_last;
    stbir__output_range( horizontal, first_column, &slice->first_n0, &unused );
  }

  STBIR_ASSERT( slice_index == info->num_column_slices );
}

// split s is row band ( s / split_cols ) of column group ( s % split_cols ). 
static void stbir__setup_split_grid( stbir__info * info, int split_cols, int split_rows )
{
  int r, c, next = 0;

  info->split_cols = split_cols;
  info->split_rows = split_rows;
  stbir__get_split_info( info->split_info, split_rows, info->vertical.scale_info.output_sub_size, info->vertical.filter_pixel_margin, info->vertical.scale_info.input_full_size, info->vertical.is_gather, &info->vertical );
  for( r = split_rows - 1 ; r >= 0 ; r-- )
  {
    stbir__per_split_info band = info->split_info[r];
    for( c = split_cols - 1 ; c >= 0 ; c-- )
    {
      stbir__per_split_info * s = info->split_info + r * split_cols + c;
      s->start_output_y = band.start_output_y;
      s->end_output_y   = band.end_output_y;
      s->start_input_y  = band.start_input_y;
      s->end_input_y    = band.end_input_y;
    }
  }
  for( c = 0 ; c < split_cols ; c++ )
  {
    int count = ( info->num_column_slices - next ) / ( split_cols - c );
    for( r = 0 ; r < split_rows ; r++ )
    {
      info->split_info[ r * split_cols + c ].first_column_slice = next;
      info->split_info[ r * split_cols + c ].num_column_slices = count;
    }
    next += count;
  }
}

// the per split buffers get a page of slack, to be repositioned within it at resize time (stbir__4K_alias_skew_buffers)
#define STBIR__4K_ALIAS_SKEW_SLACK 4096

// What kind of alpha weighting the layouts call for: 0 = none, 1 = weight in, leave premultiplied, 2 = fancy weight
//   in and unweight out, 3 = premultiplied in, unweight out, 4 = simple weight in and unweight out. Fancy weighting
//   carries extra channels through the filters (*effective_channels).
static int stbir__alpha_weighting_type( stbir__sampler const * horizontal, stbir__sampler const * vertical, stbir_internal_pixel_layout input_pixel_layout, stbir_internal_pixel_layout output_pixel_layout, int fast_alpha, int * effective_channels )
{
  int input_has_alpha = ( input_pixel_layout >= STBIRI_RGBA ) && ( input_pixel_layout <= STBIRI_AR );
  int input_premultiplied = ( input_pixel_layout >= STBIRI_RGBA_PM ) && ( input_pixel_layout <= STBIRI_AR_PM );
  int output_has_alpha = ( output_pixel_layout >= STBIRI_RGBA ) && ( output_pixel_layout <= STBIRI_AR );
  int output_premultiplied = ( output_pixel_layout >= STBIRI_RGBA_PM ) && ( output_pixel_layout <= STBIRI_AR_PM );

  int point_sampled = ( horizontal->filter_enum == STBIR_FILTER_POINT_SAMPLE ) && ( vertical->filter_enum == STBIR_FILTER_POINT_SAMPLE );

  if ( input_has_alpha && output_has_alpha )
  {
    static int fancy_alpha_effective_cnts[6] = { 7, 7, 7, 7, 3, 3 };
    if ( point_sampled )   // nothing is blended, so weighting in and unweighting out would be the identity
      return 0;
    if ( fast_alpha )
      return 4;
    *effective_channels = fancy_alpha_effective_cnts[ input_pixel_layout - STBIRI_RGBA ];
    return 2;
  }
  // (the conversions between premultiplied and not still happen when point sampling)
  if ( input_premultiplied && output_has_alpha )
    return 3;
  if ( input_has_alpha && output_premultiplied )
    return 1;
  return 0;
}

// The alpha weight and unweight functions for the weighting type (and the 3 channel color flip, which rides the same path).
static void stbir__set_alpha_functions( stbir__info * info, int alpha_weighting_type, stbir_internal_pixel_layout input_pixel_layout, stbir_internal_pixel_layout output_pixel_layout, float horizontal_scale )
{
  static stbir__alpha_weight_func * fancy_alpha_weights[6]  =    { stbir__fancy_alpha_weight_4ch,   stbir__fancy_alpha_weight_4ch,   stbir__fancy_alpha_weight_4ch,   stbir__fancy_alpha_weight_4ch,   stbir__fancy_alpha_weight_2ch,   stbir__fancy_alpha_weight_2ch };
  static stbir__alpha_unweight_func * fancy_alpha_unweights[6] = { stbir__fancy_alpha_unweight_4ch, stbir__fancy_alpha_unweight_4ch, stbir__fancy_alpha_unweight_4ch, stbir__fancy_alpha_unweight_4ch, stbir__fancy_alpha_unweight_2ch, stbir__fancy_alpha_unweight_2ch };
  static stbir__alpha_weight_func * simple_alpha_weights[6] = { stbir__simple_alpha_weight_4ch, stbir__simple_alpha_weight_4ch, stbir__simple_alpha_weight_4ch, stbir__simple_alpha_weight_4ch, stbir__simple_alpha_weight_2ch, stbir__simple_alpha_weight_2ch };
  static stbir__alpha_unweight_func * simple_alpha_unweights[6] = { stbir__simple_alpha_unweight_4ch, stbir__simple_alpha_unweight_4ch, stbir__simple_alpha_unweight_4ch, stbir__simple_alpha_unweight_4ch, stbir__simple_alpha_unweight_2ch, stbir__simple_alpha_unweight_2ch };

  info->alpha_weight = 0;
  info->alpha_unweight = 0;

  if ( alpha_weighting_type == 2 )
  {
    // high quality alpha multiplying on the way in, dividing on the way out
    info->alpha_weight = fancy_alpha_weights[ input_pixel_layout - STBIRI_RGBA ];
    info->alpha_unweight = fancy_alpha_unweights[ output_pixel_layout - STBIRI_RGBA ];
  }
  else if ( alpha_weighting_type == 4 )
  {
    // fast alpha multiplying on the way in, dividing on the way out
    info->alpha_weight = simple_alpha_weights[ input_pixel_layout - STBIRI_RGBA ];
    info->alpha_unweight = simple_alpha_unweights[ output_pixel_layout - STBIRI_RGBA ];
  }
  else if ( alpha_weighting_type == 1 )
  {
    // fast alpha on the way in, leave in premultiplied form on way out
    info->alpha_weight = simple_alpha_weights[ input_pixel_layout - STBIRI_RGBA ];
  }
  else if ( alpha_weighting_type == 3 )
  {
    // incoming is premultiplied, fast alpha dividing on the way out - non-premultiplied output
    info->alpha_unweight = simple_alpha_unweights[ output_pixel_layout - STBIRI_RGBA ];
  }

  // handle 3-chan color flipping, using the alpha weight path
  if ( ( ( input_pixel_layout == STBIRI_RGB ) && ( output_pixel_layout == STBIRI_BGR ) ) ||
       ( ( input_pixel_layout == STBIRI_BGR ) && ( output_pixel_layout == STBIRI_RGB ) ) )
  {
    // do the flipping on the smaller of the two ends
    if ( horizontal_scale < 1.0f )
      info->alpha_unweight = stbir__simple_flip_3ch;
    else
      info->alpha_weight = stbir__simple_flip_3ch;
  }
}

// Shared coefficients require identical geometry; inactive rational and exact-position fields may be uninitialized.
static int stbir__scale_info_matches( stbir__scale_info const * horizontal, stbir__scale_info const * vertical )
{
  if ( ( horizontal->input_full_size != vertical->input_full_size ) || ( horizontal->output_sub_size != vertical->output_sub_size ) ||
       ( horizontal->scale != vertical->scale ) || ( horizontal->inv_scale != vertical->inv_scale ) ||   // (the positions come from the reals)
       ( horizontal->pixel_shift != vertical->pixel_shift ) || ( horizontal->scale_is_rational != vertical->scale_is_rational ) ||
       ( horizontal->positions_exact != vertical->positions_exact ) )
  {
    return 0;
  }

  if ( horizontal->scale_is_rational &&
       ( ( horizontal->scale_numerator != vertical->scale_numerator ) || ( horizontal->scale_denominator != vertical->scale_denominator ) ) )
  {
    return 0;
  }

  if ( horizontal->positions_exact &&
       ( ( horizontal->shift_pixels != vertical->shift_pixels ) || ( horizontal->tap_scale_in != vertical->tap_scale_in ) ||
         ( horizontal->tap_scale_out != vertical->tap_scale_out ) ) )
  {
    return 0;
  }

  return 1;
}

// Are the two samplers identical? (happens a lot with mipmap generation) Then the vertical can share the horizontal's tables.
static int stbir__samplers_match( stbir__sampler const * horizontal, stbir__sampler const * vertical )
{
  return ( horizontal->filter_kernel_new == vertical->filter_kernel_new ) && ( horizontal->filter_kernel_old == vertical->filter_kernel_old ) && ( horizontal->filter_support_new == vertical->filter_support_new ) && ( horizontal->filter_radius == vertical->filter_radius ) && ( horizontal->filter_support_old == vertical->filter_support_old ) &&
         ( horizontal->edge == vertical->edge ) && ( horizontal->is_gather == vertical->is_gather ) &&
         stbir__scale_info_matches( &horizontal->scale_info, &vertical->scale_info );
}

// The horizontal kernels for this channel count and coefficient width.
static void stbir__select_horizontal_kernels( stbir__info * info, stbir__sampler const * horizontal, int effective_channels )
{
  int widest = horizontal->extent_info.widest;

  // start with defaulting to the n_coeffs functions (specialized on channels and remnant leftover)
  info->horizontal_gather_channels = stbir__horizontal_gather_n_coeffs_funcs[ effective_channels ][ widest & 3 ];

  // but if the number of coeffs <= 17, use another set of special cases. This is by far the common case
  if ( widest <= 17 )
    info->horizontal_gather_channels = stbir__horizontal_gather_channels_funcs[ effective_channels ][ widest - 1 ];

  // integer downsamples (polyphase period 1) use one coefficient set for the whole interior, so the kernel can keep the
  //   coefficients in registers for the whole run - where there is one for this channel count and width (null otherwise)
  info->horizontal_gather_period1 = 0;
  if ( ( horizontal->shared_period == 1 ) && ( widest <= 17 ) )
    info->horizontal_gather_period1 = stbir__horizontal_gather_period1_funcs[ effective_channels ][ widest ];
}

// The per split buffers are sized for one column slice (the slices run one after another and reuse them).
typedef struct stbir__slice_buffer_sizes
{
  int num_column_slices;            // a multiple of the column groups, so they get equal shares
  int slice_columns;                // the widest slice
  int slice_in_span;                // the most input pixels a slice decodes
  size_t decode_buffer_size;
  size_t ring_buffer_length_bytes;  // one ring entry
  size_t ring_buffer_size;
  size_t vertical_buffer_size;
} stbir__slice_buffer_sizes;

static void stbir__size_slice_buffers( stbir__slice_buffer_sizes * sizes, stbir__sampler const * horizontal, int conservative_in_span, int l2_slices, int split_cols, int effective_channels, int vertical_first, int ring_buffer_num_entries )
{
  int output_columns = horizontal->scale_info.output_sub_size;
  int max_slices = stbir__slice_max_count( output_columns );

  // the slice count made a multiple of the column groups (a slice is at least a step wide, so some groups may get none)
  sizes->num_column_slices = ( ( l2_slices + split_cols - 1 ) / split_cols ) * split_cols;
  if ( sizes->num_column_slices > max_slices )
    sizes->num_column_slices = max_slices;
  sizes->slice_columns = stbir__slice_columns_for( output_columns, sizes->num_column_slices );

  // the input pixels a slice decodes: its columns' geometric span, plus the filter width, plus the room
  //   stbir__pack_coefficients can move a contributor back by and the kernel can read past it (a coefficient
  //   width each). the exact ranges come from the packed contributors in the second pass, which asserts
  //   they fit in this. a single slice is exactly the conservative range.
  sizes->slice_in_span = stbir__slice_in_span_for( horizontal, sizes->slice_columns, conservative_in_span );

  // sometimes read one float off in some of the unrolled loops (with a weight of zero coeff, so it doesn't have an effect)
  //   we use a few extra floats instead of just 1, so that input callback buffer can overlap with the decode buffer without
  //   the conversion routines overwriting the callback input data.
  sizes->decode_buffer_size = (size_t)sizes->slice_in_span * effective_channels * sizeof(float) + sizeof(float)*STBIR_INPUT_CALLBACK_PADDING; // extra floats for input callback stagger

#if defined( STBIR__SEPARATE_ALLOCATIONS ) && defined(STBIR_SIMD8)
  if ( effective_channels == 3 )
    sizes->decode_buffer_size += sizeof(float); // avx in 3 channel mode needs one float at the start of the buffer (only with separate allocations)
#endif

  sizes->ring_buffer_length_bytes = (size_t)sizes->slice_columns * (size_t)effective_channels * sizeof(float) + sizeof(float)*STBIR_INPUT_CALLBACK_PADDING; // extra floats for padding

  // if we do vertical first, the ring buffer holds a whole decoded line
  if ( vertical_first )
    sizes->ring_buffer_length_bytes = ( sizes->decode_buffer_size + 15 ) & ~15;

  // keep the ring buffer's entries off a page multiple apart: the vertical gather reads taps rows at the same column,
  //   and at a 4K multiple stride they all fall in the same cache sets
  if ( ( sizes->ring_buffer_length_bytes & 4095 ) == 0 )
    sizes->ring_buffer_length_bytes += 64*3;

  sizes->ring_buffer_size = (size_t)ring_buffer_num_entries * sizes->ring_buffer_length_bytes;

  // The vertical buffer holds one horizontally resampled row: gathering, the output row (the vertical gather's result
  //   horizontal first, the horizontal's result vertical first); scattering, the input row about to be scattered
  //   (horizontal first) or the output row at eviction (vertical first). The accumulation itself is in the ring.
  sizes->vertical_buffer_size = (size_t)sizes->slice_columns * (size_t)effective_channels * sizeof(float) + sizeof(float);  // extra float for padding
}

static stbir__info * stbir__alloc_internal_mem_and_build_samplers( stbir__sampler * horizontal, stbir__sampler * vertical, stbir__contributors * conservative, stbir_pixel_layout input_pixel_layout_public, stbir_pixel_layout output_pixel_layout_public, int splits, int new_x, int new_y, int fast_alpha, int input_pixel_bytes, void * user_data STBIR_ONLY_PROFILE_BUILD_GET_INFO )
{
  static char stbir_channel_count_index[8]={ 9,0,1,2, 3,9,9,4 };

  stbir__info * info = 0;
  void * alloced = 0;
  size_t alloced_total = 0;
  int vertical_first;
  int alloc_ring_buffer_num_entries;
  int alpha_weighting_type;
  int conservative_split_output_size;
  int conservative_in_span, l2_slices, split_cols, split_rows;
  int copy_horizontal;
  stbir__slice_buffer_sizes sizes;
  stbir_internal_pixel_layout input_pixel_layout = stbir__pixel_layout_convert_public_to_internal[ input_pixel_layout_public ];
  stbir_internal_pixel_layout output_pixel_layout = stbir__pixel_layout_convert_public_to_internal[ output_pixel_layout_public ];
  int channels = stbir__pixel_channels[ input_pixel_layout ];
  int effective_channels = channels;

  // channel in and out count must match currently
  if ( channels != stbir__pixel_channels[ output_pixel_layout ] )
    return 0;

  alpha_weighting_type = stbir__alpha_weighting_type( horizontal, vertical, input_pixel_layout, output_pixel_layout, fast_alpha, &effective_channels );

  // do the horizontal and vertical store their polyphase coefficient sets once? (sizes the tables, so before anything
  //   is allocated). the vertical only when gathering (the scatter tables are per input scanline). the vertical kernels
  //   read exactly n0..n1 of a set, so its epilogue needs no clip zone.
  stbir__plan_polyphase_sharing( horizontal, 1, user_data );
  if ( vertical->is_gather )
    stbir__plan_polyphase_sharing( vertical, 0, user_data );

  vertical_first = stbir__should_do_vertical_first( (int)stbir_channel_count_index[ effective_channels ],
                                                    horizontal->filter_pixel_width, horizontal->scale_info.input_full_size, horizontal->scale_info.output_sub_size, horizontal->edge, horizontal->shared_period == 1,
                                                    vertical->filter_pixel_width, vertical->scale_info.input_full_size, vertical->scale_info.output_sub_size, vertical->edge );

  // ring rows: gathering, an output's taps rows (plus one for float rounding); scattering, the output rows an input
  //   row touches - 2 x the filter's support in output pixels, plus one, plus one for rounding
  if ( vertical->is_gather )
    alloc_ring_buffer_num_entries = vertical->filter_pixel_width + 1;
  else
    alloc_ring_buffer_num_entries = stbir__ceil_to_int( (stbir_real)2.0 * stbir__filter_radius_in_output_pixels( vertical->filter_radius, vertical->filter_support_new, vertical->filter_support_old, &vertical->scale_info, user_data ) ) + 2;

  // how many vertical slices the single threaded resize would run (the setting, or the L2 fit), then the split grid -
  //   which can lower the split count
  conservative_in_span = conservative->n1 - conservative->n0 + 1;
  l2_slices = stbir__column_slices;
  if ( l2_slices < 1 )
    l2_slices = stbir__choose_column_slices( horizontal, conservative_in_span, effective_channels, vertical_first, alloc_ring_buffer_num_entries );
  stbir__choose_split_grid( &splits, &split_cols, &split_rows, stbir__split_mode, horizontal->scale_info.output_sub_size, vertical->scale_info.output_sub_size, conservative_in_span, input_pixel_bytes, vertical->filter_pixel_width, (float) vertical->scale_info.inv_scale, l2_slices, effective_channels );
  conservative_split_output_size = stbir__get_max_split( split_rows, vertical->scale_info.output_sub_size );

  // we never need more ring buffer entries than the scanlines we're outputting when in scatter mode
  if ( ( !vertical->is_gather ) && ( alloc_ring_buffer_num_entries > conservative_split_output_size ) )
    alloc_ring_buffer_num_entries = conservative_split_output_size;

  stbir__size_slice_buffers( &sizes, horizontal, conservative_in_span, l2_slices, split_cols, effective_channels, vertical_first, alloc_ring_buffer_num_entries );

  // are the two filters identical? then the vertical shares the horizontal's tables instead of generating its own
  copy_horizontal = stbir__samplers_match( horizontal, vertical );

  // we make two passes through this loop, 1st to add everything up, 2nd to allocate and init
  for(;;)
  {
    int i;
    void * advance_mem = alloced;

#ifdef STBIR__SEPARATE_ALLOCATIONS
    // a failed allocation frees whatever got allocated: the pointers the cleanup frees are cleared as their holders are
    //   allocated, so every one is an allocation or null (if info itself failed, only the placeholder block exists)
    #define STBIR__NEXT_PTR( ptr, size, ntype ) if ( alloced ) { void * p = STBIR_MALLOC( size, user_data); if ( p == 0 ) { if ( info ) stbir__free_internal_mem( info ); else STBIR_FREE( alloced, user_data ); return 0; } (ptr) = (ntype*)p; }
#else
    #define STBIR__NEXT_PTR( ptr, size, ntype ) advance_mem = (void*) ( ( ((size_t)advance_mem) + 15 ) & ~15 ); if ( alloced ) ptr = (ntype*)advance_mem; advance_mem = (char*)(((size_t)advance_mem) + (size));
#endif

    STBIR__NEXT_PTR( info, sizeof( stbir__info ), stbir__info );
    if ( info )
    {
      info->alloced_mem = alloced;
      info->user_data = user_data;
      info->splits = splits;
      info->alloc_ring_buffer_num_entries = (int)alloc_ring_buffer_num_entries;
      info->effective_channels = effective_channels;
      info->split_info = 0;
      info->column_slices = 0;
      info->horizontal.contributors = 0;
      info->horizontal.coefficients = 0;
      info->vertical.contributors = 0;
      info->vertical.coefficients = 0;
    }

    STBIR__NEXT_PTR( info->split_info, sizeof( stbir__per_split_info ) * splits, stbir__per_split_info );
    #ifdef STBIR__SEPARATE_ALLOCATIONS
    if ( info )
    {
      for( i = 0 ; i < splits ; i++ )
      {
        info->split_info[i].ring_buffers = 0;
        info->split_info[i].decode_buffer_alloc = 0;
        info->split_info[i].vertical_buffer_alloc = 0;
      }
    }
    #endif

    if ( info )
    {
      // initialize info fields
      info->alloced_total = alloced_total;
      info->channels = channels;
      info->offset_x = new_x;
      info->offset_y = new_y;
      info->ring_buffer_num_entries = 0;
      info->ring_buffer_length_bytes = sizes.ring_buffer_length_bytes;
      info->vertical_first = vertical_first;

      info->input_pixel_layout_internal = input_pixel_layout;
      info->output_pixel_layout_internal = output_pixel_layout;

      stbir__set_alpha_functions( info, alpha_weighting_type, input_pixel_layout, output_pixel_layout, (float) horizontal->scale_info.scale );
    }

    // get all the per-split buffers (each with a page of slack, see stbir__4K_alias_skew_buffers)
    for( i = 0 ; i < splits ; i++ )
    {
      STBIR__NEXT_PTR( info->split_info[i].decode_buffer, sizes.decode_buffer_size + STBIR__4K_ALIAS_SKEW_SLACK, float );

#ifdef STBIR__SEPARATE_ALLOCATIONS

      #ifdef STBIR_SIMD8
      if ( ( info ) && ( effective_channels == 3 ) )
        ++info->split_info[i].decode_buffer; // avx in 3 channel mode needs one float at the start of the buffer
      #endif
      if ( info )
        info->split_info[i].decode_buffer_alloc = info->split_info[i].decode_buffer;

      STBIR__NEXT_PTR( info->split_info[i].ring_buffers, alloc_ring_buffer_num_entries * sizeof(float*), float* );
      {
        int j;
        if ( info )
        {
          for( j = 0 ; j < alloc_ring_buffer_num_entries ; j++ )
            info->split_info[i].ring_buffers[j] = 0;
        }
        for( j = 0 ; j < alloc_ring_buffer_num_entries ; j++ )
        {
          STBIR__NEXT_PTR( info->split_info[i].ring_buffers[j], sizes.ring_buffer_length_bytes, float );
          #ifdef STBIR_SIMD8
          if ( ( info ) && ( effective_channels == 3 ) )
            ++info->split_info[i].ring_buffers[j]; // avx in 3 channel mode needs one float at the start of the buffer
          #endif
        }
      }
#else
      STBIR__NEXT_PTR( info->split_info[i].ring_buffer, sizes.ring_buffer_size + STBIR__4K_ALIAS_SKEW_SLACK, float );
#endif
      STBIR__NEXT_PTR( info->split_info[i].vertical_buffer, sizes.vertical_buffer_size + STBIR__4K_ALIAS_SKEW_SLACK, float );

      if ( info )
      {
        #ifndef STBIR__SEPARATE_ALLOCATIONS
        info->split_info[i].decode_buffer_alloc = info->split_info[i].decode_buffer;
        info->split_info[i].ring_buffer_alloc = info->split_info[i].ring_buffer;
        #endif
        info->split_info[i].vertical_buffer_alloc = info->split_info[i].vertical_buffer;
      }
    }

    // the tables are allocated into info's samplers, and the samplers are built in the locals (copied into info below)
    STBIR__NEXT_PTR( info->horizontal.contributors, horizontal->contributors_size, stbir__contributors );
    STBIR__NEXT_PTR( info->horizontal.coefficients, horizontal->coefficients_size, float );

    if ( !copy_horizontal )   // (sharing: the vertical reads the horizontal's tables, see below)
    {
      STBIR__NEXT_PTR( info->vertical.contributors, vertical->contributors_size, stbir__contributors );
      STBIR__NEXT_PTR( info->vertical.coefficients, vertical->coefficients_size, float );
    }

    if ( info )
    {
      horizontal->contributors = info->horizontal.contributors;
      horizontal->coefficients = info->horizontal.coefficients;
      vertical->contributors = info->vertical.contributors;
      vertical->coefficients = info->vertical.coefficients;
    }

    STBIR__NEXT_PTR( info->column_slices, sizeof( stbir__column_slice ) * sizes.num_column_slices, stbir__column_slice );

    if ( info )
    {
      STBIR_PROFILE_BUILD_START( horizontal );

      stbir__calculate_filters( horizontal, user_data STBIR_ONLY_PROFILE_BUILD_SET_INFO );
      stbir__select_horizontal_kernels( info, horizontal, effective_channels );

      info->conservative_extents = *conservative;

      // pack the horizontal coeffs into the column entries the kernels walk (the vertical's aren't packed: its kernels read exactly n0..n1)
      if ( horizontal->shared_period )
        horizontal->coefficient_width = stbir__pack_shared_coefficients( horizontal, info->conservative_extents.n0, info->conservative_extents.n1 );
      else
        horizontal->coefficient_width = stbir__pack_coefficients(horizontal->num_contributors, horizontal->contributors, horizontal->coefficients, horizontal->coefficient_width, horizontal->extent_info.widest, info->conservative_extents.n0, info->conservative_extents.n1 );
      horizontal->entry_floats = STBIR__COLUMN_ENTRY_FLOATS( horizontal->coefficient_width );

      STBIR_MEMCPY( &info->horizontal, horizontal, sizeof( stbir__sampler ) );

      // now that the contributors are final, work out exactly what each column slice decodes
      info->num_column_slices = sizes.num_column_slices;
      stbir__build_column_slices( info, sizes.slice_columns, sizes.slice_in_span );

      STBIR_PROFILE_BUILD_END( horizontal );

      STBIR_PROFILE_BUILD_START( vertical );
      if ( copy_horizontal )
      {
        // identical samplers: the vertical reads the horizontal's packed column entries (two ints, then the coefficients)
        STBIR_MEMCPY( vertical, horizontal, sizeof( stbir__sampler ) );
        vertical->coefficients += 2;
        vertical->coefficient_width = horizontal->entry_floats;
      }
      else
        stbir__calculate_filters( vertical, user_data STBIR_ONLY_PROFILE_BUILD_SET_INFO );
      STBIR_MEMCPY( &info->vertical, vertical, sizeof( stbir__sampler ) );
      STBIR_PROFILE_BUILD_END( vertical );

      // the horizontal's contributors have been read for the last time (the slice table), so fill in the column entries'
      //   advance and width (an advance that doesn't fit fails the build)
      if ( !stbir__fill_column_entries( &info->horizontal, effective_channels ) )
      {
        stbir__free_internal_mem( info );
        return 0;
      }

      stbir__setup_split_grid( info, split_cols, split_rows );

      // now we know precisely how many entries we need
      info->ring_buffer_num_entries = info->vertical.extent_info.widest;

      // we never need more ring buffer entries than the scanlines we're outputting
      if ( ( !info->vertical.is_gather ) && ( info->ring_buffer_num_entries > conservative_split_output_size ) )
        info->ring_buffer_num_entries = conservative_split_output_size;
      STBIR_ASSERT( info->ring_buffer_num_entries <= info->alloc_ring_buffer_num_entries );
    }
    #undef STBIR__NEXT_PTR

    // is this the first time through loop?
    if ( info == 0 )
    {
      alloced_total = ( 15 + (size_t)advance_mem );
      alloced = STBIR_MALLOC( alloced_total, user_data );
      if ( alloced == 0 )
        return 0;
    }
    else
      return info;  // success
  }
}

// 4K aliasing avoidance: each internal buffer has a page of slack and is placed half a page from the buffer 
//   that streams into it - input -> decode -> ring -> vertical buffer (horizontal first), 
//   input -> ring -> decode -> vertical buffer (vertical first).
static size_t stbir__4K_alias_place( void * alloc, size_t want )
{
  size_t have = ( (size_t)alloc ) & 4095;
  return ( ( want - have ) & 4095 ) & ~(size_t)15;  // stays 16 byte aligned, within the slack
}

static void stbir__4K_alias_skew_buffers( stbir__info const * info, stbir__per_split_info * split_info )
{
  size_t first = ( ( (size_t)info->input_data ) + 2048 ) & 4095;   // the first internal buffer: half a page from the input row
  size_t second = ( first + 2048 ) & 4095;                          // and the next one, back at the input's offset
  size_t vertical_dest;

  if ( info->vertical_first )
  {
    // decode: input -> ring.  vertical: ring -> decode_buffer.  horizontal: decode_buffer -> vertical_buffer.  encode: vertical_buffer -> output
    #ifndef STBIR__SEPARATE_ALLOCATIONS
    split_info->ring_buffer = (float*) ( (char*)split_info->ring_buffer_alloc + stbir__4K_alias_place( split_info->ring_buffer_alloc, first ) );
    #endif
    vertical_dest = ( second + (size_t)stbir__4K_vertical_dest_shift ) & 4095;
    split_info->decode_buffer = (float*) ( (char*)split_info->decode_buffer_alloc + stbir__4K_alias_place( split_info->decode_buffer_alloc, vertical_dest ) );
    split_info->vertical_buffer = (float*) ( (char*)split_info->vertical_buffer_alloc + stbir__4K_alias_place( split_info->vertical_buffer_alloc, ( vertical_dest + 2048 ) & 4095 ) );
  }
  else
  {
    // decode: input -> decode_buffer.  horizontal: decode_buffer -> ring.  vertical: ring -> vertical_buffer.  encode: vertical_buffer -> output
    split_info->decode_buffer = (float*) ( (char*)split_info->decode_buffer_alloc + stbir__4K_alias_place( split_info->decode_buffer_alloc, first ) );
    #ifndef STBIR__SEPARATE_ALLOCATIONS
    split_info->ring_buffer = (float*) ( (char*)split_info->ring_buffer_alloc + stbir__4K_alias_place( split_info->ring_buffer_alloc, second ) );
    #endif
    vertical_dest = ( first + (size_t)stbir__4K_vertical_dest_shift ) & 4095;
    split_info->vertical_buffer = (float*) ( (char*)split_info->vertical_buffer_alloc + stbir__4K_alias_place( split_info->vertical_buffer_alloc, vertical_dest ) );
  }
}

static int stbir__perform_resize( stbir__info const * info, int split_start, int split_count )
{
  stbir__per_split_info * split_info = info->split_info + split_start;
  int b, i, can_merge;

  for( i = 0 ; i < split_count ; i++ )
    stbir__4K_alias_skew_buffers( info, split_info + i );

  STBIR_PROFILE_CLEAR_EXTRAS();

  STBIR_PROFILE_FIRST_START( looping );
  // run the whole vertical loop once per column slice, so each pass only touches a cache sized slice of the buffers.
  //   consecutive splits over the same slices are adjacent row bands and run as one band (one warm-up per slice) - except
  //   when a scatter's ring was cut down to a band's rows: an input row can touch more output rows than that across
  //   two bands, so those run one at a time
  can_merge = info->vertical.is_gather || ( info->ring_buffer_num_entries >= info->vertical.extent_info.widest );
  i = 0;
  while ( i < split_count )
  {
    int run = 1;
    while ( can_merge && ( i + run < split_count ) && ( split_info[i+run].first_column_slice == split_info[i].first_column_slice ) && ( split_info[i+run].num_column_slices == split_info[i].num_column_slices ) )
      ++run;
    STBIR_PROFILE_NEST_IN_FIRST( split_info + i );
    for( b = 0 ; b < split_info[i].num_column_slices ; b++ )
    {
      stbir__column_slice const * slice = info->column_slices + split_info[i].first_column_slice + b;
      if (info->vertical.is_gather)
        stbir__vertical_gather_loop( info, split_info + i, run, slice );
      else
        stbir__vertical_scatter_loop( info, split_info + i, run, slice );
    }
    i += run;
  }
  STBIR_PROFILE_END( looping );

  return 1;
}

static void stbir__update_info_from_resize( stbir__info * info, STBIR_RESIZE * resize )
{
  static stbir__decode_pixels_func * decode_simple[STBIR_TYPE_HALF_FLOAT-STBIR_TYPE_UINT8_SRGB+1]=
  {
    /* 1ch-4ch */ stbir__decode_uint8_srgb, stbir__decode_uint8_srgb, 0, stbir__decode_float_linear, stbir__decode_half_float_linear,
  };

  static stbir__decode_pixels_func * decode_alphas[STBIRI_AR-STBIRI_RGBA+1][STBIR_TYPE_HALF_FLOAT-STBIR_TYPE_UINT8_SRGB+1]=
  {
    { /* RGBA */ stbir__decode_uint8_srgb4_linearalpha,      stbir__decode_uint8_srgb,      0, stbir__decode_float_linear,      stbir__decode_half_float_linear },
    { /* BGRA */ stbir__decode_uint8_srgb4_linearalpha_BGRA, stbir__decode_uint8_srgb_BGRA, 0, stbir__decode_float_linear_BGRA, stbir__decode_half_float_linear_BGRA },
    { /* ARGB */ stbir__decode_uint8_srgb4_linearalpha_ARGB, stbir__decode_uint8_srgb_ARGB, 0, stbir__decode_float_linear_ARGB, stbir__decode_half_float_linear_ARGB },
    { /* ABGR */ stbir__decode_uint8_srgb4_linearalpha_ABGR, stbir__decode_uint8_srgb_ABGR, 0, stbir__decode_float_linear_ABGR, stbir__decode_half_float_linear_ABGR },
    { /* RA   */ stbir__decode_uint8_srgb2_linearalpha,      stbir__decode_uint8_srgb,      0, stbir__decode_float_linear,      stbir__decode_half_float_linear },
    { /* AR   */ stbir__decode_uint8_srgb2_linearalpha_AR,   stbir__decode_uint8_srgb_AR,   0, stbir__decode_float_linear_AR,   stbir__decode_half_float_linear_AR },
  };

  static stbir__decode_pixels_func * decode_simple_scaled_or_not[2][2]=
  {
    { stbir__decode_uint8_linear_scaled,  stbir__decode_uint8_linear }, { stbir__decode_uint16_linear_scaled, stbir__decode_uint16_linear },
  };

  static stbir__decode_pixels_func * decode_alphas_scaled_or_not[STBIRI_AR-STBIRI_RGBA+1][2][2]=
  {
    { /* RGBA */ { stbir__decode_uint8_linear_scaled,       stbir__decode_uint8_linear },      { stbir__decode_uint16_linear_scaled,      stbir__decode_uint16_linear } },
    { /* BGRA */ { stbir__decode_uint8_linear_scaled_BGRA,  stbir__decode_uint8_linear_BGRA }, { stbir__decode_uint16_linear_scaled_BGRA, stbir__decode_uint16_linear_BGRA } },
    { /* ARGB */ { stbir__decode_uint8_linear_scaled_ARGB,  stbir__decode_uint8_linear_ARGB }, { stbir__decode_uint16_linear_scaled_ARGB, stbir__decode_uint16_linear_ARGB } },
    { /* ABGR */ { stbir__decode_uint8_linear_scaled_ABGR,  stbir__decode_uint8_linear_ABGR }, { stbir__decode_uint16_linear_scaled_ABGR, stbir__decode_uint16_linear_ABGR } },
    { /* RA   */ { stbir__decode_uint8_linear_scaled,       stbir__decode_uint8_linear },      { stbir__decode_uint16_linear_scaled,      stbir__decode_uint16_linear } },
    { /* AR   */ { stbir__decode_uint8_linear_scaled_AR,    stbir__decode_uint8_linear_AR },   { stbir__decode_uint16_linear_scaled_AR,   stbir__decode_uint16_linear_AR } }
  };

  static stbir__encode_pixels_func * encode_simple[STBIR_TYPE_HALF_FLOAT-STBIR_TYPE_UINT8_SRGB+1]=
  {
    /* 1ch-4ch */ stbir__encode_uint8_srgb, stbir__encode_uint8_srgb, 0, stbir__encode_float_linear, stbir__encode_half_float_linear,
  };

  static stbir__encode_pixels_func * encode_alphas[STBIRI_AR-STBIRI_RGBA+1][STBIR_TYPE_HALF_FLOAT-STBIR_TYPE_UINT8_SRGB+1]=
  {
    { /* RGBA */ stbir__encode_uint8_srgb4_linearalpha,      stbir__encode_uint8_srgb,      0, stbir__encode_float_linear,      stbir__encode_half_float_linear },
    { /* BGRA */ stbir__encode_uint8_srgb4_linearalpha_BGRA, stbir__encode_uint8_srgb_BGRA, 0, stbir__encode_float_linear_BGRA, stbir__encode_half_float_linear_BGRA },
    { /* ARGB */ stbir__encode_uint8_srgb4_linearalpha_ARGB, stbir__encode_uint8_srgb_ARGB, 0, stbir__encode_float_linear_ARGB, stbir__encode_half_float_linear_ARGB },
    { /* ABGR */ stbir__encode_uint8_srgb4_linearalpha_ABGR, stbir__encode_uint8_srgb_ABGR, 0, stbir__encode_float_linear_ABGR, stbir__encode_half_float_linear_ABGR },
    { /* RA   */ stbir__encode_uint8_srgb2_linearalpha,      stbir__encode_uint8_srgb,      0, stbir__encode_float_linear,      stbir__encode_half_float_linear },
    { /* AR   */ stbir__encode_uint8_srgb2_linearalpha_AR,   stbir__encode_uint8_srgb_AR,   0, stbir__encode_float_linear_AR,   stbir__encode_half_float_linear_AR }
  };

  static stbir__encode_pixels_func * encode_simple_scaled_or_not[2][2]=
  {
    { stbir__encode_uint8_linear_scaled,  stbir__encode_uint8_linear }, { stbir__encode_uint16_linear_scaled, stbir__encode_uint16_linear },
  };

  static stbir__encode_pixels_func * encode_alphas_scaled_or_not[STBIRI_AR-STBIRI_RGBA+1][2][2]=
  {
    { /* RGBA */ { stbir__encode_uint8_linear_scaled,       stbir__encode_uint8_linear },       { stbir__encode_uint16_linear_scaled,      stbir__encode_uint16_linear } },
    { /* BGRA */ { stbir__encode_uint8_linear_scaled_BGRA,  stbir__encode_uint8_linear_BGRA },  { stbir__encode_uint16_linear_scaled_BGRA, stbir__encode_uint16_linear_BGRA } },
    { /* ARGB */ { stbir__encode_uint8_linear_scaled_ARGB,  stbir__encode_uint8_linear_ARGB },  { stbir__encode_uint16_linear_scaled_ARGB, stbir__encode_uint16_linear_ARGB } },
    { /* ABGR */ { stbir__encode_uint8_linear_scaled_ABGR,  stbir__encode_uint8_linear_ABGR },  { stbir__encode_uint16_linear_scaled_ABGR, stbir__encode_uint16_linear_ABGR } },
    { /* RA   */ { stbir__encode_uint8_linear_scaled,       stbir__encode_uint8_linear },       { stbir__encode_uint16_linear_scaled,      stbir__encode_uint16_linear } },
    { /* AR   */ { stbir__encode_uint8_linear_scaled_AR,    stbir__encode_uint8_linear_AR },    { stbir__encode_uint16_linear_scaled_AR,   stbir__encode_uint16_linear_AR } }
  };

  stbir__decode_pixels_func * decode_pixels = 0;
  stbir__encode_pixels_func * encode_pixels = 0;
  stbir_datatype input_type, output_type;
  int non_scaled;

  input_type = resize->input_data_type;
  output_type = resize->output_data_type;
  info->input_data = resize->input_pixels;
  info->input_stride_bytes = resize->input_stride_in_bytes;
  info->output_stride_bytes = resize->output_stride_in_bytes;

  // if we're completely point sampling, then we can turn off SRGB
  if ( ( info->horizontal.filter_enum == STBIR_FILTER_POINT_SAMPLE ) && ( info->vertical.filter_enum == STBIR_FILTER_POINT_SAMPLE ) )
  {
    if ( ( ( input_type  == STBIR_TYPE_UINT8_SRGB ) || ( input_type  == STBIR_TYPE_UINT8_SRGB_ALPHA ) ) &&
         ( ( output_type == STBIR_TYPE_UINT8_SRGB ) || ( output_type == STBIR_TYPE_UINT8_SRGB_ALPHA ) ) )
    {
      input_type = STBIR_TYPE_UINT8;
      output_type = STBIR_TYPE_UINT8;
    }
  }

  // recalc the output and input strides (zero means packed - the whole image's width, the sub rect is a region of it)
  if ( info->input_stride_bytes == 0 )
    info->input_stride_bytes = (stbir_int64)info->channels * info->horizontal.scale_info.input_full_size * stbir__type_size[input_type];

  if ( info->output_stride_bytes == 0 )
    info->output_stride_bytes = (stbir_int64)info->channels * resize->output_w * stbir__type_size[output_type];

  // calc offset
  info->output_data = ( (char*) resize->output_pixels ) + (ptrdiff_t)( info->offset_y * info->output_stride_bytes ) + ( (size_t) info->offset_x * (size_t) info->channels * (size_t) stbir__type_size[output_type] );

  info->in_pixels_cb = resize->input_cb;
  info->user_data = resize->user_data;
  info->out_pixels_cb = resize->output_cb;

  // check if we can run unscaled - 0-255.0/0-65535.0 instead of 0-1.0 (which is a tiny bit faster when doing linear 8->8 or 16->16)
  //   don't short circuit when alpha weighting (get everything to 0-1.0 as usual)
  non_scaled = 0;
  if ( ( !info->alpha_weight ) && ( !info->alpha_unweight ) )
    if ( ( ( input_type == STBIR_TYPE_UINT8 ) && ( output_type == STBIR_TYPE_UINT8 ) ) || ( ( input_type == STBIR_TYPE_UINT16 ) && ( output_type == STBIR_TYPE_UINT16 ) ) )
      non_scaled = 1;

  // setup the input format converters
  if ( ( input_type == STBIR_TYPE_UINT8 ) || ( input_type == STBIR_TYPE_UINT16 ) )
  {
    if ( info->input_pixel_layout_internal <= STBIRI_4CHANNEL )
      decode_pixels = decode_simple_scaled_or_not[ input_type == STBIR_TYPE_UINT16 ][ non_scaled ];
    else
      decode_pixels = decode_alphas_scaled_or_not[ ( info->input_pixel_layout_internal - STBIRI_RGBA ) % ( STBIRI_AR-STBIRI_RGBA+1 ) ][ input_type == STBIR_TYPE_UINT16 ][ non_scaled ];
  }
  else
  {
    if ( info->input_pixel_layout_internal <= STBIRI_4CHANNEL )
      decode_pixels = decode_simple[ input_type - STBIR_TYPE_UINT8_SRGB ];
    else
      decode_pixels = decode_alphas[ ( info->input_pixel_layout_internal - STBIRI_RGBA ) % ( STBIRI_AR-STBIRI_RGBA+1 ) ][ input_type - STBIR_TYPE_UINT8_SRGB ];
  }

  // setup the output format converters
  if ( ( output_type == STBIR_TYPE_UINT8 ) || ( output_type == STBIR_TYPE_UINT16 ) )
  {
    if ( info->output_pixel_layout_internal <= STBIRI_4CHANNEL )
      encode_pixels = encode_simple_scaled_or_not[ output_type == STBIR_TYPE_UINT16 ][ non_scaled ];
    else
      encode_pixels = encode_alphas_scaled_or_not[ ( info->output_pixel_layout_internal - STBIRI_RGBA ) % ( STBIRI_AR-STBIRI_RGBA+1 ) ][ output_type == STBIR_TYPE_UINT16 ][ non_scaled ];
  }
  else
  {
    if ( info->output_pixel_layout_internal <= STBIRI_4CHANNEL )
      encode_pixels = encode_simple[ output_type - STBIR_TYPE_UINT8_SRGB ];
    else
      encode_pixels = encode_alphas[ ( info->output_pixel_layout_internal - STBIRI_RGBA ) % ( STBIRI_AR-STBIRI_RGBA+1 ) ][ output_type - STBIR_TYPE_UINT8_SRGB ];
  }

  info->input_type = input_type;
  info->output_type = output_type;
  info->decode_pixels = decode_pixels;
  info->encode_pixels = encode_pixels;

  // the input callback's temp buffer sits inside the decode buffer, pushed forward so the decoder reads ahead of
  //   its writes - except plain float, which is a no-op decode when the temp is exactly the decode target
  info->input_callback_offset_bytes = ( decode_pixels == stbir__decode_float_linear ) ? 0 : ( sizeof(float) * STBIR_INPUT_CALLBACK_PADDING );
}

static void stbir__clip( int * outx, int * outsubw, int outw, stbir_real * u0, stbir_real * u1 )
{
  stbir_real per, adj;
  int over;

  // do left/top edge
  if ( *outx < 0 )
  {
    per = ( (stbir_real)*outx ) / ( (stbir_real)*outsubw ); // is negative
    adj = per * ( *u1 - *u0 );
    *u0 -= adj; // increases u0
    *outsubw += *outx; // and drop the clipped off pixels from the output count
    *outx = 0;
  }

  // do right/bot edge
  over = outw - ( *outx + *outsubw );
  if ( over < 0 )
  {
    per = ( (stbir_real)over ) / ( (stbir_real)*outsubw ); // is negative
    adj = per * ( *u1 - *u0 );
    *u1 += adj; // decrease u1
    *outsubw = outw - *outx;
  }
}

static stbir_uint64 stbir__gcd( stbir_uint64 a, stbir_uint64 b )
{
  while ( b )
  {
    stbir_uint64 remainder = a % b;
    a = b;
    b = remainder;
  }
  return a;
}

// The simplest fraction within tolerance of f (f>0): its continued fraction's first convergent that is, in 31 bits (or 0).
static int stbir__real_to_fraction( stbir_real f, stbir_real tolerance, stbir_uint32 * out_numer, stbir_uint32 * out_denom )
{
  stbir_real rem = f;
  stbir_uint64 numer = 1, denom = 0, numer_last = 0, denom_last = 1;   // the convergents numer / denom
  int i;

  if ( f <= (stbir_real) 0.0 )
    return 0;
  for( i = 0 ; i < (int) sizeof( stbir_real ) * 8 ; i++ )   // (a term is at least a bit of f)
  {
    stbir_uint64 term = (stbir_uint64) rem;   // the floor (rem is positive)
    stbir_uint64 next;
    stbir_real frac, err;

    if ( ( term > 0x7fffffff ) || ( ( term * numer + numer_last ) > 0x7fffffff ) || ( ( term * denom + denom_last ) > 0x7fffffff ) )
      return 0;
    next = term * numer + numer_last; numer_last = numer; numer = next;
    next = term * denom + denom_last; denom_last = denom; denom = next;

    err = (stbir_real) numer / (stbir_real) denom - f;
    if ( err < (stbir_real) 0.0 )
      err = -err;
    if ( err <= tolerance )
    {
      *out_numer = (stbir_uint32) numer;
      *out_denom = (stbir_uint32) denom;
      return 1;
    }

    frac = rem - (stbir_real) term;
    if ( frac <= (stbir_real) 0.0 )
      return 0;   // (the fraction is exactly f, and still not within tolerance - can't happen for a tolerance >= 0)
    rem = (stbir_real) 1.0 / frac;
  }
  return 0;
}

// try to find approximation that stays under the drift value: the far edge of the output moves by less than
//   1 / STBIR__POLYPHASE_DRIFT_DIV of an output pixel 
#define STBIR__POLYPHASE_DRIFT_DIV 8192
static void stbir__approximate_polyphase( stbir_uint64 numer, stbir_uint64 denom, stbir_uint64 outputs, stbir_uint64 * out_numer, stbir_uint64 * out_denom, int * out_drift_in_pixels )
{
  stbir_uint64 top = numer, bot = denom;
  stbir_uint64 p = 1, q = 0, p_last = 0, q_last = 1;

  while ( bot )
  {
    stbir_uint64 est = top / bot;
    stbir_uint64 remainder = top % bot;
    stbir_uint64 next, pd, qn, diff;

    top = bot;
    bot = remainder;

    // the next convergent p / q
    next = est * p + p_last;
    p_last = p;
    p = next;

    next = est * q + q_last;
    q_last = q;
    q = next;

    // no simpler fraction is close enough (the convergents only grow from here)
    if ( p >= outputs )
      return;

    // its drift at the far edge, in output pixels: outputs x | p D - q N | / ( q N )
    pd = p * denom;
    qn = q * numer;
    diff = ( pd > qn ) ? pd - qn : qn - pd;
    if ( diff <= qn / ( outputs * STBIR__POLYPHASE_DRIFT_DIV ) )
    {
      // that drift at the far edge in input pixels (times denom / numer), rounded up - for the conservative range
      stbir_real drift_in = (stbir_real)outputs * (stbir_real)diff * (stbir_real)denom / ( (stbir_real)qn * (stbir_real)numer );
      *out_numer = p;
      *out_denom = q;
      *out_drift_in_pixels = (int)drift_in + 1;
      return;
    }
  }
}

// The scale as a fraction: output_sub / ( input_full x ( s1 - s0 ) ), with s1 - s0 = ( n1 - n0 ) / d - exact if possible.
//   Returns 0 when there is no fraction that fits in int.
static int stbir__scale_to_rational( int output_sub_range, int input_full_range, stbir_real input_s, int s_n0, int s_n1, int s_d, stbir_uint32 * out_numer, stbir_uint32 * out_denom, int * out_approximated_drift )
{
  stbir_uint64 numer, denom, g;
  stbir_real scale, err;

  *out_numer = 0;   // (not a fraction: the fields are read before the flag in places, so never leave them unset)
  *out_denom = 0;
  *out_approximated_drift = 0;

  if ( ( s_d > 0 ) && ( s_n1 > s_n0 ) )
  {
    numer = (stbir_uint64)output_sub_range * (stbir_uint64)s_d;
    denom = (stbir_uint64)input_full_range * (stbir_uint64)( s_n1 - s_n0 );
    g = stbir__gcd( numer, denom );
    numer /= g; denom /= g;
    if ( ( numer <= 0x7fffffff ) && ( denom <= 0x7fffffff ) )
    {
      if ( numer >= (stbir_uint64)output_sub_range )
        stbir__approximate_polyphase( numer, denom, (stbir_uint64)output_sub_range, &numer, &denom, out_approximated_drift );
      *out_numer = (stbir_uint32) numer;
      *out_denom = (stbir_uint32) denom;
      return 1;
    }
  }

  scale = (stbir_real)output_sub_range / ( (stbir_real)input_full_range * input_s );
  if ( !stbir__real_to_fraction( scale, scale / ( (stbir_real)output_sub_range * (stbir_real)STBIR__POLYPHASE_DRIFT_DIV ), out_numer, out_denom ) )
    return 0;
  err = (stbir_real)*out_numer / (stbir_real)*out_denom - scale;
  if ( err < (stbir_real) 0.0 )
    err = -err;
  if ( err > (stbir_real) 0.0 )
    *out_approximated_drift = (int)( (stbir_real)output_sub_range * err / ( scale * scale ) ) + 1;   // the far edge's drift: output pixels x ( err / scale ), in input pixels ( / scale )
  return 1;
}

// clip the input area to the image (and adjust the output area), the counterpart of stbir__clip: the output pixels whose
//   centers land outside the image are dropped from the ends, so the ones left sample exactly where they did
static void stbir__clip_input( int * outx, int * outsubw, stbir_real * u0, stbir_real * u1 )
{
  stbir_real per_pixel = ( *u1 - *u0 ) / ( (stbir_real)*outsubw ); // input area per output pixel
  int drop;

  // do left/top edge
  if ( *u0 < 0 )
  {
    drop = stbir__ceil_to_int( -*u0 / per_pixel - (stbir_real)0.5 ); // pixels centered before the image (one right on the edge stays)
    if ( drop > *outsubw )
      drop = *outsubw;
    if ( drop > 0 )
    {
      *u0 += (stbir_real)drop * per_pixel; // increases u0
      *outx += drop;
      *outsubw -= drop;
    }
  }

  // do right/bot edge
  if ( *u1 > 1 )
  {
    drop = stbir__ceil_to_int( ( *u1 - 1 ) / per_pixel - (stbir_real)0.5 ); // pixels centered past the image
    if ( drop > *outsubw )
      drop = *outsubw;
    if ( drop > 0 )
    {
      *u1 -= (stbir_real)drop * per_pixel; // decreases u1
      *outsubw -= drop;
    }
  }
}

static int stbir__calculate_region_transform( stbir__scale_info * scale_info, int output_full_range, int * output_offset, int output_sub_range, int input_full_range, stbir_real input_s0, stbir_real input_s1, int s_n0, int s_n1, int s_d )
{
  stbir_real output_range, input_range, output_s, input_s, input_pixels, ratio, unclipped_input_s;
  int unclipped_output_sub_range = output_sub_range;
  int unclipped_output_offset = *output_offset;

  input_s = input_s1 - input_s0;
  unclipped_input_s = input_s;

  // null area (a negative size is an empty one)
  if ( ( output_full_range <= 0 ) || ( input_full_range <= 0 ) ||
       ( output_sub_range <= 0 ) || ( input_s <= (stbir_real)0.0 ) )
    return 0;

  // are either of the ranges completely out of bounds?
  if ( ( *output_offset >= output_full_range ) || ( ( *output_offset + output_sub_range ) <= 0 ) || ( input_s0 >= (stbir_real)1.0 ) || ( input_s1 <= (stbir_real)0.0 ) )
    return 0;

  output_range = (stbir_real)output_full_range;
  input_range = (stbir_real)input_full_range;

  output_s = ( (stbir_real)output_sub_range) / output_range;

  // figure out the scaling to use
  ratio = output_s / input_s;

  // save scale before clipping: the output pixels over the input pixels the region spans, and its inverse as the same
  //   two numbers divided the other way - one rounding each, and since a division is monotonic the two agree about 1
  //   (a scale that rounds to exactly 1 can't have an inverse over 1, which would put a ratio over 1 to the kernels)
  input_pixels = input_range * input_s;
  scale_info->scale = (stbir_real)output_sub_range / input_pixels;
  scale_info->inv_scale = input_pixels / (stbir_real)output_sub_range;

  // clip output area to left/right output edges (and adjust input area)
  stbir__clip( output_offset, &output_sub_range, output_full_range, &input_s0, &input_s1 );

  // clip input area to the image (and adjust output area)
  stbir__clip_input( output_offset, &output_sub_range, &input_s0, &input_s1 );

  // recalc input area
  input_s = input_s1 - input_s0;

  // after clipping do we have zero input area?
  if ( ( input_s <= (stbir_real)0.0 ) || ( output_sub_range <= 0 ) )
    return 0;

  // calculate and store the starting source offsets in output pixel space
  scale_info->pixel_shift = input_s0 * ratio * output_range;

  scale_info->scale_is_rational = stbir__scale_to_rational( unclipped_output_sub_range, input_full_range, unclipped_input_s, s_n0, s_n1, s_d, &scale_info->scale_numerator, &scale_info->scale_denominator, &scale_info->approximated_drift );

  // an exact fraction gives the scale itself - correctly rounded, so an exact ratio's filter widths ( ceil( 2 support / scale ) )
  //   don't pick up an extra tap from the region math landing an ulp under the integer, and a 1:1 is exactly 1 (the up / down
  //   decision). an approximated fraction leaves the real scale: it is what the kernels see, and the decision has to agree with it
  if ( ( scale_info->scale_is_rational ) && ( !scale_info->approximated_drift ) )
  {
    scale_info->scale = (stbir_real) scale_info->scale_numerator / (stbir_real) scale_info->scale_denominator;
    scale_info->inv_scale = (stbir_real) scale_info->scale_denominator / (stbir_real) scale_info->scale_numerator;
  }

  // exact tap positions when the shift is an integer ( n0 x output_sub / ( n1 - n0 ), plus the clip ) and the fraction wasn't approximated.
  //   the numerators are ( 2o + 1 + 2 shift ) D and ( 2i + 1 ) N: with sizes up to 2^30 (the resizer's limit) and the
  //   fraction's terms below 2^30 they stay below 2^62
  scale_info->positions_exact = 0;
  scale_info->shift_pixels = 0;
  if ( ( scale_info->scale_is_rational ) && ( !scale_info->approximated_drift ) && ( s_d > 0 ) && ( ( ( (stbir_int64)s_n0 * unclipped_output_sub_range ) % ( s_n1 - s_n0 ) ) == 0 ) &&
       ( scale_info->scale_numerator < ( 1 << 30 ) ) && ( scale_info->scale_denominator < ( 1 << 30 ) ) )
  {
    stbir_int64 shift = ( (stbir_int64)s_n0 * unclipped_output_sub_range ) / ( s_n1 - s_n0 ) + ( *output_offset - unclipped_output_offset );
    if ( ( shift > -( 1 << 30 ) ) && ( shift < ( 1 << 30 ) ) )   // the shift is in output pixels too (an output tile far past the image's own size)
    {
      scale_info->shift_pixels = (int) shift;
      scale_info->tap_scale_in = (stbir_real) 1.0 / ( (stbir_real) 2.0 * (stbir_real) scale_info->scale_numerator );
      scale_info->tap_scale_out = (stbir_real) 1.0 / ( (stbir_real) 2.0 * (stbir_real) scale_info->scale_denominator );
      scale_info->pixel_shift = (stbir_real) shift;
      scale_info->positions_exact = 1;
    }
  }

  scale_info->input_full_size = input_full_range;
  scale_info->output_sub_size = output_sub_range;

  return 1;
}


static void stbir__init_and_set_layout( STBIR_RESIZE * resize, stbir_pixel_layout pixel_layout, stbir_datatype data_type )
{
  resize->input_cb = 0;
  resize->output_cb = 0;
  resize->user_data = resize;
  resize->samplers = 0;
  resize->called_alloc = 0;
  resize->horizontal_filter = STBIR_FILTER_DEFAULT;
  resize->horizontal_filter_kernel = 0; resize->horizontal_filter_support = 0; resize->horizontal_filter_kernel_new = 0; resize->horizontal_filter_support_new = 0; resize->horizontal_filter_radius = 0;
  resize->vertical_filter = STBIR_FILTER_DEFAULT;
  resize->vertical_filter_kernel = 0; resize->vertical_filter_support = 0; resize->vertical_filter_kernel_new = 0; resize->vertical_filter_support_new = 0; resize->vertical_filter_radius = 0;
  resize->horizontal_edge = STBIR_EDGE_CLAMP;
  resize->vertical_edge = STBIR_EDGE_CLAMP;
  resize->input_s0 = 0; resize->input_t0 = 0; resize->input_s1 = 1; resize->input_t1 = 1;
  resize->input_s_n0 = 0; resize->input_s_n1 = 1; resize->input_s_d = 1;
  resize->input_t_n0 = 0; resize->input_t_n1 = 1; resize->input_t_d = 1;
  resize->output_subx = 0; resize->output_suby = 0; resize->output_subw = resize->output_w; resize->output_subh = resize->output_h;
  resize->input_data_type = data_type;
  resize->output_data_type = data_type;
  resize->input_pixel_layout_public = pixel_layout;
  resize->output_pixel_layout_public = pixel_layout;
  resize->needs_rebuild = 1;
}

STBIRDEF void stbir_resize_init( STBIR_RESIZE * resize,
                                 const void *input_pixels,  int input_w,  int input_h, stbir_int64 input_stride_in_bytes, // stride can be zero
                                       void *output_pixels, int output_w, int output_h, stbir_int64 output_stride_in_bytes, // stride can be zero
                                 stbir_pixel_layout pixel_layout, stbir_datatype data_type )
{
  resize->input_pixels = input_pixels;
  resize->input_w = input_w;
  resize->input_h = input_h;
  resize->input_stride_in_bytes = input_stride_in_bytes;
  resize->output_pixels = output_pixels;
  resize->output_w = output_w;
  resize->output_h = output_h;
  resize->output_stride_in_bytes = output_stride_in_bytes;
  resize->fast_alpha = 0;

  stbir__init_and_set_layout( resize, pixel_layout, data_type );
}

// You can update parameters any time after resize_init
STBIRDEF void stbir_set_datatypes( STBIR_RESIZE * resize, stbir_datatype input_type, stbir_datatype output_type )  // by default, datatype from resize_init
{
  resize->input_data_type = input_type;
  resize->output_data_type = output_type;
  if ( ( resize->samplers ) && ( !resize->needs_rebuild ) )
    stbir__update_info_from_resize( resize->samplers, resize );
}

STBIRDEF void stbir_set_pixel_callbacks( STBIR_RESIZE * resize, stbir_input_callback * input_cb, stbir_output_callback * output_cb )   // no callbacks by default
{
  resize->input_cb = input_cb;
  resize->output_cb = output_cb;

  if ( ( resize->samplers ) && ( !resize->needs_rebuild ) )
  {
    resize->samplers->in_pixels_cb = input_cb;
    resize->samplers->out_pixels_cb = output_cb;
  }
}

STBIRDEF void stbir_set_user_data( STBIR_RESIZE * resize, void * user_data )                                     // pass back STBIR_RESIZE* by default
{
  resize->user_data = user_data;
  if ( ( resize->samplers ) && ( !resize->needs_rebuild ) )
    resize->samplers->user_data = user_data;
}

STBIRDEF void stbir_set_buffer_ptrs( STBIR_RESIZE * resize, const void * input_pixels, stbir_int64 input_stride_in_bytes, void * output_pixels, stbir_int64 output_stride_in_bytes )
{
  resize->input_pixels = input_pixels;
  resize->input_stride_in_bytes = input_stride_in_bytes;
  resize->output_pixels = output_pixels;
  resize->output_stride_in_bytes = output_stride_in_bytes;
  if ( ( resize->samplers ) && ( !resize->needs_rebuild ) )
    stbir__update_info_from_resize( resize->samplers, resize );
}


STBIRDEF int stbir_set_edgemodes( STBIR_RESIZE * resize, stbir_edge horizontal_edge, stbir_edge vertical_edge )       // CLAMP by default
{
  resize->horizontal_edge = horizontal_edge;
  resize->vertical_edge = vertical_edge;
  resize->needs_rebuild = 1;
  return 1;
}

STBIRDEF int stbir_set_filters( STBIR_RESIZE * resize, stbir_filter horizontal_filter, stbir_filter vertical_filter ) // STBIR_DEFAULT_FILTER_UPSAMPLE/DOWNSAMPLE by default
{
  resize->horizontal_filter = horizontal_filter;
  resize->vertical_filter = vertical_filter;
  resize->needs_rebuild = 1;
  return 1;
}

STBIRDEF int stbir_set_filter_callbacks_new( STBIR_RESIZE * resize, stbir__kernel_callback_new * horizontal_filter, stbir__support_callback_new * horizontal_support, stbir__radius_callback_new * horizontal_radius, stbir__kernel_callback_new * vertical_filter, stbir__support_callback_new * vertical_support, stbir__radius_callback_new * vertical_radius )
{
  // a filter's extent is given one way: a support (filter pixels) or a radius (input pixels), not both
  if ( ( horizontal_support && horizontal_radius ) || ( vertical_support && vertical_radius ) )
    return 0;
  resize->horizontal_filter_kernel_new = horizontal_filter;
  resize->horizontal_filter_support_new = horizontal_support;
  resize->horizontal_filter_radius = horizontal_radius;
  resize->horizontal_filter_kernel = 0;   // (the old form's, if it was set before)
  resize->horizontal_filter_support = 0;
  resize->vertical_filter_kernel_new = vertical_filter;
  resize->vertical_filter_support_new = vertical_support;
  resize->vertical_filter_radius = vertical_radius;
  resize->vertical_filter_kernel = 0;
  resize->vertical_filter_support = 0;
  resize->needs_rebuild = 1;
  return 1;
}

// the old form: the float kernels and supports are kept apart and the sampler calls them through a conversion
STBIRDEF int stbir_set_filter_callbacks( STBIR_RESIZE * resize, stbir__kernel_callback * horizontal_filter, stbir__support_callback * horizontal_support, stbir__kernel_callback * vertical_filter, stbir__support_callback * vertical_support )
{
  resize->horizontal_filter_kernel = horizontal_filter;
  resize->horizontal_filter_support = horizontal_support;
  resize->horizontal_filter_kernel_new = 0;
  resize->horizontal_filter_support_new = 0;
  resize->horizontal_filter_radius = 0;
  resize->vertical_filter_kernel = vertical_filter;
  resize->vertical_filter_support = vertical_support;
  resize->vertical_filter_kernel_new = 0;
  resize->vertical_filter_support_new = 0;
  resize->vertical_filter_radius = 0;
  resize->needs_rebuild = 1;
  return 1;
}

STBIRDEF int stbir_set_pixel_layouts( STBIR_RESIZE * resize, stbir_pixel_layout input_pixel_layout, stbir_pixel_layout output_pixel_layout )   // sets new pixel layouts
{
  resize->input_pixel_layout_public = input_pixel_layout;
  resize->output_pixel_layout_public = output_pixel_layout;
  resize->needs_rebuild = 1;
  return 1;
}


STBIRDEF int stbir_set_non_pm_alpha_speed_over_quality( STBIR_RESIZE * resize, int non_pma_alpha_speed_over_quality )   // sets alpha speed
{
  resize->fast_alpha = non_pma_alpha_speed_over_quality;
  resize->needs_rebuild = 1;
  return 1;
}

STBIRDEF int stbir_set_input_subrect( STBIR_RESIZE * resize, stbir_real s0, stbir_real t0, stbir_real s1, stbir_real t1 )                 // sets input region (full region by default)
{
  resize->input_s0 = s0;
  resize->input_t0 = t0;
  resize->input_s1 = s1;
  resize->input_t1 = t1;
  resize->input_s_d = 0;   // not in pixels: the fractions are worked out from the reals at build time
  resize->input_t_d = 0;
  resize->needs_rebuild = 1;

  // are we inbounds? (a zero or negative area, or a region wholly past the image, is empty)
  if ( ( s1 <= (stbir_real)0.0 ) || ( s1 <= s0 ) ||
       ( t1 <= (stbir_real)0.0 ) || ( t1 <= t0 ) ||
       ( s0 >= (stbir_real)1.0 ) ||
       ( t0 >= (stbir_real)1.0 ) )
    return 0;

  return 1;
}

STBIRDEF int stbir_set_output_pixel_subrect( STBIR_RESIZE * resize, int subx, int suby, int subw, int subh )          // sets the output sub rect, in output pixels (the whole output by default)
{
  resize->output_subx = subx;
  resize->output_suby = suby;
  resize->output_subw = subw;
  resize->output_subh = subh;
  resize->needs_rebuild = 1;

  // is there anything in it? (a negative size is an empty rect - the resize then has nothing to do)
  if ( ( subx >= resize->output_w ) || ( ( subx + subw ) <= 0 ) || ( suby >= resize->output_h ) || ( ( suby + subh ) <= 0 ) || ( subw <= 0 ) || ( subh <= 0 ) )
    return 0;

  return 1;
}

STBIRDEF int stbir_set_pixel_subrect( STBIR_RESIZE * resize, int subx, int suby, int subw, int subh )                 // sets both regions (full regions by default)
{
  stbir_real s0, t0, s1, t1;

  s0 = ( (stbir_real)subx ) / ( (stbir_real)resize->output_w );
  t0 = ( (stbir_real)suby ) / ( (stbir_real)resize->output_h );
  s1 = ( (stbir_real)(subx+subw) ) / ( (stbir_real)resize->output_w );
  t1 = ( (stbir_real)(suby+subh) ) / ( (stbir_real)resize->output_h );

  resize->input_s0 = s0;
  resize->input_t0 = t0;
  resize->input_s1 = s1;
  resize->input_t1 = t1;
  resize->input_s_n0 = subx;
  resize->input_s_n1 = subx + subw;
  resize->input_s_d = resize->output_w;
  resize->input_t_n0 = suby;
  resize->input_t_n1 = suby + subh;
  resize->input_t_d = resize->output_h;
  resize->output_subx = subx;
  resize->output_suby = suby;
  resize->output_subw = subw;
  resize->output_subh = subh;
  resize->needs_rebuild = 1;

  // is there anything in it? (a negative size is an empty rect - the resize then has nothing to do)
  if ( ( subx >= resize->output_w ) || ( ( subx + subw ) <= 0 ) || ( suby >= resize->output_h ) || ( ( suby + subh ) <= 0 ) || ( subw <= 0 ) || ( subh <= 0 ) )
    return 0;

  return 1;
}

static int stbir__perform_build( STBIR_RESIZE * resize, int splits )
{
  stbir__contributors conservative = { 0, 0 };
  stbir__sampler horizontal, vertical;
  int new_output_subx, new_output_suby;
  stbir__info * out_info;
  #ifdef STBIR_PROFILE
  stbir__info profile_infod;  // used to contain building profile info before everything is allocated
  stbir__info * profile_info = &profile_infod;
  #endif

  // have we already built the samplers?
  if ( resize->samplers )
    return 0;

  #define STBIR_RETURN_ERROR_AND_ASSERT( exp )  STBIR_ASSERT( !(exp) ); if (exp) return 0;
  STBIR_RETURN_ERROR_AND_ASSERT( (unsigned)resize->horizontal_filter >= STBIR_FILTER_OTHER)
  STBIR_RETURN_ERROR_AND_ASSERT( (unsigned)resize->vertical_filter >= STBIR_FILTER_OTHER)
  #undef STBIR_RETURN_ERROR_AND_ASSERT

  if ( splits <= 0 )
    return 0;

  STBIR_PROFILE_BUILD_FIRST_START( build );

  new_output_subx = resize->output_subx;
  new_output_suby = resize->output_suby;

  // do horizontal clip and scale calcs (an empty region - zero or negative sizes, nothing in bounds - fails the build)
  if ( !stbir__calculate_region_transform( &horizontal.scale_info, resize->output_w, &new_output_subx, resize->output_subw, resize->input_w, resize->input_s0, resize->input_s1, resize->input_s_n0, resize->input_s_n1, resize->input_s_d ) )
    return 0;

  // do vertical clip and scale calcs
  if ( !stbir__calculate_region_transform( &vertical.scale_info, resize->output_h, &new_output_suby, resize->output_subh, resize->input_h, resize->input_t0, resize->input_t1, resize->input_t_n0, resize->input_t_n1, resize->input_t_d ) )
    return 0;

  // if nothing to do, just return
  if ( ( horizontal.scale_info.output_sub_size == 0 ) || ( vertical.scale_info.output_sub_size == 0 ) )
    return 0;

  stbir__set_sampler(&horizontal, resize->horizontal_filter, resize->horizontal_filter_kernel_new, resize->horizontal_filter_support_new, resize->horizontal_filter_radius, resize->horizontal_filter_kernel, resize->horizontal_filter_support, resize->horizontal_edge, &horizontal.scale_info, 1, resize->user_data );
  stbir__get_conservative_extents( &horizontal, &conservative, resize->user_data );
  stbir__set_sampler(&vertical, resize->vertical_filter, resize->vertical_filter_kernel_new, resize->vertical_filter_support_new, resize->vertical_filter_radius, resize->vertical_filter_kernel, resize->vertical_filter_support, resize->vertical_edge, &vertical.scale_info, 0, resize->user_data );

  // the split count can come down in the build (stbir__choose_split_grid): the caller gets the count it can use

  STBIR_PROFILE_BUILD_START( alloc );
  out_info = stbir__alloc_internal_mem_and_build_samplers( &horizontal, &vertical, &conservative, resize->input_pixel_layout_public, resize->output_pixel_layout_public, splits, new_output_subx, new_output_suby, resize->fast_alpha, stbir__pixel_channels[ stbir__pixel_layout_convert_public_to_internal[ resize->input_pixel_layout_public ] ] * stbir__type_size[ resize->input_data_type ], resize->user_data STBIR_ONLY_PROFILE_BUILD_SET_INFO );
  STBIR_PROFILE_BUILD_END( alloc );
  STBIR_PROFILE_BUILD_END( build );

  if ( out_info )
  {
    splits = out_info->splits;
    resize->splits = splits;
    resize->samplers = out_info;
    resize->needs_rebuild = 0;
    #ifdef STBIR_PROFILE
      STBIR_MEMCPY( &out_info->profile, &profile_infod.profile, sizeof( out_info->profile ) );
    #endif

    // update anything that can be changed without recalcing samplers
    stbir__update_info_from_resize( out_info, resize );

    return splits;
  }

  return 0;
}

STBIRDEF void stbir_free_samplers( STBIR_RESIZE * resize )
{
  if ( resize->samplers )
  {
    stbir__free_internal_mem( resize->samplers );
    resize->samplers = 0;
    resize->called_alloc = 0;
  }
}

STBIRDEF int stbir_build_samplers_with_splits( STBIR_RESIZE * resize, int splits )
{
  if ( ( resize->samplers == 0 ) || ( resize->needs_rebuild ) )
  {
    if ( resize->samplers )
      stbir_free_samplers( resize );

    resize->called_alloc = 1;
    return stbir__perform_build( resize, splits );
  }

  STBIR_PROFILE_BUILD_CLEAR( resize->samplers );

  return resize->splits;   // already built: the count that build settled on (a new request isn't applied without a rebuild)
}

STBIRDEF int stbir_build_samplers( STBIR_RESIZE * resize )
{
  return stbir_build_samplers_with_splits( resize, 1 );
}

STBIRDEF int stbir_resize_extended( STBIR_RESIZE * resize )
{
  int result;

  if ( ( resize->samplers == 0 ) || ( resize->needs_rebuild ) )
  {
    int alloc_state = resize->called_alloc;  // remember allocated state

    if ( resize->samplers )
    {
      stbir__free_internal_mem( resize->samplers );
      resize->samplers = 0;
    }

    if ( !stbir_build_samplers( resize ) )
      return 0;   // (an empty resize fails the build too, so it comes out of here as a failure)

    resize->called_alloc = alloc_state;
  }
  else
  {
    // didn't build anything - clear it
    STBIR_PROFILE_BUILD_CLEAR( resize->samplers );
  }

  // do resize
  result = stbir__perform_resize( resize->samplers, 0, resize->splits );

  // if we alloced, then free
  if ( !resize->called_alloc )
  {
    stbir_free_samplers( resize );
    resize->samplers = 0;
  }

  return result;
}

STBIRDEF int stbir_resize_extended_split( STBIR_RESIZE * resize, int split_start, int split_count )
{
  STBIR_ASSERT( resize->samplers );

  // you **must** build samplers first when using split resize (and not change the resize after)
  if ( ( resize->samplers == 0 ) || ( resize->needs_rebuild ) )
    return 0;

  // if we're just doing the whole thing, call full
  if ( ( split_start == -1 ) || ( ( split_start == 0 ) && ( split_count == resize->splits ) ) )
    return stbir_resize_extended( resize );

  if ( ( split_start >= resize->splits ) || ( split_start < 0 ) || ( ( split_start + split_count ) > resize->splits ) || ( split_count <= 0 ) )
    return 0;

  // do resize
  return stbir__perform_resize( resize->samplers, split_start, split_count );
}


static void * stbir_quick_resize_helper( const void *input_pixels , int input_w , int input_h, stbir_int64 input_stride_in_bytes,
                                               void *output_pixels, int output_w, int output_h, stbir_int64 output_stride_in_bytes,
                                               stbir_pixel_layout pixel_layout, stbir_datatype data_type, stbir_edge edge, stbir_filter filter )
{
  STBIR_RESIZE resize;
  stbir_int64 scanline_output_in_bytes;
  stbir_int64 positive_output_stride_in_bytes;
  void * start_ptr;
  void * free_ptr;

  scanline_output_in_bytes = (stbir_int64)output_w * stbir__type_size[ data_type ] * stbir__pixel_channels[ stbir__pixel_layout_convert_public_to_internal[ pixel_layout ] ];
  if ( scanline_output_in_bytes <= 0 )
    return 0;

  // if zero stride, use scanline output
  if ( output_stride_in_bytes == 0 )
    output_stride_in_bytes = scanline_output_in_bytes;

  // abs value for inverted images (negative pitches)
  positive_output_stride_in_bytes = output_stride_in_bytes;
  if ( positive_output_stride_in_bytes < 0 )
    positive_output_stride_in_bytes = -positive_output_stride_in_bytes;

  // is the requested stride smaller than the scanline output? if so, just fail
  if ( positive_output_stride_in_bytes < scanline_output_in_bytes )
    return 0;

  start_ptr = output_pixels;
  free_ptr = 0;  // no free pointer, since they passed buffer to use

  // did they pass a zero for the dest? if so, allocate the buffer
  if ( output_pixels == 0 )
  {
    size_t size;
    char * ptr;
  
    size = (size_t)positive_output_stride_in_bytes * (size_t)output_h;
    if ( size == 0 )
      return 0;

    ptr = (char*) STBIR_MALLOC( size, 0 );
    if ( ptr == 0 )
      return 0;

    free_ptr = ptr;

    // point at the last scanline, if they requested a flipped image
    if ( output_stride_in_bytes < 0 )
      start_ptr = ptr + ( (size_t)positive_output_stride_in_bytes * (size_t)( output_h - 1 ) );
    else
      start_ptr = ptr;
  }

  // ok, now do the resize
  stbir_resize_init( &resize,
                     input_pixels,  input_w,  input_h,  input_stride_in_bytes,
                     start_ptr, output_w, output_h, output_stride_in_bytes,
                     pixel_layout, data_type );

  resize.horizontal_edge = edge;
  resize.vertical_edge = edge;
  resize.horizontal_filter = filter;
  resize.vertical_filter = filter;

  if ( !stbir_resize_extended( &resize ) )
  {
    if ( free_ptr )
      STBIR_FREE( free_ptr, 0 );
    return 0;
  }

  return (free_ptr) ? free_ptr : start_ptr;
}


STBIRDEF unsigned char * stbir_resize_uint8_linear( const unsigned char *input_pixels , int input_w , int input_h, stbir_int64 input_stride_in_bytes,
                                                          unsigned char *output_pixels, int output_w, int output_h, stbir_int64 output_stride_in_bytes,
                                                          stbir_pixel_layout pixel_layout )
{
  return (unsigned char *) stbir_quick_resize_helper( input_pixels , input_w , input_h, input_stride_in_bytes, 
                                                      output_pixels, output_w, output_h, output_stride_in_bytes, 
                                                      pixel_layout, STBIR_TYPE_UINT8, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT );
}

STBIRDEF unsigned char * stbir_resize_uint8_srgb( const unsigned char *input_pixels , int input_w , int input_h, stbir_int64 input_stride_in_bytes,
                                                        unsigned char *output_pixels, int output_w, int output_h, stbir_int64 output_stride_in_bytes,
                                                        stbir_pixel_layout pixel_layout )
{
  return (unsigned char *) stbir_quick_resize_helper( input_pixels , input_w , input_h, input_stride_in_bytes, 
                                                      output_pixels, output_w, output_h, output_stride_in_bytes, 
                                                      pixel_layout, STBIR_TYPE_UINT8_SRGB, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT );
}


STBIRDEF float * stbir_resize_float_linear( const float *input_pixels , int input_w , int input_h, stbir_int64 input_stride_in_bytes,
                                                  float *output_pixels, int output_w, int output_h, stbir_int64 output_stride_in_bytes,
                                                  stbir_pixel_layout pixel_layout )
{
  return (float *) stbir_quick_resize_helper( input_pixels , input_w , input_h, input_stride_in_bytes, 
                                              output_pixels, output_w, output_h, output_stride_in_bytes, 
                                              pixel_layout, STBIR_TYPE_FLOAT, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT  );
}


STBIRDEF void * stbir_resize( const void *input_pixels , int input_w , int input_h, stbir_int64 input_stride_in_bytes,
                                    void *output_pixels, int output_w, int output_h, stbir_int64 output_stride_in_bytes,
                                    stbir_pixel_layout pixel_layout, stbir_datatype data_type,
                                    stbir_edge edge, stbir_filter filter )
{
  return (void *) stbir_quick_resize_helper( input_pixels , input_w , input_h, input_stride_in_bytes, 
                                             output_pixels, output_w, output_h, output_stride_in_bytes, 
                                             pixel_layout, data_type, edge, filter  );
}

#ifdef STBIR_PROFILE

STBIRDEF void stbir_resize_build_profile_info( STBIR_PROFILE_INFO * info, STBIR_RESIZE const * resize )
{
  static char const * bdescriptions[6] = { "Building", "Allocating", "Horizontal sampler", "Vertical sampler", "Coefficient cleanup", "Scatter coefficients" } ;
  stbir__info* samp = resize->samplers;
  int i;

  for( i = 0 ; i < (int)STBIR__ARRAY_SIZE( bdescriptions ) ; i++)
    info->clocks[i] = samp->profile.array[i+1];

  info->total_clocks = samp->profile.named.total;
  info->descriptions = bdescriptions;
  info->count = STBIR__ARRAY_SIZE( bdescriptions );
}

STBIRDEF void stbir_resize_split_profile_info( STBIR_PROFILE_INFO * info, STBIR_RESIZE const * resize, int split_start, int split_count )
{
  static char const * descriptions[7] = { "Looping", "Vertical sampling", "Horizontal sampling", "Scanline input", "Scanline output", "Alpha weighting", "Alpha unweighting" };
  stbir__per_split_info * split_info;
  int s, i;

  if ( split_start == -1 )
  {
    split_start = 0;
    split_count = resize->samplers->splits;
  }

  if ( ( split_start >= resize->splits ) || ( split_start < 0 ) || ( ( split_start + split_count ) > resize->splits ) || ( split_count <= 0 ) )
  {
    info->total_clocks = 0;
    info->descriptions = 0;
    info->count = 0;
    return;
  }

  split_info = resize->samplers->split_info + split_start;

  // sum up the profile from all the splits
  for( i = 0 ; i < (int)STBIR__ARRAY_SIZE( descriptions ) ; i++ )
  {
    stbir_uint64 sum = 0;
    for( s = 0 ; s < split_count ; s++ )
      sum += split_info[s].profile.array[i+1];
    info->clocks[i] = sum;
  }

  info->total_clocks = 0;
  for( s = 0 ; s < split_count ; s++ )
    info->total_clocks += split_info[s].profile.named.total;
  info->descriptions = descriptions;
  info->count = STBIR__ARRAY_SIZE( descriptions );
}

STBIRDEF void stbir_resize_extended_profile_info( STBIR_PROFILE_INFO * info, STBIR_RESIZE const * resize )
{
  stbir_resize_split_profile_info( info, resize, -1, 0 );
}

#endif // STBIR_PROFILE

#undef STBIR_BGR
#undef STBIR_1CHANNEL
#undef STBIR_2CHANNEL
#undef STBIR_RGB
#undef STBIR_RGBA
#undef STBIR_4CHANNEL
#undef STBIR_BGRA
#undef STBIR_ARGB
#undef STBIR_ABGR
#undef STBIR_RA
#undef STBIR_AR
#undef STBIR_RGBA_PM
#undef STBIR_BGRA_PM
#undef STBIR_ARGB_PM
#undef STBIR_ABGR_PM
#undef STBIR_RA_PM
#undef STBIR_AR_PM

#endif // STB_IMAGE_RESIZE_IMPLEMENTATION

#else  // STB_IMAGE_RESIZE_HORIZONTALS&STB_IMAGE_RESIZE_DO_VERTICALS

// we reinclude the header file to define all the horizontal functions
//   specializing each function for the number of coeffs is 20-40% faster *OVERALL*

// by including the header file again this way, we can still debug the functions

#define STBIR_strs_join2( start, mid, end ) start##mid##end
#define STBIR_strs_join1( start, mid, end ) STBIR_strs_join2( start, mid, end )

#define STBIR_strs_join24( start, mid1, mid2, end ) start##mid1##mid2##end
#define STBIR_strs_join14( start, mid1, mid2, end ) STBIR_strs_join24( start, mid1, mid2, end )

#ifdef STB_IMAGE_RESIZE_DO_CODERS

#ifdef stbir__decode_suffix
#define STBIR__CODER_NAME( name ) STBIR_strs_join1( name, _, stbir__decode_suffix )
#else
#define STBIR__CODER_NAME( name ) name
#endif

#ifdef stbir__decode_swizzle
#define stbir__decode_simdf8_flip(reg) STBIR_strs_join1( STBIR_strs_join1( STBIR_strs_join1( STBIR_strs_join1( stbir__simdf8_0123to,stbir__decode_order0,stbir__decode_order1),stbir__decode_order2,stbir__decode_order3),stbir__decode_order0,stbir__decode_order1),stbir__decode_order2,stbir__decode_order3)(reg, reg)
#define stbir__decode_simdf4_flip(reg) STBIR_strs_join1( STBIR_strs_join1( stbir__simdf_0123to,stbir__decode_order0,stbir__decode_order1),stbir__decode_order2,stbir__decode_order3)(reg, reg)
#define stbir__encode_simdf8_unflip(reg) STBIR_strs_join1( STBIR_strs_join1( STBIR_strs_join1( STBIR_strs_join1( stbir__simdf8_0123to,stbir__encode_order0,stbir__encode_order1),stbir__encode_order2,stbir__encode_order3),stbir__encode_order0,stbir__encode_order1),stbir__encode_order2,stbir__encode_order3)(reg, reg)
#define stbir__encode_simdf4_unflip(reg) STBIR_strs_join1( STBIR_strs_join1( stbir__simdf_0123to,stbir__encode_order0,stbir__encode_order1),stbir__encode_order2,stbir__encode_order3)(reg, reg)
#define stbir__encode_simdf16_unflip(reg) STBIR_strs_join1( STBIR_strs_join1( stbir__simdf16_0123to,stbir__encode_order0,stbir__encode_order1),stbir__encode_order2,stbir__encode_order3)(reg, reg)   // (one lane's order, the shuffle repeats it)
// u8 and u16 inputs are swizzled as bytes before the expansion when there is a byte shuffle
#ifdef STBIR_SIMD_BYTE_SHUFFLE
#define STBIR__SWZ8( p )        ( ( (p)*4 + stbir__decode_order0 ) | ( ( (p)*4 + stbir__decode_order1 ) << 8 ) | ( ( (p)*4 + stbir__decode_order2 ) << 16 ) | ( ( (p)*4 + stbir__decode_order3 ) << 24 ) )
#define STBIR__SWZ16( p, a, b ) ( ( 2*((p)*4 + (a)) ) | ( ( 2*((p)*4 + (a)) + 1 ) << 8 ) | ( ( 2*((p)*4 + (b)) ) << 16 ) | ( ( 2*((p)*4 + (b)) + 1 ) << 24 ) )
static STBIR__SIMDI_CONST4( STBIR__CODER_NAME( stbir__decode_bytes_swizzle ), STBIR__SWZ8(0), STBIR__SWZ8(1), STBIR__SWZ8(2), STBIR__SWZ8(3) );
static STBIR__SIMDI_CONST4( STBIR__CODER_NAME( stbir__decode_words_swizzle ), STBIR__SWZ16(0,stbir__decode_order0,stbir__decode_order1), STBIR__SWZ16(0,stbir__decode_order2,stbir__decode_order3), STBIR__SWZ16(1,stbir__decode_order0,stbir__decode_order1), STBIR__SWZ16(1,stbir__decode_order2,stbir__decode_order3) );
#define stbir__decode_simdi_flip_u8( ireg )  stbir__simdi_shuffle_bytes( ireg, ireg, STBIR__CONSTI( STBIR__CODER_NAME( stbir__decode_bytes_swizzle ) ) )
#define stbir__decode_simdi_flip_u16( ireg ) stbir__simdi_shuffle_bytes( ireg, ireg, STBIR__CONSTI( STBIR__CODER_NAME( stbir__decode_words_swizzle ) ) )
#define stbir__decode_simdf8_flip_i( reg )
#define stbir__decode_simdf4_flip_i( reg )
#define stbir__decode_simdf16_flip_i( reg )
#ifdef STBIR_SIMD16
#define stbir__decode_simdi8_flip_u16( ireg ) stbir__simdi8_shuffle_bytes( ireg, ireg, STBIR__CONSTI( STBIR__CODER_NAME( stbir__decode_words_swizzle ) ) )   // (the 16-wide u16 decoder loads 32 bytes: the same table in both lanes)
#endif
#else
#define stbir__decode_simdi_flip_u8( ireg )
#define stbir__decode_simdi_flip_u16( ireg )
#define stbir__decode_simdi8_flip_u16( ireg )
#define stbir__decode_simdf8_flip_i( reg ) stbir__decode_simdf8_flip( reg )
#define stbir__decode_simdf4_flip_i( reg ) stbir__decode_simdf4_flip( reg )
#define stbir__decode_simdf16_flip_i( reg ) STBIR_strs_join1( STBIR_strs_join1( stbir__simdf16_0123to,stbir__decode_order0,stbir__decode_order1),stbir__decode_order2,stbir__decode_order3)(reg, reg)
#endif
#else
#define stbir__decode_order0 0
#define stbir__decode_order1 1
#define stbir__decode_order2 2
#define stbir__decode_order3 3
#define stbir__encode_order0 0
#define stbir__encode_order1 1
#define stbir__encode_order2 2
#define stbir__encode_order3 3
#define stbir__decode_simdf8_flip(reg)
#define stbir__decode_simdf4_flip(reg)
#define stbir__decode_simdi_flip_u8( ireg )
#define stbir__decode_simdi_flip_u16( ireg )
#define stbir__decode_simdi8_flip_u16( ireg )
#define stbir__decode_simdf8_flip_i( reg )
#define stbir__decode_simdf4_flip_i( reg )
#define stbir__decode_simdf16_flip_i( reg )
#define stbir__encode_simdf8_unflip(reg)
#define stbir__encode_simdf4_unflip(reg)
#define stbir__encode_simdf16_unflip(reg)
#endif

#if defined(STBIR_SIMD16)
#define stbir__encode_simdfX_unflip  stbir__encode_simdf16_unflip
#elif defined(STBIR_SIMD8)
#define stbir__encode_simdfX_unflip  stbir__encode_simdf8_unflip
#else
#define stbir__encode_simdfX_unflip  stbir__encode_simdf4_unflip
#endif

static float * STBIR__CODER_NAME( stbir__decode_uint8_linear_scaled )( float * decodep, ptrdiff_t width_times_channels, void const * inputp )
{
  float STBIR_STREAMOUT_PTR( * ) decode = decodep;
  float * decode_end = (float*) decode + width_times_channels;
  unsigned char const * input = (unsigned char const*)inputp;

  #ifdef STBIR_SIMD
  unsigned char const * end_input_m16 = input + width_times_channels - 16;
  if ( width_times_channels >= 16 )
  {
    decode_end -= 16;
    STBIR_NO_UNROLL_LOOP_START_INF_FOR
    for(;;)
    {
      #if defined(STBIR_SIMD16)
      stbir__simdi i; stbir__simdi16 o;
      stbir__simdf16 of;
      STBIR_NO_UNROLL(decode);
      stbir__simdi_load( i, input );
      stbir__decode_simdi_flip_u8( i );
      stbir__simdi16_expand_u8_to_u32( o, i );
      stbir__simdi16_convert_i32_to_float( of, o );
      stbir__simdf16_mult( of, of, STBIR_max_uint8_as_float_inverted16 );
      stbir__decode_simdf16_flip_i( of );
      stbir__simdf16_store( decode, of );
      #elif defined(STBIR_SIMD8)
      stbir__simdi i; stbir__simdi8 o0,o1;
      stbir__simdf8 of0, of1;
      STBIR_NO_UNROLL(decode);
      stbir__simdi_load( i, input );
      stbir__decode_simdi_flip_u8( i );
      stbir__simdi8_expand_u8_to_u32( o0, o1, i );
      stbir__simdi8_convert_i32_to_float( of0, o0 );
      stbir__simdi8_convert_i32_to_float( of1, o1 );
      stbir__simdf8_mult( of0, of0, STBIR_max_uint8_as_float_inverted8);
      stbir__simdf8_mult( of1, of1, STBIR_max_uint8_as_float_inverted8);
      stbir__decode_simdf8_flip_i( of0 );
      stbir__decode_simdf8_flip_i( of1 );
      stbir__simdf8_store( decode + 0, of0 );
      stbir__simdf8_store( decode + 8, of1 );
      #else
      stbir__simdi i, o0, o1, o2, o3;
      stbir__simdf of0, of1, of2, of3;
      STBIR_NO_UNROLL(decode);
      stbir__simdi_load( i, input );
      stbir__decode_simdi_flip_u8( i );
      stbir__simdi_expand_u8_to_u32( o0,o1,o2,o3,i);
      stbir__simdi_convert_i32_to_float( of0, o0 );
      stbir__simdi_convert_i32_to_float( of1, o1 );
      stbir__simdi_convert_i32_to_float( of2, o2 );
      stbir__simdi_convert_i32_to_float( of3, o3 );
      stbir__simdf_mult( of0, of0, STBIR__CONSTF(STBIR_max_uint8_as_float_inverted) );
      stbir__simdf_mult( of1, of1, STBIR__CONSTF(STBIR_max_uint8_as_float_inverted) );
      stbir__simdf_mult( of2, of2, STBIR__CONSTF(STBIR_max_uint8_as_float_inverted) );
      stbir__simdf_mult( of3, of3, STBIR__CONSTF(STBIR_max_uint8_as_float_inverted) );
      stbir__decode_simdf4_flip_i( of0 );
      stbir__decode_simdf4_flip_i( of1 );
      stbir__decode_simdf4_flip_i( of2 );
      stbir__decode_simdf4_flip_i( of3 );
      stbir__simdf_store( decode + 0,  of0 );
      stbir__simdf_store( decode + 4,  of1 );
      stbir__simdf_store( decode + 8,  of2 );
      stbir__simdf_store( decode + 12, of3 );
      #endif
      decode += 16;
      input += 16;
      if ( decode <= decode_end )
        continue;
      if ( decode == ( decode_end + 16 ) )
        break;
      decode = decode_end; // backup and do last couple
      input = end_input_m16;
    }
    return decode_end + 16;
  }
  #endif

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  decode += 4;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( decode <= decode_end )
  {
    STBIR_SIMD_NO_UNROLL(decode);
    decode[0-4] = ((float)(input[stbir__decode_order0])) * stbir__max_uint8_as_float_inverted;
    decode[1-4] = ((float)(input[stbir__decode_order1])) * stbir__max_uint8_as_float_inverted;
    decode[2-4] = ((float)(input[stbir__decode_order2])) * stbir__max_uint8_as_float_inverted;
    decode[3-4] = ((float)(input[stbir__decode_order3])) * stbir__max_uint8_as_float_inverted;
    decode += 4;
    input += 4;
  }
  decode -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( decode < decode_end )
  {
    STBIR_NO_UNROLL(decode);
    decode[0] = ((float)(input[stbir__decode_order0])) * stbir__max_uint8_as_float_inverted;
    #if stbir__coder_min_num >= 2
    decode[1] = ((float)(input[stbir__decode_order1])) * stbir__max_uint8_as_float_inverted;
    #endif
    #if stbir__coder_min_num >= 3
    decode[2] = ((float)(input[stbir__decode_order2])) * stbir__max_uint8_as_float_inverted;
    #endif
    decode += stbir__coder_min_num;
    input += stbir__coder_min_num;
  }
  #endif

  return decode_end;
}

static void STBIR__CODER_NAME( stbir__encode_uint8_linear_scaled )( void * outputp, ptrdiff_t width_times_channels, float const * encode )
{
  unsigned char STBIR_SIMD_STREAMOUT_PTR( * ) output = (unsigned char *) outputp;
  unsigned char * end_output = ( (unsigned char *) output ) + width_times_channels;

  #ifdef STBIR_SIMD
  // double width blocks while they fit, then the four-at-a-time and scalar loops below finish the row. (the encode can
  //   be in place - with an output callback it converts into the float buffer - so no backing up to redo the last block)
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while ( ( end_output - output ) >= stbir__simdfX_float_count*2 )
  {
    stbir__simdfX e0, e1;
    STBIR_SIMD_NO_UNROLL(encode);
    stbir__simdfX_madd_mem( e0, STBIR_simd_point5X, STBIR_max_uint8_as_floatX, encode );
    stbir__simdfX_madd_mem( e1, STBIR_simd_point5X, STBIR_max_uint8_as_floatX, encode+stbir__simdfX_float_count );
    stbir__encode_simdfX_unflip( e0 );
    stbir__encode_simdfX_unflip( e1 );
    #if defined(STBIR_SIMD16)
    {
      stbir__simdi8 i;
      stbir__simdf16_pack_to_32bytes( i, e0, e1 );
      stbir__simdi8_store( output, i );
    }
    #elif defined(STBIR_SIMD8)
    {
      stbir__simdi i;
      stbir__simdf8_pack_to_16bytes( i, e0, e1 );
      stbir__simdi_store( output, i );
    }
    #else
    {
      stbir__simdi i;
      stbir__simdf_pack_to_8bytes( i, e0, e1 );
      stbir__simdi_store2( output, i );
    }
    #endif
    encode += stbir__simdfX_float_count*2;
    output += stbir__simdfX_float_count*2;
  }

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  STBIR_NO_UNROLL_LOOP_START
  while( output <= end_output )
  {
    stbir__simdf e0;
    stbir__simdi i0;
    STBIR_NO_UNROLL(encode);
    stbir__simdf_load( e0, encode );
    stbir__simdf_madd( e0, STBIR__CONSTF(STBIR_simd_point5), STBIR__CONSTF(STBIR_max_uint8_as_float), e0 );
    stbir__encode_simdf4_unflip( e0 );
    stbir__simdf_pack_to_8bytes( i0, e0, e0 );  // only use first 4
    *(int*)(output-4) = stbir__simdi_to_int( i0 );
    output += 4;
    encode += 4;
  }
  output -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( output < end_output )
  {
    // read the whole pixel before writing any of it - the encode can be in place, and a swizzle reads a channel after
    //   the one it lands on has been overwritten
    stbir__simdf e0;
    #if stbir__coder_min_num >= 2
    stbir__simdf e1;
    #endif
    #if stbir__coder_min_num >= 3
    stbir__simdf e2;
    #endif
    STBIR_NO_UNROLL(encode);
    stbir__simdf_madd1_mem( e0, STBIR__CONSTF(STBIR_simd_point5), STBIR__CONSTF(STBIR_max_uint8_as_float), encode+stbir__encode_order0 );
    #if stbir__coder_min_num >= 2
    stbir__simdf_madd1_mem( e1, STBIR__CONSTF(STBIR_simd_point5), STBIR__CONSTF(STBIR_max_uint8_as_float), encode+stbir__encode_order1 );
    #endif
    #if stbir__coder_min_num >= 3
    stbir__simdf_madd1_mem( e2, STBIR__CONSTF(STBIR_simd_point5), STBIR__CONSTF(STBIR_max_uint8_as_float), encode+stbir__encode_order2 );
    #endif
    output[0] = stbir__simdf_convert_float_to_uint8( e0 );
    #if stbir__coder_min_num >= 2
    output[1] = stbir__simdf_convert_float_to_uint8( e1 );
    #endif
    #if stbir__coder_min_num >= 3
    output[2] = stbir__simdf_convert_float_to_uint8( e2 );
    #endif
    output += stbir__coder_min_num;
    encode += stbir__coder_min_num;
  }
  #endif

  #else

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  while( output <= end_output )
  {
    float f0 = encode[stbir__encode_order0] * stbir__max_uint8_as_float + 0.5f;
    float f1 = encode[stbir__encode_order1] * stbir__max_uint8_as_float + 0.5f;
    float f2 = encode[stbir__encode_order2] * stbir__max_uint8_as_float + 0.5f;
    float f3 = encode[stbir__encode_order3] * stbir__max_uint8_as_float + 0.5f;
    STBIR_CLAMP(f0, 0, 255);
    STBIR_CLAMP(f1, 0, 255);
    STBIR_CLAMP(f2, 0, 255);
    STBIR_CLAMP(f3, 0, 255);
    output[0-4] = (unsigned char)f0;
    output[1-4] = (unsigned char)f1;
    output[2-4] = (unsigned char)f2;
    output[3-4] = (unsigned char)f3;
    output += 4;
    encode += 4;
  }
  output -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( output < end_output )
  {
    float f0 = encode[stbir__encode_order0] * stbir__max_uint8_as_float + 0.5f;
    #if stbir__coder_min_num >= 2
    float f1 = encode[stbir__encode_order1] * stbir__max_uint8_as_float + 0.5f;
    #endif
    #if stbir__coder_min_num >= 3
    float f2 = encode[stbir__encode_order2] * stbir__max_uint8_as_float + 0.5f;
    #endif
    STBIR_NO_UNROLL(encode);
    STBIR_CLAMP(f0, 0, 255);
    output[0] = (unsigned char)f0;
    #if stbir__coder_min_num >= 2
    STBIR_CLAMP(f1, 0, 255);
    output[1] = (unsigned char)f1;
    #endif
    #if stbir__coder_min_num >= 3
    STBIR_CLAMP(f2, 0, 255);
    output[2] = (unsigned char)f2;
    #endif
    output += stbir__coder_min_num;
    encode += stbir__coder_min_num;
  }
  #endif
  #endif
}

static float * STBIR__CODER_NAME(stbir__decode_uint8_linear)( float * decodep, ptrdiff_t width_times_channels, void const * inputp )
{
  float STBIR_STREAMOUT_PTR( * ) decode = decodep;
  float * decode_end = (float*) decode + width_times_channels;
  unsigned char const * input = (unsigned char const*)inputp;

  #ifdef STBIR_SIMD
  unsigned char const * end_input_m16 = input + width_times_channels - 16;
  if ( width_times_channels >= 16 )
  {
    decode_end -= 16;
    STBIR_NO_UNROLL_LOOP_START_INF_FOR
    for(;;)
    {
      #if defined(STBIR_SIMD16)
      stbir__simdi i; stbir__simdi16 o;
      stbir__simdf16 of;
      STBIR_NO_UNROLL(decode);
      stbir__simdi_load( i, input );
      stbir__decode_simdi_flip_u8( i );
      stbir__simdi16_expand_u8_to_u32( o, i );
      stbir__simdi16_convert_i32_to_float( of, o );
      stbir__decode_simdf16_flip_i( of );
      stbir__simdf16_store( decode, of );
      #elif defined(STBIR_SIMD8)
      stbir__simdi i; stbir__simdi8 o0,o1;
      stbir__simdf8 of0, of1;
      STBIR_NO_UNROLL(decode);
      stbir__simdi_load( i, input );
      stbir__decode_simdi_flip_u8( i );
      stbir__simdi8_expand_u8_to_u32( o0, o1, i );
      stbir__simdi8_convert_i32_to_float( of0, o0 );
      stbir__simdi8_convert_i32_to_float( of1, o1 );
      stbir__decode_simdf8_flip_i( of0 );
      stbir__decode_simdf8_flip_i( of1 );
      stbir__simdf8_store( decode + 0, of0 );
      stbir__simdf8_store( decode + 8, of1 );
      #else
      stbir__simdi i, o0, o1, o2, o3;
      stbir__simdf of0, of1, of2, of3;
      STBIR_NO_UNROLL(decode);
      stbir__simdi_load( i, input );
      stbir__decode_simdi_flip_u8( i );
      stbir__simdi_expand_u8_to_u32( o0,o1,o2,o3,i);
      stbir__simdi_convert_i32_to_float( of0, o0 );
      stbir__simdi_convert_i32_to_float( of1, o1 );
      stbir__simdi_convert_i32_to_float( of2, o2 );
      stbir__simdi_convert_i32_to_float( of3, o3 );
      stbir__decode_simdf4_flip_i( of0 );
      stbir__decode_simdf4_flip_i( of1 );
      stbir__decode_simdf4_flip_i( of2 );
      stbir__decode_simdf4_flip_i( of3 );
      stbir__simdf_store( decode + 0,  of0 );
      stbir__simdf_store( decode + 4,  of1 );
      stbir__simdf_store( decode + 8,  of2 );
      stbir__simdf_store( decode + 12, of3 );
#endif
      decode += 16;
      input += 16;
      if ( decode <= decode_end )
        continue;
      if ( decode == ( decode_end + 16 ) )
        break;
      decode = decode_end; // backup and do last couple
      input = end_input_m16;
    }
    return decode_end + 16;
  }
  #endif

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  decode += 4;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( decode <= decode_end )
  {
    STBIR_SIMD_NO_UNROLL(decode);
    decode[0-4] = ((float)(input[stbir__decode_order0]));
    decode[1-4] = ((float)(input[stbir__decode_order1]));
    decode[2-4] = ((float)(input[stbir__decode_order2]));
    decode[3-4] = ((float)(input[stbir__decode_order3]));
    decode += 4;
    input += 4;
  }
  decode -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( decode < decode_end )
  {
    STBIR_NO_UNROLL(decode);
    decode[0] = ((float)(input[stbir__decode_order0]));
    #if stbir__coder_min_num >= 2
    decode[1] = ((float)(input[stbir__decode_order1]));
    #endif
    #if stbir__coder_min_num >= 3
    decode[2] = ((float)(input[stbir__decode_order2]));
    #endif
    decode += stbir__coder_min_num;
    input += stbir__coder_min_num;
  }
  #endif
  return decode_end;
}

static void STBIR__CODER_NAME( stbir__encode_uint8_linear )( void * outputp, ptrdiff_t width_times_channels, float const * encode )
{
  unsigned char STBIR_SIMD_STREAMOUT_PTR( * ) output = (unsigned char *) outputp;
  unsigned char * end_output = ( (unsigned char *) output ) + width_times_channels;

  #ifdef STBIR_SIMD
  // double width blocks while they fit, then the four-at-a-time and scalar loops below finish the row. (the encode can
  //   be in place - with an output callback it converts into the float buffer - so no backing up to redo the last block)
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while ( ( end_output - output ) >= stbir__simdfX_float_count*2 )
  {
    stbir__simdfX e0, e1;
    STBIR_SIMD_NO_UNROLL(encode);
    stbir__simdfX_add_mem( e0, STBIR_simd_point5X, encode );
    stbir__simdfX_add_mem( e1, STBIR_simd_point5X, encode+stbir__simdfX_float_count );
    stbir__encode_simdfX_unflip( e0 );
    stbir__encode_simdfX_unflip( e1 );
    #if defined(STBIR_SIMD16)
    {
      stbir__simdi8 i;
      stbir__simdf16_pack_to_32bytes( i, e0, e1 );
      stbir__simdi8_store( output, i );
    }
    #elif defined(STBIR_SIMD8)
    {
      stbir__simdi i;
      stbir__simdf8_pack_to_16bytes( i, e0, e1 );
      stbir__simdi_store( output, i );
    }
    #else
    {
      stbir__simdi i;
      stbir__simdf_pack_to_8bytes( i, e0, e1 );
      stbir__simdi_store2( output, i );
    }
    #endif
    encode += stbir__simdfX_float_count*2;
    output += stbir__simdfX_float_count*2;
  }

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  STBIR_NO_UNROLL_LOOP_START
  while( output <= end_output )
  {
    stbir__simdf e0;
    stbir__simdi i0;
    STBIR_NO_UNROLL(encode);
    stbir__simdf_load( e0, encode );
    stbir__simdf_add( e0, STBIR__CONSTF(STBIR_simd_point5), e0 );
    stbir__encode_simdf4_unflip( e0 );
    stbir__simdf_pack_to_8bytes( i0, e0, e0 );  // only use first 4
    *(int*)(output-4) = stbir__simdi_to_int( i0 );
    output += 4;
    encode += 4;
  }
  output -= 4;
  #endif

  #else

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  while( output <= end_output )
  {
    float f0 = encode[stbir__encode_order0] + 0.5f;
    float f1 = encode[stbir__encode_order1] + 0.5f;
    float f2 = encode[stbir__encode_order2] + 0.5f;
    float f3 = encode[stbir__encode_order3] + 0.5f;
    STBIR_CLAMP(f0, 0, 255);
    STBIR_CLAMP(f1, 0, 255);
    STBIR_CLAMP(f2, 0, 255);
    STBIR_CLAMP(f3, 0, 255);
    output[0-4] = (unsigned char)f0;
    output[1-4] = (unsigned char)f1;
    output[2-4] = (unsigned char)f2;
    output[3-4] = (unsigned char)f3;
    output += 4;
    encode += 4;
  }
  output -= 4;
  #endif

  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( output < end_output )
  {
    float f0 = encode[stbir__encode_order0] + 0.5f;
    #if stbir__coder_min_num >= 2
    float f1 = encode[stbir__encode_order1] + 0.5f;
    #endif
    #if stbir__coder_min_num >= 3
    float f2 = encode[stbir__encode_order2] + 0.5f;
    #endif
    STBIR_NO_UNROLL(encode);
    STBIR_CLAMP(f0, 0, 255);
    output[0] = (unsigned char)f0;
    #if stbir__coder_min_num >= 2
    STBIR_CLAMP(f1, 0, 255);
    output[1] = (unsigned char)f1;
    #endif
    #if stbir__coder_min_num >= 3
    STBIR_CLAMP(f2, 0, 255);
    output[2] = (unsigned char)f2;
    #endif
    output += stbir__coder_min_num;
    encode += stbir__coder_min_num;
  }
  #endif
}

static float * STBIR__CODER_NAME(stbir__decode_uint8_srgb)( float * decodep, ptrdiff_t width_times_channels, void const * inputp )
{
  float STBIR_STREAMOUT_PTR( * ) decode = decodep;
  float * decode_end = (float*) decode + width_times_channels;
  unsigned char const * input = (unsigned char const *)inputp;

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  decode += 4;
  while( decode <= decode_end )
  {
    decode[0-4] = stbir__srgb_uchar_to_linear_float[ input[ stbir__decode_order0 ] ];
    decode[1-4] = stbir__srgb_uchar_to_linear_float[ input[ stbir__decode_order1 ] ];
    decode[2-4] = stbir__srgb_uchar_to_linear_float[ input[ stbir__decode_order2 ] ];
    decode[3-4] = stbir__srgb_uchar_to_linear_float[ input[ stbir__decode_order3 ] ];
    decode += 4;
    input += 4;
  }
  decode -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( decode < decode_end )
  {
    STBIR_NO_UNROLL(decode);
    decode[0] = stbir__srgb_uchar_to_linear_float[ input[ stbir__decode_order0 ] ];
    #if stbir__coder_min_num >= 2
    decode[1] = stbir__srgb_uchar_to_linear_float[ input[ stbir__decode_order1 ] ];
    #endif
    #if stbir__coder_min_num >= 3
    decode[2] = stbir__srgb_uchar_to_linear_float[ input[ stbir__decode_order2 ] ];
    #endif
    decode += stbir__coder_min_num;
    input += stbir__coder_min_num;
  }
  #endif
  return decode_end;
}

#define stbir__min_max_shift20( i, f ) \
  do { \
    stbir__simdf_max( f, f, stbir_simdf_casti(STBIR__CONSTI( STBIR_almost_zero )) ); \
    stbir__simdf_min( f, f, stbir_simdf_casti(STBIR__CONSTI( STBIR_almost_one  )) ); \
    stbir__simdi_32shr( i, stbir_simdi_castf( f ), 20 ); \
  } while(0)

#define stbir__scale_and_convert( i, f ) \
  do { \
    stbir__simdf_madd( f, STBIR__CONSTF( STBIR_simd_point5 ), STBIR__CONSTF( STBIR_max_uint8_as_float ), f ); \
    stbir__simdf_max( f, f, stbir__simdf_zeroP() ); \
    stbir__simdf_min( f, f, STBIR__CONSTF( STBIR_max_uint8_as_float ) ); \
    stbir__simdf_convert_float_to_i32( i, f ); \
  } while(0)

#define stbir__linear_to_srgb_finish( i, f ) \
{ \
    stbir__simdi temp;  \
    stbir__simdi_32shr( temp, stbir_simdi_castf( f ), 12 ) ; \
    stbir__simdi_and( temp, temp, STBIR__CONSTI(STBIR_mantissa_mask) ); \
    stbir__simdi_or( temp, temp, STBIR__CONSTI(STBIR_topscale) ); \
    stbir__simdi_16madd( i, i, temp ); \
    stbir__simdi_32shr( i, i, 16 ); \
}

// look up sRGB bytes for 4 float lanes per register (indexed by the float bits shifted right 20, see fp32_to_srgb8_tab4)
#define stbir__simdi_srgb_lookup_one( v, temp )           \
  do {                                                    \
    temp.m128i_i128 = v;                                  \
    temp.m128i_u32[0] = STBIR__SRGB8_TAB4( temp.m128i_i32[0] ); \
    temp.m128i_u32[1] = STBIR__SRGB8_TAB4( temp.m128i_i32[1] ); \
    temp.m128i_u32[2] = STBIR__SRGB8_TAB4( temp.m128i_i32[2] ); \
    temp.m128i_u32[3] = STBIR__SRGB8_TAB4( temp.m128i_i32[3] ); \
    v = temp.m128i_i128;                                  \
  } while(0)

#define stbir__simdi_srgb_lookup2( v0,v1 )                \
  do {                                                    \
    stbir__simdi_u32 temp0,temp1;                         \
    stbir__simdi_srgb_lookup_one( v0, temp0 );            \
    stbir__simdi_srgb_lookup_one( v1, temp1 );            \
  } while(0)

#define stbir__simdi_srgb_lookup3( v0,v1,v2 )             \
  do {                                                    \
    stbir__simdi_u32 temp0,temp1,temp2;                   \
    stbir__simdi_srgb_lookup_one( v0, temp0 );            \
    stbir__simdi_srgb_lookup_one( v1, temp1 );            \
    stbir__simdi_srgb_lookup_one( v2, temp2 );            \
  } while(0)

#define stbir__simdi_srgb_lookup4( v0,v1,v2,v3 )          \
  do {                                                    \
    stbir__simdi_u32 temp0,temp1,temp2,temp3;             \
    stbir__simdi_srgb_lookup_one( v0, temp0 );            \
    stbir__simdi_srgb_lookup_one( v1, temp1 );            \
    stbir__simdi_srgb_lookup_one( v2, temp2 );            \
    stbir__simdi_srgb_lookup_one( v3, temp3 );            \
  } while(0)

static void STBIR__CODER_NAME( stbir__encode_uint8_srgb )( void * outputp, ptrdiff_t width_times_channels, float const * encode )
{
  unsigned char STBIR_SIMD_STREAMOUT_PTR( * ) output = (unsigned char*) outputp;
  unsigned char * end_output = ( (unsigned char*) output ) + width_times_channels;

  #ifdef STBIR_SIMD

  // double width blocks while they fit, then the four-at-a-time and scalar loops below finish the row. (the encode can
  //   be in place - with an output callback it converts into the float buffer - so no backing up to redo the last block)
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while ( ( end_output - output ) >= 16 )
  {
    stbir__simdf f0, f1, f2, f3;
    stbir__simdi i0, i1, i2, i3;
    STBIR_SIMD_NO_UNROLL(encode);

    stbir__simdf_load4_transposed( f0, f1, f2, f3, encode );

    stbir__min_max_shift20( i0, f0 );
    stbir__min_max_shift20( i1, f1 );
    stbir__min_max_shift20( i2, f2 );
    stbir__min_max_shift20( i3, f3 );

    stbir__simdi_srgb_lookup4( i0, i1, i2, i3 );

    stbir__linear_to_srgb_finish( i0, f0 );
    stbir__linear_to_srgb_finish( i1, f1 );
    stbir__linear_to_srgb_finish( i2, f2 );
    stbir__linear_to_srgb_finish( i3, f3 );

    stbir__interleave_pack_and_store_16_u8( output,  STBIR_strs_join1(i, ,stbir__encode_order0), STBIR_strs_join1(i, ,stbir__encode_order1), STBIR_strs_join1(i, ,stbir__encode_order2), STBIR_strs_join1(i, ,stbir__encode_order3) );

    encode += 16;
    output += 16;
  }
  #endif

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while ( output <= end_output )
  {
    float e0, e1, e2, e3;
    STBIR_SIMD_NO_UNROLL(encode);

    e0 = encode[stbir__encode_order0];
    e1 = encode[stbir__encode_order1];
    e2 = encode[stbir__encode_order2];
    e3 = encode[stbir__encode_order3];
    output[0-4] = stbir__linear_to_srgb_uchar( e0 );
    output[1-4] = stbir__linear_to_srgb_uchar( e1 );
    output[2-4] = stbir__linear_to_srgb_uchar( e2 );
    output[3-4] = stbir__linear_to_srgb_uchar( e3 );

    output += 4;
    encode += 4;
  }
  output -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( output < end_output )
  {
    float e0;
    #if stbir__coder_min_num >= 2
    float e1;
    #endif
    #if stbir__coder_min_num >= 3
    float e2;
    #endif
    STBIR_NO_UNROLL(encode);
    e0 = encode[stbir__encode_order0];
    #if stbir__coder_min_num >= 2
    e1 = encode[stbir__encode_order1];
    #endif
    #if stbir__coder_min_num >= 3
    e2 = encode[stbir__encode_order2];
    #endif
    output[0] = stbir__linear_to_srgb_uchar( e0 );
    #if stbir__coder_min_num >= 2
    output[1] = stbir__linear_to_srgb_uchar( e1 );
    #endif
    #if stbir__coder_min_num >= 3
    output[2] = stbir__linear_to_srgb_uchar( e2 );
    #endif
    output += stbir__coder_min_num;
    encode += stbir__coder_min_num;
  }
  #endif
}

#if ( stbir__coder_min_num == 4 ) || ( ( stbir__coder_min_num == 1 ) && ( !defined(stbir__decode_swizzle) ) )

static float * STBIR__CODER_NAME(stbir__decode_uint8_srgb4_linearalpha)( float * decodep, ptrdiff_t width_times_channels, void const * inputp )
{
  float STBIR_STREAMOUT_PTR( * ) decode = decodep;
  float * decode_end = (float*) decode + width_times_channels;
  unsigned char const * input = (unsigned char const *)inputp;

  do {
    decode[0] = stbir__srgb_uchar_to_linear_float[ input[stbir__decode_order0] ];
    decode[1] = stbir__srgb_uchar_to_linear_float[ input[stbir__decode_order1] ];
    decode[2] = stbir__srgb_uchar_to_linear_float[ input[stbir__decode_order2] ];
    decode[3] = ( (float) input[stbir__decode_order3] ) * stbir__max_uint8_as_float_inverted;
    input += 4;
    decode += 4;
  } while( decode < decode_end );
  return decode_end;
}


static void STBIR__CODER_NAME( stbir__encode_uint8_srgb4_linearalpha )( void * outputp, ptrdiff_t width_times_channels, float const * encode )
{
  unsigned char STBIR_SIMD_STREAMOUT_PTR( * ) output = (unsigned char*) outputp;
  unsigned char * end_output = ( (unsigned char*) output ) + width_times_channels;

  #ifdef STBIR_SIMD

  // double width blocks while they fit, then the four-at-a-time and scalar loops below finish the row. (the encode can
  //   be in place - with an output callback it converts into the float buffer - so no backing up to redo the last block)
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while ( ( end_output - output ) >= 16 )
  {
    stbir__simdf f0, f1, f2, f3;
    stbir__simdi i0, i1, i2, i3;

    STBIR_SIMD_NO_UNROLL(encode);
    stbir__simdf_load4_transposed( f0, f1, f2, f3, encode );

    stbir__min_max_shift20( i0, f0 );
    stbir__min_max_shift20( i1, f1 );
    stbir__min_max_shift20( i2, f2 );
    stbir__scale_and_convert( i3, f3 );

    stbir__simdi_srgb_lookup3( i0, i1, i2 );

    stbir__linear_to_srgb_finish( i0, f0 );
    stbir__linear_to_srgb_finish( i1, f1 );
    stbir__linear_to_srgb_finish( i2, f2 );

    stbir__interleave_pack_and_store_16_u8( output,  STBIR_strs_join1(i, ,stbir__encode_order0), STBIR_strs_join1(i, ,stbir__encode_order1), STBIR_strs_join1(i, ,stbir__encode_order2), STBIR_strs_join1(i, ,stbir__encode_order3) );

    output += 16;
    encode += 16;

  }
  #endif

  // the SIMD loop above can have finished the row, so this has to test first
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while ( output < end_output )
  {
    float f;
    STBIR_SIMD_NO_UNROLL(encode);

    output[stbir__decode_order0] = stbir__linear_to_srgb_uchar( encode[0] );
    output[stbir__decode_order1] = stbir__linear_to_srgb_uchar( encode[1] );
    output[stbir__decode_order2] = stbir__linear_to_srgb_uchar( encode[2] );

    f = encode[3] * stbir__max_uint8_as_float + 0.5f;
    STBIR_CLAMP(f, 0, 255);
    output[stbir__decode_order3] = (unsigned char) f;

    output += 4;
    encode += 4;
  }
}

#endif

#if ( stbir__coder_min_num == 2 ) || ( ( stbir__coder_min_num == 1 ) && ( !defined(stbir__decode_swizzle) ) )

static float * STBIR__CODER_NAME(stbir__decode_uint8_srgb2_linearalpha)( float * decodep, ptrdiff_t width_times_channels, void const * inputp )
{
  float STBIR_STREAMOUT_PTR( * ) decode = decodep;
  float * decode_end = (float*) decode + width_times_channels;
  unsigned char const * input = (unsigned char const *)inputp;

  decode += 4;
  while( decode <= decode_end )
  {
    decode[0-4] = stbir__srgb_uchar_to_linear_float[ input[stbir__decode_order0] ];
    decode[1-4] = ( (float) input[stbir__decode_order1] ) * stbir__max_uint8_as_float_inverted;
    decode[2-4] = stbir__srgb_uchar_to_linear_float[ input[stbir__decode_order0+2] ];
    decode[3-4] = ( (float) input[stbir__decode_order1+2] ) * stbir__max_uint8_as_float_inverted;
    input += 4;
    decode += 4;
  }
  decode -= 4;
  if( decode < decode_end )
  {
    decode[0] = stbir__srgb_uchar_to_linear_float[ input[stbir__decode_order0] ];
    decode[1] = ( (float) input[stbir__decode_order1] ) * stbir__max_uint8_as_float_inverted;
  }
  return decode_end;
}

static void STBIR__CODER_NAME( stbir__encode_uint8_srgb2_linearalpha )( void * outputp, ptrdiff_t width_times_channels, float const * encode )
{
  unsigned char STBIR_SIMD_STREAMOUT_PTR( * ) output = (unsigned char*) outputp;
  unsigned char * end_output = ( (unsigned char*) output ) + width_times_channels;

  #ifdef STBIR_SIMD

  // double width blocks while they fit, then the four-at-a-time and scalar loops below finish the row. (the encode can
  //   be in place - with an output callback it converts into the float buffer - so no backing up to redo the last block)
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while ( ( end_output - output ) >= 16 )
  {
    stbir__simdf f0, f1, f2, f3;
    stbir__simdi i0, i1, i2, i3;

    STBIR_SIMD_NO_UNROLL(encode);
    stbir__simdf_load4_transposed( f0, f1, f2, f3, encode );

    stbir__min_max_shift20( i0, f0 );
    stbir__scale_and_convert( i1, f1 );
    stbir__min_max_shift20( i2, f2 );
    stbir__scale_and_convert( i3, f3 );

    stbir__simdi_srgb_lookup2( i0, i2 );

    stbir__linear_to_srgb_finish( i0, f0 );
    stbir__linear_to_srgb_finish( i2, f2 );

    stbir__interleave_pack_and_store_16_u8( output,  STBIR_strs_join1(i, ,stbir__encode_order0), STBIR_strs_join1(i, ,stbir__encode_order1), STBIR_strs_join1(i, ,stbir__encode_order2), STBIR_strs_join1(i, ,stbir__encode_order3) );

    output += 16;
    encode += 16;
  }
  #endif

  // the SIMD loop above can have finished the row, so this has to test first
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while ( output < end_output )
  {
    float f;
    STBIR_SIMD_NO_UNROLL(encode);

    output[stbir__decode_order0] = stbir__linear_to_srgb_uchar( encode[0] );

    f = encode[1] * stbir__max_uint8_as_float + 0.5f;
    STBIR_CLAMP(f, 0, 255);
    output[stbir__decode_order1] = (unsigned char) f;

    output += 2;
    encode += 2;
  }
}

#endif

static float * STBIR__CODER_NAME(stbir__decode_uint16_linear_scaled)( float * decodep, ptrdiff_t width_times_channels, void const * inputp )
{
  float STBIR_STREAMOUT_PTR( * ) decode = decodep;
  float * decode_end = (float*) decode + width_times_channels;
  unsigned short const * input = (unsigned short const *)inputp;

  #ifdef STBIR_SIMD
  // a block is 16 words at 16 wide (one 32 byte load), 8 otherwise
  unsigned short const * end_input_mb = input + width_times_channels - STBIR__U16_DECODE_BLOCK;
  if ( width_times_channels >= STBIR__U16_DECODE_BLOCK )
  {
    decode_end -= STBIR__U16_DECODE_BLOCK;
    STBIR_NO_UNROLL_LOOP_START_INF_FOR
    for(;;)
    {
      #if defined(STBIR_SIMD16)
      stbir__simdi8 i; stbir__simdi16 o;
      stbir__simdf16 of;
      STBIR_NO_UNROLL(decode);
      stbir__simdi8_load( i, input );
      stbir__decode_simdi8_flip_u16( i );
      stbir__simdi16_expand_u16_to_u32( o, i );
      stbir__simdi16_convert_i32_to_float( of, o );
      stbir__simdf16_mult( of, of, STBIR_max_uint16_as_float_inverted16 );
      stbir__decode_simdf16_flip_i( of );
      stbir__simdf16_store( decode, of );
      #elif defined(STBIR_SIMD8)
      stbir__simdi i; stbir__simdi8 o;
      stbir__simdf8 of;
      STBIR_NO_UNROLL(decode);
      stbir__simdi_load( i, input );
      stbir__decode_simdi_flip_u16( i );
      stbir__simdi8_expand_u16_to_u32( o, i );
      stbir__simdi8_convert_i32_to_float( of, o );
      stbir__simdf8_mult( of, of, STBIR_max_uint16_as_float_inverted8);
      stbir__decode_simdf8_flip_i( of );
      stbir__simdf8_store( decode + 0, of );
      #else
      stbir__simdi i, o0, o1;
      stbir__simdf of0, of1;
      STBIR_NO_UNROLL(decode);
      stbir__simdi_load( i, input );
      stbir__decode_simdi_flip_u16( i );
      stbir__simdi_expand_u16_to_u32( o0,o1,i );
      stbir__simdi_convert_i32_to_float( of0, o0 );
      stbir__simdi_convert_i32_to_float( of1, o1 );
      stbir__simdf_mult( of0, of0, STBIR__CONSTF(STBIR_max_uint16_as_float_inverted) );
      stbir__simdf_mult( of1, of1, STBIR__CONSTF(STBIR_max_uint16_as_float_inverted));
      stbir__decode_simdf4_flip_i( of0 );
      stbir__decode_simdf4_flip_i( of1 );
      stbir__simdf_store( decode + 0,  of0 );
      stbir__simdf_store( decode + 4,  of1 );
      #endif
      decode += STBIR__U16_DECODE_BLOCK;
      input += STBIR__U16_DECODE_BLOCK;
      if ( decode <= decode_end )
        continue;
      if ( decode == ( decode_end + STBIR__U16_DECODE_BLOCK ) )
        break;
      decode = decode_end; // backup and do last couple
      input = end_input_mb;
    }
    return decode_end + STBIR__U16_DECODE_BLOCK;
  }
  #endif

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  decode += 4;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( decode <= decode_end )
  {
    STBIR_SIMD_NO_UNROLL(decode);
    decode[0-4] = ((float)(input[stbir__decode_order0])) * stbir__max_uint16_as_float_inverted;
    decode[1-4] = ((float)(input[stbir__decode_order1])) * stbir__max_uint16_as_float_inverted;
    decode[2-4] = ((float)(input[stbir__decode_order2])) * stbir__max_uint16_as_float_inverted;
    decode[3-4] = ((float)(input[stbir__decode_order3])) * stbir__max_uint16_as_float_inverted;
    decode += 4;
    input += 4;
  }
  decode -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( decode < decode_end )
  {
    STBIR_NO_UNROLL(decode);
    decode[0] = ((float)(input[stbir__decode_order0])) * stbir__max_uint16_as_float_inverted;
    #if stbir__coder_min_num >= 2
    decode[1] = ((float)(input[stbir__decode_order1])) * stbir__max_uint16_as_float_inverted;
    #endif
    #if stbir__coder_min_num >= 3
    decode[2] = ((float)(input[stbir__decode_order2])) * stbir__max_uint16_as_float_inverted;
    #endif
    decode += stbir__coder_min_num;
    input += stbir__coder_min_num;
  }
  #endif
  return decode_end;
}


static void STBIR__CODER_NAME(stbir__encode_uint16_linear_scaled)( void * outputp, ptrdiff_t width_times_channels, float const * encode )
{
  unsigned short STBIR_SIMD_STREAMOUT_PTR( * ) output = (unsigned short*) outputp;
  unsigned short * end_output = ( (unsigned short*) output ) + width_times_channels;

  #ifdef STBIR_SIMD
  {
    // double width blocks while they fit, then the four-at-a-time and scalar loops below finish the row. (the encode can
    //   be in place - with an output callback it converts into the float buffer - so no backing up to redo the last block)
    STBIR_SIMD_NO_UNROLL_LOOP_START
    while ( ( end_output - output ) >= stbir__simdfX_float_count*2 )
    {
      stbir__simdfX e0, e1;
      stbir__simdiX i;
      STBIR_SIMD_NO_UNROLL(encode);
      stbir__simdfX_madd_mem( e0, STBIR_simd_point5X, STBIR_max_uint16_as_floatX, encode );
      stbir__simdfX_madd_mem( e1, STBIR_simd_point5X, STBIR_max_uint16_as_floatX, encode+stbir__simdfX_float_count );
      stbir__encode_simdfX_unflip( e0 );
      stbir__encode_simdfX_unflip( e1 );
      stbir__simdfX_pack_to_words( i, e0, e1 );
      stbir__simdiX_store( output, i );
      encode += stbir__simdfX_float_count*2;
      output += stbir__simdfX_float_count*2;
    }
  }

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  STBIR_NO_UNROLL_LOOP_START
  while( output <= end_output )
  {
    stbir__simdf e;
    stbir__simdi i;
    STBIR_NO_UNROLL(encode);
    stbir__simdf_load( e, encode );
    stbir__simdf_madd( e, STBIR__CONSTF(STBIR_simd_point5), STBIR__CONSTF(STBIR_max_uint16_as_float), e );
    stbir__encode_simdf4_unflip( e );
    stbir__simdf_pack_to_8words( i, e, e );  // only use first 4
    stbir__simdi_store2( output-4, i );
    output += 4;
    encode += 4;
  }
  output -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( output < end_output )
  {
    // read the whole pixel before writing any of it - the encode can be in place, and a swizzle reads a channel after
    //   the one it lands on has been overwritten
    stbir__simdf e0;
    #if stbir__coder_min_num >= 2
    stbir__simdf e1;
    #endif
    #if stbir__coder_min_num >= 3
    stbir__simdf e2;
    #endif
    STBIR_NO_UNROLL(encode);
    stbir__simdf_madd1_mem( e0, STBIR__CONSTF(STBIR_simd_point5), STBIR__CONSTF(STBIR_max_uint16_as_float), encode+stbir__encode_order0 );
    #if stbir__coder_min_num >= 2
    stbir__simdf_madd1_mem( e1, STBIR__CONSTF(STBIR_simd_point5), STBIR__CONSTF(STBIR_max_uint16_as_float), encode+stbir__encode_order1 );
    #endif
    #if stbir__coder_min_num >= 3
    stbir__simdf_madd1_mem( e2, STBIR__CONSTF(STBIR_simd_point5), STBIR__CONSTF(STBIR_max_uint16_as_float), encode+stbir__encode_order2 );
    #endif
    output[0] = stbir__simdf_convert_float_to_short( e0 );
    #if stbir__coder_min_num >= 2
    output[1] = stbir__simdf_convert_float_to_short( e1 );
    #endif
    #if stbir__coder_min_num >= 3
    output[2] = stbir__simdf_convert_float_to_short( e2 );
    #endif
    output += stbir__coder_min_num;
    encode += stbir__coder_min_num;
  }
  #endif

  #else

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( output <= end_output )
  {
    // read the whole pixel before writing any of it - the encode can be in place, and a swizzle reads a channel after
    //   the one it lands on has been overwritten
    float f0 = encode[stbir__encode_order0] * stbir__max_uint16_as_float + 0.5f;
    float f1 = encode[stbir__encode_order1] * stbir__max_uint16_as_float + 0.5f;
    float f2 = encode[stbir__encode_order2] * stbir__max_uint16_as_float + 0.5f;
    float f3 = encode[stbir__encode_order3] * stbir__max_uint16_as_float + 0.5f;
    STBIR_SIMD_NO_UNROLL(encode);
    STBIR_CLAMP(f0, 0, 65535); output[0-4] = (unsigned short)f0;
    STBIR_CLAMP(f1, 0, 65535); output[1-4] = (unsigned short)f1;
    STBIR_CLAMP(f2, 0, 65535); output[2-4] = (unsigned short)f2;
    STBIR_CLAMP(f3, 0, 65535); output[3-4] = (unsigned short)f3;
    output += 4;
    encode += 4;
  }
  output -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( output < end_output )
  {
    float f0 = encode[stbir__encode_order0] * stbir__max_uint16_as_float + 0.5f;
    #if stbir__coder_min_num >= 2
    float f1 = encode[stbir__encode_order1] * stbir__max_uint16_as_float + 0.5f;
    #endif
    #if stbir__coder_min_num >= 3
    float f2 = encode[stbir__encode_order2] * stbir__max_uint16_as_float + 0.5f;
    #endif
    STBIR_NO_UNROLL(encode);
    STBIR_CLAMP(f0, 0, 65535); output[0] = (unsigned short)f0;
    #if stbir__coder_min_num >= 2
    STBIR_CLAMP(f1, 0, 65535); output[1] = (unsigned short)f1;
    #endif
    #if stbir__coder_min_num >= 3
    STBIR_CLAMP(f2, 0, 65535); output[2] = (unsigned short)f2;
    #endif
    output += stbir__coder_min_num;
    encode += stbir__coder_min_num;
  }
  #endif
  #endif
}

static float * STBIR__CODER_NAME(stbir__decode_uint16_linear)( float * decodep, ptrdiff_t width_times_channels, void const * inputp )
{
  float STBIR_STREAMOUT_PTR( * ) decode = decodep;
  float * decode_end = (float*) decode + width_times_channels;
  unsigned short const * input = (unsigned short const *)inputp;

  #ifdef STBIR_SIMD
  unsigned short const * end_input_mb = input + width_times_channels - STBIR__U16_DECODE_BLOCK;
  if ( width_times_channels >= STBIR__U16_DECODE_BLOCK )
  {
    decode_end -= STBIR__U16_DECODE_BLOCK;
    STBIR_NO_UNROLL_LOOP_START_INF_FOR
    for(;;)
    {
      #if defined(STBIR_SIMD16)
      stbir__simdi8 i; stbir__simdi16 o;
      stbir__simdf16 of;
      STBIR_NO_UNROLL(decode);
      stbir__simdi8_load( i, input );
      stbir__decode_simdi8_flip_u16( i );
      stbir__simdi16_expand_u16_to_u32( o, i );
      stbir__simdi16_convert_i32_to_float( of, o );
      stbir__decode_simdf16_flip_i( of );
      stbir__simdf16_store( decode, of );
      #elif defined(STBIR_SIMD8)
      stbir__simdi i; stbir__simdi8 o;
      stbir__simdf8 of;
      STBIR_NO_UNROLL(decode);
      stbir__simdi_load( i, input );
      stbir__decode_simdi_flip_u16( i );
      stbir__simdi8_expand_u16_to_u32( o, i );
      stbir__simdi8_convert_i32_to_float( of, o );
      stbir__decode_simdf8_flip_i( of );
      stbir__simdf8_store( decode + 0, of );
      #else
      stbir__simdi i, o0, o1;
      stbir__simdf of0, of1;
      STBIR_NO_UNROLL(decode);
      stbir__simdi_load( i, input );
      stbir__decode_simdi_flip_u16( i );
      stbir__simdi_expand_u16_to_u32( o0, o1, i );
      stbir__simdi_convert_i32_to_float( of0, o0 );
      stbir__simdi_convert_i32_to_float( of1, o1 );
      stbir__decode_simdf4_flip_i( of0 );
      stbir__decode_simdf4_flip_i( of1 );
      stbir__simdf_store( decode + 0,  of0 );
      stbir__simdf_store( decode + 4,  of1 );
      #endif
      decode += STBIR__U16_DECODE_BLOCK;
      input += STBIR__U16_DECODE_BLOCK;
      if ( decode <= decode_end )
        continue;
      if ( decode == ( decode_end + STBIR__U16_DECODE_BLOCK ) )
        break;
      decode = decode_end; // backup and do last couple
      input = end_input_mb;
    }
    return decode_end + STBIR__U16_DECODE_BLOCK;
  }
  #endif

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  decode += 4;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( decode <= decode_end )
  {
    STBIR_SIMD_NO_UNROLL(decode);
    decode[0-4] = ((float)(input[stbir__decode_order0]));
    decode[1-4] = ((float)(input[stbir__decode_order1]));
    decode[2-4] = ((float)(input[stbir__decode_order2]));
    decode[3-4] = ((float)(input[stbir__decode_order3]));
    decode += 4;
    input += 4;
  }
  decode -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( decode < decode_end )
  {
    STBIR_NO_UNROLL(decode);
    decode[0] = ((float)(input[stbir__decode_order0]));
    #if stbir__coder_min_num >= 2
    decode[1] = ((float)(input[stbir__decode_order1]));
    #endif
    #if stbir__coder_min_num >= 3
    decode[2] = ((float)(input[stbir__decode_order2]));
    #endif
    decode += stbir__coder_min_num;
    input += stbir__coder_min_num;
  }
  #endif
  return decode_end;
}

static void STBIR__CODER_NAME(stbir__encode_uint16_linear)( void * outputp, ptrdiff_t width_times_channels, float const * encode )
{
  unsigned short STBIR_SIMD_STREAMOUT_PTR( * ) output = (unsigned short*) outputp;
  unsigned short * end_output = ( (unsigned short*) output ) + width_times_channels;

  #ifdef STBIR_SIMD
  {
    // double width blocks while they fit, then the four-at-a-time and scalar loops below finish the row. (the encode can
    //   be in place - with an output callback it converts into the float buffer - so no backing up to redo the last block)
    STBIR_SIMD_NO_UNROLL_LOOP_START
    while ( ( end_output - output ) >= stbir__simdfX_float_count*2 )
    {
      stbir__simdfX e0, e1;
      stbir__simdiX i;
      STBIR_SIMD_NO_UNROLL(encode);
      stbir__simdfX_add_mem( e0, STBIR_simd_point5X, encode );
      stbir__simdfX_add_mem( e1, STBIR_simd_point5X, encode+stbir__simdfX_float_count );
      stbir__encode_simdfX_unflip( e0 );
      stbir__encode_simdfX_unflip( e1 );
      stbir__simdfX_pack_to_words( i, e0, e1 );
      stbir__simdiX_store( output, i );
      encode += stbir__simdfX_float_count*2;
      output += stbir__simdfX_float_count*2;
    }
  }

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  STBIR_NO_UNROLL_LOOP_START
  while( output <= end_output )
  {
    stbir__simdf e;
    stbir__simdi i;
    STBIR_NO_UNROLL(encode);
    stbir__simdf_load( e, encode );
    stbir__simdf_add( e, STBIR__CONSTF(STBIR_simd_point5), e );
    stbir__encode_simdf4_unflip( e );
    stbir__simdf_pack_to_8words( i, e, e );  // only use first 4
    stbir__simdi_store2( output-4, i );
    output += 4;
    encode += 4;
  }
  output -= 4;
  #endif

  #else

  // try to do blocks of 4 when you can
  #if  stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( output <= end_output )
  {
    // read the whole pixel before writing any of it - the encode can be in place, and a swizzle reads a channel after
    //   the one it lands on has been overwritten
    float f0 = encode[stbir__encode_order0] + 0.5f;
    float f1 = encode[stbir__encode_order1] + 0.5f;
    float f2 = encode[stbir__encode_order2] + 0.5f;
    float f3 = encode[stbir__encode_order3] + 0.5f;
    STBIR_SIMD_NO_UNROLL(encode);
    STBIR_CLAMP(f0, 0, 65535); output[0-4] = (unsigned short)f0;
    STBIR_CLAMP(f1, 0, 65535); output[1-4] = (unsigned short)f1;
    STBIR_CLAMP(f2, 0, 65535); output[2-4] = (unsigned short)f2;
    STBIR_CLAMP(f3, 0, 65535); output[3-4] = (unsigned short)f3;
    output += 4;
    encode += 4;
  }
  output -= 4;
  #endif

  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( output < end_output )
  {
    float f0 = encode[stbir__encode_order0] + 0.5f;
    #if stbir__coder_min_num >= 2
    float f1 = encode[stbir__encode_order1] + 0.5f;
    #endif
    #if stbir__coder_min_num >= 3
    float f2 = encode[stbir__encode_order2] + 0.5f;
    #endif
    STBIR_NO_UNROLL(encode);
    STBIR_CLAMP(f0, 0, 65535); output[0] = (unsigned short)f0;
    #if stbir__coder_min_num >= 2
    STBIR_CLAMP(f1, 0, 65535); output[1] = (unsigned short)f1;
    #endif
    #if stbir__coder_min_num >= 3
    STBIR_CLAMP(f2, 0, 65535); output[2] = (unsigned short)f2;
    #endif
    output += stbir__coder_min_num;
    encode += stbir__coder_min_num;
  }
  #endif
}

static float * STBIR__CODER_NAME(stbir__decode_half_float_linear)( float * decodep, ptrdiff_t width_times_channels, void const * inputp )
{
  float STBIR_STREAMOUT_PTR( * ) decode = decodep;
  float * decode_end = (float*) decode + width_times_channels;
  stbir__FP16 const * input = (stbir__FP16 const *)inputp;

  #ifdef STBIR_SIMD
  if ( width_times_channels >= 8 )
  {
    stbir__FP16 const * end_input_m8 = input + width_times_channels - 8;
    decode_end -= 8;
    STBIR_NO_UNROLL_LOOP_START_INF_FOR
    for(;;)
    {
      STBIR_NO_UNROLL(decode);

      stbir__half_to_float_SIMD( decode, input );
      #ifdef stbir__decode_swizzle
      #ifdef STBIR_SIMD8
      {
        stbir__simdf8 of;
        stbir__simdf8_load( of, decode );
        stbir__decode_simdf8_flip( of );
        stbir__simdf8_store( decode, of );
      }
      #else
      {
        stbir__simdf of0,of1;
        stbir__simdf_load( of0, decode );
        stbir__simdf_load( of1, decode+4 );
        stbir__decode_simdf4_flip( of0 );
        stbir__decode_simdf4_flip( of1 );
        stbir__simdf_store( decode, of0 );
        stbir__simdf_store( decode+4, of1 );
      }
      #endif
      #endif
      decode += 8;
      input += 8;
      if ( decode <= decode_end )
        continue;
      if ( decode == ( decode_end + 8 ) )
        break;
      decode = decode_end; // backup and do last couple
      input = end_input_m8;
    }
    return decode_end + 8;
  }
  #endif

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  decode += 4;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( decode <= decode_end )
  {
    STBIR_SIMD_NO_UNROLL(decode);
    decode[0-4] = stbir__half_to_float(input[stbir__decode_order0]);
    decode[1-4] = stbir__half_to_float(input[stbir__decode_order1]);
    decode[2-4] = stbir__half_to_float(input[stbir__decode_order2]);
    decode[3-4] = stbir__half_to_float(input[stbir__decode_order3]);
    decode += 4;
    input += 4;
  }
  decode -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( decode < decode_end )
  {
    STBIR_NO_UNROLL(decode);
    decode[0] = stbir__half_to_float(input[stbir__decode_order0]);
    #if stbir__coder_min_num >= 2
    decode[1] = stbir__half_to_float(input[stbir__decode_order1]);
    #endif
    #if stbir__coder_min_num >= 3
    decode[2] = stbir__half_to_float(input[stbir__decode_order2]);
    #endif
    decode += stbir__coder_min_num;
    input += stbir__coder_min_num;
  }
  #endif
  return decode_end;
}

static void STBIR__CODER_NAME( stbir__encode_half_float_linear )( void * outputp, ptrdiff_t width_times_channels, float const * encode )
{
  stbir__FP16 STBIR_SIMD_STREAMOUT_PTR( * ) output = (stbir__FP16*) outputp;
  stbir__FP16 * end_output = ( (stbir__FP16*) output ) + width_times_channels;

  #ifdef STBIR_SIMD
  // double width blocks while they fit, then the four-at-a-time and scalar loops below finish the row. (the encode can
  //   be in place - with an output callback it converts into the float buffer - so no backing up to redo the last block)
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while ( ( end_output - output ) >= 8 )
  {
    STBIR_SIMD_NO_UNROLL(encode);
    #ifdef stbir__decode_swizzle
    #ifdef STBIR_SIMD8
    {
      stbir__simdf8 of;
      stbir__simdf8_load( of, encode );
      stbir__encode_simdf8_unflip( of );
      stbir__float_to_half_SIMD( output, (float*)&of );
    }
    #else
    {
      stbir__simdf of[2];
      stbir__simdf_load( of[0], encode );
      stbir__simdf_load( of[1], encode+4 );
      stbir__encode_simdf4_unflip( of[0] );
      stbir__encode_simdf4_unflip( of[1] );
      stbir__float_to_half_SIMD( output, (float*)of );
    }
    #endif
    #else
    stbir__float_to_half_SIMD( output, encode );
    #endif
    encode += 8;
    output += 8;
  }
  #endif

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( output <= end_output )
  {
    float e0, e1, e2, e3;
    STBIR_SIMD_NO_UNROLL(output);
    e0 = encode[stbir__encode_order0];
    e1 = encode[stbir__encode_order1];
    e2 = encode[stbir__encode_order2];
    e3 = encode[stbir__encode_order3];
    output[0-4] = stbir__float_to_half(e0);
    output[1-4] = stbir__float_to_half(e1);
    output[2-4] = stbir__float_to_half(e2);
    output[3-4] = stbir__float_to_half(e3);
    output += 4;
    encode += 4;
  }
  output -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( output < end_output )
  {
    float e0;
    #if stbir__coder_min_num >= 2
    float e1;
    #endif
    #if stbir__coder_min_num >= 3
    float e2;
    #endif
    STBIR_NO_UNROLL(output);
    e0 = encode[stbir__encode_order0];
    #if stbir__coder_min_num >= 2
    e1 = encode[stbir__encode_order1];
    #endif
    #if stbir__coder_min_num >= 3
    e2 = encode[stbir__encode_order2];
    #endif
    output[0] = stbir__float_to_half(e0);
    #if stbir__coder_min_num >= 2
    output[1] = stbir__float_to_half(e1);
    #endif
    #if stbir__coder_min_num >= 3
    output[2] = stbir__float_to_half(e2);
    #endif
    output += stbir__coder_min_num;
    encode += stbir__coder_min_num;
  }
  #endif
}

static float * STBIR__CODER_NAME(stbir__decode_float_linear)( float * decodep, ptrdiff_t width_times_channels, void const * inputp )
{
  #ifdef stbir__decode_swizzle
  float STBIR_STREAMOUT_PTR( * ) decode = decodep;
  float * decode_end = (float*) decode + width_times_channels;
  float const * input = (float const *)inputp;

  #ifdef STBIR_SIMD
  if ( width_times_channels >= 16 )
  {
    float const * end_input_m16 = input + width_times_channels - 16;
    decode_end -= 16;
    STBIR_NO_UNROLL_LOOP_START_INF_FOR
    for(;;)
    {
      STBIR_NO_UNROLL(decode);
      #ifdef STBIR_SIMD8
      {
        stbir__simdf8 of0,of1;
        stbir__simdf8_load( of0, input );
        stbir__simdf8_load( of1, input+8 );
        stbir__decode_simdf8_flip( of0 );
        stbir__decode_simdf8_flip( of1 );
        stbir__simdf8_store( decode, of0 );
        stbir__simdf8_store( decode+8, of1 );
      }
      #else
      {
        stbir__simdf of0,of1,of2,of3;
        stbir__simdf_load( of0, input );
        stbir__simdf_load( of1, input+4 );
        stbir__simdf_load( of2, input+8 );
        stbir__simdf_load( of3, input+12 );
        stbir__decode_simdf4_flip( of0 );
        stbir__decode_simdf4_flip( of1 );
        stbir__decode_simdf4_flip( of2 );
        stbir__decode_simdf4_flip( of3 );
        stbir__simdf_store( decode, of0 );
        stbir__simdf_store( decode+4, of1 );
        stbir__simdf_store( decode+8, of2 );
        stbir__simdf_store( decode+12, of3 );
      }
      #endif
      decode += 16;
      input += 16;
      if ( decode <= decode_end )
        continue;
      if ( decode == ( decode_end + 16 ) )
        break;
      decode = decode_end; // backup and do last couple
      input = end_input_m16;
    }
    return decode_end + 16;
  }
  #endif

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  decode += 4;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( decode <= decode_end )
  {
    STBIR_SIMD_NO_UNROLL(decode);
    decode[0-4] = input[stbir__decode_order0];
    decode[1-4] = input[stbir__decode_order1];
    decode[2-4] = input[stbir__decode_order2];
    decode[3-4] = input[stbir__decode_order3];
    decode += 4;
    input += 4;
  }
  decode -= 4;
  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( decode < decode_end )
  {
    STBIR_NO_UNROLL(decode);
    decode[0] = input[stbir__decode_order0];
    #if stbir__coder_min_num >= 2
    decode[1] = input[stbir__decode_order1];
    #endif
    #if stbir__coder_min_num >= 3
    decode[2] = input[stbir__decode_order2];
    #endif
    decode += stbir__coder_min_num;
    input += stbir__coder_min_num;
  }
  #endif
  return decode_end;

  #else

  if ( (void*)decodep != inputp )
    STBIR_MEMCPY( decodep, inputp, width_times_channels * sizeof( float ) );

  return decodep + width_times_channels;

  #endif
}

static void STBIR__CODER_NAME( stbir__encode_float_linear )( void * outputp, ptrdiff_t width_times_channels, float const * encode )
{
  #if !defined( STBIR_FLOAT_HIGH_CLAMP ) && !defined(STBIR_FLOAT_LOW_CLAMP) && !defined(stbir__decode_swizzle)

  if ( (void*)outputp != (void*) encode )
    STBIR_MEMCPY( outputp, encode, width_times_channels * sizeof( float ) );

  #else

  float STBIR_SIMD_STREAMOUT_PTR( * ) output = (float*) outputp;
  float * end_output = ( (float*) output ) + width_times_channels;

  #ifdef STBIR_FLOAT_HIGH_CLAMP
  #define stbir_scalar_hi_clamp( v ) if ( v > STBIR_FLOAT_HIGH_CLAMP ) v = STBIR_FLOAT_HIGH_CLAMP;
  #else
  #define stbir_scalar_hi_clamp( v )
  #endif
  #ifdef STBIR_FLOAT_LOW_CLAMP
  #define stbir_scalar_lo_clamp( v ) if ( v < STBIR_FLOAT_LOW_CLAMP ) v = STBIR_FLOAT_LOW_CLAMP;
  #else
  #define stbir_scalar_lo_clamp( v )
  #endif

  #ifdef STBIR_SIMD

  #ifdef STBIR_FLOAT_HIGH_CLAMP
  const stbir__simdfX high_clamp = stbir__simdf_frepX(STBIR_FLOAT_HIGH_CLAMP);
  #endif
  #ifdef STBIR_FLOAT_LOW_CLAMP
  const stbir__simdfX low_clamp = stbir__simdf_frepX(STBIR_FLOAT_LOW_CLAMP);
  #endif

  // do as many double width blocks as fit and let the four-at-a-time and scalar loops below finish the row. (with an
  //   output callback we encode in place, so we can't use the usual trick of backing up and redoing the last block)
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while ( ( end_output - output ) >= ( stbir__simdfX_float_count * 2 ) )
  {
    stbir__simdfX e0, e1;
    STBIR_SIMD_NO_UNROLL(encode);
    stbir__simdfX_load( e0, encode );
    stbir__simdfX_load( e1, encode+stbir__simdfX_float_count );
#ifdef STBIR_FLOAT_HIGH_CLAMP
    stbir__simdfX_min( e0, e0, high_clamp );
    stbir__simdfX_min( e1, e1, high_clamp );
#endif
#ifdef STBIR_FLOAT_LOW_CLAMP
    stbir__simdfX_max( e0, e0, low_clamp );
    stbir__simdfX_max( e1, e1, low_clamp );
#endif
    stbir__encode_simdfX_unflip( e0 );
    stbir__encode_simdfX_unflip( e1 );
    stbir__simdfX_store( output, e0 );
    stbir__simdfX_store( output+stbir__simdfX_float_count, e1 );
    encode += stbir__simdfX_float_count * 2;
    output += stbir__simdfX_float_count * 2;
  }

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  STBIR_NO_UNROLL_LOOP_START
  while( output <= end_output )
  {
    stbir__simdf e0;
    STBIR_NO_UNROLL(encode);
    stbir__simdf_load( e0, encode );
#ifdef STBIR_FLOAT_HIGH_CLAMP
    stbir__simdf_min( e0, e0, stbir__simdfX_cast_to_simdf4( high_clamp ) );
#endif
#ifdef STBIR_FLOAT_LOW_CLAMP
    stbir__simdf_max( e0, e0, stbir__simdfX_cast_to_simdf4( low_clamp ) );
#endif
    stbir__encode_simdf4_unflip( e0 );
    stbir__simdf_store( output-4, e0 );
    output += 4;
    encode += 4;
  }
  output -= 4;
  #endif

  #else

  // try to do blocks of 4 when you can
  #if stbir__coder_min_num != 3 // doesn't divide cleanly by four
  output += 4;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  while( output <= end_output )
  {
    // read everything before writing anything - with an output callback we encode in place, so this is an in-place unflip
    float e0 = encode[ stbir__encode_order0 ];
    float e1 = encode[ stbir__encode_order1 ];
    float e2 = encode[ stbir__encode_order2 ];
    float e3 = encode[ stbir__encode_order3 ];
    STBIR_SIMD_NO_UNROLL(encode);
    stbir_scalar_hi_clamp( e0 );
    stbir_scalar_lo_clamp( e0 );
    stbir_scalar_hi_clamp( e1 );
    stbir_scalar_lo_clamp( e1 );
    stbir_scalar_hi_clamp( e2 );
    stbir_scalar_lo_clamp( e2 );
    stbir_scalar_hi_clamp( e3 );
    stbir_scalar_lo_clamp( e3 );
    output[0-4] = e0;
    output[1-4] = e1;
    output[2-4] = e2;
    output[3-4] = e3;
    output += 4;
    encode += 4;
  }
  output -= 4;

  #endif

  #endif

  // do the remnants
  #if stbir__coder_min_num < 4
  STBIR_NO_UNROLL_LOOP_START
  while( output < end_output )
  {
    float e0 = encode[ stbir__encode_order0 ];
    #if stbir__coder_min_num >= 2
    float e1 = encode[ stbir__encode_order1 ];
    #endif
    #if stbir__coder_min_num >= 3
    float e2 = encode[ stbir__encode_order2 ];
    #endif
    STBIR_NO_UNROLL(encode);
    stbir_scalar_hi_clamp( e0 ); stbir_scalar_lo_clamp( e0 ); output[0] = e0;
    #if stbir__coder_min_num >= 2
    stbir_scalar_hi_clamp( e1 ); stbir_scalar_lo_clamp( e1 ); output[1] = e1;
    #endif
    #if stbir__coder_min_num >= 3
    stbir_scalar_hi_clamp( e2 ); stbir_scalar_lo_clamp( e2 ); output[2] = e2;
    #endif
    output += stbir__coder_min_num;
    encode += stbir__coder_min_num;
  }
  #endif

  #endif
}

#undef stbir__decode_suffix
#undef stbir__decode_simdf8_flip
#undef stbir__decode_simdf8_flip_i
#undef stbir__decode_simdf4_flip_i
#undef stbir__decode_simdi_flip_u8
#undef stbir__decode_simdi_flip_u16
#ifdef STBIR__SWZ8
#undef STBIR__SWZ8
#undef STBIR__SWZ16
#endif
#undef stbir__decode_simdf4_flip
#undef stbir__decode_order0
#undef stbir__decode_order1
#undef stbir__decode_order2
#undef stbir__decode_order3
#undef stbir__encode_order0
#undef stbir__encode_order1
#undef stbir__encode_order2
#undef stbir__encode_order3
#undef stbir__encode_simdf8_unflip
#undef stbir__encode_simdf16_unflip
#undef stbir__decode_simdf16_flip_i
#undef stbir__decode_simdi8_flip_u16
#undef stbir__encode_simdf4_unflip
#undef stbir__encode_simdfX_unflip
#undef STBIR__CODER_NAME
#undef stbir__coder_min_num
#undef stbir__decode_swizzle
#undef stbir_scalar_hi_clamp
#undef stbir_scalar_lo_clamp
#undef STB_IMAGE_RESIZE_DO_CODERS

#elif defined( STB_IMAGE_RESIZE_DO_VERTICALS)

#ifdef STB_IMAGE_RESIZE_VERTICAL_CONTINUE
#define STBIR_chans( start, end ) STBIR_strs_join14(start,STBIR__vertical_channels,end,_cont)
#else
#define STBIR_chans( start, end ) STBIR_strs_join1(start,STBIR__vertical_channels,end)
#endif

#if STBIR__vertical_channels >= 1
#define stbIF0( code ) code
#else
#define stbIF0( code )
#endif
#if STBIR__vertical_channels >= 2
#define stbIF1( code ) code
#else
#define stbIF1( code )
#endif
#if STBIR__vertical_channels >= 3
#define stbIF2( code ) code
#else
#define stbIF2( code )
#endif
#if STBIR__vertical_channels >= 4
#define stbIF3( code ) code
#else
#define stbIF3( code )
#endif
#if STBIR__vertical_channels >= 5
#define stbIF4( code ) code
#else
#define stbIF4( code )
#endif
#if STBIR__vertical_channels >= 6
#define stbIF5( code ) code
#else
#define stbIF5( code )
#endif
#if STBIR__vertical_channels >= 7
#define stbIF6( code ) code
#else
#define stbIF6( code )
#endif
#if STBIR__vertical_channels >= 8
#define stbIF7( code ) code
#else
#define stbIF7( code )
#endif

static void STBIR_chans( stbir__vertical_scatter_with_,_coeffs)( float ** outputs, float const * vertical_coefficients, float const * input, float const * input_end )
{
  stbIF0( float STBIR_SIMD_STREAMOUT_PTR( * ) output0 = outputs[0]; float c0s = vertical_coefficients[0]; )
  stbIF1( float STBIR_SIMD_STREAMOUT_PTR( * ) output1 = outputs[1]; float c1s = vertical_coefficients[1]; )
  stbIF2( float STBIR_SIMD_STREAMOUT_PTR( * ) output2 = outputs[2]; float c2s = vertical_coefficients[2]; )
  stbIF3( float STBIR_SIMD_STREAMOUT_PTR( * ) output3 = outputs[3]; float c3s = vertical_coefficients[3]; )
  stbIF4( float STBIR_SIMD_STREAMOUT_PTR( * ) output4 = outputs[4]; float c4s = vertical_coefficients[4]; )
  stbIF5( float STBIR_SIMD_STREAMOUT_PTR( * ) output5 = outputs[5]; float c5s = vertical_coefficients[5]; )
  stbIF6( float STBIR_SIMD_STREAMOUT_PTR( * ) output6 = outputs[6]; float c6s = vertical_coefficients[6]; )
  stbIF7( float STBIR_SIMD_STREAMOUT_PTR( * ) output7 = outputs[7]; float c7s = vertical_coefficients[7]; )

  #ifdef STBIR_SIMD
  {
    stbIF0(stbir__simdfX c0 = stbir__simdf_frepX( c0s ); )
    stbIF1(stbir__simdfX c1 = stbir__simdf_frepX( c1s ); )
    stbIF2(stbir__simdfX c2 = stbir__simdf_frepX( c2s ); )
    stbIF3(stbir__simdfX c3 = stbir__simdf_frepX( c3s ); )
    stbIF4(stbir__simdfX c4 = stbir__simdf_frepX( c4s ); )
    stbIF5(stbir__simdfX c5 = stbir__simdf_frepX( c5s ); )
    stbIF6(stbir__simdfX c6 = stbir__simdf_frepX( c6s ); )
    stbIF7(stbir__simdfX c7 = stbir__simdf_frepX( c7s ); )
    STBIR_SIMD_NO_UNROLL_LOOP_START
    while ( ( (char*)input_end - (char*) input ) >= (16*stbir__simdfX_float_count) )
    {
      stbir__simdfX o0, o1, o2, o3, r0, r1, r2, r3;
      STBIR_SIMD_NO_UNROLL(output0);

      stbir__simdfX_load( r0, input );               stbir__simdfX_load( r1, input+stbir__simdfX_float_count );     stbir__simdfX_load( r2, input+(2*stbir__simdfX_float_count) );      stbir__simdfX_load( r3, input+(3*stbir__simdfX_float_count) );

      #ifdef STB_IMAGE_RESIZE_VERTICAL_CONTINUE
      stbIF0( stbir__simdfX_load( o0, output0 );     stbir__simdfX_load( o1, output0+stbir__simdfX_float_count );   stbir__simdfX_load( o2, output0+(2*stbir__simdfX_float_count) );    stbir__simdfX_load( o3, output0+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c0 );  stbir__simdfX_madd( o1, o1, r1, c0 );  stbir__simdfX_madd( o2, o2, r2, c0 );   stbir__simdfX_madd( o3, o3, r3, c0 );
              stbir__simdfX_store( output0, o0 );    stbir__simdfX_store( output0+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output0+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output0+(3*stbir__simdfX_float_count), o3 ); )
      stbIF1( stbir__simdfX_load( o0, output1 );     stbir__simdfX_load( o1, output1+stbir__simdfX_float_count );   stbir__simdfX_load( o2, output1+(2*stbir__simdfX_float_count) );    stbir__simdfX_load( o3, output1+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c1 );  stbir__simdfX_madd( o1, o1, r1, c1 );  stbir__simdfX_madd( o2, o2, r2, c1 );   stbir__simdfX_madd( o3, o3, r3, c1 );
              stbir__simdfX_store( output1, o0 );    stbir__simdfX_store( output1+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output1+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output1+(3*stbir__simdfX_float_count), o3 ); )
      stbIF2( stbir__simdfX_load( o0, output2 );     stbir__simdfX_load( o1, output2+stbir__simdfX_float_count );   stbir__simdfX_load( o2, output2+(2*stbir__simdfX_float_count) );    stbir__simdfX_load( o3, output2+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c2 );  stbir__simdfX_madd( o1, o1, r1, c2 );  stbir__simdfX_madd( o2, o2, r2, c2 );   stbir__simdfX_madd( o3, o3, r3, c2 );
              stbir__simdfX_store( output2, o0 );    stbir__simdfX_store( output2+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output2+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output2+(3*stbir__simdfX_float_count), o3 ); )
      stbIF3( stbir__simdfX_load( o0, output3 );     stbir__simdfX_load( o1, output3+stbir__simdfX_float_count );   stbir__simdfX_load( o2, output3+(2*stbir__simdfX_float_count) );    stbir__simdfX_load( o3, output3+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c3 );  stbir__simdfX_madd( o1, o1, r1, c3 );  stbir__simdfX_madd( o2, o2, r2, c3 );   stbir__simdfX_madd( o3, o3, r3, c3 );
              stbir__simdfX_store( output3, o0 );    stbir__simdfX_store( output3+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output3+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output3+(3*stbir__simdfX_float_count), o3 ); )
      stbIF4( stbir__simdfX_load( o0, output4 );     stbir__simdfX_load( o1, output4+stbir__simdfX_float_count );   stbir__simdfX_load( o2, output4+(2*stbir__simdfX_float_count) );    stbir__simdfX_load( o3, output4+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c4 );  stbir__simdfX_madd( o1, o1, r1, c4 );  stbir__simdfX_madd( o2, o2, r2, c4 );   stbir__simdfX_madd( o3, o3, r3, c4 );
              stbir__simdfX_store( output4, o0 );    stbir__simdfX_store( output4+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output4+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output4+(3*stbir__simdfX_float_count), o3 ); )
      stbIF5( stbir__simdfX_load( o0, output5 );     stbir__simdfX_load( o1, output5+stbir__simdfX_float_count );   stbir__simdfX_load( o2, output5+(2*stbir__simdfX_float_count));    stbir__simdfX_load( o3, output5+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c5 );  stbir__simdfX_madd( o1, o1, r1, c5 );  stbir__simdfX_madd( o2, o2, r2, c5 );   stbir__simdfX_madd( o3, o3, r3, c5 );
              stbir__simdfX_store( output5, o0 );    stbir__simdfX_store( output5+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output5+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output5+(3*stbir__simdfX_float_count), o3 ); )
      stbIF6( stbir__simdfX_load( o0, output6 );     stbir__simdfX_load( o1, output6+stbir__simdfX_float_count );   stbir__simdfX_load( o2, output6+(2*stbir__simdfX_float_count) );    stbir__simdfX_load( o3, output6+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c6 );  stbir__simdfX_madd( o1, o1, r1, c6 );  stbir__simdfX_madd( o2, o2, r2, c6 );   stbir__simdfX_madd( o3, o3, r3, c6 );
              stbir__simdfX_store( output6, o0 );    stbir__simdfX_store( output6+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output6+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output6+(3*stbir__simdfX_float_count), o3 ); )
      stbIF7( stbir__simdfX_load( o0, output7 );     stbir__simdfX_load( o1, output7+stbir__simdfX_float_count );   stbir__simdfX_load( o2, output7+(2*stbir__simdfX_float_count) );    stbir__simdfX_load( o3, output7+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c7 );  stbir__simdfX_madd( o1, o1, r1, c7 );  stbir__simdfX_madd( o2, o2, r2, c7 );   stbir__simdfX_madd( o3, o3, r3, c7 );
              stbir__simdfX_store( output7, o0 );    stbir__simdfX_store( output7+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output7+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output7+(3*stbir__simdfX_float_count), o3 ); )
      #else
      stbIF0( stbir__simdfX_mult( o0, r0, c0 );      stbir__simdfX_mult( o1, r1, c0 );      stbir__simdfX_mult( o2, r2, c0 );       stbir__simdfX_mult( o3, r3, c0 );
              stbir__simdfX_store( output0, o0 );    stbir__simdfX_store( output0+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output0+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output0+(3*stbir__simdfX_float_count), o3 ); )
      stbIF1( stbir__simdfX_mult( o0, r0, c1 );      stbir__simdfX_mult( o1, r1, c1 );      stbir__simdfX_mult( o2, r2, c1 );       stbir__simdfX_mult( o3, r3, c1 );
              stbir__simdfX_store( output1, o0 );    stbir__simdfX_store( output1+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output1+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output1+(3*stbir__simdfX_float_count), o3 ); )
      stbIF2( stbir__simdfX_mult( o0, r0, c2 );      stbir__simdfX_mult( o1, r1, c2 );      stbir__simdfX_mult( o2, r2, c2 );       stbir__simdfX_mult( o3, r3, c2 );
              stbir__simdfX_store( output2, o0 );    stbir__simdfX_store( output2+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output2+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output2+(3*stbir__simdfX_float_count), o3 ); )
      stbIF3( stbir__simdfX_mult( o0, r0, c3 );      stbir__simdfX_mult( o1, r1, c3 );      stbir__simdfX_mult( o2, r2, c3 );       stbir__simdfX_mult( o3, r3, c3 );
              stbir__simdfX_store( output3, o0 );    stbir__simdfX_store( output3+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output3+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output3+(3*stbir__simdfX_float_count), o3 ); )
      stbIF4( stbir__simdfX_mult( o0, r0, c4 );      stbir__simdfX_mult( o1, r1, c4 );      stbir__simdfX_mult( o2, r2, c4 );       stbir__simdfX_mult( o3, r3, c4 );
              stbir__simdfX_store( output4, o0 );    stbir__simdfX_store( output4+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output4+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output4+(3*stbir__simdfX_float_count), o3 ); )
      stbIF5( stbir__simdfX_mult( o0, r0, c5 );      stbir__simdfX_mult( o1, r1, c5 );      stbir__simdfX_mult( o2, r2, c5 );       stbir__simdfX_mult( o3, r3, c5 );
              stbir__simdfX_store( output5, o0 );    stbir__simdfX_store( output5+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output5+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output5+(3*stbir__simdfX_float_count), o3 ); )
      stbIF6( stbir__simdfX_mult( o0, r0, c6 );      stbir__simdfX_mult( o1, r1, c6 );      stbir__simdfX_mult( o2, r2, c6 );       stbir__simdfX_mult( o3, r3, c6 );
              stbir__simdfX_store( output6, o0 );    stbir__simdfX_store( output6+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output6+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output6+(3*stbir__simdfX_float_count), o3 ); )
      stbIF7( stbir__simdfX_mult( o0, r0, c7 );      stbir__simdfX_mult( o1, r1, c7 );      stbir__simdfX_mult( o2, r2, c7 );       stbir__simdfX_mult( o3, r3, c7 );
              stbir__simdfX_store( output7, o0 );    stbir__simdfX_store( output7+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output7+(2*stbir__simdfX_float_count), o2 );   stbir__simdfX_store( output7+(3*stbir__simdfX_float_count), o3 ); )
      #endif

      input += (4*stbir__simdfX_float_count);
      stbIF0( output0 += (4*stbir__simdfX_float_count); )
      stbIF1( output1 += (4*stbir__simdfX_float_count); )
      stbIF2( output2 += (4*stbir__simdfX_float_count); )
      stbIF3( output3 += (4*stbir__simdfX_float_count); )
      stbIF4( output4 += (4*stbir__simdfX_float_count); )
      stbIF5( output5 += (4*stbir__simdfX_float_count); )
      stbIF6( output6 += (4*stbir__simdfX_float_count); )
      stbIF7( output7 += (4*stbir__simdfX_float_count); )
    }
    STBIR_SIMD_NO_UNROLL_LOOP_START
    while ( ( (char*)input_end - (char*) input ) >= 16 )
    {
      stbir__simdf o0, r0;
      STBIR_SIMD_NO_UNROLL(output0);

      stbir__simdf_load( r0, input );

      #ifdef STB_IMAGE_RESIZE_VERTICAL_CONTINUE
      stbIF0( stbir__simdf_load( o0, output0 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c0 ) );  stbir__simdf_store( output0, o0 ); )
      stbIF1( stbir__simdf_load( o0, output1 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c1 ) );  stbir__simdf_store( output1, o0 ); )
      stbIF2( stbir__simdf_load( o0, output2 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c2 ) );  stbir__simdf_store( output2, o0 ); )
      stbIF3( stbir__simdf_load( o0, output3 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c3 ) );  stbir__simdf_store( output3, o0 ); )
      stbIF4( stbir__simdf_load( o0, output4 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c4 ) );  stbir__simdf_store( output4, o0 ); )
      stbIF5( stbir__simdf_load( o0, output5 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c5 ) );  stbir__simdf_store( output5, o0 ); )
      stbIF6( stbir__simdf_load( o0, output6 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c6 ) );  stbir__simdf_store( output6, o0 ); )
      stbIF7( stbir__simdf_load( o0, output7 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c7 ) );  stbir__simdf_store( output7, o0 ); )
      #else
      stbIF0( stbir__simdf_mult( o0, r0, stbir__simdfX_cast_to_simdf4( c0 ) );   stbir__simdf_store( output0, o0 ); )
      stbIF1( stbir__simdf_mult( o0, r0, stbir__simdfX_cast_to_simdf4( c1 ) );   stbir__simdf_store( output1, o0 ); )
      stbIF2( stbir__simdf_mult( o0, r0, stbir__simdfX_cast_to_simdf4( c2 ) );   stbir__simdf_store( output2, o0 ); )
      stbIF3( stbir__simdf_mult( o0, r0, stbir__simdfX_cast_to_simdf4( c3 ) );   stbir__simdf_store( output3, o0 ); )
      stbIF4( stbir__simdf_mult( o0, r0, stbir__simdfX_cast_to_simdf4( c4 ) );   stbir__simdf_store( output4, o0 ); )
      stbIF5( stbir__simdf_mult( o0, r0, stbir__simdfX_cast_to_simdf4( c5 ) );   stbir__simdf_store( output5, o0 ); )
      stbIF6( stbir__simdf_mult( o0, r0, stbir__simdfX_cast_to_simdf4( c6 ) );   stbir__simdf_store( output6, o0 ); )
      stbIF7( stbir__simdf_mult( o0, r0, stbir__simdfX_cast_to_simdf4( c7 ) );   stbir__simdf_store( output7, o0 ); )
      #endif

      input += 4;
      stbIF0( output0 += 4; )
      stbIF1( output1 += 4; )
      stbIF2( output2 += 4; )
      stbIF3( output3 += 4; )
      stbIF4( output4 += 4; )
      stbIF5( output5 += 4; )
      stbIF6( output6 += 4; )
      stbIF7( output7 += 4; )
    }
  }
  #else
  STBIR_NO_UNROLL_LOOP_START
  while ( ( (char*)input_end - (char*) input ) >= 16 )
  {
    float r0, r1, r2, r3;
    STBIR_NO_UNROLL(input);

    r0 = input[0], r1 = input[1], r2 = input[2], r3 = input[3];

    #ifdef STB_IMAGE_RESIZE_VERTICAL_CONTINUE
    stbIF0( output0[0] += ( r0 * c0s ); output0[1] += ( r1 * c0s ); output0[2] += ( r2 * c0s ); output0[3] += ( r3 * c0s ); )
    stbIF1( output1[0] += ( r0 * c1s ); output1[1] += ( r1 * c1s ); output1[2] += ( r2 * c1s ); output1[3] += ( r3 * c1s ); )
    stbIF2( output2[0] += ( r0 * c2s ); output2[1] += ( r1 * c2s ); output2[2] += ( r2 * c2s ); output2[3] += ( r3 * c2s ); )
    stbIF3( output3[0] += ( r0 * c3s ); output3[1] += ( r1 * c3s ); output3[2] += ( r2 * c3s ); output3[3] += ( r3 * c3s ); )
    stbIF4( output4[0] += ( r0 * c4s ); output4[1] += ( r1 * c4s ); output4[2] += ( r2 * c4s ); output4[3] += ( r3 * c4s ); )
    stbIF5( output5[0] += ( r0 * c5s ); output5[1] += ( r1 * c5s ); output5[2] += ( r2 * c5s ); output5[3] += ( r3 * c5s ); )
    stbIF6( output6[0] += ( r0 * c6s ); output6[1] += ( r1 * c6s ); output6[2] += ( r2 * c6s ); output6[3] += ( r3 * c6s ); )
    stbIF7( output7[0] += ( r0 * c7s ); output7[1] += ( r1 * c7s ); output7[2] += ( r2 * c7s ); output7[3] += ( r3 * c7s ); )
    #else
    stbIF0( output0[0]  = ( r0 * c0s ); output0[1]  = ( r1 * c0s ); output0[2]  = ( r2 * c0s ); output0[3]  = ( r3 * c0s ); )
    stbIF1( output1[0]  = ( r0 * c1s ); output1[1]  = ( r1 * c1s ); output1[2]  = ( r2 * c1s ); output1[3]  = ( r3 * c1s ); )
    stbIF2( output2[0]  = ( r0 * c2s ); output2[1]  = ( r1 * c2s ); output2[2]  = ( r2 * c2s ); output2[3]  = ( r3 * c2s ); )
    stbIF3( output3[0]  = ( r0 * c3s ); output3[1]  = ( r1 * c3s ); output3[2]  = ( r2 * c3s ); output3[3]  = ( r3 * c3s ); )
    stbIF4( output4[0]  = ( r0 * c4s ); output4[1]  = ( r1 * c4s ); output4[2]  = ( r2 * c4s ); output4[3]  = ( r3 * c4s ); )
    stbIF5( output5[0]  = ( r0 * c5s ); output5[1]  = ( r1 * c5s ); output5[2]  = ( r2 * c5s ); output5[3]  = ( r3 * c5s ); )
    stbIF6( output6[0]  = ( r0 * c6s ); output6[1]  = ( r1 * c6s ); output6[2]  = ( r2 * c6s ); output6[3]  = ( r3 * c6s ); )
    stbIF7( output7[0]  = ( r0 * c7s ); output7[1]  = ( r1 * c7s ); output7[2]  = ( r2 * c7s ); output7[3]  = ( r3 * c7s ); )
    #endif

    input += 4;
    stbIF0( output0 += 4; )
    stbIF1( output1 += 4; )
    stbIF2( output2 += 4; )
    stbIF3( output3 += 4; )
    stbIF4( output4 += 4; )
    stbIF5( output5 += 4; )
    stbIF6( output6 += 4; )
    stbIF7( output7 += 4; )
  }
  #endif
  STBIR_NO_UNROLL_LOOP_START
  while ( input < input_end )
  {
    float r = input[0];
    STBIR_NO_UNROLL(output0);

    #ifdef STB_IMAGE_RESIZE_VERTICAL_CONTINUE
    stbIF0( output0[0] += ( r * c0s ); )
    stbIF1( output1[0] += ( r * c1s ); )
    stbIF2( output2[0] += ( r * c2s ); )
    stbIF3( output3[0] += ( r * c3s ); )
    stbIF4( output4[0] += ( r * c4s ); )
    stbIF5( output5[0] += ( r * c5s ); )
    stbIF6( output6[0] += ( r * c6s ); )
    stbIF7( output7[0] += ( r * c7s ); )
    #else
    stbIF0( output0[0]  = ( r * c0s ); )
    stbIF1( output1[0]  = ( r * c1s ); )
    stbIF2( output2[0]  = ( r * c2s ); )
    stbIF3( output3[0]  = ( r * c3s ); )
    stbIF4( output4[0]  = ( r * c4s ); )
    stbIF5( output5[0]  = ( r * c5s ); )
    stbIF6( output6[0]  = ( r * c6s ); )
    stbIF7( output7[0]  = ( r * c7s ); )
    #endif

    ++input;
    stbIF0( ++output0; )
    stbIF1( ++output1; )
    stbIF2( ++output2; )
    stbIF3( ++output3; )
    stbIF4( ++output4; )
    stbIF5( ++output5; )
    stbIF6( ++output6; )
    stbIF7( ++output7; )
  }
}

static void STBIR_chans( stbir__vertical_gather_with_,_coeffs)( float * outputp, float const * vertical_coefficients, float const ** inputs, float const * input0_end )
{
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = outputp;

  stbIF0( float const * input0 = inputs[0]; float c0s = vertical_coefficients[0]; )
  stbIF1( float const * input1 = inputs[1]; float c1s = vertical_coefficients[1]; )
  stbIF2( float const * input2 = inputs[2]; float c2s = vertical_coefficients[2]; )
  stbIF3( float const * input3 = inputs[3]; float c3s = vertical_coefficients[3]; )
  stbIF4( float const * input4 = inputs[4]; float c4s = vertical_coefficients[4]; )
  stbIF5( float const * input5 = inputs[5]; float c5s = vertical_coefficients[5]; )
  stbIF6( float const * input6 = inputs[6]; float c6s = vertical_coefficients[6]; )
  stbIF7( float const * input7 = inputs[7]; float c7s = vertical_coefficients[7]; )

#if ( STBIR__vertical_channels == 1 ) && !defined(STB_IMAGE_RESIZE_VERTICAL_CONTINUE)
  // one coefficient within a millionth of 1 (any channel count, the single coefficient instantiation): a copy
  if ( ( c0s >= (1.0f-0.000001f) ) && ( c0s <= (1.0f+0.000001f) ) )
  {
    STBIR_MEMCPY( output, input0, (char*)input0_end - (char*)input0 );
    return;
  }
#endif

  #ifdef STBIR_SIMD
  {
    stbIF0(stbir__simdfX c0 = stbir__simdf_frepX( c0s ); )
    stbIF1(stbir__simdfX c1 = stbir__simdf_frepX( c1s ); )
    stbIF2(stbir__simdfX c2 = stbir__simdf_frepX( c2s ); )
    stbIF3(stbir__simdfX c3 = stbir__simdf_frepX( c3s ); )
    stbIF4(stbir__simdfX c4 = stbir__simdf_frepX( c4s ); )
    stbIF5(stbir__simdfX c5 = stbir__simdf_frepX( c5s ); )
    stbIF6(stbir__simdfX c6 = stbir__simdf_frepX( c6s ); )
    stbIF7(stbir__simdfX c7 = stbir__simdf_frepX( c7s ); )

    STBIR_SIMD_NO_UNROLL_LOOP_START
    while ( ( (char*)input0_end - (char*) input0 ) >= (16*stbir__simdfX_float_count) )
    {
      stbir__simdfX o0, o1, o2, o3, r0, r1, r2, r3;
      STBIR_SIMD_NO_UNROLL(output);

      // prefetch four loop iterations ahead (doesn't affect much for small resizes, but helps with big ones)
      stbIF0( stbir__prefetch( input0 + (16*stbir__simdfX_float_count) ); )
      stbIF1( stbir__prefetch( input1 + (16*stbir__simdfX_float_count) ); )
      stbIF2( stbir__prefetch( input2 + (16*stbir__simdfX_float_count) ); )
      stbIF3( stbir__prefetch( input3 + (16*stbir__simdfX_float_count) ); )
      stbIF4( stbir__prefetch( input4 + (16*stbir__simdfX_float_count) ); )
      stbIF5( stbir__prefetch( input5 + (16*stbir__simdfX_float_count) ); )
      stbIF6( stbir__prefetch( input6 + (16*stbir__simdfX_float_count) ); )
      stbIF7( stbir__prefetch( input7 + (16*stbir__simdfX_float_count) ); )

      #ifdef STB_IMAGE_RESIZE_VERTICAL_CONTINUE
      stbIF0( stbir__simdfX_load( o0, output );      stbir__simdfX_load( o1, output+stbir__simdfX_float_count );   stbir__simdfX_load( o2, output+(2*stbir__simdfX_float_count) );   stbir__simdfX_load( o3, output+(3*stbir__simdfX_float_count) );
              stbir__simdfX_load( r0, input0 );      stbir__simdfX_load( r1, input0+stbir__simdfX_float_count );   stbir__simdfX_load( r2, input0+(2*stbir__simdfX_float_count) );   stbir__simdfX_load( r3, input0+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c0 );  stbir__simdfX_madd( o1, o1, r1, c0 );                         stbir__simdfX_madd( o2, o2, r2, c0 );                             stbir__simdfX_madd( o3, o3, r3, c0 ); )
      #else
      stbIF0( stbir__simdfX_load( r0, input0 );      stbir__simdfX_load( r1, input0+stbir__simdfX_float_count );   stbir__simdfX_load( r2, input0+(2*stbir__simdfX_float_count) );   stbir__simdfX_load( r3, input0+(3*stbir__simdfX_float_count) );
              stbir__simdfX_mult( o0, r0, c0 );      stbir__simdfX_mult( o1, r1, c0 );                             stbir__simdfX_mult( o2, r2, c0 );                                 stbir__simdfX_mult( o3, r3, c0 );  )
      #endif

      stbIF1( stbir__simdfX_load( r0, input1 );      stbir__simdfX_load( r1, input1+stbir__simdfX_float_count );   stbir__simdfX_load( r2, input1+(2*stbir__simdfX_float_count) );   stbir__simdfX_load( r3, input1+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c1 );  stbir__simdfX_madd( o1, o1, r1, c1 );                         stbir__simdfX_madd( o2, o2, r2, c1 );                             stbir__simdfX_madd( o3, o3, r3, c1 ); )
      stbIF2( stbir__simdfX_load( r0, input2 );      stbir__simdfX_load( r1, input2+stbir__simdfX_float_count );   stbir__simdfX_load( r2, input2+(2*stbir__simdfX_float_count) );   stbir__simdfX_load( r3, input2+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c2 );  stbir__simdfX_madd( o1, o1, r1, c2 );                         stbir__simdfX_madd( o2, o2, r2, c2 );                             stbir__simdfX_madd( o3, o3, r3, c2 ); )
      stbIF3( stbir__simdfX_load( r0, input3 );      stbir__simdfX_load( r1, input3+stbir__simdfX_float_count );   stbir__simdfX_load( r2, input3+(2*stbir__simdfX_float_count) );   stbir__simdfX_load( r3, input3+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c3 );  stbir__simdfX_madd( o1, o1, r1, c3 );                         stbir__simdfX_madd( o2, o2, r2, c3 );                             stbir__simdfX_madd( o3, o3, r3, c3 ); )
      stbIF4( stbir__simdfX_load( r0, input4 );      stbir__simdfX_load( r1, input4+stbir__simdfX_float_count );   stbir__simdfX_load( r2, input4+(2*stbir__simdfX_float_count) );   stbir__simdfX_load( r3, input4+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c4 );  stbir__simdfX_madd( o1, o1, r1, c4 );                         stbir__simdfX_madd( o2, o2, r2, c4 );                             stbir__simdfX_madd( o3, o3, r3, c4 ); )
      stbIF5( stbir__simdfX_load( r0, input5 );      stbir__simdfX_load( r1, input5+stbir__simdfX_float_count );   stbir__simdfX_load( r2, input5+(2*stbir__simdfX_float_count) );   stbir__simdfX_load( r3, input5+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c5 );  stbir__simdfX_madd( o1, o1, r1, c5 );                         stbir__simdfX_madd( o2, o2, r2, c5 );                             stbir__simdfX_madd( o3, o3, r3, c5 ); )
      stbIF6( stbir__simdfX_load( r0, input6 );      stbir__simdfX_load( r1, input6+stbir__simdfX_float_count );   stbir__simdfX_load( r2, input6+(2*stbir__simdfX_float_count) );   stbir__simdfX_load( r3, input6+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c6 );  stbir__simdfX_madd( o1, o1, r1, c6 );                         stbir__simdfX_madd( o2, o2, r2, c6 );                             stbir__simdfX_madd( o3, o3, r3, c6 ); )
      stbIF7( stbir__simdfX_load( r0, input7 );      stbir__simdfX_load( r1, input7+stbir__simdfX_float_count );   stbir__simdfX_load( r2, input7+(2*stbir__simdfX_float_count) );   stbir__simdfX_load( r3, input7+(3*stbir__simdfX_float_count) );
              stbir__simdfX_madd( o0, o0, r0, c7 );  stbir__simdfX_madd( o1, o1, r1, c7 );                         stbir__simdfX_madd( o2, o2, r2, c7 );                             stbir__simdfX_madd( o3, o3, r3, c7 ); )

      stbir__simdfX_store( output, o0 );             stbir__simdfX_store( output+stbir__simdfX_float_count, o1 );  stbir__simdfX_store( output+(2*stbir__simdfX_float_count), o2 );  stbir__simdfX_store( output+(3*stbir__simdfX_float_count), o3 );
      output += (4*stbir__simdfX_float_count);
      stbIF0( input0 += (4*stbir__simdfX_float_count); )
      stbIF1( input1 += (4*stbir__simdfX_float_count); )
      stbIF2( input2 += (4*stbir__simdfX_float_count); )
      stbIF3( input3 += (4*stbir__simdfX_float_count); )
      stbIF4( input4 += (4*stbir__simdfX_float_count); )
      stbIF5( input5 += (4*stbir__simdfX_float_count); )
      stbIF6( input6 += (4*stbir__simdfX_float_count); )
      stbIF7( input7 += (4*stbir__simdfX_float_count); )
    }

    STBIR_SIMD_NO_UNROLL_LOOP_START
    while ( ( (char*)input0_end - (char*) input0 ) >= 16 )
    {
      stbir__simdf o0, r0;
      STBIR_SIMD_NO_UNROLL(output);

      #ifdef STB_IMAGE_RESIZE_VERTICAL_CONTINUE
      stbIF0( stbir__simdf_load( o0, output );   stbir__simdf_load( r0, input0 ); stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c0 ) ); )
      #else
      stbIF0( stbir__simdf_load( r0, input0 );  stbir__simdf_mult( o0, r0, stbir__simdfX_cast_to_simdf4( c0 ) ); )
      #endif
      stbIF1( stbir__simdf_load( r0, input1 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c1 ) ); )
      stbIF2( stbir__simdf_load( r0, input2 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c2 ) ); )
      stbIF3( stbir__simdf_load( r0, input3 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c3 ) ); )
      stbIF4( stbir__simdf_load( r0, input4 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c4 ) ); )
      stbIF5( stbir__simdf_load( r0, input5 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c5 ) ); )
      stbIF6( stbir__simdf_load( r0, input6 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c6 ) ); )
      stbIF7( stbir__simdf_load( r0, input7 );  stbir__simdf_madd( o0, o0, r0, stbir__simdfX_cast_to_simdf4( c7 ) ); )

      stbir__simdf_store( output, o0 );
      output += 4;
      stbIF0( input0 += 4; )
      stbIF1( input1 += 4; )
      stbIF2( input2 += 4; )
      stbIF3( input3 += 4; )
      stbIF4( input4 += 4; )
      stbIF5( input5 += 4; )
      stbIF6( input6 += 4; )
      stbIF7( input7 += 4; )
    }
  }
  #else
  STBIR_NO_UNROLL_LOOP_START
  while ( ( (char*)input0_end - (char*) input0 ) >= 16 )
  {
    float o0, o1, o2, o3;
    STBIR_NO_UNROLL(output);
    #ifdef STB_IMAGE_RESIZE_VERTICAL_CONTINUE
    stbIF0( o0 = output[0] + input0[0] * c0s; o1 = output[1] + input0[1] * c0s; o2 = output[2] + input0[2] * c0s; o3 = output[3] + input0[3] * c0s; )
    #else
    stbIF0( o0  = input0[0] * c0s; o1  = input0[1] * c0s; o2  = input0[2] * c0s; o3  = input0[3] * c0s; )
    #endif
    stbIF1( o0 += input1[0] * c1s; o1 += input1[1] * c1s; o2 += input1[2] * c1s; o3 += input1[3] * c1s; )
    stbIF2( o0 += input2[0] * c2s; o1 += input2[1] * c2s; o2 += input2[2] * c2s; o3 += input2[3] * c2s; )
    stbIF3( o0 += input3[0] * c3s; o1 += input3[1] * c3s; o2 += input3[2] * c3s; o3 += input3[3] * c3s; )
    stbIF4( o0 += input4[0] * c4s; o1 += input4[1] * c4s; o2 += input4[2] * c4s; o3 += input4[3] * c4s; )
    stbIF5( o0 += input5[0] * c5s; o1 += input5[1] * c5s; o2 += input5[2] * c5s; o3 += input5[3] * c5s; )
    stbIF6( o0 += input6[0] * c6s; o1 += input6[1] * c6s; o2 += input6[2] * c6s; o3 += input6[3] * c6s; )
    stbIF7( o0 += input7[0] * c7s; o1 += input7[1] * c7s; o2 += input7[2] * c7s; o3 += input7[3] * c7s; )
    output[0] = o0; output[1] = o1; output[2] = o2; output[3] = o3;
    output += 4;
    stbIF0( input0 += 4; )
    stbIF1( input1 += 4; )
    stbIF2( input2 += 4; )
    stbIF3( input3 += 4; )
    stbIF4( input4 += 4; )
    stbIF5( input5 += 4; )
    stbIF6( input6 += 4; )
    stbIF7( input7 += 4; )
  }
  #endif
  STBIR_NO_UNROLL_LOOP_START
  while ( input0 < input0_end )
  {
    float o0;
    STBIR_NO_UNROLL(output);
    #ifdef STB_IMAGE_RESIZE_VERTICAL_CONTINUE
    stbIF0( o0 = output[0] + input0[0] * c0s; )
    #else
    stbIF0( o0  = input0[0] * c0s; )
    #endif
    stbIF1( o0 += input1[0] * c1s; )
    stbIF2( o0 += input2[0] * c2s; )
    stbIF3( o0 += input3[0] * c3s; )
    stbIF4( o0 += input4[0] * c4s; )
    stbIF5( o0 += input5[0] * c5s; )
    stbIF6( o0 += input6[0] * c6s; )
    stbIF7( o0 += input7[0] * c7s; )
    output[0] = o0;
    ++output;
    stbIF0( ++input0; )
    stbIF1( ++input1; )
    stbIF2( ++input2; )
    stbIF3( ++input3; )
    stbIF4( ++input4; )
    stbIF5( ++input5; )
    stbIF6( ++input6; )
    stbIF7( ++input7; )
  }
}

#undef stbIF0
#undef stbIF1
#undef stbIF2
#undef stbIF3
#undef stbIF4
#undef stbIF5
#undef stbIF6
#undef stbIF7
#undef STB_IMAGE_RESIZE_DO_VERTICALS
#undef STBIR__vertical_channels
#undef STB_IMAGE_RESIZE_DO_HORIZONTALS
#undef STBIR_strs_join24
#undef STBIR_strs_join14
#undef STBIR_chans
#ifdef STB_IMAGE_RESIZE_VERTICAL_CONTINUE
#undef STB_IMAGE_RESIZE_VERTICAL_CONTINUE
#endif

#else // !STB_IMAGE_RESIZE_DO_VERTICALS

#define STBIR_chans( start, end ) STBIR_strs_join1(start,STBIR__horizontal_channels,end)

#ifndef stbir__2_coeff_only
#define stbir__2_coeff_only()             \
    stbir__1_coeff_only();                \
    stbir__1_coeff_remnant(1);
#endif

#ifndef stbir__2_coeff_remnant
#define stbir__2_coeff_remnant( ofs )     \
    stbir__1_coeff_remnant(ofs);          \
    stbir__1_coeff_remnant((ofs)+1);
#endif

#ifndef stbir__3_coeff_only
#define stbir__3_coeff_only()             \
    stbir__2_coeff_only();                \
    stbir__1_coeff_remnant(2);
#endif

#ifndef stbir__3_coeff_remnant
#define stbir__3_coeff_remnant( ofs )     \
    stbir__2_coeff_remnant(ofs);          \
    stbir__1_coeff_remnant((ofs)+2);
#endif

#ifndef stbir__3_coeff_setup
#define stbir__3_coeff_setup()
#endif

#ifndef stbir__4_coeff_start
#define stbir__4_coeff_start()            \
    stbir__2_coeff_only();                \
    stbir__2_coeff_remnant(2);
#endif

#ifndef stbir__4_coeff_continue_from_4
#define stbir__4_coeff_continue_from_4( ofs )     \
    stbir__2_coeff_remnant(ofs);                  \
    stbir__2_coeff_remnant((ofs)+2);
#endif

#ifndef stbir__store_output_tiny
#define stbir__store_output_tiny stbir__store_output
#endif

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_1_coeff)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__1_coeff_only();
    stbir__store_output_tiny();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_2_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_2taps();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__2_coeff_only();
    stbir__store_output_tiny();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_3_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_finish3();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__3_coeff_only();
    stbir__store_output_tiny();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_4_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_5_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_rem1(4);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__1_coeff_remnant(4);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_6_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_rem2(4);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__2_coeff_remnant(4);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_7_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  stbir__3_coeff_setup();
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_rem3(4);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;

    stbir__4_coeff_start();
    stbir__3_coeff_remnant(4);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_8_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_cont(4);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__4_coeff_continue_from_4(4);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_9_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_cont(4);
    stbir__x4_rem1(8);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__4_coeff_continue_from_4(4);
    stbir__1_coeff_remnant(8);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_10_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_cont(4);
    stbir__x4_rem2(8);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__4_coeff_continue_from_4(4);
    stbir__2_coeff_remnant(8);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_11_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  stbir__3_coeff_setup();
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_cont(4);
    stbir__x4_rem3(8);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__4_coeff_continue_from_4(4);
    stbir__3_coeff_remnant(8);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_12_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_cont(4);
    stbir__x4_cont(8);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__4_coeff_continue_from_4(4);
    stbir__4_coeff_continue_from_4(8);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_13_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_cont(4);
    stbir__x4_cont(8);
    stbir__x4_rem1(12);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__4_coeff_continue_from_4(4);
    stbir__4_coeff_continue_from_4(8);
    stbir__1_coeff_remnant(12);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_14_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_cont(4);
    stbir__x4_cont(8);
    stbir__x4_rem2(12);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__4_coeff_continue_from_4(4);
    stbir__4_coeff_continue_from_4(8);
    stbir__2_coeff_remnant(12);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_15_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  stbir__3_coeff_setup();
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_cont(4);
    stbir__x4_cont(8);
    stbir__x4_rem3(12);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__4_coeff_continue_from_4(4);
    stbir__4_coeff_continue_from_4(8);
    stbir__3_coeff_remnant(12);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_16_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_cont(4);
    stbir__x4_cont(8);
    stbir__x4_cont(12);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__4_coeff_continue_from_4(4);
    stbir__4_coeff_continue_from_4(8);
    stbir__4_coeff_continue_from_4(12);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_17_coeffs)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  #ifdef stbir__x4_start
  // four outputs at a time while they last (one channel only, see the stbir__x4 macros)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__x4_setup();
    stbir__x4_start();
    stbir__x4_cont(4);
    stbir__x4_cont(8);
    stbir__x4_cont(12);
    stbir__x4_rem1(16);
    stbir__x4_finish();
    stbir__x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    float const * hc = column->coefficients;
    stbir__4_coeff_start();
    stbir__4_coeff_continue_from_4(4);
    stbir__4_coeff_continue_from_4(8);
    stbir__4_coeff_continue_from_4(12);
    stbir__1_coeff_remnant(16);
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_n_coeffs_mod0 )( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    int n = ( column->width - 4 + 3 ) >> 2;
    float const * hc = column->coefficients;

    stbir__4_coeff_start();
    STBIR_SIMD_NO_UNROLL_LOOP_START
    do {
      hc += 4;
      decode += STBIR__horizontal_channels * 4;
      stbir__4_coeff_continue_from_4( 0 );
      --n;
    } while ( n > 0 );
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_n_coeffs_mod1 )( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    int n = ( column->width - 5 + 3 ) >> 2;
    float const * hc = column->coefficients;

    stbir__4_coeff_start();
    STBIR_SIMD_NO_UNROLL_LOOP_START
    do {
      hc += 4;
      decode += STBIR__horizontal_channels * 4;
      stbir__4_coeff_continue_from_4( 0 );
      --n;
    } while ( n > 0 );
    stbir__1_coeff_remnant( 4 );
    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_n_coeffs_mod2 )( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    int n = ( column->width - 6 + 3 ) >> 2;
    float const * hc = column->coefficients;

    stbir__4_coeff_start();
    STBIR_SIMD_NO_UNROLL_LOOP_START
    do {
      hc += 4;
      decode += STBIR__horizontal_channels * 4;
      stbir__4_coeff_continue_from_4( 0 );
      --n;
    } while ( n > 0 );
    stbir__2_coeff_remnant( 4 );

    stbir__store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_n_coeffs_mod3 )( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  stbir__3_coeff_setup();
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    int n = ( column->width - 7 + 3 ) >> 2;
    float const * hc = column->coefficients;

    stbir__4_coeff_start();
    STBIR_SIMD_NO_UNROLL_LOOP_START
    do {
      hc += 4;
      decode += STBIR__horizontal_channels * 4;
      stbir__4_coeff_continue_from_4( 0 );
      --n;
    } while ( n > 0 );
    stbir__3_coeff_remnant( 4 );

    stbir__store_output();
  } while ( output < output_end );
}

// polyphase period 1 kernels (integer downsamples, see stbir__resample_horizontal_gather): every output uses the same
//   coefficient set, so it is loaded into registers once per run and the pixel loop is just the decode loads and
//   multiply-adds. the same steps as the kernels above, for the widths integer downsamples produce (4, 8: 2:1 with the
//   support 1 and 2 filters; 5, 9: the same one tap wider; 11/12/13 and 16/17: 3:1 and 4:1 with the support 2 filters -
//   3:1 trims to 11 because its outermost taps land exactly on the filter's zero at +-2).
//   only for the channel counts that define the step macros.
#ifdef stbir__period1_setup_4
static void STBIR_chans( stbir__horizontal_gather_,_channels_with_4_coeffs_period1)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  float const * hc = column->coefficients;
  int advance = column->advance;
  stbir__period1_setup_4()
  STBIR__UNUSED( entry_stride );
  #ifdef stbir__period1_x4_start
  // four outputs at a time while they last (one channel only)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__period1_x4_setup();
    stbir__period1_x4_start();
    stbir__period1_x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    stbir__period1_4_coeff_start();
    stbir__period1_store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_5_coeffs_period1)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  float const * hc = column->coefficients;
  int advance = column->advance;
  stbir__period1_setup_5()
  STBIR__UNUSED( entry_stride );
  #ifdef stbir__period1_x4_start
  // four outputs at a time while they last (one channel only)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__period1_x4_setup();
    stbir__period1_x4_start();
    stbir__period1_x4_remnant(4);
    stbir__period1_x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    stbir__period1_4_coeff_start();
    stbir__period1_1_coeff_remnant(4);
    stbir__period1_store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_8_coeffs_period1)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  float const * hc = column->coefficients;
  int advance = column->advance;
  stbir__period1_setup_8()
  STBIR__UNUSED( entry_stride );
  #ifdef stbir__period1_x4_start
  // four outputs at a time while they last (one channel only)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__period1_x4_setup();
    stbir__period1_x4_start();
    stbir__period1_x4_continue();
    stbir__period1_x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    stbir__period1_4_coeff_start();
    stbir__period1_4_coeff_continue();
    stbir__period1_store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_9_coeffs_period1)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  float const * hc = column->coefficients;
  int advance = column->advance;
  stbir__period1_setup_9()
  STBIR__UNUSED( entry_stride );
  #ifdef stbir__period1_x4_start
  // four outputs at a time while they last (one channel only)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__period1_x4_setup();
    stbir__period1_x4_start();
    stbir__period1_x4_continue();
    stbir__period1_x4_remnant(8);
    stbir__period1_x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    stbir__period1_4_coeff_start();
    stbir__period1_4_coeff_continue();
    stbir__period1_1_coeff_remnant(8);
    stbir__period1_store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_11_coeffs_period1)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  float const * hc = column->coefficients;
  int advance = column->advance;
  stbir__period1_setup_11()
  STBIR__UNUSED( entry_stride );
  #ifdef stbir__period1_x4_start
  // four outputs at a time while they last (one channel only)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__period1_x4_setup();
    stbir__period1_x4_start();
    stbir__period1_x4_continue();
    stbir__period1_x4_remnant3(8);
    stbir__period1_x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    stbir__period1_4_coeff_start();
    stbir__period1_4_coeff_continue();
    stbir__period1_3_coeff_remnant(8);
    stbir__period1_store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_12_coeffs_period1)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  float const * hc = column->coefficients;
  int advance = column->advance;
  stbir__period1_setup_12()
  STBIR__UNUSED( entry_stride );
  #ifdef stbir__period1_x4_start
  // four outputs at a time while they last (one channel only)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__period1_x4_setup();
    stbir__period1_x4_start();
    stbir__period1_x4_continue();
    stbir__period1_x4_continue2();
    stbir__period1_x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    stbir__period1_4_coeff_start();
    stbir__period1_4_coeff_continue();
    stbir__period1_4_coeff_continue2();
    stbir__period1_store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_13_coeffs_period1)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  float const * hc = column->coefficients;
  int advance = column->advance;
  stbir__period1_setup_13()
  STBIR__UNUSED( entry_stride );
  #ifdef stbir__period1_x4_start
  // four outputs at a time while they last (one channel only)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__period1_x4_setup();
    stbir__period1_x4_start();
    stbir__period1_x4_continue();
    stbir__period1_x4_continue2();
    stbir__period1_x4_remnant(12);
    stbir__period1_x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    stbir__period1_4_coeff_start();
    stbir__period1_4_coeff_continue();
    stbir__period1_4_coeff_continue2();
    stbir__period1_1_coeff_remnant(12);
    stbir__period1_store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_16_coeffs_period1)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  float const * hc = column->coefficients;
  int advance = column->advance;
  stbir__period1_setup_16()
  STBIR__UNUSED( entry_stride );
  #ifdef stbir__period1_x4_start
  // four outputs at a time while they last (one channel only)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__period1_x4_setup();
    stbir__period1_x4_start();
    stbir__period1_x4_continue();
    stbir__period1_x4_continue2();
    stbir__period1_x4_continue3();
    stbir__period1_x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    stbir__period1_4_coeff_start();
    stbir__period1_4_coeff_continue();
    stbir__period1_4_coeff_continue2();
    stbir__period1_4_coeff_continue3();
    stbir__period1_store_output();
  } while ( output < output_end );
}

static void STBIR_chans( stbir__horizontal_gather_,_channels_with_17_coeffs_period1)( float * output_buffer, unsigned int output_sub_size, float const * decode_buffer, stbir__column_entry const * column, int entry_stride )
{
  float const * output_end = output_buffer + (ptrdiff_t)output_sub_size * STBIR__horizontal_channels;
  float STBIR_SIMD_STREAMOUT_PTR( * ) output = output_buffer;
  float const * hc = column->coefficients;
  int advance = column->advance;
  stbir__period1_setup_17()
  STBIR__UNUSED( entry_stride );
  #ifdef stbir__period1_x4_start
  // four outputs at a time while they last (one channel only)
  while ( ( output_end - output ) >= 4 )
  {
    stbir__period1_x4_setup();
    stbir__period1_x4_start();
    stbir__period1_x4_continue();
    stbir__period1_x4_continue2();
    stbir__period1_x4_continue3();
    stbir__period1_x4_remnant(16);
    stbir__period1_x4_store();
  }
  if ( output < output_end )
  #endif
  STBIR_SIMD_NO_UNROLL_LOOP_START
  do {
    float const * decode = decode_buffer;
    stbir__period1_4_coeff_start();
    stbir__period1_4_coeff_continue();
    stbir__period1_4_coeff_continue2();
    stbir__period1_4_coeff_continue3();
    stbir__period1_1_coeff_remnant(16);
    stbir__period1_store_output();
  } while ( output < output_end );
}

// by coefficient count (null = use the normal kernel over the one entry)
static stbir__horizontal_gather_channels_func * STBIR_chans(stbir__horizontal_gather_,_channels_period1_funcs)[18]=
{
  0, 0, 0, 0,
  STBIR_chans(stbir__horizontal_gather_,_channels_with_4_coeffs_period1),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_5_coeffs_period1),
  0, 0,
  STBIR_chans(stbir__horizontal_gather_,_channels_with_8_coeffs_period1),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_9_coeffs_period1),
  0,
  STBIR_chans(stbir__horizontal_gather_,_channels_with_11_coeffs_period1),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_12_coeffs_period1),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_13_coeffs_period1),
  0, 0,
  STBIR_chans(stbir__horizontal_gather_,_channels_with_16_coeffs_period1),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_17_coeffs_period1)
};
#else
static stbir__horizontal_gather_channels_func * STBIR_chans(stbir__horizontal_gather_,_channels_period1_funcs)[18]= { 0 };
#endif

static stbir__horizontal_gather_channels_func * STBIR_chans(stbir__horizontal_gather_,_channels_with_n_coeffs_funcs)[4]=
{
  STBIR_chans(stbir__horizontal_gather_,_channels_with_n_coeffs_mod0),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_n_coeffs_mod1),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_n_coeffs_mod2),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_n_coeffs_mod3),
};

static stbir__horizontal_gather_channels_func * STBIR_chans(stbir__horizontal_gather_,_channels_funcs)[17]=
{
  STBIR_chans(stbir__horizontal_gather_,_channels_with_1_coeff),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_2_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_3_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_4_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_5_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_6_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_7_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_8_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_9_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_10_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_11_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_12_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_13_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_14_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_15_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_16_coeffs),
  STBIR_chans(stbir__horizontal_gather_,_channels_with_17_coeffs),
};

#ifdef stbir__period1_setup_4
#undef stbir__period1_setup_4
#undef stbir__period1_setup_5
#undef stbir__period1_setup_8
#undef stbir__period1_setup_9
#undef stbir__period1_setup_11
#undef stbir__period1_setup_12
#undef stbir__period1_setup_13
#undef stbir__period1_setup_16
#undef stbir__period1_setup_17
#undef stbir__period1_4_coeff_start
#undef stbir__period1_4_coeff_continue
#undef stbir__period1_4_coeff_continue2
#undef stbir__period1_4_coeff_continue3
#undef stbir__period1_1_coeff_remnant
#undef stbir__period1_3_coeff_remnant
#undef stbir__period1_store_output
#endif
#ifdef stbir__period1_x4_start
#undef stbir__period1_x4_setup
#undef stbir__period1_x4_start
#undef stbir__period1_x4_next
#undef stbir__period1_x4_continue
#undef stbir__period1_x4_continue2
#undef stbir__period1_x4_continue3
#undef stbir__period1_x4_remnant
#undef stbir__period1_x4_remnant3
#undef stbir__period1_x4_store
#endif
#undef STBIR__horizontal_channels
#undef STB_IMAGE_RESIZE_DO_HORIZONTALS
#ifdef stbir__x4_start
#undef stbir__x4_setup
#undef stbir__x4_start
#undef stbir__x4_cont
#undef stbir__x4_rem1
#undef stbir__x4_rem2
#undef stbir__x4_rem3
#undef stbir__x4_2taps
#undef stbir__x4_finish
#undef stbir__x4_finish3
#undef stbir__x4_store
#endif
#undef stbir__1_coeff_only
#undef stbir__1_coeff_remnant
#undef stbir__2_coeff_only
#undef stbir__2_coeff_remnant
#undef stbir__3_coeff_only
#undef stbir__3_coeff_remnant
#undef stbir__3_coeff_setup
#undef stbir__4_coeff_start
#undef stbir__4_coeff_continue_from_4
#undef stbir__store_output
#undef stbir__store_output_tiny
#undef STBIR_chans

#endif  // HORIZONALS

#undef STBIR_strs_join2
#undef STBIR_strs_join1

#endif // STB_IMAGE_RESIZE_DO_HORIZONTALS/VERTICALS/CODERS

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
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
*/
