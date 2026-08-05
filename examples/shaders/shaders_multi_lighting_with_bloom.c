/*******************************************************************************************
*
*   raylib [shaders] example - forward multi-lighting with bloom
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
* 
*   NOTE: Forward multi-point lighting combined with specular highlights and post-processing
*   tone-mapped bloom shader pass using offscreen render textures.
*
*   Example contributed by PanicTitan and reviewed by Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#include <stdlib.h>     // Required for: NULL
#include <math.h>       // Required for: sinf(), cosf()

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            "#version 330\n"
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            "#version 300 es\nprecision mediump float;\n"
#endif

//--------------------------------------------------------------------------------------
// Global Definitions
//--------------------------------------------------------------------------------------
#define MAX_LIGHTS      8

//--------------------------------------------------------------------------------------
// Embedded Shader Sources
//--------------------------------------------------------------------------------------

// Forward Vertex Shader
static const char *multiLightVS = GLSL_VERSION R"(
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;

uniform mat4 mvp;
uniform mat4 matModel;

void main() {
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragNormal = normalize(vec3(matModel * vec4(vertexNormal, 0.0)));
    fragTexCoord = vertexTexCoord;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

// Forward Fragment Shader (Point Light Falloff & Blinn-Phong Specular)
static const char *multiLightFS = GLSL_VERSION R"(
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec3 viewPos;
uniform vec3 lightPositions[8];
uniform vec3 lightColors[8];

void main() {
    vec4 texColor = texture(texture0, fragTexCoord);
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);

    // Dark ambient baseline
    vec3 ambient = 0.05 * texColor.rgb;
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    for (int i = 0; i < 8; i++) {
        vec3 lightDir = normalize(lightPositions[i] - fragPosition);
        float dist = length(lightPositions[i] - fragPosition);
        
        // Attenuation calculation
        float attenuation = 1.0 / (1.0 + 0.8 * dist + 0.4 * dist * dist);

        // Diffuse Shading
        float diff = max(dot(norm, lightDir), 0.0);
        totalDiffuse += diff * lightColors[i] * attenuation * 1.2;

        // Blinn-Phong Specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
        totalSpecular += spec * lightColors[i] * attenuation * 0.8;
    }

    vec3 result = ambient + (totalDiffuse * texColor.rgb) + totalSpecular;
    finalColor = vec4(result, texColor.a);
}
)";

// Tone-Mapped Kawase Bloom Fragment Shader
static const char *bloomFS = GLSL_VERSION R"(
in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;

void main() {
    vec2 uv = fragTexCoord;
    vec4 color = texture(texture0, uv);

    // High-pass threshold check (pixels brighter than 0.75)
    vec4 bloom = vec4(0.0);
    vec2 texel = vec2(1.0 / 1280.0, 1.0 / 720.0) * 2.5;

    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            vec4 smp = texture(texture0, uv + vec2(x, y) * texel);
            float brightness = max(smp.r, max(smp.g, smp.b));
            if (brightness > 0.75) bloom += smp;
        }
    }
    bloom /= 25.0;

    // Combine original with soft bloom glow
    vec3 HDR = color.rgb + bloom.rgb * 1.2;

    // Reinhard Tone Mapping
    vec3 LDR = HDR / (HDR + vec3(1.0));

    finalColor = vec4(LDR, color.a);
}
)";

//--------------------------------------------------------------------------------------
// Types and Structures Definition
//--------------------------------------------------------------------------------------
typedef struct AppContext {
    Camera camera;                      // 3D scene camera
    RenderTexture2D fbo;                // Offscreen render target
    Shader lightShader;                 // Multi-light forward shader
    Shader bloomShader;                 // Bloom post-processing shader
    Model cubeModel;                    // Center cube mesh model
    Model floorModel;                   // Ground plane mesh model
    
    int lightPosLoc;                    // Uniform location: light positions
    int lightColLoc;                    // Uniform location: light colors
    int viewPosLoc;                     // Uniform location: view position
    
    Vector3 lightPositions[MAX_LIGHTS]; // Array of point light positions
    Vector3 lightColors[MAX_LIGHTS];    // Array of point light RGB colors
} AppContext;

//--------------------------------------------------------------------------------------
// Module Global Variables
//--------------------------------------------------------------------------------------
static AppContext g_App = { 0 };

//--------------------------------------------------------------------------------------
// Module Functions Declaration
//--------------------------------------------------------------------------------------
void UpdateDrawFrame(void);             // Main frame loop (web & desktop)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - forward multi-lighting bloom");

    // Camera setup
    g_App.camera.position = (Vector3){ 0.0f, 5.0f, 9.0f };
    g_App.camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };
    g_App.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    g_App.camera.fovy = 45.0f;
    g_App.camera.projection = CAMERA_PERSPECTIVE;

    // Load render target for multi-pass bloom
    g_App.fbo = LoadRenderTexture(screenWidth, screenHeight);

    // Load custom shaders from memory strings
    g_App.lightShader = LoadShaderFromMemory(multiLightVS, multiLightFS);
    g_App.bloomShader = LoadShaderFromMemory(NULL, bloomFS);

    // Create models and attach custom lighting shader
    g_App.cubeModel = LoadModelFromMesh(GenMeshCube(2.0f, 2.0f, 2.0f));
    g_App.floorModel = LoadModelFromMesh(GenMeshPlane(14.0f, 14.0f, 1, 1));
    g_App.cubeModel.materials[0].shader = g_App.lightShader;
    g_App.floorModel.materials[0].shader = g_App.lightShader;

    // Fetch shader uniform locations
    g_App.lightPosLoc = GetShaderLocation(g_App.lightShader, "lightPositions");
    g_App.lightColLoc = GetShaderLocation(g_App.lightShader, "lightColors");
    g_App.viewPosLoc  = GetShaderLocation(g_App.lightShader, "viewPos");

    // Initialize 8 vivid light colors
    g_App.lightColors[0] = (Vector3){ 1.0f, 0.2f, 0.2f }; // Red
    g_App.lightColors[1] = (Vector3){ 0.2f, 1.0f, 0.3f }; // Green
    g_App.lightColors[2] = (Vector3){ 0.2f, 0.5f, 1.0f }; // Blue
    g_App.lightColors[3] = (Vector3){ 1.0f, 0.8f, 0.1f }; // Yellow
    g_App.lightColors[4] = (Vector3){ 1.0f, 0.1f, 0.8f }; // Magenta
    g_App.lightColors[5] = (Vector3){ 0.1f, 1.0f, 1.0f }; // Cyan
    g_App.lightColors[6] = (Vector3){ 1.0f, 0.4f, 0.1f }; // Orange
    g_App.lightColors[7] = (Vector3){ 0.7f, 0.2f, 1.0f }; // Purple

    SetShaderValueV(g_App.lightShader, g_App.lightColLoc, g_App.lightColors, SHADER_UNIFORM_VEC3, MAX_LIGHTS);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(g_App.cubeModel);
    UnloadModel(g_App.floorModel);
    UnloadShader(g_App.lightShader);
    UnloadShader(g_App.bloomShader);
    UnloadRenderTexture(g_App.fbo);
    CloseWindow();
    //--------------------------------------------------------------------------------------
#endif

    return 0;
}

//--------------------------------------------------------------------------------------
// Frame Update and Render Logic
//--------------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdateCamera(&g_App.camera, CAMERA_ORBITAL);
    float time = (float)GetTime();

    // Orbital path calculation for point lights
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        float angle = (i / (float)MAX_LIGHTS) * 2.0f * PI + time * 0.6f;
        float radius = 4.2f + sinf(time * 1.2f + i) * 0.4f;
        float height = 1.0f + sinf(time * 1.8f + i) * 0.6f;
        g_App.lightPositions[i] = (Vector3){ sinf(angle) * radius, height, cosf(angle) * radius };
    }

    SetShaderValueV(g_App.lightShader, g_App.lightPosLoc, g_App.lightPositions, SHADER_UNIFORM_VEC3, MAX_LIGHTS);
    SetShaderValue(g_App.lightShader, g_App.viewPosLoc, &g_App.camera.position, SHADER_UNIFORM_VEC3);
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    // PASS 1: RENDER SCENE TO OFFSCREEN FBO
    BeginTextureMode(g_App.fbo);

        ClearBackground((Color){ 12, 12, 18, 255 });

        BeginMode3D(g_App.camera);

            DrawModel(g_App.cubeModel, (Vector3){ 0.0f, 1.0f, 0.0f }, 1.0f, GRAY);
            DrawModel(g_App.floorModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, DARKGRAY);
            DrawGrid(10, 1.0f);

            // Draw glowing orb bulbs at light origins
            for (int i = 0; i < MAX_LIGHTS; i++)
            {
                Color bulbColor = (Color){
                    (unsigned char)(g_App.lightColors[i].x * 255),
                    (unsigned char)(g_App.lightColors[i].y * 255),
                    (unsigned char)(g_App.lightColors[i].z * 255),
                    255
                };
                DrawSphere(g_App.lightPositions[i], 0.15f, bulbColor);
            }

        EndMode3D();

    EndTextureMode();

    // PASS 2: PRESENT WITH TONE-MAPPED BLOOM
    BeginDrawing();

        ClearBackground(BLACK);

        Rectangle srcRec = (Rectangle){ 0, 0, (float)g_App.fbo.texture.width, (float)-g_App.fbo.texture.height };

        BeginShaderMode(g_App.bloomShader);
            DrawTextureRec(g_App.fbo.texture, srcRec, (Vector2){ 0, 0 }, WHITE);
        EndShaderMode();

        DrawText("BALANCED MULTI-LIGHT + REINHARD TONE MAPPED BLOOM", 20, 20, 20, GREEN);
        DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}