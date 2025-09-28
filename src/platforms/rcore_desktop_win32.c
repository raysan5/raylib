/**********************************************************************************************
*
*   rcore_desktop_win32 - Functions to manage window, graphics device and inputs
*
*   PLATFORM: DESKTOP: WIN32
*       - Windows (Win32, Win64)
*
*   LIMITATIONS:
*       - Initial development stage, lot of functionality missing
*       - No support for MOUSE_BUTTON_FORWARD/MOUSE_BUTTON_BACK
*
*   POSSIBLE IMPROVEMENTS:
*       - Improvement 01
*       - Improvement 02
*
*   ADDITIONAL NOTES:
*       - TRACELOG() function is located in raylib [utils] module
*
*   CONFIGURATION:
*       #define RCORE_PLATFORM_CUSTOM_FLAG
*           Custom flag for rcore on target platform -not used-
*
*   DEPENDENCIES:
*       - Win32 API (windows.h)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2025 Ramon Santamaria (@raysan5) and contributors
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

// Move windows.h symbols to new names to avoid redefining the same names as raylib
#define CloseWindow CloseWindowWin32
#define Rectangle RectangleWin32
#define ShowCursor ShowCursorWin32
#define LoadImageA LoadImageAWin32
#define LoadImageW LoadImageWin32
#define DrawTextA DrawTextAWin32
#define DrawTextW DrawTextWin32
#define DrawTextExA DrawTextExAWin32
#define DrawTextExW DrawTextExWin32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef CloseWindow      // raylib symbol collision
#undef Rectangle        // raylib symbol collision
#undef ShowCursor       // raylib symbol collision
#undef LoadImage        // raylib symbol collision
#undef LoadImageA
#undef LoadImageW
#undef DrawText         // raylib symbol collision
#undef DrawTextA
#undef DrawTextW
#undef DrawTextEx       // raylib symbol collision
#undef DrawTextExA
#undef DrawTextExW

#include <windowsx.h>
#include <shellscalingapi.h>
#include <versionhelpers.h>

#include <GL/gl.h>
//#include <GL/wglext.h>

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// NOTE: appScreenSize is the last screen size requested by the app, 
// the backend must keep the client area this size (after DPI scaling is applied) 
// when the window isn't fullscreen/maximized/minimized
typedef struct {
    HWND hwnd;
    HDC hdc;
    HGLRC glContext;
    LARGE_INTEGER timerFrequency;
    unsigned int appScreenWidth;
    unsigned int appScreenHeight;
    unsigned int desiredFlags;
    bool cursorEnabled;
} PlatformData;

// Define WGL function pointer types (no wglext.h needed)
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int *);
typedef BOOL (WINAPI *PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC, const int *, const FLOAT *, UINT, int *, UINT *);
typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int);

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific data

// Required WGL functions
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

// --------------------------------------------------------------------------------
// This part of the file contains pure functions that never access global state
// This distinction helps keep the backend maintainable as the inputs and outputs
// of every function called in this section can be fully derived from the call-site alone
// --------------------------------------------------------------------------------

// Prevent any code in this part of the file from accessing the global CORE state
#define CORE DONT_USE_CORE_HERE

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define A_TO_W_ALLOCA(outWstr, inAnsi)   do {                   \
    size_t len = AToWLen(inAnsi);                               \
    outWstr = (WCHAR *)alloca(sizeof(WCHAR)*(len + 1));         \
    AToWCopy(outWstr, len, inAnsi);                             \
    outWstr[len] = 0;                                           \
} while (0)
    
#define STYLE_MASK_ALL          0xffffffff
#define STYLE_MASK_READONLY     (WS_MINIMIZE | WS_MAXIMIZE)
#define STYLE_MASK_WRITABLE     (~STYLE_MASK_READONLY)

#define STYLE_FLAGS_RESIZABLE   WS_THICKFRAME

#define STYLE_FLAGS_UNDECORATED_OFF     (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define STYLE_FLAGS_UNDECORATED_ON      WS_POPUP

#define WINDOW_STYLE_EX         0

#define CLASS_NAME              L"RaylibWindow"

#define FLAG_MASK_OPTIONAL      (FLAG_VSYNC_HINT)
#define FLAG_MASK_REQUIRED      ~(FLAG_MASK_OPTIONAL)

// Flags that have no operations to perform during an update
#define FLAG_MASK_NO_UPDATE     (FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT)

#define WM_APP_UPDATE_WINDOW_SIZE (WM_APP + 1)

#define WGL_DRAW_TO_WINDOW_ARB              0x2001
#define WGL_ACCELERATION_ARB                0x2003
#define WGL_SUPPORT_OPENGL_ARB              0x2010
#define WGL_DOUBLE_BUFFER_ARB               0x2011
#define WGL_PIXEL_TYPE_ARB                  0x2013
#define WGL_COLOR_BITS_ARB                  0x2014
#define WGL_RED_BITS_ARB                    0x2015
#define WGL_RED_SHIFT_ARB                   0x2016
#define WGL_GREEN_BITS_ARB                  0x2017
#define WGL_GREEN_SHIFT_ARB                 0x2018
#define WGL_BLUE_BITS_ARB                   0x2019
#define WGL_BLUE_SHIFT_ARB                  0x201a
#define WGL_ALPHA_BITS_ARB                  0x201b
#define WGL_ALPHA_SHIFT_ARB                 0x201c
#define WGL_DEPTH_BITS_ARB                  0x2022
#define WGL_STENCIL_BITS_ARB                0x2023
#define WGL_TYPE_RGBA_ARB                   0x202b

// Context acceleration types
#define WGL_NO_ACCELERATION_ARB             0x2025      // OpenGL 1.1 GDI software rasterizer
#define WGL_GENERIC_ACCELERATION_ARB        0x2026
#define WGL_FULL_ACCELERATION_ARB           0x2027      // OpenGL hardware-accelerated, using GPU-drivers provided by vendor

// WGL_ARB_multisample extension supported
#define WGL_SAMPLE_BUFFERS_ARB              0x2041      // Multisampling: 1 if multisample buffers are supported
#define WGL_SAMPLES_ARB                     0x2042      // Multisampling: Number of samples per pixel (4, 8, 16)

// WGL_ARB_framebuffer_sRGB extension supported
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB    0x20a9      // GL_TRUE if the framebuffer can do sRGB conversion

#define WGL_NUMBER_PIXEL_FORMATS_ARB        0x2000
#define WGL_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB       0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB        0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB    0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { MIZED_NONE, MIZED_MIN, MIZED_MAX } Mized;

typedef enum {
    UPDATE_WINDOW_FIRST,
    UPDATE_WINDOW_NORMAL,
} UpdateWindowKind;

typedef enum {
    SANITIZE_FLAGS_FIRST,
    SANITIZE_FLAGS_NORMAL,
} SanitizeFlagsKind;

typedef struct {
    HMONITOR needle;
    int index;
    int matchIndex;
    RECT rect;
} FindMonitorContext;

typedef struct {
    DWORD set;
    DWORD clear;
} FlagsOp;

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
static size_t AToWLen(const char *a)
{
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, a, -1, NULL, 0);
    
    if (sizeNeeded < 0) TRACELOG(LOG_ERROR, "Failed to calculate wide length, result=%d, error=%u", sizeNeeded, GetLastError());

    return sizeNeeded;
}
static void AToWCopy(wchar_t *outPtr, size_t outLen, const char *a)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, a, -1, outPtr, (int)outLen);
    if (size != outLen) TRACELOG(LOG_WARNING, "WIN32: Convert %zu UTF-8 chars to WCHAR but converted %zu", outLen, size);
}

static bool DecoratedFromStyle(DWORD style)
{
    if (style & STYLE_FLAGS_UNDECORATED_ON)
    {
        if (style & STYLE_FLAGS_UNDECORATED_OFF) TRACELOG(LOG_ERROR, "FLAGS: Style 0x%x has both undecorated on/off flags", style);
        return false; // Not decorated
    }

    DWORD masked = (style & STYLE_FLAGS_UNDECORATED_OFF);
    if (STYLE_FLAGS_UNDECORATED_OFF != masked) TRACELOG(LOG_ERROR, "FLAGS: Style 0x%x is missing these flags 0x%x", masked, masked ^ STYLE_FLAGS_UNDECORATED_OFF);

    return true; // Decorated
}

static Mized MizedFromStyle(DWORD style)
{
    // minimized takes precedence over maximized
    if (style & WS_MINIMIZE) return MIZED_MIN;
    if (style & WS_MAXIMIZE) return MIZED_MAX;
    return MIZED_NONE;
}

static Mized MizedFromFlags(unsigned flags)
{
    // minimized takes precedence over maximized
    if (flags & FLAG_WINDOW_MINIMIZED) return MIZED_MIN;
    if (flags & FLAG_WINDOW_MAXIMIZED) return MIZED_MAX;
    return MIZED_NONE;
}

static DWORD MakeWindowStyle(unsigned flags)
{
    // we don't need this since we don't have any child windows, but I guess
    // it improves efficiency, plus, windows adds this flag automatically anyway
    // so it keeps our flags in sync with the OS
    DWORD style = WS_CLIPSIBLINGS ;
    
    style |= (flags & FLAG_WINDOW_HIDDEN)? 0 : WS_VISIBLE;
    style |= (flags & FLAG_WINDOW_RESIZABLE)? STYLE_FLAGS_RESIZABLE : 0;
    style |= (flags & FLAG_WINDOW_UNDECORATED)? STYLE_FLAGS_UNDECORATED_ON : STYLE_FLAGS_UNDECORATED_OFF;

    switch (MizedFromFlags(flags))
    {
        case MIZED_NONE: break;
        case MIZED_MIN: style |= WS_MINIMIZE; break;
        case MIZED_MAX: style |= WS_MAXIMIZE; break;
        default: break;
    }

    return style;
}

static bool IsMinimized2(HWND hwnd)
{
    bool isIconic = IsIconic(hwnd);
    bool styleMinimized = !!(WS_MINIMIZE & GetWindowLongPtrW(hwnd, GWL_STYLE));
    if (isIconic != styleMinimized) TRACELOG(LOG_WARNING, "IsIconic(%d) != WS_MINIMIZED(%d)", isIconic, styleMinimized);

    return isIconic;
}

// Enforces that the actual window/platform state is in sync with raylib's flags
static void CheckFlags(const char *context, HWND hwnd, DWORD flags, DWORD expectedStyle, DWORD styleCheckMask)
{
    //TRACELOG(LOG_INFO, "Verifying Flags 0x%x Style 0x%x Mask 0x%x", flags, expectedStyle & styleCheckMask, styleCheckMask);

    DWORD styleFromFlags = MakeWindowStyle(flags);
    if ((styleFromFlags & styleCheckMask) != (expectedStyle & styleCheckMask))
    {
        TRACELOG(LOG_ERROR, "%s: window flags (0x%x) produced style 0x%x which != expected 0x%x (diff=0x%x, mask=0x%x)",
            context, flags, styleFromFlags & styleCheckMask, expectedStyle & styleCheckMask,
            (styleFromFlags & styleCheckMask) ^ (expectedStyle & styleCheckMask), styleCheckMask);
    }

    SetLastError(0);
    LONG actualStyle = (LONG)GetWindowLongPtrW(hwnd, GWL_STYLE);
    if ((actualStyle & styleCheckMask) != (expectedStyle & styleCheckMask))
    {
        TRACELOG(LOG_ERROR, "%s: expected style 0x%x but got 0x%x (diff=0x%x, mask=0x%x, lasterror=%lu)",
            context, expectedStyle & styleCheckMask, actualStyle & styleCheckMask,
            (expectedStyle & styleCheckMask) ^ (actualStyle & styleCheckMask),
            styleCheckMask, GetLastError());
    }

    if (styleCheckMask & WS_MINIMIZE)
    {
        bool isIconic = IsIconic(hwnd);
        bool styleMinimized = !!(WS_MINIMIZE & actualStyle);
        if (isIconic != styleMinimized) TRACELOG(LOG_ERROR, "IsIconic(%d) != WS_MINIMIZED(%d)", isIconic, styleMinimized);
    }

    if (styleCheckMask & WS_MAXIMIZE)
    {
        WINDOWPLACEMENT placement;
        placement.length = sizeof(placement);
        if (!GetWindowPlacement(hwnd, &placement))
        {
            TRACELOG(LOG_ERROR, "%s failed, error=%lu", "GetWindowPlacement", GetLastError());
        }
        bool placementMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);
        bool styleMaximized = WS_MAXIMIZE & actualStyle;
        if (placementMaximized != styleMaximized)
        {
            TRACELOG(LOG_ERROR, "maximized state desync, placement maximized=%d (showCmd=%lu) style maximized=%d",
                placementMaximized, placement.showCmd, styleMaximized);
        }
    }
}

static SIZE PxFromPt2(float dpiScale, bool highdpiEnabled, int screenWidth, int screenHeight)
{
    // Get size in pixels from points
    return (SIZE){
        highdpiEnabled? (int)((float)screenWidth*dpiScale) : screenWidth,
        highdpiEnabled? (int)((float)screenHeight*dpiScale) : screenHeight,
    };
}

static SIZE GetClientSize(HWND hwnd)
{
    RECT rect = { 0 };
    
    if (GetClientRect(hwnd, &rect) == 0) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "GetClientRect", GetLastError());
    
    return (SIZE){ rect.right, rect.bottom };
}

static SIZE CalcWindowSize(UINT dpi, SIZE clientSize, DWORD style)
{
    RECT rect = { 0, 0, clientSize.cx, clientSize.cy };
    
    int result = AdjustWindowRectExForDpi(&rect, style, 0, WINDOW_STYLE_EX, dpi);
    
    if (result == 0) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "AdjustWindowRect", GetLastError());

    return (SIZE){ rect.right - rect.left, rect.bottom - rect.top };
}

// returns true if the window size was updated, false otherwise
static bool UpdateWindowSize(UpdateWindowKind kind, HWND hwnd, int width, int height, unsigned flags)
{
    if (flags & FLAG_WINDOW_MINIMIZED) return false;

    if (flags & FLAG_WINDOW_MAXIMIZED)
    {
        CheckFlags("UpdateWindowSize(maximized)", hwnd, flags, MakeWindowStyle(flags), STYLE_MASK_ALL);
        return false;
    }

    if (flags & FLAG_BORDERLESS_WINDOWED_MODE)
    {
        MONITORINFO info = { 0 };
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        info.cbSize = sizeof(info);
        if (!GetMonitorInfoW(monitor, &info)) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "GetMonitorInfo", GetLastError());

        RECT windowRect = { 0 };
        if (!GetWindowRect(hwnd, &windowRect)) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "GetWindowRect", GetLastError());

        if ((windowRect.left == info.rcMonitor.left) &&
            (windowRect.top == info.rcMonitor.top) &&
            ((windowRect.right - windowRect.left) == (info.rcMonitor.right - info.rcMonitor.left)) &&
            ((windowRect.bottom - windowRect.top) == (info.rcMonitor.bottom - info.rcMonitor.top))) return false;

        if (!SetWindowPos(hwnd, HWND_TOP,
            info.rcMonitor.left, info.rcMonitor.top,
            info.rcMonitor.right - info.rcMonitor.left,
            info.rcMonitor.bottom - info.rcMonitor.top,
            SWP_NOOWNERZORDER))
        {
            TRACELOG(LOG_ERROR, "%s failed, error=%lu", "SetWindowPos", GetLastError());
        }

        return true;
    }

    UINT dpi = GetDpiForWindow(hwnd);
    float dpiScale = ((float)dpi)/96.0f;
    bool dpiScaling = flags & FLAG_WINDOW_HIGHDPI;
    SIZE desired = PxFromPt2(dpiScale, dpiScaling, width, height);
    SIZE actual = GetClientSize(hwnd);
    if ((actual.cx == desired.cx) || (actual.cy == desired.cy)) return false;

    TRACELOG(LOG_INFO, "Restoring client size from [%dx%d] to [%dx%d] (dpi:%lu dpiScaling:%d app:%ix%i)",
        actual.cx, actual.cy, desired.cx, desired.cy, dpi, dpiScaling, width, height);

    SIZE windowSize = CalcWindowSize(dpi, desired, MakeWindowStyle(flags));
    POINT windowPos = (POINT){ 0, 0 };
    UINT swpFlags = SWP_NOZORDER | SWP_FRAMECHANGED;
    if (kind == UPDATE_WINDOW_FIRST)
    {
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        if (!monitor) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "MonitorFromWindow", GetLastError());

        MONITORINFO info;
        info.cbSize = sizeof(info);
        if (!GetMonitorInfoW(monitor, &info)) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "GetMonitorInfo", GetLastError());

        LONG monitorWidth = info.rcMonitor.right - info.rcMonitor.left;
        LONG monitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;
        windowPos = (POINT){
            MAX(0, (monitorWidth - windowSize.cx)/2),
            MAX(0, (monitorHeight - windowSize.cy)/2),
        };
    }
    else swpFlags |= SWP_NOMOVE;

    if (!SetWindowPos(hwnd, NULL, windowPos.x, windowPos.y, windowSize.cx, windowSize.cy, swpFlags))
    {
        TRACELOG(LOG_ERROR, "%s failed, error=%lu", "SetWindowPos", GetLastError());
    }

    return true;
}

static BOOL IsWindows10Version1703OrGreaterWin32(void)
{
    HMODULE ntdll = LoadLibraryW(L"ntdll.dll");
    
    DWORD (*Verify)(RTL_OSVERSIONINFOEXW*, ULONG, ULONGLONG) = 
        (DWORD (*)(RTL_OSVERSIONINFOEXW*, ULONG, ULONGLONG))GetProcAddress(ntdll, "RtlVerifyVersionInfo");
    if (!Verify)
    {
        TRACELOG(LOG_ERROR, "%s failed, error=%lu", "GetProcAddress 'RtlVerifyVersionInfo'", GetLastError());
        return 0;
    }

    RTL_OSVERSIONINFOEXW osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = 10;
    osvi.dwMinorVersion = 0;
    osvi.dwBuildNumber = 15063;  // Build 15063 corresponds to Windows 10 version 1703 (Creators Update)
    DWORDLONG cond = 0;
    VER_SET_CONDITION(cond, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(cond, VER_BUILDNUMBER, VER_GREATER_EQUAL);
    
    return 0 == (*Verify)(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, cond);
}

// Get OpenGL function pointers
static void *WglGetProcAddress(const char *procname)
{
    void *proc = (void *)wglGetProcAddress(procname);

    if ((proc == NULL) ||
        // NOTE: Some GPU drivers could return following
        // invalid sentinel values instead of NULL
        (proc == (void *)0x1) || 
        (proc == (void *)0x2) || 
        (proc == (void *)0x3) ||
        (proc == (void *)-1))
    {
        // TODO: Keep gl module pointer as global platform data?
        HMODULE glModule = LoadLibraryW(L"opengl32.dll");
        proc = (void *)GetProcAddress(glModule, procname);

        //if (proc == NULL) TRACELOG(LOG_ERROR, "GL: GetProcAddress() failed to get %s [%p], error=%u", procname, proc, GetLastError());
        //else TRACELOG(LOG_INFO, "GL: Found entry point for %s [%p]", procname, proc);
    }

    return proc;
}

static KeyboardKey KeyFromWparam(WPARAM wparam)
{
    switch (wparam)
    {
        /* case VK_LBUTTON: return KEY_; */
        /* case VK_RBUTTON: return KEY_; */
        /* case VK_CANCEL: return KEY_; */
        /* case VK_MBUTTON: return KEY_; */
        /* case VK_XBUTTON1: return KEY_; */
        /* case VK_XBUTTON2: return KEY_; */
        /* case VK_BACK: return KEY_; */
        /* case VK_TAB: return KEY_; */
        /* case VK_CLEAR: return KEY_; */
        case VK_RETURN: return KEY_ENTER;
        /* case VK_SHIFT: return KEY_; */
        /* case VK_CONTROL: return KEY_; */
        /* case VK_MENU: return KEY_; */
        /* case VK_PAUSE: return KEY_; */
        /* case VK_CAPITAL: return KEY_; */
        /* case VK_KANA: return KEY_; */
        /* case VK_HANGUL: return KEY_; */
        /* case VK_IME_ON: return KEY_; */
        /* case VK_JUNJA: return KEY_; */
        /* case VK_FINAL: return KEY_; */
        /* case VK_HANJA: return KEY_; */
        /* case VK_KANJI: return KEY_; */
        /* case VK_IME_OFF: return KEY_; */
        case VK_ESCAPE: return KEY_ESCAPE;
        /* case VK_CONVERT: return KEY_; */
        /* case VK_NONCONVERT: return KEY_; */
        /* case VK_ACCEPT: return KEY_; */
        /* case VK_MODECHANGE: return KEY_; */
        case VK_SPACE: return KEY_SPACE;
        /* case VK_PRIOR: return KEY_; */
        /* case VK_NEXT: return KEY_; */
        /* case VK_END: return KEY_; */
        /* case VK_HOME: return KEY_; */
        case VK_LEFT: return KEY_LEFT;
        case VK_UP: return KEY_UP;
        case VK_RIGHT: return KEY_RIGHT;
        case VK_DOWN: return KEY_DOWN;
        /* case VK_SELECT: return KEY_; */
        /* case VK_PRINT: return KEY_; */
        /* case VK_EXECUTE: return KEY_; */
        /* case VK_SNAPSHOT: return KEY_; */
        /* case VK_INSERT: return KEY_; */
        /* case VK_DELETE: return KEY_; */
        /* case VK_HELP: return KEY_; */
        case '0': return KEY_ZERO;
        case '1': return KEY_ONE;
        case '2': return KEY_TWO;
        case '3': return KEY_THREE;
        case '4': return KEY_FOUR;
        case '5': return KEY_FIVE;
        case '6': return KEY_SIX;
        case '7': return KEY_SEVEN;
        case '8': return KEY_EIGHT;
        case '9': return KEY_NINE;
        /* case 0x3A-40: return KEY_; */
        case 'A': return KEY_A;
        case 'B': return KEY_B;
        case 'C': return KEY_C;
        case 'D': return KEY_D;
        case 'E': return KEY_E;
        case 'F': return KEY_F;
        case 'G': return KEY_G;
        case 'H': return KEY_H;
        case 'I': return KEY_I;
        case 'J': return KEY_J;
        case 'K': return KEY_K;
        case 'L': return KEY_L;
        case 'M': return KEY_M;
        case 'N': return KEY_N;
        case 'O': return KEY_O;
        case 'P': return KEY_P;
        case 'Q': return KEY_Q;
        case 'R': return KEY_R;
        case 'S': return KEY_S;
        case 'T': return KEY_T;
        case 'U': return KEY_U;
        case 'V': return KEY_V;
        case 'W': return KEY_W;
        case 'X': return KEY_X;
        case 'Y': return KEY_Y;
        case 'Z': return KEY_Z;
        /* case VK_LWIN: return KEY_; */
        /* case VK_RWIN: return KEY_; */
        /* case VK_APPS: return KEY_; */
        /* case VK_SLEEP: return KEY_; */
        /* case VK_NUMPAD0: return KEY_; */
        /* case VK_NUMPAD1: return KEY_; */
        /* case VK_NUMPAD2: return KEY_; */
        /* case VK_NUMPAD3: return KEY_; */
        /* case VK_NUMPAD4: return KEY_; */
        /* case VK_NUMPAD5: return KEY_; */
        /* case VK_NUMPAD6: return KEY_; */
        /* case VK_NUMPAD7: return KEY_; */
        /* case VK_NUMPAD8: return KEY_; */
        /* case VK_NUMPAD9: return KEY_; */
        /* case VK_MULTIPLY: return KEY_; */
        /* case VK_ADD: return KEY_; */
        /* case VK_SEPARATOR: return KEY_; */
        /* case VK_SUBTRACT: return KEY_; */
        /* case VK_DECIMAL: return KEY_; */
        /* case VK_DIVIDE: return KEY_; */
        /* case VK_F1: return KEY_; */
        /* case VK_F2: return KEY_; */
        /* case VK_F3: return KEY_; */
        /* case VK_F4: return KEY_; */
        /* case VK_F5: return KEY_; */
        /* case VK_F6: return KEY_; */
        /* case VK_F7: return KEY_; */
        /* case VK_F8: return KEY_; */
        /* case VK_F9: return KEY_; */
        /* case VK_F10: return KEY_; */
        /* case VK_F11: return KEY_; */
        /* case VK_F12: return KEY_; */
        /* case VK_F13: return KEY_; */
        /* case VK_F14: return KEY_; */
        /* case VK_F15: return KEY_; */
        /* case VK_F16: return KEY_; */
        /* case VK_F17: return KEY_; */
        /* case VK_F18: return KEY_; */
        /* case VK_F19: return KEY_; */
        /* case VK_F20: return KEY_; */
        /* case VK_F21: return KEY_; */
        /* case VK_F22: return KEY_; */
        /* case VK_F23: return KEY_; */
        /* case VK_F24: return KEY_; */
        /* case VK_NUMLOCK: return KEY_; */
        /* case VK_SCROLL: return KEY_; */
        /* case VK_LSHIFT: return KEY_; */
        /* case VK_RSHIFT: return KEY_; */
        /* case VK_LCONTROL: return KEY_; */
        /* case VK_RCONTROL: return KEY_; */
        /* case VK_LMENU: return KEY_; */
        /* case VK_RMENU: return KEY_; */
        /* case VK_BROWSER_BACK: return KEY_; */
        /* case VK_BROWSER_FORWARD: return KEY_; */
        /* case VK_BROWSER_REFRESH: return KEY_; */
        /* case VK_BROWSER_STOP: return KEY_; */
        /* case VK_BROWSER_SEARCH: return KEY_; */
        /* case VK_BROWSER_FAVORITES: return KEY_; */
        /* case VK_BROWSER_HOME: return KEY_; */
        /* case VK_VOLUME_MUTE: return KEY_; */
        /* case VK_VOLUME_DOWN: return KEY_; */
        /* case VK_VOLUME_UP: return KEY_; */
        /* case VK_MEDIA_NEXT_TRACK: return KEY_; */
        /* case VK_MEDIA_PREV_TRACK: return KEY_; */
        /* case VK_MEDIA_STOP: return KEY_; */
        /* case VK_MEDIA_PLAY_PAUSE: return KEY_; */
        /* case VK_LAUNCH_MAIL: return KEY_; */
        /* case VK_LAUNCH_MEDIA_SELECT: return KEY_; */
        /* case VK_LAUNCH_APP1: return KEY_; */
        /* case VK_LAUNCH_APP2: return KEY_; */
        /* case VK_OEM_1: return KEY_; */
        /* case VK_OEM_PLUS: return KEY_; */
        /* case VK_OEM_COMMA: return KEY_; */
        /* case VK_OEM_MINUS: return KEY_; */
        /* case VK_OEM_PERIOD: return KEY_; */
        /* case VK_OEM_2: return KEY_; */
        /* case VK_OEM_3: return KEY_; */
        /* case VK_OEM_4: return KEY_; */
        /* case VK_OEM_5: return KEY_; */
        /* case VK_OEM_6: return KEY_; */
        /* case VK_OEM_7: return KEY_; */
        /* case VK_OEM_8: return KEY_; */
        /* case VK_OEM_102: return KEY_; */
        /* case VK_PROCESSKEY: return KEY_; */
        /* case VK_PACKET: return KEY_; */
        /* case VK_ATTN: return KEY_; */
        /* case VK_CRSEL: return KEY_; */
        /* case VK_EXSEL: return KEY_; */
        /* case VK_EREOF: return KEY_; */
        /* case VK_PLAY: return KEY_; */
        /* case VK_ZOOM: return KEY_; */
        /* case VK_NONAME: return KEY_; */
        /* case VK_PA1: return KEY_; */
        /* case VK_OEM_CLEAR: return KEY_; */
        default: return KEY_NULL;
    }
}

static LPCWSTR GetCursorName(int cursor)
{
    LPCWSTR name = (LPCWSTR)IDC_ARROW;

    switch (cursor)
    {
        case MOUSE_CURSOR_DEFAULT: name = (LPCWSTR)IDC_ARROW; break;
        case MOUSE_CURSOR_ARROW: name = (LPCWSTR)IDC_ARROW; break;
        case MOUSE_CURSOR_IBEAM: name = (LPCWSTR)IDC_IBEAM; break;
        case MOUSE_CURSOR_CROSSHAIR: name = (LPCWSTR)IDC_CROSS; break;
        case MOUSE_CURSOR_POINTING_HAND: name = (LPCWSTR)IDC_HAND; break;
        case MOUSE_CURSOR_RESIZE_EW: name = (LPCWSTR)IDC_SIZEWE; break;
        case MOUSE_CURSOR_RESIZE_NS: name = (LPCWSTR)IDC_SIZENS; break;
        case MOUSE_CURSOR_RESIZE_NWSE: name = (LPCWSTR)IDC_SIZENWSE; break;
        case MOUSE_CURSOR_RESIZE_NESW: name = (LPCWSTR)IDC_SIZENESW; break;
        case MOUSE_CURSOR_RESIZE_ALL: name = (LPCWSTR)IDC_SIZEALL; break;
        case MOUSE_CURSOR_NOT_ALLOWED: name = (LPCWSTR)IDC_NO; break;
        default: break;
    }

    return name;
}

static BOOL CALLBACK CountMonitorsProc(HMONITOR handle, HDC _, LPRECT rect, LPARAM lparam)
{
    int *count = (int *)lparam;
    *count += 1;
    // Always return TRUE to continue the loop, otherwise, the caller
    // can't distinguish between stopping the loop and an error
    return TRUE;
}

static BOOL CALLBACK FindMonitorProc(HMONITOR handle, HDC _, LPRECT rect, LPARAM lparam)
{
    FindMonitorContext *c = (FindMonitorContext*)lparam;
    if (handle == c->needle)
    {
        c->matchIndex = c->index;
        c->rect = *rect;
    }

    c->index += 1;
    // Always return TRUE to continue the loop, otherwise, the caller
    // can't distinguish between stopping the loop and an error
    return TRUE;
}

static void GetStyleChangeFlagOps(DWORD coreWindowFlags, STYLESTRUCT *ss, FlagsOp *deferredFlags)
{
    // Check window resizable flag change
    bool resizable = (coreWindowFlags & FLAG_WINDOW_RESIZABLE);
    bool resizableOld = ((ss->styleOld & STYLE_FLAGS_RESIZABLE) != 0);
    bool resizableNew = ((ss->styleNew & STYLE_FLAGS_RESIZABLE) != 0);
    if (resizable != resizableOld) TRACELOG(LOG_ERROR, "expected resizable %u but got %u", resizable, resizableOld);
    if (resizableOld != resizableNew)
    {
        if (resizableNew) deferredFlags->set |= FLAG_WINDOW_RESIZABLE;
        else deferredFlags->clear |= FLAG_WINDOW_RESIZABLE;
    }

    // Check window decorated flag change
    bool decorated = (0 == (coreWindowFlags & FLAG_WINDOW_UNDECORATED));
    bool decoratedOld = DecoratedFromStyle(ss->styleOld);
    bool decoratedNew = DecoratedFromStyle(ss->styleNew);
    if (decorated != decoratedOld) TRACELOG(LOG_ERROR, "expected decorated %u but got %u", decorated, decoratedOld);
    if (decoratedOld != decoratedNew)
    {
        if (decoratedNew) deferredFlags->clear |= FLAG_WINDOW_UNDECORATED;
        else deferredFlags->set |= FLAG_WINDOW_UNDECORATED;
    }

    // Check window hidden flag change
    bool hidden = (coreWindowFlags & FLAG_WINDOW_HIDDEN);
    bool hiddenOld = ((ss->styleOld & WS_VISIBLE) == 0);
    bool hiddenNew = ((ss->styleNew & WS_VISIBLE) == 0);
    if (hidden != hiddenOld) TRACELOG(LOG_ERROR, "expected hidden %u but got %u", hidden, hiddenOld);
    if (hiddenOld != hiddenNew)
    {
        if (hiddenNew) deferredFlags->set |= FLAG_WINDOW_HIDDEN;
        else deferredFlags->clear |= FLAG_WINDOW_HIDDEN;
    }
}

// Call when the window is rezised, returns true if the new window size should update the desired app size
static bool AdoptWindowResize(unsigned flags)
{
    if (flags & FLAG_WINDOW_MINIMIZED) return false;
    if (flags & FLAG_WINDOW_MAXIMIZED) return false;
    if (flags & FLAG_FULLSCREEN_MODE) return false;
    if (flags & FLAG_BORDERLESS_WINDOWED_MODE) return false;
    if (!(flags & FLAG_WINDOW_RESIZABLE)) return false;
    
    return true;
}

// ---------------------------------------------------------------------------------------------
// Here's the end of the "pure function section", the rest of the file can access global state
// ---------------------------------------------------------------------------------------------

// Unlock the ability to use CORE in the rest of the file
#undef CORE

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);             // Initialize platform (graphics, inputs and more)
void ClosePlatform(void);           // Close platform

// Win32 process messages management function
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// Win32: Handle inputs functions
static void HandleKey(WPARAM wparam, LPARAM lparam, char state);
static void HandleMouseButton(int button, char state);
static void HandleRawInput(LPARAM lparam);
static void HandleWindowResize(HWND hwnd, int *width, int *height);

static void UpdateWindowStyle(HWND hwnd, unsigned desiredFlags);
static unsigned SanitizeFlags(SanitizeFlagsKind kind, unsigned flags);
static void UpdateFlags(HWND hwnd, unsigned desiredFlags, int width, int height); // Update window flags

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// Check if application should close
bool WindowShouldClose(void)
{
    return CORE.Window.shouldClose;
}

// Toggle fullscreen mode
void ToggleFullscreen(void)
{
    TRACELOG(LOG_WARNING, "ToggleFullscreen not implemented");
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    if (CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE)
    {
        ClearWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
    }
    else
    {
        SetWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
    }
}

// Set window state: maximized, if resizable
void MaximizeWindow(void)
{
    SetWindowState(FLAG_WINDOW_MAXIMIZED);
}

// Set window state: minimized
void MinimizeWindow(void)
{
    SetWindowState(FLAG_WINDOW_MINIMIZED);
}

// Restore window from being minimized/maximized
void RestoreWindow(void)
{
    if ((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) &&
        (CORE.Window.flags & FLAG_WINDOW_MINIMIZED)
    ) {
        ClearWindowState(FLAG_WINDOW_MINIMIZED);
    }
    else
    {
        ClearWindowState(FLAG_WINDOW_MINIMIZED|FLAG_WINDOW_MAXIMIZED);
    }
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    platform.desiredFlags = SanitizeFlags(SANITIZE_FLAGS_NORMAL, CORE.Window.flags | flags);
    UpdateFlags(platform.hwnd, platform.desiredFlags, platform.appScreenWidth, platform.appScreenHeight);
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    platform.desiredFlags = SanitizeFlags(SANITIZE_FLAGS_NORMAL, CORE.Window.flags & ~flags);
    UpdateFlags(platform.hwnd, platform.desiredFlags, platform.appScreenWidth, platform.appScreenHeight);
}

// Set icon for window
void SetWindowIcon(Image image)
{
    TRACELOG(LOG_WARNING, "SetWindowIcon not implemented");
}

// Set icon for window
void SetWindowIcons(Image *images, int count)
{
    TRACELOG(LOG_WARNING, "SetWindowIcons not implemented");
}

void SetWindowTitle(const char *title)
{
    CORE.Window.title = title;
    
    WCHAR *titleWide = NULL;
    A_TO_W_ALLOCA(titleWide, CORE.Window.title);
    
    int result = SetWindowTextW(platform.hwnd, titleWide);
    if (result == 0) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "SetWindowText", GetLastError());
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
    TRACELOG(LOG_WARNING, "SetWindowPosition not implemented");
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    TRACELOG(LOG_WARNING, "SetWindowMonitor not implemented");
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    TRACELOG(LOG_WARNING, "SetWindowMinSize not implemented");

    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    TRACELOG(LOG_WARNING, "SetWindowMaxSize not implemented");

    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    TRACELOG(LOG_WARNING, "SetWindowSize not implemented");
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    TRACELOG(LOG_WARNING, "SetWindowOpacity not implemented");
}

// Set window focused
void SetWindowFocused(void)
{
    TRACELOG(LOG_WARNING, "SetWindowFocused not implemented");
}

// Get native window handle
void *GetWindowHandle(void)
{
    return platform.hwnd;
}

int GetMonitorCount(void)
{
    int count = 0;
    
    int result = EnumDisplayMonitors(NULL, NULL, CountMonitorsProc, (LPARAM)&count);
    if (result == 0) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "EnumDisplayMonitors", GetLastError());

    return count;
}

// Get current monitor where window is placed
int GetCurrentMonitor(void)
{
    HMONITOR monitor = MonitorFromWindow(platform.hwnd, MONITOR_DEFAULTTOPRIMARY);
    if (!monitor) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "MonitorFromWindow", GetLastError());

    FindMonitorContext context;
    context.needle = monitor;
    context.index = 0;
    context.matchIndex = -1;
    
    int result = EnumDisplayMonitors(NULL, NULL, FindMonitorProc, (LPARAM)&context);
    if (result == 0) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "EnumDisplayMonitors", GetLastError());

    return context.matchIndex;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPosition not implemented");
    return (Vector2){ 0, 0 };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorWidth not implemented");
    return 0;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorHeight not implemented");
    return 0;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPhysicalWidth not implemented");
    return 0;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPhysicalHeight not implemented");
    return 0;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorRefreshRate not implemented");
    return 0;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorName not implemented");
    return 0;
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    TRACELOG(LOG_WARNING, "GetWindowPosition not implemented");
    return (Vector2){ 0, 0 };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    float scale = ((float)GetDpiForWindow(platform.hwnd))/96.0f;
    return (Vector2){ scale, scale };
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
    TRACELOG(LOG_WARNING, "SetClipboardText not implemented");
}

// Get clipboard text content
const char *GetClipboardText(void)
{
    TRACELOG(LOG_WARNING, "GetClipboardText not implemented");
    return NULL;
}

// Get clipboard image
Image GetClipboardImage(void)
{
    Image image = { 0 };
    
    TRACELOG(LOG_WARNING, "GetClipboardText not implemented");

    return image;
}

// Show mouse cursor
void ShowCursor(void)
{
    CORE.Input.Mouse.cursorHidden = false;
    SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_ARROW));
}

// Hides mouse cursor
void HideCursor(void)
{
    // NOTE: we use SetCursor instead of ShowCursor because it makes it easy
    // to only hide the cursor while it's inside the client area
    CORE.Input.Mouse.cursorHidden = true;
    SetCursor(NULL);
}

// Enables cursor (unlock cursor)
void EnableCursor(void)
{
    if (platform.cursorEnabled) TRACELOG(LOG_INFO, "EnableCursor: already enabled");
    else
    {
        if (!ClipCursor(NULL)) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "ClipCursor", GetLastError());

        RAWINPUTDEVICE rid = { 0 };
        rid.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
        rid.usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
        rid.dwFlags = RIDEV_REMOVE; // Add to this window even in background
        rid.hwndTarget = NULL;
        int result = RegisterRawInputDevices(&rid, 1, sizeof(rid));
        if (result == 0) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "RegisterRawInputDevices", GetLastError());

        ShowCursor();
        platform.cursorEnabled = true;
        TRACELOG(LOG_INFO, "EnableCursor: enabled");
    }
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    if (platform.cursorEnabled)
    {
        RAWINPUTDEVICE rid = { 0 };
        rid.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
        rid.usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
        rid.dwFlags = RIDEV_INPUTSINK; // Add to this window even in background
        rid.hwndTarget = platform.hwnd;
        int result = RegisterRawInputDevices(&rid, 1, sizeof(rid));
        if (result == 0) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "RegisterRawInputDevices", GetLastError());

        RECT clientRect = { 0 };
        if (!GetClientRect(platform.hwnd, &clientRect)) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "GetClientRect", GetLastError());

        POINT topleft = { clientRect.left, clientRect.top };
        if (!ClientToScreen(platform.hwnd, &topleft)) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "ClientToScreen", GetLastError());

        LONG width = clientRect.right - clientRect.left;
        LONG height = clientRect.bottom - clientRect.top;

        TRACELOG(LOG_INFO, "ClipCursor client %d,%d %d,%d (topleft %d,%d)",
            clientRect.left, clientRect.top, clientRect.right, clientRect.bottom, topleft.x, topleft.y);
            
        LONG centerX = topleft.x + width/2;
        LONG centerY = topleft.y + height/2;
        RECT clipRect = { centerX, centerY, centerX + 1, centerY + 1 };
        if (!ClipCursor(&clipRect)) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "ClipCursor", GetLastError());

        CORE.Input.Mouse.previousPosition = (Vector2){ 0, 0 };
        CORE.Input.Mouse.currentPosition = (Vector2){ 0, 0 };
        HideCursor();

        platform.cursorEnabled = false;
        TRACELOG(LOG_INFO, "DisableCursor: disabled");
    }
    else TRACELOG(LOG_INFO, "DisableCursor: already disabled");
}

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
    if (!platform.hdc) abort();
    if (!SwapBuffers(platform.hdc)) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "SwapBuffers", GetLastError());
    if (!ValidateRect(platform.hwnd, NULL)) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "ValidateRect", GetLastError());
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds
double GetTime(void)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - CORE.Time.base)/(double)platform.timerFrequency.QuadPart;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given
// A user could craft a malicious string performing another action
// Only call this function yourself not with user input or make sure to check the string yourself
// Ref: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    // Security check to (partially) avoid malicious code on target platform
    if (strchr(url, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else
    {
        TRACELOG(LOG_WARNING, "OpenURL not implemented");
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings)
{
    TRACELOG(LOG_WARNING, "SetGamepadMappings not implemented");

    return -1;
}

// Set gamepad vibration
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration)
{
    TRACELOG(LOG_WARNING, "SetGamepadVibration not implemented");
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    if (platform.cursorEnabled)
    {
        CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
        CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
        TRACELOG(LOG_WARNING, "SetMousePosition not implemented");
    }
    else TRACELOG(LOG_WARNING, "INPUT: MOUSE: Cursor not enabled");
}

// Set mouse cursor
void SetMouseCursor(int cursor)
{
    LPCWSTR cursorName = GetCursorName(cursor);
    HCURSOR hcursor = LoadCursorW(NULL, cursorName);
    if (!hcursor) TRACELOG(LOG_ERROR, "LoadCursor %d (win32 %d) failed, error=%lu", cursor, (size_t)cursorName, GetLastError());

    SetCursor(hcursor);
    CORE.Input.Mouse.cursorHidden = false;
}

// Get physical key name
const char *GetKeyName(int key)
{
    TRACELOG(LOG_WARNING, "GetKeyName not implemented");
    return NULL;
}

// Register all input events
void PollInputEvents(void)
{
    // Reset keys/chars pressed registered
    CORE.Input.Keyboard.keyPressedQueueCount = 0;
    CORE.Input.Keyboard.charPressedQueueCount = 0;

    // Reset key repeats
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++) CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;

    // Reset last gamepad button/axis registered state
    CORE.Input.Gamepad.lastButtonPressed = 0; // GAMEPAD_BUTTON_UNKNOWN
    //CORE.Input.Gamepad.axisCount = 0;

    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    // TODO: It resets on target platform the mouse position and not filled again until a move-event,
    // so, if mouse is not moved it returns a (0, 0) position... this behaviour should be reviewed!
    //for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.position[i] = (Vector2){ 0, 0 };

    memcpy(CORE.Input.Keyboard.previousKeyState, CORE.Input.Keyboard.currentKeyState, sizeof(CORE.Input.Keyboard.previousKeyState));
    memset(CORE.Input.Keyboard.keyRepeatInFrame, 0, sizeof(CORE.Input.Keyboard.keyRepeatInFrame));

    // Register previous mouse wheel state
    CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
    CORE.Input.Mouse.currentWheelMove = (Vector2){ 0.0f, 0.0f };

    // Register previous mouse position
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    // Process windows messages
    MSG msg = { 0 };
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_PAINT) return;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize modern OpenGL context
// NOTE: We need to create a dummy context first to query requried extensions
HGLRC InitOpenGL(HWND hwnd, HDC hdc)
{
    // First, create a dummy context to get WGL extensions
    PIXELFORMATDESCRIPTOR pixelFormatDesc = {
        .nSize = sizeof(PIXELFORMATDESCRIPTOR),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .cDepthBits = 24,
        .iLayerType = PFD_MAIN_PLANE
    };

    int pixelFormat = ChoosePixelFormat(hdc, &pixelFormatDesc);
    SetPixelFormat(hdc, pixelFormat, &pixelFormatDesc);
    //int pixelFormat = ChoosePixelFormat(platform.hdc, &pixelFormatDesc);
    //if (!pixelFormat) { TRACELOG(LOG_ERROR, "%s failed, error=%lu", "ChoosePixelFormat", GetLastError()); return -1; }
    //if (!SetPixelFormat(platform.hdc, pixelFormat, &pixelFormatDesc)) { TRACELOG(LOG_ERROR, "%s failed, error=%lu", "SetPixelFormat", GetLastError()); return -1; }

    HGLRC tempContext = wglCreateContext(hdc);
    //if (!tempContext) { TRACELOG(LOG_ERROR, "%s failed, error=%lu", "wglCreateContext", GetLastError()); return -1; }
    BOOL result = wglMakeCurrent(hdc, tempContext);
    //if (!result) { TRACELOG(LOG_ERROR, "%s failed, error=%lu", "wglMakeCurrent", GetLastError()); return -1; }

    // Load WGL extension entry points
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

    // Setup modern pixel format if extension is available
    if (wglChoosePixelFormatARB)
    {
        int pixelFormatAttribs[] = {
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 32,
            //WGL_RED_BITS_ARB, 8,
            //WGL_GREEN_BITS_ARB, 8,
            //WGL_BLUE_BITS_ARB, 8,
            //WGL_ALPHA_BITS_ARB, 8,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            0 // Terminator
        };

        int format = 0;
        UINT numFormats = 0;
        if (wglChoosePixelFormatARB(hdc, pixelFormatAttribs, NULL, 1, &format, &numFormats) && (numFormats > 0))
        {
            PIXELFORMATDESCRIPTOR newPixelFormatDescriptor = { 0 };
            DescribePixelFormat(hdc, format, sizeof(newPixelFormatDescriptor), &newPixelFormatDescriptor);
            SetPixelFormat(hdc, format, &newPixelFormatDescriptor);
        }
    }

    // Create real modern OpenGL context (3.3 core)
    HGLRC realContext = NULL;
    if (wglCreateContextAttribsARB)
    {
        int contextAttribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, // WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, WGL_CONTEXT_ES_PROFILE_BIT_EXT (if supported)
            //WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB [glDebugMessageCallback()]
            0 // Terminator
        };

        // NOTE: We are not sharing context resources so, second parameters is NULL
        realContext = wglCreateContextAttribsARB(hdc, NULL, contextAttribs);

        // Check for error context creation errors
        // ERROR_INVALID_VERSION_ARB (0x2095)
        // ERROR_INVALID_PROFILE_ARB (0x2096)
        if (realContext == NULL) TRACELOG(LOG_ERROR, "GL: Error creating requested context: %lu", GetLastError());
    }

    // Cleanup dummy temp context
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempContext);

    // Activate real context
    if (realContext) wglMakeCurrent(hdc, realContext);

    // Once we got a real modern OpenGL context,
    // we can load required extensions (function pointers)
    rlLoadExtensions(WglGetProcAddress);

    return realContext;
}

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    platform.desiredFlags = SanitizeFlags(SANITIZE_FLAGS_FIRST, CORE.Window.flags);
    platform.appScreenWidth = CORE.Window.screen.width;
    platform.appScreenHeight = CORE.Window.screen.height;

    // NOTE: From this point CORE.Window.flags should always reflect the actual state of the window
    CORE.Window.flags = FLAG_WINDOW_HIDDEN | (platform.desiredFlags & FLAG_MASK_NO_UPDATE);

/*
    // TODO: Review SetProcessDpiAwarenessContext()
    // NOTE: SetProcessDpiAwarenessContext() requires Windows 10, version 1703 and shcore.lib linkage
    if (IsWindows10Version1703OrGreaterWin32())
    {
        TRACELOG(LOG_INFO, "DpiAware: >=Win10Creators");
        if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) 
            TRACELOG(LOG_ERROR, "%s failed, error %u", "SetProcessDpiAwarenessContext", GetLastError());
    }
    else
    {
        TRACELOG(LOG_INFO, "DpiAware: <Win10Creators");
        HRESULT hr = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        if (hr < 0) TRACELOG(LOG_ERROR, "%s failed, hresult=0x%lx", "SetProcessDpiAwareness", (DWORD)hr);
    }
*/

    WNDCLASSEXW windowClass = {
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = WndProc,                         // Custom procedure assigned
        .cbWndExtra = sizeof(LONG_PTR),                 // extra space for the Tuple object ptr
        .hInstance = GetModuleHandleW(0),
        .hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW), // TODO: Audit if we want to set this since we're implementing WM_SETCURSOR
        .lpszClassName = CLASS_NAME //L"GLWindowClass";
    };

    // Register window class
    if (RegisterClassExW(&windowClass) == 0) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "RegisterClass", GetLastError());

/*
    // TODO: Remove or move this code that sets the display size; should maybe go somewhere in WndProc?
    POINT primaryTopLeft = { 0 };
    HMONITOR monitor = MonitorFromPoint(primaryTopLeft, MONITOR_DEFAULTTOPRIMARY);
    if (monitor != NULL)
    {
        MONITORINFO info;
        info.cbSize = sizeof(info);
        if (!GetMonitorInfoW(monitor, &info)) TRACELOG(LOG_WARNING, "%s failed, error: %u", "GetMonitorInfo", GetLastError());
        else
        {
            CORE.Window.display.width = info.rcMonitor.right - info.rcMonitor.left;
            CORE.Window.display.height = info.rcMonitor.bottom - info.rcMonitor.top;
        }
    }
    else TRACELOG(LOG_WARNING, "MonitorFromPoint, error: %s", GetLastError()); 
*/
    
    // Create window
    // NOTE: Title string needs to be converted to WCHAR
    WCHAR *titleWide = NULL;
    A_TO_W_ALLOCA(titleWide, CORE.Window.title);
    
    // Create window and get handle
    platform.hwnd = CreateWindowExW(
        WINDOW_STYLE_EX,
        CLASS_NAME,
        titleWide,
        MakeWindowStyle(CORE.Window.flags),     // WS_OVERLAPPEDWINDOW | WS_VISIBLE
        CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0,                                   // Window size [width, height], needs to be updated
        NULL, NULL,
        GetModuleHandleW(NULL), NULL);

    if (!platform.hwnd)
    {
        TRACELOG(LOG_ERROR, "%s failed, error=%lu", "CreateWindow", GetLastError());
        return -1;
    }

    // Get handle to device drawing context
    // NOTE: Windows GDI object that represents a drawing surface
    platform.hdc = GetDC(platform.hwnd);

    // TODO: Support software rendering (instead of hardware-accelerated OpenGL context)

    // Init OpenGL modern context
    platform.glContext = InitOpenGL(platform.hwnd, platform.hdc);

    CORE.Window.ready = true;
    
    // TODO: Should this function be called before or after drawing context is created?
    UpdateWindowSize(UPDATE_WINDOW_FIRST, platform.hwnd, platform.appScreenWidth, platform.appScreenHeight, platform.desiredFlags);
    UpdateFlags(platform.hwnd, platform.desiredFlags, platform.appScreenWidth, platform.appScreenHeight);

    CORE.Window.currentFbo.width = CORE.Window.render.width;
    CORE.Window.currentFbo.height = CORE.Window.render.height;
    TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
    TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
    TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);

    CORE.Storage.basePath = GetWorkingDirectory();

    LARGE_INTEGER time = { 0 };
    QueryPerformanceCounter(&time);
    QueryPerformanceFrequency(&platform.timerFrequency);
    CORE.Time.base = time.QuadPart;

    InitTimer();
    
    // TODO: Enable cursor? -> Use default value as 0
    platform.cursorEnabled = true;

    TRACELOG(LOG_INFO, "PLATFORM: DESKTOP: WIN32: Initialized successfully");
    
    return 0;
}

// Close platform
void ClosePlatform(void)
{
    if (platform.hwnd)
    {
        int result = DestroyWindow(platform.hwnd);
        if (result == 0) TRACELOG(LOG_WARNING, "WIN32: Error on window destroy: %u", GetLastError());
        platform.hwnd = NULL;
    }
}

// Window procedure, message processing callback
// NOTE: All window event messages are processed here
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;

    // Sanity check
    DWORD mask = STYLE_MASK_ALL;
    if (platform.hwnd == hwnd)
    {
        if (msg == WM_WINDOWPOSCHANGING) mask &= ~(WS_MINIMIZE | WS_MAXIMIZE);
        CheckFlags("WndProc", hwnd, CORE.Window.flags, MakeWindowStyle(CORE.Window.flags), mask);
    }

    FlagsOp flagsOp = { 0 };
    FlagsOp *deferredFlags = &flagsOp;
        
    // Message processing
    //------------------------------------------------------------------------------------
    switch (msg)
    {
        case WM_CREATE:
        {
            // WARNING: Not recommended to do OpenGL intialization at this point

        } break;
        //case WM_ACTIVATE
        case WM_DESTROY:
        {
            // Clean up for window destruction
            wglMakeCurrent(platform.hdc, NULL);
            if (platform.glContext)
            {
                if (!wglDeleteContext(platform.glContext)) abort();
                platform.glContext = NULL;
            }

            if (platform.hdc)
            {
                if (!ReleaseDC(hwnd, platform.hdc)) abort();
                platform.hdc = NULL;
            }

        } break;
        case WM_CLOSE: CORE.Window.shouldClose = true; break; // Window close button [x], ALT+F4
        //case WM_QUIT: // Application closing, not related to window
        case WM_KILLFOCUS:
        {
            memset(CORE.Input.Keyboard.previousKeyState, 0, sizeof(CORE.Input.Keyboard.previousKeyState));
            memset(CORE.Input.Keyboard.currentKeyState, 0, sizeof(CORE.Input.Keyboard.currentKeyState));
        } break;
        case WM_SIZING:
        {
            if (CORE.Window.flags & FLAG_WINDOW_RESIZABLE)
            {
                // TODO: Enforce min/max size
            } 
            else TRACELOG(LOG_WARNING, "WINDOW: Trying to resize a non-resizable window");
            
            result = TRUE;
        } break;
        case WM_STYLECHANGING:
        {
            if (wparam == GWL_STYLE)
            {
                STYLESTRUCT *ss = (STYLESTRUCT *)lparam;
                GetStyleChangeFlagOps(CORE.Window.flags, ss, deferredFlags);

                UINT dpi = GetDpiForWindow(hwnd);
                SIZE clientSize = GetClientSize(hwnd);
                SIZE oldSize = CalcWindowSize(dpi, clientSize, ss->styleOld);
                SIZE newSize = CalcWindowSize(dpi, clientSize, ss->styleNew);
                
                if (oldSize.cx != newSize.cx || oldSize.cy != newSize.cy)
                {
                    TRACELOG(LOG_INFO, "resize from style change: %dx%d to %dx%d", oldSize.cx, oldSize.cy, newSize.cx, newSize.cy);
                    
                    if (CORE.Window.flags & FLAG_WINDOW_MAXIMIZED)
                    {
                        // looks like windows will automatically "unminimize" a window
                        // if a style changes modifies it's size
                        TRACELOG(LOG_INFO, "style change modifed window size, removing maximized flag");
                        deferredFlags->clear |= FLAG_WINDOW_MAXIMIZED;
                    }
                }
            }
        } break;
        case WM_WINDOWPOSCHANGING:
        {
            WINDOWPOS *pos = (WINDOWPOS *)lparam;
            if (pos->flags & SWP_SHOWWINDOW)
            {
                //if (pos->flags & SWP_HIDEWINDOW) abort();
                deferredFlags->clear |= FLAG_WINDOW_HIDDEN;
            }
            else if (pos->flags & SWP_HIDEWINDOW)
            {
                deferredFlags->set |= FLAG_WINDOW_HIDDEN;
            }

            Mized mized = MIZED_NONE;
            bool isIconic = IsIconic(hwnd);
            bool styleMinimized = !!(WS_MINIMIZE & GetWindowLongPtrW(hwnd, GWL_STYLE));
            if (isIconic != styleMinimized) TRACELOG(LOG_WARNING, "IsIconic(%d) != WS_MINIMIZED(%d)", isIconic, styleMinimized);
            
            if (isIconic) mized = MIZED_MIN;
            else
            {
                WINDOWPLACEMENT placement;
                placement.length = sizeof(placement);
                if (!GetWindowPlacement(hwnd, &placement)) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "GetWindowPlacement", GetLastError());

                if (placement.showCmd == SW_SHOWMAXIMIZED) mized = MIZED_MAX;
            }

            switch (mized)
            {
                case MIZED_NONE:
                {
                    deferredFlags->clear |= (FLAG_WINDOW_MINIMIZED | FLAG_WINDOW_MAXIMIZED);
                    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
                    MONITORINFO info;
                    info.cbSize = sizeof(info);
                    if (!GetMonitorInfoW(monitor, &info)) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "GetMonitorInfo", GetLastError());

                    if ((pos->x == info.rcMonitor.left) &&
                        (pos->y == info.rcMonitor.top) &&
                        (pos->cx == (info.rcMonitor.right - info.rcMonitor.left)) &&
                        (pos->cy == (info.rcMonitor.bottom - info.rcMonitor.top)))
                    {
                        deferredFlags->set |= FLAG_BORDERLESS_WINDOWED_MODE;
                    }
                    else
                    {
                        deferredFlags->clear |= FLAG_BORDERLESS_WINDOWED_MODE;
                    }

                } break;
                case MIZED_MIN:
                {
                    // !!! NOTE !!! Do not update the maximized/borderless
                    // flags because when hwnd is minimized it temporarily overrides
                    // the maximized state/flag which gets restored on SW_RESTORE
                    deferredFlags->set |= FLAG_WINDOW_MINIMIZED;
                } break;
                case MIZED_MAX:
                {
                    deferredFlags->clear |= FLAG_WINDOW_MINIMIZED;
                    deferredFlags->set |= FLAG_WINDOW_MAXIMIZED;
                } break;
                default: break;
            }
        } break;
        case WM_SIZE:
        {
            // WARNING: Don't trust the docs, they say you won't get this message if you don't call DefWindowProc
            // in response to WM_WINDOWPOSCHANGED but looks like when a window is created you'll get this
            // message without getting WM_WINDOWPOSCHANGED
            HandleWindowResize(hwnd, &platform.appScreenWidth, &platform.appScreenHeight);
        } break;
        //case WM_MOVE
        case WM_WINDOWPOSCHANGED:
        {
            WINDOWPOS *pos = (WINDOWPOS*)lparam;
            if (!(pos->flags & SWP_NOSIZE)) HandleWindowResize(hwnd, &platform.appScreenWidth, &platform.appScreenHeight);
        } break;
        case WM_GETDPISCALEDSIZE:
        {
            SIZE *inoutSize = (SIZE *)lparam;
            UINT newDpi = (UINT)wparam; // TODO: WARNING: Converting from WPARAM = UINT_PTR

            // for any of these other cases, we might want to post a window
            // resize event after the dpi changes?
            if (CORE.Window.flags & FLAG_WINDOW_MINIMIZED) return TRUE;
            if (CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) return TRUE;
            if (CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) return TRUE;

            float dpiScale = ((float)newDpi)/96.0f;
            bool dpiScaling = CORE.Window.flags & FLAG_WINDOW_HIGHDPI;
            SIZE desired = PxFromPt2(dpiScale, dpiScaling, platform.appScreenWidth, platform.appScreenHeight);
            inoutSize->cx = desired.cx;
            inoutSize->cy = desired.cy;
        
            result = TRUE;
        } break;
        case WM_DPICHANGED:
        {
            RECT *suggestedRect = (RECT*)lparam;
            // Never set the window size to anything other than the suggested rect here
            // Doing so can cause a window to stutter between monitors when transitioning between them
            if (!SetWindowPos(hwnd, NULL,
                suggestedRect->left,
                suggestedRect->top,
                suggestedRect->right - suggestedRect->left,
                suggestedRect->bottom - suggestedRect->top,
                SWP_NOZORDER | SWP_NOACTIVATE))
            {
                TRACELOG(LOG_ERROR, "%s failed, error=%lu", "SetWindowPos", GetLastError());
            }
        } break;
        case WM_SETCURSOR:
        {
            if (LOWORD(lparam) == HTCLIENT)
            {
                SetCursor(CORE.Input.Mouse.cursorHidden? NULL : LoadCursorW(NULL, (LPCWSTR)IDC_ARROW));
                return 0;
            }

            result = DefWindowProc(hwnd, msg, wparam, lparam);
        } break;
        //case WM_PAINT
        case WM_INPUT:
        {
            HandleRawInput(lparam);
        } break;
        case WM_MOUSEMOVE:
        {
            if (platform.cursorEnabled)
            {
                CORE.Input.Mouse.currentPosition.x = (float)GET_X_LPARAM(lparam);
                CORE.Input.Mouse.currentPosition.y = (float)GET_Y_LPARAM(lparam);
                CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;
            }
        } break;
        case WM_KEYDOWN: HandleKey(wparam, lparam, 1); break;
        case WM_KEYUP: HandleKey(wparam, lparam, 0); break;
        case WM_LBUTTONDOWN: HandleMouseButton(MOUSE_BUTTON_LEFT, 1); break;
        case WM_LBUTTONUP  : HandleMouseButton(MOUSE_BUTTON_LEFT, 0); break;
        case WM_RBUTTONDOWN: HandleMouseButton(MOUSE_BUTTON_RIGHT, 1); break;
        case WM_RBUTTONUP  : HandleMouseButton(MOUSE_BUTTON_RIGHT, 0); break;
        case WM_MBUTTONDOWN: HandleMouseButton(MOUSE_BUTTON_MIDDLE, 1); break;
        case WM_MBUTTONUP  : HandleMouseButton(MOUSE_BUTTON_MIDDLE, 0); break;
        case WM_XBUTTONDOWN: 
        {
            switch (HIWORD(wparam))
            {
                case XBUTTON1: HandleMouseButton(MOUSE_BUTTON_SIDE, 1); break;
                case XBUTTON2: HandleMouseButton(MOUSE_BUTTON_EXTRA, 1); break;
                default: TRACELOG(LOG_WARNING, "TODO: handle ex mouse button DOWN wparam=%u", HIWORD(wparam)); break;
            }
        } break;
        case WM_XBUTTONUP:
        {
            switch (HIWORD(wparam))
            {
                case XBUTTON1: HandleMouseButton(MOUSE_BUTTON_SIDE, 0); break;
                case XBUTTON2: HandleMouseButton(MOUSE_BUTTON_EXTRA, 0); break;
                default: TRACELOG(LOG_WARNING, "TODO: handle ex mouse button UP   wparam=%u", HIWORD(wparam)); break;
            }
        } break;
        case WM_MOUSEWHEEL: CORE.Input.Mouse.currentWheelMove.y = ((float)GET_WHEEL_DELTA_WPARAM(wparam))/WHEEL_DELTA; break;
        case WM_MOUSEHWHEEL: CORE.Input.Mouse.currentWheelMove.x = ((float)GET_WHEEL_DELTA_WPARAM(wparam))/WHEEL_DELTA; break;
        case WM_APP_UPDATE_WINDOW_SIZE:
        {
            UpdateWindowSize(UPDATE_WINDOW_NORMAL, hwnd, platform.appScreenWidth, platform.appScreenHeight, CORE.Window.flags);
        } break;

        default: result = DefWindowProcW(hwnd, msg, wparam, lparam); // Message passed directly for execution (default behaviour)
    }
    //------------------------------------------------------------------------------------

    // Sanity check
    if (platform.hwnd == hwnd)
    {
        CheckFlags("After WndProc", hwnd, CORE.Window.flags, MakeWindowStyle(CORE.Window.flags), mask);
    }

    // Operations to execute after the above check
    if (flagsOp.set & flagsOp.clear)
    {
        TRACELOG(LOG_ERROR, "the flags 0x%x were both set and cleared!", flagsOp.set & flagsOp.clear);
    }

    DWORD save = CORE.Window.flags;
    CORE.Window.flags |= flagsOp.set;
    CORE.Window.flags &= ~flagsOp.clear;
    if (save != CORE.Window.flags)
    {
        TRACELOG(LOG_DEBUG, "DeferredFlags: 0x%x > 0x%x (diff 0x%x)", save, CORE.Window.flags, save ^ CORE.Window.flags);
    }

    return result;
}

static void HandleKey(WPARAM wparam, LPARAM lparam, char state)
{
    KeyboardKey key = KeyFromWparam(wparam);
    
    // TODO: Use scancode?
    //BYTE scancode = lparam >> 16;
    //TRACELOG(LOG_INFO, "KEY key=%d vk=%lu scan=%u = %u", key, wparam, scancode, state);
    
    if (key != KEY_NULL)
    {
        CORE.Input.Keyboard.currentKeyState[key] = state;

        if ((key == KEY_ESCAPE) && (state == 1)) CORE.Window.shouldClose = 1;
    }
    else TRACELOG(LOG_WARNING, "INPUT: Unknown (or currently unhandled) virtual keycode %d (0x%x)", wparam, wparam);

    // TODO: Add key to the queue as well?
}

static void HandleMouseButton(int button, char state)
{
    CORE.Input.Mouse.currentButtonState[button] = state;
    CORE.Input.Touch.currentTouchState[button] = state;
}

static void HandleRawInput(LPARAM lparam)
{
    RAWINPUT input = { 0 };

    UINT inputSize = sizeof(input);
    UINT size = GetRawInputData((HRAWINPUT)lparam, RID_INPUT, &input, &inputSize, sizeof(RAWINPUTHEADER));
    
    if (size == (UINT)-1) TRACELOG(LOG_ERROR, "%s failed, error=%lu", "GetRawInputData", GetLastError());
    
    if (input.header.dwType != RIM_TYPEMOUSE) TRACELOG(LOG_ERROR, "Unexpected WM_INPUT type %lu", input.header.dwType);

    if (input.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) TRACELOG(LOG_ERROR, "TODO: handle absolute mouse inputs!");

    if (input.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP) TRACELOG(LOG_ERROR, "TODO: handle virtual desktop mouse inputs!");

    // Trick to keep the mouse position at 0,0 and instead move
    // the previous position so we can still get a proper mouse delta
    //CORE.Input.Mouse.previousPosition.x -= input.data.mouse.lLastX;
    //CORE.Input.Mouse.previousPosition.y -= input.data.mouse.lLastY;
    //if (CORE.Input.Mouse.currentPosition.x != 0) abort();
    //if (CORE.Input.Mouse.currentPosition.y != 0) abort();
}

static void HandleWindowResize(HWND hwnd, int *width, int *height)
{
    if (CORE.Window.flags & FLAG_WINDOW_MINIMIZED) return;

    SIZE clientSize = GetClientSize(hwnd);

    //TRACELOG(LOG_DEBUG, "WINDOW: New widow client size: [%lux%lu]", clientSize.cx, clientSize.cy);

    //CORE.Window.currentFbo.width = clientSize.cx;
    //CORE.Window.currentFbo.height = clientSize.cy;
    //glViewport(0, 0, clientSize.cx, clientSize.cy);
    //SetupFramebuffer(0, 0);

    SetupViewport(clientSize.cx, clientSize.cy);
    CORE.Window.resizedLastFrame = true;
    float dpiScale = ((float)GetDpiForWindow(hwnd))/96.0f;
    bool highdpi = !!(CORE.Window.flags & FLAG_WINDOW_HIGHDPI);
    unsigned int screenWidth = highdpi? (unsigned int)(((float)clientSize.cx)/dpiScale) : clientSize.cx;
    unsigned int screenHeight = highdpi? (unsigned int)(((float)clientSize.cy)/dpiScale) : clientSize.cy;
    CORE.Window.screen.width = screenWidth;
    CORE.Window.screen.height = screenHeight;
    
    if (AdoptWindowResize(CORE.Window.flags))
    {
        TRACELOG(LOG_DEBUG, "WINDOW: Updating app size to %ix%i from window resize", screenWidth, screenHeight);
        *width = screenWidth;
        *height = screenHeight;
    }

    CORE.Window.screenScale = MatrixScale(
        (float)CORE.Window.render.width/CORE.Window.screen.width,
        (float)CORE.Window.render.height/CORE.Window.screen.height,
        1.0f);
}

// Update window style
static void UpdateWindowStyle(HWND hwnd, unsigned desiredFlags)
{
    DWORD current = STYLE_MASK_WRITABLE & MakeWindowStyle(CORE.Window.flags);
    DWORD desired = STYLE_MASK_WRITABLE & MakeWindowStyle(desiredFlags);
    
    if (current != desired)
    {
        SetLastError(0);
        DWORD previous = STYLE_MASK_WRITABLE & SetWindowLongPtrW(hwnd, GWL_STYLE, desired);
        if (previous != current)
        {
            TRACELOG(LOG_ERROR, "SetWindowLong returned writable flags 0x%x but expected 0x%x (diff=0x%x, error=%lu)",
                previous, current, previous ^ current, GetLastError());
        }

        CheckFlags("UpdateWindowStyle", hwnd, desiredFlags, desired, STYLE_MASK_WRITABLE);
    }

    Mized currentMized = MizedFromStyle(MakeWindowStyle(CORE.Window.flags));
    Mized desiredMized = MizedFromStyle(MakeWindowStyle(desiredFlags));
    if (currentMized != desiredMized)
    {
        switch (desiredMized)
        {
            case MIZED_NONE: ShowWindow(hwnd, SW_RESTORE); break;
            case MIZED_MIN: ShowWindow(hwnd, SW_MINIMIZE); break;
            case MIZED_MAX: ShowWindow(hwnd, SW_MAXIMIZE); break;
        }
    }
}

// Sanitize flags
static unsigned SanitizeFlags(SanitizeFlagsKind kind, unsigned flags)
{
    if ((flags & FLAG_WINDOW_MAXIMIZED) && (flags & FLAG_BORDERLESS_WINDOWED_MODE))
    {
        TRACELOG(LOG_INFO, "borderless windows mode is overriding maximized");
        flags &= ~FLAG_WINDOW_MAXIMIZED;
    }

    switch (kind)
    {
        case SANITIZE_FLAGS_FIRST: break;
        case SANITIZE_FLAGS_NORMAL:
            if ((flags & FLAG_MSAA_4X_HINT) && (!(CORE.Window.flags & FLAG_MSAA_4X_HINT)))
            {
                TRACELOG(LOG_WARNING, "WINDOW: MSAA can only be configured before window initialization");
                flags &= ~FLAG_MSAA_4X_HINT;
            }
            break;
    }

    return flags;
}

// All window state changes from raylib flags go through this function. It performs
// whatever operations are needed to update the window state to match the desired flags
// In most cases this function should not update CORE.Window.flags directly, instead,
// the window itself should update CORE.Window.flags in response to actual state changes
// This means that CORE.Window.flags should always represent the actual state of the
// window. This function will continue to perform these update operations so long as
// the state continues to change
//
// This design takes care of many odd corner cases. For example, if you want to restore
// a window that was previously maximized AND minimized and you want to remove both these
// flags, you actually need to call ShowWindow with SW_RESTORE twice. Another example is
// if you have a maximized window, if the undecorated flag is modified then we'd need to
// update the window style, but updating the style would mean the window size would change
// causing the window to lose its Maximized state which would mean we'd need to update the
// window size and then update the window style a second time to restore that maximized
// state. This implementation is able to handle any/all of these special situations with a
// retry loop that continues until we either reach the desired state or the state stops changing
static void UpdateFlags(HWND hwnd, unsigned desiredFlags, int width, int height)
{
    // Flags that just apply immediately without needing any operations
    CORE.Window.flags |= (desiredFlags & FLAG_MASK_NO_UPDATE);

    int vsync = (CORE.Window.flags & FLAG_VSYNC_HINT)? 1 : 0;
    PFNWGLSWAPINTERVALEXTPROC wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapInterval)
    {
        (*wglSwapInterval)(vsync);
        if (vsync) CORE.Window.flags |= FLAG_VSYNC_HINT;
        else CORE.Window.flags &= ~FLAG_VSYNC_HINT;
    }

    DWORD previousStyle;
    for (unsigned attempt = 1; ; attempt++)
    {
        CheckFlags("UpdateFlags", hwnd, CORE.Window.flags, MakeWindowStyle(CORE.Window.flags), STYLE_MASK_ALL);

        bool windowSizeUpdated = false;
        if (MakeWindowStyle(CORE.Window.flags) == MakeWindowStyle(desiredFlags))
        {
            windowSizeUpdated = UpdateWindowSize(UPDATE_WINDOW_NORMAL, hwnd, width, height, desiredFlags);
            if ((FLAG_MASK_REQUIRED & desiredFlags) == (FLAG_MASK_REQUIRED & CORE.Window.flags)) break;
        }

        if ((attempt > 1) &&
            (previousStyle == MakeWindowStyle(CORE.Window.flags)) &&
            !windowSizeUpdated)
        {
            TRACELOG(LOG_ERROR, "WINDOW: UpdateFlags() failed after %u attempt(s) wanted 0x%x but is 0x%x (diff=0x%x)",
                attempt, desiredFlags, CORE.Window.flags, desiredFlags ^ CORE.Window.flags);
        }

        previousStyle = MakeWindowStyle(CORE.Window.flags);
        UpdateWindowStyle(hwnd, desiredFlags);
    }
}
