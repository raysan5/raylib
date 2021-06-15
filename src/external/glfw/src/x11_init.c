//========================================================================
// GLFW 3.4 X11 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <locale.h>
#include <unistd.h>


// Translate the X11 KeySyms for a key to a GLFW key code
// NOTE: This is only used as a fallback, in case the XKB method fails
//       It is layout-dependent and will fail partially on most non-US layouts
//
static int translateKeySyms(const KeySym* keysyms, int width)
{
    if (width > 1)
    {
        switch (keysyms[1])
        {
            case XK_KP_0:           return GLFW_KEY_KP_0;
            case XK_KP_1:           return GLFW_KEY_KP_1;
            case XK_KP_2:           return GLFW_KEY_KP_2;
            case XK_KP_3:           return GLFW_KEY_KP_3;
            case XK_KP_4:           return GLFW_KEY_KP_4;
            case XK_KP_5:           return GLFW_KEY_KP_5;
            case XK_KP_6:           return GLFW_KEY_KP_6;
            case XK_KP_7:           return GLFW_KEY_KP_7;
            case XK_KP_8:           return GLFW_KEY_KP_8;
            case XK_KP_9:           return GLFW_KEY_KP_9;
            case XK_KP_Separator:
            case XK_KP_Decimal:     return GLFW_KEY_KP_DECIMAL;
            case XK_KP_Equal:       return GLFW_KEY_KP_EQUAL;
            case XK_KP_Enter:       return GLFW_KEY_KP_ENTER;
            default:                break;
        }
    }

    switch (keysyms[0])
    {
        case XK_Escape:         return GLFW_KEY_ESCAPE;
        case XK_Tab:            return GLFW_KEY_TAB;
        case XK_Shift_L:        return GLFW_KEY_LEFT_SHIFT;
        case XK_Shift_R:        return GLFW_KEY_RIGHT_SHIFT;
        case XK_Control_L:      return GLFW_KEY_LEFT_CONTROL;
        case XK_Control_R:      return GLFW_KEY_RIGHT_CONTROL;
        case XK_Meta_L:
        case XK_Alt_L:          return GLFW_KEY_LEFT_ALT;
        case XK_Mode_switch: // Mapped to Alt_R on many keyboards
        case XK_ISO_Level3_Shift: // AltGr on at least some machines
        case XK_Meta_R:
        case XK_Alt_R:          return GLFW_KEY_RIGHT_ALT;
        case XK_Super_L:        return GLFW_KEY_LEFT_SUPER;
        case XK_Super_R:        return GLFW_KEY_RIGHT_SUPER;
        case XK_Menu:           return GLFW_KEY_MENU;
        case XK_Num_Lock:       return GLFW_KEY_NUM_LOCK;
        case XK_Caps_Lock:      return GLFW_KEY_CAPS_LOCK;
        case XK_Print:          return GLFW_KEY_PRINT_SCREEN;
        case XK_Scroll_Lock:    return GLFW_KEY_SCROLL_LOCK;
        case XK_Pause:          return GLFW_KEY_PAUSE;
        case XK_Delete:         return GLFW_KEY_DELETE;
        case XK_BackSpace:      return GLFW_KEY_BACKSPACE;
        case XK_Return:         return GLFW_KEY_ENTER;
        case XK_Home:           return GLFW_KEY_HOME;
        case XK_End:            return GLFW_KEY_END;
        case XK_Page_Up:        return GLFW_KEY_PAGE_UP;
        case XK_Page_Down:      return GLFW_KEY_PAGE_DOWN;
        case XK_Insert:         return GLFW_KEY_INSERT;
        case XK_Left:           return GLFW_KEY_LEFT;
        case XK_Right:          return GLFW_KEY_RIGHT;
        case XK_Down:           return GLFW_KEY_DOWN;
        case XK_Up:             return GLFW_KEY_UP;
        case XK_F1:             return GLFW_KEY_F1;
        case XK_F2:             return GLFW_KEY_F2;
        case XK_F3:             return GLFW_KEY_F3;
        case XK_F4:             return GLFW_KEY_F4;
        case XK_F5:             return GLFW_KEY_F5;
        case XK_F6:             return GLFW_KEY_F6;
        case XK_F7:             return GLFW_KEY_F7;
        case XK_F8:             return GLFW_KEY_F8;
        case XK_F9:             return GLFW_KEY_F9;
        case XK_F10:            return GLFW_KEY_F10;
        case XK_F11:            return GLFW_KEY_F11;
        case XK_F12:            return GLFW_KEY_F12;
        case XK_F13:            return GLFW_KEY_F13;
        case XK_F14:            return GLFW_KEY_F14;
        case XK_F15:            return GLFW_KEY_F15;
        case XK_F16:            return GLFW_KEY_F16;
        case XK_F17:            return GLFW_KEY_F17;
        case XK_F18:            return GLFW_KEY_F18;
        case XK_F19:            return GLFW_KEY_F19;
        case XK_F20:            return GLFW_KEY_F20;
        case XK_F21:            return GLFW_KEY_F21;
        case XK_F22:            return GLFW_KEY_F22;
        case XK_F23:            return GLFW_KEY_F23;
        case XK_F24:            return GLFW_KEY_F24;
        case XK_F25:            return GLFW_KEY_F25;

        // Numeric keypad
        case XK_KP_Divide:      return GLFW_KEY_KP_DIVIDE;
        case XK_KP_Multiply:    return GLFW_KEY_KP_MULTIPLY;
        case XK_KP_Subtract:    return GLFW_KEY_KP_SUBTRACT;
        case XK_KP_Add:         return GLFW_KEY_KP_ADD;

        // These should have been detected in secondary keysym test above!
        case XK_KP_Insert:      return GLFW_KEY_KP_0;
        case XK_KP_End:         return GLFW_KEY_KP_1;
        case XK_KP_Down:        return GLFW_KEY_KP_2;
        case XK_KP_Page_Down:   return GLFW_KEY_KP_3;
        case XK_KP_Left:        return GLFW_KEY_KP_4;
        case XK_KP_Right:       return GLFW_KEY_KP_6;
        case XK_KP_Home:        return GLFW_KEY_KP_7;
        case XK_KP_Up:          return GLFW_KEY_KP_8;
        case XK_KP_Page_Up:     return GLFW_KEY_KP_9;
        case XK_KP_Delete:      return GLFW_KEY_KP_DECIMAL;
        case XK_KP_Equal:       return GLFW_KEY_KP_EQUAL;
        case XK_KP_Enter:       return GLFW_KEY_KP_ENTER;

        // Last resort: Check for printable keys (should not happen if the XKB
        // extension is available). This will give a layout dependent mapping
        // (which is wrong, and we may miss some keys, especially on non-US
        // keyboards), but it's better than nothing...
        case XK_a:              return GLFW_KEY_A;
        case XK_b:              return GLFW_KEY_B;
        case XK_c:              return GLFW_KEY_C;
        case XK_d:              return GLFW_KEY_D;
        case XK_e:              return GLFW_KEY_E;
        case XK_f:              return GLFW_KEY_F;
        case XK_g:              return GLFW_KEY_G;
        case XK_h:              return GLFW_KEY_H;
        case XK_i:              return GLFW_KEY_I;
        case XK_j:              return GLFW_KEY_J;
        case XK_k:              return GLFW_KEY_K;
        case XK_l:              return GLFW_KEY_L;
        case XK_m:              return GLFW_KEY_M;
        case XK_n:              return GLFW_KEY_N;
        case XK_o:              return GLFW_KEY_O;
        case XK_p:              return GLFW_KEY_P;
        case XK_q:              return GLFW_KEY_Q;
        case XK_r:              return GLFW_KEY_R;
        case XK_s:              return GLFW_KEY_S;
        case XK_t:              return GLFW_KEY_T;
        case XK_u:              return GLFW_KEY_U;
        case XK_v:              return GLFW_KEY_V;
        case XK_w:              return GLFW_KEY_W;
        case XK_x:              return GLFW_KEY_X;
        case XK_y:              return GLFW_KEY_Y;
        case XK_z:              return GLFW_KEY_Z;
        case XK_1:              return GLFW_KEY_1;
        case XK_2:              return GLFW_KEY_2;
        case XK_3:              return GLFW_KEY_3;
        case XK_4:              return GLFW_KEY_4;
        case XK_5:              return GLFW_KEY_5;
        case XK_6:              return GLFW_KEY_6;
        case XK_7:              return GLFW_KEY_7;
        case XK_8:              return GLFW_KEY_8;
        case XK_9:              return GLFW_KEY_9;
        case XK_0:              return GLFW_KEY_0;
        case XK_space:          return GLFW_KEY_SPACE;
        case XK_minus:          return GLFW_KEY_MINUS;
        case XK_equal:          return GLFW_KEY_EQUAL;
        case XK_bracketleft:    return GLFW_KEY_LEFT_BRACKET;
        case XK_bracketright:   return GLFW_KEY_RIGHT_BRACKET;
        case XK_backslash:      return GLFW_KEY_BACKSLASH;
        case XK_semicolon:      return GLFW_KEY_SEMICOLON;
        case XK_apostrophe:     return GLFW_KEY_APOSTROPHE;
        case XK_grave:          return GLFW_KEY_GRAVE_ACCENT;
        case XK_comma:          return GLFW_KEY_COMMA;
        case XK_period:         return GLFW_KEY_PERIOD;
        case XK_slash:          return GLFW_KEY_SLASH;
        case XK_less:           return GLFW_KEY_WORLD_1; // At least in some layouts...
        default:                break;
    }

    // No matching translation was found
    return GLFW_KEY_UNKNOWN;
}

// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancode, scancodeMin, scancodeMax;

    memset(_glfw.x11.keycodes, -1, sizeof(_glfw.x11.keycodes));
    memset(_glfw.x11.scancodes, -1, sizeof(_glfw.x11.scancodes));

    if (_glfw.x11.xkb.available)
    {
        // Use XKB to determine physical key locations independently of the
        // current keyboard layout

        XkbDescPtr desc = XkbGetMap(_glfw.x11.display, 0, XkbUseCoreKbd);
        XkbGetNames(_glfw.x11.display, XkbKeyNamesMask | XkbKeyAliasesMask, desc);

        scancodeMin = desc->min_key_code;
        scancodeMax = desc->max_key_code;

        const struct
        {
            int key;
            char* name;
        } keymap[] =
        {
            { GLFW_KEY_GRAVE_ACCENT, "TLDE" },
            { GLFW_KEY_1, "AE01" },
            { GLFW_KEY_2, "AE02" },
            { GLFW_KEY_3, "AE03" },
            { GLFW_KEY_4, "AE04" },
            { GLFW_KEY_5, "AE05" },
            { GLFW_KEY_6, "AE06" },
            { GLFW_KEY_7, "AE07" },
            { GLFW_KEY_8, "AE08" },
            { GLFW_KEY_9, "AE09" },
            { GLFW_KEY_0, "AE10" },
            { GLFW_KEY_MINUS, "AE11" },
            { GLFW_KEY_EQUAL, "AE12" },
            { GLFW_KEY_Q, "AD01" },
            { GLFW_KEY_W, "AD02" },
            { GLFW_KEY_E, "AD03" },
            { GLFW_KEY_R, "AD04" },
            { GLFW_KEY_T, "AD05" },
            { GLFW_KEY_Y, "AD06" },
            { GLFW_KEY_U, "AD07" },
            { GLFW_KEY_I, "AD08" },
            { GLFW_KEY_O, "AD09" },
            { GLFW_KEY_P, "AD10" },
            { GLFW_KEY_LEFT_BRACKET, "AD11" },
            { GLFW_KEY_RIGHT_BRACKET, "AD12" },
            { GLFW_KEY_A, "AC01" },
            { GLFW_KEY_S, "AC02" },
            { GLFW_KEY_D, "AC03" },
            { GLFW_KEY_F, "AC04" },
            { GLFW_KEY_G, "AC05" },
            { GLFW_KEY_H, "AC06" },
            { GLFW_KEY_J, "AC07" },
            { GLFW_KEY_K, "AC08" },
            { GLFW_KEY_L, "AC09" },
            { GLFW_KEY_SEMICOLON, "AC10" },
            { GLFW_KEY_APOSTROPHE, "AC11" },
            { GLFW_KEY_Z, "AB01" },
            { GLFW_KEY_X, "AB02" },
            { GLFW_KEY_C, "AB03" },
            { GLFW_KEY_V, "AB04" },
            { GLFW_KEY_B, "AB05" },
            { GLFW_KEY_N, "AB06" },
            { GLFW_KEY_M, "AB07" },
            { GLFW_KEY_COMMA, "AB08" },
            { GLFW_KEY_PERIOD, "AB09" },
            { GLFW_KEY_SLASH, "AB10" },
            { GLFW_KEY_BACKSLASH, "BKSL" },
            { GLFW_KEY_WORLD_1, "LSGT" },
            { GLFW_KEY_SPACE, "SPCE" },
            { GLFW_KEY_ESCAPE, "ESC" },
            { GLFW_KEY_ENTER, "RTRN" },
            { GLFW_KEY_TAB, "TAB" },
            { GLFW_KEY_BACKSPACE, "BKSP" },
            { GLFW_KEY_INSERT, "INS" },
            { GLFW_KEY_DELETE, "DELE" },
            { GLFW_KEY_RIGHT, "RGHT" },
            { GLFW_KEY_LEFT, "LEFT" },
            { GLFW_KEY_DOWN, "DOWN" },
            { GLFW_KEY_UP, "UP" },
            { GLFW_KEY_PAGE_UP, "PGUP" },
            { GLFW_KEY_PAGE_DOWN, "PGDN" },
            { GLFW_KEY_HOME, "HOME" },
            { GLFW_KEY_END, "END" },
            { GLFW_KEY_CAPS_LOCK, "CAPS" },
            { GLFW_KEY_SCROLL_LOCK, "SCLK" },
            { GLFW_KEY_NUM_LOCK, "NMLK" },
            { GLFW_KEY_PRINT_SCREEN, "PRSC" },
            { GLFW_KEY_PAUSE, "PAUS" },
            { GLFW_KEY_F1, "FK01" },
            { GLFW_KEY_F2, "FK02" },
            { GLFW_KEY_F3, "FK03" },
            { GLFW_KEY_F4, "FK04" },
            { GLFW_KEY_F5, "FK05" },
            { GLFW_KEY_F6, "FK06" },
            { GLFW_KEY_F7, "FK07" },
            { GLFW_KEY_F8, "FK08" },
            { GLFW_KEY_F9, "FK09" },
            { GLFW_KEY_F10, "FK10" },
            { GLFW_KEY_F11, "FK11" },
            { GLFW_KEY_F12, "FK12" },
            { GLFW_KEY_F13, "FK13" },
            { GLFW_KEY_F14, "FK14" },
            { GLFW_KEY_F15, "FK15" },
            { GLFW_KEY_F16, "FK16" },
            { GLFW_KEY_F17, "FK17" },
            { GLFW_KEY_F18, "FK18" },
            { GLFW_KEY_F19, "FK19" },
            { GLFW_KEY_F20, "FK20" },
            { GLFW_KEY_F21, "FK21" },
            { GLFW_KEY_F22, "FK22" },
            { GLFW_KEY_F23, "FK23" },
            { GLFW_KEY_F24, "FK24" },
            { GLFW_KEY_F25, "FK25" },
            { GLFW_KEY_KP_0, "KP0" },
            { GLFW_KEY_KP_1, "KP1" },
            { GLFW_KEY_KP_2, "KP2" },
            { GLFW_KEY_KP_3, "KP3" },
            { GLFW_KEY_KP_4, "KP4" },
            { GLFW_KEY_KP_5, "KP5" },
            { GLFW_KEY_KP_6, "KP6" },
            { GLFW_KEY_KP_7, "KP7" },
            { GLFW_KEY_KP_8, "KP8" },
            { GLFW_KEY_KP_9, "KP9" },
            { GLFW_KEY_KP_DECIMAL, "KPDL" },
            { GLFW_KEY_KP_DIVIDE, "KPDV" },
            { GLFW_KEY_KP_MULTIPLY, "KPMU" },
            { GLFW_KEY_KP_SUBTRACT, "KPSU" },
            { GLFW_KEY_KP_ADD, "KPAD" },
            { GLFW_KEY_KP_ENTER, "KPEN" },
            { GLFW_KEY_KP_EQUAL, "KPEQ" },
            { GLFW_KEY_LEFT_SHIFT, "LFSH" },
            { GLFW_KEY_LEFT_CONTROL, "LCTL" },
            { GLFW_KEY_LEFT_ALT, "LALT" },
            { GLFW_KEY_LEFT_SUPER, "LWIN" },
            { GLFW_KEY_RIGHT_SHIFT, "RTSH" },
            { GLFW_KEY_RIGHT_CONTROL, "RCTL" },
            { GLFW_KEY_RIGHT_ALT, "RALT" },
            { GLFW_KEY_RIGHT_ALT, "LVL3" },
            { GLFW_KEY_RIGHT_ALT, "MDSW" },
            { GLFW_KEY_RIGHT_SUPER, "RWIN" },
            { GLFW_KEY_MENU, "MENU" }
        };

        // Find the X11 key code -> GLFW key code mapping
        for (scancode = scancodeMin;  scancode <= scancodeMax;  scancode++)
        {
            int key = GLFW_KEY_UNKNOWN;

            // Map the key name to a GLFW key code. Note: We use the US
            // keyboard layout. Because function keys aren't mapped correctly
            // when using traditional KeySym translations, they are mapped
            // here instead.
            for (int i = 0;  i < sizeof(keymap) / sizeof(keymap[0]);  i++)
            {
                if (strncmp(desc->names->keys[scancode].name,
                            keymap[i].name,
                            XkbKeyNameLength) == 0)
                {
                    key = keymap[i].key;
                    break;
                }
            }

            // Fall back to key aliases in case the key name did not match
            for (int i = 0;  i < desc->names->num_key_aliases;  i++)
            {
                if (key != GLFW_KEY_UNKNOWN)
                    break;

                if (strncmp(desc->names->key_aliases[i].real,
                            desc->names->keys[scancode].name,
                            XkbKeyNameLength) != 0)
                {
                    continue;
                }

                for (int j = 0;  j < sizeof(keymap) / sizeof(keymap[0]);  j++)
                {
                    if (strncmp(desc->names->key_aliases[i].alias,
                                keymap[j].name,
                                XkbKeyNameLength) == 0)
                    {
                        key = keymap[j].key;
                        break;
                    }
                }
            }

            _glfw.x11.keycodes[scancode] = key;
        }

        XkbFreeNames(desc, XkbKeyNamesMask, True);
        XkbFreeKeyboard(desc, 0, True);
    }
    else
        XDisplayKeycodes(_glfw.x11.display, &scancodeMin, &scancodeMax);

    int width;
    KeySym* keysyms = XGetKeyboardMapping(_glfw.x11.display,
                                          scancodeMin,
                                          scancodeMax - scancodeMin + 1,
                                          &width);

    for (scancode = scancodeMin;  scancode <= scancodeMax;  scancode++)
    {
        // Translate the un-translated key codes using traditional X11 KeySym
        // lookups
        if (_glfw.x11.keycodes[scancode] < 0)
        {
            const size_t base = (scancode - scancodeMin) * width;
            _glfw.x11.keycodes[scancode] = translateKeySyms(&keysyms[base], width);
        }

        // Store the reverse translation for faster key name lookup
        if (_glfw.x11.keycodes[scancode] > 0)
            _glfw.x11.scancodes[_glfw.x11.keycodes[scancode]] = scancode;
    }

    XFree(keysyms);
}

// Check whether the IM has a usable style
//
static GLFWbool hasUsableInputMethodStyle(void)
{
    GLFWbool found = GLFW_FALSE;
    XIMStyles* styles = NULL;

    if (XGetIMValues(_glfw.x11.im, XNQueryInputStyle, &styles, NULL) != NULL)
        return GLFW_FALSE;

    for (unsigned int i = 0;  i < styles->count_styles;  i++)
    {
        if (styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing))
        {
            found = GLFW_TRUE;
            break;
        }
    }

    XFree(styles);
    return found;
}

static void inputMethodDestroyCallback(XIM im, XPointer clientData, XPointer callData)
{
    _glfw.x11.im = NULL;
}

static void inputMethodInstantiateCallback(Display* display,
                                           XPointer clientData,
                                           XPointer callData)
{
    if (_glfw.x11.im)
        return;

    _glfw.x11.im = XOpenIM(_glfw.x11.display, 0, NULL, NULL);
    if (_glfw.x11.im)
    {
        if (!hasUsableInputMethodStyle())
        {
            XCloseIM(_glfw.x11.im);
            _glfw.x11.im = NULL;
        }
    }

    if (_glfw.x11.im)
    {
        XIMCallback callback;
        callback.callback = (XIMProc) inputMethodDestroyCallback;
        callback.client_data = NULL;
        XSetIMValues(_glfw.x11.im, XNDestroyCallback, &callback, NULL);

        for (_GLFWwindow* window = _glfw.windowListHead;  window;  window = window->next)
            _glfwCreateInputContextX11(window);
    }
}

// Return the atom ID only if it is listed in the specified array
//
static Atom getAtomIfSupported(Atom* supportedAtoms,
                               unsigned long atomCount,
                               const char* atomName)
{
    const Atom atom = XInternAtom(_glfw.x11.display, atomName, False);

    for (unsigned long i = 0;  i < atomCount;  i++)
    {
        if (supportedAtoms[i] == atom)
            return atom;
    }

    return None;
}

// Check whether the running window manager is EWMH-compliant
//
static void detectEWMH(void)
{
    // First we read the _NET_SUPPORTING_WM_CHECK property on the root window

    Window* windowFromRoot = NULL;
    if (!_glfwGetWindowPropertyX11(_glfw.x11.root,
                                   _glfw.x11.NET_SUPPORTING_WM_CHECK,
                                   XA_WINDOW,
                                   (unsigned char**) &windowFromRoot))
    {
        return;
    }

    _glfwGrabErrorHandlerX11();

    // If it exists, it should be the XID of a top-level window
    // Then we look for the same property on that window

    Window* windowFromChild = NULL;
    if (!_glfwGetWindowPropertyX11(*windowFromRoot,
                                   _glfw.x11.NET_SUPPORTING_WM_CHECK,
                                   XA_WINDOW,
                                   (unsigned char**) &windowFromChild))
    {
        XFree(windowFromRoot);
        return;
    }

    _glfwReleaseErrorHandlerX11();

    // If the property exists, it should contain the XID of the window

    if (*windowFromRoot != *windowFromChild)
    {
        XFree(windowFromRoot);
        XFree(windowFromChild);
        return;
    }

    XFree(windowFromRoot);
    XFree(windowFromChild);

    // We are now fairly sure that an EWMH-compliant WM is currently running
    // We can now start querying the WM about what features it supports by
    // looking in the _NET_SUPPORTED property on the root window
    // It should contain a list of supported EWMH protocol and state atoms

    Atom* supportedAtoms = NULL;
    const unsigned long atomCount =
        _glfwGetWindowPropertyX11(_glfw.x11.root,
                                  _glfw.x11.NET_SUPPORTED,
                                  XA_ATOM,
                                  (unsigned char**) &supportedAtoms);

    // See which of the atoms we support that are supported by the WM

    _glfw.x11.NET_WM_STATE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE");
    _glfw.x11.NET_WM_STATE_ABOVE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_ABOVE");
    _glfw.x11.NET_WM_STATE_FULLSCREEN =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_FULLSCREEN");
    _glfw.x11.NET_WM_STATE_MAXIMIZED_VERT =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_VERT");
    _glfw.x11.NET_WM_STATE_MAXIMIZED_HORZ =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_HORZ");
    _glfw.x11.NET_WM_STATE_DEMANDS_ATTENTION =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_DEMANDS_ATTENTION");
    _glfw.x11.NET_WM_FULLSCREEN_MONITORS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_FULLSCREEN_MONITORS");
    _glfw.x11.NET_WM_WINDOW_TYPE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE");
    _glfw.x11.NET_WM_WINDOW_TYPE_NORMAL =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE_NORMAL");
    _glfw.x11.NET_WORKAREA =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WORKAREA");
    _glfw.x11.NET_CURRENT_DESKTOP =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_CURRENT_DESKTOP");
    _glfw.x11.NET_ACTIVE_WINDOW =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_ACTIVE_WINDOW");
    _glfw.x11.NET_FRAME_EXTENTS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_FRAME_EXTENTS");
    _glfw.x11.NET_REQUEST_FRAME_EXTENTS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_REQUEST_FRAME_EXTENTS");

    if (supportedAtoms)
        XFree(supportedAtoms);
}

// Look for and initialize supported X11 extensions
//
static GLFWbool initExtensions(void)
{
    _glfw.x11.vidmode.handle = _glfw_dlopen("libXxf86vm.so.1");
    if (_glfw.x11.vidmode.handle)
    {
        _glfw.x11.vidmode.QueryExtension = (PFN_XF86VidModeQueryExtension)
            _glfw_dlsym(_glfw.x11.vidmode.handle, "XF86VidModeQueryExtension");
        _glfw.x11.vidmode.GetGammaRamp = (PFN_XF86VidModeGetGammaRamp)
            _glfw_dlsym(_glfw.x11.vidmode.handle, "XF86VidModeGetGammaRamp");
        _glfw.x11.vidmode.SetGammaRamp = (PFN_XF86VidModeSetGammaRamp)
            _glfw_dlsym(_glfw.x11.vidmode.handle, "XF86VidModeSetGammaRamp");
        _glfw.x11.vidmode.GetGammaRampSize = (PFN_XF86VidModeGetGammaRampSize)
            _glfw_dlsym(_glfw.x11.vidmode.handle, "XF86VidModeGetGammaRampSize");

        _glfw.x11.vidmode.available =
            XF86VidModeQueryExtension(_glfw.x11.display,
                                      &_glfw.x11.vidmode.eventBase,
                                      &_glfw.x11.vidmode.errorBase);
    }

#if defined(__CYGWIN__)
    _glfw.x11.xi.handle = _glfw_dlopen("libXi-6.so");
#else
    _glfw.x11.xi.handle = _glfw_dlopen("libXi.so.6");
#endif
    if (_glfw.x11.xi.handle)
    {
        _glfw.x11.xi.QueryVersion = (PFN_XIQueryVersion)
            _glfw_dlsym(_glfw.x11.xi.handle, "XIQueryVersion");
        _glfw.x11.xi.SelectEvents = (PFN_XISelectEvents)
            _glfw_dlsym(_glfw.x11.xi.handle, "XISelectEvents");

        if (XQueryExtension(_glfw.x11.display,
                            "XInputExtension",
                            &_glfw.x11.xi.majorOpcode,
                            &_glfw.x11.xi.eventBase,
                            &_glfw.x11.xi.errorBase))
        {
            _glfw.x11.xi.major = 2;
            _glfw.x11.xi.minor = 0;

            if (XIQueryVersion(_glfw.x11.display,
                               &_glfw.x11.xi.major,
                               &_glfw.x11.xi.minor) == Success)
            {
                _glfw.x11.xi.available = GLFW_TRUE;
            }
        }
    }

#if defined(__CYGWIN__)
    _glfw.x11.randr.handle = _glfw_dlopen("libXrandr-2.so");
#else
    _glfw.x11.randr.handle = _glfw_dlopen("libXrandr.so.2");
#endif
    if (_glfw.x11.randr.handle)
    {
        _glfw.x11.randr.AllocGamma = (PFN_XRRAllocGamma)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRAllocGamma");
        _glfw.x11.randr.FreeGamma = (PFN_XRRFreeGamma)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRFreeGamma");
        _glfw.x11.randr.FreeCrtcInfo = (PFN_XRRFreeCrtcInfo)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRFreeCrtcInfo");
        _glfw.x11.randr.FreeGamma = (PFN_XRRFreeGamma)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRFreeGamma");
        _glfw.x11.randr.FreeOutputInfo = (PFN_XRRFreeOutputInfo)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRFreeOutputInfo");
        _glfw.x11.randr.FreeScreenResources = (PFN_XRRFreeScreenResources)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRFreeScreenResources");
        _glfw.x11.randr.GetCrtcGamma = (PFN_XRRGetCrtcGamma)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRGetCrtcGamma");
        _glfw.x11.randr.GetCrtcGammaSize = (PFN_XRRGetCrtcGammaSize)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRGetCrtcGammaSize");
        _glfw.x11.randr.GetCrtcInfo = (PFN_XRRGetCrtcInfo)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRGetCrtcInfo");
        _glfw.x11.randr.GetOutputInfo = (PFN_XRRGetOutputInfo)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRGetOutputInfo");
        _glfw.x11.randr.GetOutputPrimary = (PFN_XRRGetOutputPrimary)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRGetOutputPrimary");
        _glfw.x11.randr.GetScreenResourcesCurrent = (PFN_XRRGetScreenResourcesCurrent)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRGetScreenResourcesCurrent");
        _glfw.x11.randr.QueryExtension = (PFN_XRRQueryExtension)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRQueryExtension");
        _glfw.x11.randr.QueryVersion = (PFN_XRRQueryVersion)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRQueryVersion");
        _glfw.x11.randr.SelectInput = (PFN_XRRSelectInput)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRSelectInput");
        _glfw.x11.randr.SetCrtcConfig = (PFN_XRRSetCrtcConfig)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRSetCrtcConfig");
        _glfw.x11.randr.SetCrtcGamma = (PFN_XRRSetCrtcGamma)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRSetCrtcGamma");
        _glfw.x11.randr.UpdateConfiguration = (PFN_XRRUpdateConfiguration)
            _glfw_dlsym(_glfw.x11.randr.handle, "XRRUpdateConfiguration");

        if (XRRQueryExtension(_glfw.x11.display,
                              &_glfw.x11.randr.eventBase,
                              &_glfw.x11.randr.errorBase))
        {
            if (XRRQueryVersion(_glfw.x11.display,
                                &_glfw.x11.randr.major,
                                &_glfw.x11.randr.minor))
            {
                // The GLFW RandR path requires at least version 1.3
                if (_glfw.x11.randr.major > 1 || _glfw.x11.randr.minor >= 3)
                    _glfw.x11.randr.available = GLFW_TRUE;
            }
            else
            {
                _glfwInputError(GLFW_PLATFORM_ERROR,
                                "X11: Failed to query RandR version");
            }
        }
    }

    if (_glfw.x11.randr.available)
    {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_glfw.x11.display,
                                                              _glfw.x11.root);

        if (!sr->ncrtc || !XRRGetCrtcGammaSize(_glfw.x11.display, sr->crtcs[0]))
        {
            // This is likely an older Nvidia driver with broken gamma support
            // Flag it as useless and fall back to xf86vm gamma, if available
            _glfw.x11.randr.gammaBroken = GLFW_TRUE;
        }

        if (!sr->ncrtc)
        {
            // A system without CRTCs is likely a system with broken RandR
            // Disable the RandR monitor path and fall back to core functions
            _glfw.x11.randr.monitorBroken = GLFW_TRUE;
        }

        XRRFreeScreenResources(sr);
    }

    if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken)
    {
        XRRSelectInput(_glfw.x11.display, _glfw.x11.root,
                       RROutputChangeNotifyMask);
    }

#if defined(__CYGWIN__)
    _glfw.x11.xcursor.handle = _glfw_dlopen("libXcursor-1.so");
#else
    _glfw.x11.xcursor.handle = _glfw_dlopen("libXcursor.so.1");
#endif
    if (_glfw.x11.xcursor.handle)
    {
        _glfw.x11.xcursor.ImageCreate = (PFN_XcursorImageCreate)
            _glfw_dlsym(_glfw.x11.xcursor.handle, "XcursorImageCreate");
        _glfw.x11.xcursor.ImageDestroy = (PFN_XcursorImageDestroy)
            _glfw_dlsym(_glfw.x11.xcursor.handle, "XcursorImageDestroy");
        _glfw.x11.xcursor.ImageLoadCursor = (PFN_XcursorImageLoadCursor)
            _glfw_dlsym(_glfw.x11.xcursor.handle, "XcursorImageLoadCursor");
        _glfw.x11.xcursor.GetTheme = (PFN_XcursorGetTheme)
            _glfw_dlsym(_glfw.x11.xcursor.handle, "XcursorGetTheme");
        _glfw.x11.xcursor.GetDefaultSize = (PFN_XcursorGetDefaultSize)
            _glfw_dlsym(_glfw.x11.xcursor.handle, "XcursorGetDefaultSize");
        _glfw.x11.xcursor.LibraryLoadImage = (PFN_XcursorLibraryLoadImage)
            _glfw_dlsym(_glfw.x11.xcursor.handle, "XcursorLibraryLoadImage");
    }

#if defined(__CYGWIN__)
    _glfw.x11.xinerama.handle = _glfw_dlopen("libXinerama-1.so");
#else
    _glfw.x11.xinerama.handle = _glfw_dlopen("libXinerama.so.1");
#endif
    if (_glfw.x11.xinerama.handle)
    {
        _glfw.x11.xinerama.IsActive = (PFN_XineramaIsActive)
            _glfw_dlsym(_glfw.x11.xinerama.handle, "XineramaIsActive");
        _glfw.x11.xinerama.QueryExtension = (PFN_XineramaQueryExtension)
            _glfw_dlsym(_glfw.x11.xinerama.handle, "XineramaQueryExtension");
        _glfw.x11.xinerama.QueryScreens = (PFN_XineramaQueryScreens)
            _glfw_dlsym(_glfw.x11.xinerama.handle, "XineramaQueryScreens");

        if (XineramaQueryExtension(_glfw.x11.display,
                                   &_glfw.x11.xinerama.major,
                                   &_glfw.x11.xinerama.minor))
        {
            if (XineramaIsActive(_glfw.x11.display))
                _glfw.x11.xinerama.available = GLFW_TRUE;
        }
    }

    _glfw.x11.xkb.major = 1;
    _glfw.x11.xkb.minor = 0;
    _glfw.x11.xkb.available =
        XkbQueryExtension(_glfw.x11.display,
                          &_glfw.x11.xkb.majorOpcode,
                          &_glfw.x11.xkb.eventBase,
                          &_glfw.x11.xkb.errorBase,
                          &_glfw.x11.xkb.major,
                          &_glfw.x11.xkb.minor);

    if (_glfw.x11.xkb.available)
    {
        Bool supported;

        if (XkbSetDetectableAutoRepeat(_glfw.x11.display, True, &supported))
        {
            if (supported)
                _glfw.x11.xkb.detectable = GLFW_TRUE;
        }

        XkbStateRec state;
        if (XkbGetState(_glfw.x11.display, XkbUseCoreKbd, &state) == Success)
            _glfw.x11.xkb.group = (unsigned int)state.group;

        XkbSelectEventDetails(_glfw.x11.display, XkbUseCoreKbd, XkbStateNotify,
                              XkbGroupStateMask, XkbGroupStateMask);
    }

    if (_glfw.hints.init.x11.xcbVulkanSurface)
    {
#if defined(__CYGWIN__)
        _glfw.x11.x11xcb.handle = _glfw_dlopen("libX11-xcb-1.so");
#else
        _glfw.x11.x11xcb.handle = _glfw_dlopen("libX11-xcb.so.1");
#endif
    }

    if (_glfw.x11.x11xcb.handle)
    {
        _glfw.x11.x11xcb.GetXCBConnection = (PFN_XGetXCBConnection)
            _glfw_dlsym(_glfw.x11.x11xcb.handle, "XGetXCBConnection");
    }

#if defined(__CYGWIN__)
    _glfw.x11.xrender.handle = _glfw_dlopen("libXrender-1.so");
#else
    _glfw.x11.xrender.handle = _glfw_dlopen("libXrender.so.1");
#endif
    if (_glfw.x11.xrender.handle)
    {
        _glfw.x11.xrender.QueryExtension = (PFN_XRenderQueryExtension)
            _glfw_dlsym(_glfw.x11.xrender.handle, "XRenderQueryExtension");
        _glfw.x11.xrender.QueryVersion = (PFN_XRenderQueryVersion)
            _glfw_dlsym(_glfw.x11.xrender.handle, "XRenderQueryVersion");
        _glfw.x11.xrender.FindVisualFormat = (PFN_XRenderFindVisualFormat)
            _glfw_dlsym(_glfw.x11.xrender.handle, "XRenderFindVisualFormat");

        if (XRenderQueryExtension(_glfw.x11.display,
                                  &_glfw.x11.xrender.errorBase,
                                  &_glfw.x11.xrender.eventBase))
        {
            if (XRenderQueryVersion(_glfw.x11.display,
                                    &_glfw.x11.xrender.major,
                                    &_glfw.x11.xrender.minor))
            {
                _glfw.x11.xrender.available = GLFW_TRUE;
            }
        }
    }

#if defined(__CYGWIN__)
    _glfw.x11.xshape.handle = _glfw_dlopen("libXext-6.so");
#else
    _glfw.x11.xshape.handle = _glfw_dlopen("libXext.so.6");
#endif
    if (_glfw.x11.xshape.handle)
    {
        _glfw.x11.xshape.QueryExtension = (PFN_XShapeQueryExtension)
            _glfw_dlsym(_glfw.x11.xshape.handle, "XShapeQueryExtension");
        _glfw.x11.xshape.ShapeCombineRegion = (PFN_XShapeCombineRegion)
            _glfw_dlsym(_glfw.x11.xshape.handle, "XShapeCombineRegion");
        _glfw.x11.xshape.QueryVersion = (PFN_XShapeQueryVersion)
            _glfw_dlsym(_glfw.x11.xshape.handle, "XShapeQueryVersion");
        _glfw.x11.xshape.ShapeCombineMask = (PFN_XShapeCombineMask)
            _glfw_dlsym(_glfw.x11.xshape.handle, "XShapeCombineMask");

        if (XShapeQueryExtension(_glfw.x11.display,
            &_glfw.x11.xshape.errorBase,
            &_glfw.x11.xshape.eventBase))
        {
            if (XShapeQueryVersion(_glfw.x11.display,
                &_glfw.x11.xshape.major,
                &_glfw.x11.xshape.minor))
            {
                _glfw.x11.xshape.available = GLFW_TRUE;
            }
        }
    }

    // Update the key code LUT
    // FIXME: We should listen to XkbMapNotify events to track changes to
    // the keyboard mapping.
    createKeyTables();

    // String format atoms
    _glfw.x11.NULL_ = XInternAtom(_glfw.x11.display, "NULL", False);
    _glfw.x11.UTF8_STRING = XInternAtom(_glfw.x11.display, "UTF8_STRING", False);
    _glfw.x11.ATOM_PAIR = XInternAtom(_glfw.x11.display, "ATOM_PAIR", False);

    // Custom selection property atom
    _glfw.x11.GLFW_SELECTION =
        XInternAtom(_glfw.x11.display, "GLFW_SELECTION", False);

    // ICCCM standard clipboard atoms
    _glfw.x11.TARGETS = XInternAtom(_glfw.x11.display, "TARGETS", False);
    _glfw.x11.MULTIPLE = XInternAtom(_glfw.x11.display, "MULTIPLE", False);
    _glfw.x11.PRIMARY = XInternAtom(_glfw.x11.display, "PRIMARY", False);
    _glfw.x11.INCR = XInternAtom(_glfw.x11.display, "INCR", False);
    _glfw.x11.CLIPBOARD = XInternAtom(_glfw.x11.display, "CLIPBOARD", False);

    // Clipboard manager atoms
    _glfw.x11.CLIPBOARD_MANAGER =
        XInternAtom(_glfw.x11.display, "CLIPBOARD_MANAGER", False);
    _glfw.x11.SAVE_TARGETS =
        XInternAtom(_glfw.x11.display, "SAVE_TARGETS", False);

    // Xdnd (drag and drop) atoms
    _glfw.x11.XdndAware = XInternAtom(_glfw.x11.display, "XdndAware", False);
    _glfw.x11.XdndEnter = XInternAtom(_glfw.x11.display, "XdndEnter", False);
    _glfw.x11.XdndPosition = XInternAtom(_glfw.x11.display, "XdndPosition", False);
    _glfw.x11.XdndStatus = XInternAtom(_glfw.x11.display, "XdndStatus", False);
    _glfw.x11.XdndActionCopy = XInternAtom(_glfw.x11.display, "XdndActionCopy", False);
    _glfw.x11.XdndDrop = XInternAtom(_glfw.x11.display, "XdndDrop", False);
    _glfw.x11.XdndFinished = XInternAtom(_glfw.x11.display, "XdndFinished", False);
    _glfw.x11.XdndSelection = XInternAtom(_glfw.x11.display, "XdndSelection", False);
    _glfw.x11.XdndTypeList = XInternAtom(_glfw.x11.display, "XdndTypeList", False);
    _glfw.x11.text_uri_list = XInternAtom(_glfw.x11.display, "text/uri-list", False);

    // ICCCM, EWMH and Motif window property atoms
    // These can be set safely even without WM support
    // The EWMH atoms that require WM support are handled in detectEWMH
    _glfw.x11.WM_PROTOCOLS =
        XInternAtom(_glfw.x11.display, "WM_PROTOCOLS", False);
    _glfw.x11.WM_STATE =
        XInternAtom(_glfw.x11.display, "WM_STATE", False);
    _glfw.x11.WM_DELETE_WINDOW =
        XInternAtom(_glfw.x11.display, "WM_DELETE_WINDOW", False);
    _glfw.x11.NET_SUPPORTED =
        XInternAtom(_glfw.x11.display, "_NET_SUPPORTED", False);
    _glfw.x11.NET_SUPPORTING_WM_CHECK =
        XInternAtom(_glfw.x11.display, "_NET_SUPPORTING_WM_CHECK", False);
    _glfw.x11.NET_WM_ICON =
        XInternAtom(_glfw.x11.display, "_NET_WM_ICON", False);
    _glfw.x11.NET_WM_PING =
        XInternAtom(_glfw.x11.display, "_NET_WM_PING", False);
    _glfw.x11.NET_WM_PID =
        XInternAtom(_glfw.x11.display, "_NET_WM_PID", False);
    _glfw.x11.NET_WM_NAME =
        XInternAtom(_glfw.x11.display, "_NET_WM_NAME", False);
    _glfw.x11.NET_WM_ICON_NAME =
        XInternAtom(_glfw.x11.display, "_NET_WM_ICON_NAME", False);
    _glfw.x11.NET_WM_BYPASS_COMPOSITOR =
        XInternAtom(_glfw.x11.display, "_NET_WM_BYPASS_COMPOSITOR", False);
    _glfw.x11.NET_WM_WINDOW_OPACITY =
        XInternAtom(_glfw.x11.display, "_NET_WM_WINDOW_OPACITY", False);
    _glfw.x11.MOTIF_WM_HINTS =
        XInternAtom(_glfw.x11.display, "_MOTIF_WM_HINTS", False);

    // The compositing manager selection name contains the screen number
    {
        char name[32];
        snprintf(name, sizeof(name), "_NET_WM_CM_S%u", _glfw.x11.screen);
        _glfw.x11.NET_WM_CM_Sx = XInternAtom(_glfw.x11.display, name, False);
    }

    // Detect whether an EWMH-conformant window manager is running
    detectEWMH();

    return GLFW_TRUE;
}

// Retrieve system content scale via folklore heuristics
//
static void getSystemContentScale(float* xscale, float* yscale)
{
    // Start by assuming the default X11 DPI
    // NOTE: Some desktop environments (KDE) may remove the Xft.dpi field when it
    //       would be set to 96, so assume that is the case if we cannot find it
    float xdpi = 96.f, ydpi = 96.f;

    // NOTE: Basing the scale on Xft.dpi where available should provide the most
    //       consistent user experience (matches Qt, Gtk, etc), although not
    //       always the most accurate one
    char* rms = XResourceManagerString(_glfw.x11.display);
    if (rms)
    {
        XrmDatabase db = XrmGetStringDatabase(rms);
        if (db)
        {
            XrmValue value;
            char* type = NULL;

            if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value))
            {
                if (type && strcmp(type, "String") == 0)
                    xdpi = ydpi = atof(value.addr);
            }

            XrmDestroyDatabase(db);
        }
    }

    *xscale = xdpi / 96.f;
    *yscale = ydpi / 96.f;
}

// Create a blank cursor for hidden and disabled cursor modes
//
static Cursor createHiddenCursor(void)
{
    unsigned char pixels[16 * 16 * 4] = { 0 };
    GLFWimage image = { 16, 16, pixels };
    return _glfwCreateCursorX11(&image, 0, 0);
}

// Create a helper window for IPC
//
static Window createHelperWindow(void)
{
    XSetWindowAttributes wa;
    wa.event_mask = PropertyChangeMask;

    return XCreateWindow(_glfw.x11.display, _glfw.x11.root,
                         0, 0, 1, 1, 0, 0,
                         InputOnly,
                         DefaultVisual(_glfw.x11.display, _glfw.x11.screen),
                         CWEventMask, &wa);
}

// X error handler
//
static int errorHandler(Display *display, XErrorEvent* event)
{
    if (_glfw.x11.display != display)
        return 0;

    _glfw.x11.errorCode = event->error_code;
    return 0;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Sets the X error handler callback
//
void _glfwGrabErrorHandlerX11(void)
{
    _glfw.x11.errorCode = Success;
    XSetErrorHandler(errorHandler);
}

// Clears the X error handler callback
//
void _glfwReleaseErrorHandlerX11(void)
{
    // Synchronize to make sure all commands are processed
    XSync(_glfw.x11.display, False);
    XSetErrorHandler(NULL);
}

// Reports the specified error, appending information about the last X error
//
void _glfwInputErrorX11(int error, const char* message)
{
    char buffer[_GLFW_MESSAGE_SIZE];
    XGetErrorText(_glfw.x11.display, _glfw.x11.errorCode,
                  buffer, sizeof(buffer));

    _glfwInputError(error, "%s: %s", message, buffer);
}

// Creates a native cursor object from the specified image and hotspot
//
Cursor _glfwCreateCursorX11(const GLFWimage* image, int xhot, int yhot)
{
    int i;
    Cursor cursor;

    if (!_glfw.x11.xcursor.handle)
        return None;

    XcursorImage* native = XcursorImageCreate(image->width, image->height);
    if (native == NULL)
        return None;

    native->xhot = xhot;
    native->yhot = yhot;

    unsigned char* source = (unsigned char*) image->pixels;
    XcursorPixel* target = native->pixels;

    for (i = 0;  i < image->width * image->height;  i++, target++, source += 4)
    {
        unsigned int alpha = source[3];

        *target = (alpha << 24) |
                  ((unsigned char) ((source[0] * alpha) / 255) << 16) |
                  ((unsigned char) ((source[1] * alpha) / 255) <<  8) |
                  ((unsigned char) ((source[2] * alpha) / 255) <<  0);
    }

    cursor = XcursorImageLoadCursor(_glfw.x11.display, native);
    XcursorImageDestroy(native);

    return cursor;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformInit(void)
{
    // HACK: If the application has left the locale as "C" then both wide
    //       character text input and explicit UTF-8 input via XIM will break
    //       This sets the CTYPE part of the current locale from the environment
    //       in the hope that it is set to something more sane than "C"
    if (strcmp(setlocale(LC_CTYPE, NULL), "C") == 0)
        setlocale(LC_CTYPE, "");

#if defined(__CYGWIN__)
    _glfw.x11.xlib.handle = _glfw_dlopen("libX11-6.so");
#else
    _glfw.x11.xlib.handle = _glfw_dlopen("libX11.so.6");
#endif
    if (!_glfw.x11.xlib.handle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to load Xlib");
        return GLFW_FALSE;
    }

    _glfw.x11.xlib.AllocClassHint = (PFN_XAllocClassHint)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XAllocClassHint");
    _glfw.x11.xlib.AllocSizeHints = (PFN_XAllocSizeHints)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XAllocSizeHints");
    _glfw.x11.xlib.AllocWMHints = (PFN_XAllocWMHints)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XAllocWMHints");
    _glfw.x11.xlib.ChangeProperty = (PFN_XChangeProperty)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XChangeProperty");
    _glfw.x11.xlib.ChangeWindowAttributes = (PFN_XChangeWindowAttributes)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XChangeWindowAttributes");
    _glfw.x11.xlib.CheckIfEvent = (PFN_XCheckIfEvent)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XCheckIfEvent");
    _glfw.x11.xlib.CheckTypedWindowEvent = (PFN_XCheckTypedWindowEvent)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XCheckTypedWindowEvent");
    _glfw.x11.xlib.CloseDisplay = (PFN_XCloseDisplay)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XCloseDisplay");
    _glfw.x11.xlib.CloseIM = (PFN_XCloseIM)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XCloseIM");
    _glfw.x11.xlib.ConvertSelection = (PFN_XConvertSelection)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XConvertSelection");
    _glfw.x11.xlib.CreateColormap = (PFN_XCreateColormap)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XCreateColormap");
    _glfw.x11.xlib.CreateFontCursor = (PFN_XCreateFontCursor)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XCreateFontCursor");
    _glfw.x11.xlib.CreateIC = (PFN_XCreateIC)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XCreateIC");
    _glfw.x11.xlib.CreateRegion = (PFN_XCreateRegion)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XCreateRegion");
    _glfw.x11.xlib.CreateWindow = (PFN_XCreateWindow)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XCreateWindow");
    _glfw.x11.xlib.DefineCursor = (PFN_XDefineCursor)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XDefineCursor");
    _glfw.x11.xlib.DeleteContext = (PFN_XDeleteContext)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XDeleteContext");
    _glfw.x11.xlib.DeleteProperty = (PFN_XDeleteProperty)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XDeleteProperty");
    _glfw.x11.xlib.DestroyIC = (PFN_XDestroyIC)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XDestroyIC");
    _glfw.x11.xlib.DestroyRegion = (PFN_XDestroyRegion)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XDestroyRegion");
    _glfw.x11.xlib.DestroyWindow = (PFN_XDestroyWindow)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XDestroyWindow");
    _glfw.x11.xlib.DisplayKeycodes = (PFN_XDisplayKeycodes)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XDisplayKeycodes");
    _glfw.x11.xlib.EventsQueued = (PFN_XEventsQueued)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XEventsQueued");
    _glfw.x11.xlib.FilterEvent = (PFN_XFilterEvent)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XFilterEvent");
    _glfw.x11.xlib.FindContext = (PFN_XFindContext)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XFindContext");
    _glfw.x11.xlib.Flush = (PFN_XFlush)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XFlush");
    _glfw.x11.xlib.Free = (PFN_XFree)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XFree");
    _glfw.x11.xlib.FreeColormap = (PFN_XFreeColormap)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XFreeColormap");
    _glfw.x11.xlib.FreeCursor = (PFN_XFreeCursor)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XFreeCursor");
    _glfw.x11.xlib.FreeEventData = (PFN_XFreeEventData)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XFreeEventData");
    _glfw.x11.xlib.GetErrorText = (PFN_XGetErrorText)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetErrorText");
    _glfw.x11.xlib.GetEventData = (PFN_XGetEventData)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetEventData");
    _glfw.x11.xlib.GetICValues = (PFN_XGetICValues)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetICValues");
    _glfw.x11.xlib.GetIMValues = (PFN_XGetIMValues)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetIMValues");
    _glfw.x11.xlib.GetInputFocus = (PFN_XGetInputFocus)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetInputFocus");
    _glfw.x11.xlib.GetKeyboardMapping = (PFN_XGetKeyboardMapping)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetKeyboardMapping");
    _glfw.x11.xlib.GetScreenSaver = (PFN_XGetScreenSaver)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetScreenSaver");
    _glfw.x11.xlib.GetSelectionOwner = (PFN_XGetSelectionOwner)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetSelectionOwner");
    _glfw.x11.xlib.GetVisualInfo = (PFN_XGetVisualInfo)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetVisualInfo");
    _glfw.x11.xlib.GetWMNormalHints = (PFN_XGetWMNormalHints)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetWMNormalHints");
    _glfw.x11.xlib.GetWindowAttributes = (PFN_XGetWindowAttributes)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetWindowAttributes");
    _glfw.x11.xlib.GetWindowProperty = (PFN_XGetWindowProperty)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGetWindowProperty");
    _glfw.x11.xlib.GrabPointer = (PFN_XGrabPointer)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XGrabPointer");
    _glfw.x11.xlib.IconifyWindow = (PFN_XIconifyWindow)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XIconifyWindow");
    _glfw.x11.xlib.InitThreads = (PFN_XInitThreads)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XInitThreads");
    _glfw.x11.xlib.InternAtom = (PFN_XInternAtom)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XInternAtom");
    _glfw.x11.xlib.LookupString = (PFN_XLookupString)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XLookupString");
    _glfw.x11.xlib.MapRaised = (PFN_XMapRaised)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XMapRaised");
    _glfw.x11.xlib.MapWindow = (PFN_XMapWindow)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XMapWindow");
    _glfw.x11.xlib.MoveResizeWindow = (PFN_XMoveResizeWindow)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XMoveResizeWindow");
    _glfw.x11.xlib.MoveWindow = (PFN_XMoveWindow)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XMoveWindow");
    _glfw.x11.xlib.NextEvent = (PFN_XNextEvent)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XNextEvent");
    _glfw.x11.xlib.OpenDisplay = (PFN_XOpenDisplay)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XOpenDisplay");
    _glfw.x11.xlib.OpenIM = (PFN_XOpenIM)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XOpenIM");
    _glfw.x11.xlib.PeekEvent = (PFN_XPeekEvent)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XPeekEvent");
    _glfw.x11.xlib.Pending = (PFN_XPending)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XPending");
    _glfw.x11.xlib.QueryExtension = (PFN_XQueryExtension)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XQueryExtension");
    _glfw.x11.xlib.QueryPointer = (PFN_XQueryPointer)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XQueryPointer");
    _glfw.x11.xlib.RaiseWindow = (PFN_XRaiseWindow)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XRaiseWindow");
    _glfw.x11.xlib.RegisterIMInstantiateCallback = (PFN_XRegisterIMInstantiateCallback)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XRegisterIMInstantiateCallback");
    _glfw.x11.xlib.ResizeWindow = (PFN_XResizeWindow)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XResizeWindow");
    _glfw.x11.xlib.ResourceManagerString = (PFN_XResourceManagerString)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XResourceManagerString");
    _glfw.x11.xlib.SaveContext = (PFN_XSaveContext)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSaveContext");
    _glfw.x11.xlib.SelectInput = (PFN_XSelectInput)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSelectInput");
    _glfw.x11.xlib.SendEvent = (PFN_XSendEvent)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSendEvent");
    _glfw.x11.xlib.SetClassHint = (PFN_XSetClassHint)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSetClassHint");
    _glfw.x11.xlib.SetErrorHandler = (PFN_XSetErrorHandler)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSetErrorHandler");
    _glfw.x11.xlib.SetICFocus = (PFN_XSetICFocus)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSetICFocus");
    _glfw.x11.xlib.SetIMValues = (PFN_XSetIMValues)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSetIMValues");
    _glfw.x11.xlib.SetInputFocus = (PFN_XSetInputFocus)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSetInputFocus");
    _glfw.x11.xlib.SetLocaleModifiers = (PFN_XSetLocaleModifiers)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSetLocaleModifiers");
    _glfw.x11.xlib.SetScreenSaver = (PFN_XSetScreenSaver)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSetScreenSaver");
    _glfw.x11.xlib.SetSelectionOwner = (PFN_XSetSelectionOwner)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSetSelectionOwner");
    _glfw.x11.xlib.SetWMHints = (PFN_XSetWMHints)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSetWMHints");
    _glfw.x11.xlib.SetWMNormalHints = (PFN_XSetWMNormalHints)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSetWMNormalHints");
    _glfw.x11.xlib.SetWMProtocols = (PFN_XSetWMProtocols)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSetWMProtocols");
    _glfw.x11.xlib.SupportsLocale = (PFN_XSupportsLocale)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSupportsLocale");
    _glfw.x11.xlib.Sync = (PFN_XSync)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XSync");
    _glfw.x11.xlib.TranslateCoordinates = (PFN_XTranslateCoordinates)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XTranslateCoordinates");
    _glfw.x11.xlib.UndefineCursor = (PFN_XUndefineCursor)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XUndefineCursor");
    _glfw.x11.xlib.UngrabPointer = (PFN_XUngrabPointer)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XUngrabPointer");
    _glfw.x11.xlib.UnmapWindow = (PFN_XUnmapWindow)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XUnmapWindow");
    _glfw.x11.xlib.UnsetICFocus = (PFN_XUnsetICFocus)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XUnsetICFocus");
    _glfw.x11.xlib.VisualIDFromVisual = (PFN_XVisualIDFromVisual)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XVisualIDFromVisual");
    _glfw.x11.xlib.WarpPointer = (PFN_XWarpPointer)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XWarpPointer");
    _glfw.x11.xkb.FreeKeyboard = (PFN_XkbFreeKeyboard)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XkbFreeKeyboard");
    _glfw.x11.xkb.FreeNames = (PFN_XkbFreeNames)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XkbFreeNames");
    _glfw.x11.xkb.GetMap = (PFN_XkbGetMap)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XkbGetMap");
    _glfw.x11.xkb.GetNames = (PFN_XkbGetNames)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XkbGetNames");
    _glfw.x11.xkb.GetState = (PFN_XkbGetState)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XkbGetState");
    _glfw.x11.xkb.KeycodeToKeysym = (PFN_XkbKeycodeToKeysym)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XkbKeycodeToKeysym");
    _glfw.x11.xkb.QueryExtension = (PFN_XkbQueryExtension)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XkbQueryExtension");
    _glfw.x11.xkb.SelectEventDetails = (PFN_XkbSelectEventDetails)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XkbSelectEventDetails");
    _glfw.x11.xkb.SetDetectableAutoRepeat = (PFN_XkbSetDetectableAutoRepeat)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XkbSetDetectableAutoRepeat");
    _glfw.x11.xrm.DestroyDatabase = (PFN_XrmDestroyDatabase)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XrmDestroyDatabase");
    _glfw.x11.xrm.GetResource = (PFN_XrmGetResource)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XrmGetResource");
    _glfw.x11.xrm.GetStringDatabase = (PFN_XrmGetStringDatabase)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XrmGetStringDatabase");
    _glfw.x11.xrm.Initialize = (PFN_XrmInitialize)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XrmInitialize");
    _glfw.x11.xrm.UniqueQuark = (PFN_XrmUniqueQuark)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XrmUniqueQuark");
    _glfw.x11.xlib.UnregisterIMInstantiateCallback = (PFN_XUnregisterIMInstantiateCallback)
        _glfw_dlsym(_glfw.x11.xlib.handle, "XUnregisterIMInstantiateCallback");
    _glfw.x11.xlib.utf8LookupString = (PFN_Xutf8LookupString)
        _glfw_dlsym(_glfw.x11.xlib.handle, "Xutf8LookupString");
    _glfw.x11.xlib.utf8SetWMProperties = (PFN_Xutf8SetWMProperties)
        _glfw_dlsym(_glfw.x11.xlib.handle, "Xutf8SetWMProperties");

    if (_glfw.x11.xlib.utf8LookupString && _glfw.x11.xlib.utf8SetWMProperties)
        _glfw.x11.xlib.utf8 = GLFW_TRUE;

    XInitThreads();
    XrmInitialize();

    _glfw.x11.display = XOpenDisplay(NULL);
    if (!_glfw.x11.display)
    {
        const char* display = getenv("DISPLAY");
        if (display)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "X11: Failed to open display %s", display);
        }
        else
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "X11: The DISPLAY environment variable is missing");
        }

        return GLFW_FALSE;
    }

    _glfw.x11.screen = DefaultScreen(_glfw.x11.display);
    _glfw.x11.root = RootWindow(_glfw.x11.display, _glfw.x11.screen);
    _glfw.x11.context = XUniqueContext();

    getSystemContentScale(&_glfw.x11.contentScaleX, &_glfw.x11.contentScaleY);

    if (!initExtensions())
        return GLFW_FALSE;

    _glfw.x11.helperWindowHandle = createHelperWindow();
    _glfw.x11.hiddenCursorHandle = createHiddenCursor();

    if (XSupportsLocale() && _glfw.x11.xlib.utf8)
    {
        XSetLocaleModifiers("");

        // If an IM is already present our callback will be called right away
        XRegisterIMInstantiateCallback(_glfw.x11.display,
                                       NULL, NULL, NULL,
                                       inputMethodInstantiateCallback,
                                       NULL);
    }

    _glfwInitTimerPOSIX();

    _glfwPollMonitorsX11();
    return GLFW_TRUE;
}

void _glfwPlatformTerminate(void)
{
    if (_glfw.x11.helperWindowHandle)
    {
        if (XGetSelectionOwner(_glfw.x11.display, _glfw.x11.CLIPBOARD) ==
            _glfw.x11.helperWindowHandle)
        {
            _glfwPushSelectionToManagerX11();
        }

        XDestroyWindow(_glfw.x11.display, _glfw.x11.helperWindowHandle);
        _glfw.x11.helperWindowHandle = None;
    }

    if (_glfw.x11.hiddenCursorHandle)
    {
        XFreeCursor(_glfw.x11.display, _glfw.x11.hiddenCursorHandle);
        _glfw.x11.hiddenCursorHandle = (Cursor) 0;
    }

    free(_glfw.x11.primarySelectionString);
    free(_glfw.x11.clipboardString);

    XUnregisterIMInstantiateCallback(_glfw.x11.display,
                                     NULL, NULL, NULL,
                                     inputMethodInstantiateCallback,
                                     NULL);

    if (_glfw.x11.im)
    {
        XCloseIM(_glfw.x11.im);
        _glfw.x11.im = NULL;
    }

    if (_glfw.x11.display)
    {
        XCloseDisplay(_glfw.x11.display);
        _glfw.x11.display = NULL;
    }

    if (_glfw.x11.x11xcb.handle)
    {
        _glfw_dlclose(_glfw.x11.x11xcb.handle);
        _glfw.x11.x11xcb.handle = NULL;
    }

    if (_glfw.x11.xcursor.handle)
    {
        _glfw_dlclose(_glfw.x11.xcursor.handle);
        _glfw.x11.xcursor.handle = NULL;
    }

    if (_glfw.x11.randr.handle)
    {
        _glfw_dlclose(_glfw.x11.randr.handle);
        _glfw.x11.randr.handle = NULL;
    }

    if (_glfw.x11.xinerama.handle)
    {
        _glfw_dlclose(_glfw.x11.xinerama.handle);
        _glfw.x11.xinerama.handle = NULL;
    }

    if (_glfw.x11.xrender.handle)
    {
        _glfw_dlclose(_glfw.x11.xrender.handle);
        _glfw.x11.xrender.handle = NULL;
    }

    if (_glfw.x11.vidmode.handle)
    {
        _glfw_dlclose(_glfw.x11.vidmode.handle);
        _glfw.x11.vidmode.handle = NULL;
    }

    if (_glfw.x11.xi.handle)
    {
        _glfw_dlclose(_glfw.x11.xi.handle);
        _glfw.x11.xi.handle = NULL;
    }

    // NOTE: These need to be unloaded after XCloseDisplay, as they register
    //       cleanup callbacks that get called by that function
    _glfwTerminateEGL();
    _glfwTerminateGLX();

    if (_glfw.x11.xlib.handle)
    {
        _glfw_dlclose(_glfw.x11.xlib.handle);
        _glfw.x11.xlib.handle = NULL;
    }
}

const char* _glfwPlatformGetVersionString(void)
{
    return _GLFW_VERSION_NUMBER " X11 GLX EGL OSMesa"
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
        " clock_gettime"
#else
        " gettimeofday"
#endif
#if defined(__linux__)
        " evdev"
#endif
#if defined(_GLFW_BUILD_DLL)
        " shared"
#endif
        ;
}

