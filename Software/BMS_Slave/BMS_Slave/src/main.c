/*********************************************/
/*  file:	main.c (V1.0)                    */
/*						                     */
/*  Diploma Thesis:                          */
/*   Battery Management System 2021/22       */
/* 						                     */
/*  brief:  Application for the Slaves       */
/* 						                     */
/*  Author: Simon Ball                       */
/*********************************************/

//erzeugt das programm zum calibrieren ACHTUNG
//#define CALIBRATION

//--------------CPU-FREQUENCY------------------------------------------------------//
//--Define CPU frequency, if not already defined in the platformio.ini or intellisense
#ifndef F_CPU
error! #define F_CPU 2000000L
#endif

//--------------USED-HARDWARE------------------------------------------------------//
//--Define Microcontroller, if not already defined in the platform.ini or intellisense
#ifndef __AVR_ATtiny261A__
error! not implementet!
#endif

//--------------PIN-DEFINITIONS----------------------------------------------------//
#define DEBUG_DDR DDRB
#define DEBUG_PORT PORTB
#define DEBUG_PIN PINB5 // PCINT13

//--------------LIBRARY-INCLUDES-------------------------------------------------//
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/delay.h>

//--------------SOURCE-FILES-----------------------------------------------------//
// These are stored outside of the project folder, but will still be compiled
#include "ADC.h"
#include "communication.h"
#include "timer.h"
#include "manch_m.h"
#include "status.h"

enum ADC_STAT
{
	MEASURE_VOLT,
	MEASURE_TEMP
};

enum COMM_STAT
{
	COMM_RECEIVE,
	COMM_GLOBAL,
	COMM_ADD,
	COMM_SEND
};

void bms_slave_init(void);
void eeprom_callibrate(uint8_t eeprom_stat);

uint8_t register manch_bit asm("r16"); // in manch_h verschieben!

//--------------MAIN----------------------------------------------------------------//
int main(void)
{
	uint32_t volt_k;
	bms_slave_init(); // Initiating the MCU, Registers configurated
	// Data received
	uint16_t bot_received = 0; // data received from the lower slave
	uint8_t address_received = 0;
	uint16_t top_received = 0; // data received from the upper slave

	uint8_t comm_stat_b = 0; // Status of the communication to the lower slave
	uint8_t comm_stat_t = 0; // Status of the communication to the upper slave

	// Communication Cycle
	uint8_t comm_stat = COMM_RECEIVE;

	// Timing
	uint8_t BALANCE_time = 0; // compare value

	uint8_t ADCstat = MEASURE_VOLT;
	// 0  : Set up for Battery Temperature Measurement
	// 1  : Set up for Battery Voltage Measurement

	// Measurements
	uint16_t volt_raw = 0;

	uint16_t battery_temperature=0; // battery_temperature in K 
	uint16_t battery_voltage=0;	// battery_voltage = (float)adc_value / 200; // divided by 1024 aka 10-bit, multiplied by 2,56 aka internal reference voltage * 2 (voltage divider)
//	uint8_t eeprom_stat = eeprom_read_byte(EEPROM_STATUS_ADR);
	uint16_t eeprom_voltage_h = eeprom_read_word(EEPROM_4V_ADR);
	uint16_t eeprom_voltage_l = eeprom_read_word(EEPROM_3V_ADR);
	uint16_t eeprom_temp = eeprom_read_word(EEPROM_temp_ADR);
	
	if ( (eeprom_voltage_h==0xffff) || (eeprom_voltage_l==0xffff) || (eeprom_temp==0xffff) ) // if EEPROM not complite calibrated
	{
#ifndef CALIBRATION
		while (1)
		{
			_delay_ms(250);
			stat_led_off();
			_delay_ms(100);
			stat_led_red();
		}
	}
#endif // not CALIBRATION

//============ calibration ======================================================================
#ifdef CALIBRATION
		_delay_ms(1500);

		// TEMP CALLIBRATION
		if (eeprom_temp==0xffff)
		{
			while (battery_temperature == 0) // Measure ambient temperature
				battery_temperature = measure_temperature();
			eeprom_write_word(EEPROM_temp_ADR, battery_temperature/ADC_SAMPLES_T);
		}

		// VOLT CALLIBRATION
		while (battery_voltage == 0) // Measure SUPPLY voltage
			battery_voltage = measure_voltage();

		// 3V detection
		if ((battery_voltage <= (CAL_VOLT_LT*ADC_SAMPLES_V)) && (battery_voltage >= (CAL_VOLT_LB*ADC_SAMPLES_V)) && 
								(eeprom_voltage_l==0xffff)) // battery voltage in low borders and not calibrated yet
		{
			eeprom_write_word(EEPROM_3V_ADR, battery_voltage);
			while (1)
			{
				stat_led_green();
				_delay_ms(100);
				stat_led_off();
				_delay_ms(100);
				stat_led_green();
				_delay_ms(100);
				stat_led_off();
				_delay_ms(100);
				stat_led_green();
				_delay_ms(100);
				stat_led_off();
				_delay_ms(500);
			}
		}
		 // 4V detection
		if ((battery_voltage >= (CAL_VOLT_HB*ADC_SAMPLES_V)) && (battery_voltage <= (CAL_VOLT_HT*ADC_SAMPLES_V)) && \
							(eeprom_voltage_h==0xffff)) // battery voltage in high borders and not calibrated yet
		{
			eeprom_write_word(EEPROM_4V_ADR, battery_voltage);
			while (1)
			{
				stat_led_green();
				_delay_ms(100);
				stat_led_off();
				_delay_ms(100);
				stat_led_green();
				_delay_ms(100);
				stat_led_off();
				_delay_ms(100);
				stat_led_green();
				_delay_ms(100);
				stat_led_off();
				_delay_ms(100);
				stat_led_green();
				_delay_ms(100);
				stat_led_off();
				_delay_ms(500);
			}
		}
		// battery voltage out of predefined borders
		else
		{
			while (1)
			{
				_delay_ms(200);
				stat_led_red();
				_delay_ms(200);
				stat_led_off();
			}
		}

	}
	else // calibriert!
	{
				while (1)
			{
				_delay_ms(200);
				stat_led_green();
				_delay_ms(200);
				stat_led_off();
			}

//=================== ende calibration ============================================================
#else // CALIBRATION

	else // calibriert
	{
	int8_t temp_d = CAL_TEMP - eeprom_temp; // calculate temperature offset

	volt_k = (4*1024000UL*ADC_SAMPLES_V)/eeprom_voltage_h; // steigung mit 1-punkt-cal bei 4V [1mV/1024]
//	uint16_t VOLT_D = 0;//CAL_VOLT_H_EXT - (voltage_h * VOLT_K); // Value to subtract from measurement to kill offset VOLT_D is x64

	// clear timers after startup
//	timer_clear_timer(TIMER_BALANCE);
uint8_t com_stat;
uint8_t state;
//DDRA |= 0x80;
	state = 0;


	while (1)
	{
		//--------------ADC------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
		timer_add_time(); // executed after max 32ms
//		stat_led_off();
//		_delay_ms(500);
//		stat_led_green();
//		manch_init_send();
//		manch_send();
//		_delay_ms(800);

		//============================Recieve Test==========================
		if(state==0)		//empfangen initialisieren
		{
			manch_init_receive();
			state++;
		}
		
		else if(state==1)		//Daten Empfangen
		{
			com_stat=manch_result();
			
			if (com_stat==0)		//während auf Daten gewartet wird LED orange blinken
			{
				stat_led_off();
			}
			
			else if(com_stat==1)			//wenn Daten erfolgreich Empfangen wurden LED grün
			{
				gl_manch_dat1 = gl_manch_dat;	// befehl weiter nach oben
				if (gl_manch_dat == REQ_VOLT_G) // spannungs anfrage
				{
					stat_led_green();
					gl_manch_dat = battery_voltage; //antwort spannung in 0.1mV
					state=2;
				}
				else if (gl_manch_dat == REQ_TEMP_G) // temperatur-anfrage
				{
					stat_led_green();
					gl_manch_dat = battery_temperature; //antwort temperatur in [K]
					state=2;
				}
				else if (gl_manch_dat == COM_BLC_OFF_G) // temperatur-anfrage
				{
					stat_led_green();
					BALANCE_time = 0;
					state=2;
				}
				else if ( (gl_manch_dat&0xff00) == COM_BLC_A) // balancing ein
				{
					state=2;
					stat_led_green();
					if ( (gl_manch_dat&0x00ff) == 1) // slave adressier!
					{
						gl_manch_dat1 = 0; // ungültiges command senden, => nächster leitet nix weiter
						BALANCE_time = 1;
						timer_clear_timer(TIMER_BALANCE);
					}
					else
						gl_manch_dat1 = gl_manch_dat -1;
				}				
				else // kein bekannter befehl, nix weitersenden, auf befehl warten
				{
					stat_led_red();
					state = 0;
				}
					
				timer_clear_timer(MAIN); //_delay_ms(20);
			}
			else if (com_stat==2)		//wenn Fehler beim Empfangen LED rot
			{
				stat_led_red();
				_delay_ms(100);
				state=0;
			}
			else if (com_stat==3)		//wenn Fehler beim Empfangen LED rot
			{
				stat_led_red();
				_delay_ms(100);
				stat_led_off();
				_delay_ms(100);
				stat_led_red();
				_delay_ms(100);
				state=0;
			}
			else if (com_stat==4)		//wenn Fehler beim Empfangen LED rot
			{
				stat_led_red();
				_delay_ms(100);
				stat_led_off();
				_delay_ms(100);
				stat_led_red();
				_delay_ms(100);
				stat_led_off();
				_delay_ms(100);
				stat_led_red();
				_delay_ms(100);
				state=0;
			}
		}
		else if (state==2)  // kurze pause bis zum antworten
		{
			if (timer_get_timer(MAIN) >= 7)
				state = 3; 
		}
		else if (state==3)	// antworten
		{
			manch_init_send();
			state = 4;
		}
		else if (state==4)	// warten, bis fertig gesendet
		{
			com_stat=manch_result();
			if (com_stat == 1)
			{
				//_delay_ms(3);
				state = 5;
			}
		}
		else if (state==5) // von oben empfangen
		{
			timer_clear_timer(MAIN);
			manch_init_receive1();
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
				stat_led_green();
				state=7;
			}
			else if (com_stat==2)		//wenn Fehler beim Empfangen LED rot
			{
				stat_led_red();
				_delay_ms(100); //???
				state=0;
			}
			
			if (timer_get_timer(MAIN) >= 15) // time out, kommt nix von oben
				state = 0;
		}
		else if (state==7)	// antworten mit daten von oben
		{
			manch_init_send();
			state = 8;
		}
		else if (state==8)	// warten, bis fertig gesendet
		{
			com_stat=manch_result();
			if (com_stat == 1)
			{
				state = 5;
			}
		}

//		BALANCE_time = timer_get_timer(TIMER_BALANCE);
		if (BALANCE_time)
		{
			if (timer_get_timer(TIMER_BALANCE) < 50000)
			{
				stat_led_orange();
				START_BALANCING();
			}
			else
				BALANCE_time = 0;
			
		}
		else
		{
			STOP_BALANCING();
		}

// ADC; Messen der Spannung und Temperatur
		if (!ADCstat)
		{
			volt_raw = measure_voltage();
			if (volt_raw) // make sure conversion is done
			{
				battery_voltage = (volt_k * volt_raw)/32768UL;//(1024*ADC_SAMPLES_V);
				//battery_voltage = (volt_raw/ADC_SAMPLES_V) * volt_k ;//   /10; /10 geht nicht??
				ADCstat = MEASURE_TEMP;
			}
		}
		else
		{
			volt_raw = measure_temperature();
			if (volt_raw) // make sure conversion is done
			{
				battery_temperature = volt_raw/ADC_SAMPLES_T + temp_d; // T in [K];
				ADCstat = MEASURE_VOLT;
			}
		}

	} // main-loop
#endif CALIBRATION
	}
}

void bms_slave_init() // Combining all init functions
{
	// CPU frequency settings.
#if F_CPU == 4000000L
	CLKPR = 0x80;
	CLKPR = 0x01;

#elif F_CPU == 2000000L
	CLKPR = 0x80;
	CLKPR = 0x02;

#elif F_CPU == 1000000L
	CLKPR = 0x80;
	CLKPR = 0x04;

#else
#error Invalid prescaler setting.
#endif
	timer_init_timer();
//	timer_add_time();
	ADC_init();
	stat_led_init(); // Status LED initialised
	BALANCING_DDR |= (1 << BALANCING_PIN);
	sei(); // global interrupt enable
}
