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

// --- Constants (ปรับสำหรับ 128x64 OLED — ตาใหญ่ขึ้น) ---
static const int CY      = 32;   // กึ่งกลางแนวตั้ง (0-63)
static const int SPACING = 24;   // ระยะห่างตา (เพิ่มตามขนาด)
static const int BASE_W  = 24;   // ความกว้างตา (ใหญ่ขึ้น)
static const int BASE_H  = 36;   // ความสูงตา (ใหญ่ขึ้น)
static const int BLINK_MS = 200;
static const int CORNER_R = 8;    // ความโค้งมนของดวงตา

// ขอบเขตจอที่ปลอดภัย (Margin 1px)
static const int SCREEN_TOP    = 1;
static const int SCREEN_BOTTOM = 63;

static const char* NAMES[] = {
  "NORMAL", "HAPPY", "ANGRY", "CRAZY", "SLEEPY"
};

// ============================================
// Helper: วาดกล่องมน + Clamp ไม่ให้ล้นจอ
// ============================================
static void fillRoundRect(int x, int y, int w, int h, int r) {
  // Clamp ขอบบน
  if (y < SCREEN_TOP) { h -= (SCREEN_TOP - y); y = SCREEN_TOP; }
  // Clamp ขอบล่าง
  if (y + h > SCREEN_BOTTOM) { h = SCREEN_BOTTOM - y; }
  if (w < 1 || h < 1) return;
  // ปรับ radius ให้ไม่เกินครึ่งหนึ่งของ w หรือ h
  int maxR = min(w, h) / 2;
  if (r > maxR) r = maxR;
  oled.setDrawColor(1);
  oled.drawRBox(x, y, w, h, r);
}

// ============================================
// Helper: คำนวณค่ากระพริบ (0=เปิด, 1=หลับ)
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
// Helper: วาดตา 1 ข้าง พร้อม blink (Clamp อัตโนมัติ)
// ============================================
static void drawEyeBox(int ex, int ey, int w, int h, float blink) {
  int eyeH = max(4, (int)(h * (1.0f - blink)));
  fillRoundRect(ex - w / 2, ey - eyeH / 2, w, eyeH, CORNER_R);
}

// ============================================
// NORMAL: กรอกตาลื่นๆ + กระพริบอัตโนมัติ
// ============================================
static void drawNormal() {
  int cx = SCREEN_W / 2;
  float easing = 0.12f;

  posX += (targetX - posX) * easing;
  posY += (targetY - posY) * easing;

  if (millis() > nextMoveTime) {
    targetX = random(-8, 9);
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
// HAPPY: ตาโค้งขึ้น ^_^ + เด้งๆ (ใหญ่ขึ้น)
// ============================================
static void drawHappy() {
  int cx = SCREEN_W / 2;
  float bounce = sin(millis() * 0.005f) * 3;
  int ey = CY + (int)bounce;

  for (int i : {-1, 1}) {
    int ex = cx + i * SPACING;
    oled.setDrawColor(1);
    oled.drawDisc(ex, ey + 4, 14);   // วงกลมใหญ่ขึ้น
    oled.setDrawColor(0);
    oled.drawBox(ex - 16, ey + 5, 32, 16); // ตัดครึ่งล่างออก
    oled.setDrawColor(1);
  }
}

// ============================================
// ANGRY: ตาเฉียง + สั่นเบาๆ (ใหญ่ขึ้น)
// ============================================
static void drawAngry() {
  int cx = SCREEN_W / 2;
  float shakeX = sin(millis() * 0.05f) * 2;
  float shakeY = cos(millis() * 0.07f) * 1;

  for (int i : {-1, 1}) {
    int ex = cx + i * SPACING + (int)shakeX;
    int ey = CY + (int)shakeY;

    // วาดตาพื้นฐาน (ใหญ่ขึ้น)
    fillRoundRect(ex - 10, ey - 14, 20, 28, 3);

    // ตัดมุมบนให้เฉียง → ตาดุ
    oled.setDrawColor(0);
    if (i == -1) {
      oled.drawTriangle(ex - 12, ey - 16, ex + 12, ey - 16, ex + 12, ey - 4);
    } else {
      oled.drawTriangle(ex + 12, ey - 16, ex - 12, ey - 16, ex - 12, ey - 4);
    }
    oled.setDrawColor(1);
  }
}

// ============================================
// CRAZY: สั่นหนัก + ขนาดเปลี่ยน (ใหญ่ขึ้น)
// ============================================
static void drawCrazy() {
  int cx = SCREEN_W / 2;
  int shakeX = random(-4, 5);
  int shakeY = random(-4, 5);
  int sizeOff = (int)(sin(millis() * 0.02f) * 5);
  int w = BASE_W + sizeOff;
  int h = BASE_H + sizeOff;

  for (int i : {-1, 1}) {
    int ex = cx + i * (SPACING + random(-3, 4)) + shakeX;
    int ey = CY + shakeY;
    fillRoundRect(ex - w / 2, ey - h / 2, w, h, CORNER_R);
  }
}

// ============================================
// SLEEPY: ค่อยๆ หลับ + ผงกหัว + z z z (ใหญ่ขึ้น)
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

  // วาด "z z z" ลอยขึ้นตอนหลับสนิท
  if (sleepLevel > 0.7f) {
    oled.setFont(u8g2_font_6x12_tr);
    int zy = CY - 18 + (int)(sin(millis() * 0.003f) * 4);
    oled.drawStr(92, max(8, zy), "z z");
  }
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
