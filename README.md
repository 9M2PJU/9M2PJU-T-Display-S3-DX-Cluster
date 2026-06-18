# 9M2PJU T-Display S3 DX Cluster

ESP32-S3 firmware for the LilyGO T-Display-S3 development board with the 1.9 inch ST7789 LCD. It connects to the 9M2PJU DX cluster by telnet and shows live DX spots on the built-in display.

The display keeps DX spot information as the main focus, with a fixed UTC clock at the top right and subtle animation for connection activity and new spots.

## Quick Start

1. Clone the repository:

```sh
git clone https://github.com/9M2PJU/9M2PJU-T-Display-S3-DX-Cluster.git
cd 9M2PJU-T-Display-S3-DX-Cluster
```

2. Open the folder in Visual Studio Code.

3. Install the recommended **PlatformIO IDE** extension when VS Code asks.

4. Create your private config:

```sh
cp include/config.example.h include/config.h
```

5. Edit `include/config.h`:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define DX_CLUSTER_LOGIN_CALLSIGN "9M2PJU"
```

6. Connect the T-Display S3 by USB-C.

7. In VS Code, open the PlatformIO sidebar and choose:

- **Build**
- **Upload**
- **Monitor**

That is the intended user flow: clone, open in VS Code, edit config, build, upload.

## Hardware

- LilyGO T-Display-S3 development board
- ESP32-S3 Wi-Fi and Bluetooth 5.0 module
- Built-in 1.9 inch ST7789 LCD
- 8-bit parallel LCD bus
- 320 x 170 usable landscape layout
- USB-C cable for flashing and serial monitor
- Wi-Fi network with internet access

## DX Cluster

Default cluster settings:

- Host: `9m2pju.hamradio.my`
- Port: `7300`
- Protocol: telnet
- Login: amateur radio callsign

These values are in `include/config.example.h`:

```cpp
#define DX_CLUSTER_HOST "9m2pju.hamradio.my"
#define DX_CLUSTER_PORT 7300
#define DX_CLUSTER_LOGIN_CALLSIGN "9M2PJU"
```

## Configuration

Do not edit `include/config.example.h` for your private setup. Copy it first:

```sh
cp include/config.example.h include/config.h
```

Then edit `include/config.h`.

Important values:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define DX_CLUSTER_HOST "9m2pju.hamradio.my"
#define DX_CLUSTER_PORT 7300
#define DX_CLUSTER_LOGIN_CALLSIGN "9M2PJU"

#define NTP_SERVER_PRIMARY "pool.ntp.org"
#define NTP_SERVER_SECONDARY "time.google.com"
```

`include/config.h` is ignored by git so Wi-Fi credentials are not committed.

## Build And Upload From VS Code

Install:

- [Visual Studio Code](https://code.visualstudio.com/)
- PlatformIO IDE extension for VS Code

Open this project folder in VS Code. PlatformIO will read `platformio.ini` automatically.

Use the PlatformIO sidebar:

- **Project Tasks > t-display-s3 > General > Build**
- **Project Tasks > t-display-s3 > General > Upload**
- **Project Tasks > t-display-s3 > Platform > Monitor**

If upload fails because the board is not detected, hold the BOOT button while plugging in USB-C, then upload again.

## Build And Upload From Terminal

Build:

```sh
pio run
```

Upload:

```sh
pio run --target upload
```

Serial monitor:

```sh
pio device monitor
```

## What The Firmware Does

On boot, the device:

1. Starts the display.
2. Connects to Wi-Fi.
3. Syncs UTC time from NTP.
4. Opens telnet to `9m2pju.hamradio.my:7300`.
5. Sends the configured callsign when the cluster asks for login.
6. Reads incoming cluster lines.
7. Parses common `DX de ...` spot lines.
8. Shows newest DX spots on screen.
9. Reconnects automatically if the telnet connection drops.

## Display Layout

The top bar shows the project name, connection pulse, and fixed UTC clock:

```text
+------------------------------+
| 9M2PJU DX Cluster     12:34Z |
+------------------------------+
| 14.074  JA1ABC               |
| de 9M2XYZ  FT8, CQ DX        |
|                              |
| 21.300  VK2DEF               |
| de 9M2ABC  strong signal     |
|                              |
|  7.025  DL1AAA               |
| de 9M2PJU  Europe opening    |
+------------------------------+
```

The newest spots are highlighted briefly. Band color accents make the spot list easier to scan.

## Files

- `platformio.ini`: PlatformIO project setup
- `src/main.cpp`: firmware source
- `include/config.example.h`: safe config template
- `include/config.h`: private user config, created locally and ignored by git
- `.vscode/extensions.json`: recommends the PlatformIO VS Code extension

## Notes

- Keep the clock in UTC for radio logging consistency.
- Keep real Wi-Fi passwords out of `include/config.example.h`.
- This project is configured for the T-Display-S3 1.9 inch ST7789 board using LilyGO's 8-bit parallel LCD pinout.
- LCD pin settings are passed to `TFT_eSPI` from `platformio.ini`.
- If your board revision uses different display pins, adjust the `TFT_*` build flags in `platformio.ini`.
- The first build may take a while because PlatformIO downloads the ESP32 platform and display library.
