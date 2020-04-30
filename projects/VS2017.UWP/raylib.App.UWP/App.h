#pragma once

#include <string>

#include "pch.h"

namespace raylibUWP
{
    ref class App sealed : public Windows::ApplicationModel::Core::IFrameworkView
    {
    public:
        App();

        // IFrameworkView methods.
        void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView) override;
        void SetWindow(Windows::UI::Core::CoreWindow^ window) override;
        void Load(Platform::String^ entryPoint) override;
        void Run() override;
        void Uninitialize() override;
    private:
        bool mWindowVisible = true;
        bool mSuspended = false;

        void GameLoop();
        void PreProcessInputs();
        void PostProcessInputs();

        // Helpers
        int GetRaylibKey(Windows::System::VirtualKey kVey);

        // Events
        void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
        void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
        void OnResuming(Platform::Object^ sender, Platform::Object^ args);
        void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
        void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
        void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
        void OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
        void OnPointerWheelChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
        void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
        void OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
        void OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
        void OnCharacterReceived(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CharacterReceivedEventArgs^ args);
        void OnBackRequested(Platform::Object^ sender, Windows::UI::Core::BackRequestedEventArgs^ args);
    };

    ref class AppSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
    {
    public:
        Windows::ApplicationModel::Core::IFrameworkView^ CreateView() override;
    };
}
