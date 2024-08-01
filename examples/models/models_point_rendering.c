#include "raylib.h"
#include <stdlib.h>
#include <math.h>

#define NUM_POINTS 1000

float randFloat()
{
    return (float)rand() / RAND_MAX;
}

static Mesh GenMeshCustom(void)
{
    Mesh mesh = { 
        .triangleCount = 1,
        .vertexCount = NUM_POINTS,
        .vertices = (float *)MemAlloc(NUM_POINTS * 3 * sizeof(float)),
    };

    // https://en.wikipedia.org/wiki/Spherical_coordinate_system
    for (int i = 0; i < NUM_POINTS; i++) {
        float theta = PI * randFloat();
        float phi   = 2 * PI * randFloat();
        float r     = randFloat();
        mesh.vertices[i * 3 + 0] = r * sin(theta) * cos(phi);
        mesh.vertices[i * 3 + 1] = r * sin(theta) * sin(phi);
        mesh.vertices[i * 3 + 2] = r * cos(theta);
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

    Mesh mesh = GenMeshCustom();
    Model model = LoadModelFromMesh(mesh);
    Vector3 position = {0.0f, 0.0f, 0.0f};
    bool drawSphere = true;

    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        if (IsKeyPressed(KEY_SPACE)) drawSphere = !drawSphere;

        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);
                DrawModelPoints(model, position, 1.0f, WHITE);
                if (drawSphere)
                    DrawSphereWires(position, 1.0f, 10, 10, WHITE);
            EndMode3D();
        EndDrawing();
    }

    UnloadModel(model);
    CloseWindow();
    return 0;
}
