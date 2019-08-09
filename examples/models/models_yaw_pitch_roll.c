/*******************************************************************************************
*
*   raylib [models] example - Plane rotations (yaw, pitch, roll)
*
*   This example has been created using raylib 1.8 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Berni (@Berni8k) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2017 Berni (@Berni8k) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

// Draw angle gauge controls
void DrawAngleGauge(Texture2D angleGauge, int x, int y, float angle, char title[], Color color);

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - plane rotations (yaw, pitch, roll)");

    Texture2D texAngleGauge = LoadTexture("resources/angle_gauge.png");
    Texture2D texBackground = LoadTexture("resources/background.png");
    Texture2D texPitch = LoadTexture("resources/pitch.png");
    Texture2D texPlane = LoadTexture("resources/plane.png");

    RenderTexture2D framebuffer = LoadRenderTexture(192, 192);

    // Model loading
    Model model = LoadModel("resources/plane.obj");     // Load OBJ model
    model.materials[0].maps[MAP_DIFFUSE].texture = LoadTexture("resources/plane_diffuse.png"); // Set map diffuse texture

    GenTextureMipmaps(&model.materials[0].maps[MAP_DIFFUSE].texture);

    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 60.0f, -120.0f };// Camera position perspective
    camera.target = (Vector3){ 0.0f, 12.0f, 0.0f };     // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 30.0f;                                // Camera field-of-view Y
    camera.type = CAMERA_PERSPECTIVE;                   // Camera type

    float pitch = 0.0f;
    float roll = 0.0f;
    float yaw = 0.0f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // Plane roll (x-axis) controls
        if (IsKeyDown(KEY_LEFT)) roll += 1.0f;
        else if (IsKeyDown(KEY_RIGHT)) roll -= 1.0f;
        else
        {
            if (roll > 0.0f) roll -= 0.5f;
            else if (roll < 0.0f) roll += 0.5f;
        }

        // Plane yaw (y-axis) controls
        if (IsKeyDown(KEY_S)) yaw += 1.0f;
        else if (IsKeyDown(KEY_A)) yaw -= 1.0f;
        else
        {
            if (yaw > 0.0f) yaw -= 0.5f;
            else if (yaw < 0.0f) yaw += 0.5f;
        }

        // Plane pitch (z-axis) controls
        if (IsKeyDown(KEY_DOWN)) pitch += 0.6f;
        else if (IsKeyDown(KEY_UP)) pitch -= 0.6f;
        else
        {
            if (pitch > 0.3f) pitch -= 0.3f;
            else if (pitch < -0.3f) pitch += 0.3f;
        }

        // Wraps the phase of an angle to fit between -180 and +180 degrees
        int pitchOffset = pitch;
        while (pitchOffset > 180) pitchOffset -= 360;
        while (pitchOffset < -180) pitchOffset += 360;
        pitchOffset *= 10;

        /* matrix transform done with multiplication to combine rotations
        Matrix transform = MatrixIdentity();

        transform = MatrixMultiply(transform, MatrixRotateZ(DEG2RAD*roll));
        transform = MatrixMultiply(transform, MatrixRotateX(DEG2RAD*pitch));
        transform = MatrixMultiply(transform, MatrixRotateY(DEG2RAD*yaw));

        model.transform = transform;
        */
        // matrix created from multiple axes at once
        model.transform = MatrixRotateXYZ((Vector3){DEG2RAD*pitch,DEG2RAD*yaw,DEG2RAD*roll});

        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw framebuffer texture (Ahrs Display)
            int centerX = framebuffer.texture.width/2;
            int centerY = framebuffer.texture.height/2;
            float scaleFactor = 0.5f;

            BeginTextureMode(framebuffer);

                BeginBlendMode(BLEND_ALPHA);

                DrawTexturePro(texBackground, (Rectangle){ 0, 0, texBackground.width, texBackground.height },
                               (Rectangle){ centerX, centerY, texBackground.width*scaleFactor, texBackground.height*scaleFactor},
                               (Vector2){ texBackground.width/2*scaleFactor, texBackground.height/2*scaleFactor + pitchOffset*scaleFactor }, roll, WHITE);

                DrawTexturePro(texPitch, (Rectangle){ 0, 0, texPitch.width, texPitch.height },
                               (Rectangle){ centerX, centerY, texPitch.width*scaleFactor, texPitch.height*scaleFactor },
                               (Vector2){ texPitch.width/2*scaleFactor, texPitch.height/2*scaleFactor + pitchOffset*scaleFactor }, roll, WHITE);

                DrawTexturePro(texPlane, (Rectangle){ 0, 0, texPlane.width, texPlane.height },
                               (Rectangle){ centerX, centerY, texPlane.width*scaleFactor, texPlane.height*scaleFactor },
                               (Vector2){ texPlane.width/2*scaleFactor, texPlane.height/2*scaleFactor }, 0, WHITE);

                EndBlendMode();

            EndTextureMode();

            // Draw 3D model (recomended to draw 3D always before 2D)
            BeginMode3D(camera);

                DrawModel(model, (Vector3){ 0, 6.0f, 0 }, 1.0f, WHITE);   // Draw 3d model with texture
                DrawGrid(10, 10.0f);

            EndMode3D();

            // Draw 2D GUI stuff
            DrawAngleGauge(texAngleGauge, 80, 70, roll, "roll", RED);
            DrawAngleGauge(texAngleGauge, 190, 70, pitch, "pitch", GREEN);
            DrawAngleGauge(texAngleGauge, 300, 70, yaw, "yaw", SKYBLUE);

            DrawRectangle(30, 360, 260, 70, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(30, 360, 260, 70, Fade(DARKBLUE, 0.5f));
            DrawText("Pitch controlled with: KEY_UP / KEY_DOWN", 40, 370, 10, DARKGRAY);
            DrawText("Roll controlled with: KEY_LEFT / KEY_RIGHT", 40, 390, 10, DARKGRAY);
            DrawText("Yaw controlled with: KEY_A / KEY_S", 40, 410, 10, DARKGRAY);

            // Draw framebuffer texture
            DrawTextureRec(framebuffer.texture, (Rectangle){ 0, 0, framebuffer.texture.width, -framebuffer.texture.height },
                           (Vector2){ screenWidth - framebuffer.texture.width - 20, 20 }, Fade(WHITE, 0.8f));

            DrawRectangleLines(screenWidth - framebuffer.texture.width - 20, 20, framebuffer.texture.width, framebuffer.texture.height, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // Unload all loaded data
    UnloadModel(model);

    UnloadRenderTexture(framebuffer);

    UnloadTexture(texAngleGauge);
    UnloadTexture(texBackground);
    UnloadTexture(texPitch);
    UnloadTexture(texPlane);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Draw angle gauge controls
void DrawAngleGauge(Texture2D angleGauge, int x, int y, float angle, char title[], Color color)
{
    Rectangle srcRec = { 0, 0, angleGauge.width, angleGauge.height };
    Rectangle dstRec = { x, y, angleGauge.width, angleGauge.height };
    Vector2 origin = { angleGauge.width/2, angleGauge.height/2};
    int textSize = 20;

    DrawTexturePro(angleGauge, srcRec, dstRec, origin, angle, color);

    DrawText(FormatText("%5.1f", angle), x - MeasureText(FormatText("%5.1f", angle), textSize) / 2, y + 10, textSize, DARKGRAY);
    DrawText(title, x - MeasureText(title, textSize) / 2, y + 60, textSize, DARKGRAY);
}
