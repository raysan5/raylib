/*******************************************************************************************
*
*   raylib [models] example - tesseract view
*
*   NOTE: This example only works on platforms that support drag & drop (Windows, Linux, OSX, Html5?)
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by Timothy van der Valk (@arceryz) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024-2025 Timothy van der Valk (@arceryz) and Ramon Santamaria (@raysan5)
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

    InitWindow(screenWidth, screenHeight, "raylib [models] example - tesseract view");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 4.0f, 4.0f, 4.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 50.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    // Find the coordinates by setting XYZW to +-1
    Vector4 tesseract[16] = {
        {  1,  1,  1, 1 }, {  1,  1,  1, -1 },
        {  1,  1, -1, 1 }, {  1,  1, -1, -1 },
        {  1, -1,  1, 1 }, {  1, -1,  1, -1 },
        {  1, -1, -1, 1 }, {  1, -1, -1, -1 },
        { -1,  1,  1, 1 }, { -1,  1,  1, -1 },
        { -1,  1, -1, 1 }, { -1,  1, -1, -1 },
        { -1, -1,  1, 1 }, { -1, -1,  1, -1 },
        { -1, -1, -1, 1 }, { -1, -1, -1, -1 },
    };

    float rotation = 0.0f;
    Vector3 transformed[16] = { 0 };
    float wValues[16] = { 0 };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        rotation = DEG2RAD*45.0f*GetTime();

        for (int i = 0; i < 16; i++)
        {
            Vector4 p = tesseract[i];

            // Rotate the XW part of the vector
            Vector2 rotXW = Vector2Rotate((Vector2){ p.x, p.w }, rotation);
            p.x = rotXW.x;
            p.w = rotXW.y;

            // Projection from XYZW to XYZ from perspective point (0, 0, 0, 3)
            // NOTE: Trace a ray from (0, 0, 0, 3) > p and continue until W = 0
            float c = 3.0f/(3.0f - p.w);
            p.x = c*p.x;
            p.y = c*p.y;
            p.z = c*p.z;

            // Split XYZ coordinate and W values later for drawing
            transformed[i] = (Vector3){ p.x, p.y, p.z };
            wValues[i] = p.w;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                for (int i = 0; i < 16; i++)
                {
                    // Draw spheres to indicate the W value
                    DrawSphere(transformed[i], fabsf(wValues[i]*0.1f), RED);

                    for (int j = 0; j < 16; j++)
                    {
                        // Two lines are connected if they differ by 1 coordinate
                        // This way we dont have to keep an edge list
                        Vector4 v1 = tesseract[i];
                        Vector4 v2 = tesseract[j];
                        int diff = (int)(v1.x == v2.x) + (int)(v1.y == v2.y) + (int)(v1.z == v2.z) + (int)(v1.w == v2.w);

                        // Draw only differing by 1 coordinate and the lower index only (duplicate lines)
                        if (diff == 3 && i < j) DrawLine3D(transformed[i], transformed[j], MAROON);
                    }
                }
            EndMode3D();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
