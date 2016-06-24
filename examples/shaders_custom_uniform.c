/*******************************************************************************************
*
*   raylib [shaders] example - Apply a postprocessing shader and connect a custom uniform variable
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3), to test this example
*         on OpenGL ES 2.0 platforms (Android, Raspberry Pi, HTML5), use #version 100 shaders
*         raylib comes with shaders ready for both versions, check raylib/shaders install folder
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);      // Enable Multi Sampling Anti Aliasing 4x (if available)

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - custom uniform variable");

    // Define the camera to look into our 3d world
    Camera camera = {{ 3.0f, 3.0f, 3.0f }, { 0.0f, 1.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };

    Model dwarf = LoadModel("resources/model/dwarf.obj");                   // Load OBJ model
    Texture2D texture = LoadTexture("resources/model/dwarf_diffuse.png");   // Load model texture (diffuse map)
    dwarf.material.texDiffuse = texture;                                    // Set dwarf model diffuse texture

    Vector3 position = { 0.0f, 0.0f, 0.0f };                                // Set model position
    
    Shader shader = LoadShader("resources/shaders/glsl330/base.vs", 
                               "resources/shaders/glsl330/swirl.fs");       // Load postpro shader
    
    // Get variable (uniform) location on the shader to connect with the program
    // NOTE: If uniform variable could not be found in the shader, function returns -1
    int swirlCenterLoc = GetShaderLocation(shader, "center");
    
    float swirlCenter[2] = { (float)screenWidth/2, (float)screenHeight/2 };
    
    // Create a RenderTexture2D to be used for render to texture
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    
    // Setup orbital camera
    SetCameraMode(CAMERA_ORBITAL);          // Set an orbital camera mode
    SetCameraPosition(camera.position);     // Set internal camera position to match our camera position
    SetCameraTarget(camera.target);         // Set internal camera target to match our camera target

    SetTargetFPS(60);                       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector2 mousePosition = GetMousePosition();

        swirlCenter[0] = mousePosition.x;
        swirlCenter[1] = screenHeight - mousePosition.y;

        // Send new value to the shader to be used on drawing
        SetShaderValue(shader, swirlCenterLoc, swirlCenter, 2);
        
        UpdateCamera(&camera);              // Update internal camera and our camera
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            BeginTextureMode(target);   // Enable drawing to texture

                Begin3dMode(camera);

                    DrawModel(dwarf, position, 2.0f, WHITE);   // Draw 3d model with texture

                    DrawGrid(10, 1.0f);     // Draw a grid

                End3dMode();
                
                DrawText("TEXT DRAWN IN RENDER TEXTURE", 200, 10, 30, RED);
            
            EndTextureMode();           // End drawing to texture (now we have a texture available for next passes)
            
            BeginShaderMode(shader);
            
                // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
                DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, -target.texture.height }, (Vector2){ 0, 0 }, WHITE);
            
            EndShaderMode();
            
            DrawText("(c) Dwarf 3D model by David Moreno", screenWidth - 200, screenHeight - 20, 10, GRAY);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);           // Unload shader
    UnloadTexture(texture);         // Unload texture
    UnloadModel(dwarf);             // Unload model
    UnloadRenderTexture(target);    // Unload render texture

    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}