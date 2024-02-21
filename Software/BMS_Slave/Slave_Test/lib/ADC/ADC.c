/*********************************************/
/*  file:   ADC.c                            */
/*						                     */
/*  Diploma Thesis:                          */
/*   Battery Management System 2021/22       */
/* 						                     */
/*  brief:  Functions for analog measurement */
/* 						                     */
/*  Author: Simon Ball                       */
/*********************************************/

#include <avr/io.h>
#include <stdint.h>
#include "ADC.h"
#include "manch_m.h"

static uint8_t adc_state = ST_REGISTER;
static uint16_t adc_values;
static uint8_t adc_counter;
static uint16_t adc_value = 0;
static uint16_t sort; // sort algorithm

void ADC_init()
{
	ADCSRA = 0x84; // ADC enabled // Clock Prescaler of 16

	// Result is right adjusted

	//  Single Conversion mode
	//  ADATE is not enabled, which means that the ADC is in Single Conversion Mode.
	//  By setting the ADSC bit(ADC Start Conversion) the conversion is started.
	//  Once the conversion is done, ADSC is cleared and the ADIF flag will be set.
	//  When its completed the channel can safely be changed. The next conversion takes 25 clock cycles.
	//  ADIE is not set as ADIF gets set when the conversion is done
	//  ADIF must be written to 1 in order to clear it
}

//===============================================================================
uint16_t measure_temperature()
{
	uint16_t temperature = 0;

	switch (adc_state)
	{
	case ST_REGISTER:
		ADMUX = 0xDF;  // Internal Reference Voltage 1.1V // Attaching Channel 11 to the ADC... Temperature
		ADCSRB = 0x08; // clearing REFS2, mux 5 bit set
		adc_state = ST_MEASURE;
		adc_counter = 0;
		adc_values = 0;
		ADC_START_CONVERSION();
		break;
		
	case ST_MEASURE:
		if (ADC_INTERRUPT)
		{
			ADC_CLEAR_INT();
			if (adc_counter == 0) // 1. messung nach umschalten verwerfen!
			{
				adc_counter++;
				ADC_START_CONVERSION();
			}
			else if (adc_counter < ADC_SAMPLES_T+1)
			{
				adc_values += ADC;
				adc_counter++;
				ADC_START_CONVERSION();
			}
			else
			{
				temperature = adc_values;
				adc_state = ST_REGISTER;
			}
		}
		break;		
	}
	return temperature;
}

//=====================================================================================
uint16_t measure_voltage()
{
	adc_value = 0;

	switch (adc_state)
	{
	case ST_REGISTER:
		ADMUX = 0x86;  // Internal Reference Voltage 2.56V, ADC attached to Channel 6 aka PA7
		ADCSRB = 0x10; // Internal Reference Voltage 2.56V
		adc_state = ST_MEASURE;
		adc_counter = 0;
		adc_values = 0;
		ADC_START_CONVERSION();
		break;
	case ST_MEASURE:
		if (ADC_INTERRUPT)
		{
			ADC_CLEAR_INT();
			if (adc_counter == 0) // 1. messung nach umschalten verwerfen!
			{
				adc_counter++;
				ADC_START_CONVERSION();
			}
			else if (adc_counter < ADC_SAMPLES_V+1)
			{
				adc_values += ADC;
				adc_counter++;
				ADC_START_CONVERSION();
			}
			else
			{
				adc_state = ST_FILTER;
			}
		}
		break;
	case ST_FILTER:
//PINA=0x80;
		#if ADC_FILTER_V == 1 // filters out the greatest and the smallest value measured for higher precision
			// shifting the greatest value to the right
/*			for (adc_counter = 0; adc_counter <= conversions; adc_counter++)
			{
				if (adc_values[adc_counter + 1] < adc_values[adc_counter])
				{
					sort = adc_values[adc_counter + 1];
					adc_values[adc_counter + 1] = adc_values[adc_counter];
					adc_values[adc_counter] = sort;
				}
			}

			// shifting the lowest value to the left
			for (adc_counter = conversions; adc_counter > 0; adc_counter--)
			{
				if (adc_values[adc_counter] < adc_values[adc_counter - 1])
				{
					sort = adc_values[adc_counter - 1];
					adc_values[adc_counter - 1] = adc_values[adc_counter];
					adc_values[adc_counter] = sort;
				}
			}

			// Adding all measured values to variable, except the outer ones
			adc_value = 0; // Resetting variable
			for (adc_counter = 1; adc_counter < (conversions - 1); adc_counter++)
				adc_value += adc_values[adc_counter];
			adc_value /= (conversions - 2);
*/
		#else
			// Adding all measured values to variable
//			adc_value = 0; // Resetting variable
//			for (adc_counter = 1; adc_counter < ADC_SAMPLES_V; adc_counter++)
//				adc_value += adc_values[adc_counter];
			adc_value = adc_values; //(ADC_SAMPLES_V);
		#endif
		// voltage = (float)adc_value / 400; //divided by 1024 aka 10-bit, multiplied by 2,56 aka internal reference voltage
			adc_state = ST_REGISTER;
		break;
	}
	return adc_value;
}
