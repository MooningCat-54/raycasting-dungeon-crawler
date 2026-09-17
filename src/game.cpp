#include "game.h"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <raylib.h>

Game::Game() {
  state = STATE_MENU;
  isRunning = true;
  mapWidth = 0;
  mapHeight = 0;
  m_hasTorch = false;
  m_fogK = 0.15f;
  m_currentFogColor = {10, 10, 15, 255};

  for (int y = 0; y < MAX_MAP_HEIGHT; y++) {
    for (int x = 0; x < MAX_MAP_WIDTH; x++) {
      map[y][x] = 1;
    }
  }

  // Başlangıç noktası
  player.x = 1.5f;
  player.y = 1.5f;
  player.angle = 0.0f;

  // Buton konumlandırma
  int scale = 1;
  float w = 96.0f * scale;
  float h = 32.0f * scale;

  float midX = GetScreenWidth() / 2.0f;
  float midY = GetScreenHeight() / 2.0f;

  startBtn.destRec = {midX - (w * 1.5f / 2.0f), midY - 50.0f, w * 1.5f,
                      h * 1.5f};
  startBtn.currentState = BTN_NORMAL;

  exitBtn.destRec = {midX - (w / 2.0f), midY + 20.0f, w, h};
  exitBtn.currentState = BTN_NORMAL;

  // Asset yüklemeleri
  wallTexture = LoadTextureWithFallback("assets/texture/wall.png");
  splashArt = LoadTextureWithFallback("assets/texture/splashArt.png");
  exitButton = LoadTextureWithFallback("assets/texture/exitButton.png");
  enterButton = LoadTextureWithFallback("assets/texture/enterButton.png");
  logo = LoadTextureWithFallback("assets/texture/maze.png");

  // Arka plan dokusu ve ilk fırınlama (baking)
  m_backgroundTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
  UpdateBackground();
}

Game::~Game() {
  UnloadTexture(wallTexture);
  UnloadTexture(splashArt);
  UnloadTexture(enterButton);
  UnloadTexture(exitButton);
  UnloadRenderTexture(m_backgroundTexture);
  UnloadTexture(logo);
}

bool Game::LoadMap(const std::string &filepath) {
  std::ifstream file(filepath);
  if (!file.is_open()) {
    std::cerr << "HATA: Harita dosyasi acilmadi -> " << filepath << std::endl;
    return false;
  }

  int w, h;
  file >> w >> h;

  if (w <= 0 || w > MAX_MAP_WIDTH || h <= 0 || h > MAX_MAP_HEIGHT) {
    std::cerr << "HATA: Gecersiz harita boyutlari -> " << w << "x" << h
              << std::endl;
    return false;
  }

  mapWidth = w;
  mapHeight = h;

  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      char tile;
      file >> tile;

      if (tile == 'P') {
        player.startX = x + 0.5f;
        player.startY = y + 0.5f;
        map[y][x] = 0;
      } else if (tile == 'E') {
        map[y][x] = 2;
      } else if (tile == 'M') {
        map[y][x] = 3;
      } else {
        map[y][x] = tile - '0';
      }
    }
  }

  player.x = player.startX;
  player.y = player.startY;

  file.close();
  return true;
}

Texture2D Game::LoadTextureWithFallback(const char *filepath) {
  Texture2D tex = LoadTexture(filepath);

  if (tex.id == 0) {
    Image fallbackImg = GenImageColor(64, 64, MAGENTA);
    tex = LoadTextureFromImage(fallbackImg);
    UnloadImage(fallbackImg);
  }

  SetTextureFilter(tex, TEXTURE_FILTER_POINT);
  return tex;
}

void Game::Update() {
  if (state == STATE_MENU) {
    UpdateMenu();
  } else if (state == STATE_EXPLORATION) {
    if (IsKeyPressed(KEY_T)) {
      m_hasTorch = !m_hasTorch;
      UpdateBackground();
    }
    UpdateExploration();
  } else if (state == STATE_VICTORY) {
    Rectangle artSource = {0, 0, (float)splashArt.width,
                           (float)splashArt.height};
    Rectangle artDest = {0, 0, (float)GetScreenWidth(),
                         (float)GetScreenHeight()};

    DrawTexturePro(splashArt, artSource, artDest, Vector2{0, 0}, 0.0f, WHITE);

    DrawText("TEBRIKLER! CIKISI BULDUN.", GetScreenWidth() / 2 - 160,
             GetScreenHeight() / 2 - 20, 24, GOLD);
    DrawText("Yeniden baslamak icin ENTER'a bas", GetScreenWidth() / 2 - 150,
             GetScreenHeight() / 2 + 20, 18, RAYWHITE);
    if (IsKeyPressed(KEY_ENTER)) {
      player.x = player.startX;
      player.y = player.startY;
      player.hitPoint = 10;

      state = STATE_EXPLORATION;
    }
  } else if (state == STATE_COMBAT) {
    UpdateCombat();
  }
}

void Player::Update(const int map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
  float dt = GetFrameTime();
  float moveSpeed = 3.0f;
  float radius = 0.2f;

  // 1. Fare ile bakış açısı
  float mouseSensitivity = 0.0035f;
  Vector2 mouseDelta = GetMouseDelta();
  angle += mouseDelta.x * mouseSensitivity;

  if (angle < 0.0f)
    angle += 2.0f * PI;
  if (angle >= 2.0f * PI)
    angle -= 2.0f * PI;

  if (IsKeyPressed(KEY_H)) {
    if (IsCursorHidden())
      EnableCursor();
    else
      DisableCursor();
  }

  // 2. Hareket vektörleri
  float dirX = std::cos(angle);
  float dirY = std::sin(angle);
  float dx = 0.0f;
  float dy = 0.0f;

  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
    dx += dirX;
    dy += dirY;
  }
  if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
    dx -= dirX;
    dy -= dirY;
  }
  if (IsKeyDown(KEY_D)) {
    dx += -dirY;
    dy += dirX;
  }
  if (IsKeyDown(KEY_A)) {
    dx += dirY;
    dy += -dirX;
  }

  float length = std::sqrt(dx * dx + dy * dy);
  if (length > 0.0f) {
    dx = (dx / length) * moveSpeed * dt;
    dy = (dy / length) * moveSpeed * dt;
  }

  // 3. Duvar çarpışma kontrolü
  float checkX = (dx > 0) ? (x + dx + radius) : (x + dx - radius);
  int gridCheckX = (int)checkX;
  int currentY = (int)y;
  if (gridCheckX >= 0 && gridCheckX < MAX_MAP_WIDTH && currentY >= 0 &&
      currentY < MAX_MAP_HEIGHT) {
    if (map[currentY][gridCheckX] != 1) {
      x += dx;
    }
  }

  float checkY = (dy > 0) ? (y + dy + radius) : (y + dy - radius);
  int gridCheckY = (int)checkY;
  int currentX = (int)x;
  if (gridCheckY >= 0 && gridCheckY < MAX_MAP_HEIGHT && currentX >= 0 &&
      currentX < MAX_MAP_WIDTH) {
    if (map[gridCheckY][currentX] != 1) {
      y += dy;
    }
  }
}

void Game::UpdateMenu() {
  Vector2 mousePos = GetMousePosition();

  if (IsCursorHidden())
    EnableCursor();

  if (CheckCollisionPointRec(mousePos, startBtn.destRec)) {
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      startBtn.currentState = BTN_CLICKED;
    } else {
      startBtn.currentState = BTN_HOVER;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      state = STATE_EXPLORATION;
      return;
    }
  } else {
    startBtn.currentState = BTN_NORMAL;
  }

  if (IsKeyPressed(KEY_ENTER)) {
    state = STATE_EXPLORATION;
  }

  if (CheckCollisionPointRec(mousePos, exitBtn.destRec)) {
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      isRunning = false;
    } else {
      exitBtn.currentState = BTN_HOVER;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      isRunning = false;
      return;
    }
  } else {
    exitBtn.currentState = BTN_NORMAL;
  }
}

void Game::UpdateExploration() {
  player.Update(map);

  if (IsKeyPressed(KEY_TAB)) {
    state = STATE_MENU;
  }

  if ((int)player.y >= 0 && (int)player.y < mapHeight && (int)player.x >= 0 &&
      (int)player.x < mapWidth) {

    if (map[(int)player.y][(int)player.x] == 2) {
      state = STATE_VICTORY;
    }

    if (map[(int)player.y][(int)player.x] == 3) {
      state = STATE_COMBAT;
    }
  }
}

void Game::Draw() {
  if (state == STATE_MENU)
    DrawMenu();
  else if (state == STATE_EXPLORATION)
    DrawExploration();
  else if (state == STATE_COMBAT) {
    DrawCombat();
  }
}

void Game::DrawMenu() {
  float scale = (float)GetScreenHeight() / splashArt.height;
  float artWidth = (float)splashArt.width * scale;
  float artHeight = (float)splashArt.height * scale;

  Rectangle artSource = {0, 0, (float)splashArt.width, (float)splashArt.height};
  Rectangle artDest = {0, 0, artWidth, artHeight};

  Rectangle logoSource = {0, 0, (float)logo.width, (float)logo.height};
  Rectangle logoDest = {(float)GetScreenWidth() / 2 - 192,
                        (float)GetScreenHeight() / 2 - 200,
                        (float)logo.width * 3.0f, (float)logo.height * 2.0f};

  DrawTexturePro(splashArt, artSource, artDest, Vector2{0, 0}, 0.0f, WHITE);
  DrawTexturePro(logo, logoSource, logoDest, {0, 0}, 0.0f, WHITE);
  DrawButton(enterButton, startBtn);
  DrawButton(exitButton, exitBtn);
}

void Game::DrawButton(Texture2D sheet, MenuButton btn) {
  const float BTN_SRC_W = 96.0f;
  const float BTN_SRC_H = 32.0f;

  Rectangle sourceRec = {0.0f, (float)btn.currentState * BTN_SRC_H, BTN_SRC_W,
                         BTN_SRC_H};
  DrawTexturePro(sheet, sourceRec, btn.destRec, Vector2{0, 0}, 0.0f, WHITE);
}

void Game::DrawExploration() {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  // 1. Arka planı bas (Tavan ve Zemin)
  Rectangle src = {0.0f, 0.0f, (float)m_backgroundTexture.texture.width,
                   -(float)m_backgroundTexture.texture.height};
  Rectangle dst = {0.0f, 0.0f, (float)screenWidth, (float)screenHeight};
  DrawTexturePro(m_backgroundTexture.texture, src, dst, {0.0f, 0.0f}, 0.0f,
                 WHITE);

  // 2. Duvarları çiz
  CastRays();

  // 3. Basit UI
  DrawText("WASD / Oklar: Hareket | Fare: Bakis | T: Mesale", 10, 10, 18,
           RAYWHITE);
  DrawText("Cikisi (2) Bul!", 10, 35, 16, YELLOW);
}

void Game::CastRays() {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  // Kamera düzlemi vektörleri (FOV: ~66 derece)
  float dirX = std::cos(player.angle);
  float dirY = std::sin(player.angle);
  float planeX = -dirY * 0.66f;
  float planeY = dirX * 0.66f;

  for (int i = 0; i < screenWidth; i++) {
    // Ekran koordinatını normalize et (-1.0 ile 1.0)
    float cameraX = 2.0f * i / (float)screenWidth - 1.0f;
    float rayDirX = dirX + planeX * cameraX;
    float rayDirY = dirY + planeY * cameraX;

    int mapX = (int)player.x;
    int mapY = (int)player.y;

    float deltaDistX = (rayDirX == 0.0f) ? 1e30f : std::abs(1.0f / rayDirX);
    float deltaDistY = (rayDirY == 0.0f) ? 1e30f : std::abs(1.0f / rayDirY);

    float sideDistX;
    float sideDistY;
    int stepX;
    int stepY;

    if (rayDirX < 0.0f) {
      stepX = -1;
      sideDistX = (player.x - mapX) * deltaDistX;
    } else {
      stepX = 1;
      sideDistX = (mapX + 1.0f - player.x) * deltaDistX;
    }

    if (rayDirY < 0.0f) {
      stepY = -1;
      sideDistY = (player.y - mapY) * deltaDistY;
    } else {
      stepY = 1;
      sideDistY = (mapY + 1.0f - player.y) * deltaDistY;
    }

    // --- DDA DÖNGÜSÜ ---
    bool hit = false;
    int side = 0;

    while (!hit) {
      if (sideDistX < sideDistY) {
        sideDistX += deltaDistX;
        mapX += stepX;
        side = 0;
      } else {
        sideDistY += deltaDistY;
        mapY += stepY;
        side = 1;
      }

      if (mapX >= 0 && mapX < mapWidth && mapY >= 0 && mapY < mapHeight) {
        if (map[mapY][mapX] == 1) {
          hit = true;
        }
      } else {
        hit = true;
      }
    }

    // Dik mesafe hesabı (Fisheye engellenmiş)
    float perpWallDist;
    if (side == 0)
      perpWallDist = sideDistX - deltaDistX;
    else
      perpWallDist = sideDistY - deltaDistY;

    if (perpWallDist < 0.1f)
      perpWallDist = 0.1f;

    int lineHeight = (int)(screenHeight / perpWallDist);
    int drawStart = (screenHeight / 2) - (lineHeight / 2);
    int drawEnd = (screenHeight / 2) + (lineHeight / 2);

    int clampedStart = (drawStart < 0) ? 0 : drawStart;
    int clampedEnd = (drawEnd < 0) ? 0 : drawEnd;
    if (clampedEnd > screenHeight)
      clampedEnd = screenHeight;

    Color wallTint = CalculateLighting(perpWallDist, side);

    // --- 1. TAVAN SÜTUNU ---
    if (clampedStart > 0) {
      // Tavanı daha karanlık ve soğuk tutuyoruz (meşale yukarıyı az aydınlatır)
      Color ceilNear =
          m_hasTorch ? Color{18, 16, 12, 255} : Color{3, 3, 4, 255};

      // Duvarla birleştiği yer doğrudan zifiri siyaha erisin:
      Color ceilWallEdge = Color{0, 0, 0, 255};

      DrawRectangleGradientV(i, 0, 1, clampedStart, ceilNear, ceilWallEdge);
    }

    // --- 2. DUVAR DOKU HESABI VE ÇİZİMİ ---
    float wallX;
    if (side == 0)
      wallX = player.y + perpWallDist * rayDirY;
    else
      wallX = player.x + perpWallDist * rayDirX;
    wallX -= std::floor(wallX);

    int texX = (int)(wallX * (float)wallTexture.width);
    if (side == 0 && rayDirX < 0)
      texX = wallTexture.width - texX - 1;
    if (side == 1 && rayDirY > 0)
      texX = wallTexture.width - texX - 1;

    Rectangle sourceRec = {(float)texX, 0.0f, 1.0f, (float)wallTexture.height};
    Rectangle destRec = {(float)i, (float)drawStart, 1.0f, (float)lineHeight};
    DrawTexturePro(wallTexture, sourceRec, destRec, Vector2{0, 0}, 0.0f,
                   wallTint);

    // --- 3. ZEMİN SÜTUNU ---
    if (clampedEnd < screenHeight) {
      int floorH = screenHeight - clampedEnd;

      // Duvar dibi temas gölgesi (%25):
      Color floorWallEdge;
      floorWallEdge.r = (unsigned char)(wallTint.r * 0.28f);
      floorWallEdge.g = (unsigned char)(wallTint.g * 0.28f);
      floorWallEdge.b = (unsigned char)(wallTint.b * 0.28f);
      floorWallEdge.a = 255;

      // Ayak ucu sarısı (ekranın altı)
      Color floorFootEdge =
          m_hasTorch ? Color{160, 135, 40, 255} : Color{12, 11, 10, 255};

      DrawRectangleGradientV(i, clampedEnd, 1, floorH, floorWallEdge,
                             floorFootEdge);
    }
  }
}

void Game::UpdateBackground() {
  BeginTextureMode(m_backgroundTexture);
  ClearBackground(Color{0, 0, 0, 255}); // Tamamen siyah derinlik
  EndTextureMode();
}

Color Game::CalculateLighting(float dist, int side) {
  float minLightDist = 0.5f;
  float maxLightDist = m_hasTorch ? 6.5f : 1.8f;

  float t = (dist - minLightDist) / (maxLightDist - minLightDist);
  if (t < 0.0f)
    t = 0.0f;
  if (t > 1.0f)
    t = 1.0f;

  // 1.0'dan başlayıp maxLightDist mesafesinde tam olarak 0.0'a inen pürüzsüz
  // eğri
  float smoothDim = 1.0f - (t * t * (3.0f - 2.0f * t));

  // Duvar derinlik gölgesi
  float sideShade = (side == 1) ? 0.70f : 1.0f;

  // smoothDim sıfırlandığı an parlaklık da net 0.0 olur (asılı kalan gölge
  // kalmaz)
  float brightness = smoothDim * sideShade;

  // Işık rengi
  Color lightTint =
      m_hasTorch ? Color{255, 215, 140, 255} : Color{150, 160, 180, 255};

  Color finalTint;
  finalTint.r = (unsigned char)(lightTint.r * brightness);
  finalTint.g = (unsigned char)(lightTint.g * brightness);
  finalTint.b = (unsigned char)(lightTint.b * brightness);
  finalTint.a = 255;

  return finalTint;
}

void Game::UpdateCombat() {
  bool actionTaken = false;
  int damage = 0;

  // Rastgele hasar üretimi
  std::random_device rd;
  std::mt19937 gen(rd());
  int swordMax = (player.equipment.sword > 0) ? player.equipment.sword : 5;
  std::uniform_int_distribution<> randomInt(1, swordMax);

  // 1. Oyuncu Hamlesi (Döngü yok, tek karede tuş kontrolü)
  if (IsKeyPressed(KEY_A)) { // Saldırı
    damage = randomInt(gen);
    actionTaken = true;
  } else if (IsKeyPressed(KEY_S)) { // Blok
    damage = 0;
    actionTaken = true;
  } else if (IsKeyPressed(KEY_D)) { // Savuşturma (Parry)
    int miss = 3;
    if (randomInt(gen) <= miss) {
      damage = 0;
    } else {
      damage = randomInt(gen) * 2;
    }
    actionTaken = true;
  }

  // Sadece bir tuşa basıldıysa tur oynanır
  if (actionTaken) {
    monster.healtBar -= damage;

    // Blok yapıldıysa oyuncu hasar almasın
    if (!IsKeyPressed(KEY_S)) {
      player.hitPoint -= monster.damage;
    }

    // Durum kontrolleri
    if (monster.healtBar <= 0) {
      // Canavarı haritadan sil ki tekrar combat başlamasın
      if ((int)player.y >= 0 && (int)player.y < mapHeight &&
          (int)player.x >= 0 && (int)player.x < mapWidth) {
        map[(int)player.y][(int)player.x] = 0;
      }
      state = STATE_EXPLORATION;
    } else if (player.hitPoint <= 0) {
      state = STATE_VICTORY; // veya ölüm ekranı
    }
  }
}

void Game::DrawCombat() {
  ClearBackground(BLACK);

  int screenW = GetScreenWidth();
  int screenH = GetScreenHeight();

  int monsterWidth = 100;
  int monsterHeight = 100;
  int monsterPosX = screenW / 2 - monsterWidth / 2;
  int monsterPosY = screenH / 2 - monsterHeight / 2 - 40;

  // Canavar kutusu
  DrawRectangle(monsterPosX, monsterPosY, monsterWidth, monsterHeight, RED);

  // Can göstergeleri (Crash yapmayacak TextFormat formatı)
  DrawText(TextFormat("Dusman Cani: %d", monster.healtBar), monsterPosX - 15,
           monsterPosY - 25, 20, MAROON);
  DrawText(TextFormat("Oyuncu Cani: %d", player.hitPoint), 40, 40, 20, GREEN);

  // Kontrol ipuçları
  DrawText("[A] Saldir  |  [S] Blok  |  [D] Karsi Saldiri (Parry)",
           screenW / 2 - 220, screenH - 60, 20, RAYWHITE);
}