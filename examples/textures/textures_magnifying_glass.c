/*******************************************************************************************
*
*   raylib textures example - magnifying glass
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Luke Vaughan (@badram) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Luke Vaughan (@badram)
*
********************************************************************************************/

#include "raylib.h"

#include "rlgl.h"       // Required for: rlSetBlendFactorsSeparate()

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - magnifying glass");

    Texture2D texBunny = LoadTexture("resources/raybunny.png");
    Texture2D texParrots = LoadTexture("resources/parrots.png");

    // Use image draw to generate a mask texture instead of loading it from a file.
    Image imCircle = GenImageColor(256, 256, BLANK);
    ImageDrawCircle(&imCircle, 128, 128, 128, WHITE);
    Texture2D texMask = LoadTextureFromImage(imCircle);
    UnloadImage(imCircle);

    RenderTexture2D rtMagnifiedWorld = LoadRenderTexture(256, 256);

    Camera2D camera = { 0 };
    camera.zoom = 2; // Set magnifying glass zoom
    camera.offset = (Vector2){ 128, 128 }; // Offset by half the size of the magnifying glass

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector2 mousePosition = GetMousePosition();
        camera.target = mousePosition;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw the normal version of the world
            DrawTexture(texParrots, 144, 33, WHITE);
            DrawText("Use the magnifying glass to find hidden bunnies!", 154, 6, 20, BLACK);

            // Render to a the magnifying glass
            BeginTextureMode(rtMagnifiedWorld);
                ClearBackground(RAYWHITE);

                BeginMode2D(camera);
                    // Draw the same things in the magnified world as were in the normal version
                    DrawTexture(texParrots, 144, 33, WHITE);
                    DrawText("Use the magnifying glass to find hidden bunnies!", 154, 6, 20, BLACK);

                    // Draw bunnies only in the magnified world.
                    // BLEND_MULTIPLIED lets them take on the color of the image below them.
                    BeginBlendMode(BLEND_MULTIPLIED);
                        DrawTexture(texBunny, 250, 350, WHITE);
                        DrawTexture(texBunny, 500, 100, WHITE);
                        DrawTexture(texBunny, 420, 300, WHITE);
                        DrawTexture(texBunny, 650, 10, WHITE);
                    EndBlendMode();
                EndMode2D();

                // Mask the magnifying glass view texture to a circle
                // To make the mask affect only alpha, a CUSTOM blend mode is used with SEPARATE color/alpha functions
                BeginBlendMode(BLEND_CUSTOM_SEPARATE);
                    // C: Color, A: Alpha, s: source (texture to draw), d: destination (texture drawn to)
                    //   glSrcRGB: RL_ZERO      - Cs * 0 = 0  - discard source rgb because we don't want to draw our texture's colors at all
                    //   glDstRGB: RL_ONE       - Cd * 1 = Cd - use destination colors unmodified
                    //   glSrcAlpha: RL_ONE     - As * 1 = As - use source alpha unmodified
                    //   glDstAlpha: RL_ZERO    - Ad * 0 = 0  - discard destination alpha
                    //   glEqRGB: RL_FUNC_ADD   - Cs(0) + Cd = Cd - destination color is unmodified
                    //   glEqAlpha: RL_FUNC_ADD - As + Ad(0) = As - destination alpha is set to source alpha
                    rlSetBlendFactorsSeparate(RL_ZERO, RL_ONE, RL_ONE, RL_ZERO, RL_FUNC_ADD, RL_FUNC_ADD);
                    DrawTexture(texMask, 0, 0, WHITE);
                EndBlendMode();
            EndTextureMode();

            // Draw magnifiedWorld to screen, centered on cursor
            DrawTextureRec(rtMagnifiedWorld.texture, (Rectangle){0, 0, 256, -256}, (Vector2){mousePosition.x - 128, mousePosition.y - 128}, WHITE);

            // Draw the outer ring of the magnifying glass
            DrawRing(mousePosition, 126, 130, 0, 360, 64, BLACK);

            // Draw floating specular highlight on the glass
            float rx = mousePosition.x/800;
            float ry = mousePosition.y/800;
            DrawCircle((int)(mousePosition.x - 64*rx) - 32, (int)(mousePosition.y - 64*ry) - 32, 4, ColorAlpha(WHITE, 0.5));

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texParrots);
    UnloadTexture(texBunny);
    UnloadTexture(texMask);
    UnloadRenderTexture(rtMagnifiedWorld);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
