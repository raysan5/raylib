/*******************************************************************************************
*
*   raylib [core] example - Generates a random sequence
*
*   Example originally created with raylib 5.0, last time updated with raylib 5.0
*
*   Example contributed by Dalton Overmyer (@REDl3east) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023 Dalton Overmyer (@REDl3east)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#include <stdlib.h> // Required for: malloc() and free()

typedef struct ColorRect{
  Color c;
  Rectangle r;
} ColorRect;

static Color GenerateRandomColor();
static ColorRect* GenerateRandomColorRectSequence(float rectCount, float rectWidth, float screenWidth, float screenHeight);
static void ShuffleColorRectSequence(ColorRect* rectangles, int rectCount);
static void DrawTextCenterKeyHelp(const char* key, const char* text, int posX, int posY, int fontSize, Color color);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "raylib [core] example - Generates a random sequence");

  int rectCount = 20;
  float rectSize = (float)screenWidth/rectCount;
  ColorRect* rectangles = GenerateRandomColorRectSequence(rectCount, rectSize, screenWidth, 0.75f * screenHeight);

  SetTargetFPS(60);
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Update
    //----------------------------------------------------------------------------------

    if(IsKeyPressed(KEY_SPACE))
    {
      ShuffleColorRectSequence(rectangles, rectCount);
    }

    if(IsKeyPressed(KEY_UP))
    {
      rectCount++;
      rectSize = (float)screenWidth/rectCount;
      free(rectangles);
      rectangles = GenerateRandomColorRectSequence(rectCount, rectSize, screenWidth, 0.75f * screenHeight);
    }

    if(IsKeyPressed(KEY_DOWN))
    {
      if(rectCount >= 4){
        rectCount--;
        rectSize = (float)screenWidth/rectCount;
        free(rectangles);
        rectangles = GenerateRandomColorRectSequence(rectCount, rectSize, screenWidth, 0.75f * screenHeight);
      }
    }

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);
    
    int fontSize = 20;
    for(int x=0;x<rectCount;x++)
    {
      DrawRectangleRec(rectangles[x].r, rectangles[x].c);
      DrawTextCenterKeyHelp("SPACE", "to shuffle the sequence.", 10, screenHeight - 96, fontSize, BLACK);
      DrawTextCenterKeyHelp("UP", "to add a rectangle and generate a new sequence.", 10, screenHeight - 64, fontSize, BLACK);
      DrawTextCenterKeyHelp("DOWN", "to remove a rectangle and generate a new sequence.", 10, screenHeight - 32, fontSize, BLACK);
    }

    const char* rectCountText = TextFormat("%d rectangles", rectCount);
    int rectCountTextSize = MeasureText(rectCountText, fontSize);
    DrawText(rectCountText, screenWidth - rectCountTextSize - 10, 10, fontSize, BLACK);

    DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------

  free(rectangles);
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

static Color GenerateRandomColor()
{
  return CLITERAL(Color){
    GetRandomValue(0, 255),
    GetRandomValue(0, 255),
    GetRandomValue(0, 255),
    255,
  };
}

static ColorRect* GenerateRandomColorRectSequence(float rectCount, float rectWidth, float screenWidth, float screenHeight){
  int *seq = LoadRandomSequence(rectCount, 0, rectCount-1);
  ColorRect* rectangles = (ColorRect *)malloc(rectCount*sizeof(ColorRect));

  float rectSeqWidth = rectCount * rectWidth;
  int startX = (screenWidth - rectSeqWidth) * 0.5f;

  for(int x=0;x<rectCount;x++){
    int rectHeight = Remap(seq[x], 0, rectCount-1, 0, screenHeight);
    rectangles[x].c = GenerateRandomColor();
    rectangles[x].r = CLITERAL(Rectangle){
      startX + x * rectWidth, screenHeight - rectHeight, rectWidth, rectHeight
    };
  }
  UnloadRandomSequence(seq);
  return rectangles;
}

static void ShuffleColorRectSequence(ColorRect* rectangles, int rectCount)
{
  int *seq = LoadRandomSequence(rectCount, 0, rectCount-1);
  for(int i1=0;i1<rectCount;i1++){
    ColorRect* r1 = &rectangles[i1];
    ColorRect* r2 = &rectangles[seq[i1]];

    // swap only the color and height
    ColorRect tmp = *r1;
    r1->c = r2->c;
    r1->r.height = r2->r.height;
    r1->r.y = r2->r.y;
    r2->c = tmp.c;
    r2->r.height = tmp.r.height;
    r2->r.y = tmp.r.y;

  }
  UnloadRandomSequence(seq);
}

static void DrawTextCenterKeyHelp(const char* key, const char* text, int posX, int posY, int fontSize, Color color)
{
  int spaceSize = MeasureText(" ", fontSize); 
  int pressSize = MeasureText("Press", fontSize); 
  int keySize = MeasureText(key, fontSize); 
  int textSize = MeasureText(text, fontSize); 
  int totalSize = pressSize + 2 * spaceSize + keySize + 2 * spaceSize + textSize;
  int textSizeCurrent = 0;

  DrawText("Press", posX, posY, fontSize, color);
  textSizeCurrent += pressSize + 2 * spaceSize;
  DrawText(key, posX + textSizeCurrent, posY, fontSize, RED);
  DrawRectangle(posX + textSizeCurrent, posY + fontSize, keySize, 3, RED);
  textSizeCurrent += keySize + 2 * spaceSize;
  DrawText(text, posX + textSizeCurrent, posY, fontSize, color);
}