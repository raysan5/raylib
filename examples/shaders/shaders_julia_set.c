/*******************************************************************************************
*
*   raylib [shaders] example - julia set
*
*   Example complexity rating: [★★★☆] 3/4
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3)
*
*   Example originally created with raylib 2.5, last time updated with raylib 4.0
*
*   Example contributed by Josh Colclough (@joshcol9232) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2019-2025 Josh Colclough (@joshcol9232) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

// A few good julia sets
const float pointsOfInterest[6][2] =
{
    { -0.348827f, 0.607167f },
    { -0.786268f, 0.169728f },
    { -0.8f, 0.156f },
    { 0.285f, 0.0f },
    { -0.835f, -0.2321f },
    { -0.70176f, -0.3842f },
};

const int screenWidth = 800;
const int screenHeight = 450;
const float zoomSpeed = 1.01f;
const float offsetSpeedMul = 2.0f;

const float startingZoom = 0.75f;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - julia set");

    // Load julia set shader
    // NOTE: Defining 0 (NULL) for vertex shader forces usage of internal default vertex shader
    Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/julia_set.fs", GLSL_VERSION));

    // Create a RenderTexture2D to be used for render to texture
    RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    // c constant to use in z^2 + c
    float c[2] = { pointsOfInterest[0][0], pointsOfInterest[0][1] };

    // Offset and zoom to draw the julia set at. (centered on screen and default size)
    float offset[2] = { 0.0f, 0.0f };
    float zoom = startingZoom;

    // Get variable (uniform) locations on the shader to connect with the program
    // NOTE: If uniform variable could not be found in the shader, function returns -1
    int cLoc = GetShaderLocation(shader, "c");
    int zoomLoc = GetShaderLocation(shader, "zoom");
    int offsetLoc = GetShaderLocation(shader, "offset");

    // Upload the shader uniform values!
    SetShaderValue(shader, cLoc, c, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, zoomLoc, &zoom, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, offsetLoc, offset, SHADER_UNIFORM_VEC2);

    int incrementSpeed = 0;             // Multiplier of speed to change c value
    bool showControls = true;           // Show controls

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Press [1 - 6] to reset c to a point of interest
        if (IsKeyPressed(KEY_ONE) ||
            IsKeyPressed(KEY_TWO) ||
            IsKeyPressed(KEY_THREE) ||
            IsKeyPressed(KEY_FOUR) ||
            IsKeyPressed(KEY_FIVE) ||
            IsKeyPressed(KEY_SIX))
        {
            if (IsKeyPressed(KEY_ONE)) c[0] = pointsOfInterest[0][0], c[1] = pointsOfInterest[0][1];
            else if (IsKeyPressed(KEY_TWO)) c[0] = pointsOfInterest[1][0], c[1] = pointsOfInterest[1][1];
            else if (IsKeyPressed(KEY_THREE)) c[0] = pointsOfInterest[2][0], c[1] = pointsOfInterest[2][1];
            else if (IsKeyPressed(KEY_FOUR)) c[0] = pointsOfInterest[3][0], c[1] = pointsOfInterest[3][1];
            else if (IsKeyPressed(KEY_FIVE)) c[0] = pointsOfInterest[4][0], c[1] = pointsOfInterest[4][1];
            else if (IsKeyPressed(KEY_SIX)) c[0] = pointsOfInterest[5][0], c[1] = pointsOfInterest[5][1];

            SetShaderValue(shader, cLoc, c, SHADER_UNIFORM_VEC2);
        }

        // If "R" is pressed, reset zoom and offset
        if (IsKeyPressed(KEY_R))
        {
            zoom = startingZoom;
            offset[0] = 0.0f;
            offset[1] = 0.0f;
            SetShaderValue(shader, zoomLoc, &zoom, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, offsetLoc, offset, SHADER_UNIFORM_VEC2);
        }

        if (IsKeyPressed(KEY_SPACE)) incrementSpeed = 0;         // Pause animation (c change)
        if (IsKeyPressed(KEY_F1)) showControls = !showControls;  // Toggle whether or not to show controls

        if (IsKeyPressed(KEY_RIGHT)) incrementSpeed++;
        else if (IsKeyPressed(KEY_LEFT)) incrementSpeed--;

        // If either left or right button is pressed, zoom in/out
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            // Change zoom. If Mouse left -> zoom in. Mouse right -> zoom out
            zoom *= IsMouseButtonDown(MOUSE_BUTTON_LEFT)? zoomSpeed : 1.0f/zoomSpeed;

            const Vector2 mousePos = GetMousePosition();
            Vector2 offsetVelocity;
            // Find the velocity at which to change the camera. Take the distance of the mouse
            // from the center of the screen as the direction, and adjust magnitude based on the current zoom
            offsetVelocity.x = (mousePos.x/(float)screenWidth - 0.5f)*offsetSpeedMul/zoom;
            offsetVelocity.y = (mousePos.y/(float)screenHeight - 0.5f)*offsetSpeedMul/zoom;

            // Apply move velocity to camera
            offset[0] += GetFrameTime()*offsetVelocity.x;
            offset[1] += GetFrameTime()*offsetVelocity.y;

            // Update the shader uniform values!
            SetShaderValue(shader, zoomLoc, &zoom, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, offsetLoc, offset, SHADER_UNIFORM_VEC2);
        }

        // Increment c value with time
        const float dc = GetFrameTime()*(float)incrementSpeed*0.0005f;
        c[0] += dc;
        c[1] += dc;
        SetShaderValue(shader, cLoc, c, SHADER_UNIFORM_VEC2);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        // Using a render texture to draw Julia set
        BeginTextureMode(target);       // Enable drawing to texture
            ClearBackground(BLACK);     // Clear the render texture

            // Draw a rectangle in shader mode to be used as shader canvas
            // NOTE: Rectangle uses font white character texture coordinates,
            // so shader can not be applied here directly because input vertexTexCoord
            // do not represent full screen coordinates (space where want to apply shader)
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);     // Clear screen background

            // Draw the saved texture and rendered julia set with shader
            // NOTE: We do not invert texture on Y, already considered inside shader
            BeginShaderMode(shader);
                // WARNING: If FLAG_WINDOW_HIGHDPI is enabled, HighDPI monitor scaling should be considered
                // when rendering the RenderTexture2D to fit in the HighDPI scaled Window
                DrawTextureEx(target.texture, (Vector2){ 0.0f, 0.0f }, 0.0f, 1.0f, WHITE);
            EndShaderMode();

            if (showControls)
            {
                DrawText("Press Mouse buttons right/left to zoom in/out and move", 10, 15, 10, RAYWHITE);
                DrawText("Press KEY_F1 to toggle these controls", 10, 30, 10, RAYWHITE);
                DrawText("Press KEYS [1 - 6] to change point of interest", 10, 45, 10, RAYWHITE);
                DrawText("Press KEY_LEFT | KEY_RIGHT to change speed", 10, 60, 10, RAYWHITE);
                DrawText("Press KEY_SPACE to stop movement animation", 10, 75, 10, RAYWHITE);
                DrawText("Press KEY_R to recenter the camera", 10, 90, 10, RAYWHITE);
            }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);               // Unload shader
    UnloadRenderTexture(target);        // Unload render texture

    CloseWindow();                      // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
