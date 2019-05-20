/*******************************************************************************************
*
*   raylib [shaders] example - Color palette switch
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3), to test this example
*         on OpenGL ES 2.0 platforms (Android, Raspberry Pi, HTML5), use #version 100 shaders
*         raylib comes with shaders ready for both versions, check raylib/shaders install folder
*
*   This example has been created using raylib 2.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Marco Lizza (@MarcoLizza) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2019 Marco Lizza (@MarcoLizza) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define MAX_PALETTES            3
#define COLORS_PER_PALETTE      8
#define VALUES_PER_COLOR        3

static const int palettes[MAX_PALETTES][COLORS_PER_PALETTE*VALUES_PER_COLOR] = {
    {   // 3-BIT RGB
        0, 0, 0,
        255, 0, 0,
        0, 255, 0,
        0, 0, 255,
        0, 255, 255,
        255, 0, 255,
        255, 255, 0,
        255, 255, 255,
    },
    {   // AMMO-8 (GameBoy-like)
        4, 12, 6,
        17, 35, 24,
        30, 58, 41,
        48, 93, 66,
        77, 128, 97,
        137, 162, 87,
        190, 220, 127,
        238, 255, 204,
    },
    {   // RKBV (2-strip film)
        21, 25, 26,
        138, 76, 88,
        217, 98, 117,
        230, 184, 193,
        69, 107, 115,
        75, 151, 166,
        165, 189, 194,
        255, 245, 247,
    }
};

static const char *paletteText[] = {
    "3-BIT RGB",
    "AMMO-8 (GameBoy-like)",
    "RKBV (2-strip film)"
};

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - color palette switch");

    // Load shader to be used on some parts drawing
    // NOTE 1: Using GLSL 330 shader version, on OpenGL ES 2.0 use GLSL 100 shader version
    // NOTE 2: Defining 0 (NULL) for vertex shader forces usage of internal default vertex shader
    Shader shader = LoadShader(0, FormatText("resources/shaders/glsl%i/palette_switch.fs", GLSL_VERSION));

    // Get variable (uniform) location on the shader to connect with the program
    // NOTE: If uniform variable could not be found in the shader, function returns -1
    int paletteLoc = GetShaderLocation(shader, "palette");

    int currentPalette = 0;
    int lineHeight = screenHeight/COLORS_PER_PALETTE;

    SetTargetFPS(60);                       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_RIGHT)) currentPalette++;
        else if (IsKeyPressed(KEY_LEFT)) currentPalette--;

        if (currentPalette >= MAX_PALETTES) currentPalette = 0;
        else if (currentPalette < 0) currentPalette = MAX_PALETTES - 1;

        // Send new value to the shader to be used on drawing.
        // NOTE: We are sending RGB triplets w/o the alpha channel
        SetShaderValueV(shader, paletteLoc, palettes[currentPalette], UNIFORM_IVEC3, COLORS_PER_PALETTE);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginShaderMode(shader);

                for (int i = 0; i < COLORS_PER_PALETTE; i++)
                {
                    // Draw horizontal screen-wide rectangles with increasing "palette index"
                    // The used palette index is encoded in the RGB components of the pixel
                    DrawRectangle(0, lineHeight*i, GetScreenWidth(), lineHeight, (Color){ i, i, i, 255 });
                }

            EndShaderMode();

            DrawText("< >", 10, 10, 30, DARKBLUE);
            DrawText("CURRENT PALETTE:", 60, 15, 20, RAYWHITE);
            DrawText(paletteText[currentPalette], 300, 15, 20, RED);

            DrawFPS(700, 15);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);       // Unload shader

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
