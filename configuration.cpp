#include <LittleFS.h>
#include <WiFi.h>
#include "configuration.h"
#include "esp_mac.h"

JsonDocument masterConfig;
JsonDocument signalConfig;
volatile bool signalConfNeedsRead = false;

bool getJsonBool(JsonDocument& doc, const char* key, bool defaultVal)
{
  return (signalConfig[key].is<bool>() && signalConfig[key].as<bool>());
}

void readSignalConfig(const char* configName)
{
  char signalConfigFilename[STRLN_FILENAME_BUFFER];
  snprintf(signalConfigFilename, sizeof(signalConfigFilename), "/config-%s.json", configName);
  File signalConfigFile = LittleFS.open(signalConfigFilename);
  deserializeJson(signalConfig, signalConfigFile);
  signalConfigFile.close();
}

void configFactoryReset()
{
  File defaultsDir = LittleFS.open("/defaults");
  if(!defaultsDir || !defaultsDir.isDirectory())
  {
      // Seriously, I don't even know what to do here
      Serial.printf("Failed to open defaults directory\n");
      defaultsDir.close();
      return;
  }

  File defaultsFile = defaultsDir.openNextFile();
  while (defaultsFile)
  {
    if (!defaultsFile.isDirectory())
    {
      char destPath[128];
      Serial.printf("  Restoring defaults [%s]...", defaultsFile.name());
      snprintf(destPath, sizeof(destPath), "/%s", defaultsFile.name());

      // Copy files over from defaults directory
      // Clear the old file      
      if (LittleFS.exists(destPath))
        LittleFS.remove(destPath);

      File destFile = LittleFS.open(destPath, "w");
      if (destFile)
      {
        uint8_t copyBuffer[512];
        size_t bytesRead = 0;
        while ((bytesRead = defaultsFile.read(copyBuffer, sizeof(copyBuffer))) > 0)
        {
            destFile.write(copyBuffer, bytesRead);
        }      
        destFile.close();
        Serial.printf("Done\n");
      } else {
        Serial.printf("Failed\n");
      }
    }
    defaultsFile.close();
    defaultsFile = defaultsDir.openNextFile();
  }

  /* Build a brand new msater configuration file */
  /* There's very little in here except the name of the node and the active configuration */
  JsonDocument newMasterConfig;

  uint8_t macAddr[8];
  char ssidStr[32];
  memset(macAddr, 0, sizeof(macAddr));
  esp_read_mac(macAddr, ESP_MAC_WIFI_STA);
  memset(ssidStr, 0, sizeof(ssidStr));
  snprintf(ssidStr, sizeof(ssidStr), "ISE-BSC-%02X%02X%02X", macAddr[3], macAddr[4], macAddr[5]);

  newMasterConfig["name"] = ssidStr;
  newMasterConfig["activeConfig"] = "none";

  File masterConfigFile = LittleFS.open("/config.json", "w");
  serializeJson(newMasterConfig, masterConfigFile);
  masterConfigFile.close();
}


bool configLoadConfiguration(bool forceFactoryReset)
{
  File masterConfigFile; 

  if (forceFactoryReset || !(masterConfigFile = LittleFS.open("/config.json")))
  {
    // Handle the case where the config.json file doesn't open
    // Copy over defaults
    configFactoryReset();
    masterConfigFile = LittleFS.open("/config.json");
  }

  deserializeJson(masterConfig, masterConfigFile);
  masterConfigFile.close();

  readSignalConfig(masterConfig["activeConfig"]);
  return true;
}

void displayFileTree(const char* startingDirectory, uint32_t levels) 
{
  Serial.printf("Listing directory: %s\n", startingDirectory);

  File root = LittleFS.open(startingDirectory);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("- not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    // Create indentation based on recursion depth
    for (uint8_t i = 0; i < levels; i++) {
      Serial.print("  ");
    }

    if (file.isDirectory()) {
      Serial.printf("Dir: %s\n", file.name());
      
      // Construct the new path for the recursive call
      // Note: Some frameworks provide file.path(), others require manual concatenation
      String nextDir = String(startingDirectory) + "/" + file.name();
      displayFileTree(nextDir.c_str(), levels + 1);
    } else {
      Serial.printf("File: %s  Size: %u bytes\n", file.name(), file.size());
    }
    
    file = root.openNextFile();
  }
}
