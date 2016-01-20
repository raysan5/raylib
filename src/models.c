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
    #include "utils.h"  // Android fopen function map
#endif

#include <stdio.h>      // Standard input/output functions, used to read model files data
#include <stdlib.h>     // Declares malloc() and free() for memory management
#include <string.h>     // Required for strcmp()
#include <math.h>       // Used for sin, cos, tan

#include "rlgl.h"       // raylib OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2
#include "raymath.h"    // Required for data type Matrix and Matrix functions

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
extern unsigned int whiteTexture;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static float GetHeightValue(Color pixel);
static Mesh LoadOBJ(const char *fileName);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

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

            for(int i = 0; i < (rings + 2); i++)
            {
                for(int j = 0; j < slices; j++)
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

            for(int i = 0; i < (rings + 2); i++)
            {
                for(int j = 0; j < slices; j++)
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
                for(int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(sin(DEG2RAD*i) * radiusBottom, 0, cos(DEG2RAD*i) * radiusBottom); //Bottom Left
                    rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusBottom, 0, cos(DEG2RAD*(i+360/sides)) * radiusBottom); //Bottom Right
                    rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusTop, height, cos(DEG2RAD*(i+360/sides)) * radiusTop); //Top Right

                    rlVertex3f(sin(DEG2RAD*i) * radiusTop, height, cos(DEG2RAD*i) * radiusTop); //Top Left
                    rlVertex3f(sin(DEG2RAD*i) * radiusBottom, 0, cos(DEG2RAD*i) * radiusBottom); //Bottom Left
                    rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusTop, height, cos(DEG2RAD*(i+360/sides)) * radiusTop); //Top Right
                }

                // Draw Cap --------------------------------------------------------------------------------------
                for(int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(0, height, 0);
                    rlVertex3f(sin(DEG2RAD*i) * radiusTop, height, cos(DEG2RAD*i) * radiusTop);
                    rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusTop, height, cos(DEG2RAD*(i+360/sides)) * radiusTop);
                }
            }
            else
            {
                // Draw Cone -------------------------------------------------------------------------------------
                for(int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(0, height, 0);
                    rlVertex3f(sin(DEG2RAD*i) * radiusBottom, 0, cos(DEG2RAD*i) * radiusBottom);
                    rlVertex3f(sin(DEG2RAD*(i+360/sides)) * radiusBottom, 0, cos(DEG2RAD*(i+360/sides)) * radiusBottom);
                }
            }

            // Draw Base -----------------------------------------------------------------------------------------
            for(int i = 0; i < 360; i += 360/sides)
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
    if(sides < 3) sides = 3;

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for(int i = 0; i < 360; i += 360/sides)
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
    // NOTE: QUADS usage require defining a texture on OpenGL 3.3+
    if (rlGetVersion() != OPENGL_11) rlEnableTexture(whiteTexture);    // Default white texture

    // NOTE: Plane is always created on XZ ground
    rlPushMatrix();
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(size.x, 1.0f, size.y);

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlNormal3f(0.0f, 1.0f, 0.0f);
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-0.5f, 0.0f, -0.5f);
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(-0.5f, 0.0f, 0.5f);
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.5f, 0.0f, 0.5f);
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.5f, 0.0f, -0.5f);
        rlEnd();
    rlPopMatrix();

    if (rlGetVersion() != OPENGL_11) rlDisableTexture();
}

// Draw a quad
void DrawQuad(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4, Color color)
{
    // TODO: Calculate normals from vertex position
    
    rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);
        //rlNormal3f(0.0f, 0.0f, 0.0f);

        rlVertex3f(v1.x, v1.y, v1.z);
        rlVertex3f(v2.x, v2.y, v2.z);
        rlVertex3f(v3.x, v3.y, v3.z);
        rlVertex3f(v4.x, v4.y, v4.z);
    rlEnd();
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
        for(int i = -halfSlices; i <= halfSlices; i++)
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

// Load a 3d model (from file)
Model LoadModel(const char *fileName)
{
    Model model;
    Mesh mesh = { 0 };
    
    // NOTE: Initialize default data for model in case loading fails, maybe a cube?

    if (strcmp(GetExtension(fileName),"obj") == 0) mesh = LoadOBJ(fileName);
    else TraceLog(WARNING, "[%s] Model extension not recognized, it can't be loaded", fileName);

    // NOTE: At this point we have all vertex, texcoord, normal data for the model in mesh struct
    
    if (mesh.vertexCount == 0)
    {
        TraceLog(WARNING, "Model could not be loaded");
    }
    else
    {
        // NOTE: model properties (transform, texture, shader) are initialized inside rlglLoadModel()
        model = rlglLoadModel(mesh);     // Upload vertex data to GPU

        // Now that vertex data is uploaded to GPU, we can free arrays
        // NOTE 1: We don't need CPU vertex data on OpenGL 3.3 or ES2... for static meshes...
        // NOTE 2: ...but we could keep CPU vertex data in case we need to update the mesh
        
        /*
        if (rlGetVersion() != OPENGL_11)
        {
            free(mesh.vertices);
            free(mesh.texcoords);
            free(mesh.normals);
            free(mesh.colors);
        }
        */
    }

    return model;
}

// Load a 3d model (from vertex data)
Model LoadModelEx(Mesh data)
{
    Model model;

    // NOTE: model properties (transform, texture, shader) are initialized inside rlglLoadModel()
    model = rlglLoadModel(data);     // Upload vertex data to GPU
    
    // NOTE: Vertex data is managed externally, must be deallocated manually
    
    return model;
}

// Load a heightmap image as a 3d model
Model LoadHeightmap(Image heightmap, float maxHeight)
{
    Mesh mesh;

    int mapX = heightmap.width;
    int mapZ = heightmap.height;
    
    Color *heightmapPixels = GetImageData(heightmap);

    // NOTE: One vertex per pixel
    // TODO: Consider resolution when generating model data?
    int numTriangles = (mapX-1)*(mapZ-1)*2;    // One quad every four pixels

    mesh.vertexCount = numTriangles*3;

    mesh.vertices = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.normals = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)malloc(mesh.vertexCount*2*sizeof(float));
    mesh.colors = (unsigned char *)malloc(mesh.vertexCount*4*sizeof(unsigned char)); // Not used...

    int vCounter = 0;       // Used to count vertices float by float
    int tcCounter = 0;      // Used to count texcoords float by float
    int nCounter = 0;       // Used to count normals float by float

    int trisCounter = 0;

    float scaleFactor = maxHeight/255;    // TODO: Review scaleFactor calculation

    for(int z = 0; z < mapZ-1; z++)
    {
        for(int x = 0; x < mapX-1; x++)
        {
            // Fill vertices array with data
            //----------------------------------------------------------

            // one triangle - 3 vertex
            mesh.vertices[vCounter] = x;
            mesh.vertices[vCounter + 1] = GetHeightValue(heightmapPixels[x + z*mapX])*scaleFactor;
            mesh.vertices[vCounter + 2] = z;

            mesh.vertices[vCounter + 3] = x;
            mesh.vertices[vCounter + 4] = GetHeightValue(heightmapPixels[x + (z+1)*mapX])*scaleFactor;
            mesh.vertices[vCounter + 5] = z+1;

            mesh.vertices[vCounter + 6] = x+1;
            mesh.vertices[vCounter + 7] = GetHeightValue(heightmapPixels[(x+1) + z*mapX])*scaleFactor;
            mesh.vertices[vCounter + 8] = z;

            // another triangle - 3 vertex
            mesh.vertices[vCounter + 9] = mesh.vertices[vCounter + 6];
            mesh.vertices[vCounter + 10] = mesh.vertices[vCounter + 7];
            mesh.vertices[vCounter + 11] = mesh.vertices[vCounter + 8];

            mesh.vertices[vCounter + 12] = mesh.vertices[vCounter + 3];
            mesh.vertices[vCounter + 13] = mesh.vertices[vCounter + 4];
            mesh.vertices[vCounter + 14] = mesh.vertices[vCounter + 5];

            mesh.vertices[vCounter + 15] = x+1;
            mesh.vertices[vCounter + 16] = GetHeightValue(heightmapPixels[(x+1) + (z+1)*mapX])*scaleFactor;
            mesh.vertices[vCounter + 17] = z+1;
            vCounter += 18;     // 6 vertex, 18 floats

            // Fill texcoords array with data
            //--------------------------------------------------------------
            mesh.texcoords[tcCounter] = (float)x / (mapX-1);
            mesh.texcoords[tcCounter + 1] = (float)z / (mapZ-1);

            mesh.texcoords[tcCounter + 2] = (float)x / (mapX-1);
            mesh.texcoords[tcCounter + 3] = (float)(z+1) / (mapZ-1);

            mesh.texcoords[tcCounter + 4] = (float)(x+1) / (mapX-1);
            mesh.texcoords[tcCounter + 5] = (float)z / (mapZ-1);

            mesh.texcoords[tcCounter + 6] = mesh.texcoords[tcCounter + 4];
            mesh.texcoords[tcCounter + 7] = mesh.texcoords[tcCounter + 5];

            mesh.texcoords[tcCounter + 8] = mesh.texcoords[tcCounter + 2];
            mesh.texcoords[tcCounter + 9] = mesh.texcoords[tcCounter + 3];

            mesh.texcoords[tcCounter + 10] = (float)(x+1) / (mapX-1);
            mesh.texcoords[tcCounter + 11] = (float)(z+1) / (mapZ-1);
            tcCounter += 12;    // 6 texcoords, 12 floats

            // Fill normals array with data
            //--------------------------------------------------------------
            // NOTE: Current Model implementation doe not use normals!
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
    
    free(heightmapPixels);

    // Fill color data
    // NOTE: Not used any more... just one plain color defined at DrawModel()
    for (int i = 0; i < (4*mesh.vertexCount); i++) mesh.colors[i] = 255;

    // NOTE: At this point we have all vertex, texcoord, normal data for the model in mesh struct

    Model model = rlglLoadModel(mesh);

    // Now that vertex data is uploaded to GPU, we can free arrays
    // NOTE: We don't need CPU vertex data on OpenGL 3.3 or ES2
    if (rlGetVersion() != OPENGL_11)
    {
        free(mesh.vertices);
        free(mesh.texcoords);
        free(mesh.normals);
        free(mesh.colors);
    }

    return model;
}

// Load a map image as a 3d model (cubes based)
Model LoadCubicmap(Image cubicmap)
{
    Mesh mesh;

    Color *cubicmapPixels = GetImageData(cubicmap);
    
    // Map cube size will be 1.0
    float mapCubeSide = 1.0f;
    int mapWidth = cubicmap.width*(int)mapCubeSide;
    int mapHeight = cubicmap.height*(int)mapCubeSide;

    // NOTE: Max possible number of triangles numCubes * (12 triangles by cube)
    int maxTriangles = cubicmap.width*cubicmap.height*12;

    int vCounter = 0;       // Used to count vertices
    int tcCounter = 0;      // Used to count texcoords
    int nCounter = 0;       // Used to count normals

    float w = mapCubeSide;
    float h = mapCubeSide;
    float h2 = mapCubeSide*1.5f;   // TODO: Review walls height...

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

    for (int z = 0; z < mapHeight; z += mapCubeSide)
    {
        for (int x = 0; x < mapWidth; x += mapCubeSide)
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
    mesh.colors = (unsigned char *)malloc(mesh.vertexCount*4*sizeof(unsigned char));  // Not used...

    // Fill color data
    // NOTE: Not used any more... just one plain color defined at DrawModel()
    for (int i = 0; i < (4*mesh.vertexCount); i++) mesh.colors[i] = 255;

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
    
    free(cubicmapPixels);

    // NOTE: At this point we have all vertex, texcoord, normal data for the model in mesh struct

    Model model = rlglLoadModel(mesh);

    // Now that vertex data is uploaded to GPU, we can free arrays
    // NOTE: We don't need CPU vertex data on OpenGL 3.3 or ES2
    if (rlGetVersion() != OPENGL_11)
    {
        free(mesh.vertices);
        free(mesh.texcoords);
        free(mesh.normals);
        free(mesh.colors);
    }

    return model;
}

// Unload 3d model from memory
void UnloadModel(Model model)
{
    if (rlGetVersion() == OPENGL_11)
    {
        free(model.mesh.vertices);
        free(model.mesh.texcoords);
        free(model.mesh.normals);
    }

    rlDeleteBuffers(model.mesh.vboId[0]);
    rlDeleteBuffers(model.mesh.vboId[1]);
    rlDeleteBuffers(model.mesh.vboId[2]);

    rlDeleteVertexArrays(model.mesh.vaoId);
    
    if (model.mesh.vaoId > 0) TraceLog(INFO, "[VAO ID %i] Unloaded model data from VRAM (GPU)", model.mesh.vaoId);
    else TraceLog(INFO, "[VBO ID %i][VBO ID %i][VBO ID %i] Unloaded model data from VRAM (GPU)", model.mesh.vboId[0], model.mesh.vboId[1], model.mesh.vboId[2]);
}

// Link a texture to a model
void SetModelTexture(Model *model, Texture2D texture)
{
    if (texture.id <= 0)
    {
        // Use default white texture (use mesh color)
        model->texture.id = whiteTexture;               // OpenGL 1.1
        model->shader.texDiffuseId = whiteTexture;      // OpenGL 3.3 / ES 2.0
    }
    else
    {
        model->texture = texture;
        model->shader.texDiffuseId = texture.id;
    }
}

// Draw a model (with texture if set)
void DrawModel(Model model, Vector3 position, float scale, Color tint)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 0.0f, 0.0f, 0.0f };

    DrawModelEx(model, position, 0.0f, rotationAxis, vScale, tint);
}

// Draw a model with extended parameters
void DrawModelEx(Model model, Vector3 position, float rotationAngle, Vector3 rotationAxis, Vector3 scale, Color tint)
{
    // NOTE: Rotation must be provided in degrees, it's converted to radians inside rlglDrawModel()
    rlglDrawModel(model, position, rotationAngle, rotationAxis, scale, tint, false);
}

// Draw a model wires (with texture if set)
void DrawModelWires(Model model, Vector3 position, float scale, Color color)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 0.0f, 0.0f, 0.0f };

    rlglDrawModel(model, position, 0.0f, rotationAxis, vScale, color, true);
}

// Draw a model wires (with texture if set) with extended parameters
void DrawModelWiresEx(Model model, Vector3 position, float rotationAngle, Vector3 rotationAxis, Vector3 scale, Color tint)
{
    // NOTE: Rotation must be provided in degrees, it's converted to radians inside rlglDrawModel()
    rlglDrawModel(model, position, rotationAngle, rotationAxis, scale, tint, true);
}

// Draw a billboard
void DrawBillboard(Camera camera, Texture2D texture, Vector3 center, float size, Color tint)
{
    // NOTE: Billboard size will maintain texture aspect ratio, size will be billboard width
    Vector2 sizeRatio = { size, size * (float)texture.height/texture.width };

    Matrix viewMatrix = MatrixLookAt(camera.position, camera.target, camera.up);
    MatrixTranspose(&viewMatrix);

    Vector3 right = { viewMatrix.m0, viewMatrix.m4, viewMatrix.m8 };
    //Vector3 up = { viewMatrix.m1, viewMatrix.m5, viewMatrix.m9 };
    
    // NOTE: Billboard locked to axis-Y
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

        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(a.x, a.y, a.z);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(d.x, d.y, d.z);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(c.x, c.y, c.z);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(b.x, b.y, b.z);
    rlEnd();

    rlDisableTexture();
}

// Draw a billboard (part of a texture defined by a rectangle)
void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle sourceRec, Vector3 center, float size, Color tint)
{
    // NOTE: Billboard size will maintain sourceRec aspect ratio, size will represent billboard width
    Vector2 sizeRatio = { size, size * (float)sourceRec.height/sourceRec.width };

    Matrix viewMatrix = MatrixLookAt(camera.position, camera.target, camera.up);
    MatrixTranspose(&viewMatrix);

    Vector3 right = { viewMatrix.m0, viewMatrix.m4, viewMatrix.m8 };
    Vector3 up = { viewMatrix.m1, viewMatrix.m5, viewMatrix.m9 };
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
bool CheckCollisionBoxes(Vector3 minBBox1, Vector3 maxBBox1, Vector3 minBBox2, Vector3 maxBBox2)
{
    bool collision = true;

    if ((maxBBox1.x >= minBBox2.x) && (minBBox1.x <= maxBBox2.x))
    {
        if ((maxBBox1.y < minBBox2.y) || (minBBox1.y > maxBBox2.y)) collision = false;
        if ((maxBBox1.z < minBBox2.z) || (minBBox1.z > maxBBox2.z)) collision = false;
    }
    else collision = false;

    return collision;
}

// Detect collision between box and sphere
bool CheckCollisionBoxSphere(Vector3 minBBox, Vector3 maxBBox, Vector3 centerSphere, float radiusSphere)
{
    bool collision = false;

    if ((centerSphere.x - minBBox.x > radiusSphere) && (centerSphere.y - minBBox.y > radiusSphere) && (centerSphere.z - minBBox.z > radiusSphere) &&
            (maxBBox.x - centerSphere.x > radiusSphere) && (maxBBox.y - centerSphere.y > radiusSphere) && (maxBBox.z - centerSphere.z > radiusSphere))
    {
        collision = true;
    }
    else
    {
        float dmin = 0;

        if (centerSphere.x - minBBox.x <= radiusSphere) dmin += (centerSphere.x - minBBox.x)*(centerSphere.x - minBBox.x);
        else if (maxBBox.x - centerSphere.x <= radiusSphere) dmin += (centerSphere.x - maxBBox.x)*(centerSphere.x - maxBBox.x);

        if (centerSphere.y - minBBox.y <= radiusSphere) dmin += (centerSphere.y - minBBox.y)*(centerSphere.y - minBBox.y);
        else if (maxBBox.y - centerSphere.y <= radiusSphere) dmin += (centerSphere.y - maxBBox.y)*(centerSphere.y - maxBBox.y);

        if (centerSphere.z - minBBox.z <= radiusSphere) dmin += (centerSphere.z - minBBox.z)*(centerSphere.z - minBBox.z);
        else if (maxBBox.z - centerSphere.z <= radiusSphere) dmin += (centerSphere.z - maxBBox.z)*(centerSphere.z - maxBBox.z);

        if (dmin <= radiusSphere*radiusSphere) collision = true;
    }

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
    
    if(d >= 0.0f) collision = true;
    
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
    
    if(d >= 0.0f) collision = true;
    
    // Calculate collision point
    Vector3 offset = ray.direction;
    float collisionDistance = 0;
    
    // Check if ray origin is inside the sphere to calculate the correct collision point
    if(distance < sphereRadius) collisionDistance = vector + sqrt(d);
    else collisionDistance = vector - sqrt(d);
    
    VectorScale(&offset, collisionDistance);
    Vector3 cPoint = VectorAdd(ray.position, offset);
    
    collisionPoint->x = cPoint.x;
    collisionPoint->y = cPoint.y;
    collisionPoint->z = cPoint.z;
    
    return collision;
}

// Detect collision between ray and bounding box
bool CheckCollisionRayBox(Ray ray, Vector3 minBBox, Vector3 maxBBox)
{
    bool collision = false;
    
    float t[8];
    t[0] = (minBBox.x - ray.position.x)/ray.direction.x;
    t[1] = (maxBBox.x - ray.position.x)/ray.direction.x;
    t[2] = (minBBox.y - ray.position.y)/ray.direction.y;
    t[3] = (maxBBox.y - ray.position.y)/ray.direction.y;
    t[4] = (minBBox.z - ray.position.z)/ray.direction.z;
    t[5] = (maxBBox.z - ray.position.z)/ray.direction.z;
    t[6] = fmax(fmax(fmin(t[0], t[1]), fmin(t[2], t[3])), fmin(t[4], t[5]));
    t[7] = fmin(fmin(fmax(t[0], t[1]), fmax(t[2], t[3])), fmax(t[4], t[5]));
    
    collision = !(t[7] < 0 || t[6] > t[7]);
    
    return collision;
}

// TODO: Useful function to check collision area?
//BoundingBox GetCollisionArea(BoundingBox box1, BoundingBox box2)

// Calculate mesh bounding box limits
BoundingBox CalculateBoundingBox(Mesh mesh)
{
    // Get min and max vertex to construct bounds (AABB)
    Vector3 minVertex = (Vector3){ mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
    Vector3 maxVertex = (Vector3){ mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };

    for (int i = 1; i < mesh.vertexCount; i++)
    {
		// TODO: Compare min and max with previous vertex
        //minVertex = Vector3.Min(minVertex, mesh.vertices[i]);
        //maxVertex = Vector3.Max(maxVertex, mesh.vertices[i]);
    }

    // NOTE: For OBB, transform mesh by model transform matrix
    //minVertex = VectorTransform(meshMin, mesh.transform);
    //maxVertex = VectorTransform(meshMax, mesh.transform);
 
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

// Get current vertex y altitude (proportional to pixel colors in grayscale)
static float GetHeightValue(Color pixel)
{
    return (((float)pixel.r + (float)pixel.g + (float)pixel.b)/3);
}

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
    // NOTE: faces MUST be defined as TRIANGLES, not QUADS
    while(!feof(objFile))
    {
        fscanf(objFile, "%c", &dataType);

        switch(dataType)
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
    while(!feof(objFile))
    {
        fscanf(objFile, "%c", &dataType);

        switch(dataType)
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
    mesh.colors = (unsigned char *)malloc(mesh.vertexCount*4*sizeof(unsigned char));

    int vCounter = 0;       // Used to count vertices float by float
    int tcCounter = 0;      // Used to count texcoords float by float
    int nCounter = 0;       // Used to count normals float by float

    int vNum[3], vtNum[3], vnNum[3];    // Used to store triangle indices for v, vt, vn

    rewind(objFile);        // Return to the beginning of the file, to read again

    if (numNormals == 0) TraceLog(INFO, "[%s] No normals data on OBJ, normals will be generated from faces data", fileName);

    // Third reading pass: Get faces (triangles) data and fill VertexArray
    while(!feof(objFile))
    {
        fscanf(objFile, "%c", &dataType);

        switch(dataType)
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
    
    // NOTE: We set all vertex colors to white
    // NOTE: Not used any more... just one plain color defined at DrawModel()
    for (int i = 0; i < (4*mesh.vertexCount); i++) mesh.colors[i] = 255;

    // Now we can free temp mid* arrays
    free(midVertices);
    free(midNormals);
    free(midTexCoords);

    // NOTE: At this point we have all vertex, texcoord, normal data for the model in mesh struct
    TraceLog(INFO, "[%s] Model loaded successfully in RAM (CPU)", fileName);

    return mesh;
}
