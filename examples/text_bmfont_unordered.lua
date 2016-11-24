-------------------------------------------------------------------------------------------
--
--  raylib [text] example - BMFont unordered chars loading and drawing
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

InitWindow(screenWidth, screenHeight, "raylib [text] example - bmfont unordered loading and drawing")

-- NOTE: Using chars outside the [32..127] limits!
-- NOTE: If a character is not found in the font, it just renders a space
local msg = "ASCII extended characters:\n¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆ\nÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæ\nçèéêëìíîïðñòóôõö÷øùúûüýþÿ"

-- NOTE: Loaded font has an unordered list of characters (chars in the range 32..255)
local font = LoadSpriteFont("resources/fonts/pixantiqua.fnt")       -- BMFont (AngelCode)

SetTargetFPS(60)
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

        DrawText("Font name:       PixAntiqua", 40, 50, 20, GRAY)
        DrawText(string.format("Font base size:           %i", font.size), 40, 80, 20, GRAY)
        DrawText(string.format("Font chars number:     %i", font.numChars), 40, 110, 20, GRAY)
        
        DrawTextEx(font, msg, Vector2(40, 180), font.size, 0, MAROON)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadSpriteFont(font)      -- AngelCode SpriteFont unloading

CloseWindow()                -- Close window and OpenGL context
-------------------------------------------------------------------------------------------