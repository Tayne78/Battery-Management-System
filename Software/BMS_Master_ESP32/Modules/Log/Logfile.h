#ifndef Logfile_h
#define Logfile_h

#include <Arduino.h>
#include <vector>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "RTClib.h"

class Logfile {
  public:
    Logfile();

    void begin();
    void writeData(const char *path, int t[], float v[], int numofcells);
    void createLog(const char *path);
    void readLog(const char *path, int numofLines, String [][3]);

  private:
    RTC_DS1307 rtc;
};

#endif
