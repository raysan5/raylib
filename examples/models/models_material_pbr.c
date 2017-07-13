/*******************************************************************************************
*
*   raylib [models] example - PBR material
*
*   This example has been created using raylib 1.8 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2017 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - pbr material");

    // Define the camera to look into our 3d world
    Camera camera = {{ 4.0f, 4.0f, 4.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };

    // Load model and PBR material
    Model model = LoadModel("resources/pbr/trooper.obj");
    
    Texture2D texHDR = LoadTexture("resources/pinetree.hdr");
    model.material = LoadMaterialPBR(texHDR, (Color){ 255, 255, 255, 255 }, 1.0f, 1.0f);
    
    //model.material.maps[TEXMAP_ALBEDO].tex = LoadTexture("resources/pbr/trooper_albedo.png");
    SetMaterialTexture(&model.material, TEXMAP_ALBEDO, LoadTexture("resources/pbr/trooper_albedo.png"));
/*
    SetMaterialTexture(&model.material, TEXMAP_NORMAL, LoadTexture("resources/pbr/trooper_normals.png"));
    SetTextureFilter(model.material.maps[TEXMAP_NORMAL].tex, FILTER_BILINEAR);

    SetMaterialTexture(&model.material, TEXMAP_METALNESS, LoadTexture("resources/pbr/trooper_metalness.png"));
    SetTextureFilter(model.material.maps[TEXMAP_METALNESS].tex, FILTER_BILINEAR);

    SetMaterialTexture(&model.material, TEXMAP_ROUGHNESS, LoadTexture("resources/pbr/trooper_roughness.png"));
    SetTextureFilter(model.material.maps[TEXMAP_ROUGHNESS].tex, FILTER_BILINEAR);

    SetMaterialTexture(&model.material, TEXMAP_OCCLUSION, LoadTexture("resources/pbr/trooper_ao.png"));
    SetTextureFilter(model.material.maps[TEXMAP_OCCLUSION].tex, FILTER_BILINEAR);

    for (int i = 0; i < 12; i++)
    {
       //printf("TexMap %i ID: %i\n", i, model.material.maps[i].tex.id);
    }
*/

    SetCameraMode(camera, CAMERA_ORBITAL);  // Set an orbital camera mode

    SetTargetFPS(60);                       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);              // Update camera
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawModel(model, VectorZero(), 1.0f, WHITE);
                
                DrawGrid(10, 1.0f);

            End3dMode();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);         // Unload skybox model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
