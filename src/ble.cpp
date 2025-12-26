#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include "ble.h"

NimBLECharacteristic* pCharacteristic;       // notify characteristic
NimBLECharacteristic* pWriteCharacteristic;  // write characteristic
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

// Write callbacks
class MyWriteCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string value = pCharacteristic->getValue();
        if (!value.empty()) {
            Serial.print("Received from app: ");
            Serial.println(value.c_str());

            // Try to parse JSON
            StaticJsonDocument<128> doc;
            DeserializationError err = deserializeJson(doc, value);
            if (!err) {
                if (doc.containsKey("led")) {
                    bool ledState = doc["led"];
                    // digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
                    Serial.printf("LED set to: %s\n", ledState ? "ON" : "OFF");
                }
            } else {
                Serial.println("Invalid JSON received");
            }
        }
    }
};

void bleSetup() {
    Serial.begin(115200);
    // pinMode(LED_BUILTIN, OUTPUT);

    // Initialize NimBLE
    NimBLEDevice::init("ESP32C3_JSON");

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    // Notify characteristic (ESP32 → Cordova)
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
    );

    // Write characteristic (Cordova → ESP32)
    pWriteCharacteristic = pService->createCharacteristic(
        WRITE_CHARACTERISTIC_UUID,   // define in ble.h
        NIMBLE_PROPERTY::WRITE
    );
    pWriteCharacteristic->setCallbacks(new MyWriteCallbacks());

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
