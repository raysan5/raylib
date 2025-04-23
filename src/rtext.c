/**********************************************************************************************
*
*   rtext - Basic functions to load fonts and draw text
*
*   CONFIGURATION:
*       #define SUPPORT_MODULE_RTEXT
*           rtext module is included in the build
*
*       #define SUPPORT_DEFAULT_FONT
*           Load default raylib font on initialization to be used by DrawText() and MeasureText().
*           If no default font loaded, DrawTextEx() and MeasureTextEx() are required.
*
*       #define SUPPORT_FILEFORMAT_FNT
*       #define SUPPORT_FILEFORMAT_TTF
*       #define SUPPORT_FILEFORMAT_BDF
*           Selected desired fileformats to be supported for loading. Some of those formats are
*           supported by default, to remove support, just comment unrequired #define in this module
*
*       #define SUPPORT_FONT_ATLAS_WHITE_REC
*           On font atlas image generation [GenImageFontAtlas()], add a 3x3 pixels white rectangle
*           at the bottom-right corner of the atlas. It can be useful to for shapes drawing, to allow
*           drawing text and shapes with a single draw call [SetShapesTexture()].
*
*       #define TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH
*           TextSplit() function static buffer max size
*
*       #define MAX_TEXTSPLIT_COUNT
*           TextSplit() function static substrings pointers array (pointing to static buffer)
*
*   DEPENDENCIES:
*       stb_truetype  - Load TTF file and rasterize characters data
*       stb_rect_pack - Rectangles packing algorithms, required for font atlas generation
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2025 Ramon Santamaria (@raysan5)
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

#include "raylib.h"         // Declares module functions

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
    #include "config.h"     // Defines module configuration flags
#endif

#if defined(SUPPORT_MODULE_RTEXT)

#include "utils.h"          // Required for: LoadFile*()
#include "rlgl.h"           // OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2 -> Only DrawTextPro()

#include <stdlib.h>         // Required for: malloc(), free()
#include <stdio.h>          // Required for: vsprintf()
#include <string.h>         // Required for: strcmp(), strstr(), strcpy(), strncpy() [Used in TextReplace()], sscanf() [Used in LoadBMFont()]
#include <stdarg.h>         // Required for: va_list, va_start(), vsprintf(), va_end() [Used in TextFormat()]
#include <ctype.h>          // Required for: toupper(), tolower() [Used in TextToUpper(), TextToLower()]

#if defined(SUPPORT_FILEFORMAT_TTF) || defined(SUPPORT_FILEFORMAT_BDF)
    #if defined(__GNUC__) // GCC and Clang
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-function"
    #endif

    #define STB_RECT_PACK_IMPLEMENTATION
    #include "external/stb_rect_pack.h"     // Required for: ttf/bdf font rectangles packaging

    #include <math.h>   // Required for: ttf/bdf font rectangles packaging

    #if defined(__GNUC__) // GCC and Clang
        #pragma GCC diagnostic pop
    #endif
#endif

#if defined(SUPPORT_FILEFORMAT_TTF)
    #if defined(__GNUC__) // GCC and Clang
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-function"
    #endif

    #define STBTT_STATIC
    #define STB_TRUETYPE_IMPLEMENTATION
    #include "external/stb_truetype.h"      // Required for: ttf font data reading

    #if defined(__GNUC__) // GCC and Clang
        #pragma GCC diagnostic pop
    #endif
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef MAX_TEXT_BUFFER_LENGTH
    #define MAX_TEXT_BUFFER_LENGTH              1024        // Size of internal static buffers used on some functions:
                                                            // TextFormat(), TextSubtext(), TextToUpper(), TextToLower(), TextToPascal(), TextSplit()
#endif
#ifndef MAX_TEXT_UNICODE_CHARS
    #define MAX_TEXT_UNICODE_CHARS               512        // Maximum number of unicode codepoints: GetCodepoints()
#endif
#ifndef MAX_TEXTSPLIT_COUNT
    #define MAX_TEXTSPLIT_COUNT                  128        // Maximum number of substrings to split: TextSplit()
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Global variables
//----------------------------------------------------------------------------------
extern bool isGpuReady;
#if defined(SUPPORT_DEFAULT_FONT)
// Default font provided by raylib
// NOTE: Default font is loaded on InitWindow() and disposed on CloseWindow() [module: core]
static Font defaultFont = { 0 };
#endif

//----------------------------------------------------------------------------------
// Other Modules Functions Declaration (required by text)
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(SUPPORT_FILEFORMAT_FNT)
static Font LoadBMFont(const char *fileName);   // Load a BMFont file (AngelCode font file)
#endif
#if defined(SUPPORT_FILEFORMAT_BDF)
static GlyphInfo *LoadFontDataBDF(const unsigned char *fileData, int dataSize, int *codepoints, int codepointCount, int *outFontSize);
#endif
static int textLineSpacing = 2;                 // Text vertical line spacing in pixels (between lines)

#if defined(SUPPORT_DEFAULT_FONT)
extern void LoadFontDefault(void);
extern void UnloadFontDefault(void);
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
#if defined(SUPPORT_DEFAULT_FONT)
// Load raylib default font
extern void LoadFontDefault(void)
{
    #define BIT_CHECK(a,b) ((a) & (1u << (b)))

    // NOTE: Using UTF-8 encoding table for Unicode U+0000..U+00FF Basic Latin + Latin-1 Supplement
    // Ref: http://www.utf8-chartable.de/unicode-utf8-table.pl

    defaultFont.glyphCount = 224;   // Number of chars included in our default font
    defaultFont.glyphPadding = 0;   // Characters padding

    // Default font is directly defined here (data generated from a sprite font image)
    // This way, we reconstruct Font without creating large global variables
    // This data is automatically allocated to Stack and automatically deallocated at the end of this function
    unsigned int defaultFontData[512] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200020, 0x0001b000, 0x00000000, 0x00000000, 0x8ef92520, 0x00020a00, 0x7dbe8000, 0x1f7df45f,
        0x4a2bf2a0, 0x0852091e, 0x41224000, 0x10041450, 0x2e292020, 0x08220812, 0x41222000, 0x10041450, 0x10f92020, 0x3efa084c, 0x7d22103c, 0x107df7de,
        0xe8a12020, 0x08220832, 0x05220800, 0x10450410, 0xa4a3f000, 0x08520832, 0x05220400, 0x10450410, 0xe2f92020, 0x0002085e, 0x7d3e0281, 0x107df41f,
        0x00200000, 0x8001b000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc0000fbe, 0xfbf7e00f, 0x5fbf7e7d, 0x0050bee8, 0x440808a2, 0x0a142fe8, 0x50810285, 0x0050a048,
        0x49e428a2, 0x0a142828, 0x40810284, 0x0048a048, 0x10020fbe, 0x09f7ebaf, 0xd89f3e84, 0x0047a04f, 0x09e48822, 0x0a142aa1, 0x50810284, 0x0048a048,
        0x04082822, 0x0a142fa0, 0x50810285, 0x0050a248, 0x00008fbe, 0xfbf42021, 0x5f817e7d, 0x07d09ce8, 0x00008000, 0x00000fe0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000c0180,
        0xdfbf4282, 0x0bfbf7ef, 0x42850505, 0x004804bf, 0x50a142c6, 0x08401428, 0x42852505, 0x00a808a0, 0x50a146aa, 0x08401428, 0x42852505, 0x00081090,
        0x5fa14a92, 0x0843f7e8, 0x7e792505, 0x00082088, 0x40a15282, 0x08420128, 0x40852489, 0x00084084, 0x40a16282, 0x0842022a, 0x40852451, 0x00088082,
        0xc0bf4282, 0xf843f42f, 0x7e85fc21, 0x3e0900bf, 0x00000000, 0x00000004, 0x00000000, 0x000c0180, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000402, 0x41482000, 0x00000000, 0x00000800,
        0x04000404, 0x4100203c, 0x00000000, 0x00000800, 0xf7df7df0, 0x514bef85, 0xbefbefbe, 0x04513bef, 0x14414500, 0x494a2885, 0xa28a28aa, 0x04510820,
        0xf44145f0, 0x474a289d, 0xa28a28aa, 0x04510be0, 0x14414510, 0x494a2884, 0xa28a28aa, 0x02910a00, 0xf7df7df0, 0xd14a2f85, 0xbefbe8aa, 0x011f7be0,
        0x00000000, 0x00400804, 0x20080000, 0x00000000, 0x00000000, 0x00600f84, 0x20080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0xac000000, 0x00000f01, 0x00000000, 0x00000000, 0x24000000, 0x00000f01, 0x00000000, 0x06000000, 0x24000000, 0x00000f01, 0x00000000, 0x09108000,
        0x24fa28a2, 0x00000f01, 0x00000000, 0x013e0000, 0x2242252a, 0x00000f52, 0x00000000, 0x038a8000, 0x2422222a, 0x00000f29, 0x00000000, 0x010a8000,
        0x2412252a, 0x00000f01, 0x00000000, 0x010a8000, 0x24fbe8be, 0x00000f01, 0x00000000, 0x0ebe8000, 0xac020000, 0x00000f01, 0x00000000, 0x00048000,
        0x0003e000, 0x00000f00, 0x00000000, 0x00008000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000038, 0x8443b80e, 0x00203a03,
        0x02bea080, 0xf0000020, 0xc452208a, 0x04202b02, 0xf8029122, 0x07f0003b, 0xe44b388e, 0x02203a02, 0x081e8a1c, 0x0411e92a, 0xf4420be0, 0x01248202,
        0xe8140414, 0x05d104ba, 0xe7c3b880, 0x00893a0a, 0x283c0e1c, 0x04500902, 0xc4400080, 0x00448002, 0xe8208422, 0x04500002, 0x80400000, 0x05200002,
        0x083e8e00, 0x04100002, 0x804003e0, 0x07000042, 0xf8008400, 0x07f00003, 0x80400000, 0x04000022, 0x00000000, 0x00000000, 0x80400000, 0x04000002,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00800702, 0x1848a0c2, 0x84010000, 0x02920921, 0x01042642, 0x00005121, 0x42023f7f, 0x00291002,
        0xefc01422, 0x7efdfbf7, 0xefdfa109, 0x03bbbbf7, 0x28440f12, 0x42850a14, 0x20408109, 0x01111010, 0x28440408, 0x42850a14, 0x2040817f, 0x01111010,
        0xefc78204, 0x7efdfbf7, 0xe7cf8109, 0x011111f3, 0x2850a932, 0x42850a14, 0x2040a109, 0x01111010, 0x2850b840, 0x42850a14, 0xefdfbf79, 0x03bbbbf7,
        0x001fa020, 0x00000000, 0x00001000, 0x00000000, 0x00002070, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x08022800, 0x00012283, 0x02430802, 0x01010001, 0x8404147c, 0x20000144, 0x80048404, 0x00823f08, 0xdfbf4284, 0x7e03f7ef, 0x142850a1, 0x0000210a,
        0x50a14684, 0x528a1428, 0x142850a1, 0x03efa17a, 0x50a14a9e, 0x52521428, 0x142850a1, 0x02081f4a, 0x50a15284, 0x4a221428, 0xf42850a1, 0x03efa14b,
        0x50a16284, 0x4a521428, 0x042850a1, 0x0228a17a, 0xdfbf427c, 0x7e8bf7ef, 0xf7efdfbf, 0x03efbd0b, 0x00000000, 0x04000000, 0x00000000, 0x00000008,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200508, 0x00840400, 0x11458122, 0x00014210,
        0x00514294, 0x51420800, 0x20a22a94, 0x0050a508, 0x00200000, 0x00000000, 0x00050000, 0x08000000, 0xfefbefbe, 0xfbefbefb, 0xfbeb9114, 0x00fbefbe,
        0x20820820, 0x8a28a20a, 0x8a289114, 0x3e8a28a2, 0xfefbefbe, 0xfbefbe0b, 0x8a289114, 0x008a28a2, 0x228a28a2, 0x08208208, 0x8a289114, 0x088a28a2,
        0xfefbefbe, 0xfbefbefb, 0xfa2f9114, 0x00fbefbe, 0x00000000, 0x00000040, 0x00000000, 0x00000000, 0x00000000, 0x00000020, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00210100, 0x00000004, 0x00000000, 0x00000000, 0x14508200, 0x00001402, 0x00000000, 0x00000000,
        0x00000010, 0x00000020, 0x00000000, 0x00000000, 0xa28a28be, 0x00002228, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000,
        0xa28a28aa, 0x000022a8, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000, 0xbefbefbe, 0x00003e2f, 0x00000000, 0x00000000,
        0x00000004, 0x00002028, 0x00000000, 0x00000000, 0x80000000, 0x00003e0f, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

    int charsHeight = 10;
    int charsDivisor = 1;    // Every char is separated from the consecutive by a 1 pixel divisor, horizontally and vertically

    int charsWidth[224] = { 3, 1, 4, 6, 5, 7, 6, 2, 3, 3, 5, 5, 2, 4, 1, 7, 5, 2, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 3, 4, 3, 6,
                            7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 5, 6, 5, 7, 6, 6, 6, 6, 6, 6, 7, 6, 7, 7, 6, 6, 6, 2, 7, 2, 3, 5,
                            2, 5, 5, 5, 5, 5, 4, 5, 5, 1, 2, 5, 2, 5, 5, 5, 5, 5, 5, 5, 4, 5, 5, 5, 5, 5, 5, 3, 1, 3, 4, 4,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 5, 5, 5, 7, 1, 5, 3, 7, 3, 5, 4, 1, 7, 4, 3, 5, 3, 3, 2, 5, 6, 1, 2, 2, 3, 5, 6, 6, 6, 6,
                            6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 6, 3, 3, 3, 3, 7, 6, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 4, 6,
                            5, 5, 5, 5, 5, 5, 9, 5, 5, 5, 5, 5, 2, 2, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 5 };

    // Re-construct image from defaultFontData and generate OpenGL texture
    //----------------------------------------------------------------------
    Image imFont = {
        .data = RL_CALLOC(128*128, 2),  // 2 bytes per pixel (gray + alpha)
        .width = 128,
        .height = 128,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA
    };

    // Fill image.data with defaultFontData (convert from bit to pixel!)
    for (int i = 0, counter = 0; i < imFont.width*imFont.height; i += 32)
    {
        for (int j = 31; j >= 0; j--)
        {
            if (BIT_CHECK(defaultFontData[counter], j))
            {
                // NOTE: We are unreferencing data as short, so,
                // we must consider data as little-endian order (alpha + gray)
                ((unsigned short *)imFont.data)[i + j] = 0xffff;
            }
            else
            {
                ((unsigned char *)imFont.data)[(i + j)*sizeof(short)] = 0xFF;
                ((unsigned char *)imFont.data)[(i + j)*sizeof(short) + 1] = 0x00;
            }
        }

        counter++;
    }

    if (isGpuReady) defaultFont.texture = LoadTextureFromImage(imFont);

    // Reconstruct charSet using charsWidth[], charsHeight, charsDivisor, glyphCount
    //------------------------------------------------------------------------------

    // Allocate space for our characters info data
    // NOTE: This memory must be freed at end! --> Done by CloseWindow()
    defaultFont.glyphs = (GlyphInfo *)RL_CALLOC(defaultFont.glyphCount, sizeof(GlyphInfo));
    defaultFont.recs = (Rectangle *)RL_CALLOC(defaultFont.glyphCount, sizeof(Rectangle));

    int currentLine = 0;
    int currentPosX = charsDivisor;
    int testPosX = charsDivisor;

    for (int i = 0; i < defaultFont.glyphCount; i++)
    {
        defaultFont.glyphs[i].value = 32 + i;  // First char is 32

        defaultFont.recs[i].x = (float)currentPosX;
        defaultFont.recs[i].y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        defaultFont.recs[i].width = (float)charsWidth[i];
        defaultFont.recs[i].height = (float)charsHeight;

        testPosX += (int)(defaultFont.recs[i].width + (float)charsDivisor);

        if (testPosX >= defaultFont.texture.width)
        {
            currentLine++;
            currentPosX = 2*charsDivisor + charsWidth[i];
            testPosX = currentPosX;

            defaultFont.recs[i].x = (float)charsDivisor;
            defaultFont.recs[i].y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        }
        else currentPosX = testPosX;

        // NOTE: On default font character offsets and xAdvance are not required
        defaultFont.glyphs[i].offsetX = 0;
        defaultFont.glyphs[i].offsetY = 0;
        defaultFont.glyphs[i].advanceX = 0;

        // Fill character image data from fontClear data
        defaultFont.glyphs[i].image = ImageFromImage(imFont, defaultFont.recs[i]);
    }

    UnloadImage(imFont);

    defaultFont.baseSize = (int)defaultFont.recs[0].height;

    TRACELOG(LOG_INFO, "FONT: Default font loaded successfully (%i glyphs)", defaultFont.glyphCount);
}

// Unload raylib default font
extern void UnloadFontDefault(void)
{
    for (int i = 0; i < defaultFont.glyphCount; i++) UnloadImage(defaultFont.glyphs[i].image);
    if (isGpuReady) UnloadTexture(defaultFont.texture);
    RL_FREE(defaultFont.glyphs);
    RL_FREE(defaultFont.recs);
}
#endif      // SUPPORT_DEFAULT_FONT

// Get the default font, useful to be used with extended parameters
Font GetFontDefault()
{
#if defined(SUPPORT_DEFAULT_FONT)
    return defaultFont;
#else
    Font font = { 0 };
    return font;
#endif
}

// Load Font from file into GPU memory (VRAM)
Font LoadFont(const char *fileName)
{
    // Default values for ttf font generation
#ifndef FONT_TTF_DEFAULT_SIZE
    #define FONT_TTF_DEFAULT_SIZE           32      // TTF font generation default char size (char-height)
#endif
#ifndef FONT_TTF_DEFAULT_NUMCHARS
    #define FONT_TTF_DEFAULT_NUMCHARS       95      // TTF font generation default charset: 95 glyphs (ASCII 32..126)
#endif
#ifndef FONT_TTF_DEFAULT_FIRST_CHAR
    #define FONT_TTF_DEFAULT_FIRST_CHAR     32      // TTF font generation default first char for image sprite font (32-Space)
#endif
#ifndef FONT_TTF_DEFAULT_CHARS_PADDING
    #define FONT_TTF_DEFAULT_CHARS_PADDING   4      // TTF font generation default chars padding
#endif

    Font font = { 0 };

#if defined(SUPPORT_FILEFORMAT_TTF)
    if (IsFileExtension(fileName, ".ttf") || IsFileExtension(fileName, ".otf")) font = LoadFontEx(fileName, FONT_TTF_DEFAULT_SIZE, NULL, FONT_TTF_DEFAULT_NUMCHARS);
    else
#endif
#if defined(SUPPORT_FILEFORMAT_FNT)
    if (IsFileExtension(fileName, ".fnt")) font = LoadBMFont(fileName);
    else
#endif
#if defined(SUPPORT_FILEFORMAT_BDF)
    if (IsFileExtension(fileName, ".bdf")) font = LoadFontEx(fileName, FONT_TTF_DEFAULT_SIZE, NULL, FONT_TTF_DEFAULT_NUMCHARS);
    else
#endif
    {
        Image image = LoadImage(fileName);
        if (image.data != NULL) font = LoadFontFromImage(image, MAGENTA, FONT_TTF_DEFAULT_FIRST_CHAR);
        UnloadImage(image);
    }

    if (isGpuReady)
    {
        if (font.texture.id == 0) TRACELOG(LOG_WARNING, "FONT: [%s] Failed to load font texture -> Using default font", fileName);
        else
        {
            SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);    // By default, we set point filter (the best performance)
            TRACELOG(LOG_INFO, "FONT: Data loaded successfully (%i pixel size | %i glyphs)", FONT_TTF_DEFAULT_SIZE, FONT_TTF_DEFAULT_NUMCHARS);
        }
    }

    return font;
}

// Load Font from TTF or BDF font file with generation parameters
// NOTE: You can pass an array with desired characters, those characters should be available in the font
// if array is NULL, default char set is selected 32..126
Font LoadFontEx(const char *fileName, int fontSize, int *codepoints, int codepointCount)
{
    Font font = { 0 };

    // Loading file to memory
    int dataSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &dataSize);

    if (fileData != NULL)
    {
        // Loading font from memory data
        font = LoadFontFromMemory(GetFileExtension(fileName), fileData, dataSize, fontSize, codepoints, codepointCount);

        UnloadFileData(fileData);
    }

    return font;
}

// Load an Image font file (XNA style)
Font LoadFontFromImage(Image image, Color key, int firstChar)
{
#ifndef MAX_GLYPHS_FROM_IMAGE
    #define MAX_GLYPHS_FROM_IMAGE   256     // Maximum number of glyphs supported on image scan
#endif

    #define COLOR_EQUAL(col1, col2) ((col1.r == col2.r) && (col1.g == col2.g) && (col1.b == col2.b) && (col1.a == col2.a))

    Font font = GetFontDefault();

    int charSpacing = 0;
    int lineSpacing = 0;

    int x = 0;
    int y = 0;

    // We allocate a temporal arrays for chars data measures,
    // once we get the actual number of chars, we copy data to a sized arrays
    int tempCharValues[MAX_GLYPHS_FROM_IMAGE] = { 0 };
    Rectangle tempCharRecs[MAX_GLYPHS_FROM_IMAGE] = { 0 };

    Color *pixels = LoadImageColors(image);

    // Parse image data to get charSpacing and lineSpacing
    for (y = 0; y < image.height; y++)
    {
        for (x = 0; x < image.width; x++)
        {
            if (!COLOR_EQUAL(pixels[y*image.width + x], key)) break;
        }

        if (!COLOR_EQUAL(pixels[y*image.width + x], key)) break;
    }

    if ((x == 0) || (y == 0)) return font; // Security check

    charSpacing = x;
    lineSpacing = y;

    int charHeight = 0;
    int j = 0;

    while (!COLOR_EQUAL(pixels[(lineSpacing + j)*image.width + charSpacing], key)) j++;

    charHeight = j;

    // Check array values to get characters: value, x, y, w, h
    int index = 0;
    int lineToRead = 0;
    int xPosToRead = charSpacing;

    // Parse image data to get rectangle sizes
    while ((lineSpacing + lineToRead*(charHeight + lineSpacing)) < image.height)
    {
        while ((xPosToRead < image.width) &&
              !COLOR_EQUAL((pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead]), key))
        {
            tempCharValues[index] = firstChar + index;

            tempCharRecs[index].x = (float)xPosToRead;
            tempCharRecs[index].y = (float)(lineSpacing + lineToRead*(charHeight + lineSpacing));
            tempCharRecs[index].height = (float)charHeight;

            int charWidth = 0;

            while (!COLOR_EQUAL(pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead + charWidth], key)) charWidth++;

            tempCharRecs[index].width = (float)charWidth;

            index++;

            xPosToRead += (charWidth + charSpacing);
        }

        lineToRead++;
        xPosToRead = charSpacing;
    }

    // NOTE: We need to remove key color borders from image to avoid weird
    // artifacts on texture scaling when using TEXTURE_FILTER_BILINEAR or TEXTURE_FILTER_TRILINEAR
    for (int i = 0; i < image.height*image.width; i++) if (COLOR_EQUAL(pixels[i], key)) pixels[i] = BLANK;

    // Create a new image with the processed color data (key color replaced by BLANK)
    Image fontClear = {
        .data = pixels,
        .width = image.width,
        .height = image.height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    // Set font with all data parsed from image
    if (isGpuReady) font.texture = LoadTextureFromImage(fontClear); // Convert processed image to OpenGL texture
    font.glyphCount = index;
    font.glyphPadding = 0;

    // We got tempCharValues and tempCharsRecs populated with chars data
    // Now we move temp data to sized charValues and charRecs arrays
    font.glyphs = (GlyphInfo *)RL_MALLOC(font.glyphCount*sizeof(GlyphInfo));
    font.recs = (Rectangle *)RL_MALLOC(font.glyphCount*sizeof(Rectangle));

    for (int i = 0; i < font.glyphCount; i++)
    {
        font.glyphs[i].value = tempCharValues[i];

        // Get character rectangle in the font atlas texture
        font.recs[i] = tempCharRecs[i];

        // NOTE: On image based fonts (XNA style), character offsets and xAdvance are not required (set to 0)
        font.glyphs[i].offsetX = 0;
        font.glyphs[i].offsetY = 0;
        font.glyphs[i].advanceX = 0;

        // Fill character image data from fontClear data
        font.glyphs[i].image = ImageFromImage(fontClear, tempCharRecs[i]);
    }

    UnloadImage(fontClear);     // Unload processed image once converted to texture

    font.baseSize = (int)font.recs[0].height;

    return font;
}

// Load font from memory buffer, fileType refers to extension: i.e. ".ttf"
Font LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount)
{
    Font font = { 0 };

    char fileExtLower[16] = { 0 };
    strncpy(fileExtLower, TextToLower(fileType), 16 - 1);

    font.baseSize = fontSize;
    font.glyphCount = (codepointCount > 0)? codepointCount : 95;
    font.glyphPadding = 0;

#if defined(SUPPORT_FILEFORMAT_TTF)
    if (TextIsEqual(fileExtLower, ".ttf") ||
        TextIsEqual(fileExtLower, ".otf"))
    {
        font.glyphs = LoadFontData(fileData, dataSize, font.baseSize, codepoints, font.glyphCount, FONT_DEFAULT);
    }
    else
#endif
#if defined(SUPPORT_FILEFORMAT_BDF)
    if (TextIsEqual(fileExtLower, ".bdf"))
    {
        font.glyphs = LoadFontDataBDF(fileData, dataSize, codepoints, font.glyphCount, &font.baseSize);
    }
    else
#endif
    {
        font.glyphs = NULL;
    }

#if defined(SUPPORT_FILEFORMAT_TTF) || defined(SUPPORT_FILEFORMAT_BDF)
    if (font.glyphs != NULL)
    {
        font.glyphPadding = FONT_TTF_DEFAULT_CHARS_PADDING;

        Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, font.glyphCount, font.baseSize, font.glyphPadding, 0);
        if (isGpuReady) font.texture = LoadTextureFromImage(atlas);

        // Update glyphs[i].image to use alpha, required to be used on ImageDrawText()
        for (int i = 0; i < font.glyphCount; i++)
        {
            UnloadImage(font.glyphs[i].image);
            font.glyphs[i].image = ImageFromImage(atlas, font.recs[i]);
        }

        UnloadImage(atlas);

        TRACELOG(LOG_INFO, "FONT: Data loaded successfully (%i pixel size | %i glyphs)", font.baseSize, font.glyphCount);
    }
    else font = GetFontDefault();
#else
    font = GetFontDefault();
#endif

    return font;
}

// Check if a font is valid (font data loaded)
// WARNING: GPU texture not checked
bool IsFontValid(Font font)
{
    return ((font.baseSize > 0) &&      // Validate font size
            (font.glyphCount > 0) &&    // Validate font contains some glyph
            (font.recs != NULL) &&      // Validate font recs defining glyphs on texture atlas
            (font.glyphs != NULL));     // Validate glyph data is loaded

    // NOTE: Further validations could be done to verify if recs and glyphs contain valid data (glyphs values, metrics...)
}

// Load font data for further use
// NOTE: Requires TTF font memory data and can generate SDF data
GlyphInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type)
{
    // NOTE: Using some SDF generation default values,
    // trades off precision with ability to handle *smaller* sizes
#ifndef FONT_SDF_CHAR_PADDING
    #define FONT_SDF_CHAR_PADDING            4      // SDF font generation char padding
#endif
#ifndef FONT_SDF_ON_EDGE_VALUE
    #define FONT_SDF_ON_EDGE_VALUE         128      // SDF font generation on edge value
#endif
#ifndef FONT_SDF_PIXEL_DIST_SCALE
    #define FONT_SDF_PIXEL_DIST_SCALE     64.0f     // SDF font generation pixel distance scale
#endif
#ifndef FONT_BITMAP_ALPHA_THRESHOLD
    #define FONT_BITMAP_ALPHA_THRESHOLD     80      // Bitmap (B&W) font generation alpha threshold
#endif

    GlyphInfo *chars = NULL;

#if defined(SUPPORT_FILEFORMAT_TTF)
    // Load font data (including pixel data) from TTF memory file
    // NOTE: Loaded information should be enough to generate font image atlas, using any packaging method
    if (fileData != NULL)
    {
        bool genFontChars = false;
        stbtt_fontinfo fontInfo = { 0 };

        if (stbtt_InitFont(&fontInfo, (unsigned char *)fileData, 0))     // Initialize font for data reading
        {
            // Calculate font scale factor
            float scaleFactor = stbtt_ScaleForPixelHeight(&fontInfo, (float)fontSize);

            // Calculate font basic metrics
            // NOTE: ascent is equivalent to font baseline
            int ascent, descent, lineGap;
            stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

            // In case no chars count provided, default to 95
            codepointCount = (codepointCount > 0)? codepointCount : 95;

            // Fill fontChars in case not provided externally
            // NOTE: By default we fill glyphCount consecutively, starting at 32 (Space)
            if (codepoints == NULL)
            {
                codepoints = (int *)RL_MALLOC(codepointCount*sizeof(int));
                for (int i = 0; i < codepointCount; i++) codepoints[i] = i + 32;
                genFontChars = true;
            }

            chars = (GlyphInfo *)RL_CALLOC(codepointCount, sizeof(GlyphInfo));

            // NOTE: Using simple packaging, one char after another
            for (int i = 0; i < codepointCount; i++)
            {
                int chw = 0, chh = 0;   // Character width and height (on generation)
                int ch = codepoints[i];  // Character value to get info for
                chars[i].value = ch;

                //  Render a unicode codepoint to a bitmap
                //      stbtt_GetCodepointBitmap()           -- allocates and returns a bitmap
                //      stbtt_GetCodepointBitmapBox()        -- how big the bitmap must be
                //      stbtt_MakeCodepointBitmap()          -- renders into bitmap you provide

                // Check if a glyph is available in the font
                // WARNING: if (index == 0), glyph not found, it could fallback to default .notdef glyph (if defined in font)
                int index = stbtt_FindGlyphIndex(&fontInfo, ch);

                if (index > 0)
                {
                    switch (type)
                    {
                        case FONT_DEFAULT:
                        case FONT_BITMAP: chars[i].image.data = stbtt_GetCodepointBitmap(&fontInfo, scaleFactor, scaleFactor, ch, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY); break;
                        case FONT_SDF: if (ch != 32) chars[i].image.data = stbtt_GetCodepointSDF(&fontInfo, scaleFactor, ch, FONT_SDF_CHAR_PADDING, FONT_SDF_ON_EDGE_VALUE, FONT_SDF_PIXEL_DIST_SCALE, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY); break;
                        default: break;
                    }

                    if (chars[i].image.data != NULL)    // Glyph data has been found in the font
                    {
                        stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advanceX, NULL);
                        chars[i].advanceX = (int)((float)chars[i].advanceX*scaleFactor);

                        if (chh > fontSize) TRACELOG(LOG_WARNING, "FONT: Character [0x%08x] size is bigger than expected font size", ch);

                        // Load characters images
                        chars[i].image.width = chw;
                        chars[i].image.height = chh;
                        chars[i].image.mipmaps = 1;
                        chars[i].image.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;

                        chars[i].offsetY += (int)((float)ascent*scaleFactor);
                    }

                    // NOTE: We create an empty image for space character,
                    // it could be further required for atlas packing
                    if (ch == 32)
                    {
                        stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advanceX, NULL);
                        chars[i].advanceX = (int)((float)chars[i].advanceX*scaleFactor);

                        Image imSpace = {
                            .data = RL_CALLOC(chars[i].advanceX*fontSize, 2),
                            .width = chars[i].advanceX,
                            .height = fontSize,
                            .mipmaps = 1,
                            .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
                        };

                        chars[i].image = imSpace;
                    }

                    if (type == FONT_BITMAP)
                    {
                        // Aliased bitmap (black & white) font generation, avoiding anti-aliasing
                        // NOTE: For optimum results, bitmap font should be generated at base pixel size
                        for (int p = 0; p < chw*chh; p++)
                        {
                            if (((unsigned char *)chars[i].image.data)[p] < FONT_BITMAP_ALPHA_THRESHOLD) ((unsigned char *)chars[i].image.data)[p] = 0;
                            else ((unsigned char *)chars[i].image.data)[p] = 255;
                        }
                    }
                }
                else
                {
                    // TODO: Use some fallback glyph for codepoints not found in the font
                }
            }
        }
        else TRACELOG(LOG_WARNING, "FONT: Failed to process TTF font data");

        if (genFontChars) RL_FREE(codepoints);
    }
#endif

    return chars;
}

// Generate image font atlas using chars info
// NOTE: Packing method: 0-Default, 1-Skyline
#if defined(SUPPORT_FILEFORMAT_TTF) || defined(SUPPORT_FILEFORMAT_BDF)
Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod)
{
    Image atlas = { 0 };

    if (glyphs == NULL)
    {
        TRACELOG(LOG_WARNING, "FONT: Provided chars info not valid, returning empty image atlas");
        return atlas;
    }

    *glyphRecs = NULL;

    // In case no chars count provided we suppose default of 95
    glyphCount = (glyphCount > 0)? glyphCount : 95;

    // NOTE: Rectangles memory is loaded here!
    Rectangle *recs = (Rectangle *)RL_MALLOC(glyphCount*sizeof(Rectangle));

    // Calculate image size based on total glyph width and glyph row count
    int totalWidth = 0;
    int maxGlyphWidth = 0;

    for (int i = 0; i < glyphCount; i++)
    {
        if (glyphs[i].image.width > maxGlyphWidth) maxGlyphWidth = glyphs[i].image.width;
        totalWidth += glyphs[i].image.width + 2*padding;
    }

//#define SUPPORT_FONT_ATLAS_SIZE_CONSERVATIVE
#if defined(SUPPORT_FONT_ATLAS_SIZE_CONSERVATIVE)
    int rowCount = 0;
    int imageSize = 64;  // Define minimum starting value to avoid unnecessary calculation steps for very small images

    // NOTE: maxGlyphWidth is maximum possible space left at the end of row
    while (totalWidth > (imageSize - maxGlyphWidth)*rowCount)
    {
        imageSize *= 2;                                 // Double the size of image (to keep POT)
        rowCount = imageSize/(fontSize + 2*padding);    // Calculate new row count for the new image size
    }

    atlas.width = imageSize;   // Atlas bitmap width
    atlas.height = imageSize;  // Atlas bitmap height
#else
    int paddedFontSize = fontSize + 2*padding;
    // No need for a so-conservative atlas generation
    float totalArea = totalWidth*paddedFontSize*1.2f;
    float imageMinSize = sqrtf(totalArea);
    int imageSize = (int)powf(2, ceilf(logf(imageMinSize)/logf(2)));

    if (totalArea < ((imageSize*imageSize)/2))
    {
        atlas.width = imageSize;    // Atlas bitmap width
        atlas.height = imageSize/2; // Atlas bitmap height
    }
    else
    {
        atlas.width = imageSize;   // Atlas bitmap width
        atlas.height = imageSize;  // Atlas bitmap height
    }
#endif

    atlas.data = (unsigned char *)RL_CALLOC(1, atlas.width*atlas.height);   // Create a bitmap to store characters (8 bpp)
    atlas.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    atlas.mipmaps = 1;

    // DEBUG: We can see padding in the generated image setting a gray background...
    //for (int i = 0; i < atlas.width*atlas.height; i++) ((unsigned char *)atlas.data)[i] = 100;

    if (packMethod == 0)   // Use basic packing algorithm
    {
        int offsetX = padding;
        int offsetY = padding;

        // NOTE: Using simple packaging, one char after another
        for (int i = 0; i < glyphCount; i++)
        {
            // Check remaining space for glyph
            if (offsetX >= (atlas.width - glyphs[i].image.width - 2*padding))
            {
                offsetX = padding;

                // NOTE: Be careful on offsetY for SDF fonts, by default SDF
                // use an internal padding of 4 pixels, it means char rectangle
                // height is bigger than fontSize, it could be up to (fontSize + 8)
                offsetY += (fontSize + 2*padding);

                if (offsetY > (atlas.height - fontSize - padding))
                {
                    for (int j = i + 1; j < glyphCount; j++)
                    {
                        TRACELOG(LOG_WARNING, "FONT: Failed to package character (%i)", j);
                        // Make sure remaining recs contain valid data
                        recs[j].x = 0;
                        recs[j].y = 0;
                        recs[j].width = 0;
                        recs[j].height = 0;
                    }
                    break;
                }
            }

            // Copy pixel data from glyph image to atlas
            for (int y = 0; y < glyphs[i].image.height; y++)
            {
                for (int x = 0; x < glyphs[i].image.width; x++)
                {
                    ((unsigned char *)atlas.data)[(offsetY + y)*atlas.width + (offsetX + x)] = ((unsigned char *)glyphs[i].image.data)[y*glyphs[i].image.width + x];
                }
            }

            // Fill chars rectangles in atlas info
            recs[i].x = (float)offsetX;
            recs[i].y = (float)offsetY;
            recs[i].width = (float)glyphs[i].image.width;
            recs[i].height = (float)glyphs[i].image.height;

            // Move atlas position X for next character drawing
            offsetX += (glyphs[i].image.width + 2*padding);
        }
    }
    else if (packMethod == 1)  // Use Skyline rect packing algorithm (stb_pack_rect)
    {
        stbrp_context *context = (stbrp_context *)RL_MALLOC(sizeof(*context));
        stbrp_node *nodes = (stbrp_node *)RL_MALLOC(glyphCount*sizeof(*nodes));

        stbrp_init_target(context, atlas.width, atlas.height, nodes, glyphCount);
        stbrp_rect *rects = (stbrp_rect *)RL_MALLOC(glyphCount*sizeof(stbrp_rect));

        // Fill rectangles for packaging
        for (int i = 0; i < glyphCount; i++)
        {
            rects[i].id = i;
            rects[i].w = glyphs[i].image.width + 2*padding;
            rects[i].h = glyphs[i].image.height + 2*padding;
        }

        // Package rectangles into atlas
        stbrp_pack_rects(context, rects, glyphCount);

        for (int i = 0; i < glyphCount; i++)
        {
            // It returns char rectangles in atlas
            recs[i].x = rects[i].x + (float)padding;
            recs[i].y = rects[i].y + (float)padding;
            recs[i].width = (float)glyphs[i].image.width;
            recs[i].height = (float)glyphs[i].image.height;

            if (rects[i].was_packed)
            {
                // Copy pixel data from fc.data to atlas
                for (int y = 0; y < glyphs[i].image.height; y++)
                {
                    for (int x = 0; x < glyphs[i].image.width; x++)
                    {
                        ((unsigned char *)atlas.data)[(rects[i].y + padding + y)*atlas.width + (rects[i].x + padding + x)] = ((unsigned char *)glyphs[i].image.data)[y*glyphs[i].image.width + x];
                    }
                }
            }
            else TRACELOG(LOG_WARNING, "FONT: Failed to package character (%i)", i);
        }

        RL_FREE(rects);
        RL_FREE(nodes);
        RL_FREE(context);
    }

#if defined(SUPPORT_FONT_ATLAS_WHITE_REC)
    // Add a 3x3 white rectangle at the bottom-right corner of the generated atlas,
    // useful to use as the white texture to draw shapes with raylib, using this rectangle
    // shapes and text can be backed into a single draw call: SetShapesTexture()
    for (int i = 0, k = atlas.width*atlas.height - 1; i < 3; i++)
    {
        ((unsigned char *)atlas.data)[k - 0] = 255;
        ((unsigned char *)atlas.data)[k - 1] = 255;
        ((unsigned char *)atlas.data)[k - 2] = 255;
        k -= atlas.width;
    }
#endif

    // Convert image data from GRAYSCALE to GRAY_ALPHA
    unsigned char *dataGrayAlpha = (unsigned char *)RL_MALLOC(atlas.width*atlas.height*sizeof(unsigned char)*2); // Two channels

    for (int i = 0, k = 0; i < atlas.width*atlas.height; i++, k += 2)
    {
        dataGrayAlpha[k] = 255;
        dataGrayAlpha[k + 1] = ((unsigned char *)atlas.data)[i];
    }

    RL_FREE(atlas.data);
    atlas.data = dataGrayAlpha;
    atlas.format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;

    *glyphRecs = recs;

    return atlas;
}
#endif

// Unload font glyphs info data (RAM)
void UnloadFontData(GlyphInfo *glyphs, int glyphCount)
{
    if (glyphs != NULL)
    {
        for (int i = 0; i < glyphCount; i++) UnloadImage(glyphs[i].image);

        RL_FREE(glyphs);
    }
}

// Unload Font from GPU memory (VRAM)
void UnloadFont(Font font)
{
    // NOTE: Make sure font is not default font (fallback)
    if (font.texture.id != GetFontDefault().texture.id)
    {
        UnloadFontData(font.glyphs, font.glyphCount);
        if (isGpuReady) UnloadTexture(font.texture);
        RL_FREE(font.recs);

        TRACELOGD("FONT: Unloaded font data from RAM and VRAM");
    }
}

// Export font as code file, returns true on success
bool ExportFontAsCode(Font font, const char *fileName)
{
    bool success = false;

#ifndef TEXT_BYTES_PER_LINE
    #define TEXT_BYTES_PER_LINE     20
#endif

    #define MAX_FONT_DATA_SIZE      1024*1024       // 1 MB

    // Get file name from path
    char fileNamePascal[256] = { 0 };
    strncpy(fileNamePascal, TextToPascal(GetFileNameWithoutExt(fileName)), 256 - 1);

    // NOTE: Text data buffer size is estimated considering image data size in bytes
    // and requiring 6 char bytes for every byte: "0x00, "
    char *txtData = (char *)RL_CALLOC(MAX_FONT_DATA_SIZE, sizeof(char));

    int byteCount = 0;
    byteCount += sprintf(txtData + byteCount, "////////////////////////////////////////////////////////////////////////////////////////\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// FontAsCode exporter v1.0 - Font data exported as an array of bytes                 //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// more info and bugs-report:  github.com/raysan5/raylib                              //\n");
    byteCount += sprintf(txtData + byteCount, "// feedback and support:       ray[at]raylib.com                                      //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// Copyright (c) 2018-2025 Ramon Santamaria (@raysan5)                                //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// ---------------------------------------------------------------------------------- //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// TODO: Fill the information and license of the exported font here:                  //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// Font name:    ....                                                                 //\n");
    byteCount += sprintf(txtData + byteCount, "// Font creator: ....                                                                 //\n");
    byteCount += sprintf(txtData + byteCount, "// Font LICENSE: ....                                                                 //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "////////////////////////////////////////////////////////////////////////////////////////\n\n");

    // Support font export and initialization
    // NOTE: This mechanism is highly coupled to raylib
    Image image = LoadImageFromTexture(font.texture);
    if (image.format != PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA) TRACELOG(LOG_WARNING, "Font export as code: Font image format is not GRAY+ALPHA!");
    int imageDataSize = GetPixelDataSize(image.width, image.height, image.format);

    // Image data is usually GRAYSCALE + ALPHA and can be reduced to GRAYSCALE
    //ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);

#define SUPPORT_COMPRESSED_FONT_ATLAS
#if defined(SUPPORT_COMPRESSED_FONT_ATLAS)
    // WARNING: Data is compressed using raylib CompressData() DEFLATE,
    // it requires to be decompressed with raylib DecompressData(), that requires
    // compiling raylib with SUPPORT_COMPRESSION_API config flag enabled

    // Compress font image data
    int compDataSize = 0;
    unsigned char *compData = CompressData((const unsigned char *)image.data, imageDataSize, &compDataSize);

    // Save font image data (compressed)
    byteCount += sprintf(txtData + byteCount, "#define COMPRESSED_DATA_SIZE_FONT_%s %i\n\n", TextToUpper(fileNamePascal), compDataSize);
    byteCount += sprintf(txtData + byteCount, "// Font image pixels data compressed (DEFLATE)\n");
    byteCount += sprintf(txtData + byteCount, "// NOTE: Original pixel data simplified to GRAYSCALE\n");
    byteCount += sprintf(txtData + byteCount, "static unsigned char fontData_%s[COMPRESSED_DATA_SIZE_FONT_%s] = { ", fileNamePascal, TextToUpper(fileNamePascal));
    for (int i = 0; i < compDataSize - 1; i++) byteCount += sprintf(txtData + byteCount, ((i%TEXT_BYTES_PER_LINE == 0)? "0x%02x,\n    " : "0x%02x, "), compData[i]);
    byteCount += sprintf(txtData + byteCount, "0x%02x };\n\n", compData[compDataSize - 1]);
    RL_FREE(compData);
#else
    // Save font image data (uncompressed)
    byteCount += sprintf(txtData + byteCount, "// Font image pixels data\n");
    byteCount += sprintf(txtData + byteCount, "// NOTE: 2 bytes per pixel, GRAY + ALPHA channels\n");
    byteCount += sprintf(txtData + byteCount, "static unsigned char fontImageData_%s[%i] = { ", fileNamePascal, imageDataSize);
    for (int i = 0; i < imageDataSize - 1; i++) byteCount += sprintf(txtData + byteCount, ((i%TEXT_BYTES_PER_LINE == 0)? "0x%02x,\n    " : "0x%02x, "), ((unsigned char *)imFont.data)[i]);
    byteCount += sprintf(txtData + byteCount, "0x%02x };\n\n", ((unsigned char *)imFont.data)[imageDataSize - 1]);
#endif

    // Save font recs data
    byteCount += sprintf(txtData + byteCount, "// Font characters rectangles data\n");
    byteCount += sprintf(txtData + byteCount, "static Rectangle fontRecs_%s[%i] = {\n", fileNamePascal, font.glyphCount);
    for (int i = 0; i < font.glyphCount; i++)
    {
        byteCount += sprintf(txtData + byteCount, "    { %1.0f, %1.0f, %1.0f , %1.0f },\n", font.recs[i].x, font.recs[i].y, font.recs[i].width, font.recs[i].height);
    }
    byteCount += sprintf(txtData + byteCount, "};\n\n");

    // Save font glyphs data
    // NOTE: Glyphs image data not saved (grayscale pixels),
    // it could be generated from image and recs
    byteCount += sprintf(txtData + byteCount, "// Font glyphs info data\n");
    byteCount += sprintf(txtData + byteCount, "// NOTE: No glyphs.image data provided\n");
    byteCount += sprintf(txtData + byteCount, "static GlyphInfo fontGlyphs_%s[%i] = {\n", fileNamePascal, font.glyphCount);
    for (int i = 0; i < font.glyphCount; i++)
    {
        byteCount += sprintf(txtData + byteCount, "    { %i, %i, %i, %i, { 0 }},\n", font.glyphs[i].value, font.glyphs[i].offsetX, font.glyphs[i].offsetY, font.glyphs[i].advanceX);
    }
    byteCount += sprintf(txtData + byteCount, "};\n\n");

    // Custom font loading function
    byteCount += sprintf(txtData + byteCount, "// Font loading function: %s\n", fileNamePascal);
    byteCount += sprintf(txtData + byteCount, "static Font LoadFont_%s(void)\n{\n", fileNamePascal);
    byteCount += sprintf(txtData + byteCount, "    Font font = { 0 };\n\n");
    byteCount += sprintf(txtData + byteCount, "    font.baseSize = %i;\n", font.baseSize);
    byteCount += sprintf(txtData + byteCount, "    font.glyphCount = %i;\n", font.glyphCount);
    byteCount += sprintf(txtData + byteCount, "    font.glyphPadding = %i;\n\n", font.glyphPadding);
    byteCount += sprintf(txtData + byteCount, "    // Custom font loading\n");
#if defined(SUPPORT_COMPRESSED_FONT_ATLAS)
    byteCount += sprintf(txtData + byteCount, "    // NOTE: Compressed font image data (DEFLATE), it requires DecompressData() function\n");
    byteCount += sprintf(txtData + byteCount, "    int fontDataSize_%s = 0;\n", fileNamePascal);
    byteCount += sprintf(txtData + byteCount, "    unsigned char *data = DecompressData(fontData_%s, COMPRESSED_DATA_SIZE_FONT_%s, &fontDataSize_%s);\n", fileNamePascal, TextToUpper(fileNamePascal), fileNamePascal);
    byteCount += sprintf(txtData + byteCount, "    Image imFont = { data, %i, %i, 1, %i };\n\n", image.width, image.height, image.format);
#else
    byteCount += sprintf(txtData + byteCount, "    Image imFont = { fontImageData_%s, %i, %i, 1, %i };\n\n", styleName, image.width, image.height, image.format);
#endif
    byteCount += sprintf(txtData + byteCount, "    // Load texture from image\n");
    byteCount += sprintf(txtData + byteCount, "    font.texture = LoadTextureFromImage(imFont);\n");
#if defined(SUPPORT_COMPRESSED_FONT_ATLAS)
    byteCount += sprintf(txtData + byteCount, "    UnloadImage(imFont);  // Uncompressed data can be unloaded from memory\n\n");
#endif
    // We have two possible mechanisms to assign font.recs and font.glyphs data,
    // that data is already available as global arrays, we two options to assign that data:
    //  - 1. Data copy. This option consumes more memory and Font MUST be unloaded by user, requiring additional code
    //  - 2. Data assignment. This option consumes less memory and Font MUST NOT be unloaded by user because data is on protected DATA segment
//#define SUPPORT_FONT_DATA_COPY
#if defined(SUPPORT_FONT_DATA_COPY)
    byteCount += sprintf(txtData + byteCount, "    // Copy glyph recs data from global fontRecs\n");
    byteCount += sprintf(txtData + byteCount, "    // NOTE: Required to avoid issues if trying to free font\n");
    byteCount += sprintf(txtData + byteCount, "    font.recs = (Rectangle *)malloc(font.glyphCount*sizeof(Rectangle));\n");
    byteCount += sprintf(txtData + byteCount, "    memcpy(font.recs, fontRecs_%s, font.glyphCount*sizeof(Rectangle));\n\n", fileNamePascal);

    byteCount += sprintf(txtData + byteCount, "    // Copy font glyph info data from global fontChars\n");
    byteCount += sprintf(txtData + byteCount, "    // NOTE: Required to avoid issues if trying to free font\n");
    byteCount += sprintf(txtData + byteCount, "    font.glyphs = (GlyphInfo *)malloc(font.glyphCount*sizeof(GlyphInfo));\n");
    byteCount += sprintf(txtData + byteCount, "    memcpy(font.glyphs, fontGlyphs_%s, font.glyphCount*sizeof(GlyphInfo));\n\n", fileNamePascal);
#else
    byteCount += sprintf(txtData + byteCount, "    // Assign glyph recs and info data directly\n");
    byteCount += sprintf(txtData + byteCount, "    // WARNING: This font data must not be unloaded\n");
    byteCount += sprintf(txtData + byteCount, "    font.recs = fontRecs_%s;\n", fileNamePascal);
    byteCount += sprintf(txtData + byteCount, "    font.glyphs = fontGlyphs_%s;\n\n", fileNamePascal);
#endif
    byteCount += sprintf(txtData + byteCount, "    return font;\n");
    byteCount += sprintf(txtData + byteCount, "}\n");

    UnloadImage(image);

    // NOTE: Text data size exported is determined by '\0' (NULL) character
    success = SaveFileText(fileName, txtData);

    RL_FREE(txtData);

    if (success != 0) TRACELOG(LOG_INFO, "FILEIO: [%s] Font as code exported successfully", fileName);
    else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to export font as code", fileName);

    return success;
}

// Draw current FPS
// NOTE: Uses default font
void DrawFPS(int posX, int posY)
{
    Color color = LIME;                         // Good FPS
    int fps = GetFPS();

    if ((fps < 30) && (fps >= 15)) color = ORANGE;  // Warning FPS
    else if (fps < 15) color = RED;             // Low FPS

    DrawText(TextFormat("%2i FPS", fps), posX, posY, 20, color);
}

// Draw text (using default font)
// NOTE: fontSize work like in any drawing program but if fontSize is lower than font-base-size, then font-base-size is used
// NOTE: chars spacing is proportional to fontSize
void DrawText(const char *text, int posX, int posY, int fontSize, Color color)
{
    // Check if default font has been loaded
    if (GetFontDefault().texture.id != 0)
    {
        Vector2 position = { (float)posX, (float)posY };

        int defaultFontSize = 10;   // Default Font chars height in pixel
        if (fontSize < defaultFontSize) fontSize = defaultFontSize;
        int spacing = fontSize/defaultFontSize;

        DrawTextEx(GetFontDefault(), text, position, (float)fontSize, (float)spacing, color);
    }
}

// Draw text using Font
// NOTE: chars spacing is NOT proportional to fontSize
void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint)
{
    if (font.texture.id == 0) font = GetFontDefault();  // Security check in case of not valid font

    int size = TextLength(text);    // Total size in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0;          // Offset between lines (on linebreak '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font.baseSize;         // Character quad scaling factor

    for (int i = 0; i < size;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        if (codepoint == '\n')
        {
            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            textOffsetY += (fontSize + textLineSpacing);
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint(font, codepoint, (Vector2){ position.x + textOffsetX, position.y + textOffsetY }, fontSize, tint);
            }

            if (font.glyphs[index].advanceX == 0) textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
            else textOffsetX += ((float)font.glyphs[index].advanceX*scaleFactor + spacing);
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}

// Draw text using Font and pro parameters (rotation)
void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint)
{
    rlPushMatrix();

        rlTranslatef(position.x, position.y, 0.0f);
        rlRotatef(rotation, 0.0f, 0.0f, 1.0f);
        rlTranslatef(-origin.x, -origin.y, 0.0f);

        DrawTextEx(font, text, (Vector2){ 0.0f, 0.0f }, fontSize, spacing, tint);

    rlPopMatrix();
}

// Draw one character (codepoint)
void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scaleFactor = fontSize/font.baseSize;     // Character quad scaling factor

    // Character destination rectangle on screen
    // NOTE: We consider glyphPadding on drawing
    Rectangle dstRec = { position.x + font.glyphs[index].offsetX*scaleFactor - (float)font.glyphPadding*scaleFactor,
                      position.y + font.glyphs[index].offsetY*scaleFactor - (float)font.glyphPadding*scaleFactor,
                      (font.recs[index].width + 2.0f*font.glyphPadding)*scaleFactor,
                      (font.recs[index].height + 2.0f*font.glyphPadding)*scaleFactor };

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f*font.glyphPadding, font.recs[index].height + 2.0f*font.glyphPadding };

    // Draw the character texture on the screen
    DrawTexturePro(font.texture, srcRec, dstRec, (Vector2){ 0, 0 }, 0.0f, tint);
}

// Draw multiple character (codepoints)
void DrawTextCodepoints(Font font, const int *codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint)
{
    float textOffsetY = 0;          // Offset between lines (on linebreak '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font.baseSize;         // Character quad scaling factor

    for (int i = 0; i < codepointCount; i++)
    {
        int index = GetGlyphIndex(font, codepoints[i]);

        if (codepoints[i] == '\n')
        {
            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            textOffsetY += (fontSize + textLineSpacing);
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoints[i] != ' ') && (codepoints[i] != '\t'))
            {
                DrawTextCodepoint(font, codepoints[i], (Vector2){ position.x + textOffsetX, position.y + textOffsetY }, fontSize, tint);
            }

            if (font.glyphs[index].advanceX == 0) textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
            else textOffsetX += ((float)font.glyphs[index].advanceX*scaleFactor + spacing);
        }
    }
}

// Set vertical line spacing when drawing with line-breaks
void SetTextLineSpacing(int spacing)
{
    textLineSpacing = spacing;
}

// Measure string width for default font
int MeasureText(const char *text, int fontSize)
{
    Vector2 textSize = { 0.0f, 0.0f };

    // Check if default font has been loaded
    if (GetFontDefault().texture.id != 0)
    {
        int defaultFontSize = 10;   // Default Font chars height in pixel
        if (fontSize < defaultFontSize) fontSize = defaultFontSize;
        int spacing = fontSize/defaultFontSize;

        textSize = MeasureTextEx(GetFontDefault(), text, (float)fontSize, (float)spacing);
    }

    return (int)textSize.x;
}

// Measure string size for Font
Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing)
{
    Vector2 textSize = { 0 };

    if ((isGpuReady && (font.texture.id == 0)) ||
        (text == NULL) || (text[0] == '\0')) return textSize; // Security check

    int size = TextLength(text);    // Get size in bytes of text
    int tempByteCounter = 0;        // Used to count longer text line num chars
    int byteCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f;     // Used to count longer text line width

    float textHeight = fontSize;
    float scaleFactor = fontSize/(float)font.baseSize;

    int letter = 0;                 // Current character
    int index = 0;                  // Index position in sprite font

    for (int i = 0; i < size;)
    {
        byteCounter++;

        int codepointByteCount = 0;
        letter = GetCodepointNext(&text[i], &codepointByteCount);
        index = GetGlyphIndex(font, letter);

        i += codepointByteCount;

        if (letter != '\n')
        {
            if (font.glyphs[index].advanceX > 0) textWidth += font.glyphs[index].advanceX;
            else textWidth += (font.recs[index].width + font.glyphs[index].offsetX);
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            byteCounter = 0;
            textWidth = 0;

            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            textHeight += (fontSize + textLineSpacing);
        }

        if (tempByteCounter < byteCounter) tempByteCounter = byteCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    textSize.x = tempTextWidth*scaleFactor + (float)((tempByteCounter - 1)*spacing);
    textSize.y = textHeight;

    return textSize;
}

// Get index position for a unicode character on font
// NOTE: If codepoint is not found in the font it fallbacks to '?'
int GetGlyphIndex(Font font, int codepoint)
{
    int index = 0;
    if (!IsFontValid(font)) return index;

#define SUPPORT_UNORDERED_CHARSET
#if defined(SUPPORT_UNORDERED_CHARSET)
    int fallbackIndex = 0;      // Get index of fallback glyph '?'

    // Look for character index in the unordered charset
    for (int i = 0; i < font.glyphCount; i++)
    {
        if (font.glyphs[i].value == 63) fallbackIndex = i;

        if (font.glyphs[i].value == codepoint)
        {
            index = i;
            break;
        }
    }

    if ((index == 0) && (font.glyphs[0].value != codepoint)) index = fallbackIndex;
#else
    index = codepoint - 32;
#endif

    return index;
}

// Get glyph font info data for a codepoint (unicode character)
// NOTE: If codepoint is not found in the font it fallbacks to '?'
GlyphInfo GetGlyphInfo(Font font, int codepoint)
{
    GlyphInfo info = { 0 };

    info = font.glyphs[GetGlyphIndex(font, codepoint)];

    return info;
}

// Get glyph rectangle in font atlas for a codepoint (unicode character)
// NOTE: If codepoint is not found in the font it fallbacks to '?'
Rectangle GetGlyphAtlasRec(Font font, int codepoint)
{
    Rectangle rec = { 0 };

    rec = font.recs[GetGlyphIndex(font, codepoint)];

    return rec;
}

//----------------------------------------------------------------------------------
// Text strings management functions
//----------------------------------------------------------------------------------
// Get text length in bytes, check for \0 character
unsigned int TextLength(const char *text)
{
    unsigned int length = 0;

    if (text != NULL)
    {
        // NOTE: Alternative: use strlen(text)

        while (*text++) length++;
    }

    return length;
}

// Formatting of text with variables to 'embed'
// WARNING: String returned will expire after this function is called MAX_TEXTFORMAT_BUFFERS times
const char *TextFormat(const char *text, ...)
{
#ifndef MAX_TEXTFORMAT_BUFFERS
    #define MAX_TEXTFORMAT_BUFFERS 4        // Maximum number of static buffers for text formatting
#endif

    // We create an array of buffers so strings don't expire until MAX_TEXTFORMAT_BUFFERS invocations
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
    static int index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);   // Clear buffer before using

    va_list args;
    va_start(args, text);
    int requiredByteCount = vsnprintf(currentBuffer, MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

    // If requiredByteCount is larger than the MAX_TEXT_BUFFER_LENGTH, then overflow occured
    if (requiredByteCount >= MAX_TEXT_BUFFER_LENGTH)
    {
        // Inserting "..." at the end of the string to mark as truncated
        char *truncBuffer = buffers[index] + MAX_TEXT_BUFFER_LENGTH - 4; // Adding 4 bytes = "...\0"
        sprintf(truncBuffer, "...");
    }

    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

    return currentBuffer;
}

// Get integer value from text
// NOTE: This function replaces atoi() [stdlib.h]
int TextToInteger(const char *text)
{
    int value = 0;
    int sign = 1;

    if ((text[0] == '+') || (text[0] == '-'))
    {
        if (text[0] == '-') sign = -1;
        text++;
    }

    for (int i = 0; ((text[i] >= '0') && (text[i] <= '9')); i++) value = value*10 + (int)(text[i] - '0');

    return value*sign;
}

// Get float value from text
// NOTE: This function replaces atof() [stdlib.h]
// WARNING: Only '.' character is understood as decimal point
float TextToFloat(const char *text)
{
    float value = 0.0f;
    float sign = 1.0f;

    if ((text[0] == '+') || (text[0] == '-'))
    {
        if (text[0] == '-') sign = -1.0f;
        text++;
    }

    int i = 0;
    for (; ((text[i] >= '0') && (text[i] <= '9')); i++) value = value*10.0f + (float)(text[i] - '0');

    if (text[i++] == '.')
    {
        float divisor = 10.0f;
        for (; ((text[i] >= '0') && (text[i] <= '9')); i++)
        {
            value += ((float)(text[i] - '0'))/divisor;
            divisor = divisor*10.0f;
        }
    }

    return value*sign;
}

#if defined(SUPPORT_TEXT_MANIPULATION)
// Copy one string to another, returns bytes copied
int TextCopy(char *dst, const char *src)
{
    int bytes = 0;

    if ((src != NULL) && (dst != NULL))
    {
        // NOTE: Alternative: use strcpy(dst, src)

        while (*src != '\0')
        {
            *dst = *src;
            dst++;
            src++;

            bytes++;
        }

        *dst = '\0';
    }

    return bytes;
}

// Check if two text string are equal
// REQUIRES: strcmp()
bool TextIsEqual(const char *text1, const char *text2)
{
    bool result = false;

    if ((text1 != NULL) && (text2 != NULL))
    {
        if (strcmp(text1, text2) == 0) result = true;
    }

    return result;
}

// Get a piece of a text string
const char *TextSubtext(const char *text, int position, int length)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    int textLength = TextLength(text);

    if (position >= textLength)
    {
        return buffer; //First char is already '\0' by memset
    }

    int maxLength = textLength - position;
    if (length > maxLength) length = maxLength;
    if (length >= MAX_TEXT_BUFFER_LENGTH) length = MAX_TEXT_BUFFER_LENGTH - 1;

    // NOTE: Alternative: memcpy(buffer, text + position, length)

    for (int c = 0 ; c < length ; c++)
    {
        buffer[c] = text[position + c];
    }

    buffer[length] = '\0';

    return buffer;
}

// Replace text string
// REQUIRES: strlen(), strstr(), strncpy(), strcpy()
// WARNING: Allocated memory must be manually freed
char *TextReplace(const char *text, const char *replace, const char *by)
{
    // Sanity checks and initialization
    if (!text || !replace || !by) return NULL;

    char *result = NULL;

    char *insertPoint = NULL;   // Next insert point
    char *temp = NULL;          // Temp pointer
    int replaceLen = 0;         // Replace string length of (the string to remove)
    int byLen = 0;              // Replacement length (the string to replace by)
    int lastReplacePos = 0;     // Distance between replace and end of last replace
    int count = 0;              // Number of replacements

    replaceLen = TextLength(replace);
    if (replaceLen == 0) return NULL;  // Empty replace causes infinite loop during count

    byLen = TextLength(by);

    // Count the number of replacements needed
    insertPoint = (char *)text;
    for (count = 0; (temp = strstr(insertPoint, replace)); count++) insertPoint = temp + replaceLen;

    // Allocate returning string and point temp to it
    temp = result = (char *)RL_MALLOC(TextLength(text) + (byLen - replaceLen)*count + 1);

    if (!result) return NULL;   // Memory could not be allocated

    // First time through the loop, all the variable are set correctly from here on,
    //  - 'temp' points to the end of the result string
    //  - 'insertPoint' points to the next occurrence of replace in text
    //  - 'text' points to the remainder of text after "end of replace"
    while (count--)
    {
        insertPoint = strstr(text, replace);
        lastReplacePos = (int)(insertPoint - text);
        temp = strncpy(temp, text, lastReplacePos) + lastReplacePos;
        temp = strcpy(temp, by) + byLen;
        text += lastReplacePos + replaceLen; // Move to next "end of replace"
    }

    // Copy remaind text part after replacement to result (pointed by moving temp)
    strcpy(temp, text);

    return result;
}

// Insert text in a specific position, moves all text forward
// WARNING: Allocated memory must be manually freed
char *TextInsert(const char *text, const char *insert, int position)
{
    int textLen = TextLength(text);
    int insertLen = TextLength(insert);

    char *result = (char *)RL_MALLOC(textLen + insertLen + 1);

    for (int i = 0; i < position; i++) result[i] = text[i];
    for (int i = position; i < insertLen + position; i++) result[i] = insert[i];
    for (int i = (insertLen + position); i < (textLen + insertLen); i++) result[i] = text[i];

    result[textLen + insertLen] = '\0';     // Make sure text string is valid!

    return result;
}

// Join text strings with delimiter
// REQUIRES: memset(), memcpy()
char *TextJoin(char **textList, int count, const char *delimiter)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);
    char *textPtr = buffer;

    int totalLength = 0;
    int delimiterLen = TextLength(delimiter);

    for (int i = 0; i < count; i++)
    {
        int textLength = TextLength(textList[i]);

        // Make sure joined text could fit inside MAX_TEXT_BUFFER_LENGTH
        if ((totalLength + textLength) < MAX_TEXT_BUFFER_LENGTH)
        {
            memcpy(textPtr, textList[i], textLength);
            totalLength += textLength;
            textPtr += textLength;

            if ((delimiterLen > 0) && (i < (count - 1)))
            {
                memcpy(textPtr, delimiter, delimiterLen);
                totalLength += delimiterLen;
                textPtr += delimiterLen;
            }
        }
    }

    return buffer;
}

// Split string into multiple strings
// REQUIRES: memset()
char **TextSplit(const char *text, char delimiter, int *count)
{
    // NOTE: Current implementation returns a copy of the provided string with '\0' (string end delimiter)
    // inserted between strings defined by "delimiter" parameter. No memory is dynamically allocated,
    // all used memory is static... it has some limitations:
    //      1. Maximum number of possible split strings is set by MAX_TEXTSPLIT_COUNT
    //      2. Maximum size of text to split is MAX_TEXT_BUFFER_LENGTH

    static char *result[MAX_TEXTSPLIT_COUNT] = { NULL };
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    result[0] = buffer;
    int counter = 0;

    if (text != NULL)
    {
        counter = 1;

        // Count how many substrings we have on text and point to every one
        for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
        {
            buffer[i] = text[i];
            if (buffer[i] == '\0') break;
            else if (buffer[i] == delimiter)
            {
                buffer[i] = '\0';   // Set an end of string at this point
                result[counter] = buffer + i + 1;
                counter++;

                if (counter == MAX_TEXTSPLIT_COUNT) break;
            }
        }
    }

    *count = counter;
    return result;
}

// Append text at specific position and move cursor
// WARNING: It's up to the user to make sure appended text does not overflow the buffer!
// REQUIRES: strcpy()
void TextAppend(char *text, const char *append, int *position)
{
    strcpy(text + *position, append);
    *position += TextLength(append);
}

// Find first text occurrence within a string
// REQUIRES: strstr()
int TextFindIndex(const char *text, const char *find)
{
    int position = -1;

    char *ptr = strstr(text, find);

    if (ptr != NULL) position = (int)(ptr - text);

    return position;
}

// Get upper case version of provided string
// WARNING: Limited functionality, only basic characters set
// TODO: Support UTF-8 diacritics to upper-case, check codepoints
char *TextToUpper(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    if (text != NULL)
    {
        for (int i = 0; (i < MAX_TEXT_BUFFER_LENGTH - 1) && (text[i] != '\0'); i++)
        {
            if ((text[i] >= 'a') && (text[i] <= 'z')) buffer[i] = text[i] - 32;
            else buffer[i] = text[i];
        }
    }

    return buffer;
}

// Get lower case version of provided string
// WARNING: Limited functionality, only basic characters set
char *TextToLower(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    if (text != NULL)
    {
        for (int i = 0; (i < MAX_TEXT_BUFFER_LENGTH - 1) && (text[i] != '\0'); i++)
        {
            if ((text[i] >= 'A') && (text[i] <= 'Z')) buffer[i] = text[i] + 32;
            else buffer[i] = text[i];
        }
    }

    return buffer;
}

// Get Pascal case notation version of provided string
// WARNING: Limited functionality, only basic characters set
char *TextToPascal(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    if (text != NULL)
    {
        // Upper case first character
        if ((text[0] >= 'a') && (text[0] <= 'z')) buffer[0] = text[0] - 32;
        else buffer[0] = text[0];

        // Check for next separator to upper case another character
        for (int i = 1, j = 1; (i < MAX_TEXT_BUFFER_LENGTH - 1) && (text[j] != '\0'); i++, j++)
        {
            if (text[j] != '_') buffer[i] = text[j];
            else
            {
                j++;
                if ((text[j] >= 'a') && (text[j] <= 'z')) buffer[i] = text[j] - 32;
            }
        }
    }

    return buffer;
}

// Get snake case notation version of provided string
// WARNING: Limited functionality, only basic characters set
char *TextToSnake(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = {0};
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    if (text != NULL)
    {
        // Check for next separator to upper case another character
        for (int i = 0, j = 0; (i < MAX_TEXT_BUFFER_LENGTH - 1) && (text[j] != '\0'); i++, j++)
        {
            if ((text[j] >= 'A') && (text[j] <= 'Z'))
            {
                if (i >= 1)
                {
                    buffer[i] = '_';
                    i++;
                }
                buffer[i] = text[j] + 32;
            }
            else buffer[i] = text[j];
        }
    }

    return buffer;
}

// Get Camel case notation version of provided string
// WARNING: Limited functionality, only basic characters set
char *TextToCamel(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = {0};
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    if (text != NULL)
    {
        // Lower case first character
        if ((text[0] >= 'A') && (text[0] <= 'Z')) buffer[0] = text[0] + 32;
        else buffer[0] = text[0];

        // Check for next separator to upper case another character
        for (int i = 1, j = 1; (i < MAX_TEXT_BUFFER_LENGTH - 1) && (text[j] != '\0'); i++, j++)
        {
            if (text[j] != '_') buffer[i] = text[j];
            else
            {
                j++;
                if ((text[j] >= 'a') && (text[j] <= 'z')) buffer[i] = text[j] - 32;
            }
        }
    }

    return buffer;
}

// Encode text codepoint into UTF-8 text
// REQUIRES: memcpy()
// WARNING: Allocated memory must be manually freed
char *LoadUTF8(const int *codepoints, int length)
{
    // We allocate enough memory to fit all possible codepoints
    // NOTE: 5 bytes for every codepoint should be enough
    char *text = (char *)RL_CALLOC(length*5, 1);
    const char *utf8 = NULL;
    int size = 0;

    for (int i = 0, bytes = 0; i < length; i++)
    {
        utf8 = CodepointToUTF8(codepoints[i], &bytes);
        memcpy(text + size, utf8, bytes);
        size += bytes;
    }

    // Resize memory to text length + string NULL terminator
    void *ptr = RL_REALLOC(text, size + 1);

    if (ptr != NULL) text = (char *)ptr;

    return text;
}

// Unload UTF-8 text encoded from codepoints array
void UnloadUTF8(char *text)
{
    RL_FREE(text);
}

// Load all codepoints from a UTF-8 text string, codepoints count returned by parameter
int *LoadCodepoints(const char *text, int *count)
{
    int textLength = TextLength(text);

    int codepointSize = 0;
    int codepointCount = 0;

    // Allocate a big enough buffer to store as many codepoints as text bytes
    int *codepoints = (int *)RL_CALLOC(textLength, sizeof(int));

    for (int i = 0; i < textLength; codepointCount++)
    {
        codepoints[codepointCount] = GetCodepointNext(text + i, &codepointSize);
        i += codepointSize;
    }

    // Re-allocate buffer to the actual number of codepoints loaded
    codepoints = (int *)RL_REALLOC(codepoints, codepointCount*sizeof(int));

    *count = codepointCount;

    return codepoints;
}

// Unload codepoints data from memory
void UnloadCodepoints(int *codepoints)
{
    RL_FREE(codepoints);
}

// Get total number of characters(codepoints) in a UTF-8 encoded text, until '\0' is found
// NOTE: If an invalid UTF-8 sequence is encountered a '?'(0x3f) codepoint is counted instead
int GetCodepointCount(const char *text)
{
    unsigned int length = 0;
    const char *ptr = text;

    while (*ptr != '\0')
    {
        int next = 0;
        GetCodepointNext(ptr, &next);

        ptr += next;

        length++;
    }

    return length;
}

// Encode codepoint into utf8 text (char array length returned as parameter)
// NOTE: It uses a static array to store UTF-8 bytes
const char *CodepointToUTF8(int codepoint, int *utf8Size)
{
    static char utf8[6] = { 0 };
    memset(utf8, 0, 6); // Clear static array
    int size = 0;       // Byte size of codepoint

    if (codepoint <= 0x7f)
    {
        utf8[0] = (char)codepoint;
        size = 1;
    }
    else if (codepoint <= 0x7ff)
    {
        utf8[0] = (char)(((codepoint >> 6) & 0x1f) | 0xc0);
        utf8[1] = (char)((codepoint & 0x3f) | 0x80);
        size = 2;
    }
    else if (codepoint <= 0xffff)
    {
        utf8[0] = (char)(((codepoint >> 12) & 0x0f) | 0xe0);
        utf8[1] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[2] = (char)((codepoint & 0x3f) | 0x80);
        size = 3;
    }
    else if (codepoint <= 0x10ffff)
    {
        utf8[0] = (char)(((codepoint >> 18) & 0x07) | 0xf0);
        utf8[1] = (char)(((codepoint >> 12) & 0x3f) | 0x80);
        utf8[2] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[3] = (char)((codepoint & 0x3f) | 0x80);
        size = 4;
    }

    *utf8Size = size;

    return utf8;
}
#endif      // SUPPORT_TEXT_MANIPULATION

// Get next codepoint in a UTF-8 encoded text, scanning until '\0' is found
// When an invalid UTF-8 byte is encountered we exit as soon as possible and a '?'(0x3f) codepoint is returned
// Total number of bytes processed are returned as a parameter
// NOTE: The standard says U+FFFD should be returned in case of errors
// but that character is not supported by the default font in raylib
int GetCodepoint(const char *text, int *codepointSize)
{
/*
    UTF-8 specs from https://www.ietf.org/rfc/rfc3629.txt

    Char. number range  |        UTF-8 octet sequence
      (hexadecimal)    |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
    // NOTE: on decode errors we return as soon as possible

    int codepoint = 0x3f;   // Codepoint (defaults to '?')
    int octet = (unsigned char)(text[0]); // The first UTF8 octet
    *codepointSize = 1;

    if (octet <= 0x7f)
    {
        // Only one octet (ASCII range x00-7F)
        codepoint = text[0];
    }
    else if ((octet & 0xe0) == 0xc0)
    {
        // Two octets

        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        unsigned char octet1 = text[1];

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; } // Unexpected sequence

        if ((octet >= 0xc2) && (octet <= 0xdf))
        {
            codepoint = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            *codepointSize = 2;
        }
    }
    else if ((octet & 0xf0) == 0xe0)
    {
        // Three octets
        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *codepointSize = 3; return codepoint; } // Unexpected sequence

        // [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
        // [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        // [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
        // [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)

        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) { *codepointSize = 2; return codepoint; }

        if ((octet >= 0xe0) && (octet <= 0xef))
        {
            codepoint = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            *codepointSize = 3;
        }
    }
    else if ((octet & 0xf8) == 0xf0)
    {
        // Four octets
        if (octet > 0xf4) return codepoint;

        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';
        unsigned char octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; }  // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *codepointSize = 3; return codepoint; }  // Unexpected sequence

        octet3 = text[3];

        if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { *codepointSize = 4; return codepoint; }  // Unexpected sequence

        // [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
        // [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
        // [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail

        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) { *codepointSize = 2; return codepoint; } // Unexpected sequence

        if (octet >= 0xf0)
        {
            codepoint = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            *codepointSize = 4;
        }
    }

    if (codepoint > 0x10ffff) codepoint = 0x3f;     // Codepoints after U+10ffff are invalid

    return codepoint;
}

// Get next codepoint in a byte sequence and bytes processed
int GetCodepointNext(const char *text, int *codepointSize)
{
    const char *ptr = text;
    int codepoint = 0x3f;       // Codepoint (defaults to '?')
    *codepointSize = 1;

    // Get current codepoint and bytes processed
    if (0xf0 == (0xf8 & ptr[0]))
    {
        // 4 byte UTF-8 codepoint
        if (((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80) || ((ptr[3] & 0xC0) ^ 0x80)) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x07 & ptr[0]) << 18) | ((0x3f & ptr[1]) << 12) | ((0x3f & ptr[2]) << 6) | (0x3f & ptr[3]);
        *codepointSize = 4;
    }
    else if (0xe0 == (0xf0 & ptr[0]))
    {
        // 3 byte UTF-8 codepoint */
        if (((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80)) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x0f & ptr[0]) << 12) | ((0x3f & ptr[1]) << 6) | (0x3f & ptr[2]);
        *codepointSize = 3;
    }
    else if (0xc0 == (0xe0 & ptr[0]))
    {
        // 2 byte UTF-8 codepoint
        if ((ptr[1] & 0xC0) ^ 0x80) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x1f & ptr[0]) << 6) | (0x3f & ptr[1]);
        *codepointSize = 2;
    }
    else if (0x00 == (0x80 & ptr[0]))
    {
        // 1 byte UTF-8 codepoint
        codepoint = ptr[0];
        *codepointSize = 1;
    }

    return codepoint;
}

// Get previous codepoint in a byte sequence and bytes processed
int GetCodepointPrevious(const char *text, int *codepointSize)
{
    const char *ptr = text;
    int codepoint = 0x3f;       // Codepoint (defaults to '?')
    int cpSize = 0;
    *codepointSize = 0;

    // Move to previous codepoint
    do ptr--;
    while (((0x80 & ptr[0]) != 0) && ((0xc0 & ptr[0]) ==  0x80));

    codepoint = GetCodepointNext(ptr, &cpSize);

    if (codepoint != 0) *codepointSize = cpSize;

    return codepoint;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
#if defined(SUPPORT_FILEFORMAT_FNT) || defined(SUPPORT_FILEFORMAT_BDF)
// Read a line from memory
// REQUIRES: memcpy()
// NOTE: Returns the number of bytes read
static int GetLine(const char *origin, char *buffer, int maxLength)
{
    int count = 0;
    for (; count < maxLength - 1; count++) if (origin[count] == '\n') break;
    memcpy(buffer, origin, count);
    buffer[count] = '\0';
    return count;
}
#endif

#if defined(SUPPORT_FILEFORMAT_FNT)
// Load a BMFont file (AngelCode font file)
// REQUIRES: strstr(), sscanf(), strrchr(), memcpy()
static Font LoadBMFont(const char *fileName)
{
    #define MAX_BUFFER_SIZE       256
    #define MAX_FONT_IMAGE_PAGES    8

    Font font = { 0 };

    char buffer[MAX_BUFFER_SIZE] = { 0 };
    char *searchPoint = NULL;

    int fontSize = 0;
    int glyphCount = 0;

    int imWidth = 0;
    int imHeight = 0;
    int pageCount = 1;
    char imFileName[MAX_FONT_IMAGE_PAGES][129] = { 0 };

    int base = 0;       // Useless data
    int readBytes = 0;  // Data bytes read
    int readVars = 0;   // Variables filled by sscanf()

    char *fileText = LoadFileText(fileName);

    if (fileText == NULL) return font;

    char *fileTextPtr = fileText;

    // NOTE: We skip first line, it contains no useful information
    readBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
    fileTextPtr += (readBytes + 1);

    // Read line data
    readBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
    searchPoint = strstr(buffer, "lineHeight");
    readVars = sscanf(searchPoint, "lineHeight=%i base=%i scaleW=%i scaleH=%i pages=%i", &fontSize, &base, &imWidth, &imHeight, &pageCount);
    fileTextPtr += (readBytes + 1);

    if (readVars < 4) { UnloadFileText(fileText); return font; } // Some data not available, file malformed

    if (pageCount > MAX_FONT_IMAGE_PAGES)
    {
        TRACELOG(LOG_WARNING, "FONT: [%s] Font defines more pages than supported: %i/%i", fileName, pageCount, MAX_FONT_IMAGE_PAGES);
        pageCount = MAX_FONT_IMAGE_PAGES;
    }

    for (int i = 0; i < pageCount; i++)
    {
        readBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
        searchPoint = strstr(buffer, "file");
        readVars = sscanf(searchPoint, "file=\"%128[^\"]\"", imFileName[i]);
        fileTextPtr += (readBytes + 1);

        if (readVars < 1) { UnloadFileText(fileText); return font; } // No fileName read
    }

    readBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
    searchPoint = strstr(buffer, "count");
    readVars = sscanf(searchPoint, "count=%i", &glyphCount);
    fileTextPtr += (readBytes + 1);

    if (readVars < 1) { UnloadFileText(fileText); return font; } // No glyphCount read

    // Load all required images for further compose
    Image *imFonts = (Image *)RL_CALLOC(pageCount, sizeof(Image)); // Font atlases, multiple images

    for (int i = 0; i < pageCount; i++)
    {
        imFonts[i] = LoadImage(TextFormat("%s/%s", GetDirectoryPath(fileName), imFileName[i]));

        if (imFonts[i].format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
        {
            // Convert image to GRAYSCALE + ALPHA, using the mask as the alpha channel
            Image imFontAlpha = {
                .data = RL_CALLOC(imFonts[i].width*imFonts[i].height, 2),
                .width = imFonts[i].width,
                .height = imFonts[i].height,
                .mipmaps = 1,
                .format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA
            };

            for (int p = 0, pi = 0; p < (imFonts[i].width*imFonts[i].height*2); p += 2, pi++)
            {
                ((unsigned char *)(imFontAlpha.data))[p] = 0xff;
                ((unsigned char *)(imFontAlpha.data))[p + 1] = ((unsigned char *)imFonts[i].data)[pi];
            }

            UnloadImage(imFonts[i]);
            imFonts[i] = imFontAlpha;
        }
    }

    Image fullFont = imFonts[0];
    for (int i = 1; i < pageCount; i++) UnloadImage(imFonts[i]);

    // If multiple atlas, then merge atlas
    // NOTE: WARNING: This process could be really slow!
    if (pageCount > 1)
    {
        // Resize font atlas to draw additional images
        ImageResizeCanvas(&fullFont, imWidth, imHeight*pageCount, 0, 0, BLACK);

        for (int i = 1; i < pageCount; i++)
        {
            Rectangle srcRec = { 0.0f, 0.0f, (float)imWidth, (float)imHeight };
            Rectangle destRec = { 0.0f, (float)imHeight*(float)i, (float)imWidth, (float)imHeight };
            ImageDraw(&fullFont, imFonts[i], srcRec, destRec, WHITE);
        }
    }

    RL_FREE(imFonts);

    if (isGpuReady) font.texture = LoadTextureFromImage(fullFont);

    // Fill font characters info data
    font.baseSize = fontSize;
    font.glyphCount = glyphCount;
    font.glyphPadding = 0;
    font.glyphs = (GlyphInfo *)RL_MALLOC(glyphCount*sizeof(GlyphInfo));
    font.recs = (Rectangle *)RL_MALLOC(glyphCount*sizeof(Rectangle));

    int charId, charX, charY, charWidth, charHeight, charOffsetX, charOffsetY, charAdvanceX, pageID;

    for (int i = 0; i < glyphCount; i++)
    {
        readBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
        readVars = sscanf(buffer, "char id=%i x=%i y=%i width=%i height=%i xoffset=%i yoffset=%i xadvance=%i page=%i",
                       &charId, &charX, &charY, &charWidth, &charHeight, &charOffsetX, &charOffsetY, &charAdvanceX, &pageID);
        fileTextPtr += (readBytes + 1);

        if (readVars == 9)  // Make sure all char data has been properly read
        {
            // Get character rectangle in the font atlas texture
            font.recs[i] = (Rectangle){ (float)charX, (float)charY + (float)imHeight*pageID, (float)charWidth, (float)charHeight };

            // Save data properly in sprite font
            font.glyphs[i].value = charId;
            font.glyphs[i].offsetX = charOffsetX;
            font.glyphs[i].offsetY = charOffsetY;
            font.glyphs[i].advanceX = charAdvanceX;

            // Fill character image data from full font data
            font.glyphs[i].image = ImageFromImage(fullFont, font.recs[i]);
        }
        else
        {
            font.glyphs[i].image = GenImageColor((int)font.recs[i].width, (int)font.recs[i].height, BLACK);
            TRACELOG(LOG_WARNING, "FONT: [%s] Some characters data not correctly provided", fileName);
        }
    }

    UnloadImage(fullFont);
    UnloadFileText(fileText);

    if (isGpuReady && (font.texture.id == 0))
    {
        UnloadFont(font);
        font = GetFontDefault();
        TRACELOG(LOG_WARNING, "FONT: [%s] Failed to load texture, reverted to default font", fileName);
    }
    else TRACELOG(LOG_INFO, "FONT: [%s] Font loaded successfully (%i glyphs)", fileName, font.glyphCount);

    return font;
}

#endif

#if defined(SUPPORT_FILEFORMAT_BDF)

// Convert hexadecimal to decimal (single digit)
static unsigned char HexToInt(char hex)
{
    if ((hex >= '0') && (hex <= '9')) return hex - '0';
    else if ((hex >= 'a') && (hex <= 'f')) return hex - 'a' + 10;
    else if ((hex >= 'A') && (hex <= 'F')) return hex - 'A' + 10;
    else return 0;
}

// Load font data for further use
// NOTE: Requires BDF font memory data
static GlyphInfo *LoadFontDataBDF(const unsigned char *fileData, int dataSize, int *codepoints, int codepointCount, int *outFontSize)
{
    #define MAX_BUFFER_SIZE 256

    char buffer[MAX_BUFFER_SIZE] = { 0 };

    GlyphInfo *glyphs = NULL;
    bool genFontChars = false;

    int totalReadBytes = 0;         // Data bytes read (total)
    int readBytes = 0;              // Data bytes read (line)
    int readVars = 0;               // Variables filled by sscanf()

    const char *fileText = (const char *)fileData;
    const char *fileTextPtr = fileText;

    bool fontMalformed = false;     // Is the font malformed
    bool fontStarted = false;       // Has font started (STARTFONT)
    int fontBBw = 0;                // Font base character bounding box width
    int fontBBh = 0;                // Font base character bounding box height
    int fontBBxoff0 = 0;            // Font base character bounding box X0 offset
    int fontBByoff0 = 0;            // Font base character bounding box Y0 offset
    int fontAscent = 0;             // Font ascent

    bool charStarted = false;       // Has character started (STARTCHAR)
    bool charBitmapStarted = false; // Has bitmap data started (BITMAP)
    int charBitmapNextRow = 0;      // Y position for the next row of bitmap data
    int charEncoding = -1;          // The unicode value of the character (-1 if not set)
    int charBBw = 0;                // Character bounding box width
    int charBBh = 0;                // Character bounding box height
    int charBBxoff0 = 0;            // Character bounding box X0 offset
    int charBByoff0 = 0;            // Character bounding box Y0 offset
    int charDWidthX = 0;            // Character advance X
    int charDWidthY = 0;            // Character advance Y (unused)
    GlyphInfo *charGlyphInfo = NULL; // Pointer to output glyph info (NULL if not set)

    if (fileData == NULL) return glyphs;

    // In case no chars count provided, default to 95
    codepointCount = (codepointCount > 0)? codepointCount : 95;

    // Fill fontChars in case not provided externally
    // NOTE: By default we fill glyphCount consecutively, starting at 32 (Space)
    if (codepoints == NULL)
    {
        codepoints = (int *)RL_MALLOC(codepointCount*sizeof(int));
        for (int i = 0; i < codepointCount; i++) codepoints[i] = i + 32;
        genFontChars = true;
    }

    glyphs = (GlyphInfo *)RL_CALLOC(codepointCount, sizeof(GlyphInfo));

    while (totalReadBytes <= dataSize)
    {
        readBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
        totalReadBytes += (readBytes + 1);
        fileTextPtr += (readBytes + 1);

        // Line: COMMENT
        if (strstr(buffer, "COMMENT") != NULL) continue; // Ignore line

        if (charStarted)
        {
            // Line: ENDCHAR
            if (strstr(buffer, "ENDCHAR") != NULL)
            {
                charStarted = false;
                continue;
            }

            if (charBitmapStarted)
            {
                if (charGlyphInfo != NULL)
                {
                    int pixelY = charBitmapNextRow++;

                    if (pixelY >= charGlyphInfo->image.height) break;

                    for (int x = 0; x < readBytes; x++)
                    {
                        unsigned char byte = HexToInt(buffer[x]);

                        for (int bitX = 0; bitX < 4; bitX++)
                        {
                            int pixelX = ((x*4) + bitX);

                            if (pixelX >= charGlyphInfo->image.width) break;

                            if ((byte & (8 >> bitX)) > 0) ((unsigned char *)charGlyphInfo->image.data)[(pixelY*charGlyphInfo->image.width) + pixelX] = 255;
                        }
                    }
                }
                continue;
            }

            // Line: ENCODING
            if (strstr(buffer, "ENCODING") != NULL)
            {
                readVars = sscanf(buffer, "ENCODING %i", &charEncoding);
                continue;
            }

            // Line: BBX
            if (strstr(buffer, "BBX") != NULL)
            {
                readVars = sscanf(buffer, "BBX %i %i %i %i", &charBBw, &charBBh, &charBBxoff0, &charBByoff0);
                continue;
            }

            // Line: DWIDTH
            if (strstr(buffer, "DWIDTH") != NULL)
            {
                readVars = sscanf(buffer, "DWIDTH %i %i", &charDWidthX, &charDWidthY);
                continue;
            }

            // Line: BITMAP
            if (strstr(buffer, "BITMAP") != NULL)
            {
                // Search for glyph index in codepoints
                charGlyphInfo = NULL;

                for (int codepointIndex = 0; codepointIndex < codepointCount; codepointIndex++)
                {
                    if (codepoints[codepointIndex] == charEncoding)
                    {
                        charGlyphInfo = &glyphs[codepointIndex];
                        break;
                    }
                }

                // Init glyph info
                if (charGlyphInfo != NULL)
                {
                    charGlyphInfo->value = charEncoding;
                    charGlyphInfo->offsetX = charBBxoff0 + fontBByoff0;
                    charGlyphInfo->offsetY = fontBBh - (charBBh + charBByoff0 + fontBByoff0 + fontAscent);
                    charGlyphInfo->advanceX = charDWidthX;

                    charGlyphInfo->image.data = RL_CALLOC(charBBw*charBBh, 1);
                    charGlyphInfo->image.width = charBBw;
                    charGlyphInfo->image.height = charBBh;
                    charGlyphInfo->image.mipmaps = 1;
                    charGlyphInfo->image.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
                }

                charBitmapStarted = true;
                charBitmapNextRow = 0;

                continue;
            }
        }
        else if (fontStarted)
        {
            // Line: ENDFONT
            if (strstr(buffer, "ENDFONT") != NULL)
            {
                fontStarted = false;
                break;
            }

            // Line: SIZE
            if (strstr(buffer, "SIZE") != NULL)
            {
                if (outFontSize != NULL) readVars = sscanf(buffer, "SIZE %i", outFontSize);
                continue;
            }

            // PIXEL_SIZE
            if (strstr(buffer, "PIXEL_SIZE") != NULL)
            {
                if (outFontSize != NULL) readVars = sscanf(buffer, "PIXEL_SIZE %i", outFontSize);
                continue;
            }

            // FONTBOUNDINGBOX
            if (strstr(buffer, "FONTBOUNDINGBOX") != NULL)
            {
                readVars = sscanf(buffer, "FONTBOUNDINGBOX %i %i %i %i", &fontBBw, &fontBBh, &fontBBxoff0, &fontBByoff0);
                continue;
            }

            // FONT_ASCENT
            if (strstr(buffer, "FONT_ASCENT") != NULL)
            {
                readVars = sscanf(buffer, "FONT_ASCENT %i", &fontAscent);
                continue;
            }

            // STARTCHAR
            if (strstr(buffer, "STARTCHAR") != NULL)
            {
                charStarted = true;
                charEncoding = -1;
                charGlyphInfo = NULL;
                charBBw = 0;
                charBBh = 0;
                charBBxoff0 = 0;
                charBByoff0 = 0;
                charDWidthX = 0;
                charDWidthY = 0;
                charGlyphInfo = NULL;
                charBitmapStarted = false;
                charBitmapNextRow = 0;
                continue;
            }
        }
        else
        {
            // STARTFONT
            if (strstr(buffer, "STARTFONT") != NULL)
            {
                if (fontStarted)
                {
                    fontMalformed = true;
                    break;
                }
                else
                {
                    fontStarted = true;
                    continue;
                }
            }
        }
    }

    if (genFontChars) RL_FREE(codepoints);

    if (fontMalformed)
    {
        RL_FREE(glyphs);
        glyphs = NULL;
    }

    return glyphs;
}
#endif      // SUPPORT_FILEFORMAT_BDF

#endif      // SUPPORT_MODULE_RTEXT
