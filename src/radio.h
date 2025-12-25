#pragma once

#include "packet.h"

void radioSetup();
void radioLoop();

template<typename T, typename RadioType>
bool receiveStruct(RadioType&, T&);

String locationStructToJson(const locationStruct& p, bool isValid);

