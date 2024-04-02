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

extern std::vector<int> temperature;
extern std::vector<int> voltage;
extern std::vector<bool> status;

using SlaveVoltagePair = std::tuple<unsigned int, unsigned int>;
extern std::vector<SlaveVoltagePair> sorted_voltages;

//extern int** sorted_voltages;
extern std::vector<int> sorted_temperature;
extern std::vector<int> received_data;

extern int totale_voltage;
extern int avg_temperature;

extern int maxCellVoltage;
extern int minCellVoltage;
extern int differenceMaxMin;

extern int maxCellId;
extern int minCellId;

extern int numOfSlaves;

struct BatteryParameters {
    int endVoltage;
    int minVoltage;
    int minBalanceVoltage;
    int maxTemperature;
};

BatteryParameters getBatteryParameters();

extern BatteryParameters battery;

void notFound(AsyncWebServerRequest *request);
void handleRoot(AsyncWebServerRequest *request);
void handleStyle(AsyncWebServerRequest *request);
void handleScriptJS(AsyncWebServerRequest *request);
void handleBootstrap(AsyncWebServerRequest *request);
void handleLog(AsyncWebServerRequest *request);
void handleFetch(AsyncWebServerRequest *request);

void setupWebServer(const char* ssid, const char* password);

void sortBy(const bool voltage);
void changeVoltage(unsigned int slaveId, unsigned int newVoltage);

#endif