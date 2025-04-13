/*******************************************************************************************
*
*   raylib [shader] example - render depth texture
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by Luís Almeida (@luis605)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Luís Almeida (@luis605)
*
********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

RenderTexture2D LoadRenderTextureWithDepth(int width, int height);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shader] example - render depth texture");

    // Init camera
    Camera camera = { 0 };
    camera.position = (Vector3){ 4.0f, 1.0f, 5.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Load an empty render texture with a depth texture
    RenderTexture2D target = LoadRenderTextureWithDepth(screenWidth, screenHeight);

    // Load depth shader and get depth texture shader location
    Shader depthShader = LoadShader(0, TextFormat("resources/shaders/glsl%i/depth.fs", GLSL_VERSION));
    int depthLoc = GetShaderLocation(depthShader, "depthTexture");
    int flipTextureLoc = GetShaderLocation(depthShader, "flipY");
    SetShaderValue(depthShader, flipTextureLoc, (int[]){ 1 }, SHADER_UNIFORM_INT); // Flip Y texture

    // Load models
    Model cube = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    Model floor = LoadModelFromMesh(GenMeshPlane(20.0f, 20.0f, 1, 1));

    DisableCursor();  // Limit cursor to relative movement inside the window

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FREE);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginTextureMode(target);
            ClearBackground(WHITE);
            
            BeginMode3D(camera);
                DrawModel(cube,  (Vector3){ 0.0f, 0.0f, 0.0f },  3.0f, YELLOW);
                DrawModel(floor, (Vector3){ 10.0f, 0.0f, 2.0f }, 2.0f, RED);
            EndMode3D();
        EndTextureMode();

        BeginDrawing();

            BeginShaderMode(depthShader);
                SetShaderValueTexture(depthShader, depthLoc, target.depth);
                DrawTexture(target.depth, 0, 0, WHITE);
            EndShaderMode();

            DrawRectangle( 10, 10, 320, 93, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines( 10, 10, 320, 93, BLUE);

            DrawText("Camera Controls:", 20, 20, 10, BLACK);
            DrawText("- WASD to move", 40, 40, 10, DARKGRAY);
            DrawText("- Mouse Wheel Pressed to Pan", 40, 60, 10, DARKGRAY);
            DrawText("- Z to zoom to (0, 0, 0)", 40, 80, 10, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(cube);              // Unload model
    UnloadModel(floor);             // Unload model
    UnloadRenderTexture(target);    // Unload render texture
    UnloadShader(depthShader);      // Unload shader

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}

RenderTexture2D LoadRenderTextureWithDepth(int width, int height)
{
    RenderTexture2D target = {0};

    target.id = rlLoadFramebuffer();   // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(0, width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        target.texture.mipmaps = 1;

        // Create depth texture
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT? THIS DOESN'T EXIST IN RAYLIB
        target.depth.mipmaps = 1;

        // Attach color texture and depth texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}