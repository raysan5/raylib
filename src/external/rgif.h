/**********************************************************************************************
*
*   rgif.h v0.5
*
*   Original implementation (gif.h) by Charlie Tangora [ctangora -at- gmail -dot- com]
*   adapted to C99, reformatted and renamed by Ramon Santamaria (@raysan5)
*
*   This file offers a simple, very limited way to create animated GIFs directly in code.
*
*   Those looking for particular cleverness are likely to be disappointed; it's pretty 
*   much a straight-ahead implementation of the GIF format with optional Floyd-Steinberg 
*   dithering. (It does at least use delta encoding - only the changed portions of each 
*   frame are saved.) 
*
*   So resulting files are often quite large. The hope is that it will be handy nonetheless
*   as a quick and easily-integrated way for programs to spit out animations.
*
*   Only RGBA8 is currently supported as an input format. (The alpha is ignored.)
*
*   CONFIGURATION:
*
*   #define RGIF_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   USAGE:
*       1) Create a GifWriter struct. Pass it to GifBegin() to initialize and write the header.
*       2) Pass subsequent frames to GifWriteFrame().
*       3) Finally, call GifEnd() to close the file handle and free memory.
*
*
*   LICENSE: This software is available under 2 licenses -- choose whichever you prefer
*
*   ALTERNATIVE A - MIT License
*
*   Copyright (c) 2017-2019 Ramon Santamaria (@raysan5)
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of 
*   this software and associated documentation files (the "Software"), to deal in 
*   the Software without restriction, including without limitation the rights to 
*   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
*   of the Software, and to permit persons to whom the Software is furnished to do 
*   so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all 
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*   ------------------------------------------------------------------------------
*
*   ALTERNATIVE B - public domain (www.unlicense.org)
*
*   This is free and unencumbered software released into the public domain.
*   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
*   software, either in source code form or as a compiled binary, for any purpose,
*   commercial or non-commercial, and by any means.
*   
*   In jurisdictions that recognize copyright laws, the author or authors of this
*   software dedicate any and all copyright interest in the software to the public
*   domain. We make this dedication for the benefit of the public at large and to
*   the detriment of our heirs and successors. We intend this dedication to be an
*   overt act of relinquishment in perpetuity of all present and future rights to
*   this software under copyright law.
*   
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
*   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
*   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
**********************************************************************************************/

#ifndef RGIF_H
#define RGIF_H

#include <stdio.h>          // Required for: FILE

//#define RGIF_STATIC
#ifdef RGIF_STATIC
    #define RGIFDEF static              // Functions just visible to module including this file
#else
    #ifdef __cplusplus
        #define RGIFDEF extern "C"      // Functions visible from other files (no name mangling of functions in C++)
    #else
        #define RGIFDEF extern          // Functions visible from other files
    #endif
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// NOTE: By default use bitDepth = 8, dither = false
RGIFDEF bool GifBegin(const char *filename, unsigned int width, unsigned int height, unsigned int delay, unsigned int bitDepth, bool dither);
RGIFDEF bool GifWriteFrame(const unsigned char *image, unsigned int width, unsigned int height, unsigned int delay, int bitDepth, bool dither);
RGIFDEF bool GifEnd();

#endif // RGIF_H


/***********************************************************************************
*
 *  GIF IMPLEMENTATION
*
************************************************************************************/

#if defined(RGIF_IMPLEMENTATION)

#include <stdio.h>          // Required for: FILE, fopen(), fclose()
#include <string.h>         // Required for: memcpy()

// Define these macros to hook into a custom memory allocator.
// RGIF_TEMP_MALLOC and RGIF_TEMP_FREE will only be called in stack fashion - frees in the reverse order of mallocs
// and any temp memory allocated by a function will be freed before it exits.
#if !defined(RGIF_TEMP_MALLOC)
    #include <stdlib.h>
    
    #define RGIF_TEMP_MALLOC malloc
    #define RGIF_TEMP_FREE free
#endif

// Check if custom malloc/free functions defined, if not, using standard ones
// RGIF_MALLOC and RGIF_FREE are used only by GifBegin and GifEnd respectively, 
// to allocate a buffer the size of the image, which is used to find changed pixels for delta-encoding.
#if !defined(RGIF_MALLOC)
    #include <stdlib.h>     // Required for: malloc(), free()

    #define RGIF_MALLOC(size)  malloc(size)
    #define RGIF_FREE(ptr)     free(ptr)
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define GIFMIN(a, b) (((a)<(b))?(a):(b))
#define GIFMAX(a, b) (((a)>(b))?(a):(b))
#define GIFABS(x) ((x)<0?-(x):(x))

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Gif palette structure
typedef struct GifPalette {
    int bitDepth;

    unsigned char r[256];
    unsigned char g[256];
    unsigned char b[256];
    
    // k-d tree over RGB space, organized in heap fashion
    // i.e. left child of node i is node i*2, right child is node i*2 + 1
    // nodes 256-511 are implicitly the leaves, containing a color
    unsigned char treeSplitElt[255];
    unsigned char treeSplit[255];
} GifPalette;


// Simple structure to write out the LZW-compressed 
// portion of the imageone bit at a time
typedef struct GifBitStatus {
    unsigned char bitIndex;  // how many bits in the partial byte written so far
    unsigned char byte;      // current partial byte
    
    unsigned int chunkIndex;
    unsigned char chunk[256];   // bytes are written in here until we have 256 of them, then written to the file
} GifBitStatus;

// The LZW dictionary is a 256-ary tree constructed 
// as the file is encoded, this is one node
typedef struct GifLzwNode {
    unsigned short m_next[256];
} GifLzwNode;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const int gifTransparentIndex = 0;       // Transparent color index

static FILE *gifFile;
unsigned char *gifFrame;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void GifGetClosestPaletteColor(GifPalette *pPal, int r, int g, int b, int *bestInd, int *bestDiff, int treeRoot);
static void GifSwapPixels(unsigned char *image, int pixA, int pixB);
static int GifPartition(unsigned char *image, const int left, const int right, const int elt, int pivotIndex);
static void GifPartitionByMedian(unsigned char *image, int left, int right, int com, int neededCenter);
static void GifSplitPalette(unsigned char *image, int numPixels, int firstElt, int lastElt, int splitElt, int splitDist, int treeNode, bool buildForDither, GifPalette *pal);
static int GifPickChangedPixels(const unsigned char *lastFrame, unsigned char *frame, int numPixels);
static void GifMakePalette(const unsigned char *lastFrame, const unsigned char *nextFrame, unsigned int width, unsigned int height, int bitDepth, bool buildForDither, GifPalette *pPal);
static void GifDitherImage(const unsigned char *lastFrame, const unsigned char *nextFrame, unsigned char *outFrame, unsigned int width, unsigned int height, GifPalette *pPal);
static void GifThresholdImage(const unsigned char *lastFrame, const unsigned char *nextFrame, unsigned char *outFrame, unsigned int width, unsigned int height, GifPalette *pPal);
static void GifWriteBit(GifBitStatus *stat, unsigned int bit);
static void GifWriteChunk(FILE *f, GifBitStatus *stat);
static void GifWriteCode(FILE *f, GifBitStatus *stat, unsigned int code, unsigned int length);
static void GifWritePalette(const GifPalette *pPal, FILE *f);
static void GifWriteLzwImage(FILE *f, unsigned char *image, unsigned int left, unsigned int top, unsigned int width, unsigned int height, unsigned int delay, GifPalette *pPal);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Creates a gif file
// NOTE: Initializes internal file pointer (only one gif recording at a time)
// The delay value is the time between frames in hundredths of a second - note that not all viewers pay much attention to this value.
RGIFDEF bool GifBegin(const char *filename, unsigned int width, unsigned int height, unsigned int delay, unsigned int bitDepth, bool dither)
{
#if _MSC_VER >= 1400
	gifFile = 0;
    fopen_s(&gifFile, filename, "wb");
#else
    gifFile = fopen(filename, "wb");
#endif

    if (!gifFile) return false;
    
    // Allocate space for one gif frame
    gifFrame = (unsigned char *)RGIF_MALLOC(width*height*4);
    
    // GIF Header
    fputs("GIF89a",gifFile);
    
    // Reference: http://www.onicos.com/staff/iz/formats/gif.html
    
    // GIF Screen Descriptor
    fputc(width & 0xff, gifFile);
    fputc((width >> 8) & 0xff, gifFile);      // Screen width (2 byte)
    fputc(height & 0xff, gifFile);
    fputc((height >> 8) & 0xff, gifFile);     // Screen height (2 byte)
    
    fputc(0xf0, gifFile);  // Color table flags: unsorted global color table of 2 entries (1 byte, bit-flags)
    fputc(0, gifFile);     // Background color index
    fputc(0, gifFile);     // Pixel Aspect Ratio (square, we need to specify this because it's 1989)
    
    // GIF Global Color table (just a dummy palette)
    // Color 0: black
    fputc(0, gifFile);
    fputc(0, gifFile); 
    fputc(0, gifFile);
    // Color 1: also black
    fputc(0, gifFile);
    fputc(0, gifFile);
    fputc(0, gifFile);
    
    if (delay != 0)
    {
        // Application Extension Block (19 bytes long)
        fputc(0x21, gifFile);     // GIF Extension code
        fputc(0xff, gifFile);     // Application Extension Label
        fputc(11, gifFile);       // Length of Application Block (11 byte)
        fputs("NETSCAPE2.0", gifFile); // Application Identifier (Netscape 2.0 block)
        
        fputc(0x03, gifFile);     // Length of Data Sub-Block (3 bytes)
        fputc(0x01, gifFile);     // 0x01
        fputc(0x00, gifFile);     // This specifies the number of times,  
        fputc(0x00, gifFile);     // the loop should be executed (infinitely)
        
        fputc(0x00, gifFile);     // Data Sub-Block Terminator.
    }
    
    return true;
}

// Writes out a new frame to a GIF in progress.
// NOTE: gifFile should have been initialized with GifBegin()
// AFAIK, it is legal to use different bit depths for different frames of an image -
// this may be handy to save bits in animations that don't change much.
RGIFDEF bool GifWriteFrame(const unsigned char *image, unsigned int width, unsigned int height, unsigned int delay, int bitDepth, bool dither)
{
    if (!gifFile) return false;
    
    const unsigned char *oldImage = gifFrame;
    
    GifPalette pal;
    GifMakePalette((dither ? NULL : oldImage), image, width, height, bitDepth, dither, &pal);
    
    if (dither) GifDitherImage(oldImage, image, gifFrame, width, height, &pal);
    else GifThresholdImage(oldImage, image, gifFrame, width, height, &pal);
    
    GifWriteLzwImage(gifFile, gifFrame, 0, 0, width, height, delay, &pal);
    
    return true;
}

// Writes the EOF code, closes the file handle, and frees temp memory used by a GIF.
// Many if not most viewers will still display a GIF properly if the EOF code is missing,
// but it's still a good idea to write it out.
RGIFDEF bool GifEnd()
{
    if (!gifFile) return false;
    
    fputc(0x3b, gifFile);     // Trailer (end of file)
    fclose(gifFile);
    
    RGIF_FREE(gifFrame);
    
    gifFile = NULL;
    gifFrame = NULL;
    
    return true;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
// walks the k-d tree to pick the palette entry for a desired color.
// Takes as in/out parameters the current best color and its error -
// only changes them if it finds a better color in its subtree.
// this is the major hotspot in the code at the moment.
static void GifGetClosestPaletteColor(GifPalette *pPal, int r, int g, int b, int *bestInd, int *bestDiff, int treeRoot)
{
    // base case, reached the bottom of the tree
    if (treeRoot > (1<<pPal->bitDepth)-1)
    {
        int ind = treeRoot-(1<<pPal->bitDepth);
        if (ind == gifTransparentIndex) return;
        
        // check whether this color is better than the current winner
        int r_err = r - ((int)pPal->r[ind]);
        int g_err = g - ((int)pPal->g[ind]);
        int b_err = b - ((int)pPal->b[ind]);
        int diff = GIFABS(r_err)+GIFABS(g_err)+GIFABS(b_err);
        
        if (diff < *bestDiff)
        {
            *bestInd = ind;
            *bestDiff = diff;
        }
        
        return;
    }
    
    // take the appropriate color (r, g, or b) for this node of the k-d tree
    int comps[3]; comps[0] = r; comps[1] = g; comps[2] = b;
    int splitComp = comps[pPal->treeSplitElt[treeRoot]];
    
    int splitPos = pPal->treeSplit[treeRoot];
    if (splitPos > splitComp)
    {
        // check the left subtree
        GifGetClosestPaletteColor(pPal, r, g, b, bestInd, bestDiff, treeRoot*2);
        
        if (*bestDiff > (splitPos - splitComp))
        {
            // cannot prove there's not a better value in the right subtree, check that too
            GifGetClosestPaletteColor(pPal, r, g, b, bestInd, bestDiff, treeRoot*2 + 1);
        }
    }
    else
    {
        GifGetClosestPaletteColor(pPal, r, g, b, bestInd, bestDiff, treeRoot*2 + 1);
        
        if (*bestDiff > splitComp - splitPos)
        {
            GifGetClosestPaletteColor(pPal, r, g, b, bestInd, bestDiff, treeRoot*2);
        }
    }
}

static void GifSwapPixels(unsigned char *image, int pixA, int pixB)
{
    unsigned char rA = image[pixA*4];
    unsigned char gA = image[pixA*4 + 1];
    unsigned char bA = image[pixA*4+2];
    unsigned char aA = image[pixA*4+3];
    
    unsigned char rB = image[pixB*4];
    unsigned char gB = image[pixB*4 + 1];
    unsigned char bB = image[pixB*4+2];
    unsigned char aB = image[pixA*4+3];
    
    image[pixA*4] = rB;
    image[pixA*4 + 1] = gB;
    image[pixA*4+2] = bB;
    image[pixA*4+3] = aB;
    
    image[pixB*4] = rA;
    image[pixB*4 + 1] = gA;
    image[pixB*4+2] = bA;
    image[pixB*4+3] = aA;
}

// just the partition operation from quicksort
static int GifPartition(unsigned char *image, const int left, const int right, const int elt, int pivotIndex)
{
    const int pivotValue = image[(pivotIndex)*4+elt];
    GifSwapPixels(image, pivotIndex, right-1);
    int storeIndex = left;
    bool split = 0;
    for (int ii=left; ii<right-1; ++ii)
    {
        int arrayVal = image[ii*4+elt];
        if (arrayVal < pivotValue)
        {
            GifSwapPixels(image, ii, storeIndex);
            ++storeIndex;
        }
        else if (arrayVal == pivotValue)
        {
            if (split)
            {
                GifSwapPixels(image, ii, storeIndex);
                ++storeIndex;
            }
            split = !split;
        }
    }
    GifSwapPixels(image, storeIndex, right-1);
    return storeIndex;
}

// Perform an incomplete sort, finding all elements above and below the desired median
static void GifPartitionByMedian(unsigned char *image, int left, int right, int com, int neededCenter)
{
    if (left < right-1)
    {
        int pivotIndex = left + (right-left)/2;
    
        pivotIndex = GifPartition(image, left, right, com, pivotIndex);
        
        // Only "sort" the section of the array that contains the median
        if (pivotIndex > neededCenter)
            GifPartitionByMedian(image, left, pivotIndex, com, neededCenter);
        
        if (pivotIndex < neededCenter)
            GifPartitionByMedian(image, pivotIndex + 1, right, com, neededCenter);
    }
}

// Builds a palette by creating a balanced k-d tree of all pixels in the image
static void GifSplitPalette(unsigned char *image, int numPixels, int firstElt, int lastElt, int splitElt, int splitDist, 
                            int treeNode, bool buildForDither, GifPalette *pal)
{
    if (lastElt <= firstElt || numPixels == 0)
        return;
    
    // base case, bottom of the tree
    if (lastElt == firstElt + 1)
    {
        if (buildForDither)
        {
            // Dithering needs at least one color as dark as anything
            // in the image and at least one brightest color -
            // otherwise it builds up error and produces strange artifacts
            if (firstElt == 1)
            {
                // special case: the darkest color in the image
                unsigned int r=255, g=255, b=255;
                for (int ii=0; ii<numPixels; ++ii)
                {
                    r = GIFMIN(r, image[ii*4+0]);
                    g = GIFMIN(g, image[ii*4 + 1]);
                    b = GIFMIN(b, image[ii*4+2]);
                }
                
                pal->r[firstElt] = r;
                pal->g[firstElt] = g;
                pal->b[firstElt] = b;
                
                return;
            }
            
            if (firstElt == (1 << pal->bitDepth)-1)
            {
                // special case: the lightest color in the image
                unsigned int r=0, g=0, b=0;
                for (int ii=0; ii<numPixels; ++ii)
                {
                    r = GIFMAX(r, image[ii*4+0]);
                    g = GIFMAX(g, image[ii*4 + 1]);
                    b = GIFMAX(b, image[ii*4+2]);
                }
                
                pal->r[firstElt] = r;
                pal->g[firstElt] = g;
                pal->b[firstElt] = b;
                
                return;
            }
        }
        
        // otherwise, take the average of all colors in this subcube
        unsigned long long r=0, g=0, b=0;
        for (int ii=0; ii<numPixels; ++ii)
        {
            r += image[ii*4+0];
            g += image[ii*4 + 1];
            b += image[ii*4+2];
        }
        
        r += numPixels / 2;  // round to nearest
        g += numPixels / 2;
        b += numPixels / 2;
        
        r /= numPixels;
        g /= numPixels;
        b /= numPixels;
        
        pal->r[firstElt] = (unsigned char)r;
        pal->g[firstElt] = (unsigned char)g;
        pal->b[firstElt] = (unsigned char)b;
        
        return;
    }
    
    // Find the axis with the largest range
    int minR = 255, maxR = 0;
    int minG = 255, maxG = 0;
    int minB = 255, maxB = 0;
    for (int ii=0; ii<numPixels; ++ii)
    {
        int r = image[ii*4+0];
        int g = image[ii*4 + 1];
        int b = image[ii*4+2];
        
        if (r > maxR) maxR = r;
        if (r < minR) minR = r;
        
        if (g > maxG) maxG = g;
        if (g < minG) minG = g;
        
        if (b > maxB) maxB = b;
        if (b < minB) minB = b;
    }
    
    int rRange = maxR - minR;
    int gRange = maxG - minG;
    int bRange = maxB - minB;
    
    // and split along that axis. (incidentally, this means this isn't a "proper" k-d tree but I don't know what else to call it)
    int splitCom = 1;
    if (bRange > gRange) splitCom = 2;
    if (rRange > bRange && rRange > gRange) splitCom = 0;
    
    int subPixelsA = numPixels  *(splitElt - firstElt) / (lastElt - firstElt);
    int subPixelsB = numPixels-subPixelsA;
    
    GifPartitionByMedian(image, 0, numPixels, splitCom, subPixelsA);
    
    pal->treeSplitElt[treeNode] = splitCom;
    pal->treeSplit[treeNode] = image[subPixelsA*4+splitCom];
    
    GifSplitPalette(image,              subPixelsA, firstElt, splitElt, splitElt-splitDist, splitDist/2, treeNode*2,   buildForDither, pal);
    GifSplitPalette(image+subPixelsA*4, subPixelsB, splitElt, lastElt,  splitElt+splitDist, splitDist/2, treeNode*2 + 1, buildForDither, pal);
}

// Finds all pixels that have changed from the previous image and
// moves them to the fromt of th buffer.
// This allows us to build a palette optimized for the colors of the
// changed pixels only.
static int GifPickChangedPixels(const unsigned char *lastFrame, unsigned char *frame, int numPixels)
{
    int numChanged = 0;
    unsigned char *writeIter = frame;
    
    for (int ii=0; ii<numPixels; ++ii)
    {
        if (lastFrame[0] != frame[0] ||
           lastFrame[1] != frame[1] ||
           lastFrame[2] != frame[2])
        {
            writeIter[0] = frame[0];
            writeIter[1] = frame[1];
            writeIter[2] = frame[2];
            ++numChanged;
            writeIter += 4;
        }
        lastFrame += 4;
        frame += 4;
    }
    
    return numChanged;
}

// Creates a palette by placing all the image pixels in a k-d tree and then averaging the blocks at the bottom.
// This is known as the "modified median split" technique
static void GifMakePalette(const unsigned char *lastFrame, const unsigned char *nextFrame, unsigned int width, unsigned int height, int bitDepth, bool buildForDither, GifPalette *pPal)
{
    pPal->bitDepth = bitDepth;
    
    // SplitPalette is destructive (it sorts the pixels by color) so
    // we must create a copy of the image for it to destroy
    int imageSize = width*height*4*sizeof(unsigned char);
    unsigned char *destroyableImage = (unsigned char*)RGIF_TEMP_MALLOC(imageSize);
    memcpy(destroyableImage, nextFrame, imageSize);
    
    int numPixels = width*height;
    if (lastFrame)
        numPixels = GifPickChangedPixels(lastFrame, destroyableImage, numPixels);
    
    const int lastElt = 1 << bitDepth;
    const int splitElt = lastElt/2;
    const int splitDist = splitElt/2;
    
    GifSplitPalette(destroyableImage, numPixels, 1, lastElt, splitElt, splitDist, 1, buildForDither, pPal);
    
    RGIF_TEMP_FREE(destroyableImage);
    
    // add the bottom node for the transparency index
    pPal->treeSplit[1 << (bitDepth-1)] = 0;
    pPal->treeSplitElt[1 << (bitDepth-1)] = 0;
    
    pPal->r[0] = pPal->g[0] = pPal->b[0] = 0;
}

// Implements Floyd-Steinberg dithering, writes palette value to alpha
static void GifDitherImage(const unsigned char *lastFrame, const unsigned char *nextFrame, unsigned char *outFrame, unsigned int width, unsigned int height, GifPalette *pPal)
{
    int numPixels = width*height;
    
    // quantPixels initially holds color*256 for all pixels
    // The extra 8 bits of precision allow for sub-single-color error values
    // to be propagated
    int *quantPixels = (int*)RGIF_TEMP_MALLOC(sizeof(int)*numPixels*4);
    
    for (int ii=0; ii<numPixels*4; ++ii)
    {
        unsigned char pix = nextFrame[ii];
        int pix16 = (int)pix*256;
        quantPixels[ii] = pix16;
    }
    
    for (unsigned int yy=0; yy<height; ++yy)
    {
        for (unsigned int xx=0; xx<width; ++xx)
        {
            int *nextPix = quantPixels + 4*(yy*width+xx);
            const unsigned char *lastPix = lastFrame? lastFrame + 4*(yy*width+xx) : NULL;
            
            // Compute the colors we want (rounding to nearest)
            int rr = (nextPix[0] + 127) / 256;
            int gg = (nextPix[1] + 127) / 256;
            int bb = (nextPix[2] + 127) / 256;
            
            // if it happens that we want the color from last frame, then just write out
            // a transparent pixel
            if (lastFrame &&
               lastPix[0] == rr &&
               lastPix[1] == gg &&
               lastPix[2] == bb)
            {
                nextPix[0] = rr;
                nextPix[1] = gg;
                nextPix[2] = bb;
                nextPix[3] = gifTransparentIndex;
                continue;
            }
            
            int bestDiff = 1000000;
            int bestInd = gifTransparentIndex;
            
            // Search the palete
            GifGetClosestPaletteColor(pPal, rr, gg, bb, &bestInd, &bestDiff, 1);
            
            // Write the result to the temp buffer
            int r_err = nextPix[0] - (int)(pPal->r[bestInd])*256;
            int g_err = nextPix[1] - (int)(pPal->g[bestInd])*256;
            int b_err = nextPix[2] - (int)(pPal->b[bestInd])*256;
            
            nextPix[0] = pPal->r[bestInd];
            nextPix[1] = pPal->g[bestInd];
            nextPix[2] = pPal->b[bestInd];
            nextPix[3] = bestInd;
            
            // Propagate the error to the four adjacent locations
            // that we haven't touched yet
            int quantloc_7 = (yy*width+xx + 1);
            int quantloc_3 = (yy*width+width+xx-1);
            int quantloc_5 = (yy*width+width+xx);
            int quantloc_1 = (yy*width+width+xx + 1);
            
            if (quantloc_7 < numPixels)
            {
                int *pix7 = quantPixels+4*quantloc_7;
                pix7[0] += GIFMAX(-pix7[0], r_err*7 / 16);
                pix7[1] += GIFMAX(-pix7[1], g_err*7 / 16);
                pix7[2] += GIFMAX(-pix7[2], b_err*7 / 16);
            }
            
            if (quantloc_3 < numPixels)
            {
                int *pix3 = quantPixels+4*quantloc_3;
                pix3[0] += GIFMAX(-pix3[0], r_err*3 / 16);
                pix3[1] += GIFMAX(-pix3[1], g_err*3 / 16);
                pix3[2] += GIFMAX(-pix3[2], b_err*3 / 16);
            }
            
            if (quantloc_5 < numPixels)
            {
                int *pix5 = quantPixels+4*quantloc_5;
                pix5[0] += GIFMAX(-pix5[0], r_err*5 / 16);
                pix5[1] += GIFMAX(-pix5[1], g_err*5 / 16);
                pix5[2] += GIFMAX(-pix5[2], b_err*5 / 16);
            }
            
            if (quantloc_1 < numPixels)
            {
                int *pix1 = quantPixels+4*quantloc_1;
                pix1[0] += GIFMAX(-pix1[0], r_err / 16);
                pix1[1] += GIFMAX(-pix1[1], g_err / 16);
                pix1[2] += GIFMAX(-pix1[2], b_err / 16);
            }
        }
    }
    
    // Copy the palettized result to the output buffer
    for (int ii=0; ii<numPixels*4; ++ii)
    {
        outFrame[ii] = quantPixels[ii];
    }
    
    RGIF_TEMP_FREE(quantPixels);
}

// Picks palette colors for the image using simple thresholding, no dithering
static void GifThresholdImage(const unsigned char *lastFrame, const unsigned char *nextFrame, unsigned char *outFrame, unsigned int width, unsigned int height, GifPalette *pPal)
{
    unsigned int numPixels = width*height;
    for (unsigned int ii=0; ii<numPixels; ++ii)
    {
        // if a previous color is available, and it matches the current color,
        // set the pixel to transparent
        if (lastFrame &&
           lastFrame[0] == nextFrame[0] &&
           lastFrame[1] == nextFrame[1] &&
           lastFrame[2] == nextFrame[2])
        {
            outFrame[0] = lastFrame[0];
            outFrame[1] = lastFrame[1];
            outFrame[2] = lastFrame[2];
            outFrame[3] = gifTransparentIndex;
        }
        else
        {
            // palettize the pixel
            int bestDiff = 1000000;
            int bestInd = 1;
            GifGetClosestPaletteColor(pPal, nextFrame[0], nextFrame[1], nextFrame[2], &bestInd, &bestDiff, 1);
            
            // Write the resulting color to the output buffer
            outFrame[0] = pPal->r[bestInd];
            outFrame[1] = pPal->g[bestInd];
            outFrame[2] = pPal->b[bestInd];
            outFrame[3] = bestInd;
        }
        
        if (lastFrame) lastFrame += 4;
        outFrame += 4;
        nextFrame += 4;
    }
}


// insert a single bit
static void GifWriteBit(GifBitStatus *stat, unsigned int bit)
{
    bit = bit & 1;
    bit = bit << stat->bitIndex;
    stat->byte |= bit;
    
    ++stat->bitIndex;
    if (stat->bitIndex > 7)
    {
        // move the newly-finished byte to the chunk buffer 
        stat->chunk[stat->chunkIndex++] = stat->byte;
        // and start a new byte
        stat->bitIndex = 0;
        stat->byte = 0;
    }
}

// write all bytes so far to the file
static void GifWriteChunk(FILE *f, GifBitStatus *stat)
{
    fputc(stat->chunkIndex, f);
    fwrite(stat->chunk, 1, stat->chunkIndex, f);
    
    stat->bitIndex = 0;
    stat->byte = 0;
    stat->chunkIndex = 0;
}

static void GifWriteCode(FILE *f, GifBitStatus *stat, unsigned int code, unsigned int length)
{
    for (unsigned int ii=0; ii<length; ++ii)
    {
        GifWriteBit(stat, code);
        code = code >> 1;
        
        if (stat->chunkIndex == 255)
        {
            GifWriteChunk(f, stat);
        }
    }
}

// write a 256-color (8-bit) image palette to the file
static void GifWritePalette(const GifPalette *pPal, FILE *f)
{
    fputc(0, f);  // first color: transparency
    fputc(0, f);
    fputc(0, f);
    
    for (int ii=1; ii<(1 << pPal->bitDepth); ++ii)
    {
        unsigned int r = pPal->r[ii];
        unsigned int g = pPal->g[ii];
        unsigned int b = pPal->b[ii];
        
        fputc(r, f);
        fputc(g, f);
        fputc(b, f);
    }
}

// write the image header, LZW-compress and write out the image
static void GifWriteLzwImage(FILE *f, unsigned char *image, unsigned int left, unsigned int top, unsigned int width, unsigned int height, unsigned int delay, GifPalette *pPal)
{
    // graphics control extension
    fputc(0x21, f);
    fputc(0xf9, f);
    fputc(0x04, f);
    fputc(0x05, f); // leave prev frame in place, this frame has transparency
    fputc(delay & 0xff, f);
    fputc((delay >> 8) & 0xff, f);
    fputc(gifTransparentIndex, f); // transparent color index
    fputc(0, f);
    
    fputc(0x2c, f); // image descriptor block
    
    fputc(left & 0xff, f);           // corner of image in canvas space
    fputc((left >> 8) & 0xff, f);
    fputc(top & 0xff, f);
    fputc((top >> 8) & 0xff, f);
    
    fputc(width & 0xff, f);          // width and height of image
    fputc((width >> 8) & 0xff, f);
    fputc(height & 0xff, f);
    fputc((height >> 8) & 0xff, f);
    
    //fputc(0, f); // no local color table, no transparency
    //fputc(0x80, f); // no local color table, but transparency
    
    fputc(0x80 + pPal->bitDepth-1, f); // local color table present, 2 ^ bitDepth entries
    GifWritePalette(pPal, f);
    
    const int minCodeSize = pPal->bitDepth;
    const unsigned int clearCode = 1 << pPal->bitDepth;
    
    fputc(minCodeSize, f); // min code size 8 bits
    
    GifLzwNode *codetree = (GifLzwNode *)RGIF_TEMP_MALLOC(sizeof(GifLzwNode)*4096);
    
    memset(codetree, 0, sizeof(GifLzwNode)*4096);
    int curCode = -1;
    unsigned int codeSize = minCodeSize + 1;
    unsigned int maxCode = clearCode + 1;
    
    GifBitStatus stat;
    stat.byte = 0;
    stat.bitIndex = 0;
    stat.chunkIndex = 0;
    
    GifWriteCode(f, &stat, clearCode, codeSize);  // start with a fresh LZW dictionary
    
    for (unsigned int yy=0; yy<height; ++yy)
    {
        for (unsigned int xx=0; xx<width; ++xx)
        {
            unsigned char nextValue = image[(yy*width+xx)*4+3];
            
            // "loser mode" - no compression, every single code is followed immediately by a clear
            //WriteCode(f, stat, nextValue, codeSize);
            //WriteCode(f, stat, 256, codeSize);
            
            if (curCode < 0)
            {
                // first value in a new run
                curCode = nextValue;
            }
            else if (codetree[curCode].m_next[nextValue])
            {
                // current run already in the dictionary
                curCode = codetree[curCode].m_next[nextValue];
            }
            else
            {
                // finish the current run, write a code
                GifWriteCode(f, &stat, curCode, codeSize);
                
                // insert the new run into the dictionary
                codetree[curCode].m_next[nextValue] = ++maxCode;
                
                if (maxCode >= (1ul << codeSize))
                {
                    // dictionary entry count has broken a size barrier,
                    // we need more bits for codes
                    codeSize++;
                }
                if (maxCode == 4095)
                {
                    // the dictionary is full, clear it out and begin anew
                    GifWriteCode(f, &stat, clearCode, codeSize); // clear tree
                    
                    memset(codetree, 0, sizeof(GifLzwNode)*4096);
                    codeSize = minCodeSize + 1;
                    maxCode = clearCode + 1;
                }
                
                curCode = nextValue;
            }
        }
    }
    
    // compression footer
    GifWriteCode(f, &stat, curCode, codeSize);
    GifWriteCode(f, &stat, clearCode, codeSize);
    GifWriteCode(f, &stat, clearCode + 1, minCodeSize + 1);
    
    // write out the last partial chunk
    while (stat.bitIndex) GifWriteBit(&stat, 0);
    if (stat.chunkIndex) GifWriteChunk(f, &stat);
    
    fputc(0, f); // image block terminator
    
    RGIF_TEMP_FREE(codetree);
}

#endif // RGIF_IMPLEMENTATION
