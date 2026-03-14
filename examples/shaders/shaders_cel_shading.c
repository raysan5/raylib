/*******************************************************************************************
*
*   raylib [shaders] example - cel shading
*
*   Example complexity rating: [★★★☆] 3/4
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3)
*
*   Example contributed by Gleb A (@ggrizzly) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Gleb A (@ggrizzly)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <math.h>       // Required for: sinf(), cosf()

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

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - cel shading");

    Camera camera = { 0 };
    camera.position = (Vector3){ 9.0f, 6.0f, 9.0f };
    camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    // Load model
    Model model = LoadModel("resources/models/old_car_new.glb");

    // Load cel shader
    Shader celShader = LoadShader(
        TextFormat("resources/shaders/glsl%i/cel.vs", GLSL_VERSION),
        TextFormat("resources/shaders/glsl%i/cel.fs", GLSL_VERSION));
    celShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(celShader, "viewPos");
    
    // Apply cel shader to model, keep copy of default shader
    Shader defaultShader = model.materials[0].shader;
    model.materials[0].shader = celShader;

    // numBands: controls toon quantization steps (2 = hard binary, 20 = near-smooth)
    float numBands = 10.0f;
    int numBandsLoc = GetShaderLocation(celShader, "numBands");
    SetShaderValue(celShader, numBandsLoc, &numBands, SHADER_UNIFORM_FLOAT);

    // Inverted-hull outline shader: draws back faces extruded along normals
    Shader outlineShader = LoadShader(
        TextFormat("resources/shaders/glsl%i/outline_hull.vs", GLSL_VERSION),
        TextFormat("resources/shaders/glsl%i/outline_hull.fs", GLSL_VERSION));
    int outlineThicknessLoc = GetShaderLocation(outlineShader, "outlineThickness");

    // Single directional white light, angled so toon bands are visible on the model sides.
    // Spins opposite to CAMERA_ORBITAL (0.5 rad/s) so lighting changes as you watch.
    Light lights[MAX_LIGHTS] = { 0 };
    lights[0] = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 50.0f, 50.0f, 50.0f }, Vector3Zero(), WHITE, celShader);

    bool celEnabled = true;
    bool outlineEnabled = true;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);

        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(celShader, celShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        // [Z] Toggle cel shading on/off
        if (IsKeyPressed(KEY_Z))
        {
            celEnabled = !celEnabled;
            if (celEnabled) model.materials[0].shader = celShader; // Apply cel shader to model
            else model.materials[0].shader = defaultShader; // Apply default shader to model
        }

        // [C] Toggle outline on/off
        if (IsKeyPressed(KEY_C)) outlineEnabled = !outlineEnabled;

        // [Q/E] Decrease/increase toon band count (press or hold to repeat)
        if (IsKeyPressed(KEY_E) || IsKeyPressedRepeat(KEY_E)) numBands = Clamp(numBands + 1.0f, 2.0f, 20.0f);
        if (IsKeyPressed(KEY_Q) || IsKeyPressedRepeat(KEY_Q)) numBands = Clamp(numBands - 1.0f, 2.0f, 20.0f);
        SetShaderValue(celShader, numBandsLoc, &numBands, SHADER_UNIFORM_FLOAT);

        // Spin light opposite to CAMERA_ORBITAL (0.5 rad/s), angled 45 degrees off vertical
        float t = (float)GetTime();
        lights[0].position = (Vector3){ sinf(-t*0.3f)*5.0f, 5.0f, cosf(-t*0.3f)*5.0f };

        for (int i = 0; i < MAX_LIGHTS; i++) UpdateLightValues(celShader, lights[i]);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                if (outlineEnabled)
                {
                    // Outline pass: cull front faces, draw extruded back faces as silhouette
                    float thickness = 0.005f;
                    SetShaderValue(outlineShader, outlineThicknessLoc, &thickness, SHADER_UNIFORM_FLOAT);
                    
                    rlSetCullFace(RL_CULL_FACE_FRONT);
                    
                    model.materials[0].shader = outlineShader;
                    
                    DrawModel(model, Vector3Zero(), 0.75f, WHITE);
                    
                    if (celEnabled) model.materials[0].shader = celShader; // Apply cel shader to model
                    else model.materials[0].shader = defaultShader; // Apply default shader to model
                    
                    rlSetCullFace(RL_CULL_FACE_BACK);
                }

                DrawModel(model, Vector3Zero(), 0.75f, WHITE);
                DrawSphereEx(lights[0].position, 0.2f, 50, 50, YELLOW);  // Light position indicator
                DrawGrid(10, 10.0f);

            EndMode3D();

            DrawFPS(10, 10);
            DrawText(TextFormat("Cel: %s  [Z]", celEnabled? "ON" : "OFF"), 10, 65, 20, celEnabled? DARKGREEN : DARKGRAY);
            DrawText(TextFormat("Outline: %s  [C]", outlineEnabled? "ON" : "OFF"), 10, 90, 20, outlineEnabled? DARKGREEN : DARKGRAY);
            DrawText(TextFormat("Bands: %.0f  [Q/E]", numBands), 10, 115, 20, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);
    UnloadShader(celShader);
    UnloadShader(outlineShader);
    
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
