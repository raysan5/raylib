/*******************************************************************************************
*
*   raylib [core] example - Texture source and destination rectangles (adapted for HTML5 platform)
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 800;
int screenHeight = 450;

// NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
Texture2D guybrush;

int frameWidth;
int frameHeight;

Rectangle sourceRec;
Rectangle destRec;
Vector2 origin;

int rotation = 0;


//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void);     // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [textures] examples - texture source and destination rectangles");
    
    guybrush = LoadTexture("resources/guybrush.png");        // Texture loading
    
    frameWidth = guybrush.width/7;
    frameHeight = guybrush.height;
    
    // NOTE: On PLATFORM_WEB, NPOT textures support is limited
    
    // NOTE: Source rectangle (part of the texture to use for drawing)
    sourceRec = (Rectangle){ 0, 0, frameWidth, frameHeight };

    // NOTE: Destination rectangle (screen rectangle where drawing part of texture)
    destRec = (Rectangle){ screenWidth/2, screenHeight/2, frameWidth*2, frameHeight*2 };

    // NOTE: Origin of the texture (rotation/scale point), it's relative to destination rectangle size
    origin = (Vector2){ frameWidth, frameHeight };

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(guybrush);       // Texture unloading
     
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    rotation++;
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        // NOTE: Using DrawTexturePro() we can easily rotate and scale the part of the texture we draw
        // sourceRec defines the part of the texture we use for drawing
        // destRec defines the rectangle where our texture part will fit (scaling it to fit)
        // origin defines the point of the texture used as reference for rotation and scaling
        // rotation defines the texture rotation (using origin as rotation point)
        DrawTexturePro(guybrush, sourceRec, destRec, origin, rotation, WHITE);

        DrawLine(destRec.x, 0, destRec.x, screenHeight, GRAY);
        DrawLine(0, destRec.y, screenWidth, destRec.y, GRAY);

    EndDrawing();
    //----------------------------------------------------------------------------------
}