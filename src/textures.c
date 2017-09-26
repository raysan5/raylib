/**********************************************************************************************
*
*   raylib.textures - Basic functions to load and draw Textures (2d)
*
*   CONFIGURATION:
*
*   #define SUPPORT_FILEFORMAT_BMP
*   #define SUPPORT_FILEFORMAT_PNG
*   #define SUPPORT_FILEFORMAT_TGA
*   #define SUPPORT_FILEFORMAT_JPG
*   #define SUPPORT_FILEFORMAT_GIF
*   #define SUPPORT_FILEFORMAT_PSD
*   #define SUPPORT_FILEFORMAT_HDR
*   #define SUPPORT_FILEFORMAT_DDS
*   #define SUPPORT_FILEFORMAT_PKM
*   #define SUPPORT_FILEFORMAT_KTX
*   #define SUPPORT_FILEFORMAT_PVR
*   #define SUPPORT_FILEFORMAT_ASTC
*       Selecte desired fileformats to be supported for image data loading. Some of those formats are 
*       supported by default, to remove support, just comment unrequired #define in this module
*
*   #define SUPPORT_IMAGE_MANIPULATION
*       Support multiple image editing functions to scale, adjust colors, flip, draw on images, crop...
*       If not defined only three image editing functions supported: ImageFormat(), ImageAlphaMask(), ImageToPOT()
*
*   #define SUPPORT_IMAGE_GENERATION
*       Support proedural image generation functionality (gradient, spot, perlin-noise, cellular)
*
*   DEPENDENCIES:
*       stb_image        - Multiple image formats loading (JPEG, PNG, BMP, TGA, PSD, GIF, PIC)
*                          NOTE: stb_image has been slightly modified to support Android platform.
*       stb_image_resize - Multiple image resize algorythms
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2017 Ramon Santamaria (@raysan5)
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

// Default configuration flags (supported features)
//-------------------------------------------------
#define SUPPORT_FILEFORMAT_PNG
#define SUPPORT_FILEFORMAT_DDS
#define SUPPORT_FILEFORMAT_HDR
#define SUPPORT_IMAGE_MANIPULATION
#define SUPPORT_IMAGE_GENERATION
//-------------------------------------------------

#include "raylib.h"

#include <stdlib.h>             // Required for: malloc(), free()
#include <string.h>             // Required for: strcmp(), strrchr(), strncmp()

#include "rlgl.h"               // raylib OpenGL abstraction layer to OpenGL 1.1, 3.3 or ES2
                                // Required for: rlLoadTexture() rlDeleteTextures(),
                                //      rlGenerateMipmaps(), some funcs for DrawTexturePro()

#include "utils.h"              // Required for: fopen() Android mapping

#define STB_PERLIN_IMPLEMENTATION
#include "external/stb_perlin.h"// Required for: stb_perlin_fbm_noise3

// Support only desired texture formats on stb_image
#if !defined(SUPPORT_FILEFORMAT_BMP)
    #define STBI_NO_BMP
#endif
#if !defined(SUPPORT_FILEFORMAT_PNG)
    #define STBI_NO_PNG
#endif
#if !defined(SUPPORT_FILEFORMAT_TGA)
    #define STBI_NO_TGA
#endif
#if !defined(SUPPORT_FILEFORMAT_JPG)
    #define STBI_NO_JPEG        // Image format .jpg and .jpeg
#endif
#if !defined(SUPPORT_FILEFORMAT_PSD)
    #define STBI_NO_PSD
#endif
#if !defined(SUPPORT_FILEFORMAT_GIF)
    #define STBI_NO_GIF
#endif
#if !defined(SUPPORT_FILEFORMAT_HDR)
    #define STBI_NO_HDR
#endif

// Image fileformats not supported by default
#define STBI_NO_PIC
#define STBI_NO_PNM             // Image format .ppm and .pgm

#if (defined(SUPPORT_FILEFORMAT_BMP) || defined(SUPPORT_FILEFORMAT_PNG) || defined(SUPPORT_FILEFORMAT_TGA) || \
     defined(SUPPORT_FILEFORMAT_JPG) || defined(SUPPORT_FILEFORMAT_PSD) || defined(SUPPORT_FILEFORMAT_GIF) || \
     defined(SUPPORT_FILEFORMAT_HDR))
    #define STB_IMAGE_IMPLEMENTATION
    #include "external/stb_image.h"     // Required for: stbi_load_from_file()
                                        // NOTE: Used to read image data (multiple formats support)
#endif
                                
#if defined(SUPPORT_IMAGE_MANIPULATION)
    #define STB_IMAGE_RESIZE_IMPLEMENTATION
    #include "external/stb_image_resize.h"  // Required for: stbir_resize_uint8()
                                            // NOTE: Used for image scaling on ImageResize()
#endif

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
#if defined(SUPPORT_FILEFORMAT_DDS)
static Image LoadDDS(const char *fileName);   // Load DDS file
#endif
#if defined(SUPPORT_FILEFORMAT_PKM)
static Image LoadPKM(const char *fileName);   // Load PKM file
#endif
#if defined(SUPPORT_FILEFORMAT_KTX)
static Image LoadKTX(const char *fileName);   // Load KTX file
#endif
#if defined(SUPPORT_FILEFORMAT_PVR)
static Image LoadPVR(const char *fileName);   // Load PVR file
#endif
#if defined(SUPPORT_FILEFORMAT_ASTC)
static Image LoadASTC(const char *fileName);  // Load ASTC file
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Load image from file into CPU memory (RAM)
Image LoadImage(const char *fileName)
{
    Image image = { 0 };

    if (IsFileExtension(fileName, ".rres"))
    {
        RRES rres = LoadResource(fileName, 0);

        // NOTE: Parameters for RRES_TYPE_IMAGE are: width, height, format, mipmaps

        if (rres[0].type == RRES_TYPE_IMAGE) image = LoadImagePro(rres[0].data, rres[0].param1, rres[0].param2, rres[0].param3);
        else TraceLog(LOG_WARNING, "[%s] Resource file does not contain image data", fileName);

        UnloadResource(rres);
    }
    else if ((IsFileExtension(fileName, ".png"))
#if defined(SUPPORT_FILEFORMAT_BMP)
        || (IsFileExtension(fileName, ".bmp"))
#endif
#if defined(SUPPORT_FILEFORMAT_TGA)
        || (IsFileExtension(fileName, ".tga"))
#endif
#if defined(SUPPORT_FILEFORMAT_JPG)
        || (IsFileExtension(fileName, ".jpg"))
#endif
#if defined(SUPPORT_FILEFORMAT_DDS)
        || (IsFileExtension(fileName, ".gif"))
#endif
#if defined(SUPPORT_FILEFORMAT_PSD)
        || (IsFileExtension(fileName, ".psd"))
#endif
       )
    {
        int imgWidth = 0;
        int imgHeight = 0;
        int imgBpp = 0;
        
        FILE *imFile = fopen(fileName, "rb");
        
        if (imFile != NULL)
        {
            // NOTE: Using stb_image to load images (Supports: BMP, TGA, PNG, JPG, ...)
            image.data = stbi_load_from_file(imFile, &imgWidth, &imgHeight, &imgBpp, 0);
            
            fclose(imFile);

            image.width = imgWidth;
            image.height = imgHeight;
            image.mipmaps = 1;

            if (imgBpp == 1) image.format = UNCOMPRESSED_GRAYSCALE;
            else if (imgBpp == 2) image.format = UNCOMPRESSED_GRAY_ALPHA;
            else if (imgBpp == 3) image.format = UNCOMPRESSED_R8G8B8;
            else if (imgBpp == 4) image.format = UNCOMPRESSED_R8G8B8A8;
        }
    }
#if defined(SUPPORT_FILEFORMAT_HDR)
    else if (IsFileExtension(fileName, ".hdr"))
    {
        int imgBpp = 0;
        
        FILE *imFile = fopen(fileName, "rb");
        
        stbi_set_flip_vertically_on_load(true);
        
        // Load 32 bit per channel floats data 
        image.data = stbi_loadf_from_file(imFile, &image.width, &image.height, &imgBpp, 0);
        
        stbi_set_flip_vertically_on_load(false);

        fclose(imFile);
        
        image.mipmaps = 1;
        
        if (imgBpp == 3) image.format = UNCOMPRESSED_R32G32B32;
        else 
        {
            // TODO: Support different number of channels at 32 bit float
            TraceLog(LOG_WARNING, "[%s] Image fileformat not supported (only 3 channel 32 bit floats)", fileName);
            UnloadImage(image);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_DDS)
    else if (IsFileExtension(fileName, ".dds")) image = LoadDDS(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_PKM)
    else if (IsFileExtension(fileName, ".pkm")) image = LoadPKM(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_KTX)
    else if (IsFileExtension(fileName, ".ktx")) image = LoadKTX(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_PVR)
    else if (IsFileExtension(fileName, ".pvr")) image = LoadPVR(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_ASTC)
    else if (IsFileExtension(fileName, ".astc")) image = LoadASTC(fileName);
#endif
    else TraceLog(LOG_WARNING, "[%s] Image fileformat not supported", fileName);

    if (image.data != NULL) TraceLog(LOG_INFO, "[%s] Image loaded successfully (%ix%i)", fileName, image.width, image.height);
    else TraceLog(LOG_WARNING, "[%s] Image could not be loaded", fileName);

    return image;
}

// Load image from Color array data (RGBA - 32bit)
// NOTE: Creates a copy of pixels data array
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

// Load image from raw data with parameters
// NOTE: This functions makes a copy of provided data
Image LoadImagePro(void *data, int width, int height, int format)
{
    Image srcImage = { 0 };

    srcImage.data = data;
    srcImage.width = width;
    srcImage.height = height;
    srcImage.mipmaps = 1;
    srcImage.format = format;

    Image dstImage = ImageCopy(srcImage);

    return dstImage;
}

// Load an image from RAW file data
Image LoadImageRaw(const char *fileName, int width, int height, int format, int headerSize)
{
    Image image = { 0 };

    FILE *rawFile = fopen(fileName, "rb");

    if (rawFile == NULL)
    {
        TraceLog(LOG_WARNING, "[%s] RAW image file could not be opened", fileName);
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
            case UNCOMPRESSED_R32G32B32: image.data = (float *)malloc(size*12); size *= 12; break;        // 4 byte per channel (12 byte)
            default: TraceLog(LOG_WARNING, "Image format not suported"); break;
        }

        // NOTE: fread() returns num read elements instead of bytes, 
        // to get bytes we need to read (1 byte size, elements) instead of (x byte size, 1 element)
        int bytes = fread(image.data, 1, size, rawFile);

        // Check if data has been read successfully
        if (bytes < size)
        {
            TraceLog(LOG_WARNING, "[%s] RAW image data can not be read, wrong requested format or size", fileName);

            if (image.data != NULL) free(image.data);
        }
        else
        {
            image.width = width;
            image.height = height;
            image.mipmaps = 1;
            image.format = format;
        }

        fclose(rawFile);
    }

    return image;
}

// Load texture from file into GPU memory (VRAM)
Texture2D LoadTexture(const char *fileName)
{
    Texture2D texture = { 0 };

    Image image = LoadImage(fileName);

    if (image.data != NULL)
    {
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }
    else TraceLog(LOG_WARNING, "Texture could not be created");

    return texture;
}

// Load a texture from image data
// NOTE: image is not unloaded, it must be done manually
Texture2D LoadTextureFromImage(Image image)
{
    Texture2D texture = { 0 };

    texture.id = rlLoadTexture(image.data, image.width, image.height, image.format, image.mipmaps);

    texture.width = image.width;
    texture.height = image.height;
    texture.mipmaps = image.mipmaps;
    texture.format = image.format;
    
    TraceLog(LOG_DEBUG, "[TEX ID %i] Parameters: %ix%i, %i mips, format %i", texture.id, texture.width, texture.height, texture.mipmaps, texture.format);

    return texture;
}

// Load texture for rendering (framebuffer)
RenderTexture2D LoadRenderTexture(int width, int height)
{
    RenderTexture2D target = rlLoadRenderTexture(width, height);

    return target;
}

// Unload image from CPU memory (RAM)
void UnloadImage(Image image)
{
    if (image.data != NULL) free(image.data);

    // NOTE: It becomes anoying every time a texture is loaded
    //TraceLog(LOG_INFO, "Unloaded image data");
}

// Unload texture from GPU memory (VRAM)
void UnloadTexture(Texture2D texture)
{
    if (texture.id > 0)
    {
        rlDeleteTextures(texture.id);

        TraceLog(LOG_INFO, "[TEX ID %i] Unloaded texture data from VRAM (GPU)", texture.id);
    }
}

// Unload render texture from GPU memory (VRAM)
void UnloadRenderTexture(RenderTexture2D target)
{
    if (target.id > 0) rlDeleteRenderTextures(target);
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
            default: TraceLog(LOG_WARNING, "Format not supported for pixel data retrieval"); break;
        }
    }

    return pixels;
}

// Get pixel data from GPU texture and return an Image
// NOTE: Compressed texture formats not supported
Image GetTextureData(Texture2D texture)
{
    Image image = { 0 };
    
    if (texture.format < 8)
    {
        image.data = rlReadTexturePixels(texture);

        if (image.data != NULL)
        {
            image.width = texture.width;
            image.height = texture.height;
            image.mipmaps = 1;
            
            if (rlGetVersion() == OPENGL_ES_20)
            {
                // NOTE: Data retrieved on OpenGL ES 2.0 comes as RGBA (from framebuffer)
                image.format = UNCOMPRESSED_R8G8B8A8;
            }
            else image.format = texture.format;

            TraceLog(LOG_INFO, "Texture pixel data obtained successfully");
        }
        else TraceLog(LOG_WARNING, "Texture pixel data could not be obtained");
    }
    else TraceLog(LOG_WARNING, "Compressed texture data could not be obtained");

    return image;
}

// Update GPU texture with new data
// NOTE: pixels data must match texture.format
void UpdateTexture(Texture2D texture, const void *pixels)
{
    rlUpdateTexture(texture.id, texture.width, texture.height, texture.format, pixels);
}

// Save image to a PNG file
void SaveImageAs(const char* fileName, Image image)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI)
    unsigned char* imgData = (unsigned char*)GetImageData(image); // this works since Color is just a container for the RGBA values
    SavePNG(fileName, imgData, image.width, image.height, 4);
    free(imgData);

    TraceLog(LOG_INFO, "Image saved: %s", fileName);
#endif
}

// Convert image data to desired format
void ImageFormat(Image *image, int newFormat)
{
    if (image->format != newFormat)
    {
        if ((image->format < COMPRESSED_DXT1_RGB) && (newFormat < COMPRESSED_DXT1_RGB))
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
                        ((unsigned char *)image->data)[i] = (unsigned char)((float)pixels[i].r*0.299f + (float)pixels[i].g*0.587f + (float)pixels[i].b*0.114f);
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

                    unsigned char r = 0;
                    unsigned char g = 0;
                    unsigned char b = 0;

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        r = (unsigned char)(round((float)pixels[k].r*31/255));
                        g = (unsigned char)(round((float)pixels[k].g*63/255));
                        b = (unsigned char)(round((float)pixels[k].b*31/255));

                        ((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 5 | (unsigned short)b;
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
                    #define ALPHA_THRESHOLD  50

                    image->data = (unsigned short *)malloc(image->width*image->height*sizeof(unsigned short));

                    unsigned char r = 0;
                    unsigned char g = 0;
                    unsigned char b = 0;
                    unsigned char a = 0;

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        r = (unsigned char)(round((float)pixels[i].r*31/255));
                        g = (unsigned char)(round((float)pixels[i].g*31/255));
                        b = (unsigned char)(round((float)pixels[i].b*31/255));
                        a = (pixels[i].a > ALPHA_THRESHOLD) ? 1 : 0;

                        ((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1 | (unsigned short)a;
                    }

                } break;
                case UNCOMPRESSED_R4G4B4A4:
                {
                    image->data = (unsigned short *)malloc(image->width*image->height*sizeof(unsigned short));

                    unsigned char r = 0;
                    unsigned char g = 0;
                    unsigned char b = 0;
                    unsigned char a = 0;

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        r = (unsigned char)(round((float)pixels[i].r*15/255));
                        g = (unsigned char)(round((float)pixels[i].g*15/255));
                        b = (unsigned char)(round((float)pixels[i].b*15/255));
                        a = (unsigned char)(round((float)pixels[i].a*15/255));

                        ((unsigned short *)image->data)[i] = (unsigned short)r << 12 | (unsigned short)g << 8| (unsigned short)b << 4| (unsigned short)a;
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
        else TraceLog(LOG_WARNING, "Image data format is compressed, can not be converted");
    }
}

// Apply alpha mask to image
// NOTE 1: Returned image is GRAY_ALPHA (16bit) or RGBA (32bit)
// NOTE 2: alphaMask should be same size as image
void ImageAlphaMask(Image *image, Image alphaMask)
{
    if ((image->width != alphaMask.width) || (image->height != alphaMask.height))
    {
        TraceLog(LOG_WARNING, "Alpha mask must be same size as image");
    }
    else if (image->format >= COMPRESSED_DXT1_RGB)
    {
        TraceLog(LOG_WARNING, "Alpha mask can not be applied to compressed data formats");
    }
    else
    {
        // Force mask to be Grayscale
        Image mask = ImageCopy(alphaMask);
        if (mask.format != UNCOMPRESSED_GRAYSCALE) ImageFormat(&mask, UNCOMPRESSED_GRAYSCALE);

        // In case image is only grayscale, we just add alpha channel
        if (image->format == UNCOMPRESSED_GRAYSCALE)
        {
            ImageFormat(image, UNCOMPRESSED_GRAY_ALPHA);

            // Apply alpha mask to alpha channel
            for (int i = 0, k = 1; (i < mask.width*mask.height) || (i < image->width*image->height); i++, k += 2)
            {
                ((unsigned char *)image->data)[k] = ((unsigned char *)mask.data)[i];
            }
        }
        else
        {
            // Convert image to RGBA
            if (image->format != UNCOMPRESSED_R8G8B8A8) ImageFormat(image, UNCOMPRESSED_R8G8B8A8);

            // Apply alpha mask to alpha channel
            for (int i = 0, k = 3; (i < mask.width*mask.height) || (i < image->width*image->height); i++, k += 4)
            {
                ((unsigned char *)image->data)[k] = ((unsigned char *)mask.data)[i];
            }
        }

        UnloadImage(mask);
    }
}

// Convert image to POT (power-of-two)
// NOTE: It could be useful on OpenGL ES 2.0 (RPI, HTML5)
void ImageToPOT(Image *image, Color fillColor)
{
    Color *pixels = GetImageData(*image);   // Get pixels data

    // Calculate next power-of-two values
    // NOTE: Just add the required amount of pixels at the right and bottom sides of image...
    int potWidth = (int)powf(2, ceilf(logf((float)image->width)/logf(2)));
    int potHeight = (int)powf(2, ceilf(logf((float)image->height)/logf(2)));

    // Check if POT texture generation is required (if texture is not already POT)
    if ((potWidth != image->width) || (potHeight != image->height))
    {
        Color *pixelsPOT = NULL;

        // Generate POT array from NPOT data
        pixelsPOT = (Color *)malloc(potWidth*potHeight*sizeof(Color));

        for (int j = 0; j < potHeight; j++)
        {
            for (int i = 0; i < potWidth; i++)
            {
                if ((j < image->height) && (i < image->width)) pixelsPOT[j*potWidth + i] = pixels[j*image->width + i];
                else pixelsPOT[j*potWidth + i] = fillColor;
            }
        }

        TraceLog(LOG_WARNING, "Image converted to POT: (%ix%i) -> (%ix%i)", image->width, image->height, potWidth, potHeight);

        free(pixels);                       // Free pixels data
        free(image->data);                  // Free old image data

        int format = image->format;         // Store image data format to reconvert later

        // TODO: Image width and height changes... do we want to store new values or keep the old ones?
        // NOTE: Issues when using image.width and image.height for sprite animations...
        *image = LoadImageEx(pixelsPOT, potWidth, potHeight);

        free(pixelsPOT);                    // Free POT pixels data

        ImageFormat(image, format);  // Reconvert image to previous format
    }
}

#if defined(SUPPORT_IMAGE_MANIPULATION)
// Copy an image to a new image
Image ImageCopy(Image image)
{
    Image newImage;

    int byteSize = image.width*image.height;

    switch (image.format)
    {
        case UNCOMPRESSED_GRAYSCALE: break;                 // 8 bpp (1 byte)
        case UNCOMPRESSED_GRAY_ALPHA:                       // 16 bpp
        case UNCOMPRESSED_R5G6B5:                           // 16 bpp
        case UNCOMPRESSED_R5G5B5A1:                         // 16 bpp
        case UNCOMPRESSED_R4G4B4A4: byteSize *= 2; break;   // 16 bpp (2 bytes)
        case UNCOMPRESSED_R8G8B8: byteSize *= 3; break;     // 24 bpp (3 bytes)
        case UNCOMPRESSED_R8G8B8A8: byteSize *= 4; break;   // 32 bpp (4 bytes)
        case UNCOMPRESSED_R32G32B32: byteSize *= 12; break; // 4 byte per channel (12 bytes)
        case COMPRESSED_DXT3_RGBA:
        case COMPRESSED_DXT5_RGBA:
        case COMPRESSED_ETC2_EAC_RGBA:
        case COMPRESSED_ASTC_4x4_RGBA: break;               // 8 bpp (1 byte)
        case COMPRESSED_DXT1_RGB:
        case COMPRESSED_DXT1_RGBA:
        case COMPRESSED_ETC1_RGB:
        case COMPRESSED_ETC2_RGB:
        case COMPRESSED_PVRT_RGB:
        case COMPRESSED_PVRT_RGBA: byteSize /= 2; break;    // 4 bpp
        case COMPRESSED_ASTC_8x8_RGBA: byteSize /= 4; break;// 2 bpp
        default: TraceLog(LOG_WARNING, "Image format not recognized"); break;
    }

    newImage.data = malloc(byteSize);

    if (newImage.data != NULL)
    {
        // NOTE: Size must be provided in bytes
        memcpy(newImage.data, image.data, byteSize);

        newImage.width = image.width;
        newImage.height = image.height;
        newImage.mipmaps = image.mipmaps;
        newImage.format = image.format;
    }

    return newImage;
}

// Crop an image to area defined by a rectangle
// NOTE: Security checks are performed in case rectangle goes out of bounds
void ImageCrop(Image *image, Rectangle crop)
{
    // Security checks to make sure cropping rectangle is inside margins
    if ((crop.x + crop.width) > image->width)
    {
        crop.width = image->width - crop.x;
        TraceLog(LOG_WARNING, "Crop rectangle width out of bounds, rescaled crop width: %i", crop.width);
    }

    if ((crop.y + crop.height) > image->height)
    {
        crop.height = image->height - crop.y;
        TraceLog(LOG_WARNING, "Crop rectangle height out of bounds, rescaled crop height: %i", crop.height);
    }

    if ((crop.x < image->width) && (crop.y < image->height))
    {
        // Start the cropping process
        Color *pixels = GetImageData(*image);   // Get data as Color pixels array
        Color *cropPixels = (Color *)malloc(crop.width*crop.height*sizeof(Color));

        for (int j = crop.y; j < (crop.y + crop.height); j++)
        {
            for (int i = crop.x; i < (crop.x + crop.width); i++)
            {
                cropPixels[(j - crop.y)*crop.width + (i - crop.x)] = pixels[j*image->width + i];
            }
        }

        free(pixels);

        int format = image->format;

        UnloadImage(*image);

        *image = LoadImageEx(cropPixels, crop.width, crop.height);

        free(cropPixels);

        // Reformat 32bit RGBA image to original format
        ImageFormat(image, format);
    }
    else
    {
        TraceLog(LOG_WARNING, "Image can not be cropped, crop rectangle out of bounds");
    }
}

// Resize and image to new size
// NOTE: Uses stb default scaling filters (both bicubic):
// STBIR_DEFAULT_FILTER_UPSAMPLE    STBIR_FILTER_CATMULLROM
// STBIR_DEFAULT_FILTER_DOWNSAMPLE  STBIR_FILTER_MITCHELL   (high-quality Catmull-Rom)
void ImageResize(Image *image, int newWidth, int newHeight)
{
    // Get data as Color pixels array to work with it
    Color *pixels = GetImageData(*image);
    Color *output = (Color *)malloc(newWidth*newHeight*sizeof(Color));

    // NOTE: Color data is casted to (unsigned char *), there shouldn't been any problem...
    stbir_resize_uint8((unsigned char *)pixels, image->width, image->height, 0, (unsigned char *)output, newWidth, newHeight, 0, 4);

    int format = image->format;

    UnloadImage(*image);

    *image = LoadImageEx(output, newWidth, newHeight);
    ImageFormat(image, format);  // Reformat 32bit RGBA image to original format

    free(output);
    free(pixels);
}

// Resize and image to new size using Nearest-Neighbor scaling algorithm
void ImageResizeNN(Image *image,int newWidth,int newHeight)
{
    Color *pixels = GetImageData(*image);
    Color *output = (Color *)malloc(newWidth*newHeight*sizeof(Color));

    // EDIT: added +1 to account for an early rounding problem
    int xRatio = (int)((image->width << 16)/newWidth) + 1;
    int yRatio = (int)((image->height << 16)/newHeight) + 1;

    int x2, y2;
    for (int y = 0; y < newHeight; y++)
    {
        for (int x = 0; x < newWidth; x++)
        {
            x2 = ((x*xRatio) >> 16);
            y2 = ((y*yRatio) >> 16);

            output[(y*newWidth) + x] = pixels[(y2*image->width) + x2] ;
        }
    }

    int format = image->format;

    UnloadImage(*image);

    *image = LoadImageEx(output, newWidth, newHeight);
    ImageFormat(image, format);  // Reformat 32bit RGBA image to original format

    free(output);
    free(pixels);
}

// Draw an image (source) within an image (destination)
// TODO: Feel this function could be simplified...
void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec)
{
    bool cropRequired = false;

    // Security checks to avoid size and rectangle issues (out of bounds)
    // Check that srcRec is inside src image
    if (srcRec.x < 0) srcRec.x = 0;
    if (srcRec.y < 0) srcRec.y = 0;

    if ((srcRec.x + srcRec.width) > src.width)
    {
        srcRec.width = src.width - srcRec.x;
        TraceLog(LOG_WARNING, "Source rectangle width out of bounds, rescaled width: %i", srcRec.width);
    }

    if ((srcRec.y + srcRec.height) > src.height)
    {
        srcRec.height = src.height - srcRec.y;
        TraceLog(LOG_WARNING, "Source rectangle height out of bounds, rescaled height: %i", srcRec.height);
        cropRequired = true;
    }

    Image srcCopy = ImageCopy(src);     // Make a copy of source image to work with it
    ImageCrop(&srcCopy, srcRec);        // Crop source image to desired source rectangle

    // Check that dstRec is inside dst image
    // TODO: Allow negative position within destination with cropping
    if (dstRec.x < 0) dstRec.x = 0;
    if (dstRec.y < 0) dstRec.y = 0;

    // Scale source image in case destination rec size is different than source rec size
    if ((dstRec.width != srcRec.width) || (dstRec.height != srcRec.height))
    {
        ImageResize(&srcCopy, dstRec.width, dstRec.height);
    }

    if ((dstRec.x + dstRec.width) > dst->width)
    {
        dstRec.width = dst->width - dstRec.x;
        TraceLog(LOG_WARNING, "Destination rectangle width out of bounds, rescaled width: %i", dstRec.width);
        cropRequired = true;
    }

    if ((dstRec.y + dstRec.height) > dst->height)
    {
        dstRec.height = dst->height - dstRec.y;
        TraceLog(LOG_WARNING, "Destination rectangle height out of bounds, rescaled height: %i", dstRec.height);
        cropRequired = true;
    }

    if (cropRequired)
    {
        // Crop destination rectangle if out of bounds
        Rectangle crop = { 0, 0, dstRec.width, dstRec.height };
        ImageCrop(&srcCopy, crop);
    }

    // Get image data as Color pixels array to work with it
    Color *dstPixels = GetImageData(*dst);
    Color *srcPixels = GetImageData(srcCopy);

    UnloadImage(srcCopy);       // Source copy not required any more...

    Color srcCol, dstCol;

    // Blit pixels, copy source image into destination
    // TODO: Probably out-of-bounds blitting could be considered here instead of so much cropping...
    for (int j = dstRec.y; j < (dstRec.y + dstRec.height); j++)
    {
        for (int i = dstRec.x; i < (dstRec.x + dstRec.width); i++)
        {
            // Alpha blending implementation
            dstCol = dstPixels[j*dst->width + i];
            srcCol = srcPixels[(j - dstRec.y)*dstRec.width + (i - dstRec.x)];

            dstCol.r = ((srcCol.a*(srcCol.r - dstCol.r)) >> 8) + dstCol.r;
            dstCol.g = ((srcCol.a*(srcCol.g - dstCol.g)) >> 8) + dstCol.g;
            dstCol.b = ((srcCol.a*(srcCol.b - dstCol.b)) >> 8) + dstCol.b;

            dstPixels[j*dst->width + i] = dstCol;

            // TODO: Support other blending options
        }
    }

    UnloadImage(*dst);  // NOTE: Only dst->data is unloaded

    *dst = LoadImageEx(dstPixels, dst->width, dst->height);
    ImageFormat(dst, dst->format);

    free(srcPixels);
    free(dstPixels);
}

// Create an image from text (default font)
Image ImageText(const char *text, int fontSize, Color color)
{
    int defaultFontSize = 10;   // Default Font chars height in pixel
    if (fontSize < defaultFontSize) fontSize = defaultFontSize;
    int spacing = fontSize/defaultFontSize;

    Image imText = ImageTextEx(GetDefaultFont(), text, (float)fontSize, spacing, color);

    return imText;
}

// Create an image from text (custom sprite font)
Image ImageTextEx(SpriteFont font, const char *text, float fontSize, int spacing, Color tint)
{
    int length = strlen(text);
    int posX = 0;

    Vector2 imSize = MeasureTextEx(font, text, font.baseSize, spacing);

    // NOTE: glGetTexImage() not available in OpenGL ES
    Image imFont = GetTextureData(font.texture);

    ImageFormat(&imFont, UNCOMPRESSED_R8G8B8A8);    // Convert to 32 bit for color tint
    ImageColorTint(&imFont, tint);                  // Apply color tint to font

    Color *fontPixels = GetImageData(imFont);

    // Create image to store text
    // NOTE: Pixels are initialized to BLANK color (0, 0, 0, 0)
    Color *pixels = (Color *)calloc((int)imSize.x*(int)imSize.y, sizeof(Color));

    for (int i = 0; i < length; i++)
    {
        Rectangle letterRec = font.chars[(int)text[i] - 32].rec;

        for (int y = letterRec.y; y < (letterRec.y + letterRec.height); y++)
        {
            for (int x = posX; x < (posX + letterRec.width); x++)
            {
                pixels[(y - letterRec.y)*(int)imSize.x + x] = fontPixels[y*font.texture.width + (x - posX + letterRec.x)];
            }
        }

        posX += letterRec.width + spacing;
    }

    UnloadImage(imFont);

    Image imText = LoadImageEx(pixels, (int)imSize.x, (int)imSize.y);

    // Scale image depending on text size
    if (fontSize > imSize.y)
    {
        float scaleFactor = fontSize/imSize.y;
        TraceLog(LOG_INFO, "Scalefactor: %f", scaleFactor);

        // Using nearest-neighbor scaling algorithm for default font
        if (font.texture.id == GetDefaultFont().texture.id) ImageResizeNN(&imText, (int)(imSize.x*scaleFactor), (int)(imSize.y*scaleFactor));
        else ImageResize(&imText, (int)(imSize.x*scaleFactor), (int)(imSize.y*scaleFactor));
    }

    free(pixels);
    free(fontPixels);

    return imText;
}

// Draw text (default font) within an image (destination)
void ImageDrawText(Image *dst, Vector2 position, const char *text, int fontSize, Color color)
{
    // NOTE: For default font, sapcing is set to desired font size / default font size (10)
    ImageDrawTextEx(dst, position, GetDefaultFont(), text, (float)fontSize, fontSize/10, color);
}

// Draw text (custom sprite font) within an image (destination)
void ImageDrawTextEx(Image *dst, Vector2 position, SpriteFont font, const char *text, float fontSize, int spacing, Color color)
{
    Image imText = ImageTextEx(font, text, fontSize, spacing, color);

    Rectangle srcRec = { 0, 0, imText.width, imText.height };
    Rectangle dstRec = { (int)position.x, (int)position.y, imText.width, imText.height };

    ImageDraw(dst, imText, srcRec, dstRec);

    UnloadImage(imText);
}

// Flip image vertically
void ImageFlipVertical(Image *image)
{
    Color *srcPixels = GetImageData(*image);
    Color *dstPixels = (Color *)malloc(sizeof(Color)*image->width*image->height);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            dstPixels[y*image->width + x] = srcPixels[(image->height - 1 - y)*image->width + x];
        }
    }

    Image processed = LoadImageEx(dstPixels, image->width, image->height);
    ImageFormat(&processed, image->format);
    UnloadImage(*image);

    free(srcPixels);
    free(dstPixels);

    image->data = processed.data;
}

// Flip image horizontally
void ImageFlipHorizontal(Image *image)
{
    Color *srcPixels = GetImageData(*image);
    Color *dstPixels = (Color *)malloc(sizeof(Color)*image->width*image->height);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            dstPixels[y*image->width + x] = srcPixels[y*image->width + (image->width - 1 - x)];
        }
    }

    Image processed = LoadImageEx(dstPixels, image->width, image->height);
    ImageFormat(&processed, image->format);
    UnloadImage(*image);

    free(srcPixels);
    free(dstPixels);

    image->data = processed.data;
}

// Dither image data to 16bpp or lower (Floyd-Steinberg dithering)
// NOTE: In case selected bpp do not represent an known 16bit format,
// dithered data is stored in the LSB part of the unsigned short
void ImageDither(Image *image, int rBpp, int gBpp, int bBpp, int aBpp)
{
    if (image->format >= COMPRESSED_DXT1_RGB)
    {
        TraceLog(LOG_WARNING, "Compressed data formats can not be dithered");
        return;
    }

    if ((rBpp+gBpp+bBpp+aBpp) > 16)
    {
        TraceLog(LOG_WARNING, "Unsupported dithering bpps (%ibpp), only 16bpp or lower modes supported", (rBpp+gBpp+bBpp+aBpp));
    }
    else
    {
        Color *pixels = GetImageData(*image);

        free(image->data);      // free old image data

        if ((image->format != UNCOMPRESSED_R8G8B8) && (image->format != UNCOMPRESSED_R8G8B8A8))
        {
            TraceLog(LOG_WARNING, "Image format is already 16bpp or lower, dithering could have no effect");
        }

        // Define new image format, check if desired bpp match internal known format
        if ((rBpp == 5) && (gBpp == 6) && (bBpp == 5) && (aBpp == 0)) image->format = UNCOMPRESSED_R5G6B5;
        else if ((rBpp == 5) && (gBpp == 5) && (bBpp == 5) && (aBpp == 1)) image->format = UNCOMPRESSED_R5G5B5A1;
        else if ((rBpp == 4) && (gBpp == 4) && (bBpp == 4) && (aBpp == 4)) image->format = UNCOMPRESSED_R4G4B4A4;
        else
        {
            image->format = 0;
            TraceLog(LOG_WARNING, "Unsupported dithered OpenGL internal format: %ibpp (R%iG%iB%iA%i)", (rBpp+gBpp+bBpp+aBpp), rBpp, gBpp, bBpp, aBpp);
        }

        // NOTE: We will store the dithered data as unsigned short (16bpp)
        image->data = (unsigned short *)malloc(image->width*image->height*sizeof(unsigned short));

        Color oldPixel = WHITE;
        Color newPixel = WHITE;

        int rError, gError, bError;
        unsigned short rPixel, gPixel, bPixel, aPixel;   // Used for 16bit pixel composition

        #define MIN(a,b) (((a)<(b))?(a):(b))

        for (int y = 0; y < image->height; y++)
        {
            for (int x = 0; x < image->width; x++)
            {
                oldPixel = pixels[y*image->width + x];

                // NOTE: New pixel obtained by bits truncate, it would be better to round values (check ImageFormat())
                newPixel.r = oldPixel.r >> (8 - rBpp);     // R bits
                newPixel.g = oldPixel.g >> (8 - gBpp);     // G bits
                newPixel.b = oldPixel.b >> (8 - bBpp);     // B bits
                newPixel.a = oldPixel.a >> (8 - aBpp);     // A bits (not used on dithering)

                // NOTE: Error must be computed between new and old pixel but using same number of bits!
                // We want to know how much color precision we have lost...
                rError = (int)oldPixel.r - (int)(newPixel.r << (8 - rBpp));
                gError = (int)oldPixel.g - (int)(newPixel.g << (8 - gBpp));
                bError = (int)oldPixel.b - (int)(newPixel.b << (8 - bBpp));

                pixels[y*image->width + x] = newPixel;

                // NOTE: Some cases are out of the array and should be ignored
                if (x < (image->width - 1))
                {
                    pixels[y*image->width + x+1].r = MIN((int)pixels[y*image->width + x+1].r + (int)((float)rError*7.0f/16), 0xff);
                    pixels[y*image->width + x+1].g = MIN((int)pixels[y*image->width + x+1].g + (int)((float)gError*7.0f/16), 0xff);
                    pixels[y*image->width + x+1].b = MIN((int)pixels[y*image->width + x+1].b + (int)((float)bError*7.0f/16), 0xff);
                }

                if ((x > 0) && (y < (image->height - 1)))
                {
                    pixels[(y+1)*image->width + x-1].r = MIN((int)pixels[(y+1)*image->width + x-1].r + (int)((float)rError*3.0f/16), 0xff);
                    pixels[(y+1)*image->width + x-1].g = MIN((int)pixels[(y+1)*image->width + x-1].g + (int)((float)gError*3.0f/16), 0xff);
                    pixels[(y+1)*image->width + x-1].b = MIN((int)pixels[(y+1)*image->width + x-1].b + (int)((float)bError*3.0f/16), 0xff);
                }

                if (y < (image->height - 1))
                {
                    pixels[(y+1)*image->width + x].r = MIN((int)pixels[(y+1)*image->width + x].r + (int)((float)rError*5.0f/16), 0xff);
                    pixels[(y+1)*image->width + x].g = MIN((int)pixels[(y+1)*image->width + x].g + (int)((float)gError*5.0f/16), 0xff);
                    pixels[(y+1)*image->width + x].b = MIN((int)pixels[(y+1)*image->width + x].b + (int)((float)bError*5.0f/16), 0xff);
                }

                if ((x < (image->width - 1)) && (y < (image->height - 1)))
                {
                    pixels[(y+1)*image->width + x+1].r = MIN((int)pixels[(y+1)*image->width + x+1].r + (int)((float)rError*1.0f/16), 0xff);
                    pixels[(y+1)*image->width + x+1].g = MIN((int)pixels[(y+1)*image->width + x+1].g + (int)((float)gError*1.0f/16), 0xff);
                    pixels[(y+1)*image->width + x+1].b = MIN((int)pixels[(y+1)*image->width + x+1].b + (int)((float)bError*1.0f/16), 0xff);
                }

                rPixel = (unsigned short)newPixel.r;
                gPixel = (unsigned short)newPixel.g;
                bPixel = (unsigned short)newPixel.b;
                aPixel = (unsigned short)newPixel.a;

                ((unsigned short *)image->data)[y*image->width + x] = (rPixel << (gBpp + bBpp + aBpp)) | (gPixel << (bBpp + aBpp)) | (bPixel << aBpp) | aPixel;
            }
        }

        free(pixels);
    }
}

// Modify image color: tint
void ImageColorTint(Image *image, Color color)
{
    Color *pixels = GetImageData(*image);

    float cR = (float)color.r/255;
    float cG = (float)color.g/255;
    float cB = (float)color.b/255;
    float cA = (float)color.a/255;

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            unsigned char r = 255*((float)pixels[y*image->width + x].r/255*cR);
            unsigned char g = 255*((float)pixels[y*image->width + x].g/255*cG);
            unsigned char b = 255*((float)pixels[y*image->width + x].b/255*cB);
            unsigned char a = 255*((float)pixels[y*image->width + x].a/255*cA);

            pixels[y*image->width + x].r = r;
            pixels[y*image->width + x].g = g;
            pixels[y*image->width + x].b = b;
            pixels[y*image->width + x].a = a;
        }
    }

    Image processed = LoadImageEx(pixels, image->width, image->height);
    ImageFormat(&processed, image->format);
    UnloadImage(*image);
    free(pixels);

    image->data = processed.data;
}

// Modify image color: invert
void ImageColorInvert(Image *image)
{
    Color *pixels = GetImageData(*image);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            pixels[y*image->width + x].r = 255 - pixels[y*image->width + x].r;
            pixels[y*image->width + x].g = 255 - pixels[y*image->width + x].g;
            pixels[y*image->width + x].b = 255 - pixels[y*image->width + x].b;
        }
    }

    Image processed = LoadImageEx(pixels, image->width, image->height);
    ImageFormat(&processed, image->format);
    UnloadImage(*image);
    free(pixels);

    image->data = processed.data;
}

// Modify image color: grayscale
void ImageColorGrayscale(Image *image)
{
    ImageFormat(image, UNCOMPRESSED_GRAYSCALE);
}

// Modify image color: contrast
// NOTE: Contrast values between -100 and 100
void ImageColorContrast(Image *image, float contrast)
{
    if (contrast < -100) contrast = -100;
    if (contrast > 100) contrast = 100;

    contrast = (100.0f + contrast)/100.0f;
    contrast *= contrast;

    Color *pixels = GetImageData(*image);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            float pR = (float)pixels[y*image->width + x].r/255.0f;
            pR -= 0.5;
            pR *= contrast;
            pR += 0.5;
            pR *= 255;
            if (pR < 0) pR = 0;
            if (pR > 255) pR = 255;

            float pG = (float)pixels[y*image->width + x].g/255.0f;
            pG -= 0.5;
            pG *= contrast;
            pG += 0.5;
            pG *= 255;
            if (pG < 0) pG = 0;
            if (pG > 255) pG = 255;

            float pB = (float)pixels[y*image->width + x].b/255.0f;
            pB -= 0.5;
            pB *= contrast;
            pB += 0.5;
            pB *= 255;
            if (pB < 0) pB = 0;
            if (pB > 255) pB = 255;

            pixels[y*image->width + x].r = (unsigned char)pR;
            pixels[y*image->width + x].g = (unsigned char)pG;
            pixels[y*image->width + x].b = (unsigned char)pB;
        }
    }

    Image processed = LoadImageEx(pixels, image->width, image->height);
    ImageFormat(&processed, image->format);
    UnloadImage(*image);
    free(pixels);

    image->data = processed.data;
}

// Modify image color: brightness
// NOTE: Brightness values between -255 and 255
void ImageColorBrightness(Image *image, int brightness)
{
    if (brightness < -255) brightness = -255;
    if (brightness > 255) brightness = 255;

    Color *pixels = GetImageData(*image);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            int cR = pixels[y*image->width + x].r + brightness;
            int cG = pixels[y*image->width + x].g + brightness;
            int cB = pixels[y*image->width + x].b + brightness;

            if (cR < 0) cR = 1;
            if (cR > 255) cR = 255;

            if (cG < 0) cG = 1;
            if (cG > 255) cG = 255;

            if (cB < 0) cB = 1;
            if (cB > 255) cB = 255;

            pixels[y*image->width + x].r = (unsigned char)cR;
            pixels[y*image->width + x].g = (unsigned char)cG;
            pixels[y*image->width + x].b = (unsigned char)cB;
        }
    }

    Image processed = LoadImageEx(pixels, image->width, image->height);
    ImageFormat(&processed, image->format);
    UnloadImage(*image);
    free(pixels);

    image->data = processed.data;
}
#endif      // SUPPORT_IMAGE_MANIPULATION

#if defined(SUPPORT_IMAGE_GENERATION)
// Generate image: vertical gradient
Image GenImageGradientV(int width, int height, Color top, Color bottom)
{
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    for (int j = 0; j < height; j++)
    {
        float factor = (float)j/(float)height;
        for (int i = 0; i < width; i++)
        {
            pixels[j*width + i].r = (int)((float)bottom.r*factor + (float)top.r*(1.f - factor));
            pixels[j*width + i].g = (int)((float)bottom.g*factor + (float)top.g*(1.f - factor));
            pixels[j*width + i].b = (int)((float)bottom.b*factor + (float)top.b*(1.f - factor));
            pixels[j*width + i].a = (int)((float)bottom.a*factor + (float)top.a*(1.f - factor));
        }
    }

    Image image = LoadImageEx(pixels, width, height);
    free(pixels);

    return image;
}

// Generate image: horizontal gradient
Image GenImageGradientH(int width, int height, Color left, Color right)
{
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    for (int i = 0; i < width; i++)
    {
        float factor = (float)i/(float)width;
        for (int j = 0; j < height; j++)
        {
            pixels[j*width + i].r = (int)((float)right.r*factor + (float)left.r*(1.f - factor));
            pixels[j*width + i].g = (int)((float)right.g*factor + (float)left.g*(1.f - factor));
            pixels[j*width + i].b = (int)((float)right.b*factor + (float)left.b*(1.f - factor));
            pixels[j*width + i].a = (int)((float)right.a*factor + (float)left.a*(1.f - factor));
        }
    }

    Image image = LoadImageEx(pixels, width, height);
    free(pixels);

    return image;
}

// Generate image: radial gradient
Image GenImageGradientRadial(int width, int height, float density, Color inner, Color outer)
{
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));
    float radius = (width < height) ? (float)width/2.0f : (float)height/2.0f;

    float centerX = (float)width/2.0f;
    float centerY = (float)height/2.0f;
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float dist = hypotf((float)x - centerX, (float)y - centerY);
            float factor = (dist - radius*density)/(radius*(1.0f - density));
            
            factor = fmax(factor, 0.f);
            factor = fmin(factor, 1.f); // dist can be bigger than radius so we have to check
            
            pixels[y*width + x].r = (int)((float)outer.r*factor + (float)inner.r*(1.0f - factor));
            pixels[y*width + x].g = (int)((float)outer.g*factor + (float)inner.g*(1.0f - factor));
            pixels[y*width + x].b = (int)((float)outer.b*factor + (float)inner.b*(1.0f - factor));
            pixels[y*width + x].a = (int)((float)outer.a*factor + (float)inner.a*(1.0f - factor));
        }
    }

    Image image = LoadImageEx(pixels, width, height);
    free(pixels);

    return image;
}

// Generate image: checked
Image GenImageChecked(int width, int height, int checksX, int checksY, Color col1, Color col2)
{
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if ((x/checksX + y/checksY)%2 == 0) pixels[y*width + x] = col1;
            else pixels[y*width + x] = col2;
        }
    }

    Image image = LoadImageEx(pixels, width, height);
    free(pixels);

    return image;
}

// Generate image: white noise
Image GenImageWhiteNoise(int width, int height, float factor)
{
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    for (int i = 0; i < width*height; i++)
    {
        if (GetRandomValue(0, 99) < (int)(factor*100.0f)) pixels[i] = WHITE;
        else pixels[i] = BLACK;
    }

    Image image = LoadImageEx(pixels, width, height);
    free(pixels);

    return image;
}

// Generate image: perlin noise
Image GenImagePerlinNoise(int width, int height, float scale)
{
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float nx = (float)x*scale/(float)width;
            float ny = (float)y*scale/(float)height;
            
            // we need to translate the data from [-1; 1] to [0; 1]
            float p = (stb_perlin_fbm_noise3(nx, ny, 1.0f, 2.0f, 0.5f, 6, 0, 0, 0) + 1.0f) / 2.0f;
            
            int intensity = (int)(p * 255.0f);
            pixels[y*width + x] = (Color){intensity, intensity, intensity, 255};
        }
    }

    Image image = LoadImageEx(pixels, width, height);
    free(pixels);

    return image;
}

// Generate image: cellular algorithm. Bigger tileSize means bigger cells
Image GenImageCellular(int width, int height, int tileSize)
{
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    int seedsPerRow = width/tileSize;
    int seedsPerCol = height/tileSize;
    int seedsCount = seedsPerRow * seedsPerCol;

    Vector2 *seeds = (Vector2 *)malloc(seedsCount*sizeof(Vector2));

    for (int i = 0; i < seedsCount; i++)
    {
        int y = (i/seedsPerRow)*tileSize + GetRandomValue(0, tileSize - 1);
        int x = (i%seedsPerRow)*tileSize + GetRandomValue(0, tileSize - 1);
        seeds[i] = (Vector2){x, y};
    }

    for (int y = 0; y < height; y++)
    {
        int tileY = y/tileSize;
        
        for (int x = 0; x < width; x++)
        {
            int tileX = x/tileSize;

            float minDistance = strtod("Inf", NULL);

            // Check all adjacent tiles
            for (int i = -1; i < 2; i++)
            {
                if ((tileX + i < 0) || (tileX + i >= seedsPerRow)) continue;

                for (int j = -1; j < 2; j++)
                {
                    if ((tileY + j < 0) || (tileY + j >= seedsPerCol)) continue;

                    Vector2 neighborSeed = seeds[(tileY + j)*seedsPerRow + tileX + i];

                    float dist = hypot(x - (int)neighborSeed.x, y - (int)neighborSeed.y);
                    minDistance = fmin(minDistance, dist);
                }
            }

            // I made this up but it seems to give good results at all tile sizes
            int intensity = (int)(minDistance*256.0f/tileSize);
            if (intensity > 255) intensity = 255;

            pixels[y*width + x] = (Color){ intensity, intensity, intensity, 255 };
        }
    }
    
    free(seeds);

    Image image = LoadImageEx(pixels, width, height);
    free(pixels);

    return image;
}
#endif      // SUPPORT_IMAGE_GENERATION

// Generate GPU mipmaps for a texture
void GenTextureMipmaps(Texture2D *texture)
{
#if PLATFORM_WEB
    // Calculate next power-of-two values
    int potWidth = (int)powf(2, ceilf(logf((float)texture->width)/logf(2)));
    int potHeight = (int)powf(2, ceilf(logf((float)texture->height)/logf(2)));

    // Check if texture is POT
    if ((potWidth != texture->width) || (potHeight != texture->height))
    {
        TraceLog(LOG_WARNING, "Limited NPOT support, no mipmaps available for NPOT textures");
    }
    else rlGenerateMipmaps(texture);
#else
    rlGenerateMipmaps(texture);
#endif
}

// Set texture scaling filter mode
void SetTextureFilter(Texture2D texture, int filterMode)
{
    switch (filterMode)
    {
        case FILTER_POINT:
        {
            if (texture.mipmaps > 1)
            {
                // RL_FILTER_MIP_NEAREST - tex filter: POINT, mipmaps filter: POINT (sharp switching between mipmaps)
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_MIP_NEAREST);

                // RL_FILTER_NEAREST - tex filter: POINT (no filter), no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_NEAREST);
            }
            else
            {
                // RL_FILTER_NEAREST - tex filter: POINT (no filter), no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_NEAREST);
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_NEAREST);
            }
        } break;
        case FILTER_BILINEAR:
        {
            if (texture.mipmaps > 1)
            {
                // RL_FILTER_LINEAR_MIP_NEAREST - tex filter: BILINEAR, mipmaps filter: POINT (sharp switching between mipmaps)
                // Alternative: RL_FILTER_NEAREST_MIP_LINEAR - tex filter: POINT, mipmaps filter: BILINEAR (smooth transition between mipmaps)
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_LINEAR_MIP_NEAREST);

                // RL_FILTER_LINEAR - tex filter: BILINEAR, no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_LINEAR);
            }
            else
            {
                // RL_FILTER_LINEAR - tex filter: BILINEAR, no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_LINEAR);
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_LINEAR);
            }
        } break;
        case FILTER_TRILINEAR:
        {
            if (texture.mipmaps > 1)
            {
                // RL_FILTER_MIP_LINEAR - tex filter: BILINEAR, mipmaps filter: BILINEAR (smooth transition between mipmaps)
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_MIP_LINEAR);

                // RL_FILTER_LINEAR - tex filter: BILINEAR, no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_LINEAR);
            }
            else
            {
                TraceLog(LOG_WARNING, "[TEX ID %i] No mipmaps available for TRILINEAR texture filtering", texture.id);

                // RL_FILTER_LINEAR - tex filter: BILINEAR, no mipmaps
                rlTextureParameters(texture.id, RL_TEXTURE_MIN_FILTER, RL_FILTER_LINEAR);
                rlTextureParameters(texture.id, RL_TEXTURE_MAG_FILTER, RL_FILTER_LINEAR);
            }
        } break;
        case FILTER_ANISOTROPIC_4X: rlTextureParameters(texture.id, RL_TEXTURE_ANISOTROPIC_FILTER, 4); break;
        case FILTER_ANISOTROPIC_8X: rlTextureParameters(texture.id, RL_TEXTURE_ANISOTROPIC_FILTER, 8); break;
        case FILTER_ANISOTROPIC_16X: rlTextureParameters(texture.id, RL_TEXTURE_ANISOTROPIC_FILTER, 16); break;
        default: break;
    }
}

// Set texture wrapping mode
void SetTextureWrap(Texture2D texture, int wrapMode)
{
    switch (wrapMode)
    {
        case WRAP_REPEAT:
        {
            rlTextureParameters(texture.id, RL_TEXTURE_WRAP_S, RL_WRAP_REPEAT);
            rlTextureParameters(texture.id, RL_TEXTURE_WRAP_T, RL_WRAP_REPEAT);
        } break;
        case WRAP_CLAMP:
        {
            rlTextureParameters(texture.id, RL_TEXTURE_WRAP_S, RL_WRAP_CLAMP);
            rlTextureParameters(texture.id, RL_TEXTURE_WRAP_T, RL_WRAP_CLAMP);
        } break;
        case WRAP_MIRROR:
        {
            rlTextureParameters(texture.id, RL_TEXTURE_WRAP_S, RL_WRAP_CLAMP_MIRROR);
            rlTextureParameters(texture.id, RL_TEXTURE_WRAP_T, RL_WRAP_CLAMP_MIRROR);
        } break;
        default: break;
    }
}

// Draw a Texture2D
void DrawTexture(Texture2D texture, int posX, int posY, Color tint)
{
    DrawTextureEx(texture, (Vector2){ (float)posX, (float)posY }, 0.0f, 1.0f, tint);
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
    Rectangle destRec = { (int)position.x, (int)position.y, abs(sourceRec.width), abs(sourceRec.height) };
    Vector2 origin = { 0, 0 };

    DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, tint);
}

// Draw a part of a texture (defined by a rectangle) with 'pro' parameters
// NOTE: origin is relative to destination rectangle size
void DrawTexturePro(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector2 origin, float rotation, Color tint)
{
    // Check if texture is valid
    if (texture.id > 0)
    {
        if (sourceRec.width < 0) sourceRec.x -= sourceRec.width;
        if (sourceRec.height < 0) sourceRec.y -= sourceRec.height;

        rlEnableTexture(texture.id);

        rlPushMatrix();
            rlTranslatef((float)destRec.x, (float)destRec.y, 0);
            rlRotatef(rotation, 0, 0, 1);
            rlTranslatef(-origin.x, -origin.y, 0);

            rlBegin(RL_QUADS);
                rlColor4ub(tint.r, tint.g, tint.b, tint.a);
                rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

                // Bottom-left corner for texture and quad
                rlTexCoord2f((float)sourceRec.x/texture.width, (float)sourceRec.y/texture.height);
                rlVertex2f(0.0f, 0.0f);

                // Bottom-right corner for texture and quad
                rlTexCoord2f((float)sourceRec.x/texture.width, (float)(sourceRec.y + sourceRec.height)/texture.height);
                rlVertex2f(0.0f, (float)destRec.height);

                // Top-right corner for texture and quad
                rlTexCoord2f((float)(sourceRec.x + sourceRec.width)/texture.width, (float)(sourceRec.y + sourceRec.height)/texture.height);
                rlVertex2f((float)destRec.width, (float)destRec.height);

                // Top-left corner for texture and quad
                rlTexCoord2f((float)(sourceRec.x + sourceRec.width)/texture.width, (float)sourceRec.y/texture.height);
                rlVertex2f((float)destRec.width, 0.0f);
            rlEnd();
        rlPopMatrix();

        rlDisableTexture();
    }
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

#if defined(SUPPORT_FILEFORMAT_DDS)
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
    } DDSPixelFormat;

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
        DDSPixelFormat ddspf;
        unsigned int caps;
        unsigned int caps2;
        unsigned int caps3;
        unsigned int caps4;
        unsigned int reserved2;
    } DDSHeader;

    Image image = { 0 };

    FILE *ddsFile = fopen(fileName, "rb");

    if (ddsFile == NULL)
    {
        TraceLog(LOG_WARNING, "[%s] DDS file could not be opened", fileName);
    }
    else
    {
        // Verify the type of file
        char filecode[4];

        fread(filecode, 4, 1, ddsFile);

        if (strncmp(filecode, "DDS ", 4) != 0)
        {
            TraceLog(LOG_WARNING, "[%s] DDS file does not seem to be a valid image", fileName);
        }
        else
        {
            DDSHeader ddsHeader;

            // Get the image header
            fread(&ddsHeader, sizeof(DDSHeader), 1, ddsFile);

            TraceLog(LOG_DEBUG, "[%s] DDS file header size: %i", fileName, sizeof(DDSHeader));
            TraceLog(LOG_DEBUG, "[%s] DDS file pixel format size: %i", fileName, ddsHeader.ddspf.size);
            TraceLog(LOG_DEBUG, "[%s] DDS file pixel format flags: 0x%x", fileName, ddsHeader.ddspf.flags);
            TraceLog(LOG_DEBUG, "[%s] DDS file format: 0x%x", fileName, ddsHeader.ddspf.fourCC);
            TraceLog(LOG_DEBUG, "[%s] DDS file bit count: 0x%x", fileName, ddsHeader.ddspf.rgbBitCount);

            image.width = ddsHeader.width;
            image.height = ddsHeader.height;
            
            if (ddsHeader.mipmapCount == 0) image.mipmaps = 1;      // Parameter not used
            else image.mipmaps = ddsHeader.mipmapCount;

            if (ddsHeader.ddspf.rgbBitCount == 16)     // 16bit mode, no compressed
            {
                if (ddsHeader.ddspf.flags == 0x40)         // no alpha channel
                {
                    image.data = (unsigned short *)malloc(image.width*image.height*sizeof(unsigned short));
                    fread(image.data, image.width*image.height*sizeof(unsigned short), 1, ddsFile);

                    image.format = UNCOMPRESSED_R5G6B5;
                }
                else if (ddsHeader.ddspf.flags == 0x41)        // with alpha channel
                {
                    if (ddsHeader.ddspf.aBitMask == 0x8000)    // 1bit alpha
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
                    else if (ddsHeader.ddspf.aBitMask == 0xf000)   // 4bit alpha
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
            if (ddsHeader.ddspf.flags == 0x40 && ddsHeader.ddspf.rgbBitCount == 24)   // DDS_RGB, no compressed
            {
                // NOTE: not sure if this case exists...
                image.data = (unsigned char *)malloc(image.width*image.height*3*sizeof(unsigned char));
                fread(image.data, image.width*image.height*3, 1, ddsFile);

                image.format = UNCOMPRESSED_R8G8B8;
            }
            else if (ddsHeader.ddspf.flags == 0x41 && ddsHeader.ddspf.rgbBitCount == 32) // DDS_RGBA, no compressed
            {
                image.data = (unsigned char *)malloc(image.width*image.height*4*sizeof(unsigned char));
                fread(image.data, image.width*image.height*4, 1, ddsFile);

                unsigned char blue = 0;

                // NOTE: Data comes as A8R8G8B8, it must be reordered R8G8B8A8 (view next comment)
                // DirecX understand ARGB as a 32bit DWORD but the actual memory byte alignment is BGRA
                // So, we must realign B8G8R8A8 to R8G8B8A8
                for (int i = 0; i < image.width*image.height*4; i += 4)
                {
                    blue = ((unsigned char *)image.data)[i];
                    ((unsigned char *)image.data)[i] = ((unsigned char *)image.data)[i + 2];
                    ((unsigned char *)image.data)[i + 2] = blue;
                }

                image.format = UNCOMPRESSED_R8G8B8A8;
            }
            else if (((ddsHeader.ddspf.flags == 0x04) || (ddsHeader.ddspf.flags == 0x05)) && (ddsHeader.ddspf.fourCC > 0)) // Compressed
            {
                int size;       // DDS image data size

                // Calculate data size, including all mipmaps
                if (ddsHeader.mipmapCount > 1) size = ddsHeader.pitchOrLinearSize*2;
                else size = ddsHeader.pitchOrLinearSize;

                TraceLog(LOG_DEBUG, "Pitch or linear size: %i", ddsHeader.pitchOrLinearSize);

                image.data = (unsigned char*)malloc(size*sizeof(unsigned char));

                fread(image.data, size, 1, ddsFile);

                switch (ddsHeader.ddspf.fourCC)
                {
                    case FOURCC_DXT1:
                    {
                        if (ddsHeader.ddspf.flags == 0x04) image.format = COMPRESSED_DXT1_RGB;
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
#endif

#if defined(SUPPORT_FILEFORMAT_PKM)
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
    } PKMHeader;

    // Formats list
    // version 10: format: 0=ETC1_RGB, [1=ETC1_RGBA, 2=ETC1_RGB_MIP, 3=ETC1_RGBA_MIP] (not used)
    // version 20: format: 0=ETC1_RGB, 1=ETC2_RGB, 2=ETC2_RGBA_OLD, 3=ETC2_RGBA, 4=ETC2_RGBA1, 5=ETC2_R, 6=ETC2_RG, 7=ETC2_SIGNED_R, 8=ETC2_SIGNED_R

    // NOTE: The extended width and height are the widths rounded up to a multiple of 4.
    // NOTE: ETC is always 4bit per pixel (64 bit for each 4x4 block of pixels)

    Image image = { 0 };

    FILE *pkmFile = fopen(fileName, "rb");

    if (pkmFile == NULL)
    {
        TraceLog(LOG_WARNING, "[%s] PKM file could not be opened", fileName);
    }
    else
    {
        PKMHeader pkmHeader;

        // Get the image header
        fread(&pkmHeader, sizeof(PKMHeader), 1, pkmFile);

        if (strncmp(pkmHeader.id, "PKM ", 4) != 0)
        {
            TraceLog(LOG_WARNING, "[%s] PKM file does not seem to be a valid image", fileName);
        }
        else
        {
            // NOTE: format, width and height come as big-endian, data must be swapped to little-endian
            pkmHeader.format = ((pkmHeader.format & 0x00FF) << 8) | ((pkmHeader.format & 0xFF00) >> 8);
            pkmHeader.width = ((pkmHeader.width & 0x00FF) << 8) | ((pkmHeader.width & 0xFF00) >> 8);
            pkmHeader.height = ((pkmHeader.height & 0x00FF) << 8) | ((pkmHeader.height & 0xFF00) >> 8);

            TraceLog(LOG_DEBUG, "PKM (ETC) image width: %i", pkmHeader.width);
            TraceLog(LOG_DEBUG, "PKM (ETC) image height: %i", pkmHeader.height);
            TraceLog(LOG_DEBUG, "PKM (ETC) image format: %i", pkmHeader.format);

            image.width = pkmHeader.width;
            image.height = pkmHeader.height;
            image.mipmaps = 1;

            int bpp = 4;
            if (pkmHeader.format == 3) bpp = 8;

            int size = image.width*image.height*bpp/8;  // Total data size in bytes

            image.data = (unsigned char*)malloc(size*sizeof(unsigned char));

            fread(image.data, size, 1, pkmFile);

            if (pkmHeader.format == 0) image.format = COMPRESSED_ETC1_RGB;
            else if (pkmHeader.format == 1) image.format = COMPRESSED_ETC2_RGB;
            else if (pkmHeader.format == 3) image.format = COMPRESSED_ETC2_EAC_RGBA;
        }

        fclose(pkmFile);    // Close file pointer
    }

    return image;
}
#endif

#if defined(SUPPORT_FILEFORMAT_KTX)
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
    } KTXHeader;

    // NOTE: Before start of every mipmap data block, we have: unsigned int dataSize

    Image image = { 0 };

    FILE *ktxFile = fopen(fileName, "rb");

    if (ktxFile == NULL)
    {
        TraceLog(LOG_WARNING, "[%s] KTX image file could not be opened", fileName);
    }
    else
    {
        KTXHeader ktxHeader;

        // Get the image header
        fread(&ktxHeader, sizeof(KTXHeader), 1, ktxFile);

        if ((ktxHeader.id[1] != 'K') || (ktxHeader.id[2] != 'T') || (ktxHeader.id[3] != 'X') ||
            (ktxHeader.id[4] != ' ') || (ktxHeader.id[5] != '1') || (ktxHeader.id[6] != '1'))
        {
            TraceLog(LOG_WARNING, "[%s] KTX file does not seem to be a valid file", fileName);
        }
        else
        {
            image.width = ktxHeader.width;
            image.height = ktxHeader.height;
            image.mipmaps = ktxHeader.mipmapLevels;

            TraceLog(LOG_DEBUG, "KTX (ETC) image width: %i", ktxHeader.width);
            TraceLog(LOG_DEBUG, "KTX (ETC) image height: %i", ktxHeader.height);
            TraceLog(LOG_DEBUG, "KTX (ETC) image format: 0x%x", ktxHeader.glInternalFormat);

            unsigned char unused;

            if (ktxHeader.keyValueDataSize > 0)
            {
                for (int i = 0; i < ktxHeader.keyValueDataSize; i++) fread(&unused, sizeof(unsigned char), 1, ktxFile);
            }

            int dataSize;
            fread(&dataSize, sizeof(unsigned int), 1, ktxFile);

            image.data = (unsigned char*)malloc(dataSize*sizeof(unsigned char));

            fread(image.data, dataSize, 1, ktxFile);

            if (ktxHeader.glInternalFormat == 0x8D64) image.format = COMPRESSED_ETC1_RGB;
            else if (ktxHeader.glInternalFormat == 0x9274) image.format = COMPRESSED_ETC2_RGB;
            else if (ktxHeader.glInternalFormat == 0x9278) image.format = COMPRESSED_ETC2_EAC_RGBA;
        }

        fclose(ktxFile);    // Close file pointer
    }

    return image;
}
#endif

#if defined(SUPPORT_FILEFORMAT_PVR)
// Loading PVR image data (uncompressed or PVRT compression)
// NOTE: PVR v2 not supported, use PVR v3 instead
static Image LoadPVR(const char *fileName)
{
    // Required extension:
    // GL_IMG_texture_compression_pvrtc

    // Supported tokens (defined by extensions)
    // GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG       0x8C00
    // GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG      0x8C02

#if 0   // Not used...
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
    } PVRHeaderV2;
#endif

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
    } PVRHeaderV3;

#if 0   // Not used...
    // Metadata (usually 15 bytes)
    typedef struct {
        unsigned int devFOURCC;
        unsigned int key;
        unsigned int dataSize;      // Not used?
        unsigned char *data;        // Not used?
    } PVRMetadata;
#endif

    Image image = { 0 };

    FILE *pvrFile = fopen(fileName, "rb");

    if (pvrFile == NULL)
    {
        TraceLog(LOG_WARNING, "[%s] PVR file could not be opened", fileName);
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
            PVRHeaderV3 pvrHeader;

            // Get PVR image header
            fread(&pvrHeader, sizeof(PVRHeaderV3), 1, pvrFile);

            if ((pvrHeader.id[0] != 'P') || (pvrHeader.id[1] != 'V') || (pvrHeader.id[2] != 'R') || (pvrHeader.id[3] != 3))
            {
                TraceLog(LOG_WARNING, "[%s] PVR file does not seem to be a valid image", fileName);
            }
            else
            {
                image.width = pvrHeader.width;
                image.height = pvrHeader.height;
                image.mipmaps = pvrHeader.numMipmaps;

                // Check data format
                if (((pvrHeader.channels[0] == 'l') && (pvrHeader.channels[1] == 0)) && (pvrHeader.channelDepth[0] == 8))
                    image.format = UNCOMPRESSED_GRAYSCALE;
                else if (((pvrHeader.channels[0] == 'l') && (pvrHeader.channels[1] == 'a')) && ((pvrHeader.channelDepth[0] == 8) && (pvrHeader.channelDepth[1] == 8)))
                    image.format = UNCOMPRESSED_GRAY_ALPHA;
                else if ((pvrHeader.channels[0] == 'r') && (pvrHeader.channels[1] == 'g') && (pvrHeader.channels[2] == 'b'))
                {
                    if (pvrHeader.channels[3] == 'a')
                    {
                        if ((pvrHeader.channelDepth[0] == 5) && (pvrHeader.channelDepth[1] == 5) && (pvrHeader.channelDepth[2] == 5) && (pvrHeader.channelDepth[3] == 1))
                            image.format = UNCOMPRESSED_R5G5B5A1;
                        else if ((pvrHeader.channelDepth[0] == 4) && (pvrHeader.channelDepth[1] == 4) && (pvrHeader.channelDepth[2] == 4) && (pvrHeader.channelDepth[3] == 4))
                            image.format = UNCOMPRESSED_R4G4B4A4;
                        else if ((pvrHeader.channelDepth[0] == 8) && (pvrHeader.channelDepth[1] == 8) && (pvrHeader.channelDepth[2] == 8) && (pvrHeader.channelDepth[3] == 8))
                            image.format = UNCOMPRESSED_R8G8B8A8;
                    }
                    else if (pvrHeader.channels[3] == 0)
                    {
                        if ((pvrHeader.channelDepth[0] == 5) && (pvrHeader.channelDepth[1] == 6) && (pvrHeader.channelDepth[2] == 5)) image.format = UNCOMPRESSED_R5G6B5;
                        else if ((pvrHeader.channelDepth[0] == 8) && (pvrHeader.channelDepth[1] == 8) && (pvrHeader.channelDepth[2] == 8)) image.format = UNCOMPRESSED_R8G8B8;
                    }
                }
                else if (pvrHeader.channels[0] == 2) image.format = COMPRESSED_PVRT_RGB;
                else if (pvrHeader.channels[0] == 3) image.format = COMPRESSED_PVRT_RGBA;

                // Skip meta data header
                unsigned char unused = 0;
                for (int i = 0; i < pvrHeader.metaDataSize; i++) fread(&unused, sizeof(unsigned char), 1, pvrFile);

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
        else if (pvrVersion == 52) TraceLog(LOG_INFO, "PVR v2 not supported, update your files to PVR v3");

        fclose(pvrFile);    // Close file pointer
    }

    return image;
}
#endif

#if defined(SUPPORT_FILEFORMAT_ASTC)
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
        unsigned char length[3];    // Image Z-size (1 for 2D images)
    } ASTCHeader;

    Image image = { 0 };

    FILE *astcFile = fopen(fileName, "rb");

    if (astcFile == NULL)
    {
        TraceLog(LOG_WARNING, "[%s] ASTC file could not be opened", fileName);
    }
    else
    {
        ASTCHeader astcHeader;

        // Get ASTC image header
        fread(&astcHeader, sizeof(ASTCHeader), 1, astcFile);

        if ((astcHeader.id[3] != 0x5c) || (astcHeader.id[2] != 0xa1) || (astcHeader.id[1] != 0xab) || (astcHeader.id[0] != 0x13))
        {
            TraceLog(LOG_WARNING, "[%s] ASTC file does not seem to be a valid image", fileName);
        }
        else
        {
            // NOTE: Assuming Little Endian (could it be wrong?)
            image.width = 0x00000000 | ((int)astcHeader.width[2] << 16) | ((int)astcHeader.width[1] << 8) | ((int)astcHeader.width[0]);
            image.height = 0x00000000 | ((int)astcHeader.height[2] << 16) | ((int)astcHeader.height[1] << 8) | ((int)astcHeader.height[0]);

            TraceLog(LOG_DEBUG, "ASTC image width: %i", image.width);
            TraceLog(LOG_DEBUG, "ASTC image height: %i", image.height);
            TraceLog(LOG_DEBUG, "ASTC image blocks: %ix%i", astcHeader.blockX, astcHeader.blockY);
            
            image.mipmaps = 1;      // NOTE: ASTC format only contains one mipmap level

            // NOTE: Each block is always stored in 128bit so we can calculate the bpp
            int bpp = 128/(astcHeader.blockX*astcHeader.blockY);

            // NOTE: Currently we only support 2 blocks configurations: 4x4 and 8x8
            if ((bpp == 8) || (bpp == 2))
            {
                int dataSize = image.width*image.height*bpp/8;  // Data size in bytes

                image.data = (unsigned char *)malloc(dataSize*sizeof(unsigned char));
                fread(image.data, dataSize, 1, astcFile);

                if (bpp == 8) image.format = COMPRESSED_ASTC_4x4_RGBA;
                else if (bpp == 2) image.format = COMPRESSED_ASTC_4x4_RGBA;
            }
            else TraceLog(LOG_WARNING, "[%s] ASTC block size configuration not supported", fileName);
        }

        fclose(astcFile);
    }

    return image;
}
#endif
