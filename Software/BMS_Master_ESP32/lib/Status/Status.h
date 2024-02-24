#ifndef Status_h
#define Status_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define RED 0xFF0000  
#define GREEN 0x00FF00
#define BLUE 0x0000FF

class Led
{
public:
    Led(int pin);
    void begin();
    void setColor(int r, int g, int b);
    void setColor(int color);
    void clear();

private:
    Adafruit_NeoPixel pixel;
};

class Relay
{
public:
    Relay(int relayPin1, int relayPin2);
    void begin();
    void turnOnRelay1();
    void turnOffRelay1();
    bool stateRelay1();
    void turnOnRelay2();
    void turnOffRelay2();
    bool stateRelay2();

private:
    int relayPin1;
    int relayPin2;
    bool relay1State;
    bool relay2State;
};

#endif
