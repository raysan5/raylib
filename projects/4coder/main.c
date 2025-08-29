#include <math.h>
#include "raylib.h"

int main() {
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib");

    Camera cam;
    cam.position = (Vector3){ 0.0f, 10.0f, 8.f };
    cam.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    cam.up = (Vector3){ 0.0f, 1.f, 0.0f };
    cam.fovy = 60.0f;

    Vector3 cubePos = { 0.0f, 0.0f, 0.0f };

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        cam.position.x = sin(GetTime())*10.0f;
        cam.position.z = cos(GetTime())*10.0f;

        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(cam);
                DrawCube(cubePos, 2.f, 2.f, 2.f, RED);
                DrawCubeWires(cubePos, 2.f, 2.f, 2.f, MAROON);
                DrawGrid(10, 1.f);
            EndMode3D();
            DrawText("This is a raylib example", 10, 40, 20, DARKGRAY);
            DrawFPS(10, 10);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
