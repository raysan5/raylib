-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Windows drop files
--
--  This example only works on platforms that support drag & drop (Windows, Linux, OSX)
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

InitWindow(screenWidth, screenHeight, "raylib [core] example - drop files")

local count = 0
local droppedFiles = {}

SetTargetFPS(60)
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    if (IsFileDropped()) then 
        droppedFiles = GetDroppedFiles() 
        count = #droppedFiles
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        if (count == 0) then DrawText("Drop your files to this window!", 100, 40, 20, DARKGRAY)
        else
            DrawText("Dropped files:", 100, 40, 20, DARKGRAY)
            
            for i = 0, count-1 do
                if (i%2 == 0) then DrawRectangle(0, 85 + 40*i, screenWidth, 40, Fade(LIGHTGRAY, 0.5))
                else DrawRectangle(0, 85 + 40*i, screenWidth, 40, Fade(LIGHTGRAY, 0.3)) end
                
                DrawText(droppedFiles[i+1], 120, 100 + 40*i, 10, GRAY)
            end
            
            DrawText("Drop new files...", 100, 110 + 40*count, 20, DARKGRAY)
        end

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
ClearDroppedFiles()    -- Clear internal buffers

CloseWindow()          -- Close window and OpenGL context
-------------------------------------------------------------------------------------------