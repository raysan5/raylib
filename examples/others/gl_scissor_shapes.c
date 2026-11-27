/*******************************************************************************************
*
*   raylib [others] example - scissor shapes
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 6.0
*
*   Example contributed by David Buzatto (@davidbuzatto) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 David Buzatto (@davidbuzatto)
*
********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"

#include <math.h>       // Required for: sinf()

// BeginScissorMode() only clips to an axis-aligned rectangle, since it maps
// directly to glScissor. Clipping to an arbitrary shape (the same idea as
// Graphics2D.setClip(Shape) in Java2D) needs the stencil buffer instead:
// the clip shape is rasterized into the stencil buffer first, then further
// draws are only let through where the stencil was written.
//
// rlgl has no stencil wrapper (see raysan5/raylib discussion #2964), so this
// example calls the handful of OpenGL 1.0 stencil functions directly - they
// don't need glad/extension loading, same as glScissor doesn't.
#if defined(__APPLE__)
    #include <OpenGL/gl.h>
#else
    #if !defined(APIENTRY)
        #if defined(_WIN32)
            #define APIENTRY __stdcall
        #else
            #define APIENTRY
        #endif
    #endif
    #if !defined(WINGDIAPI) && defined(_WIN32)
        #define WINGDIAPI __declspec(dllimport)
    #endif

    #include <GL/gl.h>
#endif

// mask is a callback instead of a fixed shape, so the clip region can be any
// combination of Draw* calls (circle, polygon, text, several shapes together)
typedef void (*ScissorMaskDrawFunc)(void *userData);

static void BeginScissorModeShape(ScissorMaskDrawFunc mask, void *userData)
{
    rlDrawRenderBatchActive();  // Flush before touching stencil state
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);

    // Pass 1: rasterize the mask into the stencil buffer only, no color write
    rlColorMask(false, false, false, false);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    mask(userData);
    rlDrawRenderBatchActive();  // Flush the mask draw

    // Pass 2: subsequent draws only survive where the stencil was written
    rlColorMask(true, true, true, true);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

static void EndScissorModeShape(void)
{
    rlDrawRenderBatchActive();
    glDisable(GL_STENCIL_TEST);
}

// Two example mask shapes, drawn in WHITE (the color is irrelevant here,
// only the stencil write from pass 1 matters)
typedef struct { Vector2 center; float radius; } CircleMask;
static void DrawCircleMask(void *userData)
{
    CircleMask *m = (CircleMask *)userData;
    DrawCircleV(m->center, m->radius, WHITE);
}

typedef struct { Vector2 center; float radius; int sides; } PolyMask;
static void DrawPolyMask(void *userData)
{
    PolyMask *m = (PolyMask *)userData;
    DrawPoly(m->center, m->sides, m->radius, 0.0f, WHITE);
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

    InitWindow(screenWidth, screenHeight, "raylib [others] example - scissor shapes");

    Vector2 circleCenter = { screenWidth/2.0f - 120, screenHeight/2.0f };
    Vector2 hexCenter = { screenWidth/2.0f + 120, screenHeight/2.0f };
    float maskRadius = 100.0f;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        circleCenter.y = screenHeight/2.0f + sinf((float)GetTime())*40.0f;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Circle-shaped clip: the full-screen rectangle only shows inside the circle
            CircleMask circleMask = { circleCenter, maskRadius };
            BeginScissorModeShape(DrawCircleMask, &circleMask);
                DrawRectangle(0, 0, screenWidth, screenHeight, BLUE);
            EndScissorModeShape();

            // Hexagon-shaped clip: same mechanism, a different mask shape
            PolyMask hexMask = { hexCenter, maskRadius, 6 };
            BeginScissorModeShape(DrawPolyMask, &hexMask);
                DrawRectangle(0, 0, screenWidth, screenHeight, ORANGE);
            EndScissorModeShape();

            DrawText("BeginScissorMode() only clips to a rectangle (glScissor)", 10, 10, 10, DARKGRAY);
            DrawText("This clips to arbitrary shapes instead, using the stencil buffer", 10, 25, 10, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
