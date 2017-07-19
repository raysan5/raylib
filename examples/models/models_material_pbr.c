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

#define         MAX_LIGHTS            4         // Max lights supported by shader
#define         LIGHT_DISTANCE        3.5f      // Light distance from world center
#define         LIGHT_HEIGHT          1.0f      // Light height position

typedef enum {
    LIGHT_DIRECTIONAL,
    LIGHT_POINT
} LightType;

typedef struct {
    bool enabled;
    LightType type;
    Vector3 position;
    Vector3 target;
    Color color;
    int enabledLoc;
    int typeLoc;
    int posLoc;
    int targetLoc;
    int colorLoc;
} Light;

int lightsCount = 0;                     // Current amount of created lights

Light CreateLight(int type, Vector3 pos, Vector3 targ, Color color, Shader shader);         // Defines a light and get locations from PBR shader
void UpdateLightValues(Shader shader, Light light);                                         // Send to PBR shader light values

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(screenWidth, screenHeight, "raylib [models] example - pbr material");

    // Define the camera to look into our 3d world
    Camera camera = {{ 4.0f, 4.0f, 4.0f }, { 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };

    // Load model and PBR material
    Model model = LoadModel("resources/pbr/trooper.obj");
    
    Texture2D texHDR = LoadTexture("resources/pinetree.hdr");
    model.material = LoadMaterialPBR(texHDR, (Color){ 255, 255, 255, 255 }, 1.0f, 1.0f);
    
    SetMaterialTexture(&model.material, MAP_ALBEDO, LoadTexture("resources/pbr/trooper_albedo.png"));
    SetMaterialTexture(&model.material, MAP_NORMAL, LoadTexture("resources/pbr/trooper_normals.png"));
    SetMaterialTexture(&model.material, MAP_METALNESS, LoadTexture("resources/pbr/trooper_metalness.png"));
    SetMaterialTexture(&model.material, MAP_ROUGHNESS, LoadTexture("resources/pbr/trooper_roughness.png"));
    SetMaterialTexture(&model.material, MAP_OCCLUSION, LoadTexture("resources/pbr/trooper_ao.png"));
    
    // Set textures filtering for better quality
    SetTextureFilter(model.material.maps[MAP_ALBEDO].texture, FILTER_BILINEAR);
    SetTextureFilter(model.material.maps[MAP_NORMAL].texture, FILTER_BILINEAR);
    SetTextureFilter(model.material.maps[MAP_METALNESS].texture, FILTER_BILINEAR);
    SetTextureFilter(model.material.maps[MAP_ROUGHNESS].texture, FILTER_BILINEAR);
    SetTextureFilter(model.material.maps[MAP_OCCLUSION].texture, FILTER_BILINEAR);
    
    int renderModeLoc = GetShaderLocation(model.material.shader, "renderMode");
    SetShaderValuei(model.material.shader, renderModeLoc, (int[1]){ 0 }, 1);

    SetCameraMode(camera, CAMERA_ORBITAL);  // Set an orbital camera mode
    
    // Define lights attributes
    Light lights[MAX_LIGHTS] = { CreateLight(LIGHT_POINT, (Vector3){ LIGHT_DISTANCE, LIGHT_HEIGHT, 0.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 255, 0, 0, 255 }, model.material.shader),
    CreateLight(LIGHT_POINT, (Vector3){ 0.0f, LIGHT_HEIGHT, LIGHT_DISTANCE }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 0, 255, 0, 255 }, model.material.shader),
    CreateLight(LIGHT_POINT, (Vector3){ -LIGHT_DISTANCE, LIGHT_HEIGHT, 0.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 0, 0, 255, 255 }, model.material.shader),
    CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 0.0f, LIGHT_HEIGHT*2.0f, -LIGHT_DISTANCE }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 255, 0, 255, 255 }, model.material.shader) };

    SetTargetFPS(60);                       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);              // Update camera
        
        // Send to material PBR shader camera view position
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(model.material.shader, model.material.shader.locs[LOC_VECTOR_VIEW], cameraPos, 3);
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

// Defines a light and get locations from PBR shader
Light CreateLight(int type, Vector3 pos, Vector3 targ, Color color, Shader shader)
{
    Light light = { 0 };

    if (lightsCount < MAX_LIGHTS)
    {
        light.enabled = true;
        light.type = type;
        light.position = pos;
        light.target = targ;
        light.color = color;

        char enabledName[32] = "lights[x].enabled\0";
        char typeName[32] = "lights[x].type\0";
        char posName[32] = "lights[x].position\0";
        char targetName[32] = "lights[x].target\0";
        char colorName[32] = "lights[x].color\0";
        enabledName[7] = '0' + lightsCount;
        typeName[7] = '0' + lightsCount;
        posName[7] = '0' + lightsCount;
        targetName[7] = '0' + lightsCount;
        colorName[7] = '0' + lightsCount;

        light.enabledLoc = GetShaderLocation(shader, enabledName);
        light.typeLoc = GetShaderLocation(shader, typeName);
        light.posLoc = GetShaderLocation(shader, posName);
        light.targetLoc = GetShaderLocation(shader, targetName);
        light.colorLoc = GetShaderLocation(shader, colorName);

        UpdateLightValues(shader, light);
        lightsCount++;
    }

    return light;
}

// Send to PBR shader light values
void UpdateLightValues(Shader shader, Light light)
{
    // Send to shader light enabled state and type
    SetShaderValuei(shader, light.enabledLoc, (int[1]){ light.enabled }, 1);
    SetShaderValuei(shader, light.typeLoc, (int[1]){ light.type }, 1);

    // Send to shader light position values
    float position[3] = { light.position.x, light.position.y, light.position.z };
    SetShaderValue(shader, light.posLoc, position, 3);

    // Send to shader light target position values
    float target[3] = { light.target.x, light.target.y, light.target.z };
    SetShaderValue(shader, light.targetLoc, target, 3);

    // Send to shader light color values
    float diff[4] = { (float)light.color.r/(float)255, (float)light.color.g/(float)255, (float)light.color.b/(float)255, (float)light.color.a/(float)255 };
    SetShaderValue(shader, light.colorLoc, diff, 4);
}
