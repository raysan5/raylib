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

#include <stdio.h>

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#define CUBEMAP_SIZE         512        // Cubemap texture size
#define IRRADIANCE_SIZE       32        // Irradiance texture size
#define PREFILTERED_SIZE     256        // Prefiltered HDR environment texture size
#define BRDF_SIZE            512        // BRDF LUT texture size

// PBR material loading
static Material LoadMaterialPBR(Color albedo, float metalness, float roughness);

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(screenWidth, screenHeight, "raylib [models] example - pbr material");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 4.0f, 4.0f, 4.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.type = CAMERA_PERSPECTIVE;                   // Camera mode type

    // Load model and PBR material
    Model model = LoadModel("resources/pbr/trooper.obj");
    
    // Mesh tangents are generated... and uploaded to GPU
    // NOTE: New VBO for tangents is generated at default location and also binded to mesh VAO
    MeshTangents(&model.meshes[0]);

    model.materials[0] = LoadMaterialPBR((Color){ 255, 255, 255, 255 }, 1.0f, 1.0f);

    // Define lights attributes
    // NOTE: Shader is passed to every light on creation to define shader bindings internally
    Light lights[MAX_LIGHTS] = { 
        CreateLight(LIGHT_POINT, (Vector3){ LIGHT_DISTANCE, LIGHT_HEIGHT, 0.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 255, 0, 0, 255 }, model.materials[0].shader),
        CreateLight(LIGHT_POINT, (Vector3){ 0.0f, LIGHT_HEIGHT, LIGHT_DISTANCE }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 0, 255, 0, 255 }, model.materials[0].shader),
        CreateLight(LIGHT_POINT, (Vector3){ -LIGHT_DISTANCE, LIGHT_HEIGHT, 0.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 0, 0, 255, 255 }, model.materials[0].shader),
        CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 0.0f, LIGHT_HEIGHT*2.0f, -LIGHT_DISTANCE }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 255, 0, 255, 255 }, model.materials[0].shader) 
    };
    
    SetCameraMode(camera, CAMERA_ORBITAL);  // Set an orbital camera mode

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
        SetShaderValue(model.materials[0].shader, model.materials[0].shader.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModel(model, Vector3Zero(), 1.0f, WHITE);
                
                DrawGrid(10, 1.0f);

            EndMode3D();

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

// Load PBR material (Supports: ALBEDO, NORMAL, METALNESS, ROUGHNESS, AO, EMMISIVE, HEIGHT maps)
// NOTE: PBR shader is loaded inside this function
static Material LoadMaterialPBR(Color albedo, float metalness, float roughness)
{
    Material mat = { 0 };       // NOTE: All maps textures are set to { 0 }
    
    #define     PATH_PBR_VS     "resources/shaders/pbr.vs"      // Path to physically based rendering vertex shader
    #define     PATH_PBR_FS     "resources/shaders/pbr.fs"      // Path to physically based rendering fragment shader
   
    mat.shader = LoadShader(PATH_PBR_VS, PATH_PBR_FS);
    
    // Get required locations points for PBR material
    // NOTE: Those location names must be available and used in the shader code
    mat.shader.locs[LOC_MAP_ALBEDO] = GetShaderLocation(mat.shader, "albedo.sampler");
    mat.shader.locs[LOC_MAP_METALNESS] = GetShaderLocation(mat.shader, "metalness.sampler");
    mat.shader.locs[LOC_MAP_NORMAL] = GetShaderLocation(mat.shader, "normals.sampler");
    mat.shader.locs[LOC_MAP_ROUGHNESS] = GetShaderLocation(mat.shader, "roughness.sampler");
    mat.shader.locs[LOC_MAP_OCCLUSION] = GetShaderLocation(mat.shader, "occlusion.sampler");
    //mat.shader.locs[LOC_MAP_EMISSION] = GetShaderLocation(mat.shader, "emission.sampler");
    //mat.shader.locs[LOC_MAP_HEIGHT] = GetShaderLocation(mat.shader, "height.sampler");
    mat.shader.locs[LOC_MAP_IRRADIANCE] = GetShaderLocation(mat.shader, "irradianceMap");
    mat.shader.locs[LOC_MAP_PREFILTER] = GetShaderLocation(mat.shader, "prefilterMap");
    mat.shader.locs[LOC_MAP_BRDF] = GetShaderLocation(mat.shader, "brdfLUT");

    // Set view matrix location
    mat.shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(mat.shader, "matModel");
    mat.shader.locs[LOC_MATRIX_VIEW] = GetShaderLocation(mat.shader, "view");
    mat.shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(mat.shader, "viewPos");
    
    // Set PBR standard maps
    mat.maps[MAP_ALBEDO].texture = LoadTexture("resources/pbr/trooper_albedo.png");
    mat.maps[MAP_NORMAL].texture = LoadTexture("resources/pbr/trooper_normals.png");
    mat.maps[MAP_METALNESS].texture = LoadTexture("resources/pbr/trooper_metalness.png");
    mat.maps[MAP_ROUGHNESS].texture = LoadTexture("resources/pbr/trooper_roughness.png");
    mat.maps[MAP_OCCLUSION].texture = LoadTexture("resources/pbr/trooper_ao.png");
    
    // Set environment maps
    #define     PATH_CUBEMAP_VS         "resources/shaders/cubemap.vs"          // Path to equirectangular to cubemap vertex shader
    #define     PATH_CUBEMAP_FS         "resources/shaders/cubemap.fs"          // Path to equirectangular to cubemap fragment shader
    #define     PATH_SKYBOX_VS          "resources/shaders/skybox.vs"           // Path to skybox vertex shader
    #define     PATH_IRRADIANCE_FS      "resources/shaders/irradiance.fs"       // Path to irradiance (GI) calculation fragment shader
    #define     PATH_PREFILTER_FS       "resources/shaders/prefilter.fs"        // Path to reflection prefilter calculation fragment shader
    #define     PATH_BRDF_VS            "resources/shaders/brdf.vs"     // Path to bidirectional reflectance distribution function vertex shader 
    #define     PATH_BRDF_FS            "resources/shaders/brdf.fs"     // Path to bidirectional reflectance distribution function fragment shader
    
    Shader shdrCubemap = LoadShader(PATH_CUBEMAP_VS, PATH_CUBEMAP_FS);
    printf("Loaded shader: cubemap\n");
    Shader shdrIrradiance = LoadShader(PATH_SKYBOX_VS, PATH_IRRADIANCE_FS);
    printf("Loaded shader: irradiance\n");
    Shader shdrPrefilter = LoadShader(PATH_SKYBOX_VS, PATH_PREFILTER_FS);
    printf("Loaded shader: prefilter\n");
    Shader shdrBRDF = LoadShader(PATH_BRDF_VS, PATH_BRDF_FS);
    printf("Loaded shader: brdf\n");
    
    // Setup required shader locations
    SetShaderValue(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"), (int[1]){ 0 }, UNIFORM_INT);
    SetShaderValue(shdrIrradiance, GetShaderLocation(shdrIrradiance, "environmentMap"), (int[1]){ 0 }, UNIFORM_INT);
    SetShaderValue(shdrPrefilter, GetShaderLocation(shdrPrefilter, "environmentMap"), (int[1]){ 0 }, UNIFORM_INT);

    Texture2D texHDR = LoadTexture("resources/dresden_square.hdr");
    Texture2D cubemap = GenTextureCubemap(shdrCubemap, texHDR, CUBEMAP_SIZE);
    mat.maps[MAP_IRRADIANCE].texture = GenTextureIrradiance(shdrIrradiance, cubemap, IRRADIANCE_SIZE);
    mat.maps[MAP_PREFILTER].texture = GenTexturePrefilter(shdrPrefilter, cubemap, PREFILTERED_SIZE);
    mat.maps[MAP_BRDF].texture = GenTextureBRDF(shdrBRDF, BRDF_SIZE);
    UnloadTexture(cubemap);
    UnloadTexture(texHDR);
    
    // Unload already used shaders (to create specific textures)
    UnloadShader(shdrCubemap);
    UnloadShader(shdrIrradiance);
    UnloadShader(shdrPrefilter);
    UnloadShader(shdrBRDF);
    
    // Set textures filtering for better quality
    SetTextureFilter(mat.maps[MAP_ALBEDO].texture, FILTER_BILINEAR);
    SetTextureFilter(mat.maps[MAP_NORMAL].texture, FILTER_BILINEAR);
    SetTextureFilter(mat.maps[MAP_METALNESS].texture, FILTER_BILINEAR);
    SetTextureFilter(mat.maps[MAP_ROUGHNESS].texture, FILTER_BILINEAR);
    SetTextureFilter(mat.maps[MAP_OCCLUSION].texture, FILTER_BILINEAR);
    
    // Enable sample usage in shader for assigned textures
    SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "albedo.useSampler"), (int[1]){ 1 }, UNIFORM_INT);
    SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "normals.useSampler"), (int[1]){ 1 }, UNIFORM_INT);
    SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "metalness.useSampler"), (int[1]){ 1 }, UNIFORM_INT);
    SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "roughness.useSampler"), (int[1]){ 1 }, UNIFORM_INT);
    SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "occlusion.useSampler"), (int[1]){ 1 }, UNIFORM_INT);
    
    int renderModeLoc = GetShaderLocation(mat.shader, "renderMode");
    SetShaderValue(mat.shader, renderModeLoc, (int[1]){ 0 }, UNIFORM_INT);

    // Set up material properties color
    mat.maps[MAP_ALBEDO].color = albedo;
    mat.maps[MAP_NORMAL].color = (Color){ 128, 128, 255, 255 };
    mat.maps[MAP_METALNESS].value = metalness;
    mat.maps[MAP_ROUGHNESS].value = roughness;
    mat.maps[MAP_OCCLUSION].value = 1.0f;
    mat.maps[MAP_EMISSION].value = 0.5f;
    mat.maps[MAP_HEIGHT].value = 0.5f;

    return mat;
}
