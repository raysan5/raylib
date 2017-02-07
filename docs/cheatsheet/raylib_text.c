
    // SpriteFont loading/unloading functions
    SpriteFont GetDefaultFont(void);                                                                    // Get the default SpriteFont
    SpriteFont LoadSpriteFont(const char *fileName);                                                    // Load a SpriteFont image into GPU memory
    SpriteFont LoadSpriteFontTTF(const char *fileName, int fontSize, int numChars, int *fontChars);     // Load a SpriteFont from TTF font with parameters
    void UnloadSpriteFont(SpriteFont spriteFont);                                                       // Unload SpriteFont from GPU memory

    // Text drawing functions
    void DrawText(const char *text, int posX, int posY, int fontSize, Color color);                     // Draw text (using default font)
    void DrawTextEx(SpriteFont spriteFont, const char* text, Vector2 position,                          // Draw text using SpriteFont and additional parameters
                    int fontSize, int spacing, Color tint);
    void DrawFPS(int posX, int posY);                                                                   // Shows current FPS on top-left corner
    
    // Text misc. functions
    int MeasureText(const char *text, int fontSize);                                                    // Measure string width for default font
    Vector2 MeasureTextEx(SpriteFont spriteFont, const char *text, int fontSize, int spacing);          // Measure string size for SpriteFont
    const char *FormatText(const char *text, ...);                                                      // Formatting of text with variables to 'embed'
    const char *SubText(const char *text, int position, int length);                                    // Get a piece of a text string

