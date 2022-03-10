/*******************************************************************************************
*
*   raygui v3.0 - A simple and easy-to-use immediate-mode gui library
*
*   DESCRIPTION:
*
*   raygui is a tools-dev-focused immediate-mode-gui library based on raylib but also
*   available as a standalone library, as long as input and drawing functions are provided.
*
*   Controls provided:
*
*   # Container/separators Controls
*       - WindowBox
*       - GroupBox
*       - Line
*       - Panel
*
*   # Basic Controls
*       - Label
*       - Button
*       - LabelButton   --> Label
*       - Toggle
*       - ToggleGroup   --> Toggle
*       - CheckBox
*       - ComboBox
*       - DropdownBox
*       - TextBox
*       - TextBoxMulti
*       - ValueBox      --> TextBox
*       - Spinner       --> Button, ValueBox
*       - Slider
*       - SliderBar     --> Slider
*       - ProgressBar
*       - StatusBar
*       - ScrollBar
*       - ScrollPanel
*       - DummyRec
*       - Grid
*
*   # Advance Controls
*       - ListView
*       - ColorPicker   --> ColorPanel, ColorBarHue
*       - MessageBox    --> Window, Label, Button
*       - TextInputBox  --> Window, Label, TextBox, Button
*
*   It also provides a set of functions for styling the controls based on its properties (size, color).
*
*
*   GUI STYLE (guiStyle):
*
*   raygui uses a global data array for all gui style properties (allocated on data segment by default),
*   when a new style is loaded, it is loaded over the global style... but a default gui style could always be
*   recovered with GuiLoadStyleDefault() function, that overwrites the current style to the default one
*
*   The global style array size is fixed and depends on the number of controls and properties:
*
*       static unsigned int guiStyle[RAYGUI_MAX_CONTROLS*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED)];
*
*   guiStyle size is by default: 16*(16 + 8) = 384*4 = 1536 bytes = 1.5 KB
*
*   Note that the first set of BASE properties (by default guiStyle[0..15]) belong to the generic style
*   used for all controls, when any of those base values is set, it is automatically populated to all
*   controls, so, specific control values overwriting generic style should be set after base values.
*
*   After the first BASE set we have the EXTENDED properties (by default guiStyle[16..23]), those
*   properties are actually common to all controls and can not be overwritten individually (like BASE ones)
*   Some of those properties are: TEXT_SIZE, TEXT_SPACING, LINE_COLOR, BACKGROUND_COLOR
*
*   Custom control properties can be defined using the EXTENDED properties for each independent control.
*
*   TOOL: rGuiStyler is a visual tool to customize raygui style.
*
*
*   GUI ICONS (guiIcons):
*
*   raygui could use a global array containing icons data (allocated on data segment by default),
*   a custom icons set could be loaded over this array using GuiLoadIcons(), but loaded icons set
*   must be same RICON_SIZE and no more than RICON_MAX_ICONS will be loaded
*
*   Every icon is codified in binary form, using 1 bit per pixel, so, every 16x16 icon
*   requires 8 integers (16*16/32) to be stored in memory.
*
*   When the icon is draw, actually one quad per pixel is drawn if the bit for that pixel is set.
*
*   The global icons array size is fixed and depends on the number of icons and size:
*
*       static unsigned int guiIcons[RICON_MAX_ICONS*RICON_DATA_ELEMENTS];
*
*   guiIcons size is by default: 256*(16*16/32) = 2048*4 = 8192 bytes = 8 KB
*
*   TOOL: rGuiIcons is a visual tool to customize raygui icons.
*
*
*   CONFIGURATION:
*
*   #define RAYGUI_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define RAYGUI_STANDALONE
*       Avoid raylib.h header inclusion in this file. Data types defined on raylib are defined
*       internally in the library and input management and drawing functions must be provided by
*       the user (check library implementation for further details).
*
*   #define RAYGUI_NO_RICONS
*       Avoid including embedded ricons data (256 icons, 16x16 pixels, 1-bit per pixel, 2KB)
*
*   #define RAYGUI_CUSTOM_RICONS
*       Includes custom ricons.h header defining a set of custom icons,
*       this file can be generated using rGuiIcons tool
*
*
*   VERSIONS HISTORY:
*
*       3.0 (xx-Sep-2021) Integrated ricons data to avoid external file
*                         REDESIGNED: GuiTextBoxMulti()
*                         REMOVED: GuiImageButton*()
*                         Multiple minor tweaks and bugs corrected
*       2.9 (17-Mar-2021) REMOVED: Tooltip API
*       2.8 (03-May-2020) Centralized rectangles drawing to GuiDrawRectangle()
*       2.7 (20-Feb-2020) ADDED: Possible tooltips API
*       2.6 (09-Sep-2019) ADDED: GuiTextInputBox()
*                         REDESIGNED: GuiListView*(), GuiDropdownBox(), GuiSlider*(), GuiProgressBar(), GuiMessageBox()
*                         REVIEWED: GuiTextBox(), GuiSpinner(), GuiValueBox(), GuiLoadStyle()
*                         Replaced property INNER_PADDING by TEXT_PADDING, renamed some properties
*                         ADDED: 8 new custom styles ready to use
*                         Multiple minor tweaks and bugs corrected
*       2.5 (28-May-2019) Implemented extended GuiTextBox(), GuiValueBox(), GuiSpinner()
*       2.3 (29-Apr-2019) ADDED: rIcons auxiliar library and support for it, multiple controls reviewed
*                         Refactor all controls drawing mechanism to use control state
*       2.2 (05-Feb-2019) ADDED: GuiScrollBar(), GuiScrollPanel(), reviewed GuiListView(), removed Gui*Ex() controls
*       2.1 (26-Dec-2018) REDESIGNED: GuiCheckBox(), GuiComboBox(), GuiDropdownBox(), GuiToggleGroup() > Use combined text string
*                         REDESIGNED: Style system (breaking change)
*       2.0 (08-Nov-2018) ADDED: Support controls guiLock and custom fonts
*                         REVIEWED: GuiComboBox(), GuiListView()...
*       1.9 (09-Oct-2018) REVIEWED: GuiGrid(), GuiTextBox(), GuiTextBoxMulti(), GuiValueBox()...
*       1.8 (01-May-2018) Lot of rework and redesign to align with rGuiStyler and rGuiLayout
*       1.5 (21-Jun-2017) Working in an improved styles system
*       1.4 (15-Jun-2017) Rewritten all GUI functions (removed useless ones)
*       1.3 (12-Jun-2017) Complete redesign of style system
*       1.1 (01-Jun-2017) Complete review of the library
*       1.0 (07-Jun-2016) Converted to header-only by Ramon Santamaria.
*       0.9 (07-Mar-2016) Reviewed and tested by Albert Martos, Ian Eito, Sergio Martinez and Ramon Santamaria.
*       0.8 (27-Aug-2015) Initial release. Implemented by Kevin Gato, Daniel Nicolás and Ramon Santamaria.
*
*
*   CONTRIBUTORS:
*
*       Ramon Santamaria:   Supervision, review, redesign, update and maintenance
*       Vlad Adrian:        Complete rewrite of GuiTextBox() to support extended features (2019)
*       Sergio Martinez:    Review, testing (2015) and redesign of multiple controls (2018)
*       Adria Arranz:       Testing and Implementation of additional controls (2018)
*       Jordi Jorba:        Testing and Implementation of additional controls (2018)
*       Albert Martos:      Review and testing of the library (2015)
*       Ian Eito:           Review and testing of the library (2015)
*       Kevin Gato:         Initial implementation of basic components (2014)
*       Daniel Nicolas:     Initial implementation of basic components (2014)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2021 Ramon Santamaria (@raysan5)
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

#ifndef RAYGUI_H
#define RAYGUI_H

#define RAYGUI_VERSION  "3.0"

#if !defined(RAYGUI_STANDALONE)
    #include "raylib.h"
#endif

// Function specifiers in case library is build/used as a shared library (Windows)
// NOTE: Microsoft specifiers to tell compiler that symbols are imported/exported from a .dll
#if defined(_WIN32)
    #if defined(BUILD_LIBTYPE_SHARED)
        #define RAYGUIAPI __declspec(dllexport)     // We are building the library as a Win32 shared library (.dll)
    #elif defined(USE_LIBTYPE_SHARED)
        #define RAYGUIAPI __declspec(dllimport)     // We are using the library as a Win32 shared library (.dll)
    #endif
#endif

// Function specifiers definition
#ifndef RAYGUIAPI
    #define RAYGUIAPI       // Functions defined as 'extern' by default (implicit specifiers)
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------

// Allow custom memory allocators
#ifndef RAYGUI_MALLOC
    #define RAYGUI_MALLOC(sz)       malloc(sz)
#endif
#ifndef RAYGUI_CALLOC
    #define RAYGUI_CALLOC(n,sz)     calloc(n,sz)
#endif
#ifndef RAYGUI_FREE
    #define RAYGUI_FREE(p)          free(p)
#endif

// TODO: Implement custom TraceLog()
#define TRACELOG(level, ...) (void)0

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Some types are required for RAYGUI_STANDALONE usage
//----------------------------------------------------------------------------------
#if defined(RAYGUI_STANDALONE)
    #ifndef __cplusplus
    // Boolean type
        #ifndef true
            typedef enum { false, true } bool;
        #endif
    #endif

    // Vector2 type
    typedef struct Vector2 {
        float x;
        float y;
    } Vector2;

    // Vector3 type                 // -- ConvertHSVtoRGB(), ConvertRGBtoHSV()
    typedef struct Vector3 {
        float x;
        float y;
        float z;
    } Vector3;

    // Color type, RGBA (32bit)
    typedef struct Color {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    } Color;

    // Rectangle type
    typedef struct Rectangle {
        float x;
        float y;
        float width;
        float height;
    } Rectangle;

    // TODO: Texture2D type is very coupled to raylib, required by Font type
    // It should be redesigned to be provided by user
    typedef struct Texture2D {
        unsigned int id;        // OpenGL texture id
        int width;              // Texture base width
        int height;             // Texture base height
        int mipmaps;            // Mipmap levels, 1 by default
        int format;             // Data format (PixelFormat type)
    } Texture2D;

    // GlyphInfo, font characters glyphs info
    typedef struct GlyphInfo {
        int value;              // Character value (Unicode)
        int offsetX;            // Character offset X when drawing
        int offsetY;            // Character offset Y when drawing
        int advanceX;           // Character advance position X
        Image image;            // Character image data
    } GlyphInfo;

    // TODO: Font type is very coupled to raylib, mostly required by GuiLoadStyle()
    // It should be redesigned to be provided by user
    typedef struct Font {
        int baseSize;           // Base size (default chars height)
        int glyphCount;         // Number of characters
        Texture2D texture;      // Characters texture atlas
        Rectangle *recs;        // Characters rectangles in texture
        GlyphInfo *chars;       // Characters info data
    } Font;
#endif

// Style property
typedef struct GuiStyleProp {
    unsigned short controlId;
    unsigned short propertyId;
    int propertyValue;
} GuiStyleProp;

// Gui control state
typedef enum {
    GUI_STATE_NORMAL = 0,
    GUI_STATE_FOCUSED,
    GUI_STATE_PRESSED,
    GUI_STATE_DISABLED,
} GuiControlState;

// Gui control text alignment
typedef enum {
    GUI_TEXT_ALIGN_LEFT = 0,
    GUI_TEXT_ALIGN_CENTER,
    GUI_TEXT_ALIGN_RIGHT,
} GuiTextAlignment;

// Gui controls
typedef enum {
    DEFAULT = 0,    // Generic control -> populates to all controls when set
    LABEL,          // Used also for: LABELBUTTON
    BUTTON,
    TOGGLE,         // Used also for: TOGGLEGROUP
    SLIDER,         // Used also for: SLIDERBAR
    PROGRESSBAR,
    CHECKBOX,
    COMBOBOX,
    DROPDOWNBOX,
    TEXTBOX,        // Used also for: TEXTBOXMULTI
    VALUEBOX,
    SPINNER,
    LISTVIEW,
    COLORPICKER,
    SCROLLBAR,
    STATUSBAR
} GuiControl;

// Gui base properties for every control
// NOTE: RAYGUI_MAX_PROPS_BASE properties (by default 16 properties)
typedef enum {
    BORDER_COLOR_NORMAL = 0,
    BASE_COLOR_NORMAL,
    TEXT_COLOR_NORMAL,
    BORDER_COLOR_FOCUSED,
    BASE_COLOR_FOCUSED,
    TEXT_COLOR_FOCUSED,
    BORDER_COLOR_PRESSED,
    BASE_COLOR_PRESSED,
    TEXT_COLOR_PRESSED,
    BORDER_COLOR_DISABLED,
    BASE_COLOR_DISABLED,
    TEXT_COLOR_DISABLED,
    BORDER_WIDTH,
    TEXT_PADDING,
    TEXT_ALIGNMENT,
    RESERVED
} GuiControlProperty;

// Gui extended properties depend on control
// NOTE: RAYGUI_MAX_PROPS_EXTENDED properties (by default 8 properties)

// DEFAULT extended properties
// NOTE: Those properties are actually common to all controls
typedef enum {
    TEXT_SIZE = 16,
    TEXT_SPACING,
    LINE_COLOR,
    BACKGROUND_COLOR,
} GuiDefaultProperty;

// Label
//typedef enum { } GuiLabelProperty;

// Button
//typedef enum { } GuiButtonProperty;

// Toggle/ToggleGroup
typedef enum {
    GROUP_PADDING = 16,
} GuiToggleProperty;

// Slider/SliderBar
typedef enum {
    SLIDER_WIDTH = 16,
    SLIDER_PADDING
} GuiSliderProperty;

// ProgressBar
typedef enum {
    PROGRESS_PADDING = 16,
} GuiProgressBarProperty;

// CheckBox
typedef enum {
    CHECK_PADDING = 16
} GuiCheckBoxProperty;

// ComboBox
typedef enum {
    COMBO_BUTTON_WIDTH = 16,
    COMBO_BUTTON_PADDING
} GuiComboBoxProperty;

// DropdownBox
typedef enum {
    ARROW_PADDING = 16,
    DROPDOWN_ITEMS_PADDING
} GuiDropdownBoxProperty;

// TextBox/TextBoxMulti/ValueBox/Spinner
typedef enum {
    TEXT_INNER_PADDING = 16,
    TEXT_LINES_PADDING,
    COLOR_SELECTED_FG,
    COLOR_SELECTED_BG
} GuiTextBoxProperty;

// Spinner
typedef enum {
    SPIN_BUTTON_WIDTH = 16,
    SPIN_BUTTON_PADDING,
} GuiSpinnerProperty;

// ScrollBar
typedef enum {
    ARROWS_SIZE = 16,
    ARROWS_VISIBLE,
    SCROLL_SLIDER_PADDING,
    SCROLL_SLIDER_SIZE,
    SCROLL_PADDING,
    SCROLL_SPEED,
} GuiScrollBarProperty;

// ScrollBar side
typedef enum {
    SCROLLBAR_LEFT_SIDE = 0,
    SCROLLBAR_RIGHT_SIDE
} GuiScrollBarSide;

// ListView
typedef enum {
    LIST_ITEMS_HEIGHT = 16,
    LIST_ITEMS_PADDING,
    SCROLLBAR_WIDTH,
    SCROLLBAR_SIDE,
} GuiListViewProperty;

// ColorPicker
typedef enum {
    COLOR_SELECTOR_SIZE = 16,
    HUEBAR_WIDTH,                  // Right hue bar width
    HUEBAR_PADDING,                // Right hue bar separation from panel
    HUEBAR_SELECTOR_HEIGHT,        // Right hue bar selector height
    HUEBAR_SELECTOR_OVERFLOW       // Right hue bar selector overflow
} GuiColorPickerProperty;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

// Global gui state control functions
RAYGUIAPI void GuiEnable(void);                                         // Enable gui controls (global state)
RAYGUIAPI void GuiDisable(void);                                        // Disable gui controls (global state)
RAYGUIAPI void GuiLock(void);                                           // Lock gui controls (global state)
RAYGUIAPI void GuiUnlock(void);                                         // Unlock gui controls (global state)
RAYGUIAPI bool GuiIsLocked(void);                                       // Check if gui is locked (global state)
RAYGUIAPI void GuiFade(float alpha);                                    // Set gui controls alpha (global state), alpha goes from 0.0f to 1.0f
RAYGUIAPI void GuiSetState(int state);                                  // Set gui state (global state)
RAYGUIAPI int GuiGetState(void);                                        // Get gui state (global state)

// Font set/get functions
RAYGUIAPI void GuiSetFont(Font font);                                   // Set gui custom font (global state)
RAYGUIAPI Font GuiGetFont(void);                                        // Get gui custom font (global state)

// Style set/get functions
RAYGUIAPI void GuiSetStyle(int control, int property, int value);       // Set one style property
RAYGUIAPI int GuiGetStyle(int control, int property);                   // Get one style property

// Container/separator controls, useful for controls organization
RAYGUIAPI bool GuiWindowBox(Rectangle bounds, const char *title);                                       // Window Box control, shows a window that can be closed
RAYGUIAPI void GuiGroupBox(Rectangle bounds, const char *text);                                         // Group Box control with text name
RAYGUIAPI void GuiLine(Rectangle bounds, const char *text);                                             // Line separator control, could contain text
RAYGUIAPI void GuiPanel(Rectangle bounds);                                                              // Panel control, useful to group controls
RAYGUIAPI Rectangle GuiScrollPanel(Rectangle bounds, Rectangle content, Vector2 *scroll);               // Scroll Panel control

// Basic controls set
RAYGUIAPI void GuiLabel(Rectangle bounds, const char *text);                                            // Label control, shows text
RAYGUIAPI bool GuiButton(Rectangle bounds, const char *text);                                           // Button control, returns true when clicked
RAYGUIAPI bool GuiLabelButton(Rectangle bounds, const char *text);                                      // Label button control, show true when clicked
RAYGUIAPI bool GuiToggle(Rectangle bounds, const char *text, bool active);                              // Toggle Button control, returns true when active
RAYGUIAPI int GuiToggleGroup(Rectangle bounds, const char *text, int active);                           // Toggle Group control, returns active toggle index
RAYGUIAPI bool GuiCheckBox(Rectangle bounds, const char *text, bool checked);                           // Check Box control, returns true when active
RAYGUIAPI int GuiComboBox(Rectangle bounds, const char *text, int active);                              // Combo Box control, returns selected item index
RAYGUIAPI bool GuiDropdownBox(Rectangle bounds, const char *text, int *active, bool editMode);          // Dropdown Box control, returns selected item
RAYGUIAPI bool GuiSpinner(Rectangle bounds, const char *text, int *value, int minValue, int maxValue, bool editMode);     // Spinner control, returns selected value
RAYGUIAPI bool GuiValueBox(Rectangle bounds, const char *text, int *value, int minValue, int maxValue, bool editMode);    // Value Box control, updates input text with numbers
RAYGUIAPI bool GuiTextBox(Rectangle bounds, char *text, int textSize, bool editMode);                   // Text Box control, updates input text
RAYGUIAPI bool GuiTextBoxMulti(Rectangle bounds, char *text, int textSize, bool editMode);              // Text Box control with multiple lines
RAYGUIAPI float GuiSlider(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue);       // Slider control, returns selected value
RAYGUIAPI float GuiSliderBar(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue);    // Slider Bar control, returns selected value
RAYGUIAPI float GuiProgressBar(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue);  // Progress Bar control, shows current progress value
RAYGUIAPI void GuiStatusBar(Rectangle bounds, const char *text);                                        // Status Bar control, shows info text
RAYGUIAPI void GuiDummyRec(Rectangle bounds, const char *text);                                         // Dummy control for placeholders
RAYGUIAPI int GuiScrollBar(Rectangle bounds, int value, int minValue, int maxValue);                    // Scroll Bar control
RAYGUIAPI Vector2 GuiGrid(Rectangle bounds, float spacing, int subdivs);                                // Grid control


// Advance controls set
RAYGUIAPI int GuiListView(Rectangle bounds, const char *text, int *scrollIndex, int active);            // List View control, returns selected list item index
RAYGUIAPI int GuiListViewEx(Rectangle bounds, const char **text, int count, int *focus, int *scrollIndex, int active);      // List View with extended parameters
RAYGUIAPI int GuiMessageBox(Rectangle bounds, const char *title, const char *message, const char *buttons);                 // Message Box control, displays a message
RAYGUIAPI int GuiTextInputBox(Rectangle bounds, const char *title, const char *message, const char *buttons, char *text);   // Text Input Box control, ask for text
RAYGUIAPI Color GuiColorPicker(Rectangle bounds, Color color);                                          // Color Picker control (multiple color controls)
RAYGUIAPI Color GuiColorPanel(Rectangle bounds, Color color);                                           // Color Panel control
RAYGUIAPI float GuiColorBarAlpha(Rectangle bounds, float alpha);                                        // Color Bar Alpha control
RAYGUIAPI float GuiColorBarHue(Rectangle bounds, float value);                                          // Color Bar Hue control

// Styles loading functions
RAYGUIAPI void GuiLoadStyle(const char *fileName);              // Load style file over global style variable (.rgs)
RAYGUIAPI void GuiLoadStyleDefault(void);                       // Load style default over global style

/*
typedef GuiStyle (unsigned int *)
RAYGUIAPI GuiStyle LoadGuiStyle(const char *fileName);          // Load style from file (.rgs)
RAYGUIAPI void UnloadGuiStyle(GuiStyle style);                  // Unload style
*/

RAYGUIAPI const char *GuiIconText(int iconId, const char *text); // Get text with icon id prepended (if supported)

#if !defined(RAYGUI_NO_RICONS)
// Gui icons functionality
RAYGUIAPI void GuiDrawIcon(int iconId, int posX, int posY, int pixelSize, Color color);

RAYGUIAPI unsigned int *GuiGetIcons(void);                      // Get full icons data pointer
RAYGUIAPI unsigned int *GuiGetIconData(int iconId);             // Get icon bit data
RAYGUIAPI void GuiSetIconData(int iconId, unsigned int *data);  // Set icon bit data

RAYGUIAPI void GuiSetIconPixel(int iconId, int x, int y);       // Set icon pixel value
RAYGUIAPI void GuiClearIconPixel(int iconId, int x, int y);     // Clear icon pixel value
RAYGUIAPI bool GuiCheckIconPixel(int iconId, int x, int y);     // Check icon pixel value
#endif

#if defined(__cplusplus)
}            // Prevents name mangling of functions
#endif

#endif // RAYGUI_H

/***********************************************************************************
*
*   RAYGUI IMPLEMENTATION
*
************************************************************************************/

#if defined(RAYGUI_IMPLEMENTATION)

#include <stdio.h>              // Required for: FILE, fopen(), fclose(), fprintf(), feof(), fscanf(), vsprintf() [GuiLoadStyle(), GuiLoadIcons()]
#include <stdlib.h>             // Required for: malloc(), calloc(), free() [GuiLoadStyle(), GuiLoadIcons()]
#include <string.h>             // Required for: strlen() [GuiTextBox(), GuiTextBoxMulti(), GuiValueBox()], memset(), memcpy()
#include <stdarg.h>             // Required for: va_list, va_start(), vfprintf(), va_end() [TextFormat()]
#include <math.h>               // Required for: roundf() [GuiColorPicker()]

#ifdef __cplusplus
    #define RAYGUI_CLITERAL(name) name
#else
    #define RAYGUI_CLITERAL(name) (name)
#endif

#if !defined(RAYGUI_NO_RICONS)

#if defined(RAYGUI_CUSTOM_RICONS)

#define RICONS_IMPLEMENTATION
#include "ricons.h"         // External icons data provided, it can be generated with rGuiIcons tool

#else   // Embedded raygui icons, no external file provided

#define RICON_SIZE               16       // Size of icons (squared)
#define RICON_MAX_ICONS         256       // Maximum number of icons
#define RICON_MAX_NAME_LENGTH    32       // Maximum length of icon name id

// Icons data is defined by bit array (every bit represents one pixel)
// Those arrays are stored as unsigned int data arrays, so every array
// element defines 32 pixels (bits) of information
// Number of elemens depend on RICON_SIZE (by default 16x16 pixels)
#define RICON_DATA_ELEMENTS   (RICON_SIZE*RICON_SIZE/32)

//----------------------------------------------------------------------------------
// Icons enumeration
//----------------------------------------------------------------------------------
typedef enum {
    RICON_NONE                     = 0,
    RICON_FOLDER_FILE_OPEN         = 1,
    RICON_FILE_SAVE_CLASSIC        = 2,
    RICON_FOLDER_OPEN              = 3,
    RICON_FOLDER_SAVE              = 4,
    RICON_FILE_OPEN                = 5,
    RICON_FILE_SAVE                = 6,
    RICON_FILE_EXPORT              = 7,
    RICON_FILE_NEW                 = 8,
    RICON_FILE_DELETE              = 9,
    RICON_FILETYPE_TEXT            = 10,
    RICON_FILETYPE_AUDIO           = 11,
    RICON_FILETYPE_IMAGE           = 12,
    RICON_FILETYPE_PLAY            = 13,
    RICON_FILETYPE_VIDEO           = 14,
    RICON_FILETYPE_INFO            = 15,
    RICON_FILE_COPY                = 16,
    RICON_FILE_CUT                 = 17,
    RICON_FILE_PASTE               = 18,
    RICON_CURSOR_HAND              = 19,
    RICON_CURSOR_POINTER           = 20,
    RICON_CURSOR_CLASSIC           = 21,
    RICON_PENCIL                   = 22,
    RICON_PENCIL_BIG               = 23,
    RICON_BRUSH_CLASSIC            = 24,
    RICON_BRUSH_PAINTER            = 25,
    RICON_WATER_DROP               = 26,
    RICON_COLOR_PICKER             = 27,
    RICON_RUBBER                   = 28,
    RICON_COLOR_BUCKET             = 29,
    RICON_TEXT_T                   = 30,
    RICON_TEXT_A                   = 31,
    RICON_SCALE                    = 32,
    RICON_RESIZE                   = 33,
    RICON_FILTER_POINT             = 34,
    RICON_FILTER_BILINEAR          = 35,
    RICON_CROP                     = 36,
    RICON_CROP_ALPHA               = 37,
    RICON_SQUARE_TOGGLE            = 38,
    RICON_SYMMETRY                 = 39,
    RICON_SYMMETRY_HORIZONTAL      = 40,
    RICON_SYMMETRY_VERTICAL        = 41,
    RICON_LENS                     = 42,
    RICON_LENS_BIG                 = 43,
    RICON_EYE_ON                   = 44,
    RICON_EYE_OFF                  = 45,
    RICON_FILTER_TOP               = 46,
    RICON_FILTER                   = 47,
    RICON_TARGET_POINT             = 48,
    RICON_TARGET_SMALL             = 49,
    RICON_TARGET_BIG               = 50,
    RICON_TARGET_MOVE              = 51,
    RICON_CURSOR_MOVE              = 52,
    RICON_CURSOR_SCALE             = 53,
    RICON_CURSOR_SCALE_RIGHT       = 54,
    RICON_CURSOR_SCALE_LEFT        = 55,
    RICON_UNDO                     = 56,
    RICON_REDO                     = 57,
    RICON_REREDO                   = 58,
    RICON_MUTATE                   = 59,
    RICON_ROTATE                   = 60,
    RICON_REPEAT                   = 61,
    RICON_SHUFFLE                  = 62,
    RICON_EMPTYBOX                 = 63,
    RICON_TARGET                   = 64,
    RICON_TARGET_SMALL_FILL        = 65,
    RICON_TARGET_BIG_FILL          = 66,
    RICON_TARGET_MOVE_FILL         = 67,
    RICON_CURSOR_MOVE_FILL         = 68,
    RICON_CURSOR_SCALE_FILL        = 69,
    RICON_CURSOR_SCALE_RIGHT_FILL  = 70,
    RICON_CURSOR_SCALE_LEFT_FILL   = 71,
    RICON_UNDO_FILL                = 72,
    RICON_REDO_FILL                = 73,
    RICON_REREDO_FILL              = 74,
    RICON_MUTATE_FILL              = 75,
    RICON_ROTATE_FILL              = 76,
    RICON_REPEAT_FILL              = 77,
    RICON_SHUFFLE_FILL             = 78,
    RICON_EMPTYBOX_SMALL           = 79,
    RICON_BOX                      = 80,
    RICON_BOX_TOP                  = 81,
    RICON_BOX_TOP_RIGHT            = 82,
    RICON_BOX_RIGHT                = 83,
    RICON_BOX_BOTTOM_RIGHT         = 84,
    RICON_BOX_BOTTOM               = 85,
    RICON_BOX_BOTTOM_LEFT          = 86,
    RICON_BOX_LEFT                 = 87,
    RICON_BOX_TOP_LEFT             = 88,
    RICON_BOX_CENTER               = 89,
    RICON_BOX_CIRCLE_MASK          = 90,
    RICON_POT                      = 91,
    RICON_ALPHA_MULTIPLY           = 92,
    RICON_ALPHA_CLEAR              = 93,
    RICON_DITHERING                = 94,
    RICON_MIPMAPS                  = 95,
    RICON_BOX_GRID                 = 96,
    RICON_GRID                     = 97,
    RICON_BOX_CORNERS_SMALL        = 98,
    RICON_BOX_CORNERS_BIG          = 99,
    RICON_FOUR_BOXES               = 100,
    RICON_GRID_FILL                = 101,
    RICON_BOX_MULTISIZE            = 102,
    RICON_ZOOM_SMALL               = 103,
    RICON_ZOOM_MEDIUM              = 104,
    RICON_ZOOM_BIG                 = 105,
    RICON_ZOOM_ALL                 = 106,
    RICON_ZOOM_CENTER              = 107,
    RICON_BOX_DOTS_SMALL           = 108,
    RICON_BOX_DOTS_BIG             = 109,
    RICON_BOX_CONCENTRIC           = 110,
    RICON_BOX_GRID_BIG             = 111,
    RICON_OK_TICK                  = 112,
    RICON_CROSS                    = 113,
    RICON_ARROW_LEFT               = 114,
    RICON_ARROW_RIGHT              = 115,
    RICON_ARROW_DOWN               = 116,
    RICON_ARROW_UP                 = 117,
    RICON_ARROW_LEFT_FILL          = 118,
    RICON_ARROW_RIGHT_FILL         = 119,
    RICON_ARROW_DOWN_FILL          = 120,
    RICON_ARROW_UP_FILL            = 121,
    RICON_AUDIO                    = 122,
    RICON_FX                       = 123,
    RICON_WAVE                     = 124,
    RICON_WAVE_SINUS               = 125,
    RICON_WAVE_SQUARE              = 126,
    RICON_WAVE_TRIANGULAR          = 127,
    RICON_CROSS_SMALL              = 128,
    RICON_PLAYER_PREVIOUS          = 129,
    RICON_PLAYER_PLAY_BACK         = 130,
    RICON_PLAYER_PLAY              = 131,
    RICON_PLAYER_PAUSE             = 132,
    RICON_PLAYER_STOP              = 133,
    RICON_PLAYER_NEXT              = 134,
    RICON_PLAYER_RECORD            = 135,
    RICON_MAGNET                   = 136,
    RICON_LOCK_CLOSE               = 137,
    RICON_LOCK_OPEN                = 138,
    RICON_CLOCK                    = 139,
    RICON_TOOLS                    = 140,
    RICON_GEAR                     = 141,
    RICON_GEAR_BIG                 = 142,
    RICON_BIN                      = 143,
    RICON_HAND_POINTER             = 144,
    RICON_LASER                    = 145,
    RICON_COIN                     = 146,
    RICON_EXPLOSION                = 147,
    RICON_1UP                      = 148,
    RICON_PLAYER                   = 149,
    RICON_PLAYER_JUMP              = 150,
    RICON_KEY                      = 151,
    RICON_DEMON                    = 152,
    RICON_TEXT_POPUP               = 153,
    RICON_GEAR_EX                  = 154,
    RICON_CRACK                    = 155,
    RICON_CRACK_POINTS             = 156,
    RICON_STAR                     = 157,
    RICON_DOOR                     = 158,
    RICON_EXIT                     = 159,
    RICON_MODE_2D                  = 160,
    RICON_MODE_3D                  = 161,
    RICON_CUBE                     = 162,
    RICON_CUBE_FACE_TOP            = 163,
    RICON_CUBE_FACE_LEFT           = 164,
    RICON_CUBE_FACE_FRONT          = 165,
    RICON_CUBE_FACE_BOTTOM         = 166,
    RICON_CUBE_FACE_RIGHT          = 167,
    RICON_CUBE_FACE_BACK           = 168,
    RICON_CAMERA                   = 169,
    RICON_SPECIAL                  = 170,
    RICON_LINK_NET                 = 171,
    RICON_LINK_BOXES               = 172,
    RICON_LINK_MULTI               = 173,
    RICON_LINK                     = 174,
    RICON_LINK_BROKE               = 175,
    RICON_TEXT_NOTES               = 176,
    RICON_NOTEBOOK                 = 177,
    RICON_SUITCASE                 = 178,
    RICON_SUITCASE_ZIP             = 179,
    RICON_MAILBOX                  = 180,
    RICON_MONITOR                  = 181,
    RICON_PRINTER                  = 182,
    RICON_PHOTO_CAMERA             = 183,
    RICON_PHOTO_CAMERA_FLASH       = 184,
    RICON_HOUSE                    = 185,
    RICON_HEART                    = 186,
    RICON_CORNER                   = 187,
    RICON_VERTICAL_BARS            = 188,
    RICON_VERTICAL_BARS_FILL       = 189,
    RICON_LIFE_BARS                = 190,
    RICON_INFO                     = 191,
    RICON_CROSSLINE                = 192,
    RICON_HELP                     = 193,
    RICON_FILETYPE_ALPHA           = 194,
    RICON_FILETYPE_HOME            = 195,
    RICON_LAYERS_VISIBLE           = 196,
    RICON_LAYERS                   = 197,
    RICON_WINDOW                   = 198,
    RICON_HIDPI                    = 199,
    RICON_200                      = 200,
    RICON_201                      = 201,
    RICON_202                      = 202,
    RICON_203                      = 203,
    RICON_204                      = 204,
    RICON_205                      = 205,
    RICON_206                      = 206,
    RICON_207                      = 207,
    RICON_208                      = 208,
    RICON_209                      = 209,
    RICON_210                      = 210,
    RICON_211                      = 211,
    RICON_212                      = 212,
    RICON_213                      = 213,
    RICON_214                      = 214,
    RICON_215                      = 215,
    RICON_216                      = 216,
    RICON_217                      = 217,
    RICON_218                      = 218,
    RICON_219                      = 219,
    RICON_220                      = 220,
    RICON_221                      = 221,
    RICON_222                      = 222,
    RICON_223                      = 223,
    RICON_224                      = 224,
    RICON_225                      = 225,
    RICON_226                      = 226,
    RICON_227                      = 227,
    RICON_228                      = 228,
    RICON_229                      = 229,
    RICON_230                      = 230,
    RICON_231                      = 231,
    RICON_232                      = 232,
    RICON_233                      = 233,
    RICON_234                      = 234,
    RICON_235                      = 235,
    RICON_236                      = 236,
    RICON_237                      = 237,
    RICON_238                      = 238,
    RICON_239                      = 239,
    RICON_240                      = 240,
    RICON_241                      = 241,
    RICON_242                      = 242,
    RICON_243                      = 243,
    RICON_244                      = 244,
    RICON_245                      = 245,
    RICON_246                      = 246,
    RICON_247                      = 247,
    RICON_248                      = 248,
    RICON_249                      = 249,
    RICON_250                      = 250,
    RICON_251                      = 251,
    RICON_252                      = 252,
    RICON_253                      = 253,
    RICON_254                      = 254,
    RICON_255                      = 255,
} guiIconName;

//----------------------------------------------------------------------------------
// Icons data for all gui possible icons (allocated on data segment by default)
//
// NOTE 1: Every icon is codified in binary form, using 1 bit per pixel, so,
// every 16x16 icon requires 8 integers (16*16/32) to be stored
//
// NOTE 2: A new icon set could be loaded over this array using GuiLoadIcons(),
// but loaded icons set must be same RICON_SIZE and no more than RICON_MAX_ICONS
//
// guiIcons size is by default: 256*(16*16/32) = 2048*4 = 8192 bytes = 8 KB
//----------------------------------------------------------------------------------
static unsigned int guiIcons[RICON_MAX_ICONS*RICON_DATA_ELEMENTS] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_NONE
    0x3ff80000, 0x2f082008, 0x2042207e, 0x40027fc2, 0x40024002, 0x40024002, 0x40024002, 0x00007ffe,     // RICON_FOLDER_FILE_OPEN
    0x3ffe0000, 0x44226422, 0x400247e2, 0x5ffa4002, 0x57ea500a, 0x500a500a, 0x40025ffa, 0x00007ffe,     // RICON_FILE_SAVE_CLASSIC
    0x00000000, 0x0042007e, 0x40027fc2, 0x40024002, 0x41024002, 0x44424282, 0x793e4102, 0x00000100,     // RICON_FOLDER_OPEN
    0x00000000, 0x0042007e, 0x40027fc2, 0x40024002, 0x41024102, 0x44424102, 0x793e4282, 0x00000000,     // RICON_FOLDER_SAVE
    0x3ff00000, 0x201c2010, 0x20042004, 0x21042004, 0x24442284, 0x21042104, 0x20042104, 0x00003ffc,     // RICON_FILE_OPEN
    0x3ff00000, 0x201c2010, 0x20042004, 0x21042004, 0x21042104, 0x22842444, 0x20042104, 0x00003ffc,     // RICON_FILE_SAVE
    0x3ff00000, 0x201c2010, 0x00042004, 0x20041004, 0x20844784, 0x00841384, 0x20042784, 0x00003ffc,     // RICON_FILE_EXPORT
    0x3ff00000, 0x201c2010, 0x20042004, 0x20042004, 0x22042204, 0x22042f84, 0x20042204, 0x00003ffc,     // RICON_FILE_NEW
    0x3ff00000, 0x201c2010, 0x20042004, 0x20042004, 0x25042884, 0x25042204, 0x20042884, 0x00003ffc,     // RICON_FILE_DELETE
    0x3ff00000, 0x201c2010, 0x20042004, 0x20042ff4, 0x20042ff4, 0x20042ff4, 0x20042004, 0x00003ffc,     // RICON_FILETYPE_TEXT
    0x3ff00000, 0x201c2010, 0x27042004, 0x244424c4, 0x26442444, 0x20642664, 0x20042004, 0x00003ffc,     // RICON_FILETYPE_AUDIO
    0x3ff00000, 0x201c2010, 0x26042604, 0x20042004, 0x35442884, 0x2414222c, 0x20042004, 0x00003ffc,     // RICON_FILETYPE_IMAGE
    0x3ff00000, 0x201c2010, 0x20c42004, 0x22442144, 0x22442444, 0x20c42144, 0x20042004, 0x00003ffc,     // RICON_FILETYPE_PLAY
    0x3ff00000, 0x3ffc2ff0, 0x3f3c2ff4, 0x3dbc2eb4, 0x3dbc2bb4, 0x3f3c2eb4, 0x3ffc2ff4, 0x00002ff4,     // RICON_FILETYPE_VIDEO
    0x3ff00000, 0x201c2010, 0x21842184, 0x21842004, 0x21842184, 0x21842184, 0x20042184, 0x00003ffc,     // RICON_FILETYPE_INFO
    0x0ff00000, 0x381c0810, 0x28042804, 0x28042804, 0x28042804, 0x28042804, 0x20102ffc, 0x00003ff0,     // RICON_FILE_COPY
    0x00000000, 0x701c0000, 0x079c1e14, 0x55a000f0, 0x079c00f0, 0x701c1e14, 0x00000000, 0x00000000,     // RICON_FILE_CUT
    0x01c00000, 0x13e41bec, 0x3f841004, 0x204420c4, 0x20442044, 0x20442044, 0x207c2044, 0x00003fc0,     // RICON_FILE_PASTE
    0x00000000, 0x3aa00fe0, 0x2abc2aa0, 0x2aa42aa4, 0x20042aa4, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_CURSOR_HAND
    0x00000000, 0x003c000c, 0x030800c8, 0x30100c10, 0x10202020, 0x04400840, 0x01800280, 0x00000000,     // RICON_CURSOR_POINTER
    0x00000000, 0x00180000, 0x01f00078, 0x03e007f0, 0x07c003e0, 0x04000e40, 0x00000000, 0x00000000,     // RICON_CURSOR_CLASSIC
    0x00000000, 0x04000000, 0x11000a00, 0x04400a80, 0x01100220, 0x00580088, 0x00000038, 0x00000000,     // RICON_PENCIL
    0x04000000, 0x15000a00, 0x50402880, 0x14102820, 0x05040a08, 0x015c028c, 0x007c00bc, 0x00000000,     // RICON_PENCIL_BIG
    0x01c00000, 0x01400140, 0x01400140, 0x0ff80140, 0x0ff80808, 0x0aa80808, 0x0aa80aa8, 0x00000ff8,     // RICON_BRUSH_CLASSIC
    0x1ffc0000, 0x5ffc7ffe, 0x40004000, 0x00807f80, 0x01c001c0, 0x01c001c0, 0x01c001c0, 0x00000080,     // RICON_BRUSH_PAINTER
    0x00000000, 0x00800000, 0x01c00080, 0x03e001c0, 0x07f003e0, 0x036006f0, 0x000001c0, 0x00000000,     // RICON_WATER_DROP
    0x00000000, 0x3e003800, 0x1f803f80, 0x0c201e40, 0x02080c10, 0x00840104, 0x00380044, 0x00000000,     // RICON_COLOR_PICKER
    0x00000000, 0x07800300, 0x1fe00fc0, 0x3f883fd0, 0x0e021f04, 0x02040402, 0x00f00108, 0x00000000,     // RICON_RUBBER
    0x00c00000, 0x02800140, 0x08200440, 0x20081010, 0x2ffe3004, 0x03f807fc, 0x00e001f0, 0x00000040,     // RICON_COLOR_BUCKET
    0x00000000, 0x21843ffc, 0x01800180, 0x01800180, 0x01800180, 0x01800180, 0x03c00180, 0x00000000,     // RICON_TEXT_T
    0x00800000, 0x01400180, 0x06200340, 0x0c100620, 0x1ff80c10, 0x380c1808, 0x70067004, 0x0000f80f,     // RICON_TEXT_A
    0x78000000, 0x50004000, 0x00004800, 0x03c003c0, 0x03c003c0, 0x00100000, 0x0002000a, 0x0000000e,     // RICON_SCALE
    0x75560000, 0x5e004002, 0x54001002, 0x41001202, 0x408200fe, 0x40820082, 0x40820082, 0x00006afe,     // RICON_RESIZE
    0x00000000, 0x3f003f00, 0x3f003f00, 0x3f003f00, 0x00400080, 0x001c0020, 0x001c001c, 0x00000000,     // RICON_FILTER_POINT
    0x6d800000, 0x00004080, 0x40804080, 0x40800000, 0x00406d80, 0x001c0020, 0x001c001c, 0x00000000,     // RICON_FILTER_BILINEAR
    0x40080000, 0x1ffe2008, 0x14081008, 0x11081208, 0x10481088, 0x10081028, 0x10047ff8, 0x00001002,     // RICON_CROP
    0x00100000, 0x3ffc0010, 0x2ab03550, 0x22b02550, 0x20b02150, 0x20302050, 0x2000fff0, 0x00002000,     // RICON_CROP_ALPHA
    0x40000000, 0x1ff82000, 0x04082808, 0x01082208, 0x00482088, 0x00182028, 0x35542008, 0x00000002,     // RICON_SQUARE_TOGGLE
    0x00000000, 0x02800280, 0x06c006c0, 0x0ea00ee0, 0x1e901eb0, 0x3e883e98, 0x7efc7e8c, 0x00000000,     // RICON_SIMMETRY
    0x01000000, 0x05600100, 0x1d480d50, 0x7d423d44, 0x3d447d42, 0x0d501d48, 0x01000560, 0x00000100,     // RICON_SIMMETRY_HORIZONTAL
    0x01800000, 0x04200240, 0x10080810, 0x00001ff8, 0x00007ffe, 0x0ff01ff8, 0x03c007e0, 0x00000180,     // RICON_SIMMETRY_VERTICAL
    0x00000000, 0x010800f0, 0x02040204, 0x02040204, 0x07f00308, 0x1c000e00, 0x30003800, 0x00000000,     // RICON_LENS
    0x00000000, 0x061803f0, 0x08240c0c, 0x08040814, 0x0c0c0804, 0x23f01618, 0x18002400, 0x00000000,     // RICON_LENS_BIG
    0x00000000, 0x00000000, 0x1c7007c0, 0x638e3398, 0x1c703398, 0x000007c0, 0x00000000, 0x00000000,     // RICON_EYE_ON
    0x00000000, 0x10002000, 0x04700fc0, 0x610e3218, 0x1c703098, 0x001007a0, 0x00000008, 0x00000000,     // RICON_EYE_OFF
    0x00000000, 0x00007ffc, 0x40047ffc, 0x10102008, 0x04400820, 0x02800280, 0x02800280, 0x00000100,     // RICON_FILTER_TOP
    0x00000000, 0x40027ffe, 0x10082004, 0x04200810, 0x02400240, 0x02400240, 0x01400240, 0x000000c0,     // RICON_FILTER
    0x00800000, 0x00800080, 0x00000080, 0x3c9e0000, 0x00000000, 0x00800080, 0x00800080, 0x00000000,     // RICON_TARGET_POINT
    0x00800000, 0x00800080, 0x00800080, 0x3f7e01c0, 0x008001c0, 0x00800080, 0x00800080, 0x00000000,     // RICON_TARGET_SMALL
    0x00800000, 0x00800080, 0x03e00080, 0x3e3e0220, 0x03e00220, 0x00800080, 0x00800080, 0x00000000,     // RICON_TARGET_BIG
    0x01000000, 0x04400280, 0x01000100, 0x43842008, 0x43849ab2, 0x01002008, 0x04400100, 0x01000280,     // RICON_TARGET_MOVE
    0x01000000, 0x04400280, 0x01000100, 0x41042108, 0x41049ff2, 0x01002108, 0x04400100, 0x01000280,     // RICON_CURSOR_MOVE
    0x781e0000, 0x500a4002, 0x04204812, 0x00000240, 0x02400000, 0x48120420, 0x4002500a, 0x0000781e,     // RICON_CURSOR_SCALE
    0x00000000, 0x20003c00, 0x24002800, 0x01000200, 0x00400080, 0x00140024, 0x003c0004, 0x00000000,     // RICON_CURSOR_SCALE_RIGHT
    0x00000000, 0x0004003c, 0x00240014, 0x00800040, 0x02000100, 0x28002400, 0x3c002000, 0x00000000,     // RICON_CURSOR_SCALE_LEFT
    0x00000000, 0x00100020, 0x10101fc8, 0x10001020, 0x10001000, 0x10001000, 0x00001fc0, 0x00000000,     // RICON_UNDO
    0x00000000, 0x08000400, 0x080813f8, 0x00080408, 0x00080008, 0x00080008, 0x000003f8, 0x00000000,     // RICON_REDO
    0x00000000, 0x3ffc0000, 0x20042004, 0x20002000, 0x20402000, 0x3f902020, 0x00400020, 0x00000000,     // RICON_REREDO
    0x00000000, 0x3ffc0000, 0x20042004, 0x27fc2004, 0x20202000, 0x3fc82010, 0x00200010, 0x00000000,     // RICON_MUTATE
    0x00000000, 0x0ff00000, 0x10081818, 0x11801008, 0x10001180, 0x18101020, 0x00100fc8, 0x00000020,     // RICON_ROTATE
    0x00000000, 0x04000200, 0x240429fc, 0x20042204, 0x20442004, 0x3f942024, 0x00400020, 0x00000000,     // RICON_REPEAT
    0x00000000, 0x20001000, 0x22104c0e, 0x00801120, 0x11200040, 0x4c0e2210, 0x10002000, 0x00000000,     // RICON_SHUFFLE
    0x7ffe0000, 0x50024002, 0x44024802, 0x41024202, 0x40424082, 0x40124022, 0x4002400a, 0x00007ffe,     // RICON_EMPTYBOX
    0x00800000, 0x03e00080, 0x08080490, 0x3c9e0808, 0x08080808, 0x03e00490, 0x00800080, 0x00000000,     // RICON_TARGET
    0x00800000, 0x00800080, 0x00800080, 0x3ffe01c0, 0x008001c0, 0x00800080, 0x00800080, 0x00000000,     // RICON_TARGET_SMALL_FILL
    0x00800000, 0x00800080, 0x03e00080, 0x3ffe03e0, 0x03e003e0, 0x00800080, 0x00800080, 0x00000000,     // RICON_TARGET_BIG_FILL
    0x01000000, 0x07c00380, 0x01000100, 0x638c2008, 0x638cfbbe, 0x01002008, 0x07c00100, 0x01000380,     // RICON_TARGET_MOVE_FILL
    0x01000000, 0x07c00380, 0x01000100, 0x610c2108, 0x610cfffe, 0x01002108, 0x07c00100, 0x01000380,     // RICON_CURSOR_MOVE_FILL
    0x781e0000, 0x6006700e, 0x04204812, 0x00000240, 0x02400000, 0x48120420, 0x700e6006, 0x0000781e,     // RICON_CURSOR_SCALE_FILL
    0x00000000, 0x38003c00, 0x24003000, 0x01000200, 0x00400080, 0x000c0024, 0x003c001c, 0x00000000,     // RICON_CURSOR_SCALE_RIGHT
    0x00000000, 0x001c003c, 0x0024000c, 0x00800040, 0x02000100, 0x30002400, 0x3c003800, 0x00000000,     // RICON_CURSOR_SCALE_LEFT
    0x00000000, 0x00300020, 0x10301ff8, 0x10001020, 0x10001000, 0x10001000, 0x00001fc0, 0x00000000,     // RICON_UNDO_FILL
    0x00000000, 0x0c000400, 0x0c081ff8, 0x00080408, 0x00080008, 0x00080008, 0x000003f8, 0x00000000,     // RICON_REDO_FILL
    0x00000000, 0x3ffc0000, 0x20042004, 0x20002000, 0x20402000, 0x3ff02060, 0x00400060, 0x00000000,     // RICON_REREDO_FILL
    0x00000000, 0x3ffc0000, 0x20042004, 0x27fc2004, 0x20202000, 0x3ff82030, 0x00200030, 0x00000000,     // RICON_MUTATE_FILL
    0x00000000, 0x0ff00000, 0x10081818, 0x11801008, 0x10001180, 0x18301020, 0x00300ff8, 0x00000020,     // RICON_ROTATE_FILL
    0x00000000, 0x06000200, 0x26042ffc, 0x20042204, 0x20442004, 0x3ff42064, 0x00400060, 0x00000000,     // RICON_REPEAT_FILL
    0x00000000, 0x30001000, 0x32107c0e, 0x00801120, 0x11200040, 0x7c0e3210, 0x10003000, 0x00000000,     // RICON_SHUFFLE_FILL
    0x00000000, 0x30043ffc, 0x24042804, 0x21042204, 0x20442084, 0x20142024, 0x3ffc200c, 0x00000000,     // RICON_EMPTYBOX_SMALL
    0x00000000, 0x20043ffc, 0x20042004, 0x20042004, 0x20042004, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX
    0x00000000, 0x23c43ffc, 0x23c423c4, 0x200423c4, 0x20042004, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_TOP
    0x00000000, 0x3e043ffc, 0x3e043e04, 0x20043e04, 0x20042004, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_TOP_RIGHT
    0x00000000, 0x20043ffc, 0x20042004, 0x3e043e04, 0x3e043e04, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_RIGHT
    0x00000000, 0x20043ffc, 0x20042004, 0x20042004, 0x3e042004, 0x3e043e04, 0x3ffc3e04, 0x00000000,     // RICON_BOX_BOTTOM_RIGHT
    0x00000000, 0x20043ffc, 0x20042004, 0x20042004, 0x23c42004, 0x23c423c4, 0x3ffc23c4, 0x00000000,     // RICON_BOX_BOTTOM
    0x00000000, 0x20043ffc, 0x20042004, 0x20042004, 0x207c2004, 0x207c207c, 0x3ffc207c, 0x00000000,     // RICON_BOX_BOTTOM_LEFT
    0x00000000, 0x20043ffc, 0x20042004, 0x207c207c, 0x207c207c, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_LEFT
    0x00000000, 0x207c3ffc, 0x207c207c, 0x2004207c, 0x20042004, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_TOP_LEFT
    0x00000000, 0x20043ffc, 0x20042004, 0x23c423c4, 0x23c423c4, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_CIRCLE_MASK
    0x7ffe0000, 0x40024002, 0x47e24182, 0x4ff247e2, 0x47e24ff2, 0x418247e2, 0x40024002, 0x00007ffe,     // RICON_BOX_CENTER
    0x7fff0000, 0x40014001, 0x40014001, 0x49555ddd, 0x4945495d, 0x400149c5, 0x40014001, 0x00007fff,     // RICON_POT
    0x7ffe0000, 0x53327332, 0x44ce4cce, 0x41324332, 0x404e40ce, 0x48125432, 0x4006540e, 0x00007ffe,     // RICON_ALPHA_MULTIPLY
    0x7ffe0000, 0x53327332, 0x44ce4cce, 0x41324332, 0x5c4e40ce, 0x44124432, 0x40065c0e, 0x00007ffe,     // RICON_ALPHA_CLEAR
    0x7ffe0000, 0x42fe417e, 0x42fe417e, 0x42fe417e, 0x42fe417e, 0x42fe417e, 0x42fe417e, 0x00007ffe,     // RICON_DITHERING
    0x07fe0000, 0x1ffa0002, 0x7fea000a, 0x402a402a, 0x5b2a512a, 0x5128552a, 0x40205128, 0x00007fe0,     // RICON_MIPMAPS
    0x00000000, 0x1ff80000, 0x12481248, 0x12481ff8, 0x1ff81248, 0x12481248, 0x00001ff8, 0x00000000,     // RICON_BOX_GRID
    0x12480000, 0x7ffe1248, 0x12481248, 0x12487ffe, 0x7ffe1248, 0x12481248, 0x12487ffe, 0x00001248,     // RICON_GRID
    0x00000000, 0x1c380000, 0x1c3817e8, 0x08100810, 0x08100810, 0x17e81c38, 0x00001c38, 0x00000000,     // RICON_BOX_CORNERS_SMALL
    0x700e0000, 0x700e5ffa, 0x20042004, 0x20042004, 0x20042004, 0x20042004, 0x5ffa700e, 0x0000700e,     // RICON_BOX_CORNERS_BIG
    0x3f7e0000, 0x21422142, 0x21422142, 0x00003f7e, 0x21423f7e, 0x21422142, 0x3f7e2142, 0x00000000,     // RICON_FOUR_BOXES
    0x00000000, 0x3bb80000, 0x3bb83bb8, 0x3bb80000, 0x3bb83bb8, 0x3bb80000, 0x3bb83bb8, 0x00000000,     // RICON_GRID_FILL
    0x7ffe0000, 0x7ffe7ffe, 0x77fe7000, 0x77fe77fe, 0x777e7700, 0x777e777e, 0x777e777e, 0x0000777e,     // RICON_BOX_MULTISIZE
    0x781e0000, 0x40024002, 0x00004002, 0x01800000, 0x00000180, 0x40020000, 0x40024002, 0x0000781e,     // RICON_ZOOM_SMALL
    0x781e0000, 0x40024002, 0x00004002, 0x03c003c0, 0x03c003c0, 0x40020000, 0x40024002, 0x0000781e,     // RICON_ZOOM_MEDIUM
    0x781e0000, 0x40024002, 0x07e04002, 0x07e007e0, 0x07e007e0, 0x400207e0, 0x40024002, 0x0000781e,     // RICON_ZOOM_BIG
    0x781e0000, 0x5ffa4002, 0x1ff85ffa, 0x1ff81ff8, 0x1ff81ff8, 0x5ffa1ff8, 0x40025ffa, 0x0000781e,     // RICON_ZOOM_ALL
    0x00000000, 0x2004381c, 0x00002004, 0x00000000, 0x00000000, 0x20040000, 0x381c2004, 0x00000000,     // RICON_ZOOM_CENTER
    0x00000000, 0x1db80000, 0x10081008, 0x10080000, 0x00001008, 0x10081008, 0x00001db8, 0x00000000,     // RICON_BOX_DOTS_SMALL
    0x35560000, 0x00002002, 0x00002002, 0x00002002, 0x00002002, 0x00002002, 0x35562002, 0x00000000,     // RICON_BOX_DOTS_BIG
    0x7ffe0000, 0x40024002, 0x48124ff2, 0x49924812, 0x48124992, 0x4ff24812, 0x40024002, 0x00007ffe,     // RICON_BOX_CONCENTRIC
    0x00000000, 0x10841ffc, 0x10841084, 0x1ffc1084, 0x10841084, 0x10841084, 0x00001ffc, 0x00000000,     // RICON_BOX_GRID_BIG
    0x00000000, 0x00000000, 0x10000000, 0x04000800, 0x01040200, 0x00500088, 0x00000020, 0x00000000,     // RICON_OK_TICK
    0x00000000, 0x10080000, 0x04200810, 0x01800240, 0x02400180, 0x08100420, 0x00001008, 0x00000000,     // RICON_CROSS
    0x00000000, 0x02000000, 0x00800100, 0x00200040, 0x00200010, 0x00800040, 0x02000100, 0x00000000,     // RICON_ARROW_LEFT
    0x00000000, 0x00400000, 0x01000080, 0x04000200, 0x04000800, 0x01000200, 0x00400080, 0x00000000,     // RICON_ARROW_RIGHT
    0x00000000, 0x00000000, 0x00000000, 0x08081004, 0x02200410, 0x00800140, 0x00000000, 0x00000000,     // RICON_ARROW_DOWN
    0x00000000, 0x00000000, 0x01400080, 0x04100220, 0x10040808, 0x00000000, 0x00000000, 0x00000000,     // RICON_ARROW_UP
    0x00000000, 0x02000000, 0x03800300, 0x03e003c0, 0x03e003f0, 0x038003c0, 0x02000300, 0x00000000,     // RICON_ARROW_LEFT_FILL
    0x00000000, 0x00400000, 0x01c000c0, 0x07c003c0, 0x07c00fc0, 0x01c003c0, 0x004000c0, 0x00000000,     // RICON_ARROW_RIGHT_FILL
    0x00000000, 0x00000000, 0x00000000, 0x0ff81ffc, 0x03e007f0, 0x008001c0, 0x00000000, 0x00000000,     // RICON_ARROW_DOWN_FILL
    0x00000000, 0x00000000, 0x01c00080, 0x07f003e0, 0x1ffc0ff8, 0x00000000, 0x00000000, 0x00000000,     // RICON_ARROW_UP_FILL
    0x00000000, 0x18a008c0, 0x32881290, 0x24822686, 0x26862482, 0x12903288, 0x08c018a0, 0x00000000,     // RICON_AUDIO
    0x00000000, 0x04800780, 0x004000c0, 0x662000f0, 0x08103c30, 0x130a0e18, 0x0000318e, 0x00000000,     // RICON_FX
    0x00000000, 0x00800000, 0x08880888, 0x2aaa0a8a, 0x0a8a2aaa, 0x08880888, 0x00000080, 0x00000000,     // RICON_WAVE
    0x00000000, 0x00600000, 0x01080090, 0x02040108, 0x42044204, 0x24022402, 0x00001800, 0x00000000,     // RICON_WAVE_SINUS
    0x00000000, 0x07f80000, 0x04080408, 0x04080408, 0x04080408, 0x7c0e0408, 0x00000000, 0x00000000,     // RICON_WAVE_SQUARE
    0x00000000, 0x00000000, 0x00a00040, 0x22084110, 0x08021404, 0x00000000, 0x00000000, 0x00000000,     // RICON_WAVE_TRIANGULAR
    0x00000000, 0x00000000, 0x04200000, 0x01800240, 0x02400180, 0x00000420, 0x00000000, 0x00000000,     // RICON_CROSS_SMALL
    0x00000000, 0x18380000, 0x12281428, 0x10a81128, 0x112810a8, 0x14281228, 0x00001838, 0x00000000,     // RICON_PLAYER_PREVIOUS
    0x00000000, 0x18000000, 0x11801600, 0x10181060, 0x10601018, 0x16001180, 0x00001800, 0x00000000,     // RICON_PLAYER_PLAY_BACK
    0x00000000, 0x00180000, 0x01880068, 0x18080608, 0x06081808, 0x00680188, 0x00000018, 0x00000000,     // RICON_PLAYER_PLAY
    0x00000000, 0x1e780000, 0x12481248, 0x12481248, 0x12481248, 0x12481248, 0x00001e78, 0x00000000,     // RICON_PLAYER_PAUSE
    0x00000000, 0x1ff80000, 0x10081008, 0x10081008, 0x10081008, 0x10081008, 0x00001ff8, 0x00000000,     // RICON_PLAYER_STOP
    0x00000000, 0x1c180000, 0x14481428, 0x15081488, 0x14881508, 0x14281448, 0x00001c18, 0x00000000,     // RICON_PLAYER_NEXT
    0x00000000, 0x03c00000, 0x08100420, 0x10081008, 0x10081008, 0x04200810, 0x000003c0, 0x00000000,     // RICON_PLAYER_RECORD
    0x00000000, 0x0c3007e0, 0x13c81818, 0x14281668, 0x14281428, 0x1c381c38, 0x08102244, 0x00000000,     // RICON_MAGNET
    0x07c00000, 0x08200820, 0x3ff80820, 0x23882008, 0x21082388, 0x20082108, 0x1ff02008, 0x00000000,     // RICON_LOCK_CLOSE
    0x07c00000, 0x08000800, 0x3ff80800, 0x23882008, 0x21082388, 0x20082108, 0x1ff02008, 0x00000000,     // RICON_LOCK_OPEN
    0x01c00000, 0x0c180770, 0x3086188c, 0x60832082, 0x60034781, 0x30062002, 0x0c18180c, 0x01c00770,     // RICON_CLOCK
    0x0a200000, 0x1b201b20, 0x04200e20, 0x04200420, 0x04700420, 0x0e700e70, 0x0e700e70, 0x04200e70,     // RICON_TOOLS
    0x01800000, 0x3bdc318c, 0x0ff01ff8, 0x7c3e1e78, 0x1e787c3e, 0x1ff80ff0, 0x318c3bdc, 0x00000180,     // RICON_GEAR
    0x01800000, 0x3ffc318c, 0x1c381ff8, 0x781e1818, 0x1818781e, 0x1ff81c38, 0x318c3ffc, 0x00000180,     // RICON_GEAR_BIG
    0x00000000, 0x08080ff8, 0x08081ffc, 0x0aa80aa8, 0x0aa80aa8, 0x0aa80aa8, 0x08080aa8, 0x00000ff8,     // RICON_BIN
    0x00000000, 0x00000000, 0x20043ffc, 0x08043f84, 0x04040f84, 0x04040784, 0x000007fc, 0x00000000,     // RICON_HAND_POINTER
    0x00000000, 0x24400400, 0x00001480, 0x6efe0e00, 0x00000e00, 0x24401480, 0x00000400, 0x00000000,     // RICON_LASER
    0x00000000, 0x03c00000, 0x08300460, 0x11181118, 0x11181118, 0x04600830, 0x000003c0, 0x00000000,     // RICON_COIN
    0x00000000, 0x10880080, 0x06c00810, 0x366c07e0, 0x07e00240, 0x00001768, 0x04200240, 0x00000000,     // RICON_EXPLOSION
    0x00000000, 0x3d280000, 0x2528252c, 0x3d282528, 0x05280528, 0x05e80528, 0x00000000, 0x00000000,     // RICON_1UP
    0x01800000, 0x03c003c0, 0x018003c0, 0x0ff007e0, 0x0bd00bd0, 0x0a500bd0, 0x02400240, 0x02400240,     // RICON_PLAYER
    0x01800000, 0x03c003c0, 0x118013c0, 0x03c81ff8, 0x07c003c8, 0x04400440, 0x0c080478, 0x00000000,     // RICON_PLAYER_JUMP
    0x3ff80000, 0x30183ff8, 0x30183018, 0x3ff83ff8, 0x03000300, 0x03c003c0, 0x03e00300, 0x000003e0,     // RICON_KEY
    0x3ff80000, 0x3ff83ff8, 0x33983ff8, 0x3ff83398, 0x3ff83ff8, 0x00000540, 0x0fe00aa0, 0x00000fe0,     // RICON_DEMON
    0x00000000, 0x0ff00000, 0x20041008, 0x25442004, 0x10082004, 0x06000bf0, 0x00000300, 0x00000000,     // RICON_TEXT_POPUP
    0x00000000, 0x11440000, 0x07f00be8, 0x1c1c0e38, 0x1c1c0c18, 0x07f00e38, 0x11440be8, 0x00000000,     // RICON_GEAR_EX
    0x00000000, 0x20080000, 0x0c601010, 0x07c00fe0, 0x07c007c0, 0x0c600fe0, 0x20081010, 0x00000000,     // RICON_CRACK
    0x00000000, 0x20080000, 0x0c601010, 0x04400fe0, 0x04405554, 0x0c600fe0, 0x20081010, 0x00000000,     // RICON_CRACK_POINTS
    0x00000000, 0x00800080, 0x01c001c0, 0x1ffc3ffe, 0x03e007f0, 0x07f003e0, 0x0c180770, 0x00000808,     // RICON_STAR
    0x0ff00000, 0x08180810, 0x08100818, 0x0a100810, 0x08180810, 0x08100818, 0x08100810, 0x00001ff8,     // RICON_DOOR
    0x0ff00000, 0x08100810, 0x08100810, 0x10100010, 0x4f902010, 0x10102010, 0x08100010, 0x00000ff0,     // RICON_EXIT
    0x00040000, 0x001f000e, 0x0ef40004, 0x12f41284, 0x0ef41214, 0x10040004, 0x7ffc3004, 0x10003000,     // RICON_MODE_2D
    0x78040000, 0x501f600e, 0x0ef44004, 0x12f41284, 0x0ef41284, 0x10140004, 0x7ffc300c, 0x10003000,     // RICON_MODE_3D
    0x7fe00000, 0x50286030, 0x47fe4804, 0x44224402, 0x44224422, 0x241275e2, 0x0c06140a, 0x000007fe,     // RICON_CUBE
    0x7fe00000, 0x5ff87ff0, 0x47fe4ffc, 0x44224402, 0x44224422, 0x241275e2, 0x0c06140a, 0x000007fe,     // RICON_CUBE_FACE_TOP
    0x7fe00000, 0x50386030, 0x47fe483c, 0x443e443e, 0x443e443e, 0x241e75fe, 0x0c06140e, 0x000007fe,     // RICON_CUBE_FACE_LEFT
    0x7fe00000, 0x50286030, 0x47fe4804, 0x47fe47fe, 0x47fe47fe, 0x27fe77fe, 0x0ffe17fe, 0x000007fe,     // RICON_CUBE_FACE_FRONT
    0x7fe00000, 0x50286030, 0x47fe4804, 0x44224402, 0x44224422, 0x3ff27fe2, 0x0ffe1ffa, 0x000007fe,     // RICON_CUBE_FACE_BOTTOM
    0x7fe00000, 0x70286030, 0x7ffe7804, 0x7c227c02, 0x7c227c22, 0x3c127de2, 0x0c061c0a, 0x000007fe,     // RICON_CUBE_FACE_RIGHT
    0x7fe00000, 0x7fe87ff0, 0x7ffe7fe4, 0x7fe27fe2, 0x7fe27fe2, 0x24127fe2, 0x0c06140a, 0x000007fe,     // RICON_CUBE_FACE_BACK
    0x00000000, 0x2a0233fe, 0x22022602, 0x22022202, 0x2a022602, 0x00a033fe, 0x02080110, 0x00000000,     // RICON_CAMERA
    0x00000000, 0x200c3ffc, 0x000c000c, 0x3ffc000c, 0x30003000, 0x30003000, 0x3ffc3004, 0x00000000,     // RICON_SPECIAL
    0x00000000, 0x0022003e, 0x012201e2, 0x0100013e, 0x01000100, 0x79000100, 0x4f004900, 0x00007800,     // RICON_LINK_NET
    0x00000000, 0x44007c00, 0x45004600, 0x00627cbe, 0x00620022, 0x45007cbe, 0x44004600, 0x00007c00,     // RICON_LINK_BOXES
    0x00000000, 0x0044007c, 0x0010007c, 0x3f100010, 0x3f1021f0, 0x3f100010, 0x3f0021f0, 0x00000000,     // RICON_LINK_MULTI
    0x00000000, 0x0044007c, 0x00440044, 0x0010007c, 0x00100010, 0x44107c10, 0x440047f0, 0x00007c00,     // RICON_LINK
    0x00000000, 0x0044007c, 0x00440044, 0x0000007c, 0x00000010, 0x44007c10, 0x44004550, 0x00007c00,     // RICON_LINK_BROKE
    0x02a00000, 0x22a43ffc, 0x20042004, 0x20042ff4, 0x20042ff4, 0x20042ff4, 0x20042004, 0x00003ffc,     // RICON_TEXT_NOTES
    0x3ffc0000, 0x20042004, 0x245e27c4, 0x27c42444, 0x2004201e, 0x201e2004, 0x20042004, 0x00003ffc,     // RICON_NOTEBOOK
    0x00000000, 0x07e00000, 0x04200420, 0x24243ffc, 0x24242424, 0x24242424, 0x3ffc2424, 0x00000000,     // RICON_SUITCASE
    0x00000000, 0x0fe00000, 0x08200820, 0x40047ffc, 0x7ffc5554, 0x40045554, 0x7ffc4004, 0x00000000,     // RICON_SUITCASE_ZIP
    0x00000000, 0x20043ffc, 0x3ffc2004, 0x13c81008, 0x100813c8, 0x10081008, 0x1ff81008, 0x00000000,     // RICON_MAILBOX
    0x00000000, 0x40027ffe, 0x5ffa5ffa, 0x5ffa5ffa, 0x40025ffa, 0x03c07ffe, 0x1ff81ff8, 0x00000000,     // RICON_MONITOR
    0x0ff00000, 0x6bfe7ffe, 0x7ffe7ffe, 0x68167ffe, 0x08106816, 0x08100810, 0x0ff00810, 0x00000000,     // RICON_PRINTER
    0x3ff80000, 0xfffe2008, 0x870a8002, 0x904a888a, 0x904a904a, 0x870a888a, 0xfffe8002, 0x00000000,     // RICON_PHOTO_CAMERA
    0x0fc00000, 0xfcfe0cd8, 0x8002fffe, 0x84428382, 0x84428442, 0x80028382, 0xfffe8002, 0x00000000,     // RICON_PHOTO_CAMERA_FLASH
    0x00000000, 0x02400180, 0x08100420, 0x20041008, 0x23c42004, 0x22442244, 0x3ffc2244, 0x00000000,     // RICON_HOUSE
    0x00000000, 0x1c700000, 0x3ff83ef8, 0x3ff83ff8, 0x0fe01ff0, 0x038007c0, 0x00000100, 0x00000000,     // RICON_HEART
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x80000000, 0xe000c000,     // RICON_CORNER
    0x00000000, 0x14001c00, 0x15c01400, 0x15401540, 0x155c1540, 0x15541554, 0x1ddc1554, 0x00000000,     // RICON_VERTICAL_BARS
    0x00000000, 0x03000300, 0x1b001b00, 0x1b601b60, 0x1b6c1b60, 0x1b6c1b6c, 0x1b6c1b6c, 0x00000000,     // RICON_VERTICAL_BARS_FILL
    0x00000000, 0x00000000, 0x403e7ffe, 0x7ffe403e, 0x7ffe0000, 0x43fe43fe, 0x00007ffe, 0x00000000,     // RICON_LIFE_BARS
    0x7ffc0000, 0x43844004, 0x43844284, 0x43844004, 0x42844284, 0x42844284, 0x40044384, 0x00007ffc,     // RICON_INFO
    0x40008000, 0x10002000, 0x04000800, 0x01000200, 0x00400080, 0x00100020, 0x00040008, 0x00010002,     // RICON_CROSSLINE
    0x00000000, 0x1ff01ff0, 0x18301830, 0x1f001830, 0x03001f00, 0x00000300, 0x03000300, 0x00000000,     // RICON_HELP
    0x3ff00000, 0x2abc3550, 0x2aac3554, 0x2aac3554, 0x2aac3554, 0x2aac3554, 0x2aac3554, 0x00003ffc,     // RICON_FILETYPE_ALPHA
    0x3ff00000, 0x201c2010, 0x22442184, 0x28142424, 0x29942814, 0x2ff42994, 0x20042004, 0x00003ffc,     // RICON_FILETYPE_HOME
    0x07fe0000, 0x04020402, 0x7fe20402, 0x44224422, 0x44224422, 0x402047fe, 0x40204020, 0x00007fe0,     // RICON_LAYERS_VISIBLE
    0x07fe0000, 0x04020402, 0x7c020402, 0x44024402, 0x44024402, 0x402047fe, 0x40204020, 0x00007fe0,     // RICON_LAYERS
    0x00000000, 0x40027ffe, 0x7ffe4002, 0x40024002, 0x40024002, 0x40024002, 0x7ffe4002, 0x00000000,     // RICON_WINDOW
    0x09100000, 0x09f00910, 0x09100910, 0x00000910, 0x24a2779e, 0x27a224a2, 0x709e20a2, 0x00000000,     // RICON_HIDPI
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_200
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_201
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_202
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_203
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_204
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_205
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_206
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_207
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_208
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_209
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_210
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_211
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_212
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_213
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_214
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_215
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_216
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_217
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_218
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_219
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_220
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_221
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_222
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_223
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_224
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_225
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_226
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_227
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_228
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_229
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_230
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_231
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_232
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_233
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_234
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_235
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_236
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_237
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_238
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_239
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_240
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_241
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_242
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_243
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_244
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_245
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_246
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_247
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_248
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_249
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_250
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_251
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_252
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_253
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_254
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_255
};

#endif      // RAYGUI_CUSTOM_RICONS

#endif      // !RAYGUI_NO_RICONS

#ifndef RICON_SIZE
    #define RICON_SIZE                   0
#endif

#define RAYGUI_MAX_CONTROLS             16      // Maximum number of standard controls
#define RAYGUI_MAX_PROPS_BASE           16      // Maximum number of standard properties
#define RAYGUI_MAX_PROPS_EXTENDED        8      // Maximum number of extended properties

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Gui control property style color element
typedef enum { BORDER = 0, BASE, TEXT, OTHER } GuiPropertyElement;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static GuiControlState guiState = GUI_STATE_NORMAL;

static Font guiFont = { 0 };            // Gui current font (WARNING: highly coupled to raylib)
static bool guiLocked = false;          // Gui lock state (no inputs processed)
static float guiAlpha = 1.0f;           // Gui element transpacency on drawing

//----------------------------------------------------------------------------------
// Style data array for all gui style properties (allocated on data segment by default)
//
// NOTE 1: First set of BASE properties are generic to all controls but could be individually
// overwritten per control, first set of EXTENDED properties are generic to all controls and
// can not be overwritten individually but custom EXTENDED properties can be used by control
//
// NOTE 2: A new style set could be loaded over this array using GuiLoadStyle(),
// but default gui style could always be recovered with GuiLoadStyleDefault()
//
// guiStyle size is by default: 16*(16 + 8) = 384*4 = 1536 bytes = 1.5 KB
//----------------------------------------------------------------------------------
static unsigned int guiStyle[RAYGUI_MAX_CONTROLS*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED)] = { 0 };

static bool guiStyleLoaded = false;     // Style loaded flag for lazy style initialization

//----------------------------------------------------------------------------------
// Standalone Mode Functions Declaration
//
// NOTE: raygui depend on some raylib input and drawing functions
// To use raygui as standalone library, below functions must be defined by the user
//----------------------------------------------------------------------------------
#if defined(RAYGUI_STANDALONE)

#define KEY_RIGHT           262
#define KEY_LEFT            263
#define KEY_DOWN            264
#define KEY_UP              265
#define KEY_BACKSPACE       259
#define KEY_ENTER           257

#define MOUSE_LEFT_BUTTON     0

// Input required functions
//-------------------------------------------------------------------------------
static Vector2 GetMousePosition(void);
static float GetMouseWheelMove(void);
static bool IsMouseButtonDown(int button);
static bool IsMouseButtonPressed(int button);
static bool IsMouseButtonReleased(int button);

static bool IsKeyDown(int key);
static bool IsKeyPressed(int key);
static int GetCharPressed(void);         // -- GuiTextBox(), GuiTextBoxMulti(), GuiValueBox()
//-------------------------------------------------------------------------------

// Drawing required functions
//-------------------------------------------------------------------------------
static void DrawRectangle(int x, int y, int width, int height, Color color);        // -- GuiDrawRectangle(), GuiDrawIcon()

static void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4); // -- GuiColorPicker()
//-------------------------------------------------------------------------------

// Text required functions
//-------------------------------------------------------------------------------
static Font LoadFontEx(const char *fileName, int fontSize, int *fontChars, int glyphCount); // -- GuiLoadStyle()
static Font GetFontDefault(void);                           // -- GuiLoadStyleDefault()
static Texture2D LoadTextureFromImage(Image image);         // -- GuiLoadStyle()
static void SetShapesTexture(Texture2D tex, Rectangle rec); // -- GuiLoadStyle()
static char *LoadFileText(const char *fileName);            // -- GuiLoadStyle()
static const char *GetDirectoryPath(const char *filePath);  // -- GuiLoadStyle()

static Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing);   // -- GetTextWidth(), GuiTextBoxMulti()
static void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint);  // -- GuiDrawText()
//-------------------------------------------------------------------------------

// raylib functions already implemented in raygui
//-------------------------------------------------------------------------------
static Color GetColor(int hexValue);                // Returns a Color struct from hexadecimal value
static int ColorToInt(Color color);                 // Returns hexadecimal value for a Color
static Color Fade(Color color, float alpha);        // Color fade-in or fade-out, alpha goes from 0.0f to 1.0f
static bool CheckCollisionPointRec(Vector2 point, Rectangle rec);   // Check if point is inside rectangle
static const char *TextFormat(const char *text, ...);               // Formatting of text with variables to 'embed'
static const char **TextSplit(const char *text, char delimiter, int *count);    // Split text into multiple strings
static int TextToInteger(const char *text);         // Get integer value from text
static int GetCodepoint(const char *text, int *bytesProcessed);     // Get next codepoint in a UTF-8 encoded text
static const char *CodepointToUTF8(int codepoint, int *byteSize);   // Encode codepoint into UTF-8 text (char array size returned as parameter)

static void DrawRectangleGradientV(int posX, int posY, int width, int height, Color color1, Color color2);  // Draw rectangle vertical gradient
//-------------------------------------------------------------------------------

#endif      // RAYGUI_STANDALONE

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static int GetTextWidth(const char *text);                      // Gui get text width using default font
static Rectangle GetTextBounds(int control, Rectangle bounds);  // Get text bounds considering control bounds
static const char *GetTextIcon(const char *text, int *iconId);  // Get text icon if provided and move text cursor

static void GuiDrawText(const char *text, Rectangle bounds, int alignment, Color tint);         // Gui draw text using default font
static void GuiDrawRectangle(Rectangle rec, int borderWidth, Color borderColor, Color color);   // Gui draw rectangle using default raygui style

static const char **GuiTextSplit(const char *text, int *count, int *textRow);   // Split controls text into multiple strings
static Vector3 ConvertHSVtoRGB(Vector3 hsv);                    // Convert color data from HSV to RGB
static Vector3 ConvertRGBtoHSV(Vector3 rgb);                    // Convert color data from RGB to HSV

//----------------------------------------------------------------------------------
// Gui Setup Functions Definition
//----------------------------------------------------------------------------------
// Enable gui global state
void GuiEnable(void) { guiState = GUI_STATE_NORMAL; }

// Disable gui global state
void GuiDisable(void) { guiState = GUI_STATE_DISABLED; }

// Lock gui global state
void GuiLock(void) { guiLocked = true; }

// Unlock gui global state
void GuiUnlock(void) { guiLocked = false; }

// Check if gui is locked (global state)
bool GuiIsLocked(void) { return guiLocked; }

// Set gui controls alpha global state
void GuiFade(float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    else if (alpha > 1.0f) alpha = 1.0f;

    guiAlpha = alpha;
}

// Set gui state (global state)
void GuiSetState(int state) { guiState = (GuiControlState)state; }

// Get gui state (global state)
int GuiGetState(void) { return guiState; }

// Set custom gui font
// NOTE: Font loading/unloading is external to raygui
void GuiSetFont(Font font)
{
    if (font.texture.id > 0)
    {
        // NOTE: If we try to setup a font but default style has not been
        // lazily loaded before, it will be overwritten, so we need to force
        // default style loading first
        if (!guiStyleLoaded) GuiLoadStyleDefault();

        guiFont = font;
        GuiSetStyle(DEFAULT, TEXT_SIZE, font.baseSize);
    }
}

// Get custom gui font
Font GuiGetFont(void)
{
    return guiFont;
}

// Set control style property value
void GuiSetStyle(int control, int property, int value)
{
    if (!guiStyleLoaded) GuiLoadStyleDefault();
    guiStyle[control*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED) + property] = value;

    // Default properties are propagated to all controls
    if ((control == 0) && (property < RAYGUI_MAX_PROPS_BASE))
    {
        for (int i = 1; i < RAYGUI_MAX_CONTROLS; i++) guiStyle[i*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED) + property] = value;
    }
}

// Get control style property value
int GuiGetStyle(int control, int property)
{
    if (!guiStyleLoaded) GuiLoadStyleDefault();
    return guiStyle[control*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED) + property];
}

//----------------------------------------------------------------------------------
// Gui Controls Functions Definition
//----------------------------------------------------------------------------------

// Window Box control
bool GuiWindowBox(Rectangle bounds, const char *title)
{
    // NOTE: This define is also used by GuiMessageBox() and GuiTextInputBox()
    #define WINDOW_STATUSBAR_HEIGHT        22

    //GuiControlState state = guiState;
    bool clicked = false;

    int statusBarHeight = WINDOW_STATUSBAR_HEIGHT + 2*GuiGetStyle(STATUSBAR, BORDER_WIDTH);
    statusBarHeight += (statusBarHeight%2);

    Rectangle statusBar = { bounds.x, bounds.y, bounds.width, (float)statusBarHeight };
    if (bounds.height < statusBarHeight*2.0f) bounds.height = statusBarHeight*2.0f;

    Rectangle windowPanel = { bounds.x, bounds.y + (float)statusBarHeight - 1, bounds.width, bounds.height - (float)statusBarHeight };
    Rectangle closeButtonRec = { statusBar.x + statusBar.width - GuiGetStyle(STATUSBAR, BORDER_WIDTH) - 20,
                                 statusBar.y + statusBarHeight/2.0f - 18.0f/2.0f, 18, 18 };

    // Update control
    //--------------------------------------------------------------------
    // NOTE: Logic is directly managed by button
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiStatusBar(statusBar, title); // Draw window header as status bar
    GuiPanel(windowPanel);          // Draw window base

    // Draw window close button
    int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
    int tempTextAlignment = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
#if defined(RAYGUI_NO_RICONS)
    clicked = GuiButton(closeButtonRec, "x");
#else
    clicked = GuiButton(closeButtonRec, GuiIconText(RICON_CROSS_SMALL, NULL));
#endif
    GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlignment);
    //--------------------------------------------------------------------

    return clicked;
}

// Group Box control with text name
void GuiGroupBox(Rectangle bounds, const char *text)
{
    #define GROUPBOX_LINE_THICK     1
    #define GROUPBOX_TEXT_PADDING  10

    GuiControlState state = guiState;

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, GROUPBOX_LINE_THICK, bounds.height }, 0, BLANK, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));
    GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y + bounds.height - 1, bounds.width, GROUPBOX_LINE_THICK }, 0, BLANK, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));
    GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x + bounds.width - 1, bounds.y, GROUPBOX_LINE_THICK, bounds.height }, 0, BLANK, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));

    GuiLine(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, bounds.width, 1 }, text);
    //--------------------------------------------------------------------
}

// Line control
void GuiLine(Rectangle bounds, const char *text)
{
    #define LINE_TEXT_PADDING  10

    GuiControlState state = guiState;

    Color color = Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha);

    // Draw control
    //--------------------------------------------------------------------
    if (text == NULL) GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y + bounds.height/2, bounds.width, 1 }, 0, BLANK, color);
    else
    {
        Rectangle textBounds = { 0 };
        textBounds.width = (float)GetTextWidth(text);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + LINE_TEXT_PADDING;
        textBounds.y = bounds.y - (float)GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

        // Draw line with embedded text label: "--- text --------------"
        GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, LINE_TEXT_PADDING - 2, 1 }, 0, BLANK, color);
        GuiLabel(textBounds, text);
        GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x + LINE_TEXT_PADDING + textBounds.width + 4, bounds.y, bounds.width - textBounds.width - LINE_TEXT_PADDING - 4, 1 }, 0, BLANK, color);
    }
    //--------------------------------------------------------------------
}

// Panel control
void GuiPanel(Rectangle bounds)
{
    #define PANEL_BORDER_WIDTH   1

    GuiControlState state = guiState;

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, PANEL_BORDER_WIDTH, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED: LINE_COLOR)), guiAlpha),
                     Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BASE_COLOR_DISABLED : BACKGROUND_COLOR)), guiAlpha));
    //--------------------------------------------------------------------
}

// Scroll Panel control
Rectangle GuiScrollPanel(Rectangle bounds, Rectangle content, Vector2 *scroll)
{
    GuiControlState state = guiState;

    Vector2 scrollPos = { 0.0f, 0.0f };
    if (scroll != NULL) scrollPos = *scroll;

    bool hasHorizontalScrollBar = (content.width > bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH))? true : false;
    bool hasVerticalScrollBar = (content.height > bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH))? true : false;

    // Recheck to account for the other scrollbar being visible
    if (!hasHorizontalScrollBar) hasHorizontalScrollBar = (hasVerticalScrollBar && (content.width > (bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH))))? true : false;
    if (!hasVerticalScrollBar) hasVerticalScrollBar = (hasHorizontalScrollBar && (content.height > (bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH))))? true : false;

    const int horizontalScrollBarWidth = hasHorizontalScrollBar? GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) : 0;
    const int verticalScrollBarWidth =  hasVerticalScrollBar? GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) : 0;
    const Rectangle horizontalScrollBar = { (float)((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? (float)bounds.x + verticalScrollBarWidth : (float)bounds.x) + GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)bounds.y + bounds.height - horizontalScrollBarWidth - GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)bounds.width - verticalScrollBarWidth - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)horizontalScrollBarWidth };
    const Rectangle verticalScrollBar = { (float)((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? (float)bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH) : (float)bounds.x + bounds.width - verticalScrollBarWidth - GuiGetStyle(DEFAULT, BORDER_WIDTH)), (float)bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)verticalScrollBarWidth, (float)bounds.height - horizontalScrollBarWidth - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) };

    // Calculate view area (area without the scrollbars)
    Rectangle view = (GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)?
                RAYGUI_CLITERAL(Rectangle){ bounds.x + verticalScrollBarWidth + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth, bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth } :
                RAYGUI_CLITERAL(Rectangle){ bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth, bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth };

    // Clip view area to the actual content size
    if (view.width > content.width) view.width = content.width;
    if (view.height > content.height) view.height = content.height;

    const float horizontalMin = hasHorizontalScrollBar? ((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? (float)-verticalScrollBarWidth : 0) - (float)GuiGetStyle(DEFAULT, BORDER_WIDTH) : (((float)GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? (float)-verticalScrollBarWidth : 0) - (float)GuiGetStyle(DEFAULT, BORDER_WIDTH);
    const float horizontalMax = hasHorizontalScrollBar? content.width - bounds.width + (float)verticalScrollBarWidth + GuiGetStyle(DEFAULT, BORDER_WIDTH) - (((float)GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? (float)verticalScrollBarWidth : 0) : (float)-GuiGetStyle(DEFAULT, BORDER_WIDTH);
    const float verticalMin = hasVerticalScrollBar? (float)-GuiGetStyle(DEFAULT, BORDER_WIDTH) : (float)-GuiGetStyle(DEFAULT, BORDER_WIDTH);
    const float verticalMax = hasVerticalScrollBar? content.height - bounds.height + (float)horizontalScrollBarWidth + (float)GuiGetStyle(DEFAULT, BORDER_WIDTH) : (float)-GuiGetStyle(DEFAULT, BORDER_WIDTH);

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;

            if (hasHorizontalScrollBar)
            {
                if (IsKeyDown(KEY_RIGHT)) scrollPos.x -= GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
                if (IsKeyDown(KEY_LEFT)) scrollPos.x += GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
            }

            if (hasVerticalScrollBar)
            {
                if (IsKeyDown(KEY_DOWN)) scrollPos.y -= GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
                if (IsKeyDown(KEY_UP)) scrollPos.y += GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
            }

            float wheelMove = GetMouseWheelMove();

            // Horizontal scroll (Shift + Mouse wheel)
            if (hasHorizontalScrollBar && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) scrollPos.x += wheelMove*20;
            else scrollPos.y += wheelMove*20; // Vertical scroll
        }
    }

    // Normalize scroll values
    if (scrollPos.x > -horizontalMin) scrollPos.x = -horizontalMin;
    if (scrollPos.x < -horizontalMax) scrollPos.x = -horizontalMax;
    if (scrollPos.y > -verticalMin) scrollPos.y = -verticalMin;
    if (scrollPos.y < -verticalMax) scrollPos.y = -verticalMax;
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, 0, BLANK, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));        // Draw background

    // Save size of the scrollbar slider
    const int slider = GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE);

    // Draw horizontal scrollbar if visible
    if (hasHorizontalScrollBar)
    {
        // Change scrollbar slider size to show the diff in size between the content width and the widget width
        GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, (int)(((bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth)/(int)content.width)*((int)bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth)));
        scrollPos.x = (float)-GuiScrollBar(horizontalScrollBar, (int)-scrollPos.x, (int)horizontalMin, (int)horizontalMax);
    }

    // Draw vertical scrollbar if visible
    if (hasVerticalScrollBar)
    {
        // Change scrollbar slider size to show the diff in size between the content height and the widget height
        GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, (int)(((bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth)/(int)content.height)*((int)bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth)));
        scrollPos.y = (float)-GuiScrollBar(verticalScrollBar, (int)-scrollPos.y, (int)verticalMin, (int)verticalMax);
    }

    // Draw detail corner rectangle if both scroll bars are visible
    if (hasHorizontalScrollBar && hasVerticalScrollBar)
    {
        Rectangle corner = { (GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE) ? (bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH) + 2) : (horizontalScrollBar.x + horizontalScrollBar.width + 2), verticalScrollBar.y + verticalScrollBar.height + 2, (float)horizontalScrollBarWidth - 4, (float)verticalScrollBarWidth - 4 };
        GuiDrawRectangle(corner, 0, BLANK, Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT + (state*3))), guiAlpha));
    }

    // Draw scrollbar lines depending on current state
    GuiDrawRectangle(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + (state*3))), guiAlpha), BLANK);

    // Set scrollbar slider size back to the way it was before
    GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, slider);
    //--------------------------------------------------------------------

    if (scroll != NULL) *scroll = scrollPos;

    return view;
}

// Label control
void GuiLabel(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;

    // Update control
    //--------------------------------------------------------------------
    // ...
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawText(text, GetTextBounds(LABEL, bounds), GuiGetStyle(LABEL, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LABEL, (state == GUI_STATE_DISABLED)? TEXT_COLOR_DISABLED : TEXT_COLOR_NORMAL)), guiAlpha));
    //--------------------------------------------------------------------
}

// Button control, returns true when clicked
bool GuiButton(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;
    bool pressed = false;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(BUTTON, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(BUTTON, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(BUTTON, BASE + (state*3))), guiAlpha));
    GuiDrawText(text, GetTextBounds(BUTTON, bounds), GuiGetStyle(BUTTON, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(BUTTON, TEXT + (state*3))), guiAlpha));
    //------------------------------------------------------------------

    return pressed;
}

// Label button control
bool GuiLabelButton(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;
    bool pressed = false;

    // NOTE: We force bounds.width to be all text
    float textWidth = MeasureTextEx(guiFont, text, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SPACING)).x;
    if (bounds.width < textWidth) bounds.width = textWidth;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check checkbox state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawText(text, GetTextBounds(LABEL, bounds), GuiGetStyle(LABEL, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return pressed;
}

// Toggle Button control, returns true when active
bool GuiToggle(Rectangle bounds, const char *text, bool active)
{
    GuiControlState state = guiState;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check toggle button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                state = GUI_STATE_NORMAL;
                active = !active;
            }
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (state == GUI_STATE_NORMAL)
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, (active? BORDER_COLOR_PRESSED : (BORDER + state*3)))), guiAlpha), Fade(GetColor(GuiGetStyle(TOGGLE, (active? BASE_COLOR_PRESSED : (BASE + state*3)))), guiAlpha));
        GuiDrawText(text, GetTextBounds(TOGGLE, bounds), GuiGetStyle(TOGGLE, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TOGGLE, (active? TEXT_COLOR_PRESSED : (TEXT + state*3)))), guiAlpha));
    }
    else
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, BORDER + state*3)), guiAlpha), Fade(GetColor(GuiGetStyle(TOGGLE, BASE + state*3)), guiAlpha));
        GuiDrawText(text, GetTextBounds(TOGGLE, bounds), GuiGetStyle(TOGGLE, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TOGGLE, TEXT + state*3)), guiAlpha));
    }
    //--------------------------------------------------------------------

    return active;
}

// Toggle Group control, returns toggled button index
int GuiToggleGroup(Rectangle bounds, const char *text, int active)
{
    #if !defined(TOGGLEGROUP_MAX_ELEMENTS)
        #define TOGGLEGROUP_MAX_ELEMENTS    32
    #endif

    float initBoundsX = bounds.x;

    // Get substrings items from text (items pointers)
    int rows[TOGGLEGROUP_MAX_ELEMENTS] = { 0 };
    int itemCount = 0;
    const char **items = GuiTextSplit(text, &itemCount, rows);

    int prevRow = rows[0];

    for (int i = 0; i < itemCount; i++)
    {
        if (prevRow != rows[i])
        {
            bounds.x = initBoundsX;
            bounds.y += (bounds.height + GuiGetStyle(TOGGLE, GROUP_PADDING));
            prevRow = rows[i];
        }

        if (i == active) GuiToggle(bounds, items[i], true);
        else if (GuiToggle(bounds, items[i], false) == true) active = i;

        bounds.x += (bounds.width + GuiGetStyle(TOGGLE, GROUP_PADDING));
    }

    return active;
}

// Check Box control, returns true when active
bool GuiCheckBox(Rectangle bounds, const char *text, bool checked)
{
    GuiControlState state = guiState;

    Rectangle textBounds = { 0 };

    if (text != NULL)
    {
        textBounds.width = (float)GetTextWidth(text);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(CHECKBOX, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;
        if (GuiGetStyle(CHECKBOX, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_LEFT) textBounds.x = bounds.x - textBounds.width - GuiGetStyle(CHECKBOX, TEXT_PADDING);
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        Rectangle totalBounds = {
            (GuiGetStyle(CHECKBOX, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_LEFT)? textBounds.x : bounds.x,
            bounds.y,
            bounds.width + textBounds.width + GuiGetStyle(CHECKBOX, TEXT_PADDING),
            bounds.height,
        };

        // Check checkbox state
        if (CheckCollisionPointRec(mousePoint, totalBounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) checked = !checked;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(CHECKBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(CHECKBOX, BORDER + (state*3))), guiAlpha), BLANK);

    if (checked)
    {
        Rectangle check = { bounds.x + GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, CHECK_PADDING),
                            bounds.y + GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, CHECK_PADDING),
                            bounds.width - 2*(GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, CHECK_PADDING)),
                            bounds.height - 2*(GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, CHECK_PADDING)) };
        GuiDrawRectangle(check, 0, BLANK, Fade(GetColor(GuiGetStyle(CHECKBOX, TEXT + state*3)), guiAlpha));
    }

    GuiDrawText(text, textBounds, (GuiGetStyle(CHECKBOX, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_RIGHT)? GUI_TEXT_ALIGN_LEFT : GUI_TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return checked;
}

// Combo Box control, returns selected item index
int GuiComboBox(Rectangle bounds, const char *text, int active)
{
    GuiControlState state = guiState;

    bounds.width -= (GuiGetStyle(COMBOBOX, COMBO_BUTTON_WIDTH) + GuiGetStyle(COMBOBOX, COMBO_BUTTON_PADDING));

    Rectangle selector = { (float)bounds.x + bounds.width + GuiGetStyle(COMBOBOX, COMBO_BUTTON_PADDING),
                           (float)bounds.y, (float)GuiGetStyle(COMBOBOX, COMBO_BUTTON_WIDTH), (float)bounds.height };

    // Get substrings items from text (items pointers, lengths and count)
    int itemCount = 0;
    const char **items = GuiTextSplit(text, &itemCount, NULL);

    if (active < 0) active = 0;
    else if (active > itemCount - 1) active = itemCount - 1;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked && (itemCount > 1))
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds) ||
            CheckCollisionPointRec(mousePoint, selector))
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                active += 1;
                if (active >= itemCount) active = 0;
            }

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    // Draw combo box main
    GuiDrawRectangle(bounds, GuiGetStyle(COMBOBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COMBOBOX, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(COMBOBOX, BASE + (state*3))), guiAlpha));
    GuiDrawText(items[active], GetTextBounds(COMBOBOX, bounds), GuiGetStyle(COMBOBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(COMBOBOX, TEXT + (state*3))), guiAlpha));

    // Draw selector using a custom button
    // NOTE: BORDER_WIDTH and TEXT_ALIGNMENT forced values
    int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
    int tempTextAlign = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

    GuiButton(selector, TextFormat("%i/%i", active + 1, itemCount));

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlign);
    GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);
    //--------------------------------------------------------------------

    return active;
}

// Dropdown Box control
// NOTE: Returns mouse click
bool GuiDropdownBox(Rectangle bounds, const char *text, int *active, bool editMode)
{
    GuiControlState state = guiState;
    int itemSelected = *active;
    int itemFocused = -1;

    // Get substrings items from text (items pointers, lengths and count)
    int itemCount = 0;
    const char **items = GuiTextSplit(text, &itemCount, NULL);

    Rectangle boundsOpen = bounds;
    boundsOpen.height = (itemCount + 1)*(bounds.height + GuiGetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_PADDING));

    Rectangle itemBounds = bounds;

    bool pressed = false;       // Check mouse button pressed

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && (editMode || !guiLocked) && (itemCount > 1))
    {
        Vector2 mousePoint = GetMousePosition();

        if (editMode)
        {
            state = GUI_STATE_PRESSED;

            // Check if mouse has been pressed or released outside limits
            if (!CheckCollisionPointRec(mousePoint, boundsOpen))
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
            }

            // Check if already selected item has been pressed again
            if (CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;

            // Check focused and selected item
            for (int i = 0; i < itemCount; i++)
            {
                // Update item rectangle y position for next item
                itemBounds.y += (bounds.height + GuiGetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_PADDING));

                if (CheckCollisionPointRec(mousePoint, itemBounds))
                {
                    itemFocused = i;
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                    {
                        itemSelected = i;
                        pressed = true;     // Item selected, change to editMode = false
                    }
                    break;
                }
            }

            itemBounds = bounds;
        }
        else
        {
            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    pressed = true;
                    state = GUI_STATE_PRESSED;
                }
                else state = GUI_STATE_FOCUSED;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (editMode) GuiPanel(boundsOpen);

    GuiDrawRectangle(bounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BORDER + state*3)), guiAlpha), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BASE + state*3)), guiAlpha));
    GuiDrawText(items[itemSelected], GetTextBounds(DEFAULT, bounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + state*3)), guiAlpha));

    if (editMode)
    {
        // Draw visible items
        for (int i = 0; i < itemCount; i++)
        {
            // Update item rectangle y position for next item
            itemBounds.y += (bounds.height + GuiGetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_PADDING));

            if (i == itemSelected)
            {
                GuiDrawRectangle(itemBounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_PRESSED)), guiAlpha), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_PRESSED)), guiAlpha));
                GuiDrawText(items[i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT_COLOR_PRESSED)), guiAlpha));
            }
            else if (i == itemFocused)
            {
                GuiDrawRectangle(itemBounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_FOCUSED)), guiAlpha), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_FOCUSED)), guiAlpha));
                GuiDrawText(items[i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT_COLOR_FOCUSED)), guiAlpha));
            }
            else GuiDrawText(items[i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT_COLOR_NORMAL)), guiAlpha));
        }
    }

    // Draw arrows (using icon if available)
#if defined(RAYGUI_NO_RICONS)
    GuiDrawText("v", RAYGUI_CLITERAL(Rectangle){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_PADDING), bounds.y + bounds.height/2 - 2, 10, 10 },
                GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))), guiAlpha));
#else
    GuiDrawText("#120#", RAYGUI_CLITERAL(Rectangle){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_PADDING), bounds.y + bounds.height/2 - 6, 10, 10 },
                GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))), guiAlpha));   // RICON_ARROW_DOWN_FILL
#endif
    //--------------------------------------------------------------------

    *active = itemSelected;
    return pressed;
}

// Text Box control, updates input text
// NOTE 2: Returns if KEY_ENTER pressed (useful for data validation)
bool GuiTextBox(Rectangle bounds, char *text, int textSize, bool editMode)
{
    GuiControlState state = guiState;
    bool pressed = false;

    Rectangle cursor = {
        bounds.x + GuiGetStyle(TEXTBOX, TEXT_PADDING) + GetTextWidth(text) + 2,
        bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE),
        4,
        (float)GuiGetStyle(DEFAULT, TEXT_SIZE)*2
    };

    if (cursor.height > bounds.height) cursor.height = bounds.height - GuiGetStyle(TEXTBOX, BORDER_WIDTH)*2;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (editMode)
        {
            state = GUI_STATE_PRESSED;

            int key = GetCharPressed();      // Returns codepoint as Unicode
            int keyCount = (int)strlen(text);

            // Only allow keys in range [32..125]
            if (keyCount < (textSize - 1))
            {
                float maxWidth = (bounds.width - (GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING)*2));

                if ((GetTextWidth(text) < (maxWidth - GuiGetStyle(DEFAULT, TEXT_SIZE))) && (key >= 32))
                {
                    int byteSize = 0;
                    const char *textUTF8 = CodepointToUTF8(key, &byteSize);

                    for (int i = 0; i < byteSize; i++)
                    {
                        text[keyCount] = textUTF8[i];
                        keyCount++;
                    }

                    text[keyCount] = '\0';
                }
            }

            // Delete text
            if (keyCount > 0)
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    keyCount--;
                    text[keyCount] = '\0';
                    if (keyCount < 0) keyCount = 0;
                }
            }

            if (IsKeyPressed(KEY_ENTER) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) pressed = true;

            // Check text alignment to position cursor properly
            int textAlignment = GuiGetStyle(TEXTBOX, TEXT_ALIGNMENT);
            if (textAlignment == GUI_TEXT_ALIGN_CENTER) cursor.x = bounds.x + GetTextWidth(text)/2 + bounds.width/2 + 1;
            else if (textAlignment == GUI_TEXT_ALIGN_RIGHT) cursor.x = bounds.x + bounds.width - GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING);
        }
        else
        {
            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = GUI_STATE_FOCUSED;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (state == GUI_STATE_PRESSED)
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_PRESSED)), guiAlpha));
    }
    else if (state == GUI_STATE_DISABLED)
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_DISABLED)), guiAlpha));
    }
    else GuiDrawRectangle(bounds, 1, Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), BLANK);

    GuiDrawText(text, GetTextBounds(TEXTBOX, bounds), GuiGetStyle(TEXTBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TEXTBOX, TEXT + (state*3))), guiAlpha));

    // Draw cursor
    if (editMode) GuiDrawRectangle(cursor, 0, BLANK, Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER_COLOR_PRESSED)), guiAlpha));
    //--------------------------------------------------------------------

    return pressed;
}

// Spinner control, returns selected value
bool GuiSpinner(Rectangle bounds, const char *text, int *value, int minValue, int maxValue, bool editMode)
{
    GuiControlState state = guiState;

    bool pressed = false;
    int tempValue = *value;

    Rectangle spinner = { bounds.x + GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH) + GuiGetStyle(SPINNER, SPIN_BUTTON_PADDING), bounds.y,
                          bounds.width - 2*(GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH) + GuiGetStyle(SPINNER, SPIN_BUTTON_PADDING)), bounds.height };
    Rectangle leftButtonBound = { (float)bounds.x, (float)bounds.y, (float)GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH), (float)bounds.height };
    Rectangle rightButtonBound = { (float)bounds.x + bounds.width - GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH), (float)bounds.y, (float)GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH), (float)bounds.height };

    Rectangle textBounds = { 0 };
    if (text != NULL)
    {
        textBounds.width = (float)GetTextWidth(text);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(SPINNER, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;
        if (GuiGetStyle(SPINNER, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_LEFT) textBounds.x = bounds.x - textBounds.width - GuiGetStyle(SPINNER, TEXT_PADDING);
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check spinner state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;
        }
    }

    if (!editMode)
    {
        if (tempValue < minValue) tempValue = minValue;
        if (tempValue > maxValue) tempValue = maxValue;
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    // TODO: Set Spinner properties for ValueBox
    pressed = GuiValueBox(spinner, NULL, &tempValue, minValue, maxValue, editMode);

    // Draw value selector custom buttons
    // NOTE: BORDER_WIDTH and TEXT_ALIGNMENT forced values
    int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
    int tempTextAlign = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, GuiGetStyle(SPINNER, BORDER_WIDTH));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

#if defined(RAYGUI_NO_RICONS)
    if (GuiButton(leftButtonBound, "<")) tempValue--;
    if (GuiButton(rightButtonBound, ">")) tempValue++;
#else
    if (GuiButton(leftButtonBound, GuiIconText(RICON_ARROW_LEFT_FILL, NULL))) tempValue--;
    if (GuiButton(rightButtonBound, GuiIconText(RICON_ARROW_RIGHT_FILL, NULL))) tempValue++;
#endif

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlign);
    GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);

    // Draw text label if provided
    GuiDrawText(text, textBounds, (GuiGetStyle(SPINNER, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_RIGHT)? GUI_TEXT_ALIGN_LEFT : GUI_TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    *value = tempValue;
    return pressed;
}

// Value Box control, updates input text with numbers
// NOTE: Requires static variables: frameCounter
bool GuiValueBox(Rectangle bounds, const char *text, int *value, int minValue, int maxValue, bool editMode)
{
    #if !defined(VALUEBOX_MAX_CHARS)
        #define VALUEBOX_MAX_CHARS  32
    #endif

    GuiControlState state = guiState;
    bool pressed = false;

    char textValue[VALUEBOX_MAX_CHARS + 1] = "\0";
    sprintf(textValue, "%i", *value);

    Rectangle textBounds = { 0 };
    if (text != NULL)
    {
        textBounds.width = (float)GetTextWidth(text);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(VALUEBOX, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;
        if (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_LEFT) textBounds.x = bounds.x - textBounds.width - GuiGetStyle(VALUEBOX, TEXT_PADDING);
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        bool valueHasChanged = false;

        if (editMode)
        {
            state = GUI_STATE_PRESSED;

            int keyCount = (int)strlen(textValue);

            // Only allow keys in range [48..57]
            if (keyCount < VALUEBOX_MAX_CHARS)
            {
                if (GetTextWidth(textValue) < bounds.width)
                {
                    int key = GetCharPressed();
                    if ((key >= 48) && (key <= 57))
                    {
                        textValue[keyCount] = (char)key;
                        keyCount++;
                        valueHasChanged = true;
                    }
                }
            }

            // Delete text
            if (keyCount > 0)
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    keyCount--;
                    textValue[keyCount] = '\0';
                    if (keyCount < 0) keyCount = 0;
                    valueHasChanged = true;
                }
            }

            if (valueHasChanged) *value = TextToInteger(textValue);

            if (IsKeyPressed(KEY_ENTER) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) pressed = true;
        }
        else
        {
            if (*value > maxValue) *value = maxValue;
            else if (*value < minValue) *value = minValue;

            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = GUI_STATE_FOCUSED;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    Color baseColor = BLANK;
    if (state == GUI_STATE_PRESSED) baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_PRESSED));
    else if (state == GUI_STATE_DISABLED) baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_DISABLED));

    // WARNING: BLANK color does not work properly with Fade()
    GuiDrawRectangle(bounds, GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER + (state*3))), guiAlpha), baseColor);
    GuiDrawText(textValue, GetTextBounds(VALUEBOX, bounds), GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(VALUEBOX, TEXT + (state*3))), guiAlpha));

    // Draw cursor
    if (editMode)
    {
        // NOTE: ValueBox internal text is always centered
        Rectangle cursor = { bounds.x + GetTextWidth(textValue)/2 + bounds.width/2 + 2, bounds.y + 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), 4, bounds.height - 4*GuiGetStyle(VALUEBOX, BORDER_WIDTH) };
        GuiDrawRectangle(cursor, 0, BLANK, Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER_COLOR_PRESSED)), guiAlpha));
    }

    // Draw text label if provided
    GuiDrawText(text, textBounds, (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_RIGHT)? GUI_TEXT_ALIGN_LEFT : GUI_TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return pressed;
}

// Text Box control with multiple lines
bool GuiTextBoxMulti(Rectangle bounds, char *text, int textSize, bool editMode)
{
    GuiControlState state = guiState;
    bool pressed = false;

    Rectangle textAreaBounds = {
        bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING),
        bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING),
        bounds.width - 2*(GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING)),
        bounds.height - 2*(GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING))
    };

    // Cursor position, [x, y] values should be updated
    Rectangle cursor = { 0, -1, 4, (float)GuiGetStyle(DEFAULT, TEXT_SIZE) + 2 };

    float scaleFactor = (float)GuiGetStyle(DEFAULT, TEXT_SIZE)/(float)guiFont.baseSize;     // Character rectangle scaling factor

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (editMode)
        {
            state = GUI_STATE_PRESSED;

            // We get an Unicode codepoint
            int codepoint = GetCharPressed();
            int textLength = (int)strlen(text);     // Length in bytes (UTF-8 string)

            // Introduce characters
            if (textLength < (textSize - 1))
            {
                if (IsKeyPressed(KEY_ENTER))
                {
                    text[textLength] = '\n';
                    textLength++;
                }
                else if (codepoint >= 32)
                {
                    // Supports Unicode inputs -> Encoded to UTF-8
                    int charUTF8Length = 0;
                    const char *charEncoded = CodepointToUTF8(codepoint, &charUTF8Length);
                    memcpy(text + textLength, charEncoded, charUTF8Length);
                    textLength += charUTF8Length;
                }
            }

            // Delete characters
            if (textLength > 0)
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    if ((unsigned char)text[textLength - 1] < 127)
                    {
                        // Remove ASCII equivalent character (1 byte)
                        textLength--;
                        text[textLength] = '\0';
                    }
                    else
                    {
                        // Remove latest UTF-8 unicode character introduced (n bytes)
                        int charUTF8Length = 0;
                        while (((unsigned char)text[textLength - 1 - charUTF8Length] & 0b01000000) == 0) charUTF8Length++;

                        textLength -= (charUTF8Length + 1);
                        text[textLength] = '\0';
                    }
                }
            }

            // Exit edit mode
            if (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
        }
        else
        {
            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = GUI_STATE_FOCUSED;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (state == GUI_STATE_PRESSED)
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_PRESSED)), guiAlpha));
    }
    else if (state == GUI_STATE_DISABLED)
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_DISABLED)), guiAlpha));
    }
    else GuiDrawRectangle(bounds, 1, Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), BLANK);

    int wrapMode = 1;      // 0-No wrap, 1-Char wrap, 2-Word wrap
    Vector2 cursorPos = { textAreaBounds.x, textAreaBounds.y };

    //int lastSpacePos = 0;
    //int lastSpaceWidth = 0;
    //int lastSpaceCursorPos = 0;

    for (int i = 0, codepointLength = 0; text[i] != '\0'; i += codepointLength)
    {
        int codepoint = GetCodepoint(text + i, &codepointLength);
        int index = GetGlyphIndex(guiFont, codepoint);      // If requested codepoint is not found, we get '?' (0x3f)
        Rectangle atlasRec = guiFont.recs[index];
        GlyphInfo glyphInfo = guiFont.glyphs[index];        // Glyph measures

        if ((codepointLength == 1) && (codepoint == '\n'))
        {
            cursorPos.y += (guiFont.baseSize*scaleFactor + GuiGetStyle(TEXTBOX, TEXT_LINES_PADDING));   // Line feed
            cursorPos.x = textAreaBounds.x;                 // Carriage return
        }
        else
        {
            if (wrapMode == 1)
            {
                int glyphWidth = 0;
                if (glyphInfo.advanceX != 0) glyphWidth += glyphInfo.advanceX;
                else glyphWidth += (atlasRec.width + glyphInfo.offsetX);

                // Jump line if the end of the text box area has been reached
                if ((cursorPos.x + (glyphWidth*scaleFactor)) > (textAreaBounds.x + textAreaBounds.width))
                {
                    cursorPos.y += (guiFont.baseSize*scaleFactor + GuiGetStyle(TEXTBOX, TEXT_LINES_PADDING));   // Line feed
                    cursorPos.x = textAreaBounds.x;     // Carriage return
                }
            }
            else if (wrapMode == 2)
            {
                /*
                if ((codepointLength == 1) && (codepoint == ' '))
                {
                    lastSpacePos = i;
                    lastSpaceWidth = 0;
                    lastSpaceCursorPos = cursorPos.x;
                }

                // Jump line if last word reaches end of text box area
                if ((lastSpaceCursorPos + lastSpaceWidth) > (textAreaBounds.x + textAreaBounds.width))
                {
                    cursorPos.y += 12;               // Line feed
                    cursorPos.x = textAreaBounds.x;  // Carriage return
                }
                */
            }

            // Draw current character glyph
            DrawTextCodepoint(guiFont, codepoint, cursorPos, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), Fade(GetColor(GuiGetStyle(TEXTBOX, TEXT + (state*3))), guiAlpha));

            int glyphWidth = 0;
            if (glyphInfo.advanceX != 0) glyphWidth += glyphInfo.advanceX;
            else glyphWidth += (atlasRec.width + glyphInfo.offsetX);

            cursorPos.x += (glyphWidth*scaleFactor + (float)GuiGetStyle(DEFAULT, TEXT_SPACING));
            //if (i > lastSpacePos) lastSpaceWidth += (atlasRec.width + (float)GuiGetStyle(DEFAULT, TEXT_SPACING));
        }
    }

    cursor.x = cursorPos.x;
    cursor.y = cursorPos.y;

    // Draw cursor position considering text glyphs
    if (editMode) GuiDrawRectangle(cursor, 0, BLANK, Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER_COLOR_PRESSED)), guiAlpha));
    //--------------------------------------------------------------------

    return pressed;
}

// Slider control with pro parameters
// NOTE: Other GuiSlider*() controls use this one
float GuiSliderPro(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue, int sliderWidth)
{
    GuiControlState state = guiState;

    int sliderValue = (int)(((value - minValue)/(maxValue - minValue))*(bounds.width - 2*GuiGetStyle(SLIDER, BORDER_WIDTH)));

    Rectangle slider = { bounds.x, bounds.y + GuiGetStyle(SLIDER, BORDER_WIDTH) + GuiGetStyle(SLIDER, SLIDER_PADDING),
                         0, bounds.height - 2*GuiGetStyle(SLIDER, BORDER_WIDTH) - 2*GuiGetStyle(SLIDER, SLIDER_PADDING) };

    if (sliderWidth > 0)        // Slider
    {
        slider.x += (sliderValue - sliderWidth/2);
        slider.width = (float)sliderWidth;
    }
    else if (sliderWidth == 0)  // SliderBar
    {
        slider.x += GuiGetStyle(SLIDER, BORDER_WIDTH);
        slider.width = (float)sliderValue;
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                state = GUI_STATE_PRESSED;

                // Get equivalent value and slider position from mousePoint.x
                value = ((maxValue - minValue)*(mousePoint.x - (float)(bounds.x + sliderWidth/2)))/(float)(bounds.width - sliderWidth) + minValue;

                if (sliderWidth > 0) slider.x = mousePoint.x - slider.width/2;  // Slider
                else if (sliderWidth == 0) slider.width = (float)sliderValue;          // SliderBar
            }
            else state = GUI_STATE_FOCUSED;
        }

        if (value > maxValue) value = maxValue;
        else if (value < minValue) value = minValue;
    }

    // Bar limits check
    if (sliderWidth > 0)        // Slider
    {
        if (slider.x <= (bounds.x + GuiGetStyle(SLIDER, BORDER_WIDTH))) slider.x = bounds.x + GuiGetStyle(SLIDER, BORDER_WIDTH);
        else if ((slider.x + slider.width) >= (bounds.x + bounds.width)) slider.x = bounds.x + bounds.width - slider.width - GuiGetStyle(SLIDER, BORDER_WIDTH);
    }
    else if (sliderWidth == 0)  // SliderBar
    {
        if (slider.width > bounds.width) slider.width = bounds.width - 2*GuiGetStyle(SLIDER, BORDER_WIDTH);
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(SLIDER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(SLIDER, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(SLIDER, (state != GUI_STATE_DISABLED)?  BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));

    // Draw slider internal bar (depends on state)
    if ((state == GUI_STATE_NORMAL) || (state == GUI_STATE_PRESSED)) GuiDrawRectangle(slider, 0, BLANK, Fade(GetColor(GuiGetStyle(SLIDER, BASE_COLOR_PRESSED)), guiAlpha));
    else if (state == GUI_STATE_FOCUSED) GuiDrawRectangle(slider, 0, BLANK, Fade(GetColor(GuiGetStyle(SLIDER, TEXT_COLOR_FOCUSED)), guiAlpha));

    // Draw left/right text if provided
    if (textLeft != NULL)
    {
        Rectangle textBounds = { 0 };
        textBounds.width = (float)GetTextWidth(textLeft);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x - textBounds.width - GuiGetStyle(SLIDER, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

        GuiDrawText(textLeft, textBounds, GUI_TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(SLIDER, TEXT + (state*3))), guiAlpha));
    }

    if (textRight != NULL)
    {
        Rectangle textBounds = { 0 };
        textBounds.width = (float)GetTextWidth(textRight);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(SLIDER, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

        GuiDrawText(textRight, textBounds, GUI_TEXT_ALIGN_LEFT, Fade(GetColor(GuiGetStyle(SLIDER, TEXT + (state*3))), guiAlpha));
    }
    //--------------------------------------------------------------------

    return value;
}

// Slider control extended, returns selected value and has text
float GuiSlider(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue)
{
    return GuiSliderPro(bounds, textLeft, textRight, value, minValue, maxValue, GuiGetStyle(SLIDER, SLIDER_WIDTH));
}

// Slider Bar control extended, returns selected value
float GuiSliderBar(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue)
{
    return GuiSliderPro(bounds, textLeft, textRight, value, minValue, maxValue, 0);
}

// Progress Bar control extended, shows current progress value
float GuiProgressBar(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue)
{
    GuiControlState state = guiState;

    Rectangle progress = { bounds.x + GuiGetStyle(PROGRESSBAR, BORDER_WIDTH),
                           bounds.y + GuiGetStyle(PROGRESSBAR, BORDER_WIDTH) + GuiGetStyle(PROGRESSBAR, PROGRESS_PADDING), 0,
                           bounds.height - 2*GuiGetStyle(PROGRESSBAR, BORDER_WIDTH) - 2*GuiGetStyle(PROGRESSBAR, PROGRESS_PADDING) };

    // Update control
    //--------------------------------------------------------------------
    if (state != GUI_STATE_DISABLED) progress.width = ((float)(value/(maxValue - minValue))*(float)(bounds.width - 2*GuiGetStyle(PROGRESSBAR, BORDER_WIDTH)));
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(PROGRESSBAR, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(PROGRESSBAR, BORDER + (state*3))), guiAlpha), BLANK);

    // Draw slider internal progress bar (depends on state)
    if ((state == GUI_STATE_NORMAL) || (state == GUI_STATE_PRESSED)) GuiDrawRectangle(progress, 0, BLANK, Fade(GetColor(GuiGetStyle(PROGRESSBAR, BASE_COLOR_PRESSED)), guiAlpha));
    else if (state == GUI_STATE_FOCUSED) GuiDrawRectangle(progress, 0, BLANK, Fade(GetColor(GuiGetStyle(PROGRESSBAR, TEXT_COLOR_FOCUSED)), guiAlpha));

    // Draw left/right text if provided
    if (textLeft != NULL)
    {
        Rectangle textBounds = { 0 };
        textBounds.width = (float)GetTextWidth(textLeft);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x - textBounds.width - GuiGetStyle(PROGRESSBAR, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

        GuiDrawText(textLeft, textBounds, GUI_TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(PROGRESSBAR, TEXT + (state*3))), guiAlpha));
    }

    if (textRight != NULL)
    {
        Rectangle textBounds = { 0 };
        textBounds.width = (float)GetTextWidth(textRight);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(PROGRESSBAR, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

        GuiDrawText(textRight, textBounds, GUI_TEXT_ALIGN_LEFT, Fade(GetColor(GuiGetStyle(PROGRESSBAR, TEXT + (state*3))), guiAlpha));
    }
    //--------------------------------------------------------------------

    return value;
}

// Status Bar control
void GuiStatusBar(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(STATUSBAR, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(STATUSBAR, (state != GUI_STATE_DISABLED)? BORDER_COLOR_NORMAL : BORDER_COLOR_DISABLED)), guiAlpha),
                     Fade(GetColor(GuiGetStyle(STATUSBAR, (state != GUI_STATE_DISABLED)? BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));
    GuiDrawText(text, GetTextBounds(STATUSBAR, bounds), GuiGetStyle(STATUSBAR, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(STATUSBAR, (state != GUI_STATE_DISABLED)? TEXT_COLOR_NORMAL : TEXT_COLOR_DISABLED)), guiAlpha));
    //--------------------------------------------------------------------
}

// Dummy rectangle control, intended for placeholding
void GuiDummyRec(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, 0, BLANK, Fade(GetColor(GuiGetStyle(DEFAULT, (state != GUI_STATE_DISABLED)? BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));
    GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(BUTTON, (state != GUI_STATE_DISABLED)? TEXT_COLOR_NORMAL : TEXT_COLOR_DISABLED)), guiAlpha));
    //------------------------------------------------------------------
}

// Scroll Bar control
int GuiScrollBar(Rectangle bounds, int value, int minValue, int maxValue)
{
    GuiControlState state = guiState;

    // Is the scrollbar horizontal or vertical?
    bool isVertical = (bounds.width > bounds.height)? false : true;

    // The size (width or height depending on scrollbar type) of the spinner buttons
    const int spinnerSize = GuiGetStyle(SCROLLBAR, ARROWS_VISIBLE)? (isVertical? (int)bounds.width - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH) : (int)bounds.height - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH)) : 0;

    // Arrow buttons [<] [>] [∧] [∨]
    Rectangle arrowUpLeft = { 0 };
    Rectangle arrowDownRight = { 0 };

    // Actual area of the scrollbar excluding the arrow buttons
    Rectangle scrollbar = { 0 };

    // Slider bar that moves     --[///]-----
    Rectangle slider = { 0 };

    // Normalize value
    if (value > maxValue) value = maxValue;
    if (value < minValue) value = minValue;

    const int range = maxValue - minValue;
    int sliderSize = GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE);

    // Calculate rectangles for all of the components
    arrowUpLeft = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)spinnerSize, (float)spinnerSize };

    if (isVertical)
    {
        arrowDownRight = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)bounds.y + bounds.height - spinnerSize - GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)spinnerSize, (float)spinnerSize};
        scrollbar = RAYGUI_CLITERAL(Rectangle){ bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_PADDING), arrowUpLeft.y + arrowUpLeft.height, bounds.width - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_PADDING)), bounds.height - arrowUpLeft.height - arrowDownRight.height - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH) };
        sliderSize = (sliderSize >= scrollbar.height)? ((int)scrollbar.height - 2) : sliderSize;     // Make sure the slider won't get outside of the scrollbar
        slider = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING), (float)scrollbar.y + (int)(((float)(value - minValue)/range)*(scrollbar.height - sliderSize)), (float)bounds.width - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING)), (float)sliderSize };
    }
    else
    {
        arrowDownRight = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + bounds.width - spinnerSize - GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)spinnerSize, (float)spinnerSize};
        scrollbar = RAYGUI_CLITERAL(Rectangle){ arrowUpLeft.x + arrowUpLeft.width, bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_PADDING), bounds.width - arrowUpLeft.width - arrowDownRight.width - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH), bounds.height - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_PADDING))};
        sliderSize = (sliderSize >= scrollbar.width)? ((int)scrollbar.width - 2) : sliderSize;       // Make sure the slider won't get outside of the scrollbar
        slider = RAYGUI_CLITERAL(Rectangle){ (float)scrollbar.x + (int)(((float)(value - minValue)/range)*(scrollbar.width - sliderSize)), (float)bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING), (float)sliderSize, (float)bounds.height - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING)) };
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            state = GUI_STATE_FOCUSED;

            // Handle mouse wheel
            int wheel = (int)GetMouseWheelMove();
            if (wheel != 0) value += wheel;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (CheckCollisionPointRec(mousePoint, arrowUpLeft)) value -= range/GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
                else if (CheckCollisionPointRec(mousePoint, arrowDownRight)) value += range/GuiGetStyle(SCROLLBAR, SCROLL_SPEED);

                state = GUI_STATE_PRESSED;
            }
            else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                if (!isVertical)
                {
                    Rectangle scrollArea = { arrowUpLeft.x + arrowUpLeft.width, arrowUpLeft.y, scrollbar.width, bounds.height - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH)};
                    if (CheckCollisionPointRec(mousePoint, scrollArea)) value = (int)(((float)(mousePoint.x - scrollArea.x - slider.width/2)*range)/(scrollArea.width - slider.width) + minValue);
                }
                else
                {
                    Rectangle scrollArea = { arrowUpLeft.x, arrowUpLeft.y+arrowUpLeft.height, bounds.width - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH),  scrollbar.height};
                    if (CheckCollisionPointRec(mousePoint, scrollArea)) value = (int)(((float)(mousePoint.y - scrollArea.y - slider.height/2)*range)/(scrollArea.height - slider.height) + minValue);
                }
            }
        }

        // Normalize value
        if (value > maxValue) value = maxValue;
        if (value < minValue) value = minValue;
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(SCROLLBAR, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + state*3)), guiAlpha), Fade(GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_DISABLED)), guiAlpha));   // Draw the background

    GuiDrawRectangle(scrollbar, 0, BLANK, Fade(GetColor(GuiGetStyle(BUTTON, BASE_COLOR_NORMAL)), guiAlpha));     // Draw the scrollbar active area background
    GuiDrawRectangle(slider, 0, BLANK, Fade(GetColor(GuiGetStyle(SLIDER, BORDER + state*3)), guiAlpha));         // Draw the slider bar

    // Draw arrows (using icon if available)
    if (GuiGetStyle(SCROLLBAR, ARROWS_VISIBLE))
    {
#if defined(RAYGUI_NO_RICONS)
        GuiDrawText(isVertical? "^" : "<", RAYGUI_CLITERAL(Rectangle){ arrowUpLeft.x, arrowUpLeft.y, isVertical? bounds.width : bounds.height, isVertical? bounds.width : bounds.height },
                    GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))), guiAlpha));
        GuiDrawText(isVertical? "v" : ">", RAYGUI_CLITERAL(Rectangle){ arrowDownRight.x, arrowDownRight.y, isVertical? bounds.width : bounds.height, isVertical? bounds.width : bounds.height },
                    GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))), guiAlpha));
#else
        GuiDrawText(isVertical? "#121#" : "#118#", RAYGUI_CLITERAL(Rectangle){ arrowUpLeft.x, arrowUpLeft.y, isVertical? bounds.width : bounds.height, isVertical? bounds.width : bounds.height },
                    GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(SCROLLBAR, TEXT + state*3)), guiAlpha));   // RICON_ARROW_UP_FILL / RICON_ARROW_LEFT_FILL
        GuiDrawText(isVertical? "#120#" : "#119#", RAYGUI_CLITERAL(Rectangle){ arrowDownRight.x, arrowDownRight.y, isVertical? bounds.width : bounds.height, isVertical? bounds.width : bounds.height },
                    GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(SCROLLBAR, TEXT + state*3)), guiAlpha));   // RICON_ARROW_DOWN_FILL / RICON_ARROW_RIGHT_FILL
#endif
    }
    //--------------------------------------------------------------------

    return value;
}

// List View control
int GuiListView(Rectangle bounds, const char *text, int *scrollIndex, int active)
{
    int itemCount = 0;
    const char **items = NULL;

    if (text != NULL) items = GuiTextSplit(text, &itemCount, NULL);

    return GuiListViewEx(bounds, items, itemCount, NULL, scrollIndex, active);
}

// List View control with extended parameters
int GuiListViewEx(Rectangle bounds, const char **text, int count, int *focus, int *scrollIndex, int active)
{
    GuiControlState state = guiState;
    int itemFocused = (focus == NULL)? -1 : *focus;
    int itemSelected = active;

    // Check if we need a scroll bar
    bool useScrollBar = false;
    if ((GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING))*count > bounds.height) useScrollBar = true;

    // Define base item rectangle [0]
    Rectangle itemBounds = { 0 };
    itemBounds.x = bounds.x + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING);
    itemBounds.y = bounds.y + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH);
    itemBounds.width = bounds.width - 2*GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING) - GuiGetStyle(DEFAULT, BORDER_WIDTH);
    itemBounds.height = (float)GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
    if (useScrollBar) itemBounds.width -= GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH);

    // Get items on the list
    int visibleItems = (int)bounds.height/(GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING));
    if (visibleItems > count) visibleItems = count;

    int startIndex = (scrollIndex == NULL)? 0 : *scrollIndex;
    if ((startIndex < 0) || (startIndex > (count - visibleItems))) startIndex = 0;
    int endIndex = startIndex + visibleItems;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check mouse inside list view
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            state = GUI_STATE_FOCUSED;

            // Check focused and selected item
            for (int i = 0; i < visibleItems; i++)
            {
                if (CheckCollisionPointRec(mousePoint, itemBounds))
                {
                    itemFocused = startIndex + i;
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                    {
                        if (itemSelected == (startIndex + i)) itemSelected = -1;
                        else itemSelected = startIndex + i;
                    }
                    break;
                }

                // Update item rectangle y position for next item
                itemBounds.y += (GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING));
            }

            if (useScrollBar)
            {
                int wheelMove = (int)GetMouseWheelMove();
                startIndex -= wheelMove;

                if (startIndex < 0) startIndex = 0;
                else if (startIndex > (count - visibleItems)) startIndex = count - visibleItems;

                endIndex = startIndex + visibleItems;
                if (endIndex > count) endIndex = count;
            }
        }
        else itemFocused = -1;

        // Reset item rectangle y to [0]
        itemBounds.y = bounds.y + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH);
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + state*3)), guiAlpha), GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));     // Draw background

    // Draw visible items
    for (int i = 0; ((i < visibleItems) && (text != NULL)); i++)
    {
        if (state == GUI_STATE_DISABLED)
        {
            if ((startIndex + i) == itemSelected) GuiDrawRectangle(itemBounds, GuiGetStyle(LISTVIEW, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_DISABLED)), guiAlpha), Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_DISABLED)), guiAlpha));

            GuiDrawText(text[startIndex + i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(LISTVIEW, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_DISABLED)), guiAlpha));
        }
        else
        {
            if ((startIndex + i) == itemSelected)
            {
                // Draw item selected
                GuiDrawRectangle(itemBounds, GuiGetStyle(LISTVIEW, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_PRESSED)), guiAlpha), Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_PRESSED)), guiAlpha));
                GuiDrawText(text[startIndex + i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(LISTVIEW, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_PRESSED)), guiAlpha));
            }
            else if ((startIndex + i) == itemFocused)
            {
                // Draw item focused
                GuiDrawRectangle(itemBounds, GuiGetStyle(LISTVIEW, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_FOCUSED)), guiAlpha), Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_FOCUSED)), guiAlpha));
                GuiDrawText(text[startIndex + i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(LISTVIEW, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_FOCUSED)), guiAlpha));
            }
            else
            {
                // Draw item normal
                GuiDrawText(text[startIndex + i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(LISTVIEW, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_NORMAL)), guiAlpha));
            }
        }

        // Update item rectangle y position for next item
        itemBounds.y += (GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING));
    }

    if (useScrollBar)
    {
        Rectangle scrollBarBounds = {
            bounds.x + bounds.width - GuiGetStyle(LISTVIEW, BORDER_WIDTH) - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH),
            bounds.y + GuiGetStyle(LISTVIEW, BORDER_WIDTH), (float)GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH),
            bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH)
        };

        // Calculate percentage of visible items and apply same percentage to scrollbar
        float percentVisible = (float)(endIndex - startIndex)/count;
        float sliderSize = bounds.height*percentVisible;

        int prevSliderSize = GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE);   // Save default slider size
        int prevScrollSpeed = GuiGetStyle(SCROLLBAR, SCROLL_SPEED); // Save default scroll speed
        GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, (int)sliderSize);            // Change slider size
        GuiSetStyle(SCROLLBAR, SCROLL_SPEED, count - visibleItems); // Change scroll speed

        startIndex = GuiScrollBar(scrollBarBounds, startIndex, 0, count - visibleItems);

        GuiSetStyle(SCROLLBAR, SCROLL_SPEED, prevScrollSpeed); // Reset scroll speed to default
        GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, prevSliderSize); // Reset slider size to default
    }
    //--------------------------------------------------------------------

    if (focus != NULL) *focus = itemFocused;
    if (scrollIndex != NULL) *scrollIndex = startIndex;

    return itemSelected;
}

// Color Panel control
Color GuiColorPanel(Rectangle bounds, Color color)
{
    const Color colWhite = { 255, 255, 255, 255 };
    const Color colBlack = { 0, 0, 0, 255 };

    GuiControlState state = guiState;
    Vector2 pickerSelector = { 0 };

    Vector3 vcolor = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
    Vector3 hsv = ConvertRGBtoHSV(vcolor);

    pickerSelector.x = bounds.x + (float)hsv.y*bounds.width;            // HSV: Saturation
    pickerSelector.y = bounds.y + (1.0f - (float)hsv.z)*bounds.height;  // HSV: Value

    float hue = -1.0f;
    Vector3 maxHue = { hue >= 0.0f ? hue : hsv.x, 1.0f, 1.0f };
    Vector3 rgbHue = ConvertHSVtoRGB(maxHue);
    Color maxHueCol = { (unsigned char)(255.0f*rgbHue.x),
                      (unsigned char)(255.0f*rgbHue.y),
                      (unsigned char)(255.0f*rgbHue.z), 255 };

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                state = GUI_STATE_PRESSED;
                pickerSelector = mousePoint;

                // Calculate color from picker
                Vector2 colorPick = { pickerSelector.x - bounds.x, pickerSelector.y - bounds.y };

                colorPick.x /= (float)bounds.width;     // Get normalized value on x
                colorPick.y /= (float)bounds.height;    // Get normalized value on y

                hsv.y = colorPick.x;
                hsv.z = 1.0f - colorPick.y;

                Vector3 rgb = ConvertHSVtoRGB(hsv);

                // NOTE: Vector3ToColor() only available on raylib 1.8.1
                color = RAYGUI_CLITERAL(Color){ (unsigned char)(255.0f*rgb.x),
                                 (unsigned char)(255.0f*rgb.y),
                                 (unsigned char)(255.0f*rgb.z),
                                 (unsigned char)(255.0f*(float)color.a/255.0f) };

            }
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (state != GUI_STATE_DISABLED)
    {
        DrawRectangleGradientEx(bounds, Fade(colWhite, guiAlpha), Fade(colWhite, guiAlpha), Fade(maxHueCol, guiAlpha), Fade(maxHueCol, guiAlpha));
        DrawRectangleGradientEx(bounds, Fade(colBlack, 0), Fade(colBlack, guiAlpha), Fade(colBlack, guiAlpha), Fade(colBlack, 0));

        // Draw color picker: selector
        Rectangle selector = { pickerSelector.x - GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE)/2, pickerSelector.y - GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE)/2, (float)GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE), (float)GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE) };
        GuiDrawRectangle(selector, 0, BLANK, Fade(colWhite, guiAlpha));
    }
    else
    {
        DrawRectangleGradientEx(bounds, Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), guiAlpha), Fade(Fade(colBlack, 0.6f), guiAlpha), Fade(Fade(colBlack, 0.6f), guiAlpha), Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), 0.6f), guiAlpha));
    }

    GuiDrawRectangle(bounds, GuiGetStyle(COLORPICKER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha), BLANK);
    //--------------------------------------------------------------------

    return color;
}

// Color Bar Alpha control
// NOTE: Returns alpha value normalized [0..1]
float GuiColorBarAlpha(Rectangle bounds, float alpha)
{
    #define COLORBARALPHA_CHECKED_SIZE   10

    GuiControlState state = guiState;
    Rectangle selector = { (float)bounds.x + alpha*bounds.width - GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_HEIGHT)/2, (float)bounds.y - GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_OVERFLOW), (float)GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_HEIGHT), (float)bounds.height + GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_OVERFLOW)*2 };

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds) ||
            CheckCollisionPointRec(mousePoint, selector))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                state = GUI_STATE_PRESSED;

                alpha = (mousePoint.x - bounds.x)/bounds.width;
                if (alpha <= 0.0f) alpha = 0.0f;
                if (alpha >= 1.0f) alpha = 1.0f;
                //selector.x = bounds.x + (int)(((alpha - 0)/(100 - 0))*(bounds.width - 2*GuiGetStyle(SLIDER, BORDER_WIDTH))) - selector.width/2;
            }
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------

    // Draw alpha bar: checked background
    if (state != GUI_STATE_DISABLED)
    {
        int checksX = (int)bounds.width/COLORBARALPHA_CHECKED_SIZE;
        int checksY = (int)bounds.height/COLORBARALPHA_CHECKED_SIZE;

        for (int x = 0; x < checksX; x++)
        {
            for (int y = 0; y < checksY; y++)
            {
                Rectangle check = { bounds.x + x*COLORBARALPHA_CHECKED_SIZE, bounds.y + y*COLORBARALPHA_CHECKED_SIZE, COLORBARALPHA_CHECKED_SIZE, COLORBARALPHA_CHECKED_SIZE };
                GuiDrawRectangle(check, 0, BLANK, ((x + y)%2)? Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), 0.4f), guiAlpha) : Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.4f), guiAlpha));
            }
        }

        DrawRectangleGradientEx(bounds, RAYGUI_CLITERAL(Color){ 255, 255, 255, 0 }, RAYGUI_CLITERAL(Color){ 255, 255, 255, 0 }, Fade(RAYGUI_CLITERAL(Color){ 0, 0, 0, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color){ 0, 0, 0, 255 }, guiAlpha));
    }
    else DrawRectangleGradientEx(bounds, Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha));

    GuiDrawRectangle(bounds, GuiGetStyle(COLORPICKER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha), BLANK);

    // Draw alpha bar: selector
    GuiDrawRectangle(selector, 0, BLANK, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));
    //--------------------------------------------------------------------

    return alpha;
}

// Color Bar Hue control
// Returns hue value normalized [0..1]
// NOTE: Other similar bars (for reference):
//      Color GuiColorBarSat() [WHITE->color]
//      Color GuiColorBarValue() [BLACK->color], HSV/HSL
//      float GuiColorBarLuminance() [BLACK->WHITE]
float GuiColorBarHue(Rectangle bounds, float hue)
{
    GuiControlState state = guiState;
    Rectangle selector = { (float)bounds.x - GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_OVERFLOW), (float)bounds.y + hue/360.0f*bounds.height - GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_HEIGHT)/2, (float)bounds.width + GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_OVERFLOW)*2, (float)GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_HEIGHT) };

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds) ||
            CheckCollisionPointRec(mousePoint, selector))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                state = GUI_STATE_PRESSED;

                hue = (mousePoint.y - bounds.y)*360/bounds.height;
                if (hue <= 0.0f) hue = 0.0f;
                if (hue >= 359.0f) hue = 359.0f;

            }
            else state = GUI_STATE_FOCUSED;

            /*if (IsKeyDown(KEY_UP))
            {
                hue -= 2.0f;
                if (hue <= 0.0f) hue = 0.0f;
            }
            else if (IsKeyDown(KEY_DOWN))
            {
                hue += 2.0f;
                if (hue >= 360.0f) hue = 360.0f;
            }*/
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (state != GUI_STATE_DISABLED)
    {
        // Draw hue bar:color bars
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y), (int)bounds.width, ceil(bounds.height/6),  Fade(RAYGUI_CLITERAL(Color) { 255, 0, 0, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 255, 255, 0, 255 }, guiAlpha));
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y + bounds.height/6), (int)bounds.width, ceil(bounds.height/6), Fade(RAYGUI_CLITERAL(Color) { 255, 255, 0, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 0, 255, 0, 255 }, guiAlpha));
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y + 2*(bounds.height/6)), (int)bounds.width, ceil(bounds.height/6), Fade(RAYGUI_CLITERAL(Color) { 0, 255, 0, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 0, 255, 255, 255 }, guiAlpha));
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y + 3*(bounds.height/6)), (int)bounds.width, ceil(bounds.height/6), Fade(RAYGUI_CLITERAL(Color) { 0, 255, 255, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 0, 0, 255, 255 }, guiAlpha));
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y + 4*(bounds.height/6)), (int)bounds.width, ceil(bounds.height/6), Fade(RAYGUI_CLITERAL(Color) { 0, 0, 255, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 255, 0, 255, 255 }, guiAlpha));
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y + 5*(bounds.height/6)), (int)bounds.width, (int)(bounds.height/6), Fade(RAYGUI_CLITERAL(Color) { 255, 0, 255, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 255, 0, 0, 255 }, guiAlpha));
    }
    else DrawRectangleGradientV((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), guiAlpha), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha));

    GuiDrawRectangle(bounds, GuiGetStyle(COLORPICKER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha), BLANK);

    // Draw hue bar: selector
    GuiDrawRectangle(selector, 0, BLANK, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));
    //--------------------------------------------------------------------

    return hue;
}

// Color Picker control
// NOTE: It's divided in multiple controls:
//      Color GuiColorPanel(Rectangle bounds, Color color)
//      float GuiColorBarAlpha(Rectangle bounds, float alpha)
//      float GuiColorBarHue(Rectangle bounds, float value)
// NOTE: bounds define GuiColorPanel() size
Color GuiColorPicker(Rectangle bounds, Color color)
{
    color = GuiColorPanel(bounds, color);

    Rectangle boundsHue = { (float)bounds.x + bounds.width + GuiGetStyle(COLORPICKER, HUEBAR_PADDING), (float)bounds.y, (float)GuiGetStyle(COLORPICKER, HUEBAR_WIDTH), (float)bounds.height };
    //Rectangle boundsAlpha = { bounds.x, bounds.y + bounds.height + GuiGetStyle(COLORPICKER, BARS_PADDING), bounds.width, GuiGetStyle(COLORPICKER, BARS_THICK) };

    Vector3 hsv = ConvertRGBtoHSV(RAYGUI_CLITERAL(Vector3){ color.r/255.0f, color.g/255.0f, color.b/255.0f });
    hsv.x = GuiColorBarHue(boundsHue, hsv.x);
    //color.a = (unsigned char)(GuiColorBarAlpha(boundsAlpha, (float)color.a/255.0f)*255.0f);
    Vector3 rgb = ConvertHSVtoRGB(hsv);

    color = RAYGUI_CLITERAL(Color){ (unsigned char)roundf(rgb.x*255.0f), (unsigned char)roundf(rgb.y*255.0f), (unsigned char)roundf(rgb.z*255.0f), color.a };

    return color;
}

// Message Box control
int GuiMessageBox(Rectangle bounds, const char *title, const char *message, const char *buttons)
{
    #define MESSAGEBOX_BUTTON_HEIGHT    24
    #define MESSAGEBOX_BUTTON_PADDING   10

    int clicked = -1;    // Returns clicked button from buttons list, 0 refers to closed window button

    int buttonCount = 0;
    const char **buttonsText = GuiTextSplit(buttons, &buttonCount, NULL);
    Rectangle buttonBounds = { 0 };
    buttonBounds.x = bounds.x + MESSAGEBOX_BUTTON_PADDING;
    buttonBounds.y = bounds.y + bounds.height - MESSAGEBOX_BUTTON_HEIGHT - MESSAGEBOX_BUTTON_PADDING;
    buttonBounds.width = (bounds.width - MESSAGEBOX_BUTTON_PADDING*(buttonCount + 1))/buttonCount;
    buttonBounds.height = MESSAGEBOX_BUTTON_HEIGHT;

    Vector2 textSize = MeasureTextEx(guiFont, message, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), 1);

    Rectangle textBounds = { 0 };
    textBounds.x = bounds.x + bounds.width/2 - textSize.x/2;
    textBounds.y = bounds.y + WINDOW_STATUSBAR_HEIGHT + (bounds.height - WINDOW_STATUSBAR_HEIGHT - MESSAGEBOX_BUTTON_HEIGHT - MESSAGEBOX_BUTTON_PADDING)/2 - textSize.y/2;
    textBounds.width = textSize.x;
    textBounds.height = textSize.y;

    // Draw control
    //--------------------------------------------------------------------
    if (GuiWindowBox(bounds, title)) clicked = 0;

    int prevTextAlignment = GuiGetStyle(LABEL, TEXT_ALIGNMENT);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
    GuiLabel(textBounds, message);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, prevTextAlignment);

    prevTextAlignment = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

    for (int i = 0; i < buttonCount; i++)
    {
        if (GuiButton(buttonBounds, buttonsText[i])) clicked = i + 1;
        buttonBounds.x += (buttonBounds.width + MESSAGEBOX_BUTTON_PADDING);
    }

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, prevTextAlignment);
    //--------------------------------------------------------------------

    return clicked;
}

// Text Input Box control, ask for text
int GuiTextInputBox(Rectangle bounds, const char *title, const char *message, const char *buttons, char *text)
{
    #define TEXTINPUTBOX_BUTTON_HEIGHT      24
    #define TEXTINPUTBOX_BUTTON_PADDING     10
    #define TEXTINPUTBOX_HEIGHT             30

    #define TEXTINPUTBOX_MAX_TEXT_LENGTH   256

    // Used to enable text edit mode
    // WARNING: No more than one GuiTextInputBox() should be open at the same time
    static bool textEditMode = false;

    int btnIndex = -1;

    int buttonCount = 0;
    const char **buttonsText = GuiTextSplit(buttons, &buttonCount, NULL);
    Rectangle buttonBounds = { 0 };
    buttonBounds.x = bounds.x + TEXTINPUTBOX_BUTTON_PADDING;
    buttonBounds.y = bounds.y + bounds.height - TEXTINPUTBOX_BUTTON_HEIGHT - TEXTINPUTBOX_BUTTON_PADDING;
    buttonBounds.width = (bounds.width - TEXTINPUTBOX_BUTTON_PADDING*(buttonCount + 1))/buttonCount;
    buttonBounds.height = TEXTINPUTBOX_BUTTON_HEIGHT;

    int messageInputHeight = (int)bounds.height - WINDOW_STATUSBAR_HEIGHT - GuiGetStyle(STATUSBAR, BORDER_WIDTH) - TEXTINPUTBOX_BUTTON_HEIGHT - 2*TEXTINPUTBOX_BUTTON_PADDING;

    Rectangle textBounds = { 0 };
    if (message != NULL)
    {
        Vector2 textSize = MeasureTextEx(guiFont, message, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), 1);

        textBounds.x = bounds.x + bounds.width/2 - textSize.x/2;
        textBounds.y = bounds.y + WINDOW_STATUSBAR_HEIGHT + messageInputHeight/4 - textSize.y/2;
        textBounds.width = textSize.x;
        textBounds.height = textSize.y;
    }

    Rectangle textBoxBounds = { 0 };
    textBoxBounds.x = bounds.x + TEXTINPUTBOX_BUTTON_PADDING;
    textBoxBounds.y = bounds.y + WINDOW_STATUSBAR_HEIGHT - TEXTINPUTBOX_HEIGHT/2;
    if (message == NULL) textBoxBounds.y += messageInputHeight/2;
    else textBoxBounds.y += (messageInputHeight/2 + messageInputHeight/4);
    textBoxBounds.width = bounds.width - TEXTINPUTBOX_BUTTON_PADDING*2;
    textBoxBounds.height = TEXTINPUTBOX_HEIGHT;

    // Draw control
    //--------------------------------------------------------------------
    if (GuiWindowBox(bounds, title)) btnIndex = 0;

    // Draw message if available
    if (message != NULL)
    {
        int prevTextAlignment = GuiGetStyle(LABEL, TEXT_ALIGNMENT);
        GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
        GuiLabel(textBounds, message);
        GuiSetStyle(LABEL, TEXT_ALIGNMENT, prevTextAlignment);
    }

    if (GuiTextBox(textBoxBounds, text, TEXTINPUTBOX_MAX_TEXT_LENGTH, textEditMode)) textEditMode = !textEditMode;

    int prevBtnTextAlignment = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

    for (int i = 0; i < buttonCount; i++)
    {
        if (GuiButton(buttonBounds, buttonsText[i])) btnIndex = i + 1;
        buttonBounds.x += (buttonBounds.width + MESSAGEBOX_BUTTON_PADDING);
    }

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, prevBtnTextAlignment);
    //--------------------------------------------------------------------

    return btnIndex;
}

// Grid control
// NOTE: Returns grid mouse-hover selected cell
// About drawing lines at subpixel spacing, simple put, not easy solution:
// https://stackoverflow.com/questions/4435450/2d-opengl-drawing-lines-that-dont-exactly-fit-pixel-raster
Vector2 GuiGrid(Rectangle bounds, float spacing, int subdivs)
{
    #if !defined(GRID_COLOR_ALPHA)
        #define GRID_COLOR_ALPHA    0.15f           // Grid lines alpha amount
    #endif

    GuiControlState state = guiState;
    Vector2 mousePoint = GetMousePosition();
    Vector2 currentCell = { -1, -1 };

    int linesV = ((int)(bounds.width/spacing))*subdivs + 1;
    int linesH = ((int)(bounds.height/spacing))*subdivs + 1;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            currentCell.x = (mousePoint.x - bounds.x)/spacing;
            currentCell.y = (mousePoint.y - bounds.y)/spacing;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    switch (state)
    {
        case GUI_STATE_NORMAL:
        {
            if (subdivs > 0)
            {
                // Draw vertical grid lines
                for (int i = 0; i < linesV; i++)
                {
                    Rectangle lineV = { bounds.x + spacing*i/subdivs, bounds.y, 1, bounds.height };
                    GuiDrawRectangle(lineV, 0, BLANK, ((i%subdivs) == 0) ? Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA*4) : Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA));
                }

                // Draw horizontal grid lines
                for (int i = 0; i < linesH; i++)
                {
                    Rectangle lineH = { bounds.x, bounds.y + spacing*i/subdivs, bounds.width, 1 };
                    GuiDrawRectangle(lineH, 0, BLANK, ((i%subdivs) == 0) ? Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA*4) : Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA));
                }
            }
        } break;
        default: break;
    }

    return currentCell;
}

//----------------------------------------------------------------------------------
// Styles loading functions
//----------------------------------------------------------------------------------

// Load raygui style file (.rgs)
void GuiLoadStyle(const char *fileName)
{
    bool tryBinary = false;

    // Try reading the files as text file first
    FILE *rgsFile = fopen(fileName, "rt");

    if (rgsFile != NULL)
    {
        char buffer[256] = { 0 };
        fgets(buffer, 256, rgsFile);

        if (buffer[0] == '#')
        {
            int controlId = 0;
            int propertyId = 0;
            unsigned int propertyValue = 0;

            while (!feof(rgsFile))
            {
                switch (buffer[0])
                {
                    case 'p':
                    {
                        // Style property: p <control_id> <property_id> <property_value> <property_name>

                        sscanf(buffer, "p %d %d 0x%x", &controlId, &propertyId, &propertyValue);

                        GuiSetStyle(controlId, propertyId, (int)propertyValue);

                    } break;
                    case 'f':
                    {
                        // Style font: f <gen_font_size> <charmap_file> <font_file>

                        int fontSize = 0;
                        char charmapFileName[256] = { 0 };
                        char fontFileName[256] = { 0 };
                        sscanf(buffer, "f %d %s %[^\r\n]s", &fontSize, charmapFileName, fontFileName);

                        Font font = { 0 };

                        if (charmapFileName[0] != '0')
                        {
                            // Load characters from charmap file,
                            // expected '\n' separated list of integer values
                            char *charValues = LoadFileText(charmapFileName);
                            if (charValues != NULL)
                            {
                                int glyphCount = 0;
                                const char **chars = TextSplit(charValues, '\n', &glyphCount);

                                int *values = (int *)RAYGUI_MALLOC(glyphCount*sizeof(int));
                                for (int i = 0; i < glyphCount; i++) values[i] = TextToInteger(chars[i]);

                                font = LoadFontEx(TextFormat("%s/%s", GetDirectoryPath(fileName), fontFileName), fontSize, values, glyphCount);

                                RAYGUI_FREE(values);
                            }
                        }
                        else font = LoadFontEx(TextFormat("%s/%s", GetDirectoryPath(fileName), fontFileName), fontSize, NULL, 0);

                        if ((font.texture.id > 0) && (font.glyphCount > 0)) GuiSetFont(font);

                    } break;
                    default: break;
                }

                fgets(buffer, 256, rgsFile);
            }
        }
        else tryBinary = true;

        fclose(rgsFile);
    }

    if (tryBinary)
    {
        rgsFile = fopen(fileName, "rb");

        if (rgsFile == NULL) return;

        char signature[5] = "";
        short version = 0;
        short reserved = 0;
        int propertyCount = 0;

        fread(signature, 1, 4, rgsFile);
        fread(&version, 1, sizeof(short), rgsFile);
        fread(&reserved, 1, sizeof(short), rgsFile);
        fread(&propertyCount, 1, sizeof(int), rgsFile);

        if ((signature[0] == 'r') &&
            (signature[1] == 'G') &&
            (signature[2] == 'S') &&
            (signature[3] == ' '))
        {
            short controlId = 0;
            short propertyId = 0;
            int propertyValue = 0;

            for (int i = 0; i < propertyCount; i++)
            {
                fread(&controlId, 1, sizeof(short), rgsFile);
                fread(&propertyId, 1, sizeof(short), rgsFile);
                fread(&propertyValue, 1, sizeof(int), rgsFile);

                if (controlId == 0) // DEFAULT control
                {
                    // If a DEFAULT property is loaded, it is propagated to all controls
                    // NOTE: All DEFAULT properties should be defined first in the file
                    GuiSetStyle(0, (int)propertyId, propertyValue);

                    if (propertyId < RAYGUI_MAX_PROPS_BASE) for (int i = 1; i < RAYGUI_MAX_CONTROLS; i++) GuiSetStyle(i, (int)propertyId, propertyValue);
                }
                else GuiSetStyle((int)controlId, (int)propertyId, propertyValue);
            }

            // Font loading is highly dependant on raylib API to load font data and image
#if !defined(RAYGUI_STANDALONE)
            // Load custom font if available
            int fontDataSize = 0;
            fread(&fontDataSize, 1, sizeof(int), rgsFile);

            if (fontDataSize > 0)
            {
                Font font = { 0 };
                int fontType = 0;   // 0-Normal, 1-SDF
                Rectangle whiteRec = { 0 };

                fread(&font.baseSize, 1, sizeof(int), rgsFile);
                fread(&font.glyphCount, 1, sizeof(int), rgsFile);
                fread(&fontType, 1, sizeof(int), rgsFile);

                // Load font white rectangle
                fread(&whiteRec, 1, sizeof(Rectangle), rgsFile);

                // Load font image parameters
                int fontImageSize = 0;
                fread(&fontImageSize, 1, sizeof(int), rgsFile);

                if (fontImageSize > 0)
                {
                    Image imFont = { 0 };
                    imFont.mipmaps = 1;
                    fread(&imFont.width, 1, sizeof(int), rgsFile);
                    fread(&imFont.height, 1, sizeof(int), rgsFile);
                    fread(&imFont.format, 1, sizeof(int), rgsFile);

                    imFont.data = (unsigned char *)RAYGUI_MALLOC(fontImageSize);
                    fread(imFont.data, 1, fontImageSize, rgsFile);

                    font.texture = LoadTextureFromImage(imFont);

                    RAYGUI_FREE(imFont.data);
                }

                // Load font recs data
                font.recs = (Rectangle *)RAYGUI_CALLOC(font.glyphCount, sizeof(Rectangle));
                for (int i = 0; i < font.glyphCount; i++) fread(&font.recs[i], 1, sizeof(Rectangle), rgsFile);

                // Load font chars info data
                font.glyphs = (GlyphInfo *)RAYGUI_CALLOC(font.glyphCount, sizeof(GlyphInfo));
                for (int i = 0; i < font.glyphCount; i++)
                {
                    fread(&font.glyphs[i].value, 1, sizeof(int), rgsFile);
                    fread(&font.glyphs[i].offsetX, 1, sizeof(int), rgsFile);
                    fread(&font.glyphs[i].offsetY, 1, sizeof(int), rgsFile);
                    fread(&font.glyphs[i].advanceX, 1, sizeof(int), rgsFile);
                }

                GuiSetFont(font);

                // Set font texture source rectangle to be used as white texture to draw shapes
                // NOTE: This way, all gui can be draw using a single draw call
                if ((whiteRec.width != 0) && (whiteRec.height != 0)) SetShapesTexture(font.texture, whiteRec);
            }
#endif
        }

        fclose(rgsFile);
    }
}

// Load style default over global style
void GuiLoadStyleDefault(void)
{
    // We set this variable first to avoid cyclic function calls
    // when calling GuiSetStyle() and GuiGetStyle()
    guiStyleLoaded = true;

    // Initialize default LIGHT style property values
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0x838383ff);
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, 0xc9c9c9ff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x686868ff);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0x5bb2d9ff);
    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, 0xc9effeff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0x6c9bbcff);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0x0492c7ff);
    GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, 0x97e8ffff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, 0x368bafff);
    GuiSetStyle(DEFAULT, BORDER_COLOR_DISABLED, 0xb5c1c2ff);
    GuiSetStyle(DEFAULT, BASE_COLOR_DISABLED, 0xe6e9e9ff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED, 0xaeb7b8ff);
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);                       // WARNING: Some controls use other values
    GuiSetStyle(DEFAULT, TEXT_PADDING, 0);                       // WARNING: Some controls use other values
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER); // WARNING: Some controls use other values

    // Initialize control-specific property values
    // NOTE: Those properties are in default list but require specific values by control type
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
    GuiSetStyle(SLIDER, TEXT_PADDING, 5);
    GuiSetStyle(CHECKBOX, TEXT_PADDING, 5);
    GuiSetStyle(CHECKBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_RIGHT);
    GuiSetStyle(TEXTBOX, TEXT_PADDING, 5);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
    GuiSetStyle(VALUEBOX, TEXT_PADDING, 4);
    GuiSetStyle(VALUEBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
    GuiSetStyle(SPINNER, TEXT_PADDING, 4);
    GuiSetStyle(SPINNER, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
    GuiSetStyle(STATUSBAR, TEXT_PADDING, 6);
    GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);

    // Initialize extended property values
    // NOTE: By default, extended property values are initialized to 0
    GuiSetStyle(DEFAULT, TEXT_SIZE, 10);                // DEFAULT, shared by all controls
    GuiSetStyle(DEFAULT, TEXT_SPACING, 1);              // DEFAULT, shared by all controls
    GuiSetStyle(DEFAULT, LINE_COLOR, 0x90abb5ff);       // DEFAULT specific property
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0xf5f5f5ff); // DEFAULT specific property
    GuiSetStyle(TOGGLE, GROUP_PADDING, 2);
    GuiSetStyle(SLIDER, SLIDER_WIDTH, 15);
    GuiSetStyle(SLIDER, SLIDER_PADDING, 1);
    GuiSetStyle(PROGRESSBAR, PROGRESS_PADDING, 1);
    GuiSetStyle(CHECKBOX, CHECK_PADDING, 1);
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, 30);
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_PADDING, 2);
    GuiSetStyle(DROPDOWNBOX, ARROW_PADDING, 16);
    GuiSetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_PADDING, 2);
    GuiSetStyle(TEXTBOX, TEXT_LINES_PADDING, 5);
    GuiSetStyle(TEXTBOX, TEXT_INNER_PADDING, 4);
    GuiSetStyle(TEXTBOX, COLOR_SELECTED_FG, 0xf0fffeff);
    GuiSetStyle(TEXTBOX, COLOR_SELECTED_BG, 0x839affe0);
    GuiSetStyle(SPINNER, SPIN_BUTTON_WIDTH, 20);
    GuiSetStyle(SPINNER, SPIN_BUTTON_PADDING, 2);
    GuiSetStyle(SCROLLBAR, BORDER_WIDTH, 0);
    GuiSetStyle(SCROLLBAR, ARROWS_VISIBLE, 0);
    GuiSetStyle(SCROLLBAR, ARROWS_SIZE, 6);
    GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING, 0);
    GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, 16);
    GuiSetStyle(SCROLLBAR, SCROLL_PADDING, 0);
    GuiSetStyle(SCROLLBAR, SCROLL_SPEED, 10);
    GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, 0x1e);
    GuiSetStyle(LISTVIEW, LIST_ITEMS_PADDING, 2);
    GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, 10);
    GuiSetStyle(LISTVIEW, SCROLLBAR_SIDE, SCROLLBAR_RIGHT_SIDE);
    GuiSetStyle(COLORPICKER, COLOR_SELECTOR_SIZE, 6);
    GuiSetStyle(COLORPICKER, HUEBAR_WIDTH, 0x14);
    GuiSetStyle(COLORPICKER, HUEBAR_PADDING, 0xa);
    GuiSetStyle(COLORPICKER, HUEBAR_SELECTOR_HEIGHT, 6);
    GuiSetStyle(COLORPICKER, HUEBAR_SELECTOR_OVERFLOW, 2);

    guiFont = GetFontDefault();     // Initialize default font
}

// Get text with icon id prepended
// NOTE: Useful to add icons by name id (enum) instead of
// a number that can change between ricon versions
const char *GuiIconText(int iconId, const char *text)
{
#if defined(RAYGUI_NO_RICONS)
    return NULL;
#else
    static char buffer[1024] = { 0 };
    memset(buffer, 0, 1024);

    sprintf(buffer, "#%03i#", iconId);

    if (text != NULL)
    {
        for (int i = 5; i < 1024; i++)
        {
            buffer[i] = text[i - 5];
            if (text[i - 5] == '\0') break;
        }
    }

    return buffer;
#endif
}

#if !defined(RAYGUI_NO_RICONS)

// Get full icons data pointer
unsigned int *GuiGetIcons(void) { return guiIcons; }

// Load raygui icons file (.rgi)
// NOTE: In case nameIds are required, they can be requested with loadIconsName,
// they are returned as a guiIconsName[iconCount][RICON_MAX_NAME_LENGTH],
// WARNING: guiIconsName[]][] memory should be manually freed!
char **GuiLoadIcons(const char *fileName, bool loadIconsName)
{
    // Style File Structure (.rgi)
    // ------------------------------------------------------
    // Offset  | Size    | Type       | Description
    // ------------------------------------------------------
    // 0       | 4       | char       | Signature: "rGI "
    // 4       | 2       | short      | Version: 100
    // 6       | 2       | short      | reserved

    // 8       | 2       | short      | Num icons (N)
    // 10      | 2       | short      | Icons size (Options: 16, 32, 64) (S)

    // Icons name id (32 bytes per name id)
    // foreach (icon)
    // {
    //   12+32*i  | 32   | char       | Icon NameId
    // }

    // Icons data: One bit per pixel, stored as unsigned int array (depends on icon size)
    // S*S pixels/32bit per unsigned int = K unsigned int per icon
    // foreach (icon)
    // {
    //   ...   | K       | unsigned int | Icon Data
    // }

    FILE *rgiFile = fopen(fileName, "rb");

    char **guiIconsName = NULL;

    if (rgiFile != NULL)
    {
        char signature[5] = "";
        short version = 0;
        short reserved = 0;
        short iconCount = 0;
        short iconSize = 0;

        fread(signature, 1, 4, rgiFile);
        fread(&version, 1, sizeof(short), rgiFile);
        fread(&reserved, 1, sizeof(short), rgiFile);
        fread(&iconCount, 1, sizeof(short), rgiFile);
        fread(&iconSize, 1, sizeof(short), rgiFile);

        if ((signature[0] == 'r') &&
            (signature[1] == 'G') &&
            (signature[2] == 'I') &&
            (signature[3] == ' '))
        {
            if (loadIconsName)
            {
                guiIconsName = (char **)RAYGUI_MALLOC(iconCount*sizeof(char **));
                for (int i = 0; i < iconCount; i++)
                {
                    guiIconsName[i] = (char *)RAYGUI_MALLOC(RICON_MAX_NAME_LENGTH);
                    fread(guiIconsName[i], RICON_MAX_NAME_LENGTH, 1, rgiFile);
                }
            }
            else fseek(rgiFile, iconCount*RICON_MAX_NAME_LENGTH, SEEK_CUR);

            // Read icons data directly over guiIcons data array
            fread(guiIcons, iconCount*(iconSize*iconSize/32), sizeof(unsigned int), rgiFile);
        }

        fclose(rgiFile);
    }

    return guiIconsName;
}

// Draw selected icon using rectangles pixel-by-pixel
void GuiDrawIcon(int iconId, int posX, int posY, int pixelSize, Color color)
{
    #define BIT_CHECK(a,b) ((a) & (1<<(b)))

    for (int i = 0, y = 0; i < RICON_SIZE*RICON_SIZE/32; i++)
    {
        for (int k = 0; k < 32; k++)
        {
            if (BIT_CHECK(guiIcons[iconId*RICON_DATA_ELEMENTS + i], k))
            {
            #if !defined(RAYGUI_STANDALONE)
                DrawRectangle(posX + (k%RICON_SIZE)*pixelSize, posY + y*pixelSize, pixelSize, pixelSize, color);
            #endif
            }

            if ((k == 15) || (k == 31)) y++;
        }
    }
}

// Get icon bit data
// NOTE: Bit data array grouped as unsigned int (ICON_SIZE*ICON_SIZE/32 elements)
unsigned int *GuiGetIconData(int iconId)
{
    static unsigned int iconData[RICON_DATA_ELEMENTS] = { 0 };
    memset(iconData, 0, RICON_DATA_ELEMENTS*sizeof(unsigned int));

    if (iconId < RICON_MAX_ICONS) memcpy(iconData, &guiIcons[iconId*RICON_DATA_ELEMENTS], RICON_DATA_ELEMENTS*sizeof(unsigned int));

    return iconData;
}

// Set icon bit data
// NOTE: Data must be provided as unsigned int array (ICON_SIZE*ICON_SIZE/32 elements)
void GuiSetIconData(int iconId, unsigned int *data)
{
    if (iconId < RICON_MAX_ICONS) memcpy(&guiIcons[iconId*RICON_DATA_ELEMENTS], data, RICON_DATA_ELEMENTS*sizeof(unsigned int));
}

// Set icon pixel value
void GuiSetIconPixel(int iconId, int x, int y)
{
    #define BIT_SET(a,b)   ((a) |= (1<<(b)))

    // This logic works for any RICON_SIZE pixels icons,
    // For example, in case of 16x16 pixels, every 2 lines fit in one unsigned int data element
    BIT_SET(guiIcons[iconId*RICON_DATA_ELEMENTS + y/(sizeof(unsigned int)*8/RICON_SIZE)], x + (y%(sizeof(unsigned int)*8/RICON_SIZE)*RICON_SIZE));
}

// Clear icon pixel value
void GuiClearIconPixel(int iconId, int x, int y)
{
    #define BIT_CLEAR(a,b) ((a) &= ~((1)<<(b)))

    // This logic works for any RICON_SIZE pixels icons,
    // For example, in case of 16x16 pixels, every 2 lines fit in one unsigned int data element
    BIT_CLEAR(guiIcons[iconId*RICON_DATA_ELEMENTS + y/(sizeof(unsigned int)*8/RICON_SIZE)], x + (y%(sizeof(unsigned int)*8/RICON_SIZE)*RICON_SIZE));
}

// Check icon pixel value
bool GuiCheckIconPixel(int iconId, int x, int y)
{
    #define BIT_CHECK(a,b) ((a) & (1<<(b)))

    return (BIT_CHECK(guiIcons[iconId*8 + y/2], x + (y%2*16)));
}
#endif      // !RAYGUI_NO_RICONS

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
// Gui get text width using default font
// NOTE: Icon is not considered here
static int GetTextWidth(const char *text)
{
    Vector2 size = { 0 };

    if ((text != NULL) && (text[0] != '\0'))
    {
        size = MeasureTextEx(guiFont, text, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SPACING));
    }

    return (int)size.x;
}

// Get text bounds considering control bounds
static Rectangle GetTextBounds(int control, Rectangle bounds)
{
    Rectangle textBounds = bounds;

    textBounds.x = bounds.x + GuiGetStyle(control, BORDER_WIDTH);
    textBounds.y = bounds.y + GuiGetStyle(control, BORDER_WIDTH);
    textBounds.width = bounds.width - 2*GuiGetStyle(control, BORDER_WIDTH);
    textBounds.height = bounds.height - 2*GuiGetStyle(control, BORDER_WIDTH);

    // Consider TEXT_PADDING properly, depends on control type and TEXT_ALIGNMENT
    switch (control)
    {
        case COMBOBOX: bounds.width -= (GuiGetStyle(control, COMBO_BUTTON_WIDTH) + GuiGetStyle(control, COMBO_BUTTON_PADDING)); break;
        case VALUEBOX: break;   // NOTE: ValueBox text value always centered, text padding applies to label
        default:
        {
            if (GuiGetStyle(control, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_RIGHT) textBounds.x -= GuiGetStyle(control, TEXT_PADDING);
            else textBounds.x += GuiGetStyle(control, TEXT_PADDING);
        } break;
    }

    // TODO: Special cases (no label): COMBOBOX, DROPDOWNBOX, LISTVIEW (scrollbar?)
    // More special cases (label on side): CHECKBOX, SLIDER, VALUEBOX, SPINNER

    return textBounds;
}

// Get text icon if provided and move text cursor
// NOTE: We support up to 999 values for iconId
static const char *GetTextIcon(const char *text, int *iconId)
{
#if !defined(RAYGUI_NO_RICONS)
    *iconId = -1;
    if (text[0] == '#')     // Maybe we have an icon!
    {
        char iconValue[4] = { 0 };  // Maximum length for icon value: 3 digits + '\0'

        int pos = 1;
        while ((pos < 4) && (text[pos] >= '0') && (text[pos] <= '9'))
        {
            iconValue[pos - 1] = text[pos];
            pos++;
        }

        if (text[pos] == '#')
        {
            *iconId = TextToInteger(iconValue);

            // Move text pointer after icon
            // WARNING: If only icon provided, it could point to EOL character: '\0'
            if (*iconId >= 0) text += (pos + 1);
        }
    }
#endif

    return text;
}

// Gui draw text using default font
static void GuiDrawText(const char *text, Rectangle bounds, int alignment, Color tint)
{
    #define TEXT_VALIGN_PIXEL_OFFSET(h)  ((int)h%2)     // Vertical alignment for pixel perfect

    if ((text != NULL) && (text[0] != '\0'))
    {
        int iconId = 0;
        text = GetTextIcon(text, &iconId);              // Check text for icon and move cursor

        // Get text position depending on alignment and iconId
        //---------------------------------------------------------------------------------
        #define RICON_TEXT_PADDING   4

        Vector2 position = { bounds.x, bounds.y };

        // NOTE: We get text size after icon has been processed
        int textWidth = GetTextWidth(text);
        int textHeight = GuiGetStyle(DEFAULT, TEXT_SIZE);

        // If text requires an icon, add size to measure
        if (iconId >= 0)
        {
            textWidth += RICON_SIZE;

            // WARNING: If only icon provided, text could be pointing to EOF character: '\0'
            if ((text != NULL) && (text[0] != '\0')) textWidth += RICON_TEXT_PADDING;
        }

        // Check guiTextAlign global variables
        switch (alignment)
        {
            case GUI_TEXT_ALIGN_LEFT:
            {
                position.x = bounds.x;
                position.y = bounds.y + bounds.height/2 - textHeight/2 + TEXT_VALIGN_PIXEL_OFFSET(bounds.height);
            } break;
            case GUI_TEXT_ALIGN_CENTER:
            {
                position.x = bounds.x + bounds.width/2 - textWidth/2;
                position.y = bounds.y + bounds.height/2 - textHeight/2 + TEXT_VALIGN_PIXEL_OFFSET(bounds.height);
            } break;
            case GUI_TEXT_ALIGN_RIGHT:
            {
                position.x = bounds.x + bounds.width - textWidth;
                position.y = bounds.y + bounds.height/2 - textHeight/2 + TEXT_VALIGN_PIXEL_OFFSET(bounds.height);
            } break;
            default: break;
        }

        // NOTE: Make sure we get pixel-perfect coordinates,
        // In case of decimals we got weird text positioning
        position.x = (float)((int)position.x);
        position.y = (float)((int)position.y);
        //---------------------------------------------------------------------------------

        // Draw text (with icon if available)
        //---------------------------------------------------------------------------------
#if !defined(RAYGUI_NO_RICONS)
        if (iconId >= 0)
        {
            // NOTE: We consider icon height, probably different than text size
            GuiDrawIcon(iconId, (int)position.x, (int)(bounds.y + bounds.height/2 - RICON_SIZE/2 + TEXT_VALIGN_PIXEL_OFFSET(bounds.height)), 1, tint);
            position.x += (RICON_SIZE + RICON_TEXT_PADDING);
        }
#endif
        DrawTextEx(guiFont, text, position, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SPACING), tint);
        //---------------------------------------------------------------------------------
    }
}

// Gui draw rectangle using default raygui plain style with borders
static void GuiDrawRectangle(Rectangle rec, int borderWidth, Color borderColor, Color color)
{
    if (color.a > 0)
    {
        // Draw rectangle filled with color
        DrawRectangle((int)rec.x, (int)rec.y, (int)rec.width, (int)rec.height, color);
    }

    if (borderWidth > 0)
    {
        // Draw rectangle border lines with color
        DrawRectangle((int)rec.x, (int)rec.y, (int)rec.width, borderWidth, borderColor);
        DrawRectangle((int)rec.x, (int)rec.y + borderWidth, borderWidth, (int)rec.height - 2*borderWidth, borderColor);
        DrawRectangle((int)rec.x + (int)rec.width - borderWidth, (int)rec.y + borderWidth, borderWidth, (int)rec.height - 2*borderWidth, borderColor);
        DrawRectangle((int)rec.x, (int)rec.y + (int)rec.height - borderWidth, (int)rec.width, borderWidth, borderColor);
    }
}

// Split controls text into multiple strings
// Also check for multiple columns (required by GuiToggleGroup())
static const char **GuiTextSplit(const char *text, int *count, int *textRow)
{
    // NOTE: Current implementation returns a copy of the provided string with '\0' (string end delimiter)
    // inserted between strings defined by "delimiter" parameter. No memory is dynamically allocated,
    // all used memory is static... it has some limitations:
    //      1. Maximum number of possible split strings is set by TEXTSPLIT_MAX_TEXT_ELEMENTS
    //      2. Maximum size of text to split is TEXTSPLIT_MAX_TEXT_LENGTH
    // NOTE: Those definitions could be externally provided if required

    #if !defined(TEXTSPLIT_MAX_TEXT_LENGTH)
        #define TEXTSPLIT_MAX_TEXT_LENGTH      1024
    #endif

    #if !defined(TEXTSPLIT_MAX_TEXT_ELEMENTS)
        #define TEXTSPLIT_MAX_TEXT_ELEMENTS     128
    #endif

    static const char *result[TEXTSPLIT_MAX_TEXT_ELEMENTS] = { NULL };
    static char buffer[TEXTSPLIT_MAX_TEXT_LENGTH] = { 0 };
    memset(buffer, 0, TEXTSPLIT_MAX_TEXT_LENGTH);

    result[0] = buffer;
    int counter = 1;

    if (textRow != NULL) textRow[0] = 0;

    // Count how many substrings we have on text and point to every one
    for (int i = 0; i < TEXTSPLIT_MAX_TEXT_LENGTH; i++)
    {
        buffer[i] = text[i];
        if (buffer[i] == '\0') break;
        else if ((buffer[i] == ';') || (buffer[i] == '\n'))
        {
            result[counter] = buffer + i + 1;

            if (textRow != NULL)
            {
                if (buffer[i] == '\n') textRow[counter] = textRow[counter - 1] + 1;
                else textRow[counter] = textRow[counter - 1];
            }

            buffer[i] = '\0';   // Set an end of string at this point

            counter++;
            if (counter == TEXTSPLIT_MAX_TEXT_ELEMENTS) break;
        }
    }

    *count = counter;

    return result;
}

// Convert color data from RGB to HSV
// NOTE: Color data should be passed normalized
static Vector3 ConvertRGBtoHSV(Vector3 rgb)
{
    Vector3 hsv = { 0 };
    float min = 0.0f;
    float max = 0.0f;
    float delta = 0.0f;

    min = (rgb.x < rgb.y)? rgb.x : rgb.y;
    min = (min < rgb.z)? min  : rgb.z;

    max = (rgb.x > rgb.y)? rgb.x : rgb.y;
    max = (max > rgb.z)? max  : rgb.z;

    hsv.z = max;            // Value
    delta = max - min;

    if (delta < 0.00001f)
    {
        hsv.y = 0.0f;
        hsv.x = 0.0f;           // Undefined, maybe NAN?
        return hsv;
    }

    if (max > 0.0f)
    {
        // NOTE: If max is 0, this divide would cause a crash
        hsv.y = (delta/max);    // Saturation
    }
    else
    {
        // NOTE: If max is 0, then r = g = b = 0, s = 0, h is undefined
        hsv.y = 0.0f;
        hsv.x = 0.0f;           // Undefined, maybe NAN?
        return hsv;
    }

    // NOTE: Comparing float values could not work properly
    if (rgb.x >= max) hsv.x = (rgb.y - rgb.z)/delta;    // Between yellow & magenta
    else
    {
        if (rgb.y >= max) hsv.x = 2.0f + (rgb.z - rgb.x)/delta;  // Between cyan & yellow
        else hsv.x = 4.0f + (rgb.x - rgb.y)/delta;      // Between magenta & cyan
    }

    hsv.x *= 60.0f;     // Convert to degrees

    if (hsv.x < 0.0f) hsv.x += 360.0f;

    return hsv;
}

// Convert color data from HSV to RGB
// NOTE: Color data should be passed normalized
static Vector3 ConvertHSVtoRGB(Vector3 hsv)
{
    Vector3 rgb = { 0 };
    float hh = 0.0f, p = 0.0f, q = 0.0f, t = 0.0f, ff = 0.0f;
    long i = 0;

    // NOTE: Comparing float values could not work properly
    if (hsv.y <= 0.0f)
    {
        rgb.x = hsv.z;
        rgb.y = hsv.z;
        rgb.z = hsv.z;
        return rgb;
    }

    hh = hsv.x;
    if (hh >= 360.0f) hh = 0.0f;
    hh /= 60.0f;

    i = (long)hh;
    ff = hh - i;
    p = hsv.z*(1.0f - hsv.y);
    q = hsv.z*(1.0f - (hsv.y*ff));
    t = hsv.z*(1.0f - (hsv.y*(1.0f - ff)));

    switch (i)
    {
        case 0:
        {
            rgb.x = hsv.z;
            rgb.y = t;
            rgb.z = p;
        } break;
        case 1:
        {
            rgb.x = q;
            rgb.y = hsv.z;
            rgb.z = p;
        } break;
        case 2:
        {
            rgb.x = p;
            rgb.y = hsv.z;
            rgb.z = t;
        } break;
        case 3:
        {
            rgb.x = p;
            rgb.y = q;
            rgb.z = hsv.z;
        } break;
        case 4:
        {
            rgb.x = t;
            rgb.y = p;
            rgb.z = hsv.z;
        } break;
        case 5:
        default:
        {
            rgb.x = hsv.z;
            rgb.y = p;
            rgb.z = q;
        } break;
    }

    return rgb;
}

#if defined(RAYGUI_STANDALONE)
// Returns a Color struct from hexadecimal value
static Color GetColor(int hexValue)
{
    Color color;

    color.r = (unsigned char)(hexValue >> 24) & 0xFF;
    color.g = (unsigned char)(hexValue >> 16) & 0xFF;
    color.b = (unsigned char)(hexValue >> 8) & 0xFF;
    color.a = (unsigned char)hexValue & 0xFF;

    return color;
}

// Returns hexadecimal value for a Color
static int ColorToInt(Color color)
{
    return (((int)color.r << 24) | ((int)color.g << 16) | ((int)color.b << 8) | (int)color.a);
}

// Check if point is inside rectangle
static bool CheckCollisionPointRec(Vector2 point, Rectangle rec)
{
    bool collision = false;

    if ((point.x >= rec.x) && (point.x <= (rec.x + rec.width)) &&
        (point.y >= rec.y) && (point.y <= (rec.y + rec.height))) collision = true;

    return collision;
}

// Color fade-in or fade-out, alpha goes from 0.0f to 1.0f
static Color Fade(Color color, float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    else if (alpha > 1.0f) alpha = 1.0f;

    Color result = { color.r, color.g, color.b, (unsigned char)(255.0f*alpha) };

    return result;
}

// Formatting of text with variables to 'embed'
static const char *TextFormat(const char *text, ...)
{
    #define MAX_FORMATTEXT_LENGTH   64

    static char buffer[MAX_FORMATTEXT_LENGTH];

    va_list args;
    va_start(args, text);
    vsprintf(buffer, text, args);
    va_end(args);

    return buffer;
}

// Draw rectangle with vertical gradient fill color
// NOTE: This function is only used by GuiColorPicker()
static void DrawRectangleGradientV(int posX, int posY, int width, int height, Color color1, Color color2)
{
    Rectangle bounds = { (float)posX, (float)posY, (float)width, (float)height };
    DrawRectangleGradientEx(bounds, color1, color2, color2, color1);
}

#define TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH    1024        // Size of static buffer: TextSplit()
#define TEXTSPLIT_MAX_SUBSTRINGS_COUNT       128        // Size of static pointers array: TextSplit()

// Split string into multiple strings
const char **TextSplit(const char *text, char delimiter, int *count)
{
    // NOTE: Current implementation returns a copy of the provided string with '\0' (string end delimiter)
    // inserted between strings defined by "delimiter" parameter. No memory is dynamically allocated,
    // all used memory is static... it has some limitations:
    //      1. Maximum number of possible split strings is set by TEXTSPLIT_MAX_SUBSTRINGS_COUNT
    //      2. Maximum size of text to split is TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH

    static const char *result[TEXTSPLIT_MAX_SUBSTRINGS_COUNT] = { NULL };
    static char buffer[TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH);

    result[0] = buffer;
    int counter = 0;

    if (text != NULL)
    {
        counter = 1;

        // Count how many substrings we have on text and point to every one
        for (int i = 0; i < TEXTSPLIT_MAX_TEXT_BUFFER_LENGTH; i++)
        {
            buffer[i] = text[i];
            if (buffer[i] == '\0') break;
            else if (buffer[i] == delimiter)
            {
                buffer[i] = '\0';   // Set an end of string at this point
                result[counter] = buffer + i + 1;
                counter++;

                if (counter == TEXTSPLIT_MAX_SUBSTRINGS_COUNT) break;
            }
        }
    }

    *count = counter;
    return result;
}

// Get integer value from text
// NOTE: This function replaces atoi() [stdlib.h]
static int TextToInteger(const char *text)
{
    int value = 0;
    int sign = 1;

    if ((text[0] == '+') || (text[0] == '-'))
    {
        if (text[0] == '-') sign = -1;
        text++;
    }

    for (int i = 0; ((text[i] >= '0') && (text[i] <= '9')); ++i) value = value*10 + (int)(text[i] - '0');

    return value*sign;
}

// Encode codepoint into UTF-8 text (char array size returned as parameter)
static const char *CodepointToUTF8(int codepoint, int *byteSize)
{
    static char utf8[6] = { 0 };
    int size = 0;

    if (codepoint <= 0x7f)
    {
        utf8[0] = (char)codepoint;
        size = 1;
    }
    else if (codepoint <= 0x7ff)
    {
        utf8[0] = (char)(((codepoint >> 6) & 0x1f) | 0xc0);
        utf8[1] = (char)((codepoint & 0x3f) | 0x80);
        size = 2;
    }
    else if (codepoint <= 0xffff)
    {
        utf8[0] = (char)(((codepoint >> 12) & 0x0f) | 0xe0);
        utf8[1] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[2] = (char)((codepoint & 0x3f) | 0x80);
        size = 3;
    }
    else if (codepoint <= 0x10ffff)
    {
        utf8[0] = (char)(((codepoint >> 18) & 0x07) | 0xf0);
        utf8[1] = (char)(((codepoint >> 12) & 0x3f) | 0x80);
        utf8[2] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[3] = (char)((codepoint & 0x3f) | 0x80);
        size = 4;
    }

    *byteSize = size;

    return utf8;
}

// Get next codepoint in a UTF-8 encoded text, scanning until '\0' is found
// When a invalid UTF-8 byte is encountered we exit as soon as possible and a '?'(0x3f) codepoint is returned
// Total number of bytes processed are returned as a parameter
// NOTE: the standard says U+FFFD should be returned in case of errors
// but that character is not supported by the default font in raylib
static int GetCodepoint(const char *text, int *bytesProcessed)
{
/*
    UTF-8 specs from https://www.ietf.org/rfc/rfc3629.txt

    Char. number range  |        UTF-8 octet sequence
      (hexadecimal)    |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
    // NOTE: on decode errors we return as soon as possible

    int code = 0x3f;   // Codepoint (defaults to '?')
    int octet = (unsigned char)(text[0]); // The first UTF8 octet
    *bytesProcessed = 1;

    if (octet <= 0x7f)
    {
        // Only one octet (ASCII range x00-7F)
        code = text[0];
    }
    else if ((octet & 0xe0) == 0xc0)
    {
        // Two octets

        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        unsigned char octet1 = text[1];

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } // Unexpected sequence

        if ((octet >= 0xc2) && (octet <= 0xdf))
        {
            code = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            *bytesProcessed = 2;
        }
    }
    else if ((octet & 0xf0) == 0xe0)
    {
        // Three octets
        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; } // Unexpected sequence

        // [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
        // [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        // [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
        // [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)

        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) { *bytesProcessed = 2; return code; }

        if ((octet >= 0xe0) && (0 <= 0xef))
        {
            code = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            *bytesProcessed = 3;
        }
    }
    else if ((octet & 0xf8) == 0xf0)
    {
        // Four octets
        if (octet > 0xf4) return code;

        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';
        unsigned char octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; }  // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; }  // Unexpected sequence

        octet3 = text[3];

        if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { *bytesProcessed = 4; return code; }  // Unexpected sequence

        // [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
        // [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
        // [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail

        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) { *bytesProcessed = 2; return code; } // Unexpected sequence

        if (octet >= 0xf0)
        {
            code = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            *bytesProcessed = 4;
        }
    }

    if (code > 0x10ffff) code = 0x3f;     // Codepoints after U+10ffff are invalid

    return code;
}
#endif      // RAYGUI_STANDALONE

#endif      // RAYGUI_IMPLEMENTATION
