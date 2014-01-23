/*********************************************************************************************
*
*   raylib.utils
*
*   Utils Functions Definitions
*    
*   Uses external lib:    
*       tinfl - zlib DEFLATE algorithm decompression lib
*       
*   Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*    
*   This software is provided "as-is", without any express or implied warranty. In no event 
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial 
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you 
*     wrote the original software. If you use this software in a product, an acknowledgment 
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "utils.h"

#include <stdlib.h>         // malloc(), free()
#include <stdio.h>          // printf()
//#include <string.h>       // String management functions: strlen(), strrchr(), strcmp()

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"    // Create PNG file
#include "tinfl.c"

// Data decompression function
// NOTE: Allocated data MUST be freed!
unsigned char *DecompressData(const unsigned char *data, unsigned long compSize, int uncompSize)
{
    int tempUncompSize;
    unsigned char *pUncomp;
    
    // Allocate buffer to hold decompressed data
    pUncomp = (mz_uint8 *)malloc((size_t)uncompSize);
    
    // Check correct memory allocation
    if (!pUncomp)
    {
        printf("Out of memory!\n");
        return NULL;
    }
    
    // Decompress data
    tempUncompSize = tinfl_decompress_mem_to_mem(pUncomp, (size_t)uncompSize, data, compSize, 1);
    
    if (tempUncompSize == -1)
    {
        printf("Decompression failed!\n");
        free(pUncomp);
        return NULL;
    }
    
    if (uncompSize != (int)tempUncompSize)
    {
        printf("WARNING! Expected uncompressed size do not match! Data may be corrupted!\n");
        printf(" -- Expected uncompressed size: %i\n", uncompSize);
        printf(" -- Returned uncompressed size: %i\n", tempUncompSize);
    }

    printf("Decompressed from %u bytes to %u bytes\n", (mz_uint32)compSize, (mz_uint32)tempUncompSize);
    
    return pUncomp;
}

// Creates a bitmap (BMP) file from an array of pixel data
// NOTE: This function is not explicitly available to raylib users
void WriteBitmap(const char *fileName, unsigned char *imgData, int width, int height)
{
    int filesize = 54 + 3*width*height;
    
    unsigned char bmpFileHeader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};    // Standard BMP file header
    unsigned char bmpInfoHeader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};   // Standard BMP info header

    bmpFileHeader[2] = (unsigned char)(filesize);
    bmpFileHeader[3] = (unsigned char)(filesize>>8);
    bmpFileHeader[4] = (unsigned char)(filesize>>16);
    bmpFileHeader[5] = (unsigned char)(filesize>>24);

    bmpInfoHeader[4] = (unsigned char)(width);
    bmpInfoHeader[5] = (unsigned char)(width>>8);
    bmpInfoHeader[6] = (unsigned char)(width>>16);
    bmpInfoHeader[7] = (unsigned char)(width>>24);
    bmpInfoHeader[8] = (unsigned char)(height);
    bmpInfoHeader[9] = (unsigned char)(height>>8);
    bmpInfoHeader[10] = (unsigned char)(height>>16);
    bmpInfoHeader[11] = (unsigned char)(height>>24);

    FILE *bmpFile = fopen(fileName, "wb");    // Define a pointer to bitmap file and open it in write-binary mode
    
    // NOTE: fwrite parameters are: data pointer, size in bytes of each element to be written, number of elements, file-to-write pointer
    fwrite(bmpFileHeader, sizeof(unsigned char), 14, bmpFile);    // Write BMP file header data
    fwrite(bmpInfoHeader, sizeof(unsigned char), 40, bmpFile);    // Write BMP info header data
    
    // Write pixel data to file
    for (int y = 0; y < height ; y++)
    {
        for (int x = 0; x < width; x++)
        {
            fputc(imgData[(x*4)+2 + (y*width*4)], bmpFile);
            fputc(imgData[(x*4)+1 + (y*width*4)], bmpFile);
            fputc(imgData[(x*4) + (y*width*4)], bmpFile);
        }
    }

    fclose(bmpFile);        // Close bitmap file
}

// Creates a PNG image file from an array of pixel data
// NOTE: Uses stb_image_write
void WritePNG(const char *fileName, unsigned char *imgData, int width, int height)
{
    stbi_write_png(fileName, width, height, 4, imgData, width*4); // It WORKS!!!
}