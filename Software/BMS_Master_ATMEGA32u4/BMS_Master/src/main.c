/*
 * Recieve_Test.c
 *
 * Created: 06.04.2022 08:51:21
 * Author : trist
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>


#include "communication.h"
#include "status.h"
#include "manch_m.h"
#include "timer.h" 

#define ANZ_ZELLEN 2 // max. 15 (wegen berechnungen mittelwert mean)

uint8_t register manch_bit asm("r16"); // in manch_h verschieben!

uint16_t spg[ANZ_ZELLEN+1];
uint16_t temp[ANZ_ZELLEN+1];
uint16_t xyz[ANZ_ZELLEN];
uint16_t min;
uint16_t *p_data;
uint8_t rec_nr;	// wieviel antworten wurden empfangen?
uint8_t state=8;	// für die communikation mit slaves, 0 => startet abfrage, 8 => fertig, nix zu tun
uint8_t cont_state=0;	// state für die control-funktion
uint8_t balance_nr;

void main_control(void)
{
	uint8_t i;
	switch (cont_state)
	{

//--------- spg. abfragen ------------------------------------------
		case 0: // spg. abfragen
			gl_manch_dat = REQ_VOLT_G;	// spannungsabfrage
			p_data = spg;
			state = 0;
			timer_clear_timer(CONTROL);
			cont_state = 1;
			break;

		case 1: // warten auf spannungen und auswerten
			if (state == 8) //fertig!
			{
				if (rec_nr == ANZ_ZELLEN) // ohne fehler empfangen
				{
					spg[ANZ_ZELLEN]=1; // aktuelle daten
					printf("U ");
					for (i=0; i<ANZ_ZELLEN; i++)
						printf("%d ",spg[i]);
					printf("\n");
				}
				else
				{
					spg[ANZ_ZELLEN]=0; // fehlerhafte daten
					printf("U ERROR\n");
				}
				cont_state = 2;
			}
			break;

//----------- temp abfragen ----------------------------------------------
		case 2: // temp. abfragen
			gl_manch_dat = REQ_TEMP_G;	// spannungsabfrage
			p_data = temp;
			state = 0;
			cont_state = 3;
			break;

		case 3: // warten auf temperatur und auswerten
			if (state == 8) //fertig!
			{
				if (rec_nr == ANZ_ZELLEN) // ohne fehler empfangen
				{
					temp[ANZ_ZELLEN]=1; // aktuelle daten
					printf("T ");
					for (i=0; i<ANZ_ZELLEN; i++)
						printf("%d ",temp[i]-275);
					printf("\n");
				}
				else
				{
					temp[ANZ_ZELLEN]=0; // fehlerhafte daten
					printf("T ERROR\n");
				}
				cont_state = 10;
			}
			break;

//-------------- pause ----------------------------------------------
		case 4: // pause
			if (timer_get_timer(CONTROL) >= 1000)
			{
				cont_state = 0;
			}
			break;

//------------ balancing --------------------------------------------
// wenn keine zündung und gültige spg. => 
//        wenn spg. um 50mV größer als kleinste und größer 3,7V -> entladen!
		case 10: // balancing
			printf("B ");
			min = spg[0];
			for (i=1; i<ANZ_ZELLEN; i++)
				if (spg[i] < min)
					min = spg[i];
			min = min + 50;
			balance_nr = 0;
			cont_state = 11;
			break;
			
		case 11:
			if ( (spg[balance_nr] > 3700) & (spg[balance_nr] > min) )
			{
				gl_manch_dat = COM_BLC_A + balance_nr+1;	// balancing
				p_data = xyz;
				state = 0;
				cont_state = 12;
			}
			else
				cont_state = 13;
			break;
			
		case 12: // warten auf antwort von balancen
			if (state == 8) //fertig!
			{
				if (rec_nr == balance_nr+1) // ohne fehler empfangen
				{
					printf("%d ",balance_nr);
				}
				cont_state = 13;
			}
			break;
			
		case 13: // nächste zelle
			balance_nr ++;
			if (balance_nr >= ANZ_ZELLEN)
			{
				printf("\n");
				timer_clear_timer(CONTROL);
				cont_state = 4;
			}
			else
				cont_state = 11;
			break;
				
			
				

	} // switch
}


int main(void)
{
	
	//fx
DDRB|= (1<<PINB3);
//PORTD&=~(1<<PIND0);	
	uint16_t daten;
	uint8_t com_stat=0;

	
	timer_init_timer();
	stat_led_init();
	RS232_init();

	uint16_t counter=0;

	sei(); 

   while (1) 
   {
		timer_add_time();
		main_control();
		
// ----------- communication with slave -------------------------------------------------
		if (state == 0)
		{
			//gl_manch_dat = COM_BLC_A+2;
			manch_init_send();
			state = 1;
		}
		else if (state==1)	// warten, bis fertig gesendet
		{
			com_stat=manch_result();
			if (com_stat == 1)
			{
				//_delay_ms(2);
				rec_nr=0;
				state = 5;
			}
		}
		else if (state==5) // von oben empfangen
		{
			timer_clear_timer(MAIN); 
			manch_init_receive();
			state = 6;
		}
		else if(state==6)		//Daten von oben warten
		{
			com_stat=manch_result();
			
			if (com_stat==0)		//während auf Daten gewartet wird LED orange blinken
			{
				stat_led_off();
			}
			else if(com_stat==1)			//wenn Daten erfolgreich Empfangen wurden LED grün
			{
				p_data[rec_nr]=gl_manch_dat & 0x7FFF;
				rec_nr++;
				stat_led_green();
				state=5;
			}
			else //if (com_stat==2)		//wenn Fehler beim Empfangen LED rot
			{
				stat_led_red();
				//_delay_ms(100);
				state=8;
			}
			
			if (timer_get_timer(MAIN) >= 25) // time out, kommt nix von oben
			{
				manch_stop_receive();
				//stat_led_red();
				state = 8;
			}
		}
		else if (state==8) // ready
		{
			//for (i=0; i<ANZ_ZELLEN; i++)
			//	printf("%d ",spg[i]);
			//_delay_ms(1000);
			//state = 0;
		}
		
				
    }//while
}