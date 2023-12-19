/*******************************************************************************************
*
*   raylib [core] example - Model Defuse Normal Shader (adapted for HTML5 platform)
*
*   This example is prepared to compile for PLATFORM_WEB and PLATFORM_DESKTOP
*   As you will notice, code structure is slightly different to the other examples...
*   To compile it for PLATFORM_WEB just uncomment #define PLATFORM_WEB at beginning
*
*   This example has been created using raylib 5.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2023-2024 Afan OLOVCIC (@_DevDad)  2015 Ramon Santamaria (@raysan5)
*   Model: "Old Rusty Car" (https://skfb.ly/LxRy) by Renafox is licensed under Creative Commons Attribution-NonCommercial (http://creativecommons.org/licenses/by-nc/4.0/).
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            120
#endif

#include <stdlib.h>             // Required for: NULL

#define MAX_LIGHTS  4           // Max dynamic lights supported by shader
int lightsCount;                // Current number of dynamic lights that have been created

typedef struct {
    int enabled;
    int type;
    Vector3 position;
    Vector3 target;
    float color[4];
    float intensity;

    int enabledLoc;
    int typeLoc;
    int positionLoc;
    int targetLoc;
    int colorLoc;
    int intensityLoc;
} PBRLight;

typedef enum {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT,
    LIGHT_SPOT
} PBRLightType;

// Create a light and get shader locations
PBRLight PBRLightCreate(int type, Vector3 position, Vector3 target, Color color, float intensity, Shader shader);

// Send light properties to shader
// NOTE: Light shader locations should be available
void PBRLightUpdate(Shader shader, PBRLight light);

//----------------------------------------------------------------------------------
// Main Entry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - basic pbr");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 2.0f, 2.0f, 6.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type


    Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/pbr.vs",GLSL_VERSION),
                               TextFormat("resources/shaders/glsl%i/pbr.fs",GLSL_VERSION));
    shader.locs[SHADER_LOC_MAP_ALBEDO] = GetShaderLocation(shader, "albedoMap");
    // In reality, metalness, roughness, and ambient occlusion are all packed into the MRA texture
    // We'll pass it in as the metalness map
    shader.locs[SHADER_LOC_MAP_METALNESS] = GetShaderLocation(shader, "mraMap");
    shader.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(shader, "normalMap");
    // Similarly to the MRA map, the emissive map packs different information into a single texture
    // This map stores both height and emission in reality
    shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "emissiveMap");
    shader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader, "albedoColor");

    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    int numOfLightsLoc = GetShaderLocation(shader, "numOfLights");
    int numOfLights = 4;
    SetShaderValue(shader, numOfLightsLoc, &numOfLights, SHADER_UNIFORM_INT);

    Color ambCol = (Color){ 26,32,135,255 };
    Vector3 ambColNormalized = (Vector3){ ambCol.r / 255.0f, ambCol.g / 255.0f, ambCol.b / 255.0f };
    float ambIntens = 0.02;

    int albedoLoc = GetShaderLocation(shader, "albedo");
    int ambColLoc = GetShaderLocation(shader, "ambientColor");
    int ambLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambColLoc, &ambColNormalized, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, ambLoc, &ambIntens, SHADER_UNIFORM_FLOAT);

    int emissiveIntensityLoc = GetShaderLocation(shader, "emissivePower");
    int emissiveColorLoc = GetShaderLocation(shader, "emissiveColor");
    int textureTilingLoc = GetShaderLocation(shader, "tiling");

    Model model = LoadModel("resources/models/old_car_new.glb");
    // If the OBJ file format is used, we will have to generate tangents manually:
    // GenMeshTangents(&model.meshes[0]);

    model.materials[0].shader = shader;

    model.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    model.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0.0f;
    model.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 0.0f;
    model.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;
    model.materials[0].maps[MATERIAL_MAP_EMISSION].color = (Color){ 255, 162, 0, 255 };

    model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("resources/old_car_d.png");
    model.materials[0].maps[MATERIAL_MAP_METALNESS].texture = LoadTexture("resources/old_car_mra.png");
    model.materials[0].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture("resources/old_car_n.png");
    model.materials[0].maps[MATERIAL_MAP_EMISSION].texture = LoadTexture("resources/old_car_e.png");
    // We store tiling parameters in the generic parameter slots in the Material class
    Vector2 modelTiling = (Vector2){ 0.5f, 0.5f };

    Model floor = LoadModel("resources/models/plane.glb");

    floor.materials[0].shader = shader;
    
    floor.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    floor.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0.0f;
    floor.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 0.0f;
    floor.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;
    floor.materials[0].maps[MATERIAL_MAP_EMISSION].color = BLACK;

    floor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("resources/road_a.png");
    floor.materials[0].maps[MATERIAL_MAP_METALNESS].texture = LoadTexture("resources/road_mra.png");
    floor.materials[0].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture("resources/road_n.png");

    Vector2 floorTiling = (Vector2){ 0.5f, 0.5f };

    // Create lights
    PBRLight lights[MAX_LIGHTS] = { 0 };
    lights[0] = PBRLightCreate(LIGHT_POINT, (Vector3){ -1, 1, -2 }, (Vector3){0,0,0}, YELLOW,4, shader);
    lights[1] = PBRLightCreate(LIGHT_POINT, (Vector3){ 2,  1, 1 }, (Vector3){0,0,0}, GREEN,3.3, shader);
    lights[2] = PBRLightCreate(LIGHT_POINT, (Vector3){ -2, 1, 1 }, (Vector3){0,0,0}, RED,8.3, shader);
    lights[3] = PBRLightCreate(LIGHT_POINT, (Vector3){ 1,  1, -2 }, (Vector3){0,0,0}, BLUE,2, shader);

    // The textures are always used
    int one = 1;
    SetShaderValue(shader, GetShaderLocation(shader, "useTexAlbedo"), &one, SHADER_UNIFORM_INT);
    SetShaderValue(shader, GetShaderLocation(shader, "useTexNormal"), &one, SHADER_UNIFORM_INT);
    SetShaderValue(shader, GetShaderLocation(shader, "useTexMRA"), &one, SHADER_UNIFORM_INT);
    SetShaderValue(shader, GetShaderLocation(shader, "useTexEmissive"), &one, SHADER_UNIFORM_INT);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second-------------------------------------------------------------

    int emissiveCnt = 0;
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);

        // Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
        float cameraPos[3] = {camera.position.x, camera.position.y, camera.position.z};
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        // Check key inputs to enable/disable lights
        if (IsKeyPressed(KEY_Y)) { lights[0].enabled = !lights[0].enabled; }
        if (IsKeyPressed(KEY_G)) { lights[1].enabled = !lights[1].enabled; }
        if (IsKeyPressed(KEY_R)) { lights[2].enabled = !lights[2].enabled; }
        if (IsKeyPressed(KEY_B)) { lights[3].enabled = !lights[3].enabled; }

        // Update light values (actually, only enable/disable them)
        for (int i = 0; i < MAX_LIGHTS; i++) PBRLightUpdate(shader, lights[i]);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(BLACK);
            
            BeginMode3D(camera);
                
                SetShaderValue(shader, textureTilingLoc, &floorTiling, SHADER_UNIFORM_VEC2);
                Vector4 floorEmission = ColorNormalize(floor.materials[0].maps[MATERIAL_MAP_EMISSION].color);
                SetShaderValue(shader, emissiveColorLoc, &floorEmission, SHADER_UNIFORM_VEC4);
                DrawModel(floor, (Vector3){0,0,0}, 5.0f, WHITE);

                emissiveCnt--;
                if (emissiveCnt <= 0)
                {
                    emissiveCnt = GetRandomValue(0, 20);
                    float intensity = (float)GetRandomValue(0, 100) / 100;
                    SetShaderValue(shader, emissiveIntensityLoc, &intensity, SHADER_UNIFORM_FLOAT);
                }
                SetShaderValue(shader, textureTilingLoc, &modelTiling, SHADER_UNIFORM_VEC2);
                Vector4 modelEmission = ColorNormalize(model.materials[0].maps[MATERIAL_MAP_EMISSION].color);
                SetShaderValue(shader, emissiveColorLoc, &modelEmission, SHADER_UNIFORM_VEC4);
                DrawModel(model, (Vector3) {0, 0.0, 0}, 0.005, WHITE);

                // Draw spheres to show where the lights are
                for (int i = 0; i < MAX_LIGHTS; i++)
                {
                    Color col = (Color) {lights[i].color[0] * 255, lights[i].color[1] * 255, lights[i].color[2] * 255,
                                         lights[i].color[3] * 255};
                    if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, col);
                    else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(col, 0.3f));
                }
                
            EndMode3D();

            DrawText("(c) Old Rusty Car model by Renafox (https://skfb.ly/LxRy)", screenWidth - 320, screenHeight - 20, 10, LIGHTGRAY);
            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    //--------------------------------------------------------------------------------------
    // De-Initialization
    //--------------------------------------------------------------------------------------
    model.materials[0].shader = (Shader){ 0 };
    floor.materials[0].shader = (Shader){ 0 };
    UnloadMaterial(model.materials[0]);
    UnloadMaterial(floor.materials[0]);
    model.materials[0].maps = NULL;
    floor.materials[0].maps = NULL;
    UnloadModel(floor);                 // Unload model
    UnloadModel(model);                 // Unload model
    UnloadShader(shader);               // Unload Shader
    
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

PBRLight PBRLightCreate(int type, Vector3 position, Vector3 target, Color color, float intensity, Shader shader)
{
    PBRLight light = { 0 };

    if (lightsCount < MAX_LIGHTS)
    {
        light.enabled = 1;
        light.type = type;
        light.position = position;
        light.target = target;
        light.color[0] = (float)color.r / (float)255;
        light.color[1] = (float)color.g / (float)255;
        light.color[2] = (float)color.b / (float)255;
        light.color[3] = (float)color.a / (float)255;
        light.intensity = intensity;
        // NOTE: Lighting shader naming must be the provided ones
        light.enabledLoc = GetShaderLocation(shader, TextFormat("lights[%i].enabled", lightsCount));
        light.typeLoc = GetShaderLocation(shader, TextFormat("lights[%i].type", lightsCount));
        light.positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", lightsCount));
        light.targetLoc = GetShaderLocation(shader, TextFormat("lights[%i].target", lightsCount));
        light.colorLoc = GetShaderLocation(shader, TextFormat("lights[%i].color", lightsCount));
        light.intensityLoc = GetShaderLocation(shader, TextFormat("lights[%i].intensity", lightsCount));
        PBRLightUpdate(shader, light);

        lightsCount++;
    }

    return light;
}

// Send light properties to shader
// NOTE: Light shader locations should be available
void PBRLightUpdate(Shader shader, PBRLight light)
{
    SetShaderValue(shader, light.enabledLoc, &light.enabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader, light.typeLoc, &light.type, SHADER_UNIFORM_INT);
    // Send to shader light position values
    float position[3] = { light.position.x, light.position.y, light.position.z };
    SetShaderValue(shader, light.positionLoc, position, SHADER_UNIFORM_VEC3);

    // Send to shader light target position values
    float target[3] = { light.target.x, light.target.y, light.target.z };
    SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, light.colorLoc, light.color, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, light.intensityLoc, &light.intensity, SHADER_UNIFORM_FLOAT);
}
