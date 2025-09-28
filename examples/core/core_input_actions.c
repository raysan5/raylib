
/*******************************************************************************************
*
*   raylib [core_inputactionInputs] example - presents a simple API for remapping input to actions 
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by MonstersGoBoom and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 MonstersGoBoom
*
********************************************************************************************/

/*
    Simple example for decoding input as actions, allowing remapping of input to different keys or gamepad buttons.
    for example instead of 
        IsKeyDown(KEY_LEFT)
    you'd use
        IsActionDown(ACTION_LEFT)
    which can be reassigned to e.g. KEY_A and also assigned to a gamepad button. the action will trigger with either gamepad or keys 
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "raylib.h"

//  add your own action types here

typedef enum ActionType
{
    NO_ACTION,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_FIRE,
    MAX_ACTION
} ActionType;

//  struct for key and button inputs
typedef struct ActionInput
{
    int key;
    int button;
} ActionInput;

//  gamepad index, change this if you have multiple gamepads. 
int gamepadIndex = 0;
static ActionInput actionInputs[MAX_ACTION] = {0};

//  combines IsKeyPressed and IsGameButtonPressed to one action
bool isActionPressed(int action)
{
    if (action<MAX_ACTION)
        return (IsKeyPressed(actionInputs[action].key) || IsGamepadButtonPressed(gamepadIndex, actionInputs[action].button));
    return (false);
}

//  combines IsKeyReleased and IsGameButtonReleased to one action
bool isActionReleased(int action)
{
    if (action<MAX_ACTION)  
        return (IsKeyReleased(actionInputs[action].key) || IsGamepadButtonReleased(gamepadIndex, actionInputs[action].button));
    return (false);
}

//  combines IsKeyDown and IsGameButtonDown to one action
bool isActionDown(int action) 
{
    if (action<MAX_ACTION)  
        return (IsKeyDown(actionInputs[action].key) || IsGamepadButtonDown(gamepadIndex, actionInputs[action].button));
    return (false);
}
//  define the "default" keyset. here WASD and gamepad buttons on the left side for movement
void DefaultActions()
{
    actionInputs[ACTION_UP].key = KEY_W;
    actionInputs[ACTION_DOWN].key = KEY_S;
    actionInputs[ACTION_LEFT].key = KEY_A;
    actionInputs[ACTION_RIGHT].key = KEY_D;
    actionInputs[ACTION_FIRE].key = KEY_SPACE;

    actionInputs[ACTION_UP].button = GAMEPAD_BUTTON_LEFT_FACE_UP;
    actionInputs[ACTION_DOWN].button = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
    actionInputs[ACTION_LEFT].button = GAMEPAD_BUTTON_LEFT_FACE_LEFT;
    actionInputs[ACTION_RIGHT].button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT;
    actionInputs[ACTION_FIRE].button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
}

//  define the "alternate" keyset. here Cursor Keys and gamepad buttons on the right side for movement
void CursorActions()
{
    actionInputs[ACTION_UP].key = KEY_UP;
    actionInputs[ACTION_DOWN].key = KEY_DOWN;
    actionInputs[ACTION_LEFT].key = KEY_LEFT;
    actionInputs[ACTION_RIGHT].key = KEY_RIGHT;
    actionInputs[ACTION_FIRE].key = KEY_SPACE;

    actionInputs[ACTION_UP].button = GAMEPAD_BUTTON_RIGHT_FACE_UP;
    actionInputs[ACTION_DOWN].button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
    actionInputs[ACTION_LEFT].button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT;
    actionInputs[ACTION_RIGHT].button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT;
    actionInputs[ACTION_FIRE].button = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - input via actions");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    //  set default actions 
    char actionSet = 0;
    DefaultActions();

    Vector2 position = (Vector2){100, 100};
    Vector2 size = (Vector2){32, 32};

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawText(actionSet == 0 ? "WASD Default Set" : "Cursor Set", 0, 0, 18, WHITE);
        DrawText("Tab key toggles keyset", 0, 18, 18, WHITE);
        DrawRectangleV(position, size, RED);
        EndDrawing();

        gamepadIndex = 0; //  set this to gamepad being checked
        if (isActionDown(ACTION_UP))
            position.y -= 2;
        if (isActionDown(ACTION_DOWN))
            position.y += 2;
        if (isActionDown(ACTION_LEFT))
            position.x -= 2;
        if (isActionDown(ACTION_RIGHT))
            position.x += 2;
        if (isActionPressed(ACTION_FIRE))
        {
            position.x = (screenWidth-size.x)/2;
            position.y = (screenHeight-size.y)/2;
        }

        //  switch control scheme by pressing TAB 
        if (IsKeyPressed(KEY_TAB))
        {
            actionSet = !actionSet;
            if (actionSet == 0)
                DefaultActions();
            else
                CursorActions();
        }
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------    
    CloseWindow(); // Close window and OpenGL context
    return 0;
}