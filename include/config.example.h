#pragma once

/*
 * Copy this file to include/config.h and fill in your private values.
 * Keep include/config.h out of git because it contains Wi-Fi credentials.
 */

// Wi-Fi
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// DX cluster telnet login
#define DX_CLUSTER_HOST "9m2pju.hamradio.my"
#define DX_CLUSTER_PORT 7300
#define DX_CLUSTER_LOGIN_CALLSIGN "9M2PJU"

// Optional command sent after login. Leave empty if the cluster does not need it.
#define DX_CLUSTER_POST_LOGIN_COMMAND ""

// UTC clock / NTP
#define NTP_SERVER_PRIMARY "pool.ntp.org"
#define NTP_SERVER_SECONDARY "time.google.com"
#define CLOCK_TIMEZONE_OFFSET_SECONDS 0
#define CLOCK_DAYLIGHT_OFFSET_SECONDS 0

// Display layout for LilyGO T-Display S3, normally 320x170 in landscape.
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 170
#define DISPLAY_ROTATION 1
#define DISPLAY_BACKLIGHT_BRIGHTNESS 220

// UI behavior
#define MAX_VISIBLE_DX_SPOTS 4
#define MAX_STORED_DX_SPOTS 12
#define NEW_SPOT_HIGHLIGHT_MS 5000
#define TELNET_RECONNECT_DELAY_MS 8000
#define CLOCK_REFRESH_MS 1000
#define ANIMATION_FRAME_MS 33

// Theme colors in RGB565 format. These can be tuned later once the UI is built.
#define COLOR_BACKGROUND 0x0008
#define COLOR_PANEL 0x0841
#define COLOR_TEXT_PRIMARY 0xFFFF
#define COLOR_TEXT_SECONDARY 0xBDF7
#define COLOR_ACCENT 0x07FF
#define COLOR_ALERT 0xFD20
#define COLOR_GRID 0x2104
