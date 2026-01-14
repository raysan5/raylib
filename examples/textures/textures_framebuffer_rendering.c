/*******************************************************************************************
*
*   raylib [textures] example - framebuffer rendering
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Jack Boakes (@jackboakes) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Jack Boakes (@jackboakes)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static void DrawCameraPrism(Camera3D camera, float aspect, Color color);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    const int splitWidth = screenWidth/2;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - framebuffer rendering");

    // Camera to look at the 3D world
    Camera3D subjectCamera = { 0 };
    subjectCamera.position = (Vector3){ 5.0f, 5.0f, 5.0f };
    subjectCamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    subjectCamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    subjectCamera.fovy = 45.0f;
    subjectCamera.projection = CAMERA_PERSPECTIVE;

    // Camera to observe the subject camera and 3D world
    Camera3D observerCamera = { 0 };
    observerCamera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    observerCamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    observerCamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    observerCamera.fovy = 45.0f;
    observerCamera.projection = CAMERA_PERSPECTIVE;

    // Set up render textures
    RenderTexture2D observerTarget = LoadRenderTexture(splitWidth, screenHeight);
    Rectangle observerSource = { 0.0f, 0.0f, (float)observerTarget.texture.width, -(float)observerTarget.texture.height };
    Rectangle observerDest = { 0.0f, 0.0f, (float)splitWidth, (float)screenHeight };

    RenderTexture2D subjectTarget = LoadRenderTexture(splitWidth, screenHeight);
    Rectangle subjectSource = { 0.0f, 0.0f, (float)subjectTarget.texture.width, -(float)subjectTarget.texture.height };
    Rectangle subjectDest = { (float)splitWidth, 0.0f, (float)splitWidth, (float)screenHeight };
    const float textureAspectRatio = (float)subjectTarget.texture.width/(float)subjectTarget.texture.height;

    // Rectangles for cropping render texture
    const float captureSize = 128.0f;
    Rectangle cropSource = { (subjectTarget.texture.width - captureSize)/2.0f, (subjectTarget.texture.height - captureSize)/2.0f, captureSize, -captureSize };
    Rectangle cropDest = { splitWidth + 20, 20, captureSize, captureSize};

    SetTargetFPS(60);
    DisableCursor();
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&observerCamera, CAMERA_FREE);
        UpdateCamera(&subjectCamera, CAMERA_ORBITAL);

        if (IsKeyPressed(KEY_R)) observerCamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };

        // Build LHS observer view texture
        BeginTextureMode(observerTarget);

            ClearBackground(RAYWHITE);

            BeginMode3D(observerCamera);

                DrawGrid(10, 1.0f);
                DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, GOLD);
                DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, PINK);
                DrawCameraPrism(subjectCamera, textureAspectRatio, GREEN);

            EndMode3D();

            DrawText("Observer View", 10, observerTarget.texture.height - 30, 20, BLACK);
            DrawText("WASD + Mouse to Move", 10, 10, 20, DARKGRAY);
            DrawText("Scroll to Zoom", 10, 30, 20, DARKGRAY);
            DrawText("R to Reset Observer Target", 10, 50, 20, DARKGRAY);

        EndTextureMode();

        // Build RHS subject view texture
        BeginTextureMode(subjectTarget);

            ClearBackground(RAYWHITE);

            BeginMode3D(subjectCamera);

                DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, GOLD);
                DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, PINK);
                DrawGrid(10, 1.0f);

            EndMode3D();

            DrawRectangleLines((subjectTarget.texture.width - captureSize)/2, (subjectTarget.texture.height - captureSize)/2, captureSize, captureSize, GREEN);
            DrawText("Subject View", 10, subjectTarget.texture.height - 30, 20, BLACK);

        EndTextureMode();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            // Draw observer texture LHS
            DrawTexturePro(observerTarget.texture, observerSource, observerDest, (Vector2){0.0f, 0.0f }, 0.0f, WHITE);

            // Draw subject texture RHS
            DrawTexturePro(subjectTarget.texture, subjectSource, subjectDest, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);

            // Draw the small crop overlay on top
            DrawTexturePro(subjectTarget.texture, cropSource, cropDest, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
            DrawRectangleLinesEx(cropDest, 2, BLACK);

            // Draw split screen divider line
            DrawLine(splitWidth, 0, splitWidth, screenHeight, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(observerTarget);
    UnloadRenderTexture(subjectTarget);
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
static void DrawCameraPrism(Camera3D camera, float aspect, Color color)
{
    float length = Vector3Distance(camera.position, camera.target);
    // Define the 4 corners of the camera's prism plane sliced at the target in Normalized Device Coordinates
    Vector3 planeNDC[4] = {
        { -1.0f, -1.0f, 1.0f }, // Bottom Left
        {  1.0f, -1.0f, 1.0f }, // Bottom Right
        {  1.0f,  1.0f, 1.0f }, // Top Right
        { -1.0f,  1.0f, 1.0f }  // Top Left
    };

    // Build the matrices
    Matrix view = GetCameraMatrix(camera);
    Matrix proj = MatrixPerspective(camera.fovy * DEG2RAD, aspect, 0.05f, length);
    // Combine view and projection so we can reverse the full camera transform
    Matrix viewProj = MatrixMultiply(view, proj);
    // Invert the view-projection matrix to unproject points from NDC space back into world space
    Matrix inverseViewProj = MatrixInvert(viewProj);

    // Transform the 4 plane corners from NDC into world space
    Vector3 corners[4];
    for (int i = 0; i < 4; i++)
    {
        float x = planeNDC[i].x;
        float y = planeNDC[i].y;
        float z = planeNDC[i].z;

        // Multiply NDC position by the inverse view-projection matrix
        // This produces a homogeneous (x, y, z, w) position in world space
        float vx = inverseViewProj.m0*x + inverseViewProj.m4*y + inverseViewProj.m8*z + inverseViewProj.m12;
        float vy = inverseViewProj.m1*x + inverseViewProj.m5*y + inverseViewProj.m9*z + inverseViewProj.m13;
        float vz = inverseViewProj.m2*x + inverseViewProj.m6*y + inverseViewProj.m10*z + inverseViewProj.m14;
        float vw = inverseViewProj.m3*x + inverseViewProj.m7*y + inverseViewProj.m11*z + inverseViewProj.m15;

        corners[i] = (Vector3){ vx/vw, vy/vw, vz/vw };
    }

    // Draw the far plane sliced at the target
    DrawLine3D(corners[0], corners[1], color);
    DrawLine3D(corners[1], corners[2], color);
    DrawLine3D(corners[2], corners[3], color);
    DrawLine3D(corners[3], corners[0], color);

    // Draw the prism lines from the far plane to the camera position
    for (int i = 0; i < 4; i++)
    {
        DrawLine3D(camera.position, corners[i], color);
    }
}