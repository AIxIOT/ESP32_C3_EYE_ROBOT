#include "display.h"
#include <math.h>

// ใช้ U8G2_R2 แทน U8G2_R0 เพื่อกลับหัวจอ 180 องศา
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R2, U8X8_PIN_NONE);

void display_init() {
  Serial.println("Initialing I2C...");
  Wire.setPins(PIN_SDA, PIN_SCL);
  Wire.begin();
  Wire.setTimeOut(100); 

  // --- I2C Scanner ---
  byte error, address;
  bool found = false;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      found = true;
    }
  }
  if (!found) Serial.println("No I2C devices found! Check wiring.");

  // สำหรับ U8g2 I2C address ใน setI2CAddress ต้องเป็นแบบ 8-bit (address << 1)
  // 0x3C << 1 = 0x78
  oled.setI2CAddress(0x3C * 2); 
  oled.begin();
  oled.setContrast(255); 
  oled.clearBuffer();
  oled.sendBuffer();
}

void display_clear() {
  oled.clearBuffer();
}

void display_send() {
  oled.sendBuffer();
}

void display_show_wifi_status(const char* ssid, const char* status) {
  oled.clearBuffer();
  
  // --- ชื่อ WiFi ตัวใหญ่ตรงกลาง ---
  oled.setFont(u8g2_font_helvB10_tr);
  int sw = oled.getStrWidth(ssid);
  oled.drawStr((SCREEN_W - sw) / 2, 20, ssid);
  
  // --- วงกลม Spinner หมุนๆ ---
  int cx = SCREEN_W / 2;
  int cy = 42;
  int radius = 10;
  
  // วาดวงกลมจาง
  oled.drawCircle(cx, cy, radius);
  
  // วาดจุดหมุนรอบวงกลม (3 จุด)
  float angle = (millis() % 2000) * 0.00314f; // หมุนครบรอบทุก ~2 วิ
  for (int i = 0; i < 3; i++) {
    float a = angle + i * 2.094f; // แบ่ง 3 จุดห่างกัน 120 องศา
    int dx = cx + (int)(radius * cos(a));
    int dy = cy + (int)(radius * sin(a));
    oled.drawDisc(dx, dy, 2);
  }

  // --- Status text ด้านล่าง ---
  oled.setFont(u8g2_font_5x7_tr);
  int stw = oled.getStrWidth(status);
  oled.drawStr((SCREEN_W - stw) / 2, 62, status);

  oled.sendBuffer();
}
