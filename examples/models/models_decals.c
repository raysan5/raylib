/*******************************************************************************************
*
*   raylib [models] example - decals
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.6-dev
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

#include <string.h>

#undef FLT_MAX
#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111

#define MAX_DECALS 256

typedef struct MeshBuilder {
    int vertexCount;
    int vertexCapacity;
    Vector3 *vertices;
    Vector2 *uvs;
} MeshBuilder;

// Utility functions
void AddTriangleToMeshBuilder(MeshBuilder *mb, Vector3 vertices[3]);
void FreeMeshBuilder(MeshBuilder *mb);
Mesh BuildMesh(MeshBuilder *mb);

Vector3 ClipSegment(Vector3 v0, Vector3 v1, Vector3 p, float s);

Mesh GenMeshDecal(Mesh inputMesh, Ray ray);

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
    Texture2D modelTexture = LoadTexture("resources/models/obj/character_diffuse.png"); // Load model texture
    SetTextureFilter(modelTexture, TEXTURE_FILTER_BILINEAR);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = modelTexture;            // Set model diffuse texture
    

    BoundingBox modelBBox = GetMeshBoundingBox(model.meshes[0]);    // Get mesh bounding box
    
    camera.target  = Vector3Lerp(modelBBox.min, modelBBox.max, 0.5f);
    camera.position = Vector3Scale(modelBBox.max, 1.0f);
    camera.position.x *= 0.1f;
    
    float modelSize = fminf(
        fminf(fabsf(modelBBox.max.x - modelBBox.min.x),
              fabsf(modelBBox.max.y - modelBBox.min.y)),
        fabsf(modelBBox.max.z - modelBBox.min.z)
    );
    
    camera.position = (Vector3){ 0, modelBBox.max.y * 1.2f, modelSize * 3.0f };
    
    float decalSize = modelSize * .25f;
    float decalOffset = 0.01f;
    
    Model placementCube = LoadModelFromMesh(GenMeshCube(decalSize, decalSize, decalSize));
    placementCube.materials[0].maps[0].color = LIME;
    
    Material decalMaterial = LoadMaterialDefault();
    decalMaterial.maps[0].color = YELLOW;
    
    Image decalImage = LoadImage("resources/raylib_logo.png");
    ImageResizeNN(&decalImage, decalImage.width / 4, decalImage.height / 4);
    Texture decalTexture = LoadTextureFromImage(decalImage);
    UnloadImage(decalImage);
    
    SetTextureFilter(decalTexture, TEXTURE_FILTER_BILINEAR);
    decalMaterial.maps[MATERIAL_MAP_DIFFUSE].texture = decalTexture;
    decalMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RAYWHITE;
    
    // We're going to use these to build up our decal meshes
    // They'll resize automatically as we go, we'll free them at the end
    MeshBuilder mesh_builders[2] = { {0}, {0} };
    
    bool showModel = true;
    Model decalModels[MAX_DECALS] = {0};
    int decalCount = 0;
    

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        }
        
        if (IsKeyPressed(KEY_SPACE)) {
            showModel = !showModel;
        }

        // Display information about closest hit
        RayCollision collision = { 0 };
        collision.distance = FLT_MAX;
        collision.hit = false;

        // Get mouse ray
        Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);

        // Check ray collision against bounding box first, before trying the full ray-mesh test
        RayCollision boxHitInfo = GetRayCollisionBox(ray, modelBBox);

        if ((boxHitInfo.hit) && decalCount < MAX_DECALS)
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
                    if ((!collision.hit) || (collision.distance > meshHitInfo.distance)) collision = meshHitInfo;
                }
            }

            if (meshHitInfo.hit)
            {
                collision = meshHitInfo;
            }
        }
        
        // --------------
        
        if (collision.hit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && decalCount < MAX_DECALS) {
            
            // Create the transformation to project the decal
            Vector3 origin = Vector3Add(collision.point, Vector3Scale(collision.normal, 1.0f));
            Matrix splat = MatrixLookAt(collision.point, origin, (Vector3){0,1,0});
            
            // Spin the placement around a bit
            splat = MatrixMultiply(splat, MatrixRotateZ(DEG2RAD * ((float)GetRandomValue(-180, 180))));
            
            Matrix splatInv = MatrixInvert(splat);
            
            // Reset the mesh builders
            mesh_builders[0].vertexCount = 0;
            mesh_builders[1].vertexCount = 0;
            
            // We'll be flip-flopping between the two mesh builders
            // Reading from one and writing to the other, then swapping
            int mb_index = 0;
            
            // First pass, just get any triangle inside the bounding box
            // (for each mesh of the model)
            for (int meshIndex = 0; meshIndex < model.meshCount; meshIndex++) {
                Mesh mesh = model.meshes[meshIndex];
                for (int tri = 0; tri < mesh.triangleCount; tri++) {
                    Vector3 vertices[3];
                    
                    // The way we calculate the vertices of the mesh triangle
                    // depend on whether the mesh vertices are indexed or not
                    if (mesh.indices == 0) {
                        for (int v = 0; v < 3; v++) {
                            vertices[v] = (Vector3) {
                                mesh.vertices[3*3*tri + 3*v + 0],
                                mesh.vertices[3*3*tri + 3*v + 1],
                                mesh.vertices[3*3*tri + 3*v + 2]
                            };
                        }
                    }
                    else {
                        for (int v = 0; v < 3; v++) {
                            vertices[v] = (Vector3) {
                                mesh.vertices[ 3*mesh.indices[3*tri+0] + v],
                                mesh.vertices[ 3*mesh.indices[3*tri+1] + v],
                                mesh.vertices[ 3*mesh.indices[3*tri+2] + v]
                            };
                        }
                    }
                    
                    // Transform all 3 vertices of the triangle
                    // and check if they are inside our decal box
                    int insideCount = 0;
                    for (int i = 0; i < 3; i++) {
                        // To splat space
                        Vector3 v = Vector3Transform(vertices[i], splat);
                        
                        if (fabsf(v.x)<decalSize || fabsf(v.y)<=decalSize || fabsf(v.z)<=decalSize) {
                            insideCount++;
                        }
                        
                        // We need to keep the transformed vertex
                        vertices[i] = v;
                    }
                    
                    // If any of them are inside, we add the triangle - we'll clip it later
                    if (insideCount > 0) {
                        AddTriangleToMeshBuilder(&mesh_builders[mb_index], vertices);
                    }
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
            for (int face = 0; face < 6; face++) {
                // Swap current model builder (so we read from the one we just wrote to)
                mb_index = 1 - mb_index;
                
                MeshBuilder *inMesh = &mesh_builders[1-mb_index];
                MeshBuilder *outMesh = &mesh_builders[mb_index];
                
                // Reset write builder
                outMesh->vertexCount = 0;
                
                float s = 0.5f * decalSize;
                
                for (int i = 0; i < inMesh->vertexCount; i+=3) {
                    Vector3 nV1, nV2, nV3, nV4;

                    float d1 = Vector3DotProduct(inMesh->vertices[ i + 0 ], planes[face] ) - s;
                    float d2 = Vector3DotProduct(inMesh->vertices[ i + 1 ], planes[face] ) - s;
                    float d3 = Vector3DotProduct(inMesh->vertices[ i + 2 ], planes[face] ) - s;

                    int v1Out = d1 > 0;
                    int v2Out = d2 > 0;
                    int v3Out = d3 > 0;

                    // calculate, how many vertices of the face lie outside of the clipping plane
                    int total = v1Out + v2Out + v3Out;

                    switch ( total ) {
                        case 0: {
                            // the entire face lies inside of the plane, no clipping needed
                            AddTriangleToMeshBuilder(outMesh, (Vector3[3]){inMesh->vertices[i], inMesh->vertices[i+1], inMesh->vertices[i+2]});
                            break;
                        }

                        case 1: {
                            // one vertex lies outside of the plane, perform clipping

                            if (v1Out) {
                                nV1 = inMesh->vertices[ i + 1 ];
                                nV2 = inMesh->vertices[ i + 2 ];
                                nV3 = ClipSegment( inMesh->vertices[ i ], nV1, planes[face], s );
                                nV4 = ClipSegment( inMesh->vertices[ i ], nV2, planes[face], s );
                            }

                            if (v2Out) {
                                nV1 = inMesh->vertices[ i ];
                                nV2 = inMesh->vertices[ i + 2 ];
                                nV3 = ClipSegment( inMesh->vertices[ i + 1 ], nV1, planes[face], s );
                                nV4 = ClipSegment( inMesh->vertices[ i + 1 ], nV2, planes[face], s );

                                AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV3, nV2, nV1});
                                AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV2, nV3, nV4});
                                break;
                            }

                            if (v3Out) {
                                nV1 = inMesh->vertices[ i ];
                                nV2 = inMesh->vertices[ i + 1 ];
                                nV3 = ClipSegment( inMesh->vertices[ i + 2 ], nV1, planes[face], s );
                                nV4 = ClipSegment( inMesh->vertices[ i + 2 ], nV2, planes[face], s );
                            }
                            
                            AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV1, nV2, nV3});
                            AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV4, nV3, nV2});

                            break;
                        }

                        case 2: {
                            // two vertices lies outside of the plane, perform clipping
                            if (!v1Out) {
                                nV1 = inMesh->vertices[ i ];
                                nV2 = ClipSegment( nV1, inMesh->vertices[ i + 1 ], planes[face], s );
                                nV3 = ClipSegment( nV1, inMesh->vertices[ i + 2 ], planes[face], s );
                                AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV1, nV2, nV3});
                            }
                            if (!v2Out) {
                                nV1 = inMesh->vertices[ i + 1 ];
                                nV2 = ClipSegment( nV1, inMesh->vertices[ i + 2 ], planes[face], s );
                                nV3 = ClipSegment( nV1, inMesh->vertices[ i ], planes[face], s );
                                AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV1, nV2, nV3});
                            }
                            if (!v3Out) {
                                nV1 = inMesh->vertices[ i + 2 ];
                                nV2 = ClipSegment( nV1, inMesh->vertices[ i ], planes[face], s );
                                nV3 = ClipSegment( nV1, inMesh->vertices[ i + 1 ], planes[face], s );
                                AddTriangleToMeshBuilder(outMesh, (Vector3[3]){nV1, nV2, nV3});
                            }
                            break;
                        }

                        case 3: {
                            // the entire face lies outside of the plane, so let's discard the corresponding vertices
                            break;
                        }

                    }
                }
            }
            
            // Now we just need to re-transform the vertices
            MeshBuilder *theMesh = &mesh_builders[mb_index];
            
            // Allocate room for UVs
            if (theMesh->vertexCount > 0) {
                theMesh->uvs = (Vector2*)MemAlloc(sizeof(Vector2)*theMesh->vertexCount);
                for (int i = 0; i < theMesh->vertexCount; i++) {
                    
                    // Calculate the UVs based on the projected coords
                    // They are clipped to (-decalSize .. decalSize) and we want them (0..1)
                    theMesh->uvs[i].x = (theMesh->vertices[i].x / decalSize + .5f);
                    theMesh->uvs[i].y = (theMesh->vertices[i].y / decalSize + .5f);
                    
                    // From splat space to world space
                    theMesh->vertices[i] = Vector3Transform(theMesh->vertices[i], splatInv);
                    
                    // Tiny nudge in the normal direction so it renders properly over the mesh
                    theMesh->vertices[i] = Vector3Add(theMesh->vertices[i], Vector3Scale(collision.normal, decalOffset));
                }
                
                // Decal model data ready, create it and add it
                int decalIndex = decalCount++;
                decalModels[decalIndex] = LoadModelFromMesh(BuildMesh(theMesh));
                decalModels[decalIndex].materials[0] = decalMaterial;
            }
            
        }
        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                // Draw the model at the origin and default scale
                if (showModel) {
                    DrawModel(model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
                }
                
                // Draw the decal models
                for (int i = 0; i < decalCount; i++) {
                    DrawModel(decalModels[i], (Vector3){0}, 1.0f, WHITE);
                }

                // If we hit the mesh, draw the box for the decal 
                if (collision.hit) {
                    
                    Vector3 origin = Vector3Add(collision.point, Vector3Scale(collision.normal, 1.0f));
                    Matrix splat = MatrixLookAt(collision.point, origin, (Vector3){0,1,0});
                    
                    placementCube.transform = MatrixInvert(splat);
                    DrawModel(placementCube, (Vector3){0}, 1.0f, Fade(WHITE, 0.5f));
                }

                DrawGrid(10, 10.0f);

            EndMode3D();
            
            {
                float yPos = 10;
                float x0 = GetScreenWidth() - 300;
                float x1 = x0 + 100;
                float x2 = x1 + 100;
                
                DrawText("Vertices", x1, yPos, 10, LIME);
                DrawText("Triangles", x2, yPos, 10, LIME);
                yPos += 15;
                
                int vertexCount = 0;
                int triangleCount = 0;
                
                for (int i = 0; i < model.meshCount; i++) {
                    vertexCount += model.meshes[i].vertexCount;
                    triangleCount += model.meshes[i].triangleCount;
                }
                
                DrawText("Main model", x0, yPos, 10, LIME);
                DrawText(TextFormat("%d", vertexCount), x1, yPos, 10, LIME);
                DrawText(TextFormat("%d", triangleCount), x2, yPos, 10, LIME);
                yPos += 15;
                
                for (int i = 0; i < decalCount; i++) {
                    DrawText(TextFormat("Decal #%d", i+1), x0, yPos, 10, LIME);
                    DrawText(TextFormat("%d", decalModels[i].meshes[0].vertexCount), x1, yPos, 10, LIME);
                    DrawText(TextFormat("%d", decalModels[i].meshes[0].triangleCount), x2, yPos, 10, LIME);
                    
                    vertexCount += decalModels[i].meshes[0].vertexCount;
                    triangleCount += decalModels[i].meshes[0].triangleCount;
                    yPos += 15;
                }
                
                DrawText("TOTAL", x0, yPos, 10, LIME);
                DrawText(TextFormat("%d", vertexCount), x1, yPos, 10, LIME);
                DrawText(TextFormat("%d", triangleCount), x2, yPos, 10, LIME);
                yPos += 15;
            }

            DrawText("Hold RMB to move camera", 10, 430, 10, GRAY);

            DrawText("(c) Character model and texture from kenney.nl", screenWidth - 260, screenHeight - 20, 10, GRAY);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);
    UnloadTexture(modelTexture);
    
    for (int i = 0; i < decalCount; i++) {
        UnloadModel(decalModels[i]);
    }
    
    UnloadTexture(decalTexture);
    
    FreeMeshBuilder(&mesh_builders[0]);
    FreeMeshBuilder(&mesh_builders[1]);

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// A really simple dynamic array manager
void AddTriangleToMeshBuilder(MeshBuilder *mb, Vector3 vertices[3]) {
    // Reallocate and copy if we need to
    if (mb->vertexCapacity <= mb->vertexCount + 3 ) {
        int newVertexCapacity = (1 + (mb->vertexCapacity / 256)) * 256;
        Vector3 *newVertices = (Vector3*)MemAlloc(newVertexCapacity * sizeof(Vector3));
        if (mb->vertexCapacity > 0) {
            memcpy(newVertices, mb->vertices, mb->vertexCount * sizeof(Vector3));
            MemFree(mb->vertices);
        }
        mb->vertices = newVertices;
        mb->vertexCapacity = newVertexCapacity;
    }
    
    // Add 3 vertices
    int index = mb->vertexCount;
    mb->vertexCount += 3;
    
    for (int i = 0; i < 3; i++) {
        mb->vertices[index+i] = vertices[i];
    }
}

void FreeMeshBuilder(MeshBuilder *mb) {
    MemFree(mb->vertices);
    
    if (mb->uvs) {
        MemFree(mb->uvs);
    }
    
    *mb = (MeshBuilder){ 0 };
}

// Construct a Raylib Mesh from our MeshBuilder data
Mesh BuildMesh(MeshBuilder *mb) {
    Mesh outMesh = { 0 };
    
    outMesh.vertexCount = mb->vertexCount;
    outMesh.triangleCount = mb->vertexCount / 3;
    
    outMesh.vertices = MemAlloc(outMesh.vertexCount * 3 * sizeof(float));
    
    if (mb->uvs) {
        outMesh.texcoords = MemAlloc(outMesh.vertexCount * 2 * sizeof(float));
    }
    
    for (int i = 0; i < mb->vertexCount; i++) {
        outMesh.vertices[3*i+0] = mb->vertices[i].x;
        outMesh.vertices[3*i+1] = mb->vertices[i].y;
        outMesh.vertices[3*i+2] = mb->vertices[i].z;
        if (mb->uvs) {
            outMesh.texcoords[2*i+0] = mb->uvs[i].x;
            outMesh.texcoords[2*i+1] = mb->uvs[i].y;
        }
    }
    
    UploadMesh(&outMesh, false);
    
    return outMesh;
}

Vector3 ClipSegment(Vector3 v0, Vector3 v1, Vector3 p, float s) {
    float d0 = Vector3DotProduct(v0, p) - s;
    float d1 = Vector3DotProduct(v1, p) - s;

    float s0 = d0 / ( d0 - d1 );

    Vector3 position = Vector3Lerp(v0, v1, s0);

    return position;
}