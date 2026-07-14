# ESP32 Face-Detection Servo

The most advanced project of the day: the ESP32-S3 camera looks for a human face using **on-device machine learning** (no computer, no cloud, no internet — the AI runs entirely on the chip itself), and while it sees a face, a servo motor continuously sweeps back and forth. Look away, and it stops. There's also an optional web page for demos, showing the live camera view with a green box drawn around any detected face.

This README also tells the story of the real bugs we hit building this and how we tracked each one down — debugging like this is a normal, expected part of embedded programming, not a sign something went wrong.

## The big idea

1. Camera grabs a photo (240×240 pixels — small on purpose, more on why below).
2. A tiny neural network, running directly on the ESP32-S3's own processor, looks at that photo and answers: "is there a face in here, and if so, roughly where?"
3. If yes: keep sweeping a servo motor between 0° and 90°, over and over.
4. If no: stop, and rest the servo at 0°.

## Circuit diagram

```
                     ESP32-S3 CAM board
                 (Freenove ESP32-S3 WROOM)
              ┌───────────────────────────┐
              │                           │
              │     [ OV2640 Camera ]     │   <- already attached, no wiring needed
              │      (built onto           │
              │       the board)           │
              │                           │
              │                    GPIO14 ●───────────┐  orange wire
              │                       5V  ●───────┐   │  (signal)
              │                      GND  ●───┐   │   │
              └───────────────────────────┘   │   │   │
                                               │   │   │
                                               │   │   ┌────────────┐
                                               │   │   │            │
                                               │   └───┤ red (5V)   │
                                               │       │            │
                                               └───────┤ black (GND)│
                                                        │            │
                                                        │  SERVO     │
                                                        │  (3-wire   │
                                                        │  hobby     │
                                                        │  servo)    │
                                                        └────────────┘
```

Three wires, that's it:
| Servo wire | Goes to |
|---|---|
| Orange (signal) | GPIO 14 on the board |
| Red (power) | The board's own `5V` pin |
| Black (ground) | The board's own `GND` pin |

**A note on power**: normally, for a servo, you'd want a completely separate 5V power source (not sharing power with the microcontroller itself) — a moving servo can briefly pull a lot of current, and if the chip and the servo share the exact same power rail, that current spike can very briefly "brown out" (starve of power) the chip itself, causing random resets. In our testing, this specific small hobby servo worked fine powered straight from the board's own `5V` pin — but if you ever see the board randomly resetting or acting glitchy exactly when the servo moves, that's the classic symptom, and the fix is powering the servo from somewhere else entirely (a USB power bank, or even another microcontroller board just used as a power brick), with only the ground wires connected together between the two power sources.

**Why GPIO14 specifically?** We checked Freenove's own official pinout diagram (see the top-level `esp32/README.md`) and confirmed GPIO14 is the one nearby pin that's both PWM-capable (servos need a specific repeating pulse pattern, called PWM, to know what angle to hold) and *not* already wired to the camera.

## The software pieces, explained

### The face detector: `EloquentEsp32Cam` library

Espressif does have their own official face-detection library, but it's built for a different, more complicated build system (ESP-IDF directly) that doesn't mesh cleanly with the simpler Arduino-style setup we've used all day. Instead, this project uses a community library, **EloquentEsp32Cam**, which wraps the same underlying AI models in an Arduino-friendly package.

The actual model doing the work is Espressif's own on-device face detector from their **ESP-WHO / esp-face** component — a lightweight, MTCNN-derived cascaded neural network (sometimes called MTMN: multi-task cascaded network) built specifically to run within the ESP32-S3's limited memory and compute. It's a pretrained model, not something trained from scratch for this project — `detection.accurate()` just switches it from a fast single-stage pass to a slower two-stage pass (proposal + refine), which is what took the confidence scores from 0.10–0.15 up to 0.96–1.00 (see Bug 1 below).

```cpp
camera.pinout.freenove_s3();
camera.resolution.face();
```
The library ships with ready-made pin configurations for specific known boards — `freenove_s3()` is an exact match for this board, so we don't have to hand-write a `camera_pins.h` like we did for the camera-streaming project. `resolution.face()` locks the camera to 240×240 pixels — a small, square image. Face-detection AI models are trained on small fixed-size images on purpose (smaller = much faster for a tiny chip to process); this isn't a limitation of the camera, it's a deliberate choice for the detector.

```cpp
detection.accurate();
detection.confidence(0.6);
```
More on why `accurate()` matters below — short version: without it, the detector barely works at all.

```cpp
bool faceDetected = detection.run().isOk() && detection.found();
```
Each pass through `loop()`, this asks the camera for a fresh photo, runs it through the detector, and asks "did you find anything above our confidence threshold?"

### The servo sweep + "debounce" logic

```cpp
if (detection.found()) {
  lastSeenFaceMs = millis();
}
bool faceRecentlyPresent = (millis() - lastSeenFaceMs) < FACE_LOST_GRACE_MS;
```
Face detection isn't perfectly reliable frame-to-frame — even with your face sitting still in front of the camera, an occasional single frame will fail to detect it (slight angle change, blink, lighting flicker). We initially just reacted instantly to "found" vs "not found" each frame, and the servo motion looked jittery/erratic — every single missed frame snapped it back to the start position. The fix is a small pattern called **debouncing**: instead of trusting each individual reading, we remember the *last time* we saw a face, and only treat the face as truly "gone" once it's been missing continuously for a short grace period (800 milliseconds here). This is the exact same technique used for physical button presses, which have a similar "flickery" problem at the electrical level.

## Bugs we actually found and fixed (the good stuff)

This project had by far the most debugging of the day — a good showcase of how real embedded development actually goes.

### Bug 1: face detection never triggered at all
First attempt: constant `"No face"`, even with a face clearly in frame. We added temporary debug code to print every raw candidate detection's confidence score, even ones below our threshold — and found the detector *was* finding a face-shaped region every time, but scoring it only **0.10–0.15** (out of a possible 1.0). The cause: the library defaults to a fast, low-accuracy single-pass detection mode. Calling `detection.accurate()` switches on a second verification pass, and scores immediately jumped to **0.96–1.00** for the same real face. Moral: when something's "almost working," print the actual numbers involved rather than just true/false — the raw scores told us exactly what was wrong.

### Bug 2: (carried over from the camera-streaming project) upside-down image
Same root cause as `esp32_camera_web` — this board's camera is physically mounted flipped relative to what the library assumes — fixed the same way, calling `camera.sensor.vflip(true)`.

### Bug 3: the web demo page wouldn't load at all
We first tried the library's own ready-made "face detection stream" page. It never loaded — not slow, not broken-looking, just an endless spinner. Testing directly with `curl` (a command-line tool for making web requests) confirmed the server accepted the connection but sent back **zero bytes, forever**. Reading the library's own source code revealed why: one of its routes is written as an infinite loop that's *designed* to never return (for continuously pushing live updates) — but the underlying web server it's built on can only handle one thing at a time, so once that route was hit, the *entire* server got permanently stuck, unable to serve any other page ever again. This wasn't something we did wrong — it's a genuine fragility in that part of the library. We wrote our own much simpler replacement page instead.

### Bug 4: video stream showed nothing, forever
Our own replacement first tried true continuous video streaming. Testing with `curl` again, we saw the connection succeed and headers arrive correctly, but the actual picture data never came. The cause: the camera hardware can only be talked to by one piece of code at a time (protected by something called a **mutex** — a lock that only one task can hold at once). Our face-detection loop was holding that lock for longer, each cycle, than the video-streaming code was willing to wait for it — so every single attempt to grab a video frame timed out and gave up. The fix was to stop running two competing camera-access loops entirely: our main loop already grabs one photo per cycle anyway, so the web page just displays *that exact same already-captured photo*, refreshed about every 700 milliseconds, instead of true smooth video. Slightly choppier, but completely reliable, and simpler to reason about.

## The optional web demo

```cpp
#define ENABLE_WEB_VIEWER 1
```
Set this to `1` for the demo web page (connects to WiFi, shows the picture + a green detection box) or `0` for fully offline/standalone operation (no WiFi at all — the servo logic works exactly the same either way, since the web viewer is purely for *watching* what's happening, not part of the actual detection/servo logic).

## Before you build this (if `ENABLE_WEB_VIEWER` is on)

`src/main.cpp` has your WiFi network name and password hardcoded in plain text (`WIFI_SSID`/`WIFI_PASSWORD`) — only used when `ENABLE_WEB_VIEWER` is `1`, since the standalone detection+servo logic never needs WiFi at all.

> **Before pushing this to GitHub (or sharing it anywhere):** replace those with your own network's credentials, or with placeholders like `"YOUR_WIFI_SSID"` / `"YOUR_WIFI_PASSWORD"` that you fill in locally after cloning. Never commit real WiFi credentials to a public repo.

## Hardware needed

- Freenove ESP32-S3 CAM board
- A 3-wire hobby servo (the kind with red/black/signal wires — ours came from an Arduino Mega kit, works identically here)
- Optionally, a WiFi network (only needed if `ENABLE_WEB_VIEWER` is on)

## How to build, upload, and test it

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -d "C:\Users\Saanvi\comp-sci\esp32\esp32_face_servo" --target upload --upload-port COM3
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" device monitor -p COM3 -b 115200
```

With `ENABLE_WEB_VIEWER` on, the serial monitor prints a URL like `http://192.168.68.56:82` — open that in a browser for the live demo view.

## What to try next

- Change `SWEEP_MAX_DEG` to sweep further (up to 180° for most hobby servos).
- Instead of a fixed sweep, make the servo angle track *where* the face actually is left-to-right in the frame (`detection.first.x`) — this turns it into a real face-tracking camera mount.
- Try lowering `detection.confidence(...)` and see how many false positives start appearing — a good hands-on way to understand what a confidence threshold is actually trading off.
