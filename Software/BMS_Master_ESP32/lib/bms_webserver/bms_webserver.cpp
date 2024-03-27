#include "bms_webserver.h"

AsyncWebServer server(80);

int numOfSlaves = 2;

std::vector<int> temperature;
std::vector<int> voltage;
std::vector<bool> status;

std::vector<int> sorted_temperature;
std::vector<int> received_data;

// int **sorted_voltages;

constexpr const bool SORT_BY_VOLTAGE = true;
constexpr const bool SORT_BY_SLAVE_ID = false;

std::vector<SlaveVoltagePair> sorted_voltages;

int maxCellVoltage = 0;
int minCellVoltage = 0;
int differenceMaxMin = 0;

int totale_voltage = 0;
int avg_temperature = 0;

int maxCellId = 0;
int minCellId = 0;

String batteryType="Lithium-Ionen";

void sortBy(const bool v)
{ // voltage
  std::sort(sorted_voltages.begin(), sorted_voltages.end(), [&](SlaveVoltagePair a, SlaveVoltagePair b)
            {
        if (v) return std::get<0>(a) < std::get<0>(b);
        else return std::get<1>(a) < std::get<1>(b); });
}

void changeVoltage(unsigned int slaveId, unsigned int newVoltage)
{
  for (auto &slave : sorted_voltages)
  {
    if (std::get<1>(slave) == slaveId)
      std::get<0>(slave) = newVoltage;
  }
}

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
void handleLogo(AsyncWebServerRequest *request)
{
  File file = SPIFFS.open("/logo.png");
  if (file)
  {
    request->send(SPIFFS, "/logo.png", "image/png");
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
    jsonDoc["status" + std::to_string(i + 1)] = std::to_string(status[i]);
  }
  jsonDoc["averageTemp"] = avg_temperature;
  jsonDoc["overallVolt"] = totale_voltage;

  jsonDoc["maxCell"] = maxCellId + 1;
  jsonDoc["maxCellVoltage"] = maxCellVoltage;

  jsonDoc["minCell"] = minCellId + 1;
  jsonDoc["minCellVoltage"] = minCellVoltage;

  jsonDoc["differenceMaxMin"] = differenceMaxMin;

  jsonDoc["akkutyp"] = batteryType;

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
  // const char *batteryType =
  batteryType = String(json["batteryType"].as<const char *>());
  Serial.println(batteryType);
}
void handleNumOfSlaves(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{

  DynamicJsonDocument json(256);
  deserializeJson(json, const_cast<const char *>(reinterpret_cast<char *>(data)));
  numOfSlaves = atoi(json["numOfSlaves"]);
  Serial.println(numOfSlaves);

  temperature.resize(numOfSlaves);
  voltage.resize(numOfSlaves);
  status.resize(numOfSlaves);

  int diff = numOfSlaves - sorted_voltages.size();
  const bool shrink = (diff < 0);
  sortBy(false);

  for (int i = 0; i < std::abs(diff); ++i)
  {
    if (shrink)
      sorted_voltages.pop_back();
    else
      sorted_voltages.push_back({0, sorted_voltages.size() + 1});
  }
  sorted_temperature.resize(numOfSlaves);
  received_data.resize(numOfSlaves + 1);

  /*std::fill(temperature.begin(), temperature.end(), 0);
  std::fill(voltage.begin(), voltage.end(), 0);
  std::fill(status.begin(), status.end(), 0);*/

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

  temperature.resize(numOfSlaves);
  voltage.resize(numOfSlaves);
  status.resize(numOfSlaves);
  sorted_temperature.resize(numOfSlaves);
  received_data.resize(numOfSlaves + 1);

  sorted_voltages.push_back({0, 1});
  sorted_voltages.push_back({0, 2});

  std::fill(temperature.begin(), temperature.end(), 0);
  std::fill(voltage.begin(), voltage.end(), 0);
  std::fill(status.begin(), status.end(), 0);

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
  server.on("/logo.png", HTTP_GET, handleLogo);
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
