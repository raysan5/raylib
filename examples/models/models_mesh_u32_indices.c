/*******************************************************************************************
*
*   raylib [models] example - Mesh with 16-bit and 32-bit indices
*
*   Confirms that legacy u16 indices and u32 index 65536 are both preserved, drawn,
*   used for tangent generation, and used for mesh collision tests.
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#include <string.h>

// Set the position, normal and texture coordinate for one mesh vertex
static void SetMeshVertex(Mesh *mesh, int index, float x, float y, float u, float v)
{
    mesh->vertices[index*3] = x;
    mesh->vertices[index*3 + 1] = y;
    mesh->normals[index*3 + 2] = 1.0f;
    mesh->texcoords[index*2] = u;
    mesh->texcoords[index*2 + 1] = v;
}

int main(void)
{
    const int u32VertexCount = 65537;

    InitWindow(800, 450, "raylib [models] example - mesh u16 and u32 indices");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, 6.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Legacy u16 mesh regression case
    Mesh mesh16 = { 0 };
    mesh16.vertexCount = 3;
    mesh16.triangleCount = 1;
    mesh16.vertices = (float *)MemAlloc(mesh16.vertexCount*3*sizeof(float));
    mesh16.texcoords = (float *)MemAlloc(mesh16.vertexCount*2*sizeof(float));
    mesh16.normals = (float *)MemAlloc(mesh16.vertexCount*3*sizeof(float));
    mesh16.indices = (unsigned short *)MemAlloc(3*sizeof(unsigned short));
    memset(mesh16.vertices, 0, mesh16.vertexCount*3*sizeof(float));
    memset(mesh16.texcoords, 0, mesh16.vertexCount*2*sizeof(float));
    memset(mesh16.normals, 0, mesh16.vertexCount*3*sizeof(float));
    SetMeshVertex(&mesh16, 0, -1.0f, -1.0f, 0.0f, 0.0f);
    SetMeshVertex(&mesh16, 1, 1.0f, -1.0f, 1.0f, 0.0f);
    SetMeshVertex(&mesh16, 2, 0.0f, 1.0f, 0.5f, 1.0f);
    mesh16.indices[0] = 0;
    mesh16.indices[1] = 1;
    mesh16.indices[2] = 2;
    GenMeshTangents(&mesh16);
    UploadMesh(&mesh16, false);
    Model model16 = LoadModelFromMesh(mesh16);

    // u32 mesh regression case: the third vertex cannot be addressed by a u16 index
    Mesh mesh32 = { 0 };
    mesh32.vertexCount = u32VertexCount;
    mesh32.triangleCount = 1;
    mesh32.vertices = (float *)MemAlloc(mesh32.vertexCount*3*sizeof(float));
    mesh32.texcoords = (float *)MemAlloc(mesh32.vertexCount*2*sizeof(float));
    mesh32.normals = (float *)MemAlloc(mesh32.vertexCount*3*sizeof(float));
    mesh32.indices32 = (unsigned int *)MemAlloc(3*sizeof(unsigned int));
    mesh32.indexType = MESH_INDEX_UINT32;
    memset(mesh32.vertices, 0, mesh32.vertexCount*3*sizeof(float));
    memset(mesh32.texcoords, 0, mesh32.vertexCount*2*sizeof(float));
    memset(mesh32.normals, 0, mesh32.vertexCount*3*sizeof(float));
    SetMeshVertex(&mesh32, 0, -1.0f, -1.0f, 0.0f, 0.0f);
    SetMeshVertex(&mesh32, 1, 1.0f, -1.0f, 1.0f, 0.0f);
    SetMeshVertex(&mesh32, 65536, 0.0f, 1.0f, 0.5f, 1.0f);
    mesh32.indices32[0] = 0;
    mesh32.indices32[1] = 1;
    mesh32.indices32[2] = 65536;
    GenMeshTangents(&mesh32);
    UploadMesh(&mesh32, false);
    Model model32 = LoadModelFromMesh(mesh32);

    Ray ray16 = { (Vector3){ -1.5f, 0.0f, 5.0f }, (Vector3){ 0.0f, 0.0f, -1.0f } };
    Ray ray32 = { (Vector3){ 1.5f, 0.0f, 5.0f }, (Vector3){ 0.0f, 0.0f, -1.0f } };
    bool u16Collision = GetRayCollisionMesh(ray16, mesh16, MatrixTranslate(-1.5f, 0.0f, 0.0f)).hit;
    bool u32Collision = GetRayCollisionMesh(ray32, mesh32, MatrixTranslate(1.5f, 0.0f, 0.0f)).hit;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                DrawModel(model16, (Vector3){ -1.5f, 0.0f, 0.0f }, 1.0f, DARKBLUE);
                DrawModel(model32, (Vector3){ 1.5f, 0.0f, 0.0f }, 1.0f, MAROON);
                DrawGrid(10, 1.0f);
            EndMode3D();

            DrawText("Left: legacy u16 mesh     Right: u32 mesh with index 65536", 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("Mesh collision: u16 %s, u32 %s", u16Collision? "ok" : "failed", u32Collision? "ok" : "failed"), 10, 36, 20, DARKGRAY);
            DrawText("Both paths also run GenMeshTangents()", 10, 62, 20, DARKGRAY);
            DrawFPS(10, 96);
        EndDrawing();
    }

    UnloadModel(model16);
    UnloadModel(model32);
    CloseWindow();

    return 0;
}
