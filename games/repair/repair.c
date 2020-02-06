/*******************************************************************************************
*
*   RE_PAIR [GLOBAL GAME JAM 2020]
*
*   Let's find your perfect match! 
*   Ready for dating? Do you need some face tweaks? Choose wisely!
*
*   This game has been created using raylib 3.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2020 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "screens/screens.h"    // NOTE: Defines global variable: currentScreen

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

GameScreen currentScreen = 0;
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };
Texture2D background = { 0 };
Texture2D texNPatch = { 0 };
NPatchInfo npInfo = { 0 };

Texture2D texHead, texHair, texNose, texMouth, texEyes, texComp;

Character playerBase = { 0 };
Character datingBase = { 0 };

Character player = { 0 };
Character dating = { 0 };

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
const int screenWidth = 1280;
const int screenHeight = 720;

// Required variables to manage screen transitions (fade-in, fade-out)
static float transAlpha = 0.0f;
static bool onTransition = false;
static bool transFadeOut = false;
static int transFromScreen = -1;
static int transToScreen = -1;

// NOTE: Some global variables that require to be visible for all screens,
// are defined in screens.h (i.e. currentScreen)
    
//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void ChangeToScreen(int screen);     // No transition effect

static void TransitionToScreen(int screen);
static void UpdateTransition(void);
static void DrawTransition(void);

static void UpdateDrawFrame(void);          // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "RE-PAIR [GGJ2020]");

    // Global data loading (assets that must be available in all screens, i.e. fonts)
    InitAudioDevice();

    font = LoadFont("resources/font.png");
    SetTextureFilter(font.texture, FILTER_BILINEAR);
    
    music = LoadMusicStream("resources/elevator_romance.ogg");
    fxCoin = LoadSound("resources/coin.wav");
    
    background = LoadTexture("resources/background.png");
    
    texNPatch = LoadTexture("resources/npatch.png");
    npInfo.sourceRec = (Rectangle){ 0, 0, 80, texNPatch.height },
    npInfo.left = 24;
    npInfo.top = 24;
    npInfo.right = 24;
    npInfo.bottom = 24;
    
    // Load required textures
    texHead = LoadTexture("resources/head_models.png");
    texHair = LoadTexture("resources/hair_models.png");
    texNose = LoadTexture("resources/nose_models.png");
    texMouth = LoadTexture("resources/mouth_models.png");
    texEyes = LoadTexture("resources/eyes_models.png");
    //texComp = LoadTexture("resources/comp_models.png");
    
    SetMusicVolume(music, 0.5f);
    //PlayMusicStream(music);

    // Setup and Init first screen
    currentScreen = LOGO;
    InitLogoScreen();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // Unload current screen data before closing
    switch (currentScreen)
    {
        case LOGO: UnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }
    
    // Unload all global loaded data (i.e. fonts) here!
    UnloadFont(font);
    UnloadMusicStream(music);
    UnloadSound(fxCoin);
    UnloadTexture(background);
    UnloadTexture(texNPatch);
    
    UnloadTexture(texHead);
    UnloadTexture(texHair);
    UnloadTexture(texNose);
    UnloadTexture(texMouth);
    UnloadTexture(texEyes);
    //UnloadTexture(texComp);

    CloseAudioDevice();     // Close audio context
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Public Functions Definition
//----------------------------------------------------------------------------------

Character GenerateCharacter(void)
{
    Character character = { 0 };
    
    // Generate player character!
    character.head = GetRandomValue(0, texHead.width/BASE_HEAD_WIDTH - 1);
    character.colHead = headColors[GetRandomValue(0, 5)];
    character.hair = GetRandomValue(0, texHair.width/BASE_HAIR_WIDTH - 1);
    character.colHair = hairColors[GetRandomValue(0, 9)];
    character.eyes = GetRandomValue(0, texEyes.width/BASE_EYES_WIDTH - 1);
    character.nose = GetRandomValue(0, texNose.width/BASE_NOSE_WIDTH - 1);
    character.mouth = GetRandomValue(0, texMouth.width/BASE_MOUTH_WIDTH - 1);
    
    // NOTE: No character customization at this point
    
    return character;
}

void CustomizeCharacter(Character *character)
{
    if (GetRandomValue(0, 1)) character->hair = GetRandomValue(0, texHair.width/BASE_HAIR_WIDTH - 1);
    if (GetRandomValue(0, 1)) character->colHair = hairColors[GetRandomValue(0, 9)];
    if (GetRandomValue(0, 1)) character->eyes = GetRandomValue(0, texEyes.width/BASE_EYES_WIDTH - 1);
    if (GetRandomValue(0, 1)) character->nose = GetRandomValue(0, texNose.width/BASE_NOSE_WIDTH - 1);
    if (GetRandomValue(0, 1)) character->mouth = GetRandomValue(0, texMouth.width/BASE_MOUTH_WIDTH - 1);
}

void DrawCharacter(Character character, Vector2 position)
{
    DrawTextureRec(texHair, (Rectangle){ BASE_HAIR_WIDTH*character.hair, 240, BASE_HAIR_WIDTH, texHair.height - 240 }, (Vector2){ position.x + (250 - BASE_HAIR_WIDTH)/2, position.y + 240 }, GetColor(character.colHair));
    DrawTextureRec(texHead, (Rectangle){ BASE_HEAD_WIDTH*character.head, 0, BASE_HEAD_WIDTH, texHead.height }, (Vector2){ position.x + (250 - BASE_HEAD_WIDTH)/2, position.y + 60 }, GetColor(character.colHead));
    DrawTextureRec(texHair, (Rectangle){ BASE_HAIR_WIDTH*character.hair, 0, BASE_HAIR_WIDTH, 240 }, (Vector2){ position.x + (250 - BASE_HAIR_WIDTH)/2, position.y }, GetColor(character.colHair));
    DrawTextureRec(texEyes, (Rectangle){ BASE_EYES_WIDTH*character.eyes, 0, BASE_EYES_WIDTH, texEyes.height }, (Vector2){ position.x + (250 - BASE_EYES_WIDTH)/2, position.y + 190 }, WHITE);
    DrawTextureRec(texNose, (Rectangle){ BASE_NOSE_WIDTH*character.nose, 0, BASE_NOSE_WIDTH, texNose.height }, (Vector2){ position.x + (250 - BASE_NOSE_WIDTH)/2, position.y + 275 }, GetColor(character.colHead));
    DrawTextureRec(texMouth, (Rectangle){ BASE_MOUTH_WIDTH*character.mouth, 0, BASE_MOUTH_WIDTH, texMouth.height }, (Vector2){ position.x + (250 - BASE_MOUTH_WIDTH)/2, position.y + 370 }, GetColor(character.colHead));
}

// Gui Button
bool GuiButton(Rectangle bounds, const char *text, int forcedState)
{
    static const int textColor[4] = { 0xeff6ffff, 0x78e782ff, 0xb04d5fff, 0xd6d6d6ff };
    
    int state = (forcedState >= 0)? forcedState : 0;                // NORMAL
    bool pressed = false;
    Vector2 textSize = MeasureTextEx(font, text, font.baseSize, 1);

    // Update control
    //--------------------------------------------------------------------
    if ((state < 3) && (forcedState < 0))
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = 2;    // PRESSED
            else state = 1;                                         // FOCUSED

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || IsGestureDetected(GESTURE_TAP))
            {
                pressed = true;
                PlaySound(fxCoin);
            }
        }
    }
            
    npInfo.sourceRec.x = 80*state;

    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    //DrawRectangleRec(bounds, GREEN);
    //DrawRectangleLinesEx(bounds, 4, DARKGREEN);
    DrawTextureNPatch(texNPatch, npInfo, bounds, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    DrawTextEx(font, text, (Vector2){ bounds.x + bounds.width/2 - textSize.x/2, bounds.y + bounds.height/2 - textSize.y/2 + 4 }, font.baseSize, 1, GetColor(textColor[state]));
    //------------------------------------------------------------------
    
    return pressed;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Change to next screen, no transition
static void ChangeToScreen(int screen)
{
    // Unload current screen
    switch (currentScreen)
    {
        case LOGO: UnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }
    
    // Init next screen
    switch (screen)
    {
        case LOGO: InitLogoScreen(); break;
        case TITLE: InitTitleScreen(); break;
        case GAMEPLAY: InitGameplayScreen(); break;
        case ENDING: InitEndingScreen(); break;
        default: break;
    }
    
    currentScreen = screen;
}

// Define transition to next screen
static void TransitionToScreen(int screen)
{
    onTransition = true;
    transFadeOut = false;
    transFromScreen = currentScreen;
    transToScreen = screen;
    transAlpha = 0.0f;
}

// Update transition effect
static void UpdateTransition(void)
{
    if (!transFadeOut)
    {
        transAlpha += 0.05f;
        
        // NOTE: Due to float internal representation, condition jumps on 1.0f instead of 1.05f
        // For that reason we compare against 1.01f, to avoid last frame loading stop
        if (transAlpha > 1.01f)
        {
            transAlpha = 1.0f;
        
            // Unload current screen
            switch (transFromScreen)
            {
                case LOGO: UnloadLogoScreen(); break;
                case TITLE: UnloadTitleScreen(); break;
                case GAMEPLAY: UnloadGameplayScreen(); break;
                case ENDING: UnloadEndingScreen(); break;
                default: break;
            }
            
            // Load next screen
            switch (transToScreen)
            {
                case LOGO: InitLogoScreen(); break;
                case TITLE: InitTitleScreen(); break;
                case GAMEPLAY: InitGameplayScreen(); break;
                case ENDING: InitEndingScreen(); break;
                default: break;
            }
            
            currentScreen = transToScreen;
            
            // Activate fade out effect to next loaded screen
            transFadeOut = true;
        }
    }
    else  // Transition fade out logic
    {
        transAlpha -= 0.02f;
        
        if (transAlpha < -0.01f)
        {
            transAlpha = 0.0f;
            transFadeOut = false;
            onTransition = false;
            transFromScreen = -1;
            transToScreen = -1;
        }
    }
}

// Draw transition effect (full-screen rectangle)
static void DrawTransition(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}

// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdateMusicStream(music);       // NOTE: Music keeps playing between screens
    
    if (!onTransition)
    {
        switch(currentScreen) 
        {
            case LOGO: 
            {
                UpdateLogoScreen();
                
                if (FinishLogoScreen())
                {
                    TransitionToScreen(TITLE);
                    PlayMusicStream(music);
                }

            } break;
            case TITLE: 
            {
                UpdateTitleScreen();
                    
                if (FinishTitleScreen() == 1) TransitionToScreen(GAMEPLAY);
                //else if (FinishTitleScreen() == 2) TransitionToScreen(GAMEPLAY);

            } break;
            case GAMEPLAY:
            {
                UpdateGameplayScreen();
                
                if (FinishGameplayScreen() == 1) TransitionToScreen(ENDING);
                //else if (FinishGameplayScreen() == 2) TransitionToScreen(TITLE);

            } break;
            case ENDING:
            { 
                UpdateEndingScreen();
                
                if (FinishEndingScreen() == 1) TransitionToScreen(TITLE);

            } break;
            default: break;
        }
    }
    else UpdateTransition();    // Update transition (fade-in, fade-out)
    //----------------------------------------------------------------------------------
    
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
        
        ClearBackground(RAYWHITE);
            
        switch(currentScreen) 
        {
            case LOGO: DrawLogoScreen(); break;
            case TITLE: DrawTitleScreen(); break;
            case GAMEPLAY: DrawGameplayScreen(); break;
            case ENDING: DrawEndingScreen(); break;
            default: break;
        }
         
        // Draw full screen rectangle in front of everything
        if (onTransition) DrawTransition();
        
        //DrawFPS(10, 10);
        
    EndDrawing();
    //----------------------------------------------------------------------------------
}
