#ifndef UTILITIES_H
#define UTILITIES_H

#include "mss-xcade.h"

void primeDebouncer(XCade &xcade);
const char* resetReasonStringGet(int resetReason);
void enableWifi(const char* networkName);
void disableWifi();
#endif