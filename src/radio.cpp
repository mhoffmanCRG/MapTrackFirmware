#include <RadioLib.h>
#include <SPI.h>
#include <ArduinoJson.h>

#include "radio.h"
#include "pins.h"
#include "packet.h"
#include "esp_crc.h"
#include "ble.h"
#include "util.h"

SX1262 radio = new Module(SS, DIO1, RST, BUSY, SPI);

int cnt=0;

volatile bool receivedFlag = false;

void dataReceived(void) {
  // Serial.println("Data received...");
  Serial.print("~");
  receivedFlag = true;
}

void radioSetup() {
  Serial.begin(115200);
  SPI.begin(SCK, MISO, MOSI, SS);
  
  // initialize SX1262 with default settings
  Serial.println(("[SX1262] Initializing ... "));
//
const float ch[] = {868.1, 868.3, 868.5, 867.1, 867.3, 867.5, 867.7, 867.9};
//Carrier frequency: 434.0 MHz
const float bw = 125.0;  //Bandwidth: 125.0 kHz (dual-sideband)
const int sf = 11;//Spreading factor: 12
const int cr = 5;//Coding rate: 4/7
const int sw = RADIOLIB_SX126X_SYNC_WORD_PUBLIC;//Sync word: SX126X_SYNC_WORD_PRIVATE (0x12)
const int po = 10;//Output power: 10 dBm max 22
const int pe = 12;//Preamble length: 8 symbols
const float rv = 0.0;//1.6; //TCXO reference voltage: 1.6 V (SX126x module with TCXO)

int state = radio.begin(ch[7], bw, sf, cr, sw, po, pe, rv, false);

radio.setDio1Action(dataReceived);

// radio.setCurrentLimit(50);
//radio.explicitHeader();
radio.setCRC(true);
radio.startReceive();

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
  //  while (true);
  } 
}



void processPacket() {
  receivedFlag = false;
  int len = radio.getPacketLength();
  float rssi = radio.getRSSI();
  float snr = radio.getSNR();

      uint8_t byteArr[255];
      int numBytes = radio.getPacketLength();
      int state = radio.readData(byteArr, numBytes);

      //printHex(&byteArr, numBytes);
  
  // Serial.printf("PacketSize: %d, snr: %f, rssi: %f\n", len, snr, rssi);

  if (len == sizeof(locationStruct)) {
    locationStruct loc;
    // bool isValidPacket = receiveStruct(radio, loc);
    memcpy(&loc, byteArr, numBytes);

    bool isValidPacket = checkCRC(loc);

    uint8_t packetData[sizeof(locationStruct)-sizeof(uint32_t)];
    memcpy(packetData, &loc, sizeof(packetData));
    uint32_t crc = esp_crc32_le(0x00, packetData, sizeof(packetData));
    isValidPacket = (loc.crc == crc);

    if(loc.senderId != senderId) {
      // printHex(&loc, sizeof(loc));
      String locJSON = locationStructToJson(loc, isValidPacket, rssi, snr);
      bleNotifyLoc(locJSON);
    }

  }
  radio.startReceive();
}

bool checkCRC(locationStruct packet) {
  uint8_t packetData[sizeof(locationStruct)-4];
  memcpy(packetData, &packet, sizeof(packetData));
  uint32_t crc = esp_crc32_le(0x00, packetData, sizeof(packetData));
  return packet.crc == crc;
} 

void radioLoop() {
  static unsigned long lastTxTime = 0;
  static unsigned long txIntervalMs = 1000;
  unsigned long now = millis();

  if(receivedFlag) {
    processPacket();
  }

  // if(senderId == 5152) {
  //   return;
  // }

  // Only transmit if 30 seconds have passed
  if (now - lastTxTime < txIntervalMs) {
    return;
  }

  lastTxTime = now;
  txIntervalMs = random(12000, 18000); // randomize next interval between 12-18 seconds
  

  Serial.print(F(">"));


  // // Calculate CRC and add to packetCnt
  // p.senderId = getChipId();
  // p.packetCnt = p.packetCnt + 1;
  
  // uint8_t packetData[sizeof(p)-4];
  // memcpy(packetData, &p, sizeof(packetData));
  // uint32_t crc = esp_crc32_le(0x00, packetData, sizeof(packetData));
  // p.crc = crc;
  

  // digitalWrite(LED, HIGH);
  
  // byte *charpointer = (byte*)&p;
  // unsigned long txnow = millis();
  // int state = radio.transmit(charpointer, sizeof(p));
  // Serial.print("timeOnAir " );
  // Serial.println( millis() - txnow);
  // digitalWrite(LED, LOW);
  // delay(100);
  // receivedFlag=false;
  // radio.startReceive();

    locationStruct tx;

    tx.senderId = getChipId();
    tx.packetCnt = packetCnt++;
    tx.lat = lat;
    tx.lng = lng;
    tx.speed = speed;
    tx.heading = heading;
    tx.packetType = 1;

    // tx.crc = esp_crc32_le(
    //   0,
    //   (uint8_t*)&tx,
    //   offsetof(locationStruct, crc)
    // );

    uint8_t packetData[sizeof(locationStruct)-sizeof(uint32_t)];
    memcpy(packetData, &tx, sizeof(packetData));
    uint32_t crc = esp_crc32_le(0x00, packetData, sizeof(packetData));
    tx.crc = crc;

    digitalWrite(LED, HIGH);

    int state = radio.transmit((uint8_t*)&tx, sizeof(tx));

    digitalWrite(LED, LOW);

    if (state == RADIOLIB_ERR_NONE) {
      p.packetCnt = tx.packetCnt;   // commit counter only
    }

    receivedFlag = false;
    radio.startReceive();



  if (state == RADIOLIB_ERR_NONE) {
    // the packet was successfully transmitted

    // print measured data rate
    // Serial.print(F("[SX1278] Datarate:\t"));
    // Serial.print(radio.getDataRate());
    // Serial.print(F(" bps"));

  } else if (state == RADIOLIB_ERR_PACKET_TOO_LONG) {
    // the supplied packet was longer than 256 bytes
    Serial.println(F("too long!"));
    blink(2,4);
    

  } else if (state == RADIOLIB_ERR_TX_TIMEOUT) {
    // timeout occurred while transmitting packet
    Serial.println(F("timeout!"));
    blink(2,3);

  } else {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);
    blink(2,5);    
  } 
}


String locationStructToJson(const locationStruct& p, bool isValid, float rssi, float snr) {
  JsonDocument doc;  // allocate 256 bytes on the heap


  doc["senderId"]     = p.senderId;
  doc["rssi"]         = rssi;
  doc["snr"]          = snr;
  doc["packetCnt"]    = p.packetCnt;
  doc["valid"]        = isValid;
  doc["packetType"]   = p.packetType;
  
  doc["latitude"]     = p.lat / 10000000.0;
  doc["longitude"]    = p.lng / 10000000.0;
  doc["speed"]        = p.speed;
  doc["heading"]      = p.heading;
  // doc["linkedDevice"] = senderId == p.senderId ? true : false ;

  doc["version"]      = 2512281500; // YYYYMMDDHH
  doc["id"]           = p.crc;
  
  
  String json;
  serializeJson(doc, json);
  Serial.println (json);
  return json;
}

