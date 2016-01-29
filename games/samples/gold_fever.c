/*******************************************************************************************
*
*   raylib - sample game: gold fever
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

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Player {
    Vector2 position;
    int radius;
    Vector2 speed;
    Color color;
} Player;

typedef struct Enemy {
    Vector2 position;
    int radius;
    int radiusBounds;
    Vector2 speed;
    bool moveRight;
    Color colorBounds;
    Color color;
} Enemy;

typedef struct Points {
    Vector2 position;
    int radius;
    int value;
    bool active;
    Color color;
} Points;

typedef struct Exit {
    Rectangle rec;
    bool active;
    bool save;
    Color color;
} Exit;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static int screenWidth = 800;
static int screenHeight = 450;

static int framesCounter;
static bool gameOver;
static bool pause;
static int score;
static int hiScore = 0;

static Player player;
static Enemy enemy;
static Points points;
static Exit exit;
static bool follow;

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
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------

    InitWindow(screenWidth, screenHeight, "sample game: gold fever");

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
void InitGame(void)
{
    pause = false;
    score = 0;

    player.position = (Vector2){50, 50};
    player.radius = 20;
    player.speed = (Vector2){5, 5};
    player.color = DARKGRAY;

    enemy.position = (Vector2){screenWidth - 50, screenHeight/2};
    enemy.radius = 20;
    enemy.radiusBounds = 150;
    enemy.speed = (Vector2){3, 3};
    enemy.moveRight = true;
    enemy.color = MAROON;
    enemy.colorBounds = RED;
    follow = false;

    points.radius = 10;
    points.position = (Vector2){GetRandomValue(points.radius, screenWidth - points.radius), GetRandomValue(points.radius, screenHeight - points.radius)};
    points.value = 100;
    points.active = true;
    points.color = GOLD;

    exit.rec.width = 50;
    exit.rec.height = 50;
    exit.rec.x = GetRandomValue(0, screenWidth - exit.rec.width);
    exit.rec.y = GetRandomValue(0, screenHeight - exit.rec.height);
    exit.active = false;
    exit.save = false;
    exit.color = PINK;
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            //Control player
            if (IsKeyDown(KEY_RIGHT)) player.position.x += player.speed.x;
            if (IsKeyDown(KEY_LEFT)) player.position.x -= player.speed.x;
            if (IsKeyDown(KEY_UP)) player.position.y -= player.speed.y;
            if (IsKeyDown(KEY_DOWN)) player.position.y += player.speed.y;

            //wall behaviour player
            if (player.position.x - player.radius <= 0) player.position.x = player.radius;
            if (player.position.x + player.radius >= screenWidth) player.position.x = screenWidth - player.radius;
            if (player.position.y - player.radius <= 0) player.position.y = player.radius;
            if (player.position.y + player.radius >= screenHeight) player.position.y = screenHeight - player.radius;

            //IA Enemy
            if ( (follow || CheckCollisionCircles(player.position, player.radius, enemy.position, enemy.radiusBounds)) && !exit.save)
            {
                if (player.position.x > enemy.position.x) enemy.position.x += enemy.speed.x;
                if (player.position.x < enemy.position.x) enemy.position.x -= enemy.speed.x;

                if (player.position.y > enemy.position.y) enemy.position.y += enemy.speed.y;
                if (player.position.y < enemy.position.y) enemy.position.y -= enemy.speed.y;
            }
            else
            {
                if (enemy.moveRight) enemy.position.x += enemy.speed.x;
                else enemy.position.x -= enemy.speed.x;
            }

            //wall behaviour enemy
            if (enemy.position.x - enemy.radius <= 0) enemy.moveRight = true;
            if (enemy.position.x + enemy.radius >= screenWidth) enemy.moveRight = false;

            if (enemy.position.x - enemy.radius <= 0) enemy.position.x = enemy.radius;
            if (enemy.position.x + enemy.radius >= screenWidth) enemy.position.x = screenWidth - enemy.radius;
            if (enemy.position.y - enemy.radius <= 0) enemy.position.y = enemy.radius;
            if (enemy.position.y + enemy.radius >= screenHeight) enemy.position.y = screenHeight - enemy.radius;

            //Collisions
            if (CheckCollisionCircles(player.position, player.radius, points.position, points.radius) && points.active)
            {
                follow = true;
                points.active = false;
                exit.active = true;
            }

            if (CheckCollisionCircles(player.position, player.radius, enemy.position, enemy.radius) && !exit.save)
            {
                gameOver = true;
                
                if (hiScore < score) hiScore = score;
            }

            if (CheckCollisionCircleRec(player.position, player.radius, exit.rec))
            {
               follow = false;
               
               if (!points.active)
               {
                    score += points.value;
                    points.active = true;
                    enemy.speed.x += 0.5;
                    enemy.speed.y += 0.5;
                    points.position = (Vector2){GetRandomValue(points.radius, screenWidth - points.radius), GetRandomValue(points.radius, screenHeight - points.radius)};
               }
               
               exit.save = true;
            }
            else exit.save = false;
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
            if (follow) ClearBackground(RED);

            DrawCircleLines(enemy.position.x, enemy.position.y, enemy.radiusBounds, enemy.colorBounds);
            DrawCircleV(enemy.position, enemy.radius, enemy.color);
            
            DrawCircleV(player.position, player.radius, player.color);
            DrawCircleV(points.position, points.radius, points.color);
            
            if (exit.active) DrawRectangleRec(exit.rec, exit.color);

            DrawText(FormatText("SCORE: %04i", score), 10, 10, 20, GRAY);
            DrawText(FormatText("HI-SCORE: %04i", hiScore), 300, 10, 20, GRAY);

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