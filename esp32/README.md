# ESP32-S3 Projects

Six small, standalone projects built while learning the Freenove ESP32-S3-WROOM CAM board — from a first "hello world" LED blink all the way up to on-device face detection driving a servo motor, and finally a two-board face-following robot car. Each project lives in its own folder with its own detailed README.

## The board

**Freenove ESP32-S3 WROOM CAM** — part of the Freenove Basic Starter Kit for ESP32-S3. Key specs:

| Spec | Value |
|---|---|
| Chip | ESP32-S3 (dual-core, 240MHz) |
| Flash | 8MB |
| PSRAM | 8MB (Octal SPI) |
| Camera | OV2640 (2MP) |
| Wireless | WiFi + Bluetooth Low Energy (BLE) |
| USB-to-serial chip | CH343 |
| PlatformIO board ID | `freenove_esp32_s3_wroom` |

The 8MB of PSRAM matters a lot for the camera projects specifically — capturing and processing images needs more temporary memory than the chip's own built-in RAM comfortably provides.

## The projects

| Folder | What it does |
|---|---|
| [`esp32_blink/`](esp32_blink/README.md) | Blinks the onboard LED — the standard first test for any new board |
| [`esp32_wifi/`](esp32_wifi/README.md) | Connects to WiFi and serves a simple webpage |
| [`esp32_camera_web/`](esp32_camera_web/README.md) | Streams live video from the camera to a browser, with resolution/quality/flip controls |
| [`esp32_ble/`](esp32_ble/README.md) | Advertises as a Bluetooth Low Energy peripheral, readable/writable from a phone app |
| [`esp32_face_servo/`](esp32_face_servo/README.md) | On-device face detection (no cloud, no computer) that drives a servo motor while a face is in view |
| [`esp32_face_car/`](esp32_face_car/README.md) | The "vision brain" of a two-board face-following robot car — same face detection, but sends drive commands to a separate Arduino Mega instead of moving a servo |

Each folder's own README goes into much more detail — how the code works line by line, what hardware/wiring is needed, and (for the camera and face-detection projects especially) the real bugs we ran into and how we tracked them down.

`esp32_face_car` pairs with a second project on a *different* board family — see [`arduino_mega/mega_motor_control/`](../arduino_mega/mega_motor_control/README.md) for the motor-driving side (Arduino Mega 2560 + Arduino Motor Shield Rev3 + a 2WD chassis).

## Full pinout reference

This is the complete pin map for this exact board, from Freenove's own official pinout diagram (`ESP32S3_Pinout.png`, included in the kit download). Referencing this saved real debugging time more than once — e.g. it's what confirmed GPIO14 was safe to use for the servo signal wire in `esp32_face_servo`, and confirmed the camera pin assignments used in `esp32_camera_web` exactly matched this specific board.

### Left header (top to bottom)

| Pin | GPIO | Function on this board |
|---|---|---|
| 3V3 | — | 3.3V power out |
| RST | — | Reset the chip |
| T4 / ADC1_CH3 | GPIO4 | Camera `SIOD` (I2C data to camera) |
| T5 / ADC1_CH4 | GPIO5 | Camera `SIOC` (I2C clock to camera) |
| T6 / ADC1_CH5 | GPIO6 | Camera `VSYNC` |
| T7 / ADC1_CH6 | GPIO7 | Camera `HREF` |
| U0RTS / ADC2_CH4 | GPIO15 | Camera `XCLK` |
| U0CTS / ADC2_CH5 | GPIO16 | Camera `Y9` (data line) |
| U1TXD / ADC2_CH6 | GPIO17 | Camera `Y8` (data line) |
| U1RXD / ADC2_CH7 | GPIO18 | Camera `Y7` (data line) |
| T8 / ADC1_CH7 | GPIO8 | Camera `Y4` (data line) |
| T3 / ADC1_CH2 | GPIO3 | JTAG enable (strapping pin — avoid using for general I/O) |
| — | GPIO46 | LOG (strapping pin — avoid using for general I/O) |
| T9 / ADC1_CH8 | GPIO9 | Camera `Y3` (data line) |
| T10 / ADC1_CH9 | GPIO10 | Camera `Y5` (data line) |
| T11 / ADC2_CH0 | GPIO11 | Camera `Y2` (data line) |
| T12 / ADC2_CH1 | GPIO12 | Camera `Y6` (data line) |
| T13 / ADC2_CH2 | GPIO13 | Camera `PCLK` |
| T14 / ADC2_CH3 | **GPIO14** | **Free — used for the servo signal wire in `esp32_face_servo`** |
| 5V | — | 5V power out |

### Right header (top to bottom)

| Pin | GPIO | Function on this board |
|---|---|---|
| U0TXD | GPIO43 | Serial TX (used for programming/debugging over USB) |
| U0RXD | GPIO44 | Serial RX (used for programming/debugging over USB) |
| ADC1_CH0 | GPIO1 | Touch/ADC pin |
| ADC1_CH1 / LED ON | GPIO2 | Touch/ADC pin, tied to an onboard LED indicator |
| MTMS | GPIO42 | JTAG debug pin |
| MTDI | GPIO41 | JTAG debug pin |
| MTDO | GPIO40 | JTAG debug pin |
| MTCK | GPIO39 | JTAG debug pin |
| SD_DATA | GPIO38 | (unused here — for an optional SD card add-on) |
| SD_CLK | GPIO37 | PSRAM (**do not use** — internally wired to the external memory chip) |
| SD_CMD | GPIO36 | PSRAM (**do not use**) |
| PSRAM | GPIO35 | PSRAM (**do not use**) |
| Boot | GPIO0 | Strapping pin (**avoid** — held low during boot to enter flashing mode) |
| VSPI | GPIO45 | Strapping pin (**avoid**) |
| WS2812 | GPIO48 | Onboard addressable RGB LED |
| — | GPIO47 | General GPIO |
| — | GPIO21 | General GPIO |
| USB_D− / ADC2_CH9 | GPIO20 | USB data line (**do not use** — needed for the USB port itself) |
| USB_D+ / ADC2_CH8 | GPIO19 | USB data line (**do not use**) |
| GND | — | Ground |

### Quick "which pins can I actually use" summary

- **Already used by the camera**: GPIO4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 17, 18
- **Avoid entirely** (strapping/boot/USB/PSRAM — reserved for the chip's own internal use): GPIO0, 3, 19, 20, 35, 36, 37, 45, 46
- **Used for programming/serial**: GPIO43, 44
- **Genuinely free for your own projects**: GPIO1, 2, 14, 21, 38–42, 47, 48

## PlatformIO setup notes

All five projects use [PlatformIO](https://platformio.org/) with the Arduino framework, board ID `freenove_esp32_s3_wroom`. The PlatformIO CLI on this machine lives at:
```
%USERPROFILE%\.platformio\penv\Scripts\pio.exe
```
(not on the system PATH by default — use the full path, or add it to PATH yourself).

Common commands (run from inside a project folder, or pass `-d <path>`):
```powershell
# Build only
pio.exe run -d "path\to\project"

# Build and upload (replace COM3 with your board's port)
pio.exe run -d "path\to\project" --target upload --upload-port COM3

# See what serial port your board is on
pio.exe device list

# Watch what the board prints over serial (Ctrl+C to quit)
pio.exe device monitor -p COM3 -b 115200
```

Board shows up on **COM3** here, using the **CH343** USB-to-serial chip's driver.

## A recurring quirk worth knowing about

A few times while flashing, the board got stuck reporting `boot:0x2x (DOWNLOAD(USB/UART0)) / waiting for download` when opening the serial monitor right after an upload — the chip landed in its bootloader's flashing mode instead of running the actual program. This isn't a real problem, just USB-serial timing flakiness with this particular chip/driver combo. Simply re-running the upload command (or trying the monitor command again) resolves it.

## Kit reference docs

The full kit download (`Freenove_Basic_Starter_Kit_for_ESP32_S3-main`, in `Downloads/`) includes datasheets and example code for everything in the kit — camera sensors (OV2640, GC0308), motor drivers (L293D), shift registers (74HC595), an LCD (LCD1602), an accelerometer/gyroscope (MPU-6050), and more — useful if any future project uses parts beyond what these five projects covered.
