/*******************************************************************************************
*
*   raylib [core] example - Minimal Virtual Dpad
*
*   Example originally created with raylib 5.0, last time updated with raylib 5.0
*
*   Example create by GreenSnakeLinux (@GreenSnakeLinux),
*   lighter by oblerion (@oblerion) and 
*   reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h>
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    const int dpad_x = 90;
    const int dpad_y = 300;
    const int dpad_rad = 25;//radius of each pad
    Color dpad_color = BLUE;
    int dpad_keydown = -1;//-1 if not down, else 0,1,2,3 

    // collider[4] with x,y
    const float dpad_collider[4][2]=
    {
        {dpad_x,dpad_y-dpad_rad*1.5},//up
        {dpad_x-dpad_rad*1.5,dpad_y},//left
        {dpad_x+dpad_rad*1.5,dpad_y},//right
        {dpad_x,dpad_y+dpad_rad*1.5}//down
    };
    const char dpad_label[4]="XYBA";

    float player_x=100;
    float player_y=100;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
	// update --------------------------------------------------------------------------
        dpad_keydown = -1; //reset
        float input_x=0;
        float input_y=0;
        if(GetTouchPointCount()>0)
        {//use touch pos
            input_x = GetTouchX();
            input_y = GetTouchY();
        }
        else
        {//use mouse pos
            input_x = GetMouseX();
            input_y = GetMouseY();
        }
        for(int i=0;i<4;i++)
        {
            //test distance each collider and input < radius
            if( fabsf(dpad_collider[i][1]-input_y) + fabsf(dpad_collider[i][0]-input_x) < dpad_rad)
            {
                dpad_keydown = i;
                break;
            }
        }
        // move player
        switch(dpad_keydown){
            case 0: player_y -= 50*GetFrameTime();
            break;
            case 1: player_x -= 50*GetFrameTime();
            break;
            case 2: player_x += 50*GetFrameTime();
            break;
            case 3: player_y += 50*GetFrameTime();
            default:;
        };
	// draw ----------------------------------------------------------------------------
       BeginDrawing();
            ClearBackground(RAYWHITE);

            for(int i=0;i<4;i++)
            {
                //draw all pad
                DrawCircle(dpad_collider[i][0],dpad_collider[i][1],dpad_rad,dpad_color);
                if(i!=dpad_keydown)
                {
                    //draw label
                    DrawText(TextSubtext(dpad_label,i,1),
                             dpad_collider[i][0]-5,
                             dpad_collider[i][1]-5,16,BLACK);
                }

            }
            DrawText("Player",player_x,player_y,16,BLACK);
        EndDrawing();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

