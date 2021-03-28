#include "pch.h"
#include "app.h"

#include <Windows.h>

#include <raylib.h>
#include <uwp_events.h>
#include <gestures.h>

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

// Gamepad bindings struct
struct GamepadBinding
{
    Gamepad^ Gamepad = nullptr;
    bool Ready = false;
};

// Global variables
static int posX = 100;
static int posY = 100;
static int gTime = 0;
static bool mouseLocked = false;
static GamepadBinding gGamepadBindings[MAX_GAMEPADS];

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
    UWPSetQueryTimeFunc([]() {
        static auto timeStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - timeStart);
        return time_span.count(); });

    UWPSetSleepFunc([](double seconds) { std::this_thread::sleep_for(std::chrono::duration<double>(seconds)); });

    UWPSetDisplaySizeFunc([](int* width, int* height) {
        // Get display dimensions
        DisplayInformation^ dInfo = DisplayInformation::GetForCurrentView();
        *width = dInfo->ScreenWidthInRawPixels;
        *height = dInfo->ScreenHeightInRawPixels; });

    UWPSetMouseHideFunc([]() { CoreWindow::GetForCurrentThread()->PointerCursor = nullptr; });

    UWPSetMouseShowFunc([]() { CoreWindow::GetForCurrentThread()->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0); });

    UWPSetMouseLockFunc([]() {
        CoreWindow::GetForCurrentThread()->PointerCursor = nullptr;
        mouseLocked = true; });

    UWPSetMouseUnlockFunc([]() {
        CoreWindow::GetForCurrentThread()->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
        mouseLocked = false; });

    UWPSetMouseSetPosFunc([](int x, int y) {
        CoreWindow^ window = CoreWindow::GetForCurrentThread();
        Point mousePosScreen = Point(x + window->Bounds.X, y + window->Bounds.Y);
        window->PointerPosition = mousePosScreen; });

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
    InitWindow(800, 450, "raylib UWP - Basic example");

    // Run game loop
    while (!WindowShouldClose() && !mSuspended)
    {
        if (mWindowVisible)
        {
            PreProcessInputs();
            GameLoop();
            PostProcessInputs();

            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
        } 
        else
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }
    }

    CloseWindow();
}

void App::Uninitialize()
{
    // Do any UWP cleanup here.
}

// This method is called every frame
void App::GameLoop()
{
    // Update
    //----------------------------------------------------------------------------------
    posX += GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) * 5;
    posY += GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) * -5;

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

    static float pos = 0;
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

		DrawRectangle(280, (int)pos + 50, 20, 20, BLACK);
		DrawRectangle(250, 280 + (gTime++ % 60), 10, 10, PURPLE);

	EndDrawing();
    //----------------------------------------------------------------------------------
}

void App::PreProcessInputs()
{
    // Here, we will see if we have bound gamepads. If we do we check they are still present. If they aren't present we free the binding.
    //  if anyone does not have a binding but there is a gamepad available, we will bind it to the first player who is missing a controller.
    for (auto i = 0; i < MAX_GAMEPADS; i++)
    {
        // Ensure that the gamepad bindings are still in tact
        if (gGamepadBindings[i].Gamepad != nullptr)
        {
            // Check the gamepad is present
            auto found = false;
            for (auto j = 0; j < Gamepad::Gamepads->Size; j++)
            {
                if (gGamepadBindings[i].Gamepad == Gamepad::Gamepads->GetAt(j))
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                gGamepadBindings[i].Gamepad = nullptr;
                gGamepadBindings[i].Ready = false;
            }
            else gGamepadBindings[i].Ready = true;
        }

        // Now we check to find any unbound gamepads we can use
        if (gGamepadBindings[i].Gamepad == nullptr)
        {
            // Loop over all the attached gamepads
            Gamepad^ freeGamepad = nullptr;
            for (auto j = 0; j < Gamepad::Gamepads->Size; j++)
            {
                freeGamepad = Gamepad::Gamepads->GetAt(j);
                // Loop over existing bindings
                for (auto k = 0; k < MAX_GAMEPADS; k++)
                {
                    if (gGamepadBindings[k].Gamepad == freeGamepad)
                        freeGamepad = nullptr;
                }

                // If we passed all 4, this is a free gamepad
                if (freeGamepad != nullptr) break;
            }

            if (freeGamepad != nullptr)
            {
                gGamepadBindings[i].Gamepad = freeGamepad;
                gGamepadBindings[i].Ready = true;
            }
            else gGamepadBindings[i].Ready = false;
        }
    }

    // Send the active gamepads to raylib
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        UWPActivateGamepadEvent(i, gGamepadBindings[i].Ready);
    }

    // Get current gamepad state
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (gGamepadBindings[i].Ready)
        {
            // Get current gamepad state
            auto gamepad = gGamepadBindings[i].Gamepad;
            auto reading = gamepad->GetCurrentReading();

            // Register all button presses
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

            // Register buttons for 2nd triggers (because UWP doesn't count these as buttons but rather axis)
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_LEFT_TRIGGER_2, (bool)(reading.LeftTrigger > 0.1));
            UWPRegisterGamepadButton(i, GAMEPAD_BUTTON_RIGHT_TRIGGER_2, (bool)(reading.RightTrigger > 0.1));

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

void App::PostProcessInputs()
{
    /*
     * So here's the deal. UWP doesn't officially have mouse locking, so we're doing it ourselves here.
     * If anyone has any better ideas on how to implement this feel free!
     * This is done after the game loop so getting mouse delta etc. still works.
     */
    if (mouseLocked)
    {
        auto w = GetScreenWidth();
        auto h = GetScreenHeight();
        SetMousePosition(w / 2, h / 2);
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
    auto device = args->CurrentPoint->PointerDevice;

    if (device->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
    {
        if (props->IsLeftButtonPressed) UWPMouseButtonEvent(MOUSE_LEFT_BUTTON, true);
        if (props->IsMiddleButtonPressed) UWPMouseButtonEvent(MOUSE_MIDDLE_BUTTON, true);
        if (props->IsRightButtonPressed) UWPMouseButtonEvent(MOUSE_RIGHT_BUTTON, true);
    }
    else if (device->PointerDeviceType == PointerDeviceType::Touch)
    {
        auto pos = args->CurrentPoint->Position;
        UWPGestureTouch(args->CurrentPoint->PointerId, pos.X, pos.Y, true);
    }

    args->Handled = true;
}

void App::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    auto props = args->CurrentPoint->Properties;
    auto device = args->CurrentPoint->PointerDevice;

    if (device->PointerDeviceType == PointerDeviceType::Mouse)
    {
        if (!props->IsLeftButtonPressed) UWPMouseButtonEvent(MOUSE_LEFT_BUTTON, false);
        if (!props->IsMiddleButtonPressed) UWPMouseButtonEvent(MOUSE_MIDDLE_BUTTON, false);
        if (!props->IsRightButtonPressed) UWPMouseButtonEvent(MOUSE_RIGHT_BUTTON, false);
    }
    else if (device->PointerDeviceType == PointerDeviceType::Touch)
    {
        auto pos = args->CurrentPoint->Position;
        UWPGestureTouch(args->CurrentPoint->PointerId, pos.X, pos.Y, false);
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
    if (args->CurrentPoint->PointerDevice->PointerDeviceType == PointerDeviceType::Mouse)
    {
        UWPMousePosEvent((double)pos.X, (double)pos.Y);
        args->Handled = true;
    }
    else if (args->CurrentPoint->PointerDevice->PointerDeviceType == PointerDeviceType::Touch)
    {
        UWPGestureMove(args->CurrentPoint->PointerId, pos.X, pos.Y);
    }
}

int App::GetRaylibKey(Windows::System::VirtualKey kVey)
{
    using VK = Windows::System::VirtualKey;
    int actualKey = -1;
    switch (kVey)
    {
        case VK::Back: actualKey = KEY_BACKSPACE; break;
        case VK::Space: actualKey = KEY_SPACE; break;
        case VK::Escape: actualKey = KEY_ESCAPE; break;
        case VK::Enter: actualKey = KEY_ENTER; break;
        case VK::Delete: actualKey = KEY_DELETE; break;
        case VK::Right: actualKey = KEY_RIGHT; break;
        case VK::Left: actualKey = KEY_LEFT; break;
        case VK::Down: actualKey = KEY_DOWN; break;
        case VK::Up: actualKey = KEY_UP; break;
        case VK::F1: actualKey = KEY_F1; break;
        case VK::F2: actualKey = KEY_F2; break;
        case VK::F3: actualKey = KEY_F3; break;
        case VK::F4: actualKey = KEY_F4; break;
        case VK::F5: actualKey = KEY_F5; break;
        case VK::F6: actualKey = KEY_F6; break;
        case VK::F7: actualKey = KEY_F7; break;
        case VK::F8: actualKey = KEY_F8; break;
        case VK::F9: actualKey = KEY_F9; break;
        case VK::F10: actualKey = KEY_F10; break;
        case VK::F11: actualKey = KEY_F11; break;
        case VK::F12: actualKey = KEY_F12; break;
        case VK::LeftShift: actualKey = KEY_LEFT_SHIFT; break;
        case VK::LeftControl: actualKey = KEY_LEFT_CONTROL; break;
        case VK::LeftMenu: actualKey = KEY_LEFT_ALT; break;
        case VK::RightShift: actualKey = KEY_RIGHT_SHIFT; break;
        case VK::RightControl: actualKey = KEY_RIGHT_CONTROL; break;
        case VK::RightMenu: actualKey = KEY_RIGHT_ALT; break;
        case VK::Number0: actualKey = KEY_ZERO; break;
        case VK::Number1: actualKey = KEY_ONE; break;
        case VK::Number2: actualKey = KEY_TWO; break;
        case VK::Number3: actualKey = KEY_THREE; break;
        case VK::Number4: actualKey = KEY_FOUR; break;
        case VK::Number5: actualKey = KEY_FIVE; break;
        case VK::Number6: actualKey = KEY_SIX; break;
        case VK::Number7: actualKey = KEY_SEVEN; break;
        case VK::Number8: actualKey = KEY_EIGHT; break;
        case VK::Number9: actualKey = KEY_NINE; break;
        case VK::NumberPad0: actualKey = KEY_KP_0; break;
        case VK::NumberPad1: actualKey = KEY_KP_1; break;
        case VK::NumberPad2: actualKey = KEY_KP_2; break;
        case VK::NumberPad3: actualKey = KEY_KP_3; break;
        case VK::NumberPad4: actualKey = KEY_KP_4; break;
        case VK::NumberPad5: actualKey = KEY_KP_5; break;
        case VK::NumberPad6: actualKey = KEY_KP_6; break;
        case VK::NumberPad7: actualKey = KEY_KP_7; break;
        case VK::NumberPad8: actualKey = KEY_KP_8; break;
        case VK::NumberPad9: actualKey = KEY_KP_9; break;
        case VK::Decimal: actualKey = KEY_KP_DECIMAL; break;
        case VK::Divide: actualKey = KEY_KP_DIVIDE; break;
        case VK::Multiply: actualKey = KEY_KP_MULTIPLY; break;
        case VK::Subtract: actualKey = KEY_KP_SUBTRACT; break;
        case VK::Add: actualKey = KEY_KP_ADD; break;
        // UWP Doesn't have a specific keypad enter or equal...
        case VK::A: actualKey = KEY_A; break;
        case VK::B: actualKey = KEY_B; break;
        case VK::C: actualKey = KEY_C; break;
        case VK::D: actualKey = KEY_D; break;
        case VK::E: actualKey = KEY_E; break;
        case VK::F: actualKey = KEY_F; break;
        case VK::G: actualKey = KEY_G; break;
        case VK::H: actualKey = KEY_H; break;
        case VK::I: actualKey = KEY_I; break;
        case VK::J: actualKey = KEY_J; break;
        case VK::K: actualKey = KEY_K; break;
        case VK::L: actualKey = KEY_L; break;
        case VK::M: actualKey = KEY_M; break;
        case VK::N: actualKey = KEY_N; break;
        case VK::O: actualKey = KEY_O; break;
        case VK::P: actualKey = KEY_P; break;
        case VK::Q: actualKey = KEY_Q; break;
        case VK::R: actualKey = KEY_R; break;
        case VK::S: actualKey = KEY_S; break;
        case VK::T: actualKey = KEY_T; break;
        case VK::U: actualKey = KEY_U; break;
        case VK::V: actualKey = KEY_V; break;
        case VK::W: actualKey = KEY_W; break;
        case VK::X: actualKey = KEY_X; break;
        case VK::Y: actualKey = KEY_Y; break;
        case VK::Z: actualKey = KEY_Z; break;
        // I don't think we can have any more
    }
    return actualKey;
}

void App::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    auto k = GetRaylibKey(args->VirtualKey);
    auto controlState = (sender->GetKeyState(Windows::System::VirtualKey::Control) & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down;
    if (k != -1) UWPKeyDownEvent(k, true, controlState);
    args->Handled = true;
}

void App::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    auto k = GetRaylibKey(args->VirtualKey);
    if (k != -1) UWPKeyDownEvent(k, false, false);
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
