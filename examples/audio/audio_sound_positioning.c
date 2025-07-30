/*******************************************************************************************
*
*   raylib [audio] example - Playing spatialized 3D sound
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Le Juez Victor (@Bigfoot71) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Le Juez Victor (@Bigfoot71)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

// Sound positioning function
static void SetSoundPosition(Camera listener, Sound sound, Vector3 position, float maxDist);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    InitWindow(screenWidth, screenHeight, "raylib [audio] example - Playing spatialized 3D sound");
    
    InitAudioDevice();

    Sound sound = LoadSound("resources/coin.wav");

    Camera camera = {
        .position = (Vector3) { 0, 5, 5 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60,
        .projection = CAMERA_PERSPECTIVE
    };
    
    DisableCursor();
    
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FREE);

        float th = GetTime();

        Vector3 spherePos = {
            .x = 5.0f*cosf(th),
            .y = 0.0f,
            .z = 5.0f*sinf(th)
        };

        SetSoundPosition(camera, sound, spherePos, 20.0f);
        if (!IsSoundPlaying(sound)) PlaySound(sound);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                DrawGrid(10, 2);
                DrawSphere(spherePos, 0.5f, RED);
            EndMode3D();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSound(sound);
    CloseAudioDevice();     // Close audio device
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}

// Sound positioning function
static void SetSoundPosition(Camera listener, Sound sound, Vector3 position, float maxDist)
{
    // Calculate direction vector and distance between listener and sound source
    Vector3 direction = Vector3Subtract(position, listener.position);
    float distance = Vector3Length(direction);
    
    // Apply logarithmic distance attenuation and clamp between 0-1
    float attenuation = 1.0f/(1.0f + (distance/maxDist));
    attenuation = Clamp(attenuation, 0.0f, 1.0f);
    
    // Calculate normalized vectors for spatial positioning
    Vector3 normalizedDirection = Vector3Normalize(direction);
    Vector3 forward = Vector3Normalize(Vector3Subtract(listener.target, listener.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(listener.up, forward));
    
    // Reduce volume for sounds behind the listener
    float dotProduct = Vector3DotProduct(forward, normalizedDirection);
    if (dotProduct < 0.0f) attenuation *= (1.0f + dotProduct*0.5f);
    
    // Set stereo panning based on sound position relative to listener
    float pan = 0.5f + 0.5f*Vector3DotProduct(normalizedDirection, right);
    
    // Apply final sound properties
    SetSoundVolume(sound, attenuation);
    SetSoundPan(sound, pan);
}
