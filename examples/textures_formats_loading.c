/*******************************************************************************************
*
*   raylib [textures] example - texture formats loading (compressed and uncompressed)
*
*   NOTE: This example requires raylib OpenGL 3.3+ or ES2 versions for compressed textures,
*         OpenGL 1.1 does not support compressed textures, only uncompressed ones.
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define NUM_TEXTURES    24

typedef enum { 
    PNG_R8G8B8A8 = 0, 
    PVR_GRAYSCALE, 
    PVR_GRAY_ALPHA, 
    PVR_R5G6B5, 
    PVR_R5G5B5A1, 
    PVR_R4G4B4A4,
    DDS_R5G6B5,
    DDS_R5G5B5A1,
    DDS_R4G4B4A4,
    DDS_R8G8B8A8,
    DDS_DXT1_RGB,
    DDS_DXT1_RGBA,
    DDS_DXT3_RGBA,
    DDS_DXT5_RGBA,
    PKM_ETC1_RGB,
    PKM_ETC2_RGB,
    PKM_ETC2_EAC_RGBA,
    KTX_ETC1_RGB,
    KTX_ETC2_RGB,
    KTX_ETC2_EAC_RGBA,
    ASTC_4x4_LDR,
    ASTC_8x8_LDR,
    PVR_PVRT_RGB,
    PVR_PVRT_RGBA
    
} TextureFormats;

static const char *formatText[] = {
    "PNG_R8G8B8A8",
    "PVR_GRAYSCALE",
    "PVR_GRAY_ALPHA",
    "PVR_R5G6B5",
    "PVR_R5G5B5A1",
    "PVR_R4G4B4A4",
    "DDS_R5G6B5",
    "DDS_R5G5B5A1",
    "DDS_R4G4B4A4",
    "DDS_R8G8B8A8",
    "DDS_DXT1_RGB",
    "DDS_DXT1_RGBA",
    "DDS_DXT3_RGBA",
    "DDS_DXT5_RGBA",
    "PKM_ETC1_RGB",
    "PKM_ETC2_RGB",
    "PKM_ETC2_EAC_RGBA",
    "KTX_ETC1_RGB",
    "KTX_ETC2_RGB",
    "KTX_ETC2_EAC_RGBA",
    "ASTC_4x4_LDR",
    "ASTC_8x8_LDR",
    "PVR_PVRT_RGB",
    "PVR_PVRT_RGBA"
};

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - texture formats loading");
    
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    
    Texture2D sonic[NUM_TEXTURES];

    sonic[PNG_R8G8B8A8] = LoadTexture("resources/texture_formats/sonic.png");
    
    // Load UNCOMPRESSED PVR texture data
    sonic[PVR_GRAYSCALE] = LoadTexture("resources/texture_formats/sonic_GRAYSCALE.pvr");
    sonic[PVR_GRAY_ALPHA] = LoadTexture("resources/texture_formats/sonic_L8A8.pvr");
    sonic[PVR_R5G6B5] = LoadTexture("resources/texture_formats/sonic_R5G6B5.pvr");
    sonic[PVR_R5G5B5A1] = LoadTexture("resources/texture_formats/sonic_R5G5B5A1.pvr");
    sonic[PVR_R4G4B4A4] = LoadTexture("resources/texture_formats/sonic_R4G4B4A4.pvr");
    
    // Load UNCOMPRESSED DDS texture data
    sonic[DDS_R5G6B5] = LoadTexture("resources/texture_formats/sonic_R5G6B5.dds");
    sonic[DDS_R5G5B5A1] = LoadTexture("resources/texture_formats/sonic_A1R5G5B5.dds");
    sonic[DDS_R4G4B4A4] = LoadTexture("resources/texture_formats/sonic_A4R4G4B4.dds");
    sonic[DDS_R8G8B8A8] = LoadTexture("resources/texture_formats/sonic_A8R8G8B8.dds");
   
    // Load COMPRESSED DXT DDS texture data (if supported)
    sonic[DDS_DXT1_RGB] = LoadTexture("resources/texture_formats/sonic_DXT1_RGB.dds");
    sonic[DDS_DXT1_RGBA] = LoadTexture("resources/texture_formats/sonic_DXT1_RGBA.dds");
    sonic[DDS_DXT3_RGBA] = LoadTexture("resources/texture_formats/sonic_DXT3_RGBA.dds");
    sonic[DDS_DXT5_RGBA] = LoadTexture("resources/texture_formats/sonic_DXT5_RGBA.dds");
    
    // Load COMPRESSED ETC texture data (if supported)
    sonic[PKM_ETC1_RGB] = LoadTexture("resources/texture_formats/sonic_ETC1_RGB.pkm");
    sonic[PKM_ETC2_RGB] = LoadTexture("resources/texture_formats/sonic_ETC2_RGB.pkm");
    sonic[PKM_ETC2_EAC_RGBA] = LoadTexture("resources/texture_formats/sonic_ETC2_EAC_RGBA.pkm");
    
    sonic[KTX_ETC1_RGB] = LoadTexture("resources/texture_formats/sonic_ETC1_RGB.ktx");
    sonic[KTX_ETC2_RGB] = LoadTexture("resources/texture_formats/sonic_ETC2_RGB.ktx");
    sonic[KTX_ETC2_EAC_RGBA] = LoadTexture("resources/texture_formats/sonic_ETC2_EAC_RGBA.ktx");
    
    // Load COMPRESSED ASTC texture data (if supported)
    sonic[ASTC_4x4_LDR] = LoadTexture("resources/texture_formats/sonic_ASTC_4x4_ldr.astc");
    sonic[ASTC_8x8_LDR] = LoadTexture("resources/texture_formats/sonic_ASTC_8x8_ldr.astc");

    // Load COMPRESSED PVR texture data (if supported)
    sonic[PVR_PVRT_RGB] = LoadTexture("resources/texture_formats/sonic_PVRT_RGB.pvr");
    sonic[PVR_PVRT_RGBA] = LoadTexture("resources/texture_formats/sonic_PVRT_RGBA.pvr");
    
    int selectedFormat = PNG_R8G8B8A8;
    
    Rectangle selectRecs[NUM_TEXTURES];
    
    for (int i = 0; i < NUM_TEXTURES; i++)
    {
        if (i < NUM_TEXTURES/2) selectRecs[i] = (Rectangle){ 40, 45 + 32*i, 150, 30 };
        else selectRecs[i] = (Rectangle){ 40 + 152, 45 + 32*(i - NUM_TEXTURES/2), 150, 30 };
    }
    
    // Texture sizes in KB
    float textureSizes[NUM_TEXTURES] = { 
        512*512*32/8/1024,      //PNG_R8G8B8A8 (32 bpp)
        512*512*8/8/1024,       //PVR_GRAYSCALE (8 bpp)
        512*512*16/8/1024,      //PVR_GRAY_ALPHA (16 bpp) 
        512*512*16/8/1024,      //PVR_R5G6B5 (16 bpp)
        512*512*16/8/1024,      //PVR_R5G5B5A1 (16 bpp) 
        512*512*16/8/1024,      //PVR_R4G4B4A4 (16 bpp)
        512*512*16/8/1024,      //DDS_R5G6B5 (16 bpp)
        512*512*16/8/1024,      //DDS_R5G5B5A1 (16 bpp)
        512*512*16/8/1024,      //DDS_R4G4B4A4 (16 bpp)
        512*512*32/8/1024,      //DDS_R8G8B8A8 (32 bpp)
        512*512*4/8/1024,       //DDS_DXT1_RGB (4 bpp) -Compressed-
        512*512*4/8/1024,       //DDS_DXT1_RGBA (4 bpp) -Compressed-
        512*512*8/8/1024,       //DDS_DXT3_RGBA (8 bpp) -Compressed-
        512*512*8/8/1024,       //DDS_DXT5_RGBA (8 bpp) -Compressed-
        512*512*4/8/1024,       //PKM_ETC1_RGB (4 bpp) -Compressed-
        512*512*4/8/1024,       //PKM_ETC2_RGB (4 bpp) -Compressed-
        512*512*8/8/1024,       //PKM_ETC2_EAC_RGBA (8 bpp) -Compressed-
        512*512*4/8/1024,       //KTX_ETC1_RGB (4 bpp) -Compressed-
        512*512*4/8/1024,       //KTX_ETC2_RGB (4 bpp) -Compressed-
        512*512*8/8/1024,       //KTX_ETC2_EAC_RGBA (8 bpp) -Compressed-
        512*512*8/8/1024,       //ASTC_4x4_LDR (8 bpp) -Compressed-
        512*512*2/8/1024,       //ASTC_8x8_LDR (2 bpp) -Compressed-
        512*512*4/8/1024,       //PVR_PVRT_RGB (4 bpp) -Compressed-
        512*512*4/8/1024,       //PVR_PVRT_RGBA (4 bpp) -Compressed-
    };

    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_DOWN))
        {
            selectedFormat++;
            if (selectedFormat >= NUM_TEXTURES) selectedFormat = 0;
        }
        else if (IsKeyPressed(KEY_UP))
        {
            selectedFormat--;
            if (selectedFormat < 0) selectedFormat = NUM_TEXTURES - 1;
        }
        else if (IsKeyPressed(KEY_RIGHT))
        {
            if (selectedFormat < NUM_TEXTURES/2) selectedFormat += NUM_TEXTURES/2;
        }
        else if (IsKeyPressed(KEY_LEFT))
        {
            if (selectedFormat >= NUM_TEXTURES/2) selectedFormat -= NUM_TEXTURES/2;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------

        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            // Draw rectangles
            for (int i = 0; i < NUM_TEXTURES; i++)
            {
                if (i == selectedFormat)
                {
                    DrawRectangleRec(selectRecs[i], SKYBLUE);
                    DrawRectangleLines(selectRecs[i].x, selectRecs[i].y, selectRecs[i].width, selectRecs[i].height, BLUE);
                    DrawText(formatText[i], selectRecs[i].x + selectRecs[i].width/2 - MeasureText(formatText[i], 10)/2, selectRecs[i].y + 11, 10, DARKBLUE);
                }
                else
                {
                    DrawRectangleRec(selectRecs[i], LIGHTGRAY);
                    DrawRectangleLines(selectRecs[i].x, selectRecs[i].y, selectRecs[i].width, selectRecs[i].height, GRAY);
                    DrawText(formatText[i], selectRecs[i].x + selectRecs[i].width/2 - MeasureText(formatText[i], 10)/2, selectRecs[i].y + 11, 10, DARKGRAY);
                }
            }
			
            // Draw selected texture
            if (sonic[selectedFormat].id != 0)
            {
                DrawTexture(sonic[selectedFormat], 350, 0, WHITE);
            }
            else 
            {
                DrawRectangleLines(488, 165, 200, 110, DARKGRAY);
                DrawText("FORMAT", 550, 180, 20, MAROON);
                DrawText("NOT SUPPORTED", 500, 210, 20, MAROON);
                DrawText("ON YOUR GPU", 520, 240, 20, MAROON);
            }
            
            DrawText("Select texture format (use cursor keys):", 40, 26, 10, DARKGRAY);
            DrawText("Required GPU memory size (VRAM):", 40, 442, 10, DARKGRAY);
            DrawText(FormatText("%4.0f KB", textureSizes[selectedFormat]), 240, 435, 20, DARKBLUE);
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    for (int i = 0; i < NUM_TEXTURES; i++) UnloadTexture(sonic[i]);

    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}