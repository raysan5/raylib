#include "pch.h"
#include "app.h"

#include "raylib.h"

using namespace raylibUWP;

// The main function creates an IFrameworkViewSource for our app, and runs the app.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto appSource = ref new ApplicationSource<App>();
    CoreApplication::Run(appSource);

    return 0;
}

App::App()
{
	// This does not work... need to fix this.
	SetConfigFlags(0);

	Setup(640, 480);
}

static int posX = 100;
static int posY = 100;
static int gTime = 0;

// This method is called every frame
void App::Update()
{
    // Update
    //----------------------------------------------------------------------------------
    posX += GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_LEFT_X) * 5;
    posY += GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_LEFT_Y) * -5;

    auto mPos = GetMousePosition();

    if (IsKeyPressed(KEY_A))
    {
        posX -= 50;
        EnableCursor();
    }

    if (IsKeyPressed(KEY_D))
    {
        posX += 50;
        DisableCursor();
    }

    static int pos = 0;
    pos -= GetMouseWheelMove();
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
	BeginDrawing();
	
		ClearBackground(RAYWHITE);

		DrawRectangle(posX, posY, 400, 100, RED);

		DrawLine(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);

		DrawCircle(mPos.x, mPos.y, 40, BLUE);

		if (IsKeyDown(KEY_S)) DrawCircle(100, 100, 100, BLUE);

		if (IsKeyDown(KEY_LEFT_ALT)) DrawRectangle(250, 250, 20, 20, BLACK);
		if (IsKeyDown(KEY_BACKSPACE)) DrawRectangle(280, 250, 20, 20, BLACK);
		if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) DrawRectangle(280, 250, 20, 20, BLACK);

		DrawRectangle(280, pos + 50, 20, 20, BLACK);
		DrawRectangle(250, 280 + (gTime++ % 60), 10, 10, PURPLE);

	EndDrawing();
    //----------------------------------------------------------------------------------
}