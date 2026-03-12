#pragma once
#include <Arduino.h>

// --- WiFi Config ---
#define WIFI_SSID "omgdigital_4064"
#define WIFI_PASS "60F&003ii4@4"

void wifi_init();
void wifi_update();
bool wifi_is_connected();
const char* wifi_get_time_string();
const char* wifi_get_status_string();
