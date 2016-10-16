-------------------------------------------------------------------------------------------
--
--  raylib [core] examples - Mouse wheel
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [core] example - mouse wheel")

local boxPositionY = screenHeight/2 - 40
local scrollSpeed = 4               -- Scrolling speed in pixels

SetTargetFPS(60)                    -- Set target frames-per-second
----------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ------------------------------------------------------------------------------------
    boxPositionY = boxPositionY - (GetMouseWheelMove()*scrollSpeed)
    ------------------------------------------------------------------------------------

    -- Draw
    ------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawRectangle(screenWidth/2 - 40, boxPositionY, 80, 80, MAROON)

        DrawText("Use mouse wheel to move the cube up and down!", 10, 10, 20, GRAY)
        DrawText(string.format("Box position Y: %03i", boxPositionY), 10, 40, 20, LIGHTGRAY)

    EndDrawing()
    ------------------------------------------------------------------------------------
end

-- De-Initialization
----------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
----------------------------------------------------------------------------------------