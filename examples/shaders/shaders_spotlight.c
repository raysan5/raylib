/*******************************************************************************************
*
*   raylib [shaders] example - Simple shader mask
*
*   Example originally created with raylib 2.5, last time updated with raylib 3.7
*
*   Example contributed by Chris Camacho (@chriscamacho) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2019-2022 Chris Camacho (@chriscamacho) and Ramon Santamaria (@raysan5)
*
********************************************************************************************
*
*   The shader makes alpha holes in the forground to give the appearance of a top
*   down look at a spotlight casting a pool of light...
*
*   The right hand side of the screen there is just enough light to see whats
*   going on without the spot light, great for a stealth type game where you
*   have to avoid the spotlights.
*
*   The left hand side of the screen is in pitch dark except for where the spotlights are.
*
*   Although this example doesn't scale like the letterbox example, you could integrate
*   the two techniques, but by scaling the actual colour of the render texture rather
*   than using alpha as a mask.
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#include <stddef.h>
#include <stdint.h>

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define MAX_SPOTS         3        // NOTE: It must be the same as define in shader
#define MAX_STARS       400

// Spot data
typedef struct {
    Vector2 pos;
    Vector2 vel;
    float inner;
    float radius;

    // Shader locations
    unsigned int posLoc;
    unsigned int innerLoc;
    unsigned int radiusLoc;
} Spot;

// Stars in the star field have a position and velocity
typedef struct Star {
    Vector2 pos;
    Vector2 vel;
} Star;

void UpdateStar(Star *s);
void ResetStar(Star *s);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib - shader spotlight");
    HideCursor();

    Texture texRay = LoadTexture("resources/raysan.png");

    Star stars[MAX_STARS] = { 0 };

    for (int n = 0; n < MAX_STARS; n++) ResetStar(&stars[n]);

    // Progress all the stars on, so they don't all start in the centre
    for (int m = 0; m < screenWidth/2.0; m++)
    {
        for (int n = 0; n < MAX_STARS; n++) UpdateStar(&stars[n]);
    }

    int frameCounter = 0;

    // Use default vert shader
    Shader shdrSpot = LoadShader(0, TextFormat("resources/shaders/glsl%i/spotlight.fs", GLSL_VERSION));

    // Get the locations of spots in the shader
    Spot spots[MAX_SPOTS];

    for (int i = 0; i < MAX_SPOTS; i++)
    {
        char posName[32] = "spots[x].pos\0";
        char innerName[32] = "spots[x].inner\0";
        char radiusName[32] = "spots[x].radius\0";

        posName[6] = '0' + i;
        innerName[6] = '0' + i;
        radiusName[6] = '0' + i;

        spots[i].posLoc = GetShaderLocation(shdrSpot, posName);
        spots[i].innerLoc = GetShaderLocation(shdrSpot, innerName);
        spots[i].radiusLoc = GetShaderLocation(shdrSpot, radiusName);

    }

    // Tell the shader how wide the screen is so we can have
    // a pitch black half and a dimly lit half.
    unsigned int wLoc = GetShaderLocation(shdrSpot, "screenWidth");
    float sw = (float)GetScreenWidth();
    SetShaderValue(shdrSpot, wLoc, &sw, SHADER_UNIFORM_FLOAT);

    // Randomize the locations and velocities of the spotlights
    // and initialize the shader locations
    for (int i = 0; i < MAX_SPOTS; i++)
    {
        spots[i].pos.x = (float)GetRandomValue(64, screenWidth - 64);
        spots[i].pos.y = (float)GetRandomValue(64, screenHeight - 64);
        spots[i].vel = (Vector2){ 0, 0 };

        while ((fabs(spots[i].vel.x) + fabs(spots[i].vel.y)) < 2)
        {
            spots[i].vel.x = GetRandomValue(-400, 40) / 10.0f;
            spots[i].vel.y = GetRandomValue(-400, 40) / 10.0f;
        }

        spots[i].inner = 28.0f * (i + 1);
        spots[i].radius = 48.0f * (i + 1);

        SetShaderValue(shdrSpot, spots[i].posLoc, &spots[i].pos.x, SHADER_UNIFORM_VEC2);
        SetShaderValue(shdrSpot, spots[i].innerLoc, &spots[i].inner, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shdrSpot, spots[i].radiusLoc, &spots[i].radius, SHADER_UNIFORM_FLOAT);
    }

    SetTargetFPS(60);               // Set  to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        frameCounter++;

        // Move the stars, resetting them if the go offscreen
        for (int n = 0; n < MAX_STARS; n++) UpdateStar(&stars[n]);

        // Update the spots, send them to the shader
        for (int i = 0; i < MAX_SPOTS; i++)
        {
            if (i == 0)
            {
                Vector2 mp = GetMousePosition();
                spots[i].pos.x = mp.x;
                spots[i].pos.y = screenHeight - mp.y;
            }
            else
            {
                spots[i].pos.x += spots[i].vel.x;
                spots[i].pos.y += spots[i].vel.y;

                if (spots[i].pos.x < 64) spots[i].vel.x = -spots[i].vel.x;
                if (spots[i].pos.x > (screenWidth - 64)) spots[i].vel.x = -spots[i].vel.x;
                if (spots[i].pos.y < 64) spots[i].vel.y = -spots[i].vel.y;
                if (spots[i].pos.y > (screenHeight - 64)) spots[i].vel.y = -spots[i].vel.y;
            }

            SetShaderValue(shdrSpot, spots[i].posLoc, &spots[i].pos.x, SHADER_UNIFORM_VEC2);
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKBLUE);

            // Draw stars and bobs
            for (int n = 0; n < MAX_STARS; n++)
            {
                // Single pixel is just too small these days!
                DrawRectangle((int)stars[n].pos.x, (int)stars[n].pos.y, 2, 2, WHITE);
            }

            for (int i = 0; i < 16; i++)
            {
                DrawTexture(texRay,
                    (int)((screenWidth/2.0f) + cos((frameCounter + i*8)/51.45f)*(screenWidth/2.2f) - 32),
                    (int)((screenHeight/2.0f) + sin((frameCounter + i*8)/17.87f)*(screenHeight/4.2f)), WHITE);
            }

            // Draw spot lights
            BeginShaderMode(shdrSpot);
                // Instead of a blank rectangle you could render here
                // a render texture of the full screen used to do screen
                // scaling (slight adjustment to shader would be required
                // to actually pay attention to the colour!)
                DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
            EndShaderMode();

            DrawFPS(10, 10);

            DrawText("Move the mouse!", 10, 30, 20, GREEN);
            DrawText("Pitch Black", (int)(screenWidth*0.2f), screenHeight/2, 20, GREEN);
            DrawText("Dark", (int)(screenWidth*.66f), screenHeight/2, 20, GREEN);


        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texRay);
    UnloadShader(shdrSpot);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}


void ResetStar(Star *s)
{
    s->pos = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };

    do
    {
        s->vel.x = (float)GetRandomValue(-1000, 1000)/100.0f;
        s->vel.y = (float)GetRandomValue(-1000, 1000)/100.0f;

    } while (!(fabs(s->vel.x) + (fabs(s->vel.y) > 1)));

    s->pos = Vector2Add(s->pos, Vector2Multiply(s->vel, (Vector2){ 8.0f, 8.0f }));
}

void UpdateStar(Star *s)
{
    s->pos = Vector2Add(s->pos, s->vel);

    if ((s->pos.x < 0) || (s->pos.x > GetScreenWidth()) ||
        (s->pos.y < 0) || (s->pos.y > GetScreenHeight()))
    {
        ResetStar(s);
    }
}


