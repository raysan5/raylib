-------------------------------------------------------------------------------------------
--
--  raylib [audio] example - Sound loading and playing
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

InitWindow(screenWidth, screenHeight, "raylib [audio] example - sound loading and playing")

InitAudioDevice()      -- Initialize audio device

local fxWav = LoadSound("resources/audio/weird.wav")         -- Load WAV audio file
local fxOgg = LoadSound("resources/audio/tanatana.ogg")      -- Load OGG audio file

SetTargetFPS(60)
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    if (IsKeyPressed(KEY.SPACE)) then PlaySound(fxWav) end      -- Play WAV sound
    if (IsKeyPressed(KEY.ENTER)) then PlaySound(fxOgg) end      -- Play OGG sound
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawText("Press SPACE to PLAY the WAV sound!", 200, 180, 20, LIGHTGRAY)

        DrawText("Press ENTER to PLAY the OGG sound!", 200, 220, 20, LIGHTGRAY)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadSound(fxWav)     -- Unload sound data
UnloadSound(fxOgg)     -- Unload sound data

CloseAudioDevice()     -- Close audio device

CloseWindow()          -- Close window and OpenGL context
-------------------------------------------------------------------------------------------