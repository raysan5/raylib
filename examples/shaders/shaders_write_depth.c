/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Example originally created with raylib 4.2, last time updated with raylib 4.2
*
*   Example contributed by Buğra Alptekin Sarı (@BugraAlptekinSari) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022 Buğra Alptekin Sarı (@BugraAlptekinSari)
*
********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif
//------------------------------------------------------------------------------------
// Customized render texture function to create a writable render buffer
RenderTexture2D LoadRenderTextureMOD(int width, int height)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(width, height);   // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(0, width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        target.texture.mipmaps = 1;

        // Create depth rendertexture
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}

// Unload render texture from GPU memory (VRAM)
void UnloadRenderTextureMOD(RenderTexture2D target)
{
    if (target.id > 0)
    {
        // Color texture attached to FBO is deleted
        rlUnloadTexture(target.texture.id);
        rlUnloadTexture(target.depth.id);

        // NOTE: Depth texture/renderbuffer is automatically
        // queried and deleted before deleting framebuffer
        rlUnloadFramebuffer(target.id);
    }
}
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    // The shader inverst the depth buffer by writing into it by `gl_FragDepth = 1 - gl_FragCoord.z;`
    Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/write_depth.fs", GLSL_VERSION));

    //Use Customized function to create writable depth buffer
    RenderTexture2D target = LoadRenderTextureMOD(screenWidth, screenHeight);

    Camera camera = {    // Define the camera to look into our 3d world
        .position = (Vector3){ 2.0f, 2.0f, 3.0f },    // Camera position
        .target = (Vector3){ 0.0f, 0.5f, 0.0f },      // Camera looking at point
        .up = (Vector3){ 0.0f, 1.0f, 0.0f },          // Camera up vector (rotation towards target)
        .fovy = 45.0f,                                // Camera field-of-view Y
        .projection = CAMERA_PERSPECTIVE             // Camera mode type
    };
    SetCameraMode(camera, CAMERA_ORBITAL);
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);
        // Draw
        //----------------------------------------------------------------------------------
        // Draw FBO
        BeginTextureMode(target);
        ClearBackground(WHITE);
        BeginMode3D(camera);
        BeginShaderMode(shader);
        DrawCubeWiresV((Vector3) { 0.0f, 0.5f, 1.0f }, (Vector3) { 1.0f, 1.0f, 1.0f }, RED);
        DrawCubeV((Vector3) { 0.0f, 0.5f, 1.0f }, (Vector3) { 1.0f, 1.0f, 1.0f }, PURPLE);
        DrawCubeWiresV((Vector3) { 0.0f, 0.5f, -1.0f }, (Vector3) { 1.0f, 1.0f, 1.0f }, DARKGREEN);
        DrawCubeV((Vector3) { 0.0f, 0.5f, -1.0f }, (Vector3) { 1.0f, 1.0f, 1.0f }, YELLOW);
        DrawGrid(10, 1.0f);
        EndShaderMode();
        EndMode3D();
        EndTextureMode();

        // Draw Screen
        BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawTextureRec(target.texture, (Rectangle) { 0, 0, screenWidth, -screenHeight }, (Vector2) { 0, 0 }, WHITE);
        DrawFPS(0, 0);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTextureMOD(target);
    UnloadShader(shader);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
