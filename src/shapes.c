/*********************************************************************************************
*
*   raylib.shapes
*
*   Basic functions to draw 2d Shapes
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
#include <math.h>       // Math related functions, sin() and cos() used on DrawCircle*

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
    
    // NOTE: Alternative method to draw a pixel (point)
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
void DrawCircle(int centerX, int centerY, float radius, Color color)
{
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    
    glBegin(GL_TRIANGLE_FAN);
        glColor4ub(color.r, color.g, color.b, color.a);
        glVertex2i(centerX, centerY);

        for (int i=0; i <= 360; i++)        //i++ --> Step = 1.0 pixels
        {
            float degInRad = i*DEG2RAD;
            //glVertex2f(cos(degInRad)*radius,sin(degInRad)*radius);

            glVertex2f(centerX + sin(degInRad) * radius, centerY + cos(degInRad) * radius);
        }
    glEnd();
    
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

// Draw a closed polygon defined by points
// NOTE: Array num elements MUST be passed as parameter to function
void DrawPoly(Vector2 *points, int numPoints, Color color)
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
void DrawPolyLine(Vector2 *points, int numPoints, Color color)
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
