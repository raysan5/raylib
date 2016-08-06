-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Mouse input
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

InitWindow(screenWidth, screenHeight, "raylib [core] example - mouse input")

local ballPosition = Vector2(-100.0, -100.0)
local ballColor = DARKBLUE

SetTargetFPS(60)                    -- Set target frames-per-second
-----------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ------------------------------------------------------------------------------------
    ballPosition = GetMousePosition()
    
    if (IsMouseButtonPressed(MOUSE.LEFT_BUTTON)) then ballColor = MAROON
    elseif (IsMouseButtonPressed(MOUSE.MIDDLE_BUTTON)) then ballColor = LIME
    elseif (IsMouseButtonPressed(MOUSE.RIGHT_BUTTON)) then ballColor = DARKBLUE
    end
    ------------------------------------------------------------------------------------

    -- Draw
    ------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawCircleV(ballPosition, 40, ballColor)

        DrawText("move ball with mouse and click mouse button to change color", 10, 10, 20, DARKGRAY)

    EndDrawing()
    ------------------------------------------------------------------------------------
end

-- De-Initialization
----------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
----------------------------------------------------------------------------------------