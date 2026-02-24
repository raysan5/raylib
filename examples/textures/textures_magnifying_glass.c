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
#include "rlgl.h" // for rlSetBlendFactorsSeparate()

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
        
    Texture2D bunny = LoadTexture("resources/raybunny.png");
    Texture2D parrots = LoadTexture("resources/parrots.png");
    
    // Use image draw to generate a mask texture instead of loading it from a file.
    Image circle = GenImageColor(256, 256, BLANK);
    ImageDrawCircle(&circle, 128, 128, 128, WHITE);
    Texture2D mask = LoadTextureFromImage(circle); // Copy the mask image from RAM to VRAM
    UnloadImage(circle); // Unload the image from RAM
    
    RenderTexture2D magnifiedWorld = LoadRenderTexture(256, 256);
    
    Camera2D camera = { 0 };
    // Set magnifying glass zoom
    camera.zoom = 2;
    // Offset by half the size of the magnifying glass to counteract drawing the texture centered on the mouse position
    camera.offset = (Vector2){128, 128};


    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector2 mPos = GetMousePosition();
        camera.target = mPos;

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw the normal version of the world
            DrawTexture(parrots, 144, 33, WHITE);
            DrawText("Use the magnifying glass to find hidden bunnies!", 154, 6, 20, BLACK);

            // Render to a the magnifying glass 
            BeginTextureMode(magnifiedWorld);
                ClearBackground(RAYWHITE);

                BeginMode2D(camera);
                    // Draw the same things in the magnified world as were in the normal version
                    DrawTexture(parrots, 144, 33, WHITE);
                    DrawText("Use the magnifying glass to find hidden bunnies!", 154, 6, 20, BLACK);

                    // Draw bunnies only in the magnified world.
                    // BLEND_MULTIPLIED lets them take on the color of the image below them.
                    BeginBlendMode(BLEND_MULTIPLIED);
                        DrawTexture(bunny, 250, 350, WHITE);
                        DrawTexture(bunny, 500, 100, WHITE);
                        DrawTexture(bunny, 420, 300, WHITE);
                        DrawTexture(bunny, 650, 10, WHITE);
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
                    DrawTexture(mask, 0, 0, WHITE);
                EndBlendMode();
            EndTextureMode();

            // Draw magnifiedWorld to screen, centered on cursor
            DrawTextureRec(magnifiedWorld.texture, (Rectangle){0, 0, 256, -256}, (Vector2){mPos.x - 128, mPos.y - 128}, WHITE);

            // Draw the outer ring of the magnifying glass
            DrawRing(mPos, 126, 130, 0, 360, 64, BLACK);

            // Draw floating specular highlight on the glass
            float rx = mPos.x/800;
            float ry = mPos.y/800;
            DrawCircle((int)(mPos.x - 64*rx) - 32, (int)(mPos.y - 64*ry) - 32, 4, ColorAlpha(WHITE, 0.5));

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(parrots);
    UnloadTexture(bunny);
    UnloadTexture(mask);
    UnloadRenderTexture(magnifiedWorld);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
