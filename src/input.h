#pragma once
#include <Arduino.h>

#define BTN_ACTION_PIN 10 // กดเปลี่ยนอารมณ์ / เลื่อนตาราง
#define BTN_MODE_PIN   0  // กดสลับโหมด Eye ↔ Table

void input_init();
void input_update();
bool input_btn_action();
bool input_btn_mode();
