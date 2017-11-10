
#include "pch.h"
#include "app.h"

#include "raylib.h"

using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Microsoft::WRL;
using namespace Platform;

using namespace raylibUWP;

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
