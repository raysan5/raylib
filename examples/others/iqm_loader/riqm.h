/**********************************************************************************************
*
*   riqm - InterQuake Model format (IQM) loader for animated meshes
*
*   CONFIGURATION:
*
*   #define RIQM_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018 Jonas Daeyaert (@culacant) and Ramon Santamaria (@raysan5)
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

#ifndef RIQM_H
#define RIQM_H

//#define RIQM_STATIC
#ifdef RIQM_STATIC
    #define RIQMDEF static              // Functions just visible to module including this file
#else
    #ifdef __cplusplus
        #define RIQMDEF extern "C"      // Functions visible from other files (no name mangling of functions in C++)
    #else
        #define RIQMDEF extern          // Functions visible from other files
    #endif
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

#define BONE_NAME_LENGTH    32          // BoneInfo name string length
#define MESH_NAME_LENGTH    32          // Mesh name string length

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------


#endif // RIQM_H


/***********************************************************************************
*
*  RIQM IMPLEMENTATION
*
************************************************************************************/

#if defined(RIQM_IMPLEMENTATION)

//#include "utils.h"          // Required for: fopen() Android mapping

#include <stdio.h>          // Required for: FILE, fopen(), fclose(), feof(), fseek(), fread()
#include <stdlib.h>         // Required for: malloc(), free()
#include <string.h>         // Required for: strncmp(),strcpy()

#include "raymath.h"        // Required for: Vector3, Quaternion functions

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif




#endif
