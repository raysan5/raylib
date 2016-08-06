-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Storage save/load values
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

-- NOTE: Storage positions must start with 0, directly related to file memory layout
STORAGE_SCORE = 0 
STORAGE_HISCORE = 1

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [core] example - storage save/load values")

local score = 0
local hiscore = 0

local framesCounter = 0

SetTargetFPS(60)       -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    if (IsKeyPressed(KEY.R)) then
        score = GetRandomValue(1000, 2000)
        hiscore = GetRandomValue(2000, 4000)
    end
    
    if (IsKeyPressed(KEY.ENTER)) then
        StorageSaveValue(STORAGE_SCORE, score)
        StorageSaveValue(STORAGE_HISCORE, hiscore)
    elseif (IsKeyPressed(KEY.SPACE)) then
        -- NOTE: If requested position could not be found, value 0 is returned
        score = StorageLoadValue(STORAGE_SCORE)
        hiscore = StorageLoadValue(STORAGE_HISCORE)
    end
    
    framesCounter = framesCounter + 1
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawText(string.format("SCORE: %i", score), 280, 130, 40, MAROON)
        DrawText(string.format("HI-SCORE: %i", hiscore), 210, 200, 50, BLACK)
        
        DrawText(string.format("frames: %i", framesCounter), 10, 10, 20, LIME)
        
        DrawText("Press R to generate random numbers", 220, 40, 20, LIGHTGRAY)
        DrawText("Press ENTER to SAVE values", 250, 310, 20, LIGHTGRAY)
        DrawText("Press SPACE to LOAD values", 252, 350, 20, LIGHTGRAY)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
-------------------------------------------------------------------------------------------