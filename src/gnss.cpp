#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> 
#include <MicroNMEA.h> 
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "gnss.h"
#include "pins.h"
#include "packet.h"
#include "ble.h"
#include "util.h"

SFE_UBLOX_GNSS myGNSS;
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

void gnssSleep() {
  myGNSS.powerOff(0);  // Go to deep sleep (backup mode)
  pinMode(GNSS_EXTINT, INPUT_PULLUP);  
  // EXTINT HIGH = no wake pulse
}

void gnsssWake() {
  pinMode(GNSS_EXTINT, OUTPUT);

  digitalWrite(GNSS_EXTINT, HIGH);
  delay(100);
  
  // send LOW pulse
  digitalWrite(GNSS_EXTINT, LOW);
  delay(1000);  // pulse width 2–10ms recommended
  
  // return to high so it doesn't retrigger
  digitalWrite(GNSS_EXTINT, HIGH);
  
  delay(100);  // allow GNSS to boot
}

void gnssSetup()
{
  gnsssWake();
  //myGNSS.enableDebugging(); // Uncomment this line to enable debug messages

  pinMode(GNSS_PULSE, INPUT);

  if (myGNSS.begin() == false) // Connect to the u-blox module
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }

  myGNSS.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA); // Set the I2C port to output UBX and NMEA

  // Create storage for the time pulse parameters
  UBX_CFG_TP5_data_t timePulseParameters;

  // Get the time pulse parameters
  if (myGNSS.getTimePulseParameters(&timePulseParameters) == false)
  {
    Serial.println(F("getTimePulseParameters failed! Freezing..."));
    while (1);
  }

  Serial.print(F("UBX_CFG_TP5 version: "));
  Serial.println(timePulseParameters.version);

  // --- Corrected configuration ---

  timePulseParameters.tpIdx = 0; // TIMEPULSE pin
  timePulseParameters.flags.bits.isFreq = 0;         // Period mode
  timePulseParameters.flags.bits.isLength = 1;       // Pulse length in microseconds
  timePulseParameters.flags.bits.polarity = 1;       // Rising edge active
  timePulseParameters.flags.bits.active = 1;         // Enable pulse when locked
  timePulseParameters.flags.bits.lockedOtherSet = 1; // Use locked params when GNSS is locked

  // When GNSS is *not locked* → no pulse at all
  timePulseParameters.freqPeriod = 0;
  timePulseParameters.pulseLenRatio = 0;

  // When GNSS is *locked* → 1 s pulse every 30 s
  timePulseParameters.freqPeriodLock = 30000000;   // 30 000 000 µs = 30 s
  timePulseParameters.pulseLenRatioLock = 1000000; // 1 000 000 µs = 1 s pulse

  // Apply new parameters
  if (myGNSS.setTimePulseParameters(&timePulseParameters) == false)
  {
    Serial.println(F("setTimePulseParameters failed!"));
  }
  else
  {
    Serial.println(F("Time pulse parameters set successfully."));
  }

  // Save configuration so it persists through reset
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT);

  Serial.println(F("GNSS time pulse configured."));
}


void gnssLoop()
{
  unsigned long start = millis();

  while (millis() - start < 5000) {   // x/1000 seconds
    myGNSS.checkUblox(); // Process incoming data
    if (nmea.isValid()) {

      lng = myGNSS.getLongitude();
      lat = myGNSS.getLatitude();
      speed = (uint8_t)(myGNSS.getGroundSpeed() * 0.0036);
      heading = myGNSS.getHeading()/100000/2; 

      // p.packetType = 1;
      // p.lat = myGNSS.getLatitude();
      // p.lng = myGNSS.getLongitude();
      // p.speed = (uint8_t)(myGNSS.getGroundSpeed() * 0.0036);
      // p.heading = myGNSS.getHeading()/100000/2;

      // locationStruct tmp;
      //   tmp.senderId = getChipId();
      //   tmp.packetType = 1;
      //   tmp.lat = myGNSS.getLatitude();
      //   tmp.lng = myGNSS.getLongitude();
      //   tmp.speed = (uint8_t)(myGNSS.getGroundSpeed() * 0.0036);
      //   tmp.heading = myGNSS.getHeading() / 100000 / 2;

      //   // atomic commit
      //   p = tmp;


      // long latitude_mdeg = nmea.getLatitude();
      // long longitude_mdeg = nmea.getLongitude();

      // Serial.print("Latitude (deg): ");
      // Serial.print(latitude_mdeg / 1000000.0, 6);
      // Serial.print("  Longitude (deg): ");
      // Serial.println(longitude_mdeg / 1000000.0, 6);

      nmea.clear();

      blink(0,1); // Indicate valid fix

      return;
    }
    else
    {
      delay(500);
      
      esp_task_wdt_reset();
    }
  }
    Serial.print("Waiting for fresh data: ");
    Serial.println((int)((millis() - start)/1000));
  blink(0,2); // Indicate no fix
}

// Pass incoming NMEA characters to MicroNMEA
void SFE_UBLOX_GNSS::processNMEA(char incoming)
{
  nmea.process(incoming);
}