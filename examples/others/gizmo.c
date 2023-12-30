/*******************************************************************************************
*
*   raylib [gizmo] example - Gizmo gadget for an interactive object 3D transformations
*
*   Example originally created with raylib 5.0, last time updated with raylib 5.0
*
*   Example contributed by Alexey Karnachev (@alexeykarnachev) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023-2024 Alexey Karnachev (@alexeykarnachev)
*
********************************************************************************************/

#include "raylib.h"
#include "rcamera.h"

#define RAYGIZMO_IMPLEMENTATION
#include "gizmo.h"
#undef RAYGIZMO_IMPLEMENTATION

#define CAMERA_ROT_SPEED 0.003f
#define CAMERA_MOVE_SPEED 0.01f
#define CAMERA_ZOOM_SPEED 1.0f

// Updates the camera in the orbital-style, i.e camera rotates around the look-at point by the orbit
static void updateCamera(Camera3D* camera) {
    bool isMMBDown = IsMouseButtonDown(2);
    bool isShiftDown = IsKeyDown(KEY_LEFT_SHIFT);
    Vector2 mouseDelta = GetMouseDelta();

    // Shift + MMB + mouse move -> change the camera position in the right-direction plane
    if (isMMBDown && isShiftDown) {
        CameraMoveRight(camera, -CAMERA_MOVE_SPEED * mouseDelta.x, true);

        Vector3 right = GetCameraRight(camera);
        Vector3 up = Vector3CrossProduct(
            Vector3Subtract(camera->position, camera->target), right
        );
        up = Vector3Scale(Vector3Normalize(up), CAMERA_MOVE_SPEED * mouseDelta.y);
        camera->position = Vector3Add(camera->position, up);
        camera->target = Vector3Add(camera->target, up);
    // Rotate the camera around the look-at point
    } else if (isMMBDown) {
        CameraYaw(camera, -CAMERA_ROT_SPEED * mouseDelta.x, true);
        CameraPitch(camera, CAMERA_ROT_SPEED * mouseDelta.y, true, true, false);
    }

    // Bring camera closer (or move away), to the look-at point 
    CameraMoveToTarget(camera, -GetMouseWheelMove() * CAMERA_ZOOM_SPEED);
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "raylib [gizmo] example - gizmo gadget");

    // Define 3D perspective camera
    Camera3D camera;
    camera.fovy = 45.0f;
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.position = (Vector3){5.0f, 5.0f, 5.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.projection = CAMERA_PERSPECTIVE;

    Model model = LoadModelFromMesh(GenMeshTorus(0.3, 1.5, 16.0, 16.0));  // Create simple torus model
    loadGizmo();                                                          // Load gizmo

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        updateCamera(&camera);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(DARKGRAY);
            rlEnableDepthTest();

            BeginMode3D(camera);
                // Draw main model
                DrawModel(model, (Vector3){0.0, 0.0, 0.0}, 1.0, PURPLE);

                // Draw coordinates grid
                rlSetLineWidth(1.0);
                DrawGrid(100.0, 1.0);

                // Draw coordinate x, y and z axis
                rlSetLineWidth(2.0);
                DrawLine3D(
                    (Vector3){-50.0f, 0.0f, 0.0f},
                    (Vector3){50.0f, 0.0f, 0.0f},
                    RED
                );
                DrawLine3D(
                    (Vector3){0.0f, -50.0f, 0.0f},
                    (Vector3){0.0f, 50.0f, 0.0f},
                    GREEN
                );
                DrawLine3D(
                    (Vector3){0.0f, 0.0f, -50.0f},
                    (Vector3){0.0f, 0.0f, 50.0f},
                    DARKBLUE
                );
            EndMode3D();

            // Immediately update and draw gizmo
            Vector3 position = {
                model.transform.m12, model.transform.m13, model.transform.m14};
            Matrix transform = updateAndDrawGizmo(camera, position);

            // Apply gizmo-produced transformation to the model
            model.transform = MatrixMultiply(model.transform, transform);

            // Draw camera controll keys
            DrawRectangle(0, 0, 280, 90, RAYWHITE);
            DrawText("CAMERA:", 5, 5, 20, RED);
            DrawText("    zoom: wheel", 5, 25, 20, RED);
            DrawText("    rotate: mmb", 5, 45, 20, RED);
            DrawText("    translate: shift + mmb", 5, 65, 20, RED);
        EndDrawing();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    unloadGizmo();       // Unload gizmo
    UnloadModel(model);  // Unload model

    CloseWindow();       // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

