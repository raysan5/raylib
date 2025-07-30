#include <raylib.h>
#include <math.h>

int main() {
  // Configuration
  const int screenWidth = 800;
  const int screenHeight = 450;
  const char* text = "Raylib in Space!";
  const int fontSize = 40;
  const Color textColor = WHITE;

  // Initialisation de Raylib
  InitWindow(screenWidth, screenHeight, "Levitating Text");
  SetTargetFPS(60);

  // Position initiale du texte (au centre de l'écran)
  Vector2 textPosition = { (float)screenWidth / 2 - MeasureText(text, fontSize) / 2, (float)screenHeight / 2 };

  // Variables pour l'animation
  float time = 0.0f;  // Temps écoulé pour le mouvement sinusoïdal
  float amplitude = 30.0f; // Amplitude du mouvement vertical
  float frequency = 1.5f;  // Fréquence du mouvement vertical

  // Boucle principale du jeu
  while (!WindowShouldClose()) {
    // Mise à jour

    time += GetFrameTime(); // Incrémente le temps écoulé

    // Calcule la position verticale basée sur une fonction sinusoïdale
    textPosition.y = (float)screenHeight / 2 + sin(time * frequency) * amplitude;

    // Dessin
    BeginDrawing();

      ClearBackground(BLACK);

      // Dessine le texte à la position mise à jour
      DrawText(text, (int)textPosition.x, (int)textPosition.y, fontSize, textColor);

      // Optionnel: Ajoute des étoiles en arrière-plan
      for (int i = 0; i < 100; ++i) {
          Vector2 starPosition = {(float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight)};
          float starSize = (float)GetRandomValue(1, 3);
          Color starColor = Fade(WHITE, (float)GetRandomValue(20, 80) / 100.0f); // Étoiles plus ou moins brillantes
          DrawPixelV(starPosition, starColor);
      }

    EndDrawing();
  }

  // Libération de la mémoire et fermeture de Raylib
  CloseWindow();

  return 0;
}
