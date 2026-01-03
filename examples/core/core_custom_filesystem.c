/*******************************************************************************************
*
*   raylib [core] example - Custom file system with RL_FS_* macros
*
*   This example demonstrates how to override raylib's file system operations using
*   the RL_FS_* macros. Each file operation is wrapped to add detailed logging,
*   showing when and how file operations occur.
*
*   NOTE: This example implements file I/O functions locally to demonstrate the macros.
*         In a real project, you'd define these macros before compiling raylib itself.
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024-2026 David Konsumer (@konsumer) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Custom wrapper functions that add logging to file operations
static FILE *CustomFOpen(const char *filename, const char *mode)
{
    TraceLog(LOG_INFO, "CUSTOM FS: Opening file '%s' with mode '%s'", filename, mode);
    FILE *file = fopen(filename, mode);
    if (file) TraceLog(LOG_INFO, "CUSTOM FS: Successfully opened file '%s'", filename);
    else TraceLog(LOG_WARNING, "CUSTOM FS: Failed to open file '%s'", filename);
    return file;
}

static int CustomFClose(FILE *stream)
{
    TraceLog(LOG_INFO, "CUSTOM FS: Closing file");
    int result = fclose(stream);
    if (result == 0) TraceLog(LOG_INFO, "CUSTOM FS: Successfully closed file");
    else TraceLog(LOG_WARNING, "CUSTOM FS: Error closing file");
    return result;
}

static size_t CustomFRead(void *ptr, size_t size, size_t count, FILE *stream)
{
    TraceLog(LOG_INFO, "CUSTOM FS: Reading %zu elements of %zu bytes", count, size);
    size_t result = fread(ptr, size, count, stream);
    TraceLog(LOG_INFO, "CUSTOM FS: Read %zu elements", result);
    return result;
}

static size_t CustomFWrite(const void *ptr, size_t size, size_t count, FILE *stream)
{
    TraceLog(LOG_INFO, "CUSTOM FS: Writing %zu elements of %zu bytes", count, size);
    size_t result = fwrite(ptr, size, count, stream);
    TraceLog(LOG_INFO, "CUSTOM FS: Wrote %zu elements", result);
    return result;
}

static int CustomFSeek(FILE *stream, long offset, int whence)
{
    const char *whenceStr = (whence == SEEK_SET) ? "SEEK_SET" : (whence == SEEK_CUR) ? "SEEK_CUR" : "SEEK_END";
    TraceLog(LOG_INFO, "CUSTOM FS: Seeking to offset %ld from %s", offset, whenceStr);
    int result = fseek(stream, offset, whence);
    if (result == 0) TraceLog(LOG_INFO, "CUSTOM FS: Seek successful");
    else TraceLog(LOG_WARNING, "CUSTOM FS: Seek failed");
    return result;
}

static long CustomFTell(FILE *stream)
{
    long position = ftell(stream);
    TraceLog(LOG_INFO, "CUSTOM FS: Current position: %ld", position);
    return position;
}

// Redefine RL_FS_* macros to use our custom functions
#undef RL_FS_FOPEN
#undef RL_FS_FCLOSE
#undef RL_FS_FREAD
#undef RL_FS_FWRITE
#undef RL_FS_FSEEK
#undef RL_FS_FTELL

#define RL_FS_FOPEN(name, mode)                 CustomFOpen(name, mode)
#define RL_FS_FCLOSE(stream)                    CustomFClose(stream)
#define RL_FS_FREAD(ptr, size, count, stream)   CustomFRead(ptr, size, count, stream)
#define RL_FS_FWRITE(ptr, size, count, stream)  CustomFWrite(ptr, size, count, stream)
#define RL_FS_FSEEK(stream, offset, whence)     CustomFSeek(stream, offset, whence)
#define RL_FS_FTELL(stream)                     CustomFTell(stream)

//------------------------------------------------------------------------------------
// Local file I/O functions using RL_FS_* macros
//------------------------------------------------------------------------------------

// Load data from file into a buffer
static unsigned char *CustomLoadFileData(const char *fileName, int *dataSize)
{
    unsigned char *data = NULL;
    *dataSize = 0;

    FILE *file = RL_FS_FOPEN(fileName, "rb");

    if (file != NULL)
    {
        // Get file size
        RL_FS_FSEEK(file, 0, SEEK_END);
        long size = RL_FS_FTELL(file);
        RL_FS_FSEEK(file, 0, SEEK_SET);

        if (size > 0)
        {
            data = (unsigned char *)RL_MALLOC(size * sizeof(unsigned char));

            if (data != NULL)
            {
                unsigned int count = (unsigned int)RL_FS_FREAD(data, sizeof(unsigned char), size, file);
                *dataSize = count;

                if (count != size) TraceLog(LOG_WARNING, "FILEIO: [%s] File partially loaded (%i bytes out of %i)", fileName, count, size);
                else TraceLog(LOG_INFO, "FILEIO: [%s] File loaded successfully (%i bytes)", fileName, count);
            }
            else TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to allocated memory for file reading", fileName);
        }
        else TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to read file", fileName);

        RL_FS_FCLOSE(file);
    }
    else TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);

    return data;
}

// Save data to file from buffer
static bool CustomSaveFileData(const char *fileName, void *data, int dataSize)
{
    bool success = false;

    FILE *file = RL_FS_FOPEN(fileName, "wb");

    if (file != NULL)
    {
        unsigned int count = (unsigned int)RL_FS_FWRITE(data, sizeof(unsigned char), dataSize, file);

        if (count == 0) TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to write file", fileName);
        else if (count != dataSize) TraceLog(LOG_WARNING, "FILEIO: [%s] File partially written", fileName);
        else TraceLog(LOG_INFO, "FILEIO: [%s] File saved successfully (%i bytes)", fileName, count);

        RL_FS_FCLOSE(file);

        success = (count == dataSize);
    }
    else TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);

    return success;
}

// Load text data from file, returns a '\0' terminated string
static char *CustomLoadFileText(const char *fileName)
{
    char *text = NULL;

    FILE *file = RL_FS_FOPEN(fileName, "rt");

    if (file != NULL)
    {
        // Get file size
        RL_FS_FSEEK(file, 0, SEEK_END);
        long size = RL_FS_FTELL(file);
        RL_FS_FSEEK(file, 0, SEEK_SET);

        if (size > 0)
        {
            text = (char *)RL_MALLOC((size + 1) * sizeof(char));

            if (text != NULL)
            {
                unsigned int count = (unsigned int)RL_FS_FREAD(text, sizeof(char), size, file);

                if (count < size) text = (char *)RL_REALLOC(text, count + 1);

                text[count] = '\0';

                TraceLog(LOG_INFO, "FILEIO: [%s] Text file loaded successfully (%i bytes)", fileName, count);
            }
            else TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to allocated memory for file reading", fileName);
        }
        else TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to read file", fileName);

        RL_FS_FCLOSE(file);
    }
    else TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);

    return text;
}

// Save text data to file (write), string must be '\0' terminated
static bool CustomSaveFileText(const char *fileName, char *text)
{
    bool success = false;

    FILE *file = RL_FS_FOPEN(fileName, "wt");

    if (file != NULL)
    {
        int count = fprintf(file, "%s", text);

        if (count >= 0) TraceLog(LOG_INFO, "FILEIO: [%s] Text file saved successfully", fileName);
        else TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to write text file", fileName);

        RL_FS_FCLOSE(file);

        success = (count >= 0);
    }
    else TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);

    return success;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - custom file system");

    SetTraceLogLevel(LOG_INFO);

    TraceLog(LOG_INFO, "=======================================================");
    TraceLog(LOG_INFO, "Custom File System Example");
    TraceLog(LOG_INFO, "All file operations will be logged below");
    TraceLog(LOG_INFO, "=======================================================");

    // Test text file operations
    TraceLog(LOG_INFO, "=======================================================");
    TraceLog(LOG_INFO, "Testing CustomSaveFileText/CustomLoadFileText...");
    TraceLog(LOG_INFO, "=======================================================");

    const char *testText = "Hello from custom file system!\nThis demonstrates RL_FS_* macros.";
    const char *testFilePath = "custom_fs_test.txt";

    // Save text file - will use our custom file operations
    if (CustomSaveFileText(testFilePath, (char *)testText))
    {
        TraceLog(LOG_INFO, "=======================================================");
        TraceLog(LOG_INFO, "Text file saved successfully!");
        TraceLog(LOG_INFO, "=======================================================");
    }

    // Load text file - will use our custom file operations
    char *loadedText = CustomLoadFileText(testFilePath);
    if (loadedText != NULL)
    {
        TraceLog(LOG_INFO, "=======================================================");
        TraceLog(LOG_INFO, "Text file loaded successfully!");
        TraceLog(LOG_INFO, "Content: %s", loadedText);
        TraceLog(LOG_INFO, "=======================================================");
        RL_FREE(loadedText);
    }

    // Test binary data operations
    TraceLog(LOG_INFO, "=======================================================");
    TraceLog(LOG_INFO, "Testing CustomSaveFileData/CustomLoadFileData...");
    TraceLog(LOG_INFO, "=======================================================");

    unsigned char testData[256];
    for (int i = 0; i < 256; i++) testData[i] = (unsigned char)i;

    const char *testDataPath = "custom_fs_test.bin";

    // Save binary data
    if (CustomSaveFileData(testDataPath, testData, 256))
    {
        TraceLog(LOG_INFO, "=======================================================");
        TraceLog(LOG_INFO, "Binary data saved successfully!");
        TraceLog(LOG_INFO, "=======================================================");
    }

    // Load binary data
    int dataSize = 0;
    unsigned char *loadedData = CustomLoadFileData(testDataPath, &dataSize);
    if (loadedData != NULL)
    {
        TraceLog(LOG_INFO, "=======================================================");
        TraceLog(LOG_INFO, "Binary data loaded successfully! Size: %d bytes", dataSize);
        TraceLog(LOG_INFO, "=======================================================");
        RL_FREE(loadedData);
    }

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Custom File System Example", 190, 20, 20, DARKGRAY);
            DrawText("Using RL_FS_* macros to override file operations", 140, 50, 20, GRAY);

            DrawRectangle(20, 100, 760, 280, Fade(SKYBLUE, 0.3f));
            DrawRectangleLines(20, 100, 760, 280, BLUE);

            DrawText("All file operations are logged to the console!", 40, 120, 20, DARKBLUE);
            DrawText("Check your terminal/console to see:", 40, 150, 20, DARKGRAY);

            DrawText("1. CustomSaveFileText/CustomLoadFileText operations", 60, 180, 18, DARKGRAY);
            DrawText("2. CustomSaveFileData/CustomLoadFileData operations", 60, 205, 18, DARKGRAY);
            DrawText("3. Each fopen, fread, fwrite, fseek, ftell, fclose call", 60, 230, 18, DARKGRAY);

            DrawText("Files created:", 40, 270, 20, DARKGRAY);
            DrawText("- custom_fs_test.txt (text file)", 60, 295, 18, DARKGREEN);
            DrawText("- custom_fs_test.bin (binary data)", 60, 320, 18, DARKGREEN);

            DrawText("Press ESC to exit", 300, 410, 20, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
