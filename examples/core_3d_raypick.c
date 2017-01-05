/*******************************************************************************************
*
*   raylib [core] example - Ray-Picking in 3d mode, ground plane, triangle, mesh
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*   Example contributed by Joel Davis (@joeld42)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#include <stdio.h>
#include <float.h>


int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d ray picking");

    // Define the camera to look into our 3d world
    Camera camera;
    camera.position = (Vector3){ 10.0f, 8.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 2.3f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.6f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y

    Vector3 cubePosition = { 0.0f, 1.0f, 0.0f };
    Vector3 cubeSize = { 2.0f, 2.0f, 2.0f };
    
    Ray ray;        // Picking line ray
    
    Model tower = LoadModel("resources/model/lowpoly-tower.obj");           // Load OBJ model
    Texture2D texture = LoadTexture("resources/model/lowpoly-tower.png");   // Load model texture
    tower.material.texDiffuse = texture;                                    // Set model diffuse texture
    Vector3 towerPos = { 0.0f, 0.0f, 0.0f };                                // Set model position
    BoundingBox towerBBox = CalculateBoundingBox( tower.mesh );    
    bool hitMeshBBox;
    bool hitTriangle;

    // Test triangle
    Vector3 ta = (Vector3){ -25.0, 0.5, 0.0 };    
    Vector3 tb = (Vector3){ -4.0, 2.5, 1.0 };
    Vector3 tc = (Vector3){ -8.0, 6.5, 0.0 };

    Vector3 bary = {0};

    SetCameraMode(camera, CAMERA_FREE); // Set a free camera mode

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);          // Update camera
        

        // Display information about closest hit
        RayHitInfo nearestHit;
        char *hitObjectName = "None";
        nearestHit.distance = FLT_MAX;
        nearestHit.hit = false;
        Color cursorColor = WHITE;

        // Get ray and test against ground, triangle, and mesh
        ray = GetMouseRay(GetMousePosition(), camera);
        
        RayHitInfo groundHitInfo = RaycastGroundPlane( ray, 0.0 );
        if ((groundHitInfo.hit) && (groundHitInfo.distance < nearestHit.distance)) {
            nearestHit = groundHitInfo;
            cursorColor = GREEN;
            hitObjectName = "Ground";
        }

        RayHitInfo triHitInfo = RaycastTriangle( ray, ta, tb, tc );
        if ((triHitInfo.hit) && (triHitInfo.distance < nearestHit.distance)) {
            nearestHit = triHitInfo;
            cursorColor = PURPLE;
            hitObjectName = "Triangle";

            bary = Barycentric( nearestHit.hitPosition, ta, tb, tc );
            hitTriangle = true;
        } else {
            hitTriangle = false;
        }

        RayHitInfo meshHitInfo;

        // check the bounding box first, before trying the full ray/mesh test
        if (CheckCollisionRayBox( ray, towerBBox )) {
            hitMeshBBox = true;
            meshHitInfo = RaycastMesh( ray, &tower.mesh );    
            if ((meshHitInfo.hit) && (meshHitInfo.distance < nearestHit.distance)) {
                nearestHit = meshHitInfo;
                cursorColor = ORANGE;
                hitObjectName = "Mesh";
            }
        } else {
            hitMeshBBox = false;
        }    

        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                // Draw the tower
                DrawModel( tower, towerPos, 1.0, WHITE );
                
                // Draw the test triangle
                DrawLine3D( ta, tb, PURPLE );
                DrawLine3D( tb, tc, PURPLE );
                DrawLine3D( tc, ta, PURPLE );

                // Draw the mesh bbox if we hit it
                if (hitMeshBBox) {
                    DrawBoundingBox( towerBBox, LIME );
                }

                // If we hit something, draw the cursor at the hit point
                if (nearestHit.hit) {
                    DrawCube( nearestHit.hitPosition, 0.5, 0.5, 0.5, cursorColor );
                    DrawCubeWires( nearestHit.hitPosition, 0.5, 0.5, 0.5, YELLOW );

                    Vector3 normalEnd;
                    normalEnd.x = nearestHit.hitPosition.x + nearestHit.hitNormal.x;
                    normalEnd.y = nearestHit.hitPosition.y + nearestHit.hitNormal.y;
                    normalEnd.z = nearestHit.hitPosition.z + nearestHit.hitNormal.z;
                    DrawLine3D( nearestHit.hitPosition, normalEnd, YELLOW );
                }

                DrawRay(ray, MAROON);
                
                DrawGrid(10, 1.0f);

            End3dMode();
            
            // Show some debug text
            char line[1024];
            sprintf( line, "Hit Object: %s\n", hitObjectName );
            DrawText( line, 10, 30, 15, BLACK );

            if (nearestHit.hit) {
                int ypos = 45;
                sprintf( line, "Distance: %3.2f", nearestHit.distance );
                DrawText( line, 10, ypos, 15, BLACK );
                ypos += 15;

                sprintf( line, "Hit Pos: %3.2f %3.2f %3.2f", 
                    nearestHit.hitPosition.x, nearestHit.hitPosition.y, nearestHit.hitPosition.z );
                DrawText( line, 10, ypos, 15, BLACK );
                ypos += 15;

                sprintf( line, "Hit Norm: %3.2f %3.2f %3.2f", 
                    nearestHit.hitNormal.x, nearestHit.hitNormal.y, nearestHit.hitNormal.z );
                DrawText( line, 10, ypos, 15, BLACK );
                ypos += 15;

                if (hitTriangle) {
                    sprintf( line, "Barycentric: %3.2f %3.2f %3.2f", 
                        bary.x, bary.y, bary.z );
                    DrawText( line, 10, ypos, 15, BLACK );
                }
            }

            DrawText( "Use Mouse to Move Camera", 10, 420, 15, LIGHTGRAY );

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}