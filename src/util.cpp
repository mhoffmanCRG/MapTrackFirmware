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
  