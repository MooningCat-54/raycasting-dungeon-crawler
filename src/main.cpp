#include "game.h"
#include "raylib.h"

int main() {
  const int screenWidth = 800;
  const int screenHeight = 600;

  InitWindow(screenWidth, screenHeight, "Dungeon Crawler Engine");
  SetTargetFPS(60);

  Game game;
  if (!game.LoadMap("assets/maps/level.txt")) {
    // Dosya bulunamazsa pencereyi kapatıp çık
    CloseWindow();
    return 1;
  }

  // Ana Oyun Döngüsü
  while (!WindowShouldClose() && game.isRunning) {
    // 1. Mantığı Güncelle
    game.Update();

    // 2. Çizimi Yap
    BeginDrawing();
    game.Draw();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}