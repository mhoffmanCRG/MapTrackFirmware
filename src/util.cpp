#include <Arduino.h>
#include "pins.h"

void blink(unsigned int longCount, unsigned int shortCount) {
  // Long flashes
  for (unsigned int i = 0; i < longCount; i++) {
    digitalWrite(LED, HIGH);
    delay(20);
    digitalWrite(LED, LOW);
    delay(500);       // off-time same as long on-time
  }

  for (unsigned int i = 0; i < shortCount; i++) {
      digitalWrite(LED, HIGH);
      delay(20);
      digitalWrite(LED, LOW);
      delay(200);       // off-time same as long on-time
  }
  delay(750);
}
  
uint32_t getChipId() {
  uint64_t chipId = ESP.getEfuseMac() ;
  uint32_t chipId32 =  chipId >> 32;
  return chipId32;
}

// Generic hex dump function
void printHex(const void* data, size_t length) {
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);

  Serial.print("Data (");
  Serial.print(length);
  Serial.print(" bytes):");

  for (size_t i = 0; i < length; i++) {
    if (bytes[i] < 0x10) {
      Serial.print("0");  // leading zero for single-digit hex
    }
    Serial.print(bytes[i], HEX);
    Serial.print(" ");
    // if ((i + 1) % 16 == 0) Serial.println();  // line break every 16 bytes
  }
  Serial.println();
}