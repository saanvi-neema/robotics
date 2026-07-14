# Arduino Projects

Welcome! This is a collection of Arduino projects built using the **Elegoo Mega 2560 Most Complete Starter Kit**. If you're new to Arduino, think of it as a small programmable computer that can talk to the physical world — sensors, motors, displays, lights — through its pins.

---

## The Board — Elegoo Mega 2560

The Mega 2560 is like a supercharged Arduino. It has more pins and more memory than the basic Arduino Uno, which means you can connect more components at once without running out of space.

The kit comes loaded with components to experiment with:

| Component | What it does |
|-----------|-------------|
| 830-point breadboard | A reusable board for building circuits without soldering |
| DHT11 sensor | Measures temperature and humidity |
| LCD1602 display | A small 16x2 character screen |
| Ultrasonic sensor (HC-SR04) | Measures distance using sound waves (like bat sonar) |
| Servo motor (SG90) | A small motor that rotates to a precise angle |
| Stepper motor | A motor that moves in exact steps — great for precise positioning |
| PIR motion sensor | Detects movement (like a security light) |
| Gyroscope/accelerometer (GY-521) | Measures tilt and movement in 3D space |
| IR receiver + remote | Like a TV remote — send commands wirelessly |
| Keypad | A small 4x4 button pad for entering numbers/commands |
| RTC module | A real-time clock that keeps track of date and time |
| MAX7219 LED matrix | An 8x8 grid of LEDs for displaying patterns or text |
| Buzzer, LEDs, resistors, potentiometers | The basic building blocks of electronics |

---

## Projects

- [LCD_test](LCD_test/README.md) — Simplest possible LCD wiring test, Hello World
- [weather_station](weather_station/README.md) — Reads temperature and humidity, displays on an LCD screen
- [robot_car](robot_car/README.md) — A robot car controlled by typing commands into your computer
- [servo_test](servo_test/README.md) — Control a servo motor by typing angles into your computer

---

## Breadboard Quick Reference

A breadboard lets you connect components using jumper wires without any soldering. Here's how this one is laid out:

```
- ════════════════════════════════════════════ -   top rail (GND / negative)
+ ════════════════════════════════════════════ +   top rail (5V / positive)

     1    2    3  ...  15  16  17  ... 48  49 ... 63
j   [.]  [.]  [.]     [D] [D] [D]    [.]  [.]    [.]   ← top row
i   [.]  [.]  [.]     [D] [D] [D]    [.]  [.]    [.]
h   [.]  [.]  [.]     [.]  [.]  [.]  [.]  [.]    [.]
g   [.]  [.]  [.]     [.]  [.]  [D]  [.]  [.]    [.]
f   [.]  [.]  [.]     [.]  [.]  [.]  [.]  [.]    [.]
    ──────────────────── center gap ─────────────────
e   [.]  [.]  [.]     [.]  [.]  [.]  [.]  [.]    [.]
d   [.]  [.]  [.]     [.]  [.]  [.]  [.]  [.]    [.]
c   [.]  [.]  [.]     [.]  [.]  [.]  [.]  [.]    [.]
b   [.]  [.]  [.]     [.]  [.]  [.]  [L] [L] ... [L]
a   [.]  [P]  [.] [P] [.]  [.]  [.]  [L] [L] ... [L]   ← bottom row
                                      48            63
- ════════════════════════════════════════════ -   bottom rail (GND)
+ ════════════════════════════════════════════ +   bottom rail (5V)
```

**Key rule:** holes in the same column AND same side of the center gap are connected internally. So if you plug a component into `a5` and a wire into `b5`, they are electrically connected — no extra wire needed between them.

See [weather_station/README.md](weather_station/README.md) for the full component key and wiring details.
