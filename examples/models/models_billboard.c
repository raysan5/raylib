/*******************************************************************************************
*
*   raylib [models] example - Drawing billboards
*
*   Example originally created with raylib 1.3, last time updated with raylib 3.5
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - drawing billboards");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 4.0f, 5.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    Texture2D bill = LoadTexture("resources/billboard.png");    // Our billboard texture
    Vector3 billPositionStatic = { 0.0f, 2.0f, 0.0f };          // Position of static billboard
    Vector3 billPositionRotating = { 1.0f, 2.0f, 1.0f };        // Position of rotating billboard

    // Entire billboard texture, source is used to take a segment from a larger texture.
    Rectangle source = { 0.0f, 0.0f, (float)bill.width, (float)bill.height };

    // NOTE: Billboard locked on axis-Y
    Vector3 billUp = { 0.0f, 1.0f, 0.0f };

    // Rotate around origin
    // Here we choose to rotate around the image center
    // NOTE: (-1, 1) is the range where origin.x, origin.y is inside the texture
    Vector2 rotateOrigin = { 0.0f };

    // Distance is needed for the correct billboard draw order
    // Larger distance (further away from the camera) should be drawn prior to smaller distance.
    float distanceStatic;
    float distanceRotating;
    float rotation = 0.0f;

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);

        rotation += 0.4f;
        distanceStatic = Vector3Distance(camera.position, billPositionStatic);
        distanceRotating = Vector3Distance(camera.position, billPositionRotating);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawGrid(10, 1.0f);        // Draw a grid

                // Draw order matters!
                if (distanceStatic > distanceRotating) 
                {
                    DrawBillboard(camera, bill, billPositionStatic, 2.0f, WHITE);
                    DrawBillboardPro(camera, bill, source, billPositionRotating, billUp, (Vector2) {1.0f, 1.0f}, rotateOrigin, rotation, WHITE);
                } 
                else
                {
                    DrawBillboardPro(camera, bill, source, billPositionRotating, billUp, (Vector2) {1.0f, 1.0f}, rotateOrigin, rotation, WHITE);
                    DrawBillboard(camera, bill, billPositionStatic, 2.0f, WHITE);
                }
                
            EndMode3D();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(bill);        // Unload texture

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}