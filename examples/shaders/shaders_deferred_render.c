/*******************************************************************************************
*
*   raylib [shaders] example - deferred rendering
*
*   NOTE: This example requires raylib OpenGL 3.3 or OpenGL ES 3.0
*
*   Example originally created with raylib 4.5, last time updated with raylib 4.5
*
*   Example contributed by Justin Andreas Lacoste (@27justin) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023 Justin Andreas Lacoste (@27justin)
*
********************************************************************************************/

#include "raylib.h"

#include "rlgl.h"
#include "raymath.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#include <stdlib.h>         // Required for: NULL

#define MAX_CUBES   30

typedef struct GBuffer {
    unsigned int framebuffer;

    unsigned int positionTexture;
    unsigned int normalTexture;
    unsigned int albedoSpecTexture;
    
    unsigned int depthRenderbuffer;
} GBuffer;

typedef enum {
   DEFERRED_POSITION,
   DEFERRED_NORMAL,
   DEFERRED_ALBEDO,
   DEFERRED_SHADING
} DeferredMode;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    // -------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - deferred render");

    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 4.0f, 5.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    // Load plane model from a generated mesh
    Model model = LoadModelFromMesh(GenMeshPlane(10.0f, 10.0f, 3, 3));
    Model cube = LoadModelFromMesh(GenMeshCube(2.0f, 2.0f, 2.0f));

    // Load geometry buffer (G-buffer) shader and deferred shader
    Shader gbufferShader = LoadShader("resources/shaders/glsl330/gbuffer.vs",
                               "resources/shaders/glsl330/gbuffer.fs");

    Shader deferredShader = LoadShader("resources/shaders/glsl330/deferred_shading.vs",
                               "resources/shaders/glsl330/deferred_shading.fs");
    deferredShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(deferredShader, "viewPosition");

    // Initialize the G-buffer
    GBuffer gBuffer = { 0 };
    gBuffer.framebuffer = rlLoadFramebuffer(screenWidth, screenHeight);

    if (!gBuffer.framebuffer)
    {
        TraceLog(LOG_WARNING, "Failed to create framebuffer");
        exit(1);
    }
    
    rlEnableFramebuffer(gBuffer.framebuffer);

    // Since we are storing position and normal data in these textures, 
    // we need to use a floating point format.
    gBuffer.positionTexture = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);

    gBuffer.normalTexture = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
    // Albedo (diffuse color) and specular strength can be combined into one texture.
    // The color in RGB, and the specular strength in the alpha channel.
    gBuffer.albedoSpecTexture = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);

    // Activate the draw buffers for our framebuffer
    rlActiveDrawBuffers(3);

    // Now we attach our textures to the framebuffer.
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.positionTexture, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.normalTexture, RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.albedoSpecTexture, RL_ATTACHMENT_COLOR_CHANNEL2, RL_ATTACHMENT_TEXTURE2D, 0);

    // Finally we attach the depth buffer.
    gBuffer.depthRenderbuffer = rlLoadTextureDepth(screenWidth, screenHeight, true);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.depthRenderbuffer, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

    // Make sure our framebuffer is complete.
    // NOTE: rlFramebufferComplete() automatically unbinds the framebuffer, so we don't have
    // to rlDisableFramebuffer() here.
    if (!rlFramebufferComplete(gBuffer.framebuffer))
    {
        TraceLog(LOG_WARNING, "Framebuffer is not complete");
        exit(1);
    }

    // Now we initialize the sampler2D uniform's in the deferred shader.
    // We do this by setting the uniform's value to the color channel slot we earlier
    // bound our textures to.
    rlEnableShader(deferredShader.id);

        rlSetUniformSampler(rlGetLocationUniform(deferredShader.id, "gPosition"), 0);
        rlSetUniformSampler(rlGetLocationUniform(deferredShader.id, "gNormal"), 1);
        rlSetUniformSampler(rlGetLocationUniform(deferredShader.id, "gAlbedoSpec"), 2);

    rlDisableShader();

    // Assign out lighting shader to model
    model.materials[0].shader = gbufferShader;
    cube.materials[0].shader = gbufferShader;

    // Create lights
    //--------------------------------------------------------------------------------------
    Light lights[MAX_LIGHTS] = { 0 };
    lights[0] = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, -2 }, Vector3Zero(), YELLOW, deferredShader);
    lights[1] = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, 2 }, Vector3Zero(), RED, deferredShader);
    lights[2] = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, 2 }, Vector3Zero(), GREEN, deferredShader);
    lights[3] = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, -2 }, Vector3Zero(), BLUE, deferredShader);

    const float CUBE_SCALE = 0.25;
    Vector3 cubePositions[MAX_CUBES] = { 0 };
    float cubeRotations[MAX_CUBES] = { 0 };
    
    for (int i = 0; i < MAX_CUBES; i++)
    {
        cubePositions[i] = (Vector3){
            .x = (float)(rand()%10) - 5,
            .y = (float)(rand()%5),
            .z = (float)(rand()%10) - 5,
        };
        
        cubeRotations[i] = (float)(rand()%360);
    }

    DeferredMode mode = DEFERRED_SHADING;

    rlEnableDepthTest();

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);

        // Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(deferredShader, deferredShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        
        // Check key inputs to enable/disable lights
        if (IsKeyPressed(KEY_Y)) { lights[0].enabled = !lights[0].enabled; }
        if (IsKeyPressed(KEY_R)) { lights[1].enabled = !lights[1].enabled; }
        if (IsKeyPressed(KEY_G)) { lights[2].enabled = !lights[2].enabled; }
        if (IsKeyPressed(KEY_B)) { lights[3].enabled = !lights[3].enabled; }

        // Check key inputs to switch between G-buffer textures
        if (IsKeyPressed(KEY_ONE)) mode = DEFERRED_POSITION;
        if (IsKeyPressed(KEY_TWO)) mode = DEFERRED_NORMAL;
        if (IsKeyPressed(KEY_THREE)) mode = DEFERRED_ALBEDO;
        if (IsKeyPressed(KEY_FOUR)) mode = DEFERRED_SHADING;

        // Update light values (actually, only enable/disable them)
        for (int i = 0; i < MAX_LIGHTS; i++) UpdateLightValues(deferredShader, lights[i]);
        //----------------------------------------------------------------------------------

        // Draw
        // ---------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
        
            // Draw to the geometry buffer by first activating it
            rlEnableFramebuffer(gBuffer.framebuffer);
            rlClearScreenBuffers();  // Clear color and depth buffer
            
            rlDisableColorBlend();
            BeginMode3D(camera);
                // NOTE: We have to use rlEnableShader here. `BeginShaderMode` or thus `rlSetShader`
                // will not work, as they won't immediately load the shader program.
                rlEnableShader(gbufferShader.id);
                    // When drawing a model here, make sure that the material's shaders
                    // are set to the gbuffer shader!
                    DrawModel(model, Vector3Zero(), 1.0f, WHITE);
                    DrawModel(cube, (Vector3) { 0.0, 1.0f, 0.0 }, 1.0f, WHITE);

                    for (int i = 0; i < MAX_CUBES; i++)
                    {
                        Vector3 position = cubePositions[i];
                        DrawModelEx(cube, position, (Vector3) { 1, 1, 1 }, cubeRotations[i], (Vector3) { CUBE_SCALE, CUBE_SCALE, CUBE_SCALE }, WHITE);
                    }

                rlDisableShader();
            EndMode3D();
            rlEnableColorBlend();

            // Go back to the default framebuffer (0) and draw our deferred shading.
            rlDisableFramebuffer();
            rlClearScreenBuffers(); // Clear color & depth buffer

            switch (mode)
            {
                case DEFERRED_SHADING:
                {
                    BeginMode3D(camera);
                        rlDisableColorBlend();
                        rlEnableShader(deferredShader.id);
                            // Activate our g-buffer textures
                            // These will now be bound to the sampler2D uniforms `gPosition`, `gNormal`,
                            // and `gAlbedoSpec`
                            rlActiveTextureSlot(0);
                            rlEnableTexture(gBuffer.positionTexture);
                            rlActiveTextureSlot(1);
                            rlEnableTexture(gBuffer.normalTexture);
                            rlActiveTextureSlot(2);
                            rlEnableTexture(gBuffer.albedoSpecTexture);

                            // Finally, we draw a fullscreen quad to our default framebuffer
                            // This will now be shaded using our deferred shader
                            rlLoadDrawQuad();
                        rlDisableShader();
                        rlEnableColorBlend();
                    EndMode3D();

                    // As a last step, we now copy over the depth buffer from our g-buffer to the default framebuffer.
                    rlEnableFramebuffer(gBuffer.framebuffer); //glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.framebuffer);
                    rlEnableFramebuffer(0); //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                    rlBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, 0x00000100);    // GL_DEPTH_BUFFER_BIT
                    rlDisableFramebuffer();

                    // Since our shader is now done and disabled, we can draw our lights in default
                    // forward rendering
                    BeginMode3D(camera);
                        rlEnableShader(rlGetShaderIdDefault());
                            for(int i = 0; i < MAX_LIGHTS; i++)
                            {
                                if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
                                else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
                            }
                        rlDisableShader();
                    EndMode3D();
                    DrawText("FINAL RESULT", 10, screenHeight - 30, 20, DARKGREEN);
                } break;
                
                case DEFERRED_POSITION:
                {
                    DrawTextureRec((Texture2D){
                        .id = gBuffer.positionTexture,
                        .width = screenWidth,
                        .height = screenHeight,
                    }, (Rectangle) { 0, 0, screenWidth, -screenHeight }, Vector2Zero(), RAYWHITE);
                    DrawText("POSITION TEXTURE", 10, screenHeight - 30, 20, DARKGREEN);
                } break;
                
                case DEFERRED_NORMAL:
                {
                    DrawTextureRec((Texture2D){
                        .id = gBuffer.normalTexture,
                        .width = screenWidth,
                        .height = screenHeight,
                    }, (Rectangle) { 0, 0, screenWidth, -screenHeight }, Vector2Zero(), RAYWHITE);
                    DrawText("NORMAL TEXTURE", 10, screenHeight - 30, 20, DARKGREEN);
                } break;

                case DEFERRED_ALBEDO:
                {
                    DrawTextureRec((Texture2D){
                        .id = gBuffer.albedoSpecTexture,
                        .width = screenWidth,
                        .height = screenHeight,
                    }, (Rectangle) { 0, 0, screenWidth, -screenHeight }, Vector2Zero(), RAYWHITE);
                    DrawText("ALBEDO TEXTURE", 10, screenHeight - 30, 20, DARKGREEN);
                } break;
            }

            DrawText("Toggle lights keys: [Y][R][G][B]", 10, 40, 20, DARKGRAY);
            DrawText("Switch G-buffer textures: [1][2][3][4]", 10, 70, 20, DARKGRAY);

            DrawFPS(10, 10);
            
        EndDrawing();
        // -----------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);     // Unload the models
    UnloadModel(cube);

    UnloadShader(deferredShader);   // Unload shaders
    UnloadShader(gbufferShader);

    // Unload geometry buffer and all attached textures
    rlUnloadFramebuffer(gBuffer.framebuffer);
    rlUnloadTexture(gBuffer.positionTexture);
    rlUnloadTexture(gBuffer.normalTexture);
    rlUnloadTexture(gBuffer.albedoSpecTexture);
    rlUnloadTexture(gBuffer.depthRenderbuffer);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

