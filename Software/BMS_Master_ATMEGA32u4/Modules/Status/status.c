/***************************/
/*         status.c        */
/*          Status         */
/*						   */
/*    Battery Management   */
/*        System           */
/* 						   */
/* Author: Tristan Horvath */
/***************************/

#include <avr/io.h>
#include "status.h"
#include "manch_m.h"

//Status LED
void stat_led_init(){
    LEDDDR|=(1<<LEDPINR)|(1<<LEDPING);
}
void stat_led_red(){
    LEDPORT|=(1<<LEDPINR);
    LEDPORT&=~(1<<LEDPING);
}
void stat_led_green(){
    LEDPORT|=(1<<LEDPING);
    LEDPORT&=~(1<<LEDPINR);
}
void stat_led_orange(){
    LEDPORT|=(1<<LEDPING);
    LEDPORT|=(1<<LEDPINR);
}
void stat_led_off(){
    LEDPORT&=~(1<<LEDPING);
    LEDPORT&=~(1<<LEDPINR);
}
void stat_led_toggle_green(){
    LEDPORT^=(1<<LEDPING);
}
void stat_led_toggle_red(){
    LEDPORT^=(1<<LEDPINR);
}

#ifdef __AVR_ATmega32U4__
//Status Relay
void stat_rel_init()
{
    RELAYDDR|=(1<<RELAYPIN);
}
void stat_rel_on(){
    RELAYPORT|=(1<<RELAYPIN);
}
void stat_rel_off(){
    RELAYPORT&=~(1<<RELAYPIN);
}

//Status solid state relais
void stat_ssr_init()
{
    SSRDDR|=(1<<SSRPIN);
}
void stat_ssr_on(){
    SSRPORT|=(1<<SSRPIN);
}
void stat_ssr_off(){
    SSRPORT&=~(1<<SSRPIN);
}
#endif
