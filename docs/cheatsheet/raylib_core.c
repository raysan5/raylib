
    // Window-related functions
    void InitWindow(int width, int height, char* title);                    // Initialize Window and Graphics Context (OpenGL)
    void CloseWindow(void);                                                 // Close Window and Terminate Context
    bool WindowShouldClose(void);                                           // Detect if KEY_ESCAPE pressed or Close icon pressed
    bool IsWindowMinimized(void);                                           // Detect if window has been minimized (or lost focus)
    void ToggleFullscreen(void);                                            // Fullscreen toggle (by default F11)
    int GetScreenWidth(void);                                               // Get current screen width
    int GetScreenHeight(void);                                              // Get current screen height
    
    // Cursor-related functions
    void ShowCursor(void);                                                  // Shows cursor
    void HideCursor(void);                                                  // Hides cursor
    bool IsCursorHidden(void);                                              // Returns true if cursor is not visible
    void EnableCursor(void);                                                // Enables cursor
    void DisableCursor(void);                                               // Disables cursor

    // Drawing-related functions
    void ClearBackground(Color color);                                      // Sets Background Color
    void BeginDrawing(void);                                                // Setup drawing canvas to start drawing
    void EndDrawing(void);                                                  // End canvas drawing and Swap Buffers (Double Buffering)

    void Begin2dMode(Camera2D camera);                                      // Initialize 2D mode with custom camera
    void End2dMode(void);                                                   // Ends 2D mode custom camera usage
    void Begin3dMode(Camera camera);                                        // Initializes 3D mode for drawing (Camera setup)
    void End3dMode(void);                                                   // Ends 3D mode and returns to default 2D orthographic mode
    void BeginTextureMode(RenderTexture2D target);                          // Initializes render texture for drawing
    void EndTextureMode(void);                                              // Ends drawing to render texture
    
    Ray GetMouseRay(Vector2 mousePosition, Camera camera);                  // Returns a ray trace from mouse position
    Vector2 GetWorldToScreen(Vector3 position, Camera camera);              // Returns the screen space position from a 3d world space position
    Matrix GetCameraMatrix(Camera camera);                                  // Returns camera transform matrix (view matrix)

    // Timming-related functions
    void SetTargetFPS(int fps);                                             // Set target FPS (maximum)
    float GetFPS(void);                                                     // Returns current FPS
    float GetFrameTime(void);                                               // Returns time in seconds for one frame

    // Color-related functions
    Color GetColor(int hexValue);                                           // Returns a Color struct from hexadecimal value
    int GetHexValue(Color color);                                           // Returns hexadecimal value for a Color
    float *ColorToFloat(Color color);                                       // Converts Color to float array and normalizes
    float *VectorToFloat(Vector3 vec);                                      // Converts Vector3 to float array
    float *MatrixToFloat(Matrix mat);                                       // Converts Matrix to float array

    // Misc. functions
    int GetRandomValue(int min, int max);                                   // Returns a random value between min and max (both included)
    Color Fade(Color color, float alpha);                                   // Color fade-in or fade-out, alpha goes from 0.0 to 1.0
    void SetConfigFlags(char flags);                                        // Setup some window configuration flags
    void ShowLogo(void);                                                    // Activates raylib logo at startup (can be done with flags)

    // Drag-and-drop files functions
    bool IsFileDropped(void);                                               // Check if a file have been dropped into window
    char **GetDroppedFiles(int *count);                                     // Retrieve dropped files into window
    void ClearDroppedFiles(void);                                           // Clear dropped files paths buffer

    // Persistent storage management
    void StorageSaveValue(int position, int value);                         // Storage save integer value (to defined position)
    int StorageLoadValue(int position);                                     // Storage load integer value (from defined position)
    
    // Input-related functions: keyboard
    bool IsKeyPressed(int key);                                             // Detect if a key has been pressed once
    bool IsKeyDown(int key);                                                // Detect if a key is being pressed
    bool IsKeyReleased(int key);                                            // Detect if a key has been released once
    bool IsKeyUp(int key);                                                  // Detect if a key is NOT being pressed
    int GetKeyPressed(void);                                                // Get latest key pressed
    void SetExitKey(int key);                                               // Set a custom key to exit program (default is ESC)

    // Input-related functions: gamepads
    bool IsGamepadAvailable(int gamepad);                                   // Detect if a gamepad is available
    bool IsGamepadName(int gamepad, const char *name);                      // Check gamepad name (if available)
    const char *GetGamepadName(int gamepad);                                // Return gamepad internal name id
    bool IsGamepadButtonPressed(int gamepad, int button);                   // Detect if a gamepad button has been pressed once
    bool IsGamepadButtonDown(int gamepad, int button);                      // Detect if a gamepad button is being pressed
    bool IsGamepadButtonReleased(int gamepad, int button);                  // Detect if a gamepad button has been released once
    bool IsGamepadButtonUp(int gamepad, int button);                        // Detect if a gamepad button is NOT being pressed
    int GetGamepadButtonPressed(void);                                      // Get the last gamepad button pressed
    int GetGamepadAxisCount(int gamepad);                                   // Return gamepad axis count for a gamepad
    float GetGamepadAxisMovement(int gamepad, int axis);                    // Return axis movement value for a gamepad axis

    // Input-related functions: mouse
    bool IsMouseButtonPressed(int button);                                  // Detect if a mouse button has been pressed once
    bool IsMouseButtonDown(int button);                                     // Detect if a mouse button is being pressed
    bool IsMouseButtonReleased(int button);                                 // Detect if a mouse button has been released once
    bool IsMouseButtonUp(int button);                                       // Detect if a mouse button is NOT being pressed
    int GetMouseX(void);                                                    // Returns mouse position X
    int GetMouseY(void);                                                    // Returns mouse position Y
    Vector2 GetMousePosition(void);                                         // Returns mouse position XY
    void SetMousePosition(Vector2 position);                                // Set mouse position XY
    int GetMouseWheelMove(void);                                            // Returns mouse wheel movement Y

    // Input-related functions: touch
    int GetTouchX(void);                                                    // Get touch position X for touch point 0 (relative to screen size)
    int GetTouchY(void);                                                    // Get touch position Y for touch point 0 (relative to screen size)
    Vector2 GetTouchPosition(int index);                                    // Get touch position XY for a touch point index (relative to screen size)

    // Gestures-related functions
    void SetGesturesEnabled(unsigned int gestureFlags);                     // Enable a set of gestures using flags
    bool IsGestureDetected(int gesture);                                    // Check if a gesture have been detected
    int GetGestureDetected(void);                                           // Get latest detected gesture
    int GetTouchPointsCount(void);                                          // Get touch points count
    float GetGestureHoldDuration(void);                                     // Get gesture hold time in milliseconds
    Vector2 GetGestureDragVector(void);                                     // Get gesture drag vector
    float GetGestureDragAngle(void);                                        // Get gesture drag angle
    Vector2 GetGesturePinchVector(void);                                    // Get gesture pinch delta
    float GetGesturePinchAngle(void);                                       // Get gesture pinch angle
    
    // Camera-related functions
    SetCameraMode(Camera camera, int mode);                                 // Set camera mode (multiple camera modes available)
    void UpdateCamera(Camera *camera);                                      // Update camera position for selected mode
    void SetCameraPanControl(int panKey);                                   // Set camera pan key to combine with mouse movement (free camera)
    void SetCameraAltControl(int altKey);                                   // Set camera alt key to combine with mouse movement (free camera)
    void SetCameraSmoothZoomControl(int szKey);                             // Set camera smooth zoom key to combine with mouse (free camera)
    void SetCameraMoveControls(int frontKey, int backKey,
                               int rightKey, int leftKey,
                               int upKey, int downKey);                     // Set camera move controls (1st person and 3rd person cameras)

                               