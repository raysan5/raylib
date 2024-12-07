/*******************************************************************************************
*
*   raylib [core] example - Model animation blending
* 
*   Example originally created with raylib 5.5
*
*   Example contributed by Kirandeep (@Kirandeep-Singh-Khehra)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024 Kirandeep (@Kirandeep-Singh-Khehra)
* 
*   Note: Due to limitations in the Apple OpenGL driver, this feature does not work on MacOS
*
********************************************************************************************/

#include "raylib.h"

#define clamp(x,a,b) ((x < a)? a : (x > b)? b : x)

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

    InitWindow(screenWidth, screenHeight, "raylib [models] example - Model Animation Blending");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 8.0f, 8.0f, 8.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };  // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type

    // Load gltf model
    Model characterModel = LoadModel("resources/models/gltf/robot.glb"); // Load character model
    
    // Load skinning shader
    Shader skinningShader = LoadShader(TextFormat("resources/shaders/glsl%i/skinning.vs", GLSL_VERSION),
                                       TextFormat("resources/shaders/glsl%i/skinning.fs", GLSL_VERSION));

    for (int i = 0; i < characterModel.materialCount; i++)
    {
        characterModel.materials[i].shader = skinningShader;
    }

    // Load gltf model animations
    int animsCount = 0;
    unsigned int animIndex0 = 0;
    unsigned int animIndex1 = 0;
    unsigned int animCurrentFrame = 0;
    ModelAnimation *modelAnimations = LoadModelAnimations("resources/models/gltf/robot.glb", &animsCount);

    float blendFactor = 0.5f;

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        
        // Select current animation
        if (IsKeyPressed(KEY_T)) animIndex0 = (animIndex0 + 1)%animsCount;
        else if (IsKeyPressed(KEY_G)) animIndex0 = (animIndex0 + animsCount - 1)%animsCount;
        if (IsKeyPressed(KEY_Y)) animIndex1 = (animIndex1 + 1)%animsCount;
        else if (IsKeyPressed(KEY_H)) animIndex1 = (animIndex1 + animsCount - 1)%animsCount;
        
        // Select blend factor
        if (IsKeyPressed(KEY_U)) blendFactor = clamp(blendFactor - 0.1, 0.0f, 1.0f);
        else if (IsKeyPressed(KEY_J)) blendFactor = clamp(blendFactor + 0.1, 0.0f, 1.0f);

        // Update animation
        animCurrentFrame++;

        // Update bones
        // Note: Same animation frame index is used below. By default it loops both animations
        UpdateModelAnimationBonesWithBlending(characterModel, modelAnimations[animIndex0], animCurrentFrame, modelAnimations[animIndex1], animCurrentFrame, blendFactor);        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
            
                // Draw character mesh, pose calculation is done in shader (GPU skinning)
                for (int i = 0; i < characterModel.meshCount; i++)
                {
                    DrawMesh(characterModel.meshes[i], characterModel.materials[characterModel.meshMaterial[i]], characterModel.transform);
                }


                DrawGrid(10, 1.0f);
                
            EndMode3D();

            DrawText("Use the U/J Arrow to adjust blend factor", 10, 10, 20, GRAY);
            DrawText("Use the T/G to switch animation", 10, 30, 20, GRAY);
            DrawText("Use the Y/H to switch animation", 10, 50, 20, GRAY);
            DrawText(TextFormat("Animations: %s, %s", modelAnimations[animIndex0].name, modelAnimations[animIndex1].name), 10, 70, 20, BLACK);
            DrawText(TextFormat("Blend Factor: %f", blendFactor), 10, 86, 20, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModelAnimations(modelAnimations, animsCount); // Unload model animation
    UnloadModel(characterModel);    // Unload model and meshes/material
    UnloadShader(skinningShader);   // Unload GPU skinning shader
    
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
