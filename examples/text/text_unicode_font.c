#include <stdlib.h>
#include <raylib.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 450

typedef struct {
    int* data;
    int count;
    int capacity;
} CodepointsArray;

static void AddRange(CodepointsArray* array, int start, int stop) {
    int rangeSize = stop - start + 1;
    
    if (array->count + rangeSize > array->capacity) {
        array->capacity = array->count + rangeSize + 1024;
        array->data = (int*)MemRealloc(array->data, array->capacity * sizeof(int));
        if (!array->data) {
            TraceLog(LOG_ERROR, "FONTUTIL: Memory allocation failed");
            exit(1);
        }
    }
    
    for (int i = start; i <= stop; i++) {
        array->data[array->count++] = i;
    }
}

Font LoadUnicodeFont(const char* fileName, int fontSize, int textureFilter) {
    CodepointsArray cp = {0};
    cp.capacity = 2048;
    cp.data = (int*)MemAlloc(cp.capacity * sizeof(int));

    if (!cp.data) {
        TraceLog(LOG_ERROR, "FONTUTIL: Initial allocation failed");
        return GetFontDefault();
    }

    // Basic ASCII
    AddRange(&cp, 32, 126);

    // European Languages
    AddRange(&cp, 0xC0, 0x17F);
    AddRange(&cp, 0x180, 0x24F);
    AddRange(&cp, 0x1E00, 0x1EFF);
    AddRange(&cp, 0x2C60, 0x2C7F);

    // Greek
    AddRange(&cp, 0x370, 0x3FF);
    AddRange(&cp, 0x1F00, 0x1FFF);

    // Cyrillic
    AddRange(&cp, 0x400, 0x4FF);
    AddRange(&cp, 0x500, 0x52F);
    AddRange(&cp, 0x2DE0, 0x2DFF);
    AddRange(&cp, 0xA640, 0xA69F);

    // CJK
    AddRange(&cp, 0x4E00, 0x9FFF);
    AddRange(&cp, 0x3400, 0x4DBF);
    AddRange(&cp, 0x3000, 0x303F);
    AddRange(&cp, 0x3040, 0x309F);
    AddRange(&cp, 0x30A0, 0x30FF);
    AddRange(&cp, 0x31F0, 0x31FF);
    AddRange(&cp, 0xFF00, 0xFFEF);
    AddRange(&cp, 0xAC00, 0xD7AF);
    AddRange(&cp, 0x1100, 0x11FF);

    // Other 
    AddRange(&cp, 0x900, 0x97F);  // Devanagari
    AddRange(&cp, 0x600, 0x6FF);  // Arabic
    AddRange(&cp, 0x5D0, 0x5EA);  // Hebrew

    Font font = {0};
    
    if (FileExists(fileName)) {
        font = LoadFontEx(fileName, fontSize, cp.data, cp.count);
    }
    
    if (font.texture.id == 0) {
        font = GetFontDefault();
        TraceLog(LOG_WARNING, "FONTUTIL: Using default font");
    }
    
    SetTextureFilter(font.texture, textureFilter);
    MemFree(cp.data);
    
    return font;
}

/**
 * Main entry point
 */
int main(void)
{
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Unicode Font Example");
    SetTargetFPS(60);

    // Load font with Unicode support
    Font myFont = LoadUnicodeFont("resources/NotoSansTC-Regular.ttf", 36, TEXTURE_FILTER_BILINEAR);

    // Main render loop
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            // Render test strings in different languages
            DrawTextEx(myFont, "English: Hello World!", (Vector2){50, 50}, 36, 1, DARKGRAY);
            DrawTextEx(myFont, "Русский: Привет мир!", (Vector2){50, 100}, 36, 0, DARKGRAY);
            DrawTextEx(myFont, "中文: 你好世界!", (Vector2){50, 150}, 36, 1, DARKGRAY);
            DrawTextEx(myFont, "日本語: こんにちは世界!", (Vector2){50, 200}, 36, 1, DARKGRAY);

            // Display font attribution
            DrawText("Font: Noto Sans TC. License: SIL Open Font License 1.1", 
                    10, SCREEN_HEIGHT - 20, 10, GRAY);
        EndDrawing();
    }

    // Cleanup resources
    UnloadFont(myFont);
    CloseWindow();

    return 0;
}
