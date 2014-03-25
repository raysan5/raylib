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
#include "stb_image.h"       // Used to read image data (multiple formats support)

#include "utils.h"           // rRES data decompression utility function

#include "rlgl.h"            // raylib OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef unsigned char byte;

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
// No private (static) functions in this module (.c file)

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Load an image into CPU memory (RAM)
Image LoadImage(const char *fileName)
{
    Image image;
    
    int imgWidth;
    int imgHeight;
    int imgBpp;
    
    // NOTE: Using stb_image to load images (Supports: BMP, TGA, PNG, JPG, ...)
    // Force loading to 4 components (RGBA)
    byte *imgData = stbi_load(fileName, &imgWidth, &imgHeight, &imgBpp, 4);    
    
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
    
    // ALTERNATIVE: We can load pixel data directly into Color struct pixels array, 
    // to do that struct data alignment should be the right one (4 byte); it is.
    //image.pixels = stbi_load(fileName, &imgWidth, &imgHeight, &imgBpp, 4);

    return image;
}

// Load an image from rRES file (raylib Resource)
Image LoadImageFromRES(const char *rresName, int resId)
{
    // NOTE: rresName could be directly a char array with all the data!!! ---> TODO!
    Image image;
    bool found = false;

    char id[4];             // rRES file identifier
    unsigned char version;  // rRES file version and subversion
    char useless;           // rRES header reserved data
    short numRes;
    
    ResInfoHeader infoHeader;
    
    FILE *rresFile = fopen(rresName, "rb");

    if (!rresFile) printf("Error opening raylib Resource file\n");
    
    // Read rres file (basic file check - id)
    fread(&id[0], sizeof(char), 1, rresFile);
    fread(&id[1], sizeof(char), 1, rresFile);
    fread(&id[2], sizeof(char), 1, rresFile);
    fread(&id[3], sizeof(char), 1, rresFile);
    fread(&version, sizeof(char), 1, rresFile);
    fread(&useless, sizeof(char), 1, rresFile);
    
    if ((id[0] != 'r') && (id[1] != 'R') && (id[2] != 'E') &&(id[3] != 'S'))
    {
        printf("This is not a valid raylib Resource file!\n");
        exit(1);
    }
    
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
        
                printf("Image width: %i\n", (int)imgWidth);
                printf("Image height: %i\n", (int)imgHeight);
                
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
            }
            else
            {
                printf("Required resource do not seem to be a valid IMAGE resource\n");
                exit(2);
            }
        }
        else
        {
            // Depending on type, skip the right amount of parameters
            switch (infoHeader.type)
            {
                case 0: fseek(rresFile, 6, SEEK_CUR); break;   // IMAGE: Jump 6 bytes of parameters
                case 1: fseek(rresFile, 6, SEEK_CUR); break;   // SOUND: Jump 6 bytes of parameters
                case 2: fseek(rresFile, 5, SEEK_CUR); break;   // MODEL: Jump 5 bytes of parameters (TODO: Review)
                case 3: break;   // TEXT: No parameters
                case 4: break;   // RAW: No parameters
                default: break;
            }
            
            // Jump DATA to read next infoHeader
            fseek(rresFile, infoHeader.size, SEEK_CUR);
        }    
    }
    
    fclose(rresFile);
    
    if (!found) printf("Required resource id could not be found in the raylib Resource file!\n");
    
    return image;
}

// Load an image as texture into GPU memory
Texture2D LoadTexture(const char *fileName)
{
    Texture2D texture;
    Image image;
    
    image = LoadImage(fileName);
    texture = CreateTexture(image);
    UnloadImage(image);
    
    return texture;
}

// Load an image as texture from rRES file (raylib Resource)
Texture2D LoadTextureFromRES(const char *rresName, int resId)
{
    Texture2D texture;

    Image image = LoadImageFromRES(rresName, resId);
    texture = CreateTexture(image);
    
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
    rlDeleteTextures(texture.glId);
}

// Draw a Texture2D
void DrawTexture(Texture2D texture, int posX, int posY, Color tint)
{
    DrawTextureEx(texture, (Vector2){ (float)posX, (float)posY}, 0, 1.0f, tint);
}

// Draw a Texture2D with position defined as Vector2
void DrawTextureV(Texture2D texture, Vector2 position, Color tint)
{
    DrawTextureEx(texture, position, 0, 1.0f, tint);
}

// Draw a Texture2D with extended parameters
void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint)
{
    rlEnableTexture(texture.glId);
    
    // TODO: Apply rotation to vertex! --> rotate from origin CW (0, 0)
    // TODO: Compute vertex scaling!
    
    // NOTE: Rotation is applied before translation and scaling, even being called in inverse order...
    // NOTE: Rotation point is upper-left corner
    //rlTranslatef(position.x, position.y, 0);
    //rlScalef(scale, scale, 1.0f);
    //rlRotatef(rotation, 0, 0, 1);
        
    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f);                               // Normal vector pointing towards viewer
        
        rlTexCoord2f(0.0f, 0.0f);
        rlVertex2f(position.x, position.y);                         // Bottom-left corner for texture and quad
        
        rlTexCoord2f(0.0f, 1.0f); 
        rlVertex2f(position.x, position.y + texture.height);        // Bottom-right corner for texture and quad
        
        rlTexCoord2f(1.0f, 1.0f); 
        rlVertex2f(position.x + texture.width, position.y + texture.height);  // Top-right corner for texture and quad
        
        rlTexCoord2f(1.0f, 0.0f); 
        rlVertex2f(position.x + texture.width, position.y);         // Top-left corner for texture and quad
    rlEnd();
    
    rlDisableTexture();
}

// Draw a part of a texture (defined by a rectangle)
void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint)
{
    rlEnableTexture(texture.glId);
    
    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer
        
        // Bottom-left corner for texture and quad
        rlTexCoord2f((float)sourceRec.x / texture.width, (float)sourceRec.y / texture.height); 
        rlVertex2f(position.x, position.y);
        
        // Bottom-right corner for texture and quad
        rlTexCoord2f((float)sourceRec.x / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height);
        rlVertex2f(position.x, position.y + sourceRec.height);
        
        // Top-right corner for texture and quad
        rlTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height); 
        rlVertex2f(position.x + sourceRec.width, position.y + sourceRec.height);
        
        // Top-left corner for texture and quad 
        rlTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)sourceRec.y / texture.height);
        rlVertex2f(position.x + sourceRec.width, position.y);
    rlEnd();
    
    rlDisableTexture();
}

// Draw a part of a texture (defined by a rectangle) with 'pro' parameters
// TODO: Test this function...
void DrawTexturePro(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector2 origin, float rotation, Color tint)
{
    rlEnableTexture(texture.glId);
    
    // TODO: Apply translation, rotation and scaling of vertex manually!
    
    //rlTranslatef(-origin.x, -origin.y, 0);
    //rlRotatef(rotation, 0, 0, 1);
    //rlTranslatef(destRec.x + origin.x, destRec.y + origin.y, 0);
        
    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer
        
        // Bottom-left corner for texture and quad
        rlTexCoord2f((float)sourceRec.x / texture.width, (float)sourceRec.y / texture.height); 
        rlVertex2f(0.0f, 0.0f);
        
        // Bottom-right corner for texture and quad
        rlTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)sourceRec.y / texture.height);
        rlVertex2f(destRec.width, 0.0f);
        
        // Top-right corner for texture and quad
        rlTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height); 
        rlVertex2f(destRec.width, destRec.height);
        
        // Top-left corner for texture and quad
        rlTexCoord2f((float)sourceRec.x / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height);
        rlVertex2f(0.0f, destRec.height);
    rlEnd();
    
    rlDisableTexture();
}

Texture2D CreateTexture(Image image)
{
    Texture2D texture;
    
    unsigned char *img = malloc(image.width * image.height * 4);
    
    int j = 0;
    
    for (int i = 0; i < image.width * image.height * 4; i += 4)
    {
        img[i] = image.pixels[j].r;
        img[i+1] = image.pixels[j].g;
        img[i+2] = image.pixels[j].b;
        img[i+3] = image.pixels[j].a;
        
        j++;
    }

    texture.glId = rlglTexture(image.width, image.height, img);

    texture.width = image.width;
    texture.height = image.height;
    
    free(img);
    
    return texture;
}