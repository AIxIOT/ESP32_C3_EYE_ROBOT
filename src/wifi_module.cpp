#include "wifi_module.h"
#include <WiFi.h>
#include <time.h>
#include "display.h"

// NTP Server settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600; // GMT+7 for Thailand
const int   daylightOffset_sec = 0;

static char timeString[16] = "--:--";
static char statusString[16] = "DISCONNECTED";
static unsigned long lastTimeCheck = 0;
static unsigned long lastWifiCheck = 0;

void wifi_init() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  Serial.print("Connecting to WiFi");
  int dotCount = 0;
  char statusMsg[32];
  
  while (WiFi.status() != WL_CONNECTED) {
    dotCount = (dotCount + 1) % 4;
    strcpy(statusMsg, "Connecting");
    for(int i=0; i<dotCount; i++) strcat(statusMsg, ".");
    
    display_show_wifi_status(WIFI_SSID, statusMsg);
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  display_show_wifi_status(WIFI_SSID, "CONNECTED!");
  delay(1000); 

  // Initialize NTP time
  display_show_wifi_status(WIFI_SSID, "SYNC TIME...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Wait for time to be set with a timeout
  struct tm timeinfo;
  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 10) {
    display_show_wifi_status(WIFI_SSID, "SYNC TIME...");
    Serial.print(".");
    delay(500);
    retry++;
  }
  
  if (retry < 10) {
    Serial.println("\nTime Synced!");
    strcpy(statusString, "WIFI OK");
  } else {
    Serial.println("\nTime Sync Failed!");
    strcpy(statusString, "TIME FAILED");
  }
}

void wifi_update() {
  unsigned long currentMillis = millis();

  // Check WiFi status every 5 seconds
  if (currentMillis - lastWifiCheck >= 5000) {
    lastWifiCheck = currentMillis;
    if (WiFi.status() != WL_CONNECTED) {
      strcpy(statusString, "NO WIFI");
      // Optional: attempt reconnect here if needed in a non-blocking way
      // WiFi.reconnect(); 
    } else {
      strcpy(statusString, "WIFI OK");
    }
  }

  // Update time cache every 1 second
  if (currentMillis - lastTimeCheck >= 1000) {
    lastTimeCheck = currentMillis;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      sprintf(timeString, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    } else {
      strcpy(timeString, "--:--");
    }
  }
}

bool wifi_is_connected() {
  return WiFi.status() == WL_CONNECTED;
}

const char* wifi_get_time_string() {
  return timeString;
}

const char* wifi_get_status_string() {
  return statusString;
}
