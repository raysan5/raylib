/*******************************************************************************************
*
*   raylib [shaders] example - julia sets
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3).
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by eggmund (@eggmund) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2019 eggmund (@eggmund) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

// A few good julia sets
const float POINTS_OF_INTEREST[6][2] =
{ 
    { -0.348827, 0.607167 },
    { -0.786268, 0.169728 },
    { -0.8, 0.156 },
    { 0.285, 0.0 },
    { -0.835, -0.2321 },
    { -0.70176, -0.3842 },
};

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - julia sets");

    // Load julia set shader
    // NOTE: Defining 0 (NULL) for vertex shader forces usage of internal default vertex shader
    Shader shader = LoadShader(0, "resources/shaders/glsl330/julia_shader.fs");
    
    // c constant to use in z^2 + c
    float c[2] = { POINTS_OF_INTEREST[0][0], POINTS_OF_INTEREST[0][1] };
    
    // Offset and zoom to draw the julia set at. (centered on screen and 1.6 times smaller)
    float offset[2] = { -(float)screenWidth/2, -(float)screenHeight/2 }; 
    float zoom = 1.6f;
    
    // Get variable (uniform) locations on the shader to connect with the program
    // NOTE: If uniform variable could not be found in the shader, function returns -1
    int cLoc = GetShaderLocation(shader, "c");
    int zoomLoc = GetShaderLocation(shader, "zoom");
    int offsetLoc = GetShaderLocation(shader, "offset");

    // Tell the shader what the screen dimensions, zoom, offset and c are
    float screenDims[2] = { (float)screenWidth, (float)screenHeight };
    SetShaderValue(shader, GetShaderLocation(shader, "screenDims"), screenDims, UNIFORM_VEC2);
    
    SetShaderValue(shader, cLoc, c, UNIFORM_VEC2);
    SetShaderValue(shader, zoomLoc, &zoom, UNIFORM_FLOAT);
    SetShaderValue(shader, offsetLoc, offset, UNIFORM_VEC2);

    // Create a RenderTexture2D to be used for render to texture
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    
    int incrementSpeed = 3;     // Multiplier of speed to change c value
    bool showControls = true;   // Show controls
    bool pause = false;         // Pause animation

    SetTargetFPS(60);                       // Set the window to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
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
            if (IsKeyPressed(KEY_ONE)) c[0] = POINTS_OF_INTEREST[0][0], c[1] = POINTS_OF_INTEREST[0][1];
            else if (IsKeyPressed(KEY_TWO)) c[0] = POINTS_OF_INTEREST[1][0], c[1] = POINTS_OF_INTEREST[1][1];
            else if (IsKeyPressed(KEY_THREE)) c[0] = POINTS_OF_INTEREST[2][0], c[1] = POINTS_OF_INTEREST[2][1];
            else if (IsKeyPressed(KEY_FOUR)) c[0] = POINTS_OF_INTEREST[3][0], c[1] = POINTS_OF_INTEREST[3][1];
            else if (IsKeyPressed(KEY_FIVE)) c[0] = POINTS_OF_INTEREST[4][0], c[1] = POINTS_OF_INTEREST[4][1];
            else if (IsKeyPressed(KEY_SIX)) c[0] = POINTS_OF_INTEREST[5][0], c[1] = POINTS_OF_INTEREST[5][1];

            SetShaderValue(shader, cLoc, c, UNIFORM_VEC2);
        }

        if (IsKeyPressed(KEY_P)) pause = !pause;                 // Pause animation (c change)
        if (IsKeyPressed(KEY_F1)) showControls = !showControls;  // Toggle whether or not to show controls
        
        if (!pause)
        {
            if (IsKeyDown(KEY_RIGHT)) incrementSpeed++;
            else if (IsKeyDown(KEY_LEFT)) incrementSpeed--;

            // Use mouse wheel to change zoom
            zoom -= (float)GetMouseWheelMove()/10.f;
            SetShaderValue(shader, zoomLoc, &zoom, UNIFORM_FLOAT);
            
            // Use mouse button to change offset
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                // TODO: Logic is not correct, the idea is getting zoom focus to pointed area
                Vector2 mousePos = GetMousePosition();

                offset[0] = mousePos.x -(float)screenWidth;
                offset[1] = mousePos.y -(float)screenHeight;
                
                SetShaderValue(shader, offsetLoc, offset, UNIFORM_VEC2);
            }

            // Increment c value with time
            float amount = GetFrameTime()*incrementSpeed*0.0005f;
            c[0] += amount;
            c[1] += amount;

            SetShaderValue(shader, cLoc, c, UNIFORM_VEC2);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);         // Clear the screen of the previous frame.
            
            // Using a render texture to draw Julia set
            BeginTextureMode(target);       // Enable drawing to texture
                ClearBackground(BLACK);     // Clear the render texture

                // Draw a rectangle in shader mode
                // NOTE: This acts as a canvas for the shader to draw on
                BeginShaderMode(shader);
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
                EndShaderMode();
            EndTextureMode();

            // Draw the saved texture (rendered julia set)
            DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, -target.texture.height }, Vector2Zero(), WHITE);
            
            // Draw information
            //DrawText( FormatText("cx: %f\ncy: %f\nspeed: %d", c[0], c[1], incrementSpeed), 10, 10, 10, RAYWHITE);

            if (showControls)
            {
                DrawText("Press keys [1 - 6] to change point of interest", 10, GetScreenHeight() - 60, 10, RAYWHITE);
                DrawText("Press KEY_LEFT | KEY_RIGHT to change speed", 10, GetScreenHeight() - 45, 10, RAYWHITE);
                DrawText("Press KEY_P to pause movement animation", 10, GetScreenHeight() - 30, 10, RAYWHITE);
                DrawText("Press KEY_F1 to toggle these controls", 10, GetScreenHeight() - 15, 10, RAYWHITE);
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
