/*******************************************************************************************
*
*   raylib [core] example - window flags
*
*   This example has been created using raylib 3.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2020 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main(void)
{
    // Initialization
    //---------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    // Possible window flags
    /*
    FLAG_FULLSCREEN_MODE    -> not working
    FLAG_WINDOW_RESIZABLE  
    FLAG_WINDOW_UNDECORATED
    FLAG_WINDOW_TRANSPARENT
    FLAG_WINDOW_HIDDEN     
    FLAG_WINDOW_MINIMIZED   -> not working
    FLAG_WINDOW_MAXIMIZED   -> not working
    FLAG_WINDOW_UNFOCUSED  
    FLAG_WINDOW_TOPMOST    
    FLAG_WINDOW_HIGHDPI     -> errors after minimize-resize, fb size is recalculated
    FLAG_WINDOW_ALWAYS_RUN  -> not working
    FLAG_VSYNC_HINT        
    FLAG_MSAA_4X_HINT      
    FLAG_INTERLACED_HINT   
    */
    
    // Set configuration flags for window creation
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - window flags");

    Vector2 ballPosition = { GetScreenWidth()/2, GetScreenHeight()/2 };
    Vector2 ballSpeed = { 5.0f, 4.0f };
    int ballRadius = 20;

    int framesCounter = 0;
    
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //----------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //-----------------------------------------------------
        if (IsKeyPressed(KEY_ONE)) SetWindowState(FLAG_FULLSCREEN_MODE);
        if (IsKeyPressed(KEY_TWO)) SetWindowState(FLAG_WINDOW_RESIZABLE);
        if (IsKeyPressed(KEY_THREE)) SetWindowState(FLAG_WINDOW_UNDECORATED);
        if (IsKeyPressed(KEY_FOUR)) { SetWindowState(FLAG_WINDOW_HIDDEN); framesCounter = 0; }
        if (IsKeyPressed(KEY_FIVE)) SetWindowState(FLAG_WINDOW_MINIMIZED);
        if (IsKeyPressed(KEY_SIX)) SetWindowState(FLAG_WINDOW_MAXIMIZED);   // NOTE: Requires FLAG_WINDOW_RESIZABLE!
        if (IsKeyPressed(KEY_SEVEN)) SetWindowState(FLAG_WINDOW_UNFOCUSED); // not working 
        if (IsKeyPressed(KEY_EIGHT)) SetWindowState(FLAG_WINDOW_TOPMOST);
        
        if (IsKeyPressed(KEY_F)) ToggleFullscreen();  // modifies window size when scaling!
        if (IsKeyPressed(KEY_R)) RestoreWindow();
        if (IsKeyPressed(KEY_D)) DecorateWindow();
        if (IsWindowState(FLAG_WINDOW_HIDDEN))
        {
            framesCounter++;
            if (framesCounter >= 180) UnhideWindow(); // Show window after 3 seconds
        }

        // Bouncing ball logic
        ballPosition.x += ballSpeed.x;
        ballPosition.y += ballSpeed.y;
        if ((ballPosition.x >= (GetScreenWidth() - ballRadius)) || (ballPosition.x <= ballRadius)) ballSpeed.x *= -1.0f;
        if ((ballPosition.y >= (GetScreenHeight() - ballRadius)) || (ballPosition.y <= ballRadius)) ballSpeed.y *= -1.0f;
        //-----------------------------------------------------

        // Draw
        //-----------------------------------------------------
        BeginDrawing();

            ClearBackground(BLANK);

            DrawCircleV(ballPosition, ballRadius, MAROON);
            DrawRectangleLinesEx((Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() }, 4, RAYWHITE);
            
            DrawFPS(10, 10);
            
            DrawText(FormatText("Screen Width: %i", GetScreenWidth()), 10, 40, 10, GREEN);
            DrawText(FormatText("Screen Height: %i", GetScreenHeight()), 10, 60, 10, GREEN);
            
            // Draw window state info
            DrawText(FormatText("FLAG_FULLSCREEN_MODE:    %s", IsWindowState(FLAG_FULLSCREEN_MODE)? "true" : "false"), 10, 80, 10, GREEN);
            DrawText(FormatText("FLAG_WINDOW_RESIZABLE:   %s", IsWindowState(FLAG_WINDOW_RESIZABLE)? "true" : "false"), 10, 100, 10, GREEN);  
            DrawText(FormatText("FLAG_WINDOW_UNDECORATED: %s", IsWindowState(FLAG_WINDOW_UNDECORATED)? "true" : "false"), 10, 120, 10, GREEN);
            DrawText(FormatText("FLAG_WINDOW_TRANSPARENT: %s", IsWindowState(FLAG_WINDOW_TRANSPARENT)? "true" : "false"), 10, 140, 10, GREEN);
            DrawText(FormatText("FLAG_WINDOW_HIDDEN:      %s", IsWindowState(FLAG_WINDOW_HIDDEN)? "true" : "false"), 10, 160, 10, GREEN);     
            DrawText(FormatText("FLAG_WINDOW_MINIMIZED:   %s", IsWindowState(FLAG_WINDOW_MINIMIZED)? "true" : "false"), 10, 180, 10, GREEN);
            DrawText(FormatText("FLAG_WINDOW_MAXIMIZED:   %s", IsWindowState(FLAG_WINDOW_MAXIMIZED)? "true" : "false"), 10, 200, 10, GREEN);
            DrawText(FormatText("FLAG_WINDOW_UNFOCUSED:   %s", IsWindowState(FLAG_WINDOW_UNFOCUSED)? "true" : "false"), 10, 220, 10, GREEN);  
            DrawText(FormatText("FLAG_WINDOW_TOPMOST:     %s", IsWindowState(FLAG_WINDOW_TOPMOST)? "true" : "false"), 10, 240, 10, GREEN);    
            DrawText(FormatText("FLAG_WINDOW_HIGHDPI:     %s", IsWindowState(FLAG_WINDOW_HIGHDPI)? "true" : "false"), 10, 260, 10, GREEN);
            DrawText(FormatText("FLAG_WINDOW_ALWAYS_RUN:  %s", IsWindowState(FLAG_WINDOW_ALWAYS_RUN)? "true" : "false"), 10, 280, 10, GREEN);
            DrawText(FormatText("FLAG_VSYNC_HINT:         %s", IsWindowState(FLAG_VSYNC_HINT)? "true" : "false"), 10, 300, 10, GREEN);        
            DrawText(FormatText("FLAG_MSAA_4X_HINT:       %s", IsWindowState(FLAG_MSAA_4X_HINT)? "true" : "false"), 10, 320, 10, GREEN);      
            DrawText(FormatText("FLAG_INTERLACED_HINT:    %s", IsWindowState(FLAG_INTERLACED_HINT)? "true" : "false"), 10, 340, 10, GREEN);   

        EndDrawing();
        //-----------------------------------------------------
    }

    // De-Initialization
    //---------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------

    return 0;
}
