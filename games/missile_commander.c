/*******************************************************************************************
*
*   raylib - sample game: missile commander
*
*   Sample game Marc Palau and Ramon Santamaria
*
*   This game has been created using raylib v1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define MAX_MISSILES                100
#define MAX_INTERCEPTORS            30
#define MAX_EXPLOSIONS              100
#define LAUNCHERS_AMOUNT            3           // Not a variable, should not be changed
#define BUILDINGS_AMOUNT            6           // Not a variable, should not be changed

#define LAUNCHER_SIZE               80
#define BUILDING_SIZE               60
#define EXPLOSION_RADIUS            40

#define MISSILE_SPEED               1
#define MISSILE_LAUNCH_FRAMES       80
#define INTERCEPTOR_SPEED           10
#define EXPLOSION_INCREASE_TIME     90          // In frames
#define EXPLOSION_TOTAL_TIME        210         // In frames

#define EXPLOSION_COLOR             (Color){ 125, 125, 125, 125 }

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Missile {
    Vector2 origin;
    Vector2 position;
    Vector2 objective;
    Vector2 speed;

    bool active;
} Missile;

typedef struct Interceptor {
    Vector2 origin;
    Vector2 position;
    Vector2 objective;
    Vector2 speed;

    bool active;
} Interceptor;

typedef struct Explosion {
    Vector2 position;
    float radiusMultiplier;
    int frame;
    bool active;
} Explosion;

typedef struct Launcher {
    Vector2 position;
    bool active;
} Launcher;

typedef struct Building {
    Vector2 position;
    bool active;
} Building;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static int screenWidth = 800;
static int screenHeight = 450;

static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;
static int score = 0;

static Missile missile[MAX_MISSILES] = { 0 };
static Interceptor interceptor[MAX_INTERCEPTORS] = { 0 };
static Explosion explosion[MAX_EXPLOSIONS] = { 0 };
static Launcher launcher[LAUNCHERS_AMOUNT] = { 0 };
static Building building[BUILDINGS_AMOUNT] = { 0 };
static int explosionIndex = 0;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

// Additional module functions
static void UpdateOutgoingFire();
static void UpdateIncomingFire();

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "sample game: missile commander");

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        //----------------------------------------------------------------------------------
        UpdateDrawFrame();
        //----------------------------------------------------------------------------------
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();         // Unload loaded data (textures, sounds, models...)
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------
// Game Module Functions Definition
//--------------------------------------------------------------------------------------

// Initialize game variables
void InitGame(void)
{
    // Initialize missiles
    for (int i = 0; i < MAX_MISSILES; i++)
    {
        missile[i].origin = (Vector2){ 0, 0 };
        missile[i].speed = (Vector2){ 0, 0 };
        missile[i].position = (Vector2){ 0, 0 };

        missile[i].active = false;
    }
    
    // Initialize interceptors
    for (int i = 0; i < MAX_INTERCEPTORS; i++)
    {
        interceptor[i].origin = (Vector2){ 0, 0 };
        interceptor[i].speed = (Vector2){ 0, 0 };
        interceptor[i].position = (Vector2){ 0, 0 };

        interceptor[i].active = false;
    }
    
    // Initialize explosions
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        explosion[i].position = (Vector2){ 0, 0 };
        explosion[i].frame = 0;
        explosion[i].active = false;
    }
    
    // Initialize buildings and launchers
    int sparcing = screenWidth/(LAUNCHERS_AMOUNT + BUILDINGS_AMOUNT + 1);

    // Buildings and launchers placing
    launcher[0].position = (Vector2){ 1*sparcing, screenHeight - LAUNCHER_SIZE/2 };
    building[0].position = (Vector2){ 2*sparcing, screenHeight - BUILDING_SIZE/2 };
    building[1].position = (Vector2){ 3*sparcing, screenHeight - BUILDING_SIZE/2 };
    building[2].position = (Vector2){ 4*sparcing, screenHeight - BUILDING_SIZE/2 };
    launcher[1].position = (Vector2){ 5*sparcing, screenHeight - LAUNCHER_SIZE/2 };
    building[3].position = (Vector2){ 6*sparcing, screenHeight - BUILDING_SIZE/2 };
    building[4].position = (Vector2){ 7*sparcing, screenHeight - BUILDING_SIZE/2 };
    building[5].position = (Vector2){ 8*sparcing, screenHeight - BUILDING_SIZE/2 };
    launcher[2].position = (Vector2){ 9*sparcing, screenHeight - LAUNCHER_SIZE/2 };

    // Buildings and launchers activation
    for (int i = 0; i < LAUNCHERS_AMOUNT; i++) launcher[i].active = true;
    for (int i = 0; i < BUILDINGS_AMOUNT; i++) building[i].active = true;

    // Initialize game variables
    score = 0;
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            framesCounter++;

            static 
            float distance;

            // Interceptors update
            for (int i = 0; i < MAX_INTERCEPTORS; i++)
            {
                if (interceptor[i].active)
                {
                    // Update position
                    interceptor[i].position.x += interceptor[i].speed.x;
                    interceptor[i].position.y += interceptor[i].speed.y;

                    // Distance to objective
                    distance = sqrt( pow(interceptor[i].position.x - interceptor[i].objective.x, 2) +
                                     pow(interceptor[i].position.y - interceptor[i].objective.y, 2));

                    if (distance < INTERCEPTOR_SPEED)
                    {
                        // Interceptor dissapears
                        interceptor[i].active = false;

                        // Explosion
                        explosion[explosionIndex].position = interceptor[i].position;
                        explosion[explosionIndex].active = true;
                        explosion[explosionIndex].frame = 0;
                        explosionIndex++;
                        if (explosionIndex == MAX_EXPLOSIONS) explosionIndex = 0;

                        break;
                    }
                }
            }

            // Missiles update
            for (int i = 0; i < MAX_MISSILES; i++)
            {
                if (missile[i].active)
                {
                    // Update position
                    missile[i].position.x += missile[i].speed.x;
                    missile[i].position.y += missile[i].speed.y;

                    // Collision and missile out of bounds
                    if (missile[i].position.y > screenHeight) missile[i].active = false;
                    else
                    {
                        // CHeck collision with launchers
                        for (int j = 0; j < LAUNCHERS_AMOUNT; j++)
                        {
                            if (launcher[j].active)
                            {
                                if (CheckCollisionPointRec(missile[i].position,  (Rectangle){ launcher[j].position.x - LAUNCHER_SIZE/2, launcher[j].position.y - LAUNCHER_SIZE/2,
                                                                                            LAUNCHER_SIZE, LAUNCHER_SIZE }))
                                {
                                    // Missile dissapears
                                    missile[i].active = false;

                                    // Explosion and destroy building
                                    launcher[j].active = false;

                                    explosion[explosionIndex].position = missile[i].position;
                                    explosion[explosionIndex].active = true;
                                    explosion[explosionIndex].frame = 0;
                                    explosionIndex++;
                                    if (explosionIndex == MAX_EXPLOSIONS) explosionIndex = 0;

                                    break;
                                }
                            }
                        }

                        // CHeck collision with buildings
                        for (int j = 0; j < BUILDINGS_AMOUNT; j++)
                        {
                            if (building[j].active)
                            {
                                if (CheckCollisionPointRec(missile[i].position,  (Rectangle){ building[j].position.x - BUILDING_SIZE/2, building[j].position.y - BUILDING_SIZE/2,
                                                                                            BUILDING_SIZE, BUILDING_SIZE }))
                                {
                                    // Missile dissapears
                                    missile[i].active = false;

                                    // Explosion and destroy building
                                    building[j].active = false;

                                    explosion[explosionIndex].position = missile[i].position;
                                    explosion[explosionIndex].active = true;
                                    explosion[explosionIndex].frame = 0;
                                    explosionIndex++;
                                    if (explosionIndex == MAX_EXPLOSIONS) explosionIndex = 0;

                                    break;
                                }
                            }
                        }

                        // CHeck collision with explosions
                        for (int j = 0; j < MAX_EXPLOSIONS; j++)
                        {
                            if (explosion[j].active)
                            {
                                if (CheckCollisionPointCircle(missile[i].position, explosion[j].position, EXPLOSION_RADIUS*explosion[j].radiusMultiplier))
                                {
                                    // Missile dissapears and we earn 100 points
                                    missile[i].active = false;
                                    score += 100;

                                    explosion[explosionIndex].position = missile[i].position;
                                    explosion[explosionIndex].active = true;
                                    explosion[explosionIndex].frame = 0;
                                    explosionIndex++;
                                    if (explosionIndex == MAX_EXPLOSIONS) explosionIndex = 0;

                                    break;
                                }
                            }
                        }
                    }
                }
            }

            // Explosions update
            for (int i = 0; i < MAX_EXPLOSIONS; i++)
            {
                if (explosion[i].active)
                {
                    explosion[i].frame++;

                    if (explosion[i].frame <= EXPLOSION_INCREASE_TIME) explosion[i].radiusMultiplier = explosion[i].frame/(float)EXPLOSION_INCREASE_TIME;
                    else if (explosion[i].frame <= EXPLOSION_TOTAL_TIME) explosion[i].radiusMultiplier = 1 - (explosion[i].frame - (float)EXPLOSION_INCREASE_TIME)/(float)EXPLOSION_TOTAL_TIME;
                    else
                    {
                        explosion[i].frame = 0;
                        explosion[i].active = false;
                    }
                }
            }

            // Fire logic
            UpdateOutgoingFire();
            UpdateIncomingFire();

            // Game over logic
            int checker = 0;

            for (int i = 0; i < LAUNCHERS_AMOUNT; i++)
            {
                if (!launcher[i].active) checker++;
                if (checker == LAUNCHERS_AMOUNT) gameOver = true;
            }

            checker = 0;
            for (int i = 0; i < BUILDINGS_AMOUNT; i++)
            {
                if (!building[i].active) checker++;
                if (checker == BUILDINGS_AMOUNT) gameOver = true;
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
    }
}

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(RAYWHITE);

        if (!gameOver)
        {
            // Draw missiles
            for (int i = 0; i < MAX_MISSILES; i++)
            {
                if (missile[i].active)
                {
                    DrawLine(missile[i].origin.x, missile[i].origin.y, missile[i].position.x, missile[i].position.y, RED);

                    if (framesCounter % 16 < 8) DrawCircle(missile[i].position.x, missile[i].position.y, 3, YELLOW);
                }
            }
            
            // Draw interceptors
            for (int i = 0; i < MAX_INTERCEPTORS; i++)
            {
                if (interceptor[i].active)
                {
                    DrawLine(interceptor[i].origin.x, interceptor[i].origin.y, interceptor[i].position.x, interceptor[i].position.y, GREEN);

                    if (framesCounter % 16 < 8) DrawCircle(interceptor[i].position.x, interceptor[i].position.y, 3, BLUE);
                }
            }
            
            // Draw explosions
            for (int i = 0; i < MAX_EXPLOSIONS; i++)
            {
                if (explosion[i].active) DrawCircle(explosion[i].position.x, explosion[i].position.y, EXPLOSION_RADIUS*explosion[i].radiusMultiplier, EXPLOSION_COLOR);
            }

            // Draw buildings and launchers
            for (int i = 0; i < LAUNCHERS_AMOUNT; i++)
            {
                if (launcher[i].active) DrawRectangle(launcher[i].position.x - LAUNCHER_SIZE/2, launcher[i].position.y - LAUNCHER_SIZE/2, LAUNCHER_SIZE, LAUNCHER_SIZE, GRAY);
            }

            for (int i = 0; i < BUILDINGS_AMOUNT; i++)
            {
                if (building[i].active) DrawRectangle(building[i].position.x - BUILDING_SIZE/2, building[i].position.y - BUILDING_SIZE/2, BUILDING_SIZE, BUILDING_SIZE, LIGHTGRAY);
            }

            // Draw score
            DrawText(TextFormat("SCORE %4i", score), 20, 20, 40, LIGHTGRAY);
            
            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);

    EndDrawing();
}

// Unload game variables
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}

//--------------------------------------------------------------------------------------
// Additional module functions
//--------------------------------------------------------------------------------------
static void UpdateOutgoingFire()
{
    static int interceptorNumber = 0;
    int launcherShooting = 0;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) launcherShooting = 1;
    if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) launcherShooting = 2;
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) launcherShooting = 3;

    if (launcherShooting > 0 && launcher[launcherShooting - 1].active)
    {
        float module;
        float sideX;
        float sideY;

        // Activate the interceptor
        interceptor[interceptorNumber].active = true;

        // Assign start position
        interceptor[interceptorNumber].origin = launcher[launcherShooting - 1].position;
        interceptor[interceptorNumber].position = interceptor[interceptorNumber].origin;
        interceptor[interceptorNumber].objective = GetMousePosition();

        // Calculate speed
        module = sqrt( pow(interceptor[interceptorNumber].objective.x - interceptor[interceptorNumber].origin.x, 2) +
                       pow(interceptor[interceptorNumber].objective.y - interceptor[interceptorNumber].origin.y, 2));

        sideX = (interceptor[interceptorNumber].objective.x - interceptor[interceptorNumber].origin.x)*INTERCEPTOR_SPEED/module;
        sideY = (interceptor[interceptorNumber].objective.y - interceptor[interceptorNumber].origin.y)*INTERCEPTOR_SPEED/module;

        interceptor[interceptorNumber].speed = (Vector2){ sideX, sideY };

        // Update
        interceptorNumber++;
        if (interceptorNumber == MAX_INTERCEPTORS) interceptorNumber = 0;
    }
}

static void UpdateIncomingFire()
{
    static int missileIndex = 0;

    // Launch missile
    if (framesCounter%MISSILE_LAUNCH_FRAMES == 0)
    {
        float module;
        float sideX;
        float sideY;

        // Activate the missile
        missile[missileIndex].active = true;

        // Assign start position
        missile[missileIndex].origin = (Vector2){ GetRandomValue(20, screenWidth - 20), -10 };
        missile[missileIndex].position = missile[missileIndex].origin;
        missile[missileIndex].objective = (Vector2){ GetRandomValue(20, screenWidth - 20), screenHeight + 10 };

        // Calculate speed
        module = sqrt( pow(missile[missileIndex].objective.x - missile[missileIndex].origin.x, 2) +
                       pow(missile[missileIndex].objective.y - missile[missileIndex].origin.y, 2));

        sideX = (missile[missileIndex].objective.x - missile[missileIndex].origin.x)*MISSILE_SPEED/module;
        sideY = (missile[missileIndex].objective.y - missile[missileIndex].origin.y)*MISSILE_SPEED/module;

        missile[missileIndex].speed = (Vector2){ sideX, sideY };

        // Update
        missileIndex++;
        if (missileIndex == MAX_MISSILES) missileIndex = 0;
    }
}
