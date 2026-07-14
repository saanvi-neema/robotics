# LED Fade with Transistor and Capacitor

A pure hardware circuit — no Arduino code needed. Press a button and the LED lights up. Release it and the LED slowly fades out instead of switching off instantly.

This is one of the most important beginner circuits to build — it introduces the transistor, which is the fundamental building block of all modern electronics.

---

## What You Need

- Elegoo Mega 2560 (used only for 5V and GND power — no code uploaded)
- Red LED
- 220Ω resistor (Red · Red · Brown · Gold)
- 10KΩ resistor (Brown · Black · Orange · Gold)
- 100µF electrolytic capacitor (check label on side)
- PN2222 or S8050 NPN transistor (small black D-shaped component, 3 legs)
- Tactile pushbutton (4-pin)
- Breadboard
- Jumper wires

---

## Circuit Diagram

```
                              ┌──── C ──[220Ω]──[LED +]──[LED -]──── +5V
+5V ──[BTN]──┬──[10KΩ]────── B    PN2222
             │                └──── E ──────────────────────────────── GND
          [100µF]
             │
            GND
```

**Reading the diagram:**
- Current flows from left to right / top to bottom
- `[BTN]` = pushbutton
- `B`, `C`, `E` = Base, Collector, Emitter legs of the transistor
- `[LED +]` = long leg of LED, `[LED -]` = short leg

---

## How It Works

### 1. The Resistor (220Ω)
LEDs are not like light bulbs — they do not limit their own current. Without a resistor, too much current flows and the LED burns out in seconds. The 220Ω resistor acts as a gatekeeper, allowing only the right amount of current (~14mA) to flow through the LED.

### 2. The Transistor as a Switch
The transistor (PN2222) has three legs:

```
  Flat face toward you:

  [ PN2222 ]
   │   │   │
   E   B   C

  E = Emitter   (current exits here → goes to GND)
  B = Base      (control signal enters here)
  C = Collector (current enters here ← comes from LED)
```

Think of the transistor like a water valve:
- **Base (B)** is the handle you turn
- **Collector (C) → Emitter (E)** is the pipe
- A tiny current into the Base opens the valve and lets a much larger current flow through Collector to Emitter

When you press the button → a small current flows into the Base → the transistor switches on → current flows from +5V through the LED and resistor, through the transistor, and out to GND → LED lights up.

When you release the button → no base current → transistor switches off → LED off.

The button never directly powers the LED. It only sends a small signal. The transistor does the actual switching work. This matters because transistors can switch motors, speakers, and other high-current devices that a button alone could never handle.

### 3. The Capacitor and Fade Effect
A capacitor stores electrical charge — like a tiny rechargeable battery.

```
Button pressed:   +5V fills the capacitor with charge
Button released:  capacitor slowly drains its charge through the 10KΩ resistor into the transistor Base
                  → transistor stays on a little longer, then gradually switches off → LED fades
```

The fade happens because the capacitor does not discharge instantly — it drains slowly through the 10KΩ resistor. The larger the capacitor, the longer it takes to drain, and the longer the fade.

This RC (Resistor-Capacitor) timing principle is used everywhere in electronics — in timing circuits, audio filters, camera flashes, and oscillators.

### 4. Why the 10KΩ Resistor at the Base?
The Base pin is sensitive — too much current will damage the transistor. The 10KΩ resistor limits the base current to a safe level (~0.4mA) while still being enough to switch the transistor fully on.

---

## Identifying Your Components

**Transistor — PN2222 or S8050**
Hold with flat face toward you, legs down → left to right: E · B · C

**Capacitor — 100µF electrolytic**
```
Longer leg = (+) positive  →  connects toward the button/signal side
Shorter leg or stripe = (−) negative  →  connects to GND
```
Electrolytic capacitors are polarized — reversing them will damage the component.

**Resistor color bands:**
```
220Ω  →  Red · Red · Brown · Gold
10KΩ  →  Brown · Black · Orange · Gold
```

---

## Breadboard Wiring — Step by Step

**Step 1 — Power**
- Mega 5V pin → red power rail
- Mega GND pin → blue power rail

**Step 2 — Place the transistor**
Plant transistor across 3 separate rows. Flat face toward you. Note which row is E, B, C.

**Step 3 — LED and 220Ω**
- LED long leg (+) → 5V rail
- LED short leg (−) → one end of 220Ω resistor
- Other end of 220Ω → same row as Collector (C, right leg of transistor)

**Step 4 — Emitter to GND**
- Wire from Emitter (E, left leg) → GND rail

**Step 5 — Button**
- Straddle button across center gap of breadboard (use diagonal corner legs)
- One side → 5V rail

**Step 6 — Junction point**
The output side of the button connects to 3 things in the same breadboard row:
- 10KΩ resistor → Base (B, middle leg of transistor)
- Capacitor (+) long leg
- Wire from button output

**Step 7 — Capacitor to GND**
- Capacitor (−) short leg → GND rail

---

## Test It

1. Power on the Mega (no sketch needed — just plug in USB)
2. Press and hold the button → LED lights up
3. Release the button → LED slowly fades out over ~1 second

---

## Tuning the Fade

Swap the capacitor to change how long the fade lasts:

| Capacitor | Fade time |
|-----------|-----------|
| 47µF | ~0.5 seconds |
| 100µF | ~1 second |
| 220µF | ~2 seconds |

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| LED always on | Transistor legs in wrong order — check E·B·C orientation |
| LED always off | Check 220Ω is between LED and Collector, not going to GND |
| No fade, turns off instantly | Capacitor in wrong orientation — flip it |
| Very dim LED | Wrong resistor — recheck 220Ω color bands |
| Button does nothing | Check button straddles center gap, using diagonal corner legs |
