/**********************************************************************************************
*
*   rtext - Basic functions to load fonts and draw text
*
*   CONFIGURATION:
*
*   #define SUPPORT_FILEFORMAT_FNT
*   #define SUPPORT_FILEFORMAT_TTF
*       Selected desired fileformats to be supported for loading. Some of those formats are
*       supported by default, to remove support, just comment unrequired #define in this module
*
*   #define SUPPORT_DEFAULT_FONT
*       Load default raylib font on initialization to be used by DrawText() and MeasureText().
*       If no default font loaded, DrawTextEx() and MeasureTextEx() are required.
*
*   #define TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH
*       TextSplit() function static buffer max size
*
*   #define MAX_TEXTSPLIT_COUNT
*       TextSplit() function static substrings pointers array (pointing to static buffer)
*
*
*   DEPENDENCIES:
*       stb_truetype  - Load TTF file and rasterize characters data
*       stb_rect_pack - Rectangles packing algorythms, required for font atlas generation
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2021 Ramon Santamaria (@raysan5)
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

#include "utils.h"          // Required for: LoadFileText()
#include "rlgl.h"           // OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2 -> Only DrawTextPro()

#include <stdlib.h>         // Required for: malloc(), free()
#include <stdio.h>          // Required for: vsprintf()
#include <string.h>         // Required for: strcmp(), strstr(), strcpy(), strncpy() [Used in TextReplace()], sscanf() [Used in LoadBMFont()]
#include <stdarg.h>         // Required for: va_list, va_start(), vsprintf(), va_end() [Used in TextFormat()]
#include <ctype.h>          // Requried for: toupper(), tolower() [Used in TextToUpper(), TextToLower()]

#if defined(SUPPORT_FILEFORMAT_TTF)
    #define STB_RECT_PACK_IMPLEMENTATION
    #include "external/stb_rect_pack.h"     // Required for: ttf font rectangles packaging

    #define STBTT_STATIC
    #define STB_TRUETYPE_IMPLEMENTATION
    #include "external/stb_truetype.h"      // Required for: ttf font data reading
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
// ...

//----------------------------------------------------------------------------------
// Global variables
//----------------------------------------------------------------------------------
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
static Font LoadBMFont(const char *fileName);     // Load a BMFont file (AngelCode font file)
#endif

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
        .data = calloc(128*128, 2),  // 2 bytes per pixel (gray + alpha)
        .width = 128,
        .height = 128,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,
        .mipmaps = 1
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
            else ((unsigned short *)imFont.data)[i + j] = 0x00ff;
        }

        counter++;
    }

    defaultFont.texture = LoadTextureFromImage(imFont);

    // Reconstruct charSet using charsWidth[], charsHeight, charsDivisor, glyphCount
    //------------------------------------------------------------------------------

    // Allocate space for our characters info data
    // NOTE: This memory should be freed at end! --> CloseWindow()
    defaultFont.glyphs = (GlyphInfo *)RL_MALLOC(defaultFont.glyphCount*sizeof(GlyphInfo));
    defaultFont.recs = (Rectangle *)RL_MALLOC(defaultFont.glyphCount*sizeof(Rectangle));

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
    UnloadTexture(defaultFont.texture);
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
    if (IsFileExtension(fileName, ".ttf;.otf")) font = LoadFontEx(fileName, FONT_TTF_DEFAULT_SIZE, NULL, FONT_TTF_DEFAULT_NUMCHARS);
    else
#endif
#if defined(SUPPORT_FILEFORMAT_FNT)
    if (IsFileExtension(fileName, ".fnt")) font = LoadBMFont(fileName);
    else
#endif
    {
        Image image = LoadImage(fileName);
        if (image.data != NULL) font = LoadFontFromImage(image, MAGENTA, FONT_TTF_DEFAULT_FIRST_CHAR);
        UnloadImage(image);
    }

    if (font.texture.id == 0)
    {
        TRACELOG(LOG_WARNING, "FONT: [%s] Failed to load font texture -> Using default font", fileName);
        font = GetFontDefault();
    }
    else SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);    // By default we set point filter (best performance)

    return font;
}

// Load Font from TTF font file with generation parameters
// NOTE: You can pass an array with desired characters, those characters should be available in the font
// if array is NULL, default char set is selected 32..126
Font LoadFontEx(const char *fileName, int fontSize, int *fontChars, int glyphCount)
{
    Font font = { 0 };

    // Loading file to memory
    unsigned int fileSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &fileSize);

    if (fileData != NULL)
    {
        // Loading font from memory data
        font = LoadFontFromMemory(GetFileExtension(fileName), fileData, fileSize, fontSize, fontChars, glyphCount);

        RL_FREE(fileData);
    }
    else font = GetFontDefault();

    return font;
}

// Load an Image font file (XNA style)
Font LoadFontFromImage(Image image, Color key, int firstChar)
{
#ifndef MAX_GLYPHS_FROM_IMAGE
    #define MAX_GLYPHS_FROM_IMAGE   256     // Maximum number of glyphs supported on image scan
#endif

    #define COLOR_EQUAL(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))

    int charSpacing = 0;
    int lineSpacing = 0;

    int x = 0;
    int y = 0;

    // We allocate a temporal arrays for chars data measures,
    // once we get the actual number of chars, we copy data to a sized arrays
    int tempCharValues[MAX_GLYPHS_FROM_IMAGE];
    Rectangle tempCharRecs[MAX_GLYPHS_FROM_IMAGE];

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
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };

    // Create font with all data parsed from image
    Font font = { 0 };

    font.texture = LoadTextureFromImage(fontClear); // Convert processed image to OpenGL texture
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
Font LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *fontChars, int glyphCount)
{
    Font font = { 0 };

    char fileExtLower[16] = { 0 };
    strcpy(fileExtLower, TextToLower(fileType));

#if defined(SUPPORT_FILEFORMAT_TTF)
    if (TextIsEqual(fileExtLower, ".ttf") ||
        TextIsEqual(fileExtLower, ".otf"))
    {
        font.baseSize = fontSize;
        font.glyphCount = (glyphCount > 0)? glyphCount : 95;
        font.glyphPadding = 0;
        font.glyphs = LoadFontData(fileData, dataSize, font.baseSize, fontChars, font.glyphCount, FONT_DEFAULT);

        if (font.glyphs != NULL)
        {
            font.glyphPadding = FONT_TTF_DEFAULT_CHARS_PADDING;

            Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, font.glyphCount, font.baseSize, font.glyphPadding, 0);
            font.texture = LoadTextureFromImage(atlas);

            // Update glyphs[i].image to use alpha, required to be used on ImageDrawText()
            for (int i = 0; i < font.glyphCount; i++)
            {
                UnloadImage(font.glyphs[i].image);
                font.glyphs[i].image = ImageFromImage(atlas, font.recs[i]);
            }

            UnloadImage(atlas);
            
            // TRACELOG(LOG_INFO, "FONT: Font loaded successfully (%i glyphs)", font.glyphCount);
        }
        else font = GetFontDefault();
    }
#else
    font = GetFontDefault();
#endif

    return font;
}

// Load font data for further use
// NOTE: Requires TTF font memory data and can generate SDF data
GlyphInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *fontChars, int glyphCount, int type)
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
        int genFontChars = false;
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
            glyphCount = (glyphCount > 0)? glyphCount : 95;

            // Fill fontChars in case not provided externally
            // NOTE: By default we fill glyphCount consecutevely, starting at 32 (Space)

            if (fontChars == NULL)
            {
                fontChars = (int *)RL_MALLOC(glyphCount*sizeof(int));
                for (int i = 0; i < glyphCount; i++) fontChars[i] = i + 32;
                genFontChars = true;
            }

            chars = (GlyphInfo *)RL_MALLOC(glyphCount*sizeof(GlyphInfo));

            // NOTE: Using simple packaging, one char after another
            for (int i = 0; i < glyphCount; i++)
            {
                int chw = 0, chh = 0;   // Character width and height (on generation)
                int ch = fontChars[i];  // Character value to get info for
                chars[i].value = ch;

                //  Render a unicode codepoint to a bitmap
                //      stbtt_GetCodepointBitmap()           -- allocates and returns a bitmap
                //      stbtt_GetCodepointBitmapBox()        -- how big the bitmap must be
                //      stbtt_MakeCodepointBitmap()          -- renders into bitmap you provide

                if (type != FONT_SDF) chars[i].image.data = stbtt_GetCodepointBitmap(&fontInfo, scaleFactor, scaleFactor, ch, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY);
                else if (ch != 32) chars[i].image.data = stbtt_GetCodepointSDF(&fontInfo, scaleFactor, ch, FONT_SDF_CHAR_PADDING, FONT_SDF_ON_EDGE_VALUE, FONT_SDF_PIXEL_DIST_SCALE, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY);
                else chars[i].image.data = NULL;

                stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advanceX, NULL);
                chars[i].advanceX = (int)((float)chars[i].advanceX*scaleFactor);

                // Load characters images
                chars[i].image.width = chw;
                chars[i].image.height = chh;
                chars[i].image.mipmaps = 1;
                chars[i].image.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;

                chars[i].offsetY += (int)((float)ascent*scaleFactor);

                // NOTE: We create an empty image for space character, it could be further required for atlas packing
                if (ch == 32)
                {
                    Image imSpace = {
                        .data = calloc(chars[i].advanceX*fontSize, 2),
                        .width = chars[i].advanceX,
                        .height = fontSize,
                        .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
                        .mipmaps = 1
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

                // Get bounding box for character (may be offset to account for chars that dip above or below the line)
                /*
                int chX1, chY1, chX2, chY2;
                stbtt_GetCodepointBitmapBox(&fontInfo, ch, scaleFactor, scaleFactor, &chX1, &chY1, &chX2, &chY2);

                TRACELOGD("FONT: Character box measures: %i, %i, %i, %i", chX1, chY1, chX2 - chX1, chY2 - chY1);
                TRACELOGD("FONT: Character offsetY: %i", (int)((float)ascent*scaleFactor) + chY1);
                */
            }
        }
        else TRACELOG(LOG_WARNING, "FONT: Failed to process TTF font data");

        if (genFontChars) RL_FREE(fontChars);
    }
#endif

    return chars;
}

// Generate image font atlas using chars info
// NOTE: Packing method: 0-Default, 1-Skyline
#if defined(SUPPORT_FILEFORMAT_TTF)
Image GenImageFontAtlas(const GlyphInfo *chars, Rectangle **charRecs, int glyphCount, int fontSize, int padding, int packMethod)
{
    Image atlas = { 0 };

    if (chars == NULL)
    {
        TraceLog(LOG_WARNING, "FONT: Provided chars info not valid, returning empty image atlas");
        return atlas;
    }

    *charRecs = NULL;

    // In case no chars count provided we suppose default of 95
    glyphCount = (glyphCount > 0)? glyphCount : 95;

    // NOTE: Rectangles memory is loaded here!
    Rectangle *recs = (Rectangle *)RL_MALLOC(glyphCount*sizeof(Rectangle));

    // Calculate image size based on required pixel area
    // NOTE 1: Image is forced to be squared and POT... very conservative!
    // NOTE 2: SDF font characters already contain an internal padding,
    // so image size would result bigger than default font type
    float requiredArea = 0;
    for (int i = 0; i < glyphCount; i++) requiredArea += ((chars[i].image.width + 2*padding)*(chars[i].image.height + 2*padding));
    float guessSize = sqrtf(requiredArea)*1.3f;
    int imageSize = (int)powf(2, ceilf(logf((float)guessSize)/logf(2)));  // Calculate next POT

    atlas.width = imageSize;   // Atlas bitmap width
    atlas.height = imageSize;  // Atlas bitmap height
    atlas.data = (unsigned char *)RL_CALLOC(1, atlas.width*atlas.height);      // Create a bitmap to store characters (8 bpp)
    atlas.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    atlas.mipmaps = 1;

    // DEBUG: We can see padding in the generated image setting a gray background...
    //for (int i = 0; i < atlas.width*atlas.height; i++) ((unsigned char *)atlas.data)[i] = 100;

    if (packMethod == 0)   // Use basic packing algorythm
    {
        int offsetX = padding;
        int offsetY = padding;

        // NOTE: Using simple packaging, one char after another
        for (int i = 0; i < glyphCount; i++)
        {
            // Copy pixel data from fc.data to atlas
            for (int y = 0; y < chars[i].image.height; y++)
            {
                for (int x = 0; x < chars[i].image.width; x++)
                {
                    ((unsigned char *)atlas.data)[(offsetY + y)*atlas.width + (offsetX + x)] = ((unsigned char *)chars[i].image.data)[y*chars[i].image.width + x];
                }
            }

            // Fill chars rectangles in atlas info
            recs[i].x = (float)offsetX;
            recs[i].y = (float)offsetY;
            recs[i].width = (float)chars[i].image.width;
            recs[i].height = (float)chars[i].image.height;

            // Move atlas position X for next character drawing
            offsetX += (chars[i].image.width + 2*padding);

            if (offsetX >= (atlas.width - chars[i].image.width - 2*padding))
            {
                offsetX = padding;

                // NOTE: Be careful on offsetY for SDF fonts, by default SDF
                // use an internal padding of 4 pixels, it means char rectangle
                // height is bigger than fontSize, it could be up to (fontSize + 8)
                offsetY += (fontSize + 2*padding);

                if (offsetY > (atlas.height - fontSize - padding)) break;
            }
        }
    }
    else if (packMethod == 1)  // Use Skyline rect packing algorythm (stb_pack_rect)
    {
        stbrp_context *context = (stbrp_context *)RL_MALLOC(sizeof(*context));
        stbrp_node *nodes = (stbrp_node *)RL_MALLOC(glyphCount*sizeof(*nodes));

        stbrp_init_target(context, atlas.width, atlas.height, nodes, glyphCount);
        stbrp_rect *rects = (stbrp_rect *)RL_MALLOC(glyphCount*sizeof(stbrp_rect));

        // Fill rectangles for packaging
        for (int i = 0; i < glyphCount; i++)
        {
            rects[i].id = i;
            rects[i].w = chars[i].image.width + 2*padding;
            rects[i].h = chars[i].image.height + 2*padding;
        }

        // Package rectangles into atlas
        stbrp_pack_rects(context, rects, glyphCount);

        for (int i = 0; i < glyphCount; i++)
        {
            // It return char rectangles in atlas
            recs[i].x = rects[i].x + (float)padding;
            recs[i].y = rects[i].y + (float)padding;
            recs[i].width = (float)chars[i].image.width;
            recs[i].height = (float)chars[i].image.height;

            if (rects[i].was_packed)
            {
                // Copy pixel data from fc.data to atlas
                for (int y = 0; y < chars[i].image.height; y++)
                {
                    for (int x = 0; x < chars[i].image.width; x++)
                    {
                        ((unsigned char *)atlas.data)[(rects[i].y + padding + y)*atlas.width + (rects[i].x + padding + x)] = ((unsigned char *)chars[i].image.data)[y*chars[i].image.width + x];
                    }
                }
            }
            else TRACELOG(LOG_WARNING, "FONT: Failed to package character (%i)", i);
        }

        RL_FREE(rects);
        RL_FREE(nodes);
        RL_FREE(context);
    }

    // TODO: Crop image if required for smaller size

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

    *charRecs = recs;

    return atlas;
}
#endif

// Unload font glyphs info data (RAM)
void UnloadFontData(GlyphInfo *glyphs, int glyphCount)
{
    for (int i = 0; i < glyphCount; i++) UnloadImage(glyphs[i].image);

    RL_FREE(glyphs);
}

// Unload Font from GPU memory (VRAM)
void UnloadFont(Font font)
{
    // NOTE: Make sure font is not default font (fallback)
    if (font.texture.id != GetFontDefault().texture.id)
    {
        UnloadFontData(font.glyphs, font.glyphCount);
        UnloadTexture(font.texture);
        RL_FREE(font.recs);

        TRACELOGD("FONT: Unloaded font data from RAM and VRAM");
    }
}

// Draw current FPS
// NOTE: Uses default font
void DrawFPS(int posX, int posY)
{
    Color color = LIME; // good fps
    int fps = GetFPS();

    if (fps < 30 && fps >= 15) color = ORANGE;  // warning FPS
    else if (fps < 15) color = RED;    // bad FPS

    DrawText(TextFormat("%2i FPS", GetFPS()), posX, posY, 20, color);
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

    int textOffsetY = 0;            // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font.baseSize;         // Character quad scaling factor

    for (int i = 0; i < size;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
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

// Measure string width for default font
int MeasureText(const char *text, int fontSize)
{
    Vector2 vec = { 0.0f, 0.0f };

    // Check if default font has been loaded
    if (GetFontDefault().texture.id != 0)
    {
        int defaultFontSize = 10;   // Default Font chars height in pixel
        if (fontSize < defaultFontSize) fontSize = defaultFontSize;
        int spacing = fontSize/defaultFontSize;

        vec = MeasureTextEx(GetFontDefault(), text, (float)fontSize, (float)spacing);
    }

    return (int)vec.x;
}

// Measure string size for Font
Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing)
{
    int size = TextLength(text);    // Get size in bytes of text
    int tempByteCounter = 0;        // Used to count longer text line num chars
    int byteCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f;     // Used to count longer text line width

    float textHeight = (float)font.baseSize;
    float scaleFactor = fontSize/(float)font.baseSize;

    int letter = 0;                 // Current character
    int index = 0;                  // Index position in sprite font

    for (int i = 0; i < size; i++)
    {
        byteCounter++;

        int next = 0;
        letter = GetCodepoint(&text[i], &next);
        index = GetGlyphIndex(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f) next = 1;
        i += next - 1;

        if (letter != '\n')
        {
            if (font.glyphs[index].advanceX != 0) textWidth += font.glyphs[index].advanceX;
            else textWidth += (font.recs[index].width + font.glyphs[index].offsetX);
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            byteCounter = 0;
            textWidth = 0;
            textHeight += ((float)font.baseSize*1.5f); // NOTE: Fixed line spacing of 1.5 lines
        }

        if (tempByteCounter < byteCounter) tempByteCounter = byteCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    Vector2 vec = { 0 };
    vec.x = tempTextWidth*scaleFactor + (float)((tempByteCounter - 1)*spacing); // Adds chars spacing to measure
    vec.y = textHeight*scaleFactor;

    return vec;
}

// Get index position for a unicode character on font
// NOTE: If codepoint is not found in the font it fallbacks to '?'
int GetGlyphIndex(Font font, int codepoint)
{
#ifndef GLYPH_NOTFOUND_CHAR_FALLBACK
    #define GLYPH_NOTFOUND_CHAR_FALLBACK     63      // Character used if requested codepoint is not found: '?'
#endif

// Support charsets with any characters order
#define SUPPORT_UNORDERED_CHARSET
#if defined(SUPPORT_UNORDERED_CHARSET)
    int index = GLYPH_NOTFOUND_CHAR_FALLBACK;

    for (int i = 0; i < font.glyphCount; i++)
    {
        if (font.glyphs[i].value == codepoint)
        {
            index = i;
            break;
        }
    }

    return index;
#else
    return (codepoint - 32);
#endif
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
    unsigned int length = 0; //strlen(text)

    if (text != NULL)
    {
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
    vsnprintf(currentBuffer, MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

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

    for (int i = 0; ((text[i] >= '0') && (text[i] <= '9')); ++i) value = value*10 + (int)(text[i] - '0');

    return value*sign;
}

#if defined(SUPPORT_TEXT_MANIPULATION)
// Copy one string to another, returns bytes copied
int TextCopy(char *dst, const char *src)
{
    int bytes = 0;

    if (dst != NULL)
    {
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

    if (strcmp(text1, text2) == 0) result = true;

    return result;
}

// Get a piece of a text string
const char *TextSubtext(const char *text, int position, int length)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    int textLength = TextLength(text);

    if (position >= textLength)
    {
        position = textLength - 1;
        length = 0;
    }

    if (length >= textLength) length = textLength;

    for (int c = 0 ; c < length ; c++)
    {
        *(buffer + c) = *(text + position);
        text++;
    }

    *(buffer + length) = '\0';

    return buffer;
}

// Replace text string
// REQUIRES: strstr(), strncpy(), strcpy()
// WARNING: Returned buffer must be freed by the user (if return != NULL)
char *TextReplace(char *text, const char *replace, const char *by)
{
    // Sanity checks and initialization
    if (!text || !replace || !by) return NULL;

    char *result;

    char *insertPoint;      // Next insert point
    char *temp;             // Temp pointer
    int replaceLen;         // Replace string length of (the string to remove)
    int byLen;              // Replacement length (the string to replace replace by)
    int lastReplacePos;     // Distance between replace and end of last replace
    int count;              // Number of replacements

    replaceLen = TextLength(replace);
    if (replaceLen == 0) return NULL;  // Empty replace causes infinite loop during count

    byLen = TextLength(by);

    // Count the number of replacements needed
    insertPoint = text;
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
// WARNING: Allocated memory should be manually freed
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
const char *TextJoin(const char **textList, int count, const char *delimiter)
{
    static char text[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(text, 0, MAX_TEXT_BUFFER_LENGTH);
    char *textPtr = text;

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

    return text;
}

// Split string into multiple strings
// REQUIRES: memset()
const char **TextSplit(const char *text, char delimiter, int *count)
{
    // NOTE: Current implementation returns a copy of the provided string with '\0' (string end delimiter)
    // inserted between strings defined by "delimiter" parameter. No memory is dynamically allocated,
    // all used memory is static... it has some limitations:
    //      1. Maximum number of possible split strings is set by MAX_TEXTSPLIT_COUNT
    //      2. Maximum size of text to split is MAX_TEXT_BUFFER_LENGTH

    static const char *result[MAX_TEXTSPLIT_COUNT] = { NULL };
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

// Append text at specific position and move cursor!
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
// REQUIRES: toupper()
const char *TextToUpper(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
    {
        if (text[i] != '\0')
        {
            buffer[i] = (char)toupper(text[i]);
            //if ((text[i] >= 'a') && (text[i] <= 'z')) buffer[i] = text[i] - 32;

            // TODO: Support UTF-8 diacritics!
            //if ((text[i] >= 'à') && (text[i] <= 'ý')) buffer[i] = text[i] - 32;
        }
        else { buffer[i] = '\0'; break; }
    }

    return buffer;
}

// Get lower case version of provided string
// REQUIRES: tolower()
const char *TextToLower(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
    {
        if (text[i] != '\0')
        {
            buffer[i] = (char)tolower(text[i]);
            //if ((text[i] >= 'A') && (text[i] <= 'Z')) buffer[i] = text[i] + 32;
        }
        else { buffer[i] = '\0'; break; }
    }

    return buffer;
}

// Get Pascal case notation version of provided string
// REQUIRES: toupper()
const char *TextToPascal(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    buffer[0] = (char)toupper(text[0]);

    for (int i = 1, j = 1; i < MAX_TEXT_BUFFER_LENGTH; i++, j++)
    {
        if (text[j] != '\0')
        {
            if (text[j] != '_') buffer[i] = text[j];
            else
            {
                j++;
                buffer[i] = (char)toupper(text[j]);
            }
        }
        else { buffer[i] = '\0'; break; }
    }

    return buffer;
}

// Encode text codepoint into UTF-8 text
// REQUIRES: memcpy()
// WARNING: Allocated memory should be manually freed
char *TextCodepointsToUTF8(int *codepoints, int length)
{
    // We allocate enough memory fo fit all possible codepoints
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

// Encode codepoint into utf8 text (char array length returned as parameter)
// NOTE: It uses a static array to store UTF-8 bytes
RLAPI const char *CodepointToUTF8(int codepoint, int *byteSize)
{
    static char utf8[6] = { 0 };
    int size = 0;   // Byte size of codepoint

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

    *byteSize = size;

    return utf8;
}

// Load all codepoints from a UTF-8 text string, codepoints count returned by parameter
int *LoadCodepoints(const char *text, int *count)
{
    int textLength = TextLength(text);

    int bytesProcessed = 0;
    int codepointCount = 0;

    // Allocate a big enough buffer to store as many codepoints as text bytes
    int *codepoints = RL_CALLOC(textLength, sizeof(int));

    for (int i = 0; i < textLength; codepointCount++)
    {
        codepoints[codepointCount] = GetCodepoint(text + i, &bytesProcessed);
        i += bytesProcessed;
    }

    // Re-allocate buffer to the actual number of codepoints loaded
    void *temp = RL_REALLOC(codepoints, codepointCount*sizeof(int));
    if (temp != NULL) codepoints = temp;

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
    char *ptr = (char *)&text[0];

    while (*ptr != '\0')
    {
        int next = 0;
        int letter = GetCodepoint(ptr, &next);

        if (letter == 0x3f) ptr += 1;
        else ptr += next;

        length++;
    }

    return length;
}
#endif      // SUPPORT_TEXT_MANIPULATION

// Get next codepoint in a UTF-8 encoded text, scanning until '\0' is found
// When a invalid UTF-8 byte is encountered we exit as soon as possible and a '?'(0x3f) codepoint is returned
// Total number of bytes processed are returned as a parameter
// NOTE: the standard says U+FFFD should be returned in case of errors
// but that character is not supported by the default font in raylib
// TODO: Optimize this code for speed!!
int GetCodepoint(const char *text, int *bytesProcessed)
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

    int code = 0x3f;   // Codepoint (defaults to '?')
    int octet = (unsigned char)(text[0]); // The first UTF8 octet
    *bytesProcessed = 1;

    if (octet <= 0x7f)
    {
        // Only one octet (ASCII range x00-7F)
        code = text[0];
    }
    else if ((octet & 0xe0) == 0xc0)
    {
        // Two octets

        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        unsigned char octet1 = text[1];

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } // Unexpected sequence

        if ((octet >= 0xc2) && (octet <= 0xdf))
        {
            code = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            *bytesProcessed = 2;
        }
    }
    else if ((octet & 0xf0) == 0xe0)
    {
        // Three octets
        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; } // Unexpected sequence

        // [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
        // [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        // [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
        // [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)

        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) { *bytesProcessed = 2; return code; }

        if ((octet >= 0xe0) && (0 <= 0xef))
        {
            code = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            *bytesProcessed = 3;
        }
    }
    else if ((octet & 0xf8) == 0xf0)
    {
        // Four octets
        if (octet > 0xf4) return code;

        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';
        unsigned char octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; }  // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; }  // Unexpected sequence

        octet3 = text[3];

        if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { *bytesProcessed = 4; return code; }  // Unexpected sequence

        // [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
        // [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
        // [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail

        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) { *bytesProcessed = 2; return code; } // Unexpected sequence

        if (octet >= 0xf0)
        {
            code = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            *bytesProcessed = 4;
        }
    }

    if (code > 0x10ffff) code = 0x3f;     // Codepoints after U+10ffff are invalid

    return code;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
#if defined(SUPPORT_FILEFORMAT_FNT)

// Read a line from memory
// REQUIRES: memcpy()
// NOTE: Returns the number of bytes read
static int GetLine(const char *origin, char *buffer, int maxLength)
{
    int count = 0;
    for (; count < maxLength; count++) if (origin[count] == '\n') break;
    memcpy(buffer, origin, count);
    return count;
}

// Load a BMFont file (AngelCode font file)
// REQUIRES: strstr(), sscanf(), strrchr(), memcpy()
static Font LoadBMFont(const char *fileName)
{
    #define MAX_BUFFER_SIZE     256

    Font font = { 0 };

    char buffer[MAX_BUFFER_SIZE] = { 0 };
    char *searchPoint = NULL;

    int fontSize = 0;
    int glyphCount = 0;

    int imWidth = 0;
    int imHeight = 0;
    char imFileName[129];

    int base = 0;   // Useless data

    char *fileText = LoadFileText(fileName);

    if (fileText == NULL) return font;

    char *fileTextPtr = fileText;

    // NOTE: We skip first line, it contains no useful information
    int lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
    fileTextPtr += (lineBytes + 1);

    // Read line data
    lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
    searchPoint = strstr(buffer, "lineHeight");
    sscanf(searchPoint, "lineHeight=%i base=%i scaleW=%i scaleH=%i", &fontSize, &base, &imWidth, &imHeight);
    fileTextPtr += (lineBytes + 1);

    TRACELOGD("FONT: [%s] Loaded font info:", fileName);
    TRACELOGD("    > Base size: %i", fontSize);
    TRACELOGD("    > Texture scale: %ix%i", imWidth, imHeight);

    lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
    searchPoint = strstr(buffer, "file");
    sscanf(searchPoint, "file=\"%128[^\"]\"", imFileName);
    fileTextPtr += (lineBytes + 1);

    TRACELOGD("    > Texture filename: %s", imFileName);

    lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
    searchPoint = strstr(buffer, "count");
    sscanf(searchPoint, "count=%i", &glyphCount);
    fileTextPtr += (lineBytes + 1);

    TRACELOGD("    > Chars count: %i", glyphCount);

    // Compose correct path using route of .fnt file (fileName) and imFileName
    char *imPath = NULL;
    char *lastSlash = NULL;

    lastSlash = strrchr(fileName, '/');
    if (lastSlash == NULL) lastSlash = strrchr(fileName, '\\');

    if (lastSlash != NULL)
    {
        // NOTE: We need some extra space to avoid memory corruption on next allocations!
        imPath = RL_CALLOC(TextLength(fileName) - TextLength(lastSlash) + TextLength(imFileName) + 4, 1);
        memcpy(imPath, fileName, TextLength(fileName) - TextLength(lastSlash) + 1);
        memcpy(imPath + TextLength(fileName) - TextLength(lastSlash) + 1, imFileName, TextLength(imFileName));
    }
    else imPath = imFileName;

    TRACELOGD("    > Image loading path: %s", imPath);

    Image imFont = LoadImage(imPath);

    if (imFont.format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
    {
        // Convert image to GRAYSCALE + ALPHA, using the mask as the alpha channel
        Image imFontAlpha = {
            .data = calloc(imFont.width*imFont.height, 2),
            .width = imFont.width,
            .height = imFont.height,
            .format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,
            .mipmaps = 1
        };

        for (int p = 0, i = 0; p < (imFont.width*imFont.height*2); p += 2, i++)
        {
            ((unsigned char *)(imFontAlpha.data))[p] = 0xff;
            ((unsigned char *)(imFontAlpha.data))[p + 1] = ((unsigned char *)imFont.data)[i];
        }

        UnloadImage(imFont);
        imFont = imFontAlpha;
    }

    font.texture = LoadTextureFromImage(imFont);

    if (lastSlash != NULL) RL_FREE(imPath);

    // Fill font characters info data
    font.baseSize = fontSize;
    font.glyphCount = glyphCount;
    font.glyphPadding = 0;
    font.glyphs = (GlyphInfo *)RL_MALLOC(glyphCount*sizeof(GlyphInfo));
    font.recs = (Rectangle *)RL_MALLOC(glyphCount*sizeof(Rectangle));

    int charId, charX, charY, charWidth, charHeight, charOffsetX, charOffsetY, charAdvanceX;

    for (int i = 0; i < glyphCount; i++)
    {
        lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
        sscanf(buffer, "char id=%i x=%i y=%i width=%i height=%i xoffset=%i yoffset=%i xadvance=%i",
                       &charId, &charX, &charY, &charWidth, &charHeight, &charOffsetX, &charOffsetY, &charAdvanceX);
        fileTextPtr += (lineBytes + 1);

        // Get character rectangle in the font atlas texture
        font.recs[i] = (Rectangle){ (float)charX, (float)charY, (float)charWidth, (float)charHeight };

        // Save data properly in sprite font
        font.glyphs[i].value = charId;
        font.glyphs[i].offsetX = charOffsetX;
        font.glyphs[i].offsetY = charOffsetY;
        font.glyphs[i].advanceX = charAdvanceX;

        // Fill character image data from imFont data
        font.glyphs[i].image = ImageFromImage(imFont, font.recs[i]);
    }

    UnloadImage(imFont);
    UnloadFileText(fileText);

    if (font.texture.id == 0)
    {
        UnloadFont(font);
        font = GetFontDefault();
        TRACELOG(LOG_WARNING, "FONT: [%s] Failed to load texture, reverted to default font", fileName);
    }
    else TRACELOG(LOG_INFO, "FONT: [%s] Font loaded successfully (%i glyphs)", fileName, font.glyphCount);

    return font;
}
#endif
