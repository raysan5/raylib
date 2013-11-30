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

// Unload image from CPU memory (RAM)
void UnloadImage(Image image)
{
    free(image.pixels);
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

// Creates a bitmap (BMP) file from an array of pixel data
// NOTE: This function is only used by module [core], not explicitly available to raylib users
extern void WriteBitmap(const char *fileName, const Color *imgDataPixel, int width, int height)
{
    int filesize = 54 + 3*width*height;
    
    unsigned char bmpFileHeader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};    // Standard BMP file header
    unsigned char bmpInfoHeader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};   // Standard BMP info header

    bmpFileHeader[2] = (unsigned char)(filesize);
    bmpFileHeader[3] = (unsigned char)(filesize>>8);
    bmpFileHeader[4] = (unsigned char)(filesize>>16);
    bmpFileHeader[5] = (unsigned char)(filesize>>24);

    bmpInfoHeader[4] = (unsigned char)(width);
    bmpInfoHeader[5] = (unsigned char)(width>>8);
    bmpInfoHeader[6] = (unsigned char)(width>>16);
    bmpInfoHeader[7] = (unsigned char)(width>>24);
    bmpInfoHeader[8] = (unsigned char)(height);
    bmpInfoHeader[9] = (unsigned char)(height>>8);
    bmpInfoHeader[10] = (unsigned char)(height>>16);
    bmpInfoHeader[11] = (unsigned char)(height>>24);

    FILE *bmpFile = fopen(fileName, "wb");    // Define a pointer to bitmap file and open it in write-binary mode
    
    // NOTE: fwrite parameters are: data pointer, size in bytes of each element to be written, number of elements, file-to-write pointer
    fwrite(bmpFileHeader, sizeof(unsigned char), 14, bmpFile);    // Write BMP file header data
    fwrite(bmpInfoHeader, sizeof(unsigned char), 40, bmpFile);    // Write BMP info header data
    
    // Write pixel data to file
    for (int y = 0; y < height ; y++)
    {
        for (int x = 0; x < width; x++)
        {
            fputc(imgDataPixel[x + y*width].b, bmpFile);
            fputc(imgDataPixel[x + y*width].g, bmpFile);
            fputc(imgDataPixel[x + y*width].r, bmpFile);
        }
    }

    fclose(bmpFile);        // Close bitmap file
}