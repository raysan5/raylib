/*******************************************************************************************
*
*   raylib [core] example - viewport scaling
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Agnis Aldiņš (@nezvers) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Agnis Aldiņš (@nezvers)
*
********************************************************************************************/

#include "raylib.h"

#define RESOLUTION_COUNT    4   // For iteration purposes and teaching example

typedef enum {
    // Only upscale, useful for pixel art
    KEEP_ASPECT_INTEGER,
    KEEP_HEIGHT_INTEGER,
    KEEP_WIDTH_INTEGER,
    // Can also downscale
    KEEP_ASPECT,
    KEEP_HEIGHT,
    KEEP_WIDTH,
    // For itteration purposes and as a teaching example
    VIEWPORT_TYPE_COUNT,
} ViewportType;

// For displaying on GUI
const char *ViewportTypeNames[VIEWPORT_TYPE_COUNT] = {
    "KEEP_ASPECT_INTEGER",
    "KEEP_HEIGHT_INTEGER",
    "KEEP_WIDTH_INTEGER",
    "KEEP_ASPECT",
    "KEEP_HEIGHT",
    "KEEP_WIDTH",
};

//--------------------------------------------------------------------------------------
// Module Functions Declaration
//--------------------------------------------------------------------------------------
static void KeepAspectCenteredInteger(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect);
static void KeepHeightCenteredInteger(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect);
static void KeepWidthCenteredInteger(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect);
static void KeepAspectCentered(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect);
static void KeepHeightCentered(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect);
static void KeepWidthCentered(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect);
static void ResizeRenderSize(ViewportType viewportType, int *screenWidth, int *screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect, RenderTexture2D *target);

// Example how to calculate position on RenderTexture
static Vector2 Screen2RenderTexturePosition(Vector2 point, Rectangle *textureRect, Rectangle *scaledRect);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - viewport scaling");

    // Preset resolutions that could be created by subdividing screen resolution
    Vector2 resolutionList[RESOLUTION_COUNT] = {
        (Vector2){ 64, 64 },
        (Vector2){ 256, 240 },
        (Vector2){ 320, 180 },
        // 4K doesn't work with integer scaling but included for example purposes with non-integer scaling
        (Vector2){ 3840, 2160 },
    };

    int resolutionIndex = 0;
    int gameWidth = 64;
    int gameHeight = 64;

    RenderTexture2D target = (RenderTexture2D){ 0 };
    Rectangle sourceRect = (Rectangle){ 0 };
    Rectangle destRect = (Rectangle){ 0 };

    ViewportType viewportType = KEEP_ASPECT_INTEGER;
    ResizeRenderSize(viewportType, &screenWidth, &screenHeight, gameWidth, gameHeight, &sourceRect, &destRect, &target);

    // Button rectangles
    Rectangle decreaseResolutionButton = (Rectangle){ 200, 30, 10, 10 };
    Rectangle increaseResolutionButton = (Rectangle){ 215, 30, 10, 10 };
    Rectangle decreaseTypeButton = (Rectangle){ 200, 45, 10, 10 };
    Rectangle increaseTypeButton = (Rectangle){ 215, 45, 10, 10 };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //----------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsWindowResized()) ResizeRenderSize(viewportType, &screenWidth, &screenHeight, gameWidth, gameHeight, &sourceRect, &destRect, &target);

        Vector2 mousePosition = GetMousePosition();
        bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        // Check buttons and rescale
        if (CheckCollisionPointRec(mousePosition, decreaseResolutionButton) && mousePressed)
        {
            resolutionIndex = (resolutionIndex + RESOLUTION_COUNT - 1)%RESOLUTION_COUNT;
            gameWidth = (int)resolutionList[resolutionIndex].x;
            gameHeight = (int)resolutionList[resolutionIndex].y;
            ResizeRenderSize(viewportType, &screenWidth, &screenHeight, gameWidth, gameHeight, &sourceRect, &destRect, &target);
        }

        if (CheckCollisionPointRec(mousePosition, increaseResolutionButton) && mousePressed)
        {
            resolutionIndex = (resolutionIndex + 1)%RESOLUTION_COUNT;
            gameWidth = (int)resolutionList[resolutionIndex].x;
            gameHeight = (int)resolutionList[resolutionIndex].y;
            ResizeRenderSize(viewportType, &screenWidth, &screenHeight, gameWidth, gameHeight, &sourceRect, &destRect, &target);
        }

        if (CheckCollisionPointRec(mousePosition, decreaseTypeButton) && mousePressed)
        {
            viewportType = (viewportType + VIEWPORT_TYPE_COUNT - 1)%VIEWPORT_TYPE_COUNT;
            ResizeRenderSize(viewportType, &screenWidth, &screenHeight, gameWidth, gameHeight, &sourceRect, &destRect, &target);
        }

        if (CheckCollisionPointRec(mousePosition, increaseTypeButton) && mousePressed)
        {
            viewportType = (viewportType + 1)%VIEWPORT_TYPE_COUNT;
            ResizeRenderSize(viewportType, &screenWidth, &screenHeight, gameWidth, gameHeight, &sourceRect, &destRect, &target);
        }

        Vector2 textureMousePosition = Screen2RenderTexturePosition(mousePosition, &sourceRect, &destRect);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        // Draw our scene to the render texture
        BeginTextureMode(target);
            ClearBackground(WHITE);
            DrawCircleV(textureMousePosition, 20.0f, LIME);
        EndTextureMode();

        // Draw render texture to main framebuffer
        BeginDrawing();
            ClearBackground(BLACK);

            // Draw our render texture with rotation applied
            DrawTexturePro(target.texture, sourceRect, destRect, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);

            // Draw Native resolution (GUI or anything)
            // Draw info box
            Rectangle infoRect = (Rectangle){5, 5, 330, 105};
            DrawRectangleRec(infoRect, Fade(LIGHTGRAY, 0.7f));
            DrawRectangleLinesEx(infoRect, 1, BLUE);

            DrawText(TextFormat("Window Resolution: %d x %d", screenWidth, screenHeight), 15, 15, 10, BLACK);
            DrawText(TextFormat("Game Resolution: %d x %d", gameWidth, gameHeight), 15, 30, 10, BLACK);

            DrawText(TextFormat("Type: %s", ViewportTypeNames[viewportType]), 15, 45, 10, BLACK);
            Vector2 scaleRatio = (Vector2){destRect.width/sourceRect.width, -destRect.height/sourceRect.height};
            if (scaleRatio.x < 0.001f || scaleRatio.y < 0.001f) DrawText(TextFormat("Scale ratio: INVALID"), 15, 60, 10, BLACK);
            else DrawText(TextFormat("Scale ratio: %.2f x %.2f", scaleRatio.x, scaleRatio.y), 15, 60, 10, BLACK);

            DrawText(TextFormat("Source size: %.2f x %.2f", sourceRect.width, -sourceRect.height), 15, 75, 10, BLACK);
            DrawText(TextFormat("Destination size: %.2f x %.2f", destRect.width, destRect.height), 15, 90, 10, BLACK);

            // Draw buttons
            DrawRectangleRec(decreaseTypeButton, SKYBLUE);
            DrawRectangleRec(increaseTypeButton, SKYBLUE);
            DrawRectangleRec(decreaseResolutionButton, SKYBLUE);
            DrawRectangleRec(increaseResolutionButton, SKYBLUE);
            DrawText("<", decreaseTypeButton.x + 3, decreaseTypeButton.y + 1, 10, BLACK);
            DrawText(">", increaseTypeButton.x + 3, increaseTypeButton.y + 1, 10, BLACK);
            DrawText("<", decreaseResolutionButton.x + 3, decreaseResolutionButton.y + 1, 10, BLACK);
            DrawText(">", increaseResolutionButton.x + 3, increaseResolutionButton.y + 1, 10, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //----------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------
static void KeepAspectCenteredInteger(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect)
{
    sourceRect->x = 0.0f;
    sourceRect->y = (float)gameHeight;
    sourceRect->width = (float)gameWidth;
    sourceRect->height = (float)-gameHeight;

    const int ratio_x = (screenWidth/gameWidth);
    const int ratio_y = (screenHeight/gameHeight);
    const float resizeRatio = (float)((ratio_x < ratio_y)? ratio_x : ratio_y);

    destRect->x = (float)(int)((screenWidth - (gameWidth*resizeRatio))*0.5f);
    destRect->y = (float)(int)((screenHeight - (gameHeight*resizeRatio))*0.5f);
    destRect->width = (float)(int)(gameWidth*resizeRatio);
    destRect->height = (float)(int)(gameHeight*resizeRatio);
}

static void KeepHeightCenteredInteger(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect)
{
    const float resizeRatio = (float)(screenHeight/gameHeight);
    sourceRect->x = 0.0f;
    sourceRect->y = 0.0f;
    sourceRect->width = (float)(int)(screenWidth/resizeRatio);
    sourceRect->height = (float)-gameHeight;

    destRect->x = (float)(int)((screenWidth - (sourceRect->width*resizeRatio))*0.5f);
    destRect->y = (float)(int)((screenHeight - (gameHeight*resizeRatio))*0.5f);
    destRect->width = (float)(int)(sourceRect->width*resizeRatio);
    destRect->height = (float)(int)(gameHeight*resizeRatio);
}

static void KeepWidthCenteredInteger(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect)
{
    const float resizeRatio = (float)(screenWidth/gameWidth);
    sourceRect->x = 0.0f;
    sourceRect->y = 0.0f;
    sourceRect->width = (float)gameWidth;
    sourceRect->height = (float)(int)(screenHeight/resizeRatio);

    destRect->x = (float)(int)((screenWidth - (gameWidth*resizeRatio))*0.5f);
    destRect->y = (float)(int)((screenHeight - (sourceRect->height*resizeRatio))*0.5f);
    destRect->width = (float)(int)(gameWidth*resizeRatio);
    destRect->height = (float)(int)(sourceRect->height*resizeRatio);

    sourceRect->height *= -1.0f;
}

static void KeepAspectCentered(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect)
{
    sourceRect->x = 0.0f;
    sourceRect->y = (float)gameHeight;
    sourceRect->width = (float)gameWidth;
    sourceRect->height = (float)-gameHeight;

    const float ratio_x = ((float)screenWidth/(float)gameWidth);
    const float ratio_y = ((float)screenHeight/(float)gameHeight);
    const float resizeRatio = (ratio_x < ratio_y ? ratio_x : ratio_y);

    destRect->x = (float)(int)((screenWidth - (gameWidth*resizeRatio))*0.5f);
    destRect->y = (float)(int)((screenHeight - (gameHeight*resizeRatio))*0.5f);
    destRect->width = (float)(int)(gameWidth*resizeRatio);
    destRect->height = (float)(int)(gameHeight*resizeRatio);
}

static void KeepHeightCentered(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect)
{
    const float resizeRatio = ((float)screenHeight/(float)gameHeight);
    sourceRect->x = 0.0f;
    sourceRect->y = 0.0f;
    sourceRect->width = (float)(int)((float)screenWidth/resizeRatio);
    sourceRect->height = (float)-gameHeight;

    destRect->x = (float)(int)((screenWidth - (sourceRect->width*resizeRatio))*0.5f);
    destRect->y = (float)(int)((screenHeight - (gameHeight*resizeRatio))*0.5f);
    destRect->width = (float)(int)(sourceRect->width*resizeRatio);
    destRect->height = (float)(int)(gameHeight*resizeRatio);
}

static void KeepWidthCentered(int screenWidth, int screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect)
{
    const float resizeRatio = ((float)screenWidth/(float)gameWidth);
    sourceRect->x = 0.0f;
    sourceRect->y = 0.0f;
    sourceRect->width = (float)gameWidth;
    sourceRect->height = (float)(int)((float)screenHeight/resizeRatio);

    destRect->x = (float)(int)((screenWidth - (gameWidth*resizeRatio))*0.5f);
    destRect->y = (float)(int)((screenHeight - (sourceRect->height*resizeRatio))*0.5f);
    destRect->width = (float)(int)(gameWidth*resizeRatio);
    destRect->height = (float)(int)(sourceRect->height*resizeRatio);

    sourceRect->height *= -1.f;
}

static void ResizeRenderSize(ViewportType viewportType, int *screenWidth, int *screenHeight, int gameWidth, int gameHeight, Rectangle *sourceRect, Rectangle *destRect, RenderTexture2D *target)
{
    *screenWidth = GetScreenWidth();
    *screenHeight = GetScreenHeight();

    switch(viewportType)
    {
        case KEEP_ASPECT_INTEGER: KeepAspectCenteredInteger(*screenWidth, *screenHeight, gameWidth, gameHeight, sourceRect, destRect); break;
        case KEEP_HEIGHT_INTEGER: KeepHeightCenteredInteger(*screenWidth, *screenHeight, gameWidth, gameHeight, sourceRect, destRect); break;
        case KEEP_WIDTH_INTEGER: KeepWidthCenteredInteger(*screenWidth, *screenHeight, gameWidth, gameHeight, sourceRect, destRect); break;
        case KEEP_ASPECT: KeepAspectCentered(*screenWidth, *screenHeight, gameWidth, gameHeight, sourceRect, destRect); break;
        case KEEP_HEIGHT: KeepHeightCentered(*screenWidth, *screenHeight, gameWidth, gameHeight, sourceRect, destRect); break;
        case KEEP_WIDTH: KeepWidthCentered(*screenWidth, *screenHeight, gameWidth, gameHeight, sourceRect, destRect); break;
        default: break;
    }

    UnloadRenderTexture(*target);
    *target = LoadRenderTexture(sourceRect->width, -sourceRect->height);
}

// Example how to calculate position on RenderTexture
static Vector2 Screen2RenderTexturePosition(Vector2 point, Rectangle *textureRect, Rectangle *scaledRect)
{
    Vector2 relativePosition = {point.x - scaledRect->x, point.y - scaledRect->y};
    Vector2 ratio = {textureRect->width/scaledRect->width, -textureRect->height/scaledRect->height};

    return (Vector2){relativePosition.x*ratio.x, relativePosition.y*ratio.x};
}