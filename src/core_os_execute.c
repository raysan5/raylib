#include <raylib.h>

int main(void) {
    InitWindow(0,0,"Execute");
    Execute("mkdir made_with_example");
    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Command executed successfully!!!",10,10,32,BLACK);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}