/**********************************************************************************************
*
*   rcore_desktop_win32 - Functions to manage window, graphics device and inputs
*
*   PLATFORM: DESKTOP: WIN32
*       - Windows (Win32, Win64)
*
*   LIMITATIONS:
*       - currently in initial development stage, alot is missing
*       - unsure how to support MOUSE_BUTTON_FORWARD/MOUSE_BUTTON_BACK
*
*   POSSIBLE IMPROVEMENTS:
*
*   ADDITIONAL NOTES:
*
*   CONFIGURATION:
*       #define RCORE_PLATFORM_CUSTOM_FLAG
*           Custom flag for rcore on target platform -not used-
*
*   DEPENDENCIES:
*       - the win32 API, i.e. windows.h
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

// move windows.h functions to new names to avoid redefining the same functions as raylib
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

#undef CloseWindow
#undef Rectangle
#undef ShowCursor
#undef LoadImage
#undef LoadImageA
#undef LoadImageW
#undef DrawText
#undef DrawTextA
#undef DrawTextW
#undef DrawTextEx
#undef DrawTextExA
#undef DrawTextExW

#include <windowsx.h>
#include <shellscalingapi.h>
#include <versionhelpers.h>

#include <GL/gl.h>
#include <GL/wglext.h>

// --------------------------------------------------------------------------------
// This part of the file contains pure functions that never access global state.
// This distinction helps keep the backend maintainable as the inputs and outputs
// of every function called in this section can be fully derived from the
// call-site alone.
// --------------------------------------------------------------------------------

// Prevent any code in this part of the file from accessing the global CORE state
#define CORE DONT_USE_CORE_HERE

static size_t AToWLen(const char *a)
{
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, a, -1, NULL, 0);
    if (sizeNeeded < 0)
    {
        TRACELOG(LOG_ERROR, "failed to calculate wide length, result=%d, error=%u", sizeNeeded, GetLastError());
        abort();
    }

    return sizeNeeded;
}
static void AToWCopy(wchar_t *outPtr, size_t outLen, const char *a)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, a, -1, outPtr, outLen);
    if (size != outLen)
    {
        TRACELOG(LOG_ERROR, "expected to convert %zu utf8 chars to wide but converted %zu", outLen, size);
        abort();
    }
}
#define A_TO_W_ALLOCA(outWstr, inAnsi)   do {                   \
    size_t len = AToWLen(inAnsi);                               \
    outWstr = (WCHAR*)alloca(sizeof(WCHAR)*(len + 1));        \
    AToWCopy(outWstr, len, inAnsi);                             \
    outWstr[len] = 0;                                           \
} while (0)

static void LogFail(const char *what, DWORD error)
{
    TRACELOG(LOG_ERROR, "%s failed, error=%lu", what, error);
}
static void LogFailHr(const char *what, HRESULT hr)
{
    TRACELOG(LOG_ERROR, "%s failed, hresult=0x%lx", what, (DWORD)hr);
}

#define STYLE_FLAGS_RESIZABLE  WS_THICKFRAME

#define STYLE_FLAGS_UNDECORATED_OFF (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define STYLE_FLAGS_UNDECORATED_ON  WS_POPUP

static bool ResizableFromStyle(DWORD style)
{
    return 0 != (style & STYLE_FLAGS_RESIZABLE);
}
static bool DecoratedFromStyle(DWORD style)
{
    if (style & STYLE_FLAGS_UNDECORATED_ON)
    {
        if (style & STYLE_FLAGS_UNDECORATED_OFF)
        {
            TRACELOG(LOG_ERROR, "style 0x%x has both undecorated on/off flags", style);
            abort();
        }

        return false; // not decorated
    }

    DWORD masked = (style & STYLE_FLAGS_UNDECORATED_OFF);
    if (STYLE_FLAGS_UNDECORATED_OFF != masked)
    {
        TRACELOG(LOG_ERROR, "style 0x%x is missing these flags 0x%x", masked, masked ^ STYLE_FLAGS_UNDECORATED_OFF);
        abort();
    }

    return true; // decorated
}
static bool HiddenFromStyle(DWORD style)
{
    return 0 == (style & WS_VISIBLE);
}

typedef enum { MIZED_NONE, MIZED_MIN, MIZED_MAX } Mized;
Mized MizedFromStyle(DWORD style)
{
    // minimized takes precedence over maximized
    if (style & WS_MINIMIZE) return MIZED_MIN;
    if (style & WS_MAXIMIZE) return MIZED_MAX;
    return MIZED_NONE;
}
Mized MizedFromFlags(unsigned flags)
{
    // minimized takes precedence over maximized
    if (flags & FLAG_WINDOW_MINIMIZED) return MIZED_MIN;
    if (flags & FLAG_WINDOW_MAXIMIZED) return MIZED_MAX;
    return MIZED_NONE;
}

static DWORD MakeWindowStyle(unsigned flags)
{
    DWORD style =
        // we don't need this since we don't have any child windows, but I guess
        // it improves efficiency, plus, windows adds this flag automatically anyway
        // so it keeps our flags in sync with the OS.
        WS_CLIPSIBLINGS
        ;
    style |= (flags & FLAG_WINDOW_HIDDEN)? 0 : WS_VISIBLE;
    style |= (flags & FLAG_WINDOW_RESIZABLE)? STYLE_FLAGS_RESIZABLE : 0;
    style |= (flags & FLAG_WINDOW_UNDECORATED)? STYLE_FLAGS_UNDECORATED_ON : STYLE_FLAGS_UNDECORATED_OFF;

    switch (MizedFromFlags(flags))
    {
        case MIZED_NONE: break;
        case MIZED_MIN: style |= WS_MINIMIZE; break;
        case MIZED_MAX: style |= WS_MAXIMIZE; break;
        default: abort();
    }

    // sanity checks, maybe remove later
    if (ResizableFromStyle(style) != !!(flags & FLAG_WINDOW_RESIZABLE)) abort();
    if (DecoratedFromStyle(style) != !(flags & FLAG_WINDOW_UNDECORATED)) abort();
    if (HiddenFromStyle(style) != !!(flags & FLAG_WINDOW_HIDDEN)) abort();
    if (MizedFromStyle(style) != MizedFromFlags(flags)) abort();

    return style;
}

static bool IsMinimized2(HWND hwnd)
{
    bool isIconic = IsIconic(hwnd);
    bool styleMinimized = !!(WS_MINIMIZE & GetWindowLongPtrW(hwnd, GWL_STYLE));
    if (isIconic != styleMinimized)
    {
        TRACELOG(LOG_ERROR, "IsIconic(%d) != WS_MINIMIZED(%d)", isIconic, styleMinimized);
        abort();
    }

    return isIconic;
}

#define STYLE_MASK_ALL 0xffffffff
#define STYLE_MASK_READONLY (WS_MINIMIZE | WS_MAXIMIZE)
#define STYLE_MASK_WRITABLE (~STYLE_MASK_READONLY)

// Enforces that the actual window/platform state is in sync with raylib's flags
static void CheckFlags(
    const char *context,
    HWND hwnd,
    DWORD flags,
    DWORD expectedStyle,
    DWORD styleCheckMask
) {
    //TRACELOG(LOG_INFO, "Verifying Flags 0x%x Style 0x%x Mask 0x%x", flags, expectedStyle & styleCheckMask, styleCheckMask);

    {
        DWORD styleFromFlags = MakeWindowStyle(flags);
        if ((styleFromFlags & styleCheckMask) != (expectedStyle & styleCheckMask))
        {
            TRACELOG(
                LOG_ERROR,
                "%s: window flags (0x%x) produced style 0x%x which != expected 0x%x (diff=0x%x, mask=0x%x)",
                context,
                flags,
                styleFromFlags & styleCheckMask,
                expectedStyle & styleCheckMask,
                (styleFromFlags & styleCheckMask) ^ (expectedStyle & styleCheckMask),
                styleCheckMask
            );
            abort();
        }
    }

    SetLastError(0);
    LONG actualStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
    if ((actualStyle & styleCheckMask) != (expectedStyle & styleCheckMask))
    {
        TRACELOG(
            LOG_ERROR,
            "%s: expected style 0x%x but got 0x%x (diff=0x%x, mask=0x%x, lasterror=%lu)",
            context,
            expectedStyle & styleCheckMask,
            actualStyle & styleCheckMask,
            (expectedStyle & styleCheckMask) ^ (actualStyle & styleCheckMask),
            styleCheckMask,
            GetLastError()
        );
        abort();
    }

    if (styleCheckMask & WS_MINIMIZE)
    {
        bool isIconic = IsIconic(hwnd);
        bool styleMinimized = !!(WS_MINIMIZE & actualStyle);
        if (isIconic != styleMinimized) {
            TRACELOG(LOG_ERROR, "IsIconic(%d) != WS_MINIMIZED(%d)", isIconic, styleMinimized);
            abort();
        }
    }

    if (styleCheckMask & WS_MAXIMIZE)
    {
        WINDOWPLACEMENT placement;
        placement.length = sizeof(placement);
        if (!GetWindowPlacement(hwnd, &placement)) {
            LogFail("GetWindowPlacement", GetLastError());
            abort();
        }
        bool placementMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);
        bool styleMaximized = WS_MAXIMIZE & actualStyle;
        if (placementMaximized != styleMaximized)
        {
            TRACELOG(
                LOG_ERROR,
                "maximized state desync, placement maximized=%d (showCmd=%lu) style maximized=%d",
                placementMaximized,
                placement.showCmd,
                styleMaximized
            );
            abort();
        }
    }
}

static float PtFromPx(float dpiScale, bool highdpiEnabled, int pt)
{
    return highdpiEnabled? (((float)pt)/dpiScale) : pt;
}
static int PxFromPt(float dpiScale, bool highdpiEnabled, float pt)
{
    return highdpiEnabled? roundf(pt*dpiScale) : roundf(pt);
}
static SIZE PxFromPt2(float dpiScale, bool highdpiEnabled, Vector2 screenSize)
{
    return (SIZE){
        PxFromPt(dpiScale, highdpiEnabled, screenSize.x),
        PxFromPt(dpiScale, highdpiEnabled, screenSize.y),
    };
}

static SIZE GetClientSize(HWND hwnd)
{
    RECT rect;
    if (0 == GetClientRect(hwnd, &rect))
    {
        LogFail("GetClientRect", GetLastError());
        abort();
    }

    if (rect.left != 0) abort(); // never happens AFAIK
    if (rect.top != 0) abort(); // never happens AFAIK
    return (SIZE){ rect.right, rect.bottom };
}

static UINT GetWindowDpi(HWND hwnd)
{
    UINT dpi = GetDpiForWindow(hwnd);
    if (dpi == 0)
    {
        LogFail("GetWindowDpi", GetLastError());
        abort();
    }

    return dpi;
}

static float ScaleFromDpi(UINT dpi)
{
    return ((float)dpi)/96.0f;
}

#define WINDOW_STYLE_EX 0

static SIZE CalcWindowSize(UINT dpi, SIZE clientSize, DWORD style)
{
    RECT rect = { 0, 0, clientSize.cx, clientSize.cy };
    if (!AdjustWindowRectExForDpi(&rect, style, 0, WINDOW_STYLE_EX, dpi))
    {
        LogFail("AdjustWindowRect", GetLastError());
        abort();
    }

    return (SIZE){ rect.right - rect.left, rect.bottom - rect.top };
}

typedef enum {
    UPDATE_WINDOW_FIRST,
    UPDATE_WINDOW_NORMAL,
} UpdateWindowKind;

// returns true if the window size was updated, false otherwise
static bool UpdateWindowSize(
    UpdateWindowKind kind,
    HWND hwnd,
    Vector2 appScreenSize,
    unsigned flags
) {
    if (flags & FLAG_WINDOW_MINIMIZED) return false;

    if (flags & FLAG_WINDOW_MAXIMIZED)
    {
        CheckFlags("UpdateWindowSize(maximized)", hwnd, flags, MakeWindowStyle(flags), STYLE_MASK_ALL);
        return false;
    }

    if (flags & FLAG_BORDERLESS_WINDOWED_MODE)
    {
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO info;
        info.cbSize = sizeof(info);
        if (!GetMonitorInfoW(monitor, &info))
        {
            LogFail("GetMonitorInfo", GetLastError());
            abort();
        }

        RECT windowRect;
        if (!GetWindowRect(hwnd, &windowRect))
        {
            LogFail("GetWindowRect", GetLastError());
            abort();
        }

        if (
            (windowRect.left == info.rcMonitor.left) &&
            (windowRect.top == info.rcMonitor.top) &&
            ((windowRect.right - windowRect.left) == (info.rcMonitor.right - info.rcMonitor.left)) &&
            ((windowRect.bottom - windowRect.top) == (info.rcMonitor.bottom - info.rcMonitor.top))
        ) return false;

        if (!SetWindowPos(
            hwnd,
            HWND_TOP,
            info.rcMonitor.left, info.rcMonitor.top,
            info.rcMonitor.right - info.rcMonitor.left,
            info.rcMonitor.bottom - info.rcMonitor.top,
            SWP_NOOWNERZORDER
        )) {
            LogFail("SetWindowPos", GetLastError());
            abort();
        }

        return true;
    }

    UINT dpi = GetWindowDpi(hwnd);
    float dpiScale = ScaleFromDpi(dpi);
    bool dpiScaling = flags & FLAG_WINDOW_HIGHDPI;
    SIZE desired = PxFromPt2(dpiScale, dpiScaling, appScreenSize);
    SIZE actual = GetClientSize(hwnd);
    if (actual.cx == desired.cx || actual.cy == desired.cy)
        return false;

    TRACELOG(
        LOG_INFO,
        "restoring client size from %dx%d to %dx%d (dpi=%lu dpiScaling=%d app=%fx%f)",
        actual.cx, actual.cy,
        desired.cx, desired.cy,
        dpi, dpiScaling,
        appScreenSize.x, appScreenSize.y
    );
    SIZE windowSize = CalcWindowSize(dpi, desired, MakeWindowStyle(flags));
    POINT windowPos = (POINT){ 0, 0 };
    UINT swpFlags = SWP_NOZORDER | SWP_FRAMECHANGED;
    if (kind == UPDATE_WINDOW_FIRST)
    {
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        if (!monitor)
        {
            LogFail("MonitorFromWindow", GetLastError());
            abort();
        }

        MONITORINFO info;
        info.cbSize = sizeof(info);
        if (!GetMonitorInfoW(monitor, &info))
        {
            LogFail("GetMonitorInfo", GetLastError());
            abort();
        }

        LONG monitorWidth = info.rcMonitor.right - info.rcMonitor.left;
        LONG monitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;
        windowPos = (POINT){
            MAX(0, (monitorWidth - windowSize.cx)/2),
            MAX(0, (monitorHeight - windowSize.cy)/2),
        };
    } else {
        swpFlags |= SWP_NOMOVE;
    }

    if (!SetWindowPos(
        hwnd, NULL,
        windowPos.x, windowPos.y,
        windowSize.cx, windowSize.cy,
        swpFlags
    )) {
        LogFail("SetWindowPos", GetLastError());
        abort();
    }

    return true;
}

#define CLASS_NAME L"RaylibWindow"

static void CreateWindowAlloca(const char *title, DWORD style)
{
    WCHAR *titleWide;
    A_TO_W_ALLOCA(titleWide, title);
    CreateWindowExW(
        WINDOW_STYLE_EX,
        CLASS_NAME,
        titleWide,
        style,
        0, 0,
        0, 0,
        NULL,
        NULL,
        GetModuleHandleW(NULL),
        NULL
    );
}

static BOOL IsWindows10Version1703OrGreaterWin32(void)
{
    HMODULE ntdll = LoadLibraryW(L"ntdll.dll");
    DWORD (*Verify)(
        RTL_OSVERSIONINFOEXW*, ULONG, ULONGLONG
    ) = (DWORD (*)(
        RTL_OSVERSIONINFOEXW*, ULONG, ULONGLONG
    ))GetProcAddress(ntdll, "RtlVerifyVersionInfo");
    if (!Verify)
    {
        LogFail("GetProcAddress 'RtlVerifyVersionInfo'", GetLastError());
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

static void *FindProc(const char *name)
{
    {
        void *proc = wglGetProcAddress(name);
        if (proc) return proc;
    }

    static HMODULE opengl = NULL;
    if (!opengl)
    {
        opengl = LoadLibraryW(L"opengl32");
    }
    if (opengl)
    {
        void *proc = GetProcAddress(opengl, name);
        if (proc) return proc;
    }

    return NULL;
}
static void *WglGetProcAddress(const char *name)
{
    void *result = FindProc(name);
    if (result)
    {
        //TRACELOG(LOG_DEBUG, "GetProcAddress '%s' > %p", name, result);
    }
    else
    {
        TRACELOG(LOG_ERROR, "GetProcAddress '%s' > %p failed, error=%u", name, result, GetLastError());
    }

    return result;
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
    switch (cursor)
    {
        case MOUSE_CURSOR_DEFAULT      : return (LPCWSTR)IDC_ARROW;
        case MOUSE_CURSOR_ARROW        : return (LPCWSTR)IDC_ARROW;
        case MOUSE_CURSOR_IBEAM        : return (LPCWSTR)IDC_IBEAM;
        case MOUSE_CURSOR_CROSSHAIR    : return (LPCWSTR)IDC_CROSS;
        case MOUSE_CURSOR_POINTING_HAND: return (LPCWSTR)IDC_HAND;
        case MOUSE_CURSOR_RESIZE_EW    : return (LPCWSTR)IDC_SIZEWE;
        case MOUSE_CURSOR_RESIZE_NS    : return (LPCWSTR)IDC_SIZENS;
        case MOUSE_CURSOR_RESIZE_NWSE  : return (LPCWSTR)IDC_SIZENWSE;
        case MOUSE_CURSOR_RESIZE_NESW  : return (LPCWSTR)IDC_SIZENESW;
        case MOUSE_CURSOR_RESIZE_ALL   : return (LPCWSTR)IDC_SIZEALL;
        case MOUSE_CURSOR_NOT_ALLOWED  : return (LPCWSTR)IDC_NO;
        default: abort();
    }
}


static BOOL CALLBACK CountMonitorsProc(HMONITOR handle, HDC _, LPRECT rect, LPARAM lparam)
{
    int *count = (int*)lparam;
    *count += 1;
    // always return TRUE to continue the loop, otherwise, the caller
    // can't distinguish between stopping the loop and an error
    return TRUE;
}

typedef struct {
    HMONITOR needle;
    int index;
    int matchIndex;
    RECT rect;
} FindMonitorContext;

static BOOL CALLBACK FindMonitorProc(HMONITOR handle, HDC _, LPRECT rect, LPARAM lparam)
{
    FindMonitorContext *c = (FindMonitorContext*)lparam;
    if (handle == c->needle)
    {
        c->matchIndex = c->index;
        c->rect = *rect;
    }

    c->index += 1;
    // always return TRUE to continue the loop, otherwise, the caller
    // can't distinguish between stopping the loop and an error
    return TRUE;
}

typedef struct {
    DWORD set;
    DWORD clear;
} FlagsOp;

static void GetStyleChangeFlagOps(
    DWORD coreWindowFlags,
    STYLESTRUCT *ss,
    FlagsOp *deferredFlags
) {
    {
        bool resizable = (coreWindowFlags & FLAG_WINDOW_RESIZABLE);
        bool resizableOld = ResizableFromStyle(ss->styleOld);
        bool resizableNew = ResizableFromStyle(ss->styleNew);
        if (resizable != resizableOld)
        {
            TRACELOG(LOG_ERROR, "expected resizable %u but got %u", resizable, resizableOld);
            abort();
        }

        if (resizableOld != resizableNew)
        {
            //TRACELOG(LOG_INFO, "resizable = %u", resizableNew);
            if (resizableNew)
            {
                deferredFlags->set |= FLAG_WINDOW_RESIZABLE;
            }
            else
            {
                deferredFlags->clear |= FLAG_WINDOW_RESIZABLE;
            }
        }
    }

    {
        bool decorated = (0 == (coreWindowFlags & FLAG_WINDOW_UNDECORATED));
        bool decoratedOld = DecoratedFromStyle(ss->styleOld);
        bool decoratedNew = DecoratedFromStyle(ss->styleNew);
        if (decorated != decoratedOld)
        {
            TRACELOG(LOG_ERROR, "expected decorated %u but got %u", decorated, decoratedOld);
            abort();
        }

        if (decoratedOld != decoratedNew)
        {
            //TRACELOG(LOG_INFO, "decorated = %u", decoratedNew);
            if (decoratedNew)
            {
                deferredFlags->clear |= FLAG_WINDOW_UNDECORATED;
            }
            else
            {
                deferredFlags->set |= FLAG_WINDOW_UNDECORATED;
            }
        }
    }

    {
        bool hidden = (coreWindowFlags & FLAG_WINDOW_HIDDEN);
        bool hiddenOld = HiddenFromStyle(ss->styleOld);
        bool hiddenNew = HiddenFromStyle(ss->styleNew);
        if (hidden != hiddenOld)
        {
            TRACELOG(LOG_ERROR, "expected hidden %u but got %u", hidden, hiddenOld);
            abort();
        }

        if (hiddenOld != hiddenNew)
        {
            TRACELOG(LOG_INFO, "hidden = %u", hiddenNew);
            if (hiddenNew)
            {
                deferredFlags->set |= FLAG_WINDOW_HIDDEN;
            }
            else
            {
                deferredFlags->clear |= FLAG_WINDOW_HIDDEN;
            }
        }
    }
}

// call when the window is rezised, returns true if the new window size
// should update the desired app size
static bool AdoptWindowResize(unsigned flags)
{
    if (flags & FLAG_WINDOW_MINIMIZED) return false;
    if (flags & FLAG_WINDOW_MAXIMIZED) return false;
    if (flags & FLAG_FULLSCREEN_MODE) return false;
    if (flags & FLAG_BORDERLESS_WINDOWED_MODE) return false;
    if (!(flags & FLAG_WINDOW_RESIZABLE)) return false;
    return true;
}

// --------------------------------------------------------------------------------
// Here's the end of the "pure function section", the rest of the file can access
// global state.
// --------------------------------------------------------------------------------

// unlock the ability to use CORE in the rest of the file
#undef CORE

// The last screen size requested by the app, the backend must keep the client area
// this size (after DPI scaling is applied) when the window isn't fullscreen/maximized/minimized.
static Vector2 globalAppScreenSize = (Vector2){0, 0};
static unsigned globalDesiredFlags = 0;
static HWND globalHwnd = NULL;
static HDC globalHdc = NULL;
static HGLRC globalGlContext = NULL;
static LARGE_INTEGER globalTimerFrequency;
static bool globalCursorEnabled = true;

static void HandleKey(WPARAM wparam, LPARAM lparam, char state)
{
    KeyboardKey key = KeyFromWparam(wparam);
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    /* BYTE scancode = lparam >> 16; */
    /* TRACELOG(LOG_INFO, "KEY key=%d vk=%lu scan=%u = %u", key, wparam, scancode, state); */
    if (key != KEY_NULL)
    {
        /* TRACELOG(LOG_INFO, "KEY[%d] = %d (old=%d)\n", key, state, CORE.Input.Keyboard.currentKeyState[key]); */
        CORE.Input.Keyboard.currentKeyState[key] = state;
        if (key == KEY_ESCAPE && state == 1)
        {
            CORE.Window.shouldClose = 1;
        }
    }
    else
    {
        TRACELOG(LOG_WARNING, "unknown (or currently unhandled) virtual keycode %d (0x%x)", wparam, wparam);
    }

    // TODO: add it to the queue as well?
}
static void HandleMouseButton(int button, char state)
{
    CORE.Input.Mouse.currentButtonState[button] = state;
    CORE.Input.Touch.currentTouchState[button] = state;
}

static void HandleRawInput(LPARAM lparam)
{
    RAWINPUT input;

    {
        UINT inputSize = sizeof(input);
        UINT size = GetRawInputData((HRAWINPUT)lparam, RID_INPUT, &input, &inputSize, sizeof(RAWINPUTHEADER));
        if (size == (UINT)-1) {
            LogFail("GetRawInputData", GetLastError());
            abort();
        }
        if (size != inputSize) abort(); // bug?
    }

    if (input.header.dwType != RIM_TYPEMOUSE)
    {
        TRACELOG(LOG_ERROR, "Unexpected WM_INPUT type %lu", input.header.dwType);
        abort();
    }

    if (input.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
    {
        TRACELOG(LOG_ERROR, "TODO: handle absolute mouse inputs!");
        abort();
    }

    if (input.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP)
    {
        TRACELOG(LOG_ERROR, "TODO: handle virtual desktop mouse inputs!");
        abort();
    }

    // bit of a trick, we keep the mouse position at 0,0 and instead move
    // the previous position so we can still get a proper mouse delta
    CORE.Input.Mouse.previousPosition.x -= input.data.mouse.lLastX;
    CORE.Input.Mouse.previousPosition.y -= input.data.mouse.lLastY;
    if (CORE.Input.Mouse.currentPosition.x != 0) abort();
    if (CORE.Input.Mouse.currentPosition.y != 0) abort();
}

static void HandleWindowResize(HWND hwnd, Vector2 *appScreenSizeRef)
{
    if (CORE.Window.flags & FLAG_WINDOW_MINIMIZED)
        return;

    SIZE clientSize = GetClientSize(hwnd);

    // TODO: not sure if this function is doing what we need, leaving this disabled
    //       call to workaround unused function error
    if (0) SetupFramebuffer(0, 0);

    TRACELOG(LOG_DEBUG, "NewClientSize %lux%lu", clientSize.cx, clientSize.cy);
    /* CORE.Window.currentFbo.width = clientSize.cx; */
    /* CORE.Window.currentFbo.height = clientSize.cy; */
    /* glViewport(0, 0, clientSize.cx, clientSize.cy); */
    SetupViewport(clientSize.cx, clientSize.cy);
    CORE.Window.resizedLastFrame = true;
    float dpiScale = ScaleFromDpi(GetWindowDpi(hwnd));
    bool highdpi = !!(CORE.Window.flags & FLAG_WINDOW_HIGHDPI);
    Vector2 screenSize = (Vector2){
        PtFromPx(dpiScale, highdpi, clientSize.cx),
        PtFromPx(dpiScale, highdpi, clientSize.cy),
    };
    CORE.Window.screen.width = roundf(screenSize.x);
    CORE.Window.screen.height = roundf(screenSize.y);
    if (AdoptWindowResize(CORE.Window.flags))
    {
        TRACELOG(
            LOG_DEBUG,
            "updating app size to %fx%f from window resize",
            screenSize.x, screenSize.y
        );
        *appScreenSizeRef = screenSize;
    }

    CORE.Window.screenScale = MatrixScale(
        (float)CORE.Window.render.width/CORE.Window.screen.width,
        (float)CORE.Window.render.height/CORE.Window.screen.height,
        1.0f
    );
}

static void UpdateWindowStyle(HWND hwnd, unsigned desiredFlags)
{
    {
        DWORD current = STYLE_MASK_WRITABLE & MakeWindowStyle(CORE.Window.flags);
        DWORD desired = STYLE_MASK_WRITABLE & MakeWindowStyle(desiredFlags);
        if (current != desired)
        {
            SetLastError(0);
            DWORD previous = STYLE_MASK_WRITABLE & SetWindowLongPtrW(hwnd, GWL_STYLE, desired);
            if (previous != current)
            {
                TRACELOG(
                    LOG_ERROR,
                    "SetWindowLong returned writable flags 0x%x but expected 0x%x (diff=0x%x, error=%lu)",
                    previous,
                    current,
                    previous ^ current,
                    GetLastError()
                );
                abort();
            }

            CheckFlags("UpdateWindowStyle", hwnd, desiredFlags, desired, STYLE_MASK_WRITABLE);
        }
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

typedef enum {
    SANITIZE_FLAGS_FIRST,
    SANITIZE_FLAGS_NORMAL,
} SanitizeFlagsKind;

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

#define FLAG_MASK_OPTIONAL (FLAG_VSYNC_HINT)
#define FLAG_MASK_REQUIRED ~(FLAG_MASK_OPTIONAL)

// flags that have no operations to perform during an update
#define FLAG_MASK_NO_UPDATE (FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT)


// All window state changes from raylib flags go through this function. It performs
// whatever operations are needed to update the window state to match the desired flags.
// In most cases this function should not update CORE.Window.flags directly, instead,
// the window itself should update CORE.Window.flags in response to actual state changes.
// This means that CORE.Window.flags should always represent the actual state of the
// window. This function will continue to perform these update operations so long as
// the state continues to change.
//
// This design takes care of many odd corner cases. For example, if you want to restore
// a window that was previously maximized AND minimized and you want to remove both these
// flags, you actually need to call ShowWindow with SW_RESTORE twice. Another example is
// if you have a maximized window, if the undecorated flag is modified then we'd need to
// update the window style, but updating the style would mean the window size would change
// causing the window to lose its Maximized state which would mean we'd need to update the
// window size and then update the window style a second time to restore that maximized
// state. This implementation is able to handle any/all of these special situations with a
// retry loop that continues until we either reach the desired state or the state stops
// changing.
static void UpdateFlags(HWND hwnd, unsigned desiredFlags, Vector2 appScreenSize)
{
    // flags that just apply immediately without needing any operations
    CORE.Window.flags |= (desiredFlags & FLAG_MASK_NO_UPDATE);

    {
        int vsync = (CORE.Window.flags & FLAG_VSYNC_HINT)? 1 : 0;
        PFNWGLSWAPINTERVALEXTPROC f = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        if (f)
        {
            (*f)(vsync);
            if (vsync)
            {
                CORE.Window.flags |= FLAG_VSYNC_HINT;
            }
            else
            {
                CORE.Window.flags &= ~FLAG_VSYNC_HINT;
            }
        }
    }

    DWORD previousStyle;
    for (unsigned attempt = 1; ; attempt++)
    {
        CheckFlags("UpdateFlags", hwnd, CORE.Window.flags, MakeWindowStyle(CORE.Window.flags), STYLE_MASK_ALL);

        bool windowSizeUpdated = false;
        if (MakeWindowStyle(CORE.Window.flags) == MakeWindowStyle(desiredFlags))
        {
            windowSizeUpdated = UpdateWindowSize(UPDATE_WINDOW_NORMAL, hwnd, appScreenSize, desiredFlags);
            if ((FLAG_MASK_REQUIRED & desiredFlags) == (FLAG_MASK_REQUIRED & CORE.Window.flags))
                break;
        }

        if (
            (attempt > 1) &&
            (previousStyle == MakeWindowStyle(CORE.Window.flags)) &&
            !windowSizeUpdated
        ) {
            TRACELOG(
                LOG_ERROR,
                // TODO: print more information
                "UpdateFlags failed after %u attempt(s) wanted 0x%x but is 0x%x (diff=0x%x)",
                attempt,
                desiredFlags,
                CORE.Window.flags,
                desiredFlags ^ CORE.Window.flags
            );
            abort();
        }

        previousStyle = MakeWindowStyle(CORE.Window.flags);
        UpdateWindowStyle(hwnd, desiredFlags);
    }
}

bool WindowShouldClose(void)
{
    return CORE.Window.shouldClose;
}

void ToggleFullscreen(void)
{
    TRACELOG(LOG_WARNING, "ToggleFullscreen not implemented");
    assert(0); // crash debug builds only
}

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

void MaximizeWindow(void)
{
    SetWindowState(FLAG_WINDOW_MAXIMIZED);
}

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
    globalDesiredFlags = SanitizeFlags(SANITIZE_FLAGS_NORMAL, CORE.Window.flags | flags);
    UpdateFlags(globalHwnd, globalDesiredFlags, globalAppScreenSize);
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    globalDesiredFlags = SanitizeFlags(SANITIZE_FLAGS_NORMAL, CORE.Window.flags & ~flags);
    UpdateFlags(globalHwnd, globalDesiredFlags, globalAppScreenSize);
}

// Set icon for window
void SetWindowIcon(Image image)
{
    TRACELOG(LOG_WARNING, "SetWindowIcon not implemented");
    assert(0); // crash debug builds only
}

// Set icon for window
void SetWindowIcons(Image *images, int count)
{
    TRACELOG(LOG_WARNING, "SetWindowIcons not implemented");
    assert(0); // crash debug builds only
}

void SetWindowTitle(const char *title)
{
    CORE.Window.title = title;
    WCHAR *titlew;
    A_TO_W_ALLOCA(titlew, CORE.Window.title);
    if (!SetWindowTextW(globalHwnd, titlew))
    {
        LogFail("SetWindowText", GetLastError());
        abort();
    }
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
    TRACELOG(LOG_WARNING, "SetWindowPosition not implemented");
    assert(0); // crash debug builds only
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    TRACELOG(LOG_WARNING, "SetWindowMonitor not implemented");
    assert(0); // crash debug builds only
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    TRACELOG(LOG_WARNING, "SetWindowMinSize not implemented");
    assert(0); // crash debug builds only
    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    TRACELOG(LOG_WARNING, "SetWindowMaxSize not implemented");
    assert(0); // crash debug builds only
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    TRACELOG(LOG_WARNING, "SetWindowSize not implemented");
    assert(0); // crash debug builds only
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    TRACELOG(LOG_WARNING, "SetWindowOpacity not implemented");
    assert(0); // crash debug builds only
}

void SetWindowFocused(void)
{
    TRACELOG(LOG_WARNING, "SetWindowFocused not implemented");
    assert(0); // crash debug builds only
}

// Get native window handle
void *GetWindowHandle(void)
{
    return globalHwnd;
}

int GetMonitorCount(void)
{
    int count = 0;
    if (!EnumDisplayMonitors(NULL, NULL, CountMonitorsProc, (LPARAM)&count))
    {
        LogFail("EnumDisplayMonitors", GetLastError());
        abort();
    }

    return count;
}

// Get current monitor where window is placed
int GetCurrentMonitor(void)
{
    HMONITOR monitor = MonitorFromWindow(globalHwnd, MONITOR_DEFAULTTOPRIMARY);
    if (!monitor)
    {
        LogFail("MonitorFromWindow", GetLastError());
        abort();
    }

    FindMonitorContext context;
    context.needle = monitor;
    context.index = 0;
    context.matchIndex = -1;
    if (!EnumDisplayMonitors(NULL, NULL, FindMonitorProc, (LPARAM)&context))
    {
        LogFail("EnumDisplayMonitors", GetLastError());
        abort();
    }

    return context.matchIndex;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPosition not implemented");
    assert(0); // crash debug builds only
    return (Vector2){ 0, 0 };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorWidth not implemented");
    assert(0); // crash debug builds only
    return 0;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorHeight not implemented");
    assert(0); // crash debug builds only
    return 0;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPhysicalWidth not implemented");
    assert(0); // crash debug builds only
    return 0;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPhysicalHeight not implemented");
    assert(0); // crash debug builds only
    return 0;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorRefreshRate not implemented");
    assert(0); // crash debug builds only
    return 0;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorName not implemented");
    assert(0); // crash debug builds only
    return 0;
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    TRACELOG(LOG_WARNING, "GetWindowPosition not implemented");
    assert(0); // crash debug builds only
    return (Vector2){ 0, 0 };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    float scale = ScaleFromDpi(GetWindowDpi(globalHwnd));
    return (Vector2){ scale, scale };
}

void SetClipboardText(const char *text)
{
    TRACELOG(LOG_WARNING, "SetClipboardText not implemented");
    assert(0); // crash debug builds only
}

const char *GetClipboardText(void)
{
    TRACELOG(LOG_WARNING, "GetClipboardText not implemented");
    assert(0); // crash debug builds only
    return NULL;
}

Image GetClipboardImage(void)
{
    TRACELOG(LOG_WARNING, "GetClipboardText not implemented");
    assert(0); // crash debug builds only
    Image image = { 0 };
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
    //       to only hide the cursor while it's inside the client area.
    CORE.Input.Mouse.cursorHidden = true;
    SetCursor(NULL);
}

// Enables cursor (unlock cursor)
void EnableCursor(void)
{
    if (globalCursorEnabled)
    {
        TRACELOG(LOG_INFO, "EnableCursor: already enabled");
    }
    else
    {
        if (!ClipCursor(NULL))
        {
            LogFail("ClipCursor", GetLastError());
            abort();
        }

        {
            RAWINPUTDEVICE rid;
            rid.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
            rid.usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
            rid.dwFlags = RIDEV_REMOVE; // Add to this window even in background
            rid.hwndTarget = NULL;
            if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
                LogFail("RegisterRawInputDevices", GetLastError());
                abort();
            }
        }

        ShowCursor();
        globalCursorEnabled = true;
        TRACELOG(LOG_INFO, "EnableCursor: enabled");
    }
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    if (globalCursorEnabled)
    {

        {
            RAWINPUTDEVICE rid;
            rid.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
            rid.usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
            rid.dwFlags = RIDEV_INPUTSINK; // Add to this window even in background
            rid.hwndTarget = globalHwnd;
            if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
                LogFail("RegisterRawInputDevices", GetLastError());
                abort();
            }
        }

        RECT clientRect;
        if (!GetClientRect(globalHwnd, &clientRect))
        {
            LogFail("GetClientRect", GetLastError());
            abort();
        }

        POINT topleft = { clientRect.left, clientRect.top };
        if (!ClientToScreen(globalHwnd, &topleft))
        {
            LogFail("ClientToScreen", GetLastError());
            abort();
        }

        LONG width = clientRect.right - clientRect.left;
        LONG height = clientRect.bottom - clientRect.top;

        TRACELOG(LOG_INFO, "ClipCursor client %d,%d %d,%d (topleft %d,%d)",
            clientRect.left,
            clientRect.top,
            clientRect.right,
            clientRect.bottom,
            topleft.x,
            topleft.y
        );
        LONG centerX = topleft.x + width/2;
        LONG centerY = topleft.y + height/2;
        RECT clipRect = { centerX, centerY, centerX + 1, centerY + 1 };
        if (!ClipCursor(&clipRect))
        {
            LogFail("ClipCursor", GetLastError());
            abort();
        }

        CORE.Input.Mouse.previousPosition = (Vector2){ 0, 0 };
        CORE.Input.Mouse.currentPosition = (Vector2){ 0, 0 };
        HideCursor();

        globalCursorEnabled = false;
        TRACELOG(LOG_INFO, "DisableCursor: disabled");
    }
    else
    {
        TRACELOG(LOG_INFO, "DisableCursor: already disabled");
    }
}

void SwapScreenBuffer(void)
{
    if (!globalHdc) abort();
    if (!SwapBuffers(globalHdc))
    {
        LogFail("SwapBuffers", GetLastError());
        abort();
    }

    if (!ValidateRect(globalHwnd, NULL))
    {
        LogFail("ValidateRect", GetLastError());
        abort();
    }
}

double GetTime(void)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - CORE.Time.base)/(double)globalTimerFrequency.QuadPart;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    // Security check to (partially) avoid malicious code on target platform
    if (strchr(url, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else
    {
        TRACELOG(LOG_WARNING, "OpenURL not implemented");
        assert(0); // crash debug builds only
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

int SetGamepadMappings(const char *mappings)
{
    TRACELOG(LOG_WARNING, "SetGamepadMappings not implemented");
    assert(0); // crash debug builds only
    return -1;
}

void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration)
{
    TRACELOG(LOG_WARNING, "SetGamepadVibration not implemented");
    assert(0); // crash debug builds only
}

void SetMousePosition(int x, int y)
{
    if (globalCursorEnabled)
    {
        CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
        CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
        TRACELOG(LOG_WARNING, "SetMousePosition not implemented");
        assert(0); // crash debug builds only
    }
    else
    {
        TRACELOG(LOG_ERROR, "Possible? Should we allow this?");
        abort();
    }
}

void SetMouseCursor(int cursor)
{
    LPCWSTR cursorName = GetCursorName(cursor);
    HCURSOR hcursor = LoadCursorW(NULL, cursorName);
    if (!hcursor)
    {
        TRACELOG(LOG_ERROR, "LoadCursor %d (win32 %d) failed, error=%lu", cursor, (size_t)cursorName, GetLastError());
        abort();
    }

    SetCursor(hcursor);
    CORE.Input.Mouse.cursorHidden = false;
}

const char *GetKeyName(int key)
{
    TRACELOG(LOG_WARNING, "GetKeyName not implemented");
    assert(0); // crash debug builds only
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

    memcpy(
        CORE.Input.Keyboard.previousKeyState,
        CORE.Input.Keyboard.currentKeyState,
        sizeof(CORE.Input.Keyboard.previousKeyState)
    );
    memset(CORE.Input.Keyboard.keyRepeatInFrame, 0, sizeof(CORE.Input.Keyboard.keyRepeatInFrame));

    // Register previous mouse wheel state
    CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
    CORE.Input.Mouse.currentWheelMove = (Vector2){ 0.0f, 0.0f };

    // Register previous mouse position
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_PAINT)
            return;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

#define WM_APP_UPDATE_WINDOW_SIZE (WM_APP + 1)

static LRESULT WndProc2(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, FlagsOp *deferredFlags)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            globalHdc = GetDC(hwnd);
            if (!globalHdc)
            {
                LogFail("GetDC", GetLastError());
                return -1;
            }

            if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
            {
                TRACELOG(LOG_ERROR, "TODO: implement FLAG_MSAA_4X_HINT");
                assert(0);
            }

            PIXELFORMATDESCRIPTOR pixelFormatDesc = {0};
            pixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
            pixelFormatDesc.nVersion = 1;
            pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
            pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
            pixelFormatDesc.cColorBits = 32;
            pixelFormatDesc.cAlphaBits = 8;
            pixelFormatDesc.cDepthBits = 24;

            int pixelFormat = ChoosePixelFormat(globalHdc, &pixelFormatDesc);
            if (!pixelFormat)
            {
                LogFail("ChoosePixelFormat", GetLastError());
                return -1;
            }

            if (!SetPixelFormat(globalHdc, pixelFormat, &pixelFormatDesc))
            {
                LogFail("SetPixelFormat", GetLastError());
                return -1;
            }

            globalGlContext = wglCreateContext(globalHdc);
            if (!globalGlContext)
            {
                LogFail("wglCreateContext", GetLastError());
                return -1;
            }

            if (!wglMakeCurrent(globalHdc, globalGlContext))
            {
                LogFail("wglMakeCurrent", GetLastError());
                return -1;
            }

            rlLoadExtensions(WglGetProcAddress);

            globalHwnd = hwnd;

        } return 0;
        case WM_DESTROY:
            wglMakeCurrent(globalHdc, NULL);
            if (globalGlContext)
            {
                if (!wglDeleteContext(globalGlContext)) abort();
                globalGlContext = NULL;
            }

            if (globalHdc)
            {
                if (!ReleaseDC(hwnd, globalHdc)) abort();
                globalHdc = NULL;
            }

            return 0;
        case WM_CLOSE:
            CORE.Window.shouldClose = true;
            return 0;
        case WM_KILLFOCUS:
            memset(CORE.Input.Keyboard.previousKeyState, 0, sizeof(CORE.Input.Keyboard.previousKeyState));
            memset(CORE.Input.Keyboard.currentKeyState, 0, sizeof(CORE.Input.Keyboard.currentKeyState));
            return 0;
        case WM_SIZING:
            if (CORE.Window.flags & FLAG_WINDOW_RESIZABLE) {
                // TODO: enforce min/max size
                TRACELOG(LOG_WARNING, "TODO: enforce min/max size!");
            } else {
                TRACELOG(LOG_WARNING, "non-resizable window is being resized");
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                abort(); // TODO
            }
            return TRUE;
        case WM_STYLECHANGING:
            if (wparam == GWL_STYLE)
            {
                STYLESTRUCT *ss = (STYLESTRUCT*)lparam;
                GetStyleChangeFlagOps(CORE.Window.flags, ss, deferredFlags);

                UINT dpi = GetWindowDpi(hwnd);
                SIZE clientSize = GetClientSize(hwnd);
                SIZE oldSize = CalcWindowSize(dpi, clientSize, ss->styleOld);
                SIZE newSize = CalcWindowSize(dpi, clientSize, ss->styleNew);
                if (oldSize.cx != newSize.cx || oldSize.cy != newSize.cy) {
                    TRACELOG(
                        LOG_INFO,
                        "resize from style change: %dx%d to %dx%d",
                        oldSize.cx, oldSize.cy,
                        newSize.cx, newSize.cy
                    );
                    if (CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) {
                        // looks like windows will automatically "unminimize" a window
                        // if a style changes modifies it's size
                        TRACELOG(LOG_INFO, "style change modifed window size, removing maximized flag");
                        deferredFlags->clear |= FLAG_WINDOW_MAXIMIZED;
                    }
                }
            }
            return 0;
        case WM_WINDOWPOSCHANGING:
        {
            WINDOWPOS *pos = (WINDOWPOS*)lparam;
            if (pos->flags & SWP_SHOWWINDOW)
            {
                if (pos->flags & SWP_HIDEWINDOW) abort();
                deferredFlags->clear |= FLAG_WINDOW_HIDDEN;
            }
            else if (pos->flags & SWP_HIDEWINDOW)
            {
                deferredFlags->set |= FLAG_WINDOW_HIDDEN;
            }

            Mized mized = MIZED_NONE;
            if (IsMinimized2(hwnd))
            {
                mized = MIZED_MIN;
            }
            else
            {
                WINDOWPLACEMENT placement;
                placement.length = sizeof(placement);
                if (!GetWindowPlacement(hwnd, &placement))
                {
                    LogFail("GetWindowPlacement", GetLastError());
                    abort();
                }

                if (placement.showCmd == SW_SHOWMAXIMIZED) {
                    mized = MIZED_MAX;
                }
            }

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            /* TRACELOG(LOG_INFO, "window pos=%d,%d size=%dx%d", pos->x, pos->y, pos->cx, pos->cy); */

            switch (mized)
            {
                case MIZED_NONE:
                {
                    deferredFlags->clear |= (
                        FLAG_WINDOW_MINIMIZED |
                        FLAG_WINDOW_MAXIMIZED
                    );
                    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
                    MONITORINFO info;
                    info.cbSize = sizeof(info);
                    if (!GetMonitorInfoW(monitor, &info))
                    {
                        LogFail("GetMonitorInfo", GetLastError());
                        abort();
                    }

                    if (
                        (pos->x == info.rcMonitor.left) &&
                        (pos->y == info.rcMonitor.top) &&
                        (pos->cx == (info.rcMonitor.right - info.rcMonitor.left)) &&
                        (pos->cy == (info.rcMonitor.bottom - info.rcMonitor.top))
                    ) {
                        deferredFlags->set |= FLAG_BORDERLESS_WINDOWED_MODE;
                    }
                    else
                    {
                        deferredFlags->clear |= FLAG_BORDERLESS_WINDOWED_MODE;
                    }

                } break;
                case MIZED_MIN:
                    // !!! NOTE !!! Do not update the maximized/borderless
                    // flags because when hwnd is minimized it temporarily overrides
                    // the maximized state/flag which gets restored on SW_RESTORE
                    deferredFlags->set |= FLAG_WINDOW_MINIMIZED;
                    break;
                case MIZED_MAX:
                    deferredFlags->clear |= FLAG_WINDOW_MINIMIZED;
                    deferredFlags->set |= FLAG_WINDOW_MAXIMIZED;
                    break;
            }

            return 0;
        }
        case WM_SIZE:
            // don't trust the docs, they say you won't get this message if you don't call DefWindowProc
            // in response to WM_WINDOWPOSCHANGED but looks like when a window is created you'll get this
            // message without getting WM_WINDOWPOSCHANGED.
            HandleWindowResize(hwnd, &globalAppScreenSize);
            return 0;
        case WM_WINDOWPOSCHANGED: {
            WINDOWPOS *pos = (WINDOWPOS*)lparam;
            if (!(pos->flags & SWP_NOSIZE))
            {
                HandleWindowResize(hwnd, &globalAppScreenSize);
            }

            return 0;
        }
        case WM_GETDPISCALEDSIZE:
        {
            SIZE *inoutSize = (SIZE*)lparam;
            UINT newDpi = wparam;

            // for any of these other cases, we might want to post a window
            // resize event after the dpi changes?
            if (CORE.Window.flags & FLAG_WINDOW_MINIMIZED) return TRUE;
            if (CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) return TRUE;
            if (CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) return TRUE;

            float dpiScale = ScaleFromDpi(newDpi);
            bool dpiScaling = CORE.Window.flags & FLAG_WINDOW_HIGHDPI;
            SIZE desired = PxFromPt2(dpiScale, dpiScaling, globalAppScreenSize);
            inoutSize->cx = desired.cx;
            inoutSize->cy = desired.cy;
        } return TRUE;
        case WM_DPICHANGED:
        {
            RECT *suggestedRect = (RECT*)lparam;
            // Never set the window size to anything other than the suggested rect here.
            // Doing so can cause a window to stutter between monitors when transitioning
            // between them.
            if (!SetWindowPos(
                hwnd,
                NULL,
                suggestedRect->left,
                suggestedRect->top,
                suggestedRect->right - suggestedRect->left,
                suggestedRect->bottom - suggestedRect->top,
                SWP_NOZORDER | SWP_NOACTIVATE
            )) {
                LogFail("SetWindowPos", GetLastError());
                abort();
            }

        } return 0;
        case WM_SETCURSOR:
            if (LOWORD(lparam) == HTCLIENT)
            {
                SetCursor(CORE.Input.Mouse.cursorHidden? NULL : LoadCursorW(NULL, (LPCWSTR)IDC_ARROW));
                return 0;
            }

            return DefWindowProc(hwnd, msg, wparam, lparam);
        case WM_INPUT:
            if (globalCursorEnabled)
            {
                TRACELOG(LOG_ERROR, "Unexpected raw mouse input"); // impossible right?
                abort();
            }

            HandleRawInput(lparam);
            return 0;
        case WM_MOUSEMOVE:
            if (globalCursorEnabled)
            {
                CORE.Input.Mouse.currentPosition.x = (float)GET_X_LPARAM(lparam);
                CORE.Input.Mouse.currentPosition.y = (float)GET_Y_LPARAM(lparam);
                CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;
            }

            return 0;
        case WM_KEYDOWN: HandleKey(wparam, lparam, 1); return 0;
        case WM_KEYUP: HandleKey(wparam, lparam, 0); return 0;
        case WM_LBUTTONDOWN: HandleMouseButton(MOUSE_BUTTON_LEFT, 1); return 0;
        case WM_LBUTTONUP  : HandleMouseButton(MOUSE_BUTTON_LEFT, 0); return 0;
        case WM_RBUTTONDOWN: HandleMouseButton(MOUSE_BUTTON_RIGHT, 1); return 0;
        case WM_RBUTTONUP  : HandleMouseButton(MOUSE_BUTTON_RIGHT, 0); return 0;
        case WM_MBUTTONDOWN: HandleMouseButton(MOUSE_BUTTON_MIDDLE, 1); return 0;
        case WM_MBUTTONUP  : HandleMouseButton(MOUSE_BUTTON_MIDDLE, 0); return 0;
        case WM_XBUTTONDOWN: switch (HIWORD(wparam))
        {
            case XBUTTON1: HandleMouseButton(MOUSE_BUTTON_SIDE, 1); return 0;
            case XBUTTON2: HandleMouseButton(MOUSE_BUTTON_EXTRA, 1); return 0;
            default:
                TRACELOG(LOG_WARNING, "TODO: handle ex mouse button DOWN wparam=%u", HIWORD(wparam));
                return 0;
        }
        case WM_XBUTTONUP: switch (HIWORD(wparam))
        {
            case XBUTTON1: HandleMouseButton(MOUSE_BUTTON_SIDE, 0); return 0;
            case XBUTTON2: HandleMouseButton(MOUSE_BUTTON_EXTRA, 0); return 0;
            default:
                TRACELOG(LOG_WARNING, "TODO: handle ex mouse button UP   wparam=%u", HIWORD(wparam));
                return 0;
        }
        case WM_MOUSEWHEEL:
            CORE.Input.Mouse.currentWheelMove.y = ((float)GET_WHEEL_DELTA_WPARAM(wparam))/WHEEL_DELTA;
            return 0;
        case WM_MOUSEHWHEEL:
            CORE.Input.Mouse.currentWheelMove.x = ((float)GET_WHEEL_DELTA_WPARAM(wparam))/WHEEL_DELTA;
            return 0;
        case WM_APP_UPDATE_WINDOW_SIZE:
            UpdateWindowSize(UPDATE_WINDOW_NORMAL, hwnd, globalAppScreenSize, CORE.Window.flags);
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // sanity check, should we remove this?
    DWORD mask = STYLE_MASK_ALL;
    if (globalHwnd == hwnd)
    {
        if (msg == WM_WINDOWPOSCHANGING) {
            mask &= ~(WS_MINIMIZE | WS_MAXIMIZE);
        }
        CheckFlags("WndProc", hwnd, CORE.Window.flags, MakeWindowStyle(CORE.Window.flags), mask);
    }

    FlagsOp flagsOp;
    flagsOp.set = 0;
    flagsOp.clear = 0;
    LRESULT result = WndProc2(hwnd, msg, wparam, lparam, &flagsOp);

    // sanity check, should we remove this?
    if (globalHwnd == hwnd)
     {
        CheckFlags("After WndProc", hwnd, CORE.Window.flags, MakeWindowStyle(CORE.Window.flags), mask);
    }

    // Operations to execute after the above check
    if (flagsOp.set & flagsOp.clear)
    {
        TRACELOG(LOG_ERROR, "the flags 0x%x were both set and cleared!", flagsOp.set & flagsOp.clear);
        abort();
    }

    {
        DWORD save = CORE.Window.flags;
        CORE.Window.flags |= flagsOp.set;
        CORE.Window.flags &= ~flagsOp.clear;
        if (save != CORE.Window.flags)
        {
            TRACELOG(
                LOG_DEBUG,
                "DeferredFlags: 0x%x > 0x%x (diff 0x%x)",
                save,
                CORE.Window.flags,
                save ^ CORE.Window.flags
            );
        }
    }

    return result;
}


int InitPlatform(void)
{
    globalDesiredFlags = SanitizeFlags(SANITIZE_FLAGS_FIRST, CORE.Window.flags);
    // from this point CORE.Window.flags should always reflect the actual state of the window
    CORE.Window.flags = FLAG_WINDOW_HIDDEN | (globalDesiredFlags & FLAG_MASK_NO_UPDATE);

    globalAppScreenSize = (Vector2){ CORE.Window.screen.width, CORE.Window.screen.height };

    if (IsWindows10Version1703OrGreaterWin32())
    {
        TRACELOG(LOG_INFO, "DpiAware: >=Win10Creators");
        if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
            LogFail("SetProcessDpiAwarenessContext", GetLastError());
            abort();
        }
    }
    else
    {
        TRACELOG(LOG_INFO, "DpiAware: <Win10Creators");
        HRESULT hr = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        if (hr < 0) {
            LogFailHr("SetProcessDpiAwareness", hr);
            abort();
        }
    }

    {
        WNDCLASSEXW c = {0};
        c.cbSize = sizeof(c);
        c.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        c.lpfnWndProc = WndProc;
        c.cbWndExtra = sizeof(LONG_PTR); // extra space for the Tuple object ptr
        c.hInstance = GetModuleHandleW(0);
        // TODO: audit if we want to set this since we're implementing WM_SETCURSOR
        c.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
        c.lpszClassName = CLASS_NAME;
        if (0 == RegisterClassExW(&c))
        {
            LogFail("RegisterClass", GetLastError());
            return -1;
        }
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // TODO: probably remove or at least move this code that sets the display size.
    //       should maybe go somewhere in WndProc?
    HMONITOR monitor;

    {
        POINT primaryTopLeft = {0, 0};
        monitor = MonitorFromPoint(primaryTopLeft, MONITOR_DEFAULTTOPRIMARY);
        if (!monitor)
        {
            LogFail("MonitorFromPoint", GetLastError());
            // we'll keep going
        }
        else
        {
            MONITORINFO info;
            info.cbSize = sizeof(info);
            if (!GetMonitorInfoW(monitor, &info))
            {
                LogFail("GetMonitorInfo", GetLastError());
            }
            else
            {
                CORE.Window.display.width = info.rcMonitor.right - info.rcMonitor.left;
                CORE.Window.display.height = info.rcMonitor.bottom - info.rcMonitor.top;
            }
        }
    }

    CreateWindowAlloca(CORE.Window.title, MakeWindowStyle(CORE.Window.flags));
    if (!globalHwnd)
    {
        LogFail("CreateWindow", GetLastError());
        return -1;
    }

    CORE.Window.ready = true;
    UpdateWindowSize(UPDATE_WINDOW_FIRST, globalHwnd, globalAppScreenSize, globalDesiredFlags);
    UpdateFlags(globalHwnd, globalDesiredFlags, globalAppScreenSize);

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // TODO: not sure why this needs to be here?
    CORE.Window.currentFbo.width = CORE.Window.render.width;
    CORE.Window.currentFbo.height = CORE.Window.render.height;
    TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
    TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
    TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);

    CORE.Storage.basePath = GetWorkingDirectory();

    QueryPerformanceFrequency(&globalTimerFrequency);

    {
        LARGE_INTEGER time;
        QueryPerformanceCounter(&time);
        CORE.Time.base = time.QuadPart;
    }
    InitTimer();

    TRACELOG(LOG_INFO, "PLATFORM: DESKTOP: WIN32: Initialized successfully");
    return 0;
}

void ClosePlatform(void)
{
    if (globalHwnd)
    {
        if (0 == DestroyWindow(globalHwnd))
        {
            LogFail("DestroyWindow", GetLastError());
            abort();
        }

        globalHwnd = NULL;
    }
}
