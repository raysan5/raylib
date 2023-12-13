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

#define RPBR_IMPLEMENTATION
#include "rpbr.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            120
#endif


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



    PBRModel model = PBRModelLoad("resources/models/old_car_new.glb");
    //if we use obj file formator if model doesn't have tangents we have to calculate MeshTangents
    //by using raylib function GenMeshTangents(mesh) for example: obj file doesn't support tangents
    //GenMeshTangents(&model.model.meshes[0]); 

    PBRMaterial model_mat = (PBRMaterial){0};
    PBRMaterialSetup(&model_mat, shader, NULL); //environment = NULL for now
    PBRLoadTextures(&model_mat, PBR_TEXTURE_ALBEDO, "resources/old_car_d.png");
    PBRLoadTextures(&model_mat, PBR_TEXTURE_MRA, "resources/old_car_mra.png");
    PBRLoadTextures(&model_mat, PBR_TEXTURE_NORMAL, "resources/old_car_n.png");
    PBRLoadTextures(&model_mat, PBR_TEXTURE_EMISSIVE, "resources/old_car_e.png");
    PBRSetColor(&model_mat,PBR_COLOR_EMISSIVE, (Color){255,162,0,255});
    PBRSetVec2(&model_mat, PBR_VEC2_TILING,(Vector2){0.5,0.5});
    PBRSetMaterial(&model,&model_mat,0);

    PBRModel floor = PBRModelLoad("resources/models/plane.glb");

    PBRMaterial floor_mat = (PBRMaterial){0};
    PBRMaterialSetup(&floor_mat, shader, NULL);
    PBRLoadTextures(&floor_mat, PBR_TEXTURE_ALBEDO, "resources/road_a.png");
    PBRLoadTextures(&floor_mat, PBR_TEXTURE_MRA, "resources/road_mra.png");
    PBRLoadTextures(&floor_mat, PBR_TEXTURE_NORMAL, "resources/road_n.png");
    PBRSetVec2(&floor_mat, PBR_VEC2_TILING,(Vector2){0.5,0.5});
    PBRSetMaterial(&floor,&floor_mat,0);

    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    int numOfLightsLoc = GetShaderLocation(shader, "numOfLights");
    int numOfLights = 4;
    SetShaderValue(shader, numOfLightsLoc, &numOfLights, SHADER_UNIFORM_INT);

    Color ambCol = (Color){26,32,135,255};
    float ambIntens = 0.02;

    int albedoLoc = GetShaderLocation(shader, "albedo");
    PBRSetAmbient(shader,ambCol,ambIntens);

    // Create lights
    PBRLight lights[MAX_LIGHTS] = { 0 };
    lights[0] = PBRLightCreate(LIGHT_POINT, (Vector3){ -1, 1, -2 }, (Vector3){0,0,0}, YELLOW,4, shader);
    lights[1] = PBRLightCreate(LIGHT_POINT, (Vector3){ 2,  1, 1 }, (Vector3){0,0,0}, GREEN,3.3, shader);
    lights[2] = PBRLightCreate(LIGHT_POINT, (Vector3){ -2, 1, 1 }, (Vector3){0,0,0}, RED,8.3, shader);
    lights[3] = PBRLightCreate(LIGHT_POINT, (Vector3){ 1,  1, -2 }, (Vector3){0,0,0}, BLUE,2, shader);
    SetShaderValueV(shader, GetShaderLocation(shader, "lights"), lights, SHADER_UNIFORM_FLOAT, numOfLights);

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
        emissiveCnt--;
        if(emissiveCnt<=0){
            emissiveCnt = GetRandomValue(0,20);
            PBRSetFloat(&model_mat,PBR_PARAM_EMISSIVE,(float)GetRandomValue(0,100)/100);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);

                PBRDrawModel(floor, (Vector3){0,0,0}, 5.0f);
                PBRDrawModel(model, (Vector3) {0, 0.0, 0}, 0.005);

                // Draw spheres to show where the lights are
                for (int i = 0; i < MAX_LIGHTS; i++) {
                    Color col = (Color) {lights[i].color[0] * 255, lights[i].color[1] * 255, lights[i].color[2] * 255,
                                         lights[i].color[3] * 255};
                    if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, col);
                    else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(col, 0.3f));
                }
            EndMode3D();

            DrawText("(c) Old Rusty Car model by Renafox (https://skfb.ly/LxRy)", screenWidth - 320, screenHeight - 20, 10, GRAY);
            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    //--------------------------------------------------------------------------------------
    // De-Initialization
    //--------------------------------------------------------------------------------------

    UnloadModel(floor.model);           // Unload model
    UnloadModel(model.model);           // Unload model
    UnloadShader(shader);               // Unload Shader
    UnloadPBRMaterial(floor_mat);       // Unload PBRMaterial
    UnloadPBRMaterial(model_mat);       // Unload PBRMaterial
    
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
    }
