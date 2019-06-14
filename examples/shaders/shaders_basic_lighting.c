/*******************************************************************************************
*
*   raylib [shaders] example - basic lighting
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3).
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Chris Camacho (@codifies) and reviewed by Ramon Santamaria (@raysan5)
*
*   Chris Camacho (@codifies -  http://bedroomcoders.co.uk/) notes:
*
*   This is based on the PBR lighting example, but greatly simplified to aid learning...
*   actually there is very little of the PBR example left!
*   When I first looked at the bewildering complexity of the PBR example I feared
*   I would never understand how I could do simple lighting with raylib however its
*   a testement to the authors of raylib (including rlights.h) that the example
*   came together fairly quickly.
*
*   Copyright (c) 2019 Chris Camacho (@codifies) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - basic lighting");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 2.0f, 2.0f, 6.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.type = CAMERA_PERSPECTIVE;                   // Camera mode type

    // Load models
    Model modelA = LoadModelFromMesh(GenMeshTorus(0.4f, 1.0f, 16, 32));
    Model modelB = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    Model modelC = LoadModelFromMesh(GenMeshSphere(0.5f, 32, 32));

    // Load models texture
    Texture texture = LoadTexture("resources/texel_checker.png");

    // Assign texture to default model material
    modelA.materials[0].maps[MAP_DIFFUSE].texture = texture;
    modelB.materials[0].maps[MAP_DIFFUSE].texture = texture;
    modelC.materials[0].maps[MAP_DIFFUSE].texture = texture;

    Shader shader = LoadShader("resources/shaders/glsl330/basic_lighting.vs", 
                               "resources/shaders/glsl330/basic_lighting.fs");
    
    // Get some shader loactions
    shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    // ambient light level
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){ 0.2f, 0.2f, 0.2f, 1.0f }, UNIFORM_VEC4);
    
    float angle = 6.282f;

    // All models use the same shader
    modelA.materials[0].shader = shader;
    modelB.materials[0].shader = shader;
    modelC.materials[0].shader = shader;

    // Using 4 point lights, white, red, green and blue
    Light lights[MAX_LIGHTS] = { 0 };
    lights[0] = CreateLight(LIGHT_POINT, (Vector3){ 4, 2, 4 }, Vector3Zero(), WHITE, shader);
    lights[1] = CreateLight(LIGHT_POINT, (Vector3){ 4, 2, 4 }, Vector3Zero(), RED, shader);
    lights[2] = CreateLight(LIGHT_POINT, (Vector3){ 0, 4, 2 }, Vector3Zero(), GREEN, shader);
    lights[3] = CreateLight(LIGHT_POINT, (Vector3){ 0, 4, 2 }, Vector3Zero(), BLUE, shader);

    SetCameraMode(camera, CAMERA_ORBITAL);  // Set an orbital camera mode

    SetTargetFPS(60);                       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_W)) { lights[0].enabled = !lights[0].enabled; }
        if (IsKeyPressed(KEY_R)) { lights[1].enabled = !lights[1].enabled; }
        if (IsKeyPressed(KEY_G)) { lights[2].enabled = !lights[2].enabled; }
        if (IsKeyPressed(KEY_B)) { lights[3].enabled = !lights[3].enabled; }

        UpdateCamera(&camera);              // Update camera

        // Make the lights do differing orbits
        angle -= 0.02;
        lights[0].position.x = cosf(angle)*4.0f;
        lights[0].position.z = sinf(angle)*4.0f;
        lights[1].position.x = cosf(-angle*0.6f)*4.0f;
        lights[1].position.z = sinf(-angle*0.6f)*4.0f;
        lights[2].position.y = cosf(angle*0.2f)*4.0f;
        lights[2].position.z = sinf(angle*0.2f)*4.0f;
        lights[3].position.y = cosf(-angle*0.35f)*4.0f;
        lights[3].position.z = sinf(-angle*0.35f)*4.0f;
        
        UpdateLightValues(shader, lights[0]);
        UpdateLightValues(shader, lights[1]);
        UpdateLightValues(shader, lights[2]);
        UpdateLightValues(shader, lights[3]);

        // Rotate the torus
        modelA.transform = MatrixMultiply(modelA.transform, MatrixRotateX(-0.025));
        modelA.transform = MatrixMultiply(modelA.transform, MatrixRotateZ(0.012));

        // Update the light shader with the camera view position
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                // Draw the three models
                DrawModel(modelA, Vector3Zero(), 1.0f, WHITE);
                DrawModel(modelB, (Vector3){-1.6,0,0}, 1.0f, WHITE);
                DrawModel(modelC, (Vector3){ 1.6,0,0}, 1.0f, WHITE);

                // Draw markers to show where the lights are
                if (lights[0].enabled) { DrawSphereEx(lights[0].position, 0.2f, 8, 8, WHITE); }
                if (lights[1].enabled) { DrawSphereEx(lights[1].position, 0.2f, 8, 8, RED); }
                if (lights[2].enabled) { DrawSphereEx(lights[2].position, 0.2f, 8, 8, GREEN); }
                if (lights[3].enabled) { DrawSphereEx(lights[3].position, 0.2f, 8, 8, BLUE); }

                DrawGrid(10, 1.0f);

            EndMode3D();

            DrawFPS(10, 10);
            
            DrawText("Keys RGB & W toggle lights", 10, 30, 20, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(modelA);        // Unload the modelA
    UnloadModel(modelB);        // Unload the modelB
    UnloadModel(modelC);        // Unload the modelC
    
    UnloadTexture(texture);     // Unload the texture
    UnloadShader(shader);       // Unload shader

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

