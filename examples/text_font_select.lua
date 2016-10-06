-------------------------------------------------------------------------------------------
--
--  raylib [text] example - Font selector
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

InitWindow(screenWidth, screenHeight, "raylib [text] example - font selector")

-- NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
local fonts = {}        -- SpriteFont array

fonts[1] = LoadSpriteFont("resources/fonts/alagard.rbmf")       -- SpriteFont loading
fonts[2] = LoadSpriteFont("resources/fonts/pixelplay.rbmf")     -- SpriteFont loading
fonts[3] = LoadSpriteFont("resources/fonts/mecha.rbmf")         -- SpriteFont loading
fonts[4] = LoadSpriteFont("resources/fonts/setback.rbmf")       -- SpriteFont loading
fonts[5] = LoadSpriteFont("resources/fonts/romulus.rbmf")       -- SpriteFont loading
fonts[6] = LoadSpriteFont("resources/fonts/pixantiqua.rbmf")    -- SpriteFont loading
fonts[7] = LoadSpriteFont("resources/fonts/alpha_beta.rbmf")    -- SpriteFont loading
fonts[8] = LoadSpriteFont("resources/fonts/jupiter_crash.rbmf") -- SpriteFont loading

local currentFont = 1        -- Selected font

local colors = { MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, LIME, GOLD, RED }

local fontNames = { "[1] Alagard", "[2] PixelPlay", "[3] MECHA", "[4] Setback",
                    "[5] Romulus", "[6] PixAntiqua", "[7] Alpha Beta", "[8] Jupiter Crash" }

local text = "THIS is THE FONT you SELECTED!"     -- Main text

local textSize = MeasureTextEx(fonts[currentFont], text, fonts[currentFont].size*3, 1)

local mousePoint

local btnNextOutColor = DARKBLUE       -- Button color (outside line)
local btnNextInColor = SKYBLUE         -- Button color (inside)

local framesCounter = 0      -- Useful to count frames button is 'active' = clicked

local positionY = 180        -- Text selector and button Y position

local btnNextRec = Rectangle(673, positionY, 109, 44)    -- Button rectangle (useful for collision)

SetTargetFPS(60)           -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------

    -- Keyboard-based font selection (easy)
    if (IsKeyPressed(KEY.RIGHT)) then
        if (currentFont < 8) then currentFont = currentFont + 1 end
    end

    if (IsKeyPressed(KEY.LEFT)) then
        if (currentFont > 1) then currentFont = currentFont - 1 end
    end
    
    if (IsKeyPressed(KEY.ZERO)) then currentFont = 0
    elseif (IsKeyPressed(KEY.ONE)) then currentFont = 1
    elseif (IsKeyPressed(KEY.TWO)) then currentFont = 2
    elseif (IsKeyPressed(KEY.THREE)) then currentFont = 3
    elseif (IsKeyPressed(KEY.FOUR)) then currentFont = 4
    elseif (IsKeyPressed(KEY.FIVE)) then currentFont = 5
    elseif (IsKeyPressed(KEY.SIX)) then currentFont = 6
    elseif (IsKeyPressed(KEY.SEVEN)) then currentFont = 7
    end

    -- Mouse-based font selection (NEXT button logic)
    mousePoint = GetMousePosition()

    if (CheckCollisionPointRec(mousePoint, btnNextRec)) then
        -- Mouse hover button logic
        if (framesCounter == 0) then
            btnNextOutColor = DARKPURPLE
            btnNextInColor = PURPLE
        end

        if (IsMouseButtonDown(MOUSE.LEFT_BUTTON)) then
            framesCounter = 20         -- Frames button is 'active'
            btnNextOutColor = MAROON
            btnNextInColor = RED
        end
    else
        -- Mouse not hover button
        btnNextOutColor = DARKBLUE
        btnNextInColor = SKYBLUE
    end
    
    if (framesCounter > 0) then framesCounter = framesCounter - 1 end

    if (framesCounter == 1) then     -- We change font on frame 1
        currentFont = currentFont + 1
        if (currentFont > 7) then currentFont = 0 end
    end

    -- Text measurement for better positioning on screen
    textSize = MeasureTextEx(fonts[currentFont], text, fonts[currentFont].size*3, 1)
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)
        
        DrawText("font selector - use arroys, button or numbers", 160, 80, 20, DARKGRAY)
        DrawLine(120, 120, 680, 120, DARKGRAY)

        DrawRectangle(18, positionY, 644, 44, DARKGRAY)
        DrawRectangle(20, positionY + 2, 640, 40, LIGHTGRAY)
        DrawText(fontNames[currentFont], 30, positionY + 13, 20, BLACK)
        DrawText("< >", 610, positionY + 8, 30, BLACK)

        DrawRectangleRec(btnNextRec, btnNextOutColor)
        DrawRectangle(675, positionY + 2, 105, 40, btnNextInColor)
        DrawText("NEXT", 700, positionY + 13, 20, btnNextOutColor)

        DrawTextEx(fonts[currentFont], text, Vector2(screenWidth/2 - textSize.x/2,
                   260 + (70 - textSize.y)/2), fonts[currentFont].size*3,
                   1, colors[currentFont])

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
for i = 1, 8 do UnloadSpriteFont(fonts[i]) end       -- SpriteFont(s) unloading

CloseWindow()                -- Close window and OpenGL context
-------------------------------------------------------------------------------------------