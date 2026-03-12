#include "input.h"

static const unsigned long DEBOUNCE_MS = 50;

// GPIO10
static bool _actionPressed = false;
static bool _actionLast = HIGH;
static unsigned long _actionDebounce = 0;

// GPIO0
static bool _modePressed = false;
static bool _modeLast = HIGH;
static unsigned long _modeDebounce = 0;

void input_init() {
  pinMode(BTN_ACTION_PIN, INPUT_PULLUP);
  pinMode(BTN_MODE_PIN, INPUT_PULLUP);
}

void input_update() {
  _actionPressed = false;
  _modePressed = false;

  // ACTION (GPIO10)
  bool curA = digitalRead(BTN_ACTION_PIN);
  if (_actionLast == HIGH && curA == LOW && (millis() - _actionDebounce > DEBOUNCE_MS)) {
    _actionPressed = true;
    _actionDebounce = millis();
  }
  _actionLast = curA;

  // MODE (GPIO0)
  bool curM = digitalRead(BTN_MODE_PIN);
  if (_modeLast == HIGH && curM == LOW && (millis() - _modeDebounce > DEBOUNCE_MS)) {
    _modePressed = true;
    _modeDebounce = millis();
  }
  _modeLast = curM;
}

bool input_btn_action() { return _actionPressed; }
bool input_btn_mode()   { return _modePressed; }
