/**********************************************************************************************
*
*   raylib.shapes
*
*   Basic functions to draw 2d Shapes and check collisions
*
*   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
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

#include <stdlib.h>     // Required for abs() function
#include <math.h>       // Math related functions, sin() and cos() used on DrawCircle*
                        // sqrt() and pow() and abs() used on CheckCollision*

#include "rlgl.h"       // raylib OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Not here...

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

// Draw a pixel
void DrawPixel(int posX, int posY, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2i(posX, posY);
        rlVertex2i(posX + 1, posY + 1);
    rlEnd();
}

// Draw a pixel (Vector version)
void DrawPixelV(Vector2 position, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(position.x, position.y);
        rlVertex2i(position.x + 1, position.y + 1);
    rlEnd();
}

// Draw a line
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2i(startPosX, startPosY);
        rlVertex2i(endPosX, endPosY);
    rlEnd();
}

// Draw a line  (Vector version)
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(startPos.x, startPos.y);
        rlVertex2f(endPos.x, endPos.y);
    rlEnd();
}

// Draw a color-filled circle
void DrawCircle(int centerX, int centerY, float radius, Color color)
{
    DrawPoly((Vector2){centerX, centerY}, 360, radius, 0, color);
}

// Draw a gradient-filled circle
// NOTE: Gradient goes from center (color1) to border (color2)
void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2)
{
    rlBegin(RL_TRIANGLES);
        for (int i=0; i < 360; i += 2)
        {
            rlColor4ub(color1.r, color1.g, color1.b, color1.a);
            rlVertex2i(centerX, centerY);
            rlColor4ub(color2.r, color2.g, color2.b, color2.a);
            rlVertex2f(centerX + sin(DEG2RAD*i) * radius, centerY + cos(DEG2RAD*i) * radius);
            rlColor4ub(color2.r, color2.g, color2.b, color2.a);
            rlVertex2f(centerX + sin(DEG2RAD*(i+2)) * radius, centerY + cos(DEG2RAD*(i+2)) * radius);
        }
    rlEnd();
}

// Draw a color-filled circle (Vector version)
void DrawCircleV(Vector2 center, float radius, Color color)
{
    rlBegin(RL_TRIANGLES);
        for (int i=0; i < 360; i += 2)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2i(center.x, center.y);
            rlVertex2f(center.x + sin(DEG2RAD*i) * radius, center.y + cos(DEG2RAD*i) * radius);
            rlVertex2f(center.x + sin(DEG2RAD*(i+2)) * radius, center.y + cos(DEG2RAD*(i+2)) * radius);
        }
    rlEnd();
}

// Draw circle outline
void DrawCircleLines(int centerX, int centerY, float radius, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // NOTE: Circle outline is drawn pixel by pixel every degree (0 to 360)
        for (int i=0; i < 360; i++)
        {
            rlVertex2f(centerX + sin(DEG2RAD*i) * radius, centerY + cos(DEG2RAD*i) * radius);
            rlVertex2f(centerX + sin(DEG2RAD*(i+1)) * radius, centerY + cos(DEG2RAD*(i+1)) * radius);
        }
   rlEnd();
}

// Draw a color-filled rectangle
void DrawRectangle(int posX, int posY, int width, int height, Color color)
{
    Vector2 position = { (float)posX, (float)posY };
    Vector2 size = { (float)width, (float)height };

    DrawRectangleV(position, size, color);
}

// Draw a color-filled rectangle
void DrawRectangleRec(Rectangle rec, Color color)
{
    DrawRectangle(rec.x, rec.y, rec.width, rec.height, color);
}

// Draw a gradient-filled rectangle
// NOTE: Gradient goes from bottom (color1) to top (color2)
void DrawRectangleGradient(int posX, int posY, int width, int height, Color color1, Color color2)
{
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color1.r, color1.g, color1.b, color1.a); rlVertex2i(posX, posY);
        rlColor4ub(color2.r, color2.g, color2.b, color2.a); rlVertex2i(posX, posY + height);
        rlColor4ub(color2.r, color2.g, color2.b, color2.a); rlVertex2i(posX + width, posY + height);

        rlColor4ub(color1.r, color1.g, color1.b, color1.a); rlVertex2i(posX, posY);
        rlColor4ub(color2.r, color2.g, color2.b, color2.a); rlVertex2i(posX + width, posY + height);
        rlColor4ub(color1.r, color1.g, color1.b, color1.a); rlVertex2i(posX + width, posY);
    rlEnd();
}

// Draw a color-filled rectangle (Vector version)
void DrawRectangleV(Vector2 position, Vector2 size, Color color)
{
    if (rlGetVersion() == OPENGL_11)
    {
        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlVertex2i(position.x, position.y);
            rlVertex2i(position.x, position.y + size.y);
            rlVertex2i(position.x + size.x, position.y + size.y);

            rlVertex2i(position.x, position.y);
            rlVertex2i(position.x + size.x, position.y + size.y);
            rlVertex2i(position.x + size.x, position.y);
        rlEnd();
    }
    else if ((rlGetVersion() == OPENGL_33) || (rlGetVersion() == OPENGL_ES_20))
    {
        // NOTE: This shape uses QUADS to avoid drawing order issues (view rlglDraw)
        rlEnableTexture(1); // Default white texture

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlNormal3f(0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer

            rlTexCoord2f(0.0f, 0.0f);
            rlVertex2f(position.x, position.y);

            rlTexCoord2f(0.0f, 1.0f);
            rlVertex2f(position.x, position.y + size.y);

            rlTexCoord2f(1.0f, 1.0f);
            rlVertex2f(position.x + size.x, position.y + size.y);

            rlTexCoord2f(1.0f, 0.0f);
            rlVertex2f(position.x + size.x, position.y);
        rlEnd();

        rlDisableTexture();
    }
}

// Draw rectangle outline
void DrawRectangleLines(int posX, int posY, int width, int height, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2i(posX + 1, posY + 1);
        rlVertex2i(posX + width, posY + 1);

        rlVertex2i(posX + width, posY + 1);
        rlVertex2i(posX + width, posY + height);

        rlVertex2i(posX + width, posY + height);
        rlVertex2i(posX + 1, posY + height);

        rlVertex2i(posX + 1, posY + height);
        rlVertex2i(posX + 1, posY + 1);
    rlEnd();
}

// Draw a triangle
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(v1.x, v1.y);
        rlVertex2f(v2.x, v2.y);
        rlVertex2f(v3.x, v3.y);
    rlEnd();
}

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

// Draw a regular polygon of n sides (Vector version)
void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color)
{
    if (sides < 3) sides = 3;

    rlPushMatrix();
        rlTranslatef(center.x, center.y, 0.0);
        rlRotatef(rotation, 0, 0, 1);

        rlBegin(RL_TRIANGLES);
            for (int i=0; i < 360; i += 360/sides)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);

                rlVertex2i(0, 0);
                rlVertex2f(sin(DEG2RAD*i) * radius, cos(DEG2RAD*i) * radius);
                rlVertex2f(sin(DEG2RAD*(i+360/sides)) * radius, cos(DEG2RAD*(i+360/sides)) * radius);
            }
        rlEnd();
    rlPopMatrix();
}

// Draw a closed polygon defined by points
// NOTE: Array num elements MUST be passed as parameter to function
void DrawPolyEx(Vector2 *points, int numPoints, Color color)
{
    if (numPoints >= 3)
    {
        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < numPoints - 2; i++)
            {
                rlVertex2f(points[i].x, points[i].y);
                rlVertex2f(points[i+1].x, points[i+1].y);
                rlVertex2f(points[i+2].x, points[i+2].y);
            }
        rlEnd();
    }
}

// Draw polygon lines
// NOTE: Array num elements MUST be passed as parameter to function
void DrawPolyExLines(Vector2 *points, int numPoints, Color color)
{
    if (numPoints >= 2)
    {
        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < numPoints - 1; i++)
            {
                rlVertex2f(points[i].x, points[i].y);
                rlVertex2f(points[i+1].x, points[i+1].y);
            }
        rlEnd();
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Collision Detection functions
//----------------------------------------------------------------------------------

// Check if point is inside rectangle
bool CheckCollisionPointRec(Vector2 point, Rectangle rec)
{
    bool collision = false;

    if ((point.x >= rec.x) && (point.x <= (rec.x + rec.width)) && (point.y >= rec.y) && (point.y <= (rec.y + rec.height))) collision = true;

    return collision;
}

// Check if point is inside circle
bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius)
{
    return CheckCollisionCircles(point, 0, center, radius);
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

    if ((alpha > 0) && (beta > 0) & (gamma > 0)) collision = true;

    return collision;
}

// Check collision between two rectangles
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2)
{
    bool collision = false;

    int dx = abs((rec1.x + rec1.width / 2) - (rec2.x + rec2.width / 2));
    int dy = abs((rec1.y + rec1.height / 2) - (rec2.y + rec2.height / 2));

    if ((dx <= (rec1.width / 2 + rec2.width / 2)) && ((dy <= (rec1.height / 2 + rec2.height / 2)))) collision = true;

    return collision;
}

// Check collision between two circles
bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2)
{
    bool collision = false;

    float dx = center2.x - center1.x;      // X distance between centers
    float dy = center2.y - center1.y;      // Y distance between centers

    float distance = sqrt(dx*dx + dy*dy);  // Distance between centers

    if (distance <= (radius1 + radius2)) collision = true;

    return collision;
}

// Check collision between circle and rectangle
bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec)
{
    bool collision = false;

    float dx = abs((rec.x + rec.width / 2) - center.x);
    float dy = abs((rec.y + rec.height / 2) - center.y);

    if ((dx <= (rec.width / 2 + radius)) && (dy <= (rec.height / 2 + radius))) collision = true;

    return collision;
}

// Get collision rectangle for two rectangles collision
Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2)
{
    Rectangle retRec = { 0, 0, 0, 0 };

    if (CheckCollisionRecs(rec1, rec2))
    {
        int dxx = abs(rec1.x - rec2.x);
        int dyy = abs(rec1.y - rec2.y);

        if (rec1.x <= rec2.x)
        {
            if (rec1.y <= rec2.y)
            {
                retRec.x = rec2.x;
                retRec.y = rec2.y;
                retRec.width = rec1.width - dxx;
                retRec.height = rec1.height - dyy;
            }
            else
            {
                retRec.x = rec2.x;
                retRec.y = rec1.y;
                retRec.width = rec1.width - dxx;
                retRec.height = rec2.height - dyy;
            }
        }
        else
        {
            if (rec1.y <= rec2.y)
            {
                retRec.x = rec1.x;
                retRec.y = rec2.y;
                retRec.width = rec2.width - dxx;
                retRec.height = rec1.height - dyy;
            }
            else
            {
                retRec.x = rec1.x;
                retRec.y = rec1.y;
                retRec.width = rec2.width - dxx;
                retRec.height = rec2.height - dyy;
            }
        }

        if (retRec.width >= rec2.width) retRec.width = rec2.width;
        if (retRec.height >= rec2.height) retRec.height = rec2.height;
    }

    return retRec;
}