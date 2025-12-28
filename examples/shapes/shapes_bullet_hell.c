/*******************************************************************************************
*
*   raylib [shapes] example - bullet hell
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Zero (@zerohorsepower) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Zero (@zerohorsepower)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>         // Required for: calloc(), free()
#include <math.h>           // Required for: cosf(), sinf()

#define MAX_BULLETS 500000      // Max bullets to be processed

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Bullet {
    Vector2 position;       // Bullet position on screen
    Vector2 acceleration;   // Amount of pixels to be incremented to position every frame
    bool disabled;          // Skip processing and draw case out of screen
    Color color;            // Bullet color
} Bullet;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - bullet hell");

    // Bullets definition
    Bullet *bullets = (Bullet *)RL_CALLOC(MAX_BULLETS, sizeof(Bullet)); // Bullets array
    int bulletCount = 0;
    int bulletDisabledCount = 0; // Used to calculate how many bullets are on screen
    int bulletRadius = 10;
    float bulletSpeed = 3.0f;
    int bulletRows = 6;
    Color bulletColor[2] = { RED, BLUE };

    // Spawner variables
    float baseDirection = 0;
    int angleIncrement = 5; // After spawn all bullet rows, increment this value on the baseDirection for next the frame
    float spawnCooldown = 2;
    float spawnCooldownTimer = spawnCooldown;

    // Magic circle
    float magicCircleRotation = 0;

    // Used on performance drawing
    RenderTexture bulletTexture = LoadRenderTexture(24, 24);

    // Draw circle to bullet texture, then draw bullet using DrawTexture()
    // NOTE: This is done to improve the performance, since DrawCircle() is very slow
    BeginTextureMode(bulletTexture);
        DrawCircle(12, 12, (float)bulletRadius, WHITE);
        DrawCircleLines(12, 12, (float)bulletRadius, BLACK);
    EndTextureMode();

    bool drawInPerformanceMode = true; // Switch between DrawCircle() and DrawTexture()

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Reset the bullet index
        // New bullets will replace the old ones that are already disabled due to out-of-screen
        if (bulletCount >= MAX_BULLETS)
        {
            bulletCount = 0;
            bulletDisabledCount = 0;
        }

        spawnCooldownTimer--;
        if (spawnCooldownTimer < 0)
        {
            spawnCooldownTimer = spawnCooldown;

            // Spawn bullets
            float degreesPerRow = 360.0f/bulletRows;
            for (int row = 0; row < bulletRows; row++)
            {
                if (bulletCount < MAX_BULLETS)
                {
                    bullets[bulletCount].position = (Vector2){(float) screenWidth/2, (float) screenHeight/2};
                    bullets[bulletCount].disabled = false;
                    bullets[bulletCount].color = bulletColor[row%2];

                    float bulletDirection = baseDirection + (degreesPerRow*row);

                    // Bullet speed*bullet direction, this will determine how much pixels will be incremented/decremented
                    // from the bullet position every frame. Since the bullets doesn't change its direction and speed,
                    // only need to calculate it at the spawning time
                    // 0 degrees = right, 90 degrees = down, 180 degrees = left and 270 degrees = up, basically clockwise
                    // Case you want it to be anti-clockwise, add "* -1" at the y acceleration
                    bullets[bulletCount].acceleration = (Vector2){
                        bulletSpeed*cosf(bulletDirection*DEG2RAD),
                        bulletSpeed*sinf(bulletDirection*DEG2RAD)
                    };

                    bulletCount++;
                }
            }

            baseDirection += angleIncrement;
        }

        // Update bullets position based on its acceleration
        for (int i = 0; i < bulletCount; i++)
        {
            // Only update bullet if inside the screen
            if (!bullets[i].disabled)
            {
                bullets[i].position.x += bullets[i].acceleration.x;
                bullets[i].position.y += bullets[i].acceleration.y;

                // Disable bullet if out of screen
                if ((bullets[i].position.x < -bulletRadius*2) ||
                    (bullets[i].position.x > screenWidth + bulletRadius*2) ||
                    (bullets[i].position.y < -bulletRadius*2) ||
                    (bullets[i].position.y > screenHeight + bulletRadius*2))
                {
                    bullets[i].disabled = true;
                    bulletDisabledCount++;
                }
            }
        }

        // Input logic
        if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && (bulletRows < 359)) bulletRows++;
        if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && (bulletRows > 1)) bulletRows--;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) bulletSpeed += 0.25f;
        if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && (bulletSpeed > 0.50f)) bulletSpeed -= 0.25f;
        if (IsKeyPressed(KEY_Z) && (spawnCooldown > 1)) spawnCooldown--;
        if (IsKeyPressed(KEY_X)) spawnCooldown++;
        if (IsKeyPressed(KEY_ENTER)) drawInPerformanceMode = !drawInPerformanceMode;

        if (IsKeyDown(KEY_SPACE))
        {
            angleIncrement += 1;
            angleIncrement %= 360;
        }

        if (IsKeyPressed(KEY_C))
        {
            bulletCount = 0;
            bulletDisabledCount = 0;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Draw magic circle
            magicCircleRotation++;
            DrawRectanglePro((Rectangle){ (float)screenWidth/2, (float)screenHeight/2, 120, 120 },
                (Vector2){ 60.0f, 60.0f }, magicCircleRotation, PURPLE);
            DrawRectanglePro((Rectangle){ (float)screenWidth/2, (float)screenHeight/2, 120, 120 },
                (Vector2){ 60.0f, 60.0f }, magicCircleRotation + 45, PURPLE);
            DrawCircleLines(screenWidth/2, screenHeight/2, 70, BLACK);
            DrawCircleLines(screenWidth/2, screenHeight/2, 50, BLACK);
            DrawCircleLines(screenWidth/2, screenHeight/2, 30, BLACK);

            // Draw bullets
            if (drawInPerformanceMode)
            {
                // Draw bullets using pre-rendered texture containing circle
                for (int i = 0; i < bulletCount; i++)
                {
                    // Do not draw disabled bullets (out of screen)
                    if (!bullets[i].disabled)
                    {
                        DrawTexture(bulletTexture.texture,
                            (int)(bullets[i].position.x - bulletTexture.texture.width*0.5f),
                            (int)(bullets[i].position.y - bulletTexture.texture.height*0.5f),
                            bullets[i].color);
                    }
                }
            }
            else
            {
                // Draw bullets using DrawCircle(), less performant
                for (int i = 0; i < bulletCount; i++)
                {
                    // Do not draw disabled bullets (out of screen)
                    if (!bullets[i].disabled)
                    {
                        DrawCircleV(bullets[i].position, (float)bulletRadius, bullets[i].color);
                        DrawCircleLinesV(bullets[i].position, (float)bulletRadius, BLACK);
                    }
                }
            }

            // Draw UI
            DrawRectangle(10, 10, 280, 150, (Color){0,0, 0, 200 });
            DrawText("Controls:", 20, 20, 10, LIGHTGRAY);
            DrawText("- Right/Left or A/D: Change rows number", 40, 40, 10, LIGHTGRAY);
            DrawText("- Up/Down or W/S: Change bullet speed", 40, 60, 10, LIGHTGRAY);
            DrawText("- Z or X: Change spawn cooldown", 40, 80, 10, LIGHTGRAY);
            DrawText("- Space (Hold): Change the angle increment", 40, 100, 10, LIGHTGRAY);
            DrawText("- Enter: Switch draw method (Performance)", 40, 120, 10, LIGHTGRAY);
            DrawText("- C: Clear bullets", 40, 140, 10, LIGHTGRAY);

            DrawRectangle(610, 10, 170, 30, (Color){0,0, 0, 200 });
            if (drawInPerformanceMode) DrawText("Draw method: DrawTexture(*)", 620, 20, 10, GREEN);
            else DrawText("Draw method: DrawCircle(*)", 620, 20, 10, RED);

            DrawRectangle(135, 410, 530, 30, (Color){0,0, 0, 200 });
            DrawText(TextFormat("[ FPS: %d, Bullets: %d, Rows: %d, Bullet speed: %.2f, Angle increment per frame: %d, Cooldown: %.0f ]",
                    GetFPS(), bulletCount - bulletDisabledCount, bulletRows, bulletSpeed,  angleIncrement, spawnCooldown),
                155, 420, 10, GREEN);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(bulletTexture); // Unload bullet texture

    RL_FREE(bullets);     // Free bullets array data

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}