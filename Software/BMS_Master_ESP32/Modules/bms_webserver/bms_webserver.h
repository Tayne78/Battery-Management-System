#ifndef BMS_WEBSERVER_H
#define BMS_WEBSERVER_H

#define NUMBER_OF_SLAVES 2

#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <iostream>

extern AsyncWebServer server;

extern int temperature[NUMBER_OF_SLAVES];
extern float voltage[NUMBER_OF_SLAVES];

void notFound(AsyncWebServerRequest *request);
void handleRoot(AsyncWebServerRequest *request);
void handleStyle(AsyncWebServerRequest *request);
void handleScriptJS(AsyncWebServerRequest *request);
void handleBootstrap(AsyncWebServerRequest *request);
void handleLog(AsyncWebServerRequest *request);
void handleFetch(AsyncWebServerRequest *request);

void setupWebServer(const char* ssid, const char* password);


#endif