/*******************************************************************************************
*
*   raygui - raylib IMGUI system (Immedite Mode GUI)
*
*   Copyright (c) 2015 Kevin Gato, Daniel Nicolás, Sergio Martinez and Ramon Santamaria
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

//#include "raylib.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define NUM_PROPERTIES       98

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
#endif

// Gui properties enumeration
typedef enum GuiProperty {
    GLOBAL_BASE_COLOR = 0,
    GLOBAL_BORDER_COLOR,
    GLOBAL_TEXT_COLOR,
    GLOBAL_TEXT_FONTSIZE,
    GLOBAL_BORDER_WIDTH,
    BACKGROUND_COLOR,
    LABEL_BORDER_WIDTH,
    LABEL_TEXT_COLOR,
    LABEL_TEXT_PADDING,
    BUTTON_BORDER_WIDTH,
    BUTTON_TEXT_PADDING,
    BUTTON_DEFAULT_BORDER_COLOR,
    BUTTON_DEFAULT_INSIDE_COLOR,
    BUTTON_DEFAULT_TEXT_COLOR,
    BUTTON_HOVER_BORDER_COLOR,
    BUTTON_HOVER_INSIDE_COLOR,
    BUTTON_HOVER_TEXT_COLOR,
    BUTTON_PRESSED_BORDER_COLOR,
    BUTTON_PRESSED_INSIDE_COLOR,
    BUTTON_PRESSED_TEXT_COLOR,
    TOGGLE_TEXT_PADDING,
    TOGGLE_BORDER_WIDTH,
    TOGGLE_DEFAULT_BORDER_COLOR,
    TOGGLE_DEFAULT_INSIDE_COLOR,
    TOGGLE_DEFAULT_TEXT_COLOR,
    TOGGLE_HOVER_BORDER_COLOR,
    TOGGLE_HOVER_INSIDE_COLOR,
    TOGGLE_HOVER_TEXT_COLOR,
    TOGGLE_PRESSED_BORDER_COLOR,
    TOGGLE_PRESSED_INSIDE_COLOR,
    TOGGLE_PRESSED_TEXT_COLOR,
    TOGGLE_ACTIVE_BORDER_COLOR,
    TOGGLE_ACTIVE_INSIDE_COLOR,
    TOGGLE_ACTIVE_TEXT_COLOR,
    TOGGLEGROUP_PADDING,
    SLIDER_BORDER_WIDTH,
    SLIDER_BUTTON_BORDER_WIDTH,
    SLIDER_BORDER_COLOR,
    SLIDER_INSIDE_COLOR,
    SLIDER_DEFAULT_COLOR,
    SLIDER_HOVER_COLOR,
    SLIDER_ACTIVE_COLOR,
    SLIDERBAR_BORDER_COLOR,
    SLIDERBAR_INSIDE_COLOR,
    SLIDERBAR_DEFAULT_COLOR,
    SLIDERBAR_HOVER_COLOR,
    SLIDERBAR_ACTIVE_COLOR,
    SLIDERBAR_ZERO_LINE_COLOR,
    PROGRESSBAR_BORDER_COLOR,
    PROGRESSBAR_INSIDE_COLOR,
    PROGRESSBAR_PROGRESS_COLOR,
    PROGRESSBAR_BORDER_WIDTH,
    SPINNER_LABEL_BORDER_COLOR,
    SPINNER_LABEL_INSIDE_COLOR,
    SPINNER_DEFAULT_BUTTON_BORDER_COLOR,
    SPINNER_DEFAULT_BUTTON_INSIDE_COLOR,
    SPINNER_DEFAULT_SYMBOL_COLOR,
    SPINNER_DEFAULT_TEXT_COLOR,
    SPINNER_HOVER_BUTTON_BORDER_COLOR,
    SPINNER_HOVER_BUTTON_INSIDE_COLOR,
    SPINNER_HOVER_SYMBOL_COLOR,
    SPINNER_HOVER_TEXT_COLOR,
    SPINNER_PRESSED_BUTTON_BORDER_COLOR,
    SPINNER_PRESSED_BUTTON_INSIDE_COLOR,
    SPINNER_PRESSED_SYMBOL_COLOR,
    SPINNER_PRESSED_TEXT_COLOR,
    COMBOBOX_PADDING,
    COMBOBOX_BUTTON_WIDTH,
    COMBOBOX_BUTTON_HEIGHT,
    COMBOBOX_BORDER_WIDTH,
    COMBOBOX_DEFAULT_BORDER_COLOR,
    COMBOBOX_DEFAULT_INSIDE_COLOR,
    COMBOBOX_DEFAULT_TEXT_COLOR,
    COMBOBOX_DEFAULT_LIST_TEXT_COLOR,
    COMBOBOX_HOVER_BORDER_COLOR,
    COMBOBOX_HOVER_INSIDE_COLOR,
    COMBOBOX_HOVER_TEXT_COLOR,
    COMBOBOX_HOVER_LIST_TEXT_COLOR,
    COMBOBOX_PRESSED_BORDER_COLOR,
    COMBOBOX_PRESSED_INSIDE_COLOR,
    COMBOBOX_PRESSED_TEXT_COLOR,
    COMBOBOX_PRESSED_LIST_BORDER_COLOR,
    COMBOBOX_PRESSED_LIST_INSIDE_COLOR,
    COMBOBOX_PRESSED_LIST_TEXT_COLOR,
    CHECKBOX_DEFAULT_BORDER_COLOR,
    CHECKBOX_DEFAULT_INSIDE_COLOR,
    CHECKBOX_HOVER_BORDER_COLOR,
    CHECKBOX_HOVER_INSIDE_COLOR,
    CHECKBOX_CLICK_BORDER_COLOR,
    CHECKBOX_CLICK_INSIDE_COLOR,
    CHECKBOX_STATUS_ACTIVE_COLOR,
    CHECKBOX_INSIDE_WIDTH,
    TEXTBOX_BORDER_WIDTH,
    TEXTBOX_BORDER_COLOR,
    TEXTBOX_INSIDE_COLOR,
    TEXTBOX_TEXT_COLOR,
    TEXTBOX_LINE_COLOR,
    TEXTBOX_TEXT_FONTSIZE
} GuiProperty;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const char *guiPropertyName[] = {
    "GLOBAL_BASE_COLOR",
    "GLOBAL_BORDER_COLOR",
    "GLOBAL_TEXT_COLOR",
    "GLOBAL_TEXT_FONTSIZE",
    "GLOBAL_BORDER_WIDTH",
    "BACKGROUND_COLOR",
    "LABEL_BORDER_WIDTH",
    "LABEL_TEXT_COLOR",
    "LABEL_TEXT_PADDING",
    "BUTTON_BORDER_WIDTH",
    "BUTTON_TEXT_PADDING",
    "BUTTON_DEFAULT_BORDER_COLOR",
    "BUTTON_DEFAULT_INSIDE_COLOR",
    "BUTTON_DEFAULT_TEXT_COLOR",
    "BUTTON_HOVER_BORDER_COLOR",
    "BUTTON_HOVER_INSIDE_COLOR",
    "BUTTON_HOVER_TEXT_COLOR",
    "BUTTON_PRESSED_BORDER_COLOR",
    "BUTTON_PRESSED_INSIDE_COLOR",
    "BUTTON_PRESSED_TEXT_COLOR",
    "TOGGLE_TEXT_PADDING",
    "TOGGLE_BORDER_WIDTH",
    "TOGGLE_DEFAULT_BORDER_COLOR",
    "TOGGLE_DEFAULT_INSIDE_COLOR",
    "TOGGLE_DEFAULT_TEXT_COLOR",
    "TOGGLE_HOVER_BORDER_COLOR",
    "TOGGLE_HOVER_INSIDE_COLOR",
    "TOGGLE_HOVER_TEXT_COLOR",
    "TOGGLE_PRESSED_BORDER_COLOR",
    "TOGGLE_PRESSED_INSIDE_COLOR",
    "TOGGLE_PRESSED_TEXT_COLOR",
    "TOGGLE_ACTIVE_BORDER_COLOR",
    "TOGGLE_ACTIVE_INSIDE_COLOR",
    "TOGGLE_ACTIVE_TEXT_COLOR",
    "TOGGLEGROUP_PADDING",
    "SLIDER_BORDER_WIDTH",
    "SLIDER_BUTTON_BORDER_WIDTH",
    "SLIDER_BORDER_COLOR",
    "SLIDER_INSIDE_COLOR",
    "SLIDER_DEFAULT_COLOR",
    "SLIDER_HOVER_COLOR",
    "SLIDER_ACTIVE_COLOR",
    "SLIDERBAR_BORDER_COLOR",
    "SLIDERBAR_INSIDE_COLOR",
    "SLIDERBAR_DEFAULT_COLOR",
    "SLIDERBAR_HOVER_COLOR",
    "SLIDERBAR_ACTIVE_COLOR",
    "SLIDERBAR_ZERO_LINE_COLOR",
    "PROGRESSBAR_BORDER_COLOR",
    "PROGRESSBAR_INSIDE_COLOR",
    "PROGRESSBAR_PROGRESS_COLOR",
    "PROGRESSBAR_BORDER_WIDTH",
    "SPINNER_LABEL_BORDER_COLOR",
    "SPINNER_LABEL_INSIDE_COLOR",
    "SPINNER_DEFAULT_BUTTON_BORDER_COLOR",
    "SPINNER_DEFAULT_BUTTON_INSIDE_COLOR",
    "SPINNER_DEFAULT_SYMBOL_COLOR",
    "SPINNER_DEFAULT_TEXT_COLOR",
    "SPINNER_HOVER_BUTTON_BORDER_COLOR",
    "SPINNER_HOVER_BUTTON_INSIDE_COLOR",
    "SPINNER_HOVER_SYMBOL_COLOR",
    "SPINNER_HOVER_TEXT_COLOR",
    "SPINNER_PRESSED_BUTTON_BORDER_COLOR",
    "SPINNER_PRESSED_BUTTON_INSIDE_COLOR",
    "SPINNER_PRESSED_SYMBOL_COLOR",
    "SPINNER_PRESSED_TEXT_COLOR",
    "COMBOBOX_PADDING",
    "COMBOBOX_BUTTON_WIDTH",
    "COMBOBOX_BUTTON_HEIGHT",
    "COMBOBOX_BORDER_WIDTH",
    "COMBOBOX_DEFAULT_BORDER_COLOR",
    "COMBOBOX_DEFAULT_INSIDE_COLOR",
    "COMBOBOX_DEFAULT_TEXT_COLOR",
    "COMBOBOX_DEFAULT_LIST_TEXT_COLOR",
    "COMBOBOX_HOVER_BORDER_COLOR",
    "COMBOBOX_HOVER_INSIDE_COLOR",
    "COMBOBOX_HOVER_TEXT_COLOR",
    "COMBOBOX_HOVER_LIST_TEXT_COLOR",
    "COMBOBOX_PRESSED_BORDER_COLOR",
    "COMBOBOX_PRESSED_INSIDE_COLOR",
    "COMBOBOX_PRESSED_TEXT_COLOR",
    "COMBOBOX_PRESSED_LIST_BORDER_COLOR",
    "COMBOBOX_PRESSED_LIST_INSIDE_COLOR",
    "COMBOBOX_PRESSED_LIST_TEXT_COLOR",
    "CHECKBOX_DEFAULT_BORDER_COLOR",
    "CHECKBOX_DEFAULT_INSIDE_COLOR",
    "CHECKBOX_HOVER_BORDER_COLOR",
    "CHECKBOX_HOVER_INSIDE_COLOR",
    "CHECKBOX_CLICK_BORDER_COLOR",
    "CHECKBOX_CLICK_INSIDE_COLOR",
    "CHECKBOX_STATUS_ACTIVE_COLOR",
    "CHECKBOX_INSIDE_WIDTH",
    "TEXTBOX_BORDER_WIDTH",
    "TEXTBOX_BORDER_COLOR",
    "TEXTBOX_INSIDE_COLOR",
    "TEXTBOX_TEXT_COLOR",
    "TEXTBOX_LINE_COLOR",
    "TEXTBOX_TEXT_FONTSIZE"
};

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void GuiLabel(Rectangle bounds, const char *text);                                                  // Label element, show text
void GuiLabelEx(Rectangle bounds, const char *text, Color textColor, Color border, Color inner);    // Label element extended, configurable colors
bool GuiButton(Rectangle bounds, const char *text);                                                 // Button element, returns true when clicked
bool GuiToggleButton(Rectangle bounds, const char *text, bool toggle);                              // Toggle Button element, returns true when active
int GuiToggleGroup(Rectangle bounds, int toggleNum, char **toggleText, int toggleActive);           // Toggle Group element, returns toggled button index
int GuiComboBox(Rectangle bounds, int comboNum, char **comboText, int comboActive);                 // Combo Box element, returns selected item index
bool GuiCheckBox(Rectangle bounds, const char *text, bool checked);                                 // Check Box element, returns true when active
float GuiSlider(Rectangle bounds, float value, float minValue, float maxValue);                     // Slider element, returns selected value
float GuiSliderBar(Rectangle bounds, float value, float minValue, float maxValue);                  // Slider Bar element, returns selected value
void GuiProgressBar(Rectangle bounds, float value);                                                 // Progress Bar element, shows current progress value
int GuiSpinner(Rectangle bounds, int value, int minValue, int maxValue);                            // Spinner element, returns selected value
char *GuiTextBox(Rectangle bounds, char *text);                                                     // Text Box element, returns input text

void SaveGuiStyle(const char *fileName);                        // Save GUI style file
void LoadGuiStyle(const char *fileName);                        // Load GUI style file

void SetStyleProperty(int guiProperty, int value);              // Set one style property
int GetStyleProperty(int guiProperty);                          // Get one style property

#ifdef __cplusplus
}
#endif

#endif // RAYGUI_H
