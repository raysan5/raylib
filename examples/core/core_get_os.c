// Written by Rabia Alhaffar in 11/June/2020
// Example to check current operating system used

#include <raylib.h>

int main(void) {
    InitWindow(0,0,"Check OS");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText(GetOS(),10,10,32,BLACK);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}