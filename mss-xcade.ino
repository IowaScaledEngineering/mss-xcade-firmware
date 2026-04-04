// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2016-2025 Hristo Gochkov, Mathieu Carbou, Emil Muratov

//
// Shows how to serve a static and dynamic template
//

#define ARDUINO_LOOP_STACK_SIZE (64 * 1024)

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include "esp_mac.h"
#include "esp_task_wdt.h"
#include "esp_chip_info.h"
#include "esp32s2/rom/rtc.h"

//SET_LOOP_TASK_STACK_SIZE(62 * 1024);

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


#define LOOP_UPDATE_TIME_MS 1000

// We've running on an xcade, so there's always guaranteed to be one
WireMux wireMux;
XCade xcade;
std::unique_ptr<SignalLogic> activeLogic = nullptr;
bool wifiEnabled = false;

// Creating a new Signal Logic Module
// Step 1 - Create derived dlass from SignalLogic and fill in logic
// Step 2 - Create an html file to configure it in html/index-shortname.html
// Step 3 - using the shortname, create a file in defaults/config-shortname.json for the default configuration


// Register all active modules here
void registerSignalLogic(SignalLogicRegistry &slr)
{
  Serial.printf("Registering signal logic...\n");
  signalLogicRegistry.registerType(DiagnosticLogic::shortName, DiagnosticLogic::longName, []() { return std::make_unique<DiagnosticLogic>(); });
  signalLogicRegistry.registerType(SingleCrossover::shortName, SingleCrossover::longName, []() { return std::make_unique<SingleCrossover>(); });
  signalLogicRegistry.registerType(DoubleCrossover::shortName, DoubleCrossover::longName, []() { return std::make_unique<DoubleCrossover>(); });
}

void setup() 
{
  Serial.begin(115200);
  LittleFS.begin();

  sys_delay_ms(2000);
  Serial.printf("Starting...\n");
  Wire.setPins(XCADE_I2C_SDA, XCADE_I2C_SCL);
  Wire.setClock(100000);
  Wire.begin();

  wireMux.begin(&Wire, XCADE_I2C_MUX_RESET);
  xcade.begin(&wireMux);
  primeDebouncer(xcade);

  registerSignalLogic(signalLogicRegistry);

  // Print a bunch of diagnostic header info to the serial console
  Serial.printf("[SYS]: Iowa Scaled Engineering\n");
  Serial.printf("[SYS]: Block Signal Custom (MSS-XCADE)\n");
  Serial.printf("[SYS]: Firmware %s\n", FIRMWARE_VERSION_STR);
  Serial.printf("[SYS]: IDF Ver:  [%s]\n", esp_get_idf_version());
  Serial.printf("[SYS]: ESP Arduino Ver: [%s]\n", ESP_ARDUINO_VERSION_STR);

  uint8_t macAddr[8];
  memset(macAddr, 0, sizeof(macAddr));
  esp_read_mac(macAddr, ESP_MAC_WIFI_STA);

  Serial.printf("[SYS]: MAC Addr: [%02X:%02X:%02X:%02X:%02X:%02X]\n", 
    macAddr[0], macAddr[1], macAddr[2],
    macAddr[3], macAddr[4], macAddr[5]);

  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  Serial.printf("[SYS]: ESP32S2 rev %d \n", chip_info.revision);
  Serial.printf("[SYS]: Reset Reason: [%s]\n", resetReasonStringGet(rtc_get_reset_reason(0)));
  Serial.printf("[SYS]: TICK %05lus stack: %u heap:%d\n", (uint32_t)(esp_timer_get_time() / 1000000), uxTaskGetStackHighWaterMark(NULL), xPortGetFreeHeapSize());

  bool doFactoryReset = xcade.configSwitches.getSwitch(7);
  // Load global configuration json file
  configLoadConfiguration(doFactoryReset);

  activeLogic = signalLogicRegistry.create(masterConfig["activeConfig"]);

  // Set up webserver
  //  Note - configuration needs to be loaded before this thing
  webserverSetup();

  wifiEnabled = xcade.configSwitches.getSwitch(SWITCH_1_ENABLE_WIFI);
  if (wifiEnabled)
  {
    enableWifi((const char*)masterConfig["name"]);
    webserverStart();
  }

  if (nullptr != activeLogic)
  {
    activeLogic->setup(&xcade);
    activeLogic->reconfigure(signalConfig);
  }
}

void loop() 
{
  uint32_t currentTime = millis();
  static uint32_t lastReadTime = 0;
  static bool ledState = false;

  // If the webserver has indicated that we need a reload of signal logic parameters, do a reload and clear the flag
  if (signalConfNeedsRead)
  {
    // Now, reinitialize the signal logic with the new values
    readSignalConfig((const char*)masterConfig[MASTER_CONFIG_KEY_ACTIVE_CONFIG]);
    signalConfNeedsRead = false;
    if (nullptr != activeLogic)
      activeLogic->reconfigure(signalConfig);
  }
  
  // Do the once a second tasks
	if (((uint32_t)currentTime - lastReadTime) > 1000)
  {
    // Update the last time we ran through the loop to the current time
    lastReadTime = currentTime;

    if (wifiEnabled != xcade.configSwitches.getSwitch(SWITCH_1_ENABLE_WIFI))
    {
      wifiEnabled = xcade.configSwitches.getSwitch(SWITCH_1_ENABLE_WIFI);

      if (wifiEnabled)
      {
        enableWifi((const char*)masterConfig["name"]);
        webserverStart();
      }
      else
      {
        disableWifi();
        webserverEnd();
      }
    }

    if (ledState)
    {
      rgbLedWrite(XCADE_RGB_LED, wifiEnabled?24:0, wifiEnabled?16:0, wifiEnabled?0:16);
    } else {
      rgbLedWrite(XCADE_RGB_LED, 0, 0, 0);
    }
    ledState = !ledState;

  }

  if (nullptr != activeLogic)
    activeLogic->loop();
}
