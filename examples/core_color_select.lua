-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Color selection by mouse (collision detection)
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

NUM_RECTANGLES = 21

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [core] example - color selection (collision detection)")

local colors = { DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
                 GRAY, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK, YELLOW,
                 GREEN, SKYBLUE, PURPLE, BEIGE }

local colorsRecs = {}               -- Rectangles array
local selected = {}

-- Fills colorsRecs data (for every rectangle)
for i = 1, NUM_RECTANGLES do
    colorsRecs[i] = Rectangle(0, 0, 0, 0)
    colorsRecs[i].x = 20 + 100*((i-1)%7) + 10*((i-1)%7)
    colorsRecs[i].y = 60 + 100*((i-1)//7) + 10*((i-1)//7)   -- Using floor division: //
    colorsRecs[i].width = 100
    colorsRecs[i].height = 100
    selected[i] = false
end

local mousePoint = Vector2(0, 0)

SetTargetFPS(60)               -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    mousePoint = GetMousePosition()

    for i = 1, NUM_RECTANGLES do    -- Iterate along all the rectangles
        if (CheckCollisionPointRec(mousePoint, colorsRecs[i])) then
            colors[i].a = 120
            if (IsMouseButtonPressed(MOUSE.LEFT_BUTTON)) then selected[i] = not selected[i] end
        else colors[i].a = 255 end
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        for i = 1, NUM_RECTANGLES do    -- Draw all rectangles
            DrawRectangleRec(colorsRecs[i], colors[i])

            -- Draw four rectangles around selected rectangle
            if (selected[i]) then
                DrawRectangle(colorsRecs[i].x, colorsRecs[i].y, 100, 10, RAYWHITE)        -- Square top rectangle
                DrawRectangle(colorsRecs[i].x, colorsRecs[i].y, 10, 100, RAYWHITE)        -- Square left rectangle
                DrawRectangle(colorsRecs[i].x + 90, colorsRecs[i].y, 10, 100, RAYWHITE)   -- Square right rectangle
                DrawRectangle(colorsRecs[i].x, colorsRecs[i].y + 90, 100, 10, RAYWHITE)   -- Square bottom rectangle
            end
        end

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()                -- Close window and OpenGL context
-------------------------------------------------------------------------------------------