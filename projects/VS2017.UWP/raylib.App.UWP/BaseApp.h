/**********************************************************************************************
*
*   raylib.BaseApp - UWP App generic code for managing interface between C and C++
*
*   LICENSE: zlib/libpng
*   
*   CONFIGURATION:
*   
*    #define PCH
*        This defines what header is the PCH and needs to be included
*
*   Copyright (c) 2013-2020 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#pragma once

#if defined(PCH)
#include PCH
#endif

#include <chrono>
#include <memory>
#include <wrl.h>
#include <thread>

//EGL
#include <EGL/eglplatform.h>

#include "raylib.h"
#include "utils.h"

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

// Stand-ins for "core.c" variables
#define MAX_GAMEPADS              4         // Max number of gamepads supported
#define MAX_GAMEPAD_BUTTONS       32        // Max bumber of buttons supported (per gamepad)
#define MAX_GAMEPAD_AXIS          8         // Max number of axis supported (per gamepad)

// TODO: I want to remove this "BaseApp" thing and just implement this in App.

//Base app implementation
ref class BaseApp : public Windows::ApplicationModel::Core::IFrameworkView
{
public:

    // IFrameworkView Methods.
    virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView)
    {
        // Register event handlers for app lifecycle. This example includes Activated, so that we
        // can make the CoreWindow active and start rendering on the window.
        applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &BaseApp::OnActivated);

        // Logic for other event handlers could go here.
        // Information about the Suspending and Resuming event handlers can be found here:
        // http://msdn.microsoft.com/en-us/library/windows/apps/xaml/hh994930.aspx

        CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &BaseApp::OnResuming);
    }

    virtual void SetWindow(Windows::UI::Core::CoreWindow^ window)
    {
        // Hook window events
        window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &BaseApp::OnWindowSizeChanged);
        window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &BaseApp::OnVisibilityChanged);

        // Hook mouse pointer events
        window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &BaseApp::PointerPressed);
        window->PointerReleased += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &BaseApp::PointerReleased);
        window->PointerWheelChanged += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &BaseApp::PointerWheelChanged);
        window->PointerMoved += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &BaseApp::PointerMoved);

        // Hook keyboard events.
        window->KeyDown += ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>(this, &BaseApp::OnKeyDown);
        window->KeyUp += ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>(this, &BaseApp::OnKeyUp);

        // The CoreWindow has been created, we can pass this to raylib for EGL context creation when it's time
        UWPSetCoreWindowPtr((void*) window);
    }

    virtual void Load(Platform::String^ entryPoint) {}

    void Setup(int width, int height)
    {
        // Set dimensions
        this->width = width;
        this->height = height;
    }

    virtual void Run()
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
        InitWindow(width, height, "raylib game example");
       
        // Set fps if 0
        if (GetFPS() <= 0) SetTargetFPS(60);

        while (!WindowShouldClose())
        {
            if (mWindowVisible)
            {
                // Call update function
                Update();

                PollInput();

                CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            }
            else
            {
                CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
            }
        }

        CloseWindow();
    }

    //Called every frame (Maybe add draw)
    virtual void Update() {}

    virtual void Uninitialize() {}

protected:

    // Input polling
    void PollInput()
    {
        // Process Gamepads
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
    }

    // Application lifecycle event handlers.
    void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args)
    {
        // Run() won't start until the CoreWindow is activated.
        CoreWindow::GetForCurrentThread()->Activate();
    }

    void OnResuming(Platform::Object^ sender, Platform::Object^ args)
    {
        // TODO: Suspend and Resume lifecycle. This should be implemented by the developer, however we need to implement an example of it here.
        // Basically, we'd save all resources here and free any temporary stuff as if we were about to close, however *if* resume is called, we need to be ready.
    }

    // Window event handlers.
    void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args)
    {
        UWPResizeEvent(args->Size.Width, args->Size.Height);
    }

    void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args)
    {
        mWindowVisible = args->Visible;
    }

    // Input event handlers
    void PointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
    {
        auto props = args->CurrentPoint->Properties;

        // TODO: UWP Touch input/simulation.
        if (args->CurrentPoint->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
        {
            if (props->IsLeftButtonPressed) UWPMouseButtonEvent(MOUSE_LEFT_BUTTON, true);
            if (props->IsMiddleButtonPressed) UWPMouseButtonEvent(MOUSE_MIDDLE_BUTTON, true);
            if (props->IsRightButtonPressed) UWPMouseButtonEvent(MOUSE_RIGHT_BUTTON, true);
        }
    }

    void PointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
    {
        auto props = args->CurrentPoint->Properties;

        // TODO: UWP Touch input.
        if (args->CurrentPoint->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
        {
            if (!props->IsLeftButtonPressed) UWPMouseButtonEvent(MOUSE_LEFT_BUTTON, false);
            if (!props->IsMiddleButtonPressed) UWPMouseButtonEvent(MOUSE_MIDDLE_BUTTON, false);
            if (!props->IsRightButtonPressed) UWPMouseButtonEvent(MOUSE_RIGHT_BUTTON, false);
        }
    }

    void PointerWheelChanged(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::PointerEventArgs^ args)
    {
        UWPMouseWheelEvent(args->CurrentPoint->Properties->MouseWheelDelta);
    }

    void PointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
    {
        auto pos = args->CurrentPoint->Position;
        UWPMousePosEvent((double)pos.X, (double)pos.Y);
    }

    int GetRaylibKey(Windows::System::VirtualKey kVey)
    {
        int actualKey = -1;
        switch ((int) kVey)// TODO: Use VK enum and finish keys
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

    void OnKeyDown(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::KeyEventArgs ^ args)
    {
        auto k = GetRaylibKey(args->VirtualKey);
        if (k != -1)
            UWPKeyDownEvent(k, true);
    }

    void OnKeyUp(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::KeyEventArgs ^ args)
    {
        auto k = GetRaylibKey(args->VirtualKey);
        if (k != -1)
            UWPKeyDownEvent(k, false);
    }

private:
    bool mWindowClosed = false;
    bool mWindowVisible = true;

    int width = 640;
    int height = 480;

};

// Application source for creating the program
template<typename AppType>
ref class ApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
    {
        return ref new AppType();
    }
};
