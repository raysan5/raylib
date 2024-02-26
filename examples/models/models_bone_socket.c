/*******************************************************************************************
*
*   raylib [core] example - Using bones as socket for calculating the positioning of something
* 
*   Example originally created with raylib 4.5, last time updated with raylib 4.5
*
*   Example contributed by iP (@ipzaur) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024 iP (@ipzaur)
*
********************************************************************************************/

#include "raylib.h"

#include <raymath.h>
#include <string.h>

#define BONE_SOCKETS 3

#define BONE_SOCKET_HAT 0
#define BONE_SOCKET_HAND_R 1
#define BONE_SOCKET_HAND_L 2

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - bone socket");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = { 5.0f, 5.0f, 5.0f };    // Camera position
    camera.target = { 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                       // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;    // Camera projection type

    // Load gltf model
    Model characterModel = LoadModel("greenman.glb");
    Model equipModel[BONE_SOCKETS] = {
        LoadModel("greenman_hat.glb"), // index for the hat model is the same as BONE_SOCKET_HAT
        LoadModel("greenman_sword.glb"), // index for the sword model is the same as BONE_SOCKET_HAND_R
        LoadModel("greenman_shield.glb") // index for the shield model is the same as BONE_SOCKET_HAND_L
    };
    bool showEquip[3] = {true, true, true}; // for toggle on/off equip

    // Load gltf model animations
    unsigned int animsCount = 0;
    unsigned int animIndex = 0;
    unsigned int animCurrentFrame = 0;
    ModelAnimation* modelAnimations = LoadModelAnimations("greenman.glb", &animsCount);

    // indices of bones for sockets
    int boneSocketIndex[BONE_SOCKETS] = {-1, -1, -1};

    // search bones for sockets 
    for (int i = 0; i < characterModel.boneCount; i++) {
        if (strcmp(characterModel.bones[i].name, "socket_hat") == 0) {
            boneSocketIndex[BONE_SOCKET_HAT] = i;
            continue;
        }
        if (strcmp(characterModel.bones[i].name, "socket_hand_R") == 0) {
            boneSocketIndex[BONE_SOCKET_HAND_R] = i;
            continue;
        }
        if (strcmp(characterModel.bones[i].name, "socket_hand_L") == 0) {
            boneSocketIndex[BONE_SOCKET_HAND_L] = i;
            continue;
        }
    }

    Vector3 position = { 0.0f, 0.0f, 0.0f };    // Set model position
    unsigned short angle = 0; // angle for rotate character

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        // rotate character
        if (IsKeyDown(KEY_F)) {
            angle = (angle + 1) % 360;
        } else if (IsKeyDown(KEY_H)) {
            angle = (360 + angle - 1) % 360;
        }
        // Select current animation
        if (IsKeyPressed(KEY_T)) {
            animIndex = (animIndex + 1) % animsCount;
        } else if (IsKeyPressed(KEY_G)) {
            animIndex = (animIndex + animsCount - 1) % animsCount;
        }
        // toggle shown of equip
        if (IsKeyPressed(KEY_ONE)) {
            showEquip[BONE_SOCKET_HAT] = !showEquip[BONE_SOCKET_HAT];
        }
        if (IsKeyPressed(KEY_TWO)) {
            showEquip[BONE_SOCKET_HAND_R] = !showEquip[BONE_SOCKET_HAND_R];
        }
        if (IsKeyPressed(KEY_THREE)) {
            showEquip[BONE_SOCKET_HAND_L] = !showEquip[BONE_SOCKET_HAND_L];
        }

        // Update model animation
        ModelAnimation anim = modelAnimations[animIndex];
        animCurrentFrame = (animCurrentFrame + 1) % anim.frameCount;
        UpdateModelAnimation(characterModel, anim, animCurrentFrame);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // draw character
        Quaternion characterRotate = QuaternionFromAxisAngle({ 0.0f,1.0f,0.0f }, angle*DEG2RAD);
        characterModel.transform = MatrixMultiply(QuaternionToMatrix(characterRotate), MatrixTranslate(position.x, position.y, position.z));
        UpdateModelAnimation(characterModel, anim, animCurrentFrame);
        DrawMesh(characterModel.meshes[0], characterModel.materials[1], characterModel.transform);

        // draw equipments (hat, sword, shield)
        for (unsigned short i=0; i<BONE_SOCKETS; i++) {
            if (!showEquip[i]) {
                continue;
            }
            Transform* transform = &anim.framePoses[animCurrentFrame][boneSocketIndex[i]];
            Quaternion inRotation = characterModel.bindPose[boneSocketIndex[i]].rotation;
            Quaternion outRotation = transform->rotation;
            // calculate socket rotation (angle between bone in initial pose and same bone in current animation frame)
            Quaternion rotate = QuaternionMultiply(outRotation, QuaternionInvert(inRotation));
            Matrix matrixTransform = QuaternionToMatrix(rotate);
            // translate socket to its position in the current animation
            matrixTransform = MatrixMultiply(matrixTransform, MatrixTranslate(transform->translation.x, transform->translation.y, transform->translation.z));
            // rotate socket by character angle
            matrixTransform = MatrixMultiply(matrixTransform, QuaternionToMatrix(characterRotate));
            // translate socket to character position
            matrixTransform = MatrixMultiply(matrixTransform, MatrixTranslate(position.x, position.y+0.0f, position.z));
            // draw mesh at socket position with socket angle rotation
            DrawMesh(equipModel[i].meshes[0], equipModel[i].materials[1], matrixTransform);
        }

        DrawGrid(10, 1.0f);

        EndMode3D();

        DrawText("Use the T/G to switch animation", 10, 10, 20, GRAY);
        DrawText("Use the F/H to rotate character left/right", 10, 35, 20, GRAY);
        DrawText("Use the 1,2,3 to toggle shown of hat, sword and shield", 10, 60, 20, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModelAnimations(modelAnimations, animsCount);
    UnloadModel(characterModel);         // Unload character model and meshes/material
    for (unsigned short i = 0; i < BONE_SOCKETS; i++) {
        UnloadModel(equipModel[i]);      // Unload equipment model and meshes/material
    }

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}