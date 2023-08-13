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

#define MANCH_M
#include <avr/io.h>
#include "avr/interrupt.h"
//#include <util/delay.h>
#include <compat/deprecated.h>
#include "manch_m.h"
#include "status.h"

static uint8_t manch_x; // manch_x: differs betwenn long and short 
static uint8_t manch_nr; // welche schnittstelle empfängt?

//=============================================================================
void manch_init_send(void)
{
   manch_res = 0;
   manch_i = 0;
   manch_d = (uint8_t)(gl_manch_dat >> 8) | 0x80; // msb als startbit immer 1
   manch_d1 = (uint8_t)(gl_manch_dat & 0x00ff);
   DDRMANCH |= PN_MANCH_SEND; // pin als ausgang
   CLRMANCH;                  // und auf 0
#ifdef MANCHESTER1
   manch1_d = (uint8_t)(gl_manch_dat1 >> 8) | 0x80; // msb als startbit immer 1
   manch1_d1 = (uint8_t)(gl_manch_dat1 & 0x00ff);
   DDRMANCH1 |= PN_MANCH1_SEND; // pin als ausgang
   SETMANCH1;              // und auf 1
#endif
#ifdef __AVR_ATmega32U4__
   TCCR1A = 0x02; // mode 14, fast pwm, top is icr1
   TCCR1B = 0x18; //  - " -
   OCR1B = F_CPU / BAUDRATE / CLOCK_PR / 2 +10;
   ICR1 = F_CPU / BAUDRATE / CLOCK_PR; // Bitdauern
   TIFR1 = 0xff;                // flags löschen		fx
   TIMSK1 = 0x05;               // ocr1b match und overflow interrupt;
#endif                          //__AVR_ATmega32u4
#ifdef __AVR_ATtiny261A__
   // ist default TCCR1A = 0x00; // normal mode
   OCR1B = F_CPU / BAUDRATE / CLOCK_PR / 2;
   OCR1C = F_CPU / BAUDRATE / CLOCK_PR; // Bitdauer
   TIFR = 0xFF;     // Flags cleared, erst nach synchronization clock cycle!!
   TC1H = 0;	// 10-bit register!
#endif
   TCNT1 = F_CPU / BAUDRATE / CLOCK_PR - 20; 
#ifdef __AVR_ATtiny261A__
   TIMSK = 0x24;                            // ocr1b match und overflow interrupt
#endif
   TCCR1B |= TCCR1B_TIMER_START;
}

//============================================================================
void manch_init_rec_all()
{
   manch_i = 0;
   manch_res = 0;
#ifdef __AVR_ATtiny261A__   
   // default! TCCR1A = 0x00;          // normal mode, top is ocr1c
   TC1H = 0;	// 10-bit register!
   TCNT1 = 0;
   OCR1A = 2 * F_CPU / BAUDRATE / CLOCK_PR; // timeout => bit zu lang
   OCR1C = 0xFF;				// top-wert
   TIFR = 0xFF; 				// flags löschen
   TIMSK = 0x40;           // ocr1a match interrupt;
   GIMSK = 0x20;    
#endif //__AVR_ATtiny261A__    
}

//===========================================================================	
void manch_init_receive()
{
#ifdef __AVR_ATmega32U4__
   DDRMANCH &= ~PN_MANCH_REC; // pin als Eingang
   CLRMANCH;                  // no pull-up
   // timer OCRA für receive error, timeout, wenn keine flanke kommt
   TCCR1A = 0x00;                                // mode 12, fast pwm, top is icr1
   TCCR1B = 0x18;                                //  - " -
   TCNT1 = 0;
   OCR1A = 2 * F_CPU / BAUDRATE / CLOCK_PR + 18; // timeout => bit zu lang
   ICR1 = 0xffff;  // top-wert
   TIFR1 = 0xff;   // flags löschen
   PCIFR = 0xff;	// flags löschen
   TIMSK1 = 0x02;          // ocr1a match interrupt;
   PCICR = 0x01;           // flankeninterrupt
   PCMSK0 = PN_MANCH_REC; // enable pcint on receive pin
#endif                     //__AVR_ATmega32u4
#ifdef __AVR_ATtiny261A__
   DDRMANCH &= ~PN_MANCH_REC; // pin als Eingang
   CLRMANCH;                  // no pull-up
   PCMSK1 = PN_MANCH_REC; // enable pcint on receive pin
   PCMSK0 = 0;		// defaultmäßig nicht auf 0!!
   manch_nr = 0; 
#endif                     //__AVR_ATtiny261A__
	manch_init_rec_all();
}


//====================================================================
uint8_t manch_result()
{
	return manch_res;
}

//====================================================================
manch_stop_receive()
{
               TCCR1B =0; // timer stoppen

#ifdef __AVR_ATmega32U4__
               TIMSK1 = 0x00; // overflow interrupt stoppen
               PCICR = 0x00;  // flankeninterrupt stoppen
#endif                        //__AVR_ATmega32u4
#ifdef __AVR_ATtiny261A__
               TIMSK = 0x00;            // overflow interrupt stoppen
               GIMSK = 0;  // flankeninterrupt stoppen
               PCMSK1 = 0; // enable pcint on receive pin
               PCMSK0 = 0; // - " -
#endif                                  //__AVR_ATtiny261A__
	gl_manch_dat = (manch_d<<8) + manch_d1;
}

//======================================================================
// fürs empfangen, rst: ISR_NAKED!
#ifdef __AVR_ATmega32U4__
ISR(PCINT0_vect)
{
   uint16_t tim;
#endif //__AVR_ATmega32u4
#ifdef __AVR_ATtiny261A__
ISR(PCINT_vect, ISR_NAKED)
{
asm volatile (	"push r24" "\n\t"
					"in r24,__SREG__" "\n\t"
					"push r24" "\n\t"
					"push r18" "\n\t"
					"push r25" "\n\t");
	uint8_t tim;
#endif //__AVR_ATtiny261A__

// timer einlesen und neu starten, für zeitmessung
	tim = TCNT1;
#ifdef __AVR_ATtiny261A__
	TC1H = 0;	// 10-bit register!
#endif
	TCNT1 = 0; // reset timer

#ifdef __AVR_ATtiny261A__
// eingangspin einlesen
	if ( (manch_nr&0x01) == 0)
	{
		if ((PINMANCH&PN_MANCH_REC) == PN_MANCH_REC)
			manch_bit = manch_bit|0x01; // comp.optimierung
		else	
		{
			manch_bit = 0;
//PINA=0x80;		//fx
		}
	}
  #ifdef MANCHESTER1
	else // schnittstelle von oben invertiert!
	{
		if ((PINMANCH1&PN_MANCH1_REC) == PN_MANCH1_REC)
			manch_bit = 0; // comp.optimierung
		else	
			manch_bit = manch_bit|0x01; // comp.optimierung
	}
  #endif //MANCHESTER1
//PINA=0x80;		//fx
#endif //__AVR_ATtiny261A__
#ifdef __AVR_ATmega32U4__
//	if ((PINMANCH&PN_MANCH_REC) == PN_MANCH_REC)
//		manch_bit = manch_bit|0x01; // comp.optimierung
//	else	
//		manch_bit = 0;
//PINB|=(1<<PINB3);
#endif 

   if (manch_i == 0) // anfang
   {
//   	stat_led_green();
#ifdef __AVR_ATmega32U4__
      if (READMANCH == 0) // 1 als startbit, neg. logik! fx
#endif //__AVR_ATmega32u4
#ifdef __AVR_ATtiny261A__
      if (manch_bit&0x01 == 1) // 1 als startbit, neg. logik! compiler-optimiert
#endif 
      {
//			PORTD^=(1<<PIND3);		//fx
         TCCR1B |= TCCR1B_TIMER_START; // timer starten
         manch_x = 'k';
         manch_i++;
      }
#ifdef __AVR_ATtiny261A__
//PINA=0x80;
#endif
   }
   else
   {
//flanke nach halben bit:
      if ((F_CPU / BAUDRATE / CLOCK_PR ) / 3 < tim && tim < (F_CPU / BAUDRATE / CLOCK_PR * 2) / 3) //
      {
//PINB|=0x08;
         if (manch_x == 'l') // weiter
            manch_x = 'k';
         else // bit einlesen!
         {
            manch_d1 = manch_d1 << 1;
            manch_x = 'l';      // nächstes mal einlesen nach ganzem bit
#ifdef __AVR_ATmega32U4__
            if (READMANCH == 0) // vergleich mit 1 geht nicht!!!
#endif //__AVR_ATmega32u4
#ifdef __AVR_ATtiny261A__
            if (manch_bit&0x01 == 1) //comp. optimierung, vgl mit 0 geht nicht!!!
#endif
            {
            	manch_d1 &= 0xFE;
//PINB|=0x8;
            }
            else
            {
            	manch_d1 |= 0x01;
//PINB|=0x8;//PINA=0x80;		//fx
            }
            manch_i++;
            if (manch_i == 9) // 1. byte fertig
            {
               manch_d = manch_d1;
            }
            else if (manch_i == 17) // fertig, empfang stoppen
            {
            	manch_stop_receive();
               manch_res = 1;
            }
         }
      }
// flanke nach ganzem bit
      else if ((F_CPU / BAUDRATE / CLOCK_PR * 2) / 3 < tim && tim < (F_CPU / BAUDRATE / CLOCK_PR * 4) / 3) //
      {
//PINB|=0x8;
         if (manch_x == 'l') // bit einlesen
         {
            manch_d1 = manch_d1 << 1;
#ifdef __AVR_ATmega32U4__
            if (READMANCH == 0)
#endif //__AVR_ATmega32u4
#ifdef __AVR_ATtiny261A__
            if (manch_bit&0x01 == 1) //comp. optimierung
#endif
            {
            	manch_d1 &= 0xFE;
//PINB|=0x8;
            }
            else
            {
            	manch_d1 |= 0x01;
//PINB|=0x8;//PINA=0x80;		//fx
            }
            manch_i++;
            if (manch_i == 9) // 1. byte fertig
            {
               manch_d = manch_d1;
            }
            else if (manch_i == 17) // fertig, empfang stoppen
            {
					manch_stop_receive();
               manch_res = 1;
            }
         }
         else // es hätte nur ein kurzer sein sollen! neu beginnen
         {
         	manch_stop_receive();
            manch_res = 2;
         }
      }
      else // fehler, flanke nicht zum richtigen zeitpunkt! neu beginnen
      {
      	manch_stop_receive();
			manch_res = 3;
      }
   }
#ifdef __AVR_ATtiny261A__
asm volatile (	"pop r25" "\n\t"
					"pop r18" "\n\t"
					"pop r24" "\n\t"
					"out __SREG__, r24" "\n\t"
					"pop r24" "\n\t"
					"reti");
#endif
}

//==========================================================================
ISR(TIMER1_COMPA_vect, ISR_NAKED) // timeout, eine erwartete flanke ist nicht gekommen
// gilt für beide empfänger!!
{
//PORTA ^=0x04;
	asm volatile ("push r24" "\n\t");
	manch_stop_receive();
//	manch_res = 4;
	asm volatile ("ldi r24,lo8(4)" "\n\t"
 					"mov r3,r24"  "\n\t"
					"pop r24" "\n\t"	
						"reti");
}

//===========================================================================
// fürs senden
ISR(TIMER1_OVF_vect)
{
//stat_led_green();
   if (manch_i == 16) // ende
   {
      CLRMANCH;
#ifdef MANCHESTER1
      SETMANCH1;
#endif // MANCHESTER1
	   TCCR1B = 0; // timer stoppen
#ifdef __AVR_ATmega32U4__
	   TIMSK1 = 0x00; // overflow interrupt stoppen
#endif
#ifdef __AVR_ATtiny261A__
		TIMSK = 0x00; // overflow interrupt stoppen
#endif
	   manch_res = 1;
   }
   else
   {
      if (manch_i == 8) // 2. byte
      {
         manch_d = manch_d1;
#ifdef MANCHESTER1
         manch1_d = manch1_d1;
#endif // MANCHESTER1
      }
      if (manch_d & 0x80)
         SETMANCH;
      else
         CLRMANCH;
       manch_d = manch_d << 1;
#ifdef MANCHESTER1
      if (manch1_d & 0x80)
         CLRMANCH1;
      else
         SETMANCH1;
      manch1_d = manch1_d << 1;
#endif // MANCHESTER1
      manch_i++;
   }
//stat_led_off();
}

//================================================================
ISR(TIMER1_COMPB_vect, ISR_NAKED)
{
//stat_led_red();
   TOGMANCH; // pin toggeln
#ifdef MANCHESTER1
   // ein paar nop's!
   TOGMANCH1;
#endif // MANCHESTER1
//stat_led_off();
asm volatile("reti");
}

#ifdef MANCHESTER1

void manch_init_receive1()
{
#ifdef __AVR_ATmega32U4__
	error! noch nicht implementiert!
#endif                     //__AVR_ATmega32u4
#ifdef __AVR_ATtiny261A__
   DDRMANCH1 &= ~PN_MANCH1_REC; // pin als Eingang
   CLRMANCH1;                  // no pull-up
   PCMSK0 = PN_MANCH1_REC; // enable pcint on receive pin
   PCMSK1 = 0;		// defaultmäßig nicht auf 0!!
   manch_nr = 1; 
#endif                     //__AVR_ATtiny261A__
	manch_init_rec_all();
}


#endif // MANCHESTER1

