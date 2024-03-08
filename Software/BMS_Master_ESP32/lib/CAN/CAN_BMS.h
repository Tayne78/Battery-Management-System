#ifndef CAN_BMS_h
#define CAN_BMS_h

#include <Arduino.h>
#include <CAN.h>

#define GOOD 0x00
#define CHARGE 0x01
#define END_VOLTAGE 0x02
#define ERROR 0x10
#define OVERVOLTAGE 0x20
#define UNDERVOLTAGE 0x30
#define OVERHEATING 0x40

#define CANNOT_BE_CHARGED 0x00
#define CAN_BE_CHARGED 0x01

#define DLC 5 //Packetgröße in Byte 

class CAN_BMS
{
public:
  CAN_BMS(uint8_t _rxPin, uint8_t _txPin);
  void begin();
  bool send(uint16_t total_voltage, uint8_t temperatur, uint8_t chargestate, uint8_t state_error, uint16_t canId);
  int receive();

private:
  uint8_t rxPin;
  uint8_t txPin;
};

#endif