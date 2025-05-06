# ESPDrive8266

This project lets you upload files from a web browser in local network directly to an SD card attached to your ESP8266 via a simple web interface. 
Port forwarding is required for control over the internet from remote distances

## Features

- 📶 Auto WiFi setup using **WiFiManager**
- 💾 Upload files to SD card via Web UI
- 🌐 Reset WiFi settings with a button


## Hardware Required

- ESP8266 board (e.g., NodeMCU, Wemos D1 Mini)
- SD card module (connected via SPI)
- SD card formatted as FAT32
- Common wiring for SPI (D5-D8 on most boards)

## Wiring Example

| SD Module Pin | ESP8266 Pin |
|---------------|-------------|
| MISO          | D6 (GPIO12) |
| MOSI          | D7 (GPIO13) |
| SCK           | D5 (GPIO14) |
| CS            | D8 (GPIO15) |

> Make sure your SD card module is 3.3V compatible or use level shifting.

## Libraries Used

- [ESP8266WiFi](https://arduino-esp8266.readthedocs.io/)
- [ESP8266WebServer](https://arduino-esp8266.readthedocs.io/)
- [WiFiManager](https://github.com/tzapu/WiFiManager)
- [SD](https://www.arduino.cc/en/Reference/SD)
- [FS](https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html)

## Usage

1. Flash the code to your ESP8266.
2. On first boot, connect to the WiFi AP called `ESPcard-Setup`.
3. Select and connect to your WiFi network.
4. Open a browser and go to `http://<your-esp-ip>/` or `http://espcard.local`
5. Upload files via the web interface.
6. Click **Reset WiFi Settings** if needed.

## Notes

- The uploaded files are saved to the root of the SD card.
- Speed limited to 60-70 kb/sec
- Tested on NodeMCU with 2GB SD card.

