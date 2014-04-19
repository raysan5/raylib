/*********************************************************************************************
*
*   raylib.utils
*
*   Some utility functions: rRES files data decompression
*       
*   Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
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

#ifndef UTILS_H
#define UTILS_H

//----------------------------------------------------------------------------------
// Some basic Defines
//----------------------------------------------------------------------------------
#define DO_NOT_TRACE_DEBUG_MSGS   // Use this define to avoid DEBUG tracing

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { IMAGE = 0, SOUND, MODEL, TEXT, RAW } DataType;

typedef enum { INFO = 0, ERROR, WARNING, DEBUG, OTHER } TraceLogType;

// One resource info header, every resource includes this header (8 byte)
typedef struct {
    unsigned short id;      // Resource unique identifier (2 byte)
    unsigned char type;     // Resource type (1 byte)
    unsigned char comp;     // Data Compression and Coding (1 byte)
    unsigned int size;      // Data size in .rres file (compressed or not, only DATA) (4 byte)
    unsigned int srcSize;   // Source data size (uncompressed, only DATA)
} ResInfoHeader;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
unsigned char *DecompressData(const unsigned char *data, unsigned long compSize, int uncompSize);

void WriteBitmap(const char *fileName, unsigned char *imgData, int width, int height);
void WritePNG(const char *fileName, unsigned char *imgData, int width, int height);

void TraceLog(int msgType, const char *text, ...);  // Outputs a trace log message
void TraceLogOpen(const char *logFileName);         // Open a trace log file (if desired)
void TraceLogClose();                               // Close the trace log file

const char *GetExtension(const char *fileName);

#ifdef __cplusplus
}
#endif

#endif // UTILS_H