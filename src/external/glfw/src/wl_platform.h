//========================================================================
// GLFW 3.4 Wayland - www.glfw.org
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

#include <wayland-client-core.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>

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

#include "xkb_unicode.h"
#include "posix_poll.h"

typedef int (* PFN_wl_display_flush)(struct wl_display* display);
typedef void (* PFN_wl_display_cancel_read)(struct wl_display* display);
typedef int (* PFN_wl_display_dispatch_pending)(struct wl_display* display);
typedef int (* PFN_wl_display_read_events)(struct wl_display* display);
typedef struct wl_display* (* PFN_wl_display_connect)(const char*);
typedef void (* PFN_wl_display_disconnect)(struct wl_display*);
typedef int (* PFN_wl_display_roundtrip)(struct wl_display*);
typedef int (* PFN_wl_display_get_fd)(struct wl_display*);
typedef int (* PFN_wl_display_prepare_read)(struct wl_display*);
typedef void (* PFN_wl_proxy_marshal)(struct wl_proxy*,uint32_t,...);
typedef int (* PFN_wl_proxy_add_listener)(struct wl_proxy*,void(**)(void),void*);
typedef void (* PFN_wl_proxy_destroy)(struct wl_proxy*);
typedef struct wl_proxy* (* PFN_wl_proxy_marshal_constructor)(struct wl_proxy*,uint32_t,const struct wl_interface*,...);
typedef struct wl_proxy* (* PFN_wl_proxy_marshal_constructor_versioned)(struct wl_proxy*,uint32_t,const struct wl_interface*,uint32_t,...);
typedef void* (* PFN_wl_proxy_get_user_data)(struct wl_proxy*);
typedef void (* PFN_wl_proxy_set_user_data)(struct wl_proxy*,void*);
typedef void (* PFN_wl_proxy_set_tag)(struct wl_proxy*,const char*const*);
typedef const char* const* (* PFN_wl_proxy_get_tag)(struct wl_proxy*);
typedef uint32_t (* PFN_wl_proxy_get_version)(struct wl_proxy*);
typedef struct wl_proxy* (* PFN_wl_proxy_marshal_flags)(struct wl_proxy*,uint32_t,const struct wl_interface*,uint32_t,uint32_t,...);
#define wl_display_flush _glfw.wl.client.display_flush
#define wl_display_cancel_read _glfw.wl.client.display_cancel_read
#define wl_display_dispatch_pending _glfw.wl.client.display_dispatch_pending
#define wl_display_read_events _glfw.wl.client.display_read_events
#define wl_display_disconnect _glfw.wl.client.display_disconnect
#define wl_display_roundtrip _glfw.wl.client.display_roundtrip
#define wl_display_get_fd _glfw.wl.client.display_get_fd
#define wl_display_prepare_read _glfw.wl.client.display_prepare_read
#define wl_proxy_marshal _glfw.wl.client.proxy_marshal
#define wl_proxy_add_listener _glfw.wl.client.proxy_add_listener
#define wl_proxy_destroy _glfw.wl.client.proxy_destroy
#define wl_proxy_marshal_constructor _glfw.wl.client.proxy_marshal_constructor
#define wl_proxy_marshal_constructor_versioned _glfw.wl.client.proxy_marshal_constructor_versioned
#define wl_proxy_get_user_data _glfw.wl.client.proxy_get_user_data
#define wl_proxy_set_user_data _glfw.wl.client.proxy_set_user_data
#define wl_proxy_get_tag _glfw.wl.client.proxy_get_tag
#define wl_proxy_set_tag _glfw.wl.client.proxy_set_tag
#define wl_proxy_get_version _glfw.wl.client.proxy_get_version
#define wl_proxy_marshal_flags _glfw.wl.client.proxy_marshal_flags

struct wl_shm;
struct wl_output;

#define wl_display_interface _glfw_wl_display_interface
#define wl_subcompositor_interface _glfw_wl_subcompositor_interface
#define wl_compositor_interface _glfw_wl_compositor_interface
#define wl_shm_interface _glfw_wl_shm_interface
#define wl_data_device_manager_interface _glfw_wl_data_device_manager_interface
#define wl_shell_interface _glfw_wl_shell_interface
#define wl_buffer_interface _glfw_wl_buffer_interface
#define wl_callback_interface _glfw_wl_callback_interface
#define wl_data_device_interface _glfw_wl_data_device_interface
#define wl_data_offer_interface _glfw_wl_data_offer_interface
#define wl_data_source_interface _glfw_wl_data_source_interface
#define wl_keyboard_interface _glfw_wl_keyboard_interface
#define wl_output_interface _glfw_wl_output_interface
#define wl_pointer_interface _glfw_wl_pointer_interface
#define wl_region_interface _glfw_wl_region_interface
#define wl_registry_interface _glfw_wl_registry_interface
#define wl_seat_interface _glfw_wl_seat_interface
#define wl_shell_surface_interface _glfw_wl_shell_surface_interface
#define wl_shm_pool_interface _glfw_wl_shm_pool_interface
#define wl_subsurface_interface _glfw_wl_subsurface_interface
#define wl_surface_interface _glfw_wl_surface_interface
#define wl_touch_interface _glfw_wl_touch_interface
#define zwp_idle_inhibitor_v1_interface _glfw_zwp_idle_inhibitor_v1_interface
#define zwp_idle_inhibit_manager_v1_interface _glfw_zwp_idle_inhibit_manager_v1_interface
#define zwp_confined_pointer_v1_interface _glfw_zwp_confined_pointer_v1_interface
#define zwp_locked_pointer_v1_interface _glfw_zwp_locked_pointer_v1_interface
#define zwp_pointer_constraints_v1_interface _glfw_zwp_pointer_constraints_v1_interface
#define zwp_relative_pointer_v1_interface _glfw_zwp_relative_pointer_v1_interface
#define zwp_relative_pointer_manager_v1_interface _glfw_zwp_relative_pointer_manager_v1_interface
#define wp_viewport_interface _glfw_wp_viewport_interface
#define wp_viewporter_interface _glfw_wp_viewporter_interface
#define xdg_toplevel_interface _glfw_xdg_toplevel_interface
#define zxdg_toplevel_decoration_v1_interface _glfw_zxdg_toplevel_decoration_v1_interface
#define zxdg_decoration_manager_v1_interface _glfw_zxdg_decoration_manager_v1_interface
#define xdg_popup_interface _glfw_xdg_popup_interface
#define xdg_positioner_interface _glfw_xdg_positioner_interface
#define xdg_surface_interface _glfw_xdg_surface_interface
#define xdg_toplevel_interface _glfw_xdg_toplevel_interface
#define xdg_wm_base_interface _glfw_xdg_wm_base_interface
#define xdg_activation_v1_interface _glfw_xdg_activation_v1_interface
#define xdg_activation_token_v1_interface _glfw_xdg_activation_token_v1_interface
#define wl_surface_interface _glfw_wl_surface_interface
#define wp_fractional_scale_v1_interface _glfw_wp_fractional_scale_v1_interface

#define GLFW_WAYLAND_WINDOW_STATE         _GLFWwindowWayland  wl;
#define GLFW_WAYLAND_LIBRARY_WINDOW_STATE _GLFWlibraryWayland wl;
#define GLFW_WAYLAND_MONITOR_STATE        _GLFWmonitorWayland wl;
#define GLFW_WAYLAND_CURSOR_STATE         _GLFWcursorWayland  wl;

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

typedef struct _GLFWfallbackEdgeWayland
{
    struct wl_surface*          surface;
    struct wl_subsurface*       subsurface;
    struct wp_viewport*         viewport;
} _GLFWfallbackEdgeWayland;

typedef struct _GLFWofferWayland
{
    struct wl_data_offer*       offer;
    GLFWbool                    text_plain_utf8;
    GLFWbool                    text_uri_list;
} _GLFWofferWayland;

typedef struct _GLFWscaleWayland
{
    struct wl_output*           output;
    int32_t                     factor;
} _GLFWscaleWayland;

// Wayland-specific per-window data
//
typedef struct _GLFWwindowWayland
{
    int                         width, height;
    int                         fbWidth, fbHeight;
    GLFWbool                    visible;
    GLFWbool                    maximized;
    GLFWbool                    activated;
    GLFWbool                    fullscreen;
    GLFWbool                    hovered;
    GLFWbool                    transparent;
    GLFWbool                    scaleFramebuffer;
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
    } libdecor;

    _GLFWcursor*                currentCursor;
    double                      cursorPosX, cursorPosY;

    char*                       appId;

    // We need to track the monitors the window spans on to calculate the
    // optimal scaling factor.
    int32_t                     bufferScale;
    _GLFWscaleWayland*          outputScales;
    size_t                      outputScaleCount;
    size_t                      outputScaleSize;

    struct wp_viewport*             scalingViewport;
    uint32_t                        scalingNumerator;
    struct wp_fractional_scale_v1*  fractionalScale;

    struct zwp_relative_pointer_v1* relativePointer;
    struct zwp_locked_pointer_v1*   lockedPointer;
    struct zwp_confined_pointer_v1* confinedPointer;

    struct zwp_idle_inhibitor_v1*   idleInhibitor;
    struct xdg_activation_token_v1* activationToken;

    struct {
        GLFWbool                    decorations;
        struct wl_buffer*           buffer;
        _GLFWfallbackEdgeWayland    top, left, right, bottom;
        struct wl_surface*          focus;
    } fallback;
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
    struct xdg_activation_v1*               activationManager;
    struct wp_fractional_scale_manager_v1*  fractionalScaleManager;

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
        void*                                       handle;
        PFN_wl_display_flush                        display_flush;
        PFN_wl_display_cancel_read                  display_cancel_read;
        PFN_wl_display_dispatch_pending             display_dispatch_pending;
        PFN_wl_display_read_events                  display_read_events;
        PFN_wl_display_disconnect                   display_disconnect;
        PFN_wl_display_roundtrip                    display_roundtrip;
        PFN_wl_display_get_fd                       display_get_fd;
        PFN_wl_display_prepare_read                 display_prepare_read;
        PFN_wl_proxy_marshal                        proxy_marshal;
        PFN_wl_proxy_add_listener                   proxy_add_listener;
        PFN_wl_proxy_destroy                        proxy_destroy;
        PFN_wl_proxy_marshal_constructor            proxy_marshal_constructor;
        PFN_wl_proxy_marshal_constructor_versioned  proxy_marshal_constructor_versioned;
        PFN_wl_proxy_get_user_data                  proxy_get_user_data;
        PFN_wl_proxy_set_user_data                  proxy_set_user_data;
        PFN_wl_proxy_get_tag                        proxy_get_tag;
        PFN_wl_proxy_set_tag                        proxy_set_tag;
        PFN_wl_proxy_get_version                    proxy_get_version;
        PFN_wl_proxy_marshal_flags                  proxy_marshal_flags;
    } client;

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
    int32_t                     scale;
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

GLFWbool _glfwConnectWayland(int platformID, _GLFWplatform* platform);
int _glfwInitWayland(void);
void _glfwTerminateWayland(void);

GLFWbool _glfwCreateWindowWayland(_GLFWwindow* window, const _GLFWwndconfig* wndconfig, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig);
void _glfwDestroyWindowWayland(_GLFWwindow* window);
void _glfwSetWindowTitleWayland(_GLFWwindow* window, const char* title);
void _glfwSetWindowIconWayland(_GLFWwindow* window, int count, const GLFWimage* images);
void _glfwGetWindowPosWayland(_GLFWwindow* window, int* xpos, int* ypos);
void _glfwSetWindowPosWayland(_GLFWwindow* window, int xpos, int ypos);
void _glfwGetWindowSizeWayland(_GLFWwindow* window, int* width, int* height);
void _glfwSetWindowSizeWayland(_GLFWwindow* window, int width, int height);
void _glfwSetWindowSizeLimitsWayland(_GLFWwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _glfwSetWindowAspectRatioWayland(_GLFWwindow* window, int numer, int denom);
void _glfwGetFramebufferSizeWayland(_GLFWwindow* window, int* width, int* height);
void _glfwGetWindowFrameSizeWayland(_GLFWwindow* window, int* left, int* top, int* right, int* bottom);
void _glfwGetWindowContentScaleWayland(_GLFWwindow* window, float* xscale, float* yscale);
void _glfwIconifyWindowWayland(_GLFWwindow* window);
void _glfwRestoreWindowWayland(_GLFWwindow* window);
void _glfwMaximizeWindowWayland(_GLFWwindow* window);
void _glfwShowWindowWayland(_GLFWwindow* window);
void _glfwHideWindowWayland(_GLFWwindow* window);
void _glfwRequestWindowAttentionWayland(_GLFWwindow* window);
void _glfwFocusWindowWayland(_GLFWwindow* window);
void _glfwSetWindowMonitorWayland(_GLFWwindow* window, _GLFWmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
GLFWbool _glfwWindowFocusedWayland(_GLFWwindow* window);
GLFWbool _glfwWindowIconifiedWayland(_GLFWwindow* window);
GLFWbool _glfwWindowVisibleWayland(_GLFWwindow* window);
GLFWbool _glfwWindowMaximizedWayland(_GLFWwindow* window);
GLFWbool _glfwWindowHoveredWayland(_GLFWwindow* window);
GLFWbool _glfwFramebufferTransparentWayland(_GLFWwindow* window);
void _glfwSetWindowResizableWayland(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowDecoratedWayland(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowFloatingWayland(_GLFWwindow* window, GLFWbool enabled);
float _glfwGetWindowOpacityWayland(_GLFWwindow* window);
void _glfwSetWindowOpacityWayland(_GLFWwindow* window, float opacity);
void _glfwSetWindowMousePassthroughWayland(_GLFWwindow* window, GLFWbool enabled);

void _glfwSetRawMouseMotionWayland(_GLFWwindow* window, GLFWbool enabled);
GLFWbool _glfwRawMouseMotionSupportedWayland(void);

void _glfwPollEventsWayland(void);
void _glfwWaitEventsWayland(void);
void _glfwWaitEventsTimeoutWayland(double timeout);
void _glfwPostEmptyEventWayland(void);

void _glfwGetCursorPosWayland(_GLFWwindow* window, double* xpos, double* ypos);
void _glfwSetCursorPosWayland(_GLFWwindow* window, double xpos, double ypos);
void _glfwSetCursorModeWayland(_GLFWwindow* window, int mode);
const char* _glfwGetScancodeNameWayland(int scancode);
int _glfwGetKeyScancodeWayland(int key);
GLFWbool _glfwCreateCursorWayland(_GLFWcursor* cursor, const GLFWimage* image, int xhot, int yhot);
GLFWbool _glfwCreateStandardCursorWayland(_GLFWcursor* cursor, int shape);
void _glfwDestroyCursorWayland(_GLFWcursor* cursor);
void _glfwSetCursorWayland(_GLFWwindow* window, _GLFWcursor* cursor);
void _glfwSetClipboardStringWayland(const char* string);
const char* _glfwGetClipboardStringWayland(void);

EGLenum _glfwGetEGLPlatformWayland(EGLint** attribs);
EGLNativeDisplayType _glfwGetEGLNativeDisplayWayland(void);
EGLNativeWindowType _glfwGetEGLNativeWindowWayland(_GLFWwindow* window);

void _glfwGetRequiredInstanceExtensionsWayland(char** extensions);
GLFWbool _glfwGetPhysicalDevicePresentationSupportWayland(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
VkResult _glfwCreateWindowSurfaceWayland(VkInstance instance, _GLFWwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

void _glfwFreeMonitorWayland(_GLFWmonitor* monitor);
void _glfwGetMonitorPosWayland(_GLFWmonitor* monitor, int* xpos, int* ypos);
void _glfwGetMonitorContentScaleWayland(_GLFWmonitor* monitor, float* xscale, float* yscale);
void _glfwGetMonitorWorkareaWayland(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
GLFWvidmode* _glfwGetVideoModesWayland(_GLFWmonitor* monitor, int* count);
GLFWbool _glfwGetVideoModeWayland(_GLFWmonitor* monitor, GLFWvidmode* mode);
GLFWbool _glfwGetGammaRampWayland(_GLFWmonitor* monitor, GLFWgammaramp* ramp);
void _glfwSetGammaRampWayland(_GLFWmonitor* monitor, const GLFWgammaramp* ramp);

void _glfwAddOutputWayland(uint32_t name, uint32_t version);
void _glfwUpdateBufferScaleFromOutputsWayland(_GLFWwindow* window);

void _glfwAddSeatListenerWayland(struct wl_seat* seat);
void _glfwAddDataDeviceListenerWayland(struct wl_data_device* device);

