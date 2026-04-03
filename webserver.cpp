#include "configuration.h"
#include "Webserver.h"
#include "SignalLogicRegistry.h"
#include "SignalLogic.h"
#include <ArduinoJson.h>


static AsyncWebServer server(80);
extern volatile bool signalConfNeedsRead;
extern SignalLogic* activeLogic;



void webserverSetup()
{
  server.serveStatic(URL_MAIN_CSS, LittleFS, FILE_MAIN_CSS);
  server.serveStatic(URL_MAIN_JAVASCRIPT, LittleFS, FILE_MAIN_JAVASCRIPT);

  char indexFilename[STRLN_FILENAME_BUFFER];
  snprintf(indexFilename, sizeof(indexFilename), "/html/index-%s.html", (const char*)masterConfig[MASTER_CONFIG_KEY_ACTIVE_CONFIG]);
  Serial.printf("Trying to set index to [%s]\n", indexFilename);

  // If a signal logic configuration-specific index doesn't exist, just load the default
  if (!LittleFS.exists(indexFilename))
    snprintf(indexFilename, sizeof(indexFilename), "/html/index.html");



  server.serveStatic(URL_MAIN, LittleFS, indexFilename).setTemplateProcessor([](const String &var) -> String {
    String html;
    if (var == "BASICCONFIG")
    {
      html += R"(<h3>Basic Configuration</h3>
      <div class="status-card">
          <div class="card-info-col">
              <div class="card-title">Module Identification</div>
              <p class="card-desc">Iowa Scaled Engineering<br>Block Signal Custom Module</p>
          </div>
          
          <div style="text-align: right; min-width: 100px;">
              <div style="font-size: 11px; font-weight: 700; color: #999; text-transform: uppercase; margin-bottom: 4px;">Firmware</div>
              <div style="font-size: 16px; font-weight: 600; color: #333;">)";
      html += FIRMWARE_VERSION_STR;
      html += R"(</div></div>
      </div>

      <!-- Configuration Card - Module Name -->
      <div class='control-card warning-text'>
          <strong>Warning: Changing basic configuration will result the node restarting</strong>
      </div>

      <div class='control-card' id='card-module_name'>
          <div class='card-info-col'>
              <label for='input-module-name' class='card-title'>Module Name/SSID</label>
              <p class='card-desc'>Device identifier for wireless configuration network</p>
          </div>
          <div class='control-actions'>
              <input type='text' id='input-module-name' maxlength='32' value=')";
              
      html += (const char *)masterConfig[MASTER_CONFIG_KEY_NODE_NAME];
      html += R"(' maxlength='32' class='custom-input' name='name' oninput=\"this.value = this.value.replace(/[^a-zA-Z0-9 _-]/g, '')\">
                </div>
      </div>

      <!-- Configuration Card - Logic Configuration -->
      <div class='control-card'>
          <div class='card-info-col'>
              <label for='input-signal-logic' class='card-title'>Predefined Configuration</label>
              <p class='card-desc'>Select a preset signal logic template.</p>
          </div>

          <div class='vertical-actions'>
              <div class='select-wrapper'>
                  <select id='input-signal-logic' class='custom-select' name='activeConfig'>)";

      for (uint32_t i = 0; i<signalLogicRegistry.getNumLogicModules(); i++)
      {
        bool isCurrentlyRunning = (0 == strcmp(signalLogicRegistry.getShortName(i), (const char*)masterConfig[MASTER_CONFIG_KEY_ACTIVE_CONFIG]));

        html += " <option value='";
        html += signalLogicRegistry.getShortName(i);
        if (isCurrentlyRunning)
          html += "' selected class='current-option'>";
        else
          html += "'>";
        html += signalLogicRegistry.getLongName(i);
        if (isCurrentlyRunning)
          html += " (Current)";
        html += "</option>\n";
      }
      html += R"(</select>
              </div>
          </div>
      </div>
      <button class='submit-btn' onclick='submitBasicConfig()'>Change Configuration</button>)";
    }
    return html;
  });

  // Add Handler for writing the basic node configuration - SSID & the active signal configuration
  // /api/saveBasicConfig

  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler(URL_API_SAVEBASICCONFIG, [](AsyncWebServerRequest *request, JsonVariant &json) 
  {
    // This API endpoint saves the overall module global configuration (ssid and what signal logic is running)
    JsonObject jsonObj = json.as<JsonObject>();
    Serial.println("HTTP: Got save config");

    bool fail = false;

    // Sanity checks
    if (!signalLogicRegistry.shortNameExists(jsonObj["activeConfig"]))
      fail = true;

    serializeJson(jsonObj, Serial);

    if (!fail)
    {
      File masterConfigFile = LittleFS.open(MASTER_CONFIGURATION_FILENAME, "w");
      serializeJson(jsonObj, masterConfigFile);
      masterConfigFile.close();
      request->send(200, "application/json", "{\"status\":\"ok\"}");
      sys_delay_ms(100);
      ESP.restart();
    }
    else
      request->send(400, "application/json", "{\"status\":\"error\"}");

  });
  server.addHandler(handler);

  // Add Handler for writing the signal logic configuration
  // /api/saveConfig

  AsyncCallbackJsonWebHandler* handler2 = new AsyncCallbackJsonWebHandler(URL_API_SAVELOGICCONFIG, [](AsyncWebServerRequest *request, JsonVariant &json) 
  {
    // This API endpoint saves a specific signal logic module's configuration
    JsonObject jsonObj = json.as<JsonObject>();
    bool fail = signalConfNeedsRead;

   // Sanity checks
    if (!signalLogicRegistry.shortNameExists(jsonObj["configName"]))
      fail = true;

    if (0 != strcmp(masterConfig["activeConfig"], jsonObj["configName"]))
      fail = true;

    if (!fail)
    {
      char fileName[STRLN_FILENAME_BUFFER];
      snprintf(fileName, sizeof(fileName), "/config-%s.json", (const char*)masterConfig["activeConfig"]);
      File configFile = LittleFS.open(fileName, "w");
      serializeJson(jsonObj, configFile);
      configFile.close();
      signalConfNeedsRead = true;
      request->send(200, "application/json", "{\"status\":\"ok\"}");
      Serial.printf("HTTP: API saveConfig success");
    } else {
      Serial.printf("HTTP: API saveConfig fail configName=[%s] activeConfig=[%s]", (const char*)jsonObj["configName"], (const char*)masterConfig["activeConfig"]);
      request->send(400, "application/json", "{\"status\":\"error\"}");
    }
  });
  server.addHandler(handler2);

  // Add Handler for reading the signal logic configuration
  // /api/loadConfig
  AsyncCallbackJsonWebHandler* handler3 = new AsyncCallbackJsonWebHandler(URL_API_LOADLOGICCONFIG, [](AsyncWebServerRequest *request, JsonVariant &json) 
  {
    JsonObject jsonObj = json.as<JsonObject>();
    Serial.println("HTTP: API loadConfig");

    char jsonConfigFilename[STRLN_FILENAME_BUFFER];
    snprintf(jsonConfigFilename, sizeof(jsonConfigFilename), "/config-%s.json", (const char*)masterConfig["activeConfig"]);
    if (LittleFS.exists(jsonConfigFilename))
      request->send(LittleFS, jsonConfigFilename, "application/json");
    else
      request->send(200, "application/json", "{\"status\":\"error\"}");
  });
  server.addHandler(handler3);


  // Add handler for getting current status from the signal logic
  // /api/getStatus

  AsyncCallbackJsonWebHandler* handler4 = new AsyncCallbackJsonWebHandler(URL_API_LOADSTATUS, [](AsyncWebServerRequest *request, JsonVariant &json) 
  {
    JsonObject jsonObj = json.as<JsonObject>();
    Serial.println("HTTP: API getStatus");

/*    if (NULL != activeLogic)
      activeLogic->getStatusJson();*/

    request->send(200, "application/json", "{\"status\":\"ok\"}");

  });
  server.addHandler(handler4);

  // Define the onNotFound handler
  server.onNotFound([](AsyncWebServerRequest *request) {
      // Log the unhandled request for debugging
      Serial.print("server.onNotFound triggered for URL: ");
      Serial.println(request->url());

      // Redirect the client to the root URL
      request->redirect("/index.html");
  });
}

void webserverStart()
{
  server.begin();
}
