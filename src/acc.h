#pragma once

void accSetup();
void togglePinTest(char pin);
void accEnableInterrupts();
void accDisableInterrupts();
void WireSetBit(int addr, uint8_t bit, bool val);
void gnssSleep();
void gnsssWake();
