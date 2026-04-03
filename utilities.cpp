#include <Arduino.h>
#include "utilities.h"
#include <WiFi.h>
#include "configuration.h"

void primeDebouncer(XCade &xcade)
{
  // Prime the debouncer
  for(uint8_t i=0; i<5; i++)
  {
    xcade.updateInputs();
    delay(20);
  }
}

void enableWifi(const char* networkName)
{
  Serial.printf("[SYS]: Starting WiFi - [%s]\n", networkName);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IP_ADDRESS_AP, IP_ADDRESS_AP, IP_SUBNET_MASK_AP);
  WiFi.softAP(networkName);
}

void disableWifi()
{
  Serial.printf("[SYS]: Terminating WiFi\n");
  WiFi.mode(WIFI_OFF);
}

const char* resetReasonStringGet(int resetReason)
{
  switch (resetReason)
  {
    case 1  : return "Vbat power on reset";
    case 3  : return "Software reset digital core";
    case 4  : return "Legacy watch dog reset digital core";
    case 5  : return "Deep Sleep reset digital core";
    case 6  : return "Reset by SLC module, reset digital core";
    case 7  : return "Timer Group0 Watch dog reset digital core";
    case 8  : return "Timer Group1 Watch dog reset digital core";
    case 9  : return "RTC Watch dog Reset digital core";
    case 10 : return "Instrusion tested to reset CPU";
    case 11 : return "Time Group reset CPU";
    case 12 : return "Software reset CPU";
    case 13 : return "RTC Watch dog Reset CPU";
    case 14 : return "for APP CPU, reseted by PRO CPU";
    case 15 : return "Reset when the vdd voltage is not stable";
    case 16 : return "RTC Watch dog reset digital core and rtc module";
    default : return "No Idea";
  }
}