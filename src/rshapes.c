/**********************************************************************************************
*
*   rshapes - Basic functions to draw 2d shapes and check collisions
*
*   ADDITIONAL NOTES:
*       Shapes can be draw using 3 types of primitives: LINES, TRIANGLES and QUADS
*       Some functions implement two drawing options: TRIANGLES and QUADS, by default TRIANGLES
*       are used but QUADS implementation can be selected with SUPPORT_QUADS_DRAW_MODE define
*
*       Some functions define texture coordinates (rlTexCoord2f()) for the shapes and use a
*       user-provided texture with SetShapesTexture(), the purpose of this implementation
*       is allowing to reduce draw calls when combined with a texture-atlas
*
*       By default, raylib sets the default texture and rectangle at InitWindow()[rcore] to one
*       white character of default font [rtext], this way, raylib text and shapes can be drawn with
*       a single draw call and it also allows users to configure it the same way with their own fonts
*
*   CONFIGURATION:
*       #define SUPPORT_MODULE_RSHAPES      1
*           rshapes module is included in the build
*
*       #define SUPPORT_QUADS_DRAW_MODE     1
*           Use QUADS instead of TRIANGLES for drawing when possible. Lines-based shapes still use LINES
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2026 Ramon Santamaria (@raysan5)
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

#include "raylib.h"     // Declares module functions

#include "config.h"     // Defines module configuration flags

#if SUPPORT_MODULE_RSHAPES

#include "rlgl.h"       // OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2

#include <math.h>       // Required for: sinf(), asinf(), cosf(), acosf(), sqrtf(), fabsf()
#include <float.h>      // Required for: FLT_EPSILON
#include <stdlib.h>     // Required for: RL_FREE

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef SMOOTH_CIRCLE_ERROR_RATE
    // Define error rate to calculate how many segments are needed to draw a smooth circle
    // REF: https://stackoverflow.com/a/2244088
    #define SMOOTH_CIRCLE_ERROR_RATE    0.5f      // Circle error rate
#endif
#ifndef SPLINE_SEGMENT_DIVISIONS
    #define SPLINE_SEGMENT_DIVISIONS      24      // Spline segment divisions
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Not here...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static Texture2D texShapes = { 1, 1, 1, 1, 7 };                // Texture used on shapes drawing (white pixel loaded by rlgl)
static Rectangle texShapesRec = { 0.0f, 0.0f, 1.0f, 1.0f };    // Texture source rectangle used on shapes drawing

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
static float EaseCubicInOut(float t, float b, float c, float d);    // Cubic easing

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Set texture and rectangle to be used on shapes drawing
// NOTE: It can be useful when using basic shapes and one single font,
// defining a font char white rectangle would allow drawing everything in a single draw call
void SetShapesTexture(Texture2D texture, Rectangle rec)
{
    // Reset texture to default pixel if required
    // WARNING: Shapes texture should be probably better validated,
    // it can break the rendering of all shapes if misused
    if ((texture.id == 0) || (rec.width == 0) || (rec.height == 0))
    {
        texShapes = (Texture2D){ 1, 1, 1, 1, 7 };
        texShapesRec = (Rectangle){ 0.0f, 0.0f, 1.0f, 1.0f };
    }
    else
    {
        texShapes = texture;
        texShapesRec = rec;
    }
}

// Get texture that is used for shapes drawing
Texture2D GetShapesTexture(void)
{
    return texShapes;
}

// Get texture source rectangle that is used for shapes drawing
Rectangle GetShapesTextureRectangle(void)
{
    return texShapesRec;
}

// Draw a pixel
void DrawPixel(int posX, int posY, Color color)
{
    DrawPixelV((Vector2){ (float)posX, (float)posY }, color);
}

// Draw a pixel (Vector version)
void DrawPixelV(Vector2 position, Color color)
{
#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);

        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(position.x, position.y);

        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(position.x, position.y + 1);

        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(position.x + 1, position.y + 1);

        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(position.x + 1, position.y);

    rlEnd();

    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);

        rlColor4ub(color.r, color.g, color.b, color.a);

        rlVertex2f(position.x, position.y);
        rlVertex2f(position.x, position.y + 1);
        rlVertex2f(position.x + 1, position.y);

        rlVertex2f(position.x + 1, position.y);
        rlVertex2f(position.x, position.y + 1);
        rlVertex2f(position.x + 1, position.y + 1);

    rlEnd();
#endif
}

// Draw a line (using gl lines)
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f((float)startPosX, (float)startPosY);
        rlVertex2f((float)endPosX, (float)endPosY);
    rlEnd();
}

// Draw a line defining thickness
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color)
{
    Vector2 delta = { endPos.x - startPos.x, endPos.y - startPos.y };
    float length = sqrtf(delta.x*delta.x + delta.y*delta.y);

    if ((length > 0) && (thick > 0))
    {
        float scale = thick/(2*length);

        Vector2 radius = { -scale*delta.y, scale*delta.x };
        Vector2 strip[4] = {
            { startPos.x - radius.x, startPos.y - radius.y },
            { startPos.x + radius.x, startPos.y + radius.y },
            { endPos.x - radius.x, endPos.y - radius.y },
            { endPos.x + radius.x, endPos.y + radius.y }
        };

        DrawTriangleStrip(strip, 4, color);
    }
}

// Draw a line (using gl lines)
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(startPos.x, startPos.y);
        rlVertex2f(endPos.x, endPos.y);
    rlEnd();
}

// Draw lines sequuence (using gl lines)
void DrawLineStrip(const Vector2 *points, int pointCount, Color color)
{
    if (pointCount < 2) return; // Security check

    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        for (int i = 0; i < pointCount - 1; i++)
        {
            rlVertex2f(points[i].x, points[i].y);
            rlVertex2f(points[i + 1].x, points[i + 1].y);
        }
    rlEnd();
}

// Draw line using cubic-bezier spline, in-out interpolation, no control points
void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color)
{
    Vector2 previous = startPos;
    Vector2 current = { 0 };

    Vector2 points[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    for (int i = 1; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        // Cubic easing in-out
        // NOTE: Easing is calculated only for y position value
        current.y = EaseCubicInOut((float)i, startPos.y, endPos.y - startPos.y, (float)SPLINE_SEGMENT_DIVISIONS);
        current.x = previous.x + (endPos.x - startPos.x)/(float)SPLINE_SEGMENT_DIVISIONS;

        float dy = current.y - previous.y;
        float dx = current.x - previous.x;
        float size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

        if (i == 1)
        {
            points[0].x = previous.x + dy*size;
            points[0].y = previous.y - dx*size;
            points[1].x = previous.x - dy*size;
            points[1].y = previous.y + dx*size;
        }

        points[2*i + 1].x = current.x - dy*size;
        points[2*i + 1].y = current.y + dx*size;
        points[2*i].x = current.x + dy*size;
        points[2*i].y = current.y - dx*size;

        previous = current;
    }

    DrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
}

// Draw a dashed line
void DrawLineDashed(Vector2 startPos, Vector2 endPos, int dashSize, int spaceSize, Color color)
{
    // Calculate the vector and length of the line
    float dx = endPos.x - startPos.x;
    float dy = endPos.y - startPos.y;
    float lineLength = sqrtf(dx*dx + dy*dy);

    // If the line is too short for dashing or dash size is invalid, draw a solid thick line
    if ((lineLength < (dashSize + spaceSize)) || (dashSize <= 0))
    {
        DrawLineV(startPos, endPos, color);
        return;
    }

    // Calculate the normalized direction vector of the line
    float invLineLength = 1.0f/lineLength;
    float dirX = dx*invLineLength;
    float dirY = dy*invLineLength;

    Vector2 currentPos = startPos;
    float distanceTraveled = 0;

    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        while (distanceTraveled < lineLength)
        {
            // Calculate the end of the current dash
            float dashEndDist = distanceTraveled + dashSize;
            if (dashEndDist > lineLength) dashEndDist = lineLength;

            Vector2 dashEndPos = { startPos.x + dashEndDist*dirX, startPos.y + dashEndDist*dirY };

            // Draw the dash segment
            rlVertex2f(currentPos.x, currentPos.y);
            rlVertex2f(dashEndPos.x, dashEndPos.y);

            // Update the distance traveled and move the current position for the next dash
            distanceTraveled = dashEndDist + spaceSize;
            currentPos.x = startPos.x + distanceTraveled*dirX;
            currentPos.y = startPos.y + distanceTraveled*dirY;
        }
    rlEnd();
}

// Draw a triangle
// NOTE: Vertex must be provided in counter-clockwise order
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    DrawTriangleGradient(v1, v2, v3, color, color, color);
}

// Draw triangle with interpolated colors (vertex in counter-clockwise order!)
void DrawTriangleGradient(Vector2 v1, Vector2 v2, Vector2 v3, Color c1, Color c2, Color c3)
{
#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);
        rlNormal3f(0.0f, 0.0f, 1.0f);

        rlColor4ub(c1.r, c1.g, c1.b, c1.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(v1.x, v1.y);

        rlColor4ub(c2.r, c2.g, c2.b, c2.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(v2.x, v2.y);

        rlColor4ub(c3.r, c3.g, c3.b, c3.a);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(v3.x, v3.y);

        rlColor4ub(c3.r, c3.g, c3.b, c3.a);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(v3.x, v3.y);
    rlEnd();

    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);
        rlColor4ub(c1.r, c1.g, c1.b, c1.a);
        rlVertex2f(v1.x, v1.y);
        rlColor4ub(c2.r, c2.g, c2.b, c2.a);
        rlVertex2f(v2.x, v2.y);
        rlColor4ub(c3.r, c3.g, c3.b, c3.a);
        rlVertex2f(v3.x, v3.y);
    rlEnd();
#endif
}

// Draw a triangle using lines
// NOTE: Vertex must be provided in counter-clockwise order
void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(v1.x, v1.y);
        rlVertex2f(v2.x, v2.y);

        rlVertex2f(v2.x, v2.y);
        rlVertex2f(v3.x, v3.y);

        rlVertex2f(v3.x, v3.y);
        rlVertex2f(v1.x, v1.y);
    rlEnd();
}

// Draw a triangle using lines with thickness
// NOTE: Vertex must be provided in counter-clockwise order
void DrawTriangleLinesEx(Vector2 v1, Vector2 v2, Vector2 v3, float thick, Color color)
{
    /*
    A sketch to make things simpler

    The exterior points are v1-3, the interior points are v4-6, and the exterior edges are e1-3

             v1
             /\
            /v4\
           //  \\
          //    \\
      e3 //      \\ e2
        //        \\
       //          \\
      //v5        v6\\
     v2==============v3
             e1
    */

    Vector2 e1 = {v2.x - v3.x, v2.y - v3.y};
    Vector2 e2 = {v3.x - v1.x, v3.y - v1.y};
    Vector2 e3 = {v1.x - v2.x, v1.y - v2.y};

    float e1Length = sqrtf(e1.x*e1.x + e1.y*e1.y);
    float e2Length = sqrtf(e2.x*e2.x + e2.y*e2.y);
    float e3Length = sqrtf(e3.x*e3.x + e3.y*e3.y);

    float perimeter = e1Length + e2Length + e3Length;
    float semiperimeter = perimeter/2.0f;

    // The incenter of a triangle is equidistant from each edge, which is useful for drawing a nice looking outline
    Vector2 incenter = {
        (e1Length*v1.x + e2Length*v2.x + e3Length*v3.x)/perimeter,
        (e1Length*v1.y + e2Length*v2.y + e3Length*v3.y)/perimeter
    };

    // The inradius of a triangle is the radius of the biggest circle that can fit inside of said triangle
    // That circle is also centered on the incenter
    float inradius = sqrtf(((semiperimeter - e1Length)*(semiperimeter - e2Length)*(semiperimeter - e3Length))/semiperimeter);

    // The triangle (v1, v2, v3) will be scaled by this to get (v4, v5, v6)
    float scale = 1.0f - thick/inradius;

    // Just a filled-in triangle
    if (scale <= 0.0f)
    {
        DrawTriangle(v1, v2, v3, color);
        return;
    }

    // In order for the scaling to be correct, the incenter has to be at the origin (0, 0) when scaling
    Vector2 v4 = {incenter.x + (v1.x - incenter.x)*scale, incenter.y + (v1.y - incenter.y)*scale};
    Vector2 v5 = {incenter.x + (v2.x - incenter.x)*scale, incenter.y + (v2.y - incenter.y)*scale};
    Vector2 v6 = {incenter.x + (v3.x - incenter.x)*scale, incenter.y + (v3.y - incenter.y)*scale};

    // Swap the vertices so the winding order is correct
    if (thick < 0.0f)
    {
        Vector2 temp = v1;
        v1 = v4;
        v4 = temp;

        temp = v2;
        v2 = v5;
        v5 = temp;

        temp = v3;
        v3 = v6;
        v6 = temp;
    }

#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);

        rlColor4ub(color.r, color.g, color.b, color.a);

        // Edge 3
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(v1.x, v1.y);

        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(v2.x, v2.y);

        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(v5.x, v5.y);

        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(v4.x, v4.y);

        // Edge 1
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(v2.x, v2.y);

        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(v3.x, v3.y);

        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(v6.x, v6.y);

        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(v5.x, v5.y);

        // Edge 2
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(v3.x, v3.y);

        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(v1.x, v1.y);

        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(v4.x, v4.y);

        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(v6.x, v6.y);

    rlEnd();

    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);

        rlColor4ub(color.r, color.g, color.b, color.a);

        // Edge 3
        rlVertex2f(v1.x, v1.y);
        rlVertex2f(v2.x, v2.y);
        rlVertex2f(v4.x, v4.y);

        rlVertex2f(v2.x, v2.y);
        rlVertex2f(v5.x, v5.y);
        rlVertex2f(v4.x, v4.y);

        // Edge 1
        rlVertex2f(v2.x, v2.y);
        rlVertex2f(v3.x, v3.y);
        rlVertex2f(v5.x, v5.y);

        rlVertex2f(v3.x, v3.y);
        rlVertex2f(v6.x, v6.y);
        rlVertex2f(v5.x, v5.y);

        // Edge 2
        rlVertex2f(v3.x, v3.y);
        rlVertex2f(v1.x, v1.y);
        rlVertex2f(v4.x, v4.y);

        rlVertex2f(v3.x, v3.y);
        rlVertex2f(v4.x, v4.y);
        rlVertex2f(v6.x, v6.y);

    rlEnd();
#endif
}

// Draw a triangle fan defined by points
// NOTE: First vertex provided is the center, shared by all triangles
// By default, following vertex should be provided in counter-clockwise order
void DrawTriangleFan(const Vector2 *points, int pointCount, Color color)
{
    if (pointCount >= 3)
    {
        rlSetTexture(GetShapesTexture().id);
        Rectangle shapeRect = GetShapesTextureRectangle();

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 1; i < pointCount - 1; i++)
            {
                rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(points[0].x, points[0].y);

                rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(points[i].x, points[i].y);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(points[i + 1].x, points[i + 1].y);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(points[i + 1].x, points[i + 1].y);
            }
        rlEnd();
        rlSetTexture(0);
    }
}

// Draw a triangle strip defined by points
// NOTE: Every new vertex connects with previous two
void DrawTriangleStrip(const Vector2 *points, int pointCount, Color color)
{
    if (pointCount >= 3)
    {
        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 2; i < pointCount; i++)
            {
                if ((i%2) == 0)
                {
                    rlVertex2f(points[i].x, points[i].y);
                    rlVertex2f(points[i - 2].x, points[i - 2].y);
                    rlVertex2f(points[i - 1].x, points[i - 1].y);
                }
                else
                {
                    rlVertex2f(points[i].x, points[i].y);
                    rlVertex2f(points[i - 1].x, points[i - 1].y);
                    rlVertex2f(points[i - 2].x, points[i - 2].y);
                }
            }
        rlEnd();
    }
}

// Draw a color-filled rectangle
void DrawRectangle(int posX, int posY, int width, int height, Color color)
{
    DrawRectangleV((Vector2){ (float)posX, (float)posY }, (Vector2){ (float)width, (float)height }, color);
}

// Draw a color-filled rectangle (Vector version)
// NOTE: On OpenGL 3.3 and ES2 using QUADS to avoid drawing order issues
void DrawRectangleV(Vector2 position, Vector2 size, Color color)
{
    DrawRectanglePro((Rectangle){ position.x, position.y, size.x, size.y }, (Vector2){ 0.0f, 0.0f }, 0.0f, color);
}

// Draw a color-filled rectangle
void DrawRectangleRec(Rectangle rec, Color color)
{
    DrawRectanglePro(rec, (Vector2){ 0.0f, 0.0f }, 0.0f, color);
}

// Draw a color-filled rectangle with pro parameters
void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color)
{
    Vector2 topLeft = { 0 };
    Vector2 topRight = { 0 };
    Vector2 bottomLeft = { 0 };
    Vector2 bottomRight = { 0 };

    // Only calculate rotation if needed
    if (rotation == 0.0f)
    {
        float x = rec.x - origin.x;
        float y = rec.y - origin.y;
        topLeft = (Vector2){ x, y };
        topRight = (Vector2){ x + rec.width, y };
        bottomLeft = (Vector2){ x, y + rec.height };
        bottomRight = (Vector2){ x + rec.width, y + rec.height };
    }
    else
    {
        float sinRotation = sinf(rotation*DEG2RAD);
        float cosRotation = cosf(rotation*DEG2RAD);
        float x = rec.x;
        float y = rec.y;
        float dx = -origin.x;
        float dy = -origin.y;

        topLeft.x = x + dx*cosRotation - dy*sinRotation;
        topLeft.y = y + dx*sinRotation + dy*cosRotation;

        topRight.x = x + (dx + rec.width)*cosRotation - dy*sinRotation;
        topRight.y = y + (dx + rec.width)*sinRotation + dy*cosRotation;

        bottomLeft.x = x + dx*cosRotation - (dy + rec.height)*sinRotation;
        bottomLeft.y = y + dx*sinRotation + (dy + rec.height)*cosRotation;

        bottomRight.x = x + (dx + rec.width)*cosRotation - (dy + rec.height)*sinRotation;
        bottomRight.y = y + (dx + rec.width)*sinRotation + (dy + rec.height)*cosRotation;
    }

#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);

        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(topLeft.x, topLeft.y);

        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(bottomLeft.x, bottomLeft.y);

        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(bottomRight.x, bottomRight.y);

        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(topRight.x, topRight.y);

    rlEnd();

    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);

        rlColor4ub(color.r, color.g, color.b, color.a);

        rlVertex2f(topLeft.x, topLeft.y);
        rlVertex2f(bottomLeft.x, bottomLeft.y);
        rlVertex2f(topRight.x, topRight.y);

        rlVertex2f(topRight.x, topRight.y);
        rlVertex2f(bottomLeft.x, bottomLeft.y);
        rlVertex2f(bottomRight.x, bottomRight.y);

    rlEnd();
#endif
}

// Draw a vertical-gradient-filled rectangle
void DrawRectangleGradientV(int posX, int posY, int width, int height, Color top, Color bottom)
{
    DrawRectangleGradientEx((Rectangle){ (float)posX, (float)posY, (float)width, (float)height }, top, bottom, bottom, top);
}

// Draw a horizontal-gradient-filled rectangle
void DrawRectangleGradientH(int posX, int posY, int width, int height, Color left, Color right)
{
    DrawRectangleGradientEx((Rectangle){ (float)posX, (float)posY, (float)width, (float)height }, left, left, right, right);
}

// Draw a gradient-filled rectangle with custom vertex colors, counter-clockwise color order
void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4)
{
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);
        rlNormal3f(0.0f, 0.0f, 1.0f);

        // NOTE: Default raylib font character 95 is a white square
        rlColor4ub(col1.r, col1.g, col1.b, col1.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(rec.x, rec.y);

        rlColor4ub(col2.r, col2.g, col2.b, col2.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(rec.x, rec.y + rec.height);

        rlColor4ub(col3.r, col3.g, col3.b, col3.a);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(rec.x + rec.width, rec.y + rec.height);

        rlColor4ub(col4.r, col4.g, col4.b, col4.a);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(rec.x + rec.width, rec.y);
    rlEnd();

    rlSetTexture(0);
}

// Draw rectangle outline
// WARNING: All Draw*Lines() functions use RL_LINES for drawing,
// it implies flushing the current batch and changing draw mode to RL_LINES
// but it solves another issue: https://github.com/raysan5/raylib/issues/3884
void DrawRectangleLines(int posX, int posY, int width, int height, Color color)
{
    Matrix mat = rlGetMatrixTransform();
    float xOffset = 0.5f/mat.m0;
    float yOffset = 0.5f/mat.m5;

    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f((float)posX + xOffset, (float)posY + yOffset);
        rlVertex2f((float)posX + (float)width - xOffset, (float)posY + yOffset);

        rlVertex2f((float)posX + (float)width - xOffset, (float)posY + yOffset);
        rlVertex2f((float)posX + (float)width - xOffset, (float)posY + (float)height - yOffset);

        rlVertex2f((float)posX + (float)width - xOffset, (float)posY + (float)height - yOffset);
        rlVertex2f((float)posX + xOffset, (float)posY + (float)height - yOffset);

        rlVertex2f((float)posX + xOffset, (float)posY + (float)height - yOffset);
        rlVertex2f((float)posX + xOffset, (float)posY + yOffset);
    rlEnd();

/*
// Previous implementation, it has issues... but it does not require view matrix...
#if SUPPORT_QUADS_DRAW_MODE
    DrawRectangle(posX, posY, width, 1, color);
    DrawRectangle(posX + width - 1, posY + 1, 1, height - 2, color);
    DrawRectangle(posX, posY + height - 1, width, 1, color);
    DrawRectangle(posX, posY + 1, 1, height - 2, color);
#else
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f((float)posX, (float)posY);
        rlVertex2f((float)posX + (float)width, (float)posY + 1);

        rlVertex2f((float)posX + (float)width, (float)posY + 1);
        rlVertex2f((float)posX + (float)width, (float)posY + (float)height);

        rlVertex2f((float)posX + (float)width, (float)posY + (float)height);
        rlVertex2f((float)posX + 1, (float)posY + (float)height);

        rlVertex2f((float)posX + 1, (float)posY + (float)height);
        rlVertex2f((float)posX + 1, (float)posY + 1);
    rlEnd();
#endif
*/
}

// Draw rectangle outline with line thickness
void DrawRectangleLinesEx(Rectangle rec, float thick, Color color)
{
    if ((thick > rec.width/2) || (thick > rec.height/2))
    {
        if (rec.width >= rec.height) thick = rec.height/2;
        else if (rec.width <= rec.height) thick = rec.width/2;
    }

    if (thick > 0.0f)
    {
        // When rec = { x, y, 8.0f, 6.0f } and thick = 2, the following
        // four rectangles are drawn ([T]op, [B]ottom, [L]eft, [R]ight):
        //
        //   TTTTTTTT
        //   TTTTTTTT
        //   LL    RR
        //   LL    RR
        //   BBBBBBBB
        //   BBBBBBBB
        //

        Rectangle top = { rec.x, rec.y, rec.width, thick };
        Rectangle bottom = { rec.x, rec.y - thick + rec.height, rec.width, thick };
        Rectangle left = { rec.x, rec.y + thick, thick, rec.height - thick*2.0f };
        Rectangle right = { rec.x - thick + rec.width, rec.y + thick, thick, rec.height - thick*2.0f };

        DrawRectangleRec(top, color);
        DrawRectangleRec(bottom, color);
        DrawRectangleRec(left, color);
        DrawRectangleRec(right, color);
    }
    else
    {
        // When rec = { x, y, 8.0f, 6.0f } and thick = -2, the following
        // four rectangles are drawn ([T]op, [B]ottom, [L]eft, [R]ight):
        //
        //   TTTTTTTTTTTT
        //   TTTTTTTTTTTT
        //   LL        RR
        //   LL        RR
        //   LL        RR
        //   LL        RR
        //   LL        RR
        //   LL        RR
        //   BBBBBBBBBBBB
        //   BBBBBBBBBBBB
        //

        thick *= -1.0f;

        Rectangle top = { rec.x - thick, rec.y - thick, rec.width + thick*2.0f, thick };
        Rectangle bottom = { rec.x - thick, rec.y + rec.height, rec.width + thick*2.0f, thick};
        Rectangle left = { rec.x - thick, rec.y, thick, rec.height };
        Rectangle right = { rec.x + rec.width, rec.y, thick, rec.height };

        DrawRectangleRec(top, color);
        DrawRectangleRec(bottom, color);
        DrawRectangleRec(left, color);
        DrawRectangleRec(right, color);
    }
}

// Draw rectangle with rounded edges
void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color)
{
    // Not a rounded rectangle
    if (roundness <= 0.0f)
    {
        DrawRectangleRec(rec, color);
        return;
    }

    if (roundness >= 1.0f) roundness = 1.0f;

    // Calculate corner radius
    float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
    if (radius <= 0.0f) return;

    // Calculate number of segments to use for the corners
    if (segments < 4)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)ceilf((2*PI/th)/4.0f);
        if (segments <= 0) segments = 4;
    }

    float stepLength = 90.0f/(float)segments;

    /*
    Quick sketch to make sense of all of this,
    there are 9 parts to draw, also mark the 12 points used

          P0____________________P1
          /|                    |\
         /1|          2         |3\
     P7 /__|____________________|__\ P2
       |   |P8                P9|   |
       | 8 |          9         | 4 |
       | __|____________________|__ |
     P6 \  |P11              P10|  / P3
         \7|          6         |5/
          \|____________________|/
          P5                    P4
    */
    // Coordinates of the 12 points that define the rounded rect
    const Vector2 point[12] = {
        {(float)rec.x + radius, rec.y}, {(float)(rec.x + rec.width) - radius, rec.y}, { rec.x + rec.width, (float)rec.y + radius },     // PO, P1, P2
        {rec.x + rec.width, (float)(rec.y + rec.height) - radius}, {(float)(rec.x + rec.width) - radius, rec.y + rec.height},           // P3, P4
        {(float)rec.x + radius, rec.y + rec.height}, { rec.x, (float)(rec.y + rec.height) - radius}, {rec.x, (float)rec.y + radius},    // P5, P6, P7
        {(float)rec.x + radius, (float)rec.y + radius}, {(float)(rec.x + rec.width) - radius, (float)rec.y + radius},                   // P8, P9
        {(float)(rec.x + rec.width) - radius, (float)(rec.y + rec.height) - radius}, {(float)rec.x + radius, (float)(rec.y + rec.height) - radius} // P10, P11
    };

    const Vector2 centers[4] = { point[8], point[9], point[10], point[11] };
    const float angles[4] = { 180.0f, 270.0f, 0.0f, 90.0f };

#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);
        // Draw all the 4 corners: [1] Upper Left Corner, [3] Upper Right Corner, [5] Lower Right Corner, [7] Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const Vector2 center = centers[k];

            // NOTE: Every QUAD actually represents two segments
            for (int i = 0; i < segments/2; i++)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);
                rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(center.x, center.y);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength*2))*radius, center.y + sinf(DEG2RAD*(angle + stepLength*2))*radius);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius);

                rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);

                angle += (stepLength*2);
            }

            // NOTE: In case number of segments is odd, adding one last piece to the cake
            if (segments%2)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);
                rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(center.x, center.y);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius);

                rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(center.x, center.y);
            }
        }

        // [2] Upper Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[0].x, point[0].y);
        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[8].x, point[8].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[9].x, point[9].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[1].x, point[1].y);

        // [4] Right Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[2].x, point[2].y);
        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[9].x, point[9].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[10].x, point[10].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[3].x, point[3].y);

        // [6] Bottom Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[11].x, point[11].y);
        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[5].x, point[5].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[4].x, point[4].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[10].x, point[10].y);

        // [8] Left Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[7].x, point[7].y);
        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[6].x, point[6].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[11].x, point[11].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[8].x, point[8].y);

        // [9] Middle Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[8].x, point[8].y);
        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[11].x, point[11].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[10].x, point[10].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[9].x, point[9].y);

    rlEnd();
    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);

        // Draw all of the 4 corners: [1] Upper Left Corner, [3] Upper Right Corner, [5] Lower Right Corner, [7] Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const Vector2 center = centers[k];
            for (int i = 0; i < segments; i++)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);
                rlVertex2f(center.x, center.y);
                rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius);
                rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);
                angle += stepLength;
            }
        }

        // [2] Upper Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(point[0].x, point[0].y);
        rlVertex2f(point[8].x, point[8].y);
        rlVertex2f(point[9].x, point[9].y);
        rlVertex2f(point[1].x, point[1].y);
        rlVertex2f(point[0].x, point[0].y);
        rlVertex2f(point[9].x, point[9].y);

        // [4] Right Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(point[9].x, point[9].y);
        rlVertex2f(point[10].x, point[10].y);
        rlVertex2f(point[3].x, point[3].y);
        rlVertex2f(point[2].x, point[2].y);
        rlVertex2f(point[9].x, point[9].y);
        rlVertex2f(point[3].x, point[3].y);

        // [6] Bottom Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(point[11].x, point[11].y);
        rlVertex2f(point[5].x, point[5].y);
        rlVertex2f(point[4].x, point[4].y);
        rlVertex2f(point[10].x, point[10].y);
        rlVertex2f(point[11].x, point[11].y);
        rlVertex2f(point[4].x, point[4].y);

        // [8] Left Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(point[7].x, point[7].y);
        rlVertex2f(point[6].x, point[6].y);
        rlVertex2f(point[11].x, point[11].y);
        rlVertex2f(point[8].x, point[8].y);
        rlVertex2f(point[7].x, point[7].y);
        rlVertex2f(point[11].x, point[11].y);

        // [9] Middle Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(point[8].x, point[8].y);
        rlVertex2f(point[11].x, point[11].y);
        rlVertex2f(point[10].x, point[10].y);
        rlVertex2f(point[9].x, point[9].y);
        rlVertex2f(point[8].x, point[8].y);
        rlVertex2f(point[10].x, point[10].y);
    rlEnd();
#endif
}

// Draw rectangle with rounded edges
void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, Color color)
{
    // Not a rounded rectangle
    if (roundness <= 0.0f)
    {
        DrawRectangleLines((int)rec.x, (int)rec.y, (int)rec.width, (int)rec.height, color);
        return;
    }

    if (roundness >= 1.0f) roundness = 1.0f;

    // Calculate corner radius
    float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
    if (radius <= 0.0f) return;

    // Calculate number of segments to use for the corners
    if (segments < 4)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)ceilf((2*PI/th)/4.0f);
        if (segments <= 0) segments = 4;
    }

    float stepLength = 90.0f/(float)segments;

    /*
    Quick sketch to make sense of all of this,
    marks the 8 + 4 (corner centers P8-11) points used

           P0 ------------------ P1
          /                        \
         /                          \
     P7 /                            \ P2
       |    *P8               P9*     |
       |                              |
       |                              |
     P6 \   *P11             P10*    / P3
         \                          /
          \                        /
           P5 ------------------ P4
    */

    // The x-coordinates used for the outline
    const float x0 = rec.x + radius + 0.5f;
    const float x1 = (rec.x + rec.width) - radius - 0.5f;
    const float x2 = rec.x + rec.width - 0.5f;
    const float x3 = rec.x + 0.5f;

    // The y-coordinates used for the outline
    const float y0 = rec.y + 0.5f;
    const float y1 = rec.y + radius + 0.5f;
    const float y2 = (rec.y + rec.height) - radius - 0.5f;
    const float y3 = rec.y + rec.height - 0.5f;

    const Vector2 point[8] = {
        {x0, y0}, // P0
        {x1, y0}, // P1
        {x2, y1}, // P2
        {x2, y2}, // P3
        {x1, y3}, // P4
        {x0, y3}, // P5
        {x3, y2}, // P6
        {x3, y1}, // P7
    };

    const Vector2 centers[4] = {
        {x0, y1}, // P16
        {x1, y1}, // P17
        {x1, y2}, // P18
        {x0, y2}  // P19
    };

    const float angles[4] = { 180.0f, 270.0f, 0.0f, 90.0f };

    rlBegin(RL_LINES);
        // Draw all the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const Vector2 center = centers[k];

            for (int i = 0; i < segments; i++)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);
                rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);
                rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius);
                angle += stepLength;
            }
        }

        // And now the remaining 4 lines
        for (int i = 0; i < 8; i += 2)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f(point[i].x, point[i].y);
            rlVertex2f(point[i + 1].x, point[i + 1].y);
        }
    rlEnd();
}

// Draw rectangle with rounded edges outline with line thickness
void DrawRectangleRoundedLinesEx(Rectangle rec, float roundness, int segments, float thick, Color color)
{
    // Not a rounded rectangle
    if (roundness <= 0.0f)
    {
        DrawRectangleLinesEx(rec, thick, color);
        return;
    }

    if (roundness >= 1.0f) roundness = 1.0f;

    float radius = 0.0f;
    float roundedOutlineThick = 0.0f;
    float outerRadius = 0.0f;
    float innerRadius = 0.0f;
    if (thick >= 0.0f)
    {
        // Calculate corner radius
        radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
        if (radius <= 0.0f) return;

        outerRadius = radius;
        innerRadius = outerRadius - thick;

        // The maximum thickness the outline can have and still be rounded on the interior edge is equal to the corner radius
        // Put another way, when `innerRadius <= 0`, the interior of the outline is just a normal rectangle with no rounding
        if (innerRadius <= 0.0f)
        {
            innerRadius = 0.0f;
            roundedOutlineThick = outerRadius;

            // Draw the not-rounded portion of the outline
            DrawRectangleLinesEx((Rectangle){ rec.x + outerRadius, rec.y + outerRadius, rec.width - outerRadius*2.0f, rec.height - outerRadius*2.0f }, thick - outerRadius, color);
        }
        else
        {
            roundedOutlineThick = thick;
        }

        // Calculate number of segments to use for the corners
        if (segments < 4)
        {
            // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
            float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/outerRadius, 2) - 1);
            segments = (int)ceilf((2*PI/th)/4.0f);
            if (segments <= 0) segments = 4;
        }
    }
    else
    {
        thick *= -1.0f;

        // Calculate corner radius
        radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
        if (radius <= 0.0f) return; // Only possible if the rectangle has 0 width or height

        // Expand the rectangle
        rec.x -= thick;
        rec.y -= thick;
        rec.width += thick*2.0f;
        rec.height += thick*2.0f;

        innerRadius = radius;
        outerRadius = innerRadius + thick;
        roundedOutlineThick = thick;

        // Calculate number of segments to use for the corners
        if (segments < 4)
        {
            // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
            float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/innerRadius, 2) - 1);
            segments = (int)ceilf((2*PI/th)/4.0f);
            if (segments <= 0) segments = 4;
        }
    }

    float stepLength = 90.0f/(float)segments;

    /*
    Quick sketch to make sense of all of this,
    marks the 16 + 4(corner centers P16-19) points used

           P0 ================== P1
          // P8                P9 \\
         //                        \\
     P7 // P15                  P10 \\ P2
       ||   *P16             P17*    ||
       ||                            ||
       || P14                   P11  ||
     P6 \\  *P19             P18*   // P3
         \\                        //
          \\ P13              P12 //
           P5 ================== P4
    */

    // The x-coordinates used for the outline
    const float x0 = rec.x + outerRadius;
    const float x1 = (rec.x + rec.width) - outerRadius;
    const float x2 = rec.x + rec.width;
    const float x3 = rec.x;
    const float x4 = rec.x + rec.width - roundedOutlineThick;
    const float x5 = rec.x + roundedOutlineThick;

    // The y-coordinates used for the outline
    const float y0 = rec.y;
    const float y1 = rec.y + outerRadius;
    const float y2 = (rec.y + rec.height) - outerRadius;
    const float y3 = rec.y + rec.height;
    const float y4 = rec.y + roundedOutlineThick;
    const float y5 = rec.y + rec.height - roundedOutlineThick;

    const Vector2 point[16] = {
        {x0, y0}, // P0
        {x1, y0}, // P1
        {x2, y1}, // P2
        {x2, y2}, // P3
        {x1, y3}, // P4
        {x0, y3}, // P5
        {x3, y2}, // P6
        {x3, y1}, // P7
        {x0, y4}, // P8
        {x1, y4}, // P9
        {x4, y1}, // P10
        {x4, y2}, // P11
        {x1, y5}, // P12
        {x0, y5}, // P13
        {x5, y2}, // P14
        {x5, y1}  // P15
    };

    const Vector2 centers[4] = {
        {x0, y1}, // P16
        {x1, y1}, // P17
        {x1, y2}, // P18
        {x0, y2}  // P19
    };

    const float angles[4] = { 180.0f, 270.0f, 0.0f, 90.0f };

#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);

        // Draw all the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const Vector2 center = centers[k];
            for (int i = 0; i < segments; i++)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);

                rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*outerRadius);

                rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);

                angle += stepLength;
            }
        }

        // Upper rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[0].x, point[0].y);
        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[8].x, point[8].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[9].x, point[9].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[1].x, point[1].y);

        // Right rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[2].x, point[2].y);
        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[10].x, point[10].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[11].x, point[11].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[3].x, point[3].y);

        // Lower rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[13].x, point[13].y);
        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[5].x, point[5].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[4].x, point[4].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[12].x, point[12].y);

        // Left rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[15].x, point[15].y);
        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[7].x, point[7].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
        rlVertex2f(point[6].x, point[6].y);
        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
        rlVertex2f(point[14].x, point[14].y);

    rlEnd();
    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);

        // Draw all of the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const Vector2 center = centers[k];

            for (int i = 0; i < segments; i++)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);

                rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);
                rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);
                rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);

                rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);
                rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*outerRadius);
                rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);

                angle += stepLength;
            }
        }

        // Upper rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(point[0].x, point[0].y);
        rlVertex2f(point[8].x, point[8].y);
        rlVertex2f(point[9].x, point[9].y);
        rlVertex2f(point[1].x, point[1].y);
        rlVertex2f(point[0].x, point[0].y);
        rlVertex2f(point[9].x, point[9].y);

        // Right rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(point[10].x, point[10].y);
        rlVertex2f(point[11].x, point[11].y);
        rlVertex2f(point[3].x, point[3].y);
        rlVertex2f(point[2].x, point[2].y);
        rlVertex2f(point[10].x, point[10].y);
        rlVertex2f(point[3].x, point[3].y);

        // Lower rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(point[13].x, point[13].y);
        rlVertex2f(point[5].x, point[5].y);
        rlVertex2f(point[4].x, point[4].y);
        rlVertex2f(point[12].x, point[12].y);
        rlVertex2f(point[13].x, point[13].y);
        rlVertex2f(point[4].x, point[4].y);

        // Left rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(point[7].x, point[7].y);
        rlVertex2f(point[6].x, point[6].y);
        rlVertex2f(point[14].x, point[14].y);
        rlVertex2f(point[15].x, point[15].y);
        rlVertex2f(point[7].x, point[7].y);
        rlVertex2f(point[14].x, point[14].y);
    rlEnd();
#endif
}

// Draw a polygon of n sides
void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color)
{
    if (sides < 3) sides = 3;
    float centralAngle = rotation*DEG2RAD;
    float angleStep = 360.0f/(float)sides*DEG2RAD;

#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);
        for (int i = 0; i < sides; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            float nextAngle = centralAngle + angleStep;

            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x, center.y);

            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(centralAngle)*radius, center.y + sinf(centralAngle)*radius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(nextAngle)*radius, center.y + sinf(nextAngle)*radius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(centralAngle)*radius, center.y + sinf(centralAngle)*radius);

            centralAngle = nextAngle;
        }
    rlEnd();
    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);
        for (int i = 0; i < sides; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlVertex2f(center.x, center.y);
            rlVertex2f(center.x + cosf(centralAngle + angleStep)*radius, center.y + sinf(centralAngle + angleStep)*radius);
            rlVertex2f(center.x + cosf(centralAngle)*radius, center.y + sinf(centralAngle)*radius);

            centralAngle += angleStep;
        }
    rlEnd();
#endif
}

// Draw a polygon outline of n sides
void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color)
{
    if (sides < 3) sides = 3;
    float centralAngle = rotation*DEG2RAD;
    float angleStep = 360.0f/(float)sides*DEG2RAD;

    rlBegin(RL_LINES);
        for (int i = 0; i < sides; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlVertex2f(center.x + cosf(centralAngle)*radius, center.y + sinf(centralAngle)*radius);
            rlVertex2f(center.x + cosf(centralAngle + angleStep)*radius, center.y + sinf(centralAngle + angleStep)*radius);

            centralAngle += angleStep;
        }
    rlEnd();
}

void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation, float thick, Color color)
{
    if (sides < 3) sides = 3;
    float centralAngle = rotation*DEG2RAD;
    float exteriorAngle = 360.0f/(float)sides*DEG2RAD;
    float apothem = radius*cosf(DEG2RAD*180.0f/(float)sides);

    float outerRadius = 0.0f;
    float innerRadius = 0.0f;
    if (thick >= 0.0f)
    {
        outerRadius = radius;
        innerRadius = fmaxf(0.0f, radius - thick*(radius/apothem));
    }
    else
    {
        thick *= -1.0f;
        outerRadius = radius + thick*(radius/apothem);
        innerRadius = radius;
    }

#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);
        for (int i = 0; i < sides; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            float nextAngle = centralAngle + exteriorAngle;

            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(centralAngle)*outerRadius, center.y + sinf(centralAngle)*outerRadius);

            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(centralAngle)*innerRadius, center.y + sinf(centralAngle)*innerRadius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(nextAngle)*innerRadius, center.y + sinf(nextAngle)*innerRadius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(nextAngle)*outerRadius, center.y + sinf(nextAngle)*outerRadius);

            centralAngle = nextAngle;
        }
    rlEnd();
    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);
        for (int i = 0; i < sides; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            float nextAngle = centralAngle + exteriorAngle;

            rlVertex2f(center.x + cosf(nextAngle)*outerRadius, center.y + sinf(nextAngle)*outerRadius);
            rlVertex2f(center.x + cosf(centralAngle)*outerRadius, center.y + sinf(centralAngle)*outerRadius);
            rlVertex2f(center.x + cosf(centralAngle)*innerRadius, center.y + sinf(centralAngle)*innerRadius);

            rlVertex2f(center.x + cosf(centralAngle)*innerRadius, center.y + sinf(centralAngle)*innerRadius);
            rlVertex2f(center.x + cosf(nextAngle)*innerRadius, center.y + sinf(nextAngle)*innerRadius);
            rlVertex2f(center.x + cosf(nextAngle)*outerRadius, center.y + sinf(nextAngle)*outerRadius);

            centralAngle = nextAngle;
        }
    rlEnd();
#endif
}

// Draw a color-filled circle
void DrawCircle(int centerX, int centerY, float radius, Color color)
{
    DrawCircleV((Vector2){ (float)centerX, (float)centerY }, radius, color);
}

// Draw a color-filled circle (Vector version)
// NOTE: On OpenGL 3.3 and ES2 using QUADS to avoid drawing order issues
void DrawCircleV(Vector2 center, float radius, Color color)
{
    DrawCircleSector(center, radius, 0, 360, 36, color);
}

// Draw a gradient-filled circle
void DrawCircleGradient(Vector2 center, float radius, Color inner, Color outer)
{
    rlBegin(RL_TRIANGLES);
        for (int i = 0; i < 360; i += 10)
        {
            rlColor4ub(inner.r, inner.g, inner.b, inner.a);
            rlVertex2f(center.x, center.y);
            rlColor4ub(outer.r, outer.g, outer.b, outer.a);
            rlVertex2f(center.x + cosf(DEG2RAD*(i + 10))*radius, center.y + sinf(DEG2RAD*(i + 10))*radius);
            rlColor4ub(outer.r, outer.g, outer.b, outer.a);
            rlVertex2f(center.x + cosf(DEG2RAD*i)*radius, center.y + sinf(DEG2RAD*i)*radius);
        }
    rlEnd();
}

// Draw a piece of a circle
void DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color)
{
    if (startAngle == endAngle) return;
    if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    // Drawing a whole circle, things get weird without limiting the circle to 360 degrees
    if (endAngle - startAngle >= 360.0f) endAngle = startAngle + 360.0f;

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)ceilf((endAngle - startAngle)*(2*PI/th)/360.0f);

        if (segments <= 0) segments = minSegments;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;

#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);

        // NOTE: Every QUAD actually represents two segments
        for (int i = 0; i < segments/2; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x, center.y);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength*2.0f))*radius, center.y + sinf(DEG2RAD*(angle + stepLength*2.0f))*radius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius);

            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);

            angle += (stepLength*2.0f);
        }

        // NOTE: In case number of segments is odd, adding one last piece to the cake
        if ((((unsigned int)segments)%2) == 1)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x, center.y);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius);

            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x, center.y);
        }

    rlEnd();

    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);
        for (int i = 0; i < segments; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlVertex2f(center.x, center.y);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);

            angle += stepLength;
        }
    rlEnd();
#endif
}

// Draw a piece of a circle outlines
void DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color)
{
    if (startAngle == endAngle) return;
    if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero issue

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    bool showCapLines = true;
    // Drawing a whole circle, things get weird without limiting the circle to 360 degrees
    if (endAngle - startAngle >= 360.0f)
    {
        showCapLines = false;
        endAngle = startAngle + 360.0f;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)ceilf((endAngle - startAngle)*(2*PI/th)/360.0f);

        if (segments <= 0) segments = minSegments;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;

    rlBegin(RL_LINES);
        if (showCapLines)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f(center.x, center.y);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);
        }

        for (int i = 0; i < segments; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*radius, center.y + sinf(DEG2RAD*(angle + stepLength))*radius);

            angle += stepLength;
        }

        if (showCapLines)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f(center.x, center.y);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*radius, center.y + sinf(DEG2RAD*angle)*radius);
        }
    rlEnd();
}

// Draw a piece of a circle outlines with thickness
void DrawCircleSectorLinesEx(Vector2 center, float radius, float startAngle, float endAngle, int segments, float thick, Color color)
{
    if (startAngle == endAngle) return;
    if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero issue

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    bool showCapLines = true;
    // Drawing a whole circle, things get weird without limiting the circle to 360 degrees
    if (endAngle - startAngle >= 360.0f)
    {
        showCapLines = thick >= 0.0f;
        endAngle = startAngle + 360.0f;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)ceilf((endAngle - startAngle)*(2*PI/th)/360.0f);

        if (segments <= 0) segments = minSegments;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;

    /*
    A sketch to help make things clearer

    NOTE: Some considerations are different when `thick` is negative
          The vertices used here are still relevant, but would instead be outside of the circle
          S0 is always the center

    The circle sector outline is drawn in 3 main pieces, the circle outline, cap 1, and cap 2
    The circle outline is self explanatory
    Cap 1 covers the `startAngle` edge and cap 2 covers the `endAngle` edge
    S0 is the first shared point between the caps, and also the circle's center
    S1 is the second shared point (sometimes not shared) between the caps
      S1 is also C0 and C3 in this sketch. In certain cases, S1 goes outside of
      the circle and C0 and C3 become different points
    C1 is one of cap 1's vertices that is on the inside edge of the circle outline
    C2 is like C1, but is also on the `startAngle` edge
    C4 is cap 2's vertex that corresponds with C1
    C5 is cap 2's vertex that corresponds with C2, except on the `endAngle` edge

                   [][][][][]
               [][]        []
             []      []C4[]C5
           []    [][]  {}  {}
         []    []      {}  {}
       []    []        {}C {} <- endAngle
       []  []          {}a {}
     []    []          {}p {}
     []  []            {}2 {}
     []  []            {}  {}
     []  []            {}  {}     startAngle
     []  []            {}  S0{}{}{}{}{}{}{}{}C2[][]
     []  []            {}{}       Cap1       []  []
     []  []            S1{}{}{}{}{}{}{}{}{}{}C1  []
     []  []                                  []  []
     []    []                              []    []
       []  []       Not filled in          []  []
       []    []                          []    []
         []    []                      []    []
           []    [][]              [][]    []
             []      [][][][][][][]      []
               [][]  Circle outline  [][]
                   [][][][][][][][][]

    [] = Circle outline edge pixel
    {} = Cap outline edge pixel
    */

    // We are not drawing a circle, we are drawing an n-sided polygon
    // So, we need to adjust the outline thickness of the "circle" for it to look correct with fewer segments
    float apothem = radius*cosf(DEG2RAD*((endAngle - startAngle)/2.0f)/(float)segments);
    float radiusThick = thick*(radius/apothem);

    float outerRadius = radius;
    float innerRadius = radius - radiusThick;

    if (thick >= 0.0f)
    {
        if (thick >= innerRadius)
        {
            DrawCircleSector(center, radius, startAngle, endAngle, segments, color);
            return;
        }
    }
    else
    {
        float tmp = outerRadius;
        outerRadius = innerRadius;
        innerRadius = tmp;
    }

    // Cap 1 vertices
    Vector2 c0 = { 0 };
    Vector2 c1 = { 0 };
    Vector2 c2 = { 0 };

    // Cap 2 vertices
    Vector2 c3 = { 0 };
    Vector2 c4 = { 0 };
    Vector2 c5 = { 0 };

    // The number of angle steps that come before C1 (from `startAngle`, counter clockwise)
    int stepsBeforeC1 = 0;
    bool s1OutsideOfCircle = false;
    // The number of angle steps that come before C0 (from `startAngle`, counter clockwise)
    // Only used if S1 is outside of the circle
    int stepsBeforeC0 = 0;

    if (showCapLines)
    {
        if (thick >= 0.0f)
        {
            c2 = (Vector2){ center.x + cosf(DEG2RAD*startAngle)*innerRadius, center.y + sinf(DEG2RAD*startAngle)*innerRadius };
            c5 = (Vector2){ center.x + cosf(DEG2RAD*endAngle)*innerRadius, center.y + sinf(DEG2RAD*endAngle)*innerRadius };

            // For C1 and C4, we need to find the point that lies on the circle (n-sided polygon, actually)
            // We want C1 and C4 to be `thick` pixels perpendicularly from the `startAngle` and `endAngle` edges
            // and to be on the `innerRadius` edge

            float c1Angle = RAD2DEG*asinf(thick/innerRadius);

            // There are more segments before C1 than there are segments being drawn,
            // so the whole circle sector must be covered
            if (c1Angle/stepLength >= (float)segments)
            {
                DrawCircleSector(center, radius, startAngle, endAngle, segments, color);
                return;
            }

            // Do this after the previous check just in case `stepLength` is really small and
            // dividing by it produces a very large number
            stepsBeforeC1 = (int)(c1Angle/stepLength);

            // The angles of the vertices on the circle outline before and after C1
            float vertexAngleBeforeC1 = stepLength*(float)stepsBeforeC1;
            float vertexAngleAfterC1 = stepLength*((float)(stepsBeforeC1 + 1));

            /*
            Here is another sketch

            We know which outline line segment C1 is on (`vertexAngleBeforeC1` and `vertexAngleAfterC1`)
            Now we just need to know where on that line segment C1 is

            We can change our frame of reference so that `startAngle` is 0 degrees and `center` is at the origin (0, 0)
            This makes the math much simpler because now we can just go straight down by `thick` pixels and
            use the horizontal line that passes through that point to determine where C1 is on our line segment

            The line segment is defined by p1 and p2, we need C1, which is on that edge
            The 'y' axis of C1 is equal to `thick` (within this modified frame of reference)

                         p1
                         /|
                        / |
                       /  |
                      /   |
                     /    |
                    /     |
                   /      |
                  /       |
                C1---------  <-- y axis = `thick`
                /
               /
             p2
            */

            Vector2 p1 = { cosf(DEG2RAD*vertexAngleBeforeC1)*innerRadius, sinf(DEG2RAD*vertexAngleBeforeC1)*innerRadius };
            Vector2 p2 = { cosf(DEG2RAD*vertexAngleAfterC1)*innerRadius, sinf(DEG2RAD*vertexAngleAfterC1)*innerRadius };

            // Find the `t` of C1 between p1 and p2 ('t' as in `Lerp(start, end, t)`)
            // This is used to lerp between the actual vertices (outside of our modified frame of reference)
            // before and after C1
            float t = (p1.y - thick)/(p1.y - p2.y);

            Vector2 vertexBeforeCap1Vertex = { center.x + cosf(DEG2RAD*(startAngle + vertexAngleBeforeC1))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + vertexAngleBeforeC1))*innerRadius };
            Vector2 vertexAfterCap1Vertex = { center.x + cosf(DEG2RAD*(startAngle + vertexAngleAfterC1))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + vertexAngleAfterC1))*innerRadius };

            c1.x = vertexBeforeCap1Vertex.x + (vertexAfterCap1Vertex.x - vertexBeforeCap1Vertex.x)*t;
            c1.y = vertexBeforeCap1Vertex.y + (vertexAfterCap1Vertex.y - vertexBeforeCap1Vertex.y)*t;

            Vector2 vertexBeforeCap2Vertex = { center.x + cosf(DEG2RAD*(endAngle - vertexAngleBeforeC1))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - vertexAngleBeforeC1))*innerRadius };
            Vector2 vertexAfterCap2Vertex = { center.x + cosf(DEG2RAD*(endAngle - vertexAngleAfterC1))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - vertexAngleAfterC1))*innerRadius };

            c4.x = vertexBeforeCap2Vertex.x + (vertexAfterCap2Vertex.x - vertexBeforeCap2Vertex.x)*t;
            c4.y = vertexBeforeCap2Vertex.y + (vertexAfterCap2Vertex.y - vertexBeforeCap2Vertex.y)*t;

            /*
            Another sketch couldn't hurt

            This is a "zoomed in" view of the center of the circle sector
            You can see where S0 is, and where we want S1 to be
            `innerAngleBetweenCapEnds` is the angle of the diagonal line ('//') between the cap ends
            `S1Length` is the length of that line

            Since the caps are always parallel to `startAngle` and `endAngle`,
            we always have a right triangle we can use to determine where S1 is

             []      []
             []      []
             []  C   [] <- endAngle
             []  a   []
             []  p   []
             []  2   []  startAngle
             [][][][]S0[][][][][][]
                   //[]
                 //  [] Cap 1
               //    []
             S1      [][][][][][][]
            */

            float innerAngleBetweenCapEnds = ((endAngle - 90.0f) - (startAngle + 90.0f))/2.0f;
            float s1Length = thick/cosf(DEG2RAD*innerAngleBetweenCapEnds);

            // As `startAngle` and `endAngle` draw more of a circle, S1 goes further out from the center
            // It can go so far that it is outside of the circle, by a lot
            // This case needs to be detected and handled
            // If S1 is within the circle, nothing special needs to happen
            // But, if S1 is outside of the circle, we need to find the two points (C0 and C3) where
            // the line segments C0->C1 and C0->C3 intersect the circle outline,
            // using the same method we used to find C1 and C4
            if ((innerAngleBetweenCapEnds < 90.0f) && (s1Length <= innerRadius))
            {
                // S1 is inside of the circle

                float betweenStartAndEndAngle = (endAngle + startAngle)/2.0f;
                c0 = (Vector2){ center.x + cosf(DEG2RAD*betweenStartAndEndAngle)*s1Length, center.y + sinf(DEG2RAD*betweenStartAndEndAngle)*s1Length };
                c3 = c0;

                p1 = (Vector2){ c1.x - center.x, c1.y - center.y };
                p2 = (Vector2){ c0.x - center.x, c0.y - center.y };

                // Copied from "raymath.h" Vector2Angle()
                float dot = p1.x*p2.x + p1.y*p2.y;
                float det = p1.x*p2.y - p1.y*p2.x;
                float c1ToS1Angle = atan2f(det, dot);

                // If C1 and C4 are on the wrong side of S1, the whole circle sector is covered
                if (c1ToS1Angle < 0.0f)
                {
                    DrawCircleSector(center, radius, startAngle, endAngle, segments, color);
                    return;
                }
            }
            else
            {
                // S1 is outside of the circle

                if (endAngle - startAngle <= 180.0f)
                {
                    DrawCircleSector(center, radius, startAngle, endAngle, segments, color);
                    return;
                }

                s1OutsideOfCircle = true;

                stepsBeforeC0 = (int)((180.0f + RAD2DEG*asinf(thick/-innerRadius))/stepLength);

                // Reuse the code for finding C1 and C4 to find C0 and C3

                float vertexAngleBeforeC0 = stepLength*(float)stepsBeforeC0;
                float vertexAngleAfterC0 = stepLength*((float)(stepsBeforeC0 + 1));

                p1 = (Vector2){ cosf(DEG2RAD*vertexAngleBeforeC0)*innerRadius, sinf(DEG2RAD*vertexAngleBeforeC0)*innerRadius };
                p2 = (Vector2){ cosf(DEG2RAD*vertexAngleAfterC0)*innerRadius, sinf(DEG2RAD*vertexAngleAfterC0)*innerRadius };

                t = (p1.y - thick)/(p1.y - p2.y);

                vertexBeforeCap1Vertex = (Vector2){ center.x + cosf(DEG2RAD*(startAngle + vertexAngleBeforeC0))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + vertexAngleBeforeC0))*innerRadius };
                vertexAfterCap1Vertex = (Vector2){ center.x + cosf(DEG2RAD*(startAngle + vertexAngleAfterC0))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + vertexAngleAfterC0))*innerRadius };

                c0.x = vertexBeforeCap1Vertex.x + (vertexAfterCap1Vertex.x - vertexBeforeCap1Vertex.x)*t;
                c0.y = vertexBeforeCap1Vertex.y + (vertexAfterCap1Vertex.y - vertexBeforeCap1Vertex.y)*t;

                vertexBeforeCap2Vertex = (Vector2){ center.x + cosf(DEG2RAD*(endAngle - vertexAngleBeforeC0))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - vertexAngleBeforeC0))*innerRadius };
                vertexAfterCap2Vertex = (Vector2){ center.x + cosf(DEG2RAD*(endAngle - vertexAngleAfterC0))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - vertexAngleAfterC0))*innerRadius };

                c3.x = vertexBeforeCap2Vertex.x + (vertexAfterCap2Vertex.x - vertexBeforeCap2Vertex.x)*t;
                c3.y = vertexBeforeCap2Vertex.y + (vertexAfterCap2Vertex.y - vertexBeforeCap2Vertex.y)*t;
            }
        }
        else
        {
            float outerAngleBetweenCapEnds = ((endAngle + 90.0f) - (startAngle - 90.0f))/2.0f;
            float s1Length = thick/cosf(DEG2RAD*outerAngleBetweenCapEnds);
            float betweenStartAndEndAngle = 180.0f + (endAngle + startAngle)/2.0f;
            c0 = (Vector2){ center.x + cosf(DEG2RAD*betweenStartAndEndAngle)*s1Length, center.y + sinf(DEG2RAD*betweenStartAndEndAngle)*s1Length };
            c3 = c0;

            c2 = (Vector2){ center.x + cosf(DEG2RAD*startAngle)*outerRadius, center.y + sinf(DEG2RAD*startAngle)*outerRadius };
            c5 = (Vector2){ center.x + cosf(DEG2RAD*endAngle)*outerRadius, center.y + sinf(DEG2RAD*endAngle)*outerRadius };

            // Change the frame of reference so that `center` is the origin and `startAngle` is 0 degrees

            Vector2 c0Translated = { c0.x - center.x, c0.y - center.y };
            Vector2 circleVertex1 = { c2.x - center.x, c2.y - center.y };
            Vector2 circleVertex2 = { cosf(DEG2RAD*(startAngle + stepLength))*outerRadius, sinf(DEG2RAD*(startAngle + stepLength))*outerRadius };

            // Copied from "raymath.h" Vector2Rotate()
            float tempX = c0Translated.x;
            c0Translated.x = cosf(-DEG2RAD*startAngle)*tempX - sinf(-DEG2RAD*startAngle)*c0Translated.y;
            c0Translated.y = sinf(-DEG2RAD*startAngle)*tempX + cosf(-DEG2RAD*startAngle)*c0Translated.y;

            tempX = circleVertex1.x;
            circleVertex1.x = cosf(-DEG2RAD*startAngle)*tempX - sinf(-DEG2RAD*startAngle)*circleVertex1.y;
            circleVertex1.y = sinf(-DEG2RAD*startAngle)*tempX + cosf(-DEG2RAD*startAngle)*circleVertex1.y;

            tempX = circleVertex2.x;
            circleVertex2.x = cosf(-DEG2RAD*startAngle)*tempX - sinf(-DEG2RAD*startAngle)*circleVertex2.y;
            circleVertex2.y = sinf(-DEG2RAD*startAngle)*tempX + cosf(-DEG2RAD*startAngle)*circleVertex2.y;

            // Figure out the line that `circleVertex1` and `circleVertex2` are on
            float rise = circleVertex1.y - circleVertex2.y;
            float run = circleVertex1.x - circleVertex2.x;
            // Get where that line intersects the horizontal line that `c0Translated` is on
            float c1Rise = c0Translated.y - circleVertex1.y;
            float c1Run = (c1Rise/rise)*run;
            float c1DistanceFromC0 = (circleVertex1.x + c1Run) - c0Translated.x;

            c1 = (Vector2){ c0.x + cosf(DEG2RAD*startAngle)*c1DistanceFromC0, c0.y + sinf(DEG2RAD*startAngle)*c1DistanceFromC0 };
            c4 = (Vector2){ c0.x + cosf(DEG2RAD*endAngle)*c1DistanceFromC0, c0.y + sinf(DEG2RAD*endAngle)*c1DistanceFromC0 };

            if (c1DistanceFromC0 < 0.0f)
            {
                // The caps are intersecting each other

                Vector2 circleVertex3 = { c5.x - center.x, c5.y - center.y };
                Vector2 circleVertex4 = { cosf(DEG2RAD*(endAngle - stepLength))*outerRadius, sinf(DEG2RAD*(endAngle - stepLength))*outerRadius };

                tempX = circleVertex3.x;
                circleVertex3.x = cosf(-DEG2RAD*startAngle)*tempX - sinf(-DEG2RAD*startAngle)*circleVertex3.y;
                circleVertex3.y = sinf(-DEG2RAD*startAngle)*tempX + cosf(-DEG2RAD*startAngle)*circleVertex3.y;

                tempX = circleVertex4.x;
                circleVertex4.x = cosf(-DEG2RAD*startAngle)*tempX - sinf(-DEG2RAD*startAngle)*circleVertex4.y;
                circleVertex4.y = sinf(-DEG2RAD*startAngle)*tempX + cosf(-DEG2RAD*startAngle)*circleVertex4.y;

                // `startAngle` is 0 degrees within this frame of reference,
                // so C1 just goes horizontally out from C0
                Vector2 c1Translated = { c0Translated.x + c1DistanceFromC0, c0Translated.y };

                // Make `circleVertex2` the origin
                circleVertex1.x -= circleVertex2.x;
                circleVertex1.y -= circleVertex2.y;
                circleVertex3.x -= circleVertex2.x;
                circleVertex3.y -= circleVertex2.y;
                circleVertex4.x -= circleVertex2.x;
                circleVertex4.y -= circleVertex2.y;
                c1Translated.x -= circleVertex2.x;
                c1Translated.y -= circleVertex2.y;

                // Make the line between `circleVertex1` and `circleVertex2` a horizontal line
                float theta = atan2f(circleVertex1.y, circleVertex1.x);

                // Copied from "raymath.h" Vector2Rotate()
                tempX = circleVertex1.x;
                circleVertex1.x = cosf(-theta)*tempX - sinf(-theta)*circleVertex1.y;
                circleVertex1.y = sinf(-theta)*tempX + cosf(-theta)*circleVertex1.y;

                tempX = circleVertex3.x;
                circleVertex3.x = cosf(-theta)*tempX - sinf(-theta)*circleVertex3.y;
                circleVertex3.y = sinf(-theta)*tempX + cosf(-theta)*circleVertex3.y;

                tempX = circleVertex4.x;
                circleVertex4.x = cosf(-theta)*tempX - sinf(-theta)*circleVertex4.y;
                circleVertex4.y = sinf(-theta)*tempX + cosf(-theta)*circleVertex4.y;

                tempX = c1Translated.x;
                c1Translated.x = cosf(-theta)*tempX - sinf(-theta)*c1Translated.y;
                c1Translated.y = sinf(-theta)*tempX + cosf(-theta)*c1Translated.y;

                // Find where the line that `circleVertex3` and `circleVertex4` are on would intersect the
                // line segment defined by `circleVertex1` and `c1Translated`
                rise = circleVertex3.y - circleVertex4.y;
                run = circleVertex3.x - circleVertex4.x;
                float targetRise = -circleVertex3.y;
                float targetX = circleVertex3.x + (targetRise/rise)*run;

                float t = (c1Translated.x - targetX)/(c1Translated.x - circleVertex1.x);

                c1 = (Vector2){ c1.x + (c2.x - c1.x)*t, c1.y + (c2.y - c1.y)*t };
                c4 = c1;
                c0 = c1;
                c3 = c1;
            }

            // Swap vertices to correct the winding order
            Vector2 temp = c0;
            c0 = c2;
            c2 = temp;

            temp = c3;
            c3 = c5;
            c5 = temp;
        }
    }

#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);

        rlColor4ub(color.r, color.g, color.b, color.a);

        // Draw the circle outline
        for (int i = 0; i < segments; i++)
        {
            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);

            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*outerRadius);

            angle += stepLength;
        }

        // Draw the caps
        if (showCapLines)
        {
            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x, center.y);

            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(c0.x, c0.y);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(c1.x, c1.y);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(c2.x, c2.y);


            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x, center.y);

            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(c5.x, c5.y);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(c4.x, c4.y);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(c3.x, c3.y);

            // Some extra work may be needed when `thick` is positive
            if (thick >= 0.0f)
            {
                // Fill in the gaps between cap 1 and the circle outline and cap 2 and the circle outline
                if (stepsBeforeC1 > 0)
                {
                    // Draw quads using pairs of vertices on the circle outline
                    angle = 0;
                    for (int i = 0; i < stepsBeforeC1/2; i++)
                    {
                        // Cap1
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(c1.x, c1.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + (angle + stepLength*2.0f)))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + (angle + stepLength*2.0f)))*innerRadius);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + (angle + stepLength)))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + (angle + stepLength)))*innerRadius);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*innerRadius);

                        // Cap2
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(c4.x, c4.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*innerRadius);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - (angle + stepLength)))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - (angle + stepLength)))*innerRadius);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - (angle + stepLength*2.0f)))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - (angle + stepLength*2.0f)))*innerRadius);

                        angle += stepLength*2.0f;
                    }

                    if (stepsBeforeC1%2 == 1)
                    {
                        // Cap1
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(c1.x, c1.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(c1.x, c1.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + (angle + stepLength)))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + (angle + stepLength)))*innerRadius);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*innerRadius);

                        // Cap2
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(c4.x, c4.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(c4.x, c4.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*innerRadius);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - (angle + stepLength)))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - (angle + stepLength)))*innerRadius);
                    }
                }

                // Fill in the gap between C0, C3 and the circle outline
                if (s1OutsideOfCircle)
                {
                    int verticesBetweenC0andC3 = (segments - stepsBeforeC0*2) - 1;

                    // No gap to fill
                    if (verticesBetweenC0andC3 == 0)
                    {
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(center.x, center.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(center.x, center.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(c3.x, c3.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(c0.x, c0.y);
                    }
                    // There's a gap to fill
                    else
                    {
                        // Triangle touching C0
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(center.x, center.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(center.x, center.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + stepLength*(float)(stepsBeforeC0 + 1)))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + stepLength*(float)(stepsBeforeC0 + 1)))*innerRadius);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(c0.x, c0.y);

                        // Triangle touching C3
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(center.x, center.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(center.x, center.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(c3.x, c3.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - stepLength*(float)(stepsBeforeC0 + 1)))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - stepLength*(float)(stepsBeforeC0 + 1)))*innerRadius);

                        // Triangles between the previous two
                        verticesBetweenC0andC3 -= 1;
                        angle = startAngle + stepLength*(stepsBeforeC0 + 1);
                        for (int i = 0; i < verticesBetweenC0andC3/2; i++)
                        {
                            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                            rlVertex2f(center.x, center.y);

                            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength*2.0f))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength*2.0f))*innerRadius);

                            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);

                            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);

                            angle += stepLength*2.0f;
                        }

                        if (verticesBetweenC0andC3%2 == 1)
                        {
                            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                            rlVertex2f(center.x, center.y);

                            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                            rlVertex2f(center.x, center.y);

                            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);

                            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);
                        }
                    }
                }
            }
        }
    rlEnd();

    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);

        rlColor4ub(color.r, color.g, color.b, color.a);

        // Draw the circle outline
        for (int i = 0; i < segments; i++)
        {
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);

            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*outerRadius);

            angle += stepLength;
        }

        // Draw the caps
        if (showCapLines)
        {
            // Cap 1
            rlVertex2f(center.x, center.y);
            rlVertex2f(c0.x, c0.y);
            rlVertex2f(c1.x, c1.y);

            rlVertex2f(center.x, center.y);
            rlVertex2f(c1.x, c1.y);
            rlVertex2f(c2.x, c2.y);

            // Cap 2
            rlVertex2f(center.x, center.y);
            rlVertex2f(c5.x, c5.y);
            rlVertex2f(c4.x, c4.y);

            rlVertex2f(center.x, center.y);
            rlVertex2f(c4.x, c4.y);
            rlVertex2f(c3.x, c3.y);

            // Some extra work may be needed when `thick` is positive
            if (thick >= 0.0f)
            {
                // Fill in the gaps between cap 1 and the circle outline and cap 2 and the circle outline
                if (stepsBeforeC1 > 0)
                {
                    angle = 0;
                    for (int i = 0; i < stepsBeforeC1; i++)
                    {
                        // Cap 1
                        rlVertex2f(c1.x, c1.y);
                        rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + (angle + stepLength)))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + (angle + stepLength)))*innerRadius);
                        rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*innerRadius);

                        // Cap 2
                        rlVertex2f(c4.x, c4.y);
                        rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*innerRadius);
                        rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - (angle + stepLength)))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - (angle + stepLength)))*innerRadius);

                        angle += stepLength;
                    }
                }

                // Fill in the gap between C0, C3 and the circle outline
                if (s1OutsideOfCircle)
                {
                    int verticesBetweenC0andC3 = (segments - stepsBeforeC0*2) - 1;

                    // No gap to fill
                    if (verticesBetweenC0andC3 == 0)
                    {
                        rlVertex2f(center.x, center.y);
                        rlVertex2f(c3.x, c3.y);
                        rlVertex2f(c0.x, c0.y);
                    }
                    // There's a gap to fill
                    else
                    {
                        // Triangle touching C0
                        rlVertex2f(center.x, center.y);
                        rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + stepLength*(float)(stepsBeforeC0 + 1)))*innerRadius, center.y + sinf(DEG2RAD*(startAngle + stepLength*(float)(stepsBeforeC0 + 1)))*innerRadius);
                        rlVertex2f(c0.x, c0.y);

                        // Triangle touching C3
                        rlVertex2f(center.x, center.y);
                        rlVertex2f(c3.x, c3.y);
                        rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - stepLength*(float)(stepsBeforeC0 + 1)))*innerRadius, center.y + sinf(DEG2RAD*(endAngle - stepLength*(float)(stepsBeforeC0 + 1)))*innerRadius);

                        // Triangles between the previous two
                        verticesBetweenC0andC3 -= 1;
                        angle = startAngle + stepLength*(stepsBeforeC0 + 1);
                        for (int i = 0; i < verticesBetweenC0andC3; i++)
                        {
                            rlVertex2f(center.x, center.y);
                            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);
                            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);

                            angle += stepLength;
                        }
                    }
                }
            }
        }

    rlEnd();
#endif
}

// Draw circle outline
void DrawCircleLines(int centerX, int centerY, float radius, Color color)
{
    DrawCircleLinesV((Vector2){ (float)centerX, (float)centerY }, radius, color);
}

// Draw circle outline (Vector version)
void DrawCircleLinesV(Vector2 center, float radius, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // NOTE: Circle outline is drawn pixel by pixel every degree (0 to 360)
        for (int i = 0; i < 360; i += 10)
        {
            rlVertex2f(center.x + cosf(DEG2RAD*i)*radius, center.y + sinf(DEG2RAD*i)*radius);
            rlVertex2f(center.x + cosf(DEG2RAD*(i + 10))*radius, center.y + sinf(DEG2RAD*(i + 10))*radius);
        }
    rlEnd();
}

void DrawCircleLinesEx(Vector2 center, float radius, float thick, Color color)
{
    DrawRing(center, radius - thick, radius, 0.0f, 360.0f, 36, color);
}

// Draw ellipse
void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color)
{
    DrawEllipseV((Vector2){ (float)centerX, (float)centerY }, radiusH, radiusV, color);
}

// Draw ellipse (Vector version)
void DrawEllipseV(Vector2 center, float radiusH, float radiusV, Color color)
{
    rlBegin(RL_TRIANGLES);
        for (int i = 0; i < 360; i += 10)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f(center.x,  center.y);
            rlVertex2f(center.x + cosf(DEG2RAD*(i + 10))*radiusH, center.y + sinf(DEG2RAD*(i + 10))*radiusV);
            rlVertex2f(center.x + cosf(DEG2RAD*i)*radiusH, center.y + sinf(DEG2RAD*i)*radiusV);
        }
    rlEnd();
}

// Draw ellipse outline
void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color)
{
    DrawEllipseLinesV((Vector2){ (float)centerX, (float)centerY }, radiusH, radiusV, color);
}

// Draw ellipse outline
void DrawEllipseLinesV(Vector2 center, float radiusH, float radiusV, Color color)
{
    rlBegin(RL_LINES);
        for (int i = 0; i < 360; i += 10)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f(center.x + cosf(DEG2RAD*(i + 10))*radiusH, center.y + sinf(DEG2RAD*(i + 10))*radiusV);
            rlVertex2f(center.x + cosf(DEG2RAD*i)*radiusH, center.y + sinf(DEG2RAD*i)*radiusV);
        }
    rlEnd();
}

// Draw ellipse outline with thickness
void DrawEllipseLinesEx(Vector2 center, float radiusH, float radiusV, float thick, Color color)
{
    float outerRadiusH = radiusH, innerRadiusH = radiusH - thick;
    float outerRadiusV = radiusV, innerRadiusV = radiusV - thick;

    if (thick >= 0.0f) {
        // Just a filled-in ellipse
        if (innerRadiusH <= 0.0f || innerRadiusV <= 0.0f)
        {
            DrawEllipseV(center, radiusH, radiusV, color);
            return;
        }
    }
    else
    {
        // The outline is growing outside of the ellipse, so swap the inner and outer radius
        float tmp = outerRadiusH;
        outerRadiusH = innerRadiusH;
        innerRadiusH = tmp;

        tmp = outerRadiusV;
        outerRadiusV = innerRadiusV;
        innerRadiusV = tmp;
    }

#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);

        rlColor4ub(color.r, color.g, color.b, color.a);

        for (int i = 0; i < 360; i += 10)
        {
            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*i)*innerRadiusH, center.y + sinf(DEG2RAD*i)*innerRadiusV);

            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(i + 10))*innerRadiusH, center.y + sinf(DEG2RAD*(i + 10))*innerRadiusV);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(i + 10))*outerRadiusH, center.y + sinf(DEG2RAD*(i + 10))*outerRadiusV);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*i)*outerRadiusH, center.y + sinf(DEG2RAD*i)*outerRadiusV);
        }
    rlEnd();

    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);

        rlColor4ub(color.r, color.g, color.b, color.a);

        for (int i = 0; i < 360; i += 10)
        {
            rlVertex2f(center.x + cosf(DEG2RAD*i)*innerRadiusH, center.y + sinf(DEG2RAD*i)*innerRadiusV);
            rlVertex2f(center.x + cosf(DEG2RAD*(i + 10))*innerRadiusH, center.y + sinf(DEG2RAD*(i + 10))*innerRadiusV);
            rlVertex2f(center.x + cosf(DEG2RAD*(i + 10))*outerRadiusH, center.y + sinf(DEG2RAD*(i + 10))*outerRadiusV);

            rlVertex2f(center.x + cosf(DEG2RAD*i)*innerRadiusH, center.y + sinf(DEG2RAD*i)*innerRadiusV);
            rlVertex2f(center.x + cosf(DEG2RAD*(i + 10))*outerRadiusH, center.y + sinf(DEG2RAD*(i + 10))*outerRadiusV);
            rlVertex2f(center.x + cosf(DEG2RAD*i)*outerRadiusH, center.y + sinf(DEG2RAD*i)*outerRadiusV);
        }
    rlEnd();
#endif
}

// Draw ring
void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color)
{
    if (startAngle == endAngle) return;

    // Function expects (outerRadius > innerRadius)
    if (outerRadius < innerRadius)
    {
        float tmp = outerRadius;
        outerRadius = innerRadius;
        innerRadius = tmp;

        if (outerRadius <= 0.0f) outerRadius = 0.1f;
    }

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    // Drawing a whole circle, things get weird without limiting the circle to 360 degrees
    if (endAngle - startAngle >= 360.0f) endAngle = startAngle + 360.0f;

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/outerRadius, 2) - 1);
        segments = (int)ceilf((endAngle - startAngle)*(2*PI/th)/360.0f);

        if (segments <= 0) segments = minSegments;
    }

    // Not a ring
    if (innerRadius <= 0.0f)
    {
        DrawCircleSector(center, outerRadius, startAngle, endAngle, segments, color);
        return;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;

#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);
        for (int i = 0; i < segments; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);

            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*outerRadius);

            angle += stepLength;
        }
    rlEnd();

    rlSetTexture(0);
#else
    rlBegin(RL_TRIANGLES);
        for (int i = 0; i < segments; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);

            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*outerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);

            angle += stepLength;
        }
    rlEnd();
#endif
}

// Draw ring outline
void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color)
{
    if (startAngle == endAngle) return;

    // Function expects (outerRadius > innerRadius)
    if (outerRadius < innerRadius)
    {
        float tmp = outerRadius;
        outerRadius = innerRadius;
        innerRadius = tmp;

        if (outerRadius <= 0.0f) outerRadius = 0.1f;
    }

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    bool showCapLines = true;
    // Drawing a whole circle, things get weird without limiting the circle to 360 degrees
    if (endAngle - startAngle >= 360.0f)
    {
        showCapLines = false;
        endAngle = startAngle + 360.0f;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/outerRadius, 2) - 1);
        segments = (int)ceilf((endAngle - startAngle)*(2*PI/th)/360.0f);

        if (segments <= 0) segments = minSegments;
    }

    if (innerRadius <= 0.0f)
    {
        DrawCircleSectorLines(center, outerRadius, startAngle, endAngle, segments, color);
        return;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;

    rlBegin(RL_LINES);
        if (showCapLines)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);
        }

        for (int i = 0; i < segments; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*outerRadius);

            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerRadius);

            angle += stepLength;
        }

        if (showCapLines)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerRadius, center.y + sinf(DEG2RAD*angle)*outerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerRadius, center.y + sinf(DEG2RAD*angle)*innerRadius);
        }
    rlEnd();
}

// Draw ring outline with line thickness
void DrawRingLinesEx(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, float thick, Color color)
{
    if (startAngle == endAngle) return;

    // Function expects (outerRadius > innerRadius)
    if (outerRadius < innerRadius)
    {
        float tmp = outerRadius;
        outerRadius = innerRadius;
        innerRadius = tmp;

        if (outerRadius <= 0.0f) outerRadius = 0.1f;
    }

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    bool showCapLines = true;
    // Drawing a whole circle, things get weird without limiting the circle to 360 degrees
    if (endAngle - startAngle >= 360.0f)
    {
        showCapLines = thick >= 0.0f;
        endAngle = startAngle + 360.0f;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/outerRadius, 2) - 1);
        segments = (int)ceilf((endAngle - startAngle)*(2*PI/th)/360.0f);

        if (segments <= 0) segments = minSegments;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;

    // We are not drawing a circle, we are drawing an n-sided polygon
    // So, we need to adjust the outline thickness of the "circle" for it to look correct with fewer segments
    float apothem = outerRadius*cosf(DEG2RAD*((endAngle - startAngle)/2.0f)/(float)segments);
    float radiusThick = thick*(outerRadius/apothem);

    // These names can be confusing, but they are useful
    // Since 2 rings are being drawn, there are 4 radiuses (or radii)
    // "Inner" means closer to the center, "outer" means farther from the center
    // Sorted from farthest to closest you get:
    //   1. outerOuterRadius (farthest)
    //   2. innerOuterRadius
    //   3. outerInnerRadius
    //   4. innerInnerRadius (closest)
    float innerOuterRadius = 0.0f;
    float outerOuterRadius = 0.0f;
    float innerInnerRadius = 0.0f;
    float outerInnerRadius = 0.0f;

    if (thick >= 0.0f)
    {
        innerRadius = fmaxf(0.0f, innerRadius);

        // Just a filled-in ring
        if (radiusThick > (outerRadius - innerRadius)/2.0f)
        {
            DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, segments, color);
            return;
        }

        innerInnerRadius = innerRadius;
        outerInnerRadius = innerInnerRadius + radiusThick;

        outerOuterRadius = outerRadius;
        innerOuterRadius = outerOuterRadius - radiusThick;
    }
    else
    {
        // Just a circle sector outline
        if (innerRadius <= 0.0f)
        {
            DrawCircleSectorLinesEx(center, outerRadius, startAngle, endAngle, segments, thick, color);
            return;
        }

        outerInnerRadius = innerRadius;
        innerInnerRadius = fmaxf(0.0f, outerInnerRadius + radiusThick);

        innerOuterRadius = outerRadius;
        outerOuterRadius = innerOuterRadius - radiusThick;
    }

    // For positive `thick` values
    int stepsBeforeInner = 0;
    int stepsBeforeOuter = 0;
    float tInner = 0.0f;
    float tOuter = 0.0f;
    bool innerAnglesCrossEachOther = false;

    // For negative `thick` values
    Vector2 cap1SecondInnerVertex = { 0 };
    Vector2 cap1SecondOuterVertex = { 0 };
    Vector2 cap2SecondInnerVertex = { 0 };
    Vector2 cap2SecondOuterVertex = { 0 };
    bool capsIntersect = false;
    Vector2 capIntersectionVertex = { 0 };

    if (showCapLines)
    {
        if (thick >= 0.0f)
        {
            // Get the angle of the arc that has `thick` length along the inner and outer radii
            float cap1InnerAngleEnd = RAD2DEG*(thick/outerInnerRadius);
            float cap1OuterAngleEnd = RAD2DEG*(thick/innerOuterRadius);

            // Just a filled-in ring
            if (endAngle - startAngle < cap1OuterAngleEnd*2.0f)
            {
                DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, segments, color);
                return;
            }

            if (endAngle - startAngle < cap1InnerAngleEnd*2.0f) innerAnglesCrossEachOther = true;

            stepsBeforeInner = (int)(cap1InnerAngleEnd/stepLength);
            stepsBeforeOuter = (int)(cap1OuterAngleEnd/stepLength);

            // We need to find where `cap1InnerAngleEnd` intersects the edge defined
            // by `beforeInnerVertex` and `afterInnerVertex`
            //
            // We can make this easy by making `center` the origin (0, 0) and
            // making `cap1InnerAngleEnd` 0 degrees (a horizontal line)
            //
            // With that, we know these lines intersect when 'y' equals 0,
            // so we just need to solve for 't' (as in `Lerp(start, end, t)`)
            Vector2 beforeInnerVertex = { cosf(DEG2RAD*((float)stepsBeforeInner*stepLength - cap1InnerAngleEnd))*outerInnerRadius, sinf(DEG2RAD*((float)stepsBeforeInner*stepLength - cap1InnerAngleEnd))*outerInnerRadius };
            Vector2 afterInnerVertex = { cosf(DEG2RAD*((float)(stepsBeforeInner + 1)*stepLength - cap1InnerAngleEnd))*outerInnerRadius, sinf(DEG2RAD*((float)(stepsBeforeInner + 1)*stepLength - cap1InnerAngleEnd))*outerInnerRadius };
            tInner = beforeInnerVertex.y/(beforeInnerVertex.y - afterInnerVertex.y);

            // The same as above, but for the outer edge
            Vector2 beforeOuterVertex = { cosf(DEG2RAD*((float)stepsBeforeOuter*stepLength - cap1OuterAngleEnd))*innerOuterRadius, sinf(DEG2RAD*((float)stepsBeforeOuter*stepLength - cap1OuterAngleEnd))*innerOuterRadius };
            Vector2 afterOuterVertex = { cosf(DEG2RAD*((float)(stepsBeforeOuter + 1)*stepLength - cap1OuterAngleEnd))*innerOuterRadius, sinf(DEG2RAD*((float)(stepsBeforeOuter + 1)*stepLength - cap1OuterAngleEnd))*innerOuterRadius };
            tOuter = beforeOuterVertex.y/(beforeOuterVertex.y - afterOuterVertex.y);
        }
        else
        {
            // "Cap 1" is the outline on `startAngle` and "Cap 2" is the outline on `endAngle`

            /*
            A sketch to help make all this a little more understandable
            (This is an overly simplified representation of cap 1)

             I2[][][][][]O2  <- y = thick
             []          []
             []          []
             I0----------O0  <- angle = 0 degrees, y = 0
             []          []
             I1          O1  <- angle = stepLength

            Cap 2 is a mirror copy of cap 1, the inside and outside vertices switch sides

            We're using a frame of reference where `center` is (0, 0) and `startAngle` is 0 degrees

            I0 is `innerInnerRadius` distance from `center` at `starAngle`
            I1 is `innerInnerRadius` distance from `center` at `starAngle + stepLength`
            I2 goes out from I0 perpendicular to `startAngle`
            O0 is the same as I0, except using `outerOuterRadius` instead of `innerInnerRadius`
            O1 is the same as I1, except using `outerOuterRadius` instead of `innerInnerRadius`
            O2 is the same as I2, except goes out from O0

            The intersection cases between the caps edges are:
              1. No intersections, easy
              2. The I0->I2 and I2->O2 edges intersect between the caps
              3. The I0->I2 and O0->O2 edges intersect between the caps

            Notice that cap 1 and 2's I2->O2 and O0->O2 edges can't intersect at the same time,
            and, if there's any intersection, I0->I2 is one of the edges
            */

            Vector2 cap1O0 = { outerOuterRadius, 0.0f };
            Vector2 cap1O1 = { cosf(DEG2RAD*stepLength)*outerOuterRadius, sinf(DEG2RAD*stepLength)*outerOuterRadius };

            // Assuming a linear interpolation such as `value = Lerp(start, end, t)`
            // We can find O2 by getting its 't' between O1.y and O0.y (which is always greater than 1)
            // We can solve for `t` using `t = (start - value)/(start - end)`
            // Since we know `end = 0` we can simplify it to `t = (start - value)/start`
            float tOuter = (cap1O1.y - thick)/cap1O1.y;
            Vector2 cap1O2 = { cap1O1.x + (cap1O0.x - cap1O1.x)*tOuter, thick };

            //Vector2 cap1I0 = { innerInnerRadius, 0.0f }; // Not used

            float capLongEdgeLength = outerOuterRadius - innerInnerRadius;
            Vector2 cap1I2 = { cap1O2.x - capLongEdgeLength, thick };

            Vector2 cap2O0 = { cosf(DEG2RAD*(endAngle - startAngle))*outerOuterRadius, sinf(DEG2RAD*(endAngle - startAngle))*outerOuterRadius };
            Vector2 cap2O1 = { cosf(DEG2RAD*(endAngle - startAngle - stepLength))*outerOuterRadius, sinf(DEG2RAD*(endAngle - startAngle - stepLength))*outerOuterRadius };
            Vector2 cap2O2 = { cap2O1.x + (cap2O0.x - cap2O1.x)*tOuter, cap2O1.y + (cap2O0.y - cap2O1.y)*tOuter };

            Vector2 cap2I0 = { cosf(DEG2RAD*(endAngle - startAngle))*innerInnerRadius, sinf(DEG2RAD*(endAngle - startAngle))*innerInnerRadius };
            Vector2 cap2I2 = { cap2O2.x - cosf(DEG2RAD*(endAngle - startAngle))*capLongEdgeLength, cap2O2.y - sinf(DEG2RAD*(endAngle - startAngle))*capLongEdgeLength};

            // The 't' of the intersection between I2 and O2 (`Lerp(I2, O2, t)`)
            float tCapLongEdgeCross = -1.0f;
            // Avoid division by zero
            if (cap2I2.y - cap2O2.y != 0.0f)
            {
                // Find where the long edge of cap 2 intersects the long edge of cap 1
                tCapLongEdgeCross = (cap2I2.y - thick)/(cap2I2.y - cap2O2.y);
                if ((tCapLongEdgeCross >= 0.0f) && (tCapLongEdgeCross <= 1.0f)) capsIntersect = true;
            }

            // Rotate the frame of reference so that cap 1's I0->I2 edge is a vertical line
            float rotateBy = -DEG2RAD*stepLength/2.0f;

            // Copied from "raymath.h" Vector2Rotate()
            // Though we only use the x axis, so we ignore the y axis
            float cosres = cosf(rotateBy);
            float sinres = sinf(rotateBy);

            cap1I2.x = cap1I2.x*cosres - cap1I2.y*sinres;
            cap1O2.x = cap1O2.x*cosres - cap1O2.y*sinres;
            cap2I0.x = cap2I0.x*cosres - cap2I0.y*sinres;
            cap2I2.x = cap2I2.x*cosres - cap2I2.y*sinres;
            cap2O0.x = cap2O0.x*cosres - cap2O0.y*sinres;
            cap2O2.x = cap2O2.x*cosres - cap2O2.y*sinres;

            // The 't' of the intersection between I0 and I2 (`Lerp(I0, I2, t)`)
            float tCrossInner = -1.0f;
            // Avoid division by zero
            if (cap2I0.x - cap2I2.x != 0.0f) tCrossInner = (cap2I0.x - cap1I2.x)/(cap2I0.x - cap2I2.x);
            // Make sure `tCrossInner` is 0 when it should be (mitigate floating-point rounding woes)
            if (innerInnerRadius <= 0.0f) tCrossInner = 0.0f;

            // The 't' of the intersection between O0 and O2 (`Lerp(O0, O2, t)`)
            float tCrossOuter = -1.0f;
            // Avoid division by zero
            if (cap2O0.x - cap2O2.x != 0.0f) tCrossOuter = (cap2O0.x - cap1O2.x)/(cap2O0.x - cap2O2.x);

            // With our additional information, calculate the vertices we need
            // outside of our modified frame of reference

            cap1O0 = (Vector2){ center.x + cosf(DEG2RAD*startAngle)*outerOuterRadius, center.y + sinf(DEG2RAD*startAngle)*outerOuterRadius };
            cap1O1 = (Vector2){ center.x + cosf(DEG2RAD*(startAngle + stepLength))*outerOuterRadius, center.y + sinf(DEG2RAD*(startAngle + stepLength))*outerOuterRadius };
            cap1O2 = (Vector2){ cap1O1.x + (cap1O0.x - cap1O1.x)*tOuter, cap1O1.y + (cap1O0.y - cap1O1.y)*tOuter };

            cap2O0 = (Vector2){ center.x + cosf(DEG2RAD*endAngle)*outerOuterRadius, center.y + sinf(DEG2RAD*endAngle)*outerOuterRadius };
            cap2O1 = (Vector2){ center.x + cosf(DEG2RAD*(endAngle - stepLength))*outerOuterRadius, center.y + sinf(DEG2RAD*(endAngle - stepLength))*outerOuterRadius };
            cap2O2 = (Vector2){ cap2O1.x + (cap2O0.x - cap2O1.x)*tOuter, cap2O1.y + (cap2O0.y - cap2O1.y)*tOuter };

            //cap1I0 = (Vector2){ center.x + cosf(DEG2RAD*startAngle)*innerInnerRadius, center.y + sinf(DEG2RAD*startAngle)*innerInnerRadius };
            cap1I2 = (Vector2){ cap1O2.x - cosf(DEG2RAD*startAngle)*capLongEdgeLength, cap1O2.y - sinf(DEG2RAD*startAngle)*capLongEdgeLength };

            cap2I0 = (Vector2){ center.x + cosf(DEG2RAD*endAngle)*innerInnerRadius, center.y + sinf(DEG2RAD*endAngle)*innerInnerRadius };
            cap2I2 = (Vector2){ cap2O2.x - cosf(DEG2RAD*endAngle)*capLongEdgeLength, cap2O2.y - sinf(DEG2RAD*endAngle)*capLongEdgeLength };

            if (capsIntersect)
            {
                capIntersectionVertex = (Vector2){ cap2I2.x + (cap2O2.x - cap2I2.x)*tCapLongEdgeCross, cap2I2.y + (cap2O2.y - cap2I2.y)*tCapLongEdgeCross };

                cap2I2 = (Vector2){ cap2I0.x + (cap2I2.x - cap2I0.x)*tCrossInner, cap2I0.y + (cap2I2.y - cap2I0.y)*tCrossInner };
                cap1I2 = cap2I2;
            }
            else if ((tCrossOuter >= 0.0f) && (tCrossOuter <= 1.0f))
            {
                cap2O2 = (Vector2){ cap2O0.x + (cap2O2.x - cap2O0.x)*tCrossOuter, cap2O0.y + (cap2O2.y - cap2O0.y)*tCrossOuter };
                cap1O2 = cap2O2;

                cap2I2 = (Vector2){ cap2I0.x + (cap2I2.x - cap2I0.x)*tCrossInner, cap2I0.y + (cap2I2.y - cap2I0.y)*tCrossInner };
                cap1I2 = cap2I2;
            }

            cap1SecondInnerVertex = cap1I2;
            cap1SecondOuterVertex = cap1O2;
            cap2SecondInnerVertex = cap2I2;
            cap2SecondOuterVertex = cap2O2;
        }
    }

    float angle = startAngle;

#if SUPPORT_QUADS_DRAW_MODE
    rlSetTexture(GetShapesTexture().id);
    Rectangle shapeRect = GetShapesTextureRectangle();

    rlBegin(RL_QUADS);

        rlColor4ub(color.r, color.g, color.b, color.a);

        for (int i = 0; i < segments; i++)
        {
            // `innerRadius` outline
            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerInnerRadius, center.y + sinf(DEG2RAD*angle)*outerInnerRadius);

            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerInnerRadius, center.y + sinf(DEG2RAD*angle)*innerInnerRadius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerInnerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerInnerRadius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*outerInnerRadius);

            // `outerRadius` outline
            rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerOuterRadius, center.y + sinf(DEG2RAD*angle)*outerOuterRadius);

            rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerOuterRadius, center.y + sinf(DEG2RAD*angle)*innerOuterRadius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerOuterRadius);

            rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*outerOuterRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*outerOuterRadius);

            angle += stepLength;
        }

        if (showCapLines)
        {
            if (thick >= 0.0f)
            {
                angle = 0.0f;

                for (int i = 0; i < stepsBeforeOuter; i++)
                {
                    // Cap 1
                    rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*outerInnerRadius);

                    rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle + stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle + stepLength))*outerInnerRadius);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle + stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(startAngle + angle + stepLength))*innerOuterRadius);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle))*innerOuterRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*innerOuterRadius);

                    // Cap 2
                    rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*outerInnerRadius);

                    rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle))*innerOuterRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*innerOuterRadius);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle - stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(endAngle - angle - stepLength))*innerOuterRadius);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle - stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle - stepLength))*outerInnerRadius);

                    angle += stepLength;
                }

                // We've already moved `stepsBeforeOuter` steps from each end
                int totalStepsLeft = segments - stepsBeforeOuter*2;
                int innerStepsLeft = stepsBeforeInner - stepsBeforeOuter;

                // Cap 1
                Vector2 cap1OuterVertexBeforeEnd = { center.x + cosf(DEG2RAD*(startAngle + angle))*innerOuterRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*innerOuterRadius };
                Vector2 cap1OuterVertexAfterEnd = { center.x + cosf(DEG2RAD*(startAngle + angle + stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(startAngle + angle + stepLength))*innerOuterRadius };
                Vector2 cap1InnerVertexBeforeEnd = { center.x + cosf(DEG2RAD*(startAngle + angle + (float)innerStepsLeft*stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle + (float)innerStepsLeft*stepLength))*outerInnerRadius };
                Vector2 cap1InnerVertexAfterEnd = { center.x + cosf(DEG2RAD*(startAngle + angle + (float)(innerStepsLeft + 1)*stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle + (float)(innerStepsLeft + 1)*stepLength))*outerInnerRadius };
                Vector2 cap1InnerVertexEnd = { cap1InnerVertexBeforeEnd.x + (cap1InnerVertexAfterEnd.x - cap1InnerVertexBeforeEnd.x)*tInner, cap1InnerVertexBeforeEnd.y + (cap1InnerVertexAfterEnd.y - cap1InnerVertexBeforeEnd.y)*tInner };
                Vector2 cap1OuterVertexEnd = { cap1OuterVertexBeforeEnd.x + (cap1OuterVertexAfterEnd.x - cap1OuterVertexBeforeEnd.x)*tOuter, cap1OuterVertexBeforeEnd.y + (cap1OuterVertexAfterEnd.y - cap1OuterVertexBeforeEnd.y)*tOuter };

                // Cap 2
                Vector2 cap2OuterVertexBeforeEnd = { center.x + cosf(DEG2RAD*(endAngle - angle))*innerOuterRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*innerOuterRadius };
                Vector2 cap2OuterVertexAfterEnd = { center.x + cosf(DEG2RAD*(endAngle - angle - stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(endAngle - angle - stepLength))*innerOuterRadius };
                Vector2 cap2InnerVertexBeforeEnd = { center.x + cosf(DEG2RAD*(endAngle - angle - (float)innerStepsLeft*stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle - (float)innerStepsLeft*stepLength))*outerInnerRadius };
                Vector2 cap2InnerVertexAfterEnd = { center.x + cosf(DEG2RAD*(endAngle - angle - (float)(innerStepsLeft + 1)*stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle - (float)(innerStepsLeft + 1)*stepLength))*outerInnerRadius };
                Vector2 cap2InnerVertexEnd = { cap2InnerVertexBeforeEnd.x + (cap2InnerVertexAfterEnd.x - cap2InnerVertexBeforeEnd.x)*tInner, cap2InnerVertexBeforeEnd.y + (cap2InnerVertexAfterEnd.y - cap2InnerVertexBeforeEnd.y)*tInner };
                Vector2 cap2OuterVertexEnd = { cap2OuterVertexBeforeEnd.x + (cap2OuterVertexAfterEnd.x - cap2OuterVertexBeforeEnd.x)*tOuter, cap2OuterVertexBeforeEnd.y + (cap2OuterVertexAfterEnd.y - cap2OuterVertexBeforeEnd.y)*tOuter };

                int stepsCount = (innerAnglesCrossEachOther)? totalStepsLeft/2 : innerStepsLeft;

                // Iterate over pairs of steps
                for (int i = 0; i < stepsCount/2; i++)
                {
                    // Cap 1
                    rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);

                    rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*outerInnerRadius);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle + stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle + stepLength))*outerInnerRadius);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle + stepLength*2.0f))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle + stepLength*2.0f))*outerInnerRadius);

                    // Cap 2
                    rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);

                    rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle - stepLength*2.0f))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle - stepLength*2.0f))*outerInnerRadius);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle - stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle - stepLength))*outerInnerRadius);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*outerInnerRadius);

                    angle += stepLength*2.0f;
                }

                // Handle the last step if there's an odd amount
                if (stepsCount%2 == 1)
                {
                    // Cap 1
                    rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);

                    rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*outerInnerRadius);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle + stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle + stepLength))*outerInnerRadius);

                    // Cap 2
                    rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);

                    rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle - stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle - stepLength))*outerInnerRadius);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*outerInnerRadius);

                    angle += stepLength;
                }

                // When the inner angles coming from `startAngle` and `endAngle` cross each other,
                // the `*innerVertexEnd` vertices go past each other and cause the geometry to intersect itself
                if (innerAnglesCrossEachOther)
                {
                    // We need to find where the line defined by `cap1InnerVertexEnd` and `cap1OuterVertexEnd` intersects
                    // the line defined by `cap2InnerVertexEnd` and `cap2OuterVertexEnd`
                    // That point is then used instead to prevent the outline from intersecting itself

                    // Make `cap1InnerVertexEnd` the origin and the angle to `cap1OuterVertexEnd` 0 degrees
                    Vector2 tempCap1OuterVertexEnd = { cap1OuterVertexEnd.x - cap1InnerVertexEnd.x, cap1OuterVertexEnd.y - cap1InnerVertexEnd.y };
                    Vector2 tempCap2InnerVertexEnd = { cap2InnerVertexEnd.x - cap1InnerVertexEnd.x, cap2InnerVertexEnd.y - cap1InnerVertexEnd.y };
                    Vector2 tempCap2OuterVertexEnd = { cap2OuterVertexEnd.x - cap1InnerVertexEnd.x, cap2OuterVertexEnd.y - cap1InnerVertexEnd.y };

                    float rotateBy = -atan2f(tempCap1OuterVertexEnd.y, tempCap1OuterVertexEnd.x);
                    // We only need the y coordinates, so only rotate the y coordinates
                    float start = sinf(rotateBy)*tempCap2InnerVertexEnd.x + cosf(rotateBy)*tempCap2InnerVertexEnd.y;
                    float end = sinf(rotateBy)*tempCap2OuterVertexEnd.x + cosf(rotateBy)*tempCap2OuterVertexEnd.y;
                    float tCross = start/(start - end);

                    Vector2 intersection = { cap2InnerVertexEnd.x + (cap2OuterVertexEnd.x - cap2InnerVertexEnd.x)*tCross, cap2InnerVertexEnd.y + (cap2OuterVertexEnd.y - cap2InnerVertexEnd.y)*tCross };

                    if (segments%2 == 0)
                    {
                        // There are an even number of segments, so there's 1 vertex exactly in the middle

                        Vector2 middleInnerVertex = { center.x + cosf(DEG2RAD*(startAngle + angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*outerInnerRadius };

                        // Cap 1
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(intersection.x, intersection.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(cap1OuterVertexEnd.x, cap1OuterVertexEnd.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(middleInnerVertex.x, middleInnerVertex.y);

                        // Cap 2
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(intersection.x, intersection.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(middleInnerVertex.x, middleInnerVertex.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(cap2OuterVertexEnd.x, cap2OuterVertexEnd.y);
                    }
                    else
                    {
                        // There are an odd number of segments, so there are 2 vertices in the middle

                        Vector2 middleInnerVertex1 = { center.x + cosf(DEG2RAD*(startAngle + angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*outerInnerRadius };
                        Vector2 middleInnerVertex2 = { center.x + cosf(DEG2RAD*(endAngle - angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*outerInnerRadius };

                        // Cap 1
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(intersection.x, intersection.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(cap1OuterVertexEnd.x, cap1OuterVertexEnd.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(middleInnerVertex1.x, middleInnerVertex1.y);

                        // Cap 2
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(intersection.x, intersection.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(middleInnerVertex2.x, middleInnerVertex2.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(cap2OuterVertexEnd.x, cap2OuterVertexEnd.y);

                        // Triangle between the caps
                        rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(intersection.x, intersection.y);

                        rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(intersection.x, intersection.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                        rlVertex2f(middleInnerVertex1.x, middleInnerVertex1.y);

                        rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                        rlVertex2f(middleInnerVertex2.x, middleInnerVertex2.y);
                    }
                }
                else
                {
                    // Cap 1
                    rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);

                    rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(cap1InnerVertexBeforeEnd.x, cap1InnerVertexBeforeEnd.y);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(cap1InnerVertexEnd.x, cap1InnerVertexEnd.y);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(cap1OuterVertexEnd.x, cap1OuterVertexEnd.y);

                    // Cap 2
                    rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);

                    rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(cap2OuterVertexEnd.x, cap2OuterVertexEnd.y);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(cap2InnerVertexEnd.x, cap2InnerVertexEnd.y);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(cap2InnerVertexBeforeEnd.x, cap2InnerVertexBeforeEnd.y);
                }
            }
            else
            {
                // Cap 1
                Vector2 cap1FirstInnerVertex = { center.x + cosf(DEG2RAD*startAngle)*innerInnerRadius, center.y + sinf(DEG2RAD*startAngle)*innerInnerRadius };
                Vector2 cap1FirstOuterVertex = { center.x + cosf(DEG2RAD*startAngle)*outerOuterRadius, center.y + sinf(DEG2RAD*startAngle)*outerOuterRadius };

                rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(cap1FirstInnerVertex.x, cap1FirstInnerVertex.y);

                rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(cap1FirstOuterVertex.x, cap1FirstOuterVertex.y);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(cap1SecondOuterVertex.x, cap1SecondOuterVertex.y);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(cap1SecondInnerVertex.x, cap1SecondInnerVertex.y);

                // Cap 2
                Vector2 cap2FirstInnerVertex = { center.x + cosf(DEG2RAD*endAngle)*innerInnerRadius, center.y + sinf(DEG2RAD*endAngle)*innerInnerRadius };
                Vector2 cap2FirstOuterVertex = { center.x + cosf(DEG2RAD*endAngle)*outerOuterRadius, center.y + sinf(DEG2RAD*endAngle)*outerOuterRadius };

                rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(cap2FirstInnerVertex.x, cap2FirstInnerVertex.y);

                rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(cap2SecondInnerVertex.x, cap2SecondInnerVertex.y);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                rlVertex2f(cap2SecondOuterVertex.x, cap2SecondOuterVertex.y);

                rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                rlVertex2f(cap2FirstOuterVertex.x, cap2FirstOuterVertex.y);

                if (capsIntersect)
                {
                    // Cap 1
                    rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(cap1SecondInnerVertex.x, cap1SecondInnerVertex.y);

                    rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(cap1SecondInnerVertex.x, cap1SecondInnerVertex.y);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(cap1SecondOuterVertex.x, cap1SecondOuterVertex.y);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(capIntersectionVertex.x, capIntersectionVertex.y);

                    // Cap 2
                    rlTexCoord2f(shapeRect.x/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(cap2SecondInnerVertex.x, cap2SecondInnerVertex.y);

                    rlTexCoord2f(shapeRect.x/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(capIntersectionVertex.x, capIntersectionVertex.y);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, shapeRect.y/texShapes.height);
                    rlVertex2f(cap2SecondOuterVertex.x, cap2SecondOuterVertex.y);

                    rlTexCoord2f((shapeRect.x + shapeRect.width)/texShapes.width, (shapeRect.y + shapeRect.height)/texShapes.height);
                    rlVertex2f(cap2SecondInnerVertex.x, cap2SecondInnerVertex.y);
                }
            }
        }
    rlEnd();
#else
    rlBegin(RL_TRIANGLES);

        rlColor4ub(color.r, color.g, color.b, color.a);

        for (int i = 0; i < segments; i++)
        {
            // `innerRadius` outline
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerInnerRadius, center.y + sinf(DEG2RAD*angle)*outerInnerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerInnerRadius, center.y + sinf(DEG2RAD*angle)*innerInnerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerInnerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerInnerRadius);

            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerInnerRadius, center.y + sinf(DEG2RAD*angle)*outerInnerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerInnerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerInnerRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*outerInnerRadius);

            // `outerRadius` outline
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerOuterRadius, center.y + sinf(DEG2RAD*angle)*outerOuterRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*angle)*innerOuterRadius, center.y + sinf(DEG2RAD*angle)*innerOuterRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerOuterRadius);

            rlVertex2f(center.x + cosf(DEG2RAD*angle)*outerOuterRadius, center.y + sinf(DEG2RAD*angle)*outerOuterRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*innerOuterRadius);
            rlVertex2f(center.x + cosf(DEG2RAD*(angle + stepLength))*outerOuterRadius, center.y + sinf(DEG2RAD*(angle + stepLength))*outerOuterRadius);

            angle += stepLength;
        }

        if (showCapLines)
        {
            if (thick >= 0.0f)
            {
                angle = 0.0f;

                for (int i = 0; i < stepsBeforeOuter; i++)
                {
                    // Cap 1
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*outerInnerRadius);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle + stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle + stepLength))*outerInnerRadius);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle + stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(startAngle + angle + stepLength))*innerOuterRadius);

                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*outerInnerRadius);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle + stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(startAngle + angle + stepLength))*innerOuterRadius);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle))*innerOuterRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*innerOuterRadius);

                    // Cap 2
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*outerInnerRadius);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle))*innerOuterRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*innerOuterRadius);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle - stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(endAngle - angle - stepLength))*innerOuterRadius);

                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*outerInnerRadius);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle - stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(endAngle - angle - stepLength))*innerOuterRadius);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle - stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle - stepLength))*outerInnerRadius);

                    angle += stepLength;
                }

                // We've already moved `stepsBeforeOuter` steps from each end
                int totalStepsLeft = segments - stepsBeforeOuter*2;
                int innerStepsLeft = stepsBeforeInner - stepsBeforeOuter;

                // Cap 1
                Vector2 cap1OuterVertexBeforeEnd = { center.x + cosf(DEG2RAD*(startAngle + angle))*innerOuterRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*innerOuterRadius };
                Vector2 cap1OuterVertexAfterEnd = { center.x + cosf(DEG2RAD*(startAngle + angle + stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(startAngle + angle + stepLength))*innerOuterRadius };
                Vector2 cap1InnerVertexBeforeEnd = { center.x + cosf(DEG2RAD*(startAngle + angle + (float)innerStepsLeft*stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle + (float)innerStepsLeft*stepLength))*outerInnerRadius };
                Vector2 cap1InnerVertexAfterEnd = { center.x + cosf(DEG2RAD*(startAngle + angle + (float)(innerStepsLeft + 1)*stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle + (float)(innerStepsLeft + 1)*stepLength))*outerInnerRadius };
                Vector2 cap1InnerVertexEnd = { cap1InnerVertexBeforeEnd.x + (cap1InnerVertexAfterEnd.x - cap1InnerVertexBeforeEnd.x)*tInner, cap1InnerVertexBeforeEnd.y + (cap1InnerVertexAfterEnd.y - cap1InnerVertexBeforeEnd.y)*tInner };
                Vector2 cap1OuterVertexEnd = { cap1OuterVertexBeforeEnd.x + (cap1OuterVertexAfterEnd.x - cap1OuterVertexBeforeEnd.x)*tOuter, cap1OuterVertexBeforeEnd.y + (cap1OuterVertexAfterEnd.y - cap1OuterVertexBeforeEnd.y)*tOuter };

                // Cap 2
                Vector2 cap2OuterVertexBeforeEnd = { center.x + cosf(DEG2RAD*(endAngle - angle))*innerOuterRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*innerOuterRadius };
                Vector2 cap2OuterVertexAfterEnd = { center.x + cosf(DEG2RAD*(endAngle - angle - stepLength))*innerOuterRadius, center.y + sinf(DEG2RAD*(endAngle - angle - stepLength))*innerOuterRadius };
                Vector2 cap2InnerVertexBeforeEnd = { center.x + cosf(DEG2RAD*(endAngle - angle - (float)innerStepsLeft*stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle - (float)innerStepsLeft*stepLength))*outerInnerRadius };
                Vector2 cap2InnerVertexAfterEnd = { center.x + cosf(DEG2RAD*(endAngle - angle - (float)(innerStepsLeft + 1)*stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle - (float)(innerStepsLeft + 1)*stepLength))*outerInnerRadius };
                Vector2 cap2InnerVertexEnd = { cap2InnerVertexBeforeEnd.x + (cap2InnerVertexAfterEnd.x - cap2InnerVertexBeforeEnd.x)*tInner, cap2InnerVertexBeforeEnd.y + (cap2InnerVertexAfterEnd.y - cap2InnerVertexBeforeEnd.y)*tInner };
                Vector2 cap2OuterVertexEnd = { cap2OuterVertexBeforeEnd.x + (cap2OuterVertexAfterEnd.x - cap2OuterVertexBeforeEnd.x)*tOuter, cap2OuterVertexBeforeEnd.y + (cap2OuterVertexAfterEnd.y - cap2OuterVertexBeforeEnd.y)*tOuter };

                int stepsCount = (innerAnglesCrossEachOther)? totalStepsLeft/2 : innerStepsLeft;

                for (int i = 0; i < stepsCount; i++)
                {
                    // Cap 1
                    rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*outerInnerRadius);
                    rlVertex2f(center.x + cosf(DEG2RAD*(startAngle + angle + stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle + stepLength))*outerInnerRadius);

                    // Cap 2
                    rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle - stepLength))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle - stepLength))*outerInnerRadius);
                    rlVertex2f(center.x + cosf(DEG2RAD*(endAngle - angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*outerInnerRadius);

                    angle += stepLength;
                }

                // When the inner angles coming from `startAngle` and `endAngle` cross each other,
                // the `*innerVertexEnd` vertices go past each other and cause the geometry to intersect itself
                if (innerAnglesCrossEachOther)
                {
                    // We need to find where the line defined by `cap1InnerVertexEnd` and `cap1OuterVertexEnd` intersects
                    // the line defined by `cap2InnerVertexEnd` and `cap2OuterVertexEnd`
                    // That point is then used instead to prevent the outline from intersecting itself

                    // Make `cap1InnerVertexEnd` the origin and the angle to `cap1OuterVertexEnd` 0 degrees
                    Vector2 tempCap1OuterVertexEnd = { cap1OuterVertexEnd.x - cap1InnerVertexEnd.x, cap1OuterVertexEnd.y - cap1InnerVertexEnd.y };
                    Vector2 tempCap2InnerVertexEnd = { cap2InnerVertexEnd.x - cap1InnerVertexEnd.x, cap2InnerVertexEnd.y - cap1InnerVertexEnd.y };
                    Vector2 tempCap2OuterVertexEnd = { cap2OuterVertexEnd.x - cap1InnerVertexEnd.x, cap2OuterVertexEnd.y - cap1InnerVertexEnd.y };

                    float rotateBy = -atan2f(tempCap1OuterVertexEnd.y, tempCap1OuterVertexEnd.x);
                    // We only need the y coordinates, so only rotate the y coordinates
                    float start = sinf(rotateBy)*tempCap2InnerVertexEnd.x + cosf(rotateBy)*tempCap2InnerVertexEnd.y;
                    float end = sinf(rotateBy)*tempCap2OuterVertexEnd.x + cosf(rotateBy)*tempCap2OuterVertexEnd.y;
                    float tCross = start/(start - end);

                    Vector2 intersection = { cap2InnerVertexEnd.x + (cap2OuterVertexEnd.x - cap2InnerVertexEnd.x)*tCross, cap2InnerVertexEnd.y + (cap2OuterVertexEnd.y - cap2InnerVertexEnd.y)*tCross };

                    if (segments%2 == 0)
                    {
                        // There are an even number of segments, so there's 1 vertex exactly in the middle

                        Vector2 middleInnerVertex = { center.x + cosf(DEG2RAD*(startAngle + angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*outerInnerRadius };

                        // Cap 1
                        rlVertex2f(intersection.x, intersection.y);
                        rlVertex2f(cap1OuterVertexEnd.x, cap1OuterVertexEnd.y);
                        rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);

                        rlVertex2f(intersection.x, intersection.y);
                        rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);
                        rlVertex2f(middleInnerVertex.x, middleInnerVertex.y);

                        // Cap 2
                        rlVertex2f(intersection.x, intersection.y);
                        rlVertex2f(middleInnerVertex.x, middleInnerVertex.y);
                        rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);

                        rlVertex2f(intersection.x, intersection.y);
                        rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);
                        rlVertex2f(cap2OuterVertexEnd.x, cap2OuterVertexEnd.y);
                    }
                    else
                    {
                        // There are an odd number of segments, so there are 2 vertices in the middle

                        Vector2 middleInnerVertex1 = { center.x + cosf(DEG2RAD*(startAngle + angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(startAngle + angle))*outerInnerRadius };
                        Vector2 middleInnerVertex2 = { center.x + cosf(DEG2RAD*(endAngle - angle))*outerInnerRadius, center.y + sinf(DEG2RAD*(endAngle - angle))*outerInnerRadius };

                        // Cap 1
                        rlVertex2f(intersection.x, intersection.y);
                        rlVertex2f(cap1OuterVertexEnd.x, cap1OuterVertexEnd.y);
                        rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);

                        rlVertex2f(intersection.x, intersection.y);
                        rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);
                        rlVertex2f(middleInnerVertex1.x, middleInnerVertex1.y);

                        // Cap 2
                        rlVertex2f(intersection.x, intersection.y);
                        rlVertex2f(middleInnerVertex2.x, middleInnerVertex2.y);
                        rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);

                        rlVertex2f(intersection.x, intersection.y);
                        rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);
                        rlVertex2f(cap2OuterVertexEnd.x, cap2OuterVertexEnd.y);

                        // Triangle between the caps
                        rlVertex2f(intersection.x, intersection.y);
                        rlVertex2f(intersection.x, intersection.y);
                        rlVertex2f(middleInnerVertex1.x, middleInnerVertex1.y);

                        rlVertex2f(intersection.x, intersection.y);
                        rlVertex2f(middleInnerVertex1.x, middleInnerVertex1.y);
                        rlVertex2f(middleInnerVertex2.x, middleInnerVertex2.y);
                    }
                }
                else
                {
                    // Cap 1
                    rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);
                    rlVertex2f(cap1InnerVertexBeforeEnd.x, cap1InnerVertexBeforeEnd.y);
                    rlVertex2f(cap1InnerVertexEnd.x, cap1InnerVertexEnd.y);

                    rlVertex2f(cap1OuterVertexBeforeEnd.x, cap1OuterVertexBeforeEnd.y);
                    rlVertex2f(cap1InnerVertexEnd.x, cap1InnerVertexEnd.y);
                    rlVertex2f(cap1OuterVertexEnd.x, cap1OuterVertexEnd.y);

                    // Cap 2
                    rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);
                    rlVertex2f(cap2OuterVertexEnd.x, cap2OuterVertexEnd.y);
                    rlVertex2f(cap2InnerVertexEnd.x, cap2InnerVertexEnd.y);

                    rlVertex2f(cap2OuterVertexBeforeEnd.x, cap2OuterVertexBeforeEnd.y);
                    rlVertex2f(cap2InnerVertexEnd.x, cap2InnerVertexEnd.y);
                    rlVertex2f(cap2InnerVertexBeforeEnd.x, cap2InnerVertexBeforeEnd.y);
                }
            }
            else
            {
                // Cap 1
                Vector2 cap1FirstInnerVertex = { center.x + cosf(DEG2RAD*startAngle)*innerInnerRadius, center.y + sinf(DEG2RAD*startAngle)*innerInnerRadius };
                Vector2 cap1FirstOuterVertex = { center.x + cosf(DEG2RAD*startAngle)*outerOuterRadius, center.y + sinf(DEG2RAD*startAngle)*outerOuterRadius };

                rlVertex2f(cap1FirstInnerVertex.x, cap1FirstInnerVertex.y);
                rlVertex2f(cap1FirstOuterVertex.x, cap1FirstOuterVertex.y);
                rlVertex2f(cap1SecondOuterVertex.x, cap1SecondOuterVertex.y);

                rlVertex2f(cap1FirstInnerVertex.x, cap1FirstInnerVertex.y);
                rlVertex2f(cap1SecondOuterVertex.x, cap1SecondOuterVertex.y);
                rlVertex2f(cap1SecondInnerVertex.x, cap1SecondInnerVertex.y);

                // Cap 2
                Vector2 cap2FirstInnerVertex = { center.x + cosf(DEG2RAD*endAngle)*innerInnerRadius, center.y + sinf(DEG2RAD*endAngle)*innerInnerRadius };
                Vector2 cap2FirstOuterVertex = { center.x + cosf(DEG2RAD*endAngle)*outerOuterRadius, center.y + sinf(DEG2RAD*endAngle)*outerOuterRadius };

                rlVertex2f(cap2FirstInnerVertex.x, cap2FirstInnerVertex.y);
                rlVertex2f(cap2SecondInnerVertex.x, cap2SecondInnerVertex.y);
                rlVertex2f(cap2SecondOuterVertex.x, cap2SecondOuterVertex.y);

                rlVertex2f(cap2FirstInnerVertex.x, cap2FirstInnerVertex.y);
                rlVertex2f(cap2SecondOuterVertex.x, cap2SecondOuterVertex.y);
                rlVertex2f(cap2FirstOuterVertex.x, cap2FirstOuterVertex.y);

                if (capsIntersect)
                {
                    // Cap 1
                    rlVertex2f(cap1SecondInnerVertex.x, cap1SecondInnerVertex.y);
                    rlVertex2f(cap1SecondOuterVertex.x, cap1SecondOuterVertex.y);
                    rlVertex2f(capIntersectionVertex.x, capIntersectionVertex.y);

                    // Cap 2
                    rlVertex2f(cap2SecondInnerVertex.x, cap2SecondInnerVertex.y);
                    rlVertex2f(capIntersectionVertex.x, capIntersectionVertex.y);
                    rlVertex2f(cap2SecondOuterVertex.x, cap2SecondOuterVertex.y);
                }
            }
        }

    rlEnd();
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Splines functions
//----------------------------------------------------------------------------------

// Draw spline: linear, minimum 2 points
void DrawSplineLinear(const Vector2 *points, int pointCount, float thick, Color color)
{
    if (pointCount < 2) return;

#if SUPPORT_SPLINE_MITERS
    Vector2 prevNormal = (Vector2){-(points[1].y - points[0].y), (points[1].x - points[0].x)};
    float prevLength = sqrtf(prevNormal.x*prevNormal.x + prevNormal.y*prevNormal.y);

    if (prevLength > 0.0f)
    {
        prevNormal.x /= prevLength;
        prevNormal.y /= prevLength;
    }
    else
    {
        prevNormal.x = 0.0f;
        prevNormal.y = 0.0f;
    }

    Vector2 prevRadius = { 0.5f*thick*prevNormal.x, 0.5f*thick*prevNormal.y };

    for (int i = 0; i < pointCount - 1; i++)
    {
        Vector2 normal = { 0 };

        if (i < pointCount - 2)
        {
            normal = (Vector2){-(points[i + 2].y - points[i + 1].y), (points[i + 2].x - points[i + 1].x)};
            float normalLength = sqrtf(normal.x*normal.x + normal.y*normal.y);

            if (normalLength > 0.0f)
            {
                normal.x /= normalLength;
                normal.y /= normalLength;
            }
            else
            {
                normal.x = 0.0f;
                normal.y = 0.0f;
            }
        }
        else
        {
            normal = prevNormal;
        }

        Vector2 radius = { prevNormal.x + normal.x, prevNormal.y + normal.y };
        float radiusLength = sqrtf(radius.x*radius.x + radius.y*radius.y);

        if (radiusLength > 0.0f)
        {
            radius.x /= radiusLength;
            radius.y /= radiusLength;
        }
        else
        {
            radius.x = 0.0f;
            radius.y = 0.0f;
        }

        float cosTheta = radius.x*normal.x + radius.y*normal.y;

        if (cosTheta != 0.0f)
        {
            radius.x *= (thick*0.5f/cosTheta);
            radius.y *= (thick*0.5f/cosTheta);
        }
        else
        {
            radius.x = 0.0f;
            radius.y = 0.0f;
        }

        Vector2 strip[4] = {
            { points[i].x - prevRadius.x, points[i].y - prevRadius.y },
            { points[i].x + prevRadius.x, points[i].y + prevRadius.y },
            { points[i + 1].x - radius.x, points[i + 1].y - radius.y },
            { points[i + 1].x + radius.x, points[i + 1].y + radius.y }
        };

        DrawTriangleStrip(strip, 4, color);

        prevRadius = radius;
        prevNormal = normal;
    }

#else   // !SUPPORT_SPLINE_MITERS

    Vector2 delta = { 0 };
    float length = 0.0f;
    float scale = 0.0f;

    for (int i = 0; i < pointCount - 1; i++)
    {
        delta = (Vector2){ points[i + 1].x - points[i].x, points[i + 1].y - points[i].y };
        length = sqrtf(delta.x*delta.x + delta.y*delta.y);

        if (length > 0) scale = thick/(2*length);

        Vector2 radius = { -scale*delta.y, scale*delta.x };
        Vector2 strip[4] = {
            { points[i].x - radius.x, points[i].y - radius.y },
            { points[i].x + radius.x, points[i].y + radius.y },
            { points[i + 1].x - radius.x, points[i + 1].y - radius.y },
            { points[i + 1].x + radius.x, points[i + 1].y + radius.y }
        };

        DrawTriangleStrip(strip, 4, color);
    }
#endif

#if SUPPORT_SPLINE_SEGMENT_CAPS
    // TODO: Add spline segment rounded caps at the begin/end of the spline?
#endif
}

// Draw spline: B-Spline, minimum 4 points
void DrawSplineBasis(const Vector2 *points, int pointCount, float thick, Color color)
{
    if (pointCount < 4) return;

    float a[4] = { 0 };
    float b[4] = { 0 };
    float dy = 0.0f;
    float dx = 0.0f;
    float size = 0.0f;

    Vector2 currentPoint = { 0 };
    Vector2 nextPoint = { 0 };
    Vector2 vertices[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    for (int i = 0; i < (pointCount - 3); i++)
    {
        float t = 0.0f;
        Vector2 p1 = points[i], p2 = points[i + 1], p3 = points[i + 2], p4 = points[i + 3];

        a[0] = (-p1.x + 3.0f*p2.x - 3.0f*p3.x + p4.x)/6.0f;
        a[1] = (3.0f*p1.x - 6.0f*p2.x + 3.0f*p3.x)/6.0f;
        a[2] = (-3.0f*p1.x + 3.0f*p3.x)/6.0f;
        a[3] = (p1.x + 4.0f*p2.x + p3.x)/6.0f;

        b[0] = (-p1.y + 3.0f*p2.y - 3.0f*p3.y + p4.y)/6.0f;
        b[1] = (3.0f*p1.y - 6.0f*p2.y + 3.0f*p3.y)/6.0f;
        b[2] = (-3.0f*p1.y + 3.0f*p3.y)/6.0f;
        b[3] = (p1.y + 4.0f*p2.y + p3.y)/6.0f;

        currentPoint.x = a[3];
        currentPoint.y = b[3];

        if (i == 0) DrawCircleV(currentPoint, thick/2.0f, color);   // Draw init line circle-cap

        if (i > 0)
        {
            vertices[0].x = currentPoint.x + dy*size;
            vertices[0].y = currentPoint.y - dx*size;
            vertices[1].x = currentPoint.x - dy*size;
            vertices[1].y = currentPoint.y + dx*size;
        }

        for (int j = 1; j <= SPLINE_SEGMENT_DIVISIONS; j++)
        {
            t = ((float)j)/((float)SPLINE_SEGMENT_DIVISIONS);

            nextPoint.x = a[3] + t*(a[2] + t*(a[1] + t*a[0]));
            nextPoint.y = b[3] + t*(b[2] + t*(b[1] + t*b[0]));

            dy = nextPoint.y - currentPoint.y;
            dx = nextPoint.x - currentPoint.x;
            size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

            if ((i == 0) && (j == 1))
            {
                vertices[0].x = currentPoint.x + dy*size;
                vertices[0].y = currentPoint.y - dx*size;
                vertices[1].x = currentPoint.x - dy*size;
                vertices[1].y = currentPoint.y + dx*size;
            }

            vertices[2*j + 1].x = nextPoint.x - dy*size;
            vertices[2*j + 1].y = nextPoint.y + dx*size;
            vertices[2*j].x = nextPoint.x + dy*size;
            vertices[2*j].y = nextPoint.y - dx*size;

            currentPoint = nextPoint;
        }

        DrawTriangleStrip(vertices, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
    }

    // Cap circle drawing at the end of every segment
    DrawCircleV(currentPoint, thick/2.0f, color);
}

// Draw spline: Catmull-Rom, minimum 4 points
void DrawSplineCatmullRom(const Vector2 *points, int pointCount, float thick, Color color)
{
    if (pointCount < 4) return;

    float dy = 0.0f;
    float dx = 0.0f;
    float size = 0.0f;

    Vector2 currentPoint = points[1];
    Vector2 nextPoint = { 0 };
    Vector2 vertices[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    DrawCircleV(currentPoint, thick/2.0f, color);   // Draw init line circle-cap

    for (int i = 0; i < (pointCount - 3); i++)
    {
        float t = 0.0f;
        Vector2 p1 = points[i], p2 = points[i + 1], p3 = points[i + 2], p4 = points[i + 3];

        if (i > 0)
        {
            vertices[0].x = currentPoint.x + dy*size;
            vertices[0].y = currentPoint.y - dx*size;
            vertices[1].x = currentPoint.x - dy*size;
            vertices[1].y = currentPoint.y + dx*size;
        }

        for (int j = 1; j <= SPLINE_SEGMENT_DIVISIONS; j++)
        {
            t = ((float)j)/((float)SPLINE_SEGMENT_DIVISIONS);

            float q0 = (-1.0f*t*t*t) + (2.0f*t*t) + (-1.0f*t);
            float q1 = (3.0f*t*t*t) + (-5.0f*t*t) + 2.0f;
            float q2 = (-3.0f*t*t*t) + (4.0f*t*t) + t;
            float q3 = t*t*t - t*t;

            nextPoint.x = 0.5f*((p1.x*q0) + (p2.x*q1) + (p3.x*q2) + (p4.x*q3));
            nextPoint.y = 0.5f*((p1.y*q0) + (p2.y*q1) + (p3.y*q2) + (p4.y*q3));

            dy = nextPoint.y - currentPoint.y;
            dx = nextPoint.x - currentPoint.x;
            size = (0.5f*thick)/sqrtf(dx*dx + dy*dy);

            if ((i == 0) && (j == 1))
            {
                vertices[0].x = currentPoint.x + dy*size;
                vertices[0].y = currentPoint.y - dx*size;
                vertices[1].x = currentPoint.x - dy*size;
                vertices[1].y = currentPoint.y + dx*size;
            }

            vertices[2*j + 1].x = nextPoint.x - dy*size;
            vertices[2*j + 1].y = nextPoint.y + dx*size;
            vertices[2*j].x = nextPoint.x + dy*size;
            vertices[2*j].y = nextPoint.y - dx*size;

            currentPoint = nextPoint;
        }

        DrawTriangleStrip(vertices, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
    }

    // Cap circle drawing at the end of every segment
    DrawCircleV(currentPoint, thick/2.0f, color);
}

// Draw spline: Quadratic Bezier, minimum 3 points (1 control point): [p1, c2, p3, c4...]
void DrawSplineBezierQuadratic(const Vector2 *points, int pointCount, float thick, Color color)
{
    if (pointCount >= 3)
    {
        for (int i = 0; i < pointCount - 2; i += 2) DrawSplineSegmentBezierQuadratic(points[i], points[i + 1], points[i + 2], thick, color);

        // Cap circle drawing at the end of every segment
        //for (int i = 2; i < pointCount - 2; i += 2) DrawCircleV(points[i], thick/2.0f, color);
    }
}

// Draw spline: Cubic Bezier, minimum 4 points (2 control points): [p1, c2, c3, p4, c5, c6...]
void DrawSplineBezierCubic(const Vector2 *points, int pointCount, float thick, Color color)
{
    if (pointCount >= 4)
    {
        for (int i = 0; i < pointCount - 3; i += 3) DrawSplineSegmentBezierCubic(points[i], points[i + 1], points[i + 2], points[i + 3], thick, color);

        // Cap circle drawing at the end of every segment
        //for (int i = 3; i < pointCount - 3; i += 3) DrawCircleV(points[i], thick/2.0f, color);
    }
}

// Draw spline segment: Linear, 2 points
void DrawSplineSegmentLinear(Vector2 p1, Vector2 p2, float thick, Color color)
{
    // NOTE: For the linear spline no subdivisions are used, only a single quad

    Vector2 delta = { p2.x - p1.x, p2.y - p1.y };
    float length = sqrtf(delta.x*delta.x + delta.y*delta.y);

    if ((length > 0) && (thick > 0))
    {
        float scale = thick/(2*length);

        Vector2 radius = { -scale*delta.y, scale*delta.x };
        Vector2 strip[4] = {
            { p1.x - radius.x, p1.y - radius.y },
            { p1.x + radius.x, p1.y + radius.y },
            { p2.x - radius.x, p2.y - radius.y },
            { p2.x + radius.x, p2.y + radius.y }
        };

        DrawTriangleStrip(strip, 4, color);
    }
}

// Draw spline segment: B-Spline, 4 points
void DrawSplineSegmentBasis(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float thick, Color color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    Vector2 currentPoint = { 0 };
    Vector2 nextPoint = { 0 };
    float t = 0.0f;

    Vector2 points[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    float a[4] = { 0 };
    float b[4] = { 0 };

    a[0] = (-p1.x + 3*p2.x - 3*p3.x + p4.x)/6.0f;
    a[1] = (3*p1.x - 6*p2.x + 3*p3.x)/6.0f;
    a[2] = (-3*p1.x + 3*p3.x)/6.0f;
    a[3] = (p1.x + 4*p2.x + p3.x)/6.0f;

    b[0] = (-p1.y + 3*p2.y - 3*p3.y + p4.y)/6.0f;
    b[1] = (3*p1.y - 6*p2.y + 3*p3.y)/6.0f;
    b[2] = (-3*p1.y + 3*p3.y)/6.0f;
    b[3] = (p1.y + 4*p2.y + p3.y)/6.0f;

    currentPoint.x = a[3];
    currentPoint.y = b[3];

    for (int i = 0; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        nextPoint.x = a[3] + t*(a[2] + t*(a[1] + t*a[0]));
        nextPoint.y = b[3] + t*(b[2] + t*(b[1] + t*b[0]));

        float dy = nextPoint.y - currentPoint.y;
        float dx = nextPoint.x - currentPoint.x;
        float size = (0.5f*thick)/sqrtf(dx*dx + dy*dy);

        if (i == 1)
        {
            points[0].x = currentPoint.x + dy*size;
            points[0].y = currentPoint.y - dx*size;
            points[1].x = currentPoint.x - dy*size;
            points[1].y = currentPoint.y + dx*size;
        }

        points[2*i + 1].x = nextPoint.x - dy*size;
        points[2*i + 1].y = nextPoint.y + dx*size;
        points[2*i].x = nextPoint.x + dy*size;
        points[2*i].y = nextPoint.y - dx*size;

        currentPoint = nextPoint;
    }

    DrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS+2, color);
}

// Draw spline segment: Catmull-Rom, 4 points
void DrawSplineSegmentCatmullRom(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float thick, Color color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    Vector2 currentPoint = p1;
    Vector2 nextPoint = { 0 };
    float t = 0.0f;

    Vector2 points[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    for (int i = 0; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        float q0 = (-1*t*t*t) + (2*t*t) + (-1*t);
        float q1 = (3*t*t*t) + (-5*t*t) + 2;
        float q2 = (-3*t*t*t) + (4*t*t) + t;
        float q3 = t*t*t - t*t;

        nextPoint.x = 0.5f*((p1.x*q0) + (p2.x*q1) + (p3.x*q2) + (p4.x*q3));
        nextPoint.y = 0.5f*((p1.y*q0) + (p2.y*q1) + (p3.y*q2) + (p4.y*q3));

        float dy = nextPoint.y - currentPoint.y;
        float dx = nextPoint.x - currentPoint.x;
        float size = (0.5f*thick)/sqrtf(dx*dx + dy*dy);

        if (i == 1)
        {
            points[0].x = currentPoint.x + dy*size;
            points[0].y = currentPoint.y - dx*size;
            points[1].x = currentPoint.x - dy*size;
            points[1].y = currentPoint.y + dx*size;
        }

        points[2*i + 1].x = nextPoint.x - dy*size;
        points[2*i + 1].y = nextPoint.y + dx*size;
        points[2*i].x = nextPoint.x + dy*size;
        points[2*i].y = nextPoint.y - dx*size;

        currentPoint = nextPoint;
    }

    DrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
}

// Draw spline segment: Quadratic Bezier, 2 points, 1 control point
void DrawSplineSegmentBezierQuadratic(Vector2 p1, Vector2 c2, Vector2 p3, float thick, Color color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    Vector2 previous = p1;
    Vector2 current = { 0 };
    float t = 0.0f;

    Vector2 points[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    for (int i = 1; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        float a = powf(1.0f - t, 2);
        float b = 2.0f*(1.0f - t)*t;
        float c = powf(t, 2);

        // NOTE: The easing functions aren't suitable here because they don't take a control point
        current.y = a*p1.y + b*c2.y + c*p3.y;
        current.x = a*p1.x + b*c2.x + c*p3.x;

        float dy = current.y - previous.y;
        float dx = current.x - previous.x;
        float size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

        if (i == 1)
        {
            points[0].x = previous.x + dy*size;
            points[0].y = previous.y - dx*size;
            points[1].x = previous.x - dy*size;
            points[1].y = previous.y + dx*size;
        }

        points[2*i + 1].x = current.x - dy*size;
        points[2*i + 1].y = current.y + dx*size;
        points[2*i].x = current.x + dy*size;
        points[2*i].y = current.y - dx*size;

        previous = current;
    }

    DrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
}

// Draw spline segment: Cubic Bezier, 2 points, 2 control points
void DrawSplineSegmentBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3, Vector2 p4, float thick, Color color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    Vector2 previous = p1;
    Vector2 current = { 0 };
    float t = 0.0f;

    Vector2 points[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    for (int i = 1; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        float a = powf(1.0f - t, 3);
        float b = 3.0f*powf(1.0f - t, 2)*t;
        float c = 3.0f*(1.0f - t)*powf(t, 2);
        float d = powf(t, 3);

        current.y = a*p1.y + b*c2.y + c*c3.y + d*p4.y;
        current.x = a*p1.x + b*c2.x + c*c3.x + d*p4.x;

        float dy = current.y - previous.y;
        float dx = current.x - previous.x;
        float size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

        if (i == 1)
        {
            points[0].x = previous.x + dy*size;
            points[0].y = previous.y - dx*size;
            points[1].x = previous.x - dy*size;
            points[1].y = previous.y + dx*size;
        }

        points[2*i + 1].x = current.x - dy*size;
        points[2*i + 1].y = current.y + dx*size;
        points[2*i].x = current.x + dy*size;
        points[2*i].y = current.y - dx*size;

        previous = current;
    }

    DrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
}

// Get spline point for a given t [0.0f .. 1.0f], Linear
Vector2 GetSplinePointLinear(Vector2 startPos, Vector2 endPos, float t)
{
    Vector2 point = { 0 };

    point.x = startPos.x*(1.0f - t) + endPos.x*t;
    point.y = startPos.y*(1.0f - t) + endPos.y*t;

    return point;
}

// Get spline point for a given t [0.0f .. 1.0f], B-Spline
Vector2 GetSplinePointBasis(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float t)
{
    Vector2 point = { 0 };

    float a[4] = { 0 };
    float b[4] = { 0 };

    a[0] = (-p1.x + 3*p2.x - 3*p3.x + p4.x)/6.0f;
    a[1] = (3*p1.x - 6*p2.x + 3*p3.x)/6.0f;
    a[2] = (-3*p1.x + 3*p3.x)/6.0f;
    a[3] = (p1.x + 4*p2.x + p3.x)/6.0f;

    b[0] = (-p1.y + 3*p2.y - 3*p3.y + p4.y)/6.0f;
    b[1] = (3*p1.y - 6*p2.y + 3*p3.y)/6.0f;
    b[2] = (-3*p1.y + 3*p3.y)/6.0f;
    b[3] = (p1.y + 4*p2.y + p3.y)/6.0f;

    point.x = a[3] + t*(a[2] + t*(a[1] + t*a[0]));
    point.y = b[3] + t*(b[2] + t*(b[1] + t*b[0]));

    return point;
}

// Get spline point for a given t [0.0f .. 1.0f], Catmull-Rom
Vector2 GetSplinePointCatmullRom(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float t)
{
    Vector2 point = { 0 };

    float q0 = (-1*t*t*t) + (2*t*t) + (-1*t);
    float q1 = (3*t*t*t) + (-5*t*t) + 2;
    float q2 = (-3*t*t*t) + (4*t*t) + t;
    float q3 = t*t*t - t*t;

    point.x = 0.5f*((p1.x*q0) + (p2.x*q1) + (p3.x*q2) + (p4.x*q3));
    point.y = 0.5f*((p1.y*q0) + (p2.y*q1) + (p3.y*q2) + (p4.y*q3));

    return point;
}

// Get spline point for a given t [0.0f .. 1.0f], Quadratic Bezier
Vector2 GetSplinePointBezierQuadratic(Vector2 startPos, Vector2 controlPos, Vector2 endPos, float t)
{
    Vector2 point = { 0 };

    float a = powf(1.0f - t, 2);
    float b = 2.0f*(1.0f - t)*t;
    float c = powf(t, 2);

    point.y = a*startPos.y + b*controlPos.y + c*endPos.y;
    point.x = a*startPos.x + b*controlPos.x + c*endPos.x;

    return point;
}

// Get spline point for a given t [0.0f .. 1.0f], Cubic Bezier
Vector2 GetSplinePointBezierCubic(Vector2 startPos, Vector2 startControlPos, Vector2 endControlPos, Vector2 endPos, float t)
{
    Vector2 point = { 0 };

    float a = powf(1.0f - t, 3);
    float b = 3.0f*powf(1.0f - t, 2)*t;
    float c = 3.0f*(1.0f - t)*powf(t, 2);
    float d = powf(t, 3);

    point.y = a*startPos.y + b*startControlPos.y + c*endControlPos.y + d*endPos.y;
    point.x = a*startPos.x + b*startControlPos.x + c*endControlPos.x + d*endPos.x;

    return point;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Collision Detection functions
//----------------------------------------------------------------------------------

// Check if point is inside rectangle
bool CheckCollisionPointRec(Vector2 point, Rectangle rec)
{
    bool collision = false;

    if ((point.x >= rec.x) && (point.x < (rec.x + rec.width)) && (point.y >= rec.y) && (point.y < (rec.y + rec.height))) collision = true;

    return collision;
}

// Check if point is inside circle
bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius)
{
    bool collision = false;

    float distanceSquared = (point.x - center.x)*(point.x - center.x) + (point.y - center.y)*(point.y - center.y);

    if (distanceSquared <= radius*radius) collision = true;

    return collision;
}

// Check if point is inside a triangle defined by three points (p1, p2, p3)
bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3)
{
    bool collision = false;

    float alpha = ((p2.y - p3.y)*(point.x - p3.x) + (p3.x - p2.x)*(point.y - p3.y)) /
                  ((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));

    float beta = ((p3.y - p1.y)*(point.x - p3.x) + (p1.x - p3.x)*(point.y - p3.y)) /
                 ((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));

    float gamma = 1.0f - alpha - beta;

    if ((alpha > 0) && (beta > 0) && (gamma > 0)) collision = true;

    return collision;
}

// Check if point is within a polygon described by array of vertices
// NOTE: Based on http://jeffreythompson.org/collision-detection/poly-point.php
bool CheckCollisionPointPoly(Vector2 point, const Vector2 *points, int pointCount)
{
    bool collision = false;

    if (pointCount > 2)
    {
        for (int i = 0, j = pointCount - 1; i < pointCount; j = i++)
        {
            if ((points[i].y > point.y) != (points[j].y > point.y) &&
                (point.x < (points[j].x - points[i].x)*(point.y - points[i].y)/(points[j].y - points[i].y) + points[i].x))
            {
                collision = !collision;
            }
        }
    }

    return collision;
}

// Check collision between two rectangles
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2)
{
    bool collision = false;

    if ((rec1.x < (rec2.x + rec2.width) && (rec1.x + rec1.width) > rec2.x) &&
        (rec1.y < (rec2.y + rec2.height) && (rec1.y + rec1.height) > rec2.y)) collision = true;

    return collision;
}

// Check collision between two circles
bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2)
{
    bool collision = false;

    float dx = center2.x - center1.x;      // X distance between centers
    float dy = center2.y - center1.y;      // Y distance between centers

    float distanceSquared = dx*dx + dy*dy; // Distance between centers squared
    float radiusSum = radius1 + radius2;

    collision = (distanceSquared <= (radiusSum*radiusSum));

    return collision;
}

// Check collision between circle and rectangle
// NOTE: Reviewed version to take into account corner limit case
bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec)
{
    bool collision = false;

    float recCenterX = rec.x + rec.width/2.0f;
    float recCenterY = rec.y + rec.height/2.0f;

    float dx = fabsf(center.x - recCenterX);
    float dy = fabsf(center.y - recCenterY);

    if ((dx <= (rec.width/2.0f + radius)) && (dy <= (rec.height/2.0f + radius)))
    {
        if (dx <= (rec.width/2.0f)) collision = true;
        else if (dy <= (rec.height/2.0f)) collision = true;
        else
        {
            float cornerDistanceSq = (dx - rec.width/2.0f)*(dx - rec.width/2.0f) +
                (dy - rec.height/2.0f)*(dy - rec.height/2.0f);

            collision = (cornerDistanceSq <= (radius*radius));
        }
    }

    return collision;
}

// Check the collision between two lines defined by two points each, returns collision point by reference
// REF: https://en.wikipedia.org/wiki/Line–line_intersection#Given_two_points_on_each_line_segment
bool CheckCollisionLines(Vector2 startPos1, Vector2 endPos1, Vector2 startPos2, Vector2 endPos2, Vector2 *collisionPoint)
{
    bool collision = false;

    float rx = endPos1.x - startPos1.x;
    float ry = endPos1.y - startPos1.y;
    float sx = endPos2.x - startPos2.x;
    float sy = endPos2.y - startPos2.y;

    float div = rx*sy - ry*sx;

    if (fabsf(div) >= FLT_EPSILON)
    {
        float s12x = startPos2.x - startPos1.x;
        float s12y = startPos2.y - startPos1.y;

        float t = (s12x*sy - s12y*sx)/div;
        float u = (s12x*ry - s12y*rx)/div;

        if ((0.0f <= t) && (t <= 1.0f) && (0.0f <= u) && (u <= 1.0f))
        {
            if (collisionPoint)
            {
                collisionPoint->x = startPos1.x + t*rx;
                collisionPoint->y = startPos1.y + t*ry;
            }

            collision = true;
        }
    }

    return collision;
}

// Check if point belongs to line created between two points [p1] and [p2] with defined margin in pixels [threshold]
bool CheckCollisionPointLine(Vector2 point, Vector2 p1, Vector2 p2, int threshold)
{
    bool collision = false;

    float dxc = point.x - p1.x;
    float dyc = point.y - p1.y;
    float dxl = p2.x - p1.x;
    float dyl = p2.y - p1.y;
    float cross = dxc*dyl - dyc*dxl;

    if (fabsf(cross) < (threshold*fmaxf(fabsf(dxl), fabsf(dyl))))
    {
        if (fabsf(dxl) >= fabsf(dyl)) collision = (dxl > 0)? ((p1.x <= point.x) && (point.x <= p2.x)) : ((p2.x <= point.x) && (point.x <= p1.x));
        else collision = (dyl > 0)? ((p1.y <= point.y) && (point.y <= p2.y)) : ((p2.y <= point.y) && (point.y <= p1.y));
    }

    return collision;
}

// Check if circle collides with a line created between two points [p1] and [p2]
bool CheckCollisionCircleLine(Vector2 center, float radius, Vector2 p1, Vector2 p2)
{
    bool collision = false;

    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;

    if ((fabsf(dx) + fabsf(dy)) <= FLT_EPSILON)
    {
        collision = CheckCollisionCircles(p1, 0, center, radius);
    }
    else
    {
        float lengthSQ = ((dx*dx) + (dy*dy));
        float dotProduct = (((center.x - p1.x)*(p2.x - p1.x)) + ((center.y - p1.y)*(p2.y - p1.y)))/(lengthSQ);

        if (dotProduct > 1.0f) dotProduct = 1.0f;
        else if (dotProduct < 0.0f) dotProduct = 0.0f;

        float dx2 = (p1.x - (dotProduct*(dx))) - center.x;
        float dy2 = (p1.y - (dotProduct*(dy))) - center.y;
        float distanceSQ = ((dx2*dx2) + (dy2*dy2));

        if (distanceSQ <= radius*radius) collision = true;
    }

    return collision;
}

// Get collision rectangle for two rectangles collision
Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2)
{
    Rectangle overlap = { 0 };

    float left = (rec1.x > rec2.x)? rec1.x : rec2.x;
    float right1 = rec1.x + rec1.width;
    float right2 = rec2.x + rec2.width;
    float right = (right1 < right2)? right1 : right2;
    float top = (rec1.y > rec2.y)? rec1.y : rec2.y;
    float bottom1 = rec1.y + rec1.height;
    float bottom2 = rec2.y + rec2.height;
    float bottom = (bottom1 < bottom2)? bottom1 : bottom2;

    if ((left < right) && (top < bottom))
    {
        overlap.x = left;
        overlap.y = top;
        overlap.width = right - left;
        overlap.height = bottom - top;
    }

    return overlap;
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Cubic easing in-out
// NOTE: Used by DrawLineBezier() only
static float EaseCubicInOut(float t, float b, float c, float d)
{
    float result = 0.0f;

    if ((t /= 0.5f*d) < 1) result = 0.5f*c*t*t*t + b;
    else
    {
        t -= 2;
        result = 0.5f*c*(t*t*t + 2.0f) + b;
    }

    return result;
}

#endif // SUPPORT_MODULE_RSHAPES
