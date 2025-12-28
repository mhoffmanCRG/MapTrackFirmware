#include <Arduino.h>
#include <Wire.h>
#include "esp_system.h"
#include "esp_task_wdt.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "packet.h"
#include "esp_sleep.h"
#include "acc.h" // ADXL345 setup and interrupt config
#include "gnss.h"
#include "radio.h"
#include "pins.h"
#include "ble.h"
#include "util.h"

locationStruct p;
volatile int32_t senderId;
volatile int32_t lat;
volatile int32_t lng;
volatile uint8_t speed;
volatile uint8_t heading;
volatile uint8_t packetCnt = 0;

void setup() {
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brown-out
  esp_task_wdt_init(10, true);   // 5s timeout
  esp_task_wdt_add(NULL);       // Add loop ta


  p.senderId = getChipId();
  senderId = getChipId();

  randomSeed(micros() ^ (unsigned long)ESP.getEfuseMac());

  delay(100); // wait for serial to initialize
  pinMode(LED, OUTPUT);

  blink(3,0); // Indicate startup
  

  accDisableInterrupts();
  Serial.begin(115200);

  // togglePinTest(GNSS_EXTINT);

  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

  Serial.print("Woke up from sleep wakeupReason: ");
  Serial.println(wakeupReason);

  if (wakeupReason == ESP_SLEEP_WAKEUP_GPIO) {
    Serial.println("Woke up from ADXL345 motion!");
    // After waking from motion, sleep again for 30 seconds
    // esp_sleep_enable_timer_wakeup(30 * 1000000ULL); // 30 sec
  } else if (wakeupReason == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Woke up from timer!");
    // After timer, enable ADXL345 pin wakeup again
    // esp_sleep_enable_timer_wakeup(30 * 1000000ULL); // 30 sec
    // esp_deep_sleep_enable_gpio_wakeup((1ULL << ADXL_INT), ESP_GPIO_WAKEUP_GPIO_HIGH);
  } else {
    Serial.println("Normal boot or unknown wakeup");
    // On first boot, enable ADXL345 wakeup
    // esp_sleep_enable_timer_wakeup(30 * 1000000ULL); // 30 sec
    // esp_deep_sleep_enable_gpio_wakeup((1ULL << ADXL_INT), ESP_GPIO_WAKEUP_GPIO_HIGH);
  }


  
  Wire.begin(SDA_PIN,SCL_PIN); 
  gnssSetup();
  //accSetup();
  radioSetup();
  bleSetup();

    // On first boot, enable ADXL345 wakeup
    esp_sleep_enable_timer_wakeup(300 * 1000000ULL); // 30 sec
    esp_deep_sleep_enable_gpio_wakeup((1ULL << ADXL_INT), ESP_GPIO_WAKEUP_GPIO_HIGH);
}

void deepSleep() {
  gnssSleep();
  accEnableInterrupts();
  Serial.println("Enter deep sleep...");
  esp_deep_sleep_start(); // Go back to sleep
}

void loop() {
  gnssLoop();
  radioLoop();
  //deepSleep();
  delay(100);
  // Feed watchdog
  esp_task_wdt_reset();
}




