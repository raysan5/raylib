-------------------------------------------------------------------------------------------
--
--  raylib [shaders] example - Apply a postprocessing shader and connect a custom uniform variable
--
--  NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
--        OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
--
--  NOTE: Shaders used in this example are #version 330 (OpenGL 3.3), to test this example
--        on OpenGL ES 2.0 platforms (Android, Raspberry Pi, HTML5), use #version 100 shaders
--        raylib comes with shaders ready for both versions, check raylib/shaders install folder
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

SetConfigFlags(FLAG.MSAA_4X_HINT)      -- Enable Multi Sampling Anti Aliasing 4x (if available)

InitWindow(screenWidth, screenHeight, "raylib [shaders] example - custom uniform variable")

-- Define the camera to look into our 3d world
local camera = Camera(Vector3(3.0, 3.0, 3.0), Vector3(0.0, 1.5, 0.0), Vector3(0.0, 1.0, 0.0), 45.0)

local dwarf = LoadModel("resources/model/dwarf.obj")                   -- Load OBJ model
local texture = LoadTexture("resources/model/dwarf_diffuse.png")       -- Load model texture (diffuse map)
dwarf.material.texDiffuse = texture                                    -- Set dwarf model diffuse texture

local position = Vector3(0.0, 0.0, 0.0)                                -- Set model position

local shader = LoadShader("resources/shaders/glsl330/base.vs", 
                          "resources/shaders/glsl330/swirl.fs")        -- Load postpro shader

-- Get variable (uniform) location on the shader to connect with the program
-- NOTE: If uniform variable could not be found in the shader, function returns -1
local swirlCenterLoc = GetShaderLocation(shader, "center")

local swirlCenter = { screenWidth/2, screenHeight/2 }

-- Create a RenderTexture2D to be used for render to texture
local target = LoadRenderTexture(screenWidth, screenHeight)

-- Setup orbital camera
SetCameraMode(camera, CameraMode.ORBITAL)   -- Set an orbital camera mode

SetTargetFPS(60)                            -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    local mousePosition = GetMousePosition()

    swirlCenter[1] = mousePosition.x
    swirlCenter[2] = screenHeight - mousePosition.y

    -- Send new value to the shader to be used on drawing
    SetShaderValue(shader, swirlCenterLoc, swirlCenter)
    
    camera = UpdateCamera(camera)           -- Update camera
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)
        
        BeginTextureMode(target)            -- Enable drawing to texture

            Begin3dMode(camera)

                DrawModel(dwarf, position, 2.0, WHITE)   -- Draw 3d model with texture

                DrawGrid(10, 1.0)           -- Draw a grid

            End3dMode()
            
            DrawText("TEXT DRAWN IN RENDER TEXTURE", 200, 10, 30, RED)
        
        EndTextureMode()           -- End drawing to texture (now we have a texture available for next passes)
        
        BeginShaderMode(shader)
        
            -- NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
            DrawTextureRec(target.texture, Rectangle(0, 0, target.texture.width, -target.texture.height), Vector2(0, 0), WHITE)
        
        EndShaderMode()
        
        DrawText("(c) Dwarf 3D model by David Moreno", screenWidth - 200, screenHeight - 20, 10, GRAY)

        DrawFPS(10, 10)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadShader(shader)           -- Unload shader
UnloadTexture(texture)         -- Unload texture
UnloadModel(dwarf)             -- Unload model
UnloadRenderTexture(target)    -- Unload render texture

CloseWindow()                  -- Close window and OpenGL context
-------------------------------------------------------------------------------------------