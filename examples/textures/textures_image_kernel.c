/*******************************************************************************************
*
*   raylib [textures] example - Image loading and texture creation
*
*   NOTE: Images are loaded in CPU memory (RAM); textures are loaded in GPU memory (VRAM)
*
*   Example originally created with raylib 1.3, last time updated with raylib 1.3
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2023 Ramon Santamaria (@raysan5)
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

    Image image = LoadImage("resources/cat.png");     // Loaded in CPU memory (RAM)

    const int screenWidth = image.width*4;
    const int screenHeight = image.height;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - image convolution");

    float gaussiankernel[] = {1.0, 2.0, 1.0,
                    2.0, 4.0, 2.0,
                    1.0, 2.0, 1.0};

    float sobelkernel[] = {1.0, 0.0, -1.0,
                    2.0, 0.0, -2.0,
                    1.0, 0.0, -1.0};

    float sharpenkernel[] = {0.0, 1.0, 0.0,
                        -1.0, 5.0, -1.0,
                        0.0, -1.0, 0.0};

    Image catSharpend = ImageCopy(image);
    ImageKernelConvolution(&catSharpend, sharpenkernel, 3);
 

    Image catSobel = ImageCopy(image);
    ImageKernelConvolution(&catSobel, sobelkernel, 3);

    Image catGaussian = ImageCopy(image);
    for(int i = 0; i < 6; i++){
        ImageKernelConvolution(&catGaussian, gaussiankernel, 3);
    }


    Texture2D texture = LoadTextureFromImage(image);          // Image converted to texture, GPU memory (VRAM)
    Texture2D catSharpendTexture = LoadTextureFromImage(catSharpend);
    Texture2D catSobelTexture = LoadTextureFromImage(catSobel);
    Texture2D catGaussianTexture = LoadTextureFromImage(catGaussian);
    UnloadImage(image);   // Once image has been converted to texture and uploaded to VRAM, it can be unloaded from RAM
    UnloadImage(catGaussian);
    UnloadImage(catSobel);
    UnloadImage(catSharpend);

    SetTargetFPS(60);     // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

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

            DrawTexture(catSharpendTexture, 0, 0, WHITE);
            DrawTexture(catSobelTexture, texture.width, 0, WHITE);
            DrawTexture(catGaussianTexture, texture.width*2, 0, WHITE);
            DrawTexture(texture, texture.width*3, 0, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);       // Texture unloading
    UnloadTexture(catGaussianTexture);
    UnloadTexture(catSobelTexture);
    UnloadTexture(catSharpendTexture);

    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
