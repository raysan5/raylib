/**********************************************************************************************
*
*   raylib.textures
*
*   Basic functions to load and draw Textures (2d)
*
*   Uses external lib:
*       stb_image - Multiple formats image loading (JPEG, PNG, BMP, TGA, PSD, GIF, PIC)
*                   NOTE: stb_image has been slightly modified, original library: https://github.com/nothings/stb
*
*   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
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

#include <stdlib.h>          // Declares malloc() and free() for memory management
#include <string.h>          // Required for strcmp(), strrchr(), strncmp()

#include "rlgl.h"            // raylib OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2
#include "utils.h"           // rRES data decompression utility function
                             // NOTE: Includes Android fopen function map

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"       // Used to read image data (multiple formats support)

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    unsigned char *data;    // Image raw data
    int width;              // Image base width
    int height;             // Image base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format
} ImageEx;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// It's lonely here...

//----------------------------------------------------------------------------------
// Other Modules Functions Declaration (required by text)
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static ImageEx LoadDDS(const char *fileName);   // Load DDS file
static ImageEx LoadPKM(const char *fileName);   // Load PKM file
static ImageEx LoadPVR(const char *fileName);   // Load PVR file

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Load an image into CPU memory (RAM)
Image LoadImage(const char *fileName)
{
    Image image;

    // Initial values
    image.pixels = NULL;
    image.width = 0;
    image.height = 0;

    if ((strcmp(GetExtension(fileName),"png") == 0) ||
        (strcmp(GetExtension(fileName),"bmp") == 0) ||
        (strcmp(GetExtension(fileName),"tga") == 0) ||
        (strcmp(GetExtension(fileName),"jpg") == 0) ||
        (strcmp(GetExtension(fileName),"gif") == 0) ||
        (strcmp(GetExtension(fileName),"psd") == 0) ||
        (strcmp(GetExtension(fileName),"pic") == 0))
    {
        int imgWidth;
        int imgHeight;
        int imgBpp;

        // NOTE: Using stb_image to load images (Supports: BMP, TGA, PNG, JPG, ...)
        // Force loading to 4 components (RGBA)
        byte *imgData = stbi_load(fileName, &imgWidth, &imgHeight, &imgBpp, 4);

        if (imgData != NULL)
        {
            // Convert array to pixel array for working convenience
            image.pixels = (Color *)malloc(imgWidth * imgHeight * sizeof(Color));

            int pix = 0;

            for (int i = 0; i < (imgWidth * imgHeight * 4); i += 4)
            {
                image.pixels[pix].r = imgData[i];
                image.pixels[pix].g = imgData[i+1];
                image.pixels[pix].b = imgData[i+2];
                image.pixels[pix].a = imgData[i+3];
                pix++;
            }

            stbi_image_free(imgData);

            image.width = imgWidth;
            image.height = imgHeight;

            TraceLog(INFO, "[%s] Image loaded successfully (%ix%i)", fileName, image.width, image.height);
        }
        else TraceLog(WARNING, "[%s] Image could not be loaded, file not recognized", fileName);
    }
    else if (strcmp(GetExtension(fileName),"dds") == 0)
    {
        // NOTE: DDS uncompressed images can also be loaded (discarding mipmaps...)

        ImageEx imageDDS = LoadDDS(fileName);

        if (imageDDS.format == 0)
        {
            image.pixels = (Color *)malloc(imageDDS.width * imageDDS.height * sizeof(Color));
            image.width = imageDDS.width;
            image.height = imageDDS.height;

            int pix = 0;

            for (int i = 0; i < (image.width * image.height * 4); i += 4)
            {
                image.pixels[pix].r = imageDDS.data[i];
                image.pixels[pix].g = imageDDS.data[i+1];
                image.pixels[pix].b = imageDDS.data[i+2];
                image.pixels[pix].a = imageDDS.data[i+3];
                pix++;
            }

            free(imageDDS.data);

            TraceLog(INFO, "[%s] DDS Image loaded successfully (uncompressed, no mipmaps)", fileName);
        }
        else TraceLog(WARNING, "[%s] DDS Compressed image data could not be loaded", fileName);
    }
    else if (strcmp(GetExtension(fileName),"pkm") == 0)
    {
        TraceLog(INFO, "[%s] PKM Compressed image data could not be loaded", fileName);
    }
    else TraceLog(WARNING, "[%s] Image extension not recognized, it can't be loaded", fileName);

    // ALTERNATIVE: We can load pixel data directly into Color struct pixels array,
    // to do that, struct data alignment should be the right one (4 byte); it is.
    //image.pixels = stbi_load(fileName, &imgWidth, &imgHeight, &imgBpp, 4);

    return image;
}

// Load an image from rRES file (raylib Resource)
Image LoadImageFromRES(const char *rresName, int resId)
{
    Image image;
    bool found = false;

    char id[4];             // rRES file identifier
    unsigned char version;  // rRES file version and subversion
    char useless;           // rRES header reserved data
    short numRes;

    ResInfoHeader infoHeader;

    FILE *rresFile = fopen(rresName, "rb");

    if (rresFile == NULL)
    {
        TraceLog(WARNING, "[%s] rRES raylib resource file could not be opened", rresName);
    }
    else
    {
        // Read rres file (basic file check - id)
        fread(&id[0], sizeof(char), 1, rresFile);
        fread(&id[1], sizeof(char), 1, rresFile);
        fread(&id[2], sizeof(char), 1, rresFile);
        fread(&id[3], sizeof(char), 1, rresFile);
        fread(&version, sizeof(char), 1, rresFile);
        fread(&useless, sizeof(char), 1, rresFile);

        if ((id[0] != 'r') && (id[1] != 'R') && (id[2] != 'E') &&(id[3] != 'S'))
        {
            TraceLog(WARNING, "[%s] This is not a valid raylib resource file", rresName);
        }
        else
        {
            // Read number of resources embedded
            fread(&numRes, sizeof(short), 1, rresFile);

            for (int i = 0; i < numRes; i++)
            {
                fread(&infoHeader, sizeof(ResInfoHeader), 1, rresFile);

                if (infoHeader.id == resId)
                {
                    found = true;

                    // Check data is of valid IMAGE type
                    if (infoHeader.type == 0)   // IMAGE data type
                    {
                        // TODO: Check data compression type
                        // NOTE: We suppose compression type 2 (DEFLATE - default)

                        short imgWidth, imgHeight;
                        char colorFormat, mipmaps;

                        fread(&imgWidth, sizeof(short), 1, rresFile);   // Image width
                        fread(&imgHeight, sizeof(short), 1, rresFile);  // Image height
                        fread(&colorFormat, 1, 1, rresFile);            // Image data color format (default: RGBA 32 bit)
                        fread(&mipmaps, 1, 1, rresFile);                // Mipmap images included (default: 0)

                        image.width = (int)imgWidth;
                        image.height = (int)imgHeight;

                        unsigned char *data = malloc(infoHeader.size);

                        fread(data, infoHeader.size, 1, rresFile);

                        unsigned char *imgData = DecompressData(data, infoHeader.size, infoHeader.srcSize);

                        image.pixels = (Color *)malloc(sizeof(Color)*imgWidth*imgHeight);

                        int pix = 0;

                        for (int i = 0; i < (imgWidth*imgHeight*4); i += 4)
                        {
                            image.pixels[pix].r = imgData[i];
                            image.pixels[pix].g = imgData[i+1];
                            image.pixels[pix].b = imgData[i+2];
                            image.pixels[pix].a = imgData[i+3];
                            pix++;
                        }

                        free(imgData);

                        free(data);

                        TraceLog(INFO, "[%s] Image loaded successfully from resource, size: %ix%i", rresName, image.width, image.height);
                    }
                    else
                    {
                        TraceLog(WARNING, "[%s] Required resource do not seem to be a valid IMAGE resource", rresName);
                    }
                }
                else
                {
                    // Depending on type, skip the right amount of parameters
                    switch (infoHeader.type)
                    {
                        case 0: fseek(rresFile, 6, SEEK_CUR); break;    // IMAGE: Jump 6 bytes of parameters
                        case 1: fseek(rresFile, 6, SEEK_CUR); break;    // SOUND: Jump 6 bytes of parameters
                        case 2: fseek(rresFile, 5, SEEK_CUR); break;    // MODEL: Jump 5 bytes of parameters (TODO: Review)
                        case 3: break;                                  // TEXT: No parameters
                        case 4: break;                                  // RAW: No parameters
                        default: break;
                    }

                    // Jump DATA to read next infoHeader
                    fseek(rresFile, infoHeader.size, SEEK_CUR);
                }
            }
        }

        fclose(rresFile);
    }

    if (!found) TraceLog(WARNING, "[%s] Required resource id [%i] could not be found in the raylib resource file", rresName, resId);

    return image;
}

// Load an image as texture into GPU memory
Texture2D LoadTexture(const char *fileName)
{
    Texture2D texture;

    // Init texture to default values
    texture.id = 0;
    texture.width = 0;
    texture.height = 0;

    if (strcmp(GetExtension(fileName),"dds") == 0)
    {
        ImageEx image = LoadDDS(fileName);

        texture.id = rlglLoadTexture(image.data, image.width, image.height, image.format, image.mipmaps, false);

        texture.width = image.width;
        texture.height = image.height;

        if (texture.id == 0) TraceLog(WARNING, "[%s] DDS texture could not be loaded", fileName);
        else TraceLog(INFO, "[%s] DDS texture loaded successfully", fileName);

        free(image.data);
    }
    else if (strcmp(GetExtension(fileName),"pkm") == 0)
    {
        ImageEx image = LoadPKM(fileName);

        texture.id = rlglLoadTexture(image.data, image.width, image.height, image.format, image.mipmaps, false);

        texture.width = image.width;
        texture.height = image.height;

        if (texture.id == 0) TraceLog(WARNING, "[%s] PKM texture could not be loaded", fileName);
        else TraceLog(INFO, "[%s] PKM texture loaded successfully", fileName);

        free(image.data);
    }
    else if (strcmp(GetExtension(fileName),"pvr") == 0)
    {
        ImageEx image = LoadPVR(fileName);
        
        texture.id = rlglLoadTexture(image.data, image.width, image.height, image.format, image.mipmaps, false);

        texture.width = image.width;
        texture.height = image.height;

        if (texture.id == 0) TraceLog(WARNING, "[%s] PVR texture could not be loaded", fileName);
        else TraceLog(INFO, "[%s] PVR texture loaded successfully", fileName);

        free(image.data);
    }
    else
    {
        Image image = LoadImage(fileName);

        if (image.pixels != NULL)
        {
#if defined(PLATFORM_RPI) || defined(PLATFORM_WEB)
            ConvertToPOT(&image, BLANK);
#endif
            texture = LoadTextureFromImage(image, false);
            UnloadImage(image);
        }
    }

    return texture;
}

Texture2D LoadTextureEx(void *data, int width, int height, int textureFormat, int mipmapCount, bool genMipmaps)
{
    Texture2D texture;

    texture.width = width;
    texture.height = height;
    
    texture.id = rlglLoadTexture(data, width, height, textureFormat, mipmapCount, genMipmaps);
    
    return texture;
}

// Load a texture from image data
// NOTE: image is not unloaded, it must be done manually
Texture2D LoadTextureFromImage(Image image, bool genMipmaps)
{
    Texture2D texture;

    // Init texture to default values
    texture.id = 0;
    texture.width = 0;
    texture.height = 0;

    if ((image.pixels != NULL) && (image.width > 0) && (image.height > 0))
    {
        unsigned char *imgData = malloc(image.width * image.height * 4);

        int j = 0;

        for (int i = 0; i < image.width * image.height * 4; i += 4)
        {
            imgData[i] = image.pixels[j].r;
            imgData[i+1] = image.pixels[j].g;
            imgData[i+2] = image.pixels[j].b;
            imgData[i+3] = image.pixels[j].a;

            j++;
        }

        // NOTE: rlglLoadTexture() can generate mipmaps (POT image required)
        texture.id = rlglLoadTexture(imgData, image.width, image.height, UNCOMPRESSED_R8G8B8A8, 1, genMipmaps);

        texture.width = image.width;
        texture.height = image.height;

        free(imgData);
    }
    else TraceLog(WARNING, "Texture could not be loaded, image data is not valid");

    return texture;
}

// [DEPRECATED] Load a texture from image data
// NOTE: Use LoadTextureFromImage() instead
Texture2D CreateTexture(Image image, bool genMipmaps)
{
    Texture2D texture;

    texture = LoadTextureFromImage(image, genMipmaps);

    TraceLog(INFO, "Created texture id: %i", texture.id);

    return texture;
}

// Load an image as texture from rRES file (raylib Resource)
Texture2D LoadTextureFromRES(const char *rresName, int resId)
{
    Texture2D texture;

    Image image = LoadImageFromRES(rresName, resId);
    texture = LoadTextureFromImage(image, false);
    UnloadImage(image);

    return texture;
}

// Unload image from CPU memory (RAM)
void UnloadImage(Image image)
{
    free(image.pixels);
}

// Unload texture from GPU memory
void UnloadTexture(Texture2D texture)
{
    rlDeleteTextures(texture.id);
}

// Convert image to POT (power-of-two)
// NOTE: Requirement on OpenGL ES 2.0 (RPI, HTML5)
void ConvertToPOT(Image *image, Color fillColor)
{
    // Just add the required amount of pixels at the right and bottom sides of image...
    int potWidth = GetNextPOT(image->width);
    int potHeight = GetNextPOT(image->height);

    // Check if POT texture generation is required (if texture is not already POT)
    if ((potWidth != image->width) || (potHeight != image->height))
    {
        Color *imgDataPixelPOT = NULL;

        // Generate POT array from NPOT data
        imgDataPixelPOT = (Color *)malloc(potWidth * potHeight * sizeof(Color));

        for (int j = 0; j < potHeight; j++)
        {
            for (int i = 0; i < potWidth; i++)
            {
                if ((j < image->height) && (i < image->width)) imgDataPixelPOT[j*potWidth + i] = image->pixels[j*image->width + i];
                else imgDataPixelPOT[j*potWidth + i] = fillColor;
            }
        }

        TraceLog(WARNING, "Image converted to POT: (%ix%i) -> (%ix%i)", image->width, image->height, potWidth, potHeight);

        free(image->pixels);

        image->pixels = imgDataPixelPOT;
        image->width = potWidth;
        image->height = potHeight;
    }
}

// Draw a Texture2D
void DrawTexture(Texture2D texture, int posX, int posY, Color tint)
{
    DrawTextureEx(texture, (Vector2){ (float)posX, (float)posY }, 0, 1.0f, tint);
}

// Draw a Texture2D with position defined as Vector2
void DrawTextureV(Texture2D texture, Vector2 position, Color tint)
{
    DrawTextureEx(texture, position, 0, 1.0f, tint);
}

// Draw a Texture2D with extended parameters
void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint)
{
    Rectangle sourceRec = { 0, 0, texture.width, texture.height };
    Rectangle destRec = { (int)position.x, (int)position.y, texture.width*scale, texture.height*scale };
    Vector2 origin = { 0, 0 };

    DrawTexturePro(texture, sourceRec, destRec, origin, rotation, tint);
}

// Draw a part of a texture (defined by a rectangle)
void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint)
{
    Rectangle destRec = { (int)position.x, (int)position.y, sourceRec.width, sourceRec.height };
    Vector2 origin = { 0, 0 };

    DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, tint);
}

// Draw a part of a texture (defined by a rectangle) with 'pro' parameters
// NOTE: origin is relative to destination rectangle size
void DrawTexturePro(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector2 origin, float rotation, Color tint)
{
    rlEnableTexture(texture.id);

    rlPushMatrix();
        rlTranslatef(destRec.x, destRec.y, 0);
        rlRotatef(rotation, 0, 0, 1);
        rlTranslatef(-origin.x, -origin.y, 0);

        rlBegin(RL_QUADS);
            rlColor4ub(tint.r, tint.g, tint.b, tint.a);
            rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

            // Bottom-left corner for texture and quad
            rlTexCoord2f((float)sourceRec.x / texture.width, (float)sourceRec.y / texture.height);
            rlVertex2f(0.0f, 0.0f);

            // Bottom-right corner for texture and quad
            rlTexCoord2f((float)sourceRec.x / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height);
            rlVertex2f(0.0f, destRec.height);

            // Top-right corner for texture and quad
            rlTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height);
            rlVertex2f(destRec.width, destRec.height);

            // Top-left corner for texture and quad
            rlTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)sourceRec.y / texture.height);
            rlVertex2f(destRec.width, 0.0f);
        rlEnd();
    rlPopMatrix();

    rlDisableTexture();
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Loading DDS image data (compressed or uncompressed)
// NOTE: Compressed data loading not supported on OpenGL 1.1
static ImageEx LoadDDS(const char *fileName)
{
    #define FOURCC_DXT1 0x31545844  // Equivalent to "DXT1" in ASCII
    #define FOURCC_DXT3 0x33545844  // Equivalent to "DXT3" in ASCII
    #define FOURCC_DXT5 0x35545844  // Equivalent to "DXT5" in ASCII

    // DDS Pixel Format
    typedef struct {
        unsigned int size;
        unsigned int flags;
        unsigned int fourCC;
        unsigned int rgbBitCount;
        unsigned int rBitMask;
        unsigned int gBitMask;
        unsigned int bitMask;
        unsigned int aBitMask;
    } ddsPixelFormat;

    // DDS Header (124 bytes)
    typedef struct {
        unsigned int size;
        unsigned int flags;
        unsigned int height;
        unsigned int width;
        unsigned int pitchOrLinearSize;
        unsigned int depth;
        unsigned int mipMapCount;
        unsigned int reserved1[11];
        ddsPixelFormat ddspf;
        unsigned int caps;
        unsigned int caps2;
        unsigned int caps3;
        unsigned int caps4;
        unsigned int reserved2;
    } ddsHeader;

    ImageEx image;
    ddsHeader header;

    image.data = NULL;
    image.width = 0;
    image.height = 0;
    image.mipmaps = 0;
    image.format = 0;

    FILE *ddsFile = fopen(fileName, "rb");

    if (ddsFile == NULL)
    {
        TraceLog(WARNING, "[%s] DDS image file could not be opened", fileName);
    }
    else
    {
        // Verify the type of file
        char filecode[4];

        fread(filecode, 1, 4, ddsFile);

        if (strncmp(filecode, "DDS ", 4) != 0)
        {
            TraceLog(WARNING, "[%s] DDS file does not seem to be a valid image", fileName);
            fclose(ddsFile);
        }
        else
        {
            // Get the surface descriptor
            fread(&header, sizeof(ddsHeader), 1, ddsFile);

            TraceLog(DEBUG, "[%s] DDS file header size: %i", fileName, sizeof(ddsHeader));
            TraceLog(DEBUG, "[%s] DDS file pixel format size: %i", fileName, header.ddspf.size);
            TraceLog(DEBUG, "[%s] DDS file pixel format flags: 0x%x", fileName, header.ddspf.flags);
            TraceLog(DEBUG, "[%s] DDS file format: 0x%x", fileName, header.ddspf.fourCC);

            image.width = header.width;
            image.height = header.height;

            if (header.ddspf.flags == 0x40 && header.ddspf.rgbBitCount == 24)   // DDS_RGB, no compressed
            {
                image.data = (unsigned char *)malloc(header.width * header.height * 4);
                unsigned char *buffer = (unsigned char *)malloc(header.width * header.height * 3);

                fread(buffer, image.width*image.height*3, 1, ddsFile);

                unsigned char *src = buffer;
                unsigned char *dest = image.data;

                for(int y = 0; y < image.height; y++)
                {
                    for(int x = 0; x < image.width; x++)
                    {
                        *dest++ = *src++;
                        *dest++ = *src++;
                        *dest++ = *src++;
                        *dest++ = 255;
                    }
                }
                
                free(buffer);
                
                image.mipmaps = 1;
                image.format = UNCOMPRESSED_R8G8B8;
            }
            else if (header.ddspf.flags == 0x41 && header.ddspf.rgbBitCount == 32) // DDS_RGBA, no compressed
            {
                image.data = (unsigned char *)malloc(header.width * header.height * 4);

                fread(image.data, image.width*image.height*4, 1, ddsFile);

                image.mipmaps = 1;
                image.format = UNCOMPRESSED_R8G8B8A8;
            }
            else if (((header.ddspf.flags == 0x04) || (header.ddspf.flags == 0x05)) && (header.ddspf.fourCC > 0))
            {
                //TraceLog(WARNING, "[%s] DDS image uses compression, not supported on OpenGL 1.1", fileName);
                //TraceLog(WARNING, "[%s] DDS compressed files require OpenGL 3.2+ or ES 2.0", fileName);

                int bufsize;

                // Calculate data size, including all mipmaps
                if (header.mipMapCount > 1) bufsize = header.pitchOrLinearSize * 2;
                else bufsize = header.pitchOrLinearSize;
                
                TraceLog(DEBUG, "Pitch or linear size: %i", header.pitchOrLinearSize);

                image.data = (unsigned char*)malloc(bufsize * sizeof(unsigned char));

                fread(image.data, 1, bufsize, ddsFile);

                // Close file pointer
                fclose(ddsFile);

                image.mipmaps = header.mipMapCount;

                switch(header.ddspf.fourCC)
                {
                    case FOURCC_DXT1:
                    {
                        if (header.ddspf.flags == 0x04) image.format = COMPRESSED_DXT1_RGB;
                        else image.format = COMPRESSED_DXT1_RGBA;
                    } break;
                    case FOURCC_DXT3: image.format = COMPRESSED_DXT3_RGBA; break;
                    case FOURCC_DXT5: image.format = COMPRESSED_DXT5_RGBA; break;
                    default: break;
                }
            }
        }
    }

    return image;
}

// Loading PKM image data (ETC1/ETC2 compression)
// NOTE: KTX is the standard Khronos Group compression format (ETC1/ETC2, mipmaps)
// PKM is a much simpler file format used mainly to contain a single ETC1/ETC2 compressed image (no mipmaps)
// ETC1 compression support requires extension GL_OES_compressed_ETC1_RGB8_texture
static ImageEx LoadPKM(const char *fileName)
{
    // If OpenGL ES 2.0. the following format could be supported (ETC1):
    //GL_ETC1_RGB8_OES

    // If OpenGL ES 3.0, the following formats are supported (ETC2/EAC):
    //GL_COMPRESSED_RGB8_ETC2
    //GL_COMPRESSED_RGBA8_ETC2
    //GL_COMPRESSED_RG11_EAC
    //...

    // PKM file (ETC1) Header (16 bytes)
    typedef struct {
        char id[4];                 // "PKM "
        char version[2];            // "10"
        unsigned short format;      // Format = number of mipmaps = 0 (ETC1_RGB_NO_MIPMAPS)
        unsigned short extWidth;    // Texture width (big-endian)
        unsigned short extHeight;   // Texture height (big-endian)
        unsigned short origWidth;   // Original width (big-endian)
        unsigned short origHeight;  // Original height (big-endian)
    } pkmHeader;

    // NOTE: The extended width and height are the widths rounded up to a multiple of 4.
    // NOTE: ETC is always 4bit per pixel (64 bits for each 4x4 block of pixels)

    // Bytes Swap (little-endian <-> big-endian)
    //unsigned short data;
    //unsigned short swap = ((data & 0x00FF) << 8) | ((data & 0xFF00) >> 8);

    ImageEx image;

    unsigned short width;
    unsigned short height;
    unsigned short useless;

    FILE *pkmFile = fopen(fileName, "rb");

    if (pkmFile == NULL)
    {
        TraceLog(WARNING, "[%s] PKM image file could not be opened", fileName);
    }
    else
    {
        // Verify the type of file
        char filecode[4];

        fread(filecode, 1, 4, pkmFile);

        if (strncmp(filecode, "PKM ", 4) != 0)
        {
            TraceLog(WARNING, "[%s] PKM file does not seem to be a valid image", fileName);
            fclose(pkmFile);
        }
        else
        {
            // Get the surface descriptor
            fread(&useless, sizeof(unsigned short), 1, pkmFile);    // Discard version
            fread(&useless, sizeof(unsigned short), 1, pkmFile);    // Discard format

            fread(&width, sizeof(unsigned short), 1, pkmFile);      // Read extended width
            fread(&height, sizeof(unsigned short), 1, pkmFile);     // Read extended height

            int size = (width/4)*(height/4)*8;  // Total data size in bytes

            image.data = (unsigned char*)malloc(size * sizeof(unsigned char));

            fread(image.data, 1, size, pkmFile);

            fclose(pkmFile);    // Close file pointer

            image.width = width;
            image.height = height;
            image.mipmaps = 1;
            image.format = COMPRESSED_ETC1_RGB;
        }
    }

    return image;
}

// Loading PVR image data (uncompressed or PVRT compression)
// NOTE: PVR compression requires extension GL_IMG_texture_compression_pvrtc (PowerVR GPUs)
static ImageEx LoadPVR(const char *fileName)
{
    // If supported in OpenGL ES 2.0. the following formats could be defined:
    // GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG      (RGB)
    // GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG     (RGBA)
    
    // PVR file v2 Header (52 bytes)
    typedef struct {
        unsigned int headerLength;
        unsigned int height;
        unsigned int width;
        unsigned int numMipmaps;
        unsigned int flags;
        unsigned int dataLength;
        unsigned int bpp;
        unsigned int bitmaskRed;
        unsigned int bitmaskGreen;
        unsigned int bitmaskBlue;
        unsigned int bitmaskAlpha;
        unsigned int pvrTag;
        unsigned int numSurfs;
    } pvrHeaderV2;

    // PVR file v3 Header (52 bytes)
    // NOTE: After it could be metadata (15 bytes?)
    typedef struct {
        unsigned int version;            
        unsigned int flags;          
        unsigned long long pixelFormat;        
        unsigned int colourSpace;        
        unsigned int channelType;        
        unsigned int height;         
        unsigned int width;          
        unsigned int depth;          
        unsigned int numSurfaces;        
        unsigned int numFaces;       
        unsigned int numMipmaps;     
        unsigned int metaDataSize;   
    } pvrHeaderV3;
    
    // Bytes Swap (little-endian <-> big-endian)
    //unsigned short data;
    //unsigned short swap = ((data & 0x00FF) << 8) | ((data & 0xFF00) >> 8);

    ImageEx image;

    image.data = NULL;
    image.width = 0;
    image.height = 0;
    image.mipmaps = 0;
    image.format = 0;

    FILE *pvrFile = fopen(fileName, "rb");

    if (pvrFile == NULL)
    {
        TraceLog(WARNING, "[%s] PVR image file could not be opened", fileName);
    }
    else
    {
        // Check PVR image version
        unsigned int pvrVersion = 0;
        fread(&pvrVersion, sizeof(unsigned int), 1, pvrFile);
        fsetpos(pvrFile, 0);
        
        if (pvrVersion == 52)
        {
            pvrHeaderV2 header;
            
            fread(&header, sizeof(pvrHeaderV2), 1, pvrFile);
            
            image.width = header.width;
            image.height = header.height;
            image.mipmaps = header.numMipmaps;
            image.format = COMPRESSED_PVRT_RGB; //COMPRESSED_PVRT_RGBA
        }
        else if (pvrVersion == 3)
        {
            pvrHeaderV3 header;
            
            fread(&header, sizeof(pvrHeaderV3), 1, pvrFile);
            
            image.width = header.width;
            image.height = header.height;
            image.mipmaps = header.numMipmaps;
            
            // TODO: Skip metaDataSize
            
            image.format = COMPRESSED_PVRT_RGB; //COMPRESSED_PVRT_RGBA
        }

        int size = (image.width/4)*(image.height/4)*8;  // Total data size in bytes

        image.data = (unsigned char*)malloc(size * sizeof(unsigned char));

        fread(image.data, 1, size, pvrFile);

        fclose(pvrFile);    // Close file pointer
    }

    return image;
}