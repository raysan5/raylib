/*******************************************************************************************
*
*   raylib [shaders] example - Apply a postprocessing shader to a scene
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3), to test this example
*         on OpenGL ES 2.0 platforms (Android, Raspberry Pi, HTML5), use #version 100 shaders
*         raylib comes with shaders ready for both versions, check raylib/shaders install folder
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
    #define DEFAULT_VERTEX_SHADER   "resources/shaders/glsl330/base.vs"
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
    #define DEFAULT_VERTEX_SHADER   "resources/shaders/glsl100/base.vs"
#endif

#define MAX_POSTPRO_SHADERS         12

typedef enum {
    FX_GRAYSCALE = 0,
    FX_POSTERIZATION,
    FX_DREAM_VISION,
    FX_PIXELIZER,
    FX_CROSS_HATCHING,
    FX_CROSS_STITCHING,
    FX_PREDATOR_VIEW,
    FX_SCANLINES,
    FX_FISHEYE,
    FX_SOBEL,
    FX_BLOOM,
    FX_BLUR,
    //FX_FXAA
} PostproShader;

static const char *postproShaderText[] = {
    "GRAYSCALE",
    "POSTERIZATION",
    "DREAM_VISION",
    "PIXELIZER",
    "CROSS_HATCHING",
    "CROSS_STITCHING",
    "PREDATOR_VIEW",
    "SCANLINES",
    "FISHEYE",
    "SOBEL",
    "BLOOM",
    "BLUR",
    //"FXAA"
};

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);      // Enable Multi Sampling Anti Aliasing 4x (if available)

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - postprocessing shader");

    // Define the camera to look into our 3d world
    Camera camera = {{ 3.0f, 3.0f, 3.0f }, { 0.0f, 1.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };
    
    Model dwarf = LoadModel("resources/model/dwarf.obj");                   // Load OBJ model
    Texture2D texture = LoadTexture("resources/model/dwarf_diffuse.png");   // Load model texture (diffuse map)
    dwarf.material.maps[MAP_DIFFUSE].texture = texture;          // Set dwarf model diffuse texture

    Vector3 position = { 0.0f, 0.0f, 0.0f };                                // Set model position
    
    // Load all postpro shaders
    // NOTE 1: All postpro shader use the base vertex shader (DEFAULT_VERTEX_SHADER)
    // NOTE 2: We load the correct shader depending on GLSL version
    Shader shaders[MAX_POSTPRO_SHADERS];
    
    shaders[FX_GRAYSCALE] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/grayscale.fs", GLSL_VERSION));
    shaders[FX_POSTERIZATION] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/posterization.fs", GLSL_VERSION));
    shaders[FX_DREAM_VISION] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/dream_vision.fs", GLSL_VERSION));
    shaders[FX_PIXELIZER] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/pixelizer.fs", GLSL_VERSION));
    shaders[FX_CROSS_HATCHING] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/cross_hatching.fs", GLSL_VERSION));
    shaders[FX_CROSS_STITCHING] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/cross_stitching.fs", GLSL_VERSION));
    shaders[FX_PREDATOR_VIEW] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/predator.fs", GLSL_VERSION));
    shaders[FX_SCANLINES] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/scanlines.fs", GLSL_VERSION));
    shaders[FX_FISHEYE] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/fisheye.fs", GLSL_VERSION));
    shaders[FX_SOBEL] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/sobel.fs", GLSL_VERSION));
    shaders[FX_BLOOM] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/bloom.fs", GLSL_VERSION));
    shaders[FX_BLUR] = LoadShader(DEFAULT_VERTEX_SHADER, FormatText("resources/shaders/glsl%i/blur.fs", GLSL_VERSION));
    
    int currentShader = FX_GRAYSCALE;

    // Create a RenderTexture2D to be used for render to texture
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    
    // Setup orbital camera
    SetCameraMode(camera, CAMERA_ORBITAL);  // Set an orbital camera mode

    SetTargetFPS(60);                       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);              // Update camera
        
        if (IsKeyPressed(KEY_RIGHT)) currentShader++;
        else if (IsKeyPressed(KEY_LEFT)) currentShader--;
        
        if (currentShader >= MAX_POSTPRO_SHADERS) currentShader = 0;
        else if (currentShader < 0) currentShader = MAX_POSTPRO_SHADERS - 1;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginTextureMode(target);   // Enable drawing to texture
            
                Begin3dMode(camera);

                    DrawModel(dwarf, position, 2.0f, WHITE);   // Draw 3d model with texture

                    DrawGrid(10, 1.0f);     // Draw a grid

                End3dMode();
                
            EndTextureMode();           // End drawing to texture (now we have a texture available for next passes)
            
            // Render previously generated texture using selected postpro shader
            BeginShaderMode(shaders[currentShader]);
            
                // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
                DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, -target.texture.height }, (Vector2){ 0, 0 }, WHITE);
                
            EndShaderMode();
            
            DrawRectangle(0, 9, 580, 30, Fade(LIGHTGRAY, 0.7f));
            
            DrawText("(c) Dwarf 3D model by David Moreno", screenWidth - 200, screenHeight - 20, 10, DARKGRAY);
            
            DrawText("CURRENT POSTPRO SHADER:", 10, 15, 20, BLACK);
            DrawText(postproShaderText[currentShader], 330, 15, 20, RED);
            DrawText("< >", 540, 10, 30, DARKBLUE);
            
            DrawFPS(700, 15);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // Unload all postpro shaders
    for (int i = 0; i < MAX_POSTPRO_SHADERS; i++) UnloadShader(shaders[i]);
    
    UnloadTexture(texture);         // Unload texture
    UnloadModel(dwarf);             // Unload model
    UnloadRenderTexture(target);    // Unload render texture

    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}