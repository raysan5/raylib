/*******************************************************************************************
*
*   raylib [core] example - keyboard testbed
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   NOTE: raylib defined keys refer to ENG-US Keyboard layout, 
*   mapping to other layouts is up to the user
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define  KEY_REC_SPACING      4       // Space in pixels between key rectangles

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static const char *GetKeyText(int key);
static void GuiKeyboardKey(Rectangle bounds, int key);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - keyboard testbed");
    SetExitKey(KEY_NULL); // Avoid exit on KEY_ESCAPE

    // Keyboard line 01
    int line01KeyWidths[15] = { 0 };
    for (int i = 0; i < 15; i++) line01KeyWidths[i] = 45;
    line01KeyWidths[13] = 62;   // PRINTSCREEN
    int line01Keys[15] = { 
        KEY_ESCAPE, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, 
        KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, 
        KEY_F12, KEY_PRINT_SCREEN, KEY_PAUSE 
    };
    
    // Keyboard line 02
    int line02KeyWidths[15] = { 0 };
    for (int i = 0; i < 15; i++) line02KeyWidths[i] = 45;
    line02KeyWidths[0] = 25;    // GRAVE
    line02KeyWidths[13] = 82;   // BACKSPACE
    int line02Keys[15] = { 
        KEY_GRAVE, KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, 
        KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE, 
        KEY_ZERO, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_DELETE };

    // Keyboard line 03
    int line03KeyWidths[15] = { 0 };
    for (int i = 0; i < 15; i++) line03KeyWidths[i] = 45;
    line03KeyWidths[0] = 50;    // TAB
    line03KeyWidths[13] = 57;   // BACKSLASH
    int line03Keys[15] = {
        KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y,
        KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFT_BRACKET,
        KEY_RIGHT_BRACKET, KEY_BACKSLASH, KEY_INSERT
    };

    // Keyboard line 04
    int line04KeyWidths[14] = { 0 };
    for (int i = 0; i < 14; i++) line04KeyWidths[i] = 45;
    line04KeyWidths[0] = 68;    // CAPS
    line04KeyWidths[12] = 88;   // ENTER
    int line04Keys[14] = {
        KEY_CAPS_LOCK, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G,
        KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,
        KEY_APOSTROPHE, KEY_ENTER, KEY_PAGE_UP
    };

    // Keyboard line 05
    int line05KeyWidths[14] = { 0 };
    for (int i = 0; i < 14; i++) line05KeyWidths[i] = 45;
    line05KeyWidths[0] = 80;    // LSHIFT
    line05KeyWidths[11] = 76;   // RSHIFT
    int line05Keys[14] = {
        KEY_LEFT_SHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B,
        KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, /*KEY_MINUS*/
        KEY_SLASH, KEY_RIGHT_SHIFT, KEY_UP, KEY_PAGE_DOWN
    };

    // Keyboard line 06
    int line06KeyWidths[11] = { 0 };
    for (int i = 0; i < 11; i++) line06KeyWidths[i] = 45;
    line06KeyWidths[0] = 80;    // LCTRL
    line06KeyWidths[3] = 208;   // SPACE
    line06KeyWidths[7] = 60;    // RCTRL
    int line06Keys[11] = {
        KEY_LEFT_CONTROL, KEY_LEFT_SUPER, KEY_LEFT_ALT,
        KEY_SPACE, KEY_RIGHT_ALT, 162, KEY_NULL,
        KEY_RIGHT_CONTROL, KEY_LEFT, KEY_DOWN, KEY_RIGHT
    };
    
    Vector2 keyboardOffset = { 26, 80 };

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        int key = GetKeyPressed(); // Get pressed keycode
        if (key > 0) TraceLog(LOG_INFO, "KEYBOARD TESTBED: KEY PRESSED:    %d", key);

        int ch = GetCharPressed(); // Get pressed char for text input, using OS mapping
        if (ch > 0) TraceLog(LOG_INFO,  "KEYBOARD TESTBED: CHAR PRESSED:   %c (%d)", ch, ch);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("KEYBOARD LAYOUT: ENG-US", 26, 38, 20, LIGHTGRAY);
            
            // Keyboard line 01 - 15 keys
            // ESC, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, IMP, CLOSE
            for (int i = 0, recOffsetX = 0; i < 15; i++) 
            {
                GuiKeyboardKey((Rectangle){ keyboardOffset.x + recOffsetX, keyboardOffset.y, line01KeyWidths[i], 30 }, line01Keys[i]);
                recOffsetX += line01KeyWidths[i] + KEY_REC_SPACING;
            }
 
            // Keyboard line 02 - 15 keys
            // `, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, -, =, BACKSPACE, DEL
            for (int i = 0, recOffsetX = 0; i < 15; i++) 
            {
                GuiKeyboardKey((Rectangle){ keyboardOffset.x + recOffsetX, keyboardOffset.y + 30 + KEY_REC_SPACING, line02KeyWidths[i], 38 }, line02Keys[i]);
                recOffsetX += line02KeyWidths[i] + KEY_REC_SPACING;
            }
            
            // Keyboard line 03 - 15 keys
            // TAB, Q, W, E, R, T, Y, U, I, O, P, [, ], \, INS
            for (int i = 0, recOffsetX = 0; i < 15; i++)
            {
                GuiKeyboardKey((Rectangle){ keyboardOffset.x + recOffsetX, keyboardOffset.y + 30 + 38 + KEY_REC_SPACING*2, line03KeyWidths[i], 38 }, line03Keys[i]);
                recOffsetX += line03KeyWidths[i] + KEY_REC_SPACING;
            }

            // Keyboard line 04 - 14 keys
            // MAYUS, A, S, D, F, G, H, J, K, L, ;, ', ENTER, REPAG
            for (int i = 0, recOffsetX = 0; i < 14; i++)
            {
                GuiKeyboardKey((Rectangle){ keyboardOffset.x + recOffsetX, keyboardOffset.y + 30 + 38*2 + KEY_REC_SPACING*3, line04KeyWidths[i], 38 }, line04Keys[i]);
                recOffsetX += line04KeyWidths[i] + KEY_REC_SPACING;
            }

            // Keyboard line 05 - 14 keys
            // LSHIFT, Z, X, C, V, B, N, M, ,, ., /, RSHIFT, UP, AVPAG
            for (int i = 0, recOffsetX = 0; i < 14; i++)
            {
                GuiKeyboardKey((Rectangle){ keyboardOffset.x + recOffsetX, keyboardOffset.y + 30 + 38*3 + KEY_REC_SPACING*4, line05KeyWidths[i], 38 }, line05Keys[i]);
                recOffsetX += line05KeyWidths[i] + KEY_REC_SPACING;
            }

            // Keyboard line 06 - 11 keys
            // LCTRL, WIN, LALT, SPACE, ALTGR, \, FN, RCTRL, LEFT, DOWN, RIGHT
            for (int i = 0, recOffsetX = 0; i < 11; i++)
            {
                GuiKeyboardKey((Rectangle){ keyboardOffset.x + recOffsetX, keyboardOffset.y + 30 + 38*4 + KEY_REC_SPACING*5, line06KeyWidths[i], 38 }, line06Keys[i]);
                recOffsetX += line06KeyWidths[i] + KEY_REC_SPACING;
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definition
//------------------------------------------------------------------------------------
// Get keyboard keycode as text (US keyboard)
// NOTE: Mapping for other keyboard layouts can be done here
static const char *GetKeyText(int key)
{
    switch (key)
    {
        case KEY_APOSTROPHE      : return "'";          // Key: '
        case KEY_COMMA           : return ",";          // Key: ,
        case KEY_MINUS           : return "-";          // Key: -
        case KEY_PERIOD          : return ".";          // Key: .
        case KEY_SLASH           : return "/";          // Key: /
        case KEY_ZERO            : return "0";          // Key: 0
        case KEY_ONE             : return "1";          // Key: 1
        case KEY_TWO             : return "2";          // Key: 2
        case KEY_THREE           : return "3";          // Key: 3
        case KEY_FOUR            : return "4";          // Key: 4
        case KEY_FIVE            : return "5";          // Key: 5
        case KEY_SIX             : return "6";          // Key: 6
        case KEY_SEVEN           : return "7";          // Key: 7
        case KEY_EIGHT           : return "8";          // Key: 8
        case KEY_NINE            : return "9";          // Key: 9
        case KEY_SEMICOLON       : return ";";          // Key: ;
        case KEY_EQUAL           : return "=";          // Key: =
        case KEY_A               : return "A";          // Key: A | a
        case KEY_B               : return "B";          // Key: B | b
        case KEY_C               : return "C";          // Key: C | c
        case KEY_D               : return "D";          // Key: D | d
        case KEY_E               : return "E";          // Key: E | e
        case KEY_F               : return "F";          // Key: F | f
        case KEY_G               : return "G";          // Key: G | g
        case KEY_H               : return "H";          // Key: H | h
        case KEY_I               : return "I";          // Key: I | i
        case KEY_J               : return "J";          // Key: J | j
        case KEY_K               : return "K";          // Key: K | k
        case KEY_L               : return "L";          // Key: L | l
        case KEY_M               : return "M";          // Key: M | m
        case KEY_N               : return "N";          // Key: N | n
        case KEY_O               : return "O";          // Key: O | o
        case KEY_P               : return "P";          // Key: P | p
        case KEY_Q               : return "Q";          // Key: Q | q
        case KEY_R               : return "R";          // Key: R | r
        case KEY_S               : return "S";          // Key: S | s
        case KEY_T               : return "T";          // Key: T | t
        case KEY_U               : return "U";          // Key: U | u
        case KEY_V               : return "V";          // Key: V | v
        case KEY_W               : return "W";          // Key: W | w
        case KEY_X               : return "X";          // Key: X | x
        case KEY_Y               : return "Y";          // Key: Y | y
        case KEY_Z               : return "Z";          // Key: Z | z
        case KEY_LEFT_BRACKET    : return "[";          // Key: [
        case KEY_BACKSLASH       : return "\\";         // Key: '\'
        case KEY_RIGHT_BRACKET   : return "]";          // Key: ]
        case KEY_GRAVE           : return "`";          // Key: `
        case KEY_SPACE           : return "SPACE";      // Key: Space
        case KEY_ESCAPE          : return "ESC";        // Key: Esc
        case KEY_ENTER           : return "ENTER";      // Key: Enter
        case KEY_TAB             : return "TAB";        // Key: Tab
        case KEY_BACKSPACE       : return "BACK";       // Key: Backspace
        case KEY_INSERT          : return "INS";        // Key: Ins
        case KEY_DELETE          : return "DEL";        // Key: Del
        case KEY_RIGHT           : return "RIGHT";      // Key: Cursor right
        case KEY_LEFT            : return "LEFT";       // Key: Cursor left
        case KEY_DOWN            : return "DOWN";       // Key: Cursor down
        case KEY_UP              : return "UP";         // Key: Cursor up
        case KEY_PAGE_UP         : return "PGUP";       // Key: Page up
        case KEY_PAGE_DOWN       : return "PGDOWN";     // Key: Page down
        case KEY_HOME            : return "HOME";       // Key: Home
        case KEY_END             : return "END";        // Key: End
        case KEY_CAPS_LOCK       : return "CAPS";       // Key: Caps lock
        case KEY_SCROLL_LOCK     : return "LOCK";       // Key: Scroll down
        case KEY_NUM_LOCK        : return "NUMLOCK";    // Key: Num lock
        case KEY_PRINT_SCREEN    : return "PRINTSCR";   // Key: Print screen
        case KEY_PAUSE           : return "PAUSE";      // Key: Pause
        case KEY_F1              : return "F1";         // Key: F1
        case KEY_F2              : return "F2";         // Key: F2
        case KEY_F3              : return "F3";         // Key: F3
        case KEY_F4              : return "F4";         // Key: F4
        case KEY_F5              : return "F5";         // Key: F5
        case KEY_F6              : return "F6";         // Key: F6
        case KEY_F7              : return "F7";         // Key: F7
        case KEY_F8              : return "F8";         // Key: F8
        case KEY_F9              : return "F9";         // Key: F9
        case KEY_F10             : return "F10";        // Key: F10
        case KEY_F11             : return "F11";        // Key: F11
        case KEY_F12             : return "F12";        // Key: F12
        case KEY_LEFT_SHIFT      : return "LSHIFT";     // Key: Shift left
        case KEY_LEFT_CONTROL    : return "LCTRL";      // Key: Control left
        case KEY_LEFT_ALT        : return "LALT";       // Key: Alt left
        case KEY_LEFT_SUPER      : return "WIN";        // Key: Super left
        case KEY_RIGHT_SHIFT     : return "RSHIFT";     // Key: Shift right
        case KEY_RIGHT_CONTROL   : return "RCTRL";      // Key: Control right
        case KEY_RIGHT_ALT       : return "ALTGR";      // Key: Alt right
        case KEY_RIGHT_SUPER     : return "RSUPER";     // Key: Super right
        case KEY_KB_MENU         : return "KBMENU";     // Key: KB menu
        case KEY_KP_0            : return "KP0";        // Key: Keypad 0
        case KEY_KP_1            : return "KP1";        // Key: Keypad 1
        case KEY_KP_2            : return "KP2";        // Key: Keypad 2
        case KEY_KP_3            : return "KP3";        // Key: Keypad 3
        case KEY_KP_4            : return "KP4";        // Key: Keypad 4
        case KEY_KP_5            : return "KP5";        // Key: Keypad 5
        case KEY_KP_6            : return "KP6";        // Key: Keypad 6
        case KEY_KP_7            : return "KP7";        // Key: Keypad 7
        case KEY_KP_8            : return "KP8";        // Key: Keypad 8
        case KEY_KP_9            : return "KP9";        // Key: Keypad 9
        case KEY_KP_DECIMAL      : return "KPDEC";      // Key: Keypad .
        case KEY_KP_DIVIDE       : return "KPDIV";      // Key: Keypad /
        case KEY_KP_MULTIPLY     : return "KPMUL";      // Key: Keypad *
        case KEY_KP_SUBTRACT     : return "KPSUB";      // Key: Keypad -
        case KEY_KP_ADD          : return "KPADD";      // Key: Keypad +
        case KEY_KP_ENTER        : return "KPENTER";    // Key: Keypad Enter
        case KEY_KP_EQUAL        : return "KPEQU";      // Key: Keypad =
        default: return "";
    }
}

// Draw keyboard key
static void GuiKeyboardKey(Rectangle bounds, int key)
{
    if (key == KEY_NULL) DrawRectangleLinesEx(bounds, 2.0f, LIGHTGRAY);
    else
    {
        if (IsKeyDown(key))
        {
            DrawRectangleLinesEx(bounds, 2.0f, MAROON);
            DrawText(GetKeyText(key), bounds.x + 4, bounds.y + 4, 10, MAROON);
        }
        else
        {
            DrawRectangleLinesEx(bounds, 2.0f, DARKGRAY);
            DrawText(GetKeyText(key), bounds.x + 4, bounds.y + 4, 10, DARKGRAY);
        }
    }
    
    if (CheckCollisionPointRec(GetMousePosition(), bounds)) 
    {
        DrawRectangleRec(bounds, Fade(RED, 0.2f));
        DrawRectangleLinesEx(bounds, 3.0f, RED);
    }
}