/*******************************************************************************************
*
*   raylib - Phong lighting shader example
*
*   This example has been created using raylib v1.3.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

#define SHININESS_SPEED 1.0f
#define LIGHT_SPEED 0.25f

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [lighting] example - basic blinn-phong lighting");
    SetTargetFPS(60);
    
    // Camera initialization
    Camera camera = {{ 10.0, 8.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};
    
    // Model initialization
    Vector3 position = { 0.0, 0.0, 0.0 };
    Model model = LoadModel("resources/model/dwarf.obj");
    // Shader shader = LoadShader("resources/shaders/phong.vs", "resources/shaders/phong.fs");
    SetModelShader(&model, shader);
    
    // Shader locations initialization
    int lIntensityLoc = GetShaderLocation(shader, "light_intensity");
    int lAmbientLoc = GetShaderLocation(shader, "light_ambientColor");
    int lDiffuseLoc = GetShaderLocation(shader, "light_diffuseColor");
    int lSpecularLoc = GetShaderLocation(shader, "light_specularColor");
    int lSpecIntensityLoc = GetShaderLocation(shader, "light_specIntensity");
    
    int mAmbientLoc = GetShaderLocation(shader, "mat_ambientColor");
    int mSpecularLoc = GetShaderLocation(shader, "mat_specularColor");
    int mGlossLoc = GetShaderLocation(shader, "mat_glossiness");
    
    // Camera and light vectors shader locations
    int cameraLoc = GetShaderLocation(shader, "cameraPos");
    int lightLoc = GetShaderLocation(shader, "lightPos");
    
    // Light and material definitions
    Light directionalLight;
    Material blinnMaterial;
    
    // Light initialization
    SetLightPosition(&directionalLight, (Vector3){5.0f, 1.0f, 1.0f});
    SetLightRotation(&directionalLight, (Vector3){5.0f, 1.0f, 1.0f});
    SetLightIntensity(&directionalLight, 1);
    SetLightAmbientColor(&directionalLight, (Vector3){0.6f, 0.3f, 0});
    SetLightDiffuseColor(&directionalLight, (Vector3){1, 1, 1});
    SetLightSpecularColor(&directionalLight, (Vector3){1, 1, 1});
    SetLightSpecIntensity(&directionalLight, 1);
    
    // Material initialization
    SetMaterialAmbientColor(&blinnMaterial, (Vector3){0.2f, 0.2f, 0.2f});
    SetMaterialDiffuseColor(&blinnMaterial, (Vector3){1.0f, 1.0f, 1.0f});
    SetMaterialSpecularColor(&blinnMaterial, (Vector3){1.0f, 1.0f, 1.0f});
    SetMaterialGlossiness(&blinnMaterial, 50);
    
    // Setup camera
    SetCameraMode(CAMERA_FREE);             // Set camera mode
    SetCameraPosition(camera.position);     // Set internal camera position to match our camera position
    SetCameraTarget(camera.target);         // Set internal camera target to match our camera target
    float cameraPosition[3] = { camera.position.x, camera.position.y, camera.position.z };  // Camera position vector in float array
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        // Update camera position and its float array for shader
        UpdateCamera(&camera);
        cameraPosition[0] = camera.position.x;
        cameraPosition[1] = camera.position.y;
        cameraPosition[2] = camera.position.z;
        
        // Glossiness input control
        if(IsKeyDown(KEY_UP))
        {
            blinnMaterial.glossiness[0] += SHININESS_SPEED;
        }
        else if(IsKeyDown(KEY_DOWN))
        {
            blinnMaterial.glossiness[0] -= SHININESS_SPEED;
            
            if(blinnMaterial.glossiness[0] < 0) blinnMaterial.glossiness[0] = 0;
        }
        
        // Light X movement
        if(IsKeyDown(KEY_D))
        {
            directionalLight.position[0] += LIGHT_SPEED;
        }
        else if(IsKeyDown(KEY_A))
        {
            directionalLight.position[0] -= LIGHT_SPEED;
        }
        
        // Light Y movement
        if(IsKeyDown(KEY_LEFT_SHIFT))
        {
            directionalLight.position[1] += LIGHT_SPEED;
        }
        else if(IsKeyDown(KEY_LEFT_CONTROL))
        {
            directionalLight.position[1] -= LIGHT_SPEED;
        }

        // Light Z movement
        if(IsKeyDown(KEY_S))
        {
            directionalLight.position[2] += LIGHT_SPEED;
        }
        else if(IsKeyDown(KEY_W))
        {
            directionalLight.position[2] -= LIGHT_SPEED;
        }
        
        // Send light values to shader
        SetShaderValue(shader, lIntensityLoc, directionalLight.intensity, 1);
        SetShaderValue(shader, lAmbientLoc, directionalLight.ambientColor, 3);
        SetShaderValue(shader, lDiffuseLoc, directionalLight.diffuseColor, 3);
        SetShaderValue(shader, lSpecularLoc, directionalLight.specularColor, 3);
        SetShaderValue(shader, lSpecIntensityLoc, directionalLight.specularIntensity, 1);
        
        // Send material values to shader
        SetShaderValue(shader, mAmbientLoc, blinnMaterial.ambientColor, 3);
        SetShaderValue(shader, mSpecularLoc, blinnMaterial.specularColor, 3);
        SetShaderValue(shader, mGlossLoc, blinnMaterial.glossiness, 1);
        
        // Send camera and light transform values to shader
        SetShaderValue(shader, cameraLoc, cameraPosition, 3);
        SetShaderValue(shader, lightLoc, directionalLight.position, 3);
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            Begin3dMode(camera);
                
                DrawModel(model, position, 0.1f, (Color){255 * blinnMaterial.diffuseColor[0], 255 * blinnMaterial.diffuseColor[1], 255 * blinnMaterial.diffuseColor[2], 255});

                DrawSphere((Vector3){directionalLight.position[0], directionalLight.position[1], directionalLight.position[2]}, 1, YELLOW);
                
            End3dMode();
            
            // Draw FPS
            DrawFPS(10, 10);
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Unload all loaded data
    UnloadShader(shader);
    UnloadModel(model);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}