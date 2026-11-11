/*******************************************************************************************
*
*   raylib [textures] example - image channel
*
*   NOTE: Images are loaded in CPU memory (RAM); textures are loaded in GPU memory (VRAM)
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Bruno Cabral (@brccabral) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024-2025 Bruno Cabral (@brccabral) and Ramon Santamaria (@raysan5)
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

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - image channel");

    Image imFudesumi = LoadImage("resources/fudesumi.png");

    Image imFudesumiAlpha = ImageFromChannel(imFudesumi, 3);
    ImageAlphaMask(&imFudesumiAlpha, imFudesumiAlpha);

    Image imFudesumiRed = ImageFromChannel(imFudesumi, 0);
    ImageAlphaMask(&imFudesumiRed, imFudesumiAlpha);

    Image imFudesumiGreen = ImageFromChannel(imFudesumi, 1);
    ImageAlphaMask(&imFudesumiGreen, imFudesumiAlpha);

    Image imFudesumiBlue = ImageFromChannel(imFudesumi, 2);
    ImageAlphaMask(&imFudesumiBlue, imFudesumiAlpha);

    Image imBackground = GenImageChecked(screenWidth, screenHeight, screenWidth/20, screenHeight/20, ORANGE, YELLOW);

    Texture2D texFudesumi = LoadTextureFromImage(imFudesumi);
    Texture2D texFudesumiAlpha = LoadTextureFromImage(imFudesumiAlpha);
    Texture2D texFudesumiRed = LoadTextureFromImage(imFudesumiRed);
    Texture2D texFudesumiGreen = LoadTextureFromImage(imFudesumiGreen);
    Texture2D texFudesumiBlue = LoadTextureFromImage(imFudesumiBlue);
    Texture2D texBackground = LoadTextureFromImage(imBackground);

    UnloadImage(imFudesumi);
    UnloadImage(imFudesumiAlpha);
    UnloadImage(imFudesumiRed);
    UnloadImage(imFudesumiGreen);
    UnloadImage(imFudesumiBlue);
    UnloadImage(imBackground);

    Rectangle fudesumiRec = {0, 0, (float)imFudesumi.width, (float)imFudesumi.height};

    Rectangle fudesumiPos = {50, 10, imFudesumi.width*0.8f, imFudesumi.height*0.8f};
    Rectangle redPos = { 410, 10, fudesumiPos.width/2.0f, fudesumiPos.height/2.0f };
    Rectangle greenPos = { 600, 10, fudesumiPos.width/2.0f, fudesumiPos.height/2.0f };
    Rectangle bluePos = { 410, 230, fudesumiPos.width/2.0f, fudesumiPos.height/2.0f };
    Rectangle alphaPos = { 600, 230, fudesumiPos.width/2.0f, fudesumiPos.height/2.0f };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Nothing to update...
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            DrawTexture(texBackground, 0, 0, WHITE);
            DrawTexturePro(texFudesumi, fudesumiRec, fudesumiPos, (Vector2) {0, 0}, 0, WHITE);

            DrawTexturePro(texFudesumiRed, fudesumiRec, redPos, (Vector2) {0, 0}, 0, RED);
            DrawTexturePro(texFudesumiGreen, fudesumiRec, greenPos, (Vector2) {0, 0}, 0, GREEN);
            DrawTexturePro(texFudesumiBlue, fudesumiRec, bluePos, (Vector2) {0, 0}, 0, BLUE);
            DrawTexturePro(texFudesumiAlpha, fudesumiRec, alphaPos, (Vector2) {0, 0}, 0, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texBackground);
    UnloadTexture(texFudesumi);
    UnloadTexture(texFudesumiRed);
    UnloadTexture(texFudesumiGreen);
    UnloadTexture(texFudesumiBlue);
    UnloadTexture(texFudesumiAlpha);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
