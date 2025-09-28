
/*******************************************************************************************
*
*   raylib [core_input_actions] example - presents a simple API for remapping input to actions 
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
static ActionInput _actions[MAX_ACTION] = {0};

//  combines IsKeyPressed and IsGameButtonPressed to one action
bool isActionPressed(int action)
{
    if (action<MAX_ACTION)
        return (IsKeyPressed(_actions[action].key) || IsGamepadButtonPressed(gamepadIndex, _actions[action].button));
    return (false);
}

//  combines IsKeyReleased and IsGameButtonReleased to one action
bool isActionReleased(int action)
{
    if (action<MAX_ACTION)  
        return (IsKeyReleased(_actions[action].key) || IsGamepadButtonReleased(gamepadIndex, _actions[action].button));
    return (false);
}

//  combines IsKeyDown and IsGameButtonDown to one action
bool isActionDown(int action) 
{
    if (action<MAX_ACTION)  
        return (IsKeyDown(_actions[action].key) || IsGamepadButtonDown(gamepadIndex, _actions[action].button));
    return (false);
}
//  define the "default" keyset. here WASD and gamepad buttons on the left side for movement
void DefaultActions()
{
    _actions[ACTION_UP].key = KEY_W;
    _actions[ACTION_DOWN].key = KEY_S;
    _actions[ACTION_LEFT].key = KEY_A;
    _actions[ACTION_RIGHT].key = KEY_D;
    _actions[ACTION_FIRE].key = KEY_SPACE;

    _actions[ACTION_UP].button = GAMEPAD_BUTTON_LEFT_FACE_UP;
    _actions[ACTION_DOWN].button = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
    _actions[ACTION_LEFT].button = GAMEPAD_BUTTON_LEFT_FACE_LEFT;
    _actions[ACTION_RIGHT].button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT;
    _actions[ACTION_FIRE].button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
}

//  define the "alternate" keyset. here Cursor Keys and gamepad buttons on the right side for movement
void CursorActions()
{
    _actions[ACTION_UP].key = KEY_UP;
    _actions[ACTION_DOWN].key = KEY_DOWN;
    _actions[ACTION_LEFT].key = KEY_LEFT;
    _actions[ACTION_RIGHT].key = KEY_RIGHT;
    _actions[ACTION_FIRE].key = KEY_SPACE;

    _actions[ACTION_UP].button = GAMEPAD_BUTTON_RIGHT_FACE_UP;
    _actions[ACTION_DOWN].button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
    _actions[ACTION_LEFT].button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT;
    _actions[ACTION_RIGHT].button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT;
    _actions[ACTION_FIRE].button = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
}

int main(int argc, char **argv)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - input via actions");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    //  set defaul actions 
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
    CloseWindow(); // Close window and OpenGL context
    return 0;
}