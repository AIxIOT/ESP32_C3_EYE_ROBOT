#include "dino_mode.h"
#include "display.h"

// ============================================
// Game Constants
// ============================================
static const int GROUND_Y    = 55;   // เส้นพื้น
static const int DINO_X      = 12;   // ตำแหน่ง X ไดโน (คงที่)
static const int DINO_W      = 10;   // ความกว้างไดโน
static const int DINO_H      = 14;   // ความสูงไดโน
static const int MAX_OBSTACLES = 3;
static const float GRAVITY   = 0.6f;
static const float JUMP_FORCE = -7.0f;

// ============================================
// Game State
// ============================================
enum DinoState {
  DINO_IDLE,    // รอเริ่มเกม
  DINO_PLAYING, // กำลังเล่น
  DINO_DEAD     // ตาย แสดง Score
};

static DinoState gameState = DINO_IDLE;

// Dino physics
static float dinoY = 0;       // offset จากพื้น (ลบ = ขึ้น)
static float dinoVelY = 0;
static bool dinoOnGround = true;
static int legFrame = 0;      // เฟรมขาวิ่ง

// Obstacles
struct Obstacle {
  float x;
  int h;       // ความสูง (10-22 px)
  int w;       // ความกว้าง (4-8 px)
  bool active;
};
static Obstacle obstacles[MAX_OBSTACLES];

// Game speed & score
static float gameSpeed = 1.8f;
static unsigned long gameStartTime = 0;
static unsigned long gameScore = 0; // เวลาที่รอด (ms)
static unsigned long bestScore = 0;

// Spawn control
static float distSinceLastSpawn = 0;
static float nextSpawnDist = 60;

// Ground texture scroll
static int groundScroll = 0;

// ============================================
// วาดไดโนเสาร์
// ============================================
static void drawDino(int x, int y) {
  // หัว (ยื่นออกไปด้านหน้า)
  oled.drawBox(x + 2, y, 8, 5);
  // ตา
  oled.setDrawColor(0);
  oled.drawPixel(x + 8, y + 1);
  oled.setDrawColor(1);
  // คอ + ลำตัว
  oled.drawBox(x + 1, y + 5, 7, 6);
  // หาง
  oled.drawBox(x, y + 5, 2, 3);
  // แขนสั้น
  oled.drawPixel(x + 7, y + 7);
  oled.drawPixel(x + 8, y + 8);

  // ขา (สลับเฟรม)
  if (dinoOnGround) {
    if (legFrame < 4) {
      // ขาซ้ายหน้า ขาขวาหลัง
      oled.drawBox(x + 2, y + 11, 2, 3);
      oled.drawBox(x + 5, y + 11, 2, 2);
    } else {
      // สลับ
      oled.drawBox(x + 2, y + 11, 2, 2);
      oled.drawBox(x + 5, y + 11, 2, 3);
    }
  } else {
    // กำลังกระโดด → ขาชิดกัน
    oled.drawBox(x + 2, y + 11, 2, 3);
    oled.drawBox(x + 5, y + 11, 2, 3);
  }
}

// ============================================
// วาดไดโนตาย (ล้ม)
// ============================================
static void drawDinoDead(int x, int y) {
  // หัว (ตาเป็น X)
  oled.drawBox(x + 2, y, 8, 5);
  oled.setDrawColor(0);
  oled.drawPixel(x + 7, y + 1);
  oled.drawPixel(x + 8, y + 1);
  oled.drawPixel(x + 9, y + 2);
  oled.setDrawColor(1);
  // ลำตัว
  oled.drawBox(x + 1, y + 5, 7, 6);
  oled.drawBox(x, y + 5, 2, 3);
  // ขาแบะ
  oled.drawBox(x + 1, y + 11, 2, 3);
  oled.drawBox(x + 6, y + 11, 2, 3);
}

// ============================================
// วาด Cactus (ต้นกระบองเพชร)
// ============================================
static void drawCactus(int x, int h, int w) {
  int y = GROUND_Y - h;
  // ลำต้น
  oled.drawBox(x, y, w, h);
  // แขนซ้าย
  if (h > 12) {
    oled.drawBox(x - 2, y + 3, 2, 5);
    oled.drawBox(x - 2, y + 3, 3, 2);
  }
  // แขนขวา
  if (h > 15) {
    oled.drawBox(x + w, y + 6, 2, 4);
    oled.drawBox(x + w - 1, y + 6, 3, 2);
  }
}

// ============================================
// Spawn obstacle
// ============================================
static void spawnObstacle() {
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) {
      obstacles[i].x = 130;
      obstacles[i].h = random(10, 23);
      obstacles[i].w = random(4, 9);
      obstacles[i].active = true;
      distSinceLastSpawn = 0;
      nextSpawnDist = random(50, 100);
      return;
    }
  }
}

// ============================================
// Collision check
// ============================================
static bool checkCollision() {
  int dinoDrawY  = GROUND_Y - DINO_H + (int)dinoY;
  int dinoTop    = dinoDrawY + 2;
  int dinoBottom = dinoDrawY + DINO_H;
  int dinoLeft   = DINO_X + 2;
  int dinoRight  = DINO_X + DINO_W - 1;

  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) continue;
    int ox     = (int)obstacles[i].x;
    int oTop   = GROUND_Y - obstacles[i].h;
    int oRight = ox + obstacles[i].w;

    if (dinoRight > ox && dinoLeft < oRight &&
        dinoBottom > oTop && dinoTop < GROUND_Y) {
      return true;
    }
  }
  return false;
}

// ============================================
// Reset game
// ============================================
static void resetGame() {
  dinoY = 0;
  dinoVelY = 0;
  dinoOnGround = true;
  legFrame = 0;
  gameSpeed = 1.8f;
  distSinceLastSpawn = 0;
  nextSpawnDist = 60;
  groundScroll = 0;

  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].active = false;
  }
}

// ============================================
// API
// ============================================
void dino_mode_init() {
  resetGame();
  gameState = DINO_IDLE;
}

void dino_mode_jump() {
  if (gameState == DINO_IDLE) {
    // เริ่มเกม
    resetGame();
    gameState = DINO_PLAYING;
    gameStartTime = millis();
    Serial.println("🦕 DINO START!");
    return;
  }

  if (gameState == DINO_DEAD) {
    // เริ่มใหม่
    resetGame();
    gameState = DINO_PLAYING;
    gameStartTime = millis();
    Serial.println("🦕 DINO RESTART!");
    return;
  }

  // กระโดด (เฉพาะตอนอยู่บนพื้น)
  if (gameState == DINO_PLAYING && dinoOnGround) {
    dinoVelY = JUMP_FORCE;
    dinoOnGround = false;
  }
}

void dino_mode_update() {
  oled.clearBuffer();

  // === เส้นพื้น ===
  oled.drawHLine(0, GROUND_Y, SCREEN_W);

  if (gameState == DINO_IDLE) {
    // --- หน้าจอรอเริ่ม ---
    drawDino(DINO_X, GROUND_Y - DINO_H);

    oled.setFont(u8g2_font_helvB08_tr);
    const char* title = "DINO RUN";
    int tw = oled.getStrWidth(title);
    oled.drawStr((SCREEN_W - tw) / 2, 15, title);

    oled.setFont(u8g2_font_5x7_tr);
    const char* tap = "TAP TO START";
    int sw = oled.getStrWidth(tap);
    oled.drawStr((SCREEN_W - sw) / 2, 30, tap);

    // แสดง Best Score
    if (bestScore > 0) {
      char buf[20];
      sprintf(buf, "BEST: %lu.%lus", bestScore / 1000, (bestScore % 1000) / 100);
      int bw = oled.getStrWidth(buf);
      oled.drawStr((SCREEN_W - bw) / 2, 42, buf);
    }

    oled.sendBuffer();
    return;
  }

  if (gameState == DINO_PLAYING) {
    // --- Physics ---
    // Dino jump
    dinoVelY += GRAVITY;
    dinoY += dinoVelY;
    if (dinoY >= 0) {
      dinoY = 0;
      dinoVelY = 0;
      dinoOnGround = true;
    }

    // Leg animation
    legFrame = (legFrame + 1) % 8;

    // Move obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
      if (!obstacles[i].active) continue;
      obstacles[i].x -= gameSpeed;
      if (obstacles[i].x < -10) {
        obstacles[i].active = false;
      }
    }

    // Spawn logic
    distSinceLastSpawn += gameSpeed;
    if (distSinceLastSpawn >= nextSpawnDist) {
      spawnObstacle();
    }

    // Speed up over time (ค่อยๆ เร็วขึ้น)
    gameScore = millis() - gameStartTime;
    gameSpeed = 1.8f + (gameScore / 10000.0f) * 0.8f; // เร็วขึ้นทุก 10 วิ
    if (gameSpeed > 4.5f) gameSpeed = 4.5f; // จำกัดความเร็วสูงสุด

    // Collision
    if (checkCollision()) {
      gameState = DINO_DEAD;
      if (gameScore > bestScore) bestScore = gameScore;
      Serial.printf("🦕 DEAD! Score: %lu.%lus\n", gameScore / 1000, (gameScore % 1000) / 100);
    }

    // --- Draw ---
    // Ground texture (จุดวิ่ง)
    groundScroll = (groundScroll + (int)gameSpeed) % 8;
    for (int x = -groundScroll; x < SCREEN_W; x += 8) {
      oled.drawPixel(x, GROUND_Y + 3);
      oled.drawPixel(x + 3, GROUND_Y + 5);
      oled.drawPixel(x + 6, GROUND_Y + 7);
    }

    // Obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
      if (!obstacles[i].active) continue;
      drawCactus((int)obstacles[i].x, obstacles[i].h, obstacles[i].w);
    }

    // Dino
    int dinoDrawY = GROUND_Y - DINO_H + (int)dinoY;
    drawDino(DINO_X, dinoDrawY);

    // Score (top right)
    oled.setFont(u8g2_font_5x7_tr);
    char scoreBuf[16];
    sprintf(scoreBuf, "%lu.%lus", gameScore / 1000, (gameScore % 1000) / 100);
    int sw = oled.getStrWidth(scoreBuf);
    oled.drawStr(SCREEN_W - sw - 2, 8, scoreBuf);

    oled.sendBuffer();
    return;
  }

  if (gameState == DINO_DEAD) {
    // --- Game Over Screen ---

    // วาด Obstacles ค้างไว้
    for (int i = 0; i < MAX_OBSTACLES; i++) {
      if (!obstacles[i].active) continue;
      drawCactus((int)obstacles[i].x, obstacles[i].h, obstacles[i].w);
    }

    // Ground texture
    for (int x = -groundScroll; x < SCREEN_W; x += 8) {
      oled.drawPixel(x, GROUND_Y + 3);
      oled.drawPixel(x + 3, GROUND_Y + 5);
    }

    // Dino ตาย
    int dinoDrawY = GROUND_Y - DINO_H + (int)dinoY;
    drawDinoDead(DINO_X, dinoDrawY);

    // GAME OVER text
    oled.setFont(u8g2_font_helvB08_tr);
    const char* go = "GAME OVER";
    int gow = oled.getStrWidth(go);
    oled.drawStr((SCREEN_W - gow) / 2, 15, go);

    // Score
    oled.setFont(u8g2_font_6x12_tr);
    char buf[24];
    sprintf(buf, "TIME: %lu.%lus", gameScore / 1000, (gameScore % 1000) / 100);
    int tw = oled.getStrWidth(buf);
    oled.drawStr((SCREEN_W - tw) / 2, 30, buf);

    // Best
    sprintf(buf, "BEST: %lu.%lus", bestScore / 1000, (bestScore % 1000) / 100);
    int bw = oled.getStrWidth(buf);
    oled.drawStr((SCREEN_W - bw) / 2, 42, buf);

    oled.sendBuffer();
    return;
  }
}
