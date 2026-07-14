# Temperature-Controlled LED Brightness

The warmer it gets, the brighter the LED glows. A DHT11 temperature sensor feeds readings to the Arduino every 2 seconds. The Arduino maps the temperature to a brightness level and drives an LED through a transistor — so the LED physically responds to heat in real time.

Put your finger on the DHT11 sensor and watch the LED brighten as it picks up your body heat.

---

## Build Strategy — Two Circuits First, Then Combine

This project combines two independent ideas. The smart way to build it is to get each one working on its own before wiring them together. Debugging a combined circuit is hard — debugging two simple circuits separately is easy.

---

### Circuit 1 — Transistor LED Switch

Build and test the transistor and LED before the Arduino is involved at all. Use the Mega only as a 5V/GND power supply (no sketch needed).

**What to build:**
```
        +5V
         │
      [LED +]  ← long leg
      [LED −]  ← short leg
         │
       [220Ω]
         │
         ├────── C  (Collector)
         │
         │       B  (Base) ──[10kΩ]──┐
         │                            │  (leave this end unconnected for now)
         └────── E  (Emitter)
                      │
                     GND
```

**Test it:** Touch the free end of the 10kΩ resistor (the Base side) to the +5V rail. The LED should light up. Touch it to GND — LED goes off. This confirms your transistor orientation, resistor values, and LED polarity are all correct.

If it works: you know the transistor circuit is solid before adding any Arduino complexity.

---

### Circuit 2 — DHT11 Temperature Reading

Wire just the DHT11 to the Mega and upload a minimal sketch to confirm the sensor reads correctly.

**Wiring:**
```
DHT11 GND  ──── GND
DHT11 DATA ──── Mega Pin 8
DHT11 VCC  ──── +5V
```

**Test sketch** (upload this first):
```cpp
#include <DHT.h>
DHT dht(8, DHT11);
void setup() { Serial.begin(9600); dht.begin(); }
void loop() {
  delay(2000);
  Serial.println(dht.readTemperature());
}
```

Open Serial Monitor at 9600 baud. You should see a temperature reading every 2 seconds. Put your finger on the sensor — the number should rise.

If it works: you know the sensor, its wiring, and pin 8 are all good.

---

### Combine — Replace the Manual Test with Arduino PWM

Once both circuits work independently:

1. Keep the DHT11 wired as-is (pin 8)
2. Connect **Mega pin 9** to the free end of the 10kΩ resistor (the Base side, where you were previously touching +5V manually)
3. Upload the full sketch from this project

The Arduino now does what your finger was doing — but instead of just on/off, it outputs a variable PWM signal that controls exactly how much the transistor opens, and therefore how bright the LED glows.

---

## What You Need

- Elegoo Mega 2560
- DHT11 temperature/humidity sensor module (3-pin module version)
- Red LED
- 220Ω resistor (Red · Red · Brown · Gold)
- 10kΩ resistor (Brown · Black · Orange · Gold)
- PN2222 or S8050 NPN transistor (small black D-shaped component, 3 legs)
- Breadboard
- Jumper wires

---

## How It Works

### The DHT11 Sensor
The DHT11 measures temperature using a thermistor — a resistor whose value changes with heat. It sends the reading to the Arduino over a single wire as a series of timed HIGH/LOW pulses. The DHT library decodes these pulses and gives you back a temperature in degrees Celsius.

### The Arduino as the Brain
Every 2 seconds the Arduino:
1. Reads the temperature from the DHT11
2. Maps it to a number between 0 and 255 (0 = off, 255 = full brightness)
3. Sends that number to pin 9 using `analogWrite()`

`analogWrite()` produces a **PWM signal** — it rapidly switches the pin between HIGH and LOW many times per second. The ratio of HIGH time to LOW time (called the duty cycle) determines the effective voltage seen by the circuit. At 50% duty cycle the LED sees roughly 2.5V and glows at half brightness. At 100% it sees the full 5V and glows at full brightness.

### Why a Transistor? Why Not Connect the LED Directly?
The Arduino's output pins can only source about 40mA of current — just barely enough for a single dim LED, and nothing left for anything else. More importantly, the PWM signal from pin 9 is a *control signal*, not a power source.

The transistor (PN2222) solves this by acting as a current amplifier:

```
  Flat face of PN2222 toward you, legs pointing down:

  [ PN2222 ]
   │   │   │
   E   B   C

  E = Emitter   → connects to GND
  B = Base      → receives the control signal from the Arduino
  C = Collector → connects to the load (LED circuit)
```

Think of it like a water valve:
- **Base (B)** is the handle — a small signal here opens or closes the valve
- **Collector → Emitter** is the pipe — the actual current that powers the LED flows through here
- A tiny current into the Base (from the Arduino) controls a much larger current through Collector to Emitter (from the 5V supply through the LED)

When `analogWrite(9, 128)` puts out a 50% PWM signal, the transistor Base sees a fluctuating signal. The transistor responds by letting proportionally more or less current through the Collector-Emitter path. The LED brightness follows directly.

The 10kΩ resistor between pin 9 and the Base protects both the Arduino pin and the transistor — it limits the base current to a safe level (~0.5mA) while still being enough to switch the transistor.

The 220Ω resistor in series with the LED limits the LED current to ~14mA, preventing burnout.

### Temperature to Brightness Mapping
```
TEMP_MIN (20°C) → brightness 0   → LED off
TEMP_MAX (35°C) → brightness 255 → LED full brightness
Between          → brightness scales linearly
```

You can change `TEMP_MIN` and `TEMP_MAX` in the sketch to match your environment. In a warm room, raise `TEMP_MIN`. In a hot environment, raise `TEMP_MAX`.

---

## Circuit Diagram

```
        +5V
         │
      [LED +]  ← long leg / anode
      [LED −]  ← short leg / cathode
         │
       [220Ω]
         │
         ├────── C  (Collector)
         │
         │       B  (Base) ──[10kΩ]──── Mega Pin 9
         │
         └────── E  (Emitter)
                      │
                     GND

DHT11 module:
  GND  ──── GND
  DATA ──── Mega Pin 8
  VCC  ──── +5V
```

**Full connection summary:**

```
+5V ──────────┬──────── LED anode (long leg)
              └──────── DHT11 VCC

GND ──────────┬──────── Transistor Emitter (E)
              └──────── DHT11 GND

Mega Pin 9 ───[10kΩ]── Transistor Base (B)
Mega Pin 8 ──────────── DHT11 DATA
```

---

## Board Connections — Step by Step

### Step 1 — Power
| From | To |
|------|----|
| Mega 5V | breadboard + rail |
| Mega GND | breadboard − rail |

### Step 2 — DHT11 sensor
Your DHT11 module has 3 pins. Check the label on the PCB for pin order — common orderings are GND/DATA/VCC or S/+/−. Wire accordingly:

| DHT11 pin | Connect to |
|-----------|------------|
| GND | breadboard − rail |
| DATA (or S) | Mega pin 8 |
| VCC (or +) | breadboard + rail |

### Step 3 — Transistor
Place the transistor with the flat face toward you, legs pointing into the breadboard. The legs are E (left), B (middle), C (right).

| Transistor leg | Connect to |
|----------------|------------|
| E (Emitter) | breadboard − rail |
| B (Base) | one end of 10kΩ resistor |
| C (Collector) | one end of 220Ω resistor |

### Step 4 — LED
| LED leg | Connect to |
|---------|------------|
| Long leg (anode +) | breadboard + rail |
| Short leg (cathode −) | other end of 220Ω resistor |

### Step 5 — Resistors
| Component | From | To |
|-----------|------|----|
| 10kΩ resistor | Transistor Base (B) | Mega pin 9 |
| 220Ω resistor | Transistor Collector (C) | LED short leg (cathode) |

---

## Libraries to Install

In the Arduino IDE go to **Sketch → Include Library → Manage Libraries** and install:
- **DHT sensor library** by Adafruit
- **Adafruit Unified Sensor** by Adafruit

---

## Upload the Sketch

Via Arduino CLI:
```bash
arduino-cli compile --fqbn arduino:avr:mega ~/arduino/temp-controlled-led
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:mega ~/arduino/temp-controlled-led
```

---

## Try It

1. Power on the Mega
2. The LED brightness reflects the current room temperature
3. Press your finger firmly on the DHT11 sensor — your body heat (~37°C) pushes the reading up and the LED brightens
4. Remove your finger — temperature drops, LED dims over the next few readings

Open the Serial Monitor at **9600 baud** to see the temperature and brightness value printed every 2 seconds.

---

## Calibrating the Range

Open the sketch and adjust these two lines at the top:

```cpp
#define TEMP_MIN  20.0   // temperature at which LED is off
#define TEMP_MAX  35.0   // temperature at which LED is at full brightness
```

If the LED is always on/off in your environment, narrow or shift the range to match the temperatures you're actually working with.

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| LED always off | Check TEMP_MIN — room may be below it. Lower TEMP_MIN or warm the sensor with your finger |
| LED always full brightness | Room is above TEMP_MAX — raise TEMP_MAX |
| LED does not change | Check 10kΩ is connected between pin 9 and transistor Base |
| Serial shows "Sensor error" | Check DHT11 wiring — especially the DATA wire to pin 8 |
| LED never lights at all | Check transistor orientation (E·B·C flat-face-toward-you), check 220Ω is between Collector and LED |
| LED very dim at full temperature | Check LED is not reversed — long leg should connect to +5V |
