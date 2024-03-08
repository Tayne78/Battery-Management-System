#ifndef Status_h
#define Status_h

#include <Arduino.h>
#include <driver/rmt.h>


#define RED 0x000F00  
#define GREEN 0x0F0000
#define BLUE 0x00000F
#define ORANGE 0x0F0F00
#define TURQOISE 0x4F000F

class Led
{
public:
    Led(gpio_num_t _pin);
    void setColor(uint32_t color);
private:
    gpio_num_t pin;
    rmt_item32_t led_data_buffer[24];
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
