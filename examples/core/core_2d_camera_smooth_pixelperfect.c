/*******************************************************************************************
*
*   raylib [core] example - smooth pixel-perfect camera
*
*   This example has been created using raylib 3.7 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Giancamillo Alessandroni ([discord]NotManyIdeas#9972 - [github]NotManyIdeasDev) and
*   reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2021 Giancamillo Alessandroni (NotManyIdeas#9972) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h>

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    const int virualScreenWidth = 160;
    const int virtualScreenHeight = 90;

    const float virtualRatio = (float)screenWidth / (float)virualScreenWidth;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - smooth pixel-perfect camera");

    Camera2D worldSpaceCamera = { 0 }; // Game world camera
    worldSpaceCamera.zoom = 1.0f;

    Camera2D screenSpaceCamera = { 0 }; //Smoothing camera
    screenSpaceCamera.zoom = 1.0f;

    RenderTexture2D renderTexture = LoadRenderTexture(virualScreenWidth, virtualScreenHeight); //This is where we'll draw all our objects.

    Rectangle firstRectangle = { 70.0f, 35.0f, 20.0f, 20.0f };
    Rectangle secondRectangle = { 90.0f, 55.0f, 30.0f, 10.0f };
    Rectangle thirdRectangle = { 80.0f, 65.0f, 15.0f, 25.0f };

    Vector2 origin = { 0.0f, 0.0f };
    float rotation = 0.0f;

    float cameraX = 0.0f;
    float cameraY = 0.0f;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        rotation++; // Rotate the rectangles.

        // Make the camera move to demonstrate the effect.
        cameraX = (sin(GetTime()) * 50.0f) - 10.0f;
        cameraY = cos(GetTime()) * 30.0f;

        screenSpaceCamera.target = { cameraX, cameraY };

        if (screenSpaceCamera.target.x >= 1 || screenSpaceCamera.target.x <= -1) // Round worldCamera's X, keep the decimals on screenSpaceCamera.
        {
            worldSpaceCamera.target.x = (int)screenSpaceCamera.target.x;
            screenSpaceCamera.target.x -= worldSpaceCamera.target.x;
            screenSpaceCamera.target.x *= virtualRatio;
        }

        if (screenSpaceCamera.target.y >= 1 || screenSpaceCamera.target.y <= -1) // Round worldCamera's Y, keep the decimals on screenSpaceCamera.
        {
            worldSpaceCamera.target.y = (int)screenSpaceCamera.target.y;
            screenSpaceCamera.target.y -= worldSpaceCamera.target.y;
            screenSpaceCamera.target.y *= virtualRatio;
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RED); // This is for debug purposes. If you see red, then you've probably done something wrong.

        BeginTextureMode(renderTexture);

        BeginMode2D(worldSpaceCamera);

        ClearBackground(RAYWHITE); // This is the color you should see as background color.

        // Draw the rectangles
        DrawRectanglePro(firstRectangle, origin, rotation, BLACK);
        DrawRectanglePro(secondRectangle, origin, -rotation, RED);
        DrawRectanglePro(thirdRectangle, origin, rotation + 45.0f, BLUE);

        EndMode2D();

        EndTextureMode();

        BeginMode2D(screenSpaceCamera);

        // Draw the render texture with an offset of 1 worldSpace unit/pixel, so that the content behind the renderTexture is not shown.
        // Also the renderTexture's height is flipped (in the source Rectangle), due to OpenGL reasons.
        DrawTexturePro(
            renderTexture.texture,
            { 0.0f, 0.0f, (float)renderTexture.texture.width, (float)-renderTexture.texture.height },
            { -virtualRatio, -virtualRatio, screenWidth + (virtualRatio * 2), screenHeight + (virtualRatio * 2) },
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        EndMode2D();

        DrawText("Screen resolution: 800x450", 5, 0, 20, DARKBLUE);
        DrawText("World resolution: 160x90", 5, 20, 20, DARKGREEN);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    UnloadRenderTexture(renderTexture); // RenderTexture unloading

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}