#pragma once
#include <Arduino.h>

#define INPUT_PIN 10 // ปุ่มเดียวทำทุกอย่าง (GPIO 10)

void input_init();
void input_update();
bool input_btn_short_press();
bool input_btn_long_press();
