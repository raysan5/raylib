/*******************************************************************************************
 *
 *   raylib [shaders] example - volumetric fog
 *
 *   Example complexity rating: [★★★☆] 3/4
 *
 *   Example originally created with raylib 6.1
 *
 *   Example contributed by Emec Delam (@emecdelam) and reviewed by Ramon
 * Santamaria
 * (@raysan5)
 *
 *   Example licensed under an unmodified zlib/libpng license, which is an
 * OSI-certified, BSD-like license that allows static linking with closed source
 * software
 *
 *   Copyright (c) 2026 Emec Delam (@emecdelam)
 *
 ********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {

    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] - volumetric fog with raymarching");

    Camera camera = {0};
    camera.position = (Vector3){4.0f, 4.0f, 4.0f};
    camera.target = (Vector3){0.0f, 0.5f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // mountain model from https://sketchfab.com/3d-models/low-poly
    Model mountainModel = LoadModel("resources/models/mountains.glb");

    Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/volumetric_fog.vs", GLSL_VERSION),
                               TextFormat("resources/shaders/glsl%i/volumetric_fog.fs", GLSL_VERSION));

    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    Vector3 lightPos = (Vector3){2, 2, 2};
    SetShaderValue(shader, GetShaderLocation(shader, "lightPos"), &lightPos, SHADER_UNIFORM_VEC3);

    for (int i = 0; i < mountainModel.materialCount; i++) {
        mountainModel.materials[i].shader = shader;
    }

    int fogEnabled = 0;
    float fogDensity = 0.7;
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_ORBITAL);
        // Give camera position to the shader
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &camera.position, SHADER_UNIFORM_VEC3);

        // Enable or disable the fog
        if (IsKeyPressed(KEY_SPACE)) {
            fogEnabled = !fogEnabled;
            SetShaderValue(shader, GetShaderLocation(shader, "fogEnabled"), &fogEnabled, SHADER_UNIFORM_INT);
        }
        if (IsKeyPressed(KEY_UP)) {
            fogDensity = Clamp(fogDensity + 0.1, 0.0, 2.0);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            fogDensity = Clamp(fogDensity - 0.1, 0.0, 2.0);
        }
        // Floating point rounding
        fogDensity = roundf(fogDensity * 100.0f) / 100.0f;
        SetShaderValue(shader, GetShaderLocation(shader, "fogDensity"), &fogDensity, SHADER_UNIFORM_FLOAT);
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        DrawModel(mountainModel, Vector3Zero(), 1.0, RAYWHITE);

        DrawSphereEx(lightPos, 0.2f, 8, 8, WHITE);
        DrawGrid(10, 1.0f);

        EndMode3D();

        DrawText("Arrow [UP/DOWN] to change fog density", 10, 10, 20, DARKGRAY);
        DrawText("Press space to toggle fog", 10, 40, 20, DARKGRAY);
        DrawFPS(10, 70);

        EndDrawing();
    }

    UnloadShader(shader);
    UnloadModel(mountainModel);
    CloseWindow();

    return 0;
}
