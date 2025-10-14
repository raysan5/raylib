/*******************************************************************************************
*
*   raylib [core] example - custom logging
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 2.5, last time updated with raylib 2.5
*
*   Example contributed by Pablo Marcos Oltra (@pamarcos) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2018-2025 Pablo Marcos Oltra (@pamarcos) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "../src/raylib.h"

#include <stdio.h>                  // Required for: fopen(), fclose(), fputc(), fwrite(), printf(), fprintf(), funopen()
#include <time.h>                   // Required for: time_t, tm, time(), localtime(), strftime()

// Custom logging function
void CustomLog(int msgType, const char *text, va_list args)
{

    char timeStr[64] = { 0 };
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);

    switch (msgType)
    {
        case LOG_INFO: printf(ANSI_GREEN "[%s] [INFO] : ", timeStr); break;
        case LOG_ERROR: printf(ANSI_YELLOW "[%s] [ERROR]: ", timeStr); break;
        case LOG_WARNING: printf(ANSI_RED "[%s] [WARN] : ", timeStr); break;
        case LOG_DEBUG: printf(ANSI_BLUE "[%s] [DEBUG]: ", timeStr); break;
        case LOG_FATAL: printf(ANSI_BOLD ANSI_RED "[%s] [FATAL] : ", timeStr); break;
        default: break;
    }

    printf("%s", ANSI_RESET);

    vprintf(text, args);
    printf("\n");
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    // Set custom logger
    SetTraceLogCallback(CustomLog);

    InitWindow(screenWidth, screenHeight, "raylib [core] example - custom logging");

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

        DrawText("Check out the console output to see the custom logger in action!", 60, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
