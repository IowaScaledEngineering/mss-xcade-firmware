#ifndef UTILITIES_H
#define UTILITIES_H

#include "mss-xcade.h"
#include "ArduinoJson.h"

void primeDebouncer(XCade &xcade);
const char* resetReasonStringGet(int resetReason);
void enableWifi(const char* networkName);
void disableWifi();
void mssPortToStatusJson(JsonObject& statusResponse, MSSPort &port, const char* portName);
void mssGPIOToJson(JsonObject& statusResponse, GPIO &gpio, const char* gpioName, uint8_t maxBits);
void mssSensorsToJson(JsonObject& statusResponse, GPIO &gpio, const char* sensorName, uint8_t maxBits);
#endif