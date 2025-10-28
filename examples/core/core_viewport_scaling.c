/*******************************************************************************************
*
*   raylib [core] example - viewport scaling
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Agnis Aldins (@nezvers) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Agnis Aldins (@nezvers)
*
********************************************************************************************/

#include "raylib.h"

int screenWidth = 800;
int screenHeight = 450;
int gameWidth = 320;
int gameHeight = 180;

RenderTexture2D target;
Rectangle sourceRect;
Rectangle destRect;

// For itteration purposes and teaching example
#define RESOLUTION_COUNT 4
// Preset resolutions that could be created by subdividing screen resolution
Vector2 resolutionList[RESOLUTION_COUNT] = {
    (Vector2){64, 64},
    (Vector2){256, 240},
    (Vector2){320, 180},
    // 4K doesn't work with integer scaling but included for example purposes with non-integer scaling
    (Vector2){3840, 2160},
};
int resolutionIndex = 2;

enum ViewportType 
{
    // Only upscale, useful for pixel art
    KEEP_ASPECT_INTEGER,
    KEEP_HEIGHT_INTEGER,
    KEEP_WIDTH_INTEGER,
    // Can also downscale
    KEEP_ASPECT,
    KEEP_HEIGHT,
    KEEP_WIDTH,
    // For itteration purposes and teaching example
    VIEWPORT_TYPE_COUNT,
};
// For displaying on GUI
const char *ViewportTypeNames[VIEWPORT_TYPE_COUNT] = {
    "KEEP_ASPECT_INTEGER",
    "KEEP_HEIGHT_INTEGER",
    "KEEP_WIDTH_INTEGER",
    "KEEP_ASPECT",
    "KEEP_HEIGHT",
    "KEEP_WIDTH",
};
enum ViewportType viewportType = KEEP_ASPECT_INTEGER;

// Function declarations (header part), implementation at bottom
void KeepAspectCenteredInteger(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect);
void KeepHeightCenteredInteger(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect);
void KeepWidthCenteredInteger(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect);
void KeepAspectCentered(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect);
void KeepHeightCentered(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect);
void KeepWidthCentered(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect);

void ResizeRenderSize()
{
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    switch(viewportType)
    {
        case KEEP_ASPECT_INTEGER:
        {
            KeepAspectCenteredInteger(screenWidth, screenHeight, gameWidth, gameHeight, &sourceRect, &destRect);
            break;
        }
        case KEEP_HEIGHT_INTEGER:
        {
            KeepHeightCenteredInteger(screenWidth, screenHeight, gameWidth, gameHeight, &sourceRect, &destRect);
            break;
        }
        case KEEP_WIDTH_INTEGER:
        {
            KeepWidthCenteredInteger(screenWidth, screenHeight, gameWidth, gameHeight, &sourceRect, &destRect);
            break;
        }
        case KEEP_ASPECT:
        {
            KeepAspectCentered(screenWidth, screenHeight, gameWidth, gameHeight, &sourceRect, &destRect);
            break;
        }
        case KEEP_HEIGHT:
        {
            KeepHeightCentered(screenWidth, screenHeight, gameWidth, gameHeight, &sourceRect, &destRect);
            break;
        }
        case KEEP_WIDTH:
        {
            KeepWidthCentered(screenWidth, screenHeight, gameWidth, gameHeight, &sourceRect, &destRect);
            break;
        }
        default: {}
    }
    UnloadRenderTexture(target);
    target = LoadRenderTexture(sourceRect.width, -sourceRect.height);
}

// Example how to calculate position on RenderTexture
Vector2 Screen2RenderTexturePosition(Vector2 point, Rectangle* textureRect, Rectangle* scaledRect){
    Vector2 relativePosition = {point.x - scaledRect->x, point.y - scaledRect->y};
    Vector2 ratio = {textureRect->width / scaledRect->width, -textureRect->height / scaledRect->height};

    return (Vector2){relativePosition.x * ratio.x, relativePosition.y * ratio.x};
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - Viewport Scaling");
    ResizeRenderSize();

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //----------------------------------------------------------

    // Button rectangles
    Rectangle decreaseResolutionButton = (Rectangle){200, 30, 10, 10};
    Rectangle increaseResolutionButton = (Rectangle){215, 30, 10, 10};
    Rectangle decreaseTypeButton = (Rectangle){200, 45, 10, 10};
    Rectangle increaseTypeButton = (Rectangle){215, 45, 10, 10};
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //-----------------------------------------------------
        if (IsWindowResized()){
            ResizeRenderSize();
        }
        Vector2 mousePosition = GetMousePosition();
        bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        
        // Check buttons and rescale
        if (CheckCollisionPointRec(mousePosition, decreaseResolutionButton) && mousePressed){
            resolutionIndex = (resolutionIndex + RESOLUTION_COUNT - 1) % RESOLUTION_COUNT;
            gameWidth = resolutionList[resolutionIndex].x;
            gameHeight = resolutionList[resolutionIndex].y;
            ResizeRenderSize();
        }
        if (CheckCollisionPointRec(mousePosition, increaseResolutionButton) && mousePressed){
            resolutionIndex = (resolutionIndex + 1) % RESOLUTION_COUNT;
            gameWidth = resolutionList[resolutionIndex].x;
            gameHeight = resolutionList[resolutionIndex].y;
            ResizeRenderSize();
        }
        if (CheckCollisionPointRec(mousePosition, decreaseTypeButton) && mousePressed){
            viewportType = (viewportType + VIEWPORT_TYPE_COUNT - 1) % VIEWPORT_TYPE_COUNT;
            ResizeRenderSize();
        }
        if (CheckCollisionPointRec(mousePosition, increaseTypeButton) && mousePressed){
            viewportType = (viewportType + 1) % VIEWPORT_TYPE_COUNT;
            ResizeRenderSize();
        }

        Vector2 textureMousePosition = Screen2RenderTexturePosition(mousePosition, &sourceRect, &destRect);

        // Draw
        //-----------------------------------------------------
        // Draw our scene to the render texture
        BeginTextureMode(target);
            ClearBackground(WHITE);
            DrawCircle(textureMousePosition.x, textureMousePosition.y, 20.f, LIME);


        EndTextureMode();

        // Draw render texture to main framebuffer
        BeginDrawing();
            ClearBackground(BLACK);

            // Draw our render texture with rotation applied
            const Vector2 ORIGIN_POSITION = (Vector2){ 0.0f, 0.0f };
            const float ROTATION = 0.f;
            DrawTexturePro(target.texture, sourceRect, destRect, ORIGIN_POSITION, ROTATION, WHITE);

            // Draw Native resolution (GUI or anything)
            // Draw info box
            Rectangle infoRect = (Rectangle){5, 5, 330, 105};
            DrawRectangleRec(infoRect, Fade(LIGHTGRAY, 0.7f));
            DrawRectangleLines(infoRect.x, infoRect.y, infoRect.width, infoRect.height, BLUE);

            DrawText(TextFormat("Window Resolution: %d x %d", screenWidth, screenHeight), 15, 15, 10, BLACK);
            DrawText(TextFormat("Game Resolution: %d x %d", gameWidth, gameHeight), 15, 30, 10, BLACK);

            Vector2 scaleRatio = (Vector2){destRect.width / sourceRect.width, destRect.height / -sourceRect.height};
            DrawText(TextFormat("Type: %s", ViewportTypeNames[viewportType]), 15, 45, 10, BLACK);
            DrawText(TextFormat("Scale ratio: %.2f x %.2f", scaleRatio.x, scaleRatio.y), 15, 60, 10, BLACK);
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
        //-----------------------------------------------------
    }

    // De-Initialization
    //---------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------

    return 0;
}

void KeepAspectCenteredInteger(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect){
    source_rect->x = 0.f;
    source_rect->y = (float)view_h;
    source_rect->width = (float)view_w;
    source_rect->height = (float)-view_h;

    const int ratio_x = (scr_w/view_w);
    const int ratio_y = (scr_h/view_h);
    const float resize_ratio = (float)(ratio_x < ratio_y ? ratio_x : ratio_y);

    dest_rect->x = (float)(int)((scr_w - (view_w * resize_ratio)) * 0.5);
    dest_rect->y = (float)(int)((scr_h - (view_h * resize_ratio)) * 0.5);
    dest_rect->width = (float)(int)(view_w * resize_ratio);
    dest_rect->height = (float)(int)(view_h * resize_ratio);
}

void KeepHeightCenteredInteger(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect){
    const float resize_ratio = ((float)scr_h/(float)view_h);
    source_rect->x = 0.f;
    source_rect->y = 0.f;
    source_rect->width = (float)(int)(scr_w / resize_ratio);
    source_rect->height = (float)-view_h;

    dest_rect->x = (float)(int)((scr_w - (source_rect->width * resize_ratio)) * 0.5);
    dest_rect->y = (float)(int)((scr_h - (view_h * resize_ratio)) * 0.5);
    dest_rect->width = (float)(int)(source_rect->width * resize_ratio);
    dest_rect->height = (float)(int)(view_h * resize_ratio);
}

void KeepWidthCenteredInteger(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect){
    const float resize_ratio = ((float)scr_w/(float)view_w);
    source_rect->x = 0.f;
    source_rect->y = 0.f;
    source_rect->width = (float)view_w;
    source_rect->height = (float)(int)(scr_h / resize_ratio);

    dest_rect->x = (float)(int)((scr_w - (view_w * resize_ratio)) * 0.5);
    dest_rect->y = (float)(int)((scr_h - (source_rect->height * resize_ratio)) * 0.5);
    dest_rect->width = (float)(int)(view_w * resize_ratio);
    dest_rect->height = (float)(int)(source_rect->height * resize_ratio);

    source_rect->height *= -1.f;
}

void KeepAspectCentered(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect){
    source_rect->x = 0.f;
    source_rect->y = (float)view_h;
    source_rect->width = (float)view_w;
    source_rect->height = (float)-view_h;

    const float ratio_x = ((float)scr_w/(float)view_w);
    const float ratio_y = ((float)scr_h/(float)view_h);
    const float resize_ratio = (ratio_x < ratio_y ? ratio_x : ratio_y);

    dest_rect->x = (float)(int)((scr_w - (view_w * resize_ratio)) * 0.5);
    dest_rect->y = (float)(int)((scr_h - (view_h * resize_ratio)) * 0.5);
    dest_rect->width = (float)(int)(view_w * resize_ratio);
    dest_rect->height = (float)(int)(view_h * resize_ratio);
}

void KeepHeightCentered(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect){
    const float resize_ratio = ((float)scr_h/(float)view_h);
    source_rect->x = 0.f;
    source_rect->y = 0.f;
    source_rect->width = (float)(int)((float)scr_w / resize_ratio);
    source_rect->height = (float)-view_h;

    dest_rect->x = (float)(int)((scr_w - (source_rect->width * resize_ratio)) * 0.5);
    dest_rect->y = (float)(int)((scr_h - (view_h * resize_ratio)) * 0.5);
    dest_rect->width = (float)(int)(source_rect->width * resize_ratio);
    dest_rect->height = (float)(int)(view_h * resize_ratio);
}

void KeepWidthCentered(int scr_w, int scr_h, int view_w, int view_h, Rectangle* source_rect, Rectangle* dest_rect){
    const float resize_ratio = ((float)scr_w/(float)view_w);
    source_rect->x = 0.f;
    source_rect->y = 0.f;
    source_rect->width = (float)view_w;
    source_rect->height = (float)(int)((float)scr_h / resize_ratio);

    dest_rect->x = (float)(int)((scr_w - (view_w * resize_ratio)) * 0.5);
    dest_rect->y = (float)(int)((scr_h - (source_rect->height * resize_ratio)) * 0.5);
    dest_rect->width = (float)(int)(view_w * resize_ratio);
    dest_rect->height = (float)(int)(source_rect->height * resize_ratio);

    source_rect->height *= -1.f;
}