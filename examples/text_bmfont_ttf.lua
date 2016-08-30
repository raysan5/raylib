-------------------------------------------------------------------------------------------
--
--  raylib [text] example - BMFont and TTF SpriteFonts loading
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

InitWindow(screenWidth, screenHeight, "raylib [text] example - bmfont and ttf sprite fonts loading")

local msgBm = "THIS IS AN AngelCode SPRITE FONT"
local msgTtf = "THIS FONT has been GENERATED from TTF"

-- NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)
local fontBm = LoadSpriteFont("resources/fonts/bmfont.fnt")       -- BMFont (AngelCode)
local fontTtf = LoadSpriteFont("resources/fonts/pixantiqua.ttf")  -- TTF font

local fontPosition = Vector2(0, 0)
fontPosition.x = screenWidth/2 - MeasureTextEx(fontBm, msgBm, fontBm.size, 0).x/2
fontPosition.y = screenHeight/2 - fontBm.size/2 - 80

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

        DrawTextEx(fontBm, msgBm, fontPosition, fontBm.size, 0, MAROON)
        DrawTextEx(fontTtf, msgTtf, Vector2(60.0, 240.0), fontTtf.size, 2, LIME)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadSpriteFont(fontBm)     -- AngelCode SpriteFont unloading
UnloadSpriteFont(fontTtf)    -- TTF SpriteFont unloading

CloseWindow()                -- Close window and OpenGL context
-------------------------------------------------------------------------------------------