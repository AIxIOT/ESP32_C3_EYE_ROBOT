#include "stopwatch_mode.h"
#include "display.h"

enum StopwatchState {
  SW_IDLE,
  SW_RUNNING,
  SW_STOPPED
};

static StopwatchState swState = SW_IDLE;
static unsigned long startTime = 0;
static unsigned long elapsedTime = 0;

void stopwatch_mode_init() {
  swState = SW_IDLE;
  startTime = 0;
  elapsedTime = 0;
}

void stopwatch_mode_click() {
  if (swState == SW_IDLE || swState == SW_STOPPED) {
    // Reset and Start
    startTime = millis();
    swState = SW_RUNNING;
  } else if (swState == SW_RUNNING) {
    // Stop
    elapsedTime = millis() - startTime;
    swState = SW_STOPPED;
  }
}

void stopwatch_mode_update() {
  unsigned long currentElapsed = 0;
  
  if (swState == SW_IDLE) {
    currentElapsed = 0;
  } else if (swState == SW_RUNNING) {
    currentElapsed = millis() - startTime;
  } else if (swState == SW_STOPPED) {
    currentElapsed = elapsedTime;
  }

  // Calculate Minutes and Seconds
  unsigned int mm = (currentElapsed / 60000) % 60;
  unsigned int ss = (currentElapsed / 1000) % 60;

  char timeStr[10];
  sprintf(timeStr, "%02u:%02u", mm, ss);

  oled.clearBuffer();
  
  // Use logisoso32 for the biggest MM:SS format that fits in 128x64
  oled.setFont(u8g2_font_logisoso32_tn);
  
  int tw = oled.getStrWidth(timeStr);
  int x = (SCREEN_W - tw) / 2;
  // Center vertically: font height is ~32px, screen is 64px.
  // Y baseline 48 is roughly center.
  int y = 48; 
  
  oled.drawStr(x, y, timeStr);
  
  oled.sendBuffer();
}
