# Weather Station

A simple weather station that reads temperature and humidity from a DHT11 sensor and displays them on a 16x2 LCD screen. Readings update every 2 seconds.

---

## What You'll Learn

- How to wire multiple components to a breadboard
- How a sensor communicates data to a microcontroller
- How to display text on an LCD screen
- How a potentiometer works as a voltage divider

---

## How It Works

### The DHT11 Sensor
The DHT11 is a small sensor that measures two things:
- **Temperature** using a thermistor (a resistor that changes value with heat)
- **Humidity** using a capacitive sensor (humidity changes how well a tiny capacitor holds a charge)

It sends both readings to the Arduino over a **single wire** using a timing-based protocol — basically a series of HIGH and LOW pulses where the length of each pulse represents a 0 or 1 bit of data. The Adafruit DHT library decodes all of this and hands you back simple numbers.

One thing to know: the DHT11 can only take a reading about once per second. Asking for data faster than that just gives you the same old reading, so the code waits 2 seconds between updates.

### The LCD1602 Display
LCD stands for Liquid Crystal Display. The 1602 means 16 characters wide, 2 rows tall. Each character is made up of a tiny grid of pixels.

The Arduino talks to the LCD using **6 wires**:
- **RS (Register Select)** — tells the LCD whether you're sending a command (like "clear screen") or actual text to display
- **E (Enable)** — a clock signal; the LCD reads incoming data every time this pin pulses
- **D4–D7** — 4 data pins that carry the actual characters, 4 bits at a time

The `LiquidCrystal` library handles all the timing and signalling — you just call `lcd.print("Hello")` and it works.

### The Potentiometer (Contrast Control)
The potentiometer is a knob connected between 5V and GND. The middle pin (called the wiper) outputs a voltage anywhere between 0V and 5V depending on how far you turn the knob. This voltage feeds into the LCD's **V0 pin** which controls pixel contrast.

- Too low → screen looks washed out or blank
- Too high → everything goes dark
- Sweet spot → clear readable text

This is why after powering on, the first thing to do is turn the knob slowly until the text appears sharply.

---

## Circuit Diagram

```
                    ┌─────────────────────────┐
                    │     ELEGOO MEGA 2560    │
   +5V ─────────────┤ 5V              GND    ├───── GND
                    │                         │
                    │  Pin 22 ────── RS        │
                    │  Pin 23 ────── E         │◄── LCD1602
                    │  Pin 24 ────── D4        │    (6 signal wires)
                    │  Pin 25 ────── D5        │
                    │  Pin 26 ────── D6        │
                    │  Pin 27 ────── D7        │
                    │                         │
                    │  Pin 8  ────── DATA     │◄── DHT11
                    └─────────────────────────┘

LCD1602 power and contrast:
  pin 1  (VSS) ──── GND
  pin 2  (VDD) ──── +5V
  pin 3  (V0)  ──── Potentiometer wiper  ◄── sets contrast
  pin 5  (RW)  ──── GND                      (turn knob to adjust)
  pin 15 (A)   ──── +5V  (backlight)
  pin 16 (K)   ──── GND  (backlight)

DHT11:
  GND  ──── GND
  DATA ──── Mega Pin 8
  VCC  ──── +5V

Potentiometer (B103):
  Left  ──── GND
  Wiper ──── LCD pin 3 (V0 / contrast)
  Right ──── +5V
```

---

## How the Sketch Works

Every 2 seconds the Arduino runs through this sequence:

```
DHT11 sensor
    │  sends temperature + humidity as digital pulses
    ▼
Mega Pin 8
    │  DHT library decodes the pulses into numbers
    ▼
Arduino sketch
    │  formats the text ("Temp: 24.2°C")
    ▼
Mega Pins 22–27
    │  LiquidCrystal library sends characters 4 bits at a time
    ▼
LCD1602
    │  displays the reading on screen
    ▼
delay(2000) — waits 2 seconds, then repeats
```

If the sensor returns an invalid reading (`nan`) — bad wiring, wrong pin order, or a damaged sensor — the sketch shows `Sensor error!` instead and tries again on the next cycle.

---

## Hardware (Elegoo Mega 2560 kit)
- Elegoo Mega 2560
- DHT11 temperature/humidity module
- LCD1602 16x2 display
- 10kΩ potentiometer (marked B103 on the body)

---

## Breadboard Layout

Breadboard orientation: j=top row, a=bottom row, columns 1–63 left to right.
Top rails: -(GND) topmost, +(5V) below. Bottom rails: -(GND) above, +(5V) bottommost.

### Component Placement
```
[L] = LCD1602         row a, cols 5–20   (body hangs off front edge onto second breadboard)
[P] = Potentiometer   f30 (left/GND), f31 (middle/wiper), f32 (right/5V)
[D] = DHT11           j5 (GND), j6 (DATA), j7 (VCC)
```

> Note: the DHT11 module from AliExpress has pins labeled GND, DATA, VCC left to right (facing the sensor). Some modules label it S, +, − — check the markings on your specific module.

---

## Wiring

### Step 1 — Mega power (do this first)
| From | To |
|------|----|
| Mega 5V | top + rail |
| Mega GND | top − rail |

### Step 2 — Breadboard-only wiring (no Mega needed yet)
| From | To | Purpose |
|------|----|---------|
| b5  | top − rail | LCD pin 1 — GND |
| b6  | top + rail | LCD pin 2 — 5V power |
| b7  | f31 | LCD pin 3 — contrast, connects to pot wiper |
| b9  | top − rail | LCD pin 5 — RW, always GND (write-only mode) |
| b19 | top + rail | LCD pin 15 — backlight positive |
| b20 | top − rail | LCD pin 16 — backlight negative |
| f30 | top − rail | Potentiometer left pin — GND |
| f32 | top + rail | Potentiometer right pin — 5V |
| j5  | top − rail | DHT11 GND — ground |
| j7  | top + rail | DHT11 VCC — power |

### Step 3 — Signal wires to Mega
| From | To | Purpose |
|------|----|---------|
| b8  | Mega pin 22 | LCD RS |
| b10 | Mega pin 23 | LCD E |
| b15 | Mega pin 24 | LCD D4 |
| b16 | Mega pin 25 | LCD D5 |
| b17 | Mega pin 26 | LCD D6 |
| b18 | Mega pin 27 | LCD D7 |
| j6  | Mega pin 8  | DHT11 DATA |

---

## Libraries to Install

In Arduino IDE go to **Sketch → Include Library → Manage Libraries** and install:
- **DHT sensor library** by Adafruit
- **Adafruit Unified Sensor** by Adafruit

`LiquidCrystal` is already built into the Arduino IDE — no install needed.

---

## Status
Working. Displays temperature and humidity, updates every 2 seconds.

## First Run Checklist
1. Upload the sketch
2. Turn the potentiometer knob slowly until text appears clearly on the LCD
3. If you see `Sensor error!` — check the DHT11 wires at j5, j6, j7 and the wire to Mega pin 8
4. If the LCD stays completely blank — adjust the contrast knob first before assuming a wiring problem
5. If the backlight isn't on at all — check b6 (5V) and b5 (GND)
