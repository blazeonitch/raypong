#include "include/menu.h"
#include "raylib.h"

void UpdateMenu(GameScreen *screen) {
  Vector2 mouse = GetMousePosition();
  Rectangle startBtn = {GetScreenWidth() / 2.0f - 80, 250, 160, 40};
  Rectangle quitBtn = {GetScreenWidth() / 2.0f - 80, 310, 160, 40};

  if (CheckCollisionPointRec(mouse, startBtn) &&
      IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    *screen = GAMEPLAY;
  }
  if (CheckCollisionPointRec(mouse, quitBtn) &&
      IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    *screen = EXIT;
  }

  if (IsKeyPressed(KEY_ENTER)) {
    *screen = GAMEPLAY;
  }
  if (IsKeyPressed(KEY_Q)) {
    *screen = EXIT;
  }
}

void DrawMenu(void) {
  Vector2 mouse = GetMousePosition();
  int width = GetScreenWidth();

  // Title
  DrawText("Ray Pong", width / 2 - MeasureText("cPOONG", 100) / 2, 100, 100,
           DARKBLUE);

  DrawText("by gamesbyblaze", width / 2 - MeasureText("cPOONG", 100) / 2 + 290,
           200, 25, DARKBLUE);

  // Buttons
  Rectangle startBtn = {width / 2.0f - 80, 250, 160, 40};
  Rectangle quitBtn = {width / 2.0f - 80, 310, 160, 40};

  Color hoverColor = (Color){100, 100, 255, 255};
  Color normalColor = (Color){200, 200, 255, 255};

  DrawRectangleRec(startBtn, CheckCollisionPointRec(mouse, startBtn)
                                 ? hoverColor
                                 : normalColor);
  DrawText("Start Game", startBtn.x + 20, startBtn.y + 10, 20, BLACK);

  DrawRectangleRec(quitBtn, CheckCollisionPointRec(mouse, quitBtn)
                                ? hoverColor
                                : normalColor);
  DrawText("Quit", quitBtn.x + 55, quitBtn.y + 10, 20, BLACK);
}
