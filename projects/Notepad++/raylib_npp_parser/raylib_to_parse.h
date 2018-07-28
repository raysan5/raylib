//------------------------------------------------------------------------------------
// Window and Graphics Device Functions (Module: core)
//------------------------------------------------------------------------------------

// Window-related functions
RLAPI void InitWindow(int width, int height, const char *title);  // Initialize window and OpenGL context
RLAPI void CloseWindow(void);                                     // Close window and unload OpenGL context
RLAPI bool IsWindowReady(void);                                   // Check if window has been initialized successfully
RLAPI bool WindowShouldClose(void);                               // Check if KEY_ESCAPE pressed or Close icon pressed
RLAPI bool IsWindowMinimized(void);                               // Check if window has been minimized (or lost focus)
RLAPI void ToggleFullscreen(void);                                // Toggle fullscreen mode (only PLATFORM_DESKTOP)
RLAPI void SetWindowIcon(Image image);                            // Set icon for window (only PLATFORM_DESKTOP)
RLAPI void SetWindowTitle(const char *title);                     // Set title for window (only PLATFORM_DESKTOP)
RLAPI void SetWindowPosition(int x, int y);                       // Set window position on screen (only PLATFORM_DESKTOP)
RLAPI void SetWindowMonitor(int monitor);                         // Set monitor for the current window (fullscreen mode)
RLAPI void SetWindowMinSize(int width, int height);               // Set window minimum dimensions (for FLAG_WINDOW_RESIZABLE)
RLAPI void SetWindowSize(int width, int height);                  // Set window dimensions
RLAPI int GetScreenWidth(void);                                   // Get current screen width
RLAPI int GetScreenHeight(void);                                  // Get current screen height

// Cursor-related functions
RLAPI void ShowCursor(void);                                      // Shows cursor
RLAPI void HideCursor(void);                                      // Hides cursor
RLAPI bool IsCursorHidden(void);                                  // Check if cursor is not visible
RLAPI void EnableCursor(void);                                    // Enables cursor (unlock cursor)
RLAPI void DisableCursor(void);                                   // Disables cursor (lock cursor)

// Drawing-related functions
RLAPI void ClearBackground(Color color);                          // Set background color (framebuffer clear color)
RLAPI void BeginDrawing(void);                                    // Setup canvas (framebuffer) to start drawing
RLAPI void EndDrawing(void);                                      // End canvas drawing and swap buffers (double buffering)
RLAPI void BeginMode2D(Camera2D camera);                          // Initialize 2D mode with custom camera (2D)
RLAPI void EndMode2D(void);                                       // Ends 2D mode with custom camera
RLAPI void BeginMode3D(Camera3D camera);                          // Initializes 3D mode with custom camera (3D)
RLAPI void EndMode3D(void);                                       // Ends 3D mode and returns to default 2D orthographic mode
RLAPI void BeginTextureMode(RenderTexture2D target);              // Initializes render texture for drawing
RLAPI void EndTextureMode(void);                                  // Ends drawing to render texture

// Screen-space-related functions
RLAPI Ray GetMouseRay(Vector2 mousePosition, Camera camera);      // Returns a ray trace from mouse position
RLAPI Vector2 GetWorldToScreen(Vector3 position, Camera camera);  // Returns the screen space position for a 3d world space position
RLAPI Matrix GetCameraMatrix(Camera camera);                      // Returns camera transform matrix (view matrix)

// Timing-related functions
RLAPI void SetTargetFPS(int fps);                                 // Set target FPS (maximum)
RLAPI int GetFPS(void);                                           // Returns current FPS
RLAPI float GetFrameTime(void);                                   // Returns time in seconds for last frame drawn
RLAPI double GetTime(void);                                       // Returns elapsed time in seconds since InitWindow()

// Color-related functions
RLAPI int ColorToInt(Color color);                                // Returns hexadecimal value for a Color
RLAPI Vector4 ColorNormalize(Color color);                        // Returns color normalized as float [0..1]
RLAPI Vector3 ColorToHSV(Color color);                            // Returns HSV values for a Color
RLAPI Color GetColor(int hexValue);                               // Returns a Color struct from hexadecimal value
RLAPI Color Fade(Color color, float alpha);                       // Color fade-in or fade-out, alpha goes from 0.0f to 1.0f

// Misc. functions
RLAPI void ShowLogo(void);                                        // Activate raylib logo at startup (can be done with flags)
RLAPI void SetConfigFlags(unsigned char flags);                   // Setup window configuration flags (view FLAGS)
RLAPI void SetTraceLog(unsigned char types);                      // Enable trace log message types (bit flags based)
RLAPI void TraceLog(int logType, const char *text, ...);          // Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
RLAPI void TakeScreenshot(const char *fileName);                  // Takes a screenshot of current screen (saved a .png)
RLAPI int GetRandomValue(int min, int max);                       // Returns a random value between min and max (both included)

// Files management functions
RLAPI bool IsFileExtension(const char *fileName, const char *ext);// Check file extension
RLAPI const char *GetExtension(const char *fileName);             // Get pointer to extension for a filename string
RLAPI const char *GetFileName(const char *filePath);              // Get pointer to filename for a path string
RLAPI const char *GetDirectoryPath(const char *fileName);         // Get full path for a given fileName (uses static string)
RLAPI const char *GetWorkingDirectory(void);                      // Get current working directory (uses static string)
RLAPI bool ChangeDirectory(const char *dir);                      // Change working directory, returns true if success
RLAPI bool IsFileDropped(void);                                   // Check if a file has been dropped into window
RLAPI char **GetDroppedFiles(int *count);                         // Get dropped files names
RLAPI void ClearDroppedFiles(void);                               // Clear dropped files paths buffer

// Persistent storage management
RLAPI void StorageSaveValue(int position, int value);             // Save integer value to storage file (to defined position)
RLAPI int StorageLoadValue(int position);                         // Load integer value from storage file (from defined position)

//------------------------------------------------------------------------------------
// Input Handling Functions (Module: core)
//------------------------------------------------------------------------------------

// Input-related functions: keyboard
RLAPI bool IsKeyPressed(int key);                             // Detect if a key has been pressed once
RLAPI bool IsKeyDown(int key);                                // Detect if a key is being pressed
RLAPI bool IsKeyReleased(int key);                            // Detect if a key has been released once
RLAPI bool IsKeyUp(int key);                                  // Detect if a key is NOT being pressed
RLAPI int GetKeyPressed(void);                                // Get latest key pressed
RLAPI void SetExitKey(int key);                               // Set a custom key to exit program (default is ESC)

// Input-related functions: gamepads
RLAPI bool IsGamepadAvailable(int gamepad);                   // Detect if a gamepad is available
RLAPI bool IsGamepadName(int gamepad, const char *name);      // Check gamepad name (if available)
RLAPI const char *GetGamepadName(int gamepad);                // Return gamepad internal name id
RLAPI bool IsGamepadButtonPressed(int gamepad, int button);   // Detect if a gamepad button has been pressed once
RLAPI bool IsGamepadButtonDown(int gamepad, int button);      // Detect if a gamepad button is being pressed
RLAPI bool IsGamepadButtonReleased(int gamepad, int button);  // Detect if a gamepad button has been released once
RLAPI bool IsGamepadButtonUp(int gamepad, int button);        // Detect if a gamepad button is NOT being pressed
RLAPI int GetGamepadButtonPressed(void);                      // Get the last gamepad button pressed
RLAPI int GetGamepadAxisCount(int gamepad);                   // Return gamepad axis count for a gamepad
RLAPI float GetGamepadAxisMovement(int gamepad, int axis);    // Return axis movement value for a gamepad axis

// Input-related functions: mouse
RLAPI bool IsMouseButtonPressed(int button);                  // Detect if a mouse button has been pressed once
RLAPI bool IsMouseButtonDown(int button);                     // Detect if a mouse button is being pressed
RLAPI bool IsMouseButtonReleased(int button);                 // Detect if a mouse button has been released once
RLAPI bool IsMouseButtonUp(int button);                       // Detect if a mouse button is NOT being pressed
RLAPI int GetMouseX(void);                                    // Returns mouse position X
RLAPI int GetMouseY(void);                                    // Returns mouse position Y
RLAPI Vector2 GetMousePosition(void);                         // Returns mouse position XY
RLAPI void SetMousePosition(Vector2 position);                // Set mouse position XY
RLAPI void SetMouseScale(float scale);                        // Set mouse scaling
RLAPI int GetMouseWheelMove(void);                            // Returns mouse wheel movement Y

// Input-related functions: touch
RLAPI int GetTouchX(void);                                    // Returns touch position X for touch point 0 (relative to screen size)
RLAPI int GetTouchY(void);                                    // Returns touch position Y for touch point 0 (relative to screen size)
RLAPI Vector2 GetTouchPosition(int index);                    // Returns touch position XY for a touch point index (relative to screen size)

//------------------------------------------------------------------------------------
// Gestures and Touch Handling Functions (Module: gestures)
//------------------------------------------------------------------------------------
RLAPI void SetGesturesEnabled(unsigned int gestureFlags);     // Enable a set of gestures using flags
RLAPI bool IsGestureDetected(int gesture);                    // Check if a gesture have been detected
RLAPI int GetGestureDetected(void);                           // Get latest detected gesture
RLAPI int GetTouchPointsCount(void);                          // Get touch points count
RLAPI float GetGestureHoldDuration(void);                     // Get gesture hold time in milliseconds
RLAPI Vector2 GetGestureDragVector(void);                     // Get gesture drag vector
RLAPI float GetGestureDragAngle(void);                        // Get gesture drag angle
RLAPI Vector2 GetGesturePinchVector(void);                    // Get gesture pinch delta
RLAPI float GetGesturePinchAngle(void);                       // Get gesture pinch angle

//------------------------------------------------------------------------------------
// Camera System Functions (Module: camera)
//------------------------------------------------------------------------------------
RLAPI void SetCameraMode(Camera camera, int mode);                // Set camera mode (multiple camera modes available)
RLAPI void UpdateCamera(Camera *camera);                          // Update camera position for selected mode

RLAPI void SetCameraPanControl(int panKey);                       // Set camera pan key to combine with mouse movement (free camera)
RLAPI void SetCameraAltControl(int altKey);                       // Set camera alt key to combine with mouse movement (free camera)
RLAPI void SetCameraSmoothZoomControl(int szKey);                 // Set camera smooth zoom key to combine with mouse (free camera)
RLAPI void SetCameraMoveControls(int frontKey, int backKey, int rightKey, int leftKey, int upKey, int downKey); // Set camera move controls (1st person and 3rd person cameras)

//------------------------------------------------------------------------------------
// Basic Shapes Drawing Functions (Module: shapes)
//------------------------------------------------------------------------------------

// Basic shapes drawing functions
RLAPI void DrawPixel(int posX, int posY, Color color);                                                   // Draw a pixel
RLAPI void DrawPixelV(Vector2 position, Color color);                                                    // Draw a pixel (Vector version)
RLAPI void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);                // Draw a line
RLAPI void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);                                     // Draw a line (Vector version)
RLAPI void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);                       // Draw a line defining thickness
RLAPI void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color);                   // Draw a line using cubic-bezier curves in-out
RLAPI void DrawCircle(int centerX, int centerY, float radius, Color color);                              // Draw a color-filled circle
RLAPI void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2);       // Draw a gradient-filled circle
RLAPI void DrawCircleV(Vector2 center, float radius, Color color);                                       // Draw a color-filled circle (Vector version)
RLAPI void DrawCircleLines(int centerX, int centerY, float radius, Color color);                         // Draw circle outline
RLAPI void DrawRectangle(int posX, int posY, int width, int height, Color color);                        // Draw a color-filled rectangle
RLAPI void DrawRectangleV(Vector2 position, Vector2 size, Color color);                                  // Draw a color-filled rectangle (Vector version)
RLAPI void DrawRectangleRec(Rectangle rec, Color color);                                                 // Draw a color-filled rectangle
RLAPI void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color);                 // Draw a color-filled rectangle with pro parameters
RLAPI void DrawRectangleGradientV(int posX, int posY, int width, int height, Color color1, Color color2);// Draw a vertical-gradient-filled rectangle
RLAPI void DrawRectangleGradientH(int posX, int posY, int width, int height, Color color1, Color color2);// Draw a horizontal-gradient-filled rectangle
RLAPI void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4);       // Draw a gradient-filled rectangle with custom vertex colors
RLAPI void DrawRectangleLines(int posX, int posY, int width, int height, Color color);                   // Draw rectangle outline
RLAPI void DrawRectangleLinesEx(Rectangle rec, int lineThick, Color color);                              // Draw rectangle outline with extended parameters
RLAPI void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);                                // Draw a color-filled triangle
RLAPI void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);                           // Draw triangle outline
RLAPI void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color);               // Draw a regular polygon (Vector version)
RLAPI void DrawPolyEx(Vector2 *points, int numPoints, Color color);                                      // Draw a closed polygon defined by points
RLAPI void DrawPolyExLines(Vector2 *points, int numPoints, Color color);                                 // Draw polygon lines

// Basic shapes collision detection functions
RLAPI bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);                                           // Check collision between two rectangles
RLAPI bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);        // Check collision between two circles
RLAPI bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);                         // Check collision between circle and rectangle
RLAPI Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2);                                         // Get collision rectangle for two rectangles collision
RLAPI bool CheckCollisionPointRec(Vector2 point, Rectangle rec);                                         // Check if point is inside rectangle
RLAPI bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius);                       // Check if point is inside circle
RLAPI bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3);               // Check if point is inside a triangle

//------------------------------------------------------------------------------------
// Texture Loading and Drawing Functions (Module: textures)
//------------------------------------------------------------------------------------

// Image/Texture2D data loading/unloading/saving functions
RLAPI Image LoadImage(const char *fileName);                                                             // Load image from file into CPU memory (RAM)
RLAPI Image LoadImageEx(Color *pixels, int width, int height);                                           // Load image from Color array data (RGBA - 32bit)
RLAPI Image LoadImagePro(void *data, int width, int height, int format);                                 // Load image from raw data with parameters
RLAPI Image LoadImageRaw(const char *fileName, int width, int height, int format, int headerSize);       // Load image from RAW file data
RLAPI void ExportImage(const char *fileName, Image image);                                               // Export image as a PNG file
RLAPI Texture2D LoadTexture(const char *fileName);                                                       // Load texture from file into GPU memory (VRAM)
RLAPI Texture2D LoadTextureFromImage(Image image);                                                       // Load texture from image data
RLAPI RenderTexture2D LoadRenderTexture(int width, int height);                                          // Load texture for rendering (framebuffer)
RLAPI void UnloadImage(Image image);                                                                     // Unload image from CPU memory (RAM)
RLAPI void UnloadTexture(Texture2D texture);                                                             // Unload texture from GPU memory (VRAM)
RLAPI void UnloadRenderTexture(RenderTexture2D target);                                                  // Unload render texture from GPU memory (VRAM)
RLAPI Color *GetImageData(Image image);                                                                  // Get pixel data from image as a Color struct array
RLAPI Vector4 *GetImageDataNormalized(Image image);                                                      // Get pixel data from image as Vector4 array (float normalized)
RLAPI int GetPixelDataSize(int width, int height, int format);                                           // Get pixel data size in bytes (image or texture)
RLAPI Image GetTextureData(Texture2D texture);                                                           // Get pixel data from GPU texture and return an Image
RLAPI void UpdateTexture(Texture2D texture, const void *pixels);                                         // Update GPU texture with new data

// Image manipulation functions
RLAPI Image ImageCopy(Image image);                                                                      // Create an image duplicate (useful for transformations)
RLAPI void ImageToPOT(Image *image, Color fillColor);                                                    // Convert image to POT (power-of-two)
RLAPI void ImageFormat(Image *image, int newFormat);                                                     // Convert image data to desired format
RLAPI void ImageAlphaMask(Image *image, Image alphaMask);                                                // Apply alpha mask to image
RLAPI void ImageAlphaClear(Image *image, Color color, float threshold);                                  // Clear alpha channel to desired color
RLAPI void ImageAlphaCrop(Image *image, float threshold);                                                // Crop image depending on alpha value
RLAPI void ImageAlphaPremultiply(Image *image);                                                          // Premultiply alpha channel
RLAPI void ImageCrop(Image *image, Rectangle crop);                                                      // Crop an image to a defined rectangle
RLAPI void ImageResize(Image *image, int newWidth, int newHeight);                                       // Resize and image (bilinear filtering)
RLAPI void ImageResizeNN(Image *image,int newWidth,int newHeight);                                       // Resize and image (Nearest-Neighbor scaling algorithm)
RLAPI void ImageResizeCanvas(Image *image, int newWidth, int newHeight, int offsetX, int offsetY, Color color);  // Resize canvas and fill with color
RLAPI void ImageMipmaps(Image *image);                                                                   // Generate all mipmap levels for a provided image
RLAPI void ImageDither(Image *image, int rBpp, int gBpp, int bBpp, int aBpp);                            // Dither image data to 16bpp or lower (Floyd-Steinberg dithering)
RLAPI Image ImageText(const char *text, int fontSize, Color color);                                      // Create an image from text (default font)
RLAPI Image ImageTextEx(Font font, const char *text, float fontSize, float spacing, Color tint);         // Create an image from text (custom sprite font)
RLAPI void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec);                         // Draw a source image within a destination image
RLAPI void ImageDrawRectangle(Image *dst, Vector2 position, Rectangle rec, Color color);                 // Draw rectangle within an image
RLAPI void ImageDrawText(Image *dst, Vector2 position, const char *text, int fontSize, Color color);     // Draw text (default font) within an image (destination)
RLAPI void ImageDrawTextEx(Image *dst, Vector2 position, Font font, const char *text, float fontSize, float spacing, Color color); // Draw text (custom sprite font) within an image (destination)
RLAPI void ImageFlipVertical(Image *image);                                                              // Flip image vertically
RLAPI void ImageFlipHorizontal(Image *image);                                                            // Flip image horizontally
RLAPI void ImageRotateCW(Image *image);                                                                  // Rotate image clockwise 90deg
RLAPI void ImageRotateCCW(Image *image);                                                                 // Rotate image counter-clockwise 90deg
RLAPI void ImageColorTint(Image *image, Color color);                                                    // Modify image color: tint
RLAPI void ImageColorInvert(Image *image);                                                               // Modify image color: invert
RLAPI void ImageColorGrayscale(Image *image);                                                            // Modify image color: grayscale
RLAPI void ImageColorContrast(Image *image, float contrast);                                             // Modify image color: contrast (-100 to 100)
RLAPI void ImageColorBrightness(Image *image, int brightness);                                           // Modify image color: brightness (-255 to 255)
RLAPI void ImageColorReplace(Image *image, Color color, Color replace);                                  // Modify image color: replace color

// Image generation functions
RLAPI Image GenImageColor(int width, int height, Color color);                                           // Generate image: plain color
RLAPI Image GenImageGradientV(int width, int height, Color top, Color bottom);                           // Generate image: vertical gradient
RLAPI Image GenImageGradientH(int width, int height, Color left, Color right);                           // Generate image: horizontal gradient
RLAPI Image GenImageGradientRadial(int width, int height, float density, Color inner, Color outer);      // Generate image: radial gradient
RLAPI Image GenImageChecked(int width, int height, int checksX, int checksY, Color col1, Color col2);    // Generate image: checked
RLAPI Image GenImageWhiteNoise(int width, int height, float factor);                                     // Generate image: white noise
RLAPI Image GenImagePerlinNoise(int width, int height, int offsetX, int offsetY, float scale);           // Generate image: perlin noise
RLAPI Image GenImageCellular(int width, int height, int tileSize);                                       // Generate image: cellular algorithm. Bigger tileSize means bigger cells

// Texture2D configuration functions
RLAPI void GenTextureMipmaps(Texture2D *texture);                                                        // Generate GPU mipmaps for a texture
RLAPI void SetTextureFilter(Texture2D texture, int filterMode);                                          // Set texture scaling filter mode
RLAPI void SetTextureWrap(Texture2D texture, int wrapMode);                                              // Set texture wrapping mode

// Texture2D drawing functions
RLAPI void DrawTexture(Texture2D texture, int posX, int posY, Color tint);                               // Draw a Texture2D
RLAPI void DrawTextureV(Texture2D texture, Vector2 position, Color tint);                                // Draw a Texture2D with position defined as Vector2
RLAPI void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);  // Draw a Texture2D with extended parameters
RLAPI void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint);         // Draw a part of a texture defined by a rectangle
RLAPI void DrawTexturePro(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector2 origin, float rotation, Color tint); // Draw a part of a texture defined by a rectangle with 'pro' parameters
                    

//------------------------------------------------------------------------------------
// Font Loading and Text Drawing Functions (Module: text)
//------------------------------------------------------------------------------------

// Font loading/unloading functions
RLAPI Font GetFontDefault(void);                                                            // Get the default Font
RLAPI Font LoadFont(const char *fileName);                                                  // Load font from file into GPU memory (VRAM)
RLAPI Font LoadFontEx(const char *fileName, int fontSize, int charsCount, int *fontChars);  // Load font from file with extended parameters
RLAPI CharInfo *LoadFontData(const char *fileName, int fontSize, int *fontChars, int charsCount, bool sdf); // Load font data for further use
RLAPI Image GenImageFontAtlas(CharInfo *chars, int fontSize, int charsCount, int padding, int packMethod);  // Generate image font atlas using chars info
RLAPI void UnloadFont(Font font);                                                           // Unload Font from GPU memory (VRAM)

// Text drawing functions
RLAPI void DrawFPS(int posX, int posY);                                                     // Shows current FPS
RLAPI void DrawText(const char *text, int posX, int posY, int fontSize, Color color);       // Draw text (using default font)
RLAPI void DrawTextEx(Font font, const char* text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using Font and additional parameters

// Text misc. functions
RLAPI int MeasureText(const char *text, int fontSize);                                      // Measure string width for default font
RLAPI Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing);    // Measure string size for Font
RLAPI const char *FormatText(const char *text, ...);                                        // Formatting of text with variables to 'embed'
RLAPI const char *SubText(const char *text, int position, int length);                      // Get a piece of a text string
RLAPI int GetGlyphIndex(Font font, int character);                                          // Returns index position for a unicode character on sprite font

//------------------------------------------------------------------------------------
// Basic 3d Shapes Drawing Functions (Module: models)
//------------------------------------------------------------------------------------

// Basic geometric 3D shapes drawing functions
RLAPI void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color);                                    // Draw a line in 3D world space
RLAPI void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color); // Draw a circle in 3D world space
RLAPI void DrawCube(Vector3 position, float width, float height, float length, Color color);             // Draw cube
RLAPI void DrawCubeV(Vector3 position, Vector3 size, Color color);                                       // Draw cube (Vector version)
RLAPI void DrawCubeWires(Vector3 position, float width, float height, float length, Color color);        // Draw cube wires
RLAPI void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color); // Draw cube textured
RLAPI void DrawSphere(Vector3 centerPos, float radius, Color color);                                     // Draw sphere
RLAPI void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color);            // Draw sphere with extended parameters
RLAPI void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color);         // Draw sphere wires
RLAPI void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); // Draw a cylinder/cone
RLAPI void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); // Draw a cylinder/cone wires
RLAPI void DrawPlane(Vector3 centerPos, Vector2 size, Color color);                                      // Draw a plane XZ
RLAPI void DrawRay(Ray ray, Color color);                                                                // Draw a ray line
RLAPI void DrawGrid(int slices, float spacing);                                                          // Draw a grid (centered at (0, 0, 0))
RLAPI void DrawGizmo(Vector3 position);                                                                  // Draw simple gizmo
//DrawTorus(), DrawTeapot() could be useful?

//------------------------------------------------------------------------------------
// Model 3d Loading and Drawing Functions (Module: models)
//------------------------------------------------------------------------------------

// Model loading/unloading functions
RLAPI Model LoadModel(const char *fileName);                                                            // Load model from files (mesh and material)
RLAPI Model LoadModelFromMesh(Mesh mesh);                                                               // Load model from generated mesh
RLAPI void UnloadModel(Model model);                                                                    // Unload model from memory (RAM and/or VRAM)

// Mesh loading/unloading functions
RLAPI Mesh LoadMesh(const char *fileName);                                                              // Load mesh from file
RLAPI void UnloadMesh(Mesh *mesh);                                                                      // Unload mesh from memory (RAM and/or VRAM)
RLAPI void ExportMesh(const char *fileName, Mesh mesh);                                                 // Export mesh as an OBJ file

// Mesh manipulation functions
RLAPI BoundingBox MeshBoundingBox(Mesh mesh);                                                           // Compute mesh bounding box limits
RLAPI void MeshTangents(Mesh *mesh);                                                                    // Compute mesh tangents 
RLAPI void MeshBinormals(Mesh *mesh);                                                                   // Compute mesh binormals

// Mesh generation functions
RLAPI Mesh GenMeshPlane(float width, float length, int resX, int resZ);                                 // Generate plane mesh (with subdivisions)
RLAPI Mesh GenMeshCube(float width, float height, float length);                                        // Generate cuboid mesh
RLAPI Mesh GenMeshSphere(float radius, int rings, int slices);                                          // Generate sphere mesh (standard sphere)
RLAPI Mesh GenMeshHemiSphere(float radius, int rings, int slices);                                      // Generate half-sphere mesh (no bottom cap)
RLAPI Mesh GenMeshCylinder(float radius, float height, int slices);                                     // Generate cylinder mesh
RLAPI Mesh GenMeshTorus(float radius, float size, int radSeg, int sides);                               // Generate torus mesh
RLAPI Mesh GenMeshKnot(float radius, float size, int radSeg, int sides);                                // Generate trefoil knot mesh
RLAPI Mesh GenMeshHeightmap(Image heightmap, Vector3 size);                                             // Generate heightmap mesh from image data
RLAPI Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize);                                           // Generate cubes-based map mesh from image data

// Material loading/unloading functions
RLAPI Material LoadMaterial(const char *fileName);                                                      // Load material from file
RLAPI Material LoadMaterialDefault(void);                                                               // Load default material (Supports: DIFFUSE, SPECULAR, NORMAL maps)
RLAPI void UnloadMaterial(Material material);                                                           // Unload material from GPU memory (VRAM)

// Model drawing functions
RLAPI void DrawModel(Model model, Vector3 position, float scale, Color tint);                           // Draw a model (with texture if set)
RLAPI void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); // Draw a model with extended parameters
RLAPI void DrawModelWires(Model model, Vector3 position, float scale, Color tint);                      // Draw a model wires (with texture if set)
RLAPI void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); // Draw a model wires (with texture if set) with extended parameters
RLAPI void DrawBoundingBox(BoundingBox box, Color color);                                               // Draw bounding box (wires)
RLAPI void DrawBillboard(Camera camera, Texture2D texture, Vector3 center, float size, Color tint);     // Draw a billboard texture
RLAPI void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle sourceRec, Vector3 center, float size, Color tint); // Draw a billboard texture defined by sourceRec

// Collision detection functions
RLAPI bool CheckCollisionSpheres(Vector3 centerA, float radiusA, Vector3 centerB, float radiusB);       // Detect collision between two spheres
RLAPI bool CheckCollisionBoxes(BoundingBox box1, BoundingBox box2);                                     // Detect collision between two bounding boxes
RLAPI bool CheckCollisionBoxSphere(BoundingBox box, Vector3 centerSphere, float radiusSphere);          // Detect collision between box and sphere
RLAPI bool CheckCollisionRaySphere(Ray ray, Vector3 spherePosition, float sphereRadius);                // Detect collision between ray and sphere
RLAPI bool CheckCollisionRaySphereEx(Ray ray, Vector3 spherePosition, float sphereRadius, Vector3 *collisionPoint); // Detect collision between ray and sphere, returns collision point
RLAPI bool CheckCollisionRayBox(Ray ray, BoundingBox box);                                              // Detect collision between ray and box
RLAPI RayHitInfo GetCollisionRayModel(Ray ray, Model *model);                                           // Get collision info between ray and model
RLAPI RayHitInfo GetCollisionRayTriangle(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3);                  // Get collision info between ray and triangle
RLAPI RayHitInfo GetCollisionRayGround(Ray ray, float groundHeight);                                    // Get collision info between ray and ground plane (Y-normal plane)

//------------------------------------------------------------------------------------
// Shaders System Functions (Module: rlgl)
// NOTE: This functions are useless when using OpenGL 1.1
//------------------------------------------------------------------------------------

// Shader loading/unloading functions
RLAPI char *LoadText(const char *fileName);                               // Load chars array from text file
RLAPI Shader LoadShader(const char *vsFileName, const char *fsFileName);  // Load shader from files and bind default locations
RLAPI Shader LoadShaderCode(char *vsCode, char *fsCode);                  // Load shader from code strings and bind default locations
RLAPI void UnloadShader(Shader shader);                                   // Unload shader from GPU memory (VRAM)

RLAPI Shader GetShaderDefault(void);                                      // Get default shader
RLAPI Texture2D GetTextureDefault(void);                                  // Get default texture

// Shader configuration functions
RLAPI int GetShaderLocation(Shader shader, const char *uniformName);              // Get shader uniform location
RLAPI void SetShaderValue(Shader shader, int uniformLoc, const float *value, int size); // Set shader uniform value (float)
RLAPI void SetShaderValuei(Shader shader, int uniformLoc, const int *value, int size);  // Set shader uniform value (int)
RLAPI void SetShaderValueMatrix(Shader shader, int uniformLoc, Matrix mat);       // Set shader uniform value (matrix 4x4)
RLAPI void SetMatrixProjection(Matrix proj);                              // Set a custom projection matrix (replaces internal projection matrix)
RLAPI void SetMatrixModelview(Matrix view);                               // Set a custom modelview matrix (replaces internal modelview matrix)
RLAPI Matrix GetMatrixModelview();                                        // Get internal modelview matrix

// Texture maps generation (PBR)
// NOTE: Required shaders should be provided
RLAPI Texture2D GenTextureCubemap(Shader shader, Texture2D skyHDR, int size);       // Generate cubemap texture from HDR texture
RLAPI Texture2D GenTextureIrradiance(Shader shader, Texture2D cubemap, int size);   // Generate irradiance texture using cubemap data
RLAPI Texture2D GenTexturePrefilter(Shader shader, Texture2D cubemap, int size);    // Generate prefilter texture using cubemap data
RLAPI Texture2D GenTextureBRDF(Shader shader, Texture2D cubemap, int size);         // Generate BRDF texture using cubemap data

// Shading begin/end functions
RLAPI void BeginShaderMode(Shader shader);                                // Begin custom shader drawing
RLAPI void EndShaderMode(void);                                           // End custom shader drawing (use default shader)
RLAPI void BeginBlendMode(int mode);                                      // Begin blending mode (alpha, additive, multiplied)
RLAPI void EndBlendMode(void);                                            // End blending mode (reset to default: alpha blending)

// VR control functions
RLAPI VrDeviceInfo GetVrDeviceInfo(int vrDeviceType);   // Get VR device information for some standard devices
RLAPI void InitVrSimulator(VrDeviceInfo info);          // Init VR simulator for selected device parameters
RLAPI void CloseVrSimulator(void);                      // Close VR simulator for current device
RLAPI bool IsVrSimulatorReady(void);                    // Detect if VR simulator is ready
RLAPI void SetVrDistortionShader(Shader shader);        // Set VR distortion shader for stereoscopic rendering
RLAPI void UpdateVrTracking(Camera *camera);            // Update VR tracking (position and orientation) and camera
RLAPI void ToggleVrMode(void);                          // Enable/Disable VR experience
RLAPI void BeginVrDrawing(void);                        // Begin VR simulator stereo rendering
RLAPI void EndVrDrawing(void);                          // End VR simulator stereo rendering

//------------------------------------------------------------------------------------
// Audio Loading and Playing Functions (Module: audio)
//------------------------------------------------------------------------------------

// Audio device management functions
RLAPI void InitAudioDevice(void);                                     // Initialize audio device and context
RLAPI void CloseAudioDevice(void);                                    // Close the audio device and context
RLAPI bool IsAudioDeviceReady(void);                                  // Check if audio device has been initialized successfully
RLAPI void SetMasterVolume(float volume);                             // Set master volume (listener)

// Wave/Sound loading/unloading functions
RLAPI Wave LoadWave(const char *fileName);                            // Load wave data from file
RLAPI Wave LoadWaveEx(void *data, int sampleCount, int sampleRate, int sampleSize, int channels); // Load wave data from raw array data
RLAPI Sound LoadSound(const char *fileName);                          // Load sound from file
RLAPI Sound LoadSoundFromWave(Wave wave);                             // Load sound from wave data
RLAPI void UpdateSound(Sound sound, const void *data, int samplesCount);// Update sound buffer with new data
RLAPI void UnloadWave(Wave wave);                                     // Unload wave data
RLAPI void UnloadSound(Sound sound);                                  // Unload sound

// Wave/Sound management functions
RLAPI void PlaySound(Sound sound);                                    // Play a sound
RLAPI void PauseSound(Sound sound);                                   // Pause a sound
RLAPI void ResumeSound(Sound sound);                                  // Resume a paused sound
RLAPI void StopSound(Sound sound);                                    // Stop playing a sound
RLAPI bool IsSoundPlaying(Sound sound);                               // Check if a sound is currently playing
RLAPI void SetSoundVolume(Sound sound, float volume);                 // Set volume for a sound (1.0 is max level)
RLAPI void SetSoundPitch(Sound sound, float pitch);                   // Set pitch for a sound (1.0 is base level)
RLAPI void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels);  // Convert wave data to desired format
RLAPI Wave WaveCopy(Wave wave);                                       // Copy a wave to a new wave
RLAPI void WaveCrop(Wave *wave, int initSample, int finalSample);     // Crop a wave to defined samples range
RLAPI float *GetWaveData(Wave wave);                                  // Get samples data from wave as a floats array

// Music management functions
RLAPI Music LoadMusicStream(const char *fileName);                    // Load music stream from file
RLAPI void UnloadMusicStream(Music music);                            // Unload music stream
RLAPI void PlayMusicStream(Music music);                              // Start music playing
RLAPI void UpdateMusicStream(Music music);                            // Updates buffers for music streaming
RLAPI void StopMusicStream(Music music);                              // Stop music playing
RLAPI void PauseMusicStream(Music music);                             // Pause music playing
RLAPI void ResumeMusicStream(Music music);                            // Resume playing paused music
RLAPI bool IsMusicPlaying(Music music);                               // Check if music is playing
RLAPI void SetMusicVolume(Music music, float volume);                 // Set volume for music (1.0 is max level)
RLAPI void SetMusicPitch(Music music, float pitch);                   // Set pitch for a music (1.0 is base level)
RLAPI void SetMusicLoopCount(Music music, int count);                 // Set music loop count (loop repeats)
RLAPI float GetMusicTimeLength(Music music);                          // Get music time length (in seconds)
RLAPI float GetMusicTimePlayed(Music music);                          // Get current music time played (in seconds)

// AudioStream management functions
RLAPI AudioStream InitAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels); // Init audio stream (to stream raw audio pcm data)
RLAPI void UpdateAudioStream(AudioStream stream, const void *data, int samplesCount); // Update audio stream buffers with data
RLAPI void CloseAudioStream(AudioStream stream);                      // Close audio stream and free memory
RLAPI bool IsAudioBufferProcessed(AudioStream stream);                // Check if any audio stream buffers requires refill
RLAPI void PlayAudioStream(AudioStream stream);                       // Play audio stream
RLAPI void PauseAudioStream(AudioStream stream);                      // Pause audio stream
RLAPI void ResumeAudioStream(AudioStream stream);                     // Resume audio stream
RLAPI bool IsAudioStreamPlaying(AudioStream stream);                  // Check if audio stream is playing
RLAPI void StopAudioStream(AudioStream stream);                       // Stop audio stream
RLAPI void SetAudioStreamVolume(AudioStream stream, float volume);    // Set volume for audio stream (1.0 is max level)
RLAPI void SetAudioStreamPitch(AudioStream stream, float pitch);      // Set pitch for audio stream (1.0 is base level)