#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include "ble.h"


NimBLECharacteristic* pCharacteristic;
bool deviceConnected = false;


// Server callbacks
class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) override {
        deviceConnected = true;
        Serial.println("Device connected");
    }
    void onDisconnect(NimBLEServer* pServer) override {
        deviceConnected = false;
        Serial.println("Device disconnected");
    }
};

void bleSetup() {
    Serial.begin(115200);

    // Initialize NimBLE
    NimBLEDevice::init("ESP32C3_JSON");

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
    );

    // Start service
    pService->start();

    // Start advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    Serial.println("BLE server started, waiting for connections...");
}



void bleNotifyLoc(String jsonString) {
    pCharacteristic->setValue(jsonString);
    pCharacteristic->notify();
}

