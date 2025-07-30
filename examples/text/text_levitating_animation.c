#include <raylib.h>
#include <math.h>

int main() {
    // Configuration
    const int screenWidth = 800;
    const int screenHeight = 450;
    const char* text = "Raylib in Space!";
    const int fontSize = 40;
    const Color textColor = WHITE;

    // Initialize Raylib
    InitWindow(screenWidth, screenHeight, "Levitating Text");
    SetTargetFPS(60);

    // Initial position of the text (centered on the screen)
    Vector2 textPosition = { 
        (float)screenWidth / 2 - MeasureText(text, fontSize) / 2, 
        (float)screenHeight / 2 
    };

    // Animation variables
    float time = 0.0f;       // Elapsed time for sinusoidal movement
    float amplitude = 30.0f; // Amplitude of vertical movement
    float frequency = 1.5f;  // Frequency of vertical movement

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        time += GetFrameTime(); // Increment elapsed time

        // Compute vertical position based on a sine function
        textPosition.y = (float)screenHeight / 2 + sinf(time * frequency) * amplitude;

        // Draw
        BeginDrawing();

            ClearBackground(BLACK);

            // Draw the text at the updated position
            DrawText(text, (int)textPosition.x, (int)textPosition.y, fontSize, textColor);

            // Optional: Add stars in the background
            for (int i = 0; i < 100; ++i) {
                Vector2 starPosition = {
                    (float)GetRandomValue(0, screenWidth), 
                    (float)GetRandomValue(0, screenHeight)
                };
                float starSize = (float)GetRandomValue(1, 3);
                Color starColor = Fade(WHITE, (float)GetRandomValue(20, 80) / 100.0f);

                DrawRectangleV(starPosition, (Vector2){starSize, starSize}, starColor); // Use starSize
            }

        EndDrawing();
    }

    // Clean up and close Raylib
    CloseWindow();

    return 0;
}
