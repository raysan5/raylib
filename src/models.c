/*********************************************************************************************
*
*   raylib.models
*
*   Basic functions to draw 3d shapes and load/draw 3d models (.OBJ)
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

#include <GL/gl.h>       // OpenGL functions
#include <stdio.h>       // Standard input/output functions, used to read model files data
#include <stdlib.h>      // Declares malloc() and free() for memory management
#include <math.h>        // Used for sin, cos, tan
#include "vector3.h"     // Basic Vector3 functions

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// It's lonely here...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static float GetHeightValue(Color pixel);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Draw cube
// NOTE: Cube position is de center position
void DrawCube(Vector3 position, float width, float height, float lenght, Color color)
{
    glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        //glRotatef(rotation, 0.0f, 1.0f, 0.0f);
        //glScalef(1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
            glColor4ub(color.r, color.g, color.b, color.a);
            
            // Front Face
            glNormal3f(0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-width/2, -height/2,  lenght/2);  // Bottom Left Of The Texture and Quad
            glTexCoord2f(1.0f, 0.0f); glVertex3f( width/2, -height/2,  lenght/2);  // Bottom Right Of The Texture and Quad
            glTexCoord2f(1.0f, 1.0f); glVertex3f( width/2,  height/2,  lenght/2);  // Top Right Of The Texture and Quad
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-width/2,  height/2,  lenght/2);  // Top Left Of The Texture and Quad
            // Back Face
            glNormal3f( 0.0f, 0.0f,-1.0f);                  // Normal Pointing Away From Viewer
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-width/2, -height/2, -lenght/2);  // Bottom Right Of The Texture and Quad
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-width/2,  height/2, -lenght/2);  // Top Right Of The Texture and Quad
            glTexCoord2f(0.0f, 1.0f); glVertex3f( width/2,  height/2, -lenght/2);  // Top Left Of The Texture and Quad
            glTexCoord2f(0.0f, 0.0f); glVertex3f( width/2, -height/2, -lenght/2);  // Bottom Left Of The Texture and Quad
            // Top Face
            glNormal3f( 0.0f, 1.0f, 0.0f);                  // Normal Pointing Up
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-width/2,  height/2, -lenght/2);  // Top Left Of The Texture and Quad
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-width/2,  height/2,  lenght/2);  // Bottom Left Of The Texture and Quad
            glTexCoord2f(1.0f, 0.0f); glVertex3f( width/2,  height/2,  lenght/2);  // Bottom Right Of The Texture and Quad
            glTexCoord2f(1.0f, 1.0f); glVertex3f( width/2,  height/2, -lenght/2);  // Top Right Of The Texture and Quad
            // Bottom Face
            glNormal3f( 0.0f,-1.0f, 0.0f);                  // Normal Pointing Down
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-width/2, -height/2, -lenght/2);  // Top Right Of The Texture and Quad
            glTexCoord2f(0.0f, 1.0f); glVertex3f( width/2, -height/2, -lenght/2);  // Top Left Of The Texture and Quad
            glTexCoord2f(0.0f, 0.0f); glVertex3f( width/2, -height/2,  lenght/2);  // Bottom Left Of The Texture and Quad
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-width/2, -height/2,  lenght/2);  // Bottom Right Of The Texture and Quad
            // Right face
            glNormal3f( 1.0f, 0.0f, 0.0f);                  // Normal Pointing Right
            glTexCoord2f(1.0f, 0.0f); glVertex3f( width/2, -height/2, -lenght/2);  // Bottom Right Of The Texture and Quad
            glTexCoord2f(1.0f, 1.0f); glVertex3f( width/2,  height/2, -lenght/2);  // Top Right Of The Texture and Quad
            glTexCoord2f(0.0f, 1.0f); glVertex3f( width/2,  height/2,  lenght/2);  // Top Left Of The Texture and Quad
            glTexCoord2f(0.0f, 0.0f); glVertex3f( width/2, -height/2,  lenght/2);  // Bottom Left Of The Texture and Quad
            // Left Face
            glNormal3f(-1.0f, 0.0f, 0.0f);                  // Normal Pointing Left
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-width/2, -height/2, -lenght/2);  // Bottom Left Of The Texture and Quad
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-width/2, -height/2,  lenght/2);  // Bottom Right Of The Texture and Quad
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-width/2,  height/2,  lenght/2);  // Top Right Of The Texture and Quad
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-width/2,  height/2, -lenght/2);  // Top Left Of The Texture and Quad
        glEnd();
    glPopMatrix();
}

// Draw cube (Vector version)
void DrawCubeV(Vector3 position, Vector3 size, Color color)
{
    DrawCube(position, size.x, size.y, size.z, color);
}

// Draw cube wires
void DrawCubeWires(Vector3 position, float width, float height, float lenght, Color color)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    DrawCube(position, width, height, lenght, color);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Draw sphere
void DrawSphere(Vector3 centerPos, float radius, Color color)
{
    DrawSphereEx(centerPos, radius, 16, 16, color);
}

// Draw sphere with extended parameters
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color)
{
    float lat0, z0, zr0;
    float lat1, z1, zr1;
    float lng, x, y;
    
    glPushMatrix();
        glTranslatef(centerPos.x, centerPos.y, centerPos.z);
        glRotatef(90, 1, 0, 0);
        glScalef(radius, radius, radius);
        
        glBegin(GL_QUAD_STRIP);
                    
            glColor4ub(color.r, color.g, color.b, color.a);
            
            for(int i = 0; i <= rings; i++) 
            {
                lat0 = PI * (-0.5 + (float)(i - 1) / rings);
                z0  = sin(lat0);
                zr0 =  cos(lat0);
            
                lat1 = PI * (-0.5 + (float)i / rings);
                z1 = sin(lat1);
                zr1 = cos(lat1);
        
                for(int j = 0; j <= slices; j++) 
                {
                    lng = 2 * PI * (float)(j - 1) / slices;
                    x = cos(lng);
                    y = sin(lng);
        
                    glNormal3f(x * zr0, y * zr0, z0);
                    glVertex3f(x * zr0, y * zr0, z0);
                    
                    glNormal3f(x * zr1, y * zr1, z1);
                    glVertex3f(x * zr1, y * zr1, z1);
                }
            }
        glEnd();
    glPopMatrix();
}

// Draw sphere wires
void DrawSphereWires(Vector3 centerPos, float radius, Color color)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    DrawSphere(centerPos, radius, color);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Draw a cylinder/cone
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color)    // Could be used for pyramid and cone!
{
    Vector3 a = { position.x, position.y + height, position.z };
    Vector3 d = { 0.0f, 1.0f, 0.0f };
    Vector3 p;
    Vector3 c = { a.x + (-d.x * height), a.y + (-d.y * height), a.z + (-d.z * height) };    //= a + (-d * h);
    Vector3 e0 = VectorPerpendicular(d);
    Vector3 e1 = VectorCrossProduct(e0, d);
    float angInc = 360.0 / slices * DEG2RAD;

    if (radiusTop == 0)    // Draw pyramid or cone
    {
        //void drawCone(const Vector3 &d, const Vector3 &a, const float h, const float rd, const int n)
        //d – axis defined as a normalized vector from base to apex
        //a – position of apex (top point)
        //h – height
        //rd – radius of directrix
        //n – number of radial "slices"
        
        glPushMatrix();
            //glTranslatef(centerPos.x, centerPos.y, centerPos.z);
            //glRotatef(degrees, 0.0f, 1.0f, 0.0f);
            //glScalef(1.0f, 1.0f, 1.0f);
            
            // Draw cone top
            glBegin(GL_TRIANGLE_FAN);
                glColor4ub(color.r, color.g, color.b, color.a);
                glVertex3f(a.x, a.y, a.z);
                for (int i = 0; i <= slices; i++) 
                {
                    float rad = angInc * i;
                    p.x = c.x + (((e0.x * cos(rad)) + (e1.x * sin(rad))) * radiusBottom);
                    p.y = c.y + (((e0.y * cos(rad)) + (e1.y * sin(rad))) * radiusBottom);
                    p.z = c.z + (((e0.z * cos(rad)) + (e1.z * sin(rad))) * radiusBottom);
                    glVertex3f(p.x, p.y, p.z);
                }
            glEnd();
         
            // Draw cone bottom
            glBegin(GL_TRIANGLE_FAN);
                glColor4ub(color.r, color.g, color.b, color.a);
                glVertex3f(c.x, c.y, c.z);
                for (int i = slices; i >= 0; i--) 
                {
                    float rad = angInc * i;
                    p.x = c.x + (((e0.x * cos(rad)) + (e1.x * sin(rad))) * radiusBottom);
                    p.y = c.y + (((e0.y * cos(rad)) + (e1.y * sin(rad))) * radiusBottom);
                    p.z = c.z + (((e0.z * cos(rad)) + (e1.z * sin(rad))) * radiusBottom);
                    glVertex3f(p.x, p.y, p.z);
                }
            glEnd();
            
        glPopMatrix();
    }
    else    // Draw cylinder
    {
        glPushMatrix();
            //glTranslatef(centerPos.x, centerPos.y, centerPos.z);
            //glRotatef(degrees, 0.0f, 1.0f, 0.0f);
            //glScalef(1.0f, 1.0f, 1.0f);

            // Draw cylinder top (pointed cap)
            glBegin(GL_TRIANGLE_FAN);
                glColor4ub(color.r, color.g, color.b, color.a);
                glVertex3f(c.x, c.y + height, c.z);
                for (int i = slices; i >= 0; i--) 
                {
                    float rad = angInc * i;
                    p.x = c.x + (((e0.x * cos(rad)) + (e1.x * sin(rad))) * radiusTop);
                    p.y = c.y + (((e0.y * cos(rad)) + (e1.y * sin(rad))) * radiusTop) + height;
                    p.z = c.z + (((e0.z * cos(rad)) + (e1.z * sin(rad))) * radiusTop);
                    glVertex3f(p.x, p.y, p.z);
                }
            glEnd();
            
            // Draw cylinder sides
            glBegin(GL_TRIANGLE_STRIP);
                glColor4ub(color.r, color.g, color.b, color.a);
                for (int i = slices; i >= 0; i--)
                {
                    float rad = angInc * i;
                    p.x = c.x + (((e0.x * cos(rad)) + (e1.x * sin(rad))) * radiusTop);
                    p.y = c.y + (((e0.y * cos(rad)) + (e1.y * sin(rad))) * radiusTop) + height;
                    p.z = c.z + (((e0.z * cos(rad)) + (e1.z * sin(rad))) * radiusTop);
                    glVertex3f(p.x, p.y, p.z);
                    
                    p.x = c.x + (((e0.x * cos(rad)) + (e1.x * sin(rad))) * radiusBottom);
                    p.y = c.y + (((e0.y * cos(rad)) + (e1.y * sin(rad))) * radiusBottom);
                    p.z = c.z + (((e0.z * cos(rad)) + (e1.z * sin(rad))) * radiusBottom);
                    glVertex3f(p.x, p.y, p.z);
                }
            glEnd();
            
            // Draw cylinder bottom
            glBegin(GL_TRIANGLE_FAN);
                glColor4ub(color.r, color.g, color.b, color.a);
                glVertex3f(c.x, c.y, c.z);
                for (int i = slices; i >= 0; i--) 
                {
                    float rad = angInc * i;
                    p.x = c.x + (((e0.x * cos(rad)) + (e1.x * sin(rad))) * radiusBottom);
                    p.y = c.y + (((e0.y * cos(rad)) + (e1.y * sin(rad))) * radiusBottom);
                    p.z = c.z + (((e0.z * cos(rad)) + (e1.z * sin(rad))) * radiusBottom);
                    glVertex3f(p.x, p.y, p.z);
                }
            glEnd();
            
        glPopMatrix();
    }
}

// Draw a cylinder/cone wires
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    DrawCylinder(position, radiusTop, radiusBottom, height, slices, color);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Draw a plane
void DrawPlane(Vector3 centerPos, Vector2 size, Vector3 rotation, Color color)
{
    // NOTE: Plane is always created on XZ ground and then rotated
    glPushMatrix();
        glTranslatef(centerPos.x, centerPos.y, centerPos.z);
        
        // TODO: Review multiples rotations Gimbal-Lock... use matrix or quaternions...
        glRotatef(rotation.x, 1, 0, 0);
        glRotatef(rotation.y, 0, 1, 0);
        glRotatef(rotation.z, 0, 0, 1);
        glScalef(size.x, 1.0f, size.y);
    
        glBegin(GL_QUADS);
            glColor4ub(color.r, color.g, color.b, color.a);
            glNormal3f(0.0f, 1.0f, 0.0f); 
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.0f, -0.5f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.0f, -0.5f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.0f, 0.5f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.0f, 0.5f);
        glEnd();

    glPopMatrix();
}

// Draw a plane with divisions
void DrawPlaneEx(Vector3 centerPos, Vector2 size, Vector3 rotation, int slicesX, int slicesZ, Color color)
{
    float quadWidth = size.x / slicesX;
    float quadLenght = size.y / slicesZ;
    
    float texPieceW = 1 / size.x;
    float texPieceH = 1 / size.y;

    // NOTE: Plane is always created on XZ ground and then rotated
    glPushMatrix();
        glTranslatef(-size.x / 2, 0.0f, -size.y / 2);
        glTranslatef(centerPos.x, centerPos.y, centerPos.z);
        
        // TODO: Review multiples rotations Gimbal-Lock... use matrix or quaternions...
        glRotatef(rotation.x, 1, 0, 0);
        glRotatef(rotation.y, 0, 1, 0);
        glRotatef(rotation.z, 0, 0, 1);
    
        glBegin(GL_QUADS);
            glColor4ub(color.r, color.g, color.b, color.a);
            glNormal3f(0.0f, 1.0f, 0.0f);
            
            for (int z = 0; z < slicesZ; z++)
            {
                for (int x = 0; x < slicesX; x++)
                {
                    // Draw the plane quad by quad (with textcoords)
                    glTexCoord2f((float)x * texPieceW, (float)z * texPieceH);
                    glVertex3f((float)x * quadWidth, 0.0f, (float)z * quadLenght);
                    
                    glTexCoord2f((float)x * texPieceW + texPieceW, (float)z * texPieceH);
                    glVertex3f((float)x * quadWidth  + quadWidth, 0.0f, (float)z * quadLenght);
                    
                    glTexCoord2f((float)x * texPieceW + texPieceW, (float)z * texPieceH + texPieceH);
                    glVertex3f((float)x * quadWidth + quadWidth, 0.0f, (float)z * quadLenght + quadLenght);
                    
                    glTexCoord2f((float)x * texPieceW, (float)z * texPieceH + texPieceH);
                    glVertex3f((float)x * quadWidth, 0.0f, (float)z * quadLenght + quadLenght);
                }
            }
        glEnd();

    glPopMatrix();
}

// Draw a grid centered at (0, 0, 0)
void DrawGrid(int slices, float spacing)
{
    int halfSlices = slices / 2;
    
    //glEnable(GL_LINE_SMOOTH);                    // Smoothies circle outline (anti-aliasing applied)
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);      // Best quality for line smooth (anti-aliasing best algorithm)

    glPushMatrix();
        glScalef(spacing, 1.0f, spacing);

        glBegin(GL_LINES);
            for(int i = -halfSlices; i <= halfSlices; i++)
            {
                if (i == 0) glColor3f(0.5f, 0.5f, 0.5f);
                else glColor3f(0.75f, 0.75f, 0.75f);
                
                glVertex3f((float)i, 0.0f, (float)-halfSlices);
                glVertex3f((float)i, 0.0f, (float)halfSlices);

                glVertex3f((float)-halfSlices, 0.0f, (float)i);
                glVertex3f((float)halfSlices, 0.0f, (float)i);
            }
        glEnd();
        
    glPopMatrix();
    
    //glDisable(GL_LINE_SMOOTH);
}

// Draw gizmo (with or without orbits)
void DrawGizmo(Vector3 position, bool orbits)
{
    // NOTE: RGB = XYZ
    float lenght = 1.0f;
    float radius = 1.0f;
    
    //glEnable(GL_LINE_SMOOTH);                    // Smoothies circle outline (anti-aliasing applied)
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);      // Best quality for line smooth (anti-aliasing best algorithm)
    
    glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        //glRotatef(rotation, 0, 1, 0);
        glScalef(lenght, lenght, lenght);
    
        glBegin(GL_LINES);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(1.0f, 0.0f, 0.0f);
            
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 1.0f, 0.0f);
            
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 1.0f);
        glEnd();
        
        if (orbits)
        {
            glBegin(GL_LINE_LOOP);
                glColor4f(1.0f, 0.0f, 0.0f, 0.4f);
                for (int i=0; i < 360; i++) glVertex3f(sin(DEG2RAD*i) * radius, 0, cos(DEG2RAD*i) * radius);
            glEnd();
            
            glBegin(GL_LINE_LOOP);
                glColor4f(0.0f, 1.0f, 0.0f, 0.4f);
                for (int i=0; i < 360; i++) glVertex3f(sin(DEG2RAD*i) * radius, cos(DEG2RAD*i) * radius, 0);
            glEnd();
            
            glBegin(GL_LINE_LOOP);
                glColor4f(0.0f, 0.0f, 1.0f, 0.4f);
                for (int i=0; i < 360; i++) glVertex3f(0, sin(DEG2RAD*i) * radius, cos(DEG2RAD*i) * radius);
            glEnd();
        }
    
    glPopMatrix();
    
    //glDisable(GL_LINE_SMOOTH);
}

// Load a 3d model (.OBJ)
// TODO: Add comments explaining this function process
Model LoadModel(const char *fileName)                                    
{
    Model model;
    
    char dataType;
    char comments[200];
    
    int numVertex = 0;
    int numNormals = 0;
    int numTexCoords = 0;
    int numTriangles = 0;

    FILE* objFile;

    objFile = fopen(fileName, "rt");
    
    while(!feof(objFile))
    {
        fscanf(objFile, "%c", &dataType);
        
        switch(dataType)
        {
            case '#':         // It's a comment
            {
                fgets(comments, 200, objFile);                
            } break;
            case 'v': 
            {
                fscanf(objFile, "%c", &dataType);
                
                if (dataType == 't')    // Read texCoord
                {
                    fgets(comments, 200, objFile);
                    fscanf(objFile, "%c", &dataType);
                
                    while (dataType == 'v')
                    {
                        fgets(comments, 200, objFile);
                        fscanf(objFile, "%c", &dataType);
                    }
                    
                    if (dataType == '#')
                    {
                        fscanf(objFile, "%i", &numTexCoords);
                    }
                    else printf("Ouch! Something was wrong...");
                    
                    fgets(comments, 200, objFile);
                }
                else if (dataType == 'n')    // Read normals
                {
                    fgets(comments, 200, objFile);
                    fscanf(objFile, "%c", &dataType);
                
                    while (dataType == 'v')
                    {
                        fgets(comments, 200, objFile);
                        fscanf(objFile, "%c", &dataType);
                    }
                    
                    if (dataType == '#')
                    {
                        fscanf(objFile, "%i", &numNormals);
                    }
                    else printf("Ouch! Something was wrong...");
                
                    fgets(comments, 200, objFile);
                }
                else    // Read vertex
                {
                    fgets(comments, 200, objFile);
                    fscanf(objFile, "%c", &dataType);
                
                    while (dataType == 'v')
                    {
                        fgets(comments, 200, objFile);
                        fscanf(objFile, "%c", &dataType);
                    }
                    
                    if (dataType == '#')
                    {
                        fscanf(objFile, "%i", &numVertex);
                    }
                    else printf("Ouch! Something was wrong...");
                    
                    fgets(comments, 200, objFile);
                }
            } break;
            case 'f':
            {
                fgets(comments, 200, objFile);
                fscanf(objFile, "%c", &dataType);
            
                while (dataType == 'f')
                {
                    fgets(comments, 200, objFile);
                    fscanf(objFile, "%c", &dataType);
                }
                
                if (dataType == '#')
                {
                    fscanf(objFile, "%i", &numTriangles);
                }
                else printf("Ouch! Something was wrong...");
                
                fgets(comments, 200, objFile);
            
            } break;
            default: break;
        }
    }
    
    Vector3 midVertices[numVertex];
    Vector3 midNormals[numNormals];
    Vector2 midTexCoords[numTexCoords];
    
    model.numVertices = numTriangles*3;

    model.vertices = (Vector3 *)malloc(model.numVertices * sizeof(Vector3));
    model.normals = (Vector3 *)malloc(model.numVertices * sizeof(Vector3));
    model.texcoords = (Vector2 *)malloc(model.numVertices * sizeof(Vector2));
    
    int countVertex = 0;
    int countNormals = 0;
    int countTexCoords = 0;
    
    int countMaxVertex = 0;
    
    rewind(objFile);
    
    while(!feof(objFile))
    {
        fscanf(objFile, "%c", &dataType);
        
        switch(dataType)
        {
            case '#': 
            {
                fgets(comments, 200, objFile);                
            } break;
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
            case 'f':
            {
                int vNum, vtNum, vnNum;
                fscanf(objFile, "%c", &dataType);
                fscanf(objFile, "%i/%i/%i", &vNum, &vtNum, &vnNum);
                
                model.vertices[countMaxVertex] = midVertices[vNum-1];
                model.normals[countMaxVertex] = midNormals[vnNum-1];
                model.texcoords[countMaxVertex].x = midTexCoords[vtNum-1].x;
                model.texcoords[countMaxVertex].y = -midTexCoords[vtNum-1].y;
                countMaxVertex++;
                                
                fscanf(objFile, "%i/%i/%i", &vNum, &vtNum, &vnNum);
                
                model.vertices[countMaxVertex] = midVertices[vNum-1];
                model.normals[countMaxVertex] = midNormals[vnNum-1];
                model.texcoords[countMaxVertex].x = midTexCoords[vtNum-1].x;
                model.texcoords[countMaxVertex].y = -midTexCoords[vtNum-1].y;
                countMaxVertex++;
                
                fscanf(objFile, "%i/%i/%i", &vNum, &vtNum, &vnNum);
                
                model.vertices[countMaxVertex] = midVertices[vNum-1];
                model.normals[countMaxVertex] = midNormals[vnNum-1];
                model.texcoords[countMaxVertex].x = midTexCoords[vtNum-1].x;
                model.texcoords[countMaxVertex].y = -midTexCoords[vtNum-1].y;
                countMaxVertex++;
            } break;
            default: break;
        }
    }
    
    fclose(objFile);
    
    return model;
}

// Load a heightmap image as a 3d model
Model LoadHeightmap(Image heightmap, float maxHeight)
{
    Model model;
    
    int mapX = heightmap.width;
	int mapZ = heightmap.height;
    
    // NOTE: One vertex per pixel
    // TODO: Consider resolution when generating model data?
    int numTriangles = (mapX-1)*(mapZ-1)*2;    // One quad every four pixels
  
    model.numVertices = numTriangles*3;

    model.vertices = (Vector3 *)malloc(model.numVertices * sizeof(Vector3));
    model.normals = (Vector3 *)malloc(model.numVertices * sizeof(Vector3));
    model.texcoords = (Vector2 *)malloc(model.numVertices * sizeof(Vector2));
    
    int vCounter = 0;
    int trisCounter = 0;
    
    float scaleFactor = maxHeight/255;    // TODO: Review scaleFactor calculation

	for(int z = 0; z < mapZ-1; z++)
	{
		for(int x = 0; x < mapX-1; x++)
		{
			// Fill vertices array with data
            //----------------------------------------------------------
            
            // one triangle - 3 vertex
            model.vertices[vCounter].x = x;
            model.vertices[vCounter].y = GetHeightValue(heightmap.pixels[x + z*mapX])*scaleFactor;
            model.vertices[vCounter].z = z;
            
            model.vertices[vCounter+1].x = x;
            model.vertices[vCounter+1].y = GetHeightValue(heightmap.pixels[x + (z+1)*mapX])*scaleFactor;
            model.vertices[vCounter+1].z = z+1;
            
            model.vertices[vCounter+2].x = x+1;
            model.vertices[vCounter+2].y = GetHeightValue(heightmap.pixels[(x+1) + z*mapX])*scaleFactor;
            model.vertices[vCounter+2].z = z;
            
            // another triangle - 3 vertex
            model.vertices[vCounter+3] = model.vertices[vCounter+2];
            model.vertices[vCounter+4] = model.vertices[vCounter+1];
            
            model.vertices[vCounter+5].x = x+1;
            model.vertices[vCounter+5].y = GetHeightValue(heightmap.pixels[(x+1) + (z+1)*mapX])*scaleFactor;
            model.vertices[vCounter+5].z = z+1;
            
            // Fill texcoords array with data
            //--------------------------------------------------------------
            model.texcoords[vCounter].x = (float)x / (mapX-1);
            model.texcoords[vCounter].y = (float)z / (mapZ-1);
            
            model.texcoords[vCounter+1].x = (float)x / (mapX-1);
            model.texcoords[vCounter+1].y = (float)(z+1) / (mapZ-1);
            
            model.texcoords[vCounter+2].x = (float)(x+1) / (mapX-1);
            model.texcoords[vCounter+2].y = (float)z / (mapZ-1);
            
            model.texcoords[vCounter+3] = model.texcoords[vCounter+2];
            model.texcoords[vCounter+4] = model.texcoords[vCounter+1];
            
            model.texcoords[vCounter+5].x = (float)(x+1) / (mapX-1);
            model.texcoords[vCounter+5].y = (float)(z+1) / (mapZ-1);
            
            // Fill normals array with data
            //--------------------------------------------------------------
            // TODO: Review normals calculation
            model.normals[vCounter] = (Vector3){ 0.0f, 1.0f, 0.0f };
            model.normals[vCounter+1] = (Vector3){ 0.0f, 1.0f, 0.0f };
            model.normals[vCounter+2] = (Vector3){ 0.0f, 1.0f, 0.0f };
            model.normals[vCounter+3] = (Vector3){ 0.0f, 1.0f, 0.0f };
            model.normals[vCounter+4] = (Vector3){ 0.0f, 1.0f, 0.0f };
            model.normals[vCounter+5] = (Vector3){ 0.0f, 1.0f, 0.0f };
            
            vCounter += 6;
            trisCounter += 2;
		}
	}

    return model;
}

// Unload 3d model from memory
void UnloadModel(Model model)
{
    free(model.vertices);
    free(model.texcoords);
    free(model.normals);
}

// Draw a model
void DrawModel(Model model, Vector3 position, float scale, Color color)
{
    // NOTE: For models we use Vertex Arrays (OpenGL 1.1)
    //static int rotation = 0;
    
    glEnableClientState(GL_VERTEX_ARRAY);                     // Enable vertex array
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);              // Enable texture coords array
    glEnableClientState(GL_NORMAL_ARRAY);                     // Enable normals array
        
    glVertexPointer(3, GL_FLOAT, 0, model.vertices);          // Pointer to vertex coords array
    glTexCoordPointer(2, GL_FLOAT, 0, model.texcoords);       // Pointer to texture coords array
    glNormalPointer(GL_FLOAT, 0, model.normals);              // Pointer to normals array
    //glColorPointer(4, GL_UNSIGNED_BYTE, 0, model.colors);   // Pointer to colors array (NOT USED)
        
    glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        //glRotatef(rotation * GetFrameTime(), 0, 1, 0);
        glScalef(scale, scale, scale);
        
        glColor4ub(color.r, color.g, color.b, color.a);

        glDrawArrays(GL_TRIANGLES, 0, model.numVertices);
    glPopMatrix();
    
    glDisableClientState(GL_VERTEX_ARRAY);                     // Disable vertex array
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);              // Disable texture coords array
    glDisableClientState(GL_NORMAL_ARRAY);                     // Disable normals array
    
    //rotation += 10;
}

// Draw a textured model
void DrawModelEx(Model model, Texture2D texture, Vector3 position, float scale, Color tint)
{
    glEnable(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, texture.glId);
    
    DrawModel(model, position, scale, tint);
    
    glDisable(GL_TEXTURE_2D);
}

// Draw a model wires
void DrawModelWires(Model model, Vector3 position, float scale, Color color)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    DrawModel(model, position, scale, color);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Draw a billboard
void DrawBillboard(Camera camera, Texture2D texture, Vector3 basePos, float size, Color tint)
{
    // NOTE: Billboard size will represent the width, height maintains aspect ratio
    Vector3 centerPos = { basePos.x, basePos.y + size * (float)texture.height/(float)texture.width/2, basePos.z };
    Vector2 sizeRatio = { size, size * (float)texture.height/texture.width };
    Vector3 rotation = { 90, 0, 0 };
    
    // TODO: Calculate Y rotation to face always camera (use matrix)
    // OPTION: Lock Y-axis

    glEnable(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, texture.glId);
    
    DrawPlane(centerPos, sizeRatio, rotation, tint);    // TODO: Review this function...
    
    glDisable(GL_TEXTURE_2D);
}

// Draw a billboard (part of a texture defined by a rectangle)
void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle sourceRec, Vector3 basePos, float size, Color tint)
{
    // NOTE: Billboard size will represent the width, height maintains aspect ratio
    //Vector3 centerPos = { basePos.x, basePos.y + size * (float)texture.height/(float)texture.width/2, basePos.z };
    //Vector2 sizeRatio = { size, size * (float)texture.height/texture.width };
    //Vector3 rotation = { 90, 0, 0 };
    
    // TODO: Calculate Y rotation to face always camera (use matrix)
    // OPTION: Lock Y-axis

    glEnable(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, texture.glId);
    
    // TODO: DrawPlane with correct textcoords for source rec.
    
    glDisable(GL_TEXTURE_2D);
}

// Get current vertex y altitude (proportional to pixel colors in grayscale)
static float GetHeightValue(Color pixel)
{
    return (((float)pixel.r + (float)pixel.g + (float)pixel.b)/3);
}