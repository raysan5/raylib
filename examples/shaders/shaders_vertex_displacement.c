/*******************************************************************************************
*
*   raylib [shaders] example - vertex displacement
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.0, last time updated with raylib 4.5
*
*   Example contributed by Alex ZH (@ZzzhHe) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023-2025 Alex ZH (@ZzzhHe)
*
********************************************************************************************/

#include "raylib.h"

#include "rlgl.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

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

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - vertex displacement");

    // set up camera
    Camera camera = {0};
    camera.position = (Vector3) {20.0f, 5.0f, -20.0f};
    camera.target = (Vector3) {0.0f, 0.0f, 0.0f};
    camera.up = (Vector3) {0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Load vertex and fragment shaders
    Shader shader = LoadShader(
        TextFormat("resources/shaders/glsl%i/vertex_displacement.vs", GLSL_VERSION),
        TextFormat("resources/shaders/glsl%i/vertex_displacement.fs", GLSL_VERSION));

    // Load perlin noise texture
    Image perlinNoiseImage = GenImagePerlinNoise(512, 512, 0, 0, 1.0f);
    Texture perlinNoiseMap = LoadTextureFromImage(perlinNoiseImage);
    UnloadImage(perlinNoiseImage);

    // Set shader uniform location
    int perlinNoiseMapLoc = GetShaderLocation(shader, "perlinNoiseMap");
    rlEnableShader(shader.id);
    rlActiveTextureSlot(1);
    rlEnableTexture(perlinNoiseMap.id);
    rlSetUniformSampler(perlinNoiseMapLoc, 1);

    // Create a plane mesh and model
    Mesh planeMesh = GenMeshPlane(50, 50, 50, 50);
    Model planeModel = LoadModelFromMesh(planeMesh);
    // Set plane model material
    planeModel.materials[0].shader = shader;

    float time = 0.0f;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FREE); // Update camera

        time += GetFrameTime(); // Update time variable
        SetShaderValue(shader, GetShaderLocation(shader, "time"), &time, SHADER_UNIFORM_FLOAT); // Send time value to shader

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                BeginShaderMode(shader);
                    // Draw plane model
                    DrawModel(planeModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, (Color) {255, 255, 255, 255});
                EndShaderMode();

            EndMode3D();

            DrawText("Vertex displacement", 10, 10, 20, DARKGRAY);
            DrawFPS(10, 40);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);
    UnloadModel(planeModel);
    UnloadTexture(perlinNoiseMap);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
