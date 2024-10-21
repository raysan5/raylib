/*******************************************************************************************
*
*   raylib example - point rendering
*
*   Example originally created with raylib 5.0, last time updated with raylib 5.0
*
*   Example contributed by Reese Gallagher (@satchelfrost) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024 Reese Gallagher (@satchelfrost)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>             // Required for: rand()
#include <math.h>               // Required for: cos(), sin()

#define MAX_POINTS 10000000     // 10 million
#define MIN_POINTS 1000         // 1 thousand

// Generate mesh using points
Mesh GenMeshPoints(int numPoints);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    InitWindow(screenWidth, screenHeight, "raylib [models] example - point rendering");

    Camera camera = {
        .position   = { 3.0f, 3.0f, 3.0f },
        .target     = { 0.0f, 0.0f, 0.0f },
        .up         = { 0.0f, 1.0f, 0.0f },
        .fovy       = 45.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    Vector3 position = { 0.0f, 0.0f, 0.0f };
    bool useDrawModelPoints = true;
    bool numPointsChanged = false;
    int numPoints = 1000;
    
    Mesh mesh = GenMeshPoints(numPoints);
    Model model = LoadModelFromMesh(mesh);
    
    //SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while(!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);

        if (IsKeyPressed(KEY_SPACE)) useDrawModelPoints = !useDrawModelPoints;
        if (IsKeyPressed(KEY_UP))
        {
            numPoints = (numPoints*10 > MAX_POINTS)? MAX_POINTS : numPoints*10;
            numPointsChanged = true;
        }
        if (IsKeyPressed(KEY_DOWN))
        {
            numPoints = (numPoints/10 < MIN_POINTS)? MIN_POINTS : numPoints/10;
            numPointsChanged = true;
        }

        // Upload a different point cloud size
        if (numPointsChanged)
        {
            UnloadModel(model);
            mesh = GenMeshPoints(numPoints);
            model = LoadModelFromMesh(mesh);
            numPointsChanged = false;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode3D(camera);

                // The new method only uploads the points once to the GPU
                if (useDrawModelPoints)
                {
                    DrawModelPoints(model, position, 1.0f, WHITE);
                }
                else
                {
                    // The old method must continually draw the "points" (lines)
                    for (int i = 0; i < numPoints; i++)
                    {
                        Vector3 pos = {
                            .x = mesh.vertices[i*3 + 0],
                            .y = mesh.vertices[i*3 + 1],
                            .z = mesh.vertices[i*3 + 2],
                        };
                        Color color = {
                            .r = mesh.colors[i*4 + 0],
                            .g = mesh.colors[i*4 + 1],
                            .b = mesh.colors[i*4 + 2],
                            .a = mesh.colors[i*4 + 3],
                        };
                        
                        DrawPoint3D(pos, color);
                    }
                }

                // Draw a unit sphere for reference
                DrawSphereWires(position, 1.0f, 10, 10, YELLOW);
                
            EndMode3D();

            // Draw UI text
            DrawText(TextFormat("Point Count: %d", numPoints), 20, screenHeight - 50, 40, WHITE);
            DrawText("Up - increase points", 20, 70, 20, WHITE);
            DrawText("Down - decrease points", 20, 100, 20, WHITE);
            DrawText("Space - drawing function", 20, 130, 20, WHITE);
            
            if (useDrawModelPoints) DrawText("Using: DrawModelPoints()", 20, 160, 20, GREEN);
            else DrawText("Using: DrawPoint3D()", 20, 160, 20, RED);
            
            DrawFPS(10, 10);
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);

    CloseWindow();
    //--------------------------------------------------------------------------------------
    return 0;
}

// Generate a spherical point cloud
Mesh GenMeshPoints(int numPoints)
{
    Mesh mesh = { 
        .triangleCount = 1,
        .vertexCount = numPoints,
        .vertices = (float *)MemAlloc(numPoints*3*sizeof(float)),
        .colors = (unsigned char*)MemAlloc(numPoints*4*sizeof(unsigned char)),
    };

    // https://en.wikipedia.org/wiki/Spherical_coordinate_system
    for (int i = 0; i < numPoints; i++)
    {
        float theta = PI*rand()/RAND_MAX;
        float phi = 2.0f*PI*rand()/RAND_MAX;
        float r = 10.0f*rand()/RAND_MAX;
        
        mesh.vertices[i*3 + 0] = r*sin(theta)*cos(phi);
        mesh.vertices[i*3 + 1] = r*sin(theta)*sin(phi);
        mesh.vertices[i*3 + 2] = r*cos(theta);
        
        Color color = ColorFromHSV(r*360.0f, 1.0f, 1.0f);
        
        mesh.colors[i*4 + 0] = color.r;
        mesh.colors[i*4 + 1] = color.g;
        mesh.colors[i*4 + 2] = color.b;
        mesh.colors[i*4 + 3] = color.a;
    }

    // Upload mesh data from CPU (RAM) to GPU (VRAM) memory
    UploadMesh(&mesh, false);
    
    return mesh;
}
