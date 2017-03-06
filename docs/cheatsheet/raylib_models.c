
    // Basic geometric 3D shapes drawing functions
    void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color);                                     // Draw a line in 3D world space
    void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, 
                      float rotationAngle, Color color);                                                // Draw a circle in 3D world space
    void DrawCube(Vector3 position, float width, float height, float lenght, Color color);              // Draw cube
    void DrawCubeV(Vector3 position, Vector3 size, Color color);                                        // Draw cube (Vector version)
    void DrawCubeWires(Vector3 position, float width, float height, float lenght, Color color);         // Draw cube wires
    void DrawCubeTexture(Texture2D texture, Vector3 position, float width, 
                         float height, float lenght, Color color);                                      // Draw cube textured
    void DrawSphere(Vector3 centerPos, float radius, Color color);                                      // Draw sphere
    void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color);             // Draw sphere with extended parameters
    void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color);          // Draw sphere wires
    void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, 
                      float height, int slices, Color color);                                           // Draw a cylinder/cone
    void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, 
                           float height, int slices, Color color);                                      // Draw a cylinder/cone wires
    void DrawPlane(Vector3 centerPos, Vector2 size, Color color);                                       // Draw a plane XZ
    void DrawRay(Ray ray, Color color);                                                                 // Draw a ray line
    void DrawGrid(int slices, float spacing);                                                           // Draw a grid (centered at (0, 0, 0))
    void DrawGizmo(Vector3 position);                                                                   // Draw simple gizmo
    void DrawLight(Light light);                                                                        // Draw light in 3D world

    // Model loading/unloading functions
    Model LoadModel(const char *fileName);                                                              // Load a 3d model (.OBJ)
    Model LoadModelEx(Mesh data, bool dynamic);                                                         // Load a 3d model (from mesh data)
    Model LoadModelFromRES(const char *rresName, int resId);                                            // Load a 3d model from rRES file (raylib Resource)
    Model LoadHeightmap(Image heightmap, Vector3 size);                                                 // Load a heightmap image as a 3d model
    Model LoadCubicmap(Image cubicmap);                                                                 // Load a map image as a 3d model (cubes based)
    void UnloadModel(Model model);                                                                      // Unload 3d model from memory

    // Material loading/unloading functions
    Material LoadMaterial(const char *fileName);                                                        // Load material data (from file)
    Material LoadDefaultMaterial(void);                                                                 // Load default material (uses default models shader)
    Material LoadStandardMaterial(void);                                                                // Load standard material (uses material attributes and lighting shader)
    void UnloadMaterial(Material material);                                                             // Unload material textures from VRAM

    // Model drawing functions
    void DrawModel(Model model, Vector3 position, float scale, Color tint);                             // Draw a model (with texture if set)
    void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, 
                     float rotationAngle, Vector3 scale, Color tint);                                   // Draw a model with extended parameters
    void DrawModelWires(Model model, Vector3 position, float scale, Color tint);                        // Draw a model wires (with texture if set)
    void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, 
                          float rotationAngle, Vector3 scale, Color tint);                              // Draw a model wires
    void DrawBoundingBox(BoundingBox box, Color color);                                                 // Draw bounding box (wires)
    void DrawBillboard(Camera camera, Texture2D texture, Vector3 center, float size, Color tint);       // Draw a billboard texture
    void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle sourceRec, 
                          Vector3 center, float size, Color tint);                                      // Draw a billboard texture defined by sourceRec

    // Collision detection functions
    bool CheckCollisionSpheres(Vector3 centerA, float radiusA, Vector3 centerB, float radiusB);                     // Detect collision between two spheres
    bool CheckCollisionBoxes(Vector3 minBBox1, Vector3 maxBBox1, Vector3 minBBox2, Vector3 maxBBox2);               // Detect collision between two boxes
    bool CheckCollisionBoxSphere(Vector3 minBBox, Vector3 maxBBox, Vector3 centerSphere, float radiusSphere);       // Detect collision between box and sphere
    bool CheckCollisionRaySphere(Ray ray, Vector3 spherePosition, float sphereRadius);                              // Detect collision between ray and sphere
    bool CheckCollisionRaySphereEx(Ray ray, Vector3 spherePosition, float sphereRadius, Vector3 *collisionPoint);   // Detect collision between ray and sphere ex.
    bool CheckCollisionRayBox(Ray ray, Vector3 minBBox, Vector3 maxBBox);                                           // Detect collision between ray and box
    BoundingBox CalculateBoundingBox(Mesh mesh);                                                                    // Calculate mesh bounding box limits
    
