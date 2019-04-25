#include "pch.h"
#include "app.h"

#include "raylib.h"

using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Devices::Input;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Gaming::Input;
using namespace Windows::Graphics::Display;
using namespace Microsoft::WRL;
using namespace Platform;

using namespace raylibUWP;

/* OTHER CODE */

//TODO: Remove or not...
// Helper to convert a length in device-independent pixels (DIPs) to a length in physical pixels.
inline float ConvertDipsToPixels(float dips, float dpi)
{
    static const float dipsPerInch = 96.0f;
    return floor(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
}

// The main function creates an IFrameworkViewSource for our app, and runs the app.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto appSource = ref new ApplicationSource<App>();
    CoreApplication::Run(appSource);

    return 0;
}

App::App() {}

static int posX = 100;
static int posY = 100;
static int time = 0;
// This method is called after the window becomes active.
void App::Update()
{
	// Draw
	BeginDrawing();
	
		ClearBackground(RAYWHITE);
		
		//posX += gamepadAxisState[GAMEPAD_PLAYER1][GAMEPAD_XBOX_AXIS_LEFT_X] * 5;
		//posY += gamepadAxisState[GAMEPAD_PLAYER1][GAMEPAD_XBOX_AXIS_LEFT_Y] * -5;
		DrawRectangle(posX, posY, 400, 100, RED);

		DrawLine(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);

		auto mPos = GetMousePosition();

		DrawCircle(mPos.x, mPos.y, 40, BLUE);

		if (IsKeyDown(KEY_S)) DrawCircle(100, 100, 100, BLUE);

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

		if (IsKeyDown(KEY_LEFT_ALT)) //Unable to get working on my PC
			DrawRectangle(250, 250, 20, 20, BLACK);
		if (IsKeyDown(KEY_BACKSPACE))
			DrawRectangle(280, 250, 20, 20, BLACK);
		if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
			DrawRectangle(280, 250, 20, 20, BLACK);

		static int pos = 0;
		pos -= GetMouseWheelMove();
		
		DrawRectangle(280, pos + 50, 20, 20, BLACK);
		DrawRectangle(250, 280 + (time++ % 60), 10, 10, PURPLE);

	EndDrawing();
}