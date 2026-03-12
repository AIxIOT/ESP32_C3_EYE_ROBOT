#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// --- Pin Definitions ---
static const int PIN_SCL = 21;
static const int PIN_SDA = 20;

// --- Display Config ---
static const int SCREEN_W = 128;
static const int SCREEN_H = 64;

// SH1106 1.3" OLED — I2C, full frame buffer
// U8G2_SH1106_128X64_NONAME_F_HW_I2C ใช้ hardware I2C อัตโนมัติ
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C oled;

void display_init();
void display_clear();
void display_send();
void display_show_wifi_status(const char* ssid, const char* status);
