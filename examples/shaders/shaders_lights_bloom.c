/*******************************************************************************************
*
*   raylib [shaders] example - forward multi-lighting with bloom
*
*   Example demonstrates forward multi-point lighting (8 point lights, attenuation and
*   Blinn-Phong specular) combined with a threshold-based bloom pass and Reinhard tone
*   mapping, applied as a full-screen post-process pass
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example uses resources/shaders/glsl100 and resources/shaders/glsl330, shared with the
*   rest of the shaders examples
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
*
*   Example contributed by PanicTitan (@PanicTitan) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 PanicTitan (@PanicTitan)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"
#include "rlgl.h"

#include <math.h>       // Required for: sinf(), cosf()

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//--------------------------------------------------------------------------------------
// Global Definitions
//--------------------------------------------------------------------------------------
#define MAX_LIGHTS      8

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - forward multi-lighting bloom");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 5.0f, 9.0f };
    camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);

    // Forward multi-light shader and bloom post-process shader, both loaded from the
    // resources folder shared with the rest of the shaders examples. If the GLSL version
    // guessed from the PLATFORM_DESKTOP build flag turns out to be wrong for whatever
    // raylib was built against, linking fails and silently falls back to the default
    // shader - so this retries once with the other version, and either way the result
    // is checked explicitly and reported on screen rather than staying silently wrong
    Shader lightShader = LoadShader(TextFormat("resources/shaders/glsl%i/lights_bloom.vs", GLSL_VERSION),
                                     TextFormat("resources/shaders/glsl%i/lights_bloom.fs", GLSL_VERSION));
    Shader bloomShader = LoadShader(0, TextFormat("resources/shaders/glsl%i/lights_bloom_post.fs", GLSL_VERSION));

    // Load models from generated cube mesh and plane
    // NOTE: Meshes are automatically unloaded on UnloadModel()
    Model cube = LoadModelFromMesh(GenMeshCube(2.0f, 2.0f, 2.0f));
    Model floor = LoadModelFromMesh(GenMeshPlane(14.0f, 14.0f, 1, 1));
    cube.materials[0].shader = lightShader;
    floor.materials[0].shader = lightShader;

    int lightPosLoc = GetShaderLocation(lightShader, "lightPositions");
    int lightColLoc = GetShaderLocation(lightShader, "lightColors");
    int viewPosLoc = GetShaderLocation(lightShader, "viewPos");

    Vector3 lightPositions[MAX_LIGHTS] = { 0 };
    Vector3 lightColors[MAX_LIGHTS] = {
        { 1.0f, 0.2f, 0.2f },   // Red
        { 0.2f, 1.0f, 0.3f },   // Green
        { 0.2f, 0.5f, 1.0f },   // Blue
        { 1.0f, 0.8f, 0.1f },   // Yellow
        { 1.0f, 0.1f, 0.8f },   // Magenta
        { 0.1f, 1.0f, 1.0f },   // Cyan
        { 1.0f, 0.4f, 0.1f },   // Orange
        { 0.7f, 0.2f, 1.0f },   // Purple
    };

    SetShaderValueV(lightShader, lightColLoc, lightColors, SHADER_UNIFORM_VEC3, MAX_LIGHTS);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);

        float time = (float)GetTime();

        // Orbital path for each point light, its own radius/height offset by index
        for (int i = 0; i < MAX_LIGHTS; i++)
        {
            float angle = (i/(float)MAX_LIGHTS)*2.0f*PI + time*0.6f;
            float radius = 4.2f + sinf(time*1.2f + i)*0.4f;
            float height = 1.0f + sinf(time*1.8f + i)*0.6f;

            lightPositions[i] = (Vector3){ sinf(angle)*radius, height, cosf(angle)*radius };
        }

        SetShaderValueV(lightShader, lightPosLoc, lightPositions, SHADER_UNIFORM_VEC3, MAX_LIGHTS);
        SetShaderValue(lightShader, viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        // Render the lit scene to an offscreen texture
        BeginTextureMode(target);

            ClearBackground((Color){ 12, 12, 18, 255 });

            BeginMode3D(camera);

                DrawModel(cube, (Vector3){ 0.0f, 1.0f, 0.0f }, 1.0f, GRAY);
                DrawModel(floor, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, DARKGRAY);
                DrawGrid(10, 1.0f);

                // Glowing bulbs mark each light's position
                for (int i = 0; i < MAX_LIGHTS; i++)
                {
                    Color bulbColor = (Color){
                        (unsigned char)(lightColors[i].x*255),
                        (unsigned char)(lightColors[i].y*255),
                        (unsigned char)(lightColors[i].z*255),
                        255 };

                    DrawSphere(lightPositions[i], 0.15f, bulbColor);
                }

            EndMode3D();

        EndTextureMode();

        // Present the offscreen texture through the bloom + tone mapping shader
        BeginDrawing();

            ClearBackground(BLACK);

            Rectangle srcRec = { 0, 0, (float)target.texture.width, (float)-target.texture.height };

            BeginShaderMode(bloomShader);
                // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
                DrawTextureRec(target.texture, srcRec, (Vector2){ 0, 0 }, WHITE);
            EndShaderMode();

            DrawText("BALANCED MULTI-LIGHT + REINHARD TONE MAPPED BLOOM", 20, 20, 20, GREEN);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(cube);
    UnloadModel(floor);
    UnloadShader(lightShader);
    UnloadShader(bloomShader);
    UnloadRenderTexture(target);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
