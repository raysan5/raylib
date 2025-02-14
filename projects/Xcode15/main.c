/*******************************************************************************************
*
*   raylib [core] example - Input Gestures Detection
*
*   Example originally created with raylib 1.4, last time updated with raylib 4.2
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2016-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/
#ifndef RL_IOS_NO_EXAMPLE

#include "raylib.h"

int MAX_GESTURE_STRINGS = 20;
int screenWidth = 0;
int screenHeight = 0;
Vector2 touchPosition;
Rectangle touchArea;
int gesturesCount = 0;
char gestureStrings[100][32];
int currentGesture = GESTURE_NONE;
int lastGesture = GESTURE_NONE;

void ios_ready(){
    InitWindow(0, 0, "raylib [core] example - input gestures");
    
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
    
    touchPosition = (Vector2){ 0, 0 };
    touchArea = (Rectangle){ 220, 10, screenWidth - 230.0f, screenHeight - 20.0f };
    
    //SetGesturesEnabled(0b0000000000001001);   // Enable only some gestures to be detected
    SetTargetFPS(60);
    MAX_GESTURE_STRINGS = (screenHeight - 50) / 20;
}

static void add_gesture(const char* title){
    if (gesturesCount >= MAX_GESTURE_STRINGS)
    {
        for (int i = 0; i < MAX_GESTURE_STRINGS; i++) TextCopy(gestureStrings[i], "\0");
        gesturesCount = 0;
    }
    TextCopy(gestureStrings[gesturesCount], title);
    gesturesCount++;
}

void ios_update()
{
    lastGesture = currentGesture;
    currentGesture = GetGestureDetected();
    touchPosition = GetTouchPosition(0);

    if(IsMouseButtonPressed(0)) add_gesture("MouseButtonPressed");
    if(IsMouseButtonReleased(0)) add_gesture("MouseButtonReleased");

    if (CheckCollisionPointRec(touchPosition, touchArea) && (currentGesture != GESTURE_NONE))
    {
        if (currentGesture != lastGesture)
        {
            // Store gesture string
            switch (currentGesture)
            {
                case GESTURE_TAP: add_gesture( "GESTURE TAP"); break;
                case GESTURE_DOUBLETAP: add_gesture( "GESTURE DOUBLETAP"); break;
                case GESTURE_HOLD: add_gesture( "GESTURE HOLD"); break;
                case GESTURE_DRAG: add_gesture( "GESTURE DRAG"); break;
                case GESTURE_SWIPE_RIGHT: add_gesture( "GESTURE SWIPE RIGHT"); break;
                case GESTURE_SWIPE_LEFT: add_gesture( "GESTURE SWIPE LEFT"); break;
                case GESTURE_SWIPE_UP: add_gesture( "GESTURE SWIPE UP"); break;
                case GESTURE_SWIPE_DOWN: add_gesture( "GESTURE SWIPE DOWN"); break;
                case GESTURE_PINCH_IN: add_gesture( "GESTURE PINCH IN"); break;
                case GESTURE_PINCH_OUT: add_gesture( "GESTURE PINCH OUT"); break;
                default: break;
            }
        }
    }

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawRectangleRec(touchArea, GRAY);
    DrawRectangle(225, 15, screenWidth - 240, screenHeight - 30, RAYWHITE);

    DrawText("GESTURES TEST AREA", screenWidth - 270, screenHeight - 40, 20, Fade(GRAY, 0.5f));

    for (int i = 0; i < gesturesCount; i++)
    {
        if (i % 2 == 0){
            DrawRectangle(10, 30 + 20*i, 200, 20, Fade(LIGHTGRAY, 0.5f));
        }else{
            DrawRectangle(10, 30 + 20*i, 200, 20, Fade(LIGHTGRAY, 0.3f));
        }
        if (i < gesturesCount - 1){
            DrawText(gestureStrings[i], 35, 36 + 20*i, 10, DARKGRAY);
        }else{
            DrawText(gestureStrings[i], 35, 36 + 20*i, 10, MAROON);
        }
    }

    DrawRectangleLines(10, 29, 200, screenHeight - 50, GRAY);
    DrawText(
             TextFormat("TOUCH COUNT: %d", GetTouchPointCount()),
             50, 15, 10, GRAY
             );

    for(int i=0; i < GetTouchPointCount(); i++){
        DrawCircleV(GetTouchPosition(i), 30, MAROON);
    }
    EndDrawing();
}

void ios_destroy(){
    CloseWindow();        // Close window and OpenGL context
}

#endif
