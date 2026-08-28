/*******************************************************************************************
*
*   raylib [models] example - Mesh with 32-bit indices
*
*   Confirms that index 65536 is preserved instead of being truncated to u16.
*
********************************************************************************************/

#include "raylib.h"

int main(void)
{
    const int vertexCount = 65537;

    InitWindow(800, 450, "raylib [models] example - mesh u32 indices");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, 4.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = 1;
    mesh.vertices = (float *)MemAlloc(vertexCount*3*sizeof(float));
    mesh.indices32 = (unsigned int *)MemAlloc(3*sizeof(unsigned int));
    mesh.indexType = MESH_INDEX_UINT32;

    mesh.vertices[0] = -1.0f;
    mesh.vertices[1] = -1.0f;
    mesh.vertices[3] = 1.0f;
    mesh.vertices[4] = -1.0f;
    mesh.vertices[65536*3 + 1] = 1.0f;

    mesh.indices32[0] = 0;
    mesh.indices32[1] = 1;
    mesh.indices32[2] = 65536;

    UploadMesh(&mesh, false);
    Model model = LoadModelFromMesh(mesh);

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                DrawModel(model, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, MAROON);
                DrawGrid(10, 1.0f);
            EndMode3D();

            DrawText("Mesh index 65536 is stored and drawn as u32", 10, 10, 20, DARKGRAY);
            DrawText("Expected result: one non-degenerate red triangle", 10, 36, 20, DARKGRAY);
            DrawFPS(10, 70);
        EndDrawing();
    }

    UnloadModel(model);
    CloseWindow();

    return 0;
}
