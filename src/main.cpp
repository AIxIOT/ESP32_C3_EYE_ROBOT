#include "display.h"
#include "eye_mode.h"
#include "clock_mode.h"
#include "stopwatch_mode.h"
#include "dino_mode.h"
#include "input.h"
#include "wifi_module.h"

enum AppMode {
  MODE_EYE,
  MODE_CLOCK,
  MODE_STOPWATCH,
  MODE_DINO
};

static AppMode currentMode = MODE_EYE;

// ============================================
// Auto Emotion Scheduler
// ============================================
static EyeEmotion lastSetEmotion = EMOTION_NORMAL;

// Override (กดสั้นเปลี่ยนอารมณ์ชั่วคราว 10 วิ)
static bool isOverride = false;
static unsigned long overrideEndTime = 0;
static const unsigned long OVERRIDE_DURATION = 10000; // 10 วินาที

// Random mood swing (ช่วง NORMAL สุ่มแทรกอารมณ์)
static unsigned long nextMoodSwing = 0;
static unsigned long moodSwingEnd = 0;
static bool inMoodSwing = false;

// Wake-up animation tracking
static int lastScheduleHour = -1;

// ============================================
// คำนวณ Base Emotion จากเวลา
// ============================================
EyeEmotion getScheduledEmotion(int hour, int minute) {
  if (hour < 0) return EMOTION_NORMAL; // ยังไม่ sync เวลา

  // 00:00 - 06:59 → หลับ 😴
  if (hour < 7) return EMOTION_SLEEPY;

  // 07:00 - 07:01 → ตื่นนอน (ยังงัวเงีย)
  if (hour == 7 && minute < 2) return EMOTION_SLEEPY;

  // 07:02 - 07:59 → หิวข้าวเช้า 🍔
  if (hour == 7) return EMOTION_HUNGRY;

  // 08:00 - 11:59 → ปกติ (ทำงานเช้า) 👀
  if (hour >= 8 && hour < 12) return EMOTION_NORMAL;

  // 12:00 - 12:59 → หิวข้าวเที่ยง 🍔
  if (hour == 12) return EMOTION_HUNGRY;

  // 13:00 - 17:59 → ปกติ (ทำงานบ่าย) 👀
  if (hour >= 13 && hour < 18) return EMOTION_NORMAL;

  // 18:00 - 18:59 → หิวข้าวเย็น 🍔
  if (hour == 18) return EMOTION_HUNGRY;

  // 19:00 - 21:59 → ปกติ (ช่วงค่ำ) 👀
  if (hour >= 19 && hour < 22) return EMOTION_NORMAL;

  // 22:00 - 23:59 → ปกติ แต่เหนื่อย (SAD แทรกถี่ขึ้น)
  return EMOTION_NORMAL;
}

// ============================================
// สุ่มอารมณ์แทรก (ช่วง NORMAL)
// ============================================
EyeEmotion getRandomMoodEmotion(int hour) {
  // ช่วงดึก 22-24: เริ่มเหนื่อย/ง่วง
  if (hour >= 22) {
    int r = random(0, 6);
    switch(r) {
      case 0: return EMOTION_SAD;
      case 1: return EMOTION_SAD;
      case 2: return EMOTION_SLEEPY;
      case 3: return EMOTION_HAPPY;
      case 4: return EMOTION_SAD;
      default: return EMOTION_SLEEPY;
    }
  }
  // ช่วงปกติ: สุ่มอารมณ์สนุกๆ
  int r = random(0, 6);
  switch(r) {
    case 0: return EMOTION_HAPPY;
    case 1: return EMOTION_LAUGH;
    case 2: return EMOTION_CRAZY;
    case 3: return EMOTION_ANGRY;
    case 4: return EMOTION_HAPPY;
    default: return EMOTION_LAUGH;
  }
}

// ============================================
// Set emotion เฉพาะสี่เปลี่ยนจริง (ไม่ reset ซ้ำ)
// ============================================
void setEmotionIfChanged(EyeEmotion e) {
  if (e != lastSetEmotion) {
    eye_mode_set_emotion(e);
    lastSetEmotion = e;
    Serial.printf("Auto Emotion → %s\n", eye_mode_emotion_name());
  }
}

// ============================================
// Setup
// ============================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== ESP32-C3 EYE ROBOT ===");

  input_init();
  display_init();
  wifi_init();
  eye_mode_init();
  clock_mode_init();
  stopwatch_mode_init();
  dino_mode_init();

  // ตั้ง mood swing timer เริ่มต้น
  nextMoodSwing = millis() + random(60000, 180000); // 1-3 นาทีแรก

  Serial.println("Ready!");
}

// ============================================
// Main Loop
// ============================================
void loop() {
  input_update();
  wifi_update();

  int hour = wifi_get_hour();
  int minute = wifi_get_minute();

  // ===== ปุ่ม Long Press → สลับโหมด Eye → Clock → Stopwatch → Dino → Eye =====
  if (input_btn_long_press()) {
    if (wifi_is_portal_active()) {
      wifi_stop_ap_portal(); // ปิด portal ถ้าเปิดอยู่
    }
    
    if (currentMode == MODE_EYE) {
      currentMode = MODE_CLOCK;
      Serial.println("Mode: CLOCK");
    } else if (currentMode == MODE_CLOCK) {
      currentMode = MODE_STOPWATCH;
      stopwatch_mode_init();
      Serial.println("Mode: STOPWATCH");
    } else if (currentMode == MODE_STOPWATCH) {
      currentMode = MODE_DINO;
      dino_mode_init();
      Serial.println("Mode: DINO");
    } else {
      currentMode = MODE_EYE;
      isOverride = false;
      Serial.println("Mode: EYE");
    }
  }

  // ===== โหมดนาฬิกา =====
  if (currentMode == MODE_CLOCK) {
    if (input_btn_short_press()) {
      clock_mode_toggle_info();
    }
    clock_mode_update();
    delay(16);
    return;
  }

  // ===== โหมดจับเวลา =====
  if (currentMode == MODE_STOPWATCH) {
    if (input_btn_short_press()) {
      stopwatch_mode_click();
    }
    stopwatch_mode_update();
    delay(16);
    return;
  }

  // ===== โหมดไดโนเสาร์ =====
  if (currentMode == MODE_DINO) {
    if (input_btn_short_press()) {
      dino_mode_jump();
    }
    dino_mode_update();
    delay(16);
    return;
  }

  // ===== โหมดดวงตา — Auto Emotion System =====

  // --- กดสั้น → Override ชั่วคราว (10 วิ) ---
  if (input_btn_short_press()) {
    EyeEmotion next = (EyeEmotion)((eye_mode_get_emotion() + 1) % EMOTION_COUNT);
    eye_mode_set_emotion(next);
    lastSetEmotion = next;
    isOverride = true;
    overrideEndTime = millis() + OVERRIDE_DURATION;
    inMoodSwing = false;
    Serial.printf("Override → %s (10s)\n", eye_mode_emotion_name());
  }

  // --- เช็ค Override หมดอายุ ---
  if (isOverride && millis() >= overrideEndTime) {
    isOverride = false;
    Serial.println("Override expired → back to Auto");
  }

  // --- ถ้าไม่ได้ Override → ระบบ Auto ---
  if (!isOverride) {
    EyeEmotion baseEmotion = getScheduledEmotion(hour, minute);
    bool isNormalPeriod = (baseEmotion == EMOTION_NORMAL);

    // Wake-up detection (07:02 → เปลี่ยนจาก SLEEPY เป็น NORMAL)
    if (hour == 7 && minute == 2 && lastScheduleHour != 7) {
      lastScheduleHour = 7;
      Serial.println("☀️ Wake up!");
    }

    // --- Random Mood Swing ช่วง NORMAL ---
    if (isNormalPeriod) {
      unsigned long now = millis();

      // เริ่ม mood swing ใหม่
      if (!inMoodSwing && now >= nextMoodSwing) {
        EyeEmotion moodEmotion = getRandomMoodEmotion(hour);
        setEmotionIfChanged(moodEmotion);
        inMoodSwing = true;
        moodSwingEnd = now + random(10000, 16000); // แสดง 10-15 วิ
        
        // กำหนดเวลาครั้งถัดไป
        if (hour >= 22) {
          nextMoodSwing = moodSwingEnd + random(60000, 120000);  // ดึก: ทุก 1-2 นาที
        } else {
          nextMoodSwing = moodSwingEnd + random(180000, 300000); // ปกติ: ทุก 3-5 นาที
        }
      }

      // mood swing หมดเวลา → กลับ NORMAL
      if (inMoodSwing && now >= moodSwingEnd) {
        inMoodSwing = false;
        setEmotionIfChanged(EMOTION_NORMAL);
      }

      // ถ้าไม่ได้อยู่ใน mood swing → แสดง NORMAL
      if (!inMoodSwing) {
        setEmotionIfChanged(EMOTION_NORMAL);
      }

    } else {
      // ช่วงที่ไม่ใช่ NORMAL (SLEEPY, HUNGRY) → แสดงตามตาราง
      inMoodSwing = false;
      setEmotionIfChanged(baseEmotion);
    }
  }

  // --- Render ---
  eye_mode_update();
  delay(16); // ~60 FPS
}
