# 📋 กฎการทำงาน (Core Rules)

### 💬 ด้านการสื่อสาร (Communication)
- **ภาษา**: ตอบเป็นภาษาไทยเสมอ
- **ความชัดเจน**: หากไม่เข้าใจคำสั่งหรือมีจุดที่คลุมเครือ ให้สอบถามผู้ใช้ก่อนเริ่มงานเสมอ
- **ความกระชับ**: หลีกเลี่ยงการสร้างหน้าต่าง/แท็บ Task List, Implementation Plan, และ Walkthrough โดยไม่จำเป็น เพื่อลดความซับซ้อน

### 🚀 ขั้นตอนการเริ่มโปรเจกต์ใหม่ (Bootstrap Procedure)
**หากพบบรรทัดนี้ในโปรเจกต์ที่ยังไม่มีโฟลเดอร์ `.agent` หรือไฟล์ `.agent/project.md` ให้ดำเนินการดังนี้ทันที:**
1. **Initialize Git**: รันคำสั่งเช็คและติดตั้ง `git init` และสร้าง `.gitignore` สำหรับ PlatformIO
2. **Setup Environment**:
    - หากพบไฟล์ `hardware.md`: ให้อ่านข้อมูลจากไฟล์นั้นเพื่อนำมาเป็นฐานข้อมูลเริ่มต้น
    - หากไม่มีไฟล์ Spec: ให้สอบถามผู้ใช้เกี่ยวกับรุ่นบอร์ดและรายละเอียดฮาร์ดแวร์
3. **Create Workflows**: สร้างโฟลเดอร์ `.agent/workflows/` และไฟล์คำสั่ง (upload/monitor)
4. **Generate project.md**: สร้างไฟล์ `.agent/project.md` โดยรวบรวมข้อมูลจาก `hardware.md` (ถ้ามี) และข้อมูลเฉพาะของโปรเจกต์ปัจจุบัน
5. **Clean up**: แจ้งผู้ใช้ว่าระบบพร้อมทำงานแล้ว

### 🤖 การทำงานอัตโนมัติ (Automation & Commands)
- **Command Preparation ONLY**: ห้ามรันคำสั่ง `run_command` (เช่น pio run, monitor) ผ่านระบบเอเจนท์โดยตรง
- **Manual Execution**: ให้เตรียมชุดคำสั่ง (Command Block) ที่ถูกต้องมาวางในแชท เพื่อให้ผู้ใช้ก๊อปปี้ไปวางใน Terminal ของตัวเองเท่านั้น
- **Slash Commands**:
    - `/upload`: ให้พิมพ์คำสั่ง `pio run -t upload`
    - `/monitor`: ให้พิมพ์คำสั่ง `pio device monitor -b 115200`
    - `/flash_monitor`: ให้พิมพ์คำสั่ง `pio run -t upload -t monitor`
- **Workflow**: หลังผู้ใช้รันเสร็จและส่งผลลัพธ์ (Output) กลับมา ให้วิเคราะห์ผลและดำเนินการตามกฎข้ออื่นๆ ต่อไป

### 💻 ด้านการจัดการโปรเจกต์ (Project Management)
- **Git & Ignore**: ตรวจสอบและติดตั้ง `git init` และสร้างไฟล์ `.gitignore` ที่เหมาะสมสำหรับ PlatformIO เสมอ โดยต้องครอบคลุม:
    - `.pio/`, `.pioenvs/`, `.piolibdeps/` (Build & Libs)
    - `.vscode/` (Editor config)
    - `*.bin`, `*.hex`, `*.elf` (Firmware files)
- **สภาพแวดล้อม**: ตรวจสอบเสมอว่าโปรเจกต์รันบน PlatformIO หรือไม่ (เช็คไฟล์ `platformio.ini`)
- **การบันทึกค่า**: หากมีการแก้ไขการตั้งค่า (Settings) หรือคอนฟิกูเรชันต่างๆ ให้บันทึกข้อมูลเหล่านั้นลงในไฟล์ `project.md` เสมอ

### 🛠️ ด้านโครงสร้างโค้ดและรูปแบบ (Code Structure & Style)
- **โครงสร้างไฟล์หลัก**: 
    - `main.cpp`: ไฟล์ควบคุมหลัก (จัดการโหมด Eye/Clock)
    - `module_name.cpp`: บรรจุ Logic การทำงาน (เช่น `wifi_module.cpp`)
    - `module_name.h`: บรรจุค่า Config และการกำหนดขา Pin ต่างๆ (เช่น `wifi_module.h`)
- **Header Guards**: ใช้ `#pragma once` แทนการใช้ `#ifndef`, `#define`, `#endif`
- **Naming Convention**: 
    - **ชื่อไฟล์/ตัวแปร/ฟังก์ชัน**: ใช้ `snake_case` (เช่น `my_variable`, `get_data()`)
    - **ค่าคงที่ (Constants/Macros)**: ใช้ `UPPER_SNAKE_CASE`

### 🔌 ด้านเทคนิคและการพัฒนา (Technical & Development)
- **Serial Debugging**: เน้นการแสดง Error และ Status ผ่าน Serial Monitor อย่างชัดเจน เพื่อความสะดวกในการวิเคราะห์ปัญหา
- **Non-blocking Code**: หลีกเลี่ยงการใช้ `delay()` ในลอจิกหลัก ให้ใช้เทคนิค `millis()` หรือ Timer เพื่อให้โปรเจกต์ทำงานได้หลาย Module พร้อมกัน
- **Pin Documentation**: ในไฟล์ `.h` ของแต่ละ Module ต้องเขียนคอมเมนต์ระบุหน้าที่ของ Pin และประเภทการเชื่อมต่อ (In/Out/Pull) ให้ชัดเจนเสมอ
- **Screen Boundary**: วาดอะไรบนจอ OLED ต้อง Clamp ค่าไม่ให้ล้นขอบจอ (0-63 px แนวตั้ง, 0-127 px แนวนอน)

### 🛠️ ด้านการเขียนโค้ด (Coding)
- **แก้ไขด่วน**: สำหรับการแก้ไขโค้ดที่สั้นและไม่ซับซ้อน สามารถดำเนินการได้ทันทีโดยไม่ต้องรอให้ผู้ใช้ตรวจสอบก่อน
- **ความถูกต้อง**: ตรวจสอบความถูกต้องของซอร์สโค้ดและโครงสร้างไฟล์ให้สอดคล้องกับสภาพแวดล้อมที่ใช้งานอยู่เสมอ
- **Single Button Pattern**: ปุ่มเดียว (GPIO 10) ทำหลายหน้าที่ — กดสั้น vs กดแช่ 1 วิ
