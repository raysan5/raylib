
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

void UWP_PollInput()
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

			// NOTE: Maybe it would be wiser to redefine the gamepad button mappings in "raylib.h" for the UWP platform instead of doing this
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

/* OTHER CODE*/

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

// This method is called after the window becomes active.
void App::Run()
{
    while (!mWindowClosed)
    {
        if (mWindowVisible)
        {
			// Update

			// Draw
			BeginDrawing();

				ClearBackground(RAYWHITE);

				DrawRectangle(100, 100, 400, 100, RED);

				DrawLine(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);

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
