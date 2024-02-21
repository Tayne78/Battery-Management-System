//===============================================================
//
//   Projekt sax-extender master
//
//   rst, jan 2022
//
//   Author: Reinhard Steindl
//
//   Update by: Simon Ball
//
//================================================================
#ifndef TIMER_H
#define TIMER_H

//  verwendet timer 0 und stellt funktionen fürs timing zur verfügung
#ifdef __AVR_ATmega32U4__
enum TIMER_NR {
//   TIMER_COMM,
//   TIMER_ADC,
//   TIMER_BALANCE,
	MAIN,
	CONTROL,
// letzte eintrag, neue timer davor!
   TIMER_MAX_TIMER};
#endif
#ifdef __AVR_ATtiny261A__
enum TIMER_NR {
//   TIMER_COMM,
//   TIMER_ADC,
   TIMER_BALANCE,
	MAIN,
   HEARTBEAT,
// letzte eintrag, neue timer davor!
   TIMER_MAX_TIMER};
#endif


void timer_clear_timer(uint8_t nr);
// setzt den timer mit der nummer nr zurück
uint16_t timer_get_timer(uint8_t nr);
// liefert die zeit des timers mit der nummer nr seit dem letzten zuruecksetzen in ms zurück
void timer_init_timer(void);
// am beginn zum initialisieren 1 mal aufrufen
void timer_add_time(void);
// muss in der main-loop regelmässig aufgerufen werden.
// max. zeit zwischen 2 aufrufe: 32ms
// liest hw-timer aus und addiert die timer

#endif //TIMER_H
