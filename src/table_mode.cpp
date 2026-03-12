#include "table_mode.h"
#include "display.h"
#include <Arduino.h>

struct MockStore {
  const char* name;
  int offlineDays;
  int offlineHr;
};

static const int MOCK_COUNT = 6;
static MockStore mockStores[MOCK_COUNT] = {
  {"STORE-A-BKK", 1, 5},
  {"STORE-B-CNX", 0, 12},
  {"STORE-C-PYO", 3, 0},
  {"STORE-D-BKK", 0, 2},
  {"STORE-E-BKK", 0, 1},
  {"STORE-F-BKK", 0, 8}
};

static int scrollOffset = 0;
static const int ROW_HEIGHT = 16;  // ความสูงแต่ละแถว
static const int HEADER_H = 14;   
static int visibleRows = 3;       // OLED 64px - 14px header = 50px ... 50/16 ~ 3 แถว

void table_mode_init() {
  scrollOffset = 0;
}

void table_mode_scroll_down() {
  if (MOCK_COUNT <= visibleRows) {
    scrollOffset = 0;
  } else {
    scrollOffset++;
    if (scrollOffset > MOCK_COUNT - visibleRows) {
      scrollOffset = 0; // วนกลับ
    }
  }
}

void table_mode_update() {
  oled.clearBuffer();
  
  // Header
  oled.setFont(u8g2_font_5x7_tr);
  oled.setCursor(0, 10);
  oled.print("OFFLINE: ");
  oled.print(MOCK_COUNT);
  
  oled.drawHLine(0, HEADER_H, 128);

  // Table rows
  int y = HEADER_H + 2;
  for (int i = 0; i < visibleRows && (i + scrollOffset) < MOCK_COUNT; i++) {
    MockStore& s = mockStores[i + scrollOffset];
    int rowY = y + i * ROW_HEIGHT;
    
    // ชื่อ Store
    oled.setCursor(2, rowY + 7);
    oled.print(s.name);
    
    // เวลา Offline (ขวาสุด)
    char statusBuf[16];
    if (s.offlineDays > 0) {
      sprintf(statusBuf, "%dd", s.offlineDays);
    } else {
      sprintf(statusBuf, "%dh", s.offlineHr);
    }
    // คำนวณความกว้างข้อความคร่าวๆ 
    int w = oled.getStrWidth(statusBuf);
    oled.setCursor(128 - w - 2, rowY + 7);
    oled.print(statusBuf);
    
    // เส้นแบ่งแถว
    oled.drawHLine(0, rowY + ROW_HEIGHT - 2, 128);
  }

  // Footer / Scroll indicator
  if (MOCK_COUNT > visibleRows) {
    int page = scrollOffset / visibleRows + 1;
    int totalPages = (MOCK_COUNT + visibleRows - 1) / visibleRows;
    oled.setCursor(50, 62);
    oled.printf("%d/%d", page, totalPages);
  }

  oled.sendBuffer();
}
