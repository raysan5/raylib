/*******************************************************************************************
*
*   raylib [shaders] example - demonstrates how you can use your own simple shaders in raylib
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Chris Camacho (@codifies) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2019 Chris Camacho (@codifies) and Ramon Santamaria (@raysan5)
*
********************************************************************************************
*
* after a model is loaded it has a default material, this material can be modified in place
* rather than creating one from scratch...
* While all of the MAPs have particular names, they can be used for any purpose
* Three of the MAP are applied as cubic maps (see below)
*
********************************************************************************************/


#include <stddef.h>

#include "raylib.h"
#include "raymath.h"


#define screenWidth 1280
#define screenHeight 720


int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib - simple shader");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 1.0f, 2.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.type = CAMERA_PERSPECTIVE;

    // three models to show the shader on
    Mesh torus = GenMeshTorus(.3, 1, 16, 32);
    Model model1 = LoadModelFromMesh(torus);

    Mesh cube = GenMeshCube(.8,.8,.8);
    Model model2 = LoadModelFromMesh(cube);

    // this one un shaded just so we can see the gaps in the other two
    Mesh sphere = GenMeshSphere(1, 16, 16);
    Model model3 = LoadModelFromMesh(sphere);

    // load the shader
    Shader shader = LoadShader("resources/shaders/glsl330/mask.vs",
                                "resources/shaders/glsl330/mask.fs");

    // apply the diffuse texture (colour map)
    Texture tex = LoadTexture("resources/plasma.png");
    model1.materials[0].maps[MAP_DIFFUSE].texture = tex;
    model2.materials[0].maps[MAP_DIFFUSE].texture = tex;

    // using MAP_EMISSION as a spare slot to use for 2nd texture
    // dont use MAP_IRRADIANCE, MAP_PREFILTER, or  MAP_CUBEMAP
    // as they are bound as cube maps (which don't see to work at all on my machine!)
    Texture maskTx = LoadTexture("resources/mask.png");
    model1.materials[0].maps[MAP_EMISSION].texture = maskTx;
    model2.materials[0].maps[MAP_EMISSION].texture = maskTx;
    shader.locs[LOC_MAP_EMISSION] = GetShaderLocation(shader, "mask");

    // frame is incremented each frame to animate the shader
    int shaderFrame = GetShaderLocation(shader, "frame");

    // apply the shader to the two models
    model1.materials[0].shader = shader;
    model2.materials[0].shader = shader;


    // frame counter
    int frame = 0;

    // model rotation
    Vector3 ang = { 0 };

    SetTargetFPS(60);               // Set  to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------



    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        frame ++;
        ang.x += 0.01;
        ang.y += 0.005;
        ang.z -= 0.0025;

        // animate the shader
        SetShaderValue(shader, shaderFrame, &frame, UNIFORM_INT);

        // rotate one of the models
        model1.transform = MatrixRotateXYZ(ang);

        UpdateCamera(&camera);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKBLUE);

            BeginMode3D(camera);

                DrawModel(model1, (Vector3){0.5,0,0}, 1, WHITE);
                DrawModelEx(model2, (Vector3){-.5,0,0}, (Vector3){1,1,0}, 50, (Vector3){1,1,1}, WHITE);
                DrawModel(model3,(Vector3){0,0,-1.5}, 1, WHITE);
                DrawGrid(10, 1.0f);        // Draw a grid

            EndMode3D();

            DrawFPS(10, 10);

            int l = MeasureText(FormatText("Frame %i", frame), 20);
            DrawRectangle(16, 698, l+8, 42, BLUE);
            DrawText(FormatText("Frame %i", frame), 20, 700, 20, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    UnloadModel(model1);
    UnloadModel(model2);
    UnloadModel(model3);
    UnloadTexture(tex);
    UnloadTexture(maskTx);
    UnloadShader(shader);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}


