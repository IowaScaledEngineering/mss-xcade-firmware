#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

void webserverSetup();
void webserverStart();
void webserverEnd();


#endif
