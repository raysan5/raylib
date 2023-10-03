/*******************************************************************************************
 *
 *   raylib [models] example -Draw a cube and transform it by translating, scaling,
 *   and rotating.
 *
 *   Example originally created with raylib 4.5, last time updated with raylib 4.5
 *
 *   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
 *   BSD-like license that allows static linking with closed source software
 *
 *   Copyright (c) 2014-2023 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

// Transforms of the cube
Vector3 position = (Vector3){0.0f, 0.0f, 0.0f};
Vector3 scale = (Vector3){1.0f, 1.0f, 1.0f};
Vector3 rotation = (Vector3){0.0f, 0.0f, 0.0f};

// Define movement speed
const float translationSpeed = 0.2f;
const float scaleSpeed = 0.1f;
const float rotationSpeed = 2.0f;

bool left()
{
    return IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
}

bool right()
{
    return IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
}

bool up()
{
    return IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
}

bool down()
{
    return IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
}

bool in()
{
    return IsKeyDown(KEY_E) || IsKeyDown(KEY_PAGE_DOWN);
}

bool out()
{
    return IsKeyDown(KEY_Q) || IsKeyDown(KEY_PAGE_UP);
}

bool shouldScale()
{
    return IsKeyDown(KEY_LEFT_CONTROL);
}

bool shouldMove()
{
    return IsKeyDown(KEY_LEFT_SHIFT);
}

void handleInput()
{
    if (shouldMove())
    {
        if (left())
            position.x -= translationSpeed;
        else if (right())
            position.x += translationSpeed;
        if (down())
            position.y -= translationSpeed;
        else if (up())
            position.y += translationSpeed;
        if (out())
            position.z += translationSpeed;
        else if (in())
            position.z -= translationSpeed;
    }
    else if (shouldScale())
    {
        if (left())
            scale.x -= scaleSpeed;
        else if (right())
            scale.x += scaleSpeed;
        if (down())
            scale.y -= scaleSpeed;
        else if (up())
            scale.y += scaleSpeed;
        if (out())
            scale.z += scaleSpeed;
        else if (in())
            scale.z -= scaleSpeed;
    }
    else
    {
        if (left())
            rotation.x -= rotationSpeed;
        else if (right())
            rotation.x += rotationSpeed;
        if (down())
            rotation.y -= rotationSpeed;
        else if (up())
            rotation.y += rotationSpeed;
        if (out())
            rotation.z += rotationSpeed;
        else if (in())
            rotation.z -= rotationSpeed;
    }
}
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - model transformations");

    // Generate a cube shaped model to play with
    Model cube = LoadModelFromMesh(GenMeshCube(2.0f, 2.0f, 2.0f));

    // Define the camera to look into our 3d world
    Camera camera = {0};
    camera.position = (Vector3){0.0f, 10.0f, 10.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        handleInput();

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // Set translation matrix
        Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

        // Multiply rotation matrices with each other, to rotate around each axis.
        Matrix matRotX = MatrixRotate((Vector3){1.0f, 0.0f, 0.0f}, rotation.x * DEG2RAD);
        Matrix matRotY = MatrixRotate((Vector3){0.0f, 1.0f, 0.0f}, rotation.y * DEG2RAD);
        Matrix matRotZ = MatrixRotate((Vector3){0.0f, 0.0f, 1.0f}, rotation.z * DEG2RAD);
        Matrix matRotCombined = MatrixMultiply(matRotX, matRotY);
        matRotCombined = MatrixMultiply(matRotCombined, matRotZ);

        // Set scale matrix
        Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);

        DrawModelPro(cube, matTranslation, matRotCombined, matScale, RED);
        DrawModelWiresPro(cube, matTranslation, matRotCombined, matScale, BLUE);
        DrawGrid(10, 1.0f); // Draw a grid

        EndMode3D();

        DrawFPS(10, 10);

        DrawText("Left Shift + W/A/S/D to translate", 10, 40, 20, BLUE);
        DrawText("Left Ctrl + W/A/S/D to scale", 10, 70, 20, BLUE);
        DrawText("W/A/S/D to rotate", 10, 100, 20, BLUE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}