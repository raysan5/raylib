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

#include <stdio.h>

#include <stdlib.h>                 // Required for: malloc(), free()
#include <math.h>                   // Required for: sqrtf(), asinf()

#define MAX_SAMPLES_SPEED       7   // Max speed for samples movement
#define MIN_SAMPLES_SPEED       3   // Min speed for samples movement
#define SAMPLES_SPACING       100   // Separation between samples in pixels
#define SAMPLES_MULTIPLIER    700   // Defines sample data scaling value (it would be adjusted to MAX_GAME_HEIGHT)
#define MAX_GAME_HEIGHT       400   // Defines max possible amplitude between samples (game area)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Player {
    Vector2 position;
    Vector2 speed;
    int width;
    int height;
    Color color;
} Player;

typedef struct Sample {
    Vector2 position;
    float value;            // Raw audio sample value (normalized)
    int radius;
    bool active;            // Define if sample is active (can be collected)
    bool collected;         // Define if sample has been collected
    bool renderable;        // Define if sample should be rendered
    Color color;
} Sample;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Gameplay screen global variables
static int framesCounter;
static int finishScreen;
static bool pause;

// Player variables
static Player player;
static Rectangle playerArea;    // Define player movement area (and sample collection limits)

static float warpCounter;       // Time warp counter
static float synchro;           // Calculates collected samples relation [0..1]

static int combo;
static int maxCombo;

static Rectangle waveRec;

// Samples variables
static Sample *samples;         // Game samples
static int totalSamples;        // Total game samples (proportional to waveData num samples)
static int collectedSamples;    // Samples collected by player
static int currentSample;       // Last sample to go through player collect area
static float samplesSpeed;      // All samples move at the same speed
static float waveTime;          // Total sample time in ms

// Resources variables
static Texture2D texBackground;
static Texture2D texPlayer;
static Texture2D texSampleSmall;
static Texture2D texSampleMid;
static Texture2D texSampleBig;

static RenderTexture2D waveTarget;

static Sound fxSampleOn;        // Collected sample sound
static Sound fxSampleOff;       // Miss sample sound
static Sound fxPause;           // Pause sound
// Debug variables

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void DrawSamplesMap(Sample *samples, int sampleCount, int playedSamples, Rectangle bounds, Color color);

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitGameplayScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;
    pause = false;
    endingStatus = 0;
    
    // Textures loading
    texBackground = LoadTexture("resources/textures/background_gameplay.png");
    texPlayer = LoadTexture("resources/textures/player.png");
    texSampleSmall = LoadTexture("resources/textures/sample_small.png");
    texSampleMid = LoadTexture("resources/textures/sample_mid.png");
    texSampleBig = LoadTexture("resources/textures/sample_big.png");
    
    waveRec = (Rectangle){ 32, 32, 1280 - 64, 105 };
    waveTarget = LoadRenderTexture(waveRec.width, waveRec.height);

    // Sound loading
    fxSampleOn = LoadSound("resources/audio/sample_on.wav");
    fxSampleOff = LoadSound("resources/audio/sample_off.wav");
    fxPause = LoadSound("resources/audio/pause.wav");
    
    SetSoundVolume(fxSampleOn, 0.6f);
    SetSoundVolume(fxPause, 0.5f);

    // Initialize player data
    playerArea = (Rectangle){ 200, 160, 80, 400 };
    
    player.width = 20;
    player.height = 60;
    player.speed = (Vector2){ 15, 15 };
    player.color = GOLD;
    player.position = (Vector2){ playerArea.x + playerArea.width/2 - texPlayer.width/2, 
                                 playerArea.y + playerArea.height/2 - texPlayer.height/2 };
                                 
    warpCounter = 395;
    synchro = 0.2f;
    
    combo = 0;
    maxCombo = 0;

    // Initialize wave and samples data
    Wave wave = LoadWave("resources/audio/wave.ogg");
    float *waveData = GetWaveData(wave);        // TODO: Be careful with channels!
    
    // We calculate the required parameters to adjust audio time to gameplay time
    // that way samples collected correspond to audio playing
    // Synchonization is not perfect due to possible rounding issues (float to int)
    waveTime = wave.sampleCount/wave.sampleRate;     // Total sample time in seconds
    float requiredSamples = (MAX_SAMPLES_SPEED*waveTime*60 - 1000)/SAMPLES_SPACING;
    int samplesDivision = (int)(wave.sampleCount/requiredSamples);
    
    totalSamples = wave.sampleCount/samplesDivision;
    
    // We don't need wave any more (already got waveData)
    UnloadWave(wave);
    
    collectedSamples = 0;

    // Init samples
    samples = (Sample *)malloc(totalSamples*sizeof(Sample));

    // Normalize wave data (min vs max values) to scale properly
    float minSampleValue = 0.0f;
    float maxSampleValue = 0.0f;
    
    for (int i = 0; i < totalSamples; i++)
    {
        if (waveData[i*samplesDivision] < minSampleValue) minSampleValue = waveData[i*samplesDivision];
        if (waveData[i*samplesDivision] > maxSampleValue) maxSampleValue = waveData[i*samplesDivision];
    }
    
    float sampleScaleFactor = 1.0f/(maxSampleValue - minSampleValue);  // 400 pixels maximum size

    // Initialize samples
    for (int i = 0; i < totalSamples; i++)
    {
        samples[i].value = waveData[i*samplesDivision]*sampleScaleFactor;   // Normalized value [-1.0..1.0]
        samples[i].position.x = player.position.x + 1000 + i*SAMPLES_SPACING;
        
        samples[i].position.y = GetScreenHeight()/2 + samples[i].value*SAMPLES_MULTIPLIER;
        
        if (samples[i].position.y > GetScreenHeight()/2 + MAX_GAME_HEIGHT/2) samples[i].position.y = GetScreenHeight()/2 - MAX_GAME_HEIGHT/2;
        else if (samples[i].position.y < GetScreenHeight()/2 - MAX_GAME_HEIGHT/2) samples[i].position.y = GetScreenHeight()/2 + MAX_GAME_HEIGHT/2;
        
        samples[i].radius = 6;
        samples[i].active = true;
        samples[i].collected = false;
        samples[i].color = RED;
        samples[i].renderable = false;
    }

    samplesSpeed = MAX_SAMPLES_SPEED;
    currentSample = 0;
    
    //FILE *samplesFile = fopen("resources/samples.data", "wb");
    //fwrite(samples, totalSamples*sizeof(Sample), 1, samplesFile);
    //fclose(samplesFile);

    // We already saved the samples we needed for the game, we can free waveData
    free(waveData);
    
    // Load and start playing music
    // NOTE: Music is loaded in main code base
    StopMusicStream(music);
    PlayMusicStream(music);
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
    if (IsKeyPressed('P'))
    {
        PlaySound(fxPause);
        pause = !pause;
        
        if (pause) PauseMusicStream(music);
        else ResumeMusicStream(music);
    }

    if (!pause)
    {
        framesCounter++;        // Time starts counting to awake enemies
        
        // Player movement logic (mouse)
        player.position.y = GetMousePosition().y;
        
        // Player movement logic (keyboard)
        if (IsKeyDown(KEY_W)) player.position.y -= player.speed.y;
        else if (IsKeyDown(KEY_S)) player.position.y += player.speed.y;

        // Player movement logic (gamepad)
        /*
        if (IsGamepadAvailable(GAMEPAD_PLAYER1))
        {
            Vector2 movement = { 0.0f };
            
            movement.x = GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_PS3_AXIS_LEFT_X);
            movement.y = GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_PS3_AXIS_LEFT_Y);
            
            player.position.x += movement.x*0.1f;   // Scale gamepad movement value
            player.position.y += movement.y*0.1f;   // Scale gamepad movement value
        }
        */

        // Player logic: check player area limits
        if (player.position.x < playerArea.x) player.position.x = playerArea.x;
        else if ((player.position.x + player.width) > (playerArea.x + playerArea.width)) player.position.x = playerArea.x + playerArea.width - player.width;
        
        if (player.position.y < playerArea.y) player.position.y = playerArea.y;
        else if ((player.position.y + player.height) > (playerArea.y + playerArea.height)) player.position.y = playerArea.y + playerArea.height - player.height;

        // Samples logic
        for (int i = 0; i < totalSamples; i++)
        {
            // Samples movement logic
            samples[i].position.x -= samplesSpeed;
            
            if (((samples[i].position.x + samples[i].radius) > -SAMPLES_SPACING) && 
                ((samples[i].position.x - samples[i].radius) < GetScreenWidth())) samples[i].renderable = true;
            else samples[i].renderable = false;
               
            // Samples catch logic
            if (!samples[i].collected && CheckCollisionCircleRec(samples[i].position, samples[i].radius, (Rectangle){ (int)player.position.x, (int)player.position.y, player.width, player.height }))
            {
                samples[i].collected = true;
                collectedSamples++;
                synchro += 0.02;
                
                combo++;
                if (combo > maxCombo) maxCombo = combo;
                
                if (synchro >= 1.0f) synchro = 1.0f;
                
                // Set sound pitch depending on sample position (base pitch: 1.0f)
                // NOTE: waveData[i*WAVE_SAMPLES_DIV] is scaled to [0.3..1.7]
                SetSoundPitch(fxSampleOn, samples[i].value*1.4f + 0.7f);
                
                PlaySound(fxSampleOn);
            }
            
            if ((samples[i].position.x - samples[i].radius) < player.position.x)
            {
                currentSample = i;  // Register last sample going out range
                
                if (samples[i].active)
                {
                    samples[i].active = false;
                    
                    if (!samples[i].collected)
                    {
                        synchro -= 0.05f;
                        PlaySound(fxSampleOff);
                        combo = 0;
                    }
                }
            }
        }
        
        if (IsKeyDown(KEY_SPACE) && (warpCounter > 0))
        {
            warpCounter--;
            if (warpCounter < 0) warpCounter = 0;
            
            samplesSpeed -= 0.1f;
            if (samplesSpeed <= MIN_SAMPLES_SPEED) samplesSpeed = MIN_SAMPLES_SPEED;
            
            SetMusicPitch(music, samplesSpeed/MAX_SAMPLES_SPEED);
        }
        else
        {
            warpCounter++;
            if (warpCounter > 395) warpCounter = 395;
            
            samplesSpeed += 0.1f;
            if (samplesSpeed >= MAX_SAMPLES_SPEED) samplesSpeed = MAX_SAMPLES_SPEED;
            
            SetMusicPitch(music, samplesSpeed/MAX_SAMPLES_SPEED);
        }

        // Check ending conditions
        if (currentSample >= totalSamples - 1)
        {
            endingStatus = 1;           // Win
            finishScreen = 1;
        }

        if (synchro <= 0.0f)
        {
            synchro = 0.0f;
            endingStatus = 2;           // Loose
            finishScreen = 1;
        }
    }
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
    // Draw background
    DrawTexture(texBackground, 0, 0, WHITE);

    // Screen elements drawing
    //DrawRectangleLines(playerArea.x, playerArea.y, playerArea.width, playerArea.height, BLUE);
    DrawRectangle(0, GetScreenHeight()/2 - 1, GetScreenWidth(), 2, Fade(BLUE, 0.3f));
    //DrawRectangleLines(0, GetScreenHeight()/2 - MAX_GAME_HEIGHT/2, GetScreenWidth(), MAX_GAME_HEIGHT, GRAY);

    // Draw samples
    for (int i = 0; i < totalSamples - 1; i++)
    {
        if (samples[i].renderable)
        {
            Color col = samples[i].color;
            
            if (i < (currentSample + 1)) col = Fade(DARKGRAY, 0.5f);
            else col = WHITE;
            
            if (!samples[i].collected) 
            {
                //DrawCircleV(samples[i].position, samples[i].radius, col);
                
                if (combo > 30) DrawTexture(texSampleSmall, samples[i].position.x - texSampleSmall.width/2, samples[i].position.y - texSampleSmall.height/2, col);
                else if (combo > 15) DrawTexture(texSampleMid, samples[i].position.x - texSampleMid.width/2, samples[i].position.y - texSampleMid.height/2, col);
                else DrawTexture(texSampleBig, samples[i].position.x - texSampleBig.width/2, samples[i].position.y - texSampleBig.height/2, col);
            }
            
            if (i < (currentSample + 1)) col = Fade(GRAY, 0.3f);
            else col = Fade(RED, 0.5f);
            
            // Draw line between samples
            DrawLineEx(samples[i].position, samples[i + 1].position, 3.0f, col);
        }
    }

    // Draw player
    //DrawRectangle((int)player.position.x, (int)player.position.y, player.width, player.height, player.color);
    DrawTexture(texPlayer, player.position.x - 32, player.position.y - 24, WHITE);
 
    // Draw pause message
    if (pause) DrawTextEx(font, "WAVE PAUSED", (Vector2){ 235, 400 }, font.baseSize*2, 0, WHITE);

    // Draw number of samples
    //DrawText(FormatText("%05i", collectedSamples), 900, 200, 40, GRAY);
    //DrawText(FormatText("%05i", totalSamples), 900, 250, 40, GRAY);
    DrawTextEx(font, FormatText("%05i / %05i", collectedSamples, totalSamples), (Vector2){810, 170}, font.baseSize, -2, SKYBLUE);
    
    // Draw combo
    DrawTextEx(font, FormatText("Combo: %02i [max: %02i]", combo, maxCombo), (Vector2){200, 170}, font.baseSize/2, -2, SKYBLUE);

    // Draw synchonicity level
    DrawRectangle(99, 622, 395, 32, Fade(RAYWHITE, 0.8f));
        
    if (synchro <= 0.3f) DrawRectangle(99, 622, synchro*395, 32, Fade(RED, 0.8f));
    else if (synchro <= 0.8f) DrawRectangle(99, 622, synchro*395, 32, Fade(ORANGE,0.8f));
    else if (synchro < 1.0f) DrawRectangle(99, 622, synchro*395, 32, Fade(LIME,0.8f));
    else DrawRectangle(99, 622, synchro*395, 32, Fade(GREEN, 0.9f));
    
    DrawRectangleLines(99, 622, 395, 32, MAROON);

    if (synchro == 1.0f) DrawTextEx(font, FormatText("%02i%%", (int)(synchro*100)), (Vector2){99 + 390, 600}, font.baseSize, -2, GREEN);
    else DrawTextEx(font, FormatText("%02i%%", (int)(synchro*100)), (Vector2){99 + 390, 600}, font.baseSize, -2, SKYBLUE);
    
    // Draw time warp coool-down bar
    DrawRectangle(754, 622, 395, 32, Fade(RAYWHITE, 0.8f));
    DrawRectangle(754, 622, warpCounter, 32, Fade(SKYBLUE, 0.8f));
    DrawRectangleLines(754, 622, 395, 32, DARKGRAY);
    //DrawText(FormatText("%02i%%", (int)(synchro*100)), 754 + 410, 628, 20, DARKGRAY);
    DrawTextEx(font, FormatText("%02i%%", (int)((float)warpCounter/395.0f*100.0f)), (Vector2){754 + 390, 600}, font.baseSize, -2, SKYBLUE);
    
    // Draw wave
    if (waveTarget.texture.id <= 0)     // Render target could not be loaded (OpenGL 1.1)
    {
        // Draw wave directly on screen
        DrawSamplesMap(samples, totalSamples, currentSample, waveRec, MAROON);
        DrawRectangle(waveRec.x + (int)currentSample*1215/totalSamples, waveRec.y, 2, 99, DARKGRAY);
    }
    else
    {
        // Draw wave using render target
        BeginTextureMode(waveTarget);
            ClearBackground(BLANK);
            DrawSamplesMap(samples, totalSamples, currentSample, (Rectangle){ 0, 0, waveTarget.texture.width, waveTarget.texture.height }, MAROON);
        EndTextureMode();

        // TODO: Apply antialiasing shader
        
        DrawTextureEx(waveTarget.texture, (Vector2){ waveRec.x, waveRec.y }, 0.0f, 1.0f, WHITE);
        DrawRectangle(waveRec.x + (int)currentSample*1215/totalSamples, waveRec.y, 2, 99, DARKGRAY);
    }
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    StopMusicStream(music);
    
    // Unload textures
    UnloadTexture(texBackground);
    UnloadTexture(texPlayer);
    UnloadTexture(texSampleSmall);
    UnloadTexture(texSampleMid);
    UnloadTexture(texSampleBig);
    
    UnloadRenderTexture(waveTarget);

    // Unload sounds
    UnloadSound(fxSampleOn);
    UnloadSound(fxSampleOff);
    UnloadSound(fxPause);

    free(samples);   // Unload game samples
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

// Draw samples in wave form (including already played samples in a different color!)
// NOTE: For proper visualization, MSAA x4 is recommended, alternatively
// it should be rendered to a bigger texture and then scaled down with 
// bilinear/trilinear texture filtering
static void DrawSamplesMap(Sample *samples, int sampleCount, int playedSamples, Rectangle bounds, Color color)
{
    // NOTE: We just pick a sample to draw every increment
    float sampleIncrementX = (float)bounds.width/sampleCount;

    Color col = color;
    
    for (int i = 0; i < sampleCount - 1; i++)
    {
        if (i < playedSamples) col = GRAY;
        else col = color;

        DrawLineV((Vector2){ (float)bounds.x + (float)i*sampleIncrementX, (float)(bounds.y + bounds.height/2) + samples[i].value*bounds.height }, 
                  (Vector2){ (float)bounds.x + (float)(i + 1)*sampleIncrementX, (float)(bounds.y  + bounds.height/2) + + samples[i + 1].value*bounds.height }, col);
    }
}