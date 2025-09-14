/**********************************************************************************************
*
*   fix_win32_compatibility.h   Utility to help include windows.h with raylib projects
*
*   Useage: #include "fix_win32_compatibility.h" any library that uses windows.h
*                                     order is critical, this must be done before raylib
*
*   LICENSE: MIT
*
*   Copyright (c) 2025 Jeffery Myers
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

#pragma once

// move the windows functions to new names
// note that you can't call these functions or structures from your code, but you should not neeed to
#define CloseWindow CloseWindowWin32
#define Rectangle RectangleWin32
#define ShowCursor ShowCursorWin32
#define LoadImageA LoadImageAWin32
#define LoadImageW LoadImageWin32
#define DrawTextA DrawTextAWin32
#define DrawTextW DrawTextWin32
#define DrawTextExA DrawTextExAWin32
#define DrawTextExW DrawTextExWin32
#define PlaySoundA PlaySoundAWin32
// include windows
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

// remove all our redfintions so that raylib can define them properly
#undef CloseWindow
#undef Rectangle
#undef ShowCursor
#undef LoadImage 
#undef LoadImageA
#undef LoadImageW
#undef DrawText 
#undef DrawTextA
#undef DrawTextW
#undef DrawTextEx 
#undef DrawTextExA
#undef DrawTextExW
#undef PlaySoundA
