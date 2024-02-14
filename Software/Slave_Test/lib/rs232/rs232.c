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

#define RS232
#include <avr/io.h>
#include "rs232.h"
#include "manch_m.h"

void RS232_init()
{
	UBRR1 = F_CPU / 16 / RS232_BAUDRATE -1;
	UCSR1B = (1<<TXEN1);
	fdevopen(uart_putc, 0, 0);
}


int uart_putc(unsigned char c)
{
	while (!(UCSR1A & (1<<UDRE1))); //Wait till transmission is possible
	UDR1 = c; /* send */
	return 0;
}

