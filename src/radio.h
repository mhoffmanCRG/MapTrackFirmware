#pragma once

#include "packet.h"

void radioSetup();
void radioLoop();


bool checkCRC(locationStruct p);

String locationStructToJson(const locationStruct& p, bool isValid, float rssi, float snr);

