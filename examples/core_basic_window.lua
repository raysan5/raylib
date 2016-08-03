-- Initialization
----------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [rlua] example - basic window")

InitAudioDevice()

--local pause = false
--local texture = LoadTexture("resources/texture.png")

SetTargetFPS(60)       -- Set target frames-per-second
----------------------------------------------------------------------------------------

while not WindowShouldClose() do
    -- Update
    ------------------------------------------------------------------------------------
    --if (IsKeyPressed(KEY.SPACE)) then
    --  pause = not pause
    --end
    ------------------------------------------------------------------------------------

    -- Draw
    ------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY)

    EndDrawing()
    ------------------------------------------------------------------------------------
end

-- De-Initialization
------------------------------------------------------------------------------------
CloseAudioDevice()      -- Close audio device

CloseWindow()           -- Close window and OpenGL context
------------------------------------------------------------------------------------