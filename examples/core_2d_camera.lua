-------------------------------------------------------------------------------------------
--
--  raylib [core] example - 2d camera
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

MAX_BUILDINGS = 100

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera")

local player = Rectangle(400, 280, 40, 40)
local buildings = {}
local buildColors = {}

local spacing = 0;

for i = 1, MAX_BUILDINGS do
    buildings[i] = Rectangle(0, 0, 0, 0)
    buildings[i].width = GetRandomValue(50, 200)
    buildings[i].height = GetRandomValue(100, 800)
    buildings[i].y = screenHeight - 130 - buildings[i].height
    buildings[i].x = -6000 + spacing

    spacing = spacing + buildings[i].width
    
    buildColors[i] = Color(GetRandomValue(200, 240), GetRandomValue(200, 240), GetRandomValue(200, 250), 255)
end

local camera = Camera2D(Vector2(0, 0), Vector2(0, 0), 0.0, 1.0)

camera.target = Vector2(player.x + 20, player.y + 20)
camera.offset = Vector2(0, 0)
camera.rotation = 0.0
camera.zoom = 1.0

SetTargetFPS(60)
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do                -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    if (IsKeyDown(KEY.RIGHT)) then
        player.x = player.x + 2                 -- Player movement
        camera.offset.x = camera.offset.x - 2   -- Camera displacement with player movement
    elseif (IsKeyDown(KEY.LEFT)) then
        player.x = player.x - 2                 -- Player movement
        camera.offset.x = camera.offset.x + 2   -- Camera displacement with player movement
    end
    
    -- Camera target follows player
    camera.target = Vector2(player.x + 20, player.y + 20)
    
    -- Camera rotation controls
    if (IsKeyDown(KEY.A)) then camera.rotation = camera.rotation - 1
    elseif (IsKeyDown(KEY.S)) then camera.rotation = camera.rotation + 1
    end
    
    -- Limit camera rotation to 80 degrees (-40 to 40)
    if (camera.rotation > 40) then camera.rotation = 40
    elseif (camera.rotation < -40) then camera.rotation = -40
    end

    -- Camera zoom controls
    camera.zoom = camera.zoom + (GetMouseWheelMove()*0.05)
    
    if (camera.zoom > 3.0) then camera.zoom = 3.0
    elseif (camera.zoom < 0.1) then camera.zoom = 0.1
    end
    
    -- Camera reset (zoom and rotation)
    if (IsKeyPressed(KEY.R)) then
        camera.zoom = 1.0
        camera.rotation = 0.0
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        Begin2dMode(camera)

            DrawRectangle(-6000, 320, 13000, 8000, DARKGRAY)
            
            for i = 1, MAX_BUILDINGS, 1 do DrawRectangleRec(buildings[i], buildColors[i]) end
            
            DrawRectangleRec(player, RED)

            DrawRectangle(camera.target.x, -500, 1, screenHeight*4, GREEN)
            DrawRectangle(-500, camera.target.y, screenWidth*4, 1, GREEN)
            
        End2dMode()

        DrawText("SCREEN AREA", 640, 10, 20, RED)

        DrawRectangle(0, 0, screenWidth, 5, RED)
        DrawRectangle(0, 5, 5, screenHeight - 10, RED)
        DrawRectangle(screenWidth - 5, 5, 5, screenHeight - 10, RED)
        DrawRectangle(0, screenHeight - 5, screenWidth, 5, RED)

        DrawRectangle( 10, 10, 250, 113, Fade(SKYBLUE, 0.5))
        DrawRectangleLines( 10, 10, 250, 113, BLUE)

        DrawText("Free 2d camera controls:", 20, 20, 10, BLACK)
        DrawText("- Right/Left to move Offset", 40, 40, 10, DARKGRAY)
        DrawText("- Mouse Wheel to Zoom in-out", 40, 60, 10, DARKGRAY)
        DrawText("- A / S to Rotate", 40, 80, 10, DARKGRAY)
        DrawText("- R to reset Zoom and Rotation", 40, 100, 10, DARKGRAY)

    EndDrawing();
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()           -- Close window and OpenGL context
-------------------------------------------------------------------------------------------
