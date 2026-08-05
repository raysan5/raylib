/*******************************************************************************************
*
*   raylib [other] example - 3d stencil buffer portal masking
*
*   Example demonstrates 3D stencil buffer portal masking.
*
*   Example complexity rating: [★★★☆] 3/4
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
#include "rlgl.h"
#include <math.h>

// Standard OpenGL defines for Stencil Buffer and Face Culling
#ifndef GL_STENCIL_TEST
    #define GL_STENCIL_TEST       0x0B90
    #define GL_STENCIL_BUFFER_BIT 0x00000400
    #define GL_DEPTH_BUFFER_BIT   0x00000100
    #define GL_CULL_FACE          0x0B44
    #define GL_BACK               0x0405
    #define GL_ALWAYS             0x0207
    #define GL_EQUAL              0x0202
    #define GL_REPLACE            0x1E01
    #define GL_KEEP               0x1E00
    #define GL_FALSE              0
    #define GL_TRUE               1
#endif

// External OpenGL function declarations
#if defined(__cplusplus)
extern "C" {
#endif
    RLAPI void glEnable(unsigned int cap);
    RLAPI void glDisable(unsigned int cap);
    RLAPI void glClear(unsigned int mask);
    RLAPI void glStencilFunc(unsigned int func, int ref, unsigned int mask);
    RLAPI void glStencilOp(unsigned int sfail, unsigned int dpfail, unsigned int dppass);
    RLAPI void glColorMask(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
    RLAPI void glDepthMask(unsigned char flag);
    RLAPI void glCullFace(unsigned int mode);
#if defined(__cplusplus)
}
#endif

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static void DrawPortalMaskPlane(float width, float height);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib [rlgl] example - 3d stencil buffer portal");

    // Camera to navigate the 3D world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.5f, 7.0f };
    camera.target   = (Vector3){ 0.0f, 1.8f, 0.0f };
    camera.up       = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy     = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    DisableCursor();        // Lock cursor for first-person free camera controls
    SetTargetFPS(60);       // Set target frame-rate
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FREE);
        float time = (float)GetTime();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground((Color){ 15, 18, 26, 255 });

            // 1. Render Real World (Outside Portal)
            //------------------------------------------------------------------------------
            BeginMode3D(camera);
                DrawGrid(20, 1.0f);

                // Real world side pillars
                DrawCube((Vector3){ -3.5f, 2.0f, 0.0f }, 0.8f, 4.0f, 0.8f, DARKGRAY);
                DrawCubeWires((Vector3){ -3.5f, 2.0f, 0.0f }, 0.8f, 4.0f, 0.8f, ORANGE);

                DrawCube((Vector3){ 3.5f, 2.0f, 0.0f }, 0.8f, 4.0f, 0.8f, DARKGRAY);
                DrawCubeWires((Vector3){ 3.5f, 2.0f, 0.0f }, 0.8f, 4.0f, 0.8f, ORANGE);

                // Golden archway front frame
                DrawCubeWires((Vector3){ 0.0f, 2.0f, 0.0f }, 3.2f, 4.2f, 0.2f, GOLD);
                DrawCube((Vector3){ 0.0f, 0.05f, 0.0f }, 3.4f, 0.1f, 0.6f, MAROON);

                // Solid back wall for the portal (visible when viewing from behind)
                DrawCube((Vector3){ 0.0f, 2.0f, -0.05f }, 3.1f, 4.1f, 0.05f, DARKBLUE);
            EndMode3D();

            // Flush pending render batch before modifying OpenGL stencil state
            rlDrawRenderBatchActive();

            // 2. Stencil Mask Pass (Write Portal Doorway to Stencil Buffer)
            //------------------------------------------------------------------------------
            glEnable(GL_STENCIL_TEST);
            glClear(GL_STENCIL_BUFFER_BIT);     // Reset stencil mask buffer to 0

            // Enable backface culling so the back of the portal plane does not register
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            // Stencil settings: write '1' everywhere mask geometry is drawn
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

            // Disable color and depth writes (mask geometry remains invisible)
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);

            BeginMode3D(camera);
                // Draw single-sided 2D quad mask plane facing +Z
                DrawPortalMaskPlane(3.0f, 4.0f);
            EndMode3D();

            rlDrawRenderBatchActive();

            // Restore culling state
            glDisable(GL_CULL_FACE);

            // 3. Render Secret World (Only where Stencil == 1)
            //------------------------------------------------------------------------------
            // Restore color and depth writing
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);

            // Only render fragments where stencil buffer is equal to 1
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

            // Clear depth buffer in stencil region so inner geometry renders properly
            glClear(GL_DEPTH_BUFFER_BIT);

            BeginMode3D(camera);
                // Secret world neon grid floor
                DrawGrid(30, 0.8f);

                // Floating pulsing core sphere inside alternate dimension
                Vector3 corePos = { 0.0f, 2.0f + sinf(time * 2.5f) * 0.4f, -4.0f };
                DrawSphere(corePos, 1.2f, PURPLE);
                DrawSphereWires(corePos, 1.25f, 16, 16, MAGENTA);

                // Orbiting green cubes in alternate dimension
                for (int i = 0; i < 4; i++)
                {
                    float angle = time * 1.5f + i * (PI / 2.0f);
                    Vector3 pos = { 
                        sinf(angle) * 2.5f, 
                        2.0f + cosf(time * 3.0f + i) * 0.5f, 
                        -4.0f + cosf(angle) * 2.5f 
                    };
                    DrawCube(pos, 0.5f, 0.5f, 0.5f, LIME);
                    DrawCubeWires(pos, 0.52f, 0.52f, 0.52f, DARKGREEN);
                }
            EndMode3D();

            rlDrawRenderBatchActive();

            // 4. Cleanup & 2D HUD Overlay
            //------------------------------------------------------------------------------
            glDisable(GL_STENCIL_TEST);

            // Overlay HUD
            DrawRectangle(15, 15, 420, 85, Fade(BLACK, 0.75f));
            DrawRectangleLines(15, 15, 420, 85, GOLD);
            DrawText("3D STENCIL PORTAL DEMO", 28, 25, 20, GOLD);
            DrawText("Look through the golden arch into Dimension B", 28, 52, 14, RAYWHITE);
            DrawText("Controls: Mouse to look | WASD / Keys to move", 28, 72, 12, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Draw a single 1-sided 2D quad plane facing +Z used for stencil masking
static void DrawPortalMaskPlane(float width, float height)
{
    float hw = width / 2.0f;

    rlBegin(RL_QUADS);
        rlNormal3f(0.0f, 0.0f, 1.0f);
        // Counter-Clockwise (CCW) vertex order = Front Face
        rlVertex3f(-hw, 0.0f, 0.0f);
        rlVertex3f( hw, 0.0f, 0.0f);
        rlVertex3f( hw, height, 0.0f);
        rlVertex3f(-hw, height, 0.0f);
    rlEnd();
}