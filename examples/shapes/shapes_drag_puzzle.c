/*******************************************************************************************
*
*   raylib [shapes] example - drag puzzle
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
*
*   Example contributed by Gabriel Piangers (@gabriel-piangers) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Gabriel Piangers (@gabriel-piangers)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    Vector2 center;
    float radius;
} Circle;

typedef struct {
    Vector2 v1;
    Vector2 v2;
    Vector2 v3;
} Triangle;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - drag puzzle");

    // Rectangle 
    Rectangle rec = { screenWidth/2 - 250, screenHeight/2 + 50, 100.0f, 100.0f };
    Rectangle recArea = { screenWidth/2 - 60, screenHeight/2 - 110, 110.0f, 110.0f };   
    bool recPickedUp = false;

    // Circle
    Circle circ = { screenWidth/2, screenHeight/2 + 100, 50.0f };
    Circle circArea = { screenWidth/2 - 195, screenHeight/2 - 55, 55.0f };
    bool circPickedUp = false;

    // Triangle
    Triangle tri = { { 600, 282 }, { 550, 369 }, { 650, 369 } };
    Triangle triArea = { { 600, 115 }, { 540, 222 }, { 660, 222 } };
    bool triPickedUp = false;

    Vector2 mouseOffset = { 0.0f, 0.0f };    // Stores the offset of the mouse relative to the object's pivot point

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector2 mousePosition = GetMousePosition();
        bool recPlaced = false;
        bool circPlaced = false;
        bool triPlaced = false;

        // Detect object pickup input
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePosition, rec))
        {
            recPickedUp = true;
            mouseOffset = (Vector2) { rec.x - mousePosition.x, rec.y - mousePosition.y };
        } 
        else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointCircle(mousePosition, circ.center, circ.radius))
        {
            circPickedUp = true;
            mouseOffset = (Vector2) { circ.center.x - mousePosition.x, circ.center.y - mousePosition.y };
        } 
        else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointTriangle(mousePosition, tri.v1, tri.v2, tri.v3))
        {
            triPickedUp = true;
            mouseOffset = (Vector2) { tri.v1.x - mousePosition.x, tri.v1.y - mousePosition.y }; // Uses v1 as the pivot point
        }
        
        // Detect object drop input
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && recPickedUp) recPickedUp = false; 
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && circPickedUp) circPickedUp = false; 
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && triPickedUp) triPickedUp = false;

        // Rectangle update
        if (recPickedUp)
        {
            rec.x = mousePosition.x + mouseOffset.x;
            rec.y = mousePosition.y + mouseOffset.y;
        }
        Rectangle RecCol = GetCollisionRec(recArea, rec);
        if (RecCol.width == rec.width && RecCol.height == rec.height) recPlaced = true;        

        // Circle update
        if (circPickedUp)
        {
            circ.center.x = mousePosition.x + mouseOffset.x;
            circ.center.y = mousePosition.y + mouseOffset.y;
        }
        if (Vector2Distance(circ.center, circArea.center) < circArea.radius - circ.radius) circPlaced = true;

        // Triangle update
        if (triPickedUp)
        {
            Vector2 v2Offset = { tri.v2.x - tri.v1.x, tri.v2.y - tri.v1.y };
            Vector2 v3Offset = { tri.v3.x - tri.v1.x, tri.v3.y - tri.v1.y };
            
            tri.v1 = (Vector2) { mousePosition.x + mouseOffset.x, mousePosition.y + mouseOffset.y };
            tri.v2 = (Vector2) { tri.v1.x + v2Offset.x, tri.v1.y + v2Offset.y };
            tri.v3 = (Vector2) { tri.v1.x + v3Offset.x, tri.v1.y + v3Offset.y };
        }
        if (
            CheckCollisionPointTriangle(tri.v1, triArea.v1, triArea.v2, triArea.v3) &&
            CheckCollisionPointTriangle(tri.v2, triArea.v1, triArea.v2, triArea.v3) &&
            CheckCollisionPointTriangle(tri.v2, triArea.v1, triArea.v2, triArea.v3)
        ) triPlaced = true;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawRectangleLinesEx(recArea, 2.0f, recPlaced ? GREEN : RED);
            DrawCircleLinesEx(circArea.center, circArea.radius, 2.0f, circPlaced ? GREEN : RED);
            DrawTriangleLinesEx(triArea.v1, triArea.v2, triArea.v3, 2.0f, triPlaced ? GREEN : RED);

            // Draws objects that are not picked up first
            if (!triPickedUp) DrawTriangle(tri.v1, tri.v2, tri.v3, VIOLET);
            if (!circPickedUp) DrawCircleV(circ.center, circ.radius, BLUE);
            if (!recPickedUp) DrawRectangleRec(rec, ORANGE);

            // Draws the object that is being dragged on top of others
            if (triPickedUp) DrawTriangle(tri.v1, tri.v2, tri.v3, VIOLET);
            if (circPickedUp) DrawCircleV(circ.center, circ.radius, BLUE);
            if (recPickedUp) DrawRectangleRec(rec, ORANGE);

            DrawText("Use mouse to drag and drop the objects into the right spot!", 10, 10, 20, GRAY);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}