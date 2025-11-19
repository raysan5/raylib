/*******************************************************************************************
*
*   raylib [core] example - compute hash
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static char *GetDataAsHexText(const unsigned int *data, int dataSize);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - compute hash");

    // UI controls variables
    char textInput[96] = "The quick brown fox jumps over the lazy dog.";
    bool textBoxEditMode = false;
    bool btnComputeHashes = false;

    // Data hash values
    unsigned int hashCRC32 = 0;
    unsigned int *hashMD5 = NULL;
    unsigned int *hashSHA1 = NULL;
    unsigned int *hashSHA256 = NULL;

    // Base64 encoded data
    char *base64Text = NULL;
    int base64TextSize = 0;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (btnComputeHashes)
        {
            int textInputLen = strlen(textInput);

            // Encode data to Base64 string (includes NULL terminator), memory must be MemFree()
            base64Text = EncodeDataBase64((unsigned char *)textInput, textInputLen, &base64TextSize);

            hashCRC32 = ComputeCRC32((unsigned char *)textInput, textInputLen);     // Compute CRC32 hash code (4 bytes)
            hashMD5 = ComputeMD5((unsigned char *)textInput, textInputLen);         // Compute MD5 hash code, returns static int[4] (16 bytes)
            hashSHA1 = ComputeSHA1((unsigned char *)textInput, textInputLen);       // Compute SHA1 hash code, returns static int[5] (20 bytes)
            hashSHA256 = ComputeSHA256((unsigned char *)textInput, textInputLen);   // Compute SHA256 hash code, returns static int[8] (32 bytes)
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
            GuiSetStyle(DEFAULT, TEXT_SPACING, 2);
            GuiLabel((Rectangle){ 40, 26, 720, 32 }, "INPUT DATA (TEXT):");
            GuiSetStyle(DEFAULT, TEXT_SPACING, 1);
            GuiSetStyle(DEFAULT, TEXT_SIZE, 10);

            if (GuiTextBox((Rectangle){ 40, 64, 720, 32 }, textInput, 95, textBoxEditMode)) textBoxEditMode = !textBoxEditMode;

            btnComputeHashes = GuiButton((Rectangle){ 40, 64 + 40, 720, 32 }, "COMPUTE INPUT DATA HASHES");

            GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
            GuiSetStyle(DEFAULT, TEXT_SPACING, 2);
            GuiLabel((Rectangle){ 40, 160, 720, 32 }, "INPUT DATA HASH VALUES:");
            GuiSetStyle(DEFAULT, TEXT_SPACING, 1);
            GuiSetStyle(DEFAULT, TEXT_SIZE, 10);

            GuiSetStyle(TEXTBOX, TEXT_READONLY, 1);
            GuiLabel((Rectangle){ 40, 200, 120, 32 }, "CRC32 [32 bit]:");
            GuiTextBox((Rectangle){ 40 + 120, 200, 720 - 120, 32 }, GetDataAsHexText(&hashCRC32, 1), 120, false);
            GuiLabel((Rectangle){ 40, 200 + 36, 120, 32 }, "MD5 [128 bit]:");
            GuiTextBox((Rectangle){ 40 + 120, 200 + 36, 720 - 120, 32 }, GetDataAsHexText(hashMD5, 4), 120, false);
            GuiLabel((Rectangle){ 40, 200 + 36*2, 120, 32 }, "SHA1 [160 bit]:");
            GuiTextBox((Rectangle){ 40 + 120, 200 + 36*2, 720 - 120, 32 }, GetDataAsHexText(hashSHA1, 5), 120, false);
            GuiLabel((Rectangle){ 40, 200 + 36*3, 120, 32 }, "SHA256 [256 bit]:");
            GuiTextBox((Rectangle){ 40 + 120, 200 + 36*3, 720 - 120, 32 }, GetDataAsHexText(hashSHA256, 8), 120, false);

            GuiSetState(STATE_FOCUSED);
            GuiLabel((Rectangle){ 40, 200 + 36*5 - 30, 320, 32 }, "BONUS - BAS64 ENCODED STRING:");
            GuiSetState(STATE_NORMAL);
            GuiLabel((Rectangle){ 40, 200 + 36*5, 120, 32 }, "BASE64 ENCODING:");
            GuiTextBox((Rectangle){ 40 + 120, 200 + 36*5, 720 - 120, 32 }, base64Text, 120, false);
            GuiSetStyle(TEXTBOX, TEXT_READONLY, 0);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    MemFree(base64Text);    // Free Base64 text data

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
static char *GetDataAsHexText(const unsigned int *data, int dataSize)
{
    static char text[128] = { 0 };
    memset(text, 0, 128);

    if ((data != NULL) && (dataSize > 0) && (dataSize < ((128/8) - 1)))
    {
        for (int i = 0; i < dataSize; i++) TextCopy(text + i*8, TextFormat("%08X", data[i]));
    }
    else TextCopy(text, "00000000");

    return text;
}
