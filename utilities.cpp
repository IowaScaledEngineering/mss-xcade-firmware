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

void mssGPIOToJson(JsonObject& statusResponse, GPIO &gpio, const char* gpioName, uint8_t maxBits)
{
  char buffer[32];

  if (maxBits > 6)
    maxBits = 6;

  for (uint8_t i=1; i<maxBits+1; i++)
  {
    snprintf(buffer, sizeof(buffer), "%s-%d", gpioName, i);
    statusResponse[buffer] = gpio.digitalRead(i);
  }
}

void mssSensorsToJson(JsonObject& statusResponse, GPIO &gpio, const char* sensorName, uint8_t maxBits)
{
  char buffer[32];

  if (maxBits > 10)
    maxBits = 10;

  for (uint8_t i=1; i<maxBits+1; i++)
  {
    snprintf(buffer, sizeof(buffer), "%s-%d", sensorName, i);
    statusResponse[buffer] = gpio.digitalRead(i+GPIO_SENSOR_BASE-1);
  }
}

void mssPortToStatusJson(JsonObject& statusResponse, MSSPort &port, const char* portName)
{
  bool S, A, AA, DA;
  char buffer[32];
  std::string statusText = "RX: ";

  port.getRawInputs(&S, &A, &AA, &DA);

  snprintf(buffer, sizeof(buffer), "%s-s-in", portName);
  statusResponse[buffer] = S;
  snprintf(buffer, sizeof(buffer), "%s-a-in", portName);
  statusResponse[buffer] = A;
  snprintf(buffer, sizeof(buffer), "%s-aa-in", portName);
  statusResponse[buffer] = AA;
  snprintf(buffer, sizeof(buffer), "%s-da-in", portName);
  statusResponse[buffer] = DA;

  switch(port.indicationReceivedGet())
  {
    case INDICATION_STOP:
      statusText += "Stop";
      break;
    case INDICATION_APPROACH_DIVERGING:
      statusText += "Approach Diverging";
      break;
    case INDICATION_APPROACH_DIVERGING_AA:
      statusText += "Adv Approach Diverging";
      break;
    case INDICATION_ADVANCE_APPROACH:
      statusText += "Advance Approach";
      break;
    case INDICATION_APPROACH:
      statusText += "Approach";
      break;
    case INDICATION_CLEAR:
      statusText += "Clear";
      break;
    default:
      statusText += "Unknown";
      break;
  }


  port.getRawOutputs(&S, &A, &AA, &DA);
  snprintf(buffer, sizeof(buffer), "%s-s-out", portName);
  statusResponse[buffer] = S;
  snprintf(buffer, sizeof(buffer), "%s-a-out", portName);
  statusResponse[buffer] = A;
  snprintf(buffer, sizeof(buffer), "%s-aa-out", portName);
  statusResponse[buffer] = AA;
  snprintf(buffer, sizeof(buffer), "%s-da-out", portName);
  statusResponse[buffer] = DA;

  statusText +="<br/>TX: ";
  if (S)  
    statusText += "Stop";
  else if (A && !DA)
    statusText += "Approach";
  else if (A && DA && AA)
    statusText += "Adv Approach Diverging";
  else if (A && DA && !AA)
    statusText += "Approach Diverging";
  else if (AA)
    statusText += "Advance Approach";
  else
    statusText += "Clear";

  snprintf(buffer, sizeof(buffer), "%s-statustxt", portName);
  statusResponse[buffer] = statusText;
}
