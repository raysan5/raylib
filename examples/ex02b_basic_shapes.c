/*******************************************************************************************
*
*   raylib example 02b - Draw basic shapes 2d (rectangle, circle, line...)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    int screenWidth = 800;
    int screenHeight = 450;
    
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib example 02b - basic shapes drawing");
    //----------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //-----------------------------------------------------
        // TODO: Update your variables here
        //-----------------------------------------------------
        
        // Draw
        //-----------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            // TODO: draw some shapes... with names... :P
/*            
void DrawPixel(int posX, int posY, Color color);
void DrawPixelV(Vector2 position, Color color);
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);
void DrawCircle(int centerX, int centerY, float radius, Color color);
void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2);
void DrawCircleV(Vector2 center, float radius, Color color);
void DrawCircleLines(int centerX, int centerY, float radius, Color color);
void DrawRectangle(int posX, int posY, int width, int height, Color color);
void DrawRectangleRec(Rectangle rec, Color color);
void DrawRectangleGradient(int posX, int posY, int width, int height, Color color1, Color color2);
void DrawRectangleV(Vector2 position, Vector2 size, Color color);
void DrawRectangleLines(int posX, int posY, int width, int height, Color color);
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawPoly(Vector2 *points, int numPoints, Color color);
void DrawPolyLine(Vector2 *points, int numPoints, Color color);
*/            
            DrawRectangle(screenWidth/4 - 50, screenHeight/2 - 100, 100, 100, GOLD);
            DrawCircle(3*screenWidth/4, screenHeight/2 - 50, 50, MAROON);
            
            DrawText("_____", 320, 280, 50, 1, BLACK);
        
        EndDrawing();
        //-----------------------------------------------------
    }

    // De-Initialization
    //---------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------
    
    return 0;
}