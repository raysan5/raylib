-------------------------------------------------------------------------------------------
--
--  raylib [audio] example - Music playing (streaming)
--
--  NOTE: This example requires OpenAL Soft library installed
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

InitWindow(screenWidth, screenHeight, "raylib [audio] example - music playing (streaming)")

InitAudioDevice()              -- Initialize audio device

local music = LoadMusicStream("resources/audio/guitar_noodling.ogg")

PlayMusicStream(music)

local framesCounter = 0
local timePlayed = 0.0

SetTargetFPS(60)                -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    framesCounter = framesCounter + 1

    timePlayed = GetMusicTimePlayed(music)/GetMusicTimeLength(music)*100*4 -- We scale by 4 to fit 400 pixels

    UpdateMusicStream(music)        -- Update music buffer with new stream data
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawText("MUSIC SHOULD BE PLAYING!", 255, 200, 20, LIGHTGRAY)

        DrawRectangle(200, 250, 400, 12, LIGHTGRAY)
        DrawRectangle(200, 250, timePlayed//1, 12, MAROON)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadMusicStream(music)   -- Unload music stream buffers from RAM

CloseAudioDevice()         -- Close audio device (music streaming is automatically stopped)

CloseWindow()              -- Close window and OpenGL context
-------------------------------------------------------------------------------------------