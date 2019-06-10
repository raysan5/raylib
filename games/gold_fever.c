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
    Vector2 speed;
    int radius;
} Player;

typedef struct Enemy {
    Vector2 position;
    Vector2 speed;
    int radius;
    int radiusBounds;
    bool moveRight;
} Enemy;

typedef struct Points {
    Vector2 position;
    int radius;
    int value;
    bool active;
} Points;

typedef struct Home {
    Rectangle rec;
    bool active;
    bool save;
    Color color;
} Home;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause = false;
static int score = 0;
static int hiScore = 0;

static Player player = { 0 };
static Enemy enemy = { 0 };
static Points points = { 0 };
static Home home = { 0 };
static bool follow = false;

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
void InitGame(void)
{
    pause = false;
    score = 0;

    player.position = (Vector2){50, 50};
    player.radius = 20;
    player.speed = (Vector2){5, 5};

    enemy.position = (Vector2){screenWidth - 50, screenHeight/2};
    enemy.radius = 20;
    enemy.radiusBounds = 150;
    enemy.speed = (Vector2){3, 3};
    enemy.moveRight = true;
    follow = false;

    points.radius = 10;
    points.position = (Vector2){GetRandomValue(points.radius, screenWidth - points.radius), GetRandomValue(points.radius, screenHeight - points.radius)};
    points.value = 100;
    points.active = true;

    home.rec.width = 50;
    home.rec.height = 50;
    home.rec.x = GetRandomValue(0, screenWidth - home.rec.width);
    home.rec.y = GetRandomValue(0, screenHeight - home.rec.height);
    home.active = false;
    home.save = false;
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            // Control player
            if (IsKeyDown(KEY_RIGHT)) player.position.x += player.speed.x;
            if (IsKeyDown(KEY_LEFT)) player.position.x -= player.speed.x;
            if (IsKeyDown(KEY_UP)) player.position.y -= player.speed.y;
            if (IsKeyDown(KEY_DOWN)) player.position.y += player.speed.y;

            // Wall behaviour player
            if (player.position.x - player.radius <= 0) player.position.x = player.radius;
            if (player.position.x + player.radius >= screenWidth) player.position.x = screenWidth - player.radius;
            if (player.position.y - player.radius <= 0) player.position.y = player.radius;
            if (player.position.y + player.radius >= screenHeight) player.position.y = screenHeight - player.radius;

            // IA Enemy
            if ( (follow || CheckCollisionCircles(player.position, player.radius, enemy.position, enemy.radiusBounds)) && !home.save)
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

            // Wall behaviour enemy
            if (enemy.position.x - enemy.radius <= 0) enemy.moveRight = true;
            if (enemy.position.x + enemy.radius >= screenWidth) enemy.moveRight = false;

            if (enemy.position.x - enemy.radius <= 0) enemy.position.x = enemy.radius;
            if (enemy.position.x + enemy.radius >= screenWidth) enemy.position.x = screenWidth - enemy.radius;
            if (enemy.position.y - enemy.radius <= 0) enemy.position.y = enemy.radius;
            if (enemy.position.y + enemy.radius >= screenHeight) enemy.position.y = screenHeight - enemy.radius;

            // Collisions
            if (CheckCollisionCircles(player.position, player.radius, points.position, points.radius) && points.active)
            {
                follow = true;
                points.active = false;
                home.active = true;
            }

            if (CheckCollisionCircles(player.position, player.radius, enemy.position, enemy.radius) && !home.save)
            {
                gameOver = true;
                
                if (hiScore < score) hiScore = score;
            }

            if (CheckCollisionCircleRec(player.position, player.radius, home.rec))
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
               
               home.save = true;
            }
            else home.save = false;
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
            if (follow)
            {
                DrawRectangle(0, 0, screenWidth, screenHeight, RED);
                DrawRectangle(10, 10, screenWidth - 20, screenHeight - 20, RAYWHITE);
            }
            
            DrawRectangleLines(home.rec.x, home.rec.y, home.rec.width, home.rec.height, BLUE);

            DrawCircleLines(enemy.position.x, enemy.position.y, enemy.radiusBounds, RED);
            DrawCircleV(enemy.position, enemy.radius, MAROON);
            
            DrawCircleV(player.position, player.radius, GRAY);
            if (points.active) DrawCircleV(points.position, points.radius, GOLD);

            DrawText(TextFormat("SCORE: %04i", score), 20, 15, 20, GRAY);
            DrawText(TextFormat("HI-SCORE: %04i", hiScore), 300, 15, 20, GRAY);

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
