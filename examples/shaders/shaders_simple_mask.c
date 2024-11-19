/*******************************************************************************************
*
*   raylib [shaders] example - Simple shader mask
*
*   Example originally created with raylib 2.5, last time updated with raylib 3.7
*
*   Example contributed by Chris Camacho (@chriscamacho) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2019-2024 Chris Camacho (@chriscamacho) and Ramon Santamaria (@raysan5)
*
********************************************************************************************
*
*   After a model is loaded it has a default material, this material can be
*   modified in place rather than creating one from scratch...
*   While all of the maps have particular names, they can be used for any purpose
*   except for three maps that are applied as cubic maps (see below)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

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

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - simple shader mask");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 1.0f, 2.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    // Define our three models to show the shader on
    Mesh torus = GenMeshTorus(0.3f, 1, 16, 32);
    Model model1 = LoadModelFromMesh(torus);

    Mesh cube = GenMeshCube(0.8f,0.8f,0.8f);
    Model model2 = LoadModelFromMesh(cube);

    // Generate model to be shaded just to see the gaps in the other two
    Mesh sphere = GenMeshSphere(1, 16, 16);
    Model model3 = LoadModelFromMesh(sphere);

    // Load the shader
    Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/mask.fs", GLSL_VERSION));

    // Load and apply the diffuse texture (colour map)
    Texture texDiffuse = LoadTexture("resources/plasma.png");
    model1.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texDiffuse;
    model2.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texDiffuse;

    // Using MATERIAL_MAP_EMISSION as a spare slot to use for 2nd texture
    // NOTE: Don't use MATERIAL_MAP_IRRADIANCE, MATERIAL_MAP_PREFILTER or  MATERIAL_MAP_CUBEMAP as they are bound as cube maps
    Texture texMask = LoadTexture("resources/mask.png");
    model1.materials[0].maps[MATERIAL_MAP_EMISSION].texture = texMask;
    model2.materials[0].maps[MATERIAL_MAP_EMISSION].texture = texMask;
    shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "mask");

    // Frame is incremented each frame to animate the shader
    int shaderFrame = GetShaderLocation(shader, "frame");

    // Apply the shader to the two models
    model1.materials[0].shader = shader;
    model2.materials[0].shader = shader;

    int framesCounter = 0;
    Vector3 rotation = { 0 };           // Model rotation angles

    DisableCursor();                    // Limit cursor to relative movement inside the window
    SetTargetFPS(60);                   // Set  to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        
        framesCounter++;
        rotation.x += 0.01f;
        rotation.y += 0.005f;
        rotation.z -= 0.0025f;

        // Send frames counter to shader for animation
        SetShaderValue(shader, shaderFrame, &framesCounter, SHADER_UNIFORM_INT);

        // Rotate one of the models
        model1.transform = MatrixRotateXYZ(rotation);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKBLUE);

            BeginMode3D(camera);

                DrawModel(model1, (Vector3){ 0.5f, 0.0f, 0.0f }, 1, WHITE);
                DrawModelEx(model2, (Vector3){ -0.5f, 0.0f, 0.0f }, (Vector3){ 1.0f, 1.0f, 0.0f }, 50, (Vector3){ 1.0f, 1.0f, 1.0f }, WHITE);
                DrawModel(model3,(Vector3){ 0.0f, 0.0f, -1.5f }, 1, WHITE);
                DrawGrid(10, 1.0f);        // Draw a grid

            EndMode3D();

            DrawRectangle(16, 698, MeasureText(TextFormat("Frame: %i", framesCounter), 20) + 8, 42, BLUE);
            DrawText(TextFormat("Frame: %i", framesCounter), 20, 700, 20, WHITE);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model1);
    UnloadModel(model2);
    UnloadModel(model3);

    UnloadTexture(texDiffuse);  // Unload default diffuse texture
    UnloadTexture(texMask);     // Unload texture mask

    UnloadShader(shader);       // Unload shader

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
