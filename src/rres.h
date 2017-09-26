/**********************************************************************************************
*
*   rres v1.0 - raylib resource (rRES) custom fileformat management functions
*
*   CONFIGURATION:
*
*   #define RREM_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   DEPENDENCIES:
*       tinfl   -  DEFLATE decompression functions
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2016-2017 Ramon Santamaria (@raysan5)
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

/*  
References:
    RIFF file-format:  http://www.johnloomis.org/cpe102/asgn/asgn1/riff.html
    ZIP file-format:   https://en.wikipedia.org/wiki/Zip_(file_format)
                       http://www.onicos.com/staff/iz/formats/zip.html
    XNB file-format:   http://xbox.create.msdn.com/en-US/sample/xnb_format
*/

#ifndef RRES_H
#define RRES_H

#if !defined(RRES_STANDALONE)
    #include "raylib.h"
#endif

//#define RRES_STATIC
#ifdef RRES_STATIC
    #define RRESDEF static              // Functions just visible to module including this file
#else
    #ifdef __cplusplus
        #define RRESDEF extern "C"      // Functions visible from other files (no name mangling of functions in C++)
    #else
        #define RRESDEF extern          // Functions visible from other files
    #endif
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_RESOURCES_SUPPORTED   256

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Some types are required for RAYGUI_STANDALONE usage
//----------------------------------------------------------------------------------
#if defined(RRES_STANDALONE)
    // rRES data returned when reading a resource, it contains all required data for user (24 byte)
    // NOTE: Using void *data pointer, so we can load to image.data, wave.data, mesh.*, (unsigned char *)
    typedef struct RRESData {
        unsigned int type;          // Resource type (4 byte)
        
        unsigned int param1;        // Resouce parameter 1 (4 byte)
        unsigned int param2;        // Resouce parameter 2 (4 byte)
        unsigned int param3;        // Resouce parameter 3 (4 byte)
        unsigned int param4;        // Resouce parameter 4 (4 byte)
        
        void *data;                 // Resource data pointer (4 byte)
    } RRESData;
    
    // RRES type (pointer to RRESData array)
    typedef struct RRESData *RRES;  // Resource pointer
    
    // RRESData type
    typedef enum { 
        RRES_TYPE_RAW = 0, 
        RRES_TYPE_IMAGE, 
        RRES_TYPE_WAVE, 
        RRES_TYPE_VERTEX, 
        RRES_TYPE_TEXT,
        RRES_TYPE_FONT_IMAGE,
        RRES_TYPE_FONT_CHARDATA,    // CharInfo { int value, recX, recY, recWidth, recHeight, offsetX, offsetY, xAdvance } 
        RRES_TYPE_DIRECTORY
    } RRESDataType;
    
// Parameters information depending on resource type

// RRES_TYPE_RAW params:        <custom>
// RRES_TYPE_IMAGE params:      width, height, mipmaps, format
// RRES_TYPE_WAVE params:       sampleCount, sampleRate, sampleSize, channels
// RRES_TYPE_VERTEX params:     vertexCount, vertexType, vertexFormat           // Use masks instead?
// RRES_TYPE_TEXT params:       charsCount, cultureCode
// RRES_TYPE_FONT_IMAGE params: width, height, format, mipmaps;
// RRES_TYPE_FONT_CHARDATA params:  charsCount, baseSize
// RRES_TYPE_DIRECTORY params:  fileCount, directoryCount

// SpriteFont = RRES_TYPE_FONT_IMAGE chunk + RRES_TYPE_FONT_DATA chunk
// Mesh = multiple RRES_TYPE_VERTEX chunks
    

#endif

//----------------------------------------------------------------------------------
// Global variables
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
//RRESDEF RRESData LoadResourceData(const char *rresFileName, int rresId, int part);
RRESDEF RRES LoadResource(const char *fileName, int rresId);
RRESDEF void UnloadResource(RRES rres);

/*
QUESTION: How to load each type of data from RRES ?

rres->type == RRES_TYPE_RAW
unsigned char data = (unsigned char *)rres[0]->data;

rres->type == RRES_TYPE_IMAGE
Image image;
image.data = rres[0]->data;        // Be careful, duplicate pointer
image.width = rres[0]->param1;
image.height = rres[0]->param2;
image.mipmaps = rres[0]->param3;
image.format =  rres[0]->format;

rres->type == RRES_TYPE_WAVE
Wave wave;
wave.data = rres[0]->data;
wave.sampleCount = rres[0]->param1;
wave.sampleRate = rres[0]->param2;
wave.sampleSize = rres[0]->param3;
wave.channels = rres[0]->param4;

rres->type == RRES_TYPE_VERTEX (multiple parts)
Mesh mesh;
mesh.vertexCount = rres[0]->param1;
mesh.vertices = (float *)rres[0]->data;
mesh.texcoords = (float *)rres[1]->data;
mesh.normals = (float *)rres[2]->data;
mesh.tangents = (float *)rres[3]->data;
mesh.tangents = (unsigned char *)rres[4]->data;

rres->type == RRES_TYPE_TEXT
unsigned char *text = (unsigned char *)rres->data;
Shader shader = LoadShaderText(text, rres->param1);     Shader LoadShaderText(const char *shdrText, int length);

rres->type == RRES_TYPE_FONT_IMAGE      (multiple parts)
rres->type == RRES_TYPE_FONT_CHARDATA   
SpriteFont font;
font.texture = LoadTextureFromImage(image);     // rres[0]
font.chars = (CharInfo *)rres[1]->data;
font.charsCount = rres[1]->param1;
font.baseSize = rres[1]->param2;

rres->type == RRES_TYPE_DIRECTORY
unsigned char *fileNames = (unsigned char *)rres[0]->data;  // fileNames separed by \n
int filesCount = rres[0]->param1;
*/

#endif // RRES_H


/***********************************************************************************
*
*   RRES IMPLEMENTATION
*
************************************************************************************/

#if defined(RRES_IMPLEMENTATION)

#include <stdio.h>          // Required for: FILE, fopen(), fclose()

// Check if custom malloc/free functions defined, if not, using standard ones
#if !defined(RRES_MALLOC)
    #include <stdlib.h>     // Required for: malloc(), free()

    #define RRES_MALLOC(size)  malloc(size)
    #define RRES_FREE(ptr)     free(ptr)
#endif

#include "external/tinfl.c" // Required for: tinfl_decompress_mem_to_mem()
                            // NOTE: DEFLATE algorythm data decompression

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// rRES file header (8 byte)
typedef struct {
    char id[4];                 // File identifier: rRES (4 byte)
    unsigned short version;     // File version and subversion (2 byte)
    unsigned short count;       // Number of resources in this file (2 byte)
} RRESFileHeader;

// rRES info header, every resource includes this header (16 byte + 16 byte)
typedef struct {
    unsigned int id;            // Resource unique identifier (4 byte)
    unsigned char dataType;     // Resource data type (1 byte)
    unsigned char compType;     // Resource data compression type (1 byte)
    unsigned char cryptoType;   // Resource data encryption type (1 byte)
    unsigned char partsCount;   // Resource data parts count, used for splitted data (1 byte)
    unsigned int dataSize;      // Resource data size (compressed or not, only DATA) (4 byte)
    unsigned int uncompSize;    // Resource data size (uncompressed, only DATA) (4 byte)

    unsigned int param1;        // Resouce parameter 1 (4 byte)
    unsigned int param2;        // Resouce parameter 2 (4 byte)
    unsigned int param3;        // Resouce parameter 3 (4 byte)
    unsigned int param4;        // Resouce parameter 4 (4 byte)
} RRESInfoHeader;

// Compression types
typedef enum { 
    RRES_COMP_NONE = 0,         // No data compression
    RRES_COMP_DEFLATE,          // DEFLATE compression
    RRES_COMP_LZ4,              // LZ4 compression
    RRES_COMP_LZMA,             // LZMA compression
    RRES_COMP_BROTLI,           // BROTLI compression
    // gzip, zopfli, lzo, zstd  // Other compression algorythms...
} RRESCompressionType;

// Encryption types
typedef enum {
    RRES_CRYPTO_NONE = 0,       // No data encryption
    RRES_CRYPTO_XOR,            // XOR (128 bit) encryption
    RRES_CRYPTO_AES,            // RIJNDAEL (128 bit) encryption (AES)
    RRES_CRYPTO_TDES,           // Triple DES encryption
    RRES_CRYPTO_BLOWFISH,       // BLOWFISH encryption
    RRES_CRYPTO_XTEA,           // XTEA encryption
    // twofish, RC5, RC6        // Other encryption algorythm...
} RRESEncryptionType;

// Image/Texture data type
typedef enum {
    RRES_IM_UNCOMP_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
    RRES_IM_UNCOMP_GRAY_ALPHA,        // 16 bpp (2 channels)
    RRES_IM_UNCOMP_R5G6B5,            // 16 bpp
    RRES_IM_UNCOMP_R8G8B8,            // 24 bpp
    RRES_IM_UNCOMP_R5G5B5A1,          // 16 bpp (1 bit alpha)
    RRES_IM_UNCOMP_R4G4B4A4,          // 16 bpp (4 bit alpha)
    RRES_IM_UNCOMP_R8G8B8A8,          // 32 bpp
    RRES_IM_COMP_DXT1_RGB,            // 4 bpp (no alpha)
    RRES_IM_COMP_DXT1_RGBA,           // 4 bpp (1 bit alpha)
    RRES_IM_COMP_DXT3_RGBA,           // 8 bpp
    RRES_IM_COMP_DXT5_RGBA,           // 8 bpp
    RRES_IM_COMP_ETC1_RGB,            // 4 bpp
    RRES_IM_COMP_ETC2_RGB,            // 4 bpp
    RRES_IM_COMP_ETC2_EAC_RGBA,       // 8 bpp
    RRES_IM_COMP_PVRT_RGB,            // 4 bpp
    RRES_IM_COMP_PVRT_RGBA,           // 4 bpp
    RRES_IM_COMP_ASTC_4x4_RGBA,       // 8 bpp
    RRES_IM_COMP_ASTC_8x8_RGBA        // 2 bpp
    //...
} RRESImageFormat;

// Vertex data type
typedef enum {
    RRES_VERT_POSITION,
    RRES_VERT_TEXCOORD1,
    RRES_VERT_TEXCOORD2,
    RRES_VERT_TEXCOORD3,
    RRES_VERT_TEXCOORD4,
    RRES_VERT_NORMAL,
    RRES_VERT_TANGENT,
    RRES_VERT_COLOR,
    RRES_VERT_INDEX,
    //...
} RRESVertexType;

// Vertex data format type
typedef enum {
    RRES_VERT_BYTE,
    RRES_VERT_SHORT,
    RRES_VERT_INT,
    RRES_VERT_HFLOAT,
    RRES_VERT_FLOAT,
    //...
} RRESVertexFormat;

#if defined(RRES_STANDALONE)
typedef enum { LOG_INFO = 0, LOG_ERROR, LOG_WARNING, LOG_DEBUG, LOG_OTHER } TraceLogType;
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void *DecompressData(const unsigned char *data, unsigned long compSize, int uncompSize);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Load resource from file by id (could be multiple parts)
// NOTE: Returns uncompressed data with parameters, search resource by id
RRESDEF RRES LoadResource(const char *fileName, int rresId)
{
    RRES rres = { 0 };

    RRESFileHeader fileHeader;
    RRESInfoHeader infoHeader;
    
    FILE *rresFile = fopen(fileName, "rb");

    if (rresFile == NULL) TraceLog(LOG_WARNING, "[%s] rRES raylib resource file could not be opened", fileName);
    else
    {
        // Read rres file info header
        fread(&fileHeader.id[0], sizeof(char), 1, rresFile);
        fread(&fileHeader.id[1], sizeof(char), 1, rresFile);
        fread(&fileHeader.id[2], sizeof(char), 1, rresFile);
        fread(&fileHeader.id[3], sizeof(char), 1, rresFile);
        fread(&fileHeader.version, sizeof(short), 1, rresFile);
        fread(&fileHeader.count, sizeof(short), 1, rresFile);

        // Verify "rRES" identifier
        if ((fileHeader.id[0] != 'r') && (fileHeader.id[1] != 'R') && (fileHeader.id[2] != 'E') && (fileHeader.id[3] != 'S'))
        {
            TraceLog(LOG_WARNING, "[%s] This is not a valid raylib resource file", fileName);
        }
        else
        {
            for (int i = 0; i < fileHeader.count; i++)
            {
                // Read resource info and parameters
                fread(&infoHeader, sizeof(RRESInfoHeader), 1, rresFile);
                
                if (infoHeader.id == rresId)
                {
                    rres = (RRES)malloc(sizeof(RRESData)*infoHeader.partsCount);
                                    
                    // Load all required resources parts
                    for (int k = 0; k < infoHeader.partsCount; k++)
                    {
                        // TODO: Verify again that rresId is the same in every part
                        
                        // Register data type and parameters
                        rres[k].type = infoHeader.dataType;
                        rres[k].param1 = infoHeader.param1;
                        rres[k].param2 = infoHeader.param2;
                        rres[k].param3 = infoHeader.param3;
                        rres[k].param4 = infoHeader.param4;

                        // Read resource data block
                        void *data = RRES_MALLOC(infoHeader.dataSize);
                        fread(data, infoHeader.dataSize, 1, rresFile);

                        if (infoHeader.compType == RRES_COMP_DEFLATE)
                        {
                            void *uncompData = DecompressData(data, infoHeader.dataSize, infoHeader.uncompSize);
                            
                            rres[k].data = uncompData;
                            
                            RRES_FREE(data);
                        }
                        else rres[k].data = data;

                        if (rres[k].data != NULL) TraceLog(LOG_INFO, "[%s][ID %i] Resource data loaded successfully", fileName, (int)infoHeader.id);
                        
                        // Read next part
                        fread(&infoHeader, sizeof(RRESInfoHeader), 1, rresFile); 
                    }
                }
                else
                {
                    // Skip required data to read next resource infoHeader
                    fseek(rresFile, infoHeader.dataSize, SEEK_CUR);
                }
            }
            
            if (rres[0].data == NULL) TraceLog(LOG_WARNING, "[%s][ID %i] Requested resource could not be found", fileName, (int)rresId);
        }

        fclose(rresFile);
    }

    return rres;
}

// Unload resource data
RRESDEF void UnloadResource(RRES rres)
{
    // TODO: When you load resource... how many parts conform it? depends on type? --> Not clear...
    
    if (rres[0].data != NULL) free(rres[0].data);
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Data decompression function
// NOTE: Allocated data MUST be freed by user
static void *DecompressData(const unsigned char *data, unsigned long compSize, int uncompSize)
{
    int tempUncompSize;
    void *uncompData;

    // Allocate buffer to hold decompressed data
    uncompData = (mz_uint8 *)RRES_MALLOC((size_t)uncompSize);

    // Check correct memory allocation
    if (uncompData == NULL)
    {
        TraceLog(LOG_WARNING, "Out of memory while decompressing data");
    }
    else
    {
        // Decompress data
        tempUncompSize = tinfl_decompress_mem_to_mem(uncompData, (size_t)uncompSize, data, compSize, 1);

        if (tempUncompSize == -1)
        {
            TraceLog(LOG_WARNING, "Data decompression failed");
            RRES_FREE(uncompData);
        }

        if (uncompSize != (int)tempUncompSize)
        {
            TraceLog(LOG_WARNING, "Expected uncompressed size do not match, data may be corrupted");
            TraceLog(LOG_WARNING, " -- Expected uncompressed size: %i", uncompSize);
            TraceLog(LOG_WARNING, " -- Returned uncompressed size: %i", tempUncompSize);
        }

        TraceLog(LOG_INFO, "Data decompressed successfully from %u bytes to %u bytes", (mz_uint32)compSize, (mz_uint32)tempUncompSize);
    }

    return uncompData;
}

// Some required functions for rres standalone module version
#if defined(RRES_STANDALONE)
// Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
void TraceLog(int logType, const char *text, ...)
{
    va_list args;
    va_start(args, text);

    switch (msgType)
    {
        case LOG_INFO: fprintf(stdout, "INFO: "); break;
        case LOG_ERROR: fprintf(stdout, "ERROR: "); break;
        case LOG_WARNING: fprintf(stdout, "WARNING: "); break;
        case LOG_DEBUG: fprintf(stdout, "DEBUG: "); break;
        default: break;
    }

    vfprintf(stdout, text, args);
    fprintf(stdout, "\n");

    va_end(args);

    if (msgType == LOG_ERROR) exit(1);
}
#endif

#endif // RRES_IMPLEMENTATION
