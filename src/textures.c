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

#include <GL/gl.h>           // OpenGL functions
#include <stdlib.h>          // Declares malloc() and free() for memory management
#include "stb_image.h"       // Used to read image data (multiple formats support)

#include "utils.h"           // rRES data decompression utility function

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

    int imgWidth;
    int imgHeight;
    int imgBpp;
    
    // NOTE: Using stb_image to load images (Supports: BMP, TGA, PNG, JPG, ...)
    // Force loading to 4 components (RGBA)
    byte *imgData = stbi_load(fileName, &imgWidth, &imgHeight, &imgBpp, 4);    
    
    // Convert loaded data to OpenGL texture
    //----------------------------------------
    GLuint id;
    glGenTextures(1, &id);         // Generate Pointer to the Texture
    
    glBindTexture(GL_TEXTURE_2D, id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repead on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repead on y-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Filter for pixel-perfect drawing, alternative: GL_LINEAR 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Filter for pixel-perfect drawing, alternative: GL_LINEAR
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData);
    
    // NOTE: Not using mipmappings (texture for 2D drawing)
    // At this point we have the image converted to texture and uploaded to GPU
    
    stbi_image_free(imgData);      // Now we can free loaded data from RAM memory
    
    texture.glId = id;
    texture.width = imgWidth;
    texture.height = imgHeight;
    
    return texture;
}

// Load an image as texture from rRES file (raylib Resource)
Texture2D LoadTextureFromRES(const char *rresName, int resId)
{
    Texture2D texture;

    Image image = LoadImageFromRES(rresName, resId);
    texture = CreateTexture2D(image);
    
    return texture;
}

// Load an image as texture (and convert to POT with mipmaps)
Texture2D LoadTextureEx(const char *fileName, bool createPOT, bool mipmaps)
{
    Texture2D texture;
    
    // TODO: Load and image and convert to Power-Of-Two
    // NOTE: Conversion could be done just adding extra space to image or by scaling image
    // NOTE: If scaling image, be careful with scaling algorithm (aproximation, bilinear, bicubic...)
    
    // TODO: Generate all required mipmap levels from image and convert to testure (not that easy)
    // NOTE: If using OpenGL 1.1, the only option is doing mipmap generation on CPU side (i.e. gluBuild2DMipmaps)
    // NOTE: raylib tries to minimize external dependencies so, we are not using GLU
    // NOTE: Re-implement some function similar to gluBuild2DMipmaps (not that easy...)
    
    return texture;
}

// Create a Texture2D from Image data
// NOTE: Image is not unloaded, it should be done manually...
Texture2D CreateTexture2D(Image image)
{
    Texture2D texture;
    
    // Convert image data to OpenGL texture
    //----------------------------------------
    GLuint id;
    glGenTextures(1, &id);         // Generate Pointer to the Texture
    
    glBindTexture(GL_TEXTURE_2D, id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repead on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repead on y-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Filter for pixel-perfect drawing, alternative: GL_LINEAR 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Filter for pixel-perfect drawing, alternative: GL_LINEAR
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);
    
    // NOTE: Not using mipmappings (texture for 2D drawing)
    // At this point we have the image converted to texture and uploaded to GPU
    
    texture.glId = id;
    texture.width = image.width;
    texture.height = image.height;
    
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
    glDeleteTextures(1, &texture.glId);
}

// Draw a Texture2D
void DrawTexture(Texture2D texture, int posX, int posY, Color tint)
{
    DrawTextureEx(texture, (Vector2){ (float)posX, (float)posY}, 0, 1.0f, tint);
}

// Draw a Texture2D with extended parameters
void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint)
{
    glEnable(GL_TEXTURE_2D);    // Enable textures usage
    
    glBindTexture(GL_TEXTURE_2D, texture.glId);
    
    glPushMatrix();
        // NOTE: Rotation is applied before translation and scaling, even being called in inverse order...
        // NOTE: Rotation point is upper-left corner
        glTranslatef(position.x, position.y, 0);
        glScalef(scale, scale, 1.0f);
        glRotatef(rotation, 0, 0, 1);
        
        glBegin(GL_QUADS);
            glColor4ub(tint.r, tint.g, tint.b, tint.a);
            glNormal3f(0.0f, 0.0f, 1.0f);                                         // Normal vector pointing towards viewer
            glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);                     // Bottom-left corner for texture and quad
            glTexCoord2f(1.0f, 0.0f); glVertex2f(texture.width, 0.0f);            // Bottom-right corner for texture and quad
            glTexCoord2f(1.0f, 1.0f); glVertex2f(texture.width, texture.height);  // Top-right corner for texture and quad
            glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, texture.height);           // Top-left corner for texture and quad
        glEnd();
    glPopMatrix();
    
    glDisable(GL_TEXTURE_2D);    // Disable textures usage
}

// Draw a part of a texture (defined by a rectangle)
void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint)
{
    glEnable(GL_TEXTURE_2D);    // Enable textures usage
    
    glBindTexture(GL_TEXTURE_2D, texture.glId);
    
    glPushMatrix();
        glTranslatef(position.x, position.y, 0);
        //glScalef(1.0f, 1.0f, 1.0f);
        //glRotatef(rotation, 0, 0, 1);
        
        glBegin(GL_QUADS);
            glColor4ub(tint.r, tint.g, tint.b, tint.a);
            glNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer
            
            // Bottom-left corner for texture and quad
            glTexCoord2f((float)sourceRec.x / texture.width, (float)sourceRec.y / texture.height); 
            glVertex2f(0.0f, 0.0f);
            
            // Bottom-right corner for texture and quad
            glTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)sourceRec.y / texture.height);
            glVertex2f(sourceRec.width, 0.0f);
            
            // Top-right corner for texture and quad
            glTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height); 
            glVertex2f(sourceRec.width, sourceRec.height);
            
            // Top-left corner for texture and quad
            glTexCoord2f((float)sourceRec.x / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height);
            glVertex2f(0.0f, sourceRec.height);
        glEnd();
    glPopMatrix();
    
    glDisable(GL_TEXTURE_2D);    // Disable textures usage
}

// Draw a part of a texture (defined by a rectangle) with 'pro' parameters
// TODO: Test this function...
void DrawTexturePro(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector2 origin, float rotation, Color tint)
{
    glEnable(GL_TEXTURE_2D);    // Enable textures usage
    
    glBindTexture(GL_TEXTURE_2D, texture.glId);
    
    glPushMatrix();
        glTranslatef(-origin.x, -origin.y, 0);
        glRotatef(rotation, 0, 0, 1);
        glTranslatef(destRec.x + origin.x, destRec.y + origin.y, 0);
        
        glBegin(GL_QUADS);
            glColor4ub(tint.r, tint.g, tint.b, tint.a);
            glNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer
            
            // Bottom-left corner for texture and quad
            glTexCoord2f((float)sourceRec.x / texture.width, (float)sourceRec.y / texture.height); 
            glVertex2f(0.0f, 0.0f);
            
            // Bottom-right corner for texture and quad
            glTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)sourceRec.y / texture.height);
            glVertex2f(destRec.width, 0.0f);
            
            // Top-right corner for texture and quad
            glTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height); 
            glVertex2f(destRec.width, destRec.height);
            
            // Top-left corner for texture and quad
            glTexCoord2f((float)sourceRec.x / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height);
            glVertex2f(0.0f, destRec.height);
        glEnd();
    glPopMatrix();
    
    glDisable(GL_TEXTURE_2D);    // Disable textures usage
}