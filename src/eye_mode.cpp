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

// --- Constants (ปรับสำหรับ 128x64 OLED + Header) ---
static const int HEADER_H = 10;
static const int CY      = 32;   // เลื่อนศูนย์กลางตาลงมาหน่อย (กลางพื้นที่ที่เหลือ)
static const int SPACING = 20;   // ระยะห่างตา (ลดลงนิดนึงกันล้นจอ)
static const int BASE_W  = 16;   // ความกว้างตา (เล็กลงหน่อย)
static const int BASE_H  = 24;   // ความสูงตา (เล็กลงหน่อย)
static const int BLINK_MS = 200;

static const char* NAMES[] = {
  "NORMAL", "HAPPY", "ANGRY", "CRAZY", "SLEEPY"
};

// ============================================
// Helper: U8g2 — fillRoundRect (ไม่มีใน U8g2 ตรงๆ ใช้ Box แทน)
// ============================================
static void fillRoundRect(int x, int y, int w, int h, int /*r*/) {
  // Prevent drawing over header or footer
  if (y < HEADER_H) y = HEADER_H;
  if (y + h > 54) h = 54 - y;
  if (h > 0) {
    oled.setDrawColor(1);
    oled.drawBox(x, y, w, h);
  }
}

static void clearRoundRect(int x, int y, int w, int h) {
  if (y < HEADER_H) y = HEADER_H;
  if (y + h > 54) h = 54 - y;
  if (h > 0) {
    oled.setDrawColor(0);
    oled.drawBox(x, y, w, h);
    oled.setDrawColor(1);
  }
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
// Helper: วาดตา 1 ข้าง พร้อม blink
// ============================================
static void drawEyeBox(int ex, int ey, int w, int h, float blink) {
  int eyeH = max(3, (int)(h * (1.0f - blink)));
  fillRoundRect(ex - w / 2, ey - eyeH / 2, w, eyeH, 4);
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
    targetX = random(-6, 7);
    targetY = random(-4, 5);
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
// HAPPY: ตาโค้งขึ้น ^_^ + เด้งๆ
// ============================================
static void drawHappy() {
  int cx = SCREEN_W / 2;
  float bounce = sin(millis() * 0.005f) * 2;
  int ey = CY + (int)bounce;

  for (int i : {-1, 1}) {
    int ex = cx + i * SPACING;
    oled.setDrawColor(1);
    oled.drawDisc(ex, ey + 4, 8);
    oled.setDrawColor(0);
    oled.drawBox(ex - 9, ey + 5, 18, 9);
    oled.setDrawColor(1);
  }
}

// ============================================
// ANGRY: ตาเฉียง + สั่นเบาๆ
// ============================================
static void drawAngry() {
  int cx = SCREEN_W / 2;
  float shakeX = sin(millis() * 0.05f) * 2;
  float shakeY = cos(millis() * 0.07f) * 1;

  for (int i : {-1, 1}) {
    int ex = cx + i * SPACING + (int)shakeX;
    int ey = CY + (int)shakeY;

    // วาดตาพื้นฐาน
    fillRoundRect(ex - 7, ey - 10, 14, 20, 3);

    // ตัดมุมบนให้เฉียง → ตาดุ
    oled.setDrawColor(0);
    if (i == -1) {
      oled.drawTriangle(ex - 8, ey - 11, ex + 8, ey - 11, ex + 8, ey - 3);
    } else {
      oled.drawTriangle(ex - 8, ey - 11, ex + 8, ey - 11, ex - 8, ey - 3);
    }
    oled.setDrawColor(1);
  }
}

// ============================================
// CRAZY: สั่นหนัก + ขนาดเปลี่ยน
// ============================================
static void drawCrazy() {
  int cx = SCREEN_W / 2;
  int shakeX = random(-3, 4);
  int shakeY = random(-3, 4);
  int sizeOff = (int)(sin(millis() * 0.02f) * 3);
  int w = BASE_W + sizeOff;
  int h = BASE_H + sizeOff;

  for (int i : {-1, 1}) {
    int ex = cx + i * (SPACING + random(-2, 3)) + shakeX;
    int ey = CY + shakeY;
    fillRoundRect(ex - w / 2, ey - h / 2, w, h, 4);
  }
}

// ============================================
// SLEEPY: ค่อยๆ หลับ + ผงกหัว + z z z
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
  int eyeH = max(3, (int)(BASE_H * (1.0f - sleepLevel)));

  for (int i : {-1, 1}) {
    int ex = cx + i * SPACING;
    int ey = CY + (int)drift + (int)(sleepLevel * 4);
    fillRoundRect(ex - BASE_W / 2, ey - eyeH / 2, BASE_W, eyeH, 3);
  }

  // วาด "z z" ลอยขึ้นตอนหลับสนิท
  if (sleepLevel > 0.7f) {
    int zy = CY - 14 + (int)(sin(millis() * 0.003f) * 3);
    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(88, zy, "z z");
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

#include "wifi_module.h"

void eye_mode_update() {
  display_clear();

  // --- TOP SECTION (Header) ---
  oled.setFont(u8g2_font_5x7_tr);
  oled.setDrawColor(1);
  
  // Time on left
  oled.setCursor(0, HEADER_H-2);
  oled.print(wifi_get_time_string());
  
  // Status on right
  const char* status = wifi_get_status_string();
  int sw = oled.getStrWidth(status);
  oled.setCursor(SCREEN_W - sw, HEADER_H-2);
  oled.print(status);
  
  oled.drawHLine(0, HEADER_H, SCREEN_W); // Divider line

  // --- MIDDLE SECTION (Eye) ---
  switch (emotion) {
    case EMOTION_NORMAL:  drawNormal();  break;
    case EMOTION_HAPPY:   drawHappy();   break;
    case EMOTION_ANGRY:   drawAngry();   break;
    case EMOTION_CRAZY:   drawCrazy();   break;
    case EMOTION_SLEEPY:  drawSleepy();  break;
    default: drawNormal(); break;
  }

  // --- BOTTOM SECTION (Footer) ---
  const char* name = NAMES[(int)emotion];
  oled.setFont(u8g2_font_5x7_tf);
  int tw = oled.getStrWidth(name);
  oled.drawHLine(0, 54, SCREEN_W); // Divider line
  oled.drawStr((SCREEN_W - tw) / 2, 63, name);

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
