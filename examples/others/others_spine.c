/*******************************************************************************************
*
*   raylib spine example - Loading and plating a spine animation
*
*   This example has been created using raylib 2.5 (www.raylib.com) and Spine Runtime Libraries 3.8
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*   for details about spine, check esoteric software website http://esotericsoftware.com/
*
*   Example contributed by WEREMSOFT (@werem) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2020 WEREMSOFT (@werem) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "./spine/spine.h"
#include "./spine/extension.h"
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib spine example - basic window");

    spAtlas* atlas = {0};
    spSkeletonJson* json;
    spSkeletonData* skeletonData;
    spSkeleton* skeleton;
    spAnimationStateData* animationStateData;
    spAnimationState* animationState;

    // Init spine
    atlas = spAtlas_createFromFile("resources/spine/dragon/NewDragon.atlas", 0);
    json = spSkeletonJson_create(atlas);

    skeletonData = spSkeletonJson_readSkeletonDataFile(json, "assets/dragon/NewDragon.json");

    if (!skeletonData) {
        printf("%s\n", json->error);
        spSkeletonJson_dispose(json);
        printf("ERROR!\n");
    }

    skeleton = spSkeleton_create(skeletonData);
    skeleton->scaleX = 0.5;
    skeleton->scaleY = 0.5;

    // Create the spAnimationStateData
    animationStateData = spAnimationStateData_create(skeletonData);
    animationState = spAnimationState_create(animationStateData);
    // Add the animation "walk" to track 0, without delay, and let it loop indefinitely
    int track = 0;
    int loop = 1;
    float delay = 0;
    spAnimationState_addAnimationByName(animationState, track, "flying", loop, delay);
    spAnimationState_addAnimationByName(animationState, 0, "flying", 1, 0);
    spAnimationState_update(animationState,.0f);
    spAnimationState_apply(animationState, skeleton);
    spSkeleton_updateWorldTransform(skeleton);

    Vector3 skeletonPosition = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0};

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        spAnimationState_update(animationState, GetFrameTime());
        spAnimationState_apply(animationState, skeleton);
        spSkeleton_updateWorldTransform(skeleton);

        DrawSkeleton(skeleton, skeletonPosition);

        DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    spAtlas_dispose(atlas);
    spSkeleton_dispose(skeleton);
    Texture2dDestroy(); // Destroy textures loaded by spine
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}