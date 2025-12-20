#pragma once
#include <cstdint>

// struct packetStruct {
//     volatile          short int type = 0; //2
//     volatile          byte id; //1
//     volatile          byte packet; //1
//     volatile          long lat; //4
//     volatile          long lng; //4
//     volatile          long speed; //4
//     volatile          long heading; //4
//   };

typedef struct {
  uint8_t b[3];   // MSB first
} uint24_t;

struct __attribute__((packed)) packetStruct {
  uint32_t senderId;   // 4 bytes (aligned naturally)
  uint8_t packetType;  // 1 byte
  uint8_t packetCnt;   // 1 byte

  int32_t lat; 
  int32_t lng; 
  int8_t speed;
  int8_t heading;

  int32_t crc;       // 4 bytes (aligned naturally)

  // uint8_t powerOn : 1; // 1-bit boolean
  // uint8_t coolerOn : 1; // 1-bit boolean
  // uint8_t heaterOn : 1; // 1-bit boolean
  // uint8_t pumpOn : 1; // 1-bit boolean
  // uint8_t reserved : 4; // Unused bits (ensures 1 full byte)

  // int16_t tempSet;    // *10 - 2 bytes 
  // int16_t tempVal;    // *100 - 2 bytes 
  // uint8_t tempDiff;   // *10 - 1 bytes
};


extern packetStruct p;