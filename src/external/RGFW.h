/*
* Copyright (C) 2023-24 ColleagueRiley
*
* libpng license
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
*
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
	#define RGFW_PRINT_ERRORS - (optional) makes it so RGFW prints errors when they're found
	#define RGFW_OSMESA - (optional) use OSmesa as backend (instead of system's opengl api + regular opengl)
	#define RGFW_BUFFER - (optional) just draw directly to (RGFW) window pixel buffer that is drawn to screen (the buffer is in the RGBA format)
	#define RGFW_EGL - (optional) use EGL for loading an OpenGL context (instead of the system's opengl api)
	#define RGFW_OPENGL_ES1 - (optional) use EGL to load and use Opengl ES (version 1) for backend rendering (instead of the system's opengl api)
									This version doesn't work for desktops (I'm pretty sure)
	#define RGFW_OPENGL_ES2 - (optional) use OpenGL ES (version 2)
	#define RGFW_OPENGL_ES3 - (optional) use OpenGL ES (version 3)
	#define RGFW_DIRECTX - (optional) use directX for the rendering backend (rather than opengl) (windows only, defaults to opengl for unix)
	#define RGFW_NO_API - (optional) don't use any rendering API (no opengl, no vulkan, no directX)

	#define RGFW_LINK_EGL (optional) (windows only) if EGL is being used, if EGL functions should be defined dymanically (using GetProcAddress)
	#define RGFW_LINK_OSMESA (optional) (windows only) if EGL is being used, if OS Mesa functions should be defined dymanically  (using GetProcAddress)

	#define RGFW_X11 (optional) (unix only) if X11 should be used. This option is turned on by default by unix systems except for MacOS
	#define RGFW_WGL_LOAD (optional) (windows only) if WGL should be loaded dynamically during runtime
	#define RGFW_NO_X11_CURSOR (optional) (unix only) don't use XCursor
	#define RGFW_NO_X11_CURSOR_PRELOAD (optional) (unix only) Use XCursor, but don't link it in code, (you'll have to link it with -lXcursor)

	#define RGFW_NO_DPI - Do not include calculate DPI (no XRM nor libShcore included)

	#define RGFW_ALLOC_DROPFILES (optional) if room should be allocating for drop files (by default it's global data)
	#define RGFW_MALLOC x - choose what function to use to allocate, by default the standard malloc is used
	#define RGFW_CALLOC x - choose what function to use to allocate (calloc), by default the standard calloc is used
	#define RGFW_FREE x - choose what function to use to allocated memory, by default the standard free is used

 	#define RGFW_EXPORT - Use when building RGFW 
    #define RGFW_IMPORT - Use when linking with RGFW (not as a single-header)
	
	#define RGFW_STD_INT - force the use stdint.h (for systems that might not have stdint.h (msvc)) 
*/

/*
	Credits :
		EimaMei/Sacode : Much of the code for creating windows using winapi, Wrote the Silicon library, helped with MacOS Support, siliapp.h -> referencing 

		stb - This project is heavily inspired by the stb single header files

		GLFW:
			certain parts of winapi and X11 are very poorly documented,
			GLFW's source code was referenced and used throughout the project (used code is marked in some way),
			this mainly includes, code for drag and drops, code for setting the icon to a bitmap and the code for managing the clipboard for X11 (as these parts are not documented very well)

			GLFW Copyright, https::/github.com/GLFW/GLFW

			Copyright (c) 2002-2006 Marcus Geelnard
			Copyright (c) 2006-2019 Camilla Löwy

		contributors : (feel free to put yourself here if you contribute)
		krisvers -> code review
		EimaMei (SaCode) -> code review
		Code-Nycticebus -> bug fixes
		Rob Rohan -> X11 bugs and missing features, MacOS/Cocoa fixing memory issues/bugs 
		AICDG (@THISISAGOODNAME) -> vulkan support (example)
		@Easymode -> support, testing/debugging, bug fixes and reviews
*/

#if _MSC_VER
	#pragma comment(lib, "gdi32")
	#pragma comment(lib, "shell32")
	#pragma comment(lib, "opengl32")
	#pragma comment(lib, "winmm")
	#pragma comment(lib, "user32")
#endif

#ifndef RGFW_MALLOC
	#include <stdlib.h>

	#ifndef __USE_POSIX199309
	#define __USE_POSIX199309
	#endif

	#include <time.h>
	#define RGFW_MALLOC malloc
	#define RGFW_CALLOC calloc
	#define RGFW_FREE free
#endif

#if !_MSC_VER
	#ifndef inline
		#ifndef __APPLE__
			#define inline __inline
		#endif
	#endif
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
#endif 

#ifndef RGFWDEF
	#ifdef __clang__
		#define RGFWDEF static inline
	#else
		#define RGFWDEF inline
	#endif
#endif

#ifndef RGFW_ENUM
	#define RGFW_ENUM(type, name) type name; enum
#endif

#ifndef RGFW_UNUSED
	#define RGFW_UNUSED(x) (void)(x);
#endif

#if defined(__cplusplus) && !defined(__EMSCRIPTEN__)
	extern "C" {
#endif

	/* makes sure the header file part is only defined once by default */
#ifndef RGFW_HEADER

#define RGFW_HEADER

#if !defined(u8)
	#if ((defined(_MSC_VER) || defined(__SYMBIAN32__)) && !defined(RGFW_STD_INT)) /* MSVC might not have stdint.h */
		typedef unsigned char 	u8;
		typedef signed char		i8;
		typedef unsigned short  u16;
		typedef signed short 	i16;
		typedef unsigned int 	u32;
		typedef signed int		i32;
		typedef unsigned long	u64;
		typedef signed long		i64;
	#else /* use stdint standard types instead of c ""standard"" types */
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
#endif

#if !defined(b8) /* RGFW bool type */
	typedef u8 b8;
	typedef u32 b32;
#endif

#define RGFW_TRUE 1
#define RGFW_FALSE 0

/* thse OS macros looks better & are standardized */
/* plus it helps with cross-compiling */

#ifdef __EMSCRIPTEN__
	#define RGFW_WEBASM

	#ifndef RGFW_NO_API
		#define RGFW_OPENGL
	#endif

	#ifdef RGFW_EGL
		#undef RGFW_EGL
	#endif

	#include <emscripten/html5.h>
	#include <emscripten/key_codes.h>
#endif

#if defined(RGFW_X11) && defined(__APPLE__)
	#define RGFW_MACOS_X11
	#undef __APPLE__
#endif

#if defined(_WIN32) && !defined(RGFW_X11) && !defined(RGFW_WEBASM) /* (if you're using X11 on windows some how) */
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

	#if defined(RGFW_DIRECTX)
		#include <d3d11.h>
		#include <dxgi.h>
		#include <dxgi.h>
		#include <d3dcompiler.h>

		#ifndef __cplusplus
			#define __uuidof(T) IID_##T
		#endif
	#endif

#elif defined(RGFW_WAYLAND)
    #if !defined(RGFW_NO_API) && (!defined(RGFW_BUFFER) || defined(RGFW_OPENGL))
		#define RGFW_EGL
		#define RGFW_OPENGL
		#include <wayland-egl.h>
	#endif

	#include <wayland-client.h>
#elif (defined(__unix__) || defined(RGFW_MACOS_X11) || defined(RGFW_X11))  && !defined(RGFW_WEBASM)
	#define RGFW_MACOS_X11
	#define RGFW_X11
	#include <X11/Xlib.h>
#elif defined(__APPLE__) && !defined(RGFW_MACOS_X11) && !defined(RGFW_X11)  && !defined(RGFW_WEBASM)
	#define RGFW_MACOS
#endif

#if (defined(RGFW_OPENGL_ES1) || defined(RGFW_OPENGL_ES2) || defined(RGFW_OPENGL_ES3)) && !defined(RGFW_EGL)
	#define RGFW_EGL
#endif

#if !defined(RGFW_OSMESA) && !defined(RGFW_EGL) && !defined(RGFW_OPENGL) && !defined(RGFW_DIRECTX) && !defined(RGFW_BUFFER) && !defined(RGFW_NO_API)
	#define RGFW_OPENGL
#endif

#ifdef RGFW_EGL
	#if defined(__APPLE__)
		#warning  EGL is not supported for Cocoa, switching back to the native opengl api
		#undef RGFW_EGL
	#endif

	#include <EGL/egl.h>
#elif defined(RGFW_OSMESA)
	#ifndef __APPLE__
		#include <GL/osmesa.h>
	#else
		#include <OpenGL/osmesa.h>
	#endif
#endif

#if defined(RGFW_OPENGL) && defined(RGFW_X11)
	#ifndef GLX_MESA_swap_control
		#define  GLX_MESA_swap_control
	#endif
	#include <GL/glx.h> /* GLX defs, xlib.h, gl.h */
#endif

#ifndef RGFW_ALPHA
	#define RGFW_ALPHA 128 /* alpha value for RGFW_TRANSPARENT_WINDOW (WINAPI ONLY, macOS + linux don't need this) */
#endif

/*! Optional arguments for making a windows */
#define RGFW_TRANSPARENT_WINDOW		(1L<<9) /*!< the window is transparent (only properly works on X11 and MacOS, although it's although for windows) */
#define RGFW_NO_BORDER		(1L<<3) /*!< the window doesn't have border */
#define RGFW_NO_RESIZE		(1L<<4) /*!< the window cannot be resized  by the user */
#define RGFW_ALLOW_DND     (1L<<5) /*!< the window supports drag and drop*/
#define RGFW_HIDE_MOUSE (1L<<6) /*! the window should hide the mouse or not (can be toggled later on) using `RGFW_window_mouseShow*/
#define RGFW_FULLSCREEN (1L<<8) /* the window is fullscreen by default or not */
#define RGFW_CENTER (1L<<10) /*! center the window on the screen */
#define RGFW_OPENGL_SOFTWARE (1L<<11) /*! use OpenGL software rendering */
#define RGFW_COCOA_MOVE_TO_RESOURCE_DIR (1L << 12) /* (cocoa only), move to resource folder */
#define RGFW_SCALE_TO_MONITOR (1L << 13) /* scale the window to the screen */
#define RGFW_NO_INIT_API (1L << 2) /* DO not init an API (mostly for bindings, you should use `#define RGFW_NO_API` in C */

#define RGFW_NO_GPU_RENDER (1L<<14) /* don't render (using the GPU based API)*/
#define RGFW_NO_CPU_RENDER (1L<<15) /* don't render (using the CPU based buffer rendering)*/
#define RGFW_WINDOW_HIDE (1L <<  16)/* the window is hidden */

typedef RGFW_ENUM(u8, RGFW_event_types) {
	/*! event codes */
 	RGFW_keyPressed = 1, /* a key has been pressed */
	RGFW_keyReleased, /*!< a key has been released*/
	/*! key event note
		the code of the key pressed is stored in
		RGFW_Event.keyCode
		!!Keycodes defined at the bottom of the RGFW_HEADER part of this file!!

		while a string version is stored in
		RGFW_Event.KeyString

		RGFW_Event.lockState holds the current lockState
		this means if CapsLock, NumLock are active or not
	*/
	RGFW_mouseButtonPressed, /*!< a mouse button has been pressed (left,middle,right)*/
	RGFW_mouseButtonReleased, /*!< a mouse button has been released (left,middle,right)*/
	RGFW_mousePosChanged, /*!< the position of the mouse has been changed*/
	/*! mouse event note
		the x and y of the mouse can be found in the vector, RGFW_Event.point

		RGFW_Event.button holds which mouse button was pressed
	*/
	RGFW_jsButtonPressed, /*!< a joystick button was pressed */
	RGFW_jsButtonReleased, /*!< a joystick button was released */
	RGFW_jsAxisMove, /*!< an axis of a joystick was moved*/
	/*! joystick event note
		RGFW_Event.joystick holds which joystick was altered, if any
		RGFW_Event.button holds which joystick button was pressed

		RGFW_Event.axis holds the data of all the axis
		RGFW_Event.axisCount says how many axis there are
	*/
	RGFW_windowMoved, /*!< the window was moved (by the user) */
	RGFW_windowResized, /*!< the window was resized (by the user), [on webASM this means the browser was resized] */
	RGFW_focusIn, /*!< window is in focus now */
	RGFW_focusOut, /*!< window is out of focus now */
	RGFW_mouseEnter, /* mouse entered the window */
	RGFW_mouseLeave, /* mouse left the window */
	RGFW_windowRefresh, /* The window content needs to be refreshed */

	/* attribs change event note
		The event data is sent straight to the window structure
		with win->r.x, win->r.y, win->r.w and win->r.h
	*/
	RGFW_quit, /*!< the user clicked the quit button*/ 
	RGFW_dnd, /*!< a file has been dropped into the window*/
	RGFW_dnd_init /*!< the start of a dnd event, when the place where the file drop is known */
	/* dnd data note
		The x and y coords of the drop are stored in the vector RGFW_Event.point

		RGFW_Event.droppedFilesCount holds how many files were dropped

		This is also the size of the array which stores all the dropped file string,
		RGFW_Event.droppedFiles
	*/
};

/*! mouse button codes (RGFW_Event.button) */
#define RGFW_mouseLeft  1 /*!< left mouse button is pressed*/
#define RGFW_mouseMiddle  2 /*!< mouse-wheel-button is pressed*/
#define RGFW_mouseRight  3 /*!< right mouse button is pressed*/
#define RGFW_mouseScrollUp  4 /*!< mouse wheel is scrolling up*/
#define RGFW_mouseScrollDown  5 /*!< mouse wheel is scrolling down*/

#ifndef RGFW_MAX_PATH
#define RGFW_MAX_PATH 260 /* max length of a path (for dnd) */
#endif
#ifndef RGFW_MAX_DROPS
#define RGFW_MAX_DROPS 260 /* max items you can drop at once */
#endif


/* for RGFW_Event.lockstate */
#define RGFW_CAPSLOCK (1L << 1)
#define RGFW_NUMLOCK (1L << 2)

/*! joystick button codes (based on xbox/playstation), you may need to change these values per controller */
#ifndef RGFW_joystick_codes
	typedef RGFW_ENUM(u8, RGFW_joystick_codes) {
		RGFW_JS_A = 0, /*!< or PS X button */
		RGFW_JS_B = 1, /*!< or PS circle button */
		RGFW_JS_Y = 2, /*!< or PS triangle button */
		RGFW_JS_X = 3, /*!< or PS square button */
		RGFW_JS_START = 9, /*!< start button */
		RGFW_JS_SELECT = 8, /*!< select button */
		RGFW_JS_HOME = 10, /*!< home button */
		RGFW_JS_UP = 13, /*!< dpad up */
		RGFW_JS_DOWN = 14, /*!< dpad down*/
		RGFW_JS_LEFT = 15, /*!< dpad left */
		RGFW_JS_RIGHT = 16, /*!< dpad right */
		RGFW_JS_L1 = 4, /*!< left bump */
		RGFW_JS_L2 = 5, /*!< left trigger*/
		RGFW_JS_R1 = 6, /*!< right bumper */
		RGFW_JS_R2 = 7, /*!< right trigger */
	};
#endif

/*! basic vector type, if there's not already a point/vector type of choice */
#ifndef RGFW_point
	typedef struct { i32 x, y; } RGFW_point;
#endif

/*! basic rect type, if there's not already a rect type of choice */
#ifndef RGFW_rect
	typedef struct { i32 x, y, w, h; } RGFW_rect;
#endif

/*! basic area type, if there's not already a area type of choice */
#ifndef RGFW_area
	typedef struct { u32 w, h; } RGFW_area;
#endif

#ifndef __cplusplus
#define RGFW_POINT(x, y) (RGFW_point){(i32)(x), (i32)(y)}
#define RGFW_RECT(x, y, w, h) (RGFW_rect){(i32)(x), (i32)(y), (i32)(w), (i32)(h)}
#define RGFW_AREA(w, h) (RGFW_area){(u32)(w), (u32)(h)}
#else
#define RGFW_POINT(x, y) {(i32)(x), (i32)(y)}
#define RGFW_RECT(x, y, w, h) {(i32)(x), (i32)(y), (i32)(w), (i32)(h)}
#define RGFW_AREA(w, h) {(u32)(w), (u32)(h)}
#endif

#ifndef RGFW_NO_MONITOR
	/*! structure for monitor data */
	typedef struct RGFW_monitor {
		char name[128]; /*!< monitor name */
		RGFW_rect rect; /*!< monitor Workarea */
		float scaleX, scaleY; /*!< monitor content scale*/
		float physW, physH; /*!< monitor physical size */
	} RGFW_monitor;

	/*
		NOTE : Monitor functions should be ran only as many times as needed (not in a loop)
	*/

	/*! get an array of all the monitors (max 6) */
	RGFWDEF RGFW_monitor* RGFW_getMonitors(void);
	/*! get the primary monitor */
	RGFWDEF RGFW_monitor RGFW_getPrimaryMonitor(void);
#endif

/* NOTE: some parts of the data can represent different things based on the event (read comments in RGFW_Event struct) */
/*! Event structure for checking/getting events */
typedef struct RGFW_Event {
	char keyName[16]; /*!< key name of event*/

	/*! drag and drop data */
	/* 260 max paths with a max length of 260 */
#ifdef RGFW_ALLOC_DROPFILES
	char** droppedFiles;
#else
	char droppedFiles[RGFW_MAX_DROPS][RGFW_MAX_PATH]; /*!< dropped files*/
#endif
	u32 droppedFilesCount; /*!< house many files were dropped */

	u32 type; /*!< which event has been sent?*/
	RGFW_point point; /*!< mouse x, y of event (or drop point) */
	
	u8 keyCode; /*!< keycode of event 	!!Keycodes defined at the bottom of the RGFW_HEADER part of this file!! */	
	
	b8 repeat; /*!< key press event repeated (the key is being held) */
	b8 inFocus;  /*!< if the window is in focus or not (this is always true for MacOS windows due to the api being weird) */

	u8 lockState;
	
	u8 button; /* !< which mouse button was pressed */
	double scroll; /*!< the raw mouse scroll value */

	u16 joystick; /*! which joystick this event applies to (if applicable to any) */
	u8 axisesCount; /*!< number of axises */
	RGFW_point axis[2]; /*!< x, y of axises (-100 to 100) */

	u64 frameTime, frameTime2; /*!< this is used for counting the fps */
} RGFW_Event;

/*! source data for the window (used by the APIs) */
typedef struct RGFW_window_src {
#ifdef RGFW_WINDOWS
	HWND window; /*!< source window */
	HDC hdc; /*!< source HDC */
	u32 hOffset; /*!< height offset for window */
	#if (defined(RGFW_OPENGL)) && !defined(RGFW_OSMESA) && !defined(RGFW_EGL)
		HGLRC ctx; /*!< source graphics context */
	#elif defined(RGFW_OSMESA)
		OSMesaContext ctx;
	#elif defined(RGFW_DIRECTX)
		IDXGISwapChain* swapchain;
		ID3D11RenderTargetView* renderTargetView;
		ID3D11DepthStencilView* pDepthStencilView;
	#elif defined(RGFW_EGL)
		EGLSurface EGL_surface;
		EGLDisplay EGL_display;
		EGLContext EGL_context;
	#endif

	#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER) 
		HDC hdcMem;
		HBITMAP bitmap;
	#endif
	RGFW_area maxSize, minSize; /*!< for setting max/min resize (RGFW_WINDOWS) */
#elif defined(RGFW_X11)
	Display* display; /*!< source display */
	Window window; /*!< source window */
	#if (defined(RGFW_OPENGL)) && !defined(RGFW_OSMESA) && !defined(RGFW_EGL)
		GLXContext ctx; /*!< source graphics context */
	#elif defined(RGFW_OSMESA)
		OSMesaContext ctx;
	#elif defined(RGFW_EGL)
		EGLSurface EGL_surface;
		EGLDisplay EGL_display;
		EGLContext EGL_context;
	#endif

#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER) 
		XImage* bitmap;
		GC gc;
#endif
#elif defined(RGFW_WAYLAND)
	struct wl_display* display;
	struct wl_surface* surface;
	struct wl_buffer* wl_buffer;
	struct wl_keyboard* keyboard;

	struct xdg_surface* xdg_surface;
	struct xdg_toplevel* xdg_toplevel;
	struct zxdg_toplevel_decoration_v1* decoration;
	RGFW_Event events[20];
		i32 eventLen;
		size_t eventIndex;
	#if defined(RGFW_EGL)
			struct wl_egl_window* window;
			EGLSurface EGL_surface;
			EGLDisplay EGL_display;
			EGLContext EGL_context;
	#elif defined(RGFW_OSMESA)
		OSMesaContext ctx;
	#endif
#elif defined(RGFW_MACOS)
	u32 display;
	void* displayLink;
	void* window;
	b8 dndPassed;
#if (defined(RGFW_OPENGL)) && !defined(RGFW_OSMESA) && !defined(RGFW_EGL)
		void* ctx; /*!< source graphics context */
#elif defined(RGFW_OSMESA)
		OSMesaContext ctx;
#elif defined(RGFW_EGL)
		EGLSurface EGL_surface;
		EGLDisplay EGL_display;
		EGLContext EGL_context;
#endif

	void* view; /*apple viewpoint thingy*/

#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER) 
		void* bitmap; /*!< API's bitmap for storing or managing */
		void* image;
#endif
#elif defined(RGFW_WEBASM)
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
#endif
} RGFW_window_src;



typedef struct RGFW_window {
	RGFW_window_src src; /*!< src window data */

#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER) 
	u8* buffer; /*!< buffer for non-GPU systems (OSMesa, basic software rendering) */
	/* when rendering using RGFW_BUFFER, the buffer is in the RGBA format */
#endif
	void* userPtr; /* ptr for usr data */
	
	RGFW_Event event; /*!< current event */

	RGFW_rect r; /*!< the x, y, w and h of the struct */
	
	RGFW_point _lastMousePoint; /*!< last cusor point (for raw mouse data) */
	
	u32 _winArgs; /*!< windows args (for RGFW to check) */
} RGFW_window; /*!< Window structure for managing the window */

#if defined(RGFW_X11) || defined(RGFW_MACOS)
	typedef u64 RGFW_thread; /*!< thread type unix */
#else
	typedef void* RGFW_thread; /*!< thread type for window */
#endif

/** * @defgroup Window_management
* @{ */ 

/*! this has to be set before createWindow is called, else the fulscreen size is used */
RGFWDEF void RGFW_setBufferSize(RGFW_area size); /*!< the buffer cannot be resized (by RGFW) */

RGFW_window* RGFW_createWindow(
	const char* name, /* name of the window */
	RGFW_rect rect, /* rect of window */
	u16 args /* extra arguments (NULL / (u16)0 means no args used)*/
); /*!< function to create a window struct */

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

RGFW_Event* RGFW_window_checkEvent(RGFW_window* win); /*!< check current event (returns a pointer to win->event or NULL if there is no event)*/

/*!
	for RGFW_window_eventWait and RGFW_window_checkEvents
	waitMS -> Allows th	e function to keep checking for events even after `RGFW_window_checkEvent == NULL`
			  if waitMS == 0, the loop will not wait for events
			  if waitMS == a positive integer, the loop will wait that many miliseconds after there are no more events until it returns
			  if waitMS == a negative integer, the loop will not return until it gets another event
*/
typedef RGFW_ENUM(i32, RGFW_eventWait) {
	RGFW_NEXT = -1,
	RGFW_NO_WAIT = 0
};

/*! sleep until RGFW gets an event or the timer ends (defined by OS) */
RGFWDEF void RGFW_window_eventWait(RGFW_window* win, i32 waitMS);

/*!
	check all the events until there are none left, 
	this should only be used if you're using callbacks only
*/
RGFWDEF void RGFW_window_checkEvents(RGFW_window* win, i32 waitMS);

/*! 
	Tell RGFW_window_eventWait to stop waiting, to be ran from another thread
*/
RGFWDEF void RGFW_stopCheckEvents(void);

/*! window managment functions*/
RGFWDEF void RGFW_window_close(RGFW_window* win); /*!< close the window and free leftover data */

/*! moves window to a given point */
RGFWDEF void RGFW_window_move(RGFW_window* win,
	RGFW_point v/*!< new pos*/
);

#ifndef RGFW_NO_MONITOR
	/*! move to a specific monitor */
	RGFWDEF void RGFW_window_moveToMonitor(RGFW_window* win, RGFW_monitor m /* monitor */);
#endif

/*! resize window to a current size/area */
RGFWDEF void RGFW_window_resize(RGFW_window* win, /*!< source window */
	RGFW_area a/*!< new size*/
);

/*! set the minimum size a user can shrink a window to a given size/area */
RGFWDEF void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a);
/*! set the minimum size a user can extend a window to a given size/area */
RGFWDEF void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a);

RGFWDEF void RGFW_window_maximize(RGFW_window* win); /*!< maximize the window size */
RGFWDEF void RGFW_window_minimize(RGFW_window* win); /*!< minimize the window (in taskbar (per OS))*/
RGFWDEF void RGFW_window_restore(RGFW_window* win); /*!< restore the window from minimized (per OS)*/

/*! if the window should have a border or not (borderless) based on bool value of `border` */
RGFWDEF void RGFW_window_setBorder(RGFW_window* win, b8 border);

/*! turn on / off dnd (RGFW_ALLOW_DND stil must be passed to the window)*/
RGFWDEF void RGFW_window_setDND(RGFW_window* win, b8 allow);

#ifndef RGFW_NO_PASSTHROUGH
	/*!! turn on / off mouse passthrough */
	RGFWDEF void RGFW_window_setMousePassthrough(RGFW_window* win, b8 passthrough);
#endif 

/*! rename window to a given string */
RGFWDEF void RGFW_window_setName(RGFW_window* win,
	char* name
);

void RGFW_window_setIcon(RGFW_window* win, /*!< source window */
	u8* icon /*!< icon bitmap */,
	RGFW_area a /*!< width and height of the bitmap*/,
	i32 channels /*!< how many channels the bitmap has (rgb : 3, rgba : 4) */
); /*!< image resized by default */

/*!< sets mouse to bitmap (very simular to RGFW_window_setIcon), image NOT resized by default*/
RGFWDEF void RGFW_window_setMouse(RGFW_window* win, u8* image, RGFW_area a, i32 channels);

/*!< sets the mouse to a standard API cursor (based on RGFW_MOUSE, as seen at the end of the RGFW_HEADER part of this file) */
RGFWDEF	void RGFW_window_setMouseStandard(RGFW_window* win, u8 mouse);

RGFWDEF void RGFW_window_setMouseDefault(RGFW_window* win); /*!< sets the mouse to the default mouse icon */
/*
	Locks cursor at the center of the window
	win->event.point become raw mouse movement data 

	this is useful for a 3D camera
*/
RGFWDEF void RGFW_window_mouseHold(RGFW_window* win, RGFW_area area);
/*! stop holding the mouse and let it move freely */
RGFWDEF void RGFW_window_mouseUnhold(RGFW_window* win);

/*! hide the window */
RGFWDEF void RGFW_window_hide(RGFW_window* win);
/*! show the window */
RGFWDEF void RGFW_window_show(RGFW_window* win);

/*
	makes it so `RGFW_window_shouldClose` returns true
	by setting the window event.type to RGFW_quit
*/
RGFWDEF void RGFW_window_setShouldClose(RGFW_window* win);

/*! where the mouse is on the screen */
RGFWDEF RGFW_point RGFW_getGlobalMousePoint(void);

/*! where the mouse is on the window */
RGFWDEF RGFW_point RGFW_window_getMousePoint(RGFW_window* win);

/*! show the mouse or hide the mouse*/
RGFWDEF void RGFW_window_showMouse(RGFW_window* win, i8 show);
/*! move the mouse to a set x, y pos*/
RGFWDEF void RGFW_window_moveMouse(RGFW_window* win, RGFW_point v);

/*! if the window should close (RGFW_close was sent or escape was pressed) */
RGFWDEF b8 RGFW_window_shouldClose(RGFW_window* win);
/*! if window is fullscreen'd */
RGFWDEF b8 RGFW_window_isFullscreen(RGFW_window* win);
/*! if window is hidden */
RGFWDEF b8 RGFW_window_isHidden(RGFW_window* win);
/*! if window is minimized */
RGFWDEF b8 RGFW_window_isMinimized(RGFW_window* win);
/*! if window is maximized */
RGFWDEF b8 RGFW_window_isMaximized(RGFW_window* win);

/** @} */ 

/** * @defgroup Monitor
* @{ */ 

#ifndef RGFW_NO_MONITOR
/*
scale the window to the monitor,
this is run by default if the user uses the arg `RGFW_SCALE_TO_MONITOR` during window creation
*/
RGFWDEF void RGFW_window_scaleToMonitor(RGFW_window* win);
/*! get the struct of the window's monitor  */
RGFWDEF RGFW_monitor RGFW_window_getMonitor(RGFW_window* win);
#endif

/** @} */ 

/** * @defgroup Input
* @{ */ 

/*error handling*/
RGFWDEF b8 RGFW_Error(void); /*!< returns true if an error has occurred (doesn't print errors itself) */

/*! returns true if the key should be shifted */
RGFWDEF b8 RGFW_shouldShift(u32 keycode, u8 lockState);

/*! get char from RGFW keycode (using a LUT), uses shift'd version if shift = true */
RGFWDEF char RGFW_keyCodeToChar(u32 keycode, b8 shift);
/*! get char from RGFW keycode (using a LUT), uses lockState for shouldShift) */
RGFWDEF char RGFW_keyCodeToCharAuto(u32 keycode, u8 lockState);

/*! if window == NULL, it checks if the key is pressed globally. Otherwise, it checks only if the key is pressed while the window in focus.*/
RGFWDEF b8 RGFW_isPressed(RGFW_window* win, u8 key); /*!< if key is pressed (key code)*/

RGFWDEF b8 RGFW_wasPressed(RGFW_window* win, u8 key); /*!< if key was pressed (checks previous state only) (key code)*/

RGFWDEF b8 RGFW_isHeld(RGFW_window* win, u8 key); /*!< if key is held (key code)*/
RGFWDEF b8 RGFW_isReleased(RGFW_window* win, u8 key); /*!< if key is released (key code)*/

/* if a key is pressed and then released, pretty much the same as RGFW_isReleased */
RGFWDEF b8 RGFW_isClicked(RGFW_window* win, u8 key /*!< key code*/);

/*! if a mouse button is pressed */
RGFWDEF b8 RGFW_isMousePressed(RGFW_window* win, u8 button /*!< mouse button code */ );
/*! if a mouse button is held */
RGFWDEF b8 RGFW_isMouseHeld(RGFW_window* win, u8 button /*!< mouse button code */ );
/*! if a mouse button was released */
RGFWDEF b8 RGFW_isMouseReleased(RGFW_window* win, u8 button /*!< mouse button code */ );
/*! if a mouse button was pressed (checks previous state only) */
RGFWDEF b8 RGFW_wasMousePressed(RGFW_window* win, u8 button /*!< mouse button code */ );
/** @} */ 

/** * @defgroup Clipboard
* @{ */ 
RGFWDEF char* RGFW_readClipboard(size_t* size); /*!< read clipboard data */
RGFWDEF void RGFW_clipboardFree(char* str); /*!< the string returned from RGFW_readClipboard must be freed */

RGFWDEF void RGFW_writeClipboard(const char* text, u32 textLen); /*!< write text to the clipboard */
/** @} */ 

/**
	
	
	Event callbacks, 
	these are completely optional, you can use the normal 
	RGFW_checkEvent() method if you prefer that

* @defgroup Callbacks
* @{ 
*/

/*! RGFW_windowMoved, the window and its new rect value  */
typedef void (* RGFW_windowmovefunc)(RGFW_window* win, RGFW_rect r);
/*! RGFW_windowResized, the window and its new rect value  */
typedef void (* RGFW_windowresizefunc)(RGFW_window* win, RGFW_rect r);
/*! RGFW_quit, the window that was closed */
typedef void (* RGFW_windowquitfunc)(RGFW_window* win);
/*! RGFW_focusIn / RGFW_focusOut, the window who's focus has changed and if its inFocus */
typedef void (* RGFW_focusfunc)(RGFW_window* win, b8 inFocus);
/*! RGFW_mouseEnter / RGFW_mouseLeave, the window that changed, the point of the mouse (enter only) and if the mouse has entered */
typedef void (* RGFW_mouseNotifyfunc)(RGFW_window* win, RGFW_point point, b8 status);
/*! RGFW_mousePosChanged, the window that the move happened on and the new point of the mouse  */
typedef void (* RGFW_mouseposfunc)(RGFW_window* win, RGFW_point point);
/*! RGFW_dnd_init, the window, the point of the drop on the windows */
typedef void (* RGFW_dndInitfunc)(RGFW_window* win, RGFW_point point);
/*! RGFW_windowRefresh, the window that needs to be refreshed */
typedef void (* RGFW_windowrefreshfunc)(RGFW_window* win);
/*! RGFW_keyPressed / RGFW_keyReleased, the window that got the event, the keycode, the string version, the state of mod keys, if it was a press (else it's a release) */
typedef void (* RGFW_keyfunc)(RGFW_window* win, u32 keycode, char keyName[16], u8 lockState, b8 pressed);
/*! RGFW_mouseButtonPressed / RGFW_mouseButtonReleased, the window that got the event, the button that was pressed, the scroll value, if it was a press (else it's a release)  */
typedef void (* RGFW_mousebuttonfunc)(RGFW_window* win, u8 button, double scroll, b8 pressed);
/*! RGFW_jsButtonPressed / RGFW_jsButtonReleased, the window that got the event, the button that was pressed, the scroll value, if it was a press (else it's a release) */
typedef void (* RGFW_jsButtonfunc)(RGFW_window* win, u16 joystick, u8 button, b8 pressed);
/*! RGFW_jsAxisMove, the window that got the event, the joystick in question, the axis values and the amount of axises */
typedef void (* RGFW_jsAxisfunc)(RGFW_window* win, u16 joystick, RGFW_point axis[2], u8 axisesCount);


/*!  RGFW_dnd, the window that had the drop, the drop data and the amount files dropped returns previous callback function (if it was set) */
#ifdef RGFW_ALLOC_DROPFILES
	typedef void (* RGFW_dndfunc)(RGFW_window* win, char** droppedFiles, u32 droppedFilesCount);
#else
	typedef void (* RGFW_dndfunc)(RGFW_window* win, char droppedFiles[RGFW_MAX_DROPS][RGFW_MAX_PATH], u32 droppedFilesCount);
#endif
/*! set callback for a window move event returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowmovefunc RGFW_setWindowMoveCallback(RGFW_windowmovefunc func);
/*! set callback for a window resize event returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowresizefunc RGFW_setWindowResizeCallback(RGFW_windowresizefunc func);
/*! set callback for a window quit event returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowquitfunc RGFW_setWindowQuitCallback(RGFW_windowquitfunc func);
/*! set callback for a mouse move event returns previous callback function (if it was set)  */
RGFWDEF RGFW_mouseposfunc RGFW_setMousePosCallback(RGFW_mouseposfunc func);
/*! set callback for a window refresh event returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowrefreshfunc RGFW_setWindowRefreshCallback(RGFW_windowrefreshfunc func);
/*! set callback for a window focus change event returns previous callback function (if it was set)  */
RGFWDEF RGFW_focusfunc RGFW_setFocusCallback(RGFW_focusfunc func);
/*! set callback for a mouse notify event returns previous callback function (if it was set)  */
RGFWDEF RGFW_mouseNotifyfunc RGFW_setMouseNotifyCallBack(RGFW_mouseNotifyfunc func);
/*! set callback for a drop event event returns previous callback function (if it was set)  */
RGFWDEF RGFW_dndfunc RGFW_setDndCallback(RGFW_dndfunc func);
/*! set callback for a start of a drop event returns previous callback function (if it was set)  */
RGFWDEF RGFW_dndInitfunc RGFW_setDndInitCallback(RGFW_dndInitfunc func);
/*! set callback for a key (press / release ) event returns previous callback function (if it was set)  */
RGFWDEF RGFW_keyfunc RGFW_setKeyCallback(RGFW_keyfunc func);
/*! set callback for a mouse button (press / release ) event returns previous callback function (if it was set)  */
RGFWDEF RGFW_mousebuttonfunc RGFW_setMouseButtonCallback(RGFW_mousebuttonfunc func);
/*! set callback for a controller button (press / release ) event returns previous callback function (if it was set)  */
RGFWDEF RGFW_jsButtonfunc RGFW_setjsButtonCallback(RGFW_jsButtonfunc func);
/*! set callback for a joystick axis mov event returns previous callback function (if it was set)  */
RGFWDEF RGFW_jsAxisfunc RGFW_setjsAxisCallback(RGFW_jsAxisfunc func);

/** @} */ 

/** * @defgroup Threads
* @{ */ 

#ifndef RGFW_NO_THREADS
	/*! threading functions*/

	/*! NOTE! (for X11/linux) : if you define a window in a thread, it must be run after the original thread's window is created or else there will be a memory error */
	/*
		I'd suggest you use sili's threading functions instead
		if you're going to use sili
		which is a good idea generally
	*/

	#if defined(__unix__) || defined(__APPLE__) || defined(RGFW_WEBASM) 
		typedef void* (* RGFW_threadFunc_ptr)(void*);
	#else
		typedef DWORD (__stdcall *RGFW_threadFunc_ptr) (LPVOID lpThreadParameter);  
	#endif

	RGFWDEF RGFW_thread RGFW_createThread(RGFW_threadFunc_ptr ptr, void* args); /*!< create a thread*/
	RGFWDEF void RGFW_cancelThread(RGFW_thread thread); /*!< cancels a thread*/
	RGFWDEF void RGFW_joinThread(RGFW_thread thread); /*!< join thread to current thread */
	RGFWDEF void RGFW_setThreadPriority(RGFW_thread thread, u8 priority); /*!< sets the priority priority  */
#endif

/** @} */ 

/** * @defgroup joystick
* @{ */ 

/*! joystick count starts at 0*/
/*!< register joystick to window based on a number (the number is based on when it was connected eg. /dev/js0)*/
RGFWDEF u16 RGFW_registerJoystick(RGFW_window* win, i32 jsNumber);
RGFWDEF u16 RGFW_registerJoystickF(RGFW_window* win, char* file);

RGFWDEF u32 RGFW_isPressedJS(RGFW_window* win, u16 controller, u8 button);

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

/*< updates fps / sets fps to cap (must by ran manually by the user at the end of a frame), returns current fps */
RGFWDEF u32 RGFW_window_checkFPS(RGFW_window* win, u32 fpsCap);

/* supports openGL, directX, OSMesa, EGL and software rendering */
RGFWDEF void RGFW_window_swapBuffers(RGFW_window* win); /*!< swap the rendering buffer */
RGFWDEF void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval);

RGFWDEF void RGFW_window_setGPURender(RGFW_window* win, i8 set);
RGFWDEF void RGFW_window_setCPURender(RGFW_window* win, i8 set);

/*! native API functions */
#if defined(RGFW_OPENGL) || defined(RGFW_EGL)
	/*! OpenGL init hints */
	RGFWDEF void RGFW_setGLStencil(i32 stencil); /*!< set stencil buffer bit size (8 by default) */
	RGFWDEF void RGFW_setGLSamples(i32 samples); /*!< set number of sampiling buffers (4 by default) */
	RGFWDEF void RGFW_setGLStereo(i32 stereo); /*!< use GL_STEREO (GL_FALSE by default) */
	RGFWDEF void RGFW_setGLAuxBuffers(i32 auxBuffers); /*!< number of aux buffers (0 by default) */

	/*! which profile to use for the opengl verion */
	typedef RGFW_ENUM(u8, RGFW_GL_profile)  { RGFW_GL_CORE = 0,  RGFW_GL_COMPATIBILITY  };
	/*! Set OpenGL version hint (core or compatibility profile)*/
	RGFWDEF void RGFW_setGLVersion(RGFW_GL_profile profile, i32 major, i32 minor);
	RGFWDEF void RGFW_setDoubleBuffer(b8 useDoubleBuffer); 
    RGFWDEF void* RGFW_getProcAddress(const char* procname); /*!< get native opengl proc address */
    RGFWDEF void RGFW_window_makeCurrent_OpenGL(RGFW_window* win); /*!< to be called by RGFW_window_makeCurrent */
#elif defined(RGFW_DIRECTX)
	typedef struct {
		IDXGIFactory* pFactory;
		IDXGIAdapter* pAdapter;
		ID3D11Device* pDevice;
		ID3D11DeviceContext* pDeviceContext;
	} RGFW_directXinfo;

	/*
		RGFW stores a global instance of RGFW_directXinfo,
		you can use this function to get a pointer the instance
	*/
	RGFWDEF RGFW_directXinfo* RGFW_getDirectXInfo(void);
#endif

/** @} */ 

/** * @defgroup Supporting
* @{ */ 
RGFWDEF u64 RGFW_getTime(void); /*!< get time in seconds */
RGFWDEF u64 RGFW_getTimeNS(void); /*!< get time in nanoseconds */
RGFWDEF void RGFW_sleep(u64 milisecond); /*!< sleep for a set time */

/*!
	key codes and mouse icon enums
*/

typedef RGFW_ENUM(u8, RGFW_Key) {
	RGFW_KEY_NULL = 0,
	RGFW_Escape,
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

	RGFW_Backtick,

	RGFW_0,
	RGFW_1,
	RGFW_2,
	RGFW_3,
	RGFW_4,
	RGFW_5,
	RGFW_6,
	RGFW_7,
	RGFW_8,
	RGFW_9,

	RGFW_Minus,
	RGFW_Equals,
	RGFW_BackSpace,
	RGFW_Tab,
	RGFW_CapsLock,
	RGFW_ShiftL,
	RGFW_ControlL,
	RGFW_AltL,
	RGFW_SuperL,
	RGFW_ShiftR,
	RGFW_ControlR,
	RGFW_AltR,
	RGFW_SuperR,
	RGFW_Space,

	RGFW_a,
	RGFW_b,
	RGFW_c,
	RGFW_d,
	RGFW_e,
	RGFW_f,
	RGFW_g,
	RGFW_h,
	RGFW_i,
	RGFW_j,
	RGFW_k,
	RGFW_l,
	RGFW_m,
	RGFW_n,
	RGFW_o,
	RGFW_p,
	RGFW_q,
	RGFW_r,
	RGFW_s,
	RGFW_t,
	RGFW_u,
	RGFW_v,
	RGFW_w,
	RGFW_x,
	RGFW_y,
	RGFW_z,

	RGFW_Period,
	RGFW_Comma,
	RGFW_Slash,
	RGFW_Bracket,
	RGFW_CloseBracket,
	RGFW_Semicolon,
	RGFW_Return,
	RGFW_Quote,
	RGFW_BackSlash,

	RGFW_Up,
	RGFW_Down,
	RGFW_Left,
	RGFW_Right,

	RGFW_Delete,
	RGFW_Insert,
	RGFW_End,
	RGFW_Home,
	RGFW_PageUp,
	RGFW_PageDown,

	RGFW_Numlock,
	RGFW_KP_Slash,
	RGFW_Multiply,
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

	final_key,
};


typedef RGFW_ENUM(u8, RGFW_mouseIcons) {
	RGFW_MOUSE_NORMAL = 0,
	RGFW_MOUSE_ARROW,
	RGFW_MOUSE_IBEAM,
	RGFW_MOUSE_CROSSHAIR,
	RGFW_MOUSE_POINTING_HAND,
	RGFW_MOUSE_RESIZE_EW,
	RGFW_MOUSE_RESIZE_NS,
	RGFW_MOUSE_RESIZE_NWSE,
	RGFW_MOUSE_RESIZE_NESW,
	RGFW_MOUSE_RESIZE_ALL,
	RGFW_MOUSE_NOT_ALLOWED,
};

/** @} */ 

#endif /* RGFW_HEADER */

/*
Example to get you started :

linux : gcc main.c -lX11 -lXcursor -lGL
windows : gcc main.c -lopengl32 -lshell32 -lgdi32
macos : gcc main.c -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo

#define RGFW_IMPLEMENTATION
#include "RGFW.h"

u8 icon[4 * 3 * 3] = {0xFF, 0x00, 0x00, 0xFF,    0xFF, 0x00, 0x00, 0xFF,     0xFF, 0x00, 0x00, 0xFF,   0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,     0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF};

int main() {
	RGFW_window* win = RGFW_createWindow("name", RGFW_RECT(500, 500, 500, 500), (u64)0);

	RGFW_window_setIcon(win, icon, RGFW_AREA(3, 3), 4);

	for (;;) {
		RGFW_window_checkEvent(win); // NOTE: checking events outside of a while loop may cause input lag
		if (win->event.type == RGFW_quit || RGFW_isPressed(win, RGFW_Escape))
			break;

		RGFW_window_swapBuffers(win);

		glClearColor(0xFF, 0XFF, 0xFF, 0xFF);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	RGFW_window_close(win);
}

	compiling :

	if you wish to compile the library all you have to do is create a new file with this in it

	rgfw.c
	#define RGFW_IMPLEMENTATION
	#include "RGFW.h"

	then you can use gcc (or whatever compile you wish to use) to compile the library into object file

	ex. gcc -c RGFW.c -fPIC

	after you compile the library into an object file, you can also turn the object file into an static or shared library

	(commands ar and gcc can be replaced with whatever equivalent your system uses)
	static : ar rcs RGFW.a RGFW.o
	shared :
		windows:
			gcc -shared RGFW.o -lopengl32 -lshell32 -lgdi32 -o RGFW.dll
		linux:
			gcc -shared RGFW.o -lX11 -lXcursor -lGL -o RGFW.so
		macos:
			gcc -shared RGFW.o -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo
*/

#ifdef RGFW_X11
	#define RGFW_OS_BASED_VALUE(l, w, m, h, ww) l
#elif defined(RGFW_WINDOWS)
	#define RGFW_OS_BASED_VALUE(l, w, m, h, ww) w
#elif defined(RGFW_MACOS)
	#define RGFW_OS_BASED_VALUE(l, w, m, h, ww) m
#elif defined(RGFW_WEBASM)
	#define RGFW_OS_BASED_VALUE(l, w, m, h, ww) h
#elif defined(RGFW_WAYLAND)
    #define RGFW_OS_BASED_VALUE(l, w, m, h, ww) ww  
#endif


#ifdef RGFW_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/*
RGFW_IMPLEMENTATION starts with generic RGFW defines

This is the start of keycode data

	Why not use macros instead of the numbers itself?
	Windows -> Not all virtual keys are macros (VK_0 - VK_1, VK_a - VK_z)
	Linux -> Only symcodes are values, (XK_0 - XK_1, XK_a - XK_z) are larger than 0xFF00, I can't find any way to work with them without making the array an unreasonable size
	MacOS -> windows and linux already don't have keycodes as macros, so there's no point
*/



/* 
	the c++ compiler doesn't support setting up an array like, 
	we'll have to do it during runtime using a function & this messy setup
*/
#ifndef __cplusplus
#define RGFW_NEXT ,
#define RGFW_MAP
#else 
#define RGFW_NEXT ;
#define RGFW_MAP RGFW_keycodes
#endif

#ifdef RGFW_WAYLAND
#include <linux/input-event-codes.h>
#endif

u8 RGFW_keycodes [RGFW_OS_BASED_VALUE(136, 337, 128, DOM_VK_WIN_OEM_CLEAR + 1, 130)] = {
#ifdef __cplusplus
	0
};
void RGFW_init_keys(void) {
#endif
	RGFW_MAP [RGFW_OS_BASED_VALUE(49, 192, 50, DOM_VK_BACK_QUOTE, KEY_GRAVE)] = RGFW_Backtick 		RGFW_NEXT

	RGFW_MAP [RGFW_OS_BASED_VALUE(19, 0x30, 29, DOM_VK_0, KEY_0)] = RGFW_0 					RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(10, 0x31, 18, DOM_VK_1, KEY_1)] = RGFW_1						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(11, 0x32, 19, DOM_VK_2, KEY_2)] = RGFW_2						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(12, 0x33, 20, DOM_VK_3, KEY_3)] = RGFW_3						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(13, 0x34, 21, DOM_VK_4, KEY_4)] = RGFW_4						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(14, 0x35, 23, DOM_VK_5, KEY_5)] = RGFW_5                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(15, 0x36, 22, DOM_VK_6, KEY_6)] = RGFW_6                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(16, 0x37, 26, DOM_VK_7, KEY_7)] = RGFW_7                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(17, 0x38, 28, DOM_VK_8, KEY_8)] = RGFW_8                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(18, 0x39, 25, DOM_VK_9, KEY_9)] = RGFW_9,

	RGFW_MAP [RGFW_OS_BASED_VALUE(65, 0x20, 49, DOM_VK_SPACE, KEY_SPACE)] = RGFW_Space,

	RGFW_MAP [RGFW_OS_BASED_VALUE(38, 0x41, 0, DOM_VK_A, KEY_A)] = RGFW_a                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(56, 0x42, 11, DOM_VK_B, KEY_B)] = RGFW_b                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(54, 0x43, 8, DOM_VK_C, KEY_C)] = RGFW_c                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(40, 0x44, 2, DOM_VK_D, KEY_D)] = RGFW_d                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(26, 0x45, 14, DOM_VK_E, KEY_E)] = RGFW_e                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(41, 0x46, 3, DOM_VK_F, KEY_F)] = RGFW_f                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(42, 0x47, 5, DOM_VK_G, KEY_G)] = RGFW_g                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(43, 0x48, 4, DOM_VK_H, KEY_H)] = RGFW_h                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(31, 0x49, 34, DOM_VK_I, KEY_I)] = RGFW_i                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(44, 0x4A, 38, DOM_VK_J, KEY_J)] = RGFW_j                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(45, 0x4B, 40, DOM_VK_K, KEY_K)] = RGFW_k                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(46, 0x4C, 37, DOM_VK_L, KEY_L)] = RGFW_l                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(58, 0x4D, 46, DOM_VK_M, KEY_M)] = RGFW_m                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(57, 0x4E, 45, DOM_VK_N, KEY_N)] = RGFW_n                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(32, 0x4F, 31, DOM_VK_O, KEY_O)] = RGFW_o                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(33, 0x50, 35, DOM_VK_P, KEY_P)] = RGFW_p                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(24, 0x51, 12, DOM_VK_Q, KEY_Q)] = RGFW_q                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(27, 0x52, 15, DOM_VK_R, KEY_R)] = RGFW_r                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(39, 0x53, 1, DOM_VK_S, KEY_S)] = RGFW_s                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(28, 0x54, 17, DOM_VK_T, KEY_T)] = RGFW_t                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(30, 0x55, 32, DOM_VK_U, KEY_U)] = RGFW_u                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(55, 0x56, 9, DOM_VK_V, KEY_V)] = RGFW_v                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(25, 0x57, 13, DOM_VK_W, KEY_W)] = RGFW_w                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(53, 0x58, 7, DOM_VK_X, KEY_X)] = RGFW_x                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(29, 0x59, 16, DOM_VK_Y, KEY_Y)] = RGFW_y                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(52, 0x5A, 6, DOM_VK_Z, KEY_Z)] = RGFW_z,

	RGFW_MAP [RGFW_OS_BASED_VALUE(60, 190, 47, DOM_VK_PERIOD, KEY_DOT)] = RGFW_Period             			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(59, 188, 43, DOM_VK_COMMA, KEY_COMMA)] = RGFW_Comma               			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(61, 191, 44, DOM_VK_SLASH, KEY_SLASH)] = RGFW_Slash               			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(34, 219, 33, DOM_VK_OPEN_BRACKET, KEY_LEFTBRACE)] = RGFW_Bracket      			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(35, 221, 30, DOM_VK_CLOSE_BRACKET, KEY_RIGHTBRACE)] = RGFW_CloseBracket             RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(47, 186, 41, DOM_VK_SEMICOLON, KEY_SEMICOLON)] = RGFW_Semicolon                 RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(48, 222, 39, DOM_VK_QUOTE, KEY_APOSTROPHE)] = RGFW_Quote                 			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(51, 322, 42, DOM_VK_BACK_SLASH, KEY_BACKSLASH)] = RGFW_BackSlash,
	
	RGFW_MAP [RGFW_OS_BASED_VALUE(36, 0x0D, 36, DOM_VK_RETURN, KEY_ENTER)] = RGFW_Return              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(119, 0x2E, 118, DOM_VK_DELETE, KEY_DELETE)] = RGFW_Delete                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(77, 0x90, 72, DOM_VK_NUM_LOCK, KEY_NUMLOCK)] = RGFW_Numlock               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(106, 0x6F, 82, DOM_VK_DIVIDE, KEY_KPSLASH)] = RGFW_KP_Slash               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(63, 0x6A, 76, DOM_VK_MULTIPLY, KEY_KPASTERISK)] = RGFW_Multiply              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(82, 0x6D, 67, DOM_VK_SUBTRACT, KEY_KPMINUS)] = RGFW_KP_Minus              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(87, 0x61, 84, DOM_VK_NUMPAD1, KEY_KP1)] = RGFW_KP_1               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(88, 0x62, 85, DOM_VK_NUMPAD2, KEY_KP2)] = RGFW_KP_2               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(89, 0x63, 86, DOM_VK_NUMPAD3, KEY_KP3)] = RGFW_KP_3               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(83, 0x64, 87, DOM_VK_NUMPAD4, KEY_KP4)] = RGFW_KP_4               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(84, 0x65, 88, DOM_VK_NUMPAD5, KEY_KP5)] = RGFW_KP_5               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(85, 0x66, 89, DOM_VK_NUMPAD6, KEY_KP6)] = RGFW_KP_6               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(79, 0x67, 90, DOM_VK_NUMPAD7, KEY_KP7)] = RGFW_KP_7               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(80, 0x68, 92, DOM_VK_NUMPAD8, KEY_KP8)] = RGFW_KP_8               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(81, 0x69, 93, DOM_VK_NUMPAD9, KEY_KP9)] = RGFW_KP_9               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(90, 0x60, 83, DOM_VK_NUMPAD0, KEY_KP0)] = RGFW_KP_0               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(91, 0x6E, 65, DOM_VK_DECIMAL, KEY_KPDOT)] = RGFW_KP_Period              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(104, 0x92, 77, 0, KEY_KPENTER)] = RGFW_KP_Return,
	
	RGFW_MAP [RGFW_OS_BASED_VALUE(20, 189, 27, DOM_VK_HYPHEN_MINUS, KEY_MINUS)] = RGFW_Minus              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(21, 187, 24, DOM_VK_EQUALS, KEY_EQUAL)] = RGFW_Equals               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(22, 8, 51, DOM_VK_BACK_SPACE, KEY_BACKSPACE)] = RGFW_BackSpace              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(23, 0x09, 48, DOM_VK_TAB, KEY_TAB)] = RGFW_Tab                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(66, 20, 57, DOM_VK_CAPS_LOCK, KEY_CAPSLOCK)] = RGFW_CapsLock               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(50, 0x10, 56, DOM_VK_SHIFT, KEY_LEFTSHIFT)] = RGFW_ShiftL               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(37, 0x11, 59, DOM_VK_CONTROL, KEY_LEFTCTRL)] = RGFW_ControlL               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(64,0x12, 58, DOM_VK_ALT, KEY_LEFTALT)] = RGFW_AltL                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(133, 0x5B, 55, DOM_VK_WIN, KEY_LEFTMETA)] = RGFW_SuperL,
	
	#if !defined(RGFW_WINDOWS) && !defined(RGFW_MACOS) && !defined(RGFW_WEBASM)
	RGFW_MAP [RGFW_OS_BASED_VALUE(105, 0x11, 59, 0, KEY_RIGHTCTRL)] = RGFW_ControlR               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(135, 0xA4, 55, 0, KEY_RIGHTMETA)] = RGFW_SuperR,
	RGFW_MAP [RGFW_OS_BASED_VALUE(62, 0x5C, 56, 0, KEY_RIGHTSHIFT)] = RGFW_ShiftR              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(108, 165, 58, 0, KEY_RIGHTALT)] = RGFW_AltR,
	#endif

	RGFW_MAP [RGFW_OS_BASED_VALUE(67, 0x70, 127, DOM_VK_F1, KEY_F1)] = RGFW_F1                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(68, 0x71, 121, DOM_VK_F2, KEY_F2)] = RGFW_F2                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(69, 0x72, 100, DOM_VK_F3, KEY_F3)] = RGFW_F3                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(70, 0x73, 119, DOM_VK_F4, KEY_F4)] = RGFW_F4                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(71, 0x74, 97, DOM_VK_F5, KEY_F5)] = RGFW_F5              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(72, 0x75, 98, DOM_VK_F6, KEY_F6)] = RGFW_F6              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(73, 0x76, 99, DOM_VK_F7, KEY_F7)] = RGFW_F7              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(74, 0x77, 101, DOM_VK_F8, KEY_F8)] = RGFW_F8                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(75, 0x78, 102, DOM_VK_F9, KEY_F9)] = RGFW_F9                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(76, 0x79, 110, DOM_VK_F10, KEY_F10)] = RGFW_F10               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(95, 0x7A, 104, DOM_VK_F11, KEY_F11)] = RGFW_F11               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(96, 0x7B, 112, DOM_VK_F12, KEY_F12)] = RGFW_F12               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(111, 0x26, 126, DOM_VK_UP, KEY_UP)] = RGFW_Up                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(116, 0x28, 125, DOM_VK_DOWN, KEY_DOWN)] = RGFW_Down                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(113, 0x25, 123, DOM_VK_LEFT, KEY_LEFT)] = RGFW_Left                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(114, 0x27, 124, DOM_VK_RIGHT, KEY_RIGHT)] = RGFW_Right              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(118, 0x2D, 115, DOM_VK_INSERT, KEY_INSERT)] = RGFW_Insert                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(115, 0x23, 120, DOM_VK_END, KEY_END)] = RGFW_End                  		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(112, 336, 117, DOM_VK_PAGE_UP, KEY_PAGEUP)] = RGFW_PageUp                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(117, 325, 122, DOM_VK_PAGE_DOWN, KEY_PAGEDOWN)] = RGFW_PageDown            RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(9, 0x1B, 53, DOM_VK_ESCAPE, KEY_ESC)] = RGFW_Escape                   		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(110, 0x24, 116, DOM_VK_HOME, KEY_HOME)] = RGFW_Home                    		RGFW_NEXT
#ifndef __cplusplus
};
#else 
}
#endif

#undef RGFW_NEXT
#undef RGFW_MAP

typedef struct {
	b8 current  : 1;
	b8 prev  : 1;
} RGFW_keyState;

RGFW_keyState RGFW_keyboard[final_key] = { {0, 0} };

RGFWDEF u32 RGFW_apiKeyCodeToRGFW(u32 keycode);

u32 RGFW_apiKeyCodeToRGFW(u32 keycode) {
	#ifdef __cplusplus
	if (RGFW_OS_BASED_VALUE(49, 192, 50, DOM_VK_BACK_QUOTE, KEY_GRAVE) != RGFW_Backtick) {
		RGFW_init_keys();
	}
	#endif

	/* make sure the key isn't out of bounds */
	if (keycode > sizeof(RGFW_keycodes) / sizeof(u8))
		return 0;
	
	return RGFW_keycodes[keycode];
}

RGFWDEF void RGFW_resetKey(void);
void RGFW_resetKey(void) {
	size_t len = final_key; /*!< last_key == length */
	
	size_t i; /*!< reset each previous state  */
	for (i = 0; i < len; i++)
		RGFW_keyboard[i].prev = 0;
}

b8 RGFW_shouldShift(u32 keycode, u8 lockState) {
    #define RGFW_xor(x, y) (( (x) && (!(y)) ) ||  ((y) && (!(x)) ))
    b8 caps4caps = (lockState & RGFW_CAPSLOCK) && ((keycode >= RGFW_a) && (keycode <= RGFW_z));
    b8 shouldShift = RGFW_xor((RGFW_isPressed(NULL, RGFW_ShiftL) || RGFW_isPressed(NULL, RGFW_ShiftR)), caps4caps);
    #undef RGFW_xor

	return shouldShift;
}	

char RGFW_keyCodeToChar(u32 keycode, b8 shift) {
    static const char map[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '`', '0', '1', '2', '3', '4', '5', '6', '7', '8', 
        '9', '-', '=', 0, '\t',  0, 0, 0, 0, 0, 0, 0, 0, 0, ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '.', ',', '/', '[', ']',  ';', '\n', '\'', '\\', 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  '/', '*', '-', '1', '2', '3',  '3', '5', '6', '7', '8',  '9', '0', '\n'
    };

    static const char mapCaps[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '~', ')', '!', '@', '#', '$', '%', '^', '&', '*', 
        '(', '_', '+', 0, '0',  0, 0, 0, 0, 0, 0, 0, 0, 0, ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
        'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
        'X', 'Y', 'Z', '>', '<', '?', '{', '}',  ':', '\n', '"', '|', 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '?', '*', '-', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    if (shift == RGFW_FALSE)
        return map[keycode]; 
    return mapCaps[keycode];
}

char RGFW_keyCodeToCharAuto(u32 keycode, u8 lockState) { return RGFW_keyCodeToChar(keycode, RGFW_shouldShift(keycode, lockState)); }

/*
	this is the end of keycode data
*/

/* joystick data */
u8 RGFW_jsPressed[4][16]; /*!< if a key is currently pressed or not (per joystick) */

i32 RGFW_joysticks[4]; /*!< limit of 4 joysticks at a time */
u16 RGFW_joystickCount; /*!< the actual amount of joysticks */

/* 
	event callback defines start here
*/


/*
	These exist to avoid the 
	if (func == NULL) check 
	for (allegedly) better performance
*/
void RGFW_windowmovefuncEMPTY(RGFW_window* win, RGFW_rect r) { RGFW_UNUSED(win); RGFW_UNUSED(r); }
void RGFW_windowresizefuncEMPTY(RGFW_window* win, RGFW_rect r) { RGFW_UNUSED(win); RGFW_UNUSED(r); }
void RGFW_windowquitfuncEMPTY(RGFW_window* win) { RGFW_UNUSED(win); }
void RGFW_focusfuncEMPTY(RGFW_window* win, b8 inFocus) {RGFW_UNUSED(win); RGFW_UNUSED(inFocus);}
void RGFW_mouseNotifyfuncEMPTY(RGFW_window* win, RGFW_point point, b8 status) {RGFW_UNUSED(win); RGFW_UNUSED(point); RGFW_UNUSED(status);}
void RGFW_mouseposfuncEMPTY(RGFW_window* win, RGFW_point point) {RGFW_UNUSED(win); RGFW_UNUSED(point);}
void RGFW_dndInitfuncEMPTY(RGFW_window* win, RGFW_point point) {RGFW_UNUSED(win); RGFW_UNUSED(point);}
void RGFW_windowrefreshfuncEMPTY(RGFW_window* win) {RGFW_UNUSED(win); }
void RGFW_keyfuncEMPTY(RGFW_window* win, u32 keycode, char keyName[16], u8 lockState, b8 pressed) {RGFW_UNUSED(win); RGFW_UNUSED(keycode); RGFW_UNUSED(keyName); RGFW_UNUSED(lockState); RGFW_UNUSED(pressed);}
void RGFW_mousebuttonfuncEMPTY(RGFW_window* win, u8 button, double scroll, b8 pressed) {RGFW_UNUSED(win); RGFW_UNUSED(button); RGFW_UNUSED(scroll); RGFW_UNUSED(pressed);}
void RGFW_jsButtonfuncEMPTY(RGFW_window* win, u16 joystick, u8 button, b8 pressed){RGFW_UNUSED(win); RGFW_UNUSED(joystick); RGFW_UNUSED(button); RGFW_UNUSED(pressed); }
void RGFW_jsAxisfuncEMPTY(RGFW_window* win, u16 joystick, RGFW_point axis[2], u8 axisesCount){RGFW_UNUSED(win); RGFW_UNUSED(joystick); RGFW_UNUSED(axis); RGFW_UNUSED(axisesCount); }

#ifdef RGFW_ALLOC_DROPFILES
void RGFW_dndfuncEMPTY(RGFW_window* win, char** droppedFiles, u32 droppedFilesCount) {RGFW_UNUSED(win); RGFW_UNUSED(droppedFiles); RGFW_UNUSED(droppedFilesCount);}
#else
void RGFW_dndfuncEMPTY(RGFW_window* win, char droppedFiles[RGFW_MAX_DROPS][RGFW_MAX_PATH], u32 droppedFilesCount) {RGFW_UNUSED(win); RGFW_UNUSED(droppedFiles); RGFW_UNUSED(droppedFilesCount);}
#endif

RGFW_windowmovefunc RGFW_windowMoveCallback = RGFW_windowmovefuncEMPTY;
RGFW_windowresizefunc RGFW_windowResizeCallback = RGFW_windowresizefuncEMPTY;
RGFW_windowquitfunc RGFW_windowQuitCallback = RGFW_windowquitfuncEMPTY;
RGFW_mouseposfunc RGFW_mousePosCallback = RGFW_mouseposfuncEMPTY;
RGFW_windowrefreshfunc RGFW_windowRefreshCallback = RGFW_windowrefreshfuncEMPTY;
RGFW_focusfunc RGFW_focusCallback = RGFW_focusfuncEMPTY;
RGFW_mouseNotifyfunc RGFW_mouseNotifyCallBack = RGFW_mouseNotifyfuncEMPTY;
RGFW_dndfunc RGFW_dndCallback = RGFW_dndfuncEMPTY;
RGFW_dndInitfunc RGFW_dndInitCallback = RGFW_dndInitfuncEMPTY;
RGFW_keyfunc RGFW_keyCallback = RGFW_keyfuncEMPTY;
RGFW_mousebuttonfunc RGFW_mouseButtonCallback = RGFW_mousebuttonfuncEMPTY;
RGFW_jsButtonfunc RGFW_jsButtonCallback = RGFW_jsButtonfuncEMPTY;
RGFW_jsAxisfunc RGFW_jsAxisCallback = RGFW_jsAxisfuncEMPTY;

void RGFW_window_checkEvents(RGFW_window* win, i32 waitMS) { 
	RGFW_window_eventWait(win, waitMS);

	while (RGFW_window_checkEvent(win) != NULL && RGFW_window_shouldClose(win) == 0) { 
		if (win->event.type == RGFW_quit) return; 
	}
	
	#ifdef RGFW_WEBASM /* webasm needs to run the sleep function for asyncify */
		RGFW_sleep(0);
	#endif
}

RGFW_windowmovefunc RGFW_setWindowMoveCallback(RGFW_windowmovefunc func) { 
	RGFW_windowmovefunc	prev =  (RGFW_windowMoveCallback == RGFW_windowmovefuncEMPTY) ? NULL : RGFW_windowMoveCallback;
	RGFW_windowMoveCallback = func;
	return prev;
}
RGFW_windowresizefunc RGFW_setWindowResizeCallback(RGFW_windowresizefunc func) {
    RGFW_windowresizefunc prev = (RGFW_windowResizeCallback == RGFW_windowresizefuncEMPTY) ? NULL : RGFW_windowResizeCallback;
    RGFW_windowResizeCallback = func;
    return prev;
}
RGFW_windowquitfunc RGFW_setWindowQuitCallback(RGFW_windowquitfunc func) {
    RGFW_windowquitfunc prev = (RGFW_windowQuitCallback == RGFW_windowquitfuncEMPTY) ? NULL : RGFW_windowQuitCallback;
    RGFW_windowQuitCallback = func;
    return prev;
}

RGFW_mouseposfunc RGFW_setMousePosCallback(RGFW_mouseposfunc func) {
    RGFW_mouseposfunc prev = (RGFW_mousePosCallback == RGFW_mouseposfuncEMPTY) ? NULL : RGFW_mousePosCallback;
    RGFW_mousePosCallback = func;
    return prev;
}
RGFW_windowrefreshfunc RGFW_setWindowRefreshCallback(RGFW_windowrefreshfunc func) {
    RGFW_windowrefreshfunc prev = (RGFW_windowRefreshCallback == RGFW_windowrefreshfuncEMPTY) ? NULL : RGFW_windowRefreshCallback;
    RGFW_windowRefreshCallback = func;
    return prev;
}
RGFW_focusfunc RGFW_setFocusCallback(RGFW_focusfunc func) {
    RGFW_focusfunc prev = (RGFW_focusCallback == RGFW_focusfuncEMPTY) ? NULL : RGFW_focusCallback;
    RGFW_focusCallback = func;
    return prev;
}

RGFW_mouseNotifyfunc RGFW_setMouseNotifyCallBack(RGFW_mouseNotifyfunc func) {
    RGFW_mouseNotifyfunc prev = (RGFW_mouseNotifyCallBack == RGFW_mouseNotifyfuncEMPTY) ? NULL : RGFW_mouseNotifyCallBack;
    RGFW_mouseNotifyCallBack = func;
    return prev;
}
RGFW_dndfunc RGFW_setDndCallback(RGFW_dndfunc func) {
    RGFW_dndfunc prev = (RGFW_dndCallback == RGFW_dndfuncEMPTY) ? NULL : RGFW_dndCallback;
    RGFW_dndCallback = func;
    return prev;
}
RGFW_dndInitfunc RGFW_setDndInitCallback(RGFW_dndInitfunc func) {
    RGFW_dndInitfunc prev = (RGFW_dndInitCallback == RGFW_dndInitfuncEMPTY) ? NULL : RGFW_dndInitCallback;
    RGFW_dndInitCallback = func;
    return prev;
}
RGFW_keyfunc RGFW_setKeyCallback(RGFW_keyfunc func) {
    RGFW_keyfunc prev = (RGFW_keyCallback == RGFW_keyfuncEMPTY) ? NULL : RGFW_keyCallback;
    RGFW_keyCallback = func;
    return prev;
}
RGFW_mousebuttonfunc RGFW_setMouseButtonCallback(RGFW_mousebuttonfunc func) {
    RGFW_mousebuttonfunc prev = (RGFW_mouseButtonCallback == RGFW_mousebuttonfuncEMPTY) ? NULL : RGFW_mouseButtonCallback;
    RGFW_mouseButtonCallback = func;
    return prev;
}
RGFW_jsButtonfunc RGFW_setjsButtonCallback(RGFW_jsButtonfunc func) {
    RGFW_jsButtonfunc prev = (RGFW_jsButtonCallback == RGFW_jsButtonfuncEMPTY) ? NULL : RGFW_jsButtonCallback;
    RGFW_jsButtonCallback = func;
    return prev;
}
RGFW_jsAxisfunc RGFW_setjsAxisCallback(RGFW_jsAxisfunc func) {
    RGFW_jsAxisfunc prev = (RGFW_jsAxisCallback == RGFW_jsAxisfuncEMPTY) ? NULL : RGFW_jsAxisCallback;
    RGFW_jsAxisCallback = func;
    return prev;
}
/* 
no more event call back defines
*/

#define RGFW_ASSERT(check, str) {\
	if (!(check)) { \
		printf(str); \
		assert(check); \
	} \
}

b8 RGFW_error = 0;
b8 RGFW_Error(void) { return RGFW_error; }

#define SET_ATTRIB(a, v) { \
    assert(((size_t) index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[index++] = a; \
    attribs[index++] = v; \
}
	
RGFW_area RGFW_bufferSize = {0, 0};
void RGFW_setBufferSize(RGFW_area size) {
	RGFW_bufferSize = size;
}


RGFWDEF RGFW_window* RGFW_window_basic_init(RGFW_rect rect, u16 args);

/* do a basic initialization for RGFW_window, this is to standard it for each OS */
RGFW_window* RGFW_window_basic_init(RGFW_rect rect, u16 args) {
	RGFW_window* win = (RGFW_window*) RGFW_MALLOC(sizeof(RGFW_window)); /*!< make a new RGFW struct */

	/* clear out dnd info */
#ifdef RGFW_ALLOC_DROPFILES
	win->event.droppedFiles = (char**) RGFW_MALLOC(sizeof(char*) * RGFW_MAX_DROPS);
	u32 i;
	for (i = 0; i < RGFW_MAX_DROPS; i++)
		win->event.droppedFiles[i] = (char*) RGFW_CALLOC(RGFW_MAX_PATH, sizeof(char));
#endif

	/* X11 requires us to have a display to get the screen size */
	#ifndef RGFW_X11 
	RGFW_area screenR = RGFW_getScreenSize();
	#else
	win->src.display = XOpenDisplay(NULL);
	assert(win->src.display != NULL);

	Screen* scrn = DefaultScreenOfDisplay((Display*)win->src.display);
	RGFW_area screenR = RGFW_AREA((u32)scrn->width, (u32)scrn->height);
	#endif
	
	/* rect based the requested args */
	if (args & RGFW_FULLSCREEN)
		rect = RGFW_RECT(0, 0, screenR.w, screenR.h);

	if (args & RGFW_CENTER)
		rect = RGFW_RECT((screenR.w - rect.w) / 2, (screenR.h - rect.h) / 2, rect.w, rect.h);

	/* set and init the new window's data */
	win->r = rect;
	win->event.inFocus = 1;
	win->event.droppedFilesCount = 0;
	RGFW_joystickCount = 0;
	win->_winArgs = 0;
	win->event.lockState = 0;

	return win;
}

#ifndef RGFW_NO_MONITOR
void RGFW_window_scaleToMonitor(RGFW_window* win) {
	RGFW_monitor monitor = RGFW_window_getMonitor(win);
	
	RGFW_window_resize(win, RGFW_AREA(((u32) monitor.scaleX) * win->r.w, ((u32) monitor.scaleX) * win->r.h));
}
#endif

RGFW_window* RGFW_root = NULL;


#define RGFW_HOLD_MOUSE			(1L<<2) /*!< hold the moues still */
#define RGFW_MOUSE_LEFT 		(1L<<3) /* if mouse left the window */

void RGFW_clipboardFree(char* str) { RGFW_FREE(str); }

RGFW_keyState RGFW_mouseButtons[5] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} };

b8 RGFW_isMousePressed(RGFW_window* win, u8 button) {
	assert(win != NULL);
	return RGFW_mouseButtons[button].current && (win != NULL) && win->event.inFocus; 
}
b8 RGFW_wasMousePressed(RGFW_window* win, u8 button) {
	assert(win != NULL); 
	return RGFW_mouseButtons[button].prev && (win != NULL) && win->event.inFocus; 
}
b8 RGFW_isMouseHeld(RGFW_window* win, u8 button) {
	return (RGFW_isMousePressed(win, button) && RGFW_wasMousePressed(win, button));
}
b8 RGFW_isMouseReleased(RGFW_window* win, u8 button) {
	return (!RGFW_isMousePressed(win, button) && RGFW_wasMousePressed(win, button));	
}

b8 RGFW_isPressed(RGFW_window* win, u8 key) {
	return RGFW_keyboard[key].current && (win == NULL || win->event.inFocus);
}

b8 RGFW_wasPressed(RGFW_window* win, u8 key) {
	return RGFW_keyboard[key].prev && (win == NULL || win->event.inFocus);
}

b8 RGFW_isHeld(RGFW_window* win, u8 key) {
	return (RGFW_isPressed(win, key) && RGFW_wasPressed(win, key));
}

b8 RGFW_isClicked(RGFW_window* win, u8 key) {
	return (RGFW_wasPressed(win, key) && !RGFW_isPressed(win, key));
}

b8 RGFW_isReleased(RGFW_window* win, u8 key) {
	return (!RGFW_isPressed(win, key) && RGFW_wasPressed(win, key));	
}

#if defined(RGFW_WINDOWS)  && defined(RGFW_DIRECTX) /* defines for directX context*/
	RGFW_directXinfo RGFW_dxInfo;
	RGFW_directXinfo* RGFW_getDirectXInfo(void) { return &RGFW_dxInfo; }
#endif

void RGFW_window_makeCurrent(RGFW_window* win) {
#if defined(RGFW_WINDOWS) && defined(RGFW_DIRECTX)
	if (win == NULL)
		RGFW_dxInfo.pDeviceContext->lpVtbl->OMSetRenderTargets(RGFW_dxInfo.pDeviceContext, 1, NULL, NULL);
	else
		RGFW_dxInfo.pDeviceContext->lpVtbl->OMSetRenderTargets(RGFW_dxInfo.pDeviceContext, 1, &win->src.renderTargetView, NULL);
#elif defined(RGFW_OPENGL)
	RGFW_window_makeCurrent_OpenGL(win);
#else
	RGFW_UNUSED(win)
#endif
}

void RGFW_window_setGPURender(RGFW_window* win, i8 set) {
	if (!set && !(win->_winArgs & RGFW_NO_GPU_RENDER))
		win->_winArgs |= RGFW_NO_GPU_RENDER;
		
	else if (set && win->_winArgs & RGFW_NO_GPU_RENDER)
		win->_winArgs ^= RGFW_NO_GPU_RENDER;
}

void RGFW_window_setCPURender(RGFW_window* win, i8 set) {
	if (!set && !(win->_winArgs & RGFW_NO_CPU_RENDER))
		win->_winArgs |= RGFW_NO_CPU_RENDER;

	else if (set && win->_winArgs & RGFW_NO_CPU_RENDER)
		win->_winArgs ^= RGFW_NO_CPU_RENDER;
}

void RGFW_window_maximize(RGFW_window* win) {
	assert(win != NULL);

	RGFW_area screen = RGFW_getScreenSize();

	RGFW_window_move(win, RGFW_POINT(0, 0));
	RGFW_window_resize(win, screen);
}

b8 RGFW_window_shouldClose(RGFW_window* win) {
	assert(win != NULL);
	return (win->event.type == RGFW_quit || RGFW_isPressed(win, RGFW_Escape));
}

void RGFW_window_setShouldClose(RGFW_window* win) { win->event.type = RGFW_quit; RGFW_windowQuitCallback(win); }

#ifndef RGFW_NO_MONITOR
	void RGFW_window_moveToMonitor(RGFW_window* win, RGFW_monitor m) {
		RGFW_window_move(win, RGFW_POINT(m.rect.x + win->r.x, m.rect.y + win->r.y));
	}
#endif

RGFWDEF void RGFW_captureCursor(RGFW_window* win, RGFW_rect);
RGFWDEF void RGFW_releaseCursor(RGFW_window* win);

void RGFW_window_mouseHold(RGFW_window* win, RGFW_area area) {
	if ((win->_winArgs & RGFW_HOLD_MOUSE))
		return;
	

	if (!area.w && !area.h)
		area = RGFW_AREA(win->r.w / 2, win->r.h / 2);
		
	win->_winArgs |= RGFW_HOLD_MOUSE;
	RGFW_captureCursor(win, win->r);
	RGFW_window_moveMouse(win, RGFW_POINT(win->r.x + (win->r.w / 2), win->r.y + (win->r.h / 2)));
}

void RGFW_window_mouseUnhold(RGFW_window* win) {
	if ((win->_winArgs & RGFW_HOLD_MOUSE)) {
		win->_winArgs ^= RGFW_HOLD_MOUSE;

		RGFW_releaseCursor(win);
	}
}

u32 RGFW_window_checkFPS(RGFW_window* win, u32 fpsCap) {
	u64 deltaTime = RGFW_getTimeNS() - win->event.frameTime;

	u32 output_fps = 0;
	u64 fps = round(1e+9 / deltaTime);
	output_fps= fps;

	if (fpsCap && fps > fpsCap) {
		u64 frameTimeNS = 1e+9 / fpsCap;
		u64 sleepTimeMS = (frameTimeNS - deltaTime) / 1e6;

		if (sleepTimeMS > 0) {
			RGFW_sleep(sleepTimeMS);
			win->event.frameTime = 0;
		}
	}

	win->event.frameTime = RGFW_getTimeNS();
	
	if (fpsCap == 0) 
		return (u32) output_fps;
	
	deltaTime = RGFW_getTimeNS() - win->event.frameTime2;
	output_fps = round(1e+9 / deltaTime);
	win->event.frameTime2 = RGFW_getTimeNS();

	return output_fps;
}

u32 RGFW_isPressedJS(RGFW_window* win, u16 c, u8 button) { 
	RGFW_UNUSED(win);
	return RGFW_jsPressed[c][button]; 
}

#if defined(RGFW_X11) || defined(RGFW_WINDOWS)
	void RGFW_window_showMouse(RGFW_window* win, i8 show) {
		static u8 RGFW_blk[] = { 0, 0, 0, 0 };
		if (show == 0)
			RGFW_window_setMouse(win, RGFW_blk, RGFW_AREA(1, 1), 4);
		else
			RGFW_window_setMouseDefault(win);
	}
#endif

RGFWDEF void RGFW_updateLockState(RGFW_window* win, b8 capital, b8 numlock);	
void RGFW_updateLockState(RGFW_window* win, b8 capital, b8 numlock) {
	if (capital && !(win->event.lockState & RGFW_CAPSLOCK))
		win->event.lockState |= RGFW_CAPSLOCK;
	else if (!capital && (win->event.lockState & RGFW_CAPSLOCK))			
		win->event.lockState ^= RGFW_CAPSLOCK;
	
	if (numlock && !(win->event.lockState & RGFW_NUMLOCK))
		win->event.lockState |= RGFW_NUMLOCK;
	else if (!numlock && (win->event.lockState & RGFW_NUMLOCK))
		win->event.lockState ^= RGFW_NUMLOCK;
}

#if defined(RGFW_X11) || defined(RGFW_MACOS) || defined(RGFW_WEBASM) || defined(RGFW_WAYLAND)
	struct timespec;

	int nanosleep(const struct timespec* duration, struct timespec* rem);
	int clock_gettime(clockid_t clk_id, struct timespec* tp);
	int setenv(const char *name, const char *value, int overwrite);

	void RGFW_window_setDND(RGFW_window* win, b8 allow) {
		if (allow && !(win->_winArgs & RGFW_ALLOW_DND))
			win->_winArgs |= RGFW_ALLOW_DND;

		else if (!allow && (win->_winArgs & RGFW_ALLOW_DND))
			win->_winArgs ^= RGFW_ALLOW_DND;
	}
#endif

/*
	graphics API specific code (end of generic code)
	starts here 
*/


/* 
	OpenGL defines start here   (Normal, EGL, OSMesa)
*/

#if defined(RGFW_OPENGL) || defined(RGFW_EGL) || defined(RGFW_OSMESA)
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
#if !defined(RGFW_OSMESA) 
	i32 RGFW_majorVersion = 0, RGFW_minorVersion = 0;
	b8 RGFW_profile = RGFW_GL_CORE;
	
	#ifndef RGFW_EGL
	i32 RGFW_STENCIL = 8, RGFW_SAMPLES = 4, RGFW_STEREO = 0, RGFW_AUX_BUFFERS = 0, RGFW_DOUBLE_BUFFER = 1;
	#else
	i32 RGFW_STENCIL = 0, RGFW_SAMPLES = 0, RGFW_STEREO = 0, RGFW_AUX_BUFFERS = 0, RGFW_DOUBLE_BUFFER = 1;
	#endif


	void RGFW_setGLStencil(i32 stencil) { RGFW_STENCIL = stencil; }
	void RGFW_setGLSamples(i32 samples) { RGFW_SAMPLES = samples; }
	void RGFW_setGLStereo(i32 stereo) { RGFW_STEREO = stereo; }
	void RGFW_setGLAuxBuffers(i32 auxBuffers) { RGFW_AUX_BUFFERS = auxBuffers; }
	void RGFW_setDoubleBuffer(b8 useDoubleBuffer) { RGFW_DOUBLE_BUFFER = useDoubleBuffer; }

	void RGFW_setGLVersion(b8 profile, i32 major, i32 minor) {
        RGFW_profile = profile;
		RGFW_majorVersion = major;
		RGFW_minorVersion = minor;
	}

/* OPENGL normal only (no EGL / OSMesa) */
#ifndef RGFW_EGL

#define RGFW_GL_RENDER_TYPE 		RGFW_OS_BASED_VALUE(GLX_X_VISUAL_TYPE,    	0x2003,		73, 0, 0)
	#define RGFW_GL_ALPHA_SIZE 		RGFW_OS_BASED_VALUE(GLX_ALPHA_SIZE,       	0x201b,		11,     0, 0)
	#define RGFW_GL_DEPTH_SIZE 		RGFW_OS_BASED_VALUE(GLX_DEPTH_SIZE,       	0x2022,		12,     0, 0)
	#define RGFW_GL_DOUBLEBUFFER 		RGFW_OS_BASED_VALUE(GLX_DOUBLEBUFFER,     	0x2011, 	5,  0, 0)   
	#define RGFW_GL_STENCIL_SIZE 		RGFW_OS_BASED_VALUE(GLX_STENCIL_SIZE,	 	0x2023,	13,     0, 0)
	#define RGFW_GL_SAMPLES			RGFW_OS_BASED_VALUE(GLX_SAMPLES, 		 	0x2042,	    55,     0, 0)
	#define RGFW_GL_STEREO 			RGFW_OS_BASED_VALUE(GLX_STEREO,	 		 	0x2012,			6,  0, 0)
	#define RGFW_GL_AUX_BUFFERS		RGFW_OS_BASED_VALUE(GLX_AUX_BUFFERS,	    0x2024,	7, 		    0, 0)

#if defined(RGFW_X11) || defined(RGFW_WINDOWS)
	#define RGFW_GL_DRAW 			RGFW_OS_BASED_VALUE(GLX_X_RENDERABLE,	 	0x2001,					0, 0, 0)
	#define RGFW_GL_DRAW_TYPE 		RGFW_OS_BASED_VALUE(GLX_RENDER_TYPE,     	0x2013,						0, 0, 0)
	#define RGFW_GL_FULL_FORMAT		RGFW_OS_BASED_VALUE(GLX_TRUE_COLOR,   	 	0x2027,						0, 0, 0)
	#define RGFW_GL_RED_SIZE		RGFW_OS_BASED_VALUE(GLX_RED_SIZE,         	0x2015,						0, 0, 0)
	#define RGFW_GL_GREEN_SIZE		RGFW_OS_BASED_VALUE(GLX_GREEN_SIZE,       	0x2017,						0, 0, 0)
	#define RGFW_GL_BLUE_SIZE		RGFW_OS_BASED_VALUE(GLX_BLUE_SIZE, 	 		0x2019,						0, 0, 0)
	#define RGFW_GL_USE_RGBA		RGFW_OS_BASED_VALUE(GLX_RGBA_BIT,   	 	0x202B,						0, 0, 0)
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
	static u32* RGFW_initFormatAttribs(u32 useSoftware) {
		RGFW_UNUSED(useSoftware);
		static u32 attribs[] = {
								#if defined(RGFW_X11) || defined(RGFW_WINDOWS)
								RGFW_GL_RENDER_TYPE,
								RGFW_GL_FULL_FORMAT,
								#endif
								RGFW_GL_ALPHA_SIZE      , 8,
								RGFW_GL_DEPTH_SIZE      , 24,
								#if defined(RGFW_X11) || defined(RGFW_WINDOWS)
								RGFW_GL_DRAW, 1,
								RGFW_GL_RED_SIZE        , 8,
								RGFW_GL_GREEN_SIZE      , 8,
								RGFW_GL_BLUE_SIZE       , 8,
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

								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		size_t index = (sizeof(attribs) / sizeof(attribs[0])) - 13;

#define RGFW_GL_ADD_ATTRIB(attrib, attVal) \
		if (attVal) { \
			attribs[index] = attrib;\
			attribs[index + 1] = attVal;\
			index += 2;\
		}
         
        RGFW_GL_ADD_ATTRIB(RGFW_GL_DOUBLEBUFFER, 1);
        
        RGFW_GL_ADD_ATTRIB(RGFW_GL_STENCIL_SIZE, RGFW_STENCIL);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_STEREO, RGFW_STEREO);
		RGFW_GL_ADD_ATTRIB(RGFW_GL_AUX_BUFFERS, RGFW_AUX_BUFFERS);

#ifndef RGFW_X11
		RGFW_GL_ADD_ATTRIB(RGFW_GL_SAMPLES, RGFW_SAMPLES);
#endif 

#ifdef RGFW_MACOS
		if (useSoftware) {
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

		if (RGFW_majorVersion >= 4 || RGFW_majorVersion >= 3) {
			attribs[index + 1] = (u32) ((RGFW_majorVersion >= 4) ? 0x4100 : 0x3200);
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
	PFNEGLCHOOSECONFIGPROC eglChooseConfigSource;
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


	void RGFW_createOpenGLContext(RGFW_window* win) {
#if defined(RGFW_LINK_EGL)
		eglInitializeSource = (PFNEGLINITIALIZEPROC) eglGetProcAddress("eglInitialize");
		eglGetConfigsSource = (PFNEGLGETCONFIGSPROC) eglGetProcAddress("eglGetConfigs");
		eglChooseConfigSource = (PFNEGLCHOOSECONFIGPROC) eglGetProcAddress("eglChooseConfig");
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
#endif /* RGFW_LINK_EGL */

		#ifdef RGFW_WINDOWS
		win->src.EGL_display = eglGetDisplay((EGLNativeDisplayType) win->src.hdc);
		#else
		win->src.EGL_display = eglGetDisplay((EGLNativeDisplayType) win->src.display);
		#endif

		EGLint major, minor;

		eglInitialize(win->src.EGL_display, &major, &minor);

		#ifndef EGL_OPENGL_ES1_BIT
		#define EGL_OPENGL_ES1_BIT 0x1
		#endif

		EGLint egl_config[] = {
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

		EGLConfig config;
		EGLint numConfigs;
		eglChooseConfig(win->src.EGL_display, egl_config, &config, 1, &numConfigs);


		win->src.EGL_surface = eglCreateWindowSurface(win->src.EGL_display, config, (EGLNativeWindowType) win->src.window, NULL);

		EGLint attribs[] = {
			EGL_CONTEXT_CLIENT_VERSION,
			#ifdef RGFW_OPENGL_ES1
			1,
			#else
			2,
			#endif
			EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE
		};

		size_t index = 4;
		RGFW_GL_ADD_ATTRIB(EGL_STENCIL_SIZE, RGFW_STENCIL);
		RGFW_GL_ADD_ATTRIB(EGL_SAMPLES, RGFW_SAMPLES);

        if (RGFW_DOUBLE_BUFFER)
            RGFW_GL_ADD_ATTRIB(EGL_RENDER_BUFFER, EGL_BACK_BUFFER);

		if (RGFW_majorVersion) {
			attribs[1] = RGFW_majorVersion;
	
			RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_MAJOR_VERSION, RGFW_majorVersion);
			RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_MINOR_VERSION, RGFW_minorVersion);

			if (RGFW_profile == RGFW_GL_CORE) {
				RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT);
			}
			else {
				RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT);
			}

		}

		#if defined(RGFW_OPENGL_ES1) || defined(RGFW_OPENGL_ES2) || defined(RGFW_OPENGL_ES3)
		eglBindAPI(EGL_OPENGL_ES_API);
		#else
		eglBindAPI(EGL_OPENGL_API);		
		#endif
      		
		win->src.EGL_context = eglCreateContext(win->src.EGL_display, config, EGL_NO_CONTEXT, attribs);
		
		if (win->src.EGL_context == NULL)
			fprintf(stderr, "failed to create an EGL opengl context\n");

		eglMakeCurrent(win->src.EGL_display, win->src.EGL_surface, win->src.EGL_surface, win->src.EGL_context);
		eglSwapBuffers(win->src.EGL_display, win->src.EGL_surface);
	}

	void RGFW_window_makeCurrent_OpenGL(RGFW_window* win) {
		eglMakeCurrent(win->src.EGL_display, win->src.EGL_surface, win->src.EGL_surface, win->src.EGL_context);
	}

	#ifdef RGFW_APPLE
	void* RGFWnsglFramework = NULL;
	#elif defined(RGFW_WINDOWS)
	static HMODULE wglinstance = NULL;
	#endif

	void* RGFW_getProcAddress(const char* procname) { 
		#if defined(RGFW_WINDOWS)
			void* proc = (void*) GetProcAddress(wglinstance, procname); 

			if (proc)
				return proc;
		#endif

		return (void*) eglGetProcAddress(procname); 
	}

	void RGFW_closeEGL(RGFW_window* win) {
		eglDestroySurface(win->src.EGL_display, win->src.EGL_surface);
		eglDestroyContext(win->src.EGL_display, win->src.EGL_context);

		eglTerminate(win->src.EGL_display);
	}
	
	void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) {
		assert(win != NULL);
		
		eglSwapInterval(win->src.EGL_display, swapInterval);

	}
#endif /* RGFW_EGL */

/* 
	end of RGFW_EGL defines
*/

/* OPENGL Normal / EGL defines only (no OS MESA)  Ends here */

#elif defined(RGFW_OSMESA) /* OSmesa only */
RGFWDEF void RGFW_OSMesa_reorganize(void);

/* reorganize buffer for osmesa */
void RGFW_OSMesa_reorganize(void) {
	u8* row = (u8*) RGFW_MALLOC(win->r.w * 3);

	i32 half_height = win->r.h / 2;
	i32 stride = win->r.w * 3;

	i32 y;
	for (y = 0; y < half_height; ++y) {
		i32 top_offset = y * stride;
		i32 bottom_offset = (win->r.h - y - 1) * stride;
		memcpy(row, win->buffer + top_offset, stride);
		memcpy(win->buffer + top_offset, win->buffer + bottom_offset, stride);
		memcpy(win->buffer + bottom_offset, row, stride);
	}

	RGFW_FREE(row);
}
#endif /* RGFW_OSMesa */

#endif /* RGFW_GL (OpenGL, EGL, OSMesa )*/

/*
This is where OS specific stuff starts
*/


#if defined(RGFW_WAYLAND) || defined(RGFW_X11)
	int RGFW_eventWait_forceStop[] = {0, 0, 0}; /* for wait events */

	#ifdef __linux__
		#include <linux/joystick.h>
		#include <fcntl.h>
		#include <unistd.h>
		
		RGFW_Event* RGFW_linux_updateJoystick(RGFW_window* win) {
			static int xAxis = 0, yAxis = 0;
			u8 i;
			for (i = 0; i < RGFW_joystickCount; i++) {
				struct js_event e;


				if (RGFW_joysticks[i] == 0)
					continue;

				i32 flags = fcntl(RGFW_joysticks[i], F_GETFL, 0);
				fcntl(RGFW_joysticks[i], F_SETFL, flags | O_NONBLOCK);

				ssize_t bytes;
				while ((bytes = read(RGFW_joysticks[i], &e, sizeof(e))) > 0) {
					switch (e.type) {
					case JS_EVENT_BUTTON:
						win->event.type = e.value ? RGFW_jsButtonPressed : RGFW_jsButtonReleased;
						win->event.button = e.number;
						RGFW_jsPressed[i][e.number] = e.value;
						RGFW_jsButtonCallback(win, i, e.number, e.value);
						return &win->event;
					case JS_EVENT_AXIS:
						ioctl(RGFW_joysticks[i], JSIOCGAXES, &win->event.axisesCount);

						if ((e.number == 0 || e.number % 2) && e.number != 1)
							xAxis = e.value;
						else
							yAxis = e.value;

						win->event.axis[e.number / 2].x = xAxis;
						win->event.axis[e.number / 2].y = yAxis;
						win->event.type = RGFW_jsAxisMove;
						win->event.joystick = i;
						RGFW_jsAxisCallback(win, i, win->event.axis, win->event.axisesCount);
						return &win->event;

						default: break;
					}
				}
			}

			return NULL;
		}

	#endif
#endif

/*


Start of Linux / Unix defines


*/

#ifdef RGFW_X11
#ifndef RGFW_NO_X11_CURSOR
#include <X11/Xcursor/Xcursor.h>
#endif
#include <dlfcn.h>

#ifndef RGFW_NO_DPI
#include <X11/extensions/Xrandr.h>
#include <X11/Xresource.h>
#endif

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <unistd.h>

#include <X11/XKBlib.h> /* for converting keycode to string */
#include <X11/cursorfont.h> /* for hiding */
#include <X11/extensions/shapeconst.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XInput2.h>

#include <limits.h> /* for data limits (mainly used in drag and drop functions) */
#include <poll.h>


#ifdef __linux__
#include <linux/joystick.h>
#endif

	u8 RGFW_mouseIconSrc[] = { XC_arrow, XC_left_ptr, XC_xterm, XC_crosshair, XC_hand2, XC_sb_h_double_arrow, XC_sb_v_double_arrow, XC_bottom_left_corner, XC_bottom_right_corner, XC_fleur, XC_X_cursor};  
	/*atoms needed for drag and drop*/
	Atom XdndAware, XdndTypeList, XdndSelection, XdndEnter, XdndPosition, XdndStatus, XdndLeave, XdndDrop, XdndFinished, XdndActionCopy, XdndActionMove, XdndActionLink, XdndActionAsk, XdndActionPrivate;

	Atom wm_delete_window = 0;

#if !defined(RGFW_NO_X11_CURSOR) && !defined(RGFW_NO_X11_CURSOR_PRELOAD)
	typedef XcursorImage* (*PFN_XcursorImageCreate)(int, int);
	typedef void (*PFN_XcursorImageDestroy)(XcursorImage*);
	typedef Cursor(*PFN_XcursorImageLoadCursor)(Display*, const XcursorImage*);
#endif
#ifdef RGFW_OPENGL
	typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
#endif

#if !defined(RGFW_NO_X11_XI_PRELOAD)
	typedef int (* PFN_XISelectEvents)(Display*,Window,XIEventMask*,int);
	PFN_XISelectEvents XISelectEventsSrc = NULL;
	#define XISelectEvents XISelectEventsSrc

	void* X11Xihandle = NULL;
#endif

#if !defined(RGFW_NO_X11_CURSOR) && !defined(RGFW_NO_X11_CURSOR_PRELOAD)
	PFN_XcursorImageLoadCursor XcursorImageLoadCursorSrc = NULL;
	PFN_XcursorImageCreate XcursorImageCreateSrc = NULL;
	PFN_XcursorImageDestroy XcursorImageDestroySrc = NULL;

#define XcursorImageLoadCursor XcursorImageLoadCursorSrc
#define XcursorImageCreate XcursorImageCreateSrc
#define XcursorImageDestroy XcursorImageDestroySrc

	void* X11Cursorhandle = NULL;
#endif

	u32 RGFW_windowsOpen = 0;

#ifdef RGFW_OPENGL
	void* RGFW_getProcAddress(const char* procname) { return (void*) glXGetProcAddress((GLubyte*) procname); }
#endif

	RGFWDEF void RGFW_init_buffer(RGFW_window* win, XVisualInfo* vi);
	void RGFW_init_buffer(RGFW_window* win, XVisualInfo* vi) {
#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
		if (RGFW_bufferSize.w == 0 && RGFW_bufferSize.h == 0)
			RGFW_bufferSize = RGFW_getScreenSize();
		
		win->buffer = (u8*)RGFW_MALLOC(RGFW_bufferSize.w * RGFW_bufferSize.h * 4);

		#ifdef RGFW_OSMESA
				win->src.ctx = OSMesaCreateContext(OSMESA_RGBA, NULL);
				OSMesaMakeCurrent(win->src.ctx, win->buffer, GL_UNSIGNED_BYTE, win->r.w, win->r.h);
		#endif

		win->src.bitmap = XCreateImage(
			win->src.display, XDefaultVisual(win->src.display, vi->screen),
			vi->depth,
			ZPixmap, 0, NULL, RGFW_bufferSize.w, RGFW_bufferSize.h,
			32, 0
		);

		win->src.gc = XCreateGC(win->src.display, win->src.window, 0, NULL);

		#else
		RGFW_UNUSED(win); /*!< if buffer rendering is not being used */
		RGFW_UNUSED(vi)
		#endif
	}



	void RGFW_window_setBorder(RGFW_window* win, u8 border) {
		static Atom _MOTIF_WM_HINTS = 0;
		if (_MOTIF_WM_HINTS == 0 )
			_MOTIF_WM_HINTS = XInternAtom(win->src.display, "_MOTIF_WM_HINTS", False);
		
		struct __x11WindowHints {
			unsigned long flags, functions, decorations, status;
			long input_mode;
		} hints;
		hints.flags = (1L << 1);
		hints.decorations = border;

		XChangeProperty(
			win->src.display, win->src.window,
			_MOTIF_WM_HINTS, _MOTIF_WM_HINTS,
			32, PropModeReplace, (u8*)&hints, 5
		);
	}
	
	void RGFW_releaseCursor(RGFW_window* win) {
		XUngrabPointer(win->src.display, CurrentTime);

		/* disable raw input */
		unsigned char mask[] = { 0 };
		XIEventMask em;
		em.deviceid = XIAllMasterDevices;
		em.mask_len = sizeof(mask);
		em.mask = mask;

		XISelectEvents(win->src.display, XDefaultRootWindow(win->src.display), &em, 1);
	}
	
	void RGFW_captureCursor(RGFW_window* win, RGFW_rect r) { 
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
	}

	RGFW_window* RGFW_createWindow(const char* name, RGFW_rect rect, u16 args) {
#if !defined(RGFW_NO_X11_CURSOR) && !defined(RGFW_NO_X11_CURSOR_PRELOAD)
		if (X11Cursorhandle == NULL) {
#if defined(__CYGWIN__)
			X11Cursorhandle = dlopen("libXcursor-1.so", RTLD_LAZY | RTLD_LOCAL);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
			X11Cursorhandle = dlopen("libXcursor.so", RTLD_LAZY | RTLD_LOCAL);
#else
			X11Cursorhandle = dlopen("libXcursor.so.1", RTLD_LAZY | RTLD_LOCAL);
#endif

			XcursorImageCreateSrc = (PFN_XcursorImageCreate) dlsym(X11Cursorhandle, "XcursorImageCreate");
			XcursorImageDestroySrc = (PFN_XcursorImageDestroy) dlsym(X11Cursorhandle, "XcursorImageDestroy");
			XcursorImageLoadCursorSrc = (PFN_XcursorImageLoadCursor) dlsym(X11Cursorhandle, "XcursorImageLoadCursor");
		}
#endif

#if !defined(RGFW_NO_X11_XI_PRELOAD)
		if (X11Xihandle == NULL) {
#if defined(__CYGWIN__)
			X11Xihandle = dlopen("libXi-6.so", RTLD_LAZY | RTLD_LOCAL);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
			X11Xihandle = dlopen("libXi.so", RTLD_LAZY | RTLD_LOCAL);
#else
			X11Xihandle = dlopen("libXi.so.6", RTLD_LAZY | RTLD_LOCAL);
#endif

			XISelectEventsSrc = (PFN_XISelectEvents) dlsym(X11Xihandle, "XISelectEvents");
		}
#endif

		XInitThreads(); /*!< init X11 threading*/

		if (args & RGFW_OPENGL_SOFTWARE)
			setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);

		RGFW_window* win = RGFW_window_basic_init(rect, args);

		u64 event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask | LeaveWindowMask | EnterWindowMask | ExposureMask; /*!< X11 events accepted*/

#ifdef RGFW_OPENGL
		u32* visual_attribs = RGFW_initFormatAttribs(args & RGFW_OPENGL_SOFTWARE);
		i32 fbcount;
		GLXFBConfig* fbc = glXChooseFBConfig((Display*) win->src.display, DefaultScreen(win->src.display), (i32*) visual_attribs, &fbcount);

		i32 best_fbc = -1;

		if (fbcount == 0) {
			printf("Failed to find any valid GLX visual configs\n");
			return NULL;
		}

		u32 i;
		for (i = 0; i < (u32)fbcount; i++) {
			XVisualInfo* vi = glXGetVisualFromFBConfig((Display*) win->src.display, fbc[i]);
                        if (vi == NULL)
				continue;
                        
			XFree(vi);

			i32 samp_buf, samples;
			glXGetFBConfigAttrib((Display*) win->src.display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
			glXGetFBConfigAttrib((Display*) win->src.display, fbc[i], GLX_SAMPLES, &samples);
			
			if ((best_fbc < 0 || samp_buf) && (samples == RGFW_SAMPLES || best_fbc == -1)) {
				best_fbc = i;
			}
		}

		if (best_fbc == -1) {
			printf("Failed to get a valid GLX visual\n");
			return NULL;
		}

		GLXFBConfig bestFbc = fbc[best_fbc];

		/* Get a visual */
		XVisualInfo* vi = glXGetVisualFromFBConfig((Display*) win->src.display, bestFbc);
		
		XFree(fbc);
		
		if (args & RGFW_TRANSPARENT_WINDOW) {
			XMatchVisualInfo((Display*) win->src.display, DefaultScreen((Display*) win->src.display), 32, TrueColor, vi); /*!< for RGBA backgrounds*/
		}
		
#else
		XVisualInfo viNorm;

		viNorm.visual = DefaultVisual((Display*) win->src.display, DefaultScreen((Display*) win->src.display));
		
		viNorm.depth = 0;
		XVisualInfo* vi = &viNorm;
		
		XMatchVisualInfo((Display*) win->src.display, DefaultScreen((Display*) win->src.display), 32, TrueColor, vi); /*!< for RGBA backgrounds*/
#endif
		/* make X window attrubutes*/
		XSetWindowAttributes swa;
		Colormap cmap;

		swa.colormap = cmap = XCreateColormap((Display*) win->src.display,
			DefaultRootWindow(win->src.display),
			vi->visual, AllocNone);

		swa.background_pixmap = None;
		swa.border_pixel = 0;
		swa.event_mask = event_mask;
		
		swa.background_pixel = 0;

		/* create the window*/
		win->src.window = XCreateWindow((Display*) win->src.display, DefaultRootWindow((Display*) win->src.display), win->r.x, win->r.y, win->r.w, win->r.h,
			0, vi->depth, InputOutput, vi->visual,
			CWColormap | CWBorderPixel | CWBackPixel | CWEventMask, &swa);

		XFreeColors((Display*) win->src.display, cmap, NULL, 0, 0);

		#ifdef RGFW_OPENGL
		XFree(vi);
		#endif

		// In your .desktop app, if you set the property
		// StartupWMClass=RGFW that will assoicate the launcher icon
		// with your application - robrohan 
		XClassHint *hint = XAllocClassHint();
		assert(hint != NULL);
		hint->res_class = (char*)"RGFW";
		hint->res_name = (char*)name; // just use the window name as the app name
		XSetClassHint((Display*) win->src.display, win->src.window, hint);
		XFree(hint);

		if ((args & RGFW_NO_INIT_API) == 0) {
#ifdef RGFW_OPENGL /* This is the second part of setting up opengl. This is where we ask OpenGL for a specific version. */ 
		i32 context_attribs[7] = { 0, 0, 0, 0, 0, 0, 0 };
		context_attribs[0] = GLX_CONTEXT_PROFILE_MASK_ARB;
		if (RGFW_profile == RGFW_GL_CORE) 
			context_attribs[1] = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
		else 
			context_attribs[1] = GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
		
		if (RGFW_majorVersion || RGFW_minorVersion) {
			context_attribs[2] = GLX_CONTEXT_MAJOR_VERSION_ARB;
			context_attribs[3] = RGFW_majorVersion;
			context_attribs[4] = GLX_CONTEXT_MINOR_VERSION_ARB;
			context_attribs[5] = RGFW_minorVersion;
		}

		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
		glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
			glXGetProcAddressARB((GLubyte*) "glXCreateContextAttribsARB");

		GLXContext ctx = NULL;

		if (RGFW_root != NULL)
			ctx = RGFW_root->src.ctx;

		win->src.ctx = glXCreateContextAttribsARB((Display*) win->src.display, bestFbc, ctx, True, context_attribs);
#endif
		if (RGFW_root == NULL)
			RGFW_root = win;

		RGFW_init_buffer(win, vi);
		}
		

		#ifndef RGFW_NO_MONITOR
		if (args & RGFW_SCALE_TO_MONITOR)
			RGFW_window_scaleToMonitor(win);
		#endif

		if (args & RGFW_NO_RESIZE) { /* make it so the user can't resize the window*/
			XSizeHints* sh = XAllocSizeHints();
			sh->flags = (1L << 4) | (1L << 5);
			sh->min_width = sh->max_width = win->r.w;
			sh->min_height = sh->max_height = win->r.h;

			XSetWMSizeHints((Display*) win->src.display, (Drawable) win->src.window, sh, XA_WM_NORMAL_HINTS);
			XFree(sh);
		}

		if (args & RGFW_NO_BORDER) {
			RGFW_window_setBorder(win, 0);
		}

		XSelectInput((Display*) win->src.display, (Drawable) win->src.window, event_mask); /*!< tell X11 what events we want*/

		/* make it so the user can't close the window until the program does*/
		if (wm_delete_window == 0)
			wm_delete_window = XInternAtom((Display*) win->src.display, "WM_DELETE_WINDOW", False);

		XSetWMProtocols((Display*) win->src.display, (Drawable) win->src.window, &wm_delete_window, 1);

		/* connect the context to the window*/
#ifdef RGFW_OPENGL
		if ((args & RGFW_NO_INIT_API) == 0)
			glXMakeCurrent((Display*) win->src.display, (Drawable) win->src.window, (GLXContext) win->src.ctx);
#endif

		/* set the background*/
		XStoreName((Display*) win->src.display, (Drawable) win->src.window, name); /*!< set the name*/

		XMapWindow((Display*) win->src.display, (Drawable) win->src.window);						  /* draw the window*/
		XMoveWindow((Display*) win->src.display, (Drawable) win->src.window, win->r.x, win->r.y); /*!< move the window to it's proper cords*/

		if (args & RGFW_ALLOW_DND) { /* init drag and drop atoms and turn on drag and drop for this window */
			win->_winArgs |= RGFW_ALLOW_DND;

			XdndAware = XInternAtom((Display*) win->src.display, "XdndAware", False);
			XdndTypeList = XInternAtom((Display*) win->src.display, "XdndTypeList", False);
			XdndSelection = XInternAtom((Display*) win->src.display, "XdndSelection", False);

			/* client messages */
			XdndEnter = XInternAtom((Display*) win->src.display, "XdndEnter", False);
			XdndPosition = XInternAtom((Display*) win->src.display, "XdndPosition", False);
			XdndStatus = XInternAtom((Display*) win->src.display, "XdndStatus", False);
			XdndLeave = XInternAtom((Display*) win->src.display, "XdndLeave", False);
			XdndDrop = XInternAtom((Display*) win->src.display, "XdndDrop", False);
			XdndFinished = XInternAtom((Display*) win->src.display, "XdndFinished", False);

			/* actions */
			XdndActionCopy = XInternAtom((Display*) win->src.display, "XdndActionCopy", False);
			XdndActionMove = XInternAtom((Display*) win->src.display, "XdndActionMove", False);
			XdndActionLink = XInternAtom((Display*) win->src.display, "XdndActionLink", False);
			XdndActionAsk = XInternAtom((Display*) win->src.display, "XdndActionAsk", False);
			XdndActionPrivate = XInternAtom((Display*) win->src.display, "XdndActionPrivate", False);
			const Atom version = 5;

			XChangeProperty((Display*) win->src.display, (Window) win->src.window,
				XdndAware, 4, 32,
				PropModeReplace, (u8*) &version, 1); /*!< turns on drag and drop */
		}

		#ifdef RGFW_EGL
			if ((args & RGFW_NO_INIT_API) == 0)
				RGFW_createOpenGLContext(win);
		#endif

		RGFW_window_setMouseDefault(win);

		RGFW_windowsOpen++;

		return win; /*return newly created window*/
	}

	RGFW_area RGFW_getScreenSize(void) {
		assert(RGFW_root != NULL);

		Screen* scrn = DefaultScreenOfDisplay((Display*) RGFW_root->src.display);
		return RGFW_AREA(scrn->width, scrn->height);
	}

	RGFW_point RGFW_getGlobalMousePoint(void) {
		assert(RGFW_root != NULL);

		RGFW_point RGFWMouse;

		i32 x, y;
		u32 z;
		Window window1, window2;
		XQueryPointer((Display*) RGFW_root->src.display, XDefaultRootWindow((Display*) RGFW_root->src.display), &window1, &window2, &RGFWMouse.x, &RGFWMouse.y, &x, &y, &z);
 
		return RGFWMouse;
	}

	RGFW_point RGFW_window_getMousePoint(RGFW_window* win) {
		assert(win != NULL);

		RGFW_point RGFWMouse;

		i32 x, y;
		u32 z;
		Window window1, window2;
		XQueryPointer((Display*) win->src.display, win->src.window, &window1, &window2, &x, &y, &RGFWMouse.x, &RGFWMouse.y, &z);

		return RGFWMouse;
	}

	typedef struct XDND {
		long source, version;
		i32 format;
	} XDND; /*!< data structure for xdnd events */
	XDND xdnd;

	int xAxis = 0, yAxis = 0;

	RGFW_Event* RGFW_window_checkEvent(RGFW_window* win) {
		assert(win != NULL);

		if (win->event.type == 0) 
			RGFW_resetKey();

		if (win->event.type == RGFW_quit) {
			return NULL;
		}

		win->event.type = 0;

#ifdef __linux__
	RGFW_Event* event = RGFW_linux_updateJoystick(win);
	if (event != NULL)
		return event;
#endif

		XPending(win->src.display);

		XEvent E; /*!< raw X11 event */

		/* if there is no unread qued events, get a new one */
		if ((QLength(win->src.display) || XEventsQueued((Display*) win->src.display, QueuedAlready) + XEventsQueued((Display*) win->src.display, QueuedAfterReading)) 
			&& win->event.type != RGFW_quit
		)
			XNextEvent((Display*) win->src.display, &E);
		else {
			return NULL;
		}

		u32 i;
		win->event.type = 0;


		switch (E.type) {
		case KeyPress:
		case KeyRelease: {
			win->event.repeat = RGFW_FALSE;
			/* check if it's a real key release */
			if (E.type == KeyRelease && XEventsQueued((Display*) win->src.display, QueuedAfterReading)) { /* get next event if there is one*/
				XEvent NE;
				XPeekEvent((Display*) win->src.display, &NE);

				if (E.xkey.time == NE.xkey.time && E.xkey.keycode == NE.xkey.keycode) /* check if the current and next are both the same*/
					win->event.repeat = RGFW_TRUE;
			}

			/* set event key data */
			KeySym sym = (KeySym)XkbKeycodeToKeysym((Display*) win->src.display, E.xkey.keycode, 0, E.xkey.state & ShiftMask ? 1 : 0);
			win->event.keyCode = RGFW_apiKeyCodeToRGFW(E.xkey.keycode);
			
			char* str = (char*)XKeysymToString(sym);
			if (str != NULL)
				strncpy(win->event.keyName, str, 16);

			win->event.keyName[15] = '\0';		

			RGFW_keyboard[win->event.keyCode].prev = RGFW_isPressed(win, win->event.keyCode);
			
			/* get keystate data */
			win->event.type = (E.type == KeyPress) ? RGFW_keyPressed : RGFW_keyReleased;

			XKeyboardState keystate;
			XGetKeyboardControl((Display*) win->src.display, &keystate);

			RGFW_updateLockState(win, (keystate.led_mask & 1), (keystate.led_mask & 2));
			RGFW_keyboard[win->event.keyCode].current = (E.type == KeyPress);
			RGFW_keyCallback(win, win->event.keyCode, win->event.keyName, win->event.lockState, (E.type == KeyPress));
			break;
		}
		case ButtonPress:
		case ButtonRelease:
			win->event.type = RGFW_mouseButtonPressed + (E.type == ButtonRelease); // the events match 
			
			switch(win->event.button) {
				case RGFW_mouseScrollUp:
					win->event.scroll = 1;
					break;
				case RGFW_mouseScrollDown:
					win->event.scroll = -1;
					break;
				default: break;
			}

			win->event.button = E.xbutton.button;
			RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;

			if (win->event.repeat == RGFW_FALSE)
				win->event.repeat = RGFW_isPressed(win, win->event.keyCode);

			RGFW_mouseButtons[win->event.button].current = (E.type == ButtonPress);
			RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, (E.type == ButtonPress));
			break;

		case MotionNotify:	
			win->event.point.x = E.xmotion.x;
			win->event.point.y = E.xmotion.y;
			
			if ((win->_winArgs & RGFW_HOLD_MOUSE)) {
				win->event.point.y = E.xmotion.y;

				win->event.point.x = win->_lastMousePoint.x - abs(win->event.point.x);
				win->event.point.y = win->_lastMousePoint.y - abs(win->event.point.y);
			}

			win->_lastMousePoint = RGFW_POINT(E.xmotion.x, E.xmotion.y);

			win->event.type = RGFW_mousePosChanged;
			RGFW_mousePosCallback(win, win->event.point);
			break;

		case GenericEvent: {
			/* MotionNotify is used for mouse events if the mouse isn't held */                
			if (!(win->_winArgs & RGFW_HOLD_MOUSE)) {
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

				win->event.point = RGFW_POINT((i32)deltaX, (i32)deltaY);
				
				RGFW_window_moveMouse(win, RGFW_POINT(win->r.x + (win->r.w / 2), win->r.y + (win->r.h / 2)));

				win->event.type = RGFW_mousePosChanged;
				RGFW_mousePosCallback(win, win->event.point);
            }

            XFreeEventData(win->src.display, &E.xcookie);
			break;
		}
		
		case Expose:
			win->event.type = RGFW_windowRefresh;
			RGFW_windowRefreshCallback(win);
			break;

		case ClientMessage:
			/* if the client closed the window*/
			if (E.xclient.data.l[0] == (i64) wm_delete_window) {
				win->event.type = RGFW_quit;
				RGFW_windowQuitCallback(win);
				break;
			}
			
			/* reset DND values */
			if (win->event.droppedFilesCount) {
				for (i = 0; i < win->event.droppedFilesCount; i++)
					win->event.droppedFiles[i][0] = '\0';
			}

			win->event.droppedFilesCount = 0;

			/*
				much of this event (drag and drop code) is source from glfw
			*/

			if ((win->_winArgs & RGFW_ALLOW_DND) == 0)
				break;

			if (E.xclient.message_type == XdndEnter) {
				unsigned long count;
				Atom* formats;
				Atom real_formats[6];

				Bool list = E.xclient.data.l[1] & 1;

				xdnd.source = E.xclient.data.l[0];
				xdnd.version = E.xclient.data.l[1] >> 24;
				xdnd.format = None;

				if (xdnd.version > 5)
					break;

				if (list) {
					Atom actualType;
					i32 actualFormat;
					unsigned long bytesAfter;

					XGetWindowProperty((Display*) win->src.display,
						xdnd.source,
						XdndTypeList,
						0,
						LONG_MAX,
						False,
						4,
						&actualType,
						&actualFormat,
						&count,
						&bytesAfter,
						(u8**) &formats);
				} else {
					count = 0;

					if (E.xclient.data.l[2] != None)
						real_formats[count++] = E.xclient.data.l[2];
					if (E.xclient.data.l[3] != None)
						real_formats[count++] = E.xclient.data.l[3];
					if (E.xclient.data.l[4] != None)
						real_formats[count++] = E.xclient.data.l[4];
					
					formats = real_formats;
				}

				u32 i;
				for (i = 0; i < (u32)count; i++) {
					char* name = XGetAtomName((Display*) win->src.display, formats[i]);

					char* links[2] = { (char*) (const char*) "text/uri-list", (char*) (const char*) "text/plain" };
					for (; 1; name++) {
						u32 j;
						for (j = 0; j < 2; j++) {
							if (*links[j] != *name) {
								links[j] = (char*) (const char*) "\1";
								continue;
							}

							if (*links[j] == '\0' && *name == '\0')
								xdnd.format = formats[i];

							if (*links[j] != '\0' && *links[j] != '\1')
								links[j]++;
						}

						if (*name == '\0')
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

				if (xdnd.version > 5)
					break;

				XTranslateCoordinates((Display*) win->src.display,
					XDefaultRootWindow((Display*) win->src.display),
					(Window) win->src.window,
					xabs, yabs,
					&xpos, &ypos,
					&dummy);

				win->event.point.x = xpos;
				win->event.point.y = ypos;

				XEvent reply = { ClientMessage };
				reply.xclient.window = xdnd.source;
				reply.xclient.message_type = XdndStatus;
				reply.xclient.format = 32;
				reply.xclient.data.l[0] = (long) win->src.window;
				reply.xclient.data.l[2] = 0;
				reply.xclient.data.l[3] = 0;

				if (xdnd.format) {
					reply.xclient.data.l[1] = 1;
					if (xdnd.version >= 2)
						reply.xclient.data.l[4] = XdndActionCopy;
				}

				XSendEvent((Display*) win->src.display, xdnd.source, False, NoEventMask, &reply);
				XFlush((Display*) win->src.display);
				break;
			}

			if (E.xclient.message_type != XdndDrop)
				break;

			if (xdnd.version > 5)
				break;

			win->event.type = RGFW_dnd_init;

			if (xdnd.format) {
				Time time = CurrentTime;

				if (xdnd.version >= 1)
					time = E.xclient.data.l[2];

				XConvertSelection((Display*) win->src.display,
					XdndSelection,
					xdnd.format,
					XdndSelection,
					(Window) win->src.window,
					time);
			} else if (xdnd.version >= 2) {
				XEvent reply = { ClientMessage };
				reply.xclient.window = xdnd.source;
				reply.xclient.message_type = XdndFinished;
				reply.xclient.format = 32;
				reply.xclient.data.l[0] = (long) win->src.window;
				reply.xclient.data.l[1] = 0;
				reply.xclient.data.l[2] = None;

				XSendEvent((Display*) win->src.display, xdnd.source,
					False, NoEventMask, &reply);
				XFlush((Display*) win->src.display);
			}

			RGFW_dndInitCallback(win, win->event.point);
			break;
		case SelectionNotify: {
			/* this is only for checking for xdnd drops */
			if (E.xselection.property != XdndSelection || !(win->_winArgs | RGFW_ALLOW_DND))
				break;

			char* data;
			unsigned long result;

			Atom actualType;
			i32 actualFormat;
			unsigned long bytesAfter;

			XGetWindowProperty((Display*) win->src.display, E.xselection.requestor, E.xselection.property, 0, LONG_MAX, False, E.xselection.target, &actualType, &actualFormat, &result, &bytesAfter, (u8**) &data);

			if (result == 0)
				break;

			/*
			SOURCED FROM GLFW _glfwParseUriList
			Copyright (c) 2002-2006 Marcus Geelnard
			Copyright (c) 2006-2019 Camilla Löwy
			*/

			const char* prefix = (const char*)"file://";

			char* line;

			win->event.droppedFilesCount = 0;

			win->event.type = RGFW_dnd;

			while ((line = strtok(data, "\r\n"))) {
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
						path[index] = (char) strtol(digits, NULL, 16);
						line += 2;
					} else
						path[index] = *line;

					index++;
					line++;
				}
				path[index] = '\0';
				strncpy(win->event.droppedFiles[win->event.droppedFilesCount - 1], path, index + 1);
			}

			if (data)
				XFree(data);

			if (xdnd.version >= 2) {
				XEvent reply = { ClientMessage };
				reply.xclient.window = xdnd.source;
				reply.xclient.message_type = XdndFinished;
				reply.xclient.format = 32;
				reply.xclient.data.l[0] = (long) win->src.window;
				reply.xclient.data.l[1] = result;
				reply.xclient.data.l[2] = XdndActionCopy;

				XSendEvent((Display*) win->src.display, xdnd.source, False, NoEventMask, &reply);
				XFlush((Display*) win->src.display);
			}

			RGFW_dndCallback(win, win->event.droppedFiles, win->event.droppedFilesCount);
			break;
		}
		case FocusIn:
			win->event.inFocus = 1;
			win->event.type = RGFW_focusIn;
			RGFW_focusCallback(win, 1);
			break;

			break;
		case FocusOut:
			win->event.inFocus = 0;
			win->event.type = RGFW_focusOut;
			RGFW_focusCallback(win, 0);
			break;
		
		case EnterNotify: {
			win->event.type = RGFW_mouseEnter;
			win->event.point.x = E.xcrossing.x;
			win->event.point.y = E.xcrossing.y;
			RGFW_mouseNotifyCallBack(win, win->event.point, 1);
			break;
		}

		case LeaveNotify: {
			win->event.type = RGFW_mouseLeave;
			RGFW_mouseNotifyCallBack(win, win->event.point, 0);
			break;
		}

		case ConfigureNotify: {
				/* detect resize */
      			if (E.xconfigure.width != win->r.w || E.xconfigure.height != win->r.h) {
					win->event.type = RGFW_windowResized;
					win->r = RGFW_RECT(win->r.x, win->r.y, E.xconfigure.width, E.xconfigure.height);
					RGFW_windowResizeCallback(win, win->r);
					break;
      			}  
      
      			/* detect move */
      			if (E.xconfigure.x != win->r.x || E.xconfigure.y != win->r.y) {
					win->event.type = RGFW_windowMoved;
					win->r = RGFW_RECT(E.xconfigure.x, E.xconfigure.y, win->r.w, win->r.h);
					RGFW_windowMoveCallback(win, win->r);
					break;
				} 

				break;
		}
		default: {
			break;
		}
		}

		XFlush((Display*) win->src.display);

		if (win->event.type)
			return &win->event;
		else
			return NULL;
	}

	void RGFW_window_move(RGFW_window* win, RGFW_point v) {
		assert(win != NULL);
		win->r.x = v.x;
		win->r.y = v.y;

		XMoveWindow((Display*) win->src.display, (Window) win->src.window, v.x, v.y);
	}


	void RGFW_window_resize(RGFW_window* win, RGFW_area a) {
		assert(win != NULL);
		win->r.w = a.w;
		win->r.h = a.h;

		XResizeWindow((Display*) win->src.display, (Window) win->src.window, a.w, a.h);
	}

	void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a) {
		assert(win != NULL);

		if (a.w == 0 && a.h == 0)
			return;

		XSizeHints hints;
		long flags;

		XGetWMNormalHints(win->src.display, (Window) win->src.window, &hints, &flags);

		hints.flags |= PMinSize;
		
		hints.min_width = a.w;
		hints.min_height = a.h;

		XSetWMNormalHints(win->src.display, (Window) win->src.window, &hints);
	}

	void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a) {
		assert(win != NULL);

		if (a.w == 0 && a.h == 0)
			return;

		XSizeHints hints;
		long flags;

		XGetWMNormalHints(win->src.display, (Window) win->src.window, &hints, &flags);

		hints.flags |= PMaxSize;

		hints.max_width = a.w;
		hints.max_height = a.h;

		XSetWMNormalHints(win->src.display, (Window) win->src.window, &hints);
	}


	void RGFW_window_minimize(RGFW_window* win) {
		assert(win != NULL);

		XIconifyWindow(win->src.display, (Window) win->src.window, DefaultScreen(win->src.display));
		XFlush(win->src.display);
	}

	void RGFW_window_restore(RGFW_window* win) {
		assert(win != NULL);

		XMapWindow(win->src.display, (Window) win->src.window);
		XFlush(win->src.display);
	}	

	void RGFW_window_setName(RGFW_window* win, char* name) {
		assert(win != NULL);

		XStoreName((Display*) win->src.display, (Window) win->src.window, name);
	}
	
	void* RGFW_libxshape = NULL;

	#ifndef RGFW_NO_PASSTHROUGH
	void RGFW_window_setMousePassthrough(RGFW_window* win, b8 passthrough) {
		assert(win != NULL);
		
		#if defined(__CYGWIN__)
			RGFW_libxshape = dlopen("libXext-6.so", RTLD_LAZY | RTLD_LOCAL);
		#elif defined(__OpenBSD__) || defined(__NetBSD__)
			RGFW_libxshape = dlopen("libXext.so", RTLD_LAZY | RTLD_LOCAL);
		#else
    		RGFW_libxshape = dlopen("libXext.so.6", RTLD_LAZY | RTLD_LOCAL);
		#endif
		
		typedef void (* PFN_XShapeCombineMask)(Display*,Window,int,int,int,Pixmap,int);
		static PFN_XShapeCombineMask XShapeCombineMask;
		
		typedef void (* PFN_XShapeCombineRegion)(Display*,Window,int,int,int,Region,int);
		static PFN_XShapeCombineRegion XShapeCombineRegion;
		
		if (XShapeCombineMask != NULL)
			XShapeCombineMask = (PFN_XShapeCombineMask) dlsym(RGFW_libxshape, "XShapeCombineMask");

		if (XShapeCombineRegion != NULL)
			XShapeCombineRegion = (PFN_XShapeCombineRegion) dlsym(RGFW_libxshape, "XShapeCombineMask");

		if (passthrough) {
			Region region = XCreateRegion();
			XShapeCombineRegion(win->src.display, win->src.window, ShapeInput, 0, 0, region, ShapeSet);
			XDestroyRegion(region);

			return;
		}

		XShapeCombineMask(win->src.display, win->src.window, ShapeInput, 0, 0, None, ShapeSet);
	}
	#endif

	/*
		the majority function is sourced from GLFW
	*/

	void RGFW_window_setIcon(RGFW_window* win, u8* icon, RGFW_area a, i32 channels) {
		assert(win != NULL);

		i32 longCount = 2 + a.w * a.h;

		u64* X11Icon = (u64*) RGFW_MALLOC(longCount * sizeof(u64));
		u64* target = X11Icon;

		*target++ = a.w;
		*target++ = a.h;

		u32 i;

		for (i = 0; i < a.w * a.h; i++) {
			if (channels == 3)
				*target++ = ((icon[i * 3 + 0]) << 16) |
				((icon[i * 3 + 1]) << 8) |
				((icon[i * 3 + 2]) << 0) |
				(0xFF << 24);

			else if (channels == 4)
				*target++ = ((icon[i * 4 + 0]) << 16) |
				((icon[i * 4 + 1]) << 8) |
				((icon[i * 4 + 2]) << 0) |
				((icon[i * 4 + 3]) << 24);
		}

		static Atom NET_WM_ICON = 0;
		if (NET_WM_ICON == 0)
			NET_WM_ICON = XInternAtom((Display*) win->src.display, "_NET_WM_ICON", False);

		XChangeProperty((Display*) win->src.display, (Window) win->src.window,
			NET_WM_ICON,
			6, 32,
			PropModeReplace,
			(u8*) X11Icon,
			longCount);

		RGFW_FREE(X11Icon);

		XFlush((Display*) win->src.display);
	}

	void RGFW_window_setMouse(RGFW_window* win, u8* image, RGFW_area a, i32 channels) {
		assert(win != NULL);

#ifndef RGFW_NO_X11_CURSOR
		XcursorImage* native = XcursorImageCreate(a.w, a.h);
		native->xhot = 0;
		native->yhot = 0;

		u8* source = (u8*) image;
		XcursorPixel* target = native->pixels;

		u32 i;
		for (i = 0; i < a.w * a.h; i++, target++, source += 4) {
			u8 alpha = 0xFF;
			if (channels == 4)
				alpha = source[3];

			*target = (alpha << 24) | (((source[0] * alpha) / 255) << 16) | (((source[1] * alpha) / 255) << 8) | (((source[2] * alpha) / 255) << 0);
		}

		Cursor cursor = XcursorImageLoadCursor((Display*) win->src.display, native);
		XDefineCursor((Display*) win->src.display, (Window) win->src.window, (Cursor) cursor);

		XFreeCursor((Display*) win->src.display, (Cursor) cursor);
		XcursorImageDestroy(native);
#else
	RGFW_UNUSED(image) RGFW_UNUSED(a.w) RGFW_UNUSED(channels)
#endif
	}

	void RGFW_window_moveMouse(RGFW_window* win, RGFW_point v) {
		assert(win != NULL);

		XEvent event;
		XQueryPointer(win->src.display, DefaultRootWindow(win->src.display),
			&event.xbutton.root, &event.xbutton.window,
			&event.xbutton.x_root, &event.xbutton.y_root,
			&event.xbutton.x, &event.xbutton.y,
			&event.xbutton.state);

		if (event.xbutton.x == v.x && event.xbutton.y == v.y)
			return;

		XWarpPointer(win->src.display, None, win->src.window, 0, 0, 0, 0, (int) v.x - win->r.x, (int) v.y - win->r.y);
	}

	RGFWDEF void RGFW_window_disableMouse(RGFW_window* win) {
		RGFW_UNUSED(win);
	}

	void RGFW_window_setMouseDefault(RGFW_window* win) {
		RGFW_window_setMouseStandard(win, RGFW_MOUSE_ARROW);
	}

	void RGFW_window_setMouseStandard(RGFW_window* win, u8 mouse) {
		assert(win != NULL);
		 
		if (mouse > (sizeof(RGFW_mouseIconSrc) / sizeof(u8)))
			return;
		
		mouse = RGFW_mouseIconSrc[mouse];

		Cursor cursor = XCreateFontCursor((Display*) win->src.display, mouse);
		XDefineCursor((Display*) win->src.display, (Window) win->src.window, (Cursor) cursor);

		XFreeCursor((Display*) win->src.display, (Cursor) cursor);
	}

	void RGFW_window_hide(RGFW_window* win) {
		XMapWindow(win->src.display, win->src.window);
	}

	void RGFW_window_show(RGFW_window* win) {
		XUnmapWindow(win->src.display, win->src.window);
	}

	/*
		the majority function is sourced from GLFW
	*/
	char* RGFW_readClipboard(size_t* size) {
		static Atom UTF8 = 0;
		if (UTF8 == 0)
			UTF8 = XInternAtom(RGFW_root->src.display, "UTF8_STRING", True);

		XEvent event;
		int format;
		unsigned long N, sizeN;
		char* data, * s = NULL;
		Atom target;
		Atom CLIPBOARD = 0, XSEL_DATA = 0;

		if (CLIPBOARD == 0) {
			CLIPBOARD = XInternAtom(RGFW_root->src.display, "CLIPBOARD", 0);
			XSEL_DATA = XInternAtom(RGFW_root->src.display, "XSEL_DATA", 0);
		}

		XConvertSelection(RGFW_root->src.display, CLIPBOARD, UTF8, XSEL_DATA, RGFW_root->src.window, CurrentTime);
		XSync(RGFW_root->src.display, 0);
		XNextEvent(RGFW_root->src.display, &event);

		if (event.type != SelectionNotify || event.xselection.selection != CLIPBOARD || event.xselection.property == 0)
			return NULL;

		XGetWindowProperty(event.xselection.display, event.xselection.requestor,
			event.xselection.property, 0L, (~0L), 0, AnyPropertyType, &target,
			&format, &sizeN, &N, (unsigned char**) &data);

		if (target == UTF8 || target == XA_STRING) {
			s = (char*)RGFW_MALLOC(sizeof(char) * sizeN);
			strncpy(s, data, sizeN);
			s[sizeN] = '\0';
			XFree(data);
		}

		XDeleteProperty(event.xselection.display, event.xselection.requestor, event.xselection.property);

		if (s != NULL && size != NULL)
			*size = sizeN;

		return s;
		}

	/*
		almost all of this function is sourced from GLFW
	*/
	void RGFW_writeClipboard(const char* text, u32 textLen) {
		static Atom CLIPBOARD = 0,
			UTF8_STRING = 0,
			SAVE_TARGETS = 0,
			TARGETS = 0,
			MULTIPLE = 0,
			ATOM_PAIR = 0,
			CLIPBOARD_MANAGER = 0;

		if (CLIPBOARD == 0) {
			CLIPBOARD = XInternAtom((Display*) RGFW_root->src.display, "CLIPBOARD", False);
			UTF8_STRING = XInternAtom((Display*) RGFW_root->src.display, "UTF8_STRING", False);
			SAVE_TARGETS = XInternAtom((Display*) RGFW_root->src.display, "SAVE_TARGETS", False);
			TARGETS = XInternAtom((Display*) RGFW_root->src.display, "TARGETS", False);
			MULTIPLE = XInternAtom((Display*) RGFW_root->src.display, "MULTIPLE", False);
			ATOM_PAIR = XInternAtom((Display*) RGFW_root->src.display, "ATOM_PAIR", False);
			CLIPBOARD_MANAGER = XInternAtom((Display*) RGFW_root->src.display, "CLIPBOARD_MANAGER", False);
		}
		
		XSetSelectionOwner((Display*) RGFW_root->src.display, CLIPBOARD, (Window) RGFW_root->src.window, CurrentTime);

		XConvertSelection((Display*) RGFW_root->src.display, CLIPBOARD_MANAGER, SAVE_TARGETS, None, (Window) RGFW_root->src.window, CurrentTime);
		for (;;) {
			XEvent event;

			XNextEvent((Display*) RGFW_root->src.display, &event);
			if (event.type != SelectionRequest) {
				break;
			}

			const XSelectionRequestEvent* request = &event.xselectionrequest;

			XEvent reply = { SelectionNotify };
			reply.xselection.property = 0;

			const Atom formats[] = { UTF8_STRING, XA_STRING };
			const i32 formatCount = sizeof(formats) / sizeof(formats[0]);
		
			if (request->target == TARGETS) {
				const Atom targets[] = { TARGETS,
										MULTIPLE,
										UTF8_STRING,
										XA_STRING };

				XChangeProperty((Display*) RGFW_root->src.display,
					request->requestor,
					request->property,
					4,
					32,
					PropModeReplace,
					(u8*) targets,
					sizeof(targets) / sizeof(targets[0]));

				reply.xselection.property = request->property;
			}

			if (request->target == MULTIPLE) {
				Atom* targets = NULL;

				Atom actualType = 0;
				int actualFormat = 0;
				unsigned long count = 0, bytesAfter = 0;

				XGetWindowProperty((Display*) RGFW_root->src.display, request->requestor, request->property, 0, LONG_MAX, False, ATOM_PAIR, &actualType, &actualFormat, &count, &bytesAfter, (u8**) &targets);

				unsigned long i;
				for (i = 0; i < (u32)count; i += 2) {
					i32 j;

					for (j = 0; j < formatCount; j++) {
						if (targets[i] == formats[j])
							break;
					}

					if (j < formatCount)
					{
						XChangeProperty((Display*) RGFW_root->src.display,
							request->requestor,
							targets[i + 1],
							targets[i],
							8,
							PropModeReplace,
							(u8*) text,
							textLen);
						XFlush(RGFW_root->src.display);
					} else {
						targets[i + 1] = None;
					}
				}

				XChangeProperty((Display*) RGFW_root->src.display,
					request->requestor,
					request->property,
					ATOM_PAIR,
					32,
					PropModeReplace,
					(u8*) targets,
					count);

				XFlush(RGFW_root->src.display);
				XFree(targets);

				reply.xselection.property = request->property;
			}

			reply.xselection.display = request->display;
			reply.xselection.requestor = request->requestor;
			reply.xselection.selection = request->selection;
			reply.xselection.target = request->target;
			reply.xselection.time = request->time;

			XSendEvent((Display*) RGFW_root->src.display, request->requestor, False, 0, &reply);
			XFlush(RGFW_root->src.display);
		}
	}

	u8 RGFW_window_isFullscreen(RGFW_window* win) {
		assert(win != NULL);

		XWindowAttributes windowAttributes;
		XGetWindowAttributes(win->src.display, (Window) win->src.window, &windowAttributes);

		/* check if the window is visable */
		if (windowAttributes.map_state != IsViewable)
			return 0;

		/* check if the window covers the full screen */
		return (windowAttributes.x == 0 && windowAttributes.y == 0 &&
			windowAttributes.width == XDisplayWidth(win->src.display, DefaultScreen(win->src.display)) &&
			windowAttributes.height == XDisplayHeight(win->src.display, DefaultScreen(win->src.display)));
	}

	u8 RGFW_window_isHidden(RGFW_window* win) {
		assert(win != NULL);

		XWindowAttributes windowAttributes;
		XGetWindowAttributes(win->src.display, (Window) win->src.window, &windowAttributes);

		return (windowAttributes.map_state == IsUnmapped && !RGFW_window_isMinimized(win));
	}

	u8 RGFW_window_isMinimized(RGFW_window* win) {
		assert(win != NULL);

		static Atom prop = 0;
		if (prop == 0)
			prop = XInternAtom(win->src.display, "WM_STATE", False);

		Atom actual_type;
		i32 actual_format;
		unsigned long nitems, bytes_after;
		unsigned char* prop_data;

		i16 status = XGetWindowProperty(win->src.display, (Window) win->src.window, prop, 0, 2, False,
			AnyPropertyType, &actual_type, &actual_format,
			&nitems, &bytes_after, &prop_data);

		if (status == Success && nitems >= 1 && *((int*) prop_data) == IconicState) {
			XFree(prop_data);
			return 1;
		}

		if (prop_data != NULL)
			XFree(prop_data);

		return 0;
	}

	u8 RGFW_window_isMaximized(RGFW_window* win) {
		assert(win != NULL);

		static Atom net_wm_state = 0;
		static Atom net_wm_state_maximized_horz = 0;
		static Atom net_wm_state_maximized_vert = 0;

		if (net_wm_state == 0) {
			net_wm_state = XInternAtom(win->src.display, "_NET_WM_STATE", False);
			net_wm_state_maximized_vert = XInternAtom(win->src.display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
			net_wm_state_maximized_horz = XInternAtom(win->src.display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		}

		Atom actual_type;
		i32 actual_format;
		unsigned long nitems, bytes_after;
		unsigned char* prop_data;

		i16 status = XGetWindowProperty(win->src.display, (Window) win->src.window, net_wm_state, 0, 1024, False,
			XA_ATOM, &actual_type, &actual_format,
			&nitems, &bytes_after, &prop_data);

		if (status != Success) {
			if (prop_data != NULL)
				XFree(prop_data);

			return 0;
		}

		Atom* atoms = (Atom*) prop_data;
		u64 i;
		for (i = 0; i < nitems; ++i) {
			if (atoms[i] == net_wm_state_maximized_horz ||
				atoms[i] == net_wm_state_maximized_vert) {
				XFree(prop_data);
				return 1;
			}
		}

		return 0;
	}

	static void XGetSystemContentScale(Display* display, float* xscale, float* yscale) {
		float xdpi = 96.f, ydpi = 96.f;

#ifndef RGFW_NO_DPI
		char* rms = XResourceManagerString(display);
		XrmDatabase db = NULL;

		if (rms && db)
			db = XrmGetStringDatabase(rms);

		if (db == 0) {
			*xscale = xdpi / 96.f;
			*yscale = ydpi / 96.f;
			return;
		}

		XrmValue value;
		char* type = NULL;

		if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value) && type && strncmp(type, "String", 7) == 0)
			xdpi = ydpi = atof(value.addr);
		XrmDestroyDatabase(db);
#endif

		* xscale = xdpi / 96.f;
		*yscale = ydpi / 96.f;
	}

	RGFW_monitor RGFW_XCreateMonitor(i32 screen) {
		RGFW_monitor monitor;

		Display* display = XOpenDisplay(NULL);

		monitor.rect = RGFW_RECT(0, 0, DisplayWidth(display, screen), DisplayHeight(display, screen));
		monitor.physW = (monitor.rect.w * 25.4f / 96.f);
		monitor.physH = (monitor.rect.h * 25.4f / 96.f);

		strncpy(monitor.name, DisplayString(display), 128);

		XGetSystemContentScale(display, &monitor.scaleX, &monitor.scaleY);

		XRRScreenResources* sr = XRRGetScreenResourcesCurrent(display, RootWindow(display, screen));

		XRRCrtcInfo* ci = NULL;
		int crtc = 0;

		if (sr->ncrtc > crtc) {
			ci = XRRGetCrtcInfo(display, sr, sr->crtcs[crtc]);
		}

		if (ci == NULL) {
			XRRFreeScreenResources(sr);
			XCloseDisplay(display);
			return monitor;
		}

		monitor.rect.x = ci->x;
		monitor.rect.y = ci->y;

		XRRFreeCrtcInfo(ci);
		XRRFreeScreenResources(sr);

		XCloseDisplay(display);

		return monitor;
	}

	RGFW_monitor RGFW_monitors[6];
	RGFW_monitor* RGFW_getMonitors(void) {
		size_t i;
		for (i = 0; i < (size_t)ScreenCount(RGFW_root->src.display) && i < 6; i++)
			RGFW_monitors[i] = RGFW_XCreateMonitor(i);

		return RGFW_monitors;
	}

	RGFW_monitor RGFW_getPrimaryMonitor(void) {
		assert(RGFW_root != NULL);

		i32 primary = -1;
		Window root = DefaultRootWindow(RGFW_root->src.display);
		XRRScreenResources* res = XRRGetScreenResources(RGFW_root->src.display, root);

		for (int i = 0; i < res->noutput; i++) {
			XRROutputInfo* output_info = XRRGetOutputInfo(RGFW_root->src.display, res, res->outputs[i]);
			if (output_info->connection == RR_Connected && output_info->crtc) {
				XRRCrtcInfo* crtc_info = XRRGetCrtcInfo(RGFW_root->src.display, res, output_info->crtc);
				if (crtc_info->mode != None && crtc_info->x == 0 && crtc_info->y == 0) {
					primary = i;
					XRRFreeCrtcInfo(crtc_info);
					XRRFreeOutputInfo(output_info);
					break;
				}
				XRRFreeCrtcInfo(crtc_info);
			}
			XRRFreeOutputInfo(output_info);
		}

		XRRFreeScreenResources(res);

		return RGFW_XCreateMonitor(primary);
	}

	RGFW_monitor RGFW_window_getMonitor(RGFW_window* win) {
		return RGFW_XCreateMonitor(DefaultScreen(win->src.display));
	}

	#ifdef RGFW_OPENGL
	void RGFW_window_makeCurrent_OpenGL(RGFW_window* win) {
		if (win == NULL)
			glXMakeCurrent((Display*) NULL, (Drawable)NULL, (GLXContext) NULL);
		else
			glXMakeCurrent((Display*) win->src.display, (Drawable) win->src.window, (GLXContext) win->src.ctx);
	}
	#endif


	void RGFW_window_swapBuffers(RGFW_window* win) {
		assert(win != NULL);

		/* clear the window*/
		if (!(win->_winArgs & RGFW_NO_CPU_RENDER)) {
#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
			#ifdef RGFW_OSMESA
			RGFW_OSMesa_reorganize();
			#endif
			RGFW_area area = RGFW_bufferSize;

#ifndef RGFW_X11_DONT_CONVERT_BGR
			win->src.bitmap->data = (char*) win->buffer;
			u32 x, y;
			for (y = 0; y < (u32)win->r.h; y++) {
				for (x = 0; x < (u32)win->r.w; x++) {
					u32 index = (y * 4 * area.w) + x * 4;

					u8 red = win->src.bitmap->data[index];
					win->src.bitmap->data[index] = win->buffer[index + 2];
					win->src.bitmap->data[index + 2] = red;

				}
			}
#endif	
			XPutImage(win->src.display, (Window) win->src.window, win->src.gc, win->src.bitmap, 0, 0, 0, 0, RGFW_bufferSize.w, RGFW_bufferSize.h);
#endif
		}

		if (!(win->_winArgs & RGFW_NO_GPU_RENDER)) {
			#ifdef RGFW_EGL
					eglSwapBuffers(win->src.EGL_display, win->src.EGL_surface);
			#elif defined(RGFW_OPENGL)
					glXSwapBuffers((Display*) win->src.display, (Window) win->src.window);
			#endif
		}
	}

	#if !defined(RGFW_EGL)	
	void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) {
		assert(win != NULL);

		#if defined(RGFW_OPENGL)	
		((PFNGLXSWAPINTERVALEXTPROC) glXGetProcAddress((GLubyte*) "glXSwapIntervalEXT"))((Display*) win->src.display, (Window) win->src.window, swapInterval);
		#else
		RGFW_UNUSED(swapInterval);
		#endif
	}
	#endif


	void RGFW_window_close(RGFW_window* win) {
		/* ungrab pointer if it was grabbed */
		if (win->_winArgs & RGFW_HOLD_MOUSE) 
			XUngrabPointer(win->src.display, CurrentTime);
			
		assert(win != NULL);
#ifdef RGFW_EGL
		RGFW_closeEGL(win);
#endif

#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
		if (win->buffer != NULL) {
			XDestroyImage((XImage*) win->src.bitmap);
			XFreeGC(win->src.display, win->src.gc);
		}
#endif

		if ((Display*) win->src.display) {
#ifdef RGFW_OPENGL
			glXDestroyContext((Display*) win->src.display, win->src.ctx);
#endif

			if (win == RGFW_root)
				RGFW_root = NULL;

			if ((Drawable) win->src.window)
				XDestroyWindow((Display*) win->src.display, (Drawable) win->src.window); /*!< close the window*/
			
			XCloseDisplay((Display*) win->src.display); /*!< kill the display*/
		}

#ifdef RGFW_ALLOC_DROPFILES
		{
			u32 i;
			for (i = 0; i < RGFW_MAX_DROPS; i++)
				RGFW_FREE(win->event.droppedFiles[i]);


			RGFW_FREE(win->event.droppedFiles);
		}
#endif

		RGFW_windowsOpen--;
#if !defined(RGFW_NO_X11_CURSOR_PRELOAD) && !defined(RGFW_NO_X11_CURSOR)
		if (X11Cursorhandle != NULL && RGFW_windowsOpen <= 0) {
			dlclose(X11Cursorhandle);

			X11Cursorhandle = NULL;
		}
#endif
#if !defined(RGFW_NO_X11_XI_PRELOAD)
		if (X11Xihandle != NULL && RGFW_windowsOpen <= 0) {
			dlclose(X11Xihandle);

			X11Xihandle = NULL;
		}
#endif

		if (RGFW_libxshape != NULL && RGFW_windowsOpen <= 0) {
			dlclose(RGFW_libxshape);
			RGFW_libxshape = NULL;
		}

		if (RGFW_windowsOpen <= 0) {
			if (RGFW_eventWait_forceStop[0] || RGFW_eventWait_forceStop[1]){
				close(RGFW_eventWait_forceStop[0]);
				close(RGFW_eventWait_forceStop[1]);
			}

			u8 i;
			for (i = 0; i < RGFW_joystickCount; i++)
				close(RGFW_joysticks[i]);
		}

		/* set cleared display / window to NULL for error checking */
		win->src.display = (Display*) 0;
		win->src.window = (Window) 0;

		RGFW_FREE(win); /*!< free collected window data */
	}
	

/* 
	End of X11 linux / unix defines
*/

#endif /* RGFW_X11 */


/* wayland or X11 defines*/
#if defined(RGFW_WAYLAND) || defined(RGFW_X11)
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
	u16 RGFW_registerJoystickF(RGFW_window* win, char* file) {
		assert(win != NULL);

#ifdef __linux__

		i32 js = open(file, O_RDONLY);

		if (js && RGFW_joystickCount < 4) {
			RGFW_joystickCount++;

			RGFW_joysticks[RGFW_joystickCount - 1] = open(file, O_RDONLY);

			u8 i;
			for (i = 0; i < 16; i++)
				RGFW_jsPressed[RGFW_joystickCount - 1][i] = 0;

		}

		else {
#ifdef RGFW_PRINT_ERRORS
			RGFW_error = 1;
			fprintf(stderr, "Error RGFW_registerJoystickF : Cannot open file %s\n", file);
#endif
		}

		return RGFW_joystickCount - 1;
#endif
	}
	
	u16 RGFW_registerJoystick(RGFW_window* win, i32 jsNumber) {
		assert(win != NULL);

#ifdef __linux__
		char file[15];
		sprintf(file, "/dev/input/js%i", jsNumber);

		return RGFW_registerJoystickF(win, file);
#endif
	}
	
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
		if (waitMS == 0)
			return;
		
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
			{ wl_display_get_fd(win->src.display), POLLIN, 0 },
			#else
			{ ConnectionNumber(win->src.display), POLLIN, 0 },
			#endif
			{ RGFW_eventWait_forceStop[0], POLLIN, 0 },
			#ifdef __linux__ /* blank space for 4 joystick files*/
			{ -1, POLLIN, 0 }, {-1, POLLIN, 0 }, {-1, POLLIN, 0 },  {-1, POLLIN, 0} 
			#endif
		};

		u8 index = 2;
		
		#if defined(__linux__)
			for (i = 0; i < RGFW_joystickCount; i++) {
				if (RGFW_joysticks[i] == 0)
					continue;

				fds[index].fd = RGFW_joysticks[i];
				index++;
			}
		#endif


		u64 start = RGFW_getTimeNS();

		#ifdef RGFW_WAYLAND
		while (wl_display_dispatch(win->src.display) <= 0 && waitMS >= -1) {
		#else
		while (XPending(win->src.display) == 0 && waitMS >= -1) {
		#endif
			if (poll(fds, index, waitMS) <= 0)
				break;

			if (waitMS > 0) {
				waitMS -= (RGFW_getTimeNS() - start) / 1e+6;
			}
		}

		/* drain any data in the stop request */
		if (RGFW_eventWait_forceStop[2]) {	
			char data[64];
			(void)!read(RGFW_eventWait_forceStop[0], data, sizeof(data));
			
			RGFW_eventWait_forceStop[2] = 0;
		}
	}

	u64 RGFW_getTimeNS(void) { 
		struct timespec ts = { 0 };
		clock_gettime(1, &ts);
		unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

		return nanoSeconds;
	}

	u64 RGFW_getTime(void) {
		struct timespec ts = { 0 };
		clock_gettime(1, &ts);
		unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

		return (double)(nanoSeconds) * 1e-9;
	}
#endif /* end of wayland or X11 time defines*/


/*

	Start of Wayland defines


*/

#ifdef RGFW_WAYLAND
/*
Wayland TODO:
- fix RGFW_keyPressed lock state

	RGFW_windowMoved, 		the window was moved (by the user)
	RGFW_windowResized  	the window was resized (by the user), [on webASM this means the browser was resized]
	RGFW_windowRefresh	 	The window content needs to be refreshed

	RGFW_dnd 				a file has been dropped into the window
	RGFW_dnd_init

- window args:
	#define RGFW_NO_RESIZE	 			the window cannot be resized  by the user
	#define RGFW_ALLOW_DND     			the window supports drag and drop
	#define RGFW_SCALE_TO_MONITOR 			scale the window to the screen 

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

void RGFW_eventPipe_push(RGFW_window* win, RGFW_Event event) {
	if (win == NULL) {
		win = RGFW_key_win;

		if (win == NULL) return;
	}
	
	if (win->src.eventLen >= (i32)(sizeof(win->src.events) / sizeof(win->src.events[0])))
		return;

	win->src.events[win->src.eventLen] = event;
	win->src.eventLen += 1;
}

RGFW_Event RGFW_eventPipe_pop(RGFW_window* win) {
	RGFW_Event ev;
	ev.type = 0;
	
	if (win->src.eventLen > -1)
		win->src.eventLen -= 1;
	
	if (win->src.eventLen >= 0)  
		ev = win->src.events[win->src.eventLen];
	else {
		printf("H2\n");
	}

	return ev;	
}

/* wayland global garbage (wayland bad, X11 is fine (ish) (not really)) */
#include "xdg-shell.h"
#include "xdg-decoration-unstable-v1.h"

struct xdg_wm_base *xdg_wm_base;
struct wl_compositor* RGFW_compositor = NULL;
struct wl_shm* shm = NULL;
struct wl_shell* RGFW_shell = NULL;
static struct wl_seat *seat = NULL;
static struct xkb_context *xkb_context;
static struct xkb_keymap *keymap = NULL;
static struct xkb_state *xkb_state = NULL;
enum zxdg_toplevel_decoration_v1_mode client_preferred_mode, RGFW_current_mode;
static struct zxdg_decoration_manager_v1 *decoration_manager = NULL;

struct wl_cursor_theme* RGFW_wl_cursor_theme = NULL;
struct wl_surface* RGFW_cursor_surface = NULL;
struct wl_cursor_image* RGFW_cursor_image = NULL;

static void xdg_wm_base_ping_handler(void *data,
        struct xdg_wm_base *wm_base, uint32_t serial)
{
	RGFW_UNUSED(data);
    xdg_wm_base_pong(wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping_handler,
};

b8 RGFW_wl_configured = 0;

static void xdg_surface_configure_handler(void *data,
        struct xdg_surface *xdg_surface, uint32_t serial)
{	
	RGFW_UNUSED(data);
    xdg_surface_ack_configure(xdg_surface, serial);
	#ifdef RGFW_DEBUG
	printf("Surface configured\n");
	#endif
	RGFW_wl_configured = 1;
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler,
};

static void xdg_toplevel_configure_handler(void *data,
        struct xdg_toplevel *toplevel, int32_t width, int32_t height,
        struct wl_array *states)
{
	RGFW_UNUSED(data); RGFW_UNUSED(toplevel); RGFW_UNUSED(states)
    fprintf(stderr, "XDG toplevel configure: %dx%d\n", width, height);
}

static void xdg_toplevel_close_handler(void *data,
        struct xdg_toplevel *toplevel)
{
	RGFW_UNUSED(data);
	RGFW_window* win = (RGFW_window*)xdg_toplevel_get_user_data(toplevel);
	if (win == NULL)
		win = RGFW_key_win;
	
	RGFW_Event ev;
	ev.type = RGFW_quit;

	RGFW_eventPipe_push(win, ev); 	

	RGFW_windowQuitCallback(win);
}

static void shm_format_handler(void *data,
        struct wl_shm *shm, uint32_t format)
{
	RGFW_UNUSED(data); RGFW_UNUSED(shm);
    fprintf(stderr, "Format %d\n", format);
}

static const struct wl_shm_listener shm_listener = {
    .format = shm_format_handler,
};

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure_handler,
    .close = xdg_toplevel_close_handler,
};

RGFW_window* RGFW_mouse_win = NULL;

static void pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	RGFW_UNUSED(data); RGFW_UNUSED(pointer); RGFW_UNUSED(serial); RGFW_UNUSED(surface_x); RGFW_UNUSED(surface_y);
	RGFW_window* win = (RGFW_window*)wl_surface_get_user_data(surface);
	RGFW_mouse_win = win;

	RGFW_Event ev;
	ev.type = RGFW_mouseEnter;
	ev.point = win->event.point;

	RGFW_eventPipe_push(win, ev); 	

	RGFW_mouseNotifyCallBack(win, win->event.point, RGFW_TRUE);
}
static void pointer_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {
	RGFW_UNUSED(data); RGFW_UNUSED(pointer); RGFW_UNUSED(serial); RGFW_UNUSED(surface);
	RGFW_window* win = (RGFW_window*)wl_surface_get_user_data(surface);
	if (RGFW_mouse_win == win)
		RGFW_mouse_win = NULL;
	
	RGFW_Event ev;
	ev.type = RGFW_mouseLeave;
	ev.point = win->event.point;
	RGFW_eventPipe_push(win, ev);

	RGFW_mouseNotifyCallBack(win,  win->event.point, RGFW_FALSE);
}
static void pointer_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
	RGFW_UNUSED(data); RGFW_UNUSED(pointer); RGFW_UNUSED(time); RGFW_UNUSED(x); RGFW_UNUSED(y);

	assert(RGFW_mouse_win != NULL);
	
	RGFW_Event ev;
	ev.type = RGFW_mousePosChanged;
	ev.point = RGFW_POINT(wl_fixed_to_double(x), wl_fixed_to_double(y));
	RGFW_eventPipe_push(RGFW_mouse_win, ev);
	
	RGFW_mousePosCallback(RGFW_mouse_win, RGFW_POINT(wl_fixed_to_double(x), wl_fixed_to_double(y)));
}
static void pointer_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
	RGFW_UNUSED(data); RGFW_UNUSED(pointer); RGFW_UNUSED(time); RGFW_UNUSED(serial);
	assert(RGFW_mouse_win != NULL);

	u32 b = (button - 0x110) + 1;

	/* flip right and middle button codes */
	if (b == 2) b = 3;
	else if (b == 3) b = 2;
	
	RGFW_mouseButtons[b].prev = RGFW_mouseButtons[b].current;
	RGFW_mouseButtons[b].current = state;

	RGFW_Event ev;
	ev.type = RGFW_mouseButtonPressed + state;
	ev.button = b;
	RGFW_eventPipe_push(RGFW_mouse_win, ev);

	RGFW_mouseButtonCallback(RGFW_mouse_win, b, 0, state);
}
static void pointer_axis(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
	RGFW_UNUSED(data); RGFW_UNUSED(pointer); RGFW_UNUSED(time);  RGFW_UNUSED(axis);
	assert(RGFW_mouse_win != NULL); 

	double scroll = wl_fixed_to_double(value);

	RGFW_Event ev;
	ev.type = RGFW_mouseButtonPressed;
	ev.button = RGFW_mouseScrollUp + (scroll < 0);
	RGFW_eventPipe_push(RGFW_mouse_win, ev);

	RGFW_mouseButtonCallback(RGFW_mouse_win, RGFW_mouseScrollUp + (scroll < 0), scroll, 1);
}

void RGFW_doNothing(void) { }
static struct wl_pointer_listener pointer_listener = (struct wl_pointer_listener){&pointer_enter, &pointer_leave, &pointer_motion, &pointer_button, &pointer_axis, (void*)&RGFW_doNothing, (void*)&RGFW_doNothing, (void*)&RGFW_doNothing, (void*)&RGFW_doNothing, (void*)&RGFW_doNothing, (void*)&RGFW_doNothing};

static void keyboard_keymap (void *data, struct wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size) {
	RGFW_UNUSED(data); RGFW_UNUSED(keyboard); RGFW_UNUSED(format);

	char *keymap_string = mmap (NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	xkb_keymap_unref (keymap);
	keymap = xkb_keymap_new_from_string (xkb_context, keymap_string, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	
	munmap (keymap_string, size);
	close (fd);
	xkb_state_unref (xkb_state);
	xkb_state = xkb_state_new (keymap);
}
static void keyboard_enter (void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) { 
	RGFW_UNUSED(data); RGFW_UNUSED(keyboard); RGFW_UNUSED(serial); RGFW_UNUSED(keys);

	RGFW_key_win = (RGFW_window*)wl_surface_get_user_data(surface);

	RGFW_Event ev;
	ev.type = RGFW_focusIn;
	ev.inFocus = RGFW_TRUE;
	RGFW_key_win->event.inFocus = RGFW_TRUE;

	RGFW_eventPipe_push((RGFW_window*)RGFW_mouse_win, ev);

	RGFW_focusCallback(RGFW_key_win, RGFW_TRUE);
}
static void keyboard_leave (void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface) { 
	RGFW_UNUSED(data); RGFW_UNUSED(keyboard); RGFW_UNUSED(serial);

	RGFW_window* win = (RGFW_window*)wl_surface_get_user_data(surface);
	if (RGFW_key_win == win)
		RGFW_key_win = NULL;	

	RGFW_Event ev;
	ev.type = RGFW_focusOut;
	ev.inFocus = RGFW_FALSE;
	win->event.inFocus = RGFW_FALSE;
	RGFW_eventPipe_push(win, ev);

	RGFW_focusCallback(win, RGFW_FALSE);
}
static void keyboard_key (void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	RGFW_UNUSED(data); RGFW_UNUSED(keyboard); RGFW_UNUSED(serial); RGFW_UNUSED(time); 

	assert(RGFW_key_win != NULL);

	xkb_keysym_t keysym = xkb_state_key_get_one_sym (xkb_state, key+8);
	char name[16];
	xkb_keysym_get_name(keysym, name, 16);

	u32 RGFW_key = RGFW_apiKeyCodeToRGFW(key);
	RGFW_keyboard[RGFW_key].prev = RGFW_keyboard[RGFW_key].current;
	RGFW_keyboard[RGFW_key].current = state;
	RGFW_Event ev;
	ev.type = RGFW_keyPressed + state;
	ev.keyCode = RGFW_key;
	strcpy(ev.keyName, name);
	ev.repeat = RGFW_isHeld(RGFW_key_win, RGFW_key);
	RGFW_eventPipe_push(RGFW_key_win, ev);
	
	RGFW_updateLockState(RGFW_key_win, xkb_keymap_mod_get_index(keymap, "Lock"), xkb_keymap_mod_get_index(keymap, "Mod2"));

	RGFW_keyCallback(RGFW_key_win, RGFW_key, name, RGFW_key_win->event.lockState, state);
}
static void keyboard_modifiers (void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
	RGFW_UNUSED(data); RGFW_UNUSED(keyboard); RGFW_UNUSED(serial); RGFW_UNUSED(time); 
	xkb_state_update_mask (xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}
static struct wl_keyboard_listener keyboard_listener = {&keyboard_keymap, &keyboard_enter, &keyboard_leave, &keyboard_key, &keyboard_modifiers, (void*)&RGFW_doNothing};

static void seat_capabilities (void *data, struct wl_seat *seat, uint32_t capabilities) {
	RGFW_UNUSED(data);

	if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
		struct wl_pointer *pointer = wl_seat_get_pointer (seat);
		wl_pointer_add_listener (pointer, &pointer_listener, NULL);
	}
	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
		struct wl_keyboard *keyboard = wl_seat_get_keyboard (seat);
		wl_keyboard_add_listener (keyboard, &keyboard_listener, NULL);
	}
}
static struct wl_seat_listener seat_listener = {&seat_capabilities, (void*)&RGFW_doNothing};

static void wl_global_registry_handler(void *data,
		struct wl_registry *registry, uint32_t id, const char *interface,
		uint32_t version)
{
	RGFW_UNUSED(data); RGFW_UNUSED(version);

    if (strcmp(interface, "wl_compositor") == 0) {
		RGFW_compositor = wl_registry_bind(registry,
			id, &wl_compositor_interface, 4);
	} else if (strcmp(interface, "xdg_wm_base") == 0) {
	xdg_wm_base = wl_registry_bind(registry,
		id, &xdg_wm_base_interface, 1);
	} else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
		decoration_manager = wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1);
    } else if (strcmp(interface, "wl_shm") == 0) {
        shm = wl_registry_bind(registry,
            id, &wl_shm_interface, 1);
        wl_shm_add_listener(shm, &shm_listener, NULL);
	} else if (strcmp(interface,"wl_seat") == 0) {
		seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
		wl_seat_add_listener(seat, &seat_listener, NULL);
	}

	else {
		#ifdef RGFW_DEBUG
		printf("did not register %s\n", interface);
		return;
		#endif
	}

		#ifdef RGFW_DEBUG
		printf("registered %s\n", interface);
		#endif
}

static void wl_global_registry_remove(void *data, struct wl_registry *registry, uint32_t name) { RGFW_UNUSED(data); RGFW_UNUSED(registry); RGFW_UNUSED(name); }
static const struct wl_registry_listener registry_listener = {
	.global = wl_global_registry_handler,
	.global_remove = wl_global_registry_remove,
};

static const char *get_mode_name(enum zxdg_toplevel_decoration_v1_mode mode) {
	switch (mode) {
	case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
		return "client-side decorations";
	case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
		return "server-side decorations";
	}
	abort();
}


static void decoration_handle_configure(void *data,
		struct zxdg_toplevel_decoration_v1 *decoration,
		enum zxdg_toplevel_decoration_v1_mode mode) {
	RGFW_UNUSED(data); RGFW_UNUSED(decoration);
	printf("Using %s\n", get_mode_name(mode));
	RGFW_current_mode = mode;
}

static const struct zxdg_toplevel_decoration_v1_listener decoration_listener = {
	.configure = decoration_handle_configure,
};

static void randname(char *buf) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	for (int i = 0; i < 6; ++i) {
		buf[i] = 'A'+(r&15)+(r&16)*2;
		r >>= 5;
	}
}

static int anonymous_shm_open(void) {
	char name[] = "/RGFW-wayland-XXXXXX";
	int retries = 100;

	do {
		randname(name + strlen(name) - 6);

		--retries;
		// shm_open guarantees that O_CLOEXEC is set
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

static void wl_surface_frame_done(void *data, struct wl_callback *cb, uint32_t time) {
	#ifdef RGFW_BUFFER
		RGFW_window* win = (RGFW_window*)data;
		if ((win->_winArgs & RGFW_NO_CPU_RENDER))
			return;	
		
		#ifndef RGFW_X11_DONT_CONVERT_BGR
			u32 x, y;
			for (y = 0; y < (u32)win->r.h; y++) {
				for (x = 0; x < (u32)win->r.w; x++) {
					u32 index = (y * 4 * win->r.w) + x * 4;

					u8 red = win->buffer[index];
					win->buffer[index] = win->buffer[index + 2];
					win->buffer[index + 2] = red;

				}
			}
		#endif	
	
		wl_surface_attach(win->src.surface, win->src.wl_buffer, 0, 0);
		wl_surface_damage_buffer(win->src.surface, 0, 0, win->r.w, win->r.h);
		wl_surface_commit(win->src.surface);
	#endif
}

static const struct wl_callback_listener wl_surface_frame_listener = {
	.done = wl_surface_frame_done,
};


	/* normal wayland RGFW stuff */
	
	RGFW_area RGFW_getScreenSize(void) {
		RGFW_area area = {};

		if (RGFW_root != NULL)
			/* this isn't right but it's here for buffers */
			area = RGFW_AREA(RGFW_root->r.w, RGFW_root->r.h);
		
		/* TODO wayland */
		return area;
	}
	
	void RGFW_releaseCursor(RGFW_window* win) {
		RGFW_UNUSED(win);
	}

	void RGFW_captureCursor(RGFW_window* win, RGFW_rect r) {
		RGFW_UNUSED(win); RGFW_UNUSED(r);

		/* TODO wayland */
	}


	RGFWDEF void RGFW_init_buffer(RGFW_window* win);
	void RGFW_init_buffer(RGFW_window* win) {
		#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)	
			size_t size = win->r.w * win->r.h * 4;
			int fd = create_shm_file(size);
			if (fd < 0) {
				fprintf(stderr, "Failed to create a buffer. size: %ld\n", size);
				exit(1);
			}

			win->buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			if (win->buffer == MAP_FAILED) {
				fprintf(stderr, "mmap failed!\n");
				close(fd);
				exit(1);
			}

			struct wl_shm_pool* pool = wl_shm_create_pool(shm, fd, size);
			win->src.wl_buffer = wl_shm_pool_create_buffer(pool, 0, win->r.w, win->r.h, win->r.w * 4,
				WL_SHM_FORMAT_ARGB8888);
			wl_shm_pool_destroy(pool);

			close(fd);
			
			wl_surface_attach(win->src.surface, win->src.wl_buffer, 0, 0);
			wl_surface_commit(win->src.surface);

			u8 color[] = {0x00, 0x00, 0x00, 0xFF};

			size_t i;
			for (i = 0; i < size; i += 4) {
				memcpy(&win->buffer[i], color, 4);
			}
	
			#if defined(RGFW_OSMESA)
					win->src.ctx = OSMesaCreateContext(OSMESA_RGBA, NULL);
					OSMesaMakeCurrent(win->src.ctx, win->buffer, GL_UNSIGNED_BYTE, win->r.w, win->r.h);
			#endif
		#else
		RGFW_UNUSED(win);
		#endif
	}
   

	RGFW_window* RGFW_createWindow(const char* name, RGFW_rect rect, u16 args) {
		RGFW_window* win = RGFW_window_basic_init(rect, args);
		
		fprintf(stderr, "Warning: RGFW Wayland support is experimental\n");
		
		win->src.display = wl_display_connect(NULL);
		if (win->src.display == NULL) {
			#ifdef RGFW_DEBUG
				fprintf(stderr, "Failed to load Wayland display\n");
			#endif
			return NULL;
		}
		
		struct wl_registry *registry = wl_display_get_registry(win->src.display);
		wl_registry_add_listener(registry, &registry_listener, NULL);
			
		wl_display_dispatch(win->src.display);
		wl_display_roundtrip(win->src.display);

		if (RGFW_compositor == NULL) {
			#ifdef RGFW_DEBUG
				fprintf(stderr, "Can't find compositor.\n");
			#endif
			
			return NULL;
		}
		
		if (RGFW_wl_cursor_theme == NULL) {
			RGFW_wl_cursor_theme = wl_cursor_theme_load(NULL, 24, shm);
			RGFW_cursor_surface = wl_compositor_create_surface(RGFW_compositor); 
			
			struct wl_cursor* cursor = wl_cursor_theme_get_cursor(RGFW_wl_cursor_theme, "left_ptr");
			RGFW_cursor_image = cursor->images[0];
			struct wl_buffer* cursor_buffer	= wl_cursor_image_get_buffer(RGFW_cursor_image);

			wl_surface_attach(RGFW_cursor_surface, cursor_buffer, 0, 0);
			wl_surface_commit(RGFW_cursor_surface); 
		}

		if (RGFW_root == NULL)
			xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, NULL);
		
		xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

		win->src.surface = wl_compositor_create_surface(RGFW_compositor);
		wl_surface_set_user_data(win->src.surface, win);

		win->src.xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, win->src.surface);
		xdg_surface_add_listener(win->src.xdg_surface, &xdg_surface_listener, NULL);
	
		xdg_wm_base_set_user_data(xdg_wm_base, win);

		win->src.xdg_toplevel = xdg_surface_get_toplevel(win->src.xdg_surface);
		xdg_toplevel_set_user_data(win->src.xdg_toplevel, win);
		xdg_toplevel_set_title(win->src.xdg_toplevel, name);
		xdg_toplevel_add_listener(win->src.xdg_toplevel, &xdg_toplevel_listener, NULL);

		xdg_surface_set_window_geometry(win->src.xdg_surface, 0, 0, win->r.w, win->r.h);
		
		if (!(args & RGFW_NO_BORDER)) {
			win->src.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
						decoration_manager, win->src.xdg_toplevel);
		}


		if (args & RGFW_OPENGL_SOFTWARE)
			setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);

		wl_display_roundtrip(win->src.display);

		wl_surface_commit(win->src.surface);
		
		/* wait for the surface to be configured */
		while (wl_display_dispatch(win->src.display) != -1 && !RGFW_wl_configured) { }
		
		
		#ifdef RGFW_OPENGL
			if ((args & RGFW_NO_INIT_API) == 0) {
				win->src.window = wl_egl_window_create(win->src.surface, win->r.w, win->r.h);
				RGFW_createOpenGLContext(win);
			}
		#endif	

		RGFW_init_buffer(win);

		struct wl_callback* callback = wl_surface_frame(win->src.surface);
   		wl_callback_add_listener(callback, &wl_surface_frame_listener, win);	
		wl_surface_commit(win->src.surface);

		if (args & RGFW_HIDE_MOUSE) {
			RGFW_window_showMouse(win, 0);
		}
		
		if (RGFW_root == NULL) {
			RGFW_root = win;
		}
		
		win->src.eventIndex = 0;
		win->src.eventLen = 0;
		
		return win;
	}

	RGFW_Event* RGFW_window_checkEvent(RGFW_window* win) {
		if (win->_winArgs & RGFW_WINDOW_HIDE)
			return NULL;

		if (win->src.eventIndex == 0) {
			if (wl_display_roundtrip(win->src.display) == -1) {
				return NULL;
			}
			RGFW_resetKey();
		}

		#ifdef __linux__
			RGFW_Event* event = RGFW_linux_updateJoystick(win);
			if (event != NULL)
				return event;
		#endif
		
		if (win->src.eventLen == 0) {
				return NULL;
		}

		RGFW_Event ev = RGFW_eventPipe_pop(win);
		
		if (ev.type ==  0 || win->event.type == RGFW_quit) {
			return NULL;
		}
        
		ev.frameTime = win->event.frameTime;
        ev.frameTime2 = win->event.frameTime2;
        ev.inFocus = win->event.inFocus;
        win->event = ev;
		
		return &win->event;
	}


	void RGFW_window_resize(RGFW_window* win, RGFW_area a) {
		RGFW_UNUSED(win); RGFW_UNUSED(a);

		/* TODO wayland */
	}

	void RGFW_window_move(RGFW_window* win, RGFW_point v) {
		RGFW_UNUSED(win); RGFW_UNUSED(v);

		/* TODO wayland */
	}

	void RGFW_window_setIcon(RGFW_window* win, u8* src, RGFW_area a, i32 channels) {
		RGFW_UNUSED(win); RGFW_UNUSED(src); RGFW_UNUSED(a); RGFW_UNUSED(channels)
		/* TODO wayland */
	}

	void RGFW_window_moveMouse(RGFW_window* win, RGFW_point v) {
		RGFW_UNUSED(win); RGFW_UNUSED(v);

		/* TODO wayland */
	}

	void RGFW_window_showMouse(RGFW_window* win, i8 show) {
		RGFW_UNUSED(win);

		if (show) {

		}
		else {
			
		}

		/* TODO wayland */
	}

	b8 RGFW_window_isMaximized(RGFW_window* win) {
		RGFW_UNUSED(win);
		/* TODO wayland */
		return 0;
	}

	b8 RGFW_window_isMinimized(RGFW_window* win) {
		RGFW_UNUSED(win);
		/* TODO wayland */
		return 0;
	}

	b8 RGFW_window_isHidden(RGFW_window* win) {
		RGFW_UNUSED(win);
		/* TODO wayland */
		return 0;
	}

	b8 RGFW_window_isFullscreen(RGFW_window* win) {
		RGFW_UNUSED(win);
		/* TODO wayland */
		return 0;
	}

	RGFW_point RGFW_window_getMousePoint(RGFW_window* win) {
		RGFW_UNUSED(win);
		/* TODO wayland */
		return RGFW_POINT(0, 0);
	}
			
	RGFW_point RGFW_getGlobalMousePoint(void) {
		/* TODO wayland */
		return RGFW_POINT(0, 0);
	}
			
	void RGFW_window_show(RGFW_window* win) {
		//wl_surface_attach(win->src.surface, win->rc., 0, 0);
        wl_surface_commit(win->src.surface);
		
		if (win->_winArgs & RGFW_WINDOW_HIDE)
			win->_winArgs ^= RGFW_WINDOW_HIDE;
	}
		
	void RGFW_window_hide(RGFW_window* win) {
		wl_surface_attach(win->src.surface, NULL, 0, 0);
        wl_surface_commit(win->src.surface);
		win->_winArgs |= RGFW_WINDOW_HIDE;
	}
		
	void RGFW_window_setMouseDefault(RGFW_window* win) {
		RGFW_UNUSED(win);
	
		RGFW_window_setMouseStandard(win, RGFW_MOUSE_NORMAL);
	}
		
	void RGFW_window_setMouseStandard(RGFW_window* win, u8 mouse) {
		RGFW_UNUSED(win);
		
		static const char* iconStrings[] = { "left_ptr", "left_ptr", "text", "cross", "pointer", "e-resize", "n-resize", "nw-resize", "ne-resize", "all-resize", "not-allowed" };

		struct wl_cursor* cursor = wl_cursor_theme_get_cursor(RGFW_wl_cursor_theme, iconStrings[mouse]);
		RGFW_cursor_image = cursor->images[0];
		struct wl_buffer* cursor_buffer	= wl_cursor_image_get_buffer(RGFW_cursor_image);

		wl_surface_attach(RGFW_cursor_surface, cursor_buffer, 0, 0);
		wl_surface_commit(RGFW_cursor_surface); 
	}
		
	void RGFW_window_setMouse(RGFW_window* win, u8* image, RGFW_area a, i32 channels) {
		RGFW_UNUSED(win); RGFW_UNUSED(image); RGFW_UNUSED(a); RGFW_UNUSED(channels)
		//struct wl_cursor* cursor = wl_cursor_theme_get_cursor(RGFW_wl_cursor_theme, iconStrings[mouse]);
		//RGFW_cursor_image = image;
		struct wl_buffer* cursor_buffer	= wl_cursor_image_get_buffer(RGFW_cursor_image);

		wl_surface_attach(RGFW_cursor_surface, cursor_buffer, 0, 0);
		wl_surface_commit(RGFW_cursor_surface); 
	}
		
	void RGFW_window_setName(RGFW_window* win, char* name) {
		xdg_toplevel_set_title(win->src.xdg_toplevel, name);
	}
		
	void RGFW_window_setMousePassthrough(RGFW_window* win, b8 passthrough) {
		RGFW_UNUSED(win); RGFW_UNUSED(passthrough);

	/* TODO wayland */
	}
		
	void RGFW_window_setBorder(RGFW_window* win, b8 border) {
		RGFW_UNUSED(win); RGFW_UNUSED(border);

	/* TODO wayland */
	}
		
	void RGFW_window_restore(RGFW_window* win) {
		RGFW_UNUSED(win);

	/* TODO wayland */
	}
		
	void RGFW_window_minimize(RGFW_window* win) {
		RGFW_UNUSED(win);

	/* TODO wayland */
	}
		
	void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a) {
		RGFW_UNUSED(win); RGFW_UNUSED(a);

	/* TODO wayland */
	}
		
	void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a) {
		RGFW_UNUSED(win); RGFW_UNUSED(a);

	/* TODO wayland */
	}

	RGFW_monitor RGFW_window_getMonitor(RGFW_window* win) {
		RGFW_monitor m = {};
		RGFW_UNUSED(win);
		RGFW_UNUSED(m);
		/* TODO wayland */

		return m;
	}


	#ifndef RGFW_EGL
	void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) { RGFW_UNUSED(win); RGFW_UNUSED(swapInterval); }
	#endif

	void RGFW_window_swapBuffers(RGFW_window* win) {
		assert(win != NULL);

		/* clear the window*/
		#ifdef RGFW_BUFFER	
			wl_surface_frame_done(win, NULL, 0);
			if (!(win->_winArgs & RGFW_NO_GPU_RENDER)) 
		#endif
		{
		#ifdef RGFW_OPENGL
			eglSwapBuffers(win->src.EGL_display, win->src.EGL_surface);
		#endif
		}
		
		wl_display_flush(win->src.display);
	}

	void RGFW_window_close(RGFW_window* win) {
		#ifdef RGFW_EGL
			RGFW_closeEGL(win);
		#endif

		if (RGFW_root == win) {
			RGFW_root = NULL;
		}

		xdg_toplevel_destroy(win->src.xdg_toplevel);
		xdg_surface_destroy(win->src.xdg_surface);
		wl_surface_destroy(win->src.surface);

		#ifdef RGFW_BUFFER
		wl_buffer_destroy(win->src.wl_buffer);
		#endif
		
		wl_display_disconnect(win->src.display);
		RGFW_FREE(win);
	}

	RGFW_monitor RGFW_getPrimaryMonitor(void) {
		/* TODO wayland */

		return (RGFW_monitor){};
	}
						
	RGFW_monitor* RGFW_getMonitors(void) {
		/* TODO wayland */

		return NULL;
	}

	void RGFW_writeClipboard(const char* text, u32 textLen) {
		RGFW_UNUSED(text); RGFW_UNUSED(textLen);

		/* TODO wayland */
	}

	char* RGFW_readClipboard(size_t* size) {
		RGFW_UNUSED(size);

		/* TODO wayland */

		return NULL;
	}
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

	__declspec(dllimport) int __stdcall WideCharToMultiByte( UINT CodePage, DWORD dwFlags, const WCHAR* lpWideCharStr, int cchWideChar,  LPSTR lpMultiByteStr, int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar);

	#include <processthreadsapi.h>
	#include <wchar.h>
	#include <locale.h>
	#include <windowsx.h>
	#include <shellapi.h>
	#include <shellscalingapi.h>

	#ifndef RGFW_NO_XINPUT
	typedef DWORD (WINAPI * PFN_XInputGetState)(DWORD,XINPUT_STATE*);
	PFN_XInputGetState XInputGetStateSRC = NULL;
	#define XInputGetState XInputGetStateSRC

	typedef DWORD (WINAPI * PFN_XInputGetKeystroke)(DWORD, DWORD, PXINPUT_KEYSTROKE);
	PFN_XInputGetKeystroke XInputGetKeystrokeSRC = NULL;
	#define XInputGetKeystroke XInputGetKeystrokeSRC

	static HMODULE RGFW_XInput_dll = NULL;
	#endif

	u32 RGFW_mouseIconSrc[] = {OCR_NORMAL, OCR_NORMAL, OCR_IBEAM, OCR_CROSS, OCR_HAND, OCR_SIZEWE, OCR_SIZENS, OCR_SIZENWSE, OCR_SIZENESW, OCR_SIZEALL, OCR_NO};

	char* createUTF8FromWideStringWin32(const WCHAR* source);

#define GL_FRONT				0x0404
#define GL_BACK					0x0405
#define GL_LEFT					0x0406
#define GL_RIGHT				0x0407

#if defined(RGFW_OSMESA) && defined(RGFW_LINK_OSMESA)

	typedef void (GLAPIENTRY* PFN_OSMesaDestroyContext)(OSMesaContext);
	typedef i32(GLAPIENTRY* PFN_OSMesaMakeCurrent)(OSMesaContext, void*, int, int, int);
	typedef OSMesaContext(GLAPIENTRY* PFN_OSMesaCreateContext)(GLenum, OSMesaContext);

	PFN_OSMesaMakeCurrent OSMesaMakeCurrentSource;
	PFN_OSMesaCreateContext OSMesaCreateContextSource;
	PFN_OSMesaDestroyContext OSMesaDestroyContextSource;

#define OSMesaCreateContext OSMesaCreateContextSource
#define OSMesaMakeCurrent OSMesaMakeCurrentSource
#define OSMesaDestroyContext OSMesaDestroyContextSource
#endif

	typedef int (*PFN_wglGetSwapIntervalEXT)(void);
	PFN_wglGetSwapIntervalEXT wglGetSwapIntervalEXTSrc = NULL;
#define wglGetSwapIntervalEXT wglGetSwapIntervalEXTSrc


	void* RGFWjoystickApi = NULL;

	/* these two wgl functions need to be preloaded */
	typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hdc, HGLRC hglrc, const int *attribList);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;

	/* defines for creating ARB attributes */
#define WGL_NUMBER_PIXEL_FORMATS_ARB 0x2000
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_NO_ACCELERATION_ARB 0x2025
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_RED_BITS_ARB 0x2015
#define WGL_RED_SHIFT_ARB 0x2016
#define WGL_GREEN_BITS_ARB 0x2017
#define WGL_GREEN_SHIFT_ARB 0x2018
#define WGL_BLUE_BITS_ARB 0x2019
#define WGL_BLUE_SHIFT_ARB 0x201a
#define WGL_ALPHA_BITS_ARB 0x201b
#define WGL_ALPHA_SHIFT_ARB 0x201c
#define WGL_ACCUM_BITS_ARB 0x201d
#define WGL_ACCUM_RED_BITS_ARB 0x201e
#define WGL_ACCUM_GREEN_BITS_ARB 0x201f
#define WGL_ACCUM_BLUE_BITS_ARB 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB 0x2021
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_AUX_BUFFERS_ARB 0x2024
#define WGL_STEREO_ARB 0x2012
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB 					  0x2023
#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_SAMPLE_BUFFERS_ARB               0x2041
#define WGL_SAMPLES_ARB 0x2042
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20a9

#ifndef RGFW_EGL
static HMODULE wglinstance = NULL;
#endif

#ifdef RGFW_WGL_LOAD
	typedef HGLRC(WINAPI* PFN_wglCreateContext)(HDC);
	typedef BOOL(WINAPI* PFN_wglDeleteContext)(HGLRC);
	typedef PROC(WINAPI* PFN_wglGetProcAddress)(LPCSTR);
	typedef BOOL(WINAPI* PFN_wglMakeCurrent)(HDC, HGLRC);
	typedef HDC(WINAPI* PFN_wglGetCurrentDC)();
	typedef HGLRC(WINAPI* PFN_wglGetCurrentContext)();

	PFN_wglCreateContext wglCreateContextSRC;
	PFN_wglDeleteContext wglDeleteContextSRC;
	PFN_wglGetProcAddress wglGetProcAddressSRC;
	PFN_wglMakeCurrent wglMakeCurrentSRC;
	PFN_wglGetCurrentDC wglGetCurrentDCSRC;
	PFN_wglGetCurrentContext wglGetCurrentContextSRC;

	#define wglCreateContext wglCreateContextSRC
	#define wglDeleteContext wglDeleteContextSRC
	#define wglGetProcAddress wglGetProcAddressSRC
	#define wglMakeCurrent wglMakeCurrentSRC

	#define wglGetCurrentDC wglGetCurrentDCSRC
	#define wglGetCurrentContext wglGetCurrentContextSRC
#endif

#ifdef RGFW_OPENGL
	void* RGFW_getProcAddress(const char* procname) { 
		void* proc = (void*) wglGetProcAddress(procname);
		if (proc)
			return proc;

		return (void*) GetProcAddress(wglinstance, procname); 
	}

	typedef HRESULT (APIENTRY* PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
	static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
#endif

	RGFW_window RGFW_eventWindow;

	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_MOVE:
			RGFW_eventWindow.r.x = LOWORD(lParam);
			RGFW_eventWindow.r.y = HIWORD(lParam);
			RGFW_eventWindow.src.window = hWnd;
			return DefWindowProcA(hWnd, message, wParam, lParam);
		case WM_SIZE:
			RGFW_eventWindow.r.w = LOWORD(lParam);
			RGFW_eventWindow.r.h = HIWORD(lParam);
			RGFW_eventWindow.src.window = hWnd;
			return DefWindowProcA(hWnd, message, wParam, lParam); // Call DefWindowProc after handling
		default:
			return DefWindowProcA(hWnd, message, wParam, lParam);
		}
	}
	
	#ifndef RGFW_NO_DPI
	static HMODULE RGFW_Shcore_dll = NULL;
	typedef HRESULT (WINAPI * PFN_GetDpiForMonitor)(HMONITOR,MONITOR_DPI_TYPE,UINT*,UINT*);
	PFN_GetDpiForMonitor GetDpiForMonitorSRC = NULL;
	#define GetDpiForMonitor GetDpiForMonitorSRC
	#endif

	__declspec(dllimport) u32 __stdcall timeBeginPeriod(u32 uPeriod);
	
	#ifndef RGFW_NO_XINPUT
	void RGFW_loadXInput(void) {
		u32 i;
		static const char* names[] = { 
			"xinput1_4.dll",
			"xinput1_3.dll",
			"xinput9_1_0.dll",
			"xinput1_2.dll",
			"xinput1_1.dll"
		};

		for (i = 0; i < sizeof(names) / sizeof(const char*);  i++) {
			RGFW_XInput_dll = LoadLibraryA(names[i]);

			if (RGFW_XInput_dll) {
				XInputGetStateSRC = (PFN_XInputGetState)(void*)GetProcAddress(RGFW_XInput_dll, "XInputGetState");
			
				if (XInputGetStateSRC == NULL)
					printf("Failed to load XInputGetState");
			}
		}
	}
	#endif

	RGFWDEF void RGFW_init_buffer(RGFW_window* win);
	void RGFW_init_buffer(RGFW_window* win) {
#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
	if (RGFW_bufferSize.w == 0 && RGFW_bufferSize.h == 0)
		RGFW_bufferSize = RGFW_getScreenSize();
	
	BITMAPV5HEADER bi = { 0 };
	ZeroMemory(&bi, sizeof(bi));
	bi.bV5Size = sizeof(bi);
	bi.bV5Width = RGFW_bufferSize.w;
	bi.bV5Height = -((LONG) RGFW_bufferSize.h);
	bi.bV5Planes = 1;
	bi.bV5BitCount = 32;
	bi.bV5Compression = BI_BITFIELDS;
	bi.bV5BlueMask = 0x00ff0000;
	bi.bV5GreenMask = 0x0000ff00;
	bi.bV5RedMask = 0x000000ff;
	bi.bV5AlphaMask = 0xff000000;

	win->src.bitmap = CreateDIBSection(win->src.hdc,
		(BITMAPINFO*) &bi,
		DIB_RGB_COLORS,
		(void**) &win->buffer,
		NULL,
		(DWORD) 0);
	
	win->src.hdcMem = CreateCompatibleDC(win->src.hdc);

	#if defined(RGFW_OSMESA)
	win->src.ctx = OSMesaCreateContext(OSMESA_RGBA, NULL);
	OSMesaMakeCurrent(win->src.ctx, win->buffer, GL_UNSIGNED_BYTE, win->r.w, win->r.h);
	#endif
#else
RGFW_UNUSED(win); /*!< if buffer rendering is not being used */
#endif
	}

	void RGFW_window_setDND(RGFW_window* win, b8 allow) {
		DragAcceptFiles(win->src.window, allow);
	}

	void RGFW_releaseCursor(RGFW_window* win) {
		ClipCursor(NULL);
    	const RAWINPUTDEVICE id = { 0x01, 0x02, RIDEV_REMOVE, NULL };
    	RegisterRawInputDevices(&id, 1, sizeof(id));	
	}

	void RGFW_captureCursor(RGFW_window* win, RGFW_rect rect) {
		RGFW_UNUSED(win)
		
		RECT clipRect;
		GetClientRect(win->src.window, &clipRect);
		ClientToScreen(win->src.window, (POINT*) &clipRect.left);
		ClientToScreen(win->src.window, (POINT*) &clipRect.right);
		ClipCursor(&clipRect);

	    const RAWINPUTDEVICE id = { 0x01, 0x02, 0, win->src.window };
		RegisterRawInputDevices(&id, 1, sizeof(id));
	}

	RGFW_window* RGFW_createWindow(const char* name, RGFW_rect rect, u16 args) {
		#ifndef RGFW_NO_XINPUT
		if (RGFW_XInput_dll == NULL)
			RGFW_loadXInput();
		#endif

		#ifndef RGFW_NO_DPI
		if (RGFW_Shcore_dll == NULL) {
			RGFW_Shcore_dll = LoadLibraryA("shcore.dll");
			GetDpiForMonitorSRC = (PFN_GetDpiForMonitor)(void*)GetProcAddress(RGFW_Shcore_dll, "GetDpiForMonitor");
		}
		#endif

		if (wglinstance == NULL) {
			wglinstance = LoadLibraryA("opengl32.dll");
#ifdef RGFW_WGL_LOAD
			wglCreateContextSRC = (PFN_wglCreateContext) GetProcAddress(wglinstance, "wglCreateContext");
			wglDeleteContextSRC = (PFN_wglDeleteContext) GetProcAddress(wglinstance, "wglDeleteContext");
			wglGetProcAddressSRC = (PFN_wglGetProcAddress) GetProcAddress(wglinstance, "wglGetProcAddress");
			wglMakeCurrentSRC = (PFN_wglMakeCurrent) GetProcAddress(wglinstance, "wglMakeCurrent");
			wglGetCurrentDCSRC = (PFN_wglGetCurrentDC) GetProcAddress(wglinstance, "wglGetCurrentDC");
			wglGetCurrentContextSRC = (PFN_wglGetCurrentContext) GetProcAddress(wglinstance, "wglGetCurrentContext");
#endif
		}
	
		if (name[0] == 0) name = (char*) " ";

		RGFW_eventWindow.r = RGFW_RECT(-1, -1, -1, -1);
		RGFW_eventWindow.src.window = NULL;

		RGFW_window* win = RGFW_window_basic_init(rect, args);

		win->src.maxSize = RGFW_AREA(0, 0);
		win->src.minSize = RGFW_AREA(0, 0);


		HINSTANCE inh = GetModuleHandleA(NULL);

		#ifndef __cplusplus
		WNDCLASSA Class = { 0 }; /*!< Setup the Window class. */
		#else
		WNDCLASSA Class = { };
		#endif

		Class.lpszClassName = name;
		Class.hInstance = inh;
		Class.hCursor = LoadCursor(NULL, IDC_ARROW);
		Class.lpfnWndProc = WndProc;

		RegisterClassA(&Class);

		DWORD window_style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		RECT windowRect, clientRect;

		if (!(args & RGFW_NO_BORDER)) {
			window_style |= WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX;

			if (!(args & RGFW_NO_RESIZE))
				window_style |= WS_SIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
		} else
			window_style |= WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX;

		HWND dummyWin = CreateWindowA(Class.lpszClassName, name, window_style, win->r.x, win->r.y, win->r.w, win->r.h, 0, 0, inh, 0);

		GetWindowRect(dummyWin, &windowRect);
		GetClientRect(dummyWin, &clientRect);

		win->src.hOffset = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
		win->src.window = CreateWindowA(Class.lpszClassName, name, window_style, win->r.x, win->r.y, win->r.w, win->r.h + win->src.hOffset, 0, 0, inh, 0);

		if (args & RGFW_ALLOW_DND) {
			win->_winArgs |= RGFW_ALLOW_DND;
			RGFW_window_setDND(win, 1);
		}
		win->src.hdc = GetDC(win->src.window);

		if ((args & RGFW_NO_INIT_API) == 0) {
#ifdef RGFW_DIRECTX
		assert(FAILED(CreateDXGIFactory(&__uuidof(IDXGIFactory), (void**) &RGFW_dxInfo.pFactory)) == 0);

		if (FAILED(RGFW_dxInfo.pFactory->lpVtbl->EnumAdapters(RGFW_dxInfo.pFactory, 0, &RGFW_dxInfo.pAdapter))) {
			fprintf(stderr, "Failed to enumerate DXGI adapters\n");
			RGFW_dxInfo.pFactory->lpVtbl->Release(RGFW_dxInfo.pFactory);
			return NULL;
		}

		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

		if (FAILED(D3D11CreateDevice(RGFW_dxInfo.pAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, featureLevels, 1, D3D11_SDK_VERSION, &RGFW_dxInfo.pDevice, NULL, &RGFW_dxInfo.pDeviceContext))) {
			fprintf(stderr, "Failed to create Direct3D device\n");
			RGFW_dxInfo.pAdapter->lpVtbl->Release(RGFW_dxInfo.pAdapter);
			RGFW_dxInfo.pFactory->lpVtbl->Release(RGFW_dxInfo.pFactory);
			return NULL;
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = win->r.w;
		swapChainDesc.BufferDesc.Height = win->r.h;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = win->src.window;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;
		RGFW_dxInfo.pFactory->lpVtbl->CreateSwapChain(RGFW_dxInfo.pFactory, (IUnknown*) RGFW_dxInfo.pDevice, &swapChainDesc, &win->src.swapchain);

		ID3D11Texture2D* pBackBuffer;
		win->src.swapchain->lpVtbl->GetBuffer(win->src.swapchain, 0, &__uuidof(ID3D11Texture2D), (LPVOID*) &pBackBuffer);
		RGFW_dxInfo.pDevice->lpVtbl->CreateRenderTargetView(RGFW_dxInfo.pDevice, (ID3D11Resource*) pBackBuffer, NULL, &win->src.renderTargetView);
		pBackBuffer->lpVtbl->Release(pBackBuffer);

		D3D11_TEXTURE2D_DESC depthStencilDesc = { 0 };
		depthStencilDesc.Width = win->r.w;
		depthStencilDesc.Height = win->r.h;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ID3D11Texture2D* pDepthStencilTexture = NULL;
		RGFW_dxInfo.pDevice->lpVtbl->CreateTexture2D(RGFW_dxInfo.pDevice, &depthStencilDesc, NULL, &pDepthStencilTexture);

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = { 0 };
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		RGFW_dxInfo.pDevice->lpVtbl->CreateDepthStencilView(RGFW_dxInfo.pDevice, (ID3D11Resource*) pDepthStencilTexture, &depthStencilViewDesc, &win->src.pDepthStencilView);

		pDepthStencilTexture->lpVtbl->Release(pDepthStencilTexture);

		RGFW_dxInfo.pDeviceContext->lpVtbl->OMSetRenderTargets(RGFW_dxInfo.pDeviceContext, 1, &win->src.renderTargetView, win->src.pDepthStencilView);
#endif

#ifdef RGFW_OPENGL 
		HDC dummy_dc = GetDC(dummyWin);
        
        u32 pfd_flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL; 
        
        //if (RGFW_DOUBLE_BUFFER)    
             pfd_flags |= PFD_DOUBLEBUFFER;
		
        PIXELFORMATDESCRIPTOR pfd = {
			sizeof(pfd),
			1, /* version */
			pfd_flags,
		    PFD_TYPE_RGBA, /* ipixel type */
			24, /* color bits */
			0, 0, 0, 0, 0, 0,
			8, /* alpha bits */
			0, 0, 0, 0, 0, 0,
			32, /* depth bits */
			8, /* stencil bits */ 
			0,
			PFD_MAIN_PLANE, /* Layer type */
			0, 0, 0, 0
		};

		int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
		SetPixelFormat(dummy_dc, pixel_format, &pfd);

		HGLRC dummy_context = wglCreateContext(dummy_dc);
		wglMakeCurrent(dummy_dc, dummy_context);

		if (wglChoosePixelFormatARB == NULL) {
			wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) (void*) wglGetProcAddress("wglCreateContextAttribsARB");
			wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) (void*)wglGetProcAddress("wglChoosePixelFormatARB");
		}

		wglMakeCurrent(dummy_dc, 0);
		wglDeleteContext(dummy_context);
		ReleaseDC(dummyWin, dummy_dc);
		
		/* try to create the pixel format we want for opengl and then try to create an opengl context for the specified version */ 
		if (wglCreateContextAttribsARB != NULL) {
			PIXELFORMATDESCRIPTOR pfd = {sizeof(pfd), 1, pfd_flags, PFD_TYPE_RGBA, 32, 8, PFD_MAIN_PLANE, 24, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

			if (args & RGFW_OPENGL_SOFTWARE)
				pfd.dwFlags |= PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED;

			if (wglChoosePixelFormatARB != NULL) {
				i32* pixel_format_attribs = (i32*)RGFW_initFormatAttribs(args & RGFW_OPENGL_SOFTWARE);

				int pixel_format;
				UINT num_formats;
				wglChoosePixelFormatARB(win->src.hdc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
				if (!num_formats) {
					printf("Failed to create a pixel format for WGL.\n");
				}

				DescribePixelFormat(win->src.hdc, pixel_format, sizeof(pfd), &pfd);
				if (!SetPixelFormat(win->src.hdc, pixel_format, &pfd)) {
					printf("Failed to set the WGL pixel format.\n");
				}
			}
			
			/* create opengl/WGL context for the specified version */ 
			u32 index = 0;
			i32 attribs[40];

			if (RGFW_profile == RGFW_GL_CORE) {
				SET_ATTRIB(WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB);
			}
			else {
				SET_ATTRIB(WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB);
			}
			
			if (RGFW_majorVersion || RGFW_minorVersion) {
				SET_ATTRIB(WGL_CONTEXT_MAJOR_VERSION_ARB, RGFW_majorVersion);
				SET_ATTRIB(WGL_CONTEXT_MINOR_VERSION_ARB, RGFW_minorVersion);
			}

			SET_ATTRIB(0, 0);

			win->src.ctx = (HGLRC)wglCreateContextAttribsARB(win->src.hdc, NULL, attribs);
		} else { /* fall back to a default context (probably opengl 2 or something) */
			fprintf(stderr, "Failed to create an accelerated OpenGL Context\n");

			int pixel_format = ChoosePixelFormat(win->src.hdc, &pfd);
			SetPixelFormat(win->src.hdc, pixel_format, &pfd);

			win->src.ctx = wglCreateContext(win->src.hdc);
		}
		
		wglMakeCurrent(win->src.hdc, win->src.ctx);
#endif
	}

#ifdef RGFW_OSMESA
#ifdef RGFW_LINK_OSM ESA
		OSMesaMakeCurrentSource = (PFN_OSMesaMakeCurrent) GetProcAddress(win->src.hdc, "OSMesaMakeCurrent");
		OSMesaCreateContextSource = (PFN_OSMesaCreateContext) GetProcAddress(win->src.hdc, "OSMesaCreateContext");
		OSMesaDestroyContextSource = (PFN_OSMesaDestroyContext) GetProcAddress(win->src.hdc, "OSMesaDestroyContext");
#endif
#endif

#ifdef RGFW_OPENGL
		if ((args & RGFW_NO_INIT_API) == 0) {
			ReleaseDC(win->src.window, win->src.hdc);
			win->src.hdc = GetDC(win->src.window);
			wglMakeCurrent(win->src.hdc, win->src.ctx);
		}
#endif

		DestroyWindow(dummyWin);
		RGFW_init_buffer(win);


		#ifndef RGFW_NO_MONITOR
		if (args & RGFW_SCALE_TO_MONITOR)
			RGFW_window_scaleToMonitor(win);
		#endif

#ifdef RGFW_EGL
		if ((args & RGFW_NO_INIT_API) == 0)
			RGFW_createOpenGLContext(win);
#endif

		if (args & RGFW_HIDE_MOUSE)
			RGFW_window_showMouse(win, 0);

		if (args & RGFW_TRANSPARENT_WINDOW) {
			SetWindowLong(win->src.window, GWL_EXSTYLE, GetWindowLong(win->src.window, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(win->src.window, RGB(255, 255, 255), RGFW_ALPHA, LWA_ALPHA);
		}

		ShowWindow(win->src.window, SW_SHOWNORMAL);
		
		if (RGFW_root == NULL)
			RGFW_root = win;
		
		#ifdef RGFW_OPENGL
		else 
			wglShareLists(RGFW_root->src.ctx, win->src.ctx);
		#endif

		return win;
	}

	void RGFW_window_setBorder(RGFW_window* win, u8 border) {
		DWORD style = GetWindowLong(win->src.window, GWL_STYLE);

		if (border == 0) {
			SetWindowLong(win->src.window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(
				win->src.window, HWND_TOP, 0, 0, 0, 0,
				SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE
			);
		}
		else {
			SetWindowLong(win->src.window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
			SetWindowPos(
				win->src.window, HWND_TOP, 0, 0, 0, 0,
				SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE
			);
		}
	}


	RGFW_area RGFW_getScreenSize(void) {
		return RGFW_AREA(GetDeviceCaps(GetDC(NULL), HORZRES), GetDeviceCaps(GetDC(NULL), VERTRES));
	}

	RGFW_point RGFW_getGlobalMousePoint(void) {
		POINT p;
		GetCursorPos(&p);

		return RGFW_POINT(p.x, p.y);
	}

	RGFW_point RGFW_window_getMousePoint(RGFW_window* win) {
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(win->src.window, &p);

		return RGFW_POINT(p.x, p.y);
	}

	void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a) {
		assert(win != NULL);
		win->src.minSize = a;
	}

	void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a) {
		assert(win != NULL);
		win->src.maxSize = a;
	}


	void RGFW_window_minimize(RGFW_window* win) {
		assert(win != NULL);

		ShowWindow(win->src.window, SW_MINIMIZE);
	}

	void RGFW_window_restore(RGFW_window* win) {
		assert(win != NULL);

		ShowWindow(win->src.window, SW_RESTORE);
	}


	u8 RGFW_xinput2RGFW[] = {
		RGFW_JS_A, /* or PS X button */
		RGFW_JS_B, /* or PS circle button */
		RGFW_JS_X, /* or PS square button */
		RGFW_JS_Y, /* or PS triangle button */
		RGFW_JS_R1, /* right bumper */
		RGFW_JS_L1, /* left bump */
		RGFW_JS_L2, /* left trigger*/
		RGFW_JS_R2, /* right trigger */
		0, 0, 0, 0, 0, 0, 0, 0,
		RGFW_JS_UP, /* dpad up */
		RGFW_JS_DOWN, /* dpad down*/
		RGFW_JS_LEFT, /* dpad left */
		RGFW_JS_RIGHT, /* dpad right */
		RGFW_JS_START, /* start button */
		RGFW_JS_SELECT/* select button */
	};

	static i32 RGFW_checkXInput(RGFW_window* win, RGFW_Event* e) {
		RGFW_UNUSED(win)
		
		size_t i;
		for (i = 0; i < 4; i++) {
			XINPUT_KEYSTROKE keystroke;

			if (XInputGetKeystroke == NULL)
				return 0;

			DWORD result = XInputGetKeystroke((DWORD)i, 0, &keystroke);

			if ((keystroke.Flags & XINPUT_KEYSTROKE_REPEAT) == 0 && result != ERROR_EMPTY) {
				if (result != ERROR_SUCCESS)
					return 0;

				if (keystroke.VirtualKey > VK_PAD_BACK)
					continue;

				// RGFW_jsButtonPressed + 1 = RGFW_jsButtonReleased
				e->type = RGFW_jsButtonPressed + !(keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN);
				e->button = RGFW_xinput2RGFW[keystroke.VirtualKey - 0x5800];
				RGFW_jsPressed[i][e->button] = !(keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN);

				return 1;
			}

			XINPUT_STATE state;
			if (XInputGetState == NULL ||
				XInputGetState((DWORD) i, &state) == ERROR_DEVICE_NOT_CONNECTED
			)
				return 0;
#define INPUT_DEADZONE  ( 0.24f * (float)(0x7FFF) )  // Default to 24% of the +/- 32767 range.   This is a reasonable default value but can be altered if needed.

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
			RGFW_point axis1 = RGFW_POINT(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY);
			RGFW_point axis2 = RGFW_POINT(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY);

			if (axis1.x != e->axis[0].x || axis1.y != e->axis[0].y || axis2.x != e->axis[1].x || axis2.y != e->axis[1].y) {
				e->type = RGFW_jsAxisMove;

				e->axis[0] = axis1;
				e->axis[1] = axis2;

				return 1;
			}

			e->axis[0] = axis1;
			e->axis[1] = axis2;
		}

		return 0;
	}

	void RGFW_stopCheckEvents(void) { 
		PostMessageW(RGFW_root->src.window, WM_NULL, 0, 0);
	}

	void RGFW_window_eventWait(RGFW_window* win, i32 waitMS) {
		RGFW_UNUSED(win);

		MsgWaitForMultipleObjects(0, NULL, FALSE, (DWORD) (waitMS * 1e3), QS_ALLINPUT);
	}

	RGFW_Event* RGFW_window_checkEvent(RGFW_window* win) {
		assert(win != NULL);

		if (win->event.type == RGFW_quit) {
			return NULL;
		}

		MSG msg;

		if (RGFW_eventWindow.src.window == win->src.window) {
			if (RGFW_eventWindow.r.x != -1) {
				win->r.x = RGFW_eventWindow.r.x;
				win->r.y = RGFW_eventWindow.r.y;
				win->event.type = RGFW_windowMoved;
				RGFW_windowMoveCallback(win, win->r);
			}

			if (RGFW_eventWindow.r.w != -1) {
				win->r.w = RGFW_eventWindow.r.w;
				win->r.h = RGFW_eventWindow.r.h;
				win->event.type = RGFW_windowResized;
				RGFW_windowResizeCallback(win, win->r);
			}

			RGFW_eventWindow.src.window = NULL;
			RGFW_eventWindow.r = RGFW_RECT(-1, -1, -1, -1);

			return &win->event;
		}


		static HDROP drop;
		
		if (win->event.type == RGFW_dnd_init) {
			if (win->event.droppedFilesCount) {
				u32 i;
				for (i = 0; i < win->event.droppedFilesCount; i++)
					win->event.droppedFiles[i][0] = '\0';
			}

			win->event.droppedFilesCount = 0;
			win->event.droppedFilesCount = DragQueryFileW(drop, 0xffffffff, NULL, 0);
			//win->event.droppedFiles = (char**)RGFW_CALLOC(win->event.droppedFilesCount, sizeof(char*));

			u32 i;
			for (i = 0; i < win->event.droppedFilesCount; i++) {
				const UINT length = DragQueryFileW(drop, i, NULL, 0);
				WCHAR* buffer = (WCHAR*) RGFW_CALLOC((size_t) length + 1, sizeof(WCHAR));

				DragQueryFileW(drop, i, buffer, length + 1);
				strncpy(win->event.droppedFiles[i], createUTF8FromWideStringWin32(buffer), RGFW_MAX_PATH);
				win->event.droppedFiles[i][RGFW_MAX_PATH - 1] = '\0';
				RGFW_FREE(buffer);
			}

			DragFinish(drop);
			RGFW_dndCallback(win, win->event.droppedFiles, win->event.droppedFilesCount);
			
			win->event.type = RGFW_dnd;
			return &win->event;
		}

		win->event.inFocus = (GetForegroundWindow() == win->src.window);

		if (RGFW_checkXInput(win, &win->event))
			return &win->event;

		static BYTE keyboardState[256];

		if (PeekMessageA(&msg, win->src.window, 0u, 0u, PM_REMOVE)) {
			switch (msg.message) {
			case WM_CLOSE:
			case WM_QUIT:
				RGFW_windowQuitCallback(win);
				win->event.type = RGFW_quit;
				break;

			case WM_ACTIVATE:
				win->event.inFocus = (LOWORD(msg.wParam) == WA_INACTIVE);

				if (win->event.inFocus) {
					win->event.type = RGFW_focusIn;
					RGFW_focusCallback(win, 1);
				}
				else {
					win->event.type = RGFW_focusOut;
					RGFW_focusCallback(win, 0);
				}

				break;
			
			case WM_PAINT:
				win->event.type = RGFW_windowRefresh;
				RGFW_windowRefreshCallback(win);
				break;
			
			case WM_MOUSELEAVE:
				win->event.type = RGFW_mouseLeave;
				win->_winArgs |= RGFW_MOUSE_LEFT;
				RGFW_mouseNotifyCallBack(win, win->event.point, 0);
				break;
			
			case WM_KEYUP: {
				win->event.keyCode = RGFW_apiKeyCodeToRGFW((u32) msg.wParam);
								
				RGFW_keyboard[win->event.keyCode].prev = RGFW_isPressed(win, win->event.keyCode);

				static char keyName[16];
				
				{
					GetKeyNameTextA((LONG) msg.lParam, keyName, 16);

					if ((!(GetKeyState(VK_CAPITAL) & 0x0001) && !(GetKeyState(VK_SHIFT) & 0x8000)) ||
						((GetKeyState(VK_CAPITAL) & 0x0001) && (GetKeyState(VK_SHIFT) & 0x8000))) {
						CharLowerBuffA(keyName, 16);
					}
				}

				RGFW_updateLockState(win, (GetKeyState(VK_CAPITAL) & 0x0001), (GetKeyState(VK_NUMLOCK) & 0x0001));

				strncpy(win->event.keyName, keyName, 16);

				if (RGFW_isPressed(win, RGFW_ShiftL)) {
					ToAscii((UINT) msg.wParam, MapVirtualKey((UINT) msg.wParam, MAPVK_VK_TO_CHAR),
						keyboardState, (LPWORD) win->event.keyName, 0);
				}

				win->event.type = RGFW_keyReleased;
				RGFW_keyboard[win->event.keyCode].current = 0;
				RGFW_keyCallback(win, win->event.keyCode, win->event.keyName, win->event.lockState, 0);
				break;
			}
			case WM_KEYDOWN: {
				win->event.keyCode = RGFW_apiKeyCodeToRGFW((u32) msg.wParam);

				RGFW_keyboard[win->event.keyCode].prev = RGFW_isPressed(win, win->event.keyCode);

				static char keyName[16];
				
				{
					GetKeyNameTextA((LONG) msg.lParam, keyName, 16);

					if ((!(GetKeyState(VK_CAPITAL) & 0x0001) && !(GetKeyState(VK_SHIFT) & 0x8000)) ||
						((GetKeyState(VK_CAPITAL) & 0x0001) && (GetKeyState(VK_SHIFT) & 0x8000))) {
						CharLowerBuffA(keyName, 16);
					}
				}
				
				RGFW_updateLockState(win, (GetKeyState(VK_CAPITAL) & 0x0001), (GetKeyState(VK_NUMLOCK) & 0x0001));

				strncpy(win->event.keyName, keyName, 16);

				if (RGFW_isPressed(win, RGFW_ShiftL) & 0x8000) {
					ToAscii((UINT) msg.wParam, MapVirtualKey((UINT) msg.wParam, MAPVK_VK_TO_CHAR),
						keyboardState, (LPWORD) win->event.keyName, 0);
				}

				win->event.type = RGFW_keyPressed;
				win->event.repeat = RGFW_isPressed(win, win->event.keyCode);
				RGFW_keyboard[win->event.keyCode].current = 1;
				RGFW_keyCallback(win, win->event.keyCode, win->event.keyName, win->event.lockState, 1);
				break;
			}

			case WM_MOUSEMOVE:
				if ((win->_winArgs & RGFW_HOLD_MOUSE))
					break;

				win->event.type = RGFW_mousePosChanged;

				win->event.point.x = GET_X_LPARAM(msg.lParam);
				win->event.point.y = GET_Y_LPARAM(msg.lParam);
				
				RGFW_mousePosCallback(win, win->event.point);

				if (win->_winArgs & RGFW_MOUSE_LEFT) {
					win->_winArgs ^= RGFW_MOUSE_LEFT;
					win->event.type = RGFW_mouseEnter;
					RGFW_mouseNotifyCallBack(win, win->event.point, 1);
				}

				break;

			case WM_INPUT: {
				if (!(win->_winArgs & RGFW_HOLD_MOUSE))
					break;
				
				unsigned size = sizeof(RAWINPUT);
				static RAWINPUT raw[sizeof(RAWINPUT)];
				GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));

				if (raw->header.dwType != RIM_TYPEMOUSE || (raw->data.mouse.lLastX == 0 && raw->data.mouse.lLastY == 0) )
					break;
				
				win->event.type = RGFW_mousePosChanged;
				win->event.point.x = raw->data.mouse.lLastX;
				win->event.point.y = raw->data.mouse.lLastY;
				break;
			}

			case WM_LBUTTONDOWN:
				win->event.button = RGFW_mouseLeft;
				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 1;
				win->event.type = RGFW_mouseButtonPressed;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 1);
				break;
			case WM_RBUTTONDOWN:
				win->event.button = RGFW_mouseRight;
				win->event.type = RGFW_mouseButtonPressed;
				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 1;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 1);
				break;
			case WM_MBUTTONDOWN:
				win->event.button = RGFW_mouseMiddle;
				win->event.type = RGFW_mouseButtonPressed;
				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 1;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 1);
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

			case WM_LBUTTONUP:
			
				win->event.button = RGFW_mouseLeft;
				win->event.type = RGFW_mouseButtonReleased;

				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 0;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 0);
				break;
			case WM_RBUTTONUP:
				win->event.button = RGFW_mouseRight;
				win->event.type = RGFW_mouseButtonReleased;

				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 0;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 0);
				break;
			case WM_MBUTTONUP:
				win->event.button = RGFW_mouseMiddle;
				win->event.type = RGFW_mouseButtonReleased;

				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 0;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 0);
				break;

				/*
					much of this event is source from glfw
				*/
			case WM_DROPFILES: {				
				win->event.type = RGFW_dnd_init;

				drop = (HDROP) msg.wParam;
				POINT pt;

				/* Move the mouse to the position of the drop */
				DragQueryPoint(drop, &pt);

				win->event.point.x = pt.x;
				win->event.point.y = pt.y;

				RGFW_dndInitCallback(win, win->event.point);
			}
				break;
			case WM_GETMINMAXINFO:
			{
				if (win->src.maxSize.w == 0 && win->src.maxSize.h == 0)
					break;

				MINMAXINFO* mmi = (MINMAXINFO*) msg.lParam;
				mmi->ptMinTrackSize.x = win->src.minSize.w;
				mmi->ptMinTrackSize.y = win->src.minSize.h;
				mmi->ptMaxTrackSize.x = win->src.maxSize.w;
				mmi->ptMaxTrackSize.y = win->src.maxSize.h;
				return 0;
			}
			default:
				win->event.type = 0;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		else
			win->event.type = 0;

		if (!IsWindow(win->src.window)) {
			win->event.type = RGFW_quit;
			RGFW_windowQuitCallback(win);
		}

		if (win->event.type)
			return &win->event;
		else
			return NULL;
	}

	u8 RGFW_window_isFullscreen(RGFW_window* win) {
		assert(win != NULL);

		#ifndef __cplusplus
		WINDOWPLACEMENT placement = { 0 };
		#else
		WINDOWPLACEMENT placement = {  };
		#endif
		GetWindowPlacement(win->src.window, &placement);
		return placement.showCmd == SW_SHOWMAXIMIZED;
	}

	u8 RGFW_window_isHidden(RGFW_window* win) {
		assert(win != NULL);

		return IsWindowVisible(win->src.window) == 0 && !RGFW_window_isMinimized(win);
	}

	u8 RGFW_window_isMinimized(RGFW_window* win) {
		assert(win != NULL);

		#ifndef __cplusplus
		WINDOWPLACEMENT placement = { 0 };
		#else
		WINDOWPLACEMENT placement = {  };
		#endif
		GetWindowPlacement(win->src.window, &placement);
		return placement.showCmd == SW_SHOWMINIMIZED;
	}

	u8 RGFW_window_isMaximized(RGFW_window* win) {
		assert(win != NULL);

		#ifndef __cplusplus
		WINDOWPLACEMENT placement = { 0 };
		#else
		WINDOWPLACEMENT placement = {  };
		#endif
		GetWindowPlacement(win->src.window, &placement);
		return placement.showCmd == SW_SHOWMAXIMIZED;
	}

	typedef struct { int iIndex; HMONITOR hMonitor; } RGFW_mInfo;
	BOOL CALLBACK GetMonitorByHandle(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
		RGFW_UNUSED(hdcMonitor)
		RGFW_UNUSED(lprcMonitor)

		RGFW_mInfo* info = (RGFW_mInfo*) dwData;
		if (info->hMonitor == hMonitor)
			return FALSE;

		info->iIndex++;
		return TRUE;
	}
	
	#ifndef RGFW_NO_MONITOR
	RGFW_monitor win32CreateMonitor(HMONITOR src) {
		RGFW_monitor monitor;
		MONITORINFO monitorInfo;

		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfoA(src, &monitorInfo);

		RGFW_mInfo info;
		info.iIndex = 0;
		info.hMonitor = src;

		/* get the monitor's index */
		if (EnumDisplayMonitors(NULL, NULL, GetMonitorByHandle, (LPARAM) &info)) {
			DISPLAY_DEVICEA dd;
			dd.cb = sizeof(dd);

			/* loop through the devices until you find a device with the monitor's index */
			size_t deviceIndex;
			for (deviceIndex = 0; EnumDisplayDevicesA(0, (DWORD) deviceIndex, &dd, 0); deviceIndex++) {
				char* deviceName = dd.DeviceName;
				if (EnumDisplayDevicesA(deviceName, info.iIndex, &dd, 0)) {
					strncpy(monitor.name, dd.DeviceString, 128); /*!< copy the monitor's name */
					break;
				}
			}
		}

		monitor.rect.x = monitorInfo.rcWork.left;
		monitor.rect.y = monitorInfo.rcWork.top;
		monitor.rect.w = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
		monitor.rect.h = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

#ifndef RGFW_NO_DPI
		#ifndef USER_DEFAULT_SCREEN_DPI
		#define USER_DEFAULT_SCREEN_DPI 96
		#endif

		if (GetDpiForMonitor != NULL) {
			u32 x, y;
			GetDpiForMonitor(src, MDT_ANGULAR_DPI, &x, &y);
			monitor.scaleX = (float) (x) / (float) USER_DEFAULT_SCREEN_DPI;
			monitor.scaleY = (float) (y) / (float) USER_DEFAULT_SCREEN_DPI;
		}
#endif

		HDC hdc = GetDC(NULL);
		/* get pixels per inch */
		i32 ppiX = GetDeviceCaps(hdc, LOGPIXELSX);
		i32 ppiY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);

		/* Calculate physical height in inches */
		monitor.physW = GetSystemMetrics(SM_CYSCREEN) / (float) ppiX;
		monitor.physH = GetSystemMetrics(SM_CXSCREEN) / (float) ppiY;

		return monitor;
	}
	#endif /* RGFW_NO_MONITOR */
	

	#ifndef RGFW_NO_MONITOR
	RGFW_monitor RGFW_monitors[6];
	BOOL CALLBACK GetMonitorHandle(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
		RGFW_UNUSED(hdcMonitor)
		RGFW_UNUSED(lprcMonitor)

		RGFW_mInfo* info = (RGFW_mInfo*) dwData;

		if (info->iIndex >= 6)
			return FALSE;

		RGFW_monitors[info->iIndex] = win32CreateMonitor(hMonitor);
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

	RGFW_monitor* RGFW_getMonitors(void) {
		RGFW_mInfo info;
		info.iIndex = 0;
		while (EnumDisplayMonitors(NULL, NULL, GetMonitorHandle, (LPARAM) &info));

		return RGFW_monitors;
	}

	RGFW_monitor RGFW_window_getMonitor(RGFW_window* win) {
		HMONITOR src = MonitorFromWindow(win->src.window, MONITOR_DEFAULTTOPRIMARY);
		return win32CreateMonitor(src);
	}
	#endif

	HICON RGFW_loadHandleImage(RGFW_window* win, u8* src, RGFW_area a, BOOL icon) {
		assert(win != NULL);

		u32 i;
		HDC dc;
		HICON handle;
		HBITMAP color, mask;
		BITMAPV5HEADER bi;
		ICONINFO ii;
		u8* target = NULL;
		u8* source = src;

		ZeroMemory(&bi, sizeof(bi));
		bi.bV5Size = sizeof(bi);
		bi.bV5Width = a.w;
		bi.bV5Height = -((LONG) a.h);
		bi.bV5Planes = 1;
		bi.bV5BitCount = 32;
		bi.bV5Compression = BI_BITFIELDS;
		bi.bV5RedMask = 0x00ff0000;
		bi.bV5GreenMask = 0x0000ff00;
		bi.bV5BlueMask = 0x000000ff;
		bi.bV5AlphaMask = 0xff000000;

		dc = GetDC(NULL);
		color = CreateDIBSection(dc,
			(BITMAPINFO*) &bi,
			DIB_RGB_COLORS,
			(void**) &target,
			NULL,
			(DWORD) 0);
		ReleaseDC(NULL, dc);

		mask = CreateBitmap(a.w, a.h, 1, 1, NULL);

		for (i = 0; i < a.w * a.h; i++) {
			target[0] = source[2];
			target[1] = source[1];
			target[2] = source[0];
			target[3] = source[3];
			target += 4;
			source += 4;
		}

		ZeroMemory(&ii, sizeof(ii));
		ii.fIcon = icon;
		ii.xHotspot = 0;
		ii.yHotspot = 0;
		ii.hbmMask = mask;
		ii.hbmColor = color;

		handle = CreateIconIndirect(&ii);

		DeleteObject(color);
		DeleteObject(mask);

		return handle;
	}

	void RGFW_window_setMouse(RGFW_window* win, u8* image, RGFW_area a, i32 channels) {
		assert(win != NULL);
		RGFW_UNUSED(channels)

		HCURSOR cursor = (HCURSOR) RGFW_loadHandleImage(win, image, a, FALSE);
		SetClassLongPtrA(win->src.window, GCLP_HCURSOR, (LPARAM) cursor);
		SetCursor(cursor);
		DestroyCursor(cursor);
	}

	void RGFW_window_setMouseDefault(RGFW_window* win) {
		RGFW_window_setMouseStandard(win, RGFW_MOUSE_ARROW);
	}

	void RGFW_window_setMouseStandard(RGFW_window* win, u8 mouse) {
		assert(win != NULL);

		if (mouse > (sizeof(RGFW_mouseIconSrc) / sizeof(u32)))
			return;

		char* icon = MAKEINTRESOURCEA(RGFW_mouseIconSrc[mouse]);

		SetClassLongPtrA(win->src.window, GCLP_HCURSOR, (LPARAM) LoadCursorA(NULL, icon));
		SetCursor(LoadCursorA(NULL, icon));
	}

	void RGFW_window_hide(RGFW_window* win) {
		ShowWindow(win->src.window, SW_HIDE);
	}

	void RGFW_window_show(RGFW_window* win) {
		ShowWindow(win->src.window, SW_RESTORE);
	}

	void RGFW_window_close(RGFW_window* win) {
		assert(win != NULL);

#ifdef RGFW_EGL
		RGFW_closeEGL(win);
#endif

		if (win == RGFW_root) {
#ifdef RGFW_DIRECTX
			RGFW_dxInfo.pDeviceContext->lpVtbl->Release(RGFW_dxInfo.pDeviceContext);
			RGFW_dxInfo.pDevice->lpVtbl->Release(RGFW_dxInfo.pDevice);
			RGFW_dxInfo.pAdapter->lpVtbl->Release(RGFW_dxInfo.pAdapter);
			RGFW_dxInfo.pFactory->lpVtbl->Release(RGFW_dxInfo.pFactory);
#endif
		
			if (RGFW_XInput_dll != NULL) {
				FreeLibrary(RGFW_XInput_dll);
				RGFW_XInput_dll = NULL;
			}

			#ifndef RGFW_NO_DPI
			if (RGFW_Shcore_dll != NULL) {
				FreeLibrary(RGFW_Shcore_dll);
				RGFW_Shcore_dll = NULL;
			}
			#endif

			if (wglinstance != NULL) {
				FreeLibrary(wglinstance);
				wglinstance = NULL;
			}

			RGFW_root = NULL;
		}

#ifdef RGFW_DIRECTX
		win->src.swapchain->lpVtbl->Release(win->src.swapchain);
		win->src.renderTargetView->lpVtbl->Release(win->src.renderTargetView);
		win->src.pDepthStencilView->lpVtbl->Release(win->src.pDepthStencilView);
#endif

#ifdef RGFW_BUFFER
		DeleteDC(win->src.hdcMem);
		DeleteObject(win->src.bitmap);
#endif

#ifdef RGFW_OPENGL
		wglDeleteContext((HGLRC) win->src.ctx); /*!< delete opengl context */
#endif
		DeleteDC(win->src.hdc); /*!< delete device context */
		DestroyWindow(win->src.window); /*!< delete window */

#if defined(RGFW_OSMESA)
		if (win->buffer != NULL)
			RGFW_FREE(win->buffer);
#endif

#ifdef RGFW_ALLOC_DROPFILES
		{
			u32 i;
			for (i = 0; i < RGFW_MAX_DROPS; i++)
				RGFW_FREE(win->event.droppedFiles[i]);


			RGFW_FREE(win->event.droppedFiles);
		}
#endif

		RGFW_FREE(win);
	}

	void RGFW_window_move(RGFW_window* win, RGFW_point v) {
		assert(win != NULL);

		win->r.x = v.x;
		win->r.y = v.y;
		SetWindowPos(win->src.window, HWND_TOP, win->r.x, win->r.y, 0, 0, SWP_NOSIZE);
	}

	void RGFW_window_resize(RGFW_window* win, RGFW_area a) {
		assert(win != NULL);

		win->r.w = a.w;
		win->r.h = a.h;
		SetWindowPos(win->src.window, HWND_TOP, 0, 0, win->r.w, win->r.h + win->src.hOffset, SWP_NOMOVE);
	}


	void RGFW_window_setName(RGFW_window* win, char* name) {
		assert(win != NULL);

		SetWindowTextA(win->src.window, name);
	}

	/* sourced from GLFW */
	#ifndef RGFW_NO_PASSTHROUGH
	void RGFW_window_setMousePassthrough(RGFW_window* win, b8 passthrough) {
		assert(win != NULL);
		
		COLORREF key = 0;
		BYTE alpha = 0;
		DWORD flags = 0;
		DWORD exStyle = GetWindowLongW(win->src.window, GWL_EXSTYLE);
		
		if (exStyle & WS_EX_LAYERED)
			GetLayeredWindowAttributes(win->src.window, &key, &alpha, &flags);

		if (passthrough)
			exStyle |= (WS_EX_TRANSPARENT | WS_EX_LAYERED);
		else
		{
			exStyle &= ~WS_EX_TRANSPARENT;
			// NOTE: Window opacity also needs the layered window style so do not
			//       remove it if the window is alpha blended
			if (exStyle & WS_EX_LAYERED)
			{
				if (!(flags & LWA_ALPHA))
					exStyle &= ~WS_EX_LAYERED;
			}
		}

		SetWindowLongW(win->src.window, GWL_EXSTYLE, exStyle);

		if (passthrough) {
			SetLayeredWindowAttributes(win->src.window, key, alpha, flags);
		}
	}
	#endif

	/* much of this function is sourced from GLFW */
	void RGFW_window_setIcon(RGFW_window* win, u8* src, RGFW_area a, i32 channels) {
		assert(win != NULL);
		#ifndef RGFW_WIN95
		RGFW_UNUSED(channels)

		HICON handle = RGFW_loadHandleImage(win, src, a, TRUE);

		SetClassLongPtrA(win->src.window, GCLP_HICON, (LPARAM) handle);

		DestroyIcon(handle);
		#else
		RGFW_UNUSED(src)
		RGFW_UNUSED(a)
		RGFW_UNUSED(channels)
		#endif
	}

	char* RGFW_readClipboard(size_t* size) {
		/* Open the clipboard */
		if (OpenClipboard(NULL) == 0)
			return (char*) "";

		/* Get the clipboard data as a Unicode string */
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		if (hData == NULL) {
			CloseClipboard();
			return (char*) "";
		}

		wchar_t* wstr = (wchar_t*) GlobalLock(hData);

		char* text;

		{
			setlocale(LC_ALL, "en_US.UTF-8");

			size_t textLen = wcstombs(NULL, wstr, 0);
			if (textLen == 0)
				return (char*) "";

			text = (char*) RGFW_MALLOC((textLen * sizeof(char)) + 1);

			wcstombs(text, wstr, (textLen) +1);

			if (size != NULL)
				*size = textLen + 1;

			text[textLen] = '\0';
		}

		/* Release the clipboard data */
		GlobalUnlock(hData);
		CloseClipboard();

		return text;
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

		MultiByteToWideChar(CP_UTF8, 0, text, -1, buffer, textLen);
		GlobalUnlock(object);

		if (!OpenClipboard(RGFW_root->src.window)) {
			GlobalFree(object);
			return;
		}

		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, object);
		CloseClipboard();
	}

	u16 RGFW_registerJoystick(RGFW_window* win, i32 jsNumber) {
		assert(win != NULL);

		RGFW_UNUSED(jsNumber)

		return RGFW_registerJoystickF(win, (char*) "");
	}

	u16 RGFW_registerJoystickF(RGFW_window* win, char* file) {
		assert(win != NULL);
		RGFW_UNUSED(file)

		return RGFW_joystickCount - 1;
	}

	void RGFW_window_moveMouse(RGFW_window* win, RGFW_point p) {
		assert(win != NULL);

		SetCursorPos(p.x, p.y);
	}

	#ifdef RGFW_OPENGL
	void RGFW_window_makeCurrent_OpenGL(RGFW_window* win) {
		if (win == NULL)
			wglMakeCurrent(NULL, NULL);
		else
			wglMakeCurrent(win->src.hdc, (HGLRC) win->src.ctx);
	}
	#endif

	#ifndef RGFW_EGL
	void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) {
		assert(win != NULL);
		
		#if defined(RGFW_OPENGL)
		typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALEXTPROC)(int interval);
		static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
		static void* loadSwapFunc = (void*) 1;

		if (loadSwapFunc == NULL) {
			fprintf(stderr, "wglSwapIntervalEXT not supported\n");
			return;
		}

		if (wglSwapIntervalEXT == NULL) {
			loadSwapFunc = (void*) wglGetProcAddress("wglSwapIntervalEXT");
			wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) loadSwapFunc;
		}

		if (wglSwapIntervalEXT(swapInterval) == FALSE)
			fprintf(stderr, "Failed to set swap interval\n");
		#else
        RGFW_UNUSED(swapInterval);
        #endif

	}
	#endif

	void RGFW_window_swapBuffers(RGFW_window* win) {
		//assert(win != NULL);
		/* clear the window*/

		if (!(win->_winArgs & RGFW_NO_CPU_RENDER)) {
#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
			#ifdef RGFW_OSMESA
			RGFW_OSMesa_reorganize();
			#endif

			HGDIOBJ oldbmp = SelectObject(win->src.hdcMem, win->src.bitmap);
			BitBlt(win->src.hdc, 0, 0, win->r.w, win->r.h, win->src.hdcMem, 0, 0, SRCCOPY);
			SelectObject(win->src.hdcMem, oldbmp);
#endif
		}

		if (!(win->_winArgs & RGFW_NO_GPU_RENDER)) {
			#ifdef RGFW_EGL
					eglSwapBuffers(win->src.EGL_display, win->src.EGL_surface);
			#elif defined(RGFW_OPENGL)
					SwapBuffers(win->src.hdc);
			#endif

			#if defined(RGFW_WINDOWS) && defined(RGFW_DIRECTX)
					win->src.swapchain->lpVtbl->Present(win->src.swapchain, 0, 0);
			#endif
		}
	}

	char* createUTF8FromWideStringWin32(const WCHAR* source) {
		char* target;
		i32 size;

		size = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
		if (!size) {
			return NULL;
		}

		target = (char*) RGFW_CALLOC(size, 1);

		if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, NULL, NULL)) {
			RGFW_FREE(target);
			return NULL;
		}

		return target;
	}
	
    static inline LARGE_INTEGER RGFW_win32_initTimer(void) {
		static LARGE_INTEGER frequency = {{0, 0}};
		if (frequency.QuadPart == 0) {
			timeBeginPeriod(1);
			QueryPerformanceFrequency(&frequency);
		}

		return frequency;
	}

	u64 RGFW_getTimeNS(void) {
		LARGE_INTEGER frequency = RGFW_win32_initTimer();

		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);

		return (u64) ((counter.QuadPart * 1e9) / frequency.QuadPart);
	}

	u64 RGFW_getTime(void) {
		LARGE_INTEGER frequency = RGFW_win32_initTimer();

		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return (u64) (counter.QuadPart / (double) frequency.QuadPart);
	}
	
	void RGFW_sleep(u64 ms) {
		Sleep(ms);
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

#include <CoreVideo/CVDisplayLink.h>
#include <ApplicationServices/ApplicationServices.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <mach/mach_time.h>

	typedef CGRect NSRect;
	typedef CGPoint NSPoint;
	typedef CGSize NSSize;

	typedef void NSBitmapImageRep;
	typedef void NSCursor;
	typedef void NSDraggingInfo;
	typedef void NSWindow;
	typedef void NSApplication;
	typedef void NSScreen;
	typedef void NSEvent;
	typedef void NSString;
	typedef void NSOpenGLContext;
	typedef void NSPasteboard;
	typedef void NSColor;
	typedef void NSArray;
	typedef void NSImageRep;
	typedef void NSImage;
	typedef void NSOpenGLView;


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
#define objc_msgSend_bool			((BOOL (*)(id, SEL))objc_msgSend)
#define objc_msgSend_void			((void (*)(id, SEL))objc_msgSend)
#define objc_msgSend_void_id		((void (*)(id, SEL, id))objc_msgSend)
#define objc_msgSend_uint			((NSUInteger (*)(id, SEL))objc_msgSend)
#define objc_msgSend_void_bool		((void (*)(id, SEL, BOOL))objc_msgSend)
#define objc_msgSend_bool_void		((BOOL (*)(id, SEL))objc_msgSend)
#define objc_msgSend_void_SEL		((void (*)(id, SEL, SEL))objc_msgSend)
#define objc_msgSend_id				((id (*)(id, SEL))objc_msgSend)
#define objc_msgSend_id_id				((id (*)(id, SEL, id))objc_msgSend)
#define objc_msgSend_id_bool			((BOOL (*)(id, SEL, id))objc_msgSend)
#define objc_msgSend_int ((id (*)(id, SEL, int))objc_msgSend)
#define objc_msgSend_arr ((id (*)(id, SEL, int))objc_msgSend)
#define objc_msgSend_ptr ((id (*)(id, SEL, void*))objc_msgSend)
#define objc_msgSend_class ((id (*)(Class, SEL))objc_msgSend)
#define objc_msgSend_class_char ((id (*)(Class, SEL, char*))objc_msgSend)

	NSApplication* NSApp = NULL;

	void NSRelease(id obj) {
		objc_msgSend_void(obj, sel_registerName("release"));
	}

	#define release NSRelease

	NSString* NSString_stringWithUTF8String(const char* str) {	
		return ((id(*)(id, SEL, const char*))objc_msgSend)
			((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), str);
	}

	const char* NSString_to_char(NSString* str) {
		return ((const char* (*)(id, SEL)) objc_msgSend) (str, sel_registerName("UTF8String"));
	}

	void si_impl_func_to_SEL_with_name(const char* class_name, const char* register_name, void* function) {
		Class selected_class;

		if (strcmp(class_name, "NSView") == 0) {
			selected_class = objc_getClass("ViewClass");
		} else if (strcmp(class_name, "NSWindow") == 0) {
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

	void* si_array_init_reserve(size_t sizeof_element, size_t count) {
		siArrayHeader* ptr = malloc(sizeof(siArrayHeader) + (sizeof_element * count));
		void* array = ptr + sizeof(siArrayHeader);

		siArrayHeader* header = SI_ARRAY_HEADER(array);
		header->count = count;

		return array;
	}

#define si_array_len(array) (SI_ARRAY_HEADER(array)->count)
#define si_func_to_SEL(class_name, function) si_impl_func_to_SEL_with_name(class_name, #function":", function)
	/* Creates an Objective-C method (SEL) from a regular C function with the option to set the register name.*/
#define si_func_to_SEL_with_name(class_name, register_name, function) si_impl_func_to_SEL_with_name(class_name, register_name":", function)
	
	unsigned char* NSBitmapImageRep_bitmapData(NSBitmapImageRep* imageRep) {
		return ((unsigned char* (*)(id, SEL))objc_msgSend)
			(imageRep, sel_registerName("bitmapData"));
	}

#define NS_ENUM(type, name) type name; enum

	typedef NS_ENUM(NSUInteger, NSBitmapFormat) {
		NSBitmapFormatAlphaFirst = 1 << 0,       // 0 means is alpha last (RGBA, CMYKA, etc.)
			NSBitmapFormatAlphaNonpremultiplied = 1 << 1,       // 0 means is premultiplied
			NSBitmapFormatFloatingPointSamples = 1 << 2,  // 0 is integer

			NSBitmapFormatSixteenBitLittleEndian API_AVAILABLE(macos(10.10)) = (1 << 8),
			NSBitmapFormatThirtyTwoBitLittleEndian API_AVAILABLE(macos(10.10)) = (1 << 9),
			NSBitmapFormatSixteenBitBigEndian API_AVAILABLE(macos(10.10)) = (1 << 10),
			NSBitmapFormatThirtyTwoBitBigEndian API_AVAILABLE(macos(10.10)) = (1 << 11)
	};

	NSBitmapImageRep* NSBitmapImageRep_initWithBitmapData(unsigned char** planes, NSInteger width, NSInteger height, NSInteger bps, NSInteger spp, bool alpha, bool isPlanar, const char* colorSpaceName, NSBitmapFormat bitmapFormat, NSInteger rowBytes, NSInteger pixelBits) {
		void* func = sel_registerName("initWithBitmapDataPlanes:pixelsWide:pixelsHigh:bitsPerSample:samplesPerPixel:hasAlpha:isPlanar:colorSpaceName:bitmapFormat:bytesPerRow:bitsPerPixel:");

		return (NSBitmapImageRep*) ((id(*)(id, SEL, unsigned char**, NSInteger, NSInteger, NSInteger, NSInteger, bool, bool, const char*, NSBitmapFormat, NSInteger, NSInteger))objc_msgSend)
			(NSAlloc((id)objc_getClass("NSBitmapImageRep")), func, planes, width, height, bps, spp, alpha, isPlanar, NSString_stringWithUTF8String(colorSpaceName), bitmapFormat, rowBytes, pixelBits);
	}

	NSColor* NSColor_colorWithSRGB(CGFloat red, CGFloat green, CGFloat blue, CGFloat alpha) {
		void* nsclass = objc_getClass("NSColor");
		void* func = sel_registerName("colorWithSRGBRed:green:blue:alpha:");
		return ((id(*)(id, SEL, CGFloat, CGFloat, CGFloat, CGFloat))objc_msgSend)
			(nsclass, func, red, green, blue, alpha);
	}

	NSCursor* NSCursor_initWithImage(NSImage* newImage, NSPoint aPoint) {
		void* func = sel_registerName("initWithImage:hotSpot:");
		void* nsclass = objc_getClass("NSCursor");

		return (NSCursor*) ((id(*)(id, SEL, id, NSPoint))objc_msgSend)
			(NSAlloc(nsclass), func, newImage, aPoint);
	}

	void NSImage_addRepresentation(NSImage* image, NSImageRep* imageRep) {
		void* func = sel_registerName("addRepresentation:");
		objc_msgSend_void_id(image, func, imageRep);
	}

	NSImage* NSImage_initWithSize(NSSize size) {
		void* func = sel_registerName("initWithSize:");
		return ((id(*)(id, SEL, NSSize))objc_msgSend)
			(NSAlloc((id)objc_getClass("NSImage")), func, size);
	}
#define NS_OPENGL_ENUM_DEPRECATED(minVers, maxVers) API_AVAILABLE(macos(minVers))
	typedef NS_ENUM(NSInteger, NSOpenGLContextParameter) {
		NSOpenGLContextParameterSwapInterval           NS_OPENGL_ENUM_DEPRECATED(10.0, 10.14) = 222, /* 1 param.  0 -> Don't sync, 1 -> Sync to vertical retrace     */
			NSOpenGLContextParametectxaceOrder           NS_OPENGL_ENUM_DEPRECATED(10.0, 10.14) = 235, /* 1 param.  1 -> Above Window (default), -1 -> Below Window    */
			NSOpenGLContextParametectxaceOpacity         NS_OPENGL_ENUM_DEPRECATED(10.0, 10.14) = 236, /* 1 param.  1-> Surface is opaque (default), 0 -> non-opaque   */
			NSOpenGLContextParametectxaceBackingSize     NS_OPENGL_ENUM_DEPRECATED(10.0, 10.14) = 304, /* 2 params.  Width/height of surface backing size              */
			NSOpenGLContextParameterReclaimResources       NS_OPENGL_ENUM_DEPRECATED(10.0, 10.14) = 308, /* 0 params.                                                    */
			NSOpenGLContextParameterCurrentRendererID      NS_OPENGL_ENUM_DEPRECATED(10.0, 10.14) = 309, /* 1 param.   Retrieves the current renderer ID                 */
			NSOpenGLContextParameterGPUVertexProcessing    NS_OPENGL_ENUM_DEPRECATED(10.0, 10.14) = 310, /* 1 param.   Currently processing vertices with GPU (get)      */
			NSOpenGLContextParameterGPUFragmentProcessing  NS_OPENGL_ENUM_DEPRECATED(10.0, 10.14) = 311, /* 1 param.   Currently processing fragments with GPU (get)     */
			NSOpenGLContextParameterHasDrawable            NS_OPENGL_ENUM_DEPRECATED(10.0, 10.14) = 314, /* 1 param.   Boolean returned if drawable is attached          */
			NSOpenGLContextParameterMPSwapsInFlight        NS_OPENGL_ENUM_DEPRECATED(10.0, 10.14) = 315, /* 1 param.   Max number of swaps queued by the MP GL engine    */

			NSOpenGLContextParameterSwapRectangle API_DEPRECATED("", macos(10.0, 10.14)) = 200, /* 4 params.  Set or get the swap rectangle {x, y, w, h} */
			NSOpenGLContextParameterSwapRectangleEnable API_DEPRECATED("", macos(10.0, 10.14)) = 201, /* Enable or disable the swap rectangle */
			NSOpenGLContextParameterRasterizationEnable API_DEPRECATED("", macos(10.0, 10.14)) = 221, /* Enable or disable all rasterization */
			NSOpenGLContextParameterStateValidation API_DEPRECATED("", macos(10.0, 10.14)) = 301, /* Validate state for multi-screen functionality */
			NSOpenGLContextParametectxaceSurfaceVolatile API_DEPRECATED("", macos(10.0, 10.14)) = 306, /* 1 param.   Surface volatile state */
	};


	void NSOpenGLContext_setValues(NSOpenGLContext* context, const int* vals, NSOpenGLContextParameter param) {
		void* func = sel_registerName("setValues:forParameter:");
		((void (*)(id, SEL, const int*, NSOpenGLContextParameter))objc_msgSend)
			(context, func, vals, param);
	}

	void* NSOpenGLPixelFormat_initWithAttributes(const uint32_t* attribs) {
		void* func = sel_registerName("initWithAttributes:");
		return (void*) ((id(*)(id, SEL, const uint32_t*))objc_msgSend)
			(NSAlloc((id)objc_getClass("NSOpenGLPixelFormat")), func, attribs);
	}

	NSOpenGLView* NSOpenGLView_initWithFrame(NSRect frameRect, uint32_t* format) {
		void* func = sel_registerName("initWithFrame:pixelFormat:");
		return (NSOpenGLView*) ((id(*)(id, SEL, NSRect, uint32_t*))objc_msgSend)
			(NSAlloc((id)objc_getClass("NSOpenGLView")), func, frameRect, format);
	}

	void NSCursor_performSelector(NSCursor* cursor, void* selector) {
		void* func = sel_registerName("performSelector:");
		objc_msgSend_void_SEL(cursor, func, selector);
	}

	NSPasteboard* NSPasteboard_generalPasteboard(void) {
		return (NSPasteboard*) objc_msgSend_id((id)objc_getClass("NSPasteboard"), sel_registerName("generalPasteboard"));
	}

	NSString** cstrToNSStringArray(char** strs, size_t len) {
		static NSString* nstrs[6];
		size_t i;
		for (i = 0; i < len; i++)
			nstrs[i] = NSString_stringWithUTF8String(strs[i]);

		return nstrs;
	}

	const char* NSPasteboard_stringForType(NSPasteboard* pasteboard, NSPasteboardType dataType) {
		void* func = sel_registerName("stringForType:");
		return (const char*) NSString_to_char(((id(*)(id, SEL, const char*))objc_msgSend)(pasteboard, func, NSString_stringWithUTF8String(dataType)));
	}

	NSArray* c_array_to_NSArray(void* array, size_t len) {
		SEL func = sel_registerName("initWithObjects:count:");
		void* nsclass = objc_getClass("NSArray");
		return ((id (*)(id, SEL, void*, NSUInteger))objc_msgSend)
					(NSAlloc(nsclass), func, array, len);
	}
 
	void NSregisterForDraggedTypes(void* view, NSPasteboardType* newTypes, size_t len) {
		NSString** ntypes = cstrToNSStringArray((char**)newTypes, len);

		NSArray* array = c_array_to_NSArray(ntypes, len);
		objc_msgSend_void_id(view, sel_registerName("registerForDraggedTypes:"), array);
		NSRelease(array);
	}

	NSInteger NSPasteBoard_declareTypes(NSPasteboard* pasteboard, NSPasteboardType* newTypes, size_t len, void* owner) {
		NSString** ntypes = cstrToNSStringArray((char**)newTypes, len);

		void* func = sel_registerName("declareTypes:owner:");

		NSArray* array = c_array_to_NSArray(ntypes, len);

		NSInteger output = ((NSInteger(*)(id, SEL, id, void*))objc_msgSend)
			(pasteboard, func, array, owner);
		NSRelease(array);

		return output;
	}

	bool NSPasteBoard_setString(NSPasteboard* pasteboard, const char* stringToWrite, NSPasteboardType dataType) {
		void* func = sel_registerName("setString:forType:");
		return ((bool (*)(id, SEL, id, NSPasteboardType))objc_msgSend)
			(pasteboard, func, NSString_stringWithUTF8String(stringToWrite), NSString_stringWithUTF8String(dataType));
	}

	void NSRetain(id obj) { objc_msgSend_void(obj, sel_registerName("retain")); }

	typedef enum NSApplicationActivationPolicy {
		NSApplicationActivationPolicyRegular,
		NSApplicationActivationPolicyAccessory,
		NSApplicationActivationPolicyProhibited
	} NSApplicationActivationPolicy;

	typedef NS_ENUM(u32, NSBackingStoreType) {
		NSBackingStoreRetained = 0,
			NSBackingStoreNonretained = 1,
			NSBackingStoreBuffered = 2
	};

	typedef NS_ENUM(u32, NSWindowStyleMask) {
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
			NSWindowStyleMaskNonactivatingPanel = 1 << 7,
			NSWindowStyleMaskHUDWindow = 1 << 13
	};

	NSPasteboardType const NSPasteboardTypeString = "public.utf8-plain-text"; // Replaces NSStringPboardType


	typedef NS_ENUM(i32, NSDragOperation) {
		NSDragOperationNone = 0,
			NSDragOperationCopy = 1,
			NSDragOperationLink = 2,
			NSDragOperationGeneric = 4,
			NSDragOperationPrivate = 8,
			NSDragOperationMove = 16,
			NSDragOperationDelete = 32,
			NSDragOperationEvery = ULONG_MAX,

			//NSDragOperationAll_Obsolete	API_DEPRECATED("", macos(10.0,10.10)) = 15, // Use NSDragOperationEvery
			//NSDragOperationAll API_DEPRECATED("", macos(10.0,10.10)) = NSDragOperationAll_Obsolete, // Use NSDragOperationEvery
	};

	void* NSArray_objectAtIndex(NSArray* array, NSUInteger index) {
		void* func = sel_registerName("objectAtIndex:");
		return ((id(*)(id, SEL, NSUInteger))objc_msgSend)(array, func, index);
	}

	const char** NSPasteboard_readObjectsForClasses(NSPasteboard* pasteboard, Class* classArray, size_t len, void* options) {
		void* func = sel_registerName("readObjectsForClasses:options:");

		NSArray* array = c_array_to_NSArray(classArray, len);

		NSArray* output = (NSArray*) ((id(*)(id, SEL, id, void*))objc_msgSend)
			(pasteboard, func, array, options);

		NSRelease(array);
		NSUInteger count = ((NSUInteger(*)(id, SEL))objc_msgSend)(output, sel_registerName("count"));

		const char** res = si_array_init_reserve(sizeof(const char*), count);

		void* path_func = sel_registerName("path");

		for (NSUInteger i = 0; i < count; i++) {
			void* url = NSArray_objectAtIndex(output, i);
			NSString* url_str = ((id(*)(id, SEL))objc_msgSend)(url, path_func);
			res[i] = NSString_to_char(url_str);
		}

		return res;
	}

	void* NSWindow_contentView(NSWindow* window) {
		void* func = sel_registerName("contentView");
		return objc_msgSend_id(window, func);
	}

	/*
		End of cocoa wrapper
	*/

	char* RGFW_mouseIconSrc[] = {"arrowCursor", "arrowCursor", "IBeamCursor", "crosshairCursor", "pointingHandCursor", "resizeLeftRightCursor", "resizeUpDownCursor", "_windowResizeNorthWestSouthEastCursor", "_windowResizeNorthEastSouthWestCursor", "closedHandCursor", "operationNotAllowedCursor"};

	void* RGFWnsglFramework = NULL;

#ifdef RGFW_OPENGL
	void* RGFW_getProcAddress(const char* procname) {
		if (RGFWnsglFramework == NULL)
			RGFWnsglFramework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));

		CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault, procname, kCFStringEncodingASCII);

		void* symbol = CFBundleGetFunctionPointerForName(RGFWnsglFramework, symbolName);

		CFRelease(symbolName);

		return symbol;
	}
#endif

	CVReturn displayCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* inNow, const CVTimeStamp* inOutputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext) { 
		RGFW_UNUSED(displayLink) RGFW_UNUSED(inNow) RGFW_UNUSED(inOutputTime) RGFW_UNUSED(flagsIn) RGFW_UNUSED(flagsOut) RGFW_UNUSED(displayLinkContext)
		return kCVReturnSuccess; 
	}

	id NSWindow_delegate(RGFW_window* win) {
		return (id) objc_msgSend_id(win->src.window, sel_registerName("delegate"));
	}

	u32 RGFW_OnClose(void* self) {
		RGFW_window* win = NULL;
		object_getInstanceVariable(self, "RGFW_window", (void*)&win);
		if (win == NULL)
			return true;

		win->event.type = RGFW_quit;
		RGFW_windowQuitCallback(win);

		return true;
	}

	/* NOTE(EimaMei): Fixes the constant clicking when the app is running under a terminal. */
	bool acceptsFirstResponder(void) { return true; }
	bool performKeyEquivalent(NSEvent* event) { RGFW_UNUSED(event); return true; }

	NSDragOperation draggingEntered(id self, SEL sel, id sender) { 
		RGFW_UNUSED(sender); RGFW_UNUSED(self); RGFW_UNUSED(sel);  

		return NSDragOperationCopy; 
	}
	NSDragOperation draggingUpdated(id self, SEL sel, id sender) { 
		RGFW_UNUSED(sel); 

		RGFW_window* win = NULL;
		object_getInstanceVariable(self, "RGFW_window", (void*)&win);
		if (win == NULL)
			return 0;
		
		if (!(win->_winArgs & RGFW_ALLOW_DND)) {
			return 0;
		}

		win->event.type = RGFW_dnd_init;
		win->src.dndPassed = 0;

		NSPoint p = ((NSPoint(*)(id, SEL)) objc_msgSend)(sender, sel_registerName("draggingLocation"));

		win->event.point = RGFW_POINT((u32) p.x, (u32) (win->r.h - p.y));
		RGFW_dndInitCallback(win, win->event.point);

		return NSDragOperationCopy; 
	}
	bool prepareForDragOperation(id self) {
		RGFW_window* win = NULL;
		object_getInstanceVariable(self, "RGFW_window", (void*)&win);
		if (win == NULL)
			return true;
		
		if (!(win->_winArgs & RGFW_ALLOW_DND)) {
			return false;
		}

		return true;
	}

	void RGFW__osxDraggingEnded(id self, SEL sel, id sender) { RGFW_UNUSED(sender); RGFW_UNUSED(self); RGFW_UNUSED(sel);  return; }

	/* NOTE(EimaMei): Usually, you never need 'id self, SEL cmd' for C -> Obj-C methods. This isn't the case. */
	bool performDragOperation(id self, SEL sel, id sender) {
		RGFW_UNUSED(sender); RGFW_UNUSED(self); RGFW_UNUSED(sel); 

		RGFW_window* win = NULL;
		object_getInstanceVariable(self, "RGFW_window", (void*)&win);

        if (win == NULL)
			return false;

		// NSPasteboard* pasteBoard = objc_msgSend_id(sender, sel_registerName("draggingPasteboard"));

        /////////////////////////////
        id pasteBoard = objc_msgSend_id(sender, sel_registerName("draggingPasteboard"));

        // Get the types of data available on the pasteboard
        id types = objc_msgSend_id(pasteBoard, sel_registerName("types"));

        // Get the string type for file URLs
        id fileURLsType = objc_msgSend_class_char(objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), "NSFilenamesPboardType");

        // Check if the pasteboard contains file URLs
        if (objc_msgSend_id_bool(types, sel_registerName("containsObject:"), fileURLsType) == 0) {
		    #ifdef RGFW_DEBUG
            printf("No files found on the pasteboard.\n");
			#endif

			return 0;
		}

		id fileURLs = objc_msgSend_id_id(pasteBoard, sel_registerName("propertyListForType:"), fileURLsType);
		int count = ((int (*)(id, SEL))objc_msgSend)(fileURLs, sel_registerName("count"));

		if (count == 0)
			return 0;

		for (int i = 0; i < count; i++) {
			id fileURL = objc_msgSend_arr(fileURLs, sel_registerName("objectAtIndex:"), i);
			const char *filePath = ((const char* (*)(id, SEL))objc_msgSend)(fileURL, sel_registerName("UTF8String"));
			// printf("File: %s\n", filePath);
			strncpy(win->event.droppedFiles[i], filePath, RGFW_MAX_PATH);
			win->event.droppedFiles[i][RGFW_MAX_PATH - 1] = '\0';
		}
		win->event.droppedFilesCount = count;

		win->event.type = RGFW_dnd;
		win->src.dndPassed = 0;
		
		NSPoint p = ((NSPoint(*)(id, SEL)) objc_msgSend)(sender, sel_registerName("draggingLocation"));
		win->event.point = RGFW_POINT((u32) p.x, (u32) (win->r.h - p.y));
		
		RGFW_dndCallback(win, win->event.droppedFiles, win->event.droppedFilesCount);
	
    	return false;
	}

	static void NSMoveToResourceDir(void) {
		/* sourced from glfw */
		char resourcesPath[255];

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


	NSSize RGFW__osxWindowResize(void* self, SEL sel, NSSize frameSize) {
		RGFW_UNUSED(sel); 

		RGFW_window* win = NULL;
		object_getInstanceVariable(self, "RGFW_window", (void*)&win);
		if (win == NULL)
			return frameSize;
		
		win->r.w = frameSize.width;
		win->r.h = frameSize.height;
		win->event.type = RGFW_windowResized;
		RGFW_windowResizeCallback(win, win->r);
		return frameSize;
	}

	void RGFW__osxWindowMove(void* self, SEL sel) {
		RGFW_UNUSED(sel); 

		RGFW_window* win = NULL;
		object_getInstanceVariable(self, "RGFW_window", (void*)&win);
		if (win == NULL)
			return;
		
		NSRect frame = ((NSRect(*)(id, SEL))abi_objc_msgSend_stret)(win->src.window, sel_registerName("frame"));
		win->r.x = (i32) frame.origin.x;
		win->r.y = (i32) frame.origin.y;

		win->event.type = RGFW_windowMoved;
		RGFW_windowMoveCallback(win, win->r);
	}

	void RGFW__osxUpdateLayer(void* self, SEL sel) {
		RGFW_UNUSED(sel);

		RGFW_window* win = NULL;
		object_getInstanceVariable(self, "RGFW_window", (void*)&win);
		if (win == NULL)
			return;
		
		win->event.type = RGFW_windowRefresh;
		RGFW_windowRefreshCallback(win);
	}

	RGFWDEF void RGFW_init_buffer(RGFW_window* win);
	void RGFW_init_buffer(RGFW_window* win) {
		#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
			if (RGFW_bufferSize.w == 0 && RGFW_bufferSize.h == 0)
				RGFW_bufferSize = RGFW_getScreenSize();
				
			win->buffer = RGFW_MALLOC(RGFW_bufferSize.w * RGFW_bufferSize.h * 4);

		#ifdef RGFW_OSMESA
				win->src.ctx = OSMesaCreateContext(OSMESA_RGBA, NULL);
				OSMesaMakeCurrent(win->src.ctx, win->buffer, GL_UNSIGNED_BYTE, win->r.w, win->r.h);
		#endif
		#else
		RGFW_UNUSED(win); /*!< if buffer rendering is not being used */
		#endif
	}

	NSPasteboardType const NSPasteboardTypeURL = "public.url";
	NSPasteboardType const NSPasteboardTypeFileURL  = "public.file-url";

	RGFW_window* RGFW_createWindow(const char* name, RGFW_rect rect, u16 args) {
		static u8 RGFW_loaded = 0;

		/* NOTE(EimaMei): Why does Apple hate good code? Like wtf, who thought of methods being a great idea???
		Imagine a universe, where MacOS had a proper system API (we would probably have like 20% better performance).
		*/
		si_func_to_SEL_with_name("NSObject", "windowShouldClose", RGFW_OnClose);

		/* NOTE(EimaMei): Fixes the 'Boop' sfx from constantly playing each time you click a key. Only a problem when running in the terminal. */
		si_func_to_SEL("NSWindow", acceptsFirstResponder);
		si_func_to_SEL("NSWindow", performKeyEquivalent);

		// RR Create an autorelease pool
		id pool = objc_msgSend_class(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc"));
		pool = objc_msgSend_id(pool, sel_registerName("init"));

		if (NSApp == NULL) {
			NSApp = objc_msgSend_id((id)objc_getClass("NSApplication"), sel_registerName("sharedApplication"));

			((void (*)(id, SEL, NSUInteger))objc_msgSend)
				(NSApp, sel_registerName("setActivationPolicy:"), NSApplicationActivationPolicyRegular);
		}

		RGFW_window* win = RGFW_window_basic_init(rect, args);
		
		RGFW_window_setMouseDefault(win);

		NSRect windowRect;
		windowRect.origin.x = win->r.x;
		windowRect.origin.y = win->r.y;
		windowRect.size.width = win->r.w;
		windowRect.size.height = win->r.h;

		NSBackingStoreType macArgs = NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSBackingStoreBuffered | NSWindowStyleMaskTitled;

		if (!(args & RGFW_NO_RESIZE))
			macArgs |= NSWindowStyleMaskResizable;
		if (!(args & RGFW_NO_BORDER))
			macArgs |= NSWindowStyleMaskTitled;
		else
			macArgs = NSWindowStyleMaskBorderless;
		{
			void* nsclass = objc_getClass("NSWindow");
			void* func = sel_registerName("initWithContentRect:styleMask:backing:defer:");

			win->src.window = ((id(*)(id, SEL, NSRect, NSWindowStyleMask, NSBackingStoreType, bool))objc_msgSend)
				(NSAlloc(nsclass), func, windowRect, macArgs, macArgs, false);
		}

		NSString* str = NSString_stringWithUTF8String(name);
		objc_msgSend_void_id(win->src.window, sel_registerName("setTitle:"), str);

#ifdef RGFW_OPENGL
	if ((args & RGFW_NO_INIT_API) == 0) {
		void* attrs = RGFW_initFormatAttribs(args & RGFW_OPENGL_SOFTWARE);
		void* format = NSOpenGLPixelFormat_initWithAttributes(attrs);

		if (format == NULL) {
			printf("Failed to load pixel format for OpenGL\n");

			void* attrs = RGFW_initFormatAttribs(1);
			format = NSOpenGLPixelFormat_initWithAttributes(attrs);
			if (format == NULL)
				printf("and loading software rendering OpenGL failed\n");
			else
				printf("Switching to software rendering\n");
		}
		
		/* the pixel format can be passed directly to opengl context creation to create a context 
			this is because the format also includes information about the opengl version (which may be a bad thing) */
		win->src.view = NSOpenGLView_initWithFrame((NSRect){{0, 0}, {win->r.w, win->r.h}}, format);
		objc_msgSend_void(win->src.view, sel_registerName("prepareOpenGL"));
		win->src.ctx = objc_msgSend_id(win->src.view, sel_registerName("openGLContext"));
	} else
#endif
	{
		NSRect contentRect = (NSRect){{0, 0}, {win->r.w, win->r.h}};
		win->src.view = ((id(*)(id, SEL, NSRect))objc_msgSend)
			(NSAlloc((id)objc_getClass("NSView")), sel_registerName("initWithFrame:"),
				contentRect);
	}

		void* contentView = NSWindow_contentView(win->src.window);
		objc_msgSend_void_bool(contentView, sel_registerName("setWantsLayer:"), true);

		objc_msgSend_void_id(win->src.window, sel_registerName("setContentView:"), win->src.view);

#ifdef RGFW_OPENGL
		if ((args & RGFW_NO_INIT_API) == 0)
			objc_msgSend_void(win->src.ctx, sel_registerName("makeCurrentContext"));
#endif
		if (args & RGFW_TRANSPARENT_WINDOW) {
#ifdef RGFW_OPENGL
		if ((args & RGFW_NO_INIT_API) == 0) {
			i32 opacity = 0;
			#define NSOpenGLCPSurfaceOpacity 236
			NSOpenGLContext_setValues(win->src.ctx, &opacity, NSOpenGLCPSurfaceOpacity);
		}
#endif

			objc_msgSend_void_bool(win->src.window, sel_registerName("setOpaque:"), false);

			objc_msgSend_void_id(win->src.window, sel_registerName("setBackgroundColor:"),
				NSColor_colorWithSRGB(0, 0, 0, 0));
		}

		win->src.display = CGMainDisplayID();
		CVDisplayLinkCreateWithCGDisplay(win->src.display, (CVDisplayLinkRef*)&win->src.displayLink);
		CVDisplayLinkSetOutputCallback(win->src.displayLink, displayCallback, win);
		CVDisplayLinkStart(win->src.displayLink);

		RGFW_init_buffer(win);

		#ifndef RGFW_NO_MONITOR
		if (args & RGFW_SCALE_TO_MONITOR)
			RGFW_window_scaleToMonitor(win);
		#endif

		if (args & RGFW_HIDE_MOUSE)
			RGFW_window_showMouse(win, 0);

		if (args & RGFW_COCOA_MOVE_TO_RESOURCE_DIR)
			NSMoveToResourceDir();

		Class delegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "WindowDelegate", 0);

		class_addIvar(
			delegateClass, "RGFW_window",
			sizeof(RGFW_window*), rint(log2(sizeof(RGFW_window*))),
			"L"
		);

		class_addMethod(delegateClass, sel_registerName("windowWillResize:toSize:"), (IMP) RGFW__osxWindowResize, "{NSSize=ff}@:{NSSize=ff}");
		class_addMethod(delegateClass, sel_registerName("updateLayer:"), (IMP) RGFW__osxUpdateLayer, "");
		class_addMethod(delegateClass, sel_registerName("windowWillMove:"), (IMP) RGFW__osxWindowMove, "");
		class_addMethod(delegateClass, sel_registerName("windowDidMove:"), (IMP) RGFW__osxWindowMove, "");
		class_addMethod(delegateClass, sel_registerName("draggingEntered:"), (IMP)draggingEntered, "l@:@");
		class_addMethod(delegateClass, sel_registerName("draggingUpdated:"), (IMP)draggingUpdated, "l@:@");
		class_addMethod(delegateClass, sel_registerName("draggingExited:"), (IMP)RGFW__osxDraggingEnded, "v@:@");
		class_addMethod(delegateClass, sel_registerName("draggingEnded:"), (IMP)RGFW__osxDraggingEnded, "v@:@");
		class_addMethod(delegateClass, sel_registerName("prepareForDragOperation:"), (IMP)prepareForDragOperation, "B@:@");
		class_addMethod(delegateClass, sel_registerName("performDragOperation:"), (IMP)performDragOperation, "B@:@");

		id delegate = objc_msgSend_id(NSAlloc(delegateClass), sel_registerName("init"));

		object_setInstanceVariable(delegate, "RGFW_window", win);

		objc_msgSend_void_id(win->src.window, sel_registerName("setDelegate:"), delegate);

		if (args & RGFW_ALLOW_DND) {
			win->_winArgs |= RGFW_ALLOW_DND;

			NSPasteboardType types[] = {NSPasteboardTypeURL, NSPasteboardTypeFileURL, NSPasteboardTypeString};
			NSregisterForDraggedTypes(win->src.window, types, 3);
		}

		// Show the window
		objc_msgSend_void_bool(NSApp, sel_registerName("activateIgnoringOtherApps:"), true);
		((id(*)(id, SEL, SEL))objc_msgSend)(win->src.window, sel_registerName("makeKeyAndOrderFront:"), NULL);
		objc_msgSend_void_bool(win->src.window, sel_registerName("setIsVisible:"), true);

		if (!RGFW_loaded) {
			objc_msgSend_void(win->src.window, sel_registerName("makeMainWindow"));

			RGFW_loaded = 1;
		}

		objc_msgSend_void(win->src.window, sel_registerName("makeKeyWindow"));

		objc_msgSend_void(NSApp, sel_registerName("finishLaunching"));

		if (RGFW_root == NULL)
			RGFW_root = win;

		NSRetain(win->src.window);
		NSRetain(NSApp);

		return win;
	}

	void RGFW_window_setBorder(RGFW_window* win, u8 border) {
		NSBackingStoreType storeType = NSWindowStyleMaskBorderless;
		if (!border) {
			storeType = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
		}
		if (!(win->_winArgs & RGFW_NO_RESIZE)) {
			storeType |= NSWindowStyleMaskResizable;
		}
		
		((void (*)(id, SEL, NSBackingStoreType))objc_msgSend)(win->src.window, sel_registerName("setStyleMask:"), storeType);

		objc_msgSend_void_bool(win->src.window, sel_registerName("setHasShadow:"), border);
	}

	RGFW_area RGFW_getScreenSize(void) {
		static CGDirectDisplayID display = 0;

		if (display == 0)
			display = CGMainDisplayID();

		return RGFW_AREA(CGDisplayPixelsWide(display), CGDisplayPixelsHigh(display));
	}

	RGFW_point RGFW_getGlobalMousePoint(void) {
		assert(RGFW_root != NULL);

		CGEventRef e = CGEventCreate(NULL);
		CGPoint point = CGEventGetLocation(e);
		CFRelease(e);

		return RGFW_POINT((u32) point.x, (u32) point.y); /*!< the point is loaded during event checks */
	}

	RGFW_point RGFW_window_getMousePoint(RGFW_window* win) {
		NSPoint p =  ((NSPoint(*)(id, SEL)) objc_msgSend)(win->src.window, sel_registerName("mouseLocationOutsideOfEventStream"));

		return RGFW_POINT((u32) p.x, (u32) (win->r.h - p.y));
	}

	u32 RGFW_keysPressed[10]; /*10 keys at a time*/
	typedef NS_ENUM(u32, NSEventType) {        /* various types of events */
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
			NSEventTypeGesture API_AVAILABLE(macos(10.5)) = 29,
			NSEventTypeMagnify API_AVAILABLE(macos(10.5)) = 30,
			NSEventTypeSwipe   API_AVAILABLE(macos(10.5)) = 31,
			NSEventTypeRotate  API_AVAILABLE(macos(10.5)) = 18,
			NSEventTypeBeginGesture API_AVAILABLE(macos(10.5)) = 19,
			NSEventTypeEndGesture API_AVAILABLE(macos(10.5)) = 20,

			NSEventTypeSmartMagnify API_AVAILABLE(macos(10.8)) = 32,
			NSEventTypeQuickLook API_AVAILABLE(macos(10.8)) = 33,

			NSEventTypePressure API_AVAILABLE(macos(10.10.3)) = 34,
			NSEventTypeDirectTouch API_AVAILABLE(macos(10.10)) = 37,

			NSEventTypeChangeMode API_AVAILABLE(macos(10.15)) = 38,
	};

	typedef NS_ENUM(unsigned long long, NSEventMask) { /* masks for the types of events */
		NSEventMaskLeftMouseDown = 1ULL << NSEventTypeLeftMouseDown,
			NSEventMaskLeftMouseUp = 1ULL << NSEventTypeLeftMouseUp,
			NSEventMaskRightMouseDown = 1ULL << NSEventTypeRightMouseDown,
			NSEventMaskRightMouseUp = 1ULL << NSEventTypeRightMouseUp,
			NSEventMaskMouseMoved = 1ULL << NSEventTypeMouseMoved,
			NSEventMaskLeftMouseDragged = 1ULL << NSEventTypeLeftMouseDragged,
			NSEventMaskRightMouseDragged = 1ULL << NSEventTypeRightMouseDragged,
			NSEventMaskMouseEntered = 1ULL << NSEventTypeMouseEntered,
			NSEventMaskMouseExited = 1ULL << NSEventTypeMouseExited,
			NSEventMaskKeyDown = 1ULL << NSEventTypeKeyDown,
			NSEventMaskKeyUp = 1ULL << NSEventTypeKeyUp,
			NSEventMaskFlagsChanged = 1ULL << NSEventTypeFlagsChanged,
			NSEventMaskAppKitDefined = 1ULL << NSEventTypeAppKitDefined,
			NSEventMaskSystemDefined = 1ULL << NSEventTypeSystemDefined,
			NSEventMaskApplicationDefined = 1ULL << NSEventTypeApplicationDefined,
			NSEventMaskPeriodic = 1ULL << NSEventTypePeriodic,
			NSEventMaskCursorUpdate = 1ULL << NSEventTypeCursorUpdate,
			NSEventMaskScrollWheel = 1ULL << NSEventTypeScrollWheel,
			NSEventMaskTabletPoint = 1ULL << NSEventTypeTabletPoint,
			NSEventMaskTabletProximity = 1ULL << NSEventTypeTabletProximity,
			NSEventMaskOtherMouseDown = 1ULL << NSEventTypeOtherMouseDown,
			NSEventMaskOtherMouseUp = 1ULL << NSEventTypeOtherMouseUp,
			NSEventMaskOtherMouseDragged = 1ULL << NSEventTypeOtherMouseDragged,
			/* The following event masks are available on some hardware on 10.5.2 and later */
			NSEventMaskGesture API_AVAILABLE(macos(10.5)) = 1ULL << NSEventTypeGesture,
			NSEventMaskMagnify API_AVAILABLE(macos(10.5)) = 1ULL << NSEventTypeMagnify,
			NSEventMaskSwipe API_AVAILABLE(macos(10.5)) = 1ULL << NSEventTypeSwipe,
			NSEventMaskRotate API_AVAILABLE(macos(10.5)) = 1ULL << NSEventTypeRotate,
			NSEventMaskBeginGesture API_AVAILABLE(macos(10.5)) = 1ULL << NSEventTypeBeginGesture,
			NSEventMaskEndGesture API_AVAILABLE(macos(10.5)) = 1ULL << NSEventTypeEndGesture,

			/* Note: You can only use these event masks on 64 bit. In other words, you cannot setup a local, nor global, event monitor for these event types on 32 bit. Also, you cannot search the event queue for them (nextEventMatchingMask:...) on 32 bit.
			 */
			NSEventMaskSmartMagnify API_AVAILABLE(macos(10.8)) = 1ULL << NSEventTypeSmartMagnify,
			NSEventMaskPressure API_AVAILABLE(macos(10.10.3)) = 1ULL << NSEventTypePressure,
			NSEventMaskDirectTouch API_AVAILABLE(macos(10.12.2)) = 1ULL << NSEventTypeDirectTouch,

			NSEventMaskChangeMode API_AVAILABLE(macos(10.15)) = 1ULL << NSEventTypeChangeMode,

			NSEventMaskAny = ULONG_MAX,

	};

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

		NSEvent* e = (NSEvent*) ((id(*)(id, SEL, NSEventType, NSPoint, NSEventModifierFlags, void*, NSInteger, void**, short, NSInteger, NSInteger))objc_msgSend)
			(NSApp, sel_registerName("otherEventWithType:location:modifierFlags:timestamp:windowNumber:context:subtype:data1:data2:"), 
				NSEventTypeApplicationDefined, (NSPoint){0, 0}, 0, 0, 0, NULL, 0, 0, 0);

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

		NSEvent* e = (NSEvent*) ((id(*)(id, SEL, NSEventMask, void*, NSString*, bool))objc_msgSend)
			(NSApp, sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"), 
				ULONG_MAX, date, NSString_stringWithUTF8String("kCFRunLoopDefaultMode"), true);


		if (e) {
			objc_msgSend_void_id(NSApp, sel_registerName("sendEvent:"), e);
		}

		objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
	}

	RGFW_Event* RGFW_window_checkEvent(RGFW_window* win) {
		assert(win != NULL);
		
		if (win->event.type == RGFW_quit)
			return NULL;
		
		if ((win->event.type == RGFW_dnd || win->event.type == RGFW_dnd_init) && win->src.dndPassed == 0) {
			win->src.dndPassed = 1;
			return &win->event;
		}

		id eventPool = objc_msgSend_class(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc"));
        eventPool = objc_msgSend_id(eventPool, sel_registerName("init"));

		static void* eventFunc = NULL;
		if (eventFunc == NULL) 
			eventFunc = sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:");

		if ((win->event.type == RGFW_windowMoved || win->event.type == RGFW_windowResized || win->event.type == RGFW_windowRefresh) && win->event.keyCode != 120) {
			win->event.keyCode = 120;
			objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
			return &win->event;
		}

		void* date = NULL;

		NSEvent* e = (NSEvent*) ((id(*)(id, SEL, NSEventMask, void*, NSString*, bool))objc_msgSend)
			(NSApp, eventFunc, ULONG_MAX, date, NSString_stringWithUTF8String("kCFRunLoopDefaultMode"), true);

		if (e == NULL) {
			objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
			return NULL;
		}
		
		if (objc_msgSend_id(e, sel_registerName("window")) != win->src.window) {
			((void (*)(id, SEL, id, bool))objc_msgSend)
				(NSApp, sel_registerName("postEvent:atStart:"), e, 0);
						
			objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
			return NULL;
		}

		if (win->event.droppedFilesCount) {
			u32 i;
			for (i = 0; i < win->event.droppedFilesCount; i++)
				win->event.droppedFiles[i][0] = '\0';
		}

		win->event.droppedFilesCount = 0;
		win->event.type = 0;
		
		switch (objc_msgSend_uint(e, sel_registerName("type"))) {
			case NSEventTypeMouseEntered: {
				win->event.type = RGFW_mouseEnter;
				NSPoint p = ((NSPoint(*)(id, SEL)) objc_msgSend)(e, sel_registerName("locationInWindow"));

				win->event.point = RGFW_POINT((i32) p.x, (i32) (win->r.h - p.y));
				RGFW_mouseNotifyCallBack(win, win->event.point, 1);
				break;
			}
			
			case NSEventTypeMouseExited:
				win->event.type = RGFW_mouseLeave;
				RGFW_mouseNotifyCallBack(win, win->event.point, 0);
				break;

			case NSEventTypeKeyDown: {
				u32 key = (u16) objc_msgSend_uint(e, sel_registerName("keyCode"));
				win->event.keyCode = RGFW_apiKeyCodeToRGFW(key);
				RGFW_keyboard[win->event.keyCode].prev = RGFW_keyboard[win->event.keyCode].current;

				win->event.type = RGFW_keyPressed;
				char* str = (char*)(const char*) NSString_to_char(objc_msgSend_id(e, sel_registerName("characters")));
				strncpy(win->event.keyName, str, 16);
				win->event.repeat = RGFW_isPressed(win, win->event.keyCode);
				RGFW_keyboard[win->event.keyCode].current = 1;

				RGFW_keyCallback(win, win->event.keyCode, win->event.keyName, win->event.lockState, 1);
				break;
			}

			case NSEventTypeKeyUp: {
				u32 key = (u16) objc_msgSend_uint(e, sel_registerName("keyCode"));
				win->event.keyCode = RGFW_apiKeyCodeToRGFW(key);;

				RGFW_keyboard[win->event.keyCode].prev = RGFW_keyboard[win->event.keyCode].current;

				win->event.type = RGFW_keyReleased;
				char* str = (char*)(const char*) NSString_to_char(objc_msgSend_id(e, sel_registerName("characters")));
				strncpy(win->event.keyName, str, 16);

				RGFW_keyboard[win->event.keyCode].current = 0;
				RGFW_keyCallback(win, win->event.keyCode, win->event.keyName, win->event.lockState, 0);
				break;
			}

			case NSEventTypeFlagsChanged: {
				u32 flags = objc_msgSend_uint(e, sel_registerName("modifierFlags"));
				RGFW_updateLockState(win, ((u32)(flags & NSEventModifierFlagCapsLock) % 255), ((flags & NSEventModifierFlagNumericPad) % 255));
				
				u8 i;
				for (i = 0; i < 9; i++)
					RGFW_keyboard[i + RGFW_CapsLock].prev = 0;
				
				for (i = 0; i < 5; i++) {
					u32 shift = (1 << (i + 16));
					u32 key = i + RGFW_CapsLock;

					if ((flags & shift) && !RGFW_wasPressed(win, key)) {
						RGFW_keyboard[key].current = 1;

						if (key != RGFW_CapsLock)
							RGFW_keyboard[key+ 4].current = 1;
						
						win->event.type = RGFW_keyPressed;
						win->event.keyCode = key;
						break;
					} 
					
					if (!(flags & shift) && RGFW_wasPressed(win, key)) {
						RGFW_keyboard[key].current = 0;
						
						if (key != RGFW_CapsLock)
							RGFW_keyboard[key + 4].current = 0;

						win->event.type = RGFW_keyReleased;
						win->event.keyCode = key;
						break;
					}
				}

				RGFW_keyCallback(win, win->event.keyCode, win->event.keyName, win->event.lockState, win->event.type == RGFW_keyPressed);

				break;
			}
			case NSEventTypeLeftMouseDragged:
			case NSEventTypeOtherMouseDragged:
			case NSEventTypeRightMouseDragged:
			case NSEventTypeMouseMoved:
				win->event.type = RGFW_mousePosChanged;
				NSPoint p = ((NSPoint(*)(id, SEL)) objc_msgSend)(e, sel_registerName("locationInWindow"));
				win->event.point = RGFW_POINT((u32) p.x, (u32) (win->r.h - p.y));

				if ((win->_winArgs & RGFW_HOLD_MOUSE)) {
					p.x = ((CGFloat(*)(id, SEL))abi_objc_msgSend_fpret)(e, sel_registerName("deltaX"));
					p.y = ((CGFloat(*)(id, SEL))abi_objc_msgSend_fpret)(e, sel_registerName("deltaY"));
					
					win->event.point = RGFW_POINT((i32)p.x, (i32)p.y);
				}

				RGFW_mousePosCallback(win, win->event.point);
				break;

			case NSEventTypeLeftMouseDown:
				win->event.button = RGFW_mouseLeft;
				win->event.type = RGFW_mouseButtonPressed;
				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 1;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 1);
				break;

			case NSEventTypeOtherMouseDown:
				win->event.button = RGFW_mouseMiddle;
				win->event.type = RGFW_mouseButtonPressed;
				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 1;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 1);
				break;

			case NSEventTypeRightMouseDown:
				win->event.button = RGFW_mouseRight;
				win->event.type = RGFW_mouseButtonPressed;
				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 1;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 1);
				break;

			case NSEventTypeLeftMouseUp:
				win->event.button = RGFW_mouseLeft;
				win->event.type = RGFW_mouseButtonReleased;
				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 0;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 0);
				break;

			case NSEventTypeOtherMouseUp:
				win->event.button = RGFW_mouseMiddle;
				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 0;
				win->event.type = RGFW_mouseButtonReleased;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 0);
				break;

			case NSEventTypeRightMouseUp:
				win->event.button = RGFW_mouseRight;
				RGFW_mouseButtons[win->event.button].prev = RGFW_mouseButtons[win->event.button].current;
				RGFW_mouseButtons[win->event.button].current = 0;
				win->event.type = RGFW_mouseButtonReleased;
				RGFW_mouseButtonCallback(win, win->event.button, win->event.scroll, 0);
				break;

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
				break;
		}

		objc_msgSend_void_id(NSApp, sel_registerName("sendEvent:"), e);
		((void(*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("updateWindows"));
				
		objc_msgSend_bool_void(eventPool, sel_registerName("drain"));
		return &win->event;
	}


	void RGFW_window_move(RGFW_window* win, RGFW_point v) {
		assert(win != NULL);

		win->r.x = v.x;
		win->r.y = v.y;
		((void(*)(id, SEL, NSRect, bool, bool))objc_msgSend)
			(win->src.window, sel_registerName("setFrame:display:animate:"), (NSRect){{win->r.x, win->r.y}, {win->r.w, win->r.h}}, true, true);
	}

	void RGFW_window_resize(RGFW_window* win, RGFW_area a) {
		assert(win != NULL);

		win->r.w = a.w;
		win->r.h = a.h;
		((void(*)(id, SEL, NSRect, bool, bool))objc_msgSend)
			(win->src.window, sel_registerName("setFrame:display:animate:"), (NSRect){{win->r.x, win->r.y}, {win->r.w, win->r.h}}, true, true);
	}

	void RGFW_window_minimize(RGFW_window* win) {
		assert(win != NULL);

		objc_msgSend_void_SEL(win->src.window, sel_registerName("performMiniaturize:"), NULL);
	}

	void RGFW_window_restore(RGFW_window* win) {
		assert(win != NULL);

		objc_msgSend_void_SEL(win->src.window, sel_registerName("deminiaturize:"), NULL);
	}

	void RGFW_window_setName(RGFW_window* win, char* name) {
		assert(win != NULL);

		NSString* str = NSString_stringWithUTF8String(name);
		objc_msgSend_void_id(win->src.window, sel_registerName("setTitle:"), str);
	}

	#ifndef RGFW_NO_PASSTHROUGH
	void RGFW_window_setMousePassthrough(RGFW_window* win, b8 passthrough) {
		objc_msgSend_void_bool(win->src.window, sel_registerName("setIgnoresMouseEvents:"), passthrough);
	}
	#endif

	void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a) {
		if (a.w == 0 && a.h == 0)
			return;

		((void (*)(id, SEL, NSSize))objc_msgSend)
			(win->src.window, sel_registerName("setMinSize:"), (NSSize){a.w, a.h});
	}

	void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a) {
		if (a.w == 0 && a.h == 0)
			return;

		((void (*)(id, SEL, NSSize))objc_msgSend)
			(win->src.window, sel_registerName("setMaxSize:"), (NSSize){a.w, a.h});
	}

	void RGFW_window_setIcon(RGFW_window* win, u8* data, RGFW_area area, i32 channels) {
		assert(win != NULL);

		/* code by EimaMei  */
		// Make a bitmap representation, then copy the loaded image into it.
		void* representation = NSBitmapImageRep_initWithBitmapData(NULL, area.w, area.h, 8, channels, (channels == 4), false, "NSCalibratedRGBColorSpace", 1 << 1, area.w * channels, 8 * channels);
		memcpy(NSBitmapImageRep_bitmapData(representation), data, area.w * area.h * channels);

		// Add ze representation.
		void* dock_image = NSImage_initWithSize((NSSize){area.w, area.h});
		NSImage_addRepresentation(dock_image, (void*) representation);

		// Finally, set the dock image to it.
		objc_msgSend_void_id(NSApp, sel_registerName("setApplicationIconImage:"), dock_image);
		// Free the garbage.
		release(dock_image);
		release(representation);
	}

	NSCursor* NSCursor_arrowStr(char* str) {
		void* nclass = objc_getClass("NSCursor");
		void* func = sel_registerName(str);
		return (NSCursor*) objc_msgSend_id(nclass, func);
	}

	void RGFW_window_setMouse(RGFW_window* win, u8* image, RGFW_area a, i32 channels) {
		assert(win != NULL);

		if (image == NULL) {
			objc_msgSend_void(NSCursor_arrowStr("arrowCursor"), sel_registerName("set"));
			return;
		}

		/* NOTE(EimaMei): Code by yours truly. */
		// Make a bitmap representation, then copy the loaded image into it.
		void* representation = NSBitmapImageRep_initWithBitmapData(NULL, a.w, a.h, 8, channels, (channels == 4), false, "NSCalibratedRGBColorSpace", 1 << 1, a.w * channels, 8 * channels);
		memcpy(NSBitmapImageRep_bitmapData(representation), image, a.w * a.h * channels);

		// Add ze representation.
		void* cursor_image = NSImage_initWithSize((NSSize){a.w, a.h});
		NSImage_addRepresentation(cursor_image, representation);

		// Finally, set the cursor image.
		void* cursor = NSCursor_initWithImage(cursor_image, (NSPoint){0.0, 0.0});

		objc_msgSend_void(cursor, sel_registerName("set"));

		// Free the garbage.
		release(cursor_image);
		release(representation);
	}

	void RGFW_window_setMouseDefault(RGFW_window* win) {
		RGFW_window_setMouseStandard(win, RGFW_MOUSE_ARROW);
	}

	void RGFW_window_showMouse(RGFW_window* win, i8 show) {
		RGFW_UNUSED(win);

		if (show) {
			CGDisplayShowCursor(kCGDirectMainDisplay);
		}
		else {
			CGDisplayHideCursor(kCGDirectMainDisplay);
		}
	}

	void RGFW_window_setMouseStandard(RGFW_window* win, u8 stdMouses) {
		if (stdMouses > ((sizeof(RGFW_mouseIconSrc)) / (sizeof(char*))))
			return;
		
		char* mouseStr = RGFW_mouseIconSrc[stdMouses];
		void* mouse = NSCursor_arrowStr(mouseStr);

		if (mouse == NULL)
			return;

		RGFW_UNUSED(win);
		CGDisplayShowCursor(kCGDirectMainDisplay);
		objc_msgSend_void(mouse, sel_registerName("set"));
	}
	
	void RGFW_releaseCursor(RGFW_window* win) {
		CGAssociateMouseAndMouseCursorPosition(1);	
	}

	void RGFW_captureCursor(RGFW_window* win, RGFW_rect r) { 
		RGFW_UNUSED(win)

		CGWarpMouseCursorPosition(CGPointMake(r.x + (r.w / 2), r.y + (r.h / 2)));
		CGAssociateMouseAndMouseCursorPosition(0);
	}

	void RGFW_window_moveMouse(RGFW_window* win, RGFW_point v) {
		RGFW_UNUSED(win);
		
		CGWarpMouseCursorPosition(CGPointMake(v.x, v.y));		
	}


	void RGFW_window_hide(RGFW_window* win) {
		objc_msgSend_void_bool(win->src.window, sel_registerName("setIsVisible:"), false);
	}

	void RGFW_window_show(RGFW_window* win) {
		((id(*)(id, SEL, SEL))objc_msgSend)(win->src.window, sel_registerName("makeKeyAndOrderFront:"), NULL);
		objc_msgSend_void_bool(win->src.window, sel_registerName("setIsVisible:"), true);
	}

	u8 RGFW_window_isFullscreen(RGFW_window* win) {
		assert(win != NULL);

		NSWindowStyleMask mask = (NSWindowStyleMask) objc_msgSend_uint(win->src.window, sel_registerName("styleMask"));
		return (mask & NSWindowStyleMaskFullScreen) == NSWindowStyleMaskFullScreen;
	}

	u8 RGFW_window_isHidden(RGFW_window* win) {
		assert(win != NULL);

		bool visible = objc_msgSend_bool(win->src.window, sel_registerName("isVisible"));
		return visible == NO && !RGFW_window_isMinimized(win);
	}

	u8 RGFW_window_isMinimized(RGFW_window* win) {
		assert(win != NULL);

		return objc_msgSend_bool(win->src.window, sel_registerName("isMiniaturized")) == YES;
	}

	u8 RGFW_window_isMaximized(RGFW_window* win) {
		assert(win != NULL);

		return objc_msgSend_bool(win->src.window, sel_registerName("isZoomed"));
	}

	static RGFW_monitor RGFW_NSCreateMonitor(CGDirectDisplayID display) {
		RGFW_monitor monitor;

		CGRect bounds = CGDisplayBounds(display);
		monitor.rect = RGFW_RECT((int) bounds.origin.x, (int) bounds.origin.y, (int) bounds.size.width, (int) bounds.size.height);

		CGSize screenSizeMM = CGDisplayScreenSize(display);
		monitor.physW = screenSizeMM.width / 25.4;
		monitor.physH = screenSizeMM.height / 25.4;

		monitor.scaleX = (monitor.rect.w / (screenSizeMM.width)) / 2.6;
		monitor.scaleY = (monitor.rect.h / (screenSizeMM.height)) / 2.6;

		snprintf(monitor.name, 128, "%i %i %i", CGDisplayModelNumber(display), CGDisplayVendorNumber(display), CGDisplaySerialNumber(display));

		return monitor;
	}


	static RGFW_monitor RGFW_monitors[7];

	RGFW_monitor* RGFW_getMonitors(void) {
		static CGDirectDisplayID displays[7];
		u32 count;

		if (CGGetActiveDisplayList(6, displays, &count) != kCGErrorSuccess)
			return NULL;

		for (u32 i = 0; i < count; i++)
			RGFW_monitors[i] = RGFW_NSCreateMonitor(displays[i]);

		return RGFW_monitors;
	}

	RGFW_monitor RGFW_getPrimaryMonitor(void) {
		CGDirectDisplayID primary = CGMainDisplayID();
		return RGFW_NSCreateMonitor(primary);
	}

	RGFW_monitor RGFW_window_getMonitor(RGFW_window* win) {
		return RGFW_NSCreateMonitor(win->src.display);
	}

	char* RGFW_readClipboard(size_t* size) {
		char* clip = (char*)NSPasteboard_stringForType(NSPasteboard_generalPasteboard(), NSPasteboardTypeString);
		
		size_t clip_len = 1;

		if (clip != NULL) {
			clip_len = strlen(clip) + 1; 
		}

		char* str = (char*)RGFW_MALLOC(sizeof(char) * clip_len);
		
		if (clip != NULL) {
			strncpy(str, clip, clip_len);
		}

		str[clip_len] = '\0';
		
		if (size != NULL)
			*size = clip_len;
		return str;
	}

	void RGFW_writeClipboard(const char* text, u32 textLen) {
		RGFW_UNUSED(textLen);

		NSPasteboardType array[] = { NSPasteboardTypeString, NULL };
		NSPasteBoard_declareTypes(NSPasteboard_generalPasteboard(), array, 1, NULL);

		NSPasteBoard_setString(NSPasteboard_generalPasteboard(), text, NSPasteboardTypeString);
	}

	u16 RGFW_registerJoystick(RGFW_window* win, i32 jsNumber) {
		RGFW_UNUSED(jsNumber);

		assert(win != NULL);

		return RGFW_registerJoystickF(win, (char*) "");
	}

	u16 RGFW_registerJoystickF(RGFW_window* win, char* file) {
		RGFW_UNUSED(file);

		assert(win != NULL);

		return RGFW_joystickCount - 1;
	}

	#ifdef RGFW_OPENGL
	void RGFW_window_makeCurrent_OpenGL(RGFW_window* win) {
		assert(win != NULL);
		objc_msgSend_void(win->src.ctx, sel_registerName("makeCurrentContext"));
	}
	#endif

	#if !defined(RGFW_EGL)
	void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) {
		assert(win != NULL);
		#if defined(RGFW_OPENGL)
		
		NSOpenGLContext_setValues(win->src.ctx, &swapInterval, 222);
		#endif
	}
	#endif
	
	// Function to create a CGImageRef from an array of bytes
	CGImageRef createImageFromBytes(unsigned char *buffer, int width, int height)
	{
		// Define color space
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        // Create bitmap context
        CGContextRef context = CGBitmapContextCreate(
        		buffer, 
        		width, height,
        		8,
        		RGFW_bufferSize.w * 4, 
        		colorSpace,
        		kCGImageAlphaPremultipliedLast);
        // Create image from bitmap context
        CGImageRef image = CGBitmapContextCreateImage(context);
        // Release the color space and context
        CGColorSpaceRelease(colorSpace);
        CGContextRelease(context);
                         
        return image;
    }

	void RGFW_window_swapBuffers(RGFW_window* win) {
		assert(win != NULL);
		/* clear the window*/

		if (!(win->_winArgs & RGFW_NO_CPU_RENDER)) {
#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
			#ifdef RGFW_OSMESA
			RGFW_OSMesa_reorganize();
			#endif

			void* view = NSWindow_contentView(win->src.window);
			void* layer = objc_msgSend_id(view, sel_registerName("layer"));

			((void(*)(id, SEL, NSRect))objc_msgSend)(layer,
				sel_registerName("setFrame:"),
				(NSRect){{0, 0}, {win->r.w, win->r.h}});

            CGImageRef image = createImageFromBytes(win->buffer, win->r.w, win->r.h);
            // Get the current graphics context
            id graphicsContext = objc_msgSend_class(objc_getClass("NSGraphicsContext"), sel_registerName("currentContext"));
            // Get the CGContext from the current NSGraphicsContext
            id cgContext = objc_msgSend_id(graphicsContext, sel_registerName("graphicsPort"));
			// Draw the image in the context
			NSRect bounds = (NSRect){{0,0}, {win->r.w, win->r.h}};
		    CGContextDrawImage((void*)cgContext, *(CGRect*)&bounds, image);
          	// Flush the graphics context to ensure the drawing is displayed
            objc_msgSend_id(graphicsContext, sel_registerName("flushGraphics"));
            
            objc_msgSend_void_id(layer, sel_registerName("setContents:"), (id)image);
            objc_msgSend_id(layer, sel_registerName("setNeedsDisplay"));
            
            CGImageRelease(image);
#endif
		}

		if (!(win->_winArgs & RGFW_NO_GPU_RENDER)) {
			#ifdef RGFW_EGL
					eglSwapBuffers(win->src.EGL_display, win->src.EGL_surface);
			#elif defined(RGFW_OPENGL)
					objc_msgSend_void(win->src.ctx, sel_registerName("flushBuffer"));
			#endif
		}
	}

	void RGFW_window_close(RGFW_window* win) {
		assert(win != NULL);
		release(win->src.view);

#ifdef RGFW_ALLOC_DROPFILES
		{
			u32 i;
			for (i = 0; i < RGFW_MAX_DROPS; i++)
				RGFW_FREE(win->event.droppedFiles[i]);


			RGFW_FREE(win->event.droppedFiles);
		}
#endif
	
#ifdef RGFW_BUFFER
		release(win->src.bitmap);
		release(win->src.image);
#endif

		CVDisplayLinkStop(win->src.displayLink);
		CVDisplayLinkRelease(win->src.displayLink);

		RGFW_FREE(win);
	}

	u64 RGFW_getTimeNS(void) {
		static mach_timebase_info_data_t timebase_info;
		if (timebase_info.denom == 0) {
			mach_timebase_info(&timebase_info);
		}
		return mach_absolute_time() * timebase_info.numer / timebase_info.denom;
	}

	u64 RGFW_getTime(void) {
		static mach_timebase_info_data_t timebase_info;
		if (timebase_info.denom == 0) {
			mach_timebase_info(&timebase_info);
		}
		return (double) mach_absolute_time() * (double) timebase_info.numer / ((double) timebase_info.denom * 1e9);
	}
#endif /* RGFW_MACOS */

/*
	End of MaOS defines
*/

/*
	WEBASM defines
*/

#ifdef RGFW_WEBASM
RGFW_Event RGFW_events[20];
size_t RGFW_eventLen = 0;

EM_BOOL Emscripten_on_keydown(int eventType, const EmscriptenKeyboardEvent* e, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);
	
	RGFW_events[RGFW_eventLen].type = RGFW_keyPressed;
	memcpy(RGFW_events[RGFW_eventLen].keyName, e->key, 16);
	RGFW_events[RGFW_eventLen].keyCode = RGFW_apiKeyCodeToRGFW(e->keyCode);
	RGFW_events[RGFW_eventLen].lockState = 0;
	RGFW_eventLen++;

	RGFW_keyboard[RGFW_apiKeyCodeToRGFW(e->keyCode)].prev = RGFW_keyboard[RGFW_apiKeyCodeToRGFW(e->keyCode)].current;
	RGFW_keyboard[RGFW_apiKeyCodeToRGFW(e->keyCode)].current = 1;
	RGFW_keyCallback(RGFW_root, RGFW_apiKeyCodeToRGFW(e->keyCode), RGFW_events[RGFW_eventLen].keyName, 0, 1);
	
    return EM_TRUE;
}

EM_BOOL Emscripten_on_keyup(int eventType, const EmscriptenKeyboardEvent* e, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	RGFW_events[RGFW_eventLen].type = RGFW_keyReleased;
	memcpy(RGFW_events[RGFW_eventLen].keyName, e->key, 16);
	RGFW_events[RGFW_eventLen].keyCode = RGFW_apiKeyCodeToRGFW(e->keyCode);
	RGFW_events[RGFW_eventLen].lockState = 0;
	RGFW_eventLen++;

	RGFW_keyboard[RGFW_apiKeyCodeToRGFW(e->keyCode)].prev = RGFW_keyboard[RGFW_apiKeyCodeToRGFW(e->keyCode)].current;
	RGFW_keyboard[RGFW_apiKeyCodeToRGFW(e->keyCode)].current = 0;

	RGFW_keyCallback(RGFW_root, RGFW_apiKeyCodeToRGFW(e->keyCode), RGFW_events[RGFW_eventLen].keyName, 0, 0);

    return EM_TRUE;
}

EM_BOOL Emscripten_on_resize(int eventType, const EmscriptenUiEvent* e, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	RGFW_events[RGFW_eventLen].type = RGFW_windowResized;
	RGFW_eventLen++;

	RGFW_windowResizeCallback(RGFW_root, RGFW_RECT(0, 0, e->windowInnerWidth, e->windowInnerHeight));
    return EM_TRUE;
}

EM_BOOL Emscripten_on_fullscreenchange(int eventType, const EmscriptenFullscreenChangeEvent* e, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	RGFW_events[RGFW_eventLen].type = RGFW_windowResized;
	RGFW_eventLen++;

	RGFW_root->r = RGFW_RECT(0, 0, e->elementWidth, e->elementHeight);
	RGFW_windowResizeCallback(RGFW_root, RGFW_root->r);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_focusin(int eventType, const EmscriptenFocusEvent* e, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData); RGFW_UNUSED(e);

	RGFW_events[RGFW_eventLen].type = RGFW_focusIn;
	RGFW_eventLen++;

	RGFW_root->event.inFocus = 1;
	RGFW_focusCallback(RGFW_root, 1);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_focusout(int eventType, const EmscriptenFocusEvent* e, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData); RGFW_UNUSED(e);

	RGFW_events[RGFW_eventLen].type = RGFW_focusOut;
	RGFW_eventLen++;

	RGFW_root->event.inFocus = 0;
	RGFW_focusCallback(RGFW_root, 0);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_mousemove(int eventType, const EmscriptenMouseEvent* e, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	RGFW_events[RGFW_eventLen].type = RGFW_mousePosChanged;

	if ((RGFW_root->_winArgs & RGFW_HOLD_MOUSE)) {
		RGFW_point p = RGFW_POINT(e->movementX, e->movementY);
		RGFW_events[RGFW_eventLen].point = p;
	}
	else
		RGFW_events[RGFW_eventLen].point = RGFW_POINT(e->targetX, e->targetY);
	RGFW_eventLen++;
	
	RGFW_mousePosCallback(RGFW_root, RGFW_events[RGFW_eventLen].point);
    return EM_TRUE;
}

EM_BOOL Emscripten_on_mousedown(int eventType, const EmscriptenMouseEvent* e, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	RGFW_events[RGFW_eventLen].type = RGFW_mouseButtonPressed;
	RGFW_events[RGFW_eventLen].point = RGFW_POINT(e->targetX, e->targetY);
	RGFW_events[RGFW_eventLen].button = e->button + 1; 
	RGFW_events[RGFW_eventLen].scroll = 0;

	RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].prev = RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].current;	
	RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].current = 1;

	RGFW_mouseButtonCallback(RGFW_root, RGFW_events[RGFW_eventLen].button, RGFW_events[RGFW_eventLen].scroll, 1);
	RGFW_eventLen++;
	
    return EM_TRUE;
}

EM_BOOL Emscripten_on_mouseup(int eventType, const EmscriptenMouseEvent* e, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	RGFW_events[RGFW_eventLen].type = RGFW_mouseButtonReleased;
	RGFW_events[RGFW_eventLen].point = RGFW_POINT(e->targetX, e->targetY);
	RGFW_events[RGFW_eventLen].button = e->button + 1; 
	RGFW_events[RGFW_eventLen].scroll = 0;

	RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].prev = RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].current;	
	RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].current = 0;

	RGFW_mouseButtonCallback(RGFW_root, RGFW_events[RGFW_eventLen].button, RGFW_events[RGFW_eventLen].scroll, 0);
	RGFW_eventLen++;
    return EM_TRUE;
}

EM_BOOL Emscripten_on_wheel(int eventType, const EmscriptenWheelEvent* e, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	RGFW_events[RGFW_eventLen].type = RGFW_mouseButtonPressed;
	RGFW_events[RGFW_eventLen].point = RGFW_POINT(e->mouse.targetX, e->mouse.targetY);
	RGFW_events[RGFW_eventLen].button = RGFW_mouseScrollUp + (e->deltaY < 0); 
	RGFW_events[RGFW_eventLen].scroll = e->deltaY;

	RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].prev = RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].current;	
	RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].current = 1;

	RGFW_mouseButtonCallback(RGFW_root, RGFW_events[RGFW_eventLen].button, RGFW_events[RGFW_eventLen].scroll, 1);
	RGFW_eventLen++;

    return EM_TRUE;
}

EM_BOOL Emscripten_on_touchstart(int eventType, const EmscriptenTouchEvent* e, void* userData) { 
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

    size_t i; 
    for (i = 0; i < (size_t)e->numTouches; i++) { 
	    RGFW_events[RGFW_eventLen].type = RGFW_mouseButtonPressed;
	    RGFW_events[RGFW_eventLen].point = RGFW_POINT(e->touches[i].targetX, e->touches[i].targetY);
	    RGFW_events[RGFW_eventLen].button = 1; 
	    RGFW_events[RGFW_eventLen].scroll = 0;


	    RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].prev = RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].current;	
	    RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].current = 1;

        RGFW_mousePosCallback(RGFW_root, RGFW_events[RGFW_eventLen].point);

	    RGFW_mouseButtonCallback(RGFW_root, RGFW_events[RGFW_eventLen].button, RGFW_events[RGFW_eventLen].scroll, 1);
    	RGFW_eventLen++;
    }

	return EM_TRUE;
}
EM_BOOL Emscripten_on_touchmove(int eventType, const EmscriptenTouchEvent* e, void* userData) { 
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);
    
    size_t i; 
    for (i = 0; i < (size_t)e->numTouches; i++) { 
   	    RGFW_events[RGFW_eventLen].type = RGFW_mousePosChanged;
	    RGFW_events[RGFW_eventLen].point = RGFW_POINT(e->touches[i].targetX, e->touches[i].targetY);

        RGFW_mousePosCallback(RGFW_root, RGFW_events[RGFW_eventLen].point);
	    RGFW_eventLen++;
    }
    return EM_TRUE;
}

EM_BOOL Emscripten_on_touchend(int eventType, const EmscriptenTouchEvent* e, void* userData) {
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);
	
    size_t i; 
    for (i = 0; i < (size_t)e->numTouches; i++) { 
	    RGFW_events[RGFW_eventLen].type = RGFW_mouseButtonReleased;
	    RGFW_events[RGFW_eventLen].point = RGFW_POINT(e->touches[i].targetX, e->touches[i].targetY);
	    RGFW_events[RGFW_eventLen].button = 1; 
	    RGFW_events[RGFW_eventLen].scroll = 0;

	    RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].prev = RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].current;	
	    RGFW_mouseButtons[RGFW_events[RGFW_eventLen].button].current = 0;
        
	    RGFW_mouseButtonCallback(RGFW_root, RGFW_events[RGFW_eventLen].button, RGFW_events[RGFW_eventLen].scroll, 0);
	    RGFW_eventLen++;
    }
	return EM_TRUE;
}

EM_BOOL Emscripten_on_touchcancel(int eventType, const EmscriptenTouchEvent* e, void* userData) { RGFW_UNUSED(eventType); RGFW_UNUSED(userData); RGFW_UNUSED(e); return EM_TRUE; }

EM_BOOL Emscripten_on_gamepad(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData) {	
	RGFW_UNUSED(eventType); RGFW_UNUSED(userData);

	if (gamepadEvent->index >= 4)
		return 0;

	RGFW_joysticks[gamepadEvent->index] = gamepadEvent->connected;

    return 1; // The event was consumed by the callback handler
}

void EMSCRIPTEN_KEEPALIVE Emscripten_onDrop(size_t count) {
	if (!(RGFW_root->_winArgs & RGFW_ALLOW_DND))
		return;

	RGFW_events[RGFW_eventLen].droppedFilesCount = count;	
	RGFW_dndCallback(RGFW_root, RGFW_events[RGFW_eventLen].droppedFiles, count);
	RGFW_eventLen++;
}

b8 RGFW_stopCheckEvents_bool = RGFW_FALSE;
void RGFW_stopCheckEvents(void) { 
	RGFW_stopCheckEvents_bool = RGFW_TRUE;
}

void RGFW_window_eventWait(RGFW_window* win, i32 waitMS) {
	RGFW_UNUSED(win);

	if (waitMS == 0)
		return;
	
	u32 start = (u32)(((u64)RGFW_getTimeNS()) / 1e+6);

	while ((RGFW_eventLen == 0) && RGFW_stopCheckEvents_bool == RGFW_FALSE && 
		(waitMS < 0 || (RGFW_getTimeNS() / 1e+6) - start < waitMS)
	) {
		emscripten_sleep(0);
	}
	
	RGFW_stopCheckEvents_bool = RGFW_FALSE;
}

RGFWDEF void RGFW_init_buffer(RGFW_window* win);
void RGFW_init_buffer(RGFW_window* win) {
	#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER)
		if (RGFW_bufferSize.w == 0 && RGFW_bufferSize.h == 0)
			RGFW_bufferSize = RGFW_getScreenSize();
		
		win->buffer = RGFW_MALLOC(RGFW_bufferSize.w * RGFW_bufferSize.h * 4);
	#ifdef RGFW_OSMESA
			win->src.ctx = OSMesaCreateContext(OSMESA_RGBA, NULL);
			OSMesaMakeCurrent(win->src.ctx, win->buffer, GL_UNSIGNED_BYTE, win->r.w, win->r.h);
	#endif
	#else
	RGFW_UNUSED(win); /*!< if buffer rendering is not being used */
	#endif
}

void EMSCRIPTEN_KEEPALIVE RGFW_makeSetValue(size_t index, char* file) { 
	/* This seems like a terrible idea, don't replicate this unless you hate yourself or the OS */
	/* TODO: find a better way to do this, 
		strcpy doesn't seem to work, maybe because of asyncio
	*/

	RGFW_events[RGFW_eventLen].type = RGFW_dnd;
	char** arr = (char**)&RGFW_events[RGFW_eventLen].droppedFiles[index];
	*arr = file;
}

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

void EMSCRIPTEN_KEEPALIVE RGFW_mkdir(char* name) { mkdir(name, 0755); }

void EMSCRIPTEN_KEEPALIVE RGFW_writeFile(const char *path, const char *data, size_t len) {
    FILE* file = fopen(path, "w+");
	if (file == NULL)
		return;

    fwrite(data, sizeof(char), len, file);
    fclose(file);
}

RGFW_window* RGFW_createWindow(const char* name, RGFW_rect rect, u16 args) {
	RGFW_UNUSED(name) 

	RGFW_UNUSED(RGFW_initFormatAttribs);
	
    RGFW_window* win = RGFW_window_basic_init(rect, args);

    EmscriptenWebGLContextAttributes attrs;
    attrs.alpha = EM_TRUE;
    attrs.depth = EM_TRUE;
	attrs.alpha = EM_TRUE;
    attrs.stencil = RGFW_STENCIL;
    attrs.antialias = RGFW_SAMPLES;
    attrs.premultipliedAlpha = EM_TRUE;
    attrs.preserveDrawingBuffer = EM_FALSE;
	
    if (RGFW_DOUBLE_BUFFER == 0)
        attrs.renderViaOffscreenBackBuffer = 0;
    else
        attrs.renderViaOffscreenBackBuffer = RGFW_AUX_BUFFERS;
    
    attrs.failIfMajorPerformanceCaveat = EM_FALSE;
	attrs.majorVersion = (RGFW_majorVersion == 0) ? 1 : RGFW_majorVersion;
	attrs.minorVersion = RGFW_minorVersion;
	
    attrs.enableExtensionsByDefault = EM_TRUE;
    attrs.explicitSwapControl = EM_TRUE;

    emscripten_webgl_init_context_attributes(&attrs);
    win->src.ctx = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(win->src.ctx);

	#ifdef LEGACY_GL_EMULATION
	EM_ASM("Module.useWebGL = true; GLImmediate.init();");
	#endif

	emscripten_set_canvas_element_size("#canvas", rect.w, rect.h);
	emscripten_set_window_title(name);

	/* load callbacks */
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_FALSE, Emscripten_on_keydown);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_FALSE, Emscripten_on_keyup);
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
	
	if (args & RGFW_ALLOW_DND)  {
		win->_winArgs |= RGFW_ALLOW_DND;
	}

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
				// This works weird on modern opengl
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

	RGFW_init_buffer(win);
	glViewport(0, 0, rect.w, rect.h);
	
	RGFW_root = win; 

	if (args & RGFW_HIDE_MOUSE) {
		RGFW_window_showMouse(win, 0);
	}

	if (args & RGFW_FULLSCREEN) {
		RGFW_window_resize(win, RGFW_getScreenSize());
	}

    return win;
}

RGFW_Event* RGFW_window_checkEvent(RGFW_window* win) {
	static u8 index = 0;
	
	if (index == 0) 
		RGFW_resetKey();

	/* check gamepads */
    for (int i = 0; (i < emscripten_get_num_gamepads()) && (i < 4); i++) {
		if (RGFW_joysticks[i] == 0)
			continue;;
		
        EmscriptenGamepadEvent gamepadState;

        if (emscripten_get_gamepad_status(i, &gamepadState) != EMSCRIPTEN_RESULT_SUCCESS)
			break;

		// Register buttons data for every connected gamepad
		for (int j = 0; (j < gamepadState.numButtons) && (j < 16); j++) {
			u32 map[] = {
				RGFW_JS_A, RGFW_JS_X, RGFW_JS_B, RGFW_JS_Y,
				RGFW_JS_L1, RGFW_JS_R1, RGFW_JS_L2, RGFW_JS_R2,
				RGFW_JS_SELECT, RGFW_JS_START,
				0, 0,
				RGFW_JS_UP, RGFW_JS_DOWN, RGFW_JS_LEFT, RGFW_JS_RIGHT
			};

			u32 button = map[j]; 
			if (RGFW_jsPressed[i][button] != gamepadState.digitalButton[j]) {
				win->event.type = RGFW_jsButtonPressed;
				win->event.joystick = i;
				win->event.button = map[j];
				return &win->event;
			}

			RGFW_jsPressed[i][button] = gamepadState.digitalButton[j];
		}

		for (int j = 0; (j < gamepadState.numAxes) && (j < 4); j += 2) {
			win->event.axisesCount = gamepadState.numAxes;
			if (win->event.axis[j].x != gamepadState.axis[j] || 
				win->event.axis[j].y != gamepadState.axis[j + 1]
			) {
				win->event.axis[j].x = gamepadState.axis[j];
				win->event.axis[j].y = gamepadState.axis[j + 1];
				win->event.type = RGFW_jsAxisMove;
				win->event.joystick = i;
				return &win->event;
			}
		}
    }

	/* check queued events */
	if (RGFW_eventLen == 0)
		return NULL;
	
	RGFW_events[index].frameTime = win->event.frameTime;
	RGFW_events[index].frameTime2 = win->event.frameTime2;
	RGFW_events[index].inFocus = win->event.inFocus;

	win->event = RGFW_events[index];

	RGFW_eventLen--;

	if (RGFW_eventLen)
		index++;
	else
		index = 0;

	return &win->event;
}

void RGFW_window_resize(RGFW_window* win, RGFW_area a) {
	RGFW_UNUSED(win)
	emscripten_set_canvas_element_size("#canvas", a.w, a.h);
}

/* NOTE: I don't know if this is possible */
void RGFW_window_moveMouse(RGFW_window* win, RGFW_point v) { RGFW_UNUSED(win); RGFW_UNUSED(v); }
/* this one might be possible but it looks iffy */
void RGFW_window_setMouse(RGFW_window* win, u8* image, RGFW_area a, i32 channels) { RGFW_UNUSED(win); RGFW_UNUSED(channels) RGFW_UNUSED(a) RGFW_UNUSED(image) }

const char RGFW_CURSORS[11][12] = {
    "default",
    "default",
    "text",
    "crosshair",
    "pointer",
    "ew-resize",
    "ns-resize",
    "nwse-resize",
    "nesw-resize",
    "move",
    "not-allowed"
};

void RGFW_window_setMouseStandard(RGFW_window* win, u8 mouse) {
	RGFW_UNUSED(win)
	EM_ASM( { document.getElementById("canvas").style.cursor = UTF8ToString($0); }, RGFW_CURSORS[mouse]);
}

void RGFW_window_setMouseDefault(RGFW_window* win) {
	RGFW_window_setMouseStandard(win, RGFW_MOUSE_NORMAL);
}

void RGFW_window_showMouse(RGFW_window* win, i8 show) {
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

RGFW_point RGFW_window_getMousePoint(RGFW_window* win) {
	RGFW_UNUSED(win);
	
	EmscriptenMouseEvent mouseEvent;
    emscripten_get_mouse_status(&mouseEvent);
	return RGFW_POINT( mouseEvent.targetX,  mouseEvent.targetY);
}

void RGFW_window_setMousePassthrough(RGFW_window* win, b8 passthrough) {
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
	RGFW_UNUSED(textLen)
	EM_ASM({ navigator.clipboard.writeText(UTF8ToString($0)); }, text);
}


char* RGFW_readClipboard(size_t* size) {
	/*
		placeholder code for later
		I'm not sure if this is possible do the the async stuff
	*/
	
	if (size != NULL)
		*size = 0;
	
	char* str = (char*)malloc(1);
	str[0] = '\0';

	return str;
}

void RGFW_window_swapBuffers(RGFW_window* win) {
	RGFW_UNUSED(win);
	
	#ifdef RGFW_BUFFER
	if (!(win->_winArgs & RGFW_NO_CPU_RENDER)) {		
		glEnable(GL_TEXTURE_2D);

		GLuint texture;
		glGenTextures(1,&texture);

		glBindTexture(GL_TEXTURE_2D,texture);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RGFW_bufferSize.w, RGFW_bufferSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, win->buffer);
		
		float ratioX = ((float)win->r.w / (float)RGFW_bufferSize.w);
		float ratioY = ((float)win->r.h / (float)RGFW_bufferSize.h);

		// Set up the viewport
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_TRIANGLES);
			glTexCoord2f(0, ratioY); glColor3f(1, 1, 1); glVertex2f(-1, -1);
			glTexCoord2f(0, 0); glColor3f(1, 1, 1); glVertex2f(-1, 1);
			glTexCoord2f(ratioX, ratioY); glColor3f(1, 1, 1); glVertex2f(1, -1);

			glTexCoord2f(ratioX, 0); glColor3f(1, 1, 1); glVertex2f(1, 1);
			glTexCoord2f(ratioX, ratioY); glColor3f(1, 1, 1); glVertex2f(1, -1);
			glTexCoord2f(0, 0); glColor3f(1, 1, 1); glVertex2f(-1, 1);
		glEnd();

		glDeleteTextures(1, &texture);
	}
	#endif

	emscripten_webgl_commit_frame();
	emscripten_sleep(0);
}


void RGFW_window_makeCurrent_OpenGL(RGFW_window* win) {
	if (win == NULL)
	    emscripten_webgl_make_context_current(0);
	else
	    emscripten_webgl_make_context_current(win->src.ctx);
}

#ifndef RGFW_EGL
void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) { RGFW_UNUSED(win); RGFW_UNUSED(swapInterval); }
#endif

void RGFW_window_close(RGFW_window* win) {
    emscripten_webgl_destroy_context(win->src.ctx);

    free(win);
}

int RGFW_innerWidth(void) {   return EM_ASM_INT({ return window.innerWidth; });  }
int RGFW_innerHeight(void) {  return EM_ASM_INT({ return window.innerHeight; });  }

RGFW_area RGFW_getScreenSize(void) {
	return RGFW_AREA(RGFW_innerWidth(), RGFW_innerHeight());
}

void* RGFW_getProcAddress(const char* procname) { 
	return emscripten_webgl_get_proc_address(procname);
}

void RGFW_sleep(u64 milisecond) {
	emscripten_sleep(milisecond);
}

u64 RGFW_getTimeNS(void) {
	return emscripten_get_now() * 1e+6;
}

u64 RGFW_getTime(void) {
	return emscripten_get_now() * 1000;
}

void RGFW_releaseCursor(RGFW_window* win) {
	emscripten_exit_pointerlock();
}

void RGFW_captureCursor(RGFW_window* win, RGFW_rect r) { 
	RGFW_UNUSED(win)

	emscripten_request_pointerlock("#canvas", 1);
}


void RGFW_window_setName(RGFW_window* win, char* name) {
	RGFW_UNUSED(win);
	emscripten_set_window_title(name);
}

/* unsupported functions */
RGFW_monitor* RGFW_getMonitors(void) { return NULL; }
RGFW_monitor RGFW_getPrimaryMonitor(void) { return (RGFW_monitor){}; }
void RGFW_window_move(RGFW_window* win, RGFW_point v) { RGFW_UNUSED(win) RGFW_UNUSED(v) }
void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a) { RGFW_UNUSED(win) RGFW_UNUSED(a)  }
void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a) { RGFW_UNUSED(win) RGFW_UNUSED(a)  }
void RGFW_window_minimize(RGFW_window* win) { RGFW_UNUSED(win)}
void RGFW_window_restore(RGFW_window* win) { RGFW_UNUSED(win) }
void RGFW_window_setBorder(RGFW_window* win, b8 border) { RGFW_UNUSED(win) RGFW_UNUSED(border)  }
void RGFW_window_setIcon(RGFW_window* win, u8* icon, RGFW_area a, i32 channels) { RGFW_UNUSED(win) RGFW_UNUSED(icon) RGFW_UNUSED(a) RGFW_UNUSED(channels)  }
void RGFW_window_hide(RGFW_window* win) { RGFW_UNUSED(win) }
void RGFW_window_show(RGFW_window* win) {RGFW_UNUSED(win) }
b8 RGFW_window_isHidden(RGFW_window* win) { RGFW_UNUSED(win) return 0; }
b8 RGFW_window_isMinimized(RGFW_window* win) { RGFW_UNUSED(win) return 0; }
b8 RGFW_window_isMaximized(RGFW_window* win) { RGFW_UNUSED(win) return 0; }
RGFW_monitor RGFW_window_getMonitor(RGFW_window* win) { RGFW_UNUSED(win) return (RGFW_monitor){}; }

#endif

/* end of web asm defines */

/* unix (macOS, linux, web asm) only stuff */
#if defined(RGFW_X11) || defined(RGFW_MACOS) || defined(RGFW_WEBASM)  || defined(RGFW_WAYLAND)
/* unix threading */
#ifndef RGFW_NO_THREADS
#include <pthread.h>

	RGFW_thread RGFW_createThread(RGFW_threadFunc_ptr ptr, void* args) {
		RGFW_UNUSED(args);
		
		RGFW_thread t;
		pthread_create((pthread_t*) &t, NULL, *ptr, NULL);
		return t;
	}
	void RGFW_cancelThread(RGFW_thread thread) { pthread_cancel((pthread_t) thread); }
	void RGFW_joinThread(RGFW_thread thread) { pthread_join((pthread_t) thread, NULL); }
#ifdef __linux__
	void RGFW_setThreadPriority(RGFW_thread thread, u8 priority) { pthread_setschedprio((pthread_t)thread, priority); }
#endif
#endif

#ifndef RGFW_WEBASM
/* unix sleep */
	void RGFW_sleep(u64 ms) {
		struct timespec time;
		time.tv_sec = 0;
		time.tv_nsec = ms * 1e+6;

		nanosleep(&time, NULL);
	}
#endif

#endif /* end of unix / mac stuff*/
#endif /*RGFW_IMPLEMENTATION*/

#if defined(__cplusplus) && !defined(__EMSCRIPTEN__)
}
#endif
