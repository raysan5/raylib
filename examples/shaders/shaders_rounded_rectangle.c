/*******************************************************************************************
*
*   raylib [shaders] example - rounded rectangle
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Anstro Pleuton (@anstropleuton) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Anstro Pleuton (@anstropleuton)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Rounded rectangle data
typedef struct {
    Vector4 cornerRadius; // Individual corner radius (top-left, top-right, bottom-left, bottom-right)

    // Shadow variables
    float shadowRadius;
    Vector2 shadowOffset;
    float shadowScale;

    // Border variables
    float borderThickness; // Inner-border thickness

    // Shader locations
    int rectangleLoc;
    int radiusLoc;
    int colorLoc;
    int shadowRadiusLoc;
    int shadowOffsetLoc;
    int shadowScaleLoc;
    int shadowColorLoc;
    int borderThicknessLoc;
    int borderColorLoc;
} RoundedRectangle;

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
// Create a rounded rectangle and set uniform locations
static RoundedRectangle CreateRoundedRectangle(Vector4 cornerRadius, float shadowRadius, Vector2 shadowOffset, float shadowScale, float borderThickness, Shader shader);

// Update rounded rectangle uniforms
static void UpdateRoundedRectangle(RoundedRectangle rec, Shader shader);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - rounded rectangle");

    // Load the shader
    Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/base.vs", GLSL_VERSION),
                               TextFormat("resources/shaders/glsl%i/rounded_rectangle.fs", GLSL_VERSION));

    // Create a rounded rectangle
    RoundedRectangle roundedRectangle = CreateRoundedRectangle(
        (Vector4){ 5.0f, 10.0f, 15.0f, 20.0f },     // Corner radius
        20.0f,                                      // Shadow radius
        (Vector2){ 0.0f, -5.0f },                   // Shadow offset
        0.95f,                                      // Shadow scale
        5.0f,                                       // Border thickness
        shader                                      // Shader
    );

    // Update shader uniforms
    UpdateRoundedRectangle(roundedRectangle, shader);

    const Color rectangleColor = BLUE;
    const Color shadowColor = DARKBLUE;
    const Color borderColor = SKYBLUE;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw rectangle box with rounded corners using shader
            Rectangle rec = { 50, 70, 110, 60 };
            DrawRectangleLines((int)rec.x - 20, (int)rec.y - 20, (int)rec.width + 40, (int)rec.height + 40, DARKGRAY);
            DrawText("Rounded rectangle", (int)rec.x - 20, (int)rec.y - 35, 10, DARKGRAY);

            // Flip Y axis to match shader coordinate system
            rec.y = screenHeight - rec.y - rec.height;
            SetShaderValue(shader, roundedRectangle.rectangleLoc, (float[]){ rec.x, rec.y, rec.width, rec.height }, SHADER_UNIFORM_VEC4);

            // Only rectangle color
            SetShaderValue(shader, roundedRectangle.colorLoc, (float[]) { rectangleColor.r/255.0f, rectangleColor.g/255.0f, rectangleColor.b/255.0f, rectangleColor.a/255.0f }, SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, roundedRectangle.shadowColorLoc, (float[]) { 0.0f, 0.0f, 0.0f, 0.0f }, SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, roundedRectangle.borderColorLoc, (float[]) { 0.0f, 0.0f, 0.0f, 0.0f }, SHADER_UNIFORM_VEC4);

            BeginShaderMode(shader);
                DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
            EndShaderMode();

            // Draw rectangle shadow using shader
            rec = (Rectangle){ 50, 200, 110, 60 };
            DrawRectangleLines((int)rec.x - 20, (int)rec.y - 20, (int)rec.width + 40, (int)rec.height + 40, DARKGRAY);
            DrawText("Rounded rectangle shadow", (int)rec.x - 20, (int)rec.y - 35, 10, DARKGRAY);

            rec.y = screenHeight - rec.y - rec.height;
            SetShaderValue(shader, roundedRectangle.rectangleLoc, (float[]){ rec.x, rec.y, rec.width, rec.height }, SHADER_UNIFORM_VEC4);

            // Only shadow color
            SetShaderValue(shader, roundedRectangle.colorLoc, (float[]) { 0.0f, 0.0f, 0.0f, 0.0f }, SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, roundedRectangle.shadowColorLoc, (float[]) { shadowColor.r/255.0f, shadowColor.g/255.0f, shadowColor.b/255.0f, shadowColor.a/255.0f }, SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, roundedRectangle.borderColorLoc, (float[]) { 0.0f, 0.0f, 0.0f, 0.0f }, SHADER_UNIFORM_VEC4);

            BeginShaderMode(shader);
                DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
            EndShaderMode();

            // Draw rectangle's border using shader
            rec = (Rectangle){ 50, 330, 110, 60 };
            DrawRectangleLines((int)rec.x - 20, (int)rec.y - 20, (int)rec.width + 40, (int)rec.height + 40, DARKGRAY);
            DrawText("Rounded rectangle border", (int)rec.x - 20, (int)rec.y - 35, 10, DARKGRAY);

            rec.y = screenHeight - rec.y - rec.height;
            SetShaderValue(shader, roundedRectangle.rectangleLoc, (float[]){ rec.x, rec.y, rec.width, rec.height }, SHADER_UNIFORM_VEC4);

            // Only border color
            SetShaderValue(shader, roundedRectangle.colorLoc, (float[]) { 0.0f, 0.0f, 0.0f, 0.0f }, SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, roundedRectangle.shadowColorLoc, (float[]) { 0.0f, 0.0f, 0.0f, 0.0f }, SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, roundedRectangle.borderColorLoc, (float[]) { borderColor.r/255.0f, borderColor.g/255.0f, borderColor.b/255.0f, borderColor.a/255.0f }, SHADER_UNIFORM_VEC4);

            BeginShaderMode(shader);
                DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
            EndShaderMode();

            // Draw one more rectangle with all three colors
            rec = (Rectangle){ 240, 80, 500, 300 };
            DrawRectangleLines((int)rec.x - 30, (int)rec.y - 30, (int)rec.width + 60, (int)rec.height + 60, DARKGRAY);
            DrawText("Rectangle with all three combined", (int)rec.x - 30, (int)rec.y - 45, 10, DARKGRAY);

            rec.y = screenHeight - rec.y - rec.height;
            SetShaderValue(shader, roundedRectangle.rectangleLoc, (float[]){ rec.x, rec.y, rec.width, rec.height }, SHADER_UNIFORM_VEC4);

            // All three colors
            SetShaderValue(shader, roundedRectangle.colorLoc, (float[]) { rectangleColor.r/255.0f, rectangleColor.g/255.0f, rectangleColor.b/255.0f, rectangleColor.a/255.0f }, SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, roundedRectangle.shadowColorLoc, (float[]) { shadowColor.r/255.0f, shadowColor.g/255.0f, shadowColor.b/255.0f, shadowColor.a/255.0f }, SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, roundedRectangle.borderColorLoc, (float[]) { borderColor.r/255.0f, borderColor.g/255.0f, borderColor.b/255.0f, borderColor.a/255.0f }, SHADER_UNIFORM_VEC4);

            BeginShaderMode(shader);
                DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
            EndShaderMode();

            DrawText("(c) Rounded rectangle SDF by Iñigo Quilez. MIT License.", screenWidth - 300, screenHeight - 20, 10, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader); // Unload shader

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions
//------------------------------------------------------------------------------------

// Create a rounded rectangle and set uniform locations
RoundedRectangle CreateRoundedRectangle(Vector4 cornerRadius, float shadowRadius, Vector2 shadowOffset, float shadowScale, float borderThickness, Shader shader)
{
    RoundedRectangle rec;
    rec.cornerRadius = cornerRadius;
    rec.shadowRadius = shadowRadius;
    rec.shadowOffset = shadowOffset;
    rec.shadowScale = shadowScale;
    rec.borderThickness = borderThickness;

    // Get shader uniform locations
    rec.rectangleLoc = GetShaderLocation(shader, "rectangle");
    rec.radiusLoc = GetShaderLocation(shader, "radius");
    rec.colorLoc = GetShaderLocation(shader, "color");
    rec.shadowRadiusLoc = GetShaderLocation(shader, "shadowRadius");
    rec.shadowOffsetLoc = GetShaderLocation(shader, "shadowOffset");
    rec.shadowScaleLoc = GetShaderLocation(shader, "shadowScale");
    rec.shadowColorLoc = GetShaderLocation(shader, "shadowColor");
    rec.borderThicknessLoc = GetShaderLocation(shader, "borderThickness");
    rec.borderColorLoc = GetShaderLocation(shader, "borderColor");

    UpdateRoundedRectangle(rec, shader);

    return rec;
}

// Update rounded rectangle uniforms
void UpdateRoundedRectangle(RoundedRectangle rec, Shader shader)
{
    SetShaderValue(shader, rec.radiusLoc, (float[]){ rec.cornerRadius.x, rec.cornerRadius.y, rec.cornerRadius.z, rec.cornerRadius.w }, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, rec.shadowRadiusLoc, &rec.shadowRadius, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, rec.shadowOffsetLoc, (float[]){ rec.shadowOffset.x, rec.shadowOffset.y }, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, rec.shadowScaleLoc, &rec.shadowScale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, rec.borderThicknessLoc, &rec.borderThickness, SHADER_UNIFORM_FLOAT);
}
