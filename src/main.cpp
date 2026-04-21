#include "display.h"
#include "eye_mode.h"
#include "input.h"
#include "wifi_module.h"

enum AppMode {
  MODE_EYE,
  MODE_CLOCK
};

static AppMode currentMode = MODE_EYE;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== ESP32-C3 EYE ROBOT ===");

  input_init();
  display_init();
  wifi_init();
  eye_mode_init();

  Serial.println("Ready!");
}

void loop() {
  input_update();
  wifi_update();

  // --- จัดการปุ่ม Multi-function (GPIO 10) ---
  
  // 1. กดแช่ 1 วิ = เปลี่ยนโหมด
  if (input_btn_long_press()) {
    if (currentMode == MODE_EYE) {
      currentMode = MODE_CLOCK;
      Serial.println("Mode: CLOCK");
    } else {
      currentMode = MODE_EYE;
      Serial.println("Mode: EYE");
    }
  }

  // 2. จัดการตามโหมดปัจจุบัน ---
  if (currentMode == MODE_EYE) {
    // โหมดดวงตา: กดเช้า (Short Press) เปลี่ยนอารมณ์
    if (input_btn_short_press()) {
      EyeEmotion next = (EyeEmotion)((eye_mode_get_emotion() + 1) % EMOTION_COUNT);
      eye_mode_set_emotion(next);
      Serial.printf("Emotion: %s\n", eye_mode_emotion_name());
    }
    eye_mode_update();

  } else if (currentMode == MODE_CLOCK) {
    // โหมดนาฬิกา: โชว์เวลา
    oled.clearBuffer();
    
    oled.setFont(u8g2_font_logisoso24_tf); 
    const char* timeStr = wifi_get_time_string();
    int tw = oled.getStrWidth(timeStr);
    oled.drawStr((SCREEN_W - tw) / 2, 45, timeStr);
    
    oled.setFont(u8g2_font_6x12_tr);
    const char* statusStr = "WIFI CONNECTED"; 
    int dw = oled.getStrWidth(statusStr);
    oled.drawStr((SCREEN_W - dw) / 2, 60, statusStr);
    
    oled.sendBuffer();
  }

  delay(16);  // ~60 FPS
}
