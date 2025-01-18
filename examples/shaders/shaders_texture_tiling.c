/*******************************************************************************************
*
*   raylib [shaders] example - texture tiling
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example demonstrates how to tile a texture on a 3D model using raylib.
*
*   Example originally created with raylib 4.5, last time updated with raylib 4.5
*
*   Example contributed by Luis Almeida (@luis605) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023-2025 Luis Almeida (@luis605)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - texture tiling");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 4.0f, 4.0f, 4.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    // Load a cube model
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model model = LoadModelFromMesh(cube);
    
    // Load a texture and assign to cube model
    Texture2D texture = LoadTexture("resources/cubicmap_atlas.png");
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // Set the texture tiling using a shader
    float tiling[2] = { 3.0f, 3.0f };
    Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/tiling.fs", GLSL_VERSION));
    SetShaderValue(shader, GetShaderLocation(shader, "tiling"), tiling, SHADER_UNIFORM_VEC2);
    model.materials[0].shader = shader;

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FREE);

        if (IsKeyPressed('Z')) camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
            
                BeginShaderMode(shader);
                    DrawModel(model, (Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, WHITE);
                EndShaderMode();

                DrawGrid(10, 1.0f);
                
            EndMode3D();

            DrawText("Use mouse to rotate the camera", 10, 10, 20, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);         // Unload model
    UnloadShader(shader);       // Unload shader
    UnloadTexture(texture);     // Unload texture

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}
