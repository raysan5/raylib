/*******************************************************************************************
*
*   raylib [shapes] example - hilbert curve example
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Hamza RAHAL (@hmz-rhl)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Hamza RAHAL (@hmz-rhl)
*
********************************************************************************************/


#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>

const int screenWidth = 800;

const int screenHeight = 450;

int order = 2;

int total;

int counter = 0;

Vector2 *hilbertPath = 0;

const Vector2 hilbertPoints[4] =
{
    [0] = {
        .x = 0,
        .y = 0
    },
    [1] = {
        .x = 0,
        .y = 1
    },
    [2] = {
        .x = 1,
        .y = 1
    },
    [3] = {
        .x = 1,
        .y = 0
    },
};

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
Vector2 Hilbert(int index);

void InitHilbertPath(void);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - hilbert curve example");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    InitHilbertPath();
    
    //--------------------------------------------------------------------------------------

    // Main game loop
    //--------------------------------------------------------------------------------------
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if ((IsKeyPressed(KEY_UP)) && (order < 8))
        {
            counter = 0;
            ++order;
            InitHilbertPath();
        }
        else if((IsKeyPressed(KEY_DOWN)) && (order > 1))
        {
            counter = 0;
            --order;
            InitHilbertPath();
        }
        //----------------------------------------------------------------------------------
        
        // Draw
        //--------------------------------------------------------------------------
        BeginDrawing();
        DrawText(TextFormat("(press UP or DOWN to change)\norder : %d", order), screenWidth/2 + 70, 25, 20, WHITE);
        
        if(counter < total)
        {
            ClearBackground(BLACK);
            for (int i = 1; i <= counter; i++)
            {
                DrawLineV(hilbertPath[i], hilbertPath[i-1], ColorFromHSV(((float)i / total) * 360.0f, 1.0f, 1.0f));
            }
            counter += 1;
        }
        EndDrawing();
        //--------------------------------------------------------------------------
    }
    //--------------------------------------------------------------------------------------

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    MemFree(hilbertPath);
    //--------------------------------------------------------------------------------------
    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definition
//------------------------------------------------------------------------------------

// calculate U positions
Vector2 Hilbert(int index)
{

    int hiblertIndex = index&3;
    Vector2 vect = hilbertPoints[hiblertIndex];
    float temp;
    int len;

    for (int j = 1; j < order; j++)
    {
        index = index>>2;
        hiblertIndex = index&3;
        len = 1<<j;
        switch (hiblertIndex)
        {
            case 0:
                temp = vect.x;
                vect.x = vect.y;
                vect.y = temp;
                break;
            case 2:
                vect.x += len;
            case 1:
                vect.y += len;
                break;
            case 3:
                temp = len - 1 - vect.x;
                vect.x = 2*len - 1 - vect.y;
                vect.y = temp;
                break;
        }
    }
    return vect;
}

// Calculate the whole Hilbert Path (including each U and their link)
void InitHilbertPath(void)
{
    int N;
    float len;
    N = 1<<order;
    total = N*N;
    MemFree(hilbertPath);
    hilbertPath = NULL;
    hilbertPath = (Vector2*)MemAlloc(sizeof(Vector2)*total);
    if(hilbertPath == NULL)
    {
        printf("%s: malloc failed\n", __func__);
    }
    len = (float)screenHeight/N;
    for (int i = 0; i < total; i++)
    {
        hilbertPath[i] = Hilbert(i);
        hilbertPath[i].x = hilbertPath[i].x*len + len/2.0f;
        hilbertPath[i].y = hilbertPath[i].y*len + len/2.0f;
    }
}
