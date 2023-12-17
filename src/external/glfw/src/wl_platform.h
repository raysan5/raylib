//========================================================================
// GLFW 3.3 Wayland - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2014 Jonas Ådahl <jadahl@gmail.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <dlfcn.h>

#include <stdbool.h>

typedef VkFlags VkWaylandSurfaceCreateFlagsKHR;

typedef struct VkWaylandSurfaceCreateInfoKHR
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkWaylandSurfaceCreateFlagsKHR  flags;
    struct wl_display*              display;
    struct wl_surface*              surface;
} VkWaylandSurfaceCreateInfoKHR;

typedef VkResult (APIENTRY *PFN_vkCreateWaylandSurfaceKHR)(VkInstance,const VkWaylandSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkBool32 (APIENTRY *PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)(VkPhysicalDevice,uint32_t,struct wl_display*);

#include "posix_thread.h"
#include "posix_time.h"
#ifdef __linux__
#include "linux_joystick.h"
#else
#include "null_joystick.h"
#endif
#include "xkb_unicode.h"
#include "egl_context.h"
#include "osmesa_context.h"

#include "wayland-xdg-shell-client-protocol.h"
#include "wayland-xdg-decoration-client-protocol.h"
#include "wayland-viewporter-client-protocol.h"
#include "wayland-relative-pointer-unstable-v1-client-protocol.h"
#include "wayland-pointer-constraints-unstable-v1-client-protocol.h"
#include "wayland-idle-inhibit-unstable-v1-client-protocol.h"

#define _glfw_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define _glfw_dlclose(handle) dlclose(handle)
#define _glfw_dlsym(handle, name) dlsym(handle, name)

#define _GLFW_EGL_NATIVE_WINDOW         ((EGLNativeWindowType) window->wl.egl.window)
#define _GLFW_EGL_NATIVE_DISPLAY        ((EGLNativeDisplayType) _glfw.wl.display)

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowWayland  wl
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryWayland wl
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorWayland wl
#define _GLFW_PLATFORM_CURSOR_STATE         _GLFWcursorWayland  wl

#define _GLFW_PLATFORM_CONTEXT_STATE         struct { int dummyContext; }
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE struct { int dummyLibraryContext; }

struct wl_cursor_image {
    uint32_t width;
    uint32_t height;
    uint32_t hotspot_x;
    uint32_t hotspot_y;
    uint32_t delay;
};
struct wl_cursor {
    unsigned int image_count;
    struct wl_cursor_image** images;
    char* name;
};
typedef struct wl_cursor_theme* (* PFN_wl_cursor_theme_load)(const char*, int, struct wl_shm*);
typedef void (* PFN_wl_cursor_theme_destroy)(struct wl_cursor_theme*);
typedef struct wl_cursor* (* PFN_wl_cursor_theme_get_cursor)(struct wl_cursor_theme*, const char*);
typedef struct wl_buffer* (* PFN_wl_cursor_image_get_buffer)(struct wl_cursor_image*);
#define wl_cursor_theme_load _glfw.wl.cursor.theme_load
#define wl_cursor_theme_destroy _glfw.wl.cursor.theme_destroy
#define wl_cursor_theme_get_cursor _glfw.wl.cursor.theme_get_cursor
#define wl_cursor_image_get_buffer _glfw.wl.cursor.image_get_buffer

typedef struct wl_egl_window* (* PFN_wl_egl_window_create)(struct wl_surface*, int, int);
typedef void (* PFN_wl_egl_window_destroy)(struct wl_egl_window*);
typedef void (* PFN_wl_egl_window_resize)(struct wl_egl_window*, int, int, int, int);
#define wl_egl_window_create _glfw.wl.egl.window_create
#define wl_egl_window_destroy _glfw.wl.egl.window_destroy
#define wl_egl_window_resize _glfw.wl.egl.window_resize

typedef struct xkb_context* (* PFN_xkb_context_new)(enum xkb_context_flags);
typedef void (* PFN_xkb_context_unref)(struct xkb_context*);
typedef struct xkb_keymap* (* PFN_xkb_keymap_new_from_string)(struct xkb_context*, const char*, enum xkb_keymap_format, enum xkb_keymap_compile_flags);
typedef void (* PFN_xkb_keymap_unref)(struct xkb_keymap*);
typedef xkb_mod_index_t (* PFN_xkb_keymap_mod_get_index)(struct xkb_keymap*, const char*);
typedef int (* PFN_xkb_keymap_key_repeats)(struct xkb_keymap*, xkb_keycode_t);
typedef int (* PFN_xkb_keymap_key_get_syms_by_level)(struct xkb_keymap*,xkb_keycode_t,xkb_layout_index_t,xkb_level_index_t,const xkb_keysym_t**);
typedef struct xkb_state* (* PFN_xkb_state_new)(struct xkb_keymap*);
typedef void (* PFN_xkb_state_unref)(struct xkb_state*);
typedef int (* PFN_xkb_state_key_get_syms)(struct xkb_state*, xkb_keycode_t, const xkb_keysym_t**);
typedef enum xkb_state_component (* PFN_xkb_state_update_mask)(struct xkb_state*, xkb_mod_mask_t, xkb_mod_mask_t, xkb_mod_mask_t, xkb_layout_index_t, xkb_layout_index_t, xkb_layout_index_t);
typedef xkb_layout_index_t (* PFN_xkb_state_key_get_layout)(struct xkb_state*,xkb_keycode_t);
typedef int (* PFN_xkb_state_mod_index_is_active)(struct xkb_state*,xkb_mod_index_t,enum xkb_state_component);
#define xkb_context_new _glfw.wl.xkb.context_new
#define xkb_context_unref _glfw.wl.xkb.context_unref
#define xkb_keymap_new_from_string _glfw.wl.xkb.keymap_new_from_string
#define xkb_keymap_unref _glfw.wl.xkb.keymap_unref
#define xkb_keymap_mod_get_index _glfw.wl.xkb.keymap_mod_get_index
#define xkb_keymap_key_repeats _glfw.wl.xkb.keymap_key_repeats
#define xkb_keymap_key_get_syms_by_level _glfw.wl.xkb.keymap_key_get_syms_by_level
#define xkb_state_new _glfw.wl.xkb.state_new
#define xkb_state_unref _glfw.wl.xkb.state_unref
#define xkb_state_key_get_syms _glfw.wl.xkb.state_key_get_syms
#define xkb_state_update_mask _glfw.wl.xkb.state_update_mask
#define xkb_state_key_get_layout _glfw.wl.xkb.state_key_get_layout
#define xkb_state_mod_index_is_active _glfw.wl.xkb.state_mod_index_is_active

typedef struct xkb_compose_table* (* PFN_xkb_compose_table_new_from_locale)(struct xkb_context*, const char*, enum xkb_compose_compile_flags);
typedef void (* PFN_xkb_compose_table_unref)(struct xkb_compose_table*);
typedef struct xkb_compose_state* (* PFN_xkb_compose_state_new)(struct xkb_compose_table*, enum xkb_compose_state_flags);
typedef void (* PFN_xkb_compose_state_unref)(struct xkb_compose_state*);
typedef enum xkb_compose_feed_result (* PFN_xkb_compose_state_feed)(struct xkb_compose_state*, xkb_keysym_t);
typedef enum xkb_compose_status (* PFN_xkb_compose_state_get_status)(struct xkb_compose_state*);
typedef xkb_keysym_t (* PFN_xkb_compose_state_get_one_sym)(struct xkb_compose_state*);
#define xkb_compose_table_new_from_locale _glfw.wl.xkb.compose_table_new_from_locale
#define xkb_compose_table_unref _glfw.wl.xkb.compose_table_unref
#define xkb_compose_state_new _glfw.wl.xkb.compose_state_new
#define xkb_compose_state_unref _glfw.wl.xkb.compose_state_unref
#define xkb_compose_state_feed _glfw.wl.xkb.compose_state_feed
#define xkb_compose_state_get_status _glfw.wl.xkb.compose_state_get_status
#define xkb_compose_state_get_one_sym _glfw.wl.xkb.compose_state_get_one_sym

struct libdecor;
struct libdecor_frame;
struct libdecor_state;
struct libdecor_configuration;

enum libdecor_error
{
	LIBDECOR_ERROR_COMPOSITOR_INCOMPATIBLE,
	LIBDECOR_ERROR_INVALID_FRAME_CONFIGURATION,
};

enum libdecor_window_state
{
	LIBDECOR_WINDOW_STATE_NONE = 0,
	LIBDECOR_WINDOW_STATE_ACTIVE = 1,
	LIBDECOR_WINDOW_STATE_MAXIMIZED = 2,
	LIBDECOR_WINDOW_STATE_FULLSCREEN = 4,
	LIBDECOR_WINDOW_STATE_TILED_LEFT = 8,
	LIBDECOR_WINDOW_STATE_TILED_RIGHT = 16,
	LIBDECOR_WINDOW_STATE_TILED_TOP = 32,
	LIBDECOR_WINDOW_STATE_TILED_BOTTOM = 64
};

enum libdecor_capabilities
{
	LIBDECOR_ACTION_MOVE = 1,
	LIBDECOR_ACTION_RESIZE = 2,
	LIBDECOR_ACTION_MINIMIZE = 4,
	LIBDECOR_ACTION_FULLSCREEN = 8,
	LIBDECOR_ACTION_CLOSE = 16
};

struct libdecor_interface
{
	void (* error)(struct libdecor*,enum libdecor_error,const char*);
	void (* reserved0)(void);
	void (* reserved1)(void);
	void (* reserved2)(void);
	void (* reserved3)(void);
	void (* reserved4)(void);
	void (* reserved5)(void);
	void (* reserved6)(void);
	void (* reserved7)(void);
	void (* reserved8)(void);
	void (* reserved9)(void);
};

struct libdecor_frame_interface
{
	void (* configure)(struct libdecor_frame*,struct libdecor_configuration*,void*);
	void (* close)(struct libdecor_frame*,void*);
	void (* commit)(struct libdecor_frame*,void*);
	void (* dismiss_popup)(struct libdecor_frame*,const char*,void*);
	void (* reserved0)(void);
	void (* reserved1)(void);
	void (* reserved2)(void);
	void (* reserved3)(void);
	void (* reserved4)(void);
	void (* reserved5)(void);
	void (* reserved6)(void);
	void (* reserved7)(void);
	void (* reserved8)(void);
	void (* reserved9)(void);
};

typedef struct libdecor* (* PFN_libdecor_new)(struct wl_display*,const struct libdecor_interface*);
typedef void (* PFN_libdecor_unref)(struct libdecor*);
typedef int (* PFN_libdecor_get_fd)(struct libdecor*);
typedef int (* PFN_libdecor_dispatch)(struct libdecor*,int);
typedef struct libdecor_frame* (* PFN_libdecor_decorate)(struct libdecor*,struct wl_surface*,const struct libdecor_frame_interface*,void*);
typedef void (* PFN_libdecor_frame_unref)(struct libdecor_frame*);
typedef void (* PFN_libdecor_frame_set_app_id)(struct libdecor_frame*,const char*);
typedef void (* PFN_libdecor_frame_set_title)(struct libdecor_frame*,const char*);
typedef void (* PFN_libdecor_frame_set_minimized)(struct libdecor_frame*);
typedef void (* PFN_libdecor_frame_set_fullscreen)(struct libdecor_frame*,struct wl_output*);
typedef void (* PFN_libdecor_frame_unset_fullscreen)(struct libdecor_frame*);
typedef void (* PFN_libdecor_frame_map)(struct libdecor_frame*);
typedef void (* PFN_libdecor_frame_commit)(struct libdecor_frame*,struct libdecor_state*,struct libdecor_configuration*);
typedef void (* PFN_libdecor_frame_set_min_content_size)(struct libdecor_frame*,int,int);
typedef void (* PFN_libdecor_frame_set_max_content_size)(struct libdecor_frame*,int,int);
typedef void (* PFN_libdecor_frame_set_maximized)(struct libdecor_frame*);
typedef void (* PFN_libdecor_frame_unset_maximized)(struct libdecor_frame*);
typedef void (* PFN_libdecor_frame_set_capabilities)(struct libdecor_frame*,enum libdecor_capabilities);
typedef void (* PFN_libdecor_frame_unset_capabilities)(struct libdecor_frame*,enum libdecor_capabilities);
typedef void (* PFN_libdecor_frame_set_visibility)(struct libdecor_frame*,bool visible);
typedef struct xdg_toplevel* (* PFN_libdecor_frame_get_xdg_toplevel)(struct libdecor_frame*);
typedef bool (* PFN_libdecor_configuration_get_content_size)(struct libdecor_configuration*,struct libdecor_frame*,int*,int*);
typedef bool (* PFN_libdecor_configuration_get_window_state)(struct libdecor_configuration*,enum libdecor_window_state*);
typedef struct libdecor_state* (* PFN_libdecor_state_new)(int,int);
typedef void (* PFN_libdecor_state_free)(struct libdecor_state*);
#define libdecor_new _glfw.wl.libdecor.libdecor_new_
#define libdecor_unref _glfw.wl.libdecor.libdecor_unref_
#define libdecor_get_fd _glfw.wl.libdecor.libdecor_get_fd_
#define libdecor_dispatch _glfw.wl.libdecor.libdecor_dispatch_
#define libdecor_decorate _glfw.wl.libdecor.libdecor_decorate_
#define libdecor_frame_unref _glfw.wl.libdecor.libdecor_frame_unref_
#define libdecor_frame_set_app_id _glfw.wl.libdecor.libdecor_frame_set_app_id_
#define libdecor_frame_set_title _glfw.wl.libdecor.libdecor_frame_set_title_
#define libdecor_frame_set_minimized _glfw.wl.libdecor.libdecor_frame_set_minimized_
#define libdecor_frame_set_fullscreen _glfw.wl.libdecor.libdecor_frame_set_fullscreen_
#define libdecor_frame_unset_fullscreen _glfw.wl.libdecor.libdecor_frame_unset_fullscreen_
#define libdecor_frame_map _glfw.wl.libdecor.libdecor_frame_map_
#define libdecor_frame_commit _glfw.wl.libdecor.libdecor_frame_commit_
#define libdecor_frame_set_min_content_size _glfw.wl.libdecor.libdecor_frame_set_min_content_size_
#define libdecor_frame_set_max_content_size _glfw.wl.libdecor.libdecor_frame_set_max_content_size_
#define libdecor_frame_set_maximized _glfw.wl.libdecor.libdecor_frame_set_maximized_
#define libdecor_frame_unset_maximized _glfw.wl.libdecor.libdecor_frame_unset_maximized_
#define libdecor_frame_set_capabilities _glfw.wl.libdecor.libdecor_frame_set_capabilities_
#define libdecor_frame_unset_capabilities _glfw.wl.libdecor.libdecor_frame_unset_capabilities_
#define libdecor_frame_set_visibility _glfw.wl.libdecor.libdecor_frame_set_visibility_
#define libdecor_frame_get_xdg_toplevel _glfw.wl.libdecor.libdecor_frame_get_xdg_toplevel_
#define libdecor_configuration_get_content_size _glfw.wl.libdecor.libdecor_configuration_get_content_size_
#define libdecor_configuration_get_window_state _glfw.wl.libdecor.libdecor_configuration_get_window_state_
#define libdecor_state_new _glfw.wl.libdecor.libdecor_state_new_
#define libdecor_state_free _glfw.wl.libdecor.libdecor_state_free_

typedef enum _GLFWdecorationSideWayland
{
    GLFW_MAIN_WINDOW,
    GLFW_TOP_DECORATION,
    GLFW_LEFT_DECORATION,
    GLFW_RIGHT_DECORATION,
    GLFW_BOTTOM_DECORATION
} _GLFWdecorationSideWayland;

typedef struct _GLFWdecorationWayland
{
    struct wl_surface*          surface;
    struct wl_subsurface*       subsurface;
    struct wp_viewport*         viewport;
} _GLFWdecorationWayland;

typedef struct _GLFWofferWayland
{
    struct wl_data_offer*       offer;
    GLFWbool                    text_plain_utf8;
    GLFWbool                    text_uri_list;
} _GLFWofferWayland;

typedef struct _GLFWscaleWayland
{
    struct wl_output*           output;
    int                         factor;
} _GLFWscaleWayland;

// Wayland-specific per-window data
//
typedef struct _GLFWwindowWayland
{
    int                         width, height;
    GLFWbool                    visible;
    GLFWbool                    maximized;
    GLFWbool                    activated;
    GLFWbool                    fullscreen;
    GLFWbool                    hovered;
    GLFWbool                    transparent;
    struct wl_surface*          surface;
    struct wl_callback*         callback;

    struct {
        struct wl_egl_window*   window;
    } egl;

    struct {
        int                     width, height;
        GLFWbool                maximized;
        GLFWbool                iconified;
        GLFWbool                activated;
        GLFWbool                fullscreen;
    } pending;

    struct {
        struct xdg_surface*     surface;
        struct xdg_toplevel*    toplevel;
        struct zxdg_toplevel_decoration_v1* decoration;
        uint32_t                decorationMode;
    } xdg;

    struct {
        struct libdecor_frame*  frame;
        int                     mode;
    } libdecor;

    _GLFWcursor*                currentCursor;
    double                      cursorPosX, cursorPosY;

    char*                       title;

    // We need to track the monitors the window spans on to calculate the
    // optimal scaling factor.
    int                         contentScale;
    _GLFWscaleWayland*          scales;
    int                         scaleCount;
    int                         scaleSize;

    struct zwp_relative_pointer_v1* relativePointer;
    struct zwp_locked_pointer_v1*   lockedPointer;

    struct zwp_idle_inhibitor_v1*          idleInhibitor;

    struct {
        struct wl_buffer*                  buffer;
        _GLFWdecorationWayland             top, left, right, bottom;
        _GLFWdecorationSideWayland         focus;
    } decorations;
} _GLFWwindowWayland;

// Wayland-specific global data
//
typedef struct _GLFWlibraryWayland
{
    struct wl_display*          display;
    struct wl_registry*         registry;
    struct wl_compositor*       compositor;
    struct wl_subcompositor*    subcompositor;
    struct wl_shm*              shm;
    struct wl_seat*             seat;
    struct wl_pointer*          pointer;
    struct wl_keyboard*         keyboard;
    struct wl_data_device_manager*          dataDeviceManager;
    struct wl_data_device*      dataDevice;
    struct xdg_wm_base*         wmBase;
    struct zxdg_decoration_manager_v1*      decorationManager;
    struct wp_viewporter*       viewporter;
    struct zwp_relative_pointer_manager_v1* relativePointerManager;
    struct zwp_pointer_constraints_v1*      pointerConstraints;
    struct zwp_idle_inhibit_manager_v1*     idleInhibitManager;

    _GLFWofferWayland*          offers;
    unsigned int                offerCount;

    struct wl_data_offer*       selectionOffer;
    struct wl_data_source*      selectionSource;

    struct wl_data_offer*       dragOffer;
    _GLFWwindow*                dragFocus;
    uint32_t                    dragSerial;

    const char*                 tag;

    struct wl_cursor_theme*     cursorTheme;
    struct wl_cursor_theme*     cursorThemeHiDPI;
    struct wl_surface*          cursorSurface;
    const char*                 cursorPreviousName;
    int                         cursorTimerfd;
    uint32_t                    serial;
    uint32_t                    pointerEnterSerial;

    int                         keyRepeatTimerfd;
    int32_t                     keyRepeatRate;
    int32_t                     keyRepeatDelay;
    int                         keyRepeatScancode;

    char*                       clipboardString;
    short int                   keycodes[256];
    short int                   scancodes[GLFW_KEY_LAST + 1];
    char                        keynames[GLFW_KEY_LAST + 1][5];

    struct {
        void*                   handle;
        struct xkb_context*     context;
        struct xkb_keymap*      keymap;
        struct xkb_state*       state;
        struct xkb_compose_state* composeState;

        xkb_mod_index_t         controlIndex;
        xkb_mod_index_t         altIndex;
        xkb_mod_index_t         shiftIndex;
        xkb_mod_index_t         superIndex;
        xkb_mod_index_t         capsLockIndex;
        xkb_mod_index_t         numLockIndex;
        unsigned int            modifiers;

        PFN_xkb_context_new context_new;
        PFN_xkb_context_unref context_unref;
        PFN_xkb_keymap_new_from_string keymap_new_from_string;
        PFN_xkb_keymap_unref keymap_unref;
        PFN_xkb_keymap_mod_get_index keymap_mod_get_index;
        PFN_xkb_keymap_key_repeats keymap_key_repeats;
        PFN_xkb_keymap_key_get_syms_by_level keymap_key_get_syms_by_level;
        PFN_xkb_state_new state_new;
        PFN_xkb_state_unref state_unref;
        PFN_xkb_state_key_get_syms state_key_get_syms;
        PFN_xkb_state_update_mask state_update_mask;
        PFN_xkb_state_key_get_layout state_key_get_layout;
        PFN_xkb_state_mod_index_is_active state_mod_index_is_active;

        PFN_xkb_compose_table_new_from_locale compose_table_new_from_locale;
        PFN_xkb_compose_table_unref compose_table_unref;
        PFN_xkb_compose_state_new compose_state_new;
        PFN_xkb_compose_state_unref compose_state_unref;
        PFN_xkb_compose_state_feed compose_state_feed;
        PFN_xkb_compose_state_get_status compose_state_get_status;
        PFN_xkb_compose_state_get_one_sym compose_state_get_one_sym;
    } xkb;

    _GLFWwindow*                pointerFocus;
    _GLFWwindow*                keyboardFocus;

    struct {
        void*                   handle;

        PFN_wl_cursor_theme_load theme_load;
        PFN_wl_cursor_theme_destroy theme_destroy;
        PFN_wl_cursor_theme_get_cursor theme_get_cursor;
        PFN_wl_cursor_image_get_buffer image_get_buffer;
    } cursor;

    struct {
        void*                   handle;

        PFN_wl_egl_window_create window_create;
        PFN_wl_egl_window_destroy window_destroy;
        PFN_wl_egl_window_resize window_resize;
    } egl;

    struct {
        void*                   handle;
        struct libdecor*        context;
        struct wl_callback*     callback;
        GLFWbool                ready;
        PFN_libdecor_new        libdecor_new_;
        PFN_libdecor_unref      libdecor_unref_;
        PFN_libdecor_get_fd     libdecor_get_fd_;
        PFN_libdecor_dispatch   libdecor_dispatch_;
        PFN_libdecor_decorate   libdecor_decorate_;
        PFN_libdecor_frame_unref libdecor_frame_unref_;
        PFN_libdecor_frame_set_app_id libdecor_frame_set_app_id_;
        PFN_libdecor_frame_set_title libdecor_frame_set_title_;
        PFN_libdecor_frame_set_minimized libdecor_frame_set_minimized_;
        PFN_libdecor_frame_set_fullscreen libdecor_frame_set_fullscreen_;
        PFN_libdecor_frame_unset_fullscreen libdecor_frame_unset_fullscreen_;
        PFN_libdecor_frame_map libdecor_frame_map_;
        PFN_libdecor_frame_commit libdecor_frame_commit_;
        PFN_libdecor_frame_set_min_content_size libdecor_frame_set_min_content_size_;
        PFN_libdecor_frame_set_max_content_size libdecor_frame_set_max_content_size_;
        PFN_libdecor_frame_set_maximized libdecor_frame_set_maximized_;
        PFN_libdecor_frame_unset_maximized libdecor_frame_unset_maximized_;
        PFN_libdecor_frame_set_capabilities libdecor_frame_set_capabilities_;
        PFN_libdecor_frame_unset_capabilities libdecor_frame_unset_capabilities_;
        PFN_libdecor_frame_set_visibility libdecor_frame_set_visibility_;
        PFN_libdecor_frame_get_xdg_toplevel libdecor_frame_get_xdg_toplevel_;
        PFN_libdecor_configuration_get_content_size libdecor_configuration_get_content_size_;
        PFN_libdecor_configuration_get_window_state libdecor_configuration_get_window_state_;
        PFN_libdecor_state_new libdecor_state_new_;
        PFN_libdecor_state_free libdecor_state_free_;
    } libdecor;
} _GLFWlibraryWayland;

// Wayland-specific per-monitor data
//
typedef struct _GLFWmonitorWayland
{
    struct wl_output*           output;
    uint32_t                    name;
    int                         currentMode;

    int                         x;
    int                         y;
    int                         contentScale;
} _GLFWmonitorWayland;

// Wayland-specific per-cursor data
//
typedef struct _GLFWcursorWayland
{
    struct wl_cursor*           cursor;
    struct wl_cursor*           cursorHiDPI;
    struct wl_buffer*           buffer;
    int                         width, height;
    int                         xhot, yhot;
    int                         currentImage;
} _GLFWcursorWayland;

void _glfwAddOutputWayland(uint32_t name, uint32_t version);
void _glfwUpdateContentScaleWayland(_GLFWwindow* window);

void _glfwAddSeatListenerWayland(struct wl_seat* seat);
void _glfwAddDataDeviceListenerWayland(struct wl_data_device* device);

