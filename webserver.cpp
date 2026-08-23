#include "configuration.h"
#include "Webserver.h"
#include "SignalLogicRegistry.h"
#include "SignalLogic.h"
#include <ArduinoJson.h>


static AsyncWebServer server(80);
extern volatile bool signalConfNeedsRead;
extern std::unique_ptr<SignalLogic> activeLogic;
extern volatile bool signalLogicNeedsReload;

String listFiles(fs::FS &fs, const char * dirname, uint8_t levels) 
{
  String html = "";
  File root = fs.open(dirname);
  if (!root || !root.isDirectory()) 
  {
    return "Failed to open directory";
  }

  File file = root.openNextFile();
  while (file) 
  {
    if (file.isDirectory()) 
    {
      if (levels) 
      {
          html += listFiles(fs, file.path(), levels - 1);
      }
    } else {
      html += "<li><a href=\"/downloadFile?file=" + String(file.path()) + "\">" 
            + String(file.path()) + "</a> (" + String(file.size()) + " bytes)</li>";
    }
    file = root.openNextFile();
}
  return html;
}

static String buildBasicConfigHtml()
{
  String html = "<h3>Basic Configuration</h3>";
  html += "<div class='status-card'>";
  html += "<div class='card-info-col'>";
  html += "<div class='card-title'>Module Identification</div>";
  html += "<p class='card-desc'>Iowa Scaled Engineering<br>Block Signal Custom Module</p>";
  html += "</div>";
  html += "<div style='text-align: right; min-width: 100px;'>";
  html += "<div style='font-size: 11px; font-weight: 700; color: #999; text-transform: uppercase; margin-bottom: 4px;'>Firmware</div>";
  html += "<div style='font-size: 16px; font-weight: 600; color: #333;'>";
  html += FIRMWARE_VERSION_STR;
  html += "</div></div></div>";

  html += "<div class='control-card' id='card-module_name'>";
  html += "<div class='card-info-col'>";
  html += "<label for='input-module-name' class='card-title'>Module Name/SSID</label>";
  html += "<p class='card-desc'>Device identifier for wireless configuration network</p>";
  html += "</div>";
  html += "<div class='control-actions'>";
  html += "<input type='text' id='input-module-name' maxlength='32' value='";
  html += (const char*)masterConfig[MASTER_CONFIG_KEY_NODE_NAME];
  html += "' class='custom-input' name='name' oninput=\"this.value = this.value.replace(/[^a-zA-Z0-9 _-]/g, '')\">";
  html += "</div></div>";

  html += "<div class='control-card'>";
  html += "<div class='card-info-col'>";
  html += "<label for='input-signal-logic' class='card-title'>Predefined Configuration</label>";
  html += "<p class='card-desc'>Select a preset signal logic template.</p>";
  html += "</div>";
  html += "<div class='vertical-actions'><div class='select-wrapper'><select id='input-signal-logic' class='custom-select' name='activeConfig'>";

  const size_t logicCount = signalLogicRegistry.getNumLogicModules();
  for (size_t i = 0; i < logicCount; ++i)
  {
    const char* shortName = signalLogicRegistry.getShortName(i);
    const char* longName = signalLogicRegistry.getLongName(i);
    const bool isCurrentlyRunning = (0 == strcmp(shortName, (const char*)masterConfig[MASTER_CONFIG_KEY_ACTIVE_CONFIG]));

    html += "<option value='";
    html += shortName;
    if (isCurrentlyRunning)
      html += "' selected class='current-option'>";
    else
      html += "'>";
    html += longName;
    if (isCurrentlyRunning)
      html += " (Current)";
    html += "</option>";
  }

  html += "</select></div></div></div>";
  html += "<button class='submit-btn' onclick='submitBasicConfig()'>Change Configuration</button>";
  html += "<div class='control-card' style='justify-content: center; align-items: center; padding: 25px;'>";
  html += "<img src='ise.png' alt='ISE Electronics Made Easy' style='max-width: 100%; height: auto;'>";
  html += "</div>";

  return html;
}



void webserverSetup()
{
  server.serveStatic(URL_MAIN_CSS, LittleFS, FILE_MAIN_CSS);
  server.serveStatic(URL_MAIN_JAVASCRIPT, LittleFS, FILE_MAIN_JAVASCRIPT);
  server.serveStatic(URL_MAIN_ISE_LOGO, LittleFS, FILE_MAIN_ISE_LOGO);



  // Add generic upload and download functionality

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
  {
      String html = "<html><body><h1>Files on Block Signal Pro</h1><ul>";
      html += listFiles(LittleFS, "/", 3); // Lists up to 3 folders deep
      html += "</ul><br><a href='/upload'>Go to Upload Page</a></body></html>";
      request->send(200, "text/html", html);
  });

  server.on("/downloadFile", HTTP_GET, [](AsyncWebServerRequest *request)
  {
      if (request->hasParam("file")) 
      {
          String path = request->getParam("file")->value();
          if (LittleFS.exists(path)) 
          {
              // The 'true' at the end forces the browser to download the file 
              // instead of trying to display it inline.
              request->send(LittleFS, path, String(), true);
          } else {
              request->send(404, "text/plain", "File not found");
          }
      } else {
          request->send(400, "text/plain", "File parameter missing");
      }
  });

  server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request){
      String html = R"rawliteral(
      <html><body>
      <h1>Upload File to Block Signal Pro</h1>
      <form id="uploadForm" enctype="multipart/form-data" method="post">
          <label>Target Path & Filename (e.g. /myfolder/data.csv):</label><br>
          <input type="text" id="filePath" value="/newfile.txt" size="40"><br><br>
          <input type="file" id="fileInput" name="file"><br><br>
          <input type="button" value="Upload" onclick="submitForm()">
      </form>
      <script>
      function submitForm() {
          var form = document.getElementById('uploadForm');
          var path = document.getElementById('filePath').value;
          if(!path.startsWith('/')) path = '/' + path;
          // Append the path as a query parameter so the server sees it BEFORE the file stream
          form.action = '/uploadFile?path=' + encodeURIComponent(path);
          form.submit();
      }
      </script>
      <br><a href="/">Back to Files</a>
      </body></html>
      )rawliteral";
      
      request->send(200, "text/html", html);
  });


  server.on("/uploadFile", HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Upload Complete! Go back to / to see the file.");
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      
      // Grab custom path from the URL parameter set by our JavaScript
      String savePath = filename;
      if (request->hasParam("path")) {
          savePath = request->getParam("path")->value();
      } else if (!savePath.startsWith("/")) {
          savePath = "/" + savePath;
      }

      // On first chunk, open the file
      if (!index) {
          Serial.printf("Upload Start: %s\n", savePath.c_str());
          File* f = new File();
          *f = LittleFS.open(savePath, FILE_WRITE);
          // Store the file pointer to the request object so we can access it in subsequent chunks
          request->_tempObject = (void*)f; 
      }

      // On subsequent chunks, write the data
      File* f = (File*)request->_tempObject;
      if (f && *f) {
          if (len) {
              f->write(data, len);
          }
          // On final chunk, close and clean up
          if (final) {
              f->close();
              delete f;
              request->_tempObject = NULL;
              Serial.printf("Upload End: %s, %u B\n", savePath.c_str(), index + len);
          }
      }
  });


  char indexFilename[STRLN_FILENAME_BUFFER];
  snprintf(indexFilename, sizeof(indexFilename), "/html/index-%s.html", (const char*)masterConfig[MASTER_CONFIG_KEY_ACTIVE_CONFIG]);
  Serial.printf("Trying to set index to [%s]\n", indexFilename);

  // If a signal logic configuration-specific index doesn't exist, just load the default
  if (!LittleFS.exists(indexFilename))
    snprintf(indexFilename, sizeof(indexFilename), "/html/index.html");

  server.on(URL_MAIN, HTTP_GET, [](AsyncWebServerRequest *request) {
    char indexFilename[STRLN_FILENAME_BUFFER];
    snprintf(indexFilename, sizeof(indexFilename), "/html/index-%s.html", (const char*)masterConfig[MASTER_CONFIG_KEY_ACTIVE_CONFIG]);

    if (!LittleFS.exists(indexFilename))
      snprintf(indexFilename, sizeof(indexFilename), "/html/index.html");

    File file = LittleFS.open(indexFilename, "r");
    if (!file)
    {
      request->send(404, "text/plain", "Missing index template");
      return;
    }

    String html = file.readString();
    file.close();
    html.replace("%BASICCONFIG%", buildBasicConfigHtml());
    request->send(200, "text/html", html);
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
      masterConfig[MASTER_CONFIG_KEY_NODE_NAME] = jsonObj[MASTER_CONFIG_KEY_NODE_NAME];
      masterConfig[MASTER_CONFIG_KEY_ACTIVE_CONFIG] = jsonObj[MASTER_CONFIG_KEY_ACTIVE_CONFIG];
      signalLogicNeedsReload = true;
      request->send(200, "application/json", "{\"status\":\"ok\"}");
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
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject jsonObj = json.as<JsonObject>();
    JsonObject responseObj = response->getRoot();


    if (NULL != activeLogic)
    {
      Serial.printf("HTTP: API getStatus [%s]\n", activeLogic->getShortName());
      activeLogic->getStatusJson(responseObj);
    } else {
      Serial.printf("HTTP: API getStatus no active logic\n");
      responseObj["status"] = "error";
    }

    response->setLength();
    request->send(response);

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

void webserverEnd()
{
  server.end();
}
