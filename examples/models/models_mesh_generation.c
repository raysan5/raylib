/*******************************************************************************************
*
*   raylib example - procedural mesh generation
*
*   This example has been created using raylib 1.8 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2017 Ramon Santamaria (Ray San)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - mesh generation");
    
    //Model model = LoadModelFromMesh(GenMeshPlane(2, 2, 5, 5));            // Texture coordinates must be divided by resX, resZ
    //Model model = LoadModelFromMesh(GenMeshCube(2.0f, 1.0f, 2.0f));       // OK!
    //Model model = LoadModelFromMesh(GenMeshSphere(2, 32, 32));              // OK! (par_shapes)
    //Model model = LoadModelFromMesh(GenMeshHemiSphere(2, 16, 16));              // OK! (par_shapes)
    //Model model = LoadModelFromMesh(GenMeshCylinder(1, 2, 16));              // OK! (par_shapes)
    Model model = LoadModelFromMesh(GenMeshTorus(0.25f, 4.0f, 16, 32));
    //Model model = LoadModelFromMesh(GenMeshKnot(1.0f, 2.0f, 16, 128));
    model.material.maps[MAP_DIFFUSE].texture = LoadTexture("resources/pixels.png");

    // Debug information
    /*
    printf("model.mesh.vertexCount: %i\n", model.mesh.vertexCount);
    printf("model.mesh.triangleCount: %i\n", model.mesh.triangleCount);
    printf("model.mesh.vboId (position): %i\n", model.mesh.vboId[0]);
    printf("model.mesh.vboId (texcoords): %i\n", model.mesh.vboId[1]);
    printf("model.mesh.vboId (normals): %i\n", model.mesh.vboId[2]);
    printf("model.mesh.vboId (indices): %i\n", model.mesh.vboId[6]);
    */
    
    // Define the camera to look into our 3d world
    Camera camera = {{ 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };

    Vector3 position = { 0.0f, 0.0f, 0.0f };
    
    SetCameraMode(camera, CAMERA_FREE);         // Set a free camera mode

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);      // Update internal camera and our camera
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawModel(model, position, 1.0f, WHITE);

                DrawGrid(10, 1.0);

            End3dMode();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}