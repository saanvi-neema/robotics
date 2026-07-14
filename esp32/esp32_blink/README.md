# ESP32 Blink

The "Hello World" of electronics: make an LED turn on and off, forever, using code instead of a light switch. This is always the first thing to try with a new microcontroller board — if the blink works, you know your board, your drivers, your USB cable, and your upload process all work.

## What it actually does

Every 500 milliseconds (half a second), the board:
1. Turns the LED **on**
2. Prints `"LED on"` over the serial connection (so you can watch it on your computer)
3. Waits half a second
4. Turns the LED **off**
5. Prints `"LED off"`
6. Waits half a second
7. Repeats forever

## The code, explained line by line

Open `src/main.cpp`. Here's what each part means:

```cpp
const int LED_PIN = 2;
```
The ESP32 has dozens of pins (the metal legs sticking out of the chip), and each one has a number called a **GPIO number** (GPIO = "General Purpose Input/Output" — a fancy way of saying "a pin you can control from code"). We're telling the code "the LED I care about is wired to GPIO 2." On most ESP32 dev boards, GPIO2 happens to be connected to a small LED already built onto the board, so you don't even need to wire anything up.

```cpp
void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}
```
`setup()` runs **once**, right when the board powers on or resets. Two things happen here:
- `pinMode(LED_PIN, OUTPUT)` tells the chip "I'm going to use this pin to *send out* a signal (turn things on/off), not to *read* one." Pins can be configured as `OUTPUT` (control something) or `INPUT` (read a button, sensor, etc.) — you have to say which, up front.
- `Serial.begin(9600)` opens a communication channel back to your computer over the USB cable, running at a speed of 9600 "baud" (bits per second). This is what lets `Serial.println(...)` show up in the Serial Monitor on your PC.

```cpp
void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED on");
  delay(500);

  digitalWrite(LED_PIN, LOW);
  Serial.println("LED off");
  delay(500);
}
```
`loop()` runs **over and over, forever**, right after `setup()` finishes. This is where the actual blinking happens:
- `digitalWrite(LED_PIN, HIGH)` sends electrical power to the pin — like flipping a switch to "on." `HIGH` means roughly 3.3 volts; `LOW` means 0 volts (off).
- `delay(500)` pauses the program for 500 milliseconds. Nothing else happens during a `delay()` — the chip is just waiting.
- Then it does the same thing in reverse to turn the LED off.

Since `loop()` runs endlessly, this on/half-second/off/half-second pattern repeats forever, which is what makes it "blink."

## Hardware needed

Just the ESP32 board itself and a USB cable. No wiring required — GPIO2's onboard LED does the job. (If your specific board's LED is wired to a different pin, change the `2` in `const int LED_PIN = 2;` to match.)

## Files in this project

- `platformio.ini` — tells PlatformIO which board and framework to build for (`esp32-s3-devkitc-1`, Arduino framework)
- `src/main.cpp` — the actual code (what's explained above)
- `esp32_blink.ino` — an identical copy of the same code, kept around so this project can also be opened directly in the Arduino IDE (which expects a `.ino` file, not `src/main.cpp`)

## How to build and upload it

This project uses [PlatformIO](https://platformio.org/), a build tool that works from the command line (instead of the Arduino IDE's buttons).

```powershell
# Build (compile) the code, without uploading
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -d "C:\Users\Saanvi\comp-sci\esp32\esp32_blink"

# Build AND upload to the board (replace COM3 with your board's port)
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -d "C:\Users\Saanvi\comp-sci\esp32\esp32_blink" --target upload --upload-port COM3

# Watch what the board is printing over serial
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" device monitor -p COM3 -b 9600
```

(Not sure which COM port your board is on? Run `pio device list` to see all connected boards.)

## What to try next

- Change `delay(500)` to `delay(100)` and re-upload — the LED should blink much faster.
- Change `LED_PIN` to a different GPIO number and wire up your own external LED (with a resistor!) to that pin instead of using the onboard one.
- Try making it blink in a pattern, like Morse code, by changing how long each `HIGH`/`LOW` lasts.
