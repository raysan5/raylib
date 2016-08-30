-------------------------------------------------------------------------------------------
--
--  raylib [text] example - Text formatting
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

InitWindow(screenWidth, screenHeight, "raylib [text] example - text formatting")

local score = 100020
local hiscore = 200450
local lives = 5

SetTargetFPS(60)        -- Set target frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    -- TODO: Update your variables here
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawText(string.format("Score: %08i", score), 200, 80, 20, RED)

        DrawText(string.format("HiScore: %08i", hiscore), 200, 120, 20, GREEN)

        DrawText(string.format("Lives: %02i", lives), 200, 160, 40, BLUE)

        DrawText(string.format("Elapsed Time: %02.02f ms", GetFrameTime()*1000), 200, 220, 20, BLACK)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
-------------------------------------------------------------------------------------------