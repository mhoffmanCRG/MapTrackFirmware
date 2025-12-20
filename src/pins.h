#ifndef PINS_H
#define PINS_H

    #define ADXL_INT 2    // Must be RTC-capable pin for wakeup // Shared with SS
    #define SDA_PIN 0
    #define SCL_PIN 1

    #define RST 18
    #define DIO1 10//8//3
    #define BUSY 4
    #define SCK  9//10
    #define MISO 6
    #define MOSI 7
    #define SS 2//8 // Shared with ADXL_INT

    #define GNSS_EXTINT 19
    #define GNSS_PULSE 5

    #define LED 8
#endif