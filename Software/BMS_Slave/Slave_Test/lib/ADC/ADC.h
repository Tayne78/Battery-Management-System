/*********************************************/
/*  file:   ADC.h                            */
/*						                     */
/*  Diploma Thesis:                          */
/*   Battery Management System 2021/22       */
/* 						                     */
/*  brief:  Functions for analog measurement */
/* 						                     */
/*  Author: Simon Ball                       */
/*********************************************/

#ifndef ADC_H
#define ADC_H
// Averaging samples, 64 is max
#define ADC_SAMPLES_V 32 
#define ADC_SAMPLES_T 32

// Setting the filtering for voltage (0: OFF    1: ON)
#define ADC_FILTER_V 0

#define EEPROM_3V_ADR (uint16_t *)0x01   // Address of 3v raw value
#define EEPROM_4V_ADR (uint16_t *)0x03   // Address of 4v raw value
#define EEPROM_temp_ADR (uint16_t *)0x05 // Address of ADC temperature offset
#define EEPROM_debug_ADR (uint16_t *)0x07 // zum debuggen

// Callibration
#define CAL_VOLT_HT 0x035C    //                              -> 4.3V
#define CAL_VOLT_H 0x0320     // Callibration Voltage HIGH    -> 4V
#define CAL_VOLT_H_EXT 0xC800 // Callibration Voltage HIGH    -> 4V
#define CAL_VOLT_HB 0x02E4    //                              -> 3.7V

#define CAL_VOLT_LT 0x0294    //                              -> 3.3V
#define CAL_VOLT_L 0x0258     // Callibration Voltage LOW     -> 3V
#define CAL_VOLT_L_EXT 0x9600 // Callibration Voltage LOW     -> 3V
#define CAL_VOLT_LB 0x021C    //                              -> 2.7V

#define CAL_TEMP 292 // Environment temperature during calibration, in [K]

#define EEPROM_STATUS_ADR (uint8_t *)0x00 // Address of the status byte
// Byte pointers to each bit in the eeprom status byte
#define EEPROM_CALIBRATED 0xCC
#define EEPROM_STATUS_L 0x03
#define EEPROM_STATUS_H 0x0C

//--------------MAKROS-----------------------------//
#define ADC_INTERRUPT ADCSRA &(1 << ADIF)            // ADC interrupt flag set?
#define ADC_START_CONVERSION() ADCSRA |= (1 << ADSC) // ADC start conversion
#define ADC_CLEAR_INT() ADCSRA |= (1 << ADIF)        // clear interrupt flag

// Giving Names to Numbers
enum ADC_STATES
{
    ST_REGISTER,
    ST_MEASURE,
    ST_FILTER
};

//--------------FUNCTION-DEKLARATIONS--------------//

// Function ADC_setup:
// setting all important bits of the ADC register
void ADC_init(void);

// Function measure_temperature:
// Returns the temperature in degree celsius after at least 3 calls
// ADC filtering can be enabled by defining FILTER as 1
// Filtering cuts lowest and highest value and averages the remaining ones.
// It's recommended to make at least 6 measurements when using ADC filtering
uint16_t measure_temperature();

// Function measure_voltage:
// Returns the voltage raw value after at least 3 calls
// ADC filtering can be enabled by defining FILTER as 1
// Filtering cuts lowest and highest value and averages the remaining ones.
// It's recommended to make at least 6 measurements when using ADC filtering
uint16_t measure_voltage(void);

#endif // ADC_H
