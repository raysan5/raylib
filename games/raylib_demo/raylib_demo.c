/*******************************************************************************************
*
*   raylib - Features demo 01 (Learn Videogames Programming)
*
*   This show has been created using raylib v1.4 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#include <string.h>
#include <math.h>

#define MAX_BALLS 16

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum TalkScreen { LOADING, LOGO, MODULES, ENDING, PONG } TalkScreen;
typedef enum Modules { CORE = 0, SHAPES, TEXTURES, TEXT, MODELS, AUDIO } Modules;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
int screenWidth = 1280;
int screenHeight = 720;

const char msgLoading[30] = "LOADING...";
const char msgPressEnter[30] = "Press ENTER to START";

const char msgCredits[40] = "by RAMON SANTAMARIA [@raysan5]";
const char msgWeb[30] = "www.raylib.com";

const char msgLogoA[40] = "A simple and easy-to-use library";
const char msgLogoB[40] = "to learn videogames programming";

const char msg1[50] = "THIS is a CUSTOM FONT...";
const char msg2[50] = "...and ANOTHER CUSTOM ONE...";
const char msg3[50] = "...AND ONE MORE! :)";

bool closeWindow = false;

int totalTime = 60*60*60;   // fps*sec*min
int timeCounter = 0;

TalkScreen currentScreen = LOADING;

// LOADING screen variables
int loadBarWidth = 0;
int loadBarMaxWidth = 600;

// TITLE screen variables
SpriteFont fontAlagard;
SpriteFont fontPixelplay;
SpriteFont fontMecha;
SpriteFont fontSetback;
SpriteFont fontRomulus;

Vector2 pongBallPosition;
Vector2 pongBallSpeed;
Rectangle pongPlayerRec;
Rectangle pongEnemyRec;
int pongScorePlayer = 0;
int pongScoreEnemy = 0;
bool pongAutoMode = true;
int pongAutoCounter = 0;
bool pongPaused = true;

int lettersCounter = 0;
char msgBuffer[120] = { ' ' };

// LOGO screen variables
int logoPositionX;
int logoPositionY;

int raylibLettersCount = 0;

int topSideRecWidth = 16;
int leftSideRecHeight = 16;

int bottomSideRecWidth = 16;
int rightSideRecHeight = 16;

char raylib[8] = "       \0";   // raylib text array, max 8 letters

int logoScreenState = 0;        // Tracking animation states (State Machine)
bool msgLogoADone = false;
bool msgLogoBDone = false;

// MODULES screen variables
Modules selectedModule = CORE;

Texture2D raylibWindow;
Texture2D raylibWindow01;
Texture2D raylibWindow02;
Texture2D raylibWindow03;
Texture2D platforms;
Texture2D raylibLogoB;
Texture2D lena;
Texture2D mandrill;
Texture2D texAlagard;
SpriteFont fontMechaC;
SpriteFont fontAlagardC;
SpriteFont fontJupiterC;

int coreWindow = 1;

int windowOffset = 0;
Vector2 ballPosition;

Camera camera;

Texture2D catTexture;
Model cat;

Sound fxWav;
Sound fxOgg;

Vector2 soundBallsPosition[MAX_BALLS];
Color soundBallsColor[MAX_BALLS];
bool soundBallsActive[MAX_BALLS];
float soundBallsAlpha[MAX_BALLS];
int soundBallsRadius[MAX_BALLS];

float scaleFactor = 0.0f;
float timePlayed = 0;

// ENDING screen variables
Texture2D raylibLogoA;

// Required variables to manage screen transitions (fade-in, fade-out)
float transAlpha = 0;
bool onTransition = false;
bool transFadeOut = false;
int transFromScreen = -1;
int transToScreen = -1;

int framesCounter = 0;
    
//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
void TransitionToScreen(int screen);
void UpdateTransition(void);
void DrawTransition(void);

void UpdateDrawOneFrame(void);

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const char windowTitle[30] = "raylib functionality demo";
   
    //SetupFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(screenWidth, screenHeight, windowTitle);
    
    InitAudioDevice();             // Initialize audio device

    // TITLE screen variables Initialization
    fontAlagard = LoadSpriteFont("resources/fonts/alagard.rbmf");        // rBMF font loading
    fontPixelplay = LoadSpriteFont("resources/fonts/pixelplay.rbmf");    // rBMF font loading
    fontMecha = LoadSpriteFont("resources/fonts/mecha.rbmf");            // rBMF font loading
    fontSetback = LoadSpriteFont("resources/fonts/setback.rbmf");        // rBMF font loading
    fontRomulus = LoadSpriteFont("resources/fonts/romulus.rbmf");        // rBMF font loading
    
    pongBallPosition = (Vector2){ screenWidth/2, screenHeight/2 + 20 };
    pongBallSpeed = (Vector2){ 6, 6 };
    pongPlayerRec = (Rectangle){ 20, screenHeight/2 - 50 + 40, 20, 100 };
    pongEnemyRec = (Rectangle){ screenWidth - 40, screenHeight/2 - 60, 20, 120 };

    // LOGO screen variables Initialization
    logoPositionX = screenWidth/2 - 128;
    logoPositionY = screenHeight/2 - 128;
    
    // MODULES screen variables Initialization
    raylibWindow = LoadTexture("resources/raylib_window.png");
    raylibWindow01 = LoadTexture("resources/raylib_window_01.png");
    raylibWindow02 = LoadTexture("resources/raylib_window_02.png");
    raylibWindow03 = LoadTexture("resources/raylib_window_03.png");
    platforms = LoadTexture("resources/platforms.png");
    raylibLogoB = LoadTexture("resources/raylib_logo128x128.png");
    lena = LoadTexture("resources/lena.png");
    mandrill = LoadTexture("resources/mandrill.png");
    texAlagard = LoadTexture("resources/fonts/custom_alagard.png");
    fontMechaC = LoadSpriteFont("resources/fonts/custom_mecha.png");
    fontAlagardC = LoadSpriteFont("resources/fonts/custom_alagard.png");
    fontJupiterC = LoadSpriteFont("resources/fonts/custom_jupiter_crash.png");
    
    ballPosition = (Vector2){ 520 + 656/2, 220 + 399/2 };
    
    camera = (Camera){{ 0.0, 12.0, 15.0 }, { 0.0, 3.0, 0.0 }, { 0.0, 1.0, 0.0 }};

    catTexture = LoadTexture("resources/catsham.png");   // Load model texture
    cat = LoadModel("resources/cat.obj");                // Load OBJ model
    cat.material.texDiffuse = texture;                   // Set cat model diffuse texture
    
    fxWav = LoadSound("resources/audio/weird.wav");         // Load WAV audio file
    fxOgg = LoadSound("resources/audio/tanatana.ogg");      // Load OGG audio file
    
    for (int i = 0; i < MAX_BALLS; i++)
    {
        soundBallsPosition[i] = (Vector2){ 650 + 560/2 + GetRandomValue(-280, 280), 220 + 200 + GetRandomValue(-200, 200) };
        soundBallsColor[i] = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
        soundBallsRadius[i] = GetRandomValue(2, 50);
        soundBallsAlpha[i] = 1.0f;
        
        soundBallsActive[i] = false;
    }
    
    // ENDING screen variables Initialization
    raylibLogoA = LoadTexture("resources/raylib_logo.png");

#ifndef PLATFORM_WEB
    SetTargetFPS(60);
#endif

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawOneFrame, 0, 1);
#else
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose() && !closeWindow)    // Detect window close button or ESC key
    {
        UpdateDrawOneFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // Unload all loaded data (textures, fonts, audio)
    UnloadSpriteFont(fontAlagard);      // SpriteFont unloading
    UnloadSpriteFont(fontPixelplay);    // SpriteFont unloading
    UnloadSpriteFont(fontMecha);        // SpriteFont unloading
    UnloadSpriteFont(fontSetback);      // SpriteFont unloading
    UnloadSpriteFont(fontRomulus);      // SpriteFont unloading

    UnloadTexture(raylibWindow);
    UnloadTexture(raylibWindow01);
    UnloadTexture(raylibWindow02);
    UnloadTexture(raylibWindow03);
    UnloadTexture(platforms);
    UnloadTexture(raylibLogoA);
    UnloadTexture(raylibLogoB);
    UnloadTexture(lena);
    UnloadTexture(mandrill);
    UnloadTexture(texAlagard);

    UnloadSpriteFont(fontMechaC);
    UnloadSpriteFont(fontAlagardC);
    UnloadSpriteFont(fontJupiterC);

    UnloadTexture(catTexture);
    UnloadModel(cat);

    UnloadSound(fxWav);
    UnloadSound(fxOgg);

    CloseAudioDevice();
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void TransitionToScreen(int screen)
{
    onTransition = true;
    transFromScreen = currentScreen;
    transToScreen = screen;
}

void UpdateTransition(void)
{
    if (!transFadeOut)
    {
        transAlpha += 0.02f;

        if (transAlpha >= 1.0)
        {
            transAlpha = 1.0;
            currentScreen = transToScreen;
            transFadeOut = true;
            framesCounter = 0;
        }
    }
    else  // Transition fade out logic
    {
        transAlpha -= 0.02f;

        if (transAlpha <= 0)
        {
            transAlpha = 0;
            transFadeOut = false;
            onTransition = false;
            transFromScreen = -1;
            transToScreen = -1;
        }
    }
}

void DrawTransition(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, transAlpha));
}

void UpdateDrawOneFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    if (!onTransition)
    {
        switch(currentScreen) 
        {
            case LOADING: 
            {
                // Update LOADING screen variables
                framesCounter++;    // Count frames

                if ((loadBarWidth < loadBarMaxWidth) && ((framesCounter%30) == 0)) loadBarWidth++;

                if (IsKeyDown(KEY_SPACE) && (loadBarWidth < loadBarMaxWidth)) loadBarWidth += 4;

                if (IsKeyPressed(KEY_ENTER) && (loadBarWidth >= loadBarMaxWidth)) TransitionToScreen(LOGO);

            } break;
            case LOGO:
            {
                // Update LOGO screen variables
                if (logoScreenState == 0)                 // State 0: Small box blinking
                {
                    framesCounter++;

                    if (framesCounter == 120)
                    {
                        logoScreenState = 1;
                        framesCounter = 0;      // Reset counter... will be used later...
                    }
                }
                else if (logoScreenState == 1)            // State 1: Top and left bars growing
                {
                    topSideRecWidth += 4;
                    leftSideRecHeight += 4;

                    if (topSideRecWidth == 256) logoScreenState = 2;
                }
                else if (logoScreenState == 2)            // State 2: Bottom and right bars growing
                {
                    bottomSideRecWidth += 4;
                    rightSideRecHeight += 4;

                    if (bottomSideRecWidth == 256)
                    {
                        lettersCounter = 0;
                        for (int i = 0; i < strlen(msgBuffer); i++) msgBuffer[i] = ' ';

                        logoScreenState = 3;
                    }
                }
                else if (logoScreenState == 3)            // State 3: Letters appearing (one by one)
                {
                    framesCounter++;

                    // Every 12 frames, one more letter!
                    if ((framesCounter%12) == 0) raylibLettersCount++;

                    switch (raylibLettersCount)
                    {
                        case 1: raylib[0] = 'r'; break;
                        case 2: raylib[1] = 'a'; break;
                        case 3: raylib[2] = 'y'; break;
                        case 4: raylib[3] = 'l'; break;
                        case 5: raylib[4] = 'i'; break;
                        case 6: raylib[5] = 'b'; break;
                        default: break;
                    }

                    if (raylibLettersCount >= 10)
                    {
                        // Write raylib description messages
                        if ((framesCounter%2) == 0) lettersCounter++;

                        if (!msgLogoADone)
                        {
                            if (lettersCounter <= strlen(msgLogoA)) strncpy(msgBuffer, msgLogoA, lettersCounter);
                            else
                            {
                                for (int i = 0; i < strlen(msgBuffer); i++) msgBuffer[i] = ' ';

                                lettersCounter = 0;
                                msgLogoADone = true;
                            }
                        }
                        else if (!msgLogoBDone)
                        {
                            if (lettersCounter <= strlen(msgLogoB)) strncpy(msgBuffer, msgLogoB, lettersCounter);
                            else
                            {
                                msgLogoBDone = true;
                                framesCounter = 0;
                            }
                        }
                    }
                }

                // Press enter to change to MODULES screen
                if (IsKeyPressed(KEY_ENTER) && msgLogoBDone) TransitionToScreen(MODULES);
                else if (IsKeyPressed(KEY_BACKSPACE)) TransitionToScreen(LOGO);

            } break;
            case MODULES:
            {
                // Update MODULES screen variables here!
                framesCounter++;

                if (IsKeyPressed(KEY_RIGHT) && (selectedModule < 5))
                {
                    selectedModule++;
                    framesCounter = 0;
                }
                else if (IsKeyPressed(KEY_LEFT) && (selectedModule > 0))
                {
                    selectedModule--;
                    framesCounter = 0;
                }

                if (selectedModule == CORE)
                {
                    if ((framesCounter > 60) && (windowOffset < 40))
                    {
                        windowOffset++;
                        ballPosition.x++;
                        ballPosition.y++;
                    }

                    if (framesCounter > 140)
                    {
                        if (IsKeyDown('A')) ballPosition.x -= 5;
                        if (IsKeyDown('D')) ballPosition.x += 5;
                        if (IsKeyDown('W')) ballPosition.y -= 5;
                        if (IsKeyDown('S')) ballPosition.y += 5;

                        if (IsKeyPressed('1')) coreWindow = 1;
                        if (IsKeyPressed('2')) coreWindow = 2;
                        if (IsKeyPressed('3')) coreWindow = 3;
                        if (IsKeyPressed('4')) coreWindow = 4;
                    }
                }

                if (selectedModule == TEXTURES) scaleFactor = (sinf(2*PI/240*framesCounter) + 1.0f)/2;

                if (selectedModule == AUDIO)
                {
                    if (IsKeyPressed(KEY_SPACE) && !MusicIsPlaying()) PlayMusicStream("resources/audio/guitar_noodling.ogg");         // Play music stream

                    if (IsKeyPressed('S'))
                    {
                        StopMusicStream();
                        timePlayed = 0.0f;

                        for (int i = 0; i < MAX_BALLS; i++)
                        {
                            soundBallsPosition[i] = (Vector2){ 650 + 560/2 + GetRandomValue(-280, 280), 220 + 200 + GetRandomValue(-200, 200) };
                            soundBallsColor[i] = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
                            soundBallsRadius[i] = GetRandomValue(2, 50);
                            soundBallsAlpha[i] = 1.0f;

                            soundBallsActive[i] = false;
                        }
                    }

                    if (MusicIsPlaying())
                    {
                        timePlayed = GetMusicTimePlayed() / GetMusicTimeLength() * 100 * 4;

                        if ((framesCounter%10) == 0)
                        {
                            for (int i = 0; i < MAX_BALLS; i++)
                            {
                                if (!soundBallsActive[i])
                                {
                                    soundBallsActive[i] = true;
                                    break;
                                }
                            }
                        }

                        for (int i = 0; i < MAX_BALLS; i++)
                        {
                            if (soundBallsActive[i]) soundBallsAlpha[i] -= 0.005f;

                            if (soundBallsAlpha[i] <= 0)
                            {
                                soundBallsActive[i] = false;

                                // Reset ball random
                                soundBallsPosition[i] = (Vector2){ 650 + 560/2 + GetRandomValue(-280, 280), 220 + 200 + GetRandomValue(-200, 200) };
                                soundBallsColor[i] = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
                                soundBallsRadius[i] = GetRandomValue(2, 60);
                                soundBallsAlpha[i] = 1.0f;
                            }
                        }
                    }

                    if (IsKeyPressed('N')) PlaySound(fxWav);
                    //if (IsKeyPressed('M')) PlaySound(fxOgg);
                }

                // Press enter to change to ENDING screen
                if (IsKeyPressed(KEY_ENTER)) TransitionToScreen(ENDING);
                else if (IsKeyPressed(KEY_BACKSPACE)) TransitionToScreen(LOGO);

            } break;
            case PONG:
            {
                // Update SECRET screen variables here!
                framesCounter++;

                if (IsKeyPressed('P')) pongPaused = !pongPaused;
                
                if (!pongPaused)
                {
                    pongBallPosition.x += pongBallSpeed.x;
                    pongBallPosition.y += pongBallSpeed.y;

                    if ((pongBallPosition.x >= screenWidth - 5) || (pongBallPosition.x <= 5)) pongBallSpeed.x *= -1;
                    if ((pongBallPosition.y >= screenHeight - 5) || (pongBallPosition.y <= 5)) pongBallSpeed.y *= -1;

                    if (IsKeyDown(KEY_UP) || IsKeyDown('W'))
                    {
                        pongPlayerRec.y -= 5;
                        pongAutoMode = false;
                        pongAutoCounter = 180;
                    }
                    else if (IsKeyDown(KEY_DOWN) || IsKeyDown('S'))
                    {
                        pongPlayerRec.y += 5;
                        pongAutoMode = false;
                        pongAutoCounter = 180;
                    }
                    else if (pongAutoCounter > 0)
                    {
                        pongAutoCounter--;
                        
                        if (pongAutoCounter == 0) pongAutoMode = true;
                    }

                    if ((pongBallPosition.x < 600) && pongAutoMode)
                    {
                        if (pongBallPosition.y > (pongPlayerRec.y + pongPlayerRec.height/2)) pongPlayerRec.y += 5;
                        else if (pongBallPosition.y < (pongPlayerRec.y + pongPlayerRec.height/2)) pongPlayerRec.y -= 5;
                    }

                    if (pongPlayerRec.y <= 0) pongPlayerRec.y = 0;
                    else if ((pongPlayerRec.y + pongPlayerRec.height) >= screenHeight) pongPlayerRec.y = screenHeight - pongPlayerRec.height;
                    
                    if (pongBallPosition.x > screenWidth - 600)
                    {
                        if (pongBallPosition.y > (pongEnemyRec.y + pongEnemyRec.height/2)) pongEnemyRec.y += 5;
                        else if (pongBallPosition.y < (pongEnemyRec.y + pongEnemyRec.height/2)) pongEnemyRec.y -= 5;

                        if (pongEnemyRec.y <= 0) pongEnemyRec.y = 0;
                        else if ((pongEnemyRec.y + pongEnemyRec.height) >= screenHeight) pongEnemyRec.y = screenHeight - pongEnemyRec.height;
                    }

                    if ((CheckCollisionCircleRec(pongBallPosition, 10, pongPlayerRec)) || (CheckCollisionCircleRec(pongBallPosition, 10, pongEnemyRec))) pongBallSpeed.x *= -1;
                    
                    if (pongBallPosition.x >= screenWidth - 5) pongScorePlayer++;
                    else if (pongBallPosition.x <= 5) pongScoreEnemy++;
                }

                // Press enter to move back to MODULES screen
                if (IsKeyPressed(KEY_ENTER)) TransitionToScreen(ENDING);
                if (IsKeyPressed(KEY_BACKSPACE)) TransitionToScreen(ENDING);
            } break;
            case ENDING:
            {
                // Update ENDING screen
                framesCounter++;

                // Press enter to move back to MODULES screen
                if (IsKeyPressed(KEY_ENTER)) TransitionToScreen(PONG);
                if (IsKeyPressed(KEY_BACKSPACE)) TransitionToScreen(MODULES);

            } break;
            default: break;
        }

        if ((currentScreen != LOADING) && (timeCounter < totalTime)) timeCounter++;
    }
    else UpdateTransition(); // Update transition (fade-in, fade-out)
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        switch(currentScreen)
        {
            case LOADING:
            {
                // Draw LOADING screen
                if ((loadBarWidth < loadBarMaxWidth) && ((framesCounter/40)%2)) DrawText(msgLoading, 360, 240, 40, DARKGRAY);

                DrawRectangle(360 - 4, 300 - 4, loadBarMaxWidth + 8, 60 + 8, LIGHTGRAY);
                DrawRectangle(360, 300, loadBarWidth - 1, 60, DARKGRAY);
                DrawRectangleLines(360 - 4, 300 - 5, loadBarMaxWidth + 8, 60 + 8, DARKGRAY);

                if (loadBarWidth >= loadBarMaxWidth)
                {
                    //DrawText(msgLoading, 360, 240, 40, DARKGRAY);
                    if ((framesCounter/30)%2) DrawText(msgPressEnter, screenWidth/2 - MeasureText(msgPressEnter, 40)/2 + 20, 400, 40, DARKGRAY);
                }
                else DrawText("PRESS SPACE to ACCELERATE LOADING! ;)", screenWidth/2 - 200, 400, 20, LIGHTGRAY);

            } break;
            case LOGO:
            {
                // Draw LOGO screen
                if (logoScreenState == 0)
                {
                    if ((framesCounter/15)%2) DrawRectangle(logoPositionX, logoPositionY - 60, 16, 16, BLACK);
                }
                else if (logoScreenState == 1)
                {
                    DrawRectangle(logoPositionX, logoPositionY - 60, topSideRecWidth, 16, BLACK);
                    DrawRectangle(logoPositionX, logoPositionY - 60, 16, leftSideRecHeight, BLACK);
                }
                else if (logoScreenState == 2)
                {
                    DrawRectangle(logoPositionX, logoPositionY - 60, topSideRecWidth, 16, BLACK);
                    DrawRectangle(logoPositionX, logoPositionY - 60, 16, leftSideRecHeight, BLACK);

                    DrawRectangle(logoPositionX + 240, logoPositionY - 60, 16, rightSideRecHeight, BLACK);
                    DrawRectangle(logoPositionX, logoPositionY + 240 - 60, bottomSideRecWidth, 16, BLACK);
                }
                else if (logoScreenState == 3)
                {
                    DrawRectangle(logoPositionX, logoPositionY - 60, topSideRecWidth, 16, BLACK);
                    DrawRectangle(logoPositionX, logoPositionY + 16 - 60, 16, leftSideRecHeight - 32, BLACK);

                    DrawRectangle(logoPositionX + 240, logoPositionY + 16 - 60, 16, rightSideRecHeight - 32, BLACK);
                    DrawRectangle(logoPositionX, logoPositionY + 240 - 60, bottomSideRecWidth, 16, BLACK);

                    DrawRectangle(screenWidth/2 - 112, screenHeight/2 - 112 - 60, 224, 224, RAYWHITE);

                    DrawText(raylib, screenWidth/2 - 44, screenHeight/2 + 48 - 60, 50, BLACK);

                    if (!msgLogoADone) DrawText(msgBuffer, screenWidth/2 - MeasureText(msgLogoA, 30)/2, 460, 30, GRAY);
                    else
                    {
                        DrawText(msgLogoA, screenWidth/2 - MeasureText(msgLogoA, 30)/2, 460, 30, GRAY);

                        if (!msgLogoBDone) DrawText(msgBuffer, screenWidth/2 - MeasureText(msgLogoB, 30)/2, 510, 30, GRAY);
                        else
                        {
                            DrawText(msgLogoB, screenWidth/2 - MeasureText(msgLogoA, 30)/2, 510, 30, GRAY);

                            if ((framesCounter > 90) && ((framesCounter/30)%2)) DrawText("PRESS ENTER to CONTINUE", 930, 650, 20, GRAY);
                        }
                    }
                }
            } break;
            case MODULES:
            {
                // Draw MODULES screen
                DrawTexture(raylibLogoB, 40, 40, WHITE);
                DrawText("raylib is composed of 6 main modules:", 128 + 40 + 30, 50, 20, GRAY);

                if (framesCounter < 120)
                {
                    if (((framesCounter/30)%2) == 0) DrawRectangle(128 + 40 + 30 - 4 + 175*selectedModule, 128 + 40 - 70 - 8 - 4, 158, 78, RED);
                }
                else DrawRectangle(128 + 40 + 30 - 4 + 175*selectedModule, 128 + 40 - 70 - 8 - 4, 158, 78, RED);
                
                if (selectedModule != AUDIO)
                {
                    DrawTriangle((Vector2){950 - 40, 685 - 10}, (Vector2){950 - 60, 685}, (Vector2){950 - 40, 685 + 10}, GRAY);
                    DrawTriangle((Vector2){950 - 30, 685 - 10}, (Vector2){950 - 30, 685 + 10}, (Vector2){950 - 10, 685}, GRAY);
                    DrawText("PRESS RIGHT or LEFT to EXPLORE MODULES", 960, 680, 10, GRAY);
                }

                switch (selectedModule)
                {
                    case CORE:
                    {
                        DrawText("This module give you functions to:", 48, 200, 10, GetColor(0x5c5a5aff));
                    
                        DrawTextEx(fontRomulus, "Open-Close Window", (Vector2){ 48, 230 }, fontRomulus.size*2, 4, GetColor(0x5c5a5aff));
                        DrawTextEx(fontRomulus, "Manage Drawing Area", (Vector2){ 48, 260 }, fontRomulus.size*2, 4, GetColor(0x5c5a5aff));
                        DrawTextEx(fontRomulus, "Manage Inputs", (Vector2){ 48, 290 }, fontRomulus.size*2, 4, GetColor(0x5c5a5aff));
                        DrawTextEx(fontRomulus, "Manage Timming", (Vector2){ 48, 320 }, fontRomulus.size*2, 4, GetColor(0x5c5a5aff));
                        DrawTextEx(fontRomulus, "Auxiliar Functions", (Vector2){ 48, 350 }, fontRomulus.size*2, 4, GetColor(0x5c5a5aff));

                        switch (coreWindow)
                        {
                            case 1: DrawTexture(raylibWindow, 520, 220, WHITE); break;
                            case 2: DrawTextureEx(raylibWindow01, (Vector2){ 450, 220 - 45 }, 0.0f, 4.0f, WHITE); break;
                            case 3: DrawTextureEx(raylibWindow02, (Vector2){ 430, 220 - 40 }, 0.0f, 4.0f, WHITE); break;
                            case 4: DrawTextureEx(raylibWindow03, (Vector2){ 470, 220 - 65 }, 0.0f, 4.0f, WHITE); break;
                            default: DrawTexture(raylibWindow, 520, 220, WHITE); break;
                        }
                        
                        if (framesCounter > 140) DrawText("Check the possible windows raylib can run on. PRESS KEY: 1, 2, 3 or 4", 520 + 8 + windowOffset + 160, 220 + windowOffset + 10, 10, LIGHTGRAY);                        
                        
                        DrawText("Compile raylib C code for the folowing platforms:", 48, 400, 10, MAROON);
                        
                        DrawTextureRec(platforms, (Rectangle){ 0, 0, platforms.width, platforms.height}, (Vector2){ 75, 420 }, WHITE);

                        DrawRectangle(520 + 8 + windowOffset, 220 + 31 + windowOffset, 640, 360, RAYWHITE);
                        DrawRectangleLines(520 + 8 + windowOffset - 1, 220 + 31 + windowOffset - 2, 640 + 2, 360 + 2, GRAY);
                        DrawFPS(520 + 8 + windowOffset + 10, 220 + 31 + windowOffset + 10);
                        
                        DrawRectangle(ballPosition.x - 50, ballPosition.y - 50, 100, 100, Fade(MAROON, 0.5f));
                        DrawRectangleRec(GetCollisionRec((Rectangle){ 520 + 8 + windowOffset - 1, 220 + 31 + windowOffset - 1, 640 + 2, 360 + 2 }, (Rectangle){ (int)ballPosition.x - 50, (int)ballPosition.y - 50, 100, 100 }), MAROON);
                        
                        if (framesCounter > 140)
                        {
                            DrawTextEx(fontMecha, "MOVE ME", (Vector2){ ballPosition.x - 26, ballPosition.y - 20 }, fontMecha.size, 2, BLACK);
                            DrawTextEx(fontMecha, "[ W A S D ]", (Vector2){ ballPosition.x - 36, ballPosition.y }, fontMecha.size, 2, BLACK);
                        }
                    } break;
                    case SHAPES:
                    {
                        DrawText("This module give you functions to:", 48, 200, 10, GetColor(0xcd5757ff));
                    
                        DrawTextEx(fontRomulus, "Draw Basic Shapes", (Vector2){ 48, 230 }, fontRomulus.size*2, 4, GetColor(0xcd5757ff));
                        DrawTextEx(fontRomulus, "Basic Collision Detection", (Vector2){ 48, 260 }, fontRomulus.size*2, 4, GetColor(0xcd5757ff));

                        DrawCircle(screenWidth/4, 120 + 240, 35, DARKBLUE);
                        DrawCircleGradient(screenWidth/4, 220 + 240, 60, GREEN, SKYBLUE);
                        DrawCircleLines(screenWidth/4, 340 + 240, 80, DARKBLUE);

                        DrawRectangle(screenWidth/4*2 - 110, 100 + 180, 220, 100, LIME);
                        DrawRectangleGradient(screenWidth/4*2 - 90, 170 + 240, 180, 130, MAROON, GOLD);
                        DrawRectangleLines(screenWidth/4*2 - 80, 320 + 240, 160, 80, ORANGE);

                        DrawTriangle((Vector2){screenWidth/4*3, 60 + 220}, (Vector2){screenWidth/4*3 - 60, 160 + 220}, (Vector2){screenWidth/4*3 + 60, 160 + 220}, VIOLET);

                        DrawTriangleLines((Vector2){screenWidth/4*3, 140 + 220}, (Vector2){screenWidth/4*3 - 60, 210 + 260}, (Vector2){screenWidth/4*3 + 60, 210 + 260}, SKYBLUE);

                        DrawPoly((Vector2){screenWidth/4*3, 320 + 240}, 6, 80, 0, BROWN);

                    } break;
                    case TEXTURES:
                    {
                        DrawText("This module give you functions to:", 48, 200, 10, GetColor(0x60815aff));
                    
                        DrawTextEx(fontRomulus, "Load Images and Textures", (Vector2){ 48, 230 }, fontRomulus.size*2, 4, GetColor(0x60815aff));
                        DrawTextEx(fontRomulus, "Draw Textures", (Vector2){ 48, 260 }, fontRomulus.size*2, 4, GetColor(0x60815aff));

                        DrawRectangle(138, 348, 260, 260, GRAY);
                        DrawTexturePro(lena, (Rectangle){ 0, 0, lena.width, lena.height }, (Rectangle){ 140 + 128, 350 + 128, lena.width/2*scaleFactor, lena.height/2*scaleFactor }, (Vector2){ lena.width/4*scaleFactor, lena.height/4*scaleFactor }, 0.0f, WHITE);

                        DrawTexture(lena, 600, 180, Fade(WHITE, 0.3f));
                        DrawTextureRec(lena, (Rectangle){ 225, 240, 155, 50 }, (Vector2){ 600 + 256 - 82 + 50, 180 + 241 }, PINK);

                        DrawTexturePro(mandrill, (Rectangle){ 0, 0, mandrill.width, mandrill.height }, (Rectangle){ screenWidth/2 - 40, 350 + 128, mandrill.width/2, mandrill.height/2 },
                                        (Vector2){ mandrill.width/4, mandrill.height/4 }, framesCounter, GOLD);

                    } break;
                    case TEXT:
                    {
                        DrawText("This module give you functions to:", 48, 200, 10, GetColor(0x377764ff));
                    
                        DrawTextEx(fontRomulus, "Load SpriteFonts", (Vector2){ 48, 230 }, fontRomulus.size*2, 4, GetColor(0x377764ff));
                        DrawTextEx(fontRomulus, "Draw Text", (Vector2){ 48, 260 }, fontRomulus.size*2, 4, GetColor(0x377764ff));
                        DrawTextEx(fontRomulus, "Text Formatting", (Vector2){ 48, 290 }, fontRomulus.size*2, 4, GetColor(0x377764ff));

                        DrawTexture(texAlagard, 60, 360, WHITE);

                        DrawTextEx(fontMechaC, msg1, (Vector2){ 540 + 168, 210 }, fontMechaC.size, -3, WHITE);
                        DrawTextEx(fontAlagardC, msg2, (Vector2){ 460 + 140, 260 }, fontAlagardC.size, -2, WHITE);
                        DrawTextEx(fontJupiterC, msg3, (Vector2){ 640 + 70, 300 }, fontJupiterC.size, 2, WHITE);

                        DrawTextEx(fontAlagard, "It also includes some...", (Vector2){ 650 + 70, 400 }, fontAlagard.size*2, 2, MAROON);
                        DrawTextEx(fontPixelplay, "...free fonts in rBMF format...", (Vector2){ 705 - 26, 450 }, fontPixelplay.size*2, 4, ORANGE);
                        DrawTextEx(fontMecha, "...to be used even in...", (Vector2){ 700 + 40, 500 }, fontMecha.size*2, 4, DARKGREEN);
                        DrawTextEx(fontSetback, "...comercial projects...", (Vector2){ 710, 550 }, fontSetback.size*2, 4, DARKBLUE);
                        DrawTextEx(fontRomulus, "...completely for free!", (Vector2){ 710 + 17, 600 }, fontRomulus.size*2, 3, DARKPURPLE);
                        
                        DrawText("This is a custom font spritesheet, raylib can load it automatically!", 228, 360 + 295, 10, GRAY);

                    } break;
                    case MODELS:
                    {
                        DrawText("This module give you functions to:", 48, 200, 10, GetColor(0x417794ff));
                        
                        DrawTextEx(fontRomulus, "Draw Geometric Models", (Vector2){ 48, 230 }, fontRomulus.size*2, 4, GetColor(0x417794ff));
                        DrawTextEx(fontRomulus, "Load 3D Models", (Vector2){ 48, 260 }, fontRomulus.size*2, 4, GetColor(0x417794ff));
                        DrawTextEx(fontRomulus, "Draw 3D Models", (Vector2){ 48, 290 }, fontRomulus.size*2, 4, GetColor(0x417794ff));

                        Begin3dMode(camera);

                            DrawCube((Vector3){-4, 0, 2}, 2, 5, 2, RED);
                            DrawCubeWires((Vector3){-4, 0, 2}, 2, 5, 2, GOLD);
                            DrawCubeWires((Vector3){-4, 0, -2}, 3, 6, 2, MAROON);

                            DrawSphere((Vector3){-1, 0, -2}, 1, GREEN);
                            DrawSphereWires((Vector3){1, 0, 2}, 2, 16, 16, LIME);

                            DrawCylinder((Vector3){4, 0, -2}, 1, 2, 3, 4, SKYBLUE);
                            DrawCylinderWires((Vector3){4, 0, -2}, 1, 2, 3, 4, DARKBLUE);
                            DrawCylinderWires((Vector3){4.5, -1, 2}, 1, 1, 2, 6, BROWN);

                            DrawCylinder((Vector3){1, 0, -4}, 0, 1.5, 3, 8, GOLD);
                            DrawCylinderWires((Vector3){1, 0, -4}, 0, 1.5, 3, 8, PINK);

                            DrawModelEx(cat, (Vector3){ 8.0f, 0.0f, 2.0f }, (Vector3){ 0.0f, 1.0f, 0.0f }, 0.5f*framesCounter, (Vector3){ 0.1f, 0.1f, 0.1f }, WHITE);
                            DrawGizmo((Vector3){ 8.0f, 0.0f, 2.0f });

                            DrawGrid(10.0, 1.0);        // Draw a grid

                        End3dMode();

                        DrawFPS(900, 220);

                    } break;
                    case AUDIO:
                    {
                        DrawText("This module give you functions to:", 48, 200, 10, GetColor(0x8c7539ff));
                        
                        DrawTextEx(fontRomulus, "Load and Play Sounds", (Vector2){ 48, 230 }, fontRomulus.size*2, 4, GetColor(0x8c7539ff));
                        DrawTextEx(fontRomulus, "Play Music (streaming)", (Vector2){ 48, 260 }, fontRomulus.size*2, 4, GetColor(0x8c7539ff));

                        DrawText("PRESS SPACE to START PLAYING MUSIC", 135, 350, 20, GRAY);
                        DrawRectangle(150, 390, 400, 12, LIGHTGRAY);
                        DrawRectangle(150, 390, (int)timePlayed, 12, MAROON);

                        if (MusicIsPlaying())
                        {
                            DrawText("PRESS 'S' to STOP PLAYING MUSIC", 165, 425, 20, GRAY);

                            for (int i = 0; i < MAX_BALLS; i++)
                            {
                                if (soundBallsActive[i]) DrawPoly(soundBallsPosition[i], 18, soundBallsRadius[i], 0.0f, Fade(soundBallsColor[i], soundBallsAlpha[i]));
                            }
                        }

                        DrawText("PRESS 'N' to PLAY a SOUND", 200, 540, 20, VIOLET);

                        if ((framesCounter/30)%2) DrawText("PRESS ENTER to CONTINUE", 930, 650, 20, GRAY);

                    } break;
                    default: break;
                }

                // Draw modules menu
                DrawRectangle(128 + 40 + 30, 128 + 40 - 70 - 8, 150, 70, GetColor(0x898888ff));
                DrawRectangle(128 + 40 + 30 + 8, 128 + 40 - 70, 150 - 16, 70 - 16, GetColor(0xe1e1e1ff));
                DrawText("CORE", 128 + 40 + 30 + 8 + 38, 128 + 40 - 50, 20, GetColor(0x5c5a5aff));

                DrawRectangle(128 + 40 + 30 + 175, 128 + 40 - 70 - 8, 150, 70, GetColor(0xe66666ff));
                DrawRectangle(128 + 40 + 30 + 8 + 175, 128 + 40 - 70, 150 - 16, 70 - 16, GetColor(0xf0d6d6ff));
                DrawText("SHAPES", 128 + 40 + 30 + 8 + 175 + 28, 128 + 40 - 50, 20, GetColor(0xcd5757ff));

                DrawRectangle(128 + 40 + 30 + 175*2, 128 + 40 - 70 - 8, 150, 70, GetColor(0x75a06dff));
                DrawRectangle(128 + 40 + 30 + 8 + 175*2, 128 + 40 - 70, 150 - 16, 70 - 16, GetColor(0xc8eabfff));
                DrawText("TEXTURES", 128 + 40 + 30 + 175*2 + 8 + 9, 128 + 40 - 50, 20, GetColor(0x60815aff));

                DrawRectangle(128 + 40 + 30 + 175*3, 128 + 40 - 70 - 8, 150, 70, GetColor(0x52b296ff));
                DrawRectangle(128 + 40 + 30 + 8 + 175*3, 128 + 40 - 70, 150 - 16, 70 - 16, GetColor(0xbef0ddff));
                DrawText("TEXT", 128 + 40 + 30 + 8 + 175*3 + 38, 128 + 40 - 50, 20, GetColor(0x377764ff));

                DrawRectangle(128 + 40 + 30 + 175*4, 128 + 40 - 70 - 8, 150, 70, GetColor(0x5d9cbdff));
                DrawRectangle(128 + 40 + 30 + 8 + 175*4, 128 + 40 - 70, 150 - 16, 70 - 16, GetColor(0xbedce8ff));
                DrawText("MODELS", 128 + 40 + 30 + 8 + 175*4 + 28, 128 + 40 - 50, 20, GetColor(0x417794ff));

                DrawRectangle(128 + 40 + 30 + 175*5, 128 + 40 - 70 - 8, 150, 70, GetColor(0xd3b157ff));
                DrawRectangle(128 + 40 + 30 + 8 + 175*5, 128 + 40 - 70, 150 - 16, 70 - 16, GetColor(0xebddaeff));
                DrawText("AUDIO", 128 + 40 + 30 + 8 + 175*5 + 36, 128 + 40 - 50, 20, GetColor(0x8c7539ff));

            } break;
            case ENDING:
            {
                // Draw ENDING screen
                DrawTextEx(fontAlagard, "LEARN VIDEOGAMES PROGRAMMING", (Vector2){ screenWidth/2 - MeasureTextEx(fontAlagard, "LEARN VIDEOGAMES PROGRAMMING", fontAlagard.size*4, 4).x/2, 80 }, fontAlagard.size*4, 4, MAROON);

                DrawTexture(raylibLogoA, logoPositionX, logoPositionY - 40, WHITE);

                DrawText(msgWeb, screenWidth/2 - MeasureText(msgWeb, 40)/2, 470, 40, DARKGRAY);

                if (framesCounter > 60) DrawText(msgCredits, screenWidth/2 - MeasureText(msgCredits, 30)/2, 550, 30, GRAY);

                if (framesCounter > 120) if ((framesCounter/30)%2) DrawText("PRESS ENTER to CONTINUE", screenWidth/2 - MeasureText("PRESS ENTER to CONTINUE", 20)/2, 640, 20, LIGHTGRAY);

            } break;
            case PONG:
            {
                // Pong
                DrawCircleV(pongBallPosition, 10, LIGHTGRAY);
                DrawRectangleRec(pongPlayerRec, GRAY);
                DrawRectangleRec(pongEnemyRec, GRAY);

                DrawText(FormatText("%02i", pongScorePlayer), 150, 10, 80, LIGHTGRAY);
                DrawText(FormatText("%02i", pongScoreEnemy), screenWidth - MeasureText("00", 80) - 150, 10, 80, LIGHTGRAY);

                if (pongPaused) if ((framesCounter/30)%2) DrawText("GAME PAUSED [P]", screenWidth/2 - 100, 40, 20, MAROON);
            } break;
            default: break;
        }

        if (currentScreen != LOADING) DrawRectangle(0, screenHeight - 10, ((float)timeCounter/(float)totalTime)*screenWidth, 10, LIGHTGRAY);

        if (onTransition) DrawTransition();

    EndDrawing();
    //----------------------------------------------------------------------------------
}