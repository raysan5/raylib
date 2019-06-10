/**********************************************************************************************
*
*   raylib.text - Basic functions to load Fonts and draw Text
*
*   CONFIGURATION:
*
*   #define SUPPORT_FILEFORMAT_FNT
*   #define SUPPORT_FILEFORMAT_TTF
*       Selected desired fileformats to be supported for loading. Some of those formats are
*       supported by default, to remove support, just comment unrequired #define in this module
*
*   #define SUPPORT_DEFAULT_FONT
*
*   DEPENDENCIES:
*       stb_truetype - Load TTF file and rasterize characters data
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2019 Ramon Santamaria (@raysan5)
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
    #include "config.h"         // Defines module configuration flags
#endif

#include <stdlib.h>         // Required for: malloc(), free()
#include <string.h>         // Required for: strlen()
#include <stdarg.h>         // Required for: va_list, va_start(), vfprintf(), va_end()
#include <stdio.h>          // Required for: FILE, fopen(), fclose(), fscanf(), feof(), rewind(), fgets()
#include <ctype.h>          // Required for: toupper(), tolower()

#include "utils.h"          // Required for: fopen() Android mapping

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
#define MAX_TEXT_BUFFER_LENGTH  1024        // Size of internal static buffers of some Text*() functions

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global variables
//----------------------------------------------------------------------------------
#if defined(SUPPORT_DEFAULT_FONT)
static Font defaultFont;        // Default font provided by raylib
// NOTE: defaultFont is loaded on InitWindow and disposed on CloseWindow [module: core]
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
extern void LoadDefaultFont(void);
extern void UnloadDefaultFont(void);
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
#if defined(SUPPORT_DEFAULT_FONT)

// Load raylib default font
extern void LoadDefaultFont(void)
{
    #define BIT_CHECK(a,b) ((a) & (1u << (b)))

    // NOTE: Using UTF8 encoding table for Unicode U+0000..U+00FF Basic Latin + Latin-1 Supplement
    // http://www.utf8-chartable.de/unicode-utf8-table.pl

    defaultFont.charsCount = 224;             // Number of chars included in our default font

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
    int imWidth = 128;
    int imHeight = 128;

    Color *imagePixels = (Color *)RL_MALLOC(imWidth*imHeight*sizeof(Color));

    for (int i = 0; i < imWidth*imHeight; i++) imagePixels[i] = BLANK;        // Initialize array

    int counter = 0;        // Font data elements counter

    // Fill imgData with defaultFontData (convert from bit to pixel!)
    for (int i = 0; i < imWidth*imHeight; i += 32)
    {
        for (int j = 31; j >= 0; j--)
        {
            if (BIT_CHECK(defaultFontData[counter], j)) imagePixels[i+j] = WHITE;
        }

        counter++;

        if (counter > 512) counter = 0;         // Security check...
    }

    Image image = LoadImageEx(imagePixels, imWidth, imHeight);
    ImageFormat(&image, UNCOMPRESSED_GRAY_ALPHA);

    RL_FREE(imagePixels);

    defaultFont.texture = LoadTextureFromImage(image);
    UnloadImage(image);

    // Reconstruct charSet using charsWidth[], charsHeight, charsDivisor, charsCount
    //------------------------------------------------------------------------------

    // Allocate space for our characters info data
    // NOTE: This memory should be freed at end! --> CloseWindow()
    defaultFont.chars = (CharInfo *)RL_MALLOC(defaultFont.charsCount*sizeof(CharInfo));

    int currentLine = 0;
    int currentPosX = charsDivisor;
    int testPosX = charsDivisor;

    for (int i = 0; i < defaultFont.charsCount; i++)
    {
        defaultFont.chars[i].value = 32 + i;  // First char is 32

        defaultFont.chars[i].rec.x = (float)currentPosX;
        defaultFont.chars[i].rec.y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        defaultFont.chars[i].rec.width = (float)charsWidth[i];
        defaultFont.chars[i].rec.height = (float)charsHeight;

        testPosX += (int)(defaultFont.chars[i].rec.width + (float)charsDivisor);

        if (testPosX >= defaultFont.texture.width)
        {
            currentLine++;
            currentPosX = 2*charsDivisor + charsWidth[i];
            testPosX = currentPosX;

            defaultFont.chars[i].rec.x = (float)charsDivisor;
            defaultFont.chars[i].rec.y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        }
        else currentPosX = testPosX;

        // NOTE: On default font character offsets and xAdvance are not required
        defaultFont.chars[i].offsetX = 0;
        defaultFont.chars[i].offsetY = 0;
        defaultFont.chars[i].advanceX = 0;
    }

    defaultFont.baseSize = (int)defaultFont.chars[0].rec.height;

    TraceLog(LOG_INFO, "[TEX ID %i] Default font loaded successfully", defaultFont.texture.id);
}

// Unload raylib default font
extern void UnloadDefaultFont(void)
{
    UnloadTexture(defaultFont.texture);
    RL_FREE(defaultFont.chars);
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
    // Default hardcoded values for ttf file loading
    #define DEFAULT_TTF_FONTSIZE    32      // Font first character (32 - space)
    #define DEFAULT_TTF_NUMCHARS    95      // ASCII 32..126 is 95 glyphs
    #define DEFAULT_FIRST_CHAR      32      // Expected first char for image sprite font

    Font font = { 0 };

#if defined(SUPPORT_FILEFORMAT_TTF)
    if (IsFileExtension(fileName, ".ttf") || IsFileExtension(fileName, ".otf")) font = LoadFontEx(fileName, DEFAULT_TTF_FONTSIZE, NULL, DEFAULT_TTF_NUMCHARS);
    else
#endif
#if defined(SUPPORT_FILEFORMAT_FNT)
    if (IsFileExtension(fileName, ".fnt")) font = LoadBMFont(fileName);
    else
#endif
    {
        Image image = LoadImage(fileName);
        if (image.data != NULL) font = LoadFontFromImage(image, MAGENTA, DEFAULT_FIRST_CHAR);
        UnloadImage(image);
    }

    if (font.texture.id == 0)
    {
        TraceLog(LOG_WARNING, "[%s] Font could not be loaded, using default font", fileName);
        font = GetFontDefault();
    }
    else SetTextureFilter(font.texture, FILTER_POINT);    // By default we set point filter (best performance)

    return font;
}

// Load Font from TTF font file with generation parameters
// NOTE: You can pass an array with desired characters, those characters should be available in the font
// if array is NULL, default char set is selected 32..126
Font LoadFontEx(const char *fileName, int fontSize, int *fontChars, int charsCount)
{
    Font font = { 0 };

    font.baseSize = fontSize;
    font.charsCount = (charsCount > 0)? charsCount : 95;
    font.chars = LoadFontData(fileName, font.baseSize, fontChars, font.charsCount, FONT_DEFAULT);

#if defined(SUPPORT_FILEFORMAT_TTF)
    if (font.chars != NULL)
    {
        Image atlas = GenImageFontAtlas(font.chars, font.charsCount, font.baseSize, 2, 0);
        font.texture = LoadTextureFromImage(atlas);
        UnloadImage(atlas);
    }
    else font = GetFontDefault();
#else
    font = GetFontDefault();
#endif

    return font;
}

// Load an Image font file (XNA style)
Font LoadFontFromImage(Image image, Color key, int firstChar)
{
    #define COLOR_EQUAL(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))

    int charSpacing = 0;
    int lineSpacing = 0;

    int x = 0;
    int y = 0;

    // Default number of characters supported
    #define MAX_FONTCHARS          256

    // We allocate a temporal arrays for chars data measures,
    // once we get the actual number of chars, we copy data to a sized arrays
    int tempCharValues[MAX_FONTCHARS];
    Rectangle tempCharRecs[MAX_FONTCHARS];

    Color *pixels = GetImageData(image);

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

    TraceLog(LOG_DEBUG, "Font data parsed correctly from image");

    // NOTE: We need to remove key color borders from image to avoid weird
    // artifacts on texture scaling when using FILTER_BILINEAR or FILTER_TRILINEAR
    for (int i = 0; i < image.height*image.width; i++) if (COLOR_EQUAL(pixels[i], key)) pixels[i] = BLANK;

    // Create a new image with the processed color data (key color replaced by BLANK)
    Image fontClear = LoadImageEx(pixels, image.width, image.height);

    RL_FREE(pixels);    // Free pixels array memory

    // Create spritefont with all data parsed from image
    Font spriteFont = { 0 };

    spriteFont.texture = LoadTextureFromImage(fontClear); // Convert processed image to OpenGL texture
    spriteFont.charsCount = index;

    UnloadImage(fontClear);     // Unload processed image once converted to texture

    // We got tempCharValues and tempCharsRecs populated with chars data
    // Now we move temp data to sized charValues and charRecs arrays
    spriteFont.chars = (CharInfo *)RL_MALLOC(spriteFont.charsCount*sizeof(CharInfo));

    for (int i = 0; i < spriteFont.charsCount; i++)
    {
        spriteFont.chars[i].value = tempCharValues[i];
        spriteFont.chars[i].rec = tempCharRecs[i];

        // NOTE: On image based fonts (XNA style), character offsets and xAdvance are not required (set to 0)
        spriteFont.chars[i].offsetX = 0;
        spriteFont.chars[i].offsetY = 0;
        spriteFont.chars[i].advanceX = 0;
        spriteFont.chars[i].data = NULL;
    }

    spriteFont.baseSize = (int)spriteFont.chars[0].rec.height;

    TraceLog(LOG_INFO, "Image file loaded correctly as Font");

    return spriteFont;
}

// Load font data for further use
// NOTE: Requires TTF font and can generate SDF data
CharInfo *LoadFontData(const char *fileName, int fontSize, int *fontChars, int charsCount, int type)
{
    // NOTE: Using some SDF generation default values,
    // trades off precision with ability to handle *smaller* sizes
    #define SDF_CHAR_PADDING            4
    #define SDF_ON_EDGE_VALUE         128
    #define SDF_PIXEL_DIST_SCALE     64.0f

    #define BITMAP_ALPHA_THRESHOLD     80

    CharInfo *chars = NULL;

#if defined(SUPPORT_FILEFORMAT_TTF)
    // Load font data (including pixel data) from TTF file
    // NOTE: Loaded information should be enough to generate font image atlas,
    // using any packaging method
    FILE *fontFile = fopen(fileName, "rb");     // Load font file

    if (fontFile != NULL)
    {
        fseek(fontFile, 0, SEEK_END);
        long size = ftell(fontFile);    // Get file size
        fseek(fontFile, 0, SEEK_SET);   // Reset file pointer

        unsigned char *fontBuffer = (unsigned char *)RL_MALLOC(size);

        fread(fontBuffer, size, 1, fontFile);
        fclose(fontFile);

        // Init font for data reading
        stbtt_fontinfo fontInfo;
        if (!stbtt_InitFont(&fontInfo, fontBuffer, 0)) TraceLog(LOG_WARNING, "Failed to init font!");

        // Calculate font scale factor
        float scaleFactor = stbtt_ScaleForPixelHeight(&fontInfo, (float)fontSize);

        // Calculate font basic metrics
        // NOTE: ascent is equivalent to font baseline
        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

        // In case no chars count provided, default to 95
        charsCount = (charsCount > 0)? charsCount : 95;

        // Fill fontChars in case not provided externally
        // NOTE: By default we fill charsCount consecutevely, starting at 32 (Space)
        int genFontChars = false;
        if (fontChars == NULL)
        {
            fontChars = (int *)RL_MALLOC(charsCount*sizeof(int));
            for (int i = 0; i < charsCount; i++) fontChars[i] = i + 32;
            genFontChars = true;
        }

        chars = (CharInfo *)RL_MALLOC(charsCount*sizeof(CharInfo));

        // NOTE: Using simple packaging, one char after another
        for (int i = 0; i < charsCount; i++)
        {
            int chw = 0, chh = 0;   // Character width and height (on generation)
            int ch = fontChars[i];  // Character value to get info for
            chars[i].value = ch;

            //  Render a unicode codepoint to a bitmap
            //      stbtt_GetCodepointBitmap()           -- allocates and returns a bitmap
            //      stbtt_GetCodepointBitmapBox()        -- how big the bitmap must be
            //      stbtt_MakeCodepointBitmap()          -- renders into bitmap you provide

            if (type != FONT_SDF) chars[i].data = stbtt_GetCodepointBitmap(&fontInfo, scaleFactor, scaleFactor, ch, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY);
            else if (ch != 32) chars[i].data = stbtt_GetCodepointSDF(&fontInfo, scaleFactor, ch, SDF_CHAR_PADDING, SDF_ON_EDGE_VALUE, SDF_PIXEL_DIST_SCALE, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY);
            else chars[i].data = NULL;

            if (type == FONT_BITMAP)
            {
                // Aliased bitmap (black & white) font generation, avoiding anti-aliasing
                // NOTE: For optimum results, bitmap font should be generated at base pixel size
                for (int p = 0; p < chw*chh; p++)
                {
                    if (chars[i].data[p] < BITMAP_ALPHA_THRESHOLD) chars[i].data[p] = 0;
                    else chars[i].data[p] = 255;
                }
            }

            chars[i].rec.width = (float)chw;
            chars[i].rec.height = (float)chh;
            chars[i].offsetY += (int)((float)ascent*scaleFactor);

            // Get bounding box for character (may be offset to account for chars that dip above or below the line)
            int chX1, chY1, chX2, chY2;
            stbtt_GetCodepointBitmapBox(&fontInfo, ch, scaleFactor, scaleFactor, &chX1, &chY1, &chX2, &chY2);

            TraceLog(LOG_DEBUG, "Character box measures: %i, %i, %i, %i", chX1, chY1, chX2 - chX1, chY2 - chY1);
            TraceLog(LOG_DEBUG, "Character offsetY: %i", (int)((float)ascent*scaleFactor) + chY1);

            stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advanceX, NULL);
            chars[i].advanceX *= scaleFactor;
        }

        RL_FREE(fontBuffer);
        if (genFontChars) RL_FREE(fontChars);
    }
    else TraceLog(LOG_WARNING, "[%s] TTF file could not be opened", fileName);
#else
    TraceLog(LOG_WARNING, "[%s] TTF support is disabled", fileName);
#endif

    return chars;
}

// Generate image font atlas using chars info
// NOTE: Packing method: 0-Default, 1-Skyline
#if defined(SUPPORT_FILEFORMAT_TTF)
Image GenImageFontAtlas(CharInfo *chars, int charsCount, int fontSize, int padding, int packMethod)
{
    Image atlas = { 0 };

    // In case no chars count provided we suppose default of 95
    charsCount = (charsCount > 0)? charsCount : 95;

    // Calculate image size based on required pixel area
    // NOTE 1: Image is forced to be squared and POT... very conservative!
    // NOTE 2: SDF font characters already contain an internal padding,
    // so image size would result bigger than default font type
    float requiredArea = 0;
    for (int i = 0; i < charsCount; i++) requiredArea += ((chars[i].rec.width + 2*padding)*(chars[i].rec.height + 2*padding));
    float guessSize = sqrtf(requiredArea)*1.25f;
    int imageSize = (int)powf(2, ceilf(logf((float)guessSize)/logf(2)));  // Calculate next POT

    atlas.width = imageSize;   // Atlas bitmap width
    atlas.height = imageSize;  // Atlas bitmap height
    atlas.data = (unsigned char *)RL_CALLOC(1, atlas.width*atlas.height);      // Create a bitmap to store characters (8 bpp)
    atlas.format = UNCOMPRESSED_GRAYSCALE;
    atlas.mipmaps = 1;

    // DEBUG: We can see padding in the generated image setting a gray background...
    //for (int i = 0; i < atlas.width*atlas.height; i++) ((unsigned char *)atlas.data)[i] = 100;

    if (packMethod == 0)   // Use basic packing algorythm
    {
        int offsetX = padding;
        int offsetY = padding;

        // NOTE: Using simple packaging, one char after another
        for (int i = 0; i < charsCount; i++)
        {
            // Copy pixel data from fc.data to atlas
            for (int y = 0; y < (int)chars[i].rec.height; y++)
            {
                for (int x = 0; x < (int)chars[i].rec.width; x++)
                {
                    ((unsigned char *)atlas.data)[(offsetY + y)*atlas.width + (offsetX + x)] = chars[i].data[y*(int)chars[i].rec.width + x];
                }
            }

            chars[i].rec.x = (float)offsetX;
            chars[i].rec.y = (float)offsetY;

            // Move atlas position X for next character drawing
            offsetX += ((int)chars[i].rec.width + 2*padding);

            if (offsetX >= (atlas.width - (int)chars[i].rec.width - padding))
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
        TraceLog(LOG_DEBUG, "Using Skyline packing algorythm!");

        stbrp_context *context = (stbrp_context *)RL_MALLOC(sizeof(*context));
        stbrp_node *nodes = (stbrp_node *)RL_MALLOC(charsCount*sizeof(*nodes));

        stbrp_init_target(context, atlas.width, atlas.height, nodes, charsCount);
        stbrp_rect *rects = (stbrp_rect *)RL_MALLOC(charsCount*sizeof(stbrp_rect));

        // Fill rectangles for packaging
        for (int i = 0; i < charsCount; i++)
        {
            rects[i].id = i;
            rects[i].w = (int)chars[i].rec.width + 2*padding;
            rects[i].h = (int)chars[i].rec.height + 2*padding;
        }

        // Package rectangles into atlas
        stbrp_pack_rects(context, rects, charsCount);

        for (int i = 0; i < charsCount; i++)
        {
            chars[i].rec.x = rects[i].x + (float)padding;
            chars[i].rec.y = rects[i].y + (float)padding;

            if (rects[i].was_packed)
            {
                // Copy pixel data from fc.data to atlas
                for (int y = 0; y < (int)chars[i].rec.height; y++)
                {
                    for (int x = 0; x < (int)chars[i].rec.width; x++)
                    {
                        ((unsigned char *)atlas.data)[(rects[i].y + padding + y)*atlas.width + (rects[i].x + padding + x)] = chars[i].data[y*(int)chars[i].rec.width + x];
                    }
                }
            }
            else TraceLog(LOG_WARNING, "Character could not be packed: %i", i);
        }

        RL_FREE(rects);
        RL_FREE(nodes);
        RL_FREE(context);
    }

    // TODO: Crop image if required for smaller size

    // Convert image data from GRAYSCALE to GRAY_ALPHA
    // WARNING: ImageAlphaMask(&atlas, atlas) does not work in this case, requires manual operation
    unsigned char *dataGrayAlpha = (unsigned char *)RL_MALLOC(imageSize*imageSize*sizeof(unsigned char)*2); // Two channels

    for (int i = 0, k = 0; i < atlas.width*atlas.height; i++, k += 2)
    {
        dataGrayAlpha[k] = 255;
        dataGrayAlpha[k + 1] = ((unsigned char *)atlas.data)[i];
    }

    RL_FREE(atlas.data);
    atlas.data = dataGrayAlpha;
    atlas.format = UNCOMPRESSED_GRAY_ALPHA;

    return atlas;
}
#endif

// Unload Font from GPU memory (VRAM)
void UnloadFont(Font font)
{
    // NOTE: Make sure spriteFont is not default font (fallback)
    if (font.texture.id != GetFontDefault().texture.id)
    {
        for (int i = 0; i < font.charsCount; i++)
        {
            RL_FREE(font.chars[i].data);
        }
        UnloadTexture(font.texture);
        RL_FREE(font.chars);

        TraceLog(LOG_DEBUG, "Unloaded sprite font data");
    }
}

// Shows current FPS on top-left corner
// NOTE: Uses default font
void DrawFPS(int posX, int posY)
{
    // NOTE: We are rendering fps every second for better viewing on high framerates

    static int fps = 0;
    static int counter = 0;
    static int refreshRate = 20;

    if (counter < refreshRate) counter++;
    else
    {
        fps = GetFPS();
        refreshRate = fps;
        counter = 0;
    }

    // NOTE: We have rounding errors every frame, so it oscillates a lot
    DrawText(TextFormat("%2i FPS", fps), posX, posY, 20, LIME);
}

// Returns next codepoint in a UTF8 encoded `text` scanning until '\0' is found. When a invalid UTF8 byte is encountered we exit as soon
// as possible and a `?`(0x3f) codepoint is returned. `count` will hold the total number of bytes processed.
// NOTE: the standard says U+FFFD should be returned in case of errors but that character is not supported by the default font in raylib
// TODO: optimize this code for speed!!
int GetNextCodepoint(const char* text, int* count)
{
/* 
   UTF8 specs from https://www.ietf.org/rfc/rfc3629.txt

   Char. number range  |        UTF-8 octet sequence
      (hexadecimal)    |              (binary)
   --------------------+---------------------------------------------
   0000 0000-0000 007F | 0xxxxxxx
   0000 0080-0000 07FF | 110xxxxx 10xxxxxx
   0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
   0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
    
    // NOTE: on decode errors we return as soon as possible
    
    int c = 0x3f;   // Codepoint (defaults to `?`)
    int o = (unsigned char)(text[0]); // The first UTF8 octet
    *count = 1;
    
    if( o <= 0x7f )
    { 
        // Only one octet (ASCII range x00-7F)
        c = text[0];
    }
    else if((o & 0xe0) == 0xc0)
    {
        // Two octets 
        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        unsigned char o1 = text[1];
        if(o1 == '\0' || (o1 >> 6) != 2 ) {*count = 2; return c; } // Unexpected sequence
        if(o >= 0xc2 && o <= 0xdf) 
        {
            c = ((o & 0x1f) << 6) | (o1 & 0x3f);
            *count = 2;
        }
    }
    else if( (o & 0xf0) == 0xe0 )
    {
        // Three octets
        unsigned char o1 = text[1], o2 = '\0';
        if(o1 == '\0' || (o1 >> 6) != 2) { *count = 2; return c; } // Unexpected sequence
        o2 = text[2];
        if(o2 == '\0' || (o2 >> 6) != 2) {*count = 3; return c; } // Unexpected sequence
        
        /* [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
           [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
           [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
           [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        */
        
        if((o == 0xe0 && !(o1 >= 0xa0 && o1 <= 0xbf)) || (o == 0xed && !(o1 >= 0x80 && o1 <= 0x9f)) ) {*count = 2; return c;}
        if(o >= 0xe0 && 0 <= 0xef)
        {
            c = ((o & 0xf) << 12) | ((o1 & 0x3f) << 6) | (o2 & 0x3f);
            *count = 3;
        }
    }
    else if( (o & 0xf8) == 0xf0 ) 
    {
        // Four octets
        if(o > 0xf4) return c;
        
        unsigned char o1 = text[1], o2 = '\0', o3 = '\0';
        if(o1 == '\0' || (o1 >> 6) != 2) { *count = 2; return c; }  // Unexpected sequence
        o2 = text[2];
        if(o2 == '\0' || (o2 >> 6) != 2) { *count = 3; return c; }  // Unexpected sequence
        o3 = text[3];
        if(o3 == '\0' || (o3 >> 6) != 2) { *count = 4; return c; }  // Unexpected sequence
        
        /* [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
           [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
           [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail
        */
        if((o == 0xf0 && !(o1 >= 0x90 && o1 <= 0xbf)) || (o == 0xf4 && !( o1 >= 0x80 && o1 <= 0x8f)) ) { *count = 2; return c; } // Unexpected sequence
        if( o >= 0xf0)
        {
            c = ((o & 0x7) << 18) | ((o1 & 0x3f) << 12) | ((o2 & 0x3f) << 6) | (o3 & 0x3f);
            *count = 4;
        }
    }
    
    if(c > 0x10ffff) c = 0x3f; // Codepoints after U+10ffff are invalid
    return c;
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
    int length = strlen(text);
    int textOffsetY = 0;        // Required for line break!
    float textOffsetX = 0.0f;   // Offset between characters
    float scaleFactor = 0.0f;

    int letter = 0;             // Current character
    int index = 0;              // Index position in sprite font

    scaleFactor = fontSize/font.baseSize;

    for (int i = 0; i < length; i++)
    {
        int next = 1;
        letter = GetNextCodepoint(&text[i], &next);
        // NOTE: normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set `next = 1`
        if(letter == 0x3f) next = 1; 
        index = GetGlyphIndex(font, letter);
        i += next - 1;
        
        if (letter == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 lines
            textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
            textOffsetX = 0.0f;
        }
        else
        {
            if (letter != ' ')
            {
                DrawTexturePro(font.texture, font.chars[index].rec,
                           (Rectangle){ position.x + textOffsetX + font.chars[index].offsetX*scaleFactor,
                                        position.y + textOffsetY + font.chars[index].offsetY*scaleFactor,
                                        font.chars[index].rec.width*scaleFactor,
                                        font.chars[index].rec.height*scaleFactor }, (Vector2){ 0, 0 }, 0.0f, tint);
            }

            if (font.chars[index].advanceX == 0) textOffsetX += ((float)font.chars[index].rec.width*scaleFactor + spacing);
            else textOffsetX += ((float)font.chars[index].advanceX*scaleFactor + spacing);
        }
    }
}

// Draw text using font inside rectangle limits
void DrawTextRec(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint)
{
    DrawTextRecEx(font, text, rec, fontSize, spacing, wordWrap, tint, 0, 0, WHITE, WHITE);
}

// Draw text using font inside rectangle limits with support for text selection
void DrawTextRecEx(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint,
                   int selectStart, int selectLength, Color selectText, Color selectBack)
{
    int length = strlen(text);
    int textOffsetX = 0;        // Offset between characters
    int textOffsetY = 0;        // Required for line break!
    float scaleFactor = 0.0f;

    int letter = 0;             // Current character
    int index = 0;              // Index position in sprite font

    scaleFactor = fontSize/font.baseSize;

    enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
    int state = wordWrap? MEASURE_STATE : DRAW_STATE;
    int startLine = -1;   // Index where to begin drawing (where a line begins)
    int endLine = -1;     // Index where to stop drawing (where a line ends)
    int lastk = -1;       // Holds last value of the character position
    for (int i = 0, k = 0; i < length; i++, k++)
    {
        int glyphWidth = 0;
        int next = 1;
        letter = GetNextCodepoint(&text[i], &next);
        // NOTE: normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set `next = 1`
        if(letter == 0x3f) next = 1; 
        index = GetGlyphIndex(font, letter);
        i += next - 1;

        if (letter != '\n')
        {   
            glyphWidth = (font.chars[index].advanceX == 0)?
                         (int)(font.chars[index].rec.width*scaleFactor + spacing):
                         (int)(font.chars[index].advanceX*scaleFactor + spacing);
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw
        // before going outside of the `rec` container. We store this info inside
        // `startLine` and `endLine` then we change states, draw the text between those two
        // variables then change states again and again recursively until the end of the text
        // (or until we get outside of the container).
        // When wordWrap is OFF we don't need the measure state so we go to the drawing
        // state immediately and begin drawing on the next line before we can get outside
        // the container.
        if (state == MEASURE_STATE)
        {
            // TODO: there are multiple types of `spaces` in UNICODE, maybe it's a good idea to add support for more
            // see: http://jkorpela.fi/chars/spaces.html 
            if ((letter == ' ') || (letter == '\t') || (letter == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth + 1) >= rec.width)
            {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= next;
                if ((startLine + next) == endLine) endLine = i - next;
                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;
                state = !state;
            }
            else if (letter == '\n')
            {
                state = !state;
            }

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;
                
                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }

        }
        else
        {
            if (letter == '\n')
            {
                if (!wordWrap)
                {
                    textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
                    textOffsetX = 0;
                }
            }
            else
            {
                if (!wordWrap && ((textOffsetX + glyphWidth + 1) >= rec.width))
                {
                    textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
                    textOffsetX = 0;
                }

                if ((textOffsetY + (int)(font.baseSize*scaleFactor)) > rec.height) break;

                //draw selected
                bool isGlyphSelected = false;
                if ((selectStart >= 0) && (k >= selectStart) && (k < (selectStart + selectLength)))
                {
                    Rectangle strec = {rec.x + textOffsetX-1, rec.y + textOffsetY, glyphWidth, font.baseSize*scaleFactor };
                    DrawRectangleRec(strec, selectBack);
                    isGlyphSelected = true;
                }

                //draw glyph
                if ((letter != ' ') && (letter != '\t'))
                {
                    DrawTexturePro(font.texture, font.chars[index].rec,
                      (Rectangle){ rec.x + textOffsetX + font.chars[index].offsetX*scaleFactor,
                                   rec.y + textOffsetY + font.chars[index].offsetY*scaleFactor,
                                   font.chars[index].rec.width*scaleFactor,
                                   font.chars[index].rec.height*scaleFactor }, (Vector2){ 0, 0 }, 0.0f,
                                   (!isGlyphSelected)? tint : selectText);
                }
            }

            if (wordWrap && (i == endLine))
            {
                textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                k = lastk;
                state = !state;
            }
        }

        textOffsetX += glyphWidth;
    }
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
    int len = strlen(text);
    int tempLen = 0;                // Used to count longer text line num chars
    int lenCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f;     // Used to count longer text line width

    float textHeight = (float)font.baseSize;
    float scaleFactor = fontSize/(float)font.baseSize;

    int letter = 0;                 // Current character
    int index = 0;                  // Index position in sprite font

    for (int i = 0; i < len; i++)
    {
        lenCounter++;
        
        int next = 1;
        letter = GetNextCodepoint(&text[i], &next);

        // NOTE: normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set `next = 1`
        if(letter == 0x3f) next = 1; 
        i += next - 1;  
        
        if (letter != '\n')
        {
            index = GetGlyphIndex(font, letter);
            if (font.chars[index].advanceX != 0) textWidth += font.chars[index].advanceX;
            else textWidth += (font.chars[index].rec.width + font.chars[index].offsetX);
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            lenCounter = 0;
            textWidth = 0;
            textHeight += ((float)font.baseSize*1.5f); // NOTE: Fixed line spacing of 1.5 lines
        }

        if (tempLen < lenCounter) tempLen = lenCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    Vector2 vec;
    vec.x = tempTextWidth*scaleFactor + (float)((tempLen - 1)*spacing); // Adds chars spacing to measure
    vec.y = textHeight*scaleFactor;

    return vec;
}

// Returns index position for a unicode character on spritefont
int GetGlyphIndex(Font font, int character)
{
#define UNORDERED_CHARSET
#if defined(UNORDERED_CHARSET)
    int index = 0;

    for (int i = 0; i < font.charsCount; i++)
    {
        if (font.chars[i].value == character)
        {
            index = i;
            break;
        }
    }

    return index;
#else
    return (character - 32);
#endif
}

// Text strings management functions
//----------------------------------------------------------------------------------
// Check if two text string are equal
// REQUIRES: strcmp()
bool TextIsEqual(const char *text1, const char *text2)
{
    bool result = false;

    if (strcmp(text1, text2) == 0) result = true;

    return result;
}

// Get text length in bytes, check for \0 character
unsigned int TextLength(const char *text)
{
    unsigned int length = 0;

    while (*text++) length++;

    return length;
}

// Returns total number of characters(codepoints) in a UTF8 encoded `text` until `\0` is found.
// NOTE: If a invalid UTF8 sequence is encountered a `?`(0x3f) codepoint is counted instead. 
unsigned int TextCountCodepoints(const char *text) 
{
    unsigned int len = 0;
    char* ptr = (char*)&text[0];
    while(*ptr != '\0')
    {
        int next = 0;
        int letter = GetNextCodepoint(ptr, &next);
        if(letter == 0x3f) ptr += 1;
        else ptr += next;
        ++len;
    }
    return len;
}

// Formatting of text with variables to 'embed'
const char *TextFormat(const char *text, ...)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    va_list args;
    va_start(args, text);
    vsprintf(buffer, text, args);
    va_end(args);

    return buffer;
}

// Get a piece of a text string
// REQUIRES: strlen()
const char *TextSubtext(const char *text, int position, int length)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    int textLength = strlen(text);

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
// REQUIRES: strlen(), strstr(), strncpy(), strcpy()
// WARNING: Internally allocated memory must be freed by the user (if return != NULL)
const char *TextReplace(char *text, const char *replace, const char *by)
{
    char *result;

    char *insertPoint;      // Next insert point
    char *temp;             // Temp pointer
    int replaceLen;         // Replace string length of (the string to remove)
    int byLen;              // Replacement length (the string to replace replace by)
    int lastReplacePos;     // Distance between replace and end of last replace
    int count;              // Number of replacements

    // Sanity checks and initialization
    if (!text || !replace) return NULL;

    replaceLen = strlen(replace);
    if (replaceLen == 0) return NULL;  // Empty replace causes infinite loop during count

    if (!by) by = "";           // Replace by nothing if not provided
    byLen = strlen(by);

    // Count the number of replacements needed
    insertPoint = text;
    for (count = 0; (temp = strstr(insertPoint, replace)); count++) insertPoint = temp + replaceLen;

    // Allocate returning string and point temp to it
    temp = result = RL_MALLOC(strlen(text) + (byLen - replaceLen)*count + 1);

    if (!result) return NULL;   // Memory could not be allocated

    // First time through the loop, all the variable are set correctly from here on,
    //    temp points to the end of the result string
    //    insertPoint points to the next occurrence of replace in text
    //    text points to the remainder of text after "end of replace"
    while (count--)
    {
        insertPoint = strstr(text, replace);
        lastReplacePos = insertPoint - text;
        temp = strncpy(temp, text, lastReplacePos) + lastReplacePos;
        temp = strcpy(temp, by) + byLen;
        text += lastReplacePos + replaceLen; // Move to next "end of replace"
    }

    // Copy remaind text part after replacement to result (pointed by moving temp)
    strcpy(temp, text);

    return result;
}

// Insert text in a specific position, moves all text forward
// REQUIRES: strlen(), strcpy(), strtok()
// WARNING: Allocated memory should be manually freed
const char *TextInsert(const char *text, const char *insert, int position)
{
    int textLen = strlen(text);
    int insertLen =  strlen(insert);

    char *result = (char *)RL_MALLOC(textLen + insertLen + 1);

    for (int i = 0; i < position; i++) result[i] = text[i];
    for (int i = position; i < insertLen + position; i++) result[i] = insert[i];
    for (int i = (insertLen + position); i < (textLen + insertLen); i++) result[i] = text[i];

    result[textLen + insertLen] = '\0';     // Make sure text string is valid!

    return result;
}

// Join text strings with delimiter
// REQUIRES: strcat()
const char *TextJoin(const char **textList, int count, const char *delimiter)
{
    // TODO: Make sure joined text could fit inside MAX_TEXT_BUFFER_LENGTH

    static char text[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(text, 0, MAX_TEXT_BUFFER_LENGTH);

    int delimiterLen = strlen(delimiter);

    for (int i = 0; i < count; i++)
    {
        strcat(text, textList[i]);
        if ((delimiterLen > 0) && (i < (count - 1))) strcat(text, delimiter);
    }

    return text;
}

// Split string into multiple strings
const char **TextSplit(const char *text, char delimiter, int *count)
{
    // NOTE: Current implementation returns a copy of the provided string with '\0' (string end delimiter)
    // inserted between strings defined by "delimiter" parameter. No memory is dynamically allocated,
    // all used memory is static... it has some limitations:
    //      1. Maximum number of possible split strings is set by MAX_SUBSTRINGS_COUNT
    //      2. Maximum size of text to split is MAX_TEXT_BUFFER_LENGTH

    #define MAX_SUBSTRINGS_COUNT  64

    static const char *result[MAX_SUBSTRINGS_COUNT] = { NULL };
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    result[0] = buffer;
    int counter = 1;

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

            if (counter == MAX_SUBSTRINGS_COUNT) break;
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
    *position += strlen(append);
}

// Find first text occurrence within a string
// REQUIRES: strstr()
int TextFindIndex(const char *text, const char *find)
{
    int position = -1;

    char *ptr = strstr(text, find);

    if (ptr != NULL) position = ptr - text;

    return position;
}

// Get upper case version of provided string
// REQUIRES: toupper()
const char *TextToUpper(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
    {
        if (text[i] != '\0') buffer[i] = (char)toupper(text[i]);
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
        if (text[i] != '\0') buffer[i] = (char)tolower(text[i]);
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

// Get integer value from text
// NOTE: Negative values not supported
int TextToInteger(const char *text)
{
    int result = 0;
    int len = strlen(text);
    int units = 1;

    for (int i = len - 1; i >= 0; i--)
    {
        if ((text[i] > 47) && (text[i] < 58)) result += ((int)text[i] - 48)*units;
        else { result = -1; break; }

        units *= 10;
    }

    return result;
}
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
#if defined(SUPPORT_FILEFORMAT_FNT)
// Load a BMFont file (AngelCode font file)
static Font LoadBMFont(const char *fileName)
{
    #define MAX_BUFFER_SIZE     256

    Font font = { 0 };
    font.texture.id = 0;

    char buffer[MAX_BUFFER_SIZE] = { 0 };
    char *searchPoint = NULL;

    int fontSize = 0;
    int texWidth = 0;
    int texHeight = 0;
    char texFileName[129];
    int charsCount = 0;

    int base = 0;   // Useless data

    FILE *fntFile = NULL;

    fntFile = fopen(fileName, "rt");

    if (fntFile == NULL)
    {
        TraceLog(LOG_WARNING, "[%s] FNT file could not be opened", fileName);
        return font;
    }

    // NOTE: We skip first line, it contains no useful information
    fgets(buffer, MAX_BUFFER_SIZE, fntFile);
    //searchPoint = strstr(buffer, "size");
    //sscanf(searchPoint, "size=%i", &fontSize);

    fgets(buffer, MAX_BUFFER_SIZE, fntFile);
    searchPoint = strstr(buffer, "lineHeight");
    sscanf(searchPoint, "lineHeight=%i base=%i scaleW=%i scaleH=%i", &fontSize, &base, &texWidth, &texHeight);

    TraceLog(LOG_DEBUG, "[%s] Font size: %i", fileName, fontSize);
    TraceLog(LOG_DEBUG, "[%s] Font texture scale: %ix%i", fileName, texWidth, texHeight);

    fgets(buffer, MAX_BUFFER_SIZE, fntFile);
    searchPoint = strstr(buffer, "file");
    sscanf(searchPoint, "file=\"%128[^\"]\"", texFileName);

    TraceLog(LOG_DEBUG, "[%s] Font texture filename: %s", fileName, texFileName);

    fgets(buffer, MAX_BUFFER_SIZE, fntFile);
    searchPoint = strstr(buffer, "count");
    sscanf(searchPoint, "count=%i", &charsCount);

    TraceLog(LOG_DEBUG, "[%s] Font num chars: %i", fileName, charsCount);

    // Compose correct path using route of .fnt file (fileName) and texFileName
    char *texPath = NULL;
    char *lastSlash = NULL;

    lastSlash = strrchr(fileName, '/');
    if (lastSlash == NULL)
    {
        lastSlash = strrchr(fileName, '\\');
    }

    // NOTE: We need some extra space to avoid memory corruption on next allocations!
    texPath = RL_MALLOC(strlen(fileName) - strlen(lastSlash) + strlen(texFileName) + 4);

    // NOTE: strcat() and strncat() required a '\0' terminated string to work!
    *texPath = '\0';
    strncat(texPath, fileName, strlen(fileName) - strlen(lastSlash) + 1);
    strncat(texPath, texFileName, strlen(texFileName));

    TraceLog(LOG_DEBUG, "[%s] Font texture loading path: %s", fileName, texPath);

    Image imFont = LoadImage(texPath);

    if (imFont.format == UNCOMPRESSED_GRAYSCALE)
    {
        Image imCopy = ImageCopy(imFont);

        for (int i = 0; i < imCopy.width*imCopy.height; i++) ((unsigned char *)imCopy.data)[i] = 0xff;

        ImageAlphaMask(&imCopy, imFont);
        font.texture = LoadTextureFromImage(imCopy);
        UnloadImage(imCopy);
    }
    else font.texture = LoadTextureFromImage(imFont);

    UnloadImage(imFont);
    RL_FREE(texPath);


    // Fill font characters info data
    font.baseSize = fontSize;
    font.charsCount = charsCount;
    font.chars = (CharInfo *)RL_MALLOC(charsCount*sizeof(CharInfo));

    int charId, charX, charY, charWidth, charHeight, charOffsetX, charOffsetY, charAdvanceX;

    for (int i = 0; i < charsCount; i++)
    {
        fgets(buffer, MAX_BUFFER_SIZE, fntFile);
        sscanf(buffer, "char id=%i x=%i y=%i width=%i height=%i xoffset=%i yoffset=%i xadvance=%i",
                       &charId, &charX, &charY, &charWidth, &charHeight, &charOffsetX, &charOffsetY, &charAdvanceX);

        // Save data properly in sprite font
        font.chars[i].value = charId;
        font.chars[i].rec = (Rectangle){ (float)charX, (float)charY, (float)charWidth, (float)charHeight };
        font.chars[i].offsetX = charOffsetX;
        font.chars[i].offsetY = charOffsetY;
        font.chars[i].advanceX = charAdvanceX;
        font.chars[i].data = NULL;
    }

    fclose(fntFile);

    if (font.texture.id == 0)
    {
        UnloadFont(font);
        font = GetFontDefault();
    }
    else TraceLog(LOG_INFO, "[%s] Font loaded successfully", fileName);

    return font;
}
#endif
