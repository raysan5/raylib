#include "raylib.h"
#include "rcamera.h"

#define RAYGIZMO_IMPLEMENTATION
#include "gizmo.h"
#undef RAYGIZMO_IMPLEMENTATION

void updateCamera(Camera3D* camera) {
    float rot_speed = 0.003f;
    float move_speed = 0.01f;
    float zoom_speed = 1.0f;

    bool is_middle = IsMouseButtonDown(2);
    bool is_shift = IsKeyDown(KEY_LEFT_SHIFT);
    Vector2 mouse_delta = GetMouseDelta();

    if (is_middle && is_shift) {
        CameraMoveRight(camera, -move_speed * mouse_delta.x, true);

        Vector3 right = GetCameraRight(camera);
        Vector3 up = Vector3CrossProduct(
            Vector3Subtract(camera->position, camera->target), right
        );
        up = Vector3Scale(Vector3Normalize(up), move_speed * mouse_delta.y);
        camera->position = Vector3Add(camera->position, up);
        camera->target = Vector3Add(camera->target, up);
    } else if (is_middle) {
        CameraYaw(camera, -rot_speed * mouse_delta.x, true);
        CameraPitch(camera, rot_speed * mouse_delta.y, true, true, false);
    }

    CameraMoveToTarget(camera, -GetMouseWheelMove() * zoom_speed);
}

int main(void) {
    const int screen_width = 800;
    const int screen_height = 450;
    InitWindow(screen_width, screen_height, "Gizmo");
    SetTargetFPS(60);

    Camera3D camera;
    camera.fovy = 45.0f;
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.position = (Vector3){5.0f, 5.0f, 5.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.projection = CAMERA_PERSPECTIVE;

    Model model = LoadModelFromMesh(GenMeshTorus(0.3, 1.5, 16.0, 16.0));

    LoadGizmo();

    while (!WindowShouldClose()) {
        updateCamera(&camera);

        BeginDrawing();
        {
            ClearBackground(DARKGRAY);
            rlEnableDepthTest();

            BeginMode3D(camera);
            {
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
            }
            EndMode3D();

            // Immediately update and draw gizmo
            Vector3 position = {
                model.transform.m12, model.transform.m13, model.transform.m14};
            Matrix transform = UpdateGizmo(camera, position);

            // Apply gizmo-produced transformation to the model
            model.transform = MatrixMultiply(model.transform, transform);

            // Draw camera controll keys
            DrawRectangle(0, 0, 280, 90, RAYWHITE);
            DrawText("CAMERA:", 5, 5, 20, RED);
            DrawText("    zoom: wheel", 5, 25, 20, RED);
            DrawText("    rotate: mmb", 5, 45, 20, RED);
            DrawText("    translate: shift + mmb", 5, 65, 20, RED);
        }
        EndDrawing();
    }

    UnloadGizmo();
    UnloadModel(model);
    CloseWindow();

    return 0;
}

