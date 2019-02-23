
#include <iostream>
#include "raylib.h"
#include <vector>
#include <time.h>

using namespace std;

typedef struct room
{
	int x = 0;
	int y = 0;
	int width = 10;
	int height = 10;
};


room Temp_Rect_Creat()
{
	room in_creation;

	in_creation.x = int(rand() % 700);
	in_creation.y = int(rand() % 400);

	in_creation.width = int(rand() % 50);
	if (in_creation.width < 10) in_creation.width = 10;
	in_creation.height = int(rand() % 70);
	if (in_creation.height < 10) in_creation.height = 10;

	return in_creation;
}

int main()
{
	srand(time(NULL));

	int screenWidth = 800;
	int screenHeight = 450;

	vector<room> rooms;
	InitWindow(screenWidth, screenHeight, "raylib [core] example - keyboard input");

	Vector2 ballPosition = { (float)screenWidth / 2, (float)screenHeight / 2 };

	SetTargetFPS(60);

	int i = 0;
	while (i < 20)
	{
		room temp = Temp_Rect_Creat();
		rooms.push_back(temp);
		i++;
	}

	while (!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(RAYWHITE);

		for (int i = 0; i < rooms.size(); i++)
		{
			DrawRectangle(rooms[i].x, rooms[i].y, rooms[i].width, rooms[i].height, BLACK);
		}

		EndDrawing();
	}

	CloseWindow();

	return 0;
}