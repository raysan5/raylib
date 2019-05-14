/*******************************************************************************************
*
*   raylib [shaders] example - Texture Waves
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3), to test this example
*         on OpenGL ES 2.0 platforms (Android, Raspberry Pi, HTML5), use #version 100 shaders
*         raylib comes with shaders ready for both versions, check raylib/shaders install folder
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Anata (@anatagawa) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2019 Anata (@anatagawa) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

// -------------------------------------------------------------------------------------------------------------
// Main Entry point
// -------------------------------------------------------------------------------------------------------------
int main() 
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib [shaders] example - texture waves");
	
    // Load space texture to apply shaders
	Texture2D space = LoadTexture("resources/space.png");
	
    // Load shader and setup location points and values
    Shader wave = LoadShader(0, FormatText("resources/shaders/glsl%i/wave.fs", GLSL_VERSION));

	float screenSizeLoc = GetShaderLocation(wave, "size");
	float secondsLoc = GetShaderLocation(wave, "secondes");
	float freqXLoc = GetShaderLocation(wave, "freqX");
	float freqYLoc = GetShaderLocation(wave, "freqY");
	float ampXLoc = GetShaderLocation(wave, "ampX");
	float ampYLoc = GetShaderLocation(wave, "ampY");
	float speedXLoc = GetShaderLocation(wave, "speedX");
	float speedYLoc = GetShaderLocation(wave, "speedY");

	float screenSize[2] = { 800, 450 };

    // Shader uniform values that can be updated at any time
	float freqX = 25.0f;
	float freqY = 25.0f;
	float ampX = 5.0f;
	float ampY = 5.0f;
	float speedX = 8.0f;
	float speedY = 8.0f;

	SetShaderValue(wave, screenSizeLoc, &screenSize, UNIFORM_VEC2);
	SetShaderValue(wave, freqXLoc, &freqX, UNIFORM_FLOAT);
	SetShaderValue(wave, freqYLoc, &freqY, UNIFORM_FLOAT);
	SetShaderValue(wave, ampXLoc, &ampX, UNIFORM_FLOAT);
	SetShaderValue(wave, ampYLoc, &ampY, UNIFORM_FLOAT);
	SetShaderValue(wave, speedXLoc, &speedX, UNIFORM_FLOAT);
	SetShaderValue(wave, speedYLoc, &speedY, UNIFORM_FLOAT);

	float seconds = 0.0f;
    
    SetTargetFPS(60);
	// -------------------------------------------------------------------------------------------------------------
    
    // Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
		seconds += GetFrameTime();
        
		SetShaderValue(wave, secondsLoc, &seconds, UNIFORM_FLOAT);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
		BeginDrawing();

			ClearBackground(RAYWHITE);

			BeginShaderMode(wave);
            
				DrawTexture(space, 0, 0, WHITE);
				DrawTexture(space, space.width, 0, WHITE);
                
			EndShaderMode();

		EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(wave);         // Unload shader
    UnloadTexture(space);       // Unload texture
    
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

	return 0;
}
