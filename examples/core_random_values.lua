-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Generate random values
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

InitWindow(screenWidth, screenHeight, "raylib [core] example - generate random values")

local framesCounter = 0  -- Variable used to count frames

local randValue = GetRandomValue(-8, 5)   -- Get a random integer number between -8 and 5 (both included)

SetTargetFPS(60)       -- Set our game to run at 60 frames-per-second
----------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ------------------------------------------------------------------------------------
    framesCounter = framesCounter + 1

    -- Every two seconds (120 frames) a new random value is generated
    if (((framesCounter/120)%2) == 1) then
        randValue = GetRandomValue(-8, 5)
        framesCounter = 0
    end
    ------------------------------------------------------------------------------------

    -- Draw
    ------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawText("Every 2 seconds a new random value is generated:", 130, 100, 20, MAROON)

        DrawText(string.format("%i", randValue), 360, 180, 80, LIGHTGRAY)

    EndDrawing()
    ------------------------------------------------------------------------------------
end

-- De-Initialization
----------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
----------------------------------------------------------------------------------------