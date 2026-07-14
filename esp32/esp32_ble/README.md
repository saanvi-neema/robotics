# ESP32 Bluetooth (BLE) Peripheral

This project makes the ESP32 show up as a Bluetooth device your phone can find, connect to, read data from, write data to, and get live updates from — no WiFi network or internet needed at all, just Bluetooth.

## Bluetooth vs. Bluetooth *Low Energy*

Regular Bluetooth (like what connects your headphones) is built for streaming continuous audio. **BLE** (Bluetooth Low Energy) is a different, simpler protocol designed for small, occasional bits of data using very little battery — the same tech behind fitness trackers, smart locks, and IoT sensors. This project uses BLE.

## Key BLE vocabulary (this is the confusing part, so read carefully)

- **Peripheral**: the small, low-power device advertising itself and waiting for a connection. That's our ESP32 here.
- **Central**: the device that scans for and connects to peripherals. That's your phone, running an app like nRF Connect.
- **Service**: a logical grouping of related data, identified by a unique ID called a **UUID** (a long, essentially-random string like `4fafc201-1fb5-459e-8fcc-c5c9c331914b` — long enough that two random services will basically never collide).
- **Characteristic**: a single piece of data *inside* a service — also identified by its own UUID. Think of a Service like a folder, and Characteristics like the files inside it.
- **Properties**: what you're allowed to do with a characteristic — `READ` (central can ask for the current value), `WRITE` (central can change it), `NOTIFY` (peripheral can push updates without being asked).

## What this project's Bluetooth "menu" looks like

- **Device name**: `ESP32-S3 BLE Demo` (this is what shows up when you scan for nearby devices)
- **One Service**, containing:
  - **One Characteristic** that is `READ` + `WRITE` + `NOTIFY`, holding a text value

When a phone connects:
- Reading the characteristic initially shows `"Hello from ESP32-S3"`
- With notifications enabled, it updates to `"Counter: 1"`, `"Counter: 2"`, etc. every 2 seconds, live
- Writing any text to it shows up in the ESP32's serial monitor

## The code, explained

Open `src/main.cpp`:

```cpp
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
```
These specific UUIDs don't mean anything special — they're commonly used placeholder values from Espressif's own example code. In a real product you'd generate your own random ones (there are free UUID generator websites for this), just so nobody else's device accidentally uses the same IDs.

```cpp
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { deviceConnected = true; ... }
    void onDisconnect(BLEServer* pServer) { deviceConnected = false; ...; pServer->getAdvertising()->start(); }
};
```
BLE is **event-driven** — instead of constantly checking "is anyone connected yet?", you write functions that get automatically called *when* something happens. `onConnect` fires the instant a phone connects; `onDisconnect` fires when it disconnects. Notice `onDisconnect` restarts advertising — without this, once one phone disconnected, the ESP32 would go silent and no other phone could ever find it again.

```cpp
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
        Serial.print("Client wrote: ");
        Serial.println(pChar->getValue().c_str());
    }
};
```
Same event-driven idea: `onWrite` fires automatically whenever a connected phone writes a new value to our characteristic.

```cpp
void setup() {
  BLEDevice::init("ESP32-S3 BLE Demo");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("Hello from ESP32-S3");

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
}
```
Step by step: name the device → create a server → create a service inside it → create a characteristic inside *that*, with the three properties we want → start the service → start **advertising** (broadcasting "I exist!" over Bluetooth so phones scanning nearby can see it).

The `BLE2902` descriptor is a required piece of BLE plumbing — it's what lets a phone app actually turn notifications on/off for a characteristic. Every `NOTIFY` characteristic needs one.

```cpp
void loop() {
  if (deviceConnected) {
    counter++;
    pCharacteristic->setValue(("Counter: " + String(counter)).c_str());
    pCharacteristic->notify();
  }
  delay(2000);
}
```
Every 2 seconds, if someone's connected, update the value and call `notify()` — this actively pushes the new value to the connected phone (as opposed to `READ`, where the phone has to ask).

## Hardware needed

Just the ESP32 board. No wiring, no WiFi network required.

## How to build, upload, and test it

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -d "C:\Users\Saanvi\comp-sci\esp32\esp32_ble" --target upload --upload-port COM3
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" device monitor -p COM3 -b 115200
```

Then on your phone, install a free BLE scanner app — **nRF Connect** or **LightBlue** (both on iOS and Android) — scan for nearby devices, find `ESP32-S3 BLE Demo`, and connect. Tap into the one characteristic, enable notifications, and watch the counter tick up live.

## What to try next

- Add a second characteristic, e.g. one that reads a fake "temperature" value.
- Make the counter actually mean something — hook it up to a button press count, or a sensor reading.
- Look up "BLE UART service" — a common pattern for sending free-text messages back and forth between phone and ESP32, like a tiny chat.
