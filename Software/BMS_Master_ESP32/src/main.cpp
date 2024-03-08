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

#define Cells 3
#define IgnitondetectionPin 34
#define ChargedetectionPin 35
#define ladeschlussspannung 4200
#define MAXTemperature 55
#define MINBalanceVoltage 3700

TaskHandle_t statusled;

int com_state = 0, timelastsend = 0, timereceive = 0, state = 0, charge_state = 0, timeled = 0, ledcolor = GREEN;
bool idle = 0, error = 0;
int totale_voltage_old = 0, totale_voltage_new = 0, avg_temperature = 0, sorted_voltages[Cells][2], sorted_temperature[Cells];
int data[Cells + 1];


void toggle(void *pvParameters)
{
  for (;;)
  {
    led.setColor(ledcolor);
    delay(1000);
    led.setColor(0x000000);
    delay(1000);
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

  oneWireCom.begin();

  canbus.begin();

  setupWebServer("BMS", "Thomas123");

  logfile.begin();
  logfile.createLog("/log.txt", Cells);

  relay.turnOnRelay1();
  relay.turnOffRelay2();

  xTaskCreatePinnedToCore(toggle, "StautsLED", 1000, NULL, 1, &statusled, 0);
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
    if (millis() - timereceive > (20 * Cells))
    {
      timereceive = millis();
      for (int i = 0; i < Cells + 1; i++)
      {
        data[i] = oneWireCom.receive();
        if (data[i] == 0) // Fehler
        {
          ledcolor = 0x000F0F;
          Serial.println("ERROR");
          if (error == 0)
          {
            error = 1;
          }
        }
        else
        {
          ledcolor = GREEN; // Alle Daten Empfangen
        }
      }
      Serial.println("Daten Empfangen");
      if (com_state == 1) // Daten Auswerten für Spannung
      {
        if (error == 1)
        {
          com_state = 0;
          for (int i = 0; i < Cells; i++)
            voltage[i] = 0;
        }
        else
        {
          com_state = 2;
          for (int i = 0; i < Cells; i++)
            voltage[i] = data[i + 1];
        }
        Serial.println("SPANNUNG");
        for (int i = 0; i < Cells; i++)
          Serial.println(voltage[i]);
      }
      else if (com_state == 3) // Daten Auswerten für Temperatur
      {
        if (error == 1)
        {
          com_state = 2;
          for (int i = 0; i < Cells; i++)
            voltage[i] = 0;
        }
        else
        {
          com_state = 0;
          for (int i = 0; i < Cells; i++)
            temperature[i] = (data[i + 1] - 273);
        }
        Serial.println("TEMPERATUR");
        for (int i = 0; i < Cells; i++)
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
      logfile.writeData("/log.txt", temperature, voltage, Cells);
      totale_voltage_new = 0;
      avg_temperature = 0;

      for (int i = 0; i < Cells; i++)
      {
        totale_voltage_new += voltage[i];
        avg_temperature += temperature[i];
        sorted_voltages[i][0] = voltage[i];
        sorted_voltages[i][1] = i;
      }
      if (totale_voltage_old < totale_voltage_new) // Überarbeiten
      {
        state = CHARGE;
        ledcolor = 0xF00000;
        Serial.println("LADEN");
      }
      totale_voltage_old = totale_voltage_new;
      avg_temperature = avg_temperature / Cells;
      Serial.println(totale_voltage_old);
      Serial.println(totale_voltage_new);
      Serial.println(avg_temperature);

      for (int i = 0; i < Cells - 1; i++)
      {
        for (int j = 0; j < Cells - 1 - i; j++)
        {
          if (sorted_voltages[j][0] > sorted_voltages[j + 1][0])
          {
            int temp_value = sorted_voltages[j][0];
            int temp_index = sorted_voltages[j][1];

            sorted_voltages[j][0] = sorted_voltages[j + 1][0];
            sorted_voltages[j][1] = sorted_voltages[j + 1][1];

            sorted_voltages[j + 1][0] = temp_value;
            sorted_voltages[j + 1][1] = temp_index;
          }
          if (sorted_temperature[j] > sorted_temperature[j + 1])
          {
            int temp_value = sorted_temperature[j];

            sorted_temperature[j] = sorted_temperature[j + 1];

            sorted_temperature[j + 1] = temp_value;
          }
        }
      }
      if ((sorted_voltages[Cells - 1][0] <= ladeschlussspannung) && (sorted_voltages[Cells - 1][0] >= (ladeschlussspannung - 50))) // ladeschlussspannung
      {
        relay.turnOffRelay2();
        state = END_VOLTAGE;
        charge_state = CANNOT_BE_CHARGED;
        Serial.println("FULL Charged");
      }
      else if (sorted_voltages[Cells - 1][0] > ladeschlussspannung) // Overvoltage
      {
        relay.turnOffRelay2();
        state = OVERVOLTAGE;
        charge_state = CANNOT_BE_CHARGED;
        Serial.println("OverVoltage");
      }
      else if (sorted_voltages[0][0] < 3300) // Undervoltage
      {
        relay.turnOffRelay2();
        state = UNDERVOLTAGE;
        charge_state = CANNOT_BE_CHARGED;
        Serial.println("UnderVoltage");
      }
      else
      {
        state = GOOD;
        charge_state = CAN_BE_CHARGED;
        Serial.println("IDLE");
      }

      if (sorted_temperature[Cells - 1] > MAXTemperature) // Overheating
      {
        relay.turnOffRelay2();
        state = OVERHEATING;
        charge_state = CANNOT_BE_CHARGED;
        Serial.println("Overheating");
      }

      if (charge_state == CAN_BE_CHARGED && ChargedetectionPin == HIGH) // Überarbeiten
      {
        Serial.println("LADEN");
        relay.turnOnRelay2();
      }
      canbus.send(totale_voltage_new, avg_temperature, charge_state, state, 0x100);

      // Überarbeiten
      // BALANCEN wenn Auto ausgeschalten und ein Spannungsunterschied von 50mV herscht und die Zellespannung gößer der mininmale Balancespannung ist
      if (IgnitondetectionPin == LOW && ((sorted_voltages[Cells - 1][0] >= (sorted_voltages[0][0] + 50)) && (sorted_voltages[Cells - 1][0] > MINBalanceVoltage)))
      {
        relay.turnOffRelay1(); // Laden für Balancen unterbrechen um Overheating zu vermeiden
      }
    }
  }
}
