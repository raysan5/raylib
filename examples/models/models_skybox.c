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
    Camera camera = {{ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };

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
    
    // Load HDR panorama (sphere) texture
    Texture2D texHDR = LoadTexture("resources/dresden_square.hdr");
    
    // Generate cubemap (texture with 6 quads-cube-mapping) from panorama HDR texture
    // NOTE: New texture is generated rendering to texture, shader computes the sphre->cube coordinates mapping
    skybox.material.maps[MAP_CUBEMAP].texture = GenTextureCubemap(shdrCubemap, texHDR, 512);
    
    UnloadTexture(texHDR);      // Texture not required anymore, cubemap already generated
    UnloadShader(shdrCubemap);  // Unload cubemap generation shader, not required anymore
    
    SetCameraMode(camera, CAMERA_FIRST_PERSON);  // Set a first person camera mode

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

            BeginMode3D(camera);

                DrawModel(skybox, (Vector3){0, 0, 0}, 1.0f, WHITE);
                
                DrawGrid(10, 1.0f);

            EndMode3D();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(skybox);        // Unload skybox model (and textures)

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
