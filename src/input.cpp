#include "input.h"

static const unsigned long DEBOUNCE_MS = 50;
static const unsigned long LONG_PRESS_MS = 2000; // แช่ 2 วิ = เปลี่ยนโหมด

static bool _shortPressed = false;
static bool _longPressed = false;

static bool _lastState = LOW;       // Touch Pad ปกติ = LOW
static unsigned long _pressStartTime = 0;
static bool _isPressing = false;
static bool _longPressHandled = false;

void input_init() {
  pinMode(INPUT_PIN, INPUT);  // Touch Pad ไม่ต้อง PULLUP
}

void input_update() {
  _shortPressed = false;
  _longPressed = false;

  bool currentState = digitalRead(INPUT_PIN);

  if (_lastState == LOW && currentState == HIGH) { // เริ่มแตะ (LOW → HIGH)
    _pressStartTime = millis();
    _isPressing = true;
    _longPressHandled = false;
  } 
  else if (_lastState == HIGH && currentState == LOW) { // ปล่อย (HIGH → LOW)
    if (_isPressing) {
      unsigned long duration = millis() - _pressStartTime;
      if (duration >= DEBOUNCE_MS && !_longPressHandled) {
        _shortPressed = true;  // กดสั้น (ไม่ถึง 2 วิ)
      }
      _isPressing = false;
    }
  }

  // เช็ค Long Press ระหว่างที่ยังแตะอยู่ (ไม่ต้องรอปล่อย)
  if (_isPressing && !_longPressHandled) {
    if (millis() - _pressStartTime >= LONG_PRESS_MS) {
      _longPressed = true;
      _longPressHandled = true; // ป้องกันยิงซ้ำ
    }
  }

  _lastState = currentState;
}

bool input_btn_short_press() { return _shortPressed; }
bool input_btn_long_press()  { return _longPressed; }
