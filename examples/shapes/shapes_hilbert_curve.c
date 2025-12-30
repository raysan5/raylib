/*******************************************************************************************
*
*   raylib [shapes] example - hilbert curve
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Hamza RAHAL (@hmz-rhl) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Hamza RAHAL (@hmz-rhl)
*
********************************************************************************************/


#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <stdlib.h>     // Required for: calloc(), free()

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static Vector2 *LoadHilbertPath(int order, float size, int *strokeCount);
static void UnloadHilbertPath(Vector2 *hilbertPath);
static Vector2 ComputeHilbertStep(int order, int index);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - hilbert curve");

    int order = 2;
    float size = GetScreenHeight();
    int strokeCount = 0;
    Vector2 *hilbertPath = LoadHilbertPath(order, size, &strokeCount);
    
    int prevOrder = order;
    int prevSize = (int)size;       // NOTE: Size from slider is float but for comparison we use int
    int counter = 0;
    float thick = 2.0f;
    bool animate = true;
    
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    //--------------------------------------------------------------------------------------
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Check if order or size have changed to regenerate
        // NOTE: Size from slider is float but for comparison we use int
        if ((prevOrder != order) || (prevSize != (int)size))
        {
            UnloadHilbertPath(hilbertPath);
            hilbertPath = LoadHilbertPath(order, size, &strokeCount);
            
            if (animate) counter = 0;
            else counter = strokeCount;
            
            prevOrder = order;
            prevSize = size;
        }
        //----------------------------------------------------------------------------------
        
        // Draw
        //--------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);

            if (counter < strokeCount)
            {
                // Draw Hilbert path animation, one stroke every frame
                for (int i = 1; i <= counter; i++)
                {
                    DrawLineEx(hilbertPath[i], hilbertPath[i - 1], thick, ColorFromHSV(((float)i/strokeCount)*360.0f, 1.0f, 1.0f));
                }
                
                counter += 1;
            }
            else
            {
                // Draw full Hilbert path
                for (int i = 1; i < strokeCount; i++)
                {
                    DrawLineEx(hilbertPath[i], hilbertPath[i - 1], thick, ColorFromHSV(((float)i/strokeCount)*360.0f, 1.0f, 1.0f));
                }
            }
        
            // Draw UI using raygui
            GuiCheckBox((Rectangle){ 450, 50, 20, 20 }, "ANIMATE GENERATION ON CHANGE", &animate);
            GuiSpinner((Rectangle){ 585, 100, 180, 30 }, "HILBERT CURVE ORDER:  ", &order, 2, 8, false);
            GuiSlider((Rectangle){ 524, 150, 240, 24 }, "THICKNESS:  ", NULL, &thick, 1.0f, 10.0f);
            GuiSlider((Rectangle){ 524, 190, 240, 24 }, "TOTAL SIZE: ", NULL, &size, 10.0f, GetScreenHeight()*1.5f);
        
        EndDrawing();
        //--------------------------------------------------------------------------
    }
    //--------------------------------------------------------------------------------------

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadHilbertPath(hilbertPath);
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definition
//------------------------------------------------------------------------------------
// Load the whole Hilbert Path (including each U and their link)
static Vector2 *LoadHilbertPath(int order, float size, int *strokeCount)
{
    int N = 1 << order;
    float len = size/N;
    *strokeCount = N*N;

    Vector2 *hilbertPath = (Vector2 *)RL_CALLOC(*strokeCount, sizeof(Vector2));
    
    for (int i = 0; i < *strokeCount; i++)
    {
        hilbertPath[i] = ComputeHilbertStep(order, i);
        hilbertPath[i].x = hilbertPath[i].x*len + len/2.0f;
        hilbertPath[i].y = hilbertPath[i].y*len + len/2.0f;
    }
    
    return hilbertPath;
}

// Unload Hilbert path data
static void UnloadHilbertPath(Vector2 *hilbertPath)
{
    RL_FREE(hilbertPath);
}

// Compute Hilbert path U positions
static Vector2 ComputeHilbertStep(int order, int index)
{
    // Hilbert points base pattern
    static const Vector2 hilbertPoints[4] = {
        [0] = { .x = 0, .y = 0 },
        [1] = { .x = 0, .y = 1 },
        [2] = { .x = 1, .y = 1 },
        [3] = { .x = 1, .y = 0 },
    };
       
    int hilbertIndex = index&3;
    Vector2 vect = hilbertPoints[hilbertIndex];
    float temp = 0.0f;
    int len = 0;

    for (int j = 1; j < order; j++)
    {
        index = index >> 2;
        hilbertIndex = index&3;
        len = 1 << j;
        
        switch (hilbertIndex)
        {
            case 0:
            {
                temp = vect.x;
                vect.x = vect.y;
                vect.y = temp;
            } break;
            case 2: vect.x += len;
            case 1: vect.y += len; break;
            case 3:
            {
                temp = len - 1 - vect.x;
                vect.x = 2*len - 1 - vect.y;
                vect.y = temp;
            } break;
            default: break;
        }
    }
    
    return vect;
}
