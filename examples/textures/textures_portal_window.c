/*******************************************************************************************
*
*   raylib [textures] example - portal window
*
*   Example demonstrates rendering a second scene to a texture and projecting it onto a
*   quad to create the illusion of a portal window looking into another place. The second
*   scene is rendered with an off-axis ("oblique frustum") projection matched to the
*   viewer's actual position relative to the window, so the illusion holds up correctly
*   as the player moves and looks around, rather than only looking right from one spot
*
*   Example complexity rating: [★★★★] 4/4
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
*
*   Example contributed by PanicTitan (@PanicTitan) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 PanicTitan (@PanicTitan)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"
#include "rlgl.h"

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
// Begin portal mode 3D view
static void BeginPortalMode3D(Vector3 eye, Vector3 bottomLeft, Vector3 bottomRight, Vector3 topLeft, float nearPlane, float farPlane);
// End portal 3D mode and returns to default 2D orthographic mode
static void EndPortalMode3D(void);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - portal window");

    // Camera to navigate the "real world" (Dimension A)
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.5f, 7.0f };
    camera.target = (Vector3){ 0.0f, 1.8f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // The archway opening, in Dimension A world space (used both to draw the frame and
    // as the window rectangle the oblique projection is built from)
    Vector3 archBottomLeft  = { -1.5f, 0.0f, 0.0f };
    Vector3 archBottomRight = {  1.5f, 0.0f, 0.0f };
    Vector3 archTopLeft     = { -1.5f, 4.0f, 0.0f };

    // The archway sits at portalA and looks out onto portalB, far away in world space.
    // Every frame, Dimension B gets rendered to a texture using the same relative eye
    // and window position, shifted by the offset between the two portals
    Vector3 portalA = { 0.0f, 0.0f, 0.0f };
    Vector3 portalB = { 0.0f, 0.0f, -60.0f };
    RenderTexture2D portalView = LoadRenderTexture(480, 640);

    DisableCursor();        // Lock cursor for first-person free camera controls
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FREE);
        float time = (float)GetTime();

        // Eye and window corners, shifted into Dimension B so they match the player's
        // actual position and viewing angle relative to the archway
        Vector3 offset = Vector3Subtract(portalB, portalA);
        Vector3 eyeInB = Vector3Add(camera.position, offset);
        Vector3 blInB = Vector3Add(archBottomLeft, offset);
        Vector3 brInB = Vector3Add(archBottomRight, offset);
        Vector3 tlInB = Vector3Add(archTopLeft, offset);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        // Render Dimension B into an offscreen texture, using an oblique projection so
        // its perspective lines up with the archway exactly as the real camera sees it
        BeginTextureMode(portalView);

            ClearBackground((Color){ 10, 5, 20, 255 });

            BeginPortalMode3D(eyeInB, blInB, brInB, tlInB, 0.05f, 100.0f);

                DrawGrid(30, 0.8f);

                // Floating pulsing core sphere
                Vector3 corePos = Vector3Add(portalB, (Vector3){ 0.0f, 2.0f + sinf(time * 2.5f) * 0.4f, -4.0f });
                DrawSphere(corePos, 1.2f, PURPLE);
                DrawSphereWires(corePos, 1.25f, 16, 16, MAGENTA);

                // Orbiting cubes
                for (int i = 0; i < 4; i++)
                {
                    float angle = time * 1.5f + i * (PI / 2.0f);
                    Vector3 pos = Vector3Add(portalB, (Vector3){
                        sinf(angle) * 2.5f,
                        2.0f + cosf(time * 3.0f + i) * 0.5f,
                        -4.0f + cosf(angle) * 2.5f });

                    DrawCube(pos, 0.5f, 0.5f, 0.5f, LIME);
                    DrawCubeWires(pos, 0.52f, 0.52f, 0.52f, DARKGREEN);
                }

            EndPortalMode3D();

        EndTextureMode();

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawGrid(20, 1.0f);

                // Side pillars
                DrawCube((Vector3){ -3.5f, 2.0f, 0.0f }, 0.8f, 4.0f, 0.8f, DARKGRAY);
                DrawCubeWires((Vector3){ -3.5f, 2.0f, 0.0f }, 0.8f, 4.0f, 0.8f, ORANGE);
                DrawCube((Vector3){ 3.5f, 2.0f, 0.0f }, 0.8f, 4.0f, 0.8f, DARKGRAY);
                DrawCubeWires((Vector3){ 3.5f, 2.0f, 0.0f }, 0.8f, 4.0f, 0.8f, ORANGE);

                // Golden archway frame and solid base
                DrawCubeWires((Vector3){ 0.0f, 2.0f, 0.0f }, 3.2f, 4.2f, 0.2f, GOLD);
                DrawCube((Vector3){ 0.0f, 0.05f, 0.0f }, 3.4f, 0.1f, 0.6f, MAROON);

                // Solid backing wall, only ever seen if looking at the archway from behind
                DrawCube((Vector3){ 0.0f, 2.0f, -0.05f }, 3.1f, 4.1f, 0.05f, MAROON);

                // The portal opening itself: a plain quad textured with the Dimension B
                // render, filling the archway exactly, so nothing "leaks" outside its shape
                rlSetTexture(portalView.texture.id);
                rlBegin(RL_QUADS);
                    rlColor4ub(255, 255, 255, 255);
                    rlNormal3f(0.0f, 0.0f, 1.0f);
                    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(archBottomLeft.x, archBottomLeft.y, archBottomLeft.z);
                    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(archBottomRight.x, archBottomRight.y, archBottomRight.z);
                    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(archBottomRight.x, 4.0f, archBottomRight.z);
                    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(archTopLeft.x, archTopLeft.y, archTopLeft.z);
                rlEnd();
                rlSetTexture(0);

            EndMode3D();

            // HUD overlay
            DrawRectangle(15, 15, 520, 100, Fade(BLACK, 0.75f));
            DrawRectangleLines(15, 15, 520, 100, GOLD);
            DrawText("PORTAL WINDOW", 28, 25, 20, GOLD);
            DrawText("Look through the golden arch into Dimension B", 28, 52, 20, RAYWHITE);
            DrawText("Controls: Mouse to look | WASD to move", 28, 80, 20, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(portalView);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Starts a 3D mode using an off-axis ("oblique frustum") projection, built directly from
// an eye point and 3 corners of a rectangular window, instead of a fovy centered straight
// ahead of the camera. This is the standard technique for rendering a scene as seen through
// a window that isn't necessarily faced head-on (also used for multi-monitor and VR
// rendering) - the key difference from a normal Camera3D is that the frustum is allowed
// to be asymmetric, so perspective lines through the window line up correctly from any
// eye position, instead of behaving like a flat image pasted onto the window
static void BeginPortalMode3D(Vector3 eye, Vector3 bottomLeft, Vector3 bottomRight, Vector3 topLeft, float nearPlane, float farPlane)
{
    Vector3 right = Vector3Normalize(Vector3Subtract(bottomRight, bottomLeft));
    Vector3 up = Vector3Normalize(Vector3Subtract(topLeft, bottomLeft));
    Vector3 normal = Vector3Normalize(Vector3CrossProduct(right, up));

    // Vectors from the eye to 3 corners of the window, used to project the window onto
    // the near plane and read off how far it extends left/right/bottom/top of the eye
    Vector3 toBL = Vector3Subtract(bottomLeft, eye);
    Vector3 toBR = Vector3Subtract(bottomRight, eye);
    Vector3 toTL = Vector3Subtract(topLeft, eye);

    float dist = -Vector3DotProduct(toBL, normal);
    if (dist < 0.01f) dist = 0.01f; // Keep the eye from crossing the window plane

    float scale = nearPlane/dist;

    rlDrawRenderBatchActive();

    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();
    rlSetMatrixProjection(MatrixFrustum(
        Vector3DotProduct(right, toBL)*scale, Vector3DotProduct(right, toBR)*scale,
        Vector3DotProduct(up, toBL)*scale, Vector3DotProduct(up, toTL)*scale,
        nearPlane, farPlane));

    // View orientation is fixed to the window's own plane (looking straight through it
    // along its normal), NOT aimed at any target - that's what the asymmetric frustum
    // above is for, and is what lets the eye move off to one side without distorting
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    rlMultMatrixf(MatrixToFloat(MatrixLookAt(eye, Vector3Subtract(eye, normal), up)));

    rlEnableDepthTest();
}

// End portal 3D mode and returns to default 2D orthographic mode
// NOTE: Similar implementation to EndMode3D()
static void EndPortalMode3D(void)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlMatrixMode(RL_PROJECTION);    // Switch to projection matrix
    rlPopMatrix();                  // Restore previous matrix (projection) from matrix stack

    rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlLoadIdentity();               // Reset current matrix (modelview)

    rlDisableDepthTest();           // Disable DEPTH_TEST for 2D
}
