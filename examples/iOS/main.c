#include "raylib.h"
#include "raymath.h"

#define BRICK_ROWS 5
#define BRICK_COLS 7
#define BRICK_PADDING 6.0f
#define BALL_SPEED_INIT 500.0f
#define BALL_SPEED_MAX 900.0f

typedef enum {
  STATE_TITLE,
  STATE_PLAYING,
  STATE_PAUSED,
  STATE_WIN,
  STATE_LOSE,
} GameState;

typedef struct {
  Rectangle rect;
  Color color;
  bool alive;
  int points;
} Brick;

typedef struct {
  Color background;
  Color panel;
  Color panelBorder;
  Color accent;
  Color accentBorder;
  Color text;
  Color muted;
  Color shadow;

  float margin;
  float topBarHeight;
  float buttonWidth;
  float buttonHeight;
  float actionWidth;
  float actionHeight;
  float roundness;
} UITheme;

static UITheme DefaultUITheme(void) {
  return (UITheme){
      .background = {14, 16, 22, 255},
      .panel = {245, 245, 245, 255},
      .panelBorder = {245, 245, 245, 255},
      .accent = {60, 180, 255, 255},
      .accentBorder = {245, 245, 245, 255},
      .text = {20, 20, 24, 255},
      .muted = {160, 170, 190, 255},
      .shadow = {0, 0, 0, 160},

      .margin = 12.0f,
      .topBarHeight = 34.0f,
      .buttonWidth = 104.0f,
      .buttonHeight = 34.0f,
      .actionWidth = 168.0f,
      .actionHeight = 36.0f,
      .roundness = 0.0f,
  };
}

static Color BrickColor(int row) {
  Color palette[] = {
      {255, 80, 80, 255}, {255, 160, 40, 255}, {255, 220, 40, 255},
      {80, 210, 90, 255}, {60, 180, 255, 255},
  };
  return palette[row % 5];
}

static bool CollideBallRect(Vector2 *pos, Vector2 *vel, float radius,
                            Rectangle rect, float dt) {
  Rectangle expanded = {
      rect.x - radius,
      rect.y - radius,
      rect.width + radius * 2.0f,
      rect.height + radius * 2.0f,
  };

  if (!CheckCollisionPointRec(*pos, expanded))
    return false;

  float nearX = Clamp(pos->x, rect.x, rect.x + rect.width);
  float nearY = Clamp(pos->y, rect.y, rect.y + rect.height);
  float dx = pos->x - nearX;
  float dy = pos->y - nearY;

  if (fabsf(dx) > fabsf(dy))
    vel->x = -vel->x;
  else
    vel->y = -vel->y;

  pos->x += vel->x * dt;
  pos->y += vel->y * dt;

  return true;
}

static void BuildBricks(Brick bricks[BRICK_ROWS][BRICK_COLS], float sw,
                        float brickAreaTop, float brickAreaH) {
  float brickW = (sw - BRICK_PADDING * (BRICK_COLS + 1)) / BRICK_COLS;
  float brickH = (brickAreaH - BRICK_PADDING * (BRICK_ROWS + 1)) / BRICK_ROWS;

  for (int r = 0; r < BRICK_ROWS; r++) {
    for (int c = 0; c < BRICK_COLS; c++) {
      float bx = BRICK_PADDING + c * (brickW + BRICK_PADDING);
      float by = brickAreaTop + BRICK_PADDING + r * (brickH + BRICK_PADDING);
      bricks[r][c] = (Brick){
          .rect = {bx, by, brickW, brickH},
          .color = BrickColor(r),
          .alive = true,
          .points = (BRICK_ROWS - r) * 10,
      };
    }
  }
}

int main(void) {
  InitWindow(0, 0, "Breakout - raylib iOS");
  SetTargetFPS(60);

  UITheme ui = DefaultUITheme();

  const char *logoPath =
      TextFormat("%sraylib_180x180.png", GetApplicationDirectory());
  Texture2D logo = LoadTexture(logoPath);

  int sw = GetScreenWidth();
  int sh = GetScreenHeight();

  float paddleW = sw * 0.22f;
  float paddleH = sh * 0.022f;
  float paddleY = sh - sh * 0.10f;
  float ballRadius = sw * 0.022f;
  float brickAreaTop = sh * 0.22f;
  float brickAreaH = sh * 0.32f;

  GameState state = STATE_TITLE;
  int score = 0;
  int lives = 3;
  int totalBricks = BRICK_ROWS * BRICK_COLS;
  int aliveBricks = totalBricks;

  Rectangle paddle = {sw * 0.5f - paddleW * 0.5f, paddleY, paddleW, paddleH};
  Vector2 ball = {sw * 0.5f, paddleY - ballRadius - 2.0f};
  Vector2 ballV = {BALL_SPEED_INIT * 0.6f, -BALL_SPEED_INIT};
  bool ballLaunched = false;

  Brick bricks[BRICK_ROWS][BRICK_COLS];
  BuildBricks(bricks, (float)sw, brickAreaTop, brickAreaH);

#define RESET_BALL()                                                           \
  do {                                                                         \
    ball = (Vector2){paddle.x + paddle.width * 0.5f,                           \
                     paddleY - ballRadius - 2.0f};                             \
    ballV = (Vector2){BALL_SPEED_INIT * 0.6f, -BALL_SPEED_INIT};               \
    ballLaunched = false;                                                      \
  } while (0)

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    if (dt > 0.05f)
      dt = 0.05f;

    if (GetScreenWidth() != sw || GetScreenHeight() != sh) {
      sw = GetScreenWidth();
      sh = GetScreenHeight();

      paddleW = sw * 0.22f;
      paddleH = sh * 0.022f;
      paddleY = sh - sh * 0.10f;
      ballRadius = sw * 0.022f;
      brickAreaTop = sh * 0.22f;
      brickAreaH = sh * 0.32f;

      paddle.width = paddleW;
      paddle.height = paddleH;
      paddle.y = paddleY;

      BuildBricks(bricks, (float)sw, brickAreaTop, brickAreaH);
    }

    Vector2 pointer =
        GetTouchPointCount() > 0 ? GetTouchPosition(0) : GetMousePosition();

    bool tapped = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    Rectangle pauseRect = {sw - ui.margin - ui.buttonWidth, ui.margin,
                           ui.buttonWidth, ui.buttonHeight};
    Rectangle buttonRect = {sw * 0.5f - ui.actionWidth * 0.5f,
                            sh - ui.margin - ui.actionHeight, ui.actionWidth,
                            ui.actionHeight};
    Rectangle menuRect = {sw * 0.5f - ui.actionWidth * 0.5f,
                          sh - ui.margin - ui.actionHeight * 2.0f - 8.0f,
                          ui.actionWidth, ui.actionHeight};
    Rectangle launchRect = {ui.margin, sh * 0.44f, sw - ui.margin * 2.0f,
                            sh - sh * 0.44f - ui.margin};

    bool pausePressed = tapped && CheckCollisionPointRec(pointer, pauseRect) &&
                        (state == STATE_PLAYING || state == STATE_PAUSED);
    bool buttonPressed = tapped && CheckCollisionPointRec(pointer, buttonRect);
    bool menuPressed = tapped && CheckCollisionPointRec(pointer, menuRect);
    bool launchPressed = tapped && CheckCollisionPointRec(pointer, launchRect);

    if (state == STATE_TITLE) {

      if (buttonPressed)
        state = STATE_PLAYING;

    } else if (state == STATE_PAUSED) {

      if (pausePressed)
        state = STATE_PLAYING;

    } else if (state == STATE_WIN || state == STATE_LOSE) {

      if (buttonPressed) {
        score = 0;
        lives = 3;
        aliveBricks = totalBricks;
        paddle.x = sw * 0.5f - paddleW * 0.5f;
        BuildBricks(bricks, (float)sw, brickAreaTop, brickAreaH);
        RESET_BALL();
        state = STATE_PLAYING;

      } else if (menuPressed) {
        score = 0;
        lives = 3;
        aliveBricks = totalBricks;
        paddle.x = sw * 0.5f - paddleW * 0.5f;
        BuildBricks(bricks, (float)sw, brickAreaTop, brickAreaH);
        RESET_BALL();
        state = STATE_TITLE;
      }

    } else {

      paddle.x = Clamp(pointer.x - paddle.width * 0.5f, 0.0f,
                       (float)sw - paddle.width);

      if (pausePressed) {
        state = STATE_PAUSED;

      } else if (launchPressed && !ballLaunched) {
        ballLaunched = true;
      }

      if (ballLaunched) {
        ball.x += ballV.x * dt;
        ball.y += ballV.y * dt;

        if (ball.x - ballRadius <= 0.0f) {
          ball.x = ballRadius;
          ballV.x = fabsf(ballV.x);
        }
        if (ball.x + ballRadius >= sw) {
          ball.x = sw - ballRadius;
          ballV.x = -fabsf(ballV.x);
        }
        if (ball.y - ballRadius <= 0.0f) {
          ball.y = ballRadius;
          ballV.y = fabsf(ballV.y);
        }

        if (ball.y - ballRadius > sh) {
          lives--;
          if (lives <= 0)
            state = STATE_LOSE;
          else
            RESET_BALL();
        }

        if (CollideBallRect(&ball, &ballV, ballRadius, paddle, dt)) {
          float hitPos = (ball.x - paddle.x) / paddle.width;
          float angle = (hitPos - 0.5f) * 120.0f * DEG2RAD;
          float speed = Vector2Length(ballV);
          speed = Clamp(speed + 10.0f, 0.0f, BALL_SPEED_MAX);
          ballV.x = sinf(angle) * speed;
          ballV.y = -fabsf(cosf(angle) * speed);
        }

        for (int r = 0; r < BRICK_ROWS; r++) {
          for (int c = 0; c < BRICK_COLS; c++) {
            Brick *b = &bricks[r][c];
            if (!b->alive)
              continue;

            if (CollideBallRect(&ball, &ballV, ballRadius, b->rect, dt)) {
              b->alive = false;
              aliveBricks--;
              score += b->points;
              if (aliveBricks <= 0)
                state = STATE_WIN;
              goto done_bricks;
            }
          }
        }
      done_bricks:;

      } else {
        ball.x = paddle.x + paddle.width * 0.5f;
        ball.y = paddleY - ballRadius - 2.0f;
      }
    }

    BeginDrawing();
    ClearBackground(ui.background);

    if (state == STATE_TITLE) {
      if (logo.id != 0) {
        float targetSize = sh * 0.12f;
        float scale = targetSize / (float)logo.width;
        DrawTextureEx(logo, (Vector2){sw * 0.5f - targetSize * 0.5f, 8.0f},
                      0.0f, scale, WHITE);
      }

      int ts = sw / 10;
      int ss = sw / 22;
      DrawText("BREAKOUT", sw / 2 - MeasureText("BREAKOUT", ts) / 2,
               (int)(sh * 0.18f), ts, (Color){60, 180, 255, 255});
      DrawText("tap the button to start",
               sw / 2 - MeasureText("tap the button to start", ss) / 2,
               (int)(sh * 0.18f) + ts + 8, ss, (Color){160, 170, 190, 255});
    }

    if (state != STATE_TITLE) {
      for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
          Brick *b = &bricks[r][c];
          if (!b->alive)
            continue;

          DrawRectangleRounded(b->rect, 0.25f, 6, b->color);
          DrawLineEx((Vector2){b->rect.x + 4, b->rect.y + 2},
                     (Vector2){b->rect.x + b->rect.width - 4, b->rect.y + 2},
                     1.5f, (Color){255, 255, 255, 60});
        }
      }

      DrawRectangleRounded(paddle, 0.5f, 8, (Color){60, 180, 255, 255});
      DrawCircleV(ball, ballRadius, WHITE);
      DrawCircleLines((int)ball.x, (int)ball.y, ballRadius + 2,
                      (Color){255, 255, 255, 60});

      int hs = sw / 24;
      DrawText(TextFormat("Score: %d", score), 14, 14, hs,
               (Color){200, 210, 225, 255});
      DrawText(TextFormat("Lives: %d", lives), 14, 14 + hs + 8, hs,
               (Color){255, 80, 80, 255});

      if (!ballLaunched && state == STATE_PLAYING) {
        const char *hint = "tap anywhere to launch";
        int hintSize = sw / 26;
        DrawText(hint, sw / 2 - MeasureText(hint, hintSize) / 2,
                 (int)(sh * 0.72f), hintSize, ui.muted);
      }

      if (state == STATE_PAUSED) {
        DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 150});

        int ts = sw / 10;
        int ss = sw / 22;
        DrawText("PAUSED", sw / 2 - MeasureText("PAUSED", ts) / 2, sh / 2 - ts,
                 ts, RAYWHITE);
        DrawText("tap resume to continue",
                 sw / 2 - MeasureText("tap resume to continue", ss) / 2,
                 sh / 2 + ts, ss, (Color){160, 170, 190, 180});
      }

      if (state == STATE_WIN || state == STATE_LOSE) {
        DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 160});

        const char *title = state == STATE_WIN ? "YOU WIN!" : "GAME OVER";
        Color tcol = state == STATE_WIN ? (Color){80, 220, 100, 255}
                                        : (Color){255, 80, 80, 255};
        int ts = sw / 9;
        int ss = sw / 22;

        DrawText(title, sw / 2 - MeasureText(title, ts) / 2, sh / 2 - ts, ts,
                 tcol);
        DrawText(TextFormat("Score: %d", score),
                 sw / 2 - MeasureText(TextFormat("Score: %d", score), ss) / 2,
                 sh / 2 + ts / 4, ss, (Color){200, 210, 225, 255});

        const char *msg = state == STATE_WIN ? "tap retry to play again"
                                             : "tap retry to try again";
        DrawText(msg, sw / 2 - MeasureText(msg, ss) / 2, sh / 2 + ts, ss,
                 (Color){160, 170, 190, 200});
      }
    }

    if (state == STATE_PLAYING || state == STATE_PAUSED) {
      const char *pauseText = state == STATE_PAUSED ? "RESUME" : "PAUSE";
      DrawRectangleRounded(pauseRect, ui.roundness, 8, ui.panel);
      DrawText(pauseText,
               (int)(pauseRect.x + pauseRect.width * 0.5f -
                     (float)MeasureText(pauseText, 16) * 0.5f),
               (int)(pauseRect.y + 8), 16, ui.text);
    }

    bool drawActionButton =
        (state == STATE_TITLE) || (state == STATE_WIN || state == STATE_LOSE);
    if (drawActionButton) {
      const char *buttonText =
          (state == STATE_WIN || state == STATE_LOSE) ? "RETRY" : "START";
      Rectangle actionButton = {sw * 0.5f - ui.actionWidth * 0.5f,
                                sh - ui.margin - ui.actionHeight,
                                ui.actionWidth, ui.actionHeight};
      DrawRectangleRounded(actionButton, ui.roundness, 8, ui.panel);
      DrawText(buttonText,
               (int)(actionButton.x + actionButton.width * 0.5f -
                     (float)MeasureText(buttonText, 18) * 0.5f),
               (int)(actionButton.y + 8), 18, ui.text);
    }

    if (state == STATE_WIN || state == STATE_LOSE) {
      const char *menuText = "MAIN MENU";
      DrawRectangleRounded(menuRect, ui.roundness, 8, ui.panel);
      DrawText(menuText,
               (int)(menuRect.x + menuRect.width * 0.5f -
                     (float)MeasureText(menuText, 18) * 0.5f),
               (int)(menuRect.y + 8), 18, ui.text);
    }

    EndDrawing();
  }

  UnloadTexture(logo);
  CloseWindow();
  return 0;
}