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

    Image fudesumi_image = LoadImage("resources/fudesumi.png");
    Texture2D fudesumi_texture = LoadTextureFromImage(fudesumi_image);

    Image image_red = ImageFromChannel(fudesumi_image, 0, 0.0f);
    Texture2D texture_red = LoadTextureFromImage(image_red);
    UnloadImage(image_red);

    Image image_green = ImageFromChannel(fudesumi_image, 1, 0.0f);
    Texture2D texture_green = LoadTextureFromImage(image_green);
    UnloadImage(image_green);

    Image image_blue = ImageFromChannel(fudesumi_image, 2, 0.0f);
    Texture2D texture_blue = LoadTextureFromImage(image_blue);
    UnloadImage(image_blue);

    Image image_alpha = ImageFromChannel(fudesumi_image, 3, 0.0f);
    Texture2D texture_alpha = LoadTextureFromImage(image_alpha);
    UnloadImage(image_alpha);


    Image background_image = GenImageChecked(screenWidth, screenHeight, screenWidth/20, screenHeight/20, ORANGE, YELLOW);
    Texture2D background_texture = LoadTextureFromImage(background_image);
    UnloadImage(background_image);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    Rectangle fudesumi_rec = {0, 0, fudesumi_image.width, fudesumi_image.height};
    Rectangle red_pos = { 410, 50, fudesumi_image.width / 2, fudesumi_image.height / 2 };
    Rectangle green_pos = { 600, 50, fudesumi_image.width / 2, fudesumi_image.height / 2 };
    Rectangle blue_pos = { 410, 310, fudesumi_image.width / 2, fudesumi_image.height / 2 };
    Rectangle alpha_pos = { 600, 310, fudesumi_image.width / 2, fudesumi_image.height / 2 };

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            DrawTexture(background_texture, 0, 0, WHITE);
            DrawTexture(fudesumi_texture, 50, 50, WHITE);

            DrawTexturePro(texture_red, fudesumi_rec, red_pos, (Vector2) {0, 0}, 0, RED);
            DrawTexturePro(texture_green, fudesumi_rec, green_pos, (Vector2) {0, 0}, 0, GREEN);
            DrawTexturePro(texture_blue, fudesumi_rec, blue_pos, (Vector2) {0, 0}, 0, BLUE);
            DrawTexturePro(texture_alpha, fudesumi_rec, alpha_pos, (Vector2) {0, 0}, 0, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(background_texture);
    UnloadImage(fudesumi_image);
    UnloadTexture(fudesumi_texture);
    UnloadTexture(texture_red);
    UnloadTexture(texture_green);
    UnloadTexture(texture_blue);
    UnloadTexture(texture_alpha);
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
