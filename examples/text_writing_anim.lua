-------------------------------------------------------------------------------------------
--
--  raylib [text] example - Text Writing Animation
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

InitWindow(screenWidth, screenHeight, "raylib [text] example - text writing anim")

local message = "This sample illustrates a text writing\nanimation effect! Check it out! )"

local framesCounter = 0

SetTargetFPS(60)            -- Set target frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    framesCounter = framesCounter + 1
    
    if (IsKeyPressed(KEY.ENTER)) then framesCounter = 0 end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawText(string.sub(message, 0, framesCounter//10), 210, 160, 20, MAROON)
        
        DrawText("PRESS [ENTER] to RESTART!", 240, 280, 20, LIGHTGRAY)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------   
CloseWindow()        -- Close window and OpenGL context
-------------------------------------------------------------------------------------------