/*******************************************************************************************
*
*   raylib [core] example - 2d camera extended
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#define G 400
#define PLAYER_JUMP_SPD 350.f
#define PLAYER_HOR_SPD 200.f

typedef struct Player {
    Vector2 pos;
    float vel;
    int canJump;
} Player;

typedef struct EnvItem {
    Rectangle rect;
    int blocking;
    Color color;
} EnvItem;

void updateCameraCenter(
        float delta, 
        Camera2D *camera, 
        Player *player, 
        EnvItem *envItems, 
        int envItemsLength, 
        int width, int height
) {
    camera->offset = (Vector2){ width/2, height/2 };
    camera->target = player->pos;
}

void updateCameraCenterInsideMap(
        float delta, 
        Camera2D *camera, 
        Player *player, 
        EnvItem *envItems, 
        int envItemsLength, 
        int width, int height
) {
    camera->target = player->pos;
    camera->offset = (Vector2){ width/2, height/2 };
    float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;
    for (int i = 0; i < envItemsLength; i++) {
        EnvItem *ei = envItems + i;
        minX = fminf(ei->rect.x, minX);
        maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
        minY = fminf(ei->rect.y, minY);
        maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
    }
    Vector2 max = GetWorldToScreen2D((Vector2){ maxX, maxY }, *camera);
    Vector2 min = GetWorldToScreen2D((Vector2){ minX, minY }, *camera);
    if (max.x < width) {
        camera->offset.x = width - (max.x - width/2);
    }
    if (max.y < height) {
        camera->offset.y = height - (max.y - height/2);
    }
    if (min.x > 0) {
        camera->offset.x = width/2 - min.x;
    }
    if (min.y > 0) {
        camera->offset.y = height/2- min.y;
    }
}

void updateCameraCenterSmoothFollow(
        float delta, 
        Camera2D *camera, 
        Player *player, 
        EnvItem *envItems, 
        int envItemsLength, 
        int width, int height
) {
    static float minSpeed = 30;
    static float minEffectLength = 10;
    static float fractionSpeed = 0.8f;
    camera->offset = (Vector2){ width/2, height/2 };
    Vector2 diff = Vector2Subtract(player->pos, camera->target);
    float length = Vector2Length(diff);
    if (length > minEffectLength) {
        float speed = fmaxf(fractionSpeed * length, minSpeed);
        camera->target = Vector2Add(camera->target, Vector2Scale(diff, speed*delta/length));
    }
}

void updateCameraEvenOutOnLanding(
        float delta, 
        Camera2D *camera, 
        Player *player, 
        EnvItem *envItems, 
        int envItemsLength, 
        int width, int height
) {
    static float evenOutSpeed = 700;
    static int eveningOut = false;
    static float evenOutTarget;
    camera->offset = (Vector2){ width/2, height/2 };
    camera->target.x = player->pos.x;
    if (eveningOut) {
        if (evenOutTarget > camera->target.y) {
            camera->target.y += evenOutSpeed * delta;
            if (camera->target.y > evenOutTarget) {
                camera->target.y = evenOutTarget;
                eveningOut = 0;
            }
        } else {
            camera->target.y -= evenOutSpeed * delta;
            if (camera->target.y < evenOutTarget) {
                camera->target.y = evenOutTarget;
                eveningOut = 0;
            }
        }
    } else {
        if (player->canJump && 
            player->vel == 0 && 
            player->pos.y != camera->target.y
        ) {
            eveningOut = 1;
            evenOutTarget = player->pos.y;
        }
    }
}

void updateCameraPlayerBoundsPush(
        float delta, 
        Camera2D *camera, 
        Player *player, 
        EnvItem *envItems, 
        int envItemsLength, 
        int width, int height
) {
    static Vector2 bbox = { 0.2f, 0.2f };

    Vector2 bboxWorldMin = GetScreenToWorld2D((Vector2){ (1 - bbox.x) * 0.5 * width, (1 - bbox.y) * 0.5 * height }, *camera);
    Vector2 bboxWorldMax = GetScreenToWorld2D((Vector2){ (1 + bbox.x) * 0.5 * width, (1 + bbox.y) * 0.5 * height }, *camera);
    camera->offset = (Vector2){ (1 - bbox.x) * 0.5 * width, (1 - bbox.y) * 0.5 * height };

    if (player->pos.x < bboxWorldMin.x) {
        camera->target.x = player->pos.x;
    }
    if (player->pos.y < bboxWorldMin.y) {
        camera->target.y = player->pos.y;
    }
    if (player->pos.x > bboxWorldMax.x) {
        camera->target.x = bboxWorldMin.x + (player->pos.x - bboxWorldMax.x);
    }
    if (player->pos.y > bboxWorldMax.y) {
        camera->target.y = bboxWorldMin.y + (player->pos.y - bboxWorldMax.y);
    }
}


void updatePlayer(float delta, Player *player, EnvItem *envItems, int envItemsLength) {
    if (IsKeyDown(KEY_LEFT)) player->pos.x -= PLAYER_HOR_SPD*delta;
    if (IsKeyDown(KEY_RIGHT)) player->pos.x += PLAYER_HOR_SPD*delta;
    if (IsKeyDown(KEY_SPACE) && player->canJump) {
        player->vel = -PLAYER_JUMP_SPD;
        player->canJump = 0;
    }

    int hitObstacle = 0;
    for (int i = 0; i < envItemsLength; i++) {
        EnvItem *ei = envItems + i;
        Vector2 *p = &(player->pos);
        if (ei->blocking &&
            ei->rect.x <= p->x && 
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y >= p->y &&
            ei->rect.y < p->y + player->vel * delta) 
        {
            hitObstacle = 1;
            player->vel = 0.0f;
            p->y = ei->rect.y;
        }
    }
    if (!hitObstacle) {
        player->pos.y += player->vel * delta;
        player->vel += G * delta;
        player->canJump = 0;
    } else {
        player->canJump = 1;
    }
}

void renderWorld(Player *player, EnvItem *envItems, int envItemsLength) {
    for (int i = 0; i < envItemsLength; i++) {
        DrawRectangleRec(envItems[i].rect, envItems[i].color);
    }
    Rectangle playerRect = { player->pos.x - 20, player->pos.y - 40, 40, 40 };
    DrawRectangleRec(playerRect, RED);
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera");
    SetTargetFPS(60);

    Player player;
    player.pos = (Vector2){ 400, 280 };
    player.vel = 0;
    player.canJump = 0;
    EnvItem envItems[] = {
        {{ 0, 0, 1000, 400 }, 0, LIGHTGRAY },
        {{ 0, 400, 1000, 200 }, 1, GRAY },
        {{ 300, 200, 400, 10 }, 1, GRAY },
        {{ 250, 300, 100, 10 }, 1, GRAY },
        {{ 650, 300, 100, 10 }, 1, GRAY }
    };
    int envItemsLength = sizeof(envItems) / sizeof (envItems[0]);

    Camera2D camera = { 0 };
    camera.target = player.pos;
    camera.offset = (Vector2){ screenWidth/2, screenHeight/2 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    int cameraOption = 0;
    void (*cameraUpdaters[])(float, Camera2D*, Player*, EnvItem*, int, int, int) = {
        updateCameraCenter,
        updateCameraCenterInsideMap,
        updateCameraCenterSmoothFollow,
        updateCameraEvenOutOnLanding,
        updateCameraPlayerBoundsPush
    };
    int cameraUpdatersLength = sizeof(cameraUpdaters) / sizeof(cameraUpdaters[0]);
    char* cameraDescriptions[] = {
        "Follow player center",
        "Follow player center, but clamp to map edges",
        "Follow player center; smoothed",
        "Follow player center horizontally; updateplayer center vertically after landing",
        "Player push camera on getting too close to screen edge"
    };
    
    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        updatePlayer(delta, &player, envItems, envItemsLength);

        camera.zoom += ((float)GetMouseWheelMove()*0.05f);
        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.25f) camera.zoom = 0.25f;
        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 1.0f;
        }

        if (IsKeyPressed(KEY_C)) {
            cameraOption = (cameraOption + 1) % cameraUpdatersLength;
        }
        cameraUpdaters[cameraOption](delta, &camera, &player, envItems, envItemsLength, screenWidth, screenHeight);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode2D(camera);
               renderWorld(&player, envItems, envItemsLength); 
            EndMode2D();

            DrawText("Controls:", 20, 20, 10, BLACK);
            DrawText("- Right/Left to move", 40, 40, 10, DARKGRAY);
            DrawText("- Space to jump", 40, 60, 10, DARKGRAY);
            DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom", 40, 80, 10, DARKGRAY);
            DrawText("- C to change camera mode", 40, 100, 10, DARKGRAY);
            DrawText("Current camera mode:", 20, 120, 10, BLACK);
            DrawText(cameraDescriptions[cameraOption], 40, 140, 10, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context

    return 0;
}
