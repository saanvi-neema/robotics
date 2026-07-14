# Water Sensing Circuit

A pure hardware circuit — no Arduino code needed. Dip the water sensor in water and the LED lights up. Pull it out and the LED slowly fades away.

This builds directly on the LED Fade with Transistor circuit. The water sensor replaces the button — water touching the sensor traces sends a signal to the transistor Base, switching the LED on.

**Build the LED Fade with Transistor circuit first** before attempting this one.

---

## What You Need

- Elegoo Mega 2560 (used only for 5V and GND power — no code uploaded)
- Red LED
- 220Ω resistor (Red · Red · Brown · Gold)
- 10KΩ resistor (Brown · Black · Orange · Gold)
- 100µF electrolytic capacitor (check label on side)
- PN2222 or S8050 NPN transistor
- Water / rain sensor module (small PCB with exposed copper traces at one end)
- Breadboard
- Jumper wires
- Small cup of tap water for testing

---

## Circuit Diagram

```
Sensor (+) ──────────────────────────────────────────── +5V
Sensor (−) ──────────────────────────────────────────── GND

                              ┌──── C ──[220Ω]──[LED +]──[LED -]──── +5V
Sensor (S) ──┬──[10KΩ]────── B    PN2222
             │                └──── E ──────────────────────────────── GND
          [100µF]
             │
            GND
```

**Reading the diagram:**
- The sensor has 3 pins: (+) power, (−) ground, (S) signal output
- The signal pin (S) replaces the button from the LED Fade circuit
- `B`, `C`, `E` = Base, Collector, Emitter legs of the transistor
- `[LED +]` = long leg, `[LED -]` = short leg

---

## How It Works

### 1. The Water Sensor
The sensor is a small PCB with two sets of exposed copper traces interleaved like a comb:

```
| | | | | | | | |   ← exposed copper traces (the sensing end)
```

When dry: the traces are separated by air — no connection between them.
When wet: water bridges the gap between traces. Water conducts electricity (tap water contains dissolved minerals that carry charge).

This creates a small electrical path that the sensor's onboard circuit detects, causing the Signal (S) pin to output a voltage.

### 2. From Water to Light
```
Water touches traces
        │
        ▼
Sensor S pin outputs voltage
        │
        ▼
Voltage reaches transistor Base
        │
        ▼
Transistor switches ON (Collector → Emitter path opens)
        │
        ▼
Current flows: +5V → LED → 220Ω → Collector → Emitter → GND
        │
        ▼
LED lights up
```

### 3. The Fade-Out Effect
The 100µF capacitor sits at the junction between the sensor signal and the transistor Base. When the sensor detects water, the capacitor charges up. When the sensor is removed from water, the capacitor slowly releases its stored charge through the 10KΩ resistor into the Base — keeping the transistor on a little longer. This is why the LED fades instead of switching off instantly.

The fade time depends on the capacitor size (larger = longer fade) and the resistor value (larger = longer fade).

### 4. Why Tap Water and Not Distilled?
Distilled water is almost pure H₂O — it contains no dissolved minerals and barely conducts electricity. Tap water contains dissolved salts and minerals (calcium, magnesium, chlorine) that act as charge carriers. This is why tap water works but distilled water does not.

### 5. The Transistor as an Amplifier
The water sensor outputs a weak signal — not enough to directly light an LED. The transistor amplifies this: a small base current (from the sensor) controls a much larger collector-emitter current (through the LED). The LED gets its power from the 5V supply, not from the sensor itself.

---

## Water Sensor Pins

The module has 3 pins labeled on the PCB:

```
−   → GND rail
+   → 5V rail
S   → Signal (connects to your circuit)
```

The sensing end (copper traces) is on the opposite side. That end goes in the water.

---

## Breadboard Wiring — Step by Step

**Step 1 — Power**
- Mega 5V pin → red power rail
- Mega GND pin → blue power rail

**Step 2 — Transistor, LED, 220Ω, capacitor**
Build exactly as in the LED Fade with Transistor circuit. Skip the button entirely.

**Step 3 — Connect the water sensor**
- Sensor (−) pin → GND rail
- Sensor (+) pin → 5V rail
- Sensor (S) pin → junction row (see Step 4)

**Step 4 — Junction point**
The S pin output connects to 3 things in the same breadboard row:
- 10KΩ resistor → Base (B, middle leg of transistor)
- Capacitor (+) long leg
- Wire from S pin

---

## Test It

1. Power on the Mega (no sketch needed — just plug in USB)
2. Dip the copper trace end of the sensor in a cup of tap water → LED lights up
3. Pull the sensor out → LED slowly fades out

---

## Tips

- **More water contact = stronger signal** — dip the sensor deeper for a brighter LED
- **Use tap water** — distilled or deionized water will not work
- **Dry the sensor after testing** — leaving it wet causes corrosion on the copper traces over time
- The small blue trimmer pot on the sensor module adjusts sensitivity — turn it slowly if the LED does not respond

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| LED always on without water | S pin stuck HIGH — check sensor wiring, try a different sensor module |
| LED never turns on | S pin not outputting enough voltage — touch S pin directly to 5V briefly to verify rest of circuit works |
| LED turns on but no fade | Capacitor in wrong orientation — flip it, or check it is connected at the S pin junction not elsewhere |
| LED turns off instantly, no fade | Sensor draining capacitor — add a 1N4148 diode between S pin and junction, stripe end facing the junction |
| Works but very dim | Sensor signal voltage is low — try removing the 10KΩ and connecting S directly to Base |
| Works in cup but not shallow water | Water may have low conductivity — use tap water, dip sensor deeper |
