/*******************************************************************************************
*
*   raylib [rlua] example - Lua file execution
*
*   NOTE: This example requires Lua library (http://luabinaries.sourceforge.net/download.html)
*
*   Compile example using:
*   gcc -o $(NAME_PART).exe $(FILE_NAME) $(RAYLIB_DIR)\raylib_icon          /
*       -I../src -I../src/external/lua/include -L../src/external/lua/lib    /
*       -lraylib -lglfw3 -lopengl32 -lopenal32 -llua53 -lgdi32 -std=c99
*
*   This example has been created using raylib 1.6 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define RLUA_IMPLEMENTATION
#include "rlua.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitLuaDevice();
    //--------------------------------------------------------------------------------------

    ExecuteLuaFile("core_basic_window.lua");

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseLuaDevice();        // Close Lua device and free resources
    //--------------------------------------------------------------------------------------

	return 0;
}