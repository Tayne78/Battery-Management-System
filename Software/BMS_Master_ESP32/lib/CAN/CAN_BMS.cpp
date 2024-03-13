#include "CAN_BMS.h"

CAN_BMS::CAN_BMS(uint8_t _rxPin, uint8_t _txPin)
{
    rxPin = _rxPin;
    txPin = _txPin;
    CAN.setPins(rxPin, txPin);
}
void CAN_BMS::begin(uint32_t transfer_rate)
{
    if (!CAN.begin(transfer_rate))
    {
        Serial.println("Starting CAN failed!");
        while (1)
            ;
    }
    else
    {
        Serial.println("CAN Initialized");
    }
}
bool CAN_BMS::send(uint16_t total_voltage, uint8_t temperatur, uint8_t chargestate, uint8_t state_error, uint16_t canId)
{
    CAN.beginPacket(canId,DLC,0);
    CAN.write(total_voltage & 0xFF);
    CAN.write((total_voltage >> 8) & 0xFF);
    CAN.write(temperatur);
    CAN.write(chargestate);
    CAN.write(state_error);
    if (CAN.endPacket() == 1)
    {
        return true; // Erfolgreich gesendet
    }
    else
    {
        return false; // Sendefehler
    }
}
int CAN_BMS::receive()
{
    int packetSize = CAN.parsePacket();

    if (packetSize)
    {
        // received a packet
        Serial.print("Received ");

        if (CAN.packetExtended())
        {
            Serial.print("extended ");
        }

        if (CAN.packetRtr())
        {
            // Remote transmission request, packet contains no data
            Serial.print("RTR ");
        }

        Serial.print("packet with id 0x");
        Serial.print(CAN.packetId(), HEX);

        if (CAN.packetRtr())
        {
            Serial.print(" and requested length ");
            Serial.println(CAN.packetDlc());
        }
        else
        {
            Serial.print(" and length ");
            Serial.println(packetSize);

            // only print packet data for non-RTR packets
            while (CAN.available())
            {
                Serial.print((char)CAN.read());
            }
            Serial.println();
        }

        Serial.println();
    }
    return 0;
}