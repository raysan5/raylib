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
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// It's lonely here...

//----------------------------------------------------------------------------------
// Other Modules Functions Declaration (required by text)
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static Image LoadDDS(const char *fileName);   // Load DDS file
static Image LoadPKM(const char *fileName);   // Load PKM file
static Image LoadKTX(const char *fileName);   // Load KTX file
static Image LoadPVR(const char *fileName);   // Load PVR file
static Image LoadASTC(const char *fileName);  // Load ASTC file

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Load an image into CPU memory (RAM)
Image LoadImage(const char *fileName)
{
    Image image;

    // Initialize image default values
    image.data = NULL;
    image.width = 0;
    image.height = 0;
    image.mipmaps = 0;
    image.format = 0;

    if ((strcmp(GetExtension(fileName),"png") == 0) ||
        (strcmp(GetExtension(fileName),"bmp") == 0) ||
        (strcmp(GetExtension(fileName),"tga") == 0) ||
        (strcmp(GetExtension(fileName),"jpg") == 0) ||
        (strcmp(GetExtension(fileName),"gif") == 0) ||
        (strcmp(GetExtension(fileName),"psd") == 0) ||
        (strcmp(GetExtension(fileName),"pic") == 0))
    {
        int imgWidth = 0;
        int imgHeight = 0;
        int imgBpp = 0;
        
        // NOTE: Using stb_image to load images (Supports: BMP, TGA, PNG, JPG, ...)
        image.data = stbi_load(fileName, &imgWidth, &imgHeight, &imgBpp, 0);

        image.width = imgWidth;
        image.height = imgHeight;
        image.mipmaps = 1;
        
        if (imgBpp == 1) image.format = UNCOMPRESSED_GRAYSCALE;
        else if (imgBpp == 2) image.format = UNCOMPRESSED_GRAY_ALPHA;
        else if (imgBpp == 3) image.format = UNCOMPRESSED_R8G8B8;
        else if (imgBpp == 4) image.format = UNCOMPRESSED_R8G8B8A8;
    }
    else if (strcmp(GetExtension(fileName),"dds") == 0) image = LoadDDS(fileName);
    else if (strcmp(GetExtension(fileName),"pkm") == 0) image = LoadPKM(fileName);
    else if (strcmp(GetExtension(fileName),"ktx") == 0) image = LoadKTX(fileName);
    else if (strcmp(GetExtension(fileName),"pvr") == 0) image = LoadPVR(fileName);
    else if (strcmp(GetExtension(fileName),"astc") == 0) image = LoadASTC(fileName);
    
    if (image.data != NULL)
    {     
        TraceLog(INFO, "[%s] Image loaded successfully (%ix%i)", fileName, image.width, image.height);
    }
    else TraceLog(WARNING, "[%s] Image could not be loaded, file not recognized", fileName); 

    return image;
}

// Load image data from Color array data (RGBA - 32bit)
Image LoadImageEx(Color *pixels, int width, int height)
{
    Image image;
    image.data = NULL;
    image.width = width;
    image.height = height;
    image.mipmaps = 1;
    image.format = UNCOMPRESSED_R8G8B8A8;
    
    int k = 0;

    image.data = (unsigned char *)malloc(image.width*image.height*4*sizeof(unsigned char));
        
    for (int i = 0; i < image.width*image.height*4; i += 4)
    {
        ((unsigned char *)image.data)[i] = pixels[k].r;
        ((unsigned char *)image.data)[i + 1] = pixels[k].g;
        ((unsigned char *)image.data)[i + 2] = pixels[k].b;
        ((unsigned char *)image.data)[i + 3] = pixels[k].a;
        k++;
    }

    return image;
}

// Load an image from RAW file
Image LoadImageRaw(const char *fileName, int width, int height, int format, int headerSize)
{
    Image image;

    image.data = NULL;
    image.width = 0;
    image.height = 0;
    image.mipmaps = 0;
    image.format = 0;

    FILE *rawFile = fopen(fileName, "rb");

    if (rawFile == NULL)
    {
        TraceLog(WARNING, "[%s] RAW image file could not be opened", fileName);
    }
    else
    {
        if (headerSize > 0) fseek(rawFile, headerSize, SEEK_SET);

        unsigned int size = width*height;
        
        switch (format)
        {
            case UNCOMPRESSED_GRAYSCALE: image.data = (unsigned char *)malloc(size); break;               // 8 bit per pixel (no alpha)
            case UNCOMPRESSED_GRAY_ALPHA: image.data = (unsigned char *)malloc(size*2); size *= 2; break; // 16 bpp (2 channels)
            case UNCOMPRESSED_R5G6B5: image.data = (unsigned short *)malloc(size); break;                 // 16 bpp
            case UNCOMPRESSED_R8G8B8: image.data = (unsigned char *)malloc(size*3); size *= 3; break;     // 24 bpp
            case UNCOMPRESSED_R5G5B5A1: image.data = (unsigned short *)malloc(size); break;               // 16 bpp (1 bit alpha)
            case UNCOMPRESSED_R4G4B4A4: image.data = (unsigned short *)malloc(size); break;               // 16 bpp (4 bit alpha)
            case UNCOMPRESSED_R8G8B8A8: image.data = (unsigned char *)malloc(size*4); size *= 4; break;   // 32 bpp
            default: TraceLog(WARNING, "Image format not suported"); break;
        }
        
        fread(image.data, size, 1, rawFile);
        
        // TODO: Check if data have been read
        
        image.width = width;
        image.height = height;
        image.mipmaps = 0;
        image.format = format;
        
        fclose(rawFile);
    }

    return image;
}

// Load an image from rRES file (raylib Resource)
// TODO: Review function to support multiple color modes
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

                        unsigned char *compData = malloc(infoHeader.size);

                        fread(compData, infoHeader.size, 1, rresFile);

                        unsigned char *imgData = DecompressData(compData, infoHeader.size, infoHeader.srcSize);

                        // TODO: Review color mode
                        //image.data = (unsigned char *)malloc(sizeof(unsigned char)*imgWidth*imgHeight*4);
                        image.data = imgData;

                        //free(imgData);

                        free(compData);

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

    Image image = LoadImage(fileName);
    
#if defined(PLATFORM_RPI) || defined(PLATFORM_WEB)
    ImageConvertToPOT(&image, BLANK);
#endif

    if (image.data != NULL)
    { 
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }
    else
    {
        TraceLog(WARNING, "Texture could not be created");
        
        texture.id = 0;
    }

    return texture;
}

Texture2D LoadTextureEx(void *data, int width, int height, int textureFormat, int mipmapCount)
{
    Texture2D texture;

    texture.width = width;
    texture.height = height;
    texture.mipmaps = mipmapCount;
    texture.format = textureFormat;
    
    texture.id = rlglLoadTexture(data, width, height, textureFormat, mipmapCount);
    
    return texture;
}

// Load an image as texture from rRES file (raylib Resource)
Texture2D LoadTextureFromRES(const char *rresName, int resId)
{
    Texture2D texture;

    Image image = LoadImageFromRES(rresName, resId);
    texture = LoadTextureFromImage(image);
    UnloadImage(image);

    return texture;
}

// Load a texture from image data
// NOTE: image is not unloaded, it must be done manually
Texture2D LoadTextureFromImage(Image image)
{
    Texture2D texture;

    // Init texture to default values
    texture.id = 0;
    texture.width = 0;
    texture.height = 0;
    texture.mipmaps = 0;
    texture.format = 0;
    
    texture.id = rlglLoadTexture(image.data, image.width, image.height, image.format, image.mipmaps);

    texture.width = image.width;
    texture.height = image.height;
    texture.mipmaps = image.mipmaps;
    texture.format = image.format;

    return texture;
}

// Unload image from CPU memory (RAM)
void UnloadImage(Image image)
{
    free(image.data);
    
    TraceLog(INFO, "Unloaded image data");
}

// Unload texture from GPU memory
void UnloadTexture(Texture2D texture)
{
    rlDeleteTextures(texture.id);
    
    TraceLog(INFO, "[TEX ID %i] Unloaded texture data", texture.id);
}

// Get pixel data from image in the form of Color struct array
Color *GetImageData(Image image)
{
    Color *pixels = (Color *)malloc(image.width*image.height*sizeof(Color));
    
    int k = 0;

    for (int i = 0; i < image.width*image.height; i++)
    {
        switch (image.format)
        {
            case UNCOMPRESSED_GRAYSCALE:
            {
                pixels[i].r = ((unsigned char *)image.data)[k];
                pixels[i].g = ((unsigned char *)image.data)[k];
                pixels[i].b = ((unsigned char *)image.data)[k];
                pixels[i].a = 255;
        
                k++;
            } break;
            case UNCOMPRESSED_GRAY_ALPHA:
            {
                pixels[i].r = ((unsigned char *)image.data)[k];
                pixels[i].g = ((unsigned char *)image.data)[k];
                pixels[i].b = ((unsigned char *)image.data)[k];
                pixels[i].a = ((unsigned char *)image.data)[k + 1];
        
                k += 2;
            } break;
            case UNCOMPRESSED_R5G5B5A1:
            {
                unsigned short pixel = ((unsigned short *)image.data)[k];

                pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
                pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111000000) >> 6)*(255/31));
                pixels[i].b = (unsigned char)((float)((pixel & 0b0000000000111110) >> 1)*(255/31));
                pixels[i].a = (unsigned char)((pixel & 0b0000000000000001)*255);
        
                k++;
            } break;
            case UNCOMPRESSED_R5G6B5:
            {
                unsigned short pixel = ((unsigned short *)image.data)[k];

                pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
                pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111100000) >> 5)*(255/63));
                pixels[i].b = (unsigned char)((float)(pixel & 0b0000000000011111)*(255/31));
                pixels[i].a = 255;
        
                k++;
            } break;
            case UNCOMPRESSED_R4G4B4A4:
            {
                unsigned short pixel = ((unsigned short *)image.data)[k];
                
                pixels[i].r = (unsigned char)((float)((pixel & 0b1111000000000000) >> 12)*(255/15));
                pixels[i].g = (unsigned char)((float)((pixel & 0b0000111100000000) >> 8)*(255/15));
                pixels[i].b = (unsigned char)((float)((pixel & 0b0000000011110000) >> 4)*(255/15));
                pixels[i].a = (unsigned char)((float)(pixel & 0b0000000000001111)*(255/15));
        
                k++;
            } break;
            case UNCOMPRESSED_R8G8B8A8:
            {
                pixels[i].r = ((unsigned char *)image.data)[k];
                pixels[i].g = ((unsigned char *)image.data)[k + 1];
                pixels[i].b = ((unsigned char *)image.data)[k + 2];
                pixels[i].a = ((unsigned char *)image.data)[k + 3];
        
                k += 4;
            } break;
            case UNCOMPRESSED_R8G8B8:
            {
                pixels[i].r = (unsigned char)((unsigned char *)image.data)[k];
                pixels[i].g = (unsigned char)((unsigned char *)image.data)[k + 1];
                pixels[i].b = (unsigned char)((unsigned char *)image.data)[k + 2];
                pixels[i].a = 255;
        
                k += 3;
            } break;
            default: TraceLog(WARNING, "Format not supported for pixel data retrieval"); break;
        }      
    }

    return pixels;
}

// Get pixel data from GPU texture and return an Image
// NOTE: Compressed texture formats not supported
Image GetTextureData(Texture2D texture)
{
    Image image;
    image.data = NULL;
    
    if (texture.format < 8)
    {
        image.data = rlglReadTexturePixels(texture.id, texture.format);
        
        if (image.data != NULL)
        {
            image.width = texture.width;
            image.height = texture.height;
            image.format = texture.format;
            image.mipmaps = 1;
            
            TraceLog(INFO, "Texture pixel data obtained successfully");
        }
        else TraceLog(WARNING, "Texture pixel data could not be obtained");
    }
    else TraceLog(WARNING, "Compressed texture data could not be obtained");
    
    return image;
}

// Convert image data to desired format
void ImageConvertFormat(Image *image, int newFormat)
{
    if ((image->format != newFormat) && (image->format < 8) && (newFormat < 8))
    {
        Color *pixels = GetImageData(*image);
        
        free(image->data);
        
        image->format = newFormat;

        int k = 0;
        
        switch (image->format)
        {
            case UNCOMPRESSED_GRAYSCALE:
            {
                image->data = (unsigned char *)malloc(image->width*image->height*sizeof(unsigned char));
                
                for (int i = 0; i < image->width*image->height; i++)
                {
                    ((unsigned char *)image->data)[i] = (unsigned char)((float)pixels[k].r*0.299f + (float)pixels[k].g*0.587f + (float)pixels[k].b*0.114f);
                    k++;
                }
        
            } break;
            case UNCOMPRESSED_GRAY_ALPHA:
            {
               image->data = (unsigned char *)malloc(image->width*image->height*2*sizeof(unsigned char));
                
               for (int i = 0; i < image->width*image->height*2; i += 2)
                {
                    ((unsigned char *)image->data)[i] = (unsigned char)((float)pixels[k].r*0.299f + (float)pixels[k].g*0.587f + (float)pixels[k].b*0.114f);
                    ((unsigned char *)image->data)[i + 1] = pixels[k].a;
                    k++;
                }

            } break;
            case UNCOMPRESSED_R5G6B5:
            {
                image->data = (unsigned short *)malloc(image->width*image->height*sizeof(unsigned short));
                
                unsigned char r;
                unsigned char g;
                unsigned char b;
                
                for (int i = 0; i < image->width*image->height; i++)
                {
                    r = (unsigned char)(round((float)pixels[k].r*31/255));
                    g = (unsigned char)(round((float)pixels[k].g*63/255));
                    b = (unsigned char)(round((float)pixels[k].b*31/255));
                    
                    ((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 5 | (unsigned short)b;

                    k++;
                }

            } break;
            case UNCOMPRESSED_R8G8B8:
            {
                image->data = (unsigned char *)malloc(image->width*image->height*3*sizeof(unsigned char));
                
                for (int i = 0; i < image->width*image->height*3; i += 3)
                {
                    ((unsigned char *)image->data)[i] = pixels[k].r;
                    ((unsigned char *)image->data)[i + 1] = pixels[k].g;
                    ((unsigned char *)image->data)[i + 2] = pixels[k].b;
                    k++;
                }
            } break;
            case UNCOMPRESSED_R5G5B5A1:
            {
                image->data = (unsigned short *)malloc(image->width*image->height*sizeof(unsigned short));
                
                unsigned char r;
                unsigned char g;
                unsigned char b;
                unsigned char a = 1;
                
                for (int i = 0; i < image->width*image->height; i++)
                {
                    r = (unsigned char)(round((float)pixels[k].r*31/255));
                    g = (unsigned char)(round((float)pixels[k].g*31/255));
                    b = (unsigned char)(round((float)pixels[k].b*31/255));
                    a = (pixels[k].a > 50) ? 1 : 0;
                    
                    ((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1| (unsigned short)a;

                    k++;
                }

            } break;
            case UNCOMPRESSED_R4G4B4A4:
            {
                image->data = (unsigned short *)malloc(image->width*image->height*sizeof(unsigned short));
                
                unsigned char r;
                unsigned char g;
                unsigned char b;
                unsigned char a;
                
                for (int i = 0; i < image->width*image->height; i++)
                {
                    r = (unsigned char)(round((float)pixels[k].r*15/255));
                    g = (unsigned char)(round((float)pixels[k].g*15/255));
                    b = (unsigned char)(round((float)pixels[k].b*15/255));
                    a = (unsigned char)(round((float)pixels[k].a*15/255));
                    
                    ((unsigned short *)image->data)[i] = (unsigned short)r << 12 | (unsigned short)g << 8| (unsigned short)b << 4| (unsigned short)a;

                    k++;
                }
                
            } break;
            case UNCOMPRESSED_R8G8B8A8:
            {
                image->data = (unsigned char *)malloc(image->width*image->height*4*sizeof(unsigned char));
                
                for (int i = 0; i < image->width*image->height*4; i += 4)
                {
                    ((unsigned char *)image->data)[i] = pixels[k].r;
                    ((unsigned char *)image->data)[i + 1] = pixels[k].g;
                    ((unsigned char *)image->data)[i + 2] = pixels[k].b;
                    ((unsigned char *)image->data)[i + 3] = pixels[k].a;
                    k++;
                }
            } break;
            default: break;
        }
        
        free(pixels);
    }
    else TraceLog(WARNING, "Image data format is compressed, can not be converted");
}


// Convert image to POT (power-of-two)
// NOTE: Requirement on OpenGL ES 2.0 (RPI, HTML5)
void ImageConvertToPOT(Image *image, Color fillColor)
{
    // TODO: Review for new image struct
    /*
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
                if ((j < image->height) && (i < image->width)) imgDataPixelPOT[j*potWidth + i] = image->data[j*image->width + i];
                else imgDataPixelPOT[j*potWidth + i] = fillColor;
            }
        }

        TraceLog(WARNING, "Image converted to POT: (%ix%i) -> (%ix%i)", image->width, image->height, potWidth, potHeight);

        free(image->pixels);

        image->pixels = imgDataPixelPOT;
        image->width = potWidth;
        image->height = potHeight;
    }
    */
}

// Copy an image to a new image
Image ImageCopy(Image image)
{
    Image newImage;
    
    int size = image.width*image.height;
    
    switch (image.format)
    {
        case UNCOMPRESSED_GRAYSCALE: newImage.data = (unsigned char *)malloc(size); break;               // 8 bit per pixel (no alpha)
        case UNCOMPRESSED_GRAY_ALPHA: newImage.data = (unsigned char *)malloc(size*2); size *= 2; break; // 16 bpp (2 channels)
        case UNCOMPRESSED_R5G6B5: newImage.data = (unsigned short *)malloc(size); size *= 2; break;      // 16 bpp
        case UNCOMPRESSED_R8G8B8: newImage.data = (unsigned char *)malloc(size*3); size *= 3; break;     // 24 bpp
        case UNCOMPRESSED_R5G5B5A1: newImage.data = (unsigned short *)malloc(size); size *= 2; break;    // 16 bpp (1 bit alpha)
        case UNCOMPRESSED_R4G4B4A4: newImage.data = (unsigned short *)malloc(size); size *= 2; break;    // 16 bpp (4 bit alpha)
        case UNCOMPRESSED_R8G8B8A8: newImage.data = (unsigned char *)malloc(size*4); size *= 4; break;   // 32 bpp
        default: TraceLog(WARNING, "Image format not suported for copy"); break;
    }
    
    if (newImage.data != NULL)
    {
        // NOTE: Size must be provided in bytes
        memcpy(newImage.data, image.data, size);
        
        newImage.width = image.width;
        newImage.height = image.height;
        newImage.mipmaps = image.mipmaps;
        newImage.format = image.format;
    }
    
    return newImage;
}

// TODO: Some useful functions to deal with images
//void ImageCrop(Image *image, Rectangle crop) {}
//void ImageResize(Image *image, int newWidth, int newHeight) {}
//void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec) {}
//void ImageDrawText(Image *dst, const char *text, Vector2 position, int size, Color color) {}

// Generate GPU mipmaps for a texture
void GenTextureMipmaps(Texture2D texture)
{
    rlglGenerateMipmaps(texture.id);
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
static Image LoadDDS(const char *fileName)
{
    // Required extension:
    // GL_EXT_texture_compression_s3tc
    
    // Supported tokens (defined by extensions)
    // GL_COMPRESSED_RGB_S3TC_DXT1_EXT      0x83F0
    // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT     0x83F1
    // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT     0x83F2
    // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT     0x83F3
    
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
        unsigned int bBitMask;
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
        unsigned int mipmapCount;
        unsigned int reserved1[11];
        ddsPixelFormat ddspf;
        unsigned int caps;
        unsigned int caps2;
        unsigned int caps3;
        unsigned int caps4;
        unsigned int reserved2;
    } ddsHeader;

    Image image;

    image.data = NULL;
    image.width = 0;
    image.height = 0;
    image.mipmaps = 0;
    image.format = 0;

    FILE *ddsFile = fopen(fileName, "rb");

    if (ddsFile == NULL)
    {
        TraceLog(WARNING, "[%s] DDS file could not be opened", fileName);
    }
    else
    {
        // Verify the type of file
        char filecode[4];

        fread(filecode, 1, 4, ddsFile);

        if (strncmp(filecode, "DDS ", 4) != 0)
        {
            TraceLog(WARNING, "[%s] DDS file does not seem to be a valid image", fileName);
        }
        else
        {
            ddsHeader header;
            
            // Get the image header
            fread(&header, sizeof(ddsHeader), 1, ddsFile);

            TraceLog(DEBUG, "[%s] DDS file header size: %i", fileName, sizeof(ddsHeader));
            TraceLog(DEBUG, "[%s] DDS file pixel format size: %i", fileName, header.ddspf.size);
            TraceLog(DEBUG, "[%s] DDS file pixel format flags: 0x%x", fileName, header.ddspf.flags);
            TraceLog(DEBUG, "[%s] DDS file format: 0x%x", fileName, header.ddspf.fourCC);

            image.width = header.width;
            image.height = header.height;
            image.mipmaps = 1;  // Default value, could be changed (header.mipmapCount)

            if (header.ddspf.rgbBitCount == 16)     // 16bit mode, no compressed
            {
                if (header.ddspf.flags == 0x40)         // no alpha channel
                {
                    image.data = (unsigned short *)malloc(image.width*image.height*sizeof(unsigned short));
                    fread(image.data, image.width*image.height*sizeof(unsigned short), 1, ddsFile);

                    image.format = UNCOMPRESSED_R5G6B5;
                }
                else if (header.ddspf.flags == 0x41)        // with alpha channel
                {
                    if (header.ddspf.aBitMask == 0x8000)    // 1bit alpha
                    {
                        image.data = (unsigned short *)malloc(image.width*image.height*sizeof(unsigned short));
                        fread(image.data, image.width*image.height*sizeof(unsigned short), 1, ddsFile);
                        
                        unsigned char alpha = 0;
                        
                        // NOTE: Data comes as A1R5G5B5, it must be reordered to R5G5B5A1
                        for (int i = 0; i < image.width*image.height; i++)
                        {
                            alpha = ((unsigned short *)image.data)[i] >> 15;
                            ((unsigned short *)image.data)[i] = ((unsigned short *)image.data)[i] << 1;
                            ((unsigned short *)image.data)[i] += alpha;
                        }

                        image.format = UNCOMPRESSED_R5G5B5A1;
                    }
                    else if (header.ddspf.aBitMask == 0xf000)   // 4bit alpha
                    {
                        image.data = (unsigned short *)malloc(image.width*image.height*sizeof(unsigned short));
                        fread(image.data, image.width*image.height*sizeof(unsigned short), 1, ddsFile);
                        
                        unsigned char alpha = 0;
                        
                        // NOTE: Data comes as A4R4G4B4, it must be reordered R4G4B4A4
                        for (int i = 0; i < image.width*image.height; i++)
                        {
                            alpha = ((unsigned short *)image.data)[i] >> 12;
                            ((unsigned short *)image.data)[i] = ((unsigned short *)image.data)[i] << 4;
                            ((unsigned short *)image.data)[i] += alpha;
                        }
                        
                        image.format = UNCOMPRESSED_R4G4B4A4;
                    }
                }
            }
            if (header.ddspf.flags == 0x40 && header.ddspf.rgbBitCount == 24)   // DDS_RGB, no compressed
            {
                // NOTE: not sure if this case exists...
                image.data = (unsigned char *)malloc(image.width*image.height*3*sizeof(unsigned char));
                fread(image.data, image.width*image.height*3, 1, ddsFile);
                
                image.format = UNCOMPRESSED_R8G8B8;
            }
            else if (header.ddspf.flags == 0x41 && header.ddspf.rgbBitCount == 32) // DDS_RGBA, no compressed
            {
                image.data = (unsigned char *)malloc(image.width*image.height*4*sizeof(unsigned char));
                fread(image.data, image.width*image.height*4, 1, ddsFile);

                image.format = UNCOMPRESSED_R8G8B8A8;
            }
            else if (((header.ddspf.flags == 0x04) || (header.ddspf.flags == 0x05)) && (header.ddspf.fourCC > 0))
            {
                int bufsize;

                // Calculate data size, including all mipmaps
                if (header.mipmapCount > 1) bufsize = header.pitchOrLinearSize*2;
                else bufsize = header.pitchOrLinearSize;
                
                TraceLog(DEBUG, "Pitch or linear size: %i", header.pitchOrLinearSize);

                image.data = (unsigned char*)malloc(bufsize*sizeof(unsigned char));

                fread(image.data, 1, bufsize, ddsFile);

                image.mipmaps = header.mipmapCount;

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
        
        fclose(ddsFile);    // Close file pointer
    }

    return image;
}

// Loading PKM image data (ETC1/ETC2 compression)
// NOTE: KTX is the standard Khronos Group compression format (ETC1/ETC2, mipmaps)
// PKM is a much simpler file format used mainly to contain a single ETC1/ETC2 compressed image (no mipmaps)
static Image LoadPKM(const char *fileName)
{
    // Required extensions:
    // GL_OES_compressed_ETC1_RGB8_texture  (ETC1) (OpenGL ES 2.0)
    // GL_ARB_ES3_compatibility  (ETC2/EAC) (OpenGL ES 3.0)
    
    // Supported tokens (defined by extensions)
    // GL_ETC1_RGB8_OES                 0x8D64  
    // GL_COMPRESSED_RGB8_ETC2          0x9274
    // GL_COMPRESSED_RGBA8_ETC2_EAC     0x9278

    // PKM file (ETC1) Header (16 bytes)
    typedef struct {
        char id[4];                 // "PKM "
        char version[2];            // "10" or "20"
        unsigned short format;      // Data format (big-endian) (Check list below)
        unsigned short width;       // Texture width (big-endian) (origWidth rounded to multiple of 4)
        unsigned short height;      // Texture height (big-endian) (origHeight rounded to multiple of 4)
        unsigned short origWidth;   // Original width (big-endian)
        unsigned short origHeight;  // Original height (big-endian)
    } pkmHeader;
    
    // Formats list
    // version 10: format: 0=ETC1_RGB, [1=ETC1_RGBA, 2=ETC1_RGB_MIP, 3=ETC1_RGBA_MIP] (not used)
    // version 20: format: 0=ETC1_RGB, 1=ETC2_RGB, 2=ETC2_RGBA_OLD, 3=ETC2_RGBA, 4=ETC2_RGBA1, 5=ETC2_R, 6=ETC2_RG, 7=ETC2_SIGNED_R, 8=ETC2_SIGNED_R

    // NOTE: The extended width and height are the widths rounded up to a multiple of 4.
    // NOTE: ETC is always 4bit per pixel (64 bit for each 4x4 block of pixels)

    Image image;
    
    image.data = NULL;
    image.width = 0;
    image.height = 0;
    image.mipmaps = 0;
    image.format = 0;

    FILE *pkmFile = fopen(fileName, "rb");

    if (pkmFile == NULL)
    {
        TraceLog(WARNING, "[%s] PKM file could not be opened", fileName);
    }
    else
    {
        pkmHeader header;

        // Get the image header
        fread(&header, sizeof(pkmHeader), 1, pkmFile);

        if (strncmp(header.id, "PKM ", 4) != 0)
        {
            TraceLog(WARNING, "[%s] PKM file does not seem to be a valid image", fileName);
        }
        else
        {
            // NOTE: format, width and height come as big-endian, data must be swapped to little-endian
            header.format = ((header.format & 0x00FF) << 8) | ((header.format & 0xFF00) >> 8);
            header.width = ((header.width & 0x00FF) << 8) | ((header.width & 0xFF00) >> 8);
            header.height = ((header.height & 0x00FF) << 8) | ((header.height & 0xFF00) >> 8);
            
            TraceLog(INFO, "PKM (ETC) image width: %i", header.width);
            TraceLog(INFO, "PKM (ETC) image height: %i", header.height);
            TraceLog(INFO, "PKM (ETC) image format: %i", header.format);
            
            image.width = header.width;
            image.height = header.height;
            image.mipmaps = 1;
            
            int size = image.width*image.height*4/8;  // Total data size in bytes

            image.data = (unsigned char*)malloc(size * sizeof(unsigned char));

            fread(image.data, 1, size, pkmFile);

            if (header.format == 0) image.format = COMPRESSED_ETC1_RGB;
            else if (header.format == 1) image.format = COMPRESSED_ETC2_RGB;
            else if (header.format == 3) image.format = COMPRESSED_ETC2_EAC_RGBA;
        }
        
        fclose(pkmFile);    // Close file pointer
    }

    return image;
}

// Load KTX compressed image data (ETC1/ETC2 compression)
static Image LoadKTX(const char *fileName)
{
    // Required extensions:
    // GL_OES_compressed_ETC1_RGB8_texture  (ETC1)
    // GL_ARB_ES3_compatibility  (ETC2/EAC)
    
    // Supported tokens (defined by extensions)
    // GL_ETC1_RGB8_OES                 0x8D64
    // GL_COMPRESSED_RGB8_ETC2          0x9274
    // GL_COMPRESSED_RGBA8_ETC2_EAC     0x9278
    
    // KTX file Header (64 bytes)
    // https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
    typedef struct {
        char id[12];                        // Identifier: "«KTX 11»\r\n\x1A\n"
        unsigned int endianness;            // Little endian: 0x01 0x02 0x03 0x04
        unsigned int glType;                // For compressed textures, glType must equal 0
        unsigned int glTypeSize;            // For compressed texture data, usually 1
        unsigned int glFormat;              // For compressed textures is 0
        unsigned int glInternalFormat;      // Compressed internal format
        unsigned int glBaseInternalFormat;  // Same as glFormat (RGB, RGBA, ALPHA...)
        unsigned int width;                 // Texture image width in pixels
        unsigned int height;                // Texture image height in pixels
        unsigned int depth;                 // For 2D textures is 0
        unsigned int elements;              // Number of array elements, usually 0
        unsigned int faces;                 // Cubemap faces, for no-cubemap = 1
        unsigned int mipmapLevels;          // Non-mipmapped textures = 1
        unsigned int keyValueDataSize;      // Used to encode any arbitrary data...
    } ktxHeader;
    
    // NOTE: Before start of every mipmap data block, we have: unsigned int dataSize
    
    Image image;

    image.width = 0;
    image.height = 0;
    image.mipmaps = 0;
    image.format = 0;
    
    FILE *ktxFile = fopen(fileName, "rb");

    if (ktxFile == NULL)
    {
        TraceLog(WARNING, "[%s] KTX image file could not be opened", fileName);
    }
    else
    {
        ktxHeader header;

        // Get the image header
        fread(&header, sizeof(ktxHeader), 1, ktxFile);

        if ((header.id[1] != 'K') || (header.id[2] != 'T') || (header.id[3] != 'X') ||
            (header.id[4] != ' ') || (header.id[5] != '1') || (header.id[6] != '1'))
        {
            TraceLog(WARNING, "[%s] KTX file does not seem to be a valid file", fileName);
        }
        else
        {           
            image.width = header.width;
            image.height = header.height;
            image.mipmaps = header.mipmapLevels;
            
            TraceLog(DEBUG, "KTX (ETC) image width: %i", header.width);
            TraceLog(DEBUG, "KTX (ETC) image height: %i", header.height);
            TraceLog(DEBUG, "KTX (ETC) image format: 0x%x", header.glInternalFormat);
            
            unsigned char unused;
            
            if (header.keyValueDataSize > 0)
            {
                for (int i = 0; i < header.keyValueDataSize; i++) fread(&unused, 1, 1, ktxFile);
            }
            
            int dataSize;
            fread(&dataSize, sizeof(unsigned int), 1, ktxFile);

            image.data = (unsigned char*)malloc(dataSize * sizeof(unsigned char));

            fread(image.data, 1, dataSize, ktxFile);

            if (header.glInternalFormat == 0x8D64) image.format = COMPRESSED_ETC1_RGB;
            else if (header.glInternalFormat == 0x9274) image.format = COMPRESSED_ETC2_RGB;
            else if (header.glInternalFormat == 0x9278) image.format = COMPRESSED_ETC2_EAC_RGBA;
        }
        
        fclose(ktxFile);    // Close file pointer
    }
    
    return image;
}

// Loading PVR image data (uncompressed or PVRT compression)
// NOTE: PVR v2 not supported, use PVR v3 instead
static Image LoadPVR(const char *fileName)
{
    // Required extension:
    // GL_IMG_texture_compression_pvrtc
    
    // Supported tokens (defined by extensions)
    // GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG       0x8C00
    // GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG      0x8C02
    
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
        char id[4];
        unsigned int flags;
        unsigned char channels[4];      // pixelFormat high part
        unsigned char channelDepth[4];  // pixelFormat low part
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
    
    // Metadata (usually 15 bytes)
    typedef struct {
        unsigned int devFOURCC;
        unsigned int key;
        unsigned int dataSize;      // Not used?
        unsigned char *data;        // Not used?
    } pvrMetadata; 

    Image image;

    image.data = NULL;
    image.width = 0;
    image.height = 0;
    image.mipmaps = 0;
    image.format = 0;

    FILE *pvrFile = fopen(fileName, "rb");

    if (pvrFile == NULL)
    {
        TraceLog(WARNING, "[%s] PVR file could not be opened", fileName);
    }
    else
    {
        // Check PVR image version
        unsigned char pvrVersion = 0;
        fread(&pvrVersion, sizeof(unsigned char), 1, pvrFile);
        fseek(pvrFile, 0, SEEK_SET);
        
        // Load different PVR data formats
        if (pvrVersion == 0x50)
        {
            pvrHeaderV3 header;
            
            // Get PVR image header
            fread(&header, sizeof(pvrHeaderV3), 1, pvrFile);
            
            if ((header.id[0] != 'P') || (header.id[1] != 'V') || (header.id[2] != 'R') || (header.id[3] != 3))
            {
                TraceLog(WARNING, "[%s] PVR file does not seem to be a valid image", fileName);
            }
            else
            {
                image.width = header.width;
                image.height = header.height;
                image.mipmaps = header.numMipmaps;
                
                // Check data format
                if (((header.channels[0] == 'l') && (header.channels[1] == 0)) && (header.channelDepth[0] == 8)) image.format = UNCOMPRESSED_GRAYSCALE;
                else if (((header.channels[0] == 'l') && (header.channels[1] == 'a')) && ((header.channelDepth[0] == 8) && (header.channelDepth[1] == 8))) image.format = UNCOMPRESSED_GRAY_ALPHA;
                else if ((header.channels[0] == 'r') && (header.channels[1] == 'g') && (header.channels[2] == 'b'))
                {
                    if (header.channels[3] == 'a')
                    {
                        if ((header.channelDepth[0] == 5) && (header.channelDepth[1] == 5) && (header.channelDepth[2] == 5) && (header.channelDepth[3] == 1)) image.format = UNCOMPRESSED_R5G5B5A1;
                        else if ((header.channelDepth[0] == 4) && (header.channelDepth[1] == 4) && (header.channelDepth[2] == 4) && (header.channelDepth[3] == 4)) image.format = UNCOMPRESSED_R4G4B4A4;
                        else if ((header.channelDepth[0] == 8) && (header.channelDepth[1] == 8) && (header.channelDepth[2] == 8) && (header.channelDepth[3] == 8)) image.format = UNCOMPRESSED_R8G8B8A8;
                    }
                    else if (header.channels[3] == 0)
                    {
                        if ((header.channelDepth[0] == 5) && (header.channelDepth[1] == 6) && (header.channelDepth[2] == 5)) image.format = UNCOMPRESSED_R5G6B5;
                        else if ((header.channelDepth[0] == 8) && (header.channelDepth[1] == 8) && (header.channelDepth[2] == 8)) image.format = UNCOMPRESSED_R8G8B8;
                    }
                }
                else if (header.channels[0] == 2) image.format = COMPRESSED_PVRT_RGB;
                else if (header.channels[0] == 3) image.format = COMPRESSED_PVRT_RGBA;
                
                // Skip meta data header
                unsigned char unused = 0;
                for (int i = 0; i < header.metaDataSize; i++) fread(&unused, sizeof(unsigned char), 1, pvrFile);
                
                // Calculate data size (depends on format)
                int bpp = 0;
                
                switch (image.format)
                {
                    case UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
                    case UNCOMPRESSED_GRAY_ALPHA:
                    case UNCOMPRESSED_R5G5B5A1:
                    case UNCOMPRESSED_R5G6B5:
                    case UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
                    case UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
                    case UNCOMPRESSED_R8G8B8: bpp = 24; break;
                    case COMPRESSED_PVRT_RGB:
                    case COMPRESSED_PVRT_RGBA: bpp = 4; break;
                    default: break;
                }
                
                int dataSize = image.width*image.height*bpp/8;  // Total data size in bytes
                image.data = (unsigned char*)malloc(dataSize*sizeof(unsigned char));

                // Read data from file
                fread(image.data, dataSize, 1, pvrFile);
            }
        }
        else if (pvrVersion == 52) TraceLog(INFO, "PVR v2 not supported, update your files to PVR v3");

        fclose(pvrFile);    // Close file pointer
    }

    return image;
}

// Load ASTC compressed image data (ASTC compression)
static Image LoadASTC(const char *fileName)
{
    // Required extensions:
    // GL_KHR_texture_compression_astc_hdr
    // GL_KHR_texture_compression_astc_ldr
    
    // Supported tokens (defined by extensions)
    // GL_COMPRESSED_RGBA_ASTC_4x4_KHR      0x93b0
    // GL_COMPRESSED_RGBA_ASTC_8x8_KHR      0x93b7
    
    // ASTC file Header (16 bytes)
    typedef struct {
        unsigned char id[4];        // Signature: 0x13 0xAB 0xA1 0x5C
        unsigned char blockX;       // Block X dimensions
        unsigned char blockY;       // Block Y dimensions
        unsigned char blockZ;       // Block Z dimensions (1 for 2D images)
        unsigned char width[3];     // Image width in pixels (24bit value)
        unsigned char height[3];    // Image height in pixels (24bit value)
        unsigned char lenght[3];    // Image Z-size (1 for 2D images)
    } astcHeader;

    Image image;

    image.data = NULL;
    image.width = 0;
    image.height = 0;
    image.mipmaps = 0;
    image.format = 0;
    
    FILE *astcFile = fopen(fileName, "rb");

    if (astcFile == NULL)
    {
        TraceLog(WARNING, "[%s] ASTC file could not be opened", fileName);
    }
    else
    {
        astcHeader header;

        // Get ASTC image header
        fread(&header, sizeof(astcHeader), 1, astcFile);
        
        if ((header.id[3] != 0x5c) || (header.id[2] != 0xa1) || (header.id[1] != 0xab) || (header.id[0] != 0x13))
        {
            TraceLog(WARNING, "[%s] ASTC file does not seem to be a valid image", fileName);
        }
        else
        {
            // NOTE: Assuming Little Endian (could it be wrong?)
            image.width = 0x00000000 | ((int)header.width[2] << 16) | ((int)header.width[1] << 8) | ((int)header.width[0]);
            image.height = 0x00000000 | ((int)header.height[2] << 16) | ((int)header.height[1] << 8) | ((int)header.height[0]);
            
            // NOTE: ASTC format only contains one mipmap level
            image.mipmaps = 1;
            
            TraceLog(DEBUG, "ASTC image width: %i", image.width);
            TraceLog(DEBUG, "ASTC image height: %i", image.height);
            TraceLog(DEBUG, "ASTC image blocks: %ix%i", header.blockX, header.blockY);
            
            // NOTE: Each block is always stored in 128bit so we can calculate the bpp
            int bpp = 128/(header.blockX*header.blockY);

            // NOTE: Currently we only support 2 blocks configurations: 4x4 and 8x8
            if ((bpp == 8) || (bpp == 2)) 
            {
                int dataSize = image.width*image.height*bpp/8;  // Data size in bytes
                
                image.data = (unsigned char *)malloc(dataSize*sizeof(unsigned char));
                fread(image.data, dataSize, 1, astcFile);
                
                if (bpp == 8) image.format = COMPRESSED_ASTC_4x4_RGBA;
                else if (bpp == 2) image.format = COMPRESSED_ASTC_4x4_RGBA;
            }
            else TraceLog(WARNING, "[%s] ASTC block size configuration not supported", fileName);
        }
        
        fclose(astcFile);
    }

    return image;
}
