/*******************************************************************************************
*
*   raylib - sample game: pang
*
*   Sample game developed by Ian Eito and Albert Martos and Ramon Santamaria
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
#define PLAYER_BASE_SIZE    20.0f
#define PLAYER_SPEED        5.0f
#define PLAYER_MAX_SHOOTS   1

#define MAX_BIG_BALLS       2
#define BALLS_SPEED         2.0f

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Player {
    Vector2 position;
    Vector2 speed;
    Vector3 collider;
    float rotation;
} Player;

typedef struct Shoot {
    Vector2 position;
    Vector2 speed;
    float radius;
    float rotation;
    int lifeSpawn;
    bool active;
} Shoot;

typedef struct Ball {
    Vector2 position;
    Vector2 speed;
    float radius;
    int points;
    bool active;
} Ball;

typedef struct Points {
    Vector2 position;
    int value;
    float alpha;
} Points;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;
static int score = 0;

static Player player = { 0 };
static Shoot shoot[PLAYER_MAX_SHOOTS] = { 0 };
static Ball bigBalls[MAX_BIG_BALLS] = { 0 };
static Ball mediumBalls[MAX_BIG_BALLS*2] = { 0 };
static Ball smallBalls[MAX_BIG_BALLS*4] = { 0 };
static Points points[5] = { 0 };

// NOTE: Defined triangle is isosceles with common angles of 70 degrees.
static float shipHeight = 0.0f;
static float gravity = 0.0f;

static int countmediumBallss = 0;
static int countsmallBallss = 0;
static int meteorsDestroyed = 0;
static Vector2 linePosition = { 0 };

static bool victory = false;
static bool lose = false;
static bool awake = false;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
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
    shipHeight = (PLAYER_BASE_SIZE/2)/tanf(20*DEG2RAD);

    // Initialization player
    player.position = (Vector2){ screenWidth/2, screenHeight };
    player.speed = (Vector2){ PLAYER_SPEED, PLAYER_SPEED };
    player.rotation = 0;
    player.collider = (Vector3){ player.position.x, player.position.y - shipHeight/2.0f, 12.0f };

    meteorsDestroyed = 0;

    // Initialize shoots
    for (int i = 0; i < PLAYER_MAX_SHOOTS; i++)
    {
        shoot[i].position = (Vector2){ 0, 0 };
        shoot[i].speed = (Vector2){ 0, 0 };
        shoot[i].radius = 2;
        shoot[i].active = false;
        shoot[i].lifeSpawn = 0;
    }

    // Initialize big meteors
    for (int i = 0; i < MAX_BIG_BALLS; i++)
    {
        bigBalls[i].radius = 40.0f;
        posx = GetRandomValue(0 + bigBalls[i].radius, screenWidth - bigBalls[i].radius);
        posy = GetRandomValue(0 + bigBalls[i].radius, screenHeight/2);

        bigBalls[i].position = (Vector2){ posx, posy };

        while ((velx == 0) || (vely == 0))
        {
            velx = GetRandomValue(-BALLS_SPEED, BALLS_SPEED);
            vely = GetRandomValue(-BALLS_SPEED, BALLS_SPEED);
        }

        bigBalls[i].speed = (Vector2){ velx, vely };
        bigBalls[i].points = 200;
        bigBalls[i].active = true;
    }

    // Initialize medium meteors
    for (int i = 0; i < MAX_BIG_BALLS*2; i++)
    {
        mediumBalls[i].position = (Vector2){-100, -100};
        mediumBalls[i].speed = (Vector2){0,0};
        mediumBalls[i].radius = 20.0f;
        mediumBalls[i].points = 100;
        mediumBalls[i].active = false;
    }

    // Initialize small meteors
    for (int i = 0; i < MAX_BIG_BALLS*4; i++)
    {
        smallBalls[i].position = (Vector2){ -100, -100 };
        smallBalls[i].speed = (Vector2){ 0, 0 };
        smallBalls[i].radius = 10.0f;
        smallBalls[i].points = 50;
        smallBalls[i].active = false;
    }

    // Initialize animated points
    for (int i = 0; i < 5; i++)
    {
        points[i].position = (Vector2){ 0, 0 };
        points[i].value = 0;
        points[i].alpha = 0.0f;
    }

    countmediumBallss = 0;
    countsmallBallss = 0;
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver && !victory)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            // Player logic
            if (IsKeyDown(KEY_LEFT))  player.position.x -= player.speed.x;
            if (IsKeyDown(KEY_RIGHT))  player.position.x += player.speed.x;

            // Player vs wall collision logic
            if (player.position.x + PLAYER_BASE_SIZE/2 > screenWidth) player.position.x = screenWidth - PLAYER_BASE_SIZE/2;
            else if (player.position.x - PLAYER_BASE_SIZE/2 < 0) player.position.x = 0 + PLAYER_BASE_SIZE/2;

            // Player shot logic
            if (IsKeyPressed(KEY_SPACE))
            {
                for (int i = 0; i < PLAYER_MAX_SHOOTS; i++)
                {
                    if (!shoot[i].active)
                    {
                        shoot[i].position = (Vector2){ player.position.x, player.position.y - shipHeight };
                        shoot[i].speed.y = PLAYER_SPEED;
                        shoot[i].active = true;
                        
                        linePosition = (Vector2){ player.position.x, player.position.y};

                        break;
                    }
                }
            }

            // Shoot life timer
            for (int i = 0; i < PLAYER_MAX_SHOOTS; i++)
            {
                if (shoot[i].active) shoot[i].lifeSpawn++;
            }

            // Shot logic
            for (int i = 0; i < PLAYER_MAX_SHOOTS; i++)
            {
                if (shoot[i].active)
                {
                    shoot[i].position.y -= shoot[i].speed.y;

                    // Shot vs walls collision logic
                    if ((shoot[i].position.x > screenWidth + shoot[i].radius) || (shoot[i].position.x < 0 - shoot[i].radius) ||
                        (shoot[i].position.y > screenHeight + shoot[i].radius) || (shoot[i].position.y < 0 - shoot[i].radius))
                    {
                        shoot[i].active = false;
                        shoot[i].lifeSpawn = 0;
                    }

                    // Player shot life spawn
                    if (shoot[i].lifeSpawn >= 120)
                    {
                        shoot[i].position = (Vector2){ 0.0f, 0.0f };
                        shoot[i].speed = (Vector2){ 0.0f, 0.0f };
                        shoot[i].lifeSpawn = 0;
                        shoot[i].active = false;
                    }
                }
            }

            // Player vs meteors collision logic
            player.collider = (Vector3){player.position.x, player.position.y - shipHeight/2, 12};

            for (int i = 0; i < MAX_BIG_BALLS; i++)
            {
                if (CheckCollisionCircles((Vector2){ player.collider.x, player.collider.y }, player.collider.z, bigBalls[i].position, bigBalls[i].radius) && bigBalls[i].active)
                {
                    gameOver = true;
                }
            }

            for (int i = 0; i < MAX_BIG_BALLS*2; i++)
            {
                if (CheckCollisionCircles((Vector2){ player.collider.x, player.collider.y }, player.collider.z, mediumBalls[i].position, mediumBalls[i].radius) && mediumBalls[i].active)
                {
                    gameOver = true;
                }
            }

            for (int i = 0; i < MAX_BIG_BALLS*4; i++)
            {
                if (CheckCollisionCircles((Vector2){ player.collider.x, player.collider.y }, player.collider.z, smallBalls[i].position, smallBalls[i].radius) && smallBalls[i].active)
                {
                    gameOver = true;
                }
            }

            // Meteors logic (big)
            for (int i = 0; i < MAX_BIG_BALLS; i++)
            {
                if (bigBalls[i].active)
                {
                    // Meteor movement logic
                    bigBalls[i].position.x += bigBalls[i].speed.x;
                    bigBalls[i].position.y += bigBalls[i].speed.y;

                    // Meteor vs wall collision logic
                    if (((bigBalls[i].position.x + bigBalls[i].radius) >= screenWidth) || ((bigBalls[i].position.x - bigBalls[i].radius) <= 0)) bigBalls[i].speed.x *= -1;
                    if ((bigBalls[i].position.y - bigBalls[i].radius) <= 0) bigBalls[i].speed.y *= -1.5;
                    
                    if ((bigBalls[i].position.y + bigBalls[i].radius) >= screenHeight)
                    {
                        bigBalls[i].speed.y *= -1;
                        bigBalls[i].position.y = screenHeight - bigBalls[i].radius;
                    }

                    bigBalls[i].speed.y += gravity;
                }
            }

            // Meteors logic (medium)
            for (int i = 0; i < MAX_BIG_BALLS*2; i++)
            {
                if (mediumBalls[i].active)
                {
                    // Meteor movement logic
                    mediumBalls[i].position.x += mediumBalls[i].speed.x;
                    mediumBalls[i].position.y += mediumBalls[i].speed.y;

                    // Meteor vs wall collision logic
                    if (mediumBalls[i].position.x + mediumBalls[i].radius >= screenWidth || mediumBalls[i].position.x - mediumBalls[i].radius <= 0) mediumBalls[i].speed.x *= -1;
                    if (mediumBalls[i].position.y - mediumBalls[i].radius <= 0) mediumBalls[i].speed.y *= -1;
                    if (mediumBalls[i].position.y + mediumBalls[i].radius >= screenHeight)
                    {
                        mediumBalls[i].speed.y *= -1;
                        mediumBalls[i].position.y = screenHeight - mediumBalls[i].radius;
                    }

                    mediumBalls[i].speed.y += gravity + 0.12f;
                }
            }

            // Meteors logic (small)
            for (int i = 0; i < MAX_BIG_BALLS*4; i++)
            {
                if (smallBalls[i].active)
                {
                    // Meteor movement logic
                    smallBalls[i].position.x += smallBalls[i].speed.x;
                    smallBalls[i].position.y += smallBalls[i].speed.y;

                    // Meteor vs wall collision logic
                    if (smallBalls[i].position.x + smallBalls[i].radius >= screenWidth || smallBalls[i].position.x - smallBalls[i].radius <= 0) smallBalls[i].speed.x *= -1;
                    if (smallBalls[i].position.y - smallBalls[i].radius <= 0) smallBalls[i].speed.y *= -1;
                    if (smallBalls[i].position.y + smallBalls[i].radius >= screenHeight)
                    {
                        smallBalls[i].speed.y *= -1;
                        smallBalls[i].position.y = screenHeight - smallBalls[i].radius;
                    }

                    smallBalls[i].speed.y += gravity + 0.25f;
                }
            }

            // Player-shot vs meteors logic
            for (int i = 0; i < PLAYER_MAX_SHOOTS; i++)
            {
                if ((shoot[i].active))
                {
                    for (int a = 0; a < MAX_BIG_BALLS; a++)
                    {
                        if (bigBalls[a].active && (bigBalls[a].position.x - bigBalls[a].radius <= linePosition.x && bigBalls[a].position.x + bigBalls[a].radius >= linePosition.x)
                            && (bigBalls[a].position.y + bigBalls[a].radius >= shoot[i].position.y))
                        {
                            shoot[i].active = false;
                            shoot[i].lifeSpawn = 0;
                            bigBalls[a].active = false;
                            meteorsDestroyed++;
                            score +=  bigBalls[a].points;

                            for (int z = 0; z < 5; z++)
                            {
                                if (points[z].alpha == 0.0f)
                                {
                                    points[z].position = bigBalls[a].position;
                                    points[z].value = bigBalls[a].points;
                                    points[z].alpha = 1.0f;
                                    z = 5;
                                }
                            }

                            for (int j = 0; j < 2; j ++)
                            {
                                if ((countmediumBallss%2) == 0)
                                {
                                    mediumBalls[countmediumBallss].position = (Vector2){bigBalls[a].position.x, bigBalls[a].position.y};
                                    mediumBalls[countmediumBallss].speed = (Vector2){ -1*BALLS_SPEED, BALLS_SPEED };
                                }
                                else
                                {
                                    mediumBalls[countmediumBallss].position = (Vector2){bigBalls[a].position.x, bigBalls[a].position.y};
                                    mediumBalls[countmediumBallss].speed = (Vector2){ BALLS_SPEED, BALLS_SPEED };
                                }

                                mediumBalls[countmediumBallss].active = true;
                                countmediumBallss ++;
                            }
                            
                            a = MAX_BIG_BALLS;
                        }
                    }
                }

                if ((shoot[i].active))
                {
                    for (int b = 0; b < MAX_BIG_BALLS*2; b++)
                    {
                        if (mediumBalls[b].active && (mediumBalls[b].position.x - mediumBalls[b].radius <= linePosition.x && mediumBalls[b].position.x + mediumBalls[b].radius >= linePosition.x)
                            && (mediumBalls[b].position.y + mediumBalls[b].radius >= shoot[i].position.y))
                        {
                            shoot[i].active = false;
                            shoot[i].lifeSpawn = 0;
                            mediumBalls[b].active = false;
                            meteorsDestroyed++;
                            score +=  mediumBalls[b].points;

                            for (int z = 0; z < 5; z++)
                            {
                                if (points[z].alpha == 0.0f)
                                {
                                    points[z].position = mediumBalls[b].position;
                                    points[z].value = mediumBalls[b].points;
                                    points[z].alpha = 1.0f;
                                    z = 5;
                                }
                            }

                            for (int j = 0; j < 2; j ++)
                            {
                                 if (countsmallBallss%2 == 0)
                                {
                                    smallBalls[countsmallBallss].position = (Vector2){mediumBalls[b].position.x, mediumBalls[b].position.y};
                                    smallBalls[countsmallBallss].speed = (Vector2){ BALLS_SPEED*-1, BALLS_SPEED*-1};
                                }
                                else
                                {
                                    smallBalls[countsmallBallss].position = (Vector2){mediumBalls[b].position.x, mediumBalls[b].position.y};
                                    smallBalls[countsmallBallss].speed = (Vector2){ BALLS_SPEED, BALLS_SPEED*-1};
                                }

                                smallBalls[countsmallBallss].active = true;
                                countsmallBallss ++;
                            }

                            b = MAX_BIG_BALLS*2;
                        }
                    }
                }

                if ((shoot[i].active))
                {
                    for (int c = 0; c < MAX_BIG_BALLS*4; c++)
                    {
                        if (smallBalls[c].active && (smallBalls[c].position.x - smallBalls[c].radius <= linePosition.x && smallBalls[c].position.x + smallBalls[c].radius >= linePosition.x)
                            && (smallBalls[c].position.y + smallBalls[c].radius >= shoot[i].position.y))
                        {
                            shoot[i].active = false;
                            shoot[i].lifeSpawn = 0;
                            smallBalls[c].active = false;
                            meteorsDestroyed++;
                            score +=  smallBalls[c].points;

                            for (int z = 0; z < 5; z++)
                            {
                                if (points[z].alpha == 0.0f)
                                {
                                    points[z].position = smallBalls[c].position;
                                    points[z].value = smallBalls[c].points;
                                    points[z].alpha = 1.0f;
                                    z = 5;
                                }
                            }

                            c = MAX_BIG_BALLS*4;
                        }
                    }
                }
            }

            if (meteorsDestroyed == (MAX_BIG_BALLS + MAX_BIG_BALLS*2 + MAX_BIG_BALLS*4)) victory = true;
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
    
    // Points move-up and fade logic
    for (int z = 0; z < 5; z++)
    {
        if (points[z].alpha > 0.0f)
        {
            points[z].position.y -= 2;
            points[z].alpha -= 0.02f;
        }
        
        if (points[z].alpha < 0.0f) points[z].alpha = 0.0f;
    }
}

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(RAYWHITE);
        
        if (!gameOver)
        {
            // Draw player
            Vector2 v1 = { player.position.x + sinf(player.rotation*DEG2RAD)*(shipHeight), player.position.y - cosf(player.rotation*DEG2RAD)*(shipHeight) };
            Vector2 v2 = { player.position.x - cosf(player.rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2), player.position.y - sinf(player.rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
            Vector2 v3 = { player.position.x + cosf(player.rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2), player.position.y + sinf(player.rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
            DrawTriangle(v1, v2, v3, MAROON);

            // Draw meteors (big)
            for (int i = 0;i < MAX_BIG_BALLS; i++)
            {
                if (bigBalls[i].active) DrawCircleV(bigBalls[i].position, bigBalls[i].radius, DARKGRAY);
                else DrawCircleV(bigBalls[i].position, bigBalls[i].radius, Fade(LIGHTGRAY, 0.3f));
            }

            // Draw meteors (medium)
            for (int i = 0;i < MAX_BIG_BALLS*2; i++)
            {
                if (mediumBalls[i].active) DrawCircleV(mediumBalls[i].position, mediumBalls[i].radius, GRAY);
                else DrawCircleV(mediumBalls[i].position, mediumBalls[i].radius, Fade(LIGHTGRAY, 0.3f));
            }

            // Draw meteors (small)
            for (int i = 0;i < MAX_BIG_BALLS*4; i++)
            {
                if (smallBalls[i].active) DrawCircleV(smallBalls[i].position, smallBalls[i].radius, GRAY);
                else DrawCircleV(smallBalls[i].position, smallBalls[i].radius, Fade(LIGHTGRAY, 0.3f));
            }

            // Draw shoot
            for (int i = 0; i < PLAYER_MAX_SHOOTS; i++)
            {
                if (shoot[i].active) DrawLine(linePosition.x, linePosition.y, shoot[i].position.x, shoot[i].position.y, RED);
            }

            // Draw score points
            for (int z = 0; z < 5; z++)
            {
                if (points[z].alpha > 0.0f)
                {
                    DrawText(TextFormat("+%02i", points[z].value), points[z].position.x, points[z].position.y, 20, Fade(BLUE, points[z].alpha));
                }
            }

            // Draw score (UI)
            DrawText(TextFormat("SCORE: %i", score), 10, 10, 20, LIGHTGRAY);
            
            if (victory) 
            {
                DrawText("YOU WIN!", screenWidth/2 - MeasureText("YOU WIN!", 60)/2, 100, 60, LIGHTGRAY);
                DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, LIGHTGRAY);
            }
            
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
