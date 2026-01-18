/*******************************************************************************************
*
*   raylib [models] example - decals
*
*   Example complexity rating: [★★★★] 4/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by JP Mortiboys (@themushroompirates) and reviewed by Ramon Santamaria (@raysan5)
*   Based on previous work by @mrdoob
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 JP Mortiboys (@themushroompirates) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

#include <string.h>         // Required for: memcpy()

#undef FLT_MAX
#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111

#define MAX_DECALS 256

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct MeshBuilder {
    int vertexCount;
    int vertexCapacity;
    Vector3 *vertices;
    Vector2 *uvs;
} MeshBuilder;

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static void AddTriangleToMeshBuilder(MeshBuilder *mb, Vector3 vertices[3]);
static void FreeMeshBuilder(MeshBuilder *mb);
static Mesh BuildMesh(MeshBuilder *mb);
static Mesh GenMeshDecal(Model inputModel, Matrix projection, float decalSize, float decalOffset);
static Vector3 ClipSegment(Vector3 v0, Vector3 v1, Vector3 p, float s);
static void FreeDecalMeshData(void) { GenMeshDecal((Model){ .meshCount = -1 }, (Matrix){ 0 }, 0.0f, 0.0f); }
static bool GuiButton(Rectangle rec, const char *label);

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

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 5.0f, 5.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.6f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    // Load character model
    Model model = LoadModel("resources/models/obj/character.obj");

    // Apply character skin
    Texture2D modelTexture = LoadTexture("resources/models/obj/character_diffuse.png");
    SetTextureFilter(modelTexture, TEXTURE_FILTER_BILINEAR);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = modelTexture;

    BoundingBox modelBBox = GetMeshBoundingBox(model.meshes[0]);    // Get mesh bounding box

    camera.target  = Vector3Lerp(modelBBox.min, modelBBox.max, 0.5f);
    camera.position = Vector3Scale(modelBBox.max, 1.0f);
    camera.position.x *= 0.1f;

    float modelSize = fminf(
        fminf(fabsf(modelBBox.max.x - modelBBox.min.x), fabsf(modelBBox.max.y - modelBBox.min.y)),
        fabsf(modelBBox.max.z - modelBBox.min.z));

    camera.position = (Vector3){ 0.0f, modelBBox.max.y*1.2f, modelSize*3.0f };

    float decalSize = modelSize*0.25f;
    float decalOffset = 0.01f;

    Model placementCube = LoadModelFromMesh(GenMeshCube(decalSize, decalSize, decalSize));
    placementCube.materials[0].maps[0].color = LIME;

    Material decalMaterial = LoadMaterialDefault();
    decalMaterial.maps[0].color = YELLOW;

    Image decalImage = LoadImage("resources/raylib_logo.png");
    ImageResizeNN(&decalImage, decalImage.width/4, decalImage.height/4);
    Texture decalTexture = LoadTextureFromImage(decalImage);
    UnloadImage(decalImage);

    SetTextureFilter(decalTexture, TEXTURE_FILTER_BILINEAR);
    decalMaterial.maps[MATERIAL_MAP_DIFFUSE].texture = decalTexture;
    decalMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RAYWHITE;

    bool showModel = true;
    Model decalModels[MAX_DECALS] = { 0 };
    int decalCount = 0;

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) UpdateCamera(&camera, CAMERA_THIRD_PERSON);

        // Display information about closest hit
        RayCollision collision = { 0 };
        collision.distance = FLT_MAX;
        collision.hit = false;

        // Get mouse ray
        Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);

        // Check ray collision against bounding box first, before trying the full ray-mesh test
        RayCollision boxHitInfo = GetRayCollisionBox(ray, modelBBox);

        if ((boxHitInfo.hit) && (decalCount < MAX_DECALS))
        {
            // Check ray collision against model meshes
            RayCollision meshHitInfo = { 0 };
            for (int m = 0; m < model.meshCount; m++)
            {
                // NOTE: We consider the model.transform for the collision check but
                // it can be checked against any transform Matrix, used when checking against same
                // model drawn multiple times with multiple transforms
                meshHitInfo = GetRayCollisionMesh(ray, model.meshes[m], model.transform);
                if (meshHitInfo.hit)
                {
                    // Save the closest hit mesh
                    if (!collision.hit || (collision.distance > meshHitInfo.distance)) collision = meshHitInfo;
                }
            }

            if (meshHitInfo.hit) collision = meshHitInfo;
        }

        // Add decal to mesh on hit point
        if (collision.hit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (decalCount < MAX_DECALS))
        {
            // Create the transformation to project the decal
            Vector3 origin = Vector3Add(collision.point, Vector3Scale(collision.normal, 1.0f));
            Matrix splat = MatrixLookAt(collision.point, origin, (Vector3){ 0.0f, 1.0f, 0.0f });

            // Spin the placement around a bit
            splat = MatrixMultiply(splat, MatrixRotateZ(DEG2RAD*((float)GetRandomValue(-180, 180))));

            Mesh decalMesh = GenMeshDecal(model, splat, decalSize, decalOffset);

            if (decalMesh.vertexCount > 0)
            {
                int decalIndex = decalCount++;
                decalModels[decalIndex] = LoadModelFromMesh(decalMesh);
                decalModels[decalIndex].materials[0].maps[0] = decalMaterial.maps[0];
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                // Draw the model at the origin and default scale
                if (showModel) DrawModel(model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);

                // Draw the decal models
                for (int i = 0; i < decalCount; i++) DrawModel(decalModels[i], (Vector3){0}, 1.0f, WHITE);

                // If we hit the mesh, draw the box for the decal
                if (collision.hit)
                {
                    Vector3 origin = Vector3Add(collision.point, Vector3Scale(collision.normal, 1.0f));
                    Matrix splat = MatrixLookAt(collision.point, origin, (Vector3){0,1,0});
                    placementCube.transform = MatrixInvert(splat);
                    DrawModel(placementCube, (Vector3){0}, 1.0f, Fade(WHITE, 0.5f));
                }

                DrawGrid(10, 10.0f);
            EndMode3D();

            float yPos = 10;
            float x0 = GetScreenWidth() - 300.0f;
            float x1 = x0 + 100;
            float x2 = x1 + 100;

            DrawText("Vertices", (int)x1, (int)yPos, 10, LIME);
            DrawText("Triangles", (int)x2, (int)yPos, 10, LIME);
            yPos += 15;

            int vertexCount = 0;
            int triangleCount = 0;

            for (int i = 0; i < model.meshCount; i++)
            {
                vertexCount += model.meshes[i].vertexCount;
                triangleCount += model.meshes[i].triangleCount;
            }

            DrawText("Main model", (int)x0, (int)yPos, 10, LIME);
            DrawText(TextFormat("%d", vertexCount), (int)x1, (int)yPos, 10, LIME);
            DrawText(TextFormat("%d", triangleCount), (int)x2, (int)yPos, 10, LIME);
            yPos += 15;

            for (int i = 0; i < decalCount; i++)
            {
                if (i == 20)
                {
                    DrawText("...", (int)x0, (int)yPos, 10, LIME);
                    yPos += 15;
                }

                if (i < 20)
                {
                    DrawText(TextFormat("Decal #%d", i+1), (int)x0, (int)yPos, 10, LIME);
                    DrawText(TextFormat("%d", decalModels[i].meshes[0].vertexCount), (int)x1, (int)yPos, 10, LIME);
                    DrawText(TextFormat("%d", decalModels[i].meshes[0].triangleCount), (int)x2, (int)yPos, 10, LIME);
                    yPos += 15;
                }

                vertexCount += decalModels[i].meshes[0].vertexCount;
                triangleCount += decalModels[i].meshes[0].triangleCount;
            }

            DrawText("TOTAL", (int)x0, (int)yPos, 10, LIME);
            DrawText(TextFormat("%d", vertexCount), (int)x1, (int)yPos, 10, LIME);
            DrawText(TextFormat("%d", triangleCount), (int)x2, (int)yPos, 10, LIME);
            yPos += 15;

            DrawText("Hold RMB to move camera", 10, 430, 10, GRAY);
            DrawText("(c) Character model and texture from kenney.nl", screenWidth - 260, screenHeight - 20, 10, GRAY);

            // UI elements
            if (GuiButton((Rectangle){ 10, screenHeight - 1000.f, 100, 60 }, showModel ? "Hide Model" : "Show Model")) showModel = !showModel;

            if (GuiButton((Rectangle){ 10 + 110, screenHeight - 100.0f, 100, 60 }, "Clear Decals"))
            {
                // Clear decals, unload all decal models
                for (int i = 0; i < decalCount; i++) UnloadModel(decalModels[i]);
                decalCount = 0;
            }

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);
    UnloadTexture(modelTexture);

    // Unload decal models
    for (int i = 0; i < decalCount; i++) UnloadModel(decalModels[i]);

    UnloadTexture(decalTexture);

    FreeDecalMeshData();        // Free the data for decal generation

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Add triangles to mesh builder (dynamic array manager)
static void AddTriangleToMeshBuilder(MeshBuilder *mb, Vector3 vertices[3])
{
    // Reallocate and copy if we need to
    if (mb->vertexCapacity <= (mb->vertexCount + 3))
    {
        int newVertexCapacity = (1 + (mb->vertexCapacity/256))*256;
        Vector3 *newVertices = (Vector3 *)MemAlloc(newVertexCapacity*sizeof(Vector3));

        if (mb->vertexCapacity > 0)
        {
            memcpy(newVertices, mb->vertices, mb->vertexCount*sizeof(Vector3));
            MemFree(mb->vertices);
        }

        mb->vertices = newVertices;
        mb->vertexCapacity = newVertexCapacity;
    }

    // Add 3 vertices
    int index = mb->vertexCount;
    mb->vertexCount += 3;

    for (int i = 0; i < 3; i++) mb->vertices[index+i] = vertices[i];
}

// Free mesh builder
static void FreeMeshBuilder(MeshBuilder *mb)
{
    MemFree(mb->vertices);
    if (mb->uvs) MemFree(mb->uvs);
    *mb = (MeshBuilder){ 0 };
}

// Build a Mesh from MeshBuilder data
static Mesh BuildMesh(MeshBuilder *mb)
{
    Mesh outMesh = { 0 };

    outMesh.vertexCount = mb->vertexCount;
    outMesh.triangleCount = mb->vertexCount/3;
    outMesh.vertices = MemAlloc(outMesh.vertexCount*3*sizeof(float));
    if (mb->uvs) outMesh.texcoords = MemAlloc(outMesh.vertexCount*2*sizeof(float));

    for (int i = 0; i < mb->vertexCount; i++)
    {
        outMesh.vertices[3*i+0] = mb->vertices[i].x;
        outMesh.vertices[3*i+1] = mb->vertices[i].y;
        outMesh.vertices[3*i+2] = mb->vertices[i].z;

        if (mb->uvs)
        {
            outMesh.texcoords[2*i+0] = mb->uvs[i].x;
            outMesh.texcoords[2*i+1] = mb->uvs[i].y;
        }
    }

    UploadMesh(&outMesh, false);

    return outMesh;
}

// Clip segment
static Vector3 ClipSegment(Vector3 v0, Vector3 v1, Vector3 p, float s)
{
    float d0 = Vector3DotProduct(v0, p) - s;
    float d1 = Vector3DotProduct(v1, p) - s;
    float s0 = d0/(d0 - d1);

    Vector3 position = Vector3Lerp(v0, v1, s0);

    return position;
}

// Generate mesh decals for provided model
static Mesh GenMeshDecal(Model target, Matrix projection, float decalSize, float decalOffset)
{
    // We're going to use these to build up our decal meshes
    // They'll resize automatically as we go, we'll free them at the end
    static MeshBuilder meshBuilders[2] = { 0 };

    // Ugly way of telling us to free the static MeshBuilder data
    if (target.meshCount == -1)
    {
        FreeMeshBuilder(&meshBuilders[0]);
        FreeMeshBuilder(&meshBuilders[1]);
        return (Mesh){ 0 };
    }

    // We're going to need the inverse matrix
    Matrix invProj = MatrixInvert(projection);

    // Reset the mesh builders
    meshBuilders[0].vertexCount = 0;
    meshBuilders[1].vertexCount = 0;

    // We'll be flip-flopping between the two mesh builders
    // Reading from one and writing to the other, then swapping
    int mbIndex = 0;

    // First pass, just get any triangle inside the bounding box (for each mesh of the model)
    for (int meshIndex = 0; meshIndex < target.meshCount; meshIndex++)
    {
        Mesh mesh = target.meshes[meshIndex];
        for (int tri = 0; tri < mesh.triangleCount; tri++)
        {
            Vector3 vertices[3] = { 0 };

            // The way we calculate the vertices of the mesh triangle
            // depend on whether the mesh vertices are indexed or not
            if (mesh.indices == 0)
            {
                for (int v = 0; v < 3; v++)
                {
                    vertices[v] = (Vector3){
                        mesh.vertices[3*3*tri + 3*v + 0],
                        mesh.vertices[3*3*tri + 3*v + 1],
                        mesh.vertices[3*3*tri + 3*v + 2]
                    };
                }
            }
            else
            {
                for (int v = 0; v < 3; v++)
                {
                    vertices[v] = (Vector3){
                        mesh.vertices[ 3*mesh.indices[3*tri+0] + v],
                        mesh.vertices[ 3*mesh.indices[3*tri+1] + v],
                        mesh.vertices[ 3*mesh.indices[3*tri+2] + v]
                    };
                }
            }

            // Transform all 3 vertices of the triangle
            // and check if they are inside our decal box
            int insideCount = 0;
            for (int i = 0; i < 3; i++)
            {
                // To projection space
                Vector3 v = Vector3Transform(vertices[i], projection);

                if ((fabsf(v.x) < decalSize) || (fabsf(v.y) <= decalSize) || (fabsf(v.z) <= decalSize)) insideCount++;

                // We need to keep the transformed vertex
                vertices[i] = v;
            }

            // If any of them are inside, we add the triangle - we'll clip it later
            if (insideCount > 0) AddTriangleToMeshBuilder(&meshBuilders[mbIndex], vertices);
        }
    }

    // Clipping time! We need to clip against all 6 directions
    Vector3 planes[6] = {
       {  1,  0,  0 },
       { -1,  0,  0 },
       {  0,  1,  0 },
       {  0, -1,  0 },
       {  0,  0,  1 },
       {  0,  0, -1 }
    };

    for (int face = 0; face < 6; face++)
    {
        // Swap current model builder (so we read from the one we just wrote to)
        mbIndex = 1 - mbIndex;

        MeshBuilder *inMesh = &meshBuilders[1 - mbIndex];
        MeshBuilder *outMesh = &meshBuilders[mbIndex];

        // Reset write builder
        outMesh->vertexCount = 0;

        float s = 0.5f*decalSize;

        for (int i = 0; i < inMesh->vertexCount; i += 3)
        {
            Vector3 nV1, nV2, nV3, nV4;

            float d1 = Vector3DotProduct(inMesh->vertices[ i + 0 ], planes[face] ) - s;
            float d2 = Vector3DotProduct(inMesh->vertices[ i + 1 ], planes[face] ) - s;
            float d3 = Vector3DotProduct(inMesh->vertices[ i + 2 ], planes[face] ) - s;

            int v1Out = (d1 > 0);
            int v2Out = (d2 > 0);
            int v3Out = (d3 > 0);

            // Calculate, how many vertices of the face lie outside of the clipping plane
            int total = v1Out + v2Out + v3Out;

            switch (total)
            {
                case 0:
                {
                    // The entire face lies inside of the plane, no clipping needed
                    AddTriangleToMeshBuilder(outMesh, (Vector3[3]){inMesh->vertices[i], inMesh->vertices[i+1], inMesh->vertices[i+2]});
                } break;
                case 1:
                {
                    // One vertex lies outside of the plane, perform clipping
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

                        AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV3, nV2, nV1});
                        AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV2, nV3, nV4});
                        break;
                    }

                    if (v3Out)
                    {
                        nV1 = inMesh->vertices[i];
                        nV2 = inMesh->vertices[i + 1];
                        nV3 = ClipSegment(inMesh->vertices[i + 2], nV1, planes[face], s);
                        nV4 = ClipSegment(inMesh->vertices[i + 2], nV2, planes[face], s);
                    }

                    AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV1, nV2, nV3});
                    AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV4, nV3, nV2});
                } break;
                case 2:
                {
                    // Two vertices lies outside of the plane, perform clipping
                    if (!v1Out)
                    {
                        nV1 = inMesh->vertices[i];
                        nV2 = ClipSegment(nV1, inMesh->vertices[i + 1], planes[face], s);
                        nV3 = ClipSegment(nV1, inMesh->vertices[i + 2], planes[face], s);
                        AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV1, nV2, nV3});
                    }

                    if (!v2Out)
                    {
                        nV1 = inMesh->vertices[i + 1];
                        nV2 = ClipSegment(nV1, inMesh->vertices[i + 2], planes[face], s);
                        nV3 = ClipSegment(nV1, inMesh->vertices[i], planes[face], s);
                        AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV1, nV2, nV3});
                    }

                    if (!v3Out)
                    {
                        nV1 = inMesh->vertices[i + 2];
                        nV2 = ClipSegment(nV1, inMesh->vertices[i], planes[face], s);
                        nV3 = ClipSegment(nV1, inMesh->vertices[i + 1], planes[face], s);
                        AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV1, nV2, nV3});
                    }
                } break;
                case 3: // The entire face lies outside of the plane, so let's discard the corresponding vertices
                default: break;
            }
        }
    }

    // Now we just need to re-transform the vertices
    MeshBuilder *theMesh = &meshBuilders[mbIndex];

    // Allocate room for UVs
    if (theMesh->vertexCount > 0)
    {
        theMesh->uvs = (Vector2 *)MemAlloc(sizeof(Vector2)*theMesh->vertexCount);

        for (int i = 0; i < theMesh->vertexCount; i++)
        {
            // Calculate the UVs based on the projected coords
            // They are clipped to (-decalSize .. decalSize) and we want them (0..1)
            theMesh->uvs[i].x = (theMesh->vertices[i].x/decalSize + 0.5f);
            theMesh->uvs[i].y = (theMesh->vertices[i].y/decalSize + 0.5f);

            // Tiny nudge in the normal direction so it renders properly over the mesh
            theMesh->vertices[i].z -= decalOffset;

            // From projection space to world space
            theMesh->vertices[i] = Vector3Transform(theMesh->vertices[i], invProj);
        }

        // Decal model data ready, create the mesh and return it
        return BuildMesh(theMesh);
    }
    else
    {
        // Return a blank mesh as there's nothing to add
        return (Mesh){ 0 };
    }
}

// Button UI element
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

    DrawText(label, (int)(rec.x + rec.width*0.5f - textWidth*0.5f), (int)(rec.y + rec.height*0.5f - fontSize*0.5f), fontSize, DARKGRAY);

    return pressed;
}
