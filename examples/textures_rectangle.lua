-------------------------------------------------------------------------------------------
--
--  raylib [textures] example - Texture loading and drawing a part defined by a rectangle
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

InitWindow(screenWidth, screenHeight, "raylib [texture] example - texture rectangle")

-- NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
local guybrush = LoadTexture("resources/guybrush.png")        -- Texture loading

local position = Vector2(350.0, 240.0)
local frameRec = Rectangle(0, 0, guybrush.width/7, guybrush.height)
local currentFrame = 0
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    if (IsKeyPressed(KEY.RIGHT)) then
        currentFrame = currentFrame + 1
        
        if (currentFrame > 6) then currentFrame = 0 end
        
        frameRec.x = currentFrame*guybrush.width/7
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawTexture(guybrush, 35, 40, WHITE)
        DrawRectangleLines(35, 40, guybrush.width, guybrush.height, LIME)
        
        DrawTextureRec(guybrush, frameRec, position, WHITE)  -- Draw part of the texture
        
        DrawRectangleLines(35 + frameRec.x, 40 + frameRec.y, frameRec.width, frameRec.height, RED)
        
        DrawText("PRESS RIGHT KEY to", 540, 310, 10, GRAY)
        DrawText("CHANGE DRAWING RECTANGLE", 520, 330, 10, GRAY)
        
        DrawText("Guybrush Ulysses Threepwood,", 100, 300, 10, GRAY)
        DrawText("main character of the Monkey Island series", 80, 320, 10, GRAY)
        DrawText("of computer adventure games by LucasArts.", 80, 340, 10, GRAY)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadTexture(guybrush)       -- Texture unloading

CloseWindow()                 -- Close window and OpenGL context
-------------------------------------------------------------------------------------------