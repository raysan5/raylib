/*******************************************************************************************
*
*   raylib [core] example - text file loading
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Aanjishnu Bhattacharyya (@NimComPoo-04) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 0 Aanjishnu Bhattacharyya (@NimComPoo-04)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"        // Required for: Lerp()

#include <string.h>

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - text file loading");

    // Setting up the camera
    Camera2D cam = {
        .offset = {0, 0},
        .target = {0, 0},
        .rotation = 0,
        .zoom = 1
    };

    // Loading text file from resources/text_file.txt
    const char *fileName = "resources/text_file.txt";
    char *text = LoadFileText(fileName);

    // Loading all the text lines
    int lineCount = 0;
    char **lines = LoadTextLines(text, &lineCount);

    // Stylistic choises
    int fontSize = 20;
    int textTop = 25 + fontSize; // Top of the screen from where the text is rendered
    int wrapWidth = screenWidth - 20;

    // Wrap the lines as needed
    for (int i = 0; i < lineCount; i++)
    {
        int j = 0;
        int lastSpace = 0;          // Keeping track of last valid space to insert '\n'
        int lastWrapStart = 0;      // Keeping track of the start of this wrapped line.

        while (j <= strlen(lines[i]))
        {
            if (lines[i][j] == ' ' || lines[i][j] == '\0')
            {
                char before = lines[i][j];
                // Making a C Style string by adding a '\0' at the required location so that we can use the MeasureText function
                lines[i][j] = '\0';

                // Checking if the text has crossed the wrapWidth, then going back and inserting a newline
                if (MeasureText(lines[i] + lastWrapStart, fontSize) > wrapWidth)
                {
                    lines[i][lastSpace] = '\n';

                    // Since we added a newline the place of wrap changed so we update our lastWrapStart
                    lastWrapStart = lastSpace + 1;
                }

                if(before != '\0') lines[i][j] = ' ';  // Resetting the space back
                lastSpace = j; // Since we encountered a new space we update our last encountered space location
            }

            j++;
        }
    }

    // Calculating the total height so that we can show a scrollbar
    int textHeight = 0;

    for (int i = 0; i < lineCount; i++)
    {
        Vector2 size = MeasureTextEx(GetFontDefault(), lines[i], (float)fontSize, 2);
        textHeight += (int)size.y + 10;
    }

    // A simple scrollbar on the side to show how far we have read into the file
    Rectangle scrollBar = {
        .x = (float)screenWidth - 5,
        .y = 0,
        .width = 5,
        .height = screenHeight*100.0f/(textHeight - screenHeight) // Scrollbar height is just a percentage
    };

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        float scroll = GetMouseWheelMove();
        cam.target.y -= scroll*fontSize*1.5f;   // Choosing an arbitrary speed for scroll

        if (cam.target.y < 0) cam.target.y = 0;  // Snapping to 0 if we go too far back

        // Ensuring that the camera does not scroll past all text
        if (cam.target.y > textHeight - screenHeight + textTop)
            cam.target.y = (float)textHeight - screenHeight + textTop;

        // Computing the position of the scrollBar depending on the percentage of text covered
        scrollBar.y = Lerp((float)textTop, (float)screenHeight - scrollBar.height, (float)(cam.target.y - textTop)/(textHeight - screenHeight));
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode2D(cam);
                // Going through all the read lines
                for (int i = 0, t = textTop; i < lineCount; i++)
                {
                    // Each time we go through and calculate the height of the text to move the cursor appropriately
                    Vector2 size;
                    if(strcmp(lines[i], "")){
                        // Fix for empty line in the text file
                        size = MeasureTextEx( GetFontDefault(), lines[i], (float)fontSize, 2);
                    }else{
                        size = MeasureTextEx( GetFontDefault(), " ", (float)fontSize, 2);
                    }

                    DrawText(lines[i], 10, t, fontSize, RED);

                    // Inserting extra space for real newlines,
                    // wrapped lines are rendered closer together
                    t += (int)size.y + 10;
                }
            EndMode2D();

            // Header displaying which file is being read currently
            DrawRectangle(0, 0, screenWidth, textTop - 10, BEIGE);
            DrawText(TextFormat("File: %s", fileName), 10, 10, fontSize, MAROON);

            DrawRectangleRec(scrollBar, MAROON);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTextLines(lines, lineCount);
    UnloadFileText(text);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
