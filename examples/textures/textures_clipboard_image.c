/*******************************************************************************************
*
*   raylib [textures] example - clipboard image
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Maicon Santana (@maiconpintoabreu) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026-2026 Maicon Santana (@maiconpintoabreu)
*
********************************************************************************************/

#define RCORE_PLATFORM_RGFW
#include "raylib.h"

#define MAX_IAMGE_COLLECTION_AMOUNT 1000

typedef struct ImageCollection {
    Texture2D texture;
    Vector2 position;
} ImageCollection;

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

    ImageCollection collection[MAX_IAMGE_COLLECTION_AMOUNT] = {0};
    int currentCollectionIndex = 0;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update variables / Implement example logic at this point
        //----------------------------------------------------------------------------------

        if(IsKeyPressed(KEY_R))
        {
            currentCollectionIndex = 0;

            // Unload Texture to avoid Memory leak
            for (int i=0;i<MAX_IAMGE_COLLECTION_AMOUNT;i++)
            {
                if (IsTextureValid(collection[i].texture))
                {
                    UnloadTexture(collection[i].texture);
                }
            }
        }

        if (currentCollectionIndex < MAX_IAMGE_COLLECTION_AMOUNT && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V))
        {
            Image image = GetClipboardImage();
            if (IsImageValid(image))
            {
                collection[currentCollectionIndex].texture = LoadTextureFromImage(image);
                collection[currentCollectionIndex].position = GetMousePosition();
                currentCollectionIndex++;
                UnloadImage(image);
            }
            else
            {
                TraceLog(LOG_INFO, "Nothing to paste here");
            }
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            for (int i=0;i<currentCollectionIndex;i++)
            {
                if (IsTextureValid(collection[i].texture))
                {
                    DrawTexturePro(
                        collection[i].texture, 
                        (Rectangle){0,0,collection[i].texture.width, collection[i].texture.height}, 
                        (Rectangle){collection[i].position.x,collection[i].position.y,collection[i].texture.width, collection[i].texture.height},
                        (Vector2){collection[i].texture.width*0.5f, collection[i].texture.height*0.5f}, 
                        0.0f, 
                        WHITE);
                }
            }

            DrawRectangle(0, 0, screenWidth, 40, BLACK);
            DrawText("Clipboard Image - Ctrl+V to Paste and R to Reset ", 120, 10, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    for (int i=0;i<MAX_IAMGE_COLLECTION_AMOUNT;i++)
    {
        if (IsTextureValid(collection[i].texture))
        {
            UnloadTexture(collection[i].texture);
        }
    }

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
