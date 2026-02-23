/*******************************************************************************************
*
*   raylib [models] example - animation blend custom
*
*   Example complexity rating: [★★★★] 4/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   This example demonstrates per-bone animation blending, allowing smooth transitions
*   between two animations by interpolating bone transforms. This is useful for:
*    - Blending movement animations (walk/run) with action animations (jump/attack)
*    - Creating smooth animation transitions
*    - Layering animations (e.g., upper body attack while lower body walks)
*
*   Example contributed by dmitrii-brand (@dmitrii-brand) and reviewed by Ramon Santamaria (@raysan5)
*
*   NOTE: Due to limitations in the Apple OpenGL driver, this feature does not work on MacOS
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 dmitrii-brand (@dmitrii-brand)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

#include <string.h>  // Required for: memcpy()
#include <stdlib.h>  // Required for: NULL

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static bool IsUpperBodyBone(const char *boneName);
static void BlendModelAnimationsBones(Model *model, ModelAnimation *anim1, int frame1, 
    ModelAnimation *anim2, int frame2, float blendFactor, bool upperBodyBlend);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - animation blend custom");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 5.0f, 5.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };  // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type

    // Load gltf model
    Model characterModel = LoadModel("resources/models/gltf/greenman.glb");

    // Load skinning shader
    Shader skinningShader = LoadShader(TextFormat("resources/shaders/glsl%i/skinning.vs", GLSL_VERSION),
                                       TextFormat("resources/shaders/glsl%i/skinning.fs", GLSL_VERSION));

    characterModel.materials[1].shader = skinningShader;

    // Load gltf model animations
    int animsCount = 0;
    ModelAnimation *modelAnimations = LoadModelAnimations("resources/models/gltf/greenman.glb", &animsCount);

    // Log all available animations for debugging
    TraceLog(LOG_INFO, "Found %d animations:", animsCount);
    for (int i = 0; i < animsCount; i++)
    {
        TraceLog(LOG_INFO, "  Animation %d: %s (%d frames)", i, modelAnimations[i].name, modelAnimations[i].keyframeCount);
    }

    // Use specific indices: walk/move = 2, attack = 3
    unsigned int animIndex1 = 2; // Walk/Move animation (index 2)
    unsigned int animIndex2 = 3; // Attack animation (index 3)
    unsigned int animCurrentFrame1 = 0;
    unsigned int animCurrentFrame2 = 0;
    
    // Validate indices
    if (animIndex1 >= animsCount) animIndex1 = 0;
    if (animIndex2 >= animsCount) animIndex2 = (animsCount > 1) ? 1 : 0;
    
    TraceLog(LOG_INFO, "Using Walk (index %d): %s", animIndex1, modelAnimations[animIndex1].name);
    TraceLog(LOG_INFO, "Using Attack (index %d): %s", animIndex2, modelAnimations[animIndex2].name);

    Vector3 position = { 0.0f, 0.0f, 0.0f }; // Set model position
    bool upperBodyBlend = true; // Toggle: true = upper/lower body blending, false = uniform blending (50/50)

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_THIRD_PERSON);

        // Toggle upper/lower body blending mode (SPACE key)
        if (IsKeyPressed(KEY_SPACE)) upperBodyBlend = !upperBodyBlend;

        // Update animation frames
        ModelAnimation anim1 = modelAnimations[animIndex1];
        ModelAnimation anim2 = modelAnimations[animIndex2];
        
        animCurrentFrame1 = (animCurrentFrame1 + 1)%anim1.keyframeCount;
        animCurrentFrame2 = (animCurrentFrame2 + 1)%anim2.keyframeCount;

        // Blend the two animations
        characterModel.transform = MatrixTranslate(position.x, position.y, position.z);
        // When upperBodyBlend is ON: upper body = attack (1.0), lower body = walk (0.0)
        // When upperBodyBlend is OFF: uniform blend at 0.5 (50% walk, 50% attack)
        float blendFactor = upperBodyBlend ? 1.0f : 0.5f;
        BlendModelAnimationsBones(&characterModel, &anim1, animCurrentFrame1, &anim2, animCurrentFrame2, blendFactor, upperBodyBlend);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                // Draw character mesh, pose calculation is done in shader (GPU skinning)
                DrawMesh(characterModel.meshes[0], characterModel.materials[1], characterModel.transform);

                DrawGrid(10, 1.0f);

            EndMode3D();

            // Draw UI
            DrawText("BONE BLENDING EXAMPLE", 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("Walk (Animation 2): %s", anim1.name), 10, 35, 10, GRAY);
            DrawText(TextFormat("Attack (Animation 3): %s", anim2.name), 10, 50, 10, GRAY);
            DrawText(TextFormat("Mode: %s", upperBodyBlend ? "Upper/Lower Body Blending" : "Uniform Blending"), 10, 65, 10, GRAY);
            DrawText("SPACE - Toggle blending mode", 10, GetScreenHeight() - 20, 10, DARKGRAY);

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

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Check if a bone is part of upper body (for selective blending)
static bool IsUpperBodyBone(const char *boneName)
{
    // Common upper body bone names (adjust based on your model)
    if (TextIsEqual(boneName, "spine") || TextIsEqual(boneName, "spine1") || TextIsEqual(boneName, "spine2") ||
        TextIsEqual(boneName, "chest") || TextIsEqual(boneName, "upperChest") ||
        TextIsEqual(boneName, "neck") || TextIsEqual(boneName, "head") ||
        TextIsEqual(boneName, "shoulder") || TextIsEqual(boneName, "shoulder_L") || TextIsEqual(boneName, "shoulder_R") ||
        TextIsEqual(boneName, "upperArm") || TextIsEqual(boneName, "upperArm_L") || TextIsEqual(boneName, "upperArm_R") ||
        TextIsEqual(boneName, "lowerArm") || TextIsEqual(boneName, "lowerArm_L") || TextIsEqual(boneName, "lowerArm_R") ||
        TextIsEqual(boneName, "hand") || TextIsEqual(boneName, "hand_L") || TextIsEqual(boneName, "hand_R") ||
        TextIsEqual(boneName, "clavicle") || TextIsEqual(boneName, "clavicle_L") || TextIsEqual(boneName, "clavicle_R"))
    {
        return true;
    }
    
    // Check if bone name contains upper body keywords
    if (strstr(boneName, "spine") != NULL || strstr(boneName, "chest") != NULL ||
        strstr(boneName, "neck") != NULL || strstr(boneName, "head") != NULL ||
        strstr(boneName, "shoulder") != NULL || strstr(boneName, "arm") != NULL ||
        strstr(boneName, "hand") != NULL || strstr(boneName, "clavicle") != NULL)
    {
        return true;
    }
    
    return false;
}

// Blend two animations per-bone with selective upper/lower body blending
static void BlendModelAnimationsBones(Model *model, ModelAnimation *anim1, int frame1, 
    ModelAnimation *anim2, int frame2, float blendFactor, bool upperBodyBlend)
{
    // Validate inputs
    if (anim1->boneCount == 0 || anim1->keyframePoses == NULL ||
        anim2->boneCount == 0 || anim2->keyframePoses == NULL ||
        model->skeleton.boneCount == 0 || model->skeleton.bindPose == NULL)
    {
        return;
    }
    
    // Clamp blend factor to [0, 1]
    blendFactor = fminf(1.0f, fmaxf(0.0f, blendFactor));
    
    // Ensure frame indices are valid
    if (frame1 >= anim1->keyframeCount) frame1 = anim1->keyframeCount - 1;
    if (frame2 >= anim2->keyframeCount) frame2 = anim2->keyframeCount - 1;
    if (frame1 < 0) frame1 = 0;
    if (frame2 < 0) frame2 = 0;
    
    // Get bone count (use minimum of all to be safe)
    int boneCount = model->skeleton.boneCount;
    if (anim1->boneCount < boneCount) boneCount = anim1->boneCount;
    if (anim2->boneCount < boneCount) boneCount = anim2->boneCount;
    
    // Blend each bone
    for (int boneId = 0; boneId < boneCount; boneId++)
    {
        // Determine blend factor for this bone
        float boneBlendFactor = blendFactor;
        
        // If upper body blending is enabled, use different blend factors for upper vs lower body
        if (upperBodyBlend)
        {
            const char *boneName = model->skeleton.bones[boneId].name;
            bool isUpperBody = IsUpperBodyBone(boneName);
            
            // Upper body: use anim2 (attack), Lower body: use anim1 (walk)
            // blendFactor = 0.0 means full anim1 (walk), 1.0 means full anim2 (attack)
            if (isUpperBody) boneBlendFactor = blendFactor; // Upper body: blend towards anim2 (attack)
            else boneBlendFactor = 1.0f - blendFactor; // Lower body: blend towards anim1 (walk) - invert the blend
        }
        
        // Get transforms from both animations
        Transform *bindTransform = &model->skeleton.bindPose[boneId];
        Transform *anim1Transform = &anim1->keyframePoses[frame1][boneId];
        Transform *anim2Transform = &anim2->keyframePoses[frame2][boneId];
        
        // Blend the transforms
        Transform blended = { 0 };
        blended.translation = Vector3Lerp(anim1Transform->translation, anim2Transform->translation, boneBlendFactor);
        blended.rotation = QuaternionSlerp(anim1Transform->rotation, anim2Transform->rotation, boneBlendFactor);
        blended.scale = Vector3Lerp(anim1Transform->scale, anim2Transform->scale, boneBlendFactor);
        
        // Convert bind pose to matrix
        Matrix bindMatrix = MatrixMultiply(MatrixMultiply(
            MatrixScale(bindTransform->scale.x, bindTransform->scale.y, bindTransform->scale.z),
            QuaternionToMatrix(bindTransform->rotation)),
            MatrixTranslate(bindTransform->translation.x, bindTransform->translation.y, bindTransform->translation.z));
        
        // Convert blended transform to matrix
        Matrix blendedMatrix = MatrixMultiply(MatrixMultiply(
            MatrixScale(blended.scale.x, blended.scale.y, blended.scale.z),
            QuaternionToMatrix(blended.rotation)),
            MatrixTranslate(blended.translation.x, blended.translation.y, blended.translation.z));
        
        // Calculate final bone matrix (similar to UpdateModelAnimationBones)
        model->boneMatrices[boneId] = MatrixMultiply(MatrixInvert(bindMatrix), blendedMatrix);
    }
}

