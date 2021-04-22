/*******************************************************************************************
*
*   raylib [models] example - PBR material
*
*   NOTE: This example requires raylib OpenGL 3.3 for shaders support and only #version 330
*         is currently supported. OpenGL ES 2.0 platforms are not supported at the moment.
*
*   This example has been created using raylib 1.8 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2017 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <stdio.h>

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define CUBEMAP_SIZE        1024        // Cubemap texture size
#define IRRADIANCE_SIZE       32        // Irradiance texture size
#define PREFILTERED_SIZE     256        // Prefiltered HDR environment texture size
#define BRDF_SIZE            512        // BRDF LUT texture size
#define LIGHT_DISTANCE      1000.0f
#define LIGHT_HEIGHT           1.0f

// PBR texture maps generation
static TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format); // Generate cubemap (6 faces) from equirectangular (panorama) texture
static TextureCubemap GenTextureIrradiance(Shader shader, TextureCubemap cubemap, int size);      // Generate irradiance cubemap using cubemap texture
static TextureCubemap GenTexturePrefilter(Shader shader, TextureCubemap cubemap, int size);       // Generate prefilter cubemap using cubemap texture
static Texture2D GenTextureBRDF(Shader shader, int size);              // Generate a generic BRDF texture

// PBR material loading
static Material LoadMaterialPBR(Color albedo, float metalness, float roughness);

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(screenWidth, screenHeight, "raylib [models] example - pbr material");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 4.0f, 4.0f, 4.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                   // Camera mode type

    // Load model and PBR material
    Model model = LoadModel("resources/pbr/trooper.obj");

    // Mesh tangents are generated... and uploaded to GPU
    // NOTE: New VBO for tangents is generated at default location and also binded to mesh VAO
    //MeshTangents(&model.meshes[0]);

    model.materials[0] = LoadMaterialPBR((Color){ 255, 255, 255, 255 }, 1.0f, 1.0f);

    // Create lights
    // NOTE: Lights are added to an internal lights pool automatically
    CreateLight(LIGHT_POINT, (Vector3){ LIGHT_DISTANCE, LIGHT_HEIGHT, 0.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 255, 0, 0, 255 }, model.materials[0].shader);
    CreateLight(LIGHT_POINT, (Vector3){ 0.0f, LIGHT_HEIGHT, LIGHT_DISTANCE }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 0, 255, 0, 255 }, model.materials[0].shader);
    CreateLight(LIGHT_POINT, (Vector3){ -LIGHT_DISTANCE, LIGHT_HEIGHT, 0.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 0, 0, 255, 255 }, model.materials[0].shader);
    CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 0.0f, LIGHT_HEIGHT*2.0f, -LIGHT_DISTANCE }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 255, 0, 255, 255 }, model.materials[0].shader);

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
        SetShaderValue(model.materials[0].shader, model.materials[0].shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
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
    UnloadMaterial(model.materials[0]); // Unload material: shader and textures

    UnloadModel(model);         // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Load PBR material (Supports: ALBEDO, NORMAL, METALNESS, ROUGHNESS, AO, EMMISIVE, HEIGHT maps)
// NOTE: PBR shader is loaded inside this function
static Material LoadMaterialPBR(Color albedo, float metalness, float roughness)
{
    Material mat = LoadMaterialDefault();   // Initialize material to default

    // Load PBR shader (requires several maps)
    mat.shader = LoadShader(TextFormat("resources/shaders/glsl%i/pbr.vs", GLSL_VERSION),
                            TextFormat("resources/shaders/glsl%i/pbr.fs", GLSL_VERSION));

    // Get required locations points for PBR material
    // NOTE: Those location names must be available and used in the shader code
    mat.shader.locs[SHADER_LOC_MAP_ALBEDO] = GetShaderLocation(mat.shader, "albedo.sampler");
    mat.shader.locs[SHADER_LOC_MAP_METALNESS] = GetShaderLocation(mat.shader, "metalness.sampler");
    mat.shader.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(mat.shader, "normals.sampler");
    mat.shader.locs[SHADER_LOC_MAP_ROUGHNESS] = GetShaderLocation(mat.shader, "roughness.sampler");
    mat.shader.locs[SHADER_LOC_MAP_OCCLUSION] = GetShaderLocation(mat.shader, "occlusion.sampler");
    //mat.shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(mat.shader, "emission.sampler");
    //mat.shader.locs[SHADER_LOC_MAP_HEIGHT] = GetShaderLocation(mat.shader, "height.sampler");
    mat.shader.locs[SHADER_LOC_MAP_IRRADIANCE] = GetShaderLocation(mat.shader, "irradianceMap");
    mat.shader.locs[SHADER_LOC_MAP_PREFILTER] = GetShaderLocation(mat.shader, "prefilterMap");
    mat.shader.locs[SHADER_LOC_MAP_BRDF] = GetShaderLocation(mat.shader, "brdfLUT");

    // Set view matrix location
    mat.shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(mat.shader, "matModel");
    //mat.shader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(mat.shader, "view");
    mat.shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(mat.shader, "viewPos");

    // Set PBR standard maps
    mat.maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("resources/pbr/trooper_albedo.png");
    mat.maps[MATERIAL_MAP_NORMAL].texture = LoadTexture("resources/pbr/trooper_normals.png");
    mat.maps[MATERIAL_MAP_METALNESS].texture = LoadTexture("resources/pbr/trooper_metalness.png");
    mat.maps[MATERIAL_MAP_ROUGHNESS].texture = LoadTexture("resources/pbr/trooper_roughness.png");
    mat.maps[MATERIAL_MAP_OCCLUSION].texture = LoadTexture("resources/pbr/trooper_ao.png");

    // Set textures filtering for better quality
    SetTextureFilter(mat.maps[MATERIAL_MAP_ALBEDO].texture, FILTER_BILINEAR);
    SetTextureFilter(mat.maps[MATERIAL_MAP_NORMAL].texture, FILTER_BILINEAR);
    SetTextureFilter(mat.maps[MATERIAL_MAP_METALNESS].texture, FILTER_BILINEAR);
    SetTextureFilter(mat.maps[MATERIAL_MAP_ROUGHNESS].texture, FILTER_BILINEAR);
    SetTextureFilter(mat.maps[MATERIAL_MAP_OCCLUSION].texture, FILTER_BILINEAR);

    // Enable sample usage in shader for assigned textures
    SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "albedo.useSampler"), (int[1]){ 1 }, SHADER_UNIFORM_INT);
    SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "normals.useSampler"), (int[1]){ 1 }, SHADER_UNIFORM_INT);
    SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "metalness.useSampler"), (int[1]){ 1 }, SHADER_UNIFORM_INT);
    SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "roughness.useSampler"), (int[1]){ 1 }, SHADER_UNIFORM_INT);
    SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "occlusion.useSampler"), (int[1]){ 1 }, SHADER_UNIFORM_INT);

    int renderModeLoc = GetShaderLocation(mat.shader, "renderMode");
    SetShaderValue(mat.shader, renderModeLoc, (int[1]){ 0 }, SHADER_UNIFORM_INT);

    // Set up material properties color
    mat.maps[MATERIAL_MAP_ALBEDO].color = albedo;
    mat.maps[MATERIAL_MAP_NORMAL].color = (Color){ 128, 128, 255, 255 };
    mat.maps[MATERIAL_MAP_METALNESS].value = metalness;
    mat.maps[MATERIAL_MAP_ROUGHNESS].value = roughness;
    mat.maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;
    mat.maps[MATERIAL_MAP_EMISSION].value = 0.5f;
    mat.maps[MATERIAL_MAP_HEIGHT].value = 0.5f;

    // Generate cubemap from panorama texture
    //--------------------------------------------------------------------------------------------------------
    Texture2D panorama = LoadTexture("resources/dresden_square_2k.hdr");

    // Load equirectangular to cubemap shader
    Shader shdrCubemap = LoadShader(TextFormat("resources/shaders/glsl%i/pbr.vs", GLSL_VERSION),
                                    TextFormat("resources/shaders/glsl%i/pbr.fs", GLSL_VERSION));

    SetShaderValue(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"), (int[1]){ 0 }, SHADER_UNIFORM_INT);
    TextureCubemap cubemap = GenTextureCubemap(shdrCubemap, panorama, CUBEMAP_SIZE, PIXELFORMAT_UNCOMPRESSED_R32G32B32);
    UnloadTexture(panorama);
    UnloadShader(shdrCubemap);
    //--------------------------------------------------------------------------------------------------------

    // Generate irradiance map from cubemap texture
    //--------------------------------------------------------------------------------------------------------
    // Load irradiance (GI) calculation shader
    Shader shdrIrradiance = LoadShader(TextFormat("resources/shaders/glsl%i/skybox.vs", GLSL_VERSION),
                                       TextFormat("resources/shaders/glsl%i/irradiance.fs", GLSL_VERSION));

    SetShaderValue(shdrIrradiance, GetShaderLocation(shdrIrradiance, "environmentMap"), (int[1]){ 0 }, SHADER_UNIFORM_INT);
    mat.maps[MATERIAL_MAP_IRRADIANCE].texture = GenTextureIrradiance(shdrIrradiance, cubemap, IRRADIANCE_SIZE);
    UnloadShader(shdrIrradiance);
    //--------------------------------------------------------------------------------------------------------

    // Generate prefilter map from cubemap texture
    //--------------------------------------------------------------------------------------------------------
    // Load reflection prefilter calculation shader
    Shader shdrPrefilter = LoadShader(TextFormat("resources/shaders/glsl%i/skybox.vs", GLSL_VERSION),
                                      TextFormat("resources/shaders/glsl%i/prefilter.fs", GLSL_VERSION));

    SetShaderValue(shdrPrefilter, GetShaderLocation(shdrPrefilter, "environmentMap"), (int[1]){ 0 }, SHADER_UNIFORM_INT);
    mat.maps[MATERIAL_MAP_PREFILTER].texture = GenTexturePrefilter(shdrPrefilter, cubemap, PREFILTERED_SIZE);
    UnloadTexture(cubemap);
    UnloadShader(shdrPrefilter);
    //--------------------------------------------------------------------------------------------------------

    // Generate BRDF (bidirectional reflectance distribution function) texture (using shader)
    //--------------------------------------------------------------------------------------------------------
    Shader shdrBRDF = LoadShader(TextFormat("resources/shaders/glsl%i/brdf.vs", GLSL_VERSION),
                                 TextFormat("resources/shaders/glsl%i/brdf.fs", GLSL_VERSION));

    mat.maps[MATERIAL_MAP_BRDG].texture = GenTextureBRDF(shdrBRDF, BRDF_SIZE);
    UnloadShader(shdrBRDF);
    //--------------------------------------------------------------------------------------------------------

    return mat;
}

// Texture maps generation (PBR)
//-------------------------------------------------------------------------------------------
// Generate cubemap texture from HDR texture
static TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format)
{
    TextureCubemap cubemap = { 0 };

    rlDisableBackfaceCulling();     // Disable backface culling to render inside the cube

    // STEP 1: Setup framebuffer
    //------------------------------------------------------------------------------------------
    unsigned int rbo = rlLoadTextureDepth(size, size, true);
    cubemap.id = rlLoadTextureCubemap(NULL, size, format);

    unsigned int fbo = rlLoadFramebuffer(size, size);
    rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
    rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

    // Check if framebuffer is complete with attachments (valid)
    if (rlFramebufferComplete(fbo)) TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", fbo);
    //------------------------------------------------------------------------------------------

    // STEP 2: Draw to framebuffer
    //------------------------------------------------------------------------------------------
    // NOTE: Shader is used to convert HDR equirectangular environment map to cubemap equivalent (6 faces)
    rlEnableShader(shader.id);

    // Define projection matrix and send it to shader
    Matrix matFboProjection = MatrixPerspective(90.0*DEG2RAD, 1.0, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

    // Define view matrix for every side of the cubemap
    Matrix fboViews[6] = {
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  1.0f,  0.0f,  0.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ -1.0f,  0.0f,  0.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  1.0f,  0.0f }, (Vector3){ 0.0f,  0.0f,  1.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f, -1.0f,  0.0f }, (Vector3){ 0.0f,  0.0f, -1.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  0.0f,  1.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  0.0f, -1.0f }, (Vector3){ 0.0f, -1.0f,  0.0f })
    };

    rlViewport(0, 0, size, size);   // Set viewport to current fbo dimensions

    for (int i = 0; i < 6; i++)
    {
        rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);
        rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);

        rlEnableFramebuffer(fbo);
        rlSetTexture(panorama.id);   // WARNING: It must be called after enabling current framebuffer if using internal batch system!

        rlClearScreenBuffers();
        DrawCubeV(Vector3Zero(), Vector3One(), WHITE);
        rlDrawRenderBatchActive();
    }
    //------------------------------------------------------------------------------------------

    // STEP 3: Unload framebuffer and reset state
    //------------------------------------------------------------------------------------------
    rlDisableShader();          // Unbind shader
    rlDisableTexture();         // Unbind texture
    rlDisableFramebuffer();     // Unbind framebuffer
    rlUnloadFramebuffer(fbo);   // Unload framebuffer (and automatically attached depth texture/renderbuffer)

    // Reset viewport dimensions to default
    rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
    rlEnableBackfaceCulling();
    //------------------------------------------------------------------------------------------

    cubemap.width = size;
    cubemap.height = size;
    cubemap.mipmaps = 1;
    cubemap.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32;

    return cubemap;
}

// Generate irradiance texture using cubemap data
static TextureCubemap GenTextureIrradiance(Shader shader, TextureCubemap cubemap, int size)
{
    TextureCubemap irradiance = { 0 };

    rlDisableBackfaceCulling();     // Disable backface culling to render inside the cube

    // STEP 1: Setup framebuffer
    //------------------------------------------------------------------------------------------
    unsigned int rbo = rlLoadTextureDepth(size, size, true);
    irradiance.id = rlLoadTextureCubemap(NULL, size, PIXELFORMAT_UNCOMPRESSED_R32G32B32);

    unsigned int fbo = rlLoadFramebuffer(size, size);
    rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
    rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);
    //------------------------------------------------------------------------------------------

    // STEP 2: Draw to framebuffer
    //------------------------------------------------------------------------------------------
    // NOTE: Shader is used to solve diffuse integral by convolution to create an irradiance cubemap
    rlEnableShader(shader.id);

    // Define projection matrix and send it to shader
    Matrix matFboProjection = MatrixPerspective(90.0*DEG2RAD, 1.0, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

    // Define view matrix for every side of the cubemap
    Matrix fboViews[6] = {
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  1.0f,  0.0f,  0.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ -1.0f,  0.0f,  0.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  1.0f,  0.0f }, (Vector3){ 0.0f,  0.0f,  1.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f, -1.0f,  0.0f }, (Vector3){ 0.0f,  0.0f, -1.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  0.0f,  1.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  0.0f, -1.0f }, (Vector3){ 0.0f, -1.0f,  0.0f })
    };

    rlActiveTextureSlot(0);
    rlEnableTextureCubemap(cubemap.id);

    rlViewport(0, 0, size, size);   // Set viewport to current fbo dimensions

    for (int i = 0; i < 6; i++)
    {
        rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);
        rlFramebufferAttach(fbo, irradiance.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);

        rlEnableFramebuffer(fbo);
        rlClearScreenBuffers();
        rlLoadDrawCube();
    }
    //------------------------------------------------------------------------------------------

    // STEP 3: Unload framebuffer and reset state
    //------------------------------------------------------------------------------------------
    rlDisableShader();          // Unbind shader
    rlDisableTexture();         // Unbind texture
    rlDisableFramebuffer();     // Unbind framebuffer
    rlUnloadFramebuffer(fbo);   // Unload framebuffer (and automatically attached depth texture/renderbuffer)

    // Reset viewport dimensions to default
    rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
    rlEnableBackfaceCulling();
    //------------------------------------------------------------------------------------------

    irradiance.width = size;
    irradiance.height = size;
    irradiance.mipmaps = 1;
    irradiance.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32;

    return irradiance;
}

// Generate prefilter texture using cubemap data
static TextureCubemap GenTexturePrefilter(Shader shader, TextureCubemap cubemap, int size)
{
    TextureCubemap prefilter = { 0 };

    rlDisableBackfaceCulling();     // Disable backface culling to render inside the cube

    // STEP 1: Setup framebuffer
    //------------------------------------------------------------------------------------------
    unsigned int rbo = rlLoadTextureDepth(size, size, true);
    prefilter.id = rlLoadTextureCubemap(NULL, size, PIXELFORMAT_UNCOMPRESSED_R32G32B32);
    rlTextureParameters(prefilter.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_MIP_LINEAR);

    unsigned int fbo = rlLoadFramebuffer(size, size);
    rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
    rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);
    //------------------------------------------------------------------------------------------

    // Generate mipmaps for the prefiltered HDR texture
    //glGenerateMipmap(GL_TEXTURE_CUBE_MAP);    // TODO!

    // STEP 2: Draw to framebuffer
    //------------------------------------------------------------------------------------------
    // NOTE: Shader is used to prefilter HDR and store data into mipmap levels

    // Define projection matrix and send it to shader
    Matrix fboProjection = MatrixPerspective(90.0*DEG2RAD, 1.0, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    rlEnableShader(shader.id);
    rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_PROJECTION], fboProjection);

    // Define view matrix for every side of the cubemap
    Matrix fboViews[6] = {
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  1.0f,  0.0f,  0.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ -1.0f,  0.0f,  0.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  1.0f,  0.0f }, (Vector3){ 0.0f,  0.0f,  1.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f, -1.0f,  0.0f }, (Vector3){ 0.0f,  0.0f, -1.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  0.0f,  1.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  0.0f, -1.0f }, (Vector3){ 0.0f, -1.0f,  0.0f })
    };

    rlActiveTextureSlot(0);
    rlEnableTextureCubemap(cubemap.id);

    // TODO: Locations should be taken out of this function... too shader dependant...
    int roughnessLoc = rlGetLocationUniform(shader.id, "roughness");

    rlEnableFramebuffer(fbo);

    #define MAX_MIPMAP_LEVELS   5   // Max number of prefilter texture mipmaps

    for (int mip = 0; mip < MAX_MIPMAP_LEVELS; mip++)
    {
        // Resize framebuffer according to mip-level size.
        unsigned int mipWidth  = size*(int)powf(0.5f, (float)mip);
        unsigned int mipHeight = size*(int)powf(0.5f, (float)mip);

        rlViewport(0, 0, mipWidth, mipHeight);

        //glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);

        float roughness = (float)mip/(float)(MAX_MIPMAP_LEVELS - 1);
        rlSetUniform(roughnessLoc, &roughness, SHADER_UNIFORM_FLOAT, 1);

        for (int i = 0; i < 6; i++)
        {
            rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);
            rlFramebufferAttach(fbo, prefilter.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, mip);

            rlClearScreenBuffers();
            rlLoadDrawCube();
        }
    }
    //------------------------------------------------------------------------------------------

    // STEP 3: Unload framebuffer and reset state
    //------------------------------------------------------------------------------------------
    rlDisableShader();          // Unbind shader
    rlDisableTexture();         // Unbind texture
    rlDisableFramebuffer();     // Unbind framebuffer
    rlUnloadFramebuffer(fbo);   // Unload framebuffer (and automatically attached depth texture/renderbuffer)

    // Reset viewport dimensions to default
    rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
    rlEnableBackfaceCulling();
    //------------------------------------------------------------------------------------------

    prefilter.width = size;
    prefilter.height = size;
    prefilter.mipmaps = MAX_MIPMAP_LEVELS;
    prefilter.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32;

    return prefilter;
}

// Generate BRDF texture using cubemap data
// TODO: Review implementation: https://github.com/HectorMF/BRDFGenerator
static Texture2D GenTextureBRDF(Shader shader, int size)
{
    Texture2D brdf = { 0 };

    // STEP 1: Setup framebuffer
    //------------------------------------------------------------------------------------------
    unsigned int rbo = rlLoadTextureDepth(size, size, true);
    brdf.id = rlLoadTexture(NULL, size, size, PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);

    unsigned int fbo = rlLoadFramebuffer(size, size);
    rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
    rlFramebufferAttach(fbo, brdf.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    //------------------------------------------------------------------------------------------

    // STEP 2: Draw to framebuffer
    //------------------------------------------------------------------------------------------
    // NOTE: Render BRDF LUT into a quad using FBO
    rlEnableShader(shader.id);

    rlViewport(0, 0, size, size);

    rlEnableFramebuffer(fbo);
    rlClearScreenBuffers();

    rlLoadDrawQuad();
    //------------------------------------------------------------------------------------------

    // STEP 3: Unload framebuffer and reset state
    //------------------------------------------------------------------------------------------
    rlDisableShader();          // Unbind shader
    rlDisableTexture();         // Unbind texture
    rlDisableFramebuffer();     // Unbind framebuffer
    rlUnloadFramebuffer(fbo);   // Unload framebuffer (and automatically attached depth texture/renderbuffer)

    // Reset viewport dimensions to default
    rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
    //------------------------------------------------------------------------------------------

    brdf.width = size;
    brdf.height = size;
    brdf.mipmaps = 1;
    brdf.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32;

    return brdf;
}
