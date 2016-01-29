/*******************************************************************************************
*
*   raylib - sample game: pang
*
*   Sample game developed by Ian Eito, Albert Martos and Ramon Santamaria
*
*   This game has been created using raylib v1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <math.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define MAX_SPEED           5
#define METEORS_SPEED       2
#define NUM_SHOOTS          1
#define NUM_BIG_METEORS     2
#define NUM_MEDIUM_METEORS  4
#define NUM_SMALL_METEORS   8
#define SHIP_BASE_SIZE      20.0f

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef struct Player {
    Vector2 position;
    Vector2 speed;
    float rotation;
    Vector3 collider;
    Color color;
} Player;

typedef struct Shoot {
    Vector2 position;
    Vector2 speed;
    float radius;
    float rotation;
    int lifeSpawn;
    bool active;
    Color color;
} Shoot;

typedef struct BigMeteor {
    Vector2 position;
    Vector2 speed;
    float radius;
    int points;
    bool active;
    Color color;
} BigMeteor;

typedef struct MediumMeteor {
    Vector2 position;
    Vector2 speed;
    float radius;
    int points;
    bool active;
    Color color;
} MediumMeteor;

typedef struct SmallMeteor {
    Vector2 position;
    Vector2 speed;
    float radius;
    int points;
    bool active;
    Color color;
} SmallMeteor;

typedef struct Points {
    char letter;
    Vector2 position;
    int value;
    Color color;
    float alpha;
} Points;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static int screenWidth = 800;
static int screenHeight = 450;

static int framesCounter;
static bool gameOver;
static bool pause;
static int score;

static Player player;
static Shoot shoot[NUM_SHOOTS];
static BigMeteor bigMeteor[NUM_BIG_METEORS];
static MediumMeteor mediumMeteor[NUM_MEDIUM_METEORS];
static SmallMeteor smallMeteor[NUM_SMALL_METEORS];
static Points points[5];

// NOTE: Defined triangle is isosceles with common angles of 70 degrees.
static float shipHeight;
static float gravity;

static int countMediumMeteors;
static int countSmallMeteors;
static int meteorsDestroyed;
static Vector2 linePosition;

static bool victory;
static bool lose;
static bool awake;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

static void InitShoot(Shoot shoot);
static void DrawSpaceship(Vector2 position, float rotation, Color color);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    InitWindow(screenWidth, screenHeight, "sample game: pang");

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateGame();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        DrawGame();
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

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

// Initialize game variables
static void InitGame(void)
{
    int posx, posy;
    int velx = 0;
    int vely = 0;
    
    framesCounter = 0;
    gameOver = false;
    pause = false;
    score = 0;
    
    victory = false;
    lose = false;
    awake = true;
    gravity = 0.25f;

    linePosition = (Vector2){ 0.0f , 0.0f };
    shipHeight = (SHIP_BASE_SIZE/2)/tanf(20*DEG2RAD);

    // Initialization player
    player.position = (Vector2){ screenWidth/2, screenHeight };
    player.speed = (Vector2){ MAX_SPEED, MAX_SPEED };
    player.rotation = 0;
    player.collider = (Vector3){ player.position.x, player.position.y - shipHeight/2.0f, 12.0f };
    player.color = LIGHTGRAY;

    meteorsDestroyed = 0;

    // Initialize shoots
    for (int i = 0; i < NUM_SHOOTS; i++)
    {
        shoot[i].position = (Vector2){ 0, 0 };
        shoot[i].speed = (Vector2){ 0, 0 };
        shoot[i].radius = 2;
        shoot[i].active = false;
        shoot[i].lifeSpawn = 0;
        shoot[i].color = WHITE;
    }

    // Initialize big meteors
    for (int i = 0; i < NUM_BIG_METEORS; i++)
    {
        bigMeteor[i].radius = 40.0f;
        posx = GetRandomValue(0 + bigMeteor[i].radius, screenWidth - bigMeteor[i].radius);
        posy = GetRandomValue(0 + bigMeteor[i].radius, screenHeight/2);

        bigMeteor[i].position = (Vector2){ posx, posy };

        while ((velx == 0) || (vely == 0))
        {
            velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
            vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
        }

        bigMeteor[i].speed = (Vector2){ velx, vely };
        bigMeteor[i].points = 200;
        bigMeteor[i].active = true;
        bigMeteor[i].color = BLUE;
    }

    // Initialize medium meteors
    for (int i = 0; i < NUM_MEDIUM_METEORS; i++)
    {
        mediumMeteor[i].position = (Vector2){-100, -100};
        mediumMeteor[i].speed = (Vector2){0,0};
        mediumMeteor[i].radius = 20.0f;
        mediumMeteor[i].points = 100;
        mediumMeteor[i].active = false;
        mediumMeteor[i].color = BLUE;
    }

    // Initialize small meteors
    for (int i = 0; i < NUM_SMALL_METEORS; i++)
    {
        smallMeteor[i].position = (Vector2){ -100, -100 };
        smallMeteor[i].speed = (Vector2){ 0, 0 };
        smallMeteor[i].radius = 10.0f;
        smallMeteor[i].points = 50;
        smallMeteor[i].active = false;
        smallMeteor[i].color = BLUE;
    }

    // Initialize animated points
    for (int i = 0; i < 5; i++)
    {
        points[i].position = (Vector2){ 0, 0 };
        points[i].value = 0;
        points[i].alpha = 0.0f;
    }

    countMediumMeteors = 0;
    countSmallMeteors = 0;
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            if (awake)
            {
                // Player logic
                if (IsKeyDown(KEY_LEFT))  player.position.x -= player.speed.x;
                if (IsKeyDown(KEY_RIGHT))  player.position.x += player.speed.x;

                // Wall behaviour for player
                if (player.position.x + SHIP_BASE_SIZE/2 > screenWidth) player.position.x = screenWidth - SHIP_BASE_SIZE/2;
                else if (player.position.x - SHIP_BASE_SIZE/2 < 0) player.position.x = 0 + SHIP_BASE_SIZE/2;

                 // Activation of shoot
                if (IsKeyPressed(KEY_SPACE))
                {
                    for (int i = 0; i < NUM_SHOOTS; i++)
                    {
                        if (!shoot[i].active)
                        {
                            shoot[i].position = (Vector2){ player.position.x, player.position.y - shipHeight };
                            linePosition = (Vector2){ player.position.x, player.position.y};
                            shoot[i].active = true;
                            shoot[i].speed.y = MAX_SPEED;
                            break;
                        }
                    }
                }

                // Shoot life timer
                for (int i = 0; i < NUM_SHOOTS; i++)
                {
                    if (shoot[i].active) shoot[i].lifeSpawn++;
                }

                // Shot logic
                for (int i = 0; i < NUM_SHOOTS; i++)
                {
                    if (shoot[i].active)
                    {
                        // Movement
                        shoot[i].position.y -= shoot[i].speed.y;

                        // Wall behaviour for shoot
                        if (shoot[i].position.x > screenWidth + shoot[i].radius)
                        {
                            shoot[i].active = false;
                            shoot[i].lifeSpawn = 0;
                        }
                        else if (shoot[i].position.x < 0 - shoot[i].radius)
                        {
                            shoot[i].active = false;
                            shoot[i].lifeSpawn = 0;
                        }
                        
                        if (shoot[i].position.y > screenHeight + shoot[i].radius)
                        {
                            shoot[i].active = false;
                            shoot[i].lifeSpawn = 0;
                        }
                        else if (shoot[i].position.y < 0 - shoot[i].radius)
                        {
                            shoot[i].active = false;
                            shoot[i].lifeSpawn = 0;
                        }

                        // Life of shoot
                        if (shoot[i].lifeSpawn >= 120)
                        {
                            shoot[i].position = (Vector2){0, 0};
                            shoot[i].speed = (Vector2){0, 0};
                            shoot[i].lifeSpawn = 0;
                            shoot[i].active = false;
                        }
                    }
                }

                // Player collision with meteors
                player.collider = (Vector3){player.position.x, player.position.y - shipHeight/2, 12};

                for (int i = 0; i < NUM_BIG_METEORS; i++)
                {
                    if (CheckCollisionCircles((Vector2){ player.collider.x, player.collider.y }, player.collider.z, bigMeteor[i].position, bigMeteor[i].radius) && bigMeteor[i].active)
                    {
                        gameOver = true;
                    }
                }

                for (int i = 0; i < NUM_MEDIUM_METEORS; i++)
                {
                    if (CheckCollisionCircles((Vector2){ player.collider.x, player.collider.y }, player.collider.z, mediumMeteor[i].position, mediumMeteor[i].radius) && mediumMeteor[i].active)
                    {
                        gameOver = true;
                    }
                }

                for (int i = 0; i < NUM_SMALL_METEORS; i++)
                {
                    if (CheckCollisionCircles((Vector2){ player.collider.x, player.collider.y }, player.collider.z, smallMeteor[i].position, smallMeteor[i].radius) && smallMeteor[i].active)
                    {
                        gameOver = true;
                    }
                }

                // Meteor logic
                for (int i = 0; i < NUM_BIG_METEORS; i++)
                {
                    if (bigMeteor[i].active)
                    {
                        // movement
                        bigMeteor[i].position.x += bigMeteor[i].speed.x;
                        bigMeteor[i].position.y += bigMeteor[i].speed.y;

                        // wall behaviour
                        if (((bigMeteor[i].position.x + bigMeteor[i].radius) >= screenWidth) || ((bigMeteor[i].position.x - bigMeteor[i].radius) <= 0)) bigMeteor[i].speed.x *= -1;
                        if ((bigMeteor[i].position.y - bigMeteor[i].radius) <= 0) bigMeteor[i].speed.y *= -1.5;
                        
                        if ((bigMeteor[i].position.y + bigMeteor[i].radius) >= screenHeight)
                        {
                            bigMeteor[i].speed.y *= -1;
                            bigMeteor[i].position.y = screenHeight - bigMeteor[i].radius;
                        }

                        bigMeteor[i].speed.y += gravity;
                    }
                }

                for (int i = 0; i < NUM_MEDIUM_METEORS; i++)
                {
                    if (mediumMeteor[i].active)
                    {
                        // Movement logic
                        mediumMeteor[i].position.x += mediumMeteor[i].speed.x;
                        mediumMeteor[i].position.y += mediumMeteor[i].speed.y;

                        // Wall behaviour
                        if (mediumMeteor[i].position.x + mediumMeteor[i].radius >= screenWidth || mediumMeteor[i].position.x - mediumMeteor[i].radius <= 0) mediumMeteor[i].speed.x *= -1;
                        if (mediumMeteor[i].position.y - mediumMeteor[i].radius <= 0) mediumMeteor[i].speed.y *= -1;
                        if (mediumMeteor[i].position.y + mediumMeteor[i].radius >= screenHeight)
                        {
                            mediumMeteor[i].speed.y *= -1;
                            mediumMeteor[i].position.y = screenHeight - mediumMeteor[i].radius;
                        }

                        mediumMeteor[i].speed.y += gravity + 0.12f;
                    }
                }

                for (int i = 0; i < NUM_SMALL_METEORS; i++)
                {
                    if (smallMeteor[i].active)
                    {
                        // movement
                        smallMeteor[i].position.x += smallMeteor[i].speed.x;
                        smallMeteor[i].position.y += smallMeteor[i].speed.y;

                        // wall behaviour
                        if (smallMeteor[i].position.x + smallMeteor[i].radius >= screenWidth || smallMeteor[i].position.x - smallMeteor[i].radius <= 0) smallMeteor[i].speed.x *= -1;
                        if (smallMeteor[i].position.y - smallMeteor[i].radius <= 0) smallMeteor[i].speed.y *= -1;
                        if (smallMeteor[i].position.y + smallMeteor[i].radius >= screenHeight)
                        {
                            smallMeteor[i].speed.y *= -1;
                            smallMeteor[i].position.y = screenHeight - smallMeteor[i].radius;
                        }

                        smallMeteor[i].speed.y += gravity + 0.25f;
                    }
                }

                // Collision behaviour
                for (int i = 0; i < NUM_SHOOTS; i++)
                {
                    if ((shoot[i].active))
                    {
                        for (int a = 0; a < NUM_BIG_METEORS; a++)
                        {
                            if (bigMeteor[a].active && (bigMeteor[a].position.x - bigMeteor[a].radius <= linePosition.x && bigMeteor[a].position.x + bigMeteor[a].radius >= linePosition.x)
                                && (bigMeteor[a].position.y + bigMeteor[a].radius >= shoot[i].position.y))
                            {
                                shoot[i].active = false;
                                shoot[i].lifeSpawn = 0;
                                bigMeteor[a].active = false;
                                meteorsDestroyed++;
                                score +=  bigMeteor[a].points;

                                for (int z = 0; z < 5; z++)
                                {
                                    if (points[z].alpha == 0.0f)
                                    {
                                        points[z].position = bigMeteor[a].position;
                                        points[z].value = bigMeteor[a].points;
                                        points[z].color = RED;
                                        points[z].alpha = 1.0f;
                                        z = 5;
                                    }
                                }

                                for (int j = 0; j < 2; j ++)
                                {
                                    if ((countMediumMeteors%2) == 0)
                                    {
                                        mediumMeteor[countMediumMeteors].position = (Vector2){bigMeteor[a].position.x, bigMeteor[a].position.y};
                                        mediumMeteor[countMediumMeteors].speed = (Vector2){METEORS_SPEED*-1, METEORS_SPEED};
                                    }
                                    else
                                    {
                                        mediumMeteor[countMediumMeteors].position = (Vector2){bigMeteor[a].position.x, bigMeteor[a].position.y};
                                        mediumMeteor[countMediumMeteors].speed = (Vector2){METEORS_SPEED, METEORS_SPEED};
                                    }

                                    mediumMeteor[countMediumMeteors].active = true;
                                    countMediumMeteors ++;
                                }

                                bigMeteor[a].color = RED;
                                a = NUM_BIG_METEORS;
                            }
                        }
                    }

                    if ((shoot[i].active))
                    {
                        for (int b = 0; b < NUM_MEDIUM_METEORS; b++)
                        {
                            if (mediumMeteor[b].active && (mediumMeteor[b].position.x - mediumMeteor[b].radius <= linePosition.x && mediumMeteor[b].position.x + mediumMeteor[b].radius >= linePosition.x)
                                && (mediumMeteor[b].position.y + mediumMeteor[b].radius >= shoot[i].position.y))
                            {
                                shoot[i].active = false;
                                shoot[i].lifeSpawn = 0;
                                mediumMeteor[b].active = false;
                                meteorsDestroyed++;
                                score +=  mediumMeteor[b].points;

                                for (int z = 0; z < 5; z++)
                                {
                                    if (points[z].alpha == 0.0f)
                                    {
                                        points[z].position = mediumMeteor[b].position;
                                        points[z].value = mediumMeteor[b].points;
                                        points[z].color = GREEN;
                                        points[z].alpha = 1.0f;
                                        z = 5;
                                    }
                                }

                                for (int j = 0; j < 2; j ++)
                                {
                                     if (countSmallMeteors%2 == 0)
                                    {
                                        smallMeteor[countSmallMeteors].position = (Vector2){mediumMeteor[b].position.x, mediumMeteor[b].position.y};
                                        smallMeteor[countSmallMeteors].speed = (Vector2){METEORS_SPEED*-1, METEORS_SPEED*-1};
                                    }
                                    else
                                    {
                                        smallMeteor[countSmallMeteors].position = (Vector2){mediumMeteor[b].position.x, mediumMeteor[b].position.y};
                                        smallMeteor[countSmallMeteors].speed = (Vector2){METEORS_SPEED, METEORS_SPEED*-1};
                                    }

                                    smallMeteor[countSmallMeteors].active = true;
                                    countSmallMeteors ++;
                                }
                                mediumMeteor[b].color = GREEN;
                                b = NUM_MEDIUM_METEORS;
                            }
                        }
                    }

                    if ((shoot[i].active))
                    {
                        for (int c = 0; c < NUM_SMALL_METEORS; c++)
                        {
                            if (smallMeteor[c].active && (smallMeteor[c].position.x - smallMeteor[c].radius <= linePosition.x && smallMeteor[c].position.x + smallMeteor[c].radius >= linePosition.x)
                                && (smallMeteor[c].position.y + smallMeteor[c].radius >= shoot[i].position.y))
                            {
                                shoot[i].active = false;
                                shoot[i].lifeSpawn = 0;
                                smallMeteor[c].active = false;
                                meteorsDestroyed++;
                                smallMeteor[c].color = YELLOW;
                                score +=  smallMeteor[c].points;

                                for (int z = 0; z < 5; z++)
                                {
                                    if (points[z].alpha == 0.0f)
                                    {
                                        points[z].position = smallMeteor[c].position;
                                        points[z].value = smallMeteor[c].points;
                                        points[z].color = YELLOW;
                                        points[z].alpha = 1.0f;
                                        z = 5;
                                    }
                                }

                                c = NUM_SMALL_METEORS;
                            }
                        }
                    }
                }

                for (int z = 0; z < 5; z++)
                {
                    if (points[z].alpha > 0.0f)
                    {
                        points[z].position.y -= 2;
                        points[z].alpha -= 0.02f;
                    }
                    
                    if (points[z].alpha < 0.0f) points[z].alpha = 0.0f;
                }
                
                if (meteorsDestroyed == (NUM_BIG_METEORS + NUM_MEDIUM_METEORS + NUM_SMALL_METEORS)) victory = true;
            }
            else
            {
                framesCounter++;
                if (framesCounter%180 == 0) awake = false;
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

        ClearBackground(DARKGRAY);
        
        if (!gameOver)
        {
            // Draw player
            Vector2 v1 = { player.position.x + sinf(player.rotation*DEG2RAD)*(shipHeight), player.position.y - cosf(player.rotation*DEG2RAD)*(shipHeight) };
            Vector2 v2 = { player.position.x - cosf(player.rotation*DEG2RAD)*(SHIP_BASE_SIZE/2), player.position.y - sinf(player.rotation*DEG2RAD)*(SHIP_BASE_SIZE/2) };
            Vector2 v3 = { player.position.x + cosf(player.rotation*DEG2RAD)*(SHIP_BASE_SIZE/2), player.position.y + sinf(player.rotation*DEG2RAD)*(SHIP_BASE_SIZE/2) };
            DrawTriangleLines(v1, v2, v3, player.color);

            // Draw meteor
            for (int i = 0;i < NUM_BIG_METEORS; i++)
            {
                if (bigMeteor[i].active) DrawCircleV(bigMeteor[i].position, bigMeteor[i].radius, bigMeteor[i].color);
                else
                {
                    DrawCircleV(bigMeteor[i].position, bigMeteor[i].radius, Fade(bigMeteor[i].color, 0.25f));
                    //DrawText(FormatText("%i", bigMeteor[i].points), bigMeteor[i].position.x  - MeasureText("200", 20)/2, bigMeteor[i].position.y - 10, 20, Fade(WHITE, 0.25f));
                }
            }

            for (int i = 0;i < NUM_MEDIUM_METEORS; i++)
            {
                if (mediumMeteor[i].active) DrawCircleV(mediumMeteor[i].position, mediumMeteor[i].radius, mediumMeteor[i].color);
                else
                {
                    DrawCircleV(mediumMeteor[i].position, mediumMeteor[i].radius, Fade(mediumMeteor[i].color, 0.25f));
                    //DrawText(FormatText("%i", mediumMeteor[i].points), mediumMeteor[i].position.x  - MeasureText("100", 20)/2, mediumMeteor[i].position.y - 10, 20, Fade(WHITE, 0.25f));
                }
            }

            for (int i = 0;i < NUM_SMALL_METEORS; i++)
            {
                if (smallMeteor[i].active) DrawCircleV(smallMeteor[i].position, smallMeteor[i].radius, smallMeteor[i].color);
                else
                {
                    DrawCircleV(smallMeteor[i].position, smallMeteor[i].radius, Fade(smallMeteor[i].color, 0.25f));
                    //DrawText(FormatText("%i", smallMeteor[i].points), smallMeteor[i].position.x - MeasureText("50", 10)/2, smallMeteor[i].position.y - 5, 10, Fade(WHITE, 0.25f));
                }
            }

            // Draw shoot

            for (int i = 0; i < NUM_SHOOTS; i++)
            {
                if (shoot[i].active) DrawLine(linePosition.x, linePosition.y, shoot[i].position.x, shoot[i].position.y, RED);
            }

            for (int z = 0; z < 5; z++)
            {
                if (points[z].alpha > 0.0f)
                {
                    DrawText(FormatText("+%i", points[z].value), points[z].position.x, points[z].position.y, 20, Fade(points[z].color, points[z].alpha));
                }
            }

            // Draw Text
            DrawText(FormatText("SCORE: %i", score), 10, 10, 20, LIGHTGRAY);
            
            if (victory) DrawText("VICTORY", screenWidth/2 - MeasureText("VICTORY", 40)/2, screenHeight/2 - 40, 40, LIGHTGRAY);
            
            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, LIGHTGRAY);
        }
        else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, LIGHTGRAY);

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