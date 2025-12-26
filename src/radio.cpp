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
  Serial.println("Data received...");
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
const int cr = 7;//Coding rate: 4/7
const int sw = RADIOLIB_SX126X_SYNC_WORD_PUBLIC;//Sync word: SX126X_SYNC_WORD_PRIVATE (0x12)
const int po = 22;//Output power: 10 dBm max 22
const int pe = 12;//Preamble length: 8 symbols
const float rv = 0.0;//1.6; //TCXO reference voltage: 1.6 V (SX126x module with TCXO)


int state = radio.begin(ch[7], bw, sf, cr, sw, po, pe, rv, false);

radio.setDio1Action(dataReceived);

// radio.setCurrentLimit(50);
radio.setDio2AsRfSwitch(true);
//radio.explicitHeader();
radio.setCRC(2);
radio.startReceive();

  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
  //  while (true);
  }

  // some modules have an external RF switch
  // controlled via two pins (RX enable, TX enable)
  // to enable automatic control of the switch,
  // call the following method
  // RX enable:   19
  // TX enable:   18
  
   // radio.setRfSwitchPins(-1, 18);
    

}

bool checkCRC(const locationStruct& p) {
  // Recalculate CRC over struct EXCLUDING crc field
  uint8_t packetData[sizeof(locationStruct) - sizeof(uint32_t)];

  memcpy(packetData, &p, sizeof(packetData));

  uint32_t calculated =
      esp_crc32_le(0x00, packetData, sizeof(packetData));

  return (calculated == p.crc);
}

void processPacket() {
  receivedFlag = false;
  int len = radio.getPacketLength();

  Serial.printf("PacketSize: %d\n", len);

  if (len == sizeof(locationStruct)) {
    locationStruct loc;
    if (receiveStruct(radio, loc)) {
      // Serial.println("Valid locationStruct received!");
      String locJSON = locationStructToJson(loc, true);
      bleNotifyLoc(locJSON);
    } else {
      String locJSON = locationStructToJson(loc, false);
      Serial.println(locJSON);
      bleNotifyLoc(locJSON);
    }

  } else {
    radio.startReceive();
    return;
  }
  
  radio.startReceive();
}

void radioLoop() {
  static unsigned long lastTxTime = 0;
  static unsigned long txIntervalMs = 10000;
  unsigned long now = millis();

  if(receivedFlag) {
    processPacket();
  }

  // Only transmit if 30 seconds have passed
  if (now - lastTxTime < txIntervalMs) {
    return;
  }

  lastTxTime = now;
  txIntervalMs = random(12000, 18000); // randomize next interval between 12-18 seconds
  

  Serial.print(F("[SX1278] Transmitting packet ... "));


  // Calculate CRC and add to packetCnt
  p.packetCnt = p.packetCnt + 1;
  uint8_t packetData[sizeof(locationStruct)-4];
  memcpy(packetData, &p, sizeof(packetData));
  uint32_t crc = esp_crc32_le(0x00, packetData, sizeof(packetData));
  p.crc = crc;
  

  digitalWrite(LED, HIGH);
  
  byte *charpointer = (byte*)&p;
  unsigned long txnow = millis();
  int state = radio.transmit(charpointer, sizeof(p));
  Serial.print("timeOnAir " );
  Serial.println( millis() - txnow);
  digitalWrite(LED, LOW);
  delay(100);
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

  // wait for a second before transmitting again
//   delay(3000);
  
}


template<typename T, typename RadioType>
bool receiveStruct(RadioType& radio, T& out) {
  int len = radio.getPacketLength();

  if (len != sizeof(T)) {
    Serial.println("receiveStruct: len != sizeof(T)");
    radio.startReceive();
    return false;
  }

  uint8_t buffer[sizeof(T)];
  int state = radio.readData(buffer, sizeof(buffer));
  if (state != RADIOLIB_ERR_NONE) {
    radio.startReceive();
    return false;
  }

  T temp;
  memcpy(&temp, buffer, sizeof(T));

  uint8_t data[sizeof(T) - sizeof(uint32_t)];
  memcpy(data, &temp, sizeof(data));
  uint32_t crcCalc = esp_crc32_le(0x00, data, sizeof(data));

  out = temp;
  radio.startReceive();

  if (crcCalc != temp.crc) {
    Serial.println("CRC failed");
    return false;
  }
  return true;
}

String locationStructToJson(const locationStruct& p, bool isValid) {
  JsonDocument doc;  // allocate 256 bytes on the heap

  doc["id"]           = p.crc;
  doc["senderId"]     = p.senderId;
  doc["packetType"]   = p.packetType;
  doc["packetCnt"]    = p.packetCnt;
  doc["latitude"]     = p.lat / 10000000.0;
  doc["longitude"]    = p.lng / 10000000.0;
  doc["speed"]        = p.speed;
  doc["heading"]      = p.heading;
  doc["linkedDevice"] = senderId == p.senderId ? true : false ;
  doc["type"]         = "remoteLoc";
  doc["valid"]        = isValid;
  doc["version"]      = 2512251000; // YYYYMMDDHH
  
  
  String json;
  serializeJson(doc, json);
  Serial.println (json);
  return json;
}

