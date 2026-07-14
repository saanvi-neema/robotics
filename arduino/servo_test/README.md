# Servo Test

A simple sketch to control a servo motor by typing angles into your computer. Great starting point for understanding how servos work before using them in bigger projects.

---

## What is a Servo Motor?

A servo is a small motor that rotates to a specific angle and holds it there. Unlike a regular DC motor that just spins freely, a servo has a built-in control system that lets you say "go to 90 degrees" and it will move there precisely and stay put.

Servos are used everywhere — RC cars (steering), robotic arms, camera gimbals, and more.

The angle range is **0 to 180 degrees**:
- `0` = all the way to one side
- `90` = center / neutral position
- `180` = all the way to the other side

The Arduino communicates the angle to the servo using a signal called **PWM** (Pulse Width Modulation) — basically a series of pulses where the width of each pulse tells the servo what angle to go to. The `Servo.h` library handles all of this for you.

---

## Hardware
- Arduino (any model)
- Servo motor connected to **pin 9**

Servo wires:
| Wire color | Connect to |
|------------|------------|
| Brown / Black | GND |
| Red | 5V |
| Orange / Yellow | Pin 9 (signal) |

---

## How to Use

1. Upload the sketch to your Arduino
2. Open the **Serial Monitor** in Arduino IDE (top right magnifying glass icon)
3. Set baud rate to **9600**
4. Type any number between `0` and `180` and press Send
5. The servo will rotate to that angle and print a confirmation message

---

## Good Angles to Try
- `0` — full left
- `90` — center
- `180` — full right
- `45`, `135` — quarter positions

This is a great first project to verify your servo is working before wiring it into something more complex.
