#include "pch.h"
#include "app.h"

#include <Windows.h>

#include <raylib.h>
#include <uwp_events.h>

#include <chrono>
#include <thread>

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

// Stand-ins for "core.c" variables
#define MAX_GAMEPADS              4         // Max number of gamepads supported
#define MAX_GAMEPAD_BUTTONS       32        // Max bumber of buttons supported (per gamepad)
#define MAX_GAMEPAD_AXIS          8         // Max number of axis supported (per gamepad)

// The main function creates an IFrameworkViewSource for our app, and runs the app
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto appSource = ref new AppSource();
    CoreApplication::Run(appSource);
    return 0;
}

// App implementation
App::App()
{
	// Currently, UWP ignores flags... You must implement flag functionality yourself
	SetConfigFlags(0);
}

void App::Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView)
{
    // Register event handlers for app lifecycle. This example includes Activated, so that we
        // can make the CoreWindow active and start rendering on the window.
    applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

    // Logic for other event handlers could go here.
    // Information about the Suspending and Resuming event handlers can be found here:
    // http://msdn.microsoft.com/en-us/library/windows/apps/xaml/hh994930.aspx

    CoreApplication::Suspending += ref new Windows::Foundation::EventHandler<Windows::ApplicationModel::SuspendingEventArgs^>(this, &App::OnSuspending);
    CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &App::OnResuming);

    // Store the app data directory
    auto dataPath = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
    std::wstring dataPathW(dataPath->Begin());
    static std::string dataPathA(dataPathW.begin(), dataPathW.end());
    UWPSetDataPath(dataPathA.c_str());
}

void App::SetWindow(Windows::UI::Core::CoreWindow^ window)
{
    // Hook window events
    window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);
    window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

    // Hook mouse pointer events
    window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerPressed);
    window->PointerReleased += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerReleased);
    window->PointerWheelChanged += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerWheelChanged);
    window->PointerMoved += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerMoved);

    // Hook keyboard events.
    window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKeyDown);
    window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKeyUp);
    window->CharacterReceived += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::CharacterReceivedEventArgs^>(this, &raylibUWP::App::OnCharacterReceived);

    // The CoreWindow has been created, we can pass this to raylib for EGL context creation when it's time
    UWPSetCoreWindowPtr((void*)window);

    // Register backrequested event to stop window from being closed (Most noticable on XBox when B is pressed)
    auto navigation = SystemNavigationManager::GetForCurrentView();
    navigation->BackRequested += ref new Windows::Foundation::EventHandler<Windows::UI::Core::BackRequestedEventArgs^>(this, &raylibUWP::App::OnBackRequested);
}

void App::Load(Platform::String ^entryPoint) {} // Ignored for this example

void App::Run()
{
    // Set up our UWP implementation
    UWPSetQueryTimeFunc([]()
        {
            static auto timeStart = std::chrono::high_resolution_clock::now();
            auto delta = std::chrono::high_resolution_clock::now() - timeStart;
            return (double)std::chrono::duration_cast<std::chrono::seconds>(delta).count();
        });

    UWPSetSleepFunc([](double seconds) { std::this_thread::sleep_for(std::chrono::duration<double>(seconds)); });

    UWPSetDisplaySizeFunc([](int* width, int* height)
        {
            // Get display dimensions
            DisplayInformation^ dInfo = DisplayInformation::GetForCurrentView();
            *width = dInfo->ScreenWidthInRawPixels;
            *height = dInfo->ScreenHeightInRawPixels;
        });

    UWPSetMouseHideFunc([]()
        {
            CoreWindow::GetForCurrentThread()->PointerCursor = nullptr;
        });

    UWPSetMouseShowFunc([]()
        {
            CoreWindow::GetForCurrentThread()->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
        });

    UWPSetMouseLockFunc([]()
        {
            CoreWindow::GetForCurrentThread()->PointerCursor = nullptr;
            // TODO:
        });

    UWPSetMouseUnlockFunc([]()
        {
            CoreWindow::GetForCurrentThread()->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
            // TODO:
        });

    UWPSetMouseSetPosFunc([](int x, int y)
        {
            CoreWindow^ window = CoreWindow::GetForCurrentThread();
            Point mousePosScreen = Point(x + window->Bounds.X, y + window->Bounds.Y);
            window->PointerPosition = mousePosScreen;
        });

    // Set custom output handle
    SetTraceLogCallback([](int logType, const char* text, va_list args)
        {
            std::string format = text;

            switch (logType)
            {
            case LOG_TRACE: format = std::string("TRACE: ") + format; break;
            case LOG_DEBUG: format = std::string("DEBUG: ") + format; break;
            case LOG_INFO: format = std::string("INFO: ") + format; break;
            case LOG_WARNING: format = std::string("WARNING: ") + format; break;
            case LOG_ERROR: format = std::string("ERROR: ") + format; break;
            case LOG_FATAL: format = std::string("FATAL: ") + format; break;
            default: break;
            }

            char buf[1024]; // TODO: Is this large enough?
            vsnprintf(buf, sizeof(buf), format.c_str(), args);
            std::string output = std::string(buf) + std::string("\n");
            OutputDebugStringA(output.c_str());
        });

    // Create window
    InitWindow(640, 480, "raylib game example");

    // Run game loop
    while (!WindowShouldClose() && !mSuspended)
    {
        if (mWindowVisible)
        {
            ProcessGamepads();
            GameLoop();
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
        } else CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
    }

    CloseWindow();
}

void App::Uninitialize()
{
    // Do any UWP cleanup here.
}

static int posX = 100;
static int posY = 100;
static int gTime = 0;

// This method is called every frame
void App::GameLoop()
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
	
		ClearBackground(RED);

		DrawRectangle(posX, posY, 400, 100, WHITE);

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

void App::ProcessGamepads()
{
    // Check if gamepads are ready
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        // TODO: ROVER - I want to remove this problem if possible.
        // HACK: UWP keeps a contiguous list of gamepads. For the interest of time I'm just doing a 1:1 mapping of
        // connected gamepads with their spot in the list, but this has serious robustness problems
        // e.g. player 1, 2, and 3 are playing a game - if player2 disconnects, p3's controller would now be mapped to p2's character since p3 is now second in the list.
        UWPActivateGamepadEvent(i, i < Gamepad::Gamepads->Size);
    }

    // Get current gamepad state
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (IsGamepadAvailable(i))
        {
            // Get current gamepad state
            auto gamepad = Gamepad::Gamepads->GetAt(i);
            GamepadReading reading = gamepad->GetCurrentReading();

            // NOTE: Maybe it would be wiser to redefine the gamepad button mappings in "raylib.h" for the UWP platform instead of remapping them manually
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_RIGHT_FACE_DOWN, ((reading.Buttons & GamepadButtons::A) == GamepadButtons::A));
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, ((reading.Buttons & GamepadButtons::B) == GamepadButtons::B));
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_RIGHT_FACE_LEFT, ((reading.Buttons & GamepadButtons::X) == GamepadButtons::X));
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_RIGHT_FACE_UP, ((reading.Buttons & GamepadButtons::Y) == GamepadButtons::Y));

            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_LEFT_TRIGGER_1, ((reading.Buttons & GamepadButtons::LeftShoulder) == GamepadButtons::LeftShoulder));
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_RIGHT_TRIGGER_1, ((reading.Buttons & GamepadButtons::RightShoulder) == GamepadButtons::RightShoulder));

            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_MIDDLE_LEFT, ((reading.Buttons & GamepadButtons::View) == GamepadButtons::View)); // Changed for XB1 Controller
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_MIDDLE_RIGHT, ((reading.Buttons & GamepadButtons::Menu) == GamepadButtons::Menu)); // Changed for XB1 Controller

            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_LEFT_FACE_UP, ((reading.Buttons & GamepadButtons::DPadUp) == GamepadButtons::DPadUp));
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_LEFT_FACE_RIGHT, ((reading.Buttons & GamepadButtons::DPadRight) == GamepadButtons::DPadRight));
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_LEFT_FACE_DOWN, ((reading.Buttons & GamepadButtons::DPadDown) == GamepadButtons::DPadDown));
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_LEFT_FACE_LEFT, ((reading.Buttons & GamepadButtons::DPadLeft) == GamepadButtons::DPadLeft));
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_MIDDLE, false); // Home button not supported by UWP

            // Get current axis state
            UWPRegisterGamepadAxis(i, GAMEPAD_AXIS_LEFT_X, (float)reading.LeftThumbstickX);
            UWPRegisterGamepadAxis(i, GAMEPAD_AXIS_LEFT_Y, (float)reading.LeftThumbstickY);
            UWPRegisterGamepadAxis(i, GAMEPAD_AXIS_RIGHT_X, (float)reading.RightThumbstickX);
            UWPRegisterGamepadAxis(i, GAMEPAD_AXIS_RIGHT_Y, (float)reading.RightThumbstickY);
            UWPRegisterGamepadAxis(i, GAMEPAD_AXIS_LEFT_TRIGGER, (float)reading.LeftTrigger);
            UWPRegisterGamepadAxis(i, GAMEPAD_AXIS_RIGHT_TRIGGER, (float)reading.RightTrigger);
        }
    }
}

// Events
void App::OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args)
{
    // Run() won't start until the CoreWindow is activated.
    CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
    // TODO: In your game, you will need to load your state here
    mSuspended = false;
}

void App::OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args)
{
    // TODO: In your game, you will need to save your state here
    mSuspended = true;
}

void App::OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args)
{
    UWPResizeEvent(args->Size.Width, args->Size.Height);
    args->Handled = true;
}

void App::OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args)
{
    mWindowVisible = args->Visible;
    args->Handled = true;
}

// Input event handlers
void App::OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    auto props = args->CurrentPoint->Properties;

    // TODO: UWP Touch input/simulation.
    if (args->CurrentPoint->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
    {
        if (props->IsLeftButtonPressed) UWPMouseButtonEvent(MOUSE_LEFT_BUTTON, true);
        if (props->IsMiddleButtonPressed) UWPMouseButtonEvent(MOUSE_MIDDLE_BUTTON, true);
        if (props->IsRightButtonPressed) UWPMouseButtonEvent(MOUSE_RIGHT_BUTTON, true);
    }

    args->Handled = true;
}

void App::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    auto props = args->CurrentPoint->Properties;

    // TODO: UWP Touch input.
    if (args->CurrentPoint->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
    {
        if (!props->IsLeftButtonPressed) UWPMouseButtonEvent(MOUSE_LEFT_BUTTON, false);
        if (!props->IsMiddleButtonPressed) UWPMouseButtonEvent(MOUSE_MIDDLE_BUTTON, false);
        if (!props->IsRightButtonPressed) UWPMouseButtonEvent(MOUSE_RIGHT_BUTTON, false);
    }

    args->Handled = true;
}

void App::OnPointerWheelChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    UWPMouseWheelEvent(args->CurrentPoint->Properties->MouseWheelDelta);
    args->Handled = true;
}

void App::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    auto pos = args->CurrentPoint->Position;
    UWPMousePosEvent((double)pos.X, (double)pos.Y);
    args->Handled = true;
}

int App::GetRaylibKey(Windows::System::VirtualKey kVey)
{
    int actualKey = -1;
    switch ((int)kVey)// TODO: Use VK enum and finish keys
    {
    case 0x08: actualKey = KEY_BACKSPACE; break;
    case 0x20: actualKey = KEY_SPACE; break;
    case 0x1B: actualKey = KEY_ESCAPE; break;
    case 0x0D: actualKey = KEY_ENTER; break;
    case 0x2E: actualKey = KEY_DELETE; break;
    case 0x27: actualKey = KEY_RIGHT; break;
    case 0x25: actualKey = KEY_LEFT; break;
    case 0x28: actualKey = KEY_DOWN; break;
    case 0x26: actualKey = KEY_UP; break;
    case 0x70: actualKey = KEY_F1; break;
    case 0x71: actualKey = KEY_F2; break;
    case 0x72: actualKey = KEY_F3; break;
    case 0x73: actualKey = KEY_F4; break;
    case 0x74: actualKey = KEY_F5; break;
    case 0x75: actualKey = KEY_F6; break;
    case 0x76: actualKey = KEY_F7; break;
    case 0x77: actualKey = KEY_F8; break;
    case 0x78: actualKey = KEY_F9; break;
    case 0x79: actualKey = KEY_F10; break;
    case 0x7A: actualKey = KEY_F11; break;
    case 0x7B: actualKey = KEY_F12; break;
    case 0xA0: actualKey = KEY_LEFT_SHIFT; break;
    case 0xA2: actualKey = KEY_LEFT_CONTROL; break;
    case 0xA4: actualKey = KEY_LEFT_ALT; break;
    case 0xA1: actualKey = KEY_RIGHT_SHIFT; break;
    case 0xA3: actualKey = KEY_RIGHT_CONTROL; break;
    case 0xA5: actualKey = KEY_RIGHT_ALT; break;
    case 0x30: actualKey = KEY_ZERO; break;
    case 0x31: actualKey = KEY_ONE; break;
    case 0x32: actualKey = KEY_TWO; break;
    case 0x33: actualKey = KEY_THREE; break;
    case 0x34: actualKey = KEY_FOUR; break;
    case 0x35: actualKey = KEY_FIVE; break;
    case 0x36: actualKey = KEY_SIX; break;
    case 0x37: actualKey = KEY_SEVEN; break;
    case 0x38: actualKey = KEY_EIGHT; break;
    case 0x39: actualKey = KEY_NINE; break;
    case 0x41: actualKey = KEY_A; break;
    case 0x42: actualKey = KEY_B; break;
    case 0x43: actualKey = KEY_C; break;
    case 0x44: actualKey = KEY_D; break;
    case 0x45: actualKey = KEY_E; break;
    case 0x46: actualKey = KEY_F; break;
    case 0x47: actualKey = KEY_G; break;
    case 0x48: actualKey = KEY_H; break;
    case 0x49: actualKey = KEY_I; break;
    case 0x4A: actualKey = KEY_J; break;
    case 0x4B: actualKey = KEY_K; break;
    case 0x4C: actualKey = KEY_L; break;
    case 0x4D: actualKey = KEY_M; break;
    case 0x4E: actualKey = KEY_N; break;
    case 0x4F: actualKey = KEY_O; break;
    case 0x50: actualKey = KEY_P; break;
    case 0x51: actualKey = KEY_Q; break;
    case 0x52: actualKey = KEY_R; break;
    case 0x53: actualKey = KEY_S; break;
    case 0x54: actualKey = KEY_T; break;
    case 0x55: actualKey = KEY_U; break;
    case 0x56: actualKey = KEY_V; break;
    case 0x57: actualKey = KEY_W; break;
    case 0x58: actualKey = KEY_X; break;
    case 0x59: actualKey = KEY_Y; break;
    case 0x5A: actualKey = KEY_Z; break;
    }
    return actualKey;
}

void App::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    auto k = GetRaylibKey(args->VirtualKey);
    auto controlState = (sender->GetKeyState(Windows::System::VirtualKey::Control) & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down;
    if (k != -1)
        UWPKeyDownEvent(k, true, controlState);
    args->Handled = true;
}

void App::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    auto k = GetRaylibKey(args->VirtualKey);
    if (k != -1)
        UWPKeyDownEvent(k, false, false);
    args->Handled = true;
}

void App::OnCharacterReceived(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CharacterReceivedEventArgs^ args)
{
    UWPKeyCharEvent(args->KeyCode);
}

void App::OnBackRequested(Platform::Object^ sender, Windows::UI::Core::BackRequestedEventArgs^ args)
{
    // This simply stops the program from closing.
    args->Handled = true;
}

// AppSource implementation
Windows::ApplicationModel::Core::IFrameworkView ^AppSource::CreateView()
{
    return ref new App();
}
