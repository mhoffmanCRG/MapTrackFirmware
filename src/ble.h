#pragma once
#include <Arduino.h>
// Define UUIDs for your service and characteristic
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcdefab-1234-5678-1234-abcdefabcdef"
#define WRITE_CHARACTERISTIC_UUID "fedcba98-7654-3210-7654-abcdefabcdef"


void bleSetup();
void bleNotifyLoc(String);