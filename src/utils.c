/**********************************************************************************************
*
*   raylib.utils - Some common utility functions
*
*   CONFIGURATION:
*       #define SUPPORT_TRACELOG
*           Show TraceLog() output messages
*           NOTE: By default LOG_DEBUG traces not shown
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2025 Ramon Santamaria (@raysan5)
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

#include "raylib.h"                     // WARNING: Required for: LogType enum

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
    #include "config.h"                 // Defines module configuration flags
#endif

#include "utils.h"

#if defined(PLATFORM_ANDROID)
    #include <errno.h>                  // Required for: Android error types
    #include <android/log.h>            // Required for: Android log system: __android_log_vprint()
    #include <android/asset_manager.h>  // Required for: Android assets manager: AAsset, AAssetManager_open()...
#endif

#include <stdlib.h>                     // Required for: exit()
#include <stdio.h>                      // Required for: FILE, fopen(), fseek(), ftell(), fread(), fwrite(), fprintf(), vprintf(), fclose()
#include <stdarg.h>                     // Required for: va_list, va_start(), va_end()
#include <string.h>                     // Required for: strcpy(), strcat()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef MAX_TRACELOG_MSG_LENGTH
    #define MAX_TRACELOG_MSG_LENGTH     256         // Max length of one trace-log message
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static int logTypeLevel = LOG_INFO;                 // Minimum log type level

static TraceLogCallback traceLog = NULL;            // TraceLog callback function pointer
static LoadFileDataCallback loadFileData = NULL;    // LoadFileData callback function pointer
static SaveFileDataCallback saveFileData = NULL;    // SaveFileText callback function pointer
static LoadFileTextCallback loadFileText = NULL;    // LoadFileText callback function pointer
static SaveFileTextCallback saveFileText = NULL;    // SaveFileText callback function pointer

//----------------------------------------------------------------------------------
// Functions to set internal callbacks
//----------------------------------------------------------------------------------
void SetTraceLogCallback(TraceLogCallback callback) { traceLog = callback; }              // Set custom trace log
void SetLoadFileDataCallback(LoadFileDataCallback callback) { loadFileData = callback; }  // Set custom file data loader
void SetSaveFileDataCallback(SaveFileDataCallback callback) { saveFileData = callback; }  // Set custom file data saver
void SetLoadFileTextCallback(LoadFileTextCallback callback) { loadFileText = callback; }  // Set custom file text loader
void SetSaveFileTextCallback(SaveFileTextCallback callback) { saveFileText = callback; }  // Set custom file text saver

#if defined(PLATFORM_ANDROID)
static AAssetManager *assetManager = NULL;          // Android assets manager pointer
static const char *internalDataPath = NULL;         // Android internal data path
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
FILE *funopen(const void *cookie, int (*readfn)(void *, char *, int), int (*writefn)(void *, const char *, int),
              fpos_t (*seekfn)(void *, fpos_t, int), int (*closefn)(void *));

static int android_read(void *cookie, char *buf, int size);
static int android_write(void *cookie, const char *buf, int size);
static fpos_t android_seek(void *cookie, fpos_t offset, int whence);
static int android_close(void *cookie);
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Utilities
//----------------------------------------------------------------------------------

// Set the current threshold (minimum) log level
void SetTraceLogLevel(int logType) { logTypeLevel = logType; }

// Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
void TraceLog(int logType, const char *text, ...)
{
#if defined(SUPPORT_TRACELOG)
    // Message has level below current threshold, don't emit
    if (logType < logTypeLevel) return;

    va_list args;
    va_start(args, text);

    if (traceLog)
    {
        traceLog(logType, text, args);
        va_end(args);
        return;
    }

#if defined(PLATFORM_ANDROID)
    switch (logType)
    {
        case LOG_TRACE: __android_log_vprint(ANDROID_LOG_VERBOSE, "raylib", text, args); break;
        case LOG_DEBUG: __android_log_vprint(ANDROID_LOG_DEBUG, "raylib", text, args); break;
        case LOG_INFO: __android_log_vprint(ANDROID_LOG_INFO, "raylib", text, args); break;
        case LOG_WARNING: __android_log_vprint(ANDROID_LOG_WARN, "raylib", text, args); break;
        case LOG_ERROR: __android_log_vprint(ANDROID_LOG_ERROR, "raylib", text, args); break;
        case LOG_FATAL: __android_log_vprint(ANDROID_LOG_FATAL, "raylib", text, args); break;
        default: break;
    }
#else
    char buffer[MAX_TRACELOG_MSG_LENGTH] = { 0 };

    switch (logType)
    {
        case LOG_TRACE: strcpy(buffer, "TRACE: "); break;
        case LOG_DEBUG: strcpy(buffer, "DEBUG: "); break;
        case LOG_INFO: strcpy(buffer, "INFO: "); break;
        case LOG_WARNING: strcpy(buffer, "WARNING: "); break;
        case LOG_ERROR: strcpy(buffer, "ERROR: "); break;
        case LOG_FATAL: strcpy(buffer, "FATAL: "); break;
        default: break;
    }

    unsigned int textSize = (unsigned int)strlen(text);
    memcpy(buffer + strlen(buffer), text, (textSize < (MAX_TRACELOG_MSG_LENGTH - 12))? textSize : (MAX_TRACELOG_MSG_LENGTH - 12));
    strcat(buffer, "\n");
    vprintf(buffer, args);
    fflush(stdout);
#endif

    va_end(args);

    if (logType == LOG_FATAL) exit(EXIT_FAILURE);  // If fatal logging, exit program

#endif  // SUPPORT_TRACELOG
}

// Internal memory allocator
// NOTE: Initializes to zero by default
void *MemAlloc(unsigned int size)
{
    void *ptr = RL_CALLOC(size, 1);
    return ptr;
}

// Internal memory reallocator
void *MemRealloc(void *ptr, unsigned int size)
{
    void *ret = RL_REALLOC(ptr, size);
    return ret;
}

// Internal memory free
void MemFree(void *ptr)
{
    RL_FREE(ptr);
}

// Load data from file into a buffer
unsigned char *LoadFileData(const char *fileName, int *dataSize)
{
    unsigned char *data = NULL;
    *dataSize = 0;

    if (fileName != NULL)
    {
        if (loadFileData)
        {
            data = loadFileData(fileName, dataSize);
            return data;
        }
#if defined(SUPPORT_STANDARD_FILEIO)
        FILE *file = fopen(fileName, "rb");

        if (file != NULL)
        {
            // WARNING: On binary streams SEEK_END could not be found,
            // using fseek() and ftell() could not work in some (rare) cases
            fseek(file, 0, SEEK_END);
            int size = ftell(file);     // WARNING: ftell() returns 'long int', maximum size returned is INT_MAX (2147483647 bytes)
            fseek(file, 0, SEEK_SET);

            if (size > 0)
            {
                data = (unsigned char *)RL_MALLOC(size*sizeof(unsigned char));

                if (data != NULL)
                {
                    // NOTE: fread() returns number of read elements instead of bytes, so we read [1 byte, size elements]
                    size_t count = fread(data, sizeof(unsigned char), size, file);

                    // WARNING: fread() returns a size_t value, usually 'unsigned int' (32bit compilation) and 'unsigned long long' (64bit compilation)
                    // dataSize is unified along raylib as a 'int' type, so, for file-sizes > INT_MAX (2147483647 bytes) we have a limitation
                    if (count > 2147483647)
                    {
                        TRACELOG(LOG_WARNING, "FILEIO: [%s] File is bigger than 2147483647 bytes, avoid using LoadFileData()", fileName);

                        RL_FREE(data);
                        data = NULL;
                    }
                    else
                    {
                        *dataSize = (int)count;

                        if ((*dataSize) != size) TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially loaded (%i bytes out of %i)", fileName, dataSize, count);
                        else TRACELOG(LOG_INFO, "FILEIO: [%s] File loaded successfully", fileName);
                    }
                }
                else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to allocated memory for file reading", fileName);
            }
            else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to read file", fileName);

            fclose(file);
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
#else
    TRACELOG(LOG_WARNING, "FILEIO: Standard file io not supported, use custom file callback");
#endif
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return data;
}

// Unload file data allocated by LoadFileData()
void UnloadFileData(unsigned char *data)
{
    RL_FREE(data);
}

// Save data to file from buffer
bool SaveFileData(const char *fileName, void *data, int dataSize)
{
    bool success = false;

    if (fileName != NULL)
    {
        if (saveFileData)
        {
            return saveFileData(fileName, data, dataSize);
        }
#if defined(SUPPORT_STANDARD_FILEIO)
        FILE *file = fopen(fileName, "wb");

        if (file != NULL)
        {
            // WARNING: fwrite() returns a size_t value, usually 'unsigned int' (32bit compilation) and 'unsigned long long' (64bit compilation)
            // and expects a size_t input value but as dataSize is limited to INT_MAX (2147483647 bytes), there shouldn't be a problem
            int count = (int)fwrite(data, sizeof(unsigned char), dataSize, file);

            if (count == 0) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to write file", fileName);
            else if (count != dataSize) TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially written", fileName);
            else TRACELOG(LOG_INFO, "FILEIO: [%s] File saved successfully", fileName);

            int result = fclose(file);
            if (result == 0) success = true;
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
#else
    TRACELOG(LOG_WARNING, "FILEIO: Standard file io not supported, use custom file callback");
#endif
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return success;
}

// Export data to code (.h), returns true on success
bool ExportDataAsCode(const unsigned char *data, int dataSize, const char *fileName)
{
    bool success = false;

#ifndef TEXT_BYTES_PER_LINE
    #define TEXT_BYTES_PER_LINE     20
#endif

    // NOTE: Text data buffer size is estimated considering raw data size in bytes
    // and requiring 6 char bytes for every byte: "0x00, "
    char *txtData = (char *)RL_CALLOC(dataSize*6 + 2000, sizeof(char));

    int byteCount = 0;
    byteCount += sprintf(txtData + byteCount, "////////////////////////////////////////////////////////////////////////////////////////\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// DataAsCode exporter v1.0 - Raw data exported as an array of bytes                  //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// more info and bugs-report:  github.com/raysan5/raylib                              //\n");
    byteCount += sprintf(txtData + byteCount, "// feedback and support:       ray[at]raylib.com                                      //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// Copyright (c) 2022-2025 Ramon Santamaria (@raysan5)                                //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "////////////////////////////////////////////////////////////////////////////////////////\n\n");

    // Get file name from path
    char varFileName[256] = { 0 };
    strcpy(varFileName, GetFileNameWithoutExt(fileName));
    for (int i = 0; varFileName[i] != '\0'; i++)
    {
        // Convert variable name to uppercase
        if ((varFileName[i] >= 'a') && (varFileName[i] <= 'z')) { varFileName[i] = varFileName[i] - 32; }
        // Replace non valid character for C identifier with '_'
        else if (varFileName[i] == '.' || varFileName[i] == '-' || varFileName[i] == '?' || varFileName[i] == '!' || varFileName[i] == '+') { varFileName[i] = '_'; }
    }

    byteCount += sprintf(txtData + byteCount, "#define %s_DATA_SIZE     %i\n\n", varFileName, dataSize);

    byteCount += sprintf(txtData + byteCount, "static unsigned char %s_DATA[%s_DATA_SIZE] = { ", varFileName, varFileName);
    for (int i = 0; i < (dataSize - 1); i++) byteCount += sprintf(txtData + byteCount, ((i%TEXT_BYTES_PER_LINE == 0)? "0x%x,\n" : "0x%x, "), data[i]);
    byteCount += sprintf(txtData + byteCount, "0x%x };\n", data[dataSize - 1]);

    // NOTE: Text data size exported is determined by '\0' (NULL) character
    success = SaveFileText(fileName, txtData);

    RL_FREE(txtData);

    if (success != 0) TRACELOG(LOG_INFO, "FILEIO: [%s] Data as code exported successfully", fileName);
    else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to export data as code", fileName);

    return success;
}

// Load text data from file, returns a '\0' terminated string
// NOTE: text chars array should be freed manually
char *LoadFileText(const char *fileName)
{
    char *text = NULL;

    if (fileName != NULL)
    {
        if (loadFileText)
        {
            text = loadFileText(fileName);
            return text;
        }
#if defined(SUPPORT_STANDARD_FILEIO)
        FILE *file = fopen(fileName, "rt");

        if (file != NULL)
        {
            // WARNING: When reading a file as 'text' file,
            // text mode causes carriage return-linefeed translation...
            // ...but using fseek() should return correct byte-offset
            fseek(file, 0, SEEK_END);
            unsigned int size = (unsigned int)ftell(file);
            fseek(file, 0, SEEK_SET);

            if (size > 0)
            {
                text = (char *)RL_MALLOC((size + 1)*sizeof(char));

                if (text != NULL)
                {
                    unsigned int count = (unsigned int)fread(text, sizeof(char), size, file);

                    // WARNING: \r\n is converted to \n on reading, so,
                    // read bytes count gets reduced by the number of lines
                    if (count < size) text = RL_REALLOC(text, count + 1);

                    // Zero-terminate the string
                    text[count] = '\0';

                    TRACELOG(LOG_INFO, "FILEIO: [%s] Text file loaded successfully", fileName);
                }
                else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to allocated memory for file reading", fileName);
            }
            else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to read text file", fileName);

            fclose(file);
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open text file", fileName);
#else
    TRACELOG(LOG_WARNING, "FILEIO: Standard file io not supported, use custom file callback");
#endif
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return text;
}

// Unload file text data allocated by LoadFileText()
void UnloadFileText(char *text)
{
    RL_FREE(text);
}

// Save text data to file (write), string must be '\0' terminated
bool SaveFileText(const char *fileName, char *text)
{
    bool success = false;

    if (fileName != NULL)
    {
        if (saveFileText)
        {
            return saveFileText(fileName, text);
        }
#if defined(SUPPORT_STANDARD_FILEIO)
        FILE *file = fopen(fileName, "wt");

        if (file != NULL)
        {
            int count = fprintf(file, "%s", text);

            if (count < 0) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to write text file", fileName);
            else TRACELOG(LOG_INFO, "FILEIO: [%s] Text file saved successfully", fileName);

            int result = fclose(file);
            if (result == 0) success = true;
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open text file", fileName);
#else
    TRACELOG(LOG_WARNING, "FILEIO: Standard file io not supported, use custom file callback");
#endif
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return success;
}

#if defined(PLATFORM_ANDROID)
// Initialize asset manager from android app
void InitAssetManager(AAssetManager *manager, const char *dataPath)
{
    assetManager = manager;
    internalDataPath = dataPath;
}

// Replacement for fopen()
// Ref: https://developer.android.com/ndk/reference/group/asset
FILE *android_fopen(const char *fileName, const char *mode)
{
    if (mode[0] == 'w')
    {
        // fopen() is mapped to android_fopen() that only grants read access to
        // assets directory through AAssetManager but we want to also be able to
        // write data when required using the standard stdio FILE access functions
        // Ref: https://stackoverflow.com/questions/11294487/android-writing-saving-files-from-native-code-only
        #undef fopen
        return fopen(TextFormat("%s/%s", internalDataPath, fileName), mode);
        #define fopen(name, mode) android_fopen(name, mode)
    }
    else
    {
        // NOTE: AAsset provides access to read-only asset
        AAsset *asset = AAssetManager_open(assetManager, fileName, AASSET_MODE_UNKNOWN);

        if (asset != NULL)
        {
            // Get pointer to file in the assets
            return funopen(asset, android_read, android_write, android_seek, android_close);
        }
        else
        {
            #undef fopen
            // Just do a regular open if file is not found in the assets
            return fopen(TextFormat("%s/%s", internalDataPath, fileName), mode);
            #define fopen(name, mode) android_fopen(name, mode)
        }
    }
}
#endif  // PLATFORM_ANDROID

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
static int android_read(void *cookie, char *data, int dataSize)
{
    return AAsset_read((AAsset *)cookie, data, dataSize);
}

static int android_write(void *cookie, const char *data, int dataSize)
{
    TRACELOG(LOG_WARNING, "ANDROID: Failed to provide write access to APK");

    return EACCES;
}

static fpos_t android_seek(void *cookie, fpos_t offset, int whence)
{
    return AAsset_seek((AAsset *)cookie, offset, whence);
}

static int android_close(void *cookie)
{
    AAsset_close((AAsset *)cookie);
    return 0;
}
#endif  // PLATFORM_ANDROID
