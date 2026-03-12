#include "display.h"

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
  
  // กรอบนอก
  oled.drawFrame(0, 0, SCREEN_W, SCREEN_H);
  
  // หัวข้อ
  oled.setFont(u8g2_font_haxrcorp4089_tr);
  oled.drawStr(10, 15, "WIFI CONNECTION");
  oled.drawHLine(5, 20, SCREEN_W - 10);
  
  // SSID
  oled.setFont(u8g2_font_6x12_tr);
  oled.drawStr(10, 35, "SSID:");
  oled.drawStr(45, 35, ssid);
  
  // Status
  oled.drawStr(10, 50, "Status:");
  oled.drawStr(55, 50, status);
  
  oled.sendBuffer();
}
