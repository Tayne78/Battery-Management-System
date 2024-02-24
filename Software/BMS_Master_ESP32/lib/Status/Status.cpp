#include "Status.h"

Led::Led(int pin) : pixel(Adafruit_NeoPixel(1, pin, NEO_RGB + NEO_KHZ800)) {}

void Led::begin()
{
    pixel.begin();
    pixel.show();
}
void Led::setColor(int r, int g, int b)
{
    pixel.setPixelColor(0, pixel.Color(r, g, b));
    pixel.show();
}
void Led::setColor(int color)
{
    pixel.setPixelColor(0,color);
    pixel.show();
}
void Led::clear()
{
    pixel.clear();
}

Relay::Relay(int _relayPin1, int _relayPin2)
{
    relayPin1 = _relayPin1;
    relayPin2 = _relayPin2;
    relay1State = false;
    relay2State = false;
}
void Relay::begin()
{
    pinMode(relayPin1,OUTPUT);
    pinMode(relayPin2,OUTPUT);
    turnOffRelay1();
    turnOffRelay2();
}
void Relay::turnOnRelay1()
{
    digitalWrite(relayPin1,HIGH);
    relay1State = true;
}
void Relay::turnOffRelay1()
{
    digitalWrite(relayPin1,LOW);
    relay1State = false;
}
bool Relay::stateRelay1()
{
    return relay1State;
}
void Relay::turnOnRelay2()
{
    digitalWrite(relayPin2,HIGH);
    relay2State = true;
}
void Relay::turnOffRelay2()
{
    digitalWrite(relayPin2,LOW);
    relay2State = false;
}
bool Relay::stateRelay2()
{
    return relay2State;
}