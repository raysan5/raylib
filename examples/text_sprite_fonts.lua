-------------------------------------------------------------------------------------------
--
--  raylib [text] example - SpriteFont loading and usage
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

InitWindow(screenWidth, screenHeight, "raylib [text] example - sprite fonts usage")

local msg1 = "THIS IS A custom SPRITE FONT..."
local msg2 = "...and this is ANOTHER CUSTOM font..."
local msg3 = "...and a THIRD one! GREAT! :D"

-- NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)
local font1 = LoadSpriteFont("resources/fonts/custom_mecha.png")          -- SpriteFont loading
local font2 = LoadSpriteFont("resources/fonts/custom_alagard.png")        -- SpriteFont loading
local font3 = LoadSpriteFont("resources/fonts/custom_jupiter_crash.png")  -- SpriteFont loading

local fontPosition1 = Vector2(0, 0)
local fontPosition2 = Vector2(0, 0)
local fontPosition3 = Vector2(0, 0)

fontPosition1.x = screenWidth/2 - MeasureTextEx(font1, msg1, font1.size, -3).x/2
fontPosition1.y = screenHeight/2 - font1.size/2 - 80

fontPosition2.x = screenWidth/2 - MeasureTextEx(font2, msg2, font2.size, -2).x/2
fontPosition2.y = screenHeight/2 - font2.size/2 - 10

fontPosition3.x = screenWidth/2 - MeasureTextEx(font3, msg3, font3.size, 2).x/2
fontPosition3.y = screenHeight/2 - font3.size/2 + 50

SetTargetFPS(60)            -- Set target frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    -- TODO: Update variables here...
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawTextEx(font1, msg1, fontPosition1, font1.size, -3, WHITE)
        DrawTextEx(font2, msg2, fontPosition2, font2.size, -2, WHITE)
        DrawTextEx(font3, msg3, fontPosition3, font3.size, 2, WHITE)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadSpriteFont(font1)      -- SpriteFont unloading
UnloadSpriteFont(font2)      -- SpriteFont unloading
UnloadSpriteFont(font3)      -- SpriteFont unloading

CloseWindow()                -- Close window and OpenGL context
-------------------------------------------------------------------------------------------