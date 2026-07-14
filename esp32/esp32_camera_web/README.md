# ESP32 Camera Web Server

This turns the ESP32-S3 CAM board into a tiny standalone security camera: it connects to WiFi, grabs pictures continuously from its onboard camera, and streams live video to any browser that visits its IP address — with on-page controls for resolution, brightness, flipping the image, and more.

## The hardware behind this

Your board has a small camera module glued/ribbon-cabled onto it, built around a chip called the **OV2640** — a 2-megapixel image sensor, the same basic type of component used in cheap webcams and old phone cameras. The ESP32-S3 talks to it over a set of dedicated wires (more on that below) and asks it for a fresh picture whenever it wants one.

This board specifically needs **PSRAM** (extra external memory chip, separate from the ESP32's built-in RAM) — 8MB of it — because a captured photo is a big chunk of data to hold in memory temporarily while it's being compressed and sent out over WiFi, more than the chip's built-in memory alone could comfortably handle.

## Where the code actually comes from

Unlike the other projects, most of this one **isn't code we wrote from scratch** — `app_httpd.cpp` and `camera_index.h` are Espressif's own official "CameraWebServer" example (the standard demo every ESP32 camera board ships with), copied straight from your kit. This is completely normal and expected in embedded/hobby electronics — you very rarely reinvent something like an HTTP camera streaming server from zero; you take a proven, working implementation and adapt the small parts that need to change for your specific board.

## Files in this project, and what each one does

- **`camera_pins.h`** — a giant list of `#define`s mapping every camera wire (data lines, clock, sync signals) to a specific GPIO number, with a different block for *every* camera board Espressif has ever supported (AI-Thinker, M5Stack, XIAO, etc.). We select `CAMERA_MODEL_ESP32S3_EYE`, because — after checking Freenove's own official pinout diagram (see the top-level README in the `esp32/` folder) — its pin assignments turned out to exactly match how Freenove wired this board.
- **`board_config.h`** — just `#include`s `camera_pins.h` after picking which `CAMERA_MODEL_*` to use.
- **`app_httpd.cpp`** — the actual web server: it defines every URL the camera responds to (`/stream` for live video, `/control` for changing settings, `/capture` for a single photo, etc.) and handles talking to the camera hardware to grab frames.
- **`camera_index.h`** — the entire webpage (HTML + JavaScript) you see in your browser, pre-compressed (gzip) and embedded as a giant array of numbers so it can be served instantly without needing a separate file system.
- **`src/main.cpp`** — the small amount of code we actually wrote ourselves: connects to WiFi, configures the camera, then calls `startCameraServer()` (defined in `app_httpd.cpp`) to launch everything above.
- **`partitions.csv`** — a map of how the ESP32's flash storage is divided up. The camera's compiled program is bigger than a normal sketch (all that camera + web server + JPEG code), so it needs a bigger chunk of flash reserved for it than the default layout provides.

## The code we actually wrote (`src/main.cpp`), explained

```cpp
camera_config_t config;
config.pin_d0 = Y2_GPIO_NUM;   // ... and 7 more like this ...
config.pin_xclk = XCLK_GPIO_NUM;
config.frame_size = FRAMESIZE_QVGA;
config.pixel_format = PIXFORMAT_JPEG;
config.fb_location = CAMERA_FB_IN_PSRAM;
```
This builds up a big settings object describing exactly how to talk to the camera: which GPIO does what (from `camera_pins.h`), what resolution to start at, what image format to use (JPEG — the same compressed format your phone photos use, much smaller than raw pixels), and to store captured frames in that external PSRAM chip rather than the ESP32's small internal memory.

```cpp
sensor_t * s = esp_camera_sensor_get();
if (pid == OV2640_PID) {
  s->set_hmirror(s, 1);
  s->set_vflip(s, 1);
}
```
After the camera initializes, we ask it "which exact sensor chip are you?" (`OV2640_PID` is this chip's product ID) and then apply mirror/flip settings. **This is exactly where we hit our first real bug** — see below.

## Bugs we actually found and fixed (the good stuff)

### The image was upside-down
The stock code sets `vflip` based on *assumptions about a reference board design* (Espressif's own ESP32S3-EYE) — but Freenove physically mounted the camera module on *this* board rotated differently. The fix was simple once we knew the cause: we override `s->set_vflip(s, 1)` right after camera init, correcting for this specific board's physical mounting. If you ever swap in a different camera module, you may need to flip this back.

### The video was blurry
Two separate causes, found in this order:
1. **Physical protective film** — brand new camera modules ship with a small plastic tab/film over the lens (to protect it during shipping/assembly), which we needed to physically peel off. This turned out to be the main cause.
2. **Low resolution** — we'd started at `FRAMESIZE_QVGA` (320×240), which looks soft/blurry on a big monitor since the image is small and getting stretched. Higher resolutions (`VGA`/640×480, `SVGA`/800×600) are sharper, at the cost of a lower frame rate.

### The resolution dropdown only offered small square options
We checked `app_httpd.cpp` directly and found the server-side code applies *whatever* resolution it's told, with no restriction — the limited dropdown is purely a **browser-side** JavaScript choice, not a hardware limit. You can bypass it by typing a control URL directly into your browser's address bar:
```
http://<board-ip>/control?var=framesize&val=8
```
(`val=8` = VGA. This is a good example of "the UI doesn't show you an option" not meaning "it's not supported.")

## Before you build this

`src/main.cpp` has your WiFi network name and password hardcoded in plain text (`ssid`/`password` variables):

> **Before pushing this to GitHub (or sharing it anywhere):** replace those with your own network's credentials, or with placeholders like `"YOUR_WIFI_SSID"` / `"YOUR_WIFI_PASSWORD"` that you fill in locally after cloning. Never commit real WiFi credentials to a public repo.

## Hardware needed

The Freenove ESP32-S3 CAM board (with its OV2640 camera attached) and a WiFi network. No extra wiring.

## How to build, upload, and test it

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -d "C:\Users\Saanvi\comp-sci\esp32\esp32_camera_web" --target upload --upload-port COM3
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" device monitor -p COM3 -b 115200
```

Watch serial for `Camera Ready! Use 'http://<ip>' to connect`, then open that address in a browser on the same WiFi network. You'll see a live video stream plus sliders/controls for resolution, quality, brightness, and flip/mirror.

## For Arduino IDE users

This project's files were copied from `Sketch_17.1_CameraWebServer` in the Freenove kit download — if you'd rather use the Arduino IDE instead of PlatformIO, just open that original folder directly (Arduino IDE expects all the `.cpp`/`.h` files to sit alongside the `.ino` in one folder, which that source folder already is).

## What to try next

- Try the direct control-URL trick above with other resolutions (`val=9` for SVGA, `val=13` for max UXGA — check `camera_pins.h`'s neighboring `sensor.h`/`framesize_t` enum for the full list).
- Add a simple motion-detection feature: compare brightness between consecutive frames and print "motion detected!" when it changes a lot.
- Combine this project's camera code with the BLE project's Bluetooth code — same camera, but notify a phone over Bluetooth instead of streaming to a browser.
