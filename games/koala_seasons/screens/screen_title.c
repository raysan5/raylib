/**********************************************************************************************
*
*   raylib - Koala Seasons game
*
*   Title Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
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

#include "atlas01.h"
#include "atlas02.h"

#define MAX_DURATION 120
#define MAX_particle 128

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

typedef struct {
    Vector2 position;
    Vector2 speed;
    float rotation;
    float size;
    Color color;
    float alpha;
    float rotPhy;
    bool active;
} Particle;

typedef struct {
    Vector2 position;
    Color color;
    float alpha;
    float size;
    float rotation;
    bool active;        // NOTE: Use it to activate/deactive particle
    bool fading;
    float delayCounter;
} RayParticleTitle;

typedef struct {
    Vector2 position;
    bool active;
    int spawnTime;
    int maxTime;
    Particle particle[1024];
} Stormparticleystem;

typedef struct {
    Vector2 position;
    bool active;
    int spawnTime;
    int maxTime;
    Particle particle[MAX_particle];
} particleystemTitle;

typedef struct {
    Vector2 position;
    bool active;
    int spawnTime;
    int maxTime;
    RayParticleTitle particle[20];
} RayparticleystemTitle;

// Title screen global variables
static int framesCounter;
static int finishScreen;
static int globalFrameCounter;
static int currentFrame;
static int thisFrame;
static int parallaxBackOffset;
static int parallaxFrontOffset;

static float currentValue1;
static float currentValue2;
static float initValue1;
static float initValue2;
static float finishValue1;
static float finishValue2;
static float duration;

static Vector2 fontSize;

static bool soundActive;
static bool musicActive;

static Rectangle koalaMenu;

static Rectangle bamboo[5];
static Rectangle player = {0, 0, 0, 0};
static Rectangle soundButton;
static Rectangle speakerButton;

static Color color00, color01, color02, color03;

static particleystemTitle snowParticle;
static particleystemTitle backSnowParticle;
static particleystemTitle dandelionParticle;
static particleystemTitle dandelionBackParticle;
static particleystemTitle planetreeParticle;
static particleystemTitle backPlanetreeParticle;
static particleystemTitle flowerParticle;
static particleystemTitle backFlowerParticle;
static particleystemTitle rainParticle;
static particleystemTitle backRainParticle;
static RayparticleystemTitle rayparticle;
static RayparticleystemTitle backRayparticle;
static Stormparticleystem rainStormParticle;
static Stormparticleystem snowStormParticle;

const char pressToPlay[16] = "Press to play";

//----------------------------------------------------------------------------------
// Title Screen Functions Definition
//----------------------------------------------------------------------------------
static void DrawParallaxFront(void);
static void DrawParallaxMiddle(void);
static void DrawParallaxBack(void);

static float BounceEaseOut(float t,float b , float c, float d);

// Title Screen Initialization logic
void InitTitleScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;
    initValue1 = -100;
    finishValue1 = 100;
    initValue2 = 700;
    finishValue2 = finishValue1 + 220;
    duration = MAX_DURATION;
    initSeason = GetRandomValue(0, 3);
    soundActive = true;
    musicActive = true;
    
    parallaxBackOffset = GetRandomValue(10, 100);
    parallaxFrontOffset = GetRandomValue(100, 200);

    rainChance = GetRandomValue(0, 100);

    snowParticle.position = (Vector2){ 0, 0 };
    snowParticle.active = false;
    backSnowParticle.position = (Vector2){ 0, 0 };
    backSnowParticle.active = false;
    planetreeParticle.position = (Vector2){ 0, 0 };
    planetreeParticle.active = false;
    backPlanetreeParticle.position = (Vector2){ 0, 0 };
    backPlanetreeParticle.active = false;
    dandelionParticle.active = false;
    dandelionBackParticle.position = (Vector2){ 0, 0};
    flowerParticle.position = (Vector2){ 0, 0 };
    flowerParticle.active = false;
    backFlowerParticle.position = (Vector2){ 0, 0 };
    backFlowerParticle.active = false;
    rayparticle.position = (Vector2){ 0, 0 };
    rayparticle.active = false;
    backRayparticle.position = (Vector2){ 0, 0 };
    backRayparticle.active = false;
    rainStormParticle.position = (Vector2){ 0, 0 };
    rainStormParticle.active = false;
    snowStormParticle.position = (Vector2){ 0, 0 };
    snowStormParticle.active = false;
    
    soundButton = (Rectangle){ GetScreenWidth()*0.85, GetScreenHeight()*0.7, title_music_on.width, title_music_on.height };
    speakerButton = (Rectangle){ GetScreenWidth()*0.85, GetScreenHeight()*0.85, title_speaker_on.width, title_speaker_on.height };

    for (int j = 0; j < MAX_particle; j++)
    {
        snowParticle.particle[j].active = false;
        snowParticle.particle[j].position = (Vector2){ 0, 0 };
        snowParticle.particle[j].size = (float)GetRandomValue(3, 9)/10;
        snowParticle.particle[j].rotation = GetRandomValue(0, 360);
        snowParticle.particle[j].color = WHITE;
        snowParticle.particle[j].alpha = 1.0f;

        backSnowParticle.particle[j].active = false;
        backSnowParticle.particle[j].position = (Vector2){ 0, 0 };
        backSnowParticle.particle[j].size = (float)GetRandomValue(2, 8)/10;
        backSnowParticle.particle[j].rotation = GetRandomValue(0, 360);
        backSnowParticle.particle[j].color = WHITE;
        backSnowParticle.particle[j].alpha = 0.7f;

        planetreeParticle.particle[j].active = false;
        planetreeParticle.particle[j].position = (Vector2){ 0, 0 };
        planetreeParticle.particle[j].size = (float)GetRandomValue(3, 9)/10;
        planetreeParticle.particle[j].rotation = GetRandomValue(0, 360);
        planetreeParticle.particle[j].color = WHITE;
        planetreeParticle.particle[j].alpha = 1.0f;

        backPlanetreeParticle.particle[j].active = false;
        backPlanetreeParticle.particle[j].position = (Vector2){ 0, 0 };
        backPlanetreeParticle.particle[j].size = (float)GetRandomValue(2, 8)/10;
        backPlanetreeParticle.particle[j].rotation = GetRandomValue(0, 360);
        backPlanetreeParticle.particle[j].color = WHITE;
        backPlanetreeParticle.particle[j].alpha = 0.7f;

        dandelionParticle.particle[j].active = false;
        dandelionParticle.particle[j].position = (Vector2){ 0, 0 };
        dandelionParticle.particle[j].size = (float)GetRandomValue(3, 9)/10;
        dandelionParticle.particle[j].rotation = 0;
        dandelionParticle.particle[j].color = WHITE;
        dandelionParticle.particle[j].alpha = 1;
        dandelionParticle.particle[j].rotPhy = GetRandomValue(0 , 180);

        dandelionBackParticle.particle[j].active = false;
        dandelionBackParticle.particle[j].position = (Vector2){ 0, 0 };
        dandelionBackParticle.particle[j].size = (float)GetRandomValue(2, 8)/10;
        dandelionBackParticle.particle[j].rotation = 0;
        dandelionBackParticle.particle[j].color = WHITE;
        dandelionBackParticle.particle[j].alpha = 0.7f;
        dandelionBackParticle.particle[j].rotPhy = GetRandomValue(0 , 180);

        flowerParticle.particle[j].active = false;
        flowerParticle.particle[j].position = (Vector2){ 0, 0 };
        flowerParticle.particle[j].size = (float)GetRandomValue(3, 9)/10;
        flowerParticle.particle[j].rotation = GetRandomValue(0, 360);
        flowerParticle.particle[j].color = WHITE;
        flowerParticle.particle[j].alpha = 1.0f;

        backFlowerParticle.particle[j].active = false;
        backFlowerParticle.particle[j].position = (Vector2){ 0, 0 };
        backFlowerParticle.particle[j].size = (float)GetRandomValue(2, 8)/10;
        backFlowerParticle.particle[j].rotation = GetRandomValue(0, 360);
        backFlowerParticle.particle[j].color = WHITE;
        backFlowerParticle.particle[j].alpha = 0.7f;

        rainParticle.particle[j].active = false;
        rainParticle.particle[j].position = (Vector2){ 0, 0 };
        rainParticle.particle[j].size = (float)GetRandomValue(3, 9)/10;
        rainParticle.particle[j].rotation = -20;
        rainParticle.particle[j].color = WHITE;
        rainParticle.particle[j].alpha = 1.0f;

        backRainParticle.particle[j].active = false;
        backRainParticle.particle[j].position = (Vector2){ 0, 0 };
        backRainParticle.particle[j].size = (float)GetRandomValue(2, 8)/10;
        backRainParticle.particle[j].rotation = -20;
        backRainParticle.particle[j].color = WHITE;
        backRainParticle.particle[j].alpha = 0.7f;
    }

    for (int j = 0; j < 1024; j++)
    {
        rainStormParticle.particle[j].active = false;
        rainStormParticle.particle[j].position = (Vector2){ 0, 0 };
        rainStormParticle.particle[j].size = (float)GetRandomValue(3, 9)/10;
        rainStormParticle.particle[j].rotation = -40;
        rainStormParticle.particle[j].color = WHITE;
        rainStormParticle.particle[j].alpha = 1.0f;
    }

    for (int j = 0; j < 256; j++)
    {
        snowStormParticle.particle[j].active = false;
        snowStormParticle.particle[j].position = (Vector2){ 0, 0 };
        snowStormParticle.particle[j].size = (float)GetRandomValue(4, 8)/10;
        snowStormParticle.particle[j].rotation = 40;
        snowStormParticle.particle[j].color = WHITE;
        snowStormParticle.particle[j].alpha = 1.0f;
    }

    for (int i = 0; i < 20; i++)
    {
        rayparticle.particle[i].position = (Vector2){ 0, 0 };
        rayparticle.particle[i].color.r = 255;
        rayparticle.particle[i].color.g = 255;
        rayparticle.particle[i].color.b = 182;
        rayparticle.particle[i].color.a = 255;
        rayparticle.particle[i].alpha = 0.0f;
        rayparticle.particle[i].size = (float)GetRandomValue(15, 20)/10;
        rayparticle.particle[i].rotation = 0.0f;
        rayparticle.particle[i].active = false;
        rayparticle.particle[i].fading = false;
        rayparticle.particle[i].delayCounter = 0;

        backRayparticle.particle[i].position = (Vector2){ 0, 0 };
        backRayparticle.particle[i].color.r = 255;
        backRayparticle.particle[i].color.g = 255;
        backRayparticle.particle[i].color.b = 182;
        backRayparticle.particle[i].color.a = 255;
        backRayparticle.particle[i].alpha = 0.0f;
        backRayparticle.particle[i].size = (float)GetRandomValue(5, 10)/10;
        backRayparticle.particle[i].rotation = 0.0f;
        backRayparticle.particle[i].active = false;
        backRayparticle.particle[i].fading = false;
        backRayparticle.particle[i].delayCounter = 0;
    }

    for (int i = 0; i < 5; i++)
    {
        bamboo[i].x = 150 + 200*i;
        bamboo[i].y = 0;
        bamboo[i].width = 30;
        bamboo[i].height = GetScreenHeight();
    }

    player.x = 350;
    player.y = 100;
    player.width = 35;
    player.height = 60;
    
    koalaMenu.x = gameplay_koala_menu.x;
    koalaMenu.y = gameplay_koala_menu.y;
    koalaMenu.width = gameplay_koala_menu.width/2;
    koalaMenu.height = gameplay_koala_menu.height;
    
    fontSize = MeasureTextEx(font, "PRESS TO PLAY", font.baseSize, 2);
}

// Title Screen Update logic
void UpdateTitleScreen(void)
{
    framesCounter += 1*TIME_FACTOR;
    globalFrameCounter += 1*TIME_FACTOR;

    if (framesCounter < duration)
    {
        currentValue1 = BounceEaseOut((float)framesCounter, initValue1, (finishValue1 - initValue1), duration);
        currentValue2 = BounceEaseOut((float)framesCounter, initValue2, (finishValue2 - initValue2), duration);
    }
    
    thisFrame += 1*TIME_FACTOR;

    if (thisFrame >= 40)
    {
        currentFrame++;
        thisFrame = 0;
    }

    if (currentFrame > 1) currentFrame = 0;

    koalaMenu.x = gameplay_koala_menu.x + koalaMenu.width*currentFrame;

    if (initSeason == 0)
    {
        dandelionParticle.active = true;
        dandelionBackParticle.active = true;
        rayparticle.active = true;
        backRayparticle.active = true;
        
        rainParticle.active = false;
        rainStormParticle.active = false;
        backRainParticle.active = false;

        color00 = (Color){129, 172, 86, 255};              // Summer Color
        color01 = (Color){145, 165, 125, 255};
        color02 = (Color){161, 130, 73, 255};
        color03 = (Color){198, 103, 51, 255};
    }
    else if (initSeason == 1)
    {
        if (rainChance > 40)
        {
            planetreeParticle.active = true;
            backPlanetreeParticle.active = true;
            rainParticle.active = false;
            backRainParticle.active = false;
        }
        else if (rainChance <= 40 && rainChance > 15)
        {
            rainStormParticle.active = true;
            backRainParticle.active = false;
        }
        else if (rainChance <= 15)
        {
            rainStormParticle.active = true;
            backRainParticle.active = false;
        }

        color00 = (Color){242, 113, 62, 255};              // Fall Color
        color01 = (Color){190, 135, 114, 255};
        color02 = (Color){144, 130, 101, 255};
        color03 = (Color){214, 133, 58, 255};

    }
    else if (initSeason == 2)
    {

        if (rainChance > 40)
        {
            snowParticle.active = true;
            backSnowParticle.active = true;
        }
        else
        {
            snowStormParticle.active = true;
            backSnowParticle.active = true;
        }
        
        rainParticle.active = false;
        rainStormParticle.active = false;
        backRainParticle.active = false;

        color00 = (Color){130, 130, 181, 255};              // Winter Color
        color01 = (Color){145, 145, 166, 255};
        color02 = (Color){104, 142, 144, 255};
        color03 = (Color){57, 140, 173, 255};
    }
    else if (initSeason == 3)
    {
        flowerParticle.active = true;
        backFlowerParticle.active = true;
        
        rainParticle.active = false;
        rainStormParticle.active = false;
        backRainParticle.active = false;

        color00 = (Color){196, 176, 49, 255};              // Spring Color
        color01 = (Color){178, 163, 67, 255};
        color02 = (Color){133, 143, 90, 255};
        color03 = (Color){133, 156, 42, 255};
    }

    // Snow Particle
    if (snowParticle.active)
    {
        snowParticle.spawnTime += 1*TIME_FACTOR;

        for (int i = 0; i < MAX_particle; i++)
        {
            if (!snowParticle.particle[i].active && snowParticle.spawnTime >= snowParticle.maxTime)
            {
                snowParticle.particle[i].active = true;
                snowParticle.particle[i].position = (Vector2){GetRandomValue(0, GetScreenWidth() + 200), -10};
                snowParticle.spawnTime = 0;
                snowParticle.maxTime = GetRandomValue (5, 20);
            }
        }
    }

    if (backSnowParticle.active)
    {
        backSnowParticle.spawnTime += 1*TIME_FACTOR;

        for (int i = 0; i < MAX_particle; i++)
        {
            if (!backSnowParticle.particle[i].active && backSnowParticle.spawnTime >= backSnowParticle.maxTime)
            {
                backSnowParticle.particle[i].active = true;
                backSnowParticle.particle[i].position = (Vector2){GetRandomValue(0, GetScreenWidth() + 200), -10};
                backSnowParticle.spawnTime = 0;
                backSnowParticle.maxTime = GetRandomValue (3, 10);
            }
        }
    }

    // Autumn leaves particle
    if (planetreeParticle.active)
    {
        planetreeParticle.spawnTime += 1*TIME_FACTOR;
        backPlanetreeParticle.spawnTime += 1*TIME_FACTOR;

        for (int i = 0; i < MAX_particle; i++)
        {
            if (!planetreeParticle.particle[i].active && planetreeParticle.spawnTime >= planetreeParticle.maxTime)
            {
                planetreeParticle.particle[i].active = true;
                planetreeParticle.particle[i].position = (Vector2){GetRandomValue(0, GetScreenWidth() + 200), -10};
                planetreeParticle.spawnTime = 0;
                planetreeParticle.maxTime = GetRandomValue (5, 20);
            }

            if (!backPlanetreeParticle.particle[i].active && backPlanetreeParticle.spawnTime >= backPlanetreeParticle.maxTime)
            {
                backPlanetreeParticle.particle[i].active = true;
                backPlanetreeParticle.particle[i].position = (Vector2){GetRandomValue(0, GetScreenWidth() + 200), -10};
                backPlanetreeParticle.spawnTime = 0;
                backPlanetreeParticle.maxTime = GetRandomValue (3, 10);
            }
        }
    }

    // Dandelion particle
    if (dandelionParticle.active)
    {
        dandelionParticle.spawnTime += 1*TIME_FACTOR;
        dandelionBackParticle.spawnTime += 1*TIME_FACTOR;

        for (int i = 0; i < MAX_particle; i++)
        {
            if (!dandelionParticle.particle[i].active && dandelionParticle.spawnTime >= dandelionParticle.maxTime)
            {
                dandelionParticle.particle[i].active = true;
                dandelionParticle.particle[i].position = (Vector2){GetRandomValue(0, GetScreenWidth() + 200), -10};
                dandelionParticle.spawnTime = 0;
                dandelionParticle.maxTime = GetRandomValue (5, 20);
            }

            if (!dandelionBackParticle.particle[i].active && dandelionBackParticle.spawnTime >= dandelionBackParticle.maxTime)
            {
                dandelionBackParticle.particle[i].active = true;
                dandelionBackParticle.particle[i].position = (Vector2){GetRandomValue(0, GetScreenWidth() + 200), -10};
                dandelionBackParticle.spawnTime = 0;
                dandelionBackParticle.maxTime = GetRandomValue (3, 10);
            }
        }
    }

    // Flower Particle
    if (flowerParticle.active)
    {

        flowerParticle.spawnTime += 1*TIME_FACTOR;
        backFlowerParticle.spawnTime += 1*TIME_FACTOR;

        for (int i = 0; i < MAX_particle; i++)
        {
            if (!flowerParticle.particle[i].active && flowerParticle.spawnTime >= flowerParticle.maxTime)
            {
                flowerParticle.particle[i].active = true;
                flowerParticle.particle[i].position = (Vector2){GetRandomValue(0, GetScreenWidth() + 200), -10};
                flowerParticle.spawnTime = 0;
                flowerParticle.maxTime = GetRandomValue (5, 20);
            }

            if (!backFlowerParticle.particle[i].active && backFlowerParticle.spawnTime >= backFlowerParticle.maxTime)
            {
                backFlowerParticle.particle[i].active = true;
                backFlowerParticle.particle[i].position = (Vector2){GetRandomValue(0, GetScreenWidth() + 200), -10};
                backFlowerParticle.spawnTime = 0;
                backFlowerParticle.maxTime = GetRandomValue (3, 10);
            }
        }
    }

    // Storm particle
    if (rainStormParticle.active)
    {

        rainStormParticle.spawnTime += 1*TIME_FACTOR;

        for (int i = 0; i < 1024; i++)
        {
            if (!rainStormParticle.particle[i].active && rainStormParticle.spawnTime >= rainStormParticle.maxTime)
            {
                for (int j = 0; j < 16; j++)
                {
                    rainStormParticle.particle[i+j].active = true;
                    rainStormParticle.particle[i+j].position = (Vector2){GetRandomValue(100, GetScreenWidth() + 1000), GetRandomValue(-10,-20)};
                }
                
                rainStormParticle.spawnTime = 0;
                rainStormParticle.maxTime = 4;
            }
        }
    }

    // Snow Storm particle
    if (snowStormParticle.active)
    {
        snowStormParticle.spawnTime += 1*TIME_FACTOR;

        for (int i = 0; i < 256; i++)
        {
            if (!snowStormParticle.particle[i].active && snowStormParticle.spawnTime >= snowStormParticle.maxTime)
            {
                snowStormParticle.particle[i].active = true;
                snowStormParticle.particle[i].position = (Vector2){GetRandomValue(100, GetScreenWidth() + 800), -10};
                snowStormParticle.spawnTime = 0;
                snowStormParticle.maxTime = GetRandomValue (1, 2);
            }
        }
    }

    if (rayparticle.active)
    {
        rayparticle.spawnTime += 1*TIME_FACTOR;
        backRayparticle.spawnTime += 1*TIME_FACTOR;

        for (int i = 0; i < 20; i++)
        {
            if (!rayparticle.particle[i].active && rayparticle.spawnTime >= rayparticle.maxTime)
            {
                //printf("PARTICLEEES");
                rayparticle.particle[i].active = true;
                rayparticle.particle[i].alpha = 0.0f;
                rayparticle.particle[i].size = (float)(GetRandomValue(10, 20)/10);
                rayparticle.particle[i].position = (Vector2){GetRandomValue(300, GetScreenWidth() + 200), 0};
                rayparticle.particle[i].rotation = -35;
                rayparticle.spawnTime = 0;
                rayparticle.particle[i].delayCounter = 0;
                rayparticle.maxTime = GetRandomValue (20, 50);
            }

            if (!backRayparticle.particle[i].active && backRayparticle.spawnTime >= backRayparticle.maxTime)
            {
                backRayparticle.particle[i].active = true;
                backRayparticle.particle[i].alpha = 0.0f;
                backRayparticle.particle[i].size = (float)(GetRandomValue(5, 15)/10);
                backRayparticle.particle[i].position = (Vector2){GetRandomValue(300, GetScreenWidth() + 200), 0};
                backRayparticle.particle[i].rotation = -35;
                backRayparticle.spawnTime = 0;
                backRayparticle.particle[i].delayCounter = 0;
                backRayparticle.maxTime = GetRandomValue (20, 50);
            }
        }
    }

    if (rainParticle.active)
    {
        rainParticle.spawnTime += 1*TIME_FACTOR;

        for (int i = 0; i < MAX_particle; i++)
        {
            if (!rainParticle.particle[i].active && rainParticle.spawnTime >= rainParticle.maxTime)
            {
                rainParticle.particle[i].active = true;
                rainParticle.particle[i].position = (Vector2){GetRandomValue(0, GetScreenWidth() + 200), -10};
                rainParticle.spawnTime = 0;
                rainParticle.maxTime = GetRandomValue (1, 8);
            }
        }
    }

    if (backRainParticle.active)
    {
        backRainParticle.spawnTime += 1*TIME_FACTOR;

        for (int i = 0; i < MAX_particle; i++)
        {

            if (!backRainParticle.particle[i].active && backRainParticle.spawnTime >= backRainParticle.maxTime)
            {
                backRainParticle.particle[i].active = true;
                backRainParticle.particle[i].position = (Vector2){GetRandomValue(0, GetScreenWidth() + 200), -10};
                backRainParticle.spawnTime = 0;
                backRainParticle.maxTime = GetRandomValue (3, 10);
            }
        }
    }

    // particle Logic
    for (int i = 0; i < MAX_particle; i++)
    {
        if (snowParticle.particle[i].active)
        {
            snowParticle.particle[i].position.y += 2*TIME_FACTOR;
            snowParticle.particle[i].position.x -= 2*TIME_FACTOR;
            snowParticle.particle[i].rotation += 0.5*TIME_FACTOR;
            if (snowParticle.particle[i].position.y >= GetScreenHeight()) snowParticle.particle[i].active = false;
        }

        if (backSnowParticle.particle[i].active)
        {
            backSnowParticle.particle[i].position.y += 4*TIME_FACTOR;
            backSnowParticle.particle[i].position.x -= 3*TIME_FACTOR;
            backSnowParticle.particle[i].rotation += 0.5*TIME_FACTOR;
            if (backSnowParticle.particle[i].position.y >= GetScreenHeight()) backSnowParticle.particle[i].active = false;
        }

        if (planetreeParticle.particle[i].active)
        {
            planetreeParticle.particle[i].position.y += 4*TIME_FACTOR;
            planetreeParticle.particle[i].position.x -= 2*TIME_FACTOR;
            planetreeParticle.particle[i].rotation += 0.5*TIME_FACTOR;
            if (planetreeParticle.particle[i].position.y >= GetScreenHeight()) planetreeParticle.particle[i].active = false;
        }

        if (backPlanetreeParticle.particle[i].active)
        {
            backPlanetreeParticle.particle[i].position.y += 4*TIME_FACTOR;
            backPlanetreeParticle.particle[i].position.x -= 3*TIME_FACTOR;
            backPlanetreeParticle.particle[i].rotation += 0.5*TIME_FACTOR;
            if (backPlanetreeParticle.particle[i].position.y >= GetScreenHeight()) backPlanetreeParticle.particle[i].active = false;
        }

        if (dandelionParticle.particle[i].active)
        {
            dandelionParticle.particle[i].position.y += 2.5*TIME_FACTOR;
            dandelionParticle.particle[i].position.x -= 2*TIME_FACTOR;
            dandelionParticle.particle[i].rotation = -(30*sin(2*PI/120*globalFrameCounter + dandelionParticle.particle[i].rotPhy) + 30);
            if (dandelionParticle.particle[i].position.y >= GetScreenHeight()) dandelionParticle.particle[i].active = false;
        }

        if (dandelionBackParticle.particle[i].active)
        {
            dandelionBackParticle.particle[i].position.y += 2*TIME_FACTOR;
            dandelionBackParticle.particle[i].position.x -= 3*TIME_FACTOR;
            dandelionBackParticle.particle[i].rotation = -(30*sin(2*PI/120*globalFrameCounter + dandelionParticle.particle[i].rotPhy) + 30);
            if (dandelionBackParticle.particle[i].position.y >= GetScreenHeight()) dandelionBackParticle.particle[i].active = false;
        }

        if (flowerParticle.particle[i].active)
        {
            flowerParticle.particle[i].position.y += 2.5*TIME_FACTOR;
            flowerParticle.particle[i].position.x -= 2*TIME_FACTOR;
            flowerParticle.particle[i].rotation += 0.5*TIME_FACTOR;
            if (flowerParticle.particle[i].position.y >= GetScreenHeight()) flowerParticle.particle[i].active = false;
        }

        if (backFlowerParticle.particle[i].active)
        {
            backFlowerParticle.particle[i].position.y += 2*TIME_FACTOR;
            backFlowerParticle.particle[i].position.x -= 3*TIME_FACTOR;
            backFlowerParticle.particle[i].rotation += 0.5*TIME_FACTOR;
            if (backFlowerParticle.particle[i].position.y >= GetScreenHeight()) backFlowerParticle.particle[i].active = false;
        }

        if (rainParticle.particle[i].active)
        {
            rainParticle.particle[i].position.y += 4*TIME_FACTOR;
            rainParticle.particle[i].position.x -= 5*TIME_FACTOR;
            //rainParticle.particle[i].rotation += 0.5;
            if (rainParticle.particle[i].position.y >= GetScreenHeight()) rainParticle.particle[i].active = false;
        }

        if (backRainParticle.particle[i].active)
        {
            backRainParticle.particle[i].position.y += 3*TIME_FACTOR;
            backRainParticle.particle[i].position.x -= 3*TIME_FACTOR;
            //rainParticle.particle[i].rotation += 0.5;
            if (backRainParticle.particle[i].position.y >= GetScreenHeight()) backRainParticle.particle[i].active = false;
        }
    }

    for (int i = 0; i < 1024; i++)
    {
        if (rainStormParticle.particle[i].active)
        {
            rainStormParticle.particle[i].position.y += 12*TIME_FACTOR;
            rainStormParticle.particle[i].position.x -= 15*TIME_FACTOR;
            //rainParticle.particle[i].rotation += 0.5;
            if (rainStormParticle.particle[i].position.y >= GetScreenHeight()) rainStormParticle.particle[i].active = false;
            if (rainStormParticle.active == false)rainStormParticle.particle[i].alpha -= 0.01;
        }
    }

    for (int i = 0; i < 256; i++)
    {
        if (snowStormParticle.particle[i].active)
        {
            snowStormParticle.particle[i].position.y += 12;
            snowStormParticle.particle[i].position.x -= 15;
            snowStormParticle.particle[i].rotation += 0.5;
            if (snowStormParticle.particle[i].position.y >= GetScreenHeight()) snowStormParticle.particle[i].active = false;
        }
    }

    for (int i = 0; i < 20; i++)
    {
        if (rayparticle.particle[i].active)
        {
            rayparticle.particle[i].position.x -= 0.5*TIME_FACTOR;

            if (rayparticle.particle[i].fading)
            {
                rayparticle.particle[i].alpha -= 0.01f;

                if (rayparticle.particle[i].alpha <= 0)
                {
                    rayparticle.particle[i].alpha = 0;
                    rayparticle.particle[i].delayCounter++;
                    if (rayparticle.particle[i].delayCounter >= 30)
                    {
                        rayparticle.particle[i].active = false;
                        rayparticle.particle[i].delayCounter = 0;
                        rayparticle.particle[i].fading = false;
                    }
                }
            }
            else
            {
                rayparticle.particle[i].alpha += 0.01f;

                if (rayparticle.particle[i].alpha >= 0.5f)
                {
                    rayparticle.particle[i].alpha = 0.5f;
                    rayparticle.particle[i].delayCounter++;

                    if (rayparticle.particle[i].delayCounter >= 30)
                    {
                        rayparticle.particle[i].delayCounter = 0;
                        rayparticle.particle[i].fading = true;
                    }
                }
            }
        }

        if (backRayparticle.particle[i].active)
        {
            backRayparticle.particle[i].position.x -= 0.5;

            if (backRayparticle.particle[i].fading)
            {
                backRayparticle.particle[i].alpha -= 0.01f;
                if (backRayparticle.particle[i].alpha <= 0)
                {
                    backRayparticle.particle[i].alpha = 0;
                    backRayparticle.particle[i].delayCounter++;
                    if (backRayparticle.particle[i].delayCounter >= 30)
                    {
                        backRayparticle.particle[i].active = false;
                        backRayparticle.particle[i].delayCounter = 0;
                        backRayparticle.particle[i].fading = false;
                    }
                }
            }
            else
            {
                backRayparticle.particle[i].alpha += 0.01f;
                if (backRayparticle.particle[i].alpha >= 0.5f)
                {
                    backRayparticle.particle[i].alpha = 0.5f;
                    backRayparticle.particle[i].delayCounter++;
                    if (backRayparticle.particle[i].delayCounter >= 30)
                    {
                        backRayparticle.particle[i].delayCounter = 0;
                        backRayparticle.particle[i].fading = true;
                    }
                }
            }
        }
    }

    // Press enter to change to GAMEPLAY screen
#if (defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB))
    if (((IsGestureDetected(GESTURE_TAP) || (GetGestureDetected() == GESTURE_DOUBLETAP)) && framesCounter >= duration))
    {
        //finishScreen = 1;   // OPTIONS
        finishScreen = 2;   // GAMEPLAY
    }
#elif (defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB))
    if ((IsKeyPressed(KEY_ENTER) && framesCounter >= duration))
    {
        //finishScreen = 1;   // OPTIONS
        finishScreen = 2;   // GAMEPLAY
    }
#endif
}

// Title Screen Draw logic
void DrawTitleScreen(void)
{
    BeginShaderMode(colorBlend);
    
    DrawTexturePro(atlas02, gameplay_background, (Rectangle){0, 0, gameplay_background.width*2, gameplay_background.height*2}, (Vector2){0, 0}, 0, color02);

    // Draw parallax
    DrawParallaxBack();
    DrawParallaxMiddle();

    for (int i = 0; i < MAX_particle; i++)
    {
        if (backSnowParticle.particle[i].active) DrawTexturePro(atlas02, particle_icecrystal_bw,
                                               (Rectangle){ backSnowParticle.particle[i].position.x, backSnowParticle.particle[i].position.y, particle_icecrystal_bw.width*backSnowParticle.particle[i].size, particle_icecrystal_bw.height*backSnowParticle.particle[i].size },
                                               (Vector2){ particle_icecrystal_bw.width*backSnowParticle.particle[i].size/2, particle_icecrystal_bw.height*backSnowParticle.particle[i].size/2 }, backSnowParticle.particle[i].rotation,
                                               Fade((Color){144, 214, 255, 255}, backSnowParticle.particle[i].alpha));

        if (backPlanetreeParticle.particle[i].active) DrawTexturePro(atlas02, particle_planetreeleaf_bw,
                                               (Rectangle){ backPlanetreeParticle.particle[i].position.x, backPlanetreeParticle.particle[i].position.y, particle_planetreeleaf_bw.width*backPlanetreeParticle.particle[i].size, particle_planetreeleaf_bw.height*backPlanetreeParticle.particle[i].size },
                                               (Vector2){ particle_planetreeleaf_bw.width*backPlanetreeParticle.particle[i].size/2, particle_planetreeleaf_bw.height*backPlanetreeParticle.particle[i].size/2 }, backPlanetreeParticle.particle[i].rotation,
                                               Fade((Color){179, 86, 6, 255}, backPlanetreeParticle.particle[i].alpha));

        if (dandelionBackParticle.particle[i].active) DrawTexturePro(atlas02, particle_dandelion_bw,
                                               (Rectangle){ dandelionBackParticle.particle[i].position.x, dandelionBackParticle.particle[i].position.y, particle_dandelion_bw.width*dandelionBackParticle.particle[i].size, particle_dandelion_bw.height*dandelionBackParticle.particle[i].size },
                                               (Vector2){ particle_dandelion_bw.width*dandelionBackParticle.particle[i].size/2, particle_dandelion_bw.height*dandelionBackParticle.particle[i].size/2 }, dandelionBackParticle.particle[i].rotation,
                                               Fade((Color){202, 167, 126, 255}, dandelionBackParticle.particle[i].alpha));

        if (backFlowerParticle.particle[i].active) DrawTexturePro(atlas02, particle_ecualyptusflower_bw,
                                               (Rectangle){ backFlowerParticle.particle[i].position.x, backFlowerParticle.particle[i].position.y, particle_ecualyptusflower_bw.width*backFlowerParticle.particle[i].size, particle_ecualyptusflower_bw.height*backFlowerParticle.particle[i].size },
                                               (Vector2){ particle_ecualyptusflower_bw.width*backFlowerParticle.particle[i].size/2, particle_ecualyptusflower_bw.height*backFlowerParticle.particle[i].size/2 }, backFlowerParticle.particle[i].rotation,
                                               Fade((Color){218, 84, 108, 255}, backFlowerParticle.particle[i].alpha));

        if (backRainParticle.particle[i].active) DrawTexturePro(atlas02, particle_waterdrop_bw,
                                               (Rectangle){ backRainParticle.particle[i].position.x, backRainParticle.particle[i].position.y, particle_waterdrop_bw.width*backRainParticle.particle[i].size, particle_waterdrop_bw.height*backRainParticle.particle[i].size },
                                               (Vector2){ particle_waterdrop_bw.width*backRainParticle.particle[i].size/2, particle_waterdrop_bw.height*backRainParticle.particle[i].size/2 }, backRainParticle.particle[i].rotation,
                                               Fade((Color){144, 183, 187, 255}, backRainParticle.particle[i].alpha));
    }
    
    for (int i = 0; i < 20; i++)
    {
       if (backRayparticle.particle[i].active) DrawTexturePro(atlas02, gameplay_back_fx_lightraymid,
                                               (Rectangle){ backRayparticle.particle[i].position.x, backRayparticle.particle[i].position.y, gameplay_back_fx_lightraymid.width*backRayparticle.particle[i].size, gameplay_back_fx_lightraymid.height*backRayparticle.particle[i].size },
                                               (Vector2){ gameplay_back_fx_lightraymid.width*backRayparticle.particle[i].size/2, gameplay_back_fx_lightraymid.height*backRayparticle.particle[i].size/2 }, backRayparticle.particle[i].rotation,
                                               Fade(GOLD, backRayparticle.particle[i].alpha));
    }

    DrawParallaxFront();
    
    for (int i = 0; i < 5; i++)
    {
        DrawTexturePro(atlas02, gameplay_props_tree, (Rectangle){bamboo[i].x, bamboo[i].y, 43, 720}, (Vector2){0, 0}, 0, color03);
        //DrawRectangleRec(bamboo[i], Fade(LIME, 0.5));
    }

    EndShaderMode();
    
    DrawTextureRec(atlas01, koalaMenu, (Vector2){player.x - player.width, player.y - 40}, WHITE);

    BeginShaderMode(colorBlend);

    DrawTexturePro(atlas02, gameplay_back_ground00, (Rectangle){0, 637, gameplay_back_ground00.width*2, gameplay_back_ground00.height*2}, (Vector2){0,0}, 0, color00);

    EndShaderMode();
    
    DrawTexturePro(atlas01, (Rectangle){title_titletext.x, title_titletext.y, title_titletext.width, 230},  (Rectangle){GetScreenWidth()*0.49F - title_titletext.width/2, currentValue1, title_titletext.width, 235}, (Vector2){0, 0}, 0, WHITE);
    DrawTexturePro(atlas01, (Rectangle){title_titletext.x, title_titletext.y + 232, title_titletext.width, 116}, (Rectangle){GetScreenWidth()*0.49F - title_titletext.width/2, currentValue2, title_titletext.width, 116}, (Vector2){0, 0}, 0, WHITE);

    if ((framesCounter/60)%2 && framesCounter >= duration) DrawTextEx(font, pressToPlay, (Vector2){ GetScreenWidth()/2 - fontSize.x/2, GetScreenHeight()/2 + fontSize.y*2 }, font.baseSize, 2, (Color){247, 239, 209, 255});

    for (int i = 0; i < MAX_particle; i++)
    {
        if (snowParticle.particle[i].active) DrawTexturePro(atlas01, particle_icecrystal,
                                               (Rectangle){ snowParticle.particle[i].position.x, snowParticle.particle[i].position.y, particle_icecrystal.width*snowParticle.particle[i].size, particle_icecrystal.height*snowParticle.particle[i].size },
                                               (Vector2){ particle_icecrystal.width*snowParticle.particle[i].size/2, particle_icecrystal.height*snowParticle.particle[i].size/2 }, snowParticle.particle[i].rotation,
                                               Fade(snowParticle.particle[i].color, snowParticle.particle[i].alpha));

        if (planetreeParticle.particle[i].active) DrawTexturePro(atlas01, particle_planetreeleaf,
                                               (Rectangle){ planetreeParticle.particle[i].position.x, planetreeParticle.particle[i].position.y, particle_planetreeleaf.width*planetreeParticle.particle[i].size, particle_planetreeleaf.height*planetreeParticle.particle[i].size },
                                               (Vector2){ particle_planetreeleaf.width*planetreeParticle.particle[i].size/2, particle_planetreeleaf.height*planetreeParticle.particle[i].size/2 }, planetreeParticle.particle[i].rotation,
                                               Fade(planetreeParticle.particle[i].color, planetreeParticle.particle[i].alpha));

        if (dandelionParticle.particle[i].active) DrawTexturePro(atlas01, particle_dandelion,
                                               (Rectangle){ dandelionParticle.particle[i].position.x, dandelionParticle.particle[i].position.y, particle_dandelion.width*dandelionParticle.particle[i].size, particle_dandelion.height*dandelionParticle.particle[i].size },
                                               (Vector2){ particle_dandelion.width*dandelionParticle.particle[i].size/2, particle_dandelion.height*dandelionParticle.particle[i].size/2 }, dandelionParticle.particle[i].rotation,
                                               Fade(dandelionParticle.particle[i].color, dandelionParticle.particle[i].alpha));

        if (flowerParticle.particle[i].active) DrawTexturePro(atlas01, particle_ecualyptusflower,
                                               (Rectangle){ flowerParticle.particle[i].position.x, flowerParticle.particle[i].position.y, particle_ecualyptusflower.width*flowerParticle.particle[i].size, particle_ecualyptusflower.height*flowerParticle.particle[i].size },
                                               (Vector2){ particle_ecualyptusflower.width*flowerParticle.particle[i].size/2, particle_ecualyptusflower.height*flowerParticle.particle[i].size/2 }, flowerParticle.particle[i].rotation,
                                               Fade(flowerParticle.particle[i].color, flowerParticle.particle[i].alpha));

        if (rainParticle.particle[i].active) DrawTexturePro(atlas01, particle_waterdrop,
                                               (Rectangle){ rainParticle.particle[i].position.x, rainParticle.particle[i].position.y, particle_waterdrop.width*rainParticle.particle[i].size, particle_waterdrop.height*rainParticle.particle[i].size },
                                               (Vector2){ particle_waterdrop.width*rainParticle.particle[i].size/2, particle_waterdrop.height*rainParticle.particle[i].size/2 }, rainParticle.particle[i].rotation,
                                               Fade(rainParticle.particle[i].color, rainParticle.particle[i].alpha));
    }

    for (int i = 0; i < 1024; i++)
    {
        if (rainStormParticle.particle[i].active) DrawTexturePro(atlas01, particle_waterdrop,
                                               (Rectangle){ rainStormParticle.particle[i].position.x, rainStormParticle.particle[i].position.y, particle_waterdrop.width*rainStormParticle.particle[i].size, particle_waterdrop.height*rainStormParticle.particle[i].size },
                                               (Vector2){ particle_waterdrop.width*rainStormParticle.particle[i].size/2, particle_waterdrop.height*rainStormParticle.particle[i].size/2 }, rainStormParticle.particle[i].rotation,
                                               Fade(rainStormParticle.particle[i].color, rainStormParticle.particle[i].alpha));
    }

    for (int i = 0; i < 256; i++)
    {
        if (snowStormParticle.particle[i].active) DrawTexturePro(atlas01, particle_icecrystal,
                                               (Rectangle){ snowStormParticle.particle[i].position.x, snowStormParticle.particle[i].position.y, particle_icecrystal.width*snowStormParticle.particle[i].size, particle_icecrystal.height*snowStormParticle.particle[i].size },
                                               (Vector2){ particle_icecrystal.width*snowStormParticle.particle[i].size/2, particle_icecrystal.height*snowStormParticle.particle[i].size/2 }, snowStormParticle.particle[i].rotation,
                                               Fade(snowStormParticle.particle[i].color, snowStormParticle.particle[i].alpha));
    }


    for (int i = 0; i < 20; i++)
    {
        if (rayparticle.particle[i].active) DrawTexturePro(atlas01, gameplay_fx_lightraymid,
                                               (Rectangle){ rayparticle.particle[i].position.x, rayparticle.particle[i].position.y, gameplay_fx_lightraymid.width*rayparticle.particle[i].size, gameplay_fx_lightraymid.height*rayparticle.particle[i].size },
                                               (Vector2){ gameplay_fx_lightraymid.width*rayparticle.particle[i].size/2, gameplay_fx_lightraymid.height*rayparticle.particle[i].size/2 }, rayparticle.particle[i].rotation,
                                               Fade(rayparticle.particle[i].color, rayparticle.particle[i].alpha));
    }
    
    /*
    DrawTexturePro(atlas01, title_twitter, (Rectangle){ GetScreenWidth()*0.85, GetScreenHeight()*0.1, title_twitter.width, title_twitter.height}, (Vector2){0,0}, 0, WHITE);
    DrawTexturePro(atlas01, title_facebook, (Rectangle){ GetScreenWidth()*0.85, GetScreenHeight()*0.3, title_facebook.width, title_facebook.height}, (Vector2){0,0}, 0, WHITE);
    DrawTexturePro(atlas01, title_googleplay, (Rectangle){ GetScreenWidth()*0.85, GetScreenHeight()*0.5, title_googleplay.width, title_googleplay.height}, (Vector2){0,0}, 0, WHITE);
    
    if (soundActive)DrawTexturePro(atlas01, title_music_on, (Rectangle){soundButton.x, soundButton.y, title_music_on.width, title_music_on.height}, (Vector2){0,0}, 0, WHITE);
    else DrawTexturePro(atlas01, title_music_off, (Rectangle){soundButton.x, soundButton.y, title_music_off.width, title_music_off.height}, (Vector2){0,0}, 0, WHITE);
    
    if (musicActive)DrawTexturePro(atlas01, title_speaker_on, (Rectangle){speakerButton.x, speakerButton.y, title_speaker_on.width, title_speaker_on.height}, (Vector2){0,0}, 0, WHITE);
    else DrawTexturePro(atlas01, title_speaker_off, (Rectangle){speakerButton.x, speakerButton.y, title_speaker_off.width, title_speaker_off.height}, (Vector2){0,0}, 0, WHITE);
    */
}

// Title Screen Unload logic
void UnloadTitleScreen(void)
{
    // ...
}

// Title Screen should finish?
int FinishTitleScreen(void)
{
    return finishScreen;
}

static void DrawParallaxFront(void)
{
    Rectangle ground01 = gameplay_back_ground01;

    //DrawTexturePro(atlas02, gameplay_back_tree01_layer03, (Rectangle){0, 21, gameplay_back_tree01_layer03.width*2, gameplay_back_tree01_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree01_layer01, (Rectangle){(int)parallaxFrontOffset, 60, gameplay_back_tree01_layer01.width*2, gameplay_back_tree01_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree02_layer01, (Rectangle){(int)parallaxFrontOffset + 140, 60, gameplay_back_tree02_layer01.width*2, gameplay_back_tree02_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree03_layer01, (Rectangle){(int)parallaxFrontOffset + 140*2, 55, gameplay_back_tree02_layer01.width*2, gameplay_back_tree02_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree04_layer01, (Rectangle){(int)parallaxFrontOffset + 140*3, 60, gameplay_back_tree04_layer01.width*2, gameplay_back_tree04_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree05_layer01, (Rectangle){(int)parallaxFrontOffset + 140*4, 60, gameplay_back_tree05_layer01.width*2, gameplay_back_tree05_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree06_layer01, (Rectangle){(int)parallaxFrontOffset + 140*5, 55, gameplay_back_tree06_layer01.width*2, gameplay_back_tree06_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree07_layer01, (Rectangle){(int)parallaxFrontOffset + 140*6, 60, gameplay_back_tree07_layer01.width*2, gameplay_back_tree07_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree08_layer01, (Rectangle){(int)parallaxFrontOffset + 140*7, 60, gameplay_back_tree08_layer01.width*2, gameplay_back_tree08_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_ground01, (Rectangle){0, 559, ground01.width*2, ground01.height*2}, (Vector2){0,0}, 0, color01);
    DrawTexturePro(atlas02, (Rectangle){ground01.x, ground01.y + ground01.height, ground01.width, -ground01.height}, (Rectangle){0, -33, ground01.width*2, ground01.height*2}, (Vector2){0,0}, 0, color01);
}

static void DrawParallaxMiddle(void)
{
    Rectangle ground02 = gameplay_back_ground02;

    //DrawTexturePro(atlas02, gameplay_back_tree02_layer03, (Rectangle){0, 67, gameplay_back_tree02_layer03.width*2, gameplay_back_tree02_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree01_layer02, (Rectangle){(int)0, 67, gameplay_back_tree01_layer02.width*2, gameplay_back_tree01_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree02_layer02, (Rectangle){(int)140, 67, gameplay_back_tree02_layer02.width*2, gameplay_back_tree02_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree03_layer02, (Rectangle){(int)140*2, 67, gameplay_back_tree03_layer02.width*2, gameplay_back_tree03_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree04_layer02, (Rectangle){(int)140*3, 67, gameplay_back_tree04_layer02.width*2, gameplay_back_tree04_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree05_layer02, (Rectangle){(int)140*4, 67, gameplay_back_tree05_layer02.width*2, gameplay_back_tree05_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree06_layer02, (Rectangle){(int)140*5, 67, gameplay_back_tree06_layer02.width*2, gameplay_back_tree06_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree07_layer02, (Rectangle){(int)140*6, 67, gameplay_back_tree07_layer02.width*2, gameplay_back_tree07_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree08_layer02, (Rectangle){(int)140*7, 67, gameplay_back_tree08_layer02.width*2, gameplay_back_tree08_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_ground02, (Rectangle){0, 509, ground02.width*2, ground02.height*2}, (Vector2){0,0}, 0, color01);  
    DrawTexturePro(atlas02, (Rectangle){ground02.x, ground02.y + ground02.height, ground02.width, -ground02.height}, (Rectangle){0, 19, ground02.width*2, ground02.height*2}, (Vector2){0,0}, 0, color01);
}

static void DrawParallaxBack(void)
{
    Rectangle ground03 = gameplay_back_ground03;

    //DrawTexturePro(atlas02, gameplay_back_tree02_layer03, (Rectangle){0, 67, gameplay_back_tree02_layer03.width*2, gameplay_back_tree02_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree01_layer03, (Rectangle){(int)parallaxBackOffset, 67, gameplay_back_tree01_layer03.width*2, gameplay_back_tree01_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree02_layer03, (Rectangle){(int)parallaxBackOffset + 140, 67, gameplay_back_tree02_layer03.width*2, gameplay_back_tree02_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree03_layer03, (Rectangle){(int)parallaxBackOffset + 140*2, 67, gameplay_back_tree03_layer03.width*2, gameplay_back_tree03_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree04_layer03, (Rectangle){(int)parallaxBackOffset + 140*3, 67, gameplay_back_tree04_layer03.width*2, gameplay_back_tree04_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree05_layer03, (Rectangle){(int)parallaxBackOffset + 140*4, 67, gameplay_back_tree05_layer03.width*2, gameplay_back_tree05_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree06_layer03, (Rectangle){(int)parallaxBackOffset + 140*5, 67, gameplay_back_tree06_layer03.width*2, gameplay_back_tree06_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree07_layer03, (Rectangle){(int)parallaxBackOffset + 140*6, 67, gameplay_back_tree07_layer03.width*2, gameplay_back_tree07_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree08_layer03, (Rectangle){(int)parallaxBackOffset + 140*7, 67, gameplay_back_tree08_layer03.width*2, gameplay_back_tree08_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_ground03, (Rectangle){0, 469, ground03.width*2, ground03.height*2}, (Vector2){0,0}, 0, color01);
    DrawTexturePro(atlas02, (Rectangle){ground03.x, ground03.y + ground03.height, ground03.width, -ground03.height}, (Rectangle){0, 67, ground03.width*2, ground03.height*2}, (Vector2){0,0}, 0, color01);
}

static float BounceEaseOut(float t,float b , float c, float d)
{
	if ((t/=d) < (1/2.75f)) {
		return c*(7.5625f*t*t) + b;
	} else if (t < (2/2.75f)) {
		float postFix = t-=(1.5f/2.75f);
		return c*(7.5625f*(postFix)*t + .75f) + b;
	} else if (t < (2.5/2.75)) {
			float postFix = t-=(2.25f/2.75f);
		return c*(7.5625f*(postFix)*t + .9375f) + b;
	} else {
		float postFix = t-=(2.625f/2.75f);
		return c*(7.5625f*(postFix)*t + .984375f) + b;
	}
}