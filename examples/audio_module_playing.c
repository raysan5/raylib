/*******************************************************************************************
*
*   raylib [audio] example - Module playing (streaming)
*
*   NOTE: This example requires OpenAL Soft library installed
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_CIRCLES  64

typedef struct {
    Vector2 position;
    float radius;
    float alpha;
    float speed;
    Color color;
} CircleWave;

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - module playing (streaming)");

    InitAudioDevice();              // Initialize audio device
    
    Color colors[14] = { ORANGE, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK,
                         YELLOW, GREEN, SKYBLUE, PURPLE, BEIGE };
    
    // Creates ome circles for visual effect
    CircleWave circles[MAX_CIRCLES];
    
    for (int i = MAX_CIRCLES - 1; i >= 0; i--)
    {
        circles[i].alpha = 0.0f;
        circles[i].radius = GetRandomValue(10, 40);
        circles[i].position.x = GetRandomValue(circles[i].radius, screenWidth - circles[i].radius);
        circles[i].position.y = GetRandomValue(circles[i].radius, screenHeight - circles[i].radius);
        circles[i].speed = (float)GetRandomValue(1, 100)/20000.0f;
        circles[i].color = colors[GetRandomValue(0, 13)];
    }
    
    // Load postprocessing bloom shader
    Shader shader = LoadShader("resources/shaders/glsl330/base.vs", 
                               "resources/shaders/glsl330/bloom.fs");

    // Create a RenderTexture2D to be used for render to texture
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);

    PlayMusicStream(0, "resources/audio/2t2m_spa.xm");         // Play module stream

    float timePlayed = 0.0f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        for (int i = MAX_CIRCLES - 1; i >= 0; i--)
        {
            circles[i].alpha += circles[i].speed;
            circles[i].radius += circles[i].speed*10.0f;
            
            if (circles[i].alpha > 1.0f) circles[i].speed *= -1;
            
            if (circles[i].alpha <= 0.0f)
            {
                circles[i].alpha = 0.0f;
                circles[i].radius = GetRandomValue(10, 40);
                circles[i].position.x = GetRandomValue(circles[i].radius, screenWidth - circles[i].radius);
                circles[i].position.y = GetRandomValue(circles[i].radius, screenHeight - circles[i].radius);
                circles[i].color = colors[GetRandomValue(0, 13)];
                circles[i].speed = (float)GetRandomValue(1, 100)/20000.0f;
            }
        }

        // Get timePlayed scaled to bar dimensions
        timePlayed = (GetMusicTimePlayed(0)/GetMusicTimeLength(0)*(screenWidth - 40))*2;
        
        UpdateMusicStream(0);        // Update music buffer with new stream data
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);
            
            BeginTextureMode(target);   // Enable drawing to texture

                for (int i = MAX_CIRCLES - 1; i >= 0; i--)
                {
                    DrawCircleV(circles[i].position, circles[i].radius, Fade(circles[i].color, circles[i].alpha));
                }
                
            EndTextureMode();           // End drawing to texture (now we have a texture available for next passes)
            
            BeginShaderMode(shader);

                // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
                DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, -target.texture.height }, (Vector2){ 0, 0 }, WHITE);
                
            EndShaderMode();

            // Draw time bar
            DrawRectangle(20, screenHeight - 20 - 12, screenWidth - 40, 12, LIGHTGRAY);
            DrawRectangle(20, screenHeight - 20 - 12, (int)timePlayed, 12, MAROON);
            DrawRectangleLines(20, screenHeight - 20 - 12, screenWidth - 40, 12, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);           // Unload shader
    UnloadRenderTexture(target);    // Unload render texture
    
    CloseAudioDevice();     // Close audio device (music streaming is automatically stopped)

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}