-------------------------------------------------------------------------------------------
--
--  raylib [models] example - Detect basic 3d collisions (box vs sphere vs box)
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

InitWindow(screenWidth, screenHeight, "raylib [models] example - box collisions")

-- Define the camera to look into our 3d world
local camera = Camera(Vector3(0.0, 10.0, 10.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), 45.0)

local playerPosition = Vector3(0.0, 1.0, 2.0)
local playerSize = Vector3(1.0, 2.0, 1.0)
local playerColor = GREEN

local enemyBoxPos = Vector3(-4.0, 1.0, 0.0)
local enemyBoxSize = Vector3(2.0, 2.0, 2.0)

local enemySpherePos = Vector3(4.0, 0.0, 0.0)
local enemySphereSize = 1.5

local collision = false

SetTargetFPS(60)   -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    
    -- Move player
    if (IsKeyDown(KEY.RIGHT)) then playerPosition.x = playerPosition.x + 0.2
    elseif (IsKeyDown(KEY.LEFT)) then playerPosition.x = playerPosition.x - 0.2
    elseif (IsKeyDown(KEY.DOWN)) then playerPosition.z = playerPosition.z + 0.2
    elseif (IsKeyDown(KEY.UP)) then playerPosition.z = playerPosition.z - 0.2 end
    
    collision = false
    
    -- Check collisions player vs enemy-box
    if (CheckCollisionBoxes(
        BoundingBox(Vector3(playerPosition.x - playerSize.x/2, 
                            playerPosition.y - playerSize.y/2, 
                            playerPosition.z - playerSize.z/2), 
                    Vector3(playerPosition.x + playerSize.x/2,
                            playerPosition.y + playerSize.y/2, 
                            playerPosition.z + playerSize.z/2)),
        BoundingBox(Vector3(enemyBoxPos.x - enemyBoxSize.x/2, 
                            enemyBoxPos.y - enemyBoxSize.y/2, 
                            enemyBoxPos.z - enemyBoxSize.z/2), 
                    Vector3(enemyBoxPos.x + enemyBoxSize.x/2,
                            enemyBoxPos.y + enemyBoxSize.y/2, 
                            enemyBoxPos.z + enemyBoxSize.z/2)))) then collision = true 
    end
    
    -- Check collisions player vs enemy-sphere
    if (CheckCollisionBoxSphere(
        BoundingBox(Vector3(playerPosition.x - playerSize.x/2, 
                            playerPosition.y - playerSize.y/2, 
                            playerPosition.z - playerSize.z/2), 
                    Vector3(playerPosition.x + playerSize.x/2,
                            playerPosition.y + playerSize.y/2, 
                            playerPosition.z + playerSize.z/2)), 
        enemySpherePos, enemySphereSize)) then collision = true
    end
    
    if (collision) then playerColor = RED
    else playerColor = GREEN end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        Begin3dMode(camera)

            -- Draw enemy-box
            DrawCube(enemyBoxPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, GRAY)
            DrawCubeWires(enemyBoxPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, DARKGRAY)
            
            -- Draw enemy-sphere
            DrawSphere(enemySpherePos, enemySphereSize, GRAY)
            DrawSphereWires(enemySpherePos, enemySphereSize, 16, 16, DARKGRAY)
            
            -- Draw player
            DrawCubeV(playerPosition, playerSize, playerColor)

            DrawGrid(10, 1.0)        -- Draw a grid

        End3dMode()
        
        DrawText("Move player with cursors to collide", 220, 40, 20, GRAY)

        DrawFPS(10, 10)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
-------------------------------------------------------------------------------------------