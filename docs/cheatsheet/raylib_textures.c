
    // Image/Texture2D data loading/unloading functions
    Image LoadImage(const char *fileName);                                                              // Load an image into CPU memory (RAM)
    Image LoadImageEx(Color *pixels, int width, int height);                                            // Load image data from Color array data (RGBA - 32bit)
    Image LoadImageRaw(const char *fileName, int width, int height, int format, int headerSize);        // Load image data from RAW file
    Image LoadImageFromRES(const char *rresName, int resId);                                            // Load an image from rRES file (raylib Resource)
    Texture2D LoadTexture(const char *fileName);                                                        // Load an image as texture into GPU memory
    Texture2D LoadTextureEx(void *data, int width, int height, int textureFormat, int mipmapCount);     // Load a texture from raw data into GPU memory
    Texture2D LoadTextureFromRES(const char *rresName, int resId);                                      // Load an image as texture from rRES file (raylib Resource)
    Texture2D LoadTextureFromImage(Image image);                                                        // Load a texture from image data
    RenderTexture2D LoadRenderTexture(int width, int height);                                           // Load a texture to be used for rendering
    void UnloadImage(Image image);                                                                      // Unload image from CPU memory (RAM)
    void UnloadTexture(Texture2D texture);                                                              // Unload texture from GPU memory
    void UnloadRenderTexture(RenderTexture2D target);                                                   // Unload render texture from GPU memory
    Color *GetImageData(Image image);                                                                   // Get pixel data from image as a Color struct array
    Image GetTextureData(Texture2D texture);                                                            // Get pixel data from GPU texture and return an Image
    void UpdateTexture(Texture2D texture, void *pixels);                                                // Update GPU texture with new data
    
    // Image manipulation functions
    void ImageToPOT(Image *image, Color fillColor);                                                     // Convert image to POT (power-of-two)
    void ImageFormat(Image *image, int newFormat);                                                      // Convert image data to desired format
    void ImageDither(Image *image, int rBpp, int gBpp, int bBpp, int aBpp);                             // Dither image data to 16bpp or lower (Floyd-Steinberg dithering)
    Image ImageCopy(Image image);                                                                       // Create an image duplicate (useful for transformations)
    void ImageCrop(Image *image, Rectangle crop);                                                       // Crop an image to a defined rectangle
    void ImageResize(Image *image, int newWidth, int newHeight);                                        // Resize and image (bilinear filtering)
    void ImageResizeNN(Image *image,int newWidth,int newHeight);                                        // Resize and image (Nearest-Neighbor scaling algorithm)
    Image ImageText(const char *text, int fontSize, Color color);                                       // Create an image from text (default font)
    Image ImageTextEx(SpriteFont font, const char *text, int fontSize, int spacing, Color tint);        // Create an image from text (custom sprite font)
    void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec);                          // Draw a source image within a destination image
    void ImageDrawText(Image *dst, Vector2 position, const char *text, int fontSize, Color color);      // Draw text (default font) within an image (destination)
    void ImageDrawTextEx(Image *dst, Vector2 position, SpriteFont font, const char *text, 
                         int fontSize, int spacing, Color color);                                       // Draw text (custom sprite font) within image
    void ImageFlipVertical(Image *image);                                                               // Flip image vertically
    void ImageFlipHorizontal(Image *image);                                                             // Flip image horizontally
    void ImageColorTint(Image *image, Color color);                                                     // Modify image color: tint
    void ImageColorInvert(Image *image);                                                                // Modify image color: invert
    void ImageColorGrayscale(Image *image);                                                             // Modify bimage color: grayscale
    void ImageColorContrast(Image *image, float contrast);                                              // Modify image color: contrast (-100 to 100)
    void ImageColorBrightness(Image *image, int brightness);                                            // Modify image color: brightness (-255 to 255)
    
    // Texture2D configuration functions
    void GenTextureMipmaps(Texture2D *texture);                                                         // Generate GPU mipmaps for a texture
    void SetTextureFilter(Texture2D texture, int filterMode);                                           // Set texture scaling filter mode
    void SetTextureWrap(Texture2D texture, int wrapMode);                                               // Set texture wrapping mode

    // Texture2D drawing functions
    void DrawTexture(Texture2D texture, int posX, int posY, Color tint);                                // Draw a Texture2D
    void DrawTextureV(Texture2D texture, Vector2 position, Color tint);                                 // Draw a Texture2D with position defined as Vector2
    void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);   // Draw a Texture2D with extended parameters
    void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint);          // Draw a part of a texture defined by a rectangle
    void DrawTexturePro(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector2 origin,      // Draw a part of a texture defined by a rectangle with 'pro' parameters
                        float rotation, Color tint);

