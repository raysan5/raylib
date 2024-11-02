#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

static Image img = {0};
int main(int argc, char *argv[]) {

    InitWindow(800, 450, "[core] raylib clipboard image");
    SetTraceLogLevel(LOG_TRACE);
    SetTargetFPS(60);

    Texture tex = {0};
    while(!WindowShouldClose()) {
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {
            #ifdef _WIN32
            img = GetClipboardImage();
            tex = LoadTextureFromImage(img);
            if(!IsTextureValid(tex)) {
                exit(98);
            } else {
                ExportImage(img, "Debug.bmp");
            }
            #endif
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            if (IsTextureValid(tex)) {
                DrawTexture(tex, 0, 10 + 21, WHITE);
            }
            DrawText("Print Screen and Crtl+V", 10, 10, 21, BLACK);
        EndDrawing();
    }
}
