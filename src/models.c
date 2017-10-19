/**********************************************************************************************
*
*   raylib.models - Basic functions to deal with 3d shapes and 3d models
*
*   CONFIGURATION:
*
*   #define SUPPORT_FILEFORMAT_OBJ
*       Selected desired fileformats to be supported for loading.
*
*   #define SUPPORT_FILEFORMAT_MTL
*       Selected desired fileformats to be supported for loading.
*
*   #define SUPPORT_MESH_GENERATION
*       Support procedural mesh generation functions, uses external par_shapes.h library
*       NOTE: Some generated meshes DO NOT include generated texture coordinates
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
#define SUPPORT_FILEFORMAT_OBJ
#define SUPPORT_FILEFORMAT_MTL
#define SUPPORT_MESH_GENERATION
//-------------------------------------------------

#include "raylib.h"

#if defined(PLATFORM_ANDROID)
    #include "utils.h"      // Android fopen function map
#endif

#include <stdio.h>          // Required for: FILE, fopen(), fclose(), fscanf(), feof(), rewind(), fgets()
#include <stdlib.h>         // Required for: malloc(), free()
#include <string.h>         // Required for: strcmp()
#include <math.h>           // Required for: sin(), cos()

#include "rlgl.h"           // raylib OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2

#define PAR_SHAPES_IMPLEMENTATION
#include "external/par_shapes.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// ...

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
#if defined(SUPPORT_FILEFORMAT_OBJ)
static Mesh LoadOBJ(const char *fileName);      // Load OBJ mesh data
#endif
#if defined(SUPPORT_FILEFORMAT_MTL)
static Material LoadMTL(const char *fileName);  // Load MTL material data
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Draw a line in 3D world space
void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex3f(startPos.x, startPos.y, startPos.z);
        rlVertex3f(endPos.x, endPos.y, endPos.z);
    rlEnd();
}

// Draw a circle in 3D world space
void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color)
{
    rlPushMatrix();
        rlTranslatef(center.x, center.y, center.z);
        rlRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

        rlBegin(RL_LINES);
            for (int i = 0; i < 360; i += 10)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);

                rlVertex3f(sinf(DEG2RAD*i)*radius, cosf(DEG2RAD*i)*radius, 0.0f);
                rlVertex3f(sinf(DEG2RAD*(i + 10))*radius, cosf(DEG2RAD*(i + 10))*radius, 0.0f);
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
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        rlTranslatef(position.x, position.y, position.z);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(1.0f, 1.0f, 1.0f);   // NOTE: Vertices are directly scaled on definition

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front face
            rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left

            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right

            // Back face
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right

            rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left

            // Top face
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right

            rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right

            // Bottom face
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Left
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
            rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left

            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Right
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Left

            // Right face
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left

            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left

            // Left face
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right

            rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right
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
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        //rlTranslatef(2.0f, 0.0f, 0.0f);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(2.0f, 2.0f, 2.0f);

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            // Front Face
            rlNormal3f(0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
            // Back Face
            rlNormal3f(0.0f, 0.0f, - 1.0f);                  // Normal Pointing Away From Viewer
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            // Top Face
            rlNormal3f(0.0f, 1.0f, 0.0f);                  // Normal Pointing Up
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            // Bottom Face
            rlNormal3f(0.0f, - 1.0f, 0.0f);                  // Normal Pointing Down
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            // Right face
            rlNormal3f(1.0f, 0.0f, 0.0f);                  // Normal Pointing Right
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            // Left Face
            rlNormal3f( - 1.0f, 0.0f, 0.0f);                  // Normal Pointing Left
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
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
        // NOTE: Transformation is applied in inverse order (scale -> translate)
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(radius, radius, radius);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < (rings + 2); i++)
            {
                for (int j = 0; j < slices; j++)
                {
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*(j*360/slices)));

                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i)))*sinf(DEG2RAD*((j+1)*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i)))*cosf(DEG2RAD*((j+1)*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));
                }
            }
        rlEnd();
    rlPopMatrix();
}

// Draw sphere wires
void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color)
{
    rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> translate)
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(radius, radius, radius);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < (rings + 2); i++)
            {
                for (int j = 0; j < slices; j++)
                {
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));

                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*(j*360/slices)));

                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
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
                    rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom); //Bottom Left
                    rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom); //Bottom Right
                    rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop); //Top Right

                    rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop); //Top Left
                    rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom); //Bottom Left
                    rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop); //Top Right
                }

                // Draw Cap --------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(0, height, 0);
                    rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);
                    rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop);
                }
            }
            else
            {
                // Draw Cone -------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(0, height, 0);
                    rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
                    rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);
                }
            }

            // Draw Base -----------------------------------------------------------------------------------------
            for (int i = 0; i < 360; i += 360/sides)
            {
                rlVertex3f(0, 0, 0);
                rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);
                rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
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
                rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
                rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);

                rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);
                rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop);

                rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop);
                rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);

                rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);
                rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
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
    int halfSlices = slices/2;

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

// Load model from files (mesh and material)
Model LoadModel(const char *fileName)
{
    Model model = { 0 };

    model.mesh = LoadMesh(fileName);
    model.transform = MatrixIdentity();
    model.material = LoadMaterialDefault();

    return model;
}

// Load model from generated mesh
// WARNING: A shallow copy of mesh is generated, passed by value,
// as long as struct contains pointers to data and some values, we get a copy
// of mesh pointing to same data as original version... be careful!
Model LoadModelFromMesh(Mesh mesh)
{
    Model model = { 0 };
    
    model.mesh = mesh;
    model.transform = MatrixIdentity();
    model.material = LoadMaterialDefault();

    return model;
}

// Unload model from memory (RAM and/or VRAM)
void UnloadModel(Model model)
{
    UnloadMesh(&model.mesh);
    UnloadMaterial(model.material);

    TraceLog(LOG_INFO, "Unloaded model data (mesh and material) from RAM and VRAM");
}

// Load mesh from file
// NOTE: Mesh data loaded in CPU and GPU
Mesh LoadMesh(const char *fileName)
{
    Mesh mesh = { 0 };

#if defined(SUPPORT_FILEFORMAT_OBJ)
    if (IsFileExtension(fileName, ".obj")) mesh = LoadOBJ(fileName);
#else
    TraceLog(LOG_WARNING, "[%s] Mesh fileformat not supported, it can't be loaded", fileName);
#endif

    if (mesh.vertexCount == 0) TraceLog(LOG_WARNING, "Mesh could not be loaded");
    else rlLoadMesh(&mesh, false);  // Upload vertex data to GPU (static mesh)

    // TODO: Initialize default mesh data in case loading fails, maybe a cube?

    return mesh;
}

// Unload mesh from memory (RAM and/or VRAM)
void UnloadMesh(Mesh *mesh)
{
    rlUnloadMesh(mesh);
}

#if defined(SUPPORT_MESH_GENERATION)
// Generate plane mesh (with subdivisions)
Mesh GenMeshPlane(float width, float length, int resX, int resZ)
{
    Mesh mesh = { 0 };

#define CUSTOM_MESH_GEN_PLANE
#if defined(CUSTOM_MESH_GEN_PLANE)
    resX++;
    resZ++;
    
    // Vertices definition
    int vertexCount = resX*resZ*6;  // 6 vertex by quad

    Vector3 *vertices = (Vector3 *)malloc(vertexCount*sizeof(Vector3));
    for (int z = 0; z < resZ; z++)
    {
        // [-length/2, length/2]
        float zPos = ((float)z/(resZ - 1) - 0.5f)*length;
        for (int x = 0; x < resX; x++)
        {
            // [-width/2, width/2]
            float xPos = ((float)x/(resX - 1) - 0.5f)*width;
            vertices[x + z*resX] = (Vector3){ xPos, 0.0f, zPos };
        }
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)malloc(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++) normals[n] = (Vector3){ 0.0f, 1.0f, 0.0f };   // Vector3.up;

    // TexCoords definition		
    Vector2 *texcoords = (Vector2 *)malloc(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resZ; v++)
    {
        for (int u = 0; u < resX; u++)
        {
            texcoords[u + v*resX] = (Vector2){ (float)u/(resX - 1), (float)v/(resZ - 1) };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resX - 1)*(resZ - 1);
    int *triangles = (int *)malloc(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face % (resX - 1) + (face/(resZ - 1)*resX);

        triangles[t++] = i + resX;
        triangles[t++] = i + 1;
        triangles[t++] = i;

        triangles[t++] = i + resX;	
        triangles[t++] = i + resX + 1;
        triangles[t++] = i + 1;
    }

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces*2;
    mesh.vertices = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)malloc(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)malloc(mesh.triangleCount*3*sizeof(unsigned short));
    
    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3*i] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }
    
    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2*i] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }
    
    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3*i] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }
    
    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount*3; i++) mesh.indices[i] = triangles[i];
    
    free(vertices);
    free(normals);
    free(texcoords);
    free(triangles);
    
#else       // Use par_shapes library to generate plane mesh

    par_shapes_mesh *plane = par_shapes_create_plane(resX, resZ);   // No normals/texcoords generated!!!
    par_shapes_scale(plane, width, length, 1.0f);
    par_shapes_rotate(plane, -PI/2.0f, (float[]){ 1, 0, 0 });
    par_shapes_translate(plane, -width/2, 0.0f, length/2);
    
    mesh.vertices = (float *)malloc(plane->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float *)malloc(plane->ntriangles*3*2*sizeof(float));
    mesh.normals = (float *)malloc(plane->ntriangles*3*3*sizeof(float));

    mesh.vertexCount = plane->ntriangles*3;
    mesh.triangleCount = plane->ntriangles;

    for (int k = 0; k < mesh.vertexCount; k++)
    {
        mesh.vertices[k*3] = plane->points[plane->triangles[k]*3];
        mesh.vertices[k*3 + 1] = plane->points[plane->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = plane->points[plane->triangles[k]*3 + 2];
        
        mesh.normals[k*3] = plane->normals[plane->triangles[k]*3];
        mesh.normals[k*3 + 1] = plane->normals[plane->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = plane->normals[plane->triangles[k]*3 + 2];
        
        mesh.texcoords[k*2] = plane->tcoords[plane->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = plane->tcoords[plane->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(plane);
#endif

    // Upload vertex data to GPU (static mesh)
    rlLoadMesh(&mesh, false);  

    return mesh;
}

// Generated cuboid mesh
Mesh GenMeshCube(float width, float height, float length)
{
    Mesh mesh = { 0 };

#define CUSTOM_MESH_GEN_CUBE
#if defined(CUSTOM_MESH_GEN_CUBE)
    float vertices[] = {
        -width/2, -height/2, length/2,
        width/2, -height/2, length/2,
        width/2, height/2, length/2,
        -width/2, height/2, length/2,
        -width/2, -height/2, -length/2,
        -width/2, height/2, -length/2,
        width/2, height/2, -length/2,
        width/2, -height/2, -length/2,
        -width/2, height/2, -length/2,
        -width/2, height/2, length/2,
        width/2, height/2, length/2,
        width/2, height/2, -length/2,
        -width/2, -height/2, -length/2,
        width/2, -height/2, -length/2,
        width/2, -height/2, length/2,
        -width/2, -height/2, length/2,
        width/2, -height/2, -length/2,
        width/2, height/2, -length/2,
        width/2, height/2, length/2,
        width/2, -height/2, length/2,
        -width/2, -height/2, -length/2,
        -width/2, -height/2, length/2,
        -width/2, height/2, length/2,
        -width/2, height/2, -length/2
    };
    
    float texcoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };
    
    float normals[] = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f
    };

    mesh.vertices = (float *)malloc(24*3*sizeof(float));
    memcpy(mesh.vertices, vertices, 24*3*sizeof(float));
    
    mesh.texcoords = (float *)malloc(24*2*sizeof(float));
    memcpy(mesh.texcoords, texcoords, 24*2*sizeof(float));
    
    mesh.normals = (float *)malloc(24*3*sizeof(float));
    memcpy(mesh.normals, normals, 24*3*sizeof(float));
    
    mesh.indices = (unsigned short *)malloc(36*sizeof(unsigned short));
    
    int k = 0;

    // Indices can be initialized right now
    for (int i = 0; i < 36; i+=6)
    {
        mesh.indices[i] = 4*k;
        mesh.indices[i+1] = 4*k+1;
        mesh.indices[i+2] = 4*k+2;
        mesh.indices[i+3] = 4*k;
        mesh.indices[i+4] = 4*k+2;
        mesh.indices[i+5] = 4*k+3;

        k++;
    }
    
    mesh.vertexCount = 24;
    mesh.triangleCount = 12;
    
#else               // Use par_shapes library to generate cube mesh
/*
// Platonic solids:
par_shapes_mesh* par_shapes_create_tetrahedron();       // 4 sides polyhedron (pyramid)
par_shapes_mesh* par_shapes_create_cube();              // 6 sides polyhedron (cube)
par_shapes_mesh* par_shapes_create_octahedron();        // 8 sides polyhedron (dyamond)
par_shapes_mesh* par_shapes_create_dodecahedron();      // 12 sides polyhedron
par_shapes_mesh* par_shapes_create_icosahedron();       // 20 sides polyhedron
*/
    // Platonic solid generation: cube (6 sides)
    // NOTE: No normals/texcoords generated by default
    par_shapes_mesh *cube = par_shapes_create_cube();
    cube->tcoords = PAR_MALLOC(float, 2*cube->npoints);
    for (int i = 0; i < 2*cube->npoints; i++) cube->tcoords[i] = 0.0f;    
    par_shapes_scale(cube, width, height, length);
    par_shapes_translate(cube, -width/2, 0.0f, -length/2);
    par_shapes_compute_normals(cube);
    
    mesh.vertices = (float *)malloc(cube->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float *)malloc(cube->ntriangles*3*2*sizeof(float));
    mesh.normals = (float *)malloc(cube->ntriangles*3*3*sizeof(float));

    mesh.vertexCount = cube->ntriangles*3;
    mesh.triangleCount = cube->ntriangles;

    for (int k = 0; k < mesh.vertexCount; k++)
    {
        mesh.vertices[k*3] = cube->points[cube->triangles[k]*3];
        mesh.vertices[k*3 + 1] = cube->points[cube->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = cube->points[cube->triangles[k]*3 + 2];
        
        mesh.normals[k*3] = cube->normals[cube->triangles[k]*3];
        mesh.normals[k*3 + 1] = cube->normals[cube->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = cube->normals[cube->triangles[k]*3 + 2];
        
        mesh.texcoords[k*2] = cube->tcoords[cube->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = cube->tcoords[cube->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(cube);
#endif

    // Upload vertex data to GPU (static mesh)
    rlLoadMesh(&mesh, false);  

    return mesh;
}

// Generate sphere mesh (standard sphere)
RLAPI Mesh GenMeshSphere(float radius, int rings, int slices)
{
    Mesh mesh = { 0 };

    par_shapes_mesh *sphere = par_shapes_create_parametric_sphere(slices, rings);
    par_shapes_scale(sphere, radius, radius, radius);
    // NOTE: Soft normals are computed internally 
    
    mesh.vertices = (float *)malloc(sphere->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float *)malloc(sphere->ntriangles*3*2*sizeof(float));
    mesh.normals = (float *)malloc(sphere->ntriangles*3*3*sizeof(float));

    mesh.vertexCount = sphere->ntriangles*3;
    mesh.triangleCount = sphere->ntriangles;

    for (int k = 0; k < mesh.vertexCount; k++)
    {
        mesh.vertices[k*3] = sphere->points[sphere->triangles[k]*3];
        mesh.vertices[k*3 + 1] = sphere->points[sphere->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = sphere->points[sphere->triangles[k]*3 + 2];
        
        mesh.normals[k*3] = sphere->normals[sphere->triangles[k]*3];
        mesh.normals[k*3 + 1] = sphere->normals[sphere->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = sphere->normals[sphere->triangles[k]*3 + 2];
        
        mesh.texcoords[k*2] = sphere->tcoords[sphere->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = sphere->tcoords[sphere->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(sphere);
    
    // Upload vertex data to GPU (static mesh)
    rlLoadMesh(&mesh, false);  

    return mesh;
}

// Generate hemi-sphere mesh (half sphere, no bottom cap)
RLAPI Mesh GenMeshHemiSphere(float radius, int rings, int slices)
{
    Mesh mesh = { 0 };

    par_shapes_mesh *sphere = par_shapes_create_hemisphere(slices, rings);
    par_shapes_scale(sphere, radius, radius, radius);
    // NOTE: Soft normals are computed internally 
    
    mesh.vertices = (float *)malloc(sphere->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float *)malloc(sphere->ntriangles*3*2*sizeof(float));
    mesh.normals = (float *)malloc(sphere->ntriangles*3*3*sizeof(float));

    mesh.vertexCount = sphere->ntriangles*3;
    mesh.triangleCount = sphere->ntriangles;

    for (int k = 0; k < mesh.vertexCount; k++)
    {
        mesh.vertices[k*3] = sphere->points[sphere->triangles[k]*3];
        mesh.vertices[k*3 + 1] = sphere->points[sphere->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = sphere->points[sphere->triangles[k]*3 + 2];
        
        mesh.normals[k*3] = sphere->normals[sphere->triangles[k]*3];
        mesh.normals[k*3 + 1] = sphere->normals[sphere->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = sphere->normals[sphere->triangles[k]*3 + 2];
        
        mesh.texcoords[k*2] = sphere->tcoords[sphere->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = sphere->tcoords[sphere->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(sphere);
    
    // Upload vertex data to GPU (static mesh)
    rlLoadMesh(&mesh, false);  

    return mesh;
}

// Generate cylinder mesh
Mesh GenMeshCylinder(float radius, float height, int slices)
{
    Mesh mesh = { 0 };

    // Instance a cylinder that sits on the Z=0 plane using the given tessellation
    // levels across the UV domain.  Think of "slices" like a number of pizza
    // slices, and "stacks" like a number of stacked rings.  
    // Height and radius are both 1.0, but they can easily be changed with par_shapes_scale
    par_shapes_mesh *cylinder = par_shapes_create_cylinder(slices, 8);
    par_shapes_scale(cylinder, radius, radius, height);
    par_shapes_rotate(cylinder, -PI/2.0f, (float[]){ 1, 0, 0 });

    // Generate an orientable disk shape (top cap)
    par_shapes_mesh *capTop = par_shapes_create_disk(radius, slices, (float[]){ 0, 0, 0 }, (float[]){ 0, 0, 1 });
    capTop->tcoords = PAR_MALLOC(float, 2*capTop->npoints);
    for (int i = 0; i < 2*capTop->npoints; i++) capTop->tcoords[i] = 0.0f;
    par_shapes_rotate(capTop, -PI/2.0f, (float[]){ 1, 0, 0 });
    par_shapes_translate(capTop, 0, height, 0);
    
    // Generate an orientable disk shape (bottom cap)
    par_shapes_mesh *capBottom = par_shapes_create_disk(radius, slices, (float[]){ 0, 0, 0 }, (float[]){ 0, 0, -1 });
    capBottom->tcoords = PAR_MALLOC(float, 2*capBottom->npoints);
    for (int i = 0; i < 2*capBottom->npoints; i++) capBottom->tcoords[i] = 0.95f;
    par_shapes_rotate(capBottom, PI/2.0f, (float[]){ 1, 0, 0 });
    
    par_shapes_merge_and_free(cylinder, capTop);
    par_shapes_merge_and_free(cylinder, capBottom);
    
    mesh.vertices = (float *)malloc(cylinder->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float *)malloc(cylinder->ntriangles*3*2*sizeof(float));
    mesh.normals = (float *)malloc(cylinder->ntriangles*3*3*sizeof(float));

    mesh.vertexCount = cylinder->ntriangles*3;
    mesh.triangleCount = cylinder->ntriangles;

    for (int k = 0; k < mesh.vertexCount; k++)
    {
        mesh.vertices[k*3] = cylinder->points[cylinder->triangles[k]*3];
        mesh.vertices[k*3 + 1] = cylinder->points[cylinder->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = cylinder->points[cylinder->triangles[k]*3 + 2];
        
        mesh.normals[k*3] = cylinder->normals[cylinder->triangles[k]*3];
        mesh.normals[k*3 + 1] = cylinder->normals[cylinder->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = cylinder->normals[cylinder->triangles[k]*3 + 2];
        
        mesh.texcoords[k*2] = cylinder->tcoords[cylinder->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = cylinder->tcoords[cylinder->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(cylinder);
    
    // Upload vertex data to GPU (static mesh)
    rlLoadMesh(&mesh, false);  

    return mesh;
}

// Generate torus mesh
Mesh GenMeshTorus(float radius, float size, int radSeg, int sides)
{
    Mesh mesh = { 0 };

    if (radius > 1.0f) radius = 1.0f;
    else if (radius < 0.1f) radius = 0.1f;
    
    // Create a donut that sits on the Z=0 plane with the specified inner radius
    // The outer radius can be controlled with par_shapes_scale
    par_shapes_mesh *torus = par_shapes_create_torus(radSeg, sides, radius);
    par_shapes_scale(torus, size/2, size/2, size/2);

    mesh.vertices = (float *)malloc(torus->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float *)malloc(torus->ntriangles*3*2*sizeof(float));
    mesh.normals = (float *)malloc(torus->ntriangles*3*3*sizeof(float));

    mesh.vertexCount = torus->ntriangles*3;
    mesh.triangleCount = torus->ntriangles;

    for (int k = 0; k < mesh.vertexCount; k++)
    {
        mesh.vertices[k*3] = torus->points[torus->triangles[k]*3];
        mesh.vertices[k*3 + 1] = torus->points[torus->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = torus->points[torus->triangles[k]*3 + 2];
        
        mesh.normals[k*3] = torus->normals[torus->triangles[k]*3];
        mesh.normals[k*3 + 1] = torus->normals[torus->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = torus->normals[torus->triangles[k]*3 + 2];
        
        mesh.texcoords[k*2] = torus->tcoords[torus->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = torus->tcoords[torus->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(torus);
    
    // Upload vertex data to GPU (static mesh)
    rlLoadMesh(&mesh, false);  

    return mesh;
}

// Generate trefoil knot mesh
Mesh GenMeshKnot(float radius, float size, int radSeg, int sides)
{
    Mesh mesh = { 0 };
    
    if (radius > 3.0f) radius = 3.0f;
    else if (radius < 0.5f) radius = 0.5f;

    par_shapes_mesh *knot = par_shapes_create_trefoil_knot(radSeg, sides, radius);
    par_shapes_scale(knot, size, size, size);

    mesh.vertices = (float *)malloc(knot->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float *)malloc(knot->ntriangles*3*2*sizeof(float));
    mesh.normals = (float *)malloc(knot->ntriangles*3*3*sizeof(float));

    mesh.vertexCount = knot->ntriangles*3;
    mesh.triangleCount = knot->ntriangles;

    for (int k = 0; k < mesh.vertexCount; k++)
    {
        mesh.vertices[k*3] = knot->points[knot->triangles[k]*3];
        mesh.vertices[k*3 + 1] = knot->points[knot->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = knot->points[knot->triangles[k]*3 + 2];
        
        mesh.normals[k*3] = knot->normals[knot->triangles[k]*3];
        mesh.normals[k*3 + 1] = knot->normals[knot->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = knot->normals[knot->triangles[k]*3 + 2];
        
        mesh.texcoords[k*2] = knot->tcoords[knot->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = knot->tcoords[knot->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(knot);
    
    // Upload vertex data to GPU (static mesh)
    rlLoadMesh(&mesh, false);  

    return mesh;
}

// Generate a mesh from heightmap
// NOTE: Vertex data is uploaded to GPU
Mesh GenMeshHeightmap(Image heightmap, Vector3 size)
{
    #define GRAY_VALUE(c) ((c.r+c.g+c.b)/3)

    Mesh mesh = { 0 };

    int mapX = heightmap.width;
    int mapZ = heightmap.height;

    Color *pixels = GetImageData(heightmap);

    // NOTE: One vertex per pixel
    int triangleCount = (mapX-1)*(mapZ-1)*2;    // One quad every four pixels

    mesh.vertexCount = triangleCount*3;

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
    
    // Upload vertex data to GPU (static mesh)
    rlLoadMesh(&mesh, false);

    return mesh;
}

// Generate a cubes mesh from pixel data
// NOTE: Vertex data is uploaded to GPU
Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize)
{
    Mesh mesh = { 0 };

    Color *cubicmapPixels = GetImageData(cubicmap);

    int mapWidth = cubicmap.width;
    int mapHeight = cubicmap.height;

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

    for (int z = 0; z < mapHeight; ++z)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            // Define the 8 vertex of the cube, we will combine them accordingly later...
            Vector3 v1 = { w*(x - 0.5f), h2, h*(z - 0.5f) };
            Vector3 v2 = { w*(x - 0.5f), h2, h*(z + 0.5f) };
            Vector3 v3 = { w*(x + 0.5f), h2, h*(z + 0.5f) };
            Vector3 v4 = { w*(x + 0.5f), h2, h*(z - 0.5f) };
            Vector3 v5 = { w*(x + 0.5f), 0, h*(z - 0.5f) };
            Vector3 v6 = { w*(x - 0.5f), 0, h*(z - 0.5f) };
            Vector3 v7 = { w*(x - 0.5f), 0, h*(z + 0.5f) };
            Vector3 v8 = { w*(x + 0.5f), 0, h*(z + 0.5f) };

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
    
    // Upload vertex data to GPU (static mesh)
    rlLoadMesh(&mesh, false);  

    return mesh;
}
#endif      // SUPPORT_MESH_GENERATION

// Load material data (from file)
Material LoadMaterial(const char *fileName)
{
    Material material = { 0 };

#if defined(SUPPORT_FILEFORMAT_MTL)
    if (IsFileExtension(fileName, ".mtl")) material = LoadMTL(fileName);
#else
    TraceLog(LOG_WARNING, "[%s] Material fileformat not supported, it can't be loaded", fileName);
#endif

    // Our material uses the default shader (DIFFUSE, SPECULAR, NORMAL)
    material.shader = GetShaderDefault();

    return material;
}

// Load default material (Supports: DIFFUSE, SPECULAR, NORMAL maps)
Material LoadMaterialDefault(void)
{
    Material material = { 0 };

    material.shader = GetShaderDefault();
    material.maps[MAP_DIFFUSE].texture = GetTextureDefault();   // White texture (1x1 pixel)
    //material.maps[MAP_NORMAL].texture;         // NOTE: By default, not set
    //material.maps[MAP_SPECULAR].texture;       // NOTE: By default, not set

    material.maps[MAP_DIFFUSE].color = WHITE;    // Diffuse color
    material.maps[MAP_SPECULAR].color = WHITE;   // Specular color

    return material;
}

// Unload material from memory
void UnloadMaterial(Material material)
{
    // Unload material shader
    UnloadShader(material.shader);

    // Unload loaded texture maps
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        // NOTE: We already check for (tex.id > 0) inside function
        rlDeleteTextures(material.maps[i].texture.id); 
    }
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
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle*DEG2RAD);
    Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);
    
    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

    // Combine model transformation matrix (model.transform) with matrix generated by function parameters (matTransform)
    //Matrix matModel = MatrixMultiply(model.transform, matTransform);    // Transform to world-space coordinates

    model.transform = MatrixMultiply(model.transform, matTransform);
    model.material.maps[MAP_DIFFUSE].color = tint;       // TODO: Multiply tint color by diffuse color?

    rlDrawMesh(model.mesh, model.material, model.transform);
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

    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    Vector3 right = { matView.m0, matView.m4, matView.m8 };
    //Vector3 up = { matView.m1, matView.m5, matView.m9 };

    // NOTE: Billboard locked on axis-Y
    Vector3 up = { 0.0f, 1.0f, 0.0f };
/*
    a-------b
    |       |
    |   *   |
    |       |
    d-------c
*/
    Vector3Scale(&right, sizeRatio.x/2);
    Vector3Scale(&up, sizeRatio.y/2);

    Vector3 p1 = Vector3Add(right, up);
    Vector3 p2 = Vector3Subtract(right, up);

    Vector3 a = Vector3Subtract(center, p2);
    Vector3 b = Vector3Add(center, p1);
    Vector3 c = Vector3Add(center, p2);
    Vector3 d = Vector3Subtract(center, p1);

    rlEnableTexture(texture.id);

    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        // Bottom-left corner for texture and quad
        rlTexCoord2f((float)sourceRec.x/texture.width, (float)sourceRec.y/texture.height);
        rlVertex3f(a.x, a.y, a.z);

        // Top-left corner for texture and quad
        rlTexCoord2f((float)sourceRec.x/texture.width, (float)(sourceRec.y + sourceRec.height)/texture.height);
        rlVertex3f(d.x, d.y, d.z);

        // Top-right corner for texture and quad
        rlTexCoord2f((float)(sourceRec.x + sourceRec.width)/texture.width, (float)(sourceRec.y + sourceRec.height)/texture.height);
        rlVertex3f(c.x, c.y, c.z);

        // Bottom-right corner for texture and quad
        rlTexCoord2f((float)(sourceRec.x + sourceRec.width)/texture.width, (float)sourceRec.y/texture.height);
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

    float distance = sqrtf(dx*dx + dy*dy + dz*dz);  // Distance between centers

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

    if (centerSphere.x < box.min.x) dmin += powf(centerSphere.x - box.min.x, 2);
    else if (centerSphere.x > box.max.x) dmin += powf(centerSphere.x - box.max.x, 2);

    if (centerSphere.y < box.min.y) dmin += powf(centerSphere.y - box.min.y, 2);
    else if (centerSphere.y > box.max.y) dmin += powf(centerSphere.y - box.max.y, 2);

    if (centerSphere.z < box.min.z) dmin += powf(centerSphere.z - box.min.z, 2);
    else if (centerSphere.z > box.max.z) dmin += powf(centerSphere.z - box.max.z, 2);

    if (dmin <= (radiusSphere*radiusSphere)) collision = true;

    return collision;
}

// Detect collision between ray and sphere
bool CheckCollisionRaySphere(Ray ray, Vector3 spherePosition, float sphereRadius)
{
    bool collision = false;

    Vector3 raySpherePos = Vector3Subtract(spherePosition, ray.position);
    float distance = Vector3Length(raySpherePos);
    float vector = Vector3DotProduct(raySpherePos, ray.direction);
    float d = sphereRadius*sphereRadius - (distance*distance - vector*vector);

    if (d >= 0.0f) collision = true;

    return collision;
}

// Detect collision between ray and sphere with extended parameters and collision point detection
bool CheckCollisionRaySphereEx(Ray ray, Vector3 spherePosition, float sphereRadius, Vector3 *collisionPoint)
{
    bool collision = false;

    Vector3 raySpherePos = Vector3Subtract(spherePosition, ray.position);
    float distance = Vector3Length(raySpherePos);
    float vector = Vector3DotProduct(raySpherePos, ray.direction);
    float d = sphereRadius*sphereRadius - (distance*distance - vector*vector);

    if (d >= 0.0f) collision = true;

    // Calculate collision point
    Vector3 offset = ray.direction;
    float collisionDistance = 0;

    // Check if ray origin is inside the sphere to calculate the correct collision point
    if (distance < sphereRadius) collisionDistance = vector + sqrtf(d);
    else collisionDistance = vector - sqrtf(d);

    Vector3Scale(&offset, collisionDistance);
    Vector3 cPoint = Vector3Add(ray.position, offset);

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
    t[6] = (float)fmax(fmax(fmin(t[0], t[1]), fmin(t[2], t[3])), fmin(t[4], t[5]));
    t[7] = (float)fmin(fmin(fmax(t[0], t[1]), fmax(t[2], t[3])), fmax(t[4], t[5]));

    collision = !(t[7] < 0 || t[6] > t[7]);

    return collision;
}

// Get collision info between ray and mesh
RayHitInfo GetCollisionRayMesh(Ray ray, Mesh *mesh)
{
    RayHitInfo result = { 0 };

    // If mesh doesn't have vertex data on CPU, can't test it.
    if (!mesh->vertices) return result;

    // mesh->triangleCount may not be set, vertexCount is more reliable
    int triangleCount = mesh->vertexCount/3;

    // Test against all triangles in mesh
    for (int i = 0; i < triangleCount; i++)
    {
        Vector3 a, b, c;
        Vector3 *vertdata = (Vector3 *)mesh->vertices;

        if (mesh->indices)
        {
            a = vertdata[mesh->indices[i*3 + 0]];
            b = vertdata[mesh->indices[i*3 + 1]];
            c = vertdata[mesh->indices[i*3 + 2]];
        }
        else
        {
            a = vertdata[i*3 + 0];
            b = vertdata[i*3 + 1];
            c = vertdata[i*3 + 2];
        }

        RayHitInfo triHitInfo = GetCollisionRayTriangle(ray, a, b, c);

        if (triHitInfo.hit)
        {
            // Save the closest hit triangle
            if ((!result.hit) || (result.distance > triHitInfo.distance)) result = triHitInfo;
        }
    }

    return result;
}

// Get collision info between ray and triangle
// NOTE: Based on https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
RayHitInfo GetCollisionRayTriangle(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3)
{
    #define EPSILON 0.000001        // A small number

    Vector3 edge1, edge2;
    Vector3 p, q, tv;
    float det, invDet, u, v, t;
    RayHitInfo result = {0};

    // Find vectors for two edges sharing V1
    edge1 = Vector3Subtract(p2, p1);
    edge2 = Vector3Subtract(p3, p1);

    // Begin calculating determinant - also used to calculate u parameter
    p = Vector3CrossProduct(ray.direction, edge2);

    // If determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle
    det = Vector3DotProduct(edge1, p);

    // Avoid culling!
    if ((det > -EPSILON) && (det < EPSILON)) return result;

    invDet = 1.0f/det;

    // Calculate distance from V1 to ray origin
    tv = Vector3Subtract(ray.position, p1);

    // Calculate u parameter and test bound
    u = Vector3DotProduct(tv, p)*invDet;

    // The intersection lies outside of the triangle
    if ((u < 0.0f) || (u > 1.0f)) return result;

    // Prepare to test v parameter
    q = Vector3CrossProduct(tv, edge1);

    // Calculate V parameter and test bound
    v = Vector3DotProduct(ray.direction, q)*invDet;

    // The intersection lies outside of the triangle
    if ((v < 0.0f) || ((u + v) > 1.0f)) return result;

    t = Vector3DotProduct(edge2, q)*invDet;

    if (t > EPSILON)
    {
        // Ray hit, get hit point and normal
        result.hit = true;
        result.distance = t;
        result.hit = true;
        result.normal = Vector3CrossProduct(edge1, edge2);
        Vector3Normalize(&result.normal);
        Vector3 rayDir = ray.direction;
        Vector3Scale(&rayDir, t);
        result.position = Vector3Add(ray.position, rayDir);
    }

    return result;
}

// Get collision info between ray and ground plane (Y-normal plane)
RayHitInfo GetCollisionRayGround(Ray ray, float groundHeight)
{
    #define EPSILON 0.000001        // A small number

    RayHitInfo result = { 0 };

    if (fabsf(ray.direction.y) > EPSILON)
    {
        float t = (ray.position.y - groundHeight)/-ray.direction.y;

        if (t >= 0.0)
        {
            Vector3 rayDir = ray.direction;
            Vector3Scale(&rayDir, t);
            result.hit = true;
            result.distance = t;
            result.normal = (Vector3){ 0.0, 1.0, 0.0 };
            result.position = Vector3Add(ray.position, rayDir);
        }
    }

    return result;
}

// Calculate mesh bounding box limits
// NOTE: minVertex and maxVertex should be transformed by model transform matrix
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
            minVertex = Vector3Min(minVertex, (Vector3){ mesh.vertices[i*3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2] });
            maxVertex = Vector3Max(maxVertex, (Vector3){ mesh.vertices[i*3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2] });
        }
    }

    // Create the bounding box
    BoundingBox box;
    box.min = minVertex;
    box.max = maxVertex;

    return box;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

#if defined(SUPPORT_FILEFORMAT_OBJ)
// Load OBJ mesh data
static Mesh LoadOBJ(const char *fileName)
{
    Mesh mesh = { 0 };

    char dataType;
    char comments[200];

    int vertexCount = 0;
    int normalCount = 0;
    int texcoordCount = 0;
    int triangleCount = 0;

    FILE *objFile;

    objFile = fopen(fileName, "rt");

    if (objFile == NULL)
    {
        TraceLog(LOG_WARNING, "[%s] OBJ file could not be opened", fileName);
        return mesh;
    }

    // First reading pass: Get vertexCount, normalCount, texcoordCount, triangleCount
    // NOTE: vertex, texcoords and normals could be optimized (to be used indexed on faces definition)
    // NOTE: faces MUST be defined as TRIANGLES (3 vertex per face)
    while (!feof(objFile))
    {
        dataType = '\0';
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
                    texcoordCount++;
                    fgets(comments, 200, objFile);
                }
                else if (dataType == 'n')    // Read normals
                {
                    normalCount++;
                    fgets(comments, 200, objFile);
                }
                else    // Read vertex
                {
                    vertexCount++;
                    fgets(comments, 200, objFile);
                }
            } break;
            case 'f':
            {
                triangleCount++;
                fgets(comments, 200, objFile);
            } break;
            default: break;
        }
    }

    TraceLog(LOG_DEBUG, "[%s] Model vertices: %i", fileName, vertexCount);
    TraceLog(LOG_DEBUG, "[%s] Model texcoords: %i", fileName, texcoordCount);
    TraceLog(LOG_DEBUG, "[%s] Model normals: %i", fileName, normalCount);
    TraceLog(LOG_DEBUG, "[%s] Model triangles: %i", fileName, triangleCount);

    // Once we know the number of vertices to store, we create required arrays
    Vector3 *midVertices = (Vector3 *)malloc(vertexCount*sizeof(Vector3));
    Vector3 *midNormals = NULL;
    if (normalCount > 0) midNormals = (Vector3 *)malloc(normalCount*sizeof(Vector3));
    Vector2 *midTexCoords = NULL;
    if (texcoordCount > 0) midTexCoords = (Vector2 *)malloc(texcoordCount*sizeof(Vector2));

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
                    fscanf(objFile, "%f %f%*[^\n]s\n", &midTexCoords[countTexCoords].x, &midTexCoords[countTexCoords].y);
                    countTexCoords++;

                    fscanf(objFile, "%c", &dataType);
                }
                else if (dataType == 'n')    // Read normals
                {
                    fscanf(objFile, "%f %f %f", &midNormals[countNormals].x, &midNormals[countNormals].y, &midNormals[countNormals].z);
                    countNormals++;

                    fscanf(objFile, "%c", &dataType);
                }
                else    // Read vertex
                {
                    fscanf(objFile, "%f %f %f", &midVertices[countVertex].x, &midVertices[countVertex].y, &midVertices[countVertex].z);
                    countVertex++;

                    fscanf(objFile, "%c", &dataType);
                }
            } break;
            default: break;
        }
    }

    // At this point all vertex data (v, vt, vn) has been gathered on midVertices, midTexCoords, midNormals
    // Now we can organize that data into our Mesh struct

    mesh.vertexCount = triangleCount*3;

    // Additional arrays to store vertex data as floats
    mesh.vertices = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)malloc(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)malloc(mesh.vertexCount*3*sizeof(float));
    mesh.colors = NULL;

    int vCounter = 0;       // Used to count vertices float by float
    int tcCounter = 0;      // Used to count texcoords float by float
    int nCounter = 0;       // Used to count normals float by float

    int vCount[3], vtCount[3], vnCount[3];    // Used to store triangle indices for v, vt, vn

    rewind(objFile);        // Return to the beginning of the file, to read again

    if (normalCount == 0) TraceLog(LOG_INFO, "[%s] No normals data on OBJ, normals will be generated from faces data", fileName);

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

                if ((normalCount == 0) && (texcoordCount == 0)) fscanf(objFile, "%i %i %i", &vCount[0], &vCount[1], &vCount[2]);
                else if (normalCount == 0) fscanf(objFile, "%i/%i %i/%i %i/%i", &vCount[0], &vtCount[0], &vCount[1], &vtCount[1], &vCount[2], &vtCount[2]);
                else if (texcoordCount == 0) fscanf(objFile, "%i//%i %i//%i %i//%i", &vCount[0], &vnCount[0], &vCount[1], &vnCount[1], &vCount[2], &vnCount[2]);
                else fscanf(objFile, "%i/%i/%i %i/%i/%i %i/%i/%i", &vCount[0], &vtCount[0], &vnCount[0], &vCount[1], &vtCount[1], &vnCount[1], &vCount[2], &vtCount[2], &vnCount[2]);

                mesh.vertices[vCounter] = midVertices[vCount[0]-1].x;
                mesh.vertices[vCounter + 1] = midVertices[vCount[0]-1].y;
                mesh.vertices[vCounter + 2] = midVertices[vCount[0]-1].z;
                vCounter += 3;
                mesh.vertices[vCounter] = midVertices[vCount[1]-1].x;
                mesh.vertices[vCounter + 1] = midVertices[vCount[1]-1].y;
                mesh.vertices[vCounter + 2] = midVertices[vCount[1]-1].z;
                vCounter += 3;
                mesh.vertices[vCounter] = midVertices[vCount[2]-1].x;
                mesh.vertices[vCounter + 1] = midVertices[vCount[2]-1].y;
                mesh.vertices[vCounter + 2] = midVertices[vCount[2]-1].z;
                vCounter += 3;

                if (normalCount > 0)
                {
                    mesh.normals[nCounter] = midNormals[vnCount[0]-1].x;
                    mesh.normals[nCounter + 1] = midNormals[vnCount[0]-1].y;
                    mesh.normals[nCounter + 2] = midNormals[vnCount[0]-1].z;
                    nCounter += 3;
                    mesh.normals[nCounter] = midNormals[vnCount[1]-1].x;
                    mesh.normals[nCounter + 1] = midNormals[vnCount[1]-1].y;
                    mesh.normals[nCounter + 2] = midNormals[vnCount[1]-1].z;
                    nCounter += 3;
                    mesh.normals[nCounter] = midNormals[vnCount[2]-1].x;
                    mesh.normals[nCounter + 1] = midNormals[vnCount[2]-1].y;
                    mesh.normals[nCounter + 2] = midNormals[vnCount[2]-1].z;
                    nCounter += 3;
                }
                else
                {
                    // If normals not defined, they are calculated from the 3 vertices [N = (V2 - V1) x (V3 - V1)]
                    Vector3 norm = Vector3CrossProduct(Vector3Subtract(midVertices[vCount[1]-1], midVertices[vCount[0]-1]), Vector3Subtract(midVertices[vCount[2]-1], midVertices[vCount[0]-1]));
                    Vector3Normalize(&norm);

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

                if (texcoordCount > 0)
                {
                    // NOTE: If using negative texture coordinates with a texture filter of GL_CLAMP_TO_EDGE doesn't work!
                    // NOTE: Texture coordinates are Y flipped upside-down
                    mesh.texcoords[tcCounter] = midTexCoords[vtCount[0]-1].x;
                    mesh.texcoords[tcCounter + 1] = 1.0f - midTexCoords[vtCount[0]-1].y;
                    tcCounter += 2;
                    mesh.texcoords[tcCounter] = midTexCoords[vtCount[1]-1].x;
                    mesh.texcoords[tcCounter + 1] = 1.0f - midTexCoords[vtCount[1]-1].y;
                    tcCounter += 2;
                    mesh.texcoords[tcCounter] = midTexCoords[vtCount[2]-1].x;
                    mesh.texcoords[tcCounter + 1] = 1.0f - midTexCoords[vtCount[2]-1].y;
                    tcCounter += 2;
                }
            } break;
            default: break;
        }
    }

    fclose(objFile);

    // Security check, just in case no normals or no texcoords defined in OBJ
    if (texcoordCount == 0) for (int i = 0; i < (2*mesh.vertexCount); i++) mesh.texcoords[i] = 0.0f;
    else
    {
        // Attempt to calculate mesh tangents and binormals using positions and texture coordinates
        mesh.tangents = (float *)malloc(mesh.vertexCount*3*sizeof(float));
        // mesh.binormals = (float *)malloc(mesh.vertexCount*3*sizeof(float));

        int vCount = 0;
        int uvCount = 0;
        while (vCount < mesh.vertexCount*3)
        {
            // Calculate mesh vertex positions as Vector3
            Vector3 v0 = { mesh.vertices[vCount], mesh.vertices[vCount + 1], mesh.vertices[vCount + 2] };
            Vector3 v1 = { mesh.vertices[vCount + 3], mesh.vertices[vCount + 4], mesh.vertices[vCount + 5] };
            Vector3 v2 = { mesh.vertices[vCount + 6], mesh.vertices[vCount + 7], mesh.vertices[vCount + 8] };

            // Calculate mesh texture coordinates as Vector2
            Vector2 uv0 = { mesh.texcoords[uvCount + 0], mesh.texcoords[uvCount + 1] };
            Vector2 uv1 = { mesh.texcoords[uvCount + 2], mesh.texcoords[uvCount + 3] };
            Vector2 uv2 = { mesh.texcoords[uvCount + 4], mesh.texcoords[uvCount + 5] };

            // Calculate edges of the triangle (position delta)
            Vector3 deltaPos1 = Vector3Subtract(v1, v0);
            Vector3 deltaPos2 = Vector3Subtract(v2, v0);

            // UV delta
            Vector2 deltaUV1 = { uv1.x - uv0.x, uv1.y - uv0.y };
            Vector2 deltaUV2 = { uv2.x - uv0.x, uv2.y - uv0.y };

            float r = 1.0f/(deltaUV1.x*deltaUV2.y - deltaUV1.y*deltaUV2.x);
            Vector3 t1 = { deltaPos1.x*deltaUV2.y, deltaPos1.y*deltaUV2.y, deltaPos1.z*deltaUV2.y };
            Vector3 t2 = { deltaPos2.x*deltaUV1.y, deltaPos2.y*deltaUV1.y, deltaPos2.z*deltaUV1.y };
            // Vector3 b1 = { deltaPos2.x*deltaUV1.x, deltaPos2.y*deltaUV1.x, deltaPos2.z*deltaUV1.x };
            // Vector3 b2 = { deltaPos1.x*deltaUV2.x, deltaPos1.y*deltaUV2.x, deltaPos1.z*deltaUV2.x };

            // Calculate vertex tangent
            Vector3 tangent = Vector3Subtract(t1, t2);
            Vector3Scale(&tangent, r);

            // Apply calculated tangents data to mesh struct
            mesh.tangents[vCount + 0] = tangent.x;
            mesh.tangents[vCount + 1] = tangent.y;
            mesh.tangents[vCount + 2] = tangent.z;
            mesh.tangents[vCount + 3] = tangent.x;
            mesh.tangents[vCount + 4] = tangent.y;
            mesh.tangents[vCount + 5] = tangent.z;
            mesh.tangents[vCount + 6] = tangent.x;
            mesh.tangents[vCount + 7] = tangent.y;
            mesh.tangents[vCount + 8] = tangent.z;

            // TODO: add binormals to mesh struct and assign buffers id and locations properly
            /* // Calculate vertex binormal
            Vector3 binormal = Vector3Subtract(b1, b2);
            Vector3Scale(&binormal, r);

            // Apply calculated binormals data to mesh struct
            mesh.binormals[vCount + 0] = binormal.x;
            mesh.binormals[vCount + 1] = binormal.y;
            mesh.binormals[vCount + 2] = binormal.z;
            mesh.binormals[vCount + 3] = binormal.x;
            mesh.binormals[vCount + 4] = binormal.y;
            mesh.binormals[vCount + 5] = binormal.z;
            mesh.binormals[vCount + 6] = binormal.x;
            mesh.binormals[vCount + 7] = binormal.y;
            mesh.binormals[vCount + 8] = binormal.z; */

            // Update vertex position and texture coordinates counters
            vCount += 9;
            uvCount += 6;
        }
    }

    // Now we can free temp mid* arrays
    free(midVertices);
    free(midNormals);
    free(midTexCoords);

    // NOTE: At this point we have all vertex, texcoord, normal data for the model in mesh struct
    TraceLog(LOG_INFO, "[%s] Model loaded successfully in RAM (CPU)", fileName);

    return mesh;
}
#endif

#if defined(SUPPORT_FILEFORMAT_MTL)
// Load MTL material data (specs: http://paulbourke.net/dataformats/mtl/)
// NOTE: Texture map parameters are not supported
static Material LoadMTL(const char *fileName)
{
    #define MAX_BUFFER_SIZE     128

    Material material = { 0 };

    char buffer[MAX_BUFFER_SIZE];
    Vector3 color = { 1.0f, 1.0f, 1.0f };
    char mapFileName[128];
    int result = 0;

    FILE *mtlFile;

    mtlFile = fopen(fileName, "rt");

    if (mtlFile == NULL)
    {
        TraceLog(LOG_WARNING, "[%s] MTL file could not be opened", fileName);
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

                TraceLog(LOG_INFO, "[%s] Loading material...", mapFileName);
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
                        // TODO: Support ambient color
                        //material.colAmbient.r = (unsigned char)(color.x*255);
                        //material.colAmbient.g = (unsigned char)(color.y*255);
                        //material.colAmbient.b = (unsigned char)(color.z*255);
                    } break;
                    case 'd':   // Kd float float float     Diffuse color (RGB)
                    {
                        sscanf(buffer, "Kd %f %f %f", &color.x, &color.y, &color.z);
                        material.maps[MAP_DIFFUSE].color.r = (unsigned char)(color.x*255);
                        material.maps[MAP_DIFFUSE].color.g = (unsigned char)(color.y*255);
                        material.maps[MAP_DIFFUSE].color.b = (unsigned char)(color.z*255);
                    } break;
                    case 's':   // Ks float float float     Specular color (RGB)
                    {
                        sscanf(buffer, "Ks %f %f %f", &color.x, &color.y, &color.z);
                        material.maps[MAP_SPECULAR].color.r = (unsigned char)(color.x*255);
                        material.maps[MAP_SPECULAR].color.g = (unsigned char)(color.y*255);
                        material.maps[MAP_SPECULAR].color.b = (unsigned char)(color.z*255);
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

                    //material.params[PARAM_GLOSSINES] = (float)shininess;
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
                            result = sscanf(buffer, "map_Kd %s", mapFileName);
                            if (result != EOF) material.maps[MAP_DIFFUSE].texture = LoadTexture(mapFileName);
                        }
                        else if (buffer[5] == 's')  // map_Ks string    Specular color texture map.
                        {
                            result = sscanf(buffer, "map_Ks %s", mapFileName);
                            if (result != EOF) material.maps[MAP_SPECULAR].texture = LoadTexture(mapFileName);
                        }
                        else if (buffer[5] == 'a')  // map_Ka string    Ambient color texture map.
                        {
                            // Not supported...
                        }
                    } break;
                    case 'B':       // map_Bump string      Bump texture map.
                    {
                        result = sscanf(buffer, "map_Bump %s", mapFileName);
                        if (result != EOF) material.maps[MAP_NORMAL].texture = LoadTexture(mapFileName);
                    } break;
                    case 'b':       // map_bump string      Bump texture map.
                    {
                        result = sscanf(buffer, "map_bump %s", mapFileName);
                        if (result != EOF) material.maps[MAP_NORMAL].texture = LoadTexture(mapFileName);
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
                    material.maps[MAP_DIFFUSE].color.a = (unsigned char)(alpha*255);
                }
                else if (buffer[1] == 'i')  // disp string  Displacement map
                {
                    // Not supported...
                }
            } break;
            case 'b':   // bump string      Bump texture map
            {
                result = sscanf(buffer, "bump %s", mapFileName);
                if (result != EOF) material.maps[MAP_NORMAL].texture = LoadTexture(mapFileName);
            } break;
            case 'T':   // Tr float         Transparency Tr (alpha). Tr is inverse of d
            {
                float ialpha = 0.0f;
                sscanf(buffer, "Tr %f", &ialpha);
                material.maps[MAP_DIFFUSE].color.a = (unsigned char)((1.0f - ialpha)*255);

            } break;
            case 'r':   // refl string      Reflection texture map
            default: break;
        }
    }

    fclose(mtlFile);

    // NOTE: At this point we have all material data
    TraceLog(LOG_INFO, "[%s] Material loaded successfully", fileName);

    return material;
}
#endif
