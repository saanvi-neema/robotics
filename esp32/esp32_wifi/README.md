# ESP32 WiFi + Web Server

This project makes the ESP32 join your home WiFi network — just like your phone or laptop does — and then run its own tiny website that you can visit from any browser on the same network.

## What it actually does

1. On power-up, the board connects to your WiFi router using a network name (SSID) and password, exactly like typing your WiFi password into a new phone.
2. Once connected, the router hands it an **IP address** — a unique number that identifies it on the network (like a street address, but for computers). Ours showed up as `192.168.68.56`.
3. The board then starts a tiny web server — the same kind of software that runs Google or YouTube, just extremely simplified — listening for anyone who tries to visit its IP address in a browser.
4. When you type `http://192.168.68.56` into a browser, the ESP32 responds with a simple HTML page saying "ESP32 is alive! Connected to WiFi."

## Why this matters

This is the foundation for basically every "smart" gadget you own — smart bulbs, thermostats, doorbells. They're all just small computers that join your WiFi and run a tiny web server (or talk to one), exactly like this project.

## The code, explained

Open `src/main.cpp`:

```cpp
const char* WIFI_SSID = "hem-saanvi-deco";
const char* WIFI_PASSWORD = "tGarmuwkak22!";
```
Your network name and password, in plain text.

> **Before pushing this to GitHub (or sharing it anywhere):** replace these two values with your own network's name/password, or better, with obvious placeholders like `"YOUR_WIFI_SSID"` / `"YOUR_WIFI_PASSWORD"` that you fill in locally after cloning. Anyone who can read this file can read your WiFi password — never commit real credentials to a public repo.

```cpp
WebServer server(80);
```
Creates a web server that will listen on **port 80** — the standard port web browsers use by default when you don't type one explicitly (that's why `http://192.168.68.56` works without needing `http://192.168.68.56:80`).

```cpp
void handleRoot() {
  server.send(200, "text/html", "<h1>ESP32 is alive!</h1><p>Connected to WiFi.</p>");
}
```
This function decides what to send back when someone visits the homepage (`/`). `200` is the HTTP status code for "success" (you may have seen `404` before — that means "not found"). The rest is just an HTML snippet, the same language every website is built from.

```cpp
void setup() {
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}
```
- `WiFi.begin(...)` starts the connection attempt.
- The `while` loop just waits and prints dots until `WiFi.status()` reports `WL_CONNECTED` — connecting isn't instant, so we keep checking.
- `WiFi.localIP()` is the address the router assigned us — this gets printed so you know what to type into your browser.
- `server.on("/", handleRoot)` says "when someone visits the root page `/`, run the `handleRoot` function."
- `server.begin()` actually starts listening for visitors.

```cpp
void loop() {
  server.handleClient();
}
```
Unlike the blink project, `loop()` here doesn't do the work directly — it just repeatedly calls `server.handleClient()`, which checks "has anyone tried to visit my website since the last check?" and responds if so. This has to run constantly, which is why it's the *only* thing in `loop()`.

## Hardware needed

Just the ESP32 board and a WiFi network to join. No extra wiring.

## How to build, upload, and test it

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -d "C:\Users\Saanvi\comp-sci\esp32\esp32_wifi" --target upload --upload-port COM3
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" device monitor -p COM3 -b 9600
```

Watch the serial monitor for a line like `Connected! IP address: 192.168.68.56`, then open that address in a browser on a device connected to the **same WiFi network**.

## A gap this project has (on purpose, to keep it simple)

The code only calls `WiFi.begin()` once, in `setup()`. If the WiFi ever drops (router restarts, you walk out of range briefly), it won't automatically reconnect — it'll just sit there disconnected until you power-cycle the board. A more robust version would check `WiFi.status()` in `loop()` and call `WiFi.begin()` again if it ever comes back as disconnected.

## What to try next

- Add a second page (e.g. `/about`) with its own `server.on(...)` handler.
- Make the homepage show something dynamic, like how many seconds the board has been running (`millis() / 1000`).
- Add a button on the webpage that, when clicked, makes a GPIO pin turn an LED on/off (this is exactly how "smart plug" apps work).
