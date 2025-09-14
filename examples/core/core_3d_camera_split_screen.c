/*******************************************************************************************
*
*   raylib [core] example - 3d camera split screen
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 3.7, last time updated with raylib 4.0
*
*   Example contributed by Jeffery Myers (@JeffM2501) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2021-2025 Jeffery Myers (@JeffM2501)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera split screen");

    // Setup player 1 camera and screen
    Camera cameraPlayer1 = { 0 };
    cameraPlayer1.fovy = 45.0f;
    cameraPlayer1.up.y = 1.0f;
    cameraPlayer1.target.y = 1.0f;
    cameraPlayer1.position.z = -3.0f;
    cameraPlayer1.position.y = 1.0f;

    RenderTexture screenPlayer1 = LoadRenderTexture(screenWidth/2, screenHeight);

    // Setup player two camera and screen
    Camera cameraPlayer2 = { 0 };
    cameraPlayer2.fovy = 45.0f;
    cameraPlayer2.up.y = 1.0f;
    cameraPlayer2.target.y = 3.0f;
    cameraPlayer2.position.x = -3.0f;
    cameraPlayer2.position.y = 3.0f;

    RenderTexture screenPlayer2 = LoadRenderTexture(screenWidth/2, screenHeight);

    // Build a flipped rectangle the size of the split view to use for drawing later
    Rectangle splitScreenRect = { 0.0f, 0.0f, (float)screenPlayer1.texture.width, (float)-screenPlayer1.texture.height };

    // Grid data
    int count = 5;
    float spacing = 4;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // If anyone moves this frame, how far will they move based on the time since the last frame
        // this moves things at 10 world units per second, regardless of the actual FPS
        float offsetThisFrame = 10.0f*GetFrameTime();

        // Move Player1 forward and backwards (no turning)
        if (IsKeyDown(KEY_W))
        {
            cameraPlayer1.position.z += offsetThisFrame;
            cameraPlayer1.target.z += offsetThisFrame;
        }
        else if (IsKeyDown(KEY_S))
        {
            cameraPlayer1.position.z -= offsetThisFrame;
            cameraPlayer1.target.z -= offsetThisFrame;
        }

        // Move Player2 forward and backwards (no turning)
        if (IsKeyDown(KEY_UP))
        {
            cameraPlayer2.position.x += offsetThisFrame;
            cameraPlayer2.target.x += offsetThisFrame;
        }
        else if (IsKeyDown(KEY_DOWN))
        {
            cameraPlayer2.position.x -= offsetThisFrame;
            cameraPlayer2.target.x -= offsetThisFrame;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        // Draw Player1 view to the render texture
        BeginTextureMode(screenPlayer1);
            ClearBackground(SKYBLUE);

            BeginMode3D(cameraPlayer1);

                // Draw scene: grid of cube trees on a plane to make a "world"
                DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 50, 50 }, BEIGE); // Simple world plane

                for (float x = -count*spacing; x <= count*spacing; x += spacing)
                {
                    for (float z = -count*spacing; z <= count*spacing; z += spacing)
                    {
                        DrawCube((Vector3) { x, 1.5f, z }, 1, 1, 1, LIME);
                        DrawCube((Vector3) { x, 0.5f, z }, 0.25f, 1, 0.25f, BROWN);
                    }
                }

                // Draw a cube at each player's position
                DrawCube(cameraPlayer1.position, 1, 1, 1, RED);
                DrawCube(cameraPlayer2.position, 1, 1, 1, BLUE);

            EndMode3D();

            DrawRectangle(0, 0, GetScreenWidth()/2, 40, Fade(RAYWHITE, 0.8f));
            DrawText("PLAYER1: W/S to move", 10, 10, 20, MAROON);

        EndTextureMode();

        // Draw Player2 view to the render texture
        BeginTextureMode(screenPlayer2);
            ClearBackground(SKYBLUE);

            BeginMode3D(cameraPlayer2);

                // Draw scene: grid of cube trees on a plane to make a "world"
                DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 50, 50 }, BEIGE); // Simple world plane

                for (float x = -count*spacing; x <= count*spacing; x += spacing)
                {
                    for (float z = -count*spacing; z <= count*spacing; z += spacing)
                    {
                        DrawCube((Vector3) { x, 1.5f, z }, 1, 1, 1, LIME);
                        DrawCube((Vector3) { x, 0.5f, z }, 0.25f, 1, 0.25f, BROWN);
                    }
                }

                // Draw a cube at each player's position
                DrawCube(cameraPlayer1.position, 1, 1, 1, RED);
                DrawCube(cameraPlayer2.position, 1, 1, 1, BLUE);

            EndMode3D();

            DrawRectangle(0, 0, GetScreenWidth()/2, 40, Fade(RAYWHITE, 0.8f));
            DrawText("PLAYER2: UP/DOWN to move", 10, 10, 20, DARKBLUE);

        EndTextureMode();

        // Draw both views render textures to the screen side by side
        BeginDrawing();
            ClearBackground(BLACK);

            DrawTextureRec(screenPlayer1.texture, splitScreenRect, (Vector2){ 0, 0 }, WHITE);
            DrawTextureRec(screenPlayer2.texture, splitScreenRect, (Vector2){ screenWidth/2.0f, 0 }, WHITE);

            DrawRectangle(GetScreenWidth()/2 - 2, 0, 4, GetScreenHeight(), LIGHTGRAY);
        EndDrawing();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(screenPlayer1); // Unload render texture
    UnloadRenderTexture(screenPlayer2); // Unload render texture

    CloseWindow();                      // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
