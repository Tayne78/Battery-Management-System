#include <Arduino.h>
#include "OneWireCom.h"
#include "Status.h"
#include "Logfile.h"
#include "bms_webserver.h"
#include "CAN_BMS.h"

Relay relay(26, 27);
OneWireCom oneWireCom(GPIO_NUM_2, GPIO_NUM_4);
Logfile logfile;
CAN_BMS canbus(12, 14);
Led led(GPIO_NUM_32);
HardwareSerial RS485Serial(2);

#define IGNITON_DETECTION_PIN 34
#define CHARGE_DETECTION_PIN 35
#define MAX485_ENABLE_PIN 33

#define CANID 0x100

TaskHandle_t statusled;
TaskHandle_t communication;

unsigned long timelastsend = 0, timereceive = 0, timelastsd = 0, timecharge = 0;
unsigned int com_state = 0, state = 0, charge_state = 0, ledcolor = GREEN;
bool idle = 0, error = 0;
unsigned int totale_voltage_old = 0, avg_voltage = 0;

void Led(void *pvParameters)
{
  while (1)
  {
    led.setColor(ledcolor);
    delay(1000);
    led.setColor(0x000000);
    delay(1000);
  }
}
void Communication(void *pvParameters)
{
  while (1)
  {
    digitalWrite(MAX485_ENABLE_PIN, HIGH);
    RS485Serial.print("Totale Voltage:");
    RS485Serial.print(totale_voltage);
    RS485Serial.print("Average Temperature:");
    RS485Serial.println(avg_temperature);
    RS485Serial.print("Charge State:");
    RS485Serial.println(charge_state);
    RS485Serial.print("State:");
    RS485Serial.println(state);
    RS485Serial.flush();
    digitalWrite(MAX485_ENABLE_PIN, LOW);
    canbus.send(totale_voltage / 10, avg_temperature, charge_state, state, CANID);
    delay(10000);
  }
}
void send()
{
  if (com_state == 0)
  {
    oneWireCom.send(REQ_VOLT_G);
    com_state = 1;
    timereceive = millis();
  }
  else if (com_state == 2)
  {
    delay(20);
    oneWireCom.send(REQ_TEMP_G);
    com_state = 3;
    timereceive = millis();
  }
}
void setup()
{
  Serial.begin(112500);
  relay.begin();

  canbus.begin(500E3);

  RS485Serial.begin(112500, SERIAL_8N1, 16, 17);
  pinMode(MAX485_ENABLE_PIN, OUTPUT);

  setupWebServer("BMS", "BMS1234Kaefer");

  logfile.begin("/log.txt", numOfSlaves);

  relay.turnOffRelay1(); // Externer Status
  relay.turnOffRelay2(); // Laderelay

  xTaskCreatePinnedToCore(Led, "StautsLED", 1000, NULL, 1, &statusled, 0);
  xTaskCreatePinnedToCore(Communication, "Communication", 1000, NULL, 1, &communication, 0);
}
void loop()
{
  if (millis() - timelastsend > 10000 || com_state == 2)
  {
    timelastsend = millis();
    error = 0;
    send();
  }
  if (com_state == 1 || com_state == 3)
  {
    if (millis() - timereceive > (20 * numOfSlaves))
    {
      timereceive = millis();
      for (int i = 0; i < numOfSlaves + 1; i++)
      {
        received_data[i] = oneWireCom.receive();
        if (received_data[i] == 0) // Fehler
        {
          ledcolor = RED;
          Serial.println("ERROR");
          if (error == 0)
          {
            error = 1;
          }
        }
        else
        {
          ledcolor = GREEN; // Daten Empfangen
        }
      }
      Serial.println("Daten Empfangen");
      if (com_state == 1) // Daten Auswerten für Spannung
      {
        if (error == 1)
        {
          ledcolor = RED;
          com_state = 0;
          for (int i = 0; i < numOfSlaves; i++)
            voltage[i] = 0;
        }
        else
        {
          ledcolor = GREEN;
          com_state = 2;
          for (int i = 0; i < numOfSlaves; i++)
            voltage[i] = received_data[i + 1];
        }
        Serial.println("SPANNUNG");
        for (int i = 0; i < numOfSlaves; i++)
          Serial.println(voltage[i]);
      }
      else if (com_state == 3) // Daten Auswerten für Temperatur
      {
        if (error == 1)
        {
          ledcolor = RED;
          com_state = 2;
          for (int i = 0; i < numOfSlaves; i++)
            voltage[i] = 0;
        }
        else
        {
          ledcolor = GREEN;
          com_state = 0;
          for (int i = 0; i < numOfSlaves; i++)
            temperature[i] = (received_data[i + 1] - 273);
        }
        Serial.println("TEMPERATUR");
        for (int i = 0; i < numOfSlaves; i++)
          Serial.println(temperature[i]);

        idle = 1;
      }
    }
  }
  else if (com_state == 0) // IDLE wenn alle Daten Empfangen sind (Temperatur und Spannung)
  {
    if (idle == 1)
    {
      ledcolor = GREEN;
      idle = 0;

      if ((IGNITON_DETECTION_PIN == HIGH && millis() - timelastsd > 10 * 60 * 1000) ||  
          (IGNITON_DETECTION_PIN != HIGH && millis() - timelastsd > 20 * 60 * 1000)) // Wenn Auto im Betrieb 10min sonst 20min
      {
        logfile.writeData("/log.txt", temperature.data(), voltage.data(), numOfSlaves);
        timelastsd = millis();
      }

      avg_temperature = 0;
      totale_voltage = 0;
      for (int i = 0; i < numOfSlaves; i++)
      {
        totale_voltage += voltage[i];
        avg_temperature += temperature[i];
        sorted_voltages[i] = std::make_pair(voltage[i], i);
      }
      if (millis() - timecharge > 5 * 60 * 1000 && totale_voltage_old < totale_voltage) // Alle 5 Minuten prüfen ob Spannung der Zellen gestiegen, LADEN
      {
        timecharge = millis();
        state = CHARGE;
        ledcolor = WHITE;
        Serial.println("LADEN");
        totale_voltage_old = totale_voltage;
      }
      avg_voltage = totale_voltage / numOfSlaves;
      avg_temperature = avg_temperature / numOfSlaves;

      Serial.println(avg_voltage);
      Serial.println(totale_voltage_old);
      Serial.println(totale_voltage);
      Serial.println(avg_temperature);

      for (int i = 0; i < numOfSlaves - 1; i++)
      {
        for (int j = 0; j < numOfSlaves - 1 - i; j++)
        {
          if (sorted_temperature[j] > sorted_temperature[j + 1])
          {
            int temp_value = sorted_temperature[j];
            sorted_temperature[j] = sorted_temperature[j + 1];
            sorted_temperature[j + 1] = temp_value;
          }
        }
      }

      sortBy(true);

      const auto maxCell = sorted_voltages.at(sorted_voltages.size() - 1);
      const auto minCell = sorted_voltages.at(0);

      maxCellVoltage = std::get<0>(maxCell);
      maxCellId = std::get<1>(maxCell);

      minCellVoltage = std::get<0>(minCell);
      minCellId = std::get<1>(minCell);

      differenceMaxMin = maxCellVoltage - minCellVoltage;

      if ((maxCellVoltage <= battery.endVoltage) && (maxCellVoltage >= (battery.endVoltage - 50))) // LADESCHLUSSSPANNUNG
      {
        ledcolor = TURQOISE;
        relay.turnOffRelay2(); // Laderelay
        relay.turnOnRelay1();  // Externer Status
        state = END_VOLTAGE;
        charge_state = CANNOT_BE_CHARGED;
        Serial.println("FULL Charged");
      }
      else if (maxCellVoltage > battery.endVoltage) // Overvoltage
      {
        ledcolor = BLUE;
        relay.turnOffRelay2(); // Laderelay
        relay.turnOnRelay1();  // Externer Status
        state = OVERVOLTAGE;
        charge_state = CANNOT_BE_CHARGED;
        Serial.println("OverVoltage");
      }
      else if (minCellVoltage < battery.minVoltage) // Undervoltage
      {
        ledcolor = YELLOW;
        relay.turnOffRelay2(); // Laderelay
        relay.turnOnRelay1();  // Externer Status
        state = UNDERVOLTAGE;
        charge_state = CANNOT_BE_CHARGED;
        Serial.println("UnderVoltage");
      }
      else
      {
        ledcolor = GREEN;
        state = GOOD;
        charge_state = CAN_BE_CHARGED;
        Serial.println("IDLE");
      }

      if (sorted_temperature[numOfSlaves - 1] > battery.maxTemperature) // Overheating
      {
        ledcolor = PURPLE;
        relay.turnOffRelay2(); // Laderelay
        relay.turnOnRelay1();  // Externer Status
        state = OVERHEATING;
        charge_state = CANNOT_BE_CHARGED;
        Serial.println("Overheating");
      }

      if (charge_state == CAN_BE_CHARGED && CHARGE_DETECTION_PIN == HIGH) // LADEN
      {
        Serial.println("Ladegerät angesteckt und Ladebereit");
        relay.turnOnRelay2();
      }
      // Überarbeiten
      /* // BALANCEN wenn Auto ausgeschalten und ein Spannungsunterschied von 50mV herscht und die Zellespannung gößer der mininmale Balancespannung ist
       if (IGNITON_DETECTION_PIN == LOW && )
       {
         relay.turnOffRelay1(); // Laden für Balancen unterbrechen um Overheating zu vermeiden
       }*/
    }
  }
}
