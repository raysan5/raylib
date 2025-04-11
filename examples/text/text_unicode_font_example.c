#include <stdlib.h>
#include <string.h>
#include <raylib.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 450

Font LoadUnicodeFont(const char* fileName, int fontSize, int textureFilter)
{
    int* cp = NULL;         // Array to store Unicode codepoints
    int capacity = 65536;   // Initial capacity
    int count = 0;          // Counter for codepoints
    
    // Allocate initial array
    cp = (int*)malloc(capacity * sizeof(int));
    if (!cp) return GetFontDefault();
    
    // Helper function to add a range of Unicode codepoints
    void AddRange(int start, int stop)
    {
        while (start <= stop)
        {
            // Expand array if needed
            if (count >= capacity)
            {
                capacity += 1024;
                int* newCp = (int*)realloc(cp, capacity * sizeof(int));
                if (!newCp)
                {
                    free(cp);
                    return GetFontDefault();
                }
                cp = newCp;
            }
            
            // Add current codepoint and increment
            cp[count] = start;
            count++;
            start++;
        }
    }
    
    // --------------------------------------------------
    // 1. BASIC ASCII CHARACTERS
    // --------------------------------------------------
    AddRange(32, 126);  // Basic Latin (letters, digits, punctuation)
    
    // --------------------------------------------------
    // 2. EUROPEAN LANGUAGES (LATIN SCRIPT)
    // --------------------------------------------------
    AddRange(0xC0, 0x17F);  // Latin-1 Supplement + Latin Extended-A
    AddRange(0x180, 0x24F); // Latin Extended-B
    AddRange(0x1E00, 0x1EFF); // Latin Extended Additional
    AddRange(0x2C60, 0x2C7F); // Latin Extended-C
    
    // --------------------------------------------------
    // 3. GREEK AND COPTIC
    // --------------------------------------------------
    AddRange(0x370, 0x3FF); // Greek and Coptic
    AddRange(0x1F00, 0x1FFF); // Greek Extended
    
    // --------------------------------------------------
    // 4. CYRILLIC SCRIPTS
    // --------------------------------------------------
    AddRange(0x400, 0x4FF); // Basic Cyrillic
    AddRange(0x500, 0x52F); // Cyrillic Supplement
    AddRange(0x2DE0, 0x2DFF); // Cyrillic Extended-A
    AddRange(0xA640, 0xA69F); // Cyrillic Extended-B
    
    // --------------------------------------------------
    // 5. CJK LANGUAGES (CHINESE, JAPANESE, KOREAN)
    // --------------------------------------------------
    AddRange(0x4E00, 0x9FFF); // CJK Unified Ideographs
    AddRange(0x3400, 0x4DBF); // CJK Extension A
    AddRange(0x3000, 0x303F); // CJK Symbols and Punctuation
    AddRange(0x3040, 0x309F); // Hiragana (Japanese)
    AddRange(0x30A0, 0x30FF); // Katakana (Japanese)
    AddRange(0x31F0, 0x31FF); // Katakana Phonetic Extensions
    AddRange(0xFF00, 0xFFEF); // Halfwidth and Fullwidth Forms
    AddRange(0xAC00, 0xD7AF); // Hangul Syllables (Korean)
    AddRange(0x1100, 0x11FF); // Hangul Jamo
    
    // --------------------------------------------------
    // 6. SOUTHEAST ASIAN LANGUAGES
    // --------------------------------------------------
    AddRange(0x0E00, 0x0E7F); // Thai
    AddRange(0x0E80, 0x0EFF); // Lao
    AddRange(0x1780, 0x17FF); // Khmer
    AddRange(0x1000, 0x109F); // Myanmar
    AddRange(0x1980, 0x19DF); // New Tai Lue
    
    // --------------------------------------------------
    // 7. INDIAN SUBCONTINENT LANGUAGES
    // --------------------------------------------------
    AddRange(0x900, 0x97F);  // Devanagari (Hindi, Sanskrit)
    AddRange(0x980, 0x9FF);  // Bengali
    AddRange(0xA00, 0xA7F);  // Gurmukhi (Punjabi)
    AddRange(0xA80, 0xAFF);  // Gujarati
    AddRange(0xB00, 0xB7F);  // Oriya
    AddRange(0xB80, 0xBFF);  // Tamil
    AddRange(0xC00, 0xC7F);  // Telugu
    AddRange(0xC80, 0xCFF);  // Kannada
    AddRange(0xD00, 0xD7F);  // Malayalam
    AddRange(0xD80, 0xDFF);  // Sinhala
    
    // --------------------------------------------------
    // 8. MIDDLE EASTERN LANGUAGES
    // --------------------------------------------------
    AddRange(0x600, 0x6FF);  // Arabic
    AddRange(0x750, 0x77F);  // Arabic Supplement
    AddRange(0x8A0, 0x8FF);  // Arabic Extended-A
    AddRange(0xFB50, 0xFDFF); // Arabic Presentation Forms-A
    AddRange(0x5D0, 0x5EA);  // Hebrew
    AddRange(0x591, 0x5C7);  // Hebrew Extended
    AddRange(0x7C0, 0x7FF);  // N'Ko
    AddRange(0x640, 0x6FF);  // Syriac
    
    // --------------------------------------------------
    // 9. AFRICAN LANGUAGES
    // --------------------------------------------------
    AddRange(0x2C80, 0x2CFF); // Coptic
    AddRange(0x2D30, 0x2D7F); // Tifinagh
    AddRange(0xA6A0, 0xA6FF); // Bamum
    AddRange(0xAB00, 0xAB2F); // Ethiopic Extended
    
    // --------------------------------------------------
    // 10. SPECIAL CHARACTERS AND SYMBOLS
    // --------------------------------------------------
    AddRange(0x300, 0x36F);  // Combining Diacritical Marks
    AddRange(0x1DC0, 0x1DFF); // Combining Diacritical Marks Supplement
    AddRange(0x2000, 0x206F); // General Punctuation
    AddRange(0x20A0, 0x20CF); // Currency Symbols
    AddRange(0x2100, 0x214F); // Letterlike Symbols
    AddRange(0x2190, 0x21FF); // Arrows
    AddRange(0x2200, 0x22FF); // Mathematical Operators

    
    Font result = {0};
    
    if (FileExists(fileName))
        result = LoadFontEx(fileName, fontSize, cp, count);
    
    if (result.texture.id == 0)
        result = GetFontDefault();
    
    SetTextureFilter(result.texture, textureFilter);
    free(cp);
    return result;
}

int main(void)
{
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Unicode Font Example");
    SetTargetFPS(60);

    // Load font with Unicode support
    Font myFont = LoadUnicodeFont("resources/NotoSansTC-Regular.ttf", 36, TEXTURE_FILTER_BILINEAR);

    // Main game loop
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            // Draw text
            DrawTextEx(myFont, "English: Hello World!", (Vector2){50, 50}, 36, 1, DARKGRAY);
            DrawTextEx(myFont, "Русский: Привет мир!", (Vector2){50, 100}, 36, 0, DARKGRAY);
            DrawTextEx(myFont, "中文: 你好世界!", (Vector2){50, 150}, 36, 1, DARKGRAY);
            DrawTextEx(myFont, "日本語: こんにちは世界!", (Vector2){50, 200}, 36, 1, DARKGRAY);

           
            DrawText("Font: Noto Sans TC Thin. © 2014-2020 Adobe(http://www.adobe.com/). License: SIL Open Font License 1.1", 10, SCREEN_HEIGHT - 20, 10, GRAY);

        EndDrawing();
    }

    // Cleanup
    UnloadFont(myFont);
    CloseWindow();

    return 0;
}
