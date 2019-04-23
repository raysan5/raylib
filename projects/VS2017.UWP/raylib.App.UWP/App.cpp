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

/*
TODO list:
	- Cache reference to our CoreWindow?
	- Implement gestures support
*/

// Declare uwpWindow as exter to be used by raylib internals
// NOTE: It should be properly assigned before calling InitWindow()
extern "C" { EGLNativeWindowType uwpWindow; };

/* INPUT CODE */
// Stand-ins for "core.c" variables
#define MAX_GAMEPADS              4         // Max number of gamepads supported
#define MAX_GAMEPAD_BUTTONS       32        // Max bumber of buttons supported (per gamepad)
#define MAX_GAMEPAD_AXIS          8         // Max number of axis supported (per gamepad)

static bool gamepadReady[MAX_GAMEPADS] = { false };             // Flag to know if gamepad is ready
static float gamepadAxisState[MAX_GAMEPADS][MAX_GAMEPAD_AXIS];  // Gamepad axis state
static char previousGamepadState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];    // Previous gamepad buttons state
static char currentGamepadState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];     // Current gamepad buttons state

static char previousKeyState[512] = { 0 };  // Contains previous frame keyboard state
static char currentKeyState[512] = { 0 };   // Contains current frame keyboard state

static char previousMouseState[3] = { 0 };  // Registers previous mouse button state
static char currentMouseState[3] = { 0 };   // Registers current mouse button state
static int previousMouseWheelY = 0;         // Registers previous mouse wheel variation
static int currentMouseWheelY = 0;          // Registers current mouse wheel variation

static bool cursorOnScreen = false;         // Tracks if cursor is inside client area
static bool cursorHidden = false;           // Track if cursor is hidden

static Vector2 mousePosition;
static Vector2 mouseDelta;					// NOTE: Added to keep track of mouse movement while the cursor is locked - no equivalent in "core.c"
static bool toggleCursorLock;

CoreCursor ^regularCursor = ref new CoreCursor(CoreCursorType::Arrow, 0); // The "visible arrow" cursor type

// Helper to process key events
void ProcessKeyEvent(Windows::System::VirtualKey key, int action)
{
	using Windows::System::VirtualKey;
	switch (key)
	{
	case VirtualKey::Space: currentKeyState[KEY_SPACE] = action; break;
	case VirtualKey::Escape: currentKeyState[KEY_ESCAPE] = action; break;
	case VirtualKey::Enter: currentKeyState[KEY_ENTER] = action; break;
	case VirtualKey::Delete: currentKeyState[KEY_BACKSPACE] = action; break;
	case VirtualKey::Right: currentKeyState[KEY_RIGHT] = action; break;
	case VirtualKey::Left: currentKeyState[KEY_LEFT] = action; break;
	case VirtualKey::Down: currentKeyState[KEY_DOWN] = action; break;
	case VirtualKey::Up: currentKeyState[KEY_UP] = action; break;
	case VirtualKey::F1: currentKeyState[KEY_F1] = action; break;
	case VirtualKey::F2: currentKeyState[KEY_F2] = action; break;
	case VirtualKey::F3: currentKeyState[KEY_F4] = action; break;
	case VirtualKey::F4: currentKeyState[KEY_F5] = action; break;
	case VirtualKey::F5: currentKeyState[KEY_F6] = action; break;
	case VirtualKey::F6: currentKeyState[KEY_F7] = action; break;
	case VirtualKey::F7: currentKeyState[KEY_F8] = action; break;
	case VirtualKey::F8: currentKeyState[KEY_F9] = action; break;
	case VirtualKey::F9: currentKeyState[KEY_F10] = action; break;
	case VirtualKey::F10: currentKeyState[KEY_F11] = action; break;
	case VirtualKey::F11: currentKeyState[KEY_F12] = action; break;
	case VirtualKey::LeftShift: currentKeyState[KEY_LEFT_SHIFT] = action; break;
	case VirtualKey::LeftControl: currentKeyState[KEY_LEFT_CONTROL] = action; break;
	case VirtualKey::LeftMenu: currentKeyState[KEY_LEFT_ALT] = action; break; // NOTE: Potential UWP bug with Alt key: https://social.msdn.microsoft.com/Forums/windowsapps/en-US/9bebfb0a-7637-400e-8bda-e55620091407/unexpected-behavior-in-windowscoreuicorephysicalkeystatusismenukeydown
	case VirtualKey::RightShift: currentKeyState[KEY_RIGHT_SHIFT] = action; break;
	case VirtualKey::RightControl: currentKeyState[KEY_RIGHT_CONTROL] = action; break;
	case VirtualKey::RightMenu: currentKeyState[KEY_RIGHT_ALT] = action; break;
	case VirtualKey::Number0: currentKeyState[KEY_ZERO] = action; break;
	case VirtualKey::Number1: currentKeyState[KEY_ONE] = action; break;
	case VirtualKey::Number2: currentKeyState[KEY_TWO] = action; break;
	case VirtualKey::Number3: currentKeyState[KEY_THREE] = action; break;
	case VirtualKey::Number4: currentKeyState[KEY_FOUR] = action; break;
	case VirtualKey::Number5: currentKeyState[KEY_FIVE] = action; break;
	case VirtualKey::Number6: currentKeyState[KEY_SIX] = action; break;
	case VirtualKey::Number7: currentKeyState[KEY_SEVEN] = action; break;
	case VirtualKey::Number8: currentKeyState[KEY_EIGHT] = action; break;
	case VirtualKey::Number9: currentKeyState[KEY_NINE] = action; break;
	case VirtualKey::A: currentKeyState[KEY_A] = action; break;
	case VirtualKey::B: currentKeyState[KEY_B] = action; break;
	case VirtualKey::C: currentKeyState[KEY_C] = action; break;
	case VirtualKey::D: currentKeyState[KEY_D] = action; break;
	case VirtualKey::E: currentKeyState[KEY_E] = action; break;
	case VirtualKey::F: currentKeyState[KEY_F] = action; break;
	case VirtualKey::G: currentKeyState[KEY_G] = action; break;
	case VirtualKey::H: currentKeyState[KEY_H] = action; break;
	case VirtualKey::I: currentKeyState[KEY_I] = action; break;
	case VirtualKey::J: currentKeyState[KEY_J] = action; break;
	case VirtualKey::K: currentKeyState[KEY_K] = action; break;
	case VirtualKey::L: currentKeyState[KEY_L] = action; break;
	case VirtualKey::M: currentKeyState[KEY_M] = action; break;
	case VirtualKey::N: currentKeyState[KEY_N] = action; break;
	case VirtualKey::O: currentKeyState[KEY_O] = action; break;
	case VirtualKey::P: currentKeyState[KEY_P] = action; break;
	case VirtualKey::Q: currentKeyState[KEY_Q] = action; break;
	case VirtualKey::R: currentKeyState[KEY_R] = action; break;
	case VirtualKey::S: currentKeyState[KEY_S] = action; break;
	case VirtualKey::T: currentKeyState[KEY_T] = action; break;
	case VirtualKey::U: currentKeyState[KEY_U] = action; break;
	case VirtualKey::V: currentKeyState[KEY_V] = action; break;
	case VirtualKey::W: currentKeyState[KEY_W] = action; break;
	case VirtualKey::X: currentKeyState[KEY_X] = action; break;
	case VirtualKey::Y: currentKeyState[KEY_Y] = action; break;
	case VirtualKey::Z: currentKeyState[KEY_Z] = action; break;

	}
}

// Callbacks
void App::PointerPressed(CoreWindow^ window, PointerEventArgs^ args)
{
	if (args->CurrentPoint->Properties->IsLeftButtonPressed)
	{
		currentMouseState[MOUSE_LEFT_BUTTON] = 1;
	}
	if (args->CurrentPoint->Properties->IsRightButtonPressed)
	{
		currentMouseState[MOUSE_RIGHT_BUTTON] = 1;
	}
	if (args->CurrentPoint->Properties->IsMiddleButtonPressed)
	{
		currentMouseState[MOUSE_MIDDLE_BUTTON] = 1;
	}
}

void App::PointerReleased(CoreWindow ^window, PointerEventArgs^ args)
{
	if (!(args->CurrentPoint->Properties->IsLeftButtonPressed))
	{
		currentMouseState[MOUSE_LEFT_BUTTON] = 0;
	}
	if (!(args->CurrentPoint->Properties->IsRightButtonPressed))
	{
		currentMouseState[MOUSE_RIGHT_BUTTON] = 0;
	}
	if (!(args->CurrentPoint->Properties->IsMiddleButtonPressed))
	{
		currentMouseState[MOUSE_MIDDLE_BUTTON] = 0;
	}
}

void App::PointerWheelChanged(CoreWindow ^window, PointerEventArgs^ args)
{
	// TODO: Scale the MouseWheelDelta to match GLFW's mouse wheel sensitivity.
	currentMouseWheelY += args->CurrentPoint->Properties->MouseWheelDelta;
}

void App::MouseMoved(Windows::Devices::Input::MouseDevice^ mouseDevice, Windows::Devices::Input::MouseEventArgs^ args)
{
	mouseDelta.x += args->MouseDelta.X;
	mouseDelta.y += args->MouseDelta.Y;
}

void App::OnKeyDown(CoreWindow ^ sender, KeyEventArgs ^ args)
{
	ProcessKeyEvent(args->VirtualKey, 1);
}

void App::OnKeyUp(CoreWindow ^ sender, KeyEventArgs ^ args)
{
	ProcessKeyEvent(args->VirtualKey, 0);
}

/* REIMPLEMENTED FROM CORE.C */
// Get one key state
static bool GetKeyStatus(int key)
{
	return currentKeyState[key];
}

// Show mouse cursor
void UWPShowCursor()
{
	CoreWindow::GetForCurrentThread()->PointerCursor = regularCursor;
	cursorHidden = false;
}

// Hides mouse cursor
void UWPHideCursor()
{
	CoreWindow::GetForCurrentThread()->PointerCursor = nullptr;
	cursorHidden = true;
}

// Set mouse position XY
void UWPSetMousePosition(Vector2 position)
{
	CoreWindow ^window = CoreWindow::GetForCurrentThread();
	Point mousePosScreen = Point(position.x + window->Bounds.X, position.y + window->Bounds.Y);
	window->PointerPosition = mousePosScreen;
	mousePosition = position;
}

// Enables cursor (unlock cursor)
void UWPEnableCursor()
{
	UWPShowCursor();
	UWPSetMousePosition(mousePosition); // The mouse is hidden in the center of the screen - move it to where it should appear
	toggleCursorLock = false;
}

// Disables cursor (lock cursor)
void UWPDisableCursor()
{
	UWPHideCursor();
	toggleCursorLock = true;
}

// Get one mouse button state
static bool UWPGetMouseButtonStatus(int button)
{
	return currentMouseState[button];
}

// Poll (store) all input events
void UWP_PollInput()
{
	// Register previous keyboard state
	for (int k = 0; k < 512; k++) previousKeyState[k] = currentKeyState[k];

	// Process Mouse
	{
		// Register previous mouse states
		for (int i = 0; i < 3; i++) previousMouseState[i] = currentMouseState[i];
		previousMouseWheelY = currentMouseWheelY;
		currentMouseWheelY = 0;

		CoreWindow ^window = CoreWindow::GetForCurrentThread();
		if (toggleCursorLock)
		{
			// Track cursor movement delta, recenter it on the client
			mousePosition.x += mouseDelta.x;
			mousePosition.y += mouseDelta.y;

			// Why we're not using UWPSetMousePosition here...
			//		 UWPSetMousePosition changes the "mousePosition" variable to match where the cursor actually is.
			//		 Our cursor is locked to the middle of screen, and we don't want that reflected in "mousePosition"
			Vector2 centerClient = { (float)(GetScreenWidth() / 2), (float)(GetScreenHeight() / 2) };
			window->PointerPosition = Point(centerClient.x + window->Bounds.X, centerClient.y + window->Bounds.Y);
		}
		else
		{
			// Record the cursor's position relative to the client
			mousePosition.x = window->PointerPosition.X - window->Bounds.X;
			mousePosition.y = window->PointerPosition.Y - window->Bounds.Y;
		}
		
		mouseDelta = { 0 ,0 };
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

// The following functions were ripped from core.c and have *no additional work done on them*
// Detect if a key has been pressed once
bool UWPIsKeyPressed(int key)
{
	bool pressed = false;
	
	if ((currentKeyState[key] != previousKeyState[key]) && (currentKeyState[key] == 1)) pressed = true;
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

	window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::PointerPressed);
	window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::PointerReleased);
	window->PointerWheelChanged += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::PointerWheelChanged);
	window->KeyDown += ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>(this, &App::OnKeyDown);
	window->KeyUp += ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>(this, &App::OnKeyUp);

	Windows::Devices::Input::MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice^, MouseEventArgs^>(this, &App::MouseMoved);
	
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
	currentDisplayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);
	currentDisplayInformation->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

    // The CoreWindow has been created, so EGL can be initialized.

	uwpWindow = (EGLNativeWindowType)window;

	InitWindow(800, 450, NULL);
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
			// Draw
			BeginDrawing();

				ClearBackground(RAYWHITE);
				
				posX += gamepadAxisState[GAMEPAD_PLAYER1][GAMEPAD_XBOX_AXIS_LEFT_X] * 5;
				posY += gamepadAxisState[GAMEPAD_PLAYER1][GAMEPAD_XBOX_AXIS_LEFT_Y] * -5;
				DrawRectangle(posX, posY, 400, 100, RED);

				DrawLine(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);

				DrawCircle(mousePosition.x, mousePosition.y, 40, BLUE);

				if (UWPIsKeyDown(KEY_S)) DrawCircle(100, 100, 100, BLUE);

				if (UWPIsKeyPressed(KEY_A))
				{
					posX -= 50;
					UWPEnableCursor();
				}

				if (UWPIsKeyPressed(KEY_D))
				{
					posX += 50;
					UWPDisableCursor();
				}

				if (currentKeyState[KEY_LEFT_ALT]) DrawRectangle(250, 250, 20, 20, BLACK);
				if (currentKeyState[KEY_BACKSPACE]) DrawRectangle(280, 250, 20, 20, BLACK);
				if (currentMouseState[MOUSE_LEFT_BUTTON]) DrawRectangle(280, 250, 20, 20, BLACK);

				static int pos = 0;
				pos -= currentMouseWheelY;

				DrawRectangle(280, pos + 50, 20, 20, BLACK);
				DrawRectangle(250, 280 + (time++ % 60), 10, 10, PURPLE);

		    EndDrawing();

			UWP_PollInput();

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
