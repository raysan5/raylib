/*******************************************************************************************
*
*   raylib [shaders] example - rlgl module usage for instanced meshes
*
*   This example uses [rlgl] module funtionality (pseudo-OpenGL 1.1 style coding)
*
*   This example has been created using raylib 3.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by @seanpringle and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2020 @seanpringle
*
********************************************************************************************/


#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include <stdlib.h>

#define GLSL_VERSION 330

float maxf(float a, float b){
    return (a > b ? a : b);
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
    int speed = 30;                 // speed of jump animation
    int groups = 2;                 // count of separate groups jumping around
    float amp = 10;                 // maximum amplitude of jump
    float variance = 0.8;           // global variance in jump height
    float loop=0;                   // individual cube's computed loop timer .
    float x=0,y=0,z=0;              // used for various 3D coordinate & vector ops.
    const int fps = 60;

    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - rlgl mesh instanced");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ -125.0f, 125.0f, -125.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const int count = 10000;                                 // Number of instances to display 
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);

    Matrix *rotations = RL_MALLOC(count*sizeof(Matrix));    // Rotation state of instances
    Matrix *rotationsInc = RL_MALLOC(count*sizeof(Matrix)); // Per-frame rotation animation of instances
    Matrix *translations = RL_MALLOC(count*sizeof(Matrix)); // Locations of instances

    // Scatter random cubes around
    for (int i = 0; i < count; i++)
    {
        x = GetRandomValue(-50, 50);
        y = GetRandomValue(-50, 50);
        z = GetRandomValue(-50, 50);
        translations[i] = MatrixTranslate(x, y, z); 

        x = GetRandomValue(0, 360);
        y = GetRandomValue(0, 360);
        z = GetRandomValue(0, 360);
        Vector3 axis = Vector3Normalize((Vector3){x, y, z});
        float angle = (float)GetRandomValue(0, 10) * DEG2RAD;

        rotationsInc[i] = MatrixRotate(axis, angle);
        rotations[i] = MatrixIdentity();
    }

    Matrix *transforms = RL_MALLOC(count*sizeof(Matrix));   // Pre-multiplied transformations passed to rlgl

    Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/base_lighting_instanced.vs", GLSL_VERSION), 
                               TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));

    // Get some shader loactions
    shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader, "instance");

    // Ambient light level
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){ 0.2f, 0.2f, 0.2f, 1.0f }, SHADER_UNIFORM_VEC4);

    CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 50, 50, 0 }, Vector3Zero(), WHITE, shader);

    Material material = LoadMaterialDefault();
    material.shader = shader;
    material.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    
    SetCameraMode(camera, CAMERA_ORBITAL); // Set a free camera mode

    SetTargetFPS(fps);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    int frame = 0;                      // simple frame counter to manage animation
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {

        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);
        frame ++;

        //if (IsKeyDown(KEY_RIGHT)) ballPosition.x += 2.0f;
        //if (IsKeyDown(KEY_LEFT)) ballPosition.x -= 2.0f;
        if (IsKeyDown(KEY_UP)) amp += 0.5;
        if (IsKeyDown(KEY_DOWN)) amp = amp <= 1 ? 1 : amp - 1.0;
        if (IsKeyDown(KEY_LEFT))  variance = variance <=0 ? 0 : variance - 0.01;
        if (IsKeyDown(KEY_RIGHT)) variance = variance >=1 ? 1 : variance + 0.01;
        if (IsKeyDown(KEY_ONE)) groups = 1;
        if (IsKeyDown(KEY_TWO)) groups = 2;
        if (IsKeyDown(KEY_THREE)) groups = 3;
        if (IsKeyDown(KEY_FOUR)) groups = 4;
        if (IsKeyDown(KEY_FIVE)) groups = 5;
        if (IsKeyDown(KEY_SIX)) groups = 6;
        if (IsKeyDown(KEY_SEVEN)) groups = 7;
        if (IsKeyDown(KEY_EIGHT)) groups = 8;
        if (IsKeyDown(KEY_NINE)) groups = 9;
        if (IsKeyDown(KEY_W)) {groups=7; amp = 25; speed=18; variance=0.70;}
            
        if (IsKeyDown(KEY_EQUAL)) speed = speed <= (fps *.25)  ? (fps *.25) : speed * 0.95;
        if (IsKeyDown(KEY_KP_ADD)) speed = speed <= (fps *.25)  ? (fps *.25) : speed * 0.95;
            
        if (IsKeyDown(KEY_MINUS)) speed = maxf(speed * 1.02, speed + 1) ;
        if (IsKeyDown(KEY_KP_SUBTRACT)) maxf(speed * 1.02, speed + 1) ;


        // Update the light shader with the camera view position
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        // Apply per-instance transformations
        for (int i = 0; i < count; i++){
            rotations[i] = MatrixMultiply(rotations[i], rotationsInc[i]);
            transforms[i] = MatrixMultiply(rotations[i], translations[i]);
            
            // get the animation cycle's frame for this instance.
            loop = (float)( (frame + (int)(((float)(i % groups)/groups) * speed)) % speed)  / speed;
            
            // calculate the y according to loop cycle
            y =  ( sinf( loop * PI * 2 )  )  * (amp )* ((1 - variance) +  ((variance) * (float)(i % (groups * 10)) / (groups * 10)));
            
            // clamp to floor
            y = (y<0 ? 0 : y);
            
            transforms[i] = MatrixMultiply(transforms[i], MatrixTranslate(0, y, 0));
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                rlDrawMeshInstanced(cube, material, transforms, count);
            EndMode3D();
            int u = 10;
            int u2 = u + 40;
            int u3 = u2 + 110;
            int v = 300;
            DrawText("A CUBE OF DANCING CUBES!", 490, 10, 20, MAROON);
            DrawText("PRESS KEYS:", u, v, 20, BLACK);
            
            DrawText("1 - 9", u, v+=25, 10, BLACK);  
            DrawText(": Number of groups", u2, v , 10, BLACK);
            DrawText(TextFormat(": %d", groups), u3, v , 10, BLACK);
            
            DrawText("UP", u, v+=15, 10, BLACK);  
            DrawText(": increase amplitude", u2, v, 10, BLACK);
            DrawText(TextFormat(": %.2f", amp), u3, v , 10, BLACK);
            
            DrawText("DOWN", u, v+=15, 10, BLACK);  
            DrawText(": decrease amplitude", u2, v, 10, BLACK);
            
            DrawText("LEFT", u, v+=15, 10, BLACK);  
            DrawText(": decrease variance", u2, v, 10, BLACK);
            DrawText(TextFormat(": %.2f", variance), u3, v , 10, BLACK);
            
            DrawText("RIGHT", u, v+=15, 10, BLACK);  
            DrawText(": increase variance", u2, v, 10, BLACK);
            
            DrawText("+/=", u, v+=15, 10, BLACK);  
            DrawText(": increase speed", u2, v, 10, BLACK);
            DrawText(TextFormat(": %d = %f loops/sec", speed, ((float)fps / speed)), u3, v , 10, BLACK);
            
            DrawText("-", u, v+=15, 10, BLACK);  
            DrawText(": decrease speed", u2, v, 10, BLACK);
            
            DrawText("W", u, v+=15, 10, BLACK);  
            DrawText(": Wild setup!", u2, v, 10, BLACK);
            
            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
