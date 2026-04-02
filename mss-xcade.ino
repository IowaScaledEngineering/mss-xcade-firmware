// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2016-2025 Hristo Gochkov, Mathieu Carbou, Emil Muratov

//
// Shows how to serve a static and dynamic template
//


#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <map>
#include <LittleFS.h>

#include "configuration.h"
#include "Wire.h"
#include "mss-xcade.h"
#include "SignalLogic.h"
#include "SignalLogicRegistry.h"
#include "webserver.h"
#include "utilities.h"

#include "SL-SingleCrossover.h"


#define LOOP_UPDATE_TIME_MS 500

// We've running on an xcade, so there's always guaranteed to be one
WireMux wireMux;

XCade xcade;

SignalLogic* activeLogic = NULL;


// Creating a new Signal Logic Module
// Step 1 - Create derived dlass from SignalLogic and fill in logic
// Step 2 - Create an html file to configure it in html/index-shortname.html
// Step 3 - using the shortname, create a file in defaults/config-shortname.json for the default configuration


// Register all active modules here
void registerSignalLogic(SignalLogicRegistry &slr)
{
  signalLogicRegistry.registerType(DiagnosticLogic::shortName, DiagnosticLogic::longName, []() { return std::make_unique<DiagnosticLogic>(); });
  signalLogicRegistry.registerType(SingleCrossover::shortName, SingleCrossover::longName, []() { return std::make_unique<SingleCrossover>(); });
  signalLogicRegistry.registerType(DoubleCrossover::shortName, DoubleCrossover::longName, []() { return std::make_unique<DoubleCrossover>(); });
}

void setup() 
{
  Serial.begin(115200);
  LittleFS.begin();

  sys_delay_ms(1000);

  Wire.setPins(XCADE_I2C_SDA, XCADE_I2C_SCL);
  Wire.setClock(100000);
  Wire.begin();

  wireMux.begin(&Wire, XCADE_I2C_MUX_RESET);
  xcade.begin(&wireMux);
  primeDebouncer(xcade);

  registerSignalLogic(signalLogicRegistry);

  bool doFactoryReset = xcade.configSwitches.getSwitch(7);
  // Load global configuration json file
  configLoadConfiguration(doFactoryReset);

  // Set up webserver
  //  Note - configuration needs to be loaded before this thing
  webserverSetup();

  bool enableWifi = xcade.configSwitches.getSwitch(1);
  if (enableWifi)
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAP((const char*)masterConfig["name"]);
  }

  webserverStart();

  if (NULL != activeLogic)
    activeLogic->setup();
}

void loop() 
{
  uint32_t currentTime = millis();
  static uint32_t lastReadTime = 0;  

  // If the webserver has indicated that we need a reload of signal logic parameters, do a reload and clear the flag
  if (signalConfNeedsRead)
  {
    // Now, reinitialize the signal logic with the new values
    readSignalConfig((const char*)masterConfig[MASTER_CONFIG_KEY_ACTIVE_CONFIG]);
    signalConfNeedsRead = false;
  }

	if (!(((uint32_t)currentTime - lastReadTime) > LOOP_UPDATE_TIME_MS))
    return;
  // Update the last time we ran through the loop to the current time
  lastReadTime = currentTime;

  if (NULL != activeLogic)
    activeLogic->loop();
}
