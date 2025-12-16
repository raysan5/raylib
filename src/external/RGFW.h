/*
*
*	RGFW 1.8.1

* Copyright (C) 2022-25 Riley Mabb (@ColleagueRiley)
*
* libpng license
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.

* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
*
*/

/*
	(MAKE SURE RGFW_IMPLEMENTATION is in exactly one header or you use -D RGFW_IMPLEMENTATION)
	#define RGFW_IMPLEMENTATION - makes it so source code is included with header
*/

/*
	#define RGFW_IMPLEMENTATION - (required) makes it so the source code is included
	#define RGFW_DEBUG - (optional) makes it so RGFW prints debug messages and errors when they're found
	#define RGFW_EGL - (optional) compile with OpenGL functions, allowing you to use to use EGL instead of the native OpenGL functions
	#define RGFW_DIRECTX - (optional) include integration directX functions (windows only)
	#define RGFW_VULKAN - (optional) include helpful vulkan integration functions and macros
	#define RGFW_WEBGPU - (optional) use WebGPU for rendering
	#define RGFW_NATIVE - (optional) define native RGFW types that use native API structures

	#define RGFW_X11 (optional) (unix only) if X11 should be used. This option is turned on by default by unix systems except for MacOS
	#define RGFW_WAYLAND (optional) (unix only) use Wayland. (This can be used with X11)
	#define RGFW_NO_X11 (optional) (unix only) don't fallback to X11 when using Wayland
	#define RGFW_NO_LOAD_WGL (optional) (windows only) if WGL should be loaded dynamically during runtime
	#define RGFW_NO_X11_CURSOR (optional) (unix only) don't use XCursor
	#define RGFW_NO_X11_CURSOR_PRELOAD (optional) (unix only) use XCursor, but don't link it in code, (you'll have to link it with -lXcursor)
	#define RGFW_NO_X11_EXT_PRELOAD (optional) (unix only) use Xext, but don't link it in code, (you'll have to link it with -lXext)
    #define RGFW_NO_LOAD_WINMM (optional) (windows only) use winmm (timeBeginPeriod), but don't link it in code, (you'll have to link it with -lwinmm)
	#define RGFW_NO_WINMM (optional) (windows only) don't use winmm
	#define RGFW_NO_IOKIT (optional) (macOS) don't use IOKit
	#define RGFW_NO_UNIX_CLOCK (optional) (unix) don't link unix clock functions
	#define RGFW_NO_DWM (windows only) - do not use or link dwmapi
	#define RGFW_USE_XDL (optional) (X11) if XDL (XLib Dynamic Loader) should be used to load X11 dynamically during runtime (must include XDL.h along with RGFW)
	#define RGFW_COCOA_GRAPHICS_SWITCHING - (optional) (cocoa) use automatic graphics switching (allow the system to choose to use GPU or iGPU)
	#define RGFW_COCOA_FRAME_NAME (optional) (cocoa) set frame name
	#define RGFW_NO_DPI - do not calculate DPI (no XRM nor libShcore included)
    #define RGFW_ADVANCED_SMOOTH_RESIZE - use advanced methods for smooth resizing (may result in a spike in memory usage or worse performance) (eg. WM_TIMER and XSyncValue)
    #define RGFW_NO_INFO - do not define the RGFW_info struct (without RGFW_IMPLEMENTATION)
	#define RGFW_NO_GLXWINDOW - do not use GLXWindow

	#define RGFW_ALLOC x  - choose the default allocation function (defaults to standard malloc)
	#define RGFW_FREE x  - choose the default deallocation function (defaults to standard free)
	#define RGFW_USERPTR x - choose the default userptr sent to the malloc call, (NULL by default)

	#define RGFW_EXPORT - use when building RGFW
	#define RGFW_IMPORT - use when linking with RGFW (not as a single-header)

	#define RGFW_USE_INT - force the use c-types rather than stdint.h (for systems that might not have stdint.h (msvc))
	#define RGFW_bool x - choose what type to use for bool, by default u32 is used
*/

/*
Example to get you started :

linux : gcc main.c -lX11 -lXrandr -lGL
windows : gcc main.c -lopengl32 -lgdi32
macos : gcc main.c -framework Cocoa -framework CoreVideo -framework OpenGL -framework IOKit

#define RGFW_IMPLEMENTATION
#include "RGFW.h"

u8 icon[4 * 3 * 3] = {0xFF, 0x00, 0x00, 0xFF,    0xFF, 0x00, 0x00, 0xFF,     0xFF, 0x00, 0x00, 0xFF,   0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,     0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF};

int main() {
	RGFW_window* win = RGFW_createWindow("name", 100, 100, 500, 500, (u64)0);
	RGFW_event event;

	RGFW_window_setExitKey(win, RGFW_escape);
	RGFW_window_setIcon(win, icon, 3, 3, RGFW_formatRGBA8);

	while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
		while (RGFW_window_checkEvent(win, &event)) {
			if (event.type == RGFW_quit)
				break;
		}
	}

	RGFW_window_close(win);
}

	compiling :

	if you wish to compile the library all you have to do is create a new file with this in it

	rgfw.c
	#define RGFW_IMPLEMENTATION
	#include "RGFW.h"

	You may also want to add
	`#define RGFW_EXPORT` when compiling and
	`#define RGFW_IMPORT`when linking RGFW on it's own:
	this reduces inline functions and prevents bloat in the object file

	then you can use gcc (or whatever compile you wish to use) to compile the library into object file

	ex. gcc -c RGFW.c -fPIC

	after you compile the library into an object file, you can also turn the object file into an static or shared library

	(commands ar and gcc can be replaced with whatever equivalent your system uses)

	static : ar rcs RGFW.a RGFW.o
	shared :
		windows:
			gcc -shared RGFW.o -lopengl32 -lgdi32 -o RGFW.dll
		linux:
			gcc -shared RGFW.o -lX11 -lGL -lXrandr -o RGFW.so
		macos:
			gcc -shared RGFW.o -framework CoreVideo -framework Cocoa -framework OpenGL -framework IOKit
*/



/*
	Credits :
		EimaMei/Sacode : Code review, helped with X11, MacOS and Windows support, Silicon, siliapp.h -> referencing

		stb : This project is heavily inspired by the stb single header files

		SDL, GLFW and other online resources : reference implementations

		contributors : (feel free to put yourself here if you contribute)
		krisvers (@krisvers) -> code review
		EimaMei (@SaCode) -> code review
		Nycticebus (@Code-Nycticebus) -> bug fixes
		Rob Rohan (@robrohan) -> X11 bugs and missing features, MacOS/Cocoa fixing memory issues/bugs
		AICDG (@THISISAGOODNAME) -> vulkan support (example)
		@Easymode -> support, testing/debugging, bug fixes and reviews
		Joshua Rowe (omnisci3nce) - bug fix, review (macOS)
		@lesleyrs -> bug fix, review (OpenGL)
        Nick Porcino (@meshula) - testing, organization, review (MacOS, examples)
        @therealmarrakesh -> documentation
        @DarekParodia -> code review (X11) (C++)
        @NishiOwO -> fix BSD support, fix OSMesa example
        @BaynariKattu -> code review and documentation
        Miguel Pinto (@konopimi) -> code review, fix vulkan example
        @m-doescode -> code review (wayland)
        Robert Gonzalez (@uni-dos) -> code review (wayland)
        @TheLastVoyager -> code review
        @yehoravramenko -> code review (winapi)
        @halocupcake -> code review (OpenGL)
        @GideonSerf -> documentation
        Alexandre Almeida (@M374LX) -> code review (keycodes)
        Vũ Xuân Trường (@wanwanvxt) -> code review (winapi)
        Lucas (@lightspeedlucas) -> code review (msvc++)
        Jeffery Myers (@JeffM2501) -> code review (msvc)
        Zeni (@zenitsuyo) -> documentation
        TheYahton (@TheYahton) -> documentation
        nonexistant_object (@DiarrheaMcgee
*/

#if _MSC_VER
	#pragma comment(lib, "gdi32")
	#pragma comment(lib, "shell32")
	#pragma comment(lib, "User32")
    #pragma warning( push )
	#pragma warning( disable : 4996 4191 4127)
    #if _MSC_VER < 600
        #define RGFW_C89
    #endif
#else
    #if defined(__STDC__) && !defined(__STDC_VERSION__)
        #define RGFW_C89
    #endif
#endif

#if defined(RGFW_EGL) && !defined(RGFW_OPENGL)
	#define RGFW_OPENGL
#endif

/* these OS macros look better & are standardized */
/* plus it helps with cross-compiling */

#ifdef __EMSCRIPTEN__
	#define RGFW_WASM
#endif

#if defined(RGFW_X11) && defined(__APPLE__) && !defined(RGFW_CUSTOM_BACKEND)
	#define RGFW_MACOS_X11
	#define RGFW_UNIX
#endif

#if defined(_WIN32) && !defined(RGFW_X11) && !defined(RGFW_UNIX) && !defined(RGFW_WASM) && !defined(RGFW_CUSTOM_BACKEND) /* (if you're using X11 on windows some how) */
	#define RGFW_WINDOWS
#endif
#if defined(RGFW_WAYLAND)
	#define RGFW_DEBUG /* wayland will be in debug mode by default for now */
	#define RGFW_UNIX
	#ifdef RGFW_OPENGL
		#define RGFW_EGL
	#endif
	#ifdef RGFW_X11
		#define RGFW_DYNAMIC
	#endif
#endif
#if (!defined(RGFW_WAYLAND) && !defined(RGFW_X11)) && (defined(__unix__) || defined(RGFW_MACOS_X11) || defined(RGFW_X11))  && !defined(RGFW_WASM)  && !defined(RGFW_CUSTOM_BACKEND)
		#define RGFW_MACOS_X11
		#define RGFW_X11
		#define RGFW_UNIX
#elif defined(__APPLE__) && !defined(RGFW_MACOS_X11) && !defined(RGFW_X11)  && !defined(RGFW_WASM)  && !defined(RGFW_CUSTOM_BACKEND)
	#define RGFW_MACOS
#endif

#ifndef RGFW_ASSERT
	#include <assert.h>
	#define RGFW_ASSERT assert
#endif

#if !defined(__STDC_VERSION__)
    #define RGFW_C89
#endif

#if !defined(RGFW_SNPRINTF) && (defined(RGFW_X11) || defined(RGFW_WAYLAND))

	/* required for X11 errors */
	#include <stdio.h>

    #ifdef RGFW_C89
        #include <stdarg.h>
        static int RGFW_c89_snprintf(char *dst, size_t size, const char *format, ...) {
            va_list args;
            size_t count = 0;
            va_start(args, format);
            count = (size_t)vsprintf(dst, format, args);
            RGFW_ASSERT(count + 1 < size && "Buffer overflow");
            va_end(args);
            return (int)count;
        }
     	#define RGFW_SNPRINTF RGFW_c89_snprintf
    #else
     	#define RGFW_SNPRINTF snprintf
    #endif /*RGFW_C89*/
#endif

#ifndef RGFW_USERPTR
	#define RGFW_USERPTR NULL
#endif

#ifndef RGFW_UNUSED
	#define RGFW_UNUSED(x) (void)(x)
#endif

#ifndef RGFW_ROUND
	#define RGFW_ROUND(x) (i32)((x) >= 0 ? (x) + 0.5f : (x) - 0.5f)
#endif

#ifndef RGFW_MIN
	#define RGFW_MIN(x, y) ((x < y) ? x : y)
#endif

#ifndef RGFW_ALLOC
	#include <stdlib.h>
	#define RGFW_ALLOC malloc
	#define RGFW_FREE free
#endif

#if !defined(RGFW_MEMCPY) || !defined(RGFW_STRNCMP) || !defined(RGFW_STRNCPY) || !defined(RGFW_MEMSET)
	#include <string.h>
#endif

#ifndef RGFW_MEMSET
	#define RGFW_MEMSET(ptr, value, num) memset(ptr, value, num)
#endif

#ifndef RGFW_MEMCPY
	#define RGFW_MEMCPY(dist, src, len) memcpy(dist, src, len)
#endif

#ifndef RGFW_STRNCMP
	#define RGFW_STRNCMP(s1, s2, max) strncmp(s1, s2, max)
#endif

#ifndef RGFW_STRNCPY
	#define RGFW_STRNCPY(dist, src, len) strncpy(dist, src, len)
#endif

#ifndef RGFW_STRSTR
	#define RGFW_STRSTR(str, substr) strstr(str, substr)
#endif

#ifndef RGFW_STRTOL
	/* required for X11 XDnD and X11 Monitor DPI */
	#include <stdlib.h>
	#define RGFW_STRTOL(str, endptr, base) strtol(str, endptr, base)
	#define RGFW_ATOF(num) atof(num)
#endif

#if !defined(RGFW_PRINTF) && ( defined(RGFW_DEBUG) || defined(RGFW_WAYLAND) )
    /* required when using RGFW_DEBUG */
    #include <stdio.h>
    #define RGFW_PRINTF printf
#endif

#ifndef RGFW_MAX_PATH
	#define RGFW_MAX_PATH 260 /* max length of a path (for drag andn drop) */
#endif
#ifndef RGFW_MAX_DROPS
	#define RGFW_MAX_DROPS 260 /* max items you can drop at once */
#endif

#ifndef RGFW_MAX_EVENTS
	#define RGFW_MAX_EVENTS 32
#endif

#ifndef RGFW_MAX_MONITORS
	#define RGFW_MAX_MONITORS 6
#endif

#ifndef RGFW_COCOA_FRAME_NAME
	#define RGFW_COCOA_FRAME_NAME NULL
#endif

#ifdef RGFW_WIN95 /* for windows 95 testing (not that it really works) */
	#define RGFW_NO_MONITOR
	#define RGFW_NO_PASSTHROUGH
#endif

#if defined(RGFW_EXPORT) ||  defined(RGFW_IMPORT)
	#if defined(_WIN32)
		#if defined(__TINYC__) && (defined(RGFW_EXPORT) ||  defined(RGFW_IMPORT))
			#define __declspec(x) __attribute__((x))
		#endif

		#if defined(RGFW_EXPORT)
			#define RGFWDEF __declspec(dllexport)
		#else
			#define RGFWDEF __declspec(dllimport)
		#endif
	#else
		#if defined(RGFW_EXPORT)
			#define RGFWDEF __attribute__((visibility("default")))
		#endif
	#endif
    #ifndef RGFWDEF
        #define RGFWDEF
    #endif
#endif

#ifndef RGFWDEF
	#ifdef RGFW_C89
		#define RGFWDEF __inline
	#else
		#define RGFWDEF inline
	#endif
#endif

#if defined(__cplusplus) && !defined(__EMSCRIPTEN__)
	extern "C" {
#endif

/* makes sure the header file part is only defined once by default */
#ifndef RGFW_HEADER

#define RGFW_HEADER

#include <stddef.h>
#ifndef RGFW_INT_DEFINED
	#ifdef RGFW_USE_INT /* optional for any system that might not have stdint.h */
		typedef unsigned char       u8;
		typedef signed char         i8;
		typedef unsigned short     u16;
		typedef signed short 	   i16;
		typedef unsigned long int  u32;
		typedef signed long int    i32;
		typedef unsigned long long u64;
		typedef signed long long   i64;
	#else /* use stdint standard types instead of c "standard" types */
		#include <stdint.h>

		typedef uint8_t     u8;
		typedef int8_t      i8;
		typedef uint16_t   u16;
		typedef int16_t    i16;
		typedef uint32_t   u32;
		typedef int32_t    i32;
		typedef uint64_t   u64;
		typedef int64_t    i64;
	#endif
	#define RGFW_INT_DEFINED
#endif

typedef ptrdiff_t RGFW_ssize_t;

#ifndef RGFW_BOOL_DEFINED
    #define RGFW_BOOL_DEFINED
    typedef u8 RGFW_bool;
#endif

#define RGFW_BOOL(x) (RGFW_bool)((x) != 0) /* force a value to be 0 or 1 */
#define RGFW_TRUE (RGFW_bool)1
#define RGFW_FALSE (RGFW_bool)0

#define RGFW_ENUM(type, name) type name; enum
#define RGFW_BIT(x) (1 << (x))

#ifdef RGFW_VULKAN

	#if defined(RGFW_WAYLAND) && defined(RGFW_X11)
		#define VK_USE_PLATFORM_WAYLAND_KHR
		#define VK_USE_PLATFORM_XLIB_KHR
		#define RGFW_VK_SURFACE ((RGFW_usingWayland()) ? ("VK_KHR_wayland_surface") : ("VK_KHR_xlib_surface"))
	#elif defined(RGFW_WAYLAND)
		#define VK_USE_PLATFORM_WAYLAND_KHR
		#define VK_USE_PLATFORM_XLIB_KHR
		#define RGFW_VK_SURFACE "VK_KHR_wayland_surface"
	#elif defined(RGFW_X11)
		#define VK_USE_PLATFORM_XLIB_KHR
		#define RGFW_VK_SURFACE "VK_KHR_xlib_surface"
	#elif defined(RGFW_WINDOWS)
		#define VK_USE_PLATFORM_WIN32_KHR
		#define OEMRESOURCE
		#define RGFW_VK_SURFACE "VK_KHR_win32_surface"
	#elif defined(RGFW_MACOS) && !defined(RGFW_MACOS_X11)
		#define VK_USE_PLATFORM_MACOS_MVK
		#define RGFW_VK_SURFACE "VK_MVK_macos_surface"
	#else
		#define RGFW_VK_SURFACE NULL
	#endif

#endif


/*! @brief The stucture that contains information about the current RGFW instance */
typedef struct RGFW_info RGFW_info;

/*! @brief The window stucture for interfacing with the window */
typedef struct RGFW_window RGFW_window;

/*! @brief The source window stucture for interfacing with the underlying windowing API (e.g. winapi, wayland, cocoa, etc) */
typedef struct RGFW_window_src RGFW_window_src;

/*! @brief The color format for pixel data */
typedef RGFW_ENUM(u8, RGFW_format) {
	RGFW_formatRGB8 = 0,    /*!< 8-bit RGB (3 channels) */
	RGFW_formatBGR8,    /*!< 8-bit BGR (3 channels) */
	RGFW_formatRGBA8,   /*!< 8-bit RGBA (4 channels) */
	RGFW_formatARGB8,   /*!< 8-bit RGBA (4 channels) */
	RGFW_formatBGRA8,   /*!< 8-bit BGRA (4 channels) */
	RGFW_formatABGR8,   /*!< 8-bit BGRA (4 channels) */
	RGFW_formatCount
};

/*! @brief a stucture for interfacing with the underlying native image (e.g. XImage, HBITMAP, etc) */
typedef struct RGFW_nativeImage RGFW_nativeImage;

/*! @brief a stucture for interfacing with pixel data as a renderable surface */
typedef struct RGFW_surface RGFW_surface;

/*! a raw pointer to the underlying mouse handle for setting and creating custom mouse icons */
typedef void RGFW_mouse;

/*! @brief RGFW's abstract keycodes */
typedef RGFW_ENUM(u8, RGFW_key) {
	RGFW_keyNULL = 0,
	RGFW_escape = '\033',
	RGFW_backtick = '`',
	RGFW_0 = '0',
	RGFW_1 = '1',
	RGFW_2 = '2',
	RGFW_3 = '3',
	RGFW_4 = '4',
	RGFW_5 = '5',
	RGFW_6 = '6',
	RGFW_7 = '7',
	RGFW_8 = '8',
	RGFW_9 = '9',
	RGFW_minus = '-',
	RGFW_equals = '=',
	RGFW_backSpace = '\b',
	RGFW_tab = '\t',
	RGFW_space = ' ',
	RGFW_a = 'a',
	RGFW_b = 'b',
	RGFW_c = 'c',
	RGFW_d = 'd',
	RGFW_e = 'e',
	RGFW_f = 'f',
	RGFW_g = 'g',
	RGFW_h = 'h',
	RGFW_i = 'i',
	RGFW_j = 'j',
	RGFW_k = 'k',
	RGFW_l = 'l',
	RGFW_m = 'm',
	RGFW_n = 'n',
	RGFW_o = 'o',
	RGFW_p = 'p',
	RGFW_q = 'q',
	RGFW_r = 'r',
	RGFW_s = 's',
	RGFW_t = 't',
	RGFW_u = 'u',
	RGFW_v = 'v',
	RGFW_w = 'w',
	RGFW_x = 'x',
	RGFW_y = 'y',
	RGFW_z = 'z',
	RGFW_period = '.',
	RGFW_comma = ',',
	RGFW_slash = '/',
	RGFW_bracket = '[',
    RGFW_closeBracket = ']',
    RGFW_semicolon = ';',
	RGFW_apostrophe = '\'',
	RGFW_backSlash = '\\',
	RGFW_return = '\n',
	RGFW_enter = RGFW_return,
	RGFW_delete = '\177', /* 127 */
	RGFW_F1,
	RGFW_F2,
	RGFW_F3,
	RGFW_F4,
	RGFW_F5,
	RGFW_F6,
	RGFW_F7,
	RGFW_F8,
	RGFW_F9,
	RGFW_F10,
	RGFW_F11,
	RGFW_F12,
    RGFW_F13,
    RGFW_F14,
    RGFW_F15,
    RGFW_F16,
    RGFW_F17,
    RGFW_F18,
    RGFW_F19,
    RGFW_F20,
    RGFW_F21,
    RGFW_F22,
    RGFW_F23,
    RGFW_F24,
    RGFW_F25,
	RGFW_capsLock,
	RGFW_shiftL,
	RGFW_controlL,
	RGFW_altL,
	RGFW_superL,
	RGFW_shiftR,
	RGFW_controlR,
	RGFW_altR,
	RGFW_superR,
	RGFW_up,
	RGFW_down,
	RGFW_left,
	RGFW_right,
	RGFW_insert,
	RGFW_menu,
	RGFW_end,
	RGFW_home,
	RGFW_pageUp,
	RGFW_pageDown,
	RGFW_numLock,
	RGFW_kpSlash,
	RGFW_kpMultiply,
	RGFW_kpPlus,
	RGFW_kpMinus,
	RGFW_kpEqual,
	RGFW_kp1,
	RGFW_kp2,
	RGFW_kp3,
	RGFW_kp4,
	RGFW_kp5,
	RGFW_kp6,
	RGFW_kp7,
	RGFW_kp8,
	RGFW_kp9,
	RGFW_kp0,
	RGFW_kpPeriod,
	RGFW_kpReturn,
	RGFW_scrollLock,
    RGFW_printScreen,
    RGFW_pause,
	RGFW_world1,
    RGFW_world2,
    RGFW_keyLast = 256 /* padding for alignment ~(175 by default) */
};

/*! @brief abstract mouse button codes */
typedef RGFW_ENUM(u8, RGFW_mouseButton) {
	RGFW_mouseLeft = 0, /*!< left mouse button is pressed */
	RGFW_mouseMiddle, /*!< mouse-wheel-button is pressed */
	RGFW_mouseRight, /*!< right mouse button is pressed */
	RGFW_mouseMisc1, RGFW_mouseMisc2, RGFW_mouseMisc3, RGFW_mouseMisc4, RGFW_mouseMisc5,
	RGFW_mouseFinal
};

/*! abstract key modifier codes */
typedef RGFW_ENUM(u8, RGFW_keymod) {
	RGFW_modCapsLock = RGFW_BIT(0),
	RGFW_modNumLock  = RGFW_BIT(1),
	RGFW_modControl  = RGFW_BIT(2),
	RGFW_modAlt = RGFW_BIT(3),
	RGFW_modShift  = RGFW_BIT(4),
	RGFW_modSuper = RGFW_BIT(5),
	RGFW_modScrollLock = RGFW_BIT(6)
};

/*! @brief codes for the event types that can be sent */
typedef RGFW_ENUM(u8, RGFW_eventType) {
	RGFW_eventNone = 0, /*!< no event has been sent */
 	RGFW_keyPressed, /* a key has been pressed */
	RGFW_keyReleased, /*!< a key has been released */
	/*! key event note
		the code of the key pressed is stored in
		RGFW_event.key.value
		!!Keycodes defined at the bottom of the RGFW_HEADER part of this file!!

		while a string version is stored in
		RGFW_event.key.valueString

		RGFW_event.key.mod holds the current mod
		this means if CapsLock, NumLock are active or not
	*/
	RGFW_mouseButtonPressed, /*!< a mouse button has been pressed (left,middle,right) */
	RGFW_mouseButtonReleased, /*!< a mouse button has been released (left,middle,right) */
	RGFW_mouseScroll, /*!< a mouse scroll event */
	RGFW_mousePosChanged, /*!< the position of the mouse has been changed */
	/*! mouse event note
		the x and y of the mouse can be found in the vector, RGFW_x, y

		RGFW_event.button.value holds which mouse button was pressed
	*/
	RGFW_windowMoved, /*!< the window was moved (by the user) */
	RGFW_windowResized, /*!< the window was resized (by the user), [on WASM this means the browser was resized] */
	RGFW_focusIn, /*!< window is in focus now */
	RGFW_focusOut, /*!< window is out of focus now */
	RGFW_mouseEnter, /* mouse entered the window */
	RGFW_mouseLeave, /* mouse left the window */
	RGFW_windowRefresh, /* The window content needs to be refreshed */

	/* attribs change event note
		The event data is sent straight to the window structure
		with win->x, win->y, win->w and win->h
	*/
	RGFW_quit, /*!< the user clicked the quit button */
	RGFW_dataDrop, /*!< a file has been dropped into the window */
	RGFW_dataDrag, /*!< the start of a drag and drop event, when the file is being dragged */
	/* drop data note
		The x and y coords of the drop are stored in the vector RGFW_x, y

		RGFW_event.drop.count holds how many files were dropped

		This is also the size of the array which stores all the dropped file string,
		RGFW_event.drop.files
	*/
	RGFW_windowMaximized, /*!< the window was maximized */
	RGFW_windowMinimized, /*!< the window was minimized */
	RGFW_windowRestored, /*!< the window was restored */
	RGFW_scaleUpdated /*!< content scale factor changed */
};

/*! @brief flags for toggling wether or not an event should be processed */
typedef RGFW_ENUM(u32, RGFW_eventFlag) {
    RGFW_keyPressedFlag = RGFW_BIT(RGFW_keyPressed),
    RGFW_keyReleasedFlag = RGFW_BIT(RGFW_keyReleased),
    RGFW_mouseScrollFlag = RGFW_BIT(RGFW_mouseScroll),
    RGFW_mouseButtonPressedFlag = RGFW_BIT(RGFW_mouseButtonPressed),
    RGFW_mouseButtonReleasedFlag = RGFW_BIT(RGFW_mouseButtonReleased),
    RGFW_mousePosChangedFlag = RGFW_BIT(RGFW_mousePosChanged),
    RGFW_mouseEnterFlag = RGFW_BIT(RGFW_mouseEnter),
    RGFW_mouseLeaveFlag = RGFW_BIT(RGFW_mouseLeave),
    RGFW_windowMovedFlag = RGFW_BIT(RGFW_windowMoved),
    RGFW_windowResizedFlag = RGFW_BIT(RGFW_windowResized),
    RGFW_focusInFlag = RGFW_BIT(RGFW_focusIn),
    RGFW_focusOutFlag = RGFW_BIT(RGFW_focusOut),
    RGFW_windowRefreshFlag = RGFW_BIT(RGFW_windowRefresh),
    RGFW_windowMaximizedFlag = RGFW_BIT(RGFW_windowMaximized),
    RGFW_windowMinimizedFlag = RGFW_BIT(RGFW_windowMinimized),
    RGFW_windowRestoredFlag = RGFW_BIT(RGFW_windowRestored),
    RGFW_scaleUpdatedFlag = RGFW_BIT(RGFW_scaleUpdated),
    RGFW_quitFlag = RGFW_BIT(RGFW_quit),
    RGFW_dataDropFlag = RGFW_BIT(RGFW_dataDrop),
    RGFW_dataDragFlag = RGFW_BIT(RGFW_dataDrag),

    RGFW_keyEventsFlag = RGFW_keyPressedFlag | RGFW_keyReleasedFlag,
    RGFW_mouseEventsFlag = RGFW_mouseButtonPressedFlag | RGFW_mouseButtonReleasedFlag | RGFW_mousePosChangedFlag | RGFW_mouseEnterFlag | RGFW_mouseLeaveFlag | RGFW_mouseScrollFlag ,
    RGFW_windowEventsFlag = RGFW_windowMovedFlag | RGFW_windowResizedFlag | RGFW_windowRefreshFlag | RGFW_windowMaximizedFlag | RGFW_windowMinimizedFlag | RGFW_windowRestoredFlag | RGFW_scaleUpdatedFlag,
    RGFW_focusEventsFlag = RGFW_focusInFlag | RGFW_focusOutFlag,
    RGFW_dataDropEventsFlag = RGFW_dataDropFlag | RGFW_dataDragFlag,
    RGFW_allEventFlags = RGFW_keyEventsFlag | RGFW_mouseEventsFlag | RGFW_windowEventsFlag | RGFW_focusEventsFlag | RGFW_dataDropEventsFlag | RGFW_quitFlag
};

/*! Event structure(s) and union for checking/getting events */

/*! @brief common event data across all events */
typedef struct RGFW_commonEvent {
	RGFW_eventType type; /*!< which event has been sent?*/
	RGFW_window* win; /*!< the window this event applies too (for event queue events) */
} RGFW_commonEvent;

/*! @brief event data for any mouse button event (press/release) */
typedef struct RGFW_mouseButtonEvent {
	RGFW_eventType type; /*!< which event has been sent?*/
	RGFW_window* win; /*!< the window this event applies too (for event queue events) */
	u8 value; /* !< which mouse button was pressed */
} RGFW_mouseButtonEvent;

/*! @brief event data for any mouse scroll event */
typedef struct RGFW_mouseScrollEvent {
	RGFW_eventType type; /*!< which event has been sent?*/
	RGFW_window* win; /*!< the window this event applies too (for event queue events) */
	float x, y; /*!< the raw mouse scroll value */
} RGFW_mouseScrollEvent;

/*! @brief event data for any mouse position event (RGFW_mousePosChanged) */
typedef struct RGFW_mousePosEvent {
	RGFW_eventType type; /*!< which event has been sent?*/
	RGFW_window* win; /*!< the window this event applies too (for event queue events) */
	i32 x, y; /*!< mouse x, y of event (or drop point) */
	float vecX, vecY; /*!< raw mouse movement */
} RGFW_mousePosEvent;

/*! @brief event data for any key event (press/release) */
typedef struct RGFW_keyEvent {
	RGFW_eventType type; /*!< which event has been sent?*/
	RGFW_window* win; /*!< the window this event applies too (for event queue events) */
	RGFW_key value; /*!< the physical key of the event, refers to where key is physically !!Keycodes defined at the bottom of the RGFW_HEADER part of this file!! */
	u8 sym; /*!< mapped key char of the event */
	RGFW_bool repeat; /*!< key press event repeated (the key is being held) */
	RGFW_keymod mod;
} RGFW_keyEvent;

/*! @brief event data for any data drop event */
typedef struct RGFW_dataDropEvent {
	RGFW_eventType type; /*!< which event has been sent?*/
	RGFW_window* win; /*!< the window this event applies too (for event queue events) */
	/* 260 max paths with a max length of 260 */
	char** files; /*!< dropped files */
	size_t count; /*!< how many files were dropped */
} RGFW_dataDropEvent;

/*! @brief event data for any data drag event */
typedef struct RGFW_dataDragEvent {
	RGFW_eventType type; /*!< which event has been sent?*/
	RGFW_window* win; /*!< the window this event applies too (for event queue events) */
	i32 x, y; /*!< mouse x, y of event (or drop point) */
} RGFW_dataDragEvent;

/*! @brief event data for when the window scale (DPI) is updated */
typedef struct RGFW_scaleUpdatedEvent {
	RGFW_eventType type; /*!< which event has been sent?*/
	RGFW_window* win; /*!< the window this event applies too (for event queue events) */
	float x, y; /*!< DPI scaling */
} RGFW_scaleUpdatedEvent;

/*! @brief union for all of the event stucture types */
typedef union RGFW_event {
	RGFW_eventType type; /*!< which event has been sent?*/
	RGFW_commonEvent common; /*!< common event data (e.g.) type and win */
	RGFW_mouseButtonEvent button; /*!< data for a button press/release */
	RGFW_mouseScrollEvent scroll; /*!< data for a mouse scroll */
	RGFW_mousePosEvent mouse; /*!< data for mouse motion events */
	RGFW_keyEvent key; /*!< data for key press/release/hold events */
	RGFW_dataDropEvent drop; /*!< dropping a file events */
	RGFW_dataDragEvent drag; /* data for dragging a file events */
	RGFW_scaleUpdatedEvent scale; /* data for monitor scaling events */
} RGFW_event;

/*!
	@!brief codes for for RGFW_the code is stupid and C++ waitForEvent
	waitMS -> Allows the function to keep checking for events even after there are no more events
			  if waitMS == 0, the loop will not wait for events
			  if waitMS > 0, the loop will wait that many miliseconds after there are no more events until it returns
			  if waitMS == -1 or waitMS == the max size of an unsigned 32-bit int, the loop will not return until it gets another event
*/
typedef RGFW_ENUM(i32, RGFW_eventWait) {
	RGFW_eventNoWait = 0,
	RGFW_eventWaitNext = -1
};



/*! @brief optional bitwise arguments for making a windows, these can be OR'd together */
typedef RGFW_ENUM(u32, RGFW_windowFlags) {
	RGFW_windowNoBorder = RGFW_BIT(0), /*!< the window doesn't have a border */
	RGFW_windowNoResize = RGFW_BIT(1), /*!< the window cannot be resized by the user */
	RGFW_windowAllowDND = RGFW_BIT(2), /*!< the window supports drag and drop */
	RGFW_windowHideMouse = RGFW_BIT(3), /*! the window should hide the mouse (can be toggled later on using `RGFW_window_showMouse`) */
	RGFW_windowFullscreen = RGFW_BIT(4), /*!< the window is fullscreen by default */
	RGFW_windowTransparent = RGFW_BIT(5), /*!< the window is transparent (only properly works on X11 and MacOS, although it's meant for for windows) */
	RGFW_windowCenter = RGFW_BIT(6), /*! center the window on the screen */
	RGFW_windowScaleToMonitor = RGFW_BIT(8), /*! scale the window to the screen */
	RGFW_windowHide = RGFW_BIT(9), /*! the window is hidden */
	RGFW_windowMaximize = RGFW_BIT(10), /*!< maximize the window on creation */
	RGFW_windowCenterCursor = RGFW_BIT(11), /*!< center the cursor to the window on creation */
	RGFW_windowFloating = RGFW_BIT(12), /*!< create a floating window */
	RGFW_windowFocusOnShow = RGFW_BIT(13), /*!< focus the window when it's shown */
	RGFW_windowMinimize = RGFW_BIT(14), /*!< focus the window when it's shown */
	RGFW_windowFocus = RGFW_BIT(15), /*!< if the window is in focus */
	RGFW_windowOpenGL = RGFW_BIT(17), /*!< create an OpenGL context (you can also do this manually with RGFW_window_createContext_OpenGL) */
	RGFW_windowEGL = RGFW_BIT(18), /*!< create an EGL context (you can also do this manually with RGFW_window_createContext_EGL) */
	RGFW_windowedFullscreen = RGFW_windowNoBorder | RGFW_windowMaximize
};

/*! @brief the types of icon to set */
typedef RGFW_ENUM(u8, RGFW_icon) {
	RGFW_iconTaskbar = RGFW_BIT(0),
	RGFW_iconWindow = RGFW_BIT(1),
	RGFW_iconBoth = RGFW_iconTaskbar | RGFW_iconWindow
};

/*! @brief standard mouse icons */
typedef RGFW_ENUM(u8, RGFW_mouseIcons) {
	RGFW_mouseNormal = 0,
	RGFW_mouseArrow,
	RGFW_mouseIbeam,
	RGFW_mouseCrosshair,
	RGFW_mousePointingHand,
	RGFW_mouseResizeEW,
	RGFW_mouseResizeNS,
	RGFW_mouseResizeNWSE,
	RGFW_mouseResizeNESW,
	RGFW_mouseResizeAll,
	RGFW_mouseNotAllowed,
	RGFW_mouseIconCount,
    RGFW_mouseIconFinal = 16 /* padding for alignment */
};

/*! @brief the type of debug message */
typedef RGFW_ENUM(u8, RGFW_debugType) {
	RGFW_typeError = 0, RGFW_typeWarning, RGFW_typeInfo
};

/*! @brief error codes for known failure types */
typedef RGFW_ENUM(u8, RGFW_errorCode) {
	RGFW_noError = 0, /*!< no error */
	RGFW_errOutOfMemory,
	RGFW_errOpenGLContext, RGFW_errEGLContext, /*!< error with the OpenGL context */
	RGFW_errWayland, RGFW_errX11,
	RGFW_errDirectXContext,
	RGFW_errIOKit,
	RGFW_errClipboard,
	RGFW_errFailedFuncLoad,
	RGFW_errBuffer,
	RGFW_errEventQueue,
	RGFW_infoMonitor, RGFW_infoWindow, RGFW_infoBuffer, RGFW_infoGlobal, RGFW_infoOpenGL,
	RGFW_warningWayland, RGFW_warningOpenGL
};

/*! @brief callback function type for debug messags */
typedef void (* RGFW_debugfunc)(RGFW_debugType type, RGFW_errorCode err, const char* msg);

/*! @brief RGFW_windowMoved, the window and its new rect value  */
typedef void (* RGFW_windowMovedfunc)(RGFW_window* win, i32 x, i32 y);
/*! @brief RGFW_windowResized, the window and its new rect value  */
typedef void (* RGFW_windowResizedfunc)(RGFW_window* win, i32 w, i32 h);
/*! @brief RGFW_windowRestored, the window and its new rect value  */
typedef void (* RGFW_windowRestoredfunc)(RGFW_window* win, i32 x, i32 y, i32 w, i32 h);
/*! @brief RGFW_windowMaximized, the window and its new rect value  */
typedef void (* RGFW_windowMaximizedfunc)(RGFW_window* win, i32 x, i32 y, i32 w, i32 h);
/*! @brief RGFW_windowMinimized, the window and its new rect value  */
typedef void (* RGFW_windowMinimizedfunc)(RGFW_window* win);
/*! @brief RGFW_quit, the window that was closed */
typedef void (* RGFW_windowQuitfunc)(RGFW_window* win);
/*! @brief RGFW_focusIn / RGFW_focusOut, the window who's focus has changed and if its in focus */
typedef void (* RGFW_focusfunc)(RGFW_window* win, RGFW_bool inFocus);
/*! @brief RGFW_mouseEnter / RGFW_mouseLeave, the window that changed, the point of the mouse (enter only) and if the mouse has entered */
typedef void (* RGFW_mouseNotifyfunc)(RGFW_window* win, i32 x, i32 y, RGFW_bool status);
/*! @brief RGFW_mousePosChanged, the window that the move happened on, and the new point of the mouse  */
typedef void (* RGFW_mousePosfunc)(RGFW_window* win, i32 x, i32 y, float vecX, float vecY);
/*! @brief RGFW_dataDrag, the window, the point of the drop on the windows */
typedef void (* RGFW_dataDragfunc)(RGFW_window* win, i32 x, i32 y);
/*! @brief RGFW_windowRefresh, the window that needs to be refreshed */
typedef void (* RGFW_windowRefreshfunc)(RGFW_window* win);
/*! @brief RGFW_keyPressed / RGFW_keyReleased, the window that got the event, the mapped key, the physical key, the string version, the state of the mod keys, if it was a press (else it's a release) */
typedef void (* RGFW_keyfunc)(RGFW_window* win, u8 key, u8 sym, RGFW_keymod mod, RGFW_bool repeat, RGFW_bool pressed);
/*! @brief RGFW_mouseButtonPressed / RGFW_mouseButtonReleased, the window that got the event, the button that was pressed, the scroll value, if it was a press (else it's a release)  */
typedef void (* RGFW_mouseButtonfunc)(RGFW_window* win, RGFW_mouseButton button, RGFW_bool pressed);
/*! @brief RGFW_mouseScroll, the window that got the event, the x scroll value, the y scroll value */
typedef void (* RGFW_mouseScrollfunc)(RGFW_window* win, float x, float y);
/*! @brief RGFW_dataDrop the window that had the drop, the drop data and the number of files dropped */
typedef void (* RGFW_dataDropfunc)(RGFW_window* win, char** files, size_t count);
/*! @brief RGFW_scaleUpdated, the window the event was sent to, content scaleX, content scaleY */
typedef void (* RGFW_scaleUpdatedfunc)(RGFW_window* win, float scaleX, float scaleY);

/*! @brief function pointer equivalent of void* */
typedef void (*RGFW_proc)(void);

#ifndef RGFW_NO_MONITOR

/*! @brief monitor mode data | can be changed by the user (with functions)*/
typedef struct RGFW_monitorMode {
	i32 w, h; /*!< monitor workarea size */
	u32 refreshRate; /*!< monitor refresh rate */
	u8 red, blue, green;
} RGFW_monitorMode;

/*! @brief structure for monitor data */
typedef struct RGFW_monitor {
	i32 x, y; /*!< x - y of the monitor workarea */
	char name[128]; /*!< monitor name */
	float scaleX, scaleY; /*!< monitor content scale */
	float pixelRatio; /*!< pixel ratio for monitor (1.0 for regular, 2.0 for hiDPI)  */
	float physW, physH; /*!< monitor physical size in inches */
	RGFW_monitorMode mode;
} RGFW_monitor;

/*! @brief what type of request you are making for the monitor */
typedef RGFW_ENUM(u8, RGFW_modeRequest) {
	RGFW_monitorScale = RGFW_BIT(0), /*!< scale the monitor size */
	RGFW_monitorRefresh = RGFW_BIT(1), /*!< change the refresh rate */
	RGFW_monitorRGB = RGFW_BIT(2), /*!< change the monitor RGB bits size */
	RGFW_monitorAll = RGFW_monitorScale | RGFW_monitorRefresh | RGFW_monitorRGB
};

#endif

#if defined(RGFW_OPENGL)

/*! @brief abstract structure for interfacing with the underlying OpenGL API */
typedef struct RGFW_glContext RGFW_glContext;

/*! @brief abstract structure for interfacing with the underlying EGL API */
typedef struct RGFW_eglContext RGFW_eglContext;

/*! values for the releaseBehavior hint */
typedef RGFW_ENUM(i32, RGFW_glReleaseBehavior)   {
	RGFW_glReleaseFlush = 0, /*!< flush the pipeline will be flushed when the context is release */
	RGFW_glReleaseNone /*!< do nothing on release */
};

/*! values for the profile hint */
typedef RGFW_ENUM(i32, RGFW_glProfile)  {
	RGFW_glCore = 0, /*!< the core OpenGL version, e.g. just support for that version */
	RGFW_glCompatibility, /*!< allow compatibility for older versions of RGFW as well as the requested version */
	RGFW_glES /*!< use OpenGL ES */
};

/*! values for the renderer hint */
typedef RGFW_ENUM(i32, RGFW_glRenderer)  {
	RGFW_glAccelerated = 0, /*!< hardware accelerated (GPU) */
	RGFW_glSoftware /*!< software rendered (CPU) */
};

/*! OpenGL initalization hints */
typedef struct RGFW_glHints {
	i32 stencil;  /*!< set stencil buffer bit size (0 by default) */
	i32 samples; /*!< set number of sample buffers (0 by default) */
	i32 stereo; /*!< hint the context to use stereoscopic frame buffers for 3D (false by default) */
	i32 auxBuffers; /*!< number of aux buffers (0 by default) */
	i32 doubleBuffer; /*!< request double buffering (true by default) */
	i32 red, green, blue, alpha; /*!< set color bit sizes (all 8 by default) */
	i32 depth; /*!< set depth buffer bit size (24 by default) */
	i32 accumRed, accumGreen, accumBlue, accumAlpha; /*!< set accumulated RGBA bit sizes (all 0 by default) */
	RGFW_bool sRGB; /*!< request sRGA format (false by default) */
	RGFW_bool robustness; /*!< request a "robust" (as in memory-safe) context (false by default). For more information check the overview section: https://registry.khronos.org/OpenGL/extensions/EXT/EXT_robustness.txt */
	RGFW_bool debug; /*!< request OpenGL debugging (false by default). */
	RGFW_bool noError; /*!< request no OpenGL errors (false by default). This causes OpenGL errors to be undefined behavior. For more information check the overview section: https://registry.khronos.org/OpenGL/extensions/KHR/KHR_no_error.txt */
	RGFW_glReleaseBehavior releaseBehavior; /*!< hint how the OpenGL driver should behave when changing contexts (RGFW_glReleaseNone by default). For more information check the overview section: https://registry.khronos.org/OpenGL/extensions/KHR/KHR_context_flush_control.txt */
	RGFW_glProfile profile; /*!< set OpenGL API profile (RGFW_glCore by default) */
	i32 major, minor;  /*!< set the OpenGL API profile version (by default RGFW_glMajor is 1, RGFW_glMinor is 0) */
	RGFW_glContext* share; /*!< Share this OpenGL context with newly created OpenGL contexts; defaults to NULL. */
	RGFW_eglContext* shareEGL; /*!< Share this EGL context with newly created OpenGL contexts; defaults to NULL. */
	RGFW_glRenderer renderer; /*!< renderer to use e.g. accelerated or software defaults to accelerated */
} RGFW_glHints;

#endif

/**!
 * @brief Allocates memory using the allocator defined by RGFW_ALLOC at compile time.
 * @param size The size (in bytes) of the memory block to allocate.
 * @return A pointer to the allocated memory block.
*/
RGFWDEF void* RGFW_alloc(size_t size);

/**!
 * @brief Frees memory using the deallocator defined by RGFW_FREE at compile time.
 * @param ptr A pointer to the memory block to free.
*/
RGFWDEF void RGFW_free(void* ptr);

/**!
 * @brief Returns the size (in bytes) of the RGFW_window structure.
 * @return The size of the RGFW_window structure.
*/
RGFWDEF size_t RGFW_sizeofWindow(void);

/**!
 * @brief Returns the size (in bytes) of the RGFW_window_src structure.
 * @return The size of the RGFW_window_src structure.
*/
RGFWDEF size_t RGFW_sizeofWindowSrc(void);

/**!
 * @brief (Unix) Toggles the use of Wayland.
 * This is enabled by default when compiled with `RGFW_WAYLAND`.
 * If not using `RGFW_WAYLAND`, Wayland functions are not exposed.
 * This function can be used to force the use of XWayland.
 * @param wayland A boolean value indicating whether to use Wayland (true) or not (false).
*/
RGFWDEF void RGFW_useWayland(RGFW_bool wayland);

/**!
 * @brief Checks if Wayland is currently being used.
 * @return RGFW_TRUE if using Wayland, RGFW_FALSE otherwise.
*/
RGFWDEF RGFW_bool RGFW_usingWayland(void);

/**!
 * @brief Retrieves the current Cocoa layer (macOS only).
 * @return A pointer to the Cocoa layer, or NULL if the platform is not in use.
*/
RGFWDEF void* RGFW_getLayer_OSX(void);

/**!
 * @brief Retrieves the current X11 display connection.
 * @return A pointer to the X11 display, or NULL if the platform is not in use.
*/
RGFWDEF void* RGFW_getDisplay_X11(void);

/**!
 * @brief Retrieves the current Wayland display connection.
 * @return A pointer to the Wayland display (`struct wl_display*`), or NULL if the platform is not in use.
*/
RGFWDEF struct wl_display* RGFW_getDisplay_Wayland(void);

/**!
 * @brief Sets the class name for X11 and WinAPI windows.
 * Windows with the same class name will be grouped by the window manager.
 * By default, the class name matches the root window’s name.
 * @param name The class name to assign.
*/
RGFWDEF void RGFW_setClassName(const char* name);

/**!
 * @brief Sets the X11 instance name.
 * By default, the window name will be used as the instance name.
 * @param name The X11 instance name to set.
*/
RGFWDEF void RGFW_setXInstName(const char* name);

/**!
 * @brief (macOS only) Changes the current working directory to the application’s resource folder.
*/
RGFWDEF void RGFW_moveToMacOSResourceDir(void);

/*! copy image to another image, respecting each image's format */
RGFWDEF void RGFW_copyImageData(u8* dest_data, i32 w, i32 h, RGFW_format dest_format, u8* src_data, RGFW_format src_format);

/**!
 * @brief Returns the size (in bytes) of the RGFW_nativeImage structure.
 * @return The size of the RGFW_nativeImage structure.
*/
RGFWDEF size_t RGFW_sizeofNativeImage(void);

/**!
 * @brief Returns the size (in bytes) of the RGFW_surface structure.
 * @return The size of the RGFW_surface structure.
*/
RGFWDEF size_t RGFW_sizeofSurface(void);

/**!
 * @brief Creates a new surface from raw pixel data.
 * @param data A pointer to the pixel data buffer.
 * @param w The width of the surface in pixels.
 * @param h The height of the surface in pixels.
 * @param format The pixel format of the data.
 * @return A pointer to the newly created RGFW_surface.
 *
 * NOTE: when you create a surface using RGFW_createSurface / ptr, on X11 it uses the root window's visual
 * this means it may fail to render on any other window if the visual does not match
 * RGFW_window_createSurface and RGFW_window_createSurfacePtr exist only for X11 to address this issues
 * Of course, you can also manually set the root window with RGFW_setRootWindow
*/
RGFWDEF RGFW_surface* RGFW_createSurface(u8* data, i32 w, i32 h, RGFW_format format);

/**!
 * @brief Creates a surface using a pre-allocated RGFW_surface structure.
 * @param data A pointer to the pixel data buffer.
 * @param w The width of the surface in pixels.
 * @param h The height of the surface in pixels.
 * @param format The pixel format of the data.
 * @param surface A pointer to a pre-allocated RGFW_surface structure.
 * @return RGFW_TRUE if successful, RGFW_FALSE otherwise.
*/
RGFWDEF RGFW_bool RGFW_createSurfacePtr(u8* data, i32 w, i32 h, RGFW_format format, RGFW_surface* surface);

/**!
 * @brief Retrieves the native image associated with a surface.
 * @param surface A pointer to the RGFW_surface.
 * @return A pointer to the native RGFW_nativeImage associated with the surface.
*/
RGFWDEF RGFW_nativeImage* RGFW_surface_getNativeImage(RGFW_surface* surface);

/**!
 * @brief Frees the surface pointer and any buffers used for software rendering.
 * @param surface A pointer to the RGFW_surface to free.
*/
RGFWDEF void RGFW_surface_free(RGFW_surface* surface);

/**!
 * @brief Frees only the internal buffers used for software rendering, leaving the surface struct intact.
 * @param surface A pointer to the RGFW_surface whose buffers should be freed.
*/
RGFWDEF void RGFW_surface_freePtr(RGFW_surface* surface);


/**!
 * @brief Loads a mouse icon from bitmap data (similar to RGFW_window_setIcon).
 * @param data A pointer to the bitmap pixel data.
 * @param w The width of the mouse icon in pixels.
 * @param h The height of the mouse icon in pixels.
 * @param format The pixel format of the data.
 * @return A pointer to the newly loaded RGFW_mouse structure.
 *
 * @note The icon is not resized by default.
*/
RGFWDEF RGFW_mouse* RGFW_loadMouse(u8* data, i32 w, i32 h, RGFW_format format);

/**!
 * @brief Frees the data associated with an RGFW_mouse structure.
 * @param mouse A pointer to the RGFW_mouse to free.
*/
RGFWDEF void RGFW_freeMouse(RGFW_mouse* mouse);

#ifndef RGFW_NO_MONITOR

/**!
 * @brief Retrieves an array of all available monitors.
 * @param len [OUTPUT] A pointer to store the number of monitors found (maximum of 6).
 * @return A pointer to an array of RGFW_monitor structures.
*/
RGFWDEF RGFW_monitor* RGFW_getMonitors(size_t* len);

/**!
 * @brief Retrieves the primary monitor.
 * @return The RGFW_monitor structure representing the primary monitor.
*/
RGFWDEF RGFW_monitor RGFW_getPrimaryMonitor(void);

/**!
 * @brief Requests a specific display mode for a monitor.
 * @param mon The monitor to apply the mode change to.
 * @param mode The desired RGFW_monitorMode.
 * @param request The RGFW_modeRequest describing how to handle the mode change.
 * @return RGFW_TRUE if the mode was successfully applied, otherwise RGFW_FALSE.
*/
RGFWDEF RGFW_bool RGFW_monitor_requestMode(RGFW_monitor mon, RGFW_monitorMode mode, RGFW_modeRequest request);

/**!
 * @brief Compares two monitor modes to check if they are equivalent.
 * @param mon The first monitor mode.
 * @param mon2 The second monitor mode.
 * @param request The RGFW_modeRequest that defines the comparison parameters.
 * @return RGFW_TRUE if both modes are equivalent, otherwise RGFW_FALSE.
*/
RGFWDEF RGFW_bool RGFW_monitorModeCompare(RGFW_monitorMode mon, RGFW_monitorMode mon2, RGFW_modeRequest request);

/**!
 * @brief Scales a monitor’s mode to match a window’s size.
 * @param mon The monitor to be scaled.
 * @param win The window whose size should be used as a reference.
 * @return RGFW_TRUE if the scaling was successful, otherwise RGFW_FALSE.
*/
RGFWDEF RGFW_bool RGFW_monitor_scaleToWindow(RGFW_monitor mon, struct RGFW_window* win);

#endif

/**!
* @brief sleep until RGFW gets an event or the timer ends (defined by OS)
* @param waitMS how long to wait for the next event (in miliseconds)
*/
RGFWDEF void RGFW_waitForEvent(i32 waitMS);

/**!
* @brief Set if events should be queued or not (enabled by default if the event queue is checked)
* @param queue boolean value if RGFW should queue events or not
*/
RGFWDEF void RGFW_setQueueEvents(RGFW_bool queue);

/**!
* @brief check all the events until there are none left and updates window structure attributes
*/
RGFWDEF void RGFW_pollEvents(void);

/**!
* @brief check all the events until there are none left and updates window structure attributes
* queues events if the queue is checked and/or requested
*/
RGFWDEF void RGFW_stopCheckEvents(void);

/** * @defgroup Input
* @{ */

/**!
 * @brief returns true if the key is pressed during the current frame
 * @param key the key code of the key you want to check
 * @return The boolean value if the key is pressed or not
*/
RGFWDEF RGFW_bool RGFW_isKeyPressed(RGFW_key key);

/**!
 * @brief returns true if the key was released during the current frame
 * @param key the key code of the key you want to check
 * @return The boolean value if the key is released or not
*/
RGFWDEF RGFW_bool RGFW_isKeyReleased(RGFW_key key);

/**!
 * @brief returns true if the key is down
 * @param key the key code of the key you want to check
 * @return The boolean value if the key is down or not
*/
RGFWDEF RGFW_bool RGFW_isKeyDown(RGFW_key key);

/**!
 * @brief returns true if the mouse button is pressed during the current frame
 * @param button the mouse button code of the button you want to check
 * @return The boolean value if the button is pressed or not
*/
RGFWDEF RGFW_bool RGFW_isMousePressed(RGFW_mouseButton button);

/**!
 * @brief returns true if the mouse button is released during the current frame
 * @param button the mouse button code of the button you want to check
 * @return The boolean value if the button is released or not
*/
RGFWDEF RGFW_bool RGFW_isMouseReleased(RGFW_mouseButton button);

/**!
 * @brief returns true if the mouse button is down
 * @param button the mouse button code of the button you want to check
 * @return The boolean value if the button is down or not
*/
RGFWDEF RGFW_bool RGFW_isMouseDown(RGFW_mouseButton button);

/**!
 * @brief outputs the current x, y position of the mouse
 * @param X [OUTPUT] a pointer for the output X value
 * @param Y [OUTPUT] a pointer for the output Y value
*/
RGFWDEF void RGFW_getMouseScroll(float* x, float* y);

/**!
 * @brief outputs the current x, y movement vector of the mouse
 * @param X [OUTPUT] a pointer for the output X vector value
 * @param Y [OUTPUT] a pointer for the output Y vector value
*/
RGFWDEF void RGFW_getMouseVector(float* x, float* y);
/** @} */

/**!
 * @brief creates a new window
 * @param name the requested title of the window
 * @param x the requested x position of the window
 * @param y the requested y position of the window
 * @param w the requested width of the window
 * @param h the requested height of the window
 * @param flags extra arguments ((u32)0 means no flags used)
 * @return A pointer to the newly created window structure
 *
 * NOTE: (windows) if the executable has an icon resource named RGFW_ICON, it will be set as the initial icon for the window
*/
RGFWDEF RGFW_window* RGFW_createWindow(const char* name, i32 x, i32 y, i32 w, i32 h,  RGFW_windowFlags flags);

/**!
 * @brief creates a new window using a pre-allocated window structure
 * @param name the requested title of the window
 * @param x the requested x position of the window
 * @param y the requested y position of the window
 * @param w the requested width of the window
 * @param h the requested height of the window
 * @param flags extra arguments ((u32)0 means no flags used)
 * @param win a pointer the pre-allocated window structure
 * @return A pointer to the newly created window structure
*/
RGFWDEF RGFW_window* RGFW_createWindowPtr(const char* name, i32 x, i32 y, i32 w, i32 h, RGFW_windowFlags flags, RGFW_window* win);

/**!
 * @brief creates a new surface structure
 * @param win the source window of the surface
 * @param data a pointer to the raw data of the structure (you allocate this)
 * @param w the width the data
 * @param h the height of the data
 * @return A pointer to the newly created surface structure
 *
 * NOTE: when you create a surface using RGFW_createSurface / ptr, on X11 it uses the root window's visual
 * this means it may fail to render on any other window if the visual does not match
 * RGFW_window_createSurface and RGFW_window_createSurfacePtr exist only for X11 to address this issues
 * Of course, you can also manually set the root window with RGFW_setRootWindow
 */
RGFWDEF RGFW_surface* RGFW_window_createSurface(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format);

/**!
 * @brief creates a new surface structure using a pre-allocated surface structure
 * @param win the source window of the surface
 * @param data a pointer to the raw data of the structure (you allocate this)
 * @param w the width the data
 * @param h the height of the data
 * @param a pointer to the pre-allocated surface structure
 * @return a bool if the creation was successful or not
*/
RGFWDEF RGFW_bool RGFW_window_createSurfacePtr(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format, RGFW_surface* surface);

/**!
 * @brief blits a surface stucture to the window
 * @param win a pointer the window to blit to
 * @param surface a pointer to the surface
*/
RGFWDEF void RGFW_window_blitSurface(RGFW_window* win, RGFW_surface* surface);

/**!
 * @brief gets the position of the window | with RGFW_window.x and window.y
 * @param x [OUTPUT] the x position of the window
 * @param y [OUTPUT] the y position of the window
 * @return a bool if the function was successful
*/
RGFWDEF RGFW_bool RGFW_window_getPosition(RGFW_window* win, i32* x, i32* y); /*!<  */

/**!
 * @brief gets the size of the window | with RGFW_window.w and window.h
 * @param win a pointer to the window
 * @param w [OUTPUT] the width of the window
 * @param h [OUTPUT] the height of the window
 * @return a bool if the function was successful
*/
RGFWDEF RGFW_bool RGFW_window_getSize(RGFW_window* win, i32* w, i32* h);

/**!
 * @brief gets the flags of the window | returns RGFW_window._flags
 * @param win a pointer to the window
 * @return the window flags
*/
RGFWDEF u32 RGFW_window_getFlags(RGFW_window* win);

/**!
 * @brief returns the exit key assigned to the window
 * @param win a pointer to the target window
 * @return The key code assigned as the exit key
*/
RGFWDEF RGFW_key RGFW_window_getExitKey(RGFW_window* win);

/**!
 * @brief sets the exit key for the window
 * @param win a pointer to the target window
 * @param key the key code to assign as the exit key
*/
RGFWDEF void RGFW_window_setExitKey(RGFW_window* win, RGFW_key key);

/**!
 * @brief sets the types of events you want the window to receive
 * @param win a pointer to the target window
 * @param events the event flags to enable (use RGFW_allEventFlags for all)
*/
RGFWDEF void RGFW_window_setEnabledEvents(RGFW_window* win, RGFW_eventFlag events);

/**!
 * @brief gets the currently enabled events for the window
 * @param win a pointer to the target window
 * @return The enabled event flags for the window
*/
RGFWDEF RGFW_eventFlag RGFW_window_getEnabledEvents(RGFW_window* win);

/**!
 * @brief enables all events and disables selected ones
 * @param win a pointer to the target window
 * @param events the event flags to disable
*/
RGFWDEF void RGFW_window_setDisabledEvents(RGFW_window* win, RGFW_eventFlag events);

/**!
 * @brief directly enables or disables a specific event or group of events
 * @param win a pointer to the target window
 * @param event the event flag or group of flags to modify
 * @param state RGFW_TRUE to enable, RGFW_FALSE to disable
*/
RGFWDEF void RGFW_window_setEventState(RGFW_window* win, RGFW_eventFlag event, RGFW_bool state);

/**!
 * @brief gets the user pointer associated with the window
 * @param win a pointer to the target window
 * @return The user-defined pointer stored in the window
*/
RGFWDEF void* RGFW_window_getUserPtr(RGFW_window* win);

/**!
 * @brief sets a user pointer for the window
 * @param win a pointer to the target window
 * @param ptr a pointer to associate with the window
*/
RGFWDEF void RGFW_window_setUserPtr(RGFW_window* win, void* ptr);

/**!
 * @brief retrieves the platform-specific window source pointer
 * @param win a pointer to the target window
 * @return A pointer to the internal RGFW_window_src structure
*/
RGFWDEF RGFW_window_src* RGFW_window_getSrc(RGFW_window* win);

/**!
 * @brief sets the macOS layer object associated with the window
 * @param win a pointer to the target window
 * @param layer a pointer to the macOS layer object
 * @note Only available on macOS platforms
*/
RGFWDEF void RGFW_window_setLayer_OSX(RGFW_window* win, void* layer);

/**!
 * @brief retrieves the macOS view object associated with the window
 * @param win a pointer to the target window
 * @return A pointer to the macOS view object, or NULL if not on macOS
*/
RGFWDEF void* RGFW_window_getView_OSX(RGFW_window* win);

/**!
 * @brief retrieves the macOS window object
 * @param win a pointer to the target window
 * @return A pointer to the macOS window object, or NULL if not on macOS
*/
RGFWDEF void* RGFW_window_getWindow_OSX(RGFW_window* win);

/**!
 * @brief retrieves the HWND handle for the window
 * @param win a pointer to the target window
 * @return A pointer to the Windows HWND handle, or NULL if not on Windows
*/
RGFWDEF void* RGFW_window_getHWND(RGFW_window* win);

/**!
 * @brief retrieves the HDC handle for the window
 * @param win a pointer to the target window
 * @return A pointer to the Windows HDC handle, or NULL if not on Windows
*/
RGFWDEF void* RGFW_window_getHDC(RGFW_window* win);

/**!
 * @brief retrieves the X11 Window handle for the window
 * @param win a pointer to the target window
 * @return The X11 Window handle, or 0 if not on X11
*/
RGFWDEF u64 RGFW_window_getWindow_X11(RGFW_window* win);

/**!
 * @brief retrieves the Wayland surface handle for the window
 * @param win a pointer to the target window
 * @return A pointer to the Wayland wl_surface, or NULL if not on Wayland
*/
RGFWDEF struct wl_surface* RGFW_window_getWindow_Wayland(RGFW_window* win);

/** * @defgroup Window_management
* @{ */

/*! set the window flags (will undo flags if they don't match the old ones) */
RGFWDEF void RGFW_window_setFlags(RGFW_window* win, RGFW_windowFlags);

/**!
 * @brief polls and pops the next event from the window's event queue
 * @param win a pointer to the target window
 * @param event [OUTPUT] a pointer to store the retrieved event
 * @return RGFW_TRUE if an event was found, RGFW_FALSE otherwise
 *
 * NOTE: Using this function without a loop may cause event lag.
 * For multi-threaded systems, use RGFW_pollEvents combined with RGFW_window_checkQueuedEvent.
 *
 * Example:
 * RGFW_event event;
 * while (RGFW_window_checkEvent(win, &event)) {
 *     // handle event
 * }
*/
RGFWDEF RGFW_bool RGFW_window_checkEvent(RGFW_window* win, RGFW_event* event);

/**!
 * @brief pops the first queued event for the window
 * @param win a pointer to the target window
 * @param event [OUTPUT] a pointer to store the retrieved event
 * @return RGFW_TRUE if an event was found, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_checkQueuedEvent(RGFW_window* win, RGFW_event* event);

/**!
 * @brief checks if a key was pressed while the window is in focus
 * @param win a pointer to the target window
 * @param key the key code to check
 * @return RGFW_TRUE if the key was pressed, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_isKeyPressed(RGFW_window* win, RGFW_key key);

/**!
 * @brief checks if a key is currently being held down
 * @param win a pointer to the target window
 * @param key the key code to check
 * @return RGFW_TRUE if the key is held down, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_isKeyDown(RGFW_window* win, RGFW_key key);

/**!
 * @brief checks if a key was released
 * @param win a pointer to the target window
 * @param key the key code to check
 * @return RGFW_TRUE if the key was released, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_isKeyReleased(RGFW_window* win, RGFW_key key);

/**!
 * @brief checks if a mouse button was pressed
 * @param win a pointer to the target window
 * @param button the mouse button code to check
 * @return RGFW_TRUE if the mouse button was pressed, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_isMousePressed(RGFW_window* win, RGFW_mouseButton button);

/**!
 * @brief checks if a mouse button is currently held down
 * @param win a pointer to the target window
 * @param button the mouse button code to check
 * @return RGFW_TRUE if the mouse button is down, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_isMouseDown(RGFW_window* win, RGFW_mouseButton button);

/**!
 * @brief checks if a mouse button was released
 * @param win a pointer to the target window
 * @param button the mouse button code to check
 * @return RGFW_TRUE if the mouse button was released, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_isMouseReleased(RGFW_window* win, RGFW_mouseButton button);

/**!
 * @brief checks if the mouse left the window (true only for the first frame)
 * @param win a pointer to the target window
 * @return RGFW_TRUE if the mouse left, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_didMouseLeave(RGFW_window* win);

/**!
 * @brief checks if the mouse entered the window (true only for the first frame)
 * @param win a pointer to the target window
 * @return RGFW_TRUE if the mouse entered, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_didMouseEnter(RGFW_window* win);

/**!
 * @brief checks if the mouse is currently inside the window bounds
 * @param win a pointer to the target window
 * @return RGFW_TRUE if the mouse is inside, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_isMouseInside(RGFW_window* win);

/**!
 * @brief checks if there is data being dragged into or within the window
 * @param win a pointer to the target window
 * @return RGFW_TRUE if data is being dragged, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_isDataDragging(RGFW_window* win);

/**!
 * @brief gets the position of a data drag
 * @param win a pointer to the target window
 * @param x [OUTPUT] pointer to store the x position
 * @param y [OUTPUT] pointer to store the y position
 * @return RGFW_TRUE if there is an active drag, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_getDataDrag(RGFW_window* win, i32* x, i32* y);

/**!
 * @brief checks if a data drop occurred in the window (first frame only)
 * @param win a pointer to the target window
 * @return RGFW_TRUE if data was dropped, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_didDataDrop(RGFW_window* win);

/**!
 * @brief retrieves files from a data drop (drag and drop)
 * @param win a pointer to the target window
 * @param files [OUTPUT] a pointer to the array of file paths
 * @param count [OUTPUT] the number of dropped files
 * @return RGFW_TRUE if a data drop occurred, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_getDataDrop(RGFW_window* win, const char*** files, size_t* count);

/**!
 * @brief closes the window and frees its associated structure
 * @param win a pointer to the target window
*/
RGFWDEF void RGFW_window_close(RGFW_window* win);

/**!
 * @brief closes the window without freeing its structure
 * @param win a pointer to the target window
*/
RGFWDEF void RGFW_window_closePtr(RGFW_window* win);

/**!
 * @brief moves the window to a new position on the screen
 * @param win a pointer to the target window
 * @param x the new x position
 * @param y the new y position
*/
RGFWDEF void RGFW_window_move(RGFW_window* win, i32 x, i32 y);

#ifndef RGFW_NO_MONITOR
/**!
 * @brief moves the window to a specific monitor
 * @param win a pointer to the target window
 * @param m the target monitor
*/
RGFWDEF void RGFW_window_moveToMonitor(RGFW_window* win, RGFW_monitor m);
#endif

/**!
 * @brief resizes the window to the given dimensions
 * @param win a pointer to the target window
 * @param w the new width
 * @param h the new height
*/
RGFWDEF void RGFW_window_resize(RGFW_window* win, i32 w, i32 h);

/**!
 * @brief sets the aspect ratio of the window
 * @param win a pointer to the target window
 * @param w the width ratio
 * @param h the height ratio
*/
RGFWDEF void RGFW_window_setAspectRatio(RGFW_window* win, i32 w, i32 h);

/**!
 * @brief sets the minimum size of the window
 * @param win a pointer to the target window
 * @param w the minimum width
 * @param h the minimum height
*/
RGFWDEF void RGFW_window_setMinSize(RGFW_window* win, i32 w, i32 h);

/**!
 * @brief sets the maximum size of the window
 * @param win a pointer to the target window
 * @param w the maximum width
 * @param h the maximum height
*/
RGFWDEF void RGFW_window_setMaxSize(RGFW_window* win, i32 w, i32 h);

/**!
 * @brief sets focus to the window
 * @param win a pointer to the target window
*/
RGFWDEF void RGFW_window_focus(RGFW_window* win);

/**!
 * @brief checks if the window is currently in focus
 * @param win a pointer to the target window
 * @return RGFW_TRUE if the window is in focus, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_isInFocus(RGFW_window* win);

/**!
 * @brief raises the window to the top of the stack
 * @param win a pointer to the target window
*/
RGFWDEF void RGFW_window_raise(RGFW_window* win);

/**!
 * @brief maximizes the window
 * @param win a pointer to the target window
*/
RGFWDEF void RGFW_window_maximize(RGFW_window* win);

/**!
 * @brief toggles fullscreen mode for the window
 * @param win a pointer to the target window
 * @param fullscreen RGFW_TRUE to enable fullscreen, RGFW_FALSE to disable
*/
RGFWDEF void RGFW_window_setFullscreen(RGFW_window* win, RGFW_bool fullscreen);

/**!
 * @brief centers the window on the screen
 * @param win a pointer to the target window
*/
RGFWDEF void RGFW_window_center(RGFW_window* win);

/**!
 * @brief minimizes the window
 * @param win a pointer to the target window
*/
RGFWDEF void RGFW_window_minimize(RGFW_window* win);

/**!
 * @brief restores the window from minimized state
 * @param win a pointer to the target window
*/
RGFWDEF void RGFW_window_restore(RGFW_window* win);

/**!
 * @brief makes the window a floating window
 * @param win a pointer to the target window
 * @param floating RGFW_TRUE to float, RGFW_FALSE to disable
*/
RGFWDEF void RGFW_window_setFloating(RGFW_window* win, RGFW_bool floating);

/**!
 * @brief sets the opacity level of the window
 * @param win a pointer to the target window
 * @param opacity the opacity level (0–255)
*/
RGFWDEF void RGFW_window_setOpacity(RGFW_window* win, u8 opacity);

/**!
 * @brief toggles window borders
 * @param win a pointer to the target window
 * @param border RGFW_TRUE for bordered, RGFW_FALSE for borderless
*/
RGFWDEF void RGFW_window_setBorder(RGFW_window* win, RGFW_bool border);

/**!
 * @brief checks if the window is borderless
 * @param win a pointer to the target window
 * @return RGFW_TRUE if borderless, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_borderless(RGFW_window* win);

/**!
 * @brief toggles drag-and-drop (DND) support for the window
 * @param win a pointer to the target window
 * @param allow RGFW_TRUE to allow DND, RGFW_FALSE to disable
 * @note RGFW_windowAllowDND must still be passed when creating the window
*/
RGFWDEF void RGFW_window_setDND(RGFW_window* win, RGFW_bool allow);

/**!
 * @brief checks if drag-and-drop (DND) is allowed
 * @param win a pointer to the target window
 * @return RGFW_TRUE if DND is enabled, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_allowsDND(RGFW_window* win);

#ifndef RGFW_NO_PASSTHROUGH
/**!
 * @brief toggles mouse passthrough for the window
 * @param win a pointer to the target window
 * @param passthrough RGFW_TRUE to enable passthrough, RGFW_FALSE to disable
*/
RGFWDEF void RGFW_window_setMousePassthrough(RGFW_window* win, RGFW_bool passthrough);
#endif

/**!
 * @brief renames the window
 * @param win a pointer to the target window
 * @param name the new title string for the window
*/
RGFWDEF void RGFW_window_setName(RGFW_window* win, const char* name);

/**!
 * @brief sets the icon for the window and taskbar
 * @param win a pointer to the target window
 * @param data the image data
 * @param w the width of the icon
 * @param h the height of the icon
 * @param format the image format
 * @return RGFW_TRUE if successful, RGFW_FALSE otherwise
 *
 * NOTE: The image may be resized by default.
*/
RGFWDEF RGFW_bool RGFW_window_setIcon(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format);

/**!
 * @brief sets the icon for the window and/or taskbar
 * @param win a pointer to the target window
 * @param data the image data
 * @param w the width of the icon
 * @param h the height of the icon
 * @param format the image format
 * @param type the target icon type (taskbar, window, or both)
 * @return RGFW_TRUE if successful, RGFW_FALSE otherwise
*/
RGFWDEF RGFW_bool RGFW_window_setIconEx(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format, RGFW_icon type);

/**!
 * @brief sets the mouse icon for the window using a loaded bitmap
 * @param win a pointer to the target window
 * @param mouse a pointer to the RGFW_mouse struct containing the icon
*/
RGFWDEF void RGFW_window_setMouse(RGFW_window* win, RGFW_mouse* mouse);

/**!
 * @brief Sets the mouse to a standard system cursor.
 * @param win The target window.
 * @param mouse The standard cursor type (see RGFW_MOUSE enum).
 * @return True if the standard cursor was successfully applied.
*/
RGFWDEF RGFW_bool RGFW_window_setMouseStandard(RGFW_window* win, RGFW_mouseIcons mouse);

/**!
 * @brief Sets the mouse to the default cursor icon.
 * @param win The target window.
 * @return True if the default cursor was successfully set.
*/
RGFWDEF RGFW_bool RGFW_window_setMouseDefault(RGFW_window* win);

/**!
 * @brief Locks the cursor to the center of the window.
 * @param win The target window.
 *
 * While the cursor is held, X and Y report raw mouse movement data.
 * Useful for 3D camera or first-person movement systems.
*/
RGFWDEF void RGFW_window_holdMouse(RGFW_window* win);

/**!
 * @brief Returns true if the mouse is currently held by RGFW.
 * @param win The target window.
 * @return True if the mouse is being held.
*/
RGFWDEF RGFW_bool RGFW_window_isHoldingMouse(RGFW_window* win);

/**!
 * @brief Releases the mouse so it can move freely again.
 * @param win The target window.
*/
RGFWDEF void RGFW_window_unholdMouse(RGFW_window* win);

/**!
 * @brief Hides the window from view.
 * @param win The target window.
*/
RGFWDEF void RGFW_window_hide(RGFW_window* win);

/**!
 * @brief Shows the window if it was hidden.
 * @param win The target window.
*/
RGFWDEF void RGFW_window_show(RGFW_window* win);

/**!
 * @brief Sets whether the window should close.
 * @param win The target window.
 * @param shouldClose True to signal the window should close, false to keep it open.
 *
 * This can override or trigger the `RGFW_window_shouldClose` state by modifying window flags.
*/
RGFWDEF void RGFW_window_setShouldClose(RGFW_window* win, RGFW_bool shouldClose);

/**!
 * @brief Retrieves the current global mouse position.
 * @param x [OUTPUT] Pointer to store the X position of the mouse on the screen.
 * @param y [OUTPUT] Pointer to store the Y position of the mouse on the screen.
 * @return True if the position was successfully retrieved.
*/
RGFWDEF RGFW_bool RGFW_getGlobalMouse(i32* x, i32* y);

/**!
 * @brief Retrieves the mouse position relative to the window.
 * @param win The target window.
 * @param x [OUTPUT] Pointer to store the X position within the window.
 * @param y [OUTPUT] Pointer to store the Y position within the window.
 * @return True if the position was successfully retrieved.
*/
RGFWDEF RGFW_bool RGFW_window_getMouse(RGFW_window* win, i32* x, i32* y);

/**!
 * @brief Shows or hides the mouse cursor for the window.
 * @param win The target window.
 * @param show True to show the mouse, false to hide it.
*/
RGFWDEF void RGFW_window_showMouse(RGFW_window* win, RGFW_bool show);

/**!
 * @brief Checks if the mouse is currently hidden in the window.
 * @param win The target window.
 * @return True if the mouse is hidden.
*/
RGFWDEF RGFW_bool RGFW_window_isMouseHidden(RGFW_window* win);

/**!
 * @brief Moves the mouse to the specified position within the window.
 * @param win The target window.
 * @param x The new X position.
 * @param y The new Y position.
*/
RGFWDEF void RGFW_window_moveMouse(RGFW_window* win, i32 x, i32 y);

/**!
 * @brief Checks if the window should close.
 * @param win The target window.
 * @return True if the window should close (for example, if ESC was pressed or a close event occurred).
*/
RGFWDEF RGFW_bool RGFW_window_shouldClose(RGFW_window* win);

/**!
 * @brief Checks if the window is currently fullscreen.
 * @param win The target window.
 * @return True if the window is fullscreen.
*/
RGFWDEF RGFW_bool RGFW_window_isFullscreen(RGFW_window* win);

/**!
 * @brief Checks if the window is currently hidden.
 * @param win The target window.
 * @return True if the window is hidden.
*/
RGFWDEF RGFW_bool RGFW_window_isHidden(RGFW_window* win);

/**!
 * @brief Checks if the window is minimized.
 * @param win The target window.
 * @return True if the window is minimized.
*/
RGFWDEF RGFW_bool RGFW_window_isMinimized(RGFW_window* win);

/**!
 * @brief Checks if the window is maximized.
 * @param win The target window.
 * @return True if the window is maximized.
*/
RGFWDEF RGFW_bool RGFW_window_isMaximized(RGFW_window* win);

/**!
 * @brief Checks if the window is floating.
 * @param win The target window.
 * @return True if the window is floating.
*/
RGFWDEF RGFW_bool RGFW_window_isFloating(RGFW_window* win);
/** @} */

/** * @defgroup Monitor
* @{ */

#ifndef RGFW_NO_MONITOR
/**!
 * @brief Scales the window to match its monitor’s resolution.
 * @param win The target window.
 *
 * This function is automatically called when the flag `RGFW_scaleToMonitor`
 * is used during window creation.
*/
RGFWDEF void RGFW_window_scaleToMonitor(RGFW_window* win);

/**!
 * @brief Retrieves the monitor structure associated with the window.
 * @param win The target window.
 * @return The monitor structure of the window.
*/
RGFWDEF RGFW_monitor RGFW_window_getMonitor(RGFW_window* win);
#endif

/** @} */

/** * @defgroup Clipboard
* @{ */

/**!
 * @brief Reads clipboard data.
 * @param size [OUTPUT] A pointer that will be filled with the size of the clipboard data.
 * @return A pointer to the clipboard data as a string.
*/
RGFWDEF const char* RGFW_readClipboard(size_t* size);

/**!
 * @brief Reads clipboard data into a provided buffer, or returns the required length if str is NULL.
 * @param str [OUTPUT] A pointer to the buffer that will receive the clipboard data (or NULL to get required size).
 * @param strCapacity The capacity of the provided buffer.
 * @return The number of bytes read or required length of clipboard data.
*/
RGFWDEF RGFW_ssize_t RGFW_readClipboardPtr(char* str, size_t strCapacity);

/**!
 * @brief Writes text to the clipboard.
 * @param text The text to be written to the clipboard.
 * @param textLen The length of the text being written.
*/
RGFWDEF void RGFW_writeClipboard(const char* text, u32 textLen);
/** @} */



/** * @defgroup error handling
* @{ */
/**!
 * @brief Sets the callback function to handle debug messages from RGFW.
 * @param func The function pointer to be used as the debug callback.
 * @return The previously set debug callback function.
*/
RGFWDEF RGFW_debugfunc RGFW_setDebugCallback(RGFW_debugfunc func);

/**!
 * @brief Sends a debug message manually through the currently set debug callback.
 * @param type The type of debug message being sent.
 * @param err The associated error code.
 * @param msg The debug message text.
*/
RGFWDEF void RGFW_sendDebugInfo(RGFW_debugType type, RGFW_errorCode err, const char* msg);
/** @} */

/**


	event callbacks.
	These are completely optional, so you can use the normal
	RGFW_checkEvent() method if you prefer that

* @defgroup Callbacks
* @{
*/

/**!
 * @brief Sets the callback function for window move events.
 * @param func The function to be called when the window is moved.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_windowMovedfunc RGFW_setWindowMovedCallback(RGFW_windowMovedfunc func);

/**!
 * @brief Sets the callback function for window resize events.
 * @param func The function to be called when the window is resized.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_windowResizedfunc RGFW_setWindowResizedCallback(RGFW_windowResizedfunc func);

/**!
 * @brief Sets the callback function for window quit events.
 * @param func The function to be called when the window receives a quit signal.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_windowQuitfunc RGFW_setWindowQuitCallback(RGFW_windowQuitfunc func);

/**!
 * @brief Sets the callback function for mouse move events.
 * @param func The function to be called when the mouse moves within the window.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_mousePosfunc RGFW_setMousePosCallback(RGFW_mousePosfunc func);

/**!
 * @brief Sets the callback function for window refresh events.
 * @param func The function to be called when the window needs to be refreshed.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_windowRefreshfunc RGFW_setWindowRefreshCallback(RGFW_windowRefreshfunc func);

/**!
 * @brief Sets the callback function for focus change events.
 * @param func The function to be called when the window gains or loses focus.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_focusfunc RGFW_setFocusCallback(RGFW_focusfunc func);

/**!
 * @brief Sets the callback function for mouse notification events.
 * @param func The function to be called when a mouse notification event occurs.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_mouseNotifyfunc RGFW_setMouseNotifyCallback(RGFW_mouseNotifyfunc func);

/**!
 * @brief Sets the callback function for data drop events.
 * @param func The function to be called when data is dropped into the window.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_dataDropfunc RGFW_setDataDropCallback(RGFW_dataDropfunc func);

/**!
 * @brief Sets the callback function for the start of a data drag event.
 * @param func The function to be called when data dragging begins.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_dataDragfunc RGFW_setDataDragCallback(RGFW_dataDragfunc func);

/**!
 * @brief Sets the callback function for key press and release events.
 * @param func The function to be called when a key is pressed or released.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_keyfunc RGFW_setKeyCallback(RGFW_keyfunc func);

/**!
 * @brief Sets the callback function for mouse button press and release events.
 * @param func The function to be called when a mouse button is pressed or released.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_mouseButtonfunc RGFW_setMouseButtonCallback(RGFW_mouseButtonfunc func);

/**!
 * @brief Sets the callback function for mouse scroll events.
 * @param func The function to be called when the mouse wheel is scrolled.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_mouseScrollfunc RGFW_setMouseScrollCallback(RGFW_mouseScrollfunc func);

/**!
 * @brief Sets the callback function for window maximize events.
 * @param func The function to be called when the window is maximized.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_windowMaximizedfunc RGFW_setWindowMaximizedCallback(RGFW_windowMaximizedfunc func);

/**!
 * @brief Sets the callback function for window minimize events.
 * @param func The function to be called when the window is minimized.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_windowMinimizedfunc RGFW_setWindowMinimizedCallback(RGFW_windowMinimizedfunc func);

/**!
 * @brief Sets the callback function for window restore events.
 * @param func The function to be called when the window is restored from a minimized or maximized state.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_windowRestoredfunc RGFW_setWindowRestoredCallback(RGFW_windowRestoredfunc func);

/**!
 * @brief Sets the callback function for DPI (scale) update events.
 * @param func The function to be called when the window’s DPI or scale changes.
 * @return The previously set callback function, if any.
*/
RGFWDEF RGFW_scaleUpdatedfunc RGFW_setScaleUpdatedCallback(RGFW_scaleUpdatedfunc func);
/** @} */

/** * @defgroup graphics_API
* @{ */

/*! native rendering API functions */
#if defined(RGFW_OPENGL)
/* these are native opengl specific functions and will NOT work with EGL */

/*!< make the window the current OpenGL drawing context

	NOTE:
 	if you want to switch the graphics context's thread,
	you have to run RGFW_window_makeCurrentContext_OpenGL(NULL); on the old thread
	then RGFW_window_makeCurrentContext_OpenGL(valid_window) on the new thread
*/

/**!
 * @brief Sets the global OpenGL hints to the specified pointer.
 * @param hints A pointer to the RGFW_glHints structure containing the desired OpenGL settings.
*/
RGFWDEF void RGFW_setGlobalHints_OpenGL(RGFW_glHints* hints);

/**!
 * @brief Resets the global OpenGL hints to their default values.
*/
RGFWDEF void RGFW_resetGlobalHints_OpenGL(void);

/**!
 * @brief Gets the current global OpenGL hints pointer.
 * @return A pointer to the currently active RGFW_glHints structure.
*/
RGFWDEF RGFW_glHints* RGFW_getGlobalHints_OpenGL(void);

/**!
 * @brief Creates and allocates an OpenGL context for the specified window.
 * @param win A pointer to the target RGFW_window.
 * @param hints A pointer to an RGFW_glHints structure defining context creation parameters.
 * @return A pointer to the newly created RGFW_glContext.
*/
RGFWDEF RGFW_glContext* RGFW_window_createContext_OpenGL(RGFW_window* win, RGFW_glHints* hints);

/**!
 * @brief Creates an OpenGL context for the specified window using a preallocated context structure.
 * @param win A pointer to the target RGFW_window.
 * @param ctx A pointer to an already allocated RGFW_glContext structure.
 * @param hints A pointer to an RGFW_glHints structure defining context creation parameters.
 * @return RGFW_TRUE on success, RGFW_FALSE on failure.
*/
RGFWDEF RGFW_bool RGFW_window_createContextPtr_OpenGL(RGFW_window* win, RGFW_glContext* ctx, RGFW_glHints* hints);

/**!
 * @brief Retrieves the OpenGL context associated with a window.
 * @param win A pointer to the RGFW_window.
 * @return A pointer to the associated RGFW_glContext, or NULL if none exists or if the context is EGL-based.
*/
RGFWDEF RGFW_glContext* RGFW_window_getContext_OpenGL(RGFW_window* win);

/**!
 * @brief Deletes and frees the OpenGL context.
 * @param win A pointer to the RGFW_window.
 * @param ctx A pointer to the RGFW_glContext to delete.
 *
 * @note This is automatically called by RGFW_window_close if the window’s context is not NULL.
*/
RGFWDEF void RGFW_window_deleteContext_OpenGL(RGFW_window* win, RGFW_glContext* ctx);

/**!
 * @brief Deletes the OpenGL context without freeing its memory.
 * @param win A pointer to the RGFW_window.
 * @param ctx A pointer to the RGFW_glContext to delete.
 *
 * @note This is automatically called by RGFW_window_close if the window’s context is not NULL.
*/
RGFWDEF void RGFW_window_deleteContextPtr_OpenGL(RGFW_window* win, RGFW_glContext* ctx);

/**!
 * @brief Retrieves the native source context from an RGFW_glContext.
 * @param ctx A pointer to the RGFW_glContext.
 * @return A pointer to the native OpenGL context handle.
*/
RGFWDEF void* RGFW_glContext_getSourceContext(RGFW_glContext* ctx);

/**!
 * @brief Makes the specified window the current OpenGL rendering target.
 * @param win A pointer to the RGFW_window to make current.
 *
 * @note This is typically called internally by RGFW_window_makeCurrent.
*/
RGFWDEF void RGFW_window_makeCurrentWindow_OpenGL(RGFW_window* win);

/**!
 * @brief Makes the OpenGL context of the specified window current.
 * @param win A pointer to the RGFW_window whose context should be made current.
 *
 * @note To move a context between threads, call RGFW_window_makeCurrentContext_OpenGL(NULL)
 *       on the old thread before making it current on the new one.
*/
RGFWDEF void RGFW_window_makeCurrentContext_OpenGL(RGFW_window* win);

/**!
 * @brief Swaps the OpenGL buffers for the specified window.
 * @param win A pointer to the RGFW_window whose buffers should be swapped.
 *
 * @note Typically called by RGFW_window_swapInterval.
*/
RGFWDEF void RGFW_window_swapBuffers_OpenGL(RGFW_window* win);

/**!
 * @brief Retrieves the current OpenGL context.
 * @return A pointer to the currently active OpenGL context (GLX, WGL, Cocoa, or WebGL backend).
*/
RGFWDEF void* RGFW_getCurrentContext_OpenGL(void);

/**!
 * @brief Retrieves the current OpenGL window.
 * @return A pointer to the RGFW_window currently bound as the OpenGL context target.
*/
RGFWDEF RGFW_window* RGFW_getCurrentWindow_OpenGL(void);

/**!
 * @brief Sets the OpenGL swap interval (vsync).
 * @param win A pointer to the RGFW_window.
 * @param swapInterval The desired swap interval value (0 to disable vsync, 1 to enable).
*/
RGFWDEF void RGFW_window_swapInterval_OpenGL(RGFW_window* win, i32 swapInterval);

/**!
 * @brief Retrieves the address of a native OpenGL procedure.
 * @param procname The name of the OpenGL function to look up.
 * @return A pointer to the function, or NULL if not found.
*/
RGFWDEF RGFW_proc RGFW_getProcAddress_OpenGL(const char* procname);

/**!
 * @brief Checks whether a specific OpenGL or OpenGL ES API extension is supported.
 * @param extension The name of the extension to check.
 * @param len The length of the extension string.
 * @return RGFW_TRUE if supported, RGFW_FALSE otherwise.
*/
RGFWDEF RGFW_bool RGFW_extensionSupported_OpenGL(const char* extension, size_t len);

/**!
 * @brief Checks whether a specific platform-dependent OpenGL extension is supported.
 * @param extension The name of the extension to check.
 * @param len The length of the extension string.
 * @return RGFW_TRUE if supported, RGFW_FALSE otherwise.
*/
RGFWDEF RGFW_bool RGFW_extensionSupportedPlatform_OpenGL(const char* extension, size_t len);

/* these are EGL specific functions, they may fallback to OpenGL */
#ifdef RGFW_EGL
/**!
 * @brief Creates and allocates an OpenGL/EGL context for the specified window.
 * @param win A pointer to the target RGFW_window.
 * @param hints A pointer to an RGFW_glHints structure defining context creation parameters.
 * @return A pointer to the newly created RGFW_eglContext.
*/
RGFWDEF RGFW_eglContext* RGFW_window_createContext_EGL(RGFW_window* win, RGFW_glHints* hints);

/**!
 * @brief Creates an OpenGL/EGL context for the specified window using a preallocated context structure.
 * @param win A pointer to the target RGFW_window.
 * @param ctx A pointer to an already allocated RGFW_eglContext structure.
 * @param hints A pointer to an RGFW_glHints structure defining context creation parameters.
 * @return RGFW_TRUE on success, RGFW_FALSE on failure.
*/
RGFWDEF RGFW_bool RGFW_window_createContextPtr_EGL(RGFW_window* win, RGFW_eglContext* ctx, RGFW_glHints* hints);

/**!
 * @brief Frees and deletes an OpenGL/EGL context.
 * @param win A pointer to the RGFW_window.
 * @param ctx A pointer to the RGFW_eglContext to delete.
 *
 * @note Automatically called by RGFW_window_close if RGFW owns the context.
*/
RGFWDEF void RGFW_window_deleteContext_EGL(RGFW_window* win, RGFW_eglContext* ctx);

/**!
 * @brief Deletes an OpenGL/EGL context without freeing its memory.
 * @param win A pointer to the RGFW_window.
 * @param ctx A pointer to the RGFW_eglContext to delete.
 *
 * @note Automatically called by RGFW_window_close if RGFW owns the context.
*/
RGFWDEF void RGFW_window_deleteContextPtr_EGL(RGFW_window* win, RGFW_eglContext* ctx);

/**!
 * @brief Retrieves the OpenGL/EGL context associated with a window.
 * @param win A pointer to the RGFW_window.
 * @return A pointer to the associated RGFW_eglContext, or NULL if none exists or if the context is a native OpenGL context.
*/
RGFWDEF RGFW_eglContext* RGFW_window_getContext_EGL(RGFW_window* win);

/**!
 * @brief Retrieves the EGL display handle.
 * @return A pointer to the native EGLDisplay.
*/
RGFWDEF void* RGFW_getDisplay_EGL(void);

/**!
 * @brief Retrieves the native source context from an RGFW_eglContext.
 * @param ctx A pointer to the RGFW_eglContext.
 * @return A pointer to the native EGLContext handle.
*/
RGFWDEF void* RGFW_eglContext_getSourceContext(RGFW_eglContext* ctx);

/**!
 * @brief Retrieves the EGL surface handle from an RGFW_eglContext.
 * @param ctx A pointer to the RGFW_eglContext.
 * @return A pointer to the EGLSurface associated with the context.
*/
RGFWDEF void* RGFW_eglContext_getSurface(RGFW_eglContext* ctx);

/**!
 * @brief Retrieves the Wayland EGL window handle from an RGFW_eglContext.
 * @param ctx A pointer to the RGFW_eglContext.
 * @return A pointer to the wl_egl_window associated with the EGL context.
*/
RGFWDEF struct wl_egl_window* RGFW_eglContext_wlEGLWindow(RGFW_eglContext* ctx);

/**!
 * @brief Swaps the EGL buffers for the specified window.
 * @param win A pointer to the RGFW_window whose buffers should be swapped.
 *
 * @note Typically called by RGFW_window_swapInterval.
*/
RGFWDEF void RGFW_window_swapBuffers_EGL(RGFW_window* win);

/**!
 * @brief Makes the specified window the current EGL rendering target.
 * @param win A pointer to the RGFW_window to make current.
 *
 * @note This is typically called internally by RGFW_window_makeCurrent.
*/
RGFWDEF void RGFW_window_makeCurrentWindow_EGL(RGFW_window* win);

/**!
 * @brief Makes the EGL context of the specified window current.
 * @param win A pointer to the RGFW_window whose context should be made current.
 *
 * @note To move a context between threads, call RGFW_window_makeCurrentContext_EGL(NULL)
 *       on the old thread before making it current on the new one.
*/
RGFWDEF void RGFW_window_makeCurrentContext_EGL(RGFW_window* win);

/**!
 * @brief Retrieves the current EGL context.
 * @return A pointer to the currently active EGLContext.
*/
RGFWDEF void* RGFW_getCurrentContext_EGL(void);

/**!
 * @brief Retrieves the current EGL window.
 * @return A pointer to the RGFW_window currently bound as the EGL context target.
*/
RGFWDEF RGFW_window* RGFW_getCurrentWindow_EGL(void);

/**!
 * @brief Sets the EGL swap interval (vsync).
 * @param win A pointer to the RGFW_window.
 * @param swapInterval The desired swap interval value (0 to disable vsync, 1 to enable).
*/
RGFWDEF void RGFW_window_swapInterval_EGL(RGFW_window* win, i32 swapInterval);

/**!
 * @brief Retrieves the address of a native OpenGL or OpenGL ES procedure in an EGL context.
 * @param procname The name of the OpenGL function to look up.
 * @return A pointer to the function, or NULL if not found.
*/
RGFWDEF RGFW_proc RGFW_getProcAddress_EGL(const char* procname);

/**!
 * @brief Checks whether a specific OpenGL or OpenGL ES API extension is supported in the current EGL context.
 * @param extension The name of the extension to check.
 * @param len The length of the extension string.
 * @return RGFW_TRUE if supported, RGFW_FALSE otherwise.
*/
RGFWDEF RGFW_bool RGFW_extensionSupported_EGL(const char* extension, size_t len);

/**!
 * @brief Checks whether a specific platform-dependent EGL extension is supported in the current context.
 * @param extension The name of the extension to check.
 * @param len The length of the extension string.
 * @return RGFW_TRUE if supported, RGFW_FALSE otherwise.
*/
RGFWDEF RGFW_bool RGFW_extensionSupportedPlatform_EGL(const char* extension, size_t len);
#endif
#endif

#ifdef RGFW_VULKAN
#include <vulkan/vulkan.h>

/* if you don't want to use the above macros */

/**!
 * @brief Retrieves the Vulkan instance extensions required by RGFW.
 * @param count [OUTPUT] A pointer that will receive the number of required extensions (typically 2).
 * @return A pointer to a static array of required Vulkan instance extension names.
*/
RGFWDEF const char** RGFW_getRequiredInstanceExtensions_Vulkan(size_t* count);

/**!
 * @brief Creates a Vulkan surface for the specified window.
 * @param win A pointer to the RGFW_window for which to create the Vulkan surface.
 * @param instance The Vulkan instance used to create the surface.
 * @param surface [OUTPUT] A pointer to a VkSurfaceKHR handle that will receive the created surface.
 * @return A VkResult indicating success or failure.
*/
RGFWDEF VkResult RGFW_window_createSurface_Vulkan(RGFW_window* win, VkInstance instance, VkSurfaceKHR* surface);

/**!
 * @brief Checks whether the specified Vulkan physical device and queue family support presentation for RGFW.
 * @param instance The Vulkan instance.
 * @param physicalDevice The Vulkan physical device to check.
 * @param queueFamilyIndex The index of the queue family to query for presentation support.
 * @return RGFW_TRUE if presentation is supported, RGFW_FALSE otherwise.
*/
RGFWDEF RGFW_bool RGFW_getPresentationSupport_Vulkan(VkInstance instance, VkPhysicalDevice physicalDevice, u32 queueFamilyIndex);
#endif

#ifdef RGFW_DIRECTX
#ifndef RGFW_WINDOWS
	#undef RGFW_DIRECTX
#else
	#define OEMRESOURCE
	#include <dxgi.h>

	#ifndef __cplusplus
		#define __uuidof(T) IID_##T
	#endif
/**!
 * @brief Creates a DirectX swap chain for the specified RGFW window.
 * @param win A pointer to the RGFW_window for which to create the swap chain.
 * @param pFactory A pointer to the IDXGIFactory used to create the swap chain.
 * @param pDevice A pointer to the DirectX device (e.g., ID3D11Device or ID3D12Device).
 * @param swapchain [OUTPUT] A pointer to an IDXGISwapChain pointer that will receive the created swap chain.
 * @return An integer result code (0 on success, or a DirectX error code on failure).
*/
RGFWDEF int RGFW_window_createSwapChain_DirectX(RGFW_window* win, IDXGIFactory* pFactory, IUnknown* pDevice, IDXGISwapChain** swapchain);
#endif
#endif

#ifdef RGFW_WEBGPU
	#include <webgpu/webgpu.h>
	/**!
	 * @brief Creates a WebGPU surface for the specified RGFW window.
	 * @param window A pointer to the RGFW_window for which to create the surface.
	 * @param instance The WebGPU instance used to create the surface.
	 * @return The created WGPUSurface handle.
	*/
	RGFWDEF WGPUSurface RGFW_window_createSurface_WebGPU(RGFW_window* window, WGPUInstance instance);
#endif

/** @} */

/** * @defgroup Supporting
* @{ */

/**!
 * @brief Sets the root (main) RGFW window.
 * @param win A pointer to the RGFW_window to set as the root window.
*/
RGFWDEF void RGFW_setRootWindow(RGFW_window* win);

/**!
 * @brief Retrieves the current root RGFW window.
 * @return A pointer to the current root RGFW_window.
*/
RGFWDEF RGFW_window* RGFW_getRootWindow(void);

/**!
 * @brief Pushes an event into the standard RGFW event queue.
 * @param event A pointer to the RGFW_event to be added to the queue.
*/
RGFWDEF void RGFW_eventQueuePush(const RGFW_event* event);

/**!
 * @brief Clears all events from the RGFW event queue without processing them.
*/
RGFWDEF void RGFW_eventQueueFlush(void);

/**!
 * @brief Pops the next event from the RGFW event queue for the specified window.
 * @param win A pointer to the RGFW_window to retrieve an event for.
 * @return A pointer to the popped RGFW_event, or NULL if the queue is empty.
*/
RGFWDEF RGFW_event* RGFW_eventQueuePop(RGFW_window* win);

/**!
 * @brief Converts an API keycode to the RGFW unmapped (physical) key.
 * @param keycode The platform-specific keycode.
 * @return The corresponding RGFW keycode.
*/
RGFWDEF u32 RGFW_apiKeyToRGFW(u32 keycode);

/**!
 * @brief Converts an RGFW keycode to the unmapped (physical) API key.
 * @param keycode The RGFW keycode.
 * @return The corresponding platform-specific keycode.
*/
RGFWDEF u32 RGFW_rgfwToApiKey(u32 keycode);

/**!
 * @brief Converts an RGFW keycode to the mapped character representation.
 * @param keycode The RGFW keycode.
 * @return The corresponding key character.
*/
RGFWDEF u8 RGFW_rgfwToKeyChar(u32 keycode);

/**!
 * @brief Retrieves the size of the RGFW_info structure.
 * @return The size (in bytes) of RGFW_info.
*/
RGFWDEF size_t RGFW_sizeofInfo(void);

/**!
 * @brief Initializes the RGFW library.
 * @return 0 on success, or a negative error code on failure.
 * @note This is automatically called when the first window is created.
*/
RGFWDEF i32 RGFW_init(void);

/**!
 * @brief Deinitializes the RGFW library.
 * @note This is automatically called when the last open window is closed.
*/
RGFWDEF void RGFW_deinit(void);

/**!
 * @brief Initializes RGFW using a user-provided RGFW_info structure.
 * @param info A pointer to an RGFW_info structure to be used for initialization.
 * @return 0 on success, or a negative error code on failure.
*/
RGFWDEF i32 RGFW_init_ptr(RGFW_info* info);

/**!
 * @brief Deinitializes a specific RGFW instance stored in the provided RGFW_info pointer.
 * @param info A pointer to the RGFW_info structure representing the instance to deinitialize.
*/
RGFWDEF void RGFW_deinit_ptr(RGFW_info* info);

/**!
 * @brief Sets the global RGFW_info structure pointer.
 * @param info A pointer to the RGFW_info structure to set.
*/
RGFWDEF void RGFW_setInfo(RGFW_info* info);

/**!
 * @brief Retrieves the global RGFW_info structure pointer.
 * @return A pointer to the current RGFW_info structure.
*/
RGFWDEF RGFW_info* RGFW_getInfo(void);

/** @} */
#endif /* RGFW_HEADER */

#if !defined(RGFW_NATIVE_HEADER) && (defined(RGFW_NATIVE) || defined(RGFW_IMPLEMENTATION))
#define RGFW_NATIVE_HEADER
	#if (defined(RGFW_OPENGL) || defined(RGFW_WEGL)) && defined(_MSC_VER)
		#pragma comment(lib, "opengl32")
	#endif

	#ifdef RGFW_OPENGL
		struct RGFW_eglContext {
			void* ctx;
			void* surface;
			struct wl_egl_window* eglWindow;
		};

		typedef union RGFW_gfxContext {
			RGFW_glContext* native;
			RGFW_eglContext* egl;
		} RGFW_gfxContext;

		typedef RGFW_ENUM(u32, RGFW_gfxContextType) {
			RGFW_gfxNativeOpenGL = RGFW_BIT(0),
			RGFW_gfxEGL = RGFW_BIT(1),
			RGFW_gfxOwnedByRGFW = RGFW_BIT(2)
		};
	#endif

	/*! source data for the window (used by the APIs) */
	#ifdef RGFW_WINDOWS

	#define WIN32_LEAN_AND_MEAN
	#define OEMRESOURCE
	#include <windows.h>

	struct RGFW_nativeImage {
		HBITMAP bitmap;
		u8* bitmapBits;
		RGFW_format format;
		HDC hdcMem;
	};

	#ifdef RGFW_OPENGL
		struct RGFW_glContext {    HGLRC ctx;	};
	#endif

	struct RGFW_window_src {
		HWND window; /*!< source window */
		HDC hdc; /*!< source HDC */
		i32 offsetW, offsetH; /*!< width and height offset for window */
		HICON hIconSmall, hIconBig; /*!< source window icons */
		i32 maxSizeW, maxSizeH, minSizeW, minSizeH, aspectRatioW, aspectRatioH; /*!< for setting max/min resize (RGFW_WINDOWS) */
		#ifdef RGFW_OPENGL
			RGFW_gfxContext ctx;
			RGFW_gfxContextType gfxType;
		#endif
	};

#elif defined(RGFW_UNIX)
	#ifdef RGFW_X11
		#include <X11/Xlib.h>
		#include <X11/Xutil.h>
	#endif

	#ifdef RGFW_WAYLAND
		#ifdef RGFW_LIBDECOR
			#include <libdecor-0/libdecor.h>
		#endif

		#include <wayland-client.h>
		#include <errno.h>
	#endif

	struct RGFW_nativeImage {
		#ifdef RGFW_X11
			XImage* bitmap;
		#endif
		#ifdef RGFW_WAYLAND
			struct wl_buffer* wl_buffer;
		#endif
		u8* buffer;
		RGFW_format format;
	};

	#ifdef RGFW_OPENGL
		struct RGFW_glContext {
			#ifdef RGFW_X11
				struct __GLXcontextRec* ctx; /*!< source graphics context */
				Window window;
			#endif
			#ifdef RGFW_WAYLAND
				RGFW_eglContext egl;
			#endif
		};
	#endif

	struct RGFW_window_src {
		i32 x, y, w, h;
	#ifdef RGFW_OPENGL
		RGFW_gfxContext ctx;
		RGFW_gfxContextType gfxType;
	#endif
#ifdef RGFW_X11
		Window window; /*!< source window */
		Window parent; /*!< parent window */
		GC gc;
		#ifdef RGFW_ADVANCED_SMOOTH_RESIZE
			i64 counter_value;
			XID counter;
		#endif
#endif /* RGFW_X11 */

#if defined(RGFW_WAYLAND)
		struct wl_surface* surface;
		struct xdg_surface* xdg_surface;
		struct xdg_toplevel* xdg_toplevel;
		struct zxdg_toplevel_decoration_v1* decoration;
		struct zwp_locked_pointer_v1 *locked_pointer;
		struct xdg_toplevel_icon_v1 *icon;
		u32 decoration_mode;
		/* State flags to configure the window */
		RGFW_bool pending_activated;
		RGFW_bool activated;
		RGFW_bool resizing;
		RGFW_bool pending_maximized;
		RGFW_bool maximized;
		RGFW_bool minimized;

		RGFW_bool using_custom_cursor;
		struct wl_surface* custom_cursor_surface;

		RGFW_monitor active_monitor;

		struct wl_data_source *data_source; // offer data to other clients

		#ifdef RGFW_LIBDECOR
			struct libdecor* decorContext;
		#endif
#endif /* RGFW_WAYLAND */
	};

#elif defined(RGFW_MACOS)

	struct RGFW_nativeImage {
		RGFW_format format;
	};

	#ifdef RGFW_OPENGL
		struct RGFW_glContext {	 void* ctx;  };
	#endif

	struct RGFW_window_src {
		void* window;
		void* view; /* apple viewpoint thingy */
		void* mouse;
		#ifdef RGFW_OPENGL
			RGFW_gfxContext ctx;
			RGFW_gfxContextType gfxType;
		#endif
	};

#elif defined(RGFW_WASM)

	#include <emscripten/html5.h>
	#include <emscripten/key_codes.h>

	struct RGFW_nativeImage  {
		RGFW_format format;
	};

	#ifdef RGFW_OPENGL
		struct RGFW_glContext {
			EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
		};
	#endif

	struct RGFW_window_src {
		#ifdef RGFW_OPENGL
			RGFW_gfxContext ctx;
			RGFW_gfxContextType gfxType;
		#endif
	};

#endif

struct RGFW_surface {
	u8* data;
	i32 w, h;
	RGFW_format format;
	RGFW_nativeImage native;
};

/*! internal window data that is not specific to the OS */
typedef struct RGFW_windowInternal {
	/*! which key RGFW_window_shouldClose checks. Settting this to RGFW_keyNULL disables the feature. */
	RGFW_key exitKey;
	i32 lastMouseX, lastMouseY; /*!< last cusor point (for raw mouse data) */

	RGFW_bool shouldClose;
	RGFW_bool holdMouse;
	RGFW_bool inFocus;
	RGFW_bool mouseInside;
	RGFW_keymod mod;
	RGFW_eventFlag enabledEvents;
	u32 flags; /*!< windows flags (for RGFW to check and modify) */
	i32 oldX, oldY, oldW, oldH;
} RGFW_windowInternal;

struct RGFW_window {
	RGFW_window_src src; /*!< src window data */
	RGFW_windowInternal internal; /*!< internal window data that is not specific to the OS */
	void* userPtr; /* ptr for usr data */
	i32 x, y, w, h; /*!< position and size of the window */
}; /*!< window structure for the window */

typedef struct RGFW_windowState {
	RGFW_bool mouseEnter;
	RGFW_bool dataDragging;
	RGFW_bool dataDrop;
	size_t filesCount;
	i32 dropX, dropY;
	RGFW_window* win; /*!< it's not possible for one of these events to happen in the frame that the other event happened */

	RGFW_bool mouseLeave;
	RGFW_window* winLeave; /*!< if a mouse leaves one widow and enters the next */
} RGFW_windowState;

typedef struct {
	RGFW_bool current;
	RGFW_bool prev;
} RGFW_keyState;

#ifndef RGFW_NO_MONITOR
	typedef struct RGFW_monitorNode {
		RGFW_monitor mon;
		struct RGFW_monitorNode* next;
#ifdef RGFW_WAYLAND
	u32 id; /* Add id so wl_outputs can be removed */
	struct wl_output *output;
	struct zxdg_output_v1 *xdg_output;
#endif
	} RGFW_monitorNode;

	typedef struct RGFW_monitorList {
		RGFW_monitorNode* head;
		RGFW_monitorNode* cur;
	} RGFW_monitorList;

	typedef struct RGFW_monitors {
		RGFW_monitorList list;
		RGFW_monitorList freeList;
		size_t count;
		RGFW_monitorNode data[RGFW_MAX_MONITORS];
	} RGFW_monitors;

	RGFWDEF RGFW_monitorNode* RGFW_monitors_add(RGFW_monitor mon);
	RGFWDEF void RGFW_monitors_remove(RGFW_monitorNode* node, RGFW_monitorNode* prev);
#endif

struct RGFW_info {
    RGFW_window* root;
    i32 windowCount;

	RGFW_mouse* hiddenMouse;

    RGFW_event events[RGFW_MAX_EVENTS]; /* A circular buffer (FIFO), using eventBottom/Len  */

	i32 eventBottom;
    i32 eventLen;
	RGFW_bool queueEvents;
	RGFW_bool polledEvents;

    u32 apiKeycodes[RGFW_keyLast];
	#if defined(RGFW_X11) || defined(RGFW_WAYLAND)
		u8 keycodes[256];
	#elif defined(RGFW_WINDOWS)
		u8 keycodes[512];
	#elif defined(RGFW_MACOS)
		u8 keycodes[128];
	#elif defined(RGFW_WASM)
		u8 keycodes[256];
	#endif

    const char* className;
    RGFW_bool useWaylandBool;
    RGFW_bool stopCheckEvents_bool ;
    u64 timerOffset;

    char* clipboard_data;
    char* clipboard; /* for writing to the clipboard selection */
    size_t clipboard_len;
    char filesSrc[RGFW_MAX_PATH * RGFW_MAX_DROPS];
	char** files;
	#ifdef RGFW_X11
        Display* display;
		XContext context;
        Window helperWindow;
        const char* instName;
        XErrorEvent* x11Error;
    #endif
    #ifdef RGFW_WAYLAND
        struct wl_display* wl_display;
        struct xkb_context *xkb_context;
        struct xkb_keymap *keymap;
        struct xkb_state *xkb_state;
        struct zxdg_decoration_manager_v1 *decoration_manager;
        struct zwp_relative_pointer_manager_v1 *relative_pointer_manager;
        struct zwp_relative_pointer_v1 *relative_pointer;
        struct zwp_pointer_constraints_v1 *constraint_manager;
        struct xdg_toplevel_icon_manager_v1 *icon_manager;

        struct zxdg_output_manager_v1 *xdg_output_manager;

        struct wl_data_device_manager *data_device_manager;
        struct wl_data_device *data_device; // supports clipboard and DND

        struct wl_keyboard* wl_keyboard;
        struct wl_pointer* wl_pointer;
        struct wl_compositor* compositor;
        struct xdg_wm_base* xdg_wm_base;
        struct wl_shm* shm;
        struct wl_seat *seat;
        struct wl_registry *registry;
        u32 mouse_enter_serial;
        struct wl_cursor_theme* wl_cursor_theme;
        struct wl_surface* cursor_surface;

        RGFW_window* kbOwner;

    #endif

    RGFW_monitors monitors;

    #ifdef RGFW_UNIX
	    int eventWait_forceStop[3];
    #endif

    #ifdef RGFW_MACOS
    void* NSApp;
	void* customViewClasses[2]; /* NSView and NSOpenGLView  */
	void* customWindowDelegateClass;
    #endif

	#ifdef RGFW_OPENGL
		RGFW_window* current;
	#endif
	#ifdef RGFW_EGL
		void* EGL_display;
	#endif

	RGFW_window* mouseOwner;
	RGFW_windowState windowState; /*! for checking window state events */

	RGFW_keyState mouseButtons[RGFW_mouseFinal];
	RGFW_keyState keyboard[RGFW_keyLast];
	float scrollX, scrollY;
	float vectorX, vectorY;
};
#endif /* RGFW_NATIVE_HEADER */

#ifdef RGFW_IMPLEMENTATION

/* global private API */

/* for C++ / C89 */
#define RGFW_eventQueuePushEx(eventInit) { RGFW_event e; eventInit; RGFW_eventQueuePush(&e); }

RGFWDEF RGFW_window* RGFW_createWindowPlatform(const char* name, RGFW_windowFlags flags, RGFW_window* win);
RGFWDEF void RGFW_window_closePlatform(RGFW_window* win);

RGFWDEF void RGFW_window_focusLost(RGFW_window* win);
RGFWDEF void RGFW_window_setFlagsInternal(RGFW_window* win, RGFW_windowFlags flags, RGFW_windowFlags cmpFlags);

RGFWDEF void RGFW_initKeycodes(void);
RGFWDEF void RGFW_initKeycodesPlatform(void);
RGFWDEF void RGFW_resetPrevState(void);
RGFWDEF void RGFW_resetKey(void);
RGFWDEF void RGFW_unloadEGL(void);
RGFWDEF void RGFW_updateKeyModsEx(RGFW_window* win, RGFW_bool capital, RGFW_bool numlock, RGFW_bool control, RGFW_bool alt, RGFW_bool shift, RGFW_bool super, RGFW_bool scroll);
RGFWDEF void RGFW_updateKeyMods(RGFW_window* win, RGFW_bool capital, RGFW_bool numlock, RGFW_bool scroll);
RGFWDEF void RGFW_window_showMouseFlags(RGFW_window* win, RGFW_bool show);
RGFWDEF void RGFW_updateKeyMod(RGFW_window* win, RGFW_keymod mod, RGFW_bool value);

RGFWDEF void RGFW_setBit(u32* var, u32 mask, RGFW_bool set);
RGFWDEF void RGFW_splitBPP(u32 bpp, RGFW_monitorMode* mode);

RGFWDEF void RGFW_captureCursor(RGFW_window* win);
RGFWDEF void RGFW_releaseCursor(RGFW_window* win);

RGFWDEF void RGFW_copyImageData64(u8* dest_data, i32 w, i32 h, RGFW_format dest_format,
							u8* src_data, RGFW_format src_format,			RGFW_bool is64bit);

RGFWDEF RGFW_bool RGFW_loadEGL(void);

#ifdef RGFW_OPENGL
typedef struct RGFW_attribStack {
	i32* attribs;
	size_t count;
	size_t max;
} RGFW_attribStack;
RGFWDEF void RGFW_attribStack_init(RGFW_attribStack* stack, i32* attribs, size_t max);
RGFWDEF void RGFW_attribStack_pushAttrib(RGFW_attribStack* stack, i32 attrib);
RGFWDEF void RGFW_attribStack_pushAttribs(RGFW_attribStack* stack, i32 attrib1, i32 attrib2);

RGFWDEF RGFW_bool RGFW_extensionSupportedStr(const char* extensions, const char* ext, size_t len);
#endif

typedef struct RGFW_colorLayout {  i32 r, g, b, a; } RGFW_colorLayout;

#ifdef RGFW_X11
RGFWDEF void RGFW_XCreateWindow (XVisualInfo visual, const char* name, RGFW_windowFlags flags, RGFW_window* win);
#endif
#ifdef RGFW_MACOS
RGFWDEF void RGFW_osx_initView(RGFW_window* win);
#endif
/* end of global private API defs */

RGFW_info* _RGFW = NULL;
void RGFW_setInfo(RGFW_info* info) { _RGFW = info; }
RGFW_info* RGFW_getInfo(void) { return _RGFW; }


void* RGFW_alloc(size_t size) { return RGFW_ALLOC(size); }
void RGFW_free(void* ptr) { RGFW_FREE(ptr); }

void RGFW_useWayland(RGFW_bool wayland) { RGFW_init(); _RGFW->useWaylandBool = RGFW_BOOL(wayland);  }
RGFW_bool RGFW_usingWayland(void) { return _RGFW->useWaylandBool; }

void RGFW_clipboard_switch(char* newstr);
void RGFW_clipboard_switch(char* newstr) {
	if (_RGFW->clipboard_data != NULL)
		RGFW_FREE(_RGFW->clipboard_data);
	_RGFW->clipboard_data =  newstr;
}

#define RGFW_CHECK_CLIPBOARD() \
	if (size <= 0 && _RGFW->clipboard_data != NULL) \
		return (const char*)_RGFW->clipboard_data; \
	else if (size <= 0) \
		return "\0";

const char* RGFW_readClipboard(size_t* len) {
	RGFW_ssize_t size = RGFW_readClipboardPtr(NULL, 0);
    RGFW_CHECK_CLIPBOARD();
    char* str = (char*)RGFW_ALLOC((size_t)size);
    RGFW_ASSERT(str != NULL);
    str[0] = '\0';

    size = RGFW_readClipboardPtr(str, (size_t)size);

    RGFW_CHECK_CLIPBOARD();

	if (len != NULL) *len = (size_t)size;

	RGFW_clipboard_switch(str);
	return (const char*)str;
}

/*
RGFW_IMPLEMENTATION starts with generic RGFW defines

This is the start of keycode data
*/



void RGFW_initKeycodes(void) {
	RGFW_MEMSET(_RGFW->keycodes, 0, sizeof(_RGFW->keycodes));
	RGFW_initKeycodesPlatform();
    u32 i, y;
    for (i = 0; i < RGFW_keyLast; i++) {
        for (y = 0; y < sizeof(_RGFW->keycodes); y++) {
            if (_RGFW->keycodes[y] == i) {
                _RGFW->apiKeycodes[i] = y;
                break;
            }
        }
    }


    RGFW_resetKey();
}

u32 RGFW_apiKeyToRGFW(u32 keycode) {
    /* make sure the key isn't out of bounds */
	if (keycode > sizeof(_RGFW->keycodes) / sizeof(u8))
		return 0;

	return _RGFW->keycodes[keycode];
}

u32 RGFW_rgfwToApiKey(u32 keycode) {
	/* make sure the key isn't out of bounds */
	if (keycode > sizeof(_RGFW->apiKeycodes) / sizeof(u32))
		return 0;

	return _RGFW->apiKeycodes[keycode];
}

void RGFW_resetKey(void) { RGFW_MEMSET(_RGFW->keyboard, 0, sizeof(_RGFW->keyboard)); }
/*
	this is the end of keycode data
*/

/*
	event callback defines start here
*/


/*
	These exist to avoid the
	if (func == NULL) check
	for (allegedly) better performance

	RGFW_EMPTY_DEF exists to prevent the missing-prototypes warning
*/
#define RGFW_CALLBACK_DEFINE(x, x2) \
RGFW_##x##func RGFW_##x##CallbackSrc = NULL; \
RGFW_##x##func RGFW_set##x2##Callback(RGFW_##x##func func) { \
    RGFW_##x##func prev = RGFW_##x##CallbackSrc; \
    RGFW_##x##CallbackSrc = func; \
    return prev; \
}

RGFW_CALLBACK_DEFINE(windowMaximized, WindowMaximized)
#define RGFW_windowMaximizedCallback(win, x, y, w, h) if (RGFW_windowMaximizedCallbackSrc) RGFW_windowMaximizedCallbackSrc(win, x, y, w, h);

RGFW_CALLBACK_DEFINE(windowMinimized, WindowMinimized)
#define RGFW_windowMinimizedCallback(w) if (RGFW_windowMinimizedCallbackSrc) RGFW_windowMinimizedCallbackSrc(w);

RGFW_CALLBACK_DEFINE(windowRestored, WindowRestored)
#define RGFW_windowRestoredCallback(win, x, y, w, h) if (RGFW_windowRestoredCallbackSrc) RGFW_windowRestoredCallbackSrc(win, x, y, w, h);

RGFW_CALLBACK_DEFINE(windowMoved, WindowMoved)
#define RGFW_windowMovedCallback(w, x, y) if (RGFW_windowMovedCallbackSrc) RGFW_windowMovedCallbackSrc(w, x, y);

RGFW_CALLBACK_DEFINE(windowResized, WindowResized)
#define RGFW_windowResizedCallback(win, w, h) if (RGFW_windowResizedCallbackSrc) RGFW_windowResizedCallbackSrc(win, w, h);

RGFW_CALLBACK_DEFINE(windowQuit, WindowQuit)
#define RGFW_windowQuitCallback(w) if (RGFW_windowQuitCallbackSrc) RGFW_windowQuitCallbackSrc(w);

RGFW_CALLBACK_DEFINE(mousePos, MousePos)
#define RGFW_mousePosCallback(w, x, y, vecX, vecY) if (RGFW_mousePosCallbackSrc) RGFW_mousePosCallbackSrc(w, x, y, vecX, vecY);

RGFW_CALLBACK_DEFINE(windowRefresh, WindowRefresh)
#define RGFW_windowRefreshCallback(w) if (RGFW_windowRefreshCallbackSrc) RGFW_windowRefreshCallbackSrc(w);

RGFW_CALLBACK_DEFINE(focus, Focus)
#define RGFW_focusCallback(w, inFocus) if (RGFW_focusCallbackSrc) RGFW_focusCallbackSrc(w, inFocus);

RGFW_CALLBACK_DEFINE(mouseNotify, MouseNotify)
#define RGFW_mouseNotifyCallback(w, x, y, status) if (RGFW_mouseNotifyCallbackSrc) RGFW_mouseNotifyCallbackSrc(w, x, y, status);

RGFW_CALLBACK_DEFINE(dataDrop, DataDrop)
#define RGFW_dataDropCallback(w, files, count) if (RGFW_dataDropCallbackSrc) RGFW_dataDropCallbackSrc(w, files, count);

RGFW_CALLBACK_DEFINE(dataDrag, DataDrag)
#define RGFW_dataDragCallback(w, x, y) if (RGFW_dataDragCallbackSrc) RGFW_dataDragCallbackSrc(w, x, y);

RGFW_CALLBACK_DEFINE(key, Key)
#define RGFW_keyCallback(w, key, sym, mod, repeat, press) if (RGFW_keyCallbackSrc) RGFW_keyCallbackSrc(w, key, sym, mod, repeat, press);

RGFW_CALLBACK_DEFINE(mouseButton, MouseButton)
#define RGFW_mouseButtonCallback(w, button, press) if (RGFW_mouseButtonCallbackSrc) RGFW_mouseButtonCallbackSrc(w, button, press);

RGFW_CALLBACK_DEFINE(mouseScroll, MouseScroll)
#define RGFW_mouseScrollCallback(w, x, y) if (RGFW_mouseScrollCallbackSrc) RGFW_mouseScrollCallbackSrc(w, x, y);

RGFW_CALLBACK_DEFINE(scaleUpdated, ScaleUpdated)
#define RGFW_scaleUpdatedCallback(w, scaleX, scaleY) if (RGFW_scaleUpdatedCallbackSrc) RGFW_scaleUpdatedCallbackSrc(w, scaleX, scaleY);

RGFW_CALLBACK_DEFINE(debug, Debug)
#define RGFW_debugCallback(type, err, msg) if (RGFW_debugCallbackSrc) RGFW_debugCallbackSrc(type, err, msg);
#undef RGFW_CALLBACK_DEFINE

#ifdef RGFW_DEBUG
#include <stdio.h>
#endif

void RGFW_sendDebugInfo(RGFW_debugType type, RGFW_errorCode err, const char* msg) {
	RGFW_debugCallback(type, err, msg);

    #ifdef RGFW_DEBUG
	switch (type) {
		case RGFW_typeInfo: RGFW_PRINTF("RGFW INFO (%i %i): %s", type, err, msg); break;
		case RGFW_typeError: RGFW_PRINTF("RGFW DEBUG (%i %i): %s", type, err, msg); break;
		case RGFW_typeWarning: RGFW_PRINTF("RGFW WARNING (%i %i): %s", type, err, msg); break;
		default: break;
	}

	RGFW_PRINTF("\n");
	#endif
}

void RGFW_window_checkMode(RGFW_window* win);
void RGFW_window_checkMode(RGFW_window* win) {
	if (RGFW_window_isMinimized(win) && (win->internal.enabledEvents & RGFW_windowMinimizedFlag)) {
		win->internal.flags |= RGFW_windowMinimize;
		RGFW_eventQueuePushEx(e.type = RGFW_windowMinimized; e.common.win = win);
		RGFW_windowMinimizedCallback(win);
	} else if (RGFW_window_isMaximized(win) && (win->internal.enabledEvents & RGFW_windowMaximizedFlag)) {
		win->internal.flags |= RGFW_windowMaximize;
		RGFW_eventQueuePushEx(e.type = RGFW_windowMaximized; e.common.win = win);
		RGFW_windowMaximizedCallback(win, win->x, win->y, win->w, win->h);
	} else if ((((win->internal.flags & RGFW_windowMinimize) && !RGFW_window_isMaximized(win)) ||
				(win->internal.flags & RGFW_windowMaximize && !RGFW_window_isMaximized(win))) && (win->internal.enabledEvents & RGFW_windowRestoredFlag)) {
		win->internal.flags &= ~(u32)RGFW_windowMinimize;
		if (RGFW_window_isMaximized(win) == RGFW_FALSE) win->internal.flags &= ~(u32)RGFW_windowMaximize;
		RGFW_eventQueuePushEx(e.type = RGFW_windowRestored; e.common.win = win);
		RGFW_windowRestoredCallback(win, win->x, win->y, win->w, win->h);
	}
}

/*
no more event call back defines
*/

size_t RGFW_sizeofInfo(void) { return sizeof(RGFW_info); }
size_t RGFW_sizeofNativeImage(void) { return sizeof(RGFW_nativeImage); }
size_t RGFW_sizeofSurface(void) { return sizeof(RGFW_surface); }
size_t RGFW_sizeofWindow(void) { return sizeof(RGFW_window); }
size_t RGFW_sizeofWindowSrc(void) { return sizeof(RGFW_window_src); }

RGFW_window_src* RGFW_window_getSrc(RGFW_window* win) { return &win->src; }
RGFW_bool RGFW_window_getPosition(RGFW_window* win, i32* x, i32* y) { if (x) *x = win->x; if (y) *y = win->y; return RGFW_TRUE; }
RGFW_bool RGFW_window_getSize(RGFW_window* win, i32* w, i32* h) { if (w) *w = win->w; if (h) *h = win->h; return RGFW_TRUE; }
u32 RGFW_window_getFlags(RGFW_window* win) { return win->internal.flags; }
RGFW_key RGFW_window_getExitKey(RGFW_window* win) { return win->internal.exitKey; }
void RGFW_window_setExitKey(RGFW_window* win, RGFW_key key) { win->internal.exitKey = key; }
void RGFW_window_setEnabledEvents(RGFW_window* win, RGFW_eventFlag events) { win->internal.enabledEvents = events; }
RGFW_eventFlag RGFW_window_getEnabledEvents(RGFW_window* win) { return win->internal.enabledEvents; }
void RGFW_window_setDisabledEvents(RGFW_window* win, RGFW_eventFlag events) {   RGFW_window_setEnabledEvents(win, (RGFW_allEventFlags) & ~(u32)events);  }
void RGFW_window_setEventState(RGFW_window* win, RGFW_eventFlag event, RGFW_bool state) { RGFW_setBit(&win->internal.enabledEvents, event, state); }
void* RGFW_window_getUserPtr(RGFW_window* win) { return win->userPtr; }
void RGFW_window_setUserPtr(RGFW_window* win, void* ptr) { win->userPtr = ptr; }


#if defined(RGFW_USE_XDL) && defined(RGFW_X11)
	#define XDL_IMPLEMENTATION
	#include "XDL.h"
#endif

#ifndef RGFW_FORCE_INIT
RGFW_info _rgfwGlobal;
#endif

i32 RGFW_init(void) { return RGFW_init_ptr(&_rgfwGlobal); }
void RGFW_deinit(void) { RGFW_deinit_ptr(&_rgfwGlobal); }

i32 RGFW_initPlatform(void);
void RGFW_deinitPlatform(void);

i32 RGFW_init_ptr(RGFW_info* info) {
    if (info == _RGFW || info == NULL) return 1;

    RGFW_setInfo(info);
    RGFW_MEMSET(_RGFW, 0, sizeof(RGFW_info));
	_RGFW->queueEvents = RGFW_FALSE;
	_RGFW->polledEvents = RGFW_FALSE;
#ifdef RGFW_WAYLAND
	_RGFW->useWaylandBool = RGFW_TRUE;
#endif

	_RGFW->files = (char**)(void*)_RGFW->filesSrc;
	u32 i;
	for (i = 0; i < RGFW_MAX_DROPS; i++)
		_RGFW->files[i] = (char*)(_RGFW->filesSrc + RGFW_MAX_DROPS + (i * RGFW_MAX_PATH));

	_RGFW->monitors.freeList.head = &_RGFW->monitors.data[0];
	_RGFW->monitors.freeList.cur = _RGFW->monitors.freeList.head;

	for (i = 1; i < RGFW_MAX_MONITORS; i++) {
		RGFW_monitorNode* newNode = &_RGFW->monitors.data[i];
		_RGFW->monitors.freeList.cur->next = newNode;
		_RGFW->monitors.freeList.cur = _RGFW->monitors.freeList.cur->next;
	}

    RGFW_initKeycodes();
    i32 out = RGFW_initPlatform();
    RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal, "global context initialized");

	return out;
}

#ifndef RGFW_EGL
void RGFW_unloadEGL(void) { }
#endif

void RGFW_deinit_ptr(RGFW_info* info) {
    if (info == NULL) return;

    RGFW_setInfo(info);
	RGFW_unloadEGL();
	RGFW_deinitPlatform();

    _RGFW->root = NULL;
    _RGFW->windowCount = 0;
    RGFW_setInfo(NULL);
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal, "global context deinitialized");
}

RGFW_window* RGFW_createWindow(const char* name, i32 x, i32 y, i32 w, i32 h, RGFW_windowFlags flags) {
	RGFW_window* win = (RGFW_window*)RGFW_ALLOC(sizeof(RGFW_window));
	RGFW_ASSERT(win != NULL);
    return RGFW_createWindowPtr(name, x, y, w, h, flags, win);
}

void RGFW_window_close(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_window_closePtr(win);
	RGFW_FREE(win);
}

RGFW_window* RGFW_createWindowPtr(const char* name, i32 x, i32 y, i32 w, i32 h, RGFW_windowFlags flags, RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_MEMSET(win, 0, sizeof(RGFW_window));
	if (_RGFW == NULL) RGFW_init();
    _RGFW->windowCount++;

	/* rect based the requested flags */
	if (_RGFW->root == NULL) {
		RGFW_setRootWindow(win);
	}

	/* set and init the new window's data */
	win->x = x;
	win->y = y;
	win->w = w;
	win->h = h;
	win->internal.flags = flags;
	win->internal.enabledEvents = RGFW_allEventFlags;

	RGFW_window* ret = RGFW_createWindowPlatform(name, flags, win);

#ifndef RGFW_X11
	RGFW_window_setFlagsInternal(win, flags, 0);
#endif

#ifdef RGFW_OPENGL
	win->src.gfxType = 0;
	if (flags & RGFW_windowOpenGL)
		RGFW_window_createContext_OpenGL(win, RGFW_getGlobalHints_OpenGL());
#endif

#ifdef RGFW_EGL
	if (flags & RGFW_windowEGL)
		RGFW_window_createContext_EGL(win, RGFW_getGlobalHints_OpenGL());
#endif

	/* X11 creates the window after the OpenGL context is created (because of visual garbage),
		 * so we have to wait to set the flags
		 * This is required so that way the user can create their own OpenGL context after RGFW_createWindow is used
		 * if a window is crated, CreateContext will delete the window and create a new one
		 * */
#ifdef RGFW_X11
	RGFW_window_setFlagsInternal(win, flags, 0);
#endif

#ifdef RGFW_MACOS
	/*NOTE: another OpenGL/setFlags related hack, this because OSX the 'view' class must be setup after the NSOpenGL view is made AND after setFlags happens */
	RGFW_osx_initView(win);
#endif

#ifdef RGFW_WAYLAND
	/* recieve all events needed to configure the surface */
	/* also gets the wl_outputs */
	if (RGFW_usingWayland()) {
		wl_display_roundtrip(_RGFW->wl_display);
		/* NOTE: this is a hack so that way wayland spawns a window, even if nothing is drawn */
		if (!(flags & RGFW_windowOpenGL) && !(flags & RGFW_windowEGL)) {
			u8* data = (u8*)RGFW_ALLOC((u32)(win->w * win->h * 3));
			RGFW_MEMSET(data, 0, (u32)(win->w * win->h * 3) * sizeof(u8));
			RGFW_surface* surface = RGFW_createSurface(data, win->w, win->h, RGFW_formatBGR8);
			RGFW_window_blitSurface(win, surface);
			RGFW_FREE(data);
			RGFW_surface_free(surface);
		}
	}
#endif

	RGFW_window_setMouseDefault(win);
	RGFW_window_setName(win, name);
	if (!(flags & RGFW_windowHide)) {
		RGFW_window_show(win);
	}

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, "a new window was created");


	return ret;
}

void RGFW_window_closePtr(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	#ifdef RGFW_EGL
	if ((win->src.gfxType & RGFW_gfxEGL) && win->src.ctx.egl) {
		RGFW_window_deleteContext_EGL(win, win->src.ctx.egl);
		win->src.ctx.egl = NULL;
	}
	#endif

	#ifdef RGFW_OPENGL
	if ((win->src.gfxType & RGFW_gfxNativeOpenGL) && win->src.ctx.native) {
		RGFW_window_deleteContext_OpenGL(win, win->src.ctx.native);
		win->src.ctx.native = NULL;
	}
	#endif

	RGFW_window_closePlatform(win);

	RGFW_clipboard_switch(NULL);
	_RGFW->windowCount--;
	if (_RGFW->windowCount == 0) RGFW_deinit();

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, "a window was freed");
}

void RGFW_setQueueEvents(RGFW_bool queue) {  _RGFW->queueEvents = RGFW_BOOL(queue); }

void RGFW_eventQueueFlush(void) { _RGFW->eventLen = 0; }

void RGFW_eventQueuePush(const RGFW_event* event) {
	if (_RGFW->queueEvents == RGFW_FALSE) return;
	RGFW_ASSERT(_RGFW->eventLen >= 0);

	if (_RGFW->eventLen >= RGFW_MAX_EVENTS) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errEventQueue, "Event queue limit 'RGFW_MAX_EVENTS' has been reached automatically flushing queue.");
		RGFW_eventQueueFlush();
		return;
	}

	i32 eventTop = (_RGFW->eventBottom + _RGFW->eventLen) % RGFW_MAX_EVENTS;
	_RGFW->eventLen += 1;
	_RGFW->events[eventTop] = *event;
}

RGFW_event* RGFW_eventQueuePop(RGFW_window* win) {
	RGFW_ASSERT(_RGFW->eventLen >= 0 && _RGFW->eventLen <= RGFW_MAX_EVENTS);
	RGFW_event* ev;

	if (_RGFW->eventLen == 0) {
		return NULL;
	}

	ev = &_RGFW->events[_RGFW->eventBottom];
	_RGFW->eventLen -= 1;
    _RGFW->eventBottom = (_RGFW->eventBottom + 1) % RGFW_MAX_EVENTS;

	if (ev->common.win != win && ev->common.win != NULL) {
		RGFW_eventQueuePush(ev);
		return NULL;
	}

	return ev;
}

void RGFW_resetPrevState(void) {
	size_t i; /*!< reset each previous state  */
    for (i = 0; i < RGFW_keyLast; i++) _RGFW->keyboard[i].prev = _RGFW->keyboard[i].current;
    for (i = 0; i < RGFW_mouseFinal; i++) _RGFW->mouseButtons[i].prev = _RGFW->mouseButtons[i].current;
	_RGFW->scrollX = 0.0f;
	_RGFW->scrollY = 0.0f;
	_RGFW->vectorX = (float)0.0f;
	_RGFW->vectorY = (float)0.0f;
	RGFW_MEMSET(&_RGFW->windowState, 0, sizeof(_RGFW->windowState));
}

RGFW_bool RGFW_isKeyPressed(RGFW_key key) {
    return _RGFW != NULL && _RGFW->keyboard[key].current && !_RGFW->keyboard[key].prev;
}

RGFW_bool RGFW_isKeyDown(RGFW_key key) {
	return _RGFW != NULL && _RGFW->keyboard[key].current;
}

RGFW_bool RGFW_isKeyReleased(RGFW_key key) {
	return _RGFW != NULL && !_RGFW->keyboard[key].current && _RGFW->keyboard[key].prev;
}


RGFW_bool RGFW_isMousePressed(RGFW_mouseButton button) {
	return _RGFW != NULL && _RGFW->mouseButtons[button].current && !_RGFW->mouseButtons[button].prev;
}
RGFW_bool RGFW_isMouseDown(RGFW_mouseButton button) {
	return _RGFW != NULL && _RGFW->mouseButtons[button].current;
}
RGFW_bool RGFW_isMouseReleased(RGFW_mouseButton button) {
	return _RGFW != NULL && !_RGFW->mouseButtons[button].current && _RGFW->mouseButtons[button].prev;
}

void RGFW_getMouseScroll(float* x, float* y) {
	RGFW_ASSERT(_RGFW != NULL);
	if (x) *x = _RGFW->scrollX;
	if (y) *y = _RGFW->scrollY;
}

void RGFW_getMouseVector(float* x, float* y) {
	RGFW_ASSERT(_RGFW != NULL);
	if (x) *x = _RGFW->vectorX;
	if (y) *y = _RGFW->vectorY;
}

RGFW_bool RGFW_window_didMouseLeave(RGFW_window* win) { return _RGFW->windowState.winLeave == win && _RGFW->windowState.mouseLeave; }
RGFW_bool RGFW_window_didMouseEnter(RGFW_window* win) { return _RGFW->windowState.win == win && _RGFW->windowState.mouseEnter; }
RGFW_bool RGFW_window_isMouseInside(RGFW_window* win) { return win->internal.mouseInside;  }

RGFW_bool RGFW_window_isDataDragging(RGFW_window* win) { return RGFW_window_getDataDrag(win, (i32*)NULL, (i32*)NULL); }
RGFW_bool RGFW_window_didDataDrop(RGFW_window* win) { return RGFW_window_getDataDrop(win, (const char***)NULL, (size_t*)NULL);}


RGFW_bool RGFW_window_getDataDrag(RGFW_window* win, i32* x, i32* y) {
	if (_RGFW->windowState.win != win || _RGFW->windowState.dataDragging == RGFW_FALSE) return RGFW_FALSE;
	if (x) *x = _RGFW->windowState.dropX;
	if (y) *y =  _RGFW->windowState.dropY;
	return RGFW_TRUE;
}
RGFW_bool RGFW_window_getDataDrop(RGFW_window* win, const char*** files, size_t* count) {
	if (_RGFW->windowState.win != win || _RGFW->windowState.dataDrop == RGFW_FALSE) return RGFW_FALSE;
	if (files) *files = (const char**)_RGFW->files;
	if (count) *count = _RGFW->windowState.filesCount;
	return RGFW_TRUE;
}

RGFW_bool RGFW_window_checkEvent(RGFW_window* win, RGFW_event* event) {
	if (_RGFW->eventLen == 0 && _RGFW->polledEvents == RGFW_FALSE) {
		_RGFW->queueEvents = RGFW_TRUE;
		RGFW_pollEvents();
		_RGFW->polledEvents = RGFW_TRUE;
	}

	if (RGFW_window_checkQueuedEvent(win, event) == RGFW_FALSE) {
		_RGFW->polledEvents = RGFW_FALSE;
		return RGFW_FALSE;
	}

	return RGFW_TRUE;
}

RGFW_bool RGFW_window_checkQueuedEvent(RGFW_window* win, RGFW_event* event) {
	RGFW_event* ev;
	RGFW_ASSERT(win != NULL);
	_RGFW->queueEvents = RGFW_TRUE;
	/* check queued events */
	ev = RGFW_eventQueuePop(win);
	if (ev != NULL) {
		if (ev->type == RGFW_quit) RGFW_window_setShouldClose(win, RGFW_TRUE);
		*event = *ev;
		return RGFW_TRUE;
    }

	return RGFW_FALSE;
}

void RGFW_setRootWindow(RGFW_window* win) { _RGFW->root = win; }
RGFW_window* RGFW_getRootWindow(void) { return _RGFW->root; }

#ifndef RGFW_EGL
RGFW_bool RGFW_loadEGL(void) { return RGFW_FALSE; }
#endif

void RGFW_window_setFlagsInternal(RGFW_window* win, RGFW_windowFlags flags, RGFW_windowFlags cmpFlags) {
	#ifndef RGFW_NO_MONITOR
	if (flags & RGFW_windowScaleToMonitor)			RGFW_window_scaleToMonitor(win);
	#endif

	if (flags & RGFW_windowCenter)					RGFW_window_center(win);
	if (flags & RGFW_windowCenterCursor) RGFW_window_moveMouse(win, win->x + (win->w / 2), win->y + (win->h / 2));
	if (flags & RGFW_windowNoBorder)				RGFW_window_setBorder(win, 0);
	else if (cmpFlags & RGFW_windowNoBorder) RGFW_window_setBorder(win, 1);
	if (flags & RGFW_windowFullscreen)				RGFW_window_setFullscreen(win, RGFW_TRUE);
	else if (cmpFlags & RGFW_windowFullscreen) 	RGFW_window_setFullscreen(win, 0);
	if (flags & RGFW_windowMaximize)				RGFW_window_maximize(win);
	else if (cmpFlags & RGFW_windowMaximize) 	RGFW_window_restore(win);
	if (flags & RGFW_windowMinimize)				RGFW_window_minimize(win);
	else if (cmpFlags & RGFW_windowMinimize) 	RGFW_window_restore(win);
	if (flags & RGFW_windowHideMouse)				RGFW_window_showMouse(win, 0);
	else if (cmpFlags & RGFW_windowHideMouse)  	RGFW_window_showMouse(win, 1);
	if (flags & RGFW_windowHide)				RGFW_window_hide(win);
	else if (cmpFlags & RGFW_windowHide)  		RGFW_window_show(win);
	if (flags & RGFW_windowFloating)				RGFW_window_setFloating(win, 1);
	else if (cmpFlags & RGFW_windowFloating)		RGFW_window_setFloating(win, 0);
	if (flags & RGFW_windowFocus)					RGFW_window_focus(win);

	if (flags & RGFW_windowNoResize) {
	    RGFW_window_setMaxSize(win, win->w, win->h);
	    RGFW_window_setMinSize(win, win->w, win->h);
	} else if (cmpFlags & RGFW_windowNoResize) {
		RGFW_window_setMaxSize(win, 0, 0);
		RGFW_window_setMinSize(win, 0, 0);
	}

	win->internal.flags = flags;
}


void RGFW_window_setFlags(RGFW_window* win, RGFW_windowFlags flags) { RGFW_window_setFlagsInternal(win, flags, win->internal.flags); }

RGFW_bool RGFW_window_isInFocus(RGFW_window* win) {
#ifdef RGFW_WASM
    return RGFW_TRUE;
#else
    return RGFW_BOOL(win->internal.inFocus);
#endif
}

void RGFW_setClassName(const char* name) { RGFW_init(); _RGFW->className = name; }

#ifndef RGFW_X11
void RGFW_setXInstName(const char* name) { RGFW_UNUSED(name); }
#endif

RGFW_bool RGFW_window_getMouse(RGFW_window* win, i32* x, i32* y) {
	RGFW_ASSERT(win != NULL);
	if (x) *x = win->internal.lastMouseX;
	if (y) *y = win->internal.lastMouseY;
	return RGFW_TRUE;
}

RGFW_bool RGFW_window_isKeyPressed(RGFW_window* win, RGFW_key key) { return RGFW_isKeyPressed(key)  && RGFW_window_isInFocus(win); }
RGFW_bool RGFW_window_isKeyDown(RGFW_window* win, RGFW_key key) { return RGFW_isKeyDown(key) && RGFW_window_isInFocus(win); }
RGFW_bool RGFW_window_isKeyReleased(RGFW_window* win, RGFW_key key) { return RGFW_isKeyReleased(key) && RGFW_window_isInFocus(win); }

RGFW_bool RGFW_window_isMousePressed(RGFW_window* win, RGFW_mouseButton button) { return RGFW_isMousePressed(button) && RGFW_window_isInFocus(win); }
RGFW_bool RGFW_window_isMouseDown(RGFW_window* win, RGFW_mouseButton button) { return RGFW_isMouseDown(button) && RGFW_window_isInFocus(win); }
RGFW_bool RGFW_window_isMouseReleased(RGFW_window* win, RGFW_mouseButton button) { return RGFW_isMouseReleased(button) && RGFW_window_isInFocus(win); }



#ifndef RGFW_X11
void* RGFW_getDisplay_X11(void) { return NULL; }
u64 RGFW_window_getWindow_X11(RGFW_window* win) { RGFW_UNUSED(win); return 0; }
#endif

#ifndef RGFW_WAYLAND
struct wl_display* RGFW_getDisplay_Wayland(void) { return NULL; }
struct wl_surface* RGFW_window_getWindow_Wayland(RGFW_window* win) { RGFW_UNUSED(win); return NULL; }
#endif

#ifndef RGFW_WINDOWS
void* RGFW_window_getHWND(RGFW_window* win) { RGFW_UNUSED(win); return NULL; }
void* RGFW_window_getHDC(RGFW_window* win) { RGFW_UNUSED(win); return NULL; }
#endif

#ifndef RGFW_MACOS
void* RGFW_window_getView_OSX(RGFW_window* win) { RGFW_UNUSED(win); return NULL; }
void RGFW_window_setLayer_OSX(RGFW_window* win, void* layer) { RGFW_UNUSED(win); RGFW_UNUSED(layer); }
void* RGFW_getLayer_OSX(void) { return NULL; }
void* RGFW_window_getWindow_OSX(RGFW_window* win) { RGFW_UNUSED(win); return NULL; }
#endif

void RGFW_setBit(u32* var, u32 mask, RGFW_bool set) {
	if (set) *var |=  mask;
	else     *var &= ~mask;
}

void RGFW_window_center(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_monitor mon = RGFW_window_getMonitor(win);
	RGFW_window_move(win, (i32)(mon.mode.w - win->w) / 2, (mon.mode.h - win->h) / 2);
}

RGFW_bool RGFW_monitor_scaleToWindow(RGFW_monitor mon, RGFW_window* win) {
	RGFW_monitorMode mode;
    RGFW_ASSERT(win != NULL);

	mode.w = win->w;
	mode.h = win->h;
	return RGFW_monitor_requestMode(mon, mode, RGFW_monitorScale);
}

void RGFW_splitBPP(u32 bpp, RGFW_monitorMode* mode) {
    if (bpp == 32) bpp = 24;
    mode->red = mode->green = mode->blue = (u8)(bpp / 3);

    u32 delta = bpp - (mode->red * 3); /* handle leftovers */
    if (delta >= 1) mode->green = mode->green + 1;
    if (delta == 2) mode->red = mode->red + 1;
}

RGFW_bool RGFW_monitorModeCompare(RGFW_monitorMode mon, RGFW_monitorMode mon2, RGFW_modeRequest request) {
	return (((mon.w == mon2.w && mon.h == mon2.h) || !(request & RGFW_monitorScale)) &&
			((mon.refreshRate == mon2.refreshRate) || !(request & RGFW_monitorRefresh)) &&
			((mon.red == mon2.red && mon.green == mon2.green && mon.blue == mon2.blue) || !(request & RGFW_monitorRGB)));
}

RGFW_bool RGFW_window_shouldClose(RGFW_window* win) {
	return (win == NULL || win->internal.shouldClose || (win->internal.exitKey && RGFW_window_isKeyPressed(win, win->internal.exitKey)));
}

void RGFW_window_setShouldClose(RGFW_window* win, RGFW_bool shouldClose) {
	if (shouldClose)  {
		win->internal.shouldClose = RGFW_TRUE;
		RGFW_windowQuitCallback(win);
	} else {
		win->internal.shouldClose = RGFW_FALSE;
	}
}

#ifndef RGFW_NO_MONITOR
void RGFW_window_scaleToMonitor(RGFW_window* win) {
	RGFW_monitor monitor = RGFW_window_getMonitor(win);
	if (monitor.scaleX == 0 && monitor.scaleY == 0)
		return;

	RGFW_window_resize(win, (i32)(monitor.scaleX * (float)win->w), (i32)(monitor.scaleY * (float)win->h));
}

void RGFW_window_moveToMonitor(RGFW_window* win, RGFW_monitor m) {
	RGFW_window_move(win, m.x + win->x, m.y + win->y);
}
#endif

RGFW_surface* RGFW_createSurface(u8* data, i32 w, i32 h, RGFW_format format) {
	RGFW_surface* surface = (RGFW_surface*)RGFW_ALLOC(sizeof(RGFW_surface));
	RGFW_MEMSET(surface, 0, sizeof(RGFW_surface));
	RGFW_createSurfacePtr(data, w, h, format, surface);
	return surface;
}

void RGFW_surface_free(RGFW_surface* surface) {
	RGFW_surface_freePtr(surface);
	RGFW_FREE(surface);
}

RGFW_nativeImage* RGFW_surface_getNativeImage(RGFW_surface* surface) {
	return &surface->native;
}

RGFW_surface* RGFW_window_createSurface(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format) {
	RGFW_surface* surface = (RGFW_surface*)RGFW_ALLOC(sizeof(RGFW_surface));
	RGFW_MEMSET(surface, 0, sizeof(RGFW_surface));
	RGFW_window_createSurfacePtr(win, data, w, h, format, surface);
	return surface;
}
#ifndef RGFW_X11
RGFW_bool RGFW_window_createSurfacePtr(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format, RGFW_surface* surface) {
	RGFW_UNUSED(win);
	return RGFW_createSurfacePtr(data, w, h, format, surface);
}
#endif

const RGFW_colorLayout RGFW_layouts[RGFW_formatCount] = {
	{ 0, 1, 2, 3 }, /* RGFW_formatRGB8 */
	{ 2, 1, 0, 3 }, /* RGFW_formatBGR8 */
	{ 0, 1, 2, 3 }, /* RGFW_formatRGBA8 */
	{ 1, 2, 3, 0 }, /* RGFW_formatARGB8 */
	{ 2, 1, 0, 3 }, /* RGFW_formatBGRA8 */
	{ 3, 2, 1, 0 }, /* RGFW_formatABGR8 */
};


void RGFW_copyImageData(u8* dest_data, i32 w, i32 h, RGFW_format dest_format, u8* src_data, RGFW_format src_format) {
	RGFW_copyImageData64(dest_data, w, h, dest_format, src_data, src_format, RGFW_FALSE);
}

void RGFW_copyImageData64(u8* dest_data, i32 dest_w, i32 dest_h, RGFW_format dest_format, u8* src_data, RGFW_format src_format, RGFW_bool is64bit) {
	RGFW_ASSERT(dest_data && src_data);

	u32 src_channels = (src_format >= RGFW_formatRGBA8) ? 4 : 3;
	u32 dest_channels = (dest_format >= RGFW_formatRGBA8) ? 4 : 3;

    u32 pixel_count = (u32)(dest_w * dest_h);

	if (src_format == dest_format) {
        RGFW_MEMCPY(dest_data, src_data, pixel_count * dest_channels);
        return;
    }

    const RGFW_colorLayout* src_layout  = &RGFW_layouts[src_format];
    const RGFW_colorLayout* dest_layout = &RGFW_layouts[dest_format];

	u32 i, i2 = 0;
	for (i = 0; i < pixel_count; i++) {
		const u8* src_px = &src_data[i * src_channels];
		u8* dst_px = &dest_data[i2 * dest_channels];
		u8 rgba[4] = {0};
        rgba[0] = src_px[src_layout->r];
        rgba[1] = src_px[src_layout->g];
        rgba[2] = src_px[src_layout->b];
        rgba[3] = 255;
		if (src_channels == 4)
			rgba[3] = src_px[src_layout->a];

        dst_px[dest_layout->r] = rgba[0];
        dst_px[dest_layout->g] = rgba[1];
        dst_px[dest_layout->b] = rgba[2];
		if (dest_channels == 4)
			dst_px[dest_layout->a] = rgba[3];

		i2 += 1 + is64bit;
	}
}

RGFW_monitorNode* RGFW_monitors_add(RGFW_monitor mon) {
	RGFW_monitorNode* node = NULL;
	if (_RGFW->monitors.freeList.head == NULL) return node;

	node = _RGFW->monitors.freeList.head;
	mon = node->mon;

	_RGFW->monitors.freeList.head = node->next;
	if (_RGFW->monitors.freeList.head == NULL) {
		_RGFW->monitors.freeList.cur = NULL;
	}

	node->next = NULL;

	if (_RGFW->monitors.list.head == NULL) {
		_RGFW->monitors.list.head = node;
	} else {
		_RGFW->monitors.list.cur->next = node;
	}

	_RGFW->monitors.list.cur = node;

	node->mon = mon;
	_RGFW->monitors.count += 1;
	return node;
}

void RGFW_monitors_remove(RGFW_monitorNode* node, RGFW_monitorNode* prev) {
	_RGFW->monitors.count -= 1;

	/* remove node from the list */
	if (prev != node) {
		prev->next = node->next;
	} else { /* node is the head */
		_RGFW->monitors.list.head = NULL;
	}

	node->next = NULL;

	/* move node to the free list */
	if (_RGFW->monitors.freeList.head == NULL) {
		_RGFW->monitors.freeList.head = node;
	} else {
		_RGFW->monitors.freeList.cur->next = node;
	}

	_RGFW->monitors.freeList.cur = node;
}

RGFW_bool RGFW_window_setIcon(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format) {
	return RGFW_window_setIconEx(win, data, w, h, format, RGFW_iconBoth);
}

void RGFW_window_holdMouse(RGFW_window* win) {
	win->internal.holdMouse = RGFW_TRUE;
	_RGFW->mouseOwner = win;
	RGFW_captureCursor(win);
	RGFW_window_moveMouse(win, win->x + (win->w / 2), win->y + (win->h / 2));
}

RGFW_bool RGFW_window_isHoldingMouse(RGFW_window* win) { return RGFW_BOOL(win->internal.holdMouse); }

void RGFW_window_unholdMouse(RGFW_window* win) {
	win->internal.holdMouse = RGFW_FALSE;
	_RGFW->mouseOwner = NULL;
	RGFW_releaseCursor(win);
}

void RGFW_updateKeyMod(RGFW_window* win, RGFW_keymod mod, RGFW_bool value) {
	if (value) win->internal.mod |= mod;
	else win->internal.mod &= ~mod;
}

void RGFW_updateKeyModsEx(RGFW_window* win, RGFW_bool capital, RGFW_bool numlock, RGFW_bool control, RGFW_bool alt, RGFW_bool shift, RGFW_bool super, RGFW_bool scroll) {
	RGFW_updateKeyMod(win, RGFW_modCapsLock, capital);
	RGFW_updateKeyMod(win, RGFW_modNumLock, numlock);
	RGFW_updateKeyMod(win, RGFW_modControl, control);
	RGFW_updateKeyMod(win, RGFW_modAlt, alt);
	RGFW_updateKeyMod(win, RGFW_modShift, shift);
	RGFW_updateKeyMod(win, RGFW_modSuper, super);
	RGFW_updateKeyMod(win, RGFW_modScrollLock, scroll);
}

void RGFW_updateKeyMods(RGFW_window* win, RGFW_bool capital, RGFW_bool numlock, RGFW_bool scroll) {
	RGFW_updateKeyModsEx(win, capital, numlock,
					RGFW_window_isKeyDown(win, RGFW_controlL) || RGFW_window_isKeyDown(win, RGFW_controlR),
					RGFW_window_isKeyDown(win, RGFW_altL) || RGFW_window_isKeyDown(win, RGFW_altR),
					RGFW_window_isKeyDown(win, RGFW_shiftL) || RGFW_window_isKeyDown(win, RGFW_shiftR),
					RGFW_window_isKeyDown(win, RGFW_superL) || RGFW_window_isKeyDown(win, RGFW_superR),
					scroll);
}

void RGFW_window_showMouseFlags(RGFW_window* win, RGFW_bool show) {
	if (show && (win->internal.flags & RGFW_windowHideMouse))
		win->internal.flags ^= RGFW_windowHideMouse;
	else if (!show && !(win->internal.flags & RGFW_windowHideMouse))
		win->internal.flags |= RGFW_windowHideMouse;
}

RGFW_bool RGFW_window_isMouseHidden(RGFW_window* win) {
	return (RGFW_bool)RGFW_BOOL(((RGFW_window*)win)->internal.flags & RGFW_windowHideMouse);
}

RGFW_bool RGFW_window_borderless(RGFW_window* win) {
	return (RGFW_bool)RGFW_BOOL(win->internal.flags & RGFW_windowNoBorder);
}

RGFW_bool RGFW_window_isFullscreen(RGFW_window* win){ return RGFW_BOOL(win->internal.flags & RGFW_windowFullscreen); }
RGFW_bool RGFW_window_allowsDND(RGFW_window* win) { return RGFW_BOOL(win->internal.flags & RGFW_windowAllowDND); }

void RGFW_window_focusLost(RGFW_window* win) {
    /* standard routines for when a window looses focus */
	win->internal.inFocus = RGFW_FALSE;
	if ((win->internal.flags & RGFW_windowFullscreen))
			RGFW_window_minimize(win);

    size_t key;
    for (key = 0; key < RGFW_keyLast; key++) {
		if (RGFW_isKeyDown((u8)key) == RGFW_FALSE) continue;

		_RGFW->keyboard[key].current = RGFW_FALSE;
        u8 sym = RGFW_rgfwToKeyChar((u32)key);

		if ((win->internal.enabledEvents & RGFW_BIT(RGFW_keyReleased))) {
			RGFW_keyCallback(win, (u8)key, sym, win->internal.mod, RGFW_FALSE, RGFW_FALSE);
			RGFW_eventQueuePushEx(e.type = RGFW_keyReleased;
									e.key.value = (u8)key;
									e.key.sym = sym;
									e.key.repeat = RGFW_FALSE;
									e.key.mod = win->internal.mod;
									e.common.win = win);
		}
    }

    RGFW_resetKey();
}

#ifndef RGFW_WINDOWS
void RGFW_window_setDND(RGFW_window* win, RGFW_bool allow) {
	RGFW_setBit(&win->internal.flags, RGFW_windowAllowDND, allow);
}
#endif

#if defined(RGFW_X11) || defined(RGFW_MACOS) || defined(RGFW_WASM) || defined(RGFW_WAYLAND)
#ifndef __USE_POSIX199309
	#define __USE_POSIX199309
#endif
#include <time.h>
struct timespec;
#endif

#if defined(RGFW_WAYLAND) || defined(RGFW_X11) || defined(RGFW_WINDOWS)
void RGFW_window_showMouse(RGFW_window* win, RGFW_bool show) {
	RGFW_window_showMouseFlags(win, show);
	if (show == RGFW_FALSE)
		RGFW_window_setMouse(win, _RGFW->hiddenMouse);
	else
		RGFW_window_setMouseDefault(win);
}
#endif

#ifndef RGFW_MACOS
void RGFW_moveToMacOSResourceDir(void) { }
#endif

/*
	graphics API specific code (end of generic code)
	starts here
*/


/*
	OpenGL defines start here   (Normal, EGL, OSMesa)
*/

#if defined(RGFW_OPENGL)
/* EGL, OpenGL */
#define RGFW_DEFAULT_GL_HINTS { \
	/* Stencil         */ 0, \
	/* Samples         */ 0, \
	/* Stereo          */ RGFW_FALSE, \
	/* AuxBuffers      */ 0, \
	/* DoubleBuffer    */ RGFW_TRUE, \
	/* Red             */ 8, \
	/* Green           */ 8, \
	/* Blue            */ 8, \
	/* Alpha           */ 8, \
	/* Depth           */ 24, \
	/* AccumRed        */ 0, \
	/* AccumGreen      */ 0, \
	/* AccumBlue       */ 0, \
	/* AccumAlpha      */ 0, \
	/* SRGB            */ RGFW_FALSE, \
	/* Robustness      */ RGFW_FALSE, \
	/* Debug           */ RGFW_FALSE, \
	/* NoError         */ RGFW_FALSE, \
	/* ReleaseBehavior */ RGFW_glReleaseNone, \
	/* Profile         */ RGFW_glCore, \
	/* Major           */ 1, \
	/* Minor           */ 0, \
	/* Share */ NULL, \
	/* Share_EGL */ NULL, \
	/* renderer */ RGFW_glAccelerated \
}

RGFW_glHints RGFW_globalHints_OpenGL_SRC = RGFW_DEFAULT_GL_HINTS;
RGFW_glHints* RGFW_globalHints_OpenGL = &RGFW_globalHints_OpenGL_SRC;

void RGFW_resetGlobalHints_OpenGL(void) {
#if !defined(__cplusplus) || defined(RGFW_MACOS)
	RGFW_globalHints_OpenGL_SRC = (RGFW_glHints)RGFW_DEFAULT_GL_HINTS;
#else
	RGFW_globalHints_OpenGL_SRC = RGFW_DEFAULT_GL_HINTS;
#endif
}
void RGFW_setGlobalHints_OpenGL(RGFW_glHints* hints) { RGFW_globalHints_OpenGL = hints;  }
RGFW_glHints* RGFW_getGlobalHints_OpenGL(void) { RGFW_init(); return RGFW_globalHints_OpenGL; }


void* RGFW_glContext_getSourceContext(RGFW_glContext* ctx) {
	RGFW_UNUSED(ctx);

#ifdef RGFW_WAYLAND
	if (RGFW_usingWayland()) return (void*)ctx->egl.ctx;
#endif

#if defined(RGFW_X11)
	return (void*)ctx->ctx;
#else
	return NULL;
#endif
}

RGFW_glContext* RGFW_window_createContext_OpenGL(RGFW_window* win, RGFW_glHints* hints) {
	#ifdef RGFW_WAYLAND
	if (RGFW_usingWayland()) {
		return (RGFW_glContext*)RGFW_window_createContext_EGL(win, hints);
	}
	#endif
	RGFW_glContext* ctx = (RGFW_glContext*)RGFW_ALLOC(sizeof(RGFW_glContext));
	if (RGFW_window_createContextPtr_OpenGL(win, ctx, hints) == RGFW_FALSE) {
		RGFW_FREE(ctx);
		win->src.ctx.native = NULL;
		return NULL;
	}
	win->src.gfxType |= RGFW_gfxOwnedByRGFW;
	return ctx;
}

RGFW_glContext* RGFW_window_getContext_OpenGL(RGFW_window* win) {
	if (win->src.gfxType & RGFW_windowEGL) return NULL;
	return win->src.ctx.native;
}

void RGFW_window_deleteContext_OpenGL(RGFW_window* win, RGFW_glContext* ctx) {
	RGFW_window_deleteContextPtr_OpenGL(win, ctx);
	if (win->src.gfxType & RGFW_gfxOwnedByRGFW) RGFW_FREE(ctx);
}

RGFW_bool RGFW_extensionSupportedStr(const char* extensions, const char* ext, size_t len) {
    const char *start = extensions;
    const char *where;
    const char* terminator;

    if (extensions == NULL || ext == NULL) {
        return RGFW_FALSE;
	}

	while (ext[len - 1] == '\0' && len > 3) {
		len--;
	}

    where = RGFW_STRSTR(extensions, ext);
    while (where) {
		terminator = where + len;
        if ((where == start || *(where - 1) == ' ') &&
            (*terminator == ' ' || *terminator == '\0')) {
			return RGFW_TRUE;
        }
        where = RGFW_STRSTR(terminator, ext);
    }

    return RGFW_FALSE;
}

RGFWDEF RGFW_bool RGFW_extensionSupported_base(const char* extension, size_t len);
RGFW_bool RGFW_extensionSupported_base(const char* extension, size_t len) {
    #ifdef GL_NUM_EXTENSIONS
    if (RGFW_globalHints_OpenGL->major >= 3) {
        i32 i;

        GLint count = 0;

        RGFW_proc RGFW_glGetStringi = RGFW_getProcAddress_OpenGL("glGetStringi");
        RGFW_proc RGFW_glGetIntegerv = RGFW_getProcAddress_OpenGL("glGetIntegerv");
		if (RGFW_glGetIntegerv)
            ((void(*)(GLenum, GLint*))RGFW_glGetIntegerv)(GL_NUM_EXTENSIONS, &count);

        for (i = 0; RGFW_glGetStringi && i < count;  i++) {
            const char* en = ((const char* (*)(u32, u32))RGFW_glGetStringi)(GL_EXTENSIONS, (u32)i);
            if (en && RGFW_STRNCMP(en, extension, len) == 0) {
				return RGFW_TRUE;
			}
        }
    } else
#endif
    {
        RGFW_proc RGFW_glGetString = RGFW_getProcAddress_OpenGL("glGetString");
		#define RGFW_GL_EXTENSIONS 0x1F03
        if (RGFW_glGetString) {
            const char* extensions = ((const char*(*)(u32))RGFW_glGetString)(RGFW_GL_EXTENSIONS);

            if ((extensions != NULL) && RGFW_extensionSupportedStr(extensions, extension, len)) {
				return RGFW_TRUE;
			}
        }
    }
	return RGFW_FALSE;
}

RGFW_bool RGFW_extensionSupported_OpenGL(const char* extension, size_t len) {
	if (RGFW_extensionSupported_base(extension, len))  return RGFW_TRUE;
    return RGFW_extensionSupportedPlatform_OpenGL(extension, len);
}

void RGFW_window_makeCurrentWindow_OpenGL(RGFW_window* win) {
	if (win) {
		_RGFW->current = win;
	}

    RGFW_window_makeCurrentContext_OpenGL(win);
}

RGFW_window* RGFW_getCurrentWindow_OpenGL(void) { return _RGFW->current; }
void RGFW_attribStack_init(RGFW_attribStack* stack, i32* attribs, size_t max) { stack->attribs = attribs; stack->count = 0; stack->max = max; }
void RGFW_attribStack_pushAttrib(RGFW_attribStack* stack, i32 attrib) {
	RGFW_ASSERT(stack->count < stack->max);
	stack->attribs[stack->count] = attrib;
	stack->count += 1;
}
void RGFW_attribStack_pushAttribs(RGFW_attribStack* stack, i32 attrib1, i32 attrib2) {
	RGFW_attribStack_pushAttrib(stack, attrib1);
	RGFW_attribStack_pushAttrib(stack, attrib2);
}

/* EGL */
#ifdef RGFW_EGL
#include <EGL/egl.h>

PFNEGLINITIALIZEPROC RGFW_eglInitialize;
PFNEGLGETCONFIGSPROC RGFW_eglGetConfigs;
PFNEGLCHOOSECONFIGPROC RGFW_eglChooseConfig;
PFNEGLCREATEWINDOWSURFACEPROC RGFW_eglCreateWindowSurface;
PFNEGLCREATECONTEXTPROC RGFW_eglCreateContext;
PFNEGLMAKECURRENTPROC RGFW_eglMakeCurrent;
PFNEGLGETDISPLAYPROC RGFW_eglGetDisplay;
PFNEGLSWAPBUFFERSPROC RGFW_eglSwapBuffers;
PFNEGLSWAPINTERVALPROC RGFW_eglSwapInterval;
PFNEGLBINDAPIPROC RGFW_eglBindAPI;
PFNEGLDESTROYCONTEXTPROC RGFW_eglDestroyContext;
PFNEGLTERMINATEPROC RGFW_eglTerminate;
PFNEGLDESTROYSURFACEPROC RGFW_eglDestroySurface;
PFNEGLGETCURRENTCONTEXTPROC RGFW_eglGetCurrentContext;
PFNEGLGETPROCADDRESSPROC RGFW_eglGetProcAddress = NULL;
PFNEGLQUERYSTRINGPROC RGFW_eglQueryString;
PFNEGLGETCONFIGATTRIBPROC RGFW_eglGetConfigAttrib;

#define EGL_SURFACE_MAJOR_VERSION_KHR 0x3098
#define EGL_SURFACE_MINOR_VERSION_KHR 0x30fb

#ifdef RGFW_WINDOWS
    #include <windows.h>
#elif defined(RGFW_MACOS) || defined(RGFW_UNIX)
    #include <dlfcn.h>
#endif

#ifdef RGFW_WAYLAND
#include <wayland-egl.h>
#endif

void* RGFW_eglLibHandle = NULL;

void* RGFW_getDisplay_EGL(void) { return _RGFW->EGL_display; }
void* RGFW_eglContext_getSourceContext(RGFW_eglContext* ctx) { return ctx->ctx; }
void* RGFW_eglContext_getSurface(RGFW_eglContext* ctx) { return ctx->surface; }
struct wl_egl_window* RGFW_eglContext_wlEGLWindow(RGFW_eglContext* ctx) { return ctx->eglWindow; }

RGFW_bool RGFW_loadEGL(void) {
	RGFW_init();
	if (RGFW_eglGetProcAddress != NULL) {
		return RGFW_TRUE;
	}

#ifndef RGFW_WASM
	#ifdef RGFW_WINDOWS
		const char* libNames[] = { "libEGL.dll", "EGL.dll" };
	#elif defined(RGFW_MACOS) || defined(RGFW_UNIX)
		/* Linux and macOS */
		const char* libNames[] = {
			"libEGL.so.1",  /* most common */
			"libEGL.so",    /* fallback */
			"/System/Library/Frameworks/OpenGL.framework/OpenGL"  /* fallback for older macOS EGL-like systems */
		};
	#endif

	for (size_t i = 0; i < sizeof(libNames) / sizeof(libNames[0]); ++i) {
		#ifdef RGFW_WINDOWS
			RGFW_eglLibHandle = (void*)LoadLibraryA(libNames[i]);
			if (RGFW_eglLibHandle) {
				RGFW_eglGetProcAddress = (PFNEGLGETPROCADDRESSPROC)(RGFW_proc)GetProcAddress((HMODULE)RGFW_eglLibHandle, "eglGetProcAddress");
				break;
			}
		#elif defined(RGFW_MACOS) || defined(RGFW_UNIX)
			RGFW_eglLibHandle = dlopen(libNames[i], RTLD_LAZY | RTLD_GLOBAL);
			if (RGFW_eglLibHandle) {
				void* lib = dlsym(RGFW_eglLibHandle, "eglGetProcAddress");
				if (lib != NULL) RGFW_MEMCPY(&RGFW_eglGetProcAddress, &lib, sizeof(PFNEGLGETPROCADDRESSPROC));
				break;
			}
		#endif
	}

	if (!RGFW_eglLibHandle || !RGFW_eglGetProcAddress) {
		return RGFW_FALSE;
	}

	RGFW_eglInitialize = (PFNEGLINITIALIZEPROC) RGFW_eglGetProcAddress("eglInitialize");
	RGFW_eglGetConfigs = (PFNEGLGETCONFIGSPROC) RGFW_eglGetProcAddress("eglGetConfigs");
	RGFW_eglChooseConfig = (PFNEGLCHOOSECONFIGPROC) RGFW_eglGetProcAddress("eglChooseConfig");
	RGFW_eglCreateWindowSurface = (PFNEGLCREATEWINDOWSURFACEPROC) RGFW_eglGetProcAddress("eglCreateWindowSurface");
	RGFW_eglCreateContext = (PFNEGLCREATECONTEXTPROC) RGFW_eglGetProcAddress("eglCreateContext");
	RGFW_eglMakeCurrent = (PFNEGLMAKECURRENTPROC) RGFW_eglGetProcAddress("eglMakeCurrent");
	RGFW_eglGetDisplay = (PFNEGLGETDISPLAYPROC) RGFW_eglGetProcAddress("eglGetDisplay");
	RGFW_eglSwapBuffers = (PFNEGLSWAPBUFFERSPROC) RGFW_eglGetProcAddress("eglSwapBuffers");
	RGFW_eglSwapInterval = (PFNEGLSWAPINTERVALPROC) RGFW_eglGetProcAddress("eglSwapInterval");
	RGFW_eglBindAPI = (PFNEGLBINDAPIPROC) RGFW_eglGetProcAddress("eglBindAPI");
	RGFW_eglDestroyContext = (PFNEGLDESTROYCONTEXTPROC) RGFW_eglGetProcAddress("eglDestroyContext");
	RGFW_eglTerminate = (PFNEGLTERMINATEPROC) RGFW_eglGetProcAddress("eglTerminate");
	RGFW_eglDestroySurface = (PFNEGLDESTROYSURFACEPROC) RGFW_eglGetProcAddress("eglDestroySurface");
	RGFW_eglQueryString = (PFNEGLQUERYSTRINGPROC) RGFW_eglGetProcAddress("eglQueryString");
	RGFW_eglGetCurrentContext = (PFNEGLGETCURRENTCONTEXTPROC) RGFW_eglGetProcAddress("eglGetCurrentContext");
	RGFW_eglGetConfigAttrib = (PFNEGLGETCONFIGATTRIBPROC) RGFW_eglGetProcAddress("eglGetConfigAttrib");

#else
	RGFW_eglGetProcAddress = eglGetProcAddress;
	RGFW_eglInitialize = (PFNEGLINITIALIZEPROC) eglInitialize;
	RGFW_eglGetConfigs = (PFNEGLGETCONFIGSPROC) eglGetConfigs;
	RGFW_eglChooseConfig = (PFNEGLCHOOSECONFIGPROC) eglChooseConfig;
	RGFW_eglCreateWindowSurface = (PFNEGLCREATEWINDOWSURFACEPROC) eglCreateWindowSurface;
	RGFW_eglCreateContext = (PFNEGLCREATECONTEXTPROC) eglCreateContext;
	RGFW_eglMakeCurrent = (PFNEGLMAKECURRENTPROC) eglMakeCurrent;
	RGFW_eglGetDisplay = (PFNEGLGETDISPLAYPROC) eglGetDisplay;
	RGFW_eglSwapBuffers = (PFNEGLSWAPBUFFERSPROC) eglSwapBuffers;
	RGFW_eglSwapInterval = (PFNEGLSWAPINTERVALPROC) eglSwapInterval;
	RGFW_eglBindAPI = (PFNEGLBINDAPIPROC) eglBindAPI;
	RGFW_eglDestroyContext = (PFNEGLDESTROYCONTEXTPROC) eglDestroyContext;
	RGFW_eglTerminate = (PFNEGLTERMINATEPROC) eglTerminate;
	RGFW_eglDestroySurface = (PFNEGLDESTROYSURFACEPROC) eglDestroySurface;
	RGFW_eglQueryString = (PFNEGLQUERYSTRINGPROC) eglQueryString;
	RGFW_eglGetCurrentContext = (PFNEGLGETCURRENTCONTEXTPROC) eglGetCurrentContext;
	RGFW_eglGetConfigAttrib = (PFNEGLGETCONFIGATTRIBPROC)eglGetConfigAttrib;
#endif

	RGFW_bool out = RGFW_BOOL(RGFW_eglInitialize!= NULL &&
	            RGFW_eglGetConfigs!= NULL &&
	            RGFW_eglChooseConfig!= NULL &&
	            RGFW_eglCreateWindowSurface!= NULL &&
	            RGFW_eglCreateContext!= NULL &&
	            RGFW_eglMakeCurrent!= NULL &&
	            RGFW_eglGetDisplay!= NULL &&
	            RGFW_eglSwapBuffers!= NULL &&
	            RGFW_eglSwapInterval != NULL &&
	            RGFW_eglBindAPI!= NULL &&
	            RGFW_eglDestroyContext!= NULL &&
	            RGFW_eglTerminate!= NULL &&
	            RGFW_eglDestroySurface!= NULL &&
				RGFW_eglQueryString != NULL &&
				RGFW_eglGetCurrentContext != NULL &&
				RGFW_eglGetConfigAttrib != NULL);

	if (out) {
		#ifdef RGFW_WINDOWS
		HDC dc = GetDC(NULL);
		_RGFW->EGL_display = RGFW_eglGetDisplay((EGLNativeDisplayType) dc);
		ReleaseDC(NULL, dc);
		#elif defined(RGFW_WAYLAND)
		if (_RGFW->useWaylandBool)
			_RGFW->EGL_display = RGFW_eglGetDisplay((EGLNativeDisplayType) _RGFW->wl_display);
		else
		#endif
		#ifdef RGFW_X11
			_RGFW->EGL_display = RGFW_eglGetDisplay((EGLNativeDisplayType) _RGFW->display);
		#else
		{}
		#endif
		#if !defined(RGFW_WAYLAND) && !defined(RGFW_WINDOWS) && !defined(RGFW_X11)
		_RGFW->EGL_display = RGFW_eglGetDisplay(EGL_DEFAULT_DISPLAY);
		#endif
	}

	RGFW_eglInitialize(_RGFW->EGL_display, NULL, NULL);
	return out;
}


void RGFW_unloadEGL(void) {
	if (!RGFW_eglLibHandle) return;
	RGFW_eglTerminate(_RGFW->EGL_display);
	#ifdef RGFW_WINDOWS
	    FreeLibrary((HMODULE)RGFW_eglLibHandle);
	#elif defined(RGFW_MACOS) || defined(RGFW_UNIX)
	    dlclose(RGFW_eglLibHandle);
	#endif

    RGFW_eglLibHandle = NULL;
    RGFW_eglGetProcAddress = NULL;
}

RGFW_bool RGFW_window_createContextPtr_EGL(RGFW_window* win, RGFW_eglContext* ctx, RGFW_glHints* hints) {
	if (RGFW_loadEGL() == RGFW_FALSE) return RGFW_FALSE;
	win->src.ctx.egl = ctx;
	win->src.gfxType = RGFW_gfxEGL;

#ifdef RGFW_WAYLAND
    if (_RGFW->useWaylandBool)
        win->src.ctx.egl->eglWindow = wl_egl_window_create(win->src.surface, win->w, win->h);
#endif

	#ifndef EGL_OPENGL_ES1_BIT
	#define EGL_OPENGL_ES1_BIT 0x1
	#endif

	EGLint egl_config[24];

	{
		RGFW_attribStack stack;
		RGFW_attribStack_init(&stack, egl_config, 24);

		RGFW_attribStack_pushAttribs(&stack, EGL_SURFACE_TYPE, EGL_WINDOW_BIT);
		RGFW_attribStack_pushAttrib(&stack, EGL_RENDERABLE_TYPE);

		if (hints->profile == RGFW_glES) {
			switch (hints->major) {
				case 1: RGFW_attribStack_pushAttrib(&stack, EGL_OPENGL_ES1_BIT); break;
				case 2: RGFW_attribStack_pushAttrib(&stack, EGL_OPENGL_ES2_BIT); break;
				case 3: RGFW_attribStack_pushAttrib(&stack, EGL_OPENGL_ES3_BIT); break;
				default: break;
			}
		} else {
			RGFW_attribStack_pushAttrib(&stack, EGL_OPENGL_BIT);
		}

		RGFW_attribStack_pushAttribs(&stack, EGL_RED_SIZE, hints->red);
		RGFW_attribStack_pushAttribs(&stack, EGL_GREEN_SIZE, hints->green);
		RGFW_attribStack_pushAttribs(&stack, EGL_BLUE_SIZE, hints->blue);
		RGFW_attribStack_pushAttribs(&stack, EGL_ALPHA_SIZE, hints->alpha);
		RGFW_attribStack_pushAttribs(&stack, EGL_DEPTH_SIZE, hints->depth);

		RGFW_attribStack_pushAttribs(&stack, EGL_STENCIL_SIZE, hints->stencil);
		if (hints->samples) {
			RGFW_attribStack_pushAttribs(&stack, EGL_SAMPLE_BUFFERS, 1);
			RGFW_attribStack_pushAttribs(&stack, EGL_SAMPLES, hints->samples);
		}

		RGFW_attribStack_pushAttribs(&stack, EGL_NONE, EGL_NONE);
	}

	EGLint numConfigs, best_config = -1, best_samples = 0;

	RGFW_eglChooseConfig(_RGFW->EGL_display, egl_config, NULL, 0, &numConfigs);
	EGLConfig* configs = (EGLConfig*)RGFW_ALLOC(sizeof(EGLConfig) * (u32)numConfigs);

	RGFW_eglChooseConfig(_RGFW->EGL_display, egl_config, configs, numConfigs, &numConfigs);

#ifdef RGFW_X11
	RGFW_bool transparent = (win->internal.flags & RGFW_windowTransparent);
	EGLint best_depth = 0;
#endif

	for (EGLint i = 0; i < numConfigs; i++) {
		EGLint visual_id = 0;
		EGLint samples = 0;

		RGFW_eglGetConfigAttrib(_RGFW->EGL_display, configs[i], EGL_NATIVE_VISUAL_ID, &visual_id);
		RGFW_eglGetConfigAttrib(_RGFW->EGL_display, configs[i], EGL_SAMPLES, &samples);

		if (best_config  == -1) best_config = i;

#ifdef RGFW_X11
		if (_RGFW->useWaylandBool == RGFW_FALSE) {
			XVisualInfo vinfo_template;
			vinfo_template.visualid = (VisualID)visual_id;

			int num_visuals = 0;
			XVisualInfo* vi = XGetVisualInfo(_RGFW->display, VisualIDMask, &vinfo_template, &num_visuals);
			if (!vi) continue;
			if ((!transparent || vi->depth == 32) && best_depth == 0) {
				best_config = i;
				best_depth = vi->depth;
			}

			if ((!(transparent) || vi->depth == 32) && (samples <= hints->samples && samples > best_samples)) {
				best_depth = vi->depth;
				best_config = i;
				best_samples = samples;
				XFree(vi);
				continue;
			}
		}
#endif

		if (samples <= hints->samples && samples > best_samples) {
			best_config = i;
			best_samples = samples;
		}
	}

	EGLConfig config = configs[best_config];
	RGFW_FREE(configs);
#ifdef RGFW_X11
    if (_RGFW->useWaylandBool == RGFW_FALSE) {
		/*  This is required so that way the user can create their own OpenGL context after RGFW_createWindow is used */
		XVisualInfo* result;
		XVisualInfo desired;
		EGLint visualID = 0, count = 0;

		RGFW_eglGetConfigAttrib(_RGFW->EGL_display, config, EGL_NATIVE_VISUAL_ID, &visualID);
		if (visualID) {
			desired.visualid = (VisualID)visualID;
			result = XGetVisualInfo(_RGFW->display, VisualIDMask, &desired, &count);
		} else  RGFW_sendDebugInfo(RGFW_typeError, RGFW_errEGLContext,  "Failed to fetch a valid EGL VisualID");

		if (result == NULL || count == 0) {
			if (win->src.window == 0) {
				/* try to create a EGL context anyway (this will work if you're not using a NVidia driver) */
				win->internal.flags &= ~(u32)RGFW_windowEGL;
				RGFW_createWindowPlatform("", win->internal.flags, win);
			}
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errEGLContext,  "Failed to find a valid visual for the EGL config");
		} else {
			if (win->src.window) RGFW_window_closePlatform(win);
			RGFW_XCreateWindow(*result, "", win->internal.flags, win);
			XFree(result);
		}
	}
#endif

	EGLint surf_attribs[9];

	{
		RGFW_attribStack stack;
		RGFW_attribStack_init(&stack, surf_attribs, 9);

		const char present_opaque_str[] = "EGL_EXT_present_opaque";
		RGFW_bool opaque_extension_Found = RGFW_extensionSupportedPlatform_EGL(present_opaque_str, sizeof(present_opaque_str));

		#ifndef EGL_PRESENT_OPAQUE_EXT
		#define EGL_PRESENT_OPAQUE_EXT 0x31df
		#endif

		#ifndef EGL_GL_COLORSPACE_KHR
		#define EGL_GL_COLORSPACE_KHR 0x309D
		#ifndef EGL_GL_COLORSPACE_SRGB_KHR
		#define EGL_GL_COLORSPACE_SRGB_KHR 0x3089
		#endif
		#endif

		const char gl_colorspace_str[] = "EGL_KHR_gl_colorspace";
		RGFW_bool gl_colorspace_Found = RGFW_extensionSupportedPlatform_EGL(gl_colorspace_str, sizeof(gl_colorspace_str));

		if (hints->sRGB && gl_colorspace_Found) {
			RGFW_attribStack_pushAttribs(&stack, EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR);
		}

		if (!(win->internal.flags & RGFW_windowTransparent) && opaque_extension_Found)
			RGFW_attribStack_pushAttribs(&stack, EGL_PRESENT_OPAQUE_EXT, EGL_TRUE);

		if (hints->doubleBuffer == 0) {
			RGFW_attribStack_pushAttribs(&stack, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER);
		}

		RGFW_attribStack_pushAttribs(&stack, EGL_NONE, EGL_NONE);
	}
	#if defined(RGFW_MACOS)
		void* layer = RGFW_getLayer_OSX();

		RGFW_window_setLayer_OSX(win, layer);

		win->src.ctx.egl->surface = RGFW_eglCreateWindowSurface(_RGFW->EGL_display, config, (EGLNativeWindowType) layer, surf_attribs);
	#elif defined(RGFW_WINDOWS)
		win->src.ctx.egl->surface = RGFW_eglCreateWindowSurface(_RGFW->EGL_display, config, (EGLNativeWindowType) win->src.window, surf_attribs);
	#elif defined(RGFW_WAYLAND)
		if (_RGFW->useWaylandBool)
			win->src.ctx.egl->surface = RGFW_eglCreateWindowSurface(_RGFW->EGL_display, config, (EGLNativeWindowType) win->src.ctx.egl->eglWindow, surf_attribs);
		else
    #endif
    #ifdef RGFW_X11
        win->src.ctx.egl->surface = RGFW_eglCreateWindowSurface(_RGFW->EGL_display, config, (EGLNativeWindowType) win->src.window, surf_attribs);
    #else
    {}
    #endif
	#ifdef RGFW_WASM
		win->src.ctx.egl->surface = eglCreateWindowSurface(_RGFW->EGL_display, config, 0, 0);
	#endif

	if (win->src.ctx.egl->surface == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errEGLContext, "Failed to create an EGL surface.");
		return RGFW_FALSE;
	}

	EGLint attribs[20];
	{
		RGFW_attribStack stack;
		RGFW_attribStack_init(&stack, attribs, 20);

		if (hints->major || hints->minor) {
			RGFW_attribStack_pushAttribs(&stack, EGL_CONTEXT_MAJOR_VERSION, hints->major);
			RGFW_attribStack_pushAttribs(&stack, EGL_CONTEXT_MINOR_VERSION, hints->minor);
		}

		if (hints->profile == RGFW_glCore) {
			RGFW_attribStack_pushAttribs(&stack, EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT);
		} else if (hints->profile == RGFW_glCompatibility) {
			RGFW_attribStack_pushAttribs(&stack, EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT);
		}

		RGFW_attribStack_pushAttribs(&stack, EGL_CONTEXT_OPENGL_ROBUST_ACCESS, hints->robustness);
		RGFW_attribStack_pushAttribs(&stack, EGL_CONTEXT_OPENGL_DEBUG, hints->debug);

		#ifndef EGL_CONTEXT_RELEASE_BEHAVIOR_KHR
			#define EGL_CONTEXT_RELEASE_BEHAVIOR_KHR 0x2097
		#endif

		#ifndef EGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR
			#define EGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR 0x2098
		#endif

		if (hints->releaseBehavior == RGFW_glReleaseFlush) {
			RGFW_attribStack_pushAttribs(&stack, EGL_CONTEXT_RELEASE_BEHAVIOR_KHR, EGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR);
		} else {
			RGFW_attribStack_pushAttribs(&stack, EGL_CONTEXT_RELEASE_BEHAVIOR_KHR, 0x0000);
		}

		RGFW_attribStack_pushAttribs(&stack, EGL_NONE, EGL_NONE);
	}

	if (hints->profile == RGFW_glES)
		RGFW_eglBindAPI(EGL_OPENGL_ES_API);
	else
		RGFW_eglBindAPI(EGL_OPENGL_API);

	win->src.ctx.egl->ctx = RGFW_eglCreateContext(_RGFW->EGL_display, config, hints->shareEGL, attribs);

	if (win->src.ctx.egl->ctx == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errEGLContext,  "Failed to create an EGL context.");
		return RGFW_FALSE;
	}

	RGFW_eglMakeCurrent(_RGFW->EGL_display, win->src.ctx.egl->surface, win->src.ctx.egl->surface, win->src.ctx.egl->ctx);
	RGFW_eglSwapBuffers(_RGFW->EGL_display, win->src.ctx.egl->surface);
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, "EGL context initalized.");
	return RGFW_TRUE;
}

RGFW_eglContext* RGFW_window_getContext_EGL(RGFW_window* win) {
	if (win->src.gfxType == RGFW_windowOpenGL) return NULL;
	return win->src.ctx.egl;
}

void RGFW_window_deleteContextPtr_EGL(RGFW_window* win, RGFW_eglContext* ctx) {
	if (_RGFW->EGL_display == NULL) return;

	RGFW_eglDestroySurface(_RGFW->EGL_display, ctx->surface);
	RGFW_eglDestroyContext(_RGFW->EGL_display, ctx->ctx);
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, "EGL context freed");
	#ifdef RGFW_WAYLAND
		if (_RGFW->useWaylandBool == RGFW_FALSE) return;
		wl_egl_window_destroy(win->src.ctx.egl->eglWindow);
		RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, "EGL window context freed");
	#endif
	win->src.ctx.egl = NULL;
}

void RGFW_window_makeCurrentContext_EGL(RGFW_window* win) { if (win) RGFW_ASSERT(win->src.ctx.egl);
	if (win == NULL)
        RGFW_eglMakeCurrent(_RGFW->EGL_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    else {
        RGFW_eglMakeCurrent(_RGFW->EGL_display, win->src.ctx.egl->surface, win->src.ctx.egl->surface, win->src.ctx.egl->ctx);
    }
}

void RGFW_window_swapBuffers_EGL(RGFW_window* win) {
	if (RGFW_eglSwapBuffers)
		RGFW_eglSwapBuffers(_RGFW->EGL_display, win->src.ctx.egl->surface);
	else RGFW_window_swapBuffers_OpenGL(win);
}

void* RGFW_getCurrentContext_EGL(void) {
	return RGFW_eglGetCurrentContext();
}

RGFW_proc RGFW_getProcAddress_EGL(const char* procname) {
	#if defined(RGFW_WINDOWS)
	RGFW_proc proc = (RGFW_proc) GetProcAddress(RGFW_wgl_dll, procname);

		if (proc)
			return proc;
	#endif

	return (RGFW_proc) RGFW_eglGetProcAddress(procname);
}

RGFW_bool RGFW_extensionSupportedPlatform_EGL(const char* extension, size_t len) {
	if (RGFW_loadEGL() == RGFW_FALSE) return RGFW_FALSE;
	const char* extensions = RGFW_eglQueryString(_RGFW->EGL_display, EGL_EXTENSIONS);
	return extensions != NULL && RGFW_extensionSupportedStr(extensions, extension, len);
}

void RGFW_window_swapInterval_EGL(RGFW_window* win, i32 swapInterval) {
	RGFW_ASSERT(win != NULL);
	RGFW_eglSwapInterval(_RGFW->EGL_display, swapInterval);
}

RGFW_bool RGFW_extensionSupported_EGL(const char* extension, size_t len) {
	if (RGFW_extensionSupported_base(extension, len))  return RGFW_TRUE;
    return RGFW_extensionSupportedPlatform_EGL(extension, len);
}

void RGFW_window_makeCurrentWindow_EGL(RGFW_window* win) {
    _RGFW->current = win;
    RGFW_window_makeCurrentContext_EGL(win);
}

RGFW_window* RGFW_getCurrentWindow_EGL(void) { return _RGFW->current; }

RGFW_eglContext* RGFW_window_createContext_EGL(RGFW_window* win, RGFW_glHints* hints) {
	RGFW_eglContext* ctx = (RGFW_eglContext*)RGFW_ALLOC(sizeof(RGFW_eglContext));
	if (RGFW_window_createContextPtr_EGL(win, ctx, hints) == RGFW_FALSE) {
		RGFW_FREE(ctx);
		win->src.ctx.egl = NULL;
		return NULL;
	}
	win->src.gfxType |= RGFW_gfxOwnedByRGFW;
	return ctx;
}

void RGFW_window_deleteContext_EGL(RGFW_window* win, RGFW_eglContext* ctx) {
	RGFW_window_deleteContextPtr_EGL(win, ctx);
	if (win->src.gfxType & RGFW_gfxOwnedByRGFW) RGFW_FREE(ctx);
}

#endif /* RGFW_EGL */

/*
	end of RGFW_EGL defines
*/
#endif /* end of RGFW_GL (OpenGL, EGL, OSMesa )*/

/*
	RGFW_VULKAN defines
*/
#ifdef RGFW_VULKAN
#ifdef RGFW_MACOS
#include <objc/message.h>
#endif

const char** RGFW_getRequiredInstanceExtensions_Vulkan(size_t* count) {
    static const char* arr[2] = {VK_KHR_SURFACE_EXTENSION_NAME};
    arr[1] = RGFW_VK_SURFACE;
    if (count != NULL) *count = 2;

    return (const char**)arr;
}

VkResult RGFW_window_createSurface_Vulkan(RGFW_window* win, VkInstance instance, VkSurfaceKHR* surface) {
    RGFW_ASSERT(win != NULL); RGFW_ASSERT(instance);
	RGFW_ASSERT(surface != NULL);

    *surface = VK_NULL_HANDLE;

#ifdef RGFW_X11

    VkXlibSurfaceCreateInfoKHR x11 = { VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR, 0, 0, (Display*) _RGFW->display, (Window) win->src.window };
    return vkCreateXlibSurfaceKHR(instance, &x11, NULL, surface);
#endif
#if defined(RGFW_WAYLAND)

    VkWaylandSurfaceCreateInfoKHR wayland = { VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR, 0, 0, (struct wl_display*) _RGFW->wl_display, (struct wl_surface*) win->src.surface };
    return vkCreateWaylandSurfaceKHR(instance, &wayland, NULL, surface);
#elif defined(RGFW_WINDOWS)
    VkWin32SurfaceCreateInfoKHR win32 = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, 0, 0, GetModuleHandle(NULL), (HWND)win->src.window };

    return vkCreateWin32SurfaceKHR(instance, &win32, NULL, surface);
#elif defined(RGFW_MACOS) && !defined(RGFW_MACOS_X11)
    void* contentView = ((void* (*)(id, SEL))objc_msgSend)((id)win->src.window, sel_getUid("contentView"));
    VkMacOSSurfaceCreateSurfaceMVK macos = { VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK, 0, 0, 0, (void*)contentView };
    return vkCreateMacOSSurfaceMVK(instance, &macos, NULL, surface);
#endif
}


RGFW_bool RGFW_getPresentationSupport_Vulkan(VkInstance instance, VkPhysicalDevice physicalDevice, u32 queueFamilyIndex) {
    RGFW_ASSERT(instance);
	if (_RGFW == NULL) RGFW_init();
#ifdef RGFW_X11

	Visual* visual = DefaultVisual(_RGFW->display, DefaultScreen(_RGFW->display));
    RGFW_bool out = vkGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, _RGFW->display, XVisualIDFromVisual(visual));
    return out;
#endif
#if defined(RGFW_WAYLAND)

    RGFW_bool wlout = vkGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, _RGFW->wl_display);
    return wlout;
#elif defined(RGFW_WINDOWS)
#elif defined(RGFW_MACOS) && !defined(RGFW_MACOS_X11)
    return RGFW_FALSE; /* TODO */
#endif
}
#endif /* end of RGFW_vulkan */

/*
This is where OS specific stuff starts
*/

/* start of unix (wayland or X11 (unix) ) defines */

#ifdef RGFW_UNIX
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

void RGFW_stopCheckEvents(void) {

	_RGFW->eventWait_forceStop[2] = 1;
	while (1) {
		const char byte = 0;
		const ssize_t result = write(_RGFW->eventWait_forceStop[1], &byte, 1);
		if (result == 1 || result == -1)
			break;
	}
}

RGFWDEF u64 RGFW_linux_getTimeNS(i32 clock);
u64 RGFW_linux_getTimeNS(i32 clock) {
    struct timespec ts;
    const u64 scale_factor = 1000000000;
    clock_gettime(clock, &ts);
    return (u64)ts.tv_sec * scale_factor + (u64)ts.tv_nsec;
}

void RGFW_waitForEvent(i32 waitMS) {
	if (waitMS == 0) return;

	if (_RGFW->eventWait_forceStop[0] == 0 || _RGFW->eventWait_forceStop[1] == 0) {
		if (pipe(_RGFW->eventWait_forceStop) != -1) {
			fcntl(_RGFW->eventWait_forceStop[0], F_GETFL, 0);
			fcntl(_RGFW->eventWait_forceStop[0], F_GETFD, 0);
			fcntl(_RGFW->eventWait_forceStop[1], F_GETFL, 0);
			fcntl(_RGFW->eventWait_forceStop[1], F_GETFD, 0);
		}
	}

	struct pollfd fds[2];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = _RGFW->eventWait_forceStop[0];
    fds[1].events = POLLIN;
    fds[1].revents = 0;


	if (RGFW_usingWayland()) {
		#ifdef RGFW_WAYLAND
		fds[0].fd = wl_display_get_fd(_RGFW->wl_display);

		/* empty the queue */
		while (wl_display_prepare_read(_RGFW->wl_display) != 0) {
			/* error occured when dispatching the queue */
			if (wl_display_dispatch_pending(_RGFW->wl_display) == -1) {
				return;
			}
		}

		/* send any pending requests to the compositor */
		while (wl_display_flush(_RGFW->wl_display) == -1) {

			/* queue is full dispatch them */
			if (errno == EAGAIN) {
				if (wl_display_dispatch_pending(_RGFW->wl_display) == -1) {
					return;
				}
			} else {
				return;
			}
		}
		#endif
	} else {
		#ifdef RGFW_X11
		fds[0].fd = ConnectionNumber(_RGFW->display);
		#endif
	}

	i32 clock = 0;
	#if defined(_POSIX_MONOTONIC_CLOCK)
	struct timespec ts;
	RGFW_MEMSET(&ts, 0, sizeof(struct timespec));

	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
		clock = CLOCK_MONOTONIC;
	#else
		clock = CLOCK_REALTIME;
	#endif

	u64 start = RGFW_linux_getTimeNS(clock);
	if (RGFW_usingWayland()) {
		#ifdef RGFW_WAYLAND
		while (wl_display_dispatch_pending(_RGFW->wl_display) == 0) {
			if (poll(fds, 1, waitMS) <= 0) {
				wl_display_cancel_read(_RGFW->wl_display);
				break;
			} else {
				if (wl_display_read_events(_RGFW->wl_display) == -1)
					return;
			}

			if (waitMS != RGFW_eventWaitNext) {
				waitMS -= (i32)(RGFW_linux_getTimeNS(clock) - start) / (i32)1e+6;
			}
		}

		/* queue contains events from read, dispatch them */
		if (wl_display_dispatch_pending(_RGFW->wl_display) == -1) {
			return;
		}
		#endif
	} else {
		#ifdef RGFW_X11
		while (XPending(_RGFW->display) == 0) {
			if (poll(fds, 1, waitMS) <= 0)
				break;

			if (waitMS != RGFW_eventWaitNext) {
				waitMS -= (i32)(RGFW_linux_getTimeNS(clock) - start) / (i32)1e+6;
			}
		}
		#endif
	}

	/* drain any data in the stop request */
	if (_RGFW->eventWait_forceStop[2]) {
		char data[64];
        RGFW_MEMSET(data, 0, sizeof(data));
        (void)!read(_RGFW->eventWait_forceStop[0], data, sizeof(data));

		_RGFW->eventWait_forceStop[2] = 0;
	}
}

char* RGFW_strtok(char* str, const char* delimStr);
char* RGFW_strtok(char* str, const char* delimStr) {
    static char* static_str = NULL;

    if (str != NULL)
        static_str = str;

    if (static_str == NULL) {
        return NULL;
    }

    while (*static_str != '\0') {
        RGFW_bool delim = 0;
        const char* d;
        for (d = delimStr; *d != '\0'; d++) {
            if (*static_str == *d) {
                delim = 1;
                break;
            }
        }
        if (!delim)
            break;
        static_str++;
    }

    if (*static_str == '\0')
        return NULL;

    char* token_start = static_str;
    while (*static_str != '\0') {
        int delim = 0;
        const char* d;
        for (d = delimStr; *d != '\0'; d++) {
            if (*static_str == *d) {
                delim = 1;
                break;
            }
        }

        if (delim) {
            *static_str = '\0';
            static_str++;
            break;
        }
        static_str++;
    }

    return token_start;
}

#ifdef RGFW_X11
RGFWDEF i32 RGFW_initPlatform_X11(void);
RGFWDEF void RGFW_deinitPlatform_X11(void);
#endif
#ifdef RGFW_WAYLAND
RGFWDEF i32 RGFW_initPlatform_Wayland(void);
RGFWDEF void RGFW_deinitPlatform_Wayland(void);
#endif

RGFWDEF void RGFW_load_X11(void);
RGFWDEF void RGFW_load_Wayland(void);

#if !defined(RGFW_X11) || !defined(RGFW_WAYLAND)
void RGFW_load_X11(void) { }
void RGFW_load_Wayland(void) { }
#endif

/*
 * Sadly we have to use magic linux keycodes
 * We can't use X11 functions, because that breaks Wayland, but they use the same keycodes so there's no use redeffing them
 * We can't use linux enums, because the headers don't exist on BSD
 */
void RGFW_initKeycodesPlatform(void) {
	_RGFW->keycodes[49] = RGFW_backtick;
	_RGFW->keycodes[19] = RGFW_0;
	_RGFW->keycodes[10] = RGFW_1;
	_RGFW->keycodes[11] = RGFW_2;
	_RGFW->keycodes[12] = RGFW_3;
	_RGFW->keycodes[13] = RGFW_4;
	_RGFW->keycodes[14] = RGFW_5;
	_RGFW->keycodes[15] = RGFW_6;
	_RGFW->keycodes[16] = RGFW_7;
	_RGFW->keycodes[17] = RGFW_8;
	_RGFW->keycodes[18] = RGFW_9;
	_RGFW->keycodes[65] = RGFW_space;
	_RGFW->keycodes[38] = RGFW_a;
	_RGFW->keycodes[56] = RGFW_b;
	_RGFW->keycodes[54] = RGFW_c;
	_RGFW->keycodes[40] = RGFW_d;
	_RGFW->keycodes[26] = RGFW_e;
	_RGFW->keycodes[41] = RGFW_f;
	_RGFW->keycodes[42] = RGFW_g;
	_RGFW->keycodes[43] = RGFW_h;
	_RGFW->keycodes[31] = RGFW_i;
	_RGFW->keycodes[44] = RGFW_j;
	_RGFW->keycodes[45] = RGFW_k;
	_RGFW->keycodes[46] = RGFW_l;
	_RGFW->keycodes[58] = RGFW_m;
	_RGFW->keycodes[57] = RGFW_n;
	_RGFW->keycodes[32] = RGFW_o;
	_RGFW->keycodes[33] = RGFW_p;
	_RGFW->keycodes[24] = RGFW_q;
	_RGFW->keycodes[27] = RGFW_r;
	_RGFW->keycodes[39] = RGFW_s;
	_RGFW->keycodes[28] = RGFW_t;
	_RGFW->keycodes[30] = RGFW_u;
	_RGFW->keycodes[55] = RGFW_v;
	_RGFW->keycodes[25] = RGFW_w;
	_RGFW->keycodes[53] = RGFW_x;
	_RGFW->keycodes[29] = RGFW_y;
	_RGFW->keycodes[52] = RGFW_z;
	_RGFW->keycodes[60] = RGFW_period;
	_RGFW->keycodes[59] = RGFW_comma;
	_RGFW->keycodes[61] = RGFW_slash;
	_RGFW->keycodes[34] = RGFW_bracket;
	_RGFW->keycodes[35] = RGFW_closeBracket;
	_RGFW->keycodes[47] = RGFW_semicolon;
	_RGFW->keycodes[48] = RGFW_apostrophe;
	_RGFW->keycodes[51] = RGFW_backSlash;
	_RGFW->keycodes[36] = RGFW_return;
	_RGFW->keycodes[119] = RGFW_delete;
	_RGFW->keycodes[77] = RGFW_numLock;
	_RGFW->keycodes[106] = RGFW_kpSlash;
	_RGFW->keycodes[63] = RGFW_kpMultiply;
	_RGFW->keycodes[86] = RGFW_kpPlus;
	_RGFW->keycodes[82] = RGFW_kpMinus;
	_RGFW->keycodes[87] = RGFW_kp1;
	_RGFW->keycodes[88] = RGFW_kp2;
	_RGFW->keycodes[89] = RGFW_kp3;
	_RGFW->keycodes[83] = RGFW_kp4;
	_RGFW->keycodes[84] = RGFW_kp5;
	_RGFW->keycodes[85] = RGFW_kp6;
	_RGFW->keycodes[81] = RGFW_kp9;
	_RGFW->keycodes[90] = RGFW_kp0;
	_RGFW->keycodes[91] = RGFW_kpPeriod;
	_RGFW->keycodes[104] = RGFW_kpReturn;
	_RGFW->keycodes[20] = RGFW_minus;
	_RGFW->keycodes[21] = RGFW_equals;
	_RGFW->keycodes[22] = RGFW_backSpace;
	_RGFW->keycodes[23] = RGFW_tab;
	_RGFW->keycodes[66] = RGFW_capsLock;
	_RGFW->keycodes[50] = RGFW_shiftL;
	_RGFW->keycodes[37] = RGFW_controlL;
	_RGFW->keycodes[64] = RGFW_altL;
	_RGFW->keycodes[133] = RGFW_superL;
	_RGFW->keycodes[105] = RGFW_controlR;
	_RGFW->keycodes[134] = RGFW_superR;
	_RGFW->keycodes[62] = RGFW_shiftR;
	_RGFW->keycodes[108] = RGFW_altR;
	_RGFW->keycodes[67] = RGFW_F1;
	_RGFW->keycodes[68] = RGFW_F2;
	_RGFW->keycodes[69] = RGFW_F3;
	_RGFW->keycodes[70] = RGFW_F4;
	_RGFW->keycodes[71] = RGFW_F5;
	_RGFW->keycodes[72] = RGFW_F6;
	_RGFW->keycodes[73] = RGFW_F7;
	_RGFW->keycodes[74] = RGFW_F8;
	_RGFW->keycodes[75] = RGFW_F9;
	_RGFW->keycodes[76] = RGFW_F10;
	_RGFW->keycodes[95] = RGFW_F11;
	_RGFW->keycodes[96] = RGFW_F12;
	_RGFW->keycodes[111] = RGFW_up;
	_RGFW->keycodes[116] = RGFW_down;
	_RGFW->keycodes[113] = RGFW_left;
	_RGFW->keycodes[114] = RGFW_right;
	_RGFW->keycodes[118] = RGFW_insert;
	_RGFW->keycodes[115] = RGFW_end;
	_RGFW->keycodes[112] = RGFW_pageUp;
	_RGFW->keycodes[117] = RGFW_pageDown;
	_RGFW->keycodes[9] = RGFW_escape;
	_RGFW->keycodes[110] = RGFW_home;
	_RGFW->keycodes[78] = RGFW_scrollLock;
	_RGFW->keycodes[107] = RGFW_printScreen;
	_RGFW->keycodes[128] = RGFW_pause;
    _RGFW->keycodes[191] = RGFW_F13;
    _RGFW->keycodes[192] = RGFW_F14;
    _RGFW->keycodes[193] = RGFW_F15;
    _RGFW->keycodes[194] = RGFW_F16;
    _RGFW->keycodes[195] = RGFW_F17;
    _RGFW->keycodes[196] = RGFW_F18;
    _RGFW->keycodes[197] = RGFW_F19;
    _RGFW->keycodes[198] = RGFW_F20;
    _RGFW->keycodes[199] = RGFW_F21;
    _RGFW->keycodes[200] = RGFW_F22;
    _RGFW->keycodes[201] = RGFW_F23;
    _RGFW->keycodes[202] = RGFW_F24;
    _RGFW->keycodes[203] = RGFW_F25;
	_RGFW->keycodes[142] = RGFW_kpEqual;
	_RGFW->keycodes[161] = RGFW_world1; /* non-US key #1 */
    _RGFW->keycodes[162] = RGFW_world2; /* non-US key #2 */
}

i32 RGFW_initPlatform(void) {
#ifdef RGFW_WAYLAND
	RGFW_load_Wayland();
	i32 ret = RGFW_initPlatform_Wayland();
	if (ret == 0) {
		return 0;
	} else {
		#ifdef RGFW_X11
			RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningWayland,  "Falling back to X11");
			RGFW_useWayland(0);
		#else
			return ret;
		#endif
	}
#endif
#ifdef RGFW_X11
	RGFW_load_X11();
	return RGFW_initPlatform_X11();
#else
	return 0;
#endif
}


void RGFW_deinitPlatform(void) {
    if (_RGFW->eventWait_forceStop[0] || _RGFW->eventWait_forceStop[1]){
        close(_RGFW->eventWait_forceStop[0]);
        close(_RGFW->eventWait_forceStop[1]);
    }
#ifdef RGFW_WAYLAND
	if (_RGFW->useWaylandBool) {
		RGFW_deinitPlatform_Wayland();
		return;
	}
#endif
#ifdef RGFW_X11
	RGFW_deinitPlatform_X11();
#endif
}

#endif /* end of wayland or X11 defines */


/*


Start of Linux / Unix defines


*/

#ifdef RGFW_X11
#ifdef RGFW_WAYLAND
#define RGFW_FUNC(func) func##_X11
#else
#define RGFW_FUNC(func) func
#endif

#include <dlfcn.h>
#include <unistd.h>

#include <limits.h> /* for data limits (mainly used in drag and drop functions) */
#include <poll.h>

void RGFW_setXInstName(const char* name) { _RGFW->instName = name; }
#if !defined(RGFW_NO_X11_CURSOR) && defined(RGFW_X11)
	#include <X11/Xcursor/Xcursor.h>
#endif

#ifndef RGFW_NO_DPI
	#include <X11/extensions/Xrandr.h>
	#include <X11/Xresource.h>
#endif

#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <X11/extensions/sync.h>

#include <X11/XKBlib.h> /* for converting keycode to string */
#include <X11/cursorfont.h> /* for hiding */
#include <X11/extensions/shapeconst.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XInput2.h>

#ifdef RGFW_OPENGL
	#ifndef __gl_h_
		#define __gl_h_
		#define RGFW_gl_ndef
		#define GLubyte     unsigned char
		#define GLenum      unsigned int
		#define GLint       int
		#define GLuint      unsigned int
		#define GLsizei     int
		#define GLfloat     float
		#define GLvoid      void
		#define GLbitfield  unsigned int
		#define GLintptr    ptrdiff_t
		#define GLsizeiptr  ptrdiff_t
		#define GLboolean   unsigned char
	#endif

	#include <GL/glx.h> /* GLX defs, xlib.h, gl.h */
	#ifndef GLX_MESA_swap_control
		#define  GLX_MESA_swap_control
	#endif

	#ifdef RGFW_gl_ndef
		#undef __gl_h_
		#undef GLubyte
		#undef GLenum
		#undef GLint
		#undef GLuint
		#undef GLsizei
		#undef GLfloat
		#undef GLvoid
		#undef GLbitfield
		#undef GLintptr
		#undef GLsizeiptr
		#undef GLboolean
	#endif
	typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
#endif

/* atoms needed for drag and drop */
#if !defined(RGFW_NO_X11_CURSOR) && !defined(RGFW_NO_X11_CURSOR_PRELOAD)
		typedef XcursorImage* (*PFN_XcursorImageCreate)(int, int);
		typedef void (*PFN_XcursorImageDestroy)(XcursorImage*);
		typedef Cursor(*PFN_XcursorImageLoadCursor)(Display*, const XcursorImage*);
#endif

#if !defined(RGFW_NO_X11_XI_PRELOAD)
		typedef int (* PFN_XISelectEvents)(Display*,Window,XIEventMask*,int);
		PFN_XISelectEvents XISelectEventsSRC = NULL;
		#define XISelectEvents XISelectEventsSRC

		void* X11Xihandle = NULL;
#endif

#if !defined(RGFW_NO_X11_EXT_PRELOAD)
		typedef void (* PFN_XSyncIntToValue)(XSyncValue*, int);
		PFN_XSyncIntToValue XSyncIntToValueSRC = NULL;
		#define XSyncIntToValue XSyncIntToValueSRC

		typedef Status (* PFN_XSyncSetCounter)(Display*, XSyncCounter, XSyncValue);
		PFN_XSyncSetCounter XSyncSetCounterSRC = NULL;
		#define XSyncSetCounter XSyncSetCounterSRC

		typedef XSyncCounter (* PFN_XSyncCreateCounter)(Display*, XSyncValue);
		PFN_XSyncCreateCounter XSyncCreateCounterSRC = NULL;
		#define XSyncCreateCounter XSyncCreateCounterSRC

		typedef void (* PFN_XShapeCombineMask)(Display*,Window,int,int,int,Pixmap,int);
		PFN_XShapeCombineMask XShapeCombineMaskSRC;
		#define XShapeCombineMask XShapeCombineMaskSRC

		typedef void (* PFN_XShapeCombineRegion)(Display*,Window,int,int,int,Region,int);
		PFN_XShapeCombineRegion XShapeCombineRegionSRC;
		#define XShapeCombineRegion XShapeCombineRegionSRC
		void* X11XEXThandle = NULL;
#endif

#if !defined(RGFW_NO_X11_CURSOR) && !defined(RGFW_NO_X11_CURSOR_PRELOAD)
		PFN_XcursorImageLoadCursor XcursorImageLoadCursorSRC = NULL;
		PFN_XcursorImageCreate XcursorImageCreateSRC = NULL;
		PFN_XcursorImageDestroy XcursorImageDestroySRC = NULL;

		#define XcursorImageLoadCursor XcursorImageLoadCursorSRC
		#define XcursorImageCreate XcursorImageCreateSRC
		#define XcursorImageDestroy XcursorImageDestroySRC

		void* X11Cursorhandle = NULL;
#endif

void* RGFW_getDisplay_X11(void) { return _RGFW->display; }
u64 RGFW_window_getWindow_X11(RGFW_window* win) { return (u64)win->src.window; }

RGFWDEF RGFW_format RGFW_XImage_getFormat(XImage* image);
RGFW_format RGFW_XImage_getFormat(XImage* image) {
    switch (image->bits_per_pixel) {
        case 24:
            if (image->red_mask == 0xFF0000 && image->green_mask == 0x00FF00 && image->blue_mask == 0x0000FF)
                return RGFW_formatRGB8;
            if (image->red_mask == 0x0000FF && image->green_mask == 0x00FF00 && image->blue_mask == 0xFF0000)
                return RGFW_formatBGR8;
            break;
        case 32:
            if (image->red_mask == 0x00FF0000 && image->green_mask == 0x0000FF00 && image->blue_mask == 0x000000FF)
                return RGFW_formatBGRA8;
            if (image->red_mask == 0x000000FF && image->green_mask == 0x0000FF00 && image->blue_mask == 0x00FF0000)
                return RGFW_formatRGBA8;
            if (image->red_mask == 0x0000FF00 && image->green_mask == 0x00FF0000 && image->blue_mask == 0xFF000000)
                return RGFW_formatABGR8;
            if (image->red_mask == 0x00FF0000 && image->green_mask == 0x0000FF00 && image->blue_mask == 0x000000FF)
                return RGFW_formatARGB8;  /* ambiguous without alpha */
            break;
    }
	return RGFW_formatARGB8;
}

RGFW_bool RGFW_window_createSurfacePtr(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format, RGFW_surface* surface) {
	RGFW_ASSERT(surface != NULL);
	surface->data = data;
	surface->w = w;
	surface->h = h;
	surface->format = format;

	XWindowAttributes attrs;
	if (XGetWindowAttributes(_RGFW->display, win->src.window, &attrs) == 0) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errBuffer, "Failed to get window attributes.");
		return RGFW_FALSE;
	}

	surface->native.bitmap = XCreateImage(_RGFW->display,  attrs.visual,  (u32)attrs.depth,
										ZPixmap, 0, NULL, (u32)surface->w, (u32)surface->h, 32, 0);

	surface->native.buffer = (u8*)RGFW_ALLOC((size_t)(w * h * 4));
	surface->native.format = RGFW_XImage_getFormat(surface->native.bitmap);

	if (surface->native.bitmap == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errBuffer,  "Failed to create XImage.");
		return RGFW_FALSE;
	}

	surface->native.format = RGFW_formatBGRA8;
	return RGFW_TRUE;
}

RGFW_bool RGFW_FUNC(RGFW_createSurfacePtr) (u8* data, i32 w, i32 h, RGFW_format format, RGFW_surface* surface) {
	return RGFW_window_createSurfacePtr(_RGFW->root, data, w, h, format, surface);
}

void RGFW_FUNC(RGFW_window_blitSurface) (RGFW_window* win, RGFW_surface* surface) {
	RGFW_ASSERT(surface != NULL);
	surface->native.bitmap->data = (char*)surface->native.buffer;
	RGFW_copyImageData((u8*)surface->native.buffer, surface->w, RGFW_MIN(win->h, surface->h), surface->native.format, surface->data, surface->format);

	XPutImage(_RGFW->display, win->src.window, win->src.gc, surface->native.bitmap, 0, 0, 0, 0, (u32)RGFW_MIN(win->w, surface->w), (u32)RGFW_MIN(win->h, surface->h));
	surface->native.bitmap->data = NULL;
	return;
}

void RGFW_FUNC(RGFW_surface_freePtr) (RGFW_surface* surface) {
	RGFW_ASSERT(surface != NULL);
	RGFW_FREE(surface->native.buffer);
	XDestroyImage(surface->native.bitmap);
	return;
}

#define RGFW_LOAD_ATOM(name) \
	static Atom name = 0; \
	if (name == 0) name = XInternAtom(_RGFW->display, #name, False);

void RGFW_FUNC(RGFW_window_setBorder) (RGFW_window* win, RGFW_bool border) {
	RGFW_setBit(&win->internal.flags, RGFW_windowNoBorder, !border);
	RGFW_LOAD_ATOM(_MOTIF_WM_HINTS);

	struct __x11WindowHints {
		unsigned long flags, functions, decorations, status;
		long input_mode;
	} hints;
	hints.flags = 2;
	hints.decorations = border;

	XChangeProperty(_RGFW->display, win->src.window, _MOTIF_WM_HINTS, _MOTIF_WM_HINTS, 32, PropModeReplace, (u8*)&hints, 5);

	if (RGFW_window_isHidden(win) == 0) {
		RGFW_window_hide(win);
		RGFW_window_show(win);
	}
}

void RGFW_FUNC(RGFW_releaseCursor) (RGFW_window* win) {
	RGFW_UNUSED(win);
	XUngrabPointer(_RGFW->display, CurrentTime);

	/* disable raw input */
	unsigned char mask[] = { 0 };
	XIEventMask em;
	em.deviceid = XIAllMasterDevices;
	em.mask_len = sizeof(mask);
	em.mask = mask;

	XISelectEvents(_RGFW->display, XDefaultRootWindow(_RGFW->display), &em, 1);
}

void RGFW_FUNC(RGFW_captureCursor) (RGFW_window* win) {
	/* enable raw input */
	unsigned char mask[XIMaskLen(XI_RawMotion)] = { 0 };
	XISetMask(mask, XI_RawMotion);

	XIEventMask em;
	em.deviceid = XIAllMasterDevices;
	em.mask_len = sizeof(mask);
	em.mask = mask;

	XISelectEvents(_RGFW->display, XDefaultRootWindow(_RGFW->display), &em, 1);

	unsigned int event_mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	XGrabPointer(_RGFW->display, win->src.window, False, event_mask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	RGFW_window_moveMouse(win, win->x + (i32)(win->w / 2), win->y + (i32)(win->h / 2));
}

#define RGFW_LOAD_LIBRARY(x, lib) if (x == NULL) x = dlopen(lib, RTLD_LAZY | RTLD_LOCAL)
#define RGFW_PROC_DEF(proc, name) if (name##SRC == NULL && proc != NULL) { \
	void* ptr = dlsym(proc, #name); \
	if (ptr != NULL) RGFW_MEMCPY(&name##SRC, &ptr, sizeof(PFN_##name)); \
}

RGFWDEF void RGFW_window_getVisual(XVisualInfo* visual, RGFW_bool transparent);
void RGFW_window_getVisual(XVisualInfo* visual, RGFW_bool transparent) {
	visual->visual = DefaultVisual(_RGFW->display, DefaultScreen(_RGFW->display));
	visual->depth = DefaultDepth(_RGFW->display, DefaultScreen(_RGFW->display));
	if (transparent) {
		XMatchVisualInfo(_RGFW->display, DefaultScreen(_RGFW->display), 32, TrueColor, visual); /*!< for RGBA backgrounds */
		if (visual->depth != 32)
			RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningOpenGL, "Failed to load a 32-bit depth.");
	}
}

RGFWDEF int RGFW_XErrorHandler(Display* display, XErrorEvent* ev);
int RGFW_XErrorHandler(Display* display, XErrorEvent* ev) {
    char errorText[512];
    XGetErrorText(display, ev->error_code, errorText, sizeof(errorText));

    char buf[1024];
    RGFW_SNPRINTF(buf, sizeof(buf),  "[X Error] %s\n  Error code: %d\n  Request code: %d\n  Minor code: %d\n  Serial: %lu\n",
             errorText,
             ev->error_code, ev->request_code, ev->minor_code, ev->serial);

    RGFW_sendDebugInfo(RGFW_typeError, RGFW_errX11, buf);
    _RGFW->x11Error = ev;
    return 0;
}

void RGFW_XCreateWindow (XVisualInfo visual, const char* name, RGFW_windowFlags flags, RGFW_window* win) {
	i64 event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask |
						LeaveWindowMask | EnterWindowMask | ExposureMask | VisibilityChangeMask | PropertyChangeMask;

	/* make X window attrubutes */
	XSetWindowAttributes swa;
    RGFW_MEMSET(&swa, 0, sizeof(swa));

	win->src.parent = DefaultRootWindow(_RGFW->display);

	Colormap cmap;
	swa.colormap = cmap = XCreateColormap(_RGFW->display,
		win->src.parent,
		visual.visual, AllocNone);
	swa.event_mask = event_mask;
    swa.background_pixmap = None;

	/* create the window */
    win->src.window = XCreateWindow(_RGFW->display, win->src.parent, win->x, win->y, (u32)win->w, (u32)win->h,
		0, visual.depth, InputOutput, visual.visual,
		CWBorderPixel | CWColormap | CWEventMask, &swa);

	XFreeColors(_RGFW->display, cmap, NULL, 0, 0);

	XSaveContext(_RGFW->display, win->src.window, _RGFW->context, (XPointer)win);

	win->src.gc = XCreateGC(_RGFW->display, win->src.window, 0, NULL);

	/* In your .desktop app, if you set the property
	    StartupWMClass=RGFW that will assoicate the launcher icon
	     with your application - robrohan */
	if (_RGFW->className == NULL)
		_RGFW->className = (char*)name;

	XClassHint hint;
	hint.res_class = (char*)_RGFW->className;
	if (_RGFW->instName == NULL)	hint.res_name = (char*)name;
	else 						hint.res_name = (char*)_RGFW->instName;
	XSetClassHint(_RGFW->display, win->src.window, &hint);

	#ifndef RGFW_NO_MONITOR
	if (flags & RGFW_windowScaleToMonitor)
		RGFW_window_scaleToMonitor(win);
	#endif
	XSelectInput(_RGFW->display, (Drawable) win->src.window, event_mask); /*!< tell X11 what events we want */

	/* make it so the user can't close the window until the program does */
    RGFW_LOAD_ATOM(WM_DELETE_WINDOW);
	XSetWMProtocols(_RGFW->display, (Drawable) win->src.window, &WM_DELETE_WINDOW, 1);
	/* set the background */
	RGFW_window_setName(win, name);

	XMoveWindow(_RGFW->display, (Drawable) win->src.window, win->x, win->y); /*!< move the window to it's proper cords */

	if (flags & RGFW_windowAllowDND) { /* init drag and drop atoms and turn on drag and drop for this window */
		win->internal.flags |= RGFW_windowAllowDND;

		/* actions */
		Atom XdndAware = XInternAtom(_RGFW->display, "XdndAware", False);
		const u8 version = 5;

		XChangeProperty(_RGFW->display, win->src.window,
			XdndAware, 4, 32,
			PropModeReplace, &version, 1); /*!< turns on drag and drop */
	}

#ifdef RGFW_ADVANCED_SMOOTH_RESIZE
    RGFW_LOAD_ATOM(_NET_WM_SYNC_REQUEST_COUNTER)
    RGFW_LOAD_ATOM(_NET_WM_SYNC_REQUEST)

    Atom protcols[2] = {_NET_WM_SYNC_REQUEST, WM_DELETE_WINDOW};
    XSetWMProtocols(_RGFW->display, win->src.window, protcols, 2);

    XSyncValue initial_value;
    XSyncIntToValue(&initial_value, 0);
    win->src.counter = XSyncCreateCounter(_RGFW->display, initial_value);

    XChangeProperty(_RGFW->display, win->src.window, _NET_WM_SYNC_REQUEST_COUNTER, XA_CARDINAL, 32, PropModeReplace, (uint8_t*)&win->src.counter, 1);
#endif

	win->src.x = win->x;
	win->src.y = win->y;
	win->src.w = win->w;
	win->src.h = win->h;

	XSetWindowBackground(_RGFW->display, win->src.window, None);
    XClearWindow(_RGFW->display, win->src.window);

	/* stupid hack to make resizing the window less bad */
	XSetWindowBackgroundPixmap(_RGFW->display, win->src.window, None);
}

RGFW_window* RGFW_FUNC(RGFW_createWindowPlatform) (const char* name, RGFW_windowFlags flags, RGFW_window* win) {
	if ((flags & RGFW_windowOpenGL) || (flags & RGFW_windowEGL)) {
		win->src.window = 0;
		return win;
	}

	XVisualInfo visual;
	RGFW_window_getVisual(&visual, RGFW_BOOL(win->internal.flags & RGFW_windowTransparent));
	RGFW_XCreateWindow(visual, name, flags, win);
	return win; /*return newly created window */
}

RGFW_bool RGFW_FUNC(RGFW_getGlobalMouse) (i32* fX, i32* fY) {
	RGFW_init();
	i32 x, y;
	u32 z;
	Window window1, window2;
	XQueryPointer(_RGFW->display, XDefaultRootWindow(_RGFW->display), &window1, &window2, fX, fY, &x, &y, &z);
	return RGFW_TRUE;
}

RGFWDEF void RGFW_XHandleClipboardSelection(XEvent* event);
void RGFW_XHandleClipboardSelection(XEvent* event) { RGFW_UNUSED(event);
    RGFW_LOAD_ATOM(ATOM_PAIR);
	RGFW_LOAD_ATOM(MULTIPLE);
	RGFW_LOAD_ATOM(TARGETS);
	RGFW_LOAD_ATOM(SAVE_TARGETS);
	RGFW_LOAD_ATOM(UTF8_STRING);

    const XSelectionRequestEvent* request = &event->xselectionrequest;
    Atom formats[2] = {0};
    formats[0] = UTF8_STRING;
    formats[1] = XA_STRING;
    const int formatCount = sizeof(formats) / sizeof(formats[0]);

    if (request->target == TARGETS) {
        Atom targets[4] = {0};
        targets[0] = TARGETS;
        targets[1] = MULTIPLE;
        targets[2] = UTF8_STRING;
        targets[3] = XA_STRING;

        XChangeProperty(_RGFW->display, request->requestor, request->property,
                        XA_ATOM, 32, PropModeReplace, (u8*) targets, sizeof(targets) / sizeof(Atom));
    }  else if (request->target == MULTIPLE) {
		Atom* targets = NULL;

		Atom actualType = 0;
		int actualFormat = 0;
		unsigned long count = 0, bytesAfter = 0;

		XGetWindowProperty(_RGFW->display, request->requestor, request->property, 0, LONG_MAX,
							False, ATOM_PAIR, &actualType, &actualFormat, &count, &bytesAfter, (u8**) &targets);

		unsigned long i;
		for (i = 0; i < (u32)count; i += 2) {
			if (targets[i] == UTF8_STRING || targets[i] == XA_STRING)
				XChangeProperty(_RGFW->display, request->requestor, targets[i + 1], targets[i],
					8, PropModeReplace, (const unsigned char *)_RGFW->clipboard, (i32)_RGFW->clipboard_len);
			else
				targets[i + 1] = None;
		}

		XChangeProperty(_RGFW->display,
			request->requestor, request->property, ATOM_PAIR, 32,
			PropModeReplace, (u8*) targets, (i32)count);

		XFlush(_RGFW->display);
		XFree(targets);
	} else if (request->target == SAVE_TARGETS)
        XChangeProperty(_RGFW->display, request->requestor, request->property, 0, 32, PropModeReplace, NULL, 0);
    else {
        int i;
        for (i = 0;  i < formatCount;  i++) {
			if (request->target != formats[i])
				continue;
			XChangeProperty(_RGFW->display, request->requestor, request->property, request->target,
								8, PropModeReplace, (u8*) _RGFW->clipboard, (i32)_RGFW->clipboard_len);
		}
	}

    XEvent reply = { SelectionNotify };
    reply.xselection.property = request->property;
    reply.xselection.display = request->display;
    reply.xselection.requestor = request->requestor;
    reply.xselection.selection = request->selection;
    reply.xselection.target = request->target;
    reply.xselection.time = request->time;

    XSendEvent(_RGFW->display, request->requestor, False, 0, &reply);
	XFlush(_RGFW->display);
}

i32 RGFW_XHandleClipboardSelectionHelper(void);

u8 RGFW_FUNC(RGFW_rgfwToKeyChar) (u32 key) {
    u32 keycode = RGFW_rgfwToApiKey(key);

    Window root = DefaultRootWindow(_RGFW->display);
    Window ret_root, ret_child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    XQueryPointer(_RGFW->display, root, &ret_root, &ret_child, &root_x, &root_y, &win_x, &win_y, &mask);
    KeySym sym = (KeySym)XkbKeycodeToKeysym(_RGFW->display, (KeyCode)keycode, 0, (KeyCode)mask & ShiftMask ? 1 : 0);

    if ((mask & LockMask) && sym >= XK_a && sym <= XK_z)
        sym = (mask & ShiftMask) ? sym + 32 : sym - 32;
    if ((u8)sym != (u32)sym)
        sym = 0;

    return (u8)sym;
}

RGFWDEF void RGFW_XHandleEvent(void);
void RGFW_XHandleEvent(void) {
	RGFW_LOAD_ATOM(XdndTypeList);
	RGFW_LOAD_ATOM(XdndSelection);
	RGFW_LOAD_ATOM(XdndEnter);
	RGFW_LOAD_ATOM(XdndPosition);
	RGFW_LOAD_ATOM(XdndStatus);
	RGFW_LOAD_ATOM(XdndLeave);
	RGFW_LOAD_ATOM(XdndDrop);
	RGFW_LOAD_ATOM(XdndFinished);
	RGFW_LOAD_ATOM(XdndActionCopy);
    RGFW_LOAD_ATOM(_NET_WM_SYNC_REQUEST);
    RGFW_LOAD_ATOM(WM_PROTOCOLS);
	RGFW_LOAD_ATOM(WM_STATE);
	RGFW_LOAD_ATOM(_NET_WM_STATE);

	/* xdnd data */
	static Window source = 0;
	static long version = 0;
	static i32 format = 0;

	XEvent reply = { ClientMessage };
	XEvent E;
	RGFW_event event;
	RGFW_MEMSET(&event, 0, sizeof(event));

	XNextEvent(_RGFW->display, &E);
	switch (E.type) {
		case SelectionRequest:
			RGFW_XHandleClipboardSelection(&E);
			return;
		case GenericEvent: {
			RGFW_window* win = _RGFW->mouseOwner;
			if (win == NULL) return;
			if (!(win->internal.enabledEvents & RGFW_BIT(RGFW_mousePosChanged))) return;

			/* MotionNotify is used for mouse events if the mouse isn't held */
			if (!(win->internal.holdMouse)) {
				XFreeEventData(_RGFW->display, &E.xcookie);
				return;
			}

			XGetEventData(_RGFW->display, &E.xcookie);
			if (E.xcookie.evtype == XI_RawMotion) {
				XIRawEvent *raw = (XIRawEvent *)E.xcookie.data;
				if (raw->valuators.mask_len == 0) {
					XFreeEventData(_RGFW->display, &E.xcookie);
					return;
				}

				double deltaX = 0.0f;
				double deltaY = 0.0f;

				/* check if relative motion data exists where we think it does */
				if (XIMaskIsSet(raw->valuators.mask, 0) != 0)
					deltaX += raw->raw_values[0];
				if (XIMaskIsSet(raw->valuators.mask, 1) != 0)
					deltaY += raw->raw_values[1];

				event.mouse.vecX = (float)deltaX;
				event.mouse.vecY = (float)deltaY;
				_RGFW->vectorX = (float)event.mouse.vecX;
				_RGFW->vectorY = (float)event.mouse.vecY;
				event.mouse.x = win->internal.lastMouseX + (i32)event.mouse.vecX;
				event.mouse.y = win->internal.lastMouseY + (i32)event.mouse.vecY;
				win->internal.lastMouseX = event.mouse.x;
				win->internal.lastMouseY = event.mouse.y;
				RGFW_window_moveMouse(win, win->x + (win->w / 2), win->y + (win->h / 2));

				event.type = RGFW_mousePosChanged;
				RGFW_mousePosCallback(win, event.mouse.x, event.mouse.y, (float)event.mouse.vecX, (float)event.mouse.vecY);
			}

			XFreeEventData(_RGFW->display, &E.xcookie);
			if (event.type)
				RGFW_eventQueuePush(&event);
			return;
		}
	}

	RGFW_window* win = NULL;
	if (XFindContext(_RGFW->display, E.xany.window, _RGFW->context, (XPointer*) &win) != 0) {
		return;
	}

	event.common.win = win;

	/*
		Repeated key presses are sent as a release followed by another press at the same time.
		We want to convert that into a single key press event with the repeat flag set
	*/
	if (E.type == KeyRelease && XEventsQueued(_RGFW->display, QueuedAfterReading)) {
		XEvent NE;
		XPeekEvent(_RGFW->display, &NE);
		if (NE.type == KeyPress && E.xkey.time == NE.xkey.time && E.xkey.keycode == NE.xkey.keycode) {
			/* Use the next KeyPress event */
			XNextEvent(_RGFW->display, &E);
			event.key.repeat = RGFW_TRUE;
		}
	}

	switch (E.type) {
		case KeyPress: {
			if (!(win->internal.enabledEvents & RGFW_keyPressedFlag)) return;
			event.type = RGFW_keyPressed;
			event.key.value = (u8)RGFW_apiKeyToRGFW(E.xkey.keycode);
			event.key.sym = (u8)RGFW_rgfwToKeyChar(event.key.value);

			_RGFW->keyboard[event.key.value].prev = _RGFW->keyboard[event.key.value].current;
			_RGFW->keyboard[event.key.value].current = RGFW_TRUE;

			XkbStateRec state;
			XkbGetState(_RGFW->display, XkbUseCoreKbd, &state);
			RGFW_updateKeyMods(win, (state.locked_mods & LockMask), (state.locked_mods & Mod2Mask), (state.locked_mods & Mod3Mask));

			RGFW_keyCallback(win, event.key.value, event.key.sym, win->internal.mod, event.key.repeat, RGFW_TRUE);
			break;
		}
		case KeyRelease: {
			if (!(win->internal.enabledEvents & RGFW_keyReleasedFlag)) return;

			event.type =  RGFW_keyReleased;
			event.key.value = (u8)RGFW_apiKeyToRGFW(E.xkey.keycode);
			event.key.sym = (u8)RGFW_rgfwToKeyChar(event.key.value);

			/* get keystate data */
			_RGFW->keyboard[event.key.value].prev = _RGFW->keyboard[event.key.value].current;
			_RGFW->keyboard[event.key.value].current = RGFW_FALSE;

			XkbStateRec state;
			XkbGetState(_RGFW->display, XkbUseCoreKbd, &state);
			RGFW_updateKeyMods(win, (state.locked_mods & LockMask), (state.locked_mods & Mod2Mask), (state.locked_mods & Mod3Mask));

			RGFW_keyCallback(win, event.key.value, event.key.sym, win->internal.mod, event.key.repeat, RGFW_FALSE);
			break;
		}
		case ButtonPress:
			if (E.xbutton.button >= Button4 && E.xbutton.button <= 7) {
				if (!(win->internal.enabledEvents & RGFW_mouseScrollFlag)) return;
				event.type = RGFW_mouseScroll;
			} else {
				if (!(win->internal.enabledEvents & RGFW_mouseButtonPressedFlag) || E.xbutton.button > RGFW_mouseFinal) return;
				event.type = RGFW_mouseButtonPressed;
			}

			switch(E.xbutton.button) {
				case Button1: event.button.value = RGFW_mouseLeft; break;
				case Button2: event.button.value = RGFW_mouseMiddle; break;
				case Button3: event.button.value = RGFW_mouseRight; break;
				case Button4: event.scroll.y = 1.0; break;
				case Button5: event.scroll.y = -1.0; break;
				case 6: event.scroll.x = 1.0f; break;
				case 7: event.scroll.x = -1.0f; break;
				default:
					event.button.value = (u8)E.xbutton.button - Button1 - 4;
					break;
			}

			if (event.type == RGFW_mouseScroll) {
				_RGFW->scrollX = event.scroll.x;
				_RGFW->scrollY = event.scroll.y;
				RGFW_mouseScrollCallback(win, event.scroll.x, event.scroll.y);
				break;
			}

			_RGFW->mouseButtons[event.button.value].prev = _RGFW->mouseButtons[event.button.value].current;
			_RGFW->mouseButtons[event.button.value].current = RGFW_TRUE;
			RGFW_mouseButtonCallback(win, event.button.value, RGFW_TRUE);
			break;
		case ButtonRelease:
			if (E.xbutton.button >= Button4 && E.xbutton.button <= 7) break;
			if (!(win->internal.enabledEvents & RGFW_mouseButtonReleasedFlag) || E.xbutton.button > RGFW_mouseFinal) return;
			event.type = RGFW_mouseButtonReleased;
			switch(E.xbutton.button) {
				case Button1: event.button.value = RGFW_mouseLeft; break;
				case Button2: event.button.value = RGFW_mouseMiddle; break;
				case Button3: event.button.value = RGFW_mouseRight; break;
				default:
					event.button.value = (u8)E.xbutton.button - Button1 - 4;
					break;
			}

			_RGFW->mouseButtons[event.button.value].prev = _RGFW->mouseButtons[event.button.value].current;
			_RGFW->mouseButtons[event.button.value].current = RGFW_FALSE;
			RGFW_mouseButtonCallback(win, event.button.value, RGFW_FALSE);
			break;
		case MotionNotify:
			if (win->internal.holdMouse) return;
			if (!(win->internal.enabledEvents & RGFW_mousePosChangedFlag)) return;
			event.mouse.x = E.xmotion.x;
			event.mouse.y = E.xmotion.y;

			event.mouse.vecX = (float)(event.mouse.x - win->internal.lastMouseX);
			event.mouse.vecY = (float)(event.mouse.y - win->internal.lastMouseY);
			_RGFW->vectorX = event.mouse.vecX;
			_RGFW->vectorY = event.mouse.vecY;
			win->internal.lastMouseX = event.mouse.x;
			win->internal.lastMouseY = event.mouse.y;
			event.type = RGFW_mousePosChanged;
			RGFW_mousePosCallback(win, event.mouse.x, event.mouse.y, (float)event.mouse.vecX, (float)event.mouse.vecY);
			break;

		case Expose: {
			if (!(win->internal.enabledEvents & RGFW_windowRefreshFlag)) return;
			event.type = RGFW_windowRefresh;
			RGFW_windowRefreshCallback(win);

#ifdef RGFW_ADVANCED_SMOOTH_RESIZE
			XSyncValue value;
			XSyncIntToValue(&value, (i32)win->src.counter_value);
			XSyncSetCounter(_RGFW->display, win->src.counter, value);
#endif
			break;
		}

		case PropertyNotify:
			if (E.xproperty.state != PropertyNewValue) break;

			if (E.xproperty.atom == WM_STATE) {
                if (RGFW_window_isMinimized(win) && !(win->internal.flags & RGFW_windowMinimized)) {
					win->internal.flags |= RGFW_windowMinimize;
					RGFW_eventQueuePushEx(e.type = RGFW_windowMinimized; e.common.win = win);
					RGFW_windowMinimizedCallback(win);
					break;
				}
			} else if (E.xproperty.atom == _NET_WM_STATE) {
				if (!(win->internal.flags & RGFW_windowMaximize)) {
					win->internal.flags |= RGFW_windowMaximize;
					RGFW_eventQueuePushEx(e.type = RGFW_windowMaximized; e.common.win = win);
					RGFW_windowMaximizedCallback(win, win->x, win->y, win->w, win->h);
					break;
				}
			}

			RGFW_window_checkMode(win);
			break;
		case MapNotify: case UnmapNotify: 		RGFW_window_checkMode(win); break;
		case ClientMessage: {
			RGFW_LOAD_ATOM(WM_DELETE_WINDOW);
			/* if the client closed the window */
			if (E.xclient.data.l[0] == (long)WM_DELETE_WINDOW) {
				event.type = RGFW_quit;
				RGFW_window_setShouldClose(win, RGFW_TRUE);
				RGFW_windowQuitCallback(win);
				break;
			}
#ifdef RGFW_ADVANCED_SMOOTH_RESIZE
			if (E.xclient.message_type == WM_PROTOCOLS && (Atom)E.xclient.data.l[0] == _NET_WM_SYNC_REQUEST) {
				RGFW_windowRefreshCallback(win);
				win->src.counter_value = 0;
				win->src.counter_value |= E.xclient.data.l[2];
				win->src.counter_value |= (E.xclient.data.l[3] << 32);

				XSyncValue value;
				XSyncIntToValue(&value, (i32)win->src.counter_value);
				XSyncSetCounter(_RGFW->display, win->src.counter, value);
				break;
			}
#endif
			if ((win->internal.flags & RGFW_windowAllowDND) == 0)
				return;

			reply.xclient.window = source;
			reply.xclient.format = 32;
			reply.xclient.data.l[0] = (long)win->src.window;
			reply.xclient.data.l[1] = 0;
			reply.xclient.data.l[2] = None;

			if (E.xclient.message_type == XdndEnter) {
				if (version > 5)
					break;

				unsigned long count;
				Atom* formats;
				Atom real_formats[6];
				Bool list = E.xclient.data.l[1] & 1;

				source = (unsigned long int)E.xclient.data.l[0];
				version = E.xclient.data.l[1] >> 24;
				format = None;
				if (list) {
					Atom actualType;
					i32 actualFormat;
					unsigned long bytesAfter;

					XGetWindowProperty(
						_RGFW->display, source, XdndTypeList,
						0, LONG_MAX, False, 4,
						&actualType, &actualFormat, &count, &bytesAfter, (u8**)&formats
					);
				} else {
					count = 0;

					size_t i;
					for (i = 2; i < 5; i++) {
						if (E.xclient.data.l[i] != None) {
							real_formats[count] = (unsigned long int)E.xclient.data.l[i];
							count += 1;
						}
					}

					formats = real_formats;
				}

				Atom XtextPlain = XInternAtom(_RGFW->display, "text/plain", False);
				Atom XtextUriList = XInternAtom(_RGFW->display, "text/uri-list", False);

				size_t i;
				for (i = 0; i < count; i++) {
					if (formats[i] == XtextUriList || formats[i] == XtextPlain) {
						format = (int)formats[i];
						break;
					}
				}

				if (list) {
					XFree(formats);
				}

				break;
			}

			if (E.xclient.message_type == XdndPosition) {
				const i32 xabs = (E.xclient.data.l[2] >> 16) & 0xffff;
				const i32 yabs = (E.xclient.data.l[2]) & 0xffff;
				Window dummy;
				i32 xpos, ypos;

				if (version > 5)
					break;

				XTranslateCoordinates(
					_RGFW->display, XDefaultRootWindow(_RGFW->display), win->src.window,
					xabs, yabs, &xpos, &ypos, &dummy
				);

				event.drag.x = xpos;
				event.drag.y = ypos;

				reply.xclient.window = source;
				reply.xclient.message_type = XdndStatus;

				if (format) {
					reply.xclient.data.l[1] = 1;
					if (version >= 2)
						reply.xclient.data.l[4] = (long)XdndActionCopy;
				}

				XSendEvent(_RGFW->display, source, False, NoEventMask, &reply);
				XFlush(_RGFW->display);
				break;
			}
			if (E.xclient.message_type != XdndDrop)
				break;

			if (version > 5)
				break;

			event.type = RGFW_dataDrag;

			if (format) {
				Time time = (version >= 1)
					? (Time)E.xclient.data.l[2]
					: CurrentTime;

				XConvertSelection(
					_RGFW->display, XdndSelection, (Atom)format,
					XdndSelection, win->src.window, time
				);
			} else if (version >= 2) {
				XEvent new_reply = { ClientMessage };

				XSendEvent(_RGFW->display, source, False, NoEventMask, &new_reply);
				XFlush(_RGFW->display);
			}

			_RGFW->windowState.win = win;
			_RGFW->windowState.dataDragging = RGFW_TRUE;
			_RGFW->windowState.dropX = event.drag.x;
			_RGFW->windowState.dropY = event.drag.y;

			if (win->internal.enabledEvents & RGFW_dataDragFlag) return;
			RGFW_dataDragCallback(win, event.drag.x, event.drag.y);
		} break;
		case SelectionNotify: {
			/* this is only for checking for xdnd drops */
			if (!(win->internal.enabledEvents & RGFW_dataDropFlag) || E.xselection.property != XdndSelection || !(win->internal.flags & RGFW_windowAllowDND))
				return;
			char* data;
			unsigned long result;

			Atom actualType;
			i32 actualFormat;
			unsigned long bytesAfter;

			XGetWindowProperty(_RGFW->display, E.xselection.requestor, E.xselection.property, 0, LONG_MAX, False, E.xselection.target, &actualType, &actualFormat, &result, &bytesAfter, (u8**) &data);

			if (result == 0)
				break;

			const char* prefix = (const char*)"file://";

			char* line;

			event.drop.files = _RGFW->files;
			event.drop.count = 0;
			event.type = RGFW_dataDrop;

			while ((line = (char*)RGFW_strtok(data, "\r\n"))) {
				char path[RGFW_MAX_PATH];

				data = NULL;

				if (line[0] == '#')
					continue;

				char* l;
				for (l = line; 1; l++) {
					if ((l - line) > 7)
						break;
					else if (*l != prefix[(l - line)])
						break;
					else if (*l == '\0' && prefix[(l - line)] == '\0') {
						line += 7;
						while (*line != '/')
							line++;
						break;
					} else if (*l == '\0')
						break;
				}

				event.drop.count++;

				size_t index = 0;
				while (*line) {
					if (line[0] == '%' && line[1] && line[2]) {
						char digits[3] = {0};
                        digits[0] = line[1];
                        digits[1] = line[2];
                        digits[2] = '\0';
						path[index] = (char) RGFW_STRTOL(digits, NULL, 16);
						line += 2;
					} else
					path[index] = *line;

					index++;
					line++;
				}
				path[index] = '\0';
				RGFW_MEMCPY(event.drop.files[event.drop.count - 1], path, index + 1);
			}

			_RGFW->windowState.win = win;
			_RGFW->windowState.dataDrop = RGFW_TRUE;
			_RGFW->windowState.filesCount =  event.drop.count;

			RGFW_dataDropCallback(win, event.drop.files, event.drop.count);
			if (data)
				XFree(data);

			if (version >= 2) {
				XEvent new_reply = { ClientMessage };
				new_reply.xclient.window = source;
				new_reply.xclient.message_type = XdndFinished;
				new_reply.xclient.format = 32;
				new_reply.xclient.data.l[1] = (long int)result;
				new_reply.xclient.data.l[2] = (long int)XdndActionCopy;
				XSendEvent(_RGFW->display, source, False, NoEventMask, &new_reply);
				XFlush(_RGFW->display);
			}
			break;
		}
		case FocusIn:
			if ((win->internal.flags & RGFW_windowFullscreen))
				XMapRaised(_RGFW->display, win->src.window);
			if ((win->internal.holdMouse)) RGFW_window_holdMouse(win);

			if (!(win->internal.enabledEvents & RGFW_focusInFlag)) return;
			win->internal.inFocus = RGFW_TRUE;
			event.type = RGFW_focusIn;
			RGFW_focusCallback(win, 1);

			break;
		case FocusOut:
			if (!(win->internal.enabledEvents & RGFW_focusOutFlag)) return;
			event.type = RGFW_focusOut;
			RGFW_focusCallback(win, 0);
			RGFW_window_focusLost(win);
			break;
		case EnterNotify: {
			win->internal.mouseInside = RGFW_TRUE;
			_RGFW->windowState.win = win;
			_RGFW->windowState.mouseEnter =  RGFW_TRUE;

			if (!(win->internal.enabledEvents & RGFW_mouseEnterFlag)) return;
			event.type = RGFW_mouseEnter;
			event.mouse.x = E.xcrossing.x;
			event.mouse.y = E.xcrossing.y;
			RGFW_mouseNotifyCallback(win, event.mouse.x, event.mouse.y, 1);
			break;
		}

		case LeaveNotify: {
			win->internal.mouseInside = RGFW_FALSE;
			_RGFW->windowState.winLeave = win;
			_RGFW->windowState.mouseLeave = RGFW_TRUE;
			if (!(win->internal.enabledEvents & RGFW_mouseLeaveFlag)) return;
			event.type = RGFW_mouseLeave;
			RGFW_mouseNotifyCallback(win, event.mouse.x, event.mouse.y, 0);
			break;
		}
        case ReparentNotify:
            win->src.parent = E.xreparent.parent;
			break;
		case ConfigureNotify: {
			/* detect resize */
			RGFW_window_checkMode(win);
			if (E.xconfigure.width != win->src.w || E.xconfigure.height != win->src.h) {
				win->src.w = win->w = E.xconfigure.width;
				win->src.h = win->h = E.xconfigure.height;

				if (!(win->internal.enabledEvents & RGFW_windowResizedFlag)) return;
				event.type = RGFW_windowResized;
				RGFW_windowResizedCallback(win, win->w, win->h);
				RGFW_eventQueuePush(&event);
			}

			i32 x = E.xconfigure.x;
			i32 y = E.xconfigure.y;

			/*
			 if the event came from the server and we're not a direct child of the root window then
			 we're using local coords which need to be translated into screen coords
			*/
			Window root = DefaultRootWindow(_RGFW->display);
			if (E.xany.send_event == 0 && win->src.parent != root) {
				Window dummy = 0;
                XTranslateCoordinates(_RGFW->display, win->src.parent, root, x, y, &x, &y, &dummy);
			}

			/* detect move */
			if (E.xconfigure.x != win->src.x || E.xconfigure.y != win->src.y) {
				win->src.x = win->x = E.xconfigure.x;
				win->src.y = win->y = E.xconfigure.y;

				if (!(win->internal.enabledEvents & RGFW_windowMovedFlag)) return;
				event.type = RGFW_windowMoved;
				RGFW_windowMovedCallback(win, win->x, win->y);
				RGFW_eventQueuePush(&event);
			}
			return;
		}
		default:
			break;
	}

	if (event.type) {
		RGFW_eventQueuePush(&event);
	}

	XFlush(_RGFW->display);
}

void RGFW_FUNC(RGFW_pollEvents) (void) {
	RGFW_resetPrevState();

	XPending(_RGFW->display);
    /* if there is no unread queued events, get a new one */
	while ((QLength(_RGFW->display) || XEventsQueued(_RGFW->display, QueuedAlready) + XEventsQueued(_RGFW->display, QueuedAfterReading))) {
		RGFW_XHandleEvent();
	}
}

void RGFW_FUNC(RGFW_window_move) (RGFW_window* win, i32 x, i32 y) {
	RGFW_ASSERT(win != NULL);
	win->x = x;
	win->y = y;

	XMoveWindow(_RGFW->display, win->src.window, x, y);
	return;
}


void RGFW_FUNC(RGFW_window_resize) (RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);
	win->w = (i32)w;
	win->h = (i32)h;

	XResizeWindow(_RGFW->display, win->src.window, (u32)w, (u32)h);

	if ((win->internal.flags & RGFW_windowNoResize)) {
		XSizeHints sh;
		sh.flags = (1L << 4) | (1L << 5);
		sh.min_width = sh.max_width = (i32)w;
		sh.min_height = sh.max_height = (i32)h;

		XSetWMSizeHints(_RGFW->display, (Drawable) win->src.window, &sh, XA_WM_NORMAL_HINTS);
	}
	return;
}

void RGFW_FUNC(RGFW_window_setAspectRatio) (RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);


	if (w == 0 && h == 0)
		return;
	XSizeHints hints;
	long flags;

	XGetWMNormalHints(_RGFW->display, win->src.window, &hints, &flags);

	hints.flags |= PAspect;

	hints.min_aspect.x = hints.max_aspect.x = (i32)w;
	hints.min_aspect.y = hints.max_aspect.y = (i32)h;

	XSetWMNormalHints(_RGFW->display, win->src.window, &hints);
    return;
}

void RGFW_FUNC(RGFW_window_setMinSize) (RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);

    long flags;
	XSizeHints hints;
    RGFW_MEMSET(&hints, 0, sizeof(XSizeHints));

	XGetWMNormalHints(_RGFW->display, win->src.window, &hints, &flags);

	hints.flags |= PMinSize;

	hints.min_width = (i32)w;
	hints.min_height = (i32)h;

	XSetWMNormalHints(_RGFW->display, win->src.window, &hints);
    return;
}

void RGFW_FUNC(RGFW_window_setMaxSize) (RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);

    long flags;
	XSizeHints hints;
    RGFW_MEMSET(&hints, 0, sizeof(XSizeHints));

	XGetWMNormalHints(_RGFW->display, win->src.window, &hints, &flags);

	hints.flags |= PMaxSize;

	hints.max_width = (i32)w;
	hints.max_height = (i32)h;

	XSetWMNormalHints(_RGFW->display, win->src.window, &hints);
	return;
}

void RGFW_toggleXMaximized(RGFW_window* win, RGFW_bool maximized);
void RGFW_toggleXMaximized(RGFW_window* win, RGFW_bool maximized) {
	RGFW_ASSERT(win != NULL);
	RGFW_LOAD_ATOM(_NET_WM_STATE);
	RGFW_LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
	RGFW_LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);

	XEvent xev = {0};
	xev.type = ClientMessage;
	xev.xclient.window = win->src.window;
	xev.xclient.message_type = _NET_WM_STATE;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = maximized;
	xev.xclient.data.l[1] = (long int)_NET_WM_STATE_MAXIMIZED_HORZ;
	xev.xclient.data.l[2] = (long int)_NET_WM_STATE_MAXIMIZED_VERT;
	xev.xclient.data.l[3] = 0;
	xev.xclient.data.l[4] = 0;

	XSendEvent(_RGFW->display, DefaultRootWindow(_RGFW->display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

void RGFW_FUNC(RGFW_window_maximize) (RGFW_window* win) {
	win->internal.oldX = win->x;
	win->internal.oldY = win->y;
	win->internal.oldW = win->w;
	win->internal.oldH = win->h;

    RGFW_toggleXMaximized(win, 1);
    return;
}

void RGFW_FUNC(RGFW_window_focus) (RGFW_window* win) {
	RGFW_ASSERT(win);

    XWindowAttributes attr;
    XGetWindowAttributes(_RGFW->display, win->src.window, &attr);
    if (attr.map_state != IsViewable) return;

	XSetInputFocus(_RGFW->display, win->src.window, RevertToPointerRoot, CurrentTime);
	XFlush(_RGFW->display);
}

void RGFW_FUNC(RGFW_window_raise) (RGFW_window* win) {
	RGFW_ASSERT(win);
    XRaiseWindow(_RGFW->display, win->src.window);
	XMapRaised(_RGFW->display, win->src.window);
}

void RGFW_window_setXAtom(RGFW_window* win, Atom netAtom, RGFW_bool fullscreen);
void RGFW_window_setXAtom(RGFW_window* win, Atom netAtom, RGFW_bool fullscreen) {
	RGFW_ASSERT(win != NULL);
    RGFW_LOAD_ATOM(_NET_WM_STATE);

	XEvent xev = {0};
    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.message_type = _NET_WM_STATE;
    xev.xclient.window = win->src.window;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = fullscreen;
    xev.xclient.data.l[1] = (long int)netAtom;
    xev.xclient.data.l[2] = 0;

    XSendEvent(_RGFW->display, DefaultRootWindow(_RGFW->display), False, SubstructureNotifyMask | SubstructureRedirectMask, &xev);
}

void RGFW_FUNC(RGFW_window_setFullscreen)(RGFW_window* win, RGFW_bool fullscreen) {
	RGFW_ASSERT(win != NULL);

    if (fullscreen) {
		win->internal.flags |= RGFW_windowFullscreen;
		win->internal.oldX = win->x;
		win->internal.oldY = win->y;
		win->internal.oldW = win->w;
		win->internal.oldH = win->h;
	}
	else win->internal.flags &= ~(u32)RGFW_windowFullscreen;
	RGFW_LOAD_ATOM(_NET_WM_STATE_FULLSCREEN);

	RGFW_window_setXAtom(win, _NET_WM_STATE_FULLSCREEN, fullscreen);

	XRaiseWindow(_RGFW->display, win->src.window);
	XMapRaised(_RGFW->display, win->src.window);
}

void RGFW_FUNC(RGFW_window_setFloating)(RGFW_window* win, RGFW_bool floating) {
    RGFW_ASSERT(win != NULL);
	RGFW_LOAD_ATOM(_NET_WM_STATE_ABOVE);
	RGFW_window_setXAtom(win, _NET_WM_STATE_ABOVE, floating);
}

void RGFW_FUNC(RGFW_window_setOpacity)(RGFW_window* win, u8 opacity) {
	RGFW_ASSERT(win != NULL);
    const u32 value = (u32) (0xffffffffu * (double) opacity);
	RGFW_LOAD_ATOM(NET_WM_WINDOW_OPACITY);
    XChangeProperty(_RGFW->display, win->src.window,
					NET_WM_WINDOW_OPACITY, XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &value, 1);
}

void RGFW_FUNC(RGFW_window_minimize)(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	if (RGFW_window_isMaximized(win)) return;

	win->internal.oldX = win->x;
	win->internal.oldY = win->y;
	win->internal.oldW = win->w;
	win->internal.oldH = win->h;
    XIconifyWindow(_RGFW->display, win->src.window, DefaultScreen(_RGFW->display));
	XFlush(_RGFW->display);
}

void RGFW_FUNC(RGFW_window_restore)(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_toggleXMaximized(win, RGFW_FALSE);
	RGFW_window_move(win, win->internal.oldX, win->internal.oldY);
	RGFW_window_resize(win, win->internal.oldW, win->internal.oldH);

    RGFW_window_show(win);
    XFlush(_RGFW->display);
}

RGFW_bool RGFW_FUNC(RGFW_window_isFloating)(RGFW_window* win) {
    RGFW_LOAD_ATOM(_NET_WM_STATE);
	RGFW_LOAD_ATOM(_NET_WM_STATE_ABOVE);

	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytes_after;
	Atom* prop_return = NULL;

	int status = XGetWindowProperty(_RGFW->display, win->src.window, _NET_WM_STATE, 0, (~0L), False, XA_ATOM,
									&actual_type, &actual_format, &nitems, &bytes_after,
									(unsigned char **)&prop_return);

	if (status != Success || actual_type != XA_ATOM)
		return RGFW_FALSE;

    unsigned long i;
	for (i = 0; i < nitems; i++)
		if (prop_return[i] == _NET_WM_STATE_ABOVE) return RGFW_TRUE;

	if (prop_return)
		XFree(prop_return);
	return RGFW_FALSE;
}

void RGFW_FUNC(RGFW_window_setName)(RGFW_window* win, const char* name) {
	RGFW_ASSERT(win != NULL);

	XStoreName(_RGFW->display, win->src.window, name);

	RGFW_LOAD_ATOM(_NET_WM_NAME); RGFW_LOAD_ATOM(UTF8_STRING);

    char buf[256];
    RGFW_MEMSET(buf, 0, sizeof(buf));
    RGFW_STRNCPY(buf, name, sizeof(buf) - 1);

    XChangeProperty(
		_RGFW->display, win->src.window, _NET_WM_NAME, UTF8_STRING,
		8, PropModeReplace, (u8*)buf, sizeof(buf)
	);
}

#ifndef RGFW_NO_PASSTHROUGH
void RGFW_FUNC(RGFW_window_setMousePassthrough) (RGFW_window* win, RGFW_bool passthrough) {
	RGFW_ASSERT(win != NULL);
    if (passthrough) {
		Region region = XCreateRegion();
		XShapeCombineRegion(_RGFW->display, win->src.window, ShapeInput, 0, 0, region, ShapeSet);
		XDestroyRegion(region);

		return;
	}

	XShapeCombineMask(_RGFW->display, win->src.window, ShapeInput, 0, 0, None, ShapeSet);
}
#endif /* RGFW_NO_PASSTHROUGH */

RGFW_bool RGFW_FUNC(RGFW_window_setIconEx) (RGFW_window* win, u8* data_src, i32 w, i32 h, RGFW_format format, RGFW_icon type) {
	Atom _NET_WM_ICON = XInternAtom(_RGFW->display, "_NET_WM_ICON", False);
	RGFW_ASSERT(win != NULL);
	if (data_src == NULL) {
		RGFW_bool res = (RGFW_bool)XChangeProperty(
			_RGFW->display, win->src.window, _NET_WM_ICON, XA_CARDINAL, 32,
			PropModeReplace, (u8*)NULL, 0
		);
		return res;
	}

	i32 count = (i32)(2 + (w * h));

	unsigned long* data = (unsigned long*) RGFW_ALLOC((u32)count * sizeof(unsigned long));
    RGFW_ASSERT(data != NULL);

	RGFW_MEMSET(data, 0, (u32)count * sizeof(unsigned long));
    data[0] = (unsigned long)w;
	data[1] = (unsigned long)h;

	RGFW_copyImageData64((u8*)&data[2], w, h, RGFW_formatBGRA8, data_src, format, RGFW_TRUE);
	RGFW_bool res = RGFW_TRUE;
	if (type & RGFW_iconTaskbar) {
		res = (RGFW_bool)XChangeProperty(
			_RGFW->display, win->src.window, _NET_WM_ICON, XA_CARDINAL, 32,
			PropModeReplace, (u8*)data, count
		);
	}

	RGFW_copyImageData64((u8*)&data[2], w, h, RGFW_formatBGRA8, data_src, format, RGFW_FALSE);

	if (type & RGFW_iconWindow) {
		XWMHints wm_hints;
		wm_hints.flags = IconPixmapHint;

		i32 depth = DefaultDepth(_RGFW->display, DefaultScreen(_RGFW->display));
		XImage *image = XCreateImage(_RGFW->display, DefaultVisual(_RGFW->display, DefaultScreen(_RGFW->display)),
									(u32)depth, ZPixmap, 0, (char *)&data[2], (u32)w, (u32)h, 32, 0);

		wm_hints.icon_pixmap = XCreatePixmap(_RGFW->display, win->src.window, (u32)w, (u32)h, (u32)depth);
		XPutImage(_RGFW->display, wm_hints.icon_pixmap, DefaultGC(_RGFW->display, DefaultScreen(_RGFW->display)), image, 0, 0, 0, 0, (u32)w, (u32)h);
		image->data = NULL;
		XDestroyImage(image);

		XSetWMHints(_RGFW->display, win->src.window, &wm_hints);
	}

	RGFW_FREE(data);
	XFlush(_RGFW->display);
	return RGFW_BOOL(res);
}

RGFW_mouse* RGFW_FUNC(RGFW_loadMouse) (u8* data, i32 w, i32 h, RGFW_format format) {
    RGFW_ASSERT(data);
#ifndef RGFW_NO_X11_CURSOR
	RGFW_init();
    XcursorImage* native = XcursorImageCreate((i32)w, (i32)h);
	native->xhot = 0;
	native->yhot = 0;
	RGFW_MEMSET(native->pixels, 0, (u32)(w * h * 4));
	RGFW_copyImageData((u8*)native->pixels, w, h, RGFW_formatBGRA8, data, format);

	Cursor cursor = XcursorImageLoadCursor(_RGFW->display, native);
	XcursorImageDestroy(native);

	return (void*)cursor;
#else
	RGFW_UNUSED(data); RGFW_UNUSED(w); RGFW_UNUSED(h); RGFW_UNUSED(format);
	return NULL;
#endif
}

void RGFW_FUNC(RGFW_window_setMouse)(RGFW_window* win, RGFW_mouse* mouse) {
	RGFW_ASSERT(win && mouse);
	XDefineCursor(_RGFW->display, win->src.window, (Cursor)mouse);
}

void RGFW_FUNC(RGFW_freeMouse)(RGFW_mouse* mouse) {
	RGFW_ASSERT(mouse);
	XFreeCursor(_RGFW->display, (Cursor)mouse);
}

void RGFW_FUNC(RGFW_window_moveMouse)(RGFW_window* win, i32 x, i32 y) {
	RGFW_ASSERT(win != NULL);

	XEvent event;
	XQueryPointer(_RGFW->display, DefaultRootWindow(_RGFW->display),
		&event.xbutton.root, &event.xbutton.window,
		&event.xbutton.x_root, &event.xbutton.y_root,
		&event.xbutton.x, &event.xbutton.y,
		&event.xbutton.state);

	win->internal.lastMouseX = x - win->x;
	win->internal.lastMouseY = y - win->y;
	if (event.xbutton.x == x && event.xbutton.y == y)
		return;

	XWarpPointer(_RGFW->display, None, win->src.window, 0, 0, 0, 0, (int) x - win->x, (int) y - win->y);
}

RGFW_bool RGFW_FUNC(RGFW_window_setMouseDefault) (RGFW_window* win) {
	return RGFW_window_setMouseStandard(win, RGFW_mouseArrow);
}

RGFW_bool RGFW_FUNC(RGFW_window_setMouseStandard) (RGFW_window* win, u8 mouse) {
	RGFW_ASSERT(win != NULL);

	static const u8 mouseIconSrc[16] = { XC_arrow, XC_left_ptr, XC_xterm, XC_crosshair, XC_hand2, XC_sb_h_double_arrow, XC_sb_v_double_arrow, XC_bottom_left_corner, XC_bottom_right_corner, XC_fleur, XC_X_cursor};

	if (mouse > (sizeof(mouseIconSrc) / sizeof(u8)))
		return RGFW_FALSE;

	mouse = mouseIconSrc[mouse];

	Cursor cursor = XCreateFontCursor(_RGFW->display, mouse);
	XDefineCursor(_RGFW->display, win->src.window, (Cursor) cursor);
	XFreeCursor(_RGFW->display, (Cursor) cursor);
	return RGFW_TRUE;
}

void RGFW_FUNC(RGFW_window_hide)(RGFW_window* win) {
	XUnmapWindow(_RGFW->display, win->src.window);
}

void RGFW_FUNC(RGFW_window_show) (RGFW_window* win) {
	win->internal.flags &= ~(u32)RGFW_windowHide;
	if (win->internal.flags & RGFW_windowFocusOnShow) RGFW_window_focus(win);

	XMapWindow(_RGFW->display, win->src.window);
	RGFW_window_move(win, win->x, win->y);
	return;
}

RGFW_ssize_t RGFW_FUNC(RGFW_readClipboardPtr)(char* str, size_t strCapacity) {
	RGFW_init();
	RGFW_LOAD_ATOM(XSEL_DATA); RGFW_LOAD_ATOM(UTF8_STRING); RGFW_LOAD_ATOM(CLIPBOARD);
	if (XGetSelectionOwner(_RGFW->display, CLIPBOARD) == _RGFW->helperWindow) {
		if (str != NULL)
			RGFW_STRNCPY(str, _RGFW->clipboard, _RGFW->clipboard_len - 1);
		_RGFW->clipboard[_RGFW->clipboard_len - 1] = '\0';
		return (RGFW_ssize_t)_RGFW->clipboard_len - 1;
	}

	XEvent event;
	int format;
	unsigned long N, sizeN;
	char* data;
	Atom target;

	XConvertSelection(_RGFW->display, CLIPBOARD, UTF8_STRING, XSEL_DATA, _RGFW->helperWindow, CurrentTime);
	XSync(_RGFW->display, 0);
	while (1) {
		XNextEvent(_RGFW->display, &event);
		if (event.type != SelectionNotify) continue;

		if (event.xselection.selection != CLIPBOARD || event.xselection.property == 0)
			return -1;
		break;
	}

	XGetWindowProperty(event.xselection.display, event.xselection.requestor,
			event.xselection.property, 0L, (~0L), 0, AnyPropertyType, &target,
			&format, &sizeN, &N, (u8**) &data);

	RGFW_ssize_t size;
	if (sizeN > strCapacity && str != NULL)
		size = -1;

	if ((target == UTF8_STRING || target == XA_STRING) && str != NULL) {
		RGFW_MEMCPY(str, data, sizeN);
		str[sizeN] = '\0';
		XFree(data);
	} else if (str != NULL) size = -1;

	XDeleteProperty(event.xselection.display, event.xselection.requestor, event.xselection.property);
	size = (RGFW_ssize_t)sizeN;

    return size;
}

i32 RGFW_XHandleClipboardSelectionHelper(void) {
    RGFW_LOAD_ATOM(SAVE_TARGETS);

    XEvent event;
    XPending(_RGFW->display);

    if (QLength(_RGFW->display) || XEventsQueued(_RGFW->display, QueuedAlready) + XEventsQueued(_RGFW->display, QueuedAfterReading))
        XNextEvent(_RGFW->display, &event);
    else
        return 0;

    switch (event.type) {
        case SelectionRequest:
            RGFW_XHandleClipboardSelection(&event);
            return 0;
        case SelectionNotify:
            if (event.xselection.target == SAVE_TARGETS)
                return 0;
            break;
        default: break;
    }

    return 0;
}

void RGFW_FUNC(RGFW_writeClipboard)(const char* text, u32 textLen) {
	RGFW_LOAD_ATOM(SAVE_TARGETS); RGFW_LOAD_ATOM(CLIPBOARD);
    RGFW_init();

    /* request ownership of the clipboard section and request to convert it, this means its our job to convert it */
	XSetSelectionOwner(_RGFW->display, CLIPBOARD, _RGFW->helperWindow, CurrentTime);
	if (XGetSelectionOwner(_RGFW->display, CLIPBOARD) != _RGFW->helperWindow) {
    	RGFW_sendDebugInfo(RGFW_typeError, RGFW_errClipboard,  "X11 failed to become owner of clipboard selection");
		return;
	}

	if (_RGFW->clipboard)
		RGFW_FREE(_RGFW->clipboard);

	_RGFW->clipboard = (char*)RGFW_ALLOC(textLen);
	RGFW_ASSERT(_RGFW->clipboard != NULL);

	RGFW_STRNCPY(_RGFW->clipboard, text, textLen - 1);
	_RGFW->clipboard[textLen - 1] = '\0';
	_RGFW->clipboard_len = textLen;
	return;
}

RGFW_bool RGFW_FUNC(RGFW_window_isHidden)(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	XWindowAttributes windowAttributes;
	XGetWindowAttributes(_RGFW->display, win->src.window, &windowAttributes);

	return (windowAttributes.map_state == IsUnmapped && !RGFW_window_isMinimized(win));
}

RGFW_bool RGFW_FUNC(RGFW_window_isMinimized)(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
    RGFW_LOAD_ATOM(WM_STATE);

	Atom actual_type;
	i32 actual_format;
	unsigned long nitems, bytes_after;
	unsigned char* prop_data;

	i32 status = XGetWindowProperty(_RGFW->display, win->src.window, WM_STATE, 0, 2, False,
		AnyPropertyType, &actual_type, &actual_format,
		&nitems, &bytes_after, &prop_data);

	if (status == Success && nitems >= 1 && prop_data == (unsigned char*)IconicState) {
		XFree(prop_data);
		return RGFW_TRUE;
	}

	if (prop_data != NULL)
		XFree(prop_data);

	XWindowAttributes windowAttributes;
	XGetWindowAttributes(_RGFW->display, win->src.window, &windowAttributes);
    return windowAttributes.map_state != IsViewable;
}

RGFW_bool RGFW_FUNC(RGFW_window_isMaximized)(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
    RGFW_LOAD_ATOM(_NET_WM_STATE);
	RGFW_LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
	RGFW_LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);

	Atom actual_type;
	i32 actual_format;
	unsigned long nitems, bytes_after;
	unsigned char* prop_data;

	i32 status = XGetWindowProperty(_RGFW->display, win->src.window, _NET_WM_STATE, 0, 1024, False,
		XA_ATOM, &actual_type, &actual_format,
		&nitems, &bytes_after, &prop_data);

	if (status != Success) {
		if (prop_data != NULL)
			XFree(prop_data);

		return RGFW_FALSE;
	}

	u64 i;
	for (i = 0; i < nitems; ++i) {
		if (prop_data[i] == _NET_WM_STATE_MAXIMIZED_VERT ||
			prop_data[i] == _NET_WM_STATE_MAXIMIZED_HORZ) {
			XFree(prop_data);
			return RGFW_TRUE;
		}
	}

	if (prop_data != NULL)
		XFree(prop_data);

	return RGFW_FALSE;
}

static float XGetSystemContentDPI(Display* display, i32 screen) {
	float dpi = 96.0f;

	#ifndef RGFW_NO_DPI
		RGFW_UNUSED(screen);
		char* rms = XResourceManagerString(display);
		XrmDatabase db = NULL;
		if (rms) db = XrmGetStringDatabase(rms);

		if (rms && db) {
			XrmValue value;
			char* type = NULL;

			if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value) && type && RGFW_STRNCMP(type, "String", 7) == 0)
				dpi = (float)RGFW_ATOF(value.addr);
			XrmDestroyDatabase(db);
		}
	#else
		dpi = RGFW_ROUND(DisplayWidth(display, screen) / (DisplayWidthMM(display, screen) / 25.4));
	#endif

	return dpi;
}

RGFW_monitor RGFW_XCreateMonitor(i32 screen);
RGFW_monitor RGFW_XCreateMonitor(i32 screen) {
	RGFW_monitor monitor;
    RGFW_init();

    Display* display = _RGFW->display;

	if (screen == -1) screen = DefaultScreen(display);

	Screen* scrn = DefaultScreenOfDisplay(display);

	monitor.x = 0;
	monitor.y = 0;
	monitor.mode.w = scrn->width;
	monitor.mode.h = scrn->height;
	monitor.physW = (float)DisplayWidthMM(display, screen) / 25.4f;
	monitor.physH = (float)DisplayHeightMM(display, screen) / 25.4f;

	RGFW_splitBPP((u32)DefaultDepth(display, screen), &monitor.mode);

	char* name = XDisplayName((const char*)display);
	RGFW_STRNCPY(monitor.name, name, sizeof(monitor.name) - 1);
	monitor.name[sizeof(monitor.name) - 1] = '\0';

	float dpi = XGetSystemContentDPI(display, screen);
	monitor.pixelRatio = dpi >= 192.0f ? 2 : 1.0f;
	monitor.scaleX = (float) (dpi) / 96.0f;
	monitor.scaleY = (float) (dpi) / 96.0f;

	#ifndef RGFW_NO_DPI
	XRRCrtcInfo* ci = NULL;
	XRRScreenResources* sr = NULL;

	{
		XRRScreenConfiguration* conf = XRRGetScreenInfo(display, RootWindow(display, screen));
		monitor.mode.refreshRate = (u32)XRRConfigCurrentRate(conf);

		sr = XRRGetScreenResourcesCurrent(display, RootWindow(display, screen));
		int crtc = screen;

		if (sr->ncrtc > crtc) {
			ci = XRRGetCrtcInfo(display, sr, sr->crtcs[crtc]);
		}

		XRRFreeScreenConfigInfo(conf);
	}
	#endif

	#ifndef RGFW_NO_DPI
		XRROutputInfo* info = XRRGetOutputInfo (display, sr, sr->outputs[screen]);

		if (info == NULL || ci == NULL) {
			XRRFreeScreenResources(sr);
			RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoMonitor,  "monitor found");
			return monitor;
		}


		float physW = (float)info->mm_width / 25.4f;
		float physH = (float)info->mm_height / 25.4f;

		RGFW_STRNCPY(monitor.name, info->name, sizeof(monitor.name) - 1);
		monitor.name[sizeof(monitor.name) - 1] = '\0';

		XRRFreeOutputInfo(info);
		info = NULL;

		if (physW > 0.0f && physH > 0.0f) {
			monitor.physW = physW;
			monitor.physH = physH;
		}

		monitor.x = ci->x;
		monitor.y = ci->y;

		if (ci->width && ci->height) {
			monitor.mode.w = (i32)ci->width;
			monitor.mode.h = (i32)ci->height;
		}
	#endif

	#ifndef RGFW_NO_DPI
		XRRFreeCrtcInfo(ci);
		XRRFreeScreenResources(sr);
	#endif

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoMonitor,  "monitor found");
    return monitor;
}

RGFW_monitor* RGFW_FUNC(RGFW_getMonitors)(size_t* len) {
	static RGFW_monitor monitors[7];
    RGFW_init();

	Display* display = _RGFW->display;
	i32 max = ScreenCount(display);

	i32 i;
	for (i = 0; i < max && i < 6; i++)
		monitors[i] = RGFW_XCreateMonitor(i);

	if (len != NULL) *len = (size_t)((max <= 6) ? (max) : (6));

	return monitors;
}

RGFW_monitor RGFW_FUNC(RGFW_getPrimaryMonitor)(void) {
	return RGFW_XCreateMonitor(-1);
}

RGFW_bool RGFW_FUNC(RGFW_monitor_requestMode)(RGFW_monitor mon, RGFW_monitorMode mode, RGFW_modeRequest request) {
	#ifndef RGFW_NO_DPI
    RGFW_init();
    XRRScreenConfiguration *conf = XRRGetScreenInfo(_RGFW->display, DefaultRootWindow(_RGFW->display));
    XRRScreenResources* screenRes = XRRGetScreenResources(_RGFW->display, DefaultRootWindow(_RGFW->display));
	if (screenRes == NULL) return RGFW_FALSE;

    int i;
    for (i = 0; i < screenRes->ncrtc; i++) {
		XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(_RGFW->display, screenRes, screenRes->crtcs[i]);
		if (!crtcInfo) continue;

		if (mon.x == crtcInfo->x && mon.y == crtcInfo->y && (u32)mon.mode.w == crtcInfo->width && (u32)mon.mode.h == crtcInfo->height) {
			RRMode rmode = None;
            int index;
            for (index = 0; index < screenRes->nmode; index++) {
				RGFW_monitorMode foundMode;
				foundMode.w = (i32)screenRes->modes[index].width;
				foundMode.h = (i32)screenRes->modes[index].height;
				foundMode.refreshRate = (u32)XRRConfigCurrentRate(conf);
				RGFW_splitBPP((u32)DefaultDepth(_RGFW->display, DefaultScreen(_RGFW->display)), &foundMode);

				if (RGFW_monitorModeCompare(mode, foundMode, request)) {
					rmode = screenRes->modes[index].id;

					RROutput output = screenRes->outputs[i];
					XRROutputInfo* info = XRRGetOutputInfo(_RGFW->display, screenRes, output);
					if (info) {
						XRRSetCrtcConfig(_RGFW->display, screenRes, screenRes->crtcs[i],
										CurrentTime, 0, 0, rmode, RR_Rotate_0, &output, 1);
						XRRFreeOutputInfo(info);
						XRRFreeCrtcInfo(crtcInfo);
						XRRFreeScreenResources(screenRes);
						return RGFW_TRUE;
					}
				}
			}

			XRRFreeCrtcInfo(crtcInfo);
			XRRFreeScreenResources(screenRes);
			return RGFW_FALSE;
		}

		XRRFreeCrtcInfo(crtcInfo);
	}

    XRRFreeScreenResources(screenRes);
	XRRFreeScreenConfigInfo(conf);
#endif
	return RGFW_FALSE;
}

RGFW_monitor RGFW_FUNC(RGFW_window_getMonitor) (RGFW_window* win) {
    RGFW_monitor mon;
    RGFW_MEMSET(&mon, 0, sizeof(mon));

    RGFW_ASSERT(win != NULL);

	XWindowAttributes attrs;
    if (!XGetWindowAttributes(_RGFW->display, win->src.window, &attrs)) {
        return mon;
    }

	i32 i;
	for (i = 0; i < ScreenCount(_RGFW->display) && i < 6; i++) {
		Screen* screen = ScreenOfDisplay(_RGFW->display, i);
        if (attrs.x >= 0 && attrs.x < XWidthOfScreen(screen) &&
            attrs.y >= 0 && attrs.y < XHeightOfScreen(screen))
            	return RGFW_XCreateMonitor(i);
	}
    return mon;

}

#ifdef RGFW_OPENGL
RGFW_bool RGFW_FUNC(RGFW_window_createContextPtr_OpenGL) (RGFW_window* win, RGFW_glContext* context, RGFW_glHints* hints) {
	/* for checking extensions later */
	const char sRGBARBstr[] = "GLX_ARB_framebuffer_sRGB";
	const char sRGBEXTstr[] = "GLX_EXT_framebuffer_sRGB";
	const char noErorrStr[]  = "GLX_ARB_create_context_no_error";
	const char flushStr[] = "GLX_ARB_context_flush_control";
	const char robustStr[]	= "GLX_ARB_create_context_robustness";

	/* basic RGFW int */
	win->src.ctx.native = context;
	win->src.gfxType = RGFW_gfxNativeOpenGL;
	/*  This is required so that way the user can create their own OpenGL context after RGFW_createWindow is used */
	if (win->src.window) RGFW_window_closePlatform(win);

	RGFW_bool transparent = (win->internal.flags & RGFW_windowTransparent);

	/* start by creating a GLX config / X11 Viusal */
	XVisualInfo visual;
	GLXFBConfig bestFbc;

	i32 visual_attribs[40];
	RGFW_attribStack stack;
	RGFW_attribStack_init(&stack, visual_attribs, 40);
	RGFW_attribStack_pushAttribs(&stack, GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR);
	RGFW_attribStack_pushAttribs(&stack, GLX_X_RENDERABLE, 1);
	RGFW_attribStack_pushAttribs(&stack, GLX_RENDER_TYPE, GLX_RGBA_BIT);
	RGFW_attribStack_pushAttribs(&stack, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT);
	RGFW_attribStack_pushAttribs(&stack, GLX_DOUBLEBUFFER, 1);
	RGFW_attribStack_pushAttribs(&stack, GLX_ALPHA_SIZE, hints->alpha);
	RGFW_attribStack_pushAttribs(&stack, GLX_DEPTH_SIZE, hints->depth);
	RGFW_attribStack_pushAttribs(&stack, GLX_STENCIL_SIZE, hints->stencil);
	RGFW_attribStack_pushAttribs(&stack, GLX_STEREO, hints->stereo);
	RGFW_attribStack_pushAttribs(&stack, GLX_AUX_BUFFERS, hints->auxBuffers);
	RGFW_attribStack_pushAttribs(&stack, GLX_RED_SIZE, hints->red);
	RGFW_attribStack_pushAttribs(&stack, GLX_GREEN_SIZE, hints->green);
	RGFW_attribStack_pushAttribs(&stack, GLX_BLUE_SIZE, hints->blue);
	RGFW_attribStack_pushAttribs(&stack, GLX_ACCUM_RED_SIZE, hints->accumRed);
	RGFW_attribStack_pushAttribs(&stack, GLX_ACCUM_GREEN_SIZE, hints->accumGreen);
	RGFW_attribStack_pushAttribs(&stack, GLX_ACCUM_BLUE_SIZE, hints->accumBlue);
	RGFW_attribStack_pushAttribs(&stack, GLX_ACCUM_ALPHA_SIZE, hints->accumAlpha);

	if (hints->sRGB) {
		if (RGFW_extensionSupportedPlatform_OpenGL(sRGBARBstr, sizeof(sRGBARBstr)))
			RGFW_attribStack_pushAttribs(&stack, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, hints->sRGB);
		if (RGFW_extensionSupportedPlatform_OpenGL(sRGBEXTstr, sizeof(sRGBEXTstr)))
			RGFW_attribStack_pushAttribs(&stack, GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT, hints->sRGB);
	}

	RGFW_attribStack_pushAttribs(&stack, 0, 0);

	/* find the configs */
	i32 fbcount;
	GLXFBConfig* fbc = glXChooseFBConfig(_RGFW->display, DefaultScreen(_RGFW->display), visual_attribs, &fbcount);

	i32 best_fbc = -1;
	i32 best_depth = 0;
	i32 best_samples = 0;

	if (fbcount == 0) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext, "Failed to find any valid GLX visual configs.");
		return 0;
	}

	/* search through all found configs to find the best match */
	i32 i;
	for (i = 0; i < fbcount; i++) {
		XVisualInfo* vi = glXGetVisualFromFBConfig(_RGFW->display, fbc[i]);
		if (vi == NULL)
			continue;

		i32 samp_buf, samples;
		glXGetFBConfigAttrib(_RGFW->display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
		glXGetFBConfigAttrib(_RGFW->display, fbc[i], GLX_SAMPLES, &samples);

		if (best_fbc == -1) best_fbc = i;
		if ((!(transparent) || vi->depth == 32)  && best_depth == 0) {
			best_fbc = i;
			best_depth = vi->depth;
		}
		if ((!(transparent) || vi->depth == 32) && samples <= hints->samples && samples > best_samples) {
			best_fbc = i;
			best_depth = vi->depth;
			best_samples = samples;
		}
		XFree(vi);
	}

	if (best_fbc == -1) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext, "Failed to get a valid GLX visual.");
		return 0;
	}

	/* we found a config */
	bestFbc = fbc[best_fbc];
	XVisualInfo* vi = glXGetVisualFromFBConfig(_RGFW->display, bestFbc);
	if (vi->depth != 32 && transparent)
		RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningOpenGL,  "Failed to to find a matching visual with a 32-bit depth.");

	if (best_samples < hints->samples)
		RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningOpenGL, "Failed to load a matching sample count.");

	XFree(fbc);
	visual = *vi;
	XFree(vi);

	/* use the visual to create a new window */
	RGFW_XCreateWindow(visual, "", win->internal.flags, win);

	/* create the actual OpenGL context  */
	i32 context_attribs[40];
	RGFW_attribStack_init(&stack, context_attribs, 40);

	i32 mask = 0;
	switch (hints->profile) {
		case RGFW_glES: mask |= GLX_CONTEXT_ES_PROFILE_BIT_EXT; break;
		case RGFW_glCompatibility: mask |= GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB; break;
		case RGFW_glCore: mask |= GLX_CONTEXT_CORE_PROFILE_BIT_ARB; break;
		default: mask |= GLX_CONTEXT_CORE_PROFILE_BIT_ARB; break;
	}

	RGFW_attribStack_pushAttribs(&stack, GLX_CONTEXT_PROFILE_MASK_ARB, mask);

	if (hints->minor || hints->major) {
		RGFW_attribStack_pushAttribs(&stack, GLX_CONTEXT_MAJOR_VERSION_ARB, hints->major);
		RGFW_attribStack_pushAttribs(&stack, GLX_CONTEXT_MINOR_VERSION_ARB, hints->minor);
	}


	if (RGFW_extensionSupportedPlatform_OpenGL(flushStr, sizeof(flushStr))) {
		if (hints->releaseBehavior == RGFW_glReleaseFlush) {
			RGFW_attribStack_pushAttribs(&stack, GLX_CONTEXT_RELEASE_BEHAVIOR_ARB, GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB);
		} else if (hints->releaseBehavior == RGFW_glReleaseNone) {
			RGFW_attribStack_pushAttribs(&stack, GLX_CONTEXT_RELEASE_BEHAVIOR_ARB, GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
		}
	}

	i32 flags = 0;
	if (hints->debug) flags |= GLX_CONTEXT_DEBUG_BIT_ARB;
	if (hints->robustness && RGFW_extensionSupportedPlatform_OpenGL(robustStr, sizeof(robustStr))) flags |= GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB;
	if (flags) {
		RGFW_attribStack_pushAttribs(&stack, GLX_CONTEXT_FLAGS_ARB, flags);
	}

	if (RGFW_extensionSupportedPlatform_OpenGL(noErorrStr, sizeof(noErorrStr))) {
		RGFW_attribStack_pushAttribs(&stack, GLX_CONTEXT_OPENGL_NO_ERROR_ARB, hints->noError);
	}

	RGFW_attribStack_pushAttribs(&stack, 0, 0);

	/*  create the context */
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	char str[] = "glXCreateContextAttribsARB";
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((u8*) str);

	GLXContext ctx = NULL;
	if (hints->share) {
		ctx = hints->share->ctx;
	}

	if (glXCreateContextAttribsARB == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext, "Failed to load proc address 'glXCreateContextAttribsARB', loading a generic OpenGL context.");
			win->src.ctx.native->ctx = glXCreateContext(_RGFW->display, &visual, ctx, True);
	} else {
		_RGFW->x11Error = NULL;
		win->src.ctx.native->ctx = glXCreateContextAttribsARB(_RGFW->display, bestFbc, ctx, True, context_attribs);
		if (_RGFW->x11Error || win->src.ctx.native->ctx == NULL) {
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext, "Failed to create an OpenGL context with AttribsARB, loading a generic OpenGL context.");
			win->src.ctx.native->ctx = glXCreateContext(_RGFW->display, &visual, ctx, True);
		}
	}

	#ifndef RGFW_NO_GLXWINDOW
		win->src.ctx.native->window = glXCreateWindow(_RGFW->display, bestFbc, win->src.window, NULL);
	#else
		win->src.ctx.native->window = win->src.window;
	#endif

	glXMakeCurrent(_RGFW->display, (Drawable)win->src.ctx.native->window, (GLXContext)win->src.ctx.native->ctx);
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, "OpenGL context initalized.");

	return RGFW_TRUE;
}

void RGFW_FUNC(RGFW_window_deleteContextPtr_OpenGL) (RGFW_window* win, RGFW_glContext* ctx) {
	#ifndef RGFW_NO_GLXWINDOW
	if (win->src.ctx.native->window != win->src.window) {
		glXDestroyWindow(_RGFW->display, win->src.ctx.native->window);
	}
	#endif

	glXDestroyContext(_RGFW->display, ctx->ctx);
	win->src.ctx.native = NULL;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL,  "OpenGL context freed.");
}

RGFW_bool RGFW_FUNC(RGFW_extensionSupportedPlatform_OpenGL)(const char * extension, size_t len) {
	RGFW_init();
	const char* extensions = glXQueryExtensionsString(_RGFW->display, XDefaultScreen(_RGFW->display));
	return (extensions != NULL) && RGFW_extensionSupportedStr(extensions, extension, len);
}

RGFW_proc RGFW_FUNC(RGFW_getProcAddress_OpenGL)(const char* procname) { return glXGetProcAddress((u8*) procname); }

void RGFW_FUNC(RGFW_window_makeCurrentContext_OpenGL) (RGFW_window* win) { if (win) RGFW_ASSERT(win->src.ctx.native);
	if (win == NULL)
		glXMakeCurrent(NULL, (Drawable)NULL, (GLXContext) NULL);
	else
		glXMakeCurrent(_RGFW->display, (Drawable)win->src.ctx.native->window, (GLXContext) win->src.ctx.native->ctx);
	return;
}
void* RGFW_FUNC(RGFW_getCurrentContext_OpenGL) (void) { return glXGetCurrentContext(); }
void RGFW_FUNC(RGFW_window_swapBuffers_OpenGL) (RGFW_window* win) { RGFW_ASSERT(win->src.ctx.native); glXSwapBuffers(_RGFW->display, win->src.ctx.native->window); }

void RGFW_FUNC(RGFW_window_swapInterval_OpenGL) (RGFW_window* win, i32 swapInterval) {
	RGFW_ASSERT(win != NULL);
	/* cached pfn to avoid calling glXGetProcAddress more than once */
	static PFNGLXSWAPINTERVALEXTPROC pfn = NULL;
	static int (*pfn2)(int) = NULL;

	if (pfn == NULL) {
		u8 str[] = "glXSwapIntervalEXT";
		pfn = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress(str);
		if (pfn == NULL)  {
			pfn = (PFNGLXSWAPINTERVALEXTPROC)1;
			const char* array[] = {"GLX_MESA_swap_control", "GLX_SGI_swap_control"};

			size_t i;
			for (i = 0; i < sizeof(array) / sizeof(char*) && pfn2 == NULL; i++) {
				pfn2 = (int(*)(int))glXGetProcAddress((u8*)array[i]);
			}

			if (pfn2 != NULL) {
				RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext,  "Failed to load swap interval function, fallingback to the native swapinterval function");
			} else {
				RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext,  "Failed to load swap interval function");
			}
		}
	}

	if (pfn != (PFNGLXSWAPINTERVALEXTPROC)1) {
		pfn(_RGFW->display, win->src.ctx.native->window, swapInterval);
	}
	else if (pfn2 != NULL) {
		pfn2(swapInterval);
	}
}
#endif /* RGFW_OPENGL */

i32 RGFW_initPlatform_X11(void) {
    #ifdef RGFW_USE_XDL
		XDL_init();
	#endif

	#if !defined(RGFW_NO_X11_CURSOR) && !defined(RGFW_NO_X11_CURSOR_PRELOAD)
	#if defined(__CYGWIN__)
				RGFW_LOAD_LIBRARY(X11Cursorhandle, "libXcursor-1.so");
	#elif defined(__OpenBSD__) || defined(__NetBSD__)
				RGFW_LOAD_LIBRARY(X11Cursorhandle, "libXcursor.so");
	#else
				RGFW_LOAD_LIBRARY(X11Cursorhandle, "libXcursor.so.1");
	#endif
		RGFW_PROC_DEF(X11Cursorhandle, XcursorImageCreate);
		RGFW_PROC_DEF(X11Cursorhandle, XcursorImageDestroy);
		RGFW_PROC_DEF(X11Cursorhandle, XcursorImageLoadCursor);
	#endif

	#if !defined(RGFW_NO_X11_XI_PRELOAD)
	#if defined(__CYGWIN__)
			RGFW_LOAD_LIBRARY(X11Xihandle, "libXi-6.so");
	#elif defined(__OpenBSD__) || defined(__NetBSD__)
			RGFW_LOAD_LIBRARY(X11Xihandle, "libXi.so");
	#else
			RGFW_LOAD_LIBRARY(X11Xihandle, "libXi.so.6");
	#endif
			RGFW_PROC_DEF(X11Xihandle, XISelectEvents);
	#endif

	#if !defined(RGFW_NO_X11_EXT_PRELOAD)
	#if defined(__CYGWIN__)
			RGFW_LOAD_LIBRARY(X11XEXThandle, "libXext-6.so");
	#elif defined(__OpenBSD__) || defined(__NetBSD__)
	        RGFW_LOAD_LIBRARY(X11XEXThandle, "libXext.so");
	#else
			RGFW_LOAD_LIBRARY(X11XEXThandle, "libXext.so.6");
	#endif
			RGFW_PROC_DEF(X11XEXThandle, XSyncCreateCounter);
			RGFW_PROC_DEF(X11XEXThandle, XSyncIntToValue);
			RGFW_PROC_DEF(X11XEXThandle, XSyncSetCounter);
    	    RGFW_PROC_DEF(X11XEXThandle, XShapeCombineRegion);
	        RGFW_PROC_DEF(X11XEXThandle, XShapeCombineMask);
    #endif

    XInitThreads(); /*!< init X11 threading */
    _RGFW->display = XOpenDisplay(0);
	_RGFW->context = XUniqueContext();

	XSetWindowAttributes wa;
    RGFW_MEMSET(&wa, 0, sizeof(wa));
    wa.event_mask = PropertyChangeMask;
    _RGFW->helperWindow = XCreateWindow(_RGFW->display, XDefaultRootWindow(_RGFW->display), 0, 0, 1, 1, 0, 0,
                                        InputOnly, DefaultVisual(_RGFW->display, DefaultScreen(_RGFW->display)), CWEventMask, &wa);

    u8 RGFW_blk[] = { 0, 0, 0, 0 };
	_RGFW->hiddenMouse = RGFW_loadMouse(RGFW_blk, 1, 1, RGFW_formatRGBA8);
	_RGFW->clipboard = NULL;

    XkbComponentNamesRec rec;
    XkbDescPtr desc = XkbGetMap(_RGFW->display, 0, XkbUseCoreKbd);
    XkbDescPtr evdesc;
    XSetErrorHandler(RGFW_XErrorHandler);
	u8 old[256];

    XkbGetNames(_RGFW->display, XkbKeyNamesMask, desc);

    RGFW_MEMSET(&rec, 0, sizeof(rec));
    char evdev[] = "evdev";
    rec.keycodes = evdev;
    evdesc = XkbGetKeyboardByName(_RGFW->display, XkbUseCoreKbd, &rec, XkbGBN_KeyNamesMask, XkbGBN_KeyNamesMask, False);
    /* memo: RGFW_keycodes[x11 keycode] = rgfw keycode */
    if(evdesc != NULL && desc != NULL) {
        int i, j;
        for(i = 0; i < (int)sizeof(old); i++){
    	    old[i] = _RGFW->keycodes[i];
    	    _RGFW->keycodes[i] = 0;
        }
        for(i = evdesc->min_key_code; i <= evdesc->max_key_code; i++){
    	    for(j = desc->min_key_code; j <= desc->max_key_code; j++){
                if(RGFW_STRNCMP(evdesc->names->keys[i].name, desc->names->keys[j].name, XkbKeyNameLength) == 0){
                    _RGFW->keycodes[j] = old[i];
                    break;
                }
    	    }
        }
		XkbFreeKeyboard(desc, 0, True);
		XkbFreeKeyboard(evdesc, 0, True);
    }
	return 0;
}

void RGFW_deinitPlatform_X11(void) {
    #define RGFW_FREE_LIBRARY(x) if (x != NULL) dlclose(x); x = NULL;
	/* to save the clipboard on the x server after the window is closed */
	RGFW_LOAD_ATOM(CLIPBOARD_MANAGER);  RGFW_LOAD_ATOM(CLIPBOARD);
	RGFW_LOAD_ATOM(SAVE_TARGETS);
	if (XGetSelectionOwner(_RGFW->display, CLIPBOARD) == _RGFW->helperWindow) {
		XConvertSelection(_RGFW->display, CLIPBOARD_MANAGER, SAVE_TARGETS, None, _RGFW->helperWindow, CurrentTime);
        while (RGFW_XHandleClipboardSelectionHelper());
	}
	if (_RGFW->clipboard) {
		RGFW_FREE(_RGFW->clipboard);
		_RGFW->clipboard = NULL;
	}

	if (_RGFW->hiddenMouse) {
		RGFW_freeMouse(_RGFW->hiddenMouse);
		_RGFW->hiddenMouse = NULL;
	}

    XDestroyWindow(_RGFW->display, (Drawable) _RGFW->helperWindow); /*!< close the window */
    XCloseDisplay(_RGFW->display); /*!< kill connection to the x server */

    #if !defined(RGFW_NO_X11_CURSOR_PRELOAD) && !defined(RGFW_NO_X11_CURSOR)
        RGFW_FREE_LIBRARY(X11Cursorhandle);
    #endif
    #if !defined(RGFW_NO_X11_XI_PRELOAD)
        RGFW_FREE_LIBRARY(X11Xihandle);
    #endif

    #ifdef RGFW_USE_XDL
        XDL_close();
    #endif

    #if !defined(RGFW_NO_X11_EXT_PRELOAD)
        RGFW_FREE_LIBRARY(X11XEXThandle);
    #endif
}

void RGFW_FUNC(RGFW_window_closePlatform)(RGFW_window* win) {
	if (win->internal.holdMouse)
		XUngrabPointer(_RGFW->display, CurrentTime);

	XFreeGC(_RGFW->display, win->src.gc);
	XDeleteContext(_RGFW->display, win->src.window, _RGFW->context);
	XDestroyWindow(_RGFW->display, (Drawable) win->src.window); /*!< close the window */
	return;
}

#ifdef RGFW_WEBGPU
WGPUSurface RGFW_FUNC(RGFW_window_createSurface_WebGPU) (RGFW_window* window, WGPUInstance instance) {
	WGPUSurfaceDescriptor surfaceDesc = {0};
	WGPUSurfaceSourceXlibWindow fromXlib = {0};
	fromXlib.chain.sType = WGPUSType_SurfaceSourceXlibWindow;
	fromXlib.display = _RGFW->display;
	fromXlib.window = window->src.window;

	surfaceDesc.nextInChain = (WGPUChainedStruct*)&fromXlib.chain;
	return wgpuInstanceCreateSurface(instance, &surfaceDesc);
}
#endif

#endif
/*
	End of X11 linux / wayland / unix defines
*/

/*

	Start of Wayland defayland
*/

#ifdef RGFW_WAYLAND
#ifdef RGFW_X11
#undef RGFW_FUNC /* remove previous define */
#define RGFW_FUNC(func) func##_Wayland
#else
#define RGFW_FUNC(func) func
#endif

/*
Wayland TODO: (out of date)
- fix RGFW_keyPressed lock state

	RGFW_windowMoved, 		the window was moved (by the user)
	RGFW_windowRefresh	 	The window content needs to be refreshed

	RGFW_dataDrop 				a file has been dropped into the window
	RGFW_dataDrag

- window args:
	#define RGFW_windowNoResize	 			the window cannot be resized  by the user
	#define RGFW_windowAllowDND     			the window supports drag and drop
	#define RGFW_scaleToMonitor 			scale the window to the screen

- other missing functions functions ("TODO wayland") (~30 functions)
- fix buffer rendering weird behavior
*/
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <dirent.h>
#include <linux/kd.h>
#include <wayland-cursor.h>
#include <fcntl.h>

struct wl_display* RGFW_getDisplay_Wayland(void) { return _RGFW->wl_display; }
struct wl_surface* RGFW_window_getWindow_Wayland(RGFW_window* win) { return win->src.surface; }


/* wayland global garbage (wayland bad, X11 is fine (ish) (not really)) */
#include "xdg-shell.h"
#include "xdg-toplevel-icon-v1.h"
#include "xdg-decoration-unstable-v1.h"
#include "relative-pointer-unstable-v1.h"
#include "pointer-constraints-unstable-v1.h"
#include "xdg-output-unstable-v1.h"


void RGFW_toggleWaylandMaximized(RGFW_window* win, RGFW_bool maximized);

static void RGFW_wl_setOpaque(RGFW_window* win) {
	struct wl_region* wl_region = wl_compositor_create_region(_RGFW->compositor);

	if (!wl_region) return; /* return if no region was created */

	wl_region_add(wl_region, 0, 0, win->w, win->h);
	wl_surface_set_opaque_region(win->src.surface, wl_region);
	wl_region_destroy(wl_region);

}

static void RGFW_wl_xdg_wm_base_ping_handler(void* data, struct xdg_wm_base* wm_base,
		u32 serial) {
	RGFW_UNUSED(data);
    xdg_wm_base_pong(wm_base, serial);
}
static void RGFW_wl_xdg_surface_configure_handler(void* data, struct xdg_surface* xdg_surface,
		u32 serial) {

    xdg_surface_ack_configure(xdg_surface, serial);

    RGFW_window* win = (RGFW_window*)data;

    if (win == NULL) {
		win = _RGFW->kbOwner;
		if (win == NULL)
			return;
	}

	/* useful for libdecor */
	if (win->src.activated != win->src.pending_activated) {
		win->src.activated = win->src.pending_activated;
	}

	if (win->src.maximized != win->src.pending_maximized) {
		RGFW_toggleWaylandMaximized(win, win->src.pending_maximized);

		RGFW_window_checkMode(win);
	}


	if (win->src.resizing) {

		/* Do not create a resize event if the window is maximized */
		if (!win->src.maximized && win->internal.enabledEvents & RGFW_windowResizedFlag) {
			RGFW_eventQueuePushEx(e.type = RGFW_windowResized; e.common.win = win);
			RGFW_windowResizedCallback(win, win->w, win->h);
		}
		RGFW_window_resize(win, win->w, win->h);
		if (!(win->internal.flags & RGFW_windowTransparent)) {
			RGFW_wl_setOpaque(win);
		}
	}

}

static void RGFW_wl_xdg_toplevel_configure_handler(void* data, struct xdg_toplevel* toplevel,
		i32 width, i32 height, struct wl_array* states) {

    RGFW_UNUSED(toplevel);
    RGFW_window* win = (RGFW_window*)data;


    win->src.pending_activated = RGFW_FALSE;
    win->src.pending_maximized = RGFW_FALSE;
    win->src.resizing = RGFW_FALSE;


	enum xdg_toplevel_state* state;
	wl_array_for_each(state, states) {
		switch (*state) {
			case XDG_TOPLEVEL_STATE_ACTIVATED:
				 win->src.pending_activated = RGFW_TRUE;
				break;
			case XDG_TOPLEVEL_STATE_MAXIMIZED:
				win->src.pending_maximized = RGFW_TRUE;
				break;
			default:
				break;
		}

	}
	/* if width and height are not zero and are not the same as the window */
    /* the window is resizing so update the values */
	if ((width && height) && (win->w != width ||  win->h != height)) {
		win->src.resizing = RGFW_TRUE;
		win->src.w = win->w = width;
		win->src.h = win->h = height;
	}
}

static void RGFW_wl_xdg_toplevel_close_handler(void* data, struct xdg_toplevel *toplevel) {
	RGFW_UNUSED(toplevel);
	RGFW_window* win = (RGFW_window*)data;

	if (!win->internal.shouldClose) {
		RGFW_eventQueuePushEx(e.type = RGFW_quit; e.common.win = win);
		RGFW_window_setShouldClose(win, RGFW_TRUE);
		RGFW_windowQuitCallback(win);
	}
}

static void RGFW_wl_xdg_decoration_configure_handler(void* data,
		struct zxdg_toplevel_decoration_v1* zxdg_toplevel_decoration_v1, u32 mode) {
	RGFW_window* win = (RGFW_window*)data; RGFW_UNUSED(zxdg_toplevel_decoration_v1);

	/* this is expected to run once */
    /* set the decoration mode set by earlier request */
	if (mode != win->src.decoration_mode) {
		win->src.decoration_mode = mode;
	}
}

static void RGFW_wl_shm_format_handler(void* data, struct wl_shm *shm, u32 format) {
	RGFW_UNUSED(data); RGFW_UNUSED(shm); RGFW_UNUSED(format);
}

static void RGFW_wl_relative_pointer_motion(void *data, struct zwp_relative_pointer_v1 *zwp_relative_pointer_v1,
	u32 time_hi, u32 time_lo, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dx_unaccel, wl_fixed_t dy_unaccel) {

	RGFW_UNUSED(zwp_relative_pointer_v1); RGFW_UNUSED(time_hi); RGFW_UNUSED(time_lo);
	RGFW_UNUSED(dx_unaccel); RGFW_UNUSED(dy_unaccel);

	RGFW_info* RGFW = (RGFW_info*)data;
	RGFW_window* win = RGFW->mouseOwner;

	RGFW_ASSERT(win);

	float vecX =  (float)wl_fixed_to_double(dx);
	float vecY = (float)wl_fixed_to_double(dy);

	RGFW_eventQueuePushEx(e.type = RGFW_mousePosChanged;
									e.mouse.x = win->internal.lastMouseX;
									e.mouse.y = win->internal.lastMouseY;
									e.mouse.vecX = vecX;
									e.mouse.vecY = vecY;
									e.common.win = win);

	RGFW->vectorX = vecX;
	RGFW->vectorY = vecY;
	RGFW_mousePosCallback(win, win->internal.lastMouseX, win->internal.lastMouseY, vecX, vecY);
}

static void RGFW_wl_pointer_locked(void *data, struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1) {
	RGFW_UNUSED(zwp_locked_pointer_v1);
	RGFW_info* RGFW = (RGFW_info*)data;
	RGFW_window* win = RGFW->mouseOwner;

	win->internal.lastMouseX = win->w / 2;
	win->internal.lastMouseY = win->h / 2;
	zwp_locked_pointer_v1_set_cursor_position_hint(win->src.locked_pointer, wl_fixed_from_int((win->w / 2)), wl_fixed_from_int((win->h / 2)));
	wl_pointer_set_cursor(RGFW->wl_pointer, RGFW->mouse_enter_serial, NULL, 0, 0); /* draw no cursor */
}

static void RGFW_wl_pointer_enter(void* data, struct wl_pointer* pointer, u32 serial,
		struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	RGFW_info* RGFW = (RGFW_info*)data;
	RGFW_window* win = (RGFW_window*)wl_surface_get_user_data(surface);

	/* save when the pointer is locked or using default cursor */
	RGFW->mouse_enter_serial = serial;
	win->internal.mouseInside = RGFW_TRUE;
	RGFW->windowState.win = win;
	RGFW->windowState.mouseEnter = RGFW_TRUE;

	RGFW->mouseOwner = win;

	/* set the cursor */
	if (win->src.using_custom_cursor) {
		wl_pointer_set_cursor(pointer, serial, win->src.custom_cursor_surface, 0, 0);
	}
	else {
		RGFW_window_setMouseDefault(win);
	}

	if (!(win->internal.enabledEvents & RGFW_mouseEnterFlag)) return;

	i32 x = (i32)wl_fixed_to_double(surface_x);
	i32 y = (i32)wl_fixed_to_double(surface_y);

	RGFW_eventQueuePushEx(e.type = RGFW_mouseEnter;
									e.mouse.x = x;
									e.mouse.y = y;
									e.common.win = win);

	win->internal.lastMouseX = x;
	win->internal.lastMouseY = y;

	RGFW_mouseNotifyCallback(win, x, y, RGFW_TRUE);
}

static void RGFW_wl_pointer_leave(void* data, struct wl_pointer *pointer, u32 serial, struct wl_surface *surface) {
	RGFW_UNUSED(pointer); RGFW_UNUSED(serial);
	RGFW_window* win = (RGFW_window*)wl_surface_get_user_data(surface);
	RGFW_info* RGFW = (RGFW_info*)data;
	if (RGFW->mouseOwner == win)
		RGFW->mouseOwner = NULL;

	win->internal.mouseInside = RGFW_FALSE;
	RGFW->windowState.winLeave = win;
	RGFW->windowState.mouseLeave = RGFW_TRUE;

	if (!(win->internal.enabledEvents & RGFW_mouseLeaveFlag)) return;

	RGFW_eventQueuePushEx(e.type = RGFW_mouseLeave;
									e.mouse.x = win->internal.lastMouseX;
									e.mouse.y = win->internal.lastMouseY;
									e.common.win = win);

	RGFW_mouseNotifyCallback(win, win->internal.lastMouseX, win->internal.lastMouseY, RGFW_FALSE);
}

static void RGFW_wl_pointer_motion(void* data, struct wl_pointer *pointer, u32 time, wl_fixed_t x, wl_fixed_t y) {
	RGFW_UNUSED(pointer); RGFW_UNUSED(time);

	RGFW_info* RGFW = (RGFW_info*)data;
	RGFW_ASSERT(RGFW->mouseOwner != NULL);

	RGFW_window* win = RGFW->mouseOwner;

	if (!(win->internal.enabledEvents & RGFW_mousePosChangedFlag)) return;

	i32 convertedX = (i32)wl_fixed_to_double(x);
	i32 convertedY = (i32)wl_fixed_to_double(y);
	float newVecX = (float)(convertedX - win->internal.lastMouseX);
	float newVecY = (float)(convertedY - win->internal.lastMouseY);

	RGFW_eventQueuePushEx(e.type = RGFW_mousePosChanged;
									e.mouse.x = convertedX;
									e.mouse.y = convertedY;
									e.mouse.vecX = newVecX;
									e.mouse.vecY = newVecY;
									e.common.win = win);

	RGFW->vectorX = newVecX;
	RGFW->vectorY = newVecY;
	win->internal.lastMouseX = convertedX;
	win->internal.lastMouseY = convertedY;
	RGFW_mousePosCallback(win, convertedX, convertedY, newVecX, newVecY);
}

static void RGFW_wl_pointer_button(void* data, struct wl_pointer *pointer, u32 serial, u32 time, u32 button, u32 state) {
	RGFW_UNUSED(pointer); RGFW_UNUSED(time); RGFW_UNUSED(serial);
	RGFW_info* RGFW = (RGFW_info*)data;

	RGFW_ASSERT(RGFW->mouseOwner != NULL);
	RGFW_window* win = RGFW->mouseOwner;

	if (!(win->internal.enabledEvents & (RGFW_BIT(RGFW_mouseButtonReleased - RGFW_BOOL(state))))) return;
	u32 b = (button - 0x110);

	/* flip right and middle button codes */
	if (b == 1) b = 2;
	else if (b == 2) b = 1;

	RGFW->mouseButtons[b].prev = RGFW->mouseButtons[b].current;
	RGFW->mouseButtons[b].current = RGFW_BOOL(state);

	RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonReleased - RGFW_BOOL(state);
									e.button.value = (u8)b;
									e.common.win = win);
	RGFW_mouseButtonCallback(win, (u8)b, RGFW_BOOL(state));
}

static void RGFW_wl_pointer_axis(void* data, struct wl_pointer *pointer, u32 time, u32 axis, wl_fixed_t value) {
	RGFW_UNUSED(pointer); RGFW_UNUSED(time);  RGFW_UNUSED(axis);

	RGFW_info* RGFW = (RGFW_info*)data;
	RGFW_ASSERT(RGFW->mouseOwner != NULL);
	RGFW_window* win = RGFW->mouseOwner;

	float scrollX = 0.0;
	float scrollY = 0.0;

	if (!(win->internal.enabledEvents  & (RGFW_BIT(RGFW_mouseScroll)))) return;

	if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
		scrollX = (float)(-wl_fixed_to_double(value) / 10.0);
	else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
		scrollY = (float)(-wl_fixed_to_double(value) / 10.0);


	RGFW->scrollX = (float)scrollX;
	RGFW->scrollY = (float)scrollY;
	RGFW_mouseScrollCallback(win, scrollX, scrollY);
	RGFW_eventQueuePushEx(e.type = RGFW_mouseScroll;
									e.scroll.x = scrollX;
									e.scroll.y = scrollY;
									e.common.win = win);
}


static void RGFW_doNothing(void) { }

static void RGFW_wl_keyboard_keymap(void* data, struct wl_keyboard *keyboard, u32 format, i32 fd, u32 size) {
	RGFW_UNUSED(keyboard); RGFW_UNUSED(format);
	RGFW_info* RGFW = (RGFW_info*)data;

	char *keymap_string = mmap (NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	xkb_keymap_unref(RGFW->keymap);
	RGFW->keymap = xkb_keymap_new_from_string(RGFW->xkb_context, keymap_string, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);

	munmap(keymap_string, size);
	close(fd);
	xkb_state_unref(RGFW->xkb_state);
	RGFW->xkb_state = xkb_state_new(RGFW->keymap);
}

static void RGFW_wl_keyboard_enter(void* data, struct wl_keyboard *keyboard, u32 serial, struct wl_surface *surface, struct wl_array *keys) {
	RGFW_UNUSED(keyboard); RGFW_UNUSED(keys);

	RGFW_info* RGFW = (RGFW_info*)data;
	RGFW_window* win = (RGFW_window*)wl_surface_get_user_data(surface);
	RGFW->kbOwner = win;

	// this is to prevent race conditions
	if (RGFW->data_device != NULL && win->src.data_source != NULL) {
		wl_data_device_set_selection(RGFW->data_device, win->src.data_source, serial);
	}
	if (!(win->internal.enabledEvents & RGFW_focusInFlag)) return;

	/* is set when RGFW_window_minimize is called; if the minimize button is */
	/* pressed this flag is not set since there is no event to listen for */
	if (win->src.minimized == RGFW_TRUE) win->src.minimized = RGFW_FALSE;

	win->internal.inFocus = RGFW_TRUE;
	RGFW_eventQueuePushEx(e.type = RGFW_focusIn; e.common.win = win);
	RGFW_focusCallback(win, RGFW_TRUE);

	if ((win->internal.holdMouse)) RGFW_window_holdMouse(win);
}

static void RGFW_wl_keyboard_leave(void* data, struct wl_keyboard *keyboard, u32 serial, struct wl_surface *surface) {
	RGFW_UNUSED(keyboard); RGFW_UNUSED(serial);

	RGFW_info* RGFW = (RGFW_info*)data;
	RGFW_window* win = (RGFW_window*)wl_surface_get_user_data(surface);
	if (RGFW->kbOwner == win)
		RGFW->kbOwner = NULL;

	if (!(win->internal.enabledEvents & RGFW_focusOutFlag)) return;

	RGFW_eventQueuePushEx(e.type = RGFW_focusOut; e.common.win = win);
	RGFW_focusCallback(win, RGFW_FALSE);
    RGFW_window_focusLost(win);
}

static void RGFW_wl_keyboard_key(void* data, struct wl_keyboard *keyboard, u32 serial, u32 time, u32 key, u32 state) {
	RGFW_UNUSED(keyboard); RGFW_UNUSED(serial); RGFW_UNUSED(time);

	RGFW_info* RGFW = (RGFW_info*)data;
	if (RGFW->kbOwner == NULL) return;

	RGFW_window *RGFW_key_win = RGFW->kbOwner;
	if (!(RGFW_key_win->internal.enabledEvents & (RGFW_BIT(RGFW_keyPressed + state)))) return;

	xkb_keysym_t keysym = xkb_state_key_get_one_sym(RGFW->xkb_state, key + 8);

	u32 RGFWkey = RGFW_apiKeyToRGFW(key + 8);
	RGFW->keyboard[RGFWkey].prev = RGFW->keyboard[RGFWkey].current;
	RGFW->keyboard[RGFWkey].current = RGFW_BOOL(state);

	RGFW_eventQueuePushEx(e.type = (u8)(RGFW_keyPressed + state);
									e.key.value = (u8)RGFWkey;
									e.key.sym = (u8)keysym;
									e.key.repeat = RGFW_window_isKeyDown(RGFW_key_win, (u8)RGFWkey);
									e.common.win = RGFW_key_win);

	RGFW_updateKeyMods(RGFW_key_win, RGFW_BOOL(xkb_keymap_mod_get_index(RGFW->keymap, "Lock")), RGFW_BOOL(xkb_keymap_mod_get_index(RGFW->keymap, "Mod2")), RGFW_BOOL(xkb_keymap_mod_get_index(RGFW->keymap, "ScrollLock")));
	RGFW_keyCallback(RGFW_key_win, (u8)RGFWkey, (u8)keysym, RGFW_key_win->internal.mod, RGFW_window_isKeyDown(RGFW_key_win, (u8)RGFWkey), RGFW_BOOL(state));
}

static void RGFW_wl_keyboard_modifiers(void* data, struct wl_keyboard *keyboard, u32 serial, u32 mods_depressed, u32 mods_latched, u32 mods_locked, u32 group) {
	RGFW_UNUSED(keyboard); RGFW_UNUSED(serial); RGFW_UNUSED(time);
	RGFW_info* RGFW = (RGFW_info*)data;
	xkb_state_update_mask(RGFW->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

static void RGFW_wl_seat_capabilities(void* data, struct wl_seat *seat, u32 capabilities) {
	RGFW_info* RGFW = (RGFW_info*)data;
    static struct wl_pointer_listener pointer_listener;
	RGFW_MEMSET(&pointer_listener, 0, sizeof(pointer_listener));
	pointer_listener.enter = &RGFW_wl_pointer_enter;
	pointer_listener.leave = &RGFW_wl_pointer_leave;
	pointer_listener.motion = &RGFW_wl_pointer_motion;
	pointer_listener.button = &RGFW_wl_pointer_button;
	pointer_listener.axis = &RGFW_wl_pointer_axis;

	static struct wl_keyboard_listener keyboard_listener;
	RGFW_MEMSET(&keyboard_listener, 0, sizeof(keyboard_listener));
	keyboard_listener.keymap = &RGFW_wl_keyboard_keymap;
	keyboard_listener.enter = &RGFW_wl_keyboard_enter;
	keyboard_listener.leave = &RGFW_wl_keyboard_leave;
	keyboard_listener.key = &RGFW_wl_keyboard_key;
	keyboard_listener.modifiers = &RGFW_wl_keyboard_modifiers;

    if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !RGFW->wl_pointer) {
		RGFW->wl_pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(RGFW->wl_pointer, &pointer_listener, RGFW);
	}
	if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !RGFW->wl_keyboard) {
		RGFW->wl_keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_add_listener(RGFW->wl_keyboard, &keyboard_listener, RGFW);
	}

    if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && RGFW->wl_pointer) {
		wl_pointer_destroy(RGFW->wl_pointer);
	}
	if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && RGFW->wl_keyboard) {
		wl_keyboard_destroy(RGFW->wl_keyboard);
	}
}

static void RGFW_wl_output_set_geometry(void *data, struct wl_output *wl_output,
			 int32_t x, int32_t y, int32_t physical_width, int32_t physical_height,
			 int32_t subpixel, const char *make, const char *model, int32_t transform) {

	RGFW_monitor* monitor = &((RGFW_monitorNode*)data)->mon;
	monitor->x = x;
	monitor->y = y;

	monitor->physW = (float)physical_width / 25.4f;
	monitor->physH = (float)physical_height / 25.4f;

	RGFW_UNUSED(wl_output);
	RGFW_UNUSED(subpixel);
	RGFW_UNUSED(make);
	RGFW_UNUSED(model);
	RGFW_UNUSED(transform);
}

static void RGFW_wl_output_set_mode(void *data, struct wl_output *wl_output, uint32_t flags,
		     int32_t width, int32_t height, int32_t refresh) {

	RGFW_monitor* monitor = &((RGFW_monitorNode*)data)->mon;

	monitor->mode.w = width;
	monitor->mode.h = height;
	monitor->mode.refreshRate = (u32)RGFW_ROUND( ((float)refresh / 1000) );
	RGFW_UNUSED(width);
	RGFW_UNUSED(height);
	RGFW_UNUSED(wl_output);
	RGFW_UNUSED(flags);
}

static void RGFW_wl_output_set_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	/* this is for pixelRatio */
	RGFW_monitor* monitor = &((RGFW_monitorNode*)data)->mon;

	monitor->pixelRatio = (float)factor;
	RGFW_UNUSED(wl_output);
}

static void RGFW_wl_output_set_name(void *data, struct wl_output *wl_output, const char *name) {
	RGFW_monitor* monitor = &((RGFW_monitorNode*)data)->mon;

	RGFW_STRNCPY(monitor->name, name, sizeof(monitor->name) - 1);
	monitor->name[sizeof(monitor->name) - 1] = '\0';

	RGFW_UNUSED(wl_output);

}

static void RGFW_xdg_output_logical_pos(void *data, struct zxdg_output_v1 *zxdg_output_v1, int32_t x, int32_t y) {
	RGFW_monitor* monitor = &((RGFW_monitorNode*)data)->mon;
	monitor->x = x;
	monitor->y = y;
	RGFW_UNUSED(zxdg_output_v1);
}

static void RGFW_xdg_output_logical_size(void *data, struct zxdg_output_v1 *zxdg_output_v1, int32_t width, int32_t height) {
	RGFW_monitor* monitor = &((RGFW_monitorNode*)data)->mon;

	float mon_float_width = (float) monitor->mode.w;
	float mon_float_height = (float) monitor->mode.h;

	monitor->scaleX = (mon_float_width / (float) width);
	monitor->scaleY = (mon_float_height / (float) height);

	/* under xwayland the monitor changes w & h when compositor scales it */
	monitor->mode.w = width;
	monitor->mode.h = height;
	RGFW_UNUSED(zxdg_output_v1);
}

static void RGFW_wl_create_outputs(struct wl_registry *const registry, uint32_t id) {
	struct wl_output *output = wl_registry_bind(registry, id, &wl_output_interface, wl_display_get_version(_RGFW->wl_display) < 4 ? 3 : 4);
	RGFW_monitorNode* node;
	RGFW_monitor mon;

	if (!output) return;

	char RGFW_mon_default_name[10];

	RGFW_SNPRINTF(RGFW_mon_default_name, sizeof(RGFW_mon_default_name), "monitor-%li", _RGFW->monitors.count);
	RGFW_STRNCPY(mon.name, RGFW_mon_default_name, sizeof(mon.name) - 1);
	mon.name[sizeof(mon.name) - 1] = '\0';

	/* set in case compositor does not send one */
    /* or no xdg_output support */
	mon.scaleY = mon.scaleX = mon.pixelRatio = 1.0f;

	node = RGFW_monitors_add(mon);
	if (node == NULL) return;

	node->id = id;
	node->output = output;

	static const struct wl_output_listener wl_output_listener = {
			.geometry = RGFW_wl_output_set_geometry,
			.mode = RGFW_wl_output_set_mode,
			.done = (void (*)(void *,struct wl_output *))&RGFW_doNothing,
			.scale = RGFW_wl_output_set_scale,
			.name = RGFW_wl_output_set_name,
			.description = (void (*)(void *, struct wl_output *, const char *))&RGFW_doNothing
	};

	/* the wl_output will have a reference to the node */
	wl_output_set_user_data(output, node);

	/* pass the monitor so we can access it in the callback functions */
	wl_output_add_listener(output, &wl_output_listener, node);

	if (!_RGFW->xdg_output_manager) return; /* compositor does not support it */

	static const struct zxdg_output_v1_listener xdg_output_listener = {
		.name = (void (*)(void *,struct zxdg_output_v1 *, const char *))&RGFW_doNothing,
		.done = (void (*)(void *,struct zxdg_output_v1 *))&RGFW_doNothing,
		.description = (void (*)(void *,struct zxdg_output_v1 *, const char *))&RGFW_doNothing,
		.logical_position = RGFW_xdg_output_logical_pos,
		.logical_size = RGFW_xdg_output_logical_size
	};

	node->xdg_output = zxdg_output_manager_v1_get_xdg_output(_RGFW->xdg_output_manager, node->output);
	zxdg_output_v1_add_listener(node->xdg_output, &xdg_output_listener, node);
}

static void RGFW_wl_surface_enter(void *data, struct wl_surface *wl_surface, struct wl_output *output) {
	RGFW_UNUSED(wl_surface);

	RGFW_window* win = (RGFW_window*)data;
	RGFW_monitorNode* node = wl_output_get_user_data(output);
	win->src.active_monitor = node->mon;

	#ifndef RGFW_NO_MONITOR
	if (win->internal.flags & RGFW_windowScaleToMonitor)
		RGFW_window_scaleToMonitor(win);
	#endif
}

static void RGFW_wl_data_source_send(void *data, struct wl_data_source *wl_data_source, const char *mime_type, int32_t fd) {
	RGFW_UNUSED(data); RGFW_UNUSED(wl_data_source);

	// a client can accept our clipboard
	if (RGFW_STRNCMP(mime_type, "text/plain;charset=utf-8", 25) == 0) {
		// do not write \0
		write(fd, _RGFW->clipboard, _RGFW->clipboard_len - 1);
	}

	close(fd);
}

static void RGFW_wl_data_source_cancelled(void *data, struct wl_data_source *wl_data_source) {

	RGFW_info* RGFW = (RGFW_info*)data;

	if (RGFW->kbOwner->src.data_source == wl_data_source) {
		RGFW->kbOwner->src.data_source = NULL;
	}

	wl_data_source_destroy(wl_data_source);

}

static void RGFW_wl_data_device_data_offer(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *wl_data_offer) {

	RGFW_UNUSED(data); RGFW_UNUSED(wl_data_device);
	static const struct wl_data_offer_listener wl_data_offer_listener = {
		.offer = (void (*)(void *data, struct wl_data_offer *wl_data_offer, const char *))RGFW_doNothing,
		.source_actions = (void (*)(void *data, struct wl_data_offer *wl_data_offer, uint32_t dnd_action))RGFW_doNothing,
		.action = (void (*)(void *data, struct wl_data_offer *wl_data_offer, uint32_t dnd_action))RGFW_doNothing
	};
	wl_data_offer_add_listener(wl_data_offer, &wl_data_offer_listener, NULL);
}

static void RGFW_wl_data_device_selection(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *wl_data_offer) {
	RGFW_UNUSED(data); RGFW_UNUSED(wl_data_device);
	/* Clipboard is empty */
	if (wl_data_offer == NULL) {
		return;
	}

	int pfds[2];
	pipe(pfds);

	wl_data_offer_receive(wl_data_offer, "text/plain;charset=utf-8", pfds[1]);
	close(pfds[1]);

	wl_display_roundtrip(_RGFW->wl_display);

	char buf[1024];

	ssize_t n = read(pfds[0], buf, sizeof(buf));

	_RGFW->clipboard = (char*)RGFW_ALLOC((size_t)n);
	RGFW_ASSERT(_RGFW->clipboard != NULL);
	RGFW_STRNCPY(_RGFW->clipboard, buf, (size_t)n);

	_RGFW->clipboard_len = (size_t)n + 1;

	close(pfds[0]);

	wl_data_offer_destroy(wl_data_offer);

}

static void RGFW_wl_global_registry_handler(void* data, struct wl_registry *registry, u32 id, const char *interface, u32 version) {

    static struct wl_seat_listener seat_listener = {&RGFW_wl_seat_capabilities, (void (*)(void *, struct wl_seat *, const char *))&RGFW_doNothing};
    static const struct wl_shm_listener shm_listener = { .format = RGFW_wl_shm_format_handler };

    RGFW_info* RGFW = (RGFW_info*)data;
    RGFW_UNUSED(version);

    if (RGFW_STRNCMP(interface, "wl_compositor", 16) == 0) {
		RGFW->compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 4);
	} else if (RGFW_STRNCMP(interface, "xdg_wm_base", 12) == 0) {
		RGFW->xdg_wm_base = wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
	} else if (RGFW_STRNCMP(interface, zxdg_decoration_manager_v1_interface.name, 255) == 0) {
		RGFW->decoration_manager = wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1);
    } else if (RGFW_STRNCMP(interface, zwp_pointer_constraints_v1_interface.name, 255) == 0) {
		RGFW->constraint_manager = wl_registry_bind(registry, id, &zwp_pointer_constraints_v1_interface, 1);
    } else if (RGFW_STRNCMP(interface, zwp_relative_pointer_manager_v1_interface.name, 255) == 0) {
		RGFW->relative_pointer_manager = wl_registry_bind(registry, id, &zwp_relative_pointer_manager_v1_interface, 1);
	} else if (RGFW_STRNCMP(interface, xdg_toplevel_icon_manager_v1_interface.name, 255) == 0) {
		RGFW->icon_manager = wl_registry_bind(registry, id, &xdg_toplevel_icon_manager_v1_interface, 1);
    } else if (RGFW_STRNCMP(interface, "wl_shm", 7) == 0) {
        RGFW->shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
        wl_shm_add_listener(RGFW->shm, &shm_listener, RGFW);
	} else if (RGFW_STRNCMP(interface,"wl_seat", 8) == 0) {
		RGFW->seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
		wl_seat_add_listener(RGFW->seat, &seat_listener, RGFW);
	} else if (RGFW_STRNCMP(interface, zxdg_output_manager_v1_interface.name, 255) == 0) {
		RGFW->xdg_output_manager = wl_registry_bind(registry, id, &zxdg_output_manager_v1_interface, 1);
	} else if (RGFW_STRNCMP(interface,"wl_output", 10) == 0) {
		RGFW_wl_create_outputs(registry, id);
	} else if (RGFW_STRNCMP(interface,"wl_data_device_manager", 23) == 0) {
		RGFW->data_device_manager = wl_registry_bind(registry, id, &wl_data_device_manager_interface, 1);
	}
}

static void RGFW_wl_global_registry_remove(void* data, struct wl_registry *registry, u32 id) {
	RGFW_UNUSED(data); RGFW_UNUSED(registry);
	RGFW_info* RGFW = (RGFW_info*)data;
	RGFW_monitorNode* prev = RGFW->monitors.list.head;
	RGFW_monitorNode* node = NULL;
	if (prev == NULL) return;

	if (prev->id != id) {
		/* find the first node that has a matching id */
		while(prev->next != NULL && prev->next->id != id) {
			prev = prev->next;
		}

		if (prev->next == NULL) return;
		node = prev->next;
	} else {
		node = prev;
	}

	if (node->output) {
		wl_output_destroy(node->output);
	}

	if (node->xdg_output) {
		zxdg_output_v1_destroy(node->xdg_output);
	}

	RGFW_monitors_remove(node, prev);
}

static void RGFW_wl_randname(char *buf) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;

    int i;
    for (i = 0; i < 6; ++i) {
		buf[i] = (char)('A'+(r&15)+(r&16)*2);
		r >>= 5;
	}
}

static size_t RGFW_wl_stringlen(char* name) {
	size_t i = 0;
    while (name[i]) { i++; }
	return i;
}

static int RGFW_wl_anonymous_shm_open(void) {
	char name[] = "/RGFW-wayland-XXXXXX";
	int retries = 100;

	do {
		RGFW_wl_randname(name + RGFW_wl_stringlen(name) - 6);

		--retries;
		/* shm_open guarantees that O_CLOEXEC is set */
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) {
			shm_unlink(name);
			return fd;
		}
	} while (retries > 0 && errno == EEXIST);

	return -1;
}

static int RGFW_wl_create_shm_file(off_t size) {
	int fd = RGFW_wl_anonymous_shm_open();
	if (fd < 0) {
		return fd;
	}

	if (ftruncate(fd, size) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

i32 RGFW_initPlatform_Wayland(void) {
	_RGFW->wl_display = wl_display_connect(NULL);
	if (_RGFW->wl_display == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errWayland,  "Failed to load Wayland display");
		return -1;
	}

	_RGFW->compositor = NULL;
	static const struct wl_registry_listener registry_listener = {
		.global = RGFW_wl_global_registry_handler,
		.global_remove = RGFW_wl_global_registry_remove,
	};

	_RGFW->registry = wl_display_get_registry(_RGFW->wl_display);
	wl_registry_add_listener(_RGFW->registry, &registry_listener, _RGFW);

	wl_display_roundtrip(_RGFW->wl_display); /* bind to globals */

	if (_RGFW->compositor == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errWayland, "Can't find compositor.");
		return 1;
	}

	if (_RGFW->wl_cursor_theme == NULL) {
		_RGFW->wl_cursor_theme = wl_cursor_theme_load(NULL, 24, _RGFW->shm);
		_RGFW->cursor_surface = wl_compositor_create_surface(_RGFW->compositor);
	}

	 u8 RGFW_blk[] = { 0, 0, 0, 0 };
	_RGFW->hiddenMouse = RGFW_loadMouse(RGFW_blk, 1, 1, RGFW_formatRGBA8);

	static const struct xdg_wm_base_listener xdg_wm_base_listener = {
		.ping = RGFW_wl_xdg_wm_base_ping_handler,
	};

	xdg_wm_base_add_listener(_RGFW->xdg_wm_base, &xdg_wm_base_listener, NULL);

	_RGFW->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

	static const struct wl_data_device_listener wl_data_device_listener = {
		.data_offer = RGFW_wl_data_device_data_offer,
		.enter = (void (*)(void *, struct wl_data_device *, u32, struct wl_surface*, wl_fixed_t, wl_fixed_t, struct wl_data_offer *))&RGFW_doNothing,
		.leave = (void (*)(void *, struct wl_data_device *))&RGFW_doNothing,
		.motion = (void (*)(void *, struct wl_data_device *, u32, wl_fixed_t, wl_fixed_t))&RGFW_doNothing,
		.drop = (void (*)(void *, struct wl_data_device *))&RGFW_doNothing,
		.selection = RGFW_wl_data_device_selection
	};

	if (_RGFW->seat && _RGFW->data_device_manager) {
		_RGFW->data_device = wl_data_device_manager_get_data_device(_RGFW->data_device_manager, _RGFW->seat);
		wl_data_device_add_listener(_RGFW->data_device, &wl_data_device_listener, NULL);
	}

	return 0;
}

void RGFW_deinitPlatform_Wayland(void) {
	if (_RGFW->clipboard) {
		RGFW_FREE(_RGFW->clipboard);
		_RGFW->clipboard = NULL;
	}

    if (_RGFW->wl_pointer) {
		wl_pointer_destroy(_RGFW->wl_pointer);
	}
	if (_RGFW->wl_keyboard) {
		wl_keyboard_destroy(_RGFW->wl_keyboard);
	}

	wl_registry_destroy(_RGFW->registry);
	if (_RGFW->decoration_manager != NULL)
		zxdg_decoration_manager_v1_destroy(_RGFW->decoration_manager);
	if (_RGFW->relative_pointer_manager != NULL) {
		zwp_relative_pointer_manager_v1_destroy(_RGFW->relative_pointer_manager);
	}

	if (_RGFW->relative_pointer) {
		zwp_relative_pointer_v1_destroy(_RGFW->relative_pointer);
	}

	if (_RGFW->constraint_manager != NULL) {
		zwp_pointer_constraints_v1_destroy(_RGFW->constraint_manager);
	}

	if (_RGFW->xdg_output_manager != NULL)
	if (_RGFW->icon_manager != NULL) {
		xdg_toplevel_icon_manager_v1_destroy(_RGFW->icon_manager);
	}

	if (_RGFW->xdg_output_manager) {
		zxdg_output_manager_v1_destroy(_RGFW->xdg_output_manager);
	}

	if (_RGFW->data_device_manager) {
		wl_data_device_manager_destroy(_RGFW->data_device_manager);
	}

	if (_RGFW->data_device) {
		wl_data_device_destroy(_RGFW->data_device);
	}

	if (_RGFW->wl_cursor_theme != NULL) {
		wl_cursor_theme_destroy(_RGFW->wl_cursor_theme);
	}

	RGFW_freeMouse(_RGFW->hiddenMouse);

	RGFW_monitorNode* node = _RGFW->monitors.list.head;

	while (node != NULL) {
		if (node->output) {
			wl_output_destroy(node->output);
		}

		if (node->xdg_output) {
			zxdg_output_v1_destroy(node->xdg_output);
		}

		_RGFW->monitors.count -= 1;
		node = node->next;

	}

	wl_surface_destroy(_RGFW->cursor_surface);
	wl_shm_destroy(_RGFW->shm);
	wl_seat_release(_RGFW->seat);
	xdg_wm_base_destroy(_RGFW->xdg_wm_base);
	wl_compositor_destroy(_RGFW->compositor);
	wl_display_disconnect(_RGFW->wl_display);
}

RGFW_bool RGFW_FUNC(RGFW_createSurfacePtr) (u8* data, i32 w, i32 h, RGFW_format format, RGFW_surface* surface) {
	RGFW_ASSERT(surface != NULL);
	surface->data = data;
	surface->w = w;
	surface->h = h;
	surface->format = format;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoBuffer,  "Creating a 4 channel buffer");

	u32 size = (u32)(surface->w * surface->h * 4);
	int fd = RGFW_wl_create_shm_file(size);
	if (fd < 0) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errBuffer, "Failed to create a buffer.");
		return RGFW_FALSE;
	}

	surface->native.buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (surface->native.buffer == MAP_FAILED) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errBuffer, "mmap failed.");
		return RGFW_FALSE;
	}

	struct wl_shm_pool* pool = wl_shm_create_pool(_RGFW->shm, fd, (i32)size);
	surface->native.wl_buffer = wl_shm_pool_create_buffer(pool, 0, (i32)surface->w, (i32)surface->h, (i32)surface->w * 4, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);

	close(fd);

	surface->native.format = RGFW_formatBGRA8;
	return RGFW_TRUE;
}

void RGFW_FUNC(RGFW_window_blitSurface) (RGFW_window* win, RGFW_surface* surface) {
	RGFW_ASSERT(surface != NULL);
	RGFW_copyImageData(surface->native.buffer, win->w, RGFW_MIN(win->h, surface->h), surface->native.format, surface->data, surface->format);

	wl_surface_attach(win->src.surface, surface->native.wl_buffer, 0, 0);
	wl_surface_damage(win->src.surface, 0, 0, RGFW_MIN(win->w, surface->w), RGFW_MIN(win->h, surface->h));
	wl_surface_commit(win->src.surface);
}

void RGFW_FUNC(RGFW_surface_freePtr) (RGFW_surface* surface) {
	RGFW_ASSERT(surface != NULL);
	wl_buffer_destroy(surface->native.wl_buffer);
	munmap(surface->native.buffer, (size_t)(surface->w * surface->h * 4));
}

void RGFW_FUNC(RGFW_window_setBorder) (RGFW_window* win, RGFW_bool border) {
	RGFW_setBit(&win->internal.flags, RGFW_windowNoBorder, !border);

	/* for now just toggle between SSD & CSD depending on the bool */
	if (_RGFW->decoration_manager != NULL) {
		zxdg_toplevel_decoration_v1_set_mode(win->src.decoration, (border ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE : ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE));
	}
}

void RGFW_FUNC(RGFW_releaseCursor) (RGFW_window* win) {
    RGFW_ASSERT(win);
    /* compositor has no support or window is not locked do nothing */
    if (_RGFW->constraint_manager == NULL || _RGFW->relative_pointer_manager == NULL) return;

    if (win->src.locked_pointer != NULL) {
		zwp_locked_pointer_v1_destroy(win->src.locked_pointer);
		win->src.locked_pointer = NULL;
    }
    if (_RGFW->relative_pointer != NULL) {
		zwp_relative_pointer_v1_destroy(_RGFW->relative_pointer);
		_RGFW->relative_pointer = NULL;
    }

    _RGFW->mouseOwner = win; /* unhold mouse sets this to null; set it back */
}

void RGFW_FUNC(RGFW_captureCursor) (RGFW_window* win) {
	RGFW_ASSERT(win);
	/* compositor has no support or window already is locked do nothing */
	if (_RGFW->constraint_manager == NULL || _RGFW->relative_pointer_manager == NULL) return;

	if (_RGFW->relative_pointer == NULL) {
		_RGFW->relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(_RGFW->relative_pointer_manager, _RGFW->wl_pointer);

		static const struct zwp_relative_pointer_v1_listener relative_motion_listener = {
			.relative_motion = RGFW_wl_relative_pointer_motion
		};

		zwp_relative_pointer_v1_add_listener(_RGFW->relative_pointer, &relative_motion_listener, _RGFW);
	}

	if (win->src.locked_pointer == NULL) {
		win->src.locked_pointer = zwp_pointer_constraints_v1_lock_pointer(_RGFW->constraint_manager, win->src.surface, _RGFW->wl_pointer, NULL, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);

		static const struct zwp_locked_pointer_v1_listener locked_listener = {
			.locked = RGFW_wl_pointer_locked,
			.unlocked = (void (*)(void *, struct zwp_locked_pointer_v1 *))RGFW_doNothing
		};

		zwp_locked_pointer_v1_add_listener(win->src.locked_pointer, &locked_listener, _RGFW);
	}
}

RGFW_window* RGFW_FUNC(RGFW_createWindowPlatform) (const char* name, RGFW_windowFlags flags, RGFW_window* win) {
	RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningWayland, "RGFW Wayland support is experimental");

	static const struct xdg_surface_listener xdg_surface_listener = {
		.configure = RGFW_wl_xdg_surface_configure_handler,
	};

	static const struct wl_surface_listener wl_surface_listener = {
		.enter = RGFW_wl_surface_enter,
		.leave = (void (*)(void *, struct wl_surface *, struct wl_output *))&RGFW_doNothing,
		.preferred_buffer_scale = (void (*)(void *, struct wl_surface *, i32))&RGFW_doNothing,
		.preferred_buffer_transform = (void (*)(void *, struct wl_surface *, u32))&RGFW_doNothing
	};

	win->src.surface = wl_compositor_create_surface(_RGFW->compositor);
	wl_surface_add_listener(win->src.surface, &wl_surface_listener, win);

	/* create a surface for a custom cursor */
	win->src.custom_cursor_surface = wl_compositor_create_surface(_RGFW->compositor);

	win->src.xdg_surface = xdg_wm_base_get_xdg_surface(_RGFW->xdg_wm_base, win->src.surface);
	xdg_surface_add_listener(win->src.xdg_surface, &xdg_surface_listener, win);

	xdg_wm_base_set_user_data(_RGFW->xdg_wm_base, win);

	win->src.xdg_toplevel = xdg_surface_get_toplevel(win->src.xdg_surface);

	xdg_surface_set_window_geometry(win->src.xdg_surface, 0, 0, win->w, win->h);

	if (!(win->internal.flags & RGFW_windowTransparent)) { /* no transparency */
		RGFW_wl_setOpaque(win);
	}

	static const struct xdg_toplevel_listener xdg_toplevel_listener = {
		.configure = RGFW_wl_xdg_toplevel_configure_handler,
		.close = RGFW_wl_xdg_toplevel_close_handler,
	};

	xdg_toplevel_add_listener(win->src.xdg_toplevel, &xdg_toplevel_listener, win);

	/* compositor supports both SSD & CSD
	   So choose accordingly
	 */
	if (_RGFW->decoration_manager) {
		u32 decoration_mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
		win->src.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
			_RGFW->decoration_manager, win->src.xdg_toplevel);

		static const struct zxdg_toplevel_decoration_v1_listener xdg_decoration_listener = {
			.configure = RGFW_wl_xdg_decoration_configure_handler
		};

		zxdg_toplevel_decoration_v1_add_listener(win->src.decoration, &xdg_decoration_listener, win);

		/* we want no decorations */
		if ((flags & RGFW_windowNoBorder)) {
			decoration_mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
		}

		zxdg_toplevel_decoration_v1_set_mode(win->src.decoration, decoration_mode);

        /* no xdg_decoration support */
	} else if (!(flags & RGFW_windowNoBorder)) {
		/* TODO, some fallback */
		#ifdef RGFW_LIBDECOR
			static struct libdecor_interface interface = {
				.error = NULL,
			};

			static struct libdecor_frame_interface frameInterface = {0}; /*= {
				RGFW_wl_handle_configure,
				RGFW_wl_handle_close,
				RGFW_wl_handle_commit,
				RGFW_wl_handle_dismiss_popup,
			};*/

			win->src.decorContext = libdecor_new(_RGFW->wl_display, &interface);
			if (win->src.decorContext) {
				struct libdecor_frame *frame = libdecor_decorate(win->src.decorContext, win->src.surface, &frameInterface, win);
				if (!frame) {
					libdecor_unref(win->src.decorContext);
					win->src.decorContext = NULL;
				} else {
					libdecor_frame_set_app_id(frame, "my-libdecor-app");
					libdecor_frame_set_title(frame, "My Libdecor Window");
				}
			}
		#endif
	}

	if (_RGFW->icon_manager != NULL) {
		/* set the default wayland icon */
		xdg_toplevel_icon_manager_v1_set_icon(_RGFW->icon_manager, win->src.xdg_toplevel, NULL);
	}

	wl_surface_commit(win->src.surface);
	wl_display_dispatch(_RGFW->wl_display);
	RGFW_UNUSED(name);

	return win;
}

RGFW_bool RGFW_FUNC(RGFW_getGlobalMouse) (i32* x, i32* y) {
	RGFW_init();
	if (x) *x = 0;
	if (y) *y = 0;
	return RGFW_FALSE;
}

u8 RGFW_FUNC(RGFW_rgfwToKeyChar)(u32 key) {
    return (u8)key;
}

void RGFW_FUNC(RGFW_pollEvents) (void) {
	RGFW_resetPrevState();

	/* send buffered requests to compositor */
	while (wl_display_flush(_RGFW->wl_display) == -1) {
		/* compositor not responding to new requests */
		/* so let's dispatch some events so the compositor responds */
		if (errno == EAGAIN) {
			if (wl_display_dispatch_pending(_RGFW->wl_display) == -1) {
				return;
			}
		} else {
			return;
		}
	}

	/* read the events; if empty this reads from the */
	/* wayland file descriptor */
	if (wl_display_dispatch(_RGFW->wl_display) == -1) {
		return;
	}

}

void RGFW_FUNC(RGFW_window_move) (RGFW_window* win, i32 x, i32 y) {
	RGFW_ASSERT(win != NULL);
	win->x = x;
	win->y = y;
}


void RGFW_FUNC(RGFW_window_resize) (RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);
	win->w = w;
	win->h = h;
	if (_RGFW->compositor) {
		xdg_surface_set_window_geometry(win->src.xdg_surface, 0, 0, win->w, win->h);
		#ifdef RGFW_OPENGL
		if (win->src.ctx.egl)
			wl_egl_window_resize(win->src.ctx.egl->eglWindow, (i32)w, (i32)h, 0, 0);
		#endif
	}
}

void RGFW_FUNC(RGFW_window_setAspectRatio) (RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);

	if (w == 0 && h == 0)
		return;
    xdg_toplevel_set_max_size(win->src.xdg_toplevel, (i32)w, (i32)h);
}

void RGFW_FUNC(RGFW_window_setMinSize) (RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);
    xdg_toplevel_set_min_size(win->src.xdg_toplevel, w, h);
}

void RGFW_FUNC(RGFW_window_setMaxSize) (RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);
    xdg_toplevel_set_max_size(win->src.xdg_toplevel, w, h);
}

void RGFW_toggleWaylandMaximized(RGFW_window* win, RGFW_bool maximized) {
    win->src.maximized = maximized;
    if (maximized) {
		xdg_toplevel_set_maximized(win->src.xdg_toplevel);
    } else {
		xdg_toplevel_unset_maximized(win->src.xdg_toplevel);
    }
}

void RGFW_FUNC(RGFW_window_maximize) (RGFW_window* win) {
	win->internal.oldX = win->x;
	win->internal.oldY = win->y;
	win->internal.oldW = win->w;
	win->internal.oldH = win->h;
    RGFW_toggleWaylandMaximized(win, 1);
    return;
}

void RGFW_FUNC(RGFW_window_focus)(RGFW_window* win) {
	RGFW_ASSERT(win);
}

void RGFW_FUNC(RGFW_window_raise)(RGFW_window* win) {
	RGFW_ASSERT(win);
}

void RGFW_FUNC(RGFW_window_setFullscreen)(RGFW_window* win, RGFW_bool fullscreen) {
	RGFW_ASSERT(win != NULL);
    if (fullscreen) {

		win->internal.flags |= RGFW_windowFullscreen;
		win->internal.oldX = win->x;
		win->internal.oldY = win->y;
		win->internal.oldW = win->w;
		win->internal.oldH = win->h;
		xdg_toplevel_set_fullscreen(win->src.xdg_toplevel, NULL); /* let the compositor decide */
	} else {
		win->internal.flags &= ~(u32)RGFW_windowFullscreen;
		xdg_toplevel_unset_fullscreen(win->src.xdg_toplevel);
	}

}

void RGFW_FUNC(RGFW_window_setFloating) (RGFW_window* win, RGFW_bool floating) {
    RGFW_ASSERT(win != NULL);
	RGFW_UNUSED(floating);
}

void RGFW_FUNC(RGFW_window_setOpacity) (RGFW_window* win, u8 opacity) {
	RGFW_ASSERT(win != NULL);
	RGFW_UNUSED(opacity);
}

void RGFW_FUNC(RGFW_window_minimize)(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	if (RGFW_window_isMaximized(win)) return;
	win->internal.oldX = win->x;
	win->internal.oldY = win->y;
	win->internal.oldW = win->w;
	win->internal.oldH = win->h;
	win->src.minimized = RGFW_TRUE;
	xdg_toplevel_set_minimized(win->src.xdg_toplevel);
}

void RGFW_FUNC(RGFW_window_restore)(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_toggleWaylandMaximized(win, RGFW_FALSE);

	RGFW_window_move(win, win->internal.oldX, win->internal.oldY);
	RGFW_window_resize(win, win->internal.oldW, win->internal.oldH);

	RGFW_window_show(win);
	RGFW_window_move(win, win->internal.oldX, win->internal.oldY);
	RGFW_window_resize(win, win->internal.oldW, win->internal.oldH);

    RGFW_window_show(win);
}

RGFW_bool RGFW_FUNC(RGFW_window_isFloating)(RGFW_window* win) {
	return (!RGFW_window_isFullscreen(win) && !RGFW_window_isMaximized(win));
}

void RGFW_FUNC(RGFW_window_setName) (RGFW_window* win, const char* name) {
	RGFW_ASSERT(win != NULL);
	if (_RGFW->compositor)
		xdg_toplevel_set_title(win->src.xdg_toplevel, name);
}

#ifndef RGFW_NO_PASSTHROUGH
void RGFW_FUNC(RGFW_window_setMousePassthrough) (RGFW_window* win, RGFW_bool passthrough) {
	RGFW_ASSERT(win != NULL);
    RGFW_UNUSED(passthrough);
}
#endif /* RGFW_NO_PASSTHROUGH */

RGFW_bool RGFW_FUNC(RGFW_window_setIconEx) (RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format, RGFW_icon type) {
	RGFW_ASSERT(win != NULL);
	RGFW_UNUSED(type);

	if (_RGFW->icon_manager == NULL || w != h) return RGFW_FALSE;

	if (win->src.icon) {
		xdg_toplevel_icon_v1_destroy(win->src.icon);
		win->src.icon= NULL;
	}

	RGFW_surface* surface = RGFW_createSurface(data, w, h, format);

	if (surface == NULL) return RGFW_FALSE;

	RGFW_copyImageData(surface->native.buffer, RGFW_MIN(w, surface->w), RGFW_MIN(h, surface->h), surface->native.format, surface->data, surface->format);

	win->src.icon = xdg_toplevel_icon_manager_v1_create_icon(_RGFW->icon_manager);
	xdg_toplevel_icon_v1_add_buffer(win->src.icon, surface->native.wl_buffer, 1);
	xdg_toplevel_icon_manager_v1_set_icon(_RGFW->icon_manager, win->src.xdg_toplevel, win->src.icon);

	RGFW_surface_free(surface);
	return RGFW_TRUE;
}

RGFW_mouse* RGFW_FUNC(RGFW_loadMouse)(u8* data, i32 w, i32 h, RGFW_format format) {

    RGFW_surface *mouse_surface = RGFW_createSurface(data, w, h, format);

	if (mouse_surface == NULL) return NULL;

	RGFW_copyImageData(mouse_surface->native.buffer, RGFW_MIN(w, mouse_surface->w), RGFW_MIN(h, mouse_surface->h), mouse_surface->native.format, mouse_surface->data, mouse_surface->format);

	return (void*) mouse_surface;
}

void RGFW_FUNC(RGFW_window_setMouse)(RGFW_window* win, RGFW_mouse* mouse) {
	RGFW_ASSERT(win); RGFW_ASSERT(mouse);
	RGFW_surface *mouse_surface = (RGFW_surface*)mouse;

	win->src.using_custom_cursor = RGFW_TRUE;

	struct wl_buffer *mouse_buffer = mouse_surface->native.wl_buffer;

	wl_surface_attach(win->src.custom_cursor_surface, mouse_buffer, 0, 0);
	wl_surface_damage(win->src.custom_cursor_surface, 0, 0, mouse_surface->w, mouse_surface->h);
	wl_surface_commit(win->src.custom_cursor_surface);

}

void RGFW_FUNC(RGFW_freeMouse)(RGFW_mouse* mouse) {
	if (mouse != NULL) {
		RGFW_surface_free((RGFW_surface*)mouse);
	}
}

void RGFW_FUNC(RGFW_window_moveMouse)(RGFW_window* win, i32 x, i32 y) {
    RGFW_UNUSED(win); RGFW_UNUSED(x); RGFW_UNUSED(y);
}

RGFW_bool RGFW_FUNC(RGFW_window_setMouseDefault)(RGFW_window* win) {
	return RGFW_window_setMouseStandard(win, RGFW_mouseArrow);
}

RGFW_bool RGFW_FUNC(RGFW_window_setMouseStandard)(RGFW_window* win, u8 mouse) {
	RGFW_ASSERT(win != NULL);
	static const char* iconStrings[16] = { "arrow", "left_ptr", "xterm", "crosshair", "hand2", "sb_h_double_arrow", "sb_v_double_arrow", "bottom_left_corner", "bottom_right_corner", "fleur", "forbidden" };

	win->src.using_custom_cursor = RGFW_FALSE;

	if (mouse > RGFW_mouseIconCount - 1) return RGFW_FALSE;

	struct wl_cursor* wlcursor = wl_cursor_theme_get_cursor(_RGFW->wl_cursor_theme, iconStrings[mouse]);
	struct wl_cursor_image* cursor_image = wlcursor->images[0];
	struct wl_buffer* cursor_buffer = wl_cursor_image_get_buffer(cursor_image);
	wl_pointer_set_cursor(_RGFW->wl_pointer, _RGFW->mouse_enter_serial, _RGFW->cursor_surface, (i32)cursor_image->hotspot_x, (i32)cursor_image->hotspot_y);
	wl_surface_attach(_RGFW->cursor_surface, cursor_buffer, 0, 0);
	wl_surface_damage(_RGFW->cursor_surface, 0, 0, (i32)cursor_image->width, (i32)cursor_image->height);
	wl_surface_commit(_RGFW->cursor_surface);
	return RGFW_TRUE;
}

void RGFW_FUNC(RGFW_window_hide) (RGFW_window* win) {
	wl_surface_attach(win->src.surface, NULL, 0, 0);
	wl_surface_commit(win->src.surface);
	win->internal.flags |= RGFW_windowHide;
}

void RGFW_FUNC(RGFW_window_show) (RGFW_window* win) {
	win->internal.flags &= ~(u32)RGFW_windowHide;
	if (win->internal.flags & RGFW_windowFocusOnShow) RGFW_window_focus(win);
	/* wl_surface_attach(win->src.surface, win->x, win->y, win->w, win->h, 0, 0); */
	wl_surface_commit(win->src.surface);
}

RGFW_ssize_t RGFW_FUNC(RGFW_readClipboardPtr) (char* str, size_t strCapacity) {

	RGFW_UNUSED(strCapacity);

	if (str != NULL)
		RGFW_STRNCPY(str, _RGFW->clipboard, _RGFW->clipboard_len - 1);
	_RGFW->clipboard[_RGFW->clipboard_len - 1] = '\0';
	return (RGFW_ssize_t)_RGFW->clipboard_len - 1;
}

void RGFW_FUNC(RGFW_writeClipboard) (const char* text, u32 textLen) {

	// compositor does not support wl_data_device_manager
	// clients cannot read rgfw's clipboard
	if (_RGFW->data_device_manager == NULL) return;
	// clear the clipboard
	if (_RGFW->clipboard)
		RGFW_FREE(_RGFW->clipboard);

	// set the contents
	_RGFW->clipboard = (char*)RGFW_ALLOC(textLen);
	RGFW_ASSERT(_RGFW->clipboard != NULL);
	RGFW_STRNCPY(_RGFW->clipboard, text, textLen - 1);
	_RGFW->clipboard[textLen - 1] = '\0';
	_RGFW->clipboard_len = textLen;

	// means we already wrote to the clipboard
	// so destroy it to create a new one
	RGFW_window* win = _RGFW->kbOwner;

	if (win->src.data_source != NULL) {
		wl_data_source_destroy(win->src.data_source);
		win->src.data_source = NULL;
	}

	// advertise to other clients that we offer text
	win->src.data_source = wl_data_device_manager_create_data_source(_RGFW->data_device_manager);

	// basic error checking
	if (win->src.data_source == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errClipboard, "Could not create clipboard data source");
		return;
	}
	wl_data_source_offer(win->src.data_source , "text/plain;charset=utf-8");

	// needed RGFW_doNothing because wayland will call the functions
	// if not set they are random data that lead to a crash
	static const struct wl_data_source_listener data_source_listener = {
		.target = (void (*)(void *, struct wl_data_source *, const char *))&RGFW_doNothing,
		.action = (void (*)(void *, struct wl_data_source *, u32))&RGFW_doNothing,
		.dnd_drop_performed = (void (*)(void *, struct wl_data_source *))&RGFW_doNothing,
		.dnd_finished = (void (*)(void *, struct wl_data_source *))&RGFW_doNothing,
		.send = RGFW_wl_data_source_send,
		.cancelled = RGFW_wl_data_source_cancelled
	};

	wl_data_source_add_listener(win->src.data_source, &data_source_listener, _RGFW);

}

RGFW_bool RGFW_FUNC(RGFW_window_isHidden) (RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
    return RGFW_FALSE;
}

RGFW_bool RGFW_FUNC(RGFW_window_isMinimized) (RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
    return win->src.minimized;
}

RGFW_bool RGFW_FUNC(RGFW_window_isMaximized) (RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	return win->src.maximized;
}

RGFW_monitor* RGFW_FUNC(RGFW_getMonitors) (size_t* len) {
	static RGFW_monitor monitors[RGFW_MAX_MONITORS];
	RGFW_init();
	if (len != NULL) {
		*len = _RGFW->monitors.count;
	}

	u8 i = 0;
	RGFW_monitorNode* cur_node = _RGFW->monitors.list.head;
	while (cur_node != NULL) {
		monitors[i] = cur_node->mon;
		++i;
		cur_node = cur_node->next;
	}
    return monitors;
}

RGFW_monitor RGFW_FUNC(RGFW_getPrimaryMonitor) (void) {
	return _RGFW->monitors.list.head->mon;
}

RGFW_bool RGFW_FUNC(RGFW_monitor_requestMode) (RGFW_monitor mon, RGFW_monitorMode mode, RGFW_modeRequest request) {
	RGFW_UNUSED(mon); RGFW_UNUSED(mode); RGFW_UNUSED(request);
	return RGFW_FALSE;
}

RGFW_monitor RGFW_FUNC(RGFW_window_getMonitor) (RGFW_window* win) {
	RGFW_ASSERT(win);
	return win->src.active_monitor;
}

#ifdef RGFW_OPENGL
RGFW_bool RGFW_FUNC(RGFW_extensionSupportedPlatform_OpenGL) (const char * extension, size_t len) { return RGFW_extensionSupportedPlatform_EGL(extension, len); }
RGFW_proc RGFW_FUNC(RGFW_getProcAddress_OpenGL) (const char* procname) { return RGFW_getProcAddress_EGL(procname); }


RGFW_bool RGFW_FUNC(RGFW_window_createContextPtr_OpenGL)(RGFW_window* win, RGFW_glContext* ctx, RGFW_glHints* hints) {
	RGFW_bool out = RGFW_window_createContextPtr_EGL(win, &ctx->egl, hints);
	win->src.gfxType = RGFW_gfxNativeOpenGL;
	return out;
}
void RGFW_FUNC(RGFW_window_deleteContextPtr_OpenGL) (RGFW_window* win, RGFW_glContext* ctx) { RGFW_window_deleteContextPtr_EGL(win, &ctx->egl); win->src.ctx.native = NULL; }

void RGFW_FUNC(RGFW_window_makeCurrentContext_OpenGL) (RGFW_window* win) { RGFW_window_makeCurrentContext_EGL(win); }
void* RGFW_FUNC(RGFW_getCurrentContext_OpenGL) (void) { return RGFW_getCurrentContext_EGL(); }
void RGFW_FUNC(RGFW_window_swapBuffers_OpenGL) (RGFW_window* win) { RGFW_window_swapBuffers_EGL(win); }
void RGFW_FUNC(RGFW_window_swapInterval_OpenGL) (RGFW_window* win, i32 swapInterval) { RGFW_window_swapInterval_EGL(win, swapInterval); }
#endif /* RGFW_OPENGL */

void RGFW_FUNC(RGFW_window_closePlatform)(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, "a window was freed");
	#ifdef RGFW_LIBDECOR
		if (win->src.decorContext)
			libdecor_unref(win->src.decorContext);
	#endif

	if (win->src.decoration) {
		zxdg_toplevel_decoration_v1_destroy(win->src.decoration);
	}

	if (win->src.xdg_toplevel) {
		xdg_toplevel_destroy(win->src.xdg_toplevel);
	}

	wl_surface_destroy(win->src.custom_cursor_surface);

	if (win->src.locked_pointer) {
		zwp_locked_pointer_v1_destroy(win->src.locked_pointer);
	}

	if (win->src.icon) {
		xdg_toplevel_icon_v1_destroy(win->src.icon);
	}

	xdg_surface_destroy(win->src.xdg_surface);
	wl_surface_destroy(win->src.surface);
}

#ifdef RGFW_WEBGPU
WGPUSurface RGFW_FUNC(RGFW_window_createSurface_WebGPU) (RGFW_window* window, WGPUInstance instance) {
	WGPUSurfaceDescriptor surfaceDesc = {0};
	WGPUSurfaceSourceWaylandSurface fromWl = {0};
	fromWl.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
	fromWl.display = _RGFW->wl_display;
	fromWl.surface = window->src.surface;

	surfaceDesc.nextInChain = (WGPUChainedStruct*)&fromWl.chain;
	return wgpuInstanceCreateSurface(instance, &surfaceDesc);
}
#endif



#endif /* RGFW_WAYLAND */
/*
	End of Wayland defines
*/

/*

	Start of Windows defines


*/

#ifdef RGFW_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE
#include <windows.h>

#ifndef OCR_NORMAL
#define OCR_NORMAL 32512
#define OCR_IBEAM 32513
#define OCR_WAIT 32514
#define OCR_CROSS 32515
#define OCR_UP 32516
#define OCR_SIZENWSE 32642
#define OCR_SIZENESW 32643
#define OCR_SIZEWE 32644
#define OCR_SIZENS 32645
#define OCR_SIZEALL 32646
#define OCR_NO 32648
#define OCR_HAND 32649
#define OCR_APPSTARTING 32650
#endif

#include <windowsx.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include <wchar.h>
#include <locale.h>
#include <winuser.h>

#ifndef WM_DPICHANGED
#define WM_DPICHANGED       0x02E0
#endif

RGFW_bool RGFW_createUTF8FromWideStringWin32(const WCHAR* source, char* out, size_t max);

#define GL_FRONT				0x0404
#define GL_BACK					0x0405
#define GL_LEFT					0x0406
#define GL_RIGHT				0x0407

typedef int (*PFN_wglGetSwapIntervalEXT)(void);
PFN_wglGetSwapIntervalEXT wglGetSwapIntervalEXTSrc = NULL;
#define wglGetSwapIntervalEXT wglGetSwapIntervalEXTSrc

/* these two wgl functions need to be preloaded */
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hdc, HGLRC hglrc, const int *attribList);
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;

HMODULE RGFW_wgl_dll = NULL;

#ifndef RGFW_NO_LOAD_WGL
	typedef HGLRC(WINAPI* PFN_wglCreateContext)(HDC);
	typedef BOOL(WINAPI* PFN_wglDeleteContext)(HGLRC);
	typedef PROC(WINAPI* PFN_wglGetProcAddress)(LPCSTR);
	typedef BOOL(WINAPI* PFN_wglMakeCurrent)(HDC, HGLRC);
	typedef HDC(WINAPI* PFN_wglGetCurrentDC)(void);
	typedef HGLRC(WINAPI* PFN_wglGetCurrentContext)(void);
	typedef BOOL(WINAPI* PFN_wglShareLists)(HGLRC, HGLRC);

	PFN_wglCreateContext wglCreateContextSRC;
	PFN_wglDeleteContext wglDeleteContextSRC;
	PFN_wglGetProcAddress wglGetProcAddressSRC;
	PFN_wglMakeCurrent wglMakeCurrentSRC;
	PFN_wglGetCurrentDC wglGetCurrentDCSRC;
	PFN_wglGetCurrentContext wglGetCurrentContextSRC;
	PFN_wglShareLists wglShareListsSRC;

	#define wglCreateContext wglCreateContextSRC
	#define wglDeleteContext wglDeleteContextSRC
	#define wglGetProcAddress wglGetProcAddressSRC
	#define wglMakeCurrent wglMakeCurrentSRC
	#define wglGetCurrentDC wglGetCurrentDCSRC
	#define wglGetCurrentContext wglGetCurrentContextSRC
	#define wglShareLists wglShareListsSRC
#endif

void* RGFW_window_getHWND(RGFW_window* win) { return win->src.window; }
void* RGFW_window_getHDC(RGFW_window* win) { return win->src.hdc; }

#ifdef RGFW_OPENGL
RGFWDEF void RGFW_win32_loadOpenGLFuncs(HWND dummyWin);

typedef HRESULT (APIENTRY* PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;

typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALEXTPROC)(int interval);
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
#endif

#ifndef RGFW_NO_DWM
HMODULE RGFW_dwm_dll = NULL;
#ifndef _DWMAPI_H_
typedef struct { DWORD dwFlags; int fEnable; HRGN hRgnBlur; int fTransitionOnMaximized;} DWM_BLURBEHIND;
#endif
typedef HRESULT (WINAPI * PFN_DwmEnableBlurBehindWindow)(HWND, const DWM_BLURBEHIND*);
PFN_DwmEnableBlurBehindWindow DwmEnableBlurBehindWindowSRC = NULL;
#endif
void RGFW_win32_makeWindowTransparent(RGFW_window* win);
void RGFW_win32_makeWindowTransparent(RGFW_window* win) {
	if (!(win->internal.flags & RGFW_windowTransparent)) return;

	#ifndef RGFW_NO_DWM
	if (DwmEnableBlurBehindWindowSRC != NULL) {
			DWM_BLURBEHIND bb = {0, 0, 0, 0};
			bb.dwFlags = 0x1;
			bb.fEnable = TRUE;
			bb.hRgnBlur = NULL;
			DwmEnableBlurBehindWindowSRC(win->src.window, &bb);

	} else
	#endif
	{
		SetWindowLong(win->src.window, GWL_EXSTYLE, WS_EX_LAYERED);
		SetLayeredWindowAttributes(win->src.window, 0, 128,  LWA_ALPHA);
	}
}

LRESULT CALLBACK WndProcW(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcW(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    RGFW_window* win = (RGFW_window*)GetPropW(hWnd, L"RGFW");
	if (win == NULL) return DefWindowProcW(hWnd, message, wParam, lParam);

	static BYTE keyboardState[256];
	GetKeyboardState(keyboardState);

	RGFW_event event;
	RGFW_MEMSET(&event, 0, sizeof(event));
	event.common.win = win;

	RECT windowRect;
	GetWindowRect(hWnd, &windowRect);

	switch (message) {
		case WM_CLOSE:
		case WM_QUIT:
			RGFW_window_setShouldClose(win, RGFW_TRUE);
			RGFW_windowQuitCallback(win);
			RGFW_eventQueuePushEx(e.type = RGFW_quit; e.common.win = win);
			return 0;
		case WM_ACTIVATE: {
			RGFW_bool inFocus = RGFW_BOOL(LOWORD(wParam) != WA_INACTIVE);

			win->internal.inFocus = RGFW_BOOL(inFocus);
			if ((win->internal.enabledEvents & (RGFW_BIT(RGFW_focusIn - inFocus)))) {
				RGFW_eventQueuePushEx(e.type = (RGFW_eventType)((u8)RGFW_focusOut - inFocus); e.common.win = win);
				RGFW_focusCallback(win, inFocus);
			}
            if (inFocus == RGFW_FALSE) RGFW_window_focusLost(win);
			if ((win->internal.flags & RGFW_windowFullscreen) && inFocus == RGFW_TRUE)
                RGFW_window_setFullscreen(win, 1);
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		case WM_MOVE:
			win->x = windowRect.left;
			win->y = windowRect.top;

			if (!(win->internal.enabledEvents & RGFW_windowMovedFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);;
			RGFW_eventQueuePushEx(e.type = RGFW_windowMoved; e.common.win = win);
			RGFW_windowMovedCallback(win, win->x, win->y);
			return DefWindowProcW(hWnd, message, wParam, lParam);
		case WM_SIZE: {
			if (win->src.aspectRatioW != 0 && win->src.aspectRatioH != 0) {
				double aspectRatio = (double)win->src.aspectRatioW / win->src.aspectRatioH;

				int width = windowRect.right - windowRect.left;
				int height = windowRect.bottom - windowRect.top;
				int newHeight = (int)(width / aspectRatio);
				int newWidth = (int)(height * aspectRatio);

				if (win->w > (i32)((windowRect.right - windowRect.left) - win->src.offsetW)  ||
					win->h > (i32)((windowRect.bottom - windowRect.top) - win->src.offsetH))
				{
					if (newHeight > height) windowRect.right = windowRect.left + newWidth;
					else windowRect.bottom = windowRect.top + newHeight;
				} else {
					if (newHeight < height) windowRect.right = windowRect.left + newWidth;
					else windowRect.bottom = windowRect.top + newHeight;
				}

				RGFW_window_resize(win, (windowRect.right - windowRect.left) - win->src.offsetW,
												(windowRect.bottom - windowRect.top) - win->src.offsetH);
			}

			win->w = (windowRect.right - windowRect.left) - (i32)win->src.offsetW;
			win->h = (windowRect.bottom - windowRect.top) - (i32)win->src.offsetH;
			if (!(win->internal.enabledEvents & RGFW_windowResizedFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);;
			RGFW_eventQueuePushEx(e.type = RGFW_windowResized; e.common.win = win);
			RGFW_windowResizedCallback(win, win->w, win->h);
			RGFW_window_checkMode(win);
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		#ifndef RGFW_NO_MONITOR
		case WM_DPICHANGED: {
			if (win->internal.flags & RGFW_windowScaleToMonitor) RGFW_window_scaleToMonitor(win);

			const float scaleX = HIWORD(wParam) / (float) 96;
            const float scaleY = LOWORD(wParam) / (float) 96;

			if (!(win->internal.enabledEvents & RGFW_scaleUpdatedFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);;
			RGFW_scaleUpdatedCallback(win, scaleX, scaleY);
			RGFW_eventQueuePushEx(e.type = RGFW_scaleUpdated; e.scale.x = scaleX; e.scale.y = scaleY; e.common.win = win);
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		#endif
		case WM_GETMINMAXINFO: {
			MINMAXINFO* mmi = (MINMAXINFO*) lParam;
			mmi->ptMinTrackSize.x = (LONG)(win->src.minSizeW + win->src.offsetW);
			mmi->ptMinTrackSize.y = (LONG)(win->src.minSizeH + win->src.offsetH);
			if (win->src.maxSizeW == 0 && win->src.maxSizeH == 0)
				return DefWindowProcW(hWnd, message, wParam, lParam);

			mmi->ptMaxTrackSize.x = (LONG)(win->src.maxSizeW + win->src.offsetW);
			mmi->ptMaxTrackSize.y = (LONG)(win->src.maxSizeH + win->src.offsetH);
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		case WM_PAINT: {
			if (!(win->internal.enabledEvents & RGFW_windowRefreshFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            RGFW_eventQueuePushEx(e.type = RGFW_windowRefresh; e.common.win = win);
            RGFW_windowRefreshCallback(win);
            EndPaint(hWnd, &ps);

            return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		#if(_WIN32_WINNT >= 0x0600)
		case WM_DWMCOMPOSITIONCHANGED:
		case WM_DWMCOLORIZATIONCOLORCHANGED:
			RGFW_win32_makeWindowTransparent(win);
			break;
		#endif
/* based on sokol_app.h */
#ifdef RGFW_ADVANCED_SMOOTH_RESIZE
        case WM_ENTERSIZEMOVE: SetTimer(win->src.window, 1, USER_TIMER_MINIMUM, NULL); break;
        case WM_EXITSIZEMOVE: KillTimer(win->src.window, 1); break;
        case WM_TIMER:
			if (!(win->internal.enabledEvents & RGFW_windowRefreshFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);
			RGFW_windowRefreshCallback(win); break;
#endif
        case WM_NCLBUTTONDOWN: {
            /* workaround for half-second pause when starting to move window
                see: https://gamedev.net/forums/topic/672094-keeping-things-moving-during-win32-moveresize-events/5254386/
            */
            POINT point = { 0, 0 };
            if (SendMessage(win->src.window, WM_NCHITTEST, wParam, lParam) != HTCAPTION || GetCursorPos(&point) == FALSE)
                break;

            ScreenToClient(win->src.window, &point);
            PostMessage(win->src.window, WM_MOUSEMOVE, 0, (u32)(point.x)|((u32)(point.y) << 16));
            break;
        }
		case WM_MOUSELEAVE:
			win->internal.mouseInside = RGFW_FALSE;
			_RGFW->windowState.winLeave = win;
			_RGFW->windowState.mouseLeave = RGFW_TRUE;
			if (!(win->internal.enabledEvents & RGFW_mouseLeaveFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);
			event.type = RGFW_mouseLeave;
			RGFW_window_getMouse(win, &event.mouse.x, &event.mouse.y);
			RGFW_mouseNotifyCallback(win, event.mouse.x, event.mouse.y, 0);
			break;
		case WM_SYSKEYUP: case WM_KEYUP: {
			if (!(win->internal.enabledEvents & RGFW_keyReleasedFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);
			i32 scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
			if (scancode == 0)
				scancode = (i32)MapVirtualKeyW((UINT)wParam, MAPVK_VK_TO_VSC);

			switch (scancode) {
				case 0x54: scancode = 0x137; break; /*  Alt+PrtS */
				case 0x146: scancode = 0x45; break; /* Ctrl+Pause */
				case 0x136: scancode = 0x36; break; /*  CJK IME sets the extended bit for right Shift */
				default: break;
			}

			event.key.value = (u8)RGFW_apiKeyToRGFW((u32) scancode);

			if (wParam == VK_CONTROL) {
				if (HIWORD(lParam) & KF_EXTENDED)
					event.key.value = RGFW_controlR;
				else event.key.value = RGFW_controlL;
			}

			wchar_t charBuffer;
			ToUnicodeEx((UINT)wParam, (UINT)scancode, keyboardState, (wchar_t*)&charBuffer, 1, 0, NULL);

			event.key.sym = (u8)charBuffer;

			_RGFW->keyboard[event.key.value].prev = _RGFW->keyboard[event.key.value].current;
			event.type = RGFW_keyReleased;
			event.key.repeat = ((lParam & 0x40000000) != 0) || RGFW_window_isKeyDown(win, event.key.value);
			_RGFW->keyboard[event.key.value].current = 0;

			RGFW_updateKeyMods(win, (GetKeyState(VK_CAPITAL) & 0x0001), (GetKeyState(VK_NUMLOCK) & 0x0001), (GetKeyState(VK_SCROLL) & 0x0001));
			event.key.mod = win->internal.mod;

			RGFW_keyCallback(win, event.key.value, event.key.sym, event.key.mod, event.key.repeat,0);
			break;
		}
		case WM_SYSKEYDOWN: case WM_KEYDOWN: {
			if (!(win->internal.enabledEvents & RGFW_keyPressedFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);
            i32 scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
			if (scancode == 0)
				scancode = (i32)MapVirtualKeyW((u32)wParam, MAPVK_VK_TO_VSC);

			switch (scancode) {
				case 0x54: scancode = 0x137; break; /*  Alt+PrtS */
				case 0x146: scancode = 0x45; break; /* Ctrl+Pause */
				case 0x136: scancode = 0x36; break; /*  CJK IME sets the extended bit for right Shift */
				default: break;
			}

			event.key.value = (u8)RGFW_apiKeyToRGFW((u32) scancode);
			if (wParam == VK_CONTROL) {
				if (HIWORD(lParam) & KF_EXTENDED)
					event.key.value = RGFW_controlR;
				else event.key.value = RGFW_controlL;
			}

			wchar_t charBuffer;
			ToUnicodeEx((UINT)wParam, (UINT)scancode, keyboardState, &charBuffer, 1, 0, NULL);
			event.key.sym = (u8)charBuffer;

			_RGFW->keyboard[event.key.value].prev = _RGFW->keyboard[event.key.value].current;
			event.type = RGFW_keyPressed;
			event.key.repeat = ((lParam & 0x40000000) != 0) || RGFW_window_isKeyDown(win, event.key.value);
			_RGFW->keyboard[event.key.value].current = 1;

			RGFW_updateKeyMods(win, (GetKeyState(VK_CAPITAL) & 0x0001), (GetKeyState(VK_NUMLOCK) & 0x0001), (GetKeyState(VK_SCROLL) & 0x0001));
			event.key.mod = win->internal.mod;

			RGFW_keyCallback(win, event.key.value, event.key.sym, event.key.mod, event.key.repeat, 1);
			break;
		}
		case WM_MOUSEMOVE: {
			if (!(win->internal.enabledEvents & RGFW_mousePosChangedFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);
			if ((win->internal.holdMouse))
				break;


			event.mouse.x = GET_X_LPARAM(lParam);
			event.mouse.y = GET_Y_LPARAM(lParam);
			event.mouse.vecX = (float)(event.mouse.x - win->internal.lastMouseX);
			event.mouse.vecY = (float)(event.mouse.y - win->internal.lastMouseY);
			_RGFW->vectorX = event.mouse.vecX;
			_RGFW->vectorY = event.mouse.vecY;

			RGFW_mousePosCallback(win, event.mouse.x, event.mouse.y, event.mouse.vecX, event.mouse.vecY);

			if (win->internal.mouseInside == RGFW_FALSE) {
				win->internal.mouseInside = RGFW_TRUE;
				_RGFW->windowState.win = win;
				_RGFW->windowState.mouseEnter =  RGFW_TRUE;
				event.type = RGFW_mouseEnter;
				RGFW_mouseNotifyCallback(win, event.mouse.x, event.mouse.y, 1);
				RGFW_eventQueuePush(&event);
			}

			event.type = RGFW_mousePosChanged;
			win->internal.lastMouseX = event.mouse.x;
			win->internal.lastMouseY = event.mouse.y;
			break;
		}
		case WM_INPUT: {
			if (!(win->internal.enabledEvents & RGFW_mousePosChangedFlag) || !(win->internal.holdMouse)) return DefWindowProcW(hWnd, message, wParam, lParam);
			unsigned size = sizeof(RAWINPUT);
			static RAWINPUT raw;

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));

			if (raw.header.dwType != RIM_TYPEMOUSE || (raw.data.mouse.lLastX == 0 && raw.data.mouse.lLastY == 0) )
				break;

			if (raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
				POINT pos = {0, 0};
				int width, height;

				if (raw.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP) {
					pos.x += GetSystemMetrics(SM_XVIRTUALSCREEN);
					pos.y += GetSystemMetrics(SM_YVIRTUALSCREEN);
					width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
					height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
				}
				else {
					width = GetSystemMetrics(SM_CXSCREEN);
					height = GetSystemMetrics(SM_CYSCREEN);
				}

				pos.x += (int) (((float)raw.data.mouse.lLastX / 65535.f) * (float)width);
				pos.y += (int) (((float)raw.data.mouse.lLastY / 65535.f) * (float)height);
				ScreenToClient(win->src.window, &pos);

				event.mouse.vecX = (float)(pos.x - win->internal.lastMouseX);
				event.mouse.vecY = (float)(pos.y - win->internal.lastMouseY);
			} else {
				event.mouse.vecX = (float)(raw.data.mouse.lLastX);
				event.mouse.vecY = (float)(raw.data.mouse.lLastY);
			}

			event.type = RGFW_mousePosChanged;
			win->internal.lastMouseX += (i32)event.mouse.vecX;
			win->internal.lastMouseY += (i32)event.mouse.vecY;
			_RGFW->vectorX = event.mouse.vecX;
			_RGFW->vectorY = event.mouse.vecY;
			event.mouse.x = win->internal.lastMouseX;
			event.mouse.y = win->internal.lastMouseY;
			RGFW_mousePosCallback(win, event.mouse.x, event.mouse.y, event.mouse.vecX, event.mouse.vecY);
			break;
		}
		case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN: case WM_XBUTTONDOWN:
			if (!(win->internal.enabledEvents & RGFW_mouseButtonPressedFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);
			if (message == WM_XBUTTONDOWN)
				event.button.value = RGFW_mouseMisc1 + (GET_XBUTTON_WPARAM(wParam) == XBUTTON2);
			else event.button.value = (message == WM_LBUTTONDOWN) ? (u8)RGFW_mouseLeft :
									 (message == WM_RBUTTONDOWN) ? (u8)RGFW_mouseRight : (u8)RGFW_mouseMiddle;

			event.type = RGFW_mouseButtonPressed;
			_RGFW->mouseButtons[event.button.value].prev = _RGFW->mouseButtons[event.button.value].current;
			_RGFW->mouseButtons[event.button.value].current = 1;
			RGFW_mouseButtonCallback(win, event.button.value, 1);
			break;
		case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP: case WM_XBUTTONUP:
			if (!(win->internal.enabledEvents & RGFW_mouseButtonReleasedFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);
			if (message == WM_XBUTTONUP)
				event.button.value = RGFW_mouseMisc1 + (GET_XBUTTON_WPARAM(wParam) == XBUTTON2);
			else event.button.value = (message == WM_LBUTTONUP) ? (u8)RGFW_mouseLeft :
									 (message == WM_RBUTTONUP) ? (u8)RGFW_mouseRight : (u8)RGFW_mouseMiddle;
			event.type = RGFW_mouseButtonReleased;
			_RGFW->mouseButtons[event.button.value].prev = _RGFW->mouseButtons[event.button.value].current;
			_RGFW->mouseButtons[event.button.value].current = 0;
			RGFW_mouseButtonCallback(win, event.button.value, 0);
			break;
		case WM_MOUSEWHEEL:
			if (!(win->internal.enabledEvents & RGFW_mouseScrollFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);

			event.type = RGFW_mouseScroll;
			event.scroll.x = 0.0f;
			event.scroll.y = (float)((i16) HIWORD(wParam) / (double) WHEEL_DELTA);
			_RGFW->scrollX = event.scroll.x;
			_RGFW->scrollY = event.scroll.y;

			RGFW_mouseScrollCallback(win, event.scroll.x, event.scroll.y);
			break;
		case 0x020E: /* WM_MOUSEHWHEEL */
			if (!(win->internal.enabledEvents & RGFW_mouseScrollFlag)) return DefWindowProcW(hWnd, message, wParam, lParam);

			event.type = RGFW_mouseScroll;
			event.scroll.x = -(float)((i16) HIWORD(wParam) / (double) WHEEL_DELTA);
			event.scroll.y = (float)0.0f;
			_RGFW->scrollX = event.scroll.x;
			_RGFW->scrollY = event.scroll.y;

			RGFW_mouseScrollCallback(win, event.scroll.x, event.scroll.y);
			break;
		case WM_DROPFILES: {
			event.type = RGFW_dataDrag;

			HDROP drop = (HDROP) wParam;
			POINT pt;

			/* Move the mouse to the position of the drop */
			DragQueryPoint(drop, &pt);

			event.drag.x = pt.x;
			event.drag.y = pt.y;

			_RGFW->windowState.win = win;
			_RGFW->windowState.dataDragging = RGFW_TRUE;
			_RGFW->windowState.dropX = event.drag.x;
			_RGFW->windowState.dropY = event.drag.y;

			if ((win->internal.enabledEvents & RGFW_dataDrag)) {
				RGFW_dataDragCallback(win, event.drag.x, event.drag.y);
				RGFW_eventQueuePush(&event);
			}

			if (!(win->internal.enabledEvents & RGFW_dataDrop)) return DefWindowProcW(hWnd, message, wParam, lParam);
			event.type = 0;
			event.type = RGFW_dataDrop;
			event.drop.files = _RGFW->files;
			event.drop.count = 0;
			event.drop.count = DragQueryFileW(drop, 0xffffffff, NULL, 0);

			u32 i;
			for (i = 0; i < event.drop.count; i++) {
				UINT length = DragQueryFileW(drop, i, NULL, 0);
				if (length == 0)
					continue;

				WCHAR buffer[RGFW_MAX_PATH * 2];
				if (length > (RGFW_MAX_PATH * 2) - 1)
					length = RGFW_MAX_PATH * 2;

				DragQueryFileW(drop, i, buffer, length + 1);

				RGFW_createUTF8FromWideStringWin32(buffer, event.drop.files[i], RGFW_MAX_PATH);

				event.drop.files[i][RGFW_MAX_PATH - 1] = '\0';
				event.common.win = win;
			}

			DragFinish(drop);

			_RGFW->windowState.win = win;
			_RGFW->windowState.dataDrop = RGFW_TRUE;
			_RGFW->windowState.filesCount =  event.drop.count;
			RGFW_dataDropCallback(win, event.drop.files, event.drop.count);
			break;
		}
		default: break;
	}

	if (event.type) {
		RGFW_eventQueuePush(&event);
	}

	return DefWindowProcW(hWnd, message, wParam, lParam);
}

#ifndef RGFW_NO_DPI
	HMODULE RGFW_Shcore_dll = NULL;
	typedef HRESULT (WINAPI *PFN_GetDpiForMonitor)(HMONITOR,MONITOR_DPI_TYPE,UINT*,UINT*);
	PFN_GetDpiForMonitor GetDpiForMonitorSRC = NULL;
	#define GetDpiForMonitor GetDpiForMonitorSRC
#endif

#if !defined(RGFW_NO_LOAD_WINMM) && !defined(RGFW_NO_WINMM)
	HMODULE RGFW_winmm_dll = NULL;
	typedef u32 (WINAPI * PFN_timeBeginPeriod)(u32);
	typedef PFN_timeBeginPeriod PFN_timeEndPeriod;
	PFN_timeBeginPeriod timeBeginPeriodSRC, timeEndPeriodSRC;
	#define timeBeginPeriod timeBeginPeriodSRC
	#define timeEndPeriod timeEndPeriodSRC
#elif !defined(RGFW_NO_WINMM)
	__declspec(dllimport) u32 __stdcall timeBeginPeriod(u32 uPeriod);
	__declspec(dllimport) u32 __stdcall timeEndPeriod(u32 uPeriod);
#endif
#define RGFW_PROC_DEF(proc, name) if (name##SRC == NULL && proc != NULL) { \
                                        name##SRC = (PFN_##name)(RGFW_proc)GetProcAddress((proc), (#name)); \
                                        RGFW_ASSERT(name##SRC != NULL); \
                                    }

RGFW_bool RGFW_createSurfacePtr(u8* data, i32 w, i32 h, RGFW_format format, RGFW_surface* surface) {
	RGFW_ASSERT(surface != NULL);
	surface->data = data;
	surface->w = w;
	surface->h = h;
	surface->format = format;

	BITMAPV5HEADER bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bV5Size = sizeof(bi);
	bi.bV5Width = (i32)w;
	bi.bV5Height = -((LONG) h);
	bi.bV5Planes = 1;
	bi.bV5BitCount = (format >= RGFW_formatRGBA8) ? 32 : 24;
	bi.bV5Compression = BI_RGB;

	surface->native.bitmap = CreateDIBSection(_RGFW->root->src.hdc,
		(BITMAPINFO*) &bi, DIB_RGB_COLORS,
		(void**) &surface->native.bitmapBits,
		NULL, (DWORD) 0);

	surface->native.format = (format >= RGFW_formatRGBA8) ? RGFW_formatBGRA8 : RGFW_formatBGR8;

	if (surface->native.bitmap == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errBuffer,  "Failed to create DIB section.");
		return RGFW_FALSE;
	}

	surface->native.hdcMem = CreateCompatibleDC(_RGFW->root->src.hdc);
	SelectObject(surface->native.hdcMem, surface->native.bitmap);

	return RGFW_TRUE;
}

void RGFW_surface_freePtr(RGFW_surface* surface) {
	RGFW_ASSERT(surface != NULL);

	DeleteDC(surface->native.hdcMem);
	DeleteObject(surface->native.bitmap);
}

void RGFW_window_blitSurface(RGFW_window* win, RGFW_surface* surface) {
	RGFW_copyImageData(surface->native.bitmapBits, surface->w, RGFW_MIN(win->h, surface->h), surface->native.format, surface->data, surface->format);
	BitBlt(win->src.hdc, 0, 0, RGFW_MIN(win->w, surface->w), RGFW_MIN(win->h, surface->h), surface->native.hdcMem, 0, 0, SRCCOPY);
}

void RGFW_releaseCursor(RGFW_window* win) {
	RGFW_UNUSED(win);
	ClipCursor(NULL);
	const RAWINPUTDEVICE id = { 0x01, 0x02, RIDEV_REMOVE, NULL };
	RegisterRawInputDevices(&id, 1, sizeof(id));
}

void RGFW_captureCursor(RGFW_window* win) {
	RGFW_UNUSED(win);

	RECT clipRect;
	GetClientRect(win->src.window, &clipRect);
	ClientToScreen(win->src.window, (POINT*) &clipRect.left);
	ClientToScreen(win->src.window, (POINT*) &clipRect.right);
	ClipCursor(&clipRect);

	const RAWINPUTDEVICE id = { 0x01, 0x02, 0, win->src.window };
	RegisterRawInputDevices(&id, 1, sizeof(id));
}

#define RGFW_LOAD_LIBRARY(x, lib) if (x == NULL) { x = LoadLibraryA(lib); RGFW_ASSERT(x != NULL); }

#ifdef RGFW_DIRECTX
int RGFW_window_createSwapChain_DirectX(RGFW_window* win, IDXGIFactory* pFactory, IUnknown* pDevice, IDXGISwapChain** swapchain) {
    RGFW_ASSERT(win && pFactory && pDevice && swapchain);

    static DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = win->w;
    swapChainDesc.BufferDesc.Height = win->h;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = (HWND)win->src.window;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT hr = pFactory->lpVtbl->CreateSwapChain(pFactory, (IUnknown*)pDevice, &swapChainDesc, swapchain);
    if (FAILED(hr)) {
        RGFW_sendDebugInfo(RGFW_typeError, RGFW_errDirectXContext,  "Failed to create DirectX swap chain!");
        return -2;
    }

    return 0;
}
#endif

/* we're doing it with magic numbers because some keys are missing  */
void RGFW_initKeycodesPlatform(void) {
	_RGFW->keycodes[0x00B] = RGFW_0;
	_RGFW->keycodes[0x002] = RGFW_1;
	_RGFW->keycodes[0x003] = RGFW_2;
	_RGFW->keycodes[0x004] = RGFW_3;
	_RGFW->keycodes[0x005] = RGFW_4;
	_RGFW->keycodes[0x006] = RGFW_5;
	_RGFW->keycodes[0x007] = RGFW_6;
	_RGFW->keycodes[0x008] = RGFW_7;
	_RGFW->keycodes[0x009] = RGFW_8;
	_RGFW->keycodes[0x00A] = RGFW_9;
	_RGFW->keycodes[0x01E] = RGFW_a;
	_RGFW->keycodes[0x030] = RGFW_b;
	_RGFW->keycodes[0x02E] = RGFW_c;
	_RGFW->keycodes[0x020] = RGFW_d;
	_RGFW->keycodes[0x012] = RGFW_e;
	_RGFW->keycodes[0x021] = RGFW_f;
	_RGFW->keycodes[0x022] = RGFW_g;
	_RGFW->keycodes[0x023] = RGFW_h;
	_RGFW->keycodes[0x017] = RGFW_i;
	_RGFW->keycodes[0x024] = RGFW_j;
	_RGFW->keycodes[0x025] = RGFW_k;
	_RGFW->keycodes[0x026] = RGFW_l;
	_RGFW->keycodes[0x032] = RGFW_m;
	_RGFW->keycodes[0x031] = RGFW_n;
	_RGFW->keycodes[0x018] = RGFW_o;
	_RGFW->keycodes[0x019] = RGFW_p;
	_RGFW->keycodes[0x010] = RGFW_q;
	_RGFW->keycodes[0x013] = RGFW_r;
	_RGFW->keycodes[0x01F] = RGFW_s;
	_RGFW->keycodes[0x014] = RGFW_t;
	_RGFW->keycodes[0x016] = RGFW_u;
	_RGFW->keycodes[0x02F] = RGFW_v;
	_RGFW->keycodes[0x011] = RGFW_w;
	_RGFW->keycodes[0x02D] = RGFW_x;
	_RGFW->keycodes[0x015] = RGFW_y;
	_RGFW->keycodes[0x02C] = RGFW_z;
	_RGFW->keycodes[0x028] = RGFW_apostrophe;
	_RGFW->keycodes[0x02B] = RGFW_backSlash;
	_RGFW->keycodes[0x033] = RGFW_comma;
	_RGFW->keycodes[0x00D] = RGFW_equals;
	_RGFW->keycodes[0x029] = RGFW_backtick;
	_RGFW->keycodes[0x01A] = RGFW_bracket;
	_RGFW->keycodes[0x00C] = RGFW_minus;
	_RGFW->keycodes[0x034] = RGFW_period;
	_RGFW->keycodes[0x01B] = RGFW_closeBracket;
	_RGFW->keycodes[0x027] = RGFW_semicolon;
	_RGFW->keycodes[0x035] = RGFW_slash;
	_RGFW->keycodes[0x056] = RGFW_world2;
	_RGFW->keycodes[0x00E] = RGFW_backSpace;
	_RGFW->keycodes[0x153] = RGFW_delete;
	_RGFW->keycodes[0x14F] = RGFW_end;
	_RGFW->keycodes[0x01C] = RGFW_enter;
	_RGFW->keycodes[0x001] = RGFW_escape;
	_RGFW->keycodes[0x147] = RGFW_home;
	_RGFW->keycodes[0x152] = RGFW_insert;
	_RGFW->keycodes[0x15D] = RGFW_menu;
	_RGFW->keycodes[0x151] = RGFW_pageDown;
	_RGFW->keycodes[0x149] = RGFW_pageUp;
	_RGFW->keycodes[0x045] = RGFW_pause;
	_RGFW->keycodes[0x039] = RGFW_space;
	_RGFW->keycodes[0x00F] = RGFW_tab;
	_RGFW->keycodes[0x03A] = RGFW_capsLock;
	_RGFW->keycodes[0x145] = RGFW_numLock;
	_RGFW->keycodes[0x046] = RGFW_scrollLock;
	_RGFW->keycodes[0x03B] = RGFW_F1;
	_RGFW->keycodes[0x03C] = RGFW_F2;
	_RGFW->keycodes[0x03D] = RGFW_F3;
	_RGFW->keycodes[0x03E] = RGFW_F4;
	_RGFW->keycodes[0x03F] = RGFW_F5;
	_RGFW->keycodes[0x040] = RGFW_F6;
	_RGFW->keycodes[0x041] = RGFW_F7;
	_RGFW->keycodes[0x042] = RGFW_F8;
	_RGFW->keycodes[0x043] = RGFW_F9;
	_RGFW->keycodes[0x044] = RGFW_F10;
	_RGFW->keycodes[0x057] = RGFW_F11;
	_RGFW->keycodes[0x058] = RGFW_F12;
	_RGFW->keycodes[0x064] = RGFW_F13;
	_RGFW->keycodes[0x065] = RGFW_F14;
	_RGFW->keycodes[0x066] = RGFW_F15;
	_RGFW->keycodes[0x067] = RGFW_F16;
	_RGFW->keycodes[0x068] = RGFW_F17;
	_RGFW->keycodes[0x069] = RGFW_F18;
	_RGFW->keycodes[0x06A] = RGFW_F19;
	_RGFW->keycodes[0x06B] = RGFW_F20;
	_RGFW->keycodes[0x06C] = RGFW_F21;
	_RGFW->keycodes[0x06D] = RGFW_F22;
	_RGFW->keycodes[0x06E] = RGFW_F23;
	_RGFW->keycodes[0x076] = RGFW_F24;
	_RGFW->keycodes[0x038] = RGFW_altL;
	_RGFW->keycodes[0x01D] = RGFW_controlL;
	_RGFW->keycodes[0x02A] = RGFW_shiftL;
	_RGFW->keycodes[0x15B] = RGFW_superL;
	_RGFW->keycodes[0x137] = RGFW_printScreen;
	_RGFW->keycodes[0x138] = RGFW_altR;
	_RGFW->keycodes[0x11D] = RGFW_controlR;
	_RGFW->keycodes[0x036] = RGFW_shiftR;
	_RGFW->keycodes[0x15C] = RGFW_superR;
	_RGFW->keycodes[0x150] = RGFW_down;
	_RGFW->keycodes[0x14B] = RGFW_left;
	_RGFW->keycodes[0x14D] = RGFW_right;
	_RGFW->keycodes[0x148] = RGFW_up;
	_RGFW->keycodes[0x052] = RGFW_kp0;
	_RGFW->keycodes[0x04F] = RGFW_kp1;
	_RGFW->keycodes[0x050] = RGFW_kp2;
	_RGFW->keycodes[0x051] = RGFW_kp3;
	_RGFW->keycodes[0x04B] = RGFW_kp4;
	_RGFW->keycodes[0x04C] = RGFW_kp5;
	_RGFW->keycodes[0x04D] = RGFW_kp6;
	_RGFW->keycodes[0x047] = RGFW_kp7;
	_RGFW->keycodes[0x048] = RGFW_kp8;
	_RGFW->keycodes[0x049] = RGFW_kp9;
	_RGFW->keycodes[0x04E] = RGFW_kpPlus;
	_RGFW->keycodes[0x053] = RGFW_kpPeriod;
	_RGFW->keycodes[0x135] = RGFW_kpSlash;
	_RGFW->keycodes[0x11C] = RGFW_kpReturn;
	_RGFW->keycodes[0x059] = RGFW_kpEqual;
	_RGFW->keycodes[0x037] = RGFW_kpMultiply;
	_RGFW->keycodes[0x04A] = RGFW_kpMinus;
}


i32 RGFW_initPlatform(void) {
#ifndef RGFW_NO_DPI
	#if (_WIN32_WINNT >= 0x0600)
		SetProcessDPIAware();
	#endif
#endif

	#ifndef RGFW_NO_WINMM
		#ifndef RGFW_NO_LOAD_WINMM
			RGFW_LOAD_LIBRARY(RGFW_winmm_dll, "winmm.dll");
			RGFW_PROC_DEF(RGFW_winmm_dll, timeBeginPeriod);
			RGFW_PROC_DEF(RGFW_winmm_dll, timeEndPeriod);
		#endif
		timeBeginPeriod(1);
	#endif

	#ifndef RGFW_NO_DWM
	RGFW_LOAD_LIBRARY(RGFW_dwm_dll, "dwmapi.dll");
	RGFW_PROC_DEF(RGFW_dwm_dll, DwmEnableBlurBehindWindow);
	#endif

	RGFW_LOAD_LIBRARY(RGFW_wgl_dll, "opengl32.dll");
	#ifndef RGFW_NO_LOAD_WGL
		RGFW_PROC_DEF(RGFW_wgl_dll, wglCreateContext);
		RGFW_PROC_DEF(RGFW_wgl_dll, wglDeleteContext);
		RGFW_PROC_DEF(RGFW_wgl_dll, wglGetProcAddress);
		RGFW_PROC_DEF(RGFW_wgl_dll, wglMakeCurrent);
		RGFW_PROC_DEF(RGFW_wgl_dll, wglGetCurrentDC);
		RGFW_PROC_DEF(RGFW_wgl_dll, wglGetCurrentContext);
		RGFW_PROC_DEF(RGFW_wgl_dll, wglShareLists);
	#endif

	u8 RGFW_blk[] = { 0, 0, 0, 0 };
	_RGFW->hiddenMouse = RGFW_loadMouse(RGFW_blk, 1, 1, RGFW_formatRGBA8);
    return 1;
}

RGFW_window* RGFW_createWindowPlatform(const char* name, RGFW_windowFlags flags, RGFW_window* win) {
	if (name[0] == 0) name = (char*) " ";
	win->src.hIconSmall = win->src.hIconBig = NULL;
	win->src.maxSizeW = 0;
	win->src.maxSizeH = 0;
	win->src.minSizeW = 0;
	win->src.minSizeH = 0;
	win->src.aspectRatioW = 0;
	win->src.aspectRatioH = 0;

	HINSTANCE inh = GetModuleHandleA(NULL);

	#ifndef __cplusplus
	WNDCLASSW Class = {0}; /*!< Setup the Window class. */
	#else
	WNDCLASSW Class = {};
	#endif

	if (_RGFW->className == NULL)
		_RGFW->className = (char*)name;

	wchar_t wide_class[256];
	MultiByteToWideChar(CP_UTF8, 0, _RGFW->className, -1, wide_class, 255);

	Class.lpszClassName = wide_class;
	Class.hInstance = inh;
	Class.hCursor = LoadCursor(NULL, IDC_ARROW);
	Class.lpfnWndProc = WndProcW;
	Class.cbClsExtra = sizeof(RGFW_window*);

	Class.hIcon = (HICON)LoadImageA(GetModuleHandleW(NULL), "RGFW_ICON", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	if (Class.hIcon == NULL)
		Class.hIcon = (HICON)LoadImageA(NULL, (LPCSTR)IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);

	RegisterClassW(&Class);

	DWORD window_style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	RECT windowRect, clientRect;

	if (!(flags & RGFW_windowNoBorder)) {
		window_style |= WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX;

		if (!(flags & RGFW_windowNoResize))
			window_style |= WS_SIZEBOX | WS_MAXIMIZEBOX;
	} else
		window_style |= WS_POPUP | WS_VISIBLE | WS_SYSMENU;

	wchar_t wide_name[256];
	MultiByteToWideChar(CP_UTF8, 0, name, -1, wide_name, 255);
	HWND dummyWin = CreateWindowW(Class.lpszClassName, (wchar_t*)wide_name, window_style, win->x, win->y, win->w, win->h, 0, 0, inh, 0);

	GetWindowRect(dummyWin, &windowRect);
	GetClientRect(dummyWin, &clientRect);

#ifdef RGFW_OPENGL
	RGFW_win32_loadOpenGLFuncs(dummyWin);
#endif

	DestroyWindow(dummyWin);

	win->src.offsetW = (i32)(windowRect.right - windowRect.left) - (i32)(clientRect.right - clientRect.left);
	win->src.offsetH = (i32)(windowRect.bottom - windowRect.top) - (i32)(clientRect.bottom - clientRect.top);
	win->src.window = CreateWindowW(Class.lpszClassName, (wchar_t*)wide_name, window_style, win->x, win->y, win->w + (i32)win->src.offsetW, win->h + (i32)win->src.offsetH, 0, 0, inh, 0);
	SetPropW(win->src.window, L"RGFW", win);
	RGFW_window_resize(win, win->w, win->h); /* so WM_GETMINMAXINFO gets called again */

	if (flags & RGFW_windowAllowDND) {
		win->internal.flags |= RGFW_windowAllowDND;
		RGFW_window_setDND(win, 1);
	}
	win->src.hdc = GetDC(win->src.window);

	RGFW_win32_makeWindowTransparent(win);
	return win;
}

void RGFW_window_setBorder(RGFW_window* win, RGFW_bool border) {
	RGFW_setBit(&win->internal.flags, RGFW_windowNoBorder, !border);
	LONG style = GetWindowLong(win->src.window, GWL_STYLE);

	if (border == 0) {
		SetWindowLong(win->src.window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
		SetWindowPos(
			win->src.window, HWND_TOP, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE
		);
	}
	else {
		if (win->internal.flags & RGFW_windowNoResize) style &= ~WS_MAXIMIZEBOX;
		SetWindowLong(win->src.window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPos(
			win->src.window, HWND_TOP, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE
		);
	}
}

void RGFW_window_setDND(RGFW_window* win, RGFW_bool allow) {
	RGFW_setBit(&win->internal.flags, RGFW_windowAllowDND, allow);
	DragAcceptFiles(win->src.window, allow);
}

RGFW_bool RGFW_getGlobalMouse(i32* x, i32* y) {
	POINT p;
	GetCursorPos(&p);
	if (x) *x = p.x;
	if (y) *y = p.y;
	return RGFW_TRUE;
}

void RGFW_window_setAspectRatio(RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);
	win->src.aspectRatioW = w;
	win->src.aspectRatioH = h;
}

void RGFW_window_setMinSize(RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);
	win->src.minSizeW = w;
	win->src.minSizeH = h;
}

void RGFW_window_setMaxSize(RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);
	win->src.maxSizeW = w;
	win->src.maxSizeH = h;
}

void RGFW_window_focus(RGFW_window* win) {
	RGFW_ASSERT(win);
    SetForegroundWindow(win->src.window);
    SetFocus(win->src.window);
}

void RGFW_window_raise(RGFW_window* win) {
	RGFW_ASSERT(win);
	BringWindowToTop(win->src.window);
	SetWindowPos(win->src.window, HWND_TOP, win->x, win->y, win->w, win->h, SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
}

void RGFW_window_setFullscreen(RGFW_window* win, RGFW_bool fullscreen) {
	RGFW_ASSERT(win != NULL);

	if (fullscreen == RGFW_FALSE) {
		RGFW_window_setBorder(win, 1);
		SetWindowPos(win->src.window, HWND_NOTOPMOST, win->internal.oldX, win->internal.oldY, win->internal.oldW + (i32)win->src.offsetW, win->internal.oldH + (i32)win->src.offsetH,
			 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		win->internal.flags &= ~(u32)RGFW_windowFullscreen;
		win->x  = win->internal.oldX;
		win->y = win->internal.oldY;
		win->w = win->internal.oldW;
		win->h = win->internal.oldH;
		return;
	}

	win->internal.oldX = win->x;
	win->internal.oldY = win->y;
	win->internal.oldW = win->w;
	win->internal.oldH = win->h;
	win->internal.flags |= RGFW_windowFullscreen;

	RGFW_monitor mon  = RGFW_window_getMonitor(win);
	RGFW_window_setBorder(win, 0);

    SetWindowPos(win->src.window, HWND_TOPMOST, (i32)mon.x, (i32)mon.x, (i32)mon.mode.w, (i32)mon.mode.h, SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    RGFW_monitor_scaleToWindow(mon, win);

	win->x = mon.x; win->y = mon.x;
	win->w = mon.mode.w;
	win->h = mon.mode.h;
}

void RGFW_window_maximize(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_window_hide(win);
	ShowWindow(win->src.window, SW_MAXIMIZE);
}

void RGFW_window_minimize(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	ShowWindow(win->src.window, SW_MINIMIZE);
}

void RGFW_window_setFloating(RGFW_window* win, RGFW_bool floating) {
    RGFW_ASSERT(win != NULL);
    if (floating) SetWindowPos(win->src.window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    else SetWindowPos(win->src.window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void RGFW_window_setOpacity(RGFW_window* win, u8 opacity) {
	SetWindowLong(win->src.window, GWL_EXSTYLE, WS_EX_LAYERED);
	SetLayeredWindowAttributes(win->src.window, 0, opacity, LWA_ALPHA);
}

void RGFW_window_restore(RGFW_window* win) { RGFW_window_show(win); }

RGFW_bool RGFW_window_isFloating(RGFW_window* win) {
	return (GetWindowLongPtr(win->src.window, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
}

void RGFW_stopCheckEvents(void) {
	PostMessageW(_RGFW->root->src.window, WM_NULL, 0, 0);
}

void RGFW_waitForEvent(i32 waitMS) {
	MsgWaitForMultipleObjects(0, NULL, FALSE, (DWORD)waitMS, QS_ALLINPUT);
}

u8 RGFW_rgfwToKeyChar(u32 rgfw_keycode) {
    UINT vsc = RGFW_rgfwToApiKey(rgfw_keycode);  /* Should return a Windows VK_* code */
    BYTE keyboardState[256] = {0};

    if (!GetKeyboardState(keyboardState))
        return (u8)rgfw_keycode;

    UINT vk = MapVirtualKeyW(vsc, MAPVK_VSC_TO_VK);
    HKL layout = GetKeyboardLayout(0);

    wchar_t charBuffer[2] = {0};
    int result = ToUnicodeEx(vk, vsc, keyboardState, charBuffer, 1, 0, layout);

    if (result <= 0)
        return (u8)rgfw_keycode;

    return (u8)charBuffer[0];
}

void RGFW_pollEvents(void) {
	RGFW_resetPrevState();
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0u, 0u, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

RGFW_bool RGFW_window_isHidden(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	return IsWindowVisible(win->src.window) == 0 && !RGFW_window_isMinimized(win);
}

RGFW_bool RGFW_window_isMinimized(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	#ifndef __cplusplus
	WINDOWPLACEMENT placement = {0};
	#else
	WINDOWPLACEMENT placement = {};
	#endif
	GetWindowPlacement(win->src.window, &placement);
	return placement.showCmd == SW_SHOWMINIMIZED;
}

RGFW_bool RGFW_window_isMaximized(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	#ifndef __cplusplus
	WINDOWPLACEMENT placement = {0};
	#else
	WINDOWPLACEMENT placement = {};
	#endif
	GetWindowPlacement(win->src.window, &placement);
	return placement.showCmd == SW_SHOWMAXIMIZED || IsZoomed(win->src.window);
}

typedef struct { int iIndex; HMONITOR hMonitor; RGFW_monitor* monitors; } RGFW_mInfo;
#ifndef RGFW_NO_MONITOR
RGFW_monitor RGFW_win32_createMonitor(HMONITOR src);
RGFW_monitor RGFW_win32_createMonitor(HMONITOR src) {
	RGFW_monitor monitor;
	RGFW_MEMSET(&monitor, 0, sizeof(monitor));

	MONITORINFOEXW monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFOEXW);
	GetMonitorInfoW(src, (LPMONITORINFO)&monitorInfo);

	/* get the monitor's index */
	DISPLAY_DEVICEW dd;
	dd.cb = sizeof(dd);

    DWORD deviceNum;
	for (deviceNum = 0; EnumDisplayDevicesW(NULL, deviceNum, &dd, 0); deviceNum++) {
		if (!(dd.StateFlags & DISPLAY_DEVICE_ACTIVE))
			continue;

		DEVMODEW dm;
		ZeroMemory(&dm, sizeof(dm));
		dm.dmSize = sizeof(dm);

		if (EnumDisplaySettingsW(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) {
			monitor.mode.refreshRate = dm.dmDisplayFrequency;
			RGFW_splitBPP(dm.dmBitsPerPel, &monitor.mode);
		}

		DISPLAY_DEVICEW mdd;
		mdd.cb = sizeof(mdd);

		if (EnumDisplayDevicesW(dd.DeviceName, (DWORD)deviceNum, &mdd, 0)) {
			RGFW_createUTF8FromWideStringWin32(mdd.DeviceString, monitor.name, sizeof(monitor.name));
			monitor.name[sizeof(monitor.name) - 1] = '\0';
			break;
		}
	}

	monitor.x = monitorInfo.rcWork.left;
	monitor.y = monitorInfo.rcWork.top;
	monitor.mode.w = (i32)(monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
	monitor.mode.h = (i32)(monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);

	HDC hdc = CreateDCW(monitorInfo.szDevice, NULL, NULL, NULL);
	/* get pixels per inch */
	float dpiX = (float)GetDeviceCaps(hdc, LOGPIXELSX);
	float dpiY = (float)GetDeviceCaps(hdc, LOGPIXELSX);

	monitor.scaleX = dpiX / 96.0f;
	monitor.scaleY = dpiY / 96.0f;
	monitor.pixelRatio = dpiX >= 192.0f ? 2.0f : 1.0f;

	monitor.physW = (float)GetDeviceCaps(hdc, HORZSIZE) / 25.4f;
	monitor.physH = (float)GetDeviceCaps(hdc, VERTSIZE) / 25.4f;
	DeleteDC(hdc);

	#ifndef RGFW_NO_DPI
		RGFW_LOAD_LIBRARY(RGFW_Shcore_dll, "shcore.dll");
		RGFW_PROC_DEF(RGFW_Shcore_dll, GetDpiForMonitor);

		if (GetDpiForMonitor != NULL) {
			u32 x, y;
			GetDpiForMonitor(src, MDT_EFFECTIVE_DPI, &x, &y);
			monitor.scaleX = (float) (x) / (float) 96.0f;
			monitor.scaleY = (float) (y) / (float) 96.0f;
			monitor.pixelRatio = dpiX >= 192.0f ? 2.0f : 1.0f;
		}
	#endif

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoMonitor,  "monitor found");
	return monitor;
}
#endif /* RGFW_NO_MONITOR */

#ifndef RGFW_NO_MONITOR
BOOL CALLBACK GetMonitorHandle(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
BOOL CALLBACK GetMonitorHandle(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	RGFW_UNUSED(hdcMonitor);
	RGFW_UNUSED(lprcMonitor);

	RGFW_mInfo* info = (RGFW_mInfo*) dwData;


	if (info->iIndex >= 6)
		return FALSE;

	info->monitors[info->iIndex] = RGFW_win32_createMonitor(hMonitor);
	info->iIndex++;

	return TRUE;
}

RGFW_monitor RGFW_getPrimaryMonitor(void) {
	#ifdef __cplusplus
	return RGFW_win32_createMonitor(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY));
	#else
	return RGFW_win32_createMonitor(MonitorFromPoint((POINT){0, 0}, MONITOR_DEFAULTTOPRIMARY));
	#endif
}

RGFW_monitor* RGFW_getMonitors(size_t* len) {
	static RGFW_monitor monitors[6];
	RGFW_mInfo info;
	info.iIndex = 0;
	info.monitors = monitors;

	EnumDisplayMonitors(NULL, NULL, GetMonitorHandle, (LPARAM) &info);

	if (len != NULL) *len = (size_t)info.iIndex;
	return monitors;
}

RGFW_monitor RGFW_window_getMonitor(RGFW_window* win) {
	HMONITOR src = MonitorFromWindow(win->src.window, MONITOR_DEFAULTTOPRIMARY);
	return RGFW_win32_createMonitor(src);
}

RGFW_bool RGFW_monitor_requestMode(RGFW_monitor mon, RGFW_monitorMode mode, RGFW_modeRequest request) {
    POINT p = { mon.x, mon.y };
    HMONITOR src = MonitorFromPoint(p, MONITOR_DEFAULTTOPRIMARY);

	MONITORINFOEX  monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfoA(src, (LPMONITORINFO)&monitorInfo);

    DISPLAY_DEVICEA dd;
    dd.cb = sizeof(dd);

    /* Enumerate display devices */
    DWORD deviceNum;
    for (deviceNum = 0; EnumDisplayDevicesA(NULL, deviceNum, &dd, 0); deviceNum++) {
        if (!(dd.StateFlags & DISPLAY_DEVICE_ACTIVE))
			continue;

        if (strcmp(dd.DeviceName, (const char*)monitorInfo.szDevice) != 0)
            continue;

        DEVMODEA dm;
		ZeroMemory(&dm, sizeof(dm));
		dm.dmSize = sizeof(dm);

		if (EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) {
			if (request & RGFW_monitorScale) {
				dm.dmFields |= DM_PELSWIDTH | DM_PELSHEIGHT;
				dm.dmPelsWidth = (u32)mode.w;
				dm.dmPelsHeight = (u32)mode.h;
            }

			if (request & RGFW_monitorRefresh) {
				dm.dmFields |= DM_DISPLAYFREQUENCY;
				dm.dmDisplayFrequency = mode.refreshRate;
			}

			if (request & RGFW_monitorRGB) {
				dm.dmFields |= DM_BITSPERPEL;
				dm.dmBitsPerPel = (DWORD)(mode.red + mode.green + mode.blue);
			}

			if (ChangeDisplaySettingsExA(dd.DeviceName, &dm, NULL, CDS_TEST, NULL) == DISP_CHANGE_SUCCESSFUL) {
				if (ChangeDisplaySettingsExA(dd.DeviceName, &dm, NULL, CDS_UPDATEREGISTRY, NULL) == DISP_CHANGE_SUCCESSFUL)
					return RGFW_TRUE;
				return RGFW_FALSE;
			} else return RGFW_FALSE;
		}
	}

	return RGFW_FALSE;
}

#endif
HICON RGFW_loadHandleImage(u8* data, i32 w, i32 h, RGFW_format format, BOOL icon);
HICON RGFW_loadHandleImage(u8* data, i32 w, i32 h, RGFW_format format, BOOL icon) {
	BITMAPV5HEADER bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bV5Size = sizeof(bi);
	bi.bV5Width = (i32)w;
	bi.bV5Height = -((LONG) h);
	bi.bV5Planes = 1;
	bi.bV5BitCount = (WORD)32;
	bi.bV5Compression = BI_RGB;
	HDC dc = GetDC(NULL);
	u8* target = NULL;

	HBITMAP color = CreateDIBSection(dc,
		(BITMAPINFO*) &bi, DIB_RGB_COLORS, (void**) &target,
		NULL, (DWORD) 0);

	RGFW_copyImageData(target, w, h, RGFW_formatBGRA8, data, format);
    ReleaseDC(NULL, dc);

	HBITMAP mask = CreateBitmap((i32)w, (i32)h, 1, 1, NULL);

	ICONINFO ii;
	ZeroMemory(&ii, sizeof(ii));
	ii.fIcon = icon;
	ii.xHotspot = (u32)w / 2;
	ii.yHotspot = (u32)h / 2;
	ii.hbmMask = mask;
	ii.hbmColor = color;

	HICON handle = CreateIconIndirect(&ii);

	DeleteObject(color);
	DeleteObject(mask);

	return handle;
}
RGFW_mouse* RGFW_loadMouse(u8* data, i32 w, i32 h, RGFW_format format) {
	HCURSOR cursor = (HCURSOR) RGFW_loadHandleImage(data, w, h, format, FALSE);
	return cursor;
}

void RGFW_window_setMouse(RGFW_window* win, RGFW_mouse* mouse) {
	RGFW_ASSERT(win && mouse);
	SetClassLongPtrA(win->src.window, GCLP_HCURSOR, (LPARAM) mouse);
	SetCursor((HCURSOR)mouse);
}

void RGFW_freeMouse(RGFW_mouse* mouse) {
	RGFW_ASSERT(mouse);
	DestroyCursor((HCURSOR)mouse);
}

RGFW_bool RGFW_window_setMouseDefault(RGFW_window* win) {
	return RGFW_window_setMouseStandard(win, RGFW_mouseArrow);
}

RGFW_bool RGFW_window_setMouseStandard(RGFW_window* win, u8 mouse) {
	RGFW_ASSERT(win != NULL);

	static const u32 mouseIconSrc[16] = {OCR_NORMAL, OCR_NORMAL, OCR_IBEAM, OCR_CROSS, OCR_HAND, OCR_SIZEWE, OCR_SIZENS, OCR_SIZENWSE, OCR_SIZENESW, OCR_SIZEALL, OCR_NO};
	if (mouse > (sizeof(mouseIconSrc) / sizeof(u32)))
		return RGFW_FALSE;

	char* icon = MAKEINTRESOURCEA(mouseIconSrc[mouse]);

	SetClassLongPtrA(win->src.window, GCLP_HCURSOR, (LPARAM) LoadCursorA(NULL, icon));
	SetCursor(LoadCursorA(NULL, icon));
	return RGFW_TRUE;
}

void RGFW_window_hide(RGFW_window* win) {
	ShowWindow(win->src.window, SW_HIDE);
}

void RGFW_window_show(RGFW_window* win) {
	if (win->internal.flags & RGFW_windowFocusOnShow) RGFW_window_focus(win);
	ShowWindow(win->src.window, SW_RESTORE);
}

#define RGFW_FREE_LIBRARY(x) if (x != NULL) FreeLibrary(x); x = NULL;
void RGFW_deinitPlatform(void) {
    #ifndef RGFW_NO_DPI
        RGFW_FREE_LIBRARY(RGFW_Shcore_dll);
    #endif

    #ifndef RGFW_NO_WINMM
        timeEndPeriod(1);
        #ifndef RGFW_NO_LOAD_WINMM
            RGFW_FREE_LIBRARY(RGFW_winmm_dll);
        #endif
    #endif

    RGFW_FREE_LIBRARY(RGFW_wgl_dll);

    RGFW_freeMouse(_RGFW->hiddenMouse);
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal,  "global context deinitialized");
}


void RGFW_window_closePlatform(RGFW_window* win) {
	RemovePropW(win->src.window, L"RGFW");
	ReleaseDC(win->src.window, win->src.hdc); /*!< delete device context */
	DestroyWindow(win->src.window); /*!< delete window */

	if (win->src.hIconSmall) DestroyIcon(win->src.hIconSmall);
	if (win->src.hIconBig) DestroyIcon(win->src.hIconBig);
}

void RGFW_window_move(RGFW_window* win, i32 x, i32 y) {
	RGFW_ASSERT(win != NULL);

	win->x = x;
	win->y = y;
	SetWindowPos(win->src.window, HWND_TOP, win->x, win->y, 0, 0, SWP_NOSIZE);
}

void RGFW_window_resize(RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);

	win->w = w;
	win->h = h;
	SetWindowPos(win->src.window, HWND_TOP, 0, 0, win->w + (i32)win->src.offsetW, win->h + (i32)win->src.offsetH, SWP_NOMOVE);
}


void RGFW_window_setName(RGFW_window* win, const char* name) {
	RGFW_ASSERT(win != NULL);

	wchar_t wide_name[256];
	MultiByteToWideChar(CP_UTF8, 0, name, -1, wide_name, 256);
	SetWindowTextW(win->src.window, wide_name);
}

#ifndef RGFW_NO_PASSTHROUGH
void RGFW_window_setMousePassthrough(RGFW_window* win, RGFW_bool passthrough) {
	RGFW_ASSERT(win != NULL);
	COLORREF key = 0;
	BYTE alpha = 0;
	DWORD flags = 0;
	i32 exStyle = GetWindowLongW(win->src.window, GWL_EXSTYLE);

	if (exStyle & WS_EX_LAYERED)
		GetLayeredWindowAttributes(win->src.window, &key, &alpha, &flags);

	if (passthrough)
		exStyle |= (WS_EX_TRANSPARENT | WS_EX_LAYERED);
	else {
		exStyle &= ~WS_EX_TRANSPARENT;
		if (exStyle & WS_EX_LAYERED && !(flags & LWA_ALPHA))
			exStyle &= ~WS_EX_LAYERED;
	}

	SetWindowLongW(win->src.window, GWL_EXSTYLE, exStyle);

	if (passthrough)
		SetLayeredWindowAttributes(win->src.window, key, alpha, flags);
}
#endif

RGFW_bool RGFW_window_setIconEx(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format, RGFW_icon type) {
	RGFW_ASSERT(win != NULL);
	#ifndef RGFW_WIN95
		if (win->src.hIconSmall && (type & RGFW_iconWindow)) DestroyIcon(win->src.hIconSmall);
		if (win->src.hIconBig && (type & RGFW_iconTaskbar)) DestroyIcon(win->src.hIconBig);

		if (data == NULL) {
			HICON defaultIcon = LoadIcon(NULL, IDI_APPLICATION);
			if (type & RGFW_iconWindow)
				SendMessage(win->src.window, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)defaultIcon);
			if (type & RGFW_iconTaskbar)
				SendMessage(win->src.window, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)defaultIcon);
			return RGFW_TRUE;
		}

		if (type & RGFW_iconWindow) {
			win->src.hIconSmall = RGFW_loadHandleImage(data, w, h, format, TRUE);
			SendMessage(win->src.window, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)win->src.hIconSmall);
		}
		if (type & RGFW_iconTaskbar) {
			win->src.hIconBig = RGFW_loadHandleImage(data, w, h, format, TRUE);
			SendMessage(win->src.window, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)win->src.hIconBig);
		}
		return RGFW_TRUE;
	#else
		RGFW_UNUSED(img);
		RGFW_UNUSED(type);
		return RGFW_FALSE;
	#endif
}

RGFW_ssize_t RGFW_readClipboardPtr(char* str, size_t strCapacity) {
	/* Open the clipboard */
	if (OpenClipboard(NULL) == 0)
		return -1;

	/* Get the clipboard data as a Unicode string */
	HANDLE hData = GetClipboardData(CF_UNICODETEXT);
	if (hData == NULL) {
		CloseClipboard();
		return -1;
	}

	wchar_t* wstr = (wchar_t*) GlobalLock(hData);

	RGFW_ssize_t textLen = 0;

	{
		setlocale(LC_ALL, "en_US.UTF-8");

		textLen = (RGFW_ssize_t)wcstombs(NULL, wstr, 0) + 1;
		if (str != NULL && (RGFW_ssize_t)strCapacity <= textLen - 1)
			textLen = 0;

		if (str != NULL && textLen) {
			if (textLen > 1)
				wcstombs(str, wstr, (size_t)(textLen));

			str[textLen - 1] = '\0';
		}
	}

	/* Release the clipboard data */
	GlobalUnlock(hData);
	CloseClipboard();

	return textLen;
}

void RGFW_writeClipboard(const char* text, u32 textLen) {
	HANDLE object;
	WCHAR* buffer;

	object = GlobalAlloc(GMEM_MOVEABLE, (1 + textLen) * sizeof(WCHAR));
	if (!object)
		return;

	buffer = (WCHAR*) GlobalLock(object);
	if (!buffer) {
		GlobalFree(object);
		return;
	}

	MultiByteToWideChar(CP_UTF8, 0, text, -1, buffer, (i32)textLen);
	GlobalUnlock(object);

	if (!OpenClipboard(_RGFW->root->src.window)) {
		GlobalFree(object);
		return;
	}

	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, object);
	CloseClipboard();
}

void RGFW_window_moveMouse(RGFW_window* win, i32 x, i32 y) {
	RGFW_ASSERT(win != NULL);
	win->internal.lastMouseX = x - win->x;
	win->internal.lastMouseX = y - win->y;
	SetCursorPos(x, y);
}

#ifdef RGFW_OPENGL
RGFW_bool RGFW_extensionSupportedPlatform_OpenGL(const char * extension, size_t len) {
	const char* extensions = NULL;

    RGFW_proc proc = RGFW_getProcAddress_OpenGL("wglGetExtensionsStringARB");
    RGFW_proc proc2 = RGFW_getProcAddress_OpenGL("wglGetExtensionsStringEXT");

    if (proc)
        extensions = ((const char* (*)(HDC))proc)(wglGetCurrentDC());
    else if (proc2)
        extensions = ((const char*(*)(void))proc2)();
    return extensions != NULL && RGFW_extensionSupportedStr(extensions, extension, len);
}

RGFW_proc RGFW_getProcAddress_OpenGL(const char* procname) {
    RGFW_proc proc = (RGFW_proc)wglGetProcAddress(procname);
    if (proc)
        return proc;

    return (RGFW_proc) GetProcAddress(RGFW_wgl_dll, procname);
}

void RGFW_win32_loadOpenGLFuncs(HWND dummyWin) {
     if (wglSwapIntervalEXT != NULL && wglChoosePixelFormatARB != NULL && wglChoosePixelFormatARB != NULL)
		return;

	HDC dummy_dc = GetDC(dummyWin);
	u32 pfd_flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	PIXELFORMATDESCRIPTOR pfd = {sizeof(pfd), 1, pfd_flags, PFD_TYPE_RGBA, 32, 8, PFD_MAIN_PLANE, 32, 8, 8, 8, 8, 8, 0, 0, 0, 0, 0, 32, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0};

	int dummy_pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
	SetPixelFormat(dummy_dc, dummy_pixel_format, &pfd);

	HGLRC dummy_context = wglCreateContext(dummy_dc);

	HGLRC cur = wglGetCurrentContext();
	wglMakeCurrent(dummy_dc, dummy_context);

	wglCreateContextAttribsARB = ((PFNWGLCREATECONTEXTATTRIBSARBPROC(WINAPI *)(const char*)) wglGetProcAddress)("wglCreateContextAttribsARB");
	wglChoosePixelFormatARB = ((PFNWGLCHOOSEPIXELFORMATARBPROC(WINAPI *)(const char*)) wglGetProcAddress)("wglChoosePixelFormatARB");

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)(RGFW_proc)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT == NULL) {
        RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext,  "Failed to load swap interval function");
    }

	wglMakeCurrent(dummy_dc, cur);
	wglDeleteContext(dummy_context);
	ReleaseDC(dummyWin, dummy_dc);
}

#define WGL_ACCELERATION_ARB             0x2003
#define WGL_FULL_ACCELERATION_ARB        0x2027
#define WGL_DRAW_TO_WINDOW_ARB           0x2001
#define WGL_PIXEL_TYPE_ARB               0x2013
#define WGL_TYPE_RGBA_ARB                0x202b
#define WGL_SUPPORT_OPENGL_ARB           0x2010
#define WGL_COLOR_BITS_ARB               0x2014
#define WGL_DOUBLE_BUFFER_ARB            0x2011
#define WGL_ALPHA_BITS_ARB               0x201b
#define WGL_DEPTH_BITS_ARB               0x2022
#define WGL_STENCIL_BITS_ARB             0x2023
#define WGL_STEREO_ARB                  0x2012
#define WGL_AUX_BUFFERS_ARB              0x2024
#define WGL_RED_BITS_ARB                 0x2015
#define WGL_GREEN_BITS_ARB               0x2017
#define WGL_BLUE_BITS_ARB                0x2019
#define WGL_ACCUM_RED_BITS_ARB           0x201e
#define WGL_ACCUM_GREEN_BITS_ARB         0x201f
#define WGL_ACCUM_BLUE_BITS_ARB          0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB         0x2021
#define WGL_COLORSPACE_SRGB_EXT          0x3089
#define WGL_CONTEXT_OPENGL_NO_ERROR_ARB  0x31b3
#define WGL_CONTEXT_RELEASE_BEHAVIOR_ARB         0x2097
#define WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB    0x0000
#define WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB   0x2098
#define WGL_CONTEXT_FLAGS_ARB            0x2094
#define WGL_ACCESS_READ_WRITE_NV         0x00000001
#define WGL_COVERAGE_SAMPLES_NV          0x2042
#define WGL_CONTEXT_ES_PROFILE_BIT_EXT   0x00000004
#define WGL_CONTEXT_PROFILE_MASK_ARB               0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB            0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB   0x00000002
#define WGL_CONTEXT_MAJOR_VERSION_ARB               0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB               0x2092
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB             0x20A9
#define WGL_CONTEXT_RELEASE_BEHAVIOR_ARB 0x2097
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004

RGFW_bool RGFW_window_createContextPtr_OpenGL(RGFW_window* win, RGFW_glContext* ctx, RGFW_glHints* hints) {
	const char flushControl[] = "WGL_ARB_context_flush_control";
	const char noError[] = "WGL_ARB_create_context_no_error";
	const char robustness[] = "WGL_ARB_create_context_robustness";

	win->src.ctx.native = ctx;
	win->src.gfxType = RGFW_gfxNativeOpenGL;

	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion     = 1;
	pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType   = PFD_TYPE_RGBA;
	pfd.iLayerType   = PFD_MAIN_PLANE;
	pfd.cColorBits   = 32;
	pfd.cAlphaBits   = 8;
	pfd.cDepthBits   = 24;
	pfd.cStencilBits = (BYTE)hints->stencil;
	pfd.cAuxBuffers  = (BYTE)hints->auxBuffers;
	if (hints->stereo) pfd.dwFlags |= PFD_STEREO;

	/* try to create the pixel format we want for OpenGL and then try to create an OpenGL context for the specified version */
	if (hints->renderer == RGFW_glSoftware)
		pfd.dwFlags |= PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED;

	/* get pixel format, default to a basic pixel format */
	int pixel_format = ChoosePixelFormat(win->src.hdc, &pfd);
	if (wglChoosePixelFormatARB != NULL) {
		i32 pixel_format_attribs[50];
		RGFW_attribStack stack;
		RGFW_attribStack_init(&stack, pixel_format_attribs, 50);

		RGFW_attribStack_pushAttribs(&stack, WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB);
		RGFW_attribStack_pushAttribs(&stack, WGL_DRAW_TO_WINDOW_ARB, 1);
		RGFW_attribStack_pushAttribs(&stack, WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB);
		RGFW_attribStack_pushAttribs(&stack, WGL_SUPPORT_OPENGL_ARB, 1);
		RGFW_attribStack_pushAttribs(&stack, WGL_COLOR_BITS_ARB, 32);
		RGFW_attribStack_pushAttribs(&stack, WGL_DOUBLE_BUFFER_ARB, 1);
		RGFW_attribStack_pushAttribs(&stack, WGL_ALPHA_BITS_ARB, hints->alpha);
		RGFW_attribStack_pushAttribs(&stack, WGL_DEPTH_BITS_ARB, hints->depth);
		RGFW_attribStack_pushAttribs(&stack, WGL_STENCIL_BITS_ARB, hints->stencil);
		RGFW_attribStack_pushAttribs(&stack, WGL_STEREO_ARB, hints->stereo);
		RGFW_attribStack_pushAttribs(&stack, WGL_AUX_BUFFERS_ARB, hints->auxBuffers);
		RGFW_attribStack_pushAttribs(&stack, WGL_RED_BITS_ARB, hints->red);
		RGFW_attribStack_pushAttribs(&stack, WGL_GREEN_BITS_ARB, hints->blue);
		RGFW_attribStack_pushAttribs(&stack, WGL_BLUE_BITS_ARB, hints->green);
		RGFW_attribStack_pushAttribs(&stack, WGL_ACCUM_RED_BITS_ARB, hints->accumRed);
		RGFW_attribStack_pushAttribs(&stack, WGL_ACCUM_GREEN_BITS_ARB, hints->accumGreen);
		RGFW_attribStack_pushAttribs(&stack, WGL_ACCUM_BLUE_BITS_ARB, hints->accumBlue);
		RGFW_attribStack_pushAttribs(&stack, WGL_ACCUM_ALPHA_BITS_ARB, hints->accumAlpha);

		if(hints->sRGB) {
			if (hints->profile != RGFW_glES)
				RGFW_attribStack_pushAttribs(&stack, WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, 1);
			else
				RGFW_attribStack_pushAttribs(&stack, WGL_COLORSPACE_SRGB_EXT, hints->sRGB);
		}

		RGFW_attribStack_pushAttribs(&stack, WGL_COVERAGE_SAMPLES_NV, hints->samples);

		RGFW_attribStack_pushAttribs(&stack, 0, 0);

		int new_pixel_format;
		UINT num_formats;
		wglChoosePixelFormatARB(win->src.hdc, pixel_format_attribs, 0, 1, &new_pixel_format, &num_formats);
		if (!num_formats)
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext, "Failed to create a pixel format for WGL");
		else pixel_format = new_pixel_format;
	}

	PIXELFORMATDESCRIPTOR suggested;
	if (!DescribePixelFormat(win->src.hdc, pixel_format, sizeof(suggested), &suggested) ||
		!SetPixelFormat(win->src.hdc, pixel_format, &pfd))
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext, "Failed to set the WGL pixel format");

	if (wglCreateContextAttribsARB != NULL) {
		/* create OpenGL/WGL context for the specified version */
		i32 attribs[40];
		RGFW_attribStack stack;
		RGFW_attribStack_init(&stack, attribs, 50);


		i32 mask = 0;
		switch (hints->profile) {
			case RGFW_glES: mask |= WGL_CONTEXT_ES_PROFILE_BIT_EXT; break;
			case RGFW_glCompatibility: mask |= WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB; break;
			case RGFW_glCore: mask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB; break;
			default: mask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB; break;
		}

		RGFW_attribStack_pushAttribs(&stack, WGL_CONTEXT_PROFILE_MASK_ARB, mask);

		if (hints->minor || hints->major) {
			RGFW_attribStack_pushAttribs(&stack, WGL_CONTEXT_MAJOR_VERSION_ARB, hints->major);
			RGFW_attribStack_pushAttribs(&stack, WGL_CONTEXT_MINOR_VERSION_ARB, hints->minor);
		}

		if (RGFW_extensionSupportedPlatform_OpenGL(noError, sizeof(noError)))
			RGFW_attribStack_pushAttribs(&stack, WGL_CONTEXT_OPENGL_NO_ERROR_ARB, hints->noError);

		if (RGFW_extensionSupportedPlatform_OpenGL(flushControl, sizeof(flushControl))) {
			if (hints->releaseBehavior == RGFW_glReleaseFlush) {
				RGFW_attribStack_pushAttribs(&stack, WGL_CONTEXT_RELEASE_BEHAVIOR_ARB, WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB); /* WGL_CONTEXT_RELEASE_BEHAVIOR_ARB */
			} else if (hints->releaseBehavior == RGFW_glReleaseNone) {
				RGFW_attribStack_pushAttribs(&stack, WGL_CONTEXT_RELEASE_BEHAVIOR_ARB, WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
			}
		}

		i32 flags = 0;
		if (hints->debug) flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
		if (hints->robustness && RGFW_extensionSupportedPlatform_OpenGL(robustness, sizeof(robustness))) flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;
		if (flags) {
			RGFW_attribStack_pushAttribs(&stack, WGL_CONTEXT_FLAGS_ARB, flags);
		}


		RGFW_attribStack_pushAttribs(&stack, 0, 0);

		win->src.ctx.native->ctx = (HGLRC)wglCreateContextAttribsARB(win->src.hdc, NULL, attribs);
	}

	if (wglCreateContextAttribsARB == NULL || win->src.ctx.native->ctx == NULL) { /* fall back to a default context (probably OpenGL 2 or something) */
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext, "Failed to create an accelerated OpenGL Context.");
		win->src.ctx.native->ctx = wglCreateContext(win->src.hdc);
	}

	ReleaseDC(win->src.window, win->src.hdc);
	win->src.hdc = GetDC(win->src.window);

	if (hints->share) {
		wglShareLists((HGLRC)RGFW_getCurrentContext_OpenGL(), hints->share->ctx);
	}

	wglMakeCurrent(win->src.hdc, win->src.ctx.native->ctx);
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, "OpenGL context initalized.");
	return RGFW_TRUE;
}

void RGFW_window_deleteContextPtr_OpenGL(RGFW_window* win, RGFW_glContext* ctx) {
	wglDeleteContext((HGLRC) ctx->ctx); /*!< delete OpenGL context */
	win->src.ctx.native->ctx = NULL;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, "OpenGL context freed.");
}

void RGFW_window_makeCurrentContext_OpenGL(RGFW_window* win) {
	if (win == NULL)
		wglMakeCurrent(NULL, NULL);
	else
		wglMakeCurrent(win->src.hdc, (HGLRC) win->src.ctx.native->ctx);
}
void* RGFW_getCurrentContext_OpenGL(void) {
	return wglGetCurrentContext();
}
void RGFW_window_swapBuffers_OpenGL(RGFW_window* win) {
	RGFW_ASSERT(win->src.ctx.native);
	SwapBuffers(win->src.hdc);
}

void RGFW_window_swapInterval_OpenGL(RGFW_window* win, i32 swapInterval) {
	RGFW_ASSERT(win != NULL);
    if (wglSwapIntervalEXT == NULL || wglSwapIntervalEXT(swapInterval) == FALSE)
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext, "Failed to set swap interval");
}
#endif

RGFW_bool RGFW_createUTF8FromWideStringWin32(const WCHAR* source, char* output, size_t max) {
    i32 size = 0;
    if (source == NULL) {
        return RGFW_FALSE;
	}
	size = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
	if (!size) {
		return RGFW_FALSE;
	}

	if (size > (i32)max)
		size = (i32)max;

	if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, output, size, NULL, NULL)) {
		return RGFW_FALSE;
	}

	output[size] = 0;
	return RGFW_TRUE;
}

#ifdef RGFW_WEBGPU
WGPUSurface RGFW_window_createSurface_WebGPU(RGFW_window* window, WGPUInstance instance) {
	WGPUSurfaceDescriptor surfaceDesc = {0};
	WGPUSurfaceSourceWindowsHWND fromHwnd = {0};
    fromHwnd.chain.sType = WGPUSType_SurfaceSourceWindowsHWND;
    fromHwnd.hwnd = window->src.window; /* Get HWND from RGFW window source */
    if (!fromHwnd.hwnd) {
        fprintf(stderr, "RGFW Error: HWND is NULL for Windows window.\n");
        return NULL;
    }
    fromHwnd.hinstance = GetModuleHandle(NULL); /* Get current process HINSTANCE */

    surfaceDesc.nextInChain = (WGPUChainedStruct*)&fromHwnd.chain;
    return wgpuInstanceCreateSurface(instance, &surfaceDesc);
}
#endif

#endif /* RGFW_WINDOWS */

/*
	End of Windows defines
*/



/*

	Start of MacOS defines


*/

#if defined(RGFW_MACOS)
/*
	based on silicon.h
	start of cocoa wrapper
*/

#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <mach/mach_time.h>
#include <CoreVideo/CoreVideo.h>

#ifndef  __OBJC__
typedef CGRect NSRect;
typedef CGPoint NSPoint;
typedef CGSize NSSize;

typedef const char* NSPasteboardType;
typedef unsigned long NSUInteger;
typedef long NSInteger;
typedef NSInteger NSModalResponse;

typedef enum NSApplicationActivationPolicy {
	NSApplicationActivationPolicyRegular,
	NSApplicationActivationPolicyAccessory,
	NSApplicationActivationPolicyProhibited
} NSApplicationActivationPolicy;

typedef RGFW_ENUM(u32, NSBackingStoreType) {
	NSBackingStoreRetained = 0,
		NSBackingStoreNonretained = 1,
		NSBackingStoreBuffered = 2
};

typedef RGFW_ENUM(u32, NSWindowStyleMask) {
	NSWindowStyleMaskBorderless = 0,
		NSWindowStyleMaskTitled = 1 << 0,
		NSWindowStyleMaskClosable = 1 << 1,
		NSWindowStyleMaskMiniaturizable = 1 << 2,
		NSWindowStyleMaskResizable = 1 << 3,
		NSWindowStyleMaskTexturedBackground = 1 << 8, /* deprecated */
		NSWindowStyleMaskUnifiedTitleAndToolbar = 1 << 12,
		NSWindowStyleMaskFullScreen = 1 << 14,
		NSWindowStyleMaskFullSizeContentView = 1 << 15,
		NSWindowStyleMaskUtilityWindow = 1 << 4,
		NSWindowStyleMaskDocModalWindow = 1 << 6,
		NSWindowStyleMaskNonactivatingpanel = 1 << 7,
		NSWindowStyleMaskHUDWindow = 1 << 13
};

#define NSPasteboardTypeString "public.utf8-plain-text"

typedef RGFW_ENUM(i32, NSDragOperation) {
	NSDragOperationNone = 0,
		NSDragOperationCopy = 1,
		NSDragOperationLink = 2,
		NSDragOperationGeneric = 4,
		NSDragOperationPrivate = 8,
		NSDragOperationMove = 16,
		NSDragOperationDelete = 32,
		NSDragOperationEvery = (int)ULONG_MAX
};

typedef RGFW_ENUM(NSInteger, NSOpenGLContextParameter) {
	NSOpenGLContextParameterSwapInterval            = 222, /* 1 param.  0 -> Don't sync, 1 -> Sync to vertical retrace     */
		NSOpenGLContextParametectxaceOrder            = 235, /* 1 param.  1 -> Above Window (default), -1 -> Below Window    */
		NSOpenGLContextParametectxaceOpacity          = 236, /* 1 param.  1-> Surface is opaque (default), 0 -> non-opaque   */
		NSOpenGLContextParametectxaceBackingSize      = 304, /* 2 params.  Width/height of surface backing size              */
		NSOpenGLContextParameterReclaimResources        = 308, /* 0 params.                                                    */
		NSOpenGLContextParameterCurrentRendererID       = 309, /* 1 param.   Retrieves the current renderer ID                 */
		NSOpenGLContextParameterGPUVertexProcessing     = 310, /* 1 param.   Currently processing vertices with GPU (get)      */
		NSOpenGLContextParameterGPUFragmentProcessing   = 311, /* 1 param.   Currently processing fragments with GPU (get)     */
		NSOpenGLContextParameterHasDrawable             = 314, /* 1 param.   Boolean returned if drawable is attached          */
		NSOpenGLContextParameterMPSwapsInFlight         = 315, /* 1 param.   Max number of swaps queued by the MP GL engine    */

		NSOpenGLContextParameterSwapRectangle API_DEPRECATED("", macos(10.0, 10.14)) = 200, /* 4 params.  Set or get the swap rectangle {x, y, w, h} */
		NSOpenGLContextParameterSwapRectangleEnable API_DEPRECATED("", macos(10.0, 10.14)) = 201, /* Enable or disable the swap rectangle */
		NSOpenGLContextParameterRasterizationEnable API_DEPRECATED("", macos(10.0, 10.14)) = 221, /* Enable or disable all rasterization */
		NSOpenGLContextParameterStateValidation API_DEPRECATED("", macos(10.0, 10.14)) = 301, /* Validate state for multi-screen functionality */
		NSOpenGLContextParametectxaceSurfaceVolatile API_DEPRECATED("", macos(10.0, 10.14)) = 306, /* 1 param.   Surface volatile state */
};

typedef RGFW_ENUM(NSInteger, NSWindowButton) {
    NSWindowCloseButton            = 0,
    NSWindowMiniaturizeButton      = 1,
    NSWindowZoomButton             = 2,
    NSWindowToolbarButton          = 3,
    NSWindowDocumentIconButton     = 4,
    NSWindowDocumentVersionsButton = 6,
    NSWindowFullScreenButton       = 7,
};

#define NSPasteboardTypeURL "public.url"
#define NSPasteboardTypeFileURL "public.file-url"
#define NSTrackingMouseEnteredAndExited   0x01
#define NSTrackingMouseMoved              0x02
#define NSTrackingCursorUpdate            0x04
#define NSTrackingActiveWhenFirstResponder 0x10
#define NSTrackingActiveInKeyWindow       0x20
#define NSTrackingActiveInActiveApp       0x40
#define NSTrackingActiveAlways            0x80
#define NSTrackingAssumeInside            0x100
#define NSTrackingInVisibleRect           0x200
#define NSTrackingEnabledDuringMouseDrag  0x400
enum {
	NSOpenGLPFAAllRenderers =   1,	/* choose from all available renderers          */
	NSOpenGLPFATripleBuffer =   3,	/* choose a triple buffered pixel format        */
	NSOpenGLPFADoubleBuffer      =   5,	/* choose a double buffered pixel format        */
	NSOpenGLPFAAuxBuffers   =   7,	/* number of aux buffers                        */
	NSOpenGLPFAColorSize       =   8,	/* number of color buffer bits                  */
	NSOpenGLPFAAlphaSize  =  11,	/* number of alpha component bits               */
	NSOpenGLPFADepthSize   =  12,	/* number of depth buffer bits                  */
	NSOpenGLPFAStencilSize   =  13,	/* number of stencil buffer bits                */
	NSOpenGLPFAAccumSize       =  14,	/* number of accum buffer bits                  */
	NSOpenGLPFAMinimumPolicy   =  51,	/* never choose smaller buffers than requested  */
	NSOpenGLPFAMaximumPolicy =  52,	/* choose largest buffers of type requested     */
	NSOpenGLPFASampleBuffers     =  55,	/* number of multi sample buffers               */
	NSOpenGLPFASamples            =  56,	/* number of samples per multi sample buffer    */
	NSOpenGLPFAAuxDepthStencil     =  57,	/* each aux buffer has its own depth stencil    */
	NSOpenGLPFAColorFloat   =  58,	/* color buffers store floating point pixels    */
	NSOpenGLPFAMultisample   =  59,    /* choose multisampling                         */
	NSOpenGLPFASupersample    =  60,    /* choose supersampling                         */
	NSOpenGLPFASampleAlpha     =  61,    /* request alpha filtering                      */
	NSOpenGLPFARendererID       =  70,	/* request renderer by ID                       */
	NSOpenGLPFANoRecovery   =  72,	/* disable all failure recovery systems         */
	NSOpenGLPFAAccelerated      =  73,	/* choose a hardware accelerated renderer       */
	NSOpenGLPFAClosestPolicy   =  74,	/* choose the closest color buffer to request   */
	NSOpenGLPFABackingStore     =  76,	/* back buffer contents are valid after swap    */
	NSOpenGLPFAScreenMask     =  84,	/* bit mask of supported physical screens       */
	NSOpenGLPFAAllowOfflineRenderers  =  96,  /* allow use of offline renderers               */
	NSOpenGLPFAAcceleratedCompute   =  97,	/* choose a hardware accelerated compute device */
	NSOpenGLPFAOpenGLProfile      =  99,    /* specify an OpenGL Profile to use             */
	NSOpenGLProfileVersionLegacy     = 0x1000, /* The requested profile is a legacy (pre-OpenGL 3.0) profile. */
	NSOpenGLProfileVersion3_2Core    = 0x3200, /* The 3.2 Profile of OpenGL */
	NSOpenGLProfileVersion4_1Core  = 0x3200, /* The 4.1 profile of OpenGL */
	NSOpenGLPFAVirtualScreenCount      = 128,	/* number of virtual screens in this format     */
	NSOpenGLPFAStereo                 =   6,
	NSOpenGLPFAOffScreen             =  53,
	NSOpenGLPFAFullScreen           =  54,
	NSOpenGLPFASingleRenderer      =  71,
	NSOpenGLPFARobust                 =  75,
	NSOpenGLPFAMPSafe           =  78,
	NSOpenGLPFAWindow                 =  80,
	NSOpenGLPFAMultiScreen =    81,
	NSOpenGLPFACompliant   =   83,
	NSOpenGLPFAPixelBuffer            =  90,
	NSOpenGLPFARemotePixelBuffer =  91,
};

typedef RGFW_ENUM(u32, NSEventType) {        /* various types of events */
	NSEventTypeApplicationDefined = 15,
};
typedef unsigned long long NSEventMask;

typedef enum NSEventModifierFlags {
	NSEventModifierFlagCapsLock = 1 << 16,
	NSEventModifierFlagShift = 1 << 17,
	NSEventModifierFlagControl = 1 << 18,
	NSEventModifierFlagOption = 1 << 19,
	NSEventModifierFlagCommand = 1 << 20,
	NSEventModifierFlagNumericPad = 1 << 21
} NSEventModifierFlags;

typedef RGFW_ENUM(NSUInteger, NSBitmapFormat) {
	NSBitmapFormatAlphaFirst = 1 << 0,       /* 0 means is alpha last (RGBA, CMYKA, etc.) */
		NSBitmapFormatAlphaNonpremultiplied = 1 << 1,       /* 0 means is premultiplied */
		NSBitmapFormatFloatingpointSamples = 1 << 2,  /* 0 is integer */

		NSBitmapFormatSixteenBitLittleEndian = (1 << 8),
		NSBitmapFormatThirtyTwoBitLittleEndian = (1 << 9),
		NSBitmapFormatSixteenBitBigEndian = (1 << 10),
		NSBitmapFormatThirtyTwoBitBigEndian = (1 << 11)
};

#else
#import <AppKit/AppKit.h>
#include <Foundation/Foundation.h>
#endif /* notdef __OBJC__ */

#ifdef __arm64__
	/* ARM just uses objc_msgSend */
#define abi_objc_msgSend_stret objc_msgSend
#define abi_objc_msgSend_fpret objc_msgSend
#else /* __i386__ */
	/* x86 just uses abi_objc_msgSend_fpret and (NSColor *)objc_msgSend_id respectively */
#define abi_objc_msgSend_stret objc_msgSend_stret
#define abi_objc_msgSend_fpret objc_msgSend_fpret
#endif

#define NSAlloc(nsclass) objc_msgSend_id((id)nsclass, sel_registerName("alloc"))
#define objc_msgSend_bool(x, y)			((BOOL (*)(id, SEL))objc_msgSend) ((id)(x), (SEL)y)
#define objc_msgSend_void(x, y)			((void (*)(id, SEL))objc_msgSend) ((id)(x), (SEL)y)
#define objc_msgSend_void_id(x, y, z)		((void (*)(id, SEL, id))objc_msgSend) ((id)x, (SEL)y, (id)z)
#define objc_msgSend_uint(x, y)			((NSUInteger (*)(id, SEL))objc_msgSend)  ((id)(x), (SEL)y)
#define objc_msgSend_void_bool(x, y, z)		((void (*)(id, SEL, BOOL))objc_msgSend)  ((id)(x), (SEL)y, (BOOL)z)
#define objc_msgSend_bool_void(x, y)		((BOOL (*)(id, SEL))objc_msgSend)  ((id)(x), (SEL)y)
#define objc_msgSend_void_SEL(x, y, z)		((void (*)(id, SEL, SEL))objc_msgSend)  ((id)(x), (SEL)y, (SEL)z)
#define objc_msgSend_id(x, y)				((id (*)(id, SEL))objc_msgSend)  ((id)(x), (SEL)y)
#define objc_msgSend_id_id(x, y, z)			((id (*)(id, SEL, id))objc_msgSend)  ((id)(x), (SEL)y, (id)z)
#define objc_msgSend_id_bool(x, y, z)			((BOOL (*)(id, SEL, id))objc_msgSend)  ((id)(x), (SEL)y, (id)z)
#define objc_msgSend_int(x, y, z) 				((id (*)(id, SEL, int))objc_msgSend)  ((id)(x), (SEL)y, (int)z)
#define objc_msgSend_arr(x, y, z)				 	((id (*)(id, SEL, int))objc_msgSend)  ((id)(x), (SEL)y, (int)z)
#define objc_msgSend_ptr(x, y, z) 					((id (*)(id, SEL, void*))objc_msgSend)  ((id)(x), (SEL)y, (void*)z)
#define objc_msgSend_class(x, y) 					((id (*)(Class, SEL))objc_msgSend)  ((Class)(x), (SEL)y)
#define objc_msgSend_class_char(x, y, z) 			((id (*)(Class, SEL, char*))objc_msgSend)  ((Class)(x), (SEL)y, (char*)z)

#define NSRelease(obj) objc_msgSend_void((id)obj, sel_registerName("release"))
RGFWDEF id NSString_stringWithUTF8String(const char* str);
id NSString_stringWithUTF8String(const char* str) {
	return ((id(*)(id, SEL, const char*))objc_msgSend) ((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), str);
}

const char* NSString_to_char(id str);
const char* NSString_to_char(id str) {
	return ((const char* (*)(id, SEL)) objc_msgSend) ((id)(id)str, sel_registerName("UTF8String"));
}

unsigned char* NSBitmapImageRep_bitmapData(id imageRep);
unsigned char* NSBitmapImageRep_bitmapData(id imageRep) {
	return ((unsigned char* (*)(id, SEL))objc_msgSend) ((id)imageRep, sel_registerName("bitmapData"));
}

id NSBitmapImageRep_initWithBitmapData(unsigned char** planes, NSInteger width, NSInteger height, NSInteger bps, NSInteger spp, bool alpha, bool isPlanar, const char* colorSpaceName, NSBitmapFormat bitmapFormat, NSInteger rowBytes, NSInteger pixelBits);
id NSBitmapImageRep_initWithBitmapData(unsigned char** planes, NSInteger width, NSInteger height, NSInteger bps, NSInteger spp, bool alpha, bool isPlanar, const char* colorSpaceName, NSBitmapFormat bitmapFormat, NSInteger rowBytes, NSInteger pixelBits) {
	SEL func = sel_registerName("initWithBitmapDataPlanes:pixelsWide:pixelsHigh:bitsPerSample:samplesPerPixel:hasAlpha:isPlanar:colorSpaceName:bitmapFormat:bytesPerRow:bitsPerPixel:");

	return (id) ((id(*)(id, SEL, unsigned char**, NSInteger, NSInteger, NSInteger, NSInteger, bool, bool, id, NSBitmapFormat, NSInteger, NSInteger))objc_msgSend)
		(NSAlloc((id)objc_getClass("NSBitmapImageRep")), func, planes, width, height, bps, spp, alpha, isPlanar, NSString_stringWithUTF8String(colorSpaceName), bitmapFormat, rowBytes, pixelBits);
}

id NSColor_colorWithSRGB(CGFloat red, CGFloat green, CGFloat blue, CGFloat alpha);
id NSColor_colorWithSRGB(CGFloat red, CGFloat green, CGFloat blue, CGFloat alpha) {
	Class nsclass = objc_getClass("NSColor");
	SEL func = sel_registerName("colorWithSRGBRed:green:blue:alpha:");
	return ((id(*)(id, SEL, CGFloat, CGFloat, CGFloat, CGFloat))objc_msgSend)
		((id)nsclass, func, red, green, blue, alpha);
}

id NSPasteboard_generalPasteboard(void);
id NSPasteboard_generalPasteboard(void) {
	return (id) objc_msgSend_id((id)objc_getClass("NSPasteboard"), sel_registerName("generalPasteboard"));
}

id* cstrToNSStringArray(char** strs, size_t len);
id* cstrToNSStringArray(char** strs, size_t len) {
	static id nstrs[6];
	size_t i;
	for (i = 0; i < len; i++)
		nstrs[i] = NSString_stringWithUTF8String(strs[i]);

	return nstrs;
}

const char* NSPasteboard_stringForType(id pasteboard, NSPasteboardType dataType, size_t* len);
const char* NSPasteboard_stringForType(id pasteboard, NSPasteboardType dataType, size_t* len) {
	SEL func = sel_registerName("stringForType:");
	id nsstr = NSString_stringWithUTF8String((const char*)dataType);
	id nsString = ((id(*)(id, SEL, id))objc_msgSend)(pasteboard, func, nsstr);
	const char* str = NSString_to_char(nsString);
	if (len != NULL)
		*len = (size_t)((NSUInteger(*)(id, SEL, int))objc_msgSend)(nsString, sel_registerName("maximumLengthOfBytesUsingEncoding:"), 4);
	return str;
}

id c_array_to_NSArray(void* array, size_t len);
id c_array_to_NSArray(void* array, size_t len) {
	return ((id (*)(id, SEL, void*, NSUInteger))objc_msgSend) (NSAlloc(objc_getClass("NSArray")), sel_registerName("initWithObjects:count:"), array, len);
}


void NSregisterForDraggedTypes(id view, NSPasteboardType* newTypes, size_t len);
void NSregisterForDraggedTypes(id view, NSPasteboardType* newTypes, size_t len) {
	id* ntypes = cstrToNSStringArray((char**)newTypes, len);

	id array = c_array_to_NSArray(ntypes, len);
	objc_msgSend_void_id(view, sel_registerName("registerForDraggedTypes:"), array);
	NSRelease(array);
}

NSInteger NSPasteBoard_declareTypes(id pasteboard, NSPasteboardType* newTypes, size_t len, void* owner);
NSInteger NSPasteBoard_declareTypes(id pasteboard, NSPasteboardType* newTypes, size_t len, void* owner) {
	id* ntypes = cstrToNSStringArray((char**)newTypes, len);

	SEL func = sel_registerName("declareTypes:owner:");

	id array = c_array_to_NSArray(ntypes, len);

	NSInteger output = ((NSInteger(*)(id, SEL, id, void*))objc_msgSend)
		(pasteboard, func, array, owner);
	NSRelease(array);

	return output;
}

#define NSRetain(obj) objc_msgSend_void((id)obj, sel_registerName("retain"))

/*
	End of cocoa wrapper
*/

static id RGFW__osxCustomInitWithRGFWWindow(id self, SEL _cmd, RGFW_window* win) {
	RGFW_UNUSED(_cmd);
    struct objc_super s = { self, class_getSuperclass(object_getClass(self)) };
    self = ((id (*)(struct objc_super*, SEL))objc_msgSendSuper)(&s, sel_registerName("init"));

    if (self != nil) {
        object_setInstanceVariable(self, "RGFW_window", win);
        object_setInstanceVariable(self, "trackingArea", nil);

        object_setInstanceVariable(
            self, "markedText",
            ((id (*)(id, SEL))objc_msgSend)(
                ((id (*)(Class, SEL))objc_msgSend)(objc_getClass("NSMutableAttributedString"), sel_registerName("alloc")),
                sel_registerName("init")
            )
        );

        ((void (*)(id, SEL))objc_msgSend)(self, sel_registerName("updateTrackingAreas"));

        ((void (*)(id, SEL, id))objc_msgSend)(
            self, sel_registerName("registerForDraggedTypes:"),
            ((id (*)(Class, SEL, id))objc_msgSend)(
                objc_getClass("NSArray"),
                sel_registerName("arrayWithObject:"),
                ((id (*)(Class, SEL, const char*))objc_msgSend)(
                    objc_getClass("NSString"),
                    sel_registerName("stringWithUTF8String:"),
                    "public.url"
                )
            )
        );
    }

    return self;
}

static u32 RGFW_OnClose(id self) {
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, (const char*)"RGFW_window", (void**)&win);
	if (win == NULL)
		return true;

	RGFW_window_setShouldClose(win, RGFW_TRUE);
	RGFW_eventQueuePushEx(e.type = RGFW_quit; e.common.win = win);
	RGFW_windowQuitCallback(win);

	return false;
}

/* NOTE(EimaMei): Fixes the constant clicking when the app is running under a terminal. */
static bool RGFW__osxAcceptsFirstResponder(void) { return true; }
static bool RGFW__osxPerformKeyEquivalent(id event) { RGFW_UNUSED(event); return true; }

static NSDragOperation RGFW__osxDraggingEntered(id self, SEL sel, id sender) {
	RGFW_UNUSED(sender); RGFW_UNUSED(self); RGFW_UNUSED(sel);

	return NSDragOperationCopy;
}
static NSDragOperation RGFW__osxDraggingUpdated(id self, SEL sel, id sender) {
	RGFW_UNUSED(sel);

	RGFW_window* win = NULL;

	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL || (!(win->internal.flags & RGFW_windowAllowDND)))
		return 0;
	if (!(win->internal.enabledEvents & RGFW_dataDragFlag)) return NSDragOperationCopy;

	NSPoint p = ((NSPoint(*)(id, SEL)) objc_msgSend)(sender, sel_registerName("draggingLocation"));
	RGFW_eventQueuePushEx(e.type = RGFW_dataDrag;
									e.mouse.x = (i32)p.x;  e.mouse.y = (i32)(win->h - p.y);
									e.common.win = win);

	_RGFW->windowState.win = win;
	_RGFW->windowState.dataDragging = RGFW_TRUE;
	_RGFW->windowState.dropX = (i32)p.x;
	_RGFW->windowState.dropY = (i32)(win->h - p.y);

	RGFW_dataDragCallback(win, (i32) p.x, (i32) (win->h - p.y));
	return NSDragOperationCopy;
}
static bool RGFW__osxPrepareForDragOperation(id self) {
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL || (!(win->internal.enabledEvents & RGFW_dataDropFlag)))
		return true;

	if (!(win->internal.flags & RGFW_windowAllowDND)) {
		return false;
	}

	return true;
}

void RGFW__osxDraggingEnded(id self, SEL sel, id sender);
void RGFW__osxDraggingEnded(id self, SEL sel, id sender) { RGFW_UNUSED(sender); RGFW_UNUSED(self); RGFW_UNUSED(sel);  return; }

static bool RGFW__osxPerformDragOperation(id self, SEL sel, id sender) {
	RGFW_UNUSED(sender); RGFW_UNUSED(self); RGFW_UNUSED(sel);

	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);

	if (win == NULL || (!(win->internal.enabledEvents & RGFW_dataDropFlag)))
		return false;

	/* id pasteBoard = objc_msgSend_id(sender, sel_registerName("draggingPasteboard")); */

	id pasteBoard = objc_msgSend_id(sender, sel_registerName("draggingPasteboard"));

	/* Get the types of data available on the pasteboard */
	id types = objc_msgSend_id(pasteBoard, sel_registerName("types"));

	/* Get the string type for file URLs */
	id fileURLsType = objc_msgSend_class_char(objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), "NSFilenamesPboardType");

	/* Check if the pasteboard contains file URLs */
	if (objc_msgSend_id_bool(types, sel_registerName("containsObject:"), fileURLsType) == 0) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errClipboard, "No files found on the pasteboard.");
		return 0;
	}

	id fileURLs = objc_msgSend_id_id(pasteBoard, sel_registerName("propertyListForType:"), fileURLsType);
	int count = ((int (*)(id, SEL))objc_msgSend)(fileURLs, sel_registerName("count"));

	if (count == 0)
		return 0;

	RGFW_event event;
	event.drop.files = (char**)(void*)_RGFW->files;

	u32 i;
    for (i = 0; i < (u32)count; i++) {
		id fileURL = objc_msgSend_arr(fileURLs, sel_registerName("objectAtIndex:"), i);
		const char *filePath = ((const char* (*)(id, SEL))objc_msgSend)(fileURL, sel_registerName("UTF8String"));
		RGFW_STRNCPY(event.drop.files[i], filePath, RGFW_MAX_PATH - 1);
		event.drop.files[i][RGFW_MAX_PATH - 1] = '\0';
	}

	event.drop.count = (size_t)count;
	RGFW_eventQueuePushEx(e.type = RGFW_dataDrop;
									e.drop.count = (size_t)count;
									e.drop.files = event.drop.files;
									e.common.win = win);

	_RGFW->windowState.win = win;
	_RGFW->windowState.dataDrop = RGFW_TRUE;
	_RGFW->windowState.filesCount =  event.drop.count;
	RGFW_dataDropCallback(win, event.drop.files, event.drop.count);

	return false;
}

#ifndef RGFW_NO_IOKIT
#include <IOKit/IOKitLib.h>

u32 RGFW_osx_getFallbackRefreshRate(CGDirectDisplayID displayID);
u32 RGFW_osx_getFallbackRefreshRate(CGDirectDisplayID displayID) {
    u32 refreshRate = 0;
    io_iterator_t it;
    io_service_t service;
    CFNumberRef indexRef, clockRef, countRef;
    u32 clock, count;

#ifdef kIOMainPortDefault
    if (IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching("IOFramebuffer"), &it) != 0)
#elif defined(kIOMasterPortDefault)
    if (IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching("IOFramebuffer"), &it) != 0)
#endif
        return RGFW_FALSE;

    while ((service = IOIteratorNext(it)) != 0) {
        u32 index;
        indexRef = (CFNumberRef)IORegistryEntryCreateCFProperty(service, CFSTR("IOFramebufferOpenGLIndex"), kCFAllocatorDefault, kNilOptions);
        if (indexRef == 0) continue;

        if (CFNumberGetValue(indexRef, kCFNumberIntType, &index) && CGOpenGLDisplayMaskToDisplayID(1 << index) == displayID) {
            CFRelease(indexRef);
            break;
        }

        CFRelease(indexRef);
    }

    if (service) {
        clockRef = (CFNumberRef)IORegistryEntryCreateCFProperty(service, CFSTR("IOFBCurrentPixelClock"), kCFAllocatorDefault, kNilOptions);
        if (clockRef) {
            if (CFNumberGetValue(clockRef, kCFNumberIntType, &clock) && clock) {
                countRef = (CFNumberRef)IORegistryEntryCreateCFProperty(service, CFSTR("IOFBCurrentPixelCount"), kCFAllocatorDefault, kNilOptions);
                if (countRef && CFNumberGetValue(countRef, kCFNumberIntType, &count) && count) {
					float rate = (float)((double)clock / (double) count);
					refreshRate = (u32)RGFW_ROUND(rate);
                    CFRelease(countRef);
                }
            }
            CFRelease(clockRef);
        }
    }

    IOObjectRelease(it);
    return refreshRate;
}
#endif

void RGFW_moveToMacOSResourceDir(void) {
	char resourcesPath[256];

	CFBundleRef bundle = CFBundleGetMainBundle();
	if (!bundle)
		return;

	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);
	CFStringRef last = CFURLCopyLastPathComponent(resourcesURL);

	if (
		CFStringCompare(CFSTR("Resources"), last, 0) != kCFCompareEqualTo ||
		CFURLGetFileSystemRepresentation(resourcesURL, true, (u8*) resourcesPath, 255) == 0
		) {
		CFRelease(last);
		CFRelease(resourcesURL);
		return;
	}

	CFRelease(last);
	CFRelease(resourcesURL);

	chdir(resourcesPath);
}


static void RGFW__osxWindowDeminiaturize(id self, SEL sel) {
	RGFW_UNUSED(sel);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

	win->internal.flags |= RGFW_windowMinimize;
	if (!(win->internal.enabledEvents & RGFW_windowMinimizedFlag)) return;
	RGFW_eventQueuePushEx(e.type = RGFW_windowRestored; e.common.win = win);
	RGFW_windowRestoredCallback(win, win->x, win->y, win->w, win->h);

}
static void RGFW__osxWindowMiniaturize(id self, SEL sel) {
	RGFW_UNUSED(sel);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

	win->internal.flags &= ~(u32)RGFW_windowMinimize;
	if (!(win->internal.enabledEvents & RGFW_windowMinimizedFlag)) return;
	RGFW_eventQueuePushEx(e.type = RGFW_windowMinimized; e.common.win = win);
	RGFW_windowMinimizedCallback(win);

}

static void RGFW__osxWindowBecameKey(id self, SEL sel) {
	RGFW_UNUSED(sel);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;


	win->internal.inFocus = RGFW_TRUE;
	if ((win->internal.holdMouse)) RGFW_window_holdMouse(win);
	if (!(win->internal.enabledEvents & RGFW_focusInFlag)) return;

	RGFW_eventQueuePushEx(e.type = RGFW_focusIn; e.common.win = win);
	RGFW_focusCallback(win, RGFW_TRUE);
}

static void RGFW__osxWindowResignKey(id self, SEL sel) {
	RGFW_UNUSED(sel);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

    RGFW_window_focusLost(win);
	if (!(win->internal.enabledEvents & RGFW_focusOutFlag)) return;

    RGFW_eventQueuePushEx(e.type = RGFW_focusOut; e.common.win = win);
	RGFW_focusCallback(win, RGFW_FALSE);
}

static void RGFW__osxDidWindowResize(id self, SEL _cmd, id notification) {
	RGFW_UNUSED(_cmd); RGFW_UNUSED(notification);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

	NSRect frame;
	if (win->src.view) frame = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)((id)win->src.view, sel_registerName("frame"));
	else return;

	if (frame.size.width == 0 || frame.size.height == 0) return;
	win->w = (i32)frame.size.width;
	win->h = (i32)frame.size.height;

	RGFW_monitor mon = RGFW_window_getMonitor(win);
	if ((i32)mon.mode.w == win->w && (i32)mon.mode.h - 102 <= win->h) {
		win->internal.flags |= RGFW_windowMaximize;
		if (!(win->internal.enabledEvents & RGFW_windowMaximizedFlag)) return;
		RGFW_eventQueuePushEx(e.type = RGFW_windowMaximized; e.common.win = win);
		RGFW_windowMaximizedCallback(win, 0, 0, win->w, win->h);
	} else if (win->internal.flags & RGFW_windowMaximize) {
		win->internal.flags &= ~(u32)RGFW_windowMaximize;
		if (!(win->internal.enabledEvents & RGFW_windowRestoredFlag)) return;
		RGFW_eventQueuePushEx(e.type = RGFW_windowRestored; e.common.win = win);
		RGFW_windowRestoredCallback(win, win->x, win->y, win->w, win->h);

	}

	if (!(win->internal.enabledEvents & RGFW_windowResizedFlag)) return;

	RGFW_eventQueuePushEx(e.type = RGFW_windowResized; e.common.win = win);
	RGFW_windowResizedCallback(win, win->w, win->h);
}

static void RGFW__osxWindowMove(id self, SEL sel) {
	RGFW_UNUSED(sel);

	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

	NSRect frame = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)((id)win->src.window, sel_registerName("frame"));
	win->x = (i32) frame.origin.x;
	win->y = (i32) frame.origin.y;

	if (!(win->internal.enabledEvents & RGFW_windowMovedFlag)) return;
	RGFW_eventQueuePushEx(e.type = RGFW_windowMoved; e.common.win = win);
	RGFW_windowMovedCallback(win, win->x, win->y);
}

static void RGFW__osxViewDidChangeBackingProperties(id self, SEL _cmd) {
	RGFW_UNUSED(_cmd);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL || !(win->internal.enabledEvents & RGFW_scaleUpdatedFlag)) return;

	RGFW_monitor mon = RGFW_window_getMonitor(win);
	RGFW_scaleUpdatedCallback(win, mon.scaleX, mon.scaleY);
	RGFW_eventQueuePushEx(e.type = RGFW_scaleUpdated; e.scale.x = mon.scaleX; e.scale.y = mon.scaleY ; e.common.win = win);
}

static BOOL RGFW__osxWantsUpdateLayer(id self, SEL _cmd) { RGFW_UNUSED(self); RGFW_UNUSED(_cmd); return YES; }

static void RGFW__osxUpdateLayer(id self, SEL _cmd) {
	RGFW_UNUSED(self); RGFW_UNUSED(_cmd);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL || !(win->internal.enabledEvents & RGFW_windowRefreshFlag)) return;
	RGFW_windowRefreshCallback(win);
}

static void RGFW__osxDrawRect(id self, SEL _cmd, CGRect rect) {
	RGFW_UNUSED(rect); RGFW_UNUSED(_cmd);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL || !(win->internal.enabledEvents & RGFW_scaleUpdatedFlag)) return;

	RGFW_eventQueuePushEx(e.type = RGFW_windowRefresh; e.common.win = win);
	RGFW_windowRefreshCallback(win);
}

static void RGFW__osxMouseEntered(id self, SEL _cmd, id event) {
	RGFW_UNUSED(_cmd);
	RGFW_window* win = NULL;
    object_getInstanceVariable(self, "RGFW_window", (void**)&win);
    if (win == NULL || !(win->internal.enabledEvents & RGFW_mouseEnterFlag)) return;

	win->internal.mouseInside = RGFW_TRUE;
	_RGFW->windowState.win = win;
	_RGFW->windowState.mouseEnter =  RGFW_TRUE;

	RGFW_event e;
    e.type = RGFW_mouseEnter;
    NSPoint p = ((NSPoint(*)(id, SEL))objc_msgSend)(event, sel_registerName("locationInWindow"));
    e.mouse.x = (i32)p.x;
	e.mouse.y = (i32)(win->h - p.y);
    e.common.win = win;

    RGFW_eventQueuePush(&e);
    RGFW_mouseNotifyCallback(win, e.mouse.x, e.mouse.y, 1);
}

static void RGFW__osxMouseExited(id self, SEL _cmd, id event) {
	RGFW_UNUSED(_cmd); RGFW_UNUSED(event);
	RGFW_window* win = NULL;
    object_getInstanceVariable(self, "RGFW_window", (void**)&win);
    if (win == NULL || !(win->internal.enabledEvents & RGFW_mouseLeaveFlag)) return;


	win->internal.mouseInside = RGFW_FALSE;
	_RGFW->windowState.winLeave = win;
	_RGFW->windowState.mouseLeave = RGFW_TRUE;

    RGFW_event e;
    e.type = RGFW_mouseLeave;
	e.mouse.x = 0;
	e.mouse.y = 0;
	e.common.win = win;

    RGFW_eventQueuePush(&e);
    RGFW_mouseNotifyCallback(win, e.mouse.x, e.mouse.y, 0);
}

static void RGFW__osxKeyDown(id self, SEL _cmd, id event) {
	RGFW_UNUSED(_cmd);
	RGFW_window* win = NULL;
    object_getInstanceVariable(self, "RGFW_window", (void**)&win);
    if (win == NULL || !(win->internal.enabledEvents & RGFW_keyPressedFlag)) return;

    RGFW_event e;
    u32 key = (u16)((u32(*)(id, SEL))objc_msgSend)(event, sel_registerName("keyCode"));
    u32 mappedKey = (u32)*(((char*)(const char*)NSString_to_char(((id(*)(id, SEL))objc_msgSend)(event, sel_registerName("charactersIgnoringModifiers")))));
    if ((u8)mappedKey == 239) mappedKey = 0;

    e.key.sym = (u8)mappedKey;
    e.key.value = (u8)RGFW_apiKeyToRGFW(key);
    _RGFW->keyboard[e.key.value].prev = _RGFW->keyboard[e.key.value].current;
    e.type = RGFW_keyPressed;
    e.key.repeat = RGFW_window_isKeyPressed(win, e.key.value);
    _RGFW->keyboard[e.key.value].current = 1;
    e.common.win = win;

    RGFW_eventQueuePush(&e);
    RGFW_keyCallback(win, e.key.value, e.key.sym, win->internal.mod, e.key.repeat, 1);
}

static void RGFW__osxKeyUp(id self, SEL _cmd, id event) {
	RGFW_UNUSED(_cmd);
	RGFW_window* win = NULL;
    object_getInstanceVariable(self, "RGFW_window", (void**)&win);
    if (win == NULL || !(win->internal.enabledEvents & RGFW_keyReleasedFlag)) return;

    RGFW_event e;
    u32 key = (u16)((u32(*)(id, SEL))objc_msgSend)(event, sel_registerName("keyCode"));
    u32 mappedKey = (u32)*(((char*)(const char*)NSString_to_char(((id(*)(id, SEL))objc_msgSend)(event, sel_registerName("charactersIgnoringModifiers")))));
    if ((u8)mappedKey == 239) mappedKey = 0;

    e.key.sym = (u8)mappedKey;
    e.key.value = (u8)RGFW_apiKeyToRGFW(key);
    _RGFW->keyboard[e.key.value].prev = _RGFW->keyboard[e.key.value].current;
    e.type = RGFW_keyReleased;
    e.key.repeat = RGFW_window_isKeyDown(win, (u8)e.key.value);
    _RGFW->keyboard[e.key.value].current = 0;
    e.common.win = win;

    RGFW_eventQueuePush(&e);
    RGFW_keyCallback(win, e.key.value, e.key.sym, win->internal.mod, e.key.repeat, 0);
}

static void RGFW__osxFlagsChanged(id self, SEL _cmd, id event) {
	RGFW_UNUSED(_cmd);
    RGFW_window* win = NULL;
    object_getInstanceVariable(self, "RGFW_window", (void**)&win);
    if (win == NULL) return;

    RGFW_event e;
    u32 flags = (u32)((u32(*)(id, SEL))objc_msgSend)(event, sel_registerName("modifierFlags"));
    RGFW_updateKeyModsEx(win,
                          ((u32)(flags & NSEventModifierFlagCapsLock) % 255),
                          ((flags & NSEventModifierFlagNumericPad) % 255),
                          ((flags & NSEventModifierFlagControl) % 255),
                          ((flags & NSEventModifierFlagOption) % 255),
                          ((flags & NSEventModifierFlagShift) % 255),
                          ((flags & NSEventModifierFlagCommand) % 255), 0);
    u8 i;
    for (i = 0; i < 9; i++)
        _RGFW->keyboard[i + RGFW_capsLock].prev = _RGFW->keyboard[i + RGFW_capsLock].current;

    for (i = 0; i < 5; i++) {
        u32 shift = (1 << (i + 16));
        u32 key = i + RGFW_capsLock;
        if ((flags & shift) && !RGFW_window_isKeyDown(win, (u8)key)) {
            _RGFW->keyboard[key].current = 1;
            if (key != RGFW_capsLock)
                _RGFW->keyboard[key + 4].current = 1;
            e.type = RGFW_keyPressed;
            e.key.value = (u8)key;
            break;
        }
        if (!(flags & shift) && RGFW_window_isKeyDown(win, (u8)key)) {
            _RGFW->keyboard[key].current = 0;
            if (key != RGFW_capsLock)
                _RGFW->keyboard[key + 4].current = 0;
            e.type = RGFW_keyReleased;
            e.key.value = (u8)key;
            break;
        }
    }
    e.key.repeat = RGFW_window_isKeyDown(win, (u8)e.key.value);
    e.common.win = win;

	if (!(win->internal.enabledEvents & (RGFW_BIT(e.type)))) return;
    RGFW_eventQueuePush(&e);
    RGFW_keyCallback(win, e.key.value, e.key.sym, win->internal.mod, e.key.repeat, e.type == RGFW_keyPressed);
}

static void RGFW__osxMouseMoved(id self, SEL _cmd, id event) {
	RGFW_UNUSED(_cmd);
    RGFW_window* win = NULL;
    object_getInstanceVariable(self, "RGFW_window", (void**)&win);
    if (win == NULL || !(win->internal.enabledEvents & RGFW_windowMovedFlag)) return;

    RGFW_event e;
    e.type = RGFW_mousePosChanged;
    NSPoint p = ((NSPoint(*)(id, SEL))objc_msgSend)(event, sel_registerName("locationInWindow"));
    e.mouse.x = (i32)p.x;
	e.mouse.y = (i32)(win->h - p.y);
    p.x = ((CGFloat(*)(id, SEL))abi_objc_msgSend_fpret)(event, sel_registerName("deltaX"));
    p.y = ((CGFloat(*)(id, SEL))abi_objc_msgSend_fpret)(event, sel_registerName("deltaY"));
    e.mouse.vecX = (float)p.x;
	e.mouse.vecY = (float)p.y;
	_RGFW->vectorX = e.mouse.vecX;
	_RGFW->vectorY = e.mouse.vecY;
	win->internal.lastMouseX = e.mouse.x;
    win->internal.lastMouseY = e.mouse.y;
    e.common.win = win;

    RGFW_eventQueuePush(&e);
    RGFW_mousePosCallback(win, e.mouse.x, e.mouse.y, e.mouse.vecX, e.mouse.vecY);
}

static void RGFW__osxMouseDown(id self, SEL _cmd, id event) {
	RGFW_UNUSED(_cmd);
    RGFW_window* win = NULL;
    object_getInstanceVariable(self, "RGFW_window", (void**)&win);
    if (win == NULL || !(win->internal.enabledEvents & RGFW_mouseButtonPressedFlag)) return;

    RGFW_event e;
    u32 buttonNumber = (u32)((u32(*)(id, SEL))objc_msgSend)(event, sel_registerName("buttonNumber"));
    switch (buttonNumber) {
        case 0: e.button.value = RGFW_mouseLeft; break;
        case 1: e.button.value = RGFW_mouseRight; break;
        case 2: e.button.value = RGFW_mouseMiddle; break;
        default: e.button.value = (u8)buttonNumber;
    }
    e.type = RGFW_mouseButtonPressed;
    _RGFW->mouseButtons[e.button.value].prev = _RGFW->mouseButtons[e.button.value].current;
    _RGFW->mouseButtons[e.button.value].current = 1;
    e.common.win = win;

    RGFW_eventQueuePush(&e);
    RGFW_mouseButtonCallback(win, e.button.value, 1);
}

static void RGFW__osxMouseUp(id self, SEL _cmd, id event) {
	RGFW_UNUSED(_cmd);
    RGFW_window* win = NULL;
    object_getInstanceVariable(self, "RGFW_window", (void**)&win);
    if (win == NULL|| !(win->internal.enabledEvents & RGFW_mouseButtonReleasedFlag)) return;

    RGFW_event e;
    u32 buttonNumber = (u32)((u32(*)(id, SEL))objc_msgSend)(event, sel_registerName("buttonNumber"));
    switch (buttonNumber) {
        case 0: e.button.value = RGFW_mouseLeft; break;
        case 1: e.button.value = RGFW_mouseRight; break;
        case 2: e.button.value = RGFW_mouseMiddle; break;
        default: e.button.value = (u8)buttonNumber;
    }
    e.type = RGFW_mouseButtonReleased;
    _RGFW->mouseButtons[e.button.value].prev = _RGFW->mouseButtons[e.button.value].current;
    _RGFW->mouseButtons[e.button.value].current = 0;
    e.common.win = win;

    RGFW_eventQueuePush(&e);
    RGFW_mouseButtonCallback(win, e.button.value, 0);
}

static void RGFW__osxScrollWheel(id self, SEL _cmd, id event) {
	RGFW_UNUSED(_cmd);
    RGFW_window* win = NULL;
    object_getInstanceVariable(self, "RGFW_window", (void**)&win);
    if (win == NULL|| !(win->internal.enabledEvents & RGFW_mouseScroll)) return;

    RGFW_event e;
    float deltaX = (float)((CGFloat(*)(id, SEL))abi_objc_msgSend_fpret)(event, sel_registerName("deltaX"));
	float deltaY = (float)((CGFloat(*)(id, SEL))abi_objc_msgSend_fpret)(event, sel_registerName("deltaY"));

    e.type = RGFW_mouseScroll;
	e.scroll.x = deltaX;
	e.scroll.y = deltaY;
    e.common.win = win;
	_RGFW->scrollX = e.scroll.x;
	_RGFW->scrollY = e.scroll.y;

    RGFW_eventQueuePush(&e);
    RGFW_mouseScrollCallback(win, deltaX, deltaY);
}

RGFW_bool RGFW_createSurfacePtr(u8* data, i32 w, i32 h, RGFW_format format, RGFW_surface* surface) {
	surface->data = data;
	surface->w = w;
	surface->h = h;
	surface->format = format;
	surface->native.format = RGFW_formatRGBA8;
	return RGFW_TRUE;
}

void RGFW_surface_freePtr(RGFW_surface* surface) { RGFW_UNUSED(surface); }

void RGFW_window_blitSurface(RGFW_window* win, RGFW_surface* surface) {
	RGFW_copyImageData(surface->data, surface->w, RGFW_MIN(win->h, surface->h), surface->native.format, surface->data, surface->format);

    size_t depth = (surface->format >= RGFW_formatRGBA8) ? 4 : 3;
	id image = ((id (*)(Class, SEL))objc_msgSend)(objc_getClass("NSImage"), sel_getUid("alloc"));
	NSSize size = (NSSize){(double)surface->w, (double)surface->h};
	image = ((id (*)(id, SEL, NSSize))objc_msgSend)((id)image, sel_getUid("initWithSize:"), size);

	int minX = RGFW_MIN(win->w, surface->w);
	int minY = RGFW_MIN(win->h, surface->h);

	id rep  = NSBitmapImageRep_initWithBitmapData(&surface->data, minX, minY, 8, (i32)depth, (depth == 4), false, "NSDeviceRGBColorSpace", 1 << 1, (u32)surface->w  * (u32)depth, 8 * (u32)depth);
	RGFW_copyImageData(NSBitmapImageRep_bitmapData(rep), minX, minY , RGFW_formatRGBA8, surface->data, surface->format);
	((void (*)(id, SEL, id))objc_msgSend)((id)image, sel_getUid("addRepresentation:"), rep);

	id contentView = ((id (*)(id, SEL))objc_msgSend)((id)win->src.window, sel_getUid("contentView"));
	((void (*)(id, SEL, BOOL))objc_msgSend)(contentView, sel_getUid("setWantsLayer:"), YES);
	id layer = ((id (*)(id, SEL))objc_msgSend)(contentView, sel_getUid("layer"));

	((void (*)(id, SEL, id))objc_msgSend)(layer, sel_getUid("setContents:"), (id)image);
	((void (*)(id, SEL, BOOL))objc_msgSend)(contentView, sel_getUid("setNeedsDisplay:"), YES);

	NSRelease(rep);
	NSRelease(image);
}

void* RGFW_window_getView_OSX(RGFW_window* win) { return win->src.view; }

void RGFW_window_setLayer_OSX(RGFW_window* win, void* layer) {
	objc_msgSend_void_id((id)win->src.view, sel_registerName("setLayer"), (id)layer);
}

void* RGFW_getLayer_OSX(void) {
	return objc_msgSend_class((id)objc_getClass("CAMetalLayer"), (SEL)sel_registerName("layer"));
}
void* RGFW_window_getWindow_OSX(RGFW_window* win) { return win->src.window; }

void RGFW_initKeycodesPlatform(void) {
	_RGFW->keycodes[0x1D] = RGFW_0;
	_RGFW->keycodes[0x12] = RGFW_1;
	_RGFW->keycodes[0x13] = RGFW_2;
	_RGFW->keycodes[0x14] = RGFW_3;
	_RGFW->keycodes[0x15] = RGFW_4;
	_RGFW->keycodes[0x17] = RGFW_5;
	_RGFW->keycodes[0x16] = RGFW_6;
	_RGFW->keycodes[0x1A] = RGFW_7;
	_RGFW->keycodes[0x1C] = RGFW_8;
	_RGFW->keycodes[0x19] = RGFW_9;
	_RGFW->keycodes[0x00] = RGFW_a;
	_RGFW->keycodes[0x0B] = RGFW_b;
	_RGFW->keycodes[0x08] = RGFW_c;
	_RGFW->keycodes[0x02] = RGFW_d;
	_RGFW->keycodes[0x0E] = RGFW_e;
	_RGFW->keycodes[0x03] = RGFW_f;
	_RGFW->keycodes[0x05] = RGFW_g;
	_RGFW->keycodes[0x04] = RGFW_h;
	_RGFW->keycodes[0x22] = RGFW_i;
	_RGFW->keycodes[0x26] = RGFW_j;
	_RGFW->keycodes[0x28] = RGFW_k;
	_RGFW->keycodes[0x25] = RGFW_l;
	_RGFW->keycodes[0x2E] = RGFW_m;
	_RGFW->keycodes[0x2D] = RGFW_n;
	_RGFW->keycodes[0x1F] = RGFW_o;
	_RGFW->keycodes[0x23] = RGFW_p;
	_RGFW->keycodes[0x0C] = RGFW_q;
	_RGFW->keycodes[0x0F] = RGFW_r;
	_RGFW->keycodes[0x01] = RGFW_s;
	_RGFW->keycodes[0x11] = RGFW_t;
	_RGFW->keycodes[0x20] = RGFW_u;
	_RGFW->keycodes[0x09] = RGFW_v;
	_RGFW->keycodes[0x0D] = RGFW_w;
	_RGFW->keycodes[0x07] = RGFW_x;
	_RGFW->keycodes[0x10] = RGFW_y;
	_RGFW->keycodes[0x06] = RGFW_z;
	_RGFW->keycodes[0x27] = RGFW_apostrophe;
	_RGFW->keycodes[0x2A] = RGFW_backSlash;
	_RGFW->keycodes[0x2B] = RGFW_comma;
	_RGFW->keycodes[0x18] = RGFW_equals;
	_RGFW->keycodes[0x32] = RGFW_backtick;
	_RGFW->keycodes[0x21] = RGFW_bracket;
	_RGFW->keycodes[0x1B] = RGFW_minus;
	_RGFW->keycodes[0x2F] = RGFW_period;
	_RGFW->keycodes[0x1E] = RGFW_closeBracket;
	_RGFW->keycodes[0x29] = RGFW_semicolon;
	_RGFW->keycodes[0x2C] = RGFW_slash;
	_RGFW->keycodes[0x0A] = RGFW_world1;
	_RGFW->keycodes[0x33] = RGFW_backSpace;
	_RGFW->keycodes[0x39] = RGFW_capsLock;
	_RGFW->keycodes[0x75] = RGFW_delete;
	_RGFW->keycodes[0x7D] = RGFW_down;
	_RGFW->keycodes[0x77] = RGFW_end;
	_RGFW->keycodes[0x24] = RGFW_enter;
	_RGFW->keycodes[0x35] = RGFW_escape;
	_RGFW->keycodes[0x7A] = RGFW_F1;
	_RGFW->keycodes[0x78] = RGFW_F2;
	_RGFW->keycodes[0x63] = RGFW_F3;
	_RGFW->keycodes[0x76] = RGFW_F4;
	_RGFW->keycodes[0x60] = RGFW_F5;
	_RGFW->keycodes[0x61] = RGFW_F6;
	_RGFW->keycodes[0x62] = RGFW_F7;
	_RGFW->keycodes[0x64] = RGFW_F8;
	_RGFW->keycodes[0x65] = RGFW_F9;
	_RGFW->keycodes[0x6D] = RGFW_F10;
	_RGFW->keycodes[0x67] = RGFW_F11;
	_RGFW->keycodes[0x6F] = RGFW_F12;
	_RGFW->keycodes[0x69] = RGFW_printScreen;
	_RGFW->keycodes[0x6B] = RGFW_F14;
	_RGFW->keycodes[0x71] = RGFW_F15;
	_RGFW->keycodes[0x6A] = RGFW_F16;
	_RGFW->keycodes[0x40] = RGFW_F17;
	_RGFW->keycodes[0x4F] = RGFW_F18;
	_RGFW->keycodes[0x50] = RGFW_F19;
	_RGFW->keycodes[0x5A] = RGFW_F20;
	_RGFW->keycodes[0x73] = RGFW_home;
	_RGFW->keycodes[0x72] = RGFW_insert;
	_RGFW->keycodes[0x7B] = RGFW_left;
	_RGFW->keycodes[0x3A] = RGFW_altL;
	_RGFW->keycodes[0x3B] = RGFW_controlL;
	_RGFW->keycodes[0x38] = RGFW_shiftL;
	_RGFW->keycodes[0x37] = RGFW_superL;
	_RGFW->keycodes[0x6E] = RGFW_menu;
	_RGFW->keycodes[0x47] = RGFW_numLock;
	_RGFW->keycodes[0x79] = RGFW_pageDown;
	_RGFW->keycodes[0x74] = RGFW_pageUp;
	_RGFW->keycodes[0x7C] = RGFW_right;
	_RGFW->keycodes[0x3D] = RGFW_altR;
	_RGFW->keycodes[0x3E] = RGFW_controlR;
	_RGFW->keycodes[0x3C] = RGFW_shiftR;
	_RGFW->keycodes[0x36] = RGFW_superR;
	_RGFW->keycodes[0x31] = RGFW_space;
	_RGFW->keycodes[0x30] = RGFW_tab;
	_RGFW->keycodes[0x7E] = RGFW_up;
	_RGFW->keycodes[0x52] = RGFW_kp0;
	_RGFW->keycodes[0x53] = RGFW_kp1;
	_RGFW->keycodes[0x54] = RGFW_kp2;
	_RGFW->keycodes[0x55] = RGFW_kp3;
	_RGFW->keycodes[0x56] = RGFW_kp4;
	_RGFW->keycodes[0x57] = RGFW_kp5;
	_RGFW->keycodes[0x58] = RGFW_kp6;
	_RGFW->keycodes[0x59] = RGFW_kp7;
	_RGFW->keycodes[0x5B] = RGFW_kp8;
	_RGFW->keycodes[0x5C] = RGFW_kp9;
	_RGFW->keycodes[0x45] = RGFW_kpSlash;
	_RGFW->keycodes[0x41] = RGFW_kpPeriod;
	_RGFW->keycodes[0x4B] = RGFW_kpSlash;
	_RGFW->keycodes[0x4C] = RGFW_kpReturn;
	_RGFW->keycodes[0x51] = RGFW_kpEqual;
	_RGFW->keycodes[0x43] = RGFW_kpMultiply;
	_RGFW->keycodes[0x4E] = RGFW_kpMinus;
}

i32 RGFW_initPlatform(void) {
	class_addMethod(objc_getClass("NSObject"), sel_registerName("windowShouldClose:"), (IMP)(void*)RGFW_OnClose, 0);

	/* NOTE(EimaMei): Fixes the 'Boop' sfx from constantly playing each time you click a key. Only a problem when running in the terminal. */
	class_addMethod(objc_getClass("NSWindowClass"), sel_registerName("acceptsFirstResponder:"), (IMP)(void*)RGFW__osxAcceptsFirstResponder, 0);
	class_addMethod(objc_getClass("NSWindowClass"), sel_registerName("performKeyEquivalent:"), (IMP)(void*)RGFW__osxPerformKeyEquivalent, 0);

	_RGFW->NSApp = objc_msgSend_id((id)objc_getClass("NSApplication"), sel_registerName("sharedApplication"));

	((void (*)(id, SEL, NSUInteger))objc_msgSend)
		((id)_RGFW->NSApp, sel_registerName("setActivationPolicy:"), NSApplicationActivationPolicyRegular);

	_RGFW->customViewClasses[0] = objc_allocateClassPair(objc_getClass("NSView"), "RGFWCustomView", 0);
	_RGFW->customViewClasses[1] = objc_allocateClassPair(objc_getClass("NSOpenGLView"), "RGFWOpenGLCustomView", 0);
	for (size_t i = 0; i < 2; i++) {
		class_addIvar((Class)_RGFW->customViewClasses[i], "RGFW_window", sizeof(RGFW_window*), sizeof(RGFW_window*), "L");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("drawRect:"), (IMP)RGFW__osxDrawRect, "v@:{CGRect=ffff}");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("viewDidChangeBackingProperties"), (IMP)RGFW__osxViewDidChangeBackingProperties, "v@:");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("mouseDown:"), (IMP)RGFW__osxMouseDown, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("rightMouseDown:"), (IMP)RGFW__osxMouseDown, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("otherMouseDown:"), (IMP)RGFW__osxMouseDown, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("mouseUp:"), (IMP)RGFW__osxMouseUp, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("rightMouseUp:"), (IMP)RGFW__osxMouseUp, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("otherMouseUp:"), (IMP)RGFW__osxMouseUp, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("scrollWheel:"), (IMP)RGFW__osxScrollWheel, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("mouseDragged:"), (IMP)RGFW__osxMouseMoved, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("rightMouseDragged:"), (IMP)RGFW__osxMouseMoved, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("otherMouseDragged:"), (IMP)RGFW__osxMouseMoved, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("keyDown:"), (IMP)RGFW__osxKeyDown, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("keyUp:"), (IMP)RGFW__osxKeyUp, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("mouseMoved:"), (IMP)RGFW__osxMouseMoved, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("mouseEntered:"), (IMP)RGFW__osxMouseEntered, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("mouseExited:"), (IMP)RGFW__osxMouseExited, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("flagsChanged:"), (IMP)RGFW__osxFlagsChanged, "v@:@");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_getUid("acceptsFirstResponder"), (IMP)RGFW__osxAcceptsFirstResponder, "B@:");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("initWithRGFWWindow:"), (IMP)RGFW__osxCustomInitWithRGFWWindow, "@@:{CGRect={CGPoint=dd}{CGSize=dd}}");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("wantsUpdateLayer"), (IMP)RGFW__osxWantsUpdateLayer, "B@:");
		class_addMethod((Class)_RGFW->customViewClasses[i], sel_registerName("updateLayer"), (IMP)RGFW__osxUpdateLayer, "v@:");
		objc_registerClassPair((Class)_RGFW->customViewClasses[i]);
	}
	_RGFW->customWindowDelegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "RGFWWindowDelegate", 0);
	class_addIvar((Class)_RGFW->customWindowDelegateClass, "RGFW_window", sizeof(RGFW_window*), sizeof(RGFW_window*), "L");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("windowDidResize:"), (IMP)RGFW__osxDidWindowResize, "v@:@");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("windowDidMove:"), (IMP) RGFW__osxWindowMove, "");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("windowDidMiniaturize:"), (IMP) RGFW__osxWindowMiniaturize, "");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("windowDidDeminiaturize:"), (IMP) RGFW__osxWindowDeminiaturize, "");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("windowDidBecomeKey:"), (IMP) RGFW__osxWindowBecameKey, "");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("windowDidResignKey:"), (IMP) RGFW__osxWindowResignKey, "");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("draggingEntered:"), (IMP)RGFW__osxDraggingEntered, "l@:@");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("draggingUpdated:"), (IMP)RGFW__osxDraggingUpdated, "l@:@");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("draggingExited:"), (IMP)RGFW__osxDraggingEnded, "v@:@");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("draggingEnded:"), (IMP)RGFW__osxDraggingEnded, "v@:@");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("prepareForDragOperation:"), (IMP)RGFW__osxPrepareForDragOperation, "B@:@");
	class_addMethod((Class)_RGFW->customWindowDelegateClass, sel_registerName("performDragOperation:"), (IMP)RGFW__osxPerformDragOperation, "B@:@");
	objc_registerClassPair((Class)_RGFW->customWindowDelegateClass);
	return 0;
}

void RGFW_osx_initView(RGFW_window* win) {
	NSRect contentRect;
	contentRect.origin.x = 0;
	contentRect.origin.y = 0;
	contentRect.size.width = (double)win->w;
	contentRect.size.height = (double)win->h;
	((void(*)(id, SEL, CGRect))objc_msgSend)((id)win->src.view, sel_registerName("setFrame:"), contentRect);


	if (RGFW_COCOA_FRAME_NAME)
		objc_msgSend_ptr(win->src.view, sel_registerName("setFrameAutosaveName:"), RGFW_COCOA_FRAME_NAME);

	object_setInstanceVariable((id)win->src.view, "RGFW_window", win);
	objc_msgSend_void_id((id)win->src.window, sel_registerName("setContentView:"), win->src.view);
	objc_msgSend_void_bool(win->src.view, sel_registerName("setWantsLayer:"), true);
	objc_msgSend_int((id)win->src.view, sel_registerName("setLayerContentsPlacement:"),  4);

	id trackingArea = objc_msgSend_id(objc_getClass("NSTrackingArea"), sel_registerName("alloc"));
	trackingArea = ((id (*)(id, SEL, NSRect, NSUInteger, id, id))objc_msgSend)(
		trackingArea,
		sel_registerName("initWithRect:options:owner:userInfo:"),
		contentRect,
		NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways | NSTrackingInVisibleRect,
		(id)win->src.view,
		nil
	);

	((void (*)(id, SEL, id))objc_msgSend)((id)win->src.view, sel_registerName("addTrackingArea:"), trackingArea);
	((void (*)(id, SEL))objc_msgSend)(trackingArea, sel_registerName("release"));
}

RGFW_window* RGFW_createWindowPlatform(const char* name, RGFW_windowFlags flags, RGFW_window* win) {
	/* RR Create an autorelease pool */
	id pool = objc_msgSend_class(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc"));
	pool = objc_msgSend_id(pool, sel_registerName("init"));

	RGFW_window_setMouseDefault(win);

	NSRect windowRect;
	windowRect.origin.x = (double)win->x;
	windowRect.origin.y = (double)win->y;
	windowRect.size.width = (double)win->w;
	windowRect.size.height = (double)win->h;
	NSBackingStoreType macArgs = (NSBackingStoreType)(NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSBackingStoreBuffered | NSWindowStyleMaskTitled);

	if (!(flags & RGFW_windowNoResize))
		macArgs = (NSBackingStoreType)(macArgs | (NSBackingStoreType)NSWindowStyleMaskResizable);
	if (!(flags & RGFW_windowNoBorder))
		macArgs = (NSBackingStoreType)(macArgs | (NSBackingStoreType)NSWindowStyleMaskTitled);
	{
		void* nsclass = objc_getClass("NSWindow");
		SEL func = sel_registerName("initWithContentRect:styleMask:backing:defer:");

		win->src.window = ((id(*)(id, SEL, NSRect, NSWindowStyleMask, NSBackingStoreType, bool))objc_msgSend)
			(NSAlloc(nsclass), func, windowRect, (NSWindowStyleMask)macArgs, macArgs, false);
	}

	id str = NSString_stringWithUTF8String(name);
	objc_msgSend_void_id((id)win->src.window, sel_registerName("setTitle:"), str);

	id delegate = objc_msgSend_id(NSAlloc((Class)_RGFW->customWindowDelegateClass), sel_registerName("init"));
	object_setInstanceVariable(delegate, "RGFW_window", win);

	objc_msgSend_void_id((id)win->src.window, sel_registerName("setDelegate:"), delegate);

	if (flags & RGFW_windowAllowDND) {
		win->internal.flags |= RGFW_windowAllowDND;

		NSPasteboardType types[] = {NSPasteboardTypeURL, NSPasteboardTypeFileURL, NSPasteboardTypeString};
		NSregisterForDraggedTypes((id)win->src.window, types, 3);
	}

	objc_msgSend_void_bool((id)win->src.window, sel_registerName("setAcceptsMouseMovedEvents:"), true);

	if (flags & RGFW_windowTransparent) {
		objc_msgSend_void_bool(win->src.window, sel_registerName("setOpaque:"), false);

		objc_msgSend_void_id((id)win->src.window, sel_registerName("setBackgroundColor:"),
		NSColor_colorWithSRGB(0, 0, 0, 0));
	}

	/* Show the window */
	objc_msgSend_void_bool((id)_RGFW->NSApp, sel_registerName("activateIgnoringOtherApps:"), true);

	if (_RGFW->root == NULL) {
		objc_msgSend_void(win->src.window, sel_registerName("makeMainWindow"));
	}

	objc_msgSend_void(win->src.window, sel_registerName("makeKeyWindow"));

	objc_msgSend_void((id)_RGFW->NSApp, sel_registerName("finishLaunching"));
	NSRetain(win->src.window);
	NSRetain(_RGFW->NSApp);

	win->src.view = ((id(*)(id, SEL, RGFW_window*))objc_msgSend) (NSAlloc((Class)_RGFW->customViewClasses[0]), sel_registerName("initWithRGFWWindow:"), win);
	return win;
}

void RGFW_window_setBorder(RGFW_window* win, RGFW_bool border) {
	NSRect frame = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)((id)win->src.window, sel_registerName("frame"));
	NSRect content = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)((id)win->src.view, sel_registerName("frame"));
	double offset = 0;

	RGFW_setBit(&win->internal.flags, RGFW_windowNoBorder, !border);
	NSBackingStoreType storeType = (NSBackingStoreType)(NSWindowStyleMaskBorderless | NSWindowStyleMaskFullSizeContentView);
	if (border)
		storeType = (NSBackingStoreType)(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable);
	if (!(win->internal.flags & RGFW_windowNoResize)) {
		storeType = (NSBackingStoreType)(storeType | (NSBackingStoreType)NSWindowStyleMaskResizable);
	}

	((void (*)(id, SEL, NSBackingStoreType))objc_msgSend)((id)win->src.window, sel_registerName("setStyleMask:"), storeType);

	if (!border) {
		id miniaturizeButton = objc_msgSend_int((id)win->src.window, sel_registerName("standardWindowButton:"),  NSWindowMiniaturizeButton);
		id titleBarView = objc_msgSend_id(miniaturizeButton, sel_registerName("superview"));
		objc_msgSend_void_bool(titleBarView, sel_registerName("setHidden:"), true);

		offset = (double)(frame.size.height - content.size.height);
	}

	RGFW_window_resize(win, win->w, win->h + (i32)offset);
	win->h -= (i32)offset;
}

RGFW_bool RGFW_getGlobalMouse(i32* x, i32* y) {
	RGFW_ASSERT(_RGFW->root != NULL);

	CGEventRef e = CGEventCreate(NULL);
	CGPoint point = CGEventGetLocation(e);
	CFRelease(e);

	if (x) *x = (i32)point.x;
	if (y) *y = (i32)point.y;
	return RGFW_TRUE;
}

void RGFW_stopCheckEvents(void) {
	id eventPool = objc_msgSend_class(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc"));
	eventPool = objc_msgSend_id(eventPool, sel_registerName("init"));

	id e = (id) ((id(*)(Class, SEL, NSEventType, NSPoint, NSEventModifierFlags, void*, NSInteger, void**, short, NSInteger, NSInteger))objc_msgSend)
		(objc_getClass("NSEvent"), sel_registerName("otherEventWithType:location:modifierFlags:timestamp:windowNumber:context:subtype:data1:data2:"),
			NSEventTypeApplicationDefined, (NSPoint){0, 0}, (NSEventModifierFlags)0, NULL, (NSInteger)0, NULL, 0, 0, 0);

	((void (*)(id, SEL, id, bool))objc_msgSend)
		((id)_RGFW->NSApp, sel_registerName("postEvent:atStart:"), e, 1);

	objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
}

void RGFW_waitForEvent(i32 waitMS) {
	id eventPool = objc_msgSend_class(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc"));
	eventPool = objc_msgSend_id(eventPool, sel_registerName("init"));

	void* date = (void*) ((id(*)(Class, SEL, double))objc_msgSend)
				(objc_getClass("NSDate"), sel_registerName("dateWithTimeIntervalSinceNow:"), waitMS);

	SEL eventFunc = sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:");
	id e = (id) ((id(*)(id, SEL, NSEventMask, void*, id, bool))objc_msgSend)
		((id)_RGFW->NSApp, eventFunc,
			ULONG_MAX, date, NSString_stringWithUTF8String("kCFRunLoopDefaultMode"), true);

	if (e) {
		((void (*)(id, SEL, id, bool))objc_msgSend)
			((id)_RGFW->NSApp, sel_registerName("postEvent:atStart:"), e, 1);
	}

	objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
}

u8 RGFW_rgfwToKeyChar(u32 rgfw_keycode) {
    return (u8)rgfw_keycode; /* TODO */
}

void RGFW_pollEvents(void) {
	/*
	 * TODO look to see if all these events can be replaced with callbacks
	 * callbacks seem to give better info on mac's api
	*/

	RGFW_resetPrevState();

	id eventPool = objc_msgSend_class(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc"));
	eventPool = objc_msgSend_id(eventPool, sel_registerName("init"));
	SEL eventFunc = sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:");

	while (1) {
		void* date = NULL;
		id e = (id) ((id(*)(id, SEL, NSEventMask, void*, id, bool))objc_msgSend)
			((id)_RGFW->NSApp, eventFunc, ULONG_MAX, date, NSString_stringWithUTF8String("kCFRunLoopDefaultMode"), true);

		if (e == NULL) {
			objc_msgSend_void_id((id)_RGFW->NSApp, sel_registerName("sendEvent:"), e);
			((void(*)(id, SEL))objc_msgSend)((id)_RGFW->NSApp, sel_registerName("updateWindows"));
			break;
		}

		RGFW_event event;
		RGFW_MEMSET(&event, 0, sizeof(event));
		objc_msgSend_void_id((id)_RGFW->NSApp, sel_registerName("sendEvent:"), e);
		((void(*)(id, SEL))objc_msgSend)((id)_RGFW->NSApp, sel_registerName("updateWindows"));
	}

	objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
}


void RGFW_window_move(RGFW_window* win, i32 x, i32 y) {
	RGFW_ASSERT(win != NULL);

	win->x = x;
	win->y = y;
	((void(*)(id,SEL,NSPoint))objc_msgSend)((id)win->src.window, sel_registerName("setFrameOrigin:"), (NSPoint){(double)x, (double)y});
}

void RGFW_window_resize(RGFW_window* win, i32 w, i32 h) {
	RGFW_ASSERT(win != NULL);

	NSRect frame = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)((id)win->src.window, sel_registerName("frame"));
	NSRect content = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)((id)win->src.view, sel_registerName("frame"));
	float offset = (float)(frame.size.height - content.size.height);

	win->w = w;
	win->h = h;


	((void(*)(id, SEL, CGRect))objc_msgSend)((id)win->src.view, sel_registerName("setFrame:"),  (NSRect){{0, 0}, {(double)win->w, (double)win->h}});
	((void(*)(id, SEL, NSRect, bool, bool))objc_msgSend)
		((id)win->src.window, sel_registerName("setFrame:display:animate:"), (NSRect){{(double)win->x, (double)win->y}, {(double)win->w, (double)win->h + (double)offset}}, true, true);
}

void RGFW_window_focus(RGFW_window* win) {
	RGFW_ASSERT(win);
	objc_msgSend_void_bool((id)_RGFW->NSApp, sel_registerName("activateIgnoringOtherApps:"), true);
	((void (*)(id, SEL))objc_msgSend)((id)win->src.window, sel_registerName("makeKeyWindow"));
}

void RGFW_window_raise(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	((id(*)(id, SEL, SEL))objc_msgSend)((id)win->src.window, sel_registerName("orderFront:"), (SEL)NULL);
    	objc_msgSend_void_id(win->src.window, sel_registerName("setLevel:"), kCGNormalWindowLevelKey);
}

void RGFW_window_setFullscreen(RGFW_window* win, RGFW_bool fullscreen) {
	RGFW_ASSERT(win != NULL);
	if (fullscreen && (win->internal.flags & RGFW_windowFullscreen)) return;
	if (!fullscreen && !(win->internal.flags & RGFW_windowFullscreen)) return;

	if (fullscreen) {
		if (!(win->internal.flags & RGFW_windowFullscreen)) {
			return;
		}

		win->internal.oldX = win->x;
		win->internal.oldY = win->y;
		win->internal.oldW = win->w;
		win->internal.oldH = win->h;
		RGFW_monitor mon = RGFW_window_getMonitor(win);
		win->x = mon.x;
		win->y = mon.y;
		win->w = mon.mode.w;
		win->h = mon.mode.h;
		win->internal.flags |= RGFW_windowFullscreen;
		RGFW_window_resize(win, mon.mode.w, mon.mode.h);
		RGFW_window_move(win, mon.x, mon.y);
	}
	objc_msgSend_void_SEL(win->src.window, sel_registerName("toggleFullScreen:"), NULL);

	if (!fullscreen) {
		win->x  = win->internal.oldX;
		win->y  = win->internal.oldY;
		win->w  = win->internal.oldW;
		win->h = win->internal.oldH;
		win->internal.flags &= ~(u32)RGFW_windowFullscreen;

		RGFW_window_resize(win, win->w, win->h);
		RGFW_window_move(win, win->x, win->y);
	}
}

void RGFW_window_maximize(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	if (RGFW_window_isMaximized(win)) return;

	win->internal.flags |= RGFW_windowMaximize;
	objc_msgSend_void_SEL(win->src.window, sel_registerName("zoom:"), NULL);
}

void RGFW_window_minimize(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	objc_msgSend_void_SEL(win->src.window, sel_registerName("performMiniaturize:"), NULL);
}

void RGFW_window_setFloating(RGFW_window* win, RGFW_bool floating) {
    RGFW_ASSERT(win != NULL);
    if (floating) objc_msgSend_void_id(win->src.window, sel_registerName("setLevel:"), kCGFloatingWindowLevelKey);
    else 		  objc_msgSend_void_id(win->src.window, sel_registerName("setLevel:"), kCGNormalWindowLevelKey);
}

void RGFW_window_setOpacity(RGFW_window* win, u8 opacity) {
	objc_msgSend_int(win->src.window, sel_registerName("setAlphaValue:"), opacity);
	objc_msgSend_void_bool(win->src.window, sel_registerName("setOpaque:"), (opacity < (u8)255));

	if (opacity)
		objc_msgSend_void_id((id)win->src.window, sel_registerName("setBackgroundColor:"), NSColor_colorWithSRGB(0, 0, 0, opacity));

}

void RGFW_window_restore(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	if (RGFW_window_isMaximized(win))
		objc_msgSend_void_SEL(win->src.window, sel_registerName("zoom:"), NULL);

	objc_msgSend_void_SEL(win->src.window, sel_registerName("deminiaturize:"), NULL);
	RGFW_window_show(win);
}

RGFW_bool RGFW_window_isFloating(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	int level = ((int (*)(id, SEL))objc_msgSend) ((id)(win->src.window), (SEL)sel_registerName("level"));
	return level > kCGNormalWindowLevelKey;
}

void RGFW_window_setName(RGFW_window* win, const char* name) {
	RGFW_ASSERT(win != NULL);

	id str = NSString_stringWithUTF8String(name);
	objc_msgSend_void_id((id)win->src.window, sel_registerName("setTitle:"), str);
}

#ifndef RGFW_NO_PASSTHROUGH
void RGFW_window_setMousePassthrough(RGFW_window* win, RGFW_bool passthrough) {
	objc_msgSend_void_bool(win->src.window, sel_registerName("setIgnoresMouseEvents:"), passthrough);
}
#endif

void RGFW_window_setAspectRatio(RGFW_window* win, i32 w, i32 h) {
	if (w == 0 && h == 0) {  w = 1; h = 1; };

	((void (*)(id, SEL, NSSize))objc_msgSend)
		((id)win->src.window, sel_registerName("setContentAspectRatio:"), (NSSize){(CGFloat)w, (CGFloat)h});
}

void RGFW_window_setMinSize(RGFW_window* win, i32 w, i32 h) {
	((void (*)(id, SEL, NSSize))objc_msgSend) ((id)win->src.window, sel_registerName("setMinSize:"), (NSSize){(CGFloat)w, (CGFloat)h});
}

void RGFW_window_setMaxSize(RGFW_window* win, i32 w, i32 h) {
	if (w == 0 && h == 0) {
		RGFW_monitor mon = RGFW_window_getMonitor(win);
		w = mon.mode.w;
		h = mon.mode.h;
	}

	((void (*)(id, SEL, NSSize))objc_msgSend)
		((id)win->src.window, sel_registerName("setMaxSize:"), (NSSize){(CGFloat)w, (CGFloat)h});
}

RGFW_bool RGFW_window_setIconEx(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format, RGFW_icon type) {
	RGFW_ASSERT(win != NULL);
	RGFW_UNUSED(type);

	if (data == NULL) {
		objc_msgSend_void_id((id)_RGFW->NSApp, sel_registerName("setApplicationIconImage:"), NULL);
		return RGFW_TRUE;
	}

	id representation = NSBitmapImageRep_initWithBitmapData(NULL, w, h, 8, (NSInteger)4, true, false, "NSCalibratedRGBColorSpace", 1 << 1, w * 4, 32);
	RGFW_copyImageData(NSBitmapImageRep_bitmapData(representation), w, h, RGFW_formatRGBA8, data, format);

	id dock_image = ((id(*)(id, SEL, NSSize))objc_msgSend) (NSAlloc((id)objc_getClass("NSImage")), sel_registerName("initWithSize:"), ((NSSize){(CGFloat)w, (CGFloat)h}));

	objc_msgSend_void_id(dock_image, sel_registerName("addRepresentation:"), representation);

	objc_msgSend_void_id((id)_RGFW->NSApp, sel_registerName("setApplicationIconImage:"), dock_image);

	NSRelease(dock_image);
	NSRelease(representation);

	return RGFW_TRUE;
}

id NSCursor_arrowStr(const char* str);
id NSCursor_arrowStr(const char* str) {
	void* nclass = objc_getClass("NSCursor");
	SEL func = sel_registerName(str);
	return (id) objc_msgSend_id(nclass, func);
}

RGFW_mouse* RGFW_loadMouse(u8* data, i32 w, i32 h, RGFW_format format) {
	if (data == NULL) {
		objc_msgSend_void(NSCursor_arrowStr("arrowCursor"), sel_registerName("set"));
		return NULL;
	}

	id representation = (id)NSBitmapImageRep_initWithBitmapData(NULL, w, h, 8, (NSInteger)4, true, false, "NSCalibratedRGBColorSpace", 1 << 1, w * 4, 32);
	RGFW_copyImageData(NSBitmapImageRep_bitmapData(representation), w, h, RGFW_formatRGBA8, data, format);

	id cursor_image = ((id(*)(id, SEL, NSSize))objc_msgSend) (NSAlloc((id)objc_getClass("NSImage")), sel_registerName("initWithSize:"), ((NSSize){(CGFloat)w, (CGFloat)h}));

	objc_msgSend_void_id(cursor_image, sel_registerName("addRepresentation:"), representation);

	id cursor = (id) ((id(*)(id, SEL, id, NSPoint))objc_msgSend)
		(NSAlloc(objc_getClass("NSCursor")),  sel_registerName("initWithImage:hotSpot:"), cursor_image, (NSPoint){0.0, 0.0});

	NSRelease(cursor_image);
	NSRelease(representation);

	return (void*)cursor;
}

void RGFW_window_setMouse(RGFW_window* win, RGFW_mouse* mouse) {
	RGFW_ASSERT(win != NULL); RGFW_ASSERT(mouse);
	CGDisplayShowCursor(kCGDirectMainDisplay);
	objc_msgSend_void((id)mouse, sel_registerName("set"));
	win->src.mouse = mouse;
}

void RGFW_freeMouse(RGFW_mouse* mouse) {
	RGFW_ASSERT(mouse);
	NSRelease((id)mouse);
}

RGFW_bool RGFW_window_setMouseDefault(RGFW_window* win) {
	return RGFW_window_setMouseStandard(win, RGFW_mouseArrow);
}

void RGFW_window_showMouse(RGFW_window* win, RGFW_bool show) {
	RGFW_window_showMouseFlags(win, show);
	if (show)   CGDisplayShowCursor(kCGDirectMainDisplay);
	else        CGDisplayHideCursor(kCGDirectMainDisplay);
}

RGFW_bool RGFW_window_setMouseStandard(RGFW_window* win, u8 stdMouses) {
	static const char* mouseIconSrc[16] = {"arrowCursor", "arrowCursor", "IBeamCursor", "crosshairCursor", "pointingHandCursor", "resizeLeftRightCursor", "resizeUpDownCursor", "_windowResizeNorthWestSouthEastCursor", "_windowResizeNorthEastSouthWestCursor", "closedHandCursor", "operationNotAllowedCursor"};
	if (stdMouses > ((sizeof(mouseIconSrc)) / (sizeof(char*))))
		return RGFW_FALSE;

	const char* mouseStr = mouseIconSrc[stdMouses];
	id mouse = NSCursor_arrowStr(mouseStr);

	if (mouse == NULL)
		return RGFW_FALSE;

	RGFW_UNUSED(win);
	CGDisplayShowCursor(kCGDirectMainDisplay);
	objc_msgSend_void(mouse, sel_registerName("set"));
	win->src.mouse = mouse;

	return RGFW_TRUE;
}

void RGFW_releaseCursor(RGFW_window* win) {
	RGFW_UNUSED(win);
	CGAssociateMouseAndMouseCursorPosition(1);
}

void RGFW_captureCursor(RGFW_window* win) {
	RGFW_UNUSED(win);

	CGWarpMouseCursorPosition((CGPoint){(CGFloat)(win->x + (win->w / 2)), (CGFloat)(win->y + (win->h / 2))});
	CGAssociateMouseAndMouseCursorPosition(0);
}

void RGFW_window_moveMouse(RGFW_window* win, i32 x, i32 y) {
	RGFW_UNUSED(win);

	win->internal.lastMouseX = x - win->x;
	win->internal.lastMouseY = y - win->y;
	CGWarpMouseCursorPosition((CGPoint){(CGFloat)x, (CGFloat)y});
}


void RGFW_window_hide(RGFW_window* win) {
	objc_msgSend_void_bool(win->src.window, sel_registerName("setIsVisible:"), false);
}

void RGFW_window_show(RGFW_window* win) {
	if (win->internal.flags & RGFW_windowFocusOnShow)
		((id(*)(id, SEL, SEL))objc_msgSend)((id)win->src.window, sel_registerName("makeKeyAndOrderFront:"), NULL);

	((id(*)(id, SEL, SEL))objc_msgSend)((id)win->src.window, sel_registerName("orderFront:"), NULL);
	objc_msgSend_void_bool(win->src.window, sel_registerName("setIsVisible:"), true);
}

RGFW_bool RGFW_window_isHidden(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	bool visible = objc_msgSend_bool(win->src.window, sel_registerName("isVisible"));
	return visible == NO && !RGFW_window_isMinimized(win);
}

RGFW_bool RGFW_window_isMinimized(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	return objc_msgSend_bool(win->src.window, sel_registerName("isMiniaturized")) == YES;
}

RGFW_bool RGFW_window_isMaximized(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_bool b = (RGFW_bool)objc_msgSend_bool(win->src.window, sel_registerName("isZoomed"));
	return b;
}

id RGFW_getNSScreenForDisplayID(CGDirectDisplayID display);
id RGFW_getNSScreenForDisplayID(CGDirectDisplayID display) {
	Class NSScreenClass = objc_getClass("NSScreen");

	id screens = objc_msgSend_id(NSScreenClass, sel_registerName("screens"));

	NSUInteger count = (NSUInteger)objc_msgSend_uint(screens, sel_registerName("count"));
    NSUInteger i;
	for (i = 0; i < count; i++) {
		id screen = ((id (*)(id, SEL, int))objc_msgSend) (screens, sel_registerName("objectAtIndex:"), (int)i);
		id description = objc_msgSend_id(screen, sel_registerName("deviceDescription"));
		id screenNumberKey = NSString_stringWithUTF8String("NSScreenNumber");
		id screenNumber = objc_msgSend_id_id(description, sel_registerName("objectForKey:"), screenNumberKey);

		if ((CGDirectDisplayID)objc_msgSend_uint(screenNumber, sel_registerName("unsignedIntValue")) == display) {
			return screen;
		}
	}

	return NULL;
}

u32 RGFW_osx_getRefreshRate(CGDirectDisplayID display, CGDisplayModeRef mode);
u32 RGFW_osx_getRefreshRate(CGDirectDisplayID display, CGDisplayModeRef mode) {
	if (mode) {
		u32 refreshRate = (u32)CGDisplayModeGetRefreshRate(mode);
		if (refreshRate != 0)  return refreshRate;
	}

#ifndef RGFW_NO_IOKIT
    u32 res = RGFW_osx_getFallbackRefreshRate(display);
    if (res != 0) return res;
#else
    RGFW_UNUSED(display);
#endif
    return 60;
}

RGFW_monitor RGFW_NSCreateMonitor(CGDirectDisplayID display, id screen);
RGFW_monitor RGFW_NSCreateMonitor(CGDirectDisplayID display, id screen) {
	RGFW_monitor monitor;

	const char name[] = "MacOS\0";
	RGFW_MEMCPY(monitor.name, name, 6);

	CGRect bounds = CGDisplayBounds(display);
	monitor.x = (i32)bounds.origin.x;
	monitor.y = (i32)bounds.origin.y;
	monitor.mode.w = (i32) bounds.size.width;
	monitor.mode.h  = (i32) bounds.size.height;

	monitor.mode.red = 8; monitor.mode.green = 8; monitor.mode.blue = 8;

	CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display);
	monitor.mode.refreshRate = RGFW_osx_getRefreshRate(display, mode);
	CFRelease(mode);

	CGSize screenSizeMM = CGDisplayScreenSize(display);
	monitor.physW = (float)screenSizeMM.width / 25.4f;
	monitor.physH = (float)screenSizeMM.height / 25.4f;

	float ppi_width = (monitor.mode.w/monitor.physW);
	float ppi_height = (monitor.mode.h/monitor.physH);

	monitor.pixelRatio = (float)((CGFloat (*)(id, SEL))abi_objc_msgSend_fpret) (screen, sel_registerName("backingScaleFactor"));
	float dpi = 96.0f * monitor.pixelRatio;

	monitor.scaleX = ((i32)(((float) (ppi_width) / dpi) * 10.0f)) / 10.0f;
	monitor.scaleY = ((i32)(((float) (ppi_height) / dpi) * 10.0f)) / 10.0f;

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoMonitor,  "monitor found");
	return monitor;
}


RGFW_monitor* RGFW_getMonitors(size_t* len) {
	static CGDirectDisplayID displays[7];
	u32 count;

	if (CGGetActiveDisplayList(6, displays, &count) != kCGErrorSuccess)
		return NULL;

	if (count > 6) count = 6;

	static RGFW_monitor monitors[7];

    u32 i;
	for (i = 0; i < count; i++)
		monitors[i] = RGFW_NSCreateMonitor(displays[i], RGFW_getNSScreenForDisplayID(displays[i]));

	if (len != NULL) *len = count;
	return monitors;
}

RGFW_bool RGFW_monitor_requestMode(RGFW_monitor mon, RGFW_monitorMode mode, RGFW_modeRequest request) {
    CGPoint point = { (CGFloat)mon.x, (CGFloat)mon.y };

    CGDirectDisplayID display;
    u32 displayCount = 0;
    CGError err = CGGetDisplaysWithPoint(point, 1, &display, &displayCount);
    if (err != kCGErrorSuccess || displayCount != 1)
		return RGFW_FALSE;

    CFArrayRef allModes = CGDisplayCopyAllDisplayModes(display, NULL);

    if (allModes == NULL)
        return RGFW_FALSE;

    CFIndex i;
    for (i = 0; i < CFArrayGetCount(allModes); i++) {
        CGDisplayModeRef cmode = (CGDisplayModeRef)CFArrayGetValueAtIndex(allModes, i);

		RGFW_monitorMode foundMode;
		foundMode.w = (i32)CGDisplayModeGetWidth(cmode);
		foundMode.h = (i32)CGDisplayModeGetHeight(cmode);
		foundMode.refreshRate =  RGFW_osx_getRefreshRate(display, cmode);
		foundMode.red = 8; foundMode.green = 8; foundMode.blue = 8;

		if (RGFW_monitorModeCompare(mode, foundMode, request)) {
				if (CGDisplaySetDisplayMode(display, cmode, NULL) == kCGErrorSuccess) {
					CFRelease(allModes);
					return RGFW_TRUE;
				}
				break;
        }
    }

    CFRelease(allModes);

	return RGFW_FALSE;
}

RGFW_monitor RGFW_getPrimaryMonitor(void) {
	CGDirectDisplayID primary = CGMainDisplayID();
	return RGFW_NSCreateMonitor(primary, RGFW_getNSScreenForDisplayID(primary));
}

RGFW_monitor RGFW_window_getMonitor(RGFW_window* win) {
	id screen = objc_msgSend_id(win->src.window, sel_registerName("screen"));
	id description = objc_msgSend_id(screen, sel_registerName("deviceDescription"));
	id screenNumberKey = NSString_stringWithUTF8String("NSScreenNumber");
	id screenNumber = objc_msgSend_id_id(description, sel_registerName("objectForKey:"), screenNumberKey);

	CGDirectDisplayID display = (CGDirectDisplayID)objc_msgSend_uint(screenNumber, sel_registerName("unsignedIntValue"));

	return RGFW_NSCreateMonitor(display, screen);
}

RGFW_ssize_t RGFW_readClipboardPtr(char* str, size_t strCapacity) {
	size_t clip_len;
	char* clip = (char*)NSPasteboard_stringForType(NSPasteboard_generalPasteboard(), NSPasteboardTypeString, &clip_len);
	if (clip == NULL) return -1;

	if (str != NULL) {
		if (strCapacity < clip_len)
			return 0;

		RGFW_MEMCPY(str, clip, clip_len);

		str[clip_len] = '\0';
	}

	return (RGFW_ssize_t)clip_len;
}

void RGFW_writeClipboard(const char* text, u32 textLen) {
	RGFW_UNUSED(textLen);

	NSPasteboardType array[] = { NSPasteboardTypeString, NULL };
	NSPasteBoard_declareTypes(NSPasteboard_generalPasteboard(), array, 1, NULL);

	SEL func = sel_registerName("setString:forType:");
	((bool (*)(id, SEL, id, id))objc_msgSend)
		(NSPasteboard_generalPasteboard(), func, NSString_stringWithUTF8String(text), NSString_stringWithUTF8String((const char*)NSPasteboardTypeString));
}

#ifdef RGFW_OPENGL
void NSOpenGLContext_setValues(id context, const int* vals, NSOpenGLContextParameter param);
void NSOpenGLContext_setValues(id context, const int* vals, NSOpenGLContextParameter param) {
	((void (*)(id, SEL, const int*, NSOpenGLContextParameter))objc_msgSend)
		(context, sel_registerName("setValues:forParameter:"), vals, param);
}


/* MacOS OpenGL API spares us yet again (there are no extensions) */
RGFW_bool RGFW_extensionSupportedPlatform_OpenGL(const char * extension, size_t len) { RGFW_UNUSED(extension); RGFW_UNUSED(len); return RGFW_FALSE; }

RGFW_proc RGFW_getProcAddress_OpenGL(const char* procname) {
    static CFBundleRef RGFWnsglFramework = NULL;
    if (RGFWnsglFramework == NULL)
		RGFWnsglFramework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));

	CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault, procname, kCFStringEncodingASCII);

	RGFW_proc symbol = (RGFW_proc)CFBundleGetFunctionPointerForName(RGFWnsglFramework, symbolName);

	CFRelease(symbolName);

	return symbol;
}

RGFW_bool RGFW_window_createContextPtr_OpenGL(RGFW_window* win, RGFW_glContext* ctx, RGFW_glHints* hints) {
	win->src.ctx.native = ctx;
	win->src.gfxType = RGFW_gfxNativeOpenGL;

	i32 attribs[40];
	size_t render_type_index = 0;
	{
		RGFW_attribStack stack;
		RGFW_attribStack_init(&stack, attribs, 40);

		i32 colorBits = (i32)(hints->red + hints->green +  hints->blue + hints->alpha) / 4;
		RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFAColorSize, colorBits);

		RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFAAlphaSize, hints->alpha);
		RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFADepthSize, hints->depth);
		RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFAStencilSize, hints->stencil);
		RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFAAuxBuffers, hints->auxBuffers);
		RGFW_attribStack_pushAttrib(&stack, NSOpenGLPFAClosestPolicy);
		if (hints->samples) {
			RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFASampleBuffers, 1);
			RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFASamples, hints->samples);
		} else RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFASampleBuffers, 0);

		if (hints->doubleBuffer)
			RGFW_attribStack_pushAttrib(&stack, NSOpenGLPFADoubleBuffer);

		#ifdef RGFW_COCOA_GRAPHICS_SWITCHING
		RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFAAllowOfflineRenderers, kCGLPFASupportsAutomaticGraphicsSwitching)
		#endif
		#if MAC_OS_X_VERSION_MAX_ALLOWED < 101200
		if (hints->stereo]) RGFW_attribStack_pushAttrib(&stack, NSOpenGLPFAStereo);
		#endif

		/* macOS has the surface attribs and the OpenGL attribs connected for some reason maybe this is to give macOS more control to limit openGL/the OpenGL version? */
		RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFAOpenGLProfile,
						(hints->major >= 4) ? NSOpenGLProfileVersion4_1Core  : (hints->major >= 3) ?
															NSOpenGLProfileVersion3_2Core : NSOpenGLProfileVersionLegacy);

		if (hints->major <= 2) {
			i32 accumSize = (i32)(hints->accumRed + hints->accumGreen +  hints->accumBlue + hints->accumAlpha) / 4;
			RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFAAccumSize, accumSize);
		}

		if (hints->renderer == RGFW_glSoftware) {
			RGFW_attribStack_pushAttribs(&stack, NSOpenGLPFARendererID, kCGLRendererGenericFloatID);
		} else {
			RGFW_attribStack_pushAttrib(&stack, NSOpenGLPFAAccelerated);
		}
		render_type_index = stack.count - 1;

		RGFW_attribStack_pushAttribs(&stack, 0, 0);
	}

	void* format = (void*) ((id(*)(id, SEL, const u32*))objc_msgSend) (NSAlloc((id)objc_getClass("NSOpenGLPixelFormat")), sel_registerName("initWithAttributes:"), (u32*)attribs);
	if (format == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext, "Failed to load pixel format for OpenGL");

		assert(render_type_index + 3 < (sizeof(attribs) / sizeof(attribs[0])));
		attribs[render_type_index] = NSOpenGLPFARendererID;
		attribs[render_type_index + 1] = kCGLRendererGenericFloatID;
		attribs[render_type_index + 3] = 0;

		format = (void*) ((id(*)(id, SEL, const u32*))objc_msgSend) (NSAlloc((id)objc_getClass("NSOpenGLPixelFormat")), sel_registerName("initWithAttributes:"), (u32*)attribs);
		if (format == NULL)
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenGLContext, "and loading software rendering OpenGL failed");
		else
			RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningOpenGL, "Switching to software rendering");
	}

	/* the pixel format can be passed directly to OpenGL context creation to create a context
		this is because the format also includes information about the OpenGL version (which may be a bad thing) */

	if (win->src.view)
		NSRelease(win->src.view);
	win->src.view = (id) ((id(*)(id, SEL, NSRect, u32*))objc_msgSend) (NSAlloc(_RGFW->customViewClasses[1]),
							sel_registerName("initWithFrame:pixelFormat:"), (NSRect){{0, 0}, {(double)win->w, (double)win->h}}, (u32*)format);

	id share = NULL;
	if (hints->share) {
		share = (id)hints->share->ctx;
	}

	win->src.ctx.native->ctx = ((id (*)(id, SEL, id, id))objc_msgSend)(NSAlloc(objc_getClass("NSOpenGLContext")),
												 sel_registerName("initWithFormat:shareContext:"),
												 (id)format, share);

	objc_msgSend_void_id(win->src.view, sel_registerName("setOpenGLContext:"), win->src.ctx.native->ctx);
	if (win->internal.flags & RGFW_windowTransparent) {
		i32 opacity = 0;
		#define NSOpenGLCPSurfaceOpacity 236
		NSOpenGLContext_setValues((id)win->src.ctx.native->ctx, &opacity, (NSOpenGLContextParameter)NSOpenGLCPSurfaceOpacity);

	}

	objc_msgSend_void(win->src.ctx.native->ctx, sel_registerName("makeCurrentContext"));

	objc_msgSend_void_id((id)win->src.window, sel_registerName("setContentView:"), win->src.view);
	objc_msgSend_void_bool(win->src.view, sel_registerName("setWantsLayer:"), true);
	objc_msgSend_int((id)win->src.view, sel_registerName("setLayerContentsPlacement:"),  4);

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, "OpenGL context initalized.");
	return RGFW_TRUE;
}

void RGFW_window_deleteContextPtr_OpenGL(RGFW_window* win, RGFW_glContext* ctx) {
	objc_msgSend_void(ctx->ctx, sel_registerName("release"));
	win->src.ctx.native->ctx = NULL;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, "OpenGL context freed.");
}

void RGFW_window_makeCurrentContext_OpenGL(RGFW_window* win) {
	if (win) RGFW_ASSERT(win->src.ctx.native);
	if (win != NULL)
		objc_msgSend_void(win->src.ctx.native->ctx, sel_registerName("makeCurrentContext"));
	else
		objc_msgSend_id(objc_getClass("NSOpenGLContext"), sel_registerName("clearCurrentContext"));
}
void* RGFW_getCurrentContext_OpenGL(void) {
	return objc_msgSend_id(objc_getClass("NSOpenGLContext"), sel_registerName("currentContext"));
}

void RGFW_window_swapBuffers_OpenGL(RGFW_window* win) {
	RGFW_ASSERT(win && win->src.ctx.native);
	objc_msgSend_void(win->src.ctx.native->ctx, sel_registerName("flushBuffer"));
}
void RGFW_window_swapInterval_OpenGL(RGFW_window* win, i32 swapInterval) {
	RGFW_ASSERT(win != NULL && win->src.ctx.native != NULL);
	NSOpenGLContext_setValues((id)win->src.ctx.native->ctx, &swapInterval, (NSOpenGLContextParameter)222);
}
#endif

void RGFW_deinitPlatform(void) { }

void RGFW_window_closePlatform(RGFW_window* win) {
	NSRelease(win->src.view);
}

#ifdef RGFW_WEBGPU
WGPUSurface RGFW_window_createSurface_WebGPU(RGFW_window* window, WGPUInstance instance) {
	WGPUSurfaceDescriptor surfaceDesc = {0};
    id* nsView = (id*)window->src.view;
    if (!nsView) {
        fprintf(stderr, "RGFW Error: NSView is NULL for macOS window.\n");
        return NULL;
    }

    ((void (*)(id, SEL, BOOL))objc_msgSend)(nsView, sel_registerName("setWantsLayer:"), YES);
    id layer = ((id (*)(id, SEL))objc_msgSend)(nsView, sel_registerName("layer"));

	void* metalLayer = RGFW_getLayer_OSX();
	if (metalLayer == NULL) {
		 return NULL;
	}
	((void (*)(id, SEL, id))objc_msgSend)((id)nsView, sel_registerName("setLayer:"), metalLayer);
	layer = metalLayer; /* Use the newly created layer */

    /* At this point, 'layer' should be a valid CAMetalLayer* */
    WGPUSurfaceSourceMetalLayer fromMetal = {0};
    fromMetal.chain.sType = WGPUSType_SurfaceSourceMetalLayer;
#ifdef  __OBJC__
	fromMetal.layer = (__bridge CAMetalLayer*)layer; /* Use __bridge for ARC compatibility if mixing C/Obj-C */
#else
	fromMetal.layer = layer;
#endif

    surfaceDesc.nextInChain = (WGPUChainedStruct*)&fromMetal.chain;
    return wgpuInstanceCreateSurface(instance, &surfaceDesc);
}
#endif

#endif /* RGFW_MACOS */

/*
	End of MaOS defines
*/

/*
	WASM defines
*/

#ifdef RGFW_WASM
EM_BOOL Emscripten_on_resize(int eventType, const EmscriptenUiEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	if (!(_RGFW->root->internal.enabledEvents & RGFW_windowResizedFlag)) return EM_TRUE;

	RGFW_eventQueuePushEx(e.type = RGFW_windowResized; e.common.win = _RGFW->root);
	RGFW_windowResizedCallback(_RGFW->root, E->windowInnerWidth, E->windowInnerHeight);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_fullscreenchange(int eventType, const EmscriptenFullscreenChangeEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	if (!(_RGFW->root->internal.enabledEvents & RGFW_windowResizedFlag)) return EM_TRUE;

	static u8 fullscreen = RGFW_FALSE;
	static i32 originalW, originalH;

	if (fullscreen == RGFW_FALSE) {
		originalW = _RGFW->root->w;
		originalH = _RGFW->root->h;
	}

	fullscreen = !fullscreen;
	RGFW_eventQueuePushEx(e.type = RGFW_windowResized; e.common.win = _RGFW->root);
	_RGFW->root->w = E->screenWidth;
	_RGFW->root->h =  E->screenHeight;

	EM_ASM("Module.canvas.focus();");

	if (fullscreen == RGFW_FALSE) {
		_RGFW->root->w = originalW;
		_RGFW->root->h = originalH;
	} else {
		#if __EMSCRIPTEN_major__  >= 1 && __EMSCRIPTEN_minor__  >= 29 && __EMSCRIPTEN_tiny__  >= 0
			EmscriptenFullscreenStrategy FSStrat = {0};
			FSStrat.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
			FSStrat.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
			FSStrat.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
			emscripten_request_fullscreen_strategy("#canvas", 1, &FSStrat);
		#else
			emscripten_request_fullscreen("#canvas", 1);
		#endif
	}

	emscripten_set_canvas_element_size("#canvas", _RGFW->root->w, _RGFW->root->h);
	RGFW_windowResizedCallback(_RGFW->root, _RGFW->root->w, _RGFW->root->h);
	return EM_TRUE;
}

EM_BOOL Emscripten_on_focusin(int eventType, const EmscriptenFocusEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData); RGFW_UNUSED(E);

	if (!(_RGFW->root->internal.enabledEvents & RGFW_focusInFlag)) return EM_TRUE;

	RGFW_eventQueuePushEx(e.type = RGFW_focusIn; e.common.win = _RGFW->root);
	_RGFW->root->internal.inFocus = RGFW_TRUE;
	RGFW_focusCallback(_RGFW->root, 1);

	if ((_RGFW->root->internal.holdMouse)) RGFW_window_holdMouse(_RGFW->root);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_focusout(int eventType, const EmscriptenFocusEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData); RGFW_UNUSED(E);

	if (!(_RGFW->root->internal.enabledEvents & RGFW_focusOutFlag)) return EM_TRUE;

	RGFW_eventQueuePushEx(e.type = RGFW_focusOut; e.common.win = _RGFW->root);
    RGFW_window_focusLost(_RGFW->root);
    RGFW_focusCallback(_RGFW->root, 0);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_mousemove(int eventType, const EmscriptenMouseEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	if (!(_RGFW->root->internal.enabledEvents & RGFW_mousePosChangedFlag)) return EM_TRUE;

	RGFW_eventQueuePushEx(e.type = RGFW_mousePosChanged;
							e.mouse.x = E->targetX; e.mouse.y = E->targetY;
							e.mouse.vecX = E->movementX; e.mouse.vecY = E->movementY;
							e.common.win = _RGFW->root);

	_RGFW->vectorX = E->movementX;
	_RGFW->vectorY = E->movementY;
	_RGFW->root->internal.lastMouseX = E->targetX;
	_RGFW->root->internal.lastMouseY = E->targetY;
	RGFW_mousePosCallback(_RGFW->root, E->targetX, E->targetY, E->movementX, E->movementY);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_mousedown(int eventType, const EmscriptenMouseEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	if (!(_RGFW->root->internal.enabledEvents & RGFW_mouseButtonPressedFlag)) return EM_TRUE;

	int button = E->button;
	if (button > 2)
		button += 2;

	RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonPressed;
							e.mouse.x = E->targetX; e.mouse.y = E->targetY;
							e.mouse.vecX = E->movementX; e.mouse.vecY = E->movementY;
							e.button.value = (u8)button;
							e.common.win = _RGFW->root);
	_RGFW->vectorX = E->movementX;
	_RGFW->vectorY = E->movementY;
	_RGFW->mouseButtons[button].prev = _RGFW->mouseButtons[button].current;
	_RGFW->mouseButtons[button].current = 1;

	RGFW_mouseButtonCallback(_RGFW->root, button, 1);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_mouseup(int eventType, const EmscriptenMouseEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	if (!(_RGFW->root->internal.enabledEvents & RGFW_mouseButtonReleasedFlag)) return EM_TRUE;

	int button = E->button;
	if (button > 2)
		button += 2;

	RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonReleased;
							e.mouse.x = E->targetX; e.mouse.y = E->targetY;
							e.mouse.vecX = E->movementX; e.mouse.vecY =  E->movementY;
							e.button.value = (u8)button;
							e.common.win = _RGFW->root);
	_RGFW->vectorX = E->movementX;
	_RGFW->vectorY = E->movementY;
	_RGFW->mouseButtons[button].prev = _RGFW->mouseButtons[button].current;
	_RGFW->mouseButtons[button].current = 0;

	RGFW_mouseButtonCallback(_RGFW->root, button, 0);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_wheel(int eventType, const EmscriptenWheelEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	if (!(_RGFW->root->internal.enabledEvents & RGFW_mouseScrollFlag)) return EM_TRUE;

	_RGFW->scrollX = E->deltaX;
	_RGFW->scrollY = E->deltaY;
	RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonPressed;
							e.scroll.x = E->deltaX;
							e.scroll.y = E->deltaY;
						);
	RGFW_mouseScrollCallback(_RGFW->root, E->deltaX, E->deltaY);

    return EM_TRUE;
}

EM_BOOL Emscripten_on_touchstart(int eventType, const EmscriptenTouchEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	if (!(_RGFW->root->internal.enabledEvents & RGFW_mouseButtonPressedFlag)) return EM_TRUE;

    size_t i;
    for (i = 0; i < (size_t)E->numTouches; i++) {
		RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonPressed;
								e.mouse.x = E->touches[i].targetX; e.mouse.y = E->touches[i].targetY;
								e.button.value = RGFW_mouseLeft;
								e.common.win = _RGFW->root);

	    _RGFW->mouseButtons[RGFW_mouseLeft].prev = _RGFW->mouseButtons[RGFW_mouseLeft].current;
	    _RGFW->mouseButtons[RGFW_mouseLeft].current = 1;

		_RGFW->root->internal.lastMouseX = E->touches[i].targetX;
		_RGFW->root->internal.lastMouseX = E->touches[i].targetY;
        RGFW_mousePosCallback(_RGFW->root, E->touches[i].targetX, E->touches[i].targetY, 0, 0);
	    RGFW_mouseButtonCallback(_RGFW->root, RGFW_mouseLeft, 1);
    }

	return EM_TRUE;
}

EM_BOOL Emscripten_on_touchmove(int eventType, const EmscriptenTouchEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	if (!(_RGFW->root->internal.enabledEvents & RGFW_mousePosChangedFlag)) return EM_TRUE;

    size_t i;
    for (i = 0; i < (size_t)E->numTouches; i++) {
		RGFW_eventQueuePushEx(e.type = RGFW_mousePosChanged;
			e.mouse.x = E->touches[i].targetX;
			e.mouse.y = E->touches[i].targetY;
			e.mouse.x = E->touches[i].targetX; e.mouse.y = E->touches[i].targetY;
			e.button.value = RGFW_mouseLeft;
			e.common.win = _RGFW->root);

		_RGFW->root->internal.lastMouseX = E->touches[i].targetX;
		_RGFW->root->internal.lastMouseX = E->touches[i].targetY;
        RGFW_mousePosCallback(_RGFW->root, E->touches[i].targetX, E->touches[i].targetY, 0, 0);
    }
    return EM_TRUE;
}

EM_BOOL Emscripten_on_touchend(int eventType, const EmscriptenTouchEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	if (!(_RGFW->root->internal.enabledEvents & RGFW_mouseButtonReleasedFlag)) return EM_TRUE;

    size_t i;
    for (i = 0; i < (size_t)E->numTouches; i++) {
		RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonReleased;
								e.mouse.x = E->touches[i].targetX; e.mouse.y = E->touches[i].targetY;
								e.button.value = RGFW_mouseLeft;
								e.common.win = _RGFW->root);

		_RGFW->mouseButtons[RGFW_mouseLeft].prev = _RGFW->mouseButtons[RGFW_mouseLeft].current;
		_RGFW->mouseButtons[RGFW_mouseLeft].current = 0;

		_RGFW->root->internal.lastMouseX = E->touches[i].targetX;
		_RGFW->root->internal.lastMouseY = E->touches[i].targetY;
		RGFW_mousePosCallback(_RGFW->root, E->touches[i].targetX, E->touches[i].targetY, 0, 0);
		RGFW_mouseButtonCallback(_RGFW->root, RGFW_mouseLeft, 0);
    }
	return EM_TRUE;
}

EM_BOOL Emscripten_on_touchcancel(int eventType, const EmscriptenTouchEvent* E, void* userData) { RGFW_UNUSED(eventType); RGFW_UNUSED(userData); return EM_TRUE; }

u32 RGFW_WASMPhysicalToRGFW(u32 hash);

void EMSCRIPTEN_KEEPALIVE RGFW_handleKeyEvent(char* key, char* code, RGFW_bool press) {
	const char* iCode = code;

	u32 hash = 0;
	while(*iCode) hash = ((hash ^ 0x7E057D79U) << 3) ^ (unsigned int)*iCode++;

	u32 physicalKey = RGFW_WASMPhysicalToRGFW(hash);

	u8 mappedKey = (u8)(*((u32*)key));

	if (*((u16*)key) != mappedKey) {
		mappedKey = 0;
		if (*((u32*)key) == *((u32*)"Tab")) mappedKey = RGFW_tab;
	}

	if (!(press ? (_RGFW->root->internal.enabledEvents & RGFW_keyPressedFlag) : (_RGFW->root->internal.enabledEvents & RGFW_keyReleasedFlag))) return;

	RGFW_eventQueuePushEx(e.type = (RGFW_eventType)(press ? RGFW_keyPressed : RGFW_keyReleased);
							e.key.value = (u8)physicalKey;
							e.key.sym = (u8)mappedKey;
							e.key.mod = _RGFW->root->internal.mod;
							e.key.repeat =  RGFW_window_isKeyDown(_RGFW->root, (u8)physicalKey);
							e.common.win = _RGFW->root);

	_RGFW->keyboard[physicalKey].prev = _RGFW->keyboard[physicalKey].current;
	_RGFW->keyboard[physicalKey].current = press;

	RGFW_keyCallback(_RGFW->root, physicalKey, mappedKey, _RGFW->root->internal.mod,  RGFW_window_isKeyDown(_RGFW->root, (u8)physicalKey), press);
}

void EMSCRIPTEN_KEEPALIVE RGFW_handleKeyMods(RGFW_bool capital, RGFW_bool numlock, RGFW_bool control, RGFW_bool alt, RGFW_bool shift, RGFW_bool super, RGFW_bool scroll) {
	RGFW_updateKeyModsEx(_RGFW->root, capital, numlock, control, alt, shift, super, scroll);
}

void EMSCRIPTEN_KEEPALIVE Emscripten_onDrop(size_t count) {
	if (!(_RGFW->root->internal.flags & RGFW_windowAllowDND))
		return;

	if (!(_RGFW->root->internal.enabledEvents & RGFW_dataDropFlag)) return;

	RGFW_eventQueuePushEx(e.type = RGFW_dataDrop;
							e.drop.count = count;
							e.common.win = _RGFW->root);

	_RGFW->windowState.win = _RGFW->root;
	_RGFW->windowState.dataDrop = RGFW_TRUE;
	_RGFW->windowState.filesCount = count;
	RGFW_dataDropCallback(_RGFW->root, _RGFW->files, count);
}

void RGFW_stopCheckEvents(void) {
	_RGFW->stopCheckEvents_bool = RGFW_TRUE;
}

RGFW_bool RGFW_createSurfacePtr(u8* data, i32 w, i32 h, RGFW_format format, RGFW_surface* surface) {
	surface->data = data;
	surface->w = w;
	surface->h = h;
	surface->format = format;
	return RGFW_TRUE;
}

void RGFW_window_blitSurface(RGFW_window* win, RGFW_surface* surface) {
	/* TODO: Needs fixing. */
	RGFW_copyImageData(surface->data, surface->w, RGFW_MIN(win->h, surface->h), RGFW_formatRGBA8, surface->data, surface->format);
	EM_ASM_({
		var data = Module.HEAPU8.slice($0, $0 + $1 * $2 * 4);
		let context = document.getElementById("canvas").getContext("2d");
		let image = context.getImageData(0, 0, $1, $2);
		image.data.set(data);
		context.putImageData(image, 0, $4 - $2);
	}, surface->data, surface->w, surface->h, RGFW_MIN(win->h, surface->w), RGFW_MIN(win->h, surface->h));
}

void RGFW_surface_freePtr(RGFW_surface* surface) { }

void EMSCRIPTEN_KEEPALIVE RGFW_makeSetValue(size_t index, char* file) {
	/* This seems like a terrible idea, don't replicate this unless you hate yourself or the OS */
	/* TODO: find a better way to do this
	*/
	RGFW_STRNCPY((char*)_RGFW->files[index], file, RGFW_MAX_PATH - 1);
	_RGFW->files[index][RGFW_MAX_PATH - 1] = '\0';
}

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>

void EMSCRIPTEN_KEEPALIVE RGFW_mkdir(char* name) { mkdir(name, 0755); }

void EMSCRIPTEN_KEEPALIVE RGFW_writeFile(const char *path, const char *data, size_t len) {
    FILE* file = fopen(path, "w+");
	if (file == NULL)
		return;

    fwrite(data, sizeof(char), len, file);
    fclose(file);
}

void RGFW_initKeycodesPlatform(void) {
	_RGFW->keycodes[DOM_VK_BACK_QUOTE] = RGFW_backtick;
	_RGFW->keycodes[DOM_VK_0] = RGFW_0;
	_RGFW->keycodes[DOM_VK_1] = RGFW_1;
	_RGFW->keycodes[DOM_VK_2] = RGFW_2;
	_RGFW->keycodes[DOM_VK_3] = RGFW_3;
	_RGFW->keycodes[DOM_VK_4] = RGFW_4;
	_RGFW->keycodes[DOM_VK_5] = RGFW_5;
	_RGFW->keycodes[DOM_VK_6] = RGFW_6;
	_RGFW->keycodes[DOM_VK_7] = RGFW_7;
	_RGFW->keycodes[DOM_VK_8] = RGFW_8;
	_RGFW->keycodes[DOM_VK_9] = RGFW_9;
	_RGFW->keycodes[DOM_VK_SPACE] = RGFW_space;
	_RGFW->keycodes[DOM_VK_A] = RGFW_a;
	_RGFW->keycodes[DOM_VK_B] = RGFW_b;
	_RGFW->keycodes[DOM_VK_C] = RGFW_c;
	_RGFW->keycodes[DOM_VK_D] = RGFW_d;
	_RGFW->keycodes[DOM_VK_E] = RGFW_e;
	_RGFW->keycodes[DOM_VK_F] = RGFW_f;
	_RGFW->keycodes[DOM_VK_G] = RGFW_g;
	_RGFW->keycodes[DOM_VK_H] = RGFW_h;
	_RGFW->keycodes[DOM_VK_I] = RGFW_i;
	_RGFW->keycodes[DOM_VK_J] = RGFW_j;
	_RGFW->keycodes[DOM_VK_K] = RGFW_k;
	_RGFW->keycodes[DOM_VK_L] = RGFW_l;
	_RGFW->keycodes[DOM_VK_M] = RGFW_m;
	_RGFW->keycodes[DOM_VK_N] = RGFW_n;
	_RGFW->keycodes[DOM_VK_O] = RGFW_o;
	_RGFW->keycodes[DOM_VK_P] = RGFW_p;
	_RGFW->keycodes[DOM_VK_Q] = RGFW_q;
	_RGFW->keycodes[DOM_VK_R] = RGFW_r;
	_RGFW->keycodes[DOM_VK_S] = RGFW_s;
	_RGFW->keycodes[DOM_VK_T] = RGFW_t;
	_RGFW->keycodes[DOM_VK_U] = RGFW_u;
	_RGFW->keycodes[DOM_VK_V] = RGFW_v;
	_RGFW->keycodes[DOM_VK_W] = RGFW_w;
	_RGFW->keycodes[DOM_VK_X] = RGFW_x;
	_RGFW->keycodes[DOM_VK_Y] = RGFW_y;
	_RGFW->keycodes[DOM_VK_Z] = RGFW_z;
	_RGFW->keycodes[DOM_VK_PERIOD] = RGFW_period;
	_RGFW->keycodes[DOM_VK_COMMA] = RGFW_comma;
	_RGFW->keycodes[DOM_VK_SLASH] = RGFW_slash;
	_RGFW->keycodes[DOM_VK_OPEN_BRACKET] = RGFW_bracket;
	_RGFW->keycodes[DOM_VK_CLOSE_BRACKET] = RGFW_closeBracket;
	_RGFW->keycodes[DOM_VK_SEMICOLON] = RGFW_semicolon;
	_RGFW->keycodes[DOM_VK_QUOTE] = RGFW_apostrophe;
	_RGFW->keycodes[DOM_VK_BACK_SLASH] = RGFW_backSlash;
	_RGFW->keycodes[DOM_VK_RETURN] = RGFW_return;
	_RGFW->keycodes[DOM_VK_DELETE] = RGFW_delete;
	_RGFW->keycodes[DOM_VK_NUM_LOCK] = RGFW_numLock;
	_RGFW->keycodes[DOM_VK_DIVIDE] = RGFW_kpSlash;
	_RGFW->keycodes[DOM_VK_MULTIPLY] = RGFW_kpMultiply;
	_RGFW->keycodes[DOM_VK_SUBTRACT] = RGFW_kpMinus;
	_RGFW->keycodes[DOM_VK_NUMPAD1] = RGFW_kp1;
	_RGFW->keycodes[DOM_VK_NUMPAD2] = RGFW_kp2;
	_RGFW->keycodes[DOM_VK_NUMPAD3] = RGFW_kp3;
	_RGFW->keycodes[DOM_VK_NUMPAD4] = RGFW_kp4;
	_RGFW->keycodes[DOM_VK_NUMPAD5] = RGFW_kp5;
	_RGFW->keycodes[DOM_VK_NUMPAD6] = RGFW_kp6;
	_RGFW->keycodes[DOM_VK_NUMPAD9] = RGFW_kp9;
	_RGFW->keycodes[DOM_VK_NUMPAD0] = RGFW_kp0;
	_RGFW->keycodes[DOM_VK_DECIMAL] = RGFW_kpPeriod;
	_RGFW->keycodes[DOM_VK_RETURN] = RGFW_kpReturn;
	_RGFW->keycodes[DOM_VK_HYPHEN_MINUS] = RGFW_minus;
	_RGFW->keycodes[DOM_VK_EQUALS] = RGFW_equals;
	_RGFW->keycodes[DOM_VK_BACK_SPACE] = RGFW_backSpace;
	_RGFW->keycodes[DOM_VK_TAB] = RGFW_tab;
	_RGFW->keycodes[DOM_VK_CAPS_LOCK] = RGFW_capsLock;
	_RGFW->keycodes[DOM_VK_SHIFT] = RGFW_shiftL;
	_RGFW->keycodes[DOM_VK_CONTROL] = RGFW_controlL;
	_RGFW->keycodes[DOM_VK_ALT] = RGFW_altL;
	_RGFW->keycodes[DOM_VK_META] = RGFW_superL;
	_RGFW->keycodes[DOM_VK_F1] = RGFW_F1;
	_RGFW->keycodes[DOM_VK_F2] = RGFW_F2;
	_RGFW->keycodes[DOM_VK_F3] = RGFW_F3;
	_RGFW->keycodes[DOM_VK_F4] = RGFW_F4;
	_RGFW->keycodes[DOM_VK_F5] = RGFW_F5;
	_RGFW->keycodes[DOM_VK_F6] = RGFW_F6;
	_RGFW->keycodes[DOM_VK_F7] = RGFW_F7;
	_RGFW->keycodes[DOM_VK_F8] = RGFW_F8;
	_RGFW->keycodes[DOM_VK_F9] = RGFW_F9;
	_RGFW->keycodes[DOM_VK_F10] = RGFW_F10;
	_RGFW->keycodes[DOM_VK_F11] = RGFW_F11;
	_RGFW->keycodes[DOM_VK_F12] = RGFW_F12;
	_RGFW->keycodes[DOM_VK_UP] = RGFW_up;
	_RGFW->keycodes[DOM_VK_DOWN] = RGFW_down;
	_RGFW->keycodes[DOM_VK_LEFT] = RGFW_left;
	_RGFW->keycodes[DOM_VK_RIGHT] = RGFW_right;
	_RGFW->keycodes[DOM_VK_INSERT] = RGFW_insert;
	_RGFW->keycodes[DOM_VK_END] = RGFW_end;
	_RGFW->keycodes[DOM_VK_PAGE_UP] = RGFW_pageUp;
	_RGFW->keycodes[DOM_VK_PAGE_DOWN] = RGFW_pageDown;
	_RGFW->keycodes[DOM_VK_ESCAPE] = RGFW_escape;
	_RGFW->keycodes[DOM_VK_HOME] = RGFW_home;
	_RGFW->keycodes[DOM_VK_SCROLL_LOCK] = RGFW_scrollLock;
	_RGFW->keycodes[DOM_VK_PRINTSCREEN] = RGFW_printScreen;
	_RGFW->keycodes[DOM_VK_PAUSE] = RGFW_pause;
	_RGFW->keycodes[DOM_VK_F13]  = RGFW_F13;
	_RGFW->keycodes[DOM_VK_F14]  = RGFW_F14;
	_RGFW->keycodes[DOM_VK_F15]  = RGFW_F15;
	_RGFW->keycodes[DOM_VK_F16]  = RGFW_F16;
	_RGFW->keycodes[DOM_VK_F17]  = RGFW_F17;
	_RGFW->keycodes[DOM_VK_F18]  = RGFW_F18;
	_RGFW->keycodes[DOM_VK_F19]  = RGFW_F19;
	_RGFW->keycodes[DOM_VK_F20]  = RGFW_F20;
	_RGFW->keycodes[DOM_VK_F21]  = RGFW_F21;
	_RGFW->keycodes[DOM_VK_F22]  = RGFW_F22;
	_RGFW->keycodes[DOM_VK_F23]  = RGFW_F23;
	_RGFW->keycodes[DOM_VK_F24]  = RGFW_F24;
}

i32 RGFW_initPlatform(void) { return 0; }

RGFW_window* RGFW_createWindowPlatform(const char* name, RGFW_windowFlags flags, RGFW_window* win) {
	emscripten_set_canvas_element_size("#canvas", win->w, win->h);
	emscripten_set_window_title(name);

	/* load callbacks */
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_FALSE, Emscripten_on_resize);
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, EM_FALSE, Emscripten_on_fullscreenchange);
    emscripten_set_mousemove_callback("#canvas", NULL, EM_FALSE, Emscripten_on_mousemove);
    emscripten_set_touchstart_callback("#canvas", NULL, EM_FALSE, Emscripten_on_touchstart);
    emscripten_set_touchend_callback("#canvas", NULL, EM_FALSE, Emscripten_on_touchend);
    emscripten_set_touchmove_callback("#canvas", NULL, EM_FALSE, Emscripten_on_touchmove);
    emscripten_set_touchcancel_callback("#canvas", NULL, EM_FALSE, Emscripten_on_touchcancel);
    emscripten_set_mousedown_callback("#canvas", NULL, EM_FALSE, Emscripten_on_mousedown);
    emscripten_set_mouseup_callback("#canvas", NULL, EM_FALSE, Emscripten_on_mouseup);
    emscripten_set_wheel_callback("#canvas", NULL, EM_FALSE, Emscripten_on_wheel);
    emscripten_set_focusin_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_FALSE, Emscripten_on_focusin);
    emscripten_set_focusout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_FALSE, Emscripten_on_focusout);

	if (flags & RGFW_windowAllowDND)  {
		win->internal.flags |= RGFW_windowAllowDND;
	}

	EM_ASM({
		window.addEventListener("keydown",
			(event) => {
				var key = stringToNewUTF8(event.key); var code = stringToNewUTF8(event.code);
				Module._RGFW_handleKeyMods(event.getModifierState("CapsLock"), event.getModifierState("NumLock"), event.getModifierState("Control"), event.getModifierState("Alt"), event.getModifierState("Shift"), event.getModifierState("Meta"), event.getModifierState("ScrollLock"));
				Module._RGFW_handleKeyEvent(key, code, 1);
				_free(key); _free(code);
			},
		true);
		window.addEventListener("keyup",
			(event) => {
				var key = stringToNewUTF8(event.key); var code = stringToNewUTF8(event.code);
				Module._RGFW_handleKeyMods(event.getModifierState("CapsLock"), event.getModifierState("NumLock"), event.getModifierState("Control"), event.getModifierState("Alt"), event.getModifierState("Shift"), event.getModifierState("Meta"), event.getModifierState("ScrollLock"));
				Module._RGFW_handleKeyEvent(key, code, 0);
				_free(key); _free(code);
			},
		true);
	});

    EM_ASM({
		var canvas = document.getElementById('canvas');
        canvas.addEventListener('drop', function(e) {
            e.preventDefault();
            if (e.dataTransfer.file < 0)
				return;

			var filenamesArray = [];
			var count = e.dataTransfer.files.length;

			/* Read and save the files to emscripten's files */
			var drop_dir = '.rgfw_dropped_files';
			Module._RGFW_mkdir(drop_dir);

			for (var i = 0; i < count; i++) {
				var file = e.dataTransfer.files[i];

				var path = '/' + drop_dir + '/' + file.name.replace("//", '_');
				var reader = new FileReader();

				reader.onloadend = (e) => {
					if (reader.readyState != 2) {
						out('failed to read dropped file: '+file.name+': '+reader.error);
					}
					else {
						var data = e.target.result;

						Module._RGFW_writeFile(path, new Uint8Array(data), file.size);
					}
				};

				reader.readAsArrayBuffer(file);
				/* This works weird on modern OpenGL */
				var filename = stringToNewUTF8(path);

				filenamesArray.push(filename);

				Module._RGFW_makeSetValue(i, filename);
			}

			Module._Emscripten_onDrop(count);

			for (var i = 0; i < count; ++i) {
				_free(filenamesArray[i]);
			}
        }, true);

        canvas.addEventListener('dragover', function(e) { e.preventDefault(); return false; }, true);
    });

	return win;
}

u8 RGFW_rgfwToKeyChar(u32 rgfw_keycode) {
    return (u8)rgfw_keycode; /* TODO */
}

void RGFW_pollEvents(void) {
    emscripten_sleep(0);
	RGFW_resetPrevState();
}

void RGFW_window_resize(RGFW_window* win, i32 w, i32 h) {
	RGFW_UNUSED(win);
	emscripten_set_canvas_element_size("#canvas", w, h);
}

/* NOTE: I don't know if this is possible */
void RGFW_window_moveMouse(RGFW_window* win, i32 x, i32 y) { RGFW_UNUSED(win); RGFW_UNUSED(x); RGFW_UNUSED(y); }
/* this one might be possible but it looks iffy */
RGFW_mouse* RGFW_loadMouse(u8* data, i32 w, i32 h, RGFW_format format) { RGFW_UNUSED(data); RGFW_UNUSED(w); RGFW_UNUSED(h); RGFW_UNUSED(format); return NULL; }

void RGFW_window_setMouse(RGFW_window* win, RGFW_mouse* mouse) { RGFW_UNUSED(win); RGFW_UNUSED(mouse); }
void RGFW_freeMouse(RGFW_mouse* mouse) { RGFW_UNUSED(mouse); }

RGFW_bool RGFW_window_setMouseStandard(RGFW_window* win, u8 mouse) {
	static const char cursors[16][16] = {
		"default", "default", "text", "crosshair",
		"pointer", "ew-resize", "ns-resize", "nwse-resize", "nesw-resize",
		"move", "not-allowed"
	};

	RGFW_UNUSED(win);
	EM_ASM( { document.getElementById("canvas").style.cursor = UTF8ToString($0); }, cursors[mouse]);
	return RGFW_TRUE;
}

RGFW_bool RGFW_window_setMouseDefault(RGFW_window* win) {
	return RGFW_window_setMouseStandard(win, RGFW_mouseNormal);
}

void RGFW_window_showMouse(RGFW_window* win, RGFW_bool show) {
	RGFW_window_showMouseFlags(win, show);
	if (show)
		RGFW_window_setMouseDefault(win);
	else
		EM_ASM(document.getElementById('canvas').style.cursor = 'none';);
}

RGFW_bool RGFW_getGlobalMouse(i32* x, i32* y) {
    if(x) *x = EM_ASM_INT({
        return window.mouseX || 0;
    });
    if (y) *y = EM_ASM_INT({
        return window.mouseY || 0;
    });
    return RGFW_TRUE;
}

void RGFW_window_setMousePassthrough(RGFW_window* win, RGFW_bool passthrough) {
	RGFW_UNUSED(win);

    EM_ASM_({
        var canvas = document.getElementById('canvas');
        if ($0) {
            canvas.style.pointerEvents = 'none';
        } else {
            canvas.style.pointerEvents = 'auto';
        }
    }, passthrough);
}

void RGFW_writeClipboard(const char* text, u32 textLen) {
	RGFW_UNUSED(textLen);
	EM_ASM({ navigator.clipboard.writeText(UTF8ToString($0)); }, text);
}


RGFW_ssize_t RGFW_readClipboardPtr(char* str, size_t strCapacity) {
	RGFW_UNUSED(str); RGFW_UNUSED(strCapacity);
	/*
		placeholder code for later
		I'm not sure if this is possible do the the async stuff
	*/
	return 0;
}

#ifdef RGFW_OPENGL
RGFW_bool RGFW_window_createContextPtr_OpenGL(RGFW_window* win, RGFW_glContext* ctx, RGFW_glHints* hints) {
	win->src.ctx.native = ctx;
	win->src.gfxType = RGFW_gfxNativeOpenGL;

	EmscriptenWebGLContextAttributes attrs;
	attrs.alpha = hints->alpha;
	attrs.depth = hints->depth;
	attrs.stencil = hints->stencil;
	attrs.antialias = hints->samples;
	attrs.premultipliedAlpha = EM_TRUE;
	attrs.preserveDrawingBuffer = EM_FALSE;

	if (hints->doubleBuffer == 0)
		attrs.renderViaOffscreenBackBuffer = 0;
	else
		attrs.renderViaOffscreenBackBuffer = hints->auxBuffers;

	attrs.failIfMajorPerformanceCaveat = EM_FALSE;
	attrs.majorVersion = (hints->major == 0) ? 1 : hints->major;
	attrs.minorVersion = hints->minor;

	attrs.enableExtensionsByDefault = EM_TRUE;
	attrs.explicitSwapControl = EM_TRUE;

	emscripten_webgl_init_context_attributes(&attrs);
	win->src.ctx.native->ctx = emscripten_webgl_create_context("#canvas", &attrs);
	emscripten_webgl_make_context_current(win->src.ctx.native->ctx);

	#ifdef LEGACY_GL_EMULATION
	EM_ASM("Module.useWebGL = true; GLImmediate.init();");
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, "OpenGL context initalized.");
    #endif
	return RGFW_TRUE;
}

void RGFW_window_deleteContextPtr_OpenGL(RGFW_window* win, RGFW_glContext* ctx) {
	emscripten_webgl_destroy_context(ctx->ctx);
	win->src.ctx.native->ctx = 0;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, "OpenGL context freed.");
}

void RGFW_window_makeCurrentContext_OpenGL(RGFW_window* win) {
	if (win) RGFW_ASSERT(win->src.ctx.native);
	if (win == NULL)
	    emscripten_webgl_make_context_current(0);
	else
	    emscripten_webgl_make_context_current(win->src.ctx.native->ctx);
}

void RGFW_window_swapBuffers_OpenGL(RGFW_window* win) {
	RGFW_ASSERT(win && win->src.ctx.native);
	emscripten_webgl_commit_frame();
}
void* RGFW_getCurrentContext_OpenGL(void) { return (void*)emscripten_webgl_get_current_context(); }

RGFW_bool RGFW_extensionSupportedPlatform_OpenGL(const char* extension, size_t len) {
    return EM_ASM_INT({
        var ext = UTF8ToString($0, $1);
        var canvas = document.querySelector('canvas');
        var gl = canvas.getContext('webgl') || canvas.getContext('experimental-webgl');
        if (!gl) return 0;

        var supported = gl.getSupportedExtensions();
        return supported && supported.includes(ext) ? 1 : 0;
    }, extension, len);
    return RGFW_FALSE;
}

RGFW_proc RGFW_getProcAddress_OpenGL(const char* procname) {
    return (RGFW_proc)emscripten_webgl_get_proc_address(procname);
    return NULL;
}

#endif

void RGFW_window_swapInterval_OpenGL(RGFW_window* win, i32 swapInterval) { RGFW_UNUSED(win); RGFW_UNUSED(swapInterval); }

void RGFW_deinitPlatform(void) { }

void RGFW_window_closePlatform(RGFW_window* win) { }

int RGFW_innerWidth(void) {   return EM_ASM_INT({ return window.innerWidth; });  }
int RGFW_innerHeight(void) {  return EM_ASM_INT({ return window.innerHeight; });  }

void RGFW_releaseCursor(RGFW_window* win) {
	RGFW_UNUSED(win);
	emscripten_exit_pointerlock();
}

void RGFW_captureCursor(RGFW_window* win) {
	RGFW_UNUSED(win);
	emscripten_request_pointerlock("#canvas", 1);
}


void RGFW_window_setName(RGFW_window* win, const char* name) {
	RGFW_UNUSED(win);
	emscripten_set_window_title(name);
}

void RGFW_window_maximize(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	RGFW_monitor mon = RGFW_window_getMonitor(win);
	RGFW_window_move(win, 0, 0);
	RGFW_window_resize(win, mon.mode.w, mon.mode.h);
}

void RGFW_window_setFullscreen(RGFW_window* win, RGFW_bool fullscreen) {
	RGFW_ASSERT(win != NULL);
	if (fullscreen) {
		win->internal.flags |= RGFW_windowFullscreen;
		EM_ASM( Module.requestFullscreen(false, true); );
		return;
	}
	win->internal.flags &= ~(u32)RGFW_windowFullscreen;
	EM_ASM( Module.exitFullscreen(false, true); );
}

void RGFW_window_setOpacity(RGFW_window* win, u8 opacity) {
	RGFW_UNUSED(win);
	EM_ASM({
		var element = document.getElementById("canvas");
		if (element)
		  element.style.opacity = $1;
	  }, "elementId", opacity);
}

#ifdef RGFW_WEBGPU
WGPUSurface RGFW_window_createSurface_WebGPU(RGFW_window* window, WGPUInstance instance) {
	WGPUSurfaceDescriptor surfaceDesc = {0};
	WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvasDesc = {0};
    canvasDesc.chain.sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
    canvasDesc.selector = (WGPUStringView){.data = "#canvas", .length = 7};

    surfaceDesc.nextInChain = &canvasDesc.chain;
    return wgpuInstanceCreateSurface(instance, &surfaceDesc);
}
#endif

u32 RGFW_WASMPhysicalToRGFW(u32 hash) {
	switch(hash) {             /* 0x0000 */
		case 0x67243A2DU /* Escape             */: return RGFW_escape;               /* 0x0001 */
		case 0x67251058U /* Digit0             */: return RGFW_0;                    /* 0x0002 */
		case 0x67251059U /* Digit1             */: return RGFW_1;                    /* 0x0003 */
		case 0x6725105AU /* Digit2             */: return RGFW_2;                    /* 0x0004 */
		case 0x6725105BU /* Digit3             */: return RGFW_3;                    /* 0x0005 */
		case 0x6725105CU /* Digit4             */: return RGFW_4;                    /* 0x0006 */
		case 0x6725105DU /* Digit5             */: return RGFW_5;                    /* 0x0007 */
		case 0x6725105EU /* Digit6             */: return RGFW_6;                    /* 0x0008 */
		case 0x6725105FU /* Digit7             */: return RGFW_7;                    /* 0x0009 */
		case 0x67251050U /* Digit8             */: return RGFW_8;                    /* 0x000A */
		case 0x67251051U /* Digit9             */: return RGFW_9;                    /* 0x000B */
		case 0x92E14DD3U /* Minus              */: return RGFW_minus;                /* 0x000C */
		case 0x92E1FBACU /* Equal              */: return RGFW_equals;                /* 0x000D */
		case 0x36BF1CB5U /* Backspace          */: return RGFW_backSpace;            /* 0x000E */
		case 0x7B8E51E2U  /* Tab                */: return RGFW_tab;                  /* 0x000F */
		case 0x2C595B51U /* KeyQ               */: return RGFW_q;                    /* 0x0010 */
		case 0x2C595B57U /* KeyW               */: return RGFW_w;                    /* 0x0011 */
		case 0x2C595B45U /* KeyE               */: return RGFW_e;                    /* 0x0012 */
		case 0x2C595B52U /* KeyR               */: return RGFW_r;                    /* 0x0013 */
		case 0x2C595B54U /* KeyT               */: return RGFW_t;                    /* 0x0014 */
		case 0x2C595B59U /* KeyY               */: return RGFW_y;                    /* 0x0015 */
		case 0x2C595B55U /* KeyU               */: return RGFW_u;                    /* 0x0016 */
		case 0x2C595B4FU /* KeyO               */: return RGFW_o;                    /* 0x0018 */
		case 0x2C595B50U /* KeyP               */: return RGFW_p;                    /* 0x0019 */
		case 0x45D8158CU /* BracketLeft        */: return RGFW_closeBracket;         /* 0x001A */
		case 0xDEEABF7CU /* BracketRight       */: return RGFW_bracket;        /* 0x001B */
		case 0x92E1C5D2U /* Enter              */: return RGFW_return;                /* 0x001C */
		case 0xE058958CU /* ControlLeft        */: return RGFW_controlL;         /* 0x001D */
		case 0x2C595B41U /* KeyA               */: return RGFW_a;                    /* 0x001E */
		case 0x2C595B53U /* KeyS               */: return RGFW_s;                    /* 0x001F */
		case 0x2C595B44U /* KeyD               */: return RGFW_d;                    /* 0x0020 */
		case 0x2C595B46U /* KeyF               */: return RGFW_f;                    /* 0x0021 */
		case 0x2C595B47U /* KeyG               */: return RGFW_g;                    /* 0x0022 */
		case 0x2C595B48U /* KeyH               */: return RGFW_h;                    /* 0x0023 */
		case 0x2C595B4AU /* KeyJ               */: return RGFW_j;                    /* 0x0024 */
		case 0x2C595B4BU /* KeyK               */: return RGFW_k;                    /* 0x0025 */
		case 0x2C595B4CU /* KeyL               */: return RGFW_l;                    /* 0x0026 */
		case 0x2707219EU /* Semicolon          */: return RGFW_semicolon;            /* 0x0027 */
		case 0x92E0B58DU /* Quote              */: return RGFW_apostrophe;                /* 0x0028 */
		case 0x36BF358DU /* Backquote          */: return RGFW_backtick;            /* 0x0029 */
		case 0x26B1958CU /* ShiftLeft          */: return RGFW_shiftL;           /* 0x002A */
		case 0x36BF2438U /* Backslash          */: return RGFW_backSlash;            /* 0x002B */
		case 0x2C595B5AU /* KeyZ               */: return RGFW_z;                    /* 0x002C */
		case 0x2C595B58U /* KeyX               */: return RGFW_x;                    /* 0x002D */
		case 0x2C595B43U /* KeyC               */: return RGFW_c;                    /* 0x002E */
		case 0x2C595B56U /* KeyV               */: return RGFW_v;                    /* 0x002F */
		case 0x2C595B42U /* KeyB               */: return RGFW_b;                    /* 0x0030 */
		case 0x2C595B4EU /* KeyN               */: return RGFW_n;                    /* 0x0031 */
		case 0x2C595B4DU /* KeyM               */: return RGFW_m;                    /* 0x0032 */
		case 0x92E1A1C1U /* Comma              */: return RGFW_comma;                /* 0x0033 */
		case 0x672FFAD4U /* Period             */: return RGFW_period;               /* 0x0034 */
		case 0x92E0A438U /* Slash              */: return RGFW_slash;                /* 0x0035 */
		case 0xC5A6BF7CU /* ShiftRight         */: return RGFW_shiftR;
		case 0x5D64DA91U /* NumpadMultiply     */: return RGFW_kpMultiply;
		case 0xC914958CU /* AltLeft            */: return RGFW_altL;             /* 0x0038 */
		case 0x92E09CB5U /* Space              */: return RGFW_space;                /* 0x0039 */
		case 0xB8FAE73BU  /* CapsLock           */: return RGFW_capsLock;            /* 0x003A */
		case 0x7174B789U /* F1                 */: return RGFW_F1;                   /* 0x003B */
		case 0x7174B78AU /* F2                 */: return RGFW_F2;                   /* 0x003C */
		case 0x7174B78BU /* F3                 */: return RGFW_F3;                   /* 0x003D */
		case 0x7174B78CU /* F4                 */: return RGFW_F4;                   /* 0x003E */
		case 0x7174B78DU /* F5                 */: return RGFW_F5;                   /* 0x003F */
		case 0x7174B78EU /* F6                 */: return RGFW_F6;                   /* 0x0040 */
		case 0x7174B78FU /* F7                 */: return RGFW_F7;                   /* 0x0041 */
		case 0x7174B780U /* F8                 */: return RGFW_F8;                   /* 0x0042 */
		case 0x7174B781U /* F9                 */: return RGFW_F9;                   /* 0x0043 */
		case 0x7B8E57B0U  /* F10                */: return RGFW_F10;                  /* 0x0044 */
		case 0xC925FCDFU /* Numpad7            */: return RGFW_kpMultiply;             /* 0x0047 */
		case 0xC925FCD0U /* Numpad8            */: return RGFW_kp8;             /* 0x0048 */
		case 0xC925FCD1U /* Numpad9            */: return RGFW_kp9;             /* 0x0049 */
		case 0x5EA3E8A4U /* NumpadSubtract     */: return RGFW_minus;      /* 0x004A */
		case 0xC925FCDCU /* Numpad4            */: return RGFW_kp4;             /* 0x004B */
		case 0xC925FCDDU /* Numpad5            */: return RGFW_kp5;             /* 0x004C */
		case 0xC925FCDEU /* Numpad6            */: return RGFW_kp6;             /* 0x004D */
		case 0xC925FCD9U /* Numpad1            */: return RGFW_kp1;             /* 0x004F */
		case 0xC925FCDAU /* Numpad2            */: return RGFW_kp2;             /* 0x0050 */
		case 0xC925FCDBU /* Numpad3            */: return RGFW_kp3;             /* 0x0051 */
		case 0xC925FCD8U /* Numpad0            */: return RGFW_kp0;             /* 0x0052 */
		case 0x95852DACU /* NumpadDecimal      */: return RGFW_period;       /* 0x0053 */
		case 0x7B8E57B1U  /* F11                */: return RGFW_F11;                  /* 0x0057 */
		case 0x7B8E57B2U  /* F12                */: return RGFW_F12;                  /* 0x0058 */
		case 0x7B8E57B3U /* F13                */: return DOM_PK_F13;                  /* 0x0064 */
		case 0x7B8E57B4U /* F14                */: return DOM_PK_F14;                  /* 0x0065 */
		case 0x7B8E57B5U /* F15                */: return DOM_PK_F15;                  /* 0x0066 */
		case 0x7B8E57B6U /* F16                */: return DOM_PK_F16;                  /* 0x0067 */
		case 0x7B8E57B7U /* F17                */: return DOM_PK_F17;                  /* 0x0068 */
		case 0x7B8E57B8U /* F18                */: return DOM_PK_F18;                  /* 0x0069 */
		case 0x7B8E57B9U /* F19                */: return DOM_PK_F19;                  /* 0x006A */
		case 0x7B8E57A8U /* F20                */: return DOM_PK_F20;                  /* 0x006B */
		case 0x7B8E57A9U /* F21                */: return DOM_PK_F21;                  /* 0x006C */
		case 0x7B8E57AAU /* F22                */: return DOM_PK_F22;                  /* 0x006D */
		case 0x7B8E57ABU /* F23                */: return DOM_PK_F23;                  /* 0x006E */
		case 0x7393FBACU /* NumpadEqual        */: return RGFW_kpReturn;
		case 0xB88EBF7CU  /* AltRight           */: return RGFW_altR;            /* 0xE038 */
		case 0xC925873BU /* NumLock            */: return RGFW_numLock;             /* 0xE045 */
		case 0x2C595F45U /* Home               */: return RGFW_home;                 /* 0xE047 */
		case 0xC91BB690U /* ArrowUp            */: return RGFW_up;             /* 0xE048 */
		case 0x672F9210U /* PageUp             */: return RGFW_pageUp;              /* 0xE049 */
		case 0x3799258CU /* ArrowLeft          */: return RGFW_left;           /* 0xE04B */
		case 0x4CE33F7CU /* ArrowRight         */: return RGFW_right;          /* 0xE04D */
		case 0x7B8E55DCU  /* End                */: return RGFW_end;                  /* 0xE04F */
		case 0x3799379EU /* ArrowDown          */: return RGFW_down;           /* 0xE050 */
		case 0xBA90179EU /* PageDown           */: return RGFW_pageDown;            /* 0xE051 */
		case 0x6723CB2CU /* Insert             */: return RGFW_insert;               /* 0xE052 */
		case 0x6725C50DU /* Delete             */: return RGFW_delete;               /* 0xE053 */
		case 0x6723658CU /* OSLeft             */: return RGFW_superL;              /* 0xE05B */
		case 0x39643F7CU /* MetaRight          */: return RGFW_superR;           /* 0xE05C */
		case 0x380B9C8CU /* NumpadAdd          */: return DOM_PK_NUMPAD_ADD;           /* 0x004E */
		default: return DOM_PK_UNKNOWN;
	}

	return 0;
}

/* unsupported functions */
void RGFW_window_focus(RGFW_window* win) { RGFW_UNUSED(win); }
void RGFW_window_raise(RGFW_window* win) { RGFW_UNUSED(win); }
RGFW_bool RGFW_monitor_requestMode(RGFW_monitor mon, RGFW_monitorMode mode, RGFW_modeRequest request) { RGFW_UNUSED(mon); RGFW_UNUSED(mode); RGFW_UNUSED(request); return RGFW_FALSE; }
RGFW_monitor* RGFW_getMonitors(size_t* len) { RGFW_UNUSED(len); return NULL; }
RGFW_monitor RGFW_getPrimaryMonitor(void) { return (RGFW_monitor){}; }
void RGFW_window_move(RGFW_window* win, i32 x, i32 y) { RGFW_UNUSED(win);  RGFW_UNUSED(x); RGFW_UNUSED(y);  }
void RGFW_window_setAspectRatio(RGFW_window* win, i32 w, i32 h) { RGFW_UNUSED(win);  RGFW_UNUSED(w); RGFW_UNUSED(h);  }
void RGFW_window_setMinSize(RGFW_window* win, i32 w, i32 h) { RGFW_UNUSED(win); RGFW_UNUSED(w); RGFW_UNUSED(h);  }
void RGFW_window_setMaxSize(RGFW_window* win, i32 w, i32 h) { RGFW_UNUSED(win);  RGFW_UNUSED(w); RGFW_UNUSED(h);  }
void RGFW_window_minimize(RGFW_window* win) { RGFW_UNUSED(win); }
void RGFW_window_restore(RGFW_window* win) { RGFW_UNUSED(win); }
void RGFW_window_setFloating(RGFW_window* win, RGFW_bool floating) { RGFW_UNUSED(win); RGFW_UNUSED(floating); }
void RGFW_window_setBorder(RGFW_window* win, RGFW_bool border) { RGFW_UNUSED(win); RGFW_UNUSED(border);  }
RGFW_bool RGFW_window_setIconEx(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format, RGFW_icon type) { RGFW_UNUSED(win); RGFW_UNUSED(data); RGFW_UNUSED(w); RGFW_UNUSED(h); RGFW_UNUSED(format);  RGFW_UNUSED(type); return RGFW_FALSE;  }
void RGFW_window_hide(RGFW_window* win) { RGFW_UNUSED(win); }
void RGFW_window_show(RGFW_window* win) {RGFW_UNUSED(win); }
RGFW_bool RGFW_window_isHidden(RGFW_window* win) { RGFW_UNUSED(win); return RGFW_FALSE; }
RGFW_bool RGFW_window_isMinimized(RGFW_window* win) { RGFW_UNUSED(win); return RGFW_FALSE; }
RGFW_bool RGFW_window_isMaximized(RGFW_window* win) { RGFW_UNUSED(win); return RGFW_FALSE; }
RGFW_bool RGFW_window_isFloating(RGFW_window* win) { RGFW_UNUSED(win); return RGFW_FALSE; }
RGFW_monitor RGFW_window_getMonitor(RGFW_window* win) { RGFW_UNUSED(win); return (RGFW_monitor){}; }
void RGFW_waitForEvent(i32 waitMS) { RGFW_UNUSED(waitMS); }
#endif

/* end of web asm defines */

/*
	* RGFW function pointer backend, made to allow you to compile for Wayland but fallback to X11
*/
#ifdef RGFW_DYNAMIC
typedef RGFW_window* (*RGFW_createWindowPlatform_ptr)(const char* name, RGFW_windowFlags flags, RGFW_window* win);
typedef RGFW_bool (*RGFW_getMouse_ptr)(i32* x, i32* y);
typedef u8 (*RGFW_rgfwToKeyChar_ptr)(u32 key);
typedef void (*RGFW_pollEvents_ptr)(void);
typedef void (*RGFW_window_move_ptr)(RGFW_window* win, i32 x, i32 y);
typedef void (*RGFW_window_resize_ptr)(RGFW_window* win, i32 w, i32 h);
typedef void (*RGFW_window_setAspectRatio_ptr)(RGFW_window* win, i32 w, i32 h);
typedef void (*RGFW_window_setMinSize_ptr)(RGFW_window* win, i32 w, i32 h);
typedef void (*RGFW_window_setMaxSize_ptr)(RGFW_window* win, i32 w, i32 h);
typedef void (*RGFW_window_maximize_ptr)(RGFW_window* win);
typedef void (*RGFW_window_focus_ptr)(RGFW_window* win);
typedef void (*RGFW_window_raise_ptr)(RGFW_window* win);
typedef void (*RGFW_window_setFullscreen_ptr)(RGFW_window* win, RGFW_bool fullscreen);
typedef void (*RGFW_window_setFloating_ptr)(RGFW_window* win, RGFW_bool floating);
typedef void (*RGFW_window_setOpacity_ptr)(RGFW_window* win, u8 opacity);
typedef void (*RGFW_window_minimize_ptr)(RGFW_window* win);
typedef void (*RGFW_window_restore_ptr)(RGFW_window* win);
typedef RGFW_bool (*RGFW_window_isFloating_ptr)(RGFW_window* win);
typedef void (*RGFW_window_setName_ptr)(RGFW_window* win, const char* name);
typedef void (*RGFW_window_setMousePassthrough_ptr)(RGFW_window* win, RGFW_bool passthrough);
typedef RGFW_bool (*RGFW_window_setIconEx_ptr)(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format, u8 type);
typedef RGFW_mouse* (*RGFW_loadMouse_ptr)(u8* data, i32 w, i32 h, RGFW_format format);
typedef void (*RGFW_window_setMouse_ptr)(RGFW_window* win, RGFW_mouse* mouse);
typedef void (*RGFW_window_moveMouse_ptr)(RGFW_window* win, i32 x, i32 y);
typedef RGFW_bool (*RGFW_window_setMouseDefault_ptr)(RGFW_window* win);
typedef RGFW_bool (*RGFW_window_setMouseStandard_ptr)(RGFW_window* win, u8 mouse);
typedef void (*RGFW_window_hide_ptr)(RGFW_window* win);
typedef void (*RGFW_window_show_ptr)(RGFW_window* win);
typedef RGFW_ssize_t (*RGFW_readClipboardPtr_ptr)(char* str, size_t strCapacity);
typedef void (*RGFW_writeClipboard_ptr)(const char* text, u32 textLen);
typedef RGFW_bool (*RGFW_window_isHidden_ptr)(RGFW_window* win);
typedef RGFW_bool (*RGFW_window_isMinimized_ptr)(RGFW_window* win);
typedef RGFW_bool (*RGFW_window_isMaximized_ptr)(RGFW_window* win);
typedef RGFW_monitor* (*RGFW_getMonitors_ptr)(size_t* len);
typedef RGFW_monitor (*RGFW_getPrimaryMonitor_ptr)(void);
typedef RGFW_bool (*RGFW_monitor_requestMode_ptr)(RGFW_monitor mon, RGFW_monitorMode mode, RGFW_modeRequest request);
typedef RGFW_monitor (*RGFW_window_getMonitor_ptr)(RGFW_window* win);
typedef void (*RGFW_window_closePlatform_ptr)(RGFW_window* win);
typedef RGFW_bool (*RGFW_createSurfacePtr_ptr)(u8* data, i32 w, i32 h, RGFW_format format, RGFW_surface* surface);
typedef void (*RGFW_window_blitSurface_ptr)(RGFW_window* win, RGFW_surface* surface);
typedef void (*RGFW_surface_freePtr_ptr)(RGFW_surface* surface);
typedef void (*RGFW_freeMouse_ptr)(RGFW_mouse* mouse);
typedef void (*RGFW_window_setBorder_ptr)(RGFW_window* win, RGFW_bool border);
typedef void (*RGFW_releaseCursor_ptr)(RGFW_window* win);
typedef void (*RGFW_captureCursor_ptr)(RGFW_window* win);
#ifdef RGFW_OPENGL
typedef void (*RGFW_window_makeCurrentContext_OpenGL_ptr)(RGFW_window* win);
typedef void* (*RGFW_getCurrentContext_OpenGL_ptr)(void);
typedef void (*RGFW_window_swapBuffers_OpenGL_ptr)(RGFW_window* win);
typedef void (*RGFW_window_swapInterval_OpenGL_ptr)(RGFW_window* win, i32 swapInterval);
typedef RGFW_bool (*RGFW_extensionSupportedPlatform_OpenGL_ptr)(const char* extension, size_t len);
typedef RGFW_proc (*RGFW_getProcAddress_OpenGL_ptr)(const char* procname);
typedef RGFW_bool (*RGFW_window_createContextPtr_OpenGL_ptr)(RGFW_window* win, RGFW_glContext* ctx, RGFW_glHints* hints);
typedef void (*RGFW_window_deleteContextPtr_OpenGL_ptr)(RGFW_window* win, RGFW_glContext* ctx);
#endif
#ifdef RGFW_WEBGPU
typedef WGPUSurface (*RGFW_window_createSurface_WebGPU_ptr)(RGFW_window* window, WGPUInstance instance);
#endif

/* Structure to hold all function pointers */
typedef struct RGFW_FunctionPointers {
   RGFW_createSurfacePtr_ptr createSurfacePtr;
    RGFW_window_blitSurface_ptr window_blitSurface;
	RGFW_surface_freePtr_ptr surface_freePtr;
	RGFW_freeMouse_ptr freeMouse;
    RGFW_window_setBorder_ptr window_setBorder;
    RGFW_releaseCursor_ptr releaseCursor;
    RGFW_captureCursor_ptr captureCursor;
    RGFW_createWindowPlatform_ptr createWindowPlatform;
    RGFW_getMouse_ptr getGlobalMouse;
    RGFW_rgfwToKeyChar_ptr rgfwToKeyChar;
    RGFW_pollEvents_ptr pollEvents;
    RGFW_window_move_ptr window_move;
    RGFW_window_resize_ptr window_resize;
    RGFW_window_setAspectRatio_ptr window_setAspectRatio;
    RGFW_window_setMinSize_ptr window_setMinSize;
    RGFW_window_setMaxSize_ptr window_setMaxSize;
    RGFW_window_maximize_ptr window_maximize;
    RGFW_window_focus_ptr window_focus;
    RGFW_window_raise_ptr window_raise;
    RGFW_window_setFullscreen_ptr window_setFullscreen;
    RGFW_window_setFloating_ptr window_setFloating;
    RGFW_window_setOpacity_ptr window_setOpacity;
    RGFW_window_minimize_ptr window_minimize;
    RGFW_window_restore_ptr window_restore;
    RGFW_window_isFloating_ptr window_isFloating;
    RGFW_window_setName_ptr window_setName;
    RGFW_window_setMousePassthrough_ptr window_setMousePassthrough;
    RGFW_window_setIconEx_ptr window_setIconEx;
    RGFW_loadMouse_ptr loadMouse;
    RGFW_window_setMouse_ptr window_setMouse;
    RGFW_window_moveMouse_ptr window_moveMouse;
    RGFW_window_setMouseDefault_ptr window_setMouseDefault;
    RGFW_window_setMouseStandard_ptr window_setMouseStandard;
    RGFW_window_hide_ptr window_hide;
    RGFW_window_show_ptr window_show;
    RGFW_readClipboardPtr_ptr readClipboardPtr;
    RGFW_writeClipboard_ptr writeClipboard;
    RGFW_window_isHidden_ptr window_isHidden;
    RGFW_window_isMinimized_ptr window_isMinimized;
    RGFW_window_isMaximized_ptr window_isMaximized;
    RGFW_getMonitors_ptr getMonitors;
    RGFW_getPrimaryMonitor_ptr getPrimaryMonitor;
    RGFW_monitor_requestMode_ptr monitor_requestMode;
    RGFW_window_getMonitor_ptr window_getMonitor;
    RGFW_window_closePlatform_ptr window_closePlatform;
#ifdef RGFW_OPENGL
    RGFW_extensionSupportedPlatform_OpenGL_ptr extensionSupportedPlatform_OpenGL;
    RGFW_getProcAddress_OpenGL_ptr getProcAddress_OpenGL;
    RGFW_window_createContextPtr_OpenGL_ptr window_createContextPtr_OpenGL;
    RGFW_window_deleteContextPtr_OpenGL_ptr window_deleteContextPtr_OpenGL;
    RGFW_window_makeCurrentContext_OpenGL_ptr window_makeCurrentContext_OpenGL;
    RGFW_getCurrentContext_OpenGL_ptr getCurrentContext_OpenGL;
    RGFW_window_swapBuffers_OpenGL_ptr window_swapBuffers_OpenGL;
    RGFW_window_swapInterval_OpenGL_ptr window_swapInterval_OpenGL;
#endif
#ifdef RGFW_WEBGPU
    RGFW_window_createSurface_WebGPU_ptr window_createSurface_WebGPU;
#endif
} RGFW_functionPointers;

RGFW_functionPointers RGFW_api;

RGFW_bool RGFW_createSurfacePtr(u8* data, i32 w, i32 h, RGFW_format format, RGFW_surface* surface) { return RGFW_api.createSurfacePtr(data, w, h, format, surface); }
void RGFW_surface_freePtr(RGFW_surface* surface) { RGFW_api.surface_freePtr(surface); }
void RGFW_freeMouse(RGFW_mouse* mouse) { RGFW_api.freeMouse(mouse); }
void RGFW_window_blitSurface(RGFW_window* win, RGFW_surface* surface) { RGFW_api.window_blitSurface(win, surface); }
void RGFW_window_setBorder(RGFW_window* win, RGFW_bool border) { RGFW_api.window_setBorder(win, border); }
void RGFW_releaseCursor(RGFW_window* win) { RGFW_api.releaseCursor(win); }
void RGFW_captureCursor(RGFW_window* win) { RGFW_api.captureCursor(win); }
RGFW_window* RGFW_createWindowPlatform(const char* name, RGFW_windowFlags flags, RGFW_window* win) { RGFW_init(); return RGFW_api.createWindowPlatform(name, flags, win); }
RGFW_bool RGFW_getGlobalMouse(i32* x, i32* y) { return RGFW_api.getGlobalMouse(x, y); }
u8 RGFW_rgfwToKeyChar(u32 key) { return RGFW_api.rgfwToKeyChar(key); }
void RGFW_pollEvents(void) { RGFW_api.pollEvents(); }
void RGFW_window_move(RGFW_window* win, i32 x, i32 y) { RGFW_api.window_move(win, x, y); }
void RGFW_window_resize(RGFW_window* win, i32 w, i32 h) { RGFW_api.window_resize(win, w, h); }
void RGFW_window_setAspectRatio(RGFW_window* win, i32 w, i32 h) { RGFW_api.window_setAspectRatio(win, w, h); }
void RGFW_window_setMinSize(RGFW_window* win, i32 w, i32 h) { RGFW_api.window_setMinSize(win, w, h); }
void RGFW_window_setMaxSize(RGFW_window* win, i32 w, i32 h) { RGFW_api.window_setMaxSize(win, w, h); }
void RGFW_window_maximize(RGFW_window* win) { RGFW_api.window_maximize(win); }
void RGFW_window_focus(RGFW_window* win) { RGFW_api.window_focus(win); }
void RGFW_window_raise(RGFW_window* win) { RGFW_api.window_raise(win); }
void RGFW_window_setFullscreen(RGFW_window* win, RGFW_bool fullscreen) { RGFW_api.window_setFullscreen(win, fullscreen); }
void RGFW_window_setFloating(RGFW_window* win, RGFW_bool floating) { RGFW_api.window_setFloating(win, floating); }
void RGFW_window_setOpacity(RGFW_window* win, u8 opacity) { RGFW_api.window_setOpacity(win, opacity); }
void RGFW_window_minimize(RGFW_window* win) { RGFW_api.window_minimize(win); }
void RGFW_window_restore(RGFW_window* win) { RGFW_api.window_restore(win); }
RGFW_bool RGFW_window_isFloating(RGFW_window* win) { return RGFW_api.window_isFloating(win); }
void RGFW_window_setName(RGFW_window* win, const char* name) { RGFW_api.window_setName(win, name); }

#ifndef RGFW_NO_PASSTHROUGH
void RGFW_window_setMousePassthrough(RGFW_window* win, RGFW_bool passthrough) { RGFW_api.window_setMousePassthrough(win, passthrough); }
#endif

RGFW_bool RGFW_window_setIconEx(RGFW_window* win, u8* data, i32 w, i32 h, RGFW_format format, u8 type) { return RGFW_api.window_setIconEx(win, data, w, h, format, type); }
RGFW_mouse* RGFW_loadMouse(u8* data, i32 w, i32 h, RGFW_format format) { return RGFW_api.loadMouse(data, w, h, format); }
void RGFW_window_setMouse(RGFW_window* win, RGFW_mouse* mouse) { RGFW_api.window_setMouse(win, mouse); }
void RGFW_window_moveMouse(RGFW_window* win, i32 x, i32 y) { RGFW_api.window_moveMouse(win, x, y); }
RGFW_bool RGFW_window_setMouseDefault(RGFW_window* win) { return RGFW_api.window_setMouseDefault(win); }
RGFW_bool RGFW_window_setMouseStandard(RGFW_window* win, u8 mouse) { return RGFW_api.window_setMouseStandard(win, mouse); }
void RGFW_window_hide(RGFW_window* win) { RGFW_api.window_hide(win); }
void RGFW_window_show(RGFW_window* win) { RGFW_api.window_show(win); }
RGFW_ssize_t RGFW_readClipboardPtr(char* str, size_t strCapacity) { return RGFW_api.readClipboardPtr(str, strCapacity); }
void RGFW_writeClipboard(const char* text, u32 textLen) { RGFW_api.writeClipboard(text, textLen); }
RGFW_bool RGFW_window_isHidden(RGFW_window* win) { return RGFW_api.window_isHidden(win); }
RGFW_bool RGFW_window_isMinimized(RGFW_window* win) { return RGFW_api.window_isMinimized(win); }
RGFW_bool RGFW_window_isMaximized(RGFW_window* win) { return RGFW_api.window_isMaximized(win); }
RGFW_monitor* RGFW_getMonitors(size_t* len) { return RGFW_api.getMonitors(len); }
RGFW_monitor RGFW_getPrimaryMonitor(void) { return RGFW_api.getPrimaryMonitor(); }
RGFW_bool RGFW_monitor_requestMode(RGFW_monitor mon, RGFW_monitorMode mode, RGFW_modeRequest request) { return RGFW_api.monitor_requestMode(mon, mode, request); }
RGFW_monitor RGFW_window_getMonitor(RGFW_window* win) { return RGFW_api.window_getMonitor(win); }
void RGFW_window_closePlatform(RGFW_window* win) { RGFW_api.window_closePlatform(win); }

#ifdef RGFW_OPENGL
RGFW_bool RGFW_extensionSupportedPlatform_OpenGL(const char* extension, size_t len) { return RGFW_api.extensionSupportedPlatform_OpenGL(extension, len); }
RGFW_proc RGFW_getProcAddress_OpenGL(const char* procname) { return RGFW_api.getProcAddress_OpenGL(procname); }
RGFW_bool RGFW_window_createContextPtr_OpenGL(RGFW_window* win, RGFW_glContext* ctx, RGFW_glHints* hints) { return RGFW_api.window_createContextPtr_OpenGL(win, ctx, hints); }
void RGFW_window_deleteContextPtr_OpenGL(RGFW_window* win, RGFW_glContext* ctx) { RGFW_api.window_deleteContextPtr_OpenGL(win, ctx); }
void RGFW_window_makeCurrentContext_OpenGL(RGFW_window* win) { RGFW_api.window_makeCurrentContext_OpenGL(win); }
void* RGFW_getCurrentContext_OpenGL(void) { return RGFW_api.getCurrentContext_OpenGL(); }
void RGFW_window_swapBuffers_OpenGL(RGFW_window* win) { RGFW_api.window_swapBuffers_OpenGL(win); }
void RGFW_window_swapInterval_OpenGL(RGFW_window* win, i32 swapInterval) { RGFW_api.window_swapInterval_OpenGL(win, swapInterval); }
#endif

#ifdef RGFW_WEBGPU
WGPUSurface RGFW_window_createSurface_WebGPU(RGFW_window* window, WGPUInstance instance) { return RGFW_api.window_createSurface_WebGPU(window, instance); }
#endif
#endif  /* RGFW_DYNAMIC */

/*
 * start of X11 AND wayland defines
 * this allows a single executable to support x11 AND wayland
 * falling back to x11 if wayland fails to initalize
*/
#if defined(RGFW_WAYLAND) && defined(RGFW_X11)
void RGFW_load_X11(void) {
    RGFW_api.createSurfacePtr = RGFW_createSurfacePtr_X11;
    RGFW_api.window_blitSurface = RGFW_window_blitSurface_X11;
	RGFW_api.surface_freePtr = RGFW_surface_freePtr_X11;
	RGFW_api.freeMouse = RGFW_freeMouse_X11;
    RGFW_api.window_setBorder = RGFW_window_setBorder_X11;
    RGFW_api.releaseCursor = RGFW_releaseCursor_X11;
    RGFW_api.captureCursor = RGFW_captureCursor_X11;
	RGFW_api.createWindowPlatform = RGFW_createWindowPlatform_X11;
    RGFW_api.getGlobalMouse = RGFW_getGlobalMouse_X11;
    RGFW_api.rgfwToKeyChar = RGFW_rgfwToKeyChar_X11;
    RGFW_api.pollEvents = RGFW_pollEvents_X11;
    RGFW_api.window_move = RGFW_window_move_X11;
    RGFW_api.window_resize = RGFW_window_resize_X11;
    RGFW_api.window_setAspectRatio = RGFW_window_setAspectRatio_X11;
    RGFW_api.window_setMinSize = RGFW_window_setMinSize_X11;
    RGFW_api.window_setMaxSize = RGFW_window_setMaxSize_X11;
    RGFW_api.window_maximize = RGFW_window_maximize_X11;
    RGFW_api.window_focus = RGFW_window_focus_X11;
    RGFW_api.window_raise = RGFW_window_raise_X11;
    RGFW_api.window_setFullscreen = RGFW_window_setFullscreen_X11;
    RGFW_api.window_setFloating = RGFW_window_setFloating_X11;
    RGFW_api.window_setOpacity = RGFW_window_setOpacity_X11;
    RGFW_api.window_minimize = RGFW_window_minimize_X11;
    RGFW_api.window_restore = RGFW_window_restore_X11;
    RGFW_api.window_isFloating = RGFW_window_isFloating_X11;
    RGFW_api.window_setName = RGFW_window_setName_X11;
#ifndef RGFW_NO_PASSTHROUGH
    RGFW_api.window_setMousePassthrough = RGFW_window_setMousePassthrough_X11;
#endif
    RGFW_api.window_setIconEx = RGFW_window_setIconEx_X11;
    RGFW_api.loadMouse = RGFW_loadMouse_X11;
    RGFW_api.window_setMouse = RGFW_window_setMouse_X11;
    RGFW_api.window_moveMouse = RGFW_window_moveMouse_X11;
    RGFW_api.window_setMouseDefault = RGFW_window_setMouseDefault_X11;
    RGFW_api.window_setMouseStandard = RGFW_window_setMouseStandard_X11;
    RGFW_api.window_hide = RGFW_window_hide_X11;
    RGFW_api.window_show = RGFW_window_show_X11;
    RGFW_api.readClipboardPtr = RGFW_readClipboardPtr_X11;
    RGFW_api.writeClipboard = RGFW_writeClipboard_X11;
    RGFW_api.window_isHidden = RGFW_window_isHidden_X11;
    RGFW_api.window_isMinimized = RGFW_window_isMinimized_X11;
    RGFW_api.window_isMaximized = RGFW_window_isMaximized_X11;
    RGFW_api.getMonitors = RGFW_getMonitors_X11;
    RGFW_api.getPrimaryMonitor = RGFW_getPrimaryMonitor_X11;
    RGFW_api.monitor_requestMode = RGFW_monitor_requestMode_X11;
    RGFW_api.window_getMonitor = RGFW_window_getMonitor_X11;
    RGFW_api.window_closePlatform = RGFW_window_closePlatform_X11;
#ifdef RGFW_OPENGL
    RGFW_api.extensionSupportedPlatform_OpenGL = RGFW_extensionSupportedPlatform_OpenGL_X11;
    RGFW_api.getProcAddress_OpenGL = RGFW_getProcAddress_OpenGL_X11;
	RGFW_api.window_createContextPtr_OpenGL = RGFW_window_createContextPtr_OpenGL_X11;
    RGFW_api.window_deleteContextPtr_OpenGL = RGFW_window_deleteContextPtr_OpenGL_X11;
	RGFW_api.window_makeCurrentContext_OpenGL = RGFW_window_makeCurrentContext_OpenGL_X11;
    RGFW_api.getCurrentContext_OpenGL = RGFW_getCurrentContext_OpenGL_X11;
    RGFW_api.window_swapBuffers_OpenGL = RGFW_window_swapBuffers_OpenGL_X11;
    RGFW_api.window_swapInterval_OpenGL = RGFW_window_swapInterval_OpenGL_X11;
#endif
#ifdef RGFW_WEBGPU
    RGFW_api.window_createSurface_WebGPU = RGFW_window_createSurface_WebGPU_X11;
#endif
}

void RGFW_load_Wayland(void) {
	RGFW_api.createSurfacePtr = RGFW_createSurfacePtr_Wayland;
	RGFW_api.window_blitSurface = RGFW_window_blitSurface_Wayland;
    RGFW_api.surface_freePtr = RGFW_surface_freePtr_Wayland;
	RGFW_api.freeMouse = RGFW_freeMouse_Wayland;
	RGFW_api.window_setBorder = RGFW_window_setBorder_Wayland;
    RGFW_api.releaseCursor = RGFW_releaseCursor_Wayland;
    RGFW_api.captureCursor = RGFW_captureCursor_Wayland;
    RGFW_api.createWindowPlatform = RGFW_createWindowPlatform_Wayland;
    RGFW_api.getGlobalMouse = RGFW_getGlobalMouse_Wayland;
    RGFW_api.rgfwToKeyChar = RGFW_rgfwToKeyChar_Wayland;
    RGFW_api.pollEvents = RGFW_pollEvents_Wayland;
    RGFW_api.window_move = RGFW_window_move_Wayland;
    RGFW_api.window_resize = RGFW_window_resize_Wayland;
    RGFW_api.window_setAspectRatio = RGFW_window_setAspectRatio_Wayland;
    RGFW_api.window_setMinSize = RGFW_window_setMinSize_Wayland;
    RGFW_api.window_setMaxSize = RGFW_window_setMaxSize_Wayland;
    RGFW_api.window_maximize = RGFW_window_maximize_Wayland;
    RGFW_api.window_focus = RGFW_window_focus_Wayland;
    RGFW_api.window_raise = RGFW_window_raise_Wayland;
    RGFW_api.window_setFullscreen = RGFW_window_setFullscreen_Wayland;
    RGFW_api.window_setFloating = RGFW_window_setFloating_Wayland;
    RGFW_api.window_setOpacity = RGFW_window_setOpacity_Wayland;
    RGFW_api.window_minimize = RGFW_window_minimize_Wayland;
    RGFW_api.window_restore = RGFW_window_restore_Wayland;
    RGFW_api.window_isFloating = RGFW_window_isFloating_Wayland;
    RGFW_api.window_setName = RGFW_window_setName_Wayland;
#ifndef RGFW_NO_PASSTHROUGH
    RGFW_api.window_setMousePassthrough = RGFW_window_setMousePassthrough_Wayland;
#endif
    RGFW_api.window_setIconEx = RGFW_window_setIconEx_Wayland;
	RGFW_api.loadMouse = RGFW_loadMouse_Wayland;
    RGFW_api.window_setMouse = RGFW_window_setMouse_Wayland;
    RGFW_api.window_moveMouse = RGFW_window_moveMouse_Wayland;
    RGFW_api.window_setMouseDefault = RGFW_window_setMouseDefault_Wayland;
    RGFW_api.window_setMouseStandard = RGFW_window_setMouseStandard_Wayland;
    RGFW_api.window_hide = RGFW_window_hide_Wayland;
    RGFW_api.window_show = RGFW_window_show_Wayland;
    RGFW_api.readClipboardPtr = RGFW_readClipboardPtr_Wayland;
    RGFW_api.writeClipboard = RGFW_writeClipboard_Wayland;
    RGFW_api.window_isHidden = RGFW_window_isHidden_Wayland;
    RGFW_api.window_isMinimized = RGFW_window_isMinimized_Wayland;
    RGFW_api.window_isMaximized = RGFW_window_isMaximized_Wayland;
    RGFW_api.getMonitors = RGFW_getMonitors_Wayland;
    RGFW_api.getPrimaryMonitor = RGFW_getPrimaryMonitor_Wayland;
    RGFW_api.monitor_requestMode = RGFW_monitor_requestMode_Wayland;
    RGFW_api.window_getMonitor = RGFW_window_getMonitor_Wayland;
    RGFW_api.window_closePlatform = RGFW_window_closePlatform_Wayland;
#ifdef RGFW_OPENGL
    RGFW_api.extensionSupportedPlatform_OpenGL = RGFW_extensionSupportedPlatform_OpenGL_Wayland;
    RGFW_api.getProcAddress_OpenGL = RGFW_getProcAddress_OpenGL_Wayland;
	RGFW_api.window_createContextPtr_OpenGL = RGFW_window_createContextPtr_OpenGL_Wayland;
    RGFW_api.window_deleteContextPtr_OpenGL = RGFW_window_deleteContextPtr_OpenGL_Wayland;
	RGFW_api.window_makeCurrentContext_OpenGL = RGFW_window_makeCurrentContext_OpenGL_Wayland;
    RGFW_api.getCurrentContext_OpenGL = RGFW_getCurrentContext_OpenGL_Wayland;
    RGFW_api.window_swapBuffers_OpenGL = RGFW_window_swapBuffers_OpenGL_Wayland;
    RGFW_api.window_swapInterval_OpenGL = RGFW_window_swapInterval_OpenGL_Wayland;
#endif
#ifdef RGFW_WEBGPU
    RGFW_api.window_createSurface_WebGPU = RGFW_window_createSurface_WebGPU_Wayland;
#endif
}
#endif /* wayland AND x11 */
/* end of X11 AND wayland defines */

#endif /* RGFW_IMPLEMENTATION */

#if defined(__cplusplus) && !defined(__EMSCRIPTEN__)
}
#endif

#if _MSC_VER
	#pragma warning( pop )
#endif

