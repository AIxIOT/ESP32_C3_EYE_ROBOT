# ESP32-C3 EYE ROBOT — Project Document

## Overview
โปรเจคแสดง **Eye Animation** บนจอ OLED 1.3" ผ่าน ESP32-C3 Mini  
มี 4 โหมดหลัก: **โหมดดวงตา (Eye)**, **โหมดนาฬิกา (Clock)**, **โหมดจับเวลา (Stopwatch)**, และ **โหมดไดโนเสาร์ (Dino Run)**  
ควบคุมด้วยปุ่มเดียว (GPIO 10) — กดสั้นเปลี่ยนอารมณ์/กดทำแอ็คชั่น กดแช่ 1 วิเปลี่ยนโหมด

---

## Hardware

### Microcontroller: ESP32-C3 Super Mini
- **Core:** RISC-V Single-Core 160MHz
- **Flash:** 4MB
- **SRAM:** 400KB
- **USB CDC On Boot:** Enabled

### Display: 1.3" OLED (Monochrome White)
- **Driver IC:** SH1106
- **Interface:** I2C
- **Resolution:** 128 × 64
- **I2C Address:** 0x3C
- **Rotation:** U8G2_R2 (กลับหัว 180°)

### Pinout

| Function       | GPIO | Notes |
|----------------|------|-------|
| SCL            | 21   | Hardware I2C (Wire) |
| SDA            | 20   | Hardware I2C (Wire) |
| INPUT (Button) | 10   | ปุ่มเดียว: กดสั้น = เปลี่ยนอารมณ์, กดแช่ 1 วิ = เปลี่ยนโหมด |

---

## Features

### App Modes (4 โหมด)
| Mode       | คำอธิบาย                              | สลับโดย |
|------------|---------------------------------------|---------|
| EYE        | แสดงดวงตา animation 5 อารมณ์          | กดแช่ 1 วิ |
| CLOCK      | แสดงเวลาตัวใหญ่ + สถานะ WiFi          | กดแช่ 1 วิ |
| STOPWATCH  | จับเวลาตัวใหญ่ (นาที:วินาที) กดเริ่ม/หยุด | กดแช่ 1 วิ |
| DINO       | มินิเกมกระโดดข้ามกระบองเพชร           | กดแช่ 1 วิ |

### Eye Emotions (5 แบบ)
| Emotion  | ลักษณะ                            |
|----------|-----------------------------------|
| NORMAL   | กรอกตาไปมา + กระพริบอัตโนมัติ      |
| HAPPY    | ตาโค้งขึ้น ^_^ + เด้งๆ             |
| ANGRY    | ตาเฉียงดุ + สั่นเบาๆ              |
| CRAZY    | สั่นหนัก + ขนาดเปลี่ยน            |
| SLEEPY   | ค่อยๆ หลับ + ผงกหัว + z z z       |

### Input (ปุ่มเดียว GPIO 10)
- **กดสั้น (Short Press):** เปลี่ยน Emotion ในโหมดตา
- **กดแช่ 1 วิ (Long Press):** สลับโหมด Eye ↔ Clock

### WiFi & NTP
- เชื่อมต่อ WiFi อัตโนมัติตอนเปิดเครื่อง
- **Timeout 10 วินาที** — ถ้าต่อไม่ได้จะข้ามเข้าโหมดตาเลย
- หน้า WiFi แสดงชื่อไวไฟตัวใหญ่ + วงกลม Spinner หมุน
- Sync เวลาผ่าน NTP (pool.ntp.org, GMT+7)

---

## Module Structure

```
EYE_ROBOT_01/
├── src/
│   ├── main.cpp         ← ไฟล์หลัก (setup/loop, จัดการโหมด)
│   ├── display.h        ← U8g2 config สำหรับ SH1106 I2C (GPIO 20, 21)
│   ├── display.cpp      ← display_init() + WiFi splash screen (spinner)
│   ├── eye_mode.h       ← EyeEmotion enum + API
│   ├── eye_mode.cpp     ← logic วาดตาทั้ง 5 emotion (ตาโค้งมน, clamp ขอบจอ)
│   ├── clock_mode.h     ← API โหมดนาฬิกา
│   ├── clock_mode.cpp   ← โหมดนาฬิกา และการแสดงสถานะ WiFi
│   ├── stopwatch_mode.h ← API โหมดจับเวลา
│   ├── stopwatch_mode.cpp ← โหมดจับเวลา (ตัวเลขขนาดใหญ่)
│   ├── dino_mode.h      ← API โหมดเกมไดโนเสาร์
│   ├── dino_mode.cpp    ← logic เกมไดโนเสาร์ (กระโดดหลบสิ่งกีดขวาง)
│   ├── input.h          ← Input API (short/long press)
│   ├── input.cpp        ← Single button logic (GPIO 10)
│   ├── wifi_module.h    ← WiFi config + NTP API
│   └── wifi_module.cpp  ← WiFi connect (10s timeout) + time sync
├── project.md           ← เอกสารโปรเจค (ไฟล์นี้)
├── hardware.md          ← สเปคบอร์ดและการเชื่อมต่อ
└── rule.md              ← กฎการเขียนโค้ด
```

---

## Library Dependencies
- **U8g2** (by olikraus) — รองรับ SH1106 I2C + ESP32-C3 RISC-V

---

## Notes
- จอ OLED เป็น monochrome → ใช้ขาว/ดำเท่านั้น
- ตาออกแบบให้ใหญ่ (24x36 px) + ใช้ drawRBox วาดโค้งมน
- fillRoundRect มี Clamp ป้องกันตาล้นขอบจอ (0-63 px)
- ใช้ U8g2 full-frame buffer (`_F_`) → วาดทั้งหมดใน buffer แล้วส่งทีเดียว
- ใช้ LovyanGFX ไม่ได้บน ESP32-C3 เพราะ RISC-V GPIO struct ต่างจาก Xtensa
