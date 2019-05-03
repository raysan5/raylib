/*******************************************************************************************
*
*   raygui v2.0-dev - A simple and easy-to-use immedite-mode-gui library
*
*   DESCRIPTION:
*
*   raygui is a tools-dev-focused immediate-mode-gui library based on raylib but also possible
*   to be used as a standalone library, as long as input and drawing functions are provided.
*
*   Basic controls provided:
*
*       - WindowBox
*       - GroupBox
*       - Line
*       - Panel
*       - Label
*       - Button
*       - LabelButton   --> Label
*       - ImageButton   --> Button
*       - ImageButtonEx --> Button
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
*       - ScrollPanel
*       - ListView      --> ListElement
*       - ColorPicker   --> ColorPanel, ColorBarHue
*       - MessageBox
*       - DummyRec
*       - ScrollBar
*       - Grid
*
*   It also provides a set of functions for styling the controls based on its properties (size, color).
*
*   CONFIGURATION:
*
*   #define RAYGUI_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define RAYGUI_STATIC (defined by default)
*       The generated implementation will stay private inside implementation file and all
*       internal symbols and functions will only be visible inside that file.
*
*   #define RAYGUI_STANDALONE
*       Avoid raylib.h header inclusion in this file. Data types defined on raylib are defined
*       internally in the library and input management and drawing functions must be provided by
*       the user (check library implementation for further details).
*
*   #define RAYGUI_RICONS_SUPPORT
*       Includes ricons.h header defining a set of 128 icons (binary format) to be used on
*       multiple controls and following raygui styles
*
*   VERSIONS HISTORY:
*       2.0 (xx-Dec-2018) Complete review of new controls, redesigned style system
*       1.9 (01-May-2018) Lot of rework and redesign! Lots of new controls!
*       1.5 (21-Jun-2017) Working in an improved styles system
*       1.4 (15-Jun-2017) Rewritten all GUI functions (removed useless ones)
*       1.3 (12-Jun-2017) Redesigned styles system
*       1.1 (01-Jun-2017) Complete review of the library
*       1.0 (07-Jun-2016) Converted to header-only by Ramon Santamaria.
*       0.9 (07-Mar-2016) Reviewed and tested by Albert Martos, Ian Eito, Sergio Martinez and Ramon Santamaria.
*       0.8 (27-Aug-2015) Initial release. Implemented by Kevin Gato, Daniel Nicolás and Ramon Santamaria.
*
*   CONTRIBUTORS:
*       Ramon Santamaria:   Supervision, review, redesign, update and maintenance...
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
*   Copyright (c) 2014-2018 Ramon Santamaria (@raysan5)
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

#define RAYGUI_VERSION  "2.0-dev"

#if !defined(RAYGUI_STANDALONE)
    #include "raylib.h"
#endif

#if defined(RAYGUI_IMPLEMENTATION)
    #if defined(_WIN32) && defined(BUILD_LIBTYPE_SHARED)
        #define RAYGUIDEF __declspec(dllexport) extern  // We are building raygui as a Win32 shared library (.dll).
    #elif defined(_WIN32) && defined(USE_LIBTYPE_SHARED)
        #define RAYGUIDEF __declspec(dllimport)         // We are using raygui as a Win32 shared library (.dll)
    #else
        #ifdef __cplusplus
            #define RAYGUIDEF extern "C"    // Functions visible from other files (no name mangling of functions in C++)
        #else
            #define RAYGUIDEF extern        // Functions visible from other files
        #endif
    #endif
#elif defined(RAYGUI_STATIC)
    #define RAYGUIDEF static                // Functions just visible to module including this file
#endif

#include <stdlib.h>                         // Required for: atoi()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define VALIGN_OFFSET(h)         ((int)h%2)     // Vertical alignment for pixel perfect

#define TEXTEDIT_CURSOR_BLINK_FRAMES    20      // Text edit controls cursor blink timming

#define NUM_CONTROLS                    13      // Number of standard controls
#define NUM_PROPS_DEFAULT               16      // Number of standard properties
#define NUM_PROPS_EXTENDED               8      // Number of extended properties

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

    // Vector3 type
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
        int x;
        int y;
        int width;
        int height;
    } Rectangle;
    
    // Texture2D type
    typedef struct Texture2D { } Texture2D;
    
    // Font type
    typedef struct Font { } Font;
#endif

// Gui global state enum
typedef enum {
    GUI_STATE_NORMAL = 0,
    GUI_STATE_FOCUSED,
    GUI_STATE_PRESSED,
    GUI_STATE_DISABLED,
} GuiControlState;

// Gui global text alignment
typedef enum {
    GUI_TEXT_ALIGN_LEFT = 0,
    GUI_TEXT_ALIGN_CENTER,
    GUI_TEXT_ALIGN_RIGHT,
} GuiTextAlignment;

// Gui standard controls
typedef enum {
    DEFAULT = 0,
    LABEL,          // LABELBUTTON
    BUTTON,         // IMAGEBUTTON
    TOGGLE,         // TOGGLEGROUP
    SLIDER,         // SLIDERBAR
    PROGRESSBAR,
    CHECKBOX,
    COMBOBOX,
    DROPDOWNBOX,
    TEXTBOX,        // VALUEBOX, SPINNER, TEXTBOXMULTI -> TODO: Probably they should not be dependant on TEXTBOX style!
    LISTVIEW,
    COLORPICKER,
    SCROLLBAR
} GuiControlStandard;

// Gui default properties for every control
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
    INNER_PADDING,
    TEXT_ALIGNMENT,
    RESERVED02
} GuiControlProperty;

// Gui extended properties depending on control type
// NOTE: We reserve a fixed size of additional properties per control (8)

// Default properties
typedef enum {
    TEXT_SIZE = 16,
    TEXT_SPACING,
    LINE_COLOR,
    //LINE_THICK,
    BACKGROUND_COLOR,
} GuiDefaultProperty;

// Label
//typedef enum { } GuiLabelProperty;

// Button
//typedef enum { } GuiButtonProperty;

// Toggle / ToggleGroup
typedef enum {
    GROUP_PADDING = 16,
} GuiToggleProperty;

// Slider / SliderBar
typedef enum {
    SLIDER_WIDTH = 16,
    TEXT_PADDING
} GuiSliderProperty;

// ProgressBar
//typedef enum { } GuiProgressBarProperty;

// TextBox / TextBoxMulti / ValueBox / Spinner
typedef enum {
    MULTILINE_PADDING = 16,
    SPINNER_BUTTON_WIDTH,
    SPINNER_BUTTON_PADDING,
    SPINNER_BUTTON_BORDER_WIDTH
} GuiTextBoxProperty;

// CheckBox
typedef enum {
    CHECK_TEXT_PADDING = 16
} GuiCheckBoxProperty;

// ComboBox
typedef enum {
    SELECTOR_WIDTH = 16,
    SELECTOR_PADDING
} GuiComboBoxProperty;

// DropdownBox
typedef enum {
    ARROW_RIGHT_PADDING = 16,
} GuiDropdownBoxProperty;

// ColorPicker
typedef enum {
    COLOR_SELECTOR_SIZE = 16,
    BAR_WIDTH,                  // Lateral bar width
    BAR_PADDING,                // Lateral bar separation from panel
    BAR_SELECTOR_HEIGHT,        // Lateral bar selector height
    BAR_SELECTOR_PADDING        // Lateral bar selector outer padding
} GuiColorPickerProperty;

// ListView
typedef enum {
    ELEMENTS_HEIGHT = 16,
    ELEMENTS_PADDING,
    SCROLLBAR_WIDTH,
    SCROLLBAR_SIDE,             // This property defines vertical scrollbar side (SCROLLBAR_LEFT_SIDE or SCROLLBAR_RIGHT_SIDE)
} GuiListViewProperty;

// ScrollBar
typedef enum {
    ARROWS_SIZE = 16,
    SLIDER_PADDING,
    SLIDER_SIZE,
    SCROLL_SPEED,
    SHOW_SPINNER_BUTTONS
} GuiScrollBarProperty;

// ScrollBar side
typedef enum {
    SCROLLBAR_LEFT_SIDE = 0,
    SCROLLBAR_RIGHT_SIDE
} GuiScrollBarSide;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// Global gui modification functions
RAYGUIDEF void GuiEnable(void);                                         // Enable gui controls (global state)
RAYGUIDEF void GuiDisable(void);                                        // Disable gui controls (global state)
RAYGUIDEF void GuiLock(void);                                           // Lock gui controls (global state)
RAYGUIDEF void GuiUnlock(void);                                         // Unlock gui controls (global state)
RAYGUIDEF void GuiState(int state);                                     // Set gui state (global state)
RAYGUIDEF void GuiFont(Font font);                                      // Set gui custom font (global state)
RAYGUIDEF void GuiFade(float alpha);                                    // Set gui controls alpha (global state), alpha goes from 0.0f to 1.0f

// Style set/get functions
RAYGUIDEF void GuiSetStyle(int control, int property, int value);       // Set one style property
RAYGUIDEF int GuiGetStyle(int control, int property);                   // Get one style property

// Container/separator controls, useful for controls organization
RAYGUIDEF bool GuiWindowBox(Rectangle bounds, const char *text);                                        // Window Box control, shows a window that can be closed
RAYGUIDEF void GuiGroupBox(Rectangle bounds, const char *text);                                         // Group Box control with title name
RAYGUIDEF void GuiLine(Rectangle bounds, const char *text);                                             // Line separator control, could contain text
RAYGUIDEF void GuiPanel(Rectangle bounds);                                                              // Panel control, useful to group controls
RAYGUIDEF Rectangle GuiScrollPanel(Rectangle bounds, Rectangle content, Vector2 *scroll);               // Scroll Panel control

// Basic controls set
RAYGUIDEF void GuiLabel(Rectangle bounds, const char *text);                                            // Label control, shows text
RAYGUIDEF bool GuiButton(Rectangle bounds, const char *text);                                           // Button control, returns true when clicked
RAYGUIDEF bool GuiLabelButton(Rectangle bounds, const char *text);                                      // Label button control, show true when clicked
RAYGUIDEF bool GuiImageButton(Rectangle bounds, Texture2D texture);                                     // Image button control, returns true when clicked
RAYGUIDEF bool GuiImageButtonEx(Rectangle bounds, Texture2D texture, Rectangle texSource, const char *text);        // Image button extended control, returns true when clicked
RAYGUIDEF bool GuiToggle(Rectangle bounds, const char *text, bool active);                              // Toggle Button control, returns true when active
RAYGUIDEF int GuiToggleGroup(Rectangle bounds, const char *text, int active);                           // Toggle Group control, returns active toggle index
RAYGUIDEF bool GuiCheckBox(Rectangle bounds, const char *text, bool checked);                           // Check Box control, returns true when active
RAYGUIDEF int GuiComboBox(Rectangle bounds, const char *text, int active);                              // Combo Box control, returns selected item index
RAYGUIDEF bool GuiDropdownBox(Rectangle bounds, const char *text, int *active, bool editMode);          // Dropdown Box control, returns selected item
RAYGUIDEF bool GuiSpinner(Rectangle bounds, int *value, int minValue, int maxValue, bool editMode);     // Spinner control, returns selected value
RAYGUIDEF bool GuiValueBox(Rectangle bounds, int *value, int minValue, int maxValue, bool editMode);    // Value Box control, updates input text with numbers
RAYGUIDEF bool GuiTextBox(Rectangle bounds, char *text, int textSize, bool editMode);                   // Text Box control, updates input text
RAYGUIDEF bool GuiTextBoxMulti(Rectangle bounds, char *text, int textSize, bool editMode);              // Text Box control with multiple lines
RAYGUIDEF float GuiSlider(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue);       // Slider control, returns selected value
RAYGUIDEF float GuiSliderBar(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue);    // Slider Bar control, returns selected value
RAYGUIDEF float GuiProgressBar(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue);  // Progress Bar control, shows current progress value
RAYGUIDEF void GuiStatusBar(Rectangle bounds, const char *text);                                        // Status Bar control, shows info text
RAYGUIDEF void GuiDummyRec(Rectangle bounds, const char *text);                                         // Dummy control for placeholders
RAYGUIDEF int GuiScrollBar(Rectangle bounds, int value, int minValue, int maxValue);                    // Scroll Bar control

// Advance controls set
RAYGUIDEF bool GuiListView(Rectangle bounds, const char *text, int *active, int *scrollIndex, bool editMode);       // List View control, returns selected list element index
RAYGUIDEF bool GuiListViewEx(Rectangle bounds, const char **text, int count, int *enabled, int *active, int *focus, int *scrollIndex, bool editMode); // List View with extended parameters
RAYGUIDEF int GuiMessageBox(Rectangle bounds, const char *windowTitle, const char *message, const char *buttons);   // Message Box control, displays a message
RAYGUIDEF Color GuiColorPicker(Rectangle bounds, Color color);                                          // Color Picker control
RAYGUIDEF Vector2 GuiGrid(Rectangle bounds, float spacing, int subdivs);                                // Grid

// Styles loading functions
RAYGUIDEF void GuiLoadStyle(const char *fileName);              // Load style file (.rgs)
RAYGUIDEF void GuiLoadStyleProps(const int *props, int count);  // Load style properties from array
RAYGUIDEF void GuiLoadStyleDefault(void);                       // Load style default over global style
RAYGUIDEF void GuiUpdateStyleComplete(void);                    // Updates full style properties set with default values

/*
typedef GuiStyle (unsigned int *)
RAYGUIDEF GuiStyle LoadGuiStyle(const char *fileName);          // Load style from file (.rgs)
RAYGUIDEF void UnloadGuiStyle(GuiStyle style);                  // Unload style
*/

RAYGUIDEF const char *GuiIconText(int iconId, const char *text); // Get text with icon id prepended

#endif // RAYGUI_H


/***********************************************************************************
*
*   RAYGUI IMPLEMENTATION
*
************************************************************************************/

#if defined(RAYGUI_IMPLEMENTATION)

#if defined(RAYGUI_RICONS_SUPPORT)
    #if defined(RAYGUI_STANDALONE)
        #define RICONS_STANDALONE
    #endif
    
    #define RICONS_IMPLEMENTATION
    #include "ricons.h"     // Required for: raygui icons
#endif

#include <stdio.h>          // Required for: FILE, fopen(), fclose(), fprintf(), feof(), fscanf(), vsprintf()
#include <string.h>         // Required for: strlen() on GuiTextBox()

#if defined(RAYGUI_STANDALONE)
    #include <stdarg.h>     // Required for: va_list, va_start(), vfprintf(), va_end()
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Gui control property style element
typedef enum { BORDER = 0, BASE, TEXT, OTHER } GuiPropertyElement;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static GuiControlState guiState = GUI_STATE_NORMAL;

static Font guiFont = { 0 };            // NOTE: Highly coupled to raylib
static bool guiLocked = false;
static float guiAlpha = 1.0f;

// Global gui style array (allocated on heap by default)
// NOTE: In raygui we manage a single int array with all the possible style properties.
// When a new style is loaded, it loads over the global style... but default gui style
// could always be recovered with GuiLoadStyleDefault()
static unsigned int guiStyle[NUM_CONTROLS*(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED)] = { 0 };
static bool guiStyleLoaded = false;

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

#ifdef __cplusplus
    #define CLITERAL
#else
    #define CLITERAL    (Color)
#endif

#define WHITE      CLITERAL{ 255, 255, 255, 255 }   // White
#define BLACK      CLITERAL{ 0, 0, 0, 255 }         // Black
#define RAYWHITE   CLITERAL{ 245, 245, 245, 255 }   // My own White (raylib logo)
#define GRAY       CLITERAL{ 130, 130, 130, 255 }   // Gray -- GuiColorBarAlpha()

// raylib functions are already implemented in raygui
//-------------------------------------------------------------------------------
static Color GetColor(int hexValue);                // Returns a Color struct from hexadecimal value
static int ColorToInt(Color color);                 // Returns hexadecimal value for a Color
static Color Fade(Color color, float alpha);        // Color fade-in or fade-out, alpha goes from 0.0f to 1.0f
static bool CheckCollisionPointRec(Vector2 point, Rectangle rec);   // Check if point is inside rectangle
static const char *TextFormat(const char *text, ...);               // Formatting of text with variables to 'embed'
//-------------------------------------------------------------------------------

// Input required functions
//-------------------------------------------------------------------------------
static Vector2 GetMousePosition(void) { return (Vector2){ 0, 0 }; }
static int GetMouseWheelMove(void) { return 0; }
static bool IsMouseButtonDown(int button) { return false; }
static bool IsMouseButtonPressed(int button) { return false; }
static bool IsMouseButtonReleased(int button) { return false; }

static bool IsKeyDown(int key) { return false; }
static bool IsKeyPressed(int key) { return false; }
static int GetKeyPressed(void) { return 0; }                     // -- GuiTextBox()
//-------------------------------------------------------------------------------

// Drawing required functions
//-------------------------------------------------------------------------------
static void DrawRectangle(int x, int y, int width, int height, Color color) { /* TODO */ }
static void DrawRectangleRec(Rectangle rec, Color color) { DrawRectangle(rec.x, rec.y, rec.width, rec.height, color); }

static void DrawRectangleLinesEx(Rectangle rec, int lineThick, Color color) { /* TODO */ }

static void DrawRectangleLines(int x, int y, int width, int height, Color color) { /* TODO */ }             // -- GuiColorPicker()
static void DrawRectangleGradientV(int posX, int posY, int width, int height, Color color1, Color color2);  // -- GuiColorPicker()
static void DrawRectangleGradientH(int posX, int posY, int width, int height, Color color1, Color color2);  // -- GuiColorPicker()
static void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4);         // -- GuiColorPicker()

static void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color) { /* TODO */ }                    // -- GuiDropdownBox()
static void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color) { /* TODO */ }           // -- GuiScrollBar()

static void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint) { }        // -- GuiImageButtonEx()
//-------------------------------------------------------------------------------

// Text required functions
//-------------------------------------------------------------------------------
static Font GetFontDefault(void);   // --  GetTextWidth()

static Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing) { return (Vector2){ 0.0f }; }  // Measure text size depending on font
static void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint) {  }   // Draw text using font and additional parameters
//-------------------------------------------------------------------------------

#endif      // RAYGUI_STANDALONE

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------

// List Element control, returns element state
static bool GuiListElement(Rectangle bounds, const char *text, bool active, bool editMode);

static Vector3 ConvertHSVtoRGB(Vector3 hsv);        // Convert color data from HSV to RGB
static Vector3 ConvertRGBtoHSV(Vector3 rgb);        // Convert color data from RGB to HSV

// Gui get text width using default font
static int GetTextWidth(const char *text)       // TODO: GetTextSize()
{
    Vector2 size = { 0 };

    if (guiFont.texture.id == 0) guiFont = GetFontDefault();

    if ((text != NULL) && (text[0] != '\0')) size = MeasureTextEx(guiFont, text, GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING));

    // TODO: Consider text icon width here???

    return (int)size.x;
}

// Get text bounds considering control bounds
static Rectangle GetTextBounds(int control, Rectangle bounds)
{
    Rectangle textBounds = { 0 };

    textBounds.x = bounds.x + GuiGetStyle(control, BORDER_WIDTH) + GuiGetStyle(control, INNER_PADDING);
    textBounds.y = bounds.y + GuiGetStyle(control, BORDER_WIDTH) + GuiGetStyle(control, INNER_PADDING);
    textBounds.width = bounds.width - 2*(GuiGetStyle(control, BORDER_WIDTH) + GuiGetStyle(control, INNER_PADDING));
    textBounds.height = bounds.height - 2*(GuiGetStyle(control, BORDER_WIDTH) + GuiGetStyle(control, INNER_PADDING));

    switch (control)
    {
        case COMBOBOX: bounds.width -= (GuiGetStyle(control, SELECTOR_WIDTH) + GuiGetStyle(control, SELECTOR_PADDING)); break;
        case CHECKBOX: bounds.x += (bounds.width + GuiGetStyle(control, CHECK_TEXT_PADDING)); break;
        default: break;
    }
    // TODO: Special cases: COMBOBOX, DROPDOWNBOX, SPINNER, LISTVIEW (scrollbar?)
    // More special cases: CHECKBOX, SLIDER

    return textBounds;
}

// Get text icon if provided and move text cursor
static const char *GetTextIcon(const char *text, int *iconId)
{
#if defined(RAYGUI_RICONS_SUPPORT)
    if (text[0] == '#')     // Maybe we have an icon!
    {
        char iconValue[4] = { 0 };

        int i = 1;
        for (i = 1; i < 4; i++)
        {
            if ((text[i] != '#') && (text[i] != '\0')) iconValue[i - 1] = text[i];
            else break;
        }

        iconValue[3] = '\0';
        *iconId = atoi(iconValue);

        // Move text pointer after icon
        // WARNING: If only icon provided, it could point to EOL character!
        if (*iconId > 0) text += (i + 1);
    }
#endif

    return text;
}

// Gui draw text using default font
static void GuiDrawText(const char *text, Rectangle bounds, int alignment, Color tint)
{
    if (guiFont.texture.id == 0) guiFont = GetFontDefault();

    if ((text != NULL) && (text[0] != '\0'))
    {
        int iconId = 0;
        text = GetTextIcon(text, &iconId);  // Check text for icon and move cursor

        // Get text position depending on alignment and iconId
        //---------------------------------------------------------------------------------
        #define ICON_TEXT_PADDING   4

        Vector2 position = { bounds.x, bounds.y };

        // NOTE: We get text size after icon been processed
        int textWidth = GetTextWidth(text);
        int textHeight = GuiGetStyle(DEFAULT, TEXT_SIZE);

#if defined(RAYGUI_RICONS_SUPPORT)
        if (iconId > 0)
        {
            textWidth += RICONS_SIZE;

            // WARNING: If only icon provided, text could be pointing to eof character!
            if ((text != NULL) && (text[0] != '\0')) textWidth += ICON_TEXT_PADDING;
        }
#endif

        // Check guiTextAlign global variables
        switch (alignment)
        {
            case GUI_TEXT_ALIGN_LEFT:
            {
                position.x = bounds.x;
                position.y = bounds.y + bounds.height/2 - textHeight/2 + VALIGN_OFFSET(bounds.height);
            } break;
            case GUI_TEXT_ALIGN_CENTER:
            {
                position.x = bounds.x + bounds.width/2 - textWidth/2;
                position.y = bounds.y + bounds.height/2 - textHeight/2 + VALIGN_OFFSET(bounds.height);
            } break;
            case GUI_TEXT_ALIGN_RIGHT:
            {
                position.x = bounds.x + bounds.width - textWidth;
                position.y = bounds.y + bounds.height/2 - textHeight/2 + VALIGN_OFFSET(bounds.height);
            } break;
            default: break;
        }
        //---------------------------------------------------------------------------------

        // Draw text (with icon if available)
        //---------------------------------------------------------------------------------
#if defined(RAYGUI_RICONS_SUPPORT)
        #define ICON_TEXT_PADDING   4

        if (iconId > 0)
        {
            // NOTE: We consider icon height, probably different than text size
            DrawIcon(iconId, (Vector2){ position.x, bounds.y + bounds.height/2 - RICONS_SIZE/2 + VALIGN_OFFSET(bounds.height) }, 1, tint);
            position.x += (RICONS_SIZE + ICON_TEXT_PADDING);
        }
#endif
        DrawTextEx(guiFont, text, position, GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), tint);
        //---------------------------------------------------------------------------------
    }
}

// Split controls text into multiple strings
// Also check for multiple columns (required by GuiToggleGroup())
static const char **GuiTextSplit(const char *text, int *count, int *textRow);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Enable gui global state
RAYGUIDEF void GuiEnable(void) { guiState = GUI_STATE_NORMAL; }

// Disable gui global state
RAYGUIDEF void GuiDisable(void) { guiState = GUI_STATE_DISABLED; }

// Lock gui global state
RAYGUIDEF void GuiLock(void) { guiLocked = true; }

// Unlock gui global state
RAYGUIDEF void GuiUnlock(void) { guiLocked = false; }

// Set gui state (global state)
RAYGUIDEF void GuiState(int state) { guiState = (GuiControlState)state; }

// Define custom gui font
RAYGUIDEF void GuiFont(Font font)
{
    if (font.texture.id > 0)
    {
        guiFont = font;
        GuiSetStyle(DEFAULT, TEXT_SIZE, font.baseSize);

        // Populate all controls with new font size
        for (int i = 1; i < NUM_CONTROLS; i++) GuiSetStyle(i, TEXT_SIZE, GuiGetStyle(DEFAULT, TEXT_SIZE));

        // NOTE: Loaded font spacing must be set manually
        //GuiSetStyle(DEFAULT, TEXT_SPACING, 1);
    }
}

// Set gui controls alpha global state
RAYGUIDEF void GuiFade(float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    else if (alpha > 1.0f) alpha = 1.0f;

    guiAlpha = alpha;
}

// Set control style property value
RAYGUIDEF void GuiSetStyle(int control, int property, int value)
{
    if (!guiStyleLoaded) GuiLoadStyleDefault();
    guiStyle[control*(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED) + property] = value;
}

// Get control style property value
RAYGUIDEF int GuiGetStyle(int control, int property)
{
    if (!guiStyleLoaded) GuiLoadStyleDefault();
    return guiStyle[control*(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED) + property];
}

// Window Box control
RAYGUIDEF bool GuiWindowBox(Rectangle bounds, const char *text)
{
    #define WINDOW_CLOSE_BUTTON_PADDING    2
    #define WINDOW_STATUSBAR_HEIGHT        24

    GuiControlState state = guiState;
    bool clicked = false;

    Rectangle statusBar = { bounds.x, bounds.y, bounds.width, WINDOW_STATUSBAR_HEIGHT };
    if (bounds.height < WINDOW_STATUSBAR_HEIGHT*2) bounds.height = WINDOW_STATUSBAR_HEIGHT*2;

    Rectangle buttonRec = { statusBar.x + statusBar.width - GuiGetStyle(DEFAULT, BORDER_WIDTH) - WINDOW_CLOSE_BUTTON_PADDING - 20,
                            statusBar.y + GuiGetStyle(DEFAULT, BORDER_WIDTH) + WINDOW_CLOSE_BUTTON_PADDING, 18, 18 };
    // Update control
    //--------------------------------------------------------------------
    // NOTE: Logic is directly managed by button
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------

    // Draw window base
    DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DEFAULT, BORDER + (state*3))), guiAlpha));
    DrawRectangleRec((Rectangle){ bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH),
                                  bounds.width - GuiGetStyle(DEFAULT, BORDER_WIDTH)*2, bounds.height - GuiGetStyle(DEFAULT, BORDER_WIDTH)*2 },
                                  Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), guiAlpha));

    // Draw window header as status bar
    int defaultPadding = GuiGetStyle(DEFAULT, INNER_PADDING);
    int defaultTextAlign = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT);
    GuiSetStyle(DEFAULT, INNER_PADDING, 8);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
    GuiStatusBar(statusBar, text);
    GuiSetStyle(DEFAULT, INNER_PADDING, defaultPadding);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, defaultTextAlign);

    // Draw window close button
    int buttonBorder = GuiGetStyle(BUTTON, BORDER_WIDTH);
    int buttonTextAlignment = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
#if defined(RAYGUI_RICONS_SUPPORT)
    clicked = GuiButton(buttonRec, GuiIconText(RICON_CROSS_SMALL, NULL));
#else
    clicked = GuiButton(buttonRec, "x");
#endif
    GuiSetStyle(BUTTON, BORDER_WIDTH, buttonBorder);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, buttonTextAlignment);
    //--------------------------------------------------------------------

    return clicked;
}

// Group Box control with title name
RAYGUIDEF void GuiGroupBox(Rectangle bounds, const char *text)
{
    #define GROUPBOX_LINE_THICK     1
    #define GROUPBOX_TEXT_PADDING  10
    #define GROUPBOX_PADDING        2

    GuiControlState state = guiState;

    // Draw control
    //--------------------------------------------------------------------
    DrawRectangle(bounds.x, bounds.y, GROUPBOX_LINE_THICK, bounds.height, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));
    DrawRectangle(bounds.x, bounds.y + bounds.height - 1, bounds.width, GROUPBOX_LINE_THICK, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));
    DrawRectangle(bounds.x + bounds.width - 1, bounds.y, GROUPBOX_LINE_THICK, bounds.height, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));

    GuiLine((Rectangle){ bounds.x, bounds.y, bounds.width, 1 }, text);
    //--------------------------------------------------------------------
}

// Line control
RAYGUIDEF void GuiLine(Rectangle bounds, const char *text)
{
    #define LINE_THICK          1
    #define LINE_TEXT_PADDING  10
    #define LINE_TEXT_SPACING   2

    GuiControlState state = guiState;

    Color color = Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha);

    // Draw control
    //--------------------------------------------------------------------
    if (text == NULL) DrawRectangle(bounds.x, bounds.y + bounds.height/2, bounds.width, 1, color);
    else
    {
        Rectangle textBounds = { 0 };
        textBounds.width = GetTextWidth(text) + 2*LINE_TEXT_SPACING;      // TODO: Consider text icon
        textBounds.height = GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + LINE_TEXT_PADDING + LINE_TEXT_SPACING;
        textBounds.y = bounds.y - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

        // Draw line with embedded text label: "--- text --------------"
        DrawRectangle(bounds.x, bounds.y, LINE_TEXT_PADDING, 1, color);
        GuiLabel(textBounds, text);
        DrawRectangle(bounds.x + textBounds.width + LINE_TEXT_PADDING + 2*LINE_TEXT_SPACING, bounds.y, bounds.width - (textBounds.width + LINE_TEXT_PADDING + 2*LINE_TEXT_SPACING), 1, color);
    }
    //--------------------------------------------------------------------
}

// Panel control
RAYGUIDEF void GuiPanel(Rectangle bounds)
{
    #define PANEL_BORDER_WIDTH   1

    GuiControlState state = guiState;

    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleRec(bounds, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BASE_COLOR_DISABLED : BACKGROUND_COLOR)), guiAlpha));
    DrawRectangleLinesEx(bounds, PANEL_BORDER_WIDTH, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED: LINE_COLOR)), guiAlpha));
    //--------------------------------------------------------------------
}

// Scroll Panel control
RAYGUIDEF Rectangle GuiScrollPanel(Rectangle bounds, Rectangle content, Vector2 *scroll)
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
    const Rectangle horizontalScrollBar = { ((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? bounds.x + verticalScrollBarWidth : bounds.x) + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + bounds.height - horizontalScrollBarWidth - GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.width - verticalScrollBarWidth - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH), horizontalScrollBarWidth };
    const Rectangle verticalScrollBar = { ((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH) : bounds.x + bounds.width - verticalScrollBarWidth - GuiGetStyle(DEFAULT, BORDER_WIDTH)), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), verticalScrollBarWidth, bounds.height - horizontalScrollBarWidth - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) };

    // Calculate view area (area without the scrollbars)
    Rectangle view = (GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)?
                (Rectangle){ bounds.x + verticalScrollBarWidth + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth, bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth } :
                (Rectangle){ bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth, bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth };

    // Clip view area to the actual content size
    if (view.width > content.width) view.width = content.width;
    if (view.height > content.height) view.height = content.height;

    // TODO: Review!
    const int horizontalMin = hasHorizontalScrollBar? ((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? -verticalScrollBarWidth : 0) - GuiGetStyle(DEFAULT, BORDER_WIDTH) : ((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? -verticalScrollBarWidth : 0) - GuiGetStyle(DEFAULT, BORDER_WIDTH);
    const int horizontalMax = hasHorizontalScrollBar? content.width - bounds.width + verticalScrollBarWidth + GuiGetStyle(DEFAULT, BORDER_WIDTH) - ((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? verticalScrollBarWidth : 0) : -GuiGetStyle(DEFAULT, BORDER_WIDTH);
    const int verticalMin = hasVerticalScrollBar? -GuiGetStyle(DEFAULT, BORDER_WIDTH) : -GuiGetStyle(DEFAULT, BORDER_WIDTH);
    const int verticalMax = hasVerticalScrollBar? content.height - bounds.height + horizontalScrollBarWidth + GuiGetStyle(DEFAULT, BORDER_WIDTH) : -GuiGetStyle(DEFAULT, BORDER_WIDTH);

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
            
            scrollPos.y += GetMouseWheelMove()*20;
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
    DrawRectangleRec(bounds, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));        // Draw background

    // Save size of the scrollbar slider
    const int slider = GuiGetStyle(SCROLLBAR, SLIDER_SIZE);

    // Draw horizontal scrollbar if visible
    if (hasHorizontalScrollBar)
    {
        // Change scrollbar slider size to show the diff in size between the content width and the widget width
        GuiSetStyle(SCROLLBAR, SLIDER_SIZE, ((bounds.width - 2 * GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth)/content.width)*(bounds.width - 2 * GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth));
        scrollPos.x = -GuiScrollBar(horizontalScrollBar, -scrollPos.x, horizontalMin, horizontalMax);
    }

    // Draw vertical scrollbar if visible
    if (hasVerticalScrollBar)
    {
        // Change scrollbar slider size to show the diff in size between the content height and the widget height
        GuiSetStyle(SCROLLBAR, SLIDER_SIZE, ((bounds.height - 2 * GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth)/content.height)* (bounds.height - 2 * GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth));
        scrollPos.y = -GuiScrollBar(verticalScrollBar, -scrollPos.y, verticalMin, verticalMax);
    }
    
    // Draw detail corner rectangle if both scroll bars are visible
    if (hasHorizontalScrollBar && hasVerticalScrollBar) 
    {
        // TODO: Consider scroll bars side
        DrawRectangle(horizontalScrollBar.x + horizontalScrollBar.width + 2, 
                      verticalScrollBar.y + verticalScrollBar.height + 2, 
                      horizontalScrollBarWidth - 4, verticalScrollBarWidth - 4, 
                      Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT + (state*3))), guiAlpha));
    }

    // Set scrollbar slider size back to the way it was before
    GuiSetStyle(SCROLLBAR, SLIDER_SIZE, slider);

    // Draw scrollbar lines depending on current state
    DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    if (scroll != NULL) *scroll = scrollPos;

    return view;
}

// Label control
RAYGUIDEF void GuiLabel(Rectangle bounds, const char *text)
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
RAYGUIDEF bool GuiButton(Rectangle bounds, const char *text)
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
    DrawRectangleLinesEx(bounds, GuiGetStyle(BUTTON, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(BUTTON, BORDER + (state*3))), guiAlpha));
    DrawRectangle(bounds.x + GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.y + GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(BUTTON, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(BUTTON, BASE + (state*3))), guiAlpha));

    GuiDrawText(text, GetTextBounds(BUTTON, bounds), GuiGetStyle(BUTTON, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(BUTTON, TEXT + (state*3))), guiAlpha));
    //------------------------------------------------------------------

    return pressed;
}

// Label button control
RAYGUIDEF bool GuiLabelButton(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;
    bool pressed = false;

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

// Image button control, returns true when clicked
RAYGUIDEF bool GuiImageButton(Rectangle bounds, Texture2D texture)
{
    return GuiImageButtonEx(bounds, texture, (Rectangle){ 0, 0, texture.width, texture.height }, NULL);
}

// Image button control, returns true when clicked
RAYGUIDEF bool GuiImageButtonEx(Rectangle bounds, Texture2D texture, Rectangle texSource, const char *text)
{
    GuiControlState state = guiState;
    bool clicked = false;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) clicked = true;
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleLinesEx(bounds, GuiGetStyle(BUTTON, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(BUTTON, BORDER + (state*3))), guiAlpha));
    DrawRectangle(bounds.x + GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.y + GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(BUTTON, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(BUTTON, BASE + (state*3))), guiAlpha));

    if (text != NULL) GuiDrawText(text, GetTextBounds(BUTTON, bounds), GuiGetStyle(BUTTON, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(BUTTON, TEXT + (state*3))), guiAlpha));
    if (texture.id > 0) DrawTextureRec(texture, texSource, (Vector2){ bounds.x + bounds.width/2 - (texSource.width + GuiGetStyle(BUTTON, INNER_PADDING)/2)/2, bounds.y + bounds.height/2 - texSource.height/2 }, Fade(GetColor(GuiGetStyle(BUTTON, TEXT + (state*3))), guiAlpha));
    //------------------------------------------------------------------

    return clicked;
}

// Toggle Button control, returns true when active
RAYGUIDEF bool GuiToggle(Rectangle bounds, const char *text, bool active)
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
        DrawRectangleLinesEx(bounds, GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, (active? BORDER_COLOR_PRESSED : (BORDER + state*3)))), guiAlpha));
        DrawRectangle(bounds.x + GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.y + GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, (active? BASE_COLOR_PRESSED : (BASE + state*3)))), guiAlpha));

        GuiDrawText(text, GetTextBounds(TOGGLE, bounds), GuiGetStyle(TOGGLE, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TOGGLE, (active? TEXT_COLOR_PRESSED : (TEXT + state*3)))), guiAlpha));
    }
    else
    {
        DrawRectangleLinesEx(bounds, GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, BORDER + state*3)), guiAlpha));
        DrawRectangle(bounds.x + GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.y + GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, BASE + state*3)), guiAlpha));

        GuiDrawText(text, GetTextBounds(TOGGLE, bounds), GuiGetStyle(TOGGLE, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TOGGLE, TEXT + state*3)), guiAlpha));
    }
    //--------------------------------------------------------------------

    return active;
}

// Toggle Group control, returns toggled button index
RAYGUIDEF int GuiToggleGroup(Rectangle bounds, const char *text, int active)
{
    float initBoundsX = bounds.x;

    // Get substrings elements from text (elements pointers)
    int rows[64] = { 0 };
    int elementsCount = 0;
    const char **elementsPtrs = GuiTextSplit(text, &elementsCount, rows);

    int prevRow = rows[0];

    for (int i = 0; i < elementsCount; i++)
    {
        if (prevRow != rows[i])
        {
            bounds.x = initBoundsX;
            bounds.y += (bounds.height + GuiGetStyle(TOGGLE, GROUP_PADDING));
            prevRow = rows[i];
        }

        if (i == active) GuiToggle(bounds, elementsPtrs[i], true);
        else if (GuiToggle(bounds, elementsPtrs[i], false) == true) active = i;

        bounds.x += (bounds.width + GuiGetStyle(TOGGLE, GROUP_PADDING));
    }

    return active;
}

// Check Box control, returns true when active
RAYGUIDEF bool GuiCheckBox(Rectangle bounds, const char *text, bool checked)
{
    GuiControlState state = guiState;

    Rectangle textBounds = { 0 };
    textBounds.x = bounds.x + bounds.width + GuiGetStyle(CHECKBOX, CHECK_TEXT_PADDING);
    textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;
    textBounds.width = GetTextWidth(text);      // TODO: Consider text icon
    textBounds.height = GuiGetStyle(DEFAULT, TEXT_SIZE);

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check checkbox state
        if (CheckCollisionPointRec(mousePoint, (Rectangle){ bounds.x, bounds.y, bounds.width + textBounds.width + GuiGetStyle(CHECKBOX, CHECK_TEXT_PADDING), bounds.height }))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) checked = !checked;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleLinesEx(bounds, GuiGetStyle(CHECKBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(CHECKBOX, BORDER + (state*3))), guiAlpha));
    if (checked) DrawRectangle(bounds.x + GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, INNER_PADDING),
                               bounds.y + GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, INNER_PADDING),
                               bounds.width - 2*(GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, INNER_PADDING)),
                               bounds.height - 2*(GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, INNER_PADDING)),
                               Fade(GetColor(GuiGetStyle(CHECKBOX, TEXT + state*3)), guiAlpha));

    // NOTE: Forced left text alignment
    GuiDrawText(text, textBounds, GUI_TEXT_ALIGN_LEFT, Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return checked;
}

// Combo Box control, returns selected item index
RAYGUIDEF int GuiComboBox(Rectangle bounds, const char *text, int active)
{
    GuiControlState state = guiState;

    bounds.width -= (GuiGetStyle(COMBOBOX, SELECTOR_WIDTH) + GuiGetStyle(COMBOBOX, SELECTOR_PADDING));

    Rectangle selector = { bounds.x + bounds.width + GuiGetStyle(COMBOBOX, SELECTOR_PADDING),
                           bounds.y, GuiGetStyle(COMBOBOX, SELECTOR_WIDTH), bounds.height };

    // Get substrings elements from text (elements pointers, lengths and count)
    int elementsCount = 0;
    const char **elementsPtrs = GuiTextSplit(text, &elementsCount, NULL);

    if (active < 0) active = 0;
    else if (active > elementsCount - 1) active = elementsCount - 1;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds) ||
            CheckCollisionPointRec(mousePoint, selector))
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                active += 1;
                if (active >= elementsCount) active = 0;
            }

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    // Draw combo box main
    DrawRectangleLinesEx(bounds, GuiGetStyle(COMBOBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COMBOBOX, BORDER + (state*3))), guiAlpha));
    DrawRectangle(bounds.x + GuiGetStyle(COMBOBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(COMBOBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(COMBOBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(COMBOBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COMBOBOX, BASE + (state*3))), guiAlpha));

    GuiDrawText(elementsPtrs[active], GetTextBounds(COMBOBOX, bounds), GuiGetStyle(COMBOBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(COMBOBOX, TEXT + (state*3))), guiAlpha));

    // Draw selector using a custom button
    // NOTE: BORDER_WIDTH and TEXT_ALIGNMENT forced values
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    int tempTextAlign = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

    GuiButton(selector, TextFormat("%i/%i", active + 1, elementsCount));

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlign);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
    //--------------------------------------------------------------------

    return active;
}

// Dropdown Box control, returns selected item
RAYGUIDEF bool GuiDropdownBox(Rectangle bounds, const char *text, int *active, bool editMode)
{
    GuiControlState state = guiState;

    // Get substrings elements from text (elements pointers, lengths and count)
    int elementsCount = 0;
    const char **elementsPtrs = GuiTextSplit(text, &elementsCount, NULL);

    bool pressed = false;
    int auxActive = *active;

    Rectangle closeBounds = bounds;
    Rectangle openBounds = bounds;

    openBounds.height *= (elementsCount + 1);

    // Update control
    //--------------------------------------------------------------------
    if (guiLocked && editMode) guiLocked = false;

    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (editMode) state = GUI_STATE_PRESSED;

        if (!editMode)
        {
            if (CheckCollisionPointRec(mousePoint, closeBounds))
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
                else state = GUI_STATE_FOCUSED;
            }
        }
        else
        {
            if (CheckCollisionPointRec(mousePoint, closeBounds))
            {
                 if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
            }
            else if (!CheckCollisionPointRec(mousePoint, openBounds))
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------

    // TODO: Review this ugly hack... DROPDOWNBOX depends on GiListElement() that uses DEFAULT_TEXT_ALIGNMENT
    int tempTextAlign = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT));

    switch (state)
    {
        case GUI_STATE_NORMAL:
        {
            DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_NORMAL)), guiAlpha));
            DrawRectangleLinesEx(bounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_NORMAL)), guiAlpha));

            GuiListElement((Rectangle){ bounds.x, bounds.y, bounds.width, bounds.height }, elementsPtrs[auxActive], false, false);
        } break;
        case GUI_STATE_FOCUSED:
        {
            GuiListElement((Rectangle){ bounds.x, bounds.y, bounds.width, bounds.height }, elementsPtrs[auxActive], false, editMode);
        } break;
        case GUI_STATE_PRESSED:
        {
            if (!editMode) GuiListElement((Rectangle){ bounds.x, bounds.y, bounds.width, bounds.height }, elementsPtrs[auxActive], true, true);
            if (editMode)
            {
                GuiPanel(openBounds);

                GuiListElement((Rectangle){ bounds.x, bounds.y, bounds.width, bounds.height }, elementsPtrs[auxActive], true, true);

                for (int i = 0; i < elementsCount; i++)
                {
                    if (i == auxActive && editMode)
                    {
                        if (GuiListElement((Rectangle){ bounds.x, bounds.y + bounds.height*(i + 1) + GuiGetStyle(DROPDOWNBOX, INNER_PADDING), 
                                                        bounds.width, bounds.height - GuiGetStyle(DROPDOWNBOX, INNER_PADDING) }, 
                                                        elementsPtrs[i], true, true) == false) pressed = true;
                    }
                    else
                    {
                        if (GuiListElement((Rectangle){ bounds.x, bounds.y + bounds.height*(i+1) + GuiGetStyle(DROPDOWNBOX, INNER_PADDING), 
                                                        bounds.width, bounds.height - GuiGetStyle(DROPDOWNBOX, INNER_PADDING) }, 
                                                        elementsPtrs[i], false, true))
                        {
                            auxActive = i;
                            pressed = true;
                        }
                    }
                }
            }
        } break;
        case GUI_STATE_DISABLED:
        {
            DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_DISABLED)), guiAlpha));
            DrawRectangleLinesEx(bounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_DISABLED)), guiAlpha));

            GuiListElement((Rectangle){ bounds.x, bounds.y, bounds.width, bounds.height }, elementsPtrs[auxActive], false, false);
        } break;
        default: break;
    }

    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, tempTextAlign);

    DrawTriangle((Vector2){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_RIGHT_PADDING), bounds.y + bounds.height/2 - 2 },
                 (Vector2){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_RIGHT_PADDING) + 5, bounds.y + bounds.height/2 - 2 + 5 },
                 (Vector2){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_RIGHT_PADDING) + 10, bounds.y + bounds.height/2 - 2 }, 
                 Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    *active = auxActive;
    return pressed;
}

// Spinner control, returns selected value
// NOTE: Requires static variables: framesCounter, valueSpeed - ERROR!
RAYGUIDEF bool GuiSpinner(Rectangle bounds, int *value, int minValue, int maxValue, bool editMode)
{
    bool pressed = false;
    int tempValue = *value;

    Rectangle spinner = { bounds.x + GuiGetStyle(TEXTBOX, SPINNER_BUTTON_WIDTH) + GuiGetStyle(TEXTBOX, SPINNER_BUTTON_PADDING), bounds.y,
                          bounds.width - 2*(GuiGetStyle(TEXTBOX, SPINNER_BUTTON_WIDTH) + GuiGetStyle(TEXTBOX, SPINNER_BUTTON_PADDING)), bounds.height };
    Rectangle leftButtonBound = { bounds.x, bounds.y, GuiGetStyle(TEXTBOX, SPINNER_BUTTON_WIDTH), bounds.height };
    Rectangle rightButtonBound = { bounds.x + bounds.width - GuiGetStyle(TEXTBOX, SPINNER_BUTTON_WIDTH), bounds.y, GuiGetStyle(TEXTBOX, SPINNER_BUTTON_WIDTH), bounds.height };

    // Update control
    //--------------------------------------------------------------------
    if (!editMode)
    {
        if (tempValue < minValue) tempValue = minValue;
        if (tempValue > maxValue) tempValue = maxValue;
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    pressed = GuiValueBox(spinner, &tempValue, minValue, maxValue, editMode);

    // Draw value selector custom buttons
    // NOTE: BORDER_WIDTH and TEXT_ALIGNMENT forced values
    int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
    GuiSetStyle(BUTTON, BORDER_WIDTH, GuiGetStyle(TEXTBOX, SPINNER_BUTTON_BORDER_WIDTH));

    int tempTextAlign = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

#if defined(RAYGUI_RICONS_SUPPORT)
    if (GuiButton(leftButtonBound, GuiIconText(RICON_ARROW_LEFT_FILL, NULL))) tempValue--;
    if (GuiButton(rightButtonBound, GuiIconText(RICON_ARROW_RIGHT_FILL, NULL))) tempValue++;
#else
    if (GuiButton(leftButtonBound, "<")) tempValue--;
    if (GuiButton(rightButtonBound, ">")) tempValue++;
#endif

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlign);
    GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);
    //--------------------------------------------------------------------

    *value = tempValue;
    return pressed;
}

// Value Box control, updates input text with numbers
// NOTE: Requires static variables: framesCounter
RAYGUIDEF bool GuiValueBox(Rectangle bounds, int *value, int minValue, int maxValue, bool editMode)
{
    #define VALUEBOX_MAX_CHARS          32

    static int framesCounter = 0;           // Required for blinking cursor

    GuiControlState state = guiState;
    bool pressed = false;

    char text[VALUEBOX_MAX_CHARS + 1] = "\0";
    sprintf(text, "%i", *value);

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        bool valueHasChanged = false;

        if (editMode)
        {
            state = GUI_STATE_PRESSED;

            framesCounter++;

            int keyCount = strlen(text);

            // Only allow keys in range [48..57]
            if (keyCount < VALUEBOX_MAX_CHARS)
            {
                int maxWidth = (bounds.width - (GuiGetStyle(DEFAULT, INNER_PADDING)*2));
                if (GetTextWidth(text) < maxWidth)
                {
                    int key = GetKeyPressed();
                    if ((key >= 48) && (key <= 57))
                    {
                        text[keyCount] = (char)key;
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
                    text[keyCount] = '\0';
                    framesCounter = 0;
                    if (keyCount < 0) keyCount = 0;
                    valueHasChanged = true;
                }
                else if (IsKeyDown(KEY_BACKSPACE))
                {
                    if ((framesCounter > TEXTEDIT_CURSOR_BLINK_FRAMES) && (framesCounter%2) == 0) keyCount--;
                    text[keyCount] = '\0';
                    if (keyCount < 0) keyCount = 0;
                    valueHasChanged = true;
                }
            }

            if (valueHasChanged) *value = atoi(text);
        }
        else
        {
            if (*value > maxValue) *value = maxValue;
            else if (*value < minValue) *value = minValue;
        }

        if (!editMode)
        {
            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = GUI_STATE_FOCUSED;
                if (IsMouseButtonPressed(0)) pressed = true;
            }
        }
        else
        {
            if (IsKeyPressed(KEY_ENTER) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(0))) pressed = true;
        }

        if (pressed) framesCounter = 0;
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleLinesEx(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha));

    if (state == GUI_STATE_PRESSED)
    {
        DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_FOCUSED)), guiAlpha));
        if (editMode && ((framesCounter/20)%2 == 0)) DrawRectangle(bounds.x + GetTextWidth(text)/2 + bounds.width/2 + 2, bounds.y + GuiGetStyle(TEXTBOX, INNER_PADDING), 1, bounds.height - GuiGetStyle(TEXTBOX, INNER_PADDING)*2, Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER_COLOR_FOCUSED)), guiAlpha));
    }
    else if (state == GUI_STATE_DISABLED)
    {
        DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_DISABLED)), guiAlpha));
    }

    GuiDrawText(text, GetTextBounds(TEXTBOX, bounds), GuiGetStyle(TEXTBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TEXTBOX, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return pressed;
}

// Text Box control, updates input text
// NOTE 1: Requires static variables: framesCounter
// NOTE 2: Returns if KEY_ENTER pressed (useful for data validation)
RAYGUIDEF bool GuiTextBox(Rectangle bounds, char *text, int textSize, bool editMode)
{
    static int framesCounter = 0;           // Required for blinking cursor

    GuiControlState state = guiState;
    bool pressed = false;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (editMode)
        {
            state = GUI_STATE_PRESSED;
            framesCounter++;

            int key = GetKeyPressed();
            int keyCount = strlen(text);

            // Only allow keys in range [32..125]
            if (keyCount < (textSize - 1))
            {
                int maxWidth = (bounds.width - (GuiGetStyle(DEFAULT, INNER_PADDING)*2));

                if (GetTextWidth(text) < (maxWidth - GuiGetStyle(DEFAULT, TEXT_SIZE)))
                {
                    if (((key >= 32) && (key <= 125)) ||
                        ((key >= 128) && (key < 255)))
                    {
                        text[keyCount] = (char)key;
                        keyCount++;
                    }
                }
            }

            // Delete text
            if (keyCount > 0)
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    keyCount--;
                    text[keyCount] = '\0';
                    framesCounter = 0;
                    if (keyCount < 0) keyCount = 0;
                }
                else if (IsKeyDown(KEY_BACKSPACE))
                {
                    if ((framesCounter > TEXTEDIT_CURSOR_BLINK_FRAMES) && (framesCounter%2) == 0) keyCount--;
                    text[keyCount] = '\0';
                    if (keyCount < 0) keyCount = 0;
                }
            }
        }

        if (!editMode)
        {
            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = GUI_STATE_FOCUSED;
                if (IsMouseButtonPressed(0)) pressed = true;
            }
        }
        else
        {
            if (IsKeyPressed(KEY_ENTER) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(0))) pressed = true;
        }

        if (pressed) framesCounter = 0;
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleLinesEx(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha));

    if (state == GUI_STATE_PRESSED)
    {
        DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_FOCUSED)), guiAlpha));
        
        // Draw blinking cursor
        // TODO: Consider TEXTBOX TEXT_ALIGNMENT
        if (editMode && ((framesCounter/20)%2 == 0)) DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, INNER_PADDING) + GetTextWidth(text) + 2, bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE), 1, GuiGetStyle(DEFAULT, TEXT_SIZE)*2, Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER_COLOR_PRESSED)), guiAlpha));
    }
    else if (state == GUI_STATE_DISABLED)
    {
        DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_DISABLED)), guiAlpha));
    }

    GuiDrawText(text, GetTextBounds(TEXTBOX, bounds), GuiGetStyle(TEXTBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TEXTBOX, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return pressed;
}

// Text Box control with multiple lines
RAYGUIDEF bool GuiTextBoxMulti(Rectangle bounds, char *text, int textSize, bool editMode)
{
    static int framesCounter = 0;           // Required for blinking cursor

    GuiControlState state = guiState;
    bool pressed = false;

    bool textHasChange = false;
    int currentLine = 0;
    //const char *numChars = NULL;

    // Security check because font is used directly in this control
    if (guiFont.texture.id == 0) guiFont = GetFontDefault();

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (editMode)
        {
            state = GUI_STATE_PRESSED;

            framesCounter++;

            int keyCount = strlen(text);
            int maxWidth = (bounds.width - (GuiGetStyle(TEXTBOX, INNER_PADDING)*2));
            int maxHeight = (bounds.height - (GuiGetStyle(TEXTBOX, INNER_PADDING)*2));

            //numChars = TextFormat("%i/%i", keyCount, textSize - 1);

            // Only allow keys in range [32..125]
            if (keyCount < (textSize - 1))
            {
                int key = GetKeyPressed();

                if (MeasureTextEx(guiFont, text, GuiGetStyle(DEFAULT, TEXT_SIZE), 1).y < (maxHeight - GuiGetStyle(DEFAULT, TEXT_SIZE)))
                {
                    if (IsKeyPressed(KEY_ENTER))
                    {
                        text[keyCount] = '\n';
                        keyCount++;
                    }
                    else if (((key >= 32) && (key <= 125)) ||
                        ((key >= 128) && (key < 255)))
                    {
                        text[keyCount] = (char)key;
                        keyCount++;
                        textHasChange = true;
                    }
                }
                else if (GetTextWidth(strrchr(text, '\n')) < (maxWidth - GuiGetStyle(DEFAULT, TEXT_SIZE)))
                {
                    if (((key >= 32) && (key <= 125)) ||
                        ((key >= 128) && (key < 255)))
                    {
                        text[keyCount] = (char)key;
                        keyCount++;
                        textHasChange = true;
                    }
                }
            }

            // Delete text
            if (keyCount > 0)
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    keyCount--;
                    text[keyCount] = '\0';
                    framesCounter = 0;
                    if (keyCount < 0) keyCount = 0;
                    textHasChange = true;
                }
                else if (IsKeyDown(KEY_BACKSPACE))
                {
                    if ((framesCounter > TEXTEDIT_CURSOR_BLINK_FRAMES) && (framesCounter%2) == 0) keyCount--;
                    text[keyCount] = '\0';
                    if (keyCount < 0) keyCount = 0;
                    textHasChange = true;
                }
            }

            // Introduce automatic new line if necessary
            if (textHasChange)
            {
                textHasChange = false;

                char *lastLine = strrchr(text, '\n');
                int maxWidth = (bounds.width - (GuiGetStyle(TEXTBOX, INNER_PADDING)*2));

                if (lastLine != NULL)
                {
                    if (GetTextWidth(lastLine) > maxWidth)
                    {
                        int firstIndex = lastLine - text;

                        char *lastSpace = strrchr(lastLine, 32);

                        if (lastSpace != NULL)
                        {
                            int secondIndex = lastSpace - lastLine;
                            text[firstIndex + secondIndex] = '\n';
                        }
                        else
                        {
                            int len = (lastLine != NULL)? strlen(lastLine) : 0;
                            char lastChar = lastLine[len - 1];
                            lastLine[len - 1] = '\n';
                            lastLine[len] = lastChar;
                            lastLine[len + 1] = '\0';
                            keyCount++;
                        }
                    }
                }
                else
                {
                    if (GetTextWidth(text) > maxWidth)
                    {
                        char *lastSpace = strrchr(text, 32);

                        if (lastSpace != NULL)
                        {
                            int index = lastSpace - text;
                            text[index] = '\n';
                        }
                        else
                        {
                            int len = (lastLine != NULL)? strlen(lastLine) : 0;
                            char lastChar = lastLine[len - 1];
                            lastLine[len - 1] = '\n';
                            lastLine[len] = lastChar;
                            lastLine[len + 1] = '\0';
                            keyCount++;
                        }
                    }
                }
            }

            // Counting how many new lines
            for (int i = 0; i < keyCount; i++)
            {
                if (text[i] == '\n') currentLine++;
            }
        }

        // Changing edit mode
        if (!editMode)
        {
            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = GUI_STATE_FOCUSED;
                if (IsMouseButtonPressed(0)) pressed = true;
            }
        }
        else
        {
            if (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(0)) pressed = true;
        }

        if (pressed) framesCounter = 0;
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleLinesEx(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha));

    if (state == GUI_STATE_PRESSED)
    {
        DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_FOCUSED)), guiAlpha));

        if (editMode)
        {
            if ((framesCounter/20)%2 == 0)
            {
                char *line = NULL;
                if (currentLine > 0) line = strrchr(text, '\n');
                else line = text;

                // Draw text cursor
                DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, INNER_PADDING) + GetTextWidth(line),
                              bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, INNER_PADDING)/2 + ((GuiGetStyle(DEFAULT, TEXT_SIZE) + GuiGetStyle(TEXTBOX, INNER_PADDING))*currentLine),
                              1, GuiGetStyle(DEFAULT, TEXT_SIZE) + GuiGetStyle(TEXTBOX, INNER_PADDING), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER_COLOR_FOCUSED)), guiAlpha));
            }

            // Draw characters counter
            //GuiDrawText(numChars, (Vector2){ bounds.x + bounds.width - GetTextWidth(numChars) - GuiGetStyle(TEXTBOX, INNER_PADDING), bounds.y + bounds.height - GuiGetStyle(DEFAULT, TEXT_SIZE) - GuiGetStyle(TEXTBOX, INNER_PADDING) }, Fade(GetColor(GuiGetStyle(TEXTBOX, TEXT_COLOR_PRESSED)), guiAlpha/2));
        }
    }
    else if (state == GUI_STATE_DISABLED)
    {
        DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_DISABLED)), guiAlpha));
    }

    GuiDrawText(text, GetTextBounds(TEXTBOX, bounds), GuiGetStyle(TEXTBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TEXTBOX, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return pressed;
}

// Slider control with pro parameters
// NOTE: Other GuiSlider*() controls use this one
RAYGUIDEF float GuiSliderPro(Rectangle bounds, const char *text, float value, float minValue, float maxValue, int sliderWidth, bool showValue)
{
    GuiControlState state = guiState;

    int sliderValue = (int)(((value - minValue)/(maxValue - minValue))*(bounds.width - 2*GuiGetStyle(SLIDER, BORDER_WIDTH)));

    Rectangle slider = { bounds.x, bounds.y + GuiGetStyle(SLIDER, BORDER_WIDTH) + GuiGetStyle(SLIDER, INNER_PADDING),
                         0, bounds.height - 2*GuiGetStyle(SLIDER, BORDER_WIDTH) - 2*GuiGetStyle(SLIDER, INNER_PADDING) };

    if (sliderWidth > 0)        // Slider
    {
        slider.x += (sliderValue - sliderWidth/2);
        slider.width = sliderWidth;
    }
    else if (sliderWidth == 0)  // SliderBar
    {
        slider.x += GuiGetStyle(SLIDER, BORDER_WIDTH);
        slider.width = sliderValue;
    }

    Rectangle textBounds = { 0 };
    textBounds.width = GetTextWidth(text);  // TODO: Consider text icon
    textBounds.height = GuiGetStyle(DEFAULT, TEXT_SIZE);
    textBounds.x = bounds.x - textBounds.width - GuiGetStyle(SLIDER, TEXT_PADDING);
    textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

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
                else if (sliderWidth == 0) slider.width = sliderValue;          // SliderBar
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
    DrawRectangleLinesEx(bounds, GuiGetStyle(SLIDER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(SLIDER, BORDER + (state*3))), guiAlpha));
    DrawRectangle(bounds.x + GuiGetStyle(SLIDER, BORDER_WIDTH), bounds.y + GuiGetStyle(SLIDER, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(SLIDER, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(SLIDER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(SLIDER, (state != GUI_STATE_DISABLED)?  BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));
    DrawRectangleRec(slider, Fade(GetColor(GuiGetStyle(SLIDER, (state == GUI_STATE_NORMAL)? BASE_COLOR_PRESSED : (BASE + (state*3)))), guiAlpha));

    GuiDrawText(text, textBounds, GuiGetStyle(SLIDER, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(SLIDER, TEXT + (state*3))), guiAlpha));

    // TODO: Review showValue parameter, really ugly...
    if (showValue) GuiDrawText(TextFormat("%.02f", value), (Rectangle){ bounds.x + bounds.width + GuiGetStyle(SLIDER, TEXT_PADDING),
                               bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2 + GuiGetStyle(SLIDER, INNER_PADDING),
                               GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SIZE) }, GUI_TEXT_ALIGN_LEFT, 
                               Fade(GetColor(GuiGetStyle(SLIDER, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return value;
}

// Slider control extended, returns selected value and has text
RAYGUIDEF float GuiSlider(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue)
{
    return GuiSliderPro(bounds, text, value, minValue, maxValue, GuiGetStyle(SLIDER, SLIDER_WIDTH), showValue);
}

// Slider Bar control extended, returns selected value
RAYGUIDEF float GuiSliderBar(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue)
{
    return GuiSliderPro(bounds, text, value, minValue, maxValue, 0, showValue);
}

// Progress Bar control extended, shows current progress value
RAYGUIDEF float GuiProgressBar(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue)
{
    GuiControlState state = guiState;

    Rectangle progress = { bounds.x + GuiGetStyle(PROGRESSBAR, BORDER_WIDTH),
                           bounds.y + GuiGetStyle(PROGRESSBAR, BORDER_WIDTH) + GuiGetStyle(PROGRESSBAR, INNER_PADDING), 0,
                           bounds.height - 2*GuiGetStyle(PROGRESSBAR, BORDER_WIDTH) - 2*GuiGetStyle(PROGRESSBAR, INNER_PADDING) };

    // Update control
    //--------------------------------------------------------------------
    if (state != GUI_STATE_DISABLED) progress.width = (int)(value/(maxValue - minValue)*(float)(bounds.width - 2*GuiGetStyle(PROGRESSBAR, BORDER_WIDTH)));
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (showValue) GuiLabel((Rectangle){ bounds.x + bounds.width + GuiGetStyle(SLIDER, TEXT_PADDING), bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2 + GuiGetStyle(SLIDER, INNER_PADDING), GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SIZE) }, TextFormat("%.02f", value));

    DrawRectangleLinesEx(bounds, GuiGetStyle(PROGRESSBAR, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(PROGRESSBAR, (state != GUI_STATE_DISABLED)? BORDER_COLOR_NORMAL : BORDER_COLOR_DISABLED)), guiAlpha));
    DrawRectangle(bounds.x + GuiGetStyle(PROGRESSBAR, BORDER_WIDTH), bounds.y + GuiGetStyle(PROGRESSBAR, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(PROGRESSBAR, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(PROGRESSBAR, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), guiAlpha));
    DrawRectangleRec(progress, Fade(GetColor(GuiGetStyle(PROGRESSBAR, (state != GUI_STATE_DISABLED)? BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));
    //--------------------------------------------------------------------

    return value;
}

// Status Bar control
RAYGUIDEF void GuiStatusBar(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;

    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DEFAULT, (state != GUI_STATE_DISABLED)? BORDER_COLOR_NORMAL : BORDER_COLOR_DISABLED)), guiAlpha));
    DrawRectangleRec((Rectangle){ bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.width - GuiGetStyle(DEFAULT, BORDER_WIDTH)*2, bounds.height - GuiGetStyle(DEFAULT, BORDER_WIDTH)*2 }, Fade(GetColor(GuiGetStyle(DEFAULT, (state != GUI_STATE_DISABLED)? BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));

    GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GuiGetStyle(DEFAULT, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(DEFAULT, (state != GUI_STATE_DISABLED)? TEXT_COLOR_NORMAL : TEXT_COLOR_DISABLED)), guiAlpha));
    //--------------------------------------------------------------------
}

// Dummy rectangle control, intended for placeholding
RAYGUIDEF void GuiDummyRec(Rectangle bounds, const char *text)
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
    DrawRectangleRec(bounds, Fade(GetColor(GuiGetStyle(DEFAULT, (state != GUI_STATE_DISABLED)? BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));

    GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(BUTTON, (state != GUI_STATE_DISABLED)? TEXT_COLOR_NORMAL : TEXT_COLOR_DISABLED)), guiAlpha));
    //------------------------------------------------------------------
}

// Scroll Bar control
RAYGUIDEF int GuiScrollBar(Rectangle bounds, int value, int minValue, int maxValue)
{
    GuiControlState state = guiState;

    // Is the scrollbar horizontal or vertical?
    bool isVertical = (bounds.width > bounds.height)? false : true;

    // The size (width or height depending on scrollbar type) of the spinner buttons
    const int spinnerSize = GuiGetStyle(SCROLLBAR, SHOW_SPINNER_BUTTONS)? (isVertical? bounds.width - 2 * GuiGetStyle(SCROLLBAR, BORDER_WIDTH) : bounds.height - 2 * GuiGetStyle(SCROLLBAR, BORDER_WIDTH)) : 0;

    // Spinner buttons [<] [>] [∧] [∨]
    Rectangle spinnerUpLeft, spinnerDownRight;
    // Actual area of the scrollbar excluding the spinner buttons
    Rectangle scrollbar;        //  ------------
    // Slider bar that moves     --[///]-----
    Rectangle slider;

    // Normalize value
    if (value > maxValue) value = maxValue;
    if (value < minValue) value = minValue;

    const int range = maxValue - minValue;
    int sliderSize = GuiGetStyle(SCROLLBAR, SLIDER_SIZE);
    
    // Calculate rectangles for all of the components
    spinnerUpLeft = (Rectangle){ bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), spinnerSize, spinnerSize };

    if (isVertical)
    {
        spinnerDownRight = (Rectangle){ bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), bounds.y + bounds.height - spinnerSize - GuiGetStyle(SCROLLBAR, BORDER_WIDTH), spinnerSize, spinnerSize};
        scrollbar = (Rectangle){ bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, INNER_PADDING), spinnerUpLeft.y + spinnerUpLeft.height, bounds.width - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, INNER_PADDING)), bounds.height - spinnerUpLeft.height - spinnerDownRight.height - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH) };
        sliderSize = (sliderSize >= scrollbar.height)? (scrollbar.height - 2) : sliderSize;     // Make sure the slider won't get outside of the scrollbar
        slider = (Rectangle){ bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SLIDER_PADDING),scrollbar.y + (int)(((float)(value - minValue)/range)*(scrollbar.height - sliderSize)),bounds.width - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SLIDER_PADDING)), sliderSize };
    }
    else
    {
        spinnerDownRight = (Rectangle){ bounds.x + bounds.width - spinnerSize - GuiGetStyle(SCROLLBAR, BORDER_WIDTH), bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), spinnerSize, spinnerSize};
        scrollbar = (Rectangle){ spinnerUpLeft.x + spinnerUpLeft.width, bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, INNER_PADDING), bounds.width - spinnerUpLeft.width - spinnerDownRight.width - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH), bounds.height - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, INNER_PADDING))};
        sliderSize = (sliderSize >= scrollbar.width)? (scrollbar.width - 2) : sliderSize;       // Make sure the slider won't get outside of the scrollbar
        slider = (Rectangle){ scrollbar.x + (int)(((float)(value - minValue)/range)*(scrollbar.width - sliderSize)), bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SLIDER_PADDING), sliderSize, bounds.height - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SLIDER_PADDING)) };
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
            int wheel = GetMouseWheelMove();
            if (wheel != 0) value += wheel;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (CheckCollisionPointRec(mousePoint, spinnerUpLeft)) value -= range/GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
                else if (CheckCollisionPointRec(mousePoint, spinnerDownRight)) value += range/GuiGetStyle(SCROLLBAR, SCROLL_SPEED);

                state = GUI_STATE_PRESSED;
            }
            else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                if (!isVertical)
                {
                    Rectangle scrollArea = { spinnerUpLeft.x + spinnerUpLeft.width, spinnerUpLeft.y, scrollbar.width, bounds.height - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH)};
                    if (CheckCollisionPointRec(mousePoint, scrollArea)) value = ((float)(mousePoint.x - scrollArea.x - slider.width/2)*range)/(scrollArea.width - slider.width) + minValue;
                }
                else
                {
                    Rectangle scrollArea = { spinnerUpLeft.x, spinnerUpLeft.y+spinnerUpLeft.height, bounds.width - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH),  scrollbar.height};
                    if (CheckCollisionPointRec(mousePoint, scrollArea)) value = ((float)(mousePoint.y - scrollArea.y - slider.height/2)*range)/(scrollArea.height - slider.height) + minValue;
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
    DrawRectangleRec(bounds, Fade(GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_DISABLED)), guiAlpha));   // Draw the background
    DrawRectangleRec(scrollbar, Fade(GetColor(GuiGetStyle(BUTTON, BASE_COLOR_NORMAL)), guiAlpha));     // Draw the scrollbar active area background

    DrawRectangleLinesEx(bounds, GuiGetStyle(SCROLLBAR, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + state*3)), guiAlpha));

    DrawRectangleRec(slider, Fade(GetColor(GuiGetStyle(SLIDER, BORDER + state*3)), guiAlpha));         // Draw the slider bar

    // Draw arrows using lines
    const int padding = (spinnerSize - GuiGetStyle(SCROLLBAR, ARROWS_SIZE))/2;
    const Vector2 lineCoords[] =
    {
        //coordinates for <     0,1,2
        {spinnerUpLeft.x + padding, spinnerUpLeft.y + spinnerSize/2},
        {spinnerUpLeft.x + spinnerSize - padding, spinnerUpLeft.y + padding },
        {spinnerUpLeft.x + spinnerSize - padding, spinnerUpLeft.y + spinnerSize - padding},

        //coordinates for >     3,4,5
        {spinnerDownRight.x + padding, spinnerDownRight.y + padding},
        {spinnerDownRight.x + spinnerSize - padding, spinnerDownRight.y + spinnerSize/2 },
        {spinnerDownRight.x + padding, spinnerDownRight.y + spinnerSize - padding},

        //coordinates for ∧     6,7,8
        {spinnerUpLeft.x + spinnerSize/2, spinnerUpLeft.y + padding},
        {spinnerUpLeft.x + padding, spinnerUpLeft.y + spinnerSize - padding},
        {spinnerUpLeft.x + spinnerSize - padding, spinnerUpLeft.y + spinnerSize - padding},

        //coordinates for ∨     9,10,11
        {spinnerDownRight.x + padding, spinnerDownRight.y + padding},
        {spinnerDownRight.x + spinnerSize/2, spinnerDownRight.y + spinnerSize - padding },
        {spinnerDownRight.x + spinnerSize - padding, spinnerDownRight.y + padding}
    };

    Color lineColor = Fade(GetColor(GuiGetStyle(BUTTON, TEXT + state*3)), guiAlpha);

    if (GuiGetStyle(SCROLLBAR, SHOW_SPINNER_BUTTONS))
    {
        if (isVertical)
        {
            // Draw ∧
            DrawLineEx(lineCoords[6], lineCoords[7], 3.0f, lineColor);
            DrawLineEx(lineCoords[6], lineCoords[8], 3.0f, lineColor);

            // Draw ∨
            DrawLineEx(lineCoords[9], lineCoords[10], 3.0f, lineColor);
            DrawLineEx(lineCoords[11], lineCoords[10], 3.0f, lineColor);
        }
        else
        {
            // Draw <
            DrawLineEx(lineCoords[0], lineCoords[1], 3.0f, lineColor);
            DrawLineEx(lineCoords[0], lineCoords[2], 3.0f, lineColor);

            // Draw >
            DrawLineEx(lineCoords[3], lineCoords[4], 3.0f, lineColor);
            DrawLineEx(lineCoords[5], lineCoords[4], 3.0f, lineColor);
        }
    }
    //--------------------------------------------------------------------

    return value;
}

// List Element control, returns element state
static bool GuiListElement(Rectangle bounds, const char *text, bool active, bool editMode)
{
    GuiControlState state = guiState;

    if (!guiLocked && editMode) state = GUI_STATE_NORMAL;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (!active)
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
                else state = GUI_STATE_FOCUSED;
            }

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) active = !active;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    // Draw element rectangle
    switch (state)
    {
        case GUI_STATE_NORMAL:
        {
            if (active)
            {
                DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_PRESSED)), guiAlpha));
                DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_PRESSED)), guiAlpha));
            }
        } break;
        case GUI_STATE_FOCUSED:
        {
            DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_FOCUSED)), guiAlpha));
            DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_FOCUSED)), guiAlpha));
        } break;
        case GUI_STATE_PRESSED:
        {
            DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_PRESSED)), guiAlpha));
            DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_PRESSED)), guiAlpha));
        } break;
        case GUI_STATE_DISABLED:
        {
            if (active)
            {
                DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_DISABLED)), guiAlpha));
                DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_NORMAL)), guiAlpha));
            }
        } break;
        default: break;
    }

    // Draw text depending on state
    if (state == GUI_STATE_NORMAL) GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GuiGetStyle(DEFAULT, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, active? TEXT_COLOR_PRESSED : TEXT_COLOR_NORMAL)), guiAlpha));
    else if (state == GUI_STATE_DISABLED) GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GuiGetStyle(DEFAULT, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, active? TEXT_COLOR_NORMAL : TEXT_COLOR_DISABLED)), guiAlpha));
    else GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GuiGetStyle(DEFAULT, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT + state*3)), guiAlpha));
    //--------------------------------------------------------------------

    return active;
}

// List View control
RAYGUIDEF bool GuiListView(Rectangle bounds, const char *text, int *active, int *scrollIndex, bool editMode)
{
    bool result = 0;

    int count = 0;
    const char **textList = GuiTextSplit(text, &count, NULL);

    result = GuiListViewEx(bounds, textList, count, NULL, active, NULL, scrollIndex, editMode);

    return result;
}

// List View control extended parameters
// NOTE: Elements could be disabled individually and focused element could be obtained:
//  int *enabled defines an array with enabled elements inside the list
//  int *focus returns focused element (may be not pressed)
RAYGUIDEF bool GuiListViewEx(Rectangle bounds, const char **text, int count, int *enabled, int *active, int *focus, int *scrollIndex, bool editMode)
{
    GuiControlState state = guiState;
    bool pressed = false;

    int focusElement = -1;
    int startIndex = (scrollIndex == NULL)? 0 : *scrollIndex;
    bool useScrollBar = true;
    bool pressedKey = false;

    int visibleElements = bounds.height/(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING));
    if ((startIndex < 0) || (startIndex > count - visibleElements)) startIndex = 0;
    int endIndex = startIndex + visibleElements;

    int auxActive = *active;

    float barHeight = bounds.height;
    float minBarHeight = 10;

    // Update control
    //--------------------------------------------------------------------
    // All the elements fit inside ListView and dont need scrollbar.
    if (visibleElements >= count)
    {
        useScrollBar = false;
        startIndex = 0;
        endIndex = count;
    }

    // Calculate position X and width to draw each element.
    int posX = bounds.x + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING);
    int elementWidth = bounds.width - 2*GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) - GuiGetStyle(DEFAULT, BORDER_WIDTH);

    if (useScrollBar)
    {
        posX = GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE? posX + GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) : posX;
        elementWidth = bounds.width - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) - 2*GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) - GuiGetStyle(DEFAULT, BORDER_WIDTH);
    }

    Rectangle scrollBarRect = { bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH), bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) };

    if (GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_RIGHT_SIDE) scrollBarRect.x = posX + elementWidth + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING);

    // Area without the scrollbar
    Rectangle viewArea = { posX, bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), elementWidth, bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) };

    if ((state != GUI_STATE_DISABLED) && !guiLocked) // && !guiLocked
    {
        Vector2 mousePoint = GetMousePosition();

        if (editMode)
        {
            state = GUI_STATE_PRESSED;

            // Change active with keys
            if (IsKeyPressed(KEY_UP))
            {
                if (auxActive > 0)
                {
                    auxActive--;
                    if ((useScrollBar) && (auxActive < startIndex)) startIndex--;
                }

                pressedKey = true;
            }
            else if (IsKeyPressed(KEY_DOWN))
            {
                if (auxActive < count - 1)
                {
                    auxActive++;
                    if ((useScrollBar) && (auxActive >= endIndex)) startIndex++;
                }

                pressedKey = true;
            }

            if (useScrollBar)
            {
                endIndex = startIndex + visibleElements;
                if (CheckCollisionPointRec(mousePoint, viewArea))
                {
                    int wheel = GetMouseWheelMove();

                    if (wheel < 0 && endIndex < count) startIndex -= wheel;
                    else if (wheel > 0 && startIndex > 0)  startIndex -= wheel;
                }

                if (pressedKey)
                {
                    pressedKey = false;
                    if ((auxActive < startIndex) || (auxActive  >= endIndex)) startIndex = auxActive;
                }

                if (startIndex < 0) startIndex = 0;
                else if (startIndex > (count - (endIndex - startIndex)))
                {
                    startIndex = count - (endIndex - startIndex);
                }

                endIndex = startIndex + visibleElements;

                if (endIndex > count) endIndex = count;
            }
        }

        if (!editMode)
        {
            if (CheckCollisionPointRec(mousePoint, viewArea))
            {
                state = GUI_STATE_FOCUSED;
                if (IsMouseButtonPressed(0)) pressed = true;

                startIndex -= GetMouseWheelMove();

                if (startIndex < 0) startIndex = 0;
                else if (startIndex > (count - (endIndex - startIndex)))
                {
                    startIndex = count - (endIndex - startIndex);
                }

                pressed = true;
            }
        }
        else
        {
            if (!CheckCollisionPointRec(mousePoint, viewArea))
            {
                if (IsMouseButtonPressed(0) || (GetMouseWheelMove() != 0)) pressed = true;
            }
        }

        // Get focused element
        for (int i = startIndex; i < endIndex; i++)
        {
            if (CheckCollisionPointRec(mousePoint, (Rectangle){ posX, bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }))
            {
                focusElement = i;
            }
        }
    }

    const int slider = GuiGetStyle(SCROLLBAR, SLIDER_SIZE); // Save default slider size
    
    // Calculate percentage of visible elements and apply same percentage to scrollbar
    if (useScrollBar)
    {
        float percentVisible = (endIndex - startIndex)*100/count;
        barHeight *= percentVisible/100;

        if (barHeight < minBarHeight) barHeight = minBarHeight;
        else if (barHeight > bounds.height) barHeight = bounds.height;

        GuiSetStyle(SCROLLBAR, SLIDER_SIZE, barHeight); // Change slider size
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleRec(bounds, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));     // Draw background

    // Draw scrollBar
    if (useScrollBar)
    {
        const int scrollSpeed = GuiGetStyle(SCROLLBAR, SCROLL_SPEED);     // Save default scroll speed
        GuiSetStyle(SCROLLBAR, SCROLL_SPEED, count - visibleElements);    // Hack to make the spinner buttons work

        int index = scrollIndex != NULL? *scrollIndex : startIndex;
        index = GuiScrollBar(scrollBarRect, index, 0, count - visibleElements);

        GuiSetStyle(SCROLLBAR, SCROLL_SPEED, scrollSpeed); // Reset scroll speed to default
        GuiSetStyle(SCROLLBAR, SLIDER_SIZE, slider); // Reset slider size to default

        // FIXME: Quick hack to make this thing work, think of a better way
        if (scrollIndex != NULL && CheckCollisionPointRec(GetMousePosition(), scrollBarRect) && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            startIndex = index;
            if (startIndex < 0) startIndex = 0;
            if (startIndex > (count - (endIndex - startIndex)))
            {
                startIndex = count - (endIndex - startIndex);
            }

            endIndex = startIndex + visibleElements;

            if (endIndex > count) endIndex = count;
        }
    }

    DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + state*3)), guiAlpha));

    // Draw ListView states
    switch (state)
    {
        case GUI_STATE_NORMAL:
        {
            for (int i = startIndex; i < endIndex; i++)
            {
                if ((enabled != NULL) && (enabled[i] == 0))
                {
                    GuiDisable();
                    GuiListElement((Rectangle){ posX, bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
                    GuiEnable();
                }
                else if (i == auxActive)
                {
                    GuiDisable();
                    GuiListElement((Rectangle){ posX, bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], true, false);
                    GuiEnable();
                }
                else GuiListElement((Rectangle){ posX, bounds.y  + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
            }
        } break;
        case GUI_STATE_FOCUSED:
        {
            for (int i = startIndex; i < endIndex; i++)
            {
                if ((enabled != NULL) && (enabled[i] == 0))
                {
                    GuiDisable();
                    GuiListElement((Rectangle){ posX, bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
                    GuiEnable();
                }
                else if (i == auxActive) GuiListElement((Rectangle){ posX, bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], true, false);
                else GuiListElement((Rectangle){ posX, bounds.y  + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
            }
        } break;
        case GUI_STATE_PRESSED:
        {
            for (int i = startIndex; i < endIndex; i++)
            {
                if ((enabled != NULL) && (enabled[i] == 0))
                {
                    GuiDisable();
                    GuiListElement((Rectangle){ posX, bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
                    GuiEnable();
                }
                else if ((i == auxActive) && editMode)
                {
                    if (GuiListElement((Rectangle){ posX, bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], true, true) == false) auxActive = -1;
                }
                else
                {
                    if (GuiListElement((Rectangle){ posX, bounds.y  + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, true) == true) auxActive = i;
                }
            }
        } break;
        case GUI_STATE_DISABLED:
        {
            for (int i = startIndex; i < endIndex; i++)
            {
                if (i == auxActive) GuiListElement((Rectangle){ posX, bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], true, false);
                else GuiListElement((Rectangle){ posX, bounds.y  + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), elementWidth, GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
            }
        } break;
        default: break;
    }
    //--------------------------------------------------------------------

    if (scrollIndex != NULL) *scrollIndex = startIndex;
    if (focus != NULL) *focus = focusElement;
    *active = auxActive;

    return pressed;
}

// Color Panel control
RAYGUIDEF Color GuiColorPanel(Rectangle bounds, Color color)
{
    GuiControlState state = guiState;
    Vector2 pickerSelector = { 0 };

    Vector3 vcolor = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
    Vector3 hsv = ConvertRGBtoHSV(vcolor);

    pickerSelector.x = bounds.x + (float)hsv.y*bounds.width;            // HSV: Saturation
    pickerSelector.y = bounds.y + (1.0f - (float)hsv.z)*bounds.height;  // HSV: Value

    Vector3 maxHue = { hsv.x, 1.0f, 1.0f };
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
                color = (Color){ (unsigned char)(255.0f*rgb.x),
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
        DrawRectangleGradientEx(bounds, Fade(WHITE, guiAlpha), Fade(WHITE, guiAlpha), Fade(maxHueCol, guiAlpha), Fade(maxHueCol, guiAlpha));
        DrawRectangleGradientEx(bounds, Fade(BLACK, 0), Fade(BLACK, guiAlpha), Fade(BLACK, guiAlpha), Fade(BLACK, 0));

        // Draw color picker: selector
        DrawRectangle(pickerSelector.x - GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE)/2, pickerSelector.y - GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE)/2, GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE), GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE), Fade(WHITE, guiAlpha));
    }
    else
    {
        DrawRectangleGradientEx(bounds, Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), guiAlpha), Fade(Fade(BLACK, 0.6f), guiAlpha), Fade(Fade(BLACK, 0.6f), guiAlpha), Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), 0.6f), guiAlpha));
    }

    DrawRectangleLines(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));
    //--------------------------------------------------------------------

    return color;
}

// Color Bar Alpha control
// NOTE: Returns alpha value normalized [0..1]
RAYGUIDEF float GuiColorBarAlpha(Rectangle bounds, float alpha)
{
    #define COLORBARALPHA_CHECKED_SIZE          10

    GuiControlState state = guiState;
    Rectangle selector = { bounds.x + alpha*bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), bounds.y - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), GuiGetStyle(COLORPICKER, BAR_SELECTOR_HEIGHT), bounds.height + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)*2 };

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
                selector.x = mousePoint.x - selector.width/2;

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
        for (int i = 0; i < bounds.width/COLORBARALPHA_CHECKED_SIZE; i++) DrawRectangle(bounds.x + COLORBARALPHA_CHECKED_SIZE*(i%((int)bounds.width/COLORBARALPHA_CHECKED_SIZE)), bounds.y, bounds.width/(bounds.width/COLORBARALPHA_CHECKED_SIZE), COLORBARALPHA_CHECKED_SIZE, (i%2)? Fade(Fade(GRAY, 0.4f), guiAlpha) : Fade(Fade(RAYWHITE, 0.4f), guiAlpha));
        for (int i = 0; i < bounds.width/COLORBARALPHA_CHECKED_SIZE; i++) DrawRectangle(bounds.x + COLORBARALPHA_CHECKED_SIZE*(i%((int)bounds.width/COLORBARALPHA_CHECKED_SIZE)), bounds.y + COLORBARALPHA_CHECKED_SIZE, bounds.width/(bounds.width/COLORBARALPHA_CHECKED_SIZE), COLORBARALPHA_CHECKED_SIZE, (i%2)? Fade(Fade(RAYWHITE, 0.4f), guiAlpha) : Fade(Fade(GRAY, 0.4f), guiAlpha));
        DrawRectangleGradientH(bounds.x, bounds.y, bounds.width, bounds.height, Fade((Color){ 255,255,255,0 }, guiAlpha), Fade((Color){ 0,0,0,255 }, guiAlpha));
    }
    else DrawRectangleGradientH(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha));

    DrawRectangleLines(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));

    switch (state)
    {
        case GUI_STATE_NORMAL: DrawRectangle(selector.x , selector.y, selector.width, selector.height, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_PRESSED)), guiAlpha)); break;
        case GUI_STATE_FOCUSED: DrawRectangle(selector.x, selector.y, selector.width, selector.height, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_FOCUSED)), guiAlpha)); break;
        case GUI_STATE_PRESSED: DrawRectangle(selector.x, selector.y, selector.width, selector.height, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_PRESSED)), guiAlpha)); break;
        case GUI_STATE_DISABLED: DrawRectangleRec(selector, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha)); break;
        default: break;
    }
    //--------------------------------------------------------------------

    return alpha;
}

// Color Bar Hue control
// NOTE: Returns hue value normalized [0..1]
RAYGUIDEF float GuiColorBarHue(Rectangle bounds, float hue)
{
    GuiControlState state = guiState;
    Rectangle selector = { bounds.x - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), bounds.y + hue/360.0f*bounds.height - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), bounds.width + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)*2, GuiGetStyle(COLORPICKER, BAR_SELECTOR_HEIGHT) };

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
                selector.y = mousePoint.y - selector.height/2;

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
        DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6, Fade((Color){ 255,0,0,255 }, guiAlpha), Fade((Color){ 255,255,0,255 }, guiAlpha));
        DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + (int)bounds.height/6 + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6, Fade((Color){ 255,255,0,255 }, guiAlpha), Fade((Color){ 0,255,0,255 }, guiAlpha));
        DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + 2*((int)bounds.height/6) + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6, Fade((Color){ 0,255,0,255 }, guiAlpha), Fade((Color){ 0,255,255,255 }, guiAlpha));
        DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + 3*((int)bounds.height/6) + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6, Fade((Color){ 0,255,255,255 }, guiAlpha), Fade((Color){ 0,0,255,255 }, guiAlpha));
        DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + 4*((int)bounds.height/6) + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6, Fade((Color){ 0,0,255,255 }, guiAlpha), Fade((Color){ 255,0,255,255 }, guiAlpha));
        DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + 5*((int)bounds.height/6) + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6 - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), Fade((Color){ 255,0,255,255 }, guiAlpha), Fade((Color){ 255,0,0,255 }, guiAlpha));
    }
    else
    {
        DrawRectangleGradientV(bounds.x, bounds.y, bounds.width, bounds.height, Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), guiAlpha), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha));
    }

    // Draw hue bar: selector
    DrawRectangleLines(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));
    DrawRectangle(selector.x, selector.y, selector.width, selector.height, Fade(GetColor(GuiGetStyle(COLORPICKER, (state == GUI_STATE_NORMAL)? BORDER_COLOR_PRESSED : (BORDER + state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return hue;
}

// TODO: Color GuiColorBarSat() [WHITE->color]
// TODO: Color GuiColorBarValue() [BLACK->color], HSV / HSL
// TODO: float GuiColorBarLuminance() [BLACK->WHITE]

// Color Picker control
// NOTE: It's divided in multiple controls:
//      Color GuiColorPanel() - Color select panel
//      float GuiColorBarAlpha(Rectangle bounds, float alpha)
//      float GuiColorBarHue(Rectangle bounds, float value)
// NOTE: bounds define GuiColorPanel() size
RAYGUIDEF Color GuiColorPicker(Rectangle bounds, Color color)
{
    color = GuiColorPanel(bounds, color);

    Rectangle boundsHue = { bounds.x + bounds.width + GuiGetStyle(COLORPICKER, BAR_PADDING), bounds.y, GuiGetStyle(COLORPICKER, BAR_WIDTH), bounds.height };
    //Rectangle boundsAlpha = { bounds.x, bounds.y + bounds.height + GuiGetStyle(COLORPICKER, BARS_PADDING), bounds.width, GuiGetStyle(COLORPICKER, BARS_THICK) };

    Vector3 hsv = ConvertRGBtoHSV((Vector3){ color.r/255.0f, color.g/255.0f, color.b/255.0f });
    hsv.x = GuiColorBarHue(boundsHue, hsv.x);
    //color.a = (unsigned char)(GuiColorBarAlpha(boundsAlpha, (float)color.a/255.0f)*255.0f);
    Vector3 rgb = ConvertHSVtoRGB(hsv);
    color = (Color){ (unsigned char)(rgb.x*255.0f), (unsigned char)(rgb.y*255.0f), (unsigned char)(rgb.z*255.0f), color.a };

    return color;
}

// Message Box control
RAYGUIDEF int GuiMessageBox(Rectangle bounds, const char *windowTitle, const char *message, const char *buttons)
{
    #define MESSAGEBOX_BUTTON_HEIGHT            24
    #define MESSAGEBOX_BUTTON_PADDING           10

    int clicked = -1;    // Returns clicked button from buttons list, 0 refers to closed window button
    
    int buttonsCount = 0;
    const char **buttonsText = GuiTextSplit(buttons, &buttonsCount, NULL);

    Vector2 textSize = MeasureTextEx(guiFont, message, GuiGetStyle(DEFAULT, TEXT_SIZE), 1);
    
    Rectangle textBounds = { 0 };
    textBounds.x = bounds.x + bounds.width/2 - textSize.x/2;
    textBounds.y = bounds.y + WINDOW_STATUSBAR_HEIGHT + (bounds.height - WINDOW_STATUSBAR_HEIGHT)/4 - textSize.y/2;
    textBounds.width = textSize.x;
    textBounds.height = textSize.y;

    Rectangle buttonBounds = { 0 };
    buttonBounds.x = bounds.x + MESSAGEBOX_BUTTON_PADDING;
    buttonBounds.y = bounds.y + bounds.height/2 + bounds.height/4 - MESSAGEBOX_BUTTON_HEIGHT/2;
    buttonBounds.width = (bounds.width - MESSAGEBOX_BUTTON_PADDING*(buttonsCount + 1))/buttonsCount;
    buttonBounds.height = MESSAGEBOX_BUTTON_HEIGHT;

    // Draw control
    //--------------------------------------------------------------------
    if (GuiWindowBox(bounds, windowTitle)) clicked = 0;
    
    int prevTextAlignment = GuiGetStyle(LABEL, TEXT_ALIGNMENT);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
    GuiLabel(textBounds, message);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, prevTextAlignment);

    prevTextAlignment = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
    
    for (int i = 0; i < buttonsCount; i++)
    {
        if (GuiButton(buttonBounds, buttonsText[i])) clicked = i + 1;
        buttonBounds.x += (buttonBounds.width + MESSAGEBOX_BUTTON_PADDING);
    }
    
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, prevTextAlignment);
    //--------------------------------------------------------------------

    return clicked;
}

// Grid control
// NOTE: Returns grid mouse-hover selected cell
// About drawing lines at subpixel spacing, simple put, not easy solution:
// https://stackoverflow.com/questions/4435450/2d-opengl-drawing-lines-that-dont-exactly-fit-pixel-raster
RAYGUIDEF Vector2 GuiGrid(Rectangle bounds, float spacing, int subdivs)
{
    #define GRID_COLOR_ALPHA    0.15f           // Grid lines alpha amount

    GuiControlState state = guiState;
    Vector2 mousePoint = GetMousePosition();
    Vector2 currentCell = { -1, -1 };

    int linesV = ((int)(bounds.width/spacing) + 1)*subdivs;
    int linesH = ((int)(bounds.height/spacing) + 1)*subdivs;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            currentCell.x = (int)((mousePoint.x - bounds.x)/spacing);
            currentCell.y = (int)((mousePoint.y - bounds.y)/spacing);
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    switch (state)
    {
        case GUI_STATE_NORMAL:
        {
            // Draw vertical grid lines
            for (int i = 0; i < linesV; i++)
            {
                DrawRectangleRec((Rectangle){ bounds.x + spacing*i, bounds.y, 1, bounds.height }, ((i%subdivs) == 0)? Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA*4) : Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA));
            }

            // Draw horizontal grid lines
            for (int i = 0; i < linesH; i++)
            {
                DrawRectangleRec((Rectangle){ bounds.x, bounds.y + spacing*i, bounds.width, 1 }, ((i%subdivs) == 0)? Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA*4) : Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA));
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
RAYGUIDEF void GuiLoadStyle(const char *fileName)
{
    FILE *rgsFile = fopen(fileName, "rb");

    if (rgsFile != NULL)
    {
        unsigned int value = 0;

        char signature[5] = "";
        short version = 0;
        short numControls = 0;
        short numPropsDefault = 0;
        short numPropsExtended = 0;

        fread(signature, 1, 4, rgsFile);
        fread(&version, 1, sizeof(short), rgsFile);
        fread(&numControls, 1, sizeof(short), rgsFile);
        fread(&numPropsDefault, 1, sizeof(short), rgsFile);
        fread(&numPropsExtended, 1, sizeof(short), rgsFile);

        if ((signature[0] == 'r') &&
            (signature[1] == 'G') &&
            (signature[2] == 'S') &&
            (signature[3] == ' '))
        {
            for (int i = 0; i < NUM_CONTROLS; i++)
            {
                for (int j = 0; j < NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED; j++)
                {
                    fread(&value, 1, sizeof(unsigned int), rgsFile);
                    GuiSetStyle(i, j, value);
                }
            }

            // Font loading is highly dependant on raylib API to load font data and image
            // TODO: Find some mechanism to support it in standalone mode
            
#if !defined(RAYGUI_STANDALONE)
            // Load custom font if available
            int fontDataSize = 0;
            fwrite(&fontDataSize, 1, sizeof(int), rgsFile);

            if (fontDataSize > 0)
            {
                Font font = { 0 };
                int fontType = 0;   // 0-Normal, 1-SDF
                Rectangle whiteRec = { 0 };

                fread(&font.baseSize, 1, sizeof(int), rgsFile);
                fread(&font.charsCount, 1, sizeof(int), rgsFile);
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
                    fread(&imFont.data, 1, fontImageSize, rgsFile);

                    font.texture = LoadTextureFromImage(imFont);
                    UnloadImage(imFont);
                }

                // Load font chars data
                font.chars = (CharInfo *)calloc(font.charsCount, sizeof(CharInfo));
                for (int i = 0; i < font.charsCount; i++)
                {
                    fread(&font.chars[i].rec, 1, sizeof(Rectangle), rgsFile);
                    fread(&font.chars[i].value, 1, sizeof(int), rgsFile);
                    fread(&font.chars[i].offsetX, 1, sizeof(int), rgsFile);
                    fread(&font.chars[i].offsetY, 1, sizeof(int), rgsFile);
                    fread(&font.chars[i].advanceX, 1, sizeof(int), rgsFile);
                }

                GuiFont(font);

                // Set font texture source rectangle to be used as white texture to draw shapes
                // NOTE: This way, all gui can be draw using a single draw call
                if ((whiteRec.width != 0) && (whiteRec.height != 0)) SetShapesTexture(font.texture, whiteRec);
            }
#endif
        }
        
        fclose(rgsFile);
    }
}

// Load style from a palette values array
RAYGUIDEF void GuiLoadStyleProps(const int *props, int count)
{
    int completeSets = count/(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED);
    int uncompleteSetProps = count%(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED);

    // Load style palette values from array (complete property sets)
    for (int i = 0; i < completeSets; i++)
    {
        for (int j = 0; j < (NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED); i++) GuiSetStyle(i, j, props[i]);
    }

    // Load style palette values from array (uncomplete property set)
    for (int k = 0; k < uncompleteSetProps; k++) GuiSetStyle(completeSets, k, props[completeSets*(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED) + k]);
}

// Load style default over global style
RAYGUIDEF void GuiLoadStyleDefault(void)
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
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
    GuiSetStyle(DEFAULT, INNER_PADDING, 1);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

    // Populate all controls with default style
    for (int i = 1; i < NUM_CONTROLS; i++)
    {
        for (int j = 0; j < NUM_PROPS_DEFAULT; j++) GuiSetStyle(i, j, GuiGetStyle(DEFAULT, j));
    }

    // Initialize extended property values
    // NOTE: By default, extended property values are initialized to 0
    GuiSetStyle(DEFAULT, TEXT_SIZE, 10);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 1);
    GuiSetStyle(DEFAULT, LINE_COLOR, 0x90abb5ff);           // DEFAULT specific property
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0xf5f5f5ff);     // DEFAULT specific property

    GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
    GuiSetStyle(BUTTON, INNER_PADDING, 4);
    GuiSetStyle(TOGGLE, GROUP_PADDING, 2);
    GuiSetStyle(SLIDER, SLIDER_WIDTH, 15);
    GuiSetStyle(SLIDER, TEXT_PADDING, 5);
    GuiSetStyle(CHECKBOX, CHECK_TEXT_PADDING, 5);
    GuiSetStyle(COMBOBOX, SELECTOR_WIDTH, 30);
    GuiSetStyle(COMBOBOX, SELECTOR_PADDING, 2);
    GuiSetStyle(DROPDOWNBOX, ARROW_RIGHT_PADDING, 16);
    GuiSetStyle(TEXTBOX, INNER_PADDING, 4);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
    GuiSetStyle(TEXTBOX, MULTILINE_PADDING, 5);
    GuiSetStyle(TEXTBOX, SPINNER_BUTTON_WIDTH, 20);         // SPINNER specific property
    GuiSetStyle(TEXTBOX, SPINNER_BUTTON_PADDING, 2);        // SPINNER specific property
    GuiSetStyle(TEXTBOX, SPINNER_BUTTON_BORDER_WIDTH, 1);   // SPINNER specific property
    //GuiSetStyle(VALUEBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);     // TODO.
    GuiSetStyle(COLORPICKER, COLOR_SELECTOR_SIZE, 6);
    GuiSetStyle(COLORPICKER, BAR_WIDTH, 0x14);
    GuiSetStyle(COLORPICKER, BAR_PADDING, 0xa);
    GuiSetStyle(COLORPICKER, BAR_SELECTOR_HEIGHT, 6);
    GuiSetStyle(COLORPICKER, BAR_SELECTOR_PADDING, 2);
    GuiSetStyle(LISTVIEW, ELEMENTS_HEIGHT, 0x1e);
    GuiSetStyle(LISTVIEW, ELEMENTS_PADDING, 2);
    GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, 10);
    GuiSetStyle(LISTVIEW, SCROLLBAR_SIDE, SCROLLBAR_RIGHT_SIDE);
    GuiSetStyle(SCROLLBAR, BORDER_WIDTH, 0);
    GuiSetStyle(SCROLLBAR, SHOW_SPINNER_BUTTONS, 0);
    GuiSetStyle(SCROLLBAR, ARROWS_SIZE, 6);
    GuiSetStyle(SCROLLBAR, INNER_PADDING, 0);
    GuiSetStyle(SCROLLBAR, SLIDER_PADDING, 0);
    GuiSetStyle(SCROLLBAR, SLIDER_SIZE, 16);
    GuiSetStyle(SCROLLBAR, SCROLL_SPEED, 10);
}

// Updates controls style with default values
RAYGUIDEF void GuiUpdateStyleComplete(void)
{
    // Populate all controls with default style
    // NOTE: Extended style properties are ignored
    for (int i = 1; i < NUM_CONTROLS; i++)
    {
        for (int j = 0; j < NUM_PROPS_DEFAULT; j++) GuiSetStyle(i, j, GuiGetStyle(DEFAULT, j));
    }
}

// Get text with icon id prepended
// NOTE: Useful to add icons by name id (enum) instead of 
// a number that can change between ricon versions
RAYGUIDEF const char *GuiIconText(int iconId, const char *text)
{
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
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Split controls text into multiple strings
// Also check for multiple columns (required by GuiToggleGroup())
static const char **GuiTextSplit(const char *text, int *count, int *textRow)
{
    // NOTE: Current implementation returns a copy of the provided string with '\0' (string end delimiter)
    // inserted between strings defined by "delimiter" parameter. No memory is dynamically allocated,
    // all used memory is static... it has some limitations:
    //      1. Maximum number of possible split strings is set by MAX_SUBSTRINGS_COUNT
    //      2. Maximum size of text to split is MAX_TEXT_BUFFER_LENGTH

    #define MAX_TEXT_BUFFER_LENGTH   1024
    #define MAX_SUBSTRINGS_COUNT       64

    static const char *result[MAX_SUBSTRINGS_COUNT] = { NULL };
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    result[0] = buffer;
    int counter = 1;

    if (textRow != NULL) textRow[0] = 0;

    // Count how many substrings we have on text and point to every one
    for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
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
            if (counter == MAX_SUBSTRINGS_COUNT) break;
        }
    }

    *count = counter;

    return result;
}

// Convert color data from RGB to HSV
// NOTE: Color data should be passed normalized
static Vector3 ConvertRGBtoHSV(Vector3 rgb)
{
    Vector3 hsv = { 0.0f };
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
        hsv.x = 0.0f;       // Undefined, maybe NAN?
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
        hsv.x = 0.0f;        // Undefined, maybe NAN?
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
    Vector3 rgb = { 0.0f };
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

    return (Color){color.r, color.g, color.b, (unsigned char)(255.0f*alpha)};
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
#endif      // RAYGUI_STANDALONE

#endif      // RAYGUI_IMPLEMENTATION
