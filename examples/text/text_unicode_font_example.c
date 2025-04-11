#include <stdlib.h>
#include <string.h>
#include <raylib.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 450
#define INITIAL_CAPACITY 65536  // 2^16 - sufficient for most Unicode BMP characters


static int AddCodeRange(int* codePoints, int* count, int start, int stop)
{
    // Verify we have enough capacity for this range
    if (*count + (stop - start + 1) > INITIAL_CAPACITY) {
        return 0;  // Not enough space
    }

    // Add all code points in the range
    while (start <= stop) {
        codePoints[*count] = start;
        (*count)++;
        start++;
    }
    return 1;  // Success
}

Font LoadUnicodeFont(const char* fileName, int fontSize, int textureFilter)
{
    // Allocate memory for code points (fixed size - no reallocation needed)
    int* codePoints = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
    if (!codePoints) return GetFontDefault();
    
    int count = 0;  // Tracks number of added code points

    // --------------------------------------------------
    // 1. BASIC ASCII CHARACTERS
    // --------------------------------------------------
    AddCodeRange(codePoints, &count, 32, 126);  // Basic Latin (letters, digits, punctuation)
    
    // --------------------------------------------------
    // 2. EUROPEAN LANGUAGES (LATIN SCRIPT)
    // --------------------------------------------------
    AddCodeRange(codePoints, &count, 0xC0, 0x17F);  // Latin-1 Supplement + Latin Extended-A
    AddCodeRange(codePoints, &count, 0x180, 0x24F); // Latin Extended-B
    AddCodeRange(codePoints, &count, 0x1E00, 0x1EFF); // Latin Extended Additional
    AddCodeRange(codePoints, &count, 0x2C60, 0x2C7F); // Latin Extended-C
    
    // --------------------------------------------------
    // 3. GREEK AND COPTIC
    // --------------------------------------------------
    AddCodeRange(codePoints, &count, 0x370, 0x3FF); // Greek and Coptic
    AddCodeRange(codePoints, &count, 0x1F00, 0x1FFF); // Greek Extended
    
    // --------------------------------------------------
    // 4. CYRILLIC SCRIPTS
    // --------------------------------------------------
    AddCodeRange(codePoints, &count, 0x400, 0x4FF); // Basic Cyrillic
    AddCodeRange(codePoints, &count, 0x500, 0x52F); // Cyrillic Supplement
    AddCodeRange(codePoints, &count, 0x2DE0, 0x2DFF); // Cyrillic Extended-A
    AddCodeRange(codePoints, &count, 0xA640, 0xA69F); // Cyrillic Extended-B
    
    // --------------------------------------------------
    // 5. CJK LANGUAGES (CHINESE, JAPANESE, KOREAN)
    // --------------------------------------------------
    AddCodeRange(codePoints, &count, 0x4E00, 0x9FFF); // CJK Unified Ideographs
    AddCodeRange(codePoints, &count, 0x3400, 0x4DBF); // CJK Extension A
    AddCodeRange(codePoints, &count, 0x3000, 0x303F); // CJK Symbols and Punctuation
    AddCodeRange(codePoints, &count, 0x3040, 0x309F); // Hiragana (Japanese)
    AddCodeRange(codePoints, &count, 0x30A0, 0x30FF); // Katakana (Japanese)
    AddCodeRange(codePoints, &count, 0x31F0, 0x31FF); // Katakana Phonetic Extensions
    AddCodeRange(codePoints, &count, 0xFF00, 0xFFEF); // Halfwidth and Fullwidth Forms
    AddCodeRange(codePoints, &count, 0xAC00, 0xD7AF); // Hangul Syllables (Korean)
    AddCodeRange(codePoints, &count, 0x1100, 0x11FF); // Hangul Jamo
    
    // --------------------------------------------------
    // 6. SOUTHEAST ASIAN LANGUAGES
    // --------------------------------------------------
    AddCodeRange(codePoints, &count, 0x0E00, 0x0E7F); // Thai
    AddCodeRange(codePoints, &count, 0x0E80, 0x0EFF); // Lao
    AddCodeRange(codePoints, &count, 0x1780, 0x17FF); // Khmer
    AddCodeRange(codePoints, &count, 0x1000, 0x109F); // Myanmar
    AddCodeRange(codePoints, &count, 0x1980, 0x19DF); // New Tai Lue
    
    // --------------------------------------------------
    // 7. INDIAN SUBCONTINENT LANGUAGES
    // --------------------------------------------------
    AddCodeRange(codePoints, &count, 0x900, 0x97F);  // Devanagari (Hindi, Sanskrit)
    AddCodeRange(codePoints, &count, 0x980, 0x9FF);  // Bengali
    AddCodeRange(codePoints, &count, 0xA00, 0xA7F);  // Gurmukhi (Punjabi)
    AddCodeRange(codePoints, &count, 0xA80, 0xAFF);  // Gujarati
    AddCodeRange(codePoints, &count, 0xB00, 0xB7F);  // Oriya
    AddCodeRange(codePoints, &count, 0xB80, 0xBFF);  // Tamil
    AddCodeRange(codePoints, &count, 0xC00, 0xC7F);  // Telugu
    AddCodeRange(codePoints, &count, 0xC80, 0xCFF);  // Kannada
    AddCodeRange(codePoints, &count, 0xD00, 0xD7F);  // Malayalam
    AddCodeRange(codePoints, &count, 0xD80, 0xDFF);  // Sinhala
    
    // --------------------------------------------------
    // 8. MIDDLE EASTERN LANGUAGES
    // --------------------------------------------------
    AddCodeRange(codePoints, &count, 0x600, 0x6FF);  // Arabic
    AddCodeRange(codePoints, &count, 0x750, 0x77F);  // Arabic Supplement
    AddCodeRange(codePoints, &count, 0x8A0, 0x8FF);  // Arabic Extended-A
    AddCodeRange(codePoints, &count, 0xFB50, 0xFDFF); // Arabic Presentation Forms-A
    AddCodeRange(codePoints, &count, 0x5D0, 0x5EA);  // Hebrew
    AddCodeRange(codePoints, &count, 0x591, 0x5C7);  // Hebrew Extended
    AddCodeRange(codePoints, &count, 0x7C0, 0x7FF);  // N'Ko
    AddCodeRange(codePoints, &count, 0x640, 0x6FF);  // Syriac
    
    // --------------------------------------------------
    // 9. AFRICAN LANGUAGES
    // --------------------------------------------------
    AddCodeRange(codePoints, &count, 0x2C80, 0x2CFF); // Coptic
    AddCodeRange(codePoints, &count, 0x2D30, 0x2D7F); // Tifinagh
    AddCodeRange(codePoints, &count, 0xA6A0, 0xA6FF); // Bamum
    AddCodeRange(codePoints, &count, 0xAB00, 0xAB2F); // Ethiopic Extended
    
    // --------------------------------------------------
    // 10. SPECIAL CHARACTERS AND SYMBOLS
    // --------------------------------------------------
    AddCodeRange(codePoints, &count, 0x300, 0x36F);  // Combining Diacritical Marks
    AddCodeRange(codePoints, &count, 0x1DC0, 0x1DFF); // Combining Diacritical Marks Supplement
    AddCodeRange(codePoints, &count, 0x2000, 0x206F); // General Punctuation
    AddCodeRange(codePoints, &count, 0x20A0, 0x20CF); // Currency Symbols
    AddCodeRange(codePoints, &count, 0x2100, 0x214F); // Letterlike Symbols
    AddCodeRange(codePoints, &count, 0x2190, 0x21FF); // Arrows
    AddCodeRange(codePoints, &count, 0x2200, 0x22FF); // Mathematical Operators

    Font result = {0};
    
    // Attempt to load the font with collected code points
    if (FileExists(fileName)) {
        result = LoadFontEx(fileName, fontSize, codePoints, count);
    }
    
    // Fallback to default font if loading fails
    if (result.texture.id == 0) {
        result = GetFontDefault();
    }
    
    // Apply texture filtering
    SetTextureFilter(result.texture, textureFilter);
    
    // Clean up
    free(codePoints);
    return result;
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
