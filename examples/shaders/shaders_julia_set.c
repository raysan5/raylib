/*******************************************************************************************
*
*   raylib [shaders] example - Render julia sets using a shader.
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3).
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Author: eggmund (https://github.com/eggmund)
*
********************************************************************************************/

#include "raylib.h"
#include <string.h> // For memcpy

// Speed when using auto
const float AUTO_SPEED = 0.0005;

// A few good julia sets
const float POINTS_OF_INTEREST[6][2] =
{ 
    {-0.348827, 0.607167},
    {-0.786268, 0.169728},
    {-0.8, 0.156},
    {0.285, 0.0},
    {-0.835, -0.2321},
    {-0.70176, -0.3842},
};

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1280;
    int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - julia set renderer");

    // If julia set is rendered for this frame.
    bool rendered = false;

    bool showControls = true;

    // Multiplier of speed to change c value. Set to 3 to start off with.
    int incrementSpeed = 3;

    // Offset and zoom to draw the julia set at. (centered on screen and 1.6 times smaller)
    float offset[2] = { -(float)screenWidth/2, -(float)screenHeight/2 }; 
    float zoom = 1.6;

    // c constant to use in z^2 + c
    float c[2];
    // Copy a point of interest into the c variable. 4 bytes per float (32 bits).
    memcpy(c, &POINTS_OF_INTEREST[0], 8);

    // Load julia set shader
    // NOTE: Defining 0 (NULL) for vertex shader forces usage of internal default vertex shader
    Shader shader = LoadShader(0, "resources/shaders/glsl330/julia_shader.fs");
    
    // Get variable (uniform) location on the shader to connect with the program
    // NOTE: If uniform variable could not be found in the shader, function returns -1
    // The location of c will be stored since we will need to change this whenever c changes
    int cLoc = GetShaderLocation(shader, "c");

    // Tell the shader what the screen dimensions, zoom, offset and c are
    float screenDims[2] = { (float)screenWidth, (float)screenHeight };
    SetShaderValue(shader, GetShaderLocation(shader, "screenDims"), screenDims, UNIFORM_VEC2);
    SetShaderValue(shader, GetShaderLocation(shader, "zoom"), &zoom, UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "offset"), offset, UNIFORM_VEC2);

    SetShaderValue(shader, cLoc, c, UNIFORM_VEC2);
    
    // Create a RenderTexture2D to be used for render to texture
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    
    SetTargetFPS(60);                       // Set the window to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // Get input
        //----------------------------------------------------------------------------------

        // Press 0 - 4 to reset c to a point of interest.
        if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_FIVE) || IsKeyPressed(KEY_SIX))
        {
            if (IsKeyPressed(KEY_ONE))
            {
                memcpy(c, &POINTS_OF_INTEREST[0], 8);
            }
            else if (IsKeyPressed(KEY_TWO))
            {
                memcpy(c, &POINTS_OF_INTEREST[1], 8);
            }
            else if (IsKeyPressed(KEY_THREE))
            {
                memcpy(c, &POINTS_OF_INTEREST[2], 8);
            }
            else if (IsKeyPressed(KEY_FOUR))
            {
                memcpy(c, &POINTS_OF_INTEREST[3], 8);
            }
            else if (IsKeyPressed(KEY_FIVE))
            {
                memcpy(c, &POINTS_OF_INTEREST[4], 8);
            }
            else if (IsKeyPressed(KEY_SIX))
            {
                memcpy(c, &POINTS_OF_INTEREST[5], 8);
            }
            SetShaderValue(shader, cLoc, c, UNIFORM_VEC2);
            rendered = false;  // c value has changed, so render the set again.
        }

        // Press "r" to stop changing c
        if (IsKeyPressed(KEY_R))
        {
            incrementSpeed = 0;
        }

        // Toggle whether or not to show controls
        if (IsKeyPressed(KEY_H))
        {
            showControls = !showControls;
        }

        // Scroll to change c increment speed.
        int mouseMv = GetMouseWheelMove();  // Get the amount the mouse has moved this frame
        if (mouseMv != 0)
        {
            if (IsKeyDown(KEY_LEFT_SHIFT))
            {
                incrementSpeed += mouseMv * 10;
            }
            else
            {
                incrementSpeed += mouseMv;
            }
            rendered = false;
        }

        if (incrementSpeed != 0)
        {
            float amount = GetFrameTime() * incrementSpeed * AUTO_SPEED;
            c[0] += amount;
            c[1] += amount;

            // Update the c value in the shader.
            SetShaderValue(shader, cLoc, c, UNIFORM_VEC2);
            rendered = false;
        }
        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);  // Clear the screen of the previous frame.
            
            // If the c value has changed, redraw the julia set using the shader, onto the render texture.
            if (!rendered)
            {
                BeginTextureMode(target);   // Enable drawing to texture

                    ClearBackground(BLACK); // Clear the last frame drawn on the texture.

                    // Draw a rectangle in shader mode. This acts as a canvas for the shader to draw on.
                    BeginShaderMode(shader);
                        DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);
                    EndShaderMode();

                EndTextureMode();

                rendered = true; // The set is now rendered, so do not compute it again until it next changes.
            }

            // Draw the saved texture (rendered julia set).
            DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, target.texture.height }, (Vector2){ 0, 0 }, WHITE);
            
            // Print information.
            DrawText( FormatText("cx: %f\ncy: %f\nspeed: %d", c[0], c[1], incrementSpeed), 10, 10, 20, RAYWHITE );

            if (showControls)
            {
                DrawText("Press keys 1 - 6 to change point of interest.", 10, screenHeight - 88, 20, RAYWHITE);
                DrawText("Use the scroll wheel to auto increment the c value. Hold shift while scrolling to increase speed by 10.", 10, screenHeight - 66, 20, RAYWHITE);
                DrawText("Press 'r' to reset speed.", 10, screenHeight - 44, 20, RAYWHITE);
                DrawText("Press 'h' to hide these controls.", 10, screenHeight - 22, 20, RAYWHITE);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);           // Unload shader
    UnloadRenderTexture(target);    // Unload render texture

    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
