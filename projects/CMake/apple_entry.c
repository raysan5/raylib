/**********************************************************************************************
*
*   raylib - UIKit entry point for a C game
*
*   This software is provided under the zlib/libpng license. See LICENSE for details.
*
**********************************************************************************************/

#include "raylib_apple.h"

extern int RaylibGameMain(void);

int main(int argc, char **argv)
{
    return RaylibRunAppleApp(argc, argv, RaylibGameMain);
}
