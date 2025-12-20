#include <Arduino.h>
#include <Wire.h>
#include <ADXL345_WE.h>
#include "esp_sleep.h"
#include "acc.h"
#include "pins.h"

#define ADXL_ADDR 0x53

ADXL345_WE myAcc = ADXL345_WE(ADXL_ADDR);


void accSetup() {
  if (!myAcc.init()) {
    Serial.println("ADXL345 not detected!");
    while (1);
  }

  //RESET TO DEFAULT
  Wire.beginTransmission(ADXL_ADDR);
  Wire.write(0x1E);   // OFSX register = reset command location
  Wire.write(0x52);   // Reset command
  Wire.endTransmission();

  myAcc.setMeasureMode(true);
  myAcc.setRange(ADXL345_RANGE_2G);



  // ---- Configure Activity Interrupt (Manual Register Access) ----
  Wire.beginTransmission(ADXL_ADDR);
  Wire.write(0x24);  // THRESH_ACT (activity threshold)
  Wire.write(25);     // Lower value for more sensitivity (≈ 5 * 62.5mg)
  Wire.endTransmission();

  Wire.beginTransmission(ADXL_ADDR);
  Wire.write(0x27);  // ACT_INACT_CTL
  Wire.write(0x70);  // Enable X,Y,Z activity detection
  Wire.endTransmission();

  Wire.beginTransmission(ADXL_ADDR);
  Wire.write(0x2E);  // INT_ENABLE
  //Wire.write(0x10);  // Enable ACTIVITY interrupt
  Wire.write(0x00);  // Disable ALL interrupts
  Wire.endTransmission();

  //SET INT1 TRIGGER TO LOW
  //WireSetBit(0x31, 5 , 1);

  accDisableInterrupts();
}

void accEnableInterrupts() {
 // WireSetBit(0x2E, 4 , true);
  pinMode(ADXL_INT, INPUT_PULLDOWN);
  Wire.beginTransmission(ADXL_ADDR);
  Wire.write(0x2E);  // INT_ENABLE
  Wire.write(0x10);  // Enable ACTIVITY interrupt
  Wire.endTransmission();

  //SET INT1 TRIGGER TO LOW
  WireSetBit(0x31, 5 , 1);
}

void accDisableInterrupts() {
  Wire.beginTransmission(ADXL_ADDR);
  Wire.write(0x2E);  // INT_ENABLE
  Wire.write(0x00);  // Disable ALL interrupts
  Wire.endTransmission();
  //SET INT1 TRIGGER TO LOW
  WireSetBit(0x31, 5 , 0);
}

void WireSetBit(int addr, uint8_t bit, bool bit_val) {
return;
  uint8_t reg_val = 0;

  // Step 1: read current value
  Wire.beginTransmission(ADXL_ADDR);
  Wire.write(addr);               // DATA_FORMAT register
  Wire.endTransmission(false);    // repeated start

  Wire.requestFrom(ADXL_ADDR, 1);
  if (Wire.available()) {
    reg_val = Wire.read();
  }

  // Step 2: modify only bit 5
  reg_val |= (1 << bit);   // set bit 5
  // reg_val &= ~(1 << 5); // (use this to clear bit 5)

  // Step 3: write back
  Wire.beginTransmission(ADXL_ADDR);
  Wire.write(addr);
  Wire.write(reg_val);
  Wire.endTransmission();
} 

void togglePinTest(char pin) {
  pinMode(pin, OUTPUT);

  unsigned long start = millis();

  while (millis() - start < 5000) {   // 10 seconds
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
  }

  // Optional: final state
  digitalWrite(pin, LOW);
  pinMode(pin, INPUT);
}