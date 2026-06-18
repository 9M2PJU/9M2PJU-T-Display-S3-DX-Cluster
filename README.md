# 9M2PJU T-Display S3 DX Cluster

Firmware project for a LilyGO T-Display S3 / ESP32-S3 DX cluster display.

The goal is to connect the ESP32-S3 to the 9M2PJU DX cluster over telnet, log in with an amateur radio callsign, and show live DX spots on the built-in display. The main screen is intended to focus on DX spot information, with a fixed UTC clock at the top right and subtle animation for connection activity and new spots.

## Target Hardware

- LilyGO T-Display S3, ESP32-S3 based
- Built-in landscape TFT display, normally 320 x 170
- USB-C cable for flashing and serial monitor
- Wi-Fi network with internet access

## DX Cluster

Default cluster settings are prepared in the config template:

- Host: `9m2pju.hamradio.my`
- Port: `7300`
- Protocol: telnet
- Login: amateur radio callsign

## Repository Status

This repository currently contains the project configuration template. The firmware source will be added next.

Current files:

- `include/config.example.h`: safe example config for Wi-Fi, cluster login, display layout, UTC clock, and UI settings
- `.gitignore`: keeps private credentials and build output out of git

Planned firmware files:

- `platformio.ini`
- `src/main.cpp`
- display renderer
- telnet client
- DX spot parser
- UTC/NTP clock handling

## Configuration

Create your private config file from the example:

```sh
cp include/config.example.h include/config.h
```

Edit `include/config.h` and set your Wi-Fi credentials:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

Set the callsign used to log in to the DX cluster:

```cpp
#define DX_CLUSTER_LOGIN_CALLSIGN "9M2PJU"
```

The private file `include/config.h` is ignored by git, so your Wi-Fi password should not be committed.

## Important Config Values

Wi-Fi:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

DX cluster:

```cpp
#define DX_CLUSTER_HOST "9m2pju.hamradio.my"
#define DX_CLUSTER_PORT 7300
#define DX_CLUSTER_LOGIN_CALLSIGN "9M2PJU"
```

UTC clock:

```cpp
#define NTP_SERVER_PRIMARY "pool.ntp.org"
#define NTP_SERVER_SECONDARY "time.google.com"
#define CLOCK_TIMEZONE_OFFSET_SECONDS 0
#define CLOCK_DAYLIGHT_OFFSET_SECONDS 0
```

Display:

```cpp
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 170
#define DISPLAY_ROTATION 1
#define DISPLAY_BACKLIGHT_BRIGHTNESS 220
```

## Intended Display Layout

The screen should prioritize DX spots, with a compact status bar above them:

```text
+------------------------------+
| 9M2PJU DX Cluster     12:34Z |
+------------------------------+
| 14.074  JA1ABC  FT8          |
| by 9M2XYZ  3 min ago         |
|                              |
| 21.300  VK2DEF  SSB          |
| CQ DX, strong signal         |
|                              |
|  7.025  DL1AAA  CW           |
| Europe opening               |
+------------------------------+
```

The UTC clock should stay fixed at the top right. Animation should support the display, not compete with the DX data.

Good animation ideas:

- subtle signal sweep in the header
- small connection pulse while telnet is active
- slide or fade for a new DX spot
- short highlight on newly received spots
- band color accents for quick scanning

## Build And Upload

The firmware is intended to use PlatformIO with the Arduino framework.

After `platformio.ini` and `src/main.cpp` are added, install PlatformIO and run:

```sh
pio run
```

Upload to the T-Display S3:

```sh
pio run --target upload
```

Open the serial monitor:

```sh
pio device monitor
```

If the board is not detected, hold the BOOT button while plugging in USB-C, then try the upload again.

## Expected Runtime Flow

1. Boot the ESP32-S3.
2. Connect to Wi-Fi.
3. Sync UTC time from NTP.
4. Open telnet connection to `9m2pju.hamradio.my:7300`.
5. Send the configured callsign when the cluster asks for login.
6. Read incoming cluster lines.
7. Parse DX spot data.
8. Keep the newest spots on screen.
9. Reconnect automatically if Wi-Fi or telnet drops.

## Safety Notes

- Do not commit `include/config.h`.
- Do not put real Wi-Fi passwords into `include/config.example.h`.
- Use UTC for the display clock, not local time, so radio logs and DX spots stay consistent.

## Development Notes

Recommended implementation approach:

- Use non-blocking Wi-Fi and telnet handling.
- Keep display rendering separate from telnet parsing.
- Store recent DX spots in a small fixed-size ring buffer.
- Redraw the clock once per second.
- Run animation at about 30 FPS, using `ANIMATION_FRAME_MS`.
- Avoid blocking `delay()` calls in the main loop except very short timing yields.
