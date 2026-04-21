# 📟 MCU Specification: ESP32-C3 Super Mini

### 📋 General Specs
- **Model:** ESP32-C3 (RISC-V Single-Core)
- **Max Clock:** 160 MHz
- **Flash:** 4 MB (Super Mini)
- **SRAM:** 400 KB (Internal)
- **Voltage:** 3.3V (Recommended)
- **Onboard LED:** GPIO 8 (Active Low)

### 🔌 Detailed Pinout & Peripheral Reference
| Peripheral | Pins | Function |
|---|---|---|
| **I2C (OLED)** | **GPIO 21 (SCL), GPIO 20 (SDA)** | SH1106 Display (128x64) |
| **Input (Button)** | **GPIO 10** | ปุ่มเดียว: กดสั้น = เปลี่ยนอารมณ์, กดแช่ 1 วิ = สลับโหมด |
| **USB-C** | Internal CDC | Upload / Serial Monitor |

### ⚙️ OLED Display: 1.3" Monochrome
- **Driver:** SH1106
- **Address:** 0x3C
- **Interface:** I2C (Hardware)
- **U8G2 Config:** `U8G2_SH1106_128X64_NONAME_F_HW_I2C`
- **Rotation:** `U8G2_R2` (กลับหัว 180°)

### ⚙️ Recommended PlatformIO Config
```ini
[env:esp32c3]
platform = espressif32
board = lolin_c3_mini
framework = arduino
monitor_speed = 115200
board_build.mcu = esp32c3
lib_deps = 
	olikraus/U8g2
```

### ⚠️ Common Hardware Issues / Tips
- **USB CDC on Boot:** มั่นใจว่าเปิดใช้ USB CDC ในบอร์ด `esp32c3` เพื่อให้ดู Log ผ่าน USB ได้
- **Active Low:** ปุ่ม GPIO 10 ใช้ `INPUT_PULLUP` (กดแล้วเป็น LOW)
- **I2C Scanning:** หากจอไม่ติด ให้เช็ค Log จะมี I2C Scanner บอก address (ปกติ 0x3C)
- **Rotation:** โค้ดใช้ `U8G2_R2` เพื่อกลับหัวจอ 180 องศาให้ตรงกับการวางบอร์ด
- **WiFi Timeout:** ต่อ WiFi ไม่ได้ภายใน 10 วินาทีจะข้ามเข้าโหมดดวงตาอัตโนมัติ
- **Screen Clamp:** ตาทุกอารมณ์มี Clamp ไม่ให้ pixel ล้นขอบจอ (0-63)
