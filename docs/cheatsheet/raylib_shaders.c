
    // Shader loading/unloading functions
    Shader LoadShader(char *vsFileName, char *fsFileName);                                              // Load a custom shader and bind default locations
    void UnloadShader(Shader shader);                                                                   // Unload a custom shader from memory
    Shader GetDefaultShader(void);                                                                      // Get default shader
    Shader GetStandardShader(void);                                                                     // Get standard shader
    Texture2D GetDefaultTexture(void);                                                                  // Get default texture

    // Shader access functions
    int GetShaderLocation(Shader shader, const char *uniformName);                                      // Get shader uniform location
    void SetShaderValue(Shader shader, int uniformLoc, float *value, int size);                         // Set shader uniform value (float)
    void SetShaderValuei(Shader shader, int uniformLoc, int *value, int size);                          // Set shader uniform value (int)
    void SetShaderValueMatrix(Shader shader, int uniformLoc, Matrix mat);                               // Set shader uniform value (matrix 4x4)
    void SetMatrixProjection(Matrix proj);                                                              // Set a custom projection matrix (replaces internal projection matrix)
    void SetMatrixModelview(Matrix view);                                                               // Set a custom modelview matrix (replaces internal modelview matrix)

    // Shading beegin/end functions
    void BeginShaderMode(Shader shader);                                                                // Begin custom shader drawing
    void EndShaderMode(void);                                                                           // End custom shader drawing (use default shader)
    void BeginBlendMode(int mode);                                                                      // Begin blending mode (alpha, additive, multiplied)
    void EndBlendMode(void);                                                                            // End blending mode (reset to default: alpha blending)
    
    // Light creation/destruction functions
    Light CreateLight(int type, Vector3 position, Color diffuse);                                       // Create a new light, initialize it and add to pool
    void DestroyLight(Light light);                                                                     // Destroy a light and take it out of the list
    
    // VR control functions
    void InitVrDevice(int vrDevice);                                                                    // Init VR device
    void CloseVrDevice(void);                                                                           // Close VR device
    bool IsVrDeviceReady(void);                                                                         // Detect if VR device is ready
    bool IsVrSimulator(void);                                                                           // Detect if VR simulator is running
    void UpdateVrTracking(Camera *camera);                                                              // Update VR tracking (position and orientation) and camera
    void ToggleVrMode(void);                                                                            // Enable/Disable VR experience (device or simulator)
    
