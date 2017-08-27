/*******************************************************************************************
*
*   raylib [shaders] example - Standard lighting (materials and lights)
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3), to test this example
*         on OpenGL ES 2.0 platforms (Android, Raspberry Pi, HTML5), use #version 100 shaders
*         raylib comes with shaders ready for both versions, check raylib/shaders install folder
*
*   This example has been created using raylib 1.7 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016-2017 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>         // Required for: NULL
#include <string.h>         // Required for: strcpy()
#include <math.h>           // Required for: vector math

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_LIGHTS      8   // Max lights supported by standard shader

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Light type
typedef struct LightData {
    unsigned int id;        // Light unique id
    bool enabled;           // Light enabled
    int type;               // Light type: LIGHT_POINT, LIGHT_DIRECTIONAL, LIGHT_SPOT

    Vector3 position;       // Light position
    Vector3 target;         // Light direction: LIGHT_DIRECTIONAL and LIGHT_SPOT (cone direction target)
    float radius;           // Light attenuation radius light intensity reduced with distance (world distance)

    Color diffuse;          // Light diffuse color
    float intensity;        // Light intensity level

    float coneAngle;        // Light cone max angle: LIGHT_SPOT
} LightData, *Light;

// Light types
typedef enum { LIGHT_POINT, LIGHT_DIRECTIONAL, LIGHT_SPOT } LightType;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static Light lights[MAX_LIGHTS];            // Lights pool
static int lightsCount = 0;                 // Enabled lights counter
static int lightsLocs[MAX_LIGHTS][8];       // Lights location points in shader: 8 possible points per light: 
                                            // enabled, type, position, target, radius, diffuse, intensity, coneAngle

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static Light CreateLight(int type, Vector3 position, Color diffuse); // Create a new light, initialize it and add to pool
static void DestroyLight(Light light);     // Destroy a light and take it out of the list
static void DrawLight(Light light);        // Draw light in 3D world

static void GetShaderLightsLocations(Shader shader);    // Get shader locations for lights (up to MAX_LIGHTS)
static void SetShaderLightsValues(Shader shader);       // Set shader uniform values for lights

// Vector3 math functions
static float VectorLength(const Vector3 v);             // Calculate vector length
static void VectorNormalize(Vector3 *v);                // Normalize provided vector
static Vector3 VectorSubtract(Vector3 v1, Vector3 v2);  // Substract two vectors


//https://www.gamedev.net/topic/655969-speed-gluniform-vs-uniform-buffer-objects/
//https://www.reddit.com/r/opengl/comments/4ri20g/is_gluniform_more_expensive_than_glprogramuniform/
//http://cg.alexandra.dk/?p=3778 - AZDO
//https://developer.apple.com/library/content/documentation/3DDrawing/Conceptual/OpenGLES_ProgrammingGuide/BestPracticesforShaders/BestPracticesforShaders.html

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);      // Enable Multi Sampling Anti Aliasing 4x (if available)

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - model shader");

    // Define the camera to look into our 3d world
    Camera camera = {{ 4.0f, 4.0f, 4.0f }, { 0.0f, 1.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };
    Vector3 position = { 0.0f, 0.0f, 0.0f };   // Set model position
    
    Model dwarf = LoadModel("resources/model/dwarf.obj");                     // Load OBJ model

    Material material;// = LoadStandardMaterial();
    
    material.shader = LoadShader("resources/shaders/glsl330/standard.vs", 
                                 "resources/shaders/glsl330/standard.fs");
    
    // Try to get lights location points (if available)
    GetShaderLightsLocations(material.shader);
    
    material.texDiffuse = LoadTexture("resources/model/dwarf_diffuse.png");   // Load model diffuse texture
    material.texNormal = LoadTexture("resources/model/dwarf_normal.png");     // Load model normal texture
    material.texSpecular = LoadTexture("resources/model/dwarf_specular.png"); // Load model specular texture
    material.colDiffuse = WHITE;
    material.colAmbient = (Color){0, 0, 10, 255};
    material.colSpecular = WHITE;
    material.glossiness = 50.0f;
    
    dwarf.material = material;      // Apply material to model

    Light spotLight = CreateLight(LIGHT_SPOT, (Vector3){3.0f, 5.0f, 2.0f}, (Color){255, 255, 255, 255});
    spotLight->target = (Vector3){0.0f, 0.0f, 0.0f};
    spotLight->intensity = 2.0f;
    spotLight->diffuse = (Color){255, 100, 100, 255};
    spotLight->coneAngle = 60.0f;

    Light dirLight = CreateLight(LIGHT_DIRECTIONAL, (Vector3){0.0f, -3.0f, -3.0f}, (Color){255, 255, 255, 255});
    dirLight->target = (Vector3){1.0f, -2.0f, -2.0f};
    dirLight->intensity = 2.0f;
    dirLight->diffuse = (Color){100, 255, 100, 255};

    Light pointLight = CreateLight(LIGHT_POINT, (Vector3){0.0f, 4.0f, 5.0f}, (Color){255, 255, 255, 255});
    pointLight->intensity = 2.0f;
    pointLight->diffuse = (Color){100, 100, 255, 255};
    pointLight->radius = 3.0f;
    
    // Set shader lights values for enabled lights
    // NOTE: If values are not changed in real time, they can be set at initialization!!!
    SetShaderLightsValues(material.shader);
    
    //SetShaderActive(0);

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
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);
                
                DrawModel(dwarf, position, 2.0f, WHITE);   // Draw 3d model with texture
                
                DrawLight(spotLight);   // Draw spot light
                DrawLight(dirLight);    // Draw directional light
                DrawLight(pointLight);  // Draw point light

                DrawGrid(10, 1.0f);     // Draw a grid

            End3dMode();
            
            DrawText("(c) Dwarf 3D model by David Moreno", screenWidth - 200, screenHeight - 20, 10, GRAY);
            
            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadMaterial(material);   // Unload material and assigned textures
    UnloadModel(dwarf);         // Unload model
    
    // Destroy all created lights
    DestroyLight(pointLight);
    DestroyLight(dirLight);
    DestroyLight(spotLight);

    // Unload lights
    if (lightsCount > 0)
    {
        for (int i = 0; i < lightsCount; i++) free(lights[i]);
        lightsCount = 0;
    }

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module Functions Definitions
//--------------------------------------------------------------------------------------------

// Create a new light, initialize it and add to pool
Light CreateLight(int type, Vector3 position, Color diffuse)
{
    Light light = NULL;
    
    if (lightsCount < MAX_LIGHTS)
    {
        // Allocate dynamic memory
        light = (Light)malloc(sizeof(LightData));
        
        // Initialize light values with generic values
        light->id = lightsCount;
        light->type = type;
        light->enabled = true;
        
        light->position = position;
        light->target = (Vector3){ 0.0f, 0.0f, 0.0f };
        light->intensity = 1.0f;
        light->diffuse = diffuse;
        
        // Add new light to the array
        lights[lightsCount] = light;
        
        // Increase enabled lights count
        lightsCount++;
    }
    else
    {
        // NOTE: Returning latest created light to avoid crashes
        light = lights[lightsCount];
    }

    return light;
}

// Destroy a light and take it out of the list
void DestroyLight(Light light)
{
    if (light != NULL)
    {
        int lightId = light->id;

        // Free dynamic memory allocation
        free(lights[lightId]);

        // Remove *obj from the pointers array
        for (int i = lightId; i < lightsCount; i++)
        {
            // Resort all the following pointers of the array
            if ((i + 1) < lightsCount)
            {
                lights[i] = lights[i + 1];
                lights[i]->id = lights[i + 1]->id;
            }
        }
        
        // Decrease enabled physic objects count
        lightsCount--;
    }
}

// Draw light in 3D world
void DrawLight(Light light)
{
    switch (light->type)
    {
        case LIGHT_POINT:
        {
            DrawSphereWires(light->position, 0.3f*light->intensity, 8, 8, (light->enabled ? light->diffuse : GRAY));
            
            DrawCircle3D(light->position, light->radius, (Vector3){ 0, 0, 0 }, 0.0f, (light->enabled ? light->diffuse : GRAY));
            DrawCircle3D(light->position, light->radius, (Vector3){ 1, 0, 0 }, 90.0f, (light->enabled ? light->diffuse : GRAY));
            DrawCircle3D(light->position, light->radius, (Vector3){ 0, 1, 0 },90.0f, (light->enabled ? light->diffuse : GRAY));
        } break;
        case LIGHT_DIRECTIONAL:
        {
            DrawLine3D(light->position, light->target, (light->enabled ? light->diffuse : GRAY));
            
            DrawSphereWires(light->position, 0.3f*light->intensity, 8, 8, (light->enabled ? light->diffuse : GRAY));
            DrawCubeWires(light->target, 0.3f, 0.3f, 0.3f, (light->enabled ? light->diffuse : GRAY));
        } break;
        case LIGHT_SPOT:
        {
            DrawLine3D(light->position, light->target, (light->enabled ? light->diffuse : GRAY));
            
            Vector3 dir = VectorSubtract(light->target, light->position);
            VectorNormalize(&dir);
            
            DrawCircle3D(light->position, 0.5f, dir, 0.0f, (light->enabled ? light->diffuse : GRAY));
            
            //DrawCylinderWires(light->position, 0.0f, 0.3f*light->coneAngle/50, 0.6f, 5, (light->enabled ? light->diffuse : GRAY));
            DrawCubeWires(light->target, 0.3f, 0.3f, 0.3f, (light->enabled ? light->diffuse : GRAY));
        } break;
        default: break;
    }
}

// Get shader locations for lights (up to MAX_LIGHTS)
static void GetShaderLightsLocations(Shader shader)
{
    char locName[32] = "lights[x].\0";
    char locNameUpdated[64];
    
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        locName[7] = '0' + i;
        
        strcpy(locNameUpdated, locName);
        strcat(locNameUpdated, "enabled\0");
        lightsLocs[i][0] = GetShaderLocation(shader, locNameUpdated);
        
        locNameUpdated[0] = '\0';
        strcpy(locNameUpdated, locName);
        strcat(locNameUpdated, "type\0");
        lightsLocs[i][1] = GetShaderLocation(shader, locNameUpdated);

        locNameUpdated[0] = '\0';
        strcpy(locNameUpdated, locName);
        strcat(locNameUpdated, "position\0");
        lightsLocs[i][2] = GetShaderLocation(shader, locNameUpdated);
        
        locNameUpdated[0] = '\0';
        strcpy(locNameUpdated, locName);
        strcat(locNameUpdated, "direction\0");
        lightsLocs[i][3] = GetShaderLocation(shader, locNameUpdated);
        
        locNameUpdated[0] = '\0';
        strcpy(locNameUpdated, locName);
        strcat(locNameUpdated, "radius\0");
        lightsLocs[i][4] = GetShaderLocation(shader, locNameUpdated);
        
        locNameUpdated[0] = '\0';
        strcpy(locNameUpdated, locName);
        strcat(locNameUpdated, "diffuse\0");
        lightsLocs[i][5] = GetShaderLocation(shader, locNameUpdated);
        
        locNameUpdated[0] = '\0';
        strcpy(locNameUpdated, locName);
        strcat(locNameUpdated, "intensity\0");
        lightsLocs[i][6] = GetShaderLocation(shader, locNameUpdated);
        
        locNameUpdated[0] = '\0';
        strcpy(locNameUpdated, locName);
        strcat(locNameUpdated, "coneAngle\0");
        lightsLocs[i][7] = GetShaderLocation(shader, locNameUpdated);
    }
}

// Set shader uniform values for lights
// NOTE: It would be far easier with shader UBOs but are not supported on OpenGL ES 2.0
// TODO: Replace glUniform1i(), glUniform1f(), glUniform3f(), glUniform4f():
//SetShaderValue(Shader shader, int uniformLoc, float *value, int size)
//SetShaderValuei(Shader shader, int uniformLoc, int *value, int size)
static void SetShaderLightsValues(Shader shader)
{
    int tempInt[8] = { 0 };
    float tempFloat[8] = { 0.0f };
    
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (i < lightsCount)
        {
            tempInt[0] = lights[i]->enabled;
            SetShaderValuei(shader, lightsLocs[i][0], tempInt, 1); //glUniform1i(lightsLocs[i][0], lights[i]->enabled);
            
            tempInt[0] = lights[i]->type;
            SetShaderValuei(shader, lightsLocs[i][1], tempInt, 1); //glUniform1i(lightsLocs[i][1], lights[i]->type);
            
            tempFloat[0] = (float)lights[i]->diffuse.r/255.0f;
            tempFloat[1] = (float)lights[i]->diffuse.g/255.0f;
            tempFloat[2] = (float)lights[i]->diffuse.b/255.0f;
            tempFloat[3] = (float)lights[i]->diffuse.a/255.0f;
            SetShaderValue(shader, lightsLocs[i][5], tempFloat, 4);
            //glUniform4f(lightsLocs[i][5], (float)lights[i]->diffuse.r/255, (float)lights[i]->diffuse.g/255, (float)lights[i]->diffuse.b/255, (float)lights[i]->diffuse.a/255);
            
            tempFloat[0] = lights[i]->intensity;
            SetShaderValue(shader, lightsLocs[i][6], tempFloat, 1);
            
            switch (lights[i]->type)
            {
                case LIGHT_POINT:
                {
                    tempFloat[0] = lights[i]->position.x;
                    tempFloat[1] = lights[i]->position.y;
                    tempFloat[2] = lights[i]->position.z;
                    SetShaderValue(shader, lightsLocs[i][2], tempFloat, 3);

                    tempFloat[0] = lights[i]->radius;
                    SetShaderValue(shader, lightsLocs[i][4], tempFloat, 1);
            
                    //glUniform3f(lightsLocs[i][2], lights[i]->position.x, lights[i]->position.y, lights[i]->position.z);
                    //glUniform1f(lightsLocs[i][4], lights[i]->radius);
                } break;
                case LIGHT_DIRECTIONAL:
                {
                    Vector3 direction = VectorSubtract(lights[i]->target, lights[i]->position);
                    VectorNormalize(&direction);
                    
                    tempFloat[0] = direction.x;
                    tempFloat[1] = direction.y;
                    tempFloat[2] = direction.z;
                    SetShaderValue(shader, lightsLocs[i][3], tempFloat, 3);
                    
                    //glUniform3f(lightsLocs[i][3], direction.x, direction.y, direction.z);
                } break;
                case LIGHT_SPOT:
                {
                    tempFloat[0] = lights[i]->position.x;
                    tempFloat[1] = lights[i]->position.y;
                    tempFloat[2] = lights[i]->position.z;
                    SetShaderValue(shader, lightsLocs[i][2], tempFloat, 3);
                    
                    //glUniform3f(lightsLocs[i][2], lights[i]->position.x, lights[i]->position.y, lights[i]->position.z);
                    
                    Vector3 direction = VectorSubtract(lights[i]->target, lights[i]->position);
                    VectorNormalize(&direction);
                    
                    tempFloat[0] = direction.x;
                    tempFloat[1] = direction.y;
                    tempFloat[2] = direction.z;
                    SetShaderValue(shader, lightsLocs[i][3], tempFloat, 3);
                    //glUniform3f(lightsLocs[i][3], direction.x, direction.y, direction.z);
                    
                    tempFloat[0] = lights[i]->coneAngle;
                    SetShaderValue(shader, lightsLocs[i][7], tempFloat, 1);
                    //glUniform1f(lightsLocs[i][7], lights[i]->coneAngle);
                } break;
                default: break;
            }
        }
        else
        {
            tempInt[0] = 0;
            SetShaderValuei(shader, lightsLocs[i][0], tempInt, 1); //glUniform1i(lightsLocs[i][0], 0);   // Light disabled
        }
    }
}

// Calculate vector length
float VectorLength(const Vector3 v)
{
    float length;

    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);

    return length;
}

// Normalize provided vector
void VectorNormalize(Vector3 *v)
{
    float length, ilength;

    length = VectorLength(*v);

    if (length == 0.0f) length = 1.0f;

    ilength = 1.0f/length;

    v->x *= ilength;
    v->y *= ilength;
    v->z *= ilength;
}

// Substract two vectors
Vector3 VectorSubtract(Vector3 v1, Vector3 v2)
{
    Vector3 result;

    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;

    return result;
}
