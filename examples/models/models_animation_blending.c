/*******************************************************************************************
*
*   raylib [models] example - animation blending
*
*   Example complexity rating: [☆☆☆☆] 0/4
* 
*   Example originally created with raylib 5.5, last time updated with raylib 5.6-dev
*
*   Example contributed by Kirandeep (@Kirandeep-Singh-Khehra)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024 Kirandeep (@Kirandeep-Singh-Khehra)
* 
*   Note: Due to limitations in the Apple OpenGL driver, this feature does not work on MacOS
*   Note: This example uses CPU for updating meshes.
*           For GPU skinning see comments with 'INFO:'.
*
********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"             // Required for: UI controls

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

    InitWindow(screenWidth, screenHeight, "raylib [models] example - animation blending");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 6.0f, 6.0f, 6.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };  // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type

    // Load model
    Model model = LoadModel("resources/models/gltf/robot.glb"); // Load character model
    Vector3 position = { 0.0f, 0.0f, 0.0f }; // Set model world position

    // Load skinning shader
    // WARNING: It requires SUPPORT_GPU_SKINNING enabled on raylib (disabled by default)
    Shader skinningShader = LoadShader(TextFormat("resources/shaders/glsl%i/skinning.vs", GLSL_VERSION),
                                       TextFormat("resources/shaders/glsl%i/skinning.fs", GLSL_VERSION));
    
    // Assign skinning shader to all materials shaders
    //for (int i = 0; i < model.materialCount; i++) model.materials[i].shader = skinningShader;

    // Load model animations
    int animCount = 0;
    ModelAnimation *anims = LoadModelAnimations("resources/models/gltf/robot.glb", &animCount);

    // Animation playing variables
    // NOTE: Two animations are played with a smooth transition between them
    int currentAnimPlaying = 0;         // Current animation playing (0 o 1)
    int nextAnimToPlay = 1;             // Next animation to play (to transition)
    bool animTransition = false;        // Flag to register anim transition state

    int animIndex0 = 10;                // Current animation playing (walking)
    float animCurrentFrame0 = 0.0f;     // Current animation frame (supporting interpolated frames)
    float animFrameSpeed0 = 0.5f;       // Current animation play speed
    int animIndex1 = 6;                 // Next animation to play (running)
    float animCurrentFrame1 = 0.0f;     // Next animation frame (supporting interpolated frames)
    float animFrameSpeed1 = 0.5f;       // Next animation play speed

    float animBlendFactor = 0.0f;       // Blend factor from anim0[frame0] --> anim1[frame1], [0.0f..1.0f]
                                        // NOTE: 0.0f results in full anim0[] and 1.0f in full anim1[]

    float animBlendTime = 2.0f;         // Time to blend from one playing animation to another (in seconds)
    float animBlendTimeCounter = 0.0f;  // Time counter (delta time)

    bool animPause = false;             // Pause animation

    // UI required variables
    char *animNames[64] = { 0 };        // Pointers to animation names for dropdown box
    for (int i = 0; i < animCount; i++) animNames[i] = anims[i].name;

    bool dropdownEditMode0 = false;
    bool dropdownEditMode1 = false;
    float animFrameProgress0 = 0.0f;
    float animFrameProgress1 = 0.0f;
    float animBlendProgress = 0.0f;

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);
        
        if (IsKeyPressed(KEY_P)) animPause = !animPause;

        if (!animPause)
        {
            // Start transition from anim0[] to anim1[]
            if (IsKeyPressed(KEY_SPACE) && !animTransition)
            {
                if (currentAnimPlaying == 0)
                {
                    // Transition anim0 --> anim1
                    nextAnimToPlay = 1;
                    animCurrentFrame1 = 0.0f;
                }
                else
                {
                    // Transition anim1 --> anim0
                    nextAnimToPlay = 0;
                    animCurrentFrame0 = 0.0f;
                }

                // Set animation transition
                animTransition = true; 
                animBlendTimeCounter = 0.0f;
                animBlendFactor = 0.0f;
            }

            if (animTransition)
            {
                // Playing anim0 and anim1 at the same time
                animCurrentFrame0 += animFrameSpeed0;
                if (animCurrentFrame0 >= anims[animIndex0].keyframeCount) animCurrentFrame0 = 0.0f;
                animCurrentFrame1 += animFrameSpeed1;
                if (animCurrentFrame1 >= anims[animIndex1].keyframeCount) animCurrentFrame1 = 0.0f;

                // Increment blend factor over time to transition from anim0 --> anim1 over time
                // NOTE: Time blending could be other than linear, using some easing
                animBlendFactor = animBlendTimeCounter/animBlendTime;
                animBlendTimeCounter += GetFrameTime();
                animBlendProgress = animBlendFactor;

                // Update model with animations blending
                if (nextAnimToPlay == 1)
                {
                    // Blend anim0 --> anim1
                    UpdateModelAnimationEx(model, anims[animIndex0], animCurrentFrame0,
                        anims[animIndex1], animCurrentFrame1, animBlendFactor);
                }
                else
                {
                    // Blend anim1 --> anim0
                    UpdateModelAnimationEx(model, anims[animIndex1], animCurrentFrame1,
                        anims[animIndex0], animCurrentFrame0, animBlendFactor);
                }

                // Check if transition completed
                if (animBlendFactor > 1.0f)
                {
                    // Reset frame states
                    if (currentAnimPlaying == 0) animCurrentFrame0 = 0.0f;
                    else if (currentAnimPlaying == 1) animCurrentFrame1 = 0.0f;
                    currentAnimPlaying = nextAnimToPlay; // Update current animation playing

                    animBlendFactor = 0.0f; // Reset blend factor
                    animTransition = false; // Exit transition mode
                    animBlendTimeCounter = 0.0f;
                }
            }
            else
            {
                // Play only one anim, the current one
                if (currentAnimPlaying == 0)
                {
                    // Playing anim0 at defined speed
                    animCurrentFrame0 += animFrameSpeed0;
                    if (animCurrentFrame0 >= anims[animIndex0].keyframeCount) animCurrentFrame0 = 0.0f;
                    UpdateModelAnimation(model, anims[animIndex0], animCurrentFrame0);
                    //UpdateModelAnimationEx(model, anims[animIndex0], animCurrentFrame0, 
                    //    anims[animIndex1], animCurrentFrame1, 0.0f); // Same as above, first animation frame blend
                }
                else if (currentAnimPlaying == 1)
                {
                    // Playing anim1 at defined speed
                    animCurrentFrame1 += animFrameSpeed1;
                    if (animCurrentFrame1 >= anims[animIndex1].keyframeCount) animCurrentFrame1 = 0.0f;
                    UpdateModelAnimation(model, anims[animIndex1], animCurrentFrame1);
                    //UpdateModelAnimationEx(model, anims[animIndex0], animCurrentFrame0, 
                    //    anims[animIndex1], animCurrentFrame1, 1.0f); // Same as above, second animation frame blend
                }
            }
        }

        // Update progress bars values with current frame for each animation
        animFrameProgress0 = animCurrentFrame0;
        animFrameProgress1 = animCurrentFrame1;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModel(model, position, 1.0f, WHITE); // Draw animated model

                DrawGrid(10, 1.0f);
                
            EndMode3D();

            if (animTransition) DrawText("ANIM TRANSITION BLENDING!", 170, 50, 30, BLUE);

            // Draw UI elements
            //---------------------------------------------------------------------------------------------
            if (dropdownEditMode0) GuiDisable();
            GuiSlider((Rectangle){ 10, 38, 160, 12 }, 
                NULL, TextFormat("x%.1f", animFrameSpeed0), &animFrameSpeed0, 0.1f, 2.0f);
            GuiEnable();
            if (dropdownEditMode1) GuiDisable();
            GuiSlider((Rectangle){ GetScreenWidth() - 170, 38, 160, 12 }, 
                TextFormat("%.1fx", animFrameSpeed1), NULL, &animFrameSpeed1, 0.1f, 2.0f);
            GuiEnable();

            // Draw animation selectors for blending transition
            // NOTE: Transition does not start until requested 
            GuiSetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_SPACING, 1);
            if (GuiDropdownBox((Rectangle){ 10, 10, 160, 24 }, TextJoin(animNames, animCount, ";"), 
                &animIndex0, dropdownEditMode0)) dropdownEditMode0 = !dropdownEditMode0;

            // Blending process progress bar
            if (nextAnimToPlay == 1) GuiSetStyle(PROGRESSBAR, PROGRESS_SIDE, 0); // Left-->Right
            else GuiSetStyle(PROGRESSBAR, PROGRESS_SIDE, 1); // Right-->Left
            GuiProgressBar((Rectangle){ 180, 14, 440, 16 }, NULL, NULL, &animBlendProgress, 0.0f, 1.0f);
            GuiSetStyle(PROGRESSBAR, PROGRESS_SIDE, 0); // Reset to Left-->Right

            if (GuiDropdownBox((Rectangle){ GetScreenWidth() - 170, 10, 160, 24 }, TextJoin(animNames, animCount, ";"),
                &animIndex1, dropdownEditMode1)) dropdownEditMode1 = !dropdownEditMode1;

            // Draw playing timeline with keyframes for anim0[]
            GuiProgressBar((Rectangle){ 60, GetScreenHeight() - 60, GetScreenWidth() - 180, 20 }, "ANIM 0",
                TextFormat("FRAME: %.2f / %i", animFrameProgress0, anims[animIndex0].keyframeCount),
                &animFrameProgress0, 0.0f, (float)anims[animIndex0].keyframeCount);
            for (int i = 0; i < anims[animIndex0].keyframeCount; i++)
                DrawRectangle(60 + ((float)(GetScreenWidth() - 180)/(float)anims[animIndex0].keyframeCount)*(float)i, 
                    GetScreenHeight() - 60, 1, 20, BLUE);

            // Draw playing timeline with keyframes for anim1[]
            GuiProgressBar((Rectangle){ 60, GetScreenHeight() - 30, GetScreenWidth() - 180, 20 }, "ANIM 1",
                TextFormat("FRAME: %.2f / %i", animFrameProgress1, anims[animIndex1].keyframeCount),
                &animFrameProgress1, 0.0f, (float)anims[animIndex1].keyframeCount);
            for (int i = 0; i < anims[animIndex1].keyframeCount; i++)
                DrawRectangle(60 + ((float)(GetScreenWidth() - 180)/(float)anims[animIndex1].keyframeCount)*(float)i, 
                    GetScreenHeight() - 30, 1, 20, BLUE);
            //---------------------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModelAnimations(anims, animCount); // Unload model animation
    UnloadModel(model);             // Unload model and meshes/material
    UnloadShader(skinningShader);   // Unload GPU skinning shader
    
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
