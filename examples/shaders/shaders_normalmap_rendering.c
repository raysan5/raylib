/*******************************************************************************************
*
*   raylib [shaders] example - normalmap rendering
*
*   Example complexity rating: [★★★★] 4/4
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*        OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Jeremy Montgomery (@Sir_Irk) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Jeremy Montgomery (@Sir_Irk) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - normalmap rendering");

    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, -4.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Load basic normal map lighting shader
    Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/normalmap.vs", GLSL_VERSION), 
                               TextFormat("resources/shaders/glsl%i/normalmap.fs", GLSL_VERSION));

    // Get some required shader locations
    shader.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(shader, "normalMap");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    // NOTE: "matModel" location name is automatically assigned on shader loading,
    // no need to get the location again if using that uniform name
    // shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");

    // This example uses just 1 point light
    Vector3 lightPosition = { 0.0f, 1.0f, 0.0f };
    int lightPosLoc = GetShaderLocation(shader, "lightPos");

    // Load a plane model that has proper normals and tangents
    Model plane = LoadModel("resources/models/plane.glb");

    // Set the plane model's shader and texture maps
    plane.materials[0].shader = shader;
    plane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("resources/tiles_diffuse.png");
    plane.materials[0].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture("resources/tiles_normal.png");

    // Generate Mipmaps and use TRILINEAR filtering to help with texture aliasing
    GenTextureMipmaps(&plane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
    GenTextureMipmaps(&plane.materials[0].maps[MATERIAL_MAP_NORMAL].texture);

    SetTextureFilter(plane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture, TEXTURE_FILTER_TRILINEAR);
    SetTextureFilter(plane.materials[0].maps[MATERIAL_MAP_NORMAL].texture, TEXTURE_FILTER_TRILINEAR);

    // Specular exponent AKA shininess of the material
    float specularExponent = 8.0f;
    int specularExponentLoc = GetShaderLocation(shader, "specularExponent");

    // Allow toggling the normal map on and off for comparison purposes
    int useNormalMap = 1;
    int useNormalMapLoc = GetShaderLocation(shader, "useNormalMap");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Move the light around on the X and Z axis using WASD keys
        Vector3 direction = { 0 };
        if (IsKeyDown(KEY_W)) direction = Vector3Add(direction, (Vector3){ 0.0f, 0.0f, 1.0f });
        if (IsKeyDown(KEY_S)) direction = Vector3Add(direction, (Vector3){ 0.0f, 0.0f, -1.0f });
        if (IsKeyDown(KEY_D)) direction = Vector3Add(direction, (Vector3){ -1.0f, 0.0f, 0.0f });
        if (IsKeyDown(KEY_A)) direction = Vector3Add(direction, (Vector3){ 1.0f, 0.0f, 0.0f });

        direction = Vector3Normalize(direction);
        lightPosition = Vector3Add(lightPosition, Vector3Scale(direction, GetFrameTime()*3.0f));

        // Increase/Decrease the specular exponent(shininess)
        if (IsKeyDown(KEY_UP)) specularExponent = Clamp(specularExponent + 40.0f*GetFrameTime(), 2.0f, 128.0f);
        if (IsKeyDown(KEY_DOWN)) specularExponent = Clamp(specularExponent - 40.0f*GetFrameTime(), 2.0f, 128.0f);

        // Toggle normal map on and off
        if (IsKeyPressed(KEY_N)) useNormalMap = !useNormalMap;

        // Spin plane model at a constant rate
        plane.transform = MatrixRotateY(GetTime()*0.5f);

        // Update shader values
        float lightPos[3] = {lightPosition.x, lightPosition.y, lightPosition.z};
        SetShaderValue(shader, lightPosLoc, lightPos, SHADER_UNIFORM_VEC3);

        float camPos[3] = {camera.position.x, camera.position.y, camera.position.z};
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], camPos, SHADER_UNIFORM_VEC3);

        SetShaderValue(shader, specularExponentLoc, &specularExponent, SHADER_UNIFORM_FLOAT);

        SetShaderValue(shader, useNormalMapLoc, &useNormalMap, SHADER_UNIFORM_INT);
        //--------------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                BeginShaderMode(shader);

                    DrawModel(plane, Vector3Zero(), 2.0f, WHITE);

                EndShaderMode();

                // Draw sphere to show light position
                DrawSphereWires(lightPosition, 0.2f, 8, 8, ORANGE);

            EndMode3D();

            Color textColor = (useNormalMap) ? DARKGREEN : RED;
            const char *toggleStr = (useNormalMap) ? "On" : "Off";
            DrawText(TextFormat("Use key [N] to toggle normal map: %s", toggleStr), 10, 10, 10, textColor);

            int yOffset = 24;
            DrawText("Use keys [W][A][S][D] to move the light", 10, 10 + yOffset*1, 10, BLACK);
            DrawText("Use keys [Up][Down] to change specular exponent", 10, 10 + yOffset*2, 10, BLACK);
            DrawText(TextFormat("Specular Exponent: %.2f", specularExponent), 10, 10 + yOffset*3, 10, BLUE);

            DrawFPS(screenWidth - 90, 10);

        EndDrawing();
        //--------------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);
    UnloadModel(plane);

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
