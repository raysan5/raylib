/*******************************************************************************************
*
*   raylib [core] example - UI input
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Maicon Santana (@maiconpintoabreu) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2018-2025 Maicon Santana (@maiconpintoabreu)
*
********************************************************************************************/

#include "raylib.h"

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static bool UIButtomIcon(Vector2 position, Texture2D icon); // Check if button clicked

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - ui input");
    Image arrowImage = GenImageColor(64, 64, BLANK);
    ImageDrawTriangle(&arrowImage, (Vector2){4, 31}, (Vector2){37, 4}, (Vector2){37, 59}, GRAY);
    Texture2D leftArrow = LoadTextureFromImage(arrowImage);
    ImageRotate(&arrowImage, 180);
    Texture2D rightArrow = LoadTextureFromImage(arrowImage);
    UnloadImage(arrowImage);

    Vector2 ballPosition = { (float)screenWidth/2, (float)screenHeight/3 };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT)) ballPosition.x += 2.0f;
        if (IsKeyDown(KEY_LEFT)) ballPosition.x -= 2.0f;
        if (IsKeyDown(KEY_UP)) ballPosition.y -= 2.0f;
        if (IsKeyDown(KEY_DOWN)) ballPosition.y += 2.0f;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("move the ball with ui buttons", 10, 10, 20, DARKGRAY);
            
            if (UIButtomIcon((Vector2){250, screenHeight - 120}, rightArrow)) ballPosition.x += 2.0f;
            if (UIButtomIcon((Vector2){100, screenHeight - 120}, leftArrow)) ballPosition.x -= 2.0f;

            DrawCircleV(ballPosition, 50, MAROON);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    UnloadTexture(rightArrow);
    UnloadTexture(leftArrow);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

static bool UIButtomIcon(Vector2 position, Texture2D icon) {
    // Check for Houver and Draw the button
    //--------------------------------------------------------------------------------------
    bool checkHouver = CheckCollisionPointCircle(GetMousePosition(), position, icon.width);
    DrawCircle(position.x, position.y, icon.width, checkHouver ? LIGHTGRAY : DARKGRAY);
    DrawTexture(icon, position.x - icon.width / 2, position.y - icon.width / 2, WHITE);
    //--------------------------------------------------------------------------------------

    // Check Mouse Left Button
    //--------------------------------------------------------------------------------------
    if (checkHouver && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        return true;
    }
    //--------------------------------------------------------------------------------------

    // Check All Current Touch Points
    //--------------------------------------------------------------------------------------
    int touchCount = GetTouchPointCount();
    for (int i = 0; i < touchCount; i++) {
        if (CheckCollisionPointCircle(GetTouchPosition(i), position, icon.width)) {
            return true;
        }
    }
    //--------------------------------------------------------------------------------------
    return false;
}