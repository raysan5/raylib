-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Picking in 3d mode
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

InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d picking")

-- Define the camera to look into our 3d world
local camera = {}
camera.position = Vector3(0.0, 10.0, 10.0)  -- Camera position
camera.target = Vector3(0.0, 0.0, 0.0)      -- Camera looking at point
camera.up = Vector3(0.0, 1.0, 0.0)          -- Camera up vector (rotation towards target)
camera.fovy = 45.0                          -- Camera field-of-view Y

local cubePosition = Vector3(0.0, 1.0, 0.0)
local cubeSize = Vector3(2.0, 2.0, 2.0)

local ray = Ray(Vector3(0, 0, 0), Vector3(0, 0, 0)) -- Picking line ray

local collision = false

SetCameraMode(camera, CameraMode.FREE)      -- Set a free camera mode

SetTargetFPS(60)                            -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    camera = UpdateCamera(camera)           -- Update camera
    
    if (IsMouseButtonPressed(MOUSE.LEFT_BUTTON)) then
        -- NOTE: This function is NOT WORKING properly!
        ray = GetMouseRay(GetMousePosition(), camera)
        
        -- Check collision between ray and box
        collision = CheckCollisionRayBox(ray,
                        BoundingBox(Vector3(cubePosition.x - cubeSize.x/2, cubePosition.y - cubeSize.y/2, cubePosition.z - cubeSize.z/2),
                                    Vector3(cubePosition.x + cubeSize.x/2, cubePosition.y + cubeSize.y/2, cubePosition.z + cubeSize.z/2)))

        --print("collision check:", collision)
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        Begin3dMode(camera)

            if (collision) then
                DrawCube(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, RED)
                DrawCubeWires(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, MAROON)

                DrawCubeWires(cubePosition, cubeSize.x + 0.2, cubeSize.y + 0.2, cubeSize.z + 0.2, GREEN)
            else
                DrawCube(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, GRAY)
                DrawCubeWires(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, DARKGRAY)
            end
            
            DrawRay(ray, MAROON)
            
            DrawGrid(10, 1.0)

        End3dMode()
        
        DrawText("Try selecting the box with mouse!", 240, 10, 20, DARKGRAY)
        
        if (collision) then 
            DrawText("BOX SELECTED", (screenWidth - MeasureText("BOX SELECTED", 30))/2, screenHeight*0.1, 30, GREEN)
        end

        DrawFPS(10, 10)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
-------------------------------------------------------------------------------------------