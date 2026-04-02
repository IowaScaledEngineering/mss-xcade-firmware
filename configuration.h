#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <ArduinoJson.h>

extern JsonDocument masterConfig;
extern JsonDocument signalConfig;
extern volatile bool signalConfNeedsRead;

#define MASTER_CONFIGURATION_FILENAME   "/config.json"
#define MASTER_CONFIG_KEY_NODE_NAME     "name"
#define MASTER_CONFIG_KEY_ACTIVE_CONFIG "activeConfig"

#define FILE_MAIN_CSS                   "/html/config.css"
#define FILE_MAIN_JAVASCRIPT            "/html/core.js"


#define URL_MAIN                        "/index.html"
#define URL_MAIN_CSS                    "/config.css"
#define URL_MAIN_JAVASCRIPT             "/core.js"

#define URL_API_SAVEBASICCONFIG         "/api/saveBasicConfig"
#define URL_API_SAVELOGICCONFIG         "/api/saveConfig"
#define URL_API_LOADLOGICCONFIG         "/api/loadConfig"
#define URL_API_LOADSTATUS              "/api/getStatus"

#define STRLN_FILENAME_BUFFER 64

void readSignalConfig(const char* configName);
void configFactoryReset();
bool configLoadConfiguration(bool forceFactoryReset);
void displayFileTree(const char* startingDirectory="/", uint32_t levels=0);



#endif