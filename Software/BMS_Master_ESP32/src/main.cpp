#include <Arduino.h>
#include "OneWireCom.h"
#include "Status.h"
#include "Logfile.h"
#include "bms_webserver.h"
#include "CAN_BMS.h"

//TaskHandle_t CAN;

Relay relay(26, 27);
OneWireCom oneWireCom(GPIO_NUM_2, GPIO_NUM_4);
Logfile logfile;
CAN_BMS canbus(12,14);
Led led(GPIO_NUM_32);


#define Cells 2
#define IgnitondetectionPin 34
#define ChargedetectionPin 35
#define ladeschlusssspannung 4200

int state = 0, com_stat = 0, timelastsend = 0, lastChange = 0;
bool idle = 0;
bool error = 0;
int totale_voltage=0;
int data[Cells + 1];

/*void can(void *parameter)
{
  while (1)
  {
    canbus.send(total_voltage,12,CAN_BE_CHARGED,GOOD,0x100);
  }
  
}*/

void setup()
{
  Serial.begin(112500);
  relay.begin();
  delay(1000);
  
  oneWireCom.begin();

  canbus.begin();

  setupWebServer("BMS","Thomas123");

  logfile.begin();
  logfile.createLog("/log.txt",Cells);
 // xTaskCreatePinnedToCore(can, "Core1Task", 10000, NULL, 2, &CAN, 0);
}
void send()
{
  if (state == 0)
  {
    oneWireCom.send(REQ_VOLT_G);
    state = 1;
    lastChange = millis();
  }
  else if (state == 2)
  {
    delay(20);
    oneWireCom.send(REQ_TEMP_G);
    state = 3;
    lastChange = millis();
  }
}

void loop()
{
  if (millis() - timelastsend > 5000 || state == 2)
  {
    timelastsend = millis();
    error=0;
    send();
  }
  if (state == 1 || state == 3)
  {
    if (millis() - lastChange > (20 * Cells))
    {
      lastChange = millis();
      for (int i = 0; i < Cells + 1; i++)
      {
        data[i] = oneWireCom.receive();
        if (data[i] == 0)
        {
          led.setColor(RED);  //Fehler
          Serial.println("ERROR");
          if (error == 0)
          {
            error = 1;
          }
        }
        else
        {
          led.setColor(GREEN); //Alle Daten Empfangen
        }
      }
      Serial.println("Daten Empfangen");
      if (state == 1)   //Daten Auswerten für Spannung
      {
        if (error == 1)
        {
          state = 0;
          for (int i = 0; i < Cells; i++)   
            voltage[i] = 0;
        }
        else
        {
          state = 2;
          for (int i = 0; i < Cells; i++)
            voltage[i] = data[i + 1];
        }
        Serial.println("SPANNUNG");
        for (int i = 0; i < Cells; i++)
          Serial.println(voltage[i]);
      }
      else if (state == 3) //Daten Auswerten für Temperatur
      {
        if (error == 1)
        {
          state = 2;
          for (int i = 0; i < Cells; i++)
            voltage[i] = 0;
        }
        else
        {
          state = 0;
          for (int i = 0; i < Cells; i++)
            temperature[i] = (data[i + 1] - 273);
        }
        Serial.println("TEMPERATUR");
        for (int i = 0; i < Cells; i++)
          Serial.println(temperature[i]);

        idle=1;
      }
    }
  }
  else if(state==0) //IDLE wenn alle Daten Empfangen sind (Temperatur und Spannung)
  {
    if(idle==1) 
    {
      led.setColor(BLUE); 
      idle=0;
      logfile.writeData("/log.csv",temperature,voltage,Cells);
      totale_voltage = 0;
      for(int i=0;i<Cells;i++)
      {
        totale_voltage += voltage[i];
        status[i]=1;
      }
      Serial.println(totale_voltage);
      status[0]=0;

      //TODO
      if(IgnitondetectionPin==LOW)
      {
        
      }
    }
  }


  /*
  com_stat = oneWireCom.getstatus();
  Serial.println(com_stat);
  if (com_stat == 0)
  {
    // led.setColor(BLUE);
  }
  else if (com_stat == 1)
  {
    data[num] = oneWireCom.receive();
    lastChange = millis();
    oneWireCom.setstatus();
    if(data[num]==0)
    {
      //ERROR
      // led.setColor(RED);
    }
    else
    {
      num++;
      // led.setColor(GREEN);
    }
  }
  if (com_stat == 0 && (millis() - lastChange >= 25 && millis() - lastChange <= 30))
  {

    Serial.println("Daten Empfangen");
    Serial.println(num);
    if(num==(Cells+1))
    {
      //led.setColor(GREEN);
    }
    else
    {
      //Daten unvollständig
      //led.setColor(RED);
    }
    num = 0;
    if (state == 1)
    {
      for(int i=0;i<Cells;i++)
        voltages[i]=data[i+1];
      state = 2;
    }
    else if (state == 3)
    {
        for(int i=0;i<Cells;i++)
          temperatures[i]=data[i+1];
        state = 0;
    }
    for(int i=0;i<3;i++)
    {
      Serial.println(data[i]);
      data[i]=0;
    }
  }*/

  /* if (digitalRead(IgnitondetectionPin) != HIGH) // Zündung Aus
   {
   }

   for (int i = 0; i < Cells - 1; i++)
   {
     float temp1, temp2;
     for (int j = 0; j < Cells - 1 - i; j++)
     {
       if (voltages[j][0] > voltages[j + 1][0])
       {
         temp1 = voltages[j + 1][0];
         temp2 = voltages[j + 1][1];
         voltages[j + 1][0] = voltages[j][0];
         voltages[j + 1][1] = voltages[j][1];
         voltages[j][0] = temp1;
         voltages[j][1] = temp2;
       }
     }
   }
   delay(1000);*/
}
