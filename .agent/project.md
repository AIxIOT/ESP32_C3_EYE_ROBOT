# ESP32_C3_OLED_EYE — Project Document

## Overview
โปรเจคแสดง **Eye Animation** บนจอ OLED 1.3" ผ่าน ESP32-C3 Mini  
เป็นโปรเจคน้องของ [ESP32S3_LCD_SCREEN](../ESP32S3_LCD_SCREEN/) — ใช้คนละบอร์ด คนละจอ ฟีเจอร์น้อยกว่า  
**Phase แรก: แสดงตา animation เท่านั้น** (ยังไม่มี WiFi / MQTT / Table Mode)

---

## Hardware

### Microcontroller: ESP32-C3 Mini
#### Arduino IDE Settings:
- **Board:** ESP32C3 Dev Module
- **USB CDC On Boot:** Enabled
- **CPU Frequency:** 160MHz
- **Flash Size:** 4MB (default)
- **Partition:** Default 4MB with spiffs
- **Upload Speed:** 115200

### Display: 1.3" OLED (Monochrome White)
- **Driver IC:** SH1106
- **Interface:** I2C
- **Resolution:** 128 × 64
- **Color:** White only (monochrome)
- **I2C Address:** 0x3C

### Pinout

| Function | GPIO | Notes |
|----------|------|-------|
| SCL      | 21   | Hardware I2C (Wire) |
| SDA      | 20   | Hardware I2C (Wire) |
| BTN ACTION | 10  | Change Emotion / Scroll |
| BTN MODE   | 0   | Switch Mode |

---

## Features (Eye & Table Mode)

### Eye Emotions (5 แบบ)
| Emotion  | ลักษณะ                            |
|----------|-----------------------------------|
| NORMAL   | กรอกตาไปมา + กระพริบอัตโนมัติ      |
| HAPPY    | ตาโค้งขึ้น ^_^ + เด้งๆ             |
| ANGRY    | ตาเฉียงดุ + สั่นเบาๆ              |
| CRAZY    | สั่นหนัก + ขนาดเปลี่ยน            |
| SLEEPY   | ค่อยๆ หลับ + ผงกหัว + z z z       |

### Input
- **GPIO 10 (Action):** กดเปลี่ยน Emotion หรือเลื่อนตาราง
- **GPIO 0 (Mode):** สลับโหมด Eye ↔ Table

---

## Module Structure

```
EYE_ROBOT_01/
├── src/
│   ├── main.cpp         ← ไฟล์หลัก (setup/loop)
│   ├── display.h        ← U8g2 config สำหรับ SH1106 I2C (GPIO 20, 21)
│   ├── display.cpp      ← display_init() + helper
│   ├── eye_mode.h       ← EyeEmotion enum + API
│   ├── eye_mode.cpp     ← logic วาดตาทั้ง 5 emotion
│   ├── table_mode.h     ← Table mode view
│   ├── table_mode.cpp   ← scroll logic
│   ├── input.h          ← Input API
│   ├── input.cpp        ← Button debounce (GPIO 0, 10)
│   ├── wifi_module.h    ← WiFi + NTP setup
│   └── wifi_module.cpp  ← Sync time & status
├── project.md           ← เอกสารโปรเจค (ไฟล์นี้)
├── hardware.md          ← สเปคบอร์ดและการเชื่อมต่อ
└── rule.md              ← กฎการเขียนโค้ด
```

---

## Library Dependencies
- **U8g2** (by olikraus) — รองรับ SH1106 I2C + ESP32-C3 RISC-V เต็มที่

---

## Notes
- จอ OLED เป็น monochrome → ไม่มีสี ใช้ขาว/ดำเท่านั้น
- ตาออกแบบสำหรับ 128×64 → ขนาดเล็กกว่าโปรเจคพี่ (128×160)
- ใช้ U8g2 full-frame buffer (`_F_`) → วาดทั้งหมดใน buffer แล้วส่งทีเดียว (แทน Sprite ของ LovyanGFX)
- ใช้ LovyanGFX ไม่ได้บน ESP32-C3 เพราะ RISC-V GPIO struct ต่างจาก Xtensa (ESP32-S3)
