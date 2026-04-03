#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <ArduinoJson.h>

extern JsonDocument masterConfig;
extern JsonDocument signalConfig;
extern volatile bool signalConfNeedsRead;

#define SWITCH_7_FACTORY_RESET          7
#define SWITCH_1_ENABLE_WIFI            1

#define FIRMWARE_VERSION_STR            "0.0.1"

#define MASTER_CONFIGURATION_FILENAME   "/config.json"
#define MASTER_CONFIG_KEY_NODE_NAME     "name"
#define MASTER_CONFIG_KEY_ACTIVE_CONFIG "activeConfig"

#define FILE_MAIN_CSS                   "/html/config.css"
#define FILE_MAIN_JAVASCRIPT            "/html/core.js"
#define FILE_MAIN_ISE_LOGO              "/html/ise.png"


#define URL_MAIN                        "/index.html"
#define URL_MAIN_CSS                    "/config.css"
#define URL_MAIN_JAVASCRIPT             "/core.js"
#define URL_MAIN_ISE_LOGO               "/ise.png"

#define URL_API_SAVEBASICCONFIG         "/api/saveBasicConfig"
#define URL_API_SAVELOGICCONFIG         "/api/saveConfig"
#define URL_API_LOADLOGICCONFIG         "/api/loadConfig"
#define URL_API_LOADSTATUS              "/api/getStatus"

#define IP_ADDRESS_AP                   IPAddress(192, 168, 1, 1)
#define IP_SUBNET_MASK_AP               IPAddress(255, 255, 255, 0)

#define STRLN_FILENAME_BUFFER 64

void readSignalConfig(const char* configName);
void configFactoryReset();
bool configLoadConfiguration(bool forceFactoryReset);
void displayFileTree(const char* startingDirectory="/", uint32_t levels=0);
bool getJsonBool(JsonDocument& doc, const char* key, bool defaultVal=false);


#endif