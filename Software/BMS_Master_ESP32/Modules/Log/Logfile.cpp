#include "Logfile.h"

Logfile::Logfile() {}

void Logfile::begin()
{
  if (!SD.begin(5))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1)
      delay(10);
  }
  if (!rtc.isrunning())
  {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void Logfile::writeData(const char *path, int t[], float v[], int numofcells)
{
  DateTime now = rtc.now();
  String data = String(now.year()) + '.' + String(now.month()) + '.' + String(now.day()) + '/' + String(now.hour()) + ':' + String(now.minute()) + ':' + String(now.second()) + "/";
  for (int i = 0; i < numofcells; i++)
    data += "Cell:" + String(i + 1) + " T:" + String(t[i]) + " V:" + String(v[i]) + '/';
  data += '\n';
  Serial.print("\nSaving data: ");
  Serial.println(data);
  File file = SD.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(data))
  {
    Serial.println("File written\n");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}

void Logfile::createLog(const char *path)
{
  File file = SD.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to create file");
    return;
  }
  file.close();
}

void Logfile::readLog(const char *path, int numofLines, String data[][3])
{

  if (!SD.exists(path)) {
    createLog(path);
  }
  File file = SD.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  String lines[numofLines];
  int lineCount = 0;

  while (file.available()) // Daten von SD-Karte zwischenspeichern
  {
    String line = file.readStringUntil('\n');
    if (lineCount >= numofLines) {
      for (int i = 0; i < numofLines - 1; i++) {
        lines[i] = lines[i + 1];
      }
      lines[numofLines - 1] = line;
    } else {
      lines[lineCount] = line;
      lineCount++;
    }
  }
  file.close();
  for (int i = 0; i < numofLines; i++) {
    int endfirstIndex = lines[i].indexOf('/');
    data[i][0] = lines[i].substring(0, endfirstIndex);
    
    int endsecondIndex = lines[i].indexOf('/', endfirstIndex + 1);
    data[i][1] = lines[i].substring(endfirstIndex + 1, endsecondIndex);

    String dataPart = lines[i].substring(endsecondIndex + 1);

    data[i][2] = dataPart;

    data[i][2].replace("Cell:", "\nCell:");

  }
}