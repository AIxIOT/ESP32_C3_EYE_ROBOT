#include "display.h"
#include "eye_mode.h"
#include "table_mode.h"
#include "input.h"
#include "wifi_module.h"

enum AppMode {
  MODE_EYE,
  MODE_TABLE
};

static AppMode currentMode = MODE_EYE;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== ESP32-C3 OLED App ===");

  input_init();
  display_init();
  wifi_init();
  eye_mode_init();
  table_mode_init();

  Serial.println("Ready!");
}

void loop() {
  input_update();
  wifi_update();

  // จัดการปุ่ม Mode (สลับโหมด: Eye ↔ Table)
  if (input_btn_mode()) {
    if (currentMode == MODE_EYE) {
      currentMode = MODE_TABLE;
      Serial.println("Mode: TABLE");
    } else {
      currentMode = MODE_EYE;
      Serial.println("Mode: EYE");
    }
  }

  // จัดการปุ่ม Action (เปลี่ยนอารมณ์ใน Eye หรือ Scroll ใน Table)
  if (input_btn_action()) {
    if (currentMode == MODE_EYE) {
      EyeEmotion next = (EyeEmotion)((eye_mode_get_emotion() + 1) % EMOTION_COUNT);
      eye_mode_set_emotion(next);
      Serial.printf("Emotion: %s\n", eye_mode_emotion_name());
    } else if (currentMode == MODE_TABLE) {
      table_mode_scroll_down();
      Serial.println("Table: Scroll Down");
    }
  }

  // Render ตามโหมดปัจจุบัน
  if (currentMode == MODE_EYE) {
    eye_mode_update();
  } else if (currentMode == MODE_TABLE) {
    table_mode_update();
  }

  delay(16);  // ~60 FPS
}
