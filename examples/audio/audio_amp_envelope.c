/*******************************************************************************************
*
*   raylib [audio] example - amp envelope
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
*
*   Example contributed by Arbinda Rizki Muhammad (@arbipink) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Arbinda Rizki Muhammad (@arbipink)
*
********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <math.h>           // Required for: sinf()

#define BUFFER_SIZE      4096
#define SAMPLE_RATE     44100

// Wave state
typedef enum {
    IDLE,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
} ADSRState;

// Grouping all ADSR parameters and state into a struct
typedef struct {
    float attackTime;
    float decayTime;
    float sustainLevel;
    float releaseTime;
    float currentValue;
    ADSRState state;
} Envelope;

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static void FillAudioBuffer(int i, float *buffer, float envelopeValue, float *audioTime);
static void UpdateEnvelope(Envelope *env);
static void DrawADSRGraph(Envelope *env, Rectangle bounds);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - amp envelope");

    InitAudioDevice();

    // Set the number of samples the stream will keep in memory at a time to BUFFER_SIZE
    SetAudioStreamBufferSizeDefault(BUFFER_SIZE);
    float buffer[BUFFER_SIZE] = { 0 };

    // Init raw audio stream (sample rate: 44100, sample size: 32bit-float, channels: 1-mono)
    AudioStream stream = LoadAudioStream(SAMPLE_RATE, 32, 1);

    // Init Phase
    float audioTime = 0.0f;

    // Initialize the struct
    Envelope env = {
        .attackTime = 1.0f,
        .decayTime = 1.0f,
        .sustainLevel = 0.5f,
        .releaseTime = 1.0f,
        .currentValue = 0.0f,
        .state = IDLE
    };

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_SPACE)) env.state = ATTACK;

        if (IsKeyReleased(KEY_SPACE) && (env.state != IDLE)) env.state = RELEASE;

        if (IsAudioStreamProcessed(stream))
        {
            if ((env.state != IDLE) || (env.currentValue > 0.0f))
            {
                for (int i = 0; i < BUFFER_SIZE; i++)
                {
                    UpdateEnvelope(&env);
                    FillAudioBuffer(i, buffer, env.currentValue, &audioTime);
                }
            }
            else
            {
                // Clear buffer if silent to avoid looping noise
                for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = 0;
                audioTime = 0.0f;
            }

            UpdateAudioStream(stream, buffer, BUFFER_SIZE);
        }

        if (!IsAudioStreamPlaying(stream)) PlayAudioStream(stream);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            GuiSliderBar((Rectangle){ 100, 60, 400, 30 }, "Attack (s)", TextFormat("%2.2fs", env.attackTime), &env.attackTime, 0.1f, 3.0f);
            GuiSliderBar((Rectangle){ 100, 100, 400, 30 }, "Decay (s)", TextFormat("%2.2fs", env.decayTime), &env.decayTime, 0.1f, 3.0f);
            GuiSliderBar((Rectangle){ 100, 140, 400, 30 }, "Sustain", TextFormat("%2.2f", env.sustainLevel), &env.sustainLevel, 0.0f, 1.0f);
            GuiSliderBar((Rectangle){ 100, 180, 400, 30 }, "Release (s)", TextFormat("%2.2fs", env.releaseTime), &env.releaseTime, 0.1f, 3.0f);

            DrawADSRGraph(&env, (Rectangle){ 100, 250, 400, 100 });

            DrawCircleV((Vector2){ 520, 350 - (env.currentValue * 100) }, 5, MAROON);
            DrawText(TextFormat("Current Gain: %2.2f", env.currentValue), 535, 345 - (env.currentValue * 100), 10, MAROON);

            DrawText("Press SPACE to PLAY the sound!", 200, 400, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadAudioStream(stream);
    CloseAudioDevice();

    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definition
//------------------------------------------------------------------------------------
static void FillAudioBuffer(int i, float *buffer, float envelopeValue, float *audioTime)
{
    int frequency = 440;
    buffer[i] = envelopeValue*sinf(2.0f*PI*frequency*(*audioTime));
    *audioTime += (1.0f/SAMPLE_RATE);
}

static void UpdateEnvelope(Envelope *env)
{
    // Calculate the time delta for ONE sample (1/44100)
    float sampleTime = 1.0f/SAMPLE_RATE;

    switch(env->state)
    {
        case ATTACK:
        {
            env->currentValue += (1.0f/env->attackTime)*sampleTime;
            if (env->currentValue >= 1.0f)
            {
                env->currentValue = 1.0f;
                env->state = DECAY;
            }

        } break;
        case DECAY:
        {
            env->currentValue -= ((1.0f - env->sustainLevel)/env->decayTime)*sampleTime;
            if (env->currentValue <= env->sustainLevel)
            {
                env->currentValue = env->sustainLevel;
                env->state = SUSTAIN;
            }

        } break;
        case SUSTAIN:
        {
            env->currentValue = env->sustainLevel;

        } break;
        case RELEASE:
        {
            env->currentValue -= (env->sustainLevel/env->releaseTime)*sampleTime;
            if (env->currentValue <= 0.001f) // Use a small threshold to avoid infinite tail
            {
                env->currentValue = 0.0f;
                env->state = IDLE;
            }

        } break;
        default: break;
    }
}

static void DrawADSRGraph(Envelope *env, Rectangle bounds)
{
    DrawRectangleRec(bounds, Fade(LIGHTGRAY, 0.3f));
    DrawRectangleLinesEx(bounds, 1, GRAY);

    // Fixed visual width for sustain stage since it's an amplitude not a time value
    float sustainWidth = 1.0f;

    // Total time to visualize (sum of A, D, R + a padding for Sustain)
    float totalTime = env->attackTime + env->decayTime + sustainWidth + env->releaseTime;

    float scaleX = bounds.width/totalTime;
    float scaleY = bounds.height;

    Vector2 start = { bounds.x, bounds.y + bounds.height };
    Vector2 peak = { start.x + (env->attackTime*scaleX), bounds.y };
    Vector2 sustain = { peak.x + (env->decayTime*scaleX), bounds.y + (1.0f - env->sustainLevel)*scaleY };
    Vector2 rel = { sustain.x + (sustainWidth*scaleX), sustain.y };
    Vector2 end = { rel.x + (env->releaseTime*scaleX), bounds.y + bounds.height };

    DrawLineV(start, peak, SKYBLUE);
    DrawLineV(peak, sustain, BLUE);
    DrawLineV(sustain, rel, DARKBLUE);
    DrawLineV(rel, end, ORANGE);

    DrawText("ADSR Visualizer", bounds.x, bounds.y - 20, 10, DARKGRAY);
}
