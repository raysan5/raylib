#include "raylib.h"

// Some defines
#define SNAKE_LENGTH 24
#define BOX_SIZE 50

// Types and structure definition
typedef struct Snake
{
	Vector2 position;
	Color color;
} Snake;

typedef struct Food
{
	Vector2 position;
	Color color;
} Food;

typedef struct  Offset	 
{
	int x, y;
} Offset;

// Global variable declaration
static const int screenWidth = 1030;
static Snake snake[SNAKE_LENGTH] = {0};
static const int screenHeight = 780;
static Food fruit = { 0 };
static bool gameOver = false;
static bool pause = false;
static Offset offset = { 0 };
static int tailCount = 0;
static bool isFruitAvailable = false;
static bool shouldFruitSpawn = false;
static Vector2 snakeSpeed = { 0 };
static int frameCounter = 0;
static int allowInput = false;

// Modules declaration (local)
static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);

int main(void)
{
	InitWindow(screenWidth, screenHeight, "The Snake Game");
	InitGame();
	// Main game loop
	SetTargetFPS(60);
	while (!WindowShouldClose())
	{
		UpdateDrawFrame();
	}
	UnloadGame();
	CloseWindow();


	return 0;
}

static void InitGame(void)
{
	// Init some vars
	gameOver = false;
	pause = false;
	offset.x = screenWidth % BOX_SIZE;
	offset.y = screenHeight % BOX_SIZE;
	tailCount = 0;
	snakeSpeed = (Vector2){ BOX_SIZE, 0 };
	// snake initial position
	for (int i = 0; i < SNAKE_LENGTH; i++)
	{
		if (i == 0)
		{
			snake[i].color = BLUE;
			snake[i].position = (Vector2){ offset.x / 2, offset.y / 2 };
		}
		else {
			snake[i].color = LIGHTGRAY;
			snake[i].position = (Vector2){ -100, -100 };
		}
	}
	fruit.color = RED;
	shouldFruitSpawn = true;
	allowInput = true;

	
}

static void UpdateDrawFrame(void)
{
	UpdateGame();
	DrawGame();
}

static void UpdateGame(void)
{
	if (!gameOver)
	{
		if (IsKeyPressed('P') )pause = !pause;
		if (!pause)
	{
		// handle snake
			// Player input
			if (IsKeyPressed(KEY_RIGHT) && allowInput && snakeSpeed.x == 0)
			{
				snakeSpeed = (Vector2){ BOX_SIZE, 0 };
				allowInput = false;
			}
			if (IsKeyPressed(KEY_DOWN) && allowInput && snakeSpeed.y == 0)
			{
				snakeSpeed = (Vector2){ 0, BOX_SIZE };
				allowInput = false;
			}
			if (IsKeyPressed(KEY_LEFT) && allowInput && snakeSpeed.x == 0)
			{
				snakeSpeed = (Vector2){ - BOX_SIZE, 0 };
				allowInput = false;
			}
			if (IsKeyPressed(KEY_UP) && allowInput && snakeSpeed.y == 0)
			{
				snakeSpeed = (Vector2){ 0, - BOX_SIZE };
				allowInput = false;
			}
			
			// Snake movement
			if (frameCounter % 5 == 0)
			{
				// tail positions
				for (int i = tailCount; i > 0; i--)
				{
					//printf("%d", tailCount);
					snake[i].position.x = snake[i - 1].position.x;
					snake[i].position.y = snake[i - 1].position.y;
				}
				// head position
				//printf("%f", snake[0].position.x);
				snake[0].position.x += snakeSpeed.x;
				snake[0].position.y += snakeSpeed.y;
				allowInput = true;
			}
		
			
		
		
			// handle fruit
			// check if fruit should be spawn
			if (shouldFruitSpawn)
			{
				while (!isFruitAvailable)
				{
					isFruitAvailable = true;
					fruit.position.x = offset.x /2 + GetRandomValue(0 , screenWidth / BOX_SIZE - 1) * BOX_SIZE;
					fruit.position.y = offset.y/2 + GetRandomValue(0, screenHeight / BOX_SIZE - 1) * BOX_SIZE;
			
					for (int i = 0; i < tailCount ; i++)
					{	
						// Check if fruit spawn is overlap with any of the snake body
						if ((fruit.position.x == snake[i].position.x) && (fruit.position.y == snake[i].position.y))
						{
							isFruitAvailable = false;
							shouldFruitSpawn = false;
						}
					}
				}
			}
		
			// handle collision
			// collision with fruit
			if (snake[0].position.x == fruit.position.x && snake[0].position.y == fruit.position.y)
			{
				tailCount++;
				shouldFruitSpawn = true;
				isFruitAvailable = false;
			}
			// collision with wall
			/*if ((snake[0].position.x > screenWidth - offset.x || snake[0].position.x < 0) || 
				(snake[0].position.y > screenHeight - offset.y || snake[0].position.y < 0)) 
					gameOver = true;*/
			if (((snake[0].position.x) > (screenWidth - offset.x)) ||
				((snake[0].position.y) > (screenHeight - offset.y)) ||
				(snake[0].position.x < 0) || (snake[0].position.y < 0))
			{
				gameOver = true;
			}

			// collision with self
			for (int i = 1; i <= tailCount; i++)
			{
				gameOver = (snake[0].position.x == snake[i].position.x) && (snake[0].position.y == snake[i].position.y);
				if (gameOver)
				{
					break;
				}
			}
			frameCounter++;
		}
	}
	else
	{
		if (IsKeyPressed(KEY_ENTER))
		{
			InitGame();
		}
	}
}


static void DrawGame(void)
{
	BeginDrawing();
	
	ClearBackground(RAYWHITE);
	
	if (!gameOver)
	{
		// Draw grid vertical
		for (int i = 0; i < screenWidth / BOX_SIZE + 1; i++)
		{
			DrawLine( offset.x /2 + i * BOX_SIZE, offset.y / 2, offset.x /2 + i * BOX_SIZE, screenHeight - offset.y / 2, GRAY);

		}
		// Draw grid horizontal
		for (int i = 0; i < screenHeight / BOX_SIZE + 1; i++)
		{
			DrawLine( offset.x / 2, offset.y /2 + i * BOX_SIZE, screenWidth - offset.x /2 , offset.y /2  + i * BOX_SIZE, GRAY);
		}
		// Draw snake head
		DrawRectangle(snake[0].position.x, snake[0].position.y, BOX_SIZE, BOX_SIZE, snake[0].color);
		// Draw snake tail
		for (int i = tailCount; i > 0; i--)
		{
			DrawRectangle(snake[i].position.x, snake[i].position.y, BOX_SIZE, BOX_SIZE, snake[i].color);
		}
		// Draw fruit

		DrawRectangle(fruit.position.x, fruit.position.y, BOX_SIZE, BOX_SIZE, fruit.color);
		if (pause)
		{
			DrawText("Game paused", screenWidth / 2 - MeasureText("Game paused", 40) / 2, screenHeight/2 - 40, 40, GRAY);
		}

	}
	else
	{
		DrawText("Press [Enter] to player again!", screenWidth / 2 - MeasureText(("Press [Enter] to player again!"), 40) / 2, screenHeight / 2 - 40, 40, GRAY);
	}

	EndDrawing();
}

static void UnloadGame(void)
{

}

