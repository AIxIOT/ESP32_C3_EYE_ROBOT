#pragma once
#include <Arduino.h>

// --- WiFi Config ---
#define WIFI_SSID "Digitalnatives3"
#define WIFI_PASS "BC202$1$9@"

void wifi_init();
void wifi_update();
bool wifi_is_connected();
const char* wifi_get_time_string();
const char* wifi_get_status_string();
int wifi_get_hour();    // -1 ถ้ายังไม่ได้ sync เวลา
int wifi_get_minute();  // -1 ถ้ายังไม่ได้ sync เวลา
