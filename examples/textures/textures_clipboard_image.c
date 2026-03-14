/*******************************************************************************************
*
*   raylib [textures] example - clipboard image
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
*
*   Example contributed by Maicon Santana (@maiconpintoabreu) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Maicon Santana (@maiconpintoabreu)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_TEXTURE_COLLECTION  20

typedef struct TextureCollection {
    Texture2D texture;
    Vector2 position;
} TextureCollection;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - clipboard_image");

    TextureCollection collection[MAX_TEXTURE_COLLECTION] = { 0 };
    int currentCollectionIndex = 0;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_R))    // Reset image collection
        {
            // Unload textures to avoid memory leaks
            for (int i = 0; i < MAX_TEXTURE_COLLECTION; i++) UnloadTexture(collection[i].texture);
            
            currentCollectionIndex = 0;
        }

        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V) &&
            (currentCollectionIndex < MAX_TEXTURE_COLLECTION))
        {
            Image image = GetClipboardImage();
            
            if (IsImageValid(image))
            {
                collection[currentCollectionIndex].texture = LoadTextureFromImage(image);
                collection[currentCollectionIndex].position = GetMousePosition();
                currentCollectionIndex++;
                UnloadImage(image);
            }
            else TraceLog(LOG_INFO, "IMAGE: Could not retrieve image from clipboard");
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            for (int i = 0; i < currentCollectionIndex; i++)
            {
                if (IsTextureValid(collection[i].texture))
                {
                    DrawTexturePro(collection[i].texture, 
                        (Rectangle){0,0,collection[i].texture.width, collection[i].texture.height}, 
                        (Rectangle){collection[i].position.x,collection[i].position.y,collection[i].texture.width, collection[i].texture.height},
                        (Vector2){collection[i].texture.width*0.5f, collection[i].texture.height*0.5f}, 
                        0.0f, WHITE);
                }
            }

            DrawRectangle(0, 0, screenWidth, 40, BLACK);
            DrawText("Clipboard Image - Ctrl+V to Paste and R to Reset ", 120, 10, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    for (int i = 0; i < MAX_TEXTURE_COLLECTION;i ++)
    {
        UnloadTexture(collection[i].texture);
    }

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
