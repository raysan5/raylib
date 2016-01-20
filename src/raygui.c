/**********************************************************************************************
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

#include "raygui.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>     // Required for malloc(), free()
#include <string.h>     // Required for strcmp()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// GUI elements states
typedef enum { BUTTON_DEFAULT, BUTTON_HOVER, BUTTON_PRESSED, BUTTON_CLICKED } ButtonState;
typedef enum { TOGGLE_UNACTIVE, TOGGLE_HOVER, TOGGLE_PRESSED, TOGGLE_ACTIVE } ToggleState;
typedef enum { COMBOBOX_UNACTIVE, COMBOBOX_HOVER, COMBOBOX_PRESSED, COMBOBOX_ACTIVE } ComboBoxState;
typedef enum { SPINNER_DEFAULT, SPINNER_HOVER, SPINNER_PRESSED } SpinnerState;
typedef enum { CHECKBOX_STATUS, CHECKBOX_HOVER, CHECKBOX_PRESSED } CheckBoxState;
typedef enum { SLIDER_DEFAULT, SLIDER_HOVER, SLIDER_ACTIVE } SliderState;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

//Current GUI style (default light)
static int style[NUM_PROPERTIES] = {
    0xf5f5f5ff,         // GLOBAL_BASE_COLOR,
    0xf5f5f5ff,         // GLOBAL_BORDER_COLOR,
    0xf5f5f5ff,         // GLOBAL_TEXT_COLOR,
    10,                 // GLOBAL_TEXT_FONTSIZE
    1,                  // GLOBAL_BORDER_WIDTH
    0xf5f5f5ff,         // BACKGROUND_COLOR
    1,                  // LABEL_BORDER_WIDTH
    0x000000ff,         // LABEL_TEXT_COLOR
    20,                 // LABEL_TEXT_PADDING
    2,                  // BUTTON_BORDER_WIDTH
    20,                 // BUTTON_TEXT_PADDING
    0x828282ff,         // BUTTON_DEFAULT_BORDER_COLOR
    0xc8c8c8ff,         // BUTTON_DEFAULT_INSIDE_COLOR
    0x000000ff,         // BUTTON_DEFAULT_TEXT_COLOR
    0xc8c8c8ff,         // BUTTON_HOVER_BORDER_COLOR
    0xffffffff,         // BUTTON_HOVER_INSIDE_COLOR
    0x000000ff,         // BUTTON_HOVER_TEXT_COLOR
    0x7bb0d6ff,         // BUTTON_PRESSED_BORDER_COLOR
    0xbcecffff,         // BUTTON_PRESSED_INSIDE_COLOR
    0x5f9aa7ff,         // BUTTON_PRESSED_TEXT_COLOR
    20,                 // TOGGLE_TEXT_PADDING
    1,                  // TOGGLE_BORDER_WIDTH
    0x828282ff,         // TOGGLE_DEFAULT_BORDER_COLOR
    0xc8c8c8ff,         // TOGGLE_DEFAULT_INSIDE_COLOR
    0x828282ff,         // TOGGLE_DEFAULT_TEXT_COLOR
    0xc8c8c8ff,         // TOGGLE_HOVER_BORDER_COLOR
    0xffffffff,         // TOGGLE_HOVER_INSIDE_COLOR
    0x828282ff,         // TOGGLE_HOVER_TEXT_COLOR
    0xbdd7eaff,         // TOGGLE_PRESSED_BORDER_COLOR
    0xddf5ffff,         // TOGGLE_PRESSED_INSIDE_COLOR
    0xafccd3ff,         // TOGGLE_PRESSED_TEXT_COLOR
    0x7bb0d6ff,         // TOGGLE_ACTIVE_BORDER_COLOR
    0xbcecffff,         // TOGGLE_ACTIVE_INSIDE_COLOR
    0x5f9aa7ff,         // TOGGLE_ACTIVE_TEXT_COLOR
    3,                  // TOGGLEGROUP_PADDING
    1,                  // SLIDER_BORDER_WIDTH
    1,                  // SLIDER_BUTTON_BORDER_WIDTH
    0x828282ff,         // SLIDER_BORDER_COLOR
    0xc8c8c8ff,         // SLIDER_INSIDE_COLOR
    0xbcecffff,         // SLIDER_DEFAULT_COLOR
    0xffffffff,         // SLIDER_HOVER_COLOR
    0xddf5ffff,         // SLIDER_ACTIVE_COLOR
    0x828282ff,         // SLIDERBAR_BORDER_COLOR
    0xc8c8c8ff,         // SLIDERBAR_INSIDE_COLOR
    0xbcecffff,         // SLIDERBAR_DEFAULT_COLOR
    0xffffffff,         // SLIDERBAR_HOVER_COLOR
    0xddf5ffff,         // SLIDERBAR_ACTIVE_COLOR
    0x828282ff,         // SLIDERBAR_ZERO_LINE_COLOR
    0x828282ff,         // PROGRESSBAR_BORDER_COLOR
    0xc8c8c8ff,         // PROGRESSBAR_INSIDE_COLOR
    0xbcecffff,         // PROGRESSBAR_PROGRESS_COLOR
    2,                  // PROGRESSBAR_BORDER_WIDTH
    0x828282ff,         // SPINNER_LABEL_BORDER_COLOR
    0xc8c8c8ff,         // SPINNER_LABEL_INSIDE_COLOR
    0x828282ff,         // SPINNER_DEFAULT_BUTTON_BORDER_COLOR
    0xc8c8c8ff,         // SPINNER_DEFAULT_BUTTON_INSIDE_COLOR
    0x000000ff,         // SPINNER_DEFAULT_SYMBOL_COLOR
    0x000000ff,         // SPINNER_DEFAULT_TEXT_COLOR
    0xc8c8c8ff,         // SPINNER_HOVER_BUTTON_BORDER_COLOR
    0xffffffff,         // SPINNER_HOVER_BUTTON_INSIDE_COLOR
    0x000000ff,         // SPINNER_HOVER_SYMBOL_COLOR
    0x000000ff,         // SPINNER_HOVER_TEXT_COLOR
    0x7bb0d6ff,         // SPINNER_PRESSED_BUTTON_BORDER_COLOR
    0xbcecffff,         // SPINNER_PRESSED_BUTTON_INSIDE_COLOR
    0x5f9aa7ff,         // SPINNER_PRESSED_SYMBOL_COLOR
    0x000000ff,         // SPINNER_PRESSED_TEXT_COLOR
    1,                  // COMBOBOX_PADDING
    30,                 // COMBOBOX_BUTTON_WIDTH
    30,                 // COMBOBOX_BUTTON_HEIGHT
    1,                  // COMBOBOX_BORDER_WIDTH
    0x828282ff,         // COMBOBOX_DEFAULT_BORDER_COLOR
    0xc8c8c8ff,         // COMBOBOX_DEFAULT_INSIDE_COLOR
    0x828282ff,         // COMBOBOX_DEFAULT_TEXT_COLOR
    0x828282ff,         // COMBOBOX_DEFAULT_LIST_TEXT_COLOR
    0xc8c8c8ff,         // COMBOBOX_HOVER_BORDER_COLOR
    0xffffffff,         // COMBOBOX_HOVER_INSIDE_COLOR
    0x828282ff,         // COMBOBOX_HOVER_TEXT_COLOR
    0x828282ff,         // COMBOBOX_HOVER_LIST_TEXT_COLOR
    0x7bb0d6ff,         // COMBOBOX_PRESSED_BORDER_COLOR
    0xbcecffff,         // COMBOBOX_PRESSED_INSIDE_COLOR
    0x5f9aa7ff,         // COMBOBOX_PRESSED_TEXT_COLOR
    0x0078acff,         // COMBOBOX_PRESSED_LIST_BORDER_COLOR
    0x66e7ffff,         // COMBOBOX_PRESSED_LIST_INSIDE_COLOR
    0x0078acff,         // COMBOBOX_PRESSED_LIST_TEXT_COLOR
    0x828282ff,         // CHECKBOX_DEFAULT_BORDER_COLOR
    0xffffffff,         // CHECKBOX_DEFAULT_INSIDE_COLOR
    0xc8c8c8ff,         // CHECKBOX_HOVER_BORDER_COLOR
    0xffffffff,         // CHECKBOX_HOVER_INSIDE_COLOR
    0x66e7ffff,         // CHECKBOX_CLICK_BORDER_COLOR
    0xddf5ffff,         // CHECKBOX_CLICK_INSIDE_COLOR
    0x7bb0d6ff,         // CHECKBOX_STATUS_ACTIVE_COLOR
    4,                  // CHECKBOX_INSIDE_WIDTH
    1,                  // TEXTBOX_BORDER_WIDTH
    0x828282ff,         // TEXTBOX_BORDER_COLOR
    0xf5f5f5ff,         // TEXTBOX_INSIDE_COLOR
    0x000000ff,         // TEXTBOX_TEXT_COLOR
    0x000000ff,         // TEXTBOX_LINE_COLOR
    10                  // TEXTBOX_TEXT_FONTSIZE
};

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static Color ColorMultiply(Color baseColor, float value);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Label element, show text
void GuiLabel(Rectangle bounds, const char *text)
{
    GuiLabelEx(bounds,text, GetColor(style[LABEL_TEXT_COLOR]), BLANK, BLANK);
}

// Label element extended, configurable colors
void GuiLabelEx(Rectangle bounds, const char *text, Color textColor, Color border, Color inner)
{
    // Update control
    //--------------------------------------------------------------------
    int textWidth = MeasureText(text, style[GLOBAL_TEXT_FONTSIZE]);
    int textHeight = GetDefaultFont().size;

    if (bounds.width < textWidth) bounds.width = textWidth + style[LABEL_TEXT_PADDING];
    if (bounds.height < textHeight) bounds.height = textHeight + style[LABEL_TEXT_PADDING]/2;
    //--------------------------------------------------------------------
    
    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleRec(bounds, border);
    DrawRectangle(bounds.x + style[LABEL_BORDER_WIDTH], bounds.y + style[LABEL_BORDER_WIDTH], bounds.width - (2 * style[LABEL_BORDER_WIDTH]), bounds.height - (2 * style[LABEL_BORDER_WIDTH]), inner);
    DrawText(text, bounds.x + ((bounds.width/2) - (textWidth/2)), bounds.y + ((bounds.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], textColor);
    //--------------------------------------------------------------------
}

// Button element, returns true when clicked
bool GuiButton(Rectangle bounds, const char *text)
{
    ButtonState buttonState = BUTTON_DEFAULT;
    Vector2 mousePoint = GetMousePosition();
    
    int textWidth = MeasureText(text, style[GLOBAL_TEXT_FONTSIZE]);
    int textHeight = GetDefaultFont().size;
    
    // Update control
    //--------------------------------------------------------------------
    if (bounds.width < textWidth) bounds.width = textWidth + style[BUTTON_TEXT_PADDING];
    if (bounds.height < textHeight) bounds.height = textHeight + style[BUTTON_TEXT_PADDING]/2;
    
    if (CheckCollisionPointRec(mousePoint, bounds))
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) buttonState = BUTTON_PRESSED;
        else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) buttonState = BUTTON_CLICKED;
        else buttonState = BUTTON_HOVER;
    }
    //--------------------------------------------------------------------
    
    // Draw control
    //--------------------------------------------------------------------
    switch (buttonState)
    {
        case BUTTON_DEFAULT:
        {
            DrawRectangleRec(bounds, GetColor(style[BUTTON_DEFAULT_BORDER_COLOR]));
            DrawRectangle((int)(bounds.x + style[BUTTON_BORDER_WIDTH]), (int)(bounds.y + style[BUTTON_BORDER_WIDTH]) , (int)(bounds.width - (2 * style[BUTTON_BORDER_WIDTH])), (int)(bounds.height - (2 * style[BUTTON_BORDER_WIDTH])), GetColor(style[BUTTON_DEFAULT_INSIDE_COLOR]));
            DrawText(text, bounds.x + ((bounds.width/2) - (MeasureText(text, style[GLOBAL_TEXT_FONTSIZE])/2)), bounds.y + ((bounds.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[BUTTON_DEFAULT_TEXT_COLOR]));
        } break;
        case BUTTON_HOVER:
        {
            DrawRectangleRec(bounds, GetColor(style[BUTTON_HOVER_BORDER_COLOR]));
            DrawRectangle((int)(bounds.x + style[BUTTON_BORDER_WIDTH]), (int)(bounds.y + style[BUTTON_BORDER_WIDTH]) , (int)(bounds.width - (2 * style[BUTTON_BORDER_WIDTH])), (int)(bounds.height - (2 * style[BUTTON_BORDER_WIDTH])), GetColor(style[BUTTON_HOVER_INSIDE_COLOR]));
            DrawText(text, bounds.x + ((bounds.width/2) - (MeasureText(text, style[GLOBAL_TEXT_FONTSIZE])/2)), bounds.y + ((bounds.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[BUTTON_HOVER_TEXT_COLOR]));
        } break;
        case BUTTON_PRESSED:
        {
            DrawRectangleRec(bounds, GetColor(style[BUTTON_PRESSED_BORDER_COLOR]));
            DrawRectangle((int)(bounds.x + style[BUTTON_BORDER_WIDTH]), (int)(bounds.y + style[BUTTON_BORDER_WIDTH]) , (int)(bounds.width - (2 * style[BUTTON_BORDER_WIDTH])), (int)(bounds.height - (2 * style[BUTTON_BORDER_WIDTH])), GetColor(style[BUTTON_PRESSED_INSIDE_COLOR]));
            DrawText(text, bounds.x + ((bounds.width/2) - (MeasureText(text, style[GLOBAL_TEXT_FONTSIZE])/2)), bounds.y + ((bounds.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[BUTTON_PRESSED_TEXT_COLOR]));
        } break;
        case BUTTON_CLICKED:
        {
            DrawRectangleRec(bounds, GetColor(style[BUTTON_PRESSED_BORDER_COLOR]));
            DrawRectangle((int)(bounds.x + style[BUTTON_BORDER_WIDTH]), (int)(bounds.y + style[BUTTON_BORDER_WIDTH]) , (int)(bounds.width - (2 * style[BUTTON_BORDER_WIDTH])), (int)(bounds.height - (2 * style[BUTTON_BORDER_WIDTH])), GetColor(style[BUTTON_PRESSED_INSIDE_COLOR]));
        } break;
        default: break;
    }
    //------------------------------------------------------------------
    
    if (buttonState == BUTTON_CLICKED) return true;
    else return false;
}

// Toggle Button element, returns true when active
bool GuiToggleButton(Rectangle bounds, const char *text, bool toggle)
{
    ToggleState toggleState = TOGGLE_UNACTIVE;
    Rectangle toggleButton = bounds;
    Vector2 mousePoint = GetMousePosition();
    
    int textWidth = MeasureText(text, style[GLOBAL_TEXT_FONTSIZE]);
    int textHeight = GetDefaultFont().size;
    
    // Update control
    //--------------------------------------------------------------------   
    if (toggleButton.width < textWidth) toggleButton.width = textWidth + style[TOGGLE_TEXT_PADDING];
    if (toggleButton.height < textHeight) toggleButton.height = textHeight + style[TOGGLE_TEXT_PADDING]/2;
    if (CheckCollisionPointRec(mousePoint, toggleButton))
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) toggleState = TOGGLE_PRESSED;
        else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) toggleState = TOGGLE_ACTIVE;
        else toggleState = TOGGLE_HOVER;
    }
     
    if (toggleState == TOGGLE_ACTIVE && !toggle) toggle = true;
    if (toggle) toggleState = TOGGLE_ACTIVE;
    //--------------------------------------------------------------------   
    
    // Draw control
    //--------------------------------------------------------------------
    switch (toggleState)
    {
        case TOGGLE_UNACTIVE:
        {
            DrawRectangleRec(toggleButton, GetColor(style[TOGGLE_DEFAULT_BORDER_COLOR]));
            DrawRectangle((int)(toggleButton.x + style[TOGGLE_BORDER_WIDTH]), (int)(toggleButton.y + style[TOGGLE_BORDER_WIDTH]) , (int)(toggleButton.width - (2 * style[TOGGLE_BORDER_WIDTH])), (int)(toggleButton.height - (2 * style[TOGGLE_BORDER_WIDTH])), GetColor(style[TOGGLE_DEFAULT_INSIDE_COLOR]));
            DrawText(text, toggleButton.x + ((toggleButton.width/2) - (MeasureText(text, style[GLOBAL_TEXT_FONTSIZE])/2)), toggleButton.y + ((toggleButton.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[TOGGLE_DEFAULT_TEXT_COLOR]));
        } break;
        case TOGGLE_HOVER:
        {
            DrawRectangleRec(toggleButton, GetColor(style[TOGGLE_HOVER_BORDER_COLOR]));
            DrawRectangle((int)(toggleButton.x + style[TOGGLE_BORDER_WIDTH]), (int)(toggleButton.y + style[TOGGLE_BORDER_WIDTH]) , (int)(toggleButton.width - (2 * style[TOGGLE_BORDER_WIDTH])), (int)(toggleButton.height - (2 * style[TOGGLE_BORDER_WIDTH])), GetColor(style[TOGGLE_HOVER_INSIDE_COLOR]));
            DrawText(text, toggleButton.x + ((toggleButton.width/2) - (MeasureText(text, style[GLOBAL_TEXT_FONTSIZE])/2)), toggleButton.y + ((toggleButton.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[TOGGLE_HOVER_TEXT_COLOR]));
        } break;
        case TOGGLE_PRESSED:
        {
            DrawRectangleRec(toggleButton, GetColor(style[TOGGLE_PRESSED_BORDER_COLOR]));
            DrawRectangle((int)(toggleButton.x + style[TOGGLE_BORDER_WIDTH]), (int)(toggleButton.y + style[TOGGLE_BORDER_WIDTH]) , (int)(toggleButton.width - (2 * style[TOGGLE_BORDER_WIDTH])), (int)(toggleButton.height - (2 * style[TOGGLE_BORDER_WIDTH])), GetColor(style[TOGGLE_PRESSED_INSIDE_COLOR]));
            DrawText(text, toggleButton.x + ((toggleButton.width/2) - (MeasureText(text, style[GLOBAL_TEXT_FONTSIZE])/2)), toggleButton.y + ((toggleButton.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[TOGGLE_PRESSED_TEXT_COLOR]));
        } break;
        case TOGGLE_ACTIVE:
        {
            DrawRectangleRec(toggleButton, GetColor(style[TOGGLE_ACTIVE_BORDER_COLOR]));
            DrawRectangle((int)(toggleButton.x + style[TOGGLE_BORDER_WIDTH]), (int)(toggleButton.y + style[TOGGLE_BORDER_WIDTH]) , (int)(toggleButton.width - (2 * style[TOGGLE_BORDER_WIDTH])), (int)(toggleButton.height - (2 * style[TOGGLE_BORDER_WIDTH])), GetColor(style[TOGGLE_ACTIVE_INSIDE_COLOR]));
            DrawText(text, toggleButton.x + ((toggleButton.width/2) - (MeasureText(text, style[GLOBAL_TEXT_FONTSIZE])/2)), toggleButton.y + ((toggleButton.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[TOGGLE_ACTIVE_TEXT_COLOR]));
        } break;
        default: break;
    }
    //-------------------------------------------------------------------- 
    
    return toggle;
}

// Toggle Group element, returns toggled button index
int GuiToggleGroup(Rectangle bounds, int toggleNum, char **toggleText, int toggleActive)
{ 
    for (int i = 0; i < toggleNum; i++)
    {
        if (i == toggleActive) GuiToggleButton((Rectangle){bounds.x + i*(bounds.width + style[TOGGLEGROUP_PADDING]),bounds.y,bounds.width,bounds.height}, toggleText[i], true);      
        else if (GuiToggleButton((Rectangle){bounds.x + i*(bounds.width + style[TOGGLEGROUP_PADDING]),bounds.y,bounds.width,bounds.height}, toggleText[i], false) == true) toggleActive = i;
    }
    
    return toggleActive;
}

// Combo Box element, returns selected item index
int GuiComboBox(Rectangle bounds, int comboNum, char **comboText, int comboActive)
{
    ComboBoxState comboBoxState = COMBOBOX_UNACTIVE;
    Rectangle comboBoxButton = bounds;
    Rectangle click = { bounds.x + bounds.width + style[COMBOBOX_PADDING], bounds.y, style[COMBOBOX_BUTTON_WIDTH], style[COMBOBOX_BUTTON_HEIGHT] };
    Vector2 mousePoint = GetMousePosition();

    int textHeight = GetDefaultFont().size;
     
    for (int i = 0; i < comboNum; i++)
    {
        if (i == comboActive)
        {
		    // Update control
    		//-------------------------------------------------------------------- 
            int textWidth = MeasureText(comboText[i], style[GLOBAL_TEXT_FONTSIZE]);
                
            if (comboBoxButton.width < textWidth) comboBoxButton.width = textWidth + style[TOGGLE_TEXT_PADDING];
            if (comboBoxButton.height < textHeight) comboBoxButton.height = textHeight + style[TOGGLE_TEXT_PADDING]/2;
            
            if (CheckCollisionPointRec(mousePoint, comboBoxButton) || CheckCollisionPointRec(mousePoint, click))
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) comboBoxState = COMBOBOX_PRESSED;
                else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) comboBoxState = COMBOBOX_ACTIVE;
                else comboBoxState = COMBOBOX_HOVER;
            }
    		//--------------------------------------------------------------------   
            
            // Draw control
            //--------------------------------------------------------------------
            switch (comboBoxState)
            {
                case COMBOBOX_UNACTIVE:
                {
                    DrawRectangleRec(comboBoxButton, GetColor(style[COMBOBOX_DEFAULT_BORDER_COLOR]));
                    DrawRectangle((int)(comboBoxButton.x + style[COMBOBOX_BORDER_WIDTH]), (int)(comboBoxButton.y + style[COMBOBOX_BORDER_WIDTH]), (int)(comboBoxButton.width - (2 * style[COMBOBOX_BORDER_WIDTH])), (int)(comboBoxButton.height - (2*style[COMBOBOX_BORDER_WIDTH])), GetColor(style[COMBOBOX_DEFAULT_INSIDE_COLOR]));
                
                    DrawRectangleRec(click, GetColor(style[COMBOBOX_DEFAULT_BORDER_COLOR]));
                    DrawRectangle((int)(click.x + style[COMBOBOX_BORDER_WIDTH]), (int)(click.y + style[COMBOBOX_BORDER_WIDTH]) , (int)(click.width - (2*style[COMBOBOX_BORDER_WIDTH])), (int)(click.height - (2*style[COMBOBOX_BORDER_WIDTH])), GetColor(style[COMBOBOX_DEFAULT_INSIDE_COLOR]));
                    DrawText(FormatText("%i/%i", comboActive + 1, comboNum), click.x + ((click.width/2) - (MeasureText(FormatText("%i/%i", comboActive + 1, comboNum), style[GLOBAL_TEXT_FONTSIZE])/2)), click.y + ((click.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[COMBOBOX_DEFAULT_LIST_TEXT_COLOR]));
                    DrawText(comboText[i], comboBoxButton.x + ((comboBoxButton.width/2) - (MeasureText(comboText[i], style[GLOBAL_TEXT_FONTSIZE])/2)), comboBoxButton.y + ((comboBoxButton.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[COMBOBOX_DEFAULT_TEXT_COLOR]));
                } break;
                case COMBOBOX_HOVER:
                {
                    DrawRectangleRec(comboBoxButton, GetColor(style[COMBOBOX_HOVER_BORDER_COLOR]));
                    DrawRectangle((int)(comboBoxButton.x + style[COMBOBOX_BORDER_WIDTH]), (int)(comboBoxButton.y + style[COMBOBOX_BORDER_WIDTH]), (int)(comboBoxButton.width - (2 * style[COMBOBOX_BORDER_WIDTH])), (int)(comboBoxButton.height - (2*style[COMBOBOX_BORDER_WIDTH])), GetColor(style[COMBOBOX_HOVER_INSIDE_COLOR]));
                    
                    DrawRectangleRec(click, GetColor(style[COMBOBOX_HOVER_BORDER_COLOR]));
                    DrawRectangle((int)(click.x + style[COMBOBOX_BORDER_WIDTH]), (int)(click.y + style[COMBOBOX_BORDER_WIDTH]) , (int)(click.width - (2*style[COMBOBOX_BORDER_WIDTH])), (int)(click.height - (2*style[COMBOBOX_BORDER_WIDTH])), GetColor(style[COMBOBOX_HOVER_INSIDE_COLOR]));
                    DrawText(FormatText("%i/%i", comboActive + 1, comboNum), click.x + ((click.width/2) - (MeasureText(FormatText("%i/%i", comboActive + 1, comboNum), style[GLOBAL_TEXT_FONTSIZE])/2)), click.y + ((click.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[COMBOBOX_HOVER_LIST_TEXT_COLOR]));
                    DrawText(comboText[i], comboBoxButton.x + ((comboBoxButton.width/2) - (MeasureText(comboText[i], style[GLOBAL_TEXT_FONTSIZE])/2)), comboBoxButton.y + ((comboBoxButton.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[COMBOBOX_HOVER_TEXT_COLOR]));
                } break;
                case COMBOBOX_PRESSED:
                {
                    DrawRectangleRec(comboBoxButton, GetColor(style[COMBOBOX_PRESSED_BORDER_COLOR]));
                    DrawRectangle((int)(comboBoxButton.x + style[COMBOBOX_BORDER_WIDTH]), (int)(comboBoxButton.y + style[COMBOBOX_BORDER_WIDTH]), (int)(comboBoxButton.width - (2 * style[COMBOBOX_BORDER_WIDTH])), (int)(comboBoxButton.height - (2*style[COMBOBOX_BORDER_WIDTH])), GetColor(style[COMBOBOX_PRESSED_INSIDE_COLOR]));
                
                    DrawRectangleRec(click, GetColor(style[COMBOBOX_PRESSED_LIST_BORDER_COLOR]));
                    DrawRectangle((int)(click.x + style[COMBOBOX_BORDER_WIDTH]), (int)(click.y + style[COMBOBOX_BORDER_WIDTH]) , (int)(click.width - (2*style[COMBOBOX_BORDER_WIDTH])), (int)(click.height - (2*style[COMBOBOX_BORDER_WIDTH])), GetColor(style[COMBOBOX_PRESSED_LIST_INSIDE_COLOR]));
                    DrawText(FormatText("%i/%i", comboActive + 1, comboNum), click.x + ((click.width/2) - (MeasureText(FormatText("%i/%i", comboActive + 1, comboNum), style[GLOBAL_TEXT_FONTSIZE])/2)), click.y + ((click.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[COMBOBOX_PRESSED_LIST_TEXT_COLOR]));
                    DrawText(comboText[i], comboBoxButton.x + ((comboBoxButton.width/2) - (MeasureText(comboText[i], style[GLOBAL_TEXT_FONTSIZE])/2)), comboBoxButton.y + ((comboBoxButton.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[COMBOBOX_PRESSED_TEXT_COLOR]));
                } break;
                case COMBOBOX_ACTIVE:
                {
                    DrawRectangleRec(comboBoxButton, GetColor(style[COMBOBOX_PRESSED_BORDER_COLOR]));
                    DrawRectangle((int)(comboBoxButton.x + style[COMBOBOX_BORDER_WIDTH]), (int)(comboBoxButton.y + style[COMBOBOX_BORDER_WIDTH]), (int)(comboBoxButton.width - (2 * style[COMBOBOX_BORDER_WIDTH])), (int)(comboBoxButton.height - (2*style[COMBOBOX_BORDER_WIDTH])), GetColor(style[COMBOBOX_PRESSED_INSIDE_COLOR]));
                
                    DrawRectangleRec(click, GetColor(style[COMBOBOX_PRESSED_LIST_BORDER_COLOR]));
                    DrawRectangle((int)(click.x + style[COMBOBOX_BORDER_WIDTH]), (int)(click.y + style[COMBOBOX_BORDER_WIDTH]) , (int)(click.width - (2*style[COMBOBOX_BORDER_WIDTH])), (int)(click.height - (2*style[COMBOBOX_BORDER_WIDTH])), GetColor(style[COMBOBOX_PRESSED_LIST_INSIDE_COLOR]));
                    DrawText(FormatText("%i/%i", comboActive + 1, comboNum), click.x + ((click.width/2) - (MeasureText(FormatText("%i/%i", comboActive + 1, comboNum), style[GLOBAL_TEXT_FONTSIZE])/2)), click.y + ((click.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[COMBOBOX_PRESSED_LIST_TEXT_COLOR]));
                    DrawText(comboText[i], comboBoxButton.x + ((comboBoxButton.width/2) - (MeasureText(comboText[i], style[GLOBAL_TEXT_FONTSIZE])/2)), comboBoxButton.y + ((comboBoxButton.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[COMBOBOX_PRESSED_TEXT_COLOR]));
                } break;
                default: break;
            }
            
            //DrawText(comboText[i], comboBoxButton.x + ((comboBoxButton.width/2) - (MeasureText(comboText[i], style[]globalTextFontSize)/2)), comboBoxButton.y + ((comboBoxButton.height/2) - (style[]globalTextFontSize/2)), style[]globalTextFontSize, COMBOBOX_PRESSED_TEXT_COLOR);            
            //-------------------------------------------------------------------- 
        }
    }
    
    if (CheckCollisionPointRec(GetMousePosition(), bounds) || CheckCollisionPointRec(GetMousePosition(), click))
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
        {
           comboActive += 1; 
           if(comboActive >= comboNum) comboActive = 0;
        }
    } 

    return comboActive;
}

// Check Box element, returns true when active
bool GuiCheckBox(Rectangle checkBoxBounds, const char *text, bool checked)
{
    CheckBoxState checkBoxState = CHECKBOX_STATUS;
    Vector2 mousePoint = GetMousePosition();
    
    // Update control
    //-------------------------------------------------------------------- 
    if (CheckCollisionPointRec(mousePoint, checkBoxBounds))
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) checkBoxState = CHECKBOX_PRESSED;
        else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            checkBoxState = CHECKBOX_STATUS;
            checked = !checked;
        } 
        else checkBoxState = CHECKBOX_HOVER;
    }
    //-------------------------------------------------------------------- 
    
    // Draw control
    //--------------------------------------------------------------------
    switch (checkBoxState)
    {
        case CHECKBOX_HOVER:
        {
            DrawRectangleRec(checkBoxBounds, GetColor(style[CHECKBOX_HOVER_BORDER_COLOR]));
            DrawRectangle((int)(checkBoxBounds.x + style[TOGGLE_BORDER_WIDTH]), (int)(checkBoxBounds.y + style[TOGGLE_BORDER_WIDTH]), (int)(checkBoxBounds.width - (2*style[TOGGLE_BORDER_WIDTH])), (int)(checkBoxBounds.height - (2*style[TOGGLE_BORDER_WIDTH])), GetColor(style[CHECKBOX_HOVER_INSIDE_COLOR]));        
        } break;
        case CHECKBOX_STATUS:
        { 
            DrawRectangleRec(checkBoxBounds, GetColor(style[CHECKBOX_DEFAULT_BORDER_COLOR]));
            DrawRectangle((int)(checkBoxBounds.x + style[TOGGLE_BORDER_WIDTH]), (int)(checkBoxBounds.y + style[TOGGLE_BORDER_WIDTH]), (int)(checkBoxBounds.width - (2*style[TOGGLE_BORDER_WIDTH])), (int)(checkBoxBounds.height - (2*style[TOGGLE_BORDER_WIDTH])), GetColor(style[CHECKBOX_DEFAULT_INSIDE_COLOR]));
        } break; 
        case CHECKBOX_PRESSED:
        {
            DrawRectangleRec(checkBoxBounds, GetColor(style[CHECKBOX_CLICK_BORDER_COLOR]));
            DrawRectangle((int)(checkBoxBounds.x + style[TOGGLE_BORDER_WIDTH]), (int)(checkBoxBounds.y + style[TOGGLE_BORDER_WIDTH]), (int)(checkBoxBounds.width - (2*style[TOGGLE_BORDER_WIDTH])), (int)(checkBoxBounds.height - (2*style[TOGGLE_BORDER_WIDTH])), GetColor(style[CHECKBOX_CLICK_INSIDE_COLOR]));
        } break;
        default: break;               
    }
    
    if (text != NULL) DrawText(text, checkBoxBounds.x + checkBoxBounds.width + 2, checkBoxBounds.y + ((checkBoxBounds.height/2) - (style[GLOBAL_TEXT_FONTSIZE]/2) + 1), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[LABEL_TEXT_COLOR]));

    if (checked)
    {
        DrawRectangle((int)(checkBoxBounds.x + style[CHECKBOX_INSIDE_WIDTH]), (int)(checkBoxBounds.y + style[CHECKBOX_INSIDE_WIDTH]), (int)(checkBoxBounds.width - (2*style[CHECKBOX_INSIDE_WIDTH])), (int)(checkBoxBounds.height - (2*style[CHECKBOX_INSIDE_WIDTH])), GetColor(style[CHECKBOX_STATUS_ACTIVE_COLOR]));
    }
    //--------------------------------------------------------------------
    
    return checked;
}

// Slider element, returns selected value
float GuiSlider(Rectangle bounds, float value, float minValue, float maxValue)
{
    SliderState sliderState = SLIDER_DEFAULT;
    float buttonTravelDistance = 0;
    float sliderPos = 0;
    Vector2 mousePoint = GetMousePosition();
    
    // Update control
    //--------------------------------------------------------------------     
    if (value < minValue) value = minValue;
    else if (value >= maxValue) value = maxValue;
    
    sliderPos = (value - minValue)/(maxValue - minValue);
    
    Rectangle sliderButton;
    sliderButton.width = ((int)(bounds.width - (2 * style[SLIDER_BUTTON_BORDER_WIDTH]))/10 - 8);
    sliderButton.height =((int)(bounds.height - ( 2 * style[SLIDER_BORDER_WIDTH] + 2 * style[SLIDER_BUTTON_BORDER_WIDTH])));
    
    float sliderButtonMinPos = bounds.x + style[SLIDER_BORDER_WIDTH] + style[SLIDER_BUTTON_BORDER_WIDTH];
    float sliderButtonMaxPos = bounds.x + bounds.width - (style[SLIDER_BORDER_WIDTH] + style[SLIDER_BUTTON_BORDER_WIDTH] + sliderButton.width);
    
    buttonTravelDistance = sliderButtonMaxPos - sliderButtonMinPos;
    
    sliderButton.x = ((int)(bounds.x + style[SLIDER_BORDER_WIDTH] + style[SLIDER_BUTTON_BORDER_WIDTH]) + (sliderPos * buttonTravelDistance));
    sliderButton.y = ((int)(bounds.y + style[SLIDER_BORDER_WIDTH] + style[SLIDER_BUTTON_BORDER_WIDTH]));
    
    if (CheckCollisionPointRec(mousePoint, bounds))
    {
        sliderState = SLIDER_HOVER;
        
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) sliderState = SLIDER_ACTIVE;
        
        if ((sliderState == SLIDER_ACTIVE) && (IsMouseButtonDown(MOUSE_LEFT_BUTTON)))
        {
            sliderButton.x = mousePoint.x - sliderButton.width / 2;
            
            if (sliderButton.x <= sliderButtonMinPos) sliderButton.x = sliderButtonMinPos;
            else if (sliderButton.x >= sliderButtonMaxPos) sliderButton.x = sliderButtonMaxPos;
            
            sliderPos = (sliderButton.x - sliderButtonMinPos) / buttonTravelDistance;
        }   
    }
    else sliderState = SLIDER_DEFAULT;
    //-------------------------------------------------------------------- 
    
    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleRec(bounds, GetColor(style[SLIDER_BORDER_COLOR]));
    DrawRectangle((int)(bounds.x + style[SLIDER_BORDER_WIDTH]), (int)(bounds.y + style[SLIDER_BORDER_WIDTH]), (int)(bounds.width - (2*style[SLIDER_BORDER_WIDTH])), (int)(bounds.height - (2*style[SLIDER_BORDER_WIDTH])), GetColor(style[SLIDER_INSIDE_COLOR]));
    
    switch (sliderState)
    {
        case SLIDER_DEFAULT: DrawRectangleRec(sliderButton, GetColor(style[SLIDER_DEFAULT_COLOR])); break;
        case SLIDER_HOVER: DrawRectangleRec(sliderButton, GetColor(style[SLIDER_HOVER_COLOR])); break;
        case SLIDER_ACTIVE: DrawRectangleRec(sliderButton, GetColor(style[SLIDER_ACTIVE_COLOR])); break;
        default: break;
    } 
    //--------------------------------------------------------------------
    
    return minValue + (maxValue - minValue)*sliderPos;
}

// Slider Bar element, returns selected value
float GuiSliderBar(Rectangle bounds, float value, float minValue, float maxValue)
{
    SliderState sliderState = SLIDER_DEFAULT;
    Vector2 mousePoint = GetMousePosition();
    float fixedValue;
    float fixedMinValue;
    
    fixedValue = value - minValue;
    maxValue = maxValue - minValue;
    fixedMinValue = 0;
    
    // Update control
    //-------------------------------------------------------------------- 
    if (fixedValue <= fixedMinValue) fixedValue = fixedMinValue;
    else if (fixedValue >= maxValue) fixedValue = maxValue;
 
    Rectangle sliderBar;

    sliderBar.x = bounds.x + style[SLIDER_BORDER_WIDTH];
    sliderBar.y = bounds.y + style[SLIDER_BORDER_WIDTH];
    sliderBar.width = ((fixedValue*((float)bounds.width - 2*style[SLIDER_BORDER_WIDTH]))/(maxValue - fixedMinValue));
    sliderBar.height = bounds.height - 2*style[SLIDER_BORDER_WIDTH];

    if (CheckCollisionPointRec(mousePoint, bounds))
    {
        sliderState = SLIDER_HOVER;
        
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) 
        {
            sliderState = SLIDER_ACTIVE;
            
            sliderBar.width = (mousePoint.x - bounds.x - style[SLIDER_BORDER_WIDTH]);
            
            if (mousePoint.x <= (bounds.x + style[SLIDER_BORDER_WIDTH])) sliderBar.width = 0;
            else if (mousePoint.x >= (bounds.x + bounds.width - style[SLIDER_BORDER_WIDTH])) sliderBar.width = bounds.width - 2*style[SLIDER_BORDER_WIDTH];
        }
    }
    else sliderState = SLIDER_DEFAULT;

    fixedValue = ((float)sliderBar.width*(maxValue - fixedMinValue))/((float)bounds.width - 2*style[SLIDER_BORDER_WIDTH]);
    //-------------------------------------------------------------------- 
    
    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleRec(bounds, GetColor(style[SLIDERBAR_BORDER_COLOR]));
    DrawRectangle((int)(bounds.x + style[SLIDER_BORDER_WIDTH]), (int)(bounds.y + style[SLIDER_BORDER_WIDTH]) , (int)(bounds.width - (2*style[SLIDER_BORDER_WIDTH])), (int)(bounds.height - (2*style[SLIDER_BORDER_WIDTH])), GetColor(style[SLIDERBAR_INSIDE_COLOR]));
    
    switch (sliderState)
    {
        case SLIDER_DEFAULT: DrawRectangleRec(sliderBar, GetColor(style[SLIDERBAR_DEFAULT_COLOR])); break;
        case SLIDER_HOVER: DrawRectangleRec(sliderBar, GetColor(style[SLIDERBAR_HOVER_COLOR])); break;
        case SLIDER_ACTIVE: DrawRectangleRec(sliderBar, GetColor(style[SLIDERBAR_ACTIVE_COLOR])); break;
        default: break;
    }
    
    if (minValue < 0 && maxValue > 0) DrawRectangle((bounds.x + style[SLIDER_BORDER_WIDTH]) - (minValue * ((bounds.width - (style[SLIDER_BORDER_WIDTH]*2))/maxValue)), sliderBar.y, 1, sliderBar.height, GetColor(style[SLIDERBAR_ZERO_LINE_COLOR]));
    //--------------------------------------------------------------------

    return fixedValue + minValue;
}

// Progress Bar element, shows current progress value
void GuiProgressBar(Rectangle bounds, float value)
{
    if (value > 1.0f) value = 1.0f;
    else if (value < 0.0f) value = 0.0f;

    Rectangle progressBar = { bounds.x + style[PROGRESSBAR_BORDER_WIDTH], bounds.y + style[PROGRESSBAR_BORDER_WIDTH], bounds.width - (style[PROGRESSBAR_BORDER_WIDTH] * 2), bounds.height - (style[PROGRESSBAR_BORDER_WIDTH] * 2)};
    Rectangle progressValue = { bounds.x + style[PROGRESSBAR_BORDER_WIDTH], bounds.y + style[PROGRESSBAR_BORDER_WIDTH], value * (bounds.width - (style[PROGRESSBAR_BORDER_WIDTH] * 2)), bounds.height - (style[PROGRESSBAR_BORDER_WIDTH] * 2)};

    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleRec(bounds, GetColor(style[PROGRESSBAR_BORDER_COLOR]));
    DrawRectangleRec(progressBar, GetColor(style[PROGRESSBAR_INSIDE_COLOR]));
    DrawRectangleRec(progressValue, GetColor(style[PROGRESSBAR_PROGRESS_COLOR]));
    //--------------------------------------------------------------------
}

// Spinner element, returns selected value
// NOTE: Requires static variables: framesCounter, valueSpeed - ERROR!
int GuiSpinner(Rectangle bounds, int value, int minValue, int maxValue)
{
    SpinnerState spinnerState = SPINNER_DEFAULT;
    Rectangle labelBoxBound = { bounds.x + bounds.width/4 + 1, bounds.y, bounds.width/2, bounds.height };
    Rectangle leftButtonBound = { bounds.x, bounds.y, bounds.width/4, bounds.height };
    Rectangle rightButtonBound = { bounds.x + bounds.width - bounds.width/4 + 1, bounds.y, bounds.width/4, bounds.height };
    Vector2 mousePoint = GetMousePosition();

    int textHeight = GetDefaultFont().size;

    int textWidth = MeasureText(FormatText("%i", value), style[GLOBAL_TEXT_FONTSIZE]);
    
    int buttonSide = 0;
    
    static int framesCounter = 0;
    static bool valueSpeed = false;;
    
    //if (comboBoxButton.width < textWidth) comboBoxButton.width = textWidth + style[TOGGLE_TEXT_PADDING];
    //if (comboBoxButton.height < textHeight) comboBoxButton.height = textHeight + style[TOGGLE_TEXT_PADDING]/2;
    
    // Update control
    //-------------------------------------------------------------------- 
    if (CheckCollisionPointRec(mousePoint, leftButtonBound) || CheckCollisionPointRec(mousePoint, rightButtonBound) || CheckCollisionPointRec(mousePoint, labelBoxBound))
    {
        if (IsKeyDown(KEY_LEFT))
        {
            spinnerState = SPINNER_PRESSED;
            buttonSide = 1;
            
            if (value > minValue) value -= 1;
        }
        else if (IsKeyDown(KEY_RIGHT))
        {
            spinnerState = SPINNER_PRESSED;
            buttonSide = 2;
            
            if (value < maxValue) value += 1;
        }
    }
    
    if (CheckCollisionPointRec(mousePoint, leftButtonBound))
    {
       buttonSide = 1;
       spinnerState = SPINNER_HOVER;
       
       if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
       {
            if (!valueSpeed) 
            {
                if (value > minValue) value--;
                valueSpeed = true;
            }
            else framesCounter++;
            
            spinnerState = SPINNER_PRESSED;
            
            if (value > minValue) 
            {
                if (framesCounter >= 30) value -= 1;
            }
       }
    }
    else if (CheckCollisionPointRec(mousePoint, rightButtonBound))
    {
        buttonSide = 2;
        spinnerState = SPINNER_HOVER;
        
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) 
       {
            if (!valueSpeed) 
            {
                if (value < maxValue) value++;
                valueSpeed = true;
            }
            else framesCounter++;
            
            spinnerState = SPINNER_PRESSED;
            
            if (value < maxValue) 
            {
                if (framesCounter >= 30) value += 1;
            }
       }
    }
    else if (!CheckCollisionPointRec(mousePoint, labelBoxBound)) buttonSide = 0;
    
    if (IsMouseButtonUp(MOUSE_LEFT_BUTTON))
    {
        valueSpeed = false;
        framesCounter = 0;
    }
    //-------------------------------------------------------------------- 
    
    // Draw control
    //--------------------------------------------------------------------
    switch (spinnerState)
    {
        case SPINNER_DEFAULT:
        {
            DrawRectangleRec(leftButtonBound, GetColor(style[SPINNER_DEFAULT_BUTTON_BORDER_COLOR]));
            DrawRectangle(leftButtonBound.x + 2, leftButtonBound.y + 2, leftButtonBound.width - 4, leftButtonBound.height - 4, GetColor(style[SPINNER_DEFAULT_BUTTON_INSIDE_COLOR]));
            
            DrawRectangleRec(rightButtonBound, GetColor(style[SPINNER_DEFAULT_BUTTON_BORDER_COLOR]));
            DrawRectangle(rightButtonBound.x + 2, rightButtonBound.y + 2, rightButtonBound.width - 4, rightButtonBound.height - 4, GetColor(style[SPINNER_DEFAULT_BUTTON_INSIDE_COLOR]));
            
            DrawText(FormatText("-"), leftButtonBound.x + (leftButtonBound.width/2 - (MeasureText(FormatText("+"), style[GLOBAL_TEXT_FONTSIZE]))/2), leftButtonBound.y + (leftButtonBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_DEFAULT_SYMBOL_COLOR])); 
            DrawText(FormatText("+"), rightButtonBound.x + (rightButtonBound.width/2 - (MeasureText(FormatText("-"), style[GLOBAL_TEXT_FONTSIZE]))/2), rightButtonBound.y + (rightButtonBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_DEFAULT_SYMBOL_COLOR])); 
            
            DrawRectangleRec(labelBoxBound, GetColor(style[SPINNER_LABEL_BORDER_COLOR]));
            DrawRectangle(labelBoxBound.x + 1, labelBoxBound.y + 1, labelBoxBound.width - 2, labelBoxBound.height - 2, GetColor(style[SPINNER_LABEL_INSIDE_COLOR]));
            
            DrawText(FormatText("%i", value), labelBoxBound.x + (labelBoxBound.width/2 - textWidth/2), labelBoxBound.y + (labelBoxBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_DEFAULT_TEXT_COLOR]));
        } break;
        case SPINNER_HOVER:
        {
            if (buttonSide == 1)
            {
                DrawRectangleRec(leftButtonBound, GetColor(style[SPINNER_HOVER_BUTTON_BORDER_COLOR]));
                DrawRectangle(leftButtonBound.x + 2, leftButtonBound.y + 2, leftButtonBound.width - 4, leftButtonBound.height - 4, GetColor(style[SPINNER_HOVER_BUTTON_INSIDE_COLOR]));
                
                DrawRectangleRec(rightButtonBound, GetColor(style[SPINNER_DEFAULT_BUTTON_BORDER_COLOR]));
                DrawRectangle(rightButtonBound.x + 2, rightButtonBound.y + 2, rightButtonBound.width - 4, rightButtonBound.height - 4, GetColor(style[SPINNER_DEFAULT_BUTTON_INSIDE_COLOR]));
                
                DrawText(FormatText("-"), leftButtonBound.x + (leftButtonBound.width/2 - (MeasureText(FormatText("+"), style[GLOBAL_TEXT_FONTSIZE]))/2), leftButtonBound.y + (leftButtonBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_HOVER_SYMBOL_COLOR])); 
                DrawText(FormatText("+"), rightButtonBound.x + (rightButtonBound.width/2 - (MeasureText(FormatText("-"), style[GLOBAL_TEXT_FONTSIZE]))/2), rightButtonBound.y + (rightButtonBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_DEFAULT_SYMBOL_COLOR])); 
            }                
            else if (buttonSide == 2)
            {
                DrawRectangleRec(leftButtonBound, GetColor(style[SPINNER_DEFAULT_BUTTON_BORDER_COLOR]));
                DrawRectangle(leftButtonBound.x + 2, leftButtonBound.y + 2, leftButtonBound.width - 4, leftButtonBound.height - 4, GetColor(style[SPINNER_DEFAULT_BUTTON_INSIDE_COLOR]));
                
                DrawRectangleRec(rightButtonBound, GetColor(style[SPINNER_HOVER_BUTTON_BORDER_COLOR]));
                DrawRectangle(rightButtonBound.x + 2, rightButtonBound.y + 2, rightButtonBound.width - 4, rightButtonBound.height - 4, GetColor(style[SPINNER_HOVER_BUTTON_INSIDE_COLOR]));
                
                DrawText(FormatText("-"), leftButtonBound.x + (leftButtonBound.width/2 - (MeasureText(FormatText("+"), style[GLOBAL_TEXT_FONTSIZE]))/2), leftButtonBound.y + (leftButtonBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_DEFAULT_SYMBOL_COLOR])); 
                DrawText(FormatText("+"), rightButtonBound.x + (rightButtonBound.width/2 - (MeasureText(FormatText("-"), style[GLOBAL_TEXT_FONTSIZE]))/2), rightButtonBound.y + (rightButtonBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_HOVER_SYMBOL_COLOR])); 
            }
            
            DrawRectangleRec(labelBoxBound, GetColor(style[SPINNER_LABEL_BORDER_COLOR]));
            DrawRectangle(labelBoxBound.x + 1, labelBoxBound.y + 1, labelBoxBound.width - 2, labelBoxBound.height - 2, GetColor(style[SPINNER_LABEL_INSIDE_COLOR]));
            
            DrawText(FormatText("%i", value), labelBoxBound.x + (labelBoxBound.width/2 - textWidth/2), labelBoxBound.y + (labelBoxBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_HOVER_TEXT_COLOR]));  
        } break;
        case SPINNER_PRESSED:
        {
            if (buttonSide == 1)
            {
                DrawRectangleRec(leftButtonBound, GetColor(style[SPINNER_PRESSED_BUTTON_BORDER_COLOR]));
                DrawRectangle(leftButtonBound.x + 2, leftButtonBound.y + 2, leftButtonBound.width - 4, leftButtonBound.height - 4, GetColor(style[SPINNER_PRESSED_BUTTON_INSIDE_COLOR]));
                
                DrawRectangleRec(rightButtonBound, GetColor(style[SPINNER_DEFAULT_BUTTON_BORDER_COLOR]));
                DrawRectangle(rightButtonBound.x + 2, rightButtonBound.y + 2, rightButtonBound.width - 4, rightButtonBound.height - 4, GetColor(style[SPINNER_DEFAULT_BUTTON_INSIDE_COLOR]));
                
                DrawText(FormatText("-"), leftButtonBound.x + (leftButtonBound.width/2 - (MeasureText(FormatText("+"), style[GLOBAL_TEXT_FONTSIZE]))/2), leftButtonBound.y + (leftButtonBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_PRESSED_SYMBOL_COLOR])); 
                DrawText(FormatText("+"), rightButtonBound.x + (rightButtonBound.width/2 - (MeasureText(FormatText("-"), style[GLOBAL_TEXT_FONTSIZE]))/2), rightButtonBound.y + (rightButtonBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_DEFAULT_SYMBOL_COLOR])); 
            }
            else if (buttonSide == 2)
            {
                DrawRectangleRec(leftButtonBound, GetColor(style[SPINNER_DEFAULT_BUTTON_BORDER_COLOR]));
                DrawRectangle(leftButtonBound.x + 2, leftButtonBound.y + 2, leftButtonBound.width - 4, leftButtonBound.height - 4, GetColor(style[SPINNER_DEFAULT_BUTTON_INSIDE_COLOR]));
                
                DrawRectangleRec(rightButtonBound, GetColor(style[SPINNER_PRESSED_BUTTON_BORDER_COLOR]));
                DrawRectangle(rightButtonBound.x + 2, rightButtonBound.y + 2, rightButtonBound.width - 4, rightButtonBound.height - 4, GetColor(style[SPINNER_PRESSED_BUTTON_INSIDE_COLOR]));
                
                DrawText(FormatText("-"), leftButtonBound.x + (leftButtonBound.width/2 - (MeasureText(FormatText("+"), style[GLOBAL_TEXT_FONTSIZE]))/2), leftButtonBound.y + (leftButtonBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_DEFAULT_SYMBOL_COLOR])); 
                DrawText(FormatText("+"), rightButtonBound.x + (rightButtonBound.width/2 - (MeasureText(FormatText("-"), style[GLOBAL_TEXT_FONTSIZE]))/2), rightButtonBound.y + (rightButtonBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_PRESSED_SYMBOL_COLOR])); 
            }
            
            DrawRectangleRec(labelBoxBound, GetColor(style[SPINNER_LABEL_BORDER_COLOR]));
            DrawRectangle(labelBoxBound.x + 1, labelBoxBound.y + 1, labelBoxBound.width - 2, labelBoxBound.height - 2, GetColor(style[SPINNER_LABEL_INSIDE_COLOR]));
            
            DrawText(FormatText("%i", value), labelBoxBound.x + (labelBoxBound.width/2 - textWidth/2), labelBoxBound.y + (labelBoxBound.height/2 - (style[GLOBAL_TEXT_FONTSIZE]/2)), style[GLOBAL_TEXT_FONTSIZE], GetColor(style[SPINNER_PRESSED_TEXT_COLOR]));  
        } break;
        default: break;
    }
    
    return value;
}

// Text Box element, returns input text
// NOTE: Requires static variables: framesCounter - ERROR!
char *GuiTextBox(Rectangle bounds, char *text)
{
    #define MAX_CHARS_LENGTH  	20
    #define KEY_BACKSPACE_TEXT   3
    
    int initPos = bounds.x + 4;
    char letter = -1;
    static int framesCounter = 0;
    Vector2 mousePoint = GetMousePosition();
    
    // Update control
    //-------------------------------------------------------------------- 
    framesCounter++;
    
    letter = GetKeyPressed();
  
    if (CheckCollisionPointRec(mousePoint, bounds)) 
    {        
        if (letter != -1)
        {
            if (letter == KEY_BACKSPACE_TEXT)
            {
                for (int i = 0; i < MAX_CHARS_LENGTH; i++)
                {
                    if ((text[i] == '\0') && (i > 0))
                    {
                        text[i - 1] = '\0';
                        break;
                    }
                }
                
                text[MAX_CHARS_LENGTH - 1] = '\0';
            }
            else
            {
                for (int i = 0; i < MAX_CHARS_LENGTH; i++)
                {
                    if (text[i] == '\0')
                    {
                        text[i] = letter;
                        break;
                    }
                }
            }
        }
    }
    //-------------------------------------------------------------------- 

    // Draw control
    //--------------------------------------------------------------------
    if (CheckCollisionPointRec(mousePoint, bounds)) DrawRectangleRec(bounds, GetColor(style[TOGGLE_ACTIVE_BORDER_COLOR]));
    else DrawRectangleRec(bounds, GetColor(style[TEXTBOX_BORDER_COLOR]));
    
    DrawRectangle(bounds.x + style[TEXTBOX_BORDER_WIDTH], bounds.y + style[TEXTBOX_BORDER_WIDTH], bounds.width - (style[TEXTBOX_BORDER_WIDTH] * 2), bounds.height - (style[TEXTBOX_BORDER_WIDTH] * 2), GetColor(style[TEXTBOX_INSIDE_COLOR]));
    
    for (int i = 0; i < MAX_CHARS_LENGTH; i++)
    {
        if (text[i] == '\0') break;
        
        DrawText(FormatText("%c", text[i]), initPos, bounds.y + style[TEXTBOX_TEXT_FONTSIZE], style[TEXTBOX_TEXT_FONTSIZE], GetColor(style[TEXTBOX_TEXT_COLOR]));
        
        initPos += ((GetDefaultFont().charRecs[(int)text[i] - 32].width + 2));
    }

    if ((framesCounter/20)%2 && CheckCollisionPointRec(mousePoint, bounds)) DrawLine(initPos + 2, bounds.y + 5, initPos + 2, bounds.y + 10 + 15, GetColor(style[TEXTBOX_LINE_COLOR]));
    //--------------------------------------------------------------------    

    return text;
}

// TODO: GuiBox?
// TODO: GuiWindow?
// TODO: GuiPanel?

// Save current GUI style into a text file
void SaveGuiStyle(const char *fileName)
{
    FILE *styleFile = fopen(fileName, "wt");
    
    for (int i = 0; i < NUM_PROPERTIES; i++) fprintf(styleFile, "%-40s0x%x\n", guiPropertyName[i], GetStyleProperty(i));
    
    fclose(styleFile);
}

// Load GUI style from a text file
void LoadGuiStyle(const char *fileName)
{
    typedef struct {
        char id[64];
        int value;
    } StyleProperty;
    
    StyleProperty *styleProp = (StyleProperty *)malloc(128*sizeof(StyleProperty));; 
    int counter = 0;
    
    FILE *styleFile = fopen(fileName, "rt");

    while (!feof(styleFile))
    {
        fscanf(styleFile, "%s %i\n", styleProp[counter].id, &styleProp[counter].value);
        counter++;
    }

    fclose(styleFile);

    for (int i = 0; i < counter; i++)
    {
        for (int j = 0; j < NUM_PROPERTIES; j++)
        {
            if (strcmp(styleProp[i].id, guiPropertyName[j]) == 0)
            {
                // Assign correct property to style
                style[j] = styleProp[i].value;
            }
        }
    }
    
    free(styleProp);
}

// Set one style property value
void SetStyleProperty(int guiProperty, int value) 
{
    #define NUM_COLOR_SAMPLES   10
    
    if (guiProperty == GLOBAL_BASE_COLOR)
    {
        Color baseColor = GetColor(value);
        Color fadeColor[NUM_COLOR_SAMPLES];
        
        for (int i = 0; i < NUM_COLOR_SAMPLES; i++) fadeColor[i] = ColorMultiply(baseColor, 1.0f - (float)i/(NUM_COLOR_SAMPLES - 1));
        
        style[GLOBAL_BASE_COLOR] = value;
        style[BACKGROUND_COLOR] = GetHexValue(fadeColor[3]);
        style[BUTTON_DEFAULT_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[BUTTON_HOVER_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[BUTTON_PRESSED_INSIDE_COLOR] = GetHexValue(fadeColor[5]);
        style[TOGGLE_DEFAULT_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[TOGGLE_HOVER_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[TOGGLE_PRESSED_INSIDE_COLOR] = GetHexValue(fadeColor[5]);
        style[TOGGLE_ACTIVE_INSIDE_COLOR] = GetHexValue(fadeColor[8]);
        style[SLIDER_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[SLIDER_DEFAULT_COLOR] = GetHexValue(fadeColor[6]);
        style[SLIDER_HOVER_COLOR] = GetHexValue(fadeColor[7]);
        style[SLIDER_ACTIVE_COLOR] = GetHexValue(fadeColor[9]);
        style[SLIDERBAR_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[SLIDERBAR_DEFAULT_COLOR] = GetHexValue(fadeColor[6]);
        style[SLIDERBAR_HOVER_COLOR] = GetHexValue(fadeColor[7]);
        style[SLIDERBAR_ACTIVE_COLOR] = GetHexValue(fadeColor[9]);
        style[SLIDERBAR_ZERO_LINE_COLOR] = GetHexValue(fadeColor[8]);
        style[PROGRESSBAR_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[PROGRESSBAR_PROGRESS_COLOR] = GetHexValue(fadeColor[6]);
        style[SPINNER_LABEL_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[SPINNER_DEFAULT_BUTTON_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[SPINNER_HOVER_BUTTON_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[SPINNER_PRESSED_BUTTON_INSIDE_COLOR] = GetHexValue(fadeColor[5]);
        style[COMBOBOX_DEFAULT_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[COMBOBOX_HOVER_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[COMBOBOX_PRESSED_INSIDE_COLOR] = GetHexValue(fadeColor[8]);
        style[COMBOBOX_PRESSED_LIST_INSIDE_COLOR] = GetHexValue(fadeColor[8]);
        style[CHECKBOX_DEFAULT_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
        style[CHECKBOX_CLICK_INSIDE_COLOR] = GetHexValue(fadeColor[6]);
        style[CHECKBOX_STATUS_ACTIVE_COLOR] = GetHexValue(fadeColor[8]);
        style[TEXTBOX_INSIDE_COLOR] = GetHexValue(fadeColor[4]);
    }
    else if (guiProperty == GLOBAL_BORDER_COLOR)
    {
        Color baseColor = GetColor(value);
        Color fadeColor[NUM_COLOR_SAMPLES];
        
        for (int i = 0; i < NUM_COLOR_SAMPLES; i++) fadeColor[i] = ColorMultiply(baseColor, 1.0f - (float)i/(NUM_COLOR_SAMPLES - 1));
        
        style[GLOBAL_BORDER_COLOR] = value;
        style[BUTTON_DEFAULT_BORDER_COLOR] = GetHexValue(fadeColor[7]);
        style[BUTTON_HOVER_BORDER_COLOR] = GetHexValue(fadeColor[8]);
        style[BUTTON_PRESSED_BORDER_COLOR] = GetHexValue(fadeColor[9]);
        style[TOGGLE_DEFAULT_BORDER_COLOR] = GetHexValue(fadeColor[7]);
        style[TOGGLE_HOVER_BORDER_COLOR] = GetHexValue(fadeColor[8]);
        style[TOGGLE_PRESSED_BORDER_COLOR] = GetHexValue(fadeColor[9]);
        style[TOGGLE_ACTIVE_BORDER_COLOR] = GetHexValue(fadeColor[9]);
        style[SLIDER_BORDER_COLOR] = GetHexValue(fadeColor[7]);
        style[SLIDERBAR_BORDER_COLOR] = GetHexValue(fadeColor[7]);
        style[PROGRESSBAR_BORDER_COLOR] = GetHexValue(fadeColor[7]);
        style[SPINNER_LABEL_BORDER_COLOR] = GetHexValue(fadeColor[7]);
        style[SPINNER_DEFAULT_BUTTON_BORDER_COLOR] = GetHexValue(fadeColor[7]);
        style[SPINNER_HOVER_BUTTON_BORDER_COLOR] = GetHexValue(fadeColor[8]);
        style[SPINNER_PRESSED_BUTTON_BORDER_COLOR] = GetHexValue(fadeColor[9]);
        style[COMBOBOX_DEFAULT_BORDER_COLOR] = GetHexValue(fadeColor[7]);
        style[COMBOBOX_HOVER_BORDER_COLOR] = GetHexValue(fadeColor[8]);
        style[COMBOBOX_PRESSED_BORDER_COLOR] = GetHexValue(fadeColor[9]);
        style[COMBOBOX_PRESSED_LIST_BORDER_COLOR] = GetHexValue(fadeColor[9]);
        style[CHECKBOX_DEFAULT_BORDER_COLOR] = GetHexValue(fadeColor[7]);
        style[CHECKBOX_HOVER_BORDER_COLOR] = GetHexValue(fadeColor[8]);
        style[CHECKBOX_CLICK_BORDER_COLOR] = GetHexValue(fadeColor[9]);
        style[TEXTBOX_BORDER_COLOR] = GetHexValue(fadeColor[7]);
    }
    else if (guiProperty == GLOBAL_TEXT_COLOR)
    {
        Color baseColor = GetColor(value);
        Color fadeColor[NUM_COLOR_SAMPLES];
        
        for (int i = 0; i < NUM_COLOR_SAMPLES; i++) fadeColor[i] = ColorMultiply(baseColor, 1.0f - (float)i/(NUM_COLOR_SAMPLES - 1));
        
        style[GLOBAL_TEXT_COLOR] = value;
        style[LABEL_TEXT_COLOR] = GetHexValue(fadeColor[9]);
        style[BUTTON_DEFAULT_TEXT_COLOR] = GetHexValue(fadeColor[9]);
        style[BUTTON_HOVER_TEXT_COLOR] = GetHexValue(fadeColor[8]);
        style[BUTTON_PRESSED_TEXT_COLOR] = GetHexValue(fadeColor[5]);
        style[TOGGLE_DEFAULT_TEXT_COLOR] = GetHexValue(fadeColor[9]);
        style[TOGGLE_HOVER_TEXT_COLOR] = GetHexValue(fadeColor[8]);
        style[TOGGLE_PRESSED_TEXT_COLOR] = GetHexValue(fadeColor[5]);
        style[TOGGLE_ACTIVE_TEXT_COLOR] = GetHexValue(fadeColor[5]);
        style[SPINNER_DEFAULT_SYMBOL_COLOR] = GetHexValue(fadeColor[9]);
        style[SPINNER_DEFAULT_TEXT_COLOR] = GetHexValue(fadeColor[9]);
        style[SPINNER_HOVER_SYMBOL_COLOR] = GetHexValue(fadeColor[8]);
        style[SPINNER_HOVER_TEXT_COLOR] = GetHexValue(fadeColor[8]);
        style[SPINNER_PRESSED_SYMBOL_COLOR] = GetHexValue(fadeColor[5]);
        style[SPINNER_PRESSED_TEXT_COLOR] = GetHexValue(fadeColor[5]);
        style[COMBOBOX_DEFAULT_TEXT_COLOR] = GetHexValue(fadeColor[9]);
        style[COMBOBOX_DEFAULT_LIST_TEXT_COLOR] = GetHexValue(fadeColor[9]);
        style[COMBOBOX_HOVER_TEXT_COLOR] = GetHexValue(fadeColor[8]);
        style[COMBOBOX_HOVER_LIST_TEXT_COLOR] = GetHexValue(fadeColor[8]);
        style[COMBOBOX_PRESSED_TEXT_COLOR] = GetHexValue(fadeColor[4]);
        style[COMBOBOX_PRESSED_LIST_TEXT_COLOR] = GetHexValue(fadeColor[4]);
        style[TEXTBOX_TEXT_COLOR] = GetHexValue(fadeColor[9]);
        style[TEXTBOX_LINE_COLOR] = GetHexValue(fadeColor[6]);
    }
    else style[guiProperty] = value; 

}

// Get one style property value
int GetStyleProperty(int guiProperty) { return style[guiProperty]; }

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

static Color ColorMultiply(Color baseColor, float value)
{
    Color multColor = baseColor;

    if (value > 1.0f) value = 1.0f;
    else if (value < 0.0f) value = 0.0f;
    
    multColor.r += (255 - multColor.r)*value;
    multColor.g += (255 - multColor.g)*value;
    multColor.b += (255 - multColor.b)*value;
    
    return multColor;
}