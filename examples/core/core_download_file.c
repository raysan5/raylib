// Example written by Rabia Alhaffar in 14/June/2020
// To know how to use DownloadFile function

#include <raylib.h>

const char *file2download = "https://raw.githubusercontent.com/Rabios/raylib/master/src/core.c";

int main(void) {
    InitWindow(0,0,"DOWNLOAD FILE!!!");
    DownloadFile(file2download,"made_with_example");
    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(WHITE);
            DrawText("FILE DOWNLOADED SUCCESSFULLY!!!",10,10,32,BLACK);
        EndDrawing();        
    } 
    CloseWindow();
    return 0;
}