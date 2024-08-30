#include "rcore_none.h"
#include <stddef.h>

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context


static PlatformData platform = { 0 };   // Platform specific data

#define GEN_CALLBACK_BODY(fn_ret, ret_value, fn_name, named_args, args) \
    fn_ret fn_name(LIST named_args) { \
        if(internal##fn_name##Callback == NULL) {\
            FUNCTION_MISSING_WARNING(fn_name) \
            return ret_value; \
        } else {\
            return (*internal##fn_name##Callback)(LIST args); \
        }\
    }

// Generate a callback that returns a void.
#define GEN_CALLBACK_BODY_VOID(fn_name, named_args, args) \
    void fn_name(LIST named_args) { \
        if(internal##fn_name##Callback == NULL) {\
            FUNCTION_MISSING_WARNING(fn_name) \
        } else {\
            (*internal##fn_name##Callback)(LIST args); \
        }\
    }

// Generate any of the if/else statements in OverrideInternalFunction.
#define GEN_CASE(fn_name) \
    if (strcmp(funcName, #fn_name) == 0) { \
        internal##fn_name##Callback = &(func-> fn_name); \
        return; \
    }

// For the functions that have default behavior on PLATFORM_OFFSCREEN, check if the function exists
// first.
#define CHECK_AND_CALL(fn_name, args) \
    if(internal##fn_name##Callback != NULL) {\
        return (*internal##fn_name##Callback)(LIST args); \
    }

// For the functions that have default behavior on PLATFORM_OFFSCREEN, check if the function exists
// first (void version).
#define CHECK_AND_CALL_VOID(fn_name, args) \
    if(internal##fn_name##Callback != NULL) {\
        (*internal##fn_name##Callback)(LIST args); \
    }

GEN_CALLBACK_BODY_VOID(ClosePlatform, (void), ())
GEN_CALLBACK_BODY_VOID(ToggleFullscreen, (void), ())
GEN_CALLBACK_BODY_VOID(ToggleBorderlessWindowed, (void), ())
GEN_CALLBACK_BODY_VOID(MaximizeWindow, (void), ())
GEN_CALLBACK_BODY_VOID(MinimizeWindow, (void), ())
GEN_CALLBACK_BODY_VOID(RestoreWindow, (void), ())
GEN_CALLBACK_BODY_VOID(SetWindowIcon, (Image image), (image))
GEN_CALLBACK_BODY_VOID(SetWindowIcons, (Image *images, int count), (images, count))
GEN_CALLBACK_BODY_VOID(SetWindowTitle, (const char *title), (title))
GEN_CALLBACK_BODY_VOID(SetWindowPosition, (int x, int y), (x,y))
GEN_CALLBACK_BODY_VOID(SetWindowMonitor, (int monitor), (monitor))
GEN_CALLBACK_BODY_VOID(SetWindowOpacity, (float opacity), (opacity))
GEN_CALLBACK_BODY_VOID(SetWindowFocused, (void), ())
GEN_CALLBACK_BODY_VOID(EnableCursor, (void), ())
GEN_CALLBACK_BODY_VOID(DisableCursor, (void), ())
GEN_CALLBACK_BODY_VOID(SwapScreenBuffer, (void), ())
GEN_CALLBACK_BODY_VOID(SetGamepadVibration, (int gamepad, float leftMotor, float rightMotor), (gamepad,leftMotor,rightMotor))
GEN_CALLBACK_BODY_VOID(SetMousePosition, (int x, int y), (x,y))
GEN_CALLBACK_BODY_VOID(SetMouseCursor, (int cursor), (cursor))
GEN_CALLBACK_BODY_VOID(PollInputEvents, (void), ())
GEN_CALLBACK_BODY_VOID(SetClipboardText, (const char *text), (text))
GEN_CALLBACK_BODY(int, 0, SetGamepadMappings, (const char *mappings), (mappings))
GEN_CALLBACK_BODY(void *, NULL, GetWindowHandle, (void), ())
GEN_CALLBACK_BODY(Vector2, Vector2Zero(), GetMonitorPosition, (int monitor), (monitor))
GEN_CALLBACK_BODY(int, 0, GetMonitorWidth, (int monitor), (monitor))
GEN_CALLBACK_BODY(int, 0, GetMonitorHeight, (int monitor), (monitor))
GEN_CALLBACK_BODY(int, 0, GetMonitorPhysicalWidth, (int monitor), (monitor))
GEN_CALLBACK_BODY(int, 0, GetMonitorPhysicalHeight, (int monitor), (monitor))
GEN_CALLBACK_BODY(int, 0, GetMonitorRefreshRate, (int monitor), (monitor))
GEN_CALLBACK_BODY(const char*, "", GetMonitorName, (int monitor), (monitor))
GEN_CALLBACK_BODY(Vector2, Vector2Zero(), GetWindowPosition, (void), ())
GEN_CALLBACK_BODY(Vector2, Vector2Zero(), GetWindowScaleDPI, (void), ())
GEN_CALLBACK_BODY(const char *, "", GetClipboardText, (void), ())
GEN_CALLBACK_BODY(const char *, "", GetKeyName, (int key), (key))
GEN_CALLBACK_BODY(double, 0.0, GetTime, (void), ())
GEN_CALLBACK_BODY_VOID(OpenURL, (const char *url), (url))

// Override an internal platform function with your own (PLATFORM_OFFSCREEN and PLATFORM_NONE only).
// Note that there is no responsibility here for the Raylib maintainers to keep a stable API for core functions
void OverrideInternalFunction(const char * funcName, union OverridableFunctionPointer * func) {
    GEN_CASE(InitPlatform)
    GEN_CASE(ClosePlatform)
    GEN_CASE(ToggleFullscreen)
    GEN_CASE(ToggleBorderlessWindowed)
    GEN_CASE(MaximizeWindow)
    GEN_CASE(MinimizeWindow)
    GEN_CASE(RestoreWindow)
    GEN_CASE(SetWindowIcon)
    GEN_CASE(SetWindowIcons)
    GEN_CASE(SetWindowTitle)
    GEN_CASE(SetWindowPosition)
    GEN_CASE(SetWindowMonitor)
    GEN_CASE(SetWindowOpacity)
    GEN_CASE(SetWindowFocused)
    GEN_CASE(EnableCursor)
    GEN_CASE(DisableCursor)
    GEN_CASE(SwapScreenBuffer)
    GEN_CASE(SetGamepadVibration)
    GEN_CASE(SetMousePosition)
    GEN_CASE(SetMouseCursor)
    GEN_CASE(PollInputEvents)
    GEN_CASE(SetClipboardText)
    GEN_CASE(SetGamepadMappings)
    GEN_CASE(GetWindowHandle)
    GEN_CASE(GetMonitorPosition)
    GEN_CASE(GetMonitorWidth)
    GEN_CASE(GetMonitorHeight)
    GEN_CASE(GetMonitorPhysicalWidth)
    GEN_CASE(GetMonitorPhysicalHeight)
    GEN_CASE(GetMonitorRefreshRate)
    GEN_CASE(GetMonitorName)
    GEN_CASE(GetWindowPosition)
    GEN_CASE(GetWindowScaleDPI)
    GEN_CASE(GetClipboardText)
    GEN_CASE(GetKeyName)
    GEN_CASE(GetTime)

    TraceLog(LOG_ERROR, "Unknown function name \"%s\", did not bind.",funcName);
};

CoreData * GetCore() {
    return &CORE;
}

#ifdef PLATFORM_OFFSCREEN
// 
int InitPlatform(void) { 
    CHECK_AND_CALL(InitPlatform, ());
    CORE.Window.ready = true;
    TRACELOG(LOG_WARNING, "InitPlatform was called but not overriden by the user"); 
    return 0;
}

// Check if application should close
// NOTE: By default, if KEY_ESCAPE pressed or window close icon clicked

bool WindowShouldClose(void)
{
    CHECK_AND_CALL(WindowShouldClose, ());
    if (CORE.Window.ready) return CORE.Window.shouldClose;
    else return true;
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    CHECK_AND_CALL_VOID(SetWindowState, (flags));
    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: FLAG_VSYNC_HINT
    if (((CORE.Window.flags & FLAG_VSYNC_HINT) != (flags & FLAG_VSYNC_HINT)) && ((flags & FLAG_VSYNC_HINT) > 0))
    {
                CORE.Window.flags |= FLAG_VSYNC_HINT;
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    // NOTE: This must be handled before FLAG_FULLSCREEN_MODE because ToggleBorderlessWindowed() needs to get some fullscreen values if fullscreen is running
    if (((CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) != (flags & FLAG_BORDERLESS_WINDOWED_MODE)) && ((flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0))
    {
        ToggleBorderlessWindowed();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_FULLSCREEN_MODE
    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) != (flags & FLAG_FULLSCREEN_MODE))
    {
        ToggleFullscreen();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if (((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) != (flags & FLAG_WINDOW_RESIZABLE)) && ((flags & FLAG_WINDOW_RESIZABLE) > 0))
    {
        CORE.Window.flags |= FLAG_WINDOW_RESIZABLE;
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if (((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) != (flags & FLAG_WINDOW_UNDECORATED)) && (flags & FLAG_WINDOW_UNDECORATED))
    {
        CORE.Window.flags |= FLAG_WINDOW_UNDECORATED;
    }

    // State change: FLAG_WINDOW_HIDDEN
    if (((CORE.Window.flags & FLAG_WINDOW_HIDDEN) != (flags & FLAG_WINDOW_HIDDEN)) && ((flags & FLAG_WINDOW_HIDDEN) > 0))
    {
        CORE.Window.flags |= FLAG_WINDOW_HIDDEN;
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) != (flags & FLAG_WINDOW_MINIMIZED)) && ((flags & FLAG_WINDOW_MINIMIZED) > 0))
    {
        //MinimizeWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) != (flags & FLAG_WINDOW_MAXIMIZED)) && ((flags & FLAG_WINDOW_MAXIMIZED) > 0))
    {
        //MaximizeWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if (((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) != (flags & FLAG_WINDOW_UNFOCUSED)) && ((flags & FLAG_WINDOW_UNFOCUSED) > 0))
    {
        CORE.Window.flags |= FLAG_WINDOW_UNFOCUSED;
    }

    // State change: FLAG_WINDOW_TOPMOST
    if (((CORE.Window.flags & FLAG_WINDOW_TOPMOST) != (flags & FLAG_WINDOW_TOPMOST)) && ((flags & FLAG_WINDOW_TOPMOST) > 0))
    {
        CORE.Window.flags |= FLAG_WINDOW_TOPMOST;
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if (((CORE.Window.flags & FLAG_WINDOW_ALWAYS_RUN) != (flags & FLAG_WINDOW_ALWAYS_RUN)) && ((flags & FLAG_WINDOW_ALWAYS_RUN) > 0))
    {
        CORE.Window.flags |= FLAG_WINDOW_ALWAYS_RUN;
    }

    // The following states can not be changed after window creation

    // State change: FLAG_WINDOW_TRANSPARENT
    if (((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) != (flags & FLAG_WINDOW_TRANSPARENT)) && ((flags & FLAG_WINDOW_TRANSPARENT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if (((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) != (flags & FLAG_WINDOW_HIGHDPI)) && ((flags & FLAG_WINDOW_HIGHDPI) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if (((CORE.Window.flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) != (flags & FLAG_WINDOW_MOUSE_PASSTHROUGH)) && ((flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0))
    {
        CORE.Window.flags |= FLAG_WINDOW_MOUSE_PASSTHROUGH;
    }

    // State change: FLAG_MSAA_4X_HINT
    if (((CORE.Window.flags & FLAG_MSAA_4X_HINT) != (flags & FLAG_MSAA_4X_HINT)) && ((flags & FLAG_MSAA_4X_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: MSAA can only be configured before window initialization");
    }

    // State change: FLAG_INTERLACED_HINT
    if (((CORE.Window.flags & FLAG_INTERLACED_HINT) != (flags & FLAG_INTERLACED_HINT)) && ((flags & FLAG_INTERLACED_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    CHECK_AND_CALL_VOID(ClearWindowState, (flags));
    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: FLAG_VSYNC_HINT
    if (((CORE.Window.flags & FLAG_VSYNC_HINT) > 0) && ((flags & FLAG_VSYNC_HINT) > 0))
    {
        CORE.Window.flags &= ~FLAG_VSYNC_HINT;
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    // NOTE: This must be handled before FLAG_FULLSCREEN_MODE because ToggleBorderlessWindowed() needs to get some fullscreen values if fullscreen is running
    if (((CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0) && ((flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0))
    {
        ToggleBorderlessWindowed();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_FULLSCREEN_MODE
    if (((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0) && ((flags & FLAG_FULLSCREEN_MODE) > 0))
    {
        ToggleFullscreen();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if (((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) > 0) && ((flags & FLAG_WINDOW_RESIZABLE) > 0))
    {
        CORE.Window.flags &= ~FLAG_WINDOW_RESIZABLE;
    }

    // State change: FLAG_WINDOW_HIDDEN
    if (((CORE.Window.flags & FLAG_WINDOW_HIDDEN) > 0) && ((flags & FLAG_WINDOW_HIDDEN) > 0))
    {
        CORE.Window.flags &= ~FLAG_WINDOW_HIDDEN;
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) && ((flags & FLAG_WINDOW_MINIMIZED) > 0))
    {
        RestoreWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) > 0) && ((flags & FLAG_WINDOW_MAXIMIZED) > 0))
    {
        RestoreWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if (((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) > 0) && ((flags & FLAG_WINDOW_UNDECORATED) > 0))
    {
        CORE.Window.flags &= ~FLAG_WINDOW_UNDECORATED;
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if (((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) > 0) && ((flags & FLAG_WINDOW_UNFOCUSED) > 0))
    {
        CORE.Window.flags &= ~FLAG_WINDOW_UNFOCUSED;
    }

    // State change: FLAG_WINDOW_TOPMOST
    if (((CORE.Window.flags & FLAG_WINDOW_TOPMOST) > 0) && ((flags & FLAG_WINDOW_TOPMOST) > 0))
    {
        CORE.Window.flags &= ~FLAG_WINDOW_TOPMOST;
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if (((CORE.Window.flags & FLAG_WINDOW_ALWAYS_RUN) > 0) && ((flags & FLAG_WINDOW_ALWAYS_RUN) > 0))
    {
        CORE.Window.flags &= ~FLAG_WINDOW_ALWAYS_RUN;
    }

    // The following states can not be changed after window creation

    // State change: FLAG_WINDOW_TRANSPARENT
    if (((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) > 0) && ((flags & FLAG_WINDOW_TRANSPARENT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if (((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0) && ((flags & FLAG_WINDOW_HIGHDPI) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if (((CORE.Window.flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0) && ((flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0))
    {
        CORE.Window.flags &= ~FLAG_WINDOW_MOUSE_PASSTHROUGH;
    }

    // State change: FLAG_MSAA_4X_HINT
    if (((CORE.Window.flags & FLAG_MSAA_4X_HINT) > 0) && ((flags & FLAG_MSAA_4X_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: MSAA can only be configured before window initialization");
    }

    // State change: FLAG_INTERLACED_HINT
    if (((CORE.Window.flags & FLAG_INTERLACED_HINT) > 0) && ((flags & FLAG_INTERLACED_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    CHECK_AND_CALL_VOID(SetWindowMinSize, (width, height));
    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    CHECK_AND_CALL_VOID(SetWindowMaxSize, (width, height));
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    CHECK_AND_CALL_VOID(SetWindowSize, (width, height));
    CORE.Window.screen.width = width;
    CORE.Window.screen.height = height;
}

// Get number of monitors
int GetMonitorCount(void)
{
    CHECK_AND_CALL(GetMonitorCount, ());
    return 1;
}

// Get number of monitors
int GetCurrentMonitor(void)
{
    CHECK_AND_CALL(GetCurrentMonitor, ());
    return 0;
}

// Show mouse cursor
void ShowCursor(void)
{
    CHECK_AND_CALL_VOID(ShowCursor, ());
    CORE.Input.Mouse.cursorHidden = false;
}
// Hides mouse cursor
void HideCursor(void)
{
    CHECK_AND_CALL_VOID(HideCursor, ());
    CORE.Input.Mouse.cursorHidden = true;
}


#endif
