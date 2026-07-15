# ESP32 Face Car — Vision Side

This is the "brain" of a robot car project: the ESP32-S3 CAM board looks for a human face using on-device AI (no computer, no internet — same detection technology as `esp32_face_servo`), and instead of moving a servo itself, it sends simple text commands over a wire to a **separate** Arduino Mega board, which does the actual driving.

This project is one half of a two-board system:
- **This project (`esp32_face_car`)** — the "eyes": camera + face detection + decides what the car should do
- **[`mega_motor_control`](../../arduino_mega/mega_motor_control/README.md)** — the "muscle": receives commands and actually spins the motors, via an Arduino Motor Shield Rev3

They're separate boards because an Arduino Motor Shield physically plugs onto an Arduino Uno/Mega's header — it can't attach to the ESP32-S3 board at all, which has a completely different shape and pin layout. So each board does the job it's actually suited for.

## Why two boards instead of one, technically

The ESP32-S3 is great at running a camera and AI model, but has very few genuinely free GPIO pins left once the camera uses most of them (see the pinout table in the top-level `esp32/README.md`). The Mega has tons of free pins and is exactly what the Motor Shield expects. Rather than force one chip to do a job it's awkward at, we let each board do what it's good at, and connect them with a simple 3-wire serial link.

## The full picture: how all three pieces connect

There are three separate physical things in this project, and it helps to see how they all plug into each other before wiring anything:

1. **This board (`esp32_face_car`)** — camera + face detection, decides forward/stop
2. **The Arduino Mega 2560**, with the **Arduino Motor Shield Rev3** plugged directly on top of it — receives the decision, drives the motors
3. **The car chassis's two DC gear motors** (left wheel, right wheel) — wired into the Motor Shield's two motor screw terminals

```
                    ESP32-S3 CAM board                       Arduino Mega 2560
                    (esp32_face_car)                         + Motor Shield Rev3 on top
                    -- own power supply --
                 ┌───────────────────────┐                ┌────────────────────────────┐
                 │  GPIO21 (TX)      ●────┼────────────────┼──►  Pin 19 (RX1)            │
                 │  GPIO14 (RX)      ●◄───┼────────────────┼───  Pin 18 (TX1)            │
                 │  GND              ●────┼────────────────┼───  GND (shared)            │
                 └───────────────────────┘                │                             │
                                                            │  Shield Motor A terminal ───┼──► Left wheel motor
                                                            │  Shield Motor B terminal ───┼──► Right wheel motor
                                                            │                             │
                                                            │  Power in  ◄────────────────┼──  Chassis battery pack
                                                            └────────────────────────────┘
```

What each connection is actually for:
- **Serial link (3 wires, ESP32 ↔ Mega)** — the only connection between the "brain" and the "muscle." TX on one side always wires to RX on the other, and both GNDs must be tied together so the two boards share an electrical reference point. Exact pins are in the table below.
- **Motor Shield → motors** — the screw terminals you've been wiring connect directly to the two DC gear motors. Motor A drives one wheel, Motor B drives the other. If a wheel spins the "wrong" way once you test it, that's a quick fix (swap that motor's two wires, or flip `HIGH`/`LOW` in code) — see `mega_motor_control`'s Calibration section, not a wiring mistake.
- **Power — two separate supplies, one shared ground** — the ESP32 gets its own USB power bank/phone charger. The Mega + shield + both motors run off the chassis's battery pack. These two power systems connect to each other **only** through that single shared GND wire in the serial link — motors are electrically noisy, and keeping power separate avoids that noise resetting or glitching the camera board.

For the complete pin-by-pin build notes on the Mega/motor side (exact screw terminal pins, calibration, safety timeout), see [`mega_motor_control`](../../arduino_mega/mega_motor_control/README.md).

## How it works, step by step

1. Camera captures a 240×240 photo.
2. The on-device face detector checks if there's a face in it (exact same two-stage "accurate" detection used in `esp32_face_servo`, for the same reason: the fast single-stage mode scored real faces only 0.10–0.15 out of 1.0, useless; two-stage scored 0.96–1.00, reliable).
3. Using the same **debounce** trick as `esp32_face_servo` (only treat the face as "gone" after it's been missing for 800ms straight, not on a single missed frame — detection isn't perfectly reliable frame-to-frame even with a real face sitting still), we decide: is a face *recently* present or not?
4. Send a single character over a serial wire to the Mega: `'F'` (forward) if a face is recently present, `'S'` (stop) if not.
5. Repeat, forever, many times per second.

The Mega doesn't know or care *how* we decided to send `F` or `S` — from its point of view, it's just receiving simple commands. This separation (one board *decides*, the other board *acts*) is a really common and useful pattern in robotics — it means you could completely change how the "brain" works later (add real steering, add distance control, swap the camera for something else entirely) without touching a single line of the motor-control code.

## The code, explained

```cpp
const int CAR_LINK_TX_PIN = 21;
const int CAR_LINK_RX_PIN = 14;
HardwareSerial CarLink(1);
...
CarLink.begin(CAR_LINK_BAUD, SERIAL_8N1, CAR_LINK_RX_PIN, CAR_LINK_TX_PIN);
```
The ESP32-S3 chip actually has *multiple* independent serial ports built in (called UARTs) — we're already using one (`Serial`, over USB) to talk to your computer for debugging. This line sets up a **second, completely separate** one (`CarLink`, using hardware UART #1) on two spare GPIO pins, purely for talking to the Mega. Keeping these separate means you can have the debug serial monitor open on your computer *at the same time* the car is actively talking to the Mega — they don't interfere with each other.

```cpp
if (faceRecentlyPresent) {
  CarLink.println('F');
} else {
  CarLink.println('S');
}
```
This is the entire "decision" this board makes right now — deliberately as simple as possible for a first working version. Later stages would replace this with something like "if the face is on the left side of frame, send `'L'` instead."

## Wiring to the Mega

| ESP32-S3 pin | Wire to Mega pin |
|---|---|
| GPIO21 (TX) | Pin 19 (RX1) |
| GPIO14 (RX) | Pin 18 (TX1) |
| GND | GND |

Notice TX connects to RX and vice versa (this is normal for serial connections — one side's "I'm talking" wire is the other side's "I'm listening" wire). The shared ground is just as important as the two signal wires — without it, the electrical signals have no common reference point and won't be read correctly.

## Power

This board should have its **own** power supply (a USB power bank, or a phone charger), separate from the Mega/motor/chassis battery pack. Motors and motor drivers are electrically "noisy" and can cause voltage dips that would reset or confuse a camera+AI chip sharing the same power. Only the ground wire needs to be shared between the two systems.

## Hardware needed

- Freenove ESP32-S3 CAM board
- 3 wires running to the Mega (see table above)
- A separate power source for this board (not shared with the motors)

## How to build (compile only, from this project's folder)

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -d "C:\Users\Saanvi\comp-sci\esp32\esp32_face_car"
```

To actually upload it to a board, add `--target upload --upload-port COM3` (or whichever port your board shows up on) — but note this will **replace** whatever is currently flashed on that board (e.g. `esp32_face_servo`'s servo-sweeping demo), so only do that once you're ready to switch the board over to car duty.

## What to try next

- Add left/right steering: use `detection.first.x` (the detected face's horizontal position in frame) to decide whether to send `'L'`, `'R'`, or `'F'`.
- Add distance awareness: use `detection.first.width` (a bigger box means a closer face) to send `'S'` once the face gets close enough, so the car stops a comfortable distance away instead of driving into you.
- Print the actual x/y/width/height numbers over `Serial` (the USB debug one) while testing, the same debugging trick that helped fix `esp32_face_servo`'s detection confidence bug.
