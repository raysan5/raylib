/*******************************************************************************************
*
*   raylib [text] example - unicode ranges
*
*   Example complexity rating: [★★★★] 4/4
*
*   Example originally created with raylib 2.5, last time updated with raylib 4.0
*
*   Example contributed by Vlad Adrian (@demizdor) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2019-2025 Vlad Adrian (@demizdor) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>

typedef struct {
    int* data;
    int count;
    int capacity;
} CodepointsArray;

//--------------------------------------------------------------------------------------
// Module functions declaration
//--------------------------------------------------------------------------------------
static void AddRange(CodepointsArray* array, int start, int stop);
static Font LoadUnicodeFont(const char* fileName, int fontSize);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - unicode ranges");

    // Load font with Unicode support
    Font fontUni = LoadUnicodeFont("resources/NotoSansTC-Regular.ttf", 32);
    SetTextureFilter(fontUni.texture, TEXTURE_FILTER_BILINEAR);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        //...
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            // Render test strings in different languages
            DrawTextEx(fontUni, "English: Hello World!", (Vector2){ 50, 50 }, 32, 1, DARKGRAY); // English
            DrawTextEx(fontUni, "Español: Hola mundo!", (Vector2){ 50, 100 }, 32, 1, DARKGRAY); // Spanish
            DrawTextEx(fontUni, "Ελληνικά: Γειά σου κόσμε!", (Vector2){ 50, 150 }, 32, 1, DARKGRAY); // Greek
            DrawTextEx(fontUni, "Русский: Привет мир!", (Vector2){ 50, 200 }, 32, 0, DARKGRAY); // Russian
            DrawTextEx(fontUni, "中文: 你好世界!", (Vector2){ 50, 250 }, 32, 1, DARKGRAY);        // Chinese
            DrawTextEx(fontUni, "日本語: こんにちは世界!", (Vector2){ 50, 300 }, 32, 1, DARKGRAY);   // Japanese
            DrawTextEx(fontUni, "देवनागरी: होला मुंडो!", (Vector2){ 50, 350 }, 32, 1, DARKGRAY);     // Devanagari
            
            DrawRectangle(400, 16, 380, 400, BLACK);
            DrawTexturePro(fontUni.texture, (Rectangle){ 0, 0, fontUni.texture.width, fontUni.texture.height },
                (Rectangle){ 400, 16, 380, 400 }, (Vector2){ 0, 0 }, 0.0f, WHITE);

            // Display font attribution
            DrawText("Font: Noto Sans TC. License: SIL Open Font License 1.1", screenWidth - 300, screenHeight - 20, 10, GRAY);
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadFont(fontUni);        // Unload font resource

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------
static void AddRange(CodepointsArray* array, int start, int stop)
{
    int rangeSize = stop - start + 1;
    
    if ((array->count + rangeSize) > array->capacity)
    {
        array->capacity = array->count + rangeSize + 1024;
        array->data = (int *)MemRealloc(array->data, array->capacity*sizeof(int));
        
        if (!array->data)
        {
            TraceLog(LOG_ERROR, "FONTUTIL: Memory allocation failed");
            exit(1);
        }
    }
    
    for (int i = start; i <= stop; i++) array->data[array->count++] = i;
}

Font LoadUnicodeFont(const char *fileName, int fontSize)
{
    CodepointsArray cp = { 0 };
    cp.capacity = 2048;
    cp.data = (int *)MemAlloc(cp.capacity*sizeof(int));

    if (!cp.data)
    {
        TraceLog(LOG_ERROR, "FONTUTIL: Initial allocation failed");
        return GetFontDefault();
    }

    // Unicode range: Basic ASCII
    AddRange(&cp, 32, 126);

    // Unicode range: European Languages
    AddRange(&cp, 0xC0, 0x17F);
    AddRange(&cp, 0x180, 0x24F);
    AddRange(&cp, 0x1E00, 0x1EFF);
    AddRange(&cp, 0x2C60, 0x2C7F);

    // Unicode range: Greek
    AddRange(&cp, 0x370, 0x3FF);
    AddRange(&cp, 0x1F00, 0x1FFF);

    // Unicode range: Cyrillic
    AddRange(&cp, 0x400, 0x4FF);
    AddRange(&cp, 0x500, 0x52F);
    AddRange(&cp, 0x2DE0, 0x2DFF);
    AddRange(&cp, 0xA640, 0xA69F);

    // Unicode range: CJK
    AddRange(&cp, 0x4E00, 0x9FFF);
    AddRange(&cp, 0x3400, 0x4DBF);
    AddRange(&cp, 0x3000, 0x303F);
    AddRange(&cp, 0x3040, 0x309F);
    AddRange(&cp, 0x30A0, 0x30FF);
    AddRange(&cp, 0x31F0, 0x31FF);
    AddRange(&cp, 0xFF00, 0xFFEF);
    AddRange(&cp, 0xAC00, 0xD7AF);
    AddRange(&cp, 0x1100, 0x11FF);

    // Unicode range: Other
    // WARNING: Not available on provided font
    AddRange(&cp, 0x900, 0x97F);  // Devanagari
    AddRange(&cp, 0x600, 0x6FF);  // Arabic
    AddRange(&cp, 0x5D0, 0x5EA);  // Hebrew

    Font font = {0};
    
    if (FileExists(fileName))
    {
        font = LoadFontEx(fileName, fontSize, cp.data, cp.count);
    }
    
    if (font.texture.id == 0)
    {
        font = GetFontDefault();
        TraceLog(LOG_WARNING, "FONTUTIL: Using default font");
    }

    MemFree(cp.data);
    
    return font;
}
