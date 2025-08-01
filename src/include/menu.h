#ifndef MENU_H
#define MENU_H

typedef enum GameScreen{
  MENU,
  GAMEPLAY,
  EXIT
} GameScreen;

void UpdateMenu(GameScreen *screen);
void DrawMenu(void);

#endif
