#include "bms_webserver.h"

AsyncWebServer server(80);
/*int temperature[NUMBER_OF_SLAVES] = {0};
float voltage[NUMBER_OF_SLAVES] = {0};
bool status[NUMBER_OF_SLAVES] = {0};*/

int numOfSlaves = 2;

int *temperature = (int *)malloc(numOfSlaves * sizeof(int));
float *voltage = (float *)malloc(numOfSlaves * sizeof(float));
bool *status = (bool *)malloc(numOfSlaves * sizeof(bool));

float maxCellVoltage = 0;
float minCellVoltage = 0;
float differenceMaxMin = 0;

int maxCell = 0;
int minCell = 0;

unsigned char akkutyp = 1;

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not Found");
}

void handleRoot(AsyncWebServerRequest *request)
{
  File file = SPIFFS.open("/index.html");
  if (file)
  {
    request->send(SPIFFS, "/index.html", "text/html");
    file.close();
  }
  else
  {
    notFound(request);
  }
}
void handleFavicon(AsyncWebServerRequest *request)
{
  File file = SPIFFS.open("/favicon.ico");
  if (file)
  {
    request->send(SPIFFS, "/favicon.ico", "image/x-icon");
    file.close();
  }
  else
  {
    notFound(request);
  }
}
void handleAboutUs(AsyncWebServerRequest *request)
{
  File file = SPIFFS.open("/about-us.html");
  if (file)
  {
    request->send(SPIFFS, "/about-us.html", "text/html");
    file.close();
  }
  else
  {
    notFound(request);
  }
}

void handleBootstrap(AsyncWebServerRequest *request)
{
  File file = SPIFFS.open("/css/bootstrap.min.css");
  if (file)
  {
    request->send(SPIFFS, "/css/bootstrap.min.css", "text/css");
    file.close();
  }
  else
  {
    notFound(request);
  }
}

void handleScriptJS(AsyncWebServerRequest *request)
{
  String filePath = request->url();
  File file = SPIFFS.open(filePath);
  if (file)
  {
    request->send(SPIFFS, filePath, "application/javascript");
    file.close();
  }
  else
  {
    notFound(request);
  }
}

void handleLog(AsyncWebServerRequest *request)
{
  File file = SD.open("/log.txt"); // Replace with your file path
  if (file)
  {
    request->send(SD, "/log.txt", "text/plain");
    file.close();
  }
  else
  {
    Serial.println("ERROR loading log file!!!");
    request->send(404, "text/plain", "File not found");
  }
}

void handleFetch(AsyncWebServerRequest *request)
{
  DynamicJsonDocument jsonDoc(256);

  jsonDoc["NUMBER_OF_SLAVES"] = numOfSlaves;

  for (int i = 0; i < numOfSlaves; i++)
  {
    jsonDoc["voltage" + std::to_string(i + 1)] = voltage[i];
    jsonDoc["temperature" + std::to_string(i + 1)] = temperature[i];
    jsonDoc["status" + std::to_string(i + 1)] = status[i];
  }
  jsonDoc["maxCell"] = maxCell + 1;
  jsonDoc["maxCellVoltage"] = maxCellVoltage;

  jsonDoc["minCell"] = minCell + 1;
  jsonDoc["minCellVoltage"] = minCellVoltage;

  jsonDoc["differenceMaxMin"] = differenceMaxMin;

  /* jsonDoc["akkutyp"] = ;
   jsonDoc["numCells"] = ;*/

  String response;
  serializeJson(jsonDoc, response);
  /*Serial.println("--------------------");
  Serial.println(response);
  Serial.println("--------------------");*/
  request->send(200, "application/json", response);
}

void handleAkkutype(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  DynamicJsonDocument json(256);
  deserializeJson(json, const_cast<const char *>(reinterpret_cast<char *>(data)));
  const char *batteryType = json["batteryType"];
  Serial.println(batteryType);
}
void handleNumOfSlaves(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{

  DynamicJsonDocument json(256);
  deserializeJson(json, const_cast<const char *>(reinterpret_cast<char *>(data)));
  numOfSlaves = atoi(json["numOfSlaves"]);

  temperature = (int *)realloc(temperature, numOfSlaves * sizeof(int));
  voltage = (float *)realloc(voltage, numOfSlaves * sizeof(float));
  status = (bool *)realloc(status, numOfSlaves * sizeof(bool));

  for(int i=0;i<numOfSlaves;i++)
  {
    temperature[i]=0;
    voltage[i]=0;
    status[i]=0;
  }

  Serial.println(numOfSlaves);
}

void setupWebServer(const char *ssid, const char *password)
{
  Serial.begin(115200);

  // Starte den Access Point
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP-Adresse: ");
  Serial.println(IP);

  for(int i=0;i<numOfSlaves;i++)
  {
    temperature[i]=0;
    voltage[i]=0;
    status[i]=0;
  }

  // Initialisiere SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("Fehler beim Initialisieren von SPIFFS");
    return;
  }
  Serial.println("SPIFFS initialisiert");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/index.html", HTTP_GET, handleRoot);
  server.on("/favicon.ico", HTTP_GET, handleFavicon);
  server.on("/about-us.html", HTTP_GET, handleAboutUs);
  server.on("/css/bootstrap.min.css", HTTP_GET, handleBootstrap);
  server.on("/js/jquery-3.2.1.slim.min.js", HTTP_GET, handleScriptJS);
  server.on("/js/popper.min.js", HTTP_GET, handleScriptJS);
  server.on("/js/bootstrap.min.js", HTTP_GET, handleScriptJS);
  server.on("/js/script.js", HTTP_GET, handleScriptJS);
  server.on("/log", HTTP_GET, handleLog);
  server.on("/api/measured-values", HTTP_GET, handleFetch);

  server.on(
      "/api/numOfSlaves", HTTP_POST, [](AsyncWebServerRequest *req)
      { req->send(200, "text/plain", "success"); },
      NULL, handleNumOfSlaves);

  server.on(
      "/api/akkutyp", HTTP_POST, [](AsyncWebServerRequest *req)
      { req->send(200, "text/plain", "success"); },
      NULL, handleAkkutype);

  // Fange 404-Fehler ab
  server.onNotFound(notFound);

  server.begin();
}
