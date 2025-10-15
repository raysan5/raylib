#include "raylib.h"
#include <stdlib.h>

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - clipboard text operations");

    char textBuffer[256] = "Try copying this text!";
    char clipboardText[256] = "Nothing copied yet...";
    
    Rectangle textBox = { 100, 180, 600, 50 };
    Rectangle copyButton = { 100, 250, 200, 40 };
    Rectangle pasteButton = { 500, 250, 200, 40 };
    
    bool mouseOnText = false;
    int framesCounter = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        mouseOnText = CheckCollisionPointRec(GetMousePosition(), textBox);
        
        if (mouseOnText)
        {
            SetMouseCursor(MOUSE_CURSOR_IBEAM);
            
            int key = GetCharPressed();
            while (key > 0)
            {
                if ((key >= 32) && (key <= 125))
                {
                    int length = TextLength(textBuffer);
                    if (length < 255)
                    {
                        textBuffer[length] = (char)key;
                        textBuffer[length + 1] = '\0';
                    }
                }
                key = GetCharPressed();
            }
            
            if (IsKeyPressed(KEY_BACKSPACE))
            {
                int length = TextLength(textBuffer);
                if (length > 0) textBuffer[length - 1] = '\0';
            }
        }
        else
        {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
        
        if (CheckCollisionPointRec(GetMousePosition(), copyButton))
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                SetClipboardText(textBuffer);
                TextCopy(clipboardText, "Text copied to clipboard!");
            }
        }
        
        if (CheckCollisionPointRec(GetMousePosition(), pasteButton))
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                const char *clipboard = GetClipboardText();
                if (clipboard != NULL && TextLength(clipboard) > 0)
                {
                    TextCopy(clipboardText, TextFormat("Pasted: %s", clipboard));
                }
                else
                {
                    TextCopy(clipboardText, "Clipboard is empty!");
                }
            }
        }
        
        if (mouseOnText) framesCounter++;
        else framesCounter = 0;

        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("CLIPBOARD TEXT OPERATIONS", 240, 40, 20, DARKGRAY);
            DrawText("Try copying text from this input box:", 100, 140, 20, DARKGRAY);
            
            DrawRectangleRec(textBox, LIGHTGRAY);
            if (mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
            else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);
            
            DrawText(textBuffer, (int)textBox.x + 5, (int)textBox.y + 8, 20, MAROON);
            
            if (mouseOnText && (((framesCounter/20)%2) == 0))
            {
                DrawText("_", (int)textBox.x + 8 + MeasureText(textBuffer, 20), (int)textBox.y + 12, 20, MAROON);
            }
            
            DrawRectangleRec(copyButton, CheckCollisionPointRec(GetMousePosition(), copyButton) ? SKYBLUE : BLUE);
            DrawRectangleRec(pasteButton, CheckCollisionPointRec(GetMousePosition(), pasteButton) ? SKYBLUE : BLUE);
            
            DrawText("COPY TO CLIPBOARD", (int)copyButton.x + 10, (int)copyButton.y + 10, 20, WHITE);
            DrawText("PASTE FROM CLIPBOARD", (int)pasteButton.x + 10, (int)pasteButton.y + 10, 20, WHITE);
            
            DrawText("Clipboard Status:", 100, 320, 20, DARKGRAY);
            DrawText(clipboardText, 100, 350, 20, DARKBLUE);
            
            DrawText("Try copying text from other applications and paste here!", 100, 400, 15, GRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}