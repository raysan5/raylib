/**********************************************************************************************
*
*   raylib.models
*
*   Basic functions to draw 3d shapes and load/draw 3d models (.OBJ)
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

#if defined(PLATFORM_ANDROID)
    #include "utils.h"      // Android fopen function map
#endif

#include <stdio.h>          // Required for: FILE, fopen(), fclose(), fscanf(), feof(), rewind(), fgets()
#include <stdlib.h>         // Required for: malloc(), free()
#include <string.h>         // Required for: strcmp()
#include <math.h>           // Required for: sin(), cos()

#include "rlgl.h"           // raylib OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2
#include "raymath.h"        // Matrix data type and Matrix functions

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define CUBIC_MAP_HALF_BLOCK_SIZE           0.5

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static Mesh LoadOBJ(const char *fileName);      // Load OBJ mesh data
static Material LoadMTL(const char *fileName);  // Load MTL material data

static Mesh GenMeshHeightmap(Image image, Vector3 size);
static Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Draw a line in 3D world space
void Draw3DLine(Vector3 startPos, Vector3 endPos, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex3f(startPos.x, startPos.y, startPos.z);
        rlVertex3f(endPos.x, endPos.y, endPos.z);
    rlEnd();
}

// Draw a circle in 3D world space
void Draw3DCircle(Vector3 center, float radius, float rotationAngle, Vector3 rotation, Color color)
{
    rlPushMatrix();
        rlTranslatef(center.x, center.y, center.z);
        rlRotatef(rotationAngle, rotation.x, rotation.y, rotation.z);
        
        rlBegin(RL_LINES);
            for (int i = 0; i < 360; i += 10)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);
                
                rlVertex3f(sin(DEG2RAD*i)*radius, cos(DEG2RAD*i)*radius, 0.0f);
                rlVertex3f(sin(DEG2RAD*(i + 10)) * radius, cos(DEG2RAD*(i + 10)) * radius, 0.0f);
            }
        rlEnd();
    rlPopMatrix();
}

// Draw cube
// NOTE: Cube position is the center position
void DrawCube(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlPushMatrix();

        // NOTE: Be careful! Function order matters (rotate -> scale -> translate)
        rlTranslatef(position.x, position.y, position.z);
        //rlScalef(2.0f, 2.0f, 2.0f);
        //rlRotatef(45, 0, 1, 0);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front Face -----------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left

            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right

            // Back Face ------------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right

            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left

            // Top Face -------------------------------------------------------
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Bottom Right

            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Bottom Right

            // Bottom Face ----------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left

            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left

            // Right face -----------------------------------------------------
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Left

            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Left

            // Left Face ------------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Right

            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Right
        rlEnd();
    rlPopMatrix();
}

// Draw cube (Vector version)
void DrawCubeV(Vector3 position, Vector3 size, Color color)
{
    DrawCube(position, size.x, size.y, size.z, color);
}

// Draw cube wires
void DrawCubeWires(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlPushMatrix();

        rlTranslatef(position.x, position.y, position.z);
        //rlRotatef(45, 0, 1, 0);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front Face -----------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left

            // Back Face ------------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left

            // Top Face -------------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right Back

            // Bottom Face  ---------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Top Right Back
        rlEnd();
    rlPopMatrix();
}

// Draw cube
// NOTE: Cube position is the center position
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    rlEnableTexture(texture.id);

    //rlPushMatrix();
        // NOTE: Be careful! Function order matters (scale, translate, rotate)
        //rlScalef(2.0f, 2.0f, 2.0f);
        //rlTranslatef(2.0f, 0.0f, 0.0f);
        //rlRotatef(45, 0, 1, 0);

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            // Front Face
            rlNormal3f(0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left Of The Texture and Quad
            // Back Face
            rlNormal3f( 0.0f, 0.0f,-1.0f);                  // Normal Pointing Away From Viewer
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Left Of The Texture and Quad
            // Top Face
            rlNormal3f( 0.0f, 1.0f, 0.0f);                  // Normal Pointing Up
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x-width/2, y+height/2, z+length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x+width/2, y+height/2, z+length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right Of The Texture and Quad
            // Bottom Face
            rlNormal3f( 0.0f,-1.0f, 0.0f);                  // Normal Pointing Down
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x+width/2, y-height/2, z-length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Right Of The Texture and Quad
            // Right face
            rlNormal3f( 1.0f, 0.0f, 0.0f);                  // Normal Pointing Right
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Left Of The Texture and Quad
            // Left Face
            rlNormal3f(-1.0f, 0.0f, 0.0f);                  // Normal Pointing Left
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left Of The Texture and Quad
        rlEnd();
    //rlPopMatrix();

    rlDisableTexture();
}

// Draw sphere
void DrawSphere(Vector3 centerPos, float radius, Color color)
{
    DrawSphereEx(centerPos, radius, 16, 16, color);
}

// Draw sphere with extended parameters
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color)
{
    rlPushMatrix();
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(radius, radius, radius);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < (rings + 2); i++)
            {
                for (int j = 0; j < slices; j++)
                {
                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*i)) * sin(DEG2RAD*(j*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cos(DEG2RAD*(270+(180/(rings + 1))*i)) * cos(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * sin(DEG2RAD*((j+1)*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * cos(DEG2RAD*((j+1)*360/slices)));
                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * sin(DEG2RAD*(j*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * cos(DEG2RAD*(j*360/slices)));

                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*i)) * sin(DEG2RAD*(j*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cos(DEG2RAD*(270+(180/(rings + 1))*i)) * cos(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*(i))) * sin(DEG2RAD*((j+1)*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*(i))),
                               cos(DEG2RAD*(270+(180/(rings + 1))*(i))) * cos(DEG2RAD*((j+1)*360/slices)));
                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * sin(DEG2RAD*((j+1)*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * cos(DEG2RAD*((j+1)*360/slices)));
                }
            }
        rlEnd();
    rlPopMatrix();
}

// Draw sphere wires
void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color)
{
    rlPushMatrix();
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(radius, radius, radius);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < (rings + 2); i++)
            {
                for (int j = 0; j < slices; j++)
                {
                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*i)) * sin(DEG2RAD*(j*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cos(DEG2RAD*(270+(180/(rings + 1))*i)) * cos(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * sin(DEG2RAD*((j+1)*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * cos(DEG2RAD*((j+1)*360/slices)));

                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * sin(DEG2RAD*((j+1)*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * cos(DEG2RAD*((j+1)*360/slices)));
                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * sin(DEG2RAD*(j*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * cos(DEG2RAD*(j*360/slices)));

                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * sin(DEG2RAD*(j*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cos(DEG2RAD*(270+(180/(rings + 1))*(i+1))) * cos(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cos(DEG2RAD*(270+(180/(rings + 1))*i)) * sin(DEG2RAD*(j*360/slices)),
                               sin(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cos(DEG2RAD*(270+(180/(rings + 1))*i)) * cos(DEG2RAD*(j*360/slices)));
                }
            }
        rlEnd();
    rlPopMatrix();
}

// Draw a cylinder
// NOTE: It could be also used for pyramid and cone
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int sides, Color color)
{
    if (sides < 3) sides = 3;

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            if (radiusTop > 0)
            {
                // Draw Body -------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(sin(DEG2RAD*i) * radiusBottom, 0, cos(DEG2RAD*i) * radiusBottom); //Bottom Left
                    rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusBottom, 0, cos(DEG2RAD*(i+360/sides)) * radiusBottom); //Bottom Right
                    rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusTop, height, cos(DEG2RAD*(i+360/sides)) * radiusTop); //Top Right

                    rlVertex3f(sin(DEG2RAD*i) * radiusTop, height, cos(DEG2RAD*i) * radiusTop); //Top Left
                    rlVertex3f(sin(DEG2RAD*i) * radiusBottom, 0, cos(DEG2RAD*i) * radiusBottom); //Bottom Left
                    rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusTop, height, cos(DEG2RAD*(i+360/sides)) * radiusTop); //Top Right
                }

                // Draw Cap --------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(0, height, 0);
                    rlVertex3f(sin(DEG2RAD*i) * radiusTop, height, cos(DEG2RAD*i) * radiusTop);
                    rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusTop, height, cos(DEG2RAD*(i+360/sides)) * radiusTop);
                }
            }
            else
            {
                // Draw Cone -------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(0, height, 0);
                    rlVertex3f(sin(DEG2RAD*i) * radiusBottom, 0, cos(DEG2RAD*i) * radiusBottom);
                    rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusBottom, 0, cos(DEG2RAD*(i+360/sides)) * radiusBottom);
                }
            }

            // Draw Base -----------------------------------------------------------------------------------------
            for (int i = 0; i < 360; i += 360/sides)
            {
                rlVertex3f(0, 0, 0);
                rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusBottom, 0, cos(DEG2RAD*(i+360/sides)) * radiusBottom);
                rlVertex3f(sin(DEG2RAD*i) * radiusBottom, 0, cos(DEG2RAD*i) * radiusBottom);
            }
        rlEnd();
    rlPopMatrix();
}

// Draw a wired cylinder
// NOTE: It could be also used for pyramid and cone
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int sides, Color color)
{
    if (sides < 3) sides = 3;

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < 360; i += 360/sides)
            {
                rlVertex3f(sin(DEG2RAD*i) * radiusBottom, 0, cos(DEG2RAD*i) * radiusBottom);
                rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusBottom, 0, cos(DEG2RAD*(i+360/sides)) * radiusBottom);

                rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusBottom, 0, cos(DEG2RAD*(i+360/sides)) * radiusBottom);
                rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusTop, height, cos(DEG2RAD*(i+360/sides)) * radiusTop);

                rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusTop, height, cos(DEG2RAD*(i+360/sides)) * radiusTop);
                rlVertex3f(sin(DEG2RAD*i) * radiusTop, height, cos(DEG2RAD*i) * radiusTop);

                rlVertex3f(sin(DEG2RAD*i) * radiusTop, height, cos(DEG2RAD*i) * radiusTop);
                rlVertex3f(sin(DEG2RAD*i) * radiusBottom, 0, cos(DEG2RAD*i) * radiusBottom);
            }
        rlEnd();
    rlPopMatrix();
}

// Draw a plane
void DrawPlane(Vector3 centerPos, Vector2 size, Color color)
{
    // NOTE: Plane is always created on XZ ground
    rlPushMatrix();
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(size.x, 1.0f, size.y);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlNormal3f(0.0f, 1.0f, 0.0f);

            rlVertex3f(0.5f, 0.0f, -0.5f);
            rlVertex3f(-0.5f, 0.0f, -0.5f);
            rlVertex3f(-0.5f, 0.0f, 0.5f);

            rlVertex3f(-0.5f, 0.0f, 0.5f);
            rlVertex3f(0.5f, 0.0f, 0.5f);
            rlVertex3f(0.5f, 0.0f, -0.5f);
        rlEnd();
    rlPopMatrix();
}

// Draw a ray line
void DrawRay(Ray ray, Color color)
{
    float scale = 10000;

    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlVertex3f(ray.position.x, ray.position.y, ray.position.z);
        rlVertex3f(ray.position.x + ray.direction.x*scale, ray.position.y + ray.direction.y*scale, ray.position.z + ray.direction.z*scale);
    rlEnd();
}

// Draw a grid centered at (0, 0, 0)
void DrawGrid(int slices, float spacing)
{
    int halfSlices = slices / 2;

    rlBegin(RL_LINES);
        for (int i = -halfSlices; i <= halfSlices; i++)
        {
            if (i == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f((float)i*spacing, 0.0f, (float)-halfSlices*spacing);
            rlVertex3f((float)i*spacing, 0.0f, (float)halfSlices*spacing);

            rlVertex3f((float)-halfSlices*spacing, 0.0f, (float)i*spacing);
            rlVertex3f((float)halfSlices*spacing, 0.0f, (float)i*spacing);
        }
    rlEnd();
}

// Draw gizmo
void DrawGizmo(Vector3 position)
{
    // NOTE: RGB = XYZ
    float length = 1.0f;

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);
        //rlRotatef(rotation, 0, 1, 0);
        rlScalef(length, length, length);

        rlBegin(RL_LINES);
            rlColor3f(1.0f, 0.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
            rlColor3f(1.0f, 0.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 0.0f);

            rlColor3f(0.0f, 1.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
            rlColor3f(0.0f, 1.0f, 0.0f); rlVertex3f(0.0f, 1.0f, 0.0f);

            rlColor3f(0.0f, 0.0f, 1.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
            rlColor3f(0.0f, 0.0f, 1.0f); rlVertex3f(0.0f, 0.0f, 1.0f);
        rlEnd();
    rlPopMatrix();
}


// Draw light in 3D world
void DrawLight(Light light)
{
    switch (light->type)
    {
        case LIGHT_POINT:
        {
            DrawSphereWires(light->position, 0.3f*light->intensity, 4, 8, (light->enabled ? light->diffuse : BLACK));
            Draw3DCircle(light->position, light->radius, 0.0f, (Vector3){ 0, 0, 0 }, (light->enabled ? light->diffuse : BLACK));
            Draw3DCircle(light->position, light->radius, 90.0f, (Vector3){ 1, 0, 0 }, (light->enabled ? light->diffuse : BLACK));
            Draw3DCircle(light->position, light->radius, 90.0f, (Vector3){ 0, 1, 0 }, (light->enabled ? light->diffuse : BLACK));
        } break;
        case LIGHT_DIRECTIONAL:
        {                
            Draw3DLine(light->position, light->target, (light->enabled ? light->diffuse : BLACK));
            DrawSphereWires(light->position, 0.3f*light->intensity, 4, 8, (light->enabled ? light->diffuse : BLACK));
            DrawCubeWires(light->target, 0.3f, 0.3f, 0.3f, (light->enabled ? light->diffuse : BLACK));
        } break;
        case LIGHT_SPOT:
        {                
            Draw3DLine(light->position, light->target, (light->enabled ? light->diffuse : BLACK));
            DrawCylinderWires(light->position, 0.0f, 0.3f*light->coneAngle/50, 0.6f, 5, (light->enabled ? light->diffuse : BLACK));
            DrawCubeWires(light->target, 0.3f, 0.3f, 0.3f, (light->enabled ? light->diffuse : BLACK));
        } break;
        default: break;
    }
}

// Load a 3d model (from file)
Model LoadModel(const char *fileName)
{
    Model model = { 0 };
    
    // TODO: Initialize default data for model in case loading fails, maybe a cube?

    if (strcmp(GetExtension(fileName), "obj") == 0) model.mesh = LoadOBJ(fileName);
    else TraceLog(WARNING, "[%s] Model extension not recognized, it can't be loaded", fileName);

    if (model.mesh.vertexCount == 0) TraceLog(WARNING, "Model could not be loaded");
    else
    {
        rlglLoadMesh(&model.mesh, false);  // Upload vertex data to GPU (static model)
        
        model.transform = MatrixIdentity();
        model.material = LoadDefaultMaterial();
    }

    return model;
}

// Load a 3d model (from vertex data)
Model LoadModelEx(Mesh data, bool dynamic)
{
    Model model = { 0 };

    model.mesh = data;
    
    rlglLoadMesh(&model.mesh, dynamic);  // Upload vertex data to GPU
    
    model.transform = MatrixIdentity();
    model.material = LoadDefaultMaterial();
    
    return model;
}

// Load a 3d model from rRES file (raylib Resource)
Model LoadModelFromRES(const char *rresName, int resId)
{
    Model model = { 0 };
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

                    // Check data is of valid MODEL type
                    if (infoHeader.type == 8)
                    {
                        // TODO: Load model data
                    }
                    else
                    {
                        TraceLog(WARNING, "[%s] Required resource do not seem to be a valid MODEL resource", rresName);
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

    return model;
}

// Load a heightmap image as a 3d model
// NOTE: model map size is defined in generic units
Model LoadHeightmap(Image heightmap, Vector3 size)
{
    Model model = { 0 };
    
    model.mesh = GenMeshHeightmap(heightmap, size);
    
    rlglLoadMesh(&model.mesh, false);  // Upload vertex data to GPU (static model)
    
    model.transform = MatrixIdentity();
    model.material = LoadDefaultMaterial();

    return model;
}

// Load a map image as a 3d model (cubes based)
Model LoadCubicmap(Image cubicmap)
{
    Model model = { 0 };
    
    model.mesh = GenMeshCubicmap(cubicmap, (Vector3){ 1.0f, 1.5f, 1.0f });
    
    rlglLoadMesh(&model.mesh, false);  // Upload vertex data to GPU (static model)
    
    model.transform = MatrixIdentity();
    model.material = LoadDefaultMaterial();

    return model;
}

// Unload 3d model from memory (mesh and material)
void UnloadModel(Model model)
{
    rlglUnloadMesh(&model.mesh);

    UnloadMaterial(model.material);
    
    TraceLog(INFO, "Unloaded model data from RAM and VRAM");
}

// Load material data (from file)
Material LoadMaterial(const char *fileName)
{
    Material material = { 0 };
    
    if (strcmp(GetExtension(fileName), "mtl") == 0) material = LoadMTL(fileName);
    else TraceLog(WARNING, "[%s] Material extension not recognized, it can't be loaded", fileName);
    
    return material;
}

// Load default material (uses default models shader)
Material LoadDefaultMaterial(void)
{
    Material material = { 0 };
    
    material.shader = GetDefaultShader();
    material.texDiffuse = GetDefaultTexture();      // White texture (1x1 pixel)
    //material.texNormal;           // NOTE: By default, not set
    //material.texSpecular;         // NOTE: By default, not set

    material.colDiffuse = WHITE;    // Diffuse color
    material.colAmbient = WHITE;    // Ambient color
    material.colSpecular = WHITE;   // Specular color
    
    material.glossiness = 100.0f;   // Glossiness level
    
    return material;
}

// Load standard material (uses material attributes and lighting shader)
// NOTE: Standard shader supports multiple maps and lights
Material LoadStandardMaterial(void)
{
    Material material = LoadDefaultMaterial();
    
    material.shader = GetStandardShader();

    return material;
}

// Unload material from memory
void UnloadMaterial(Material material)
{
    rlDeleteTextures(material.texDiffuse.id);
    rlDeleteTextures(material.texNormal.id);
    rlDeleteTextures(material.texSpecular.id);
}

// Link a texture to a model
void SetModelTexture(Model *model, Texture2D texture)
{
    if (texture.id <= 0) model->material.texDiffuse = GetDefaultTexture();  // Use default white texture
    else model->material.texDiffuse = texture;
}

// Generate a mesh from heightmap
static Mesh GenMeshHeightmap(Image heightmap, Vector3 size)
{
    #define GRAY_VALUE(c) ((c.r+c.g+c.b)/3)
    
    Mesh mesh = { 0 };

    int mapX = heightmap.width;
    int mapZ = heightmap.height;
    
    Color *pixels = GetImageData(heightmap);

    // NOTE: One vertex per pixel
    int numTriangles = (mapX-1)*(mapZ-1)*2;    // One quad every four pixels

    mesh.vertexCount = numTriangles*3;

    mesh.vertices = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.normals = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)malloc(mesh.vertexCount*2*sizeof(float));
    mesh.colors = NULL;

    int vCounter = 0;       // Used to count vertices float by float
    int tcCounter = 0;      // Used to count texcoords float by float
    int nCounter = 0;       // Used to count normals float by float

    int trisCounter = 0;

    Vector3 scaleFactor = { size.x/mapX, size.y/255.0f, size.z/mapZ };

    for (int z = 0; z < mapZ-1; z++)
    {
        for (int x = 0; x < mapX-1; x++)
        {
            // Fill vertices array with data
            //----------------------------------------------------------

            // one triangle - 3 vertex
            mesh.vertices[vCounter] = (float)x*scaleFactor.x;
            mesh.vertices[vCounter + 1] = (float)GRAY_VALUE(pixels[x + z*mapX])*scaleFactor.y;
            mesh.vertices[vCounter + 2] = (float)z*scaleFactor.z;

            mesh.vertices[vCounter + 3] = (float)x*scaleFactor.x;
            mesh.vertices[vCounter + 4] = (float)GRAY_VALUE(pixels[x + (z + 1)*mapX])*scaleFactor.y;
            mesh.vertices[vCounter + 5] = (float)(z + 1)*scaleFactor.z;

            mesh.vertices[vCounter + 6] = (float)(x + 1)*scaleFactor.x;
            mesh.vertices[vCounter + 7] = (float)GRAY_VALUE(pixels[(x + 1) + z*mapX])*scaleFactor.y;
            mesh.vertices[vCounter + 8] = (float)z*scaleFactor.z;

            // another triangle - 3 vertex
            mesh.vertices[vCounter + 9] = mesh.vertices[vCounter + 6];
            mesh.vertices[vCounter + 10] = mesh.vertices[vCounter + 7];
            mesh.vertices[vCounter + 11] = mesh.vertices[vCounter + 8];

            mesh.vertices[vCounter + 12] = mesh.vertices[vCounter + 3];
            mesh.vertices[vCounter + 13] = mesh.vertices[vCounter + 4];
            mesh.vertices[vCounter + 14] = mesh.vertices[vCounter + 5];

            mesh.vertices[vCounter + 15] = (float)(x + 1)*scaleFactor.x;
            mesh.vertices[vCounter + 16] = (float)GRAY_VALUE(pixels[(x + 1) + (z + 1)*mapX])*scaleFactor.y;
            mesh.vertices[vCounter + 17] = (float)(z + 1)*scaleFactor.z;
            vCounter += 18;     // 6 vertex, 18 floats

            // Fill texcoords array with data
            //--------------------------------------------------------------
            mesh.texcoords[tcCounter] = (float)x/(mapX - 1);
            mesh.texcoords[tcCounter + 1] = (float)z/(mapZ - 1);

            mesh.texcoords[tcCounter + 2] = (float)x/(mapX - 1);
            mesh.texcoords[tcCounter + 3] = (float)(z + 1)/(mapZ - 1);

            mesh.texcoords[tcCounter + 4] = (float)(x + 1)/(mapX - 1);
            mesh.texcoords[tcCounter + 5] = (float)z/(mapZ - 1);

            mesh.texcoords[tcCounter + 6] = mesh.texcoords[tcCounter + 4];
            mesh.texcoords[tcCounter + 7] = mesh.texcoords[tcCounter + 5];

            mesh.texcoords[tcCounter + 8] = mesh.texcoords[tcCounter + 2];
            mesh.texcoords[tcCounter + 9] = mesh.texcoords[tcCounter + 3];

            mesh.texcoords[tcCounter + 10] = (float)(x + 1)/(mapX - 1);
            mesh.texcoords[tcCounter + 11] = (float)(z + 1)/(mapZ - 1);
            tcCounter += 12;    // 6 texcoords, 12 floats

            // Fill normals array with data
            //--------------------------------------------------------------
            for (int i = 0; i < 18; i += 3)
            {
                mesh.normals[nCounter + i] = 0.0f;
                mesh.normals[nCounter + i + 1] = 1.0f;
                mesh.normals[nCounter + i + 2] = 0.0f;
            }

            // TODO: Calculate normals in an efficient way

            nCounter += 18;     // 6 vertex, 18 floats
            trisCounter += 2;
        }
    }
    
    free(pixels);

    return mesh;
}

static Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize)
{
    Mesh mesh = { 0 };

    Color *cubicmapPixels = GetImageData(cubicmap);
    
    int mapWidth = cubicmap.width*(int)cubeSize.x;
    int mapHeight = cubicmap.height*(int)cubeSize.z;

    // NOTE: Max possible number of triangles numCubes * (12 triangles by cube)
    int maxTriangles = cubicmap.width*cubicmap.height*12;

    int vCounter = 0;       // Used to count vertices
    int tcCounter = 0;      // Used to count texcoords
    int nCounter = 0;       // Used to count normals

    float w = cubeSize.x;
    float h = cubeSize.z;
    float h2 = cubeSize.y;

    Vector3 *mapVertices = (Vector3 *)malloc(maxTriangles*3*sizeof(Vector3));
    Vector2 *mapTexcoords = (Vector2 *)malloc(maxTriangles*3*sizeof(Vector2));
    Vector3 *mapNormals = (Vector3 *)malloc(maxTriangles*3*sizeof(Vector3));

    // Define the 6 normals of the cube, we will combine them accordingly later...
    Vector3 n1 = { 1.0f, 0.0f, 0.0f };
    Vector3 n2 = { -1.0f, 0.0f, 0.0f };
    Vector3 n3 = { 0.0f, 1.0f, 0.0f };
    Vector3 n4 = { 0.0f, -1.0f, 0.0f };
    Vector3 n5 = { 0.0f, 0.0f, 1.0f };
    Vector3 n6 = { 0.0f, 0.0f, -1.0f };

    // NOTE: We use texture rectangles to define different textures for top-bottom-front-back-right-left (6)
    typedef struct RectangleF {
        float x;
        float y;
        float width;
        float height;
    } RectangleF;

    RectangleF rightTexUV = { 0.0f, 0.0f, 0.5f, 0.5f };
    RectangleF leftTexUV = { 0.5f, 0.0f, 0.5f, 0.5f };
    RectangleF frontTexUV = { 0.0f, 0.0f, 0.5f, 0.5f };
    RectangleF backTexUV = { 0.5f, 0.0f, 0.5f, 0.5f };
    RectangleF topTexUV = { 0.0f, 0.5f, 0.5f, 0.5f };
    RectangleF bottomTexUV = { 0.5f, 0.5f, 0.5f, 0.5f };

    for (int z = 0; z < mapHeight; z += cubeSize.z)
    {
        for (int x = 0; x < mapWidth; x += cubeSize.x)
        {
            // Define the 8 vertex of the cube, we will combine them accordingly later...
            Vector3 v1 = { x - w/2, h2, z - h/2 };
            Vector3 v2 = { x - w/2, h2, z + h/2 };
            Vector3 v3 = { x + w/2, h2, z + h/2 };
            Vector3 v4 = { x + w/2, h2, z - h/2 };
            Vector3 v5 = { x + w/2, 0, z - h/2 };
            Vector3 v6 = { x - w/2, 0, z - h/2 };
            Vector3 v7 = { x - w/2, 0, z + h/2 };
            Vector3 v8 = { x + w/2, 0, z + h/2 };

            // We check pixel color to be WHITE, we will full cubes
            if ((cubicmapPixels[z*cubicmap.width + x].r == 255) &&
                (cubicmapPixels[z*cubicmap.width + x].g == 255) &&
                (cubicmapPixels[z*cubicmap.width + x].b == 255))
            {
                // Define triangles (Checking Collateral Cubes!)
                //----------------------------------------------

                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                mapVertices[vCounter] = v1;
                mapVertices[vCounter + 1] = v2;
                mapVertices[vCounter + 2] = v3;
                mapVertices[vCounter + 3] = v1;
                mapVertices[vCounter + 4] = v3;
                mapVertices[vCounter + 5] = v4;
                vCounter += 6;

                mapNormals[nCounter] = n3;
                mapNormals[nCounter + 1] = n3;
                mapNormals[nCounter + 2] = n3;
                mapNormals[nCounter + 3] = n3;
                mapNormals[nCounter + 4] = n3;
                mapNormals[nCounter + 5] = n3;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vCounter] = v6;
                mapVertices[vCounter + 1] = v8;
                mapVertices[vCounter + 2] = v7;
                mapVertices[vCounter + 3] = v6;
                mapVertices[vCounter + 4] = v5;
                mapVertices[vCounter + 5] = v8;
                vCounter += 6;

                mapNormals[nCounter] = n4;
                mapNormals[nCounter + 1] = n4;
                mapNormals[nCounter + 2] = n4;
                mapNormals[nCounter + 3] = n4;
                mapNormals[nCounter + 4] = n4;
                mapNormals[nCounter + 5] = n4;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
                mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                tcCounter += 6;

                if (((z < cubicmap.height - 1) &&
                (cubicmapPixels[(z + 1)*cubicmap.width + x].r == 0) &&
                (cubicmapPixels[(z + 1)*cubicmap.width + x].g == 0) &&
                (cubicmapPixels[(z + 1)*cubicmap.width + x].b == 0)) || (z == cubicmap.height - 1))
                {
                    // Define front triangles (2 tris, 6 vertex) --> v2 v7 v3, v3 v7 v8
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v2;
                    mapVertices[vCounter + 1] = v7;
                    mapVertices[vCounter + 2] = v3;
                    mapVertices[vCounter + 3] = v3;
                    mapVertices[vCounter + 4] = v7;
                    mapVertices[vCounter + 5] = v8;
                    vCounter += 6;

                    mapNormals[nCounter] = n6;
                    mapNormals[nCounter + 1] = n6;
                    mapNormals[nCounter + 2] = n6;
                    mapNormals[nCounter + 3] = n6;
                    mapNormals[nCounter + 4] = n6;
                    mapNormals[nCounter + 5] = n6;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ frontTexUV.x, frontTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y + frontTexUV.height };
                    tcCounter += 6;
                }

                if (((z > 0) &&
                (cubicmapPixels[(z - 1)*cubicmap.width + x].r == 0) &&
                (cubicmapPixels[(z - 1)*cubicmap.width + x].g == 0) &&
                (cubicmapPixels[(z - 1)*cubicmap.width + x].b == 0)) || (z == 0))
                {
                    // Define back triangles (2 tris, 6 vertex) --> v1 v5 v6, v1 v4 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1;
                    mapVertices[vCounter + 1] = v5;
                    mapVertices[vCounter + 2] = v6;
                    mapVertices[vCounter + 3] = v1;
                    mapVertices[vCounter + 4] = v4;
                    mapVertices[vCounter + 5] = v5;
                    vCounter += 6;

                    mapNormals[nCounter] = n5;
                    mapNormals[nCounter + 1] = n5;
                    mapNormals[nCounter + 2] = n5;
                    mapNormals[nCounter + 3] = n5;
                    mapNormals[nCounter + 4] = n5;
                    mapNormals[nCounter + 5] = n5;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 3] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ backTexUV.x, backTexUV.y };
                    mapTexcoords[tcCounter + 5] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    tcCounter += 6;
                }

                if (((x < cubicmap.width - 1) &&
                (cubicmapPixels[z*cubicmap.width + (x + 1)].r == 0) &&
                (cubicmapPixels[z*cubicmap.width + (x + 1)].g == 0) &&
                (cubicmapPixels[z*cubicmap.width + (x + 1)].b == 0)) || (x == cubicmap.width - 1))
                {
                    // Define right triangles (2 tris, 6 vertex) --> v3 v8 v4, v4 v8 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v3;
                    mapVertices[vCounter + 1] = v8;
                    mapVertices[vCounter + 2] = v4;
                    mapVertices[vCounter + 3] = v4;
                    mapVertices[vCounter + 4] = v8;
                    mapVertices[vCounter + 5] = v5;
                    vCounter += 6;

                    mapNormals[nCounter] = n1;
                    mapNormals[nCounter + 1] = n1;
                    mapNormals[nCounter + 2] = n1;
                    mapNormals[nCounter + 3] = n1;
                    mapNormals[nCounter + 4] = n1;
                    mapNormals[nCounter + 5] = n1;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ rightTexUV.x, rightTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y + rightTexUV.height };
                    tcCounter += 6;
                }

                if (((x > 0) &&
                (cubicmapPixels[z*cubicmap.width + (x - 1)].r == 0) &&
                (cubicmapPixels[z*cubicmap.width + (x - 1)].g == 0) &&
                (cubicmapPixels[z*cubicmap.width + (x - 1)].b == 0)) || (x == 0))
                {
                    // Define left triangles (2 tris, 6 vertex) --> v1 v7 v2, v1 v6 v7
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1;
                    mapVertices[vCounter + 1] = v7;
                    mapVertices[vCounter + 2] = v2;
                    mapVertices[vCounter + 3] = v1;
                    mapVertices[vCounter + 4] = v6;
                    mapVertices[vCounter + 5] = v7;
                    vCounter += 6;

                    mapNormals[nCounter] = n2;
                    mapNormals[nCounter + 1] = n2;
                    mapNormals[nCounter + 2] = n2;
                    mapNormals[nCounter + 3] = n2;
                    mapNormals[nCounter + 4] = n2;
                    mapNormals[nCounter + 5] = n2;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ leftTexUV.x, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    tcCounter += 6;
                }
            }
            // We check pixel color to be BLACK, we will only draw floor and roof
            else if  ((cubicmapPixels[z*cubicmap.width + x].r == 0) &&
                      (cubicmapPixels[z*cubicmap.width + x].g == 0) &&
                      (cubicmapPixels[z*cubicmap.width + x].b == 0))
            {
                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                mapVertices[vCounter] = v1;
                mapVertices[vCounter + 1] = v3;
                mapVertices[vCounter + 2] = v2;
                mapVertices[vCounter + 3] = v1;
                mapVertices[vCounter + 4] = v4;
                mapVertices[vCounter + 5] = v3;
                vCounter += 6;

                mapNormals[nCounter] = n4;
                mapNormals[nCounter + 1] = n4;
                mapNormals[nCounter + 2] = n4;
                mapNormals[nCounter + 3] = n4;
                mapNormals[nCounter + 4] = n4;
                mapNormals[nCounter + 5] = n4;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vCounter] = v6;
                mapVertices[vCounter + 1] = v7;
                mapVertices[vCounter + 2] = v8;
                mapVertices[vCounter + 3] = v6;
                mapVertices[vCounter + 4] = v8;
                mapVertices[vCounter + 5] = v5;
                vCounter += 6;

                mapNormals[nCounter] = n3;
                mapNormals[nCounter + 1] = n3;
                mapNormals[nCounter + 2] = n3;
                mapNormals[nCounter + 3] = n3;
                mapNormals[nCounter + 4] = n3;
                mapNormals[nCounter + 5] = n3;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
                tcCounter += 6;
            }
        }
    }

    // Move data from mapVertices temp arays to vertices float array
    mesh.vertexCount = vCounter;

    mesh.vertices = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.normals = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)malloc(mesh.vertexCount*2*sizeof(float));
    mesh.colors = NULL;

    int fCounter = 0;

    // Move vertices data
    for (int i = 0; i < vCounter; i++)
    {
        mesh.vertices[fCounter] = mapVertices[i].x;
        mesh.vertices[fCounter + 1] = mapVertices[i].y;
        mesh.vertices[fCounter + 2] = mapVertices[i].z;
        fCounter += 3;
    }

    fCounter = 0;

    // Move normals data
    for (int i = 0; i < nCounter; i++)
    {
        mesh.normals[fCounter] = mapNormals[i].x;
        mesh.normals[fCounter + 1] = mapNormals[i].y;
        mesh.normals[fCounter + 2] = mapNormals[i].z;
        fCounter += 3;
    }

    fCounter = 0;

    // Move texcoords data
    for (int i = 0; i < tcCounter; i++)
    {
        mesh.texcoords[fCounter] = mapTexcoords[i].x;
        mesh.texcoords[fCounter + 1] = mapTexcoords[i].y;
        fCounter += 2;
    }

    free(mapVertices);
    free(mapNormals);
    free(mapTexcoords);
    
    free(cubicmapPixels);   // Free image pixel data
    
    return mesh;
}

// Draw a model (with texture if set)
void DrawModel(Model model, Vector3 position, float scale, Color tint)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 0.0f, 0.0f, 0.0f };
    
    DrawModelEx(model, position, rotationAxis, 0.0f, vScale, tint);
}

// Draw a model with extended parameters
void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{
    // Calculate transformation matrix from function parameters
    // Get transform matrix (rotation -> scale -> translation)
    Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle*DEG2RAD);
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);
    
    // Combine model transformation matrix (model.transform) with matrix generated by function parameters (matTransform)
    //Matrix matModel = MatrixMultiply(model.transform, matTransform);    // Transform to world-space coordinates
    
    model.transform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);
    model.material.colDiffuse = tint;       // TODO: Multiply tint color by diffuse color?
    
    rlglDrawMesh(model.mesh, model.material, model.transform);
}

// Draw a model wires (with texture if set)
void DrawModelWires(Model model, Vector3 position, float scale, Color tint)
{
    rlEnableWireMode();
    
    DrawModel(model, position, scale, tint);
    
    rlDisableWireMode();
}

// Draw a model wires (with texture if set) with extended parameters
void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{
    rlEnableWireMode();
    
    DrawModelEx(model, position, rotationAxis, rotationAngle, scale, tint);
    
    rlDisableWireMode();
}

// Draw a billboard
void DrawBillboard(Camera camera, Texture2D texture, Vector3 center, float size, Color tint)
{
    Rectangle sourceRec = { 0, 0, texture.width, texture.height };
    
    DrawBillboardRec(camera, texture, sourceRec, center, size, tint);
}

// Draw a billboard (part of a texture defined by a rectangle)
void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle sourceRec, Vector3 center, float size, Color tint)
{
    // NOTE: Billboard size will maintain sourceRec aspect ratio, size will represent billboard width
    Vector2 sizeRatio = { size, size*(float)sourceRec.height/sourceRec.width };

    Matrix viewMatrix = MatrixLookAt(camera.position, camera.target, camera.up);
    MatrixTranspose(&viewMatrix);

    Vector3 right = { viewMatrix.m0, viewMatrix.m4, viewMatrix.m8 };
    //Vector3 up = { viewMatrix.m1, viewMatrix.m5, viewMatrix.m9 };
    
    // NOTE: Billboard locked on axis-Y
    Vector3 up = { 0.0f, 1.0f, 0.0f };
/*
    a-------b
    |       |
    |   *   |
    |       |
    d-------c
*/
    VectorScale(&right, sizeRatio.x/2);
    VectorScale(&up, sizeRatio.y/2);

    Vector3 p1 = VectorAdd(right, up);
    Vector3 p2 = VectorSubtract(right, up);

    Vector3 a = VectorSubtract(center, p2);
    Vector3 b = VectorAdd(center, p1);
    Vector3 c = VectorAdd(center, p2);
    Vector3 d = VectorSubtract(center, p1);

    rlEnableTexture(texture.id);

    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        // Bottom-left corner for texture and quad
        rlTexCoord2f((float)sourceRec.x / texture.width, (float)sourceRec.y / texture.height);
        rlVertex3f(a.x, a.y, a.z);

        // Top-left corner for texture and quad
        rlTexCoord2f((float)sourceRec.x / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height);
        rlVertex3f(d.x, d.y, d.z);

        // Top-right corner for texture and quad
        rlTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)(sourceRec.y + sourceRec.height) / texture.height);
        rlVertex3f(c.x, c.y, c.z);

        // Bottom-right corner for texture and quad
        rlTexCoord2f((float)(sourceRec.x + sourceRec.width) / texture.width, (float)sourceRec.y / texture.height);
        rlVertex3f(b.x, b.y, b.z);
    rlEnd();

    rlDisableTexture();
}

// Draw a bounding box with wires
void DrawBoundingBox(BoundingBox box, Color color)
{
    Vector3 size;
    
    size.x = fabsf(box.max.x - box.min.x);
    size.y = fabsf(box.max.y - box.min.y);
    size.z = fabsf(box.max.z - box.min.z);
    
    Vector3 center = { box.min.x + size.x/2.0f, box.min.y + size.y/2.0f, box.min.z + size.z/2.0f };
    
    DrawCubeWires(center, size.x, size.y, size.z, color);
}

// Detect collision between two spheres
bool CheckCollisionSpheres(Vector3 centerA, float radiusA, Vector3 centerB, float radiusB)
{
    bool collision = false;

    float dx = centerA.x - centerB.x;      // X distance between centers
    float dy = centerA.y - centerB.y;      // Y distance between centers
    float dz = centerA.z - centerB.z;      // Y distance between centers

    float distance = sqrt(dx*dx + dy*dy + dz*dz);  // Distance between centers

    if (distance <= (radiusA + radiusB)) collision = true;

    return collision;
}

// Detect collision between two boxes
// NOTE: Boxes are defined by two points minimum and maximum
bool CheckCollisionBoxes(BoundingBox box1, BoundingBox box2)
{
    bool collision = true;

    if ((box1.max.x >= box2.min.x) && (box1.min.x <= box2.max.x))
    {
        if ((box1.max.y < box2.min.y) || (box1.min.y > box2.max.y)) collision = false;
        if ((box1.max.z < box2.min.z) || (box1.min.z > box2.max.z)) collision = false;
    }
    else collision = false;

    return collision;
}

// Detect collision between box and sphere
bool CheckCollisionBoxSphere(BoundingBox box, Vector3 centerSphere, float radiusSphere)
{
    bool collision = false;

    float dmin = 0;

    if (centerSphere.x < box.min.x) dmin += pow(centerSphere.x - box.min.x, 2);
    else if (centerSphere.x > box.max.x) dmin += pow(centerSphere.x - box.max.x, 2);

    if (centerSphere.y < box.min.y) dmin += pow(centerSphere.y - box.min.y, 2);
    else if (centerSphere.y > box.max.y) dmin += pow(centerSphere.y - box.max.y, 2);

    if (centerSphere.z < box.min.z) dmin += pow(centerSphere.z - box.min.z, 2);
    else if (centerSphere.z > box.max.z) dmin += pow(centerSphere.z - box.max.z, 2);

    if (dmin <= (radiusSphere*radiusSphere)) collision = true;

    return collision;
}

// Detect collision between ray and sphere
bool CheckCollisionRaySphere(Ray ray, Vector3 spherePosition, float sphereRadius)
{
    bool collision = false;
    
    Vector3 raySpherePos = VectorSubtract(spherePosition, ray.position);
    float distance = VectorLength(raySpherePos);
    float vector = VectorDotProduct(raySpherePos, ray.direction);
    float d = sphereRadius*sphereRadius - (distance*distance - vector*vector);
    
    if (d >= 0.0f) collision = true;
    
    return collision;
}

// Detect collision between ray and sphere with extended parameters and collision point detection
bool CheckCollisionRaySphereEx(Ray ray, Vector3 spherePosition, float sphereRadius, Vector3 *collisionPoint)
{
    bool collision = false;
    
    Vector3 raySpherePos = VectorSubtract(spherePosition, ray.position);
    float distance = VectorLength(raySpherePos);
    float vector = VectorDotProduct(raySpherePos, ray.direction);
    float d = sphereRadius*sphereRadius - (distance*distance - vector*vector);
    
    if (d >= 0.0f) collision = true;
    
    // Calculate collision point
    Vector3 offset = ray.direction;
    float collisionDistance = 0;
    
    // Check if ray origin is inside the sphere to calculate the correct collision point
    if (distance < sphereRadius) collisionDistance = vector + sqrt(d);
    else collisionDistance = vector - sqrt(d);
    
    VectorScale(&offset, collisionDistance);
    Vector3 cPoint = VectorAdd(ray.position, offset);
    
    collisionPoint->x = cPoint.x;
    collisionPoint->y = cPoint.y;
    collisionPoint->z = cPoint.z;
    
    return collision;
}

// Detect collision between ray and bounding box
bool CheckCollisionRayBox(Ray ray, BoundingBox box)
{
    bool collision = false;
    
    float t[8];
    t[0] = (box.min.x - ray.position.x)/ray.direction.x;
    t[1] = (box.max.x - ray.position.x)/ray.direction.x;
    t[2] = (box.min.y - ray.position.y)/ray.direction.y;
    t[3] = (box.max.y - ray.position.y)/ray.direction.y;
    t[4] = (box.min.z - ray.position.z)/ray.direction.z;
    t[5] = (box.max.z - ray.position.z)/ray.direction.z;
    t[6] = fmax(fmax(fmin(t[0], t[1]), fmin(t[2], t[3])), fmin(t[4], t[5]));
    t[7] = fmin(fmin(fmax(t[0], t[1]), fmax(t[2], t[3])), fmax(t[4], t[5]));
    
    collision = !(t[7] < 0 || t[6] > t[7]);
    
    return collision;
}

// Calculate mesh bounding box limits
// NOTE: minVertex and maxVertex should be transformed by model transform matrix (position, scale, rotate)
BoundingBox CalculateBoundingBox(Mesh mesh)
{
    // Get min and max vertex to construct bounds (AABB)
    Vector3 minVertex = { 0 };
    Vector3 maxVertex = { 0 };

    if (mesh.vertices != NULL)
    {
        minVertex = (Vector3){ mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
        maxVertex = (Vector3){ mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
    
        for (int i = 1; i < mesh.vertexCount; i++)
        {
            minVertex = VectorMin(minVertex, (Vector3){ mesh.vertices[i*3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2] });
            maxVertex = VectorMax(maxVertex, (Vector3){ mesh.vertices[i*3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2] });
        }
    }
    
    // Create the bounding box
    BoundingBox box;
    box.min = minVertex;
    box.max = maxVertex;
    
    return box;
}

// Detect and resolve cubicmap collisions
// NOTE: player position (or camera) is modified inside this function
Vector3 ResolveCollisionCubicmap(Image cubicmap, Vector3 mapPosition, Vector3 *playerPosition, float radius)
{
    Color *cubicmapPixels = GetImageData(cubicmap);
    
    // Detect the cell where the player is located
    Vector3 impactDirection = { 0.0f, 0.0f, 0.0f };

    int locationCellX = 0;
    int locationCellY = 0;

    locationCellX = floor(playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE);
    locationCellY = floor(playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE);

    if (locationCellX >= 0 && locationCellY >= 0 && locationCellX < cubicmap.width && locationCellY < cubicmap.height)
    {
        // Multiple Axis --------------------------------------------------------------------------------------------

        // Axis x-, y-
        if (locationCellX > 0 && locationCellY > 0)
        {
            if ((cubicmapPixels[locationCellY * cubicmap.width + (locationCellX - 1)].r != 0) &&
                (cubicmapPixels[(locationCellY - 1) * cubicmap.width + (locationCellX)].r != 0))
            {
                if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX < radius) &&
                    ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY < radius))
                {
                    playerPosition->x = locationCellX + mapPosition.x - (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    playerPosition->z = locationCellY + mapPosition.z - (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    impactDirection = (Vector3){ 1.0f, 0.0f, 1.0f };
                }
            }
        }

        // Axis x-, y+
        if (locationCellX > 0 && locationCellY < cubicmap.height - 1)
        {
            if ((cubicmapPixels[locationCellY * cubicmap.width + (locationCellX - 1)].r != 0) &&
                (cubicmapPixels[(locationCellY + 1) * cubicmap.width + (locationCellX)].r != 0))
            {
                if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX < radius) &&
                    ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY > 1 - radius))
                {
                    playerPosition->x = locationCellX + mapPosition.x - (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    playerPosition->z = locationCellY + mapPosition.z + (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    impactDirection = (Vector3){ 1.0f, 0.0f, 1.0f };
                }
            }
        }

        // Axis x+, y-
        if (locationCellX < cubicmap.width - 1 && locationCellY > 0)
        {
            if ((cubicmapPixels[locationCellY * cubicmap.width + (locationCellX + 1)].r != 0) &&
                (cubicmapPixels[(locationCellY - 1) * cubicmap.width + (locationCellX)].r != 0))
            {
                if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX > 1 - radius) &&
                    ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY < radius))
                {
                    playerPosition->x = locationCellX + mapPosition.x + (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    playerPosition->z = locationCellY + mapPosition.z - (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    impactDirection = (Vector3){ 1.0f, 0.0f, 1.0f };
                }
            }
        }

        // Axis x+, y+
        if (locationCellX < cubicmap.width - 1 && locationCellY < cubicmap.height - 1)
        {
            if ((cubicmapPixels[locationCellY * cubicmap.width + (locationCellX + 1)].r != 0) &&
                (cubicmapPixels[(locationCellY + 1) * cubicmap.width + (locationCellX)].r != 0))
            {
                if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX > 1 - radius) &&
                    ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY > 1 - radius))
                {
                    playerPosition->x = locationCellX + mapPosition.x + (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    playerPosition->z = locationCellY + mapPosition.z + (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    impactDirection = (Vector3){ 1.0f, 0.0f, 1.0f };
                }
            }
        }

        // Single Axis ---------------------------------------------------------------------------------------------------

        // Axis x-
        if (locationCellX > 0)
        {
            if (cubicmapPixels[locationCellY * cubicmap.width + (locationCellX - 1)].r != 0)
            {
                if ((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX < radius)
                {
                    playerPosition->x = locationCellX + mapPosition.x - (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    impactDirection = (Vector3){ 1.0f, 0.0f, 0.0f };
                }
            }
        }
        // Axis x+
        if (locationCellX < cubicmap.width - 1)
        {
            if (cubicmapPixels[locationCellY * cubicmap.width + (locationCellX + 1)].r != 0)
            {
                if ((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX > 1 - radius)
                {
                    playerPosition->x = locationCellX + mapPosition.x + (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    impactDirection = (Vector3){ 1.0f, 0.0f, 0.0f };
                }
            }
        }
        // Axis y-
        if (locationCellY > 0)
        {
            if (cubicmapPixels[(locationCellY - 1) * cubicmap.width + (locationCellX)].r != 0)
            {
                if ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY < radius)
                {
                    playerPosition->z = locationCellY + mapPosition.z - (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    impactDirection = (Vector3){ 0.0f, 0.0f, 1.0f };
                }
            }
        }
        // Axis y+
        if (locationCellY < cubicmap.height - 1)
        {
            if (cubicmapPixels[(locationCellY + 1) * cubicmap.width + (locationCellX)].r != 0)
            {
                if ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY > 1 - radius)
                {
                    playerPosition->z = locationCellY + mapPosition.z + (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    impactDirection = (Vector3){ 0.0f, 0.0f, 1.0f };
                }
            }
        }

        // Diagonals -------------------------------------------------------------------------------------------------------

        // Axis x-, y-
        if (locationCellX > 0 && locationCellY > 0)
        {
            if ((cubicmapPixels[locationCellY * cubicmap.width + (locationCellX - 1)].r == 0) &&
                (cubicmapPixels[(locationCellY - 1) * cubicmap.width + (locationCellX)].r == 0) &&
                (cubicmapPixels[(locationCellY - 1) * cubicmap.width + (locationCellX - 1)].r != 0))
            {
                if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX < radius) &&
                    ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY < radius))
                {
                    if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX) > ((playerPosition->z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY)) playerPosition->x = locationCellX + mapPosition.x - (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    else playerPosition->z = locationCellY + mapPosition.z - (CUBIC_MAP_HALF_BLOCK_SIZE - radius);

                    // Return ricochet
                    if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX < radius / 3) &&
                        ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY < radius / 3))
                    {
                        impactDirection = (Vector3){ 1.0f, 0.0f, 1.0f };
                    }
                }
            }
        }

        // Axis x-, y+
        if (locationCellX > 0 && locationCellY < cubicmap.height - 1)
        {
            if ((cubicmapPixels[locationCellY * cubicmap.width + (locationCellX - 1)].r == 0) &&
                (cubicmapPixels[(locationCellY + 1) * cubicmap.width + (locationCellX)].r == 0) &&
                (cubicmapPixels[(locationCellY + 1) * cubicmap.width + (locationCellX - 1)].r != 0))
            {
                if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX < radius) &&
                    ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY > 1 - radius))
                {
                    if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX) > (1 - ((playerPosition->z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY))) playerPosition->x = locationCellX + mapPosition.x - (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    else playerPosition->z = locationCellY + mapPosition.z + (CUBIC_MAP_HALF_BLOCK_SIZE - radius);

                    // Return ricochet
                    if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX < radius / 3) &&
                        ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY > 1 - radius / 3))
                    {
                        impactDirection = (Vector3){ 1.0f, 0.0f, 1.0f };
                    }
                }
            }
        }

        // Axis x+, y-
        if (locationCellX < cubicmap.width - 1 && locationCellY > 0)
        {
            if ((cubicmapPixels[locationCellY * cubicmap.width + (locationCellX + 1)].r == 0) &&
                (cubicmapPixels[(locationCellY - 1) * cubicmap.width + (locationCellX)].r == 0) &&
                (cubicmapPixels[(locationCellY - 1) * cubicmap.width + (locationCellX + 1)].r != 0))
            {
                if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX > 1 - radius) &&
                    ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY < radius))
                {
                    if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX) < (1 - ((playerPosition->z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY))) playerPosition->x = locationCellX + mapPosition.x + (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    else playerPosition->z = locationCellY + mapPosition.z - (CUBIC_MAP_HALF_BLOCK_SIZE - radius);

                    // Return ricochet
                    if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX > 1 - radius / 3) &&
                        ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY < radius / 3))
                    {
                        impactDirection = (Vector3){ 1.0f, 0.0f, 1.0f };
                    }
                }
            }
        }

        // Axis x+, y+
        if (locationCellX < cubicmap.width - 1 && locationCellY < cubicmap.height - 1)
        {
            if ((cubicmapPixels[locationCellY * cubicmap.width + (locationCellX + 1)].r == 0) &&
                (cubicmapPixels[(locationCellY + 1) * cubicmap.width + (locationCellX)].r == 0) &&
                (cubicmapPixels[(locationCellY + 1) * cubicmap.width + (locationCellX + 1)].r != 0))
            {
                if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX > 1 - radius) &&
                    ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY > 1 - radius))
                {
                    if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX) < ((playerPosition->z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY)) playerPosition->x = locationCellX + mapPosition.x + (CUBIC_MAP_HALF_BLOCK_SIZE - radius);
                    else playerPosition->z = locationCellY + mapPosition.z + (CUBIC_MAP_HALF_BLOCK_SIZE - radius);

                    // Return ricochet
                    if (((playerPosition->x - mapPosition.x + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellX > 1 - radius / 3) &&
                        ((playerPosition->z - mapPosition.z + CUBIC_MAP_HALF_BLOCK_SIZE) - locationCellY > 1 - radius / 3))
                    {
                        impactDirection = (Vector3){ 1.0f, 0.0f, 1.0f };
                    }
                }
            }
        }
    }

    // Floor collision
    if (playerPosition->y <= radius)
    {
        playerPosition->y = radius + 0.01f;
        impactDirection = (Vector3) { impactDirection.x, 1, impactDirection.z};
    }
    // Roof collision
    else if (playerPosition->y >= (1.5f - radius))
    {
        playerPosition->y = (1.5f - radius) - 0.01f;
        impactDirection = (Vector3) { impactDirection.x, 1, impactDirection.z};
    }
    
    free(cubicmapPixels);

    return impactDirection;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Load OBJ mesh data
static Mesh LoadOBJ(const char *fileName)
{
    Mesh mesh = { 0 };

    char dataType;
    char comments[200];

    int numVertex = 0;
    int numNormals = 0;
    int numTexCoords = 0;
    int numTriangles = 0;

    FILE *objFile;

    objFile = fopen(fileName, "rt");

    if (objFile == NULL)
    {
        TraceLog(WARNING, "[%s] OBJ file could not be opened", fileName);
        return mesh;
    }

    // First reading pass: Get numVertex, numNormals, numTexCoords, numTriangles
    // NOTE: vertex, texcoords and normals could be optimized (to be used indexed on faces definition)
    // NOTE: faces MUST be defined as TRIANGLES (3 vertex per face)
    while (!feof(objFile))
    {
        fscanf(objFile, "%c", &dataType);

        switch (dataType)
        {
            case '#':   // Comments
            case 'o':   // Object name (One OBJ file can contain multible named meshes)
            case 'g':   // Group name
            case 's':   // Smoothing level
            case 'm':   // mtllib [external .mtl file name]
            case 'u':   // usemtl [material name]
            {
                fgets(comments, 200, objFile);
            } break;
            case 'v':
            {
                fscanf(objFile, "%c", &dataType);

                if (dataType == 't')    // Read texCoord
                {
                    numTexCoords++;
                    fgets(comments, 200, objFile);
                }
                else if (dataType == 'n')    // Read normals
                {
                    numNormals++;
                    fgets(comments, 200, objFile);
                }
                else    // Read vertex
                {
                    numVertex++;
                    fgets(comments, 200, objFile);
                }
            } break;
            case 'f':
            {
                numTriangles++;
                fgets(comments, 200, objFile);
            } break;
            default: break;
        }
    }

    TraceLog(DEBUG, "[%s] Model num vertices: %i", fileName, numVertex);
    TraceLog(DEBUG, "[%s] Model num texcoords: %i", fileName, numTexCoords);
    TraceLog(DEBUG, "[%s] Model num normals: %i", fileName, numNormals);
    TraceLog(DEBUG, "[%s] Model num triangles: %i", fileName, numTriangles);

    // Once we know the number of vertices to store, we create required arrays
    Vector3 *midVertices = (Vector3 *)malloc(numVertex*sizeof(Vector3));
    Vector3 *midNormals = NULL;
    if (numNormals > 0) midNormals = (Vector3 *)malloc(numNormals*sizeof(Vector3));
    Vector2 *midTexCoords = NULL;
    if (numTexCoords > 0) midTexCoords = (Vector2 *)malloc(numTexCoords*sizeof(Vector2));

    int countVertex = 0;
    int countNormals = 0;
    int countTexCoords = 0;

    rewind(objFile);        // Return to the beginning of the file, to read again

    // Second reading pass: Get vertex data to fill intermediate arrays
    // NOTE: This second pass is required in case of multiple meshes defined in same OBJ
    // TODO: Consider that different meshes can have different vertex data available (position, texcoords, normals)
    while (!feof(objFile))
    {
        fscanf(objFile, "%c", &dataType);

        switch (dataType)
        {
            case '#': case 'o': case 'g': case 's': case 'm': case 'u': case 'f': fgets(comments, 200, objFile); break;
            case 'v':
            {
                fscanf(objFile, "%c", &dataType);

                if (dataType == 't')    // Read texCoord
                {
                    float useless = 0;

                    fscanf(objFile, "%f %f %f", &midTexCoords[countTexCoords].x, &midTexCoords[countTexCoords].y, &useless);
                    countTexCoords++;

                    fscanf(objFile, "%c", &dataType);
                }
                else if (dataType == 'n')    // Read normals
                {
                    fscanf(objFile, "%f %f %f", &midNormals[countNormals].x, &midNormals[countNormals].y, &midNormals[countNormals].z );
                    countNormals++;

                    fscanf(objFile, "%c", &dataType);
                }
                else    // Read vertex
                {
                    fscanf(objFile, "%f %f %f", &midVertices[countVertex].x, &midVertices[countVertex].y, &midVertices[countVertex].z );
                    countVertex++;

                    fscanf(objFile, "%c", &dataType);
                }
            } break;
            default: break;
        }
    }

    // At this point all vertex data (v, vt, vn) has been gathered on midVertices, midTexCoords, midNormals
    // Now we can organize that data into our Mesh struct

    mesh.vertexCount = numTriangles*3;

    // Additional arrays to store vertex data as floats
    mesh.vertices = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)malloc(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.colors = NULL;

    int vCounter = 0;       // Used to count vertices float by float
    int tcCounter = 0;      // Used to count texcoords float by float
    int nCounter = 0;       // Used to count normals float by float

    int vNum[3], vtNum[3], vnNum[3];    // Used to store triangle indices for v, vt, vn

    rewind(objFile);        // Return to the beginning of the file, to read again

    if (numNormals == 0) TraceLog(INFO, "[%s] No normals data on OBJ, normals will be generated from faces data", fileName);

    // Third reading pass: Get faces (triangles) data and fill VertexArray
    while (!feof(objFile))
    {
        fscanf(objFile, "%c", &dataType);

        switch (dataType)
        {
            case '#': case 'o': case 'g': case 's': case 'm': case 'u': case 'v': fgets(comments, 200, objFile); break;
            case 'f':
            {
                // NOTE: It could be that OBJ does not have normals or texcoords defined!

                if ((numNormals == 0) && (numTexCoords == 0)) fscanf(objFile, "%i %i %i", &vNum[0], &vNum[1], &vNum[2]);
                else if (numNormals == 0) fscanf(objFile, "%i/%i %i/%i %i/%i", &vNum[0], &vtNum[0], &vNum[1], &vtNum[1], &vNum[2], &vtNum[2]);
                else fscanf(objFile, "%i/%i/%i %i/%i/%i %i/%i/%i", &vNum[0], &vtNum[0], &vnNum[0], &vNum[1], &vtNum[1], &vnNum[1], &vNum[2], &vtNum[2], &vnNum[2]);

                mesh.vertices[vCounter] = midVertices[vNum[0]-1].x;
                mesh.vertices[vCounter + 1] = midVertices[vNum[0]-1].y;
                mesh.vertices[vCounter + 2] = midVertices[vNum[0]-1].z;
                vCounter += 3;
                mesh.vertices[vCounter] = midVertices[vNum[1]-1].x;
                mesh.vertices[vCounter + 1] = midVertices[vNum[1]-1].y;
                mesh.vertices[vCounter + 2] = midVertices[vNum[1]-1].z;
                vCounter += 3;
                mesh.vertices[vCounter] = midVertices[vNum[2]-1].x;
                mesh.vertices[vCounter + 1] = midVertices[vNum[2]-1].y;
                mesh.vertices[vCounter + 2] = midVertices[vNum[2]-1].z;
                vCounter += 3;

                if (numNormals > 0)
                {
                    mesh.normals[nCounter] = midNormals[vnNum[0]-1].x;
                    mesh.normals[nCounter + 1] = midNormals[vnNum[0]-1].y;
                    mesh.normals[nCounter + 2] = midNormals[vnNum[0]-1].z;
                    nCounter += 3;
                    mesh.normals[nCounter] = midNormals[vnNum[1]-1].x;
                    mesh.normals[nCounter + 1] = midNormals[vnNum[1]-1].y;
                    mesh.normals[nCounter + 2] = midNormals[vnNum[1]-1].z;
                    nCounter += 3;
                    mesh.normals[nCounter] = midNormals[vnNum[2]-1].x;
                    mesh.normals[nCounter + 1] = midNormals[vnNum[2]-1].y;
                    mesh.normals[nCounter + 2] = midNormals[vnNum[2]-1].z;
                    nCounter += 3;
                }
                else
                {
                    // If normals not defined, they are calculated from the 3 vertices [N = (V2 - V1) x (V3 - V1)]
                    Vector3 norm = VectorCrossProduct(VectorSubtract(midVertices[vNum[1]-1], midVertices[vNum[0]-1]), VectorSubtract(midVertices[vNum[2]-1], midVertices[vNum[0]-1]));
                    VectorNormalize(&norm);

                    mesh.normals[nCounter] = norm.x;
                    mesh.normals[nCounter + 1] = norm.y;
                    mesh.normals[nCounter + 2] = norm.z;
                    nCounter += 3;
                    mesh.normals[nCounter] = norm.x;
                    mesh.normals[nCounter + 1] = norm.y;
                    mesh.normals[nCounter + 2] = norm.z;
                    nCounter += 3;
                    mesh.normals[nCounter] = norm.x;
                    mesh.normals[nCounter + 1] = norm.y;
                    mesh.normals[nCounter + 2] = norm.z;
                    nCounter += 3;
                }

                if (numTexCoords > 0)
                {
                    // NOTE: If using negative texture coordinates with a texture filter of GL_CLAMP_TO_EDGE doesn't work!
                    // NOTE: Texture coordinates are Y flipped upside-down
                    mesh.texcoords[tcCounter] = midTexCoords[vtNum[0]-1].x;
                    mesh.texcoords[tcCounter + 1] = 1.0f - midTexCoords[vtNum[0]-1].y;
                    tcCounter += 2;
                    mesh.texcoords[tcCounter] = midTexCoords[vtNum[1]-1].x;
                    mesh.texcoords[tcCounter + 1] = 1.0f - midTexCoords[vtNum[1]-1].y;
                    tcCounter += 2;
                    mesh.texcoords[tcCounter] = midTexCoords[vtNum[2]-1].x;
                    mesh.texcoords[tcCounter + 1] = 1.0f - midTexCoords[vtNum[2]-1].y;
                    tcCounter += 2;
                }
            } break;
            default: break;
        }
    }

    fclose(objFile);

    // Security check, just in case no normals or no texcoords defined in OBJ
    if (numTexCoords == 0) for (int i = 0; i < (2*mesh.vertexCount); i++) mesh.texcoords[i] = 0.0f;

    // Now we can free temp mid* arrays
    free(midVertices);
    free(midNormals);
    free(midTexCoords);

    // NOTE: At this point we have all vertex, texcoord, normal data for the model in mesh struct
    TraceLog(INFO, "[%s] Model loaded successfully in RAM (CPU)", fileName);

    return mesh;
}

// Load MTL material data (specs: http://paulbourke.net/dataformats/mtl/)
// NOTE: Texture map parameters are not supported
static Material LoadMTL(const char *fileName)
{
    #define MAX_BUFFER_SIZE     128
    
    Material material = { 0 };  // LoadDefaultMaterial();
    
    char buffer[MAX_BUFFER_SIZE];
    Vector3 color = { 1.0f, 1.0f, 1.0f };
    char *mapFileName = NULL;

    FILE *mtlFile;

    mtlFile = fopen(fileName, "rt");

    if (mtlFile == NULL)
    {
        TraceLog(WARNING, "[%s] MTL file could not be opened", fileName);
        return material;
    }

    while (!feof(mtlFile))
    {
        fgets(buffer, MAX_BUFFER_SIZE, mtlFile);
        
        switch (buffer[0])
        {
            case 'n':   // newmtl string    Material name. Begins a new material description.
            {
                // TODO: Support multiple materials in a single .mtl
                sscanf(buffer, "newmtl %s", mapFileName);
                
                TraceLog(INFO, "[%s] Loading material...", mapFileName);
            }
            case 'i':   // illum int        Illumination model
            {
                // illum = 1 if specular disabled
                // illum = 2 if specular enabled (lambertian model)
                // ...
            }
            case 'K':   // Ka, Kd, Ks, Ke
            {
                switch (buffer[1])
                {
                    case 'a':   // Ka float float float    Ambient color (RGB)
                    {
                        sscanf(buffer, "Ka %f %f %f", &color.x, &color.y, &color.z);
                        material.colAmbient.r = (unsigned char)(color.x*255);
                        material.colAmbient.g = (unsigned char)(color.y*255);
                        material.colAmbient.b = (unsigned char)(color.z*255);
                    } break;
                    case 'd':   // Kd float float float     Diffuse color (RGB)
                    {
                        sscanf(buffer, "Kd %f %f %f", &color.x, &color.y, &color.z);
                        material.colDiffuse.r = (unsigned char)(color.x*255);
                        material.colDiffuse.g = (unsigned char)(color.y*255);
                        material.colDiffuse.b = (unsigned char)(color.z*255);
                    } break;
                    case 's':   // Ks float float float     Specular color (RGB)
                    {
                        sscanf(buffer, "Ks %f %f %f", &color.x, &color.y, &color.z);
                        material.colSpecular.r = (unsigned char)(color.x*255);
                        material.colSpecular.g = (unsigned char)(color.y*255);
                        material.colSpecular.b = (unsigned char)(color.z*255);
                    } break;
                    case 'e':   // Ke float float float     Emmisive color (RGB)
                    {
                        // TODO: Support Ke ?
                    } break;
                    default: break;
                }
            } break;
            case 'N':   // Ns, Ni
            {
                if (buffer[1] == 's')       // Ns int   Shininess (specular exponent). Ranges from 0 to 1000.
                {
                    int shininess = 0;
                    sscanf(buffer, "Ns %i", &shininess);
                    
                    material.glossiness = (float)shininess;
                }
                else if (buffer[1] == 'i')  // Ni int   Refraction index.
                {
                    // Not supported...
                }
            } break;
            case 'm':   // map_Kd, map_Ks, map_Ka, map_Bump, map_d
            {
                switch (buffer[4])
                {
                    case 'K':   // Color texture maps
                    {
                        if (buffer[5] == 'd')       // map_Kd string    Diffuse color texture map.
                        {
                            sscanf(buffer, "map_Kd %s", mapFileName);
                            if (mapFileName != NULL) material.texDiffuse = LoadTexture(mapFileName);
                        }
                        else if (buffer[5] == 's')  // map_Ks string    Specular color texture map.
                        {
                            sscanf(buffer, "map_Ks %s", mapFileName);
                            if (mapFileName != NULL) material.texSpecular = LoadTexture(mapFileName);
                        }
                        else if (buffer[5] == 'a')  // map_Ka string    Ambient color texture map.
                        {
                            // Not supported...
                        }
                    } break;
                    case 'B':       // map_Bump string      Bump texture map.
                    {
                        sscanf(buffer, "map_Bump %s", mapFileName);
                        if (mapFileName != NULL) material.texNormal = LoadTexture(mapFileName);
                    } break;
                    case 'b':       // map_bump string      Bump texture map.
                    {
                        sscanf(buffer, "map_bump %s", mapFileName);
                        if (mapFileName != NULL) material.texNormal = LoadTexture(mapFileName);
                    } break;
                    case 'd':       // map_d string         Opacity texture map.
                    {
                        // Not supported...
                    } break;
                    default: break;
                }
            } break;
            case 'd':   // d, disp
            {
                if (buffer[1] == ' ')       // d float      Dissolve factor. d is inverse of Tr
                {
                    float alpha = 1.0f;
                    sscanf(buffer, "d %f", &alpha);
                    material.colDiffuse.a = (unsigned char)(alpha*255);
                }
                else if (buffer[1] == 'i')  // disp string  Displacement map
                {
                    // Not supported...
                }
            } break;
            case 'b':   // bump string      Bump texture map
            {
                sscanf(buffer, "bump %s", mapFileName);
                if (mapFileName != NULL) material.texNormal = LoadTexture(mapFileName);
            } break;
            case 'T':   // Tr float         Transparency Tr (alpha). Tr is inverse of d
            {
                float ialpha = 0.0f;
                sscanf(buffer, "Tr %f", &ialpha);
                material.colDiffuse.a = (unsigned char)((1.0f - ialpha)*255);
                
            } break;
            case 'r':   // refl string      Reflection texture map
            default: break;
        }
    }

    fclose(mtlFile);

    // NOTE: At this point we have all material data
    TraceLog(INFO, "[%s] Material loaded successfully", fileName);
    
    return material;
}
