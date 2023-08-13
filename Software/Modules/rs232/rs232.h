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
#ifndef RS232_H
#define RS232_H

#define RS232_BAUDRATE 9600  

void RS232_init(void);
int uart_putc(unsigned char c);

//==========globale variablen================================
#ifdef RS232
	#define EXTERN
#else
	#define EXTERN extern
#endif 


#undef EXTERN

#endif // MANCH_H
