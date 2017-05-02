/*******************************************************************************************
*
*   raylib [models] example - Mesh picking in 3d mode, ground plane, triangle, mesh
*
*   This example has been created using raylib 1.7 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*   Example contributed by Joel Davis (@joeld42)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#define FLT_MAX     3.40282347E+38F     // Maximum value of a float, defined in <float.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const int screenWidth = 800;
const int screenHeight = 450;

Camera camera;
    
Vector3 cubePosition = { 0.0f, 1.0f, 0.0f };
Vector3 cubeSize = { 2.0f, 2.0f, 2.0f };

Ray ray;        // Picking line ray

Model tower;
Texture2D texture;

Vector3 towerPos = { 0.0f, 0.0f, 0.0f };
BoundingBox towerBBox;
bool hitMeshBBox = false;
bool hitTriangle = false;

// Test triangle
Vector3 ta = (Vector3){ -25.0, 0.5, 0.0 };    
Vector3 tb = (Vector3){ -4.0, 2.5, 1.0 };
Vector3 tc = (Vector3){ -8.0, 6.5, 0.0 };

Vector3 bary = { 0.0f, 0.0f, 0.0f };

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void);     // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - 3d mesh picking");

    // Define the camera to look into our 3d world
    camera.position = (Vector3){ 10.0f, 8.0f, 10.0f };          // Camera position
    camera.target = (Vector3){ 0.0f, 2.3f, 0.0f };              // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.6f, 0.0f };                  // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                        // Camera field-of-view Y

    tower = LoadModel("resources/tower.obj");     // Load OBJ model
    texture = LoadTexture("resources/tower.png"); // Load model texture
    tower.material.texDiffuse = texture;                        // Set model diffuse texture
    
    towerBBox = CalculateBoundingBox(tower.mesh);

    SetCameraMode(camera, CAMERA_FREE);     // Set a free camera mode

    SetTargetFPS(60);                       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(tower);         // Unload model from GPU
    UnloadTexture(texture);     // Unload texture from GPU
    
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void)
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
    
    // Check ray collision aginst ground plane
    RayHitInfo groundHitInfo = GetCollisionRayGround(ray, 0.0f);
    
    if ((groundHitInfo.hit) && (groundHitInfo.distance < nearestHit.distance))
    {
        nearestHit = groundHitInfo;
        cursorColor = GREEN;
        hitObjectName = "Ground";
    }

    // Check ray collision against test triangle
    RayHitInfo triHitInfo = GetCollisionRayTriangle(ray, ta, tb, tc);
    
    if ((triHitInfo.hit) && (triHitInfo.distance < nearestHit.distance)) 
    {
        nearestHit = triHitInfo;
        cursorColor = PURPLE;
        hitObjectName = "Triangle";

        bary = VectorBarycenter(nearestHit.hitPosition, ta, tb, tc);
        hitTriangle = true;
    } 
    else hitTriangle = false;

    RayHitInfo meshHitInfo;

    // Check ray collision against bounding box first, before trying the full ray-mesh test
    if (CheckCollisionRayBox(ray, towerBBox)) 
    {
        hitMeshBBox = true;
        
        // Check ray collision against mesh
        meshHitInfo = GetCollisionRayMesh(ray, &tower.mesh);    
        
        if ((meshHitInfo.hit) && (meshHitInfo.distance < nearestHit.distance)) 
        {
            nearestHit = meshHitInfo;
            cursorColor = ORANGE;
            hitObjectName = "Mesh";
        }
        
    } hitMeshBBox = false; 
    //----------------------------------------------------------------------------------
    
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        Begin3dMode(camera);

            // Draw the tower
            DrawModel(tower, towerPos, 1.0, WHITE);
            
            // Draw the test triangle
            DrawLine3D(ta, tb, PURPLE);
            DrawLine3D(tb, tc, PURPLE);
            DrawLine3D(tc, ta, PURPLE);

            // Draw the mesh bbox if we hit it
            if (hitMeshBBox) DrawBoundingBox(towerBBox, LIME);

            // If we hit something, draw the cursor at the hit point
            if (nearestHit.hit) 
            {
                DrawCube(nearestHit.hitPosition, 0.5, 0.5, 0.5, cursorColor);
                DrawCubeWires(nearestHit.hitPosition, 0.5, 0.5, 0.5, YELLOW);

                Vector3 normalEnd;
                normalEnd.x = nearestHit.hitPosition.x + nearestHit.hitNormal.x;
                normalEnd.y = nearestHit.hitPosition.y + nearestHit.hitNormal.y;
                normalEnd.z = nearestHit.hitPosition.z + nearestHit.hitNormal.z;
                
                DrawLine3D(nearestHit.hitPosition, normalEnd, YELLOW);
            }

            DrawRay(ray, MAROON);
            
            DrawGrid(100, 1.0f);

        End3dMode();
        
        // Draw some debug GUI text
        DrawText(FormatText("Hit Object: %s", hitObjectName), 10, 50, 10, BLACK);

        if (nearestHit.hit) 
        {
            int ypos = 70;

            DrawText(FormatText("Distance: %3.2f", nearestHit.distance), 10, ypos, 10, BLACK);
            
            DrawText(FormatText("Hit Pos: %3.2f %3.2f %3.2f", 
                                nearestHit.hitPosition.x, 
                                nearestHit.hitPosition.y, 
                                nearestHit.hitPosition.z), 10, ypos + 15, 10, BLACK);
                                
            DrawText(FormatText("Hit Norm: %3.2f %3.2f %3.2f", 
                                nearestHit.hitNormal.x, 
                                nearestHit.hitNormal.y, 
                                nearestHit.hitNormal.z), 10, ypos + 30, 10, BLACK);

            if (hitTriangle) DrawText(FormatText("Barycenter: %3.2f %3.2f %3.2f",  bary.x, bary.y, bary.z), 10, ypos + 45, 10, BLACK);
        }

        DrawText("Use Mouse to Move Camera", 10, 430, 10, GRAY);

        DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
