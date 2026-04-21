#include "eye_mode.h"
#include "display.h"
#include <Arduino.h>

static EyeEmotion emotion = EMOTION_NORMAL;

// --- Animation state ---
static float posX = 0, posY = 0;
static float targetX = 0, targetY = 0;
static unsigned long nextMoveTime = 0;
static unsigned long blinkStart = 0;
static bool isBlinking = false;
static unsigned long nextBlinkTime = 0;
static float sleepLevel = 0;
static bool sleepClosing = true;
static unsigned long sleepHoldTime = 0;

// --- Constants (128x64 OLED — ตาโตสุดๆ) ---
static const int CY      = 32;   // กึ่งกลางแนวตั้ง
static const int SPACING = 26;   // ระยะห่างตา
static const int BASE_W  = 30;   // ความกว้างตา (โตมาก)
static const int BASE_H  = 44;   // ความสูงตา (โตมาก)
static const int BLINK_MS = 200;
static const int CORNER_R = 10;   // ความโค้งมน

// ขอบเขตจอ
static const int SCREEN_TOP    = 0;
static const int SCREEN_BOTTOM = 64;

static const char* NAMES[] = {
  "NORMAL", "HAPPY", "ANGRY", "CRAZY", "SLEEPY",
  "HUNGRY", "SAD", "LAUGH"
};

// ============================================
// Helper: วาดกล่องมน + Clamp ไม่ให้ล้นจอ
// ============================================
static void fillRoundRect(int x, int y, int w, int h, int r) {
  if (y < SCREEN_TOP) { h -= (SCREEN_TOP - y); y = SCREEN_TOP; }
  if (y + h > SCREEN_BOTTOM) { h = SCREEN_BOTTOM - y; }
  if (w < 1 || h < 1) return;
  int maxR = min(w, h) / 2;
  if (r > maxR) r = maxR;
  oled.setDrawColor(1);
  oled.drawRBox(x, y, w, h, r);
}

// ============================================
// Helper: กระพริบ
// ============================================
static float getBlinkAmount() {
  if (!isBlinking) return 0;
  unsigned long elapsed = millis() - blinkStart;
  if (elapsed >= (unsigned long)BLINK_MS) {
    isBlinking = false;
    return 0;
  }
  float t = (float)elapsed / BLINK_MS;
  return (t < 0.5f) ? (t * 2.0f) : ((1.0f - t) * 2.0f);
}

static void triggerBlink() {
  if (!isBlinking) {
    isBlinking = true;
    blinkStart = millis();
  }
}

// ============================================
// Helper: วาดตา 1 ข้าง
// ============================================
static void drawEyeBox(int ex, int ey, int w, int h, float blink) {
  int eyeH = max(4, (int)(h * (1.0f - blink)));
  fillRoundRect(ex - w / 2, ey - eyeH / 2, w, eyeH, CORNER_R);
}

// ============================================
// NORMAL: กรอกตาลื่นๆ + กระพริบ
// ============================================
static void drawNormal() {
  int cx = SCREEN_W / 2;
  float easing = 0.12f;
  posX += (targetX - posX) * easing;
  posY += (targetY - posY) * easing;

  if (millis() > nextMoveTime) {
    targetX = random(-10, 11);
    targetY = random(-3, 4);
    nextMoveTime = millis() + random(800, 2500);
  }

  if (millis() > nextBlinkTime && !isBlinking) {
    triggerBlink();
    nextBlinkTime = millis() + random(2000, 5000);
  }

  float blink = getBlinkAmount();
  for (int i : {-1, 1}) {
    drawEyeBox(cx + i * SPACING + (int)posX, CY + (int)posY, BASE_W, BASE_H, blink);
  }
}

// ============================================
// HAPPY: ตาโค้งยิ้ม ^_^
// ============================================
static void drawHappy() {
  int cx = SCREEN_W / 2;
  float bounce = sin(millis() * 0.005f) * 3;
  int ey = CY + (int)bounce;

  for (int i : {-1, 1}) {
    int ex = cx + i * SPACING;
    oled.setDrawColor(1);
    oled.drawDisc(ex, ey + 2, 16);
    oled.setDrawColor(0);
    oled.drawBox(ex - 18, ey + 3, 36, 18);
    oled.setDrawColor(1);
  }
}

// ============================================
// ANGRY: ตาเฉียงดุ
// ============================================
static void drawAngry() {
  int cx = SCREEN_W / 2;
  float shakeX = sin(millis() * 0.05f) * 2;
  float shakeY = cos(millis() * 0.07f) * 1;

  for (int i : {-1, 1}) {
    int ex = cx + i * SPACING + (int)shakeX;
    int ey = CY + (int)shakeY;

    fillRoundRect(ex - 12, ey - 16, 24, 32, 4);

    // คิ้วเฉียง
    oled.setDrawColor(0);
    if (i == -1) {
      oled.drawTriangle(ex - 14, ey - 18, ex + 14, ey - 18, ex + 14, ey - 4);
    } else {
      oled.drawTriangle(ex + 14, ey - 18, ex - 14, ey - 18, ex - 14, ey - 4);
    }
    oled.setDrawColor(1);
  }
}

// ============================================
// CRAZY: สั่นหนัก + ขนาดเปลี่ยน
// ============================================
static void drawCrazy() {
  int cx = SCREEN_W / 2;
  int shakeX = random(-5, 6);
  int shakeY = random(-5, 6);
  int sizeOff = (int)(sin(millis() * 0.02f) * 6);
  int w = BASE_W + sizeOff;
  int h = BASE_H + sizeOff;

  for (int i : {-1, 1}) {
    int ex = cx + i * (SPACING + random(-3, 4)) + shakeX;
    int ey = CY + shakeY;
    fillRoundRect(ex - w / 2, ey - h / 2, w, h, CORNER_R);
  }
}

// ============================================
// SLEEPY: ค่อยๆ หลับ + z z z
// ============================================
static void drawSleepy() {
  int cx = SCREEN_W / 2;

  if (sleepClosing) {
    sleepLevel += 0.008f;
    if (sleepLevel >= 1.0f) {
      sleepLevel = 1.0f;
      sleepClosing = false;
      sleepHoldTime = millis() + 1500;
    }
  } else {
    if (millis() > sleepHoldTime) {
      sleepLevel = 0.15f;
      sleepClosing = true;
    }
  }

  float drift = sin(millis() * 0.001f) * 2;
  int eyeH = max(4, (int)(BASE_H * (1.0f - sleepLevel)));

  for (int i : {-1, 1}) {
    int ex = cx + i * SPACING;
    int ey = CY + (int)drift + (int)(sleepLevel * 5);
    fillRoundRect(ex - BASE_W / 2, ey - eyeH / 2, BASE_W, eyeH, CORNER_R);
  }

  if (sleepLevel > 0.7f) {
    oled.setFont(u8g2_font_6x12_tr);
    int zy = CY - 18 + (int)(sin(millis() * 0.003f) * 4);
    oled.drawStr(96, max(8, zy), "z z");
  }
}

// ============================================
// HUNGRY: ตาปรือ + น้ำลายหยด
// ============================================
static void drawHungry() {
  int cx = SCREEN_W / 2;
  float drift = sin(millis() * 0.002f) * 2;

  // ตาปรือ (แบนลง)
  for (int i : {-1, 1}) {
    int ex = cx + i * SPACING;
    int ey = CY + (int)drift;
    int eyeH = 12; // ตาแบนๆ
    fillRoundRect(ex - BASE_W / 2, ey - eyeH / 2, BASE_W, eyeH, 5);
  }

  // น้ำลายหยดจากปากข้างซ้าย (จุดเลื่อนลง)
  int dripY = (millis() / 8) % 20;  // หยดวนซ้ำ
  int dripX = cx - 3;
  oled.drawCircle(dripX, CY + 16 + dripY, 2);
  if (dripY > 5) {
    oled.drawVLine(dripX, CY + 16, dripY - 3);
  }

  // วาดปากยื่น
  oled.drawEllipse(cx, CY + 14, 6, 3, U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT);
}

// ============================================
// SAD: ตาลู่ลง + น้ำตา
// ============================================
static void drawSad() {
  int cx = SCREEN_W / 2;
  float drift = sin(millis() * 0.001f) * 1;

  for (int i : {-1, 1}) {
    int ex = cx + i * SPACING;
    int ey = CY + 4 + (int)drift; // เลื่อนลงนิดนึง (ตาตก)

    // ตากลมโต แต่มีคิ้วตก
    fillRoundRect(ex - 13, ey - 14, 26, 28, 8);

    // คิ้วตก (เฉียงลง ตรงข้ามกับ Angry)
    oled.setDrawColor(0);
    if (i == -1) {
      oled.drawTriangle(ex - 15, ey - 6, ex - 15, ey - 16, ex + 5, ey - 16);
    } else {
      oled.drawTriangle(ex + 15, ey - 6, ex + 15, ey - 16, ex - 5, ey - 16);
    }
    oled.setDrawColor(1);

    // น้ำตาไหล (เส้นแนวตั้ง + หยด)
    int tearY = (millis() / 6) % 24;
    int tearX = ex + i * 8;
    if (tearY > 2) {
      oled.drawVLine(tearX, ey + 10, min(tearY, 14));
    }
    oled.drawDisc(tearX, ey + 10 + min(tearY, 14), 2);
  }
}

// ============================================
// LAUGH: ตาหยี + เด้งหัวเราะ
// ============================================
static void drawLaugh() {
  int cx = SCREEN_W / 2;

  // เด้งขึ้นลง (หัวเราะ)
  float bounce = abs(sin(millis() * 0.008f)) * 5;
  int ey = CY - (int)bounce;

  // สั่นเบาๆ
  int shakeX = random(-1, 2);

  for (int i : {-1, 1}) {
    int ex = cx + i * SPACING + shakeX;

    // ตาหยีเป็นเส้นโค้ง (เหมือน Happy แต่โค้งมากกว่า)
    oled.setDrawColor(1);
    oled.drawDisc(ex, ey, 14);
    oled.setDrawColor(0);
    oled.drawBox(ex - 16, ey + 1, 32, 16);
    oled.setDrawColor(1);

    // เพิ่มเส้นยิ้มใต้ตา
    oled.drawCircle(ex, ey + 2, 12, U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT);
  }

  // ปากเปิดกว้าง (วงรีล่าง)
  oled.drawDisc(cx + shakeX, ey + 22, 6);
  oled.setDrawColor(0);
  oled.drawBox(cx + shakeX - 8, ey + 14, 16, 9);
  oled.setDrawColor(1);
}

// ============================================
// API หลัก
// ============================================
void eye_mode_init() {
  nextBlinkTime  = millis() + random(2000, 5000);
  nextMoveTime   = millis() + 500;
  sleepHoldTime  = millis();
}

void eye_mode_update() {
  display_clear();

  switch (emotion) {
    case EMOTION_NORMAL:  drawNormal();  break;
    case EMOTION_HAPPY:   drawHappy();   break;
    case EMOTION_ANGRY:   drawAngry();   break;
    case EMOTION_CRAZY:   drawCrazy();   break;
    case EMOTION_SLEEPY:  drawSleepy();  break;
    case EMOTION_HUNGRY:  drawHungry();  break;
    case EMOTION_SAD:     drawSad();     break;
    case EMOTION_LAUGH:   drawLaugh();   break;
    default: drawNormal(); break;
  }

  display_send();
}

void eye_mode_set_emotion(EyeEmotion e) {
  emotion       = e;
  sleepLevel    = 0;
  sleepClosing  = true;
  sleepHoldTime = millis();
  isBlinking    = false;
  posX = posY   = 0;
  targetX = targetY = 0;
}

EyeEmotion eye_mode_get_emotion() {
  return emotion;
}

const char* eye_mode_emotion_name() {
  return NAMES[(int)emotion];
}
