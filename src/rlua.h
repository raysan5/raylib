/**********************************************************************************************
*
*   rlua - raylib Lua bindings
*
*   NOTE 01:
*   The following types:
*       Color, Vector2, Vector3, Rectangle, Ray, Camera, Camera2D
*   are treated as objects with named fields, same as in C.
*
*   Lua defines utility functions for creating those objects.
*   Usage:
*       local cl = Color(255,255,255,255)
*       local rec = Rectangle(10, 10, 100, 100)
*       local ray = Ray(Vector3(20, 20, 20), Vector3(50, 50, 50))
*       local x2 = rec.x + rec.width
*
*   The following types:
*       Image, Texture2D, RenderTexture2D, SpriteFont
*   are immutable, and you can only read their non-pointer arguments (e.g. sprfnt.size).
*
*   All other object types are opaque, that is, you cannot access or
*   change their fields directly.
*
*   Remember that ALL raylib types have REFERENCE SEMANTICS in Lua.
*   There is currently no way to create a copy of an opaque object.
*
*   NOTE 02:
*   Some raylib functions take a pointer to an array, and the size of that array.
*   The equivalent Lua functions take only an array table of the specified type UNLESS
*   it's a pointer to a large char array (e.g. for images), then it takes (and potentially returns)
*   a Lua string (without the size argument, as Lua strings are sized by default).
*
*   NOTE 03:
*   Some raylib functions take pointers to objects to modify (e.g. ImageToPOT, etc.)
*   In Lua, these functions take values and return a new changed value, instead.
*   So, in C:
*       ImageToPOT(&img, BLACK);
*   In Lua becomes:
*       img = ImageToPOT(img, BLACK)
*
*   Remember that functions can return multiple values, so:
*       UpdateCameraPlayer(&cam, &playerPos);
*       Vector3 vec = ResolveCollisionCubicmap(img, mapPos, &playerPos, 5.0);
*   becomes:
*       cam, playerPos = UpdateCameraPlayer(cam, playerPos)
*       vec, playerPos = ResolveCollisionCubicmap(img, mapPos, playerPos, 5)
*
*   This is to preserve value semantics of raylib objects.
*
*
*   This Lua binding for raylib was originally created by Ghassan Al-Mashareqa (ghassan@ghassan.pl)
*   for raylib 1.3 and later on reviewed and updated to raylib 1.6 by Ramon Santamaria.
*
*   Copyright (c) 2015-2016 Ghassan Al-Mashareqa and Ramon Santamaria (@raysan5)
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

#pragma once

#include "raylib.h"

#define RLUA_STATIC
#ifdef RLUA_STATIC
    #define RLUADEF static            // Functions just visible to module including this file
#else
    #ifdef __cplusplus
        #define RLUADEF extern "C"    // Functions visible from other files (no name mangling of functions in C++)
    #else
        #define RLUADEF extern        // Functions visible from other files
    #endif
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
RLUADEF void InitLuaDevice(void);                   // Initialize Lua system
RLUADEF void ExecuteLuaCode(const char *code);      // Execute raylib Lua code
RLUADEF void ExecuteLuaFile(const char *filename);  // Execute raylib Lua script
RLUADEF void CloseLuaDevice(void);                  // De-initialize Lua system

/***********************************************************************************
*
*   RLUA IMPLEMENTATION
*
************************************************************************************/

#if defined(RLUA_IMPLEMENTATION)

#include "raylib.h"
#include "utils.h"
#include "raymath.h"

#include <string.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define LuaPush_Image(L, img) LuaPushOpaqueTypeWithMetatable(L, img, Image)
#define LuaPush_Texture2D(L, tex) LuaPushOpaqueTypeWithMetatable(L, tex, Texture2D)
#define LuaPush_RenderTexture2D(L, tex) LuaPushOpaqueTypeWithMetatable(L, tex, RenderTexture2D)
#define LuaPush_SpriteFont(L, sf) LuaPushOpaqueTypeWithMetatable(L, sf, SpriteFont)
#define LuaPush_Mesh(L, vd) LuaPushOpaqueType(L, vd)
#define LuaPush_Shader(L, s) LuaPushOpaqueType(L, s)
#define LuaPush_Light(L, light) LuaPushOpaqueTypeWithMetatable(L, light, Light)
#define LuaPush_Sound(L, snd) LuaPushOpaqueType(L, snd)
#define LuaPush_Wave(L, wav) LuaPushOpaqueType(L, wav)
#define LuaPush_Music(L, mus) LuaPushOpaqueType(L, mus)
#define LuaPush_AudioStream(L, aud) LuaPushOpaqueType(L, aud)

#define LuaGetArgument_string luaL_checkstring
#define LuaGetArgument_int (int)luaL_checkinteger
#define LuaGetArgument_unsigned (unsigned)luaL_checkinteger
#define LuaGetArgument_char (char)luaL_checkinteger
#define LuaGetArgument_float (float)luaL_checknumber
#define LuaGetArgument_double luaL_checknumber

#define LuaGetArgument_Image(L, img) *(Image*)LuaGetArgumentOpaqueTypeWithMetatable(L, img, "Image")
#define LuaGetArgument_Texture2D(L, tex) *(Texture2D*)LuaGetArgumentOpaqueTypeWithMetatable(L, tex, "Texture2D")
#define LuaGetArgument_RenderTexture2D(L, rtex) *(RenderTexture2D*)LuaGetArgumentOpaqueTypeWithMetatable(L, rtex, "RenderTexture2D")
#define LuaGetArgument_SpriteFont(L, sf) *(SpriteFont*)LuaGetArgumentOpaqueTypeWithMetatable(L, sf, "SpriteFont")
#define LuaGetArgument_Mesh(L, vd) *(Mesh*)LuaGetArgumentOpaqueType(L, vd)
#define LuaGetArgument_Shader(L, s) *(Shader*)LuaGetArgumentOpaqueType(L, s)
#define LuaGetArgument_Light(L, light) *(Light*)LuaGetArgumentOpaqueType(L, light)
#define LuaGetArgument_Sound(L, snd) *(Sound*)LuaGetArgumentOpaqueType(L, snd)
#define LuaGetArgument_Wave(L, wav) *(Wave*)LuaGetArgumentOpaqueType(L, wav)
#define LuaGetArgument_Music(L, mus) *(Music*)LuaGetArgumentOpaqueType(L, mus)
#define LuaGetArgument_AudioStream(L, aud) *(AudioStream*)LuaGetArgumentOpaqueType(L, aud)

#define LuaPushOpaqueType(L, str) LuaPushOpaque(L, &str, sizeof(str))
#define LuaPushOpaqueTypeWithMetatable(L, str, meta) LuaPushOpaqueWithMetatable(L, &str, sizeof(str), #meta)

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static lua_State* mainLuaState = 0;
static lua_State* L = 0;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void LuaPush_Color(lua_State* L, Color color);
static void LuaPush_Vector2(lua_State* L, Vector2 vec);
static void LuaPush_Vector3(lua_State* L, Vector3 vec);
static void LuaPush_Quaternion(lua_State* L, Quaternion vec);
static void LuaPush_Matrix(lua_State* L, Matrix *matrix);
static void LuaPush_Rectangle(lua_State* L, Rectangle rect);
static void LuaPush_Model(lua_State* L, Model mdl);
static void LuaPush_Ray(lua_State* L, Ray ray);
static void LuaPush_Camera(lua_State* L, Camera cam);

static Vector2 LuaGetArgument_Vector2(lua_State* L, int index);
static Vector3 LuaGetArgument_Vector3(lua_State* L, int index);
static Quaternion LuaGetArgument_Quaternion(lua_State* L, int index);
static Color LuaGetArgument_Color(lua_State* L, int index);
static Rectangle LuaGetArgument_Rectangle(lua_State* L, int index);
static Camera LuaGetArgument_Camera(lua_State* L, int index);
static Camera2D LuaGetArgument_Camera2D(lua_State* L, int index);
static Ray LuaGetArgument_Ray(lua_State* L, int index);
static Matrix LuaGetArgument_Matrix(lua_State* L, int index);
static Model LuaGetArgument_Model(lua_State* L, int index);

//----------------------------------------------------------------------------------
// rlua Helper Functions
//----------------------------------------------------------------------------------
static void LuaStartEnum(void)
{
    lua_newtable(L);
}

static void LuaSetEnum(const char *name, int value)
{
    lua_pushinteger(L, value);
    lua_setfield(L, -2, name);
}

static void LuaSetEnumColor(const char *name, Color color)
{
    LuaPush_Color(L, color);
    lua_setfield(L, -2, name);
}

static void LuaEndEnum(const char *name)
{
    lua_setglobal(L, name);
}

static void LuaPushOpaque(lua_State* L, void *ptr, size_t size)
{
    void *ud = lua_newuserdata(L, size);
    memcpy(ud, ptr, size);
}

static void LuaPushOpaqueWithMetatable(lua_State* L, void *ptr, size_t size, const char *metatable_name)
{
    void *ud = lua_newuserdata(L, size);
    memcpy(ud, ptr, size);
    luaL_setmetatable(L, metatable_name);
}

static void* LuaGetArgumentOpaqueType(lua_State* L, int index)
{
    return lua_touserdata(L, index);
}

static void* LuaGetArgumentOpaqueTypeWithMetatable(lua_State* L, int index, const char *metatable_name)
{
    return luaL_checkudata(L, index, metatable_name);
}

//----------------------------------------------------------------------------------
// LuaIndex* functions
//----------------------------------------------------------------------------------
static int LuaIndexImage(lua_State* L)
{
    Image img = LuaGetArgument_Image(L, 1);
    const char *key = luaL_checkstring(L, 2);
    if (!strcmp(key, "width"))
        lua_pushinteger(L, img.width);
    else if (!strcmp(key, "height"))
        lua_pushinteger(L, img.height);
    else if (!strcmp(key, "mipmaps"))
        lua_pushinteger(L, img.mipmaps);
    else if (!strcmp(key, "format"))
        lua_pushinteger(L, img.format);
    else
        return 0;
    return 1;
}

static int LuaIndexTexture2D(lua_State* L)
{
    Texture2D img = LuaGetArgument_Texture2D(L, 1);
    const char *key = luaL_checkstring(L, 2);
    if (!strcmp(key, "width"))
        lua_pushinteger(L, img.width);
    else if (!strcmp(key, "height"))
        lua_pushinteger(L, img.height);
    else if (!strcmp(key, "mipmaps"))
        lua_pushinteger(L, img.mipmaps);
    else if (!strcmp(key, "format"))
        lua_pushinteger(L, img.format);
    else if (!strcmp(key, "id"))
        lua_pushinteger(L, img.id);
    else
        return 0;
    return 1;
}

static int LuaIndexRenderTexture2D(lua_State* L)
{
    RenderTexture2D img = LuaGetArgument_RenderTexture2D(L, 1);
    const char *key = luaL_checkstring(L, 2);
    if (!strcmp(key, "texture"))
        LuaPush_Texture2D(L, img.texture);
    else if (!strcmp(key, "depth"))
        LuaPush_Texture2D(L, img.depth);
    else
        return 0;
    return 1;
}

static int LuaIndexSpriteFont(lua_State* L)
{
    SpriteFont img = LuaGetArgument_SpriteFont(L, 1);
    const char *key = luaL_checkstring(L, 2);
    if (!strcmp(key, "size"))
        lua_pushinteger(L, img.size);
    else if (!strcmp(key, "texture"))
        LuaPush_Texture2D(L, img.texture);
    else if (!strcmp(key, "numChars"))
        lua_pushinteger(L, img.numChars);
    else
        return 0;
    return 1;
}

static int LuaIndexLight(lua_State* L)
{
    Light light = LuaGetArgument_Light(L, 1);
    const char *key = luaL_checkstring(L, 2);
    if (!strcmp(key, "id"))
        lua_pushinteger(L, light->id);
    else if (!strcmp(key, "enabled"))
        lua_pushboolean(L, light->enabled);
    else if (!strcmp(key, "type"))
        lua_pushinteger(L, light->type);
    else if (!strcmp(key, "position"))
        LuaPush_Vector3(L, light->position);
    else if (!strcmp(key, "target"))
        LuaPush_Vector3(L, light->target);
    else if (!strcmp(key, "radius"))
        lua_pushnumber(L, light->radius);
    else if (!strcmp(key, "diffuse"))
        LuaPush_Color(L, light->diffuse);
    else if (!strcmp(key, "intensity"))
        lua_pushnumber(L, light->intensity);
    else if (!strcmp(key, "coneAngle"))
        lua_pushnumber(L, light->coneAngle);
    else
        return 0;
    return 1;
}

static int LuaNewIndexLight(lua_State* L)
{
    Light light = LuaGetArgument_Light(L, 1);
    const char *key = luaL_checkstring(L, 2);
    if (!strcmp(key, "id"))
        light->id = LuaGetArgument_int(L, 3);
    else if (!strcmp(key, "enabled"))
        light->enabled = lua_toboolean(L, 3);
    else if (!strcmp(key, "type"))
        light->type = LuaGetArgument_int(L, 3);
        else if (!strcmp(key, "position"))
        light->position = LuaGetArgument_Vector3(L, 3);
        else if (!strcmp(key, "target"))
        light->target = LuaGetArgument_Vector3(L, 3);
        else if (!strcmp(key, "radius"))
        light->radius = LuaGetArgument_float(L, 3);
        else if (!strcmp(key, "diffuse"))
        light->diffuse = LuaGetArgument_Color(L, 3);
        else if (!strcmp(key, "intensity"))
        light->intensity = LuaGetArgument_float(L, 3);
        else if (!strcmp(key, "coneAngle"))
        light->coneAngle = LuaGetArgument_float(L, 3);
    return 0;
}

static void LuaBuildOpaqueMetatables(void)
{
    luaL_newmetatable(L, "Image");
    lua_pushcfunction(L, &LuaIndexImage);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    luaL_newmetatable(L, "Texture2D");
    lua_pushcfunction(L, &LuaIndexTexture2D);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    luaL_newmetatable(L, "RenderTexture2D");
    lua_pushcfunction(L, &LuaIndexRenderTexture2D);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

        luaL_newmetatable(L, "SpriteFont");
        lua_pushcfunction(L, &LuaIndexSpriteFont);
        lua_setfield(L, -2, "__index");
        lua_pop(L, 1);

        luaL_newmetatable(L, "Light");
        lua_pushcfunction(L, &LuaIndexLight);
        lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, &LuaNewIndexLight);
        lua_setfield(L, -2, "__newindex");
        lua_pop(L, 1);
}

//----------------------------------------------------------------------------------
// LuaGetArgument functions
//----------------------------------------------------------------------------------

static Vector2 LuaGetArgument_Vector2(lua_State* L, int index)
{
    index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values
    luaL_argcheck(L, lua_getfield(L, index, "x") == LUA_TNUMBER, index, "Expected Vector2");
    float x = (float)lua_tonumber(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "y") == LUA_TNUMBER, index, "Expected Vector2");
    float y = (float)lua_tonumber(L, -1);
    lua_pop(L, 2);
    return (Vector2) { x, y };
}

static Vector3 LuaGetArgument_Vector3(lua_State* L, int index)
{
    index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values
    luaL_argcheck(L, lua_getfield(L, index, "x") == LUA_TNUMBER, index, "Expected Vector3");
    float x = (float)lua_tonumber(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "y") == LUA_TNUMBER, index, "Expected Vector3");
    float y = (float)lua_tonumber(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "z") == LUA_TNUMBER, index, "Expected Vector3");
    float z = (float)lua_tonumber(L, -1);
    lua_pop(L, 3);
    return (Vector3) { x, y, z };
}

static Quaternion LuaGetArgument_Quaternion(lua_State* L, int index)
{
    index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values
    luaL_argcheck(L, lua_getfield(L, index, "x") == LUA_TNUMBER, index, "Expected Quaternion");
    float x = (float)lua_tonumber(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "y") == LUA_TNUMBER, index, "Expected Quaternion");
    float y = (float)lua_tonumber(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "z") == LUA_TNUMBER, index, "Expected Quaternion");
    float z = (float)lua_tonumber(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "w") == LUA_TNUMBER, index, "Expected Quaternion");
    float w = (float)lua_tonumber(L, -1);
    lua_pop(L, 4);
    return (Quaternion) { x, y, z, w };
}

static Color LuaGetArgument_Color(lua_State* L, int index)
{
    index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values
    luaL_argcheck(L, lua_getfield(L, index, "r") == LUA_TNUMBER, index, "Expected Color");
    unsigned char r = (unsigned char)lua_tointeger(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "g") == LUA_TNUMBER, index, "Expected Color");
    unsigned char g = (unsigned char)lua_tointeger(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "b") == LUA_TNUMBER, index, "Expected Color");
    unsigned char b = (unsigned char)lua_tointeger(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "a") == LUA_TNUMBER, index, "Expected Color");
    unsigned char a = (unsigned char)lua_tointeger(L, -1);
    lua_pop(L, 4);
    return (Color) { r, g, b, a };
}

static Rectangle LuaGetArgument_Rectangle(lua_State* L, int index)
{
    index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values
    luaL_argcheck(L, lua_getfield(L, index, "x") == LUA_TNUMBER, index, "Expected Rectangle");
    int x = (int)lua_tointeger(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "y") == LUA_TNUMBER, index, "Expected Rectangle");
    int y = (int)lua_tointeger(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "width") == LUA_TNUMBER, index, "Expected Rectangle");
    int w = (int)lua_tointeger(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "height") == LUA_TNUMBER, index, "Expected Rectangle");
    int h = (int)lua_tointeger(L, -1);
    lua_pop(L, 4);
    return (Rectangle) { x, y, w, h };
}

static Camera LuaGetArgument_Camera(lua_State* L, int index)
{
    Camera result;
    index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values
    luaL_argcheck(L, lua_getfield(L, index, "position") == LUA_TTABLE, index, "Expected Camera");
    result.position = LuaGetArgument_Vector3(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "target") == LUA_TTABLE, index, "Expected Camera");
    result.target = LuaGetArgument_Vector3(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "up") == LUA_TTABLE, index, "Expected Camera");
    result.up = LuaGetArgument_Vector3(L, -1);
  luaL_argcheck(L, lua_getfield(L, index, "fovy") == LUA_TNUMBER, index, "Expected Camera");
    result.fovy = LuaGetArgument_float(L, -1);
    lua_pop(L, 4);
    return result;
}

static Camera2D LuaGetArgument_Camera2D(lua_State* L, int index)
{
    Camera2D result;
    index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values
    luaL_argcheck(L, lua_getfield(L, index, "offset") == LUA_TTABLE, index, "Expected Camera2D");
    result.offset = LuaGetArgument_Vector2(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "target") == LUA_TTABLE, index, "Expected Camera2D");
    result.target = LuaGetArgument_Vector2(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "rotation") == LUA_TNUMBER, index, "Expected Camera2D");
    result.rotation = LuaGetArgument_float(L, -1);
  luaL_argcheck(L, lua_getfield(L, index, "zoom") == LUA_TNUMBER, index, "Expected Camera2D");
    result.zoom = LuaGetArgument_float(L, -1);
    lua_pop(L, 4);
    return result;
}

static BoundingBox LuaGetArgument_BoundingBox(lua_State* L, int index)
{
    BoundingBox result;
    index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values
    luaL_argcheck(L, lua_getfield(L, index, "min") == LUA_TTABLE, index, "Expected BoundingBox");
    result.min = LuaGetArgument_Vector3(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "max") == LUA_TTABLE, index, "Expected BoundingBox");
    result.max = LuaGetArgument_Vector3(L, -1);
    lua_pop(L, 2);
    return result;
}

static Ray LuaGetArgument_Ray(lua_State* L, int index)
{
    Ray result;
    index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values
    luaL_argcheck(L, lua_getfield(L, index, "position") == LUA_TTABLE, index, "Expected Ray");
    result.position = LuaGetArgument_Vector3(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "direction") == LUA_TTABLE, index, "Expected Ray");
    result.direction = LuaGetArgument_Vector3(L, -1);
    lua_pop(L, 2);
    return result;
}

static Matrix LuaGetArgument_Matrix(lua_State* L, int index)
{
    Matrix result = { 0 };
    float* ptr = &result.m0;
    index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values

    for (int i = 0; i < 16; i++)
    {
        lua_geti(L, index, i+1);
        ptr[i] = luaL_checknumber(L, -1);
    }
    lua_pop(L, 16);
    return result;
}

static Material LuaGetArgument_Material(lua_State* L, int index)
{
    Material result;
        index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values
    luaL_argcheck(L, lua_getfield(L, index, "shader") == LUA_TUSERDATA, index, "Expected Material");
    result.shader = LuaGetArgument_Shader(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "texDiffuse") == LUA_TUSERDATA, index, "Expected Material");
    result.texDiffuse = LuaGetArgument_Texture2D(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "texNormal") == LUA_TUSERDATA, index, "Expected Material");
    result.texNormal = LuaGetArgument_Texture2D(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "texSpecular") == LUA_TUSERDATA, index, "Expected Material");
    result.texSpecular = LuaGetArgument_Texture2D(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "colDiffuse") == LUA_TTABLE, index, "Expected Material");
    result.colDiffuse = LuaGetArgument_Color(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "colAmbient") == LUA_TTABLE, index, "Expected Material");
    result.colAmbient = LuaGetArgument_Color(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "colSpecular") == LUA_TTABLE, index, "Expected Material");
    result.colSpecular = LuaGetArgument_Color(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "glossiness") == LUA_TNUMBER, index, "Expected Material");
    result.glossiness = LuaGetArgument_float(L, -1);
    lua_pop(L, 8);
    return result;
}

static Model LuaGetArgument_Model(lua_State* L, int index)
{
    Model result;
    index = lua_absindex(L, index); // Makes sure we use absolute indices because we push multiple values
    luaL_argcheck(L, lua_getfield(L, index, "mesh") == LUA_TUSERDATA, index, "Expected Model");
    result.mesh = LuaGetArgument_Mesh(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "transform") == LUA_TTABLE, index, "Expected Model");
    result.transform = LuaGetArgument_Matrix(L, -1);
    luaL_argcheck(L, lua_getfield(L, index, "material") == LUA_TTABLE, index, "Expected Model");
    result.material = LuaGetArgument_Material(L, -1);
    lua_pop(L, 3);
    return result;
}

//----------------------------------------------------------------------------------
// LuaPush functions
//----------------------------------------------------------------------------------
static void LuaPush_Color(lua_State* L, Color color)
{
    lua_createtable(L, 0, 4);
    lua_pushinteger(L, color.r);
    lua_setfield(L, -2, "r");
    lua_pushinteger(L, color.g);
    lua_setfield(L, -2, "g");
    lua_pushinteger(L, color.b);
    lua_setfield(L, -2, "b");
    lua_pushinteger(L, color.a);
    lua_setfield(L, -2, "a");
}

static void LuaPush_Vector2(lua_State* L, Vector2 vec)
{
    lua_createtable(L, 0, 2);
    lua_pushnumber(L, vec.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, vec.y);
    lua_setfield(L, -2, "y");
}

static void LuaPush_Vector3(lua_State* L, Vector3 vec)
{
    lua_createtable(L, 0, 3);
    lua_pushnumber(L, vec.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, vec.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, vec.z);
    lua_setfield(L, -2, "z");
}

static void LuaPush_Quaternion(lua_State* L, Quaternion vec)
{
    lua_createtable(L, 0, 4);
    lua_pushnumber(L, vec.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, vec.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, vec.z);
    lua_setfield(L, -2, "z");
    lua_pushnumber(L, vec.w);
    lua_setfield(L, -2, "w");
}

static void LuaPush_Matrix(lua_State* L, Matrix *matrix)
{
    int i;
    lua_createtable(L, 16, 0);
    float* num = (&matrix->m0);
    for (i = 0; i < 16; i++)
    {
        lua_pushnumber(L, num[i]);
        lua_rawseti(L, -2, i + 1);
    }
}

static void LuaPush_Rectangle(lua_State* L, Rectangle rect)
{
    lua_createtable(L, 0, 4);
    lua_pushinteger(L, rect.x);
    lua_setfield(L, -2, "x");
    lua_pushinteger(L, rect.y);
    lua_setfield(L, -2, "y");
    lua_pushinteger(L, rect.width);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, rect.height);
    lua_setfield(L, -2, "height");
}

static void LuaPush_Ray(lua_State* L, Ray ray)
{
    lua_createtable(L, 0, 2);
    LuaPush_Vector3(L, ray.position);
    lua_setfield(L, -2, "position");
    LuaPush_Vector3(L, ray.direction);
    lua_setfield(L, -2, "direction");
}

static void LuaPush_BoundingBox(lua_State* L, BoundingBox bb)
{
    lua_createtable(L, 0, 2);
    LuaPush_Vector3(L, bb.min);
    lua_setfield(L, -2, "min");
    LuaPush_Vector3(L, bb.max);
    lua_setfield(L, -2, "max");
}

static void LuaPush_Camera(lua_State* L, Camera cam)
{
    lua_createtable(L, 0, 4);
    LuaPush_Vector3(L, cam.position);
    lua_setfield(L, -2, "position");
    LuaPush_Vector3(L, cam.target);
    lua_setfield(L, -2, "target");
    LuaPush_Vector3(L, cam.up);
    lua_setfield(L, -2, "up");
    lua_pushnumber(L, cam.fovy);
    lua_setfield(L, -2, "fovy");
}

static void LuaPush_Camera2D(lua_State* L, Camera2D cam)
{
    lua_createtable(L, 0, 4);
    LuaPush_Vector2(L, cam.offset);
    lua_setfield(L, -2, "offset");
    LuaPush_Vector2(L, cam.target);
    lua_setfield(L, -2, "target");
    lua_pushnumber(L, cam.rotation);
    lua_setfield(L, -2, "rotation");
    lua_pushnumber(L, cam.zoom);
    lua_setfield(L, -2, "zoom");
}

static void LuaPush_Material(lua_State* L, Material mat)
{
    lua_createtable(L, 0, 8);
    LuaPush_Shader(L, mat.shader);
    lua_setfield(L, -2, "shader");
    LuaPush_Texture2D(L, mat.texDiffuse);
    lua_setfield(L, -2, "texDiffuse");
    LuaPush_Texture2D(L, mat.texNormal);
    lua_setfield(L, -2, "texNormal");
    LuaPush_Texture2D(L, mat.texSpecular);
    lua_setfield(L, -2, "texSpecular");
    LuaPush_Color(L, mat.colDiffuse);
    lua_setfield(L, -2, "colDiffuse");
    LuaPush_Color(L, mat.colAmbient);
    lua_setfield(L, -2, "colAmbient");
    LuaPush_Color(L, mat.colSpecular);
    lua_setfield(L, -2, "colSpecular");
    lua_pushnumber(L, mat.glossiness);
    lua_setfield(L, -2, "glossiness");
}

static void LuaPush_Model(lua_State* L, Model mdl)
{
    lua_createtable(L, 0, 4);
    LuaPush_Mesh(L, mdl.mesh);
    lua_setfield(L, -2, "mesh");
    LuaPush_Matrix(L, &mdl.transform);
    lua_setfield(L, -2, "transform");
    LuaPush_Material(L, mdl.material);
    lua_setfield(L, -2, "material");
}

//----------------------------------------------------------------------------------
// raylib Lua Structure constructors
//----------------------------------------------------------------------------------
static int lua_Color(lua_State* L)
{
    LuaPush_Color(L, (Color) { (unsigned char)luaL_checkinteger(L, 1), (unsigned char)luaL_checkinteger(L, 2), (unsigned char)luaL_checkinteger(L, 3), (unsigned char)luaL_checkinteger(L, 4) });
    return 1;
}

static int lua_Vector2(lua_State* L)
{
    LuaPush_Vector2(L, (Vector2) { (float)luaL_checknumber(L, 1), (float)luaL_checknumber(L, 2) });
    return 1;
}

static int lua_Vector3(lua_State* L)
{
    LuaPush_Vector3(L, (Vector3) { (float)luaL_checknumber(L, 1), (float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3) });
    return 1;
}

static int lua_Quaternion(lua_State* L)
{
    LuaPush_Quaternion(L, (Quaternion) { (float)luaL_checknumber(L, 1), (float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4) });
    return 1;
}

static int lua_Rectangle(lua_State* L)
{
    LuaPush_Rectangle(L, (Rectangle) { (int)luaL_checkinteger(L, 1), (int)luaL_checkinteger(L, 2), (int)luaL_checkinteger(L, 3), (int)luaL_checkinteger(L, 4) });
    return 1;
}

static int lua_Ray(lua_State* L)
{
    Vector2 pos = LuaGetArgument_Vector2(L, 1);
    Vector2 dir = LuaGetArgument_Vector2(L, 2);
    LuaPush_Ray(L, (Ray) { { pos.x, pos.y }, { dir.x, dir.y } });
    return 1;
}

static int lua_BoundingBox(lua_State* L)
{
    Vector3 min = LuaGetArgument_Vector3(L, 1);
    Vector3 max = LuaGetArgument_Vector3(L, 2);
    LuaPush_BoundingBox(L, (BoundingBox) { { min.x, min.y, min.z }, { max.x, max.y, max.z } });
    return 1;
}

static int lua_Camera(lua_State* L)
{
    Vector3 pos = LuaGetArgument_Vector3(L, 1);
    Vector3 tar = LuaGetArgument_Vector3(L, 2);
    Vector3 up = LuaGetArgument_Vector3(L, 3);
    float fovy = LuaGetArgument_float(L, 4);
    LuaPush_Camera(L, (Camera) { { pos.x, pos.y, pos.z }, { tar.x, tar.y, tar.z }, { up.x, up.y, up.z }, fovy });
    return 1;
}

static int lua_Camera2D(lua_State* L)
{
    Vector2 off = LuaGetArgument_Vector2(L, 1);
    Vector2 tar = LuaGetArgument_Vector2(L, 2);
    float rot = LuaGetArgument_float(L, 3);
    float zoom = LuaGetArgument_float(L, 4);
    LuaPush_Camera2D(L, (Camera2D) { { off.x, off.y }, { tar.x, tar.y }, rot, zoom });
    return 1;
}

/*************************************************************************************
*  raylib Lua Functions Bindings
**************************************************************************************/

//------------------------------------------------------------------------------------
// raylib [core] module functions - Window and Graphics Device
//------------------------------------------------------------------------------------
int lua_InitWindow(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    const char * arg3 = LuaGetArgument_string(L, 3);
    InitWindow(arg1, arg2, arg3);
    return 0;
}

int lua_CloseWindow(lua_State* L)
{
    CloseWindow();
    return 0;
}

int lua_WindowShouldClose(lua_State* L)
{
    bool result = WindowShouldClose();
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsWindowMinimized(lua_State* L)
{
    bool result = IsWindowMinimized();
    lua_pushboolean(L, result);
    return 1;
}

int lua_ToggleFullscreen(lua_State* L)
{
    ToggleFullscreen();
    return 0;
}

int lua_GetScreenWidth(lua_State* L)
{
    int result = GetScreenWidth();
    lua_pushinteger(L, result);
    return 1;
}

int lua_GetScreenHeight(lua_State* L)
{
    int result = GetScreenHeight();
    lua_pushinteger(L, result);
    return 1;
}

int lua_ShowCursor(lua_State* L)
{
    ShowCursor();
    return 0;
}

int lua_HideCursor(lua_State* L)
{
    HideCursor();
    return 0;
}

int lua_IsCursorHidden(lua_State* L)
{
    bool result = IsCursorHidden();
    lua_pushboolean(L, result);
    return 1;
}

int lua_EnableCursor(lua_State* L)
{
    EnableCursor();
    return 0;
}

int lua_DisableCursor(lua_State* L)
{
    DisableCursor();
    return 0;
}

int lua_ClearBackground(lua_State* L)
{
    Color arg1 = LuaGetArgument_Color(L, 1);
    ClearBackground(arg1);
    return 0;
}

int lua_BeginDrawing(lua_State* L)
{
    BeginDrawing();
    return 0;
}

int lua_EndDrawing(lua_State* L)
{
    EndDrawing();
    return 0;
}

int lua_Begin2dMode(lua_State* L)
{
    Camera2D arg1 = LuaGetArgument_Camera2D(L, 1);
    Begin2dMode(arg1);
    return 0;
}

int lua_End2dMode(lua_State* L)
{
    End2dMode();
    return 0;
}

int lua_Begin3dMode(lua_State* L)
{
    Camera arg1 = LuaGetArgument_Camera(L, 1);
    Begin3dMode(arg1);
    return 0;
}

int lua_End3dMode(lua_State* L)
{
    End3dMode();
    return 0;
}

int lua_BeginTextureMode(lua_State* L)
{
    RenderTexture2D arg1 = LuaGetArgument_RenderTexture2D(L, 1);
    BeginTextureMode(arg1);
    return 0;
}

int lua_EndTextureMode(lua_State* L)
{
    EndTextureMode();
    return 0;
}

int lua_GetMouseRay(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    Camera arg2 = LuaGetArgument_Camera(L, 2);
    Ray result = GetMouseRay(arg1, arg2);
    LuaPush_Ray(L, result);
    return 1;
}

int lua_GetWorldToScreen(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Camera arg2 = LuaGetArgument_Camera(L, 2);
    Vector2 result = GetWorldToScreen(arg1, arg2);
    LuaPush_Vector2(L, result);
    return 1;
}

int lua_GetCameraMatrix(lua_State* L)
{
    Camera arg1 = LuaGetArgument_Camera(L, 1);
    Matrix result = GetCameraMatrix(arg1);
    LuaPush_Matrix(L, &result);
    return 1;
}

#if defined(PLATFORM_WEB)

static int LuaDrawLoopFunc;

static void LuaDrawLoop()
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, LuaDrawLoopFunc);
    lua_call(L, 0, 0);
}

int lua_SetDrawingLoop(lua_State* L)
{
    luaL_argcheck(L, lua_isfunction(L, 1), 1, "Loop function expected");
    lua_pushvalue(L, 1);
    LuaDrawLoopFunc = luaL_ref(L, LUA_REGISTRYINDEX);
    SetDrawingLoop(&LuaDrawLoop);
    return 0;
}

#else

int lua_SetTargetFPS(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    SetTargetFPS(arg1);
    return 0;
}
#endif

int lua_GetFPS(lua_State* L)
{
    float result = GetFPS();
    lua_pushnumber(L, result);
    return 1;
}

int lua_GetFrameTime(lua_State* L)
{
    float result = GetFrameTime();
    lua_pushnumber(L, result);
    return 1;
}

int lua_GetColor(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    Color result = GetColor(arg1);
    LuaPush_Color(L, result);
    return 1;
}

int lua_GetHexValue(lua_State* L)
{
    Color arg1 = LuaGetArgument_Color(L, 1);
    int result = GetHexValue(arg1);
    lua_pushinteger(L, result);
    return 1;
}

int lua_ColorToFloat(lua_State* L)
{
    Color arg1 = LuaGetArgument_Color(L, 1);
    float * result = ColorToFloat(arg1);
    lua_createtable(L, 4, 0);
    for (int i = 0; i < 4; i++)
    {
        lua_pushnumber(L, result[i]);
        lua_rawseti(L, -2, i + 1);
    }
    free(result);
    return 1;
}

int lua_VectorToFloat(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float * result = VectorToFloat(arg1);
    lua_createtable(L, 3, 0);
    for (int i = 0; i < 3; i++)
    {
        lua_pushnumber(L, result[i]);
        lua_rawseti(L, -2, i + 1);
    }
    free(result);
    return 1;
}

int lua_MatrixToFloat(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    float * result = MatrixToFloat(arg1);
    lua_createtable(L, 16, 0);
    for (int i = 0; i < 16; i++)
    {
        lua_pushnumber(L, result[i]);
        lua_rawseti(L, -2, i + 1);
    }
    free(result);
    return 1;
}

int lua_GetRandomValue(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int result = GetRandomValue(arg1, arg2);
    lua_pushinteger(L, result);
    return 1;
}

int lua_Fade(lua_State* L)
{
    Color arg1 = LuaGetArgument_Color(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    Color result = Fade(arg1, arg2);
    LuaPush_Color(L, result);
    return 1;
}

int lua_SetConfigFlags(lua_State* L)
{
    char arg1 = LuaGetArgument_char(L, 1);
    SetConfigFlags(arg1);
    return 0;
}

int lua_ShowLogo(lua_State* L)
{
    ShowLogo();
    return 0;
}

int lua_IsFileDropped(lua_State* L)
{
    bool result = IsFileDropped();
    lua_pushboolean(L, result);
    return 1;
}

int lua_GetDroppedFiles(lua_State* L)
{
    int count = 0;
    char ** result = GetDroppedFiles(&count);
    lua_createtable(L, count, 0);
    for (int i = 0; i < count; i++)
    {
        lua_pushstring(L, result[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

int lua_ClearDroppedFiles(lua_State* L)
{
    ClearDroppedFiles();
    return 0;
}

int lua_StorageSaveValue(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    StorageSaveValue(arg1, arg2);
    return 0;
}

int lua_StorageLoadValue(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int result = StorageLoadValue(arg1);
    lua_pushinteger(L, result);
    return 1;
}

//------------------------------------------------------------------------------------
// raylib [core] module functions - Input Handling
//------------------------------------------------------------------------------------
int lua_IsKeyPressed(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsKeyPressed(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsKeyDown(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsKeyDown(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsKeyReleased(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsKeyReleased(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsKeyUp(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsKeyUp(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_GetKeyPressed(lua_State* L)
{
    int result = GetKeyPressed();
    lua_pushinteger(L, result);
    return 1;
}

int lua_SetExitKey(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    SetExitKey(arg1);
    return 0;
}

int lua_IsGamepadAvailable(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsGamepadAvailable(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsGamepadName(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    const char * arg2 = LuaGetArgument_string(L, 2);
    bool result = IsGamepadName(arg1, arg2);
    lua_pushboolean(L, result);
    return 1;
}

int lua_GetGamepadName(lua_State* L)
{
    // TODO: Return gamepad name id
    
    int arg1 = LuaGetArgument_int(L, 1);
    char * result = GetGamepadName(arg1);
    //lua_pushboolean(L, result);
    return 1;
}

int lua_IsGamepadButtonPressed(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    bool result = IsGamepadButtonPressed(arg1, arg2);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsGamepadButtonDown(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    bool result = IsGamepadButtonDown(arg1, arg2);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsGamepadButtonReleased(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    bool result = IsGamepadButtonReleased(arg1, arg2);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsGamepadButtonUp(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    bool result = IsGamepadButtonUp(arg1, arg2);
    lua_pushboolean(L, result);
    return 1;
}

int lua_GetGamepadButtonPressed(lua_State* L)
{
    int result = GetGamepadButtonPressed();
    lua_pushinteger(L, result);
    return 1;
}

int lua_GetGamepadAxisCount(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int result = GetGamepadAxisCount(arg1);
    lua_pushinteger(L, result);
    return 1;
}

int lua_GetGamepadAxisMovement(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    float result = GetGamepadAxisMovement(arg1, arg2);
    lua_pushnumber(L, result);
    return 1;
}

int lua_IsMouseButtonPressed(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsMouseButtonPressed(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsMouseButtonDown(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsMouseButtonDown(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsMouseButtonReleased(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsMouseButtonReleased(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsMouseButtonUp(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsMouseButtonUp(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_GetMouseX(lua_State* L)
{
    int result = GetMouseX();
    lua_pushinteger(L, result);
    return 1;
}

int lua_GetMouseY(lua_State* L)
{
    int result = GetMouseY();
    lua_pushinteger(L, result);
    return 1;
}

int lua_GetMousePosition(lua_State* L)
{
    Vector2 result = GetMousePosition();
    LuaPush_Vector2(L, result);
    return 1;
}

int lua_SetMousePosition(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    SetMousePosition(arg1);
    return 0;
}

int lua_GetMouseWheelMove(lua_State* L)
{
    int result = GetMouseWheelMove();
    lua_pushinteger(L, result);
    return 1;
}

int lua_GetTouchX(lua_State* L)
{
    int result = GetTouchX();
    lua_pushinteger(L, result);
    return 1;
}

int lua_GetTouchY(lua_State* L)
{
    int result = GetTouchY();
    lua_pushinteger(L, result);
    return 1;
}

int lua_GetTouchPosition(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    Vector2 result = GetTouchPosition(arg1);
    LuaPush_Vector2(L, result);
    return 1;
}


#if defined(PLATFORM_ANDROID)
int lua_IsButtonPressed(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsButtonPressed(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsButtonDown(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsButtonDown(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsButtonReleased(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsButtonReleased(arg1);
    lua_pushboolean(L, result);
    return 1;
}
#endif

//------------------------------------------------------------------------------------
// raylib [gestures] module functions - Gestures and Touch Handling
//------------------------------------------------------------------------------------
int lua_SetGesturesEnabled(lua_State* L)
{
    unsigned arg1 = LuaGetArgument_unsigned(L, 1);
    SetGesturesEnabled(arg1);
    return 0;
}

int lua_IsGestureDetected(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    bool result = IsGestureDetected(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_GetTouchPointsCount(lua_State* L)
{
    int result = GetTouchPointsCount();
    lua_pushinteger(L, result);
    return 1;
}

int lua_GetGestureDetected(lua_State* L)
{
    int result = GetGestureDetected();
    lua_pushinteger(L, result);
    return 1;
}

int lua_GetGestureHoldDuration(lua_State* L)
{
    int result = GetGestureHoldDuration();
    lua_pushinteger(L, result);
    return 1;
}

int lua_GetGestureDragVector(lua_State* L)
{
    Vector2 result = GetGestureDragVector();
    LuaPush_Vector2(L, result);
    return 1;
}

int lua_GetGestureDragAngle(lua_State* L)
{
    float result = GetGestureDragAngle();
    lua_pushnumber(L, result);
    return 1;
}

int lua_GetGesturePinchVector(lua_State* L)
{
    Vector2 result = GetGesturePinchVector();
    LuaPush_Vector2(L, result);
    return 1;
}

int lua_GetGesturePinchAngle(lua_State* L)
{
    float result = GetGesturePinchAngle();
    lua_pushnumber(L, result);
    return 1;
}

//------------------------------------------------------------------------------------
// raylib [camera] module functions - Camera System
//------------------------------------------------------------------------------------
int lua_SetCameraMode(lua_State* L)
{
    Camera arg1 = LuaGetArgument_Camera(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    SetCameraMode(arg1, arg2);
    return 0;
}

int lua_UpdateCamera(lua_State* L)
{
    Camera arg1 = LuaGetArgument_Camera(L, 1);
    UpdateCamera(&arg1);
    LuaPush_Camera(L, arg1);
    return 1;
}

int lua_SetCameraPanControl(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    SetCameraPanControl(arg1);
    return 0;
}

int lua_SetCameraAltControl(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    SetCameraAltControl(arg1);
    return 0;
}

int lua_SetCameraSmoothZoomControl(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    SetCameraSmoothZoomControl(arg1);
    return 0;
}

int lua_SetCameraMoveControls(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    int arg5 = LuaGetArgument_int(L, 5);
    int arg6 = LuaGetArgument_int(L, 6);
    SetCameraMoveControls(arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

//------------------------------------------------------------------------------------
// raylib [shapes] module functions - Basic Shapes Drawing
//------------------------------------------------------------------------------------
int lua_DrawPixel(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    Color arg3 = LuaGetArgument_Color(L, 3);
    DrawPixel(arg1, arg2, arg3);
    return 0;
}

int lua_DrawPixelV(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    Color arg2 = LuaGetArgument_Color(L, 2);
    DrawPixelV(arg1, arg2);
    return 0;
}

int lua_DrawLine(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawLine(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawLineV(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    Vector2 arg2 = LuaGetArgument_Vector2(L, 2);
    Color arg3 = LuaGetArgument_Color(L, 3);
    DrawLineV(arg1, arg2, arg3);
    return 0;
}

int lua_DrawCircle(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    Color arg4 = LuaGetArgument_Color(L, 4);
    DrawCircle(arg1, arg2, arg3, arg4);
    return 0;
}

int lua_DrawCircleGradient(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    Color arg4 = LuaGetArgument_Color(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawCircleGradient(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawCircleV(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    Color arg3 = LuaGetArgument_Color(L, 3);
    DrawCircleV(arg1, arg2, arg3);
    return 0;
}

int lua_DrawCircleLines(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    Color arg4 = LuaGetArgument_Color(L, 4);
    DrawCircleLines(arg1, arg2, arg3, arg4);
    return 0;
}

int lua_DrawRectangle(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawRectangle(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawRectangleRec(lua_State* L)
{
    Rectangle arg1 = LuaGetArgument_Rectangle(L, 1);
    Color arg2 = LuaGetArgument_Color(L, 2);
    DrawRectangleRec(arg1, arg2);
    return 0;
}

int lua_DrawRectangleGradient(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    Color arg6 = LuaGetArgument_Color(L, 6);
    DrawRectangleGradient(arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

int lua_DrawRectangleV(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    Vector2 arg2 = LuaGetArgument_Vector2(L, 2);
    Color arg3 = LuaGetArgument_Color(L, 3);
    DrawRectangleV(arg1, arg2, arg3);
    return 0;
}

int lua_DrawRectangleLines(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawRectangleLines(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawTriangle(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    Vector2 arg2 = LuaGetArgument_Vector2(L, 2);
    Vector2 arg3 = LuaGetArgument_Vector2(L, 3);
    Color arg4 = LuaGetArgument_Color(L, 4);
    DrawTriangle(arg1, arg2, arg3, arg4);
    return 0;
}

int lua_DrawTriangleLines(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    Vector2 arg2 = LuaGetArgument_Vector2(L, 2);
    Vector2 arg3 = LuaGetArgument_Vector2(L, 3);
    Color arg4 = LuaGetArgument_Color(L, 4);
    DrawTriangleLines(arg1, arg2, arg3, arg4);
    return 0;
}

int lua_DrawPoly(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawPoly(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

#define GET_TABLE(type, name, index) \
    type* name = 0; \
    size_t name##_size = 0; \
    { \
        size_t sz = 0; \
        luaL_checktype(L, index, LUA_TTABLE); \
        lua_pushnil(L); \
        while (lua_next(L, index)) { \
            LuaGetArgument_##type(L, -1); \
            sz++; \
            lua_pop(L, 1); \
        } \
        name = calloc(sz, sizeof(type)); \
        sz = 0; \
        lua_pushnil(L); \
        while (lua_next(L, index)) { \
            name[sz] = LuaGetArgument_##type(L, -1); \
            sz++; \
            lua_pop(L, 1); \
        } \
        lua_pop(L, 1); \
        name##_size = sz; \
    }


int lua_DrawPolyEx(lua_State* L)
{
    GET_TABLE(Vector2, arg1, 1);
    Color arg2 = LuaGetArgument_Color(L, 2);
    DrawPolyEx(arg1, arg1_size, arg2);
    free(arg1);
    return 0;
}

int lua_DrawPolyExLines(lua_State* L)
{
    GET_TABLE(Vector2, arg1, 1);
    Color arg2 = LuaGetArgument_Color(L, 2);
    DrawPolyExLines(arg1, arg1_size, arg2);
    free(arg1);
    return 0;
}

int lua_CheckCollisionRecs(lua_State* L)
{
    Rectangle arg1 = LuaGetArgument_Rectangle(L, 1);
    Rectangle arg2 = LuaGetArgument_Rectangle(L, 2);
    bool result = CheckCollisionRecs(arg1, arg2);
    lua_pushboolean(L, result);
    return 1;
}

int lua_CheckCollisionCircles(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    Vector2 arg3 = LuaGetArgument_Vector2(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    bool result = CheckCollisionCircles(arg1, arg2, arg3, arg4);
    lua_pushboolean(L, result);
    return 1;
}

int lua_CheckCollisionCircleRec(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    Rectangle arg3 = LuaGetArgument_Rectangle(L, 3);
    bool result = CheckCollisionCircleRec(arg1, arg2, arg3);
    lua_pushboolean(L, result);
    return 1;
}

int lua_GetCollisionRec(lua_State* L)
{
    Rectangle arg1 = LuaGetArgument_Rectangle(L, 1);
    Rectangle arg2 = LuaGetArgument_Rectangle(L, 2);
    Rectangle result = GetCollisionRec(arg1, arg2);
    LuaPush_Rectangle(L, result);
    return 1;
}

int lua_CheckCollisionPointRec(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    Rectangle arg2 = LuaGetArgument_Rectangle(L, 2);
    bool result = CheckCollisionPointRec(arg1, arg2);
    lua_pushboolean(L, result);
    return 1;
}

int lua_CheckCollisionPointCircle(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    Vector2 arg2 = LuaGetArgument_Vector2(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    bool result = CheckCollisionPointCircle(arg1, arg2, arg3);
    lua_pushboolean(L, result);
    return 1;
}

int lua_CheckCollisionPointTriangle(lua_State* L)
{
    Vector2 arg1 = LuaGetArgument_Vector2(L, 1);
    Vector2 arg2 = LuaGetArgument_Vector2(L, 2);
    Vector2 arg3 = LuaGetArgument_Vector2(L, 3);
    Vector2 arg4 = LuaGetArgument_Vector2(L, 4);
    bool result = CheckCollisionPointTriangle(arg1, arg2, arg3, arg4);
    lua_pushboolean(L, result);
    return 1;
}

//------------------------------------------------------------------------------------
// raylib [textures] module functions - Texture Loading and Drawing
//------------------------------------------------------------------------------------
int lua_LoadImage(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    Image result = LoadImage(arg1);
    LuaPush_Image(L, result);
    return 1;
}

int lua_LoadImageEx(lua_State* L)
{
    // TODO: Image LoadImageEx(Color *pixels, int width, int height);
    
    GET_TABLE(Color, arg1, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    Image result = LoadImageEx(arg1, arg2, arg3); // ISSUE: #3 number expected, got no value
    LuaPush_Image(L, result);
    free(arg1);
    return 1;
}

int lua_LoadImageRaw(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    int arg5 = LuaGetArgument_int(L, 5);
    Image result = LoadImageRaw(arg1, arg2, arg3, arg4, arg5);
    LuaPush_Image(L, result);
    return 1;
}

int lua_LoadImageFromRES(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    Image result = LoadImageFromRES(arg1, arg2);
    LuaPush_Image(L, result);
    return 1;
}

int lua_LoadTexture(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    Texture2D result = LoadTexture(arg1);
    LuaPush_Texture2D(L, result);
    return 1;
}

int lua_LoadTextureEx(lua_State* L)
{
    void * arg1 = (char *)LuaGetArgument_string(L, 1);  // NOTE: getting argument as string?
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    Texture2D result = LoadTextureEx(arg1, arg2, arg3, arg4);
    LuaPush_Texture2D(L, result);
    return 1;
}

int lua_LoadTextureFromRES(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    Texture2D result = LoadTextureFromRES(arg1, arg2);
    LuaPush_Texture2D(L, result);
    return 1;
}

int lua_LoadTextureFromImage(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    Texture2D result = LoadTextureFromImage(arg1);
    LuaPush_Texture2D(L, result);
    return 1;
}

int lua_LoadRenderTexture(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    RenderTexture2D result = LoadRenderTexture(arg1, arg2);
    LuaPush_RenderTexture2D(L, result);
    return 1;
}

int lua_UnloadImage(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    UnloadImage(arg1);
    return 0;
}

int lua_UnloadTexture(lua_State* L)
{
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    UnloadTexture(arg1);
    return 0;
}

int lua_UnloadRenderTexture(lua_State* L)
{
    RenderTexture2D arg1 = LuaGetArgument_RenderTexture2D(L, 1);
    UnloadRenderTexture(arg1);
    return 0;
}

int lua_GetImageData(lua_State* L)
{
    // TODO: Color *GetImageData(Image image);
    
    Image arg1 = LuaGetArgument_Image(L, 1);
    Color * result = GetImageData(arg1);
    lua_createtable(L, arg1.width*arg1.height, 0);
    for (int i = 0; i < arg1.width*arg1.height; i++)
    {
        LuaPush_Color(L, result[i]);
        lua_rawseti(L, -2, i + 1);
    }
    free(result);
    return 1;
}

int lua_GetTextureData(lua_State* L)
{
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    Image result = GetTextureData(arg1);
    LuaPush_Image(L, result);
    return 1;
}

int lua_UpdateTexture(lua_State* L)
{
    // TODO: void UpdateTexture(Texture2D texture, void *pixels);
    
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    void * arg2 = (char *)LuaGetArgument_string(L, 2);  // NOTE: Getting (void *) as string?
    UpdateTexture(arg1, arg2);      // ISSUE: #2 string expected, got table -> GetImageData() returns a table!
    return 0;
}

int lua_ImageToPOT(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    Color arg2 = LuaGetArgument_Color(L, 2);
    ImageToPOT(&arg1, arg2);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageFormat(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    ImageFormat(&arg1, arg2);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageDither(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    int arg5 = LuaGetArgument_int(L, 5);
    ImageDither(&arg1, arg2, arg3, arg4, arg5);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageCopy(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    Image result = ImageCopy(arg1);
    LuaPush_Image(L, result);
    return 1;
}

int lua_ImageCrop(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    Rectangle arg2 = LuaGetArgument_Rectangle(L, 2);
    ImageCrop(&arg1, arg2);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageResize(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    ImageResize(&arg1, arg2, arg3);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageResizeNN(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    ImageResizeNN(&arg1, arg2, arg3);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageText(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    Color arg3 = LuaGetArgument_Color(L, 3);
    Image result = ImageText(arg1, arg2, arg3);
    LuaPush_Image(L, result);
    return 1;
}

int lua_ImageTextEx(lua_State* L)
{
    SpriteFont arg1 = LuaGetArgument_SpriteFont(L, 1);
    const char * arg2 = LuaGetArgument_string(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    Image result = ImageTextEx(arg1, arg2, arg3, arg4, arg5);
    LuaPush_Image(L, result);
    return 1;
}

int lua_ImageDraw(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    Image arg2 = LuaGetArgument_Image(L, 2);
    Rectangle arg3 = LuaGetArgument_Rectangle(L, 3);
    Rectangle arg4 = LuaGetArgument_Rectangle(L, 4);
    ImageDraw(&arg1, arg2, arg3, arg4);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageDrawText(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    Vector2 arg2 = LuaGetArgument_Vector2(L, 2);
    const char * arg3 = LuaGetArgument_string(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    ImageDrawText(&arg1, arg2, arg3, arg4, arg5);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageDrawTextEx(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    Vector2 arg2 = LuaGetArgument_Vector2(L, 2);
    SpriteFont arg3 = LuaGetArgument_SpriteFont(L, 3);
    const char * arg4 = LuaGetArgument_string(L, 4);
    float arg5 = LuaGetArgument_float(L, 5);
    int arg6 = LuaGetArgument_int(L, 6);
    Color arg7 = LuaGetArgument_Color(L, 7);
    ImageDrawTextEx(&arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageFlipVertical(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    ImageFlipVertical(&arg1);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageFlipHorizontal(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    ImageFlipHorizontal(&arg1);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageColorTint(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    Color arg2 = LuaGetArgument_Color(L, 2);
    ImageColorTint(&arg1, arg2);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageColorInvert(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    ImageColorInvert(&arg1);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageColorGrayscale(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    ImageColorGrayscale(&arg1);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageColorContrast(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    ImageColorContrast(&arg1, arg2);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_ImageColorBrightness(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    ImageColorBrightness(&arg1, arg2);
    LuaPush_Image(L, arg1);
    return 1;
}

int lua_GenTextureMipmaps(lua_State* L)
{
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    GenTextureMipmaps(&arg1);
    LuaPush_Texture2D(L, arg1);
    return 1;
}

int lua_SetTextureFilter(lua_State* L)
{
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    SetTextureFilter(arg1, arg2);
    return 0;
}

int lua_SetTextureWrap(lua_State* L)
{
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    SetTextureWrap(arg1, arg2);
    return 0;
}

int lua_DrawTexture(lua_State* L)
{
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    Color arg4 = LuaGetArgument_Color(L, 4);
    DrawTexture(arg1, arg2, arg3, arg4);
    return 0;
}

int lua_DrawTextureV(lua_State* L)
{
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    Vector2 arg2 = LuaGetArgument_Vector2(L, 2);
    Color arg3 = LuaGetArgument_Color(L, 3);
    DrawTextureV(arg1, arg2, arg3);
    return 0;
}

int lua_DrawTextureEx(lua_State* L)
{
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    Vector2 arg2 = LuaGetArgument_Vector2(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawTextureEx(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawTextureRec(lua_State* L)
{
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    Rectangle arg2 = LuaGetArgument_Rectangle(L, 2);
    Vector2 arg3 = LuaGetArgument_Vector2(L, 3);
    Color arg4 = LuaGetArgument_Color(L, 4);
    DrawTextureRec(arg1, arg2, arg3, arg4);
    return 0;
}

int lua_DrawTexturePro(lua_State* L)
{
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    Rectangle arg2 = LuaGetArgument_Rectangle(L, 2);
    Rectangle arg3 = LuaGetArgument_Rectangle(L, 3);
    Vector2 arg4 = LuaGetArgument_Vector2(L, 4);
    float arg5 = LuaGetArgument_float(L, 5);
    Color arg6 = LuaGetArgument_Color(L, 6);
    DrawTexturePro(arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

//------------------------------------------------------------------------------------
// raylib [text] module functions - Font Loading and Text Drawing
//------------------------------------------------------------------------------------
int lua_GetDefaultFont(lua_State* L)
{
    SpriteFont result = GetDefaultFont();
    LuaPush_SpriteFont(L, result);
    return 1;
}

int lua_LoadSpriteFont(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    SpriteFont result = LoadSpriteFont(arg1);
    LuaPush_SpriteFont(L, result);
    return 1;
}

int lua_LoadSpriteFontTTF(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    //LoadSpriteFontTTF(const char *fileName, int fontSize, int numChars, int *fontChars);
    SpriteFont result = LoadSpriteFontTTF(arg1, arg2, arg3, &arg4);
    LuaPush_SpriteFont(L, result);
    return 1;
}

int lua_UnloadSpriteFont(lua_State* L)
{
    SpriteFont arg1 = LuaGetArgument_SpriteFont(L, 1);
    UnloadSpriteFont(arg1);
    return 0;
}

int lua_DrawText(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawText(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawTextEx(lua_State* L)
{
    SpriteFont arg1 = LuaGetArgument_SpriteFont(L, 1);
    const char * arg2 = LuaGetArgument_string(L, 2);
    Vector2 arg3 = LuaGetArgument_Vector2(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    int arg5 = LuaGetArgument_int(L, 5);
    Color arg6 = LuaGetArgument_Color(L, 6);
    DrawTextEx(arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

int lua_MeasureText(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int result = MeasureText(arg1, arg2);
    lua_pushinteger(L, result);
    return 1;
}

int lua_MeasureTextEx(lua_State* L)
{
    SpriteFont arg1 = LuaGetArgument_SpriteFont(L, 1);
    const char * arg2 = LuaGetArgument_string(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    Vector2 result = MeasureTextEx(arg1, arg2, arg3, arg4);
    LuaPush_Vector2(L, result);
    return 1;
}

int lua_DrawFPS(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    DrawFPS(arg1, arg2);
    return 0;
}

// NOTE: FormatText() can be replaced by Lua function: string.format()
// NOTE: SubText() can be replaced by Lua function: string.sub()

//------------------------------------------------------------------------------------
// raylib [models] module functions - Basic 3d Shapes Drawing Functions
//------------------------------------------------------------------------------------
int lua_DrawLine3D(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    Color arg3 = LuaGetArgument_Color(L, 3);
    DrawLine3D(arg1, arg2, arg3);
    return 0;
}

int lua_DrawCircle3D(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    Vector3 arg3 = LuaGetArgument_Vector3(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawCircle3D(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawCube(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawCube(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawCubeV(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    Color arg3 = LuaGetArgument_Color(L, 3);
    DrawCubeV(arg1, arg2, arg3);
    return 0;
}

int lua_DrawCubeWires(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawCubeWires(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawCubeTexture(lua_State* L)
{
    Texture2D arg1 = LuaGetArgument_Texture2D(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    float arg5 = LuaGetArgument_float(L, 5);
    Color arg6 = LuaGetArgument_Color(L, 6);
    DrawCubeTexture(arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

int lua_DrawSphere(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    Color arg3 = LuaGetArgument_Color(L, 3);
    DrawSphere(arg1, arg2, arg3);
    return 0;
}

int lua_DrawSphereEx(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawSphereEx(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawSphereWires(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawSphereWires(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawCylinder(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    int arg5 = LuaGetArgument_int(L, 5);
    Color arg6 = LuaGetArgument_Color(L, 6);
    DrawCylinder(arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

int lua_DrawCylinderWires(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    int arg5 = LuaGetArgument_int(L, 5);
    Color arg6 = LuaGetArgument_Color(L, 6);
    DrawCylinderWires(arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

int lua_DrawPlane(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector2 arg2 = LuaGetArgument_Vector2(L, 2);
    Color arg3 = LuaGetArgument_Color(L, 3);
    DrawPlane(arg1, arg2, arg3);
    return 0;
}

int lua_DrawRay(lua_State* L)
{
    Ray arg1 = LuaGetArgument_Ray(L, 1);
    Color arg2 = LuaGetArgument_Color(L, 2);
    DrawRay(arg1, arg2);
    return 0;
}

int lua_DrawGrid(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    DrawGrid(arg1, arg2);
    return 0;
}

int lua_DrawGizmo(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    DrawGizmo(arg1);
    return 0;
}

int lua_DrawLight(lua_State* L)
{
    Light arg1 = LuaGetArgument_Light(L, 1);
    DrawLight(arg1);
    return 0;
}

//------------------------------------------------------------------------------------
// raylib [models] module functions
//------------------------------------------------------------------------------------
int lua_LoadModel(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    Model result = LoadModel(arg1);
    LuaPush_Model(L, result);
    return 1;
}

int lua_LoadModelEx(lua_State* L)
{
    Mesh arg1 = LuaGetArgument_Mesh(L, 1);
    bool arg2 = LuaGetArgument_int(L, 2);
    Model result = LoadModelEx(arg1, arg2);
    LuaPush_Model(L, result);
    return 1;
}

int lua_LoadModelFromRES(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    Model result = LoadModelFromRES(arg1, arg2);
    LuaPush_Model(L, result);
    return 1;
}

int lua_LoadHeightmap(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    Model result = LoadHeightmap(arg1, arg2);
    LuaPush_Model(L, result);
    return 1;
}

int lua_LoadCubicmap(lua_State* L)
{
    Image arg1 = LuaGetArgument_Image(L, 1);
    Model result = LoadCubicmap(arg1);
    LuaPush_Model(L, result);
    return 1;
}

int lua_UnloadModel(lua_State* L)
{
    Model arg1 = LuaGetArgument_Model(L, 1);
    UnloadModel(arg1);
    return 0;
}

// TODO: GenMesh*() functionality (not ready yet on raylib 1.6)

int lua_LoadMaterial(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    Material result = LoadMaterial(arg1);
    LuaPush_Material(L, result);
    return 1;
}

int lua_LoadDefaultMaterial(lua_State* L)
{
    Material result = LoadDefaultMaterial();
    LuaPush_Material(L, result);
    return 1;
}

int lua_LoadStandardMaterial(lua_State* L)
{
    Material result = LoadStandardMaterial();
    LuaPush_Material(L, result);
    return 1;
}

int lua_UnloadMaterial(lua_State* L)
{
    Material arg1 = LuaGetArgument_Material(L, 1);
    UnloadMaterial(arg1);
    return 0;
}

int lua_DrawModel(lua_State* L)
{
    Model arg1 = LuaGetArgument_Model(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    Color arg4 = LuaGetArgument_Color(L, 4);
    DrawModel(arg1, arg2, arg3, arg4);
    return 0;
}

int lua_DrawModelEx(lua_State* L)
{
    Model arg1 = LuaGetArgument_Model(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    Vector3 arg3 = LuaGetArgument_Vector3(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    Vector3 arg5 = LuaGetArgument_Vector3(L, 5);
    Color arg6 = LuaGetArgument_Color(L, 6);
    DrawModelEx(arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

int lua_DrawModelWires(lua_State* L)
{
    Model arg1 = LuaGetArgument_Model(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    Color arg4 = LuaGetArgument_Color(L, 4);
    DrawModelWires(arg1, arg2, arg3, arg4);
    return 0;
}

int lua_DrawModelWiresEx(lua_State* L)
{
    Model arg1 = LuaGetArgument_Model(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    Vector3 arg3 = LuaGetArgument_Vector3(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    Vector3 arg5 = LuaGetArgument_Vector3(L, 5);
    Color arg6 = LuaGetArgument_Color(L, 6);
    DrawModelWiresEx(arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

int lua_DrawBillboard(lua_State* L)
{
    Camera arg1 = LuaGetArgument_Camera(L, 1);
    Texture2D arg2 = LuaGetArgument_Texture2D(L, 2);
    Vector3 arg3 = LuaGetArgument_Vector3(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    Color arg5 = LuaGetArgument_Color(L, 5);
    DrawBillboard(arg1, arg2, arg3, arg4, arg5);
    return 0;
}

int lua_DrawBillboardRec(lua_State* L)
{
    Camera arg1 = LuaGetArgument_Camera(L, 1);
    Texture2D arg2 = LuaGetArgument_Texture2D(L, 2);
    Rectangle arg3 = LuaGetArgument_Rectangle(L, 3);
    Vector3 arg4 = LuaGetArgument_Vector3(L, 4);
    float arg5 = LuaGetArgument_float(L, 5);
    Color arg6 = LuaGetArgument_Color(L, 6);
    DrawBillboardRec(arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

int lua_CalculateBoundingBox(lua_State* L)
{
    Mesh arg1 = LuaGetArgument_Mesh(L, 1);
    BoundingBox result = CalculateBoundingBox(arg1);
    LuaPush_BoundingBox(L, result);
    return 1;
}

int lua_CheckCollisionSpheres(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    Vector3 arg3 = LuaGetArgument_Vector3(L, 3);
    float arg4 = LuaGetArgument_float(L, 4);
    bool result = CheckCollisionSpheres(arg1, arg2, arg3, arg4);
    lua_pushboolean(L, result);
    return 1;
}

int lua_CheckCollisionBoxes(lua_State* L)
{
    BoundingBox arg1 = LuaGetArgument_BoundingBox(L, 1);
    BoundingBox arg2 = LuaGetArgument_BoundingBox(L, 2);
    bool result = CheckCollisionBoxes(arg1, arg2);
    lua_pushboolean(L, result);
    return 1;
}

int lua_CheckCollisionBoxSphere(lua_State* L)
{
    BoundingBox arg1 = LuaGetArgument_BoundingBox(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    bool result = CheckCollisionBoxSphere(arg1, arg2, arg3);
    lua_pushboolean(L, result);
    return 1;
}

int lua_CheckCollisionRaySphere(lua_State* L)
{
    Ray arg1 = LuaGetArgument_Ray(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    bool result = CheckCollisionRaySphere(arg1, arg2, arg3);
    lua_pushboolean(L, result);
    return 1;
}

int lua_CheckCollisionRaySphereEx(lua_State* L)
{
    Ray arg1 = LuaGetArgument_Ray(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    Vector3 arg4 = LuaGetArgument_Vector3(L, 4);
    bool result = CheckCollisionRaySphereEx(arg1, arg2, arg3, &arg4);
    lua_pushboolean(L, result);
    LuaPush_Vector3(L, arg4);
    return 2;
}

int lua_CheckCollisionRayBox(lua_State* L)
{
    Ray arg1 = LuaGetArgument_Ray(L, 1);
    BoundingBox arg2 = LuaGetArgument_BoundingBox(L, 2);
    bool result = CheckCollisionRayBox(arg1, arg2);
    lua_pushboolean(L, result);
    return 1;
}

//------------------------------------------------------------------------------------
// raylib [raymath] module functions - Shaders
//------------------------------------------------------------------------------------
int lua_LoadShader(lua_State* L)
{
    char * arg1 = (char *)LuaGetArgument_string(L, 1);
    char * arg2 = (char *)LuaGetArgument_string(L, 2);
    Shader result = LoadShader(arg1, arg2);
    LuaPush_Shader(L, result);
    return 1;
}

int lua_UnloadShader(lua_State* L)
{
    Shader arg1 = LuaGetArgument_Shader(L, 1);
    UnloadShader(arg1);
    return 0;
}

int lua_GetDefaultShader(lua_State* L)
{
    Shader result = GetDefaultShader();
    LuaPush_Shader(L, result);
    return 1;
}

int lua_GetStandardShader(lua_State* L)
{
    Shader result = GetStandardShader();
    LuaPush_Shader(L, result);
    return 1;
}

int lua_GetDefaultTexture(lua_State* L)
{
    Texture2D result = GetDefaultTexture();
    LuaPush_Texture2D(L, result);
    return 1;
}

int lua_GetShaderLocation(lua_State* L)
{
    Shader arg1 = LuaGetArgument_Shader(L, 1);
    const char * arg2 = LuaGetArgument_string(L, 2);
    int result = GetShaderLocation(arg1, arg2);
    lua_pushinteger(L, result);
    return 1;
}

int lua_SetShaderValue(lua_State* L)
{
    Shader arg1 = LuaGetArgument_Shader(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    GET_TABLE(float, arg3, 3);
    SetShaderValue(arg1, arg2, arg3, arg3_size);
    free(arg3);
    return 0;
}

int lua_SetShaderValuei(lua_State* L)
{
    Shader arg1 = LuaGetArgument_Shader(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    GET_TABLE(int, arg3, 3);
    SetShaderValuei(arg1, arg2, arg3, arg3_size);
    free(arg3);
    return 0;
}

int lua_SetShaderValueMatrix(lua_State* L)
{
    Shader arg1 = LuaGetArgument_Shader(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    Matrix arg3 = LuaGetArgument_Matrix(L, 3);
    SetShaderValueMatrix(arg1, arg2, arg3);
    return 0;
}

int lua_SetMatrixProjection(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    SetMatrixProjection(arg1);
    return 0;
}

int lua_SetMatrixModelview(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    SetMatrixModelview(arg1);
    return 0;
}

int lua_BeginShaderMode(lua_State* L)
{
    Shader arg1 = LuaGetArgument_Shader(L, 1);
    BeginShaderMode(arg1);
    return 0;
}

int lua_EndShaderMode(lua_State* L)
{
    EndShaderMode();
    return 0;
}

int lua_BeginBlendMode(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    BeginBlendMode(arg1);
    return 0;
}

int lua_EndBlendMode(lua_State* L)
{
    EndBlendMode();
    return 0;
}

int lua_CreateLight(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    Color arg3 = LuaGetArgument_Color(L, 3);
    Light result = CreateLight(arg1, arg2, arg3);
    LuaPush_Light(L, result);
    return 1;
}

int lua_DestroyLight(lua_State* L)
{
    Light arg1 = LuaGetArgument_Light(L, 1);
    DestroyLight(arg1);
    return 0;
}


//------------------------------------------------------------------------------------
// raylib [rlgl] module functions - VR experience
//------------------------------------------------------------------------------------
int lua_InitVrDevice(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    InitVrDevice(arg1);
    return 0;
}

int lua_CloseVrDevice(lua_State* L)
{
    CloseVrDevice();
    return 0;
}

int lua_IsVrDeviceReady(lua_State* L)
{
    bool result = IsVrDeviceReady();
    lua_pushboolean(L, result);
    return 1;
}

int lua_IsVrSimulator(lua_State* L)
{
    bool result = IsVrSimulator();
    lua_pushboolean(L, result);
    return 1;
}

int lua_UpdateVrTracking(lua_State* L)
{
    Camera arg1 = LuaGetArgument_Camera(L, 1);
    UpdateVrTracking(&arg1);
    LuaPush_Camera(L, arg1);
    return 1;
}

int lua_ToggleVrMode(lua_State* L)
{
    ToggleVrMode();
    return 0;
}

//------------------------------------------------------------------------------------
// raylib [audio] module functions - Audio Loading and Playing
//------------------------------------------------------------------------------------
int lua_InitAudioDevice(lua_State* L)
{
    InitAudioDevice();
    return 0;
}

int lua_CloseAudioDevice(lua_State* L)
{
    CloseAudioDevice();
    return 0;
}

int lua_IsAudioDeviceReady(lua_State* L)
{
    bool result = IsAudioDeviceReady();
    lua_pushboolean(L, result);
    return 1;
}

int lua_LoadWave(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    Wave result = LoadWave((char *)arg1);
    LuaPush_Wave(L, result);
    return 1;
}

int lua_LoadWaveEx(lua_State* L)
{
    // TODO: Wave LoadWaveEx(float *data, int sampleCount, int sampleRate, int sampleSize, int channels);
    
    int arg1 = 0;
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    int arg5 = LuaGetArgument_int(L, 5);
    Wave result = LoadWaveEx(arg1, arg2, arg3, arg4, arg5);
    LuaPush_Wave(L, result);
    return 1;
}

int lua_LoadSound(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    Sound result = LoadSound((char*)arg1);
    LuaPush_Sound(L, result);
    return 1;
}

int lua_LoadSoundFromWave(lua_State* L)
{
    Wave arg1 = LuaGetArgument_Wave(L, 1);
    Sound result = LoadSoundFromWave(arg1);
    LuaPush_Sound(L, result);
    return 1;
}

int lua_LoadSoundFromRES(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    Sound result = LoadSoundFromRES(arg1, arg2);
    LuaPush_Sound(L, result);
    return 1;
}

int lua_UpdateSound(lua_State* L)
{
    // TODO: void UpdateSound(Sound sound, void *data, int numSamples);
    
    Sound arg1 = LuaGetArgument_Sound(L, 1);
    const char * arg2 = LuaGetArgument_string(L, 2);
    int * arg3 = LuaGetArgument_int(L, 3);
    UpdateSound(arg1, arg2, arg3);
    return 0;
}

int lua_UnloadWave(lua_State* L)
{
    Wave arg1 = LuaGetArgument_Wave(L, 1);
    UnloadWave(arg1);
    return 0;
}

int lua_UnloadSound(lua_State* L)
{
    Sound arg1 = LuaGetArgument_Sound(L, 1);
    UnloadSound(arg1);
    return 0;
}

int lua_PlaySound(lua_State* L)
{
    Sound arg1 = LuaGetArgument_Sound(L, 1);
    PlaySound(arg1);
    return 0;
}

int lua_PauseSound(lua_State* L)
{
    Sound arg1 = LuaGetArgument_Sound(L, 1);
    PauseSound(arg1);
    return 0;
}

int lua_ResumeSound(lua_State* L)
{
    Sound arg1 = LuaGetArgument_Sound(L, 1);
    ResumeSound(arg1);
    return 0;
}

int lua_StopSound(lua_State* L)
{
    Sound arg1 = LuaGetArgument_Sound(L, 1);
    StopSound(arg1);
    return 0;
}

int lua_IsSoundPlaying(lua_State* L)
{
    Sound arg1 = LuaGetArgument_Sound(L, 1);
    bool result = IsSoundPlaying(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_SetSoundVolume(lua_State* L)
{
    Sound arg1 = LuaGetArgument_Sound(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    SetSoundVolume(arg1, arg2);
    return 0;
}

int lua_SetSoundPitch(lua_State* L)
{
    Sound arg1 = LuaGetArgument_Sound(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    SetSoundPitch(arg1, arg2);
    return 0;
}

int lua_WaveFormat(lua_State* L)
{
    Wave arg1 = LuaGetArgument_Wave(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    WaveFormat(&arg1, arg2, arg3, arg4);
    return 0;
}

int lua_WaveCopy(lua_State* L)
{
    Wave arg1 = LuaGetArgument_Wave(L, 1);
    Wave result = WaveCopy(arg1);
    LuaPush_Wave(L, result);
    return 1;
}

int lua_WaveCrop(lua_State* L)
{
    Wave arg1 = LuaGetArgument_Wave(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    WaveCrop(&arg1, arg2, arg3);
    return 0;
}

int lua_GetWaveData(lua_State* L)
{
    // TODO: float *GetWaveData(Wave wave);
    
    Wave arg1 = LuaGetArgument_Wave(L, 1);
    float * result = GetWaveData(arg1);
    //LuaPush_float(L, result);
    //lua_pushnumber(L, result);
    return 0;
}

int lua_LoadMusicStream(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    Music result = LoadMusicStream((char *)arg1);
    LuaPush_Music(L, result);
    return 1;
}

int lua_UnloadMusicStream(lua_State* L)
{
    Music arg1 = LuaGetArgument_Music(L, 1);
    UnloadMusicStream(arg1);
    return 0;
}

int lua_UpdateMusicStream(lua_State* L)
{
    Music arg1 = LuaGetArgument_Music(L, 1);
    UpdateMusicStream(arg1);
    return 0;
}

int lua_PlayMusicStream(lua_State* L)
{
    Music arg1 = LuaGetArgument_Music(L, 1);
    PlayMusicStream(arg1);
    return 0;
}

int lua_StopMusicStream(lua_State* L)
{
    Music arg1 = LuaGetArgument_Music(L, 1);
    StopMusicStream(arg1);
    return 0;
}

int lua_PauseMusicStream(lua_State* L)
{
    Music arg1 = LuaGetArgument_Music(L, 1);
    PauseMusicStream(arg1);
    return 0;
}

int lua_ResumeMusicStream(lua_State* L)
{
    Music arg1 = LuaGetArgument_Music(L, 1);
    ResumeMusicStream(arg1);
    return 0;
}

int lua_IsMusicPlaying(lua_State* L)
{
    Music arg1 = LuaGetArgument_Music(L, 1);
    bool result = IsMusicPlaying(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_SetMusicVolume(lua_State* L)
{
    Music arg1 = LuaGetArgument_Music(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    SetMusicVolume(arg1, arg2);
    return 0;
}

int lua_SetMusicPitch(lua_State* L)
{
    Music arg1 = LuaGetArgument_Music(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    SetMusicPitch(arg1, arg2);
    return 0;
}

int lua_GetMusicTimeLength(lua_State* L)
{
    Music arg1 = LuaGetArgument_Music(L, 1);
    float result = GetMusicTimeLength(arg1);
    lua_pushnumber(L, result);
    return 1;
}

int lua_GetMusicTimePlayed(lua_State* L)
{
    Music arg1 = LuaGetArgument_Music(L, 1);
    float result = GetMusicTimePlayed(arg1);
    lua_pushnumber(L, result);
    return 1;
}

int lua_InitAudioStream(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int arg2 = LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    AudioStream result = InitAudioStream(arg1, arg2, arg3);
    LuaPush_AudioStream(L, result);
    return 1;
}

int lua_CloseAudioStream(lua_State* L)
{
    AudioStream arg1 = LuaGetArgument_AudioStream(L, 1);
    CloseAudioStream(arg1);
    return 0;
}

int lua_UpdateAudioStream(lua_State* L)
{
    // TODO: void UpdateAudioStream(AudioStream stream, void *data, int numSamples);
    
    AudioStream arg1 = LuaGetArgument_AudioStream(L, 1);
    void * arg2 = (char *)LuaGetArgument_string(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    UpdateAudioStream(arg1, arg2, arg3);
    return 0;
}

int lua_IsAudioBufferProcessed(lua_State* L)
{
    AudioStream arg1 = LuaGetArgument_AudioStream(L, 1);
    bool result = IsAudioBufferProcessed(arg1);
    lua_pushboolean(L, result);
    return 1;
}

int lua_PlayAudioStream(lua_State* L)
{
    AudioStream arg1 = LuaGetArgument_AudioStream(L, 1);
    PlayAudioStream(arg1);
    return 0;
}


int lua_StopAudioStream(lua_State* L)
{
    AudioStream arg1 = LuaGetArgument_AudioStream(L, 1);
    StopAudioStream(arg1);
    return 0;
}

int lua_PauseAudioStream(lua_State* L)
{
    AudioStream arg1 = LuaGetArgument_AudioStream(L, 1);
    PauseAudioStream(arg1);
    return 0;
}

int lua_ResumeAudioStream(lua_State* L)
{
    AudioStream arg1 = LuaGetArgument_AudioStream(L, 1);
    ResumeAudioStream(arg1);
    return 0;
}

//----------------------------------------------------------------------------------
// raylib [utils] module functions
//----------------------------------------------------------------------------------
int lua_DecompressData(lua_State* L)
{
    unsigned char *arg1 = (unsigned char *)LuaGetArgument_string(L, 1);
    unsigned arg2 = (unsigned)LuaGetArgument_int(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    unsigned char *result = DecompressData(arg1, arg2, arg3);
    lua_pushlstring(L, (const char *)result, arg3);
    return 1;
}

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI)
int lua_WriteBitmap(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    unsigned char* arg2 = (unsigned char*)LuaGetArgument_string(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    WriteBitmap(arg1, arg2, arg3, arg4);
    return 0;
}

int lua_WritePNG(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    unsigned char* arg2 = (unsigned char*)LuaGetArgument_string(L, 2);
    int arg3 = LuaGetArgument_int(L, 3);
    int arg4 = LuaGetArgument_int(L, 4);
    int arg5 = LuaGetArgument_int(L, 5);
    WritePNG(arg1, arg2, arg3, arg4, arg5);
    return 0;
}
#endif

int lua_TraceLog(lua_State* L)
{
    int num_args = lua_gettop(L) - 1;
    int arg1 = LuaGetArgument_int(L, 1);

    /// type, fmt, args...

    lua_rotate(L, 1, -1); /// fmt, args..., type
    lua_pop(L, 1); /// fmt, args...

    lua_getglobal(L, "string"); /// fmt, args..., [string]
    lua_getfield(L, 1, "format"); /// fmt, args..., [string], format()
    lua_rotate(L, 1, 2); /// [string], format(), fmt, args...
    lua_call(L, num_args, 1); /// [string], formatted_string

    TraceLog(arg1, "%s", luaL_checkstring(L,-1));
    return 0;
}

int lua_GetExtension(lua_State* L)
{
    const char * arg1 = LuaGetArgument_string(L, 1);
    const char* result = GetExtension(arg1);
    lua_pushstring(L, result);
    return 1;
}

int lua_GetNextPOT(lua_State* L)
{
    int arg1 = LuaGetArgument_int(L, 1);
    int result = GetNextPOT(arg1);
    lua_pushinteger(L, result);
    return 1;
}

//----------------------------------------------------------------------------------
// raylib [raymath] module functions - Vector3 math
//----------------------------------------------------------------------------------
int lua_VectorAdd(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    Vector3 result = VectorAdd(arg1, arg2);
    LuaPush_Vector3(L, result);
    return 1;
}

int lua_VectorSubtract(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    Vector3 result = VectorSubtract(arg1, arg2);
    LuaPush_Vector3(L, result);
    return 1;
}

int lua_VectorCrossProduct(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    Vector3 result = VectorCrossProduct(arg1, arg2);
    LuaPush_Vector3(L, result);
    return 1;
}

int lua_VectorPerpendicular(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector3 result = VectorPerpendicular(arg1);
    LuaPush_Vector3(L, result);
    return 1;
}

int lua_VectorDotProduct(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    float result = VectorDotProduct(arg1, arg2);
    lua_pushnumber(L, result);
    return 1;
}

int lua_VectorLength(lua_State* L)
{
    const Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float result = VectorLength(arg1);
    lua_pushnumber(L, result);
    return 1;
}

int lua_VectorScale(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    VectorScale(&arg1, arg2);
    LuaPush_Vector3(L, arg1);
    return 1;
}

int lua_VectorNegate(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    VectorNegate(&arg1);
    LuaPush_Vector3(L, arg1);
    return 1;
}

int lua_VectorNormalize(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    VectorNormalize(&arg1);
    LuaPush_Vector3(L, arg1);
    return 1;
}

int lua_VectorDistance(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    float result = VectorDistance(arg1, arg2);
    lua_pushnumber(L, result);
    return 1;
}

int lua_VectorLerp(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    Vector3 result = VectorLerp(arg1, arg2, arg3);
    LuaPush_Vector3(L, result);
    return 1;
}

int lua_VectorReflect(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    Vector3 result = VectorReflect(arg1, arg2);
    LuaPush_Vector3(L, result);
    return 1;
}

int lua_VectorTransform(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Matrix arg2 = LuaGetArgument_Matrix(L, 2);
    VectorTransform(&arg1, arg2);
    LuaPush_Vector3(L, arg1);
    return 1;
}

int lua_VectorZero(lua_State* L)
{
    Vector3 result = VectorZero();
    LuaPush_Vector3(L, result);
    return 1;
}

//----------------------------------------------------------------------------------
// raylib [raymath] module functions - Matrix math
//----------------------------------------------------------------------------------
int lua_MatrixDeterminant(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    float result = MatrixDeterminant(arg1);
    lua_pushnumber(L, result);
    return 1;
}

int lua_MatrixTrace(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    float result = MatrixTrace(arg1);
    lua_pushnumber(L, result);
    return 1;
}

int lua_MatrixTranspose(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    MatrixTranspose(&arg1);
    LuaPush_Matrix(L, &arg1);
    return 1;
}

int lua_MatrixInvert(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    MatrixInvert(&arg1);
    LuaPush_Matrix(L, &arg1);
    return 1;
}

int lua_MatrixNormalize(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    MatrixNormalize(&arg1);
    LuaPush_Matrix(L, &arg1);
    return 1;
}

int lua_MatrixIdentity(lua_State* L)
{
    Matrix result = MatrixIdentity();
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixAdd(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    Matrix arg2 = LuaGetArgument_Matrix(L, 2);
    Matrix result = MatrixAdd(arg1, arg2);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixSubstract(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    Matrix arg2 = LuaGetArgument_Matrix(L, 2);
    Matrix result = MatrixSubstract(arg1, arg2);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixTranslate(lua_State* L)
{
    float arg1 = LuaGetArgument_float(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    Matrix result = MatrixTranslate(arg1, arg2, arg3);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixRotate(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    Matrix result = MatrixRotate(arg1, arg2);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixRotateX(lua_State* L)
{
    float arg1 = LuaGetArgument_float(L, 1);
    Matrix result = MatrixRotateX(arg1);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixRotateY(lua_State* L)
{
    float arg1 = LuaGetArgument_float(L, 1);
    Matrix result = MatrixRotateY(arg1);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixRotateZ(lua_State* L)
{
    float arg1 = LuaGetArgument_float(L, 1);
    Matrix result = MatrixRotateZ(arg1);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixScale(lua_State* L)
{
    float arg1 = LuaGetArgument_float(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    Matrix result = MatrixScale(arg1, arg2, arg3);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixMultiply(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    Matrix arg2 = LuaGetArgument_Matrix(L, 2);
    Matrix result = MatrixMultiply(arg1, arg2);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixFrustum(lua_State* L)
{
    double arg1 = LuaGetArgument_double(L, 1);
    double arg2 = LuaGetArgument_double(L, 2);
    double arg3 = LuaGetArgument_double(L, 3);
    double arg4 = LuaGetArgument_double(L, 4);
    double arg5 = LuaGetArgument_double(L, 5);
    double arg6 = LuaGetArgument_double(L, 6);
    Matrix result = MatrixFrustum(arg1, arg2, arg3, arg4, arg5, arg6);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixPerspective(lua_State* L)
{
    double arg1 = LuaGetArgument_double(L, 1);
    double arg2 = LuaGetArgument_double(L, 2);
    double arg3 = LuaGetArgument_double(L, 3);
    double arg4 = LuaGetArgument_double(L, 4);
    Matrix result = MatrixPerspective(arg1, arg2, arg3, arg4);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixOrtho(lua_State* L)
{
    double arg1 = LuaGetArgument_double(L, 1);
    double arg2 = LuaGetArgument_double(L, 2);
    double arg3 = LuaGetArgument_double(L, 3);
    double arg4 = LuaGetArgument_double(L, 4);
    double arg5 = LuaGetArgument_double(L, 5);
    double arg6 = LuaGetArgument_double(L, 6);
    Matrix result = MatrixOrtho(arg1, arg2, arg3, arg4, arg5, arg6);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_MatrixLookAt(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    Vector3 arg2 = LuaGetArgument_Vector3(L, 2);
    Vector3 arg3 = LuaGetArgument_Vector3(L, 3);
    Matrix result = MatrixLookAt(arg1, arg2, arg3);
    LuaPush_Matrix(L, &result);
    return 1;
}

//----------------------------------------------------------------------------------
// raylib [raymath] module functions - Quaternion math
//----------------------------------------------------------------------------------
int lua_QuaternionLength(lua_State* L)
{
    Quaternion arg1 = LuaGetArgument_Quaternion(L, 1);
    float result = QuaternionLength(arg1);
    lua_pushnumber(L, result);
    return 1;
}

int lua_QuaternionNormalize(lua_State* L)
{
    Quaternion arg1 = LuaGetArgument_Quaternion(L, 1);
    QuaternionNormalize(&arg1);
    LuaPush_Quaternion(L, arg1);
    return 1;
}

int lua_QuaternionMultiply(lua_State* L)
{
    Quaternion arg1 = LuaGetArgument_Quaternion(L, 1);
    Quaternion arg2 = LuaGetArgument_Quaternion(L, 2);
    Quaternion result = QuaternionMultiply(arg1, arg2);
    LuaPush_Quaternion(L, result);
    return 1;
}

int lua_QuaternionSlerp(lua_State* L)
{
    Quaternion arg1 = LuaGetArgument_Quaternion(L, 1);
    Quaternion arg2 = LuaGetArgument_Quaternion(L, 2);
    float arg3 = LuaGetArgument_float(L, 3);
    Quaternion result = QuaternionSlerp(arg1, arg2, arg3);
    LuaPush_Quaternion(L, result);
    return 1;
}

int lua_QuaternionFromMatrix(lua_State* L)
{
    Matrix arg1 = LuaGetArgument_Matrix(L, 1);
    Quaternion result = QuaternionFromMatrix(arg1);
    LuaPush_Quaternion(L, result);
    return 1;
}

int lua_QuaternionToMatrix(lua_State* L)
{
    Quaternion arg1 = LuaGetArgument_Quaternion(L, 1);
    Matrix result = QuaternionToMatrix(arg1);
    LuaPush_Matrix(L, &result);
    return 1;
}

int lua_QuaternionFromAxisAngle(lua_State* L)
{
    Vector3 arg1 = LuaGetArgument_Vector3(L, 1);
    float arg2 = LuaGetArgument_float(L, 2);
    Quaternion result = QuaternionFromAxisAngle(arg1, arg2);
    LuaPush_Quaternion(L, result);
    return 1;
}

int lua_QuaternionToAxisAngle(lua_State* L)
{
    Quaternion arg1 = LuaGetArgument_Quaternion(L, 1);
    Vector3 arg2;
    float arg3 = 0;
    QuaternionToAxisAngle(arg1, &arg2, &arg3);
    LuaPush_Vector3(L, arg2);
    lua_pushnumber(L, arg3);
    return 2;
}

int lua_QuaternionTransform(lua_State* L)
{
    Quaternion arg1 = LuaGetArgument_Quaternion(L, 1);
    Matrix arg2 = LuaGetArgument_Matrix(L, 2);
    QuaternionTransform(&arg1, arg2);
    LuaPush_Quaternion(L, arg1);
    return 1;
}


//----------------------------------------------------------------------------------
// Functions Registering
//----------------------------------------------------------------------------------
#define REG(name) { #name, lua_##name },

// raylib Functions (and data types) list
static luaL_Reg raylib_functions[] = {

    // Register non-opaque data types
    REG(Color)
    REG(Vector2)
    REG(Vector3)
    //REG(Matrix)
    REG(Quaternion)
    REG(Rectangle)
    REG(Ray)
    REG(Camera)
    REG(Camera2D)
    REG(BoundingBox)
    //REG(Material)

    // Register functions
    REG(InitWindow)
    REG(CloseWindow)
    REG(WindowShouldClose)
    REG(IsWindowMinimized)
    REG(ToggleFullscreen)
    REG(GetScreenWidth)
    REG(GetScreenHeight)

    REG(ShowCursor)
    REG(HideCursor)
    REG(IsCursorHidden)
    REG(EnableCursor)
    REG(DisableCursor)

    REG(ClearBackground)
    REG(BeginDrawing)
    REG(EndDrawing)
    REG(Begin2dMode)
    REG(End2dMode)
    REG(Begin3dMode)
    REG(End3dMode)
    REG(BeginTextureMode)
    REG(EndTextureMode)

    REG(GetMouseRay)
    REG(GetWorldToScreen)
    REG(GetCameraMatrix)

#if defined(PLATFORM_WEB)
    REG(SetDrawingLoop)
#else
    REG(SetTargetFPS)
#endif
    REG(GetFPS)
    REG(GetFrameTime)

    REG(GetColor)
    REG(GetHexValue)
    REG(ColorToFloat)
    REG(VectorToFloat)
    REG(MatrixToFloat)
    REG(GetRandomValue)
    REG(Fade)
    REG(SetConfigFlags)
    REG(ShowLogo)

    REG(IsFileDropped)
    REG(GetDroppedFiles)
    REG(ClearDroppedFiles)
    REG(StorageSaveValue)
    REG(StorageLoadValue)

    REG(IsKeyPressed)
    REG(IsKeyDown)
    REG(IsKeyReleased)
    REG(IsKeyUp)
    REG(GetKeyPressed)
    REG(SetExitKey)

    REG(IsGamepadAvailable)
    REG(IsGamepadName)
    REG(GetGamepadName)
    REG(IsGamepadButtonPressed)
    REG(IsGamepadButtonDown)
    REG(IsGamepadButtonReleased)
    REG(IsGamepadButtonUp)
    REG(GetGamepadButtonPressed)
    REG(GetGamepadAxisCount)
    REG(GetGamepadAxisMovement)

    REG(IsMouseButtonPressed)
    REG(IsMouseButtonDown)
    REG(IsMouseButtonReleased)
    REG(IsMouseButtonUp)
    REG(GetMouseX)
    REG(GetMouseY)
    REG(GetMousePosition)
    REG(SetMousePosition)
    REG(GetMouseWheelMove)
    REG(GetTouchX)
    REG(GetTouchY)
    REG(GetTouchPosition)

#if defined(PLATFORM_ANDROID)
    REG(IsButtonPressed)
    REG(IsButtonDown)
    REG(IsButtonReleased)
#endif

    REG(SetGesturesEnabled)
    REG(IsGestureDetected)
    REG(GetGestureDetected)
    REG(GetTouchPointsCount)
    REG(GetGestureHoldDuration)
    REG(GetGestureDragVector)
    REG(GetGestureDragAngle)
    REG(GetGesturePinchVector)
    REG(GetGesturePinchAngle)

    REG(SetCameraMode)
    REG(UpdateCamera)
    REG(SetCameraPanControl)
    REG(SetCameraAltControl)
    REG(SetCameraSmoothZoomControl)
    REG(SetCameraMoveControls)

    REG(DrawPixel)
    REG(DrawPixelV)
    REG(DrawLine)
    REG(DrawLineV)
    REG(DrawCircle)
    REG(DrawCircleGradient)
    REG(DrawCircleV)
    REG(DrawCircleLines)
    REG(DrawRectangle)
    REG(DrawRectangleRec)
    REG(DrawRectangleGradient)
    REG(DrawRectangleV)
    REG(DrawRectangleLines)
    REG(DrawTriangle)
    REG(DrawTriangleLines)
    REG(DrawPoly)
    REG(DrawPolyEx)
    REG(DrawPolyExLines)

    REG(CheckCollisionRecs)
    REG(CheckCollisionCircles)
    REG(CheckCollisionCircleRec)
    REG(GetCollisionRec)
    REG(CheckCollisionPointRec)
    REG(CheckCollisionPointCircle)
    REG(CheckCollisionPointTriangle)

    REG(LoadImage)
    REG(LoadImageEx)
    REG(LoadImageRaw)
    REG(LoadImageFromRES)
    REG(LoadTexture)
    REG(LoadTextureEx)
    REG(LoadTextureFromRES)
    REG(LoadTextureFromImage)
    REG(LoadRenderTexture)
    REG(UnloadImage)
    REG(UnloadTexture)
    REG(UnloadRenderTexture)
    REG(GetImageData)
    REG(GetTextureData)
    REG(UpdateTexture)
    REG(ImageToPOT)
    REG(ImageFormat)
    REG(ImageDither)
    REG(ImageCopy)
    REG(ImageCrop)
    REG(ImageResize)
    REG(ImageResizeNN)
    REG(ImageText)
    REG(ImageTextEx)
    REG(ImageDraw)
    REG(ImageDrawText)
    REG(ImageDrawTextEx)
    REG(ImageFlipVertical)
    REG(ImageFlipHorizontal)
    REG(ImageColorTint)
    REG(ImageColorInvert)
    REG(ImageColorGrayscale)
    REG(ImageColorContrast)
    REG(ImageColorBrightness)
    REG(GenTextureMipmaps)
    REG(SetTextureFilter)
    REG(SetTextureWrap)
    
    REG(DrawTexture)
    REG(DrawTextureV)
    REG(DrawTextureEx)
    REG(DrawTextureRec)
    REG(DrawTexturePro)

    REG(GetDefaultFont)
    REG(LoadSpriteFont)
    REG(LoadSpriteFontTTF)
    REG(UnloadSpriteFont)
    REG(DrawText)
    REG(DrawTextEx)
    REG(MeasureText)
    REG(MeasureTextEx)
    REG(DrawFPS)

    REG(DrawLine3D)
    REG(DrawCircle3D)
    REG(DrawCube)
    REG(DrawCubeV)
    REG(DrawCubeWires)
    REG(DrawCubeTexture)
    REG(DrawSphere)
    REG(DrawSphereEx)
    REG(DrawSphereWires)
    REG(DrawCylinder)
    REG(DrawCylinderWires)
    REG(DrawPlane)
    REG(DrawRay)
    REG(DrawGrid)
    REG(DrawGizmo)
    REG(DrawLight)

    REG(LoadModel)
    REG(LoadModelEx)
    REG(LoadModelFromRES)
    REG(LoadHeightmap)
    REG(LoadCubicmap)
    REG(UnloadModel)
    REG(LoadMaterial)
    REG(LoadDefaultMaterial)
    REG(LoadStandardMaterial)
    REG(UnloadMaterial)
    //REG(GenMesh*)     // Not ready yet...

    REG(DrawModel)
    REG(DrawModelEx)
    REG(DrawModelWires)
    REG(DrawModelWiresEx)
    REG(DrawBillboard)
    REG(DrawBillboardRec)
    REG(CalculateBoundingBox)
    REG(CheckCollisionSpheres)
    REG(CheckCollisionBoxes)
    REG(CheckCollisionBoxSphere)
    REG(CheckCollisionRaySphere)
    REG(CheckCollisionRaySphereEx)
    REG(CheckCollisionRayBox)

    REG(LoadShader)
    REG(UnloadShader)
    REG(GetDefaultShader)
    REG(GetStandardShader)
    REG(GetDefaultTexture)
    REG(GetShaderLocation)
    REG(SetShaderValue)
    REG(SetShaderValuei)
    REG(SetShaderValueMatrix)
    REG(SetMatrixProjection)
    REG(SetMatrixModelview)
    REG(BeginShaderMode)
    REG(EndShaderMode)
    REG(BeginBlendMode)
    REG(EndBlendMode)
    REG(CreateLight)
    REG(DestroyLight)

    REG(InitVrDevice)
    REG(CloseVrDevice)
    REG(IsVrDeviceReady)
    REG(IsVrSimulator)
    REG(UpdateVrTracking)
    REG(ToggleVrMode)

    REG(InitAudioDevice)
    REG(CloseAudioDevice)
    REG(IsAudioDeviceReady)
    REG(LoadWave)
    REG(LoadWaveEx)
    REG(LoadSound)
    REG(LoadSoundFromWave)
    REG(LoadSoundFromRES)
    REG(UpdateSound)
    REG(UnloadWave)
    REG(UnloadSound)
    REG(PlaySound)
    REG(PauseSound)
    REG(ResumeSound)
    REG(StopSound)
    REG(IsSoundPlaying)
    REG(SetSoundVolume)
    REG(SetSoundPitch)
    REG(WaveFormat)
    REG(WaveCopy)
    REG(WaveCrop)
    REG(GetWaveData)

    REG(LoadMusicStream)
    REG(UnloadMusicStream)
    REG(UpdateMusicStream)
    REG(PlayMusicStream)
    REG(StopMusicStream)
    REG(PauseMusicStream)
    REG(ResumeMusicStream)
    REG(IsMusicPlaying)
    REG(SetMusicVolume)
    REG(SetMusicPitch)
    REG(GetMusicTimeLength)
    REG(GetMusicTimePlayed)

    REG(InitAudioStream)
    REG(UpdateAudioStream)
    REG(CloseAudioStream)
    REG(IsAudioBufferProcessed)
    REG(PlayAudioStream)
    REG(PauseAudioStream)
    REG(ResumeAudioStream)
    REG(StopAudioStream)

    /// Math and util
    REG(DecompressData)
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI)
    REG(WriteBitmap)
    REG(WritePNG)
#endif
    REG(TraceLog)
    REG(GetExtension)
    REG(GetNextPOT)
    REG(VectorAdd)
    REG(VectorSubtract)
    REG(VectorCrossProduct)
    REG(VectorPerpendicular)
    REG(VectorDotProduct)
    REG(VectorLength)
    REG(VectorScale)
    REG(VectorNegate)
    REG(VectorNormalize)
    REG(VectorDistance)
    REG(VectorLerp)
    REG(VectorReflect)
    REG(VectorTransform)
    REG(VectorZero)
    REG(MatrixDeterminant)
    REG(MatrixTrace)
    REG(MatrixTranspose)
    REG(MatrixInvert)
    REG(MatrixNormalize)
    REG(MatrixIdentity)
    REG(MatrixAdd)
    REG(MatrixSubstract)
    REG(MatrixTranslate)
    REG(MatrixRotate)
    REG(MatrixRotateX)
    REG(MatrixRotateY)
    REG(MatrixRotateZ)
    REG(MatrixScale)
    REG(MatrixMultiply)
    REG(MatrixFrustum)
    REG(MatrixPerspective)
    REG(MatrixOrtho)
    REG(MatrixLookAt)
    REG(QuaternionLength)
    REG(QuaternionNormalize)
    REG(QuaternionMultiply)
    REG(QuaternionSlerp)
    REG(QuaternionFromMatrix)
    REG(QuaternionToMatrix)
    REG(QuaternionFromAxisAngle)
    REG(QuaternionToAxisAngle)
    REG(QuaternionTransform)

    {NULL, NULL}  // sentinel: end signal
};

// Register raylib functionality
static void LuaRegisterRayLib(const char *opt_table)
{
    if (opt_table) lua_createtable(L, 0, sizeof(raylib_functions)/sizeof(raylib_functions[0]));
    else lua_pushglobaltable(L);

    luaL_setfuncs(L, raylib_functions, 0);
}

//----------------------------------------------------------------------------------
// raylib Lua API
//----------------------------------------------------------------------------------

// Initialize Lua system
RLUADEF void InitLuaDevice(void)
{
    mainLuaState = luaL_newstate();
    L = mainLuaState;

    LuaStartEnum();
    LuaSetEnum("FULLSCREEN_MODE", 1);
    LuaSetEnum("SHOW_LOGO", 2);
    LuaSetEnum("SHOW_MOUSE_CURSOR", 4);
    LuaSetEnum("CENTERED_MODE", 8);
    LuaSetEnum("MSAA_4X_HINT", 16);
    LuaSetEnum("VSYNC_HINT", 32);
    LuaEndEnum("FLAG");

    LuaStartEnum();
    LuaSetEnum("SPACE", 32);
    LuaSetEnum("ESCAPE", 256);
    LuaSetEnum("ENTER", 257);
    LuaSetEnum("BACKSPACE", 259);
    LuaSetEnum("RIGHT", 262);
    LuaSetEnum("LEFT", 263);
    LuaSetEnum("DOWN", 264);
    LuaSetEnum("UP", 265);
    LuaSetEnum("F1", 290);
    LuaSetEnum("F2", 291);
    LuaSetEnum("F3", 292);
    LuaSetEnum("F4", 293);
    LuaSetEnum("F5", 294);
    LuaSetEnum("F6", 295);
    LuaSetEnum("F7", 296);
    LuaSetEnum("F8", 297);
    LuaSetEnum("F9", 298);
    LuaSetEnum("F10", 299);
    LuaSetEnum("LEFT_SHIFT", 340);
    LuaSetEnum("LEFT_CONTROL", 341);
    LuaSetEnum("LEFT_ALT", 342);
    LuaSetEnum("RIGHT_SHIFT", 344);
    LuaSetEnum("RIGHT_CONTROL", 345);
    LuaSetEnum("RIGHT_ALT", 346);
    LuaSetEnum("ZERO", 48);
    LuaSetEnum("ONE", 49);
    LuaSetEnum("TWO", 50);
    LuaSetEnum("THREE", 51);
    LuaSetEnum("FOUR", 52);
    LuaSetEnum("FIVE", 53);
    LuaSetEnum("SIX", 54);
    LuaSetEnum("SEVEN", 55);
    LuaSetEnum("EIGHT", 56);
    LuaSetEnum("NINE", 57);
    LuaSetEnum("A", 65);
    LuaSetEnum("B", 66);
    LuaSetEnum("C", 67);
    LuaSetEnum("D", 68);
    LuaSetEnum("E", 69);
    LuaSetEnum("F", 70);
    LuaSetEnum("G", 71);
    LuaSetEnum("H", 72);
    LuaSetEnum("I", 73);
    LuaSetEnum("J", 74);
    LuaSetEnum("K", 75);
    LuaSetEnum("L", 76);
    LuaSetEnum("M", 77);
    LuaSetEnum("N", 78);
    LuaSetEnum("O", 79);
    LuaSetEnum("P", 80);
    LuaSetEnum("Q", 81);
    LuaSetEnum("R", 82);
    LuaSetEnum("S", 83);
    LuaSetEnum("T", 84);
    LuaSetEnum("U", 85);
    LuaSetEnum("V", 86);
    LuaSetEnum("W", 87);
    LuaSetEnum("X", 88);
    LuaSetEnum("Y", 89);
    LuaSetEnum("Z", 90);
    LuaEndEnum("KEY");

    LuaStartEnum();
    LuaSetEnum("LEFT_BUTTON", 0);
    LuaSetEnum("RIGHT_BUTTON", 1);
    LuaSetEnum("MIDDLE_BUTTON", 2);
    LuaEndEnum("MOUSE");
    
    LuaStartEnum();
    LuaSetEnum("PLAYER1", 0);
    LuaSetEnum("PLAYER2", 1);
    LuaSetEnum("PLAYER3", 2);
    LuaSetEnum("PLAYER4", 3);

    LuaSetEnum("PS3_BUTTON_TRIANGLE", 0);
    LuaSetEnum("PS3_BUTTON_CIRCLE", 1);
    LuaSetEnum("PS3_BUTTON_CROSS", 2);
    LuaSetEnum("PS3_BUTTON_SQUARE", 3);
    LuaSetEnum("PS3_BUTTON_L1", 6);
    LuaSetEnum("PS3_BUTTON_R1", 7);
    LuaSetEnum("PS3_BUTTON_L2", 4);
    LuaSetEnum("PS3_BUTTON_R2",  5);
    LuaSetEnum("PS3_BUTTON_START", 8);
    LuaSetEnum("PS3_BUTTON_SELECT", 9);
    LuaSetEnum("PS3_BUTTON_UP", 24);
    LuaSetEnum("PS3_BUTTON_RIGHT", 25);
    LuaSetEnum("PS3_BUTTON_DOWN", 26);
    LuaSetEnum("PS3_BUTTON_LEFT", 27);
    LuaSetEnum("PS3_BUTTON_PS", 12);
    LuaSetEnum("PS3_AXIS_LEFT_X", 0);
    LuaSetEnum("PS3_AXIS_LEFT_Y", 1);
    LuaSetEnum("PS3_AXIS_RIGHT_X", 2);
    LuaSetEnum("PS3_AXIS_RIGHT_Y", 5);
    LuaSetEnum("PS3_AXIS_L2", 3);       // [1..-1] (pressure-level)
    LuaSetEnum("PS3_AXIS_R2", 4);       // [1..-1] (pressure-level)

// Xbox360 USB Controller Buttons
    LuaSetEnum("XBOX_BUTTON_A", 0);
    LuaSetEnum("XBOX_BUTTON_B", 1);
    LuaSetEnum("XBOX_BUTTON_X", 2);
    LuaSetEnum("XBOX_BUTTON_Y", 3);
    LuaSetEnum("XBOX_BUTTON_LB", 4);
    LuaSetEnum("XBOX_BUTTON_RB", 5);
    LuaSetEnum("XBOX_BUTTON_SELECT", 6);
    LuaSetEnum("XBOX_BUTTON_START", 7);
    LuaSetEnum("XBOX_BUTTON_UP", 10);
    LuaSetEnum("XBOX_BUTTON_RIGHT", 11);
    LuaSetEnum("XBOX_BUTTON_DOWN", 12);
    LuaSetEnum("XBOX_BUTTON_LEFT", 13);
    LuaSetEnum("XBOX_BUTTON_HOME", 8);
#if defined(PLATFORM_RPI)
    LuaSetEnum("XBOX_AXIS_LEFT_X", 0);      // [-1..1] (left->right)
    LuaSetEnum("XBOX_AXIS_LEFT_Y", 1);      // [-1..1] (up->down)
    LuaSetEnum("XBOX_AXIS_RIGHT_X", 3);     // [-1..1] (left->right)
    LuaSetEnum("XBOX_AXIS_RIGHT_Y", 4);     // [-1..1] (up->down)
    LuaSetEnum("XBOX_AXIS_LT", 2);          // [-1..1] (pressure-level)
    LuaSetEnum("XBOX_AXIS_RT", 5);          // [-1..1] (pressure-level)
#else
    LuaSetEnum("XBOX_AXIS_LEFT_X", 0);      // [-1..1] (left->right)
    LuaSetEnum("XBOX_AXIS_LEFT_Y", 1);      // [1..-1] (up->down)
    LuaSetEnum("XBOX_AXIS_RIGHT_X", 2);     // [-1..1] (left->right)
    LuaSetEnum("XBOX_AXIS_RIGHT_Y", 3);     // [1..-1] (up->down)
    LuaSetEnum("XBOX_AXIS_LT", 4);          // [-1..1] (pressure-level)
    LuaSetEnum("XBOX_AXIS_RT", 5);          // [-1..1] (pressure-level)
#endif
    LuaEndEnum("GAMEPAD");

    lua_pushglobaltable(L);
    LuaSetEnumColor("LIGHTGRAY", LIGHTGRAY);
    LuaSetEnumColor("GRAY", GRAY);
    LuaSetEnumColor("DARKGRAY", DARKGRAY);
    LuaSetEnumColor("YELLOW", YELLOW);
    LuaSetEnumColor("GOLD", GOLD);
    LuaSetEnumColor("ORANGE", ORANGE);
    LuaSetEnumColor("PINK", PINK);
    LuaSetEnumColor("RED", RED);
    LuaSetEnumColor("MAROON", MAROON);
    LuaSetEnumColor("GREEN", GREEN);
    LuaSetEnumColor("LIME", LIME);
    LuaSetEnumColor("DARKGREEN", DARKGREEN);
    LuaSetEnumColor("SKYBLUE", SKYBLUE);
    LuaSetEnumColor("BLUE", BLUE);
    LuaSetEnumColor("DARKBLUE", DARKBLUE);
    LuaSetEnumColor("PURPLE", PURPLE);
    LuaSetEnumColor("VIOLET", VIOLET);
    LuaSetEnumColor("DARKPURPLE", DARKPURPLE);
    LuaSetEnumColor("BEIGE", BEIGE);
    LuaSetEnumColor("BROWN", BROWN);
    LuaSetEnumColor("DARKBROWN", DARKBROWN);
    LuaSetEnumColor("WHITE", WHITE);
    LuaSetEnumColor("BLACK", BLACK);
    LuaSetEnumColor("BLANK", BLANK);
    LuaSetEnumColor("MAGENTA", MAGENTA);
    LuaSetEnumColor("RAYWHITE", RAYWHITE);
    lua_pop(L, 1);

    LuaStartEnum();
    LuaSetEnum("UNCOMPRESSED_GRAYSCALE", UNCOMPRESSED_GRAYSCALE);
    LuaSetEnum("UNCOMPRESSED_GRAY_ALPHA", UNCOMPRESSED_GRAY_ALPHA);
    LuaSetEnum("UNCOMPRESSED_R5G6B5", UNCOMPRESSED_R5G6B5);
    LuaSetEnum("UNCOMPRESSED_R8G8B8", UNCOMPRESSED_R8G8B8);
    LuaSetEnum("UNCOMPRESSED_R5G5B5A1", UNCOMPRESSED_R5G5B5A1);
    LuaSetEnum("UNCOMPRESSED_R4G4B4A4", UNCOMPRESSED_R4G4B4A4);
    LuaSetEnum("UNCOMPRESSED_R8G8B8A8", UNCOMPRESSED_R8G8B8A8);
    LuaSetEnum("COMPRESSED_DXT1_RGB", COMPRESSED_DXT1_RGB);
    LuaSetEnum("COMPRESSED_DXT1_RGBA", COMPRESSED_DXT1_RGBA);
    LuaSetEnum("COMPRESSED_DXT3_RGBA", COMPRESSED_DXT3_RGBA);
    LuaSetEnum("COMPRESSED_DXT5_RGBA", COMPRESSED_DXT5_RGBA);
    LuaSetEnum("COMPRESSED_ETC1_RGB", COMPRESSED_ETC1_RGB);
    LuaSetEnum("COMPRESSED_ETC2_RGB", COMPRESSED_ETC2_RGB);
    LuaSetEnum("COMPRESSED_ETC2_EAC_RGBA", COMPRESSED_ETC2_EAC_RGBA);
    LuaSetEnum("COMPRESSED_PVRT_RGB", COMPRESSED_PVRT_RGB);
    LuaSetEnum("COMPRESSED_PVRT_RGBA", COMPRESSED_PVRT_RGBA);
    LuaSetEnum("COMPRESSED_ASTC_4x4_RGBA", COMPRESSED_ASTC_4x4_RGBA);
    LuaSetEnum("COMPRESSED_ASTC_8x8_RGBA", COMPRESSED_ASTC_8x8_RGBA);
    LuaEndEnum("TextureFormat");

    LuaStartEnum();
    LuaSetEnum("ALPHA", BLEND_ALPHA);
    LuaSetEnum("ADDITIVE", BLEND_ADDITIVE);
    LuaSetEnum("MULTIPLIED", BLEND_MULTIPLIED);
    LuaEndEnum("BlendMode");

    LuaStartEnum();
    LuaSetEnum("POINT", LIGHT_POINT);
    LuaSetEnum("DIRECTIONAL", LIGHT_DIRECTIONAL);
    LuaSetEnum("SPOT", LIGHT_SPOT);
    LuaEndEnum("LightType");
    
    LuaStartEnum();
    LuaSetEnum("POINT", FILTER_POINT);
    LuaSetEnum("BILINEAR", FILTER_BILINEAR);
    LuaSetEnum("TRILINEAR", FILTER_TRILINEAR);
    LuaSetEnum("ANISOTROPIC_4X", FILTER_ANISOTROPIC_4X);
    LuaSetEnum("ANISOTROPIC_8X", FILTER_ANISOTROPIC_8X);
    LuaSetEnum("ANISOTROPIC_16X", FILTER_ANISOTROPIC_16X);
    LuaEndEnum("TextureFilter");

    LuaStartEnum();
    LuaSetEnum("NONE", GESTURE_NONE);
    LuaSetEnum("TAP", GESTURE_TAP);
    LuaSetEnum("DOUBLETAP", GESTURE_DOUBLETAP);
    LuaSetEnum("HOLD", GESTURE_HOLD);
    LuaSetEnum("DRAG", GESTURE_DRAG);
    LuaSetEnum("SWIPE_RIGHT", GESTURE_SWIPE_RIGHT);
    LuaSetEnum("SWIPE_LEFT", GESTURE_SWIPE_LEFT);
    LuaSetEnum("SWIPE_UP", GESTURE_SWIPE_UP);
    LuaSetEnum("SWIPE_DOWN", GESTURE_SWIPE_DOWN);
    LuaSetEnum("PINCH_IN", GESTURE_PINCH_IN);
    LuaSetEnum("PINCH_OUT", GESTURE_PINCH_OUT);
    LuaEndEnum("Gestures");

    LuaStartEnum();
    LuaSetEnum("CUSTOM", CAMERA_CUSTOM);
    LuaSetEnum("FREE", CAMERA_FREE);
    LuaSetEnum("ORBITAL", CAMERA_ORBITAL);
    LuaSetEnum("FIRST_PERSON", CAMERA_FIRST_PERSON);
    LuaSetEnum("THIRD_PERSON", CAMERA_THIRD_PERSON);
    LuaEndEnum("CameraMode");

    LuaStartEnum();
    LuaSetEnum("DEFAULT_DEVICE", HMD_DEFAULT_DEVICE);
    LuaSetEnum("OCULUS_RIFT_DK2", HMD_OCULUS_RIFT_DK2);
    LuaSetEnum("OCULUS_RIFT_CV1", HMD_OCULUS_RIFT_CV1);
    LuaSetEnum("VALVE_HTC_VIVE", HMD_VALVE_HTC_VIVE);
    LuaSetEnum("SAMSUNG_GEAR_VR", HMD_SAMSUNG_GEAR_VR);
    LuaSetEnum("GOOGLE_CARDBOARD", HMD_GOOGLE_CARDBOARD);
    LuaSetEnum("SONY_PLAYSTATION_VR", HMD_SONY_PLAYSTATION_VR);
    LuaSetEnum("RAZER_OSVR", HMD_RAZER_OSVR);
    LuaSetEnum("FOVE_VR", HMD_FOVE_VR);
    LuaEndEnum("VrDevice");

    lua_pushglobaltable(L);
    LuaSetEnum("INFO", INFO);
    LuaSetEnum("ERROR", ERROR);
    LuaSetEnum("WARNING", WARNING);
    LuaSetEnum("DEBUG", DEBUG);
    LuaSetEnum("OTHER", OTHER);
    lua_pop(L, 1);

    lua_pushboolean(L, true);
#if defined(PLATFORM_DESKTOP)
    lua_setglobal(L, "PLATFORM_DESKTOP");
#elif defined(PLATFORM_ANDROID)
    lua_setglobal(L, "PLATFORM_ANDROID");
#elif defined(PLATFORM_RPI)
    lua_setglobal(L, "PLATFORM_RPI");
#elif defined(PLATFORM_WEB)
    lua_setglobal(L, "PLATFORM_WEB");
#endif

    luaL_openlibs(L);
    LuaBuildOpaqueMetatables();

    LuaRegisterRayLib(0);
}

// De-initialize Lua system
RLUADEF void CloseLuaDevice(void)
{
    if (mainLuaState)
    {
        lua_close(mainLuaState);
        mainLuaState = 0;
        L = 0;
    }
}

// Execute raylib Lua code
RLUADEF void ExecuteLuaCode(const char *code)
{
    if (!mainLuaState)
    {
        TraceLog(WARNING, "Lua device not initialized");
        return;
    }

    int result = luaL_dostring(L, code);

    switch (result)
    {
        case LUA_OK: break;
        case LUA_ERRRUN: TraceLog(ERROR, "Lua Runtime Error: %s", lua_tostring(L, -1)); break;
        case LUA_ERRMEM: TraceLog(ERROR, "Lua Memory Error: %s", lua_tostring(L, -1)); break;
        default: TraceLog(ERROR, "Lua Error: %s", lua_tostring(L, -1)); break;
    }
}

// Execute raylib Lua script
RLUADEF void ExecuteLuaFile(const char *filename)
{
    if (!mainLuaState)
    {
        TraceLog(WARNING, "Lua device not initialized");
        return;
    }

    int result = luaL_dofile(L, filename);

    switch (result)
    {
        case LUA_OK: break;
        case LUA_ERRRUN: TraceLog(ERROR, "Lua Runtime Error: %s", lua_tostring(L, -1));
        case LUA_ERRMEM: TraceLog(ERROR, "Lua Memory Error: %s", lua_tostring(L, -1));
        default: TraceLog(ERROR, "Lua Error: %s", lua_tostring(L, -1));
    }
}

#endif // RLUA_IMPLEMENTATION
