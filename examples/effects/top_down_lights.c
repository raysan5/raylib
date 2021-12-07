/*******************************************************************************************
*
*   raylib [effects] example - Top Down Lights
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 4.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2021 Jeffery Myers
*
********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"

// Custom Blend Modes
#define RLGL_SRC_ALPHA 0x0302
#define RLGL_MIN 0x8007
#define RLGL_MAX 0x8008


#define MAX_BOXES 20
Rectangle Boxes[MAX_BOXES];

typedef struct  
{
    Vector2 Vertecies[4];
}ShadowGeometry;

#define MAX_SHADOWS 40 // MAX_BOXES * 2. Each box can cast up to two shadow volumes for the edges

typedef struct 
{
    // is this light slot active
    bool Active;

    // does this light need to be updated
    bool Dirty;

    // is this light in a valid position
    bool Valid;

    // Light position
    Vector2 Position;

    // alpha mask for the light
	RenderTexture Mask;

    // the distance the light touches
	float OuterRadius;

    // a cached rectangle of the light bounds to help with culling
	Rectangle Bounds;

    ShadowGeometry Shadows[MAX_SHADOWS];
    int ShadowCount;
}LightInfo;

#define MAX_LIGHTS 16
LightInfo Lights[MAX_LIGHTS] = { 0 };


// move a light and mark it as dirty so that we update it's mask next frame
void MoveLight(int slot, float x, float y)
{
    Lights[slot].Dirty = true;
    Lights[slot].Position.x = x; 
    Lights[slot].Position.y = y;

    // update the cached bounds
    Lights[slot].Bounds.x = x - Lights[slot].OuterRadius;
    Lights[slot].Bounds.y = y - Lights[slot].OuterRadius;
}

// setup a light
void SetUpLight(int slot, float x, float y, float radius)
{
    Lights[slot].Active = true;
    Lights[slot].Mask = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    Lights[slot].OuterRadius = radius;

    Lights[slot].Bounds.width = radius * 2;
    Lights[slot].Bounds.height = radius * 2;

    MoveLight(slot, x, y);
}

// see if a light needs to update it's mask
void UpdateLight(int slot, Rectangle* boxes, int count)
{
    if (!Lights[slot].Active || !Lights[slot].Dirty)
        return;

    Lights[slot].Dirty = false;

    Lights[slot].ShadowCount = 0;
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [effects] example - top down lights");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}