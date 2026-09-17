#pragma once
#include <raylib.h>
#include <string>

constexpr int MAX_MAP_WIDTH = 32;
constexpr int MAX_MAP_HEIGHT = 32;

enum GameState { STATE_EXPLORATION, STATE_VICTORY, STATE_MENU, STATE_COMBAT };

enum ButtonState { BTN_NORMAL = 0, BTN_HOVER = 1, BTN_CLICKED = 2 };

enum AttackState { ATTACK, BLOCK, PARRY };
struct Equipment {
  int sword = 10;
  int dagger = 5;
  int warhamer = 16;
};

struct MenuButton {
  Rectangle destRec;
  ButtonState currentState;
  const char *label;
};

struct Player {
  float x;
  float y;
  float startX;
  float startY;
  float angle;

  int hitPoint = 10;
  Equipment equipment;

  void Update(const int map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]);
};

struct Monster {
  int healtBar = 20;
  int damage = 2;
};

class Game {
public:
  Game();
  ~Game();

  void Update();
  void Draw();
  bool LoadMap(const std::string &filepath);

  bool isRunning;

private:
  // Oyun ve Oyuncu Durumu
  GameState state;
  Player player;
  Monster monster;

  // Harita Verisi
  int map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
  int mapWidth;
  int mapHeight;

  // Aydınlatma ve Sis (Atmosfer)
  RenderTexture2D m_backgroundTexture;
  bool m_hasTorch = false;
  float m_fogK = 0.18f;
  Color m_currentFogColor;

  // UI Butonları
  MenuButton startBtn;
  MenuButton exitBtn;

  // Dokular
  Texture2D wallTexture;
  Texture2D splashArt;
  Texture2D enterButton;
  Texture2D exitButton;
  Texture2D logo;

  // Yardımcı Metotlar
  Texture2D LoadTextureWithFallback(const char *filepath);
  // Exploring
  void UpdateExploration();
  void DrawExploration();
  // Menu ve Buttonlar
  void UpdateMenu();
  void DrawMenu();
  void DrawButton(Texture2D sheet, MenuButton btn);
  // Combat
  void UpdateCombat();
  void DrawCombat();

  void CastRays();
  void UpdateBackground();
  Color CalculateLighting(float dist, int side);
};