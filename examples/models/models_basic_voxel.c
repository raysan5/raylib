/*******************************************************************************************
*
*   raylib [models] example - basic voxel
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Tim Little (@timlittle) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Tim Little (@timlittle)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#define WORLD_SIZE 8   // Size of our voxel world (8x8x8 cubes)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - basic voxel");

    DisableCursor();                    // Lock mouse to window center

    // Define the camera to look into our 3d world (first person)
    Camera3D camera = { 0 };
    camera.position = (Vector3){ -2.0f, 0.0f, -2.0f };  // Camera position at ground level
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector
    camera.fovy = 45.0f;                                 // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;              // Camera projection type

    // Create a cube model
    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);      // Create a unit cube mesh
    Model cubeModel = LoadModelFromMesh(cubeMesh);       // Convert mesh to a model
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BEIGE;

    // Initialize voxel world - fill with voxels
    bool voxels[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE] = { false };
    for (int x = 0; x < WORLD_SIZE; x++)
    {
        for (int y = 0; y < WORLD_SIZE; y++)
        {
            for (int z = 0; z < WORLD_SIZE; z++)
            {
                voxels[x][y][z] = true;
            }
        }
    }

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);

        // Handle voxel removal with mouse click
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // Cast a ray from the screen center (where crosshair would be)
            Vector2 screenCenter = { screenWidth/2.0f, screenHeight/2.0f };
            Ray ray = GetMouseRay(screenCenter, camera);

            // Check ray collision with all voxels
            bool voxelRemoved = false;
            for (int x = 0; (x < WORLD_SIZE) && !voxelRemoved; x++)
            {
                for (int y = 0; (y < WORLD_SIZE) && !voxelRemoved; y++)
                {
                    for (int z = 0; (z < WORLD_SIZE) && !voxelRemoved; z++)
                    {
                        if (!voxels[x][y][z]) continue; // Skip empty voxels

                        // Build a bounding box for this voxel
                        Vector3 position = { (float)x, (float)y, (float)z };
                        BoundingBox box = {
                            (Vector3){ position.x - 0.5f, position.y - 0.5f, position.z - 0.5f },
                            (Vector3){ position.x + 0.5f, position.y + 0.5f, position.z + 0.5f }
                        };

                        // Check ray-box collision
                        RayCollision collision = GetRayCollisionBox(ray, box);
                        if (collision.hit)
                        {
                            voxels[x][y][z] = false;    // Remove this voxel
                            voxelRemoved = true;        // Exit all loops
                        }
                    }
                }
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawGrid(10, 1.0f);

                // Draw all voxels
                for (int x = 0; x < WORLD_SIZE; x++)
                {
                    for (int y = 0; y < WORLD_SIZE; y++)
                    {
                        for (int z = 0; z < WORLD_SIZE; z++)
                        {
                            if (!voxels[x][y][z]) continue;

                            Vector3 position = { (float)x, (float)y, (float)z };
                            DrawModel(cubeModel, position, 1.0f, BEIGE);
                            DrawCubeWires(position, 1.0f, 1.0f, 1.0f, BLACK);
                        }
                    }
                }

            EndMode3D();

            DrawText("Left-click a voxel to remove it!", 10, 10, 20, DARKGRAY);
            DrawText("WASD to move, mouse to look around", 10, 35, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(cubeModel);
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
