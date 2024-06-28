/*******************************************************************************************
*
*   raylib [textures] example - Retrive image channel (mask)
*
*   NOTE: Images are loaded in CPU memory (RAM); textures are loaded in GPU memory (VRAM)
*
*   Example originally created with raylib 5.1-dev, last time updated with raylib 5.1-dev
*
*   Example contributed by Bruno Cabral (github.com/brccabral) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024-2024 Bruno Cabral (github.com/brccabral) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include <raylib.h>

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    const int screenWidth = 800;
    const int screenHeight = 612;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - extract channel from image");

    Image fudesumiImage = LoadImage("resources/fudesumi.png");
    Texture2D fudesumiTexture = LoadTextureFromImage(fudesumiImage);

    Image imageRed = ImageFromChannel(fudesumiImage, 0, 0.0f);
    Texture2D textureRed = LoadTextureFromImage(imageRed);
    UnloadImage(imageRed);

    Image imageGreen = ImageFromChannel(fudesumiImage, 1, 0.0f);
    Texture2D textureGreen = LoadTextureFromImage(imageGreen);
    UnloadImage(imageGreen);

    Image imageBlue = ImageFromChannel(fudesumiImage, 2, 0.0f);
    Texture2D textureBlue = LoadTextureFromImage(imageBlue);
    UnloadImage(imageBlue);

    Image imageAlpha = ImageFromChannel(fudesumiImage, 3, 0.0f);
    Texture2D textureAlpha = LoadTextureFromImage(imageAlpha);
    UnloadImage(imageAlpha);


    Image backgroundImage = GenImageChecked(screenWidth, screenHeight, screenWidth/20, screenHeight/20, ORANGE, YELLOW);
    Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);
    UnloadImage(backgroundImage);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    Rectangle fudesumiRec = {0, 0, fudesumiImage.width, fudesumiImage.height};
    Rectangle redPos = { 410, 50, fudesumiImage.width / 2, fudesumiImage.height / 2 };
    Rectangle greenPos = { 600, 50, fudesumiImage.width / 2, fudesumiImage.height / 2 };
    Rectangle bluePos = { 410, 310, fudesumiImage.width / 2, fudesumiImage.height / 2 };
    Rectangle alphaPos = { 600, 310, fudesumiImage.width / 2, fudesumiImage.height / 2 };

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            DrawTexture(backgroundTexture, 0, 0, WHITE);
            DrawTexture(fudesumiTexture, 50, 50, WHITE);

            DrawTexturePro(textureRed, fudesumiRec, redPos, (Vector2) {0, 0}, 0, RED);
            DrawTexturePro(textureGreen, fudesumiRec, greenPos, (Vector2) {0, 0}, 0, GREEN);
            DrawTexturePro(textureBlue, fudesumiRec, bluePos, (Vector2) {0, 0}, 0, BLUE);
            DrawTexturePro(textureAlpha, fudesumiRec, alphaPos, (Vector2) {0, 0}, 0, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(backgroundTexture);
    UnloadImage(fudesumiImage);
    UnloadTexture(fudesumiTexture);
    UnloadTexture(textureRed);
    UnloadTexture(textureGreen);
    UnloadTexture(textureBlue);
    UnloadTexture(textureAlpha);
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
