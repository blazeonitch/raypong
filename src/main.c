#include "./include/menu.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define CLAMP(val, min, max)                                                   \
  ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#define SetTimeScale(scale) (timeScale = scale)
#define Lerp(a, b, t) (a + (b - a) * t)

// CONSTANTS
const int screenWidth = 800;
const int screenHeight = 540;
Color goalColor = {21, 21, 21, 255};
Vector2 scaleObject = {1.0f, 1.5f};

// GLOBAL VARIABLES
Font robotoFont;
Music music1;
Sound hitPaddleFX;

int p1Score = 0;
int p2Score = 0;
bool isGamePaused = false;
bool ballInP1Goal = false;
bool ballInP2Goal = false;

float timeScale = 1.0f;
float slowMoTarget = 1.0f;
float slowMoSpeed = 1.5f;

struct GameObject {
  char name[15];
  float speed;
  Vector2 position;
  Texture2D texture;
};

void DisplayText(Font font, int fontSize, Color fpsColor, Vector2 fpsPosition) {
  char fpsText[16];
  snprintf(fpsText, sizeof(fpsText), "FPS:%d", GetFPS());
  DrawTextEx(font, fpsText, fpsPosition, fontSize, 10, fpsColor);
}

void ResetBall(Vector2 *position, Vector2 *velocity, bool toLeft) {
  *position = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
  *velocity = (Vector2){(toLeft ? -600 : 600), 0};
}

void ResetGame(struct GameObject *player1, struct GameObject *player2,
               Vector2 *ballPosition, Vector2 *ballVelocity) {
  p1Score = 0;
  p2Score = 0;
  ballInP1Goal = false;
  ballInP2Goal = false;
  isGamePaused = false;

  // Reset paddle positions
  player1->position =
      (Vector2){100.0f, screenHeight / 2.0f -
                            (player1->texture.height * scaleObject.y) / 2};
  player2->position = (Vector2){
      screenWidth - 100.0f,
      screenHeight / 2.0f - (player2->texture.height * scaleObject.y) / 2};

  // Reset ball
  ResetBall(ballPosition, ballVelocity, GetRandomValue(0, 1));
}

int main(void) {
  SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_ALWAYS_RUN);
  InitWindow(screenWidth, screenHeight, "Poong");
  InitAudioDevice();
  SetExitKey(KEY_NULL);

  GameScreen currentScreen = MENU;

  robotoFont = LoadFont("resources/font/roboto.ttf");
  music1 = LoadMusicStream("resources/sfx/music/music1.ogg");
  hitPaddleFX = LoadSound("resources/sfx/hitPaddleFX.wav");
  if (WindowShouldClose()) {
    TraceLog(LOG_ERROR, "Window initialization failed!");
    return -1;
  }

  SetMusicVolume(music1, 0.5f);
  PlayMusicStream(music1);

  // -- Player Setup --
  struct GameObject player1 = {
      .name = "Blaise", .speed = 500, .position = {100.0f, 100.0f}};
  struct GameObject player2 = {.name = "Dick_Head",
                               .speed = 700,
                               .position = {screenWidth - 100.0f, 100.0f}};

  Image player1Image = LoadImage("resources/textures/player_concept.png");
  Image player2Image = LoadImage("resources/textures/player_concept.png");

  player1.texture = LoadTextureFromImage(player1Image);
  player2.texture = LoadTextureFromImage(player2Image);

  UnloadImage(player1Image);
  UnloadImage(player2Image);
  float player1TargetY = player1.position.y;
  float player2TargetY = player2.position.y;

  // -- Ball Setup --
  Vector2 ballPosition = {screenWidth / 2.0f, screenHeight / 2.0f};
  Vector2 ballVelocity = {600, 0};
  float ballRadius = 5;

  Rectangle barrier = {screenWidth / 2.0f - 5, screenHeight / 2.0f - 100, 10,
                       200};
  Rectangle p1Goal = {player1.position.x - 100, screenHeight / 2.0f - 300, 5,
                      1000};
  Rectangle p2Goal = {player2.position.x + 95, screenHeight / 2.0f - 300, 5,
                      1000};

  SetTargetFPS(120);

  while (!WindowShouldClose() && currentScreen != EXIT) {
    // Get unscaled delta time (seconds since last frame)
    float realDelta = GetFrameTime();

    GameScreen lastScreen = currentScreen;

    // Smoothly interpolate timeScale toward slowMoTarget for smooth slow-mo
    // effects
    if (fabs(timeScale - slowMoTarget) > 0.001f) {
      float step = slowMoSpeed * realDelta;

      if (timeScale < slowMoTarget)
        timeScale = CLAMP(timeScale + step, 0.0f, slowMoTarget);
      else
        timeScale = CLAMP(timeScale - step, slowMoTarget, 1.0f);
    }

    // Scaled delta time used for all game movement and logic to apply slow-mo
    // effect
    float deltaTime = realDelta * timeScale;

    // Pause toggle
    if (IsKeyPressed(KEY_P)) {
      isGamePaused = !isGamePaused;
    }

    // Update music stream if not paused
    if (!isGamePaused) {

      UpdateMusicStream(music1);

      // Paddle movement input with scaled deltaTime
      if (IsKeyDown(KEY_W))
        player1TargetY -= player1.speed * deltaTime;
      if (IsKeyDown(KEY_S))
        player1TargetY += player1.speed * deltaTime;
      if (IsKeyDown(KEY_UP))
        player2TargetY -= player2.speed * deltaTime;
      if (IsKeyDown(KEY_DOWN))
        player2TargetY += player2.speed * deltaTime;

      // Clamp paddle movement to screen bounds, scaled by texture height and
      // scale
      float scaledHeight = player1.texture.height * scaleObject.y;

      player1TargetY = CLAMP(player1TargetY, 0, screenHeight - scaledHeight);
      player2TargetY = CLAMP(player2TargetY, 0, screenHeight - scaledHeight);

      // Smooth movement using LERP
      player1.position.y =
          Lerp(player1.position.y, player1TargetY, 10.0f * deltaTime);
      player2.position.y =
          Lerp(player2.position.y, player2TargetY, 10.0f * deltaTime);

      // Ball movement with scaled deltaTime
      ballPosition.x += ballVelocity.x * deltaTime;
      ballPosition.y += ballVelocity.y * deltaTime;

      // Ball collision with top and bottom walls - invert y velocity
      if ((ballPosition.y - ballRadius <= 0 && ballVelocity.y < 0) ||
          (ballPosition.y + ballRadius >= screenHeight && ballVelocity.y > 0)) {
        ballVelocity.y *= -1;
      }

      // Ball collision with left and right walls - invert x velocity
      if ((ballPosition.x - ballRadius <= 0 && ballVelocity.x < 0) ||
          (ballPosition.x + ballRadius >= screenWidth && ballVelocity.x > 0)) {
        ballVelocity.x *= -1;
      }

      // Scoring logic with slow motion trigger
      if (CheckCollisionCircleRec(ballPosition, ballRadius, p1Goal) &&
          currentScreen == GAMEPLAY) {
        if (!ballInP1Goal) {
          PlaySound(hitPaddleFX);
          p2Score++;
          ballInP1Goal = true;
        }
      } else {
        ballInP1Goal = false;
      }

      if (CheckCollisionCircleRec(ballPosition, ballRadius, p2Goal) &&
          currentScreen == GAMEPLAY) {
        if (!ballInP2Goal) {
          PlaySound(hitPaddleFX);
          p1Score++;
          ballInP2Goal = true;
        }
      } else {
        ballInP2Goal = false;
      }

      // Paddle collision rectangles for ball bounce
      Rectangle player1Rect = {player1.position.x, player1.position.y,
                               player1.texture.width * scaleObject.x,
                               player1.texture.height * scaleObject.y};
      Rectangle player2Rect = {player2.position.x, player2.position.y,
                               player2.texture.width * scaleObject.x,
                               player2.texture.height * scaleObject.y};

      // Ball collision with paddles - invert x velocity and tweak y velocity
      // for bounce angle
      if (CheckCollisionCircleRec(ballPosition, ballRadius, player1Rect) &&
          ballVelocity.x < 0) {
        PlaySound(hitPaddleFX);
        ballVelocity.x *= -1;

        float dy = (ballPosition.y - (player1Rect.y + player1Rect.height / 2)) /
                   (player1Rect.height / 2);
        ballVelocity.y = dy * 250;

        ballPosition.x = player1Rect.x + player1Rect.width + ballRadius;
      }

      if (CheckCollisionCircleRec(ballPosition, ballRadius, player2Rect) &&
          ballVelocity.x > 0) {
        PlaySound(hitPaddleFX);
        ballVelocity.x *= -1;

        float dy = (ballPosition.y - (player2Rect.y + player2Rect.height / 2)) /
                   (player2Rect.height / 2);
        ballVelocity.y = dy * 250;

        ballPosition.x = player2Rect.x - ballRadius;
      }
    }

    // -- DRAWING --

    BeginDrawing();
    ClearBackground(BLACK);
    // Draw FPS using your font helper
    DisplayText(robotoFont, 20, RAYWHITE, (Vector2){10, 10});

    switch (currentScreen) {
    case MENU:
      UpdateMenu(&currentScreen);
      DrawMenu();
      if (currentScreen == GAMEPLAY && lastScreen == MENU) {
        ResetGame(&player1, &player2, &ballPosition, &ballVelocity);
      }
      break;

    case GAMEPLAY:

      // Draw static game elements
      DrawRectangleRec(barrier, DARKGRAY);
      DrawRectangleRec(p1Goal, goalColor);
      DrawRectangleRec(p2Goal, goalColor);

      // Draw ball
      DrawCircleV(ballPosition, ballRadius, WHITE);

      // Draw scores
      DrawText(TextFormat("%d", p1Score), 300, 20, 20, WHITE);
      DrawText(" - ", 365, 20, 50, WHITE);
      DrawText(TextFormat("%d", p2Score), 500, 20, 20, WHITE);

      // Draw player labels above paddles
      // DrawText("Player: 1", player1.position.x, player1.position.y - 30, 20,
      // RED); DrawText("Player: 2", player2.position.x, player2.position.y -
      // 30, 20, RED);

      // Draw paddles with scaling
      DrawTextureEx(player1.texture, player1.position, 0.0f, scaleObject.y,
                    WHITE);
      DrawTextureEx(player2.texture, player2.position, 0.0f, scaleObject.y,
                    WHITE);

      // Draw pause text if paused
      if (isGamePaused && currentScreen == GAMEPLAY) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));
        DrawText("PAUSED", screenWidth / 2 - 100, screenHeight / 2 - 50, 50,
                 GREEN);
        DrawText("Press [P] to Resume", screenWidth / 2 - 120,
                 screenHeight / 2 + 20, 20, GRAY);
        DrawText("Press [Q] to Exit to Start Menu", screenWidth / 2 - 120,
                 screenHeight / 2 + 60, 20, GRAY);
        if (IsKeyPressed(KEY_Q)) {
          currentScreen = MENU;
        }
        SetMusicVolume(music1, 0.1f);
      } else {
        SetMusicVolume(music1, 0.5f);
      }
      break;

    case EXIT:
      break;
    }
    EndDrawing();
  }

  // -- CLEANUP --
  UnloadMusicStream(music1);
  UnloadTexture(player1.texture);
  UnloadTexture(player2.texture);
  UnloadFont(robotoFont);
  CloseAudioDevice();
  CloseWindow();
  return 0;
}
