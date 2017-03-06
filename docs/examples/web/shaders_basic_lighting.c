/*******************************************************************************************
*
*   raylib [shaders] example - Basic lighting: Blinn-Phong (adapted for HTML5 platform)
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define SHININESS_SPEED 1.0f
#define LIGHT_SPEED 0.25f

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 800;
int screenHeight = 450;

// Light type
typedef struct Light {
    Vector3 position;
    Vector3 direction;
    float intensity;
    float specIntensity;
    Color diffuse;
    Color ambient;
    Color specular;
} Light;

// Camera initialization
Camera camera = {{ 8.0f, 8.0f, 8.0f }, { 0.0f, 3.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }};

Vector3 position = { 0.0f, 0.0f, 0.0f };  // Set model position

Model model;
Shader shader;

int lIntensityLoc;
int lAmbientLoc;
int lDiffuseLoc;
int lSpecularLoc;
int lSpecIntensityLoc;

int mAmbientLoc;
int mSpecularLoc;
int mGlossLoc;

// Camera and light vectors shader locations
int cameraLoc;
int lightLoc;

// Model and View matrix locations (required for lighting)
int modelLoc;

// Light and material definitions
Light light;
Material matBlinn;

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
    SetConfigFlags(FLAG_MSAA_4X_HINT);      // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - model shader");

    // Model initialization
    model = LoadModel("resources/model/dwarf.obj");
    shader = LoadShader("resources/shaders/phong.vs",
                        "resources/shaders/phong.fs");
    SetModelShader(&model, shader);
    // Shader locations initialization
    lIntensityLoc = GetShaderLocation(shader, "light_intensity");
    lAmbientLoc = GetShaderLocation(shader, "light_ambientColor");
    lDiffuseLoc = GetShaderLocation(shader, "light_diffuseColor");
    lSpecularLoc = GetShaderLocation(shader, "light_specularColor");
    lSpecIntensityLoc = GetShaderLocation(shader, "light_specIntensity");
    
    mAmbientLoc = GetShaderLocation(shader, "mat_ambientColor");
    mSpecularLoc = GetShaderLocation(shader, "mat_specularColor");
    mGlossLoc = GetShaderLocation(shader, "mat_glossiness");
    
    // Camera and light vectors shader locations
    cameraLoc = GetShaderLocation(shader, "cameraPos");
    lightLoc = GetShaderLocation(shader, "lightPos");
    
    // Model and View matrix locations (required for lighting)
    modelLoc = GetShaderLocation(shader, "modelMatrix");
    //int viewLoc = GetShaderLocation(shader, "viewMatrix");        // Not used
    
    // Light initialization
    light.position = (Vector3){ 4.0f, 2.0f, 0.0f };
    light.direction = (Vector3){ 5.0f, 1.0f, 1.0f };
    light.intensity = 1.0f;
    light.diffuse = WHITE;
    light.ambient = (Color){ 150, 75, 0, 255 };
    light.specular = WHITE;
    light.specIntensity = 1.0f;
    
    // Material initialization
    matBlinn.colDiffuse = WHITE;
    matBlinn.colAmbient = (Color){ 50, 50, 50, 255 };
    matBlinn.colSpecular = WHITE;
    matBlinn.glossiness = 50.0f;
    
    // Setup camera
    SetCameraMode(CAMERA_FREE);             // Set camera mode
    SetCameraPosition(camera.position);     // Set internal camera position to match our camera position
    SetCameraTarget(camera.target);         // Set internal camera target to match our camera target

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
    UnloadShader(shader);       // Unload shader
    UnloadModel(model);         // Unload model

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
    UpdateCamera(&camera);      // Update camera position
    
    // NOTE: Model transform can be set in model.transform or directly with params at draw... WATCH OUT!
    SetShaderValueMatrix(shader, modelLoc, model.transform);            // Send model matrix to shader
    //SetShaderValueMatrix(shader, viewLoc, GetCameraMatrix(camera));   // Not used
    
    // Glossiness input control
    if(IsKeyDown(KEY_UP)) matBlinn.glossiness += SHININESS_SPEED;
    else if(IsKeyDown(KEY_DOWN))
    {
        matBlinn.glossiness -= SHININESS_SPEED;
        if( matBlinn.glossiness < 0) matBlinn.glossiness = 0.0f;
    }
    
    // Light X movement
    if (IsKeyDown(KEY_D)) light.position.x += LIGHT_SPEED;
    else if(IsKeyDown(KEY_A)) light.position.x -= LIGHT_SPEED;
    
    // Light Y movement
    if (IsKeyDown(KEY_LEFT_SHIFT)) light.position.y += LIGHT_SPEED;
    else if (IsKeyDown(KEY_LEFT_CONTROL)) light.position.y -= LIGHT_SPEED;

    // Light Z movement
    if (IsKeyDown(KEY_S)) light.position.z += LIGHT_SPEED;
    else if (IsKeyDown(KEY_W)) light.position.z -= LIGHT_SPEED;
    
    // Send light values to shader
    SetShaderValue(shader, lIntensityLoc, &light.intensity, 1);
    SetShaderValue(shader, lAmbientLoc, ColorToFloat(light.ambient), 3);
    SetShaderValue(shader, lDiffuseLoc, ColorToFloat(light.diffuse), 3);
    SetShaderValue(shader, lSpecularLoc, ColorToFloat(light.specular), 3);
    SetShaderValue(shader, lSpecIntensityLoc, &light.specIntensity, 1);
    
    // Send material values to shader
    SetShaderValue(shader, mAmbientLoc, ColorToFloat(matBlinn.colAmbient), 3);
    SetShaderValue(shader, mSpecularLoc, ColorToFloat(matBlinn.colSpecular), 3);
    SetShaderValue(shader, mGlossLoc, &matBlinn.glossiness, 1);
    
    // Send camera and light transform values to shader
    SetShaderValue(shader, cameraLoc, VectorToFloat(camera.position), 3);
    SetShaderValue(shader, lightLoc, VectorToFloat(light.position), 3);
    //----------------------------------------------------------------------------------
    
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    
        ClearBackground(RAYWHITE);
        
        Begin3dMode(camera);
            
            DrawModel(model, position, 4.0f, matBlinn.colDiffuse);
            DrawSphere(light.position, 0.5f, GOLD);
            
            DrawGrid(20, 1.0f);
            
        End3dMode();
        
        DrawFPS(10, 10);                // Draw FPS
        
    EndDrawing();
    //----------------------------------------------------------------------------------
}