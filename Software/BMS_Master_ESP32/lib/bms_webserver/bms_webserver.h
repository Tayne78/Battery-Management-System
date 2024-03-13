#ifndef BMS_WEBSERVER_H
#define BMS_WEBSERVER_H

#define BALANCING 1
#define IDLE 0

#define NiCd 1
#define NiMH 2
#define Li_Ion 3
#define LiPo 4

#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <iostream>

extern AsyncWebServer server;

/*extern int temperature[NUMBER_OF_SLAVES];
extern float voltage[NUMBER_OF_SLAVES];
extern bool status[NUMBER_OF_SLAVES];*/

extern int* temperature;
extern int* voltage;
extern bool* status;

//extern std::vector<std::vector<int>> sorted_voltages;
extern int* sorted_temperature;
extern int* received_data;

extern int totale_voltage;
extern int avg_temperature;

extern int maxCellVoltage;
extern int minCellVoltage;
extern int differenceMaxMin;

extern int maxCell;
extern int minCell;

extern int numOfSlaves;

extern unsigned char akkutyp;

void notFound(AsyncWebServerRequest *request);
void handleRoot(AsyncWebServerRequest *request);
void handleStyle(AsyncWebServerRequest *request);
void handleScriptJS(AsyncWebServerRequest *request);
void handleBootstrap(AsyncWebServerRequest *request);
void handleLog(AsyncWebServerRequest *request);
void handleFetch(AsyncWebServerRequest *request);

void setupWebServer(const char* ssid, const char* password);


#endif