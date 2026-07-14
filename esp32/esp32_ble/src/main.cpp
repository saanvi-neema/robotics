// Advertises the ESP32-S3 as a Bluetooth Low Energy peripheral with one
// characteristic that's readable, writable, and notifies a counter every
// 2 seconds while a client is connected. Connect with a phone BLE scanner
// app (e.g. nRF Connect, LightBlue) to see it and interact with it.

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected, restarting advertising");
    pServer->getAdvertising()->start();
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    Serial.print("Client wrote: ");
    Serial.println(pChar->getValue().c_str());
  }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("ESP32-S3 BLE Demo");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pCharacteristic->setValue("Hello from ESP32-S3");

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->start();

  Serial.println("BLE advertising started as 'ESP32-S3 BLE Demo', waiting for a client...");
}

uint32_t counter = 0;

void loop() {
  if (deviceConnected) {
    counter++;
    String value = "Counter: " + String(counter);
    pCharacteristic->setValue(value.c_str());
    pCharacteristic->notify();
  }
  delay(2000);
}
