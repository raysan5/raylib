/*******************************************************************************************
*
*   raylib [core] example - input actions
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Jett (@JettMonstersGoBoom) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Jett (@JettMonstersGoBoom)
*
********************************************************************************************/

// Simple example for decoding input as actions, allowing remapping of input to different keys or gamepad buttons
// For example instead of using `IsKeyDown(KEY_LEFT)`, you can use `IsActionDown(ACTION_LEFT)`
// which can be reassigned to e.g. KEY_A and also assigned to a gamepad button. the action will trigger with either gamepad or keys

#include "raylib.h"

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum ActionType {
    NO_ACTION = 0,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_FIRE,
    MAX_ACTION
} ActionType;

// Key and button inputs
typedef struct ActionInput {
    int key;
    int button;
} ActionInput;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static int gamepadIndex = 0; // Gamepad default index
static ActionInput actionInputs[MAX_ACTION] = { 0 };

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static bool IsActionPressed(int action);    // Check action key/button pressed
static bool IsActionReleased(int action);   // Check action key/button released
static bool IsActionDown(int action);       // Check action key/button down

static void SetActionsDefault(void);        // Set the "default" keyset
static void SetActionsCursor(void);         // Set the "alternate" keyset

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - input actions");

    // Set default actions
    char actionSet = 0;
    SetActionsDefault();
    bool releaseAction = false;

    Vector2 position = (Vector2){ 400.0f, 200.0f };
    Vector2 size = (Vector2){ 40.0f, 40.0f };

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        gamepadIndex = 0; //  Set gamepad being checked

        if (IsActionDown(ACTION_UP)) position.y -= 2;
        if (IsActionDown(ACTION_DOWN)) position.y += 2;
        if (IsActionDown(ACTION_LEFT)) position.x -= 2;
        if (IsActionDown(ACTION_RIGHT)) position.x += 2;
        if (IsActionPressed(ACTION_FIRE))
        {
            position.x = (screenWidth-size.x)/2;
            position.y = (screenHeight-size.y)/2;
        }

        // Register release action for one frame
        releaseAction = false;
        if (IsActionReleased(ACTION_FIRE)) releaseAction = true;

        // Switch control scheme by pressing TAB
        if (IsKeyPressed(KEY_TAB))
        {
            actionSet = !actionSet;
            if (actionSet == 0) SetActionsDefault();
            else SetActionsCursor();
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(GRAY);

            DrawRectangleV(position, size, releaseAction? BLUE : RED);

            DrawText((actionSet == 0)? "Current input set: WASD (default)" : "Current input set: Cursor", 10, 10, 20, WHITE);
            DrawText("Use TAB key to toggles Actions keyset", 10, 50, 20, GREEN);

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
// Check action key/button pressed
// NOTE: Combines key pressed and gamepad button pressed in one action
static bool IsActionPressed(int action)
{
    bool result = false;

    if (action < MAX_ACTION) result = (IsKeyPressed(actionInputs[action].key) || IsGamepadButtonPressed(gamepadIndex, actionInputs[action].button));

    return result;
}

// Check action key/button released
// NOTE: Combines key released and gamepad button released in one action
static bool IsActionReleased(int action)
{
    bool result = false;

    if (action < MAX_ACTION) result = (IsKeyReleased(actionInputs[action].key) || IsGamepadButtonReleased(gamepadIndex, actionInputs[action].button));

    return result;
}

// Check action key/button down
// NOTE: Combines key down and gamepad button down in one action
static bool IsActionDown(int action)
{
    bool result = false;

    if (action < MAX_ACTION) result = (IsKeyDown(actionInputs[action].key) || IsGamepadButtonDown(gamepadIndex, actionInputs[action].button));

    return result;
}

// Set the "default" keyset
// NOTE: Here WASD and gamepad buttons on the left side for movement
static void SetActionsDefault(void)
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

// Set the "alternate" keyset
// NOTE: Here cursor keys and gamepad buttons on the right side for movement
static void SetActionsCursor(void)
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
