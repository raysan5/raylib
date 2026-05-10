/*******************************************************************************************
*
*   raylib [models] example - loading iqm
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 2.5, last time updated with raylib 3.5
*
*   Example contributed by Culacant (@culacant) and reviewed by Ramon Santamaria (@raysan5)
*
*   NOTES: To export an IQM model from blender, make sure it is not posed, the vertices need
*   to be in the same position as they would be in edit mode and the scale of the models is
*   set to 0; scaling can be set from the export menu
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2019-2025 Culacant (@culacant) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - loading iqm");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 4.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    Model model = LoadModel("resources/models/iqm/guy.iqm");                    // Load the animated model mesh and basic data
    Texture2D texture = LoadTexture("resources/models/iqm/guytex.png");         // Load model texture and set material
    SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, texture);     // Set model material map texture
    Vector3 position = { 0.0f, 0.0f, 0.0f }; // Set model position

    // Load animation data
    int animCount = 0;
    ModelAnimation *anims = LoadModelAnimations("resources/models/iqm/guyanim.iqm", &animCount);

    // Animation playing variables
    unsigned int animIndex = 0;         // Current animation playing
    float animCurrentFrame = 0.0f;      // Current animation frame (supporting interpolated frames)

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);

        // Play animation when spacebar is held down
        animCurrentFrame += 1.0f;
        UpdateModelAnimation(model, anims[0], animCurrentFrame);
        if (animCurrentFrame >= anims[0].keyframeCount) animCurrentFrame = 0;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModelEx(model, position, (Vector3){ 1.0f, 0.0f, 0.0f }, -90.0f, (Vector3){ 1.0f, 1.0f, 1.0f }, WHITE);

                DrawGrid(10, 1.0f);

            EndMode3D();

            DrawText(TextFormat("Current animation: %s", anims[animIndex].name), 10, 10, 20, MAROON);
            DrawText("(c) Guy IQM 3D model by @culacant", screenWidth - 200, screenHeight - 20, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);                    // Unload texture
    UnloadModelAnimations(anims, animCount);   // Unload model animations data
    UnloadModel(model);                        // Unload model

    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
