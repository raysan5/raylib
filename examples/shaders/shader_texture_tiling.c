/*******************************************************************************************
*
*   raylib [textures] example - Texture Tiling
*
*   Example demonstrates how to tile a texture on a 3D model using raylib.
*
*   Example contributed by Luís Almeida (https://github.com/luis605)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023 Luís Almeida (https://github.com/luis605)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 600;

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Raylib Texture Tiling");

    SetTargetFPS(60);

    // Load a texture
    Texture2D texture = LoadTexture("resources/raylib_logo.png");

    // Create a cube mesh
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);

    // Load the texture onto the GPU
    Model model = LoadModelFromMesh(cube);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // Set the tiling of the texture
    float tiling[2] = {3.0f, 3.0f};
    Shader shader = LoadShader(0, "resources/shaders/glsl330/tiling.fs"); // Create a custom shader in a .glsl file
    SetShaderValue(shader, GetShaderLocation(shader, "tiling"), tiling, SHADER_UNIFORM_VEC2);
    model.materials[0].shader = shader;

    // Camera setup
    Camera camera = { 0 };
    camera.position = (Vector3){ 3.0f, 3.0f, 3.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------

        BeginDrawing();
        ClearBackground(RAYWHITE);
        UpdateCamera(&camera, CAMERA_FREE);

        // Draw the model
        {
            BeginMode3D(camera);
            BeginShaderMode(shader);

            DrawModel(model, (Vector3){ 0.0f, 0.0f, 0.0f }, 5.0f, WHITE);

            EndShaderMode();
            EndMode3D();
        }
        
        DrawText("Use mouse to rotate the camera", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    UnloadTexture(texture);     // Unload texture
    UnloadModel(model);         // Unload model
    UnloadShader(shader);       // Unload shader


    CloseWindow();              // Close window and OpenGL context

    return 0;
}
