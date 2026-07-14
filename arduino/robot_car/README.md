# Robot Car

A robot car you control by typing commands into your computer over USB. Type `F` and it goes forward, `B` and it reverses, `L` and `R` for turning, `S` to stop. You can also control a servo motor (think of it like a rotating arm or camera mount) by typing an angle between 0 and 180.

---

## How It Works

The Arduino receives commands through the **Serial Monitor** (a text window in the Arduino IDE). When it sees a letter like `F`, it activates the motors in the right direction. When it sees a number like `90`, it rotates the servo to that angle.

The motors are driven by a **Motor Shield REV3** — a board that sits on top of the Arduino and gives it the ability to control motors. The Arduino itself can't power motors directly (not enough current), so the motor shield acts as a middleman — the Arduino tells it what to do, and the shield handles the actual power.

Each motor has three control signals:
- **Direction pin** — HIGH or LOW to set which way the motor spins
- **PWM pin** — controls speed (0 = stopped, 255 = full speed)
- **Brake pin** — HIGH to stop the motor instantly

---

## Hardware
- Arduino with Motor Shield REV3
- 2x DC motors (Motor A = left wheel, Motor B = right wheel)
- 1x Servo motor (connected to pin 6)

---

## Pin Assignments

| Component | Pin | Purpose |
|-----------|-----|---------|
| Servo | 6 | Rotates 0–180° |
| Motor A direction | 12 | Left wheel forward/backward |
| Motor A speed (PWM) | 3 | Left wheel speed |
| Motor A brake | 9 | Left wheel stop |
| Motor B direction | 13 | Right wheel forward/backward |
| Motor B speed (PWM) | 11 | Right wheel speed |
| Motor B brake | 8 | Right wheel stop |

> **Why pin 6 for servo?** Pin 9 is already used by the Motor Shield for the Motor A brake. Using it for the servo too would cause a conflict, so the servo gets pin 6 instead.

---

## Serial Commands

Open the Arduino IDE Serial Monitor at **9600 baud** and type these:

| Command | Action |
|---------|--------|
| `F` | Forward |
| `B` | Backward |
| `L` | Turn left |
| `R` | Turn right |
| `S` | Stop |
| `0` to `180` | Rotate servo to that angle |

---

## Notes
- Motor speed is set to 200 out of 255 — about 78% power. Change `MOTOR_SPEED` in the code to adjust.
- Serial baud rate: 9600
