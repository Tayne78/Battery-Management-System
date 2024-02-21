/***************************/
/*         status.h        */
/*          Status         */
/*						   */
/*    Battery Management   */
/*        System           */
/* 						   */
/* Author: Tristan Horvath */
/***************************/
#ifndef STATUS_H
#define STATUS_H

#ifdef __AVR_ATtiny261__
#define LEDDDR  DDRA
#define LEDPORT PORTA
#define LEDPINR PINA5
#define LEDPING PINA6
//BALANCING-------------------------------------------------------//
#define BALANCING_DDR DDRB
#define BALANCING_PORT PORTB
#define BALANCING_PIN PINB4
#define START_BALANCING() BALANCING_PORT |= (1 << BALANCING_PIN)
#define STOP_BALANCING() BALANCING_PORT &= ~(1 << BALANCING_PIN)
#endif

#ifdef __AVR_ATmega32U4__
#define LEDDDR  DDRD
#define LEDPORT PORTD
#define LEDPINR PIND6
#define LEDPING PIND7
#define SSRDDR DDRF
#define SSRPORT PORTF
#define SSRPIN PINF5
#define RELAYDDR DDRF
#define RELAYPORT PORTF
#define RELAYPIN PINF6
#endif

//Status LED
void stat_led_init();
//initialize status led
void stat_led_red();
//make status LED light up RED
void stat_led_green();
//make status LED light up GREEN
void stat_led_orange();
//make status LED light up ORANGE
void stat_led_off();
//turn off status LED
void stat_led_toggle_green();
//toggle green status LED
void stat_led_toggle_red();
//toggle red status LED

#ifdef __AVR_ATmega32U4__
//Status Relay
void stat_rel_init();
//initialize status relay
void stat_rel_on();
//switch status relay on
void stat_rel_off();
//switch status relay off

//Status solid state relais
void stat_ssr_init();
//initialize status solid state relais
void stat_ssr_on();
//switch status solid state relais on
void stat_ssr_off();
//switch status solid state relais off
#endif
#endif  //STAUTS_H
