/*
*
*   minigamepad - alpha

* Copyright (C) 2025 ColleagueRiley
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
    (MAKE SURE MG_IMPLEMENTATION is in exactly one header or you use -D MG_IMPLEMENTATION)
    #define MG_IMPLEMENTATION - makes it so source code is included with header
*/

/*
    #define MG_IMPLEMENTATION - (required) makes it so the source code is included
    #define MG_ALLOC x  - choose the default allocation function (defaults to standard malloc)
    #define MG_FREE x  - choose the default deallocation function (defaults to standard free)

    #define MG_EXPORT - use when building minigamepad
    #define MG_IMPORT - use when linking with minigamepad (not as a single-header)

    #define MG_USE_INT - force the use c-types rather than stdint.h (for systems that might not have stdint.h (msvc))
    #define mg_bool x - choose what type to use for bool, by default u32 is used

    #define MG_MAX_GAMEPADS x - set the max number of gamepads (4 by default)
*/



/*
    Credits :
        GLFW and SDL - used as a resource
        https:///github.com/MysteriousJ/Joystick-Input-Examples - good resoruce for learning how to use gamepad code

        contributors : (feel free to put yourself here if you contribute)
*/

#if _MSC_VER
    #if _MSC_VER < 600
        #define MG_C89
    #endif
#else
    #if defined(__STDC__) && !defined(__STDC_VERSION__)
        #define MG_C89
    #endif
#endif

#ifndef MG_UNUSED
    #define MG_UNUSED(x) (void)(x)
#endif

#ifndef MG_ALLOC
    #include <stdlib.h>
    #define MG_ALLOC malloc
    #define MG_FREE free
#endif

#ifndef MG_ASSERT
    #include <assert.h>
    #define MG_ASSERT assert
#endif

#if !defined(MG_MEMCPY) || !defined(MG_STRNCMP) || !defined(MG_STRNCPY) || !defined(MG_MEMSET) || !defined(MG_STRCSPN) || !defined(MG_STRSPN)
    #include <string.h>
#endif

#ifndef MG_MEMSET
    #define MG_MEMSET(ptr, value, num) memset(ptr, value, num)
#endif

#ifndef MG_MEMCPY
    #define MG_MEMCPY(dist, src, len) memcpy(dist, src, len)
#endif

#ifndef MG_STRNCMP
    #define MG_STRNCMP(s1, s2, max) strncmp(s1, s2, max)
#endif

#ifndef MG_STRNCPY
    #define MG_STRNCPY(dist, src, len) strncpy(dist, src, len)
#endif

#ifndef MG_STRCSPN
    #define MG_STRCSPN(str1, str2) strcspn(str1, str2)
#endif

#ifndef MG_STRSPN
    #define MG_STRSPN(str1, str2) strspn(str1, str2)
#endif

#if !defined(MG_SPRINTF) || !defined(MG_FPRINTF)
    #include <stdio.h>
#endif


#if !defined(MG_SPRINTF)
    #define MG_SPRINTF sprintf
#endif

#if !defined(MG_FPRINTF)
    #define MG_FPRINTF fprintf
#endif

#ifndef MG_FABS
#include <math.h>
#define MG_FABS(x) fabs(x)
#endif

#if defined(MG_EXPORT) ||  defined(MG_IMPORT)
    #if defined(_WIN32)
        #if defined(__TINYC__) && (defined(MG_EXPORT) ||  defined(MG_IMPORT))
            #define __declspec(x) __attribute__((x))
        #endif

        #if defined(MG_EXPORT)
            #define MG_API __declspec(dllexport)
        #else
            #define MG_API __declspec(dllimport)
        #endif
    #else
        #if defined(MG_EXPORT)
            #define MG_API __attribute__((visibility("default")))
        #endif
    #endif
    #ifndef MG_API
        #define MG_API
    #endif
#endif

#ifndef MG_API
    #ifdef MG_C89
        #define MG_API
    #else
        #define MG_API inline
    #endif
#endif

#ifndef MG_ENUM
    #define MG_ENUM(type, name) type name; enum
#endif


#if defined(__cplusplus) && !defined(__EMSCRIPTEN__)
    extern "C" {
#endif

    /* makes sure the header file part is only defined once by default */
#ifndef MG_HEADER

#define MG_HEADER

#ifdef __EMSCRIPTEN__
    #define MG_WASM
#elif defined(_WIN32) || defined(__WIN32) || defined(__WIN32__)
    #define MG_WINDOWS
#elif defined(__linux__)
    #define MG_LINUX
#elif defined(__APPLE__)
    #define MG_MACOS
#endif

#ifndef MG_INT_DEFINED
    #ifdef MG_USE_INT /* optional for any system that might not have stdint.h */
        typedef unsigned char       u8;
        typedef signed char         i8;
        typedef unsigned short     u16;
        typedef signed short       i16;
        typedef unsigned long int  u32;
        typedef signed long int    i32;
        typedef unsigned long long u64;
        typedef signed long long   i64;


        typedef signed long mg_ssize_t;
        typedef unsigned long mg_size_t;
    #else /* use stdint standard types instead of c "standard" types */
        #include <stdint.h>
        #include <stddef.h>

        typedef uint8_t     u8;
        typedef int8_t      i8;
        typedef uint16_t   u16;
        typedef int16_t    i16;
        typedef uint32_t   u32;
        typedef int32_t    i32;
        typedef uint64_t   u64;
        typedef int64_t    i64;

#ifdef __linux__
        #include <sys/types.h>
        typedef ssize_t mg_ssize_t;
#endif
        typedef size_t mg_size_t;
    #endif
    #define MG_INT_DEFINED
#endif

#ifndef MG_BOOL_DEFINED
    #define MG_BOOL_DEFINED
    typedef u8 mg_bool;
#endif

#define MG_BOOL(x) (mg_bool)((x) ? MG_TRUE : MG_FALSE) /* force an value to be 0 or 1 */
#define MG_TRUE (mg_bool)1
#define MG_FALSE (mg_bool)0

#ifndef MG_MAX_GAMEPADS
    #define MG_MAX_GAMEPADS 4
#endif

#ifndef MG_MAX_EVENTS
    #define MG_MAX_EVENTS 32
#endif

/**!
 * @brief global stucture for the source API data of all the gamepads
*/
typedef struct mg_gamepads_src mg_gamepads_src;

/**!
 * @brief global stucture for the data of all the gamepads
*/
typedef struct mg_gamepads mg_gamepads;

/**!
 * @brief the source API data of the gamepad object
*/
typedef struct mg_gamepad_src mg_gamepad_src;

/**!
 * @brief data stucture for gamepad objects
*/
typedef struct mg_gamepad mg_gamepad;

/**!
 * @brief source gamepad list object
*/
typedef struct mg_gamepad_list {
    mg_gamepad* head; /* head node of the list */
    mg_gamepad* cur; /* current/tail node of the list */
    mg_size_t count; /* number of nodes */
} mg_gamepad_list;

/**!
 * @brief abstract constants for gamepad buttons
*/
typedef MG_ENUM(i8, mg_button) {
    MG_BUTTON_UNKNOWN = -1,
    MG_BUTTON_SOUTH,           /**< Bottom face button (e.g. Xbox A button) */
    MG_BUTTON_EAST,            /**< Right face button (e.g. Xbox B button) */
    MG_BUTTON_WEST,            /**< Left face button (e.g. Xbox X button) */
    MG_BUTTON_NORTH,           /**< Top face button (e.g. Xbox Y button) */
    MG_BUTTON_BACK,             /**< back (or select) button on the gamepad */
    MG_BUTTON_GUIDE,            /**< guide button on the controller (e.g. the Xbox button or ps button) */
    MG_BUTTON_START,            /**< start button on the gamepad */
    MG_BUTTON_LEFT_STICK,       /**< left stick button (L3) */
    MG_BUTTON_RIGHT_STICK,      /**< left shoulder button (R4) */
    MG_BUTTON_LEFT_SHOULDER,    /**< left shoulder button (L1) */
    MG_BUTTON_RIGHT_SHOULDER,   /**< left shoulder button (R1) */
    MG_BUTTON_DPAD_LEFT,        /**< dpad left button */
    MG_BUTTON_DPAD_RIGHT,       /**< dpad right button */
    MG_BUTTON_DPAD_UP,          /**< dpad up button */
    MG_BUTTON_DPAD_DOWN,        /**< dpad down button */
    MG_BUTTON_LEFT_TRIGGER,     /**< left trigger button (L2) */
    MG_BUTTON_RIGHT_TRIGGER,    /**< right trigger button (R2) */
 /* extras */
    MG_BUTTON_MISC1,           /**< Additional button (e.g. Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button, Google Stadia capture button) */
    MG_BUTTON_RIGHT_PADDLE1,   /**< Upper or primary paddle, under your right hand (e.g. Xbox Elite paddle P1) */
    MG_BUTTON_LEFT_PADDLE1,    /**< Upper or primary paddle, under your left hand (e.g. Xbox Elite paddle P3) */
    MG_BUTTON_RIGHT_PADDLE2,   /**< Lower or secondary paddle, under your right hand (e.g. Xbox Elite paddle P2) */
    MG_BUTTON_LEFT_PADDLE2,    /**< Lower or secondary paddle, under your left hand (e.g. Xbox Elite paddle P4) */
    MG_BUTTON_TOUCHPAD,        /**< PS4/PS5 touchpad button */
    MG_BUTTON_MISC2,           /**< Additional button */
    MG_BUTTON_MISC3,           /**< Additional button */
    MG_BUTTON_MISC4,           /**< Additional button */
    MG_BUTTON_MISC5,           /**< Additional button */
    MG_BUTTON_MISC6,           /**< Additional button */
    MG_BUTTON_COUNT
};

/**!
 * @brief abstract constants for gamepad axes
*/
typedef MG_ENUM(i8, mg_axis) {
    MG_AXIS_UNKNOWN = -1, /**< */
    MG_AXIS_LEFT_X, /**< X axis of the left stick */
    MG_AXIS_LEFT_Y, /**< Y axis of the left stick */
    MG_AXIS_RIGHT_X, /**< X axis of the right stick */
    MG_AXIS_RIGHT_Y, /**< Y axis of the left stick */
    MG_AXIS_LEFT_TRIGGER, /**< Axis of the left triggle (0 - 1) */
    MG_AXIS_RIGHT_TRIGGER, /**< Axis of the right triggle (0 - 1) */
    MG_AXIS_HAT_DPAD_LEFT_RIGHT, /**< d-pad left-right hat value  */
    MG_AXIS_HAT_DPAD_LEFT = MG_AXIS_HAT_DPAD_LEFT_RIGHT,
    MG_AXIS_HAT_DPAD_RIGHT = MG_AXIS_HAT_DPAD_LEFT_RIGHT,
    MG_AXIS_HAT_DPAD_UP_DOWN, /**<  d-pad up-down hat value */
    MG_AXIS_HAT_DPAD_UP = MG_AXIS_HAT_DPAD_UP_DOWN, /**< */
    MG_AXIS_HAT_DPAD_DOWN = MG_AXIS_HAT_DPAD_UP_DOWN,
/* extras */
    MG_AXIS_THROTTLE,
    MG_AXIS_RUDDER,
    MG_AXIS_WHEEL,
    MG_AXIS_GAS,
    MG_AXIS_BRAKE,
    MG_AXIS_HAT1X,
    MG_AXIS_HAT1Y,
    MG_AXIS_HAT2X,
    MG_AXIS_HAT2Y,
    MG_AXIS_HAT3X,
    MG_AXIS_HAT3Y,
    MG_AXIS_PRESSURE,
    MG_AXIS_DISTANCE,
    MG_AXIS_TILT_X,
    MG_AXIS_TILT_Y,
    MG_AXIS_TOOL_WIDTH,
    MG_AXIS_VOLUME,
    MG_AXIS_PROFILE,
    MG_AXIS_MISC,

    MG_AXIS_COUNT
};

/**!
 * @brief the type of event in the queue
*/
typedef MG_ENUM(u8, mg_event_type) {
    MG_EVENT_NONE = 0, /**< no/null event */
    MG_EVENT_GAMEPAD_CONNECT, /**< a new gamepad connected */
    MG_EVENT_GAMEPAD_DISCONNECT, /**< a gamepad was disconnected */
    MG_EVENT_BUTTON_PRESS, /**< gamepad button was pressed */
    MG_EVENT_BUTTON_RELEASE, /**< gamepad button was released */
    MG_EVENT_AXIS_MOVE /**< gamepad axis was moved/changed */
};

/**!
 * @brief button (press or release) event type
*/
typedef struct mg_button_state {
    mg_bool supported; /* if the button is supported or not by the gamepad */
    mg_bool current; /* the current state of the button */
    mg_bool prev; /* the previous state of the button */
} mg_button_state;

/**!
 * @brief axis motion event type
*/
typedef struct mg_axis_state {
    mg_bool supported; /* if the axis is supported or not by the gamepad */
    float value; /* the current value of the axis */
    float deadzone; /* deadzones of the axis */
} mg_axis_state;

/**!
 * @brief event structure for processing event data
*/
typedef struct mg_event {
    mg_event_type type; /* the type of event */
    mg_button button; /* button value */
    mg_axis axis; /* axis value */
    mg_gamepad* gamepad; /* which gamepad */
} mg_event;

/**!
 * @brief stucture for all event data for collecting events
*/
typedef struct mg_events {
    mg_event queue[MG_MAX_EVENTS]; /* event queue array for collecting the frame's events */
    size_t len; /* the number of events in the array */
} mg_events;

/* callbacks */
/**!
 * @brief type for the connection callback function
*/
typedef void (*mg_gamepad_connection_func)(mg_gamepad* gamepad, mg_bool connected);

/**!
 * @brief type for the button state callback function
*/
typedef void (*mg_gamepad_button_func)(mg_gamepad* gamepad, mg_button button, mg_bool pressed);

/**!
 * @brief type for the axis move callback function
*/
typedef void (*mg_gamepad_axis_func)(mg_gamepad* gamepad, mg_axis);

/**!
 * @brief init gamepads, api and internal data
 * @param pointer to a pre-allocated gamepads object
*/
MG_API void mg_gamepads_init(mg_gamepads* gamepads);

/**!
 * @brief enable or disable the event queue [disabled by default and enabled by default when mg_gamepads_check_event is called]
 * @param gamepads object to modify
*/
MG_API void mg_gamepads_set_queue_events(mg_gamepads* gamepads, mg_bool queue_events);

/**!
 * @brief poll information on the gamepads
 * @param gamepads object that needs to be updated
 * @return returns a boolean value if there was an event or not to process
*/
MG_API mg_bool mg_gamepads_poll(mg_gamepads* gamepads);

/**!
 * @brief check and pop top event in the gamepad's event queue, does not poll for events
 * @param gamepads object that needs to be updated
 * @param pointer to a event object to fill the event with [can be NULL]
 * @return returns a boolean value if there was an event or not to process
*/
MG_API mg_bool mg_gamepads_check_queued_event(mg_gamepads* gamepads, mg_event* event);

/**!
 * @brief poll for events if they weren't already polled then check and pop the top event in the event queue
 * @param gamepads object that needs to be updated
 * @param pointer to a event object to fill the event with [can be NULL]
 * @return returns a boolean value if there was an event or not to process
*/
MG_API mg_bool mg_gamepads_check_event(mg_gamepads* gamepads, mg_event* event);

/**!
 * @brief free internal gamepads data
 * @param gamepads object
*/
MG_API void mg_gamepads_free(mg_gamepads* gamepads);

/**!
 * @brief returns if a button of a gamepad was pressed or not
 * @param gamepad object
 * @param button to check
 * @return boolean value button for if the button was pressed
*/

MG_API mg_bool mg_gamepad_button_is_pressed(mg_gamepad* gamepad, mg_button button);


/* add a new mapping */
/**!
 * @brief returns if a button of a gamepad was released down or not
 * @param the gamepad object
 * @param button to check
 * @return the boolean value for if the button was released
*/

MG_API mg_bool mg_gamepad_button_is_released(mg_gamepad* gamepad, mg_button button);

/* add a new mapping */
/**!
 * @brief returns if a button of a gamepad is down or not
 * @param the gamepad object
 * @param the button to check
 * @return the boolean state of the button
*/

MG_API mg_bool mg_gamepad_button_is_down(mg_gamepad* gamepad, mg_button button);

/* add a new mapping */
/**!
 * @brief returns the axis value of an axis of a gamepad
 * @param the source gamepad object
 * @param the axis to check
 * @return the current floating point value of the axis
*/

MG_API float mg_gamepad_axis_value(mg_gamepad* gamepad, mg_axis axis);

/* add a new mapping */
/**!
 * @brief set the function object for gamepad connection events
 * @param the function object to use
 * @return the original function object in use
*/

MG_API mg_gamepad_connection_func mg_set_gamepad_connected_callback(mg_gamepad_connection_func func);

/**!
 * @brief set the function object for gamepad release events
 * @param the function object to use
 * @return the original function object in use
*/
MG_API mg_gamepad_button_func mg_set_gamepad_release_callback(mg_gamepad_button_func func);

/**!
 * @brief set the function object for gamepad axis movement aevents
 * @param the function object to use
 * @return the current function object in use
*/
MG_API mg_gamepad_axis_func mg_set_gamepad_axis_callback(mg_gamepad_axis_func func);

/**!
 * @brief set the gamepad disconnected callback function object
 * @param the function object to use
 * @return the original value of the disconnected callback
*/

MG_API mg_gamepad_connection_func mg_set_gamepad_disconnected_callback(mg_gamepad_connection_func func);

/**!
 * @brief set the gamepad press callbqack function to use
 * @param the function object to use
 * @return the original value of the gamepad press callback
*/
MG_API mg_gamepad_button_func mg_set_gamepad_press_callback(mg_gamepad_button_func func);

/* add a new mapping */
/**!
 * @brief update the mappings the gamepads object uses with a new mapping
 * @param the new mapping string
 * @return returns a boolean value based on success
*/
MG_API mg_bool mg_update_gamepad_mappings(mg_gamepads* gamepads, const char* string);

/**!
 * @brief fetch the string name of a button (for testing)
 * @param the button enum value
 * @return constant c-string that represents the button's name
*/
MG_API const char* mg_button_get_name(mg_button button);

/**!
 * @brief fetch the string name of a axis (for testing)
 * @param the axis enum value
 * @return constant c-string that represents the axis's name
*/
MG_API const char* mg_axis_get_name(mg_axis axis);

#endif /* MG_HEADER */

#if (defined(MG_NATIVE) || defined(MG_IMPLEMENTATION)) && !defined(MG_NATIVE_HEADER)
#define MG_NATIVE_HEADER

#ifdef MG_LINUX

struct mg_input_absinfo {
    i32 value;
    i32 minimum;
    i32 maximum;
    i32 fuzz;
    i32 flat;
    i32 resolution;
};

struct mg_gamepad_src {
    int fd;
    u8 keyMap[512];
    u8 absMap[64];
    struct mg_input_absinfo absInfo[64];
    char full_path[256];
};

#elif defined(MG_WINDOWS)

struct mg_gamepad_src {
    void* device;
    u32 xinput_index;
};

#elif defined(MG_MACOS)

struct mg_gamepad_src {
    void* device;
    void* events;
};

#elif defined(MG_WASM)

struct mg_gamepad_src {
    int index;
};

#endif

struct mg_mapping;

struct mg_gamepad {
    char name[128];
    char guid[33];

    mg_button_state buttons[MG_BUTTON_COUNT];
    mg_axis_state axes[MG_AXIS_COUNT];

    mg_bool connected;
    mg_size_t index; /* index in gamepad array */

    struct mg_mapping* mapping;
    struct mg_gamepad* prev;
    struct mg_gamepad* next;
    mg_gamepad_src src;
};

#ifdef MG_LINUX
#include <linux/input.h>
#include <linux/input-event-codes.h>

struct mg_gamepads_src {
    int inotify, watch;
};
#elif defined(MG_WINDOWS)
struct mg_gamepads_src {
    void* dinput;
    void* ginput;
    void* dummy_win;
};
#elif defined(MG_MACOS)
struct mg_gamepads_src {
    void* hidManager;
};
#elif defined(MG_WASM)
struct mg_gamepads_src {
    int TODO;
};
#endif

struct mg_gamepads {
    mg_gamepad gamepads[MG_MAX_GAMEPADS];

    mg_gamepad_list list;
    mg_gamepad_list free_list;

    mg_events events;

    mg_bool queue_events;
    mg_bool polled_events;

    mg_gamepads_src src;
};

#endif /* MG_NATIVE */

#ifdef MG_IMPLEMENTATION

/* gamepads->src.global API */
/* find a valid unused gamepad or return NULL */
MG_API mg_gamepad* mg_gamepad_find(mg_gamepads* gamepads);
MG_API void mg_gamepad_release(mg_gamepads* gamepads, mg_gamepad* gamepad);
MG_API void mg_list_swap_gamepad(mg_gamepad_list* from, mg_gamepad_list* to, mg_gamepad* gamepad);

/* gamepads->src.platform-specific API */
MG_API void mg_gamepads_init_platform(mg_gamepads* gamepads);
/* updates gamepad structure by checking if any new gamepads are connected and adding them */
MG_API mg_bool mg_gamepads_poll_platform(mg_gamepads* gamepads, mg_events* events);
MG_API void mg_gamepads_free_platform(mg_gamepads* gamepads);
MG_API mg_bool mg_gamepad_update_platform(mg_gamepad* gamepad, mg_events* events);
MG_API void mg_gamepad_release_platform(mg_gamepad* gamepad);
MG_API mg_button mg_get_gamepad_button_platform(u32 button);
MG_API mg_axis mg_get_gamepad_axis_platform(u32 axis);

/* gamepads->src.mappings API */
MG_API struct mg_mapping* mg_gamepad_find_valid_mapping(mg_gamepad* gamepad);
MG_API mg_button mg_get_gamepad_button(mg_gamepad* gamepad, u8 button);
MG_API mg_axis mg_get_gamepad_axis(mg_gamepad* gamepad, u8 axis);
MG_API void mg_mappings_init(void);
/* public/global API implementation */

mg_bool mg_gamepad_button_is_pressed(mg_gamepad* gamepad, mg_button button) {
    return gamepad->buttons[button].current;
}

mg_bool mg_gamepad_button_is_released(mg_gamepad* gamepad, mg_button button) {
    return gamepad->buttons[button].prev && !gamepad->buttons[button].current;
}

mg_bool mg_gamepad_button_is_down(mg_gamepad* gamepad, mg_button button) {
    return gamepad->buttons[button].prev && gamepad->buttons[button].current;
}

float mg_gamepad_axis_value(mg_gamepad* gamepad, mg_axis axis) {
    return gamepad->axes[axis].value;
}

static mg_gamepad_connection_func mg_gamepad_connected_callback = NULL;

mg_gamepad_connection_func mg_set_gamepad_connected_callback(mg_gamepad_connection_func func) {
    mg_gamepad_connection_func prev = mg_gamepad_connected_callback;
    mg_gamepad_connected_callback = func;
    return prev;
}

static mg_gamepad_connection_func mg_gamepad_disconnected_callback = NULL;

mg_gamepad_connection_func mg_set_gamepad_disconnected_callback(mg_gamepad_connection_func func) {
    mg_gamepad_connection_func prev = mg_gamepad_disconnected_callback;
    mg_gamepad_disconnected_callback = func;
    return prev;
}

static mg_gamepad_button_func mg_gamepad_press_callback = NULL;

mg_gamepad_button_func mg_set_gamepad_press_callback(mg_gamepad_button_func func) {
    mg_gamepad_button_func prev = mg_gamepad_press_callback;
    mg_gamepad_press_callback = func;
    return prev;
}

static mg_gamepad_button_func mg_gamepad_release_callback = NULL;
#define mg_release_callback(gamepad, button) if (mg_gamepad_release_callback) mg_gamepad_release_callback(gamepad, button, MG_FALSE)

mg_gamepad_button_func mg_set_gamepad_release_callback(mg_gamepad_button_func func) {
    mg_gamepad_button_func prev = mg_gamepad_release_callback;
    mg_gamepad_release_callback = func;
    return prev;
}

static mg_gamepad_axis_func mg_gamepad_axis_callback = NULL;
mg_gamepad_axis_func mg_set_gamepad_axis_callback(mg_gamepad_axis_func func) {
    mg_gamepad_axis_func prev = mg_gamepad_axis_callback;
    mg_gamepad_axis_callback = func;
    return prev;
}

void mg_handle_event(mg_events* events, mg_event_type type, mg_button btn, mg_axis axis, mg_bool state, float value, mg_gamepad* gamepad) {
    mg_event event;
    event.gamepad = gamepad;
    event.axis = axis;
    event.button = btn;

    switch (type) {
        case MG_EVENT_GAMEPAD_CONNECT:
        case MG_EVENT_GAMEPAD_DISCONNECT:
            if (state) {
                type = MG_EVENT_GAMEPAD_CONNECT;
                if (mg_gamepad_connected_callback) mg_gamepad_connected_callback(gamepad, state);
            } else {
                type = MG_EVENT_GAMEPAD_DISCONNECT;
                if (mg_gamepad_disconnected_callback) mg_gamepad_disconnected_callback(gamepad, state);
            }
            break;
        case MG_EVENT_BUTTON_PRESS:
        case MG_EVENT_BUTTON_RELEASE:
            if (state == gamepad->buttons[btn].current) {
                return;
            }

            gamepad->buttons[btn].prev = gamepad->buttons[btn].current;
            gamepad->buttons[btn].current = state;
            if (state) {
                type = MG_EVENT_BUTTON_PRESS;
                if (mg_gamepad_press_callback) mg_gamepad_press_callback(gamepad, btn, state);
            }
            else {
                type = MG_EVENT_BUTTON_RELEASE;
                if (mg_gamepad_release_callback) mg_gamepad_release_callback(gamepad, btn, state);
            }
            break;
        case MG_EVENT_AXIS_MOVE:
            if (value == gamepad->axes[axis].value) {
                return;
            }

            gamepad->axes[axis].value = value;
            if (mg_gamepad_axis_callback) mg_gamepad_axis_callback(gamepad, axis);
            break;
        default: break;
    }

    /* push event */
    if (events == NULL || events->len >= MG_MAX_EVENTS) {
        return;
    }

    event.type = type;
    events->len += 1;
    events->queue[MG_MAX_EVENTS - events->len] = event;
}

#define mg_handle_connection_event(events, state, gamepad) mg_handle_event(events, MG_EVENT_GAMEPAD_CONNECT, 0, 0, state, 0, gamepad)
#define mg_handle_button_event(events, btn, state, gamepad) mg_handle_event(events, MG_EVENT_BUTTON_PRESS, btn, 0, state, 0, gamepad)
#define mg_handle_axis_event(events, axis, value, gamepad) mg_handle_event(events, MG_EVENT_AXIS_MOVE, 0, axis, 0, value, gamepad)

void mg_gamepads_init(mg_gamepads* gamepads) {
    MG_ASSERT(gamepads != NULL);

    gamepads->polled_events = MG_FALSE;
    gamepads->queue_events = MG_FALSE;

    mg_mappings_init();
    MG_MEMSET(gamepads, 0, sizeof(mg_gamepads));

    /* create free list */
    gamepads->free_list.head = &gamepads->gamepads[0];
    gamepads->free_list.cur = gamepads->free_list.head;

    {
        mg_size_t i;
        for (i = 0; i < MG_MAX_GAMEPADS; i++) {
            gamepads->free_list.cur->prev = NULL;
            gamepads->free_list.cur->next = NULL;

            if (i) {
                gamepads->free_list.cur->prev = &gamepads->gamepads[i - 1];
            }

            if (i != MG_MAX_GAMEPADS - 1) {
                gamepads->free_list.cur->next = &gamepads->gamepads[i + 1];
                gamepads->free_list.cur = gamepads->free_list.cur->next;
            }
        }
    }

    mg_gamepads_init_platform(gamepads);
}

void mg_gamepads_set_queue_events(mg_gamepads* gamepads, mg_bool queue_events) {
    gamepads->polled_events = queue_events;
}

mg_bool mg_gamepads_poll(mg_gamepads* gamepads) {
    mg_gamepad* cur;
    mg_bool out = MG_FALSE;

    mg_events* events = (gamepads->queue_events) ? &gamepads->events : NULL;

    if (mg_gamepads_poll_platform(gamepads, events)) {
        out = MG_TRUE;
    }

    for (cur = gamepads->list.head; cur != NULL; cur = cur->next) {
        if (mg_gamepad_update_platform(cur, events)) {
            out = MG_TRUE;
        }
    }

    return out;
}

mg_bool mg_events_pop(mg_events* events, mg_event* ev) {
    MG_ASSERT(events->len <= MG_MAX_EVENTS);

    if (events->len == 0) {
        return MG_FALSE;
    }

    if (ev) {
        *ev = events->queue[MG_MAX_EVENTS - events->len];
    }

    events->len -= 1;
    return MG_TRUE;
}

mg_bool mg_gamepads_check_event(mg_gamepads* gamepads, mg_event* event) {
    if (gamepads->events.len == 0 && gamepads->polled_events == MG_FALSE) {
        gamepads->queue_events = MG_TRUE;
        mg_gamepads_poll(gamepads);
        gamepads->polled_events = MG_TRUE;
    }

    if (mg_gamepads_check_queued_event(gamepads, event) == MG_FALSE) {
        gamepads->polled_events = MG_FALSE;
        return MG_FALSE;
    }

    return MG_TRUE;
}

mg_bool mg_gamepads_check_queued_event(mg_gamepads* gamepads, mg_event* event) {
    MG_ASSERT(gamepads != NULL);
    gamepads->polled_events = MG_TRUE;

    /* check queued events */
    return mg_events_pop(&gamepads->events, event);
}

void mg_gamepads_free(mg_gamepads* gamepads) {
    mg_gamepad* cur;
    MG_ASSERT(gamepads != NULL);

    mg_gamepads_free_platform(gamepads);

    for (cur = gamepads->list.cur; cur != NULL; cur = cur->prev) {
        mg_gamepad_release(gamepads, cur);
    }
    MG_MEMSET(gamepads, 0, sizeof(mg_gamepads));
}

void mg_list_swap_gamepad(mg_gamepad_list* from, mg_gamepad_list* to, mg_gamepad* gamepad) {
    if (gamepad->prev != NULL) {
        gamepad->prev->next = gamepad->next;
    }

    if (gamepad->next != NULL) {
        gamepad->next->prev = gamepad->prev;
    }

    if (from->cur == gamepad) {
        from->cur = gamepad->prev;
    }

    if (from->head == gamepad) {
        from->head = gamepad->next;
    }

    MG_MEMSET(gamepad, 0, sizeof(mg_gamepad));

    if (to->head == NULL) {
        to->head = gamepad;
        to->cur = NULL;
    } else if (to->cur) {
        to->cur->next = gamepad;
    }

    gamepad->prev = to->cur;
    to->cur = gamepad;
}

mg_gamepad* mg_gamepad_find(mg_gamepads* gamepads) {
    mg_gamepad* gamepad = gamepads->free_list.cur;
    if (gamepad == NULL)
        return NULL;

    mg_list_swap_gamepad(&gamepads->free_list, &gamepads->list, gamepad);
    gamepad->index = (mg_size_t)(gamepad - gamepads->gamepads);
    return gamepad;
}


void mg_gamepad_release(mg_gamepads* gamepads, mg_gamepad* gamepad) {
    mg_gamepad_release_platform(gamepad);
    mg_list_swap_gamepad(&gamepads->list, &gamepads->free_list, gamepad);
}

/*
 * start of linux
 */

#if defined(MG_LINUX)

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/input.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

mg_gamepad* mg_linux_setup_gamepad(mg_gamepads* gamepads, const char* full_path) {
    int fd = 0;
    struct input_id id = {0};
    char evBits[(EV_CNT + 7) / 8] = {0};
    char keyBits[(KEY_CNT + 7) / 8] = {0};
    char absBits[(ABS_CNT + 7) / 8] = {0};
    u32 i, btn, axis;
    mg_size_t buttonCount = 0, axisCount = 0;

    mg_gamepad* gamepad = mg_gamepad_find(gamepads);
    if (gamepad == NULL) {
        return NULL;
    }

    MG_STRNCPY(gamepad->src.full_path, full_path, 256);

    gamepad->src.fd = open(full_path, O_RDWR);

    fd = gamepad->src.fd;
    if (fd <= 0) {
        mg_gamepad_release(gamepads, gamepad);
        return NULL;
    }

    if (ioctl(fd, EVIOCGBIT(0, sizeof(evBits)), evBits) < 0 ||
        ioctl(fd, EVIOCGID, &id) < 0 ||
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits) < 0 ||
        ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits) < 0
    ) {
        mg_gamepad_release(gamepads, gamepad);
        return NULL;
    }

    #define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))
    if (!isBitSet(EV_ABS, evBits)) {
        mg_gamepad_release(gamepads, gamepad);
        return NULL;
    }

    memset(gamepad->buttons, 0, sizeof(gamepad->buttons));
    memset(gamepad->buttons, 0, sizeof(gamepad->axes));

    /* go through any buttons a gamepad would have */
    for (i = BTN_MISC; i < KEY_CNT; i++) {
        if (!isBitSet(i, keyBits))
            continue;

        gamepad->src.keyMap[i - BTN_MISC] = (u8)buttonCount;
        buttonCount++;
    }

    /* go through any axes a gamepad would have */
    for (i = 0; i < ABS_CNT; i++) {
        if (!isBitSet(i, absBits))
            continue;

        if (ioctl(fd, EVIOCGABS(i), (struct input_absinfo*)&gamepad->src.absInfo[i]) < 0)
            continue;

        gamepad->src.absMap[i] = (u8)axisCount;
        axisCount++;
    }

    if ((axisCount == 0 && buttonCount == 0) || buttonCount > MG_BUTTON_COUNT + 10) {
        mg_gamepad_release(gamepads, gamepad);
        return NULL;
    }

    /* Generate a joystick GUID that matches the SDL 2.0.5+ one (sourced from GLFW) */
    if (ioctl(gamepad->src.fd, EVIOCGNAME(sizeof(gamepad->name)), gamepad->name) < 0)
        MG_STRNCPY(gamepad->name, "Unknown", sizeof(gamepad->name));

    if (id.vendor && id.product && id.version) {
        MG_SPRINTF(gamepad->guid, "%02x%02x0000%02x%02x0000%02x%02x0000%02x%02x0000",
                 id.bustype & 0xff, id.bustype >> 8,
                 id.vendor & 0xff,  id.vendor >> 8,
                 id.product & 0xff, id.product >> 8,
                 id.version & 0xff, id.version >> 8);
    } else {
        const char* name = (const char*)gamepad->name;
        MG_SPRINTF(gamepad->guid, "%02x%02x0000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",
                 id.bustype & 0xff, id.bustype >> 8,
                 name[0], name[1], name[2], name[3],
                 name[4], name[5], name[6], name[7],
                 name[8], name[9], name[10]);
    }

    gamepad->mapping = mg_gamepad_find_valid_mapping(gamepad);

    for (btn = BTN_MISC; btn < KEY_CNT; btn++) {
        mg_button key = mg_get_gamepad_button(gamepad, gamepad->src.keyMap[btn - BTN_MISC]);
        if (key == MG_BUTTON_UNKNOWN)
            key = mg_get_gamepad_button_platform(btn);
        if (key == MG_BUTTON_UNKNOWN)
            continue;

        if (gamepad->buttons[key].supported)
            continue;

        if (!isBitSet(btn, keyBits)) {
            gamepad->buttons[key].supported = MG_FALSE;
            continue;
        }

        gamepad->buttons[key].supported = MG_TRUE;
        gamepad->buttons[key].current = 0;
    }

    for (axis = 0; axis < ABS_CNT; axis++) {
        mg_axis key;
        float deadzone = 0;
        if (!isBitSet(axis, absBits)) {
            continue;
        }

        switch (axis) {
            case ABS_HAT0X:
                gamepad->buttons[MG_BUTTON_DPAD_LEFT].supported = MG_TRUE;
                gamepad->buttons[MG_BUTTON_DPAD_LEFT].current = 0;
                gamepad->buttons[MG_BUTTON_DPAD_RIGHT].supported = MG_TRUE;
                gamepad->buttons[MG_BUTTON_DPAD_RIGHT].current = 0;
                deadzone = 0;
                break;
            case ABS_HAT0Y:
                gamepad->buttons[MG_BUTTON_DPAD_UP].supported = MG_TRUE;
                gamepad->buttons[MG_BUTTON_DPAD_UP].current = 0;
                gamepad->buttons[MG_BUTTON_DPAD_DOWN].supported = MG_TRUE;
                gamepad->buttons[MG_BUTTON_DPAD_DOWN].current = 0;
                deadzone = 0;
                break;
            case ABS_HAT1X:
            case ABS_HAT1Y:
            case ABS_HAT2X:
            case ABS_HAT2Y:
            case ABS_HAT3X:
            case ABS_HAT3Y:
                deadzone = 0;
                break;
            case ABS_Z:
                gamepad->buttons[MG_BUTTON_LEFT_TRIGGER].supported = MG_TRUE;
                gamepad->buttons[MG_BUTTON_LEFT_TRIGGER].current = 0;

                gamepad->axes[MG_AXIS_LEFT_TRIGGER].supported = MG_TRUE;
                gamepad->axes[MG_AXIS_LEFT_TRIGGER].value = 0;
                deadzone = 0;
                break;
            case ABS_RZ:
                gamepad->buttons[MG_BUTTON_RIGHT_TRIGGER].supported = MG_TRUE;
                gamepad->buttons[MG_BUTTON_RIGHT_TRIGGER].current = 0;

                gamepad->axes[MG_AXIS_RIGHT_TRIGGER].supported = MG_TRUE;
                gamepad->axes[MG_AXIS_RIGHT_TRIGGER].value = 0;
                deadzone = 0;
                break;
            default:
                deadzone = 0.15f;
                break;
        }

        key = mg_get_gamepad_axis(gamepad, gamepad->src.absMap[axis]);
        if (key == MG_AXIS_UNKNOWN) {
            key = mg_get_gamepad_axis_platform(axis);
        }
        if (key == MG_AXIS_UNKNOWN)
            continue;

        gamepad->axes[key].supported = MG_TRUE;
        gamepad->axes[key].value = 0;
        gamepad->axes[key].deadzone = deadzone;
    }

    #undef isBitSet

    {
        i32 flags = fcntl(gamepad->src.fd, F_GETFL, 0);
        fcntl(gamepad->src.fd, F_SETFL, flags | O_NONBLOCK);
    }
    gamepad->connected = MG_TRUE;

    return gamepad;
}

void mg_gamepad_release_platform(mg_gamepad* gamepad) {
    close(gamepad->src.fd);
}


void mg_gamepads_init_platform(mg_gamepads* gamepads) {
    struct dirent *dp;
    DIR *dfd;
    char full_path[256];

    gamepads->src.inotify = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (gamepads->src.inotify > 0) {
        /* HACK: Register for IN_ATTRIB to get notified when udev is done
               This works well in practice but the MG_TRUE way is libudev */

        gamepads->src.watch = inotify_add_watch(gamepads->src.inotify, "/dev/input/", IN_CREATE | IN_ATTRIB | IN_DELETE);
    }

    /* open the directory where all the devices are gonna be */
    if ((dfd = opendir("/dev/input/")) == NULL) {
        MG_FPRINTF(stderr, "Can't open /dev/input/\n");
        return;
    }

    /* for each file found: */
    while ((dp = readdir(dfd)) != NULL) {
        /* get the full path of it (size of path + size of file name) */
        const char path[] = "/dev/input/";
        mg_gamepad* gamepad;

        MG_STRNCPY(full_path, path, sizeof(full_path));
        MG_STRNCPY(&full_path[sizeof(path) - 1], dp->d_name, sizeof(full_path) - sizeof(path));
        full_path[255] = '\0';
        gamepad = (mg_gamepad*)mg_linux_setup_gamepad(gamepads, (const char*)full_path);
        if (gamepad) {
            mg_handle_connection_event(&gamepads->events, MG_TRUE, gamepad);
        }
    }

    closedir(dfd);
}

mg_bool mg_gamepads_poll_platform(mg_gamepads* gamepads, mg_events* events) {
    mg_ssize_t offset = 0;
    char buffer[16384];
    char full_path[256];
    const char path[] = "/dev/input/";
    mg_ssize_t size;

    if (gamepads->src.inotify <= 0)
        return MG_FALSE;

    size = read(gamepads->src.inotify, buffer, sizeof(buffer));

    while (size > offset) {
        const struct inotify_event* e = (struct inotify_event*) (buffer + offset);

        offset += (mg_ssize_t)sizeof(struct inotify_event) + e->len;

        if (strncmp(e->name, "event", 5) != 0) {
            continue;
        }

        MG_STRNCPY(full_path, path, sizeof(full_path));
        MG_STRNCPY(&full_path[sizeof(path) - 1], e->name, sizeof(full_path) - sizeof(path));
        full_path[255] = '\0';

        if (e->mask & (IN_CREATE | IN_ATTRIB)) {
            mg_gamepad* gamepad = mg_linux_setup_gamepad(gamepads, full_path);
            if (gamepad) {
                mg_handle_connection_event(events, MG_TRUE, gamepad);
                return MG_TRUE;
            }
        }

        else if (e->mask & IN_DELETE) {
            mg_gamepad* cur;
            for (cur = gamepads->list.head; cur != NULL; cur = cur->next) {
                if (MG_STRNCMP(cur->src.full_path, full_path, sizeof(cur->src.full_path)) == 0) {
                    mg_handle_connection_event(events, MG_FALSE, cur);
                    mg_gamepad_release(gamepads, cur);
                    return MG_TRUE;
                }
            }
        }
    }

    return MG_FALSE;
}

void mg_gamepads_free_platform(mg_gamepads* gamepads) {
    if (gamepads->src.inotify > 0) {
        if (gamepads->src.watch > 0)
            inotify_rm_watch(gamepads->src.inotify, gamepads->src.watch);

        close(gamepads->src.inotify);
    }

    gamepads->src.inotify = 0;
    gamepads->src.watch = 0;
}

mg_bool mg_gamepad_update_platform(mg_gamepad* gamepad, mg_events* events) {
    mg_bool event_handled = MG_FALSE;
    struct input_event ev;

    i8 i;
    if (gamepad->connected == MG_FALSE) return MG_FALSE;

    for (i = 0; i < 2; i++) {
        mg_button button = MG_BUTTON_LEFT_TRIGGER + i;
        mg_axis axis = MG_AXIS_LEFT_TRIGGER + i;
        mg_handle_button_event(events, button, MG_BOOL(gamepad->axes[axis].value >= 0.98f), gamepad);
    }

    for (i = 0; i < 2; i++) {
        mg_button button = MG_BUTTON_DPAD_LEFT + (mg_button)(i * 2);
        mg_button button2 = MG_BUTTON_DPAD_LEFT + 1 + (mg_button)(i * 2);
        mg_axis axis = (mg_axis)(MG_AXIS_HAT_DPAD_LEFT + i);

        mg_handle_button_event(events, button, gamepad->axes[axis].value < 0, gamepad);
        mg_handle_button_event(events, button2, gamepad->axes[axis].value > 0, gamepad);
    }


    MG_MEMSET(&ev, 0, sizeof(ev));
    while (read(gamepad->src.fd, &ev, sizeof(ev)) > 0) {
        if (ev.type != EV_KEY && ev.type != EV_ABS) continue;

        switch (ev.type) {
            case EV_KEY: {
                mg_button btn = mg_get_gamepad_button(gamepad, (u8)(gamepad->src.keyMap[ev.code - BTN_MISC]));
                if (btn == MG_BUTTON_UNKNOWN) {
                    btn = mg_get_gamepad_button_platform(ev.code);
                }

                if (btn == MG_BUTTON_UNKNOWN) {
                    break;
                }

                mg_handle_button_event(events, btn, MG_BOOL(ev.value), gamepad);
                event_handled = MG_TRUE;
                break;
            }
            case EV_ABS: {
                float deadzone, event_val;

                mg_axis axis = mg_get_gamepad_axis(gamepad, gamepad->src.absMap[ev.code]);
                const struct mg_input_absinfo info = gamepad->src.absInfo[ev.code];
                float normalized = (float)ev.value;
                const float range = (float)(info.maximum - info.minimum);

                if (axis == MG_AXIS_UNKNOWN)
                    axis = mg_get_gamepad_axis_platform(ev.code);
                if (axis == MG_AXIS_UNKNOWN) {
                    break;
                }

                if (range) {
                    /* Normalize to 0.0 -> 1.0 */
                    normalized = (normalized - (float)info.minimum) / range;
                    /* Normalize to -1.0 -> 1.0 */
                    normalized = normalized * 2.0f - 1.0f;
                }

                deadzone = gamepad->axes[axis].deadzone;
                event_val = normalized;
                if (MG_FABS(event_val) < deadzone) {
                    event_val = 0;
                }

                mg_handle_axis_event(events, axis, event_val, gamepad);
                event_handled = MG_TRUE;
                break;
            }
            default:
                break;
        }
    }

    return event_handled;
}

mg_button mg_get_gamepad_button_platform(u32 button) {
    switch (button) {
        case BTN_WEST:
            return MG_BUTTON_WEST;
        case BTN_A:
            return MG_BUTTON_SOUTH;
        case BTN_NORTH:
            return MG_BUTTON_NORTH;
        case BTN_EAST:
            return MG_BUTTON_EAST;
        case BTN_BACK:
            return MG_BUTTON_BACK;
        case BTN_MODE:
            return MG_BUTTON_GUIDE;
        case BTN_START:
            return MG_BUTTON_START;
        case BTN_THUMBL:
            return MG_BUTTON_LEFT_STICK;
        case BTN_THUMBR:
            return MG_BUTTON_RIGHT_STICK;
        case BTN_TL:
            return MG_BUTTON_LEFT_SHOULDER;
        case BTN_DPAD_UP:
            return MG_BUTTON_DPAD_UP;
        case BTN_DPAD_DOWN:
            return MG_BUTTON_DPAD_DOWN;
        case BTN_DPAD_LEFT:
            return MG_BUTTON_DPAD_LEFT;
        case BTN_DPAD_RIGHT:
            return MG_BUTTON_DPAD_RIGHT;
        case BTN_TR:
            return MG_BUTTON_RIGHT_SHOULDER;
        case BTN_TOUCH:
            return MG_BUTTON_TOUCHPAD;
        case BTN_TRIGGER_HAPPY4:
            return MG_BUTTON_RIGHT_PADDLE1;
        case BTN_TRIGGER_HAPPY6:
            return MG_BUTTON_RIGHT_PADDLE2;
        case BTN_TRIGGER_HAPPY7:
            return MG_BUTTON_LEFT_PADDLE1;
        case BTN_TRIGGER_HAPPY8:
            return MG_BUTTON_LEFT_PADDLE2;

        case BTN_SELECT:
            return MG_BUTTON_MISC1;
        case BTN_TRIGGER_HAPPY2:
            return MG_BUTTON_MISC2;
        case BTN_TRIGGER_HAPPY3:
            return MG_BUTTON_MISC3;
        case BTN_TRIGGER_HAPPY9:
            return MG_BUTTON_MISC5;
        case BTN_TRIGGER_HAPPY10:
            return MG_BUTTON_MISC6;

        case BTN_TRIGGER:     return MG_BUTTON_WEST;
        case BTN_THUMB:       return MG_BUTTON_SOUTH;
        case BTN_THUMB2:      return MG_BUTTON_EAST;
        case BTN_TOP:         return MG_BUTTON_NORTH;
        case BTN_TOP2:        return MG_BUTTON_START;
        case BTN_PINKIE:      return MG_BUTTON_LEFT_SHOULDER;
        case BTN_BASE:        return MG_BUTTON_RIGHT_SHOULDER;
        case BTN_BASE2:       return MG_BUTTON_BACK;

        case BTN_BASE3: return MG_BUTTON_BACK;
        case BTN_BASE4: return MG_BUTTON_START;
        case BTN_BASE5: return MG_BUTTON_START;
        case BTN_BASE6: return MG_BUTTON_RIGHT_STICK;
        default:
            return MG_BUTTON_UNKNOWN;
    }
    return MG_BUTTON_UNKNOWN;
}

mg_axis mg_get_gamepad_axis_platform(u32 axis) {
    switch (axis) {
        case ABS_X:
           return MG_AXIS_LEFT_X;
        case ABS_Y:
           return MG_AXIS_LEFT_Y;
        case ABS_Z:
           return MG_AXIS_LEFT_TRIGGER;
        case ABS_RX:
           return MG_AXIS_RIGHT_X;
        case ABS_RY:
           return MG_AXIS_RIGHT_Y;
        case ABS_RZ:
           return MG_AXIS_RIGHT_TRIGGER;
        case ABS_THROTTLE:
           return MG_AXIS_THROTTLE;
        case ABS_RUDDER:
           return MG_AXIS_RUDDER;
        case ABS_WHEEL:
           return MG_AXIS_WHEEL;
        case ABS_GAS:
           return MG_AXIS_GAS;
        case ABS_BRAKE:
           return MG_AXIS_BRAKE;
        case ABS_HAT0X:
           return MG_AXIS_HAT_DPAD_LEFT_RIGHT;
        case ABS_HAT0Y:
           return MG_AXIS_HAT_DPAD_UP_DOWN;
        case ABS_HAT1X:
           return MG_AXIS_HAT1X;
        case ABS_HAT1Y:
           return MG_AXIS_HAT1Y;
        case ABS_HAT2X:
           return MG_AXIS_HAT2X;
        case ABS_HAT2Y:
           return MG_AXIS_HAT2Y;
        case ABS_HAT3X:
           return MG_AXIS_HAT3X;
        case ABS_HAT3Y:
           return MG_AXIS_HAT3Y;
        case ABS_PRESSURE:
           return MG_AXIS_PRESSURE;
        case ABS_DISTANCE:
           return MG_AXIS_DISTANCE;
        case ABS_TILT_X:
           return MG_AXIS_TILT_X;
        case ABS_TILT_Y:
           return MG_AXIS_TILT_Y;
        case ABS_TOOL_WIDTH:
           return MG_AXIS_TOOL_WIDTH;
        case ABS_VOLUME:
           return MG_AXIS_VOLUME;
        case ABS_PROFILE:
           return MG_AXIS_PROFILE;
        case ABS_MISC:
           return MG_AXIS_MISC;
        default:
           return MG_AXIS_UNKNOWN;
    }

    return MG_AXIS_UNKNOWN;
}
#endif /* MG_LINUX */

/*
 * start of windows
 */

#if defined(MG_WINDOWS)

#ifdef __cplusplus
#define MG_REF_GUID(g) *(g)
#else
#define MG_REF_GUID(g) (g)
#endif

#include <xinput.h>
#include <dinput.h>

#ifndef DIDFT_OPTIONAL
#define DIDFT_OPTIONAL          0x80000000
#endif

typedef void (*mg_proc)(void); /* function pointer equivalent of void* */

MG_API void mg_xinput_fetch_gamepads(mg_gamepads* gamepads, mg_events* events);

mg_gamepad* mg_xinput_list[XUSER_MAX_COUNT];
typedef DWORD (* PFN_XInputGetState)(DWORD,XINPUT_STATE*);
typedef DWORD (* PFN_XInputGetCapabilities)(DWORD,DWORD,XINPUT_CAPABILITIES*);
typedef DWORD (* PFN_XInputGetKeystroke)(DWORD, DWORD, PXINPUT_KEYSTROKE);
typedef HRESULT (WINAPI * PFN_DirectInput8Create)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);
typedef HRESULT (WINAPI * PFN_GameInputCreate)(void* gameinput);

HINSTANCE mg_gameinput_dll = NULL;
HINSTANCE mg_xinput_dll = NULL;
HINSTANCE mg_dinput_dll = NULL;

PFN_GameInputCreate GameInputCreateSrc = NULL;

PFN_XInputGetState XInputGetStateSrc = NULL;
PFN_XInputGetKeystroke XInputGetKeystrokeSrc = NULL;
PFN_XInputGetCapabilities XInputGetCapabilitiesSrc = NULL;
PFN_DirectInput8Create DInput8CreateSrc = NULL;

const GUID MG_IID_IDirectInput8W =
    {0xbf798031,0x483a,0x4da2,{0xaa,0x99,0x5d,0x64,0xed,0x36,0x97,0x00}};
const GUID MG_GUID_XAxis =
    {0xa36d02e0,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_YAxis =
    {0xa36d02e1,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_ZAxis =
    {0xa36d02e2,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_RxAxis =
    {0xa36d02f4,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_RyAxis =
    {0xa36d02f5,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_RzAxis =
    {0xa36d02e3,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_Slider =
    {0xa36d02e4,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_POV =
    {0xa36d02f2,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};

static DIOBJECTDATAFORMAT mg_objectDataFormats[] = {
    { &MG_GUID_XAxis,DIJOFS_X,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_YAxis,DIJOFS_Y,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_ZAxis,DIJOFS_Z,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_RxAxis,DIJOFS_RX,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_RyAxis,DIJOFS_RY,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_RzAxis,DIJOFS_RZ,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_Slider,DIJOFS_SLIDER(0),DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_Slider,DIJOFS_SLIDER(1),DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_POV,DIJOFS_POV(0),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { &MG_GUID_POV,DIJOFS_POV(1),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { &MG_GUID_POV,DIJOFS_POV(2),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { &MG_GUID_POV,DIJOFS_POV(3),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(0),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(1),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(2),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(3),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(4),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(5),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(6),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(7),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(8),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(9),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(10),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(11),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(12),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(13),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(14),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(15),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(16),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(17),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(18),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(19),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(20),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(21),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(22),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(23),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(24),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(25),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(26),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(27),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(28),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(29),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(30),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(31),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
};

const DIDATAFORMAT mg_dataFormat = {
    sizeof(DIDATAFORMAT),
    sizeof(DIOBJECTDATAFORMAT),
    DIDFT_ABSAXIS,
    sizeof(DIJOYSTATE),
    sizeof(mg_objectDataFormats) / sizeof(DIOBJECTDATAFORMAT),
    mg_objectDataFormats
};

mg_bool mg_supportsXInput(const GUID* guid) {
    RAWINPUTDEVICELIST* list;
    unsigned int count = 0;
    mg_size_t i;

    if (mg_xinput_dll == NULL) {
        return MG_FALSE;
    }

    if (GetRawInputDeviceList(NULL, &count, sizeof(RAWINPUTDEVICELIST)) != 0)
        return MG_FALSE;

    list = (RAWINPUTDEVICELIST*)malloc(count * sizeof(RAWINPUTDEVICELIST));
    MG_MEMSET(list, 0, count * sizeof(RAWINPUTDEVICELIST));

    if ((int)GetRawInputDeviceList(list, &count, sizeof(RAWINPUTDEVICELIST)) == -1) {
        free(list);
        return MG_FALSE;
    }

    for (i = 0;  i < count;  i++) {
        RID_DEVICE_INFO rdi = {0};
        char name[256];
        UINT size = sizeof(rdi);

        rdi.cbSize = sizeof(rdi);

        if (list[i].dwType != RIM_TYPEHID)
            continue;

        if ((int)GetRawInputDeviceInfoA(list[i].hDevice, RIDI_DEVICEINFO, &rdi, &size) == -1) {
            continue;
        }

        if (MAKELONG(rdi.hid.dwVendorId, rdi.hid.dwProductId) != (LONG) guid->Data1)
            continue;

        MG_MEMSET(name, 0, sizeof(name));
        size = sizeof(name);

        if ((int)GetRawInputDeviceInfoA(list[i].hDevice, RIDI_DEVICENAME, name, &size) == -1) {
            break;
        }

        name[sizeof(name) - 1] = '\0';
        if (strstr((char*)name, "IG_")) {
            free(list);
            return MG_TRUE;
        }
    }

    free(list);
    return MG_FALSE;
}

#include <math.h>

BOOL CALLBACK DirectInputEnumDevicesCallback(LPCDIDEVICEINSTANCE inst, LPVOID userData) {
    mg_gamepads* gamepads = (mg_gamepads*)userData;
    mg_gamepad* gamepad;
    u32 i;
    DIDEVCAPS caps;
    DIPROPDWORD dipd;
    /* avoid clones */

    if (mg_supportsXInput(&inst->guidProduct)) {
        return DIENUM_CONTINUE;
    }

    gamepad = mg_gamepad_find(gamepads);
    if (gamepad == NULL) return FALSE;

    gamepad->src.device = NULL;

    if (FAILED(IDirectInput8_CreateDevice((IDirectInput8*)gamepads->src.dinput, MG_REF_GUID(&inst->guidInstance), (IDirectInputDevice8**)&gamepad->src.device, NULL))) {
        mg_gamepad_release(gamepads, gamepad);
        return DIENUM_CONTINUE;
    }


    if (FAILED(IDirectInputDevice8_SetDataFormat((IDirectInputDevice8*)gamepad->src.device, &mg_dataFormat ))) {
        mg_gamepad_release(gamepads, gamepad);
        return DIENUM_CONTINUE;
    }

    MG_MEMSET(&caps, 0, sizeof(caps));
    caps.dwSize = sizeof(DIDEVCAPS);

    IDirectInputDevice8_GetCapabilities((IDirectInputDevice8*)gamepad->src.device, &caps);

    MG_MEMSET(&dipd, 0, sizeof(dipd));
    dipd.diph.dwSize = sizeof(dipd);
    dipd.diph.dwHeaderSize = sizeof(dipd.diph);
    dipd.diph.dwHow = DIPH_DEVICE;
    dipd.dwData = DIPROPAXISMODE_ABS;

    if (FAILED(IDirectInputDevice8_SetProperty((IDirectInputDevice8*)gamepad->src.device, DIPROP_AXISMODE, &dipd.diph))) {
        mg_gamepad_release(gamepads, gamepad);
        return DIENUM_CONTINUE;
    }

   if (!WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)inst->tszInstanceName, -1, gamepad->name, sizeof(gamepad->name), NULL, NULL)) {
        mg_gamepad_release(gamepads, gamepad);
        return DIENUM_STOP;
    }

    if (memcmp(&inst->guidProduct.Data4[2], "PIDVID", 6) == 0) {
        MG_SPRINTF(gamepad->guid, "03000000%02x%02x0000%02x%02x000000000000",
                (uint8_t) inst->guidProduct.Data1,
                (uint8_t) (inst->guidProduct.Data1 >> 8),
                (uint8_t) (inst->guidProduct.Data1 >> 16),
                (uint8_t) (inst->guidProduct.Data1 >> 24));
    } else {
        MG_SPRINTF(gamepad->guid, "05000000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",
                gamepad->name[0], gamepad->name[1], gamepad->name[2], gamepad->name[3],
                gamepad->name[4], gamepad->name[5], gamepad->name[6], gamepad->name[7],
                gamepad->name[8], gamepad->name[9], gamepad->name[10]);
    }


    gamepad->mapping = mg_gamepad_find_valid_mapping(gamepad);

{
    HRESULT result;
    DIJOYSTATE state;
    MG_MEMSET(&state, 0, sizeof(state));

    IDirectInputDevice8_Acquire((IDirectInputDevice8*)gamepad->src.device);
    IDirectInputDevice8_Poll((IDirectInputDevice8*)gamepad->src.device);

    result = IDirectInputDevice8_GetDeviceState((IDirectInputDevice8*)gamepad->src.device, sizeof(state), &state);
    if (FAILED(result)) {
        mg_gamepad_release(gamepads, gamepad);
        return DIENUM_CONTINUE;
    }
}

    for (i = 0; i < caps.dwButtons; i++) {
        mg_button key = mg_get_gamepad_button(gamepad, (u8)i);
        if (key == MG_BUTTON_UNKNOWN) {
            key = mg_get_gamepad_button_platform(i);
            if (key == MG_BUTTON_UNKNOWN) continue;
        }

        if (gamepad->buttons[key].supported)
            continue;

        gamepad->buttons[key].supported = MG_TRUE;
        gamepad->buttons[key].current = 0;
    }

    for (i = 0; i < caps.dwAxes; i++) {
        mg_axis key = mg_get_gamepad_axis(gamepad, (u8)i);
        if (key == MG_AXIS_UNKNOWN) {
            key = mg_get_gamepad_axis_platform(i);
            if (key == MG_AXIS_UNKNOWN) continue;
        }

        if (gamepad->axes[key].supported)
            continue;

        gamepad->axes[key].supported = MG_TRUE;
        gamepad->axes[key].value = 0;
    }

    if (caps.dwPOVs) {
        const mg_axis povs[2] = {
            MG_AXIS_HAT_DPAD_UP_DOWN,
            MG_AXIS_HAT_DPAD_LEFT_RIGHT,
        };

        for (i = 0; i < 2; i++) {
            mg_axis key = povs[i];
            gamepad->axes[key].supported = MG_TRUE;
            gamepad->axes[key].value = 0;
        }
    }

    gamepad->connected = MG_TRUE;
    mg_handle_connection_event(&gamepads->events, MG_TRUE, gamepad);

    return DIENUM_CONTINUE;
}

#include <dbt.h>

static LRESULT CALLBACK mg_winproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    mg_gamepads* gamepads = (mg_gamepads*)GetPropW(hWnd, L"gamepads");
    if (gamepads == NULL) return DefWindowProcW(hWnd, message, wParam, lParam);

    switch (message)    {
        case WM_DEVICECHANGE: {
                if (gamepads->src.dinput) {
                    IDirectInput8_EnumDevices((IDirectInput8*)gamepads->src.dinput,
                               DI8DEVCLASS_GAMECTRL,
                               DirectInputEnumDevicesCallback,
                               (void*)gamepads,
                               DIEDFL_ATTACHEDONLY);
                }

                mg_xinput_fetch_gamepads(gamepads, &gamepads->events);
            break;
        }
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}


void mg_gamepads_init_platform(mg_gamepads* gamepads) {
    HINSTANCE hInstance = GetModuleHandle(0);
    WNDCLASSW Class;

    if (mg_gameinput_dll == NULL) {
        /* load gameinput dll and functions (if it's available) */
        static const char* names[] = {"gameinput.lib", "gameinput.dll"};

        uint32_t i;
        for (i = 0; i < sizeof(names) / sizeof(const char*);  i++) {
            mg_gameinput_dll = LoadLibraryA(names[i]);
            if (mg_gameinput_dll) {
                GameInputCreateSrc = (PFN_GameInputCreate)(mg_proc)GetProcAddress(mg_gameinput_dll, "GameInputCreate");
            }
        }


        if (mg_gameinput_dll && GameInputCreateSrc) {
            /*HRESULT hr = GameInputCreateSrc(&gamepads->src.ginput);
            MG_UNUSED(hr); TODO */
        }
    }

    /* init global gamepads->src.data */
    if (mg_xinput_dll == NULL) {
        /* load xinput dll and functions (if it's available) */
        static const char* names[] = {"xinput0_4.dll", "xinput9_1_0.dll", "xinput1_2.dll", "xinput1_1.dll"};

        uint32_t i;
        for (i = 0; i < sizeof(names) / sizeof(const char*) && (XInputGetStateSrc == NULL || XInputGetKeystrokeSrc != NULL);  i++) {
            mg_xinput_dll = LoadLibraryA(names[i]);
            if (mg_xinput_dll) {
                XInputGetStateSrc = (PFN_XInputGetState)(mg_proc)GetProcAddress(mg_xinput_dll, "XInputGetState");
                XInputGetKeystrokeSrc = (PFN_XInputGetKeystroke)(mg_proc)GetProcAddress(mg_xinput_dll, "XInputGetKeystroke");
                XInputGetCapabilitiesSrc =  (PFN_XInputGetCapabilities)(mg_proc)GetProcAddress(mg_xinput_dll, "XInputGetCapabilities");
            }
        }

        if (mg_xinput_dll) {
            mg_xinput_fetch_gamepads(gamepads, &gamepads->events);
        }
    }

    if (mg_dinput_dll == NULL && DInput8CreateSrc == NULL) {
        /* load directinput dll and functions  */
        mg_dinput_dll = LoadLibraryA("dinput8.dll");
        if (mg_dinput_dll)
            DInput8CreateSrc = (PFN_DirectInput8Create)(mg_proc)GetProcAddress(mg_dinput_dll, "DirectInput8Create");
    }

    if (DInput8CreateSrc) {
        if (FAILED(DInput8CreateSrc(hInstance,
                                      DIRECTINPUT_VERSION,
                                      MG_REF_GUID(&MG_IID_IDirectInput8W),
                                      (void**) &gamepads->src.dinput,
                                      NULL)) ||
            FAILED(IDirectInput8_EnumDevices((IDirectInput8*)gamepads->src.dinput,
                                 DI8DEVCLASS_GAMECTRL,
                                 DirectInputEnumDevicesCallback,
                                 (void*)gamepads,
                                 DIEDFL_ALLDEVICES))) {
            mg_dinput_dll = NULL;
        }
    }

    MG_MEMSET(&Class, 0, sizeof(Class));

    Class.hInstance = hInstance;
    Class.lpfnWndProc = mg_winproc;
    Class.cbClsExtra = sizeof(mg_gamepads*);
    Class.lpszClassName = L"minigamepadclass";

    RegisterClassW(&Class);

    gamepads->src.dummy_win = CreateWindowW(Class.lpszClassName, (wchar_t*)"", 0, 0, 0, 0, 0, 0, 0, hInstance, 0);

    SetPropW((HWND)gamepads->src.dummy_win, L"gamepads", gamepads);
}

#ifndef XINPUT_DEVSUBTYPE_FLIGHT_STICK
    /* MINGW PLEASE FIX THIS */
    #define XINPUT_DEVSUBTYPE_FLIGHT_STICK 0x04
#endif

static const char* mg_xinput_gamepad_name(const XINPUT_CAPABILITIES xic) {
    switch (xic.SubType) {
        case XINPUT_DEVSUBTYPE_WHEEL:
            return "XInput Wheel";
        case XINPUT_DEVSUBTYPE_ARCADE_STICK:
            return "XInput Arcade Stick";
        case XINPUT_DEVSUBTYPE_FLIGHT_STICK:
            return "XInput Flight Stick";
        case XINPUT_DEVSUBTYPE_DANCE_PAD:
            return "XInput Dance Pad";
        case XINPUT_DEVSUBTYPE_GUITAR:
            return "XInput Guitar";
        case XINPUT_DEVSUBTYPE_DRUM_KIT:
            return "XInput Drum Kit";
        case XINPUT_DEVSUBTYPE_GAMEPAD: {
            if (xic.Flags & XINPUT_CAPS_WIRELESS)
                return "Wireless Xbox Controller";
            else
                return "Xbox Controller";
        }
    }

    return "Unknown XInput Device";
}

void mg_xinput_fetch_gamepads(mg_gamepads* gamepads, mg_events* events) {
    DWORD dwResult, i;
    if (mg_xinput_dll == NULL) return;

    for (i = 0; i < XUSER_MAX_COUNT; i++) {
        mg_gamepad* gamepad = mg_xinput_list[i];
        XINPUT_STATE state;
        mg_button button;
        mg_axis axis;
        char* name;
        XINPUT_CAPABILITIES xic;

        MG_MEMSET(&state, 0, sizeof(state));

        dwResult = XInputGetStateSrc(i, &state);

        if ((dwResult == ERROR_SUCCESS && gamepad) ||
            (dwResult != ERROR_SUCCESS && gamepad == NULL)
        )
            continue;

        if (dwResult != ERROR_SUCCESS) {
            gamepad->src.xinput_index = 0;
            mg_xinput_list[i] = NULL;
            mg_handle_connection_event(events, MG_FALSE, gamepad);
            mg_gamepad_release(gamepads, gamepad);

            continue;
        }

        if (XInputGetCapabilitiesSrc(i, 0, &xic) != ERROR_SUCCESS)
            continue;

        gamepad = mg_gamepad_find(gamepads);

        gamepad->src.xinput_index = i + 1;
        MG_SPRINTF(gamepad->guid, "78696e707574%02x000000000000000000", xic.SubType & 0xff);

        for (button = 0; button < MG_BUTTON_MISC1; button++) {
            if (button == MG_BUTTON_GUIDE) continue;

            gamepad->buttons[button].supported = MG_TRUE;
            gamepad->buttons[button].current = MG_FALSE;
            gamepad->buttons[button].prev = MG_FALSE;
        }

        for (axis = 0; axis < MG_AXIS_HAT_DPAD_LEFT_RIGHT; axis++) {
            gamepad->axes[axis].value = 0;
            gamepad->axes[axis].supported = MG_TRUE;
        }

        name = (char*)mg_xinput_gamepad_name(xic);
        MG_STRNCPY(gamepad->name, name, sizeof(gamepad->name));

        gamepad->connected = MG_TRUE;
        mg_xinput_list[i] = gamepad;
        mg_handle_connection_event(events, MG_TRUE, gamepad);
    }
}

mg_bool mg_gamepads_poll_platform(mg_gamepads* gamepads, mg_events* events) {
    mg_bool out = MG_FALSE;
    MSG msg;

    MG_UNUSED(gamepads); MG_UNUSED(events);

    while (PeekMessageA(&msg, NULL, 0u, 0u, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return out;
}

void mg_gamepads_free_platform(mg_gamepads* gamepads) {
    DestroyWindow((HWND)gamepads->src.dummy_win);

    if (mg_xinput_dll) {
        FreeLibrary(mg_xinput_dll);
        mg_xinput_dll = NULL;
    }

    if (mg_dinput_dll) {
        if (gamepads->src.dinput)
            IDirectInput8_Release((IDirectInput8*)gamepads->src.dinput);

        FreeLibrary(mg_dinput_dll);
    }
}


const u32 mg_xinput_map[MG_BUTTON_MISC1] = {
    XINPUT_GAMEPAD_A,
    XINPUT_GAMEPAD_B,
    XINPUT_GAMEPAD_X,
    XINPUT_GAMEPAD_Y,
    XINPUT_GAMEPAD_BACK,
    0,
    XINPUT_GAMEPAD_START,
    XINPUT_GAMEPAD_LEFT_THUMB,
    XINPUT_GAMEPAD_RIGHT_THUMB,
    XINPUT_GAMEPAD_LEFT_SHOULDER,
    XINPUT_GAMEPAD_RIGHT_SHOULDER,
    XINPUT_GAMEPAD_DPAD_LEFT,
    XINPUT_GAMEPAD_DPAD_RIGHT,
    XINPUT_GAMEPAD_DPAD_UP,
    XINPUT_GAMEPAD_DPAD_DOWN,
    0, 0
};

mg_bool mg_gamepad_update_platform(mg_gamepad* gamepad, mg_events* events) {

    if (gamepad->connected == MG_FALSE) {
/*        mg_gamepad_release(gamepad); */
        return MG_FALSE;
    }

    if (gamepad->src.xinput_index) {
        DWORD dwResult;
        XINPUT_STATE state;
        DWORD i = gamepad->src.xinput_index - 1;
        mg_button button;

        if (gamepad != mg_xinput_list[i]) {
            gamepad->connected = MG_FALSE;
            return MG_TRUE;
        }

        MG_MEMSET(&state, 0, sizeof(state));
        dwResult = XInputGetStateSrc(i, &state);
        if (dwResult != ERROR_SUCCESS) {
            gamepad->connected = MG_FALSE;
            mg_handle_connection_event(events, MG_FALSE, gamepad);
            return MG_TRUE;
        }

        for (button = 0; button < MG_BUTTON_MISC1; button++) {
            u32 btn = mg_xinput_map[button];
            mg_bool btn_state;

            if (btn == 0) continue;
            btn_state = MG_BOOL((state.Gamepad.wButtons & btn));

            mg_handle_button_event(events, button, btn_state, gamepad);
        }

        mg_handle_button_event(events, MG_BUTTON_LEFT_TRIGGER, MG_BOOL(gamepad->axes[MG_AXIS_LEFT_TRIGGER].value > 0), gamepad);
        mg_handle_button_event(events, MG_BUTTON_RIGHT_TRIGGER, MG_BOOL(gamepad->axes[MG_AXIS_RIGHT_TRIGGER].value > 0), gamepad);

        mg_handle_axis_event(events, MG_AXIS_LEFT_TRIGGER, (state.Gamepad.bLeftTrigger / 127.5f - 1.0f), gamepad);
        mg_handle_axis_event(events, MG_AXIS_RIGHT_TRIGGER, (state.Gamepad.bRightTrigger / 127.5f - 1.0f), gamepad);

        mg_handle_axis_event(events, MG_AXIS_LEFT_X, (state.Gamepad.sThumbLX + 0.5f) / 32767.5f, gamepad);
        mg_handle_axis_event(events, MG_AXIS_LEFT_Y, -(state.Gamepad.sThumbLY + 0.5f) / 32767.5f, gamepad);

        mg_handle_axis_event(events, MG_AXIS_RIGHT_X, (state.Gamepad.sThumbRX + 0.5f) / 32767.5f, gamepad);
        mg_handle_axis_event(events, MG_AXIS_RIGHT_Y, -(state.Gamepad.sThumbRY + 0.5f) / 32767.5f, gamepad);
   }

    if (gamepad->src.device) {
        u32 i;
        DIDEVCAPS caps;
        DIJOYSTATE state;
        HRESULT result;
        LONG axes_state[6];

        MG_MEMSET(&caps, 0, sizeof(caps));
        caps.dwSize = sizeof(DIDEVCAPS);

        IDirectInputDevice8_GetCapabilities((IDirectInputDevice8*)gamepad->src.device, &caps);
        IDirectInputDevice8_Poll((IDirectInputDevice8*)gamepad->src.device);
        result = IDirectInputDevice8_GetDeviceState((IDirectInputDevice8*)gamepad->src.device, sizeof(state), &state);
        if (result == DIERR_NOTACQUIRED || result == DIERR_INPUTLOST) {
            IDirectInputDevice8_Acquire((IDirectInputDevice8*)gamepad->src.device);
            IDirectInputDevice8_Poll((IDirectInputDevice8*)gamepad->src.device);

            result = IDirectInputDevice8_GetDeviceState((IDirectInputDevice8*)gamepad->src.device, sizeof(state), &state);
        }

        if (FAILED(result)) {
            gamepad->connected = MG_FALSE;
            mg_handle_connection_event(events, MG_FALSE, gamepad);
            return MG_FALSE;
        }

        for (i = 0; i < caps.dwButtons; i++) {
            mg_button key = mg_get_gamepad_button(gamepad, (u8)i);
            if (key == MG_BUTTON_UNKNOWN) {
                key = mg_get_gamepad_button_platform(i);
                if (key == MG_BUTTON_UNKNOWN) continue;
            }

            mg_handle_button_event(events, key, MG_BOOL(state.rgbButtons[i]), gamepad);
        }

        memcpy(&axes_state, &state, sizeof(axes_state));

        for (i = 0; i < 6; i++) {
            float value;
            mg_axis key = mg_get_gamepad_axis(gamepad, (u8)i);
            if (key == MG_AXIS_UNKNOWN) {
                key = mg_get_gamepad_axis_platform(i);
                if (key == MG_AXIS_UNKNOWN) continue;
            }

            /* NOTE: this doesn't work with triggers because triggers are combined into one input
             * TODO: fix this (or just use xinput)
             * */
            value = (((float)axes_state[i] + 0.5f) / 32767.5f) - 1.0f;
            mg_handle_axis_event(events, key, value, gamepad);
        }

        if (caps.dwPOVs) {
            DWORD pov = state.rgdwPOV[0];

            if (pov != 0xFFFF) {
                float angle = (float)pov / (45.0f * DI_DEGREES);
                i32 x = 0, y = 0;

                switch ((u32)angle) {
                    case 0: /* up */       x = 0;  y = -1; break;
                    case 1: /* up-right */ x = 1;  y = -1; break;
                    case 2: /* right */    x = 1;  y = 0;  break;
                    case 3: /* down-right */ x = 1; y = 1; break;
                    case 4: /* down */     x = 0;  y = 1;  break;
                    case 5: /* down-left */ x = -1; y = 1; break;
                    case 6: /* left */     x = -1; y = 0;  break;
                    case 7: /* up-left */  x = -1; y = -1; break;
                }

                mg_handle_button_event(events, MG_BUTTON_DPAD_LEFT, x < 0, gamepad);
                mg_handle_button_event(events, MG_BUTTON_DPAD_RIGHT, x > 0, gamepad);
                mg_handle_button_event(events, MG_BUTTON_DPAD_UP, y < 0, gamepad);
                mg_handle_button_event(events, MG_BUTTON_DPAD_DOWN, y > 0, gamepad);
            } else {
                mg_handle_axis_event(events, MG_AXIS_HAT_DPAD_LEFT_RIGHT, (float)0.0f, gamepad);
                mg_handle_axis_event(events, MG_AXIS_HAT_DPAD_UP_DOWN, (float)0.0f, gamepad);
            }
        }
    }

    return MG_FALSE;
}

void mg_gamepad_release_platform(mg_gamepad* gamepad) {
    if (gamepad->src.device) {
        IDirectInputDevice8_Release((IDirectInputDevice8*)gamepad->src.device);
    }

    if (gamepad->src.xinput_index) {
        mg_xinput_list[gamepad->src.xinput_index - 1] = NULL;
    }
}

mg_button mg_get_gamepad_button_platform(u32 button) {
    /* TODO */
    switch (button) {
        default: break;
    }
    return MG_BUTTON_UNKNOWN;
}

mg_axis mg_get_gamepad_axis_platform(u32 axis) {
    /* TODO */
    switch (axis) {
        case 2: return MG_AXIS_LEFT_TRIGGER;
        case 5: return MG_AXIS_RIGHT_TRIGGER;
        default: break;
    }

    return MG_AXIS_UNKNOWN;
}
#endif /* MG_WINDOWS */

/*
 * start of macos
 */

#if defined(MG_MACOS)
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDManager.h>

void mg_osx_input_value_changed_callback(void *context, IOReturn result, void *sender, IOHIDValueRef value) {
    mg_gamepad* gamepad = (mg_gamepad*)context;

    IOHIDElementRef element = IOHIDValueGetElement(value);

    IOHIDDeviceRef device = IOHIDElementGetDevice(element);

    uint32_t usagePage = IOHIDElementGetUsagePage(element);
    uint32_t usage = IOHIDElementGetUsage(element);

    CFIndex intValue = IOHIDValueGetIntegerValue(value);
    MG_UNUSED(result); MG_UNUSED(sender);

    if ((IOHIDDeviceRef)gamepad->src.device != device)
        return;

    switch (usagePage) {
        case kHIDPage_Button: {
            mg_button btn = mg_get_gamepad_button(gamepad, (u8)usage);
            if (btn == 0)
                btn = mg_get_gamepad_button_platform(usage);
            if (btn == 0)
                break;

            mg_handle_button_event((mg_events*)gamepad->src.events, btn, MG_BOOL(intValue), gamepad);
            break;
        }
        case kHIDPage_GenericDesktop: {
            CFIndex logicalMin = IOHIDElementGetLogicalMin(element);
            CFIndex logicalMax = IOHIDElementGetLogicalMax(element);
            mg_axis btn = mg_get_gamepad_axis(gamepad, (u8)usage);
            if (btn == 0)
                btn = mg_get_gamepad_axis_platform(usage);
            if (btn == 0)
                break;

            if (logicalMax <= logicalMin) return;
            if (intValue < logicalMin) intValue = logicalMin;
            if (intValue > logicalMax) intValue = logicalMax;

            mg_handle_axis_event((mg_events*)gamepad->src.events, btn, (-1.0f + ((intValue - logicalMin) * 2.0f) / (float)(logicalMax - logicalMin)), gamepad);
        }
    }
}


void mg_osx_device_added_callback(void* context, IOReturn result, void *sender, IOHIDDeviceRef device) {
    mg_gamepad* gamepad;
    mg_gamepads* gamepads = (mg_gamepads*)context;

    CFIndex i;
    CFArrayRef elements;
    CFTypeRef property;
    u32 vendor = 0, product = 0, version = 0;
    CFStringRef deviceName;
    CFTypeRef usageRef = (CFTypeRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsageKey));
    int usage = 0;
    if (usageRef)
        CFNumberGetValue((CFNumberRef)usageRef, (CFNumberType)kCFNumberIntType, (void*)&usage);

    MG_UNUSED(context); MG_UNUSED(result); MG_UNUSED(sender);
    if (usage != kHIDUsage_GD_Joystick && usage != kHIDUsage_GD_GamePad && usage != kHIDUsage_GD_MultiAxisController) {
        return;
    }

    elements = IOHIDDeviceCopyMatchingElements(device, NULL, kIOHIDOptionsTypeNone);
    if (elements == NULL)
        return;

    gamepad = mg_gamepad_find(gamepads);
    if (gamepad == NULL) {
        return;
    }

    IOHIDDeviceRegisterInputValueCallback(device, mg_osx_input_value_changed_callback, gamepad);

    deviceName = (CFStringRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
    if (deviceName)
        CFStringGetCString(deviceName, gamepad->name, sizeof(gamepad->name), kCFStringEncodingUTF8);
    else
        MG_STRNCPY(gamepad->name, "Unknown", sizeof(gamepad->name));

    property = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey));
    if (property)
        CFNumberGetValue((CFNumberRef)property, (CFNumberType)kCFNumberSInt32Type, (void*)&vendor);

    property = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey));
    if (property)
        CFNumberGetValue((CFNumberRef)property, kCFNumberSInt32Type, (void*)&product);

    property = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVersionNumberKey));
    if (property)
        CFNumberGetValue((CFNumberRef)property, (CFNumberType)kCFNumberSInt32Type, (void*)&version);

    if (vendor && product) {
        MG_SPRINTF(gamepad->guid, "03000000%02x%02x0000%02x%02x0000%02x%02x0000",
                (u8) vendor, (u8) (vendor >> 8),
                (u8) product, (u8) (product >> 8),
                (u8) version, (u8) (version >> 8));
    }
    else {
        MG_SPRINTF(gamepad->guid, "05000000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",
                gamepad->name[0],  gamepad->name[1],  gamepad->name[2],  gamepad->name[3],
                gamepad->name[4], gamepad->name[5],  gamepad->name[6],  gamepad->name[7],
                gamepad->name[8], gamepad->name[9],  gamepad->name[10]);
    }


    gamepad->mapping = mg_gamepad_find_valid_mapping(gamepad);
    gamepad->connected = MG_TRUE;

    for (i = 0;  i < CFArrayGetCount(elements);  i++) {
        u32 elm_usage = 0, page = 0;
        IOHIDElementType type;
        IOHIDElementRef native = (IOHIDElementRef)
            CFArrayGetValueAtIndex(elements, i);
        if (CFGetTypeID(native) != IOHIDElementGetTypeID())
            continue;

        type = IOHIDElementGetType(native);
        if ((type != kIOHIDElementTypeInput_Axis) &&
            (type != kIOHIDElementTypeInput_Button) &&
            (type != kIOHIDElementTypeInput_Misc))
        {
            continue;
        }


        elm_usage = IOHIDElementGetUsage(native);
        page = IOHIDElementGetUsagePage(native);

        switch (page) {
            case kHIDPage_Button: {
                mg_button btn = mg_get_gamepad_button(gamepad, (u8)elm_usage);
                if (btn == 0)
                    btn = mg_get_gamepad_button_platform(elm_usage);
                if (btn == 0)
                    break;

                gamepad->buttons[btn].prev = 0;
                gamepad->buttons[btn].current = 0;
                gamepad->buttons[btn].supported = MG_TRUE;
                break;
            }
            case kHIDPage_GenericDesktop: {
                mg_axis btn = mg_get_gamepad_axis(gamepad, (u8)elm_usage);
                if (btn == 0)
                    btn = mg_get_gamepad_axis_platform(elm_usage);
                if (btn == 0)
                    break;

                gamepad->axes[btn].value = 0.0f;
                gamepad->axes[btn].supported = MG_TRUE;
                break;
            }
        }
    }

    gamepad->src.events = (void*)&gamepads->events;
    mg_handle_connection_event(&gamepads->events, MG_TRUE, gamepad);
    CFRelease(elements);
}

void mg_osx_device_removed_callback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
    mg_gamepads* gamepads = (mg_gamepads*)context;
    mg_gamepad* cur = NULL;

    CFNumberRef usageRef = (CFNumberRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsageKey));
    int usage = 0;
    if (usageRef)
        CFNumberGetValue((CFNumberRef)usageRef, (CFNumberType)kCFNumberIntType, (void*)&usage);

    MG_UNUSED(context); MG_UNUSED(result); MG_UNUSED(sender); MG_UNUSED(device);
    if (usage != kHIDUsage_GD_Joystick && usage != kHIDUsage_GD_GamePad && usage != kHIDUsage_GD_MultiAxisController) {
        return;
    }

    for (cur = gamepads->list.head; cur; cur = cur->next) {
        if ((IOHIDDeviceRef)cur->src.device == device) {
            mg_handle_connection_event(&gamepads->events, MG_FALSE, cur);
            mg_gamepad_release(gamepads, cur);
            return;
        }
    }
}

void mg_gamepads_init_platform(mg_gamepads* gamepads) {
    const int filter[] = {kHIDPage_GenericDesktop};

    CFMutableDictionaryRef matchingDictionary;
    gamepads->src.hidManager = (void*)IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

    matchingDictionary = CFDictionaryCreateMutable(
        kCFAllocatorDefault,
        0,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );
    if (!matchingDictionary) {
        CFRelease((IOHIDManagerRef)gamepads->src.hidManager);
        return;
    }

    CFDictionarySetValue(
        matchingDictionary,
        CFSTR(kIOHIDDeviceUsagePageKey),
        CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, filter)
    );

    IOHIDManagerSetDeviceMatching((IOHIDManagerRef)gamepads->src.hidManager, matchingDictionary);
  CFRelease(matchingDictionary);

    IOHIDManagerRegisterDeviceMatchingCallback((IOHIDManagerRef)gamepads->src.hidManager, mg_osx_device_added_callback, gamepads);
    IOHIDManagerRegisterDeviceRemovalCallback((IOHIDManagerRef)gamepads->src.hidManager, mg_osx_device_removed_callback, gamepads);

    IOHIDManagerScheduleWithRunLoop((IOHIDManagerRef)gamepads->src.hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    IOHIDManagerOpen((IOHIDManagerRef)gamepads->src.hidManager, kIOHIDOptionsTypeNone);

    /* Execute the run loop once in order to register any initially-attached joysticks */
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);

}

mg_bool mg_gamepads_poll_platform(mg_gamepads* gamepads, mg_events* events) {
    MG_UNUSED(gamepads); MG_UNUSED(events);
    return MG_FALSE;
}

void mg_gamepads_free_platform(mg_gamepads* gamepads) {
    CFRelease((IOHIDManagerRef)gamepads->src.hidManager);
}

mg_bool mg_gamepad_update_platform(mg_gamepad* gamepad, mg_events* events) {
    gamepad->src.events = (void*)events;
    return MG_FALSE;
}


void mg_gamepad_release_platform(mg_gamepad* gamepads) {
    MG_UNUSED(gamepads);
}

mg_button mg_get_gamepad_button_platform(u32 button) {
    switch (button) {
        case kHIDUsage_GD_DPadUp: return MG_BUTTON_DPAD_UP;
        case kHIDUsage_GD_DPadRight:  return MG_BUTTON_DPAD_RIGHT;
        case kHIDUsage_GD_DPadDown:  return MG_BUTTON_DPAD_DOWN;
        case kHIDUsage_GD_DPadLeft:  return MG_BUTTON_DPAD_LEFT;
        case kHIDUsage_GD_SystemMainMenu:  return MG_BUTTON_GUIDE;
        case kHIDUsage_GD_Select:  return MG_BUTTON_BACK;
        case kHIDUsage_GD_Start: return MG_BUTTON_START;
        case kHIDUsage_Button_1: return MG_BUTTON_SOUTH;
        case kHIDUsage_Button_2: return MG_BUTTON_WEST;
        case kHIDUsage_Button_3: return MG_BUTTON_EAST;
        case kHIDUsage_Button_4: return MG_BUTTON_NORTH;
        case kHIDUsage_Button_5: return MG_BUTTON_LEFT_SHOULDER;
        case kHIDUsage_Button_6: return MG_BUTTON_RIGHT_SHOULDER;
        case kHIDUsage_Button_7: return MG_BUTTON_LEFT_TRIGGER;
        case kHIDUsage_Button_8: return MG_BUTTON_RIGHT_TRIGGER;
        case kHIDUsage_Button_9: return MG_BUTTON_LEFT_STICK;
        case kHIDUsage_Button_10: return MG_BUTTON_RIGHT_STICK;
        default: break;
    }

    return MG_BUTTON_UNKNOWN;
}

mg_axis mg_get_gamepad_axis_platform(u32 axis) {
    switch (axis) {
        case kHIDUsage_GD_X: return MG_AXIS_LEFT_X;
        case kHIDUsage_GD_Y: return MG_AXIS_LEFT_Y;
        case kHIDUsage_GD_Z: return MG_AXIS_LEFT_TRIGGER;
        case kHIDUsage_GD_Rx: return MG_AXIS_RIGHT_X;
        case kHIDUsage_GD_Ry: return MG_AXIS_RIGHT_Y;
        case kHIDUsage_GD_Rz: return MG_AXIS_RIGHT_TRIGGER;
        default: break;
    }
    return MG_AXIS_UNKNOWN;
}
#endif /* MG_MACOS */

/*
 * start of wasm
 */

#if defined(MG_WASM)
#include <emscripten/html5.h>

mg_gamepad* mg_wasm_gamepads[MG_MAX_GAMEPADS];


EM_BOOL mg_emscripten_on_gamepad(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData) {
    mg_gamepad* gamepad;
    mg_gamepads* gamepads = (mg_gamepads*)userData;
    if (gamepads == NULL) {
        return 0;
    }

    MG_UNUSED(eventType);

    gamepad = mg_wasm_gamepads[gamepadEvent->index];

    if (gamepadEvent->connected) {
        mg_button button;
        mg_axis axis;

        if (gamepad) {
            mg_gamepad_release(gamepads, mg_wasm_gamepads[gamepadEvent->index]);
        }

        gamepad = mg_gamepad_find(gamepads);
        if (gamepad == NULL) {
            return 0;
        }

        for (button = 0; button < MG_BUTTON_MISC1; button++) {
            gamepad->buttons[button].supported = MG_TRUE;
            gamepad->buttons[button].current = MG_FALSE;
            gamepad->buttons[button].prev = MG_FALSE;
        }

        for (axis = 0; axis < MG_AXIS_HAT_DPAD_LEFT_RIGHT; axis++) {
            gamepad->axes[axis].value = 0;
            gamepad->axes[axis].supported = MG_TRUE;
        }

        gamepad->connected = MG_TRUE;
        mg_wasm_gamepads[gamepadEvent->index] = gamepad;
    } else {
        mg_gamepad_release(gamepads, mg_wasm_gamepads[gamepadEvent->index]);
        mg_wasm_gamepads[gamepadEvent->index] = NULL;
    }

    return 1; /* The event was consumed by the callback handler */
}

void mg_gamepads_init_platform(mg_gamepads* gamepads) {
    emscripten_set_gamepadconnected_callback(gamepads, 1, mg_emscripten_on_gamepad);
    emscripten_set_gamepaddisconnected_callback(gamepads, 1, mg_emscripten_on_gamepad);

    emscripten_sample_gamepad_data();
}

mg_bool mg_gamepads_poll_platform(mg_gamepads* gamepads, mg_events* events) {
    MG_UNUSED(gamepads);  MG_UNUSED(events);
    return MG_FALSE;
}

void mg_gamepads_free_platform(mg_gamepads* gamepads) {
    MG_UNUSED(gamepads);
}

mg_bool mg_gamepad_update_platform(mg_gamepad* gamepad, mg_events* events) {
    int i = gamepad->src.index;
    int j;
    EmscriptenGamepadEvent gamepadState;

    emscripten_sample_gamepad_data();

    if (emscripten_get_gamepad_status(i, &gamepadState) != EMSCRIPTEN_RESULT_SUCCESS)
        return MG_FALSE;

    for (j = 0; j < gamepadState.numButtons; j++) {
        mg_button map[] = {
            MG_BUTTON_SOUTH, MG_BUTTON_EAST, MG_BUTTON_WEST, MG_BUTTON_NORTH,
            MG_BUTTON_LEFT_SHOULDER, MG_BUTTON_RIGHT_SHOULDER, MG_BUTTON_LEFT_TRIGGER, MG_BUTTON_RIGHT_TRIGGER,
            MG_BUTTON_BACK, MG_BUTTON_START,
            MG_BUTTON_LEFT_STICK, MG_BUTTON_RIGHT_STICK,
            MG_BUTTON_DPAD_UP, MG_BUTTON_DPAD_DOWN, MG_BUTTON_DPAD_LEFT, MG_BUTTON_DPAD_RIGHT,
            MG_BUTTON_GUIDE
        };
        mg_button btn = MG_BUTTON_UNKNOWN;
        if (j < (int)(sizeof(map) / sizeof(mg_button)))
            btn = map[j];
        if (btn == MG_BUTTON_UNKNOWN)
            continue;

        mg_handle_button_event(events, btn, MG_BOOL(gamepadState.digitalButton[j]), gamepad);
    }

    for (j = 0; j < gamepadState.numAxes; j += 2) {
        mg_axis map[] = {
            MG_AXIS_LEFT_X, MG_AXIS_LEFT_Y,
            MG_AXIS_RIGHT_X, MG_AXIS_RIGHT_Y,
            MG_AXIS_LEFT_TRIGGER, MG_AXIS_LEFT_TRIGGER
        };
        mg_axis btn = MG_AXIS_UNKNOWN;
        if (j < (int)(sizeof(map) / sizeof(mg_axis)))
            btn = map[j];
        if (btn == MG_AXIS_UNKNOWN)
            continue;

        mg_handle_axis_event(events, btn, (float)gamepadState.axis[j], gamepad);
    }

    return MG_FALSE;
}

void mg_gamepad_release_platform(mg_gamepad* gamepad) {
    mg_wasm_gamepads[gamepad->src.index] = NULL;
}

mg_button mg_get_gamepad_button_platform(u32 button) {
    /* TODO */
    MG_UNUSED(button);
    return MG_BUTTON_UNKNOWN;
}

mg_axis mg_get_gamepad_axis_platform(u32 axis) {
    /* TODO */
    MG_UNUSED(axis);
    return MG_AXIS_UNKNOWN;
}

#endif

extern const char* sdl_db[];

typedef struct mg_element {
    unsigned char         type;
    unsigned char         index;
    signed char          axisScale;
    signed char          axisOffset;
} mg_element;

typedef struct mg_mapping {
    char            name[128];
    char            guid[33];
    mg_element buttons[16];
    mg_element axes[6];

    mg_button rButtons[256];
    mg_axis rAxes[MG_AXIS_COUNT];
} mg_mapping;

#define MG_JOYSTICK_AXIS     1
#define MG_JOYSTICK_BUTTON   2
#define MG_JOYSTICK_HATBIT   3

typedef struct mappings_data {
    mg_mapping       mappings[1300];
    int               mappingCount;
    int               mappingMax;
} mappings_data;

mappings_data mappings;

mg_button mg_get_gamepad_button(mg_gamepad* gamepad, u8 btn) {
    if (gamepad->mapping == NULL) {
        return MG_BUTTON_UNKNOWN;
    }

    return gamepad->mapping->rButtons[btn];
}

mg_axis mg_get_gamepad_axis(mg_gamepad* gamepad, u8 axis) {
    if (gamepad->mapping == NULL) {
        return MG_AXIS_UNKNOWN;
    }

    if (axis >= MG_AXIS_COUNT) {
        return MG_AXIS_UNKNOWN;
    }

    return gamepad->mapping->rAxes[axis];
}

void updateGamepadGUID(char* guid) {
#ifdef __APPLE__
    if ((strncmp(&guid[4], "000000000000", 12) == 0) &&
        (strncmp(&guid[20], "000000000000", 12) == 0))
    {
        char original[33];
        MG_STRNCPY(original, guid, sizeof(original) - 1);
        MG_SPRINTF(guid,"03000000%.4s0000%.4s000000000000",
                original, original + 16);
    }
#elif defined(_WIN32)
    if (strncmp(&guid[20], "504944564944", 12) == 0) {
        char original[33];
        MG_STRNCPY(original, guid, sizeof(original) - 1);
        MG_SPRINTF(guid, "03000000%.4s0000%.4s000000000000",
                original, original + 4);
    }
#else
    (void)(guid); /* prevent unused warning */
#endif
}

MG_API mg_mapping* findMapping(const char* guid);
mg_mapping* findMapping(const char* guid) {
    int i;
    for (i = 0;  i < mappings.mappingCount;  i++) {
        if (strncmp(mappings.mappings[i].guid, guid, sizeof(mappings.mappings[i].guid)) == 0) {
            return &mappings.mappings[i];
        }
    }

    return NULL;
}

MG_API mg_mapping* findMappingPermisive(const char* guid);
mg_mapping* findMappingPermisive(const char* guid) {
    int i;
    for (i = 0;  i < mappings.mappingCount;  i++) {
        if (strncmp(mappings.mappings[i].guid, guid, sizeof(mappings.mappings[i].guid) - 8) == 0) {
            return &mappings.mappings[i];
        }
    }

    return NULL;
}

mg_mapping* mg_gamepad_find_valid_mapping(mg_gamepad* js) {
    mg_mapping* mapping = findMapping(js->guid);
    if (mapping == NULL) {
        mapping = findMappingPermisive(js->guid);
        if (mapping == NULL)
            return NULL;
    }

    return mapping;
}

typedef struct mg_field {
    const char* name;
    mg_size_t len;
    u8 val;
    mg_element* element;
} mg_field;


MG_API mg_bool parseMapping(mg_mapping* mapping, const char* string);
mg_bool parseMapping(mg_mapping* mapping, const char* string) {
    const char* substr = string;
    mg_size_t i, length, len;
    mg_field fields[] = {
        { "platform", 8,     0, NULL },
        { "a", 1,             MG_BUTTON_SOUTH , NULL},
        { "b", 1,           MG_BUTTON_EAST , NULL},
        { "x", 1,            MG_BUTTON_WEST , NULL},
        { "y", 1,            MG_BUTTON_NORTH , NULL},
        { "back", 4,          MG_BUTTON_BACK , NULL},
        { "start", 5,         MG_BUTTON_START , NULL},
        { "guide", 5,         MG_BUTTON_GUIDE , NULL},
        { "leftshoulder", 12,  MG_BUTTON_LEFT_SHOULDER , NULL},
        { "rightshoulder", 13, MG_BUTTON_RIGHT_SHOULDER , NULL},
        { "leftstick", 9,     MG_BUTTON_LEFT_STICK , NULL},
        { "rightstick", 10,    MG_BUTTON_RIGHT_STICK , NULL},
        { "dpup",   4,       MG_BUTTON_DPAD_UP , NULL},
        { "dpright", 7,       MG_BUTTON_DPAD_RIGHT , NULL},
        { "dpdown", 6,       MG_BUTTON_DPAD_DOWN , NULL},
        { "dpleft", 6,        MG_BUTTON_DPAD_LEFT , NULL},
        { "lefttrigger", 11,   MG_BUTTON_LEFT_TRIGGER , NULL},
        { "righttrigger", 12,  MG_BUTTON_RIGHT_TRIGGER , NULL },

        { "lefttrigger", 11,   MG_AXIS_LEFT_TRIGGER, NULL},
        { "righttrigger", 12,  MG_AXIS_RIGHT_TRIGGER, NULL },
        { "leftx",  5,       MG_AXIS_LEFT_X, NULL },
        { "lefty",  5,       MG_AXIS_LEFT_Y, NULL } ,
        { "rightx", 6,       MG_AXIS_RIGHT_X, NULL},
        { "righty", 6,        MG_AXIS_RIGHT_Y, NULL }
    };

    len = (sizeof(fields) / sizeof(mg_field));

    for (i = 1; i < len - 8; i++) {
        fields[i].element = &mapping->buttons[fields[i].val];
    }

    for (i = len - 8; i < len; i++) {
        fields[i].element = &mapping->axes[fields[i].val];
    }

    length = MG_STRCSPN(substr, ",");
    if (length != 32 || substr[length] != ',') {
        return MG_FALSE;
    }

    MG_MEMCPY(mapping->guid, substr, length);
    substr += length + 1;

    length = MG_STRCSPN(substr, ",");
    if (length >= sizeof(mapping->name) || substr[length] != ',') {
        return MG_FALSE;
    }

    MG_MEMCPY(mapping->name, substr, length);
    substr += length + 1;

    while (substr[0]) {
        /* TODO: Implement output modifiers */
        char mod = 0;;
        if (substr[0] == '+' || substr[0] == '-') {
            mod = substr[0];
            substr++;
        }

        for (i = 0;  i < sizeof(fields) / sizeof(fields[0]);  i++) {
            int8_t minimum = -1;
            int8_t maximum = 1;
            mg_element* e;

            switch (mod) {
                case '+':
                    minimum = 0;
                    break;
                case '-':
                    maximum = 0;
                    break;
                default: break;
            }

            length = fields[i].len;
            if (strncmp(substr, fields[i].name, length) != 0 || substr[length] != ':')
                continue;
            substr += length + 1;

            if (fields[i].element == NULL) {
                #if defined(_WIN32)
                    const char name[] = "Windows";
                #elif defined(__APPLE__)
                    const char name[] = "Mac OS X";
                #elif defined(EMSCRIPTEN)
                    const char name[] = "Web";
                #elif defined(__linux__)
                    const char name[] = "Linux";
                #else
                    const char name[] = "";
                #endif
                if (strncmp(substr, name, sizeof(name)) != 0)
                    return MG_FALSE;

                break;
            }

            e = fields[i].element;
            if (e >= mapping->axes && e <= &mapping->axes[6] && substr[0] == 'b') {
                continue;
            }

            switch (substr[0]) {
                case '+':
                    minimum = 0;
                    substr += 1;
                    break;
                case '-':
                    maximum = 0;
                    substr += 1;
                    break;
                default: break;
            }

            switch (substr[0]) {
                case 'a':

                    e->type = MG_JOYSTICK_AXIS;
                    e->axisScale = (signed char)(2 / (maximum - minimum));
                    e->axisOffset = (signed char)(-(maximum + minimum));

                    if (substr[0] == '~') {
                        e->axisScale = -e->axisScale;
                        e->axisOffset = -e->axisOffset;
                    }
                    e->index = (uint8_t) strtoul(&substr[1], (char**) &substr, 10);
                    break;
                case 'b':
                    e->type = MG_JOYSTICK_BUTTON;
                    e->index = (uint8_t) strtoul(&substr[1], (char**) &substr, 10);
                    break;
                case 'h': {
                    const unsigned long hat = strtoul(&substr[1], (char**) &substr, 10);
                    const unsigned long bit = strtoul(&substr[1], (char**) &substr, 10);

                    e->type = MG_JOYSTICK_HATBIT;
                    e->index = (uint8_t) ((hat << 4) | bit);

                    break;
                }
                default: break;
            }

            break;
        }

        substr += MG_STRCSPN(substr, ",");
        substr += MG_STRSPN(substr, ",");
    }

    for (i = 0;  i < 32;  i++) {
        if (mapping->guid[i] >= 'A' && mapping->guid[i] <= 'F')
            mapping->guid[i] += 'a' - 'A';
    }

    for (i = 0; i < 255; i++) {
        mg_size_t y;
        mapping->rButtons[i] = MG_BUTTON_UNKNOWN;
        for (y = 0; y < 16; y++) {
            mg_element e = mapping->buttons[y];
            if (e.index == i) {
                mapping->rButtons[i] = (mg_button)y;
                break;
            }
        }
    }

    for (i = 0; i < MG_AXIS_COUNT; i++) {
        mg_size_t y;
        mapping->rAxes[i] = MG_AXIS_UNKNOWN;
        for (y = 0; y < 6; y++) {
            mg_element e = mapping->axes[y];
            if (e.index == i) {
                mapping->rAxes[i] = (mg_axis)y;
                break;
            }
        }
    }

    updateGamepadGUID(mapping->guid);
    return MG_TRUE;
}

mg_bool mg_update_gamepad_mappings(mg_gamepads* gamepads, const char* string) {
    const char* substr = string;
    mg_gamepad* cur;
    mg_mapping mapping;
    mg_size_t length;
    char line[1024];

    if (mappings.mappingCount >= mappings.mappingMax) {
        return MG_FALSE;
    }

    while (*substr) {
        if (!(*substr >= '0' && *substr <= '9') &&
            !(*substr >= 'a' && *substr <= 'f') &&
            !(*substr >= 'A' && *substr <= 'F')) {
                substr += MG_STRCSPN(substr, "\r\n");
                substr += MG_STRSPN(substr, "\r\n");
                break;
        }

        length = MG_STRCSPN(substr, "\r\n");
        if (length < sizeof(line)) {
            MG_MEMSET(&mapping, 0, sizeof(mapping));
            MG_MEMCPY(line, substr, length);
            line[length] = '\0';

            if (parseMapping(&mapping, line)) {
                mg_mapping* previous = findMapping(mapping.guid);
                if (previous)
                    *previous = mapping;
                else {
                    mappings.mappingCount++;
                    /*mappings.mappings =
                            realloc(mappings.mappings,
                                          sizeof(mg_mapping) * (mg_size_t)mappings.mappingCount);*/
                    mappings.mappings[mappings.mappingCount - 1] = mapping;
                }
            }
        }

        substr += length;
    }

    for (cur = gamepads->list.head; cur; cur = cur->next) {
        cur->mapping = mg_gamepad_find_valid_mapping(cur);
    }

    return MG_TRUE;
}

const char * sdl_db[] = {
#ifdef _WIN32
"03000000300f00000a01000000000000,3 In 1 Conversion Box,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b8,x:b3,y:b0,",
"03000000fa190000918d000000000000,3 In 1 Conversion Box,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b8,x:b3,y:b0,",
"03000000fa2d00000100000000000000,3dRudder Foot Motion Controller,leftx:a0,lefty:a1,rightx:a5,righty:a2,",
"03000000d0160000040d000000000000,4Play Adapter,a:b1,b:b3,back:b4,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b6,leftstick:b14,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b15,righttrigger:b9,rightx:a3,righty:a4,start:b5,x:b0,y:b2,",
"03000000d0160000050d000000000000,4Play Adapter,a:b1,b:b3,back:b4,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b6,leftstick:b14,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b15,righttrigger:b9,rightx:a3,righty:a4,start:b5,x:b0,y:b2,",
"03000000d0160000060d000000000000,4Play Adapter,a:b1,b:b3,back:b4,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b6,leftstick:b14,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b15,righttrigger:b9,rightx:a3,righty:a4,start:b5,x:b0,y:b2,",
"03000000d0160000070d000000000000,4Play Adapter,a:b1,b:b3,back:b4,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b6,leftstick:b14,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b15,righttrigger:b9,rightx:a3,righty:a4,start:b5,x:b0,y:b2,",
"03000000d0160000600a000000000000,4Play Adapter,a:b1,b:b3,back:b4,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b6,leftstick:b14,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b15,righttrigger:b9,rightx:a3,righty:a4,start:b5,x:b0,y:b2,",
"03000000c82d00000031000000000000,8BitDo Adapter,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c82d00000531000000000000,8BitDo Adapter 2,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c82d00000951000000000000,8BitDo Dogbone,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a2,rightx:a3,righty:a5,start:b11,",
"03000000008000000210000000000000,8BitDo F30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"030000003512000011ab000000000000,8BitDo F30 Arcade Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000c82d00001028000000000000,8BitDo F30 Arcade Joystick,a:b0,b:b1,back:b10,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,start:b11,x:b3,y:b4,",
"03000000c82d000011ab000000000000,8BitDo F30 Arcade Joystick,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000801000000900000000000000,8BitDo F30 Arcade Stick,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"03000000c82d00001038000000000000,8BitDo F30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b4,y:b3,",
"03000000c82d00000090000000000000,8BitDo FC30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00006a28000000000000,8BitDo GameCube,a:b0,b:b3,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b9,paddle2:b8,rightshoulder:b10,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b1,y:b4,",
"03000000c82d00001251000000000000,8BitDo Lite 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00001151000000000000,8BitDo Lite SE,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00000150000000000000,8BitDo M30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,rightx:a3,righty:a5,start:b11,x:b4,y:b3,",
"03000000c82d00000151000000000000,8BitDo M30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a2,rightshoulder:b6,righttrigger:b7,rightx:a3,righty:a5,start:b11,x:b3,y:b4,",
"03000000c82d00000650000000000000,8BitDo M30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b8,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c82d00005106000000000000,8BitDo M30,a:b0,b:b1,back:b10,dpdown:+a2,dpleft:-a0,dpright:+a0,dpup:-a2,guide:b2,leftshoulder:b8,lefttrigger:b9,rightshoulder:b6,righttrigger:b7,start:b11,x:b3,y:b4,",
"03000000c82d00002090000000000000,8BitDo Micro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00000310000000000000,8BitDo N30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,start:b11,x:b3,y:b4,",
"03000000c82d00000451000000000000,8BitDo N30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a2,rightx:a3,righty:a5,start:b11,",
"03000000c82d00002028000000000000,8BitDo N30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b4,y:b3,",
"03000000c82d00008010000000000000,8BitDo N30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,start:b11,x:b3,y:b4,",
"03000000c82d0000e002000000000000,8BitDo N30,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,start:b6,",
"03000000c82d00000190000000000000,8BitDo N30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00001590000000000000,8BitDo N30 Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b4,y:b3,",
"03000000c82d00006528000000000000,8BitDo N30 Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00000290000000000000,8BitDo N64,+rightx:b9,+righty:b3,-rightx:b4,-righty:b8,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b7,start:b11,",
"03000000c82d00003038000000000000,8BitDo N64,+rightx:b9,+righty:b3,-rightx:b4,-righty:b8,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b7,start:b11,",
"03000000c82d00006928000000000000,8BitDo N64,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a3,righty:a4,start:b11,",
"03000000c82d00002590000000000000,8BitDo NEOGEO,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"030000003512000012ab000000000000,8BitDo NES30,a:b2,b:b1,back:b6,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b7,x:b3,y:b0,",
"03000000c82d000012ab000000000000,8BitDo NES30,a:b1,b:b0,back:b10,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000022000000090000000000000,8BitDo NES30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000203800000900000000000000,8BitDo NES30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00002038000000000000,8BitDo NES30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00000751000000000000,8BitDo P30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a2,rightshoulder:b7,righttrigger:b9,rightx:a3,righty:a5,start:b11,x:b3,y:b4,",
"03000000c82d00000851000000000000,8BitDo P30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a2,rightshoulder:b7,righttrigger:b9,rightx:a3,righty:a5,start:b11,x:b3,y:b4,",
"03000000c82d00000360000000000000,8BitDo Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00000361000000000000,8BitDo Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00000660000000000000,8BitDo Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00000131000000000000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00000231000000000000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00000331000000000000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00000431000000000000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00002867000000000000,8BitDo S30,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b8,lefttrigger:b9,leftx:a0,lefty:a2,rightshoulder:b6,righttrigger:b7,rightx:a3,righty:a5,start:b10,x:b3,y:b4,",
"03000000c82d00000130000000000000,8BitDo SF30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b4,y:b3,",
"03000000c82d00000060000000000000,8BitDo SF30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000061000000000000,8BitDo SF30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000102800000900000000000000,8BitDo SFC30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d000021ab000000000000,8BitDo SFC30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d00003028000000000000,8BitDo SFC30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"030000003512000020ab000000000000,8BitDo SN30,a:b0,b:b1,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b11,x:b4,y:b3,",
"03000000c82d00000030000000000000,8BitDo SN30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d00000351000000000000,8BitDo SN30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a2,rightshoulder:b7,rightx:a3,righty:a5,start:b11,x:b4,y:b3,",
"03000000c82d00001290000000000000,8BitDo SN30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d000020ab000000000000,8BitDo SN30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b4,y:b3,",
"03000000c82d00004028000000000000,8BitDo SN30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b4,y:b3,",
"03000000c82d00006228000000000000,8BitDo SN30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d00000021000000000000,8BitDo SN30 Pro,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"03000000c82d00000160000000000000,8BitDo SN30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000161000000000000,8BitDo SN30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000260000000000000,8BitDo SN30 Pro Plus,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000261000000000000,8BitDo SN30 Pro Plus,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00001230000000000000,8BitDo Ultimate,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,paddle1:b2,paddle2:b5,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c82d00001b30000000000000,8BitDo Ultimate 2C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,paddle1:b5,paddle2:b2,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c82d00001d30000000000000,8BitDo Ultimate 2C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,paddle1:b5,paddle2:b2,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c82d00001530000000000000,8BitDo Ultimate C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c82d00001630000000000000,8BitDo Ultimate C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c82d00001730000000000000,8BitDo Ultimate C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c82d00001130000000000000,8BitDo Ultimate Wired,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,misc1:b26,paddle1:b24,paddle2:b25,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c82d00001330000000000000,8BitDo Ultimate Wireless,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b26,paddle1:b23,paddle2:b19,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000121000000000000,8BitDo Xbox One SN30 Pro,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000a00500003232000000000000,8BitDo Zero,a:b0,b:b1,back:b10,dpdown:+a2,dpleft:-a0,dpright:+a0,dpup:-a2,leftshoulder:b6,rightshoulder:b7,start:b11,x:b3,y:b4,",
"03000000c82d00001890000000000000,8BitDo Zero 2,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d00003032000000000000,8BitDo Zero 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"030000008f0e00001200000000000000,Acme GA02,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b2,y:b3,",
"03000000c01100000355000000000000,Acrux,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000fa190000f0ff000000000000,Acteck AGJ 3200,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000d1180000402c000000000000,ADT1,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a3,rightx:a2,righty:a5,x:b3,y:b4,",
"030000006f0e00008801000000000000,Afterglow Deluxe Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000341a00003608000000000000,Afterglow PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00000263000000000000,Afterglow PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00001101000000000000,Afterglow PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00001401000000000000,Afterglow PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00001402000000000000,Afterglow PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00001901000000000000,Afterglow PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00001a01000000000000,Afterglow PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00001301000000000000,Afterglow Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00001302000000000000,Afterglow Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00001304000000000000,Afterglow Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00001413000000000000,Afterglow Xbox Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00003901000000000000,Afterglow Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000ab1200000103000000000000,Afterglow Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000ad1b000000f9000000000000,Afterglow Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000100000008200000000000000,Akishop Customs PS360,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000007c1800000006000000000000,Alienware Dual Compatible PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b3,",
"03000000491900001904000000000000,Amazon Luna Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b9,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b7,x:b2,y:b3,",
"03000000710100001904000000000000,Amazon Luna Controller,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b11,leftshoulder:b5,leftstick:b8,leftx:a0,lefty:a1,misc1:b9,rightshoulder:b4,rightstick:b7,rightx:a3,righty:a4,start:b6,x:b3,y:b2,",
"0300000008100000e501000000000000,Anbernic Game Pad,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a4,start:b11,x:b3,y:b4,",
"03000000020500000913000000000000,Anbernic RG P01,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a5,start:b11,x:b3,y:b4,",
"03000000373500000710000000000000,Anbernic RG P01,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000373500004610000000000000,Anbernic RG P01,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000830500000160000000000000,Arcade,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b3,x:b4,y:b4,",
"03000000120c0000100e000000000000,Armor 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000490b00004406000000000000,ASCII Seamic Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b9,x:b3,y:b4,",
"03000000869800002500000000000000,Astro C40 TR PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000a30c00002700000000000000,Astro City Mini,a:b2,b:b1,back:b8,dpdown:+a4,dpleft:-a3,dpright:+a3,dpup:-a4,rightshoulder:b4,righttrigger:b5,start:b9,x:b3,y:b0,",
"03000000a30c00002800000000000000,Astro City Mini,a:b2,b:b1,back:b8,leftx:a3,lefty:a4,rightshoulder:b4,righttrigger:b5,start:b9,x:b3,y:b0,",
"03000000050b00000579000000000000,ASUS ROG Kunai 3,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000050b00000679000000000000,ASUS ROG Kunai 3,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000503200000110000000000000,Atari VCS Classic Controller,a:b0,b:b1,back:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,start:b3,",
"03000000503200000210000000000000,Atari VCS Modern Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,rightx:a3,righty:a4,start:b9,x:b2,y:b3,",
"03000000380800001889000000000000,AtGames Legends Gamer Pro,a:b1,b:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b13,lefttrigger:b14,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000008a3500000102000000000000,Backbone One,a:b4,b:b5,back:b14,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b16,leftshoulder:b10,leftstick:b17,lefttrigger:b12,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b18,righttrigger:b13,rightx:a3,righty:a4,start:b15,x:b7,y:b8,",
"030000008a3500000201000000000000,Backbone One,a:b4,b:b5,back:b14,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b16,leftshoulder:b10,leftstick:b17,lefttrigger:b12,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b18,righttrigger:b13,rightx:a3,righty:a4,start:b15,x:b7,y:b8,",
"030000008a3500000302000000000000,Backbone One,a:b4,b:b5,back:b14,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b16,leftshoulder:b10,leftstick:b17,lefttrigger:b12,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b18,righttrigger:b13,rightx:a3,righty:a4,start:b15,x:b7,y:b8,",
"030000008a3500000402000000000000,Backbone One,a:b4,b:b5,back:b14,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b16,leftshoulder:b10,leftstick:b17,lefttrigger:b12,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b18,righttrigger:b13,rightx:a3,righty:a4,start:b15,x:b7,y:b8,",
"03000000e4150000103f000000000000,Batarang,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d6200000e557000000000000,Batarang PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000c01100001352000000000000,Battalife Joystick,a:b6,b:b7,back:b2,leftshoulder:b0,leftx:a0,lefty:a1,rightshoulder:b1,start:b3,x:b4,y:b5,",
"030000006f0e00003201000000000000,Battlefield 4 PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000ad1b000001f9000000000000,BB 070,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d62000002a79000000000000,BDA PS4 Fightpad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000bc2000005250000000000000,Beitong G3,a:b0,b:b1,back:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b11,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b5,righttrigger:b9,rightx:a3,righty:a4,start:b15,x:b3,y:b4,",
"030000000d0500000208000000000000,Belkin Nostromo N40,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,rightx:a5,righty:a2,start:b9,x:b2,y:b3,",
"03000000bc2000006012000000000000,Betop 2126F,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000bc2000000055000000000000,Betop BFM,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000bc2000006312000000000000,Betop Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000bc2000006321000000000000,Betop Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000bc2000006412000000000000,Betop Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000c01100000555000000000000,Betop Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000c01100000655000000000000,Betop Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000790000000700000000000000,Betop Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a4,start:b9,x:b3,y:b0,",
"03000000808300000300000000000000,Betop Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a4,start:b9,x:b3,y:b0,",
"030000006f0e00006401000000000000,BF One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a2,righty:a5,start:b7,x:b2,y:b3,",
"03000000300f00000202000000000000,Bigben,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a5,righty:a2,start:b7,x:b2,y:b3,",
"030000006b1400000209000000000000,Bigben,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006b1400000055000000000000,Bigben PS3 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000006b1400000103000000000000,Bigben PS3 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b2,",
"03000000120c0000200e000000000000,Brook Mars PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000210e000000000000,Brook Mars PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000f10e000000000000,Brook PS2 Adapter,a:b1,b:b2,back:b13,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000120c0000310c000000000000,Brook Super Converter,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b5,start:b9,x:b3,y:b0,",
"03000000d81d00000b00000000000000,Buffalo BSGP1601 Series,a:b5,b:b3,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b8,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b9,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b13,x:b4,y:b2,",
"030000005a1c00002400000000000000,Capcom Home Arcade Controller,a:b3,b:b4,back:b7,leftshoulder:b2,leftx:a0,lefty:a1,rightshoulder:b5,start:b6,x:b0,y:b1,",
"030000005b1c00002400000000000000,Capcom Home Arcade Controller,a:b3,b:b4,back:b7,leftshoulder:b2,leftx:a0,lefty:a1,rightshoulder:b5,start:b6,x:b0,y:b1,",
"030000005b1c00002500000000000000,Capcom Home Arcade Controller,a:b3,b:b4,back:b7,leftshoulder:b2,leftx:a0,lefty:a1,rightshoulder:b5,start:b6,x:b0,y:b1,",
"030000006d04000042c2000000000000,ChillStream,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000e82000006058000000000000,Cideko AK08b,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000457500000401000000000000,Cobra,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000b0400003365000000000000,Competition Pro,a:b0,b:b1,back:b2,leftx:a0,lefty:a1,start:b3,",
"030000004c050000c505000000000000,CronusMax Adapter,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000d814000007cd000000000000,Cthulhu,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000d8140000cefa000000000000,Cthulhu,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000260900008888000000000000,Cyber Gadget GameCube Controller,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:a4,rightx:a2,righty:a3~,start:b7,x:b2,y:b3,",
"030000003807000002cb000000000000,Cyborg,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000a306000022f6000000000000,Cyborg V.3 Rumble,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:+a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:-a3,rightx:a2,righty:a4,start:b9,x:b0,y:b3,",
"03000000f806000000a3000000000000,DA Leader,a:b7,b:b6,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b0,leftstick:b8,lefttrigger:b1,leftx:a0,lefty:a1,rightshoulder:b2,rightstick:b9,righttrigger:b3,rightx:a2,righty:a3,start:b12,x:b4,y:b5,",
"030000001a1c00000001000000000000,Datel Arcade Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000451300000830000000000000,Defender Game Racer X7,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000791d00000103000000000000,Dual Box Wii,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b6,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b5,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000c0160000e105000000000000,Dual Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a4,start:b9,x:b0,y:b3,",
"030000004f040000070f000000000000,Dual Power,a:b8,b:b9,back:b4,dpdown:b1,dpleft:b2,dpright:b3,dpup:b0,leftshoulder:b13,leftstick:b6,lefttrigger:b14,leftx:a0,lefty:a1,rightshoulder:b12,rightstick:b7,righttrigger:b15,start:b5,x:b10,y:b11,",
"030000004f04000012b3000000000000,Dual Power 3,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"030000004f04000020b3000000000000,Dual Trigger,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"03000000bd12000002e0000000000000,Dual Vibration Joystick,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b9,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b10,righttrigger:b5,rightx:a3,righty:a2,start:b11,x:b3,y:b0,",
"03000000ff1100003133000000000000,DualForce,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a4,start:b9,x:b0,y:b1,",
"030000006f0e00003001000000000000,EA Sports PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000fc0400000250000000000000,Easy Grip,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b9,x:b3,y:b4,",
"03000000bc2000000091000000000000,EasySMX Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000006e0500000a20000000000000,Elecom DUX60 MMO,a:b2,b:b3,back:b17,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b8,leftstick:b14,lefttrigger:b12,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b15,righttrigger:b13,rightx:a3,righty:a4,start:b20,x:b0,y:b1,",
"03000000b80500000410000000000000,Elecom Gamepad,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b1,",
"03000000b80500000610000000000000,Elecom Gamepad,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b1,",
"03000095090000010000000000000000,Elecom JC-U609,a:b0,b:b1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,start:b8,x:b3,y:b4,",
"0300004112000000e500000000000000,Elecom JC-U909Z,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b7,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,start:b8,x:b3,y:b4,",
"03000041120000001050000000000000,Elecom JC-U911,a:b1,b:b2,back:b3,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,start:b0,x:b4,y:b5,",
"030000006e0500000520000000000000,Elecom P301U PlayStation Controller Adapter,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3,start:b11,x:b0,y:b1,",
"03000000411200004450000000000000,Elecom U1012,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3,start:b11,x:b0,y:b1,",
"030000006e0500000320000000000000,Elecom U3613M,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3,start:b11,x:b0,y:b1,",
"030000006e0500000e20000000000000,Elecom U3912T,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3,start:b11,x:b0,y:b1,",
"030000006e0500000f20000000000000,Elecom U4013S,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3,start:b11,x:b0,y:b1,",
"030000006e0500001320000000000000,Elecom U4113,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006e0500001020000000000000,Elecom U4113S,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a3,righty:a2,start:b11,x:b0,y:b1,",
"030000006e0500000720000000000000,Elecom W01U,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b1,",
"030000007d0400000640000000000000,Eliminator AfterShock,a:b1,b:b2,back:b9,dpdown:+a3,dpleft:-a5,dpright:+a5,dpup:-a3,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a4,righty:a2,start:b8,x:b0,y:b3,",
"03000000120c0000f61c000000000000,Elite,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000430b00000300000000000000,EMS Production PS2 Adapter,a:b2,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"03000000062000001801000000000000,EMS TrioLinker Plus II,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b2,y:b3,",
"03000000242f000000b7000000000000,ESM 9110,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"03000000101c0000181c000000000000,Essential,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b4,leftx:a1,lefty:a0,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"030000008f0e00000f31000000000000,EXEQ,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b2,",
"03000000341a00000108000000000000,EXEQ RF Gamepad,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000006f0e00008401000000000000,Faceoff Deluxe Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00008101000000000000,Faceoff Deluxe Pro Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00008001000000000000,Faceoff Pro Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000021000000090000000000000,FC30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b8,leftstick:b13,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b9,rightstick:b14,righttrigger:b7,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"0300000011040000c600000000000000,FC801,a:b0,b:b1,back:b6,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,start:b7,x:b2,y:b3,",
"03000000852100000201000000000000,FF GP1,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000ad1b000028f0000000000000,Fightpad,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b11,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000ad1b00002ef0000000000000,Fightpad,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000ad1b000038f0000000000000,Fightpad TE,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b8,rightshoulder:b5,righttrigger:b9,start:b7,x:b2,y:b3,",
"03005036852100000000000000000000,Final Fantasy XIV Online Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000f806000001a3000000000000,Firestorm,a:b9,b:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b0,leftstick:b10,lefttrigger:b1,leftx:a0,lefty:a1,rightshoulder:b2,rightstick:b11,righttrigger:b3,start:b12,x:b8,y:b4,",
"03000000b50700000399000000000000,Firestorm 2,a:b2,b:b4,back:b10,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b8,righttrigger:b9,start:b11,x:b3,y:b5,",
"03000000b50700001302000000000000,Firestorm D3,a:b0,b:b2,leftshoulder:b4,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,x:b1,y:b3,",
"03000000b40400001024000000000000,Flydigi Apex,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"03000000151900004000000000000000,Flydigi Vader 2,a:b27,b:b26,back:b19,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b6,leftshoulder:b23,leftstick:b17,lefttrigger:b21,leftx:a0,lefty:a1,rightshoulder:b22,rightstick:b16,righttrigger:b20,rightx:a3,righty:a4,start:b18,x:b25,y:b24,",
"03000000b40400001124000000000000,Flydigi Vader 2,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b12,lefttrigger:b8,leftx:a0,lefty:a1,misc1:b14,paddle1:b4,paddle2:b5,paddle3:b16,paddle4:b17,rightshoulder:b7,rightstick:b13,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b2,y:b3,",
"03000000b40400001224000000000000,Flydigi Vader 2,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b12,lefttrigger:b8,leftx:a0,lefty:a1,misc1:b2,paddle1:b16,paddle2:b17,paddle3:b14,paddle4:b15,rightshoulder:b7,rightstick:b13,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"030000008305000000a0000000000000,G08XU,a:b0,b:b1,back:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b5,x:b2,y:b3,",
"0300000066f700000100000000000000,Game VIB Joystick,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a3,righty:a2,start:b11,x:b0,y:b1,",
"03000000260900002625000000000000,GameCube Controller,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b6,lefttrigger:a4,leftx:a0,lefty:a1,righttrigger:a5,rightx:a2,righty:a3,start:b7,x:b2,y:b3,",
"03000000341a000005f7000000000000,GameCube Controller,a:b2,b:b3,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b1,y:b0,",
"03000000430b00000500000000000000,GameCube Controller,a:b0,b:b2,dpdown:b10,dpleft:b8,dpright:b9,dpup:b11,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:a3,rightx:a5,righty:a2,start:b7,x:b1,y:b3,",
"03000000790000004718000000000000,GameCube Controller,a:b1,b:b0,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b2,y:b3,",
"03000000790000004618000000000000,GameCube Controller Adapter,a:b1,b:b0,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b2,y:b3,",
"030000008f0e00000d31000000000000,Gamepad 3 Turbo,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000ac0500003d03000000000000,GameSir G3,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000ac0500005b05000000000000,GameSir G3w,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000ac0500002d02000000000000,GameSir G4,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"03000000ac0500004d04000000000000,GameSir G4,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000ac0500001a06000000000000,GameSir-T3 2.02,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"030000004c0e00001035000000000000,Gamester,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"030000000d0f00001110000000000000,GameStick Controller,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"0300000047530000616d000000000000,GameStop,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000c01100000140000000000000,GameStop PS4 Fun Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000b62500000100000000000000,Gametel GT004 01,a:b3,b:b0,dpdown:b10,dpleft:b9,dpright:b8,dpup:b11,leftshoulder:b4,rightshoulder:b5,start:b7,x:b1,y:b2,",
"030000008f0e00001411000000000000,Gamo2 Divaller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000120c0000a857000000000000,Gator Claw,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000c9110000f055000000000000,GC100XF,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000008305000009a0000000000000,Genius,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000008305000031b0000000000000,Genius Maxfire Blaze 3,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000451300000010000000000000,Genius Maxfire Grandias 12,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000005c1a00003330000000000000,Genius MaxFire Grandias 12V,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b9,x:b2,y:b3,",
"03000000300f00000b01000000000000,GGE909 Recoil,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"03000000f0250000c283000000000000,Gioteck PlayStation Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000f025000021c1000000000000,Gioteck PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000f025000031c1000000000000,Gioteck PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000f0250000c383000000000000,Gioteck VX2 PlayStation Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000f0250000c483000000000000,Gioteck VX2 PlayStation Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000d11800000094000000000000,Google Stadia Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:b12,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:b11,rightx:a3,righty:a4,start:b9,x:b2,y:b3,",
"030000004f04000026b3000000000000,GP XID,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"0300000079000000d418000000000000,GPD Win,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c6240000025b000000000000,GPX,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000007d0400000840000000000000,Gravis Destroyer Tilt,+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b1,b:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,x:b0,y:b3,",
"030000007d0400000540000000000000,Gravis Eliminator Pro,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000280400000140000000000000,Gravis GamePad Pro,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a3,dpup:-a4,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000008f0e00000610000000000000,GreenAsia,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b9,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b10,righttrigger:b5,rightx:a5,righty:a2,start:b11,x:b3,y:b0,",
"03000000ac0500006b05000000000000,GT2a,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"03000000341a00000302000000000000,Hama Scorpad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00004900000000000000,Hatsune Miku Sho PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000001008000001e1000000000000,Havit HV G60,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b3,y:b0,",
"030000000d0f00000c00000000000000,HEXT,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d81400000862000000000000,HitBox Edition Cthulhu,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b5,lefttrigger:b4,rightshoulder:b7,righttrigger:b6,start:b9,x:b0,y:b3,",
"03000000632500002605000000000000,HJD X,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"030000000d0f00000a00000000000000,Hori DOA,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f00008500000000000000,Hori Fighting Commander 2016 PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00002500000000000000,Hori Fighting Commander 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f00002d00000000000000,Hori Fighting Commander 3 Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00005f00000000000000,Hori Fighting Commander 4 PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00005e00000000000000,Hori Fighting Commander 4 PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f00008400000000000000,Hori Fighting Commander 5,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00006201000000000000,Hori Fighting Commander Octa,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f00006401000000000000,Hori Fighting Commander Octa,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:+a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:-a2,start:b7,x:b2,y:b3,",
"030000000d0f00005100000000000000,Hori Fighting Commander PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00008600000000000000,Hori Fighting Commander Xbox 360,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f0000ba00000000000000,Hori Fighting Commander Xbox 360,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f00008800000000000000,Hori Fighting Stick mini 4 PS3,a:b1,b:b2,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b8,x:b0,y:b3,",
"030000000d0f00008700000000000000,Hori Fighting Stick mini 4 PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f00001000000000000000,Hori Fightstick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00003200000000000000,Hori Fightstick 3W,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f0000c000000000000000,Hori Fightstick 4,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f00000d00000000000000,Hori Fightstick EX2,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"030000000d0f00003701000000000000,Hori Fightstick Mini,a:b1,b:b0,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a3,righty:a4,start:b7,x:b3,y:b2,",
"030000000d0f00004000000000000000,Hori Fightstick Mini 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b5,lefttrigger:b4,rightshoulder:b7,righttrigger:b6,start:b9,x:b0,y:b3,",
"030000000d0f00002100000000000000,Hori Fightstick V3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00002700000000000000,Hori Fightstick V3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f0000a000000000000000,Hori Grip TAC4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b13,x:b0,y:b3,",
"030000000d0f0000a500000000000000,Hori Miku Project Diva X HD PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f0000a600000000000000,Hori Miku Project Diva X HD PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f00000101000000000000,Hori Mini Hatsune Miku FT,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00005400000000000000,Hori Pad 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00000900000000000000,Hori Pad 3 Turbo,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00004d00000000000000,Hori Pad A,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00003801000000000000,Hori PC Engine Mini Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,start:b9,",
"030000000d0f00009200000000000000,Hori Pokken Tournament DX Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f00002301000000000000,Hori PS4 Controller Light,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"030000000d0f00001100000000000000,Hori Real Arcade Pro 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,rightshoulder:b5,rightstick:b11,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f00002600000000000000,Hori Real Arcade Pro 3P,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00004b00000000000000,Hori Real Arcade Pro 3W,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00006a00000000000000,Hori Real Arcade Pro 4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00006b00000000000000,Hori Real Arcade Pro 4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00008a00000000000000,Hori Real Arcade Pro 4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00008b00000000000000,Hori Real Arcade Pro 4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00006f00000000000000,Hori Real Arcade Pro 4 VLX,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00007000000000000000,Hori Real Arcade Pro 4 VLX,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,rightshoulder:b5,rightstick:b11,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f00003d00000000000000,Hori Real Arcade Pro N3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b10,leftstick:b4,lefttrigger:b11,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b6,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f0000ae00000000000000,Hori Real Arcade Pro N4,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f00008c00000000000000,Hori Real Arcade Pro P4,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f0000aa00000000000000,Hori Real Arcade Pro S,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f0000d800000000000000,Hori Real Arcade Pro S,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"030000000d0f00002200000000000000,Hori Real Arcade Pro V3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00005b00000000000000,Hori Real Arcade Pro V4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00005c00000000000000,Hori Real Arcade Pro V4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f0000af00000000000000,Hori Real Arcade Pro VHS,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b0,y:b3,",
"030000000d0f00001b00000000000000,Hori Real Arcade Pro VX,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000ad1b000002f5000000000000,Hori Real Arcade Pro VX,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b11,rightx:a2,righty:a5,start:b6,x:b2,y:b3,",
"030000000d0f00009c00000000000000,Hori TAC Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f0000c900000000000000,Hori Taiko Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00006400000000000000,Horipad 3TP,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00001300000000000000,Horipad 3W,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00005500000000000000,Horipad 4 FPS,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00006e00000000000000,Horipad 4 PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00006600000000000000,Horipad 4 PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f00004200000000000000,Horipad A,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000ad1b000001f5000000000000,Horipad EXT2,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f0000ee00000000000000,Horipad Mini 4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f0000c100000000000000,Horipad Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f0000f600000000000000,Horipad Nintendo Switch Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000000d0f00006700000000000000,Horipad One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f00009601000000000000,Horipad Steam,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,misc2:b2,paddle1:b5,paddle2:b15,paddle3:b18,paddle4:b19,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"030000000d0f0000dc00000000000000,Horipad Switch,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000242e00000b20000000000000,Hyperkin Admiral N64 Controller,+rightx:b11,+righty:b13,-rightx:b8,-righty:b12,a:b1,b:b0,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b14,leftx:a0,lefty:a1,rightshoulder:b5,start:b9,",
"03000000242e0000ff0b000000000000,Hyperkin N64 Adapter,a:b1,b:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightx:a2,righty:a3,start:b9,",
"03000000790000004e95000000000000,Hyperkin N64 Controller Adapter,a:b1,b:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b7,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b5,rightx:a5,righty:a2,start:b9,",
"03000000242e00006a48000000000000,Hyperkin RetroN Sq,a:b3,b:b7,back:b5,dpdown:+a4,dpleft:-a0,dpright:+a0,dpup:-a4,leftshoulder:b0,rightshoulder:b1,start:b4,x:b2,y:b6,",
"03000000242f00000a20000000000000,Hyperkin Scout,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,rightshoulder:b5,start:b9,x:b2,y:b3,",
"03000000242e00000a20000000000000,Hyperkin Scout Premium SNES Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,rightshoulder:b5,start:b9,x:b2,y:b3,",
"03000000242e00006a38000000000000,Hyperkin Trooper 2,a:b0,b:b1,back:b4,leftshoulder:b2,leftx:a0,lefty:a1,rightshoulder:b3,start:b5,",
"03000000d81d00000e00000000000000,iBuffalo AC02 Arcade Joystick,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b2,rightstick:b11,righttrigger:b3,rightx:a2,righty:a5,start:b8,x:b4,y:b5,",
"03000000d81d00000f00000000000000,iBuffalo BSGP1204 Series,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000d81d00001000000000000000,iBuffalo BSGP1204P Series,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000005c0a00000285000000000000,iDroidCon,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b4,y:b6,",
"03000000696400006964000000000000,iDroidCon Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000511d00000230000000000000,iGUGU Gamecore,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b1,leftstick:b4,lefttrigger:b3,leftx:a0,lefty:a1,rightshoulder:b0,righttrigger:b2,",
"03000000b50700001403000000000000,Impact Black,a:b2,b:b3,back:b8,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b1,",
"030000006f0e00002401000000000000,Injustice Fightstick PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000830500005130000000000000,InterAct ActionPad,a:b0,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b7,rightshoulder:b5,righttrigger:b2,start:b9,x:b3,y:b4,",
"03000000ef0500000300000000000000,InterAct AxisPad,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a3,righty:a2,start:b11,x:b0,y:b1,",
"03000000fd0500000230000000000000,InterAct AxisPad,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a5,start:b11,x:b0,y:b1,",
"03000000fd0500000030000000000000,Interact GoPad,a:b3,b:b4,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,x:b0,y:b1,",
"03000000fd0500003902000000000000,InterAct Hammerhead,a:b3,b:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b2,lefttrigger:b8,rightshoulder:b7,rightstick:b5,righttrigger:b9,start:b10,x:b0,y:b1,",
"03000000fd0500002a26000000000000,InterAct Hammerhead FX,a:b3,b:b4,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b0,y:b1,",
"03000000fd0500002f26000000000000,InterAct Hammerhead FX,a:b4,b:b5,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b1,y:b2,",
"03000000fd0500005302000000000000,InterAct ProPad,a:b3,b:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,x:b0,y:b1,",
"03000000ac0500002c02000000000000,Ipega Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b8,leftstick:b13,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b9,rightstick:b14,righttrigger:b7,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000491900000204000000000000,Ipega PG9023,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000491900000304000000000000,Ipega PG9087,+righty:+a5,-righty:-a4,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,start:b11,x:b3,y:b4,",
"030000007e0500000620000000000000,Joy-Con (L),+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b13,leftshoulder:b4,leftstick:b10,rightshoulder:b5,start:b8,x:b2,y:b3,",
"030000007e0500000720000000000000,Joy-Con (R),+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b12,leftshoulder:b4,leftstick:b11,rightshoulder:b5,start:b9,x:b2,y:b3,",
"03000000250900000017000000000000,Joypad Adapter,a:b2,b:b1,back:b9,leftshoulder:b5,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b6,start:b8,x:b3,y:b0,",
"03000000bd12000003c0000000000000,Joypad Alpha Shock,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000ff1100004033000000000000,JPD FFB,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a2,start:b15,x:b3,y:b0,",
"03000000242f00002d00000000000000,JYS Adapter,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000242f00008a00000000000000,JYS Adapter,a:b1,b:b4,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b0,y:b3,",
"03000000c4100000c082000000000000,KADE,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000828200000180000000000000,Keio,a:b4,b:b5,back:b8,leftshoulder:b2,lefttrigger:b3,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,start:b9,x:b0,y:b1,",
"03000000790000000200000000000000,King PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a4,start:b9,x:b3,y:b0,",
"03000000bd12000001e0000000000000,Leadership,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"030000006f0e00000103000000000000,Logic3,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00000104000000000000,Logic3,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000008f0e00001300000000000000,Logic3,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000006d040000d1ca000000000000,Logitech ChillStream,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d040000d2ca000000000000,Logitech Cordless Precision,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000011c2000000000000,Logitech Cordless Wingman,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b9,leftstick:b5,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b2,righttrigger:b7,rightx:a3,righty:a4,x:b4,",
"030000006d04000016c2000000000000,Logitech Dual Action,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d0400001dc2000000000000,Logitech F310,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006d04000018c2000000000000,Logitech F510,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d0400001ec2000000000000,Logitech F510,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006d04000019c2000000000000,Logitech F710,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d0400001fc2000000000000,Logitech F710,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006d0400001ac2000000000000,Logitech Precision,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000006d04000009c2000000000000,Logitech WingMan,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b8,x:b3,y:b4,",
"030000006d0400000bc2000000000000,Logitech WingMan Action Pad,a:b0,b:b1,back:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b8,lefttrigger:a5~,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b5,righttrigger:a2~,start:b8,x:b3,y:b4,",
"030000006d0400000ac2000000000000,Logitech WingMan RumblePad,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,rightx:a3,righty:a4,x:b3,y:b4,",
"03000000380700005645000000000000,Lynx,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000222200006000000000000000,Macally,a:b1,b:b2,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b33,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700003888000000000000,Mad Catz Arcade Fightstick TE S Plus PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700008532000000000000,Mad Catz Arcade Fightstick TE S PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700006352000000000000,Mad Catz CTRLR,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000380700006652000000000000,Mad Catz CTRLR,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a4,start:b9,x:b0,y:b3,",
"03000000380700005032000000000000,Mad Catz Fightpad Pro PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700005082000000000000,Mad Catz Fightpad Pro PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000380700008031000000000000,Mad Catz FightStick Alpha PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000003807000038b7000000000000,Mad Catz Fightstick TE,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b8,rightshoulder:b5,righttrigger:b9,start:b7,x:b2,y:b3,",
"03000000380700008433000000000000,Mad Catz Fightstick TE S PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700008483000000000000,Mad Catz Fightstick TE S PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000380700008134000000000000,Mad Catz Fightstick TE2 PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b7,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b4,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700008184000000000000,Mad Catz Fightstick TE2 PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b5,leftstick:b10,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000380700006252000000000000,Mad Catz Micro CTRLR,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a4,start:b9,x:b0,y:b3,",
"03000000380700008232000000000000,Mad Catz PlayStation Brawlpad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700008731000000000000,Mad Catz PlayStation Fightstick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000003807000056a8000000000000,Mad Catz PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700001888000000000000,Mad Catz SFIV Fightstick PS3,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b5,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b4,righttrigger:b6,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000380700008081000000000000,Mad Catz SFV Arcade Fightstick Alpha PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000380700001847000000000000,Mad Catz Street Fighter 4 Xbox 360 FightStick,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b8,rightshoulder:b5,righttrigger:b9,start:b7,x:b2,y:b3,",
"03000000380700008034000000000000,Mad Catz TE2 PS3 Fightstick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700008084000000000000,Mad Catz TE2 PS4 Fightstick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000002a0600001024000000000000,Matricom,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a4,start:b9,x:b2,y:b3,",
"030000009f000000adbb000000000000,MaxJoypad Virtual Controller,a:b1,b:b2,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"03000000250900000128000000000000,Mayflash Arcade Stick,a:b1,b:b2,back:b8,leftshoulder:b0,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b3,righttrigger:b7,start:b9,x:b5,y:b6,",
"030000008f0e00001330000000000000,Mayflash Controller Adapter,a:b1,b:b2,back:b8,dpdown:h0.8,dpleft:h0.2,dpright:h0.1,dpup:h0.4,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a3~,righty:a2,start:b9,x:b0,y:b3,",
"03000000242f00003700000000000000,Mayflash F101,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000790000003018000000000000,Mayflash F300 Arcade Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000242f00003900000000000000,Mayflash F300 Elite Arcade Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000790000004418000000000000,Mayflash GameCube Controller,a:b1,b:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b0,y:b3,",
"03000000790000004318000000000000,Mayflash GameCube Controller Adapter,a:b1,b:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b0,y:b3,",
"03000000242f00007300000000000000,Mayflash Magic NS,a:b1,b:b4,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b0,y:b3,",
"0300000079000000d218000000000000,Mayflash Magic NS,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000d620000010a7000000000000,Mayflash Magic NS,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000242f0000f500000000000000,Mayflash N64 Adapter,a:b2,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightx:a2,righty:a5,start:b9,",
"03000000242f0000f400000000000000,Mayflash N64 Controller Adapter,a:b1,b:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightx:a2,righty:a5,start:b9,",
"03000000790000007918000000000000,Mayflash N64 Controller Adapter,a:b1,b:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b9,leftx:a0,lefty:a1,righttrigger:b7,rightx:a3,righty:a2,start:b8,",
"030000008f0e00001030000000000000,Mayflash Saturn Adapter,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,lefttrigger:b7,rightshoulder:b6,righttrigger:b2,start:b9,x:b3,y:b4,",
"0300000025090000e803000000000000,Mayflash Wii Classic Adapter,a:b1,b:b0,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:a4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:a5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b2,",
"03000000790000000318000000000000,Mayflash Wii DolphinBar,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b11,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b1,",
"03000000790000000018000000000000,Mayflash Wii U Pro Adapter,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000790000002418000000000000,Mega Drive Controller,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,rightshoulder:b2,start:b9,x:b3,y:b4,",
"0300000079000000ae18000000000000,Mega Drive Controller,a:b0,b:b1,back:b7,dpdown:b14,dpleft:b15,dpright:b13,dpup:b2,rightshoulder:b6,righttrigger:b2,start:b9,x:b3,y:b4,",
"03000000c0160000990a000000000000,Mega Drive Controller,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,righttrigger:b2,start:b3,",
"030000005e0400002800000000000000,Microsoft Dual Strike,a:b3,b:b2,back:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b8,rightshoulder:b7,rightx:a0,righty:a1~,start:b5,x:b1,y:b0,",
"030000005e0400000300000000000000,Microsoft SideWinder,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b8,x:b3,y:b4,",
"030000005e0400000700000000000000,Microsoft SideWinder,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b9,x:b3,y:b4,",
"030000005e0400000e00000000000000,Microsoft SideWinder Freestyle Pro,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,start:b8,x:b3,y:b4,",
"030000005e0400002700000000000000,Microsoft SideWinder Plug and Play,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,lefttrigger:b4,righttrigger:b5,x:b2,y:b3,",
"03000000280d00000202000000000000,Miller Lite Cantroller,a:b0,b:b1,back:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,start:b5,x:b2,y:b3,",
"03000000ad1b000023f0000000000000,MLG,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a6,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"03000000ad1b00003ef0000000000000,MLG Fightstick TE,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b8,rightshoulder:b5,righttrigger:b9,start:b7,x:b2,y:b3,",
"03000000380700006382000000000000,MLG PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000004523000015e0000000000000,Mobapad Chitu HD,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000491900000904000000000000,Mobapad Chitu HD,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000ffff00000000000000000000,Mocute M053,a:b3,b:b2,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b11,leftstick:b7,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b6,righttrigger:b4,rightx:a3,righty:a4,start:b8,x:b1,y:b0,",
"03000000d6200000e589000000000000,Moga 2,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a2,righty:a5,start:b7,x:b2,y:b3,",
"03000000d62000007162000000000000,Moga Pro,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a2,righty:a5,start:b7,x:b2,y:b3,",
"03000000d6200000ad0d000000000000,Moga Pro,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c62400002a89000000000000,Moga XP5A Plus,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c62400002b89000000000000,Moga XP5A Plus,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c62400001a89000000000000,Moga XP5X Plus,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c62400001b89000000000000,Moga XP5X Plus,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000250900006688000000000000,MP-8866 Super Dual Box,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"03000000091200004488000000000000,MUSIA PlayStation 2 Input Display,a:b0,b:b2,back:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b8,leftstick:b6,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b9,rightstick:b7,righttrigger:b11,rightx:a2,righty:a3,start:b5,x:b1,y:b3,",
"03000000f70600000100000000000000,N64 Adaptoid,+rightx:b2,+righty:b1,-rightx:b4,-righty:b5,a:b0,b:b3,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b6,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b7,start:b8,",
"030000006b140000010c000000000000,Nacon GC 400ES,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000006b1400001106000000000000,Nacon Revolution 3 PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"0300000085320000170d000000000000,Nacon Revolution 5 Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"0300000085320000190d000000000000,Nacon Revolution 5 Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000006b140000100d000000000000,Nacon Revolution Infinity PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,touchpad:b13,x:b0,y:b3,",
"030000006b140000080d000000000000,Nacon Revolution Unlimited Pro Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000bd12000001c0000000000000,Nebular,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a5,righty:a2,start:b9,x:b3,y:b0,",
"03000000eb0300000000000000000000,NeGcon Adapter,a:a2,b:b13,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,lefttrigger:a4,leftx:a1,righttrigger:b11,start:b3,x:a3,y:b12,",
"0300000038070000efbe000000000000,NEO SE,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"0300000092120000474e000000000000,NeoGeo X Arcade Stick,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,start:b9,x:b3,y:b2,",
"03000000921200004b46000000000000,NES 2 port Adapter,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,start:b11,",
"03000000000f00000100000000000000,NES Controller,a:b1,b:b0,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,start:b3,",
"03000000921200004346000000000000,NES Controller,a:b0,b:b1,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,start:b3,",
"03000000790000004518000000000000,NEXILUX GameCube Controller Adapter,a:b1,b:b0,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b2,y:b3,",
"030000001008000001e5000000000000,NEXT SNES Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,righttrigger:b6,start:b9,x:b3,y:b0,",
"03000000050b00000045000000000000,Nexus,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b10,x:b2,y:b3,",
"03000000152000000182000000000000,NGDS,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a4,start:b9,x:b3,y:b0,",
"030000007e0500000920000000000000,Nintendo Switch Pro Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000000d0500000308000000000000,Nostromo N45,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b12,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b10,x:b2,y:b3,",
"030000007e0500001920000000000000,NSO N64 Controller,+rightx:b8,+righty:b2,-rightx:b3,-righty:b7,a:b1,b:b0,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,righttrigger:b10,start:b9,",
"030000007e0500001720000000000000,NSO SNES Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b15,start:b9,x:b2,y:b3,",
"03000000550900001472000000000000,NVIDIA Controller,a:b11,b:b10,back:b13,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b7,leftstick:b5,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b4,righttrigger:a5,rightx:a3,righty:a6,start:b3,x:b9,y:b8,",
"03000000550900001072000000000000,NVIDIA Shield,a:b9,b:b8,back:b11,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b3,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b2,righttrigger:a4,rightx:a2,righty:a5,start:b0,x:b7,y:b6,",
"030000005509000000b4000000000000,NVIDIA Virtual,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:+a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:-a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000120c00000288000000000000,Nyko Air Flo Xbox Controller,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"030000004b120000014d000000000000,NYKO Airflo EX,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b2,y:b3,",
"03000000d62000001d57000000000000,Nyko Airflo PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000791d00000900000000000000,Nyko Playpad,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"03000000782300000a10000000000000,Onlive Controller,a:b15,b:b14,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b5,leftshoulder:b11,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a3,righty:a4,start:b6,x:b13,y:b12,",
"030000000d0f00000401000000000000,Onyx,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000008916000001fd000000000000,Onza CE,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a3,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000008916000000fd000000000000,Onza TE,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d62000006d57000000000000,OPP PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006b14000001a1000000000000,Orange Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b2,y:b3,",
"0300000009120000072f000000000000,OrangeFox86 DreamPicoPort,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,lefttrigger:-a2,leftx:a0,lefty:a1,righttrigger:-a5,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000362800000100000000000000,OUYA Controller,a:b0,b:b3,dpdown:b9,dpleft:b10,dpright:b11,dpup:b8,guide:b14,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,x:b1,y:b2,",
"03000000120c0000f60e000000000000,P4 Gamepad,a:b1,b:b2,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b5,lefttrigger:b7,rightshoulder:b4,righttrigger:b6,start:b9,x:b0,y:b3,",
"03000000790000002201000000000000,PC Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000006f0e00008501000000000000,PDP Fightpad Pro GameCube Controller,a:b1,b:b0,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000006f0e00000901000000000000,PDP PS3 Versus Fighting,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000006f0e00008901000000000000,PDP Realmz Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000008f0e00004100000000000000,PlaySega,a:b1,b:b0,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b7,rightshoulder:b5,righttrigger:b2,start:b8,x:b4,y:b3,",
"03000000d620000011a7000000000000,PowerA Core Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000dd62000015a7000000000000,PowerA Fusion Nintendo Switch Arcade Stick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d620000012a7000000000000,PowerA Fusion Nintendo Switch Fight Pad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000dd62000016a7000000000000,PowerA Fusion Pro Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d620000013a7000000000000,PowerA Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d62000003340000000000000,PowerA OPS Pro Wireless Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000d62000002640000000000000,PowerA OPS Wireless Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000d62000006dca000000000000,PowerA Pro Ex,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"0300000062060000d570000000000000,PowerA PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d620000014a7000000000000,PowerA Spectra Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000084ca000000000000,Precision,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"03000000d62000009557000000000000,Pro Elite PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000c62400001a53000000000000,Pro Ex Mini,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d62000009f31000000000000,Pro Ex mini PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d6200000c757000000000000,Pro Ex mini PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000120c0000110e000000000000,Pro5,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000100800000100000000000000,PS1 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"030000008f0e00007530000000000000,PS1 Controller,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b1,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000100800000300000000000000,PS2 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a4,righty:a2,start:b9,x:b3,y:b0,",
"03000000250900000088000000000000,PS2 Controller,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"03000000250900006888000000000000,PS2 Controller,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b6,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"03000000250900008888000000000000,PS2 Controller,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"030000006b1400000303000000000000,PS2 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000009d0d00001330000000000000,PS2 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000151a00006222000000000000,PS2 Dual Plus Adapter,a:b2,b:b1,back:b9,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"03000000120a00000100000000000000,PS3 Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"03000000120c00001307000000000000,PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000120c00001cf1000000000000,PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000120c0000f90e000000000000,PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000250900000118000000000000,PS3 Controller,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"03000000250900000218000000000000,PS3 Controller,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"03000000250900000500000000000000,PS3 Controller,a:b2,b:b1,back:b9,dpdown:h0.8,dpleft:h0.4,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b0,y:b3,",
"030000004c0500006802000000000000,PS3 Controller,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b10,lefttrigger:a3~,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:a4~,rightx:a2,righty:a5,start:b8,x:b3,y:b0,",
"030000004f1f00000800000000000000,PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000632500007505000000000000,PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000888800000803000000000000,PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b9,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:b7,rightx:a3,righty:a4,start:b11,x:b3,y:b0,",
"03000000888800000804000000000000,PS3 Controller,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:b9,rightx:a2,righty:a3,start:b3,x:b15,y:b12,",
"030000008f0e00000300000000000000,PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b3,y:b0,",
"030000008f0e00001431000000000000,PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000ba2200002010000000000000,PS3 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a5,righty:a2,start:b9,x:b3,y:b2,",
"03000000120c00000807000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000111e000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000121e000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000130e000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000150e000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000180e000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000181e000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000191e000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c00001e0e000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000a957000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000aa57000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000f21c000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000f31c000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000f41c000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000f51c000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000f70e000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120e0000120c000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000160e0000120c000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000001a1e0000120c000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000a00b000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000c405000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000cc09000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c0500005f0e000000000000,PS5 Access Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b14,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000e60c000000000000,PS5 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b14,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000f20d000000000000,PS5 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b14,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000830500005020000000000000,PSX,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3,start:b11,x:b2,y:b3,",
"03000000300f00000111000000000000,Qanba 2,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000300f00000211000000000000,Qanba 2P,a:b1,b:b0,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000300f00000011000000000000,Qanba Arcade Stick 1008,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,start:b10,x:b0,y:b3,",
"03000000300f00001611000000000000,Qanba Arcade Stick 4018,a:b1,b:b2,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b8,x:b0,y:b3,",
"03000000222c00000025000000000000,Qanba Dragon Arcade Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000222c00000020000000000000,Qanba Drone Arcade Stick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:a3,rightshoulder:b5,righttrigger:a4,start:b9,x:b0,y:b3,",
"03000000300f00001211000000000000,Qanba Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000300f00001210000000000000,Qanba Joystick Plus,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,rightshoulder:b5,start:b9,x:b2,y:b3,",
"03000000341a00000104000000000000,Qanba Joystick Q4RAF,a:b5,b:b6,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b0,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b3,righttrigger:b7,start:b9,x:b1,y:b2,",
"03000000222c00000223000000000000,Qanba Obsidian Arcade Stick PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000222c00000023000000000000,Qanba Obsidian Arcade Stick PS4,a:b1,b:b2,back:b13,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000008a2400006682000000000000,R1 Mobile Controller,a:b3,b:b1,back:b7,leftx:a0,lefty:a1,start:b6,x:b4,y:b0,",
"03000000086700006626000000000000,RadioShack,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b3,y:b0,",
"03000000ff1100004733000000000000,Ramox FPS Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,start:b9,x:b3,y:b0,",
"030000009b2800002300000000000000,Raphnet 3DO Adapter,a:b0,b:b1,back:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b2,start:b3,",
"030000009b2800006900000000000000,Raphnet 3DO Adapter,a:b0,b:b1,back:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b2,start:b3,",
"030000009b2800000800000000000000,Raphnet Dreamcast Adapter,a:b2,b:b1,dpdown:b5,dpleft:b6,dpright:b7,dpup:b4,lefttrigger:a2,leftx:a0,righttrigger:a3,righty:a1,start:b3,x:b10,y:b9,",
"030000009b280000d000000000000000,Raphnet Dreamcast Adapter,a:b1,b:b0,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,lefttrigger:+a5,leftx:a0,lefty:a1,righttrigger:+a2,start:b3,x:b5,y:b4,",
"030000009b2800006200000000000000,Raphnet GameCube Adapter,a:b0,b:b7,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,rightx:a3,righty:a4,start:b3,x:b1,y:b8,",
"030000009b2800003200000000000000,Raphnet GC and N64 Adapter,a:b0,b:b7,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,lefttrigger:+a5,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:+a2,rightx:a3,righty:a4,start:b3,x:b1,y:b8,",
"030000009b2800006000000000000000,Raphnet GC and N64 Adapter,a:b0,b:b7,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,lefttrigger:+a5,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:+a2,rightx:a3,righty:a4,start:b3,x:b1,y:b8,",
"030000009b2800001800000000000000,Raphnet Jaguar Adapter,a:b2,b:b1,back:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b0,righttrigger:b10,start:b3,x:b11,y:b12,",
"030000009b2800003c00000000000000,Raphnet N64 Adapter,+rightx:b9,+righty:b7,-rightx:b8,-righty:b6,a:b0,b:b1,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b4,lefttrigger:b2,leftx:a0,lefty:a1,rightshoulder:b5,start:b3,",
"030000009b2800006100000000000000,Raphnet N64 Adapter,+rightx:b9,+righty:b7,-rightx:b8,-righty:b6,a:b0,b:b1,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b4,lefttrigger:b2,leftx:a0,lefty:a1,rightshoulder:b5,start:b3,",
"030000009b2800006300000000000000,Raphnet N64 Adapter,+rightx:b9,+righty:b7,-rightx:b8,-righty:b6,a:b0,b:b1,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b4,lefttrigger:b2,leftx:a0,lefty:a1,rightshoulder:b5,start:b3,",
"030000009b2800006400000000000000,Raphnet N64 Adapter,+rightx:b9,+righty:b7,-rightx:b8,-righty:b6,a:b0,b:b1,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b4,lefttrigger:b2,leftx:a0,lefty:a1,rightshoulder:b5,start:b3,",
"030000009b2800000200000000000000,Raphnet NES Adapter,a:b7,b:b6,back:b5,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,start:b4,",
"030000009b2800004400000000000000,Raphnet PS1 and PS2 Adapter,a:b1,b:b2,back:b5,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,leftshoulder:b6,leftstick:b10,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b9,rightx:a3,righty:a4,start:b4,x:b0,y:b3,",
"030000009b2800004300000000000000,Raphnet Saturn,a:b0,b:b1,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b8,x:b3,y:b4,",
"030000009b2800000500000000000000,Raphnet Saturn Adapter 2.0,a:b1,b:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b4,rightshoulder:b7,righttrigger:b5,start:b9,x:b0,y:b3,",
"030000009b2800000300000000000000,Raphnet SNES Adapter,a:b0,b:b4,back:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,start:b3,x:b1,y:b5,",
"030000009b2800002600000000000000,Raphnet SNES Adapter,a:b1,b:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b0,y:b5,",
"030000009b2800002e00000000000000,Raphnet SNES Adapter,a:b1,b:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b0,y:b5,",
"030000009b2800002f00000000000000,Raphnet SNES Adapter,a:b1,b:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b0,y:b5,",
"030000009b2800005600000000000000,Raphnet SNES Adapter,a:b1,b:b4,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b0,y:b5,",
"030000009b2800005700000000000000,Raphnet SNES Adapter,a:b1,b:b4,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b0,y:b5,",
"030000009b2800001e00000000000000,Raphnet Vectrex Adapter,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a1,lefty:a2,x:b2,y:b3,",
"030000009b2800002b00000000000000,Raphnet Wii Classic Adapter,a:b1,b:b4,back:b2,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,guide:b10,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a3,righty:a4,start:b3,x:b0,y:b5,",
"030000009b2800002c00000000000000,Raphnet Wii Classic Adapter,a:b1,b:b4,back:b2,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,guide:b10,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a3,righty:a4,start:b3,x:b0,y:b5,",
"030000009b2800008000000000000000,Raphnet Wii Classic Adapter,a:b1,b:b4,back:b2,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,guide:b10,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a3,righty:a4,start:b3,x:b0,y:b5,",
"03000000790000008f18000000000000,Rapoo Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b3,y:b0,",
"0300000032150000a602000000000000,Razer Huntsman V3 Pro,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b12,dpright:b13,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b8,lefttrigger:+a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:-a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000321500000003000000000000,Razer Hydra,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000f8270000bf0b000000000000,Razer Kishi,a:b6,b:b7,back:b16,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b18,leftshoulder:b12,leftstick:b19,lefttrigger:b14,leftx:a0,lefty:a1,rightshoulder:b13,rightstick:b20,righttrigger:b15,rightx:a3,righty:a4,start:b17,x:b9,y:b10,",
"03000000321500000204000000000000,Razer Panthera PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000321500000104000000000000,Razer Panthera PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000321500000010000000000000,Razer Raiju,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000321500000507000000000000,Razer Raiju Mobile,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000321500000707000000000000,Razer Raiju Mobile,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000321500000710000000000000,Razer Raiju TE,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000321500000a10000000000000,Razer Raiju TE,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000321500000410000000000000,Razer Raiju UE,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000321500000910000000000000,Razer Raiju UE,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000321500000011000000000000,Razer Raion PS4 Fightpad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000321500000009000000000000,Razer Serval,+lefty:+a2,-lefty:-a1,a:b0,b:b1,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b8,leftx:a0,rightshoulder:b5,rightstick:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000921200004547000000000000,Retro Bit Sega Genesis Controller Adapter,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,lefttrigger:b7,rightshoulder:b5,righttrigger:b2,start:b6,x:b3,y:b4,",
"03000000790000001100000000000000,Retro Controller,a:b1,b:b2,back:b8,dpdown:+a4,dpleft:-a3,dpright:+a3,dpup:-a4,leftshoulder:b6,lefttrigger:b7,rightshoulder:b4,righttrigger:b5,start:b9,x:b0,y:b3,",
"03000000830500006020000000000000,Retro Controller,a:b0,b:b1,back:b6,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,lefttrigger:b5,rightshoulder:b8,righttrigger:b9,start:b7,x:b2,y:b3,",
"03000000632500007805000000000000,Retro Fighters Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"0300000003040000c197000000000000,Retrode Adapter,a:b0,b:b4,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b1,y:b5,",
"03000000bd12000013d0000000000000,Retrolink Sega Saturn Classic Controller,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b5,lefttrigger:b6,rightshoulder:b2,righttrigger:b7,start:b8,x:b3,y:b4,",
"03000000bd12000015d0000000000000,Retrolink SNES Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b9,x:b3,y:b0,",
"03000000341200000400000000000000,RetroUSB N64 RetroPort,+rightx:b8,+righty:b10,-rightx:b9,-righty:b11,a:b7,b:b6,dpdown:b2,dpleft:b1,dpright:b0,dpup:b3,leftshoulder:b13,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b12,start:b4,",
"0300000000f000000300000000000000,RetroUSB RetroPad,a:b1,b:b5,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b0,y:b4,",
"0300000000f00000f100000000000000,RetroUSB Super RetroPort,a:b1,b:b5,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b0,y:b4,",
"03000000830500000960000000000000,Revenger,a:b0,b:b1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b3,x:b4,y:b5,",
"030000006b140000010d000000000000,Revolution Pro Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000006b140000020d000000000000,Revolution Pro Controller 2,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000006b140000130d000000000000,Revolution Pro Controller 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000006f0e00001f01000000000000,Rock Candy,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00004601000000000000,Rock Candy,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c6240000fefa000000000000,Rock Candy Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00008701000000000000,Rock Candy Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00001e01000000000000,Rock Candy PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00002801000000000000,Rock Candy PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00002f01000000000000,Rock Candy PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000830500007030000000000000,Rockfire Space Ranger,a:b0,b:b1,back:b5,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b7,rightshoulder:b9,righttrigger:b8,start:b2,x:b3,y:b4,",
"03000000050b0000e318000000000000,ROG Chakram,a:b1,b:b0,leftx:a0,lefty:a1,x:b2,y:b3,",
"03000000050b0000e518000000000000,ROG Chakram,a:b1,b:b0,leftx:a0,lefty:a1,x:b2,y:b3,",
"03000000050b00005819000000000000,ROG Chakram Core,a:b1,b:b0,leftx:a0,lefty:a1,x:b2,y:b3,",
"03000000050b0000181a000000000000,ROG Chakram X,a:b1,b:b0,leftx:a0,lefty:a1,x:b2,y:b3,",
"03000000050b00001a1a000000000000,ROG Chakram X,a:b1,b:b0,leftx:a0,lefty:a1,x:b2,y:b3,",
"03000000050b00001c1a000000000000,ROG Chakram X,a:b1,b:b0,leftx:a0,lefty:a1,x:b2,y:b3,",
"030000004f04000001d0000000000000,Rumble Force,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"030000000d0f0000ad00000000000000,RX Gamepad,a:b0,b:b4,back:b11,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b3,rightshoulder:b6,start:b9,x:b2,y:b1,",
"030000008916000000fe000000000000,Sabertooth,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c6240000045d000000000000,Sabertooth,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00001311000000000000,Saffun Controller,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b0,",
"03000000a30600001af5000000000000,Saitek Cyborg,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a4,start:b9,x:b0,y:b3,",
"03000000a306000023f6000000000000,Saitek Cyborg,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a4,start:b9,x:b0,y:b3,",
"03000000300f00001201000000000000,Saitek Dual Analog,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b1,",
"03000000a30600000701000000000000,Saitek P220,a:b2,b:b3,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b7,rightshoulder:b4,righttrigger:b5,x:b0,y:b1,",
"03000000a30600000cff000000000000,Saitek P2500 Force Rumble,a:b2,b:b3,back:b11,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3,start:b10,x:b0,y:b1,",
"03000000a30600000d5f000000000000,Saitek P2600,a:b1,b:b2,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a3,righty:a2,start:b8,x:b0,y:b3,",
"03000000a30600000dff000000000000,Saitek P2600,a:b1,b:b2,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a5,righty:a2,start:b8,x:b0,y:b3,",
"03000000a30600000c04000000000000,Saitek P2900,a:b1,b:b2,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b3,",
"03000000a306000018f5000000000000,Saitek P3200,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a4,start:b9,x:b0,y:b3,",
"03000000300f00001001000000000000,Saitek P480 Rumble,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b1,",
"03000000a30600000901000000000000,Saitek P880,a:b2,b:b3,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b8,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b9,righttrigger:b5,rightx:a3,righty:a2,x:b0,y:b1,",
"03000000a30600000b04000000000000,Saitek P990,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b3,",
"03000000a30600002106000000000000,Saitek PS1000 PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a4,start:b9,x:b0,y:b3,",
"03000000a306000020f6000000000000,Saitek PS2700 PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a4,start:b9,x:b0,y:b3,",
"03000000300f00001101000000000000,Saitek Rumble,a:b2,b:b3,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b1,",
"03000000e804000000a0000000000000,Samsung EIGP20,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000c01100000252000000000000,Sanwa Easy Grip,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b9,x:b3,y:b4,",
"03000000c01100004350000000000000,Sanwa Micro Grip P3,a:b1,b:b0,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,x:b3,y:b2,",
"03000000411200004550000000000000,Sanwa Micro Grip Pro,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a1,righty:a2,start:b9,x:b1,y:b3,",
"03000000c01100004150000000000000,Sanwa Micro Grip Pro,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"03000000c01100004450000000000000,Sanwa Online Grip,a:b0,b:b1,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b8,rightstick:b11,righttrigger:b9,rightx:a3,righty:a2,start:b14,x:b3,y:b4,",
"03000000730700000401000000000000,Sanwa PlayOnline Mobile,a:b0,b:b1,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,start:b3,",
"03000000830500006120000000000000,Sanwa Smart Grip II,a:b0,b:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,x:b1,y:b3,",
"03000000c01100000051000000000000,Satechi Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"030000004f04000028b3000000000000,Score A,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000952e00002577000000000000,Scuf PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000a30c00002500000000000000,Sega Genesis Mini 3B Controller,a:b2,b:b1,dpdown:+a4,dpleft:-a3,dpright:+a3,dpup:-a4,righttrigger:b5,start:b9,",
"03000000a30c00002400000000000000,Sega Mega Drive Mini 6B Controller,a:b2,b:b1,dpdown:+a4,dpleft:-a3,dpright:+a3,dpup:-a4,rightshoulder:b4,righttrigger:b5,start:b9,x:b3,y:b0,",
"03000000d804000086e6000000000000,Sega Multi Controller,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:a2,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b8,x:b3,y:b4,",
"0300000000050000289b000000000000,Sega Saturn Adapter,a:b1,b:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b4,rightshoulder:b7,righttrigger:b5,start:b9,x:b0,y:b3,",
"0300000000f000000800000000000000,Sega Saturn Controller,a:b1,b:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,rightshoulder:b7,righttrigger:b3,start:b0,x:b5,y:b6,",
"03000000730700000601000000000000,Sega Saturn Controller,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b9,x:b3,y:b4,",
"03000000b40400000a01000000000000,Sega Saturn Controller,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b7,rightshoulder:b5,righttrigger:b2,start:b8,x:b3,y:b4,",
"030000003b07000004a1000000000000,SFX,a:b0,b:b2,back:b7,leftshoulder:b6,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b9,righttrigger:b5,start:b8,x:b1,y:b3,",
"03000000f82100001900000000000000,Shogun Bros Chameleon X1,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"03000000120c00001c1e000000000000,SnakeByte 4S PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000140300000918000000000000,SNES Controller,a:b0,b:b1,back:b6,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b7,x:b2,y:b3,",
"0300000081170000960a000000000000,SNES Controller,a:b4,b:b0,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b5,y:b1,",
"03000000811700009d0a000000000000,SNES Controller,a:b0,b:b4,back:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,start:b3,x:b1,y:b5,",
"030000008b2800000300000000000000,SNES Controller,a:b0,b:b4,back:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,start:b3,x:b1,y:b5,",
"03000000921200004653000000000000,SNES Controller,a:b0,b:b4,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b1,y:b5,",
"030000008f0e00000910000000000000,Sony DualShock 2,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b9,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b10,righttrigger:b5,rightx:a3,righty:a2,start:b11,x:b3,y:b0,",
"03000000317300000100000000000000,Sony DualShock 3,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"03000000666600006706000000000000,Sony PlayStation Adapter,a:b2,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b6,leftstick:b9,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b10,righttrigger:b5,rightx:a2,righty:a3,start:b11,x:b3,y:b0,",
"03000000e30500009605000000000000,Sony PlayStation Adapter,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"03000000fe1400002a23000000000000,Sony PlayStation Adapter,a:b0,b:b1,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,x:b2,y:b3,",
"030000004c050000da0c000000000000,Sony PlayStation Classic Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b4,rightshoulder:b7,righttrigger:b5,start:b9,x:b3,y:b0,",
"03000000632500002306000000000000,Sony PlayStation Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000f0250000c183000000000000,Sony PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d9040000160f000000000000,Sony PlayStation Controller Adapter,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"03000000ff000000cb01000000000000,Sony PlayStation Portable,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,start:b7,x:b2,y:b3,",
"030000004c0500003713000000000000,Sony PlayStation Vita,a:b1,b:b2,back:b8,dpdown:b13,dpleft:b15,dpright:b14,dpup:b12,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,rightx:a3,righty:a4,start:b9,x:b0,y:b3,",
"03000000341a00000208000000000000,Speedlink 6555,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:-a4,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a4,rightx:a3,righty:a2,start:b7,x:b2,y:b3,",
"03000000341a00000908000000000000,Speedlink 6566,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000380700001722000000000000,Speedlink Competition Pro,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,x:b2,y:b3,",
"030000008f0e00000800000000000000,Speedlink Strike FX,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000c01100000591000000000000,Speedlink Torid,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000de280000fc11000000000000,Steam Virtual Gamepad,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000de280000ff11000000000000,Steam Virtual Gamepad,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:+a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:-a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000120c0000160e000000000000,Steel Play Metaltech PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000110100001914000000000000,SteelSeries,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftstick:b13,lefttrigger:b6,leftx:a0,lefty:a1,rightstick:b14,righttrigger:b7,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000381000001214000000000000,SteelSeries Free,a:b0,b:b1,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000110100003114000000000000,SteelSeries Stratus Duo,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000381000003014000000000000,SteelSeries Stratus Duo,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000381000003114000000000000,SteelSeries Stratus Duo,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000381000001814000000000000,SteelSeries Stratus XL,a:b0,b:b1,back:b18,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,guide:b19,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b2,y:b3,",
"03000000380700003847000000000000,Street Fighter Fightstick TE,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b11,start:b7,x:b2,y:b3,",
"030000001f08000001e4000000000000,Super Famicom Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b9,x:b3,y:b0,",
"03000000790000000418000000000000,Super Famicom Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b33,rightshoulder:b5,start:b7,x:b2,y:b3,",
"03000000341200001300000000000000,Super Racer,a:b2,b:b3,back:b8,leftshoulder:b5,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b4,righttrigger:b7,x:b0,y:b1,",
"03000000457500002211000000000000,Szmy Power PC Gamepad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000004f0400000ab1000000000000,T16000M,a:b0,b:b1,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b11,leftshoulder:b4,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,start:b10,x:b2,y:b3,",
"030000000d0f00007b00000000000000,TAC GEAR,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000e40a00000307000000000000,Taito Egret II Mini Control Panel,a:b4,b:b2,back:b6,guide:b9,leftx:a0,lefty:a1,rightshoulder:b0,righttrigger:b1,start:b7,x:b8,y:b3,",
"03000000e40a00000207000000000000,Taito Egret II Mini Controller,a:b4,b:b2,back:b6,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,guide:b9,rightshoulder:b0,righttrigger:b1,start:b7,x:b8,y:b3,",
"03000000d814000001a0000000000000,TE Kitty,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000fa1900000706000000000000,Team 5,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000b50700001203000000000000,Techmobility X6-38V,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b1,",
"03000000ba2200000701000000000000,Technology Innovation PS2 Adapter,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b3,y:b2,",
"03000000c61100001000000000000000,Tencent Xianyou Gamepad,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,x:b3,y:b4,",
"03000000790000001c18000000000000,TGZ Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000790000002601000000000000,TGZ Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b3,y:b0,",
"03000000591c00002400000000000000,THEC64 Joystick,a:b0,b:b1,back:b6,leftshoulder:b4,leftx:a0,lefty:a4,rightshoulder:b5,start:b7,x:b2,y:b3,",
"03000000591c00002600000000000000,THEGamepad,a:b2,b:b1,back:b6,leftx:a0,lefty:a1,start:b7,x:b3,y:b0,",
"030000004f04000015b3000000000000,Thrustmaster Dual Analog 4,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"030000004f04000023b3000000000000,Thrustmaster Dual Trigger PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000004f0400000ed0000000000000,Thrustmaster eSwap Pro Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000004f04000008d0000000000000,Thrustmaster Ferrari 150 PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000004f04000000b3000000000000,Thrustmaster Firestorm Dual Power,a:b0,b:b2,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b11,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b12,righttrigger:b7,rightx:a2,righty:a3,start:b10,x:b1,y:b3,",
"030000004f04000004b3000000000000,Thrustmaster Firestorm Dual Power,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"030000004f04000003d0000000000000,Thrustmaster Run N Drive PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b7,leftshoulder:a3,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:a4,rightstick:b11,righttrigger:b5,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000004f04000009d0000000000000,Thrustmaster Run N Drive PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000088ca000000000000,Thunderpad,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"03000000666600000288000000000000,TigerGame PlayStation Adapter,a:b2,b:b1,back:b9,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"03000000666600000488000000000000,TigerGame PlayStation Adapter,a:b2,b:b1,back:b9,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"030000004f04000007d0000000000000,TMini,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000571d00002100000000000000,Tomee NES Controller Adapter,a:b1,b:b0,back:b2,dpdown:+a4,dpleft:-a0,dpright:+a0,dpup:-a4,start:b3,",
"03000000571d00002000000000000000,Tomee SNES Controller Adapter,a:b0,b:b1,back:b6,dpdown:+a4,dpleft:-a0,dpright:+a0,dpup:-a4,leftshoulder:b4,rightshoulder:b5,start:b7,x:b2,y:b3,",
"03000000d62000006000000000000000,Tournament PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000c01100000055000000000000,Tronsmart,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000005f140000c501000000000000,Trust Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000b80500000210000000000000,Trust Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000004f04000087b6000000000000,TWCS Throttle,dpdown:b8,dpleft:b9,dpright:b7,dpup:b6,leftstick:b5,lefttrigger:-a5,leftx:a0,lefty:a1,righttrigger:+a5,",
"03000000411200000450000000000000,Twin Shock,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b9,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b10,righttrigger:b5,rightx:a3,righty:a4,start:b11,x:b3,y:b0,",
"03000000d90400000200000000000000,TwinShock PS2 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"03000000151900005678000000000000,Uniplay U6,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000101c0000171c000000000000,uRage Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000000b0400003065000000000000,USB Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b3,y:b0,",
"03000000242f00006e00000000000000,USB Controller,a:b1,b:b4,back:b10,leftshoulder:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b9,righttrigger:b7,rightx:a2,righty:a5,start:b11,x:b0,y:b3,",
"03000000300f00000701000000000000,USB Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"03000000341a00002308000000000000,USB Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000666600000188000000000000,USB Controller,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"030000006b1400000203000000000000,USB Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000790000000a00000000000000,USB Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a4,start:b9,x:b3,y:b0,",
"03000000b404000081c6000000000000,USB Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b3,y:b0,",
"03000000b50700001503000000000000,USB Controller,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a5,righty:a2,start:b9,x:b0,y:b1,",
"03000000bd12000012d0000000000000,USB Controller,a:b0,b:b1,back:b6,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,start:b7,x:b2,y:b3,",
"03000000ff1100004133000000000000,USB Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a4,righty:a2,start:b9,x:b3,y:b0,",
"03000000632500002305000000000000,USB Vibration Joystick,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000882800000305000000000000,V5 Game Pad,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,x:b2,y:b3,",
"03000000790000001a18000000000000,Venom PS4 Arcade Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000790000001b18000000000000,Venom PS4 Arcade Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000006f0e00000302000000000000,Victrix PS4 Pro Fightstick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,touchpad:b13,x:b0,y:b3,",
"030000006f0e00000702000000000000,Victrix PS4 Pro Fightstick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,touchpad:b13,x:b0,y:b3,",
"0300000034120000adbe000000000000,vJoy Device,a:b0,b:b1,back:b15,dpdown:b6,dpleft:b7,dpright:b8,dpup:b5,guide:b16,leftshoulder:b9,leftstick:b13,lefttrigger:b11,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b14,righttrigger:b12,rightx:a3,righty:a4,start:b4,x:b2,y:b3,",
"03000000120c0000ab57000000000000,Warrior Joypad JS083,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000007e0500003003000000000000,Wii U Pro,a:b0,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b16,dpup:b13,leftshoulder:b6,leftstick:b11,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b12,righttrigger:b5,rightx:a2,righty:a3,start:b9,x:b3,y:b2,",
"0300000032150000030a000000000000,Wildcat,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"0300000032150000140a000000000000,Wolverine,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000002e160000efbe000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b10,rightshoulder:b5,righttrigger:b11,start:b7,x:b2,y:b3,",
"03000000380700001647000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000380700002045000000000000,Xbox 360 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000380700002644000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b7,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a2,righty:a5,start:b8,x:b2,y:b3,",
"03000000380700002647000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000003807000026b7000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000380700003647000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a7,righty:a5,start:b7,x:b2,y:b3,",
"030000005e0400001907000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400008e02000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:+a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:-a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400009102000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000ad1b000000fd000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000ad1b000001fd000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000ad1b000016f0000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000ad1b00008e02000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c62400000053000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c6240000fdfa000000000000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000380700002847000000000000,Xbox 360 Fightpad,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b11,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000a102000000000000,Xbox 360 Wireless Receiver,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:+a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:-a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400000a0b000000000000,Xbox Adaptive Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:+a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:-a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000120c00000a88000000000000,Xbox Controller,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a2,righty:a4,start:b6,x:b2,y:b3,",
"03000000120c00001088000000000000,Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2~,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5~,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000002a0600002000000000000000,Xbox Controller,a:b0,b:b1,back:b13,dpdown:b9,dpleft:b10,dpright:b11,dpup:b8,leftshoulder:b5,leftstick:b14,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b15,righttrigger:b7,rightx:a2,righty:a5,start:b12,x:b2,y:b3,",
"03000000380700001645000000000000,Xbox Controller,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"03000000380700002645000000000000,Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000380700003645000000000000,Xbox Controller,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"03000000380700008645000000000000,Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400000202000000000000,Xbox Controller,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b9,righttrigger:b11,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"030000005e0400008502000000000000,Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400008702000000000000,Xbox Controller,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b8,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b9,righttrigger:b7,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"030000005e0400008902000000000000,Xbox Controller,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b10,leftstick:b8,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b9,righttrigger:b4,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"030000005e0400000c0b000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000d102000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000dd02000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000e002000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000e302000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000ea02000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:+a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:-a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000fd02000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000ff02000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:+a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:-a2,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e0000a802000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e0000c802000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c62400003a54000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000130b000000000000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000341a00000608000000000000,Xeox,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000450c00002043000000000000,Xeox SL6556BK,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000006f0e00000300000000000000,XGear,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a5,righty:a2,start:b9,x:b3,y:b0,",
"03000000e0ff00000201000000000000,Xiaomi Black Shark (L),back:b0,dpdown:b11,dpleft:b9,dpright:b10,dpup:b8,leftshoulder:b5,lefttrigger:b7,leftx:a0,lefty:a1,",
"03000000172700004431000000000000,Xiaomi Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b20,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a7,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"03000000172700003350000000000000,Xiaomi XMGP01YM,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000bc2000005060000000000000,Xiaomi XMGP01YM,+lefty:+a2,+righty:+a5,-lefty:-a1,-righty:-a4,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,start:b11,x:b3,y:b4,",
"xinput,XInput Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000007d0400000340000000000000,Xterminator Digital Gamepad,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:-a4,lefttrigger:+a4,leftx:a0,lefty:a1,paddle1:b7,paddle2:b6,rightshoulder:b5,rightstick:b9,righttrigger:b2,rightx:a3,righty:a5,start:b8,x:b3,y:b4,",
"030000002c3600000100000000000000,Yawman Arrow,+rightx:h0.2,+righty:h0.4,-rightx:h0.8,-righty:h0.1,a:b4,b:b5,back:b6,dpdown:b15,dpleft:b14,dpright:b16,dpup:b13,leftshoulder:b10,leftstick:b0,lefttrigger:-a4,leftx:a0,lefty:a1,paddle1:b11,paddle2:b12,rightshoulder:b8,rightstick:b9,righttrigger:+a4,start:b3,x:b1,y:b2,",
"03000000790000004f18000000000000,ZDT Android Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"03000000120c00000500000000000000,Zeroplus Adapter,a:b2,b:b1,back:b11,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b9,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b0,righttrigger:b5,rightx:a3,righty:a2,start:b8,x:b3,y:b0,",
"03000000120c0000101e000000000000,Zeroplus P4 Gamepad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,"
#elif defined(__APPLE__)
"030000008f0e00000300000009010000,2 In 1 Joystick,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000c82d00000031000001000000,8BitDo Adapter,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000531000000020000,8BitDo Adapter 2,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000951000000010000,8BitDo Dogbone,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,rightx:a2,righty:a3,start:b11,",
"03000000c82d00000090000001000000,8BitDo FC30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a5,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00001038000000010000,8BitDo FC30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00006a28000000010000,8BitDo GameCube,a:b0,b:b3,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b9,paddle2:b8,rightshoulder:b10,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b1,y:b4,",
"03000000c82d00001251000000010000,8BitDo Lite 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00001251000000020000,8BitDo Lite 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00001151000000010000,8BitDo Lite SE,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00001151000000020000,8BitDo Lite SE,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000a30c00002400000006020000,8BitDo M30,a:b2,b:b1,dpdown:+a4,dpleft:-a3,dpright:+a3,dpup:-a4,guide:b9,leftshoulder:b6,lefttrigger:b5,rightshoulder:b4,righttrigger:b7,start:b8,x:b3,y:b0,",
"03000000c82d00000151000000010000,8BitDo M30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000650000001000000,8BitDo M30,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b8,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00005106000000010000,8BitDo M30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,guide:b2,leftshoulder:b6,lefttrigger:a5,rightshoulder:b7,righttrigger:a4,start:b11,x:b4,y:b3,",
"03000000c82d00002090000000010000,8BitDo Micro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000451000000010000,8BitDo N30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,rightx:a2,righty:a3,start:b11,",
"03000000c82d00001590000001000000,8BitDo N30 Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a5,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00006528000000010000,8BitDo N30 Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00006928000000010000,8BitDo N64,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a3,start:b11,",
"03000000c82d00002590000000010000,8BitDo NEOGEO,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00002590000001000000,8BitDo NEOGEO,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00002690000000010000,8BitDo NEOGEO,+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b10,leftshoulder:b6,rightshoulder:b7,start:b11,x:b3,y:b4,",
"030000003512000012ab000001000000,8BitDo NES30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d000012ab000001000000,8BitDo NES30,a:b0,b:b1,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b3,y:b4,",
"03000000c82d00002028000000010000,8BitDo NES30,a:b0,b:b1,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b3,y:b4,",
"03000000022000000090000001000000,8BitDo NES30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000203800000900000000010000,8BitDo NES30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000190000001000000,8BitDo NES30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000751000000010000,8BitDo P30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000851000000010000,8BitDo P30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000660000000010000,8BitDo Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000660000000020000,8BitDo Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000131000001000000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000231000001000000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000331000001000000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000431000001000000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00002867000000010000,8BitDo S30,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b8,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,rightx:a2,righty:a3,start:b10,x:b3,y:b4,",
"03000000c82d00003028000000010000,8Bitdo SFC30 Gamepad,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000102800000900000000000000,8BitDo SFC30 Joystick,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d00000351000000010000,8BitDo SN30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00001290000001000000,8BitDo SN30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d00004028000000010000,8BitDo SN30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d00000160000001000000,8BitDo SN30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a5,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000161000000010000,8BitDo SN30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a5,start:b11,x:b4,y:b3,",
"03000000c82d00000260000001000000,8BitDo SN30 Pro Plus,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000261000000010000,8BitDo SN30 Pro Plus,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00001230000000010000,8BitDo Ultimate,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b2,paddle2:b5,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001b30000001000000,8BitDo Ultimate 2C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b5,paddle2:b2,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001d30000001000000,8BitDo Ultimate 2C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b5,paddle2:b2,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001530000001000000,8BitDo Ultimate C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001630000001000000,8BitDo Ultimate C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001730000001000000,8BitDo Ultimate C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001130000000020000,8BitDo Ultimate Wired,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b26,paddle1:b24,paddle2:b25,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001330000001000000,8BitDo Ultimate Wireless,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b26,paddle1:b23,paddle2:b19,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001330000000020000,8BitDo Ultimate Wireless Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b26,paddle1:b23,paddle2:b19,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000a00500003232000008010000,8BitDo Zero,a:b0,b:b1,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b3,y:b4,",
"03000000a00500003232000009010000,8BitDo Zero,a:b0,b:b1,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b3,y:b4,",
"03000000c82d00001890000001000000,8BitDo Zero 2,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d00003032000000010000,8BitDo Zero 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a2,righty:a31,start:b11,x:b4,y:b3,",
"03000000491900001904000001010000,Amazon Luna Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b9,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b7,x:b2,y:b3,",
"03000000710100001904000000010000,Amazon Luna Controller,a:b0,b:b1,back:b11,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b9,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"0300000008100000e501000019040000,Anbernic Handheld,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a4,start:b11,x:b4,y:b3,",
"03000000373500004610000001000000,Anbernic RG P01,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000a30c00002700000003030000,Astro City Mini,a:b2,b:b1,back:b8,dpdown:+a4,dpleft:-a3,dpright:+a3,dpup:-a4,rightshoulder:b4,righttrigger:b5,start:b9,x:b3,y:b0,",
"03000000a30c00002800000003030000,Astro City Mini,a:b2,b:b1,back:b8,leftx:a3,lefty:a4,rightshoulder:b4,righttrigger:b5,start:b9,x:b3,y:b0,",
"03000000050b00000045000031000000,ASUS Gamepad,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000050b00000579000000010000,ASUS ROG Kunai 3,a:b0,b:b1,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b14,leftshoulder:b6,leftstick:b15,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b42,paddle1:b9,paddle2:b11,rightshoulder:b7,rightstick:b16,righttrigger:a4,rightx:a2,righty:a3,start:b13,x:b3,y:b4,",
"03000000050b00000679000000010000,ASUS ROG Kunai 3,a:b0,b:b1,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b14,leftshoulder:b6,leftstick:b15,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b23,rightshoulder:b7,rightstick:b16,righttrigger:a4,rightx:a2,righty:a3,start:b13,x:b3,y:b4,",
"03000000503200000110000045010000,Atari VCS Classic,a:b0,b:b1,back:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b3,start:b2,",
"03000000503200000110000047010000,Atari VCS Classic Controller,a:b0,b:b1,back:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b3,start:b2,",
"03000000503200000210000047010000,Atari VCS Modern Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a4,rightx:a2,righty:a3,start:b8,x:b2,y:b3,",
"030000008a3500000102000000010000,Backbone One,a:b0,b:b1,back:b16,dpdown:b11,dpleft:b13,dpright:b12,dpup:b10,guide:b17,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1~,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3~,start:b15,x:b2,y:b3,",
"030000008a3500000201000000010000,Backbone One,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000008a3500000202000000010000,Backbone One,a:b0,b:b1,back:b16,dpdown:b11,dpleft:b13,dpright:b12,dpup:b10,guide:b17,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1~,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3~,start:b15,x:b2,y:b3,",
"030000008a3500000402000000010000,Backbone One,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000008a3500000302000000010000,Backbone One PlayStation Edition,a:b0,b:b1,back:b16,dpdown:b11,dpleft:b13,dpright:b12,dpup:b10,guide:b17,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1~,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3~,start:b15,x:b2,y:b3,",
"03000000c62400001a89000000010000,BDA MOGA XP5-X Plus,a:b0,b:b1,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b14,leftshoulder:b6,leftstick:b15,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b16,righttrigger:a4,rightx:a2,righty:a3,start:b13,x:b3,y:b4,",
"03000000c62400001b89000000010000,BDA MOGA XP5-X Plus,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000d62000002a79000000010000,BDA PS4 Fightpad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000200e000000010000,Brook Mars PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000210e000000010000,Brook Mars PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,touchpad:b13,x:b0,y:b3,",
"030000008305000031b0000000000000,Cideko AK08b,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d8140000cecf000000000000,Cthulhu,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000260900008888000088020000,Cyber Gadget GameCube Controller,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:a5,rightx:a2,righty:a3~,start:b7,x:b2,y:b3,",
"03000000a306000022f6000001030000,Cyborg V3 Rumble Pad PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:+a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:-a3,rightx:a2,righty:a4,start:b9,x:b0,y:b3,",
"03000000791d00000103000009010000,Dual Box Wii Classic Adapter,a:b2,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,guide:b10,leftshoulder:b6,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b5,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000006e0500000720000010020000,Elecom JC-W01U,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b1,",
"030000006f0e00008401000003010000,Faceoff Deluxe Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b13,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000151900004000000001000000,Flydigi Vader 2,a:b14,b:b15,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b21,leftshoulder:b6,leftstick:b12,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b2,paddle2:b5,paddle3:b16,paddle4:b17,rightshoulder:b7,rightstick:b13,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b0,y:b1,",
"03000000b40400001124000001040000,Flydigi Vader 2,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b12,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b14,paddle1:b2,paddle2:b5,paddle3:b16,paddle4:b17,rightshoulder:b7,rightstick:b13,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000b40400001224000003030000,Flydigi Vader 2,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b12,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b2,paddle1:b16,paddle2:b17,paddle3:b14,paddle4:b15,rightshoulder:b7,rightstick:b13,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000790000004618000000010000,GameCube Controller Adapter,a:b4,b:b0,dpdown:b56,dpleft:b60,dpright:b52,dpup:b48,lefttrigger:a12,leftx:a0,lefty:a4,rightshoulder:b28,righttrigger:a16,rightx:a20,righty:a8,start:b36,x:b8,y:b12,",
"03000000ac0500001a06000002020000,GameSir-T3 2.02,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000ad1b000001f9000000000000,Gamestop BB070 X360 Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"0500000047532047616d657061640000,GameStop Gamepad,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000c01100000140000000010000,GameStop PS4 Fun Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000006f0e00000102000000000000,GameStop Xbox 360 Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"03000000ff1100003133000007010000,GameWare PC Control Pad,a:b2,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a3,righty:a4,start:b11,x:b3,y:b0,",
"03000000d11800000094000000010000,Google Stadia Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a4,rightx:a2,righty:a3,start:b7,x:b2,y:b3,",
"030000007d0400000540000001010000,Gravis Eliminator Pro,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000280400000140000000020000,Gravis GamePad Pro,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000008f0e00000300000007010000,GreenAsia Joystick,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b1,",
"030000000d0f00002d00000000100000,Hori Fighting Commander 3 Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00005f00000000000000,Hori Fighting Commander 4 PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00005f00000000010000,Hori Fighting Commander 4 PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00005e00000000000000,Hori Fighting Commander 4 PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f00005e00000000010000,Hori Fighting Commander 4 PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f00008400000000010000,Hori Fighting Commander PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00008500000000010000,Hori Fighting Commander PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000341a00000302000014010000,Hori Fighting Stick Mini,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f00008800000000010000,Hori Fighting Stick mini 4 PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,rightshoulder:b5,rightstick:b11,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f00008700000000010000,Hori Fighting Stick mini 4 PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,rightshoulder:b5,rightstick:b11,righttrigger:b7,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f00004d00000000000000,Hori Gem Pad 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00003801000008010000,Hori PC Engine Mini Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,start:b9,",
"030000000d0f00009200000000010000,Hori Pokken Tournament DX Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f0000aa00000072050000,Hori Real Arcade Pro for Nintendo Switch,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000000d0f00000002000017010000,Hori Split Pad Fit,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00000002000015010000,Hori Switch Split Pad Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00006e00000000010000,Horipad 4 PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00006600000000010000,Horipad 4 PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f00006600000000000000,Horipad FPS Plus 4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f0000ee00000000010000,Horipad Mini 4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f0000c100000072050000,Horipad Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000242e0000ff0b000000010000,Hyperkin N64 Adapter,a:b1,b:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightx:a2,righty:a3,start:b9,",
"03000000790000004e95000000010000,Hyperkin N64 Controller Adapter,a:b1,b:b2,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b7,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightx:a5,righty:a2,start:b9,",
"03000000830500006020000000000000,iBuffalo Super Famicom Controller,a:b1,b:b0,back:b6,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b7,x:b3,y:b2,",
"03000000ef0500000300000000020000,InterAct AxisPad,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a3,righty:a2,start:b11,x:b0,y:b1,",
"03000000fd0500000030000010010000,Interact GoPad,a:b3,b:b4,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,x:b0,y:b1,",
"030000007e0500000620000001000000,Joy-Con (L),+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b13,leftshoulder:b4,leftstick:b10,rightshoulder:b5,start:b8,x:b2,y:b3,",
"030000007e0500000720000001000000,Joy-Con (R),+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b12,leftshoulder:b4,leftstick:b11,rightshoulder:b5,start:b9,x:b2,y:b3,",
"03000000242f00002d00000007010000,JYS Adapter,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000006d04000019c2000000000000,Logitech Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000016c2000000020000,Logitech Dual Action,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000016c2000000030000,Logitech Dual Action,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000016c2000014040000,Logitech Dual Action,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000016c2000000000000,Logitech F310,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000018c2000000000000,Logitech F510,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000019c2000005030000,Logitech F710,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d0400001fc2000000000000,Logitech F710,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"030000006d04000018c2000000010000,Logitech RumblePad 2,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1~,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3~,start:b9,x:b0,y:b3,",
"03000000380700005032000000010000,Mad Catz PS3 Fightpad Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700008433000000010000,Mad Catz PS3 Fightstick TE S Plus,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700005082000000010000,Mad Catz PS4 Fightpad Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000380700008483000000010000,Mad Catz PS4 Fightstick TE S Plus,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"0300000049190000020400001b010000,Manba One,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b22,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000790000000600000007010000,Marvo GT-004,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000008f0e00001330000011010000,Mayflash Controller Adapter,a:b2,b:b4,back:b16,dpdown:h0.8,dpleft:h0.2,dpright:h0.1,dpup:h0.4,leftshoulder:b12,lefttrigger:b16,leftx:a0,lefty:a2,rightshoulder:b14,rightx:a6~,righty:a4,start:b18,x:b0,y:b6,",
"03000000790000004318000000010000,Mayflash GameCube Adapter,a:b4,b:b0,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,lefttrigger:a12,leftx:a0,lefty:a4,rightshoulder:b28,righttrigger:a16,rightx:a20,righty:a8,start:b36,x:b8,y:b12,",
"03000000790000004418000000010000,Mayflash GameCube Controller,a:b1,b:b2,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b0,y:b3,",
"03000000242f00007300000000020000,Mayflash Magic NS,a:b1,b:b4,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b0,y:b3,",
"0300000079000000d218000026010000,Mayflash Magic NS,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000d620000010a7000003010000,Mayflash Magic NS,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000008f0e00001030000011010000,Mayflash Saturn Adapter,a:b0,b:b2,dpdown:b28,dpleft:b30,dpright:b26,dpup:b24,leftshoulder:b10,lefttrigger:b14,rightshoulder:b12,righttrigger:b4,start:b18,x:b6,y:b8,",
"0300000025090000e803000000000000,Mayflash Wii Classic Adapter,a:b1,b:b0,back:b8,dpdown:b13,dpleft:b12,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b2,",
"03000000790000000318000000010000,Mayflash Wii DolphinBar,a:b8,b:b12,back:b32,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b44,leftshoulder:b16,lefttrigger:b24,leftx:a0,lefty:a4,rightshoulder:b20,righttrigger:b28,rightx:a8,righty:a12,start:b36,x:b0,y:b4,",
"03000000790000000018000000000000,Mayflash Wii U Pro Adapter,a:b4,b:b8,back:b32,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b16,leftstick:b40,lefttrigger:b24,leftx:a0,lefty:a4,rightshoulder:b20,rightstick:b44,righttrigger:b28,rightx:a8,righty:a12,start:b36,x:b0,y:b12,",
"03000000790000000018000000010000,Mayflash Wii U Pro Adapter,a:b4,b:b8,back:b32,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b16,leftstick:b40,lefttrigger:b24,leftx:a0,lefty:a4,rightshoulder:b20,rightstick:b44,righttrigger:b28,rightx:a8,righty:a12,start:b36,x:b0,y:b12,",
"030000005e0400002800000002010000,Microsoft Dual Strike,a:b3,b:b2,back:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b8,rightshoulder:b7,rightx:a0,righty:a1~,start:b5,x:b1,y:b0,",
"030000005e0400000300000006010000,Microsoft SideWinder,a:b0,b:b1,back:b9,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b8,x:b3,y:b4,",
"030000005e0400000700000006010000,Microsoft SideWinder,a:b0,b:b1,back:b8,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b9,x:b3,y:b4,",
"030000005e0400002700000001010000,Microsoft SideWinder Plug and Play,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,lefttrigger:b4,righttrigger:b5,x:b2,y:b3,",
"030000004523000015e0000072050000,Mobapad Chitu HD,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000d62000007162000001000000,Moga Pro 2,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"03000000c62400002a89000000010000,MOGA XP5A Plus,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b21,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c62400002b89000000010000,MOGA XP5A Plus,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000632500007505000000020000,NeoGeo mini PAD Controller,a:b1,b:b0,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,start:b9,x:b2,y:b3,",
"03000000921200004b46000003020000,NES 2-port Adapter,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,start:b11,",
"030000001008000001e5000006010000,NEXT SNES Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,righttrigger:b6,start:b9,x:b3,y:b0,",
"030000007e0500000920000000000000,Nintendo Switch Pro Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000007e0500000920000001000000,Nintendo Switch Pro Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000007e0500000920000010020000,Nintendo Switch Pro Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b3,",
"050000007e05000009200000ff070000,Nintendo Switch Pro Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b3,",
"030000007e0500001920000001000000,NSO N64 Controller,+rightx:b8,+righty:b7,-rightx:b3,-righty:b2,a:b1,b:b0,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,righttrigger:b10,start:b9,",
"030000007e0500001720000001000000,NSO SNES Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b15,start:b9,x:b2,y:b3,",
"03000000550900001472000025050000,NVIDIA Controller,a:b0,b:b1,back:b17,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b4,leftstick:b7,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a5,start:b6,x:b2,y:b3,",
"030000004b120000014d000000010000,Nyko Airflo EX,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b2,y:b3,",
"0300000009120000072f000000010000,OrangeFox86 DreamPicoPort,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,lefttrigger:a2,leftx:a0,lefty:a1,righttrigger:a5,rightx:a3,righty:a4,start:b11,x:b3,y:b4,",
"030000006f0e00000901000002010000,PDP PS3 Versus Fighting,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000008f0e00000300000000000000,Piranha Xtreme PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"03000000d620000011a7000000020000,PowerA Core Plus Gamecube Controller,a:b1,b:b0,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000d620000011a7000010050000,PowerA Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d62000006dca000000010000,PowerA Pro Ex,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000100800000300000006010000,PS2 Adapter,a:b2,b:b1,back:b8,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a4,righty:a3,start:b9,x:b3,y:b0,",
"030000004c0500006802000000000000,PS3 Controller,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:b9,rightx:a2,righty:a3,start:b3,x:b15,y:b12,",
"030000004c0500006802000000010000,PS3 Controller,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:b9,rightx:a2,righty:a3,start:b3,x:b15,y:b12,",
"030000004c0500006802000072050000,PS3 Controller,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:b9,rightx:a2,righty:a3,start:b3,x:b15,y:b12,",
"030000004c050000a00b000000010000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000c405000000000000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000c405000000010000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000cc09000000010000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"0300004b4c0500005f0e000000010000,PS5 Access Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b14,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000e60c000000010000,PS5 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b14,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000f20d000000010000,PS5 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b14,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"050000004c050000e60c000000010000,PS5 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"050000004c050000f20d000000010000,PS5 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000005e040000e002000001000000,PXN P30 Pro Mobile,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000222c00000225000000010000,Qanba Dragon Arcade Joystick PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000222c00000020000000010000,Qanba Drone Arcade Stick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000009b2800005600000020020000,Raphnet SNES Adapter,a:b1,b:b4,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b0,y:b5,",
"030000009b2800008000000022020000,Raphnet Wii Classic Adapter,a:b1,b:b4,back:b2,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,guide:b10,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a3,righty:a4,start:b3,x:b0,y:b5,",
"030000008916000000fd000000000000,Razer Onza TE,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"03000000321500000204000000010000,Razer Panthera PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000321500000104000000010000,Razer Panthera PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000321500000010000000010000,Razer Raiju,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000321500000507000001010000,Razer Raiju Mobile,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b21,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000321500000011000000010000,Razer Raion PS4 Fightpad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000321500000009000000020000,Razer Serval,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a4,rightx:a2,righty:a3,start:b7,x:b2,y:b3,",
"030000003215000000090000163a0000,Razer Serval,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a4,rightx:a2,righty:a3,start:b7,x:b2,y:b3,",
"0300000032150000030a000000000000,Razer Wildcat,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"03000000632500008005000000010000,Redgear,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000632500002305000000010000,Redragon Saturn,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000921200004547000000020000,Retro Bit Sega Genesis Controller Adapter,a:b0,b:b2,dpdown:+a2,dpleft:-a0,dpright:+a0,dpup:-a2,lefttrigger:b14,rightshoulder:b10,righttrigger:b4,start:b12,x:b6,y:b8,",
"03000000790000001100000000000000,Retro Controller,a:b1,b:b2,back:b8,dpdown:+a4,dpleft:-a3,dpright:+a3,dpup:-a4,leftshoulder:b6,lefttrigger:b7,rightshoulder:b4,righttrigger:b5,start:b9,x:b0,y:b3,",
"03000000790000001100000005010000,Retro Controller,a:b1,b:b2,back:b8,dpdown:+a4,dpleft:-a3,dpright:+a3,dpup:-a4,leftshoulder:b6,lefttrigger:b7,rightshoulder:b5,righttrigger:b4,start:b9,x:b0,y:b3,",
"03000000830500006020000000010000,Retro Controller,a:b0,b:b1,back:b6,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,lefttrigger:b5,rightshoulder:b8,righttrigger:b9,start:b7,x:b2,y:b3,",
"0300000003040000c197000000000000,Retrode Adapter,a:b0,b:b4,back:b2,dpdown:+a4,dpleft:-a0,dpright:+a0,dpup:-a4,leftshoulder:b6,rightshoulder:b7,start:b3,x:b1,y:b5,",
"03000000790000001100000006010000,Retrolink SNES Controller,a:b2,b:b1,back:b8,dpdown:+a4,dpleft:-a3,dpright:+a3,dpup:-a4,leftshoulder:b4,rightshoulder:b5,start:b9,x:b3,y:b0,",
"03000000341200000400000000000000,RetroUSB N64 RetroPort,+rightx:b8,+righty:b10,-rightx:b9,-righty:b11,a:b7,b:b6,dpdown:b2,dpleft:b1,dpright:b0,dpup:b3,leftshoulder:b13,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b12,start:b4,",
"030000006b140000010d000000010000,Revolution Pro Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000006b140000130d000000010000,Revolution Pro Controller 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000004c0500006802000002100000,Rii RK707,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b2,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b3,righttrigger:b9,rightx:a2,righty:a3,start:b1,x:b15,y:b12,",
"030000006f0e00008701000005010000,Rock Candy Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000c6240000fefa000000000000,Rock Candy PS3,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"03000000e804000000a000001b010000,Samsung EIGP20,a:b1,b:b3,back:b15,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b20,leftshoulder:b11,leftx:a1,lefty:a3,rightshoulder:b12,rightx:a4,righty:a5,start:b16,x:b7,y:b9,",
"03000000730700000401000000010000,Sanwa PlayOnline Mobile,a:b0,b:b1,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,start:b3,",
"03000000a30c00002500000006020000,Sega Genesis Mini 3B Controller,a:b2,b:b1,dpdown:+a4,dpleft:-a3,dpright:+a3,dpup:-a4,righttrigger:b5,start:b9,",
"03000000811700007e05000000000000,Sega Saturn,a:b2,b:b4,dpdown:b16,dpleft:b15,dpright:b14,dpup:b17,leftshoulder:b8,lefttrigger:a5,leftx:a0,lefty:a2,rightshoulder:b9,righttrigger:a4,start:b13,x:b0,y:b6,",
"03000000b40400000a01000000000000,Sega Saturn,a:b0,b:b1,back:b5,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,guide:b2,leftshoulder:b6,rightshoulder:b7,start:b8,x:b3,y:b4,",
"030000003512000021ab000000000000,SFC30 Joystick,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"0300000000f00000f100000000000000,SNES RetroPort,a:b2,b:b3,back:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b5,rightshoulder:b7,start:b6,x:b0,y:b1,",
"030000004c050000a00b000000000000,Sony DualShock 4 Adapter,a:b1,b:b2,back:b13,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000cc09000000000000,Sony DualShock 4 V2,a:b1,b:b2,back:b13,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000666600006706000088020000,Sony PlayStation Adapter,a:b2,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b6,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b5,rightx:a2,righty:a3,start:b11,x:b3,y:b0,",
"030000004c050000da0c000000010000,Sony PlayStation Classic Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b4,rightshoulder:b7,righttrigger:b5,start:b9,x:b3,y:b0,",
"030000004c0500003713000000010000,Sony PlayStation Vita,a:b1,b:b2,back:b8,dpdown:b13,dpleft:b15,dpright:b14,dpup:b12,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,rightx:a3,righty:a4,start:b9,x:b0,y:b3,",
"030000005e0400008e02000001000000,Steam Virtual Gamepad,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"03000000110100002014000000000000,SteelSeries Nimbus,a:b0,b:b1,dpdown:b9,dpleft:b11,dpright:b10,dpup:b8,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1~,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3~,x:b2,y:b3,",
"03000000110100002014000001000000,SteelSeries Nimbus,a:b0,b:b1,dpdown:b9,dpleft:b11,dpright:b10,dpup:b8,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1~,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3~,x:b2,y:b3,",
"03000000381000002014000001000000,SteelSeries Nimbus,a:b0,b:b1,dpdown:b9,dpleft:b11,dpright:b10,dpup:b8,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1~,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3~,x:b2,y:b3,",
"05000000484944204465766963650000,SteelSeries Nimbus Plus,a:b0,b:b1,back:b15,dpdown:b11,dpleft:b13,dpright:b12,dpup:b10,guide:b16,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1~,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3~,start:b14,x:b2,y:b3,",
"050000004e696d6275732b0000000000,SteelSeries Nimbus Plus,a:b0,b:b1,back:b15,dpdown:b11,dpleft:b13,dpright:b12,dpup:b10,guide:b16,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1~,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3~,start:b14,x:b2,y:b3,",
"03000000381000003014000000000000,SteelSeries Stratus Duo,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"03000000381000003114000000000000,SteelSeries Stratus Duo,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"03000000110100001714000000000000,SteelSeries Stratus XL,a:b0,b:b1,dpdown:b9,dpleft:b11,dpright:b10,dpup:b8,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1~,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3~,start:b12,x:b2,y:b3,",
"03000000110100001714000020010000,SteelSeries Stratus XL,a:b0,b:b1,dpdown:b9,dpleft:b11,dpright:b10,dpup:b8,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1~,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3~,start:b12,x:b2,y:b3,",
"030000000d0f0000f600000000010000,Switch Hori Pad,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000457500002211000000010000,SZMY Power PC Gamepad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000e40a00000307000001000000,Taito Egret II Mini Control Panel,a:b4,b:b2,back:b6,guide:b9,leftx:a0,lefty:a1,rightshoulder:b0,righttrigger:b1,start:b7,x:b8,y:b3,",
"03000000e40a00000207000001000000,Taito Egret II Mini Controller,a:b4,b:b2,back:b6,guide:b9,leftx:a0,lefty:a1,rightshoulder:b0,righttrigger:b1,start:b7,x:b8,y:b3,",
"03000000790000001c18000000010000,TGZ Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000790000001c18000003100000,TGZ Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000591c00002400000021000000,THEC64 Joystick,a:b0,b:b1,back:b6,leftshoulder:b4,leftx:a0,lefty:a4,rightshoulder:b5,start:b7,x:b2,y:b3,",
"03000000591c00002600000021000000,THEGamepad,a:b2,b:b1,back:b6,dpdown:+a4,dpleft:-a0,dpright:+a0,dpup:-a4,leftshoulder:b4,rightshoulder:b5,start:b7,x:b3,y:b0,",
"030000004f04000015b3000000000000,Thrustmaster Dual Analog 3.2,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"030000004f0400000ed0000000020000,Thrustmaster eSwap Pro Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000004f04000000b3000000000000,Thrustmaster Firestorm Dual Power,a:b0,b:b2,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b11,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,rightx:a2,righty:a3,start:b10,x:b1,y:b3,",
"03000000571d00002100000021000000,Tomee NES Controller Adapter,a:b1,b:b0,back:b2,dpdown:+a4,dpleft:-a0,dpright:+a0,dpup:-a4,start:b3,",
"03000000bd12000015d0000000010000,Tomee Retro Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b9,x:b3,y:b0,",
"03000000bd12000015d0000000000000,Tomee SNES Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b9,x:b3,y:b0,",
"03000000571d00002000000021000000,Tomee SNES Controller Adapter,a:b0,b:b1,back:b6,dpdown:+a4,dpleft:-a0,dpright:+a0,dpup:-a4,leftshoulder:b4,rightshoulder:b5,start:b7,x:b2,y:b3,",
"030000005f140000c501000000020000,Trust Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000100800000100000000000000,Twin USB Joystick,a:b4,b:b2,back:b16,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b12,leftstick:b20,lefttrigger:b8,leftx:a0,lefty:a2,rightshoulder:b14,rightstick:b22,righttrigger:b10,rightx:a6,righty:a4,start:b18,x:b6,y:b0,",
"03000000632500002605000000010000,Uberwith Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000151900005678000010010000,Uniplay U6,a:b3,b:b6,back:b25,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b17,leftstick:b31,lefttrigger:b21,leftx:a1,lefty:a3,rightshoulder:b19,rightstick:b33,righttrigger:b23,rightx:a4,righty:a5,start:b27,x:b11,y:b13,",
"030000006f0e00000302000025040000,Victrix PS4 Pro Fightstick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,touchpad:b13,x:b0,y:b3,",
"030000006f0e00000702000003060000,Victrix PS4 Pro Fightstick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,touchpad:b13,x:b0,y:b3,",
"050000005769696d6f74652028303000,Wii Remote,a:b4,b:b5,back:b7,dpdown:b3,dpleft:b0,dpright:b1,dpup:b2,guide:b8,leftshoulder:b11,lefttrigger:b12,leftx:a0,lefty:a1,start:b6,x:b10,y:b9,",
"050000005769696d6f74652028313800,Wii U Pro Controller,a:b16,b:b15,back:b7,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b8,leftshoulder:b19,leftstick:b23,lefttrigger:b21,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b24,righttrigger:b22,rightx:a2,righty:a3,start:b6,x:b18,y:b17,",
"030000005e0400008e02000000000000,Xbox 360 Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"030000005e0400008e02000010010000,Xbox 360 Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1~,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4~,start:b8,x:b2,y:b3,",
"030000006f0e00000104000000000000,Xbox 360 Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"03000000c6240000045d000000000000,Xbox 360 Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"030000005e0400000a0b000000000000,Xbox Adaptive Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"030000005e040000050b000003090000,Xbox Elite Controller Series 2,a:b0,b:b1,back:b31,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b53,leftshoulder:b6,leftstick:b13,lefttrigger:a6,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a5,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000130b000011050000,Xbox One Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000200b000011050000,Xbox One Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000200b000013050000,Xbox One Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000200b000015050000,Xbox One Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000d102000000000000,Xbox One Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"030000005e040000dd02000000000000,Xbox One Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"030000005e040000e002000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000e002000003090000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000e302000000000000,Xbox One Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"030000005e040000ea02000000000000,Xbox One Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"030000005e040000fd02000003090000,Xbox One Controller,a:b0,b:b1,back:b16,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c62400003a54000000000000,Xbox One PowerA Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b8,x:b2,y:b3,",
"030000005e040000130b000001050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000130b000005050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000130b000009050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000130b000013050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000130b000015050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000130b000007050000,Xbox Wireless Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000130b000017050000,Xbox Wireless Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000130b000022050000,Xbox Wireless Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000220b000017050000,Xbox Wireless Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000172700004431000029010000,XiaoMi Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a6,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"03000000120c0000100e000000010000,Zeroplus P4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000120c0000101e000000010000,Zeroplus P4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,"
#elif defined(TARGET_OS_IPHONE)
"05000000ac0500000100000000006d01,*,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b6,leftshoulder:b4,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a5,rightx:a3,righty:a4,x:b2,y:b3,platform:iOS,",
"05000000ac050000010000004f066d01,*,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b6,leftshoulder:b4,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a5,rightx:a3,righty:a4,x:b2,y:b3,platform:iOS,",
"05000000ac05000001000000cf076d01,*,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,x:b2,y:b3,platform:iOS,",
"05000000ac05000001000000df076d01,*,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b2,y:b3,platform:iOS,",
"05000000ac05000001000000ff076d01,*,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b3,platform:iOS,",
"05000000ac0500000200000000006d02,*,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b6,leftshoulder:b4,rightshoulder:b5,x:b2,y:b3,platform:iOS,",
"05000000ac050000020000004f066d02,*,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b6,leftshoulder:b4,rightshoulder:b5,x:b2,y:b3,platform:iOS,",
"05000000ac05000004000000a8986d04,8BitDo Micro gamepad,a:b1,b:b0,back:b4,dpdown:b7,dpleft:b8,dpright:b9,dpup:b10,guide:b2,leftshoulder:b11,lefttrigger:b12,rightshoulder:b13,righttrigger:b14,start:b3,x:b6,y:b5,platform:iOS,",
"05000000ac050000040000003b8a6d04,8BitDo SN30 Pro+,a:b1,b:b0,back:b4,dpdown:b7,dpleft:b8,dpright:b9,dpup:b10,guide:b2,leftshoulder:b11,leftstick:b12,lefttrigger:b13,leftx:a0,lefty:a1~,rightshoulder:b14,rightstick:b15,righttrigger:b16,rightx:a2,righty:a3~,start:b3,x:b6,y:b5,platform:iOS,",
"050000008a35000003010000ff070000,Backbone One,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b3,platform:iOS,",
"050000008a35000004010000ff070000,Backbone One,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b3,platform:iOS,",
"4d466947616d65706164010000000000,MFi Extended Gamepad,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a5,rightx:a3,righty:a4,start:b6,x:b2,y:b3,platform:iOS,",
"4d466947616d65706164020000000000,MFi Gamepad,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,rightshoulder:b5,start:b6,x:b2,y:b3,platform:iOS,",
"050000007e050000062000000f060000,Nintendo Switch Joy-Con (L),+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b2,leftshoulder:b4,rightshoulder:b5,x:b1,y:b3,platform:iOS,",
"050000007e050000062000004f060000,Nintendo Switch Joy-Con (L),+leftx:h0.1,+lefty:h0.2,-leftx:h0.4,-lefty:h0.8,dpdown:b2,dpleft:b0,dpright:b3,dpup:b1,leftshoulder:b4,misc1:b6,rightshoulder:b5,platform:iOS,",
"050000007e05000008200000df070000,Nintendo Switch Joy-Con (L/R),a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b2,y:b3,platform:iOS,",
"050000007e0500000e200000df070000,Nintendo Switch Joy-Con (L/R),a:b1,b:b0,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,platform:iOS,",
"050000007e050000072000004f060000,Nintendo Switch Joy-Con (R),+rightx:h0.4,+righty:h0.8,-rightx:h0.1,-righty:h0.2,a:b1,b:b0,guide:b6,leftshoulder:b4,rightshoulder:b5,x:b3,y:b2,platform:iOS,",
"050000007e05000009200000df870000,Nintendo Switch Pro Controller,a:b1,b:b0,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b10,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,platform:iOS,",
"050000007e05000009200000ff870000,Nintendo Switch Pro Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b3,platform:iOS,",
"050000004c050000cc090000df070000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b2,y:b3,platform:iOS,",
"050000004c050000cc090000df870001,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b2,y:b3,platform:iOS,",
"050000004c050000cc090000ff070000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b3,platform:iOS,",
"050000004c050000cc090000ff870001,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,touchpad:b11,x:b2,y:b3,platform:iOS,",
"050000004c050000cc090000ff876d01,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b3,platform:iOS,",
"050000004c050000e60c0000df870000,PS5 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b9,touchpad:b10,x:b2,y:b3,platform:iOS,",
"050000004c050000e60c0000ff870000,PS5 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,touchpad:b11,x:b2,y:b3,platform:iOS,",
"05000000ac0500000300000000006d03,Remote,a:b0,b:b2,leftx:a0,lefty:a1,platform:iOS,",
"05000000ac0500000300000043006d03,Remote,a:b0,b:b2,leftx:a0,lefty:a1,platform:iOS,",
"05000000de2800000511000001000000,Steam Controller,a:b0,b:b1,back:b6,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a3,start:b7,x:b2,y:b3,platform:iOS,",
"05000000de2800000611000001000000,Steam Controller,a:b0,b:b1,back:b6,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a3,start:b7,x:b2,y:b3,platform:iOS,",
"050000005e040000050b0000df070001,Xbox Elite Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,paddle1:b10,paddle2:b12,paddle3:b11,paddle4:b13,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b2,y:b3,platform:iOS,",
"050000005e040000050b0000ff070001,Xbox Elite Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,paddle1:b11,paddle2:b13,paddle3:b12,paddle4:b14,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b3,platform:iOS,",
"050000005e040000e0020000df070000,Xbox One Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b2,y:b3,platform:iOS,",
"050000005e040000e0020000ff070000,Xbox One Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b3,platform:iOS,",
"050000005e040000130b0000df870001,Xbox Series Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b10,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b2,y:b3,platform:iOS,",
"050000005e040000130b0000ff870001,Xbox Series Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b3,platform:iOS,"
#elif defined(__linux__)
    "03000000c82d00000031000011010000,8BitDo Adapter,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000631000000010000,8BitDo Adapter 2,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c82d00000951000000010000,8BitDo Dogbone,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,rightx:a2,righty:a3,start:b11,",
"03000000021000000090000011010000,8BitDo FC30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000090000011010000,8BitDo FC30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a5,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00001038000000010000,8BitDo FC30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00006a28000000010000,8BitDo GameCube,a:b0,b:b3,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b9,paddle2:b8,rightshoulder:b10,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b1,y:b4,",
"03000000c82d00001251000011010000,8BitDo Lite 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a5,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00001251000000010000,8BitDo Lite 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00001151000011010000,8BitDo Lite SE,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00001151000000010000,8BitDo Lite SE,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000151000000010000,8BitDo M30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000650000011010000,8BitDo M30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000c82d00005106000000010000,8BitDo M30,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b8,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,start:b11,x:b3,y:b4,",
"03000000c82d00000a20000000020000,8BitDo M30 Xbox,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,start:b7,x:b2,y:b3,",
"03000000c82d00002090000011010000,8BitDo Micro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00002090000000010000,8BitDo Micro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000451000000010000,8BitDo N30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,rightx:a2,righty:a3,start:b11,",
"03000000c82d00001590000011010000,8BitDo N30 Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00006528000000010000,8BitDo N30 Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00006928000011010000,8BitDo N64,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a3,start:b11,",
"05000000c82d00006928000000010000,8BitDo N64,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a3,start:b11,",
"05000000c82d00002590000001000000,8BitDo NEOGEO,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000008000000210000011010000,8BitDo NES30,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000c82d00000310000011010000,8BitDo NES30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b7,lefttrigger:b6,rightshoulder:b9,righttrigger:b8,start:b11,x:b3,y:b4,",
"05000000c82d00008010000000010000,8BitDo NES30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b7,lefttrigger:b6,rightshoulder:b9,righttrigger:b8,start:b11,x:b3,y:b4,",
"03000000022000000090000011010000,8BitDo NES30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000190000011010000,8BitDo NES30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a5,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000203800000900000000010000,8BitDo NES30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00002038000000010000,8BitDo NES30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000751000000010000,8BitDo P30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:a8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000c82d00000851000000010000,8BitDo P30,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:a8,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000660000011010000,8BitDo Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00001030000011010000,8BitDo Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00000660000000010000,8BitDo Pro 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000020000000000000,8BitDo Pro 2 for Xbox,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"06000000c82d00000020000006010000,8BitDo Pro 2 for Xbox,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c82d00000131000011010000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000231000011010000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000331000011010000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000431000011010000,8BitDo Receiver,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00002867000000010000,8BitDo S30,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b8,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:b7,rightx:a2,righty:a3,start:b10,x:b3,y:b4,",
"03000000c82d00000060000011010000,8BitDo SF30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00000060000000010000,8BitDo SF30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00000061000000010000,8BitDo SF30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"030000003512000012ab000010010000,8BitDo SFC30,a:b2,b:b1,back:b6,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b7,x:b3,y:b0,",
"030000003512000021ab000010010000,8BitDo SFC30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,start:b11,x:b4,y:b3,",
"03000000c82d000021ab000010010000,8BitDo SFC30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"05000000102800000900000000010000,8BitDo SFC30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"05000000c82d00003028000000010000,8BitDo SFC30,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"05000000c82d00000351000000010000,8BitDo SN30,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000160000000000000,8BitDo SN30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00000160000011010000,8BitDo SN30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000161000000000000,8BitDo SN30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"03000000c82d00001290000011010000,8BitDo SN30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a3,righty:a4,start:b11,x:b4,y:b3,",
"05000000c82d00000161000000010000,8BitDo SN30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b2,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00006228000000010000,8BitDo SN30 Pro,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00000260000011010000,8BitDo SN30 Pro Plus,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000c82d00000261000000010000,8BitDo SN30 Pro Plus,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"05000000202800000900000000010000,8BitDo SNES30,a:b1,b:b0,back:b10,dpdown:b122,dpleft:b119,dpright:b120,dpup:b117,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"05000000c82d00001230000000010000,8BitDo Ultimate,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000a31000014010000,8BitDo Ultimate 2C,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c82d00001d30000011010000,8BitDo Ultimate 2C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b5,paddle2:b2,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000c82d00001b30000001000000,8BitDo Ultimate 2C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b5,paddle2:b2,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001530000011010000,8BitDo Ultimate C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001630000011010000,8BitDo Ultimate C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001730000011010000,8BitDo Ultimate C,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001130000011010000,8BitDo Ultimate Wired,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b26,paddle1:b24,paddle2:b25,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000631000010010000,8BitDo Ultimate Wireless,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c82d00000760000011010000,8BitDo Ultimate Wireless,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c82d00001230000011010000,8BitDo Ultimate Wireless,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b2,paddle2:b5,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00001330000011010000,8BitDo Ultimate Wireless,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b26,paddle1:b23,paddle2:b19,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c82d00000631000014010000,8BitDo Ultimate Wireless Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c82d00000121000011010000,8BitDo Xbox One SN30 Pro,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000c82d00000121000000010000,8BitDo Xbox One SN30 Pro,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000a00500003232000001000000,8BitDo Zero,a:b0,b:b1,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b3,y:b4,",
"05000000a00500003232000008010000,8BitDo Zero,a:b0,b:b1,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b3,y:b4,",
"03000000c82d00001890000011010000,8BitDo Zero 2,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,",
"05000000c82d00003032000000010000,8BitDo Zero 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a2,righty:a3,start:b11,x:b4,y:b3,",
"03000000c01100000355000011010000,Acrux Gamepad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00008801000011010000,Afterglow Deluxe Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00003901000000430000,Afterglow Prismatic Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00003901000013020000,Afterglow Prismatic Controller 048-007-NA,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00001302000000010000,Afterglow Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00003901000020060000,Afterglow Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000100000008200000011010000,Akishop Customs PS360,a:b1,b:b2,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000007c1800000006000010010000,Alienware Dual Compatible Game PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b3,",
"05000000491900000204000021000000,Amazon Fire Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b17,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b12,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000491900001904000011010000,Amazon Luna Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b9,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b7,x:b2,y:b3,",
"05000000710100001904000000010000,Amazon Luna Controller,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"0300000008100000e501000001010000,Anbernic Handheld,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a4,start:b11,x:b3,y:b4,",
"03000000020500000913000010010000,Anbernic RG P01,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000373500000710000010010000,Anbernic RG P01,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000373500004610000001000000,Anbernic RG P01,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000790000003018000011010000,Arcade Fightstick F300,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000a30c00002700000011010000,Astro City Mini,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,rightshoulder:b4,righttrigger:b5,start:b9,x:b3,y:b0,",
"03000000a30c00002800000011010000,Astro City Mini,a:b2,b:b1,back:b8,leftx:a0,lefty:a1,rightshoulder:b4,righttrigger:b5,start:b9,x:b3,y:b0,",
"05000000050b00000045000031000000,ASUS Gamepad,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b6,leftshoulder:b4,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b10,x:b2,y:b3,",
"05000000050b00000045000040000000,ASUS Gamepad,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b6,leftshoulder:b4,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b10,x:b2,y:b3,",
"03000000050b00000579000011010000,ASUS ROG Kunai 3,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b36,paddle1:b52,paddle2:b53,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000050b00000679000000010000,ASUS ROG Kunai 3,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b21,paddle1:b22,paddle2:b23,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000503200000110000000000000,Atari VCS Classic Controller,a:b0,b:b1,back:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b4,start:b3,",
"03000000503200000110000011010000,Atari VCS Classic Controller,a:b0,b:b1,back:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b4,start:b3,",
"05000000503200000110000000000000,Atari VCS Classic Controller,a:b0,b:b1,back:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b4,start:b3,",
"05000000503200000110000044010000,Atari VCS Classic Controller,a:b0,b:b1,back:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b4,start:b3,",
"05000000503200000110000046010000,Atari VCS Classic Controller,a:b0,b:b1,back:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b4,start:b3,",
"03000000503200000210000000000000,Atari VCS Modern Controller,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a4,rightx:a2,righty:a3,start:b8,x:b2,y:b3,",
"03000000503200000210000011010000,Atari VCS Modern Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b3,y:b2,",
"05000000503200000210000000000000,Atari VCS Modern Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b3,y:b2,",
"05000000503200000210000045010000,Atari VCS Modern Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b3,y:b2,",
"05000000503200000210000046010000,Atari VCS Modern Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b3,y:b2,",
"05000000503200000210000047010000,Atari VCS Modern Controller,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b6,lefttrigger:+a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:-a4,rightx:a2,righty:a3,start:b8,x:b2,y:b3,",
"030000008a3500000201000011010000,Backbone One,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000008a3500000202000011010000,Backbone One,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000008a3500000302000011010000,Backbone One,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000008a3500000402000011010000,Backbone One,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000c62400001b89000011010000,BDA MOGA XP5X Plus,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000d62000002a79000011010000,BDA PS4 Fightpad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000c21100000791000011010000,Be1 GC101 Controller 1.03,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000c31100000791000011010000,Be1 GC101 Controller 1.03,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e0400008e02000003030000,Be1 GC101 Xbox 360,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000bc2000004d50000011010000,Beitong A1T2 BFM,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000bc2000000055000001000000,Betop AX1 BFM,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000bc2000006412000011010000,Betop Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b30,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000006b1400000209000011010000,Bigben,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000120c0000300e000011010000,Brook Audio Fighting Board PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000120c0000310e000011010000,Brook Audio Fighting Board PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000200e000011010000,Brook Mars PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000210e000011010000,Brook Mars PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000120c0000f70e000011010000,Brook Universal Fighting Board,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,rightshoulder:b5,rightstick:b11,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000d81d00000b00000010010000,Buffalo BSGP1601,a:b5,b:b3,back:b12,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b8,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b14,rightshoulder:b9,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b13,x:b4,y:b2,",
"03000000e82000006058000001010000,Cideko AK08b,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000af1e00002400000010010000,Clockwork Pi DevTerm,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,start:b9,x:b3,y:b0,",
"030000000b0400003365000000010000,Competition Pro,a:b0,b:b1,back:b2,leftx:a0,lefty:a1,start:b3,",
"03000000260900008888000000010000,Cyber Gadget GameCube Controller,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:a5,rightx:a2,righty:a3~,start:b7,x:b2,y:b3,",
"03000000a306000022f6000011010000,Cyborg V3 Rumble,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:+a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:-a3,rightx:a2,righty:a4,start:b9,x:b0,y:b3,",
"030000005e0400008e02000002010000,Data Frog S80,a:b1,b:b0,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b3,y:b2,",
"03000000791d00000103000010010000,Dual Box Wii Classic Adapter,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b6,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b5,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000006f0e00003001000001010000,EA Sports PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000c11100000191000011010000,EasySMX,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000242f00009100000000010000,EasySMX ESM-9101,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006e0500000320000010010000,Elecom U3613M,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3,start:b11,x:b0,y:b1,",
"030000006e0500000720000010010000,Elecom W01U,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b1,",
"030000007d0400000640000010010000,Eliminator AfterShock,a:b1,b:b2,back:b9,dpdown:+a3,dpleft:-a5,dpright:+a5,dpup:-a3,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a4,righty:a2,start:b8,x:b0,y:b3,",
"03000000430b00000300000000010000,EMS Production PS2 Adapter,a:b2,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a5,righty:a2,start:b9,x:b3,y:b0,",
"030000006f0e00008401000011010000,Faceoff Deluxe Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00008101000011010000,Faceoff Deluxe Pro Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00008001000011010000,Faceoff Pro Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03005036852100000201000010010000,Final Fantasy XIV Online Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"05000000b40400001224000001010000,Flydigi APEX 4,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b14,leftshoulder:b4,leftstick:b10,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b20,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000b40400001124000011010000,Flydigi Vader 2,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b12,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b14,paddle1:b2,paddle2:b5,paddle3:b16,paddle4:b17,rightshoulder:b7,rightstick:b13,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000b40400001224000011010000,Flydigi Vader 2,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b12,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b2,paddle1:b16,paddle2:b17,paddle3:b14,paddle4:b15,rightshoulder:b7,rightstick:b13,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000151900004000000001000000,Flydigi Vader 2,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b21,leftshoulder:b6,leftstick:b12,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b14,paddle1:b2,paddle2:b5,paddle3:b16,paddle4:b17,rightshoulder:b7,rightstick:b13,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000007e0500003703000000000000,GameCube Adapter,a:b0,b:b1,dpdown:b9,dpleft:b10,dpright:b11,dpup:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b6,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b3,y:b2,",
"19000000030000000300000002030000,GameForce Controller,a:b1,b:b0,back:b8,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,guide:b16,leftshoulder:b4,leftstick:b14,lefttrigger:b6,leftx:a1,lefty:a0,rightshoulder:b5,rightstick:b15,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b2,y:b3,",
"03000000ac0500005b05000010010000,GameSir G3w,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000bc2000000055000011010000,GameSir G3w,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000558500001b06000010010000,GameSir G4 Pro,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000ac0500002d0200001b010000,GameSir G4s,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b33,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000ac0500007a05000011010000,GameSir G5,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b16,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000373500009710000001020000,GameSir Kaleid Flux,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000bc2000005656000011010000,GameSir T4w,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000ac0500001a06000011010000,GameSir-T3 2.02,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"0500000047532047616d657061640000,GameStop Gamepad,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000006f0e00000104000000010000,Gamestop Logic3 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000008f0e00000800000010010000,Gasia PlayStation Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000451300000010000010010000,Genius Maxfire Grandias 12,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000f0250000c283000010010000,Gioteck VX2 PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"190000004b4800000010000000010000,GO-Advance Controller,a:b1,b:b0,back:b10,dpdown:b7,dpleft:b8,dpright:b9,dpup:b6,leftshoulder:b4,lefttrigger:b12,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b13,start:b15,x:b2,y:b3,",
"190000004b4800000010000001010000,GO-Advance Controller,a:b1,b:b0,back:b12,dpdown:b9,dpleft:b10,dpright:b11,dpup:b8,leftshoulder:b4,leftstick:b13,lefttrigger:b14,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b16,righttrigger:b15,start:b17,x:b2,y:b3,",
"190000004b4800000011000000010000,GO-Super Gamepad,a:b0,b:b1,back:b12,dpdown:b9,dpleft:b10,dpright:b11,dpup:b8,guide:b16,leftshoulder:b4,leftstick:b14,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b15,righttrigger:b7,rightx:a2,righty:a3,start:b13,x:b3,y:b2,",
"03000000f0250000c183000010010000,Goodbetterbest Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d11800000094000011010000,Google Stadia Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a4,rightx:a2,righty:a3,start:b7,x:b2,y:b3,",
"05000000d11800000094000000010000,Google Stadia Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a4,rightx:a2,righty:a3,start:b7,x:b2,y:b3,",
"0300000079000000d418000000010000,GPD Win 2 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400008e02000001010000,GPD Win Max 2 6800U Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000007d0400000540000000010000,Gravis Eliminator Pro,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000280400000140000000010000,Gravis GamePad Pro,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000008f0e00000610000000010000,GreenAsia Electronics Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b9,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b10,righttrigger:b5,rightx:a3,righty:a2,start:b11,x:b3,y:b0,",
"030000008f0e00001200000010010000,GreenAsia Joystick,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b2,y:b3,",
"0500000047532067616d657061640000,GS gamepad,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000f0250000c383000010010000,GT VX2,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"06000000adde0000efbe000002010000,Hidromancer Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d81400000862000011010000,HitBox PS3 PC Analog Mode,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,guide:b9,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b12,x:b0,y:b3,",
"03000000c9110000f055000011010000,HJC Gamepad,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000000d0f00006d00000020010000,Hori EDGE 301,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:+a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:+a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f00008400000011010000,Hori Fighting Commander,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00005f00000011010000,Hori Fighting Commander 4 PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00005e00000011010000,Hori Fighting Commander 4 PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f00005001000009040000,Hori Fighting Commander Octa Xbox One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f00008500000010010000,Hori Fighting Commander PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00008600000002010000,Hori Fighting Commander Xbox 360,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000000d0f00003701000013010000,Hori Fighting Stick Mini,a:b1,b:b0,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,lefttrigger:a2,rightshoulder:b5,righttrigger:a5,start:b7,x:b3,y:b2,",
"030000000d0f00008800000011010000,Hori Fighting Stick mini 4 PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,rightshoulder:b5,rightstick:b11,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f00008700000011010000,Hori Fighting Stick mini 4 PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,rightshoulder:b5,rightstick:b11,righttrigger:a4,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f00001000000011010000,Hori Fightstick 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000ad1b000003f5000033050000,Hori Fightstick VX,+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b8,guide:b10,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b2,y:b3,",
"030000000d0f00004d00000011010000,Hori Gem Pad 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000ad1b000001f5000033050000,Hori Pad EX Turbo 2,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f00003801000011010000,Hori PC Engine Mini Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,start:b9,",
"030000000d0f00009200000011010000,Hori Pokken Tournament DX Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f00001100000011010000,Hori Real Arcade Pro 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00002200000011010000,Hori Real Arcade Pro 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f00006a00000011010000,Hori Real Arcade Pro 4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f00006b00000011010000,Hori Real Arcade Pro 4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00001600000000010000,Hori Real Arcade Pro EXSE,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b2,y:b3,",
"030000000d0f0000aa00000011010000,Hori Real Arcade Pro for Nintendo Switch,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000000d0f00008501000017010000,Hori Split Pad Fit,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f00008501000015010000,Hori Switch Split Pad Pro,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f00006e00000011010000,Horipad 4 PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00006600000011010000,Horipad 4 PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f0000ee00000011010000,Horipad Mini 4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b13,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000000d0f0000c100000011010000,Horipad Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000000d0f00006700000001010000,Horipad One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f0000ab01000011010000,Horipad Steam,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc2:b2,paddle1:b19,paddle2:b18,paddle3:b15,paddle4:b5,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000000d0f00009601000091000000,Horipad Steam,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc2:b2,paddle1:b19,paddle2:b18,paddle3:b15,paddle4:b5,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000000d0f0000f600000001000000,Horipad Switch Pro Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000341a000005f7000010010000,HuiJia GameCube Controller Adapter,a:b1,b:b2,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b0,y:b3,",
"05000000242e00000b20000001000000,Hyperkin Admiral N64 Controller,+rightx:b11,+righty:b13,-rightx:b8,-righty:b12,a:b1,b:b0,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b14,leftx:a0,lefty:a1,rightshoulder:b5,start:b9,",
"03000000242e0000ff0b000011010000,Hyperkin N64 Adapter,a:b1,b:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightx:a2,righty:a3,start:b9,",
"03000000242e00006a38000010010000,Hyperkin Trooper 2,a:b0,b:b1,back:b4,leftshoulder:b2,leftx:a0,lefty:a1,rightshoulder:b3,start:b5,",
"03000000242e00008816000001010000,Hyperkin X91,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000f00300008d03000011010000,HyperX Clutch,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000830500006020000010010000,iBuffalo Super Famicom Controller,a:b1,b:b0,back:b6,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b7,x:b3,y:b2,",
"030000008f0e00001330000001010000,iCode Retro Adapter,b:b3,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b9,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b1,start:b7,x:b2,y:b0,",
"050000006964726f69643a636f6e0000,idroidcon Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000b50700001503000010010000,Impact,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b1,",
"03000000d80400008200000003000000,IMS PCU0,a:b1,b:b0,back:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,start:b5,x:b3,y:b2,",
"03000000120c00000500000010010000,InterAct AxisPad,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a3,righty:a2,start:b11,x:b0,y:b1,",
"03000000ef0500000300000000010000,InterAct AxisPad,a:b2,b:b3,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a3,righty:a2,start:b11,x:b0,y:b1,",
"03000000fd0500000030000000010000,InterAct GoPad,a:b3,b:b4,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,x:b0,y:b1,",
"03000000fd0500002a26000000010000,InterAct HammerHead FX,a:b3,b:b4,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b2,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b5,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b0,y:b1,",
"0500000049190000020400001b010000,Ipega PG 9069,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b161,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000632500007505000011010000,Ipega PG 9099,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"0500000049190000030400001b010000,Ipega PG9099,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000491900000204000000000000,Ipega PG9118,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000300f00001101000010010000,Jess Tech Colour Rumble Pad,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b1,",
"03000000300f00001001000010010000,Jess Tech Dual Analog Rumble,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b1,",
"03000000300f00000b01000010010000,Jess Tech GGE909 PC Recoil,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"03000000ba2200002010000001010000,Jess Technology Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"030000007e0500000620000001000000,Joy-Con (L),+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b13,leftshoulder:b4,leftstick:b10,rightshoulder:b5,start:b8,x:b2,y:b3,",
"050000007e0500000620000001000000,Joy-Con (L),+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b13,leftshoulder:b4,leftstick:b10,rightshoulder:b5,start:b8,x:b2,y:b3,",
"030000007e0500000720000001000000,Joy-Con (R),+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b12,leftshoulder:b4,leftstick:b11,rightshoulder:b5,start:b9,x:b2,y:b3,",
"050000007e0500000720000001000000,Joy-Con (R),+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b12,leftshoulder:b4,leftstick:b11,rightshoulder:b5,start:b9,x:b2,y:b3,",
"03000000bd12000003c0000010010000,Joypad Alpha Shock,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000242f00002d00000011010000,JYS Adapter,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000242f00008a00000011010000,JYS Adapter,a:b1,b:b4,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b0,y:b3,",
"030000006f0e00000103000000020000,Logic3 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006d040000d1ca000000000000,Logitech Chillstream,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d040000d1ca000011010000,Logitech Chillstream,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000019c2000010010000,Logitech Cordless RumblePad 2,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000016c2000010010000,Logitech Dual Action,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
    "030000006d0400001dc2000014400000,Logitech F310,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006d0400001ec2000019200000,Logitech F510,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006d0400001ec2000020200000,Logitech F510,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006d04000019c2000011010000,Logitech F710,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d0400001fc2000005030000,Logitech F710,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006d04000018c2000010010000,Logitech RumblePad 2,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006d04000011c2000010010000,Logitech WingMan Cordless RumblePad,a:b0,b:b1,back:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b5,leftshoulder:b6,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b10,rightx:a3,righty:a4,start:b8,x:b3,y:b4,",
"030000006d0400000ac2000010010000,Logitech WingMan RumblePad,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,rightx:a3,righty:a4,x:b3,y:b4,",
"05000000380700006652000025010000,Mad Catz CTRLR,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700008532000010010000,Mad Catz Fightpad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b5,rightshoulder:b6,righttrigger:b7,start:b9,x:b0,y:b3,",
"03000000380700005032000011010000,Mad Catz Fightpad Pro PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700005082000011010000,Mad Catz Fightpad Pro PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000ad1b00002ef0000090040000,Mad Catz Fightpad SFxT,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,lefttrigger:a2,rightshoulder:b5,righttrigger:a5,start:b7,x:b2,y:b3,",
"03000000380700008031000011010000,Mad Catz FightStick Alpha PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700008081000011010000,Mad Catz FightStick Alpha PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700008034000011010000,Mad Catz Fightstick PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700008084000011010000,Mad Catz Fightstick PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000380700008433000011010000,Mad Catz Fightstick TE S PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700008483000011010000,Mad Catz Fightstick TE S PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000380700001888000010010000,Mad Catz Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700003888000010010000,Mad Catz Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:a0,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000380700001647000010040000,Mad Catz Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000380700003847000090040000,Mad Catz Xbox 360 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000ad1b000016f0000090040000,Mad Catz Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000120c00000500000000010000,Manta DualShock 2,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b2,y:b3,",
"030000008f0e00001330000010010000,Mayflash Controller Adapter,a:b1,b:b2,back:b8,dpdown:h0.8,dpleft:h0.2,dpright:h0.1,dpup:h0.4,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a3~,righty:a2,start:b9,x:b0,y:b3,",
"03000000790000004318000010010000,Mayflash GameCube Adapter,a:b1,b:b0,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b2,y:b3,",
"03000000790000004418000010010000,Mayflash GameCube Controller,a:b1,b:b0,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b2,y:b3,",
"03000000242f00007300000011010000,Mayflash Magic NS,a:b1,b:b4,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b0,y:b3,",
"0300000079000000d218000011010000,Mayflash Magic NS,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d620000010a7000011010000,Mayflash Magic NS,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000242f0000f700000001010000,Mayflash Magic S Pro,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000008f0e00001030000010010000,Mayflash Saturn Adapter,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,lefttrigger:b7,rightshoulder:b6,righttrigger:b2,start:b9,x:b3,y:b4,",
"0300000025090000e803000001010000,Mayflash Wii Classic Adapter,a:b1,b:b0,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:a4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:a5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b2,",
"03000000790000000318000011010000,Mayflash Wii DolphinBar,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b11,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b1,",
"03000000790000000018000011010000,Mayflash Wii U Pro Adapter,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000b50700001203000010010000,Mega World Logic 3 Controller,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b1,",
"03000000b50700004f00000000010000,Mega World Logic 3 Controller,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b1,",
"03000000780000000600000010010000,Microntek Joystick,a:b2,b:b1,back:b8,leftshoulder:b6,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b5,start:b9,x:b3,y:b0,",
"030000005e0400002800000000010000,Microsoft Dual Strike,a:b3,b:b2,back:b4,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b8,rightshoulder:b7,rightx:a0,righty:a1~,start:b5,x:b1,y:b0,",
"030000005e0400000300000000010000,Microsoft SideWinder,a:b0,b:b1,back:b9,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b8,x:b3,y:b4,",
"030000005e0400000700000000010000,Microsoft SideWinder,a:b0,b:b1,back:b8,leftshoulder:b6,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b2,start:b9,x:b3,y:b4,",
"030000005e0400000e00000000010000,Microsoft SideWinder Freestyle Pro,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,rightshoulder:b7,start:b8,x:b3,y:b4,",
"030000005e0400002700000000010000,Microsoft SideWinder Plug and Play,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,lefttrigger:b4,righttrigger:b5,x:b2,y:b3,",
"030000005e0400008502000000010000,Microsoft Xbox,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b2,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b3,y:b4,",
"030000005e0400008902000021010000,Microsoft Xbox,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b2,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b3,y:b4,",
"030000005e0400008e02000001000000,Microsoft Xbox 360,a:b0,b:b1,back:b6,dpdown:h0.1,dpleft:h0.2,dpright:h0.8,dpup:h0.4,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400008e02000004010000,Microsoft Xbox 360,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400008e02000056210000,Microsoft Xbox 360,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400008e02000062230000,Microsoft Xbox 360,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000d102000001010000,Microsoft Xbox One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000d102000003020000,Microsoft Xbox One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000dd02000003020000,Microsoft Xbox One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000ea02000008040000,Microsoft Xbox One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000ea0200000f050000,Microsoft Xbox One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"060000005e040000120b000009050000,Microsoft Xbox One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000e302000003020000,Microsoft Xbox One Elite,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000000b000007040000,Microsoft Xbox One Elite 2,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,paddle1:b12,paddle2:b14,paddle3:b13,paddle4:b15,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000000b000008040000,Microsoft Xbox One Elite 2,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,paddle1:b12,paddle2:b14,paddle3:b13,paddle4:b15,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"050000005e040000050b000003090000,Microsoft Xbox One Elite 2,a:b0,b:b1,back:b17,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a6,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a5,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e0400008e02000030110000,Microsoft Xbox One Elite 2,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,paddle1:b11,paddle2:b13,paddle3:b12,paddle4:b14,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b00000b050000,Microsoft Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b000016050000,Microsoft Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b000017050000,Microsoft Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"060000005e040000120b000001050000,Microsoft Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000030000000300000002000000,Miroof,a:b1,b:b0,back:b6,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,start:b7,x:b3,y:b2,",
"03000000790000001c18000010010000,Mobapad Chitu HD,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000004d4f435554452d3035335800,Mocute 053X,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"05000000e80400006e0400001b010000,Mocute 053X M59,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000004d4f435554452d3035305800,Mocute 054X,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000d6200000e589000001000000,Moga 2,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"05000000d6200000ad0d000001000000,Moga Pro,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"05000000d62000007162000001000000,Moga Pro 2,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"03000000c62400002b89000011010000,MOGA XP5A Plus,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000c62400002a89000000010000,MOGA XP5A Plus,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b22,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000c62400001a89000000010000,MOGA XP5X Plus,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000250900006688000000010000,MP8866 Super Dual Box,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"030000005e0400008e02000010020000,MSI GC20 V2,a:b0,b:b1,back:b6,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000f70600000100000000010000,N64 Adaptoid,+rightx:b2,+righty:b1,-rightx:b4,-righty:b5,a:b0,b:b3,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b6,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b7,start:b8,",
"030000006b1400000906000014010000,Nacon Asymmetric Wireless PS4 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006b140000010c000010010000,Nacon GC 400ES,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"03000000853200000706000012010000,Nacon GC-100,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"05000000853200000503000000010000,Nacon MG-X Pro,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"0300000085320000170d000011010000,Nacon Revolution 5 Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"0300000085320000190d000011010000,Nacon Revolution 5 Pro,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000000d0f00000900000010010000,Natec Genesis P44,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000004f1f00000800000011010000,NeoGeo PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"0300000092120000474e000000010000,NeoGeo X Arcade Stick,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,start:b9,x:b3,y:b2,",
"03000000790000004518000010010000,Nexilux GameCube Controller Adapter,a:b1,b:b0,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:a4,rightx:a5,righty:a2,start:b9,x:b2,y:b3,",
"030000001008000001e5000010010000,NEXT SNES Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,righttrigger:b6,start:b9,x:b3,y:b0,",
"060000007e0500003713000000000000,Nintendo 3DS,a:b0,b:b1,back:b8,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,rightx:a2,righty:a3,start:b9,x:b3,y:b2,",
"030000007e0500003703000000016800,Nintendo GameCube Controller,a:b0,b:b2,dpdown:b6,dpleft:b4,dpright:b5,dpup:b7,lefttrigger:a4,leftx:a0,lefty:a1~,rightshoulder:b9,righttrigger:a5,rightx:a2,righty:a3~,start:b8,x:b1,y:b3,",
"03000000790000004618000010010000,Nintendo GameCube Controller Adapter,a:b1,b:b0,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b5,rightx:a5~,righty:a2~,start:b9,x:b2,y:b3,",
"060000004e696e74656e646f20537700,Nintendo Switch Combined Joy-Cons,a:b0,b:b1,back:b9,dpdown:b15,dpleft:b16,dpright:b17,dpup:b14,guide:b11,leftshoulder:b5,leftstick:b12,lefttrigger:b7,leftx:a0,lefty:a1,misc1:b4,rightshoulder:b6,rightstick:b13,righttrigger:b8,rightx:a2,righty:a3,start:b10,x:b3,y:b2,",
"060000007e0500000620000000000000,Nintendo Switch Combined Joy-Cons,a:b0,b:b1,back:b9,dpdown:b15,dpleft:b16,dpright:b17,dpup:b14,guide:b11,leftshoulder:b5,leftstick:b12,lefttrigger:b7,leftx:a0,lefty:a1,misc1:b4,rightshoulder:b6,rightstick:b13,righttrigger:b8,rightx:a2,righty:a3,start:b10,x:b3,y:b2,",
"060000007e0500000820000000000000,Nintendo Switch Combined Joy-Cons,a:b0,b:b1,back:b9,dpdown:b15,dpleft:b16,dpright:b17,dpup:b14,guide:b11,leftshoulder:b5,leftstick:b12,lefttrigger:b7,leftx:a0,lefty:a1,misc1:b4,rightshoulder:b6,rightstick:b13,righttrigger:b8,rightx:a2,righty:a3,start:b10,x:b3,y:b2,",
"050000004c69632050726f20436f6e00,Nintendo Switch Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"050000007e0500000620000001800000,Nintendo Switch Left Joy-Con,a:b16,b:b15,back:b4,leftshoulder:b6,leftstick:b12,leftx:a1,lefty:a0~,rightshoulder:b8,start:b9,x:b14,y:b17,",
"030000007e0500000920000000026803,Nintendo Switch Pro Controller,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"030000007e0500000920000011810000,Nintendo Switch Pro Controller,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b11,leftshoulder:b5,leftstick:b12,lefttrigger:b7,leftx:a0,lefty:a1,misc1:b4,rightshoulder:b6,rightstick:b13,righttrigger:b8,rightx:a2,righty:a3,start:b10,x:b3,y:b2,",
"050000007e0500000920000001000000,Nintendo Switch Pro Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"050000007e0500000920000001800000,Nintendo Switch Pro Controller,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b11,leftshoulder:b5,leftstick:b12,lefttrigger:b7,leftx:a0,lefty:a1,misc1:b4,rightshoulder:b6,rightstick:b13,righttrigger:b8,rightx:a2,righty:a3,start:b10,x:b3,y:b2,",
"050000007e0500000720000001800000,Nintendo Switch Right Joy-Con,a:b1,b:b2,back:b9,leftshoulder:b4,leftstick:b10,leftx:a1~,lefty:a0,rightshoulder:b6,start:b8,x:b0,y:b3,",
"05000000010000000100000003000000,Nintendo Wii Remote,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"050000007e0500003003000001000000,Nintendo Wii U Pro Controller,a:b0,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b16,dpup:b13,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b2,",
"050000005a1d00000218000003000000,Nokia GC 5000,a:b9,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000000d0500000308000010010000,Nostromo n45 Dual Analog,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b4,leftstick:b12,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b10,x:b2,y:b3,",
"030000007e0500001920000011810000,NSO N64 Controller,+rightx:b2,+righty:b3,-rightx:b4,-righty:b10,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,misc1:b5,rightshoulder:b7,righttrigger:b9,start:b11,",
"050000007e0500001920000001000000,NSO N64 Controller,+rightx:b8,+righty:b7,-rightx:b3,-righty:b2,a:b1,b:b0,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,righttrigger:b10,start:b9,",
"050000007e0500001920000001800000,NSO N64 Controller,+rightx:b2,+righty:b3,-rightx:b4,-righty:b10,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,lefttrigger:b8,leftx:a0,lefty:a1,misc1:b5,rightshoulder:b7,righttrigger:b9,start:b11,",
"030000007e0500001e20000011810000,NSO Sega Genesis Controller,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b6,misc1:b3,rightshoulder:b2,righttrigger:b4,start:b5,",
"030000007e0500001720000011810000,NSO SNES Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b3,y:b2,",
"050000007e0500001720000001000000,NSO SNES Controller,a:b0,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,lefttrigger:b7,rightshoulder:b6,righttrigger:b8,start:b10,x:b3,y:b2,",
"050000007e0500001720000001800000,NSO SNES Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b3,y:b2,",
"03000000550900001072000011010000,NVIDIA Controller,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b13,leftshoulder:b4,leftstick:b8,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a4,rightx:a2,righty:a3,start:b7,x:b2,y:b3,",
"03000000550900001472000011010000,NVIDIA Controller,a:b0,b:b1,back:b14,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b16,leftshoulder:b4,leftstick:b7,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b8,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a5,start:b6,x:b2,y:b3,",
"05000000550900001472000001000000,NVIDIA Controller,a:b0,b:b1,back:b14,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b16,leftshoulder:b4,leftstick:b7,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b8,rightshoulder:b5,rightstick:b8,righttrigger:a4,rightx:a2,righty:a5,start:b6,x:b2,y:b3,",
"030000004b120000014d000000010000,NYKO Airflo EX,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b2,y:b3,",
"03000000451300000830000010010000,NYKO CORE,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"19000000010000000100000001010000,ODROID Go 2,a:b1,b:b0,dpdown:b7,dpleft:b8,dpright:b9,dpup:b6,guide:b10,leftshoulder:b4,leftstick:b12,lefttrigger:b11,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b13,righttrigger:b14,start:b15,x:b2,y:b3,",
"19000000010000000200000011000000,ODROID Go 2,a:b1,b:b0,dpdown:b9,dpleft:b10,dpright:b11,dpup:b8,guide:b12,leftshoulder:b4,leftstick:b14,lefttrigger:b13,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b15,righttrigger:b16,start:b17,x:b2,y:b3,",
"05000000362800000100000002010000,OUYA Controller,a:b0,b:b3,dpdown:b9,dpleft:b10,dpright:b11,dpup:b8,guide:b14,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,x:b1,y:b2,",
"05000000362800000100000003010000,OUYA Controller,a:b0,b:b3,dpdown:b9,dpleft:b10,dpright:b11,dpup:b8,guide:b14,leftshoulder:b4,leftstick:b6,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,x:b1,y:b2,",
"05000000362800000100000004010000,OUYA Controller,a:b0,b:b3,back:b14,dpdown:b9,dpleft:b10,dpright:b11,dpup:b8,leftshoulder:b4,leftstick:b6,lefttrigger:b12,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b7,righttrigger:b13,rightx:a3,righty:a4,start:b16,x:b1,y:b2,",
"03000000830500005020000010010000,Padix Rockfire PlayStation Bridge,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3,start:b11,x:b2,y:b3,",
"03000000ff1100003133000010010000,PC Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000006f0e0000b802000001010000,PDP Afterglow Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e0000b802000013020000,PDP Afterglow Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00006401000001010000,PDP Battlefield One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e0000d702000006640000,PDP Black Camo Wired Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:b13,dpleft:b14,dpright:b13,dpup:b14,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00003101000000010000,PDP EA Sports Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00008501000011010000,PDP Fightpad Pro Gamecube Controller,a:b1,b:b0,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000006f0e0000c802000012010000,PDP Kingdom Hearts Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00002801000011010000,PDP PS3 Rock Candy Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00000901000011010000,PDP PS3 Versus Fighting,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000006f0e00002f01000011010000,PDP Wired PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000ad1b000004f9000000010000,PDP Xbox 360 Versus Fighting,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,lefttrigger:a2,rightshoulder:b5,righttrigger:a5,start:b7,x:b2,y:b3,",
"030000006f0e0000f102000000000000,PDP Xbox Atomic,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e0000a802000023020000,PDP Xbox One Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000006f0e0000a702000023020000,PDP Xbox One Raven Black,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e0000d802000006640000,PDP Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e0000ef02000007640000,PDP Xbox Series Kinetic Wired Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c62400000053000000010000,PowerA,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c62400003a54000001010000,PowerA 1428124-01,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d62000000540000001010000,PowerA Advantage Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d620000011a7000011010000,PowerA Core Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000dd62000015a7000011010000,PowerA Fusion Nintendo Switch Arcade Stick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d620000012a7000011010000,PowerA Fusion Nintendo Switch Fight Pad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d62000000140000001010000,PowerA Fusion Pro 2 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000dd62000016a7000000000000,PowerA Fusion Pro Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000c62400001a53000000010000,PowerA Mini Pro Ex,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d620000013a7000011010000,PowerA Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d62000006dca000011010000,PowerA Pro Ex,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000d620000014a7000011010000,PowerA Spectra Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000c62400001a58000001010000,PowerA Xbox One,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d62000000220000001010000,PowerA Xbox One Controller,a:b0,b:b1,back:b7,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"03000000d62000000228000001010000,PowerA Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c62400001a54000001010000,PowerA Xbox One Mini Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d62000000240000001010000,PowerA Xbox One Spectra Infinity,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d62000000520000050010000,PowerA Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d62000000b20000001010000,PowerA Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000d62000000f20000001010000,PowerA Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b7,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006d040000d2ca000011010000,Precision Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000250900000017000010010000,PS/SS/N64 Adapter,a:b1,b:b2,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b5,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a2~,righty:a3,start:b8,",
"03000000ff1100004133000010010000,PS2 Controller,a:b2,b:b1,back:b8,leftshoulder:b6,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b5,start:b9,x:b3,y:b0,",
"03000000341a00003608000011010000,PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000004c0500006802000010010000,PS3 Controller,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:b9,rightx:a2,righty:a3,start:b3,x:b15,y:b12,",
"030000004c0500006802000010810000,PS3 Controller,a:b0,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b16,dpup:b13,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"030000004c0500006802000011010000,PS3 Controller,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:b9,rightx:a2,righty:a3,start:b3,x:b15,y:b12,",
"030000004c0500006802000011810000,PS3 Controller,a:b0,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b16,dpup:b13,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"030000005f1400003102000010010000,PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000006f0e00001402000011010000,PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000008f0e00000300000010010000,PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"050000004c0500006802000000000000,PS3 Controller,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:b9,rightx:a2,righty:a3,start:b3,x:b15,y:b12,",
"050000004c0500006802000000010000,PS3 Controller,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:a12,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:a13,rightx:a2,righty:a3,start:b3,x:b15,y:b12,",
"050000004c0500006802000000800000,PS3 Controller,a:b0,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b16,dpup:b13,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"050000004c0500006802000000810000,PS3 Controller,a:b0,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b16,dpup:b13,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"05000000504c415953544154494f4e00,PS3 Controller,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:b9,rightx:a2,righty:a3,start:b3,x:b15,y:b12,",
"060000004c0500006802000000010000,PS3 Controller,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:b9,rightx:a2,righty:a3,start:b3,x:b15,y:b12,",
"030000004c050000a00b000011010000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000a00b000011810000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"030000004c050000c405000000810000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"030000004c050000c405000011010000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000c405000011810000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"030000004c050000cc09000000010000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000cc09000011010000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000cc09000011810000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"03000000c01100000140000011010000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"050000004c050000c405000000010000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"050000004c050000c405000000810000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"050000004c050000c405000001800000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"050000004c050000cc09000000010000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"050000004c050000cc09000000810000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"050000004c050000cc09000001800000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"0300004b4c0500005f0e000011010000,PS5 Access Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b14,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000e60c000011010000,PS5 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b14,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000e60c000011810000,PS5 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"030000004c050000f20d000011010000,PS5 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b14,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000004c050000f20d000011810000,PS5 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"050000004c050000e60c000000010000,PS5 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"050000004c050000e60c000000810000,PS5 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"050000004c050000f20d000000010000,PS5 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"050000004c050000f20d000000810000,PS5 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b11,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:a5,rightx:a3,righty:a4,start:b9,x:b3,y:b2,",
"03000000300f00001211000011010000,Qanba Arcade Joystick,a:b2,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b5,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b6,start:b9,x:b1,y:b3,",
"03000000222c00000225000011010000,Qanba Dragon Arcade Joystick PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000222c00000025000011010000,Qanba Dragon Arcade Joystick PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000222c00001220000011010000,Qanba Drone 2 Arcade Joystick PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000222c00001020000011010000,Qanba Drone 2 Arcade Joystick PS5,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000222c00000020000011010000,Qanba Drone Arcade PS4 Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,rightshoulder:b5,righttrigger:a4,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000300f00001210000010010000,Qanba Joystick Plus,a:b0,b:b1,back:b8,leftshoulder:b5,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b4,righttrigger:b6,start:b9,x:b2,y:b3,",
"03000000222c00000223000011010000,Qanba Obsidian Arcade Joystick PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000222c00000023000011010000,Qanba Obsidian Arcade Joystick PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000009b2800000300000001010000,Raphnet 4nes4snes,a:b0,b:b4,back:b2,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,start:b3,x:b1,y:b5,",
"030000009b2800004200000001010000,Raphnet Dual NES Adapter,a:b0,b:b1,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,start:b3,",
"0300132d9b2800006500000000000000,Raphnet GameCube Adapter,a:b0,b:b7,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,rightx:a3,righty:a4,start:b3,x:b1,y:b8,",
"0300132d9b2800006500000001010000,Raphnet GameCube Adapter,a:b0,b:b7,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,rightx:a3,righty:a4,start:b3,x:b1,y:b8,",
"030000009b2800003200000001010000,Raphnet GC and N64 Adapter,a:b0,b:b7,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,rightx:a3,righty:a4,start:b3,x:b1,y:b8,",
"030000009b2800006000000001010000,Raphnet GC and N64 Adapter,a:b0,b:b7,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b2,righttrigger:b5,rightx:a3,righty:a4,start:b3,x:b1,y:b8,",
"030000009b2800008000000020020000,Raphnet Wii Classic Adapter,a:b1,b:b4,back:b2,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,leftshoulder:b6,rightshoulder:b7,start:b3,x:b0,y:b5,",
"030000009b2800008000000001010000,Raphnet Wii Classic Adapter V3,a:b1,b:b4,back:b2,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,leftshoulder:b6,rightshoulder:b7,start:b3,x:b0,y:b5,",
"03000000f8270000bf0b000011010000,Razer Kishi,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000008916000001fd000024010000,Razer Onza Classic Edition,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b11,dpright:b12,dpup:b13,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000321500000204000011010000,Razer Panthera PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000321500000104000011010000,Razer Panthera PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000321500000810000011010000,Razer Panthera PS4 Evo Arcade Stick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b13,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000321500000010000011010000,Razer Raiju,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000321500000507000000010000,Razer Raiju Mobile,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b21,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000321500000a10000001000000,Razer Raiju Tournament Edition,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b13,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000321500000011000011010000,Razer Raion PS4 Fightpad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"030000008916000000fe000024010000,Razer Sabertooth,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c6240000045d000024010000,Razer Sabertooth,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c6240000045d000025010000,Razer Sabertooth,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000321500000009000011010000,Razer Serval,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a4,rightx:a2,righty:a3,start:b7,x:b2,y:b3,",
"050000003215000000090000163a0000,Razer Serval,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a4,rightx:a2,righty:a3,start:b7,x:b2,y:b3,",
"0300000032150000030a000001010000,Razer Wildcat,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000321500000b10000011010000,Razer Wolverine PS5 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"0300000032150000140a000001010000,Razer Wolverine Ultimate Xbox,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000000d0f0000c100000010010000,Retro Bit Legacy16,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,guide:b12,leftshoulder:b4,lefttrigger:b6,misc1:b13,rightshoulder:b5,righttrigger:b7,start:b9,x:b0,y:b3,",
"030000000d0f0000c100000072056800,Retro Bit Legacy16,a:b1,b:b0,back:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,guide:b5,leftshoulder:b9,lefttrigger:+a4,misc1:b11,rightshoulder:b10,righttrigger:+a5,start:b6,x:b3,y:b2,",
"03000000790000001100000010010000,Retro Controller,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b7,rightshoulder:b4,righttrigger:b5,start:b9,x:b0,y:b3,",
"0300000003040000c197000011010000,Retrode Adapter,a:b0,b:b4,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b1,y:b5,",
"190000004b4800000111000000010000,RetroGame Joypad,a:b1,b:b0,back:b8,dpdown:b14,dpleft:b15,dpright:b16,dpup:b13,leftshoulder:b4,leftstick:b11,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"0300000081170000990a000001010000,Retronic Adapter,a:b0,leftx:a0,lefty:a1,",
"0300000000f000000300000000010000,RetroPad,a:b1,b:b5,back:b2,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,start:b3,x:b0,y:b4,",
"00000000526574726f53746f6e653200,RetroStone 2 Controller,a:b1,b:b0,back:b10,dpdown:b15,dpleft:b16,dpright:b17,dpup:b14,leftshoulder:b6,lefttrigger:b8,rightshoulder:b7,righttrigger:b9,start:b11,x:b4,y:b3,",
"03000000341200000400000000010000,RetroUSB N64 RetroPort,+rightx:b8,+righty:b10,-rightx:b9,-righty:b11,a:b7,b:b6,dpdown:b2,dpleft:b1,dpright:b0,dpup:b3,leftshoulder:b13,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b12,start:b4,",
"030000006b140000010d000011010000,Revolution Pro Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000006b140000130d000011010000,Revolution Pro Controller 3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000006f0e00001f01000000010000,Rock Candy,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00008701000011010000,Rock Candy Nintendo Switch Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000006f0e00001e01000011010000,Rock Candy PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000c6240000fefa000000010000,Rock Candy Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00004601000001010000,Rock Candy Xbox One Controller,a:b0,b:b1,back:b6,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00001311000011010000,Saffun Controller,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b0,",
"03000000a306000023f6000011010000,Saitek Cyborg PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a4,start:b9,x:b0,y:b3,",
"03000000a30600001005000000010000,Saitek P150,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b7,lefttrigger:b6,rightshoulder:b2,righttrigger:b5,x:b3,y:b4,",
"03000000a30600000701000000010000,Saitek P220,a:b2,b:b3,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b7,rightshoulder:b4,righttrigger:b5,x:b0,y:b1,",
"03000000a30600000cff000010010000,Saitek P2500 Force Rumble,a:b2,b:b3,back:b11,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a3,righty:a2,start:b10,x:b0,y:b1,",
"03000000a30600000d5f000010010000,Saitek P2600,a:b1,b:b2,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b11,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,rightx:a3,righty:a2,start:b8,x:b0,y:b3,",
"03000000a30600000c04000011010000,Saitek P2900,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b9,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b12,x:b0,y:b3,",
"03000000a306000018f5000010010000,Saitek P3200 Rumble,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a4,start:b9,x:b0,y:b3,",
"03000000300f00001201000010010000,Saitek P380,a:b2,b:b3,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b9,x:b0,y:b1,",
"03000000a30600000901000000010000,Saitek P880,a:b2,b:b3,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a3,righty:a2,x:b0,y:b1,",
"03000000a30600000b04000000010000,Saitek P990 Dual Analog,a:b1,b:b2,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a2,start:b8,x:b0,y:b3,",
"03000000a306000020f6000011010000,Saitek PS2700 Rumble,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a4,start:b9,x:b0,y:b3,",
"05000000e804000000a000001b010000,Samsung EIGP20,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000d81d00000e00000010010000,Savior,a:b0,b:b1,back:b8,leftshoulder:b6,leftstick:b10,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b2,rightstick:b11,righttrigger:b3,start:b9,x:b4,y:b5,",
"03000000952e00004b43000011010000,Scuf Envision,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b8,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b9,righttrigger:a4,rightx:a2,righty:a5,start:b7,x:b2,y:b3,",
"03000000952e00004d43000011010000,Scuf Envision,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b8,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b9,righttrigger:a4,rightx:a2,righty:a5,start:b7,x:b2,y:b3,",
"03000000952e00004e43000011010000,Scuf Envision,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b8,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b9,righttrigger:a4,rightx:a2,righty:a5,start:b7,x:b2,y:b3,",
"03000000a30c00002500000011010000,Sega Genesis Mini 3B Controller,a:b2,b:b1,dpdown:+a4,dpleft:-a3,dpright:+a3,dpup:-a4,righttrigger:b5,start:b9,",
"03000000790000001100000011010000,Sega Saturn,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b7,rightshoulder:b5,righttrigger:b4,start:b9,x:b0,y:b3,",
"03000000790000002201000011010000,Sega Saturn,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b5,rightshoulder:b6,righttrigger:b7,start:b9,x:b2,y:b3,",
"03000000b40400000a01000000010000,Sega Saturn,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b7,rightshoulder:b5,righttrigger:b2,start:b8,x:b3,y:b4,",
"03000000632500002305000010010000,ShanWan Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000632500002605000010010000,Shanwan Gamepad,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000632500007505000010010000,Shanwan Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000bc2000000055000010010000,Shanwan Gamepad,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000f025000021c1000010010000,Shanwan Gioteck PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000341a00000908000010010000,SL6566,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"030000004b2900000430000011000000,Snakebyte Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"050000004c050000cc09000001000000,Sony DualShock 4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,touchpad:b13,x:b0,y:b3,",
"03000000666600006706000000010000,Sony PlayStation Adapter,a:b2,b:b1,back:b8,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b6,leftstick:b9,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b10,righttrigger:b5,rightx:a2,righty:a3,start:b11,x:b3,y:b0,",
"030000004c050000da0c000011010000,Sony PlayStation Controller,a:b2,b:b1,back:b8,leftshoulder:b6,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,righttrigger:b5,start:b9,x:b3,y:b0,",
"03000000d9040000160f000000010000,Sony PlayStation Controller Adapter,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"03000000ff000000cb01000010010000,Sony PlayStation Portable,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,start:b7,x:b2,y:b3,",
"030000004c0500003713000011010000,Sony PlayStation Vita,a:b1,b:b2,back:b8,dpdown:b13,dpleft:b15,dpright:b14,dpup:b12,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,rightx:a3,righty:a4,start:b9,x:b0,y:b3,",
"03000000250900000500000000010000,Sony PS2 pad with SmartJoy Adapter,a:b2,b:b1,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"030000005e0400008e02000073050000,Speedlink Torid,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400008e02000020200000,SpeedLink Xeox Pro Analog,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000de2800000112000001000000,Steam Controller,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,paddle1:b11,paddle2:b10,rightshoulder:b5,righttrigger:a3,start:b7,x:b2,y:b3,",
"03000000de2800000112000011010000,Steam Controller,a:b2,b:b3,back:b10,dpdown:+a5,dpleft:-a4,dpright:+a4,dpup:-a5,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a7,leftx:a0,lefty:a1,paddle1:b15,paddle2:b16,rightshoulder:b7,rightstick:b14,righttrigger:a6,rightx:a2,righty:a3,start:b11,x:b4,y:b5,",
"03000000de2800000211000001000000,Steam Controller,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,paddle1:b11,paddle2:b10,rightshoulder:b5,righttrigger:a3,start:b7,x:b2,y:b3,",
"03000000de2800000211000011010000,Steam Controller,a:b2,b:b3,back:b10,dpdown:b18,dpleft:b19,dpright:b20,dpup:b17,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,paddle1:b16,paddle2:b15,rightshoulder:b7,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b4,y:b5,",
"03000000de2800004211000001000000,Steam Controller,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,paddle1:b11,paddle2:b10,rightshoulder:b5,righttrigger:a3,start:b7,x:b2,y:b3,",
"03000000de2800004211000011010000,Steam Controller,a:b2,b:b3,back:b10,dpdown:b18,dpleft:b19,dpright:b20,dpup:b17,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a7,leftx:a0,lefty:a1,paddle1:b16,paddle2:b15,rightshoulder:b7,righttrigger:a6,rightx:a2,righty:a3,start:b11,x:b4,y:b5,",
"03000000de280000fc11000001000000,Steam Controller,a:b0,b:b1,back:b6,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"05000000de2800000212000001000000,Steam Controller,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,paddle1:b11,paddle2:b10,rightshoulder:b5,righttrigger:a3,start:b7,x:b2,y:b3,",
"05000000de2800000511000001000000,Steam Controller,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,paddle1:b11,paddle2:b10,rightshoulder:b5,righttrigger:a3,start:b7,x:b2,y:b3,",
"05000000de2800000611000001000000,Steam Controller,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,paddle1:b11,paddle2:b10,rightshoulder:b5,righttrigger:a3,start:b7,x:b2,y:b3,",
"03000000de2800000512000010010000,Steam Deck,a:b3,b:b4,back:b11,dpdown:b17,dpleft:b18,dpright:b19,dpup:b16,guide:b13,leftshoulder:b7,leftstick:b14,lefttrigger:a9,leftx:a0,lefty:a1,rightshoulder:b8,rightstick:b15,righttrigger:a8,rightx:a2,righty:a3,start:b12,x:b5,y:b6,",
"03000000de2800000512000011010000,Steam Deck,a:b3,b:b4,back:b11,dpdown:b17,dpleft:b18,dpright:b19,dpup:b16,guide:b13,leftshoulder:b7,leftstick:b14,lefttrigger:a9,leftx:a0,lefty:a1,misc1:b2,paddle1:b21,paddle2:b20,paddle3:b23,paddle4:b22,rightshoulder:b8,rightstick:b15,righttrigger:a8,rightx:a2,righty:a3,start:b12,x:b5,y:b6,",
"03000000de280000ff11000001000000,Steam Virtual Gamepad,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"050000004e696d6275732b0000000000,SteelSeries Nimbus Plus,a:b0,b:b1,back:b10,guide:b11,leftshoulder:b4,leftstick:b8,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:b7,rightx:a2,righty:a3,start:b12,x:b2,y:b3,",
"03000000381000003014000075010000,SteelSeries Stratus Duo,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000381000003114000075010000,SteelSeries Stratus Duo,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"0500000011010000311400001b010000,SteelSeries Stratus Duo,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b32,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"05000000110100001914000009010000,SteelSeries Stratus XL,a:b0,b:b1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000ad1b000038f0000090040000,Street Fighter IV Fightstick TE,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000003b07000004a1000000010000,Suncom SFX Plus,a:b0,b:b2,back:b7,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b4,rightshoulder:b9,righttrigger:b5,start:b8,x:b1,y:b3,",
"030000001f08000001e4000010010000,Super Famicom Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b9,x:b3,y:b0,",
"03000000666600000488000000010000,Super Joy Box 5 Pro,a:b2,b:b1,back:b9,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b3,y:b0,",
"0300000000f00000f100000000010000,Super RetroPort,a:b1,b:b5,back:b2,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b3,x:b0,y:b4,",
"030000008f0e00000d31000010010000,SZMY Power 3 Turbo,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000457500000401000011010000,SZMY Power DS4 Wired Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,misc1:b13,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000457500002211000010010000,SZMY Power Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"030000008f0e00001431000010010000,SZMY Power PS3,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000e40a00000307000011010000,Taito Egret II Mini Control Panel,a:b4,b:b2,back:b6,guide:b9,leftx:a0,lefty:a1,rightshoulder:b0,righttrigger:b1,start:b7,x:b8,y:b3,",
"03000000e40a00000207000011010000,Taito Egret II Mini Controller,a:b4,b:b2,back:b6,guide:b9,leftx:a0,lefty:a1,rightshoulder:b0,righttrigger:b1,start:b7,x:b8,y:b3,",
"03000000ba2200000701000001010000,Technology Innovation PS2 Adapter,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a5,righty:a2,start:b9,x:b3,y:b2,",
"03000000790000001c18000011010000,TGZ Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b15,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:b9,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000591c00002400000010010000,THEC64 Joystick,a:b0,b:b1,back:b6,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,start:b7,x:b2,y:b3,",
"03000000591c00002600000010010000,THEGamepad,a:b2,b:b1,back:b6,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,start:b7,x:b3,y:b0,",
"030000004f04000015b3000001010000,Thrustmaster Dual Analog 3.2,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"030000004f04000015b3000010010000,Thrustmaster Dual Analog 4,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"030000004f04000020b3000010010000,Thrustmaster Dual Trigger,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"030000004f04000023b3000000010000,Thrustmaster Dual Trigger PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000004f0400000ed0000011010000,Thrustmaster eSwap Pro Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000b50700000399000000010000,Thrustmaster Firestorm Digital 2,a:b2,b:b4,back:b11,leftshoulder:b6,leftstick:b10,lefttrigger:b7,leftx:a0,lefty:a1,rightshoulder:b8,rightstick:b0,righttrigger:b9,start:b1,x:b3,y:b5,",
"030000004f04000003b3000010010000,Thrustmaster Firestorm Dual Analog 2,a:b0,b:b2,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b9,rightx:a2,righty:a3,x:b1,y:b3,",
"030000004f04000000b3000010010000,Thrustmaster Firestorm Dual Power,a:b0,b:b2,back:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b11,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b12,righttrigger:b7,rightx:a2,righty:a3,start:b10,x:b1,y:b3,",
"030000004f04000004b3000010010000,Thrustmaster Firestorm Dual Power,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"030000004f04000026b3000002040000,Thrustmaster GP XID,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c6240000025b000002020000,Thrustmaster GPX,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000004f04000008d0000000010000,Thrustmaster Run N Drive PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000004f04000009d0000000010000,Thrustmaster Run N Drive PlayStation Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000004f04000007d0000000010000,Thrustmaster T Mini,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"030000004f04000012b3000010010000,Thrustmaster Vibrating Gamepad,a:b0,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b6,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b1,y:b3,",
"03000000571d00002000000010010000,Tomee SNES Adapter,a:b0,b:b1,back:b6,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b7,x:b2,y:b3,",
"03000000bd12000015d0000010010000,Tomee SNES Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,start:b9,x:b3,y:b0,",
"03000000d814000007cd000011010000,Toodles 2008 Chimp PC PS3,a:b0,b:b1,back:b8,leftshoulder:b4,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b7,start:b9,x:b3,y:b2,",
"030000005e0400008e02000070050000,Torid,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000c01100000591000011010000,Torid,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"03000000680a00000300000003000000,TRBot Virtual Joypad,a:b11,b:b12,back:b15,dpdown:b6,dpleft:b3,dpright:b4,dpup:b5,leftshoulder:b17,leftstick:b21,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b22,righttrigger:a2,rightx:a3,righty:a4,start:b16,x:b13,y:b14,",
"03000000780300000300000003000000,TRBot Virtual Joypad,a:b11,b:b12,back:b15,dpdown:b6,dpleft:b3,dpright:b4,dpup:b5,leftshoulder:b17,leftstick:b21,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b22,righttrigger:a2,rightx:a3,righty:a4,start:b16,x:b13,y:b14,",
"03000000e00d00000300000003000000,TRBot Virtual Joypad,a:b11,b:b12,back:b15,dpdown:b6,dpleft:b3,dpright:b4,dpup:b5,leftshoulder:b17,leftstick:b21,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b22,righttrigger:a2,rightx:a3,righty:a4,start:b16,x:b13,y:b14,",
"03000000f00600000300000003000000,TRBot Virtual Joypad,a:b11,b:b12,back:b15,dpdown:b6,dpleft:b3,dpright:b4,dpup:b5,leftshoulder:b17,leftstick:b21,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b22,righttrigger:a2,rightx:a3,righty:a4,start:b16,x:b13,y:b14,",
"030000005f140000c501000010010000,Trust Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b0,",
"06000000f51000000870000003010000,Turtle Beach Recon,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000100800000100000010010000,Twin PS2 Adapter,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"03000000151900005678000010010000,Uniplay U6,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000100800000300000010010000,USB Gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b9,x:b3,y:b0,",
"03000000790000000600000007010000,USB gamepad,a:b2,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a3,righty:a4,start:b9,x:b3,y:b0,",
"03000000790000001100000000010000,USB Gamepad,a:b2,b:b1,back:b8,dpdown:a0,dpleft:a1,dpright:a2,dpup:a4,start:b9,",
"03000000790000001a18000011010000,Venom PS4 Arcade Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,",
"03000000790000001b18000011010000,Venom PS4 Arcade Joystick,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"030000006f0e00000302000011010000,Victrix Pro Fightstick PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,touchpad:b13,x:b0,y:b3,",
"030000006f0e00000702000011010000,Victrix Pro Fightstick PS4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,start:b9,touchpad:b13,x:b0,y:b3,",
"05000000ac0500003232000001000000,VR Box Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a3,righty:a2,start:b9,x:b2,y:b3,",
"05000000434f4d4d414e440000000000,VX Gaming Command Series,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"0000000058626f782033363020576900,Xbox 360 Controller,a:b0,b:b1,back:b14,dpdown:b11,dpleft:b12,dpright:b13,dpup:b10,guide:b7,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"030000005e0400001907000000010000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b11,dpright:b12,dpup:b13,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400008e02000010010000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400008e02000014010000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400009102000007010000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b11,dpright:b12,dpup:b13,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000a102000000010000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b11,dpright:b12,dpup:b13,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000a102000007010000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000a102000030060000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00001503000000020000,Xbox 360 Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e0400008e02000000010000,Xbox 360 EasySMX,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000a102000014010000,Xbox 360 Receiver,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b11,dpright:b12,dpup:b13,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"0000000058626f782047616d65706100,Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a4,rightx:a2,righty:a3,start:b7,x:b2,y:b3,",
"030000005e0400000202000000010000,Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b2,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b3,y:b4,",
"030000005e0400008e02000072050000,Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000006f0e00001304000000010000,Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000ffff0000ffff000000010000,Xbox Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b5,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b2,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b3,y:b4,",
"030000005e0400000a0b000005040000,Xbox One Controller,a:b1,b:b0,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b11,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b12,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b3,y:b2,",
"030000005e040000d102000002010000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000ea02000000000000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000ea02000001030000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"050000005e040000e002000003090000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b10,leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"050000005e040000fd02000003090000,Xbox One Controller,a:b0,b:b1,back:b15,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b16,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000fd02000030110000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"060000005e040000dd02000003020000,Xbox One Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"050000005e040000e302000002090000,Xbox One Elite,a:b0,b:b1,back:b136,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a6,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a5,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000220b000013050000,Xbox One Elite 2 Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000050b000002090000,Xbox One Elite Series 2,a:b0,b:b1,back:b136,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftstick:b13,lefttrigger:a6,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a5,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"030000005e040000ea02000011050000,Xbox One S Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
   "030000005e040000ea02000015050000,Xbox One S Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"060000005e040000ea0200000b050000,Xbox One S Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"060000005e040000ea0200000d050000,Xbox One S Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"060000005e040000ea02000016050000,Xbox One S Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b000001050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b000005050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b000007050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b000009050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b00000d050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b00000f050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b000011050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b000014050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000120b000015050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"030000005e040000130b000005050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000130b000001050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000130b000005050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000130b000007050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000130b000009050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000130b000011050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000130b000013050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000130b000015050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000130b000017050000,Xbox Series Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,misc1:b15,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"060000005e040000120b000007050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"060000005e040000120b00000b050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"060000005e040000120b00000d050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"060000005e040000120b00000f050000,Xbox Series Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,misc1:b11,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"050000005e040000200b000013050000,Xbox Wireless Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000200b000017050000,Xbox Wireless Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"050000005e040000220b000017050000,Xbox Wireless Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4,",
"03000000450c00002043000010010000,XEOX SL6556 BK,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,",
"05000000172700004431000029010000,XiaoMi Controller,a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b20,leftshoulder:b6,leftstick:b13,lefttrigger:a7,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a6,rightx:a2,righty:a5,start:b11,x:b3,y:b4,",
"03000000c0160000e105000001010000,XinMo Dual Arcade,a:b4,b:b3,back:b6,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b9,leftshoulder:b2,leftx:a0,lefty:a1,rightshoulder:b5,start:b7,x:b1,y:b0,",
"030000005e0400008e02000020010000,XInput Adapter,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"xinput,XInput Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
"03000000120c0000100e000011010000,Zeroplus P4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,",
"03000000120c0000101e000011010000,Zeroplus P4,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,"
#elif defined(__ANDROID_API__)
"38653964633230666463343334313533,8BitDo Adapter,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"36666264316630653965636634386234,8BitDo Adapter 2,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b19,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"38426974446f20417263616465205374,8BitDo Arcade Stick,a:b0,b:b1,back:b15,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,guide:b5,leftshoulder:b9,lefttrigger:a4,rightshoulder:b10,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"61393962646434393836356631636132,8BitDo Arcade Stick,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b20,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b19,y:b2,",
"64323139346131306233636562663738,8BitDo Arcade Stick,a:b0,b:b1,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b19,y:b2,",
"64643565386136613265663236636564,8BitDo Arcade Stick,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b20,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b19,y:b2,",
"33313433353539306634656436353432,8BitDo Dogbone,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"38426974446f20446f67626f6e65204d,8BitDo Dogbone,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,start:b6,",
"34343439373236623466343934376233,8BitDo FC30 Pro,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b28,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b4,rightstick:b29,righttrigger:b7,start:b5,x:b30,y:b2,",
"38426974446f204e4743204d6f646b69,8BitDo GameCube,a:b0,b:b2,back:b4,dpdown:b12,dpleft:b13,dpright:b14,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,paddle1:b18,paddle2:b17,rightshoulder:b15,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b1,y:b3,",
"38426974446f2038426974446f204c69,8BitDo Lite,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"30643332373663313263316637356631,8BitDo Lite 2,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38426974446f204c6974652032000000,8BitDo Lite 2,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"62656331626461363634633735353032,8BitDo Lite 2,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38393936616436383062666232653338,8BitDo Lite SE,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38426974446f204c6974652053450000,8BitDo Lite SE,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"39356430616562366466646636643435,8BitDo Lite SE,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"05000000c82d000006500000ffff3f00,8BitDo M30,a:b1,b:b0,back:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,guide:b17,leftshoulder:b9,lefttrigger:a5,rightshoulder:b10,righttrigger:a4,start:b6,x:b3,y:b2,",
"05000000c82d000051060000ffff3f00,8BitDo M30,a:b1,b:b0,back:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,guide:b17,leftshoulder:b9,lefttrigger:a4,rightshoulder:b10,righttrigger:a5,start:b6,x:b3,y:b2,",
"32323161363037623637326438643634,8BitDo M30,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftx:a0,lefty:a1,rightshoulder:b9,righttrigger:b10,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"33656266353630643966653238646264,8BitDo M30,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b20,righttrigger:a5,start:b10,x:b19,y:b2,",
"38426974446f204d3330204d6f646b69,8BitDo M30,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftx:a0,lefty:a1,rightshoulder:b9,righttrigger:b10,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"39366630663062373237616566353437,8BitDo M30,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,righttrigger:b18,start:b6,x:b2,y:b3,",
"64653533313537373934323436343563,8BitDo M30,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:a4,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b9,righttrigger:b10,start:b6,x:b2,y:b3,",
"66356438346136366337386437653934,8BitDo M30,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,righttrigger:b10,start:b18,x:b19,y:b2,",
"66393064393162303732356665666366,8BitDo M30,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,righttrigger:a5,start:b6,x:b2,y:b3,",
"38426974446f204d6963726f2067616d,8BitDo Micro,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,lefttrigger:a4,leftx:b0,lefty:b1,rightshoulder:b10,righttrigger:a5,rightx:b2,righty:b3,start:b6,x:b3,y:b2,",
"61653365323561356263373333643266,8BitDo Micro,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,lefttrigger:a4,leftx:b0,lefty:b1,rightshoulder:b10,righttrigger:a5,rightx:b2,righty:b3,start:b6,x:b3,y:b2,",
"62613137616239666338343866326336,8BitDo Micro,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,lefttrigger:a4,leftx:b0,lefty:b1,rightshoulder:b10,righttrigger:a5,rightx:b2,righty:b3,start:b6,x:b3,y:b2,",
"33663431326134333366393233616633,8BitDo N30,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,start:b6,",
"38426974446f204e3330204d6f646b69,8BitDo N30,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,start:b6,",
"05000000c82d000015900000ffff3f00,8BitDo N30 Pro 2,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"05000000c82d000065280000ffff3f00,8BitDo N30 Pro 2,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b17,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38323035343766666239373834336637,8BitDo N64,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,righttrigger:b18,rightx:a2,righty:a3,start:b6,",
"38426974446f204e3634204d6f646b69,8BitDo N64,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,righttrigger:b18,rightx:a2,righty:a3,start:b6,",
"32363135613966656338666638666237,8BitDo NEOGEO,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftx:a0,lefty:a1,rightshoulder:b10,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"35363534633333373639386466346631,8BitDo NEOGEO,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftx:a0,lefty:a1,rightshoulder:b10,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"38426974446f204e454f47454f204750,8BitDo NEOGEO,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftx:a0,lefty:a1,rightshoulder:b10,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"39383963623932353561633733306334,8BitDo NEOGEO,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftx:a0,lefty:a1,rightshoulder:b10,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000000220000000900000ffff3f00,8BitDo NES30 Pro,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"050000002038000009000000ffff3f00,8BitDo NES30 Pro,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38313433643131656262306631373166,8BitDo P30,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"38326536643339353865323063616339,8BitDo P30,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"38426974446f2050333020636c617373,8BitDo P30,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"35376664343164386333616535333434,8BitDo Pro 2,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b17,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b18,righttrigger:a5,rightx:a3,start:b10,x:b19,y:b2,",
"38426974446f2038426974446f205072,8BitDo Pro 2,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38426974446f2050726f203200000000,8BitDo Pro 2,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"61333362366131643730353063616330,8BitDo Pro 2,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"62373739366537363166326238653463,8BitDo Pro 2,a:b1,b:b0,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,x:b3,y:b2,",
"38386464613034326435626130396565,8BitDo Receiver,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b19,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38426974446f2038426974446f205265,8BitDo Receiver,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b19,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"66303230343038613365623964393766,8BitDo Receiver,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b19,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38426974446f20533330204d6f646b69,8BitDo S30,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:a4,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b9,righttrigger:b10,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"66316462353561376330346462316137,8BitDo S30,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:a4,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b9,righttrigger:b10,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"05000000c82d000000600000ffff3f00,8BitDo SF30 Pro,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b15,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b16,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"05000000c82d000000610000ffff3f00,8BitDo SF30 Pro,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38426974646f20534633302050726f00,8BitDo SF30 Pro,a:b1,b:b0,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b16,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b2,y:b17,",
"61623334636338643233383735326439,8BitDo SFC30,a:b0,b:b1,back:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b3,rightshoulder:b31,start:b5,x:b30,y:b2,",
"05000000c82d000012900000ffff3f00,8BitDo SN30,a:b1,b:b0,back:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,rightshoulder:b10,start:b6,x:b3,y:b2,",
"05000000c82d000062280000ffff3f00,8BitDo SN30,a:b1,b:b0,back:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,rightshoulder:b10,start:b6,x:b3,y:b2,",
"38316230613931613964356666353839,8BitDo SN30,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftx:a0,lefty:a1,rightshoulder:b10,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38426974446f20534e3330204d6f646b,8BitDo SN30,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftx:a0,lefty:a1,rightshoulder:b10,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"65323563303231646531383162646335,8BitDo SN30,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftx:a0,lefty:a1,rightshoulder:b10,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"35383531346263653330306238353131,8BitDo SN30 PP,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"05000000c82d000001600000ffff3f00,8BitDo SN30 Pro,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"05000000c82d000002600000ffff0f00,8BitDo SN30 Pro Plus,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b17,leftshoulder:b9,leftstick:b7,lefttrigger:b15,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b16,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"36653638656632326235346264663661,8BitDo SN30 Pro Plus,a:b0,b:b1,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b19,y:b2,",
"38303232393133383836366330346462,8BitDo SN30 Pro Plus,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b17,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b18,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b19,y:b2,",
"38346630346135363335366265656666,8BitDo SN30 Pro Plus,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38426974446f20534e33302050726f2b,8BitDo SN30 Pro Plus,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b19,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"536f6e7920436f6d707574657220456e,8BitDo SN30 Pro Plus,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"66306331643531333230306437353936,8BitDo SN30 Pro Plus,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"050000002028000009000000ffff3f00,8BitDo SNES30,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"050000003512000020ab000000780f00,8BitDo SNES30,a:b21,b:b20,back:b30,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b26,rightshoulder:b27,start:b31,x:b24,y:b23,",
"33666663316164653937326237613331,8BitDo Zero,a:b0,b:b1,back:b15,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,rightshoulder:b10,start:b6,x:b2,y:b3,",
"38426974646f205a65726f2047616d65,8BitDo Zero,a:b0,b:b1,back:b15,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,rightshoulder:b10,start:b6,x:b2,y:b3,",
"05000000c82d000018900000ffff0f00,8BitDo Zero 2,a:b1,b:b0,back:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,rightshoulder:b10,start:b6,x:b3,y:b2,",
"05000000c82d000030320000ffff0f00,8BitDo Zero 2,a:b1,b:b0,back:b4,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,rightshoulder:b10,start:b6,x:b3,y:b2,",
"33663434393362303033616630346337,8BitDo Zero 2,a:b0,b:b1,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftx:a0,lefty:a1,rightshoulder:b20,start:b18,x:b19,y:b2,",
"34656330626361666438323266633963,8BitDo Zero 2,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b20,start:b10,x:b19,y:b2,",
"63396666386564393334393236386630,8BitDo Zero 2,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftx:a0,lefty:a1,rightshoulder:b10,start:b6,x:b3,y:b2,",
"63633435623263373466343461646430,8BitDo Zero 2,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftx:a0,lefty:a1,rightshoulder:b10,start:b6,x:b2,y:b3,",
"32333634613735616163326165323731,Amazon Luna Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,x:b2,y:b3,",
"4c696e757820342e31392e3137322077,Anbernic Handheld,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a4,start:b6,x:b2,y:b3,",
"417374726f2063697479206d696e6920,Astro City Mini,a:b23,b:b22,back:b29,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,rightshoulder:b25,righttrigger:b26,start:b30,x:b24,y:b21,",
"35643263313264386134376362363435,Atari VCS Classic Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,start:b6,",
"32353831643566306563643065356239,Atari VCS Modern Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"4f64696e20436f6e74726f6c6c657200,AYN Odin,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b14,dpright:b13,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:+a5,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:+a4,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"32303165626138343962363666346165,Brook Mars PS4 Controller,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b0,y:b2,",
"38383337343564366131323064613561,Brook Mars PS4 Controller,a:b1,b:b19,back:b17,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b0,y:b2,",
"34313430343161653665353737323365,Elecom JC-W01U,a:b23,b:b24,back:b29,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b25,lefttrigger:b27,leftx:a0,lefty:a1,rightshoulder:b26,righttrigger:b28,rightx:a2,righty:a3,start:b30,x:b21,y:b22,",
"4875694a6961204a432d573031550000,Elecom JC-W01U,a:b23,b:b24,back:b29,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b25,lefttrigger:b27,leftx:a0,lefty:a1,rightshoulder:b26,righttrigger:b28,rightx:a2,righty:a3,start:b30,x:b21,y:b22,",
"30363230653635633863366338623265,Evo VR,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftx:a0,lefty:a1,x:b2,y:b3,",
"05000000b404000011240000dfff3f00,Flydigi Vader 2,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,paddle1:b17,paddle2:b18,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"05000000bc20000000550000ffff3f00,GameSir G3w,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"34323662653333636330306631326233,Google Nexus,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"35383633353935396534393230616564,Google Stadia Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"476f6f676c65204c4c43205374616469,Google Stadia Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"5374616469614e3848532d6532633400,Google Stadia Controller,a:b0,b:b1,back:b15,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"05000000d6020000e5890000dfff3f00,GPD XD Plus,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a4,rightx:a2,righty:a5,start:b6,x:b2,y:b3,",
"05000000d6020000e5890000dfff3f80,GPD XD Plus,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a3,rightx:a4,righty:a5,start:b6,x:b2,y:b3,",
"66633030656131663837396562323935,Hori Battle,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"35623466343433653739346434636330,Hori Fighting Commander 3 Pro,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b0,y:b2,",
"484f524920434f2e2c4c54442e203130,Hori Fighting Commander 3 Pro,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b20,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b3,righttrigger:b9,rightx:a2,righty:a3,start:b18,x:b0,y:b2,",
"484f524920434f2e2c4c544420205041,Hori Gem Pad 3,a:b1,b:b17,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b4,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b16,x:b0,y:b2,",
"65656436646661313232656661616130,Hori PC Engine Mini Controller,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,start:b18,",
"31303433326562636431653534636633,Hori Real Arcade Pro 3,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b0,y:b2,",
"32656664353964393561366362333636,Hori Switch Split Pad Pro,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"30306539356238653637313730656134,HORIPAD Switch Pro Controller,a:b0,b:b1,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b19,y:b2,",
"48797065726b696e2050616400000000,Hyperkin Admiral N64 Controller,+rightx:b6,+righty:b7,-rightx:b17,-righty:b5,a:b1,b:b0,leftshoulder:b3,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b20,start:b18,",
"62333331353131353034386136626636,Hyperkin Admiral N64 Controller,+rightx:b6,+righty:b7,-rightx:b17,-righty:b5,a:b1,b:b0,leftshoulder:b3,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b20,start:b18,",
"31306635363562663834633739396333,Hyperkin N64 Adapter,a:b1,b:b19,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightx:a2,righty:a3,start:b18,",
"5368616e57616e202020202048797065,Hyperkin N64 Adapter,a:b1,b:b19,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightx:a2,righty:a3,start:b18,",
"0500000083050000602000000ffe0000,iBuffalo SNES Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b15,rightshoulder:b16,start:b10,x:b2,y:b3,",
"5553422c322d6178697320382d627574,iBuffalo Super Famicom Controller,a:b1,b:b0,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b17,rightshoulder:b18,start:b10,x:b3,y:b2,",
"64306137363261396266353433303531,InterAct GoPad,a:b24,b:b25,leftshoulder:b23,lefttrigger:b27,leftx:a0,lefty:a1,rightshoulder:b26,righttrigger:b28,x:b21,y:b22,",
"532e542e442e20496e74657261637420,InterAct HammerHead FX,a:b23,b:b24,back:b30,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b26,leftstick:b22,lefttrigger:b28,leftx:a0,lefty:a1,rightshoulder:b27,rightstick:b25,righttrigger:b29,rightx:a2,righty:a3,start:b31,x:b20,y:b21,",
"65346535636333663931613264643164,Joy-Con,a:b21,b:b22,back:b29,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b25,lefttrigger:b27,leftx:a0,lefty:a1,rightshoulder:b26,righttrigger:b28,rightx:a2,righty:a3,start:b30,x:b23,y:b24,",
"33346566643039343630376565326335,Joy-Con (L),a:b0,b:b1,back:b7,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,rightshoulder:b20,start:b17,x:b19,y:b2,",
"35313531613435623366313835326238,Joy-Con (L),a:b0,b:b1,back:b7,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,rightshoulder:b20,start:b17,x:b19,y:b2,",
"4a6f792d436f6e20284c290000000000,Joy-Con (L),a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,rightshoulder:b20,start:b17,x:b19,y:b2,",
"38383665633039363066383334653465,Joy-Con (R),a:b0,b:b1,back:b5,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,rightshoulder:b20,start:b18,x:b19,y:b2,",
"39363561613936303237333537383931,Joy-Con (R),a:b0,b:b1,back:b5,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,rightshoulder:b20,start:b18,x:b19,y:b2,",
"39373064396565646338333134303131,Joy-Con (R),a:b1,b:b2,back:b5,leftstick:b8,leftx:a1~,lefty:a0,start:b6,x:b0,y:b3,",
"4a6f792d436f6e202852290000000000,Joy-Con (R),a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,rightshoulder:b20,start:b18,x:b19,y:b2,",
"39656136363638323036303865326464,JYS Aapter,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b0,y:b2,",
"63316564383539663166353034616434,JYS Adapter,a:b1,b:b3,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b0,y:b2,",
"64623163333561643339623235373232,Logitech F310,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"35623364393661626231343866613337,Logitech F710,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"4c6f6769746563682047616d65706164,Logitech F710,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"64396331333230326333313330336533,Logitech F710,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"39653365373864633935383236363438,Logitech G Cloud,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"416d617a6f6e2047616d6520436f6e74,Luna Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"4c756e612047616d6570616400000000,Luna Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"30363066623539323534363639323363,Magic NS,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b0,y:b2,",
"31353762393935386662336365626334,Magic NS,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b0,y:b2,",
"39623565346366623931666633323530,Magic NS,a:b1,b:b3,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b0,y:b2,",
"6d6179666c617368206c696d69746564,Mayflash GameCube Adapter,a:b22,b:b21,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,lefttrigger:b25,leftx:a0,lefty:a1,rightshoulder:b28,righttrigger:b26,rightx:a5,righty:a2,start:b30,x:b23,y:b24,",
"436f6e74726f6c6c6572000000000000,Mayflash N64 Adapter,a:b1,b:b19,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightx:a2,righty:a3,start:b18,",
"65666330633838383061313633326461,Mayflash N64 Adapter,a:b1,b:b19,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightx:a2,righty:a3,start:b18,",
"37316565396364386635383230353365,Mayflash Saturn Adapter,a:b21,b:b22,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b26,lefttrigger:b28,rightshoulder:b27,righttrigger:b23,start:b30,x:b24,y:b25,",
"4875694a696120205553422047616d65,Mayflash Saturn Adapter,a:b21,b:b22,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b26,lefttrigger:b28,rightshoulder:b27,righttrigger:b23,start:b30,x:b24,y:b25,",
"535a4d792d706f776572204c54442043,Mayflash Wii Classic Adapter,a:b23,b:b22,back:b29,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b31,leftshoulder:b27,lefttrigger:b25,leftx:a0,lefty:a1,rightshoulder:b28,righttrigger:b26,rightx:a2,righty:a3,start:b30,x:b24,y:b21,",
"30653962643666303631376438373532,Mayflash Wii DolphinBar,a:b23,b:b24,back:b29,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b0,leftshoulder:b25,lefttrigger:b27,leftx:a0,lefty:a1,rightshoulder:b26,righttrigger:b28,rightx:a2,righty:a3,start:b30,x:b21,y:b22,",
"39346131396233376535393665363161,Mayflash Wii U Pro Adapter,a:b22,b:b23,back:b29,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b25,leftstick:b31,lefttrigger:b27,rightshoulder:b26,rightstick:b0,righttrigger:b28,rightx:a0,righty:a1,start:b30,x:b21,y:b24,",
"31323564663862633234646330373138,Mega Drive,a:b23,b:b22,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,rightshoulder:b25,righttrigger:b26,start:b30,x:b24,y:b21,",
"37333564393261653735306132613061,Mega Drive,a:b21,b:b22,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b26,lefttrigger:b28,rightshoulder:b27,righttrigger:b23,start:b30,x:b24,y:b25,",
"64363363336633363736393038313464,Mega Drive,a:b1,b:b0,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,start:b9,x:b2,y:b3,",
"33323763323132376537376266393366,Microsoft Dual Strike,a:b24,b:b23,back:b25,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b27,lefttrigger:b29,rightshoulder:b78,rightx:a0,righty:a1~,start:b26,x:b22,y:b21,",
"30306461613834333439303734316539,Microsoft SideWinder Pro,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b20,lefttrigger:b9,rightshoulder:b19,righttrigger:b10,start:b17,x:b2,y:b3,",
"32386235353630393033393135613831,Microsoft Xbox Series Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"4d4f42415041442050726f2d48440000,Mobapad Chitu HD,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"4d4f435554452d303533582d4d35312d,Mocute 053X,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"33343361376163623438613466616531,Mocute M053,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"39306635663061636563316166303966,Mocute M053,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"7573622067616d657061642020202020,NEXT SNES Controller,a:b2,b:b1,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b4,rightshoulder:b5,righttrigger:b6,start:b9,x:b3,y:b0,",
"050000007e05000009200000ffff0f00,Nintendo Switch Pro Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,leftstick:b4,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b16,x:b17,y:b2,",
"31316661666466633938376335383661,Nintendo Switch Pro Controller,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,misc1:b5,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,start:b6,x:b3,y:b2,",
"34323437396534643531326161633738,Nintendo Switch Pro Controller,a:b0,b:b1,back:b15,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,leftstick:b7,lefttrigger:b17,misc1:b5,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"50726f20436f6e74726f6c6c65720000,Nintendo Switch Pro Controller,a:b0,b:b1,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b2,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:b10,rightx:a2,righty:a3,start:b18,y:b3,",
"36326533353166323965623661303933,NSO N64 Controller,+rightx:b17,+righty:b10,-rightx:b2,-righty:b19,a:b1,b:b0,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,lefttrigger:b9,leftx:a0,lefty:a1,misc1:b7,rightshoulder:b20,righttrigger:b15,start:b18,",
"4e363420436f6e74726f6c6c65720000,NSO N64 Controller,+rightx:b17,+righty:b10,-rightx:b2,-righty:b19,a:b1,b:b0,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,lefttrigger:b9,leftx:a0,lefty:a1,misc1:b7,rightshoulder:b20,righttrigger:b15,start:b18,",
"534e455320436f6e74726f6c6c657200,NSO SNES Controller,a:b0,b:b1,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,rightshoulder:b20,start:b18,x:b19,y:b2,",
"64623863346133633561626136366634,NSO SNES Controller,a:b0,b:b1,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,rightshoulder:b20,start:b18,x:b19,y:b2,",
"050000005509000003720000cf7f3f00,NVIDIA Controller,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000005509000010720000ffff3f00,NVIDIA Controller,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000005509000014720000df7f3f00,NVIDIA Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a4,rightx:a2,righty:a5,start:b6,x:b2,y:b3,",
"050000005509000014720000df7f3f80,NVIDIA Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a3,rightx:a4,righty:a5,start:b6,x:b2,y:b3,",
"37336435666338653565313731303834,NVIDIA Controller,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"4e564944494120436f72706f72617469,NVIDIA Controller,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"61363931656135336130663561616264,NVIDIA Controller,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"39383335313438623439373538343266,OUYA Controller,a:b0,b:b2,dpdown:b18,dpleft:b15,dpright:b16,dpup:b17,leftshoulder:b3,leftstick:b9,lefttrigger:b5,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b10,righttrigger:b7,rightx:a3,righty:a4,x:b1,y:b19,",
"4f5559412047616d6520436f6e74726f,OUYA Controller,a:b0,b:b2,dpdown:b18,dpleft:b15,dpright:b6,dpup:b17,leftshoulder:b3,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,x:b1,y:b19,",
"506572666f726d616e63652044657369,PDP PS3 Rock Candy Controller,a:b1,b:b17,back:h0.2,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b4,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b16,x:b0,y:b2,",
"61653962353232366130326530363061,Pokken,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,lefttrigger:b9,rightshoulder:b20,righttrigger:b10,start:b18,x:b0,y:b2,",
"32666633663735353234363064386132,PS2,a:b23,b:b22,back:b29,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b27,lefttrigger:b25,leftx:a0,lefty:a1,rightshoulder:b28,righttrigger:b26,rightx:a3,righty:a2,start:b30,x:b24,y:b21,",
"050000004c05000068020000dfff3f00,PS3 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"536f6e7920504c415953544154494f4e,PS3 Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"61363034663839376638653463633865,PS3 Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"66366539656564653432353139356536,PS3 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"66383132326164626636313737373037,PS3 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000004c050000c405000000783f00,PS4 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000004c050000c4050000fffe3f00,PS4 Controller,a:b1,b:b17,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,leftstick:b4,lefttrigger:+a3,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b6,righttrigger:+a4,rightx:a2,righty:a5,start:b16,x:b0,y:b2,",
"050000004c050000c4050000fffe3f80,PS4 Controller,a:b1,b:b17,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,leftstick:b4,lefttrigger:+a2,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b6,righttrigger:+a3,rightx:a4,righty:a5,start:b16,x:b0,y:b2,",
"050000004c050000c4050000ffff3f00,PS4 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000004c050000cc090000fffe3f00,PS4 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000004c050000cc090000ffff3f00,PS4 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"30303839663330346632363232623138,PS4 Controller,a:b1,b:b17,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b4,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b6,righttrigger:a4,rightx:a2,righty:a5,start:b16,x:b0,y:b2,",
"31326235383662333266633463653332,PS4 Controller,a:b1,b:b16,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b4,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b6,righttrigger:a4,rightx:a2,righty:a5,start:b17,x:b0,y:b2,",
"31373231336561636235613666323035,PS4 Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"31663838336334393132303338353963,PS4 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"34613139376634626133336530386430,PS4 Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"35643031303033326130316330353564,PS4 Controller,a:b1,b:b17,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,leftstick:b4,lefttrigger:+a3,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b6,righttrigger:+a4,rightx:a2,righty:a5,start:b16,x:b0,y:b2,",
"37626233336235343937333961353732,PS4 Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"37626464343430636562316661643863,PS4 Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"38393161636261653636653532386639,PS4 Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"63313733393535663339656564343962,PS4 Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"63393662363836383439353064663939,PS4 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"65366465656364636137653363376531,PS4 Controller,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b0,y:b2,",
"66613532303965383534396638613230,PS4 Controller,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a5,start:b18,x:b0,y:b2,",
"050000004c050000e60c0000fffe3f00,PS5 Controller,a:b1,b:b17,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,leftstick:b4,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b6,righttrigger:a4,rightx:a2,righty:a5,start:b16,x:b0,y:b2,",
"050000004c050000e60c0000fffe3f80,PS5 Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,leftstick:b4,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b6,righttrigger:a3,rightx:a4,righty:a5,start:b16,x:b2,y:b17,",
"050000004c050000e60c0000ffff3f00,PS5 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"32346465346533616263386539323932,PS5 Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"32633532643734376632656664383733,PS5 Controller,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a5,start:b18,x:b0,y:b2,",
"37363764353731323963323639666565,PS5 Controller,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a5,start:b18,x:b0,y:b2,",
"61303162353165316365336436343139,PS5 Controller,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,misc1:b8,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a5,start:b18,x:b0,y:b2,",
"64336263393933626535303339616332,Qanba 4RAF,a:b0,b:b1,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b20,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:b3,righttrigger:b9,rightx:a2,righty:a3,start:b18,x:b19,y:b2,",
"36626666353861663864336130363137,Razer Junglecat,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"05000000f8270000bf0b0000ffff3f00,Razer Kishi,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"62653861643333663663383332396665,Razer Kishi,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000003215000005070000ffff3f00,Razer Raiju Mobile,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000003215000007070000ffff3f00,Razer Raiju Mobile,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000003215000000090000bf7f3f00,Razer Serval,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,x:b2,y:b3,",
"5a6869587520526574726f2042697420,Retro Bit Saturn Controller,a:b21,b:b22,back:b29,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b25,lefttrigger:b26,rightshoulder:b27,righttrigger:b28,start:b30,x:b23,y:b24,",
"32417865732031314b6579732047616d,Retro Bit SNES Controller,a:b0,b:b1,back:b15,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,rightshoulder:b10,start:b6,x:b2,y:b3,",
"36313938306539326233393732613361,Retro Bit SNES Controller,a:b0,b:b1,back:b15,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,rightshoulder:b10,start:b6,x:b2,y:b3,",
"526574726f466c616720576972656420,Retro Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b17,rightshoulder:b18,start:b10,x:b2,y:b3,",
"61343739353764363165343237303336,Retro Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b17,lefttrigger:b18,leftx:a0,lefty:a1,start:b10,x:b2,y:b3,",
"526574726f696420506f636b65742043,Retroid Pocket,a:b1,b:b0,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b17,paddle2:b18,rightshoulder:b10,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"582d426f7820436f6e74726f6c6c6572,Retroid Pocket,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,paddle1:b17,paddle2:b18,rightshoulder:b10,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"64633735616665613536653363336132,Retroid Pocket,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,paddle1:b19,paddle2:b20,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38653130373365613538333235303036,Retroid Pocket 2,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"64363363336633363736393038313463,Retrolink,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,start:b6,",
"37393234373533633333323633646531,RetroUSB N64 RetroPort,+rightx:b17,+righty:b15,-rightx:b18,-righty:b6,a:b10,b:b9,dpdown:b19,dpleft:b1,dpright:b0,dpup:b2,leftshoulder:b7,lefttrigger:b20,leftx:a0,lefty:a1,rightshoulder:b5,start:b3,",
"5365616c6965436f6d707574696e6720,RetroUSB N64 RetroPort,+rightx:b17,+righty:b15,-rightx:b18,-righty:b6,a:b10,b:b9,dpdown:b19,dpleft:b1,dpright:b0,dpup:b2,leftshoulder:b7,lefttrigger:b20,leftx:a0,lefty:a1,rightshoulder:b5,start:b3,",
"526574726f5553422e636f6d20534e45,RetroUSB SNES RetroPort,a:b1,b:b20,back:b19,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,rightshoulder:b10,start:b2,x:b0,y:b3,",
"64643037633038386238303966376137,RetroUSB SNES RetroPort,a:b1,b:b20,back:b19,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,rightshoulder:b10,start:b2,x:b0,y:b3,",
"37656564346533643138636436356230,Rock Candy Switch Controller,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b3,leftstick:b15,lefttrigger:b9,leftx:a0,lefty:a1,misc1:b7,rightshoulder:b20,rightstick:b6,righttrigger:b10,rightx:a2,righty:a3,start:b18,x:b0,y:b2,",
"33373336396634316434323337666361,RumblePad 2,a:b22,b:b23,back:b29,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b25,lefttrigger:b27,leftx:a0,lefty:a1,rightshoulder:b26,righttrigger:b28,rightx:a2,righty:a3,start:b30,x:b21,y:b24,",
"36363537303435333566386638366333,Samsung EIGP20,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"53616d73756e672047616d6520506164,Samsung EIGP20,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"66386565396238363534313863353065,Sanwa PlayOnline Mobile,a:b21,b:b22,back:b23,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,start:b24,",
"32383165316333383766336338373261,Saturn,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:a4,righttrigger:a5,x:b2,y:b3,",
"38613865396530353338373763623431,Saturn,a:b0,b:b1,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,lefttrigger:b10,rightshoulder:b20,righttrigger:b19,start:b17,x:b2,y:b3,",
"61316232336262373631343137633631,Saturn,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,lefttrigger:b10,leftx:a0,lefty:a1,rightshoulder:a4,righttrigger:a5,x:b2,y:b3,",
"30353835333338613130373363646337,SG H510,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b17,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b18,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b19,y:b2,",
"66386262366536653765333235343634,SG H510,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,x:b2,y:b3,",
"66633132393363353531373465633064,SG H510,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b17,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b20,rightstick:b18,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b19,y:b2,",
"62653761636366393366613135366338,SN30 PP,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b3,y:b2,",
"38376662666661636265313264613039,SNES,a:b0,b:b1,back:b9,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b3,rightshoulder:b20,start:b10,x:b19,y:b2,",
"5346432f555342205061640000000000,SNES Adapter,a:b0,b:b1,back:b9,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b3,rightshoulder:b20,start:b10,x:b19,y:b2,",
"5553422047616d657061642000000000,SNES Controller,a:b1,b:b0,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,rightshoulder:b10,start:b6,x:b3,y:b2,",
"62653335326261303663356263626339,Sony PlayStation Classic Controller,a:b19,b:b1,back:b17,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b9,lefttrigger:b3,rightshoulder:b10,righttrigger:b20,start:b18,x:b2,y:b0,",
"536f6e7920496e746572616374697665,Sony PlayStation Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,misc1:b8,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"576972656c65737320436f6e74726f6c,Sony PlayStation Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"63303964303462366136616266653561,Sony PSP,a:b21,b:b22,back:b27,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b25,leftx:a0,lefty:a1,rightshoulder:b26,start:b28,x:b23,y:b24,",
"63376637643462343766333462383235,Sony Vita,a:b1,b:b19,back:b17,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftx:a0,lefty:a1,rightshoulder:b20,rightx:a3,righty:a4,start:b18,x:b0,y:b2,",
"05000000de2800000511000001000000,Steam Controller,a:b0,b:b1,back:b6,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a3,start:b7,x:b2,y:b3,",
"05000000de2800000611000001000000,Steam Controller,a:b0,b:b1,back:b6,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,righttrigger:a3,start:b7,x:b2,y:b3,",
"0500000011010000201400000f7e0f00,SteelSeries Nimbus,a:b0,b:b1,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,lefttrigger:b9,leftx:a0,lefty:a1,rightshoulder:b20,righttrigger:b10,rightx:a2,righty:a3,x:b19,y:b2,",
"35306436396437373135383665646464,SteelSeries Nimbus Plus,a:b0,b:b1,leftshoulder:b3,leftstick:b17,lefttrigger:b9,leftx:a0,rightshoulder:b20,rightstick:b18,righttrigger:b10,rightx:a2,x:b19,y:b2,",
"33313930373536613937326534303931,Taito Egret II Mini Control Panel,a:b25,b:b23,back:b27,guide:b30,leftx:a0,lefty:a1,rightshoulder:b21,righttrigger:b22,start:b28,x:b29,y:b24,",
"54475a20436f6e74726f6c6c65720000,TGZ Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"62363434353532386238336663643836,TGZ Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b17,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:b18,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"37323236633763666465316365313236,THEC64 Joystick,a:b21,b:b22,back:b27,leftshoulder:b25,leftx:a0,lefty:a1,rightshoulder:b26,start:b27,x:b23,y:b24,",
"38346162326232346533316164363336,THEGamepad,a:b23,b:b22,back:b27,leftshoulder:b25,leftx:a0,lefty:a1,rightshoulder:b26,start:b28,x:b24,y:b21,",
"050000004f0400000ed00000fffe3f00,Thrustmaster eSwap Pro Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"5477696e20555342204a6f7973746963,Twin Joystick,a:b22,b:b21,back:b28,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b26,leftstick:b30,lefttrigger:b24,leftx:a0,lefty:a1,rightshoulder:b27,rightstick:b31,righttrigger:b25,rightx:a3,righty:a2,start:b29,x:b23,y:b20,",
"30623739343039643830333266346439,Valve Steam Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,leftx:a0,lefty:a1,paddle1:b24,paddle2:b23,rightshoulder:b10,rightstick:b8,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"31643365666432386133346639383937,Valve Steam Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,leftx:a0,lefty:a1,paddle1:b24,paddle2:b23,rightshoulder:b10,rightstick:b8,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"30386438313564306161393537333663,Wii Classic Adapter,a:b23,b:b22,back:b29,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b27,lefttrigger:b25,leftx:a0,lefty:a1,rightshoulder:b28,righttrigger:b26,rightx:a2,righty:a3,start:b30,x:b24,y:b21,",
"33333034646336346339646538643633,Wii Classic Adapter,a:b23,b:b22,back:b29,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b27,lefttrigger:b25,leftx:a0,lefty:a1,rightshoulder:b28,righttrigger:b26,rightx:a2,righty:a3,start:b30,x:b24,y:b21,",
"050000005e0400008e02000000783f00,Xbox 360 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"30396232393162346330326334636566,Xbox 360 Controller,a:b0,b:b1,back:b4,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"38313038323730383864666463383533,Xbox 360 Controller,a:b0,b:b1,back:b4,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"58626f782033363020576972656c6573,Xbox 360 Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"65353331386662343338643939643636,Xbox 360 Controller,a:b0,b:b1,back:b4,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"65613532386633373963616462363038,Xbox 360 Controller,a:b0,b:b1,back:b4,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"47656e6572696320582d426f78207061,Xbox Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"4d6963726f736f667420582d426f7820,Xbox Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a4,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"64633436313965656664373634323364,Xbox Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b19,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000005e04000091020000ff073f00,Xbox One Controller,a:b0,b:b1,back:b4,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"050000005e04000091020000ff073f80,Xbox One Controller,a:b0,b:b1,back:b4,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000005e040000e00200000ffe3f00,Xbox One Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b16,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b17,y:b2,",
"050000005e040000e00200000ffe3f80,Xbox One Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b3,leftstick:b15,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b16,righttrigger:a5,rightx:a2,righty:a3,start:b10,x:b17,y:b2,",
"050000005e040000e0020000ffff3f00,Xbox One Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b4,leftshoulder:b3,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b7,righttrigger:a5,rightx:a3,righty:a4,start:b10,x:b17,y:b2,",
"050000005e040000e0020000ffff3f80,Xbox One Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b4,leftshoulder:b3,leftstick:b8,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b7,righttrigger:a5,rightx:a2,righty:a3,start:b10,x:b17,y:b2,",
"050000005e040000fd020000ffff3f00,Xbox One Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"33356661323266333733373865656366,Xbox One Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"34356136633366613530316338376136,Xbox One Controller,a:b0,b:b1,back:b9,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b10,leftshoulder:b3,leftstick:b15,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b18,rightstick:b16,righttrigger:a5,rightx:a3,righty:a4,x:b17,y:b2,",
"35623965373264386238353433656138,Xbox One Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"36616131643361333337396261666433,Xbox One Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"58626f7820576972656c65737320436f,Xbox One Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"65316262316265373335666131623538,Xbox One Controller,a:b0,b:b1,back:b15,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000005e040000000b000000783f00,Xbox One Elite 2 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"050000005e040000000b000000783f80,Xbox One Elite 2 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000005e040000050b0000ffff3f00,Xbox One Elite 2 Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a6,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000005e040000e002000000783f00,Xbox One S Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000005e040000ea02000000783f00,Xbox One S Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000005e040000fd020000ff7f3f00,Xbox One S Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000005e040000120b000000783f00,Xbox Series Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a3,righty:a4,start:b6,x:b2,y:b3,",
"050000005e040000120b000000783f80,Xbox Series Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000005e040000130b0000ffff3f00,Xbox Series Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"65633038363832353634653836396239,Xbox Series Controller,a:b0,b:b1,back:b15,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,guide:b5,leftshoulder:b9,leftstick:b7,lefttrigger:a4,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a5,rightx:a2,righty:a3,start:b6,x:b2,y:b3,",
"050000001727000044310000ffff3f00,XiaoMi Controller,a:b0,b:b1,back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:a7,leftx:a0,lefty:a1,rightshoulder:b10,rightstick:b8,righttrigger:a6,rightx:a2,righty:a5,start:b6,x:b2,y:b3,"
#elif defined(EMSCRIPTEN)
"\0" /* look into at some point */
#endif
};

void mg_mappings_init(void) {
    mg_size_t i;
    mappings.mappingCount = 0;
    mappings.mappingMax = 1300;
    MG_MEMSET(mappings.mappings, 0, sizeof(mappings.mappings));

    for (i = 0;  i < (sizeof(sdl_db) / sizeof(char*));  i++) {
        if (parseMapping(&mappings.mappings[mappings.mappingCount], sdl_db[i]))
            mappings.mappingCount++;
    }
}

const char* mg_button_get_name(mg_button btn) {
    switch (btn) {
        case MG_BUTTON_COUNT:
        case MG_BUTTON_UNKNOWN:
            return "Unknown Button";
        case MG_BUTTON_SOUTH:
            return "South Button";
        case MG_BUTTON_WEST:
            return "West Button";
        case MG_BUTTON_NORTH:
            return "North Button";
        case MG_BUTTON_EAST:
            return "East Button";
        case MG_BUTTON_BACK:
            return "Back Button";
        case MG_BUTTON_GUIDE:
            return "Guide Button";
        case MG_BUTTON_START:
            return "Start Button";
        case MG_BUTTON_LEFT_STICK:
            return "Left Stick Button";
        case MG_BUTTON_RIGHT_STICK:
            return "Right Stick Button";
        case MG_BUTTON_DPAD_UP:
            return "D-pad Up";
        case MG_BUTTON_DPAD_DOWN:
            return "D-pad Down";
        case MG_BUTTON_DPAD_LEFT:
            return "D-pad Left";
        case MG_BUTTON_DPAD_RIGHT:
            return "D-pad Right";
        case MG_BUTTON_LEFT_SHOULDER:
            return "Left Shoulder Button";
        case MG_BUTTON_RIGHT_SHOULDER:
            return "Right Shoulder Button";
        case MG_BUTTON_LEFT_TRIGGER:
            return "Left Trigger Button";
        case MG_BUTTON_RIGHT_TRIGGER:
            return "Right Trigger Button";
        case MG_BUTTON_MISC1:
            return "Misc Button 1";
        case MG_BUTTON_RIGHT_PADDLE1:
            return "Paddle 1 Right";
        case MG_BUTTON_LEFT_PADDLE1:
            return "Paddle 1 Left";
        case MG_BUTTON_RIGHT_PADDLE2:
            return "Paddle 2 Right";
        case MG_BUTTON_LEFT_PADDLE2:
            return "Paddle 2 Left";
        case MG_BUTTON_TOUCHPAD:
            return "Touchpad";
        case MG_BUTTON_MISC2:
            return "Misc Button 2";
        case MG_BUTTON_MISC3:
            return "Misc Button 3";
        case MG_BUTTON_MISC4:
            return "Misc Button 4";
        case MG_BUTTON_MISC5:
            return "Misc Button 5";
        case MG_BUTTON_MISC6:
            return "Misc Button 6";
        default: return NULL;
    }
    return NULL;
}

const char* mg_axis_get_name(mg_axis axis) {
    switch (axis) {
        case MG_AXIS_COUNT:
        case MG_AXIS_UNKNOWN:
            return "Unknown Axis";
        case MG_AXIS_LEFT_X:
            return "X Axis";
        case MG_AXIS_LEFT_Y:
            return "Y Axis";
        case MG_AXIS_LEFT_TRIGGER:
            return "Z Axis";
        case MG_AXIS_RIGHT_X:
            return "RX Axis";
        case MG_AXIS_RIGHT_Y:
            return "RY Axis";
        case MG_AXIS_RIGHT_TRIGGER:
            return "RZ Axis";
        case MG_AXIS_THROTTLE:
            return "Throttle";
        case MG_AXIS_RUDDER:
            return "Rudder";
        case MG_AXIS_WHEEL:
            return "Wheel";
        case MG_AXIS_GAS:
            return "Gas";
        case MG_AXIS_BRAKE:
            return "Brake";
        case MG_AXIS_HAT_DPAD_LEFT_RIGHT:
            return "Hat D-Pad Left-Right Axis";
        case MG_AXIS_HAT_DPAD_UP_DOWN:
            return "Hat D-Pad Up-Down Axis";
        case MG_AXIS_HAT1X:
            return "Hat 1 X Axis";
        case MG_AXIS_HAT1Y:
            return "Hat 1 Y Axis";
        case MG_AXIS_HAT2X:
            return "Hat 2 X Axis";
        case MG_AXIS_HAT2Y:
            return "Hat 2 Y Axis";
        case MG_AXIS_HAT3X:
            return "Hat 3 X Axis";
        case MG_AXIS_HAT3Y:
            return "Hat 3 Y Axis";
        case MG_AXIS_PRESSURE:
            return "Pressure Axis";
        case MG_AXIS_DISTANCE:
            return "Distance Axis";
        case MG_AXIS_TILT_X:
            return "Tilt X Axis";
        case MG_AXIS_TILT_Y:
            return "Tilt Y Axis";
        case MG_AXIS_TOOL_WIDTH:
            return "Tool Width Axis";
        case MG_AXIS_VOLUME:
            return "Volume Axis";
        case MG_AXIS_PROFILE:
            return "Profile Axis";
        case MG_AXIS_MISC:
            return "Misc Axis";
        default: break;
    }
    return NULL;
}

#endif /* MG_IMPLEMENTATION */

#if defined(__cplusplus) && !defined(__EMSCRIPTEN__)
}
#endif
