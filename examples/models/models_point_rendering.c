#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAX_POINTS 10000000
#define MIN_POINTS 1000

float randFloat()
{
    return (float)rand() / RAND_MAX;
}

static Mesh GenMeshCustom(int numPoints)
{
    Mesh mesh = { 
        .triangleCount = 1,
        .vertexCount = numPoints,
        .vertices = (float *)MemAlloc(numPoints * 3 * sizeof(float)),
        .colors = (unsigned char*)MemAlloc(numPoints * 4 * sizeof(unsigned char)),
    };

    // https://en.wikipedia.org/wiki/Spherical_coordinate_system
    for (int i = 0; i < numPoints; i++) {
        float theta = PI * randFloat();
        float phi   = 2 * PI * randFloat();
        float r     = 10.0f * randFloat();
        mesh.vertices[i * 3 + 0] = r * sin(theta) * cos(phi);
        mesh.vertices[i * 3 + 1] = r * sin(theta) * sin(phi);
        mesh.vertices[i * 3 + 2] = r * cos(theta);
        Color color = ColorFromHSV(r * 360.0f, 1.0f, 1.0f);
        mesh.colors[i * 4 + 0] = color.r;
        mesh.colors[i * 4 + 1] = color.g;
        mesh.colors[i * 4 + 2] = color.b;
        mesh.colors[i * 4 + 3] = color.a;
    }

    // Upload mesh data from CPU (RAM) to GPU (VRAM) memory
    UploadMesh(&mesh, false);
    return mesh;
}

int main()
{
    Camera camera = {
        .position   = {3.0f, 3.0f, 3.0f},
        .target     = {0.0f, 0.0f, 0.0f},
        .up         = {0.0f, 1.0f, 0.0f},
        .fovy       = 45.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

    InitWindow(800, 600, "Point Rendering");

    Vector3 position = {0.0f, 0.0f, 0.0f};
    bool newMethod = true;
    bool numPointsChanged = false;
    int numPoints = 1000;
    Mesh mesh = GenMeshCustom(numPoints);
    Model model = LoadModelFromMesh(mesh);

    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        if (IsKeyPressed(KEY_SPACE)) newMethod = !newMethod;
        if (IsKeyPressed(KEY_UP)) {
            numPoints = (numPoints * 10 > MAX_POINTS) ? MAX_POINTS : numPoints * 10;
            numPointsChanged = true;
            TraceLog(LOG_INFO, "num points %d", numPoints);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            numPoints = (numPoints / 10 < MIN_POINTS) ? MIN_POINTS : numPoints / 10;
            numPointsChanged = true;
            TraceLog(LOG_INFO, "num points %d", numPoints);
        }

        /* re-upload a different point cloud size */
        if (numPointsChanged) {
            UnloadModel(model);
            mesh = GenMeshCustom(numPoints);
            model = LoadModelFromMesh(mesh);
            numPointsChanged = false;
        }

        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);

                /* The new method only uploads the points once to the GPU */
                if (newMethod) {
                    DrawModelPoints(model, position, 1.0f, WHITE);
                } else {
                /* The old method must continually draw the "points" (lines) */
                    for (int i = 0; i < numPoints; i++) {
                        Vector3 pos = {
                            .x = mesh.vertices[i * 3 + 0],
                            .y = mesh.vertices[i * 3 + 1],
                            .z = mesh.vertices[i * 3 + 2],
                        };
                        Color color = {
                            .r = mesh.colors[i * 4 + 0],
                            .g = mesh.colors[i * 4 + 1],
                            .b = mesh.colors[i * 4 + 2],
                            .a = mesh.colors[i * 4 + 3],
                        };
                        DrawPoint3D(pos, color);
                    }
                }

                DrawSphereWires(position, 1.0f, 10, 10, YELLOW);
            EndMode3D();

            /* Text formatting */
            Color color = WHITE;
            int fps = GetFPS();
            if ((fps < 30) && (fps >= 15)) color = ORANGE;
            else if (fps < 15) color = RED;
            DrawText(TextFormat("%2i FPS", fps), 20, 20, 40, color);
            DrawText(TextFormat("Point Count %d", numPoints), 20, 550, 40, WHITE);
            DrawText("Up - increase points", 20, 70, 20, WHITE);
            DrawText("Down - decrease points", 20, 100, 20, WHITE);
            DrawText("Space - draw mode", 20, 130, 20, WHITE);
            if (newMethod) {
                DrawText("DrawModelPoints", 20, 160, 20, GREEN);
            } else {
                DrawText("DrawPoint3D", 20, 160, 20, RED);
            }
        EndDrawing();
    }

    UnloadModel(model);
    CloseWindow();
    return 0;
}
