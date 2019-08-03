/**********************************************************************************************
*
*   raylib - Koala Seasons game
*
*   Gameplay Screen Functions Definitions (Init, Update, Draw, Unload)
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

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#include "raylib.h"
#include "screens.h"

#include <time.h>
#include <stdio.h>
#include <math.h>

#include "atlas01.h"
#include "atlas02.h"

//#define DEBUG

#define MAX_ENEMIES            16
#define MAX_BAMBOO             16
#define MAX_LEAVES             14
#define MAX_FIRE               10
#define MAX_FIRE_FLAMES        20
#define MAX_ICE                10
#define MAX_RESIN              10
#define MAX_WIND               10
#define MAX_PARTICLES         128
#define MAX_PARTICLES_RAY       8
#define MAX_PARTICLES_SPEED    64
#define MAX_PARTICLES_STORM   512

#define SPEED                  3*TIME_FACTOR       // Speed of koala, trees, enemies, ...
#define JUMP                  15*TIME_FACTOR       // Jump speed
#define FLYINGMOV             10*TIME_FACTOR       // Up and Down speed when final form
#define GRAVITY                1*TIME_FACTOR       // Gravity when grabbed to tree
#define ICEGRAVITY             4*TIME_FACTOR       // Gravity when grabbed to an icy tree
#define KICKSPEED              3*TIME_FACTOR       // Gravity when kicking
#define SEASONCHANGE                    1200       // Frames duration per season
#define SEASONTRANSITION      SEASONCHANGE/6       // Season transition time

//SPAWNCHANCE - Chance of spawning things everytime a tree spawns
#define ICESPAWNCHANCE      30       // Chance of spawning ice everytime a tree spawns
#define RESINSPAWNCHANCE    30       // Chance of spawning resin everytime a tree spawns
#define FIRESPAWNCHANCE     30       // Chance of spawning fire everytime a tree spawns
#define WINDSPAWNCHANCE     30       // Chance of spawning wind everytime a tree spawns

//ENEMYSPAWNCHANCE - Chance of spawning enemies everytime a tree spawns
#define DINGOSPAWNCHANCE    30       // Chance of spawning dingos everytime a tree spawns
#define OWLSPAWNCHANCE      30       // Chance of spawning owls everytime a tree spawns
#define SNAKESPAWNCHANCE    30       // Chance of spawning snakes everytime a tree spawns
#define BEE_SPAWNCHANCE     10       // Chance of spawning bees everytime a tree spawns
#define EAGLE_SPAWNCHANCE    5       // Chance of spawning eagles everytime a tree spawns

#define EAGLE_TIME_DELAY   600       

//SCORE - Score increase everytime an enemy is killed
#define DINGOSCORE          100       // Score increase everytime a dingo is killed
#define OWLSCORE            100       // Score increase everytime an owl is killed
#define SNAKESCORE          100       // Score increase everytime a snake is killed
#define BEESCORE            300       // Score increase everytime a bee is killed
#define EAGLESCORE          300       // Score increase everytime an eagle is killed

#define LEAVESTOTRANSFORM             100     // Number of leaves recquired for the transformation
#define MAXTIMESPAWN                   85       // Maximum time for tree spawn
#define MINTIMESPAWN                   35       // Minimum time for tree spawn
#define STARTINGMONTH                   0       // Starting month (0 = January (summer))

#define PROGRESION_START               3600       // Time to start the progresion
#define PROGRESION_DURATION           12000       // Maximum time    
#define PROGRESOIN_MAX_SPAWNCHANCE       30       // Maximum spawn chance increase
#define PROGRESION_MAX_SPEED            0.5       // Maximum speed modification by progresion

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { WINTER, SPRING, SUMMER, FALL, TRANSITION } SeasonState;
typedef enum { JUMPING, KICK, FINALFORM, GRABED, ONWIND } KoalaState;

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
    Vector2 speed;
    float rotation;
    Vector2 size;
    Color color;
    float alpha;
    bool active;
} ParticleSpeed;

typedef struct {
    Vector2 position;
    Color color;
    float alpha;
    float size;
    float rotation;
    bool active;        // NOTE: Use it to activate/deactive particles
    bool fading;
    float delayCounter;
} ParticleRay;

typedef struct {
    Vector2 position;
    bool active;
    int spawnTime;
    int maxTime;
    Particle particles[MAX_PARTICLES];
} ParticleSystem;

// DONE: Rename for coherence: ParticleSystemStorm
typedef struct {
    Vector2 position;
    bool active;
    int spawnTime;
    int maxTime;
    Particle particles[MAX_PARTICLES_STORM];
} ParticleSystemStorm;

typedef struct {
    Vector2 position;
    bool active;
    float alpha;
    float scale;
    int score;
} PopUpScore;

typedef struct {
    Vector2 position;
    bool active;
    int spawnTime;
    int maxTime;
    ParticleSpeed particle[MAX_PARTICLES_SPEED];
} ParticleSystemSpeed;

typedef struct {
    Vector2 position;
    bool active;
    int spawnTime;
    int maxTime;
    ParticleRay particles[MAX_PARTICLES_RAY];
} ParticleSystemRay;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static float gravity;
static SeasonState season;
static KoalaState state;

// Gameplay screen global variables
static int framesCounter;
static int finishScreen;
static int grabCounter;
static int velocity;
static int speed;
static int bambooTimer;
static int bambooSpawnTime;
static int colorTimer;
static int jumpSpeed;
static int power;
static int maxPower;
static int transCount;
static int posArray[2];
static int posArrayDingo[2];
static int enemyVel[MAX_ENEMIES];
static int beeVelocity;
static int fireSpeed;
static int windCounter;
static int seasonTimer;
static int seasonChange;
static int resinCountjump;
static int resinCountdrag;
static int resinCount;
static int currentMonth;
static int monthTimer;
static int monthChange;
static int initMonth;
static int fireCounter[MAX_FIRE];
static int fireOffset;
static int beeMov;
static int killCounter;
static int leafType[MAX_LEAVES];
static int posArrayLeaf[3];
static int transAniCounter;
static int globalFrameCounter;
static int startCounter;
static int animCounter;
static int startNum = 3;
static int finalFormEnd;
static int randomMessage;
static int parallaxBackOffset;
static int parallaxFrontOffset;

// Animation Variables
static int thisFrame = 1;
static int currentFrame = 0;
static int thisFrameWind = 1;
static int currentFrameWind = 0;
static int thisFrameBee = 1;
static int thisFrameSnake = 1;
static int thisFrameDingo = 1;
static int thisFrameOwl = 1;
static int thisFrameEagle = 1;
static int curFrameEagle;
static int curFrameBee = 0;
static int curFrameSnake = 0;
static int curFrameDingo = 0;
static int curFrameOwl = 0;
static int curFrame;
static int curFrame1;
static int curFrame2;
static int curFrame3;
static int transitionFramesCounter;
static int thisFrameKoala;
static int curFrameKoala;
static int fogSpeed;
static int fogPosition;
static int progresionDelay;
static int progresionFramesCounter;
static int initLeaves;
static int eagleDelay;

// Stage data variables
static int jumpCounter;
static int resinCounter;
static int tornadoCounter;
static int dashCounter;
static int superKoalaCounter;

// Global data variables
static int snakeKillCounter;
static int dingoKillCounter;
static int owlKillCounter;
static int beeKillCounter;
static int eagleKillCounter;
static int globalKillCounter;
static int deathsCounter;

static float scrollFront;
static float scrollMiddle;
static float scrollBack;
static float scrollSpeed;
static float rightAlpha = 0.5;
static float leftAlpha = 0.5;
static float speedMod;
static float groundPos;
static float transRotation;
static float clockRotation;
static float clockSpeedRotation;
static float numberAlpha;
static float numberScale;
static float fogAlpha;
static float speedIncrease;
static float speedProgresion;
static float progresionSpawnChance;
static float UIfade;
static float filterAlpha;
static float leafGUIglowFade;
static float leafGUIpulseFade;
static float leafGUIpulseScale;
static float clockInitRotation;
static float clockFinalRotation;

// Game text strings
const char textFinalForm[32] = "THIS ISN'T EVEN MY FINAL FORM!";
const char textSpring1[32] = "FLOWER POWER!";
const char textSummer1[32] = "PREPARE FOR THE SUMMER!";
const char textFall1[32] = "HERE COMES THE FALL!";
const char textWinter1[32] = "WINTER IS COMING!";
const char textSpring2[32] = "POLLEN IS IN THE AIR";
const char textSummer2[32] = "HAPPY NEW YEAR!";
const char textFall2[32] = "IT'S RAINING RAIN";
const char textWinter2[32] = "LET IT SNOW!";

static bool snakeActive[MAX_ENEMIES];
static bool dingoActive[MAX_ENEMIES];
static bool owlActive[MAX_ENEMIES];
static bool branchActive[MAX_ENEMIES];
static bool bambooActive[MAX_BAMBOO];
static bool leafActive[MAX_LEAVES];
static bool fireActive[MAX_FIRE];
static bool iceActive[MAX_ICE];
static bool windActive[MAX_WIND];
static bool resinActive[MAX_RESIN];
static bool isHitSnake[MAX_ENEMIES];
static bool isHitDingo[MAX_ENEMIES];
static bool isHitOwl[MAX_ENEMIES];
static bool isHitBee;
static bool isHitEagle;
static bool onFire[MAX_FIRE];
static bool onIce;
static bool onResin;
static bool playerActive;
static bool play;
static bool transforming;
static bool onWind;
static bool glowing;
static bool beeActive;
static bool eagleActive;
static bool eagleAlert;
static bool alertActive;
static bool alertBeeActive;
static bool coolDown;
static bool leafSide[MAX_LEAVES];
static bool transBackAnim;
static bool fog;
static bool leafGUIglow;

static Rectangle player = {0, 0, 0, 0};
static Rectangle leftButton = {0, 0, 0, 0};
static Rectangle rightButton = {0, 0, 0, 0};
static Rectangle powerButton = {0, 0, 0, 0};
static Rectangle fire[MAX_FIRE];
static Rectangle ice[MAX_ICE];
static Rectangle resin[MAX_RESIN];
static Rectangle wind[MAX_WIND];
static Rectangle bamboo[MAX_BAMBOO];
static Rectangle snake[MAX_ENEMIES];
static Rectangle dingo[MAX_ENEMIES];
static Rectangle owl[MAX_ENEMIES];
static Rectangle leaf[MAX_LEAVES];
static Rectangle powerBar;
static Rectangle backBar;
static Rectangle fireAnimation;
static Rectangle windAnimation;
static Rectangle beeAnimation;
static Rectangle snakeAnimation;
static Rectangle dingoAnimation;
static Rectangle owlAnimation;
static Rectangle bee;
static Rectangle eagle;
static Rectangle eagleAnimation;
static Rectangle koalaAnimationIddle;
static Rectangle koalaAnimationJump;
static Rectangle koalaAnimationFly;
static Rectangle koalaAnimationTransform;
static Rectangle alertRectangle;
static Rectangle beeAlertRectangle;

static time_t rawtime;
static struct tm *ptm;

static Color finalColor;
static Color finalColor2;
static Color flyColor;
static Color counterColor;
static Color color00, color01, color02, color03;
static Color initcolor00, initcolor01, initcolor02, initcolor03;
static Color finalcolor00, finalcolor01, finalcolor02, finalcolor03;
static Vector2 zero;
static Vector2 firePos;
static Vector2 branchPos[MAX_ENEMIES];
static Vector2 textSize;
static Vector2 clockPosition;

static Particle enemyHit[MAX_ENEMIES];
static ParticleSystem leafParticles[MAX_LEAVES];
static ParticleSystem snowParticle;
static ParticleSystem backSnowParticle;
static ParticleSystem dandelionParticle;
static ParticleSystem dandelionBackParticle;
static ParticleSystem planetreeParticle;
static ParticleSystem backPlanetreeParticle;
static ParticleSystem flowerParticle;
static ParticleSystem backFlowerParticle;
static ParticleSystem rainParticle;
static ParticleSystem backRainParticle;
static ParticleSystemStorm rainStormParticle;
static ParticleSystemStorm snowStormParticle;
static ParticleSystemRay rayParticles;
static ParticleSystemRay backRayParticles;
static ParticleSystemSpeed speedFX;
static PopUpScore popupScore[MAX_ENEMIES];
static PopUpScore popupLeaves[MAX_LEAVES];
static PopUpScore popupBee;
static PopUpScore popupEagle;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void BambooSpawn(void);
static void SnakeSpawn(int chance);
static void DingoSpawn(int chance);
static void OwlSpawn(int chance);
static void BeeSpawn(int chance);
static void EagleSpawn(int chance);
static void FireSpawn(int chance);
static void IceSpawn(int chance);
static void ResinSpawn(int chance);
static void WindSpawn(int chance);
static void LeafSpawn(void);

static void DrawParallaxFront(void);
static void DrawParallaxMiddle(void);
static void DrawParallaxBack(void);
static float LinearEaseIn(float t, float b, float c, float d);

static Color ColorTransition(Color initialColor, Color finalColor, int framesCounter);
static bool CheckArrayValue(int *array, int arrayLength, int value);

static void Reset(void);

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitGameplayScreen(void)
{
    // Get current time at the moment of running game
    time(&rawtime);
    ptm = gmtime(&rawtime);

    currentMonth = ptm->tm_mon;
    initMonth = ptm->tm_mon;
    years = 1900 + ptm->tm_year;
    
    Reset();
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
    //if ((IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(0)) && playerActive) play = true;

    if (play == false && playerActive)
    {
        startCounter+= 1*TIME_FACTOR;

        if (startCounter > 20 && startCounter < 110)
        {
            animCounter+=1*TIME_FACTOR;
            numberAlpha -= 0.033f*TIME_FACTOR;
            numberScale -= 0.0825f*TIME_FACTOR;

            if (animCounter <= 30) startNum = 3;
            else if (animCounter > 30 && animCounter <= 60) startNum = 2;
            else startNum = 1;
        }
        else if (startCounter >= 110) play = true;

        if (numberAlpha <= 0) numberAlpha = 1;
        if (numberScale <= 0) numberScale = 2.5f;

        textSize = MeasureTextEx(font, FormatText("%01i", startNum), font.baseSize*numberScale, 2);
    }

    if (playerActive) finishScreen = 0;

    if (play && playerActive)
    {
        seasonTimer += speedMod*TIME_FACTOR;
        monthTimer += speedMod*TIME_FACTOR;
        eagleDelay += speedMod*TIME_FACTOR;
        
        globalFrameCounter++;

        if (monthTimer >= monthChange)
        {
            if (currentMonth == 10)
            {
                clockInitRotation = 225;
                clockFinalRotation = clockInitRotation + 90;
                rainChance = GetRandomValue(0, 100);
                clockSpeedRotation = 0;
                seasons++;
            }
            else if (currentMonth == 1)
            {
                clockInitRotation = 315;
                clockFinalRotation = clockInitRotation + 90;
                rainChance = GetRandomValue(0, 100);
                clockSpeedRotation = 0;
                seasons++;
            }
            else if (currentMonth == 4)
            {
                clockInitRotation = 45;
                clockFinalRotation = clockInitRotation + 90;
                rainChance = GetRandomValue(0, 100);
                clockSpeedRotation = 0;
                seasons++;
            }
            else if (currentMonth == 7) 
            {
                clockInitRotation = 135;
                clockFinalRotation = clockInitRotation + 90;
                rainChance = GetRandomValue(0, 100);
                clockSpeedRotation = 0;
                seasons++;
            }
            
            currentMonth++;
            monthTimer = 0;
            
            //EagleSpawn();
        }

        if (currentMonth > 11)
        {
            currentMonth = 0;
            years++;
        }

        if ((currentMonth == 11) || (currentMonth == 0) || ((currentMonth == 1) && (monthTimer <= SEASONTRANSITION)))
        {
            dandelionParticle.active = true;
            dandelionBackParticle.active = true;
            rayParticles.active = true;
            backRayParticles.active = true;

            transitionFramesCounter = 0;
            randomMessage = GetRandomValue(0, 10);
            
            fog = false;

            initcolor00 = color00;                                  // Summer Color
            initcolor01 = color01;
            initcolor02 = color02;
            initcolor03 = color03;
            finalcolor00 = (Color){242, 113, 62, 255};              // Fall Autum
            finalcolor01 = (Color){190, 135, 114, 255};
            finalcolor02 = (Color){144, 130, 101, 255};
            finalcolor03 = (Color){214, 133, 58, 255};
            season = SUMMER;
            currentSeason = 0;
        }
        else if ((currentMonth == 2) || (currentMonth == 3) || ((currentMonth == 4) && (monthTimer <= SEASONTRANSITION)))
        {
            if ((rainChance <= 40) && (rainChance > 24))
            {
                //rainParticle.active = true;
                rainStormParticle.active = true;
                backRainParticle.active = false;
                fog = true;
            }
            else if (rainChance <= 24)
            {
                //rainParticle.active = true;
                rainStormParticle.active = true;
                backRainParticle.active = false;
                fog = true;
            }
            else
            {
                planetreeParticle.active = true;
                backPlanetreeParticle.active = true;
                fog = false;
            }

            transitionFramesCounter = 0;
            randomMessage = GetRandomValue(0, 10);
            
            initcolor00 = color00;                                  // Fall Color
            initcolor01 = color01;
            initcolor02 = color02;
            initcolor03 = color03;
            finalcolor00 = (Color){130, 130, 181, 255};               // Winter Autum
            finalcolor01 = (Color){145, 145, 166, 255};
            finalcolor02 = (Color){104, 142, 144, 255};
            finalcolor03 = (Color){57, 140, 173, 255};

            season = FALL;
            currentSeason = 1;
        }
        else if ((currentMonth == 5) || (currentMonth == 6) || ((currentMonth == 7) && (monthTimer <= SEASONTRANSITION)))
        {
            if (rainChance <= 40)
            {
                //rainParticle.active = true;
                snowStormParticle.active = true;
                backSnowParticle.active = true;
                fog = true;
            }
            else
            {
                snowParticle.active = true;
                backSnowParticle.active = true;
                fog = false;
            }

            transitionFramesCounter = 0;
            randomMessage = GetRandomValue(0, 10);

            initcolor00 = color00;                                   // Winter Color
            initcolor01 = color01;
            initcolor02 = color02;
            initcolor03 = color03;
            finalcolor00 = (Color){196, 176, 49, 255};               // Spring Autum
            finalcolor01 = (Color){178, 163, 67, 255};
            finalcolor02 = (Color){133, 143, 90, 255};
            finalcolor03 = (Color){133, 156, 42, 255};

            season = WINTER;
            currentSeason = 2;
        }
        else if ((currentMonth == 8) || (currentMonth == 9) || ((currentMonth == 10) && (monthTimer <= SEASONTRANSITION)))
        {
            flowerParticle.active = true;
            backFlowerParticle.active = true;

            transitionFramesCounter = 0;
            randomMessage = GetRandomValue(0, 9);
            
            fog = false;

            initcolor00 = color00;                                  // Spring Color
            initcolor01 = color01;
            initcolor02 = color02;
            initcolor03 = color03;
            finalcolor00 = (Color){129, 172, 86, 255};               // Summer Autum
            finalcolor01 = (Color){145, 165, 125, 255};
            finalcolor02 = (Color){161, 130, 73, 255};
            finalcolor03 = (Color){198, 103, 51, 255};

            season = SPRING;
            currentSeason = 3;
        }
        else
        {
            flowerParticle.active = false;
            backFlowerParticle.active = false;
            snowParticle.active = false;
            backSnowParticle.active = false;
            planetreeParticle.active = false;
            backPlanetreeParticle.active = false;
            dandelionParticle.active = false;
            dandelionBackParticle.active = false;
            rainParticle.active = false;
            rainStormParticle.active = false;
            backRainParticle.active = false;
            rayParticles.active = false;
            backRayParticles.active = false;
            snowStormParticle.active = false;
            
            fog = false;

            transitionFramesCounter += speedMod*TIME_FACTOR;

            if (transitionFramesCounter <= SEASONTRANSITION)
            {
                color00 = ColorTransition(initcolor00, finalcolor00, transitionFramesCounter);
                color01 = ColorTransition(initcolor01, finalcolor01, transitionFramesCounter);
                color02 = ColorTransition(initcolor02, finalcolor02, transitionFramesCounter);
                color03 = ColorTransition(initcolor03, finalcolor03, transitionFramesCounter);
            }

            season = TRANSITION;
        }

        // Update scrolling values
        if (!transforming)
        {
            scrollFront -= scrollSpeed;
            scrollMiddle -= (scrollSpeed*0.75f);
            scrollBack -= scrollSpeed/2;
            
            fogPosition -= fogSpeed;
            
            groundPos -= speed;
            clockRotation += clockSpeedRotation;
        }

        player.y += gravity;
        bambooTimer += (speedMod*TIME_FACTOR);
        speed = SPEED*speedMod;
        
        if (player.x >= GetScreenWidth()*0.6 && state != FINALFORM)
        {
            speedIncrease = (player.x - GetScreenWidth()*0.6f)/GetScreenWidth();
        }
        else if (player.x < GetScreenWidth()*0.6 && state != FINALFORM)
        {
            speedIncrease = 0;
        }
        
        if (state != FINALFORM) speedMod = 1.2 + speedIncrease + speedProgresion;
        
        progresionDelay++;
        
        if (progresionDelay >= PROGRESION_START)
        {
            progresionFramesCounter++;
            
            if (progresionFramesCounter < PROGRESION_DURATION)
            {
                speedProgresion = LinearEaseIn((float)progresionFramesCounter, 0.0f, (float)PROGRESION_MAX_SPEED, (float)PROGRESION_DURATION);
                progresionSpawnChance = LinearEaseIn((float)progresionFramesCounter, 0.0f, (float)PROGRESOIN_MAX_SPAWNCHANCE, (float)PROGRESION_DURATION);
            }
        }

        if (scrollFront <= -GetScreenWidth()) scrollFront = 0;
        if (scrollMiddle <= -GetScreenWidth()) scrollMiddle = 0;
        if (scrollBack <= -GetScreenWidth()) scrollBack = 0;
        if (groundPos <= -GetScreenWidth()) groundPos = 0;
        if (fogPosition <= -GetScreenWidth()) fogPosition = 0;
        
        if (fogAlpha > 0 && !fog) fogAlpha -= 0.03f*speedMod;
        else if (fog && fogAlpha < 1) fogAlpha += 0.03f*speedMod;
        
        if (filterAlpha > 0 && !fog) filterAlpha -= 0.02f*speedMod;
        else if (fog && filterAlpha < 0.15f) filterAlpha += 0.02f*speedMod;
        
        //if (state != FINALFORM) clockSpeedRotation = (float)((90/(float)seasonChange)*speedMod)*1.75;
        clockSpeedRotation += speedMod*TIME_FACTOR;
        
        if (clockSpeedRotation <= (SEASONCHANGE)) clockRotation = (float)LinearEaseIn((float)clockSpeedRotation, clockInitRotation, 90.0f, (float)(SEASONCHANGE));
        else clockRotation = clockFinalRotation;

        if (CheckCollisionCircleRec(clockPosition, gameplay_gui_seasonsclock_disc.width, player))
        {
           if (UIfade > 0.4f) UIfade -= 0.01f*TIME_FACTOR;
        }
        else 
        {
            if (UIfade < 1) UIfade += 0.01f*TIME_FACTOR;
        }
        
        //----------------------------------------------------------------------------------
        // Animations
        //----------------------------------------------------------------------------------

        // Wind Animation
        thisFrameWind += 1*TIME_FACTOR;

        if (thisFrameWind >= 12)
        {
            currentFrameWind++;
            thisFrameWind = 0;
        }

        if (currentFrameWind > 3) currentFrameWind = 0;

        windAnimation.x = gameplay_props_whirlwind_spritesheet.x + windAnimation.width*currentFrameWind;

        // Fire Animation
        thisFrame += 1*TIME_FACTOR;

        if (thisFrame >= 8)
        {
            curFrame++;
            curFrame1++;
            curFrame2++;
            curFrame3++;

            thisFrame = 0;
        }

        if (curFrame > 3) curFrame = 0;
        if (curFrame1 > 3) curFrame1 = 0;
        if (curFrame2 > 3) curFrame2 = 0;
        if (curFrame3 > 3) curFrame3 = 0;

        if (!transforming)
        {
            // Eagle Animation
            curFrameEagle += 1*TIME_FACTOR;

            if (curFrameEagle >= 6*TIME_FACTOR)
            {
                thisFrameEagle ++;
                curFrameEagle = 0;
            }

            if (thisFrameEagle > 1) thisFrameEagle = 0;

            eagleAnimation.x = gameplay_enemy_eagle.x + eagleAnimation.width*thisFrameEagle;

            // Bee Animation
            curFrameBee += 1*TIME_FACTOR;

            if (curFrameBee >= 3*TIME_FACTOR)
            {
                thisFrameBee ++;
                curFrameBee = 0;
            }

            if (thisFrameBee > 3) thisFrameBee = 0;

            beeAnimation.x = gameplay_enemy_bee.x + beeAnimation.width*thisFrameBee;

            // Snake Animation
            thisFrameSnake += 1*TIME_FACTOR;

            if (thisFrameSnake >= 24*TIME_FACTOR)
            {
                curFrameSnake ++;
                thisFrameSnake = 0;
            }

            if (curFrameSnake > 1) curFrameSnake = 0;

            snakeAnimation.x = gameplay_enemy_snake.x + snakeAnimation.width*curFrameSnake;

            // Dingo Animation
            curFrameDingo += 1*TIME_FACTOR;

            if (curFrameDingo >= 24*TIME_FACTOR)
            {
                thisFrameDingo ++;
                curFrameDingo = 0;
            }

            if (thisFrameDingo > 1) thisFrameDingo = 0;

            dingoAnimation.x = gameplay_enemy_dingo.x + dingoAnimation.width*thisFrameDingo;

            // Owl Animation
            curFrameOwl += 1*TIME_FACTOR;

            if (curFrameOwl >= 24*TIME_FACTOR)
            {
                thisFrameOwl ++;
                curFrameOwl = 0;
            }

            if (thisFrameOwl > 1) thisFrameOwl = 0;

            owlAnimation.x = gameplay_enemy_owl.x + owlAnimation.width*thisFrameOwl;

            // Alert Animation
            if (alertActive)
            {
                if (eagleAlert)
                {
                    alertRectangle.x -= 100*TIME_FACTOR;
                    alertRectangle.width += 100*TIME_FACTOR;
                    alertRectangle.height += 5*TIME_FACTOR;
                    alertRectangle.y -= 5*TIME_FACTOR;
                    
                    if (alertRectangle.height >= 100) eagleAlert = false;
                }
                else
                {
                    alertRectangle.height -= 1*TIME_FACTOR;
                    alertRectangle.y += 1*TIME_FACTOR;
                    
                    if (alertRectangle.height <= 0)
                    {
                        eagleAlert = true;
                        eagleActive = true;
                        alertActive = false;
                    }
                }
            }

            // Eagle Logic
            if (eagleActive == true && !isHitEagle)
            {
                eagle.x -= 10*speed*TIME_FACTOR;

                if (CheckCollisionRecs(eagle, player) && (state != FINALFORM) && (state != KICK))
                {
                    velocity = 8;
                    jumpSpeed = 2;
                    play = false;
                    playerActive = false;
                    killer = 6;
                }
                else if (CheckCollisionRecs(eagle, player) && (state == FINALFORM) && (state != KICK))
                {
                    isHitEagle = true;
                    beeVelocity = 8;
                    killHistory[killCounter] = 5;
                    killCounter++;
                    
                    score += EAGLESCORE;
                    eagleKillCounter++;
                    globalKillCounter++;
                    
                    popupEagle.position = (Vector2){ eagle.x, eagle.y };
                    popupEagle.scale = 1.0f;
                    popupEagle.alpha = 1.0f;
                    popupEagle.score = EAGLESCORE; 
                    popupEagle.active = true; 
                }
            }
            else if (isHitEagle)
            {
                if ((eagle.y + eagle.height) > GetScreenHeight())
                {
                    eagleActive = false;
                    isHitEagle = false;
                }

                eagle.x += 2*TIME_FACTOR;
                beeVelocity -= 1*TIME_FACTOR*TIME_FACTOR;
                eagle.y -= beeVelocity*TIME_FACTOR;
            }
            
            if (eagle.x + eagle.width <= 0) eagleActive = false;
            
            // Bee Alert Animation
            if (alertBeeActive)
            {
                
                beeAlertRectangle.x -= 100*TIME_FACTOR;
                beeAlertRectangle.width += 100*TIME_FACTOR;
                beeAlertRectangle.height += 2.5*TIME_FACTOR;
                beeAlertRectangle.y += 1.25*TIME_FACTOR;
                
                if (beeAlertRectangle.height >= 100)
                {
                    beeActive = true;
                    alertBeeActive = false;
                }
            }

            // Bee Logic
            if (beeActive == true && !isHitBee)
            {
                bee.x -= 3*speed;
                beeMov = sin(2*PI/400*bee.x) * 5;
                bee.y += beeMov*TIME_FACTOR;

                if (CheckCollisionRecs(bee, player) && (state != FINALFORM) && (state != KICK))
                {
                    velocity = 8;
                    jumpSpeed = 2;
                    play = false;
                    playerActive = false;
                    killer = 5;
                }
                else if (CheckCollisionRecs(bee, player) && (state == FINALFORM))
                {
                    isHitBee = true;
                    beeVelocity = 8;
                    killHistory[killCounter] = 4;
                    killCounter++;
                    
                    score += BEESCORE;
                    beeKillCounter++;
                    globalKillCounter++;
                    
                    popupBee.position = (Vector2){ bee.x, bee.y };
                    popupBee.scale = 1.0f;
                    popupBee.alpha = 1.0f;
                    popupBee.score = BEESCORE; 
                    popupBee.active = true; 
                }
            }
            else if (isHitBee)
            {
                if ((bee.y + bee.height) > GetScreenHeight())
                {
                    beeActive = false;
                    isHitBee = false;
                }

                bee.x += 2*TIME_FACTOR;
                beeVelocity -= 1*TIME_FACTOR*TIME_FACTOR;
                bee.y -= beeVelocity*TIME_FACTOR;
            }
            
            if (bee.x + bee.width <= 0) beeActive = false;
        }

        // Power bar logic
        powerBar.width = power;

        if (power >= maxPower) power = maxPower;

        if (currentLeaves >= LEAVESTOTRANSFORM && !coolDown)
        {
            flyColor = ORANGE;
            
            if (leafGUIglow)
            {
                leafGUIglowFade += 0.01f*TIME_FACTOR;
                if (leafGUIglowFade >= 1) leafGUIglow = false;
            }    
            else 
            {
                leafGUIglowFade -= 0.01f*TIME_FACTOR;
                if (leafGUIglowFade <= 0) leafGUIglow = true;
            }
            
            leafGUIpulseFade -= 0.01f*TIME_FACTOR;
            leafGUIpulseScale += 0.005F*TIME_FACTOR;
            
            if (leafGUIpulseFade <= 0) 
            {    
                leafGUIpulseFade = 1;
                leafGUIpulseScale = 1;
            }
                
#if (defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB))
            if ((IsGestureDetected(GESTURE_TAP) && CheckCollisionPointRec(GetTouchPosition(0), powerButton)) && (state != FINALFORM))
            {
                state = FINALFORM;
                transforming = true;
                //currentLeaves -= LEAVESTOTRANSFORM;
                initLeaves = currentLeaves;
                curFrameKoala = 0;
                thisFrameKoala = 0;
                superKoalaCounter++;
            }
#elif (defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB))

            if ((IsKeyPressed(KEY_ENTER) || (CheckCollisionPointRec(GetMousePosition(), powerButton) && IsMouseButtonPressed(0))) && (state != FINALFORM))
            {
                state = FINALFORM;
                transforming = true;
                //currentLeaves -= LEAVESTOTRANSFORM;
                initLeaves = currentLeaves;
                curFrameKoala = 0;
                thisFrameKoala = 0;
                superKoalaCounter++;
            }          
#endif
        }
#if defined(DEBUG)
        if ((currentLeaves < LEAVESTOTRANSFORM) && (IsKeyPressed(KEY_ENTER))) currentLeaves += LEAVESTOTRANSFORM;
#endif
        if (coolDown)
        {
            power += 20;
            
            if (power >= maxPower) coolDown = false;
        }

        colorTimer += 1*TIME_FACTOR;

        if (colorTimer > 10)
        {
            finalColor.r = GetRandomValue(0, 255);
            finalColor.g = GetRandomValue(0, 255);
            finalColor.b = GetRandomValue(0, 255);
            colorTimer = 0;
        }

        // Ice logic
        for (int i = 0; i < MAX_ICE; i++)
        {
            if (!iceActive[i]) ice[i].x = -100;

            if (ice[i].x <= - ice[i].width) iceActive[i] = false;

            if (CheckCollisionRecs(ice[i], player) && (state == GRABED)) onIce = true;
        }

        // Resin logic
        for (int i = 0; i < MAX_RESIN; i++)
        {
            if (!resinActive[i]) resin[i].x = -100;

            if (resin[i].x <= -resin[i].width) resinActive[i] = false;

            if (CheckCollisionRecs(resin[i], player) && resinCount >= 30*TIME_FACTOR && state != FINALFORM)
            {
                if (!onResin)
                {
                    PlaySound(fxHitResin);
                    resinCounter++;
                }

                onResin = true;
                grabCounter = 10;
                
                //gravity = 0;
                state = GRABED;
            }
        }

        // Wind logic
        for (int i = 0; i < MAX_WIND; i++)
        {
            if (!windActive[i]) wind[i].x = -500;

            else wind[i].x -= 9*speedMod*TIME_FACTOR;

            if (wind[i].x <= - wind[i].width) windActive[i] = false;

            if (CheckCollisionRecs(wind[i], player) && state != ONWIND && (windCounter >= 35) && state != FINALFORM)
            {
                state = ONWIND;
                windCounter = 0;
                velocity = JUMP;
                grabCounter = 0;
                jumpSpeed = 10;
                rightAlpha = 1;
                onIce = false;
                onResin = false;
                resinCountjump = 0;
                resinCountdrag = 0;
                tornadoCounter++;
            }
        }

        // Fire logic
        for (int i = 0; i < MAX_FIRE; i++)
        {
            //fire[i].x -= speed;

            if (fireActive[i] == false) fire[i].x = -200;

            if (fire[i].x <= (player.x + player.width) && !onFire[i])
            {
                onFire[i] = true;
            }

            if (onFire[i] && fire[i].y > -50 && !transforming)
            {
                        fireCounter[i]++;
                        fire[i].y -= fireSpeed*TIME_FACTOR;
                        fireCounter[i] = 0;
                        //fire[i].height += fireSpeed;
            }

            if (fire[i].x <= -fire[i].width)
            {
                fireActive[i] = false;
                onFire[i] = false;
            }

            if (CheckCollisionRecs(player, fire[i]))
            {
                if (state != FINALFORM)
                {
                    velocity = 8;
                    jumpSpeed = 2;
                    play = false;
                    playerActive = false;
                    killer = 0;
                }
            }

            for (int k = 0; k < MAX_ENEMIES; k++)
            {
                if (CheckCollisionRecs(fire[i], snake[k]) && !isHitSnake[k])
                {
                    isHitSnake[k] = true;
                    enemyVel[k] = 8;
                }
            }
        }

        // Bamboo logic
        for (int i = 0; i < MAX_BAMBOO; i++)
        {
            if (bambooActive[i])
            {
                bamboo[i].x -= speed;

                if ((CheckCollisionRecs(player, bamboo[i])) && (state != FINALFORM))
                {
                    if (grabCounter >= 10)
                    {
                        player.x = bamboo[i].x - 25;
                        state = GRABED;
                    }
                }

                if ((CheckCollisionRecs(player, bamboo[i])) && (state == FINALFORM))
                {
                    if (power <= 1)
                    {
                        state = JUMPING;
                        velocity = JUMP;
                        jumpSpeed = 6;
                        scrollSpeed = 1.6f;
                        speedMod = 1.2f;
                        coolDown = true;
                        flyColor = GRAY;
                        speedFX.active = false;
                    }
                }

                for (int k = 0; k < MAX_ENEMIES; k++)
                {
                    if ((CheckCollisionRecs(snake[k], bamboo[i])) && snakeActive[k] && !isHitSnake[k]) snake[k].x = bamboo[i].x - 15;
                    if ((CheckCollisionRecs(dingo[k], bamboo[i])) && dingoActive[k] && !isHitDingo[k]) dingo[k].x = bamboo[i].x - 15;
                    if ((CheckCollisionRecs(owl[k], bamboo[i])) && owlActive[k] && !isHitOwl[k]) owl[k].x = bamboo[i].x - 22;
                    if ((CheckCollisionRecs((Rectangle){branchPos[k].x, branchPos[k].y, gameplay_props_owl_branch.width, gameplay_props_owl_branch.height}, bamboo[i])) && branchActive[k]) branchPos[k].x = bamboo[i].x -24;
                }

                for (int j = 0; j < MAX_LEAVES; j++)
                {
                    if ((CheckCollisionRecs(leaf[j], bamboo[i])) && leafActive[j])
                    {
                        if (leafSide[j])leaf[j].x = bamboo[i].x + 18;
                        else leaf[j].x = bamboo[i].x - 18;
                    }

                    //else leaf[j].x -= speed;
                }

                for (int z = 0; z < MAX_FIRE; z++)
                {
                    if ((CheckCollisionRecs(fire[z], bamboo[i])) && fireActive[z]) fire[z].x = bamboo[i].x - 5;
                    //else fire[z].x -= speed;
                }

                for (int n = 0; n < MAX_ICE; n++)
                {
                    if ((CheckCollisionRecs(ice[n], bamboo[i])) && iceActive[n]) ice[n].x = bamboo[i].x;
                }

                for (int m = 0; m < MAX_RESIN; m++)
                {
                    if ((CheckCollisionRecs(resin[m], bamboo[i])) && resinActive[m]) resin[m].x = bamboo[i].x;
                }

                if (bamboo[i].x <= -(bamboo[i].width + 30))
                {
                    bambooActive[i] = false;
                }
            }
        }

        // Enemy logic
        for (int k = 0; k < MAX_ENEMIES; k++)
        {
            //if (snakeActive[k] && !isHitSnake[k])snake[k].x -= speed;

            if (snake[k].x <= -snake[k].width) snakeActive[k] = false;
            if (dingo[k].x <= -dingo[k].width) dingoActive[k] = false;
            if (owl[k].x <= -owl[k].width) owlActive[k] = false;
            if (branchPos[k].x <= -owl[k].width) branchActive[k] = false;

            if (!snakeActive[k]) snake[k].x = -500;
            if (!dingoActive[k]) dingo[k].x = -500;
            if (!owlActive[k]) owl[k].x = -500;

            if (CheckCollisionRecs(player, snake[k]) && (state != KICK) && !isHitSnake[k])
            {
                if (state != FINALFORM)
                {
                    velocity = 8;
                    jumpSpeed = 2;
                    enemyVel[k] = 8;
                    play = false;
                    playerActive = false;
                    killer = 1;
                }
                else if (state == FINALFORM)
                {
                    isHitSnake[k] = true;
                    enemyVel[k] = 8;
                    killHistory[killCounter] = 1;
                    killCounter++;
                    
                    snakeKillCounter++;
                    globalKillCounter++;
                    score += SNAKESCORE;
                    
                    PlaySound(fxDieSnake);

                    enemyHit[k].position = (Vector2){ snake[k].x, snake[k].y };
                    enemyHit[k].speed = (Vector2){ 0, 0 };
                    enemyHit[k].size = (float)GetRandomValue(0, 10)/30;
                    enemyHit[k].rotation = 0.0f;
                    //enemyHit[i].color = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
                    enemyHit[k].alpha = 1.0f;
                    enemyHit[k].active = true;
                    
                    popupScore[k].position = (Vector2){ snake[k].x,snake[k].y };
                    popupScore[k].scale = 1.0f;
                    popupScore[k].alpha = 1.0f;
                    popupScore[k].score = SNAKESCORE; 
                    popupScore[k].active = true; 
                }
            }

            if (CheckCollisionRecs(player, dingo[k]) && (state != KICK) && !isHitDingo[k])
            {
                if (state != FINALFORM)
                {
                    velocity = 8;
                    jumpSpeed = 2;
                    play = false;
                    enemyVel[k] = 8;
                    playerActive = false;
                    killer = 2;
                }
                else if (state == FINALFORM)
                {
                    isHitDingo[k] = true;
                    enemyVel[k] = 8;
                    killHistory[killCounter] = 2;
                    killCounter++;
                    
                    score += DINGOSCORE;
                    dingoKillCounter++;
                    globalKillCounter++;

                    enemyHit[k].position = (Vector2){ dingo[k].x,dingo[k].y };
                    enemyHit[k].speed = (Vector2){ 0, 0 };
                    enemyHit[k].size = (float)GetRandomValue(5, 10)/30;
                    enemyHit[k].rotation = 0.0f;
                    //enemyHit[i].color = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
                    enemyHit[k].alpha = 1.0f;
                    enemyHit[k].active = true;

                    popupScore[k].position = (Vector2){ dingo[k].x,dingo[k].y };
                    popupScore[k].scale = 1.0f;
                    popupScore[k].alpha = 1.0f;
                    popupScore[k].score = DINGOSCORE;
                    popupScore[k].active = true;
                }              
            }

            if (CheckCollisionRecs(player, owl[k]) && (state != KICK) && !isHitOwl[k])
            {
                if (state != FINALFORM)
                {
                    velocity = 8;
                    enemyVel[k] = 8;
                    jumpSpeed = 2;
                    play = false;
                    playerActive = false;
                    killer = 3;
                }
                else if (state == FINALFORM)
                {
                    isHitOwl[k] = true;
                    enemyVel[k] = 8;
                    killHistory[killCounter] = 3;
                    killCounter++;
                    
                    score += OWLSCORE;
                    owlKillCounter++;
                    globalKillCounter++;

                    enemyHit[k].position = (Vector2){ owl[k].x, owl[k].y };
                    enemyHit[k].speed = (Vector2){ owl[k].x, owl[k].y };
                    enemyHit[k].size = (float)GetRandomValue(5, 10)/30;
                    enemyHit[k].rotation = 0.0f;
                    //enemyHit[i].color = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
                    enemyHit[k].alpha = 1.0f;
                    enemyHit[k].active = true;

                    popupScore[k].position = (Vector2){ owl[k].x,owl[k].y };
                    popupScore[k].scale = 1.0f;
                    popupScore[k].alpha = 1.0f;
                    popupScore[k].score = OWLSCORE;
                    popupScore[k].active = true; 
                }
            }

            if (isHitSnake[k])
            {
                if ((snake[k].y + snake[k].height) > GetScreenHeight())
                {
                    snakeActive[k] = false;
                    isHitSnake[k] = false;
                }

                snake[k].x += 2*TIME_FACTOR;
                enemyVel[k] -= 1*TIME_FACTOR*TIME_FACTOR;
                snake[k].y -= enemyVel[k]*TIME_FACTOR;
            }

            if (isHitDingo[k])
            {
                if ((dingo[k].y) > GetScreenHeight())
                {
                    dingoActive[k] = false;
                    isHitDingo[k] = false;
                }

                dingo[k].x += 2*TIME_FACTOR;
                enemyVel[k] -= 1*TIME_FACTOR*TIME_FACTOR;
                dingo[k].y -= enemyVel[k]*TIME_FACTOR;
            }

            if (isHitOwl[k])
            {
                if ((owl[k].y) > GetScreenHeight())
                {
                    owlActive[k] = false;
                    isHitOwl[k] = false;
                }

                owl[k].x += 2*TIME_FACTOR;
                enemyVel[k] -= 1*TIME_FACTOR*TIME_FACTOR;
                owl[k].y -= enemyVel[k];
            }
        }

        // Leaves logic
        for (int j = 0; j < MAX_LEAVES; j++)
        {
            //leaf[j].x -= speed;

            leafParticles[j].position = (Vector2){ leaf[j].x, leaf[j].y};
            
           
            if (leaf[j].x <= -leaf[j].width) leafActive[j] = false;

            if (CheckCollisionRecs(player, leaf[j]) && leafActive[j])
            {
                popupLeaves[j].position = (Vector2){ leaf[j].x, leaf[j].y };
                popupLeaves[j].scale = 1.0f;
                popupLeaves[j].alpha = 1.0f;
                popupLeaves[j].active = true; 
                
                PlaySound(fxEatLeaves);

                if (leafType[j] == 0)
                {
                    currentLeaves++;
                    popupLeaves[j].score = 1;
                }

                else if (leafType[j] == 1)
                {
                    currentLeaves += 2;
                    popupLeaves[j].score = 2;
                }

                else if (leafType[j] == 2)
                {
                    currentLeaves += 3;
                    popupLeaves[j].score = 3;
                }

                else if (leafType[j] == 3)
                {
                    currentLeaves += 4;
                    popupLeaves[j].score = 4;
                }

                leafActive[j] = false;
                leafParticles[j].active = true;

                for (int h = 0; h < 32; h++)
                {
                    leafParticles[j].particles[h].active = true;
                    leafParticles[j].particles[h].position = (Vector2){ leafParticles[j].position.x, leafParticles[j].position.y};
                    leafParticles[j].particles[h].speed = (Vector2){ (float)GetRandomValue(-400, 400)/100, (float)GetRandomValue(-400, 400)/100 };
                    leafParticles[j].particles[h].size = (float)GetRandomValue(4, 8)/10;
                    leafParticles[j].particles[h].rotation = GetRandomValue(-180, 180);
                    //enemyHit[i].color = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
                    leafParticles[j].particles[h].alpha = 1.0f;
                }
            }
        }

        //----------------------------------------------------------------------------------
        // Particles Logic
        //----------------------------------------------------------------------------------
        
        // Leaf particles logic
        for (int i = 0; i < MAX_LEAVES; i++)
        {
            
            if (leafParticles[i].active == true)
            {

                for (int j = 0; j < 32; j++)
                {
                    //enemyHit[i].speed.y -= 1;

                    leafParticles[i].particles[j].position.x +=  leafParticles[i].particles[j].speed.x*TIME_FACTOR;
                    leafParticles[i].particles[j].position.y +=  leafParticles[i].particles[j].speed.y;

                    //if (((enemyHitPosition.x + enemyHit[i].position.x + enemyHit[i].size) >= screenWidth) || ((enemyHitPosition.x + enemyHit[i].position.x - enemyHit[i].size) <= 0)) enemyHit[i].speed.x *= -1;
                    //if (((enemyHitPosition.y + enemyHit[i].position.y + enemyHit[i].size) >= screenHeight) || ((enemyHitPosition.y + enemyHit[i].position.y - enemyHit[i].size) <= 0)) enemyHit[i].speed.y *= -1;

                    leafParticles[i].particles[j].rotation += 6*TIME_FACTOR;
                    leafParticles[i].particles[j].alpha -= 0.03f*TIME_FACTOR;
                    leafParticles[i].particles[j].size -= 0.004*TIME_FACTOR;

                    if (leafParticles[i].particles[j].size <= 0) leafParticles[i].particles[j].size = 0.0f;

                    if (leafParticles[i].particles[j].alpha <= 0)
                    {
                        leafParticles[i].particles[j].alpha = 0.0f;
                        leafParticles[i].particles[j].active = false;
                        leafParticles[i].active = false;
                    }
                }
            }
            
            if (popupLeaves[i].active)
            {
                //mouseTail[i].position.y += gravity;
                popupLeaves[i].alpha -= 0.02f;
                popupLeaves[i].scale += 0.1f;
                popupLeaves[i].position.y -= 3.0f;
                popupLeaves[i].position.x -= speed;

                if (popupLeaves[i].alpha <= 0.0f) popupLeaves[i].active = false;
            }
        }

        // Enemy Particles
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (enemyHit[i].active)
            {
                //mouseTail[i].position.y += gravity;
                enemyHit[i].alpha -= 0.1f*TIME_FACTOR;
                enemyHit[i].size += 0.1f*TIME_FACTOR;

                if (enemyHit[i].alpha <= 0.0f) enemyHit[i].active = false;
            }
            
            if (popupScore[i].active)
            {
                //mouseTail[i].position.y += gravity;
                popupScore[i].alpha -= 0.02f;
                popupScore[i].scale += 0.2f;
                popupScore[i].position.y -= 4.0f;
                popupScore[i].position.x -= speed;

                if (popupScore[i].alpha <= 0.0f) popupScore[i].active = false;
            }
        }
        
        if (popupBee.active)
        {
            //mouseTail[i].position.y += gravity;
            popupBee.alpha -= 0.02f;
            popupBee.scale += 0.2f;
            popupBee.position.y -= 4.0f;
            popupBee.position.x -= speed;

            if (popupBee.alpha <= 0.0f) popupBee.active = false;
        }
        
        if (popupEagle.active)
        {
            //mouseTail[i].position.y += gravity;
            popupEagle.alpha -= 0.02f;
            popupEagle.scale += 0.2f;
            popupEagle.position.y -= 4.0f;
            popupEagle.position.x -= speed;

            if (popupEagle.alpha <= 0.0f) popupEagle.active = false;
        }

        if (state != FINALFORM)
        {
            // Snow Particle
            if (snowParticle.active)
            {
                snowParticle.spawnTime += 1*TIME_FACTOR;

                for (int i = 0; i < MAX_PARTICLES; i++)
                {
                    if (!snowParticle.particles[i].active && (snowParticle.spawnTime >= snowParticle.maxTime))
                    {
                        snowParticle.particles[i].active = true;
                        snowParticle.particles[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth() + 200), -10 };
                        snowParticle.spawnTime = 0;
                        snowParticle.maxTime = GetRandomValue(5, 20);
                    }
                }
            }

            if (backSnowParticle.active)
            {
                snowParticle.spawnTime += 1*TIME_FACTOR;

                for (int i = 0; i < MAX_PARTICLES; i++)
                {
                    if (!backSnowParticle.particles[i].active && (backSnowParticle.spawnTime >= backSnowParticle.maxTime))
                    {
                        backSnowParticle.particles[i].active = true;
                        backSnowParticle.particles[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth() + 200), -10 };
                        backSnowParticle.spawnTime = 0;
                        backSnowParticle.maxTime = GetRandomValue(3, 10);
                    }
                }
            }
            
            // Autumn leaves particles
            if (planetreeParticle.active)
            {
                planetreeParticle.spawnTime += 1*TIME_FACTOR;
                backPlanetreeParticle.spawnTime += 1*TIME_FACTOR;

                for (int i = 0; i < MAX_PARTICLES; i++)
                {
                    if (!planetreeParticle.particles[i].active && (planetreeParticle.spawnTime >= planetreeParticle.maxTime))
                    {
                        planetreeParticle.particles[i].active = true;
                        planetreeParticle.particles[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth() + 200), -10 };
                        planetreeParticle.spawnTime = 0;
                        planetreeParticle.maxTime = GetRandomValue(5, 20);
                    }

                    if (!backPlanetreeParticle.particles[i].active && (backPlanetreeParticle.spawnTime >= backPlanetreeParticle.maxTime))
                    {
                        backPlanetreeParticle.particles[i].active = true;
                        backPlanetreeParticle.particles[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth() + 200), -10 };
                        backPlanetreeParticle.spawnTime = 0;
                        backPlanetreeParticle.maxTime = GetRandomValue(3, 10);
                    }
                }
            }

            // Dandelion particle
            if (dandelionParticle.active)
            {
                dandelionParticle.spawnTime += 1*TIME_FACTOR;
                dandelionBackParticle.spawnTime += 1*TIME_FACTOR;

                for (int i = 0; i < MAX_PARTICLES; i++)
                {
                    if (!dandelionParticle.particles[i].active && (dandelionParticle.spawnTime >= dandelionParticle.maxTime))
                    {
                        dandelionParticle.particles[i].active = true;
                        dandelionParticle.particles[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth() + 200), -10 };
                        dandelionParticle.spawnTime = 0;
                        dandelionParticle.maxTime = GetRandomValue(5, 20);
                    }

                    if (!dandelionBackParticle.particles[i].active && (dandelionBackParticle.spawnTime >= dandelionBackParticle.maxTime))
                    {
                        dandelionBackParticle.particles[i].active = true;
                        dandelionBackParticle.particles[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth() + 200), -10 };
                        dandelionBackParticle.spawnTime = 0;
                        dandelionBackParticle.maxTime = GetRandomValue(3, 10);
                    }
                }
            }

            // Flower Particle
            if (flowerParticle.active)
            {

                flowerParticle.spawnTime += 1*TIME_FACTOR;
                backFlowerParticle.spawnTime += 1*TIME_FACTOR;

                for (int i = 0; i < MAX_PARTICLES; i++)
                {
                    if (!flowerParticle.particles[i].active && (flowerParticle.spawnTime >= flowerParticle.maxTime))
                    {
                        flowerParticle.particles[i].active = true;
                        flowerParticle.particles[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth() + 200), -10 };
                        flowerParticle.spawnTime = 0;
                        flowerParticle.maxTime = GetRandomValue(5, 20);
                    }

                    if (!backFlowerParticle.particles[i].active && (backFlowerParticle.spawnTime >= backFlowerParticle.maxTime))
                    {
                        backFlowerParticle.particles[i].active = true;
                        backFlowerParticle.particles[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth() + 200), -10 };
                        backFlowerParticle.spawnTime = 0;
                        backFlowerParticle.maxTime = GetRandomValue(3, 10);
                    }
                }
            }

            // Rain Particle
            if (rainParticle.active)
            {
                rainParticle.spawnTime += 1*TIME_FACTOR;

                for (int i = 0; i < MAX_PARTICLES; i++)
                {
                    if (!rainParticle.particles[i].active && (rainParticle.spawnTime >= rainParticle.maxTime))
                    {
                        rainParticle.particles[i].active = true;
                        rainParticle.particles[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth() + 200), -10 };
                        rainParticle.spawnTime = 0;
                        rainParticle.maxTime = GetRandomValue(1, 8);
                    }
                }
            }

            // BackRain Particles
            if (backRainParticle.active)
            {
                backRainParticle.spawnTime += 1*TIME_FACTOR;

                for (int i = 0; i < MAX_PARTICLES; i++)
                {
                    if (!backRainParticle.particles[i].active && (backRainParticle.spawnTime >= backRainParticle.maxTime))
                    {
                        backRainParticle.particles[i].active = true;
                        backRainParticle.particles[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth() + 200), -10 };
                        backRainParticle.spawnTime = 0;
                        backRainParticle.maxTime = GetRandomValue(1, 8);
                    }
                }
            }

            // Storm Particles
            if (rainStormParticle.active)
            {
                rainStormParticle.spawnTime += 1*TIME_FACTOR;

                for (int i = 0; i < MAX_PARTICLES_STORM; i++)
                {
                    if (!rainStormParticle.particles[i].active && (rainStormParticle.spawnTime >= rainStormParticle.maxTime))
                    {
                        for (int j = 0; j < 16; j++)
                        {
                            rainStormParticle.particles[i+j].active = true;
                            rainStormParticle.particles[i+j].position = (Vector2){ GetRandomValue(100, GetScreenWidth() + 1000), GetRandomValue(-10,-20) };
                        }
                        
                        rainStormParticle.spawnTime = 0;
                        rainStormParticle.maxTime = 4;
                    }
                }
            }

            // Snow Storm Particles
            if (snowStormParticle.active)
            {
                snowStormParticle.spawnTime += 1*TIME_FACTOR;

                for (int i = 0; i < MAX_PARTICLES_STORM; i++)
                {
                    if (!snowStormParticle.particles[i].active && (snowStormParticle.spawnTime >= snowStormParticle.maxTime))
                    {
                        snowStormParticle.particles[i].active = true;
                        snowStormParticle.particles[i].position = (Vector2){ GetRandomValue(100, GetScreenWidth() + 800), -10 };
                        snowStormParticle.spawnTime = 0;
                        snowStormParticle.maxTime = GetRandomValue(1, 2);
                    }
                }
            }

        }

        // Speed Particles
        if (speedFX.active)
        {
            speedFX.spawnTime++;

            for (int i = 0; i < MAX_PARTICLES_SPEED; i++)
            {
                if (!speedFX.particle[i].active && (speedFX.spawnTime >= speedFX.maxTime))
                {
                    speedFX.particle[i].active = true;
                    speedFX.particle[i].alpha = 0.7f;
                    speedFX.particle[i].size = (Vector2){ GetScreenWidth(), GetRandomValue(5, 30) };
                    speedFX.particle[i].position = (Vector2){ GetScreenWidth(), GetRandomValue(0, GetScreenHeight() - 10) };
                    speedFX.spawnTime = 0;
                    speedFX.maxTime = GetRandomValue(1, 10);
                    //i = MAX_PARTICLES;
                }
            }
        }

        for (int i = 0; i < MAX_PARTICLES_SPEED; i++)
        {
            if (speedFX.particle[i].active)
            {
                speedFX.particle[i].position.x -= 40;
                speedFX.particle[i].alpha -= 0.015f;
                speedFX.particle[i].size.y -= 0.1f;

                if (speedFX.particle[i].size.y <= 0) speedFX.particle[i].active = false;
            }
        }

        // Ray Particles
        if (rayParticles.active)
        {
            rayParticles.spawnTime += 1*TIME_FACTOR;
            backRayParticles.spawnTime += 1*TIME_FACTOR;

            for (int i = 0; i < MAX_PARTICLES_RAY; i++)
            {
                if (!rayParticles.particles[i].active && (rayParticles.spawnTime >= rayParticles.maxTime))
                {
                    //printf("PARTICLEEES");
                    rayParticles.particles[i].active = true;
                    rayParticles.particles[i].alpha = 0.0f;
                    rayParticles.particles[i].size = (float)(GetRandomValue(10, 20)/10);
                    rayParticles.particles[i].position = (Vector2){ GetRandomValue(300, GetScreenWidth() + 200), 0 };
                    rayParticles.particles[i].rotation = -35;
                    rayParticles.spawnTime = 0;
                    rayParticles.particles[i].delayCounter = 0;
                    rayParticles.maxTime = GetRandomValue(20, 50);
                }

                if (!backRayParticles.particles[i].active && (backRayParticles.spawnTime >= backRayParticles.maxTime))
                {
                    backRayParticles.particles[i].active = true;
                    backRayParticles.particles[i].alpha = 0.0f;
                    backRayParticles.particles[i].size = (float)(GetRandomValue(5, 15)/10);
                    backRayParticles.particles[i].position = (Vector2){ GetRandomValue(300, GetScreenWidth() + 200), 0 };
                    backRayParticles.particles[i].rotation = -35;
                    backRayParticles.spawnTime = 0;
                    backRayParticles.particles[i].delayCounter = 0;
                    backRayParticles.maxTime = GetRandomValue(20, 50);
                }
            }
        }

        // Particles Logic
        for (int i = 0; i < MAX_PARTICLES; i++)
        {
            if (snowParticle.particles[i].active)
            {
                snowParticle.particles[i].position.y += 2*speedMod*TIME_FACTOR;
                snowParticle.particles[i].position.x -= 4*speedMod*TIME_FACTOR;
                snowParticle.particles[i].rotation += 0.5*TIME_FACTOR;

                if (snowParticle.particles[i].position.y >= GetScreenHeight()) snowParticle.particles[i].active = false;
            }

            if (backSnowParticle.particles[i].active)
            {
                backSnowParticle.particles[i].position.y += (int)1.5f*speedMod*TIME_FACTOR;
                backSnowParticle.particles[i].position.x -= 5*speedMod*TIME_FACTOR;
                backSnowParticle.particles[i].rotation += 0.5*TIME_FACTOR;

                if (backSnowParticle.particles[i].position.y >= GetScreenHeight()) backSnowParticle.particles[i].active = false;
            }

            if (planetreeParticle.particles[i].active)
            {
                planetreeParticle.particles[i].position.y += 4*speedMod*TIME_FACTOR;
                planetreeParticle.particles[i].position.x -= 5*speedMod*TIME_FACTOR;
                planetreeParticle.particles[i].rotation += 0.5*TIME_FACTOR;

                if (planetreeParticle.particles[i].position.y >= GetScreenHeight()) planetreeParticle.particles[i].active = false;
            }

            if (backPlanetreeParticle.particles[i].active)
            {
                backPlanetreeParticle.particles[i].position.y += 3*speedMod*TIME_FACTOR;
                backPlanetreeParticle.particles[i].position.x -= 5*speedMod*TIME_FACTOR;
                backPlanetreeParticle.particles[i].rotation += 0.5*TIME_FACTOR;

                if (backPlanetreeParticle.particles[i].position.y >= GetScreenHeight()) backPlanetreeParticle.particles[i].active = false;
            }

            if (dandelionParticle.particles[i].active)
            {
                dandelionParticle.particles[i].position.y += 3*speedMod*TIME_FACTOR;
                dandelionParticle.particles[i].position.x -= 5*speedMod*TIME_FACTOR;
                dandelionParticle.particles[i].rotation = -(30*sin(2*PI/120*globalFrameCounter + dandelionParticle.particles[i].rotPhy) + 30);

                if (dandelionParticle.particles[i].position.y >= GetScreenHeight()) dandelionParticle.particles[i].active = false;
            }

            if (dandelionBackParticle.particles[i].active)
            {
                dandelionBackParticle.particles[i].position.y += (int)1.5f*speedMod*TIME_FACTOR;
                dandelionBackParticle.particles[i].position.x -= 5*speedMod*TIME_FACTOR;
                dandelionBackParticle.particles[i].rotation = 30*sin(2*PI/120*globalFrameCounter + dandelionParticle.particles[i].rotPhy) + 30;

                if (dandelionBackParticle.particles[i].position.y >= GetScreenHeight()) dandelionBackParticle.particles[i].active = false;
            }

            if (flowerParticle.particles[i].active)
            {
                flowerParticle.particles[i].position.y += 2.5f*speedMod*TIME_FACTOR;
                flowerParticle.particles[i].position.x -= 4*speedMod*TIME_FACTOR;
                flowerParticle.particles[i].rotation += 0.5*TIME_FACTOR;

                if (flowerParticle.particles[i].position.y >= GetScreenHeight()) flowerParticle.particles[i].active = false;
            }

            if (backFlowerParticle.particles[i].active)
            {
                backFlowerParticle.particles[i].position.y += 2.5f*speedMod*TIME_FACTOR;
                backFlowerParticle.particles[i].position.x -= 5*speedMod*TIME_FACTOR;
                backFlowerParticle.particles[i].rotation += 0.5*TIME_FACTOR;

                if (backFlowerParticle.particles[i].position.y >= GetScreenHeight()) backFlowerParticle.particles[i].active = false;
            }

            if (rainParticle.particles[i].active)
            {
                rainParticle.particles[i].position.y += 8*speedMod*TIME_FACTOR;
                rainParticle.particles[i].position.x -= 10*speedMod*TIME_FACTOR;
                //rainParticle.particles[i].rotation += 0.5;

                if (rainParticle.particles[i].position.y >= GetScreenHeight()) rainParticle.particles[i].active = false;
            }

            if (backRainParticle.particles[i].active)
            {
                backRainParticle.particles[i].position.y += 6*speedMod*TIME_FACTOR;
                backRainParticle.particles[i].position.x -= 6*speedMod*TIME_FACTOR;
                //rainParticle.particles[i].rotation += 0.5;

                if (backRainParticle.particles[i].position.y >= GetScreenHeight()) backRainParticle.particles[i].active = false;
            }
        }

        for (int i = 0; i < MAX_PARTICLES_STORM; i++)
        {
            if (rainStormParticle.particles[i].active)
            {
                rainStormParticle.particles[i].position.y += 12*speedMod*TIME_FACTOR;
                rainStormParticle.particles[i].position.x -= 15*speedMod*TIME_FACTOR;
                //rainParticle.particles[i].rotation += 0.5;

                if (rainStormParticle.particles[i].position.y >= GetScreenHeight()) rainStormParticle.particles[i].active = false;
                if (rainStormParticle.active == false)rainStormParticle.particles[i].alpha -= 0.01;
            }
        }

        for (int i = 0; i < MAX_PARTICLES_STORM; i++)
        {
            if (snowStormParticle.particles[i].active)
            {
                snowStormParticle.particles[i].position.y += 12*speedMod*TIME_FACTOR;
                snowStormParticle.particles[i].position.x -= 15*speedMod*TIME_FACTOR;
                snowStormParticle.particles[i].rotation += 0.5*TIME_FACTOR;
                if (snowStormParticle.particles[i].position.y >= GetScreenHeight()) snowStormParticle.particles[i].active = false;
            }
        }

        for (int i = 0; i < MAX_PARTICLES_RAY; i++)
        {
            if (rayParticles.particles[i].active)
            {
                rayParticles.particles[i].position.x -= 0.5*speedMod*TIME_FACTOR;
                //printf("RAY X: %i", rayParticles.particles[i].position.x);

                if (rayParticles.particles[i].fading)
                {
                    rayParticles.particles[i].alpha -= 0.01f;

                    if (rayParticles.particles[i].alpha <= 0)
                    {
                        rayParticles.particles[i].alpha = 0;
                        rayParticles.particles[i].delayCounter += 1;
                        if (rayParticles.particles[i].delayCounter >= 30)
                        {
                            rayParticles.particles[i].active = false;
                            rayParticles.particles[i].delayCounter = 0;
                            rayParticles.particles[i].fading = false;
                        }
                    }
                }
                else
                {
                    rayParticles.particles[i].alpha += 0.01f;

                    if (rayParticles.particles[i].alpha >= 0.5f)
                    {
                        rayParticles.particles[i].alpha = 0.5f;
                        rayParticles.particles[i].delayCounter += 1*TIME_FACTOR;
                        if (rayParticles.particles[i].delayCounter >= 30)
                        {
                            rayParticles.particles[i].delayCounter = 0;
                            rayParticles.particles[i].fading = true;
                        }
                    }
                }
            }

            if (backRayParticles.particles[i].active)
            {
                backRayParticles.particles[i].position.x -= 0.5*speedMod*TIME_FACTOR;

                if (backRayParticles.particles[i].fading)
                {
                    backRayParticles.particles[i].alpha -= 0.01f;

                    if (backRayParticles.particles[i].alpha <= 0)
                    {
                        backRayParticles.particles[i].alpha = 0;
                        backRayParticles.particles[i].delayCounter += 1;

                        if (backRayParticles.particles[i].delayCounter >= 30)
                        {
                            backRayParticles.particles[i].active = false;
                            backRayParticles.particles[i].delayCounter = 0;
                            backRayParticles.particles[i].fading = false;
                        }
                    }
                }
                else
                {
                    backRayParticles.particles[i].alpha += 0.01f;

                    if (backRayParticles.particles[i].alpha >= 0.5f)
                    {
                        backRayParticles.particles[i].alpha = 0.5f;
                        backRayParticles.particles[i].delayCounter +=1;

                        if (backRayParticles.particles[i].delayCounter >= 30)
                        {
                            backRayParticles.particles[i].delayCounter = 0;
                            backRayParticles.particles[i].fading = true;
                        }
                    }
                }
            }
        }

        // Player States
        switch (state)
        {
            case GRABED:
            {
                onWind = false;
                windCounter += 1*TIME_FACTOR;
                resinCount += 1*TIME_FACTOR;
                //speedMod = 1;
                
                thisFrameKoala += 1*TIME_FACTOR;

                if (thisFrameKoala >= 24)
                {
                    curFrameKoala += 1;
                    thisFrameKoala = 0;
                }

                if (curFrameKoala > 2) curFrameKoala = 0;

                koalaAnimationIddle.x = gameplay_koala_idle.x + koalaAnimationIddle.width*curFrameKoala;

                if (!onResin)
                {
#if (defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB))
                    if (((IsGestureDetected(GESTURE_TAP) || (GetGestureDetected() == GESTURE_DOUBLETAP)) && CheckCollisionPointRec(GetTouchPosition(0), rightButton)))
                    {
                        state = JUMPING;
                        velocity = JUMP;
                        grabCounter = 0;
                        jumpSpeed = 6;
                        rightAlpha = 1;
                        onIce = false;
                        onResin = false;
                        thisFrameKoala = 0;
                        PlaySound(fxJump);
                        
                        jumpCounter++;
                    }

                    if (((IsGestureDetected(GESTURE_TAP) || (GetGestureDetected() == GESTURE_DOUBLETAP)) && CheckCollisionPointRec(GetTouchPosition(0), leftButton)))
                    {
                        if(!onIce)gravity = KICKSPEED;
                        else gravity = ICEGRAVITY;
                        PlaySound(fxDash);

                        state = KICK;
                        grabCounter = 0;
                        leftAlpha = 1;
                        onResin = false;
                        dashCounter++;
                        //thisFrameKoala = 0;
                    }
                    else
                    {
                        if(!onIce)gravity = GRAVITY;
                        else gravity = ICEGRAVITY;
                        //thisFrameKoala = 0;
                    }
#elif (defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB))
                        
                    if (IsKeyPressed(KEY_SPACE) || (CheckCollisionPointRec(GetMousePosition(), rightButton) && IsMouseButtonPressed(0)))
                    {
                        state = JUMPING;
                        velocity = JUMP;
                        grabCounter = 0;
                        jumpSpeed = 6;
                        rightAlpha = 1;
                        onIce = false;
                        onResin = false;
                        thisFrameKoala = 0;
                        PlaySound(fxJump);
                        jumpCounter++;
                    }

                    if (IsKeyPressed(KEY_DOWN) || (CheckCollisionPointRec(GetMousePosition(), leftButton) && IsMouseButtonPressed(0)))
                    {
                        if(!onIce)gravity = KICKSPEED;
                        else gravity = ICEGRAVITY;
                        PlaySound(fxDash);

                        state = KICK;
                        grabCounter = 0;
                        leftAlpha = 1;
                        onResin = false;
                        dashCounter++;
                        //thisFrameKoala = 0;
                    }
                    else
                    {
                        if(!onIce)gravity = GRAVITY;
                        else gravity = ICEGRAVITY;
                        //thisFrameKoala = 0;
                    }
#endif
                }
                else
                {
#if (defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB))
                    if (((IsGestureDetected(GESTURE_TAP) || (GetGestureDetected() == GESTURE_DOUBLETAP)) && CheckCollisionPointRec(GetTouchPosition(0), rightButton)))
                    {
                        resinCountjump++;

                        if (resinCountjump >= 2)
                        {
                            //thisFrameKoala = 0;

                            state = JUMPING;
                            velocity = JUMP;
                            grabCounter = 0;
                            jumpSpeed = 6;
                            rightAlpha = 1;
                            onIce = false;
                            onResin = false;
                            resinCountjump = 0;
                            resinCountdrag = 0;
                            resinCount = 0;
                            jumpCounter++;
                        }
                    }

                    if (((IsGestureDetected(GESTURE_TAP) || (GetGestureDetected() == GESTURE_DOUBLETAP)) && CheckCollisionPointRec(GetTouchPosition(0), leftButton)))
                    {
                        resinCountdrag ++;

                        if (resinCountdrag >= 2)
                        {
                            //thisFrameKoala = 0;
                            gravity = KICKSPEED;
                            state = KICK;
                            grabCounter = 0;
                            leftAlpha = 1;
                            onResin = false;
                            resinCountjump = 0;
                            resinCountdrag = 0;
                            resinCount = 0;
                            dashCounter++;
                        }
                    }
                    else
                    {
                        gravity = 0;
                    }
#elif (defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB))
                    if (IsKeyPressed(KEY_SPACE) || (CheckCollisionPointRec(GetMousePosition(), rightButton) && IsMouseButtonPressed(0)))
                    {
                        resinCountjump++;

                        if (resinCountjump >= 2)
                        {
                            //thisFrameKoala = 0;

                            state = JUMPING;
                            velocity = JUMP;
                            grabCounter = 0;
                            jumpSpeed = 6;
                            rightAlpha = 1;
                            onIce = false;
                            onResin = false;
                            resinCountjump = 0;
                            resinCountdrag = 0;
                            resinCount = 0;
                            jumpCounter++;
                        }
                    }

                    if (IsKeyPressed(KEY_DOWN) || (CheckCollisionPointRec(GetMousePosition(), leftButton) && IsMouseButtonPressed(0)))
                    {
                        resinCountdrag ++;

                        if (resinCountdrag >= 2)
                        {
                            //thisFrameKoala = 0;
                            gravity = KICKSPEED;
                            state = KICK;
                            grabCounter = 0;
                            leftAlpha = 1;
                            onResin = false;
                            resinCountjump = 0;
                            resinCountdrag = 0;
                            resinCount = 0;
                            dashCounter++;
                        }
                    }
                    else
                    {
                        gravity = 0;
                    }
#endif
                }
            } break;
            case JUMPING:
            {
                player.x += jumpSpeed*TIME_FACTOR;
                velocity -= 1*TIME_FACTOR*TIME_FACTOR;
                player.y -= velocity;
                framesCounter += 1*TIME_FACTOR;
                grabCounter+= 1*TIME_FACTOR;
                
            } break;
            case KICK:
            {
                gravity += 1*TIME_FACTOR*TIME_FACTOR;
                player.y += gravity;
                player.x -= speed;
                grabCounter += 1*TIME_FACTOR;

                for (int i = 0; i < MAX_ENEMIES; i++)
                {
                    if (CheckCollisionRecs(player, snake[i]) && !isHitSnake[i] && snakeActive[i])
                    {
                        state = JUMPING;
                        velocity = JUMP;
                        enemyVel[i] = 8;
                        grabCounter = 3;
                        gravity = KICKSPEED;
                        isHitSnake[i] = true;
                        jumpSpeed = -3;
                        score += SNAKESCORE;
                        killHistory[killCounter] = 1;
                        killCounter++;
                        PlaySound(fxDieSnake);
                        snakeKillCounter++;
                        globalKillCounter++;

                        enemyHit[i].position = (Vector2){ snake[i].x, snake[i].y };
                        enemyHit[i].speed = (Vector2){ snake[i].x, snake[i].y };
                        enemyHit[i].size = (float)GetRandomValue(5, 10)/30;
                        enemyHit[i].rotation = 0.0f;
                        //enemyHit[i].color = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
                        enemyHit[i].alpha = 1.0f;
                        enemyHit[i].active = true;

                        popupScore[i].position = (Vector2){ snake[i].x,snake[i].y };
                        popupScore[i].scale = 1.0f;
                        popupScore[i].alpha = 1.0f;
                        popupScore[i].score = SNAKESCORE;
                        popupScore[i].active = true; 
                    }

                    if (CheckCollisionRecs(player, dingo[i]) && !isHitDingo[i] && dingoActive[i])
                    {
                        state = JUMPING;
                        velocity = JUMP;
                        enemyVel[i] = 8*TIME_FACTOR;
                        grabCounter = 3*TIME_FACTOR;
                        gravity = KICKSPEED;
                        isHitDingo[i] = true;
                        jumpSpeed = -3;
                        score += DINGOSCORE;
                        killHistory[killCounter] = 2;
                        killCounter++;
                        PlaySound(fxDieDingo);
                        dingoKillCounter++;
                        globalKillCounter++;

                        enemyHit[i].position = (Vector2){ dingo[i].x, dingo[i].y };
                        enemyHit[i].speed = (Vector2){ dingo[i].x, dingo[i].y };
                        enemyHit[i].size = (float)GetRandomValue(5, 10)/30;
                        enemyHit[i].rotation = 0.0f;
                        enemyHit[i].alpha = 1.0f;
                        enemyHit[i].active = true;
                        
                        popupScore[i].position = (Vector2){ dingo[i].x,dingo[i].y };
                        popupScore[i].scale = 1.0f;
                        popupScore[i].alpha = 1.0f;
                        popupScore[i].score = DINGOSCORE;
                        popupScore[i].active = true; 
                    }

                    if (CheckCollisionRecs(player, owl[i]) && !isHitOwl[i] && owlActive[i])
                    {
                        state = JUMPING;
                        velocity = JUMP;
                        enemyVel[i] = 8;
                        grabCounter = 3;
                        gravity = KICKSPEED;
                        isHitOwl[i] = true;
                        jumpSpeed = -3;
                        score += OWLSCORE;
                        killHistory[killCounter] = 3;
                        killCounter++;
                        PlaySound(fxDieOwl);
                        owlKillCounter++;
                        globalKillCounter++;

                        enemyHit[i].position = (Vector2){ owl[i].x, owl[i].y };
                        enemyHit[i].speed = (Vector2){ owl[i].x, owl[i].y };
                        enemyHit[i].size = (float)GetRandomValue(5, 10)/30;
                        enemyHit[i].rotation = 0.0f;
                        enemyHit[i].alpha = 1.0f;
                        enemyHit[i].active = true;
                        
                        popupScore[i].position = (Vector2){ owl[i].x,owl[i].y };
                        popupScore[i].scale = 1.0f;
                        popupScore[i].alpha = 1.0f;
                        popupScore[i].score = OWLSCORE;
                        popupScore[i].active = true; 
                    }
                }
                
                if (CheckCollisionRecs(player, bee) && !isHitBee && beeActive)
                {
                    state = JUMPING;
                    velocity = JUMP;
                    beeVelocity = 8;
                    grabCounter = 3;
                    gravity = KICKSPEED;
                    isHitBee = true;
                    jumpSpeed = -3;
                    score += BEESCORE;
                    killHistory[killCounter] = 4;
                    killCounter++;
                    beeKillCounter++;
                    globalKillCounter++;
                    
                    popupBee.position = (Vector2){ bee.x, bee.y };
                    popupBee.scale = 1.0f;
                    popupBee.alpha = 1.0f;
                    popupBee.score = BEESCORE; 
                    popupBee.active = true; 
                }
                
                if (CheckCollisionRecs(player, eagle) && !isHitEagle && eagleActive)
                {
                    state = JUMPING;
                    velocity = JUMP;
                    beeVelocity = 8;
                    grabCounter = 3;
                    gravity = KICKSPEED;
                    isHitEagle = true;
                    jumpSpeed = -3;
                    score += EAGLESCORE;
                    killHistory[killCounter] = 5;
                    killCounter++;
                    eagleKillCounter++;
                    globalKillCounter++;
                    
                    popupEagle.position = (Vector2){ eagle.x, eagle.y };
                    popupEagle.scale = 1.0f;
                    popupEagle.alpha = 1.0f;
                    popupEagle.score = EAGLESCORE; 
                    popupEagle.active = true; 
                }
            } break;
            case FINALFORM:
            {
                if (transforming)
                {
                    speedMod = 0;
                    transCount += 1*TIME_FACTOR;
                    transRotation += 1*TIME_FACTOR;
                    transAniCounter += 1*TIME_FACTOR;
                    
                    thisFrameKoala += 1*TIME_FACTOR;
                    
                    currentLeaves = LinearEaseIn((float)transCount, (float)initLeaves, (float)-LEAVESTOTRANSFORM, 120.0f);

                    if (thisFrameKoala >= 24)
                    {
                        curFrameKoala += 1;
                        thisFrameKoala = 0;
                    }
                    
                    if (curFrameKoala <= 1) koalaAnimationTransform.x = gameplay_koala_transform.x + koalaAnimationTransform.width*curFrameKoala;

                    if (transAniCounter >= 5)
                    {
                        transBackAnim = !transBackAnim;
                        transAniCounter = 0;
                    }

                    if (transBackAnim)
                    {
                        finalColor = RED;
                        finalColor2 = WHITE;
                    }
                    else
                    {
                        finalColor = WHITE;
                        finalColor2 = RED;
                    }

                    if (transCount >= 120)
                    {
                        transforming = false;
                        thisFrameKoala = 0;
                        curFrameKoala = 0;
                        speedFX.active = true;
                        transCount = 0;
                        bambooTimer += 15*TIME_FACTOR;
                    }
                }
                else if (!transforming)
                {
                    speedMod = 5;
                    scrollSpeed = 3.2f*TIME_FACTOR;
                    power -= 1*TIME_FACTOR;

                    thisFrameKoala += 1*TIME_FACTOR;

                    if (thisFrameKoala >= 12)
                    {
                        curFrameKoala ++;
                        thisFrameKoala = 0;
                    }
                    
                    if (curFrameKoala > 1) curFrameKoala = 0;
                    if (curFrameKoala <= 1) koalaAnimationFly.x = gameplay_koala_fly.x + koalaAnimationFly.width*curFrameKoala;
                    if (player.x > GetScreenWidth()/3) player.x -= 2;
                    if (player.x < GetScreenWidth()/3) player.x++;
                    
                    if (power <= maxPower/5)
                    {
                        finalFormEnd += 1*TIME_FACTOR;
                        if (finalFormEnd >= 5)
                        {
                            transBackAnim = !transBackAnim;
                            finalFormEnd = 0;
                        }
                        
                        if (transBackAnim)
                        {
                            finalColor = RED;
                        }

                        if (!transBackAnim)
                        {
                            finalColor = WHITE;
                        }
                    }
                    else finalColor = WHITE;
                    
#if (defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB))
                    if ((IsGestureDetected(GESTURE_HOLD) || (GetGestureDetected() == GESTURE_DRAG)) && CheckCollisionPointRec(GetTouchPosition(0), leftButton)) player.y += FLYINGMOV;
                    if ((IsGestureDetected(GESTURE_HOLD) || (GetGestureDetected() == GESTURE_DRAG)) && CheckCollisionPointRec(GetTouchPosition(0), rightButton)) player.y -= FLYINGMOV;
#elif (defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB))
                    if (IsKeyDown(KEY_DOWN) || (CheckCollisionPointRec(GetMousePosition(), leftButton) && IsMouseButtonDown(0))) player.y += FLYINGMOV;
                    if (IsKeyDown(KEY_UP) || (CheckCollisionPointRec(GetMousePosition(), rightButton) && IsMouseButtonDown(0))) player.y -= FLYINGMOV;
#endif
                }

                gravity = 0;
                grabCounter ++;

            } break;
            case ONWIND:
            {
                player.x -= jumpSpeed*TIME_FACTOR;
                velocity -= 2*TIME_FACTOR;
                player.y -= velocity;
                framesCounter += 1*TIME_FACTOR;
                grabCounter += 1*TIME_FACTOR;

            } break;
            default: break;
         }

        if (player.x <= (-player.width))
        {
            play = false;
            playerActive = false;
            killer = 4;
        }

        if ((player.y + player.height) >= GetScreenHeight())
        {
            if (state == FINALFORM) player.y = GetScreenHeight() - player.height;
            else
            {
                play = false;
                playerActive = false;
                killer = 4;
            }
        }

        if ((player.y) <= 0 && state == FINALFORM) player.y = 0;
        if (player.x >= (GetScreenWidth() - player.width)) player.x = (GetScreenWidth() - player.width);
        if (player.y <= -32) player.y = -32;

        if (bambooTimer > bambooSpawnTime)
        {
            if (!transforming)
            {
                BambooSpawn();
                
                if (state != FINALFORM && eagleDelay >= EAGLE_TIME_DELAY)EagleSpawn(EAGLE_SPAWNCHANCE);

                switch (season)
                {
                    case WINTER:
                        IceSpawn(ICESPAWNCHANCE);
                        OwlSpawn(OWLSPAWNCHANCE + (int)progresionSpawnChance);
                    break;
                    case SPRING:
                        ResinSpawn(RESINSPAWNCHANCE);
                        if (state != FINALFORM)BeeSpawn(BEE_SPAWNCHANCE);
                        DingoSpawn(DINGOSPAWNCHANCE + (int)progresionSpawnChance);
                    break;
                    case SUMMER:
                        FireSpawn(FIRESPAWNCHANCE);
                        SnakeSpawn(SNAKESPAWNCHANCE + (int)progresionSpawnChance);
                    break;
                    case FALL:
                        WindSpawn(WINDSPAWNCHANCE);
                        SnakeSpawn(SNAKESPAWNCHANCE + (int)progresionSpawnChance);
                    break;
                    default: break;
                }

                LeafSpawn();
            }

            bambooTimer = 0;
            bambooSpawnTime = GetRandomValue(MINTIMESPAWN, MAXTIMESPAWN);
        }
    }
    else if (!play && !playerActive)
    {
        if (score > hiscore) hiscore = score;

        player.x -= jumpSpeed;
        velocity -= 1*TIME_FACTOR;
        player.y -= velocity;

        if (player.y >= GetScreenHeight())
        {
            deathsCounter++;
            finishScreen = 1;
        }
    }
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
    BeginShaderMode(colorBlend);

    DrawTexturePro(atlas02, gameplay_background, (Rectangle){0, 0, gameplay_background.width*2, gameplay_background.height*2}, (Vector2){0, 0}, 0, color02);

    // Draw parallax
    DrawParallaxBack();
    DrawParallaxMiddle();

    // Draw particles (only if active)
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (backSnowParticle.particles[i].active) DrawTexturePro(atlas02, particle_icecrystal_bw,
                                               (Rectangle){ backSnowParticle.particles[i].position.x, backSnowParticle.particles[i].position.y, particle_icecrystal_bw.width*backSnowParticle.particles[i].size, particle_icecrystal_bw.height*backSnowParticle.particles[i].size },
                                               (Vector2){ particle_icecrystal_bw.width*backSnowParticle.particles[i].size/2, particle_icecrystal_bw.height*backSnowParticle.particles[i].size/2 }, backSnowParticle.particles[i].rotation,
                                               Fade((Color){144, 214, 255, 255}, backSnowParticle.particles[i].alpha));

        if (backPlanetreeParticle.particles[i].active) DrawTexturePro(atlas02, particle_planetreeleaf_bw,
                                               (Rectangle){ backPlanetreeParticle.particles[i].position.x, backPlanetreeParticle.particles[i].position.y, particle_planetreeleaf_bw.width*backPlanetreeParticle.particles[i].size, particle_planetreeleaf_bw.height*backPlanetreeParticle.particles[i].size },
                                               (Vector2){ particle_planetreeleaf_bw.width*backPlanetreeParticle.particles[i].size/2, particle_planetreeleaf_bw.height*backPlanetreeParticle.particles[i].size/2 }, backPlanetreeParticle.particles[i].rotation,
                                               Fade((Color){179, 86, 6, 255}, backPlanetreeParticle.particles[i].alpha));

        if (dandelionBackParticle.particles[i].active) DrawTexturePro(atlas02, particle_dandelion_bw,
                                               (Rectangle){ dandelionBackParticle.particles[i].position.x, dandelionBackParticle.particles[i].position.y, particle_dandelion_bw.width*dandelionBackParticle.particles[i].size, particle_dandelion_bw.height*dandelionBackParticle.particles[i].size },
                                               (Vector2){ particle_dandelion_bw.width*dandelionBackParticle.particles[i].size/2, particle_dandelion_bw.height*dandelionBackParticle.particles[i].size/2 }, dandelionBackParticle.particles[i].rotation,
                                               Fade((Color){202, 167, 126, 255}, dandelionBackParticle.particles[i].alpha));

        if (backFlowerParticle.particles[i].active) DrawTexturePro(atlas02, particle_ecualyptusflower_bw,
                                               (Rectangle){ backFlowerParticle.particles[i].position.x, backFlowerParticle.particles[i].position.y, particle_ecualyptusflower_bw.width*backFlowerParticle.particles[i].size, particle_ecualyptusflower_bw.height*backFlowerParticle.particles[i].size },
                                               (Vector2){ particle_ecualyptusflower_bw.width*backFlowerParticle.particles[i].size/2, particle_ecualyptusflower_bw.height*backFlowerParticle.particles[i].size/2 }, backFlowerParticle.particles[i].rotation,
                                               Fade((Color){218, 84, 108, 255}, backFlowerParticle.particles[i].alpha));

        if (backRainParticle.particles[i].active) DrawTexturePro(atlas02, particle_waterdrop_bw,
                                               (Rectangle){ backRainParticle.particles[i].position.x, backRainParticle.particles[i].position.y, particle_waterdrop_bw.width*backRainParticle.particles[i].size, particle_waterdrop_bw.height*backRainParticle.particles[i].size },
                                               (Vector2){ particle_waterdrop_bw.width*backRainParticle.particles[i].size/2, particle_waterdrop_bw.height*backRainParticle.particles[i].size/2 }, backRainParticle.particles[i].rotation,
                                               Fade((Color){144, 183, 187, 255}, backRainParticle.particles[i].alpha));
    }

    for (int i = 0; i < MAX_PARTICLES_RAY; i++)
    {
       if (backRayParticles.particles[i].active) DrawTexturePro(atlas02, gameplay_back_fx_lightraymid,
                                               (Rectangle){ backRayParticles.particles[i].position.x, backRayParticles.particles[i].position.y, gameplay_back_fx_lightraymid.width*backRayParticles.particles[i].size, gameplay_back_fx_lightraymid.height*backRayParticles.particles[i].size },
                                               (Vector2){ gameplay_back_fx_lightraymid.width*backRayParticles.particles[i].size/2, gameplay_back_fx_lightraymid.height*backRayParticles.particles[i].size/2 }, backRayParticles.particles[i].rotation,
                                               Fade(backRayParticles.particles[i].color, backRayParticles.particles[i].alpha));
    }

    DrawParallaxFront();

    for (int i = 0; i < MAX_BAMBOO; i++)
    {
        if (bambooActive[i])
        {
            DrawTexturePro(atlas02, gameplay_props_tree, (Rectangle){bamboo[i].x, bamboo[i].y, 43, 720}, (Vector2){0, 0}, 0, color03);
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (branchActive[i])
        {
            DrawTexturePro(atlas02, gameplay_props_owl_branch, (Rectangle){branchPos[i].x, branchPos[i].y, 36, 20}, (Vector2){0, 0}, 0, color03);
        }
    }

    EndShaderMode();

    for (int i = 0; i < MAX_FIRE; i++)
    {
        DrawTexturePro(atlas01, (Rectangle){gameplay_props_burnttree.x, gameplay_props_burnttree.y + fire[i].y + gameplay_props_burnttree.height/14, gameplay_props_burnttree.width, gameplay_props_burnttree.height},
                    (Rectangle){fire[i].x + 5, fire[i].y + gameplay_props_burnttree.height/14, gameplay_props_burnttree.width, gameplay_props_burnttree.height}, (Vector2){0, 0}, 0, WHITE);
        DrawTextureRec(atlas01, fireAnimation, (Vector2){fire[i].x, GetScreenHeight() - gameplay_props_burnttree.height/7}, WHITE);

        for(int j = MAX_FIRE_FLAMES; j > -2; j--)
        {
            if ((fire[i].y - 25 <= (j*43)) && fireActive[i])
            {
                if (j%2 > 0)
                {
                    DrawTextureRec(atlas01, fireAnimation, (Vector2){fire[i].x + fireOffset - 10, 40*j}, WHITE);
                    fireAnimation.x = gameplay_props_fire_spritesheet.x + fireAnimation.width*curFrame1;
                }
                else if (j%2 + 1 == 1)
                {
                    DrawTextureRec(atlas01, fireAnimation, (Vector2){fire[i].x - fireOffset , 40*j}, WHITE);
                    fireAnimation.x = gameplay_props_fire_spritesheet.x + fireAnimation.width*curFrame2;
                }
                else
                {
                    DrawTextureRec(atlas01, fireAnimation, (Vector2){fire[i].x - fireOffset , 40*j}, WHITE);
                    fireAnimation.x = gameplay_props_fire_spritesheet.x + fireAnimation.width*curFrame3;
                }
            }
        }
    }

    for (int i = 0; i < MAX_ICE; i++)
    {
        if (iceActive[i]) for (int k = 0; k < GetScreenHeight(); k += (GetScreenHeight()/6)) DrawTexturePro(atlas01, gameplay_props_ice_sprite, (Rectangle){ice[i].x - 5, ice[i].y+k, gameplay_props_ice_sprite.width, gameplay_props_ice_sprite.height}, (Vector2){0,0}, 0, WHITE);
    }

    BeginShaderMode(colorBlend);

    DrawTexturePro(atlas02, gameplay_back_ground00, (Rectangle){(int)groundPos + GetScreenWidth(), 637, gameplay_back_ground00.width*2, gameplay_back_ground00.height*2}, (Vector2){0,0}, 0, color00);
    DrawTexturePro(atlas02, gameplay_back_ground00, (Rectangle){(int)groundPos, 637, gameplay_back_ground00.width*2, gameplay_back_ground00.height*2}, (Vector2){0,0}, 0, color00);

    EndShaderMode();

    for (int i = 0; i < MAX_RESIN; i++)
    {
        if (resinActive[i]) DrawTextureRec(atlas01, gameplay_props_resin_sprite,(Vector2){ resin[i].x - resin[i].width/3, resin[i].y - resin[i].height/5}, WHITE);
    }

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (snakeActive[i])
        {
            if (!isHitSnake[i])DrawTextureRec(atlas01, snakeAnimation, (Vector2){snake[i].x - snake[i].width, snake[i].y - snake[i].height/2}, WHITE);
            else DrawTextureRec(atlas01, (Rectangle){gameplay_enemy_snake.x + snakeAnimation.width*2, gameplay_enemy_snake.y, snakeAnimation.width, snakeAnimation.height}, (Vector2){snake[i].x - snake[i].width/2, snake[i].y - snake[i].height/2}, WHITE);
        }

        if (dingoActive[i])
        {
            if (!isHitDingo[i]) DrawTextureRec(atlas01, dingoAnimation, (Vector2){dingo[i].x - dingo[i].width/2, dingo[i].y - dingo[i].height/4}, WHITE);
            else DrawTextureRec(atlas01, (Rectangle){gameplay_enemy_dingo.x + dingoAnimation.width*2, gameplay_enemy_dingo.y, dingoAnimation.width, dingoAnimation.height}, (Vector2){dingo[i].x - dingo[i].width/2, dingo[i].y - dingo[i].height/4}, WHITE);
        }

        if (owlActive[i])
        {
            if (!isHitOwl[i])DrawTextureRec(atlas01, owlAnimation, (Vector2){owl[i].x - owl[i].width*0.7, owl[i].y - owl[i].height*0.1}, WHITE);
            else DrawTextureRec(atlas01, (Rectangle){gameplay_enemy_owl.x + owlAnimation.width*2, gameplay_enemy_owl.y, owlAnimation.width, owlAnimation.height}, (Vector2){owl[i].x - owl[i].width/2, owl[i].y - owl[i].height/6}, WHITE);
        }

        if (enemyHit[i].active)
        {
            DrawTexturePro(atlas01, particle_hit,
                                (Rectangle){ enemyHit[i].position.x, enemyHit[i].position.y, particle_hit.width*enemyHit[i].size, particle_hit.height*enemyHit[i].size },
                                (Vector2){ particle_hit.width*enemyHit[i].size/2, particle_hit.height*enemyHit[i].size/2 }, enemyHit[i].rotation,
                                Fade(enemyHit[i].color, enemyHit[i].alpha));
        }
    }
    
    // Only one bee / eagle / alert at the same time
    
    for (int i = 0; i < MAX_LEAVES; i++)
    {
        if (leafActive[i])
        {
            if (leafSide[i])
            {
                if (leafType[i] == 0) DrawTextureRec(atlas01, (Rectangle){ gameplay_props_leaf_lil.x, gameplay_props_leaf_lil.y, -gameplay_props_leaf_lil.width, gameplay_props_leaf_lil.height }, (Vector2){ leaf[i].x, leaf[i].y - 15 }, WHITE);
                else if (leafType[i] == 1) DrawTextureRec(atlas01, (Rectangle){ gameplay_props_leaf_lil.x, gameplay_props_leaf_lil.y, -gameplay_props_leaf_lil.width, gameplay_props_leaf_lil.height }, (Vector2){leaf[i].x, leaf[i].y + 10 }, WHITE);
                else if (leafType[i] == 2) DrawTextureRec(atlas01, (Rectangle){ gameplay_props_leaf_mid.x, gameplay_props_leaf_mid.y, -gameplay_props_leaf_mid.width, gameplay_props_leaf_mid.height }, (Vector2){leaf[i].x, leaf[i].y - 15 }, WHITE);
                else if (leafType[i] == 3) DrawTextureRec(atlas01, (Rectangle){ gameplay_props_leaf_big.x, gameplay_props_leaf_big.y, -gameplay_props_leaf_big.width, gameplay_props_leaf_big.height }, (Vector2){leaf[i].x, leaf[i].y - 15 }, WHITE);
            }
            else
            {
                if (leafType[i] == 0) DrawTextureRec(atlas01, gameplay_props_leaf_lil, (Vector2){ leaf[i].x - 25, leaf[i].y - 15 }, WHITE);
                else if (leafType[i] == 1) DrawTextureRec(atlas01, gameplay_props_leaf_lil, (Vector2){leaf[i].x - 25, leaf[i].y + 10 }, WHITE);
                else if (leafType[i] == 2) DrawTextureRec(atlas01, gameplay_props_leaf_mid, (Vector2){leaf[i].x - 25, leaf[i].y - 15 }, WHITE);
                else if (leafType[i] == 3) DrawTextureRec(atlas01, gameplay_props_leaf_big, (Vector2){leaf[i].x - 25, leaf[i].y - 15 }, WHITE);
            }
#if defined(DEBUG)
            DrawRectangle(leaf[i].x, leaf[i].y, 64, 64, Fade(GREEN, 0.5f));
#endif
        }

        if (leafParticles[i].active)
        {
            for (int j = 0; j < 32; j++)
            {
                DrawTexturePro(atlas01, particle_ecualyptusleaf,
                              (Rectangle){ leafParticles[i].particles[j].position.x, leafParticles[i].particles[j].position.y, particle_ecualyptusleaf.width*leafParticles[i].particles[j].size, particle_ecualyptusleaf.height*leafParticles[i].particles[j].size },
                              (Vector2){ particle_ecualyptusleaf.width/2*leafParticles[i].particles[j].size, particle_ecualyptusleaf.height/2*leafParticles[i].particles[j].size }, leafParticles[i].particles[j].rotation, Fade(WHITE,leafParticles[i].particles[j].alpha));
            }
        }
    }
    
    if (beeActive && !isHitBee) DrawTextureRec(atlas01, beeAnimation, (Vector2){bee.x, bee.y - gameplay_enemy_bee.height/2}, WHITE);
    else if (beeActive && isHitBee) DrawTexturePro(atlas01, (Rectangle){gameplay_enemy_bee.x + beeAnimation.width*4, gameplay_enemy_bee.y, beeAnimation.width, gameplay_enemy_bee.height},
                                                    (Rectangle){bee.x, bee.y, beeAnimation.width, gameplay_enemy_bee.height}, (Vector2){0, 0}, 0, WHITE);
                                                    
    if (eagleActive && !isHitEagle) DrawTextureRec(atlas01, eagleAnimation, (Vector2){eagle.x, eagle.y}, WHITE);
    else if (eagleActive && isHitEagle) DrawTextureRec(atlas01, gameplay_enemy_eagle_death, (Vector2){eagle.x, eagle.y}, WHITE);
    
    if (alertActive) DrawTexturePro(atlas01, gameplay_fx_eaglealert, alertRectangle, (Vector2){0, 0}, 0, Fade(RED, 0.7f));
    if (alertBeeActive) DrawTexturePro(atlas01, gameplay_fx_eaglealert, beeAlertRectangle, (Vector2){0, 0}, 0, Fade(ORANGE, 0.7f));                                        

    if (transforming)
    {
        for (int i = 0; i < 8; i++)
        {
            DrawTexturePro(atlas02, background_transformation,
                (Rectangle){player.x + player.width/2 , player.y + player.height/2, background_transformation.width*4, background_transformation.height*4},
                (Vector2){0, background_transformation.height*2}, 45*i, Fade(finalColor, 0.7f));
        }

        for (int i = 0; i < 8; i++)
        {
            DrawTexturePro(atlas02, background_transformation,
                (Rectangle){player.x + player.width/2 , player.y + player.height/2, background_transformation.width*4, background_transformation.height},
                (Vector2){0, background_transformation.height/2}, 22.5 + 45*i, Fade(finalColor2, 0.7f));
        }
    }

    if (playerActive && play)
    {
        switch(state)
        {
            case GRABED: DrawTextureRec(atlas01, koalaAnimationIddle, (Vector2){player.x - player.width, player.y - gameplay_koala_idle.height/4}, WHITE); break;
            case JUMPING: DrawTexturePro(atlas01, gameplay_koala_jump, (Rectangle){player.x - player.width, player.y - gameplay_koala_jump.height/4, gameplay_koala_jump.width, gameplay_koala_jump.height}, (Vector2){0, 0}, 0, WHITE); break;
            case KICK:DrawTexturePro(atlas01, gameplay_koala_dash, (Rectangle){player.x - player.width, player.y - gameplay_koala_jump.height/4, gameplay_koala_dash.width, gameplay_koala_dash.height}, (Vector2){0, 0}, 0, WHITE);  break;
            case FINALFORM:
            {
                if (transforming)DrawTexturePro(atlas01, koalaAnimationTransform, (Rectangle){player.x - player.width, player.y - gameplay_koala_transform.height/4, gameplay_koala_transform.width/2, gameplay_koala_transform.height}, (Vector2){0, 0}, 0, finalColor);
                else DrawTexturePro(atlas01, koalaAnimationFly, (Rectangle){player.x - gameplay_koala_fly.width/3, player.y - gameplay_koala_fly.height/4, gameplay_koala_fly.width/2, gameplay_koala_fly.height}, (Vector2){0, 0}, 0, finalColor);//DrawTextureRec((koalaFly), (Rectangle){0, 0, 128, 128}, (Vector2){player.x - 50, player.y - 40}, WHITE);
            
            } break;
            case ONWIND: DrawTexturePro(atlas01, gameplay_koala_jump, (Rectangle){player.x - player.width, player.y - gameplay_koala_jump.height/4, gameplay_koala_jump.width, gameplay_koala_jump.height}, (Vector2) { 0, 0}, 0, WHITE); break;
            default: break;
        }
    }
    else if (play == false && playerActive) DrawTextureRec(atlas01, (Rectangle){gameplay_koala_idle.x, gameplay_koala_idle.y, gameplay_koala_idle.width/3, gameplay_koala_idle.height}, (Vector2){player.x - player.width, player.y - gameplay_koala_idle.height/4}, WHITE);
    else DrawTexturePro(atlas01, gameplay_koala_die, (Rectangle){player.x - player.width, player.y - gameplay_koala_die.height/4, gameplay_koala_die.width, gameplay_koala_die.height}, (Vector2) { 0, 0}, 0, WHITE);

    for (int i = 0; i < MAX_WIND; i++)
    {
        if (windActive[i]) DrawTextureRec(atlas01, windAnimation, (Vector2){wind[i].x - 14, wind[i].y - 14}, WHITE);
    }

    if (playerActive && !play) 
    {
        if (initSeason == 0) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade((Color){216, 200, 39, 255}, 0.4));
        else if (initSeason == 1) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade((Color){155, 70, 22, 255}, 0.4));
        else if (initSeason == 2) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade((Color){17, 129, 162, 255}, 0.4));
        else DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade((Color){82, 174, 7, 255}, 0.4));
    }

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (snowParticle.particles[i].active) DrawTexturePro(atlas01, particle_icecrystal,
                                               (Rectangle){ snowParticle.particles[i].position.x, snowParticle.particles[i].position.y, particle_icecrystal.width*snowParticle.particles[i].size, particle_icecrystal.height*snowParticle.particles[i].size },
                                               (Vector2){ particle_icecrystal.width*snowParticle.particles[i].size/2, particle_icecrystal.height*snowParticle.particles[i].size/2 }, snowParticle.particles[i].rotation,
                                               Fade(snowParticle.particles[i].color, snowParticle.particles[i].alpha));

        if (planetreeParticle.particles[i].active) DrawTexturePro(atlas01, particle_planetreeleaf,
                                               (Rectangle){ planetreeParticle.particles[i].position.x, planetreeParticle.particles[i].position.y, particle_planetreeleaf.width*planetreeParticle.particles[i].size, particle_planetreeleaf.height*planetreeParticle.particles[i].size },
                                               (Vector2){ particle_planetreeleaf.width*planetreeParticle.particles[i].size/2, particle_planetreeleaf.height*planetreeParticle.particles[i].size/2 }, planetreeParticle.particles[i].rotation,
                                               Fade(planetreeParticle.particles[i].color, planetreeParticle.particles[i].alpha));

        if (dandelionParticle.particles[i].active) DrawTexturePro(atlas01, particle_dandelion,
                                               (Rectangle){ dandelionParticle.particles[i].position.x, dandelionParticle.particles[i].position.y, particle_dandelion.width*dandelionParticle.particles[i].size, particle_dandelion.height*dandelionParticle.particles[i].size },
                                               (Vector2){ particle_dandelion.width*dandelionParticle.particles[i].size/2, particle_dandelion.height*dandelionParticle.particles[i].size/2 }, dandelionParticle.particles[i].rotation,
                                               Fade(dandelionParticle.particles[i].color, dandelionParticle.particles[i].alpha));

        if (flowerParticle.particles[i].active) DrawTexturePro(atlas01, particle_ecualyptusflower,
                                               (Rectangle){ flowerParticle.particles[i].position.x, flowerParticle.particles[i].position.y, particle_ecualyptusflower.width*flowerParticle.particles[i].size, particle_ecualyptusflower.height*flowerParticle.particles[i].size },
                                               (Vector2){ particle_ecualyptusflower.width*flowerParticle.particles[i].size/2, particle_ecualyptusflower.height*flowerParticle.particles[i].size/2 }, flowerParticle.particles[i].rotation,
                                               Fade(flowerParticle.particles[i].color, flowerParticle.particles[i].alpha));

        if (rainParticle.particles[i].active) DrawTexturePro(atlas01, particle_waterdrop,
                                               (Rectangle){ rainParticle.particles[i].position.x, rainParticle.particles[i].position.y, particle_waterdrop.width*rainParticle.particles[i].size, particle_waterdrop.height*rainParticle.particles[i].size },
                                               (Vector2){ particle_waterdrop.width*rainParticle.particles[i].size/2, particle_waterdrop.height*rainParticle.particles[i].size/2 }, rainParticle.particles[i].rotation,
                                               Fade(rainParticle.particles[i].color, rainParticle.particles[i].alpha));
    }
    
    // Draw Speed Particles
    for (int i = 0; i < MAX_PARTICLES_SPEED; i++)
    {
       if (speedFX.particle[i].active) DrawRectangle(speedFX.particle[i].position.x, speedFX.particle[i].position.y, speedFX.particle[i].size.x, speedFX.particle[i].size.y , Fade(WHITE, speedFX.particle[i].alpha));
    }

    for (int i = 0; i < MAX_PARTICLES_STORM; i++)
    {
        if (rainStormParticle.particles[i].active) DrawTexturePro(atlas01, particle_waterdrop,
                                               (Rectangle){ rainStormParticle.particles[i].position.x, rainStormParticle.particles[i].position.y, particle_waterdrop.width*rainStormParticle.particles[i].size, particle_waterdrop.height*rainStormParticle.particles[i].size },
                                               (Vector2){ particle_waterdrop.width*rainStormParticle.particles[i].size/2, particle_waterdrop.height*rainStormParticle.particles[i].size/2 }, rainStormParticle.particles[i].rotation,
                                               Fade(rainStormParticle.particles[i].color, rainStormParticle.particles[i].alpha));
    }

    for (int i = 0; i < MAX_PARTICLES_STORM; i++)
    {
        if (snowStormParticle.particles[i].active) DrawTexturePro(atlas01, particle_icecrystal,
                                               (Rectangle){ snowStormParticle.particles[i].position.x, snowStormParticle.particles[i].position.y, particle_icecrystal.width*snowStormParticle.particles[i].size, particle_icecrystal.height*snowStormParticle.particles[i].size },
                                               (Vector2){ particle_icecrystal.width*snowStormParticle.particles[i].size/2, particle_icecrystal.height*snowStormParticle.particles[i].size/2 }, snowStormParticle.particles[i].rotation,
                                               Fade(snowStormParticle.particles[i].color, snowStormParticle.particles[i].alpha));
    }

    for (int i = 0; i < MAX_PARTICLES_RAY; i++)
    {
       if (rayParticles.particles[i].active) DrawTexturePro(atlas01, gameplay_fx_lightraymid,
                                               (Rectangle){ rayParticles.particles[i].position.x, rayParticles.particles[i].position.y, gameplay_fx_lightraymid.width*rayParticles.particles[i].size, gameplay_fx_lightraymid.height*rayParticles.particles[i].size },
                                               (Vector2){ gameplay_fx_lightraymid.width*rayParticles.particles[i].size/2, gameplay_fx_lightraymid.height*rayParticles.particles[i].size/2 }, rayParticles.particles[i].rotation,
                                               Fade(rayParticles.particles[i].color, rayParticles.particles[i].alpha));
    }

    if (fogAlpha != 0)
    {
        DrawTexturePro(atlas02, background_fog02, (Rectangle){ fogPosition, GetScreenHeight()*0.6, GetScreenWidth(), background_fog02.height}, (Vector2){ 0 , 0 }, 0, Fade(WHITE, fogAlpha));
        DrawTexturePro(atlas02, background_fog02, (Rectangle){ fogPosition+GetScreenWidth(), GetScreenHeight()*0.6, GetScreenWidth(), background_fog02.height}, (Vector2){ 0 , 0 }, 0, Fade(WHITE, fogAlpha));
    }
    
    if (filterAlpha != 0 && state != FINALFORM) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(SKYBLUE, filterAlpha));

    DrawTexturePro(atlas01, gameplay_gui_leafcounter_base, (Rectangle){ 0, 0, gameplay_gui_leafcounter_base.width, gameplay_gui_leafcounter_base.height}, (Vector2){ 0 , 0 }, 0, WHITE);

    DrawTexturePro(atlas01, gameplay_gui_seasonsclock_disc, (Rectangle) {GetScreenWidth(), 0, gameplay_gui_seasonsclock_disc.width, gameplay_gui_seasonsclock_disc.height}, (Vector2) {gameplay_gui_seasonsclock_disc.width/2, gameplay_gui_seasonsclock_disc.height/2},     // Draw a part of a texture defined by a rectangle with 'pro' parameters
                    clockRotation, Fade(WHITE, UIfade));

    DrawTexturePro(atlas01, gameplay_gui_seasonsclock_base, (Rectangle){ (GetScreenWidth() - gameplay_gui_seasonsclock_base.width ), 0, gameplay_gui_seasonsclock_base.width, gameplay_gui_seasonsclock_base.height}, (Vector2){ 0 , 0 }, 0, Fade(WHITE, UIfade));
    
    for (int i = 0; i < 20; i++)
    {
        if (((currentLeaves/5) > i) && (state != FINALFORM)) DrawTexturePro(atlas01, gameplay_gui_leafcounter_cell, (Rectangle) {87, 83, gameplay_gui_leafcounter_cell.width, gameplay_gui_leafcounter_cell.height}, (Vector2) {gameplay_gui_leafcounter_cell.width/4, 69}, i*(-18), WHITE);
        else if ((power/18 >= i) && (state == FINALFORM)) DrawTexturePro(atlas01, gameplay_gui_leafcounter_cell, (Rectangle) {87, 83, gameplay_gui_leafcounter_cell.width, gameplay_gui_leafcounter_cell.height}, (Vector2) {gameplay_gui_leafcounter_cell.width/4, 69}, i*(-18), WHITE);
    }

    if ((currentLeaves >= LEAVESTOTRANSFORM) && (state != FINALFORM)) 
    {
        DrawTexturePro(atlas01, gameplay_gui_leafcounter_pulsel, 
                      (Rectangle){ 85, 84, gameplay_gui_leafcounter_pulsel.width*leafGUIpulseScale, gameplay_gui_leafcounter_pulsel.height*leafGUIpulseScale}, 
                      (Vector2){ gameplay_gui_leafcounter_pulsel.width*leafGUIpulseScale/2 , gameplay_gui_leafcounter_pulsel.height*leafGUIpulseScale/2 }, 0, Fade((Color){126, 248, 25, 255}, leafGUIpulseFade));
                      
        DrawTexturePro(atlas01, gameplay_gui_leafcounter_glow, 
                        (Rectangle){ 84, 83, gameplay_gui_leafcounter_glow.width, gameplay_gui_leafcounter_glow.height}, 
                        (Vector2){ gameplay_gui_leafcounter_glow.width/2 , gameplay_gui_leafcounter_glow.height/2 }, 0, Fade(WHITE, leafGUIglowFade));
    }
    
    if ((play == false) && playerActive)
    {
        if (startNum == 3) DrawTexturePro(atlas01, gameplay_countdown_3, 
                      (Rectangle){ GetScreenWidth()/2, GetScreenHeight()/2, gameplay_countdown_3.width*numberScale, gameplay_countdown_3.height*numberScale}, 
                      (Vector2){ gameplay_countdown_3.width*numberScale/2 , gameplay_countdown_3.height*numberScale/2 }, 0, Fade(RED, numberAlpha));
        else if (startNum == 2) DrawTexturePro(atlas01, gameplay_countdown_2, 
                      (Rectangle){ GetScreenWidth()/2, GetScreenHeight()/2, gameplay_countdown_2.width*numberScale, gameplay_countdown_2.height*numberScale}, 
                      (Vector2){ gameplay_countdown_2.width*numberScale/2 , gameplay_countdown_2.height*numberScale/2 }, 0, Fade(RED, leafGUIpulseFade));
        else if (startNum == 1) DrawTexturePro(atlas01, gameplay_countdown_1, 
                      (Rectangle){ GetScreenWidth()/2, GetScreenHeight()/2, gameplay_countdown_1.width*numberScale, gameplay_countdown_1.height*numberScale}, 
                      (Vector2){ gameplay_countdown_1.width*numberScale/2 , gameplay_countdown_1.height*numberScale/2 }, 0, Fade(RED, leafGUIpulseFade));
    }        

    // Draw text elements
    //--------------------------
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (popupScore[i].active)
        {
            DrawTextEx(font, FormatText("%i", popupScore[i].score), popupScore[i].position, font.baseSize/4*popupScore[i].scale, -5, Fade((Color){255, 73, 73, 255}, popupScore[i].alpha));
        }
    }
    
    if (popupBee.active) DrawTextEx(font, FormatText("%i", popupBee.score), popupBee.position, font.baseSize/4*popupBee.scale, -5, Fade((Color){255, 73, 73, 255}, popupBee.alpha));
    if (popupEagle.active) DrawTextEx(font, FormatText("%i", popupEagle.score), popupEagle.position, font.baseSize/4*popupEagle.scale, -5, Fade((Color){255, 73, 73, 255}, popupEagle.alpha));

    for (int i = 0; i < MAX_LEAVES; i++)
    {
        if (popupLeaves[i].active) DrawTextEx(font, FormatText("+ %i", popupLeaves[i].score), popupLeaves[i].position, font.baseSize/4*popupLeaves[i].scale, -5, Fade((Color){139, 179, 0, 255}, popupLeaves[i].alpha));
    }
    

    DrawTextEx(font, FormatText("%03i", currentLeaves), (Vector2){ 47, 50 }, font.baseSize, -8, counterColor);

    if (transforming) DrawTextEx(font, textFinalForm, (Vector2){ GetScreenWidth()/2 - MeasureText(textFinalForm, 40)/2, GetScreenHeight()/4}, font.baseSize, -5, (Color){246, 133, 133, 255});
    
    if ((currentMonth == 7) && (transitionFramesCounter >= SEASONTRANSITION/2))
    {
        if (randomMessage <= 4) DrawTextEx(font, textSpring1, (Vector2){GetScreenWidth()/2 - MeasureText(textSpring1, 40)/2, GetScreenHeight()/3}, font.baseSize, -5, (Color){185, 222, 105, 255});
        else DrawTextEx(font, textSpring2, (Vector2){GetScreenWidth()/2 - MeasureText(textSpring2, 40)/2, GetScreenHeight()/3}, font.baseSize, -5, (Color){185, 222, 105, 255});
    }      
    else if ((currentMonth == 10) && (transitionFramesCounter >= SEASONTRANSITION/2))
    {
        if (randomMessage <= 4) DrawTextEx(font, textSummer1, (Vector2){GetScreenWidth()/2 - MeasureText(textSummer1, 40)/2, GetScreenHeight()/3}, font.baseSize, -5, (Color){253, 200, 108, 255}); 
        else DrawTextEx(font, textSummer2, (Vector2){GetScreenWidth()/2 - MeasureText(textSummer2, 40)/2, GetScreenHeight()/3}, font.baseSize, -5, (Color){253, 200, 108, 255}); 
    }    
    else if ((currentMonth == 1) && (transitionFramesCounter >= SEASONTRANSITION/2))
    {
        if (randomMessage <= 4) DrawTextEx(font, textFall1, (Vector2){GetScreenWidth()/2 - MeasureText(textFall1, 40)/2, GetScreenHeight()/3}, font.baseSize, -5, (Color){255, 149, 107, 255}); 
        else DrawTextEx(font, textFall2, (Vector2){GetScreenWidth()/2 - MeasureText(textFall2, 40)/2, GetScreenHeight()/3}, font.baseSize, -5, (Color){255, 149, 107, 255}); 
    }    
    else if (currentMonth == 4 && transitionFramesCounter >= SEASONTRANSITION/2) 
    {
        if (randomMessage <= 4) DrawTextEx(font, textWinter1, (Vector2){GetScreenWidth()/2 - MeasureText(textWinter1, 40)/2, GetScreenHeight()/3}, font.baseSize, -5, (Color){133, 249, 253, 255});
        else DrawTextEx(font, textWinter2, (Vector2){GetScreenWidth()/2 - MeasureText(textWinter2, 40)/2, GetScreenHeight()/3}, font.baseSize, -5, (Color){133, 249, 253, 255});
    }
    
#if defined(DEBUG)
    DrawRectangle(player.x, player.y, player.width, player.height, Fade(WHITE, 0.5));
    
    for (int i = 0; i < MAX_WIND; i++)
    {
        if (windActive[i]) DrawRectangleRec(wind[i], Fade (GRAY, 0.4));
    }
    
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (owlActive[i]) DrawRectangleRec(owl[i], Fade(BLACK, 0.5f));
        if (dingoActive[i]) DrawRectangleRec(dingo[i], Fade(BLACK, 0.5f));
        if (snakeActive[i]) DrawRectangleRec(snake[i], BLACK);
    }
    
    if (beeActive) DrawRectangleRec(bee, Fade(BLACK, 0.5f));
    if (eagleActive) DrawRectangleRec(eagle, Fade(BLACK, 0.5f));

    switch (season)
    {
        case WINTER:
        {
            if (currentMonth == 5) DrawText("June", GetScreenWidth() - 140, GetScreenHeight() - 20, 20, RED);
            if (currentMonth == 6) DrawText("July", GetScreenWidth() - 140, GetScreenHeight() - 20, 20, RED);

        } break;
        case SPRING:
        {
            if (currentMonth == 8) DrawText("September", GetScreenWidth() - 140, GetScreenHeight() - 20, 20, RED);
            if (currentMonth == 9) DrawText("October", GetScreenWidth() - 140, GetScreenHeight() - 20, 20, RED);

        } break;
        case SUMMER:
        {
            if (currentMonth == 11) DrawText("December", GetScreenWidth() - 140, GetScreenHeight() - 20, 20, RED);
            if (currentMonth == 0) DrawText("January", GetScreenWidth() - 140, GetScreenHeight() - 20, 20, RED);

        } break;
        case FALL:
        {
            if (currentMonth == 2) DrawText("March", GetScreenWidth() - 140, GetScreenHeight() - 20, 20, RED);
            if (currentMonth == 3) DrawText("April", GetScreenWidth() - 140, GetScreenHeight() - 20, 20, RED);

        } break;
        case TRANSITION:
        {
            if (currentMonth == 4) DrawText("May", GetScreenWidth() - 140, GetScreenHeight() - 20, 20, RED);
            
        } break;
        default: break;
    }
    
    DrawText(FormatText("Score: %02i", score), 140, GetScreenHeight() - 20, 20, RED);
    DrawText(FormatText("HighScore: %02i", hiscore), 600, GetScreenHeight() - 20, 20, RED);
    DrawText(FormatText("SeasonChange: %03i", seasonTimer), 300, GetScreenHeight() - 20, 20, RED);
#endif
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    // ...
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}

// Tree Spawn
static void BambooSpawn(void)
{
    int counter = 0;

    for (int k = 0; k < MAX_ENEMIES; k++)
    {
        if ((!bambooActive[k]) && (counter < 1))
        {
            bamboo[k].y = 0;
            bamboo[k].x = GetScreenWidth();
            bambooActive[k] = true;
            counter++;
        }
    }
}

//Snake Spawn
static void SnakeSpawn(int chance)
{
    int position;
    int counter = 0;

    for (int z = 0; z < 2; z++) posArray[z] = -1;

    if (GetRandomValue(0, 100) <= chance)
    {
        for (int k = 0; k < MAX_ENEMIES; k++)
        {
            if ((!snakeActive[k]) && (counter < 1))
            {
                position = GetRandomValue(0, 4);
                
                if (counter == 0) posArray[counter] = position;

                snake[k].x = GetScreenWidth() - 15;
                snake[k].y = 25 + GetScreenHeight()/5*position;
                snakeActive[k] = true;
                isHitSnake[k] = false;
                counter++;
            }
        }
    }
}

// Dingo Spawn
static void DingoSpawn(int chance)
{
    int position;
    int counter = 0;

    for (int z = 0; z < 2; z++) posArrayDingo[z] = -1;

    if (GetRandomValue(0, 100) <= chance)
    {
        for(int k = 0; k < MAX_ENEMIES; k++)
        {
            if ((!dingoActive[k]) && (counter < 1))
            {
                position = GetRandomValue(1, 3);
                
                if (counter == 0) posArray[counter] = position;

                dingo[k].x = GetScreenWidth() - 15;
                dingo[k].y = 25 + GetScreenHeight()/5*position;
                dingoActive[k] = true;
                isHitDingo[k] = false;
                counter++;
            }
        }
    }
}

// Owl Spawn
static void OwlSpawn(int chance)
{
    int position;
    int counter = 0;

    for (int z = 0; z < 2; z++) posArray[z] = -1;

    if (GetRandomValue(0, 100) <= chance)
    {
        for (int k = 0; k < MAX_ENEMIES; k++)
        {
            if ((!owlActive[k]) && (!branchActive[k]) && (counter < 1))
            {
                position = GetRandomValue(1, 3);

                if (counter == 0) posArray[counter] = position;

                owl[k].x = GetScreenWidth() - 15;
                owl[k].y = 25 + GetScreenHeight()/5*position;
                owlActive[k] = true;
                branchPos[k].x = owl[k].x;
                branchPos[k].y = owl[k].y + 64;
                branchActive[k] = true;
                counter++;
            }
        }
    }
}

// Leaf spawn function
static void LeafSpawn(void)
{
    int counter = 0;
    int maxLeavesCounter = GetRandomValue(0, 2);
    
    for (int z = 0; z < 2; z++) posArrayLeaf[z] = -1;

    for (int k = 0; k < MAX_LEAVES; k++)
    {
        if ((!leafActive[k]) && (counter <= maxLeavesCounter))
        {
            int leafPosition = GetRandomValue(0, 4);
            int leafTypeSelection = GetRandomValue(0,99);
            int leafSideSelection = GetRandomValue(0,1);
            leafSide[k] = leafSideSelection;

            if (counter == 0)
            {
                while (CheckArrayValue(posArray, 2, leafPosition)) leafPosition = GetRandomValue(0, 4); 
                posArrayLeaf[counter] =  leafPosition;
            }
            else if (counter == 1)
            {
                while(leafPosition == posArrayLeaf[counter - 1] || CheckArrayValue(posArray, 2, leafPosition)) leafPosition = GetRandomValue(0, 4);
                posArrayLeaf[counter] =  leafPosition;
            }
            else if (counter == 2)
            {
                while((leafPosition == posArrayLeaf[counter - 1] || (leafPosition == posArrayLeaf[counter - 2])) || CheckArrayValue(posArray, 2, leafPosition)) leafPosition = GetRandomValue(0, 4);
                posArrayLeaf[counter] =  leafPosition;
            }

            leaf[k].y = 30 + GetScreenHeight()/5*leafPosition;
            leaf[k].x = GetScreenWidth() - 18;
            leafActive[k] = true;
            
            if (leafTypeSelection <= 24) leafType[k] = 0;
            else if ((leafTypeSelection > 24) && leafTypeSelection <= 50) leafType[k] = 1;
            else if ((leafTypeSelection > 50) && leafTypeSelection <= 75) leafType[k] = 2;
            else leafType[k] = 3;
            
            counter++;
        }
    }
}

static void FireSpawn(int chance)
{
    int counter = 0;

    if (GetRandomValue(0, 100) <= chance)
    {
        for (int k = 0; k < MAX_FIRE; k++)
        {
            if ((!fireActive[k]) && (counter < 1))
            {
                fire[k].y = GetScreenHeight() - 30;
                fire[k].x = GetScreenWidth() - 5;
                //fire[k].height = 30;
                fireActive[k] = true;
                onFire[k] = false;
                counter++;
            }
        }
    }
}

static void IceSpawn(int chance)
{
    int counter = 0;

    if (GetRandomValue(0, 100) <= chance)
    {
        for (int k = 0; k < MAX_ICE; k++)
        {
            if ((!iceActive[k]) && (counter < 1))
            {
                ice[k].y = 0;
                ice[k].x = GetScreenWidth() + 5;
                iceActive[k] = true;
                counter++;
            }
        }
    }
}

static void ResinSpawn(int chance)
{
    int counter = 0;

    if (GetRandomValue(0, 100) <= chance)
    {
        for (int k = 0; k < MAX_RESIN; k++)
        {
            if ((!resinActive[k]) && (counter < 1))
            {
                int resPosition = GetRandomValue(0, 4);
                
                while (CheckArrayValue(posArray, 2, resPosition)) resPosition = GetRandomValue(0, 4);
                
                resin[k].y = 25 + GetScreenHeight()/5*resPosition;
                resin[k].x = GetScreenWidth() + 5;
                resinActive[k] = true;
                counter++;
            }
        }
    }
}

static void WindSpawn(int chance)
{
    int counter = 0;

    if (GetRandomValue(0, 100) <= chance)
    {
        for (int k = 0; k < MAX_WIND ; k++)
        {
            if ((!windActive[k]) && (counter < 1))
            {
                int resPosition = GetRandomValue(0, 4);
                
                while (CheckArrayValue(posArray, 2, resPosition)) resPosition = GetRandomValue(0, 4);
                
                wind[k].y = 25 + GetScreenHeight()/5*resPosition;
                wind[k].x = GetScreenWidth() + 5;
                windActive[k] = true;
                counter++;
            }
        }
    }
}

// Spawn bee enemy
static void BeeSpawn(int chance)
{
    if ((GetRandomValue(0, 100) <= chance) && !beeActive && !alertBeeActive)
    {
        bee.x = GetScreenWidth();
        bee.y = GetRandomValue(40, GetScreenHeight() - bee.height - 40);
        beeAlertRectangle = (Rectangle){GetScreenWidth(), bee.y + gameplay_enemy_bee.height/2, 0, 0};
        beeActive = false;
        alertBeeActive = true;
    }
}

// Spawn eagle enemy
static void EagleSpawn(int chance)
{
    if ((GetRandomValue(0, 100) <= chance) && !eagleActive && !alertActive)
    {
        eagleDelay = 0;
        eagle.x = GetScreenWidth();
        eagle.y = player.y;
        alertRectangle = (Rectangle){GetScreenWidth(), eagle.y + gameplay_enemy_eagle.height/2, 0, 0};
        eagleActive = false;
        eagleAlert = true;
        alertActive = true;
    }
}

// Check if the array contains a value
static bool CheckArrayValue(int *array, int arrayLength, int value)
{
    for (int n = 0; n < arrayLength; n++)
    {
        if (array[n] == value) return 1;
    }

    return 0;
}

// Scroll functions
// Front parallax drawing
static void DrawParallaxFront(void)
{
    Rectangle ground01 = gameplay_back_ground01;

    DrawTexturePro(atlas02, gameplay_back_tree01_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset, 60, gameplay_back_tree01_layer01.width*2, gameplay_back_tree01_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree02_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + 140, 60, gameplay_back_tree02_layer01.width*2, gameplay_back_tree02_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree03_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + 140*2, 55, gameplay_back_tree02_layer01.width*2, gameplay_back_tree02_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree04_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + 140*3, 60, gameplay_back_tree04_layer01.width*2, gameplay_back_tree04_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree05_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + 140*4, 60, gameplay_back_tree05_layer01.width*2, gameplay_back_tree05_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree06_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + 140*5, 55, gameplay_back_tree06_layer01.width*2, gameplay_back_tree06_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree07_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + 140*6, 60, gameplay_back_tree07_layer01.width*2, gameplay_back_tree07_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree08_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + 140*7, 60, gameplay_back_tree08_layer01.width*2, gameplay_back_tree08_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_ground01, (Rectangle){(int)scrollFront, 559, ground01.width*2, ground01.height*2}, (Vector2){0,0}, 0, color01);
    DrawTexturePro(atlas02, (Rectangle){ground01.x, ground01.y + ground01.height, ground01.width, -ground01.height}, (Rectangle){(int)scrollFront, -33, ground01.width*2, ground01.height*2}, (Vector2){0,0}, 0, color01);

    DrawTexturePro(atlas02, gameplay_back_tree01_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + GetScreenWidth(), 60, gameplay_back_tree01_layer01.width*2, gameplay_back_tree01_layer01.height*2},(Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree02_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + GetScreenWidth() + 140, 60, gameplay_back_tree02_layer01.width*2, gameplay_back_tree02_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree03_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + GetScreenWidth() + 140*2, 55, gameplay_back_tree03_layer01.width*2, gameplay_back_tree03_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree04_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + GetScreenWidth() + 140*3, 60, gameplay_back_tree04_layer01.width*2, gameplay_back_tree04_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree05_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + GetScreenWidth() + 140*4, 60, gameplay_back_tree05_layer01.width*2, gameplay_back_tree05_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree06_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + GetScreenWidth() + 140*5, 55, gameplay_back_tree06_layer01.width*2, gameplay_back_tree06_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree07_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset + GetScreenWidth() + 140*6, 60, gameplay_back_tree07_layer01.width*2, gameplay_back_tree07_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree08_layer01, (Rectangle){(int)scrollFront + parallaxFrontOffset+ GetScreenWidth() + 140*7, 60, gameplay_back_tree08_layer01.width*2, gameplay_back_tree08_layer01.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_ground01, (Rectangle){(int)scrollFront + GetScreenWidth(), 559, ground01.width*2, ground01.height*2}, (Vector2){0,0}, 0, color01); 
    DrawTexturePro(atlas02, (Rectangle){ground01.x, ground01.y + ground01.height, ground01.width, -ground01.height}, (Rectangle){(int)scrollFront+ GetScreenWidth(), -33, ground01.width*2, ground01.height*2}, (Vector2){0,0}, 0, color01);
}

// Middle parallax drawing
static void DrawParallaxMiddle(void)
{
    Rectangle ground02 = gameplay_back_ground02;

    DrawTexturePro(atlas02, gameplay_back_tree01_layer02, (Rectangle){(int)scrollMiddle, 67, gameplay_back_tree01_layer02.width*2, gameplay_back_tree01_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree02_layer02, (Rectangle){(int)scrollMiddle + 140, 67, gameplay_back_tree02_layer02.width*2, gameplay_back_tree02_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree03_layer02, (Rectangle){(int)scrollMiddle + 140*2, 67, gameplay_back_tree03_layer02.width*2, gameplay_back_tree03_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree04_layer02, (Rectangle){(int)scrollMiddle + 140*3, 67, gameplay_back_tree04_layer02.width*2, gameplay_back_tree04_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree05_layer02, (Rectangle){(int)scrollMiddle + 140*4, 67, gameplay_back_tree05_layer02.width*2, gameplay_back_tree05_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree06_layer02, (Rectangle){(int)scrollMiddle + 140*5, 67, gameplay_back_tree06_layer02.width*2, gameplay_back_tree06_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree07_layer02, (Rectangle){(int)scrollMiddle + 140*6, 67, gameplay_back_tree07_layer02.width*2, gameplay_back_tree07_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree08_layer02, (Rectangle){(int)scrollMiddle + 140*7, 67, gameplay_back_tree08_layer02.width*2, gameplay_back_tree08_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_ground02, (Rectangle){(int)scrollMiddle, 509, ground02.width*2, ground02.height*2}, (Vector2){0,0}, 0, color01);
    DrawTexturePro(atlas02, (Rectangle){ground02.x, ground02.y + ground02.height, ground02.width, -ground02.height}, (Rectangle){(int)scrollMiddle, 19, ground02.width*2, ground02.height*2}, (Vector2){0,0}, 0, color01);

    DrawTexturePro(atlas02, gameplay_back_tree02_layer02, (Rectangle){(int)scrollMiddle + GetScreenWidth() + 140, 67, gameplay_back_tree02_layer02.width*2, gameplay_back_tree02_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree03_layer02, (Rectangle){(int)scrollMiddle + GetScreenWidth() + 140*2, 67, gameplay_back_tree03_layer02.width*2, gameplay_back_tree03_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree04_layer02, (Rectangle){(int)scrollMiddle + GetScreenWidth() + 140*3, 67, gameplay_back_tree04_layer02.width*2, gameplay_back_tree04_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree05_layer02, (Rectangle){(int)scrollMiddle + GetScreenWidth() + 140*4, 67, gameplay_back_tree05_layer02.width*2, gameplay_back_tree05_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree06_layer02, (Rectangle){(int)scrollMiddle + GetScreenWidth() + 140*5, 67, gameplay_back_tree06_layer02.width*2, gameplay_back_tree06_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree07_layer02, (Rectangle){(int)scrollMiddle + GetScreenWidth() + 140*6, 67, gameplay_back_tree07_layer02.width*2, gameplay_back_tree07_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree08_layer02, (Rectangle){(int)scrollMiddle + GetScreenWidth() + 140*7, 67, gameplay_back_tree08_layer02.width*2, gameplay_back_tree08_layer02.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree01_layer02, (Rectangle){(int)scrollMiddle+ GetScreenWidth(), 67, gameplay_back_tree01_layer02.width*2, gameplay_back_tree01_layer02.height*2},(Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_ground02, (Rectangle){(int)scrollMiddle + GetScreenWidth(), 509, ground02.width*2, ground02.height*2},(Vector2){0,0}, 0, color01);
    DrawTexturePro(atlas02, (Rectangle){ground02.x, ground02.y + ground02.height, ground02.width, -ground02.height}, (Rectangle){(int)scrollMiddle+ GetScreenWidth(), 19, ground02.width*2, ground02.height*2}, (Vector2){0,0}, 0, color01);
}

// Back parallax drawing
static void DrawParallaxBack(void)
{
    Rectangle ground03 = gameplay_back_ground03;

    DrawTexturePro(atlas02, gameplay_back_tree01_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset, 67, gameplay_back_tree01_layer03.width*2, gameplay_back_tree01_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree02_layer03, (Rectangle){(int)scrollBack  + parallaxBackOffset + 140, 67, gameplay_back_tree02_layer03.width*2, gameplay_back_tree02_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree03_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + 140*2, 67, gameplay_back_tree03_layer03.width*2, gameplay_back_tree03_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree04_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + 140*3, 67, gameplay_back_tree04_layer03.width*2, gameplay_back_tree04_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree05_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + 140*4, 67, gameplay_back_tree05_layer03.width*2, gameplay_back_tree05_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree06_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + 140*5, 67, gameplay_back_tree06_layer03.width*2, gameplay_back_tree06_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree07_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + 140*6, 67, gameplay_back_tree07_layer03.width*2, gameplay_back_tree07_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree08_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + 140*7, 67, gameplay_back_tree08_layer03.width*2, gameplay_back_tree08_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_ground03, (Rectangle){(int)scrollBack, 469, ground03.width*2, ground03.height*2}, (Vector2){0,0}, 0, color01);
    DrawTexturePro(atlas02, (Rectangle){ground03.x, ground03.y + ground03.height, ground03.width, -ground03.height}, (Rectangle){(int)scrollBack, 67, ground03.width*2, ground03.height*2}, (Vector2){0,0}, 0, color01);

    DrawTexturePro(atlas02, gameplay_back_tree01_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset+ GetScreenWidth(), 67, gameplay_back_tree01_layer03.width*2, gameplay_back_tree01_layer03.height*2},(Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree02_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + GetScreenWidth() + 140, 67, gameplay_back_tree02_layer03.width*2, gameplay_back_tree02_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree03_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + GetScreenWidth() + 140*2, 67, gameplay_back_tree03_layer03.width*2, gameplay_back_tree03_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree04_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + GetScreenWidth() + 140*3, 67, gameplay_back_tree04_layer03.width*2, gameplay_back_tree04_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree05_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + GetScreenWidth() + 140*4, 67, gameplay_back_tree05_layer03.width*2, gameplay_back_tree05_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree06_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + GetScreenWidth() + 140*5, 67, gameplay_back_tree06_layer03.width*2, gameplay_back_tree06_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree07_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + GetScreenWidth() + 140*6, 67, gameplay_back_tree07_layer03.width*2, gameplay_back_tree07_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_tree08_layer03, (Rectangle){(int)scrollBack + parallaxBackOffset + GetScreenWidth() + 140*7, 67, gameplay_back_tree08_layer03.width*2, gameplay_back_tree08_layer03.height*2}, (Vector2){0,0}, 0, color02);
    DrawTexturePro(atlas02, gameplay_back_ground03, (Rectangle){(int)scrollBack + GetScreenWidth(), 469, ground03.width*2, ground03.height*2}, (Vector2){0,0}, 0, color01);
    DrawTexturePro(atlas02, (Rectangle){ground03.x, ground03.y + ground03.height, ground03.width, -ground03.height}, (Rectangle){(int)scrollBack+ GetScreenWidth(), 67, ground03.width*2, ground03.height*2}, (Vector2){0,0}, 0, color01);
}

// Linear easing animation
static float LinearEaseIn(float t, float b, float c, float d) { return c*t/d + b; }

// Transition from one color to another
static Color ColorTransition(Color initialColor, Color finalColor, int framesCounter)
{
    Color currentColor;

    currentColor.r = (unsigned char)LinearEaseIn((float)framesCounter, (float)initialColor.r, (float)(finalColor.r - initialColor.r), (float)(SEASONTRANSITION));
    currentColor.g = (unsigned char)LinearEaseIn((float)framesCounter, (float)initialColor.g, (float)(finalColor.g - initialColor.g), (float)(SEASONTRANSITION));
    currentColor.b = (unsigned char)LinearEaseIn((float)framesCounter, (float)initialColor.b, (float)(finalColor.b - initialColor.b), (float)(SEASONTRANSITION));
    currentColor.a = 255;
    
    return currentColor;
}

static void Reset(void)
{
    framesCounter = 0;
    finishScreen = 0;
    grabCounter = 10;
    bambooTimer = 0;
    bambooSpawnTime = 0;
    gravity = 5;
    speed = 3;
    score = 0;
    hiscore = 0;
    power = 360;
    resinCount = 0;
    rightAlpha = 0.5;
    leftAlpha = 0.5;
    speedMod = 1.2f;
    transCount = 0;
    windCounter = 0;
    maxPower = 360;
    playerActive = true;
    scrollFront = 0;
    scrollMiddle = 0;
    scrollBack = 0;
    scrollSpeed = 1.6f*TIME_FACTOR;
    groundPos = 0;
    resinCountjump = 0;
    resinCountdrag = 0;
    colorTimer = 0;
    play = false;
    onIce = false;
    onResin = false;
    jumpSpeed = 6;
    transforming = false;
    eagleAlert = false;
    alertActive = false;
    fireSpeed = 4;
    seasonTimer = 0;
    seasonChange = SEASONCHANGE;
    monthChange = seasonChange/3;
    glowing = true;
    currentFrame = 0;
    curFrame = 0;
    curFrame1 = 1;
    curFrame2 = 2;
    curFrame3 = 3;
    curFrameBee = 0;
    fireOffset = 20;
    beeMov = 0;
    killCounter = 0;
    currentLeaves = 0;
    clockRotation = 0;
    flyColor = GRAY;
    globalFrameCounter = 0;
    startCounter = 0;
    numberAlpha = 1;
    numberScale = 2.5f;
    startNum = 3;
    animCounter = 0;
    finalFormEnd = 0;
    randomMessage = 0;
    years = 0;
    UIfade = 1;
    fogAlpha = 0;
    seasons = 0;
    fog = false;
    clockSpeedRotation = 0; 
    eagleDelay = 0;
    
    parallaxBackOffset = GetRandomValue (10, 100);
    parallaxFrontOffset = GetRandomValue (100, 200);
    
    progresionDelay = 0;
    progresionFramesCounter = 0;
    speedProgresion = 0;
    
    jumpCounter = 0;
    resinCounter = 0;
    tornadoCounter = 0;
    dashCounter = 0;
    superKoalaCounter = 0;
    
    fogSpeed = 2;
    
    leafGUIglow = true;
    leafGUIglowFade = 0;
    leafGUIpulseFade = 1;
    leafGUIpulseScale = 1;

    //initMonth = ptm->tm_mon;
    initYears = 1900 + ptm->tm_year;

    //initMonth = STARTINGMONTH;

    if (initSeason == 0)
    {
        initMonth  = 11;
        clockRotation = 225;
        clockInitRotation = 225;
        clockFinalRotation = clockInitRotation + 90;
        color00 = (Color){129, 172, 86, 255};              // Summer Color
        color01 = (Color){145, 165, 125, 255};
        color02 = (Color){161, 130, 73, 255};
        color03 = (Color){198, 103, 51, 255};
    }
    else if (initSeason == 1)
    {
        initMonth = 2;
        clockRotation = 315;
        clockInitRotation = 315;
        clockFinalRotation = clockInitRotation + 90;
        color00 = (Color){242, 113, 62, 255};              // Fall Color
        color01 = (Color){190, 135, 114, 255};
        color02 = (Color){144, 130, 101, 255};
        color03 = (Color){214, 133, 58, 255};
    }
    else if (initSeason == 2)
    {
        initMonth = 5;
        clockRotation = 45;
        clockInitRotation = 45;
        clockFinalRotation = clockInitRotation + 90;
        color00 = (Color){130, 130, 181, 255};              // Winter Color
        color01 = (Color){145, 145, 166, 255};
        color02 = (Color){104, 142, 144, 255};
        color03 = (Color){57, 140, 173, 255};
    }
    else if (initSeason == 3)
    {
        initMonth = 8;
        clockRotation = 135;
        clockInitRotation = 135;
        clockFinalRotation = clockInitRotation + 90;
        color00 = (Color){196, 176, 49, 255};              // Spring Color
        color01 = (Color){178, 163, 67, 255};
        color02 = (Color){133, 143, 90, 255};
        color03 = (Color){133, 156, 42, 255};
    }

    currentMonth = initMonth;

    leftButton.x = 0;
    leftButton.y = 200;
    leftButton.width = GetScreenWidth()/2;
    leftButton.height = GetScreenHeight();

    rightButton.x = GetScreenWidth()/2;
    rightButton.y = 200;
    rightButton.width = GetScreenWidth()/2;
    rightButton.height = GetScreenHeight();

    powerButton.x = 0;
    powerButton.y = 0;
    powerButton.width = GetScreenWidth()/2;
    powerButton.height = 200;

    finalColor.r = GetRandomValue(0, 255);
    finalColor.g = GetRandomValue(0, 255);
    finalColor.b = GetRandomValue(0, 255);
    finalColor.a = 255;

    backBar.x = 20;
    backBar.y = 22;
    backBar.width = maxPower + 4;
    backBar.height = 24;
    powerBar.x = 22;
    powerBar.y = 23;
    powerBar.width = power;
    powerBar.height = 22;

    fireAnimation.x = gameplay_props_fire_spritesheet.x;
    fireAnimation.y = gameplay_props_fire_spritesheet.y;
    fireAnimation.width = gameplay_props_fire_spritesheet.width/4;
    fireAnimation.height = gameplay_props_fire_spritesheet.height;

    windAnimation.x = gameplay_props_whirlwind_spritesheet.x;
    windAnimation.y = gameplay_props_whirlwind_spritesheet.y;
    windAnimation.width = gameplay_props_whirlwind_spritesheet.width/4;
    windAnimation.height = gameplay_props_whirlwind_spritesheet.height;

    beeAnimation.x = gameplay_enemy_bee.x;
    beeAnimation.y = gameplay_enemy_bee.y;
    beeAnimation.width = gameplay_enemy_bee.width/5;
    beeAnimation.height = gameplay_enemy_bee.height;

    eagleAnimation.x = gameplay_enemy_eagle.x;
    eagleAnimation.y = gameplay_enemy_eagle.y;
    eagleAnimation.width = gameplay_enemy_eagle.width/2;
    eagleAnimation.height = gameplay_enemy_eagle.height;

    snakeAnimation.x = gameplay_enemy_snake.x;
    snakeAnimation.y = gameplay_enemy_snake.y;
    snakeAnimation.width = gameplay_enemy_snake.width/3;
    snakeAnimation.height = gameplay_enemy_snake.height;

    dingoAnimation.x = gameplay_enemy_dingo.x;
    dingoAnimation.y = gameplay_enemy_dingo.y;
    dingoAnimation.width = gameplay_enemy_dingo.width/3;
    dingoAnimation.height = gameplay_enemy_dingo.height;

    owlAnimation.x = gameplay_enemy_owl.x;
    owlAnimation.y = gameplay_enemy_owl.y;
    owlAnimation.width = gameplay_enemy_owl.width/3;
    owlAnimation.height = gameplay_enemy_owl.height;
    
    koalaAnimationIddle = gameplay_koala_idle;
    koalaAnimationIddle.width = gameplay_koala_idle.width/3;
    koalaAnimationJump = gameplay_koala_jump;
    koalaAnimationFly = gameplay_koala_fly;
    koalaAnimationFly.width = gameplay_koala_fly.width/2;
    koalaAnimationTransform = gameplay_koala_transform;
    koalaAnimationTransform.width = gameplay_koala_transform.width/2;

    snowParticle.position = (Vector2){ 0, 0 };
    snowParticle.active = false;
    snowStormParticle.position = (Vector2){ 0, 0 };
    snowStormParticle.active = false;
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
    rainParticle.position = (Vector2){ 0, 0 };
    rainParticle.active = false;
    rainStormParticle.position = (Vector2){ 0, 0 };
    rainStormParticle.active = false;
    backRainParticle.position = (Vector2){ 0, 0 };
    backRainParticle.active = false;
    rayParticles.position = (Vector2){ 0, 0 };
    rayParticles.active = false;
    backRayParticles.position = (Vector2){ 0, 0 };
    backRayParticles.active = false;
    speedFX.active = false;
    
    clockPosition = (Vector2){GetScreenWidth(), 0};

    for (int j = 0; j < MAX_PARTICLES; j++)
    {
        snowParticle.particles[j].active = false;
        snowParticle.particles[j].position = (Vector2){ 0, 0 };
        snowParticle.particles[j].size = (float)GetRandomValue(3, 9)/10;
        snowParticle.particles[j].rotation = GetRandomValue(0, 360);
        snowParticle.particles[j].color = WHITE;
        snowParticle.particles[j].alpha = 1.0f;

        backSnowParticle.particles[j].active = false;
        backSnowParticle.particles[j].position = (Vector2){ 0, 0 };
        backSnowParticle.particles[j].size = (float)GetRandomValue(2, 8)/10;
        backSnowParticle.particles[j].rotation = GetRandomValue(0, 360);
        backSnowParticle.particles[j].color = WHITE;
        backSnowParticle.particles[j].alpha = 0.7f;

        planetreeParticle.particles[j].active = false;
        planetreeParticle.particles[j].position = (Vector2){ 0, 0 };
        planetreeParticle.particles[j].size = (float)GetRandomValue(3, 9)/10;
        planetreeParticle.particles[j].rotation = GetRandomValue(0, 360);
        planetreeParticle.particles[j].color = WHITE;
        planetreeParticle.particles[j].alpha = 1.0f;

        backPlanetreeParticle.particles[j].active = false;
        backPlanetreeParticle.particles[j].position = (Vector2){ 0, 0 };
        backPlanetreeParticle.particles[j].size = (float)GetRandomValue(2, 8)/10;
        backPlanetreeParticle.particles[j].rotation = GetRandomValue(0, 360);
        backPlanetreeParticle.particles[j].color = WHITE;
        backPlanetreeParticle.particles[j].alpha = 0.7f;

        dandelionParticle.particles[j].active = false;
        dandelionParticle.particles[j].position = (Vector2){ 0, 0 };
        dandelionParticle.particles[j].size = (float)GetRandomValue(3, 9)/10;
        dandelionParticle.particles[j].rotation = 0;
        dandelionParticle.particles[j].color = WHITE;
        dandelionParticle.particles[j].alpha = 1;
        dandelionParticle.particles[j].rotPhy = GetRandomValue(0 , 180);

        dandelionBackParticle.particles[j].active = false;
        dandelionBackParticle.particles[j].position = (Vector2){ 0, 0 };
        dandelionBackParticle.particles[j].size = (float)GetRandomValue(2, 8)/10;
        dandelionBackParticle.particles[j].rotation = 0;
        dandelionBackParticle.particles[j].color = WHITE;
        dandelionBackParticle.particles[j].alpha = 0.7f;
        dandelionBackParticle.particles[j].rotPhy = GetRandomValue(0 , 180);

        flowerParticle.particles[j].active = false;
        flowerParticle.particles[j].position = (Vector2){ 0, 0 };
        flowerParticle.particles[j].size = (float)GetRandomValue(3, 9)/10;
        flowerParticle.particles[j].rotation = GetRandomValue(0, 360);
        flowerParticle.particles[j].color = WHITE;
        flowerParticle.particles[j].alpha = 1.0f;

        backFlowerParticle.particles[j].active = false;
        backFlowerParticle.particles[j].position = (Vector2){ 0, 0 };
        backFlowerParticle.particles[j].size = (float)GetRandomValue(2, 8)/10;
        backFlowerParticle.particles[j].rotation = GetRandomValue(0, 360);
        backFlowerParticle.particles[j].color = WHITE;
        backFlowerParticle.particles[j].alpha = 0.7f;

        rainParticle.particles[j].active = false;
        rainParticle.particles[j].position = (Vector2){ 0, 0 };
        rainParticle.particles[j].size = (float)GetRandomValue(3, 9)/10;
        rainParticle.particles[j].rotation = -20;
        rainParticle.particles[j].color = WHITE;
        rainParticle.particles[j].alpha = 1.0f;

        backRainParticle.particles[j].active = false;
        backRainParticle.particles[j].position = (Vector2){ 0, 0 };
        backRainParticle.particles[j].size = (float)GetRandomValue(2, 8)/10;
        backRainParticle.particles[j].rotation = -20;
        backRainParticle.particles[j].color = WHITE;
        backRainParticle.particles[j].alpha = 0.7f;

    }
    
    for (int j = 0; j < MAX_PARTICLES_SPEED; j++)
    {
        speedFX.particle[j].position = (Vector2){ 0, 0 };
        speedFX.particle[j].color = WHITE;
        speedFX.particle[j].alpha = 1.0f;
        speedFX.particle[j].size = (Vector2){GetScreenWidth(), GetRandomValue(10, 50)/10};
        speedFX.particle[j].rotation = 0.0f;
        speedFX.particle[j].active = false;
    }

    for (int j = 0; j < MAX_PARTICLES_STORM; j++)
    {
        rainStormParticle.particles[j].active = false;
        rainStormParticle.particles[j].position = (Vector2){ 0, 0 };
        rainStormParticle.particles[j].size = (float)GetRandomValue(3, 9)/10;
        rainStormParticle.particles[j].rotation = -40;
        rainStormParticle.particles[j].color = WHITE;
        rainStormParticle.particles[j].alpha = 1.0f;
    }

    for (int j = 0; j < MAX_PARTICLES_STORM; j++)
    {
        snowStormParticle.particles[j].active = false;
        snowStormParticle.particles[j].position = (Vector2){ 0, 0 };
        snowStormParticle.particles[j].size = (float)GetRandomValue(2, 8)/10;
        snowStormParticle.particles[j].rotation = 40;
        snowStormParticle.particles[j].color = WHITE;
        snowStormParticle.particles[j].alpha = 1.0f;
    }

    for (int i = 0; i < MAX_PARTICLES_RAY; i++)
    {
        rayParticles.particles[i].position = (Vector2){ 0, 0 };
        rayParticles.particles[i].color.r = 255;
        rayParticles.particles[i].color.g = 255;
        rayParticles.particles[i].color.b = 182;
        rayParticles.particles[i].color.a = 255;
        rayParticles.particles[i].alpha = 0.0f;
        rayParticles.particles[i].size = (float)(GetRandomValue(30, 70)/10);
        rayParticles.particles[i].rotation = 0.0f;
        rayParticles.particles[i].active = false;
        rayParticles.particles[i].fading = false;
        rayParticles.particles[i].delayCounter = 0;

        backRayParticles.particles[i].position = (Vector2){ 0, 0 };
        backRayParticles.particles[i].color.r = 255;
        backRayParticles.particles[i].color.g = 255;
        backRayParticles.particles[i].color.b = 182;
        backRayParticles.particles[i].color.a = 255;
        backRayParticles.particles[i].alpha = 0.0f;
        backRayParticles.particles[i].size = (float)(GetRandomValue(10, 20)/10);
        backRayParticles.particles[i].rotation = 0.0f;
        backRayParticles.particles[i].active = false;
        backRayParticles.particles[i].fading = false;
        backRayParticles.particles[i].delayCounter = 0;
    }

    for (int i = 0; i < MAX_KILLS; i++) killHistory[i] = 0;

    for (int i = 0; i < MAX_BAMBOO; i++)
    {
        bamboo[i].x = 150 + 200*i;
        bamboo[i].y = 0;
        bamboo[i].width = 50;
        bamboo[i].height = GetScreenHeight();
        if (i > 5) bambooActive[i] = false;
        else bambooActive[i] = true;
    }

    for (int i = 0; i < MAX_FIRE; i++)
    {
        fire[i].x = -200;
        fire[i].y = GetScreenHeight() - 30;
        fire[i].width = 30;
        fire[i].height = 720;
        fireActive[i] = false;
        onFire[i] = false;
        fireCounter[i] = 0;
    }

    for (int i = 0; i < MAX_ICE; i++)
    {
        ice[i].x = -100;
        ice[i].y = 0;
        ice[i].width = 10;
        ice[i].height = GetScreenHeight();
        iceActive[i] = false;
    }

    for (int i = 0; i < MAX_RESIN; i++)
    {
        resin[i].x = -100;
        resin[i].y = 0;
        resin[i].width = 32;
        resin[i].height = 50;
        resinActive[i] = false;
    }

    for (int i = 0; i < MAX_WIND; i++)
    {
        wind[i].x = -100;
        wind[i].y = 0;
        wind[i].width = 70;
        wind[i].height = 100;
        windActive[i] = false;
    }

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        snake[i].x = 0;
        snake[i].y = 0;
        snake[i].width = 50;
        snake[i].height = 60;
        snakeActive[i] = false;
        isHitSnake[i] = false;

        dingo[i].x = -100;
        dingo[i].y = 0;
        dingo[i].width = 64;
        dingo[i].height = 90;
        dingoActive[i] = false;
        isHitDingo[i] = false;

        owl[i].x = -100;
        owl[i].y = 0;
        owl[i].width = 40;
        owl[i].height = 60;
        owlActive[i] = false;
        branchActive[i] = false;
        isHitOwl[i] = false;

        branchPos[i].x = owl[i].x;
        branchPos[i].y = owl[i].y;

        enemyHit[i].position = (Vector2){ GetRandomValue(-20, 20), GetRandomValue(-20, 20) };
        enemyHit[i].speed = (Vector2){ (float)GetRandomValue(-500, 500)/100, (float)GetRandomValue(-500, 500)/100 };
        enemyHit[i].size = (float)GetRandomValue(1, 45)/30;
        enemyHit[i].rotation = GetRandomValue(0, 360);
        enemyHit[i].color = RED;
        enemyHit[i].alpha = 1.0f;
        enemyHit[i].active = false;
        
        popupScore[i].position = (Vector2){ GetRandomValue(-20, 20), GetRandomValue(-20, 20) };
        popupScore[i].scale = (float)GetRandomValue(1, 45)/30;
        popupScore[i].alpha = 1.0f;
        popupScore[i].active = false;
    }

    for (int i = 0; i < MAX_LEAVES; i++)
    {
        leaf[i].x = 0;
        leaf[i].y = 0;
        leaf[i].width = 30;
        leaf[i].height = 30;
        leafActive[i] = false;
        leafType[i] = -1;

        leafParticles[i].position = (Vector2){ 0, 0 };
        leafParticles[i].active = false;
        
        popupLeaves[i].position = (Vector2){ GetRandomValue(-20, 20), GetRandomValue(-20, 20) };
        popupLeaves[i].scale = (float)GetRandomValue(1, 45)/30;
        popupLeaves[i].alpha = 1.0f;
        popupLeaves[i].score = 0;
        popupLeaves[i].active = false;

        for (int j = 0; j < 32; j++)
        {

            leafParticles[i].particles[j].active = false;
            leafParticles[i].particles[j].position = (Vector2){ GetRandomValue(-20, 20), GetRandomValue(-20, 20) };
            leafParticles[i].particles[j].speed = (Vector2){ (float)GetRandomValue(-500, 500)/100, (float)GetRandomValue(-500, 500)/100 };
            leafParticles[i].particles[j].size = (float)GetRandomValue(3, 10)/5;
            leafParticles[i].particles[j].rotation = GetRandomValue(0, 360);
            leafParticles[i].particles[j].color = WHITE;
            leafParticles[i].particles[j].alpha = 1.0f;
        }
    }

    player.x = GetScreenWidth()*0.26f;
    player.y = 100;
    player.width = 35;
    player.height = 60;

    bee.x = -200;
    bee.y = 0;
    bee.width = 50;
    bee.height = 32;
    beeActive = false;
    
    popupBee.position = (Vector2){ GetRandomValue(-20, 20), GetRandomValue(-20, 20) };
    popupBee.scale = (float)GetRandomValue(1, 45)/30;
    popupBee.alpha = 1.0f;
    popupBee.active = false;

    eagle.x = -128;
    eagle.y = 0;
    eagle.width = 200;
    eagle.height = 80;
    eagleActive = false;
    
    popupEagle.position = (Vector2){ GetRandomValue(-20, 20), GetRandomValue(-20, 20) };
    popupEagle.scale = (float)GetRandomValue(1, 45)/30;
    popupEagle.alpha = 1.0f;
    popupEagle.active = false;

    counterColor.r = 255;
    counterColor.g = 224;
    counterColor.b = 185;
    counterColor.a = 255;

    zero.x = 0;
    zero.y = 0;

    firePos.x = -200;
    firePos.y = 0;
    
    textSize = MeasureTextEx(font, "3", font.baseSize*5, 2);
}
