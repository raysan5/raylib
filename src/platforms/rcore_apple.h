/**********************************************************************************************
*
*   rcore_apple - Interface between rcore and the UIKit application host
*
*   This software is provided under the zlib/libpng license. See LICENSE for details.
*
**********************************************************************************************/

#ifndef RCORE_APPLE_H
#define RCORE_APPLE_H

#include <stdbool.h>

#include "raylib.h"

enum {
    APPLE_GAMEPADS = 4,
    APPLE_GAMEPAD_BUTTONS = GAMEPAD_BUTTON_RIGHT_THUMB + 1,
    APPLE_GAMEPAD_AXES = GAMEPAD_AXIS_RIGHT_TRIGGER + 1,
    APPLE_REMOTE_KEYS = 6
};

typedef struct AppleGamepad {
    bool connected;
    char name[128];
    bool buttons[APPLE_GAMEPAD_BUTTONS];
    float axes[APPLE_GAMEPAD_AXES];
} AppleGamepad;

typedef struct AppleFrame {
    double timestamp;
    double interval;
    int width;
    int height;
    float pointWidth;
    float pointHeight;
    int displayWidth;
    int displayHeight;
    int refreshRate;
    void *window;
    unsigned int framebuffer;
    bool resetInput;
    bool remoteKeys[APPLE_REMOTE_KEYS];
    AppleGamepad gamepads[APPLE_GAMEPADS];
} AppleFrame;

typedef enum AppleTouchAction {
    APPLE_TOUCH_UP = 0,
    APPLE_TOUCH_DOWN,
    APPLE_TOUCH_MOVE,
    APPLE_TOUCH_CANCEL
} AppleTouchAction;

typedef struct AppleTouchEvent {
    AppleTouchAction action;
    int id;
    float x;
    float y;
} AppleTouchEvent;

// The following functions are called on the rendering thread
bool AppleWaitFrame(double minimumTimestamp, AppleFrame *frame);
void ApplePresentFrame(void);
bool AppleNextTouch(AppleTouchEvent *event);
void AppleOpenURL(const char *url);

#endif  // RCORE_APPLE_H
