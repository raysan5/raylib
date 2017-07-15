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
#include "raymath.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - skybox loading and drawing");

    // Define the camera to look into our 3d world
    Camera camera = {{ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };

    // Load skybox model and shader
    #define PATH_SKYBOX_VS      "resources/shaders/skybox.vs"       // Path to skybox vertex shader
    #define PATH_SKYBOX_FS      "resources/shaders/skybox.fs"       // Path to skybox vertex shader
    #define PATH_TEXTURES_HDR   "resources/pinetree.hdr"
    #define LOC_CUSTOM_SKYRESOLUTION    15
    
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model skybox = LoadModelFromMesh(cube, false);
    skybox.material.shader = LoadShader(PATH_SKYBOX_VS, PATH_SKYBOX_FS);

    Texture2D texHDR = LoadTexture(PATH_TEXTURES_HDR);
    skybox.material.maps[TEXMAP_CUBEMAP].tex = rlGenMapCubemap(texHDR, 512);
    SetShaderValuei(skybox.material.shader, GetShaderLocation(skybox.material.shader, "environmentMap"), (int[1]){ TEXMAP_CUBEMAP }, 1);

    // Get skybox shader locations
    int skyProjectionLoc = GetShaderLocation(skybox.material.shader, "projection");
    skybox.material.shader.locs[LOC_MATRIX_VIEW] = GetShaderLocation(skybox.material.shader, "view");
    
    // Then before rendering, configure the viewport to the actual screen dimensions
    Matrix proj = MatrixPerspective(60.0, (double)GetScreenWidth()/(double)GetScreenHeight(), 0.01, 1000.0);
    MatrixTranspose(&proj);
    SetShaderValueMatrix(skybox.material.shader, skyProjectionLoc, proj);

    SetCameraMode(camera, CAMERA_ORBITAL);  // Set an orbital camera mode

    SetTargetFPS(60);                       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);              // Update camera
        
        Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);
        SetShaderValueMatrix(skybox.material.shader, skybox.material.shader.locs[LOC_MATRIX_VIEW], view);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawModel(skybox, VectorZero(), 1.0f, RED);
                
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
