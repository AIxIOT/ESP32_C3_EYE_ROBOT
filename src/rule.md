# Project Rules

## File Naming
- ชื่อไฟล์ธรรมดา ไม่มี prefix
  - ✅ `wifi_module.h` / `wifi_module.cpp`
  - ✅ `pump_module.h` / `pump_module.cpp`
- ไฟล์หลัก: `.ino`

## Code Style
- ภาษา: C++ (Arduino framework)
- ตั้งชื่อตัวแปร/ฟังก์ชัน: `camelCase`
- ตั้งชื่อ constant: `UPPER_SNAKE_CASE`
- แต่ละ module แยกเป็น `.h` + `.cpp` คู่กัน
- `.h` ทุกไฟล์ใช้ `#pragma once` แทน `#ifndef` guard
- `.h` เก็บเฉพาะ pin definitions + config constants
- `.cpp` เก็บ logic จริง

## General
- ห้ามเขียนโค้ดโดยไม่มี project.md อ้างอิง
- ห้าม hardcode ค่าที่ควรเป็น constant ใน logic
- comment เฉพาะจุดที่ไม่ self-evident
