
#include "pch.h"
#include "app.h"

#include "raylib.h"

using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Gaming::Input;
using namespace Windows::Graphics::Display;
using namespace Microsoft::WRL;
using namespace Platform;

using namespace raylibUWP;

/* GAMEPAD CODE */

// Stand-ins for "core.c" variables
#define MAX_GAMEPADS              4         // Max number of gamepads supported
#define MAX_GAMEPAD_BUTTONS       32        // Max bumber of buttons supported (per gamepad)
#define MAX_GAMEPAD_AXIS          8         // Max number of axis supported (per gamepad)
static bool gamepadReady[MAX_GAMEPADS] = { false };             // Flag to know if gamepad is ready
static float gamepadAxisState[MAX_GAMEPADS][MAX_GAMEPAD_AXIS];  // Gamepad axis state
static char previousGamepadState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];    // Previous gamepad buttons state
static char currentGamepadState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];     // Current gamepad buttons state

//#define MAX_KEYS 512
static char previousKeyState[512] = { 0 };  // Contains previous frame keyboard state
static char currentKeyState[512] = { 0 };   // Contains current frame keyboard state

void UWP_PollInput()
{
	// Process Keyboard
	{
		// Register previous keyboard state
		for (int k = 0; k < 512; k++) previousKeyState[k] = currentKeyState[k];

		// Poll keyboard input
		CoreWindow ^window = CoreWindow::GetForCurrentThread();
		using Windows::System::VirtualKey;
		 // NOTE: Potential UWP bug with Alt key: https://social.msdn.microsoft.com/Forums/windowsapps/en-US/9bebfb0a-7637-400e-8bda-e55620091407/unexpected-behavior-in-windowscoreuicorephysicalkeystatusismenukeydown
		currentKeyState[KEY_SPACE] = (window->GetAsyncKeyState(VirtualKey::Space) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_ESCAPE] = (window->GetAsyncKeyState(VirtualKey::Escape) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_ENTER] = (window->GetAsyncKeyState(VirtualKey::Enter) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_BACKSPACE] = (window->GetAsyncKeyState(VirtualKey::Back) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_RIGHT] = (window->GetAsyncKeyState(VirtualKey::Right) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_LEFT] = (window->GetAsyncKeyState(VirtualKey::Left) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_DOWN] = (window->GetAsyncKeyState(VirtualKey::Down) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_UP] = (window->GetAsyncKeyState(VirtualKey::Up) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F1] = (window->GetAsyncKeyState(VirtualKey::F1) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F2] = (window->GetAsyncKeyState(VirtualKey::F2) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F3] = (window->GetAsyncKeyState(VirtualKey::F3) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F4] = (window->GetAsyncKeyState(VirtualKey::F4) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F5] = (window->GetAsyncKeyState(VirtualKey::F5) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F6] = (window->GetAsyncKeyState(VirtualKey::F6) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F7] = (window->GetAsyncKeyState(VirtualKey::F7) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F8] = (window->GetAsyncKeyState(VirtualKey::F8) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F9] = (window->GetAsyncKeyState(VirtualKey::F9) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F10] = (window->GetAsyncKeyState(VirtualKey::F10) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F11] = (window->GetAsyncKeyState(VirtualKey::F11) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F12] = (window->GetAsyncKeyState(VirtualKey::F12) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_LEFT_SHIFT] = (window->GetAsyncKeyState(VirtualKey::LeftShift) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_LEFT_CONTROL] = (window->GetAsyncKeyState(VirtualKey::LeftControl) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_LEFT_ALT] = (window->GetAsyncKeyState(VirtualKey::LeftMenu) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_RIGHT_SHIFT] = (window->GetAsyncKeyState(VirtualKey::RightShift) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_RIGHT_CONTROL] = (window->GetAsyncKeyState(VirtualKey::RightControl) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_RIGHT_ALT] = (window->GetAsyncKeyState(VirtualKey::RightMenu) == CoreVirtualKeyStates::Down);

		currentKeyState[KEY_ZERO] = (window->GetAsyncKeyState(VirtualKey::Number0) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_ONE] = (window->GetAsyncKeyState(VirtualKey::Number1) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_TWO] = (window->GetAsyncKeyState(VirtualKey::Number2) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_THREE] = (window->GetAsyncKeyState(VirtualKey::Number3) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_FOUR] = (window->GetAsyncKeyState(VirtualKey::Number4) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_FIVE] = (window->GetAsyncKeyState(VirtualKey::Number5) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_SIX] = (window->GetAsyncKeyState(VirtualKey::Number6) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_SEVEN] = (window->GetAsyncKeyState(VirtualKey::Number7) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_EIGHT] = (window->GetAsyncKeyState(VirtualKey::Number8) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_NINE] = (window->GetAsyncKeyState(VirtualKey::Number9) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_A] = (window->GetAsyncKeyState(VirtualKey::A) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_B] = (window->GetAsyncKeyState(VirtualKey::B) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_C] = (window->GetAsyncKeyState(VirtualKey::C) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_D] = (window->GetAsyncKeyState(VirtualKey::D) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_E] = (window->GetAsyncKeyState(VirtualKey::E) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_F] = (window->GetAsyncKeyState(VirtualKey::F) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_G] = (window->GetAsyncKeyState(VirtualKey::G) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_H] = (window->GetAsyncKeyState(VirtualKey::H) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_I] = (window->GetAsyncKeyState(VirtualKey::I) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_J] = (window->GetAsyncKeyState(VirtualKey::J) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_K] = (window->GetAsyncKeyState(VirtualKey::K) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_L] = (window->GetAsyncKeyState(VirtualKey::L) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_M] = (window->GetAsyncKeyState(VirtualKey::M) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_N] = (window->GetAsyncKeyState(VirtualKey::N) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_O] = (window->GetAsyncKeyState(VirtualKey::O) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_P] = (window->GetAsyncKeyState(VirtualKey::P) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_Q] = (window->GetAsyncKeyState(VirtualKey::Q) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_R] = (window->GetAsyncKeyState(VirtualKey::R) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_S] = (window->GetAsyncKeyState(VirtualKey::S) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_T] = (window->GetAsyncKeyState(VirtualKey::T) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_U] = (window->GetAsyncKeyState(VirtualKey::U) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_V] = (window->GetAsyncKeyState(VirtualKey::V) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_W] = (window->GetAsyncKeyState(VirtualKey::W) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_X] = (window->GetAsyncKeyState(VirtualKey::X) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_Y] = (window->GetAsyncKeyState(VirtualKey::Y) == CoreVirtualKeyStates::Down);
		currentKeyState[KEY_Z] = (window->GetAsyncKeyState(VirtualKey::Z) == CoreVirtualKeyStates::Down);
	}

	// Process Gamepads
	{
		// Check if gamepads are ready
		for (int i = 0; i < MAX_GAMEPADS; i++)
		{
			// HACK: UWP keeps a contiguous list of gamepads. For the interest of time I'm just doing a 1:1 mapping of
			// connected gamepads with their spot in the list, but this has serious robustness problems
			// e.g. player 1, 2, and 3 are playing a game - if player2 disconnects, p3's controller would now be mapped to p2's character since p3 is now second in the list.

			gamepadReady[i] = (i < Gamepad::Gamepads->Size);
		}

		// Get current gamepad state
		for (int i = 0; i < MAX_GAMEPADS; i++)
		{
			if (gamepadReady[i])
			{
				// Register previous gamepad states
				for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) previousGamepadState[i][k] = currentGamepadState[i][k];

				// Get current gamepad state
				auto gamepad = Gamepad::Gamepads->GetAt(i);
				GamepadReading reading = gamepad->GetCurrentReading();

				// NOTE: Maybe it would be wiser to redefine the gamepad button mappings in "raylib.h" for the UWP platform instead of remapping them manually
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_A] = ((reading.Buttons & GamepadButtons::A) == GamepadButtons::A);
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_B] = ((reading.Buttons & GamepadButtons::B) == GamepadButtons::B);
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_X] = ((reading.Buttons & GamepadButtons::X) == GamepadButtons::X);
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_Y] = ((reading.Buttons & GamepadButtons::Y) == GamepadButtons::Y);
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_LB] = ((reading.Buttons & GamepadButtons::LeftShoulder) == GamepadButtons::LeftShoulder);
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_RB] = ((reading.Buttons & GamepadButtons::RightShoulder) == GamepadButtons::RightShoulder);
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_SELECT] = ((reading.Buttons & GamepadButtons::View) == GamepadButtons::View); // Changed for XB1 Controller
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_START] = ((reading.Buttons & GamepadButtons::Menu) == GamepadButtons::Menu); // Changed for XB1 Controller
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_UP] = ((reading.Buttons & GamepadButtons::DPadUp) == GamepadButtons::DPadUp);
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_RIGHT] = ((reading.Buttons & GamepadButtons::DPadRight) == GamepadButtons::DPadRight);
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_DOWN] = ((reading.Buttons & GamepadButtons::DPadLeft) == GamepadButtons::DPadDown);
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_LEFT] = ((reading.Buttons & GamepadButtons::DPadDown) == GamepadButtons::DPadLeft);
				currentGamepadState[i][GAMEPAD_XBOX_BUTTON_HOME] = false; // Home button not supported by UWP

				// Get current axis state
				gamepadAxisState[i][GAMEPAD_XBOX_AXIS_LEFT_X] = reading.LeftThumbstickX;
				gamepadAxisState[i][GAMEPAD_XBOX_AXIS_LEFT_Y] = reading.LeftThumbstickY;
				gamepadAxisState[i][GAMEPAD_XBOX_AXIS_RIGHT_X] = reading.RightThumbstickX;
				gamepadAxisState[i][GAMEPAD_XBOX_AXIS_RIGHT_Y] = reading.RightThumbstickY;
				gamepadAxisState[i][GAMEPAD_XBOX_AXIS_LT] = reading.LeftTrigger;
				gamepadAxisState[i][GAMEPAD_XBOX_AXIS_RT] = reading.RightTrigger;
			}
		}
	}

}

// The following functions were reimplemented for UWP from core.c
static bool GetKeyStatus(int key)
{
	return currentKeyState[key];
}

// The following functions were ripped from core.c
// Detect if a key has been pressed once
bool UWPIsKeyPressed(int key)
{
	bool pressed = false;
	
	if ((currentKeyState[key] != previousKeyState[key]) && (currentKeyState[key] == 1))
		pressed = true;
	else pressed = false;

	return pressed;
}

// Detect if a key is being pressed (key held down)
bool UWPIsKeyDown(int key)
{
	if (GetKeyStatus(key) == 1) return true;
	else return false;
}

// Detect if a key has been released once
bool UWPIsKeyReleased(int key)
{
	bool released = false;

	if ((currentKeyState[key] != previousKeyState[key]) && (currentKeyState[key] == 0)) released = true;
	else released = false;

	return released;
}

// Detect if a key is NOT being pressed (key not held down)
bool UWPIsKeyUp(int key)
{
	if (GetKeyStatus(key) == 0) return true;
	else return false;
}

/* OTHER CODE */

// Helper to convert a length in device-independent pixels (DIPs) to a length in physical pixels.
inline float ConvertDipsToPixels(float dips, float dpi)
{
    static const float dipsPerInch = 96.0f;
    return floor(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
}

// Implementation of the IFrameworkViewSource interface, necessary to run our app.
ref class SimpleApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
    {
        return ref new App();
    }
};

// The main function creates an IFrameworkViewSource for our app, and runs the app.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto simpleApplicationSource = ref new SimpleApplicationSource();
    CoreApplication::Run(simpleApplicationSource);

    return 0;
}

App::App() :
    mWindowClosed(false),
    mWindowVisible(true)
{
}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
    // Register event handlers for app lifecycle. This example includes Activated, so that we
    // can make the CoreWindow active and start rendering on the window.
    applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

    // Logic for other event handlers could go here.
    // Information about the Suspending and Resuming event handlers can be found here:
    // http://msdn.microsoft.com/en-us/library/windows/apps/xaml/hh994930.aspx

	CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &App::OnResuming);
}

// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);
    window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);
    window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
	currentDisplayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);
	currentDisplayInformation->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

    // The CoreWindow has been created, so EGL can be initialized.
	InitWindow(800, 450, (EGLNativeWindowType)window);
}

// Initializes scene resources
void App::Load(Platform::String^ entryPoint)
{
    // InitWindow() --> rlglInit()
}

static int posX = 100;
static int posY = 100;
static int time = 0;
// This method is called after the window becomes active.
void App::Run()
{
    while (!mWindowClosed)
    {
        if (mWindowVisible)
        {
			// Update
			UWP_PollInput();

			// Draw
			BeginDrawing();

				ClearBackground(RAYWHITE);
				
				
				posX += gamepadAxisState[GAMEPAD_PLAYER1][GAMEPAD_XBOX_AXIS_LEFT_X] * 5;
				posY += gamepadAxisState[GAMEPAD_PLAYER1][GAMEPAD_XBOX_AXIS_LEFT_Y] * -5;
				DrawRectangle(posX, posY, 400, 100, RED);

				DrawLine(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);


				if(UWPIsKeyDown(KEY_S))
				{
					DrawCircle(100, 100, 100, BLUE);
				}

				if(UWPIsKeyPressed(KEY_A))
				{
					posX -= 50;
				}
				if (UWPIsKeyPressed(KEY_D))
				{
					posX += 50;
				}

				if(currentKeyState[KEY_LEFT_ALT])
					DrawRectangle(250, 250, 20, 20, BLACK);
				if (currentKeyState[KEY_BACKSPACE])
					DrawRectangle(280, 250, 20, 20, BLACK);

				DrawRectangle(250, 280 + (time++ % 60), 10, 10, PURPLE);

		    EndDrawing();
			
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
        }
        else
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }
    }

	CloseWindow();
}

// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{
	// CloseWindow();
}

// Application lifecycle event handler.
void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
    // Run() won't start until the CoreWindow is activated.
    CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnResuming(Object^ sender, Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.
}

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	// TODO: Update window and render area size
	//m_deviceResources->SetLogicalSize(Size(sender->Bounds.Width, sender->Bounds.Height));
	//m_main->UpdateForWindowSizeChange();
}

// Window event handlers.
void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
    mWindowVisible = args->Visible;

	// raylib core has the variable windowMinimized to register state,
	// it should be modifyed by this event...
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
    mWindowClosed = true;

	// raylib core has the variable windowShouldClose to register state,
	// it should be modifyed by this event...
}

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	//m_deviceResources->SetDpi(sender->LogicalDpi);
	//m_main->UpdateForWindowSizeChange();
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	//m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	//m_main->UpdateForWindowSizeChange();
}
