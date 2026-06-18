#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <cctype>
#include <cstring>
#include <time.h>

#if __has_include("config.h")
#include "config.h"
#else
#error "Missing include/config.h. Copy include/config.example.h to include/config.h and edit your Wi-Fi settings."
#endif

#ifndef DX_CLUSTER_POST_LOGIN_COMMAND
#define DX_CLUSTER_POST_LOGIN_COMMAND ""
#endif

// LilyGO T-Display-S3 1.9-inch ST7789 backlight pin.
// LCD parallel bus pins are configured in platformio.ini using LilyGO's official setup.
constexpr int LCD_BL = 38;

TFT_eSPI tft;

WiFiClient clusterClient;

struct DxSpot {
  String spotter;
  String frequency;
  String dxCall;
  String comment;
  time_t receivedAt = 0;
  bool valid = false;
};

DxSpot spots[MAX_STORED_DX_SPOTS];
size_t spotCount = 0;

String telnetLine;
uint32_t lastConnectAttemptMs = 0;
uint32_t connectedAtMs = 0;
uint32_t lastReceivedMs = 0;
uint32_t lastClockDrawMs = 0;
uint32_t lastFrameMs = 0;
bool loginSent = false;
bool postLoginSent = false;
bool fullRedrawNeeded = true;
char utcClock[8] = "--:--Z";

uint16_t bandColor(const String &frequency) {
  float mhz = frequency.toFloat();
  if (mhz < 2.0f) return 0xB81F;
  if (mhz < 4.0f) return 0x07FF;
  if (mhz < 8.0f) return 0x07E0;
  if (mhz < 15.0f) return 0xFFE0;
  if (mhz < 22.0f) return 0xFD20;
  if (mhz < 30.0f) return 0xF81F;
  return COLOR_ACCENT;
}

String trimCopy(String value) {
  value.trim();
  return value;
}

String nextToken(const String &text, int &index) {
  while (index < text.length() && isspace(text[index])) {
    index++;
  }

  int start = index;
  while (index < text.length() && !isspace(text[index])) {
    index++;
  }

  return text.substring(start, index);
}

bool parseDxSpot(const String &line, DxSpot &spot) {
  if (!line.startsWith("DX de ")) {
    return false;
  }

  int colon = line.indexOf(':');
  if (colon < 6) {
    return false;
  }

  spot.spotter = trimCopy(line.substring(6, colon));
  int index = colon + 1;
  spot.frequency = nextToken(line, index);
  spot.dxCall = nextToken(line, index);
  spot.comment = trimCopy(line.substring(index));

  if (spot.frequency.isEmpty() || spot.dxCall.isEmpty()) {
    return false;
  }

  spot.receivedAt = time(nullptr);
  spot.valid = true;
  return true;
}

void addSpot(const DxSpot &spot) {
  size_t moveCount = spotCount < (MAX_STORED_DX_SPOTS - 1) ? spotCount : (MAX_STORED_DX_SPOTS - 1);
  for (int i = static_cast<int>(moveCount); i > 0; --i) {
    spots[i] = spots[i - 1];
  }

  spots[0] = spot;
  if (spotCount < MAX_STORED_DX_SPOTS) {
    spotCount++;
  }

  fullRedrawNeeded = true;
}

void sendClusterLogin() {
  if (!clusterClient.connected() || loginSent) {
    return;
  }

  clusterClient.print(DX_CLUSTER_LOGIN_CALLSIGN);
  clusterClient.print("\r\n");
  loginSent = true;
}

void sendPostLoginCommand() {
  if (!clusterClient.connected() || postLoginSent || strlen(DX_CLUSTER_POST_LOGIN_COMMAND) == 0) {
    return;
  }

  clusterClient.print(DX_CLUSTER_POST_LOGIN_COMMAND);
  clusterClient.print("\r\n");
  postLoginSent = true;
}

void handleClusterLine(String line) {
  line.trim();
  if (line.isEmpty()) {
    return;
  }

  Serial.println(line);
  String lower = line;
  lower.toLowerCase();

  if (!loginSent && (lower.indexOf("login") >= 0 || lower.indexOf("call") >= 0 || lower.indexOf("callsign") >= 0)) {
    sendClusterLogin();
    return;
  }

  if (loginSent) {
    sendPostLoginCommand();
  }

  DxSpot spot;
  if (parseDxSpot(line, spot)) {
    addSpot(spot);
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.print("\nWi-Fi connected: ");
  Serial.println(WiFi.localIP());
}

void syncUtcClock() {
  configTime(CLOCK_TIMEZONE_OFFSET_SECONDS, CLOCK_DAYLIGHT_OFFSET_SECONDS, NTP_SERVER_PRIMARY, NTP_SERVER_SECONDARY);
  Serial.print("Syncing UTC clock");

  tm timeInfo;
  for (int i = 0; i < 30; ++i) {
    if (getLocalTime(&timeInfo, 500)) {
      Serial.println("\nUTC clock synced");
      return;
    }
    Serial.print(".");
  }

  Serial.println("\nUTC clock sync timed out; will keep trying in background");
}

void connectCluster() {
  uint32_t now = millis();
  if (clusterClient.connected() || now - lastConnectAttemptMs < TELNET_RECONNECT_DELAY_MS) {
    return;
  }

  lastConnectAttemptMs = now;
  loginSent = false;
  postLoginSent = false;
  telnetLine = "";

  Serial.printf("Connecting telnet %s:%d\n", DX_CLUSTER_HOST, DX_CLUSTER_PORT);
  if (clusterClient.connect(DX_CLUSTER_HOST, DX_CLUSTER_PORT)) {
    connectedAtMs = now;
    lastReceivedMs = now;
    Serial.println("DX cluster connected");
    fullRedrawNeeded = true;
  } else {
    Serial.println("DX cluster connect failed");
  }
}

void readCluster() {
  if (!clusterClient.connected()) {
    return;
  }

  while (clusterClient.available()) {
    char c = static_cast<char>(clusterClient.read());
    lastReceivedMs = millis();

    if (c == '\n') {
      handleClusterLine(telnetLine);
      telnetLine = "";
    } else if (c != '\r' && telnetLine.length() < 240) {
      telnetLine += c;
    }
  }

  if (!loginSent && millis() - connectedAtMs > 2500) {
    sendClusterLogin();
  }
}

void updateClockText() {
  tm timeInfo;
  if (getLocalTime(&timeInfo, 10)) {
    snprintf(utcClock, sizeof(utcClock), "%02d:%02dZ", timeInfo.tm_hour, timeInfo.tm_min);
  }
}

void drawHeader(uint32_t now) {
  tft.fillRect(0, 0, DISPLAY_WIDTH, 28, COLOR_BACKGROUND);
  tft.drawFastHLine(0, 27, DISPLAY_WIDTH, COLOR_GRID);

  int sweep = (now / 18) % DISPLAY_WIDTH;
  int sweepStart = sweep > 28 ? sweep - 28 : 0;
  int sweepWidth = sweep > 28 ? 28 : sweep;
  if (sweepWidth > 0) {
    tft.drawFastHLine(sweepStart, 26, sweepWidth, COLOR_ACCENT);
  }

  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT_SECONDARY);
  tft.setCursor(8, 6);
  tft.print("9M2PJU DX Cluster");

  uint16_t statusColor = clusterClient.connected() ? COLOR_ACCENT : COLOR_ALERT;
  int pulse = clusterClient.connected() ? 2 + ((now / 300) % 3) : 2;
  tft.fillCircle(118, 10, pulse, statusColor);

  tft.setTextColor(COLOR_TEXT_PRIMARY);
  tft.setCursor(DISPLAY_WIDTH - 47, 6);
  tft.print(utcClock);
}

void drawSpot(const DxSpot &spot, int y, bool fresh) {
  uint16_t accent = bandColor(spot.frequency);
  tft.fillRect(0, y, DISPLAY_WIDTH, 33, fresh ? COLOR_PANEL : COLOR_BACKGROUND);
  tft.fillRect(4, y + 3, 3, 26, accent);

  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT_PRIMARY);
  tft.setCursor(12, y + 4);
  tft.print(spot.frequency);

  tft.setTextColor(accent);
  tft.setCursor(82, y + 4);
  tft.print(spot.dxCall);

  tft.setTextColor(COLOR_TEXT_SECONDARY);
  tft.setCursor(12, y + 17);
  tft.print("de ");
  tft.print(spot.spotter);

  String comment = spot.comment;
  if (comment.length() > 26) {
    comment = comment.substring(0, 25) + ".";
  }

  tft.setCursor(132, y + 17);
  tft.print(comment);
}

void drawEmptyState() {
  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT_SECONDARY);
  tft.setCursor(26, 72);
  tft.print(clusterClient.connected() ? "Waiting for DX spots..." : "Connecting to DX cluster...");
}

void drawScreen() {
  uint32_t now = millis();
  if (!fullRedrawNeeded && now - lastFrameMs < ANIMATION_FRAME_MS) {
    return;
  }

  lastFrameMs = now;
  tft.fillScreen(COLOR_BACKGROUND);
  drawHeader(now);

  if (spotCount == 0) {
    drawEmptyState();
    fullRedrawNeeded = false;
    return;
  }

  size_t visible = spotCount < MAX_VISIBLE_DX_SPOTS ? spotCount : MAX_VISIBLE_DX_SPOTS;
  for (size_t i = 0; i < visible; ++i) {
    bool fresh = spots[i].valid && (time(nullptr) - spots[i].receivedAt) * 1000 < NEW_SPOT_HIGHLIGHT_MS;
    drawSpot(spots[i], 34 + static_cast<int>(i) * 34, fresh);
  }

  fullRedrawNeeded = false;
}

void setupDisplay() {
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, DISPLAY_BACKLIGHT_BRIGHTNESS > 0 ? HIGH : LOW);

  tft.init();
  tft.setRotation(DISPLAY_ROTATION);
  tft.fillScreen(COLOR_BACKGROUND);
  tft.setTextWrap(false);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT_PRIMARY);
  tft.setCursor(20, 70);
  tft.print("9M2PJU DX Cluster");
}

void setup() {
  Serial.begin(115200);
  delay(300);

  setupDisplay();
  connectWiFi();
  syncUtcClock();
  updateClockText();
  fullRedrawNeeded = true;
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  connectCluster();
  readCluster();

  if (millis() - lastClockDrawMs >= CLOCK_REFRESH_MS) {
    lastClockDrawMs = millis();
    updateClockText();
    fullRedrawNeeded = true;
  }

  drawScreen();
  delay(2);
}
