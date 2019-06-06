/*******************************************************************************************
*
*   raylib [models] example - Third Person Camera
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2019 Ramon Santamaria (@raysan5) and @PompPenguin
*
*
* 
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"


int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib example - models_third_person");

    // Define the camera to look into our 3d world
    Camera camera = {{ 30.0f, 30.0f, 30.0f }, { 0.0f, 10.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };

    Model model = LoadModel("resources/models/turret.obj");                     // Load default model obj
    Texture2D texture = LoadTexture("resources/models/turret_diffuse.png");     // Load default model texture
    model.materials[0].maps[MAP_DIFFUSE].texture = texture; // Bind texture to model
    
    Vector3 position = { 0.0f, 0.0f, 0.0f };                   // Set model position

    SetCameraMode(camera, CAMERA_THIRD_PERSON);     // Set a third person camera mode

    //Used for camera forward
    Vector3 CameraPerspective = { 0.0f, 0.0f, 0.0f };
    //Used for Vector3 calculations
    Vector3 tempVec3 = { 0.0f, 0.0f, 0.0f };
    //Used for camera right
    Vector3 v3Right ={ 0.0f, 0.0f, 0.0f };
    //Rotation    
    float angle = 0.0f;

    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        camera.target = (Vector3){ position.x, position.y, position.z };
        camera.position = (Vector3){ position.x, position.y, position.z };

        UpdateCamera(&camera);


        //Get the camera foward
        CameraPerspective = Vector3Subtract(camera.position,camera.target);
        
        //Move model and rotate Y axis in Cardinal Directions
        if (IsKeyDown(KEY_W)){
            //Move based on camera forward
            tempVec3 = Vector3Multiply(Vector3Negate(CameraPerspective),1.0f);
            tempVec3 = Vector3Multiply(tempVec3,GetFrameTime());
            position = Vector3Add(position, (Vector3){tempVec3.x,0.0f,tempVec3.z});
            
            //rotate the model
            angle = atan2 (CameraPerspective.z, CameraPerspective.x);
            model.transform =  MatrixRotateY(angle);
        }
        if (IsKeyDown(KEY_A)){
            //Get right of camera forward
            v3Right = Vector3CrossProduct(camera.up,CameraPerspective);
            
            //Move based on camera forward
            tempVec3 = Vector3Multiply(Vector3Negate(v3Right),1.0f);
            tempVec3 = Vector3Multiply(tempVec3,GetFrameTime());
            position = Vector3Add(position, (Vector3){tempVec3.x,0.0f,tempVec3.z});
            
            //rotate the model
            angle = atan2 (-(v3Right.z), -(v3Right.x));
            model.transform =  MatrixRotateY(angle);
        }
        if (IsKeyDown(KEY_D)){
            //Get right of camera forward
            v3Right = Vector3CrossProduct(camera.up,CameraPerspective);
            //Move based on camera forward
            tempVec3 = Vector3Multiply(v3Right,1.0f);
            tempVec3 = Vector3Multiply(tempVec3,GetFrameTime());
            position = Vector3Add(position, (Vector3){tempVec3.x,0.0f,tempVec3.z});
            
            //rotate the model
            angle = atan2 (v3Right.z, v3Right.x);
            model.transform =  MatrixRotateY(angle);
        }
        if (IsKeyDown(KEY_S)){
            //Move based on camera forward
            tempVec3 = Vector3Multiply(CameraPerspective,1.0f);
            tempVec3 = Vector3Multiply(tempVec3,GetFrameTime());
            position = Vector3Add(position, (Vector3){tempVec3.x,0.0f,tempVec3.z});
        
            //rotate the model
            CameraPerspective = Vector3Negate(CameraPerspective);
            angle = atan2 (CameraPerspective.z, CameraPerspective.x);
            model.transform =  MatrixRotateY(angle);
        }

     
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModel(model, position, 1.0f, WHITE);   // Draw 3d model with texture

                DrawGrid(20.0, 10.0);        // Draw a grid

            EndMode3D();
            
            DrawText("(c) Turret 3D model by Alberto Cano", screenWidth - 200, screenHeight - 20, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);         // Unload model
    UnloadTexture(texture);     // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}