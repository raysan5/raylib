/*********************************************************************************************
*
*   raylib.textures
*
*   Basic functions to load and draw Textures (2d)
*    
*   Uses external lib:
*       stb_image - Multiple formats image loading (JPEG, PNG, BMP, TGA, PSD, GIF, PIC)
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

#include "raylib.h"

#include <stdlib.h>          // Declares malloc() and free() for memory management
#include <string.h>          // Required for strcmp(), strrchr(), strncmp()
#include "stb_image.h"       // Used to read image data (multiple formats support)
#include "utils.h"           // rRES data decompression utility function

#include "rlgl.h"            // raylib OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2

// Security check in case no USE_OPENGL_* defined
#if !defined(USE_OPENGL_11) && !defined(USE_OPENGL_33) && !defined(USE_OPENGL_ES2)
    #define USE_OPENGL_11
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef unsigned char byte;

typedef struct {
    unsigned char *data;    // Image raw data
    int width;              // Image base width
    int height;             // Image base height
    //int bpp;              // bytes per pixel
    //int components;       // num color components
    int mipmaps;            // Mipmap levels, 1 by default
    int compFormat;         // Compressed data format, 0 if no compression
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
static ImageEx LoadDDS(const char *fileName);

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
            
            TraceLog(INFO, "[%s] Image loaded successfully", fileName);
        }
        else TraceLog(WARNING, "[%s] Image could not be loaded", fileName);
    }
    else if (strcmp(GetExtension(fileName),"dds") == 0)
    {
        // NOTE: DDS uncompressed images can also be loaded (discarding mipmaps...)
        
        ImageEx imageDDS = LoadDDS(fileName);
        
        if (imageDDS.compFormat == 0)
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
            
            TraceLog(INFO, "[%s] Image loaded successfully", fileName);
        }
        else TraceLog(WARNING, "[%s] Compressed image data could not be loaded", fileName);    
    }
    else TraceLog(WARNING, "[%s] Image extension not recognized, it can't be loaded", fileName);
    
    // ALTERNATIVE: We can load pixel data directly into Color struct pixels array, 
    // to do that struct data alignment should be the right one (4 byte); it is.
    //image.pixels = stbi_load(fileName, &imgWidth, &imgHeight, &imgBpp, 4);

    return image;
}

// Load an image from rRES file (raylib Resource)
Image LoadImageFromRES(const char *rresName, int resId)
{
    // TODO: rresName could be directly a char array with all the data! --> support it! :P
    
    Image image;
    bool found = false;

    char id[4];             // rRES file identifier
    unsigned char version;  // rRES file version and subversion
    char useless;           // rRES header reserved data
    short numRes;
    
    ResInfoHeader infoHeader;
    
    FILE *rresFile = fopen(rresName, "rb");

    if (!rresFile) TraceLog(WARNING, "[%s] Could not open raylib resource file", rresName);
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

    if (strcmp(GetExtension(fileName),"dds") == 0)
    {
        ImageEx image = LoadDDS(fileName);
        
        if (image.compFormat == 0)
        {
            texture.id = rlglLoadTexture(image.data, image.width, image.height, false);
        }
        else
        {
#if defined(USE_OPENGL_33) || defined(USE_OPENGL_ES2)
            texture.id = rlglLoadCompressedTexture(image.data, image.width, image.height, image.mipmaps, image.compFormat);
#endif
        }
        
        texture.width = image.width;
        texture.height = image.height;
        
        if (texture.id == 0) TraceLog(WARNING, "[%s] DDS texture could not be loaded", fileName);
        else TraceLog(INFO, "[%s] DDS texture loaded successfully", fileName);
        
        free(image.data);
    }
    else
    {
        Image image = LoadImage(fileName);
        
        if (image.pixels != NULL)
        {
            texture = CreateTexture(image, false);
            UnloadImage(image);
        }
    }
    
    return texture;
}

// Load an image as texture from rRES file (raylib Resource)
Texture2D LoadTextureFromRES(const char *rresName, int resId)
{
    Texture2D texture;

    Image image = LoadImageFromRES(rresName, resId);
    texture = CreateTexture(image, false);
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
    
    DrawTexturePro(texture, sourceRec, destRec, origin, 0, tint);
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

// Create a texture from an image
// NOTE: image is not unloaded, iot must be done manually
Texture2D CreateTexture(Image image, bool genMipmaps)
{
    Texture2D texture;
    
    // Init texture to default values
    texture.id = 0;
    texture.width = 0;
    texture.height = 0;
    
    if (image.pixels != NULL)
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
        texture.id = rlglLoadTexture(imgData, image.width, image.height, genMipmaps);

        texture.width = image.width;
        texture.height = image.height;
        
        TraceLog(INFO, "[ID %i] Texture created successfully", texture.id);
        
        free(imgData);
    }
    else TraceLog(WARNING, "Texture could not be created, image data is not valid");
    
    return texture;
}

// Loading DDS image data (compressed or uncompressed)
// NOTE: Compressed data loading not supported on OpenGL 1.1
ImageEx LoadDDS(const char *fileName)
{   
    #define FOURCC_DXT1 0x31545844  // Equivalent to "DXT1" in ASCII
    #define FOURCC_DXT3 0x33545844  // Equivalent to "DXT3" in ASCII
    #define FOURCC_DXT5 0x35545844  // Equivalent to "DXT5" in ASCII
    
    #ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
    #endif

    #ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
    #endif

    #ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
    #endif
    
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

    FILE *ddsFile = fopen(fileName, "rb");
    
    if (ddsFile == NULL)
    {
        TraceLog(WARNING, "DDS File could not be opened");
    }
    else
    {
        // Verify the type of file
        char filecode[4];
        
        fread(filecode, 1, 4, ddsFile);
        
        if (strncmp(filecode, "DDS ", 4) != 0) 
        { 
            TraceLog(WARNING, "DDS File does not seem to be valid");
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
            image.mipmaps = 1;
            image.compFormat = 0;
            
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
            }
            else if (header.ddspf.flags == 0x41 && header.ddspf.rgbBitCount == 32) // DDS_RGBA, no compressed
            {
                image.data = (unsigned char *)malloc(header.width * header.height * 4);
            
                fread(image.data, image.width*image.height*4, 1, ddsFile);
            
                image.mipmaps = 1;
                image.compFormat = 0;
            }
            else if ((header.ddspf.flags == 0x04) && (header.ddspf.fourCC > 0))
            {
#ifdef USE_OPENGL_11 
                TraceLog(WARNING, "[%s] DDS image uses compression, not supported by current OpenGL version", fileName);
                TraceLog(WARNING, "[%s] DDS compressed files require OpenGL 3.2+ or ES 2.0", fileName);
                fclose(ddsFile);
#else
                int bufsize;
                
                // Calculate data size, including all mipmaps
                if (header.mipMapCount > 1) bufsize = header.pitchOrLinearSize * 2;
                else bufsize = header.pitchOrLinearSize; 
                
                image.data = (unsigned char*)malloc(bufsize * sizeof(unsigned char)); 
                
                fread(image.data, 1, bufsize, ddsFile); 
                
                // Close file pointer
                fclose(ddsFile);
                
                image.mipmaps = header.mipMapCount;
                image.compFormat = 0;
           
                switch(header.ddspf.fourCC)
                { 
                    case FOURCC_DXT1: image.compFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break; 
                    case FOURCC_DXT3: image.compFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break; 
                    case FOURCC_DXT5: image.compFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break; 
                    default: break;
                }
            
                // NOTE: Image num color components not required... for now...
                //if (fourCC == FOURCC_DXT1) image.components = 3;
                //else image.components = 4;
#endif
            }
        }
    }
    
    return image;
}