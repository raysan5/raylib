/*******************************************************************************************
*
*   raylib [shaders] example - Rounded Rectangle
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Anstro Pleuton (@anstropleuton) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025-2025 Anstro Pleuton (@anstropleuton)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - Rounded Rectangle");

    // Shader loading
    //--------------------------------------------------------------------------------------
    // Load the rectangle shader which will draw the rectangle with rounded corners
    Shader rectangleShader = LoadShader(TextFormat("resources/shaders/glsl%i/base.vs", GLSL_VERSION),
                                        TextFormat("resources/shaders/glsl%i/rounded_rectangle.fs", GLSL_VERSION));

    // Organized into an array for easy access
    int rectangleShaderLocs[4] = {
        GetShaderLocation(rectangleShader, "rectangle"),  // vec4, rectangle bounds (x, y, width, height, NOTE: Y axis is flipped)
        GetShaderLocation(rectangleShader, "radius"),     // vec4, radius corners (top-left, top-right, bottom-left, bottom-right)
        GetShaderLocation(rectangleShader, "aaPower"),    // float, anti-aliasing power
        GetShaderLocation(rectangleShader, "aaDistance")  // float, anti-aliasing distance
    };

    // Load the rectangle shadow shader which will draw the rectangle's shadow
    Shader rectangleShadowShader = LoadShader(TextFormat("resources/shaders/glsl%i/base.vs", GLSL_VERSION),
                                              TextFormat("resources/shaders/glsl%i/rounded_rectangle_shadow.fs", GLSL_VERSION));

    int rectangleShadowShaderLocs[8] = {
        GetShaderLocation(rectangleShadowShader, "rectangle"),      // vec4, rectangle bounds (x, y, width, height)
        GetShaderLocation(rectangleShadowShader, "radius"),         // vec4, radius corners (top-left, top-right, bottom-left, bottom-right)
        GetShaderLocation(rectangleShadowShader, "shadowRadius"),   // float, shadow radius
        GetShaderLocation(rectangleShadowShader, "shadowPower"),    // float, shadow power
        GetShaderLocation(rectangleShadowShader, "shadowOffset"),   // vec2, shadow offset (NOTE: Y axis is flipped)
        GetShaderLocation(rectangleShadowShader, "shadowScale"),    // float, shadow scale
        GetShaderLocation(rectangleShadowShader, "aaPower"),        // float, anti-aliasing power
        GetShaderLocation(rectangleShadowShader, "aaDistance")      // float, anti-aliasing distance
    };

    // Load the rectangle border shader which will draw the rectangle's border
    Shader rectangleBorderShader = LoadShader(TextFormat("resources/shaders/glsl%i/base.vs", GLSL_VERSION),
                                              TextFormat("resources/shaders/glsl%i/rounded_rectangle_border.fs", GLSL_VERSION));

    int rectangleBorderShaderLocs[5] = {
        GetShaderLocation(rectangleBorderShader, "rectangle"),          // vec4, rectangle bounds (x, y, width, height)
        GetShaderLocation(rectangleBorderShader, "radius"),             // vec4, radius corners (top-left, top-right, bottom-left, bottom-right)
        GetShaderLocation(rectangleBorderShader, "borderThickness"),    // float, border thickness
        GetShaderLocation(rectangleBorderShader, "aaPower"),            // float, anti-aliasing power
        GetShaderLocation(rectangleBorderShader, "aaDistance")          // float, anti-aliasing distance
    };
    //--------------------------------------------------------------------------------------

    // Set parameters
    //--------------------------------------------------------------------------------------
    // Set 4 radius values for the rounded corners in pixels (top-left, top-right, bottom-left, bottom-right)
    SetShaderValue(rectangleShader, rectangleShaderLocs[1], (float[]){ 5.0f, 10.0f, 15.0f, 20.0f }, SHADER_UNIFORM_VEC4);
    SetShaderValue(rectangleShadowShader, rectangleShadowShaderLocs[1], (float[]){ 5.0f, 10.0f, 15.0f, 20.0f }, SHADER_UNIFORM_VEC4);
    SetShaderValue(rectangleBorderShader, rectangleBorderShaderLocs[1], (float[]){ 5.0f, 10.0f, 15.0f, 20.0f }, SHADER_UNIFORM_VEC4);

    // Set shadow parameters (in pixels)
    SetShaderValue(rectangleShadowShader, rectangleShadowShaderLocs[2], (float[]){ 20.0f }, SHADER_UNIFORM_FLOAT);      // Shadow radius
    SetShaderValue(rectangleShadowShader, rectangleShadowShaderLocs[3], (float[]){ 1.5f }, SHADER_UNIFORM_FLOAT);       // Shadow power
    SetShaderValue(rectangleShadowShader, rectangleShadowShaderLocs[4], (float[]){ 0.0f, -5.0f }, SHADER_UNIFORM_VEC2); // Shadow offset
    SetShaderValue(rectangleShadowShader, rectangleShadowShaderLocs[5], (float[]){ 0.95f }, SHADER_UNIFORM_FLOAT);      // Shadow scale

    // Set border parameters (in pixels)
    SetShaderValue(rectangleBorderShader, rectangleBorderShaderLocs[2], (float[]){ 5.0f }, SHADER_UNIFORM_FLOAT);       // Border thickness

    // Set anti-aliasing (power and distance) parameters for all shaders
    SetShaderValue(rectangleShader, rectangleShaderLocs[2], (float[]){ 1.5f }, SHADER_UNIFORM_FLOAT);
    SetShaderValue(rectangleShader, rectangleShaderLocs[3], (float[]){ 1.0f }, SHADER_UNIFORM_FLOAT);
    SetShaderValue(rectangleShadowShader, rectangleShadowShaderLocs[6], (float[]){ 1.5f }, SHADER_UNIFORM_FLOAT);
    SetShaderValue(rectangleShadowShader, rectangleShadowShaderLocs[7], (float[]){ 1.0f }, SHADER_UNIFORM_FLOAT);
    SetShaderValue(rectangleBorderShader, rectangleBorderShaderLocs[3], (float[]){ 1.5f }, SHADER_UNIFORM_FLOAT);
    SetShaderValue(rectangleBorderShader, rectangleBorderShaderLocs[4], (float[]){ 1.0f }, SHADER_UNIFORM_FLOAT);

    //--------------------------------------------------------------------------------------

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // NOTE: Draw rectangle's shadow first using shader
            DrawRectangleLines(30, 50, 150, 100, DARKGRAY);
            DrawText("Rectangle shadow shader", 30, 35, 10, DARKGRAY);

            Rectangle rec = { 50, 70, 110, 60 };

            // Flip Y axis to match shader coordinate system
            rec.y = GetScreenHeight() - rec.y - rec.height;

            SetShaderValue(rectangleShadowShader, rectangleShadowShaderLocs[0], (float[]){ rec.x, rec.y, rec.width, rec.height }, SHADER_UNIFORM_VEC4);

            BeginShaderMode(rectangleShadowShader);
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), DARKBLUE);
            EndShaderMode();

            // Draw rectangle box with rounded corners using shader
            DrawRectangleLines(30, 180, 150, 100, DARKGRAY);
            DrawText("Rounded rectangle shader", 30, 165, 10, DARKGRAY);

            rec = (Rectangle){ 50, 200, 110, 60 };
            rec.y = GetScreenHeight() - rec.y - rec.height;

            SetShaderValue(rectangleShader, rectangleShaderLocs[0], (float[]){ rec.x, rec.y, rec.width, rec.height }, SHADER_UNIFORM_VEC4);

            BeginShaderMode(rectangleShader);
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);
            EndShaderMode();

            // Draw rectangle's border using shader
            DrawRectangleLines(30, 310, 150, 100, DARKGRAY);
            DrawText("Rectangle border shader", 30, 295, 10, DARKGRAY);

            rec = (Rectangle){ 50, 330, 110, 60 };
            rec.y = GetScreenHeight() - rec.y - rec.height;

            SetShaderValue(rectangleBorderShader, rectangleBorderShaderLocs[0], (float[]){ rec.x, rec.y, rec.width, rec.height }, SHADER_UNIFORM_VEC4);

            BeginShaderMode(rectangleBorderShader);
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), SKYBLUE);
            EndShaderMode();

            // Draw one more rectangle with all three combined
            //--------------------------------------------------------------------------------------------------
            DrawRectangleLines(210, 50, 560, 360, DARKGRAY);
            DrawText("Rectangle all three combined", 210, 35, 10, DARKGRAY);

            rec = (Rectangle){ 240, 80, 500, 300 };
            rec.y = GetScreenHeight() - rec.y - rec.height;

            // Draw shadow
            SetShaderValue(rectangleShadowShader, rectangleShadowShaderLocs[0], (float[]){ rec.x, rec.y, rec.width, rec.height }, SHADER_UNIFORM_VEC4);

            BeginShaderMode(rectangleShadowShader);
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), DARKBLUE);
            EndShaderMode();

            // Draw rectangle
            SetShaderValue(rectangleShader, rectangleShaderLocs[0], (float[]){ rec.x, rec.y, rec.width, rec.height }, SHADER_UNIFORM_VEC4);

            BeginShaderMode(rectangleShader);
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);
            EndShaderMode();

            // Draw border
            SetShaderValue(rectangleBorderShader, rectangleBorderShaderLocs[0], (float[]){ rec.x, rec.y, rec.width, rec.height }, SHADER_UNIFORM_VEC4);

            BeginShaderMode(rectangleBorderShader);
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), SKYBLUE);
            EndShaderMode();
            //--------------------------------------------------------------------------------------------------

            DrawText("(c) Rounded rectangle SDF by Iñigo Quilez. MIT License.", GetScreenWidth() - 300, GetScreenHeight() - 20, 10, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // Unload shader
    UnloadShader(rectangleShader);
    UnloadShader(rectangleShadowShader);
    UnloadShader(rectangleBorderShader);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}