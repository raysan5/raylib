/*
*
*	RGFW 1.7.5-dev

* Copyright (C) 2022-25 ColleagueRiley
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
	#define RGFW_OSMESA - (optional) use OSmesa as backend (instead of system's opengl api + regular opengl)
	#define RGFW_BUFFER - (optional) draw directly to (RGFW) window pixel buffer that is drawn to screen (the buffer is in the RGBA format)
	#define RGFW_EGL - (optional) use EGL for loading an OpenGL context (instead of the system's opengl api)
	#define RGFW_OPENGL_ES1 - (optional) use EGL to load and use Opengl ES (version 1) for backend rendering (instead of the system's opengl api)
									This version doesn't work for desktops (I'm pretty sure)
	#define RGFW_OPENGL_ES2 - (optional) use OpenGL ES (version 2)
	#define RGFW_OPENGL_ES3 - (optional) use OpenGL ES (version 3)
	#define RGFW_DIRECTX - (optional) include integration directX functions (windows only)
	#define RGFW_VULKAN - (optional) include helpful vulkan integration functions and macros
	#define RGFW_WEBGPU - (optional) use webGPU for rendering (Web ONLY)
	#define RGFW_NO_API - (optional) don't use any rendering API (no opengl, no vulkan, no directX)

	#define RGFW_LINK_EGL (optional) (windows only) if EGL is being used, if EGL functions should be defined dymanically (using GetProcAddress)
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
	#define RGFW_BUFFER_BGR - use the BGR format for bufffers instead of RGB, saves processing time
    #define RGFW_ADVANCED_SMOOTH_RESIZE - use advanced methods for smooth resizing (may result in a spike in memory usage or worse performance) (eg. WM_TIMER and XSyncValue)

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
	RGFW_window* win = RGFW_createWindow("name", RGFW_RECT(100, 100, 500, 500), (u64)0);

	RGFW_window_setIcon(win, icon, RGFW_AREA(3, 3), 4);

	while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
		while (RGFW_window_checkEvent(win)) {
		    if (win->event.type == RGFW_quit || RGFW_isPressed(win, RGFW_escape))
			    break;
        }

		RGFW_window_swapBuffers(win);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
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
		EimaMei/Sacode : Much of the code for creating windows using winapi, Wrote the Silicon library, helped with MacOS Support, siliapp.h -> referencing

		stb - This project is heavily inspired by the stb single header files

		GLFW:
			certain parts of winapi and X11 are very poorly documented,
			GLFW's source code was referenced and used throughout the project.

		contributors : (feel free to put yourself here if you contribute)
		krisvers -> code review
		EimaMei (SaCode) -> code review
		Code-Nycticebus -> bug fixes
		Rob Rohan -> X11 bugs and missing features, MacOS/Cocoa fixing memory issues/bugs
		AICDG (@THISISAGOODNAME) -> vulkan support (example)
		@Easymode -> support, testing/debugging, bug fixes and reviews
		Joshua Rowe (omnisci3nce) - bug fix, review (macOS)
		@lesleyrs -> bug fix, review (OpenGL)
        Nick Porcino (meshula) - testing, organization, review (MacOS, examples)
		@DarekParodia -> code review (X11) (C++)
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

#ifndef RGFW_USERPTR
	#define RGFW_USERPTR NULL
#endif

#ifndef RGFW_UNUSED
	#define RGFW_UNUSED(x) (void)(x)
#endif

#ifndef RGFW_ROUND
	#define RGFW_ROUND(x) (i32)((x) >= 0 ? (x) + 0.5f : (x) - 0.5f)
#endif

#ifndef RGFW_ALLOC
	#include <stdlib.h>
	#define RGFW_ALLOC malloc
	#define RGFW_FREE free
#endif

#ifndef RGFW_ASSERT
	#include <assert.h>
	#define RGFW_ASSERT assert
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

#ifndef RGFW_ENUM
	#define RGFW_ENUM(type, name) type name; enum
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

#ifndef RGFW_BOOL_DEFINED
    #define RGFW_BOOL_DEFINED
    typedef u8 RGFW_bool;
#endif

#define RGFW_BOOL(x) (RGFW_bool)((x) ? RGFW_TRUE : RGFW_FALSE) /* force an value to be 0 or 1 */
#define RGFW_TRUE (RGFW_bool)1
#define RGFW_FALSE (RGFW_bool)0

/* these OS macros look better & are standardized */
/* plus it helps with cross-compiling */

#ifdef __EMSCRIPTEN__
	#define RGFW_WASM

	#if !defined(RGFW_NO_API) && !defined(RGFW_WEBGPU)
		#define RGFW_OPENGL
	#endif

	#ifdef RGFW_EGL
		#undef RGFW_EGL
	#endif

	#include <emscripten/html5.h>
	#include <emscripten/key_codes.h>

	#ifdef RGFW_WEBGPU
		#include <emscripten/html5_webgpu.h>
	#endif
#endif

#if defined(RGFW_X11) && defined(__APPLE__) && !defined(RGFW_CUSTOM_BACKEND)
	#define RGFW_MACOS_X11
	#define RGFW_UNIX
	#undef __APPLE__
#endif

#if defined(_WIN32) && !defined(RGFW_X11) && !defined(RGFW_UNIX) && !defined(RGFW_WASM) && !defined(RGFW_CUSTOM_BACKEND) /* (if you're using X11 on windows some how) */
	#define RGFW_WINDOWS
	/* make sure the correct architecture is defined */
	#if defined(_WIN64)
		#define _AMD64_
		#undef _X86_
	#else
		#undef _AMD64_
		#ifndef _X86_
			#define _X86_
		#endif
	#endif

	#ifndef RGFW_NO_XINPUT
		#ifdef __MINGW32__ /* try to find the right header */
			#include <xinput.h>
		#else
			#include <XInput.h>
		#endif
	#endif
#endif
#if defined(RGFW_WAYLAND)
	#define RGFW_DEBUG /* wayland will be in debug mode by default for now */
    #if !defined(RGFW_NO_API) && (!defined(RGFW_BUFFER) || defined(RGFW_OPENGL)) && !defined(RGFW_OSMESA)
		#define RGFW_EGL
		#define RGFW_OPENGL
		#include <wayland-egl.h>
	#endif

	#define RGFW_UNIX
	#include <wayland-client.h>
#endif
#if !defined(RGFW_NO_X11) && (defined(__unix__) || defined(RGFW_MACOS_X11) || defined(RGFW_X11))  && !defined(RGFW_WASM)  && !defined(RGFW_CUSTOM_BACKEND)
	#define RGFW_MACOS_X11
	#define RGFW_X11
	#define RGFW_UNIX
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
#elif defined(__APPLE__) && !defined(RGFW_MACOS_X11) && !defined(RGFW_X11)  && !defined(RGFW_WASM)  && !defined(RGFW_CUSTOM_BACKEND)
	#define RGFW_MACOS
	#if !defined(RGFW_BUFFER_BGR)
		#define RGFW_BUFFER_BGR
	#else
		#undef RGFW_BUFFER_BGR
	#endif
#endif

#if (defined(RGFW_OPENGL_ES1) || defined(RGFW_OPENGL_ES2) || defined(RGFW_OPENGL_ES3)) && !defined(RGFW_EGL)
	#define RGFW_EGL
#endif

#if !defined(RGFW_OSMESA) && !defined(RGFW_EGL) && !defined(RGFW_OPENGL) && !defined(RGFW_DIRECTX) && !defined(RGFW_BUFFER) && !defined(RGFW_NO_API)
	#define RGFW_OPENGL
#endif

#ifdef RGFW_EGL
	#include <EGL/egl.h>
#elif defined(RGFW_OSMESA)
	#ifdef RGFW_WINDOWS
	#define OEMRESOURCE
	#include <GL/gl.h>
    	#ifndef GLAPIENTRY
	#define GLAPIENTRY APIENTRY
	#endif
    	#ifndef GLAPI
	#define GLAPI WINGDIAPI
    	#endif
	#endif

	#ifndef __APPLE__
		#include <GL/osmesa.h>
	#else
		#include <OpenGL/osmesa.h>
	#endif
#endif

#if (defined(RGFW_OPENGL) || defined(RGFW_WEGL)) && defined(_MSC_VER)
    #pragma comment(lib, "opengl32")
#endif

#if defined(RGFW_OPENGL) && defined(RGFW_X11)
	#ifndef GLX_MESA_swap_control
		#define  GLX_MESA_swap_control
	#endif
	#include <GL/glx.h> /* GLX defs, xlib.h, gl.h */
#endif

#define RGFW_COCOA_FRAME_NAME NULL

/*! (unix) Toggle use of wayland. This will be on by default if you use `RGFW_WAYLAND` (if you don't use RGFW_WAYLAND, you don't expose WAYLAND functions)
	this is mostly used to allow you to force the use of XWayland
*/
RGFWDEF void RGFW_useWayland(RGFW_bool wayland);
RGFWDEF RGFW_bool RGFW_usingWayland(void);
/*
	regular RGFW stuff
*/

#define RGFW_key u8

typedef RGFW_ENUM(u8, RGFW_eventType) {
	/*! event codes */
	RGFW_eventNone = 0, /*!< no event has been sent */
 	RGFW_keyPressed, /* a key has been pressed */
	RGFW_keyReleased, /*!< a key has been released */
	/*! key event note
		the code of the key pressed is stored in
		RGFW_event.key
		!!Keycodes defined at the bottom of the RGFW_HEADER part of this file!!

		while a string version is stored in
		RGFW_event.KeyString

		RGFW_event.keyMod holds the current keyMod
		this means if CapsLock, NumLock are active or not
	*/
	RGFW_mouseButtonPressed, /*!< a mouse button has been pressed (left,middle,right) */
	RGFW_mouseButtonReleased, /*!< a mouse button has been released (left,middle,right) */
	RGFW_mousePosChanged, /*!< the position of the mouse has been changed */
	/*! mouse event note
		the x and y of the mouse can be found in the vector, RGFW_event.point

		RGFW_event.button holds which mouse button was pressed
	*/
	RGFW_gamepadConnected, /*!< a gamepad was connected */
	RGFW_gamepadDisconnected, /*!< a gamepad was disconnected */
	RGFW_gamepadButtonPressed, /*!< a gamepad button was pressed */
	RGFW_gamepadButtonReleased, /*!< a gamepad button was released */
	RGFW_gamepadAxisMove, /*!< an axis of a gamepad was moved */
	/*! gamepad event note
		RGFW_event.gamepad holds which gamepad was altered, if any
		RGFW_event.button holds which gamepad button was pressed

		RGFW_event.axis holds the data of all the axises
		RGFW_event.axisesCount says how many axises there are
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
		with win->r.x, win->r.y, win->r.w and win->r.h
	*/
	RGFW_quit, /*!< the user clicked the quit button */
	RGFW_DND, /*!< a file has been dropped into the window */
	RGFW_DNDInit, /*!< the start of a dnd event, when the place where the file drop is known */
	/* dnd data note
		The x and y coords of the drop are stored in the vector RGFW_event.point

		RGFW_event.droppedFilesCount holds how many files were dropped

		This is also the size of the array which stores all the dropped file string,
		RGFW_event.droppedFiles
	*/
	RGFW_windowMaximized, /*!< the window was maximized */
	RGFW_windowMinimized, /*!< the window was minimized */
	RGFW_windowRestored, /*!< the window was restored */
	RGFW_scaleUpdated /*!< content scale factor changed */
};

/*! mouse button codes (RGFW_event.button) */
typedef RGFW_ENUM(u8, RGFW_mouseButton) {
	RGFW_mouseLeft = 0, /*!< left mouse button is pressed */
	RGFW_mouseMiddle, /*!< mouse-wheel-button is pressed */
	RGFW_mouseRight, /*!< right mouse button is pressed */
	RGFW_mouseScrollUp, /*!< mouse wheel is scrolling up */
	RGFW_mouseScrollDown, /*!< mouse wheel is scrolling down */
	RGFW_mouseMisc1, RGFW_mouseMisc2, RGFW_mouseMisc3, RGFW_mouseMisc4, RGFW_mouseMisc5,
	RGFW_mouseFinal
};

#ifndef RGFW_MAX_PATH
#define RGFW_MAX_PATH 260 /* max length of a path (for dnd) */
#endif
#ifndef RGFW_MAX_DROPS
#define RGFW_MAX_DROPS 260 /* max items you can drop at once */
#endif

#define RGFW_BIT(x) (1 << x)

/* for RGFW_event.lockstate */
typedef RGFW_ENUM(u8, RGFW_keymod) {
	RGFW_modCapsLock = RGFW_BIT(0),
	RGFW_modNumLock  = RGFW_BIT(1),
	RGFW_modControl  = RGFW_BIT(2),
	RGFW_modAlt = RGFW_BIT(3),
	RGFW_modShift  = RGFW_BIT(4),
	RGFW_modSuper = RGFW_BIT(5),
	RGFW_modScrollLock = RGFW_BIT(6)
};

/*! gamepad button codes (based on xbox/playstation), you may need to change these values per controller */
typedef RGFW_ENUM(u8, RGFW_gamepadCodes) {
	RGFW_gamepadNone = 0, /*!< or PS X button */
	RGFW_gamepadA, /*!< or PS X button */
	RGFW_gamepadB, /*!< or PS circle button */
	RGFW_gamepadY, /*!< or PS triangle button */
	RGFW_gamepadX, /*!< or PS square button */
	RGFW_gamepadStart, /*!< start button */
	RGFW_gamepadSelect, /*!< select button */
	RGFW_gamepadHome, /*!< home button */
	RGFW_gamepadUp, /*!< dpad up */
	RGFW_gamepadDown, /*!< dpad down */
	RGFW_gamepadLeft, /*!< dpad left */
	RGFW_gamepadRight, /*!< dpad right */
	RGFW_gamepadL1, /*!< left bump */
	RGFW_gamepadL2, /*!< left trigger */
	RGFW_gamepadR1, /*!< right bumper */
	RGFW_gamepadR2, /*!< right trigger */
	RGFW_gamepadL3,  /* left thumb stick */
	RGFW_gamepadR3, /*!< right thumb stick */
	RGFW_gamepadFinal
};

/*! basic vector type, if there's not already a point/vector type of choice */
#ifndef RGFW_point
	typedef struct RGFW_point { i32 x, y; } RGFW_point;
#endif

/*! basic rect type, if there's not already a rect type of choice */
#ifndef RGFW_rect
	typedef struct RGFW_rect { i32 x, y, w, h; } RGFW_rect;
#endif

/*! basic area type, if there's not already a area type of choice */
#ifndef RGFW_area
	typedef struct RGFW_area { u32 w, h; } RGFW_area;
#endif

#if defined(__cplusplus) && !defined(__APPLE__)
#define RGFW_POINT(x, y) {(i32)x, (i32)y}
#define RGFW_RECT(x, y, w, h) {(i32)x, (i32)y, (i32)w, (i32)h}
#define RGFW_AREA(w, h) {(u32)w, (u32)h}
#else
#define RGFW_POINT(x, y) (RGFW_point){(i32)(x), (i32)(y)}
#define RGFW_RECT(x, y, w, h) (RGFW_rect){(i32)(x), (i32)(y), (i32)(w), (i32)(h)}
#define RGFW_AREA(w, h) (RGFW_area){(u32)(w), (u32)(h)}
#endif

#ifndef RGFW_NO_MONITOR
	/* monitor mode data | can be changed by the user (with functions)*/
	typedef struct RGFW_monitorMode {
		RGFW_area area; /*!< monitor workarea size */
		u32 refreshRate; /*!< monitor refresh rate */
		u8 red, blue, green;
	} RGFW_monitorMode;

	/*! structure for monitor data */
	typedef struct RGFW_monitor {
		i32 x, y; /*!< x - y of the monitor workarea */
		char name[128]; /*!< monitor name */
		float scaleX, scaleY; /*!< monitor content scale */
		float pixelRatio; /*!< pixel ratio for monitor (1.0 for regular, 2.0 for hiDPI)  */
		float physW, physH; /*!< monitor physical size in inches */

		RGFW_monitorMode mode;
	} RGFW_monitor;

	/*! get an array of all the monitors (max 6) */
	RGFWDEF RGFW_monitor* RGFW_getMonitors(size_t* len);
	/*! get the primary monitor */
	RGFWDEF RGFW_monitor RGFW_getPrimaryMonitor(void);

	typedef RGFW_ENUM(u8, RGFW_modeRequest) {
		RGFW_monitorScale = RGFW_BIT(0), /*!< scale the monitor size */
		RGFW_monitorRefresh = RGFW_BIT(1), /*!< change the refresh rate */
		RGFW_monitorRGB = RGFW_BIT(2), /*!< change the monitor RGB bits size */
		RGFW_monitorAll = RGFW_monitorScale | RGFW_monitorRefresh | RGFW_monitorRGB
	};

	/*! request a specific mode */
	RGFWDEF RGFW_bool RGFW_monitor_requestMode(RGFW_monitor mon, RGFW_monitorMode mode, RGFW_modeRequest request);
	/*! check if 2 monitor modes are the same */
	RGFWDEF RGFW_bool RGFW_monitorModeCompare(RGFW_monitorMode mon, RGFW_monitorMode mon2, RGFW_modeRequest request);
#endif

/* RGFW mouse loading */
typedef void RGFW_mouse;

/*!< loads mouse icon from bitmap (similar to RGFW_window_setIcon). Icon NOT resized by default */
RGFWDEF RGFW_mouse* RGFW_loadMouse(u8* icon, RGFW_area a, i32 channels);
/*!< frees RGFW_mouse data */
RGFWDEF void RGFW_freeMouse(RGFW_mouse* mouse);

/* NOTE: some parts of the data can represent different things based on the event (read comments in RGFW_event struct) */
/*! Event structure for checking/getting events */
typedef struct RGFW_event {
	RGFW_eventType type; /*!< which event has been sent?*/
	RGFW_point point; /*!< mouse x, y of event (or drop point) */
	RGFW_point vector; /*!< raw mouse movement */
	float scaleX, scaleY; /*!< DPI scaling */

	RGFW_key key; /*!< the physical key of the event, refers to where key is physically !!Keycodes defined at the bottom of the RGFW_HEADER part of this file!! */
	u8 keyChar; /*!< mapped key char of the event */

	RGFW_bool repeat; /*!< key press event repeated (the key is being held) */
	RGFW_keymod keyMod;

	u8 button; /* !< which mouse (or gamepad) button was pressed */
	double scroll; /*!< the raw mouse scroll value */

	u16 gamepad; /*! which gamepad this event applies to (if applicable to any) */
	u8 axisesCount; /*!< number of axises */

	u8 whichAxis; /* which axis was effected */
	RGFW_point axis[4]; /*!< x, y of axises (-100 to 100) */

	/*! drag and drop data */
	/* 260 max paths with a max length of 260 */
	char** droppedFiles; /*!< dropped files */
	size_t droppedFilesCount; /*!< house many files were dropped */

	void* _win; /*!< the window this event applies too (for event queue events) */
} RGFW_event;

/*! source data for the window (used by the APIs) */
#ifdef RGFW_WINDOWS
typedef struct RGFW_window_src {
	HWND window; /*!< source window */
	HDC hdc; /*!< source HDC */
	u32 hOffset; /*!< height offset for window */
	HICON hIconSmall, hIconBig; /*!< source window icons */
	#if (defined(RGFW_OPENGL)) && !defined(RGFW_OSMESA) && !defined(RGFW_EGL)
		HGLRC ctx; /*!< source graphics context */
	#elif defined(RGFW_OSMESA)
		OSMesaContext ctx;
	#elif defined(RGFW_EGL)
		EGLSurface EGL_surface;
		EGLDisplay EGL_display;
		EGLContext EGL_context;
	#endif

	#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
		HDC hdcMem;
		HBITMAP bitmap;
		u8* bitmapBits;
	#endif
	RGFW_area maxSize, minSize, aspectRatio; /*!< for setting max/min resize (RGFW_WINDOWS) */
} RGFW_window_src;
#elif defined(RGFW_UNIX)
typedef struct RGFW_window_src {
#if defined(RGFW_X11)
	Display* display; /*!< source display */
	Window window; /*!< source window */
	#if (defined(RGFW_OPENGL)) && !defined(RGFW_OSMESA) && !defined(RGFW_EGL)
		GLXContext ctx; /*!< source graphics context */
        GLXFBConfig bestFbc; 
	#elif defined(RGFW_OSMESA)
		OSMesaContext ctx;
	#elif defined(RGFW_EGL)
		EGLSurface EGL_surface;
		EGLDisplay EGL_display;
		EGLContext EGL_context;
	#endif

	#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
			XImage* bitmap;
	#endif
	GC gc;
	XVisualInfo visual;
    #ifdef RGFW_ADVANCED_SMOOTH_RESIZE
        i64 counter_value;
        XID counter;
    #endif
    RGFW_rect r;
#endif /* RGFW_X11 */
#if defined(RGFW_WAYLAND)
	struct wl_display* wl_display;
	struct wl_surface* surface;
	struct wl_buffer* wl_buffer;
	struct wl_keyboard* keyboard;

	struct wl_compositor* compositor;
	struct xdg_surface* xdg_surface;
	struct xdg_toplevel* xdg_toplevel;
	struct zxdg_toplevel_decoration_v1* decoration;
	struct xdg_wm_base* xdg_wm_base;
	struct wl_shm* shm;
	struct wl_seat *seat;
	u8* buffer;
	#if defined(RGFW_EGL)
		struct wl_egl_window* eglWindow;
	#endif
	#if defined(RGFW_EGL) && !defined(RGFW_X11)
			EGLSurface EGL_surface;
			EGLDisplay EGL_display;
			EGLContext EGL_context;
	#elif defined(RGFW_OSMESA) && !defined(RGFW_X11)
		OSMesaContext ctx;
	#endif
#endif /* RGFW_WAYLAND */
} RGFW_window_src;
#endif /* RGFW_UNIX */
#if defined(RGFW_MACOS)
typedef struct RGFW_window_src {
	void* window;
#if (defined(RGFW_OPENGL)) && !defined(RGFW_OSMESA) && !defined(RGFW_EGL)
		void* ctx; /*!< source graphics context */
#elif defined(RGFW_OSMESA)
		OSMesaContext ctx;
#elif defined(RGFW_EGL)
		EGLSurface EGL_surface;
		EGLDisplay EGL_display;
		EGLContext EGL_context;
#endif

	void* view; /* apple viewpoint thingy */
	void* mouse;
#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
#endif
} RGFW_window_src;
#elif defined(RGFW_WASM)
typedef struct RGFW_window_src {
	#if defined(RGFW_WEBGPU)
		WGPUInstance ctx;
        WGPUDevice device;
        WGPUQueue queue;
	#elif defined(RGFW_OSMESA)
		OSMesaContext ctx;
	#else
		EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
	#endif
} RGFW_window_src;
#endif

/*! Optional arguments for making a windows */
typedef RGFW_ENUM(u32, RGFW_windowFlags) {
	RGFW_windowNoInitAPI = RGFW_BIT(0), /* do NOT init an API (including the software rendering buffer) (mostly for bindings. you can also use `#define RGFW_NO_API`) */
	RGFW_windowNoBorder = RGFW_BIT(1), /*!< the window doesn't have a border */
	RGFW_windowNoResize = RGFW_BIT(2), /*!< the window cannot be resized by the user */
	RGFW_windowAllowDND = RGFW_BIT(3), /*!< the window supports drag and drop */
	RGFW_windowHideMouse = RGFW_BIT(4), /*! the window should hide the mouse (can be toggled later on using `RGFW_window_mouseShow`) */
	RGFW_windowFullscreen = RGFW_BIT(5), /*!< the window is fullscreen by default */
	RGFW_windowTransparent = RGFW_BIT(6), /*!< the window is transparent (only properly works on X11 and MacOS, although it's meant for for windows) */
	RGFW_windowCenter = RGFW_BIT(7), /*! center the window on the screen */
	RGFW_windowOpenglSoftware = RGFW_BIT(8), /*! use OpenGL software rendering */
	RGFW_windowCocoaCHDirToRes = RGFW_BIT(9), /*! (cocoa only), change directory to resource folder */
	RGFW_windowScaleToMonitor = RGFW_BIT(10), /*! scale the window to the screen */
	RGFW_windowHide = RGFW_BIT(11), /*! the window is hidden */
	RGFW_windowMaximize = RGFW_BIT(12),
	RGFW_windowCenterCursor = RGFW_BIT(13),
	RGFW_windowFloating = RGFW_BIT(14), /*!< create a floating window */
	RGFW_windowFreeOnClose = RGFW_BIT(15), /*!< free (RGFW_window_close) the RGFW_window struct when the window is closed (by the end user) */
	RGFW_windowFocusOnShow = RGFW_BIT(16), /*!< focus the window when it's shown */
	RGFW_windowMinimize = RGFW_BIT(17), /*!< focus the window when it's shown */
	RGFW_windowFocus = RGFW_BIT(18), /*!< if the window is in focus */
	RGFW_windowedFullscreen = RGFW_windowNoBorder | RGFW_windowMaximize
};

typedef struct RGFW_window {
	RGFW_window_src src; /*!< src window data */

#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
	u8* buffer; /*!< buffer for non-GPU systems (OSMesa, basic software rendering) */
	/* when rendering using RGFW_BUFFER, the buffer is in the RGBA format */
	RGFW_area bufferSize;
#endif
	void* userPtr; /* ptr for usr data */

	RGFW_event event; /*!< current event */

	RGFW_rect r; /*!< the x, y, w and h of the struct */
    
    /*! which key RGFW_window_shouldClose checks. Settting this to RGFW_keyNULL disables the feature. */
    RGFW_key exitKey;     
	RGFW_point _lastMousePoint; /*!< last cusor point (for raw mouse data) */
    
	u32 _flags; /*!< windows flags (for RGFW to check) */
	RGFW_rect _oldRect; /*!< rect before fullscreen */
} RGFW_window; /*!< window structure for managing the window */

#if defined(RGFW_X11) || defined(RGFW_MACOS)
	typedef u64 RGFW_thread; /*!< thread type unix */
#else
	typedef void* RGFW_thread; /*!< thread type for windows */
#endif

/*! scale monitor to window size */
RGFWDEF RGFW_bool RGFW_monitor_scaleToWindow(RGFW_monitor mon, RGFW_window* win);

/** * @defgroup Window_management
* @{ */


/*!
 * the class name for X11 and WinAPI. apps with the same class will be grouped by the WM
 * by default the class name will == the root window's name
*/
RGFWDEF void RGFW_setClassName(const char* name);
RGFWDEF void RGFW_setXInstName(const char* name); /*!< X11 instance name (window name will by used by default) */

/*! (cocoa only) change directory to resource folder */
RGFWDEF void RGFW_moveToMacOSResourceDir(void);

/* NOTE: (windows) if the executable has an icon resource named RGFW_ICON, it will be set as the initial icon for the window */

RGFWDEF RGFW_window* RGFW_createWindow(
	const char* name, /* name of the window */
	RGFW_rect rect, /* rect of window */
	RGFW_windowFlags flags /* extra arguments ((u32)0 means no flags used)*/
); /*!< function to create a window and struct */

RGFWDEF RGFW_window* RGFW_createWindowPtr(
	const char* name, /* name of the window */
	RGFW_rect rect, /* rect of window */
	RGFW_windowFlags flags, /* extra arguments (NULL / (u32)0 means no flags used) */
	RGFW_window* win /* ptr to the window struct you want to use */
); /*!< function to create a window (without allocating a window struct) */

RGFWDEF void RGFW_window_initBuffer(RGFW_window* win);
RGFWDEF void RGFW_window_initBufferSize(RGFW_window* win, RGFW_area area);
RGFWDEF void RGFW_window_initBufferPtr(RGFW_window* win, u8* buffer, RGFW_area area);

/*! set the window flags (will undo flags if they don't match the old ones) */
RGFWDEF void RGFW_window_setFlags(RGFW_window* win, RGFW_windowFlags);

/*! get the size of the screen to an area struct */
RGFWDEF RGFW_area RGFW_getScreenSize(void);


/*!
	this function checks an *individual* event (and updates window structure attributes)
	this means, using this function without a while loop may cause event lag

	ex.

	while (RGFW_window_checkEvent(win) != NULL) [this keeps checking events until it reaches the last one]

	this function is optional if you choose to use event callbacks,
	although you still need some way to tell RGFW to process events eg. `RGFW_window_checkEvents`
*/

RGFWDEF RGFW_event* RGFW_window_checkEvent(RGFW_window* win); /*!< check current event (returns a pointer to win->event or NULL if there is no event)*/

/*!
	for RGFW_window_eventWait and RGFW_window_checkEvents
	waitMS -> Allows the function to keep checking for events even after `RGFW_window_checkEvent == NULL`
			  if waitMS == 0, the loop will not wait for events
			  if waitMS > 0, the loop will wait that many miliseconds after there are no more events until it returns
			  if waitMS == -1 or waitMS == the max size of an unsigned 32-bit int, the loop will not return until it gets another event
*/
typedef RGFW_ENUM(i32, RGFW_eventWait) {
	RGFW_eventNoWait = 0,
	RGFW_eventWaitNext = -1
};

/*! sleep until RGFW gets an event or the timer ends (defined by OS) */
RGFWDEF void RGFW_window_eventWait(RGFW_window* win, i32 waitMS);

/*!
	check all the events until there are none left.
	This should only be used if you're using callbacks only
*/
RGFWDEF void RGFW_window_checkEvents(RGFW_window* win, i32 waitMS);

/*!
	tell RGFW_window_eventWait to stop waiting (to be ran from another thread)
*/
RGFWDEF void RGFW_stopCheckEvents(void);

/*! window managment functions */
RGFWDEF void RGFW_window_close(RGFW_window* win); /*!< close the window and free leftover data */

/*! move a window to a given point */
RGFWDEF void RGFW_window_move(RGFW_window* win,
	RGFW_point v /*!< new pos */
);

#ifndef RGFW_NO_MONITOR
	/*! move window to a specific monitor */
	RGFWDEF void RGFW_window_moveToMonitor(RGFW_window* win, RGFW_monitor m /* monitor */);
#endif

/*! resize window to a current size/area */
RGFWDEF void RGFW_window_resize(RGFW_window* win, /*!< source window */
	RGFW_area a /*!< new size */
);

/*! set window aspect ratio */
RGFWDEF void RGFW_window_setAspectRatio(RGFW_window* win, RGFW_area a);
/*! set the minimum dimensions of a window */
RGFWDEF void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a);
/*! set the maximum dimensions of a window */
RGFWDEF void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a);

RGFWDEF void RGFW_window_focus(RGFW_window* win); /*!< sets the focus to this window */
RGFWDEF RGFW_bool RGFW_window_isInFocus(RGFW_window* win); /*!< checks the focus to this window */
RGFWDEF void RGFW_window_raise(RGFW_window* win); /*!< raise the window (to the top) */
RGFWDEF void RGFW_window_maximize(RGFW_window* win); /*!< maximize the window */
RGFWDEF void RGFW_window_setFullscreen(RGFW_window* win, RGFW_bool fullscreen); /*!< turn fullscreen on / off for a window */
RGFWDEF void RGFW_window_center(RGFW_window* win); /*!< center the window */
RGFWDEF void RGFW_window_minimize(RGFW_window* win); /*!< minimize the window (in taskbar (per OS))*/
RGFWDEF void RGFW_window_restore(RGFW_window* win); /*!< restore the window from minimized (per OS)*/
RGFWDEF void RGFW_window_setFloating(RGFW_window* win, RGFW_bool floating); /*!< make the window a floating window */
RGFWDEF void RGFW_window_setOpacity(RGFW_window* win, u8 opacity); /*!< sets the opacity of a window */

RGFWDEF RGFW_bool RGFW_window_opengl_isSoftware(RGFW_window* win);

/*! if the window should have a border or not (borderless) based on bool value of `border` */
RGFWDEF void RGFW_window_setBorder(RGFW_window* win, RGFW_bool border);
RGFWDEF RGFW_bool RGFW_window_borderless(RGFW_window* win);

/*! turn on / off dnd (RGFW_windowAllowDND stil must be passed to the window)*/
RGFWDEF void RGFW_window_setDND(RGFW_window* win, RGFW_bool allow);
/*! check if DND is allowed */
RGFWDEF RGFW_bool RGFW_window_allowsDND(RGFW_window* win);


#ifndef RGFW_NO_PASSTHROUGH
	/*! turn on / off mouse passthrough */
	RGFWDEF void RGFW_window_setMousePassthrough(RGFW_window* win, RGFW_bool passthrough);
#endif

/*! rename window to a given string */
RGFWDEF void RGFW_window_setName(RGFW_window* win,
	const char* name
);

RGFWDEF RGFW_bool RGFW_window_setIcon(RGFW_window* win, /*!< source window */
	u8* icon /*!< icon bitmap */,
	RGFW_area a /*!< width and height of the bitmap */,
	i32 channels /*!< how many channels the bitmap has (rgb : 3, rgba : 4) */
); /*!< image MAY be resized by default, set both the taskbar and window icon */

typedef RGFW_ENUM(u8, RGFW_icon) {
	RGFW_iconTaskbar = RGFW_BIT(0),
	RGFW_iconWindow = RGFW_BIT(1),
	RGFW_iconBoth = RGFW_iconTaskbar | RGFW_iconWindow
};
RGFWDEF RGFW_bool RGFW_window_setIconEx(RGFW_window* win, u8* icon, RGFW_area a, i32 channels, u8 type);

/*!< sets mouse to RGFW_mouse icon (loaded from a bitmap struct) */
RGFWDEF void RGFW_window_setMouse(RGFW_window* win, RGFW_mouse* mouse);

/*!< sets the mouse to a standard API cursor (based on RGFW_MOUSE, as seen at the end of the RGFW_HEADER part of this file) */
RGFWDEF	RGFW_bool RGFW_window_setMouseStandard(RGFW_window* win, u8 mouse);

RGFWDEF RGFW_bool RGFW_window_setMouseDefault(RGFW_window* win); /*!< sets the mouse to the default mouse icon */
/*
	Locks cursor at the center of the window
	win->event.point becomes raw mouse movement data

	this is useful for a 3D camera
*/
RGFWDEF void RGFW_window_mouseHold(RGFW_window* win, RGFW_area area);
/*! if the mouse is held by RGFW */
RGFWDEF RGFW_bool RGFW_window_mouseHeld(RGFW_window* win);
/*! stop holding the mouse and let it move freely */
RGFWDEF void RGFW_window_mouseUnhold(RGFW_window* win);

/*! hide the window */
RGFWDEF void RGFW_window_hide(RGFW_window* win);
/*! show the window */
RGFWDEF void RGFW_window_show(RGFW_window* win);

/*
	makes it so `RGFW_window_shouldClose` returns true or overrides a window close
	by modifying window flags
*/
RGFWDEF void RGFW_window_setShouldClose(RGFW_window* win, RGFW_bool shouldClose);

/*! where the mouse is on the screen */
RGFWDEF RGFW_point RGFW_getGlobalMousePoint(void);

/*! where the mouse is on the window */
RGFWDEF RGFW_point RGFW_window_getMousePoint(RGFW_window* win);

/*! show the mouse or hide the mouse */
RGFWDEF void RGFW_window_showMouse(RGFW_window* win, RGFW_bool show);
/*! if the mouse is hidden */
RGFWDEF RGFW_bool RGFW_window_mouseHidden(RGFW_window* win);
/*! move the mouse to a given point */
RGFWDEF void RGFW_window_moveMouse(RGFW_window* win, RGFW_point v);

/*! if the window should close (RGFW_close was sent or escape was pressed) */
RGFWDEF RGFW_bool RGFW_window_shouldClose(RGFW_window* win);
/*! if the window is fullscreen */
RGFWDEF RGFW_bool RGFW_window_isFullscreen(RGFW_window* win);
/*! if the window is hidden */
RGFWDEF RGFW_bool RGFW_window_isHidden(RGFW_window* win);
/*! if the window is minimized */
RGFWDEF RGFW_bool RGFW_window_isMinimized(RGFW_window* win);
/*! if the window is maximized */
RGFWDEF RGFW_bool RGFW_window_isMaximized(RGFW_window* win);
/*! if the window is floating */
RGFWDEF RGFW_bool RGFW_window_isFloating(RGFW_window* win);
/** @} */

/** * @defgroup Monitor
* @{ */

#ifndef RGFW_NO_MONITOR
/*
	scale the window to the monitor.
	This is run by default if the user uses the arg `RGFW_scaleToMonitor` during window creation
*/
RGFWDEF void RGFW_window_scaleToMonitor(RGFW_window* win);
/*! get the struct of the window's monitor  */
RGFWDEF RGFW_monitor RGFW_window_getMonitor(RGFW_window* win);
#endif

/** @} */

/** * @defgroup Input
* @{ */

/*! if window == NULL, it checks if the key is pressed globally. Otherwise, it checks only if the key is pressed while the window in focus. */
RGFWDEF RGFW_bool RGFW_isPressed(RGFW_window* win, RGFW_key key); /*!< if key is pressed (key code)*/

RGFWDEF RGFW_bool RGFW_wasPressed(RGFW_window* win, RGFW_key key); /*!< if key was pressed (checks previous state only) (key code) */

RGFWDEF RGFW_bool RGFW_isHeld(RGFW_window* win, RGFW_key key); /*!< if key is held (key code) */
RGFWDEF RGFW_bool RGFW_isReleased(RGFW_window* win, RGFW_key key); /*!< if key is released (key code) */

/* if a key is pressed and then released, pretty much the same as RGFW_isReleased */
RGFWDEF RGFW_bool RGFW_isClicked(RGFW_window* win, RGFW_key key /*!< key code */);

/*! if a mouse button is pressed */
RGFWDEF RGFW_bool RGFW_isMousePressed(RGFW_window* win, RGFW_mouseButton button /*!< mouse button code */ );
/*! if a mouse button is held */
RGFWDEF RGFW_bool RGFW_isMouseHeld(RGFW_window* win, RGFW_mouseButton button /*!< mouse button code */ );
/*! if a mouse button was released */
RGFWDEF RGFW_bool RGFW_isMouseReleased(RGFW_window* win, RGFW_mouseButton button /*!< mouse button code */ );
/*! if a mouse button was pressed (checks previous state only) */
RGFWDEF RGFW_bool RGFW_wasMousePressed(RGFW_window* win, RGFW_mouseButton button /*!< mouse button code */ );
/** @} */

/** * @defgroup Clipboard
* @{ */
typedef ptrdiff_t RGFW_ssize_t;

RGFWDEF const char* RGFW_readClipboard(size_t* size); /*!< read clipboard data */
/*! read clipboard data or send a NULL str to just get the length of the clipboard data */
RGFWDEF RGFW_ssize_t RGFW_readClipboardPtr(char* str, size_t strCapacity);
RGFWDEF void RGFW_writeClipboard(const char* text, u32 textLen); /*!< write text to the clipboard */
/** @} */



/** * @defgroup error handling
* @{ */
typedef RGFW_ENUM(u8, RGFW_debugType) {
	RGFW_typeError = 0, RGFW_typeWarning, RGFW_typeInfo
};

typedef RGFW_ENUM(u8, RGFW_errorCode) {
	RGFW_noError = 0, /*!< no error */
	RGFW_errOpenglContext, RGFW_errEGLContext, /*!< error with the OpenGL context */
	RGFW_errWayland,
	RGFW_errDirectXContext,
	RGFW_errIOKit,
	RGFW_errClipboard,
	RGFW_errFailedFuncLoad,
	RGFW_errBuffer,
	RGFW_infoMonitor, RGFW_infoWindow, RGFW_infoBuffer, RGFW_infoGlobal, RGFW_infoOpenGL,
	RGFW_warningWayland, RGFW_warningOpenGL
};

typedef struct RGFW_debugContext { RGFW_window* win; RGFW_monitor* monitor; u32 srcError; } RGFW_debugContext;

#if defined(__cplusplus) && !defined(__APPLE__)
#define RGFW_DEBUG_CTX(win, err) {win, NULL, err}
#define RGFW_DEBUG_CTX_MON(monitor) {_RGFW.root, &monitor, 0}
#else
#define RGFW_DEBUG_CTX(win, err) (RGFW_debugContext){win, NULL, err}
#define RGFW_DEBUG_CTX_MON(monitor) (RGFW_debugContext){_RGFW.root, &monitor, 0}
#endif

typedef void (* RGFW_debugfunc)(RGFW_debugType type, RGFW_errorCode err, RGFW_debugContext ctx, const char* msg);
RGFWDEF RGFW_debugfunc RGFW_setDebugCallback(RGFW_debugfunc func);
RGFWDEF void RGFW_sendDebugInfo(RGFW_debugType type, RGFW_errorCode err, RGFW_debugContext ctx, const char* msg);
/** @} */

/**


	event callbacks.
	These are completely optional, so you can use the normal
	RGFW_checkEvent() method if you prefer that

* @defgroup Callbacks
* @{
*/

/*! RGFW_windowMoved, the window and its new rect value  */
typedef void (* RGFW_windowMovedfunc)(RGFW_window* win, RGFW_rect r);
/*! RGFW_windowResized, the window and its new rect value  */
typedef void (* RGFW_windowResizedfunc)(RGFW_window* win, RGFW_rect r);
/*! RGFW_windowRestored, the window and its new rect value  */
typedef void (* RGFW_windowRestoredfunc)(RGFW_window* win, RGFW_rect r);
/*! RGFW_windowMaximized, the window and its new rect value  */
typedef void (* RGFW_windowMaximizedfunc)(RGFW_window* win, RGFW_rect r);
/*! RGFW_windowMinimized, the window and its new rect value  */
typedef void (* RGFW_windowMinimizedfunc)(RGFW_window* win, RGFW_rect r);
/*! RGFW_quit, the window that was closed */
typedef void (* RGFW_windowQuitfunc)(RGFW_window* win);
/*! RGFW_focusIn / RGFW_focusOut, the window who's focus has changed and if its in focus */
typedef void (* RGFW_focusfunc)(RGFW_window* win, RGFW_bool inFocus);
/*! RGFW_mouseEnter / RGFW_mouseLeave, the window that changed, the point of the mouse (enter only) and if the mouse has entered */
typedef void (* RGFW_mouseNotifyfunc)(RGFW_window* win, RGFW_point point, RGFW_bool status);
/*! RGFW_mousePosChanged, the window that the move happened on, and the new point of the mouse  */
typedef void (* RGFW_mousePosfunc)(RGFW_window* win, RGFW_point point, RGFW_point vector);
/*! RGFW_DNDInit, the window, the point of the drop on the windows */
typedef void (* RGFW_dndInitfunc)(RGFW_window* win, RGFW_point point);
/*! RGFW_windowRefresh, the window that needs to be refreshed */
typedef void (* RGFW_windowRefreshfunc)(RGFW_window* win);
/*! RGFW_keyPressed / RGFW_keyReleased, the window that got the event, the mapped key, the physical key, the string version, the state of the mod keys, if it was a press (else it's a release) */
typedef void (* RGFW_keyfunc)(RGFW_window* win, u8 key, u8 keyChar, RGFW_keymod keyMod, RGFW_bool pressed);
/*! RGFW_mouseButtonPressed / RGFW_mouseButtonReleased, the window that got the event, the button that was pressed, the scroll value, if it was a press (else it's a release)  */
typedef void (* RGFW_mouseButtonfunc)(RGFW_window* win, RGFW_mouseButton button, double scroll, RGFW_bool pressed);
/*! RGFW_gamepadButtonPressed, the window that got the event, the button that was pressed, the scroll value, if it was a press (else it's a release) */
typedef void (* RGFW_gamepadButtonfunc)(RGFW_window* win, u16 gamepad, u8 button, RGFW_bool pressed);
/*! RGFW_gamepadAxisMove, the window that got the event, the gamepad in question, the axis values and the axis count */
typedef void (* RGFW_gamepadAxisfunc)(RGFW_window* win, u16 gamepad, RGFW_point axis[2], u8 axisesCount, u8 whichAxis);
/*! RGFW_gamepadConnected / RGFW_gamepadDisconnected, the window that got the event, the gamepad in question, if the controller was connected (else it was disconnected) */
typedef void (* RGFW_gamepadfunc)(RGFW_window* win, u16 gamepad, RGFW_bool connected);
/*! RGFW_dnd, the window that had the drop, the drop data and the number of files dropped */
typedef void (* RGFW_dndfunc)(RGFW_window* win, char** droppedFiles, size_t droppedFilesCount);
/*! RGFW_scaleUpdated, the window the event was sent to, content scaleX, content scaleY */
typedef void (* RGFW_scaleUpdatedfunc)(RGFW_window* win, float scaleX, float scaleY);

/*! set callback for a window move event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowMovedfunc RGFW_setWindowMovedCallback(RGFW_windowMovedfunc func);
/*! set callback for a window resize event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowResizedfunc RGFW_setWindowResizedCallback(RGFW_windowResizedfunc func);
/*! set callback for a window quit event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowQuitfunc RGFW_setWindowQuitCallback(RGFW_windowQuitfunc func);
/*! set callback for a mouse move event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_mousePosfunc RGFW_setMousePosCallback(RGFW_mousePosfunc func);
/*! set callback for a window refresh event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowRefreshfunc RGFW_setWindowRefreshCallback(RGFW_windowRefreshfunc func);
/*! set callback for a window focus change event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_focusfunc RGFW_setFocusCallback(RGFW_focusfunc func);
/*! set callback for a mouse notify event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_mouseNotifyfunc RGFW_setMouseNotifyCallback(RGFW_mouseNotifyfunc func);
/*! set callback for a drop event event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_dndfunc RGFW_setDndCallback(RGFW_dndfunc func);
/*! set callback for a start of a drop event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_dndInitfunc RGFW_setDndInitCallback(RGFW_dndInitfunc func);
/*! set callback for a key (press / release) event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_keyfunc RGFW_setKeyCallback(RGFW_keyfunc func);
/*! set callback for a mouse button (press / release) event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_mouseButtonfunc RGFW_setMouseButtonCallback(RGFW_mouseButtonfunc func);
/*! set callback for a controller button (press / release) event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_gamepadButtonfunc RGFW_setGamepadButtonCallback(RGFW_gamepadButtonfunc func);
/*! set callback for a gamepad axis move event. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_gamepadAxisfunc RGFW_setGamepadAxisCallback(RGFW_gamepadAxisfunc func);
/*! set callback for when a controller is connected or disconnected. Returns the previous callback function (if it was set) */
RGFWDEF RGFW_gamepadfunc RGFW_setGamepadCallback(RGFW_gamepadfunc func);
/*! set call back for when window is maximized. Returns the previous callback function (if it was set) */
RGFWDEF RGFW_windowResizedfunc RGFW_setWindowMaximizedCallback(RGFW_windowResizedfunc func);
/*! set call back for when window is minimized. Returns the previous callback function (if it was set) */
RGFWDEF RGFW_windowResizedfunc RGFW_setWindowMinimizedCallback(RGFW_windowResizedfunc func);
/*! set call back for when window is restored. Returns the previous callback function (if it was set) */
RGFWDEF RGFW_windowResizedfunc RGFW_setWindowRestoredCallback(RGFW_windowResizedfunc func);
/*! set callback for when the DPI changes. Returns previous callback function (if it was set)  */
RGFWDEF RGFW_scaleUpdatedfunc RGFW_setScaleUpdatedCallback(RGFW_scaleUpdatedfunc func);
/** @} */

/** * @defgroup Threads
* @{ */

#ifndef RGFW_NO_THREADS
/*! threading functions */

/*! NOTE! (for X11/linux) : if you define a window in a thread, it must be run after the original thread's window is created or else there will be a memory error */
/*
	I'd suggest you use sili's threading functions instead
	if you're going to use sili
	which is a good idea generally
*/

#if defined(__unix__) || defined(__APPLE__) || defined(RGFW_WASM) || defined(RGFW_CUSTOM_BACKEND)
	typedef void* (* RGFW_threadFunc_ptr)(void*);
#else
	typedef DWORD (__stdcall *RGFW_threadFunc_ptr) (LPVOID lpThreadParameter);
#endif

RGFWDEF RGFW_thread RGFW_createThread(RGFW_threadFunc_ptr ptr, void* args); /*!< create a thread */
RGFWDEF void RGFW_cancelThread(RGFW_thread thread); /*!< cancels a thread */
RGFWDEF void RGFW_joinThread(RGFW_thread thread); /*!< join thread to current thread */
RGFWDEF void RGFW_setThreadPriority(RGFW_thread thread, u8 priority); /*!< sets the priority priority  */
#endif

/** @} */

/** * @defgroup gamepad
* @{ */

typedef RGFW_ENUM(u8, RGFW_gamepadType) {
	RGFW_gamepadMicrosoft = 0, RGFW_gamepadSony, RGFW_gamepadNintendo, RGFW_gamepadLogitech, RGFW_gamepadUnknown
};

/*! gamepad count starts at 0*/
RGFWDEF u32 RGFW_isPressedGamepad(RGFW_window* win, u8 controller, RGFW_gamepadCodes button);
RGFWDEF u32 RGFW_isReleasedGamepad(RGFW_window* win, u8 controller, RGFW_gamepadCodes button);
RGFWDEF u32 RGFW_isHeldGamepad(RGFW_window* win, u8 controller, RGFW_gamepadCodes button);
RGFWDEF u32 RGFW_wasPressedGamepad(RGFW_window* win, u8 controller, RGFW_gamepadCodes button);
RGFWDEF RGFW_point RGFW_getGamepadAxis(RGFW_window* win, u16 controller, u16 whichAxis);
RGFWDEF const char* RGFW_getGamepadName(RGFW_window* win, u16 controller);
RGFWDEF size_t RGFW_getGamepadCount(RGFW_window* win);
RGFWDEF RGFW_gamepadType RGFW_getGamepadType(RGFW_window* win, u16 controller);

/** @} */

/** * @defgroup graphics_API
* @{ */

/*!< make the window the current opengl drawing context

	NOTE:
 	if you want to switch the graphics context's thread,
	you have to run RGFW_window_makeCurrent(NULL); on the old thread
	then RGFW_window_makeCurrent(valid_window) on the new thread
*/
RGFWDEF void RGFW_window_makeCurrent(RGFW_window* win);

/*! get current RGFW window graphics context */
RGFWDEF RGFW_window* RGFW_getCurrent(void);

/* supports openGL, directX, OSMesa, EGL and software rendering */
RGFWDEF void RGFW_window_swapBuffers(RGFW_window* win); /*!< swap the rendering buffer */
RGFWDEF void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval);
/*!< render the software rendering buffer (this is called by RGFW_window_swapInterval)  */
RGFWDEF void RGFW_window_swapBuffers_software(RGFW_window* win);

typedef void (*RGFW_proc)(void); /* function pointer equivalent of void* */

/*! native API functions */
#if defined(RGFW_OPENGL) || defined(RGFW_EGL)
/*!< create an opengl context for the RGFW window, run by createWindow by default (unless the RGFW_windowNoInitAPI is included) */
RGFWDEF void RGFW_window_initOpenGL(RGFW_window* win);
/*!< called by `RGFW_window_close` by default (unless the RGFW_windowNoInitAPI is set) */
RGFWDEF void RGFW_window_freeOpenGL(RGFW_window* win);

/*! OpenGL init hints */
typedef RGFW_ENUM(u8, RGFW_glHints)  {
	RGFW_glStencil = 0,  /*!< set stencil buffer bit size (8 by default) */
	RGFW_glSamples, /*!< set number of sampiling buffers (4 by default) */
	RGFW_glStereo, /*!< use GL_STEREO (GL_FALSE by default) */
	RGFW_glAuxBuffers, /*!< number of aux buffers (0 by default) */
	RGFW_glDoubleBuffer, /*!< request double buffering */
	RGFW_glRed, RGFW_glGreen, RGFW_glBlue, RGFW_glAlpha, /*!< set RGBA bit sizes */
	RGFW_glDepth,
	RGFW_glAccumRed, RGFW_glAccumGreen, RGFW_glAccumBlue,RGFW_glAccumAlpha, /*!< set accumulated RGBA bit sizes */
	RGFW_glSRGB, /*!< request sRGA */
	RGFW_glRobustness, /*!< request a robust context */
	RGFW_glDebug, /*!< request opengl debugging */
	RGFW_glNoError, /*!< request no opengl errors */
	RGFW_glReleaseBehavior,
	RGFW_glProfile,
	RGFW_glMajor, RGFW_glMinor,
	RGFW_glFinalHint = 32, /*!< the final hint (not for setting) */
	RGFW_releaseFlush = 0,  RGFW_glReleaseNone, /* RGFW_glReleaseBehavior options */
	RGFW_glCore = 0,  RGFW_glCompatibility /*!< RGFW_glProfile options */
};
RGFWDEF void RGFW_setGLHint(RGFW_glHints hint, i32 value);
RGFWDEF RGFW_bool RGFW_extensionSupported(const char* extension, size_t len);	/*!< check if whether the specified API extension is supported by the current OpenGL or OpenGL ES context */
RGFWDEF RGFW_proc RGFW_getProcAddress(const char* procname); /*!< get native opengl proc address */
RGFWDEF void RGFW_window_makeCurrent_OpenGL(RGFW_window* win); /*!< to be called by RGFW_window_makeCurrent */
RGFWDEF void RGFW_window_swapBuffers_OpenGL(RGFW_window* win); /*!< swap opengl buffer (only) called by RGFW_window_swapInterval  */
void* RGFW_getCurrent_OpenGL(void); /*!< get the current context (OpenGL backend (GLX) (WGL) (EGL) (cocoa) (webgl))*/

RGFWDEF RGFW_bool RGFW_extensionSupportedPlatform(const char* extension, size_t len);	/*!< check if whether the specified platform-specific API extension is supported by the current OpenGL or OpenGL ES context */
#endif
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

/* if you don't want to use the above macros */
RGFWDEF const char** RGFW_getVKRequiredInstanceExtensions(size_t* count); /*!< gets (static) extension array (and size (which will be 2)) */

#include <vulkan/vulkan.h>

RGFWDEF VkResult RGFW_window_createVKSurface(RGFW_window* win, VkInstance instance, VkSurfaceKHR* surface);
RGFWDEF RGFW_bool RGFW_getVKPresentationSupport(VkInstance instance, VkPhysicalDevice physicalDevice, u32 queueFamilyIndex);
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
RGFWDEF int RGFW_window_createDXSwapChain(RGFW_window* win, IDXGIFactory* pFactory, IUnknown* pDevice, IDXGISwapChain** swapchain);
#endif
#endif

/** @} */

/** * @defgroup Supporting
* @{ */

/*! optional init/deinit function */
RGFWDEF i32 RGFW_init(void); /*!< is called by default when the first window is created by default */
RGFWDEF void RGFW_deinit(void); /*!< is called by default when the last open window is closed */

RGFWDEF double RGFW_getTime(void); /*!< get time in seconds since RGFW_setTime, which ran when the first window is open  */
RGFWDEF u64 RGFW_getTimeNS(void); /*!< get time in nanoseconds RGFW_setTime, which ran when the first window is open */
RGFWDEF void RGFW_sleep(u64 milisecond); /*!< sleep for a set time */
RGFWDEF void RGFW_setTime(double time); /*!< set timer in seconds */
RGFWDEF u64 RGFW_getTimerValue(void); /*!< get API timer value */
RGFWDEF u64 RGFW_getTimerFreq(void); /*!< get API time freq */

/*< updates fps / sets fps to cap (must by ran manually by the user at the end of a frame), returns current fps */
RGFWDEF u32 RGFW_checkFPS(double startTime, u32 frameCount, u32 fpsCap);

/*!< change which window is the root window */
RGFWDEF void RGFW_setRootWindow(RGFW_window* win);
RGFWDEF RGFW_window* RGFW_getRootWindow(void);

/*! standard event queue, used for injecting events and returning source API callback events like any other queue check */
/* these are all used internally by RGFW */
void RGFW_eventQueuePush(RGFW_event event);
RGFW_event* RGFW_eventQueuePop(RGFW_window* win);

/* for C++ / C89 */
#define RGFW_eventQueuePushEx(eventInit) { RGFW_event e; eventInit; RGFW_eventQueuePush(e); }

/*!
	key codes and mouse icon enums
*/
#undef RGFW_key
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
	RGFW_end,
	RGFW_home,
	RGFW_pageUp,
	RGFW_pageDown,

	RGFW_numLock,
	RGFW_KP_Slash,
	RGFW_multiply,
	RGFW_KP_Minus,
	RGFW_KP_1,
	RGFW_KP_2,
	RGFW_KP_3,
	RGFW_KP_4,
	RGFW_KP_5,
	RGFW_KP_6,
	RGFW_KP_7,
	RGFW_KP_8,
	RGFW_KP_9,
	RGFW_KP_0,
	RGFW_KP_Period,
	RGFW_KP_Return,
	RGFW_scrollLock,
    RGFW_printScreen,
    RGFW_pause,
    RGFW_keyLast = 256 /* padding for alignment ~(175 by default) */
 };


/*! converts api keycode to the RGFW unmapped/physical key */
RGFWDEF u32 RGFW_apiKeyToRGFW(u32 keycode);
/*! converts RGFW keycode to the unmapped/physical api key */
RGFWDEF u32 RGFW_rgfwToApiKey(u32 keycode);
/*! converts RGFW keycode to the mapped keychar */
RGFWDEF u8 RGFW_rgfwToKeyChar(u32 keycode);

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
    RGFW_mouseIconFinal = 16 /* padding for alignment */
};
/** @} */

#endif /* RGFW_HEADER */
#if defined(RGFW_X11) || defined(RGFW_WAYLAND)
	#define RGFW_OS_BASED_VALUE(l, w, m, h) l
#elif defined(RGFW_WINDOWS)
	#define RGFW_OS_BASED_VALUE(l, w, m, h) w
#elif defined(RGFW_MACOS)
	#define RGFW_OS_BASED_VALUE(l, w, m, h) m
#elif defined(RGFW_WASM)
	#define RGFW_OS_BASED_VALUE(l, w, m, h) h
#endif


#ifdef RGFW_IMPLEMENTATION
RGFW_bool RGFW_useWaylandBool = 1;
void RGFW_useWayland(RGFW_bool wayland) { RGFW_useWaylandBool = wayland;  }
RGFW_bool RGFW_usingWayland(void) { return RGFW_useWaylandBool; }

#if !defined(RGFW_NO_X11) && defined(RGFW_WAYLAND)
#define RGFW_GOTO_WAYLAND(fallback) if (RGFW_useWaylandBool && fallback == 0) goto wayland
#define RGFW_WAYLAND_LABEL wayland:; 
#else
#define RGFW_GOTO_WAYLAND(fallback)
#define RGFW_WAYLAND_LABEL 
#endif

char* RGFW_clipboard_data;
void RGFW_clipboard_switch(char* newstr);
void RGFW_clipboard_switch(char* newstr) {
	if (RGFW_clipboard_data != NULL)
		RGFW_FREE(RGFW_clipboard_data);
	RGFW_clipboard_data =  newstr;
}

#define RGFW_CHECK_CLIPBOARD() \
	if (size <= 0 && RGFW_clipboard_data != NULL) \
		return (const char*)RGFW_clipboard_data; \
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

RGFW_debugfunc RGFW_debugCallback = NULL;
RGFW_debugfunc RGFW_setDebugCallback(RGFW_debugfunc func) {
	RGFW_debugfunc RGFW_debugCallbackPrev = RGFW_debugCallback;
	RGFW_debugCallback = func;
	return RGFW_debugCallbackPrev;
}

#ifdef RGFW_DEBUG
#include <stdio.h>
#endif

void RGFW_sendDebugInfo(RGFW_debugType type, RGFW_errorCode err, RGFW_debugContext ctx, const char* msg) {
	if (RGFW_debugCallback) RGFW_debugCallback(type, err, ctx, msg);
	#ifdef RGFW_DEBUG
	switch (type) {
		case RGFW_typeInfo: printf("RGFW INFO (%i %i): %s", type, err, msg); break;
		case RGFW_typeError: printf("RGFW DEBUG (%i %i): %s", type, err, msg); break;
		case RGFW_typeWarning: printf("RGFW WARNING (%i %i): %s", type, err, msg); break;
		default: break;
	}

	switch (err) {
		#ifdef RGFW_BUFFER
		case RGFW_errBuffer: case RGFW_infoBuffer: printf(" buffer size: %i %i\n", ctx.win->bufferSize.w, ctx.win->bufferSize.h); break;
		#endif
		case RGFW_infoMonitor: printf(": scale (%s):\n   rect: {%i, %i, %i, %i}\n   physical size:%f %f\n   scale: %f %f\n   pixelRatio: %f\n   refreshRate: %i\n   depth: %i\n", ctx.monitor->name, ctx.monitor->x, ctx.monitor->y, ctx.monitor->mode.area.w, ctx.monitor->mode.area.h, ctx.monitor->physW, ctx.monitor->physH, ctx.monitor->scaleX, ctx.monitor->scaleY, ctx.monitor->pixelRatio, ctx.monitor->mode.refreshRate, ctx.monitor->mode.red + ctx.monitor->mode.green + ctx.monitor->mode.blue); break;
		case RGFW_infoWindow: printf("  with rect of {%i, %i, %i, %i} \n", ctx.win->r.x, ctx.win->r.y,ctx. win->r.w, ctx.win->r.h); break;
		case RGFW_errDirectXContext: printf(" srcError %i\n", ctx.srcError); break;
		default: printf("\n");
	}
	#endif
}

u64 RGFW_timerOffset = 0;
void RGFW_setTime(double time) {
    RGFW_timerOffset = RGFW_getTimerValue() - (u64)(time * (double)RGFW_getTimerFreq());
}

double RGFW_getTime(void) {
	return (double) ((double)(RGFW_getTimerValue() - RGFW_timerOffset) / (double)RGFW_getTimerFreq());
}

u64 RGFW_getTimeNS(void) {
	return (u64)(((double)((RGFW_getTimerValue() - RGFW_timerOffset)) * 1e9) / (double)RGFW_getTimerFreq());
}

/*
RGFW_IMPLEMENTATION starts with generic RGFW defines

This is the start of keycode data
*/



/*
	the c++ compiler doesn't support setting up an array like,
	we'll have to do it during runtime using a function & this messy setup
*/

#ifndef RGFW_CUSTOM_BACKEND

#if !defined(__cplusplus) && !defined(RGFW_C89)
#define RGFW_NEXT ,
#define RGFW_MAP
#else
#define RGFW_NEXT ;
#define RGFW_MAP RGFW_keycodes
#endif

u32 RGFW_apiKeycodes[RGFW_keyLast] = { 0 };

u8 RGFW_keycodes [RGFW_OS_BASED_VALUE(256, 512, 128, 256)] = {
#if defined(__cplusplus) || defined(RGFW_C89)
	0
};
void RGFW_init_keys(void);
void RGFW_init_keys(void) {
#endif
	RGFW_MAP [RGFW_OS_BASED_VALUE(49, 0x029, 50, DOM_VK_BACK_QUOTE)] = RGFW_backtick 		RGFW_NEXT

	RGFW_MAP [RGFW_OS_BASED_VALUE(19, 0x00B, 29, DOM_VK_0)] = RGFW_0 					RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(10, 0x002, 18, DOM_VK_1)] = RGFW_1						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(11, 0x003, 19, DOM_VK_2)] = RGFW_2						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(12, 0x004, 20, DOM_VK_3)] = RGFW_3						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(13, 0x005, 21, DOM_VK_4)] = RGFW_4						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(14, 0x006, 23, DOM_VK_5)] = RGFW_5                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(15, 0x007, 22, DOM_VK_6)] = RGFW_6                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(16, 0x008, 26, DOM_VK_7)] = RGFW_7                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(17, 0x009, 28, DOM_VK_8)] = RGFW_8                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(18, 0x00A, 25, DOM_VK_9)] = RGFW_9,
	RGFW_MAP [RGFW_OS_BASED_VALUE(65, 0x039, 49, DOM_VK_SPACE)] = RGFW_space,
	RGFW_MAP [RGFW_OS_BASED_VALUE(38, 0x01E, 0, DOM_VK_A)] = RGFW_a                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(56, 0x030, 11, DOM_VK_B)] = RGFW_b                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(54, 0x02E, 8, DOM_VK_C)] = RGFW_c                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(40, 0x020, 2, DOM_VK_D)] = RGFW_d                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(26, 0x012, 14, DOM_VK_E)] = RGFW_e                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(41, 0x021, 3, DOM_VK_F)] = RGFW_f                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(42, 0x022, 5, DOM_VK_G)] = RGFW_g                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(43, 0x023, 4, DOM_VK_H)] = RGFW_h                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(31, 0x017, 34, DOM_VK_I)] = RGFW_i                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(44, 0x024, 38, DOM_VK_J)] = RGFW_j                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(45, 0x025, 40, DOM_VK_K)] = RGFW_k                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(46, 0x026, 37, DOM_VK_L)] = RGFW_l                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(58, 0x032, 46, DOM_VK_M)] = RGFW_m                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(57, 0x031, 45, DOM_VK_N)] = RGFW_n                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(32, 0x018, 31, DOM_VK_O)] = RGFW_o                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(33, 0x019, 35, DOM_VK_P)] = RGFW_p                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(24, 0x010, 12, DOM_VK_Q)] = RGFW_q                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(27, 0x013, 15, DOM_VK_R)] = RGFW_r                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(39, 0x01F, 1, DOM_VK_S)] = RGFW_s                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(28, 0x014, 17, DOM_VK_T)] = RGFW_t                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(30, 0x016, 32, DOM_VK_U)] = RGFW_u                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(55, 0x02F, 9, DOM_VK_V)] = RGFW_v                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(25, 0x011, 13, DOM_VK_W)] = RGFW_w                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(53, 0x02D, 7, DOM_VK_X)] = RGFW_x                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(29, 0x015, 16, DOM_VK_Y)] = RGFW_y                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(52, 0x02C, 6, DOM_VK_Z)] = RGFW_z,
	RGFW_MAP [RGFW_OS_BASED_VALUE(60, 0x034, 47, DOM_VK_PERIOD)] = RGFW_period             			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(59, 0x033, 43, DOM_VK_COMMA)] = RGFW_comma               			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(61, 0x035, 44, DOM_VK_SLASH)] = RGFW_slash               			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(34, 0x01A, 33, DOM_VK_OPEN_BRACKET)] = RGFW_bracket      			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(35, 0x01B, 30, DOM_VK_CLOSE_BRACKET)] = RGFW_closeBracket             RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(47, 0x027, 41, DOM_VK_SEMICOLON)] = RGFW_semicolon                 RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(48, 0x028, 39, DOM_VK_QUOTE)] = RGFW_apostrophe                 			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(51, 0x02B, 42, DOM_VK_BACK_SLASH)] = RGFW_backSlash,
	RGFW_MAP [RGFW_OS_BASED_VALUE(36, 0x01C, 36, DOM_VK_RETURN)] = RGFW_return              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(119, 0x153, 118, DOM_VK_DELETE)] = RGFW_delete                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(77, 0x145, 72, DOM_VK_NUM_LOCK)] = RGFW_numLock               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(106, 0x135, 82, DOM_VK_DIVIDE)] = RGFW_KP_Slash               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(63, 0x037, 76, DOM_VK_MULTIPLY)] = RGFW_multiply              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(82, 0x04A, 67, DOM_VK_SUBTRACT)] = RGFW_KP_Minus              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(87, 0x04F, 84, DOM_VK_NUMPAD1)] = RGFW_KP_1               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(88, 0x050, 85, DOM_VK_NUMPAD2)] = RGFW_KP_2               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(89, 0x051, 86, DOM_VK_NUMPAD3)] = RGFW_KP_3               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(83, 0x04B, 87, DOM_VK_NUMPAD4)] = RGFW_KP_4               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(84, 0x04C, 88, DOM_VK_NUMPAD5)] = RGFW_KP_5               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(85, 0x04D, 89, DOM_VK_NUMPAD6)] = RGFW_KP_6               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(79, 0x047, 90, DOM_VK_NUMPAD7)] = RGFW_KP_7               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(80, 0x048, 92, DOM_VK_NUMPAD8)] = RGFW_KP_8               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(81, 0x049, 93, DOM_VK_NUMPAD9)] = RGFW_KP_9               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(90, 0x052, 83, DOM_VK_NUMPAD0)] = RGFW_KP_0               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(91, 0x053, 65, DOM_VK_DECIMAL)] = RGFW_KP_Period              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(104, 0x11C, 77, 0)] = RGFW_KP_Return,
	RGFW_MAP [RGFW_OS_BASED_VALUE(20, 0x00C, 27, DOM_VK_HYPHEN_MINUS)] = RGFW_minus              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(21, 0x00D, 24, DOM_VK_EQUALS)] = RGFW_equals               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(22, 0x00E, 51, DOM_VK_BACK_SPACE)] = RGFW_backSpace              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(23, 0x00F, 48, DOM_VK_TAB)] = RGFW_tab                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(66, 0x03A, 57, DOM_VK_CAPS_LOCK)] = RGFW_capsLock               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(50, 0x02A, 56, DOM_VK_SHIFT)] = RGFW_shiftL               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(37, 0x01D, 59, DOM_VK_CONTROL)] = RGFW_controlL               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(64, 0x038, 58, DOM_VK_ALT)] = RGFW_altL                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(133, 0x15B, 55, DOM_VK_WIN)] = RGFW_superL,
	#if !defined(RGFW_MACOS) && !defined(RGFW_WASM)
	RGFW_MAP [RGFW_OS_BASED_VALUE(105, 0x11D, 59, 0)] = RGFW_controlR               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(134, 0x15C, 55, 0)] = RGFW_superR,
	RGFW_MAP [RGFW_OS_BASED_VALUE(62, 0x036, 56, 0)] = RGFW_shiftR              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(108, 0x138, 58, 0)] = RGFW_altR,
	#endif
	RGFW_MAP [RGFW_OS_BASED_VALUE(67, 0x03B, 127, DOM_VK_F1)] = RGFW_F1                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(68, 0x03C, 121, DOM_VK_F2)] = RGFW_F2                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(69, 0x03D, 100, DOM_VK_F3)] = RGFW_F3                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(70, 0x03E, 119, DOM_VK_F4)] = RGFW_F4                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(71, 0x03F, 97, DOM_VK_F5)] = RGFW_F5              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(72, 0x040, 98, DOM_VK_F6)] = RGFW_F6              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(73, 0x041, 99, DOM_VK_F7)] = RGFW_F7              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(74, 0x042, 101, DOM_VK_F8)] = RGFW_F8                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(75, 0x043, 102, DOM_VK_F9)] = RGFW_F9                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(76, 0x044, 110, DOM_VK_F10)] = RGFW_F10               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(95, 0x057, 104, DOM_VK_F11)] = RGFW_F11               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(96, 0x058, 111, DOM_VK_F12)] = RGFW_F12               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(111, 0x148, 126, DOM_VK_UP)] = RGFW_up                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(116, 0x150, 125, DOM_VK_DOWN)] = RGFW_down                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(113, 0x14B, 123, DOM_VK_LEFT)] = RGFW_left                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(114, 0x14D, 124, DOM_VK_RIGHT)] = RGFW_right              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(118, 0x152, 115, DOM_VK_INSERT)] = RGFW_insert                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(115, 0x14F, 120, DOM_VK_END)] = RGFW_end                  		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(112, 0x149, 117, DOM_VK_PAGE_UP)] = RGFW_pageUp                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(117, 0x151, 122, DOM_VK_PAGE_DOWN)] = RGFW_pageDown            RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(9, 0x001, 53, DOM_VK_ESCAPE)] = RGFW_escape                   		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(110, 0x147, 116, DOM_VK_HOME)] = RGFW_home                    		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(78, 0x046, 107, DOM_VK_SCROLL_LOCK)] = RGFW_scrollLock               RGFW_NEXT
    RGFW_MAP [RGFW_OS_BASED_VALUE(107, 0x137, 105, DOM_VK_PRINTSCREEN)] = RGFW_printScreen              RGFW_NEXT
    RGFW_MAP [RGFW_OS_BASED_VALUE(128, 0x045, 113, DOM_VK_PAUSE)] = RGFW_pause               RGFW_NEXT
#if defined(__cplusplus) || defined(RGFW_C89)
}
#else
};
#endif

#undef RGFW_NEXT
#undef RGFW_MAP

u32 RGFW_apiKeyToRGFW(u32 keycode) {
    #if defined(__cplusplus) || defined(RGFW_C89)
	if (RGFW_keycodes[RGFW_OS_BASED_VALUE(49, 0x029, 50, DOM_VK_BACK_QUOTE)] != RGFW_backtick) {
		RGFW_init_keys();
	}
	#endif

	/* make sure the key isn't out of bounds */
	if (keycode > sizeof(RGFW_keycodes) / sizeof(u8))
		return 0;

	return RGFW_keycodes[keycode];
}

u32 RGFW_rgfwToApiKey(u32 keycode) {
	if (RGFW_apiKeycodes[RGFW_backtick] != RGFW_OS_BASED_VALUE(49, 0x029, 50, DOM_VK_BACK_QUOTE)) {
        for (u32 i = 0; i < RGFW_keyLast; i++) {
            for (u32 y = 0; y < sizeof(RGFW_keycodes); y++) {
                if (RGFW_keycodes[y] == i) {
                    RGFW_apiKeycodes[i] = y;
                    break;
                }
            }
        }
    }

	/* make sure the key isn't out of bounds */
	if (keycode > sizeof(RGFW_apiKeycodes) / sizeof(u32))
		return 0;

	return RGFW_apiKeycodes[keycode];
}
#endif /* RGFW_CUSTOM_BACKEND */

typedef struct {
	RGFW_bool current  : 1;
	RGFW_bool prev  : 1;
} RGFW_keyState;

RGFW_keyState RGFW_keyboard[RGFW_keyLast] = { {0, 0} };

RGFWDEF void RGFW_resetKeyPrev(void);
void RGFW_resetKeyPrev(void) {
	size_t i; /*!< reset each previous state  */
    for (i = 0; i < RGFW_keyLast; i++) RGFW_keyboard[i].prev = 0;
}
RGFWDEF void RGFW_resetKey(void);
void RGFW_resetKey(void) { RGFW_MEMSET(RGFW_keyboard, 0, sizeof(RGFW_keyboard)); }
/*
	this is the end of keycode data
*/

/* gamepad data */
RGFW_keyState RGFW_gamepadPressed[4][32]; /*!< if a key is currently pressed or not (per gamepad) */
RGFW_point RGFW_gamepadAxes[4][4]; /*!< if a key is currently pressed or not (per gamepad) */

RGFW_gamepadType RGFW_gamepads_type[4]; /*!< if a key is currently pressed or not (per gamepad) */
i32 RGFW_gamepads[4] = {0, 0, 0, 0}; /*!< limit of 4 gamepads at a time */
char RGFW_gamepads_name[4][128]; /*!< gamepad names */
u16 RGFW_gamepadCount = 0; /*!< the actual amount of gamepads */

/*
	event callback defines start here
*/


/*
	These exist to avoid the
	if (func == NULL) check
	for (allegedly) better performance

	RGFW_EMPTY_DEF exists to prevent the missing-prototypes warning
*/
static void RGFW_windowMovedfuncEMPTY(RGFW_window* win, RGFW_rect r) { RGFW_UNUSED(win); RGFW_UNUSED(r); }
static void RGFW_windowResizedfuncEMPTY(RGFW_window* win, RGFW_rect r) { RGFW_UNUSED(win); RGFW_UNUSED(r); }
static void RGFW_windowRestoredfuncEMPTY(RGFW_window* win, RGFW_rect r) { RGFW_UNUSED(win); RGFW_UNUSED(r); }
static void RGFW_windowMinimizedfuncEMPTY(RGFW_window* win, RGFW_rect r) { RGFW_UNUSED(win); RGFW_UNUSED(r); }
static void RGFW_windowMaximizedfuncEMPTY(RGFW_window* win, RGFW_rect r) { RGFW_UNUSED(win); RGFW_UNUSED(r); }
static void RGFW_windowQuitfuncEMPTY(RGFW_window* win) { RGFW_UNUSED(win); }
static void RGFW_focusfuncEMPTY(RGFW_window* win, RGFW_bool inFocus) {RGFW_UNUSED(win); RGFW_UNUSED(inFocus);}
static void RGFW_mouseNotifyfuncEMPTY(RGFW_window* win, RGFW_point point, RGFW_bool status) {RGFW_UNUSED(win); RGFW_UNUSED(point); RGFW_UNUSED(status);}
static void RGFW_mousePosfuncEMPTY(RGFW_window* win, RGFW_point point, RGFW_point vector) {RGFW_UNUSED(win); RGFW_UNUSED(point); RGFW_UNUSED(vector);}
static void RGFW_dndInitfuncEMPTY(RGFW_window* win, RGFW_point point) {RGFW_UNUSED(win); RGFW_UNUSED(point);}
static void RGFW_windowRefreshfuncEMPTY(RGFW_window* win) {RGFW_UNUSED(win); }
static void RGFW_keyfuncEMPTY(RGFW_window* win, RGFW_key key, u8 keyChar, RGFW_keymod keyMod, RGFW_bool pressed) {RGFW_UNUSED(win); RGFW_UNUSED(key); RGFW_UNUSED(keyChar); RGFW_UNUSED(keyMod); RGFW_UNUSED(pressed);}
static void RGFW_mouseButtonfuncEMPTY(RGFW_window* win, RGFW_mouseButton button, double scroll, RGFW_bool pressed) {RGFW_UNUSED(win); RGFW_UNUSED(button); RGFW_UNUSED(scroll); RGFW_UNUSED(pressed);}
static void RGFW_gamepadButtonfuncEMPTY(RGFW_window* win, u16 gamepad, u8 button, RGFW_bool pressed) {RGFW_UNUSED(win); RGFW_UNUSED(gamepad); RGFW_UNUSED(button); RGFW_UNUSED(pressed); }
static void RGFW_gamepadAxisfuncEMPTY(RGFW_window* win, u16 gamepad, RGFW_point axis[2], u8 axisesCount, u8 whichAxis) {RGFW_UNUSED(win); RGFW_UNUSED(gamepad); RGFW_UNUSED(axis); RGFW_UNUSED(axisesCount); RGFW_UNUSED(whichAxis); }
static void RGFW_gamepadfuncEMPTY(RGFW_window* win, u16 gamepad, RGFW_bool connected) {RGFW_UNUSED(win); RGFW_UNUSED(gamepad); RGFW_UNUSED(connected);}
static void RGFW_dndfuncEMPTY(RGFW_window* win, char** droppedFiles, size_t droppedFilesCount) {RGFW_UNUSED(win); RGFW_UNUSED(droppedFiles); RGFW_UNUSED(droppedFilesCount);}
static void RGFW_scaleUpdatedfuncEMPTY(RGFW_window* win, float scaleX, float scaleY) {RGFW_UNUSED(win); RGFW_UNUSED(scaleX); RGFW_UNUSED(scaleY); }

#define RGFW_CALLBACK_DEFINE(x, x2) \
RGFW_##x##func RGFW_##x##Callback = RGFW_##x##funcEMPTY; \
RGFW_##x##func RGFW_set##x2##Callback(RGFW_##x##func func) { \
    RGFW_##x##func prev = RGFW_##x##Callback; \
    RGFW_##x##Callback = func; \
    return prev; \
}
RGFW_CALLBACK_DEFINE(windowMaximized, WindowMaximized)
RGFW_CALLBACK_DEFINE(windowMinimized, WindowMinimized)
RGFW_CALLBACK_DEFINE(windowRestored, WindowRestored)
RGFW_CALLBACK_DEFINE(windowMoved, WindowMoved)
RGFW_CALLBACK_DEFINE(windowResized, WindowResized)
RGFW_CALLBACK_DEFINE(windowQuit, WindowQuit)
RGFW_CALLBACK_DEFINE(mousePos, MousePos)
RGFW_CALLBACK_DEFINE(windowRefresh, WindowRefresh)
RGFW_CALLBACK_DEFINE(focus, Focus)
RGFW_CALLBACK_DEFINE(mouseNotify, MouseNotify)
RGFW_CALLBACK_DEFINE(dnd, Dnd)
RGFW_CALLBACK_DEFINE(dndInit, DndInit)
RGFW_CALLBACK_DEFINE(key, Key)
RGFW_CALLBACK_DEFINE(mouseButton, MouseButton)
RGFW_CALLBACK_DEFINE(gamepadButton, GamepadButton)
RGFW_CALLBACK_DEFINE(gamepadAxis, GamepadAxis)
RGFW_CALLBACK_DEFINE(gamepad, Gamepad)
RGFW_CALLBACK_DEFINE(scaleUpdated, ScaleUpdated)
#undef RGFW_CALLBACK_DEFINE

void RGFW_window_checkEvents(RGFW_window* win, i32 waitMS) {
	RGFW_window_eventWait(win, waitMS);

	while (RGFW_window_checkEvent(win) != NULL && RGFW_window_shouldClose(win) == 0) {
		if (win->event.type == RGFW_quit) return;
	}

	#ifdef RGFW_WASM /* WASM needs to run the sleep function for asyncify */
		RGFW_sleep(0);
	#endif
}

void RGFW_window_checkMode(RGFW_window* win);
void RGFW_window_checkMode(RGFW_window* win) {
	if (RGFW_window_isMinimized(win)) {
		win->_flags |= RGFW_windowMinimize;
		RGFW_windowMinimizedCallback(win, win->r);
	} else if (RGFW_window_isMaximized(win)) {
		win->_flags |= RGFW_windowMaximize;
		RGFW_eventQueuePushEx(e.type = RGFW_windowMaximized; e._win = win);
		RGFW_windowMaximizedCallback(win, win->r);
	} else if (((win->_flags & RGFW_windowMinimize) && !RGFW_window_isMaximized(win)) ||
				(win->_flags & RGFW_windowMaximize && !RGFW_window_isMaximized(win))) {
		win->_flags &= ~(u32)RGFW_windowMinimize;
		if (RGFW_window_isMaximized(win) == RGFW_FALSE) win->_flags &= ~(u32)RGFW_windowMaximize;
		RGFW_eventQueuePushEx(e.type = RGFW_windowRestored; e._win = win);
		RGFW_windowRestoredCallback(win, win->r);
	}
}

/*
no more event call back defines
*/

#define SET_ATTRIB(a, v) { \
    RGFW_ASSERT(((size_t) index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[index++] = a; \
    attribs[index++] = v; \
}

#define RGFW_EVENT_PASSED 		RGFW_BIT(24) /* if a queued event was passed */
#define RGFW_EVENT_QUIT 		RGFW_BIT(25) /* the window close button was pressed */
#define RGFW_HOLD_MOUSE			RGFW_BIT(26) /*!< hold the moues still */
#define RGFW_MOUSE_LEFT 		RGFW_BIT(27) /* if mouse left the window */
#define RGFW_WINDOW_ALLOC 		RGFW_BIT(28) /* if window was allocated by RGFW */
#define RGFW_BUFFER_ALLOC 		RGFW_BIT(29) /* if window.buffer was allocated by RGFW */
#define RGFW_WINDOW_INIT 		RGFW_BIT(30) /* if window.buffer was allocated by RGFW */
#define RGFW_INTERNAL_FLAGS (RGFW_EVENT_QUIT | RGFW_EVENT_PASSED | RGFW_HOLD_MOUSE |  RGFW_MOUSE_LEFT | RGFW_WINDOW_ALLOC | RGFW_BUFFER_ALLOC | RGFW_windowFocus)

RGFW_window* RGFW_createWindow(const char* name, RGFW_rect rect, RGFW_windowFlags flags) {
	RGFW_window* win = (RGFW_window*)RGFW_ALLOC(sizeof(RGFW_window));
	RGFW_ASSERT(win != NULL);
    win->_flags = RGFW_WINDOW_ALLOC;
    return RGFW_createWindowPtr(name, rect, flags, win);
}

#if defined(RGFW_USE_XDL) && defined(RGFW_X11)
	#define XDL_IMPLEMENTATION
	#include "XDL.h"
#endif

#define RGFW_MAX_EVENTS 32
typedef struct RGFW_globalStruct {
    RGFW_window* root;
    RGFW_window* current;
    i32 windowCount;
    i32 eventLen;
    i32 eventIndex;

    #ifdef RGFW_X11
        Display* display;
        Window helperWindow;
    	char* clipboard; /* for writing to the clipboard selection */
	    size_t clipboard_len;
    #endif
    #ifdef RGFW_WAYLAND
	    struct wl_display* wl_display;
    #endif
    #if defined(RGFW_X11) || defined(RGFW_WINDOWS) || defined(RGFW_WAYLAND)
        RGFW_mouse* hiddenMouse;
    #endif
    RGFW_event events[RGFW_MAX_EVENTS];

} RGFW_globalStruct;
#if !defined(RGFW_C89) && !defined(__cplusplus)
RGFW_globalStruct _RGFW = {.root = NULL, .current = NULL, .windowCount = -1, .eventLen = 0, .eventIndex = 0};
#define _RGFW_init RGFW_TRUE
#else
RGFW_bool _RGFW_init = RGFW_FALSE;
RGFW_globalStruct _RGFW;
#endif

void RGFW_eventQueuePush(RGFW_event event) {
	if (_RGFW.eventLen >= RGFW_MAX_EVENTS) return;
	_RGFW.events[_RGFW.eventLen] = event;
    _RGFW.eventLen++;
}

RGFW_event* RGFW_eventQueuePop(RGFW_window* win) {
	RGFW_event* ev; 
    if (_RGFW.eventLen == 0) return NULL;

	ev = (RGFW_event*)&_RGFW.events[_RGFW.eventIndex];

    _RGFW.eventLen--;
    if (_RGFW.eventLen >= 0 && _RGFW.eventIndex < _RGFW.eventLen) {
		_RGFW.eventIndex++;
    } else if (_RGFW.eventLen == 0) {
        _RGFW.eventIndex = 0;
    }

	if (ev->_win != win && ev->_win != NULL) {
        RGFW_eventQueuePush(*ev);
        return NULL;
	}

    ev->droppedFilesCount = win->event.droppedFilesCount;
	ev->droppedFiles = win->event.droppedFiles;
	return ev;
}

RGFW_event* RGFW_window_checkEventCore(RGFW_window* win);
RGFW_event* RGFW_window_checkEventCore(RGFW_window* win) {
	RGFW_event* ev;
    RGFW_ASSERT(win != NULL);
    if (win->event.type == 0 && _RGFW.eventLen == 0)
		RGFW_resetKeyPrev();

	if (win->event.type == RGFW_quit && win->_flags & RGFW_windowFreeOnClose) {
        static RGFW_event event;
        event = win->event;
        RGFW_window_close(win);
	    return &event;
    }

	if (win->event.type != RGFW_DNDInit) win->event.type = 0;

	/* check queued events */
	ev = RGFW_eventQueuePop(win);
	if (ev != NULL) {
		if (ev->type == RGFW_quit) RGFW_window_setShouldClose(win, RGFW_TRUE);
		win->event = *ev;
    }
	else return NULL;

	return &win->event;
}


RGFWDEF void RGFW_window_basic_init(RGFW_window* win, RGFW_rect rect, RGFW_windowFlags flags);
void RGFW_setRootWindow(RGFW_window* win) { _RGFW.root = win; }
RGFW_window* RGFW_getRootWindow(void) { return _RGFW.root; }

/* do a basic initialization for RGFW_window, this is to standard it for each OS */
void RGFW_window_basic_init(RGFW_window* win, RGFW_rect rect, RGFW_windowFlags flags) {
	RGFW_UNUSED(flags);
    if (_RGFW.windowCount == -1 || _RGFW_init == RGFW_FALSE) RGFW_init();
    _RGFW.windowCount++;

    /* rect based the requested flags */
	if (_RGFW.root == NULL) {
		RGFW_setRootWindow(win);
		RGFW_setTime(0);
	}

	if (!(win->_flags & RGFW_WINDOW_ALLOC)) win->_flags = 0;

	/* set and init the new window's data */
	win->r = rect;
	win->exitKey = RGFW_escape;
	win->event.droppedFilesCount = 0;

	win->_flags = 0 | (win->_flags & RGFW_WINDOW_ALLOC);
    win->_flags |= flags;
	win->event.keyMod = 0;
	win->_lastMousePoint.x = 0; 
	win->_lastMousePoint.y = 0; 

	win->event.droppedFiles = (char**)RGFW_ALLOC(RGFW_MAX_PATH * RGFW_MAX_DROPS);
	RGFW_ASSERT(win->event.droppedFiles != NULL);
     
    { 
        u32 i;
        for (i = 0; i < RGFW_MAX_DROPS; i++)
		    win->event.droppedFiles[i] = (char*)(win->event.droppedFiles + RGFW_MAX_DROPS + (i * RGFW_MAX_PATH));
    }
}

void RGFW_window_setFlags(RGFW_window* win, RGFW_windowFlags flags) {
	RGFW_windowFlags cmpFlags = win->_flags;
	if (win->_flags & RGFW_WINDOW_INIT) cmpFlags = 0;

	#ifndef RGFW_NO_MONITOR
	if (flags & RGFW_windowScaleToMonitor)			RGFW_window_scaleToMonitor(win);
	#endif

	if (flags & RGFW_windowCenter)					RGFW_window_center(win);
	if (flags & RGFW_windowCenterCursor)
		RGFW_window_moveMouse(win, RGFW_POINT(win->r.x + (win->r.w / 2), win->r.y + (win->r.h / 2)));
	if (flags & RGFW_windowNoBorder)				RGFW_window_setBorder(win, 0);
	else RGFW_window_setBorder(win, 1);
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
	if (flags & RGFW_windowCocoaCHDirToRes)			RGFW_moveToMacOSResourceDir();
	if (flags & RGFW_windowFloating)				RGFW_window_setFloating(win, 1);
	else if (cmpFlags & RGFW_windowFloating)		RGFW_window_setFloating(win, 0);
	if (flags & RGFW_windowFocus)					RGFW_window_focus(win);

	if (flags & RGFW_windowNoResize) {
	    RGFW_window_setMaxSize(win, RGFW_AREA(win->r.w, win->r.h));
	    RGFW_window_setMinSize(win, RGFW_AREA(win->r.w, win->r.h));
	} else if (cmpFlags & RGFW_windowNoResize) {
		RGFW_window_setMaxSize(win, RGFW_AREA(0, 0));
		RGFW_window_setMinSize(win, RGFW_AREA(0, 0));
	}

	win->_flags = flags | (win->_flags & RGFW_INTERNAL_FLAGS);
}

RGFW_bool RGFW_window_opengl_isSoftware(RGFW_window* win) {
    return RGFW_BOOL(win->_flags |= RGFW_windowOpenglSoftware);
}

RGFW_bool RGFW_window_isInFocus(RGFW_window* win) {
#ifdef RGFW_WASM
    return RGFW_TRUE;
#else
    return RGFW_BOOL(win->_flags & RGFW_windowFocus);
#endif
}

void RGFW_window_initBuffer(RGFW_window* win) {
    RGFW_area area = RGFW_getScreenSize();
    if ((win->_flags & RGFW_windowNoResize))
        area = RGFW_AREA(win->r.w, win->r.h);

    RGFW_window_initBufferSize(win, area);
}

void RGFW_window_initBufferSize(RGFW_window* win, RGFW_area area) {
#if defined(RGFW_BUFFER) || defined(RGFW_OSMESA)
    win->_flags |= RGFW_BUFFER_ALLOC;
	#ifndef RGFW_WINDOWS
        u8* buffer = (u8*)RGFW_ALLOC(area.w * area.h * 4);
        RGFW_ASSERT(buffer != NULL);

        RGFW_window_initBufferPtr(win, buffer, area);
    #else /* windows's bitmap allocs memory for us */
	RGFW_window_initBufferPtr(win, (u8*)NULL, area);
	#endif
#else
    RGFW_UNUSED(win); RGFW_UNUSED(area);
#endif
}

#ifdef RGFW_MACOS
RGFWDEF void RGFW_window_cocoaSetLayer(RGFW_window* win, void* layer);
RGFWDEF void* RGFW_cocoaGetLayer(void);
#endif

const char* RGFW_className = NULL;
void RGFW_setClassName(const char* name) { RGFW_className = name; }

#ifndef RGFW_X11
void RGFW_setXInstName(const char* name) { RGFW_UNUSED(name); }
#endif

RGFW_keyState RGFW_mouseButtons[RGFW_mouseFinal] = {  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} };

RGFW_bool RGFW_isMousePressed(RGFW_window* win, RGFW_mouseButton button) {
	return RGFW_mouseButtons[button].current && (win == NULL || RGFW_window_isInFocus(win));
}
RGFW_bool RGFW_wasMousePressed(RGFW_window* win, RGFW_mouseButton button) {
	return RGFW_mouseButtons[button].prev && (win != NULL || RGFW_window_isInFocus(win));
}
RGFW_bool RGFW_isMouseHeld(RGFW_window* win, RGFW_mouseButton button) {
	return (RGFW_isMousePressed(win, button) && RGFW_wasMousePressed(win, button));
}
RGFW_bool RGFW_isMouseReleased(RGFW_window* win, RGFW_mouseButton button) {
	return (!RGFW_isMousePressed(win, button) && RGFW_wasMousePressed(win, button));
}

RGFW_point RGFW_window_getMousePoint(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	return win->_lastMousePoint;
}

RGFW_bool RGFW_isPressed(RGFW_window* win, RGFW_key key) {
	return RGFW_keyboard[key].current && (win == NULL || RGFW_window_isInFocus(win));
}

RGFW_bool RGFW_wasPressed(RGFW_window* win, RGFW_key key) {
	return RGFW_keyboard[key].prev && (win == NULL || RGFW_window_isInFocus(win));
}

RGFW_bool RGFW_isHeld(RGFW_window* win, RGFW_key key) {
	return (RGFW_isPressed(win, key) && RGFW_wasPressed(win, key));
}

RGFW_bool RGFW_isClicked(RGFW_window* win, RGFW_key key) {
	return (RGFW_wasPressed(win, key) && !RGFW_isPressed(win, key));
}

RGFW_bool RGFW_isReleased(RGFW_window* win, RGFW_key key) {
	return (!RGFW_isPressed(win, key) && RGFW_wasPressed(win, key));
}

void RGFW_window_makeCurrent(RGFW_window* win) {
    _RGFW.current = win;
#if defined(RGFW_OPENGL) || defined(RGFW_EGL)
    RGFW_window_makeCurrent_OpenGL(win);
#endif
}

RGFW_window* RGFW_getCurrent(void) {
    return _RGFW.current;
}

void RGFW_window_swapBuffers(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_window_swapBuffers_software(win);
#if defined(RGFW_OPENGL) || defined(RGFW_EGL)
	RGFW_window_swapBuffers_OpenGL(win);
#endif
}

RGFWDEF void RGFW_setBit(u32* data, u32 bit, RGFW_bool value);
void RGFW_setBit(u32* data, u32 bit, RGFW_bool value) {
	if (value)
		*data |= bit;
	else if (!value && (*(data) & bit))
		*data ^= bit;
}

void RGFW_window_center(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_area screenR = RGFW_getScreenSize();
	RGFW_window_move(win, RGFW_POINT((i32)(screenR.w - (u32)win->r.w) / 2, (screenR.h - (u32)win->r.h) / 2));
}

RGFW_bool RGFW_monitor_scaleToWindow(RGFW_monitor mon, RGFW_window* win) {
	RGFW_monitorMode mode;
    RGFW_ASSERT(win != NULL);

	mode.area.w = (u32)win->r.w;
	mode.area.h = (u32)win->r.h;
	return RGFW_monitor_requestMode(mon, mode, RGFW_monitorScale);
}

void RGFW_splitBPP(u32 bpp, RGFW_monitorMode* mode);
void RGFW_splitBPP(u32 bpp, RGFW_monitorMode* mode) {
    if (bpp == 32) bpp = 24;
    mode->red = mode->green = mode->blue = (u8)(bpp / 3);

    u32 delta = bpp - (mode->red * 3); /* handle leftovers */
    if (delta >= 1) mode->green = mode->green + 1;
    if (delta == 2) mode->red = mode->red + 1;
}

RGFW_bool RGFW_monitorModeCompare(RGFW_monitorMode mon, RGFW_monitorMode mon2, RGFW_modeRequest request) {
	return (((mon.area.w == mon2.area.w && mon.area.h == mon2.area.h) || !(request & RGFW_monitorScale)) &&
			((mon.refreshRate == mon2.refreshRate) || !(request & RGFW_monitorRefresh)) &&
			((mon.red == mon2.red && mon.green == mon2.green && mon.blue == mon2.blue) || !(request & RGFW_monitorRGB)));
}

RGFW_bool RGFW_window_shouldClose(RGFW_window* win) {
	return (win == NULL || (win->_flags & RGFW_EVENT_QUIT)|| (win->exitKey && RGFW_isPressed(win, win->exitKey)));
}

void RGFW_window_setShouldClose(RGFW_window* win, RGFW_bool shouldClose) {
	if (shouldClose)  {
		win->_flags |= RGFW_EVENT_QUIT;
		RGFW_windowQuitCallback(win);
	} else {
		win->_flags &= ~(u32)RGFW_EVENT_QUIT;
	}
}

#ifndef RGFW_NO_MONITOR
void RGFW_window_scaleToMonitor(RGFW_window* win) {
	RGFW_monitor monitor = RGFW_window_getMonitor(win);
	if (monitor.scaleX == 0 && monitor.scaleY == 0)
		return;

	RGFW_window_resize(win, RGFW_AREA((u32)(monitor.scaleX * (float)win->r.w), (u32)(monitor.scaleY * (float)win->r.h)));
}

void RGFW_window_moveToMonitor(RGFW_window* win, RGFW_monitor m) {
	RGFW_window_move(win, RGFW_POINT(m.x + win->r.x, m.y + win->r.y));
}
#endif

RGFW_bool RGFW_window_setIcon(RGFW_window* win, u8* icon, RGFW_area a, i32 channels) {
	return RGFW_window_setIconEx(win, icon, a, channels, RGFW_iconBoth);
}

RGFWDEF void RGFW_captureCursor(RGFW_window* win, RGFW_rect);
RGFWDEF void RGFW_releaseCursor(RGFW_window* win);


RGFW_bool RGFW_window_mouseHeld(RGFW_window* win) { return RGFW_BOOL(win->_flags & RGFW_HOLD_MOUSE); }

void RGFW_window_mouseHold(RGFW_window* win, RGFW_area area) {
	if (!area.w && !area.h)
		area = RGFW_AREA(win->r.w / 2, win->r.h / 2);

	win->_flags |= RGFW_HOLD_MOUSE;
    RGFW_captureCursor(win, win->r);
	RGFW_window_moveMouse(win, RGFW_POINT(win->r.x + (win->r.w / 2), win->r.y + (win->r.h / 2)));
}

void RGFW_window_mouseUnhold(RGFW_window* win) {
	win->_flags &= ~(u32)RGFW_HOLD_MOUSE;
	RGFW_releaseCursor(win);
}

u32 RGFW_checkFPS(double startTime, u32 frameCount, u32 fpsCap) {
	double deltaTime = RGFW_getTime() - startTime;
	if (deltaTime == 0) return 0;

	double fps = (frameCount / deltaTime); /* the numer of frames over the time it took for them to render */
	if (fpsCap && fps > fpsCap) {
		double frameTime = (double)frameCount / (double)fpsCap; /* how long it should take to finish the frames */
		double sleepTime = frameTime - deltaTime; /* subtract how long it should have taken with how long it did take */

		if (sleepTime > 0) RGFW_sleep((u32)(sleepTime * 1000));
	}

	return (u32) fps;
}

#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
void RGFW_RGB_to_BGR(RGFW_window* win, u8* data) {
	#if !defined(RGFW_BUFFER_BGR) && !defined(RGFW_OSMESA)
	u32 x, y;
	for (y = 0; y < (u32)win->r.h; y++) {
		for (x = 0; x < (u32)win->r.w; x++) {
			u32 index = (y * 4 * win->bufferSize.w) + x * 4;

			u8 red = data[index];
			data[index] = win->buffer[index + 2];
			data[index + 2] = red;
		}
	}
    #elif defined(RGFW_OSMESA)
	u32 y;
	for(y = 0; y < (u32)win->r.h; y++){
		u32 index_from = (y + (win->bufferSize.h - win->r.h)) * 4 * win->bufferSize.w;
		u32 index_to = y * 4 * win->bufferSize.w;
		memcpy(&data[index_to], &data[index_from], 4 * win->bufferSize.w);
	}
    #else
	RGFW_UNUSED(win); RGFW_UNUSED(data);
	#endif
}
#endif

u32 RGFW_isPressedGamepad(RGFW_window* win, u8 c, RGFW_gamepadCodes button) {
	RGFW_UNUSED(win);
	return RGFW_gamepadPressed[c][button].current;
}
u32 RGFW_wasPressedGamepad(RGFW_window* win, u8 c, RGFW_gamepadCodes button) {
	RGFW_UNUSED(win);
	return RGFW_gamepadPressed[c][button].prev;
}
u32 RGFW_isReleasedGamepad(RGFW_window* win, u8 controller, RGFW_gamepadCodes button) {
	RGFW_UNUSED(win);
	return !RGFW_isPressedGamepad(win, controller, button) && RGFW_wasPressedGamepad(win, controller, button);
}
u32 RGFW_isHeldGamepad(RGFW_window* win, u8 controller, RGFW_gamepadCodes button) {
	RGFW_UNUSED(win);
	return RGFW_isPressedGamepad(win, controller, button) && RGFW_wasPressedGamepad(win, controller, button);
}

RGFW_point RGFW_getGamepadAxis(RGFW_window* win, u16 controller, u16 whichAxis) {
	RGFW_UNUSED(win);
	return RGFW_gamepadAxes[controller][whichAxis];
}
const char* RGFW_getGamepadName(RGFW_window* win, u16 controller) {
	RGFW_UNUSED(win);
	return (const char*)RGFW_gamepads_name[controller];
}

size_t RGFW_getGamepadCount(RGFW_window* win) {
	RGFW_UNUSED(win);
	return RGFW_gamepadCount;
}

RGFW_gamepadType RGFW_getGamepadType(RGFW_window* win, u16 controller) {
	RGFW_UNUSED(win);
	return RGFW_gamepads_type[controller];
}

RGFWDEF void RGFW_updateKeyMod(RGFW_window* win, RGFW_keymod mod, RGFW_bool value);
void RGFW_updateKeyMod(RGFW_window* win, RGFW_keymod mod, RGFW_bool value) {
	if (value) win->event.keyMod |= mod;
	else win->event.keyMod &= ~mod;
}

RGFWDEF void RGFW_updateKeyModsPro(RGFW_window* win, RGFW_bool capital, RGFW_bool numlock, RGFW_bool control, RGFW_bool alt, RGFW_bool shift, RGFW_bool super, RGFW_bool scroll);
void RGFW_updateKeyModsPro(RGFW_window* win, RGFW_bool capital, RGFW_bool numlock, RGFW_bool control, RGFW_bool alt, RGFW_bool shift, RGFW_bool super, RGFW_bool scroll) {
	RGFW_updateKeyMod(win, RGFW_modCapsLock, capital);
	RGFW_updateKeyMod(win, RGFW_modNumLock, numlock);
	RGFW_updateKeyMod(win, RGFW_modControl, control);
	RGFW_updateKeyMod(win, RGFW_modAlt, alt);
	RGFW_updateKeyMod(win, RGFW_modShift, shift);
	RGFW_updateKeyMod(win, RGFW_modSuper, super);
	RGFW_updateKeyMod(win, RGFW_modScrollLock, scroll);
}

RGFWDEF void RGFW_updateKeyMods(RGFW_window* win, RGFW_bool capital, RGFW_bool numlock, RGFW_bool scroll);
void RGFW_updateKeyMods(RGFW_window* win, RGFW_bool capital, RGFW_bool numlock, RGFW_bool scroll) {
	RGFW_updateKeyModsPro(win, capital, numlock,
					RGFW_isPressed(win, RGFW_controlL) || RGFW_isPressed(win, RGFW_controlR),
					RGFW_isPressed(win, RGFW_altL) || RGFW_isPressed(win, RGFW_altR),
					RGFW_isPressed(win, RGFW_shiftL) || RGFW_isPressed(win, RGFW_shiftR),
					RGFW_isPressed(win, RGFW_superL) || RGFW_isPressed(win, RGFW_superR),
					scroll);
}

RGFWDEF void RGFW_window_showMouseFlags(RGFW_window* win, RGFW_bool show);
void RGFW_window_showMouseFlags(RGFW_window* win, RGFW_bool show) {
	if (show && (win->_flags & RGFW_windowHideMouse))
		win->_flags ^= RGFW_windowHideMouse;
	else if (!show && !(win->_flags & RGFW_windowHideMouse))
		win->_flags |= RGFW_windowHideMouse;
}

RGFW_bool RGFW_window_mouseHidden(RGFW_window* win) {
	return (RGFW_bool)RGFW_BOOL(win->_flags & RGFW_windowHideMouse);
}

RGFW_bool RGFW_window_borderless(RGFW_window* win) {
	return (RGFW_bool)RGFW_BOOL(win->_flags & RGFW_windowNoBorder);
}

RGFW_bool RGFW_window_isFullscreen(RGFW_window* win){ return RGFW_BOOL(win->_flags & RGFW_windowFullscreen); }
RGFW_bool RGFW_window_allowsDND(RGFW_window* win) { return RGFW_BOOL(win->_flags & RGFW_windowAllowDND); }

void RGFW_window_focusLost(RGFW_window* win) {
    /* standard routines for when a window looses focus */
	_RGFW.root->_flags &= ~(u32)RGFW_windowFocus;
	if ((win->_flags & RGFW_windowFullscreen))
			RGFW_window_minimize(win);

    for (size_t key = 0; key < RGFW_keyLast; key++) {
        if (RGFW_isPressed(NULL, (u8)key) == RGFW_FALSE) continue;
	    RGFW_keyboard[key].current = RGFW_FALSE; 
        u8 keyChar = RGFW_rgfwToKeyChar((u32)key);
        RGFW_keyCallback(win, (u8)key, keyChar, win->event.keyMod, RGFW_FALSE);
        RGFW_eventQueuePushEx(e.type = RGFW_keyReleased;
                                e.key = (u8)key;
                                e.keyChar = keyChar;
                                e.repeat = RGFW_FALSE;
                                e.keyMod = win->event.keyMod;
                                e._win = win);
    }
    
    RGFW_resetKey();
}

#ifndef RGFW_WINDOWS
void RGFW_window_setDND(RGFW_window* win, RGFW_bool allow) {
	RGFW_setBit(&win->_flags, RGFW_windowAllowDND, allow);
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
	if (show == 0)
		RGFW_window_setMouse(win, _RGFW.hiddenMouse);
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

#if defined(RGFW_OPENGL) || defined(RGFW_EGL)

#ifdef RGFW_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#define OEMRESOURCE
	#include <windows.h>
#endif

#if !defined(__APPLE__) && !defined(RGFW_NO_GL_HEADER)
	#include <GL/gl.h>
#elif defined(__APPLE__)
	#ifndef GL_SILENCE_DEPRECATION
		#define GL_SILENCE_DEPRECATION
	#endif
	#include <OpenGL/gl.h>
	#include <OpenGL/OpenGL.h>
#endif

/* EGL, normal OpenGL only */
#ifndef RGFW_EGL
i32 RGFW_GL_HINTS[RGFW_glFinalHint] = {8,
#else
i32 RGFW_GL_HINTS[RGFW_glFinalHint] = {0,
#endif
	0, 0, 0, 1, 8, 8, 8, 8, 24, 0, 0, 0, 0, 0, 0, 0, 0, RGFW_glReleaseNone, RGFW_glCore, 0, 0};

void RGFW_setGLHint(RGFW_glHints hint, i32 value) {
	if (hint < RGFW_glFinalHint && hint) RGFW_GL_HINTS[hint] = value;
}

RGFW_bool RGFW_extensionSupportedStr(const char* extensions, const char* ext, size_t len) {
    const char *start = extensions;
    const char *where; 
    const char* terminator;

    if (extensions == NULL || ext == NULL)
        return RGFW_FALSE;

    where = strstr(extensions, ext);
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

RGFW_bool RGFW_extensionSupported(const char* extension, size_t len) {
    #ifdef GL_NUM_EXTENSIONS
    if (RGFW_GL_HINTS[RGFW_glMajor] >= 3) {
        i32 i;
        GLint count = 0;

        RGFW_proc RGFW_glGetStringi = RGFW_getProcAddress("glGetStringi");
        RGFW_proc RGFW_glGetIntegerv = RGFW_getProcAddress("RGFW_glGetIntegerv");
        if (RGFW_glGetIntegerv) 
            ((void(*)(GLenum, GLint*))RGFW_glGetIntegerv)(GL_NUM_EXTENSIONS, &count);

        for (i = 0; RGFW_glGetStringi && i < count;  i++) {
            const char* en = ((const char* (*)(u32, u32))RGFW_glGetStringi)(GL_EXTENSIONS, (u32)i);
            if (en && RGFW_STRNCMP(en, extension, len) == 0)
                return RGFW_TRUE;
        }
    } else 
#endif
    {
        RGFW_proc RGFW_glGetString = RGFW_getProcAddress("glGetString");
            
        if (RGFW_glGetString) {
            const char* extensions = ((const char*(*)(u32))RGFW_glGetString)(GL_EXTENSIONS);
            if ((extensions != NULL) && RGFW_extensionSupportedStr(extensions, extension, len))
                return RGFW_TRUE;
        }
    }

    return RGFW_extensionSupportedPlatform(extension, len);
}

/* OPENGL normal only (no EGL / OSMesa) */
#if defined(RGFW_OPENGL) && !defined(RGFW_EGL) && !defined(RGFW_CUSTOM_BACKEND) && !defined(RGFW_WASM)

#define RGFW_GL_RENDER_TYPE 		RGFW_OS_BASED_VALUE(GLX_X_VISUAL_TYPE,    	0x2003,		73, 0)
	#define RGFW_GL_ALPHA_SIZE 		RGFW_OS_BASED_VALUE(GLX_ALPHA_SIZE,       	0x201b,		11,     0)
	#define RGFW_GL_DEPTH_SIZE 		RGFW_OS_BASED_VALUE(GLX_DEPTH_SIZE,       	0x2022,		12,     0)
	#define RGFW_GL_DOUBLEBUFFER 		RGFW_OS_BASED_VALUE(GLX_DOUBLEBUFFER,     	0x2011, 	5,  0)
	#define RGFW_GL_STENCIL_SIZE 		RGFW_OS_BASED_VALUE(GLX_STENCIL_SIZE,	 	0x2023,	13,     0)
	#define RGFW_GL_SAMPLES			RGFW_OS_BASED_VALUE(GLX_SAMPLES, 		 	0x2042,	    55,     0)
	#define RGFW_GL_STEREO 			RGFW_OS_BASED_VALUE(GLX_STEREO,	 		 	0x2012,			6,  0)
	#define RGFW_GL_AUX_BUFFERS		RGFW_OS_BASED_VALUE(GLX_AUX_BUFFERS,	    0x2024,	7, 		    0)

#if defined(RGFW_X11) || defined(RGFW_WINDOWS)
	#define RGFW_GL_DRAW 			RGFW_OS_BASED_VALUE(GLX_X_RENDERABLE,	 	0x2001,					0, 0)
	#define RGFW_GL_DRAW_TYPE 		RGFW_OS_BASED_VALUE(GLX_RENDER_TYPE,     	0x2013,						0, 0)
	#define RGFW_GL_FULL_FORMAT		RGFW_OS_BASED_VALUE(GLX_TRUE_COLOR,   	 	0x2027,						0, 0)
	#define RGFW_GL_RED_SIZE		RGFW_OS_BASED_VALUE(GLX_RED_SIZE,         	0x2015,						0, 0)
	#define RGFW_GL_GREEN_SIZE		RGFW_OS_BASED_VALUE(GLX_GREEN_SIZE,       	0x2017,						0, 0)
	#define RGFW_GL_BLUE_SIZE		RGFW_OS_BASED_VALUE(GLX_BLUE_SIZE, 	 		0x2019,						0, 0)
	#define RGFW_GL_USE_RGBA		RGFW_OS_BASED_VALUE(GLX_RGBA_BIT,   	 	0x202B,						0, 0)
	#define RGFW_GL_ACCUM_RED_SIZE 	RGFW_OS_BASED_VALUE(14,   	 	0x201E,						0, 0)
	#define RGFW_GL_ACCUM_GREEN_SIZE RGFW_OS_BASED_VALUE(15,   	 	0x201F,						0, 0)
	#define RGFW_GL_ACCUM_BLUE_SIZE	 RGFW_OS_BASED_VALUE(16,   	 	0x2020,						0, 0)
	#define RGFW_GL_ACCUM_ALPHA_SIZE	 RGFW_OS_BASED_VALUE(17,   	 	0x2021,						0, 0)
	#define RGFW_GL_SRGB	 RGFW_OS_BASED_VALUE(0x20b2,   	 	0x3089,						0, 0)
	#define RGFW_GL_NOERROR	 RGFW_OS_BASED_VALUE(0x31b3,   	 	0x31b3,						0, 0)
	#define RGFW_GL_FLAGS	 RGFW_OS_BASED_VALUE(GLX_CONTEXT_FLAGS_ARB,   	 	0x2094,						0, 0)
	#define RGFW_GL_RELEASE_BEHAVIOR	 RGFW_OS_BASED_VALUE(GLX_CONTEXT_RELEASE_BEHAVIOR_ARB,   	 	0x2097 ,						0, 0)
	#define RGFW_GL_CONTEXT_RELEASE	 RGFW_OS_BASED_VALUE(GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB,   	 	0x2098,						0, 0)
	#define RGFW_GL_CONTEXT_NONE	 RGFW_OS_BASED_VALUE(GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB,   	 	0x0000,						0, 0)
	#define RGFW_GL_FLAGS	 RGFW_OS_BASED_VALUE(GLX_CONTEXT_FLAGS_ARB,   	 	0x2094,						0, 0)
	#define RGFW_GL_DEBUG_BIT	 RGFW_OS_BASED_VALUE(GLX_CONTEXT_FLAGS_ARB,   	 	0x2094,						0, 0)
	#define RGFW_GL_ROBUST_BIT	 RGFW_OS_BASED_VALUE(GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB,   	 	0x00000004,						0, 0)
#endif

#ifdef RGFW_WINDOWS
	#define WGL_SUPPORT_OPENGL_ARB                    0x2010
	#define WGL_COLOR_BITS_ARB                        0x2014
	#define WGL_NUMBER_PIXEL_FORMATS_ARB 			0x2000
	#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
	#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
	#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
	#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
	#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
	#define WGL_SAMPLE_BUFFERS_ARB               0x2041
	#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20a9
	#define WGL_PIXEL_TYPE_ARB                        0x2013
	#define WGL_TYPE_RGBA_ARB                         0x202B

	#define WGL_TRANSPARENT_ARB   					  0x200A
#endif

/*  The window'ing api needs to know how to render the data we (or opengl) give it
	MacOS and Windows do this using a structure called a "pixel format"
	X11 calls it a "Visual"
	This function returns the attributes for the format we want */
i32* RGFW_initFormatAttribs(void);
i32* RGFW_initFormatAttribs(void) {
	static i32 attribs[] = {
							#if defined(RGFW_X11) || defined(RGFW_WINDOWS)
							RGFW_GL_RENDER_TYPE,
							RGFW_GL_FULL_FORMAT,
							RGFW_GL_DRAW, 1,
							RGFW_GL_DRAW_TYPE     , RGFW_GL_USE_RGBA,
							#endif

							#ifdef RGFW_X11
							GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
							#endif

							#ifdef RGFW_MACOS
							72,
							8, 24,
							#endif

							#ifdef RGFW_WINDOWS
							WGL_SUPPORT_OPENGL_ARB,		1,
							WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
							WGL_COLOR_BITS_ARB,	 32,
							#endif
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	size_t index = (sizeof(attribs) / sizeof(attribs[0])) - 27;

	#define RGFW_GL_ADD_ATTRIB(attrib, attVal) \
		if (attVal) { \
			attribs[index] = attrib;\
			attribs[index + 1] = attVal;\
			index += 2;\
		}

		#if defined(RGFW_MACOS) && defined(RGFW_COCOA_GRAPHICS_SWITCHING)
		RGFW_GL_ADD_ATTRIB(96, kCGLPFASupportsAutomaticGraphicsSwitching);
		#endif

        RGFW_GL_ADD_ATTRIB(RGFW_GL_DOUBLEBUFFER, 1);

		RGFW_GL_ADD_ATTRIB(RGFW_GL_ALPHA_SIZE, RGFW_GL_HINTS[RGFW_glAlpha]);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_DEPTH_SIZE, RGFW_GL_HINTS[RGFW_glDepth]);
        RGFW_GL_ADD_ATTRIB(RGFW_GL_STENCIL_SIZE, RGFW_GL_HINTS[RGFW_glStencil]);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_STEREO, RGFW_GL_HINTS[RGFW_glStereo]);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_AUX_BUFFERS, RGFW_GL_HINTS[RGFW_glAuxBuffers]);

	#if defined(RGFW_X11) || defined(RGFW_WINDOWS)
		RGFW_GL_ADD_ATTRIB(RGFW_GL_RED_SIZE, RGFW_GL_HINTS[RGFW_glRed]);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_GREEN_SIZE, RGFW_GL_HINTS[RGFW_glBlue]);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_BLUE_SIZE, RGFW_GL_HINTS[RGFW_glGreen]);
	#endif

	#if defined(RGFW_X11) || defined(RGFW_WINDOWS)
		RGFW_GL_ADD_ATTRIB(RGFW_GL_ACCUM_RED_SIZE, RGFW_GL_HINTS[RGFW_glAccumRed]);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_ACCUM_GREEN_SIZE, RGFW_GL_HINTS[RGFW_glAccumBlue]);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_ACCUM_BLUE_SIZE, RGFW_GL_HINTS[RGFW_glAccumGreen]);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_ACCUM_ALPHA_SIZE, RGFW_GL_HINTS[RGFW_glAccumAlpha]);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_SRGB, RGFW_GL_HINTS[RGFW_glSRGB]);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_NOERROR, RGFW_GL_HINTS[RGFW_glNoError]);

		if (RGFW_GL_HINTS[RGFW_glReleaseBehavior] == RGFW_releaseFlush) {
			RGFW_GL_ADD_ATTRIB(RGFW_GL_RELEASE_BEHAVIOR, RGFW_GL_CONTEXT_RELEASE);
		} else if (RGFW_GL_HINTS[RGFW_glReleaseBehavior] == RGFW_glReleaseNone) {
			RGFW_GL_ADD_ATTRIB(RGFW_GL_RELEASE_BEHAVIOR, RGFW_GL_CONTEXT_NONE);
		}

		i32 flags = 0;
		if (RGFW_GL_HINTS[RGFW_glDebug]) flags |= RGFW_GL_DEBUG_BIT;
		if (RGFW_GL_HINTS[RGFW_glRobustness]) flags |= RGFW_GL_ROBUST_BIT;
		RGFW_GL_ADD_ATTRIB(RGFW_GL_FLAGS, flags);
	#else
		i32 accumSize = (i32)(RGFW_GL_HINTS[RGFW_glAccumRed] + RGFW_GL_HINTS[RGFW_glAccumGreen] +  RGFW_GL_HINTS[RGFW_glAccumBlue] + RGFW_GL_HINTS[RGFW_glAccumAlpha]) / 4;
		RGFW_GL_ADD_ATTRIB(14, accumSize);
	#endif

	#ifndef RGFW_X11
		RGFW_GL_ADD_ATTRIB(RGFW_GL_SAMPLES, RGFW_GL_HINTS[RGFW_glSamples]);
	#endif

	#ifdef RGFW_MACOS
		if (_RGFW.root->_flags & RGFW_windowOpenglSoftware) {
			RGFW_GL_ADD_ATTRIB(70, kCGLRendererGenericFloatID);
		} else {
			attribs[index] = RGFW_GL_RENDER_TYPE;
			index += 1;
		}
	#endif

	#ifdef RGFW_MACOS
		/* macOS has the surface attribs and the opengl attribs connected for some reason
			maybe this is to give macOS more control to limit openGL/the opengl version? */

		attribs[index] = 99;
		attribs[index + 1] = 0x1000;


		if (RGFW_GL_HINTS[RGFW_glMajor] >= 4 || RGFW_GL_HINTS[RGFW_glMajor] >= 3) {
			attribs[index + 1] = (i32) ((RGFW_GL_HINTS[RGFW_glMajor] >= 4) ? 0x4100 : 0x3200);
		}
	#endif

	RGFW_GL_ADD_ATTRIB(0, 0);

	return attribs;
}

/* EGL only (no OSMesa nor normal OPENGL) */
#elif defined(RGFW_EGL)

#include <EGL/egl.h>

#if defined(RGFW_LINK_EGL)
	typedef EGLBoolean(EGLAPIENTRY* PFN_eglInitialize)(EGLDisplay, EGLint*, EGLint*);

	PFNEGLINITIALIZEPROC eglInitializeSource;
	PFNEGLGETCONFIGSPROC eglGetConfigsSource;
	PFNEGLCHOOSECONFIgamepadROC eglChooseConfigSource;
	PFNEGLCREATEWINDOWSURFACEPROC eglCreateWindowSurfaceSource;
	PFNEGLCREATECONTEXTPROC eglCreateContextSource;
	PFNEGLMAKECURRENTPROC eglMakeCurrentSource;
	PFNEGLGETDISPLAYPROC eglGetDisplaySource;
	PFNEGLSWAPBUFFERSPROC eglSwapBuffersSource;
	PFNEGLSWAPINTERVALPROC eglSwapIntervalSource;
	PFNEGLBINDAPIPROC eglBindAPISource;
	PFNEGLDESTROYCONTEXTPROC eglDestroyContextSource;
	PFNEGLTERMINATEPROC eglTerminateSource;
	PFNEGLDESTROYSURFACEPROC eglDestroySurfaceSource;

	#define eglInitialize eglInitializeSource
	#define eglGetConfigs eglGetConfigsSource
	#define eglChooseConfig eglChooseConfigSource
	#define eglCreateWindowSurface eglCreateWindowSurfaceSource
	#define eglCreateContext eglCreateContextSource
	#define eglMakeCurrent eglMakeCurrentSource
	#define eglGetDisplay eglGetDisplaySource
	#define eglSwapBuffers eglSwapBuffersSource
	#define eglSwapInterval eglSwapIntervalSource
	#define eglBindAPI eglBindAPISource
	#define eglDestroyContext eglDestroyContextSource
	#define eglTerminate eglTerminateSource
	#define eglDestroySurface eglDestroySurfaceSource;
#endif


#define EGL_SURFACE_MAJOR_VERSION_KHR 0x3098
#define EGL_SURFACE_MINOR_VERSION_KHR 0x30fb

#ifndef RGFW_GL_ADD_ATTRIB
#define RGFW_GL_ADD_ATTRIB(attrib, attVal) \
	if (attVal) { \
		attribs[index] = attrib;\
		attribs[index + 1] = attVal;\
		index += 2;\
	}
#endif


void RGFW_window_initOpenGL(RGFW_window* win) {
#if defined(RGFW_LINK_EGL)
	eglInitializeSource = (PFNEGLINITIALIZEPROC) eglGetProcAddress("eglInitialize");
	eglGetConfigsSource = (PFNEGLGETCONFIGSPROC) eglGetProcAddress("eglGetConfigs");
	eglChooseConfigSource = (PFNEGLCHOOSECONFIgamepadROC) eglGetProcAddress("eglChooseConfig");
	eglCreateWindowSurfaceSource = (PFNEGLCREATEWINDOWSURFACEPROC) eglGetProcAddress("eglCreateWindowSurface");
	eglCreateContextSource = (PFNEGLCREATECONTEXTPROC) eglGetProcAddress("eglCreateContext");
	eglMakeCurrentSource = (PFNEGLMAKECURRENTPROC) eglGetProcAddress("eglMakeCurrent");
	eglGetDisplaySource = (PFNEGLGETDISPLAYPROC) eglGetProcAddress("eglGetDisplay");
	eglSwapBuffersSource = (PFNEGLSWAPBUFFERSPROC) eglGetProcAddress("eglSwapBuffers");
	eglSwapIntervalSource = (PFNEGLSWAPINTERVALPROC) eglGetProcAddress("eglSwapInterval");
	eglBindAPISource = (PFNEGLBINDAPIPROC) eglGetProcAddress("eglBindAPI");
	eglDestroyContextSource = (PFNEGLDESTROYCONTEXTPROC) eglGetProcAddress("eglDestroyContext");
	eglTerminateSource = (PFNEGLTERMINATEPROC) eglGetProcAddress("eglTerminate");
	eglDestroySurfaceSource = (PFNEGLDESTROYSURFACEPROC) eglGetProcAddress("eglDestroySurface");

	RGFW_ASSERT(eglInitializeSource != NULL &&
	            eglGetConfigsSource != NULL &&
	            eglChooseConfigSource != NULL &&
	            eglCreateWindowSurfaceSource != NULL &&
	            eglCreateContextSource != NULL &&
	            eglMakeCurrentSource != NULL &&
	            eglGetDisplaySource != NULL &&
	            eglSwapBuffersSource != NULL &&
	            eglSwapIntervalsSource != NULL &&
	            eglBindAPISource != NULL &&
	            eglDestroyContextSource != NULL &&
	            eglTerminateSource != NULL &&
	            eglDestroySurfaceSource != NULL);
#endif /* RGFW_LINK_EGL */

#ifdef RGFW_WAYLAND
    if (RGFW_useWaylandBool)
        win->src.eglWindow = wl_egl_window_create(win->src.surface, win->r.w, win->r.h);
#endif

	#ifdef RGFW_WINDOWS
	win->src.EGL_display = eglGetDisplay((EGLNativeDisplayType) win->src.hdc);
	#elif defined(RGFW_MACOS)
	win->src.EGL_display = eglGetDisplay((EGLNativeDisplayType)0);
	#elif defined(RGFW_WAYLAND)
	if (RGFW_useWaylandBool)
		win->src.EGL_display = eglGetDisplay((EGLNativeDisplayType) win->src.wl_display);
    else
    #endif
    #ifdef RGFW_X11
		win->src.EGL_display = eglGetDisplay((EGLNativeDisplayType) win->src.display);
    #else 
    {}
    #endif
    #if !defined(RGFW_WAYLAND) && !defined(RGFW_WINDOWS) && !defined(RGFW_X11) 
        win->src.EGL_display = eglGetDisplay((EGLNativeDisplayType) win->src.display);
	#endif

	EGLint major, minor;

	eglInitialize(win->src.EGL_display, &major, &minor);

	#ifndef EGL_OPENGL_ES1_BIT
	#define EGL_OPENGL_ES1_BIT 0x1
	#endif

	EGLint egl_config[24] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE,
		#ifdef RGFW_OPENGL_ES1
		EGL_OPENGL_ES1_BIT,
		#elif defined(RGFW_OPENGL_ES3)
		EGL_OPENGL_ES3_BIT,
		#elif defined(RGFW_OPENGL_ES2)
		EGL_OPENGL_ES2_BIT,
		#else
		EGL_OPENGL_BIT,
		#endif
		EGL_NONE, EGL_NONE
	};

	{
		size_t index = 7;
		EGLint* attribs = egl_config;

		RGFW_GL_ADD_ATTRIB(EGL_RED_SIZE, RGFW_GL_HINTS[RGFW_glRed]);
		RGFW_GL_ADD_ATTRIB(EGL_GREEN_SIZE, RGFW_GL_HINTS[RGFW_glBlue]);
		RGFW_GL_ADD_ATTRIB(EGL_BLUE_SIZE, RGFW_GL_HINTS[RGFW_glGreen]);
		RGFW_GL_ADD_ATTRIB(EGL_ALPHA_SIZE, RGFW_GL_HINTS[RGFW_glAlpha]);
		RGFW_GL_ADD_ATTRIB(EGL_DEPTH_SIZE, RGFW_GL_HINTS[RGFW_glDepth]);

		if (RGFW_GL_HINTS[RGFW_glSRGB])
			RGFW_GL_ADD_ATTRIB(0x3089, RGFW_GL_HINTS[RGFW_glSRGB]);

		RGFW_GL_ADD_ATTRIB(EGL_NONE, EGL_NONE);
	}

	EGLConfig config;
	EGLint numConfigs;
	eglChooseConfig(win->src.EGL_display, egl_config, &config, 1, &numConfigs);

	#if defined(RGFW_MACOS)
		void* layer = RGFW_cocoaGetLayer();

		RGFW_window_cocoaSetLayer(win, layer);

		win->src.EGL_surface = eglCreateWindowSurface(win->src.EGL_display, config, (EGLNativeWindowType) layer, NULL);
	#elif defined(RGFW_WINDOWS)
		win->src.EGL_surface = eglCreateWindowSurface(win->src.EGL_display, config, (EGLNativeWindowType) win->src.window, NULL);
	#elif defined(RGFW_WAYLAND)
		if (RGFW_useWaylandBool)
			win->src.EGL_surface = eglCreateWindowSurface(win->src.EGL_display, config, (EGLNativeWindowType) win->src.eglWindow, NULL);
		else
    #endif
    #ifdef RGFW_X11
            win->src.EGL_surface = eglCreateWindowSurface(win->src.EGL_display, config, (EGLNativeWindowType) win->src.window, NULL);
    #else
    {}
    #endif
    #if !defined(RGFW_X11) && !defined(RGFW_WAYLAND) && !defined(RGFW_MACOS)
		win->src.EGL_surface = eglCreateWindowSurface(win->src.EGL_display, config, (EGLNativeWindowType) win->src.window, NULL);
	#endif

	EGLint attribs[12];
	size_t index = 0;

#ifdef RGFW_OPENGL_ES1
    RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_CLIENT_VERSION, 1);
#elif defined(RGFW_OPENGL_ES2)
    RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_CLIENT_VERSION, 2);
#elif defined(RGFW_OPENGL_ES3)
    RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_CLIENT_VERSION, 3);
#endif

    RGFW_GL_ADD_ATTRIB(EGL_STENCIL_SIZE, RGFW_GL_HINTS[RGFW_glStencil]);
	RGFW_GL_ADD_ATTRIB(EGL_SAMPLES, RGFW_GL_HINTS[RGFW_glSamples]);

    if (RGFW_GL_HINTS[RGFW_glDoubleBuffer] == 0)
		RGFW_GL_ADD_ATTRIB(EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER);

	if (RGFW_GL_HINTS[RGFW_glMajor]) {
		RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_MAJOR_VERSION, RGFW_GL_HINTS[RGFW_glMajor]);
		RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_MINOR_VERSION, RGFW_GL_HINTS[RGFW_glMinor]);

		if (RGFW_GL_HINTS[RGFW_glProfile] == RGFW_glCore) {
			RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT);
		}
		else {
			RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT);
		}
	}

	RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_OPENGL_ROBUST_ACCESS, RGFW_GL_HINTS[RGFW_glRobustness]);
	RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_OPENGL_DEBUG, RGFW_GL_HINTS[RGFW_glDebug]);
	if (RGFW_GL_HINTS[RGFW_glReleaseBehavior] == RGFW_releaseFlush) {
		RGFW_GL_ADD_ATTRIB(0x2097, 0x2098);
	} else {
		RGFW_GL_ADD_ATTRIB(0x2096, 0x0000);
	}

	RGFW_GL_ADD_ATTRIB(EGL_NONE, EGL_NONE);

	#if defined(RGFW_OPENGL_ES1) || defined(RGFW_OPENGL_ES2) || defined(RGFW_OPENGL_ES3)
	eglBindAPI(EGL_OPENGL_ES_API);
	#else
	eglBindAPI(EGL_OPENGL_API);
	#endif

	win->src.EGL_context = eglCreateContext(win->src.EGL_display, config, EGL_NO_CONTEXT, attribs);

	if (win->src.EGL_context == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errEGLContext, RGFW_DEBUG_CTX(win, 0), "failed to create an EGL opengl context");
		return;
	}

	eglMakeCurrent(win->src.EGL_display, win->src.EGL_surface, win->src.EGL_surface, win->src.EGL_context);
	eglSwapBuffers(win->src.EGL_display, win->src.EGL_surface);
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, RGFW_DEBUG_CTX(win, 0), "EGL opengl context initalized");
}

void RGFW_window_freeOpenGL(RGFW_window* win) {
	if (win->src.EGL_display == NULL) return;

	eglDestroySurface(win->src.EGL_display, win->src.EGL_surface);
	eglDestroyContext(win->src.EGL_display, win->src.EGL_context);
	eglTerminate(win->src.EGL_display);
    win->src.EGL_display = NULL;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, RGFW_DEBUG_CTX(win, 0), "EGL opengl context freed");
}

void RGFW_window_makeCurrent_OpenGL(RGFW_window* win) {
    if (win == NULL)
        eglMakeCurrent(_RGFW.root->src.EGL_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    else {
        eglMakeCurrent(win->src.EGL_display, win->src.EGL_surface, win->src.EGL_surface, win->src.EGL_context);
    }
}

void RGFW_window_swapBuffers_OpenGL(RGFW_window* win) { eglSwapBuffers(win->src.EGL_display, win->src.EGL_surface); }

void* RGFW_getCurrent_OpenGL(void) { return eglGetCurrentContext(); }

#ifdef RGFW_APPLE
void* RGFWnsglFramework = NULL;
#elif defined(RGFW_WINDOWS)
HMODULE RGFW_wgl_dll = NULL;
#endif

RGFW_proc RGFW_getProcAddress(const char* procname) {
	#if defined(RGFW_WINDOWS)
	RGFW_proc proc = (RGFW_proc) GetProcAddress(RGFW_wgl_dll, procname);

		if (proc)
			return proc;
	#endif

	return (RGFW_proc) eglGetProcAddress(procname);
}

RGFW_bool RGFW_extensionSupportedPlatform(const char* extension, size_t len) {
    const char* extensions = eglQueryString(_RGFW.root->src.EGL_display, EGL_EXTENSIONS);
    return extensions != NULL && RGFW_extensionSupportedStr(extensions, extension, len); 
}

void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) {
	RGFW_ASSERT(win != NULL);

	eglSwapInterval(win->src.EGL_display, swapInterval);

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

const char** RGFW_getVKRequiredInstanceExtensions(size_t* count) {
    static const char* arr[2] = {VK_KHR_SURFACE_EXTENSION_NAME};
    arr[1] = RGFW_VK_SURFACE;
    if (count != NULL) *count = 2;

    return (const char**)arr;
}

VkResult RGFW_window_createVKSurface(RGFW_window* win, VkInstance instance, VkSurfaceKHR* surface) {
    RGFW_ASSERT(win != NULL); RGFW_ASSERT(instance);
	RGFW_ASSERT(surface != NULL);

    *surface = VK_NULL_HANDLE;

#ifdef RGFW_X11
    RGFW_GOTO_WAYLAND(0);
    VkXlibSurfaceCreateInfoKHR x11 = { VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR, 0, 0, (Display*) win->src.display, (Window) win->src.window };
    return vkCreateXlibSurfaceKHR(instance, &x11, NULL, surface);
#endif
#if defined(RGFW_WAYLAND)
RGFW_WAYLAND_LABEL
    VkWaylandSurfaceCreateInfoKHR wayland = { VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR, 0, 0, (struct wl_display*) win->src.wl_display, (struct wl_surface*) win->src.surface };
    return vkCreateWaylandSurfaceKHR(instance, &wayland, NULL, surface);
#elif defined(RGFW_WINDOWS)
    VkWin32SurfaceCreateInfoKHR win32 = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, 0, 0, GetModuleHandle(NULL), (HWND)win->src.window };

    return vkCreateWin32SurfaceKHR(instance, &win32, NULL, surface);
#elif defined(RGFW_MACOS) && !defined(RGFW_MACOS_X11)
    void* contentView = ((void* (*)(id, SEL))objc_msgSend)((id)win->src.window, sel_getUid("contentView"));
    VkMacOSSurfaceCreateFlagsMVK macos = { VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK, 0, 0, win->src.display, (void*)contentView };

    return vkCreateMacOSSurfaceMVK(instance, &macos, NULL, surface);
#endif
}


RGFW_bool RGFW_getVKPresentationSupport(VkInstance instance, VkPhysicalDevice physicalDevice, u32 queueFamilyIndex) {
    RGFW_ASSERT(instance);
	if (_RGFW.windowCount == -1 || _RGFW_init == RGFW_FALSE) RGFW_init();
#ifdef RGFW_X11
    RGFW_GOTO_WAYLAND(0);
	Visual* visual = DefaultVisual(_RGFW.display, DefaultScreen(_RGFW.display));
    if (_RGFW.root)
        visual = _RGFW.root->src.visual.visual;

    RGFW_bool out = vkGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, _RGFW.display, XVisualIDFromVisual(visual));
    return out;
#endif
#if defined(RGFW_WAYLAND)
RGFW_WAYLAND_LABEL
    RGFW_bool wlout = vkGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, _RGFW.wl_display);
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


#if (defined(RGFW_WAYLAND) || defined(RGFW_X11)) && !defined(RGFW_NO_LINUX)
	int RGFW_eventWait_forceStop[] = {0, 0, 0}; /* for wait events */

	#if defined(__linux__)
		#include <linux/joystick.h>
		#include <fcntl.h>
		#include <unistd.h>
		#include <errno.h>

		u32 RGFW_linux_updateGamepad(RGFW_window* win);
		u32 RGFW_linux_updateGamepad(RGFW_window* win) {
			/* check for new gamepads */
			static const char* str[] = {"/dev/input/js0", "/dev/input/js1", "/dev/input/js2", "/dev/input/js3", "/dev/input/js4", "/dev/input/js5"};
			static u8 RGFW_rawGamepads[6];
            {
                u16 i;
                for (i = 0; i < 6; i++) {
                    u16 index = RGFW_gamepadCount;
                    if (RGFW_rawGamepads[i]) {
                        struct input_id device_info;
                        if (ioctl(RGFW_rawGamepads[i], EVIOCGID, &device_info) == -2) {
                            if (errno == ENODEV) {
                                RGFW_rawGamepads[i] = 0;
                            }
                        }
                        continue;
                    }

                    i32 js = open(str[i], O_RDONLY);

                    if (js <= 0)
                        break;

                    if (RGFW_gamepadCount >= 4) {
                        close(js);
                        break;
                    }

                    RGFW_rawGamepads[i] = 1;

                    int axes, buttons;
                    if (ioctl(js, JSIOCGAXES, &axes) < 0 || ioctl(js, JSIOCGBUTTONS, &buttons) < 0) {
                        close(js);
                        continue;
                    }

                    if (buttons <= 5 || buttons >= 30) {
                        close(js);
                        continue;
                    }

                    RGFW_gamepadCount++;

                    RGFW_gamepads[index] = js;

                    ioctl(js, JSIOCGNAME(sizeof(RGFW_gamepads_name[index])), RGFW_gamepads_name[index]);
                    RGFW_gamepads_name[index][sizeof(RGFW_gamepads_name[index]) - 1] = 0;

                    u8 j;
                    for (j = 0; j < 16; j++) {
                        RGFW_gamepadPressed[index][j].prev = 0;
                        RGFW_gamepadPressed[index][j].current = 0;
                    }

                    win->event.type = RGFW_gamepadConnected;

                    RGFW_gamepads_type[index] = RGFW_gamepadUnknown;
                    if (RGFW_STRSTR(RGFW_gamepads_name[index], "Microsoft") || RGFW_STRSTR(RGFW_gamepads_name[index], "X-Box"))
                        RGFW_gamepads_type[index] = RGFW_gamepadMicrosoft;
                    else if (RGFW_STRSTR(RGFW_gamepads_name[index], "PlayStation") || RGFW_STRSTR(RGFW_gamepads_name[index], "PS3") || RGFW_STRSTR(RGFW_gamepads_name[index], "PS4") || RGFW_STRSTR(RGFW_gamepads_name[index], "PS5"))
                        RGFW_gamepads_type[index] = RGFW_gamepadSony;
                    else if (RGFW_STRSTR(RGFW_gamepads_name[index], "Nintendo"))
                        RGFW_gamepads_type[index] = RGFW_gamepadNintendo;
                    else if (RGFW_STRSTR(RGFW_gamepads_name[index], "Logitech"))
                        RGFW_gamepads_type[index] = RGFW_gamepadLogitech;

                    win->event.gamepad = index;
                    RGFW_gamepadCallback(win, index, 1);
                    return 1;
                }
            }
			/* check gamepad events */
			u8 i;

			for (i = 0; i < RGFW_gamepadCount; i++) {
				struct js_event e;
				if (RGFW_gamepads[i] == 0)
					continue;

				i32 flags = fcntl(RGFW_gamepads[i], F_GETFL, 0);
				fcntl(RGFW_gamepads[i], F_SETFL, flags | O_NONBLOCK);

				ssize_t bytes;
				while ((bytes = read(RGFW_gamepads[i], &e, sizeof(e))) > 0) {
					switch (e.type) {
						case JS_EVENT_BUTTON: {
							size_t typeIndex = 0;
							if (RGFW_gamepads_type[i] == RGFW_gamepadMicrosoft) typeIndex = 1;
							else if (RGFW_gamepads_type[i] == RGFW_gamepadLogitech) typeIndex = 2;

							win->event.type = e.value ? RGFW_gamepadButtonPressed : RGFW_gamepadButtonReleased;
							u8 RGFW_linux2RGFW[3][RGFW_gamepadR3 + 8] = {{ /* ps */
									RGFW_gamepadA, RGFW_gamepadB, RGFW_gamepadY, RGFW_gamepadX, RGFW_gamepadL1, RGFW_gamepadR1, RGFW_gamepadL2, RGFW_gamepadR2,
									RGFW_gamepadSelect, RGFW_gamepadStart, RGFW_gamepadHome, RGFW_gamepadL3, RGFW_gamepadR3, RGFW_gamepadUp, RGFW_gamepadDown, RGFW_gamepadLeft, RGFW_gamepadRight,
								},{ /* xbox */
									RGFW_gamepadA, RGFW_gamepadB, RGFW_gamepadX, RGFW_gamepadY, RGFW_gamepadL1, RGFW_gamepadR1, RGFW_gamepadSelect, RGFW_gamepadStart,
									RGFW_gamepadHome, RGFW_gamepadL3, RGFW_gamepadR3, 255, 255, RGFW_gamepadUp, RGFW_gamepadDown, RGFW_gamepadLeft, RGFW_gamepadRight
							    },{ /* Logitech */
									RGFW_gamepadA, RGFW_gamepadB, RGFW_gamepadX, RGFW_gamepadY, RGFW_gamepadL1, RGFW_gamepadR1, RGFW_gamepadL2, RGFW_gamepadR2,
									RGFW_gamepadSelect, RGFW_gamepadStart, RGFW_gamepadHome, RGFW_gamepadL3, RGFW_gamepadR3, RGFW_gamepadUp, RGFW_gamepadDown, RGFW_gamepadLeft, RGFW_gamepadRight
								}
							};

							win->event.button = RGFW_linux2RGFW[typeIndex][e.number];
							win->event.gamepad = i;
							if (win->event.button == 255) break;

							RGFW_gamepadPressed[i][win->event.button].prev = RGFW_gamepadPressed[i][win->event.button].current;
							RGFW_gamepadPressed[i][win->event.button].current = RGFW_BOOL(e.value);
							RGFW_gamepadButtonCallback(win, i, win->event.button, RGFW_BOOL(e.value));

							return 1;
						}
						case JS_EVENT_AXIS: {
							size_t axis = e.number / 2;
							if (axis == 2) axis = 1;

							ioctl(RGFW_gamepads[i], JSIOCGAXES, &win->event.axisesCount);
							win->event.axisesCount = 2;

							if (axis < 3) {
								if (e.number == 0 || e.number == 3)
									RGFW_gamepadAxes[i][axis].x = (i32)((e.value / 32767.0f) * 100);
								else if (e.number == 1 || e.number == 4) {
									RGFW_gamepadAxes[i][axis].y = (i32)((e.value / 32767.0f) * 100);
								}
							}

							win->event.axis[axis] = RGFW_gamepadAxes[i][axis];
							win->event.type = RGFW_gamepadAxisMove;
							win->event.gamepad = i;
							win->event.whichAxis = (u8)axis;
							RGFW_gamepadAxisCallback(win, i, win->event.axis, win->event.axisesCount, win->event.whichAxis);
							return 1;
						}
						default: break;
					}
				}
				if (bytes == -1 && errno == ENODEV) {
					RGFW_gamepadCount--;
					close(RGFW_gamepads[i]);
					RGFW_gamepads[i] = 0;

					win->event.type = RGFW_gamepadDisconnected;
					win->event.gamepad = i;
					RGFW_gamepadCallback(win, i, 0);
					return 1;
				}
			}
			return 0;
		}

	#endif
#endif



/*

	Start of Wayland defines


*/

#ifdef RGFW_WAYLAND
/*
Wayland TODO: (out of date)
- fix RGFW_keyPressed lock state

	RGFW_windowMoved, 		the window was moved (by the user)
	RGFW_windowResized  	the window was resized (by the user), [on WASM this means the browser was resized]
	RGFW_windowRefresh	 	The window content needs to be refreshed

	RGFW_DND 				a file has been dropped into the window
	RGFW_DNDInit

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

RGFW_window* RGFW_key_win = NULL;

/* wayland global garbage (wayland bad, X11 is fine (ish) (not really)) */
#include "xdg-shell.h"
#include "xdg-decoration-unstable-v1.h"

struct xkb_context *xkb_context;
struct xkb_keymap *keymap = NULL;
struct xkb_state *xkb_state = NULL;
enum zxdg_toplevel_decoration_v1_mode client_preferred_mode, RGFW_current_mode;
struct zxdg_decoration_manager_v1 *decoration_manager = NULL;

struct wl_cursor_theme* RGFW_wl_cursor_theme = NULL;
struct wl_surface* RGFW_cursor_surface = NULL;
struct wl_cursor_image* RGFW_cursor_image = NULL;

void xdg_wm_base_ping_handler(void *data,
        struct xdg_wm_base *wm_base, uint32_t serial)
{
	RGFW_UNUSED(data);
    xdg_wm_base_pong(wm_base, serial);
}

const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping_handler,
};

RGFW_bool RGFW_wl_configured = 0;

void xdg_surface_configure_handler(void *data,
        struct xdg_surface *xdg_surface, uint32_t serial)
{
	RGFW_UNUSED(data);
    xdg_surface_ack_configure(xdg_surface, serial);
	RGFW_wl_configured = 1;
}

const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler,
};

void xdg_toplevel_configure_handler(void *data,
        struct xdg_toplevel *toplevel, int32_t width, int32_t height,
        struct wl_array *states)
{
	RGFW_UNUSED(data); RGFW_UNUSED(toplevel); RGFW_UNUSED(states);
    RGFW_UNUSED(width); RGFW_UNUSED(height);
}

void xdg_toplevel_close_handler(void *data,
        struct xdg_toplevel *toplevel)
{
	RGFW_UNUSED(data);
	RGFW_window* win = (RGFW_window*)xdg_toplevel_get_user_data(toplevel);
	if (win == NULL)
		win = RGFW_key_win;

	RGFW_eventQueuePushEx(e.type = RGFW_quit; e._win = win);
	RGFW_windowQuitCallback(win);
}

void shm_format_handler(void *data,
        struct wl_shm *shm, uint32_t format)
{
	RGFW_UNUSED(data); RGFW_UNUSED(shm); RGFW_UNUSED(format);
}

const struct wl_shm_listener shm_listener = {
    .format = shm_format_handler,
};

const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure_handler,
    .close = xdg_toplevel_close_handler,
};

RGFW_window* RGFW_mouse_win = NULL;

void pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	RGFW_UNUSED(data); RGFW_UNUSED(pointer); RGFW_UNUSED(serial); RGFW_UNUSED(surface_x); RGFW_UNUSED(surface_y);
	RGFW_window* win = (RGFW_window*)wl_surface_get_user_data(surface);
	RGFW_mouse_win = win;

	RGFW_eventQueuePushEx(e.type = RGFW_mouseEnter;
									e.point = RGFW_POINT(wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
									e._win = win);

	RGFW_mouseNotifyCallback(win, win->event.point, RGFW_TRUE);
}
void pointer_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {
	RGFW_UNUSED(data); RGFW_UNUSED(pointer); RGFW_UNUSED(serial); RGFW_UNUSED(surface);
	RGFW_window* win = (RGFW_window*)wl_surface_get_user_data(surface);
	if (RGFW_mouse_win == win)
		RGFW_mouse_win = NULL;

	RGFW_eventQueuePushEx(e.type = RGFW_mouseLeave;
									e.point = win->event.point;
									e._win = win);

	RGFW_mouseNotifyCallback(win,  win->event.point, RGFW_FALSE);
}
void pointer_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
	RGFW_UNUSED(data); RGFW_UNUSED(pointer); RGFW_UNUSED(time); RGFW_UNUSED(x); RGFW_UNUSED(y);

	RGFW_ASSERT(RGFW_mouse_win != NULL);
	RGFW_eventQueuePushEx(e.type = RGFW_mousePosChanged;
									e.point = RGFW_POINT(wl_fixed_to_double(x), wl_fixed_to_double(y));
									e._win = RGFW_mouse_win);

	RGFW_mousePosCallback(RGFW_mouse_win, RGFW_POINT(wl_fixed_to_double(x), wl_fixed_to_double(y)), RGFW_mouse_win->event.vector);
}
void pointer_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
	RGFW_UNUSED(data); RGFW_UNUSED(pointer); RGFW_UNUSED(time); RGFW_UNUSED(serial);
	RGFW_ASSERT(RGFW_mouse_win != NULL);

	u32 b = (button - 0x110);

	/* flip right and middle button codes */
	if (b == 1) b = 2;
	else if (b == 2) b = 1;

	RGFW_mouseButtons[b].prev = RGFW_mouseButtons[b].current;
	RGFW_mouseButtons[b].current = RGFW_BOOL(state);

	RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonReleased - RGFW_BOOL(state);
									e.point = RGFW_mouse_win->event.point;
									e.button = (u8)b;
									e._win = RGFW_mouse_win);
	RGFW_mouseButtonCallback(RGFW_mouse_win, (u8)b, 0, RGFW_BOOL(state));
}
void pointer_axis(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
	RGFW_UNUSED(data); RGFW_UNUSED(pointer); RGFW_UNUSED(time);  RGFW_UNUSED(axis);
	RGFW_ASSERT(RGFW_mouse_win != NULL);

	double scroll = - wl_fixed_to_double(value);

	RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonPressed;
									e.point = RGFW_mouse_win->event.point;
									e.button = RGFW_mouseScrollUp + (scroll < 0);
									e.scroll = scroll;
									e._win = RGFW_mouse_win);

	RGFW_mouseButtonCallback(RGFW_mouse_win, RGFW_mouseScrollUp + (scroll < 0), scroll, 1);
}

void RGFW_doNothing(void) { }

void keyboard_keymap (void *data, struct wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size) {
	RGFW_UNUSED(data); RGFW_UNUSED(keyboard); RGFW_UNUSED(format);

	char *keymap_string = mmap (NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	xkb_keymap_unref (keymap);
	keymap = xkb_keymap_new_from_string (xkb_context, keymap_string, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);

	munmap (keymap_string, size);
	close (fd);
	xkb_state_unref (xkb_state);
	xkb_state = xkb_state_new (keymap);
}
void keyboard_enter (void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
	RGFW_UNUSED(data); RGFW_UNUSED(keyboard); RGFW_UNUSED(serial); RGFW_UNUSED(keys);

	RGFW_key_win = (RGFW_window*)wl_surface_get_user_data(surface);

	RGFW_key_win->_flags |= RGFW_windowFocus;
	RGFW_eventQueuePushEx(e.type = RGFW_focusIn; e._win = RGFW_key_win);
	RGFW_focusCallback(RGFW_key_win, RGFW_TRUE);

	if ((RGFW_key_win->_flags & RGFW_HOLD_MOUSE)) RGFW_window_mouseHold(RGFW_key_win, RGFW_AREA(RGFW_key_win->r.w, RGFW_key_win->r.h));
}
void keyboard_leave (void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface) {
	RGFW_UNUSED(data); RGFW_UNUSED(keyboard); RGFW_UNUSED(serial);

	RGFW_window* win = (RGFW_window*)wl_surface_get_user_data(surface);
	if (RGFW_key_win == win)
		RGFW_key_win = NULL;

	RGFW_eventQueuePushEx(e.type = RGFW_focusOut; e._win = win);
	RGFW_focusCallback(win, RGFW_FALSE);
    RGFW_window_focusLost(win);
}
void keyboard_key (void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	RGFW_UNUSED(data); RGFW_UNUSED(keyboard); RGFW_UNUSED(serial); RGFW_UNUSED(time);

	if (RGFW_key_win == NULL) return;

	xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkb_state, key + 8);

	u32 RGFWkey = RGFW_apiKeyToRGFW(key + 8);
	RGFW_keyboard[RGFWkey].prev = RGFW_keyboard[RGFWkey].current;
	RGFW_keyboard[RGFWkey].current = RGFW_BOOL(state);

	RGFW_eventQueuePushEx(e.type = (u8)(RGFW_keyPressed + state);
									e.key = (u8)RGFWkey;
									e.keyChar = (u8)keysym;
									e.repeat = RGFW_isHeld(RGFW_key_win, (u8)RGFWkey);
									e._win = RGFW_key_win);

	RGFW_updateKeyMods(RGFW_key_win, RGFW_BOOL(xkb_keymap_mod_get_index(keymap, "Lock")), RGFW_BOOL(xkb_keymap_mod_get_index(keymap, "Mod2")), RGFW_BOOL(xkb_keymap_mod_get_index(keymap, "ScrollLock")));
	RGFW_keyCallback(RGFW_key_win, (u8)RGFWkey, (u8)keysym, RGFW_key_win->event.keyMod, RGFW_BOOL(state));
}
void keyboard_modifiers (void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
	RGFW_UNUSED(data); RGFW_UNUSED(keyboard); RGFW_UNUSED(serial); RGFW_UNUSED(time);
	xkb_state_update_mask (xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}
struct wl_keyboard_listener keyboard_listener = {&keyboard_keymap, &keyboard_enter, &keyboard_leave, &keyboard_key, &keyboard_modifiers, (void (*)(void *, struct wl_keyboard *, 
int,  int))&RGFW_doNothing};

void seat_capabilities (void *data, struct wl_seat *seat, uint32_t capabilities) {
	RGFW_UNUSED(data);
    static struct wl_pointer_listener pointer_listener = {&pointer_enter, &pointer_leave, &pointer_motion, &pointer_button, &pointer_axis, (void (*)(void *, struct wl_pointer *))&RGFW_doNothing, (void (*)(void *, struct wl_pointer *, uint32_t))&RGFW_doNothing, (void (*)(void *, struct wl_pointer *, uint32_t, uint32_t))&RGFW_doNothing, (void (*)(void *, struct wl_pointer *, uint32_t, int32_t))&RGFW_doNothing, (void (*)(void *, struct wl_pointer *, uint32_t, int32_t))&RGFW_doNothing, (void (*)(void*, struct wl_pointer*, uint32_t, uint32_t))&RGFW_doNothing};

    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
		struct wl_pointer *pointer = wl_seat_get_pointer (seat);
		wl_pointer_add_listener (pointer, &pointer_listener, NULL);
	}
	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
		struct wl_keyboard *keyboard = wl_seat_get_keyboard (seat);
		wl_keyboard_add_listener (keyboard, &keyboard_listener, NULL);
	}
}
struct wl_seat_listener seat_listener = {&seat_capabilities, (void (*)(void *, struct wl_seat *, const char *))&RGFW_doNothing};

void wl_global_registry_handler(void *data,
		struct wl_registry *registry, uint32_t id, const char *interface,
		uint32_t version)
{
	RGFW_window* win = (RGFW_window*)data;
	RGFW_UNUSED(version);
    if (RGFW_STRNCMP(interface, "wl_compositor", 16) == 0) {
		win->src.compositor = wl_registry_bind(registry,
			id, &wl_compositor_interface, 4);
	} else if (RGFW_STRNCMP(interface, "xdg_wm_base", 12) == 0) {
		win->src.xdg_wm_base = wl_registry_bind(registry,
		id, &xdg_wm_base_interface, 1);
	} else if (RGFW_STRNCMP(interface, zxdg_decoration_manager_v1_interface.name, 255) == 0) {
		decoration_manager = wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1);
    } else if (RGFW_STRNCMP(interface, "wl_shm", 7) == 0) {
        win->src.shm = wl_registry_bind(registry,
            id, &wl_shm_interface, 1);
        wl_shm_add_listener(win->src.shm, &shm_listener, NULL);
	} else if (RGFW_STRNCMP(interface,"wl_seat", 8) == 0) {
		win->src.seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
		wl_seat_add_listener(win->src.seat, &seat_listener, NULL);
	}
}

void wl_global_registry_remove(void *data, struct wl_registry *registry, uint32_t name) { RGFW_UNUSED(data); RGFW_UNUSED(registry); RGFW_UNUSED(name); }
const struct wl_registry_listener registry_listener = {
	.global = wl_global_registry_handler,
	.global_remove = wl_global_registry_remove,
};

void decoration_handle_configure(void *data,
		struct zxdg_toplevel_decoration_v1 *decoration,
		enum zxdg_toplevel_decoration_v1_mode mode) {
	RGFW_UNUSED(data); RGFW_UNUSED(decoration);
	RGFW_current_mode = mode;
}

const struct zxdg_toplevel_decoration_v1_listener decoration_listener = {
	.configure = decoration_handle_configure,
};

void randname(char *buf) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;

    int i;
    for (i = 0; i < 6; ++i) {
		buf[i] = (char)('A'+(r&15)+(r&16)*2);
		r >>= 5;
	}
}

size_t wl_stringlen(char* name) {
	size_t i = 0;
    while (name[i]) { i++; }
	return i;
}

int anonymous_shm_open(void) {
	char name[] = "/RGFW-wayland-XXXXXX";
	int retries = 100;

	do {
		randname(name + wl_stringlen(name) - 6);

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

int create_shm_file(off_t size) {
	int fd = anonymous_shm_open();
	if (fd < 0) {
		return fd;
	}

	if (ftruncate(fd, size) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

void wl_surface_frame_done(void *data, struct wl_callback *cb, uint32_t time) {
	RGFW_UNUSED(data); RGFW_UNUSED(cb); RGFW_UNUSED(time);

	#ifdef RGFW_BUFFER
		RGFW_window* win = (RGFW_window*)data;
		wl_surface_attach(win->src.surface, win->src.wl_buffer, 0, 0);
		wl_surface_damage_buffer(win->src.surface, 0, 0, win->r.w, win->r.h);
		wl_surface_commit(win->src.surface);
	#endif
}

const struct wl_callback_listener wl_surface_frame_listener = {
	.done = wl_surface_frame_done,
};
#endif /* RGFW_WAYLAND */
/*
	End of Wayland defines
*/

/*


Start of Linux / Unix defines


*/

#ifdef RGFW_UNIX
#if !defined(RGFW_NO_X11_CURSOR) && defined(RGFW_X11)
#include <X11/Xcursor/Xcursor.h>
#endif

#include <dlfcn.h>

#ifndef RGFW_NO_DPI
#include <X11/extensions/Xrandr.h>
#include <X11/Xresource.h>
#endif

#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <X11/extensions/sync.h>
#include <unistd.h>

#include <X11/XKBlib.h> /* for converting keycode to string */
#include <X11/cursorfont.h> /* for hiding */
#include <X11/extensions/shapeconst.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XInput2.h>

#include <limits.h> /* for data limits (mainly used in drag and drop functions) */
#include <poll.h>

/* atoms needed for drag and drop */
Atom XdndAware, XtextPlain, XtextUriList;
Atom RGFW_XUTF8_STRING = 0;

Atom wm_delete_window = 0, RGFW_XCLIPBOARD = 0;

#if defined(RGFW_X11) && !defined(RGFW_NO_X11_CURSOR) && !defined(RGFW_NO_X11_CURSOR_PRELOAD)
	typedef XcursorImage* (*PFN_XcursorImageCreate)(int, int);
	typedef void (*PFN_XcursorImageDestroy)(XcursorImage*);
	typedef Cursor(*PFN_XcursorImageLoadCursor)(Display*, const XcursorImage*);
#endif
#if defined(RGFW_OPENGL) && defined(RGFW_X11)
	typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
#endif

#if !defined(RGFW_NO_X11_XI_PRELOAD) && defined(RGFW_X11)
	typedef int (* PFN_XISelectEvents)(Display*,Window,XIEventMask*,int);
	PFN_XISelectEvents XISelectEventsSRC = NULL;
	#define XISelectEvents XISelectEventsSRC

	void* X11Xihandle = NULL;
#endif

#if !defined(RGFW_NO_X11_EXT_PRELOAD) && defined(RGFW_X11)
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

#if !defined(RGFW_NO_X11_CURSOR) && !defined(RGFW_NO_X11_CURSOR_PRELOAD) && defined(RGFW_X11)
	PFN_XcursorImageLoadCursor XcursorImageLoadCursorSRC = NULL;
	PFN_XcursorImageCreate XcursorImageCreateSRC = NULL;
	PFN_XcursorImageDestroy XcursorImageDestroySRC = NULL;

	#define XcursorImageLoadCursor XcursorImageLoadCursorSRC
	#define XcursorImageCreate XcursorImageCreateSRC
	#define XcursorImageDestroy XcursorImageDestroySRC

	void* X11Cursorhandle = NULL;
#endif

#ifdef RGFW_X11
const char* RGFW_instName = NULL;
void RGFW_setXInstName(const char* name) { RGFW_instName = name; }
#endif

#if defined(RGFW_OPENGL) && !defined(RGFW_EGL)
RGFW_bool RGFW_extensionSupportedPlatform(const char * extension, size_t len) {
    const char* extensions = glXQueryExtensionsString(_RGFW.display, XDefaultScreen(_RGFW.display));
    return (extensions != NULL) && RGFW_extensionSupportedStr(extensions, extension, len); 
}
RGFW_proc RGFW_getProcAddress(const char* procname) { return (RGFW_proc) glXGetProcAddress((GLubyte*) procname); }
#endif

void RGFW_window_initBufferPtr(RGFW_window* win, u8* buffer, RGFW_area area) {
	RGFW_GOTO_WAYLAND(0);

#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
	win->buffer = (u8*)buffer;
	win->bufferSize = area;

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoBuffer, RGFW_DEBUG_CTX(win, 0), "createing a 4 channel buffer");
	#ifdef RGFW_X11
		#ifdef RGFW_OSMESA
				win->src.ctx = OSMesaCreateContext(OSMESA_BGRA, NULL);
				OSMesaMakeCurrent(win->src.ctx, win->buffer, GL_UNSIGNED_BYTE, area.w, area.h);
				OSMesaPixelStore(OSMESA_Y_UP, 0);
		#endif

		win->src.bitmap = XCreateImage(
			win->src.display, win->src.visual.visual, (u32)win->src.visual.depth,
			ZPixmap, 0, NULL, area.w, area.h, 32, 0
		);
	#endif
	#ifdef RGFW_WAYLAND
		RGFW_WAYLAND_LABEL {}
		u32 size = (u32)(win->r.w * win->r.h * 4);
		int fd = create_shm_file(size);
		if (fd < 0) {
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errBuffer, RGFW_DEBUG_CTX(win, (u32)fd),"Failed to create a buffer.");
			exit(1);
        }

		win->src.buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (win->src.buffer == MAP_FAILED) {
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errBuffer, RGFW_DEBUG_CTX(win, 0), "mmap failed!");
			close(fd);
			exit(1);
		}

		win->_flags |= RGFW_BUFFER_ALLOC;

		struct wl_shm_pool* pool = wl_shm_create_pool(win->src.shm, fd, (i32)size);
		win->src.wl_buffer = wl_shm_pool_create_buffer(pool, 0, win->r.w, win->r.h, win->r.w * 4,
			WL_SHM_FORMAT_ARGB8888);
		wl_shm_pool_destroy(pool);

		close(fd);

		wl_surface_attach(win->src.surface, win->src.wl_buffer, 0, 0);
		wl_surface_commit(win->src.surface);

		u8 color[] = {0x00, 0x00, 0x00, 0xFF};

		size_t i;
		for (i = 0; i < area.w * area.h * 4; i += 4) {
			RGFW_MEMCPY(&win->buffer[i], color, 4);
		}

		RGFW_MEMCPY(win->src.buffer, win->buffer, (size_t)(win->r.w * win->r.h * 4));

		#if defined(RGFW_OSMESA)
				win->src.ctx = OSMesaCreateContext(OSMESA_BGRA, NULL);
				OSMesaMakeCurrent(win->src.ctx, win->buffer, GL_UNSIGNED_BYTE, area.w, area.h);
				OSMesaPixelStore(OSMESA_Y_UP, 0);
		#endif
	#endif
#else
	#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL{}
	#endif

	RGFW_UNUSED(win); RGFW_UNUSED(buffer); RGFW_UNUSED(area);
#endif
}

#define RGFW_LOAD_ATOM(name) \
	static Atom name = 0; \
	if (name == 0) name = XInternAtom(_RGFW.display, #name, False);

void RGFW_window_setBorder(RGFW_window* win, RGFW_bool border) {
	RGFW_setBit(&win->_flags, RGFW_windowNoBorder, !border);

	RGFW_GOTO_WAYLAND(0);
	#ifdef RGFW_X11
	RGFW_LOAD_ATOM(_MOTIF_WM_HINTS);

	struct __x11WindowHints {
		unsigned long flags, functions, decorations, status;
		long input_mode;
	} hints;
	hints.flags = 2;
	hints.decorations = border;

	XChangeProperty(win->src.display, win->src.window, _MOTIF_WM_HINTS, _MOTIF_WM_HINTS, 32,
				PropModeReplace, (u8*)&hints, 5
	);

	if (RGFW_window_isHidden(win) == 0) {
		RGFW_window_hide(win);
		RGFW_window_show(win);
	}

	#endif
	#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
    RGFW_UNUSED(win); RGFW_UNUSED(border);
	#endif
}

void RGFW_releaseCursor(RGFW_window* win) {
RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	XUngrabPointer(win->src.display, CurrentTime);

	/* disable raw input */
	unsigned char mask[] = { 0 };
	XIEventMask em;
	em.deviceid = XIAllMasterDevices;
	em.mask_len = sizeof(mask);
	em.mask = mask;

	XISelectEvents(win->src.display, XDefaultRootWindow(win->src.display), &em, 1);
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
    RGFW_UNUSED(win);
#endif
}

void RGFW_captureCursor(RGFW_window* win, RGFW_rect r) {
RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	/* enable raw input */
	unsigned char mask[XIMaskLen(XI_RawMotion)] = { 0 };
	XISetMask(mask, XI_RawMotion);

	XIEventMask em;
	em.deviceid = XIAllMasterDevices;
	em.mask_len = sizeof(mask);
	em.mask = mask;

	XISelectEvents(win->src.display, XDefaultRootWindow(win->src.display), &em, 1);

	XGrabPointer(win->src.display, win->src.window, True, PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	RGFW_window_moveMouse(win, RGFW_POINT(win->r.x + (i32)(r.w / 2), win->r.y + (i32)(r.h / 2)));
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
    RGFW_UNUSED(win); RGFW_UNUSED(r);
#endif
}

#define RGFW_LOAD_LIBRARY(x, lib) if (x == NULL) x = dlopen(lib, RTLD_LAZY | RTLD_LOCAL)
#define RGFW_PROC_DEF(proc, name) if (name##SRC == NULL && proc != NULL) { \
	void* ptr = dlsym(proc, #name); \
	if (ptr != NULL) memcpy(&name##SRC, &ptr, sizeof(PFN_##name)); \
}

#ifdef RGFW_X11
void RGFW_window_getVisual(RGFW_window* win) {
#if defined(RGFW_OPENGL) && !defined(RGFW_EGL)
		i32* visual_attribs = RGFW_initFormatAttribs();
		i32 fbcount;
		GLXFBConfig* fbc = glXChooseFBConfig(win->src.display, DefaultScreen(win->src.display), visual_attribs, &fbcount);

		i32 best_fbc = -1;
		i32 best_depth = 0;
		i32 best_samples = 0;

		if (fbcount == 0) {
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(win, 0), "Failed to find any valid GLX visual configs");
			return;
		}

        i32 i;
		for (i = 0; i < fbcount; i++) {
			XVisualInfo* vi = glXGetVisualFromFBConfig(win->src.display, fbc[i]);
			if (vi == NULL)
				continue;

			i32 samp_buf, samples;
			glXGetFBConfigAttrib(win->src.display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
			glXGetFBConfigAttrib(win->src.display, fbc[i], GLX_SAMPLES, &samples);

			if (best_fbc == -1) best_fbc = i;
			if ((!(win->_flags & RGFW_windowTransparent) || vi->depth == 32)  && best_depth == 0) {
				best_fbc = i;
				best_depth = vi->depth;
			}
			if ((!(win->_flags & RGFW_windowTransparent) || vi->depth == 32) && samples <= RGFW_GL_HINTS[RGFW_glSamples] && samples > best_samples) {
				best_fbc = i;
				best_depth = vi->depth;
				best_samples = samples;
			}
			XFree(vi);
		}

		if (best_fbc == -1) {
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(win, 0), "Failed to get a valid GLX visual");
			return;
        }		

		win->src.bestFbc = fbc[best_fbc];
		XVisualInfo* vi = glXGetVisualFromFBConfig(win->src.display, win->src.bestFbc);
		if (vi->depth != 32 && (win->_flags & RGFW_windowTransparent))
			RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningOpenGL, RGFW_DEBUG_CTX(win, 0), "Failed to to find a matching visual with a 32-bit depth");

		if (best_samples < RGFW_GL_HINTS[RGFW_glSamples])
			RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningOpenGL, RGFW_DEBUG_CTX(win, 0), "Failed to load matching sampiling");

        int configCaveat;
        if (glXGetFBConfigAttrib(win->src.display, win->src.bestFbc, GLX_CONFIG_CAVEAT, &configCaveat) == Success &&
            configCaveat == GLX_SLOW_CONFIG) {
                win->_flags |= RGFW_windowOpenglSoftware;
        }

		XFree(fbc);
        win->src.visual = *vi;
		XFree(vi);
#else
	win->src.visual.visual = DefaultVisual(win->src.display, DefaultScreen(win->src.display));
	win->src.visual.depth = DefaultDepth(win->src.display, DefaultScreen(win->src.display));
	if (win->_flags & RGFW_windowTransparent) {
		XMatchVisualInfo(win->src.display, DefaultScreen(win->src.display), 32, TrueColor, &win->src.visual); /*!< for RGBA backgrounds */
		if (win->src.visual.depth != 32)
		RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningOpenGL, RGFW_DEBUG_CTX(win, 0), "Failed to load a 32-bit depth");
	}
#endif
}
#endif
#ifndef RGFW_EGL
void RGFW_window_initOpenGL(RGFW_window* win) {
#ifdef RGFW_OPENGL
        i32 context_attribs[7] = { 0, 0, 0, 0, 0, 0, 0 };
		context_attribs[0] = GLX_CONTEXT_PROFILE_MASK_ARB;
		if (RGFW_GL_HINTS[RGFW_glProfile] == RGFW_glCore)
			context_attribs[1] = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
		else
			context_attribs[1] = GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;

		if (RGFW_GL_HINTS[RGFW_glMinor] || RGFW_GL_HINTS[RGFW_glMajor]) {
			context_attribs[2] = GLX_CONTEXT_MAJOR_VERSION_ARB;
			context_attribs[3] = RGFW_GL_HINTS[RGFW_glMajor];
			context_attribs[4] = GLX_CONTEXT_MINOR_VERSION_ARB;
			context_attribs[5] = RGFW_GL_HINTS[RGFW_glMinor];
		}

		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
		glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
			glXGetProcAddressARB((GLubyte*) "glXCreateContextAttribsARB");

		GLXContext ctx = NULL;
		if (_RGFW.root != NULL && _RGFW.root != win) {
			ctx = _RGFW.root->src.ctx;
            RGFW_window_makeCurrent_OpenGL(_RGFW.root);
        }

		if (glXCreateContextAttribsARB == NULL) {
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(win, 0), "failed to load proc address 'glXCreateContextAttribsARB', loading a generic opengl context");
			win->src.ctx = glXCreateContext(win->src.display, &win->src.visual, ctx, True);
		}
		else {
				win->src.ctx = glXCreateContextAttribsARB(win->src.display, win->src.bestFbc, ctx, True, context_attribs);
				XSync(win->src.display, False);
				if (win->src.ctx == NULL) {
					RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(win, 0), "failed to create an opengl context with AttribsARB, loading a generic opengl context");
					win->src.ctx = glXCreateContext(win->src.display, &win->src.visual, ctx, True);
            }
		}
        
        glXMakeCurrent(win->src.display, (Drawable) win->src.window, (GLXContext) win->src.ctx);    
        RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, RGFW_DEBUG_CTX(win, 0), "opengl context initalized");
#else
	RGFW_UNUSED(win);
#endif
}

void RGFW_window_freeOpenGL(RGFW_window* win) {
#ifdef RGFW_OPENGL
	if (win->src.ctx == NULL) return;
	glXDestroyContext(win->src.display, win->src.ctx);
	win->src.ctx = NULL;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, RGFW_DEBUG_CTX(win, 0), "opengl context freed");
#else
RGFW_UNUSED(win);
#endif
}
#endif


i32 RGFW_init(void) {
    RGFW_GOTO_WAYLAND(1);
#if defined(RGFW_C89) || defined(__cplusplus)
    if (_RGFW_init) return 0; 
    _RGFW_init = RGFW_TRUE;    
    _RGFW.root = NULL; _RGFW.current = NULL; _RGFW.windowCount = -1; _RGFW.eventLen = 0; _RGFW.eventIndex = 0;
#endif

#ifdef RGFW_X11
    if (_RGFW.windowCount != -1) return 0;
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
    _RGFW.display = XOpenDisplay(0);
    XSetWindowAttributes wa;
    RGFW_MEMSET(&wa, 0, sizeof(wa));
    wa.event_mask = PropertyChangeMask;
    _RGFW.helperWindow = XCreateWindow(_RGFW.display, XDefaultRootWindow(_RGFW.display), 0, 0, 1, 1, 0, 0,
                                        InputOnly, DefaultVisual(_RGFW.display, DefaultScreen(_RGFW.display)), CWEventMask, &wa);

    _RGFW.windowCount = 0;
    u8 RGFW_blk[] = { 0, 0, 0, 0 };
	_RGFW.hiddenMouse = RGFW_loadMouse(RGFW_blk, RGFW_AREA(1, 1), 4);
	_RGFW.clipboard = NULL;

    XkbComponentNamesRec rec;
    XkbDescPtr desc = XkbGetMap(_RGFW.display, 0, XkbUseCoreKbd);
    XkbDescPtr evdesc;
    u8 old[sizeof(RGFW_keycodes) / sizeof(RGFW_keycodes[0])];

    XkbGetNames(_RGFW.display, XkbKeyNamesMask, desc);

    RGFW_MEMSET(&rec, 0, sizeof(rec));
    rec.keycodes = (char*)"evdev";
    evdesc = XkbGetKeyboardByName(_RGFW.display, XkbUseCoreKbd, &rec, XkbGBN_KeyNamesMask, XkbGBN_KeyNamesMask, False);
    /* memo: RGFW_keycodes[x11 keycode] = rgfw keycode */
    if(evdesc != NULL && desc != NULL){
        for(int i = 0; i < (int)sizeof(RGFW_keycodes) / (int)sizeof(RGFW_keycodes[0]); i++){
    	    old[i] = RGFW_keycodes[i];
    	    RGFW_keycodes[i] = 0;
        }
        for(int i = evdesc->min_key_code; i <= evdesc->max_key_code; i++){
    	    for(int j = desc->min_key_code; j <= desc->max_key_code; j++){
    		if(strncmp(evdesc->names->keys[i].name, desc->names->keys[j].name, XkbKeyNameLength) == 0){
    			RGFW_keycodes[j] = old[i];
    			break;
    		}
    	    }
        }
	XkbFreeKeyboard(desc, 0, True);
	XkbFreeKeyboard(evdesc, 0, True);
    }
#endif
#ifdef RGFW_WAYLAND
RGFW_WAYLAND_LABEL
    _RGFW.wl_display = wl_display_connect(NULL);
#endif
    _RGFW.windowCount = 0;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal, RGFW_DEBUG_CTX(NULL, 0), "global context initialized");
    return 0;
}


RGFW_window* RGFW_createWindowPtr(const char* name, RGFW_rect rect, RGFW_windowFlags flags, RGFW_window* win) {
	RGFW_window_basic_init(win, rect, flags);

#ifdef RGFW_WAYLAND
	win->src.compositor = NULL;
#endif
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	i64 event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask | LeaveWindowMask | EnterWindowMask | ExposureMask; /*!< X11 events accepted */

    win->src.display = XOpenDisplay(NULL);
    RGFW_window_getVisual(win);

    /* make X window attrubutes */
	XSetWindowAttributes swa;
    RGFW_MEMSET(&swa, 0, sizeof(swa));

	Colormap cmap;
	swa.colormap = cmap = XCreateColormap(win->src.display,
		DefaultRootWindow(win->src.display),
		win->src.visual.visual, AllocNone);
	swa.event_mask = event_mask;

	/* create the window */
    win->src.window = XCreateWindow(win->src.display, DefaultRootWindow(win->src.display), win->r.x, win->r.y, (u32)win->r.w, (u32)win->r.h,
		0, win->src.visual.depth, InputOutput, win->src.visual.visual,
		CWColormap | CWBorderPixel | CWEventMask, &swa);

	XFreeColors(win->src.display, cmap, NULL, 0, 0);

	win->src.gc = XCreateGC(win->src.display, win->src.window, 0, NULL);

	/* In your .desktop app, if you set the property
	    StartupWMClass=RGFW that will assoicate the launcher icon
	     with your application - robrohan */
	if (RGFW_className == NULL)
		RGFW_className = (char*)name;

	XClassHint hint;
	hint.res_class = (char*)RGFW_className;
	if (RGFW_instName == NULL)	hint.res_name = (char*)name;
	else 						hint.res_name = (char*)RGFW_instName;
	XSetClassHint(win->src.display, win->src.window, &hint);

	#ifndef RGFW_NO_MONITOR
	if (flags & RGFW_windowScaleToMonitor)
		RGFW_window_scaleToMonitor(win);
	#endif
	XSelectInput(win->src.display, (Drawable) win->src.window, event_mask); /*!< tell X11 what events we want */

	/* make it so the user can't close the window until the program does */
	if (wm_delete_window == 0) {
		wm_delete_window = XInternAtom(win->src.display, "WM_DELETE_WINDOW", False);
		RGFW_XUTF8_STRING = XInternAtom(win->src.display, "UTF8_STRING", False);
		RGFW_XCLIPBOARD = XInternAtom(win->src.display, "CLIPBOARD", False);
	}

	XSetWMProtocols(win->src.display, (Drawable) win->src.window, &wm_delete_window, 1);
	/* set the background */
	RGFW_window_setName(win, name);

	XMoveWindow(win->src.display, (Drawable) win->src.window, win->r.x, win->r.y); /*!< move the window to it's proper cords */

	if (flags & RGFW_windowAllowDND) { /* init drag and drop atoms and turn on drag and drop for this window */
		win->_flags |= RGFW_windowAllowDND;

		/* actions */
		XtextUriList = XInternAtom(win->src.display, "text/uri-list", False);
		XtextPlain = XInternAtom(win->src.display, "text/plain", False);
		XdndAware = XInternAtom(win->src.display, "XdndAware", False);
		const u8 version = 5;

		XChangeProperty(win->src.display, win->src.window,
			XdndAware, 4, 32,
			PropModeReplace, &version, 1); /*!< turns on drag and drop */
	}

#ifdef RGFW_ADVANCED_SMOOTH_RESIZE
    RGFW_LOAD_ATOM(_NET_WM_SYNC_REQUEST_COUNTER)
    RGFW_LOAD_ATOM(_NET_WM_SYNC_REQUEST)
    Atom protcols[2] = {_NET_WM_SYNC_REQUEST, wm_delete_window};
    XSetWMProtocols(win->src.display, win->src.window, protcols, 2);

    XSyncValue initial_value;
    XSyncIntToValue(&initial_value, 0);
    win->src.counter = XSyncCreateCounter(win->src.display, initial_value);

    XChangeProperty(win->src.display, win->src.window, _NET_WM_SYNC_REQUEST_COUNTER, XA_CARDINAL, 32, PropModeReplace, (uint8_t*)&win->src.counter, 1);
#endif

	if ((flags & RGFW_windowNoInitAPI) == 0) {
		RGFW_window_initOpenGL(win);
        RGFW_window_initBuffer(win);
    }

    RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, RGFW_DEBUG_CTX(win, 0), "a new window was created");
	RGFW_window_setMouseDefault(win);
	RGFW_window_setFlags(win, flags);
    
    win->src.r = win->r;

	RGFW_window_show(win);
    return win; /*return newly created window */
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
	RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningWayland, RGFW_DEBUG_CTX(win, 0), "RGFW Wayland support is experimental");

	win->src.wl_display = _RGFW.wl_display;
	if (win->src.wl_display == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errWayland, RGFW_DEBUG_CTX(win, 0), "Failed to load Wayland display");
		#ifdef RGFW_X11
			RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningWayland, RGFW_DEBUG_CTX(win, 0), "Falling back to X11");
			RGFW_useWayland(0);
			return RGFW_createWindowPtr(name, rect, flags, win);
		#endif
		return NULL;
	}


	#ifdef RGFW_X11
        win->src.display = _RGFW.display;
		win->src.window = _RGFW.helperWindow;
        XMapWindow(_RGFW.display, win->src.window);
		XFlush(win->src.display);
		if (wm_delete_window == 0) {
			wm_delete_window = XInternAtom(win->src.display, "WM_DELETE_WINDOW", False);
			RGFW_XUTF8_STRING = XInternAtom(win->src.display, "UTF8_STRING", False);
			RGFW_XCLIPBOARD = XInternAtom(win->src.display, "CLIPBOARD", False);
		}
	#endif

	struct wl_registry *registry = wl_display_get_registry(win->src.wl_display);
	wl_registry_add_listener(registry, &registry_listener, win);

	wl_display_roundtrip(win->src.wl_display);
	wl_display_dispatch(win->src.wl_display);

	if (win->src.compositor == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errWayland, RGFW_DEBUG_CTX(win, 0), "Can't find compositor.");
		return NULL;
	}

	if (RGFW_wl_cursor_theme == NULL) {
		RGFW_wl_cursor_theme = wl_cursor_theme_load(NULL, 24, win->src.shm);
		RGFW_cursor_surface = wl_compositor_create_surface(win->src.compositor);

		struct wl_cursor* cursor = wl_cursor_theme_get_cursor(RGFW_wl_cursor_theme, "left_ptr");
		RGFW_cursor_image = cursor->images[0];
		struct wl_buffer* cursor_buffer	= wl_cursor_image_get_buffer(RGFW_cursor_image);

		wl_surface_attach(RGFW_cursor_surface, cursor_buffer, 0, 0);
		wl_surface_commit(RGFW_cursor_surface);
	}

	xdg_wm_base_add_listener(win->src.xdg_wm_base, &xdg_wm_base_listener, NULL);

	xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

	win->src.surface = wl_compositor_create_surface(win->src.compositor);
	wl_surface_set_user_data(win->src.surface, win);

	win->src.xdg_surface = xdg_wm_base_get_xdg_surface(win->src.xdg_wm_base, win->src.surface);
	xdg_surface_add_listener(win->src.xdg_surface, &xdg_surface_listener, NULL);

	xdg_wm_base_set_user_data(win->src.xdg_wm_base, win);

	win->src.xdg_toplevel = xdg_surface_get_toplevel(win->src.xdg_surface);
	xdg_toplevel_set_user_data(win->src.xdg_toplevel, win);
	xdg_toplevel_add_listener(win->src.xdg_toplevel, &xdg_toplevel_listener, NULL);

	xdg_surface_set_window_geometry(win->src.xdg_surface, 0, 0, win->r.w, win->r.h);

	if (!(flags & RGFW_windowNoBorder)) {
		win->src.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
					decoration_manager, win->src.xdg_toplevel);
	}

	wl_display_roundtrip(win->src.wl_display);

	wl_surface_commit(win->src.surface);
	RGFW_window_show(win);

	/* wait for the surface to be configured */
	while (wl_display_dispatch(win->src.wl_display) != -1 && !RGFW_wl_configured) { }

    if ((flags & RGFW_windowNoInitAPI) == 0) {
        RGFW_window_initOpenGL(win);
        RGFW_window_initBuffer(win);
    }
	struct wl_callback* callback = wl_surface_frame(win->src.surface);
	wl_callback_add_listener(callback, &wl_surface_frame_listener, win);
	wl_surface_commit(win->src.surface);
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, RGFW_DEBUG_CTX(win, 0), "a new window was created");

	#ifndef RGFW_NO_MONITOR
	if (flags & RGFW_windowScaleToMonitor)
		RGFW_window_scaleToMonitor(win);
	#endif
    
    RGFW_window_setName(win, name);
	RGFW_window_setMouseDefault(win);
	RGFW_window_setFlags(win, flags);
	return win; /* return newly created window */
#endif
}

RGFW_area RGFW_getScreenSize(void) {
	RGFW_GOTO_WAYLAND(1);
	RGFW_init();

	#ifdef RGFW_X11
	Screen* scrn = DefaultScreenOfDisplay(_RGFW.display);
	return RGFW_AREA(scrn->width, scrn->height);
	#endif
	#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL return RGFW_AREA(_RGFW.root->r.w, _RGFW.root->r.h); /* TODO */
	#endif
}

RGFW_point RGFW_getGlobalMousePoint(void) {
	RGFW_init();
	RGFW_point RGFWMouse = RGFW_POINT(0, 0);
    RGFW_GOTO_WAYLAND(1);
#ifdef RGFW_X11
	i32 x, y;
	u32 z;
	Window window1, window2;
	XQueryPointer(_RGFW.display, XDefaultRootWindow(_RGFW.display), &window1, &window2, &RGFWMouse.x, &RGFWMouse.y, &x, &y, &z);
	return RGFWMouse;
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL
    return RGFWMouse;
#endif
}

RGFWDEF void RGFW_XHandleClipboardSelection(XEvent* event);
void RGFW_XHandleClipboardSelection(XEvent* event) { RGFW_UNUSED(event);
#ifdef RGFW_X11
    RGFW_LOAD_ATOM(ATOM_PAIR);
	RGFW_LOAD_ATOM(MULTIPLE);
	RGFW_LOAD_ATOM(TARGETS);
	RGFW_LOAD_ATOM(SAVE_TARGETS);

    const XSelectionRequestEvent* request = &event->xselectionrequest;
    const Atom formats[] = { RGFW_XUTF8_STRING, XA_STRING };
    const int formatCount = sizeof(formats) / sizeof(formats[0]);

    if (request->target == TARGETS) {
        const Atom targets[] = { TARGETS, MULTIPLE, RGFW_XUTF8_STRING, XA_STRING };

        XChangeProperty(_RGFW.display, request->requestor, request->property,
                        XA_ATOM, 32, PropModeReplace, (u8*) targets, sizeof(targets) / sizeof(Atom));
    }  else if (request->target == MULTIPLE) {
		Atom* targets = NULL;

		Atom actualType = 0;
		int actualFormat = 0;
		unsigned long count = 0, bytesAfter = 0;

		XGetWindowProperty(_RGFW.display, request->requestor, request->property, 0, LONG_MAX,
							False, ATOM_PAIR, &actualType, &actualFormat, &count, &bytesAfter, (u8**) &targets);

		unsigned long i;
		for (i = 0; i < (u32)count; i += 2) {
			if (targets[i] == RGFW_XUTF8_STRING || targets[i] == XA_STRING)
				XChangeProperty(_RGFW.display, request->requestor, targets[i + 1], targets[i],
					8, PropModeReplace, (const unsigned char *)_RGFW.clipboard, (i32)_RGFW.clipboard_len);
			else
				targets[i + 1] = None;
		}

		XChangeProperty(_RGFW.display,
			request->requestor, request->property, ATOM_PAIR, 32,
			PropModeReplace, (u8*) targets, (i32)count);

		XFlush(_RGFW.display);
		XFree(targets);
	} else if (request->target == SAVE_TARGETS)
        XChangeProperty(_RGFW.display, request->requestor, request->property, 0, 32, PropModeReplace, NULL, 0);
    else {
        int i;
        for (i = 0;  i < formatCount;  i++) {
			if (request->target != formats[i])
				continue;
			XChangeProperty(_RGFW.display, request->requestor, request->property, request->target,
								8, PropModeReplace, (u8*) _RGFW.clipboard, (i32)_RGFW.clipboard_len);
		}
	}

    XEvent reply = { SelectionNotify };
    reply.xselection.property = request->property;
    reply.xselection.display = request->display;
    reply.xselection.requestor = request->requestor;
    reply.xselection.selection = request->selection;
    reply.xselection.target = request->target;
    reply.xselection.time = request->time;

    XSendEvent(_RGFW.display, request->requestor, False, 0, &reply);
#endif
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

i32 RGFW_XHandleClipboardSelectionHelper(void);


u8 RGFW_rgfwToKeyChar(u32 key) {
    u32 keycode = RGFW_rgfwToApiKey(key);
    RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
    Window root = DefaultRootWindow(_RGFW.display);
    Window ret_root, ret_child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    XQueryPointer(_RGFW.display, root, &ret_root, &ret_child, &root_x, &root_y, &win_x, &win_y, &mask);
    KeySym sym = (KeySym)XkbKeycodeToKeysym(_RGFW.display, (KeyCode)keycode, 0, (KeyCode)mask & ShiftMask ? 1 : 0);

    if ((mask & LockMask) && sym >= XK_a && sym <= XK_z)
        sym = (mask & ShiftMask) ? sym + 32 : sym - 32;
    if ((u8)sym != (u32)sym)
        sym = 0;

    return (u8)sym;
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL RGFW_UNUSED(keycode);
    return (u8)key;
#endif
}

RGFW_event* RGFW_window_checkEvent(RGFW_window* win) {
    RGFW_XHandleClipboardSelectionHelper();

    if (win == NULL || ((win->_flags & RGFW_windowFreeOnClose) && (win->_flags & RGFW_EVENT_QUIT))) return NULL;
    RGFW_event* ev = RGFW_window_checkEventCore(win);
	if (ev) return ev;

	#if defined(__linux__) && !defined(RGFW_NO_LINUX)
		if (RGFW_linux_updateGamepad(win)) return &win->event;
	#endif
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
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
	XPending(win->src.display);

	XEvent E; /*!< raw X11 event */

    /* if there is no unread qued events, get a new one */
	if ((QLength(win->src.display) || XEventsQueued(win->src.display, QueuedAlready) + XEventsQueued(win->src.display, QueuedAfterReading))
		&& win->event.type != RGFW_quit
	)
		XNextEvent(win->src.display, &E);
	else {
		return NULL;
	}

	win->event.type = 0;

	/* xdnd data */
	static Window source = 0;
	static long version = 0;
	static i32 format = 0;

	XEvent reply = { ClientMessage };

	switch (E.type) {
	case KeyPress:
	case KeyRelease: {
		win->event.repeat = RGFW_FALSE;
		/* check if it's a real key release */
		if (E.type == KeyRelease && XEventsQueued(win->src.display, QueuedAfterReading)) { /* get next event if there is one */
			XEvent NE;
			XPeekEvent(win->src.display, &NE);

			if (E.xkey.time == NE.xkey.time && E.xkey.keycode == NE.xkey.keycode) /* check if the current and next are both the same */
				win->event.repeat = RGFW_TRUE;
		}

		/* set event key data */
		win->event.key = (u8)RGFW_apiKeyToRGFW(E.xkey.keycode);
		win->event.keyChar = (u8)RGFW_rgfwToKeyChar(win->event.key);

		RGFW_keyboard[win->event.key].prev = RGFW_keyboard[win->event.key].current;

		/* get keystate data */
		win->event.type = (E.type == KeyPress) ? RGFW_keyPressed : RGFW_keyReleased;

        XKeyboardState keystate;
		XGetKeyboardControl(win->src.display, &keystate);

		RGFW_keyboard[win->event.key].current = (E.type == KeyPress);

		XkbStateRec state;
		XkbGetState(win->src.display, XkbUseCoreKbd, &state);
		RGFW_updateKeyMods(win, (state.locked_mods & LockMask), (state.locked_mods & Mod2Mask), (state.locked_mods & Mod3Mask));

		RGFW_keyCallback(win, win->event.key, win->event.keyChar, win->event.keyMod, (E.type == KeyPress));
		break;
	}
	case ButtonPress:
	case ButtonRelease:
		if (E.xbutton.button > RGFW_mouseFinal) { /* skip this event */
			XFlush(win->src.display);
			return RGFW_window_checkEvent(win);
		}

		win->event.type = RGFW_mouseButtonPressed + (E.type == ButtonRelease); /* the events match */
		win->event.button = (u8)(E.xbutton.button - 1);
		switch(win->event.button) {
			case RGFW_mouseScrollUp:
				win->event.scroll = 1;
				break;
			case RGFW_mouseScrollDown:
				win->event.scroll = -1;
				break;
			default: break;
		}

		RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;

		if (win->event.repeat == RGFW_FALSE)
			win->event.repeat = RGFW_isPressed(win, win->event.key);

		RGFW_mouseButtons[win->event.button].current = (E.type == ButtonPress);
		RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, (E.type == ButtonPress));
		break;

	case MotionNotify:
		win->event.point.x = E.xmotion.x;
		win->event.point.y = E.xmotion.y;

		win->event.vector.x = win->event.point.x - win->_lastMousePoint.x;
		win->event.vector.y = win->event.point.y - win->_lastMousePoint.y;
		win->_lastMousePoint = win->event.point;

		win->event.type = RGFW_mousePosChanged;
		RGFW_mousePosCallback(win, win->event.point, win->event.vector);
		break;

	case GenericEvent: {
		/* MotionNotify is used for mouse events if the mouse isn't held */
		if (!(win->_flags & RGFW_HOLD_MOUSE)) {
			XFreeEventData(win->src.display, &E.xcookie);
			break;
		}

		XGetEventData(win->src.display, &E.xcookie);
		if (E.xcookie.evtype == XI_RawMotion) {
			XIRawEvent *raw = (XIRawEvent *)E.xcookie.data;
			if (raw->valuators.mask_len == 0) {
				XFreeEventData(win->src.display, &E.xcookie);
				break;
			}

			double deltaX = 0.0f;
			double deltaY = 0.0f;

			/* check if relative motion data exists where we think it does */
			if (XIMaskIsSet(raw->valuators.mask, 0) != 0)
				deltaX += raw->raw_values[0];
			if (XIMaskIsSet(raw->valuators.mask, 1) != 0)
				deltaY += raw->raw_values[1];

			win->event.vector = RGFW_POINT((i32)deltaX, (i32)deltaY);
			win->event.point.x = win->_lastMousePoint.x + win->event.vector.x;
			win->event.point.y = win->_lastMousePoint.y + win->event.vector.y;
			win->_lastMousePoint = win->event.point;

			RGFW_window_moveMouse(win, RGFW_POINT(win->r.x + (win->r.w / 2), win->r.y + (win->r.h / 2)));

			win->event.type = RGFW_mousePosChanged;
			RGFW_mousePosCallback(win, win->event.point, win->event.vector);
		}

		XFreeEventData(win->src.display, &E.xcookie);
		break;
	}

	case Expose: {
		win->event.type = RGFW_windowRefresh;
		RGFW_windowRefreshCallback(win);

#ifdef RGFW_ADVANCED_SMOOTH_RESIZE
        XSyncValue value;
        XSyncIntToValue(&value, (i32)win->src.counter_value);
        XSyncSetCounter(win->src.display, win->src.counter, value);
#endif
        break;
    }
	case MapNotify: case UnmapNotify: 		RGFW_window_checkMode(win); break;
	case ClientMessage: {
		/* if the client closed the window */
		if (E.xclient.data.l[0] == (long)wm_delete_window) {
			win->event.type = RGFW_quit;
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
            XSyncSetCounter(win->src.display, win->src.counter, value);
            break;
        }
#endif
        if ((win->_flags & RGFW_windowAllowDND) == 0)
			break;

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
					win->src.display, source, XdndTypeList,
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
				win->src.display, XDefaultRootWindow(win->src.display), win->src.window,
				xabs, yabs, &xpos, &ypos, &dummy
			);

			win->event.point.x = xpos;
			win->event.point.y = ypos;

			reply.xclient.window = source;
			reply.xclient.message_type = XdndStatus;

			if (format) {
				reply.xclient.data.l[1] = 1;
				if (version >= 2)
					reply.xclient.data.l[4] = (long)XdndActionCopy;
			}

			XSendEvent(win->src.display, source, False, NoEventMask, &reply);
			XFlush(win->src.display);
			break;
		}
		if (E.xclient.message_type != XdndDrop)
			break;

		if (version > 5)
			break;

	        size_t i;
		for (i = 0; i < win->event.droppedFilesCount; i++)
			win->event.droppedFiles[i][0] = '\0';

		win->event.droppedFilesCount = 0;


		win->event.type = RGFW_DNDInit;

		if (format) {
			Time time = (version >= 1)
				? (Time)E.xclient.data.l[2]
				: CurrentTime;

			XConvertSelection(
				win->src.display, XdndSelection, (Atom)format,
				XdndSelection, win->src.window, time
			);
		} else if (version >= 2) {
			XEvent new_reply = { ClientMessage };

			XSendEvent(win->src.display, source, False, NoEventMask, &new_reply);
			XFlush(win->src.display);
		}

		RGFW_dndInitCallback(win, win->event.point);
	} break;
	case SelectionRequest:
		RGFW_XHandleClipboardSelection(&E);
		XFlush(win->src.display);
		return RGFW_window_checkEvent(win);
	case SelectionNotify: {
		/* this is only for checking for xdnd drops */
        if (E.xselection.property != XdndSelection || !(win->_flags & RGFW_windowAllowDND))
			break;
		char* data;
		unsigned long result;

		Atom actualType;
		i32 actualFormat;
		unsigned long bytesAfter;

		XGetWindowProperty(win->src.display, E.xselection.requestor, E.xselection.property, 0, LONG_MAX, False, E.xselection.target, &actualType, &actualFormat, &result, &bytesAfter, (u8**) &data);

		if (result == 0)
			break;

		const char* prefix = (const char*)"file://";

		char* line;

		win->event.droppedFilesCount = 0;
		win->event.type = RGFW_DND;

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

			win->event.droppedFilesCount++;

			size_t index = 0;
			while (*line) {
				if (line[0] == '%' && line[1] && line[2]) {
					const char digits[3] = { line[1], line[2], '\0' };
					path[index] = (char) RGFW_STRTOL(digits, NULL, 16);
					line += 2;
				} else
					path[index] = *line;

				index++;
				line++;
			}
			path[index] = '\0';
			RGFW_MEMCPY(win->event.droppedFiles[win->event.droppedFilesCount - 1], path, index + 1);
		}

		RGFW_dndCallback(win, win->event.droppedFiles, win->event.droppedFilesCount);
		if (data)
			XFree(data);

		if (version >= 2) {
            XEvent new_reply = { ClientMessage };
            new_reply.xclient.window = source;
            new_reply.xclient.message_type = XdndFinished;
            new_reply.xclient.format = 32;
			new_reply.xclient.data.l[1] = (long int)result;
			new_reply.xclient.data.l[2] = (long int)XdndActionCopy;
            XSendEvent(win->src.display, source, False, NoEventMask, &new_reply);
			XFlush(win->src.display);
		}
		break;
	}
	case FocusIn:
		if ((win->_flags & RGFW_windowFullscreen))
			XMapRaised(win->src.display, win->src.window);

		win->_flags |= RGFW_windowFocus;
		win->event.type = RGFW_focusIn;
		RGFW_focusCallback(win, 1);


	    if ((win->_flags & RGFW_HOLD_MOUSE)) RGFW_window_mouseHold(win, RGFW_AREA(win->r.w, win->r.h));
        break;
	case FocusOut:
        win->event.type = RGFW_focusOut;
		RGFW_focusCallback(win, 0);
        RGFW_window_focusLost(win);
		break;
	case PropertyNotify: RGFW_window_checkMode(win); break;
	case EnterNotify: {
		win->event.type = RGFW_mouseEnter;
		win->event.point.x = E.xcrossing.x;
		win->event.point.y = E.xcrossing.y;
		RGFW_mouseNotifyCallback(win, win->event.point, 1);
		break;
	}

	case LeaveNotify: {
		win->event.type = RGFW_mouseLeave;
		RGFW_mouseNotifyCallback(win, win->event.point, 0);
		break;
	}

	case ConfigureNotify: {
		/* detect resize */
		RGFW_window_checkMode(win);
		if (E.xconfigure.width != win->src.r.w || E.xconfigure.height != win->src.r.h) {
			win->event.type = RGFW_windowResized;
			win->src.r = win->r = RGFW_RECT(win->src.r.x, win->src.r.y, E.xconfigure.width, E.xconfigure.height);
			RGFW_windowResizedCallback(win, win->r);
			break;
		}

		/* detect move */
		if (E.xconfigure.x != win->src.r.x || E.xconfigure.y != win->src.r.y) {
			win->event.type = RGFW_windowMoved;
            win->src.r = win->r = RGFW_RECT(E.xconfigure.x, E.xconfigure.y, win->src.r.w, win->src.r.h);
			RGFW_windowMovedCallback(win, win->r);
			break;
		}

		break;
	}
	default:
		XFlush(win->src.display);
		return RGFW_window_checkEvent(win);
	}
	XFlush(win->src.display);
	if (win->event.type) return &win->event;
	else return NULL;
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
	if ((win->_flags & RGFW_windowHide) == 0)
        wl_display_roundtrip(win->src.wl_display);
	return NULL;
#endif
}

void RGFW_window_move(RGFW_window* win, RGFW_point v) {
	RGFW_ASSERT(win != NULL);
	win->r.x = v.x;
	win->r.y = v.y;
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	XMoveWindow(win->src.display, win->src.window, v.x, v.y);
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
	RGFW_ASSERT(win != NULL);

	if (win->src.compositor) {
		struct wl_pointer *pointer = wl_seat_get_pointer(win->src.seat);
			if (!pointer) {
				return;
			}

		wl_display_flush(win->src.wl_display);
	}
#endif
}


void RGFW_window_resize(RGFW_window* win, RGFW_area a) {
	RGFW_ASSERT(win != NULL);
	win->r.w = (i32)a.w;
	win->r.h = (i32)a.h;
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	XResizeWindow(win->src.display, win->src.window, a.w, a.h);

	if ((win->_flags & RGFW_windowNoResize)) {
		XSizeHints sh;
		sh.flags = (1L << 4) | (1L << 5);
		sh.min_width = sh.max_width = (i32)a.w;
		sh.min_height = sh.max_height = (i32)a.h;

		XSetWMSizeHints(win->src.display, (Drawable) win->src.window, &sh, XA_WM_NORMAL_HINTS);
	}
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
	if (win->src.compositor) {
		xdg_surface_set_window_geometry(win->src.xdg_surface, 0, 0, win->r.w, win->r.h);
		#ifdef RGFW_OPENGL
		wl_egl_window_resize(win->src.eglWindow, (i32)a.w, (i32)a.h, 0, 0);
		#endif
	}
#endif
}

void RGFW_window_setAspectRatio(RGFW_window* win, RGFW_area a) {
	RGFW_ASSERT(win != NULL);
    RGFW_GOTO_WAYLAND(0);

	if (a.w == 0 && a.h == 0)
		return;
#ifdef RGFW_X11
	XSizeHints hints;
	long flags;

	XGetWMNormalHints(win->src.display, win->src.window, &hints, &flags);

	hints.flags |= PAspect;

	hints.min_aspect.x = hints.max_aspect.x = (i32)a.w;
	hints.min_aspect.y = hints.max_aspect.y = (i32)a.h;

	XSetWMNormalHints(win->src.display, win->src.window, &hints);
    return;
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL 
#endif
}

void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a) {
	RGFW_ASSERT(win != NULL);
    RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
    long flags;
	XSizeHints hints;
    RGFW_MEMSET(&hints, 0, sizeof(XSizeHints));

	XGetWMNormalHints(win->src.display, win->src.window, &hints, &flags);

	hints.flags |= PMinSize;

	hints.min_width = (i32)a.w;
	hints.min_height = (i32)a.h;

	XSetWMNormalHints(win->src.display, win->src.window, &hints);
    return;
#endif
#ifdef RGFW_WAYLAND
RGFW_WAYLAND_LABEL RGFW_UNUSED(a);
#endif
}

void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a) {
	RGFW_ASSERT(win != NULL);
    RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
    long flags;
	XSizeHints hints;
    RGFW_MEMSET(&hints, 0, sizeof(XSizeHints));

	XGetWMNormalHints(win->src.display, win->src.window, &hints, &flags);

	hints.flags |= PMaxSize;

	hints.max_width = (i32)a.w;
	hints.max_height = (i32)a.h;

	XSetWMNormalHints(win->src.display, win->src.window, &hints);
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL RGFW_UNUSED(a);
#endif
}

#ifdef RGFW_X11
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

	XSendEvent(win->src.display, DefaultRootWindow(win->src.display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}
#endif

void RGFW_window_maximize(RGFW_window* win) {
	win->_oldRect = win->r;
    RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11 
    RGFW_toggleXMaximized(win, 1);
    return;
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL
    return;
#endif
}

void RGFW_window_focus(RGFW_window* win) {
	RGFW_ASSERT(win);
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
    XWindowAttributes attr;
    XGetWindowAttributes(win->src.display, win->src.window, &attr);
    if (attr.map_state != IsViewable) return;

	XSetInputFocus(win->src.display, win->src.window, RevertToPointerRoot, CurrentTime);
	XFlush(win->src.display);
#endif
#ifdef RGFW_WAYLAND
RGFW_WAYLAND_LABEL;
#endif
}

void RGFW_window_raise(RGFW_window* win) {
	RGFW_ASSERT(win);
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
    XRaiseWindow(win->src.display, win->src.window);
	XMapRaised(win->src.display, win->src.window);
#endif
#ifdef RGFW_WAYLAND
RGFW_WAYLAND_LABEL;
#endif
}

#ifdef RGFW_X11
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

    XSendEvent(win->src.display, DefaultRootWindow(win->src.display), False, SubstructureNotifyMask | SubstructureRedirectMask, &xev);
}
#endif

void RGFW_window_setFullscreen(RGFW_window* win, RGFW_bool fullscreen) {
	RGFW_ASSERT(win != NULL);
	RGFW_GOTO_WAYLAND(0);
    if (fullscreen) {
		win->_flags |= RGFW_windowFullscreen;
		win->_oldRect = win->r;
	}
	else win->_flags &= ~(u32)RGFW_windowFullscreen;
#ifdef RGFW_X11
	RGFW_LOAD_ATOM(_NET_WM_STATE_FULLSCREEN);

	RGFW_window_setXAtom(win, _NET_WM_STATE_FULLSCREEN, fullscreen);

	XRaiseWindow(win->src.display, win->src.window);
	XMapRaised(win->src.display, win->src.window);
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL;
#endif
}

void RGFW_window_setFloating(RGFW_window* win, RGFW_bool floating) {
    RGFW_ASSERT(win != NULL);
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	RGFW_LOAD_ATOM(_NET_WM_STATE_ABOVE);
	RGFW_window_setXAtom(win, _NET_WM_STATE_ABOVE, floating);
#endif
#ifdef RGFW_WAYLAND
RGFW_WAYLAND_LABEL RGFW_UNUSED(floating);
#endif
}

void RGFW_window_setOpacity(RGFW_window* win, u8 opacity) {
	RGFW_ASSERT(win != NULL);
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
    const u32 value = (u32) (0xffffffffu * (double) opacity);
	RGFW_LOAD_ATOM(NET_WM_WINDOW_OPACITY);
    XChangeProperty(win->src.display, win->src.window,
					NET_WM_WINDOW_OPACITY, XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &value, 1);
#endif
#ifdef RGFW_WAYLAND
RGFW_WAYLAND_LABEL RGFW_UNUSED(opacity);
#endif
}

void RGFW_window_minimize(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_GOTO_WAYLAND(0);
	if (RGFW_window_isMaximized(win)) return;

	win->_oldRect = win->r;
#ifdef RGFW_X11
    XIconifyWindow(win->src.display, win->src.window, DefaultScreen(win->src.display));
	XFlush(win->src.display);
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL;
#endif
}

void RGFW_window_restore(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
    RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
    RGFW_toggleXMaximized(win, 0);
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL
#endif
	win->r = win->_oldRect;
	RGFW_window_move(win, RGFW_POINT(win->r.x, win->r.y));
	RGFW_window_resize(win, RGFW_AREA(win->r.w, win->r.h));
	
    RGFW_window_show(win);
#ifdef RGFW_X11	
    XFlush(win->src.display);
#endif
}

RGFW_bool RGFW_window_isFloating(RGFW_window* win) {
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
    RGFW_LOAD_ATOM(_NET_WM_STATE);
	RGFW_LOAD_ATOM(_NET_WM_STATE_ABOVE);

	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytes_after;
	Atom* prop_return = NULL;

	int status = XGetWindowProperty(win->src.display, win->src.window, _NET_WM_STATE, 0, (~0L), False, XA_ATOM,
									&actual_type, &actual_format, &nitems, &bytes_after,
									(unsigned char **)&prop_return);

	if (status != Success || actual_type != XA_ATOM)
		return RGFW_FALSE;

    unsigned long i;
	for (i = 0; i < nitems; i++)
		if (prop_return[i] == _NET_WM_STATE_ABOVE) return RGFW_TRUE;

	if (prop_return)
		XFree(prop_return);
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL RGFW_UNUSED(win);
#endif
	return RGFW_FALSE;
}

void RGFW_window_setName(RGFW_window* win, const char* name) {
	RGFW_ASSERT(win != NULL);
	RGFW_GOTO_WAYLAND(0);
	#ifdef RGFW_X11
	XStoreName(win->src.display, win->src.window, name);

	RGFW_LOAD_ATOM(_NET_WM_NAME);

    char buf[256];
    RGFW_MEMSET(buf, 0, sizeof(buf));
    RGFW_STRNCPY(buf, name, sizeof(buf) - 1);

    XChangeProperty(
		win->src.display, win->src.window, _NET_WM_NAME, RGFW_XUTF8_STRING,
		8, PropModeReplace, (u8*)buf, sizeof(buf)
	);
	#endif
	#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
	if (win->src.compositor)
		xdg_toplevel_set_title(win->src.xdg_toplevel, name);
	#endif
}

#ifndef RGFW_NO_PASSTHROUGH
void RGFW_window_setMousePassthrough(RGFW_window* win, RGFW_bool passthrough) {
	RGFW_ASSERT(win != NULL);
    RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
    if (passthrough) {
		Region region = XCreateRegion();
		XShapeCombineRegion(win->src.display, win->src.window, ShapeInput, 0, 0, region, ShapeSet);
		XDestroyRegion(region);

		return;
	}

	XShapeCombineMask(win->src.display, win->src.window, ShapeInput, 0, 0, None, ShapeSet);
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL RGFW_UNUSED(passthrough);
#endif
}
#endif /* RGFW_NO_PASSTHROUGH */

RGFW_bool RGFW_window_setIconEx(RGFW_window* win, u8* icon, RGFW_area a, i32 channels, u8 type) {
	RGFW_ASSERT(win != NULL);
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	RGFW_LOAD_ATOM(_NET_WM_ICON);
	if (icon == NULL || (channels != 3 && channels != 4)) {
		RGFW_bool res = (RGFW_bool)XChangeProperty(
			win->src.display, win->src.window, _NET_WM_ICON, XA_CARDINAL, 32,
			PropModeReplace, (u8*)NULL, 0
		);
		return res;
	}

	i32 count = (i32)(2 + (a.w * a.h));

	unsigned long* data = (unsigned long*) RGFW_ALLOC((u32)count * sizeof(unsigned long));
    RGFW_ASSERT(data != NULL);

    data[0] = (unsigned long)a.w;
	data[1] = (unsigned long)a.h;

	unsigned long* target = &data[2];
	u32 x, y;

	for (x = 0; x < a.w; x++) {
		for (y = 0; y < a.h; y++) {
			size_t i = y * a.w + x;
			u32 alpha = (channels == 4) ? icon[i * 4 + 3] : 0xFF;

			target[i] = (unsigned long)((icon[i * 4 + 0]) << 16) |
						(unsigned long)((icon[i * 4 + 1]) <<  8) |
						(unsigned long)((icon[i * 4 + 2]) <<  0) |
						(unsigned long)(alpha << 24);
		}
	}

	RGFW_bool res = RGFW_TRUE;
	if (type & RGFW_iconTaskbar) {
		res = (RGFW_bool)XChangeProperty(
			win->src.display, win->src.window, _NET_WM_ICON, XA_CARDINAL, 32,
			PropModeReplace, (u8*)data, count
		);
	}

	if (type & RGFW_iconWindow) {
		XWMHints wm_hints;
		wm_hints.flags = IconPixmapHint;

		i32 depth = DefaultDepth(win->src.display, DefaultScreen(win->src.display));
		XImage *image = XCreateImage(win->src.display, DefaultVisual(win->src.display, DefaultScreen(win->src.display)),
									(u32)depth, ZPixmap, 0, (char *)target, a.w, a.h, 32, 0);

		wm_hints.icon_pixmap = XCreatePixmap(win->src.display, win->src.window, a.w, a.h, (u32)depth);
		XPutImage(win->src.display, wm_hints.icon_pixmap, DefaultGC(win->src.display, DefaultScreen(win->src.display)), image, 0, 0, 0, 0, a.w, a.h);
		image->data = NULL;
		XDestroyImage(image);

		XSetWMHints(win->src.display, win->src.window, &wm_hints);
	}

	RGFW_FREE(data);
	XFlush(win->src.display);
	return RGFW_BOOL(res);
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL  RGFW_UNUSED(icon); RGFW_UNUSED(a); RGFW_UNUSED(channels); RGFW_UNUSED(type);
	return RGFW_FALSE;
#endif
}

RGFW_mouse* RGFW_loadMouse(u8* icon, RGFW_area a, i32 channels) {
    RGFW_ASSERT(icon);
	RGFW_ASSERT(channels == 3 || channels == 4);
	RGFW_GOTO_WAYLAND(0);

#ifdef RGFW_X11
#ifndef RGFW_NO_X11_CURSOR
	RGFW_init();
    XcursorImage* native = XcursorImageCreate((i32)a.w, (i32)a.h);
	native->xhot = 0;
	native->yhot = 0;

	XcursorPixel* target = native->pixels;
    size_t x, y;
    for (x = 0; x < a.w; x++) {
		for (y = 0; y < a.h; y++) {
			size_t i = y * a.w + x;
			u32 alpha = (channels == 4) ? icon[i * 4 + 3] : 0xFF;

			target[i] = (u32)((icon[i * 4 + 0]) << 16)
				| (u32)((icon[i * 4 + 1]) << 8)
				| (u32)((icon[i * 4 + 2]) << 0)
				| (u32)(alpha << 24);
		}
	}

	Cursor cursor = XcursorImageLoadCursor(_RGFW.display, native);
	XcursorImageDestroy(native);

	return (void*)cursor;
#else
	RGFW_UNUSED(image); RGFW_UNUSED(a.w); RGFW_UNUSED(channels);
	return NULL;
#endif
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
	RGFW_UNUSED(icon); RGFW_UNUSED(a); RGFW_UNUSED(channels);
	return NULL; /* TODO */
#endif
}

void RGFW_window_setMouse(RGFW_window* win, RGFW_mouse* mouse) {
RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	RGFW_ASSERT(win && mouse);
	XDefineCursor(win->src.display, win->src.window, (Cursor)mouse);
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
    RGFW_UNUSED(win); RGFW_UNUSED(mouse);
#endif
}

void RGFW_freeMouse(RGFW_mouse* mouse) {
RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	RGFW_ASSERT(mouse);
	XFreeCursor(_RGFW.display, (Cursor)mouse);
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
    RGFW_UNUSED(mouse);
#endif
}

void RGFW_window_moveMouse(RGFW_window* win, RGFW_point p) {
RGFW_GOTO_WAYLAND(1);
#ifdef RGFW_X11
	RGFW_ASSERT(win != NULL);

	XEvent event;
	XQueryPointer(win->src.display, DefaultRootWindow(win->src.display),
		&event.xbutton.root, &event.xbutton.window,
		&event.xbutton.x_root, &event.xbutton.y_root,
		&event.xbutton.x, &event.xbutton.y,
		&event.xbutton.state);

	win->_lastMousePoint = RGFW_POINT(p.x - win->r.x, p.y - win->r.y);
	if (event.xbutton.x == p.x && event.xbutton.y == p.y)
		return;

	XWarpPointer(win->src.display, None, win->src.window, 0, 0, 0, 0, (int) p.x - win->r.x, (int) p.y - win->r.y);
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
    RGFW_UNUSED(win); RGFW_UNUSED(p);
#endif
}

RGFW_bool RGFW_window_setMouseDefault(RGFW_window* win) {
	return RGFW_window_setMouseStandard(win, RGFW_mouseArrow);
}

RGFW_bool RGFW_window_setMouseStandard(RGFW_window* win, u8 mouse) {
	RGFW_ASSERT(win != NULL);
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	static const u8 mouseIconSrc[16] = { XC_arrow, XC_left_ptr, XC_xterm, XC_crosshair, XC_hand2, XC_sb_h_double_arrow, XC_sb_v_double_arrow, XC_bottom_left_corner, XC_bottom_right_corner, XC_fleur, XC_X_cursor};

	if (mouse > (sizeof(mouseIconSrc) / sizeof(u8)))
		return RGFW_FALSE;

	mouse = mouseIconSrc[mouse];

	Cursor cursor = XCreateFontCursor(win->src.display, mouse);
	XDefineCursor(win->src.display, win->src.window, (Cursor) cursor);

	XFreeCursor(win->src.display, (Cursor) cursor);
	return RGFW_TRUE;
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL { }
	static const char* iconStrings[16] = { "left_ptr", "left_ptr", "text", "cross", "pointer", "e-resize", "n-resize", "nw-resize", "ne-resize", "all-resize", "not-allowed" };

	struct wl_cursor* wlcursor = wl_cursor_theme_get_cursor(RGFW_wl_cursor_theme, iconStrings[mouse]);
	RGFW_cursor_image = wlcursor->images[0];
	struct wl_buffer* cursor_buffer	= wl_cursor_image_get_buffer(RGFW_cursor_image);

	wl_surface_attach(RGFW_cursor_surface, cursor_buffer, 0, 0);
	wl_surface_commit(RGFW_cursor_surface);
	return RGFW_TRUE;
    
#endif
}

void RGFW_window_hide(RGFW_window* win) {
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	XUnmapWindow(win->src.display, win->src.window);
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
	wl_surface_attach(win->src.surface, NULL, 0, 0);
	wl_surface_commit(win->src.surface);
	win->_flags |= RGFW_windowHide;
#endif
}

void RGFW_window_show(RGFW_window* win) {
	win->_flags &= ~(u32)RGFW_windowHide;
	if (win->_flags & RGFW_windowFocusOnShow) RGFW_window_focus(win);
	RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
	XMapWindow(win->src.display, win->src.window);
#endif
#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
	/* wl_surface_attach(win->src.surface, win->rc., 0, 0); */
	wl_surface_commit(win->src.surface);
#endif
}

RGFW_ssize_t RGFW_readClipboardPtr(char* str, size_t strCapacity) {
	RGFW_GOTO_WAYLAND(1);
#ifdef RGFW_X11
	RGFW_init();
	if (XGetSelectionOwner(_RGFW.display, RGFW_XCLIPBOARD) == _RGFW.helperWindow) {
		if (str != NULL)
			RGFW_STRNCPY(str, _RGFW.clipboard, _RGFW.clipboard_len - 1);
		_RGFW.clipboard[_RGFW.clipboard_len - 1] = '\0';
		return (RGFW_ssize_t)_RGFW.clipboard_len - 1;
	}

	XEvent event;
	int format;
	unsigned long N, sizeN;
	char* data;
	Atom target;

	RGFW_LOAD_ATOM(XSEL_DATA);

	XConvertSelection(_RGFW.display, RGFW_XCLIPBOARD, RGFW_XUTF8_STRING, XSEL_DATA, _RGFW.helperWindow, CurrentTime);
	XSync(_RGFW.display, 0);
	while (1) {
		XNextEvent(_RGFW.display, &event);
		if (event.type != SelectionNotify) continue;

		if (event.xselection.selection != RGFW_XCLIPBOARD || event.xselection.property == 0)
			return -1;
		break;
	}

	XGetWindowProperty(event.xselection.display, event.xselection.requestor,
			event.xselection.property, 0L, (~0L), 0, AnyPropertyType, &target,
			&format, &sizeN, &N, (u8**) &data);

	RGFW_ssize_t size;
	if (sizeN > strCapacity && str != NULL)
		size = -1;

	if ((target == RGFW_XUTF8_STRING || target == XA_STRING) && str != NULL) {
		RGFW_MEMCPY(str, data, sizeN);
		str[sizeN] = '\0';
		XFree(data);
	} else if (str != NULL) size = -1;

	XDeleteProperty(event.xselection.display, event.xselection.requestor, event.xselection.property);
	size = (RGFW_ssize_t)sizeN;

    return size;
	#endif
	#if defined(RGFW_WAYLAND) 
	RGFW_WAYLAND_LABEL RGFW_UNUSED(str); RGFW_UNUSED(strCapacity); 
	return 0;
	#endif
}

i32 RGFW_XHandleClipboardSelectionHelper(void) {
#ifdef RGFW_X11
    RGFW_LOAD_ATOM(SAVE_TARGETS);

    XEvent event;
    XPending(_RGFW.display);

    if (QLength(_RGFW.display) || XEventsQueued(_RGFW.display, QueuedAlready) + XEventsQueued(_RGFW.display, QueuedAfterReading))
        XNextEvent(_RGFW.display, &event);
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
#else
    return 1;
#endif
}

void RGFW_writeClipboard(const char* text, u32 textLen) {
	RGFW_GOTO_WAYLAND(1);
	#ifdef RGFW_X11
	RGFW_LOAD_ATOM(SAVE_TARGETS);
    RGFW_init();

    /* request ownership of the clipboard section and request to convert it, this means its our job to convert it */
	XSetSelectionOwner(_RGFW.display, RGFW_XCLIPBOARD, _RGFW.helperWindow, CurrentTime);
	if (XGetSelectionOwner(_RGFW.display, RGFW_XCLIPBOARD) != _RGFW.helperWindow) {
    	RGFW_sendDebugInfo(RGFW_typeError, RGFW_errClipboard, RGFW_DEBUG_CTX(_RGFW.root, 0), "X11 failed to become owner of clipboard selection");
		return;
	}

	if (_RGFW.clipboard)
		RGFW_FREE(_RGFW.clipboard);

	_RGFW.clipboard = (char*)RGFW_ALLOC(textLen);
	RGFW_ASSERT(_RGFW.clipboard != NULL);

	RGFW_STRNCPY(_RGFW.clipboard, text, textLen - 1);
	_RGFW.clipboard[textLen - 1] = '\0';
	_RGFW.clipboard_len = textLen;
	#endif
	#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
    RGFW_UNUSED(text); RGFW_UNUSED(textLen);
	#endif
}

RGFW_bool RGFW_window_isHidden(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
    RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11     

	XWindowAttributes windowAttributes;
	XGetWindowAttributes(win->src.display, win->src.window, &windowAttributes);

	return (windowAttributes.map_state == IsUnmapped && !RGFW_window_isMinimized(win));
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL
    return RGFW_FALSE;
#endif
}

RGFW_bool RGFW_window_isMinimized(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
    RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11     
    RGFW_LOAD_ATOM(WM_STATE);

	Atom actual_type;
	i32 actual_format;
	unsigned long nitems, bytes_after;
	unsigned char* prop_data;

	i32 status = XGetWindowProperty(win->src.display, win->src.window, WM_STATE, 0, 2, False,
		AnyPropertyType, &actual_type, &actual_format,
		&nitems, &bytes_after, &prop_data);

	if (status == Success && nitems >= 1 && prop_data == (unsigned char*)IconicState) {
		XFree(prop_data);
		return RGFW_TRUE;
	}

	if (prop_data != NULL)
		XFree(prop_data);

	XWindowAttributes windowAttributes;
	XGetWindowAttributes(win->src.display, win->src.window, &windowAttributes);
    return windowAttributes.map_state != IsViewable;
#endif
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL
    return RGFW_FALSE;
#endif
}

RGFW_bool RGFW_window_isMaximized(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
    RGFW_GOTO_WAYLAND(0);
#ifdef RGFW_X11
    RGFW_LOAD_ATOM(_NET_WM_STATE);
	RGFW_LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
	RGFW_LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);

	Atom actual_type;
	i32 actual_format;
	unsigned long nitems, bytes_after;
	unsigned char* prop_data;

	i32 status = XGetWindowProperty(win->src.display, win->src.window, _NET_WM_STATE, 0, 1024, False,
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
#endif
#ifdef RGFW_WAYLAND
RGFW_WAYLAND_LABEL;
#endif
	return RGFW_FALSE;
}

#ifndef RGFW_NO_DPI
u32 RGFW_XCalculateRefreshRate(XRRModeInfo mi);
u32 RGFW_XCalculateRefreshRate(XRRModeInfo mi) {
    if (mi.hTotal == 0 || mi.vTotal == 0) return 0;
	return (u32) RGFW_ROUND((double) mi.dotClock / ((double) mi.hTotal * (double) mi.vTotal));
}
#endif


#ifdef RGFW_X11
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
#endif

RGFW_monitor RGFW_XCreateMonitor(i32 screen);
RGFW_monitor RGFW_XCreateMonitor(i32 screen) {
	RGFW_monitor monitor;
    RGFW_init();

	RGFW_GOTO_WAYLAND(1);
#ifdef RGFW_X11
    Display* display = _RGFW.display;

	if (screen == -1) screen = DefaultScreen(display);

	Screen* scrn = DefaultScreenOfDisplay(display);
	RGFW_area size = RGFW_AREA(scrn->width, scrn->height);

	monitor.x = 0;
	monitor.y = 0;
	monitor.mode.area = RGFW_AREA(size.w, size.h);
	monitor.physW = (float)DisplayWidthMM(display, screen) / 25.4f;
	monitor.physH = (float)DisplayHeightMM(display, screen) / 25.4f;

	RGFW_splitBPP((u32)DefaultDepth(display, DefaultScreen(display)), &monitor.mode);

	char* name = XDisplayName((const char*)display);
	RGFW_STRNCPY(monitor.name, name, sizeof(monitor.name) - 1);
	monitor.name[sizeof(monitor.name) - 1] = '\0';

	float dpi = XGetSystemContentDPI(display, screen);
	monitor.pixelRatio = dpi >= 192.0f ? 2 : 1;
	monitor.scaleX = (float) (dpi) / 96.0f;
	monitor.scaleY = (float) (dpi) / 96.0f;

	#ifndef RGFW_NO_DPI
		XRRScreenResources* sr = XRRGetScreenResourcesCurrent(display, RootWindow(display, screen));
		monitor.mode.refreshRate = RGFW_XCalculateRefreshRate(sr->modes[screen]);

		XRRCrtcInfo* ci = NULL;
		int crtc = screen;

		if (sr->ncrtc > crtc) {
			ci = XRRGetCrtcInfo(display, sr, sr->crtcs[crtc]);
		}
	#endif

	#ifndef RGFW_NO_DPI
		XRROutputInfo* info = XRRGetOutputInfo (display, sr, sr->outputs[screen]);

		if (info == NULL || ci == NULL) {
			XRRFreeScreenResources(sr);
			RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoMonitor, RGFW_DEBUG_CTX_MON(monitor), "monitor found");
			return monitor;
		}


		float physW = (float)info->mm_width / 25.4f;
		float physH = (float)info->mm_height / 25.4f;

		RGFW_STRNCPY(monitor.name, info->name, sizeof(monitor.name) - 1);
		monitor.name[sizeof(monitor.name) - 1] = '\0';

	if ((u8)physW && (u8)physH) {
		monitor.physW = physW;
		monitor.physH = physH;
	}

	monitor.x = ci->x;
	monitor.y = ci->y;

	if (ci->width && ci->height) {
		monitor.mode.area.w = (u32)ci->width;
		monitor.mode.area.h = (u32)ci->height;
	}
	#endif

	#ifndef RGFW_NO_DPI
		XRRFreeCrtcInfo(ci);
		XRRFreeScreenResources(sr);
	#endif

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoMonitor, RGFW_DEBUG_CTX_MON(monitor), "monitor found");
    return monitor;
#endif
#ifdef RGFW_WAYLAND
RGFW_WAYLAND_LABEL  RGFW_UNUSED(screen);
    RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoMonitor, RGFW_DEBUG_CTX_MON(monitor), "monitor found");
    return monitor;
#endif
}

RGFW_monitor* RGFW_getMonitors(size_t* len) {
	static RGFW_monitor monitors[7];

	RGFW_GOTO_WAYLAND(1);
	#ifdef RGFW_X11
    RGFW_init();

	Display* display = _RGFW.display;
	i32 max = ScreenCount(display);

	i32 i;
	for (i = 0; i < max && i < 6; i++)
		monitors[i] = RGFW_XCreateMonitor(i);

	if (len != NULL) *len = (size_t)((max <= 6) ? (max) : (6));

	return monitors;
	#endif
	#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL RGFW_UNUSED(len);
    return monitors; /* TODO WAYLAND */
	#endif
}

RGFW_monitor RGFW_getPrimaryMonitor(void) {
	RGFW_GOTO_WAYLAND(1);
	#ifdef RGFW_X11
	return RGFW_XCreateMonitor(-1);
	#endif
	#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL return (RGFW_monitor){ 0 }; /* TODO WAYLAND */
	#endif
}

RGFW_bool RGFW_monitor_requestMode(RGFW_monitor mon, RGFW_monitorMode mode, RGFW_modeRequest request) {
	RGFW_GOTO_WAYLAND(1);
#ifdef RGFW_X11
	#ifndef RGFW_NO_DPI
    RGFW_init();
    XRRScreenResources* screenRes = XRRGetScreenResources(_RGFW.display, DefaultRootWindow(_RGFW.display));
	if (screenRes == NULL) return RGFW_FALSE;

    int i;
    for (i = 0; i < screenRes->ncrtc; i++) {
		XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(_RGFW.display, screenRes, screenRes->crtcs[i]);
		if (!crtcInfo) continue;

		if (mon.x == crtcInfo->x && mon.y == crtcInfo->y && (u32)mon.mode.area.w == crtcInfo->width && (u32)mon.mode.area.h == crtcInfo->height) {
			RRMode rmode = None;
            int index;
            for (index = 0; index < screenRes->nmode; index++) {
				RGFW_monitorMode foundMode;
				foundMode.area = RGFW_AREA(screenRes->modes[index].width, screenRes->modes[index].height);
				foundMode.refreshRate =  RGFW_XCalculateRefreshRate(screenRes->modes[index]);
				RGFW_splitBPP((u32)DefaultDepth(_RGFW.display, DefaultScreen(_RGFW.display)), &foundMode);

				if (RGFW_monitorModeCompare(mode, foundMode, request)) {
					rmode = screenRes->modes[index].id;

					RROutput output = screenRes->outputs[i];
					XRROutputInfo* info = XRRGetOutputInfo(_RGFW.display, screenRes, output);
					if (info) {
						XRRSetCrtcConfig(_RGFW.display, screenRes, screenRes->crtcs[i],
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
	return RGFW_FALSE;
	#endif
#endif
#ifdef RGFW_WAYLAND
RGFW_WAYLAND_LABEL RGFW_UNUSED(mon); RGFW_UNUSED(mode); RGFW_UNUSED(request);
#endif
	return RGFW_FALSE;
}

RGFW_monitor RGFW_window_getMonitor(RGFW_window* win) {
    RGFW_monitor mon;
    RGFW_MEMSET(&mon, 0, sizeof(mon));

    RGFW_ASSERT(win != NULL);
	RGFW_GOTO_WAYLAND(1);
#ifdef RGFW_X11
	XWindowAttributes attrs;
    if (!XGetWindowAttributes(win->src.display, win->src.window, &attrs)) {
        return mon;
    }

	i32 i;
	for (i = 0; i < ScreenCount(win->src.display) && i < 6; i++) {
		Screen* screen = ScreenOfDisplay(win->src.display, i);
        if (attrs.x >= 0 && attrs.x < XWidthOfScreen(screen) &&
            attrs.y >= 0 && attrs.y < XHeightOfScreen(screen))
            	return RGFW_XCreateMonitor(i);
	}
#endif
#ifdef RGFW_WAYLAND
RGFW_WAYLAND_LABEL
#endif
    return mon;

}

#if defined(RGFW_OPENGL) && !defined(RGFW_EGL)
void RGFW_window_makeCurrent_OpenGL(RGFW_window* win) {
	if (win == NULL)
		glXMakeCurrent(NULL, (Drawable)NULL, (GLXContext) NULL);
	else
		glXMakeCurrent(win->src.display, (Drawable) win->src.window, (GLXContext) win->src.ctx);
}
void* RGFW_getCurrent_OpenGL(void) { return glXGetCurrentContext();  }
void RGFW_window_swapBuffers_OpenGL(RGFW_window* win) { glXSwapBuffers(win->src.display, win->src.window); }
#endif

void RGFW_window_swapBuffers_software(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	RGFW_GOTO_WAYLAND(0);
#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
	#ifdef RGFW_X11
		win->src.bitmap->data = (char*) win->buffer;
		RGFW_RGB_to_BGR(win, (u8*)win->src.bitmap->data);
		XPutImage(win->src.display, win->src.window, win->src.gc, win->src.bitmap, 0, 0, 0, 0, win->bufferSize.w, win->bufferSize.h);
		win->src.bitmap->data = NULL;
		return;
	#endif
	#ifdef RGFW_WAYLAND
	RGFW_WAYLAND_LABEL
        #if !defined(RGFW_BUFFER_BGR) && !defined(RGFW_OSMESA)
		RGFW_RGB_to_BGR(win, win->src.buffer);
		#else
        size_t y;
		for (y = 0; y < win->r.h; y++) {
			u32 index = (y * 4 * win->r.w);
			u32 index2 = (y * 4 * win->bufferSize.w);
			RGFW_MEMCPY(&win->src.buffer[index], &win->buffer[index2], win->r.w * 4);
		}
		#endif

		wl_surface_frame_done(win, NULL, 0);
		wl_surface_commit(win->src.surface);
	#endif
#else
#ifdef RGFW_WAYLAND
    RGFW_WAYLAND_LABEL
#endif
    RGFW_UNUSED(win);
#endif
}

#if !defined(RGFW_EGL)

void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) {
	RGFW_ASSERT(win != NULL);

	#if defined(RGFW_OPENGL)
	// cached pfn to avoid calling glXGetProcAddress more than once
	static PFNGLXSWAPINTERVALEXTPROC pfn = (PFNGLXSWAPINTERVALEXTPROC)123;
    static int (*pfn2)(int) = NULL;

	if (pfn == (PFNGLXSWAPINTERVALEXTPROC)123) {
		pfn = ((PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((GLubyte*) "glXSwapIntervalEXT"));
		if (pfn == NULL)  {
            const char* array[] = {"GLX_MESA_swap_control", "GLX_SGI_swap_control"};
            u32 i;
            for (i = 0; i < sizeof(array) / sizeof(char*) && pfn2 == NULL; i++)
    		    pfn2 = ((int(*)(int))glXGetProcAddress((GLubyte*) array[i]));

            if (pfn2 != NULL) {
        		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(_RGFW.root, 0), "Failed to load swap interval function, fallingback to the native swapinterval function");
            } else {
        		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(_RGFW.root, 0), "Failed to load swap interval function");
            }
        }
    }
	if (pfn != NULL)
		pfn(win->src.display, win->src.window, swapInterval);
    else if (pfn2 != NULL) {
        pfn2(swapInterval);
    }
	#else
	RGFW_UNUSED(swapInterval);
	#endif
}
#endif

void RGFW_deinit(void) {
    if (_RGFW.windowCount == -1 || _RGFW_init == RGFW_FALSE) return;
    #define RGFW_FREE_LIBRARY(x) if (x != NULL) dlclose(x); x = NULL;
#ifdef RGFW_X11
	/* to save the clipboard on the x server after the window is closed */
	RGFW_LOAD_ATOM(CLIPBOARD_MANAGER);
	RGFW_LOAD_ATOM(SAVE_TARGETS);
	if (XGetSelectionOwner(_RGFW.display, RGFW_XCLIPBOARD) == _RGFW.helperWindow) {
		XConvertSelection(_RGFW.display, CLIPBOARD_MANAGER, SAVE_TARGETS, None, _RGFW.helperWindow, CurrentTime);
        while (RGFW_XHandleClipboardSelectionHelper());
	}
	if (_RGFW.clipboard) {
		RGFW_FREE(_RGFW.clipboard);
		_RGFW.clipboard = NULL;
	}

    RGFW_freeMouse(_RGFW.hiddenMouse);

    XDestroyWindow(_RGFW.display, (Drawable) _RGFW.helperWindow); /*!< close the window */
    XCloseDisplay(_RGFW.display); /*!< kill connection to the x server */

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
#endif
#ifdef RGFW_WAYLAND
	wl_display_disconnect(_RGFW.wl_display);
#endif
    #ifndef RGFW_NO_LINUX
    if (RGFW_eventWait_forceStop[0] || RGFW_eventWait_forceStop[1]){
        close(RGFW_eventWait_forceStop[0]);
        close(RGFW_eventWait_forceStop[1]);
    }

    u8 i;
    for (i = 0; i < RGFW_gamepadCount; i++) {
        if(RGFW_gamepads[i])
            close(RGFW_gamepads[i]);
    }
    #endif

    _RGFW.root = NULL;
    _RGFW.windowCount = -1;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal, RGFW_DEBUG_CTX(NULL, 0), "global context deinitialized");
}

void RGFW_window_close(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	if ((win->_flags & RGFW_windowNoInitAPI) == 0) RGFW_window_freeOpenGL(win);

	RGFW_GOTO_WAYLAND(0);
	#ifdef RGFW_X11
    /* ungrab pointer if it was grabbed */
	if (win->_flags & RGFW_HOLD_MOUSE)
		XUngrabPointer(win->src.display, CurrentTime);

    #if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
		if (win->buffer != NULL) {
			if ((win->_flags & RGFW_BUFFER_ALLOC))
				RGFW_FREE(win->buffer);
			XDestroyImage((XImage*) win->src.bitmap);
		}
	#endif

    XFreeGC(win->src.display, win->src.gc);
    XDestroyWindow(win->src.display, (Drawable) win->src.window); /*!< close the window */
    win->src.window = 0;
    XCloseDisplay(win->src.display);

    RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, RGFW_DEBUG_CTX(win, 0), "a window was freed");
    _RGFW.windowCount--;
    if (_RGFW.windowCount == 0) RGFW_deinit();

	RGFW_clipboard_switch(NULL);
	RGFW_FREE(win->event.droppedFiles);
    if ((win->_flags & RGFW_WINDOW_ALLOC)) {
		RGFW_FREE(win);
        win = NULL;
    }
	return;
	#endif

	#ifdef RGFW_WAYLAND
		RGFW_WAYLAND_LABEL

	    RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, RGFW_DEBUG_CTX(win, 0), "a window was freed");

        xdg_toplevel_destroy(win->src.xdg_toplevel);
        xdg_surface_destroy(win->src.xdg_surface);
		wl_surface_destroy(win->src.surface);

		_RGFW.windowCount--;
        if (_RGFW.windowCount == 0) RGFW_deinit();

		#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
			wl_buffer_destroy(win->src.wl_buffer);
			if ((win->_flags & RGFW_BUFFER_ALLOC))
				RGFW_FREE(win->buffer);

			munmap(win->src.buffer, (size_t)(win->r.w * win->r.h * 4));
		#endif

		RGFW_clipboard_switch(NULL);
		RGFW_FREE(win->event.droppedFiles);
        if ((win->_flags & RGFW_WINDOW_ALLOC)) {
			RGFW_FREE(win);
            win = NULL;
        }
	#endif
}


/*
	End of X11 linux / wayland / unix defines
*/

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

void RGFW_stopCheckEvents(void) {

	RGFW_eventWait_forceStop[2] = 1;
	while (1) {
		const char byte = 0;
		const ssize_t result = write(RGFW_eventWait_forceStop[1], &byte, 1);
		if (result == 1 || result == -1)
			break;
	}
}

void RGFW_window_eventWait(RGFW_window* win, i32 waitMS) {
	if (waitMS == 0) return;

	u8 i;
	if (RGFW_eventWait_forceStop[0] == 0 || RGFW_eventWait_forceStop[1] == 0) {
		if (pipe(RGFW_eventWait_forceStop) != -1) {
			fcntl(RGFW_eventWait_forceStop[0], F_GETFL, 0);
			fcntl(RGFW_eventWait_forceStop[0], F_GETFD, 0);
			fcntl(RGFW_eventWait_forceStop[1], F_GETFL, 0);
			fcntl(RGFW_eventWait_forceStop[1], F_GETFD, 0);
		}
	}

	struct pollfd fds[] = {
		#ifdef RGFW_WAYLAND
		{ wl_display_get_fd(win->src.wl_display), POLLIN, 0 },
		#else
		{ ConnectionNumber(win->src.display), POLLIN, 0 },
        #endif
        #ifdef RGFW_X11
		{ ConnectionNumber(_RGFW.display), POLLIN, 0 },
        #endif
        { RGFW_eventWait_forceStop[0], POLLIN, 0 },
		#if defined(__linux__)
		{ -1, POLLIN, 0 }, {-1, POLLIN, 0 }, {-1, POLLIN, 0 },  {-1, POLLIN, 0}
		#endif
	};

	u8 index = 2;
#ifdef RGFW_X11
    index++;
#endif

	#if defined(__linux__) || defined(__NetBSD__)
		for (i = 0; i < RGFW_gamepadCount; i++) {
			if (RGFW_gamepads[i] == 0)
				continue;

			fds[index].fd = RGFW_gamepads[i];
			index++;
		}
	#endif


	u64 start = RGFW_getTimeNS();


	#ifdef RGFW_WAYLAND
		while (wl_display_dispatch(win->src.wl_display) <= 0
	#else
		while (XPending(win->src.display) == 0
	#endif
    #ifdef RGFW_X11
        && XPending(_RGFW.display) == 0
    #endif
    ) {
		if (poll(fds, index, waitMS) <= 0)
			break;

		if (waitMS != RGFW_eventWaitNext)
			waitMS -= (i32)(RGFW_getTimeNS() - start) / (i32)1e+6;
	}

	/* drain any data in the stop request */
	if (RGFW_eventWait_forceStop[2]) {
		char data[64];
		(void)!read(RGFW_eventWait_forceStop[0], data, sizeof(data));

		RGFW_eventWait_forceStop[2] = 0;
	}
}

i32 RGFW_getClock(void);
i32 RGFW_getClock(void) {
	static i32 clock = -1;
	if (clock != -1) return clock;

	#if defined(_POSIX_MONOTONIC_CLOCK)
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
		clock = CLOCK_MONOTONIC;
	#else
		clock = CLOCK_REALTIME;
	#endif

	return clock;
}

u64 RGFW_getTimerFreq(void) { return 1000000000LLU; }
u64 RGFW_getTimerValue(void) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
    return (u64)ts.tv_sec * RGFW_getTimerFreq() + (u64)ts.tv_nsec;
}
#endif /* end of wayland or X11 defines */



/*

	Start of Windows defines


*/

#ifdef RGFW_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE
#include <windows.h>

#include <processthreadsapi.h>
#include <windowsx.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include <wchar.h>
#include <locale.h>
#include <winuser.h>

#ifndef WM_DPICHANGED
#define WM_DPICHANGED       0x02E0
#endif

#ifndef RGFW_NO_XINPUT
	typedef DWORD (WINAPI * PFN_XInputGetState)(DWORD,XINPUT_STATE*);
	PFN_XInputGetState XInputGetStateSRC = NULL;
	#define XInputGetState XInputGetStateSRC

	typedef DWORD (WINAPI * PFN_XInputGetKeystroke)(DWORD, DWORD, PXINPUT_KEYSTROKE);
	PFN_XInputGetKeystroke XInputGetKeystrokeSRC = NULL;
	#define XInputGetKeystroke XInputGetKeystrokeSRC

	HMODULE RGFW_XInput_dll = NULL;
#endif

char* RGFW_createUTF8FromWideStringWin32(const WCHAR* source);

#define GL_FRONT				0x0404
#define GL_BACK					0x0405
#define GL_LEFT					0x0406
#define GL_RIGHT				0x0407

typedef int (*PFN_wglGetSwapIntervalEXT)(void);
PFN_wglGetSwapIntervalEXT wglGetSwapIntervalEXTSrc = NULL;
#define wglGetSwapIntervalEXT wglGetSwapIntervalEXTSrc


void* RGFWgamepadApi = NULL;

/* these two wgl functions need to be preloaded */
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hdc, HGLRC hglrc, const int *attribList);
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;

#ifndef RGFW_EGL
	HMODULE RGFW_wgl_dll = NULL;
#endif

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

#if defined(RGFW_OPENGL) && !defined(RGFW_EGL)
RGFW_bool RGFW_extensionSupportedPlatform(const char * extension, size_t len) {
    const char* extensions = NULL;

    RGFW_proc proc = RGFW_getProcAddress("wglGetExtensionsStringARB");
    RGFW_proc proc2 = RGFW_getProcAddress("wglGetExtensionsStringEXT");

    if (proc)
        extensions = ((const char* (*)(HDC))proc)(wglGetCurrentDC());
    else if (proc2)
        extensions = ((const char*(*)(void))proc2)();

    return extensions != NULL && RGFW_extensionSupportedStr(extensions, extension, len); 
}

RGFW_proc RGFW_getProcAddress(const char* procname) {
    RGFW_proc proc = (RGFW_proc)wglGetProcAddress(procname);
    if (proc)
        return proc;

    return (RGFW_proc) GetProcAddress(RGFW_wgl_dll, procname);
}

typedef HRESULT (APIENTRY* PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;

typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALEXTPROC)(int interval);
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
#endif

#ifndef RGFW_NO_DWM
HMODULE RGFW_dwm_dll = NULL;
typedef struct { DWORD dwFlags; int fEnable; HRGN hRgnBlur; int fTransitionOnMaximized;} DWM_BLURBEHIND;
typedef HRESULT (WINAPI * PFN_DwmEnableBlurBehindWindow)(HWND, const DWM_BLURBEHIND*);
PFN_DwmEnableBlurBehindWindow DwmEnableBlurBehindWindowSRC = NULL;
#endif
void RGFW_win32_makeWindowTransparent(RGFW_window* win);
void RGFW_win32_makeWindowTransparent(RGFW_window* win) {
	if (!(win->_flags & RGFW_windowTransparent)) return;

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

	RECT windowRect;
	GetWindowRect(hWnd, &windowRect);

	switch (message) {
		case WM_CLOSE:
		case WM_QUIT:
			RGFW_eventQueuePushEx(e.type = RGFW_quit; e._win = win);
			RGFW_windowQuitCallback(win);
			return 0;
		case WM_ACTIVATE: {
			RGFW_bool inFocus = RGFW_BOOL(LOWORD(wParam) != WA_INACTIVE);
			if (inFocus) win->_flags |= RGFW_windowFocus;
			else 		win->_flags &= ~ (u32)RGFW_windowFocus;
			RGFW_eventQueuePushEx(e.type = (RGFW_eventType)((u8)RGFW_focusOut - inFocus); e._win = win);

			RGFW_focusCallback(win, inFocus);
            RGFW_window_focusLost(win);

			if ((win->_flags & RGFW_windowFullscreen) == 0)
				return DefWindowProcW(hWnd, message, wParam, lParam);

			win->_flags &= ~(u32)RGFW_EVENT_PASSED;
			if (inFocus == RGFW_FALSE) RGFW_window_minimize(win);
			else RGFW_window_setFullscreen(win, 1);
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		case WM_MOVE:
			win->r.x = windowRect.left;
			win->r.y = windowRect.top;
			RGFW_eventQueuePushEx(e.type = RGFW_windowMoved; e._win = win);
			RGFW_windowMovedCallback(win, win->r);
			return DefWindowProcW(hWnd, message, wParam, lParam);
		case WM_SIZE: {
			if (win->src.aspectRatio.w != 0 && win->src.aspectRatio.h != 0) {
				double aspectRatio = (double)win->src.aspectRatio.w / win->src.aspectRatio.h;

				int width = windowRect.right - windowRect.left;
				int height = windowRect.bottom - windowRect.top;
				int newHeight = (int)(width / aspectRatio);
				int newWidth = (int)(height * aspectRatio);

				if (win->r.w > windowRect.right - windowRect.left ||
					win->r.h > (i32)((u32)(windowRect.bottom - windowRect.top) - win->src.hOffset))
				{
					if (newHeight > height) windowRect.right = windowRect.left + newWidth;
					else windowRect.bottom = windowRect.top + newHeight;
				} else {
					if (newHeight < height) windowRect.right = windowRect.left + newWidth;
					else windowRect.bottom = windowRect.top + newHeight;
				}

				RGFW_window_resize(win, RGFW_AREA((windowRect.right - windowRect.left),
												(u32)(windowRect.bottom - windowRect.top) - (u32)win->src.hOffset));
			}

			win->r.w = windowRect.right -  windowRect.left;
			win->r.h = (windowRect.bottom - windowRect.top) - (i32)win->src.hOffset;
			RGFW_eventQueuePushEx(e.type = RGFW_windowResized; e._win = win);
			RGFW_windowResizedCallback(win, win->r);
			RGFW_window_checkMode(win);
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		#ifndef RGFW_NO_MONITOR
		case WM_DPICHANGED: {
			if (win->_flags & RGFW_windowScaleToMonitor) RGFW_window_scaleToMonitor(win);

			const float scaleX = HIWORD(wParam) / (float) 96;
            const float scaleY = LOWORD(wParam) / (float) 96;
			RGFW_scaleUpdatedCallback(win, scaleX, scaleY);
			RGFW_eventQueuePushEx(e.type = RGFW_scaleUpdated; e.scaleX = scaleX; e.scaleY = scaleY; e._win = win);
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		#endif
		case WM_GETMINMAXINFO: {
			MINMAXINFO* mmi = (MINMAXINFO*) lParam;
			mmi->ptMinTrackSize.x = (LONG)win->src.minSize.w;
			mmi->ptMinTrackSize.y = (LONG)(win->src.minSize.h + win->src.hOffset);
			if (win->src.maxSize.w == 0 && win->src.maxSize.h == 0)
				return DefWindowProcW(hWnd, message, wParam, lParam);

			mmi->ptMaxTrackSize.x = (LONG)win->src.maxSize.w;
			mmi->ptMaxTrackSize.y = (LONG)(win->src.maxSize.h + win->src.hOffset);
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            RGFW_eventQueuePushEx(e.type = RGFW_windowRefresh; e._win = win);
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
        case WM_TIMER: RGFW_windowRefreshCallback(win); break;
#endif
        case WM_NCLBUTTONDOWN: {
            /* workaround for half-second pause when starting to move window
                see: https://gamedev.net/forums/topic/672094-keeping-things-moving-during-win32-moveresize-events/5254386/
            */
            POINT point = { 0, 0 };
            if (SendMessage(win->src.window, WM_NCHITTEST, wParam, lParam) != HTCAPTION || GetCursorPos(&point) == FALSE)
                break;

            ScreenToClient(win->src.window, &point);
            PostMessage(win->src.window, WM_MOUSEMOVE, 0, ((uint32_t)point.x)|(((uint32_t)point.y) << 16));
            break;
        }
		default: break;
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

#ifndef RGFW_NO_XINPUT
void RGFW_loadXInput(void);
void RGFW_loadXInput(void) {
	u32 i;
	static const char* names[] = {"xinput1_4.dll", "xinput9_1_0.dll", "xinput1_2.dll", "xinput1_1.dll"};

	for (i = 0; i < sizeof(names) / sizeof(const char*) && (XInputGetStateSRC == NULL || XInputGetKeystrokeSRC != NULL);  i++) {
		RGFW_XInput_dll = LoadLibraryA(names[i]);
		RGFW_PROC_DEF(RGFW_XInput_dll, XInputGetState);
		RGFW_PROC_DEF(RGFW_XInput_dll, XInputGetKeystroke);
	}

	if (XInputGetStateSRC == NULL)
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errFailedFuncLoad, RGFW_DEBUG_CTX(_RGFW.root, 0), "Failed to load XInputGetState");
	if (XInputGetKeystrokeSRC == NULL)
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errFailedFuncLoad, RGFW_DEBUG_CTX(_RGFW.root, 0), "Failed to load XInputGetKeystroke");
}
#endif

void RGFW_window_initBufferPtr(RGFW_window* win, u8* buffer, RGFW_area area){
#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
	win->buffer = buffer;
	win->bufferSize = area;

	BITMAPV5HEADER bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bV5Size = sizeof(bi);
	bi.bV5Width = (i32)area.w;
	bi.bV5Height = -((LONG) area.h);
	bi.bV5Planes = 1;
	bi.bV5BitCount = 32;
	bi.bV5Compression = BI_RGB;

	win->src.bitmap = CreateDIBSection(win->src.hdc,
		(BITMAPINFO*) &bi, DIB_RGB_COLORS,
		(void**) &win->src.bitmapBits,
		NULL, (DWORD) 0);

	if (win->buffer == NULL)
		win->buffer = win->src.bitmapBits;

	win->src.hdcMem = CreateCompatibleDC(win->src.hdc);
	SelectObject(win->src.hdcMem, win->src.bitmap);

	#if defined(RGFW_OSMESA)
	win->src.ctx = OSMesaCreateContext(OSMESA_BGRA, NULL);
	OSMesaMakeCurrent(win->src.ctx, win->buffer, GL_UNSIGNED_BYTE, area.w, area.h);
	OSMesaPixelStore(OSMESA_Y_UP, 0);
	#endif
	#else
	RGFW_UNUSED(win); RGFW_UNUSED(buffer); RGFW_UNUSED(area); /*!< if buffer rendering is not being used */
	#endif
}

void RGFW_releaseCursor(RGFW_window* win) {
	RGFW_UNUSED(win);
	ClipCursor(NULL);
	const RAWINPUTDEVICE id = { 0x01, 0x02, RIDEV_REMOVE, NULL };
	RegisterRawInputDevices(&id, 1, sizeof(id));
}

void RGFW_captureCursor(RGFW_window* win, RGFW_rect rect) {
	RGFW_UNUSED(win); RGFW_UNUSED(rect);

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
int RGFW_window_createDXSwapChain(RGFW_window* win, IDXGIFactory* pFactory, IUnknown* pDevice, IDXGISwapChain** swapchain) {
    RGFW_ASSERT(win && pFactory && pDevice && swapchain);

    static DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = win->r.w;
    swapChainDesc.BufferDesc.Height = win->r.h;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = (HWND)win->src.window;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT hr = pFactory->lpVtbl->CreateSwapChain(pFactory, (IUnknown*)pDevice, &swapChainDesc, swapchain);
    if (FAILED(hr)) {
        RGFW_sendDebugInfo(RGFW_typeError, RGFW_errDirectXContext, RGFW_DEBUG_CTX(win, hr), "Failed to create DirectX swap chain!");
        return -2;
    }

    return 0;
}
#endif

void RGFW_win32_loadOpenGLFuncs(HWND dummyWin);
void RGFW_win32_loadOpenGLFuncs(HWND dummyWin) {
#ifdef RGFW_OPENGL
     if (wglSwapIntervalEXT != NULL && wglChoosePixelFormatARB != NULL && wglChoosePixelFormatARB != NULL)
		return;

	HDC dummy_dc = GetDC(dummyWin);
	u32 pfd_flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	PIXELFORMATDESCRIPTOR pfd = {sizeof(pfd), 1, pfd_flags, PFD_TYPE_RGBA, 32, 8, PFD_MAIN_PLANE, 32, 8, 8, 8, 8, 8, 0, 0, 0, 0, 0, 32, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0};

	int dummy_pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
	SetPixelFormat(dummy_dc, dummy_pixel_format, &pfd);

	HGLRC dummy_context = wglCreateContext(dummy_dc);
	wglMakeCurrent(dummy_dc, dummy_context);

	wglCreateContextAttribsARB = ((PFNWGLCREATECONTEXTATTRIBSARBPROC(WINAPI *)(const char*)) wglGetProcAddress)("wglCreateContextAttribsARB");
	wglChoosePixelFormatARB = ((PFNWGLCHOOSEPIXELFORMATARBPROC(WINAPI *)(const char*)) wglGetProcAddress)("wglChoosePixelFormatARB");

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)(RGFW_proc)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT == NULL) {
        RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(_RGFW.root, 0), "Failed to load swap interval function");
    }

	wglMakeCurrent(dummy_dc, 0);
	wglDeleteContext(dummy_context);
	ReleaseDC(dummyWin, dummy_dc);
#else
	RGFW_UNUSED(dummyWin);
#endif
}

#ifndef RGFW_EGL
void RGFW_window_initOpenGL(RGFW_window* win) {
#ifdef RGFW_OPENGL
	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion     = 1;
	pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType   = PFD_TYPE_RGBA;
	pfd.iLayerType   = PFD_MAIN_PLANE;
	pfd.cColorBits   = 32;
	pfd.cAlphaBits   = 8;
	pfd.cDepthBits   = 24;
	pfd.cStencilBits = (BYTE)RGFW_GL_HINTS[RGFW_glStencil];
	pfd.cAuxBuffers  = (BYTE)RGFW_GL_HINTS[RGFW_glAuxBuffers];
	if (RGFW_GL_HINTS[RGFW_glStereo]) pfd.dwFlags |= PFD_STEREO;

	/* try to create the pixel format we want for opengl and then try to create an opengl context for the specified version */
	if (win->_flags & RGFW_windowOpenglSoftware)
		pfd.dwFlags |= PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED;

	/* get pixel format, default to a basic pixel format */
	int pixel_format = ChoosePixelFormat(win->src.hdc, &pfd);
	if (wglChoosePixelFormatARB != NULL) {
		i32* pixel_format_attribs = (i32*)RGFW_initFormatAttribs();

		int new_pixel_format;
		UINT num_formats;
		wglChoosePixelFormatARB(win->src.hdc, pixel_format_attribs, 0, 1, &new_pixel_format, &num_formats);
		if (!num_formats)
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(win, 0), "Failed to create a pixel format for WGL");
		else pixel_format = new_pixel_format;
	}

	PIXELFORMATDESCRIPTOR suggested;
	if (!DescribePixelFormat(win->src.hdc, pixel_format, sizeof(suggested), &suggested) ||
		!SetPixelFormat(win->src.hdc, pixel_format, &pfd))
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(win, 0), "Failed to set the WGL pixel format");
    
    if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED)) {
        win->_flags |= RGFW_windowOpenglSoftware;
    }

	if (wglCreateContextAttribsARB != NULL) {
		/* create opengl/WGL context for the specified version */
		u32 index = 0;
		i32 attribs[40];

		if (RGFW_GL_HINTS[RGFW_glProfile]== RGFW_glCore) {
			SET_ATTRIB(WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB);
		}
		else {
			SET_ATTRIB(WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB);
		}

		if (RGFW_GL_HINTS[RGFW_glMinor] || RGFW_GL_HINTS[RGFW_glMajor]) {
			SET_ATTRIB(WGL_CONTEXT_MAJOR_VERSION_ARB, RGFW_GL_HINTS[RGFW_glMajor]);
			SET_ATTRIB(WGL_CONTEXT_MINOR_VERSION_ARB, RGFW_GL_HINTS[RGFW_glMinor]);
		}

		SET_ATTRIB(0, 0);

		win->src.ctx = (HGLRC)wglCreateContextAttribsARB(win->src.hdc, NULL, attribs);
	} else { /* fall back to a default context (probably opengl 2 or something) */
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(win, 0), "Failed to create an accelerated OpenGL Context");
		win->src.ctx = wglCreateContext(win->src.hdc);
	}

	ReleaseDC(win->src.window, win->src.hdc);
	win->src.hdc = GetDC(win->src.window);
	wglMakeCurrent(win->src.hdc, win->src.ctx);

	if (_RGFW.root != win)
		wglShareLists(_RGFW.root->src.ctx, win->src.ctx);
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, RGFW_DEBUG_CTX(win, 0), "opengl context initalized");
#else
	RGFW_UNUSED(win);
#endif
}

void RGFW_window_freeOpenGL(RGFW_window* win) {
#ifdef RGFW_OPENGL
	if (win->src.ctx == NULL) return;
	wglDeleteContext((HGLRC) win->src.ctx); /*!< delete opengl context */
	win->src.ctx = NULL;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, RGFW_DEBUG_CTX(win, 0), "opengl context freed");
#else
	RGFW_UNUSED(win);
#endif
}
#endif


i32 RGFW_init(void) {
#if defined(RGFW_C89) || defined(__cplusplus)
    if (_RGFW_init) return 0; 
    _RGFW_init = RGFW_TRUE;
    _RGFW.root = NULL; _RGFW.current = NULL; _RGFW.windowCount = -1; _RGFW.eventLen = 0; _RGFW.eventIndex = 0;
#endif

    #ifndef RGFW_NO_XINPUT
		if (RGFW_XInput_dll == NULL)
			RGFW_loadXInput();
	#endif

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
	_RGFW.hiddenMouse = RGFW_loadMouse(RGFW_blk, RGFW_AREA(1, 1), 4);

    _RGFW.windowCount = 0;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal, RGFW_DEBUG_CTX(NULL, 0), "global context initialized");
    return 1;
}

RGFW_window* RGFW_createWindowPtr(const char* name, RGFW_rect rect, RGFW_windowFlags flags, RGFW_window* win) {
	if (name[0] == 0) name = (char*) " ";

	RGFW_window_basic_init(win, rect, flags);

	win->src.hIconSmall = win->src.hIconBig = NULL;
	win->src.maxSize = RGFW_AREA(0, 0);
	win->src.minSize = RGFW_AREA(0, 0);
	win->src.aspectRatio = RGFW_AREA(0, 0);

	HINSTANCE inh = GetModuleHandleA(NULL);

	#ifndef __cplusplus
	WNDCLASSW Class = { 0 }; /*!< Setup the Window class. */
	#else
	WNDCLASSW Class = { };
	#endif

	if (RGFW_className == NULL)
		RGFW_className = (char*)name;

	wchar_t wide_class[256];
	MultiByteToWideChar(CP_UTF8, 0, RGFW_className, -1, wide_class, 255);

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
		window_style |= WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX | WS_THICKFRAME;

		if (!(flags & RGFW_windowNoResize))
			window_style |= WS_SIZEBOX | WS_MAXIMIZEBOX;
	} else
		window_style |= WS_POPUP | WS_VISIBLE | WS_SYSMENU;

	wchar_t wide_name[256];
	MultiByteToWideChar(CP_UTF8, 0, name, -1, wide_name, 255);
	HWND dummyWin = CreateWindowW(Class.lpszClassName, (wchar_t*)wide_name, window_style, win->r.x, win->r.y, win->r.w, win->r.h, 0, 0, inh, 0);

	GetWindowRect(dummyWin, &windowRect);
	GetClientRect(dummyWin, &clientRect);

	RGFW_win32_loadOpenGLFuncs(dummyWin);
	DestroyWindow(dummyWin);

	win->src.hOffset = (u32)(windowRect.bottom - windowRect.top) - (u32)(clientRect.bottom - clientRect.top);
	win->src.window = CreateWindowW(Class.lpszClassName, (wchar_t*)wide_name, window_style, win->r.x, win->r.y, win->r.w, win->r.h + (i32)win->src.hOffset, 0, 0, inh, 0);
	SetPropW(win->src.window, L"RGFW", win);
	RGFW_window_resize(win, RGFW_AREA(win->r.w, win->r.h)); /* so WM_GETMINMAXINFO gets called again */

	if (flags & RGFW_windowAllowDND) {
		win->_flags |= RGFW_windowAllowDND;
		RGFW_window_setDND(win, 1);
	}
	win->src.hdc = GetDC(win->src.window);

	if ((flags & RGFW_windowNoInitAPI) == 0) {
        RGFW_window_initOpenGL(win);
        RGFW_window_initBuffer(win);
    }

	RGFW_window_setFlags(win, flags);
	RGFW_win32_makeWindowTransparent(win);
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, RGFW_DEBUG_CTX(win, 0), "a new window was created");
    RGFW_window_show(win);

    return win;
}

void RGFW_window_setBorder(RGFW_window* win, RGFW_bool border) {
	RGFW_setBit(&win->_flags, RGFW_windowNoBorder, !border);
	LONG style = GetWindowLong(win->src.window, GWL_STYLE);


	if (border == 0) {
		SetWindowLong(win->src.window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
		SetWindowPos(
			win->src.window, HWND_TOP, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE
		);
	}
	else {
		style |= WS_OVERLAPPEDWINDOW;
		if (win->_flags & RGFW_windowNoResize) style &= ~WS_MAXIMIZEBOX;
		SetWindowPos(
			win->src.window, HWND_TOP, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE
		);
	}
}

void RGFW_window_setDND(RGFW_window* win, RGFW_bool allow) {
	RGFW_setBit(&win->_flags, RGFW_windowAllowDND, allow);
	DragAcceptFiles(win->src.window, allow);
}

RGFW_area RGFW_getScreenSize(void) {
	HDC dc = GetDC(NULL);
	RGFW_area area = RGFW_AREA(GetDeviceCaps(dc, HORZRES), GetDeviceCaps(dc, VERTRES));
	ReleaseDC(NULL, dc);
	return area;
}

RGFW_point RGFW_getGlobalMousePoint(void) {
	POINT p;
	GetCursorPos(&p);

	return RGFW_POINT(p.x, p.y);
}

void RGFW_window_setAspectRatio(RGFW_window* win, RGFW_area a) {
	RGFW_ASSERT(win != NULL);
	win->src.aspectRatio = a;
}

void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a) {
	RGFW_ASSERT(win != NULL);
	win->src.minSize = a;
}

void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a) {
	RGFW_ASSERT(win != NULL);
	win->src.maxSize = a;
}

void RGFW_window_focus(RGFW_window* win) {
	RGFW_ASSERT(win);
    SetForegroundWindow(win->src.window);
    SetFocus(win->src.window);
}

void RGFW_window_raise(RGFW_window* win) {
	RGFW_ASSERT(win);
	BringWindowToTop(win->src.window);
	SetWindowPos(win->src.window, HWND_TOP, win->r.x, win->r.y, win->r.w, win->r.h, SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
}

void RGFW_window_setFullscreen(RGFW_window* win, RGFW_bool fullscreen) {
	RGFW_ASSERT(win != NULL);

	if (fullscreen == RGFW_FALSE) {
		RGFW_window_setBorder(win, 1);
		SetWindowPos(win->src.window, HWND_NOTOPMOST, win->_oldRect.x, win->_oldRect.y, win->_oldRect.w, win->_oldRect.h + (i32)win->src.hOffset,
			 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		win->_flags &= ~(u32)RGFW_windowFullscreen;
		win->r = win->_oldRect;
		return;
	}

	win->_oldRect = win->r;
	win->_flags |= RGFW_windowFullscreen;

	RGFW_monitor mon  = RGFW_window_getMonitor(win);
	RGFW_window_setBorder(win, 0);

    SetWindowPos(win->src.window, HWND_TOPMOST, 0, 0, (i32)mon.mode.area.w, (i32)mon.mode.area.h, SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    RGFW_monitor_scaleToWindow(mon, win);

	win->r = RGFW_RECT(0, 0, mon.mode.area.w, mon.mode.area.h);
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

u8 RGFW_xinput2RGFW[] = {
	RGFW_gamepadA, /* or PS X button */
	RGFW_gamepadB, /* or PS circle button */
	RGFW_gamepadX, /* or PS square button */
	RGFW_gamepadY, /* or PS triangle button */
	RGFW_gamepadR1, /* right bumper */
	RGFW_gamepadL1, /* left bump */
	RGFW_gamepadL2, /* left trigger */
	RGFW_gamepadR2, /* right trigger */
	0, 0, 0, 0, 0, 0, 0, 0,
	RGFW_gamepadUp, /* dpad up */
	RGFW_gamepadDown, /* dpad down */
	RGFW_gamepadLeft, /* dpad left */
	RGFW_gamepadRight, /* dpad right */
	RGFW_gamepadStart, /* start button */
	RGFW_gamepadSelect,/* select button */
	RGFW_gamepadL3,
	RGFW_gamepadR3,
};
i32 RGFW_checkXInput(RGFW_window* win, RGFW_event* e);
i32 RGFW_checkXInput(RGFW_window* win, RGFW_event* e) {
	#ifndef RGFW_NO_XINPUT

	RGFW_UNUSED(win);
    u16 i;
	for (i = 0; i < 4; i++) {
		XINPUT_KEYSTROKE keystroke;

		if (XInputGetKeystroke == NULL)
			return 0;

		DWORD result = XInputGetKeystroke((DWORD)i, 0, &keystroke);

		if ((keystroke.Flags & XINPUT_KEYSTROKE_REPEAT) == 0 && result != ERROR_EMPTY) {
			if (result != ERROR_SUCCESS)
				return 0;

			if (keystroke.VirtualKey > VK_PAD_RTHUMB_PRESS)
				continue;

			/* gamepad + 1 = RGFW_gamepadButtonReleased */
			e->type = RGFW_gamepadButtonPressed + !(keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN);
			e->button = RGFW_xinput2RGFW[keystroke.VirtualKey - 0x5800];
			RGFW_gamepadPressed[i][e->button].prev = RGFW_gamepadPressed[i][e->button].current;
			RGFW_gamepadPressed[i][e->button].current = RGFW_BOOL(keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN);

			RGFW_gamepadButtonCallback(win, i, e->button, e->type == RGFW_gamepadButtonPressed);
			return 1;
		}

		XINPUT_STATE state;
		if (XInputGetState == NULL ||
			XInputGetState((DWORD) i, &state) == ERROR_DEVICE_NOT_CONNECTED
		) {
			if (RGFW_gamepads[i] == 0)
				continue;

			RGFW_gamepads[i] = 0;
			RGFW_gamepadCount--;

			win->event.type = RGFW_gamepadDisconnected;
			win->event.gamepad = (u16)i;
			RGFW_gamepadCallback(win, i, 0);
			return 1;
		}

		if (RGFW_gamepads[i] == 0) {
			RGFW_gamepads[i] = 1;
			RGFW_gamepadCount++;

			char str[] = "Microsoft X-Box (XInput device)";
			RGFW_MEMCPY(RGFW_gamepads_name[i], str, sizeof(str));
			RGFW_gamepads_name[i][sizeof(RGFW_gamepads_name[i]) - 1] = '\0';
			win->event.type = RGFW_gamepadConnected;
			win->event.gamepad = i;
			RGFW_gamepads_type[i] = RGFW_gamepadMicrosoft;

			RGFW_gamepadCallback(win, i, 1);
			return 1;
		}

#define INPUT_DEADZONE  ( 0.24f * (float)(0x7FFF) )  /* Default to 24% of the +/- 32767 range.   This is a reasonable default value but can be altered if needed. */

		if ((state.Gamepad.sThumbLX < INPUT_DEADZONE &&
			state.Gamepad.sThumbLX > -INPUT_DEADZONE) &&
			(state.Gamepad.sThumbLY < INPUT_DEADZONE &&
				state.Gamepad.sThumbLY > -INPUT_DEADZONE))
		{
			state.Gamepad.sThumbLX = 0;
			state.Gamepad.sThumbLY = 0;
		}

		if ((state.Gamepad.sThumbRX < INPUT_DEADZONE &&
			state.Gamepad.sThumbRX > -INPUT_DEADZONE) &&
			(state.Gamepad.sThumbRY < INPUT_DEADZONE &&
				state.Gamepad.sThumbRY > -INPUT_DEADZONE))
		{
			state.Gamepad.sThumbRX = 0;
			state.Gamepad.sThumbRY = 0;
		}

		e->axisesCount = 2;
		RGFW_point axis1 = RGFW_POINT(((float)state.Gamepad.sThumbLX / 32768.0f) * 100, ((float)state.Gamepad.sThumbLY / -32768.0f) * 100);
		RGFW_point axis2 = RGFW_POINT(((float)state.Gamepad.sThumbRX / 32768.0f) * 100, ((float)state.Gamepad.sThumbRY / -32768.0f) * 100);

		if (axis1.x != e->axis[0].x || axis1.y != e->axis[0].y){
			win->event.whichAxis = 0;

			e->type = RGFW_gamepadAxisMove;
			e->axis[0] = axis1;
			RGFW_gamepadAxes[i][0] = e->axis[0];

			RGFW_gamepadAxisCallback(win, e->gamepad, e->axis, e->axisesCount, e->whichAxis);
			return 1;
		}

		if (axis2.x != e->axis[1].x || axis2.y != e->axis[1].y) {
			win->event.whichAxis = 1;
			e->type = RGFW_gamepadAxisMove;
			e->axis[1] = axis2;
			RGFW_gamepadAxes[i][1] = e->axis[1];

			RGFW_gamepadAxisCallback(win, e->gamepad, e->axis, e->axisesCount, e->whichAxis);
			return 1;
		}
	}

	#endif

	return 0;
}

void RGFW_stopCheckEvents(void) {
	PostMessageW(_RGFW.root->src.window, WM_NULL, 0, 0);
}

void RGFW_window_eventWait(RGFW_window* win, i32 waitMS) {
	RGFW_UNUSED(win);
	MsgWaitForMultipleObjects(0, NULL, FALSE, (DWORD)waitMS, QS_ALLINPUT);
}

u8 RGFW_rgfwToKeyChar(u32 rgfw_keycode) {
    UINT vsc = RGFW_rgfwToApiKey(rgfw_keycode);  // Should return a Windows VK_* code
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

RGFW_event* RGFW_window_checkEvent(RGFW_window* win) {
    if (win == NULL || ((win->_flags & RGFW_windowFreeOnClose) && (win->_flags & RGFW_EVENT_QUIT))) return NULL;
    RGFW_event* ev = RGFW_window_checkEventCore(win);
	if (ev) {
        return ev;
    }

    static HDROP drop;
	if (win->event.type == RGFW_DNDInit) {
		if (win->event.droppedFilesCount) {
			u32 i;
			for (i = 0; i < win->event.droppedFilesCount; i++)
				win->event.droppedFiles[i][0] = '\0';
		}

		win->event.droppedFilesCount = 0;
		win->event.droppedFilesCount = DragQueryFileW(drop, 0xffffffff, NULL, 0);

		u32 i;
		for (i = 0; i < win->event.droppedFilesCount; i++) {
			UINT length = DragQueryFileW(drop, i, NULL, 0);
			if (length == 0)
				continue;

			WCHAR buffer[RGFW_MAX_PATH * 2];
			if (length > (RGFW_MAX_PATH * 2) - 1)
				length = RGFW_MAX_PATH * 2;

			DragQueryFileW(drop, i, buffer, length + 1);

			char* str = RGFW_createUTF8FromWideStringWin32(buffer);
			if (str != NULL)
				RGFW_MEMCPY(win->event.droppedFiles[i], str, length + 1);

			win->event.droppedFiles[i][RGFW_MAX_PATH - 1] = '\0';
		}

		DragFinish(drop);
		RGFW_dndCallback(win, win->event.droppedFiles, win->event.droppedFilesCount);

		win->event.type = RGFW_DND;
		return &win->event;
	}

	if (RGFW_checkXInput(win, &win->event))
		return &win->event;

	static BYTE keyboardState[256];
	GetKeyboardState(keyboardState);

    MSG msg;
    if (PeekMessageA(&msg, NULL, 0u, 0u, PM_REMOVE)) {
        if (msg.hwnd != win->src.window && msg.hwnd != NULL) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
            return RGFW_window_checkEvent(win);
        }
    } else {
        return NULL;
    }

    switch (msg.message) {
		case WM_MOUSELEAVE:
			win->event.type = RGFW_mouseLeave;
			win->_flags |= RGFW_MOUSE_LEFT;
			RGFW_mouseNotifyCallback(win, win->event.point, 0);
			break;
		case WM_SYSKEYUP: case WM_KEYUP: {
			i32 scancode = (HIWORD(msg.lParam) & (KF_EXTENDED | 0xff));
			if (scancode == 0)
				scancode = (i32)MapVirtualKeyW((UINT)msg.wParam, MAPVK_VK_TO_VSC);

			switch (scancode) {
				case 0x54: scancode = 0x137; break; /*  Alt+PrtS */
				case 0x146: scancode = 0x45; break; /* Ctrl+Pause */
				case 0x136: scancode = 0x36; break; /*  CJK IME sets the extended bit for right Shift */
				default: break;
			}

			win->event.key = (u8)RGFW_apiKeyToRGFW((u32) scancode);

			if (msg.wParam == VK_CONTROL) {
				if (HIWORD(msg.lParam) & KF_EXTENDED)
					win->event.key = RGFW_controlR;
				else win->event.key = RGFW_controlL;
			}

			wchar_t charBuffer;
			ToUnicodeEx((UINT)msg.wParam, (UINT)scancode, keyboardState, (wchar_t*)&charBuffer, 1, 0, NULL);

			win->event.keyChar = (u8)charBuffer;

			RGFW_keyboard[win->event.key].prev = RGFW_keyboard[win->event.key].current;
			win->event.type = RGFW_keyReleased;
			RGFW_keyboard[win->event.key].current = 0;

			RGFW_updateKeyMods(win, (GetKeyState(VK_CAPITAL) & 0x0001), (GetKeyState(VK_NUMLOCK) & 0x0001), (GetKeyState(VK_SCROLL) & 0x0001));

			RGFW_keyCallback(win, win->event.key, win->event.keyChar, win->event.keyMod, 0);
			break;
		}
		case WM_SYSKEYDOWN: case WM_KEYDOWN: {
            i32 scancode = (HIWORD(msg.lParam) & (KF_EXTENDED | 0xff));
			if (scancode == 0)
				scancode = (i32)MapVirtualKeyW((u32)msg.wParam, MAPVK_VK_TO_VSC);

			switch (scancode) {
				case 0x54: scancode = 0x137; break; /*  Alt+PrtS */
				case 0x146: scancode = 0x45; break; /* Ctrl+Pause */
				case 0x136: scancode = 0x36; break; /*  CJK IME sets the extended bit for right Shift */
				default: break;
			}

			win->event.key = (u8)RGFW_apiKeyToRGFW((u32) scancode);
			if (msg.wParam == VK_CONTROL) {
				if (HIWORD(msg.lParam) & KF_EXTENDED)
					win->event.key = RGFW_controlR;
				else win->event.key = RGFW_controlL;
			}

			wchar_t charBuffer;
			ToUnicodeEx((UINT)msg.wParam, (UINT)scancode, keyboardState, &charBuffer, 1, 0, NULL);
			win->event.keyChar = (u8)charBuffer;

			RGFW_keyboard[win->event.key].prev = RGFW_keyboard[win->event.key].current;

			win->event.type = RGFW_keyPressed;
			win->event.repeat = RGFW_isPressed(win, win->event.key);
			RGFW_keyboard[win->event.key].current = 1;
			RGFW_updateKeyMods(win, (GetKeyState(VK_CAPITAL) & 0x0001), (GetKeyState(VK_NUMLOCK) & 0x0001), (GetKeyState(VK_SCROLL) & 0x0001));

			RGFW_keyCallback(win, win->event.key, win->event.keyChar, win->event.keyMod, 1);
			break;
		}
		case WM_MOUSEMOVE: {
			if ((win->_flags & RGFW_HOLD_MOUSE))
				break;

			win->event.type = RGFW_mousePosChanged;

			i32 x = GET_X_LPARAM(msg.lParam);
			i32 y = GET_Y_LPARAM(msg.lParam);

			RGFW_mousePosCallback(win, win->event.point, win->event.vector);

			if (win->_flags & RGFW_MOUSE_LEFT) {
				win->_flags &= ~(u32)RGFW_MOUSE_LEFT;
				win->event.type = RGFW_mouseEnter;
				RGFW_mouseNotifyCallback(win, win->event.point, 1);
			}

			win->event.point.x = x;
			win->event.point.y = y;
			win->_lastMousePoint = RGFW_POINT(x, y);

			break;
		}
		case WM_INPUT: {
			if (!(win->_flags & RGFW_HOLD_MOUSE))
				break;

			unsigned size = sizeof(RAWINPUT);
			static RAWINPUT raw;

			GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));

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

				win->event.vector.x = pos.x - win->_lastMousePoint.x;
				win->event.vector.y = pos.y - win->_lastMousePoint.y;
			} else {
				win->event.vector.x = raw.data.mouse.lLastX;
				win->event.vector.y = raw.data.mouse.lLastY;
			}

			win->event.type = RGFW_mousePosChanged;
			win->_lastMousePoint.x += win->event.vector.x;
			win->_lastMousePoint.y += win->event.vector.y;
			win->event.point = win->_lastMousePoint;
			RGFW_mousePosCallback(win, win->event.point, win->event.vector);
			break;
		}
		case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN: case WM_XBUTTONDOWN:
			if (msg.message == WM_XBUTTONDOWN)
				win->event.button = RGFW_mouseMisc1 + (GET_XBUTTON_WPARAM(msg.wParam) == XBUTTON2);
			else win->event.button = (msg.message == WM_LBUTTONDOWN) ? RGFW_mouseLeft :
									 (msg.message == WM_RBUTTONDOWN) ? RGFW_mouseRight : RGFW_mouseMiddle;

			win->event.type = RGFW_mouseButtonPressed;
			RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
			RGFW_mouseButtons[win->event.button].current = 1;
			RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 1);
			break;
		case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP: case WM_XBUTTONUP:
			if (msg.message == WM_XBUTTONUP)
				win->event.button = RGFW_mouseMisc1 + (GET_XBUTTON_WPARAM(msg.wParam) == XBUTTON2);
			else win->event.button = (msg.message == WM_LBUTTONUP) ? RGFW_mouseLeft :
									 (msg.message == WM_RBUTTONUP) ? RGFW_mouseRight : RGFW_mouseMiddle;
			win->event.type = RGFW_mouseButtonReleased;
			RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
			RGFW_mouseButtons[win->event.button].current = 0;
			RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 0);
			break;
		case WM_MOUSEWHEEL:
			if (msg.wParam > 0)
				win->event.button = RGFW_mouseScrollUp;
			else
				win->event.button = RGFW_mouseScrollDown;

			RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
			RGFW_mouseButtons[win->event.button].current = 1;

			win->event.scroll = (SHORT) HIWORD(msg.wParam) / (double) WHEEL_DELTA;

			win->event.type = RGFW_mouseButtonPressed;
			RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 1);
			break;
		case WM_DROPFILES: {
			win->event.type = RGFW_DNDInit;

			drop = (HDROP) msg.wParam;
			POINT pt;

			/* Move the mouse to the position of the drop */
			DragQueryPoint(drop, &pt);

			win->event.point.x = pt.x;
			win->event.point.y = pt.y;

			RGFW_dndInitCallback(win, win->event.point);
		}
			break;
		default:
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
			return RGFW_window_checkEvent(win);
	}

    TranslateMessage(&msg);
	DispatchMessageA(&msg);

	return &win->event;
}

RGFW_bool RGFW_window_isHidden(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	return IsWindowVisible(win->src.window) == 0 && !RGFW_window_isMinimized(win);
}

RGFW_bool RGFW_window_isMinimized(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	#ifndef __cplusplus
	WINDOWPLACEMENT placement = { 0 };
	#else
	WINDOWPLACEMENT placement = {  };
	#endif
	GetWindowPlacement(win->src.window, &placement);
	return placement.showCmd == SW_SHOWMINIMIZED;
}

RGFW_bool RGFW_window_isMaximized(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	#ifndef __cplusplus
	WINDOWPLACEMENT placement = { 0 };
	#else
	WINDOWPLACEMENT placement = {  };
	#endif
	GetWindowPlacement(win->src.window, &placement);
	return placement.showCmd == SW_SHOWMAXIMIZED || IsZoomed(win->src.window);
}

typedef struct { int iIndex; HMONITOR hMonitor; RGFW_monitor* monitors; } RGFW_mInfo;
#ifndef RGFW_NO_MONITOR
RGFW_monitor win32CreateMonitor(HMONITOR src);
RGFW_monitor win32CreateMonitor(HMONITOR src) {
	RGFW_monitor monitor;
	MONITORINFOEX  monitorInfo;

	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfoA(src, (LPMONITORINFO)&monitorInfo);

	/* get the monitor's index */
	DISPLAY_DEVICEA dd;
	dd.cb = sizeof(dd);

    DWORD deviceNum;
	for (deviceNum = 0; EnumDisplayDevicesA(NULL, deviceNum, &dd, 0); deviceNum++) {
		if (!(dd.StateFlags & DISPLAY_DEVICE_ACTIVE))
			continue;

		DEVMODEA dm;
		ZeroMemory(&dm, sizeof(dm));
		dm.dmSize = sizeof(dm);

		if (EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) {
			monitor.mode.refreshRate = dm.dmDisplayFrequency;
			RGFW_splitBPP(dm.dmBitsPerPel, &monitor.mode);
		}

		DISPLAY_DEVICEA mdd;
		mdd.cb = sizeof(mdd);

		if (EnumDisplayDevicesA(dd.DeviceName, (DWORD)deviceNum, &mdd, 0)) {
			RGFW_STRNCPY(monitor.name, mdd.DeviceString, sizeof(monitor.name) - 1);
			monitor.name[sizeof(monitor.name) - 1] = '\0';
			break;
		}
	}




	monitor.x = monitorInfo.rcWork.left;
	monitor.y = monitorInfo.rcWork.top;
	monitor.mode.area.w = (u32)(monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
	monitor.mode.area.h = (u32)(monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);

	HDC hdc = CreateDC(monitorInfo.szDevice, NULL, NULL, NULL);
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

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoMonitor, RGFW_DEBUG_CTX_MON(monitor), "monitor found");
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

	info->monitors[info->iIndex] = win32CreateMonitor(hMonitor);
	info->iIndex++;

	return TRUE;
}

RGFW_monitor RGFW_getPrimaryMonitor(void) {
	#ifdef __cplusplus
	return win32CreateMonitor(MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
	#else
	return win32CreateMonitor(MonitorFromPoint((POINT) { 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
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
	return win32CreateMonitor(src);
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
				dm.dmPelsWidth = mode.area.w;
				dm.dmPelsHeight = mode.area.h;
            }

			if (request & RGFW_monitorRefresh) {
				dm.dmFields |= DM_DISPLAYFREQUENCY;
				dm.dmDisplayFrequency = mode.refreshRate;
			}

			if (request & RGFW_monitorRGB) {
				dm.dmFields |= DM_BITSPERPEL;
				dm.dmBitsPerPel = (DWORD)(mode.red + mode.green + mode.blue);
			}

			if (ChangeDisplaySettingsEx(dd.DeviceName, &dm, NULL, CDS_TEST, NULL) == DISP_CHANGE_SUCCESSFUL) {
				if (ChangeDisplaySettingsEx(dd.DeviceName, &dm, NULL, CDS_UPDATEREGISTRY, NULL) == DISP_CHANGE_SUCCESSFUL)
					return RGFW_TRUE;
				return RGFW_FALSE;
			} else return RGFW_FALSE;
		}
	}

	return RGFW_FALSE;
}

#endif
HICON RGFW_loadHandleImage(u8* src, i32 c, RGFW_area a, BOOL icon);
HICON RGFW_loadHandleImage(u8* src, i32 c, RGFW_area a, BOOL icon) {
    size_t channels = (size_t)c;

	BITMAPV5HEADER bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bV5Size = sizeof(bi);
	bi.bV5Width = (i32)a.w;
	bi.bV5Height = -((LONG) a.h);
	bi.bV5Planes = 1;
	bi.bV5BitCount = (WORD)(channels * 8);
	bi.bV5Compression = BI_RGB;
	HDC dc = GetDC(NULL);
	u8* target = NULL;

	HBITMAP color = CreateDIBSection(dc,
		(BITMAPINFO*) &bi, DIB_RGB_COLORS, (void**) &target,
		NULL, (DWORD) 0);

    size_t x, y;
    for (y = 0; y < a.h; y++) {
        for (x = 0; x < a.w; x++) {
			size_t index = (y * 4 * (size_t)a.w) + x * channels;
            target[index] = src[index + 2];
            target[index + 1] = src[index + 1];
            target[index + 2] = src[index];
            target[index + 3] = src[index + 3];
        }
    }

    ReleaseDC(NULL, dc);

	HBITMAP mask = CreateBitmap((i32)a.w, (i32)a.h, 1, 1, NULL);

	ICONINFO ii;
	ZeroMemory(&ii, sizeof(ii));
	ii.fIcon = icon;
	ii.xHotspot = a.w / 2;
	ii.yHotspot = a.h / 2;
	ii.hbmMask = mask;
	ii.hbmColor = color;

	HICON handle = CreateIconIndirect(&ii);

	DeleteObject(color);
	DeleteObject(mask);

	return handle;
}

void* RGFW_loadMouse(u8* icon, RGFW_area a, i32 channels) {
	HCURSOR cursor = (HCURSOR) RGFW_loadHandleImage(icon, channels, a, FALSE);
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
	if (win->_flags & RGFW_windowFocusOnShow) RGFW_window_focus(win);
	ShowWindow(win->src.window, SW_RESTORE);
}

#define RGFW_FREE_LIBRARY(x) if (x != NULL) FreeLibrary(x); x = NULL;
void RGFW_deinit(void) {
    #ifndef RGFW_NO_XINPUT
    RGFW_FREE_LIBRARY(RGFW_XInput_dll);
    #endif

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
    _RGFW.root = NULL;

    RGFW_freeMouse(_RGFW.hiddenMouse);
    _RGFW.windowCount = -1;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal, RGFW_DEBUG_CTX(NULL, 0), "global context deinitialized");
}


void RGFW_window_close(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	#ifdef RGFW_BUFFER
		DeleteDC(win->src.hdcMem);
		DeleteObject(win->src.bitmap);
	#endif

	if ((win->_flags & RGFW_windowNoInitAPI) == 0) RGFW_window_freeOpenGL(win);
	RemovePropW(win->src.window, L"RGFW");
	ReleaseDC(win->src.window, win->src.hdc); /*!< delete device context */
	DestroyWindow(win->src.window); /*!< delete window */

	if (win->src.hIconSmall) DestroyIcon(win->src.hIconSmall);
	if (win->src.hIconBig) DestroyIcon(win->src.hIconBig);

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, RGFW_DEBUG_CTX(win, 0), "a window was freed");
    _RGFW.windowCount--;
	if (_RGFW.windowCount == 0) RGFW_deinit();

    RGFW_clipboard_switch(NULL);
	RGFW_FREE(win->event.droppedFiles);
	if ((win->_flags & RGFW_WINDOW_ALLOC)) {
		RGFW_FREE(win);
        win = NULL;
    }
}

void RGFW_window_move(RGFW_window* win, RGFW_point v) {
	RGFW_ASSERT(win != NULL);

	win->r.x = v.x;
	win->r.y = v.y;
	SetWindowPos(win->src.window, HWND_TOP, win->r.x, win->r.y, 0, 0, SWP_NOSIZE);
}

void RGFW_window_resize(RGFW_window* win, RGFW_area a) {
	RGFW_ASSERT(win != NULL);

	win->r.w = (i32)a.w;
	win->r.h = (i32)a.h;
	SetWindowPos(win->src.window, HWND_TOP, 0, 0, win->r.w, win->r.h + (i32)win->src.hOffset, SWP_NOMOVE);
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

RGFW_bool RGFW_window_setIconEx(RGFW_window* win, u8* src, RGFW_area a, i32 channels, u8 type) {
	RGFW_ASSERT(win != NULL);
	#ifndef RGFW_WIN95
		RGFW_UNUSED(channels);

		if (win->src.hIconSmall && (type & RGFW_iconWindow)) DestroyIcon(win->src.hIconSmall);
		if (win->src.hIconBig && (type & RGFW_iconTaskbar)) DestroyIcon(win->src.hIconBig);

		if (src == NULL) {
			HICON defaultIcon = LoadIcon(NULL, IDI_APPLICATION);
			if (type & RGFW_iconWindow)
				SendMessage(win->src.window, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)defaultIcon);
			if (type & RGFW_iconTaskbar)
				SendMessage(win->src.window, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)defaultIcon);
			return RGFW_TRUE;
		}

		if (type & RGFW_iconWindow) {
			win->src.hIconSmall = RGFW_loadHandleImage(src, channels, a, TRUE);
			SendMessage(win->src.window, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)win->src.hIconSmall);
		}
		if (type & RGFW_iconTaskbar) {
			win->src.hIconBig = RGFW_loadHandleImage(src, channels, a, TRUE);
			SendMessage(win->src.window, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)win->src.hIconBig);
		}
		return RGFW_TRUE;
	#else
		RGFW_UNUSED(src);
		RGFW_UNUSED(a);
		RGFW_UNUSED(channels);
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

			str[textLen] = '\0';
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

	if (!OpenClipboard(_RGFW.root->src.window)) {
		GlobalFree(object);
		return;
	}

	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, object);
	CloseClipboard();
}

void RGFW_window_moveMouse(RGFW_window* win, RGFW_point p) {
	RGFW_ASSERT(win != NULL);
	win->_lastMousePoint = RGFW_POINT(p.x - win->r.x, p.y - win->r.y);
	SetCursorPos(p.x, p.y);
}

#ifdef RGFW_OPENGL
void RGFW_window_makeCurrent_OpenGL(RGFW_window* win) {
	if (win == NULL)
		wglMakeCurrent(NULL, NULL);
	else
		wglMakeCurrent(win->src.hdc, (HGLRC) win->src.ctx);
}
void* RGFW_getCurrent_OpenGL(void) { return wglGetCurrentContext(); }
void RGFW_window_swapBuffers_OpenGL(RGFW_window* win){ SwapBuffers(win->src.hdc); }
#endif

#ifndef RGFW_EGL
void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) {
	RGFW_ASSERT(win != NULL);
#if defined(RGFW_OPENGL)
    if (wglSwapIntervalEXT == NULL || wglSwapIntervalEXT(swapInterval) == FALSE)
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(win, 0), "Failed to set swap interval");
#else
	RGFW_UNUSED(swapInterval);
#endif
}
#endif

void RGFW_window_swapBuffers_software(RGFW_window* win) {
#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
	if (win->buffer != win->src.bitmapBits)
	memcpy(win->src.bitmapBits, win->buffer, win->bufferSize.w * win->bufferSize.h * 4);

	RGFW_RGB_to_BGR(win, win->src.bitmapBits);
	BitBlt(win->src.hdc, 0, 0, win->r.w, win->r.h, win->src.hdcMem, 0, 0, SRCCOPY);
#else
	RGFW_UNUSED(win);
#endif
}

char* RGFW_createUTF8FromWideStringWin32(const WCHAR* source) {
	if (source == NULL) {
		return NULL;
	}
	i32 size = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
	if (!size) {
		return NULL;
	}

	static char target[RGFW_MAX_PATH * 2];
	if (size > RGFW_MAX_PATH * 2)
		size = RGFW_MAX_PATH * 2;

	target[size] = 0;

	if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, NULL, NULL)) {
		return NULL;
	}

	return target;
}

u64 RGFW_getTimerFreq(void) {
	static u64 frequency = 0;
	if (frequency == 0) QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

	return frequency;
}

u64 RGFW_getTimerValue(void) {
	u64 value;
	QueryPerformanceCounter((LARGE_INTEGER*)&value);
	return value;
}

void RGFW_sleep(u64 ms) {
	Sleep((u32)ms);
}

#ifndef RGFW_NO_THREADS

RGFW_thread RGFW_createThread(RGFW_threadFunc_ptr ptr, void* args) { return CreateThread(NULL, 0, ptr, args, 0, NULL); }
void RGFW_cancelThread(RGFW_thread thread) { CloseHandle((HANDLE) thread); }
void RGFW_joinThread(RGFW_thread thread) { WaitForSingleObject((HANDLE) thread, INFINITE); }
void RGFW_setThreadPriority(RGFW_thread thread, u8 priority) { SetThreadPriority((HANDLE) thread, priority); }

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

typedef CGRect NSRect;
typedef CGPoint NSPoint;
typedef CGSize NSSize;

typedef const char* NSPasteboardType;
typedef unsigned long NSUInteger;
typedef long NSInteger;
typedef NSInteger NSModalResponse;

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

id NSApp = NULL;

#define NSRelease(obj) objc_msgSend_void((id)obj, sel_registerName("release"))
id NSString_stringWithUTF8String(const char* str);
id NSString_stringWithUTF8String(const char* str) {
	return ((id(*)(id, SEL, const char*))objc_msgSend)
		((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), str);
}

const char* NSString_to_char(id str);
const char* NSString_to_char(id str) {
	return ((const char* (*)(id, SEL)) objc_msgSend) ((id)(id)str, sel_registerName("UTF8String"));
}

void si_impl_func_to_SEL_with_name(const char* class_name, const char* register_name, void* function);
void si_impl_func_to_SEL_with_name(const char* class_name, const char* register_name, void* function) {
	Class selected_class;

	if (RGFW_STRNCMP(class_name, "NSView", 6) == 0) {
		selected_class = objc_getClass("ViewClass");
	} else if (RGFW_STRNCMP(class_name, "NSWindow", 8) == 0) {
		selected_class = objc_getClass("WindowClass");
	} else {
		selected_class = objc_getClass(class_name);
	}

	class_addMethod(selected_class, sel_registerName(register_name), (IMP) function, 0);
}

/* Header for the array. */
typedef struct siArrayHeader {
	size_t count;
	/* TODO(EimaMei): Add a `type_width` later on. */
} siArrayHeader;

/* Gets the header of the siArray. */
#define SI_ARRAY_HEADER(s) ((siArrayHeader*)s - 1)
#define si_array_len(array) (SI_ARRAY_HEADER(array)->count)
#define si_func_to_SEL(class_name, function) si_impl_func_to_SEL_with_name(class_name, #function":", (void*)function)
/* Creates an Objective-C method (SEL) from a regular C function with the option to set the register name.*/
#define si_func_to_SEL_with_name(class_name, register_name, function) si_impl_func_to_SEL_with_name(class_name, register_name":", (void*)function)

unsigned char* NSBitmapImageRep_bitmapData(id imageRep);
unsigned char* NSBitmapImageRep_bitmapData(id imageRep) {
	return ((unsigned char* (*)(id, SEL))objc_msgSend) ((id)imageRep, sel_registerName("bitmapData"));
}

typedef RGFW_ENUM(NSUInteger, NSBitmapFormat) {
	NSBitmapFormatAlphaFirst = 1 << 0,       /* 0 means is alpha last (RGBA, CMYKA, etc.) */
		NSBitmapFormatAlphaNonpremultiplied = 1 << 1,       /* 0 means is premultiplied */
		NSBitmapFormatFloatingpointSamples = 1 << 2,  /* 0 is integer */

		NSBitmapFormatSixteenBitLittleEndian = (1 << 8),
		NSBitmapFormatThirtyTwoBitLittleEndian = (1 << 9),
		NSBitmapFormatSixteenBitBigEndian = (1 << 10),
		NSBitmapFormatThirtyTwoBitBigEndian = (1 << 11)
};

id NSBitmapImageRep_initWithBitmapData(unsigned char** planes, NSInteger width, NSInteger height, NSInteger bps, NSInteger spp, bool alpha, bool isPlanar, const char* colorSpaceName, NSBitmapFormat bitmapFormat, NSInteger rowBytes, NSInteger pixelBits);
id NSBitmapImageRep_initWithBitmapData(unsigned char** planes, NSInteger width, NSInteger height, NSInteger bps, NSInteger spp, bool alpha, bool isPlanar, const char* colorSpaceName, NSBitmapFormat bitmapFormat, NSInteger rowBytes, NSInteger pixelBits) {
	SEL func = sel_registerName("initWithBitmapDataPlanes:pixelsWide:pixelsHigh:bitsPerSample:samplesPerPixel:hasAlpha:isPlanar:colorSpaceName:bitmapFormat:bytesPerRow:bitsPerPixel:");

	return (id) ((id(*)(id, SEL, unsigned char**, NSInteger, NSInteger, NSInteger, NSInteger, bool, bool, id, NSBitmapFormat, NSInteger, NSInteger))objc_msgSend)
		(NSAlloc((id)objc_getClass("NSBitmapImageRep")), func, planes, width, height, bps, spp, alpha, isPlanar, NSString_stringWithUTF8String(colorSpaceName), bitmapFormat, rowBytes, pixelBits);
}

id NSColor_colorWithSRGB(CGFloat red, CGFloat green, CGFloat blue, CGFloat alpha);
id NSColor_colorWithSRGB(CGFloat red, CGFloat green, CGFloat blue, CGFloat alpha) {
	void* nsclass = objc_getClass("NSColor");
	SEL func = sel_registerName("colorWithSRGBRed:green:blue:alpha:");
	return ((id(*)(id, SEL, CGFloat, CGFloat, CGFloat, CGFloat))objc_msgSend)
		((id)nsclass, func, red, green, blue, alpha);
}

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
void NSOpenGLContext_setValues(id context, const int* vals, NSOpenGLContextParameter param);
void NSOpenGLContext_setValues(id context, const int* vals, NSOpenGLContextParameter param) {
	((void (*)(id, SEL, const int*, NSOpenGLContextParameter))objc_msgSend)
		(context, sel_registerName("setValues:forParameter:"), vals, param);
}
void* NSOpenGLPixelFormat_initWithAttributes(const uint32_t* attribs);
void* NSOpenGLPixelFormat_initWithAttributes(const uint32_t* attribs) {
	return (void*) ((id(*)(id, SEL, const uint32_t*))objc_msgSend)
		(NSAlloc((id)objc_getClass("NSOpenGLPixelFormat")), sel_registerName("initWithAttributes:"), attribs);
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
	id nsstr = NSString_stringWithUTF8String(dataType);
	id nsString = ((id(*)(id, SEL, id))objc_msgSend)(pasteboard, func, nsstr);
	const char* str = NSString_to_char(nsString);
	if (len != NULL)
		*len = (size_t)((NSUInteger(*)(id, SEL, int))objc_msgSend)(nsString, sel_registerName("maximumLengthOfBytesUsingEncoding:"), 4);
	return str;
}

id c_array_to_NSArray(void* array, size_t len);
id c_array_to_NSArray(void* array, size_t len) {
	SEL func = sel_registerName("initWithObjects:count:");
	void* nsclass = objc_getClass("NSArray");
	return ((id (*)(id, SEL, void*, NSUInteger))objc_msgSend)
				(NSAlloc(nsclass), func, array, len);
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

NSPasteboardType const NSPasteboardTypeString = "public.utf8-plain-text"; /* Replaces NSStringPasteboardType */


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

void* NSArray_objectAtIndex(id array, NSUInteger index) {
	SEL func = sel_registerName("objectAtIndex:");
	return ((id(*)(id, SEL, NSUInteger))objc_msgSend)(array, func, index);
}

id NSWindow_contentView(id window) {
	SEL func = sel_registerName("contentView");
	return objc_msgSend_id(window, func);
}

/*
	End of cocoa wrapper
*/

#ifdef RGFW_OPENGL
/* MacOS opengl API spares us yet again (there are no extensions) */
RGFW_bool RGFW_extensionSupportedPlatform(const char * extension, size_t len) { RGFW_UNUSED(extension); RGFW_UNUSED(len); return RGFW_FALSE; }
CFBundleRef RGFWnsglFramework = NULL;

RGFW_proc RGFW_getProcAddress(const char* procname) {
	if (RGFWnsglFramework == NULL)
		RGFWnsglFramework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));

	CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault, procname, kCFStringEncodingASCII);

	RGFW_proc symbol = (RGFW_proc)CFBundleGetFunctionPointerForName(RGFWnsglFramework, symbolName);

	CFRelease(symbolName);

	return symbol;
}
#endif

id NSWindow_delegate(RGFW_window* win) {
	return (id) objc_msgSend_id((id)win->src.window, sel_registerName("delegate"));
}

u32 RGFW_OnClose(id self) {
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, (const char*)"RGFW_window", (void**)&win);
	if (win == NULL)
		return true;

	RGFW_eventQueuePushEx(e.type = RGFW_quit; e._win = win);
	RGFW_windowQuitCallback(win);

	return false;
}

/* NOTE(EimaMei): Fixes the constant clicking when the app is running under a terminal. */
bool acceptsFirstResponder(void) { return true; }
bool performKeyEquivalent(id event) { RGFW_UNUSED(event); return true; }

NSDragOperation draggingEntered(id self, SEL sel, id sender) {
	RGFW_UNUSED(sender); RGFW_UNUSED(self); RGFW_UNUSED(sel);

	return NSDragOperationCopy;
}
NSDragOperation draggingUpdated(id self, SEL sel, id sender) {
	RGFW_UNUSED(sel);

	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL || (!(win->_flags & RGFW_windowAllowDND)))
		return 0;

	NSPoint p = ((NSPoint(*)(id, SEL)) objc_msgSend)(sender, sel_registerName("draggingLocation"));
	RGFW_eventQueuePushEx(e.type = RGFW_DNDInit;
									e.point = RGFW_POINT((u32) p.x, (u32) (win->r.h - p.y));
									e._win = win);

	RGFW_dndInitCallback(win, win->event.point);
	return NSDragOperationCopy;
}
bool prepareForDragOperation(id self) {
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL)
		return true;

	if (!(win->_flags & RGFW_windowAllowDND)) {
		return false;
	}

	return true;
}

void RGFW__osxDraggingEnded(id self, SEL sel, id sender);
void RGFW__osxDraggingEnded(id self, SEL sel, id sender) { RGFW_UNUSED(sender); RGFW_UNUSED(self); RGFW_UNUSED(sel);  return; }

/* NOTE(EimaMei): Usually, you never need 'id self, SEL cmd' for C -> Obj-C methods. This isn't the case. */
bool performDragOperation(id self, SEL sel, id sender) {
	RGFW_UNUSED(sender); RGFW_UNUSED(self); RGFW_UNUSED(sel);

	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);

	if (win == NULL)
		return false;

	/* id pasteBoard = objc_msgSend_id(sender, sel_registerName("draggingPasteboard")); */

	id pasteBoard = objc_msgSend_id(sender, sel_registerName("draggingPasteboard"));

	/* Get the types of data available on the pasteboard */
	id types = objc_msgSend_id(pasteBoard, sel_registerName("types"));

	/* Get the string type for file URLs */
	id fileURLsType = objc_msgSend_class_char(objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), "NSFilenamesPboardType");

	/* Check if the pasteboard contains file URLs */
	if (objc_msgSend_id_bool(types, sel_registerName("containsObject:"), fileURLsType) == 0) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errClipboard, RGFW_DEBUG_CTX(win, 0), "No files found on the pasteboard.");
		return 0;
	}

	id fileURLs = objc_msgSend_id_id(pasteBoard, sel_registerName("propertyListForType:"), fileURLsType);
	int count = ((int (*)(id, SEL))objc_msgSend)(fileURLs, sel_registerName("count"));

	if (count == 0)
		return 0;

    int i;
    for (i = 0; i < count; i++) {
		id fileURL = objc_msgSend_arr(fileURLs, sel_registerName("objectAtIndex:"), i);
		const char *filePath = ((const char* (*)(id, SEL))objc_msgSend)(fileURL, sel_registerName("UTF8String"));
		RGFW_STRNCPY(win->event.droppedFiles[i], filePath, RGFW_MAX_PATH - 1);
		win->event.droppedFiles[i][RGFW_MAX_PATH - 1] = '\0';
	}
	NSPoint p = ((NSPoint(*)(id, SEL)) objc_msgSend)(sender, sel_registerName("draggingLocation"));
	
	win->event.droppedFilesCount = (size_t)count; 
	RGFW_eventQueuePushEx(e.type = RGFW_DND;
									e.point = RGFW_POINT((u32) p.x, (u32) (win->r.h - p.y));
									e.droppedFilesCount = (size_t)count;
									e._win = win);
	
	RGFW_dndCallback(win, win->event.droppedFiles, win->event.droppedFilesCount);

	return false;
}

#ifndef RGFW_NO_IOKIT
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDManager.h>

u32 RGFW_osx_getFallbackRefreshRate(CGDirectDisplayID displayID) {
    u32 refreshRate = 0;
    io_iterator_t it;
    io_service_t service;
    CFNumberRef indexRef, clockRef, countRef;
    uint32_t clock, count;

#ifdef kIOMainPortDefault 
    if (IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching("IOFramebuffer"), &it) != 0)
#elif defined(kIOMasterPortDefault) 
    if (IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching("IOFramebuffer"), &it) != 0)
#endif
        return RGFW_FALSE;

    while ((service = IOIteratorNext(it)) != 0) {
        uint32_t index;
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
                    refreshRate = (u32)RGFW_ROUND(clock / (double) count);
                    CFRelease(countRef);
                }
            }
            CFRelease(clockRef);
        }
    }

    IOObjectRelease(it);
    return refreshRate;
}

IOHIDDeviceRef RGFW_osxControllers[4] = {NULL};

size_t findControllerIndex(IOHIDDeviceRef device) {
    size_t i;
    for (i = 0; i < 4; i++)
		if (RGFW_osxControllers[i] == device)
			return i;
	return (size_t)-1;
}

void RGFW__osxInputValueChangedCallback(void *context, IOReturn result, void *sender, IOHIDValueRef value) {
	RGFW_UNUSED(context); RGFW_UNUSED(result); RGFW_UNUSED(sender);
	IOHIDElementRef element = IOHIDValueGetElement(value);

	IOHIDDeviceRef device = IOHIDElementGetDevice(element);
	size_t index = findControllerIndex(device);
	if (index == (size_t)-1) return;

	uint32_t usagePage = IOHIDElementGetUsagePage(element);
	uint32_t usage = IOHIDElementGetUsage(element);

	CFIndex intValue = IOHIDValueGetIntegerValue(value);

	u8 RGFW_osx2RGFWSrc[2][RGFW_gamepadFinal] = {{
		0, RGFW_gamepadSelect, RGFW_gamepadL3, RGFW_gamepadR3, RGFW_gamepadStart,
		RGFW_gamepadUp, RGFW_gamepadRight, RGFW_gamepadDown, RGFW_gamepadLeft,
		RGFW_gamepadL2, RGFW_gamepadR2, RGFW_gamepadL1, RGFW_gamepadR1,
		RGFW_gamepadY, RGFW_gamepadB, RGFW_gamepadA, RGFW_gamepadX, RGFW_gamepadHome},
		{0, RGFW_gamepadA, RGFW_gamepadB, RGFW_gamepadR3, RGFW_gamepadX,
		RGFW_gamepadY, RGFW_gamepadRight, RGFW_gamepadL1, RGFW_gamepadR1,
		RGFW_gamepadL2, RGFW_gamepadR2, RGFW_gamepadDown, RGFW_gamepadStart,
		RGFW_gamepadUp, RGFW_gamepadL3, RGFW_gamepadSelect, RGFW_gamepadStart, RGFW_gamepadHome}
	};

	u8* RGFW_osx2RGFW = RGFW_osx2RGFWSrc[0];
	if (RGFW_gamepads_type[index] == RGFW_gamepadMicrosoft)
		RGFW_osx2RGFW = RGFW_osx2RGFWSrc[1];

	switch (usagePage) {
		case kHIDPage_Button: {
			u8 button = 0;
			if (usage < sizeof(RGFW_osx2RGFW))
				button = RGFW_osx2RGFW[usage];

			RGFW_gamepadButtonCallback(_RGFW.root, (u16)index, button, (u8)intValue);
			RGFW_gamepadPressed[index][button].prev = RGFW_gamepadPressed[index][button].current;
			RGFW_gamepadPressed[index][button].current = RGFW_BOOL(intValue);
			RGFW_eventQueuePushEx(e.type = intValue ? RGFW_gamepadButtonPressed: RGFW_gamepadButtonReleased;
											e.button = button;
											e.gamepad = (u16)index;
											e._win = _RGFW.root);
			break;
		}
		case kHIDPage_GenericDesktop: {
			CFIndex logicalMin = IOHIDElementGetLogicalMin(element);
			CFIndex logicalMax = IOHIDElementGetLogicalMax(element);

			if (logicalMax <= logicalMin) return;
			if (intValue < logicalMin) intValue = logicalMin;
			if (intValue > logicalMax) intValue = logicalMax;

			i8 axisValue = (i8)(-100.0 + ((intValue - logicalMin) * 200.0) / (logicalMax - logicalMin));

			u8 whichAxis = 0;
			switch (usage) {
				case kHIDUsage_GD_X: RGFW_gamepadAxes[index][0].x = axisValue; whichAxis = 0; break;
				case kHIDUsage_GD_Y: RGFW_gamepadAxes[index][0].y = axisValue; whichAxis = 0; break;
				case kHIDUsage_GD_Z: RGFW_gamepadAxes[index][1].x = axisValue; whichAxis = 1; break;
				case kHIDUsage_GD_Rz: RGFW_gamepadAxes[index][1].y = axisValue; whichAxis = 1; break;
				default: return;
			}

            RGFW_event e;
            e.type = RGFW_gamepadAxisMove;
            e.gamepad = (u16)index;
            e.whichAxis = whichAxis;
            e._win = _RGFW.root;
            for (size_t i = 0; i < 4; i++)
                e.axis[i] = RGFW_gamepadAxes[index][i]; 

			RGFW_eventQueuePush(e);

			RGFW_gamepadAxisCallback(_RGFW.root, (u16)index, RGFW_gamepadAxes[index], 2, whichAxis);
		}
	}
}

void RGFW__osxDeviceAddedCallback(void* context, IOReturn result, void *sender, IOHIDDeviceRef device) {
	RGFW_UNUSED(context); RGFW_UNUSED(result); RGFW_UNUSED(sender);
	CFTypeRef usageRef = (CFTypeRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsageKey));
	int usage = 0;
	if (usageRef)
		CFNumberGetValue((CFNumberRef)usageRef, kCFNumberIntType, (void*)&usage);

	if (usage != kHIDUsage_GD_Joystick && usage != kHIDUsage_GD_GamePad && usage != kHIDUsage_GD_MultiAxisController) {
		return;
	}

    size_t i;
    for (i = 0; i < 4; i++) {
		if (RGFW_osxControllers[i] != NULL)
			continue;

		RGFW_osxControllers[i] = device;

		IOHIDDeviceRegisterInputValueCallback(device, RGFW__osxInputValueChangedCallback, NULL);

		CFStringRef deviceName = (CFStringRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
		if (deviceName)
			CFStringGetCString(deviceName, RGFW_gamepads_name[i], sizeof(RGFW_gamepads_name[i]), kCFStringEncodingUTF8);

		RGFW_gamepads_type[i] = RGFW_gamepadUnknown;
		if (RGFW_STRSTR(RGFW_gamepads_name[i], "Microsoft") || RGFW_STRSTR(RGFW_gamepads_name[i], "X-Box") || RGFW_STRSTR(RGFW_gamepads_name[i], "Xbox"))
			RGFW_gamepads_type[i] = RGFW_gamepadMicrosoft;
		else if (RGFW_STRSTR(RGFW_gamepads_name[i], "PlayStation") || RGFW_STRSTR(RGFW_gamepads_name[i], "PS3") || RGFW_STRSTR(RGFW_gamepads_name[i], "PS4") || RGFW_STRSTR(RGFW_gamepads_name[i], "PS5"))
			RGFW_gamepads_type[i] = RGFW_gamepadSony;
		else if (RGFW_STRSTR(RGFW_gamepads_name[i], "Nintendo"))
			RGFW_gamepads_type[i] = RGFW_gamepadNintendo;
		else if (RGFW_STRSTR(RGFW_gamepads_name[i], "Logitech"))
			RGFW_gamepads_type[i] = RGFW_gamepadLogitech;

		RGFW_gamepads[i] = (u16)i;
		RGFW_gamepadCount++;

		RGFW_eventQueuePushEx(e.type = RGFW_gamepadConnected;
										e.gamepad = (u16)i;
										e._win = _RGFW.root);

		RGFW_gamepadCallback(_RGFW.root, (u16)i, 1);
		break;
	}
}

void RGFW__osxDeviceRemovedCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
	RGFW_UNUSED(context); RGFW_UNUSED(result); RGFW_UNUSED(sender); RGFW_UNUSED(device);
	CFNumberRef usageRef = (CFNumberRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsageKey));
	int usage = 0;
	if (usageRef)
		CFNumberGetValue(usageRef, kCFNumberIntType, &usage);

	if (usage != kHIDUsage_GD_Joystick && usage != kHIDUsage_GD_GamePad && usage != kHIDUsage_GD_MultiAxisController) {
		return;
	}

	size_t index = findControllerIndex(device);
	if (index != (size_t)-1)
		RGFW_osxControllers[index] = NULL;

	RGFW_eventQueuePushEx(e.type = RGFW_gamepadDisconnected;
									e.gamepad = (u16)index;
									e._win = _RGFW.root);
	RGFW_gamepadCallback(_RGFW.root, (u16)index, 0);

	RGFW_gamepadCount--;
}

RGFWDEF void RGFW_osxInitIOKit(void);
void RGFW_osxInitIOKit(void) {
	IOHIDManagerRef hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (!hidManager) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errIOKit, RGFW_DEBUG_CTX(_RGFW.root, 0), "Failed to create IOHIDManager.");
		return;
	}

	CFMutableDictionaryRef matchingDictionary = CFDictionaryCreateMutable(
		kCFAllocatorDefault,
		0,
		&kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks
	);
	if (!matchingDictionary) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errIOKit, RGFW_DEBUG_CTX(_RGFW.root, 0), "Failed to create matching dictionary for IOKit.");
		CFRelease(hidManager);
		return;
	}

	CFDictionarySetValue(
		matchingDictionary,
		CFSTR(kIOHIDDeviceUsagePageKey),
		CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, (int[]){kHIDPage_GenericDesktop})
	);

	IOHIDManagerSetDeviceMatching(hidManager, matchingDictionary);

	IOHIDManagerRegisterDeviceMatchingCallback(hidManager, RGFW__osxDeviceAddedCallback, NULL);
	IOHIDManagerRegisterDeviceRemovalCallback(hidManager, RGFW__osxDeviceRemovedCallback, NULL);

	IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);

	/* Execute the run loop once in order to register any initially-attached joysticks */
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
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


void RGFW__osxWindowDeminiaturize(id self, SEL sel) {
	RGFW_UNUSED(sel);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

	win->_flags |= RGFW_windowMinimize;
	RGFW_eventQueuePushEx(e.type = RGFW_windowRestored; e._win = win);
	RGFW_windowRestoredCallback(win, win->r);

}
void RGFW__osxWindowMiniaturize(id self, SEL sel) {
	RGFW_UNUSED(sel);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

	win->_flags &= ~(u32)RGFW_windowMinimize;
	RGFW_eventQueuePushEx(e.type = RGFW_windowMinimized; e._win = win);
	RGFW_windowMinimizedCallback(win, win->r);

}

void RGFW__osxWindowBecameKey(id self, SEL sel) {
	RGFW_UNUSED(sel);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

	win->_flags |= RGFW_windowFocus;
	RGFW_eventQueuePushEx(e.type = RGFW_focusIn; e._win = win);

	RGFW_focusCallback(win, RGFW_TRUE);

	if ((win->_flags & RGFW_HOLD_MOUSE)) RGFW_window_mouseHold(win, RGFW_AREA(win->r.w, win->r.h));
}

void RGFW__osxWindowResignKey(id self, SEL sel) {
	RGFW_UNUSED(sel);
	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

    RGFW_window_focusLost(win);
    RGFW_eventQueuePushEx(e.type = RGFW_focusOut; e._win = win);
	RGFW_focusCallback(win, RGFW_FALSE);
}

NSSize RGFW__osxWindowResize(id self, SEL sel, NSSize frameSize) {
	RGFW_UNUSED(sel);

	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return frameSize;

	win->r.w = (i32)frameSize.width;
	win->r.h = (i32)frameSize.height;

	RGFW_monitor mon = RGFW_window_getMonitor(win);
	if ((i32)mon.mode.area.w == win->r.w && (i32)mon.mode.area.h - 102 <= win->r.h) {
		win->_flags |= RGFW_windowMaximize;
		RGFW_eventQueuePushEx(e.type = RGFW_windowMaximized; e._win = win);
		RGFW_windowMaximizedCallback(win, win->r);
	} else if (win->_flags & RGFW_windowMaximize) {
		win->_flags &= ~(u32)RGFW_windowMaximize;
		RGFW_eventQueuePushEx(e.type = RGFW_windowRestored; e._win = win);
		RGFW_windowRestoredCallback(win, win->r);

	}


	RGFW_eventQueuePushEx(e.type = RGFW_windowResized; e._win = win);
	RGFW_windowResizedCallback(win, win->r);
	return frameSize;
}

void RGFW__osxWindowMove(id self, SEL sel) {
	RGFW_UNUSED(sel);

	RGFW_window* win = NULL;
	object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

	NSRect frame = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)((id)win->src.window, sel_registerName("frame"));
	win->r.x = (i32) frame.origin.x;
	win->r.y = (i32) frame.origin.y;

	RGFW_eventQueuePushEx(e.type = RGFW_windowMoved; e._win = win);
	RGFW_windowMovedCallback(win, win->r);
}

void RGFW__osxViewDidChangeBackingProperties(id self, SEL _cmd) {
	RGFW_UNUSED(_cmd);
        RGFW_window* win = NULL;
        object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

	RGFW_monitor mon = RGFW_window_getMonitor(win);
	RGFW_scaleUpdatedCallback(win, mon.scaleX, mon.scaleY);
	RGFW_eventQueuePushEx(e.type = RGFW_scaleUpdated; e.scaleX = mon.scaleX; e.scaleY = mon.scaleY ; e._win = win);
}

void RGFW__osxDrawRect(id self, SEL _cmd, CGRect rect) {
	RGFW_UNUSED(rect); RGFW_UNUSED(_cmd);
        RGFW_window* win = NULL;
        object_getInstanceVariable(self, "RGFW_window", (void**)&win);
	if (win == NULL) return;

        RGFW_eventQueuePushEx(e.type = RGFW_windowRefresh; e._win = win);
        RGFW_windowRefreshCallback(win);
}

void RGFW_window_initBufferPtr(RGFW_window* win, u8* buffer, RGFW_area area) {
	#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
		win->buffer = buffer;
		win->bufferSize = area;
		win->_flags |= RGFW_BUFFER_ALLOC;
	#ifdef RGFW_OSMESA
		win->src.ctx = OSMesaCreateContext(OSMESA_RGBA, NULL);
		OSMesaMakeCurrent(win->src.ctx, win->buffer, GL_UNSIGNED_BYTE, area.w, area.h);
		OSMesaPixelStore(OSMESA_Y_UP, 0);
	#endif
	#else
		RGFW_UNUSED(win);  RGFW_UNUSED(buffer); RGFW_UNUSED(area); /*!< if buffer rendering is not being used */
	#endif
}

void RGFW_window_cocoaSetLayer(RGFW_window* win, void* layer) {
	objc_msgSend_void_id((id)win->src.view, sel_registerName("setLayer"), (id)layer);
}

void* RGFW_cocoaGetLayer(void) {
	return objc_msgSend_class((id)objc_getClass("CAMetalLayer"), (SEL)sel_registerName("layer"));
}

NSPasteboardType const NSPasteboardTypeURL = "public.url";
NSPasteboardType const NSPasteboardTypeFileURL  = "public.file-url";

id RGFW__osx_generateViewClass(const char* subclass, RGFW_window* win) {
	Class customViewClass;
	customViewClass = objc_allocateClassPair(objc_getClass(subclass), "RGFWCustomView", 0);

	class_addIvar( customViewClass, "RGFW_window", sizeof(RGFW_window*), (u8)rint(log2(sizeof(RGFW_window*))), "L");
	class_addMethod(customViewClass, sel_registerName("drawRect:"), (IMP)RGFW__osxDrawRect, "v@:{CGRect=ffff}");
	class_addMethod(customViewClass, sel_registerName("viewDidChangeBackingProperties"), (IMP)RGFW__osxViewDidChangeBackingProperties, "");

	id customView  = objc_msgSend_id(NSAlloc(customViewClass), sel_registerName("init"));
	object_setInstanceVariable(customView, "RGFW_window", win);

	return customView;
}

#ifndef RGFW_EGL
void RGFW_window_initOpenGL(RGFW_window* win) {
#ifdef RGFW_OPENGL
	void* attrs = RGFW_initFormatAttribs();
	void* format = NSOpenGLPixelFormat_initWithAttributes((uint32_t*)attrs);

	if (format == NULL) {
		RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(win, 0), "Failed to load pixel format for OpenGL");
        win->_flags |= RGFW_windowOpenglSoftware;
        void* subAttrs = RGFW_initFormatAttribs();
		format = NSOpenGLPixelFormat_initWithAttributes((uint32_t*)subAttrs);

		if (format == NULL)
			RGFW_sendDebugInfo(RGFW_typeError, RGFW_errOpenglContext, RGFW_DEBUG_CTX(win, 0), "and loading software rendering OpenGL failed");
		else
			RGFW_sendDebugInfo(RGFW_typeWarning, RGFW_warningOpenGL, RGFW_DEBUG_CTX(win, 0), "Switching to software rendering");
	}

	/* the pixel format can be passed directly to opengl context creation to create a context
		this is because the format also includes information about the opengl version (which may be a bad thing) */

	win->src.view = (id) ((id(*)(id, SEL, NSRect, uint32_t*))objc_msgSend) (RGFW__osx_generateViewClass("NSOpenGLView", win),
							sel_registerName("initWithFrame:pixelFormat:"), (NSRect){{0, 0}, {win->r.w, win->r.h}}, (uint32_t*)format);

	objc_msgSend_void(win->src.view, sel_registerName("prepareOpenGL"));
	win->src.ctx = objc_msgSend_id(win->src.view, sel_registerName("openGLContext"));

	if (win->_flags & RGFW_windowTransparent) {
		i32 opacity = 0;
		#define NSOpenGLCPSurfaceOpacity 236
		NSOpenGLContext_setValues((id)win->src.ctx, &opacity, NSOpenGLCPSurfaceOpacity);
	}

	objc_msgSend_void(win->src.ctx, sel_registerName("makeCurrentContext"));
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, RGFW_DEBUG_CTX(win, 0), "opengl context initalized");
#else
	RGFW_UNUSED(win);
#endif
}

void RGFW_window_freeOpenGL(RGFW_window* win) {
#ifdef RGFW_OPENGL
	if (win->src.ctx == NULL) return;
	objc_msgSend_void(win->src.ctx, sel_registerName("release"));
	win->src.ctx = NULL;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, RGFW_DEBUG_CTX(win, 0), "opengl context freed");
#else
	RGFW_UNUSED(win);
#endif
}
#endif


i32 RGFW_init(void) {
#if defined(RGFW_C89) || defined(__cplusplus)
    if (_RGFW_init) return 0; 
    _RGFW_init = RGFW_TRUE;
    _RGFW.root = NULL; _RGFW.current = NULL; _RGFW.windowCount = -1; _RGFW.eventLen = 0; _RGFW.eventIndex = 0;
#endif

    /* NOTE(EimaMei): Why does Apple hate good code? Like wtf, who thought of methods being a great idea???
	Imagine a universe, where MacOS had a proper system API (we would probably have like 20% better performance).
	*/
	si_func_to_SEL_with_name("NSObject", "windowShouldClose", (void*)RGFW_OnClose);

	/* NOTE(EimaMei): Fixes the 'Boop' sfx from constantly playing each time you click a key. Only a problem when running in the terminal. */
	si_func_to_SEL("NSWindow", acceptsFirstResponder);
	si_func_to_SEL("NSWindow", performKeyEquivalent);

    if (NSApp == NULL) {
		NSApp = objc_msgSend_id((id)objc_getClass("NSApplication"), sel_registerName("sharedApplication"));

		((void (*)(id, SEL, NSUInteger))objc_msgSend)
			(NSApp, sel_registerName("setActivationPolicy:"), NSApplicationActivationPolicyRegular);

		#ifndef RGFW_NO_IOKIT
			RGFW_osxInitIOKit();
		#endif
	}

    _RGFW.windowCount = 0;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal, RGFW_DEBUG_CTX(NULL, 0), "global context initialized");
    return 0;
}

RGFW_window* RGFW_createWindowPtr(const char* name, RGFW_rect rect, RGFW_windowFlags flags, RGFW_window* win) {
	static u8 RGFW_loaded = 0;
	RGFW_window_basic_init(win, rect, flags);

    /* RR Create an autorelease pool */
	id pool = objc_msgSend_class(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc"));
	pool = objc_msgSend_id(pool, sel_registerName("init"));

	RGFW_window_setMouseDefault(win);

	NSRect windowRect;
	windowRect.origin.x = win->r.x;
	windowRect.origin.y = win->r.y;
	windowRect.size.width = win->r.w;
	windowRect.size.height = win->r.h;

	NSBackingStoreType macArgs = NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSBackingStoreBuffered | NSWindowStyleMaskTitled;

	if (!(flags & RGFW_windowNoResize))
		macArgs |= NSWindowStyleMaskResizable;
	if (!(flags & RGFW_windowNoBorder))
		macArgs |= NSWindowStyleMaskTitled;
	{
		void* nsclass = objc_getClass("NSWindow");
		SEL func = sel_registerName("initWithContentRect:styleMask:backing:defer:");

		win->src.window = ((id(*)(id, SEL, NSRect, NSWindowStyleMask, NSBackingStoreType, bool))objc_msgSend)
			(NSAlloc(nsclass), func, windowRect, macArgs, macArgs, false);
	}

	id str = NSString_stringWithUTF8String(name);
	objc_msgSend_void_id((id)win->src.window, sel_registerName("setTitle:"), str);

	if ((flags & RGFW_windowNoInitAPI) == 0) {
		RGFW_window_initOpenGL(win);
        RGFW_window_initBuffer(win);
    }

	#ifdef RGFW_OPENGL
	else
	#endif
	{
		NSRect contentRect = (NSRect){{0, 0}, {win->r.w, win->r.h}};
		win->src.view = ((id(*)(id, SEL, NSRect))objc_msgSend) (NSAlloc(objc_getClass("NSView")), sel_registerName("initWithFrame:"), contentRect);
    	}

	void* contentView = NSWindow_contentView((id)win->src.window);
	objc_msgSend_void_bool(contentView, sel_registerName("setWantsLayer:"), true);
	objc_msgSend_int((id)win->src.view, sel_registerName("setLayerContentsPlacement:"),  4);
	objc_msgSend_void_id((id)win->src.window, sel_registerName("setContentView:"), win->src.view);

	if (flags & RGFW_windowTransparent) {
		objc_msgSend_void_bool(win->src.window, sel_registerName("setOpaque:"), false);

		objc_msgSend_void_id((id)win->src.window, sel_registerName("setBackgroundColor:"),
			NSColor_colorWithSRGB(0, 0, 0, 0));
	}

	Class delegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "WindowDelegate", 0);

	class_addIvar(
		delegateClass, "RGFW_window",
		sizeof(RGFW_window*), (u8)rint(log2(sizeof(RGFW_window*))),
		"L"
	);

	class_addMethod(delegateClass, sel_registerName("windowWillResize:toSize:"), (IMP) RGFW__osxWindowResize, "{NSSize=ff}@:{NSSize=ff}");
	class_addMethod(delegateClass, sel_registerName("windowWillMove:"), (IMP) RGFW__osxWindowMove, "");
	class_addMethod(delegateClass, sel_registerName("windowDidMove:"), (IMP) RGFW__osxWindowMove, "");
	class_addMethod(delegateClass, sel_registerName("windowDidMiniaturize:"), (IMP) RGFW__osxWindowMiniaturize, "");
	class_addMethod(delegateClass, sel_registerName("windowDidDeminiaturize:"), (IMP) RGFW__osxWindowDeminiaturize, "");
	class_addMethod(delegateClass, sel_registerName("windowDidBecomeKey:"), (IMP) RGFW__osxWindowBecameKey, "");
	class_addMethod(delegateClass, sel_registerName("windowDidResignKey:"), (IMP) RGFW__osxWindowResignKey, "");
	class_addMethod(delegateClass, sel_registerName("draggingEntered:"), (IMP)draggingEntered, "l@:@");
	class_addMethod(delegateClass, sel_registerName("draggingUpdated:"), (IMP)draggingUpdated, "l@:@");
	class_addMethod(delegateClass, sel_registerName("draggingExited:"), (IMP)RGFW__osxDraggingEnded, "v@:@");
	class_addMethod(delegateClass, sel_registerName("draggingEnded:"), (IMP)RGFW__osxDraggingEnded, "v@:@");
	class_addMethod(delegateClass, sel_registerName("prepareForDragOperation:"), (IMP)prepareForDragOperation, "B@:@");
	class_addMethod(delegateClass, sel_registerName("performDragOperation:"), (IMP)performDragOperation, "B@:@");

	id delegate = objc_msgSend_id(NSAlloc(delegateClass), sel_registerName("init"));

	if (RGFW_COCOA_FRAME_NAME)
		objc_msgSend_ptr(win->src.view, sel_registerName("setFrameAutosaveName:"), RGFW_COCOA_FRAME_NAME);

	object_setInstanceVariable(delegate, "RGFW_window", win);

	objc_msgSend_void_id((id)win->src.window, sel_registerName("setDelegate:"), delegate);

	if (flags & RGFW_windowAllowDND) {
		win->_flags |= RGFW_windowAllowDND;

		NSPasteboardType types[] = {NSPasteboardTypeURL, NSPasteboardTypeFileURL, NSPasteboardTypeString};
		NSregisterForDraggedTypes((id)win->src.window, types, 3);
	}

	RGFW_window_setFlags(win, flags);

    /* Show the window */
	objc_msgSend_void_bool(NSApp, sel_registerName("activateIgnoringOtherApps:"), true);
	((id(*)(id, SEL, SEL))objc_msgSend)((id)win->src.window, sel_registerName("makeKeyAndOrderFront:"), NULL);
	RGFW_window_show(win);

	if (!RGFW_loaded) {
		objc_msgSend_void(win->src.window, sel_registerName("makeMainWindow"));

		RGFW_loaded = 1;
	}

	objc_msgSend_void(win->src.window, sel_registerName("makeKeyWindow"));

	objc_msgSend_void(NSApp, sel_registerName("finishLaunching"));
	NSRetain(win->src.window);
	NSRetain(NSApp);

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, RGFW_DEBUG_CTX(win, 0), "a new  window was created");
	return win;
}

void RGFW_window_setBorder(RGFW_window* win, RGFW_bool border) {
	NSRect frame = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)((id)win->src.window, sel_registerName("frame"));
	NSRect content = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)((id)win->src.view, sel_registerName("frame"));
	float offset = 0;

	RGFW_setBit(&win->_flags, RGFW_windowNoBorder, !border);
	NSBackingStoreType storeType = NSWindowStyleMaskBorderless | NSWindowStyleMaskFullSizeContentView;
	if (border)
		storeType = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
	if (!(win->_flags & RGFW_windowNoResize)) {
		storeType |= NSWindowStyleMaskResizable;
	}

	((void (*)(id, SEL, NSBackingStoreType))objc_msgSend)((id)win->src.window, sel_registerName("setStyleMask:"), storeType);

	if (!border) {
		id miniaturizeButton = objc_msgSend_int((id)win->src.window, sel_registerName("standardWindowButton:"),  NSWindowMiniaturizeButton);
		id titleBarView = objc_msgSend_id(miniaturizeButton, sel_registerName("superview"));
		objc_msgSend_void_bool(titleBarView, sel_registerName("setHidden:"), true);

		offset = (float)(frame.size.height - content.size.height);
	}

	RGFW_window_resize(win, RGFW_AREA(win->r.w, win->r.h + offset));
	win->r.h -= (i32)offset;
}

RGFW_area RGFW_getScreenSize(void) {
	static CGDirectDisplayID display = 0;

	if (display == 0)
		display = CGMainDisplayID();

	return RGFW_AREA(CGDisplayPixelsWide(display), CGDisplayPixelsHigh(display));
}

RGFW_point RGFW_getGlobalMousePoint(void) {
	RGFW_ASSERT(_RGFW.root != NULL);

	CGEventRef e = CGEventCreate(NULL);
	CGPoint point = CGEventGetLocation(e);
	CFRelease(e);

	return RGFW_POINT((u32) point.x, (u32) point.y); /*!< the point is loaded during event checks */
}

typedef RGFW_ENUM(u32, NSEventType) {        /* various types of events */
	NSEventTypeLeftMouseDown = 1,
		NSEventTypeLeftMouseUp = 2,
		NSEventTypeRightMouseDown = 3,
		NSEventTypeRightMouseUp = 4,
		NSEventTypeMouseMoved = 5,
		NSEventTypeLeftMouseDragged = 6,
		NSEventTypeRightMouseDragged = 7,
		NSEventTypeMouseEntered = 8,
		NSEventTypeMouseExited = 9,
		NSEventTypeKeyDown = 10,
		NSEventTypeKeyUp = 11,
		NSEventTypeFlagsChanged = 12,
		NSEventTypeAppKitDefined = 13,
		NSEventTypeSystemDefined = 14,
		NSEventTypeApplicationDefined = 15,
		NSEventTypePeriodic = 16,
		NSEventTypeCursorUpdate = 17,
		NSEventTypeScrollWheel = 22,
		NSEventTypeTabletPoint = 23,
		NSEventTypeTabletProximity = 24,
		NSEventTypeOtherMouseDown = 25,
		NSEventTypeOtherMouseUp = 26,
		NSEventTypeOtherMouseDragged = 27,
		/* The following event types are available on some hardware on 10.5.2 and later */
		NSEventTypeGesture  = 29,
		NSEventTypeMagnify  = 30,
		NSEventTypeSwipe    = 31,
		NSEventTypeRotate   = 18,
		NSEventTypeBeginGesture  = 19,
		NSEventTypeEndGesture  = 20,

		NSEventTypeSmartMagnify  = 32,
		NSEventTypeQuickLook  = 33,

		NSEventTypePressure  = 34,
		NSEventTypeDirectTouch  = 37,

		NSEventTypeChangeMode  = 38,
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

void RGFW_stopCheckEvents(void) {
	id eventPool = objc_msgSend_class(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc"));
	eventPool = objc_msgSend_id(eventPool, sel_registerName("init"));

	id e = (id) ((id(*)(Class, SEL, NSEventType, NSPoint, NSEventModifierFlags, void*, NSInteger, void**, short, NSInteger, NSInteger))objc_msgSend)
		(objc_getClass("NSEvent"), sel_registerName("otherEventWithType:location:modifierFlags:timestamp:windowNumber:context:subtype:data1:data2:"),
			NSEventTypeApplicationDefined, (NSPoint){0, 0}, (NSEventModifierFlags)0, NULL, (NSInteger)0, NULL, 0, 0, 0);

	((void (*)(id, SEL, id, bool))objc_msgSend)
		(NSApp, sel_registerName("postEvent:atStart:"), e, 1);

	objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
}

void RGFW_window_eventWait(RGFW_window* win, i32 waitMS) {
	RGFW_UNUSED(win);

	id eventPool = objc_msgSend_class(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc"));
	eventPool = objc_msgSend_id(eventPool, sel_registerName("init"));

	void* date = (void*) ((id(*)(Class, SEL, double))objc_msgSend)
				(objc_getClass("NSDate"), sel_registerName("dateWithTimeIntervalSinceNow:"), waitMS);

	SEL eventFunc = sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:");
	id e = (id) ((id(*)(id, SEL, NSEventMask, void*, id, bool))objc_msgSend)
		(NSApp, eventFunc,
			ULONG_MAX, date, NSString_stringWithUTF8String("kCFRunLoopDefaultMode"), true);

	if (e) {
		((void (*)(id, SEL, id, bool))objc_msgSend)
			(NSApp, sel_registerName("postEvent:atStart:"), e, 1);
	}

	objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
}

u8 RGFW_rgfwToKeyChar(u32 rgfw_keycode) {
    return (u8)rgfw_keycode; /* TODO */
}

RGFW_event* RGFW_window_checkEvent(RGFW_window* win) {
    if (win == NULL || ((win->_flags & RGFW_windowFreeOnClose) && (win->_flags & RGFW_EVENT_QUIT))) return NULL;

    objc_msgSend_void((id)win->src.mouse, sel_registerName("set"));
    RGFW_event* ev =  RGFW_window_checkEventCore(win);
	if (ev) {
		((void(*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("updateWindows"));
		return ev;
	}

	id eventPool = objc_msgSend_class(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc"));
	eventPool = objc_msgSend_id(eventPool, sel_registerName("init"));

	SEL eventFunc = sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:");

	void* date = NULL;

	id e = (id) ((id(*)(id, SEL, NSEventMask, void*, id, bool))objc_msgSend)
		(NSApp, eventFunc, ULONG_MAX, date, NSString_stringWithUTF8String("kCFRunLoopDefaultMode"), true);

	if (e == NULL) {
		objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
		objc_msgSend_void_id(NSApp, sel_registerName("sendEvent:"), e);
		((void(*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("updateWindows"));
		return NULL;
	}

	if (objc_msgSend_id(e, sel_registerName("window")) != win->src.window) {
		((void (*)(id, SEL, id, bool))objc_msgSend)
			(NSApp, sel_registerName("postEvent:atStart:"), e, 0);

		objc_msgSend_void_id(NSApp, sel_registerName("sendEvent:"), e);
		objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
		((void(*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("updateWindows"));
		return NULL;
	}

	if (win->event.droppedFilesCount) {
		u32 i;
		for (i = 0; i < win->event.droppedFilesCount; i++)
			win->event.droppedFiles[i][0] = '\0';
	}

	win->event.droppedFilesCount = 0;
	win->event.type = 0;

	u32 type = (u32)objc_msgSend_uint(e, sel_registerName("type"));
	switch (type) {
		case NSEventTypeMouseEntered: {
			win->event.type = RGFW_mouseEnter;
			NSPoint p = ((NSPoint(*)(id, SEL)) objc_msgSend)(e, sel_registerName("locationInWindow"));

			win->event.point = RGFW_POINT((i32) p.x, (i32) (win->r.h - p.y));
			RGFW_mouseNotifyCallback(win, win->event.point, 1);
			break;
		}

		case NSEventTypeMouseExited:
			win->event.type = RGFW_mouseLeave;
			RGFW_mouseNotifyCallback(win, win->event.point, 0);
			break;

		case NSEventTypeKeyDown: {
			u32 key = (u16) objc_msgSend_uint(e, sel_registerName("keyCode"));

			u32 mappedKey = (u32)*(((char*)(const char*) NSString_to_char(objc_msgSend_id(e, sel_registerName("charactersIgnoringModifiers")))));
			if (((u8)mappedKey) == 239)
				mappedKey = 0;

			win->event.keyChar = (u8)mappedKey;

			win->event.key = (u8)RGFW_apiKeyToRGFW(key);
			RGFW_keyboard[win->event.key].prev = RGFW_keyboard[win->event.key].current;

			win->event.type = RGFW_keyPressed;
			win->event.repeat = RGFW_isPressed(win, win->event.key);
			RGFW_keyboard[win->event.key].current = 1;

			RGFW_keyCallback(win, win->event.key, win->event.keyChar, win->event.keyMod, 1);
			break;
		}

		case NSEventTypeKeyUp: {
			u32 key = (u16) objc_msgSend_uint(e, sel_registerName("keyCode"));
			u32 mappedKey = (u32)*(((char*)(const char*) NSString_to_char(objc_msgSend_id(e, sel_registerName("charactersIgnoringModifiers")))));
			if (((u8)mappedKey) == 239)
				mappedKey = 0;

			win->event.keyChar = (u8)mappedKey;

			win->event.key = (u8)RGFW_apiKeyToRGFW(key);

			RGFW_keyboard[win->event.key].prev = RGFW_keyboard[win->event.key].current;

			win->event.type = RGFW_keyReleased;

			RGFW_keyboard[win->event.key].current = 0;
			RGFW_keyCallback(win, win->event.key, win->event.keyChar, win->event.keyMod, 0);
			break;
		}

		case NSEventTypeFlagsChanged: {
			u32 flags = (u32)objc_msgSend_uint(e, sel_registerName("modifierFlags"));
			RGFW_updateKeyModsPro(win, ((u32)(flags & NSEventModifierFlagCapsLock) % 255), ((flags & NSEventModifierFlagNumericPad) % 255),
										((flags & NSEventModifierFlagControl) % 255), ((flags & NSEventModifierFlagOption) % 255),
										((flags & NSEventModifierFlagShift) % 255), ((flags & NSEventModifierFlagCommand) % 255), 0);
			u8 i;
			for (i = 0; i < 9; i++)
				RGFW_keyboard[i + RGFW_capsLock].prev = 0;

			for (i = 0; i < 5; i++) {
				u32 shift = (1 << (i + 16));
				u32 key = i + RGFW_capsLock;

				if ((flags & shift) && !RGFW_wasPressed(win, (u8)key)) {
					RGFW_keyboard[key].current = 1;

					if (key != RGFW_capsLock)
						RGFW_keyboard[key+ 4].current = 1;

					win->event.type = RGFW_keyPressed;
					win->event.key = (u8)key;
					break;
				}

				if (!(flags & shift) && RGFW_wasPressed(win, (u8)key)) {
					RGFW_keyboard[key].current = 0;

					if (key != RGFW_capsLock)
						RGFW_keyboard[key + 4].current = 0;

					win->event.type = RGFW_keyReleased;
					win->event.key = (u8)key;
					break;
				}
			}

			RGFW_keyCallback(win, win->event.key, win->event.keyChar, win->event.keyMod, win->event.type == RGFW_keyPressed);

			break;
		}
		case NSEventTypeLeftMouseDragged:
		case NSEventTypeOtherMouseDragged:
		case NSEventTypeRightMouseDragged:
		case NSEventTypeMouseMoved: {
			win->event.type = RGFW_mousePosChanged;
			NSPoint p = ((NSPoint(*)(id, SEL)) objc_msgSend)(e, sel_registerName("locationInWindow"));
			win->event.point = RGFW_POINT((u32) p.x, (u32) (win->r.h - p.y));

			p.x = ((CGFloat(*)(id, SEL))abi_objc_msgSend_fpret)(e, sel_registerName("deltaX"));
			p.y = ((CGFloat(*)(id, SEL))abi_objc_msgSend_fpret)(e, sel_registerName("deltaY"));
			win->event.vector = RGFW_POINT((i32)p.x, (i32)p.y);

			win->_lastMousePoint = win->event.point;
			RGFW_mousePosCallback(win, win->event.point, win->event.vector);
			break;
		}
		case NSEventTypeLeftMouseDown: case NSEventTypeRightMouseDown: case NSEventTypeOtherMouseDown: {
			u32 buttonNumber = (u32)objc_msgSend_uint(e, sel_registerName("buttonNumber"));
			switch (buttonNumber) {
				case 0: win->event.button = RGFW_mouseLeft; break;
				case 1: win->event.button = RGFW_mouseRight; break;
				case 2: win->event.button = RGFW_mouseMiddle; break;
				default: win->event.button = (u8)buttonNumber;
			}

			win->event.type = RGFW_mouseButtonPressed;
			RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
			RGFW_mouseButtons[win->event.button].current = 1;
			RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 1);
			break;
		}
		case NSEventTypeLeftMouseUp: case NSEventTypeRightMouseUp: case NSEventTypeOtherMouseUp: {
			u32 buttonNumber = (u32)objc_msgSend_uint(e, sel_registerName("buttonNumber"));
			switch (buttonNumber) {
				case 0: win->event.button = RGFW_mouseLeft; break;
				case 1: win->event.button = RGFW_mouseRight; break;
				case 2: win->event.button = RGFW_mouseMiddle; break;
				default: win->event.button = (u8)buttonNumber;
			}
			RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
			RGFW_mouseButtons[win->event.button].current = 0;
			win->event.type = RGFW_mouseButtonReleased;
			RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 0);
			break;
		}
		case NSEventTypeScrollWheel: {
			double deltaY = ((CGFloat(*)(id, SEL))abi_objc_msgSend_fpret)(e, sel_registerName("deltaY"));

			if (deltaY > 0) {
				win->event.button = RGFW_mouseScrollUp;
			}
			else if (deltaY < 0) {
				win->event.button = RGFW_mouseScrollDown;
			}

			RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
			RGFW_mouseButtons[win->event.button].current = 1;

			win->event.scroll = deltaY;

			win->event.type = RGFW_mouseButtonPressed;
			RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 1);
			break;
		}

		default:
			objc_msgSend_void_id(NSApp, sel_registerName("sendEvent:"), e);
			((void(*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("updateWindows"));
			return RGFW_window_checkEvent(win);
	}

	objc_msgSend_void_id(NSApp, sel_registerName("sendEvent:"), e);
	((void(*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("updateWindows"));
	objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
	return &win->event;
}


void RGFW_window_move(RGFW_window* win, RGFW_point v) {
	RGFW_ASSERT(win != NULL);

	win->r.x = v.x;
	win->r.y = v.y;
	((void(*)(id, SEL, NSRect, bool, bool))objc_msgSend)
		((id)win->src.window, sel_registerName("setFrame:display:animate:"), (NSRect){{win->r.x, win->r.y}, {win->r.w, win->r.h}}, true, true);
}

void RGFW_window_resize(RGFW_window* win, RGFW_area a) {
	RGFW_ASSERT(win != NULL);

	NSRect frame = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)((id)win->src.window, sel_registerName("frame"));
	NSRect content = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)((id)win->src.view, sel_registerName("frame"));
	float offset = (float)(frame.size.height - content.size.height);

	win->r.w = (i32)a.w;
	win->r.h = (i32)a.h;

	((void(*)(id, SEL, NSRect, bool, bool))objc_msgSend)
		((id)win->src.window, sel_registerName("setFrame:display:animate:"), (NSRect){{win->r.x, win->r.y}, {win->r.w, win->r.h + offset}}, true, true);
}

void RGFW_window_focus(RGFW_window* win) {
	RGFW_ASSERT(win);
	objc_msgSend_void_bool(NSApp, sel_registerName("activateIgnoringOtherApps:"), true);
	((void (*)(id, SEL))objc_msgSend)((id)win->src.window, sel_registerName("makeKeyWindow"));
}

void RGFW_window_raise(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	((id(*)(id, SEL, SEL))objc_msgSend)((id)win->src.window, sel_registerName("orderFront:"), (SEL)NULL);
    	objc_msgSend_void_id(win->src.window, sel_registerName("setLevel:"), kCGNormalWindowLevelKey);
}

void RGFW_window_setFullscreen(RGFW_window* win, RGFW_bool fullscreen) {
	RGFW_ASSERT(win != NULL);
	if (fullscreen && (win->_flags & RGFW_windowFullscreen)) return;
	if (!fullscreen && !(win->_flags & RGFW_windowFullscreen)) return;

	if (fullscreen) {
		win->_oldRect = win->r;
		RGFW_monitor mon = RGFW_window_getMonitor(win);
		win->r = RGFW_RECT(0, 0, mon.x, mon.y);
		win->_flags |= RGFW_windowFullscreen;
		RGFW_window_resize(win, RGFW_AREA(mon.mode.area.w, mon.mode.area.h));
		RGFW_window_move(win, RGFW_POINT(0, 0));
	}
	objc_msgSend_void_SEL(win->src.window, sel_registerName("toggleFullScreen:"), NULL);

	if (!fullscreen) {
		win->r = win->_oldRect;
		win->_flags &= ~(u32)RGFW_windowFullscreen;

		RGFW_window_resize(win, RGFW_AREA(win->r.w, win->r.h));
		RGFW_window_move(win, RGFW_POINT(win->r.x, win->r.y));
	}
}

void RGFW_window_maximize(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	if (RGFW_window_isMaximized(win)) return;

	win->_flags |= RGFW_windowMaximize;
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

void RGFW_window_setAspectRatio(RGFW_window* win, RGFW_area a) {
	if (a.w == 0 && a.h == 0) a = RGFW_AREA(1, 1);

	((void (*)(id, SEL, NSSize))objc_msgSend)
		((id)win->src.window, sel_registerName("setContentAspectRatio:"), (NSSize){a.w, a.h});
}

void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a) {
	((void (*)(id, SEL, NSSize))objc_msgSend)
		((id)win->src.window, sel_registerName("setMinSize:"), (NSSize){a.w, a.h});
}

void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a) {
	if (a.w == 0 && a.h == 0) {
		a = RGFW_getScreenSize();
	}

	((void (*)(id, SEL, NSSize))objc_msgSend)
		((id)win->src.window, sel_registerName("setMaxSize:"), (NSSize){a.w, a.h});
}

RGFW_bool RGFW_window_setIconEx(RGFW_window* win, u8* data, RGFW_area area, i32 channels, u8 type) {
	RGFW_ASSERT(win != NULL);
	RGFW_UNUSED(type);

	if (data == NULL) {
		objc_msgSend_void_id(NSApp, sel_registerName("setApplicationIconImage:"), NULL);
		return RGFW_TRUE;
	}

	/* code by EimaMei: Make a bitmap representation, then copy the loaded image into it. */
	id representation = NSBitmapImageRep_initWithBitmapData(NULL, area.w, area.h, 8, channels, (channels == 4), false, "NSCalibratedRGBColorSpace", 1 << 1, area.w * (u32)channels, 8 * (u32)channels);
	RGFW_MEMCPY(NSBitmapImageRep_bitmapData(representation), data, area.w * area.h * (u32)channels);

	/* Add ze representation. */
	id dock_image = ((id(*)(id, SEL, NSSize))objc_msgSend) (NSAlloc((id)objc_getClass("NSImage")), sel_registerName("initWithSize:"), ((NSSize){area.w, area.h}));

	objc_msgSend_void_id(dock_image, sel_registerName("addRepresentation:"), representation);

	/* Finally, set the dock image to it. */
	objc_msgSend_void_id(NSApp, sel_registerName("setApplicationIconImage:"), dock_image);
	/* Free the garbage. */
	NSRelease(dock_image);
	NSRelease(representation);

	return RGFW_TRUE;
}

id NSCursor_arrowStr(const char* str) {
	void* nclass = objc_getClass("NSCursor");
	SEL func = sel_registerName(str);
	return (id) objc_msgSend_id(nclass, func);
}

RGFW_mouse* RGFW_loadMouse(u8* icon, RGFW_area a, i32 channels) {
	if (icon == NULL) {
		objc_msgSend_void(NSCursor_arrowStr("arrowCursor"), sel_registerName("set"));
		return NULL;
	}

	/* NOTE(EimaMei): Code by yours truly. */
	/* Make a bitmap representation, then copy the loaded image into it. */
	id representation = (id)NSBitmapImageRep_initWithBitmapData(NULL, a.w, a.h, 8, channels, (channels == 4), false, "NSCalibratedRGBColorSpace", 1 << 1, a.w * (u32)channels, 8 * (u32)channels);
	RGFW_MEMCPY(NSBitmapImageRep_bitmapData(representation), icon, a.w * a.h * (u32)channels);

	/* Add ze representation. */
	id cursor_image = ((id(*)(id, SEL, NSSize))objc_msgSend) (NSAlloc((id)objc_getClass("NSImage")), sel_registerName("initWithSize:"), ((NSSize){a.w, a.h}));

	objc_msgSend_void_id(cursor_image, sel_registerName("addRepresentation:"), representation);

	/* Finally, set the cursor image. */
	id cursor = (id) ((id(*)(id, SEL, id, NSPoint))objc_msgSend)
		(NSAlloc(objc_getClass("NSCursor")),  sel_registerName("initWithImage:hotSpot:"), cursor_image, (NSPoint){0.0, 0.0});

	/* Free the garbage. */
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

void RGFW_captureCursor(RGFW_window* win, RGFW_rect r) {
	RGFW_UNUSED(win);

	CGWarpMouseCursorPosition((CGPoint){r.x + (r.w / 2), r.y + (r.h / 2)});
	CGAssociateMouseAndMouseCursorPosition(0);
}

void RGFW_window_moveMouse(RGFW_window* win, RGFW_point v) {
	RGFW_UNUSED(win);

	win->_lastMousePoint = RGFW_POINT(v.x - win->r.x, v.y - win->r.y);
	CGWarpMouseCursorPosition((CGPoint){v.x, v.y});
}


void RGFW_window_hide(RGFW_window* win) {
	objc_msgSend_void_bool(win->src.window, sel_registerName("setIsVisible:"), false);
}

void RGFW_window_show(RGFW_window* win) {
	if (win->_flags & RGFW_windowFocusOnShow)
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

u32 RGFW_osx_getFallbackRefreshRate(CGDirectDisplayID displayID);

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

RGFW_monitor RGFW_NSCreateMonitor(CGDirectDisplayID display, id screen) {
	RGFW_monitor monitor;

	const char name[] = "MacOS\0";
	RGFW_MEMCPY(monitor.name, name, 6);

	CGRect bounds = CGDisplayBounds(display);
	monitor.x = (i32)bounds.origin.x;
	monitor.y = (i32)bounds.origin.y;
	monitor.mode.area = RGFW_AREA((int) bounds.size.width, (int) bounds.size.height);

	monitor.mode.red = 8; monitor.mode.green = 8; monitor.mode.blue = 8;

	CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display);
	monitor.mode.refreshRate = RGFW_osx_getRefreshRate(display, mode);
	CFRelease(mode);

	CGSize screenSizeMM = CGDisplayScreenSize(display);
	monitor.physW = (float)screenSizeMM.width / 25.4f;
	monitor.physH = (float)screenSizeMM.height / 25.4f;

	float ppi_width = (monitor.mode.area.w/monitor.physW);
	float ppi_height = (monitor.mode.area.h/monitor.physH);

	monitor.pixelRatio = (float)((CGFloat (*)(id, SEL))abi_objc_msgSend_fpret) (screen, sel_registerName("backingScaleFactor"));
	float dpi = 96.0f * monitor.pixelRatio;

	monitor.scaleX = ((i32)(((float) (ppi_width) / dpi) * 10.0f)) / 10.0f;
	monitor.scaleY = ((i32)(((float) (ppi_height) / dpi) * 10.0f)) / 10.0f;

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoMonitor, RGFW_DEBUG_CTX_MON(monitor), "monitor found");
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
    CGPoint point = { mon.x, mon.y };

    CGDirectDisplayID display;
    uint32_t displayCount = 0;
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
		foundMode.area = RGFW_AREA(CGDisplayModeGetWidth(cmode), CGDisplayModeGetHeight(cmode));
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
		(NSPasteboard_generalPasteboard(), func, NSString_stringWithUTF8String(text), NSString_stringWithUTF8String(NSPasteboardTypeString));
}

	#ifdef RGFW_OPENGL
	void RGFW_window_makeCurrent_OpenGL(RGFW_window* win) {
		if (win != NULL)
			objc_msgSend_void(win->src.ctx, sel_registerName("makeCurrentContext"));
		else
			objc_msgSend_id(objc_getClass("NSOpenGLContext"), sel_registerName("clearCurrentContext"));
	}
	void* RGFW_getCurrent_OpenGL(void) {
		return objc_msgSend_id(objc_getClass("NSOpenGLContext"), sel_registerName("currentContext"));
	}

	void RGFW_window_swapBuffers_OpenGL(RGFW_window* win) {
		objc_msgSend_void(win->src.ctx, sel_registerName("flushBuffer"));
	}
	#endif

	#if !defined(RGFW_EGL)

	void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) {
		RGFW_ASSERT(win != NULL);
		#if defined(RGFW_OPENGL)

		NSOpenGLContext_setValues((id)win->src.ctx, &swapInterval, 222);
		#else
		RGFW_UNUSED(swapInterval);
		#endif
	}

	#endif

void RGFW_window_swapBuffers_software(RGFW_window* win) {
#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
	RGFW_RGB_to_BGR(win, win->buffer);
	i32 channels = 4;
	id image = ((id (*)(Class, SEL))objc_msgSend)(objc_getClass("NSImage"), sel_getUid("alloc"));
	NSSize size = (NSSize){win->bufferSize.w, win->bufferSize.h};
	image = ((id (*)(id, SEL, NSSize))objc_msgSend)((id)image, sel_getUid("initWithSize:"), size);

	id rep  = NSBitmapImageRep_initWithBitmapData(&win->buffer, win->r.w, win->r.h , 8, channels, (channels == 4), false,
							"NSDeviceRGBColorSpace", 1 << 1, (u32)win->bufferSize.w  * (u32)channels, 8 * (u32)channels);
	((void (*)(id, SEL, id))objc_msgSend)((id)image, sel_getUid("addRepresentation:"), rep);

	id contentView = ((id (*)(id, SEL))objc_msgSend)((id)win->src.window, sel_getUid("contentView"));
	((void (*)(id, SEL, BOOL))objc_msgSend)(contentView, sel_getUid("setWantsLayer:"), YES);
	id layer = ((id (*)(id, SEL))objc_msgSend)(contentView, sel_getUid("layer"));

	((void (*)(id, SEL, id))objc_msgSend)(layer, sel_getUid("setContents:"), (id)image);
	((void (*)(id, SEL, BOOL))objc_msgSend)(contentView, sel_getUid("setNeedsDisplay:"), YES);

	NSRelease(rep);
	NSRelease(image);
#else
	RGFW_UNUSED(win);
#endif
}

void RGFW_deinit(void) {
    _RGFW.windowCount = -1;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal, RGFW_DEBUG_CTX(NULL, 0), "global context deinitialized");
}

void RGFW_window_close(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);
	NSRelease(win->src.view);
	if ((win->_flags & RGFW_windowNoInitAPI) == 0) RGFW_window_freeOpenGL(win);

	#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
		if ((win->_flags & RGFW_BUFFER_ALLOC))
			RGFW_FREE(win->buffer);
	#endif

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal, RGFW_DEBUG_CTX(NULL, 0), "global context deinitialized");
    _RGFW.windowCount--;
    if (_RGFW.windowCount == 0) RGFW_deinit();

    RGFW_clipboard_switch(NULL);
	RGFW_FREE(win->event.droppedFiles);
	if ((win->_flags & RGFW_WINDOW_ALLOC)) {
		RGFW_FREE(win);
        win = NULL;
    }
}

u64 RGFW_getTimerFreq(void) {
	static u64 freq = 0;
	if (freq == 0) {
		mach_timebase_info_data_t info;
		mach_timebase_info(&info);
		freq = (u64)((info.denom * 1e9) / info.numer);
	}

	return freq;
}

u64 RGFW_getTimerValue(void) { return (u64)mach_absolute_time(); }

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

	RGFW_eventQueuePushEx(e.type = RGFW_windowResized; e._win = _RGFW.root);
	RGFW_windowResizedCallback(_RGFW.root, RGFW_RECT(0, 0, E->windowInnerWidth, E->windowInnerHeight));
    return EM_TRUE;
}

EM_BOOL Emscripten_on_fullscreenchange(int eventType, const EmscriptenFullscreenChangeEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);
	static u8 fullscreen = RGFW_FALSE;
	static RGFW_rect ogRect;

	if (fullscreen == RGFW_FALSE) {
		ogRect = _RGFW.root->r;
	}

	fullscreen = !fullscreen;
	RGFW_eventQueuePushEx(e.type = RGFW_windowResized; e._win = _RGFW.root);
	_RGFW.root->r = RGFW_RECT(0, 0, E->screenWidth, E->screenHeight);

	EM_ASM("Module.canvas.focus();");

	if (fullscreen == RGFW_FALSE) {
		_RGFW.root->r = RGFW_RECT(0, 0, ogRect.w, ogRect.h);
		/* emscripten_request_fullscreen("#canvas", 0); */
	} else {
		#if __EMSCRIPTEN_major__  >= 1 && __EMSCRIPTEN_minor__  >= 29 && __EMSCRIPTEN_tiny__  >= 0
			EmscriptenFullscreenStrategy FSStrat = {0};
			FSStrat.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH; /* EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT : EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH; */
			FSStrat.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
			FSStrat.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
			emscripten_request_fullscreen_strategy("#canvas", 1, &FSStrat);
		#else
			emscripten_request_fullscreen("#canvas", 1);
		#endif
	}

	emscripten_set_canvas_element_size("#canvas", _RGFW.root->r.w, _RGFW.root->r.h);

	RGFW_windowResizedCallback(_RGFW.root, _RGFW.root->r);
	return EM_TRUE;
}



EM_BOOL Emscripten_on_focusin(int eventType, const EmscriptenFocusEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData); RGFW_UNUSED(E);

	RGFW_eventQueuePushEx(e.type = RGFW_focusIn; e._win = _RGFW.root);
	_RGFW.root->_flags |= RGFW_windowFocus;
	RGFW_focusCallback(_RGFW.root, 1);

	if ((_RGFW.root->_flags & RGFW_HOLD_MOUSE)) RGFW_window_mouseHold(_RGFW.root, RGFW_AREA(_RGFW.root->r.w, _RGFW.root->r.h));
    return EM_TRUE;
}

EM_BOOL Emscripten_on_focusout(int eventType, const EmscriptenFocusEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData); RGFW_UNUSED(E);

	RGFW_eventQueuePushEx(e.type = RGFW_focusOut; e._win = _RGFW.root);
    RGFW_window_focusLost(_RGFW.root);
    RGFW_focusCallback(_RGFW.root, 0);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_mousemove(int eventType, const EmscriptenMouseEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);
	RGFW_eventQueuePushEx(e.type = RGFW_mousePosChanged;
									e.point = RGFW_POINT(E->targetX, E->targetY);
									e.vector = RGFW_POINT(E->movementX, E->movementY);
									e._win = _RGFW.root);

	_RGFW.root->_lastMousePoint = RGFW_POINT(E->targetX, E->targetY);
	RGFW_mousePosCallback(_RGFW.root, RGFW_POINT(E->targetX, E->targetY), RGFW_POINT(E->movementX, E->movementY));
    return EM_TRUE;
}

EM_BOOL Emscripten_on_mousedown(int eventType, const EmscriptenMouseEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	int button = E->button;
	if (button > 2)
		button += 2;

	RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonPressed;
									e.point = RGFW_POINT(E->targetX, E->targetY);
									e.vector = RGFW_POINT(E->movementX, E->movementY);
									e.button = (u8)button;
									e.scroll = 0;
									e._win = _RGFW.root);
	RGFW_mouseButtons[button].prev = RGFW_mouseButtons[button].current;
	RGFW_mouseButtons[button].current = 1;

	RGFW_mouseButtonCallback(_RGFW.root, button, 0, 1);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_mouseup(int eventType, const EmscriptenMouseEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	int button = E->button;
	if (button > 2)
		button += 2;

	RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonReleased;
									e.point = RGFW_POINT(E->targetX, E->targetY);
									e.vector = RGFW_POINT(E->movementX, E->movementY);
									e.button = (u8)button;
									e.scroll = 0;
									e._win = _RGFW.root);
	RGFW_mouseButtons[button].prev = RGFW_mouseButtons[button].current;
	RGFW_mouseButtons[button].current = 0;

	RGFW_mouseButtonCallback(_RGFW.root, button, 0, 0);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_wheel(int eventType, const EmscriptenWheelEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	int button =  RGFW_mouseScrollUp + (E->deltaY < 0);
	RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonPressed;
									e.button = (u8)button;
									e.scroll = (double)(E->deltaY < 0 ? 1 : -1);
									e._win = _RGFW.root);
	RGFW_mouseButtons[button].prev = RGFW_mouseButtons[button].current;
	RGFW_mouseButtons[button].current = 1;
	RGFW_mouseButtonCallback(_RGFW.root, button, E->deltaY < 0 ? 1 : -1, 1);

    return EM_TRUE;
}

EM_BOOL Emscripten_on_touchstart(int eventType, const EmscriptenTouchEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

    size_t i;
    for (i = 0; i < (size_t)E->numTouches; i++) {
		RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonPressed;
										e.point = RGFW_POINT(E->touches[i].targetX, E->touches[i].targetY);
										e.button = RGFW_mouseLeft;
										e._win = _RGFW.root);

	    RGFW_mouseButtons[RGFW_mouseLeft].prev = RGFW_mouseButtons[RGFW_mouseLeft].current;
	    RGFW_mouseButtons[RGFW_mouseLeft].current = 1;

		_RGFW.root->_lastMousePoint = RGFW_POINT(E->touches[i].targetX, E->touches[i].targetY);
        RGFW_mousePosCallback(_RGFW.root, RGFW_POINT(E->touches[i].targetX, E->touches[i].targetY), _RGFW.root->event.vector);
	    RGFW_mouseButtonCallback(_RGFW.root, RGFW_mouseLeft, 0, 1);
    }

	return EM_TRUE;
}
EM_BOOL Emscripten_on_touchmove(int eventType, const EmscriptenTouchEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

    size_t i;
    for (i = 0; i < (size_t)E->numTouches; i++) {
		RGFW_eventQueuePushEx(e.type = RGFW_mousePosChanged;
			e.point = RGFW_POINT(E->touches[i].targetX, E->touches[i].targetY);
			e.button = RGFW_mouseLeft;
			e._win = _RGFW.root);

		_RGFW.root->_lastMousePoint = RGFW_POINT(E->touches[i].targetX, E->touches[i].targetY);
        RGFW_mousePosCallback(_RGFW.root, RGFW_POINT(E->touches[i].targetX, E->touches[i].targetY), _RGFW.root->event.vector);
    }
    return EM_TRUE;
}

EM_BOOL Emscripten_on_touchend(int eventType, const EmscriptenTouchEvent* E, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

    size_t i;
    for (i = 0; i < (size_t)E->numTouches; i++) {
		RGFW_eventQueuePushEx(e.type = RGFW_mouseButtonReleased;
										e.point = RGFW_POINT(E->touches[i].targetX, E->touches[i].targetY);
										e.button = RGFW_mouseLeft;
										e._win = _RGFW.root);

		RGFW_mouseButtons[RGFW_mouseLeft].prev = RGFW_mouseButtons[RGFW_mouseLeft].current;
		RGFW_mouseButtons[RGFW_mouseLeft].current = 0;

		_RGFW.root->_lastMousePoint = RGFW_POINT(E->touches[i].targetX, E->touches[i].targetY);
		RGFW_mousePosCallback(_RGFW.root, RGFW_POINT(E->touches[i].targetX, E->touches[i].targetY), _RGFW.root->event.vector);
		RGFW_mouseButtonCallback(_RGFW.root, RGFW_mouseLeft, 0, 0);
    }
	return EM_TRUE;
}

EM_BOOL Emscripten_on_touchcancel(int eventType, const EmscriptenTouchEvent* E, void* userData) { RGFW_UNUSED(eventType); RGFW_UNUSED(userData); return EM_TRUE; }

EM_BOOL Emscripten_on_gamepad(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	if (gamepadEvent->index >= 4)
		return 0;

	size_t i = gamepadEvent->index;
	if (gamepadEvent->connected) {
		RGFW_STRNCPY(RGFW_gamepads_name[gamepadEvent->index], gamepadEvent->id, sizeof(RGFW_gamepads_name[gamepadEvent->index]) - 1);
		RGFW_gamepads_name[gamepadEvent->index][sizeof(RGFW_gamepads_name[gamepadEvent->index]) - 1] = '\0';
		RGFW_gamepads_type[i] = RGFW_gamepadUnknown;
		if (RGFW_STRSTR(RGFW_gamepads_name[i], "Microsoft") || RGFW_STRSTR(RGFW_gamepads_name[i], "X-Box"))
			RGFW_gamepads_type[i] = RGFW_gamepadMicrosoft;
		else if (RGFW_STRSTR(RGFW_gamepads_name[i], "PlayStation") || RGFW_STRSTR(RGFW_gamepads_name[i], "PS3") || RGFW_STRSTR(RGFW_gamepads_name[i], "PS4") || RGFW_STRSTR(RGFW_gamepads_name[i], "PS5"))
			RGFW_gamepads_type[i] = RGFW_gamepadSony;
		else if (RGFW_STRSTR(RGFW_gamepads_name[i], "Nintendo"))
			RGFW_gamepads_type[i] = RGFW_gamepadNintendo;
		else if (RGFW_STRSTR(RGFW_gamepads_name[i], "Logitech"))
			RGFW_gamepads_type[i] = RGFW_gamepadLogitech;
		RGFW_gamepadCount++;
	} else {
		RGFW_gamepadCount--;
	}

	RGFW_eventQueuePushEx(e.type = (RGFW_eventType)(gamepadEvent->connected ? RGFW_gamepadConnected : RGFW_gamepadConnected);
									e.gamepad = (u16)gamepadEvent->index;
									e._win = _RGFW.root);

	RGFW_gamepadCallback(_RGFW.root, gamepadEvent->index, gamepadEvent->connected);
	RGFW_gamepads[gamepadEvent->index] = gamepadEvent->connected;

    return 1; /* The event was consumed by the callback handler */
}

u32 RGFW_wASMPhysicalToRGFW(u32 hash) {
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
		case 0x5D64DA91U /* NumpadMultiply     */: return RGFW_multiply;
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
		case 0xC925FCDFU /* Numpad7            */: return RGFW_multiply;             /* 0x0047 */
		case 0xC925FCD0U /* Numpad8            */: return RGFW_KP_8;             /* 0x0048 */
		case 0xC925FCD1U /* Numpad9            */: return RGFW_KP_9;             /* 0x0049 */
		case 0x5EA3E8A4U /* NumpadSubtract     */: return RGFW_minus;      /* 0x004A */
		case 0xC925FCDCU /* Numpad4            */: return RGFW_KP_4;             /* 0x004B */
		case 0xC925FCDDU /* Numpad5            */: return RGFW_KP_5;             /* 0x004C */
		case 0xC925FCDEU /* Numpad6            */: return RGFW_KP_6;             /* 0x004D */
		case 0xC925FCD9U /* Numpad1            */: return RGFW_KP_1;             /* 0x004F */
		case 0xC925FCDAU /* Numpad2            */: return RGFW_KP_2;             /* 0x0050 */
		case 0xC925FCDBU /* Numpad3            */: return RGFW_KP_3;             /* 0x0051 */
		case 0xC925FCD8U /* Numpad0            */: return RGFW_KP_0;             /* 0x0052 */
		case 0x95852DACU /* NumpadDecimal      */: return RGFW_period;       /* 0x0053 */
		case 0x7B8E57B1U  /* F11                */: return RGFW_F11;                  /* 0x0057 */
		case 0x7B8E57B2U  /* F12                */: return RGFW_F12;                  /* 0x0058 */
		case 0x7393FBACU /* NumpadEqual        */: return RGFW_KP_Return;
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
	}

	return 0;
}

void EMSCRIPTEN_KEEPALIVE RGFW_handleKeyEvent(char* key, char* code, RGFW_bool press) {
	const char* iCode = code;

	u32 hash = 0;
	while(*iCode) hash = ((hash ^ 0x7E057D79U) << 3) ^ (unsigned int)*iCode++;

	u32 physicalKey = RGFW_wASMPhysicalToRGFW(hash);

	u8 mappedKey = (u8)(*((u32*)key));

	if (*((u16*)key) != mappedKey) {
		mappedKey = 0;
		if (*((u32*)key) == *((u32*)"Tab")) mappedKey = RGFW_tab;
	}

	RGFW_eventQueuePushEx(e.type = (RGFW_eventType)(press ? RGFW_keyPressed : RGFW_keyReleased);
										e.key = (u8)physicalKey;
										e.keyChar = (u8)mappedKey;
										e.keyMod = _RGFW.root->event.keyMod;
										e._win = _RGFW.root);

	RGFW_keyboard[physicalKey].prev = RGFW_keyboard[physicalKey].current;
	RGFW_keyboard[physicalKey].current = press;

	RGFW_keyCallback(_RGFW.root, physicalKey, mappedKey, _RGFW.root->event.keyMod, press);
}

void EMSCRIPTEN_KEEPALIVE RGFW_handleKeyMods(RGFW_bool capital, RGFW_bool numlock, RGFW_bool control, RGFW_bool alt, RGFW_bool shift, RGFW_bool super, RGFW_bool scroll) {
	RGFW_updateKeyModsPro(_RGFW.root, capital, numlock, control, alt, shift, super, scroll);
}

void EMSCRIPTEN_KEEPALIVE Emscripten_onDrop(size_t count) {
	if (!(_RGFW.root->_flags & RGFW_windowAllowDND))
		return;

	_RGFW.root->event.droppedFilesCount = count; 
	RGFW_eventQueuePushEx(e.type = RGFW_DND;
									e.droppedFilesCount = count;
									e._win = _RGFW.root);
	RGFW_dndCallback(_RGFW.root, _RGFW.root->event.droppedFiles, count);
}

RGFW_bool RGFW_stopCheckEvents_bool = RGFW_FALSE;
void RGFW_stopCheckEvents(void) {
	RGFW_stopCheckEvents_bool = RGFW_TRUE;
}

void RGFW_window_eventWait(RGFW_window* win, i32 waitMS) {
	RGFW_UNUSED(win);
	if (waitMS == 0) return;

	u32 start = (u32)(((u64)RGFW_getTimeNS()) / 1e+6);

	while ((_RGFW.eventLen == 0) && RGFW_stopCheckEvents_bool == RGFW_FALSE && (RGFW_getTimeNS() / 1e+6) - start < waitMS)
		emscripten_sleep(0);

	RGFW_stopCheckEvents_bool = RGFW_FALSE;
}

void RGFW_window_initBufferPtr(RGFW_window* win, u8* buffer, RGFW_area area){
	#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
		win->buffer = buffer;
		win->bufferSize = area;
	#ifdef RGFW_OSMESA
			win->src.ctx = OSMesaCreateContext(OSMESA_RGBA, NULL);
			OSMesaMakeCurrent(win->src.ctx, win->buffer, GL_UNSIGNED_BYTE, area.w, area.h);
			OSMesaPixelStore(OSMESA_Y_UP, 0);
	#endif
	#else
	RGFW_UNUSED(win);  RGFW_UNUSED(buffer); RGFW_UNUSED(area); /*!< if buffer rendering is not being used */
	#endif
}

void EMSCRIPTEN_KEEPALIVE RGFW_makeSetValue(size_t index, char* file) {
	/* This seems like a terrible idea, don't replicate this unless you hate yourself or the OS */
	/* TODO: find a better way to do this
	*/
	RGFW_STRNCPY((char*)_RGFW.root->event.droppedFiles[index], file, RGFW_MAX_PATH - 1);
	_RGFW.root->event.droppedFiles[index][RGFW_MAX_PATH - 1] = '\0';
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

void RGFW_window_initOpenGL(RGFW_window* win) {
#if defined(RGFW_OPENGL) && !defined(RGFW_WEBGPU) && !defined(RGFW_OSMESA) && !defined(RGFW_BUFFER)
	EmscriptenWebGLContextAttributes attrs;
	attrs.alpha = RGFW_GL_HINTS[RGFW_glDepth];
	attrs.depth = RGFW_GL_HINTS[RGFW_glAlpha];
	attrs.stencil = RGFW_GL_HINTS[RGFW_glStencil];
	attrs.antialias = RGFW_GL_HINTS[RGFW_glSamples];
	attrs.premultipliedAlpha = EM_TRUE;
	attrs.preserveDrawingBuffer = EM_FALSE;

	if (RGFW_GL_HINTS[RGFW_glDoubleBuffer] == 0)
		attrs.renderViaOffscreenBackBuffer = 0;
	else
		attrs.renderViaOffscreenBackBuffer = RGFW_GL_HINTS[RGFW_glAuxBuffers];

	attrs.failIfMajorPerformanceCaveat = EM_FALSE;
	attrs.majorVersion = (RGFW_GL_HINTS[RGFW_glMajor] == 0) ? 1 : RGFW_GL_HINTS[RGFW_glMajor];
	attrs.minorVersion = RGFW_GL_HINTS[RGFW_glMinor];

	attrs.enableExtensionsByDefault = EM_TRUE;
	attrs.explicitSwapControl = EM_TRUE;

	emscripten_webgl_init_context_attributes(&attrs);
	win->src.ctx = emscripten_webgl_create_context("#canvas", &attrs);
	emscripten_webgl_make_context_current(win->src.ctx);

	#ifdef LEGACY_GL_EMULATION
	EM_ASM("Module.useWebGL = true; GLImmediate.init();");
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, RGFW_DEBUG_CTX(win, 0), "opengl context initalized");
    #endif
    glViewport(0, 0, win->r.w, win->r.h);
#endif
}

void RGFW_window_freeOpenGL(RGFW_window* win) {
#if defined(RGFW_OPENGL) && !defined(RGFW_WEBGPU) && !defined(RGFW_OSMESA) && !defined(RGFW_OSMESA)
	if (win->src.ctx == 0) return;
	emscripten_webgl_destroy_context(win->src.ctx);
	win->src.ctx = 0;
	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoOpenGL, RGFW_DEBUG_CTX(win, 0), "opengl context freed");
#elif defined(RGFW_OPENGL) && defined(RGFW_OSMESA)
	if(win->src.ctx == 0) return;
	OSMesaDestroyContext(win->src.ctx);
	win->src.ctx = 0;
#else
	RGFW_UNUSED(win);
#endif
}

i32 RGFW_init(void) {  
#if defined(RGFW_C89) || defined(__cplusplus)
    if (_RGFW_init) return 0; 
    _RGFW_init = RGFW_TRUE;
    _RGFW.root = NULL; _RGFW.current = NULL; _RGFW.windowCount = -2; _RGFW.eventLen = 0; _RGFW.eventIndex = 0;
#endif

    _RGFW.windowCount = 0;  
    RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal, RGFW_DEBUG_CTX(NULL, 0), "global context initialized"); 
    return 0; 
}

RGFW_window* RGFW_createWindowPtr(const char* name, RGFW_rect rect, RGFW_windowFlags flags, RGFW_window* win) {
    RGFW_window_basic_init(win, rect, flags);
	RGFW_window_initOpenGL(win);

	#if defined(RGFW_WEBGPU)
		win->src.ctx = wgpuCreateInstance(NULL);
		win->src.device = emscripten_webgpu_get_device();
		win->src.queue = wgpuDeviceGetQueue(win->src.device);
	#endif

	emscripten_set_canvas_element_size("#canvas", rect.w, rect.h);
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
	emscripten_set_gamepadconnected_callback(NULL, 1, Emscripten_on_gamepad);
	emscripten_set_gamepaddisconnected_callback(NULL, 1, Emscripten_on_gamepad);

	if (flags & RGFW_windowAllowDND)  {
		win->_flags |= RGFW_windowAllowDND;
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

						_RGFW_writeFile(path, new Uint8Array(data), file.size);
					}
				};

				reader.readAsArrayBuffer(file);
				/* This works weird on modern opengl */
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

	RGFW_window_setFlags(win, flags);

	if ((flags & RGFW_windowNoInitAPI) == 0) {
        RGFW_window_initBuffer(win);
    }

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, RGFW_DEBUG_CTX(win, 0), "a new  window was created");
    return win;
}

u8 RGFW_rgfwToKeyChar(u32 rgfw_keycode) {
    return (u8)rgfw_keycode; /* TODO */
}

RGFW_event* RGFW_window_checkEvent(RGFW_window* win) {
    if (win == NULL || ((win->_flags & RGFW_windowFreeOnClose) && (win->_flags & RGFW_EVENT_QUIT))) return NULL;
    RGFW_event* ev =  RGFW_window_checkEventCore(win);
	if (ev) return ev;

	emscripten_sample_gamepad_data();
	/* check gamepads */
    int i;
    for (i = 0; (i < emscripten_get_num_gamepads()) && (i < 4); i++) {
		if (RGFW_gamepads[i] == 0)
			continue;
        EmscriptenGamepadEvent gamepadState;

        if (emscripten_get_gamepad_status(i, &gamepadState) != EMSCRIPTEN_RESULT_SUCCESS)
			break;

		/* Register buttons data for every connected gamepad */
        int j;
        for (j = 0; (j < gamepadState.numButtons) && (j < 16); j++) {
			u32 map[] = {
				RGFW_gamepadA, RGFW_gamepadB, RGFW_gamepadX, RGFW_gamepadY,
				RGFW_gamepadL1, RGFW_gamepadR1, RGFW_gamepadL2, RGFW_gamepadR2,
				RGFW_gamepadSelect, RGFW_gamepadStart,
				RGFW_gamepadL3, RGFW_gamepadR3,
				RGFW_gamepadUp, RGFW_gamepadDown, RGFW_gamepadLeft, RGFW_gamepadRight, RGFW_gamepadHome
			};


			u32 button = map[j];
			if (button == 404)
				continue;

			if (RGFW_gamepadPressed[i][button].current != gamepadState.digitalButton[j]) {
				if (gamepadState.digitalButton[j])
					win->event.type = RGFW_gamepadButtonPressed;
				else
					win->event.type = RGFW_gamepadButtonReleased;

				win->event.gamepad = i;
				win->event.button = map[j];

				RGFW_gamepadPressed[i][button].prev = RGFW_gamepadPressed[i][button].current;
				RGFW_gamepadPressed[i][button].current = gamepadState.digitalButton[j];

				RGFW_gamepadButtonCallback(win, win->event.gamepad, win->event.button, gamepadState.digitalButton[j]);
				return &win->event;
			}
		}

		for (j = 0; (j < gamepadState.numAxes) && (j < 4); j += 2) {
			win->event.axisesCount = gamepadState.numAxes / 2;
			if (RGFW_gamepadAxes[i][(size_t)(j / 2)].x != (i8)(gamepadState.axis[j] * 100.0f) ||
				RGFW_gamepadAxes[i][(size_t)(j / 2)].y != (i8)(gamepadState.axis[j + 1] * 100.0f)
			) {

				RGFW_gamepadAxes[i][(size_t)(j / 2)].x = (i8)(gamepadState.axis[j] * 100.0f);
				RGFW_gamepadAxes[i][(size_t)(j / 2)].y = (i8)(gamepadState.axis[j + 1] * 100.0f);
				win->event.axis[(size_t)(j / 2)] = RGFW_gamepadAxes[i][(size_t)(j / 2)];

				win->event.type = RGFW_gamepadAxisMove;
				win->event.gamepad = i;
				win->event.whichAxis = j / 2;

				RGFW_gamepadAxisCallback(win, win->event.gamepad, win->event.axis, win->event.axisesCount, win->event.whichAxis);
				return &win->event;
			}
		}
    }

	return NULL;
}

void RGFW_window_resize(RGFW_window* win, RGFW_area a) {
	RGFW_UNUSED(win);
	emscripten_set_canvas_element_size("#canvas", a.w, a.h);
}

/* NOTE: I don't know if this is possible */
void RGFW_window_moveMouse(RGFW_window* win, RGFW_point v) { RGFW_UNUSED(win); RGFW_UNUSED(v); }
/* this one might be possible but it looks iffy */
RGFW_mouse* RGFW_loadMouse(u8* icon, RGFW_area a, i32 channels) { RGFW_UNUSED(channels); RGFW_UNUSED(a); RGFW_UNUSED(icon); return NULL; }

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

RGFW_point RGFW_getGlobalMousePoint(void) {
    RGFW_point point;
    point.x = EM_ASM_INT({
        return window.mouseX || 0;
    });
    point.y = EM_ASM_INT({
        return window.mouseY || 0;
    });
    return point;
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

void RGFW_window_swapBuffers_software(RGFW_window* win) {
#if defined(RGFW_OSMESA)
	EM_ASM_({
		var data = Module.HEAPU8.slice($0, $0 + $1 * $2 * 4);
		let context = document.getElementById("canvas").getContext("2d");
		let image = context.getImageData(0, 0, $1, $2);
		image.data.set(data);
		context.putImageData(image, 0, $4 - $2);
	}, win->buffer, win->bufferSize.w, win->bufferSize.h, win->r.w, win->r.h);
#elif defined(RGFW_BUFFER)
	EM_ASM_({
		var data = Module.HEAPU8.slice($0, $0 + $1 * $2 * 4);
		let context = document.getElementById("canvas").getContext("2d");
		let image = context.getImageData(0, 0, $1, $2);
		image.data.set(data);
		context.putImageData(image, 0, 0);
	}, win->buffer, win->bufferSize.w, win->bufferSize.h, win->r.w, win->r.h);
	emscripten_sleep(0);
#else
	RGFW_UNUSED(win);
#endif
}

void RGFW_window_makeCurrent_OpenGL(RGFW_window* win) {
#if !defined(RGFW_WEBGPU) && !(defined(RGFW_OSMESA) || defined(RGFW_BUFFER))
	if (win == NULL)
	    emscripten_webgl_make_context_current(0);
	else
	    emscripten_webgl_make_context_current(win->src.ctx);
#endif
}


void RGFW_window_swapBuffers_OpenGL(RGFW_window* win) {
#ifndef RGFW_WEBGPU
	emscripten_webgl_commit_frame();

#endif
    emscripten_sleep(0);
}

#ifndef RGFW_WEBGPU
void* RGFW_getCurrent_OpenGL(void) { return (void*)emscripten_webgl_get_current_context(); }
#endif

#ifndef RGFW_EGL
void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) { RGFW_UNUSED(win); RGFW_UNUSED(swapInterval); }
#endif

void RGFW_deinit(void) { _RGFW.windowCount = -1; RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoGlobal, RGFW_DEBUG_CTX(NULL, 0), "global context deinitialized"); }

void RGFW_window_close(RGFW_window* win) {
	if ((win->_flags & RGFW_windowNoInitAPI) == 0) RGFW_window_freeOpenGL(win);

	#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
	if ((win->_flags & RGFW_BUFFER_ALLOC))
		RGFW_FREE(win->buffer);
	#endif

	RGFW_sendDebugInfo(RGFW_typeInfo, RGFW_infoWindow, RGFW_DEBUG_CTX(win, 0), "a window was freed");
    _RGFW.windowCount--;
    if (_RGFW.windowCount == 0) RGFW_deinit();

    RGFW_clipboard_switch(NULL);
	RGFW_FREE(win->event.droppedFiles);
	if ((win->_flags & RGFW_WINDOW_ALLOC)) {
	    RGFW_FREE(win);
        win = NULL;
    }
}

int RGFW_innerWidth(void) {   return EM_ASM_INT({ return window.innerWidth; });  }
int RGFW_innerHeight(void) {  return EM_ASM_INT({ return window.innerHeight; });  }

RGFW_area RGFW_getScreenSize(void) {
	return RGFW_AREA(RGFW_innerWidth(), RGFW_innerHeight());
}

RGFW_bool RGFW_extensionSupportedPlatform(const char* extension, size_t len) {
#ifdef RGFW_OPENGL
    return EM_ASM_INT({
        var ext = UTF8ToString($0, $1);
        var canvas = document.querySelector('canvas');
        var gl = canvas.getContext('webgl') || canvas.getContext('experimental-webgl');
        if (!gl) return 0;

        var supported = gl.getSupportedExtensions();
        return supported && supported.includes(ext) ? 1 : 0;
    }, extension, len);
#else
    return RGFW_FALSE;
#endif
}

RGFW_proc RGFW_getProcAddress(const char* procname) {
#ifdef RGFW_OPENGL
    return (RGFW_proc)emscripten_webgl_get_proc_address(procname);
#else
    return NULL
#endif
}

void RGFW_sleep(u64 milisecond) {
	emscripten_sleep(milisecond);
}

u64 RGFW_getTimerFreq(void) { return (u64)1000; }
u64 RGFW_getTimerValue(void) { return emscripten_get_now() * 1e+6; }

void RGFW_releaseCursor(RGFW_window* win) {
	RGFW_UNUSED(win);
	emscripten_exit_pointerlock();
}

void RGFW_captureCursor(RGFW_window* win, RGFW_rect r) {
	RGFW_UNUSED(win); RGFW_UNUSED(r);

	emscripten_request_pointerlock("#canvas", 1);
}


void RGFW_window_setName(RGFW_window* win, const char* name) {
	RGFW_UNUSED(win);
	emscripten_set_window_title(name);
}

void RGFW_window_maximize(RGFW_window* win) {
	RGFW_ASSERT(win != NULL);

	RGFW_area screen = RGFW_getScreenSize();
	RGFW_window_move(win, RGFW_POINT(0, 0));
	RGFW_window_resize(win, screen);
}

void RGFW_window_setFullscreen(RGFW_window* win, RGFW_bool fullscreen) {
	RGFW_ASSERT(win != NULL);
	if (fullscreen) {
		win->_flags |= RGFW_windowFullscreen;
		EM_ASM( Module.requestFullscreen(false, true); );
		return;
	}
	win->_flags &= ~(u32)RGFW_windowFullscreen;
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

/* unsupported functions */
void RGFW_window_focus(RGFW_window* win) { RGFW_UNUSED(win); }
void RGFW_window_raise(RGFW_window* win) { RGFW_UNUSED(win); }
RGFW_bool RGFW_monitor_requestMode(RGFW_monitor mon, RGFW_monitorMode mode, RGFW_modeRequest request) { RGFW_UNUSED(mon); RGFW_UNUSED(mode); RGFW_UNUSED(request); return RGFW_FALSE; }
RGFW_monitor* RGFW_getMonitors(size_t* len) { RGFW_UNUSED(len); return NULL; }
RGFW_monitor RGFW_getPrimaryMonitor(void) { return (RGFW_monitor){}; }
void RGFW_window_move(RGFW_window* win, RGFW_point v) { RGFW_UNUSED(win); RGFW_UNUSED(v); }
void RGFW_window_setAspectRatio(RGFW_window* win, RGFW_area a) { RGFW_UNUSED(win); RGFW_UNUSED(a); }
void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a) { RGFW_UNUSED(win); RGFW_UNUSED(a);  }
void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a) { RGFW_UNUSED(win); RGFW_UNUSED(a);  }
void RGFW_window_minimize(RGFW_window* win) { RGFW_UNUSED(win); }
void RGFW_window_restore(RGFW_window* win) { RGFW_UNUSED(win); }
void RGFW_window_setFloating(RGFW_window* win, RGFW_bool floating) { RGFW_UNUSED(win); RGFW_UNUSED(floating); }
void RGFW_window_setBorder(RGFW_window* win, RGFW_bool border) { RGFW_UNUSED(win); RGFW_UNUSED(border);  }
RGFW_bool RGFW_window_setIconEx(RGFW_window* win, u8* icon, RGFW_area a, i32 channels, u8 type) { RGFW_UNUSED(win); RGFW_UNUSED(icon); RGFW_UNUSED(a); RGFW_UNUSED(channels); RGFW_UNUSED(type); return RGFW_FALSE;  }
void RGFW_window_hide(RGFW_window* win) { RGFW_UNUSED(win); }
void RGFW_window_show(RGFW_window* win) {RGFW_UNUSED(win); }
RGFW_bool RGFW_window_isHidden(RGFW_window* win) { RGFW_UNUSED(win); return RGFW_FALSE; }
RGFW_bool RGFW_window_isMinimized(RGFW_window* win) { RGFW_UNUSED(win); return RGFW_FALSE; }
RGFW_bool RGFW_window_isMaximized(RGFW_window* win) { RGFW_UNUSED(win); return RGFW_FALSE; }
RGFW_bool RGFW_window_isFloating(RGFW_window* win) { RGFW_UNUSED(win); return RGFW_FALSE; }
RGFW_monitor RGFW_window_getMonitor(RGFW_window* win) { RGFW_UNUSED(win); return (RGFW_monitor){}; }
#endif

/* end of web asm defines */

/* unix (macOS, linux, web asm) only stuff */
#if defined(RGFW_X11) || defined(RGFW_MACOS) || defined(RGFW_WASM)  || defined(RGFW_WAYLAND)
#ifndef RGFW_NO_THREADS
#include <pthread.h>

RGFW_thread RGFW_createThread(RGFW_threadFunc_ptr ptr, void* args) {
	RGFW_thread t;
	pthread_create((pthread_t*) &t, NULL, *ptr, args);
	return t;
}
void RGFW_cancelThread(RGFW_thread thread) { pthread_cancel((pthread_t) thread); }
void RGFW_joinThread(RGFW_thread thread) { pthread_join((pthread_t) thread, NULL); }

#if defined(__linux__)
void RGFW_setThreadPriority(RGFW_thread thread, u8 priority) { pthread_setschedprio((pthread_t)thread, priority); }
#else
void RGFW_setThreadPriority(RGFW_thread thread, u8 priority) { RGFW_UNUSED(thread); RGFW_UNUSED(priority); }
#endif
#endif

#ifndef RGFW_WASM
void RGFW_sleep(u64 ms) {
	struct timespec time;
	time.tv_sec = 0;
	time.tv_nsec = (long int)((double)ms * 1e+6);

	#ifndef RGFW_NO_UNIX_CLOCK
	nanosleep(&time, NULL);
	#endif
}
#endif

#endif /* end of unix / mac stuff */
#endif /* RGFW_IMPLEMENTATION */

#if defined(__cplusplus) && !defined(__EMSCRIPTEN__)
}
#endif

#if _MSC_VER
	#pragma warning( pop )
#endif
