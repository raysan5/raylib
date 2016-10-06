-------------------------------------------------------------------------------------------
--
--  raylib [shapes] example - raylib logo animation
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

InitWindow(screenWidth, screenHeight, "raylib [shapes] example - raylib logo animation")

local logoPositionX = screenWidth/2 - 128
local logoPositionY = screenHeight/2 - 128

local framesCounter = 0
local lettersCount = 0

local topSideRecWidth = 16
local leftSideRecHeight = 16

local bottomSideRecWidth = 16
local rightSideRecHeight = 16

local state = 0                -- Tracking animation states (State Machine)
local alpha = 1.0              -- Useful for fading

SetTargetFPS(60)               -- Set target frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    if (state == 0) then                    -- State 0: Small box blinking
        framesCounter = framesCounter + 1

        if (framesCounter == 120) then
            state = 1
            framesCounter = 0               -- Reset counter... will be used later...
        end
    elseif (state == 1) then                -- State 1: Top and left bars growing
        topSideRecWidth = topSideRecWidth + 4
        leftSideRecHeight = leftSideRecHeight + 4

        if (topSideRecWidth == 256) then state = 2 end
    elseif (state == 2) then                -- State 2: Bottom and right bars growing
        bottomSideRecWidth = bottomSideRecWidth + 4
        rightSideRecHeight = rightSideRecHeight + 4

        if (bottomSideRecWidth == 256) then state = 3 end
    elseif (state == 3) then                -- State 3: Letters appearing (one by one)
        framesCounter = framesCounter + 1

        if (framesCounter//12 == 1) then    -- Every 12 frames, one more letter!
            lettersCount = lettersCount + 1
            framesCounter = 0
        end

        if (lettersCount >= 10) then        -- When all letters have appeared, just fade out everything
            alpha = alpha - 0.02

            if (alpha <= 0.0) then
                alpha = 0.0
                state = 4
            end
        end
    elseif (state == 4) then                -- State 4: Reset and Replay
        if (IsKeyPressed(KEY.R)) then
            framesCounter = 0
            lettersCount = 0

            topSideRecWidth = 16
            leftSideRecHeight = 16

            bottomSideRecWidth = 16
            rightSideRecHeight = 16

            alpha = 1.0
            state = 0                       -- Return to State 0
        end
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        if (state == 0) then
            if ((framesCounter//15)%2 == 1) then DrawRectangle(logoPositionX, logoPositionY, 16, 16, BLACK) end
        elseif (state == 1) then
            DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, BLACK)
            DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, BLACK)
        elseif (state == 2) then
            DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, BLACK)
            DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, BLACK)

            DrawRectangle(logoPositionX + 240, logoPositionY, 16, rightSideRecHeight, BLACK)
            DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, BLACK)
        elseif (state == 3) then
            DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, Fade(BLACK, alpha))
            DrawRectangle(logoPositionX, logoPositionY + 16, 16, leftSideRecHeight - 32, Fade(BLACK, alpha))

            DrawRectangle(logoPositionX + 240, logoPositionY + 16, 16, rightSideRecHeight - 32, Fade(BLACK, alpha))
            DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, Fade(BLACK, alpha))

            DrawRectangle(screenWidth/2 - 112, screenHeight/2 - 112, 224, 224, Fade(RAYWHITE, alpha))

            DrawText(string.sub("raylib", 0, lettersCount), screenWidth/2 - 44, screenHeight/2 + 48, 50, Fade(BLACK, alpha))
        elseif (state == 4) then DrawText("[R] REPLAY", 340, 200, 20, GRAY) end

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
-------------------------------------------------------------------------------------------