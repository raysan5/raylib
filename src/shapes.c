/*********************************************************************************************
*
*   raylib.shapes
*
*   Basic functions to draw 2d Shapes and check collisions
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

#include <GL/gl.h>      // OpenGL functions
#include <stdlib.h>     // Required for abs() function
#include <math.h>       // Math related functions, sin() and cos() used on DrawCircle*
                        // sqrt() and pow() and abs() used on CheckCollision*

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
    glBegin(GL_POINTS);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2i(posX, posY);
    glEnd();

    // NOTE1: Alternative method to draw a pixel (GL_LINES)
/*
    glBegin(GL_LINES);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2i(posX, posY);
        glVertex2i(posX+1, posY+1);
    glEnd();
*/    
    // NOTE2: Alternative method to draw a pixel (glPoint())
/*
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);    // Deprecated on OGL 3.0
    
    glPointSize(1.0f);
    glPoint((float)posX, (float)posY, 0.0f);
*/
}

// Draw a pixel (Vector version)
void DrawPixelV(Vector2 position, Color color)
{
    glBegin(GL_POINTS);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2f(position.x, position.y);
    glEnd();
}

// Draw a line
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color)
{
    glBegin(GL_LINES);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2i(startPosX, startPosY);
        glVertex2i(endPosX, endPosY);
    glEnd();
}

// Draw a line  (Vector version)
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color)
{
    glBegin(GL_LINES);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2f(startPos.x, startPos.y);
        glVertex2f(endPos.x, endPos.y);
    glEnd();
}

// Draw a color-filled circle
// TODO: Review, on some GPUs is drawn with a weird transparency (GL_POLYGON_SMOOTH issue?)
void DrawCircle(int centerX, int centerY, float radius, Color color)
{
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);  // Deprecated on OGL 3.0
    
    DrawPoly((Vector2){centerX, centerY}, 360, radius, 0, color);
    
    glDisable(GL_POLYGON_SMOOTH);
    
    // NOTE: Alternative method to draw a circle (point)
/*
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);    // Deprecated on OGL 3.0
    
    glPointSize(radius);
    glPoint((float)centerX, (float)centerY, 0.0f);
*/
}

// Draw a gradient-filled circle
// NOTE: Gradient goes from center (color1) to border (color2)
void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2)
{
    glBegin(GL_TRIANGLE_FAN);
        glColor4ub(color1.r, color1.g, color1.b, color1.a);
        glVertex2i(centerX, centerY);
        glColor4ub(color2.r, color2.g, color2.b, color2.a);
        
        for (int i=0; i <= 360; i++)        //i++ --> Step = 1.0 pixels
        {
            glVertex2f(centerX + sin(DEG2RAD*i) * radius, centerY + cos(DEG2RAD*i) * radius);
        }
    glEnd();
}

// Draw a color-filled circle (Vector version)
void DrawCircleV(Vector2 center, float radius, Color color)
{
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    
    glBegin(GL_TRIANGLE_FAN);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2f(center.x, center.y);

        for (int i=0; i <= 360; i++)        //i++ --> Step = 1.0 pixels
        {
            glVertex2f(center.x + sin(DEG2RAD*i) * radius, center.y + cos(DEG2RAD*i) * radius);
        }
    glEnd();
    
    glDisable(GL_POLYGON_SMOOTH);
}

// Draw circle outline
void DrawCircleLines(int centerX, int centerY, float radius, Color color)
{
    glEnable(GL_LINE_SMOOTH);                    // Smoothies circle outline (anti-aliasing applied)
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);      // Best quality for line smooth (anti-aliasing best algorithm)
    
    glBegin(GL_LINE_LOOP);
        glColor4ub(color.r, color.g, color.b, color.a);
        
        // NOTE: Circle outline is drawn pixel by pixel every degree (0 to 360)
        for (int i=0; i < 360; i++)
        {
            glVertex2f(centerX + sin(DEG2RAD*i) * radius, centerY + cos(DEG2RAD*i) * radius);
        }
   glEnd();
   
// NOTE: Alternative method to draw circle outline
/*
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    DrawCircle(centerX, centerY, radius, color);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
*/
   glDisable(GL_LINE_SMOOTH);
}

// Draw a color-filled rectangle
void DrawRectangle(int posX, int posY, int width, int height, Color color)
{
    glBegin(GL_QUADS);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2i(posX, posY);
        glVertex2i(posX + width, posY);
        glVertex2i(posX + width, posY + height);
        glVertex2i(posX, posY + height);
    glEnd();
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
    glBegin(GL_QUADS);
        glColor4ub(color1.r, color1.g, color1.b, color1.a);
        glVertex2i(posX, posY);
        glVertex2i(posX + width, posY);
        glColor4ub(color2.r, color2.g, color2.b, color2.a);
        glVertex2i(posX + width, posY + height);
        glVertex2i(posX, posY + height);
    glEnd();
}

// Draw a color-filled rectangle (Vector version)
void DrawRectangleV(Vector2 position, Vector2 size, Color color)
{
    glBegin(GL_QUADS);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2i(position.x, position.y);
        glVertex2i(position.x + size.x, position.y);
        glVertex2i(position.x + size.x, position.y + size.y);
        glVertex2i(position.x, position.y + size.y);
    glEnd();
}

// Draw rectangle outline
void DrawRectangleLines(int posX, int posY, int width, int height, Color color)
{
    //glEnable(GL_LINE_SMOOTH);                      // Smoothies circle outline (anti-aliasing applied)
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);        // Best quality for line smooth (anti-aliasing best algorithm)
    
    // NOTE: Lines are rasterized using the "Diamond Exit" rule so, it's nearly impossible to obtain a pixel-perfect engine
    // NOTE: Recommended trying to avoid using lines, at least >1.0f pixel lines with anti-aliasing (glLineWidth function)

    glBegin(GL_LINE_LOOP);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2i(posX, posY);
        glVertex2i(posX + width - 1, posY);
        glVertex2i(posX + width - 1, posY + height - 1);
        glVertex2i(posX, posY + height - 1);
    glEnd();

// NOTE: Alternative method to draw rectangle outline
/*
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    DrawRectangle(posX, posY, width - 1, height - 1, color);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
*/
    //glDisable(GL_LINE_SMOOTH);
}

// Draw a triangle
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    glBegin(GL_TRIANGLES);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2f(v1.x, v1.y);
        glVertex2f(v2.x, v2.y);
        glVertex2f(v3.x, v3.y);
    glEnd();
}

void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    glBegin(GL_LINE_LOOP);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2f(v1.x, v1.y);
        glVertex2f(v2.x, v2.y);
        glVertex2f(v3.x, v3.y);
    glEnd();
}

// Draw a regular polygon of n sides (Vector version)
void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color)
{
    if (sides < 3) sides = 3;

    glPushMatrix();
        glTranslatef(center.x, center.y, 0);
        glRotatef(rotation, 0, 0, 1);
        
        glBegin(GL_TRIANGLE_FAN);
            glColor4ub(color.r, color.g, color.b, color.a);
            glVertex2f(0, 0);

            for (int i=0; i <= sides; i++)
            { 
                glVertex2f(radius*cos(i*2*PI/sides), radius*sin(i*2*PI/sides));
            }
        glEnd();
    glPopMatrix();
}

// Draw a closed polygon defined by points
// NOTE: Array num elements MUST be passed as parameter to function
void DrawPolyEx(Vector2 *points, int numPoints, Color color)
{
    if (numPoints >= 3)
    {
        glEnable(GL_POLYGON_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    
        glBegin(GL_POLYGON);
            glColor4ub(color.r, color.g, color.b, color.a);
            
            for (int i = 0; i < numPoints; i++)
            {
                glVertex2f(points[i].x, points[i].y);
            }
        glEnd();
        
        glDisable(GL_POLYGON_SMOOTH);
    }
}

// Draw polygon lines
// NOTE: Array num elements MUST be passed as parameter to function                                                
void DrawPolyExLines(Vector2 *points, int numPoints, Color color)
{
    if (numPoints >= 2)
    {
        //glEnable(GL_LINE_SMOOTH);                      // Smoothies circle outline (anti-aliasing applied)
        //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);        // Best quality for line smooth (anti-aliasing best algorithm)
    
        glBegin(GL_LINE_LOOP);
            glColor4ub(color.r, color.g, color.b, color.a);
            
            for (int i = 0; i < numPoints; i++)
            {
                glVertex2f(points[i].x, points[i].y);
            }
        glEnd();
        
        //glDisable(GL_LINE_SMOOTH);
    }
}

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
