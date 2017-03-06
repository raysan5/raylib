
    // Basic shapes drawing functions
    void DrawPixel(int posX, int posY, Color color);                                                    // Draw a pixel
    void DrawPixelV(Vector2 position, Color color);                                                     // Draw a pixel (Vector version)
    void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);                 // Draw a line
    void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);                                      // Draw a line (Vector version)
    void DrawCircle(int centerX, int centerY, float radius, Color color);                               // Draw a color-filled circle
    void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2);        // Draw a gradient-filled circle
    void DrawCircleV(Vector2 center, float radius, Color color);                                        // Draw a color-filled circle (Vector version)
    void DrawCircleLines(int centerX, int centerY, float radius, Color color);                          // Draw circle outline
    void DrawRectangle(int posX, int posY, int width, int height, Color color);                         // Draw a color-filled rectangle
    void DrawRectangleRec(Rectangle rec, Color color);                                                  // Draw a color-filled rectangle
    void DrawRectangleGradient(int posX, int posY, int width, int height, Color color1, Color color2);  // Draw a gradient-filled rectangle
    void DrawRectangleV(Vector2 position, Vector2 size, Color color);                                   // Draw a color-filled rectangle (Vector version)
    void DrawRectangleLines(int posX, int posY, int width, int height, Color color);                    // Draw rectangle outline
    void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);                                 // Draw a color-filled triangle
    void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);                            // Draw triangle outline
    void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color);                // Draw a regular polygon (Vector version)
    void DrawPolyEx(Vector2 *points, int numPoints, Color color);                                       // Draw a closed polygon defined by points
    void DrawPolyExLines(Vector2 *points, int numPoints, Color color);                                  // Draw polygon lines

    // Basic shapes collision detection functions
    bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);                                            // Check collision between two rectangles
    bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);         // Check collision between two circles
    bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);                          // Check collision between circle and rectangle
    Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2);                                          // Get collision rectangle for two rectangles collision
    bool CheckCollisionPointRec(Vector2 point, Rectangle rec);                                          // Check if point is inside rectangle
    bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius);                        // Check if point is inside circle
    bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3);                // Check if point is inside a triangle

