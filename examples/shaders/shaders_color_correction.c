/*******************************************************************************************
*
*   raylib [shaders] example - color correction
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Jordi Santonja (@JordSant) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Jordi Santonja (@JordSant)
*
********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                 // Required for GUI controls

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define MAX_TEXTURES 4

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - color correction");

    Texture2D texture[MAX_TEXTURES] = {
        LoadTexture("resources/parrots.png"),
        LoadTexture("resources/cat.png"),
        LoadTexture("resources/mandrill.png"),
        LoadTexture("resources/fudesumi.png")
    };

    Shader shdrColorCorrection = LoadShader(0, TextFormat("resources/shaders/glsl%i/color_correction.fs", GLSL_VERSION));

    int imageIndex = 0;
    int resetButtonClicked = 0;

    float contrast = 0.0f;
    float saturation = 0.0f;
    float brightness = 0.0f;

    // Get shader locations
    int contrastLoc = GetShaderLocation(shdrColorCorrection, "contrast");
    int saturationLoc = GetShaderLocation(shdrColorCorrection, "saturation");
    int brightnessLoc = GetShaderLocation(shdrColorCorrection, "brightness");

    // Set shader values (they can be changed later)
    SetShaderValue(shdrColorCorrection, contrastLoc, &contrast, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shdrColorCorrection, saturationLoc, &saturation, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shdrColorCorrection, brightnessLoc, &brightness, SHADER_UNIFORM_FLOAT);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Select texture to draw
        if (IsKeyPressed(KEY_ONE)) imageIndex = 0;
        else if (IsKeyPressed(KEY_TWO)) imageIndex = 1;
        else if (IsKeyPressed(KEY_THREE)) imageIndex = 2;
        else if (IsKeyPressed(KEY_FOUR)) imageIndex = 3;

        // Reset values to 0
        if (IsKeyPressed(KEY_R) || resetButtonClicked)
        {
            contrast = 0.0f;
            saturation = 0.0f;
            brightness = 0.0f;
        }

        // Send the values to the shader
        SetShaderValue(shdrColorCorrection, contrastLoc, &contrast, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shdrColorCorrection, saturationLoc, &saturation, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shdrColorCorrection, brightnessLoc, &brightness, SHADER_UNIFORM_FLOAT);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginShaderMode(shdrColorCorrection);

                DrawTexture(texture[imageIndex], 580/2 - texture[imageIndex].width/2, GetScreenHeight()/2 - texture[imageIndex].height/2, WHITE);

            EndShaderMode();

            DrawLine(580, 0, 580, GetScreenHeight(), (Color){ 218, 218, 218, 255 });
            DrawRectangle(580, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 232, 232, 232, 255 });

            // Draw UI info text
            DrawText("Color Correction", 585, 40, 20, GRAY);

            DrawText("Picture", 602, 75, 10, GRAY);
            DrawText("Press [1] - [4] to Change Picture", 600, 230, 8, GRAY);
            DrawText("Press [R] to Reset Values", 600, 250, 8, GRAY);

            // Draw GUI controls
            //------------------------------------------------------------------------------
            GuiToggleGroup((Rectangle){ 645, 70, 20, 20 }, "1;2;3;4", &imageIndex);

            GuiSliderBar((Rectangle){ 645, 100, 120, 20 }, "Contrast", TextFormat("%.0f", contrast), &contrast, -100.0f, 100.0f);
            GuiSliderBar((Rectangle){ 645, 130, 120, 20 }, "Saturation", TextFormat("%.0f", saturation), &saturation, -100.0f, 100.0f);
            GuiSliderBar((Rectangle){ 645, 160, 120, 20 }, "Brightness", TextFormat("%.0f", brightness), &brightness, -100.0f, 100.0f);

            resetButtonClicked = GuiButton((Rectangle){ 645, 190, 40, 20 }, "Reset");
            //------------------------------------------------------------------------------

            DrawFPS(710, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    for (int i = 0; i < MAX_TEXTURES; i++) UnloadTexture(texture[i]);
    UnloadShader(shdrColorCorrection);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
