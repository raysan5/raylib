/*******************************************************************************************
*
*   raylib [textures] example - Fog of war
*
*   Example originally created with raylib 4.2, last time updated with raylib 4.2
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2018-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>                 // Required for: calloc(), free()

#define MAP_TILE_SIZE    32         // Tiles size 32x32 pixels
#define PLAYER_SIZE      16         // Player size
#define PLAYER_TILE_VISIBILITY  2   // Player can see 2 tiles around its position

// Map data type
typedef struct Map {
    unsigned int tilesX;            // Number of tiles in X axis
    unsigned int tilesY;            // Number of tiles in Y axis
    unsigned char *tileIds;         // Tile ids (tilesX*tilesY), defines type of tile to draw
    unsigned char *tileFog;         // Tile fog state (tilesX*tilesY), defines if a tile has fog or half-fog
} Map;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - fog of war");

    Map map = { 0 };
    map.tilesX = 25;
    map.tilesY = 15;

    // NOTE: We can have up to 256 values for tile ids and for tile fog state,
    // probably we don't need that many values for fog state, it can be optimized
    // to use only 2 bits per fog state (reducing size by 4) but logic will be a bit more complex
    map.tileIds = (unsigned char *)calloc(map.tilesX*map.tilesY, sizeof(unsigned char));
    map.tileFog = (unsigned char *)calloc(map.tilesX*map.tilesY, sizeof(unsigned char));

    // Load map tiles (generating 2 random tile ids for testing)
    // NOTE: Map tile ids should be probably loaded from an external map file
    for (unsigned int i = 0; i < map.tilesY*map.tilesX; i++) map.tileIds[i] = GetRandomValue(0, 1);

    // Player position on the screen (pixel coordinates, not tile coordinates)
    Vector2 playerPosition = { 180, 130 };
    int playerTileX = 0;
    int playerTileY = 0;

    // Render texture to render fog of war
    // NOTE: To get an automatic smooth-fog effect we use a render texture to render fog
    // at a smaller size (one pixel per tile) and scale it on drawing with bilinear filtering
    RenderTexture2D fogOfWar = LoadRenderTexture(map.tilesX, map.tilesY);
    SetTextureFilter(fogOfWar.texture, TEXTURE_FILTER_BILINEAR);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Move player around
        if (IsKeyDown(KEY_RIGHT)) playerPosition.x += 5;
        if (IsKeyDown(KEY_LEFT)) playerPosition.x -= 5;
        if (IsKeyDown(KEY_DOWN)) playerPosition.y += 5;
        if (IsKeyDown(KEY_UP)) playerPosition.y -= 5;

        // Check player position to avoid moving outside tilemap limits
        if (playerPosition.x < 0) playerPosition.x = 0;
        else if ((playerPosition.x + PLAYER_SIZE) > (map.tilesX*MAP_TILE_SIZE)) playerPosition.x = (float)map.tilesX*MAP_TILE_SIZE - PLAYER_SIZE;
        if (playerPosition.y < 0) playerPosition.y = 0;
        else if ((playerPosition.y + PLAYER_SIZE) > (map.tilesY*MAP_TILE_SIZE)) playerPosition.y = (float)map.tilesY*MAP_TILE_SIZE - PLAYER_SIZE;

        // Previous visited tiles are set to partial fog
        for (unsigned int i = 0; i < map.tilesX*map.tilesY; i++) if (map.tileFog[i] == 1) map.tileFog[i] = 2;

        // Get current tile position from player pixel position
        playerTileX = (int)((playerPosition.x + MAP_TILE_SIZE/2)/MAP_TILE_SIZE);
        playerTileY = (int)((playerPosition.y + MAP_TILE_SIZE/2)/MAP_TILE_SIZE);

        // Check visibility and update fog
        // NOTE: We check tilemap limits to avoid processing tiles out-of-array-bounds (it could crash program)
        for (int y = (playerTileY - PLAYER_TILE_VISIBILITY); y < (playerTileY + PLAYER_TILE_VISIBILITY); y++)
            for (int x = (playerTileX - PLAYER_TILE_VISIBILITY); x < (playerTileX + PLAYER_TILE_VISIBILITY); x++)
                if ((x >= 0) && (x < (int)map.tilesX) && (y >= 0) && (y < (int)map.tilesY)) map.tileFog[y*map.tilesX + x] = 1;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        // Draw fog of war to a small render texture for automatic smoothing on scaling
        BeginTextureMode(fogOfWar);
            ClearBackground(BLANK);
            for (unsigned int y = 0; y < map.tilesY; y++)
                for (unsigned int x = 0; x < map.tilesX; x++)
                    if (map.tileFog[y*map.tilesX + x] == 0) DrawRectangle(x, y, 1, 1, BLACK);
                    else if (map.tileFog[y*map.tilesX + x] == 2) DrawRectangle(x, y, 1, 1, Fade(BLACK, 0.8f));
        EndTextureMode();

        BeginDrawing();

            ClearBackground(RAYWHITE);

            for (unsigned int y = 0; y < map.tilesY; y++)
            {
                for (unsigned int x = 0; x < map.tilesX; x++)
                {
                    // Draw tiles from id (and tile borders)
                    DrawRectangle(x*MAP_TILE_SIZE, y*MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE,
                                  (map.tileIds[y*map.tilesX + x] == 0)? BLUE : Fade(BLUE, 0.9f));
                    DrawRectangleLines(x*MAP_TILE_SIZE, y*MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE, Fade(DARKBLUE, 0.5f));
                }
            }

            // Draw player
            DrawRectangleV(playerPosition, (Vector2){ PLAYER_SIZE, PLAYER_SIZE }, RED);


            // Draw fog of war (scaled to full map, bilinear filtering)
            DrawTexturePro(fogOfWar.texture, (Rectangle){ 0, 0, (float)fogOfWar.texture.width, (float)-fogOfWar.texture.height },
                           (Rectangle){ 0, 0, (float)map.tilesX*MAP_TILE_SIZE, (float)map.tilesY*MAP_TILE_SIZE },
                           (Vector2){ 0, 0 }, 0.0f, WHITE);

            // Draw player current tile
            DrawText(TextFormat("Current tile: [%i,%i]", playerTileX, playerTileY), 10, 10, 20, LIME);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    free(map.tileIds);      // Free allocated map tile ids
    free(map.tileFog);      // Free allocated map tile fog state

    UnloadRenderTexture(fogOfWar);  // Unload render texture

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
