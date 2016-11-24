-------------------------------------------------------------------------------------------
--
--  raylib [shaders] example - Standard lighting (materials and lights)
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

InitWindow(screenWidth, screenHeight, "raylib [shaders] example - model shader")

-- Define the camera to look into our 3d world
local camera = Camera(Vector3(4.0, 4.0, 4.0), Vector3(0.0, 1.5, 0.0), Vector3(0.0, 1.0, 0.0), 45.0)

local dwarf = LoadModel("resources/model/dwarf.obj")                     -- Load OBJ model
local position = Vector3(0.0, 0.0, 0.0)                                  -- Set model position

local material = LoadStandardMaterial()

material.texDiffuse = LoadTexture("resources/model/dwarf_diffuse.png")   -- Load model diffuse texture
material.texNormal = LoadTexture("resources/model/dwarf_normal.png")     -- Load model normal texture
material.texSpecular = LoadTexture("resources/model/dwarf_specular.png") -- Load model specular texture
material.colDiffuse = WHITE
material.colAmbient = Color(0, 0, 10, 255)
material.colSpecular = WHITE
material.glossiness = 50.0

dwarf.material = material      -- Apply material to model

local spotLight = CreateLight(LightType.SPOT, Vector3(3.0, 5.0, 2.0), Color(255, 255, 255, 255))
spotLight.target = Vector3(0.0, 0.0, 0.0)
spotLight.intensity = 2.0
spotLight.diffuse = Color(255, 100, 100, 255)
spotLight.coneAngle = 60.0

local dirLight = CreateLight(LightType.DIRECTIONAL, Vector3(0.0, -3.0, -3.0), Color(255, 255, 255, 255))
dirLight.target = Vector3(1.0, -2.0, -2.0)
dirLight.intensity = 2.0
dirLight.diffuse = Color(100, 255, 100, 255)

local pointLight = CreateLight(LightType.POINT, Vector3(0.0, 4.0, 5.0), Color(255, 255, 255, 255))
pointLight.intensity = 2.0
pointLight.diffuse = Color(100, 100, 255, 255)
pointLight.radius = 3.0

-- Setup orbital camera
SetCameraMode(camera, CameraMode.ORBITAL)   -- Set an orbital camera mode

SetTargetFPS(60)                            -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    camera = UpdateCamera(camera)           -- Update camera
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        Begin3dMode(camera)
            
            DrawModel(dwarf, position, 2.0, WHITE)   -- Draw 3d model with texture
            
            DrawLight(spotLight)       -- Draw spot light
            DrawLight(dirLight)        -- Draw directional light
            DrawLight(pointLight)      -- Draw point light

            DrawGrid(10, 1.0)          -- Draw a grid

        End3dMode()
        
        DrawText("(c) Dwarf 3D model by David Moreno", screenWidth - 200, screenHeight - 20, 10, GRAY)
        
        DrawFPS(10, 10)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadMaterial(material)   -- Unload material and assigned textures
UnloadModel(dwarf)         -- Unload model

-- Destroy all created lights
DestroyLight(pointLight)
DestroyLight(dirLight)
DestroyLight(spotLight)

CloseWindow()              -- Close window and OpenGL context
-------------------------------------------------------------------------------------------