/*******************************************************************************************
*
*   raylib [shaders] example - game of life
*
*   Example complexity rating: [★★★☆] 3/4
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Jordi Santonja (@JordSant) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Jordi Santonja (@JordSant)
*
********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                 // Required for GUI controls

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Interaction mode
typedef enum {
    MODE_RUN = 0,
    MODE_PAUSE,
    MODE_DRAW,
} InteractionMode;

// Struct to store example preset patterns
typedef struct {
    char *name;
    Vector2 position;
} PresetPattern;

//----------------------------------------------------------------------------------
// Functions declaration
//----------------------------------------------------------------------------------
void FreeImageToDraw(Image **imageToDraw);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - game of life");

    const int menuWidth = 100;
    const int windowWidth = screenWidth - menuWidth;
    const int windowHeight = screenHeight;

    const int worldWidth = 2048;
    const int worldHeight = 2048;

    const int randomTiles = 8;      // Random preset: divide the world to compute random points in each tile

    const Rectangle worldRectSource = { 0, 0, (float)worldWidth, (float)-worldHeight };
    const Rectangle worldRectDest = { 0, 0, (float)worldWidth, (float)worldHeight };
    const Rectangle textureOnScreen = { 0, 0, (float)windowWidth, (float)windowHeight };

    const PresetPattern presetPatterns[] = {
        { "Glider", { 0.5f, 0.5f } }, { "R-pentomino", { 0.5f, 0.5f } }, { "Acorn", { 0.5f,0.5f } },
        { "Spaceships", { 0.1f, 0.5f } }, { "Still lifes", { 0.5f, 0.5f } }, { "Oscillators", { 0.5f, 0.5f } },
        { "Puffer train", { 0.1f, 0.5f } }, { "Glider Gun", { 0.2f, 0.2f } }, { "Breeder", { 0.1f, 0.5f } },
        { "Random", { 0.5f, 0.5f } }
    };
    
    const int numberOfPresets = sizeof(presetPatterns)/sizeof(presetPatterns[0]);

    int zoom = 1;
    float offsetX = (worldWidth - windowWidth)/2.0f;    // Centered on window
    float offsetY = (worldHeight - windowHeight)/2.0f;  // Centered on window
    int framesPerStep = 1;
    int frame = 0;

    int preset = -1;            // No button pressed for preset 
    int mode = MODE_RUN;        // Starting mode: running
    bool buttonZoomIn = false;  // Button states: false not pressed
    bool buttonZomOut = false;
    bool buttonFaster = false;
    bool buttonSlower = false;

    // Load shader
    Shader shdrGameOfLife = LoadShader(0, TextFormat("resources/shaders/glsl%i/game_of_life.fs", GLSL_VERSION));

    // Set shader uniform size of the world
    int resolutionLoc = GetShaderLocation(shdrGameOfLife, "resolution");
    const float resolution[2] = { (float)worldWidth, (float)worldHeight };
    SetShaderValue(shdrGameOfLife, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);

    // Define two textures: the current world and the previous world
    RenderTexture2D world1 = LoadRenderTexture(worldWidth, worldHeight);
    RenderTexture2D world2 = LoadRenderTexture(worldWidth, worldHeight);
    BeginTextureMode(world2);
        ClearBackground(RAYWHITE);
    EndTextureMode();

    Image startPattern = LoadImage("resources/game_of_life/r_pentomino.png");
    UpdateTextureRec(world2.texture, (Rectangle){ worldWidth/2.0f, worldHeight/2.0f, (float)(startPattern.width), (float)(startPattern.height) }, startPattern.data);
    UnloadImage(startPattern);

    // Pointers to the two textures, to be swapped
    RenderTexture2D *currentWorld = &world2;
    RenderTexture2D *previousWorld = &world1;

    // Image to be used in DRAW mode, to be changed with mouse input
    Image *imageToDraw = NULL;

    SetTargetFPS(60);               // Set at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        frame++;

        // Change zoom: both by buttons or by mouse wheel
        float mouseWheelMove = GetMouseWheelMove();
        if (buttonZoomIn || (buttonZomOut && (zoom > 1)) || (mouseWheelMove != 0.0f))
        {
            FreeImageToDraw(&imageToDraw);  // Zoom change: free the image to draw to be recreated again

            const float centerX = offsetX + (windowWidth/2.0f)/zoom;
            const float centerY = offsetY + (windowHeight/2.0f)/zoom;
            if (buttonZoomIn || (mouseWheelMove > 0.0f)) zoom *= 2;
            if ((buttonZomOut || (mouseWheelMove < 0.0f)) && (zoom > 1)) zoom /= 2;
            offsetX = centerX - (windowWidth/2.0f)/zoom;
            offsetY = centerY - (windowHeight/2.0f)/zoom;
        }

        // Change speed: number of frames per step
        if (buttonFaster && framesPerStep > 1)    framesPerStep--;
        if (buttonSlower)                         framesPerStep++;

        // Mouse management
        if ((mode == MODE_RUN) || (mode == MODE_PAUSE))
        {
            FreeImageToDraw(&imageToDraw);  // Free the image to draw: no longer needed in these modes

            // Pan with mouse left button
            static Vector2 previousMousePosition = { 0.0f, 0.0f };
            const Vector2 mousePosition = GetMousePosition();
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && (mousePosition.x < windowWidth))
            {
                offsetX -= (mousePosition.x - previousMousePosition.x)/zoom;
                offsetY -= (mousePosition.y - previousMousePosition.y)/zoom;
            }
            previousMousePosition = mousePosition;
        }
        else // MODE_DRAW
        {
            const float offsetDecimalX = offsetX - floorf(offsetX);
            const float offsetDecimalY = offsetY - floorf(offsetY);
            int sizeInWorldX = (int)(ceilf((float)(windowWidth + offsetDecimalX*zoom)/zoom));
            int sizeInWorldY = (int)(ceilf((float)(windowHeight + offsetDecimalY*zoom)/zoom));
            if (offsetX + sizeInWorldX >= worldWidth) sizeInWorldX = worldWidth - (int)floorf(offsetX);
            if (offsetY + sizeInWorldY >= worldHeight) sizeInWorldY = worldHeight - (int)floorf(offsetY);

            // Create image to draw if not created yet
            if (imageToDraw == NULL)
            {
                RenderTexture2D worldOnScreen = LoadRenderTexture(sizeInWorldX, sizeInWorldY);
                BeginTextureMode(worldOnScreen);
                    DrawTexturePro(currentWorld->texture, (Rectangle) { floorf(offsetX), floorf(offsetY), (float)(sizeInWorldX), -(float)(sizeInWorldY) },
                            (Rectangle) { 0, 0, (float)(sizeInWorldX), (float)(sizeInWorldY) }, (Vector2) { 0, 0 }, 0.0f, WHITE);
                EndTextureMode();
                imageToDraw = (Image*)RL_MALLOC(sizeof(Image));
                *imageToDraw = LoadImageFromTexture(worldOnScreen.texture);
                
                UnloadRenderTexture(worldOnScreen);
            }

            const Vector2 mousePosition = GetMousePosition();
            static int firstColor = -1;
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && (mousePosition.x < windowWidth))
            {
                int mouseX = (int)(mousePosition.x + offsetDecimalX*zoom)/zoom;
                int mouseY = (int)(mousePosition.y + offsetDecimalY*zoom)/zoom;
                if (mouseX >= sizeInWorldX) mouseX = sizeInWorldX - 1;
                if (mouseY >= sizeInWorldY) mouseY = sizeInWorldY - 1;
                if (firstColor == -1) firstColor = (GetImageColor(*imageToDraw, mouseX, mouseY).r < 5)? 0 : 1;
                const int prevColor = (GetImageColor(*imageToDraw, mouseX, mouseY).r < 5)? 0 : 1;
                
                ImageDrawPixel(imageToDraw, mouseX, mouseY, (firstColor) ? BLACK : RAYWHITE);
                
                if (prevColor != firstColor) UpdateTextureRec(currentWorld->texture, (Rectangle){ floorf(offsetX), floorf(offsetY), (float)(sizeInWorldX), (float)(sizeInWorldY) }, imageToDraw->data);
            }
            else firstColor = -1;
        }

        // Load selected preset
        if (preset >= 0)
        {
            Image pattern;
            if (preset < numberOfPresets - 1)   // Preset with pattern image lo load
            {
                switch (preset)
                {
                    case 0: pattern = LoadImage("resources/game_of_life/glider.png"); break;
                    case 1: pattern = LoadImage("resources/game_of_life/r_pentomino.png"); break;
                    case 2: pattern = LoadImage("resources/game_of_life/acorn.png"); break;
                    case 3: pattern = LoadImage("resources/game_of_life/spaceships.png"); break;
                    case 4: pattern = LoadImage("resources/game_of_life/still_lifes.png"); break;
                    case 5: pattern = LoadImage("resources/game_of_life/oscillators.png"); break;
                    case 6: pattern = LoadImage("resources/game_of_life/puffer_train.png"); break;
                    case 7: pattern = LoadImage("resources/game_of_life/glider_gun.png"); break;
                    case 8: pattern = LoadImage("resources/game_of_life/breeder.png"); break;
                }
                BeginTextureMode(*currentWorld);
                    ClearBackground(RAYWHITE);
                EndTextureMode();
                
                UpdateTextureRec(currentWorld->texture, (Rectangle){ worldWidth*presetPatterns[preset].position.x - pattern.width/2.0f,
                                                                     worldHeight*presetPatterns[preset].position.y - pattern.height/2.0f,
                                                                     (float)(pattern.width), (float)(pattern.height) }, pattern.data);
            }
            else    // Last preset: Random values
            {
                pattern = GenImageColor(worldWidth/randomTiles, worldHeight/randomTiles, RAYWHITE);
                for (int i = 0; i < randomTiles; i++)
                {
                    for (int j = 0; j < randomTiles; j++)
                    {
                        ImageClearBackground(&pattern, RAYWHITE);
                        for (int x = 0; x < pattern.width; x++)
                        {
                            for (int y = 0; y < pattern.height; y++)
                            {
                                if (GetRandomValue(0, 100) < 15) ImageDrawPixel(&pattern, x, y, BLACK);
                            }
                        }
                        UpdateTextureRec(currentWorld->texture,
                                         (Rectangle){ (float)(pattern.width*i), (float)(pattern.height*j),
                                                      (float)(pattern.width), (float)(pattern.height) }, pattern.data);
                    }
                }
            }

            UnloadImage(pattern);
            
            mode = MODE_PAUSE;
            offsetX = worldWidth*presetPatterns[preset].position.x - windowWidth/zoom/2.0f;
            offsetY = worldHeight*presetPatterns[preset].position.y - windowHeight/zoom/2.0f;
        }

        // Check window draw inside world limits
        if (offsetX < 0) offsetX = 0;
        if (offsetY < 0) offsetY = 0;
        if (offsetX > worldWidth - (float)(windowWidth)/zoom) offsetX = worldWidth - (float)(windowWidth)/zoom;
        if (offsetY > worldHeight - (float)(windowHeight)/zoom) offsetY = worldHeight - (float)(windowHeight)/zoom;

        // Rectangles for drawing texture portion to screen
        const Rectangle textureSourceToScreen = { offsetX, offsetY, (float)windowWidth/zoom, (float)windowHeight/zoom };
        //----------------------------------------------------------------------------------

        // Draw to texture
        //----------------------------------------------------------------------------------
        if ((mode == MODE_RUN) && ((frame%framesPerStep) == 0))
        {
            // Swap worlds
            RenderTexture2D *tempWorld = currentWorld;
            currentWorld = previousWorld;
            previousWorld = tempWorld;

            // Draw to texture
            BeginTextureMode(*currentWorld);
                BeginShaderMode(shdrGameOfLife);
                    DrawTexturePro(previousWorld->texture, worldRectSource, worldRectDest, (Vector2){ 0, 0 }, 0.0f, RAYWHITE);
                EndShaderMode();
            EndTextureMode();
        }
        //----------------------------------------------------------------------------------

        // Draw to screen
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            DrawTexturePro(currentWorld->texture, textureSourceToScreen, textureOnScreen, (Vector2){ 0, 0 }, 0.0f, WHITE);

            DrawLine(windowWidth, 0, windowWidth, screenHeight, (Color){ 218, 218, 218, 255 });
            DrawRectangle(windowWidth, 0, screenWidth - windowWidth, screenHeight, (Color){ 232, 232, 232, 255 });

            DrawText("Conway's", 704, 4, 20, DARKBLUE);
            DrawText(" game of", 704, 19, 20, DARKBLUE);
            DrawText("  life", 708, 34, 20, DARKBLUE);
            DrawText("in raylib", 757, 42, 6, BLACK);

            DrawText("Presets", 710, 58, 8, GRAY);
            preset = -1;
            for (int i = 0; i < numberOfPresets; i++)
                if (GuiButton((Rectangle){ 710.0f, 70.0f + 18*i, 80.0f, 16.0f }, presetPatterns[i].name)) preset = i;

            GuiToggleGroup((Rectangle){ 710, 258, 80, 16 }, "Run\nPause\nDraw", &mode);

            DrawText(TextFormat("Zoom: %ix", zoom), 710, 316, 8, GRAY);
            buttonZoomIn = GuiButton((Rectangle){ 710, 328, 80, 16 }, "Zoom in");
            buttonZomOut = GuiButton((Rectangle){ 710, 346, 80, 16 }, "Zoom out");

            DrawText(TextFormat("Speed: %i frame%s", framesPerStep, (framesPerStep > 1)? "s" : ""), 710, 370, 8, GRAY);
            buttonFaster = GuiButton((Rectangle){ 710, 382, 80, 16 }, "Faster");
            buttonSlower = GuiButton((Rectangle){ 710, 400, 80, 16 }, "Slower");

            DrawFPS(712, 426);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shdrGameOfLife);
    UnloadRenderTexture(world1);
    UnloadRenderTexture(world2);

    FreeImageToDraw(&imageToDraw);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Functions definition
//----------------------------------------------------------------------------------
void FreeImageToDraw(Image **imageToDraw)
{
    if (*imageToDraw != NULL)
    {
        UnloadImage(**imageToDraw);
        RL_FREE(*imageToDraw);
        *imageToDraw = NULL;
    }
}
