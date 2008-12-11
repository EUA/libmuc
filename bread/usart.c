#include <stdio.h>
#include <avr/io.h>
#include <avr/power.h>
#include "defines.h"
#include "usart.h"

static int usart0_putchar(char c, FILE *stream);
static FILE usart0_stdout = FDEV_SETUP_STREAM(usart0_putchar, NULL,_FDEV_SETUP_WRITE);


/*
static int usart_getchar(char c, FILE *stream) {
	return 1;
}
*/

static int usart0_putchar(char c, FILE *stream) {

  if (c == '\n')
	usart0_putchar('\r', stream);
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
  return 0;
}


void usart0_init(void) {
	power_usart0_enable();

	/* Set baud rate (12bit) */
	#define BAUD 19200
	#include <util/setbaud.h>
	UBRR0 = UBRR_VALUE;
	#if USE_2X
	#warning "U2X0 enabled"
	UCSR0A |=  (1 << U2X0);
	#else
	UCSR0A &= ~(1 << U2X0);
	#endif
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)	;
	/* Enable r/t interupts, hangles input when used with some buffering functions */
	//UCSR0B =|(1<<RXCIE0)|(1<<TXCIE0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (0<<USBS0)|(1<<UCSZ00)|(1<<UCSZ01);
	
	stdout=&usart0_stdout;
}

