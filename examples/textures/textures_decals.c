/*******************************************************************************************
*
*   raylib [textures] example - decals
*
*   Example demonstrates decal projection onto 3D mesh surface using geometry clipping.
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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//--------------------------------------------------------------------------------------
// Defines and Macros
//--------------------------------------------------------------------------------------
#undef FLT_MAX
#define FLT_MAX 340282346638528859811704183484516925440.0f
#define MAX_DECALS 256

//--------------------------------------------------------------------------------------
// Types and Structures Definition
//--------------------------------------------------------------------------------------
typedef struct MeshBuilder {
    int vertexCount;
    int vertexCapacity;
    Vector3 *vertices;
    Vector2 *uvs;
} MeshBuilder;

typedef struct AppContext {
    Camera camera;
    Model model;
    Texture2D modelTexture;
    Model placementCube;
    Material decalMaterial;
    Texture2D decalTexture;
    BoundingBox modelBBox;
    Model decalModels[MAX_DECALS];
    int decalCount;
    float decalSize;
    float decalOffset;
    bool showModel;
} AppContext;

//--------------------------------------------------------------------------------------
// Global Variables Definition
//--------------------------------------------------------------------------------------
static AppContext g_App = { 0 };

//--------------------------------------------------------------------------------------
// Module Functions Declaration
//--------------------------------------------------------------------------------------
static void AddTriangleToMeshBuilder(MeshBuilder *mb, Vector3 v0, Vector3 v1, Vector3 v2);
static void FreeMeshBuilder(MeshBuilder *mb);
static Mesh BuildMesh(MeshBuilder *mb);
static Mesh GenMeshDecal(Model inputModel, Matrix projection, float decalSize, float decalOffset);
static Vector3 ClipSegment(Vector3 v0, Vector3 v1, Vector3 p, float s);
static void FreeDecalMeshData(void);
static bool GuiButton(Rectangle rec, const char *label);
void UpdateDrawFrame(void);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [models] example - decals");

    // Camera setup
    g_App.camera.position = (Vector3){ 0.0f, 2.5f, 5.0f };
    g_App.camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    g_App.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    g_App.camera.fovy = 45.0f;
    g_App.camera.projection = CAMERA_PERSPECTIVE;

    // Procedural model generation
    Mesh knotMesh = GenMeshTorus(0.8f, 1.8f, 32, 64);
    g_App.model = LoadModelFromMesh(knotMesh);

    // Procedural texture generation
    Image modelImg = GenImageChecked(512, 512, 32, 32, LIGHTGRAY, GRAY);
    g_App.modelTexture = LoadTextureFromImage(modelImg);
    UnloadImage(modelImg);
    SetTextureFilter(g_App.modelTexture, TEXTURE_FILTER_BILINEAR);
    g_App.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = g_App.modelTexture;

    // Bounding box setup
    g_App.modelBBox = GetMeshBoundingBox(g_App.model.meshes[0]);
    g_App.camera.target = Vector3Lerp(g_App.modelBBox.min, g_App.modelBBox.max, 0.5f);
    
    float modelSize = fminf(
        fminf(fabsf(g_App.modelBBox.max.x - g_App.modelBBox.min.x), fabsf(g_App.modelBBox.max.y - g_App.modelBBox.min.y)),
        fabsf(g_App.modelBBox.max.z - g_App.modelBBox.min.z));

    g_App.decalSize = modelSize * 0.35f;
    g_App.decalOffset = 0.01f;

    // Placement helper cube
    g_App.placementCube = LoadModelFromMesh(GenMeshCube(g_App.decalSize, g_App.decalSize, g_App.decalSize));
    g_App.placementCube.materials[0].maps[0].color = LIME;

    // Procedural decal texture generation
    Image decalImg = GenImageColor(128, 128, BLANK);
    ImageDrawCircle(&decalImg, 64, 64, 60, RED);
    ImageDrawCircle(&decalImg, 64, 64, 45, WHITE);
    ImageDrawCircle(&decalImg, 64, 64, 30, RED);
    ImageDrawCircle(&decalImg, 64, 64, 15, YELLOW);
    
    g_App.decalTexture = LoadTextureFromImage(decalImg);
    UnloadImage(decalImg);
    SetTextureFilter(g_App.decalTexture, TEXTURE_FILTER_BILINEAR);

    g_App.decalMaterial = LoadMaterialDefault();
    g_App.decalMaterial.maps[MATERIAL_MAP_DIFFUSE].texture = g_App.decalTexture;
    g_App.decalMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RAYWHITE;

    g_App.showModel = true;
    g_App.decalCount = 0;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(g_App.model);
    UnloadTexture(g_App.modelTexture);
    UnloadModel(g_App.placementCube);
    UnloadTexture(g_App.decalTexture);

    for (int i = 0; i < g_App.decalCount; i++) UnloadModel(g_App.decalModels[i]);

    FreeDecalMeshData();
    CloseWindow();
    //--------------------------------------------------------------------------------------
#endif

    return 0;
}

//------------------------------------------------------------------------------------
// Frame Update and Render Loop
//------------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    // Update
    //----------------------------------------------------------------------------------
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) UpdateCamera(&g_App.camera, CAMERA_THIRD_PERSON);

    RayCollision collision = { 0 };
    collision.distance = FLT_MAX;
    collision.hit = false;

    Ray ray = GetScreenToWorldRay(GetMousePosition(), g_App.camera);
    RayCollision boxHitInfo = GetRayCollisionBox(ray, g_App.modelBBox);

    if (boxHitInfo.hit && (g_App.decalCount < MAX_DECALS))
    {
        RayCollision meshHitInfo = { 0 };
        for (int m = 0; m < g_App.model.meshCount; m++)
        {
            meshHitInfo = GetRayCollisionMesh(ray, g_App.model.meshes[m], g_App.model.transform);
            if (meshHitInfo.hit)
            {
                if (!collision.hit || (collision.distance > meshHitInfo.distance)) collision = meshHitInfo;
            }
        }
    }

    if (collision.hit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (g_App.decalCount < MAX_DECALS))
    {
        Vector3 origin = Vector3Add(collision.point, Vector3Scale(collision.normal, 1.0f));
        Matrix splat = MatrixLookAt(collision.point, origin, (Vector3){ 0.0f, 1.0f, 0.0f });
        splat = MatrixMultiply(splat, MatrixRotateZ(DEG2RAD * ((float)GetRandomValue(-180, 180))));

        Mesh decalMesh = GenMeshDecal(g_App.model, splat, g_App.decalSize, g_App.decalOffset);

        if (decalMesh.vertexCount > 0)
        {
            int decalIndex = g_App.decalCount++;
            g_App.decalModels[decalIndex] = LoadModelFromMesh(decalMesh);
            g_App.decalModels[decalIndex].materials[0].maps[0] = g_App.decalMaterial.maps[0];
        }
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(g_App.camera);
            if (g_App.showModel) DrawModel(g_App.model, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);

            for (int i = 0; i < g_App.decalCount; i++) DrawModel(g_App.decalModels[i], (Vector3){ 0 }, 1.0f, WHITE);

            if (collision.hit)
            {
                Vector3 origin = Vector3Add(collision.point, Vector3Scale(collision.normal, 1.0f));
                Matrix splat = MatrixLookAt(collision.point, origin, (Vector3){ 0, 1, 0 });
                g_App.placementCube.transform = MatrixInvert(splat);
                DrawModel(g_App.placementCube, (Vector3){ 0 }, 1.0f, Fade(WHITE, 0.5f));
            }

            DrawGrid(10, 1.0f);
        EndMode3D();

        // Stats UI Display
        float yPos = 10;
        float x0 = screenWidth - 280.0f;
        float x1 = x0 + 90;
        float x2 = x1 + 90;

        DrawText("Vertices", (int)x1, (int)yPos, 10, LIME);
        DrawText("Triangles", (int)x2, (int)yPos, 10, LIME);
        yPos += 15;

        int vertexCount = 0;
        int triangleCount = 0;

        for (int i = 0; i < g_App.model.meshCount; i++)
        {
            vertexCount += g_App.model.meshes[i].vertexCount;
            triangleCount += g_App.model.meshes[i].triangleCount;
        }

        DrawText("Base Model", (int)x0, (int)yPos, 10, LIME);
        DrawText(TextFormat("%d", vertexCount), (int)x1, (int)yPos, 10, LIME);
        DrawText(TextFormat("%d", triangleCount), (int)x2, (int)yPos, 10, LIME);
        yPos += 15;

        for (int i = 0; i < g_App.decalCount; i++)
        {
            vertexCount += g_App.decalModels[i].meshes[0].vertexCount;
            triangleCount += g_App.decalModels[i].meshes[0].triangleCount;
        }

        DrawText("TOTAL", (int)x0, (int)yPos, 10, LIME);
        DrawText(TextFormat("%d", vertexCount), (int)x1, (int)yPos, 10, LIME);
        DrawText(TextFormat("%d", triangleCount), (int)x2, (int)yPos, 10, LIME);

        DrawText("Left Click: Place Decal | Hold Right Click: Rotate Camera", 10, screenHeight - 25, 10, DARKGRAY);

        // GUI controls
        if (GuiButton((Rectangle){ 10, (float)screenHeight - 80.0f, 100, 40 }, g_App.showModel ? "Hide Model" : "Show Model")) 
        {
            g_App.showModel = !g_App.showModel;
        }

        if (GuiButton((Rectangle){ 120, (float)screenHeight - 80.0f, 100, 40 }, "Clear Decals"))
        {
            for (int i = 0; i < g_App.decalCount; i++) UnloadModel(g_App.decalModels[i]);
            g_App.decalCount = 0;
        }

        DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}

//--------------------------------------------------------------------------------------
// Module Functions Definitions
//--------------------------------------------------------------------------------------

// Add a triangle definition to mesh builder buffer
static void AddTriangleToMeshBuilder(MeshBuilder *mb, Vector3 v0, Vector3 v1, Vector3 v2)
{
    if (mb->vertexCapacity <= (mb->vertexCount + 3))
    {
        int newVertexCapacity = (1 + (mb->vertexCapacity / 256)) * 256;
        Vector3 *newVertices = (Vector3 *)MemAlloc(newVertexCapacity * sizeof(Vector3));

        if (mb->vertexCapacity > 0)
        {
            memcpy(newVertices, mb->vertices, mb->vertexCount * sizeof(Vector3));
            MemFree(mb->vertices);
        }

        mb->vertices = newVertices;
        mb->vertexCapacity = newVertexCapacity;
    }

    int index = mb->vertexCount;
    mb->vertexCount += 3;

    mb->vertices[index + 0] = v0;
    mb->vertices[index + 1] = v1;
    mb->vertices[index + 2] = v2;
}

// Free mesh builder memory buffer
static void FreeMeshBuilder(MeshBuilder *mb)
{
    if (mb->vertices) MemFree(mb->vertices);
    if (mb->uvs) MemFree(mb->uvs);
    *mb = (MeshBuilder){ 0 };
}

// Build standard raylib Mesh from builder object
static Mesh BuildMesh(MeshBuilder *mb)
{
    Mesh outMesh = { 0 };

    outMesh.vertexCount = mb->vertexCount;
    outMesh.triangleCount = mb->vertexCount / 3;
    outMesh.vertices = (float *)MemAlloc(outMesh.vertexCount * 3 * sizeof(float));
    if (mb->uvs) outMesh.texcoords = (float *)MemAlloc(outMesh.vertexCount * 2 * sizeof(float));

    for (int i = 0; i < mb->vertexCount; i++)
    {
        outMesh.vertices[3 * i + 0] = mb->vertices[i].x;
        outMesh.vertices[3 * i + 1] = mb->vertices[i].y;
        outMesh.vertices[3 * i + 2] = mb->vertices[i].z;

        if (mb->uvs)
        {
            outMesh.texcoords[2 * i + 0] = mb->uvs[i].x;
            outMesh.texcoords[2 * i + 1] = mb->uvs[i].y;
        }
    }

    UploadMesh(&outMesh, false);
    return outMesh;
}

// Helper to clean up static internal mesh builder allocations
static void FreeDecalMeshData(void) 
{ 
    Model dummy = { 0 }; 
    dummy.meshCount = -1; 
    Matrix m = { 0 }; 
    GenMeshDecal(dummy, m, 0.0f, 0.0f); 
}

// Clip line segment against clipping plane
static Vector3 ClipSegment(Vector3 v0, Vector3 v1, Vector3 p, float s)
{
    float d0 = Vector3DotProduct(v0, p) - s;
    float d1 = Vector3DotProduct(v1, p) - s;
    float s0 = d0 / (d0 - d1);

    return Vector3Lerp(v0, v1, s0);
}

// Generate a clipped decal mesh mapped over target model using projection matrix
static Mesh GenMeshDecal(Model target, Matrix projection, float decalSize, float decalOffset)
{
    static MeshBuilder meshBuilders[2] = { 0 };

    if (target.meshCount == -1)
    {
        FreeMeshBuilder(&meshBuilders[0]);
        FreeMeshBuilder(&meshBuilders[1]);
        return (Mesh){ 0 };
    }

    Matrix invProj = MatrixInvert(projection);

    meshBuilders[0].vertexCount = 0;
    meshBuilders[1].vertexCount = 0;

    int mbIndex = 0;

    for (int meshIndex = 0; meshIndex < target.meshCount; meshIndex++)
    {
        Mesh mesh = target.meshes[meshIndex];
        for (int tri = 0; tri < mesh.triangleCount; tri++)
        {
            Vector3 vertices[3] = { 0 };

            if (mesh.indices == NULL)
            {
                for (int v = 0; v < 3; v++)
                {
                    vertices[v] = (Vector3){
                        mesh.vertices[3 * 3 * tri + 3 * v + 0],
                        mesh.vertices[3 * 3 * tri + 3 * v + 1],
                        mesh.vertices[3 * 3 * tri + 3 * v + 2]
                    };
                }
            }
            else
            {
                for (int v = 0; v < 3; v++)
                {
                    int idx = mesh.indices[3 * tri + v];
                    vertices[v] = (Vector3){
                        mesh.vertices[3 * idx + 0],
                        mesh.vertices[3 * idx + 1],
                        mesh.vertices[3 * idx + 2]
                    };
                }
            }

            int insideCount = 0;
            for (int i = 0; i < 3; i++)
            {
                Vector3 v = Vector3Transform(vertices[i], projection);
                if ((fabsf(v.x) < decalSize) || (fabsf(v.y) <= decalSize) || (fabsf(v.z) <= decalSize)) insideCount++;
                vertices[i] = v;
            }

            if (insideCount > 0) AddTriangleToMeshBuilder(&meshBuilders[mbIndex], vertices[0], vertices[1], vertices[2]);
        }
    }

    Vector3 planes[6] = {
        {  1,  0,  0 }, { -1,  0,  0 },
        {  0,  1,  0 }, {  0, -1,  0 },
        {  0,  0,  1 }, {  0,  0, -1 }
    };

    for (int face = 0; face < 6; face++)
    {
        mbIndex = 1 - mbIndex;

        MeshBuilder *inMesh = &meshBuilders[1 - mbIndex];
        MeshBuilder *outMesh = &meshBuilders[mbIndex];

        outMesh->vertexCount = 0;
        float s = 0.5f * decalSize;

        for (int i = 0; i < inMesh->vertexCount; i += 3)
        {
            Vector3 nV1, nV2, nV3, nV4;

            float d1 = Vector3DotProduct(inMesh->vertices[i + 0], planes[face]) - s;
            float d2 = Vector3DotProduct(inMesh->vertices[i + 1], planes[face]) - s;
            float d3 = Vector3DotProduct(inMesh->vertices[i + 2], planes[face]) - s;

            int v1Out = (d1 > 0);
            int v2Out = (d2 > 0);
            int v3Out = (d3 > 0);

            int total = v1Out + v2Out + v3Out;

            switch (total)
            {
                case 0:
                    AddTriangleToMeshBuilder(outMesh, inMesh->vertices[i], inMesh->vertices[i + 1], inMesh->vertices[i + 2]);
                    break;
                case 1:
                    if (v1Out)
                    {
                        nV1 = inMesh->vertices[i + 1];
                        nV2 = inMesh->vertices[i + 2];
                        nV3 = ClipSegment(inMesh->vertices[i], nV1, planes[face], s);
                        nV4 = ClipSegment(inMesh->vertices[i], nV2, planes[face], s);
                    }

                    if (v2Out)
                    {
                        nV1 = inMesh->vertices[i];
                        nV2 = inMesh->vertices[i + 2];
                        nV3 = ClipSegment(inMesh->vertices[i + 1], nV1, planes[face], s);
                        nV4 = ClipSegment(inMesh->vertices[i + 1], nV2, planes[face], s);

                        AddTriangleToMeshBuilder(outMesh, nV3, nV2, nV1);
                        AddTriangleToMeshBuilder(outMesh, nV2, nV3, nV4);
                        break;
                    }

                    if (v3Out)
                    {
                        nV1 = inMesh->vertices[i];
                        nV2 = inMesh->vertices[i + 1];
                        nV3 = ClipSegment(inMesh->vertices[i + 2], nV1, planes[face], s);
                        nV4 = ClipSegment(inMesh->vertices[i + 2], nV2, planes[face], s);
                    }

                    AddTriangleToMeshBuilder(outMesh, nV1, nV2, nV3);
                    AddTriangleToMeshBuilder(outMesh, nV4, nV3, nV2);
                    break;
                case 2:
                    if (!v1Out)
                    {
                        nV1 = inMesh->vertices[i];
                        nV2 = ClipSegment(nV1, inMesh->vertices[i + 1], planes[face], s);
                        nV3 = ClipSegment(nV1, inMesh->vertices[i + 2], planes[face], s);
                        AddTriangleToMeshBuilder(outMesh, nV1, nV2, nV3);
                    }

                    if (!v2Out)
                    {
                        nV1 = inMesh->vertices[i + 1];
                        nV2 = ClipSegment(nV1, inMesh->vertices[i + 2], planes[face], s);
                        nV3 = ClipSegment(nV1, inMesh->vertices[i], planes[face], s);
                        AddTriangleToMeshBuilder(outMesh, nV1, nV2, nV3);
                    }

                    if (!v3Out)
                    {
                        nV1 = inMesh->vertices[i + 2];
                        nV2 = ClipSegment(nV1, inMesh->vertices[i], planes[face], s);
                        nV3 = ClipSegment(nV1, inMesh->vertices[i + 1], planes[face], s);
                        AddTriangleToMeshBuilder(outMesh, nV1, nV2, nV3);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    MeshBuilder *theMesh = &meshBuilders[mbIndex];

    if (theMesh->vertexCount > 0)
    {
        theMesh->uvs = (Vector2 *)MemAlloc(sizeof(Vector2) * theMesh->vertexCount);

        for (int i = 0; i < theMesh->vertexCount; i++)
        {
            theMesh->uvs[i].x = (theMesh->vertices[i].x / decalSize + 0.5f);
            theMesh->uvs[i].y = (theMesh->vertices[i].y / decalSize + 0.5f);

            theMesh->vertices[i].z -= decalOffset;
            theMesh->vertices[i] = Vector3Transform(theMesh->vertices[i], invProj);
        }

        return BuildMesh(theMesh);
    }
    
    return (Mesh){ 0 };
}

// Simple custom GUI button helper
static bool GuiButton(Rectangle rec, const char *label)
{
    Color bgColor = GRAY;
    bool pressed = false;

    if (CheckCollisionPointRec(GetMousePosition(), rec))
    {
        bgColor = LIGHTGRAY;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) pressed = true;
    }

    DrawRectangleRec(rec, bgColor);
    DrawRectangleLinesEx(rec, 2.0f, DARKGRAY);

    int fontSize = 10;
    int textWidth = MeasureText(label, fontSize);

    DrawText(label, (int)(rec.x + rec.width * 0.5f - textWidth * 0.5f), (int)(rec.y + rec.height * 0.5f - fontSize * 0.5f), fontSize, DARKGRAY);

    return pressed;
}