/*******************************************************************************************
*
*   raylib [core] example - undo redo
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>     // Required for: calloc(), free()
#include <string.h>     // Required for: memcpy(), strcmp()

#define MAX_UNDO_STATES             26      // Maximum undo states supported for the ring buffer

#define GRID_CELL_SIZE              24
#define MAX_GRID_CELLS_X            30
#define MAX_GRID_CELLS_Y            13

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Point struct, like Vector2 but using int
typedef struct {
    int x;
    int y;
} Point;

// Player state struct
// NOTE: Contains all player data that needs to be affected by undo/redo
typedef struct {
    Point cell;
    Color color;
} PlayerState;

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
// Draw undo system visualization logic
static void DrawUndoBuffer(Vector2 position, int firstUndoIndex, int lastUndoIndex, int currentUndoIndex, int slotSize);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    // We have multiple options to implement an Undo/Redo system
    // Probably the most professional one is using the Command pattern to
    // define Actions and store those actions into an array as the events happen,
    // raylib internal Automation System actually uses a similar approach,
    // but in this example we are using another more simple solution,
    // just record PlayerState changes when detected, checking for changes every certain frames
    // This approach requires more memory and is more performance costly but it is quite simple to implement

    InitWindow(screenWidth, screenHeight, "raylib [core] example - undo redo");

    // Undo/redo system variables
    int currentUndoIndex = 0;
    int firstUndoIndex = 0;
    int lastUndoIndex = 0;
    int undoFrameCounter = 0;
    Vector2 undoInfoPos = { 110, 400 };

    // Init current player state and undo/redo recorded states array
    PlayerState player = { 0 };
    player.cell = (Point){ 10, 10 };
    player.color = RED;

    // Init undo buffer to store MAX_UNDO_STATES states
    PlayerState *states = (PlayerState *)RL_CALLOC(MAX_UNDO_STATES, sizeof(PlayerState));
    // Init all undo states to current state
    for (int i = 0; i < MAX_UNDO_STATES; i++) memcpy(&states[i], &player, sizeof(PlayerState));

    // Grid variables
    Vector2 gridPosition = { 40, 60 };

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Player movement logic
        if (IsKeyPressed(KEY_RIGHT)) player.cell.x++;
        else if (IsKeyPressed(KEY_LEFT)) player.cell.x--;
        else if (IsKeyPressed(KEY_UP)) player.cell.y--;
        else if (IsKeyPressed(KEY_DOWN)) player.cell.y++;

        // Make sure player does not go out of bounds
        if (player.cell.x < 0) player.cell.x = 0;
        else if (player.cell.x >= MAX_GRID_CELLS_X) player.cell.x = MAX_GRID_CELLS_X - 1;
        if (player.cell.y < 0) player.cell.y = 0;
        else if (player.cell.y >= MAX_GRID_CELLS_Y) player.cell.y = MAX_GRID_CELLS_Y - 1;

        // Player color change logic
        if (IsKeyPressed(KEY_SPACE))
        {
            player.color.r = (unsigned char)GetRandomValue(20, 255);
            player.color.g = (unsigned char)GetRandomValue(20, 220);
            player.color.b = (unsigned char)GetRandomValue(20, 240);
        }

        // Undo state change logic
        undoFrameCounter++;

        // Waiting a number of frames before checking if we should store a new state snapshot
        if (undoFrameCounter >= 2) // Checking every 2 frames
        {
            if (memcmp(&states[currentUndoIndex], &player, sizeof(PlayerState)) != 0)
            {
                // Move cursor to next available position of the undo ring buffer to record state
                currentUndoIndex++;
                if (currentUndoIndex >= MAX_UNDO_STATES) currentUndoIndex = 0;
                if (currentUndoIndex == firstUndoIndex) firstUndoIndex++;
                if (firstUndoIndex >= MAX_UNDO_STATES) firstUndoIndex = 0;

                memcpy(&states[currentUndoIndex], &player, sizeof(PlayerState));
                lastUndoIndex = currentUndoIndex;
            }

            undoFrameCounter = 0;
        }

        // Recover previous state from buffer: CTRL+Z
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z))
        {
            if (currentUndoIndex != firstUndoIndex)
            {
                currentUndoIndex--;
                if (currentUndoIndex < 0) currentUndoIndex = MAX_UNDO_STATES - 1;

                if (memcmp(&states[currentUndoIndex], &player, sizeof(PlayerState)) != 0)
                {
                    memcpy(&player, &states[currentUndoIndex], sizeof(PlayerState));
                }
            }
        }

        // Recover next state from buffer: CTRL+Y
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Y))
        {
            if (currentUndoIndex != lastUndoIndex)
            {
                int nextUndoIndex = currentUndoIndex + 1;
                if (nextUndoIndex >= MAX_UNDO_STATES) nextUndoIndex = 0;

                if (nextUndoIndex != firstUndoIndex)
                {
                    currentUndoIndex = nextUndoIndex;

                    if (memcmp(&states[currentUndoIndex], &player, sizeof(PlayerState)) != 0)
                    {
                        memcpy(&player, &states[currentUndoIndex], sizeof(PlayerState));
                    }
                }
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Draw controls info
            DrawText("[ARROWS] MOVE PLAYER - [SPACE] CHANGE PLAYER COLOR", 40, 20, 20, DARKGRAY);

            // Draw player visited cells recorded by undo
            // NOTE: Remember we are using a ring buffer approach so,
            // some cells info could start at the end of the array and end at the beginning
            if (lastUndoIndex > firstUndoIndex)
            {
                for (int i = firstUndoIndex; i < currentUndoIndex; i++)
                    DrawRectangleRec((Rectangle){gridPosition.x + states[i].cell.x*GRID_CELL_SIZE, gridPosition.y + states[i].cell.y*GRID_CELL_SIZE,
                        GRID_CELL_SIZE, GRID_CELL_SIZE }, LIGHTGRAY);
            }
            else if (firstUndoIndex > lastUndoIndex)
            {
                if ((currentUndoIndex < MAX_UNDO_STATES) && (currentUndoIndex > lastUndoIndex))
                {
                    for (int i = firstUndoIndex; i < currentUndoIndex; i++)
                        DrawRectangleRec((Rectangle) { gridPosition.x + states[i].cell.x*GRID_CELL_SIZE, gridPosition.y + states[i].cell.y*GRID_CELL_SIZE,
                            GRID_CELL_SIZE, GRID_CELL_SIZE }, LIGHTGRAY);
                }
                else
                {
                    for (int i = firstUndoIndex; i < MAX_UNDO_STATES; i++)
                        DrawRectangle((int)gridPosition.x + states[i].cell.x*GRID_CELL_SIZE, (int)gridPosition.y + states[i].cell.y*GRID_CELL_SIZE,
                            GRID_CELL_SIZE, GRID_CELL_SIZE, LIGHTGRAY);
                    for (int i = 0; i < currentUndoIndex; i++)
                        DrawRectangle((int)gridPosition.x + states[i].cell.x*GRID_CELL_SIZE, (int)gridPosition.y + states[i].cell.y*GRID_CELL_SIZE,
                            GRID_CELL_SIZE, GRID_CELL_SIZE, LIGHTGRAY);
                }
            }

            // Draw game grid
            for (int y = 0; y <= MAX_GRID_CELLS_Y; y++)
                DrawLine((int)gridPosition.x, (int)gridPosition.y + y*GRID_CELL_SIZE,
                    (int)gridPosition.x + MAX_GRID_CELLS_X*GRID_CELL_SIZE, (int)gridPosition.y + y*GRID_CELL_SIZE, GRAY);
            for (int x = 0; x <= MAX_GRID_CELLS_X; x++)
                DrawLine((int)gridPosition.x + x*GRID_CELL_SIZE, (int)gridPosition.y,
                    (int)gridPosition.x + x*GRID_CELL_SIZE, (int)gridPosition.y + MAX_GRID_CELLS_Y*GRID_CELL_SIZE, GRAY);

            // Draw player
            DrawRectangle((int)gridPosition.x + player.cell.x*GRID_CELL_SIZE, (int)gridPosition.y + player.cell.y*GRID_CELL_SIZE,
                GRID_CELL_SIZE + 1, GRID_CELL_SIZE + 1, player.color);

            // Draw undo system buffer info
            DrawText("UNDO STATES:", (int)undoInfoPos.x - 85, (int)undoInfoPos.y + 9, 10, DARKGRAY);
            DrawUndoBuffer(undoInfoPos, firstUndoIndex, lastUndoIndex, currentUndoIndex, 24);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    RL_FREE(states);        // Free undo states array

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definition
//------------------------------------------------------------------------------------
// Draw undo system visualization logic
// NOTE: Visualizing the ring buffer array, every square can store a player state
static void DrawUndoBuffer(Vector2 position, int firstUndoIndex, int lastUndoIndex, int currentUndoIndex, int slotSize)
{
    // Draw index marks
    DrawRectangle((int)position.x + 8 + slotSize*currentUndoIndex, (int)position.y - 10, 8, 8, RED);
    DrawRectangleLines((int)position.x + 2 + slotSize*firstUndoIndex, (int)position.y + 27, 8, 8, BLACK);
    DrawRectangle((int)position.x + 14 + slotSize*lastUndoIndex, (int)position.y + 27, 8, 8, BLACK);

    // Draw background gray slots
    for (int i = 0; i < MAX_UNDO_STATES; i++)
    {
        DrawRectangle((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, LIGHTGRAY);
        DrawRectangleLines((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, GRAY);
    }

    // Draw occupied slots: firstUndoIndex --> lastUndoIndex
    if (firstUndoIndex <= lastUndoIndex)
    {
        for (int i = firstUndoIndex; i < lastUndoIndex + 1; i++)
        {
            DrawRectangle((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, SKYBLUE);
            DrawRectangleLines((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, BLUE);
        }
    }
    else if (lastUndoIndex < firstUndoIndex)
    {
        for (int i = firstUndoIndex; i < MAX_UNDO_STATES; i++)
        {
            DrawRectangle((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, SKYBLUE);
            DrawRectangleLines((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, BLUE);
        }

        for (int i = 0; i < lastUndoIndex + 1; i++)
        {
            DrawRectangle((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, SKYBLUE);
            DrawRectangleLines((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, BLUE);
        }
    }

    // Draw occupied slots: firstUndoIndex --> currentUndoIndex
    if (firstUndoIndex < currentUndoIndex)
    {
        for (int i = firstUndoIndex; i < currentUndoIndex; i++)
        {
            DrawRectangle((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, GREEN);
            DrawRectangleLines((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, LIME);
        }
    }
    else if (currentUndoIndex < firstUndoIndex)
    {
        for (int i = firstUndoIndex; i < MAX_UNDO_STATES; i++)
        {
            DrawRectangle((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, GREEN);
            DrawRectangleLines((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, LIME);
        }

        for (int i = 0; i < currentUndoIndex; i++)
        {
            DrawRectangle((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, GREEN);
            DrawRectangleLines((int)position.x + slotSize*i, (int)position.y, slotSize, slotSize, LIME);
        }
    }

    // Draw current selected UNDO slot
    DrawRectangle((int)position.x + slotSize*currentUndoIndex, (int)position.y, slotSize, slotSize, GOLD);
    DrawRectangleLines((int)position.x + slotSize*currentUndoIndex, (int)position.y, slotSize, slotSize, ORANGE);
}
