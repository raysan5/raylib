/*******************************************************************************************
*
*   raylib [models] example - Skybox loading and drawing
*
*   This example has been created using raylib 1.8 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2017 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - skybox loading and drawing");

    // Define the camera to look into our 3d world
    Camera camera = {{ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };

    // Load skybox model   
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model skybox = LoadModelFromMesh(cube);
    
    // Load skybox shader and set required locations
    // NOTE: Some locations are automatically set at shader loading
    skybox.material.shader = LoadShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    SetShaderValuei(skybox.material.shader, GetShaderLocation(skybox.material.shader, "environmentMap"), (int[1]){ MAP_CUBEMAP }, 1);

    // Load cubemap shader and setup required shader locations
    Shader shdrCubemap = LoadShader("resources/shaders/cubemap.vs", "resources/shaders/cubemap.fs");
    SetShaderValuei(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"), (int[1]){ 0 }, 1);
    
    Texture2D texHDR = LoadTexture("resources/pinetree.hdr");
    skybox.material.maps[MAP_CUBEMAP].texture = GenTextureCubemap(shdrCubemap, texHDR, 512);
    
    UnloadShader(shdrCubemap);  // Cubemap generation shader not required any more
    
    SetCameraMode(camera, CAMERA_ORBITAL);  // Set an orbital camera mode

    SetTargetFPS(60);                       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);              // Update camera
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawModel(skybox, Vector3Zero(), 1.0f, WHITE);
                
                DrawGrid(10, 1.0f);

            End3dMode();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(skybox);        // Unload skybox model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
