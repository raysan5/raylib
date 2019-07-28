/*******************************************************************************************
*
*   raylib [textures] example - Mouse painting
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2019 Chris Dill (@MysteriousSpace)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_COLORS_COUNT 21 // Number of colors available

int main(void) 
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight,
                "raylib [textures] example - texture painting");

    // Different colours to choose from
    Color colors[MAX_COLORS_COUNT] = {
        DARKGRAY,  MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
        GRAY,      RED,    GOLD,   LIME,      BLUE,     VIOLET,     BROWN,
        LIGHTGRAY, PINK,   YELLOW, GREEN,     SKYBLUE,  PURPLE,     BEIGE};

    const char *colorNames[MAX_COLORS_COUNT] = {
        "DARKGRAY",  "MAROON", "ORANGE",    "DARKGREEN", "DARKBLUE", "DARKPURPLE",
        "DARKBROWN", "GRAY",   "RED",       "GOLD",      "LIME",     "BLUE",
        "VIOLET",    "BROWN",  "LIGHTGRAY", "PINK",      "YELLOW",   "GREEN",
        "SKYBLUE",   "PURPLE", "BEIGE"};

    int colorState = 0;
    int brushSize = 20;

    // Create a RenderTexture2D to use as a canvas
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    Color clearColor = RAYWHITE;

    BeginTextureMode(target);
    ClearBackground(clearColor);
    EndTextureMode();

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
	    // Update
	    //----------------------------------------------------------------------------------
	    // Switch between colors
	    if (IsKeyPressed(KEY_RIGHT))
	        colorState++;
	    else if (IsKeyPressed(KEY_LEFT))
	        colorState--;

	    if (colorState >= MAX_COLORS_COUNT)
	        colorState = 0;
	    else if (colorState < 0)
	        colorState = MAX_COLORS_COUNT - 1;

	    brushSize += GetMouseWheelMove() * 5;
	    if (brushSize < 0)
	        brushSize = 0;
	    if (brushSize > 50)
	        brushSize = 50;

	    Vector2 position = GetMousePosition();

	    if (IsKeyPressed(KEY_C)) {
	        BeginTextureMode(target);
	        ClearBackground(RAYWHITE);
	        EndTextureMode();
	    }

	    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
	        TraceLog(LOG_INFO, "Painting x: %f y: %f", position.x, position.y);
	        BeginTextureMode(target);
	        DrawCircle(position.x, position.y, brushSize, colors[colorState]);
	        EndTextureMode();
	    }

	    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
	        TraceLog(LOG_INFO, "Erasing x: %f y: %f", position.x, position.y);
	        BeginTextureMode(target);
	        DrawCircle(position.x, position.y, brushSize, clearColor);
	        EndTextureMode();
	    }

	    if (IsKeyPressed(KEY_S)) {
	        TakeScreenshot("textures_mouse_painting.png");
	    }
	    //----------------------------------------------------------------------------------

	    // Draw
	    //----------------------------------------------------------------------------------
	    BeginDrawing();

	    ClearBackground(RAYWHITE);

	        // NOTE: Render texture must be y-flipped due to default OpenGL coordinates
	        // (left-bottom)
	        DrawTextureRec(target.texture, (Rectangle){0, 0, target.texture.width, -target.texture.height}, (Vector2){0, 0}, WHITE);

	        // Draw 2d shapes and text over drawn texture
	        DrawRectangle(0, 9, 380, 60, Fade(LIGHTGRAY, 0.7f));

	        DrawText("COLOR:", 10, 15, 20, BLACK);
	        DrawText(colorNames[colorState], 130, 15, 20, RED);
	        DrawText("< >", 340, 10, 30, DARKBLUE);

	        DrawText("Size:", 10, 40, 20, BLACK);
	        DrawText(FormatText("%i", brushSize), 130, 40, 20, RED);

	        DrawCircle(GetMouseX(), GetMouseY(), brushSize, colors[colorState]);

	        DrawFPS(700, 15);

	    EndDrawing();
	    //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
