/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Gameplay Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"
#include "screens.h"

#include <math.h>

#define MAX_LIGHTS_I         8
#define MAX_LIGHTS_II       12
#define MAX_LIGHTS_III      20

#define MAX_ENEMIES          8

#define MAX_PLAYER_ENERGY   40.0f
#define ENERGY_REFILL_RATIO  0.2f

#define GAMEPAD_SENSITIVITY  4.0f     // More sensitivity, more speed :P

#define LIGHT_ANIM_FRAMES    7

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Player {
    Vector2 position;
    Vector2 speed;
    int radius;
    Color color;
    float lightEnergy;
} Player;

typedef struct Enemy {
    Vector2 position;
    Vector2 targetPos;  // light target position
    int targetNum;      // light target number
    float speed;        // scalar value
    int radius;
    int active;
    int awakeFramesDelay;
    int framesCounter;
    Color color;
} Enemy;

typedef struct LightSpot {
    Vector2 position;
    int radius;
    int requiredEnergy;
    bool active;
    Color color;
    
    int framesCounter;
    int currentFrame;
    Rectangle frameRec;
} LightSpot;

typedef enum { LEVEL_I, LEVEL_II, LEVEL_III, LEVEL_FINISHED } LightedLevel;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Gameplay screen global variables
static int framesCounter;
static int finishScreen;

//static Texture2D background;

static bool pause;

static Player player;

static LightSpot lightsI[MAX_LIGHTS_I];
static LightSpot lightsII[MAX_LIGHTS_II];
static LightSpot lightsIII[MAX_LIGHTS_III];

static Enemy enemies[MAX_ENEMIES];

static int ritualLevel;
static int previousLightedLevel;
static int currentLightedLevel;

static Vector2 lighterPosition;

static int maxLightEnergy;
static int currentLightEnergy;

static float ritualTime;
static bool startRitual;
static float alphaRitual;

static bool timeOver;
static int nextStarsAlignment;

static Texture2D background;
static Texture2D foregroundI;
static Texture2D foregroundII;
static Texture2D foregroundIII;
static Texture2D texPlayer;
static Texture2D texEnemy;
static Texture2D texLight;
static Texture2D lightGlow;
static Texture2D lightRay;
static Texture2D book;
static Texture2D texRitual;
static Texture2D texTimeOver;
static Texture2D circleIoff, circleIIoff, circleIIIoff;
static Texture2D circleIon, circleIIon, circleIIIon;

static Rectangle lightOff, lightOn;

static Sound fxLightOn, fxLightOff;

static Music music;

// Debug variables
static bool enemiesStopped;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static bool ColorEqual(Color col1, Color col2);    // Check if two colors are equal
static Vector2 Vector2Subtract(Vector2 v1, Vector2 v2);
static void Vector2Normalize(Vector2 *v);
static void EnemyReset(Enemy *enemy);

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitGameplayScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;
    pause = false;
    
    // Textures loading
    background = LoadTexture("resources/textures/background.png");
    foregroundI = LoadTexture("resources/textures/foreground_level_i.png");
    foregroundII = LoadTexture("resources/textures/foreground_level_ii.png");
    foregroundIII = LoadTexture("resources/textures/foreground_level_iii.png");
    texPlayer = LoadTexture("resources/textures/player.png");
    texEnemy = LoadTexture("resources/textures/enemy.png");
    texLight = LoadTexture("resources/textures/light.png");
    lightGlow = LoadTexture("resources/textures/light_glow.png");
    lightRay = LoadTexture("resources/textures/light_ray.png");
    book = LoadTexture("resources/textures/book.png");
    texRitual = LoadTexture("resources/textures/msg_ritual.png");
    texTimeOver = LoadTexture("resources/textures/time_over.png");
    
    circleIoff = LoadTexture("resources/textures/circle_level_i_off.png");
    circleIIoff = LoadTexture("resources/textures/circle_level_ii_off.png");
    circleIIIoff = LoadTexture("resources/textures/circle_level_iii_off.png");
    circleIon = LoadTexture("resources/textures/circle_level_i_on.png");
    circleIIon = LoadTexture("resources/textures/circle_level_ii_on.png");
    circleIIIon = LoadTexture("resources/textures/circle_level_iii_on.png");

    lightOff = (Rectangle){ 0, 0, 64, 64 };
    lightOn = (Rectangle){ 64, 0, 64, 64 };
    
    fxLightOn = LoadSound("resources/audio/light_on.wav");
    fxLightOff = LoadSound("resources/audio/light_off.wav");
    
    // Initialize player
    player.position = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 - 40 };
    player.radius = 20;
    player.speed = (Vector2){5, 5};
    player.color = WHITE;
    
    // Initialize lights positions based on lights map image data
    int kI = 0, kII = 0, kIII = 0;
    for (int y = 0; y < lightsMapHeight; y++)
    {
        for (int x = 0; x < lightsMapWidth; x++)
        {
            if (ColorEqual(lightsMap[y*lightsMapWidth + x], (Color){ 255, 0, 0, 255 }))
            {
                // Store light position I
                lightsI[kI].position.x = (float)x*10;
                lightsI[kI].position.y = (float)y*10;
                kI++;
                
                //printf("Light %02i position: %i, %i\n", kI, (int)lightsI[kI - 1].position.x, (int)lightsI[kI - 1].position.y);
            }
            else if (ColorEqual(lightsMap[y*lightsMapWidth + x], (Color){ 0, 255, 0, 255 }))
            {
                // Store light position II
                lightsII[kII].position.x = (float)x*10;
                lightsII[kII].position.y = (float)y*10;
                kII++;
            }
            else if (ColorEqual(lightsMap[y*lightsMapWidth + x], (Color){ 0, 0, 255, 255 }))
            {
                // Store light position III
                lightsIII[kIII].position.x = (float)x*10;
                lightsIII[kIII].position.y = (float)y*10;
                kIII++;
            }
        }
    }
    
    // Initialize lights I
    for (int i = 0; i < MAX_LIGHTS_I; i++)
    {
        lightsI[i].radius = 12;
        lightsI[i].requiredEnergy = GetRandomValue(3, 9);
        lightsI[i].active = false;
        lightsI[i].color = GOLD;
        
        lightsI[i].framesCounter = 0;
        lightsI[i].currentFrame = 0;
        lightsI[i].frameRec = (Rectangle){ 0, 0, 64, 64 };
    }
    
    // Initialize lights II
    for (int i = 0; i < MAX_LIGHTS_II; i++)
    {
        lightsII[i].radius = 8;
        lightsII[i].requiredEnergy = GetRandomValue(3, 8);
        lightsII[i].active = false;
        lightsII[i].color = GOLD;
        
        lightsII[i].framesCounter = 0;
        lightsII[i].currentFrame = 0;
        lightsII[i].frameRec = (Rectangle){ 0, 0, 64, 64 };
    }
    
    // Initialize lights III
    for (int i = 0; i < MAX_LIGHTS_III; i++)
    {
        lightsIII[i].radius = 8;
        lightsIII[i].requiredEnergy = GetRandomValue(4, 10);
        lightsIII[i].active = false;
        lightsIII[i].color = GOLD;
        
        lightsIII[i].framesCounter = 0;
        lightsIII[i].currentFrame = 0;
        lightsIII[i].frameRec = (Rectangle){ 0, 0, 64, 64 };
    }
    
    // Initialize ritual level
    ritualLevel = 0;
    currentLightedLevel = LEVEL_I;
    lighterPosition = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };

    // Initialize enemies
    for (int i = 0; i < MAX_ENEMIES; i++) EnemyReset(&enemies[i]);
    
    // Initialize max light energy (depends on lights randomness)
    maxLightEnergy = 0;
    
    for (int i = 0; i < MAX_LIGHTS_I; i++) maxLightEnergy += lightsI[i].requiredEnergy;
    for (int i = 0; i < MAX_LIGHTS_II; i++) maxLightEnergy += lightsII[i].requiredEnergy;
    for (int i = 0; i < MAX_LIGHTS_III; i++) maxLightEnergy += lightsIII[i].requiredEnergy;
    
    //printf("Max light energy: %i\n", maxLightEnergy);
    
    // Initialize ritual variables
    ritualTime = 0.0f;
    startRitual = false;;
    alphaRitual = 0.0f;
    
    timeOver = false;
    nextStarsAlignment = GetRandomValue(500, 1000);
    
    enemiesStopped = false;
    
    music = LoadMusicStream("resources/audio/ritual.ogg");
    PlayMusicStream(music);
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
    if (IsKeyPressed('P')) pause = !pause;

    if (!pause && (currentLightedLevel != LEVEL_FINISHED) && !timeOver)
    {
        framesCounter++;        // Time starts counting to awake enemies
        
        // Player movement logic
        if (IsKeyDown(KEY_RIGHT)) player.position.x += player.speed.x;
        else if (IsKeyDown(KEY_LEFT)) player.position.x -= player.speed.x;
        
        if (IsKeyDown(KEY_UP)) player.position.y -= player.speed.y;
        else if (IsKeyDown(KEY_DOWN)) player.position.y += player.speed.y;
        
        // Debug key to stop enemies
        if (IsKeyPressed(KEY_S)) enemiesStopped = !enemiesStopped;
        
        /*
        if (IsGamepadAvailable(GAMEPAD_PLAYER1))
        {
            Vector2 movement = GetGamepadMovement(GAMEPAD_PLAYER1);
            
            player.position.x += movement.x*GAMEPAD_SENSITIVITY;
            player.position.y += movement.y*GAMEPAD_SENSITIVITY;
        }
        */
        
        // Player light energy filling logic
        if (CheckCollisionCircles(player.position, player.radius, lighterPosition, 50))
        {
            player.lightEnergy += ENERGY_REFILL_RATIO;
            player.color = (Color){ 255, 255, 100, 255 };
        }
        else player.color = WHITE;
        
        if (player.lightEnergy > MAX_PLAYER_ENERGY) player.lightEnergy = MAX_PLAYER_ENERGY;
        
        // Player vs lights collision detection (depends on lighted level)
        if (currentLightedLevel == LEVEL_I)
        {
            for (int i = 0; i < MAX_LIGHTS_I; i++)
            {
                // Check player vs lightI collision
                if (CheckCollisionCircles(player.position, player.radius, lightsI[i].position, lightsI[i].radius))
                {
                    if (!lightsI[i].active && (player.lightEnergy >= lightsI[i].requiredEnergy))
                    {
                        lightsI[i].active = true;
                        lightsI[i].currentFrame = 1;
                        player.lightEnergy -= lightsI[i].requiredEnergy;
                        
                        PlaySound(fxLightOn);
                    }
                }
            }
        }
        else if (currentLightedLevel == LEVEL_II)
        {
            for (int i = 0; i < MAX_LIGHTS_II; i++)
            {
                if (CheckCollisionCircles(player.position, player.radius, lightsII[i].position, lightsII[i].radius))
                {
                    if (!lightsII[i].active && (player.lightEnergy >= lightsII[i].requiredEnergy))
                    {
                        lightsII[i].active = true;
                        player.lightEnergy -= lightsII[i].requiredEnergy;
                        
                        PlaySound(fxLightOn);
                    }
                }
            }
        }
        else if (currentLightedLevel == LEVEL_III)
        {
            for (int i = 0; i < MAX_LIGHTS_III; i++)
            {
                if (CheckCollisionCircles(player.position, player.radius, lightsIII[i].position, lightsIII[i].radius))
                {
                    if (!lightsIII[i].active && (player.lightEnergy >= lightsIII[i].requiredEnergy))
                    {
                        lightsIII[i].active = true;
                        player.lightEnergy -= lightsIII[i].requiredEnergy;
                        
                        PlaySound(fxLightOn);
                    }
                }
            }
        }
        
        // Lights animation (it doesn't depend on currentLightedLevel)
        for (int i = 0; i < MAX_LIGHTS_I; i++)
        {
            // Light animation
            if (lightsI[i].active)
            {
                lightsI[i].framesCounter++;
                
                if (lightsI[i].framesCounter > 10)
                {
                    lightsI[i].currentFrame++;
                    
                    if (lightsI[i].currentFrame > LIGHT_ANIM_FRAMES - 1) lightsI[i].currentFrame = 1;
                    
                    lightsI[i].framesCounter = 0;
                }
            }
            
            lightsI[i].frameRec.x = lightsI[i].currentFrame*texLight.width/LIGHT_ANIM_FRAMES;
        }
        
        for (int i = 0; i < MAX_LIGHTS_II; i++)
        {
            // Light animation
            if (lightsII[i].active)
            {
                lightsII[i].framesCounter++;
                
                if (lightsII[i].framesCounter > 10)
                {
                    lightsII[i].currentFrame++;
                    
                    if (lightsII[i].currentFrame > LIGHT_ANIM_FRAMES - 1) lightsII[i].currentFrame = 1;
                    
                    lightsII[i].framesCounter = 0;
                }
            }
            
            lightsII[i].frameRec.x = lightsII[i].currentFrame*texLight.width/LIGHT_ANIM_FRAMES;
        }
        
        for (int i = 0; i < MAX_LIGHTS_III; i++)
        {
            // Light animation
            if (lightsIII[i].active)
            {
                lightsIII[i].framesCounter++;
                
                if (lightsIII[i].framesCounter > 10)
                {
                    lightsIII[i].currentFrame++;
                    
                    if (lightsIII[i].currentFrame > LIGHT_ANIM_FRAMES - 1) lightsIII[i].currentFrame = 1;
                    
                    lightsIII[i].framesCounter = 0;
                }
            }
            
            lightsIII[i].frameRec.x = lightsIII[i].currentFrame*texLight.width/LIGHT_ANIM_FRAMES;
        }

        // Enemies logic
        if (!enemiesStopped)
        {
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                if (!enemies[i].active) enemies[i].framesCounter++;
                
                if (enemies[i].framesCounter > enemies[i].awakeFramesDelay) enemies[i].active = true;
                
                if (enemies[i].active)
                {
                    // Move to the target
                    Vector2 dir = Vector2Subtract(enemies[i].targetPos, enemies[i].position);
                    Vector2Normalize(&dir);
                    
                    enemies[i].position.x += dir.x*enemies[i].speed;
                    enemies[i].position.y += dir.y*enemies[i].speed;
                    
                    if (currentLightedLevel == LEVEL_I)
                    {
                        if (CheckCollisionCircles(enemies[i].position, enemies[i].radius, enemies[i].targetPos, lightsI[enemies[i].targetNum].radius))
                        {
                            lightsI[enemies[i].targetNum].active = false;
                            lightsI[enemies[i].targetNum].framesCounter = 0;
                            lightsI[enemies[i].targetNum].currentFrame = 0;
                            lightsI[enemies[i].targetNum].frameRec = (Rectangle){ 0, 0, 64, 64 };
                            
                            EnemyReset(&enemies[i]);
                            
                            PlaySound(fxLightOff);
                        }
                    }
                    else if (currentLightedLevel == LEVEL_II)
                    {
                        if (CheckCollisionCircles(enemies[i].position, enemies[i].radius, enemies[i].targetPos, lightsII[enemies[i].targetNum].radius))
                        {
                            lightsII[enemies[i].targetNum].active = false;
                            lightsII[enemies[i].targetNum].framesCounter = 0;
                            lightsII[enemies[i].targetNum].currentFrame = 0;
                            lightsII[enemies[i].targetNum].frameRec = (Rectangle){ 0, 0, 64, 64 };
                            
                            EnemyReset(&enemies[i]);
                            
                            PlaySound(fxLightOff);
                        }
                    }
                    else if (currentLightedLevel == LEVEL_III)
                    {
                        if (CheckCollisionCircles(enemies[i].position, enemies[i].radius, enemies[i].targetPos, lightsIII[enemies[i].targetNum].radius))
                        {
                            lightsIII[enemies[i].targetNum].active = false;
                            lightsIII[enemies[i].targetNum].framesCounter = 0;
                            lightsIII[enemies[i].targetNum].currentFrame = 0;
                            lightsIII[enemies[i].targetNum].frameRec = (Rectangle){ 0, 0, 64, 64 };
                            
                            EnemyReset(&enemies[i]);
                            
                            PlaySound(fxLightOff);
                        }
                    }
                }
            }
        }
                    
        // Check current light energy (for right bar)
        currentLightEnergy = 0;
        
        for (int i = 0; i < MAX_LIGHTS_I; i++) if (lightsI[i].active) currentLightEnergy += lightsI[i].requiredEnergy;
        for (int i = 0; i < MAX_LIGHTS_II; i++) if (lightsII[i].active) currentLightEnergy += lightsII[i].requiredEnergy;
        for (int i = 0; i < MAX_LIGHTS_III; i++) if (lightsIII[i].active) currentLightEnergy += lightsIII[i].requiredEnergy;
    
        // Check current lighted level
        // Check ending conditions: all lights off, ritual level reached
        previousLightedLevel = currentLightedLevel;
        
        currentLightedLevel = LEVEL_I;
        
        bool lightedLevel = true;
        for (int i = 0; i < MAX_LIGHTS_I; i++) if (!lightsI[i].active) lightedLevel = false;
        if (lightedLevel) currentLightedLevel = LEVEL_II;

        for (int i = 0; i < MAX_LIGHTS_II; i++) if (!lightsII[i].active) lightedLevel = false;
        if (lightedLevel) currentLightedLevel = LEVEL_III;
        
        for (int i = 0; i < MAX_LIGHTS_III; i++) if (!lightsIII[i].active) lightedLevel = false;
        if (lightedLevel)
        {
            currentLightedLevel = LEVEL_FINISHED;
            
            for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
        }
        
        if (currentLightedLevel != previousLightedLevel) for (int i = 0; i < MAX_ENEMIES; i++) EnemyReset(&enemies[i]);
    
        ritualTime = (float)framesCounter/60;
        
        // Check game over condition (time run out)
        if ((99.0f - ritualTime) <= 0.0f)
        {
            ritualTime = 99.0f;
            timeOver = true;
        }
    }
    
    if (startRitual)
    {
        alphaRitual += 0.02f;
        
        SetMusicVolume(music, 1.0f - alphaRitual);  
        
        if (alphaRitual > 1.0f) finishScreen = 1;
    }
    
    UpdateMusicStream(music);
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
    DrawTexture(background, 0, 0, WHITE);
    
    // DrawText("STARS ARE ALIGNED! NO TIME TO LOOSE! LIGHT MY RITUAL!",
    
    // Draw foreground and circles
    if ((currentLightedLevel == LEVEL_FINISHED) || (currentLightedLevel == LEVEL_III)) DrawTexture(foregroundIII, 0, 0, WHITE);
    else if (currentLightedLevel == LEVEL_II) DrawTexture(foregroundII, 0, 0, WHITE);
    else if (currentLightedLevel == LEVEL_I) DrawTexture(foregroundI, 0, 0, WHITE);

    // Draw lighted circles (depends on current lighted level)
    switch (currentLightedLevel)
    {
        case LEVEL_FINISHED:
        {
            DrawTexture(circleIIIon, GetScreenWidth()/2 - circleIIIon.width/2, GetScreenHeight()/2 - circleIIIon.height/2, WHITE);
            DrawTexture(circleIIon, GetScreenWidth()/2 - circleIIon.width/2, GetScreenHeight()/2 - circleIIon.height/2, WHITE);
            DrawTexture(circleIon, GetScreenWidth()/2 - circleIon.width/2, GetScreenHeight()/2 - circleIon.height/2, WHITE);
        } break;
        case LEVEL_III:
        {
            DrawTexture(circleIIIoff, GetScreenWidth()/2 - circleIIIoff.width/2, GetScreenHeight()/2 - circleIIIoff.height/2, WHITE);
            DrawTexture(circleIIon, GetScreenWidth()/2 - circleIIon.width/2, GetScreenHeight()/2 - circleIIon.height/2, WHITE);
            DrawTexture(circleIon, GetScreenWidth()/2 - circleIon.width/2, GetScreenHeight()/2 - circleIon.height/2, WHITE);
        } break;
        case LEVEL_II:
        {
            DrawTexture(circleIIoff, GetScreenWidth()/2 - circleIIoff.width/2, GetScreenHeight()/2 - circleIIoff.height/2, WHITE);
            DrawTexture(circleIon, GetScreenWidth()/2 - circleIon.width/2, GetScreenHeight()/2 - circleIon.height/2, WHITE);
        } break;
        case LEVEL_I:
        {
            DrawTexture(circleIoff, GetScreenWidth()/2 - circleIoff.width/2, GetScreenHeight()/2 - circleIoff.height/2, WHITE);
        } break;
        default: break;
    }
    
    // Draw lights (depends on current lighted level)
    switch (currentLightedLevel)
    {
        case LEVEL_FINISHED:
        case LEVEL_III:
        {
            for (int i = 0; i < MAX_LIGHTS_III; i++)
            {
                //if (lightsIII[i].active) DrawCircleV(lightsIII[i].position, lightsIII[i].radius, GOLD);
                //else DrawCircleLines(lightsIII[i].position.x, lightsIII[i].position.y, lightsIII[i].radius, GRAY);
                DrawTextureRec(texLight, lightsIII[i].frameRec, (Vector2){ lightsIII[i].position.x - 32, lightsIII[i].position.y - 32 }, WHITE);
            }
            
            for (int i = 0; i < MAX_LIGHTS_III; i++) if (lightsIII[i].active) DrawTexture(lightGlow, lightsIII[i].position.x - lightGlow.width/2, lightsIII[i].position.y - lightGlow.height/2, Fade(WHITE, 0.3f));
            for (int i = 0; i < MAX_LIGHTS_III; i++) DrawText(FormatText("%02i", lightsIII[i].requiredEnergy), lightsIII[i].position.x - 10, lightsIII[i].position.y + 14, 20, lightsIII[i].active ? GRAY : YELLOW);
        }
        case LEVEL_II:
        {
            for (int i = 0; i < MAX_LIGHTS_II; i++)
            {
                //if (lightsII[i].active) DrawCircleV(lightsII[i].position, lightsII[i].radius, GOLD);
                //else DrawCircleLines(lightsI[i].position.x, lightsI[i].position.y, lightsI[i].radius, GRAY);
                DrawTextureRec(texLight, lightsII[i].frameRec, (Vector2){ lightsII[i].position.x - 32, lightsII[i].position.y - 32 }, WHITE);
            }
            
            for (int i = 0; i < MAX_LIGHTS_II; i++) if (lightsII[i].active) DrawTexture(lightGlow, lightsII[i].position.x - lightGlow.width/2, lightsII[i].position.y - lightGlow.height/2, Fade(WHITE, 0.3f));
            for (int i = 0; i < MAX_LIGHTS_II; i++) DrawText(FormatText("%02i", lightsII[i].requiredEnergy), lightsII[i].position.x - 10, lightsII[i].position.y + 14, 20, lightsII[i].active ? GRAY : YELLOW);
        }
        case LEVEL_I:
        {
            for (int i = 0; i < MAX_LIGHTS_I; i++)
            {
                //if (lightsI[i].active) DrawCircleV(lightsI[i].position, lightsI[i].radius, GOLD);
                //else DrawCircleLines(lightsI[i].position.x, lightsI[i].position.y, lightsI[i].radius, GRAY);
                DrawTextureRec(texLight, lightsI[i].frameRec, (Vector2){ lightsI[i].position.x - 32, lightsI[i].position.y - 32 }, WHITE);
            }
            
            for (int i = 0; i < MAX_LIGHTS_I; i++) if (lightsI[i].active) DrawTexture(lightGlow, lightsI[i].position.x - lightGlow.width/2, lightsI[i].position.y - lightGlow.height/2, Fade(WHITE, 0.3f));
            for (int i = 0; i < MAX_LIGHTS_I; i++) DrawText(FormatText("%02i", lightsI[i].requiredEnergy), lightsI[i].position.x - 10, lightsI[i].position.y + 14, 20, lightsI[i].active ? GRAY : YELLOW);
        }
        default: break;
    }
    
    // Draw main lighter
    DrawTexture(book, GetScreenWidth()/2 - book.width/2, GetScreenHeight()/2, WHITE);
    DrawTexture(lightRay, GetScreenWidth()/2 - lightRay.width/2, 0, Fade(WHITE, 0.5f));
    
    // Draw player
    //DrawCircleV(player.position, player.radius, player.color);
    DrawTexture(texPlayer, player.position.x - 32, player.position.y - 32, player.color);
    
    if (currentLightedLevel != LEVEL_FINISHED)
    {
        // Draw enemies (depends on current lighted level)
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (enemies[i].active) 
            {
                //DrawCircleV(enemies[i].position, enemies[i].radius, enemies[i].color);
                DrawTextureRec(texEnemy, (Rectangle){ 0, 0, 64, 64 }, (Vector2){ enemies[i].position.x - 32, enemies[i].position.y - 32 }, WHITE);
            }
        }
        
        // Draw time left for ritual
        DrawTextEx(font, FormatText("%02.2f", (99.0f - ritualTime)), (Vector2){ 560, 20 }, font.baseSize, 0, WHITE);
        
        // Draw light energy bar
        DrawRectangle(20, 30, 400, 20, GRAY);
        DrawRectangle(20, 30, (400*player.lightEnergy)/MAX_PLAYER_ENERGY, 20, GOLD);
        DrawRectangleLines(20, 30, 400, 20, LIGHTGRAY);
        DrawText(FormatText("%03.0f", player.lightEnergy), 430, 30, 20, WHITE);
        
        // Draw level lighted bar (for completion)
        DrawRectangle(GetScreenWidth() - 40, 30, 20, 660, GRAY);
        DrawRectangle(GetScreenWidth() - 40, 30 + 660 - 660*currentLightEnergy/maxLightEnergy, 20, 660*currentLightEnergy/maxLightEnergy, YELLOW);
        DrawRectangleLines(GetScreenWidth() - 40, 30, 20, 660, LIGHTGRAY);
        
        // Show message: "You run out of light!!!" if player.lightEnergy <= 0
        if (player.lightEnergy < 2)
        {
            if ((framesCounter/20)%2) DrawTextEx(font, "YOU'RE RUNNING OUT OF LIGHT!", (Vector2){ 20, 60 }, font.baseSize/2, 0, WHITE);
        }
    }
    else if (!timeOver)   // LEVEL_FINISHED
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.4f));
        
        // Wait some time before jumping to ending: raylib
        DrawTexture(texRitual, GetScreenWidth()/2 - texRitual.width/2, 100, WHITE);
        DrawTextEx(font, FormatText("BEST LIGHTING TIME: %02.2f", ritualTime), (Vector2){ 320, 340 }, 50, 0, WHITE);
        DrawTextEx(font, "PRESS ENTER to START the RITUAL", (Vector2){ 160, 480 }, 60, 0, WHITE);
        
        if (IsKeyPressed(KEY_ENTER)) startRitual = true;
    }
    
    if (timeOver)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.4f));
        
        DrawTexture(texTimeOver, GetScreenWidth()/2 - texTimeOver.width/2, 140, WHITE);
        DrawTextEx(font, FormatText("NEXT STARS ALIGNMENT IN %i YEARS", nextStarsAlignment), (Vector2){ 200, 360 }, 50, 0, WHITE);
        DrawTextEx(font, "PRESS ENTER to GO HOME...", (Vector2){ 260, 480 }, 60, 0, WHITE);
        
        if (IsKeyPressed(KEY_ENTER)) finishScreen = 2;
    }
    
    if (startRitual) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, alphaRitual));
    
    if (pause) DrawTextEx(font, "RITUAL PAUSED", (Vector2){ GetScreenWidth()/2 - MeasureText("RITUAL PAUSED", 40)/2, 110 }, 50, 0, WHITE);
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    // Unload GAMEPLAY screen variables here!
    UnloadTexture(background);
    UnloadTexture(foregroundI);
    UnloadTexture(foregroundII);
    UnloadTexture(foregroundIII);
    UnloadTexture(texPlayer);
    UnloadTexture(texEnemy);
    UnloadTexture(texLight);
    UnloadTexture(lightGlow);
    UnloadTexture(lightRay);
    UnloadTexture(book);
    UnloadTexture(texRitual);
    UnloadTexture(texTimeOver);
    
    // Unload circles
    UnloadTexture(circleIoff);
    UnloadTexture(circleIIoff);
    UnloadTexture(circleIIIoff);
    UnloadTexture(circleIon);
    UnloadTexture(circleIIon);
    UnloadTexture(circleIIIon);
    
    // Unload sounds
    UnloadSound(fxLightOn);
    UnloadSound(fxLightOff);
    
    UnloadMusicStream(music);
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

// Check two colors if equal
static bool ColorEqual(Color col1, Color col2)
{
    return ((col1.r == col2.r) && (col1.g == col2.g) && (col1.b == col2.b) && (col1.a == col2.a));
}

// Substract two vectors
static Vector2 Vector2Subtract(Vector2 v1, Vector2 v2)
{
    Vector2 result;

    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;

    return result;
}

// Normalize provided vector
static void Vector2Normalize(Vector2 *v)
{
    float length, ilength;

    length = sqrt(v->x*v->x + v->y*v->y);

    if (length == 0) length = 1.0f;

    ilength = 1.0f/length;

    v->x *= ilength;
    v->y *= ilength;
}

// Reset enemy parameters
// NOTE: Depends on currentLightedLevel
static void EnemyReset(Enemy *enemy)
{
    enemy->active = false;
    enemy->framesCounter = 0;
    enemy->color = RED;
    enemy->radius = 10;

    int side = GetRandomValue(0, 1);
    
    if (side) enemy->position = (Vector2){ GetRandomValue(50, 150), GetRandomValue(50, GetScreenHeight() - 50) };
    else enemy->position = (Vector2){ GetRandomValue(GetScreenWidth() - 150, GetScreenWidth() - 50), GetRandomValue(50, GetScreenHeight() - 50) };

    // TODO: Choose only active lights
    // TODO: if currentLightedLevel has no active lights, choose light from a lower level!
    
    if (currentLightedLevel == LEVEL_I)
    {
        enemy->targetNum = GetRandomValue(0, MAX_LIGHTS_I - 1);         // LEVEL_I
        enemy->targetPos = lightsI[enemy->targetNum].position;
        enemy->speed = (float)GetRandomValue(15, 20)/10.0f;
        enemy->awakeFramesDelay = GetRandomValue(90, 400);
    }
    else if (currentLightedLevel == LEVEL_II)
    {
        enemy->targetNum = GetRandomValue(0, MAX_LIGHTS_II - 1);         // LEVEL_II
        enemy->targetPos = lightsII[enemy->targetNum].position;
        enemy->speed = (float)GetRandomValue(10, 20)/10.0f;
        enemy->awakeFramesDelay = GetRandomValue(240, 800);
    }
    else if (currentLightedLevel == LEVEL_III)
    {
        enemy->targetNum = GetRandomValue(0, MAX_LIGHTS_III - 1);         // LEVEL_III
        enemy->targetPos = lightsIII[enemy->targetNum].position;
        enemy->speed = (float)GetRandomValue(8, 18)/10.0f;
        enemy->awakeFramesDelay = GetRandomValue(180, 1200);
    }
}
