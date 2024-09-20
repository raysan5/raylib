/*******************************************************************************************
*
*   raylib [core] example - Virtual Dpad
*
*   Example originally created with raylib 5.0, last time updated with raylib 5.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2014-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

/**
 * Display a virtual DPad and handle user input screen touch
 * Usage:
 *  Call loadDpad before while loop
 *  Call onDPadTouchEvent during while loop with parameters: GetTouchX() and GetTouchY()
 *  Call updateDpad during while loop with current virtual key press
 *  Call drawDpad in while loop between BeginDrawing() and EndDrawing() (outside BeginMode2D)
 */

// Define a structure to represent an arc
typedef struct {
    Vector2 center;
    float radius;
    float startAngle;
    float endAngle;
} Arc;

// Initialize the arc rectangles
Arc upArc, downArc, leftArc, rightArc;

const float directionSectionAngle = 90.0f;
const float sectionAngleHalf = 90.0f / 2;
const float downLowerLimit = 90 - sectionAngleHalf;
const float downUpperLimit = 90 + sectionAngleHalf;
const float upLowerLimit = 270 - sectionAngleHalf;
const float upUpperLimit = 270 + sectionAngleHalf;
const float leftLowerLimit = 180 - sectionAngleHalf;
const float leftUpperLimit = 180 + sectionAngleHalf;
const float rightLowerLimit = 360 - sectionAngleHalf;
const float rightUpperLimit = sectionAngleHalf;
const Color arcColor = (Color){ 255, 255, 255, 128};
Color dpadColor, dpadColorPushed;
Rectangle arcsRect;
Vector2 point;
int currentDirection;
bool showVirtualPosition;

/**
 * Initialize DPad
 * @param location of the DPad : KEY_RIGHT or KEY_LEFT
 * @param size of the DPad (300 if set to 0)
 * @color Color of the dpad
 */
void loadDpad(int location, Color color, int size) {
    showVirtualPosition = true; // To show the limits where the user can touch the button (Can be deactivated)
    int dPadSize = size == 0 ? 300 : size;

    if (location == KEY_LEFT) {
        arcsRect = (Rectangle) {50.0f, (float) GetScreenHeight() - ((float)size + 50.0f), (float) dPadSize,
                                (float) dPadSize};
    } else {
        arcsRect = (Rectangle) {(float) GetScreenWidth() - ((float)size + 50.0f), (float) GetScreenHeight() - ((float)size + 50.0f), (float) dPadSize,
                                (float) dPadSize};
    }
    dpadColor = color;
    dpadColorPushed = ColorLerp(color, (Color){ 0, 0, 0, 255 }, 0.5f);

    // Initialize arcs (adjust values as needed)
    upArc.center = (Vector2){ arcsRect.x + arcsRect.width / 2, arcsRect.y };
    upArc.radius = arcsRect.height / 2;
    upArc.startAngle = 0.0f;
    upArc.endAngle = PI / 2;

    downArc.center = (Vector2){ arcsRect.x + arcsRect.width / 2, arcsRect.y + arcsRect.height };
    downArc.radius = arcsRect.height / 2;
    downArc.startAngle = 3 * PI / 2;
    downArc.endAngle = 2 * PI;

    leftArc.center = (Vector2){ arcsRect.x, arcsRect.y + arcsRect.height / 2 };
    leftArc.radius = arcsRect.width / 2;
    leftArc.startAngle = PI;
    leftArc.endAngle = 3 * PI / 2;

    rightArc.center = (Vector2){ arcsRect.x + arcsRect.width, arcsRect.y + arcsRect.height / 2 };
    rightArc.radius = arcsRect.width / 2;
    leftArc.startAngle = PI / 2;
    leftArc.endAngle = PI;

    currentDirection = -1;
}

/**
 * Update Dpad button when user press it
 * @param value KEY_UP, KEY_DOWN, KEY_RIGHT or KEY_LEFT
 */
void updateDpad(int value) {
    currentDirection = value;
}

/**
 * Draw arc
 * @param rect
 * @param startAngle
 * @param endAngle
 * @param fill
 * @param color
 */
void DrawArc(Rectangle rect, float startAngle, float endAngle, bool fill, Color color) {
    // Calculate center point and radius of the rectangle
    Vector2 center = { rect.x + rect.width / 2, rect.y + rect.height / 2 };
    float radius = (rect.width > rect.height) ? rect.width / 2 : rect.height / 2;

    // Draw the arc using DrawCircleSector or DrawCircleSectorLines depending of fill parameter
    if(fill) {
        DrawCircleSector(center, radius, startAngle, endAngle, 0, color);
    } else {
        DrawCircleSectorLines(center, radius, startAngle, endAngle, 0, color);
    }
}

/**
 * Draw Dpad
 */
void drawDpad(void) {
    if (showVirtualPosition) {
        DrawArc(arcsRect, upLowerLimit, upLowerLimit + directionSectionAngle,
                currentDirection == KEY_UP, arcColor);
        DrawArc(arcsRect, downLowerLimit, downLowerLimit + directionSectionAngle,
                currentDirection == KEY_DOWN, arcColor);
        DrawArc(arcsRect, rightLowerLimit, rightLowerLimit + directionSectionAngle,
                currentDirection == KEY_RIGHT, arcColor);
        DrawArc(arcsRect, leftLowerLimit, leftLowerLimit + directionSectionAngle,
                currentDirection == KEY_LEFT, arcColor);
    }

    // Draw the central square
    DrawRectangleRounded((Rectangle){arcsRect.x, arcsRect.y+((float)arcsRect.height/3), (float)arcsRect.width/3, (float)arcsRect.height/3},  0.5f, (int)0, currentDirection == KEY_LEFT ?  dpadColorPushed : dpadColor); // Left
    //DrawRectangleRounded((Rectangle){arcsRect.x+(float)arcsRect.width/3, arcsRect.y+((float)arcsRect.height/3), (float)arcsRect.width/3, (float)arcsRect.height/3},  0.5f, (int)0, dpadColor); // Center
    DrawRectangleRounded((Rectangle){arcsRect.x+(float)arcsRect.width*2/3, arcsRect.y+((float)arcsRect.height/3), (float)arcsRect.width/3, (float)arcsRect.height/3},  0.5f, (int)0, currentDirection == KEY_RIGHT ? dpadColorPushed : dpadColor); // Right
    DrawRectangleRounded((Rectangle){arcsRect.x+(float)arcsRect.width/3, arcsRect.y, (float)arcsRect.width/3, (float)arcsRect.height/3},  0.5f, (int)0, currentDirection == KEY_UP ? dpadColorPushed : dpadColor); // Up
    DrawRectangleRounded((Rectangle){arcsRect.x+(float)arcsRect.width/3, arcsRect.y+(float)arcsRect.height*2/3, (float)arcsRect.width/3, (float)arcsRect.height/3},  0.5f, (int)0, currentDirection == KEY_DOWN ? dpadColorPushed : dpadColor); // Down

    // Draw arrows
    Vector2 arrowSize = { (float) arcsRect.width / 18, (float) arcsRect.height / 18};
    Vector2 upArrow = { arcsRect.x + (float) arcsRect.width / 2, arcsRect.y + arrowSize.y};
    Vector2 downArrow = { arcsRect.x + (float) arcsRect.width / 2 - arrowSize.x, arcsRect.y + arcsRect.height - arrowSize.y * 2};
    Vector2 leftArrow = { arcsRect.x + arrowSize.x * 2, arcsRect.y + (float) arcsRect.height / 2 - arrowSize.y };
    Vector2 rightArrow = { arcsRect.x + (float) arcsRect.width - arrowSize.x * 2, arcsRect.y + (float) arcsRect.height / 2 - arrowSize.y };

    // Up arrow
    DrawTriangle(upArrow,(Vector2){upArrow.x-arrowSize.x,upArrow.y+arrowSize.y},(Vector2){upArrow.x+arrowSize.x,upArrow.y+arrowSize.y}, BLACK);
    // Down arrow
    DrawTriangle(downArrow,(Vector2){downArrow.x+arrowSize.x,downArrow.y+arrowSize.y},(Vector2){ downArrow.x+arrowSize.x*2, downArrow.y}, BLACK);
    // Left arrow
    DrawTriangle(leftArrow,(Vector2){leftArrow.x-arrowSize.x,leftArrow.y+arrowSize.y},(Vector2){ leftArrow.x, leftArrow.y+arrowSize.y*2}, BLACK);
    // Right arrow
    DrawTriangle(rightArrow,(Vector2){rightArrow.x,rightArrow.y+arrowSize.y*2},(Vector2){ rightArrow.x+arrowSize.x,rightArrow.y+arrowSize.y}, BLACK);
}

/**
 * Get DPad button touch by user
 * @param x User x input from GetTouchX()
 * @param y User x input from GetTouchY()
 * @return KEY_UP, KEY_DOWN, KEY_RIGHT or KEY_LEFT
 */
int onDPadTouchEvent(int x, int y) {
    point.x = (float) x; point.y = (float) y;
    if (CheckCollisionPointCircle(point, upArc.center, upArc.radius)) {
        // User touched up arc
        return KEY_UP;
    } else if (CheckCollisionPointCircle(point, downArc.center, downArc.radius)) {
        // User touched down arc
        return KEY_DOWN;
    } else if (CheckCollisionPointCircle(point, rightArc.center, rightArc.radius)) {
        // User touched right arc
        return KEY_RIGHT;
    } else if (CheckCollisionPointCircle(point, leftArc.center, leftArc.radius)) {
        // User touched left arc
        return KEY_LEFT;
    }
    return -1;
}



//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    int currentKey;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - virtual Dpad");

    loadDpad(KEY_LEFT, (Color){ 125, 127, 244, 255 }, 300);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        currentKey = -1;
        if (GetTouchPointCount() > 0) {
            currentKey = onDPadTouchEvent(GetTouchX(), GetTouchY());
        }
        updateDpad(currentKey);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        drawDpad();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}