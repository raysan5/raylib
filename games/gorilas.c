/*******************************************************************************************
*
*   raylib - sample game: gorilas
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
#define MAX_BUILDINGS                    15
#define MAX_EXPLOSIONS                  200
#define MAX_PLAYERS                       2

#define BUILDING_RELATIVE_ERROR          30        // Building size random range %
#define BUILDING_MIN_RELATIVE_HEIGHT     20        // Minimum height in % of the screenHeight
#define BUILDING_MAX_RELATIVE_HEIGHT     60        // Maximum height in % of the screenHeight
#define BUILDING_MIN_GRAYSCALE_COLOR    120        // Minimum gray color for the buildings
#define BUILDING_MAX_GRAYSCALE_COLOR    200        // Maximum gray color for the buildings

#define MIN_PLAYER_POSITION               5        // Minimum x position %
#define MAX_PLAYER_POSITION              20        // Maximum x position %

#define GRAVITY                       9.81f
#define DELTA_FPS                        60

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Player {
    Vector2 position;
    Vector2 size;

    Vector2 aimingPoint;
    int aimingAngle;
    int aimingPower;

    Vector2 previousPoint;
    int previousAngle;
    int previousPower;

    Vector2 impactPoint;

    bool isLeftTeam;                // This player belongs to the left or to the right team
    bool isPlayer;                  // If is a player or an AI
    bool isAlive;
} Player;

typedef struct Building {
    Rectangle rectangle;
    Color color;
} Building;

typedef struct Explosion {
    Vector2 position;
    int radius;
    bool active;
} Explosion;

typedef struct Ball {
    Vector2 position;
    Vector2 speed;
    int radius;
    bool active;
} Ball;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause = false;

static Player player[MAX_PLAYERS] = { 0 };
static Building building[MAX_BUILDINGS] = { 0 };
static Explosion explosion[MAX_EXPLOSIONS] = { 0 };
static Ball ball = { 0 };

static int playerTurn = 0;
static bool ballOnAir = false;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

// Additional module functions
static void InitBuildings(void);
static void InitPlayers(void);
static bool UpdatePlayer(int playerTurn);
static bool UpdateBall(int playerTurn);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "sample game: gorilas");

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
    // Init shoot
    ball.radius = 10;
    ballOnAir = false;
    ball.active = false;

    InitBuildings();
    InitPlayers();
    
    // Init explosions
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        explosion[i].position = (Vector2){ 0.0f, 0.0f };
        explosion[i].radius = 30;
        explosion[i].active = false;
    }
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            if (!ballOnAir) ballOnAir = UpdatePlayer(playerTurn); // If we are aiming
            else
            {
                if (UpdateBall(playerTurn))                       // If collision
                {
                    // Game over logic
                    bool leftTeamAlive = false;
                    bool rightTeamAlive = false;

                    for (int i = 0; i < MAX_PLAYERS; i++)
                    {
                        if (player[i].isAlive)
                        {
                            if (player[i].isLeftTeam) leftTeamAlive = true;
                            if (!player[i].isLeftTeam) rightTeamAlive = true;
                        }
                    }
                    
                    if (leftTeamAlive && rightTeamAlive)
                    {
                        ballOnAir = false;
                        ball.active = false;

                        playerTurn++;

                        if (playerTurn == MAX_PLAYERS) playerTurn = 0;
                    }
                    else
                    {
                        gameOver = true;
                        
                        // if (leftTeamAlive) left team wins
                        // if (rightTeamAlive) right team wins
                    }
                }
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
            // Draw buildings
            for (int i = 0; i < MAX_BUILDINGS; i++) DrawRectangleRec(building[i].rectangle, building[i].color);

            // Draw explosions
            for (int i = 0; i < MAX_EXPLOSIONS; i++)
            {
                if (explosion[i].active) DrawCircle(explosion[i].position.x, explosion[i].position.y, explosion[i].radius, RAYWHITE);
            }
            
            // Draw players
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (player[i].isAlive)
                {
                    if (player[i].isLeftTeam) DrawRectangle(player[i].position.x - player[i].size.x/2, player[i].position.y - player[i].size.y/2,
                                                             player[i].size.x, player[i].size.y, BLUE);
                    else DrawRectangle(player[i].position.x - player[i].size.x/2, player[i].position.y - player[i].size.y/2,
                                                             player[i].size.x, player[i].size.y, RED);
                }
            }

            // Draw ball
            if (ball.active) DrawCircle(ball.position.x, ball.position.y, ball.radius, MAROON);

            // Draw the angle and the power of the aim, and the previous ones
            if (!ballOnAir)
            {
                // Draw shot information
                /*
                if (player[playerTurn].isLeftTeam)
                {
                    DrawText(TextFormat("Previous Point %i, %i", (int)player[playerTurn].previousPoint.x, (int)player[playerTurn].previousPoint.y), 20, 20, 20, DARKBLUE);
                    DrawText(TextFormat("Previous Angle %i", player[playerTurn].previousAngle), 20, 50, 20, DARKBLUE);
                    DrawText(TextFormat("Previous Power %i", player[playerTurn].previousPower), 20, 80, 20, DARKBLUE);
                    DrawText(TextFormat("Aiming Point %i, %i", (int)player[playerTurn].aimingPoint.x, (int)player[playerTurn].aimingPoint.y), 20, 110, 20, DARKBLUE);
                    DrawText(TextFormat("Aiming Angle %i", player[playerTurn].aimingAngle), 20, 140, 20, DARKBLUE);
                    DrawText(TextFormat("Aiming Power %i", player[playerTurn].aimingPower), 20, 170, 20, DARKBLUE);
                }
                else
                {
                    DrawText(TextFormat("Previous Point %i, %i", (int)player[playerTurn].previousPoint.x, (int)player[playerTurn].previousPoint.y), screenWidth*3/4, 20, 20, DARKBLUE);
                    DrawText(TextFormat("Previous Angle %i", player[playerTurn].previousAngle), screenWidth*3/4, 50, 20, DARKBLUE);
                    DrawText(TextFormat("Previous Power %i", player[playerTurn].previousPower), screenWidth*3/4, 80, 20, DARKBLUE);
                    DrawText(TextFormat("Aiming Point %i, %i", (int)player[playerTurn].aimingPoint.x, (int)player[playerTurn].aimingPoint.y), screenWidth*3/4, 110, 20, DARKBLUE);
                    DrawText(TextFormat("Aiming Angle %i", player[playerTurn].aimingAngle), screenWidth*3/4, 140, 20, DARKBLUE);
                    DrawText(TextFormat("Aiming Power %i", player[playerTurn].aimingPower), screenWidth*3/4, 170, 20, DARKBLUE);
                }
                */
                
                // Draw aim
                if (player[playerTurn].isLeftTeam)
                {
                    // Previous aiming
                    DrawTriangle((Vector2){ player[playerTurn].position.x - player[playerTurn].size.x/4, player[playerTurn].position.y - player[playerTurn].size.y/4 },
                                 (Vector2){ player[playerTurn].position.x + player[playerTurn].size.x/4, player[playerTurn].position.y + player[playerTurn].size.y/4 },
                                 player[playerTurn].previousPoint, GRAY);

                    // Actual aiming
                    DrawTriangle((Vector2){ player[playerTurn].position.x - player[playerTurn].size.x/4, player[playerTurn].position.y - player[playerTurn].size.y/4 },
                                 (Vector2){ player[playerTurn].position.x + player[playerTurn].size.x/4, player[playerTurn].position.y + player[playerTurn].size.y/4 },
                                 player[playerTurn].aimingPoint, DARKBLUE);
                }
                else
                {
                    // Previous aiming
                    DrawTriangle((Vector2){ player[playerTurn].position.x - player[playerTurn].size.x/4, player[playerTurn].position.y + player[playerTurn].size.y/4 },
                                 (Vector2){ player[playerTurn].position.x + player[playerTurn].size.x/4, player[playerTurn].position.y - player[playerTurn].size.y/4 },
                                 player[playerTurn].previousPoint, GRAY);

                    // Actual aiming
                    DrawTriangle((Vector2){ player[playerTurn].position.x - player[playerTurn].size.x/4, player[playerTurn].position.y + player[playerTurn].size.y/4 },
                                 (Vector2){ player[playerTurn].position.x + player[playerTurn].size.x/4, player[playerTurn].position.y - player[playerTurn].size.y/4 },
                                 player[playerTurn].aimingPoint, MAROON);
                }
            }

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
static void InitBuildings(void)
{
    // Horizontal generation
    int currentWidth = 0;

    // We make sure the absolute error randomly generated for each building, has as a minimum value the screenWidth.
    // This way all the screen will be filled with buildings. Each building will have a different, random width.

    float relativeWidth = 100/(100 - BUILDING_RELATIVE_ERROR);
    float buildingWidthMean = (screenWidth*relativeWidth/MAX_BUILDINGS) + 1;        // We add one to make sure we will cover the whole screen.

    // Vertical generation
    int currentHeighth = 0;
    int grayLevel;

    // Creation
    for (int i = 0; i < MAX_BUILDINGS; i++)
    {
        // Horizontal
        building[i].rectangle.x = currentWidth;
        building[i].rectangle.width = GetRandomValue(buildingWidthMean*(100 - BUILDING_RELATIVE_ERROR/2)/100 + 1, buildingWidthMean*(100 + BUILDING_RELATIVE_ERROR)/100);

        currentWidth += building[i].rectangle.width;

        // Vertical
        currentHeighth = GetRandomValue(BUILDING_MIN_RELATIVE_HEIGHT, BUILDING_MAX_RELATIVE_HEIGHT);
        building[i].rectangle.y = screenHeight - (screenHeight*currentHeighth/100);
        building[i].rectangle.height = screenHeight*currentHeighth/100 + 1;

        // Color
        grayLevel = GetRandomValue(BUILDING_MIN_GRAYSCALE_COLOR, BUILDING_MAX_GRAYSCALE_COLOR);
        building[i].color = (Color){ grayLevel, grayLevel, grayLevel, 255 };
    }
}

static void InitPlayers(void)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        player[i].isAlive = true;

        // Decide the team of this player
        if (i % 2 == 0) player[i].isLeftTeam = true;
        else player[i].isLeftTeam = false;

        // Now there is no AI
        player[i].isPlayer = true;

        // Set size, by default by now
        player[i].size = (Vector2){ 40, 40 };

        // Set position
        if (player[i].isLeftTeam) player[i].position.x = GetRandomValue(screenWidth*MIN_PLAYER_POSITION/100, screenWidth*MAX_PLAYER_POSITION/100);
        else player[i].position.x = screenWidth - GetRandomValue(screenWidth*MIN_PLAYER_POSITION/100, screenWidth*MAX_PLAYER_POSITION/100);

        for (int j = 0; j < MAX_BUILDINGS; j++)
        {
            if (building[j].rectangle.x > player[i].position.x)
            {
                // Set the player in the center of the building
                player[i].position.x = building[j-1].rectangle.x + building[j-1].rectangle.width/2;
                // Set the player at the top of the building
                player[i].position.y = building[j-1].rectangle.y - player[i].size.y/2;
                break;
            }
        }

        // Set statistics to 0
        player[i].aimingPoint = player[i].position;
        player[i].previousAngle = 0;
        player[i].previousPower = 0;
        player[i].previousPoint = player[i].position;
        player[i].aimingAngle = 0;
        player[i].aimingPower = 0;

        player[i].impactPoint = (Vector2){ -100, -100 };
    }
}

static bool UpdatePlayer(int playerTurn)
{
    // If we are aiming at the firing quadrant, we calculate the angle
    if (GetMousePosition().y <= player[playerTurn].position.y)
    {
        // Left team
        if (player[playerTurn].isLeftTeam && GetMousePosition().x >= player[playerTurn].position.x)
        {
            // Distance (calculating the fire power)
            player[playerTurn].aimingPower = sqrt(pow(player[playerTurn].position.x - GetMousePosition().x, 2) + pow(player[playerTurn].position.y - GetMousePosition().y, 2));
            // Calculates the angle via arcsin
            player[playerTurn].aimingAngle = asin((player[playerTurn].position.y - GetMousePosition().y)/player[playerTurn].aimingPower)*RAD2DEG;
            // Point of the screen we are aiming at
            player[playerTurn].aimingPoint = GetMousePosition();

            // Ball fired
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                player[playerTurn].previousPoint = player[playerTurn].aimingPoint;
                player[playerTurn].previousPower = player[playerTurn].aimingPower;
                player[playerTurn].previousAngle = player[playerTurn].aimingAngle;
                ball.position = player[playerTurn].position;

                return true;
            }
        }
        // Right team
        else if (!player[playerTurn].isLeftTeam && GetMousePosition().x <= player[playerTurn].position.x)
        {
            // Distance (calculating the fire power)
            player[playerTurn].aimingPower = sqrt(pow(player[playerTurn].position.x - GetMousePosition().x, 2) + pow(player[playerTurn].position.y - GetMousePosition().y, 2));
            // Calculates the angle via arcsin
            player[playerTurn].aimingAngle = asin((player[playerTurn].position.y - GetMousePosition().y)/player[playerTurn].aimingPower)*RAD2DEG;
            // Point of the screen we are aiming at
            player[playerTurn].aimingPoint = GetMousePosition();

            // Ball fired
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                player[playerTurn].previousPoint = player[playerTurn].aimingPoint;
                player[playerTurn].previousPower = player[playerTurn].aimingPower;
                player[playerTurn].previousAngle = player[playerTurn].aimingAngle;
                ball.position = player[playerTurn].position;

                return true;
            }
        }
        else
        {
            player[playerTurn].aimingPoint = player[playerTurn].position;
            player[playerTurn].aimingPower = 0;
            player[playerTurn].aimingAngle = 0;
        }
    }
    else
    {
        player[playerTurn].aimingPoint = player[playerTurn].position;
        player[playerTurn].aimingPower = 0;
        player[playerTurn].aimingAngle = 0;
    }

    return false;
}

static bool UpdateBall(int playerTurn)
{
    static int explosionNumber = 0;

    // Activate ball
    if (!ball.active)
    {
        if (player[playerTurn].isLeftTeam)
        {
            ball.speed.x = cos(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
            ball.speed.y = -sin(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
            ball.active = true;
        }
        else
        {
            ball.speed.x = -cos(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
            ball.speed.y = -sin(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
            ball.active = true;
        }
    }

    ball.position.x += ball.speed.x;
    ball.position.y += ball.speed.y;
    ball.speed.y += GRAVITY/DELTA_FPS;

    // Collision
    if (ball.position.x + ball.radius < 0) return true;
    else if (ball.position.x - ball.radius > screenWidth) return true;
    else
    {
        // Player collision
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (CheckCollisionCircleRec(ball.position, ball.radius,  (Rectangle){ player[i].position.x - player[i].size.x/2, player[i].position.y - player[i].size.y/2,
                                                                                  player[i].size.x, player[i].size.y }))
            {
                // We can't hit ourselves
                if (i == playerTurn) return false;
                else
                {
                    // We set the impact point
                    player[playerTurn].impactPoint.x = ball.position.x;
                    player[playerTurn].impactPoint.y = ball.position.y + ball.radius;

                    // We destroy the player
                    player[i].isAlive = false;
                    return true;
                }
            }
        }

        // Building collision
        // NOTE: We only check building collision if we are not inside an explosion
        for (int i = 0; i < MAX_BUILDINGS; i++)
        {
            if (CheckCollisionCircles(ball.position, ball.radius, explosion[i].position, explosion[i].radius - ball.radius))
            {
                return false;
            }
        }

        for (int i = 0; i < MAX_BUILDINGS; i++)
        {
            if (CheckCollisionCircleRec(ball.position, ball.radius, building[i].rectangle))
            {
                // We set the impact point
                player[playerTurn].impactPoint.x = ball.position.x;
                player[playerTurn].impactPoint.y = ball.position.y + ball.radius;

                // We create an explosion
                explosion[explosionNumber].position = player[playerTurn].impactPoint;
                explosion[explosionNumber].active = true;
                explosionNumber++;

                return true;
            }
        }
    }

    return false;
}
