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
            void DrawPixel(int posX, int posY, Color color);                                                   // Draw a pixel
            void DrawPixelV(Vector2 position, Color color);                                                    // Draw a pixel (Vector version)
            void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);                // Draw a line
            void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);                                     // Draw a line (Vector version)
            void DrawCircle(int centerX, int centerY, float radius, Color color);                              // Draw a color-filled circle
            void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2);       // Draw a gradient-filled circle
            void DrawCircleV(Vector2 center, float radius, Color color);                                       // Draw a color-filled circle (Vector version)
            void DrawCircleLines(int centerX, int centerY, float radius, Color color);                         // Draw circle outline
            void DrawRectangle(int posX, int posY, int width, int height, Color color);                        // Draw a color-filled rectangle
            void DrawRectangleRec(Rectangle rec, Color color);                                                 // Draw a color-filled rectangle
            void DrawRectangleGradient(int posX, int posY, int width, int height, Color color1, Color color2); // Draw a gradient-filled rectangle
            void DrawRectangleV(Vector2 position, Vector2 size, Color color);                                  // Draw a color-filled rectangle (Vector version)
            void DrawRectangleLines(int posX, int posY, int width, int height, Color color);                   // Draw rectangle outline
            void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);                                // Draw a color-filled triangle
            void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);                           // Draw triangle outline
            void DrawPoly(Vector2 *points, int numPoints, Color color);                                        // Draw a closed polygon defined by points
            void DrawPolyLine(Vector2 *points, int numPoints, Color color);                                    // Draw polygon lines
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