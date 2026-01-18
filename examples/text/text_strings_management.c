/*******************************************************************************************
*
*   raylib [text] example - strings management
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by David Buzatto (@davidbuzatto) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 David Buzatto (@davidbuzatto)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>

#define MAX_TEXT_LENGTH      100
#define MAX_TEXT_PARTICLES   100
#define FONT_SIZE             30

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct TextParticle {
    char text[MAX_TEXT_LENGTH];
    Rectangle rect;    // Boundary
    Vector2 vel;       // Velocity
    Vector2 ppos;      // Previous position
    float padding;
    float borderWidth;
    float friction;   
    float elasticity;
    Color color;
    bool grabbed;
} TextParticle;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void PrepareFirstTextParticle(const char* text, TextParticle *tps, int *particleCount);
TextParticle CreateTextParticle(const char *text, float x, float y, Color color);
void SliceTextParticle(TextParticle *tp, int particlePos, int sliceLength, TextParticle *tps, int *particleCount);
void SliceTextParticleByChar(TextParticle *tp, char charToSlice, TextParticle *tps, int *particleCount);
void ShatterTextParticle(TextParticle *tp, int particlePos, TextParticle *tps, int *particleCount);
void GlueTextParticles(TextParticle *grabbed, TextParticle *target, TextParticle *tps, int *particleCount);
void RealocateTextParticles(TextParticle *tps, int particlePos, int *particleCount);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - strings management");

    TextParticle textParticles[MAX_TEXT_PARTICLES] = { 0 };
    int particleCount = 0;
    TextParticle *grabbedTextParticle = NULL;
    Vector2 pressOffset = {0};

    PrepareFirstTextParticle("raylib => fun videogames programming!", textParticles, &particleCount);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        float delta = GetFrameTime();
        Vector2 mousePos = GetMousePosition();

        // Checks if a text particle was grabbed
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            for (int i = particleCount - 1; i >= 0; i--)
            {
                TextParticle *tp = &textParticles[i];
                pressOffset.x = mousePos.x - tp->rect.x;
                pressOffset.y = mousePos.y - tp->rect.y;
                if (CheckCollisionPointRec(mousePos, tp->rect))
                {
                    tp->grabbed = true;
                    grabbedTextParticle = tp;
                    break;
                }
            }
        }

        // Releases any text particle the was grabbed
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            if (grabbedTextParticle != NULL)
            {
                grabbedTextParticle->grabbed = false;
                grabbedTextParticle = NULL;
            }
        }

        // Slice os shatter a text particle
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            for (int i = particleCount - 1; i >= 0; i--)
            {
                TextParticle *tp = &textParticles[i];
                if (CheckCollisionPointRec(mousePos, tp->rect))
                {
                    if (IsKeyDown(KEY_LEFT_SHIFT))
                    {
                        ShatterTextParticle(tp, i, textParticles, &particleCount);
                    } 
                    else
                    {
                        SliceTextParticle(tp, i, TextLength(tp->text)/2, textParticles, &particleCount);
                    }
                    break;
                }
            }
        }

        // Shake text particles
        if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
        {
            for (int i = 0; i < particleCount; i++)
            {
                if (!textParticles[i].grabbed) textParticles[i].vel = (Vector2){ (float)GetRandomValue(-2000, 2000), (float)GetRandomValue(-2000, 2000) };
            }
        }

        // Reset using TextTo* functions
        if (IsKeyPressed(KEY_ONE)) PrepareFirstTextParticle("raylib => fun videogames programming!", textParticles, &particleCount);
        if (IsKeyPressed(KEY_TWO)) PrepareFirstTextParticle(TextToUpper("raylib => fun videogames programming!"), textParticles, &particleCount);
        if (IsKeyPressed(KEY_THREE)) PrepareFirstTextParticle(TextToLower("raylib => fun videogames programming!"), textParticles, &particleCount);
        if (IsKeyPressed(KEY_FOUR)) PrepareFirstTextParticle(TextToPascal("raylib_fun_videogames_programming"), textParticles, &particleCount);
        if (IsKeyPressed(KEY_FIVE)) PrepareFirstTextParticle(TextToSnake("RaylibFunVideogamesProgramming"), textParticles, &particleCount);
        if (IsKeyPressed(KEY_SIX)) PrepareFirstTextParticle(TextToCamel("raylib_fun_videogames_programming"), textParticles, &particleCount);

        // Slice by char pressed only when we have one text particle
        char charPressed = GetCharPressed();
        if ((charPressed >= 'A') && (charPressed <= 'z') && (particleCount == 1))
        {
            SliceTextParticleByChar(&textParticles[0], charPressed, textParticles, &particleCount);
        }

        // Updates each text particle state
        for (int i = 0; i < particleCount; i++)
        {
            TextParticle *tp = &textParticles[i];

            // The text particle is not grabbed
            if (!tp->grabbed) 
            {
                // text particle repositioning using the velocity
                tp->rect.x += tp->vel.x * delta;
                tp->rect.y += tp->vel.y * delta;

                // Does the text particle hit the screen right boundary?
                if ((tp->rect.x + tp->rect.width) >= screenWidth) 
                {
                    tp->rect.x = screenWidth - tp->rect.width; // Text particle repositioning
                    tp->vel.x = -tp->vel.x*tp->elasticity;  // Elasticity makes the text particle lose 10% of its velocity on hit
                } 
                // Does the text particle hit the screen left boundary?
                else if (tp->rect.x <= 0)
                { 
                    tp->rect.x = 0.0f;
                    tp->vel.x = -tp->vel.x*tp->elasticity;
                }

                // The same for y axis
                if ((tp->rect.y + tp->rect.height) >= screenHeight) 
                {
                    tp->rect.y = screenHeight - tp->rect.height;
                    tp->vel.y = -tp->vel.y*tp->elasticity;
                } 
                else if (tp->rect.y <= 0) 
                { 
                    tp->rect.y = 0.0f;
                    tp->vel.y = -tp->vel.y*tp->elasticity;
                }

                // Friction makes the text particle lose 1% of its velocity each frame
                tp->vel.x = tp->vel.x*tp->friction;
                tp->vel.y = tp->vel.y*tp->friction;
            }
            else
            {
                // Text particle repositioning using the mouse position
                tp->rect.x = mousePos.x - pressOffset.x;
                tp->rect.y = mousePos.y - pressOffset.y;

                // While the text particle is grabbed, recalculates its velocity
                tp->vel.x = (tp->rect.x - tp->ppos.x)/delta;
                tp->vel.y = (tp->rect.y - tp->ppos.y)/delta;
                tp->ppos.x = tp->rect.x;
                tp->ppos.y = tp->rect.y;

                // Glue text particles when dragging and pressing left ctrl
                if (IsKeyDown(KEY_LEFT_CONTROL))
                {
                    for (int i = 0; i < particleCount; i++)
                    {
                        if (&textParticles[i] != grabbedTextParticle && grabbedTextParticle->grabbed)
                        {
                            if (CheckCollisionRecs(grabbedTextParticle->rect, textParticles[i].rect))
                            {
                                GlueTextParticles(grabbedTextParticle, &textParticles[i], textParticles, &particleCount);
                                grabbedTextParticle = &textParticles[particleCount-1];
                            }
                        }
                    }
                }
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            for (int i = 0; i < particleCount; i++)
            {
                TextParticle *tp = &textParticles[i];
                DrawRectangleRec((Rectangle) { tp->rect.x - tp->borderWidth, tp->rect.y - tp->borderWidth, tp->rect.width + tp->borderWidth * 2, tp->rect.height + tp->borderWidth * 2 }, BLACK);
                DrawRectangleRec(tp->rect, tp->color);
                DrawText(tp->text, (int)(tp->rect.x+tp->padding), (int)(tp->rect.y+tp->padding), FONT_SIZE, BLACK);
            }

            DrawText("grab a text particle by pressing with the mouse and throw it by releasing", 10, 10, 10, DARKGRAY);
            DrawText("slice a text particle by pressing it with the mouse right button", 10, 30, 10, DARKGRAY);
            DrawText("shatter a text particle keeping left shift pressed and pressing it with the mouse right button", 10, 50, 10, DARKGRAY);
            DrawText("glue text particles by grabbing than and keeping left control pressed", 10, 70, 10, DARKGRAY);
            DrawText("1 to 6 to reset", 10, 90, 10, DARKGRAY);
            DrawText("when you have only one text particle, you can slice it by pressing a char", 10, 110, 10, DARKGRAY);
            DrawText(TextFormat("TEXT PARTICLE COUNT: %d", particleCount), 10, GetScreenHeight() - 30, 20, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void PrepareFirstTextParticle(const char* text, TextParticle *tps, int *particleCount)
{
    tps[0] = CreateTextParticle(
        text, 
        GetScreenWidth()/2.0f, 
        GetScreenHeight()/2.0f, 
        RAYWHITE
    );
    *particleCount = 1;
}

TextParticle CreateTextParticle(const char *text, float x, float y, Color color)
{
    TextParticle tp = {
        .text = "",
        .rect = { x, y, 30, 30 },
        .vel = { (float)GetRandomValue(-200, 200), (float)GetRandomValue(-200, 200) },
        .ppos = { 0 },
        .padding = 5.0f,
        .borderWidth = 5.0f,
        .friction = 0.99f,
        .elasticity = 0.9f,
        .color = color,
        .grabbed = false
    };

    TextCopy(tp.text, text);
    tp.rect.width = MeasureText(tp.text, FONT_SIZE)+tp.padding*2;
    tp.rect.height = FONT_SIZE+tp.padding*2;
    return tp;
}

void SliceTextParticle(TextParticle *tp, int particlePos, int sliceLength, TextParticle *tps, int *particleCount)
{
    int length = TextLength(tp->text);

    if((length > 1) && ((*particleCount+length) < MAX_TEXT_PARTICLES))
    {
        for (int i = 0; i < length; i += sliceLength)
        {
            const char *text = sliceLength == 1 ? TextFormat("%c", tp->text[i]) : TextSubtext(tp->text, i, sliceLength);
            tps[(*particleCount)++] = CreateTextParticle(
                text,
                tp->rect.x + i * tp->rect.width/length,
                tp->rect.y,
                (Color) { GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 }
            );
        }
        RealocateTextParticles(tps, particlePos, particleCount);
    }
}

void SliceTextParticleByChar(TextParticle *tp, char charToSlice, TextParticle *tps, int *particleCount)
{
    int tokenCount = 0;
    char **tokens = TextSplit(tp->text, charToSlice, &tokenCount);
    
    if (tokenCount > 1)
    {
        int textLength = TextLength(tp->text);
        for (int i = 0; i < textLength; i++)
        {
            if (tp->text[i] == charToSlice)
            {
                tps[(*particleCount)++] = CreateTextParticle(
                    TextFormat("%c", charToSlice),
                    tp->rect.x,
                    tp->rect.y,
                    (Color) { GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 }
                );
            }
        }
        for (int i = 0; i < tokenCount; i++)
        {
            int tokenLength = TextLength(tokens[i]);
            tps[(*particleCount)++] = CreateTextParticle(
                TextFormat("%s", tokens[i]),
                tp->rect.x + i * tp->rect.width/tokenLength,
                tp->rect.y,
                (Color) { GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 }
            );
        }
        if (tokenCount)
        {
            RealocateTextParticles(tps, 0, particleCount);
        }
    }
}

void ShatterTextParticle(TextParticle *tp, int particlePos, TextParticle *tps, int *particleCount)
{
    SliceTextParticle(tp, particlePos, 1, tps, particleCount);
}

void GlueTextParticles(TextParticle *grabbed, TextParticle *target, TextParticle *tps, int *particleCount)
{
    int p1 = -1;
    int p2 = -1;

    for (int i = 0; i < *particleCount; i++)
    {
        if (&tps[i] == grabbed) p1 = i;
        if (&tps[i] == target) p2 = i;
    }

    if ((p1 != -1) && (p2 != -1))
    {
        TextParticle tp = CreateTextParticle(
            TextFormat( "%s%s", grabbed->text, target->text),
            grabbed->rect.x,
            grabbed->rect.y,
            RAYWHITE
        );
        tp.grabbed = true;
        tps[(*particleCount)++] = tp;
        grabbed->grabbed = false;
        if (p1 < p2)
        {
            RealocateTextParticles(tps, p2, particleCount);
            RealocateTextParticles(tps, p1, particleCount);
        }
        else
        {
            RealocateTextParticles(tps, p1, particleCount);
            RealocateTextParticles(tps, p2, particleCount);
        }
    }
}

void RealocateTextParticles(TextParticle *tps, int particlePos, int *particleCount)
{
    for (int i = particlePos+1; i < *particleCount; i++)
    {
        tps[i-1] = tps[i];
    }
    (*particleCount)--;
}