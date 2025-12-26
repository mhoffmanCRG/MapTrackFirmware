#pragma once
#include <cstdint>

typedef struct {
  uint8_t b[3];   // MSB first
} uint24_t;

struct __attribute__((packed)) locationStruct {
  uint32_t senderId;   // 4 bytes (aligned naturally)
  uint8_t packetType;  // 1 byte
  uint8_t packetCnt;   // 1 byte

  int32_t lat; 
  int32_t lng; 
  uint8_t speed;
  uint8_t heading;

  uint32_t crc;       // 4 bytes (aligned naturally)
};


extern locationStruct p;
extern int32_t senderId;