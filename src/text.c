/**********************************************************************************************
*
*   raylib.text
*
*   Basic functions to load SpriteFonts and draw Text
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
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

#include "raylib.h"

#include <stdlib.h>         // Required for: malloc(), free()
#include <string.h>         // Required for: strlen()
#include <stdarg.h>         // Required for: va_list, va_start(), vfprintf(), va_end()
#include <stdio.h>          // Required for: FILE, fopen(), fclose(), fscanf(), feof(), rewind(), fgets()

#include "utils.h"          // Required for: GetExtension()

// Following libs are used on LoadTTF()
#define STB_TRUETYPE_IMPLEMENTATION
#include "external/stb_truetype.h"   // Required for: stbtt_BakeFontBitmap()

// Rectangle packing functions (not used at the moment)
//#define STB_RECT_PACK_IMPLEMENTATION
//#include "stb_rect_pack.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define FONT_FIRST_CHAR         32
#define MAX_FONTCHARS          128
#define MAX_FORMATTEXT_LENGTH   64
#define MAX_SUBTEXT_LENGTH      64

#define BIT_CHECK(a,b) ((a) & (1<<(b)))

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global variables
//----------------------------------------------------------------------------------
static SpriteFont defaultFont;        // Default font provided by raylib
// NOTE: defaultFont is loaded on InitWindow and disposed on CloseWindow [module: core]

//----------------------------------------------------------------------------------
// Other Modules Functions Declaration (required by text)
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static SpriteFont LoadImageFont(Image image, Color key, int firstChar); // Load a Image font file (XNA style)
static SpriteFont LoadRBMF(const char *fileName);   // Load a rBMF font file (raylib BitMap Font)
static SpriteFont LoadBMFont(const char *fileName); // Load a BMFont file (AngelCode font file)
static SpriteFont LoadTTF(const char *fileName, int fontSize); // Generate a sprite font image from TTF data (font size required)

extern void LoadDefaultFont(void);
extern void UnloadDefaultFont(void);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
extern void LoadDefaultFont(void)
{
    // NOTE: Using UTF8 encoding table for Unicode U+0000..U+00FF Basic Latin + Latin-1 Supplement
    // http://www.utf8-chartable.de/unicode-utf8-table.pl

    defaultFont.numChars = 224;             // Number of chars included in our default font

    // Default font is directly defined here (data generated from a sprite font image)
    // This way, we reconstruct SpriteFont without creating large global variables
    // This data is automatically allocated to Stack and automatically deallocated at the end of this function
    int defaultFontData[512] = {
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
        0xac000000, 0x00000f01, 0x00000000, 0x00000000, 0x24000000, 0x00000901, 0x00000000, 0x06000000, 0x24000000, 0x00000901, 0x00000000, 0x09108000,
        0x24fa28a2, 0x00000901, 0x00000000, 0x013e0000, 0x2242252a, 0x00000952, 0x00000000, 0x038a8000, 0x2422222a, 0x00000929, 0x00000000, 0x010a8000,
        0x2412252a, 0x00000901, 0x00000000, 0x010a8000, 0x24fbe8be, 0x00000901, 0x00000000, 0x0ebe8000, 0xac020000, 0x00000f01, 0x00000000, 0x00048000,
        0x0003e000, 0x00000000, 0x00000000, 0x00008000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000038, 0x8443b80e, 0x00203a03,
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
    
    Color *imagePixels = (Color *)malloc(imWidth*imHeight*sizeof(Color));

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

    //FILE *myimage = fopen("default_font.raw", "wb");
    //fwrite(image.pixels, 1, 128*128*4, myimage);
    //fclose(myimage);
    
    Image image = LoadImageEx(imagePixels, imWidth, imHeight);
    ImageFormat(&image, UNCOMPRESSED_GRAY_ALPHA);

    free(imagePixels);

    defaultFont.texture = LoadTextureFromImage(image);
    UnloadImage(image);

    // Reconstruct charSet using charsWidth[], charsHeight, charsDivisor, numChars
    //------------------------------------------------------------------------------
    defaultFont.charValues = (int *)malloc(defaultFont.numChars*sizeof(int)); 
    defaultFont.charRecs = (Rectangle *)malloc(defaultFont.numChars*sizeof(Rectangle));   // Allocate space for our character rectangle data
                                                                                          // This memory should be freed at end! --> Done on CloseWindow()
    
    defaultFont.charOffsets = (Vector2 *)malloc(defaultFont.numChars*sizeof(Vector2));
    defaultFont.charAdvanceX = (int *)malloc(defaultFont.numChars*sizeof(int));
    
    int currentLine = 0;
    int currentPosX = charsDivisor;
    int testPosX = charsDivisor;

    for (int i = 0; i < defaultFont.numChars; i++)
    {
        defaultFont.charValues[i] = FONT_FIRST_CHAR + i;  // First char is 32
        
        defaultFont.charRecs[i].x = currentPosX;
        defaultFont.charRecs[i].y = charsDivisor + currentLine * (charsHeight + charsDivisor);
        defaultFont.charRecs[i].width = charsWidth[i];
        defaultFont.charRecs[i].height = charsHeight;

        testPosX += (defaultFont.charRecs[i].width + charsDivisor);

        if (testPosX >= defaultFont.texture.width)
        {
            currentLine++;
            currentPosX = 2*charsDivisor + charsWidth[i];
            testPosX = currentPosX;

            defaultFont.charRecs[i].x = charsDivisor;
            defaultFont.charRecs[i].y = charsDivisor + currentLine*(charsHeight + charsDivisor);
        }
        else currentPosX = testPosX;
        
        // NOTE: On default font character offsets and xAdvance are not required
        defaultFont.charOffsets[i] = (Vector2){ 0.0f, 0.0f };
        defaultFont.charAdvanceX[i] = 0;
    }
    
    defaultFont.size = defaultFont.charRecs[0].height;

    TraceLog(INFO, "[TEX ID %i] Default font loaded successfully", defaultFont.texture.id);
}

extern void UnloadDefaultFont(void)
{
    UnloadTexture(defaultFont.texture);
    free(defaultFont.charValues);
    free(defaultFont.charRecs);
    free(defaultFont.charOffsets);
    free(defaultFont.charAdvanceX);
}

// Get the default font, useful to be used with extended parameters
SpriteFont GetDefaultFont()
{
    return defaultFont;
}

// Load a SpriteFont image into GPU memory
SpriteFont LoadSpriteFont(const char *fileName)
{
    SpriteFont spriteFont = { 0 };

    // Check file extension
    if (strcmp(GetExtension(fileName),"rbmf") == 0) spriteFont = LoadRBMF(fileName);
    else if (strcmp(GetExtension(fileName),"ttf") == 0) spriteFont = LoadTTF(fileName, 32);
    else if (strcmp(GetExtension(fileName),"fnt") == 0) spriteFont = LoadBMFont(fileName);
    else
    {
        Image image = LoadImage(fileName);
        if (image.data != NULL) spriteFont = LoadImageFont(image, MAGENTA, 32);
        UnloadImage(image);
    }
    
    if (spriteFont.texture.id == 0)
    {
        TraceLog(WARNING, "[%s] SpriteFont could not be loaded, using default font", fileName);
        spriteFont = GetDefaultFont();
    }

    return spriteFont;
}

// Unload SpriteFont from GPU memory
void UnloadSpriteFont(SpriteFont spriteFont)
{
    // NOTE: Make sure spriteFont is not default font (fallback)
    if (spriteFont.texture.id != defaultFont.texture.id)
    {
        UnloadTexture(spriteFont.texture);
        free(spriteFont.charValues);
        free(spriteFont.charRecs);
        free(spriteFont.charOffsets);
        free(spriteFont.charAdvanceX);

        TraceLog(DEBUG, "Unloaded sprite font data");
    }
}

// Draw text (using default font)
// NOTE: fontSize work like in any drawing program but if fontSize is lower than font-base-size, then font-base-size is used
// NOTE: chars spacing is proportional to fontSize
void DrawText(const char *text, int posX, int posY, int fontSize, Color color)
{
    Vector2 position = { (float)posX, (float)posY };

    int defaultFontSize = 10;   // Default Font chars height in pixel
    if (fontSize < defaultFontSize) fontSize = defaultFontSize;
    int spacing = fontSize / defaultFontSize;

    DrawTextEx(defaultFont, text, position, fontSize, spacing, color);
}

// Draw text using SpriteFont
// NOTE: chars spacing is NOT proportional to fontSize
void DrawTextEx(SpriteFont spriteFont, const char *text, Vector2 position, int fontSize, int spacing, Color tint)
{
    int length = strlen(text);
    int textOffsetX = 0;
    int textOffsetY = 0;    // Line break!
    float scaleFactor;
    unsigned char letter;

    Rectangle rec;

    scaleFactor = (float)fontSize/spriteFont.size;

    // NOTE: Some ugly hacks are made to support Latin-1 Extended characters directly 
    // written in C code files (codified by default as UTF-8)
    
    for(int i = 0; i < length; i++)
    {
        // TODO: Right now we are supposing characters that follow a continous order and start at FONT_FIRST_CHAR,
        // this sytem can be improved to support any characters order and init value...
        // An intermediate table could be created to link char values with predefined char position index in chars rectangle array
        
        if ((unsigned char)text[i] == 0xc2)         // UTF-8 encoding identification HACK!
        {
            // Support UTF-8 encoded values from [0xc2 0x80] -> [0xc2 0xbf](¿)
            letter = (unsigned char)text[i + 1];
            rec = spriteFont.charRecs[letter - FONT_FIRST_CHAR];
            i++;
        }
        else if ((unsigned char)text[i] == 0xc3)    // UTF-8 encoding identification HACK!
        {
            // Support UTF-8 encoded values from [0xc3 0x80](À) -> [0xc3 0xbf](ÿ)
            letter = (unsigned char)text[i + 1];
            rec = spriteFont.charRecs[letter - FONT_FIRST_CHAR + 64];
            i++;
        }
        else
        {
            if ((unsigned char)text[i] == '\n')
            {
                // NOTE: Fixed line spacing of 1.5 lines
                textOffsetY += ((spriteFont.size + spriteFont.size/2)*scaleFactor);
                textOffsetX = 0;
                rec.x = -1;
            }
            else rec = spriteFont.charRecs[(int)text[i] - FONT_FIRST_CHAR];
        }

        if (rec.x > 0)
        {
            DrawTexturePro(spriteFont.texture, rec, (Rectangle){ position.x + textOffsetX + spriteFont.charOffsets[(int)text[i] - FONT_FIRST_CHAR].x*scaleFactor, 
                                                                 position.y + textOffsetY + spriteFont.charOffsets[(int)text[i] - FONT_FIRST_CHAR].y*scaleFactor, 
                                                                 rec.width*scaleFactor, rec.height*scaleFactor} , (Vector2){ 0, 0 }, 0.0f, tint);

            if (spriteFont.charAdvanceX[(int)text[i] - FONT_FIRST_CHAR] == 0) textOffsetX += (rec.width*scaleFactor + spacing);
            else textOffsetX += (spriteFont.charAdvanceX[(int)text[i] - FONT_FIRST_CHAR]*scaleFactor + spacing);
        }
    }
}

// Formatting of text with variables to 'embed'
const char *FormatText(const char *text, ...)
{
    static char buffer[MAX_FORMATTEXT_LENGTH];

    va_list args;
    va_start(args, text);
    vsprintf(buffer, text, args);
    va_end(args);

    return buffer;
}

// Get a piece of a text string
const char *SubText(const char *text, int position, int length)
{
    static char buffer[MAX_SUBTEXT_LENGTH];
    int textLength = strlen(text);
    
    if (position >= textLength)
    {
        position = textLength - 1;
        length = 0;
    }
    
    if (length >= textLength) length = textLength;
 
    for (int c = 0 ; c < length ; c++)
    {
        *(buffer+c) = *(text+position);
        text++;
    }

    *(buffer+length) = '\0';

    return buffer;
}

// Measure string width for default font
int MeasureText(const char *text, int fontSize)
{
    Vector2 vec;

    int defaultFontSize = 10;   // Default Font chars height in pixel
    if (fontSize < defaultFontSize) fontSize = defaultFontSize;
    int spacing = fontSize / defaultFontSize;

    vec = MeasureTextEx(defaultFont, text, fontSize, spacing);

    return (int)vec.x;
}

// Measure string size for SpriteFont
Vector2 MeasureTextEx(SpriteFont spriteFont, const char *text, int fontSize, int spacing)
{
    int len = strlen(text);
    int tempLen = 0;            // Used to count longer text line num chars
    int lenCounter = 0;
    
    int textWidth = 0;
    int tempTextWidth = 0;      // Used to count longer text line width
    
    int textHeight = spriteFont.size;
    float scaleFactor;

    for (int i = 0; i < len; i++)
    {
        lenCounter++;
        
        if (text[i] != '\n')
        {
            if (spriteFont.charAdvanceX[(int)text[i] - FONT_FIRST_CHAR] != 0) textWidth += spriteFont.charAdvanceX[(int)text[i] - FONT_FIRST_CHAR];
            else textWidth += (spriteFont.charRecs[(int)text[i] - FONT_FIRST_CHAR].width + spriteFont.charOffsets[(int)text[i] - FONT_FIRST_CHAR].x);
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            lenCounter = 0;
            textWidth = 0;
            textHeight += (spriteFont.size + spriteFont.size/2); // NOTE: Fixed line spacing of 1.5 lines
        }
        
        if (tempLen < lenCounter) tempLen = lenCounter;
    }
    
    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    if (fontSize <= spriteFont.size) scaleFactor = 1.0f;
    else scaleFactor = (float)fontSize/spriteFont.size;

    Vector2 vec;
    vec.x = (float)tempTextWidth*scaleFactor + (tempLen - 1)*spacing; // Adds chars spacing to measure
    vec.y = (float)textHeight*scaleFactor;

    return vec;
}

// Shows current FPS on top-left corner
// NOTE: Uses default font
void DrawFPS(int posX, int posY)
{
    char buffer[20];

    // NOTE: We are rendering fps every second for better viewing on high framerates
    // TODO: Not working properly on ANDROID and RPI

    static float fps = 0.0f;
    static int counter = 0;
    static int refreshRate = 20;

    if (counter < refreshRate)
    {
        counter++;
    }
    else
    {
        fps = GetFPS();
        refreshRate = fps;
        counter = 0;
    }

    sprintf(buffer, "%2.0f FPS", fps);
    DrawText(buffer, posX, posY, 20, LIME);
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Load a Image font file (XNA style)
static SpriteFont LoadImageFont(Image image, Color key, int firstChar)
{
    #define COLOR_EQUAL(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))
    
    int charSpacing = 0;
    int lineSpacing = 0;

    int x = 0;
    int y = 0;

    // We allocate a temporal arrays for chars data measures, 
    // once we get the actual number of chars, we copy data to a sized arrays
    int tempCharValues[MAX_FONTCHARS];
    Rectangle tempCharRecs[MAX_FONTCHARS];
    
    Color *pixels = GetImageData(image);

    // Parse image data to get charSpacing and lineSpacing
    for(y = 0; y < image.height; y++)
    {
        for(x = 0; x < image.width; x++)
        {
            if (!COLOR_EQUAL(pixels[y*image.width + x], key)) break;
        }
        if (!COLOR_EQUAL(pixels[y*image.width + x], key)) break;
    }

    charSpacing = x;
    lineSpacing = y;

    int charHeight = 0;
    int j = 0;

    while(!COLOR_EQUAL(pixels[(lineSpacing + j)*image.width + charSpacing], key)) j++;

    charHeight = j;

    // Check array values to get characters: value, x, y, w, h
    int index = 0;
    int lineToRead = 0;
    int xPosToRead = charSpacing;

    // Parse image data to get rectangle sizes
    while((lineSpacing + lineToRead * (charHeight + lineSpacing)) < image.height)
    {
        while((xPosToRead < image.width) &&
              !COLOR_EQUAL((pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead]), key))
        {
            tempCharValues[index] = firstChar + index;
            
            tempCharRecs[index].x = xPosToRead;
            tempCharRecs[index].y = lineSpacing + lineToRead * (charHeight + lineSpacing);
            tempCharRecs[index].height = charHeight;

            int charWidth = 0;

            while(!COLOR_EQUAL(pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead + charWidth], key)) charWidth++;

            tempCharRecs[index].width = charWidth;

            index++;

            xPosToRead += (charWidth + charSpacing);
        }

        lineToRead++;
        xPosToRead = charSpacing;
    }
    
    free(pixels);
    
    TraceLog(DEBUG, "SpriteFont data parsed correctly from image");
    
    // Create spritefont with all data parsed from image
    SpriteFont spriteFont = { 0 };
    
    spriteFont.texture = LoadTextureFromImage(image); // Convert loaded image to OpenGL texture
    spriteFont.numChars = index;

    // We got tempCharValues and tempCharsRecs populated with chars data
    // Now we move temp data to sized charValues and charRecs arrays
    spriteFont.charRecs = (Rectangle *)malloc(spriteFont.numChars*sizeof(Rectangle));
    spriteFont.charValues = (int *)malloc(spriteFont.numChars*sizeof(int));
    spriteFont.charOffsets = (Vector2 *)malloc(spriteFont.numChars*sizeof(Vector2));
    spriteFont.charAdvanceX = (int *)malloc(spriteFont.numChars*sizeof(int));

    for (int i = 0; i < spriteFont.numChars; i++)
    {
        spriteFont.charValues[i] = tempCharValues[i];
        spriteFont.charRecs[i] = tempCharRecs[i];
        
        // NOTE: On image based fonts (XNA style), character offsets and xAdvance are not required (set to 0)
        spriteFont.charOffsets[i] = (Vector2){ 0.0f, 0.0f };
        spriteFont.charAdvanceX[i] = 0;
    }
    
    spriteFont.size = spriteFont.charRecs[0].height;

    return spriteFont;
}

// Load a rBMF font file (raylib BitMap Font)
static SpriteFont LoadRBMF(const char *fileName)
{
    // rBMF Info Header (16 bytes)
    typedef struct {
        char id[4];             // rBMF file identifier
        char version;           // rBMF file version
                                //      4 MSB --> main version
                                //      4 LSB --> subversion
        char firstChar;         // First character in the font
                                // NOTE: Depending on charDataType, it could be useless
        short imgWidth;         // Image width - always POT (power-of-two)
        short imgHeight;        // Image height - always POT (power-of-two)
        short numChars;         // Number of characters contained
        short charHeight;       // Characters height - the same for all characters
        char compType;          // Compression type:
                                //      4 MSB --> image data compression
                                //      4 LSB --> chars data compression
        char charsDataType;     // Char data type provided
    } rbmfInfoHeader;

    SpriteFont spriteFont = { 0 };

    rbmfInfoHeader rbmfHeader;
    unsigned int *rbmfFileData = NULL;
    unsigned char *rbmfCharWidthData = NULL;

    int charsDivisor = 1;    // Every char is separated from the consecutive by a 1 pixel divisor, horizontally and vertically

    FILE *rbmfFile = fopen(fileName, "rb");        // Define a pointer to bitmap file and open it in read-binary mode

    if (rbmfFile == NULL)
    {
        TraceLog(WARNING, "[%s] rBMF font file could not be opened, using default font", fileName);
        
        spriteFont = GetDefaultFont();
    }
    else
    {
        fread(&rbmfHeader, sizeof(rbmfInfoHeader), 1, rbmfFile);

        TraceLog(DEBUG, "[%s] Loading rBMF file, size: %ix%i, numChars: %i, charHeight: %i", fileName, rbmfHeader.imgWidth, rbmfHeader.imgHeight, rbmfHeader.numChars, rbmfHeader.charHeight);

        spriteFont.numChars = (int)rbmfHeader.numChars;

        int numPixelBits = rbmfHeader.imgWidth * rbmfHeader.imgHeight / 32;

        rbmfFileData = (unsigned int *)malloc(numPixelBits * sizeof(unsigned int));

        for(int i = 0; i < numPixelBits; i++) fread(&rbmfFileData[i], sizeof(unsigned int), 1, rbmfFile);

        rbmfCharWidthData = (unsigned char *)malloc(spriteFont.numChars * sizeof(unsigned char));

        for(int i = 0; i < spriteFont.numChars; i++) fread(&rbmfCharWidthData[i], sizeof(unsigned char), 1, rbmfFile);

        // Re-construct image from rbmfFileData
        //-----------------------------------------
        Color *imagePixels = (Color *)malloc(rbmfHeader.imgWidth*rbmfHeader.imgHeight*sizeof(Color));

        for (int i = 0; i < rbmfHeader.imgWidth*rbmfHeader.imgHeight; i++) imagePixels[i] = BLANK;        // Initialize array

        int counter = 0;        // Font data elements counter

        // Fill image data (convert from bit to pixel!)
        for (int i = 0; i < rbmfHeader.imgWidth*rbmfHeader.imgHeight; i += 32)
        {
            for (int j = 31; j >= 0; j--)
            {
                if (BIT_CHECK(rbmfFileData[counter], j)) imagePixels[i+j] = WHITE;
            }

            counter++;
        }
        
        Image image = LoadImageEx(imagePixels, rbmfHeader.imgWidth, rbmfHeader.imgHeight);
        ImageFormat(&image, UNCOMPRESSED_GRAY_ALPHA);
        
        free(imagePixels);

        TraceLog(DEBUG, "[%s] Image reconstructed correctly, now converting it to texture", fileName);

        // Create spritefont with all data read from rbmf file
        spriteFont.texture = LoadTextureFromImage(image);
        UnloadImage(image);     // Unload image data

        //TraceLog(INFO, "[%s] Starting chars set reconstruction", fileName);

        // Get characters data using rbmfCharWidthData, rbmfHeader.charHeight, charsDivisor, rbmfHeader.numChars
        spriteFont.charValues = (int *)malloc(spriteFont.numChars*sizeof(int)); 
        spriteFont.charRecs = (Rectangle *)malloc(spriteFont.numChars*sizeof(Rectangle));
        spriteFont.charOffsets = (Vector2 *)malloc(spriteFont.numChars*sizeof(Vector2));
        spriteFont.charAdvanceX = (int *)malloc(spriteFont.numChars*sizeof(int));

        int currentLine = 0;
        int currentPosX = charsDivisor;
        int testPosX = charsDivisor;

        for (int i = 0; i < spriteFont.numChars; i++)
        {
            spriteFont.charValues[i] = (int)rbmfHeader.firstChar + i;
            
            spriteFont.charRecs[i].x = currentPosX;
            spriteFont.charRecs[i].y = charsDivisor + currentLine * ((int)rbmfHeader.charHeight + charsDivisor);
            spriteFont.charRecs[i].width = (int)rbmfCharWidthData[i];
            spriteFont.charRecs[i].height = (int)rbmfHeader.charHeight;
            
            // NOTE: On image based fonts (XNA style), character offsets and xAdvance are not required (set to 0)
            spriteFont.charOffsets[i] = (Vector2){ 0.0f, 0.0f };
            spriteFont.charAdvanceX[i] = 0;

            testPosX += (spriteFont.charRecs[i].width + charsDivisor);

            if (testPosX > spriteFont.texture.width)
            {
                currentLine++;
                currentPosX = 2 * charsDivisor + (int)rbmfCharWidthData[i];
                testPosX = currentPosX;

                spriteFont.charRecs[i].x = charsDivisor;
                spriteFont.charRecs[i].y = charsDivisor + currentLine * (rbmfHeader.charHeight + charsDivisor);
            }
            else currentPosX = testPosX;
        }
        
        spriteFont.size = spriteFont.charRecs[0].height;

        TraceLog(INFO, "[%s] rBMF file loaded correctly as SpriteFont", fileName);
    }

    fclose(rbmfFile);

    free(rbmfFileData);                // Now we can free loaded data from RAM memory
    free(rbmfCharWidthData);

    return spriteFont;
}

// Load a BMFont file (AngelCode font file)
static SpriteFont LoadBMFont(const char *fileName)
{
    #define MAX_BUFFER_SIZE     256
    
    SpriteFont font = { 0 };
    font.texture.id = 0;
    
    char buffer[MAX_BUFFER_SIZE];
    char *searchPoint = NULL;
    
    int fontSize = 0;
    int texWidth, texHeight;
    char texFileName[128];
    int numChars = 0;

    int base;   // Useless data
    
    FILE *fntFile;

    fntFile = fopen(fileName, "rt");

    if (fntFile == NULL)
    {
        TraceLog(WARNING, "[%s] FNT file could not be opened", fileName);
        return font;
    }

    // NOTE: We skip first line, it contains no useful information
    fgets(buffer, MAX_BUFFER_SIZE, fntFile);
    //searchPoint = strstr(buffer, "size");
    //sscanf(searchPoint, "size=%i", &fontSize);
    
    fgets(buffer, MAX_BUFFER_SIZE, fntFile);
    searchPoint = strstr(buffer, "lineHeight");
    sscanf(searchPoint, "lineHeight=%i base=%i scaleW=%i scaleH=%i", &fontSize, &base, &texWidth, &texHeight);
    
    TraceLog(DEBUG, "[%s] Font size: %i", fileName, fontSize);
    TraceLog(DEBUG, "[%s] Font texture scale: %ix%i", fileName, texWidth, texHeight);
    
    fgets(buffer, MAX_BUFFER_SIZE, fntFile);
    searchPoint = strstr(buffer, "file");
    sscanf(searchPoint, "file=\"%128[^\"]\"", texFileName);
    
    TraceLog(DEBUG, "[%s] Font texture filename: %s", fileName, texFileName);
    
    fgets(buffer, MAX_BUFFER_SIZE, fntFile);
    searchPoint = strstr(buffer, "count");
    sscanf(searchPoint, "count=%i", &numChars);
    
    TraceLog(DEBUG, "[%s] Font num chars: %i", fileName, numChars);
    
    // Compose correct path using route of .fnt file (fileName) and texFileName
    char *texPath = NULL;
    char *lastSlash = NULL;

    lastSlash = strrchr(fileName, '/');
    
    // NOTE: We need some extra space to avoid memory corruption on next allocations!
    texPath = malloc(strlen(fileName) - strlen(lastSlash) + strlen(texFileName) + 4);
    
    // NOTE: strcat() and strncat() required a '\0' terminated string to work!
    *texPath = '\0';
    strncat(texPath, fileName, strlen(fileName) - strlen(lastSlash) + 1);
    strncat(texPath, texFileName, strlen(texFileName));

    TraceLog(DEBUG, "[%s] Font texture loading path: %s", fileName, texPath);
    
    font.texture = LoadTexture(texPath);
    font.size = fontSize;
    font.numChars = numChars;
    font.charValues = (int *)malloc(numChars*sizeof(int));
    font.charRecs = (Rectangle *)malloc(numChars*sizeof(Rectangle));
    font.charOffsets = (Vector2 *)malloc(numChars*sizeof(Vector2));
    font.charAdvanceX = (int *)malloc(numChars*sizeof(int));
    
    free(texPath);
    
    int charId, charX, charY, charWidth, charHeight, charOffsetX, charOffsetY, charAdvanceX;
    
    bool unorderedChars = false;
    int firstChar = 0;
    
    for (int i = 0; i < numChars; i++)
    {
        fgets(buffer, MAX_BUFFER_SIZE, fntFile);
        sscanf(buffer, "char id=%i x=%i y=%i width=%i height=%i xoffset=%i yoffset=%i xadvance=%i", 
                       &charId, &charX, &charY, &charWidth, &charHeight, &charOffsetX, &charOffsetY, &charAdvanceX);
                       
        if (i == 0) firstChar = charId;
        else if (i != (charId - firstChar)) unorderedChars = true;
        
        // Save data properly in sprite font
        font.charValues[i] = charId;
        font.charRecs[i] = (Rectangle){ charX, charY, charWidth, charHeight };
        font.charOffsets[i] = (Vector2){ (float)charOffsetX, (float)charOffsetY };
        font.charAdvanceX[i] = charAdvanceX;
    }
    
    fclose(fntFile);
    
    if (firstChar != FONT_FIRST_CHAR) TraceLog(WARNING, "BMFont not supported: expected SPACE(32) as first character, falling back to default font");
    else if (unorderedChars) TraceLog(WARNING, "BMFont not supported: unordered chars data, falling back to default font");
    
    // NOTE: Font data could be not ordered by charId: 32,33,34,35... raylib does not support unordered BMFonts
    if ((firstChar != FONT_FIRST_CHAR) || (unorderedChars) || (font.texture.id == 0))
    {
        UnloadSpriteFont(font);
        font = GetDefaultFont();
    }
    else TraceLog(INFO, "[%s] SpriteFont loaded successfully", fileName);

    return font;
}

// Generate a sprite font from TTF file data (font size required)
// TODO: Review texture packing method and generation (use oversampling)
static SpriteFont LoadTTF(const char *fileName, int fontSize)
{
    // NOTE: Generated font uses some hardcoded values
    #define FONT_TEXTURE_WIDTH      512     // Font texture width
    #define FONT_TEXTURE_HEIGHT     512     // Font texture height
    #define FONT_FIRST_CHAR          32     // Font first character (32 - space)
    #define FONT_NUM_CHARS           95     // ASCII 32..126 is 95 glyphs

    unsigned char *ttfBuffer = (unsigned char *)malloc(1 << 25);
    unsigned char *dataBitmap = (unsigned char *)malloc(FONT_TEXTURE_WIDTH*FONT_TEXTURE_HEIGHT*sizeof(unsigned char));   // One channel bitmap returned!
    stbtt_bakedchar charData[FONT_NUM_CHARS];  // ASCII 32..126 is 95 glyphs

    SpriteFont font = { 0 };
    
    FILE *ttfFile = fopen(fileName, "rb");
    
    if (ttfFile == NULL)
    {
        TraceLog(WARNING, "[%s] FNT file could not be opened", fileName);
        return font;
    }

    fread(ttfBuffer, 1, 1<<25, ttfFile);

    // NOTE: Using stb_truetype crappy packing method, no guarante the font fits the image...
    stbtt_BakeFontBitmap(ttfBuffer,0, fontSize, dataBitmap, FONT_TEXTURE_WIDTH, FONT_TEXTURE_HEIGHT, FONT_FIRST_CHAR, FONT_NUM_CHARS, charData);

    free(ttfBuffer);
    
    // Convert image data from grayscale to to UNCOMPRESSED_GRAY_ALPHA
    unsigned char *dataGrayAlpha = (unsigned char *)malloc(FONT_TEXTURE_WIDTH*FONT_TEXTURE_HEIGHT*sizeof(unsigned char)*2); // Two channels
    int k = 0;
    
    for (int i = 0; i < FONT_TEXTURE_WIDTH*FONT_TEXTURE_HEIGHT; i++)
    {
        dataGrayAlpha[k] = 255;
        dataGrayAlpha[k + 1] = dataBitmap[i];

        k += 2;
    }
    
    free(dataBitmap);
    
    // Sprite font generation from TTF extracted data
    Image image;
    image.width = FONT_TEXTURE_WIDTH;
    image.height = FONT_TEXTURE_HEIGHT;
    image.mipmaps = 1;
    image.format = UNCOMPRESSED_GRAY_ALPHA;
    image.data = dataGrayAlpha;

    font.texture = LoadTextureFromImage(image);
    UnloadImage(image);     // Unloads dataGrayAlpha

    font.size = fontSize;
    font.numChars = FONT_NUM_CHARS;
    font.charValues = (int *)malloc(font.numChars*sizeof(int));
    font.charRecs = (Rectangle *)malloc(font.numChars*sizeof(Rectangle));
    font.charOffsets = (Vector2 *)malloc(font.numChars*sizeof(Vector2));
    font.charAdvanceX = (int *)malloc(font.numChars*sizeof(int));
    
    for (int i = 0; i < font.numChars; i++)
    {
        font.charValues[i] = i + FONT_FIRST_CHAR;

        font.charRecs[i].x = (int)charData[i].x0;
        font.charRecs[i].y = (int)charData[i].y0;
        font.charRecs[i].width = (int)charData[i].x1 - (int)charData[i].x0;
        font.charRecs[i].height = (int)charData[i].y1 - (int)charData[i].y0;
        
        font.charOffsets[i] = (Vector2){ charData[i].xoff, charData[i].yoff };
        font.charAdvanceX[i] = (int)charData[i].xadvance;
    }

    return font;
}