#include "uart.h"


void UART_Init(void)
{
	#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega1284P__)
	// baud rate
	UBRR0H = HI(bauddivider);
	UBRR0L = LO(bauddivider);

	UCSR0A = 0x00;
	// разрешаем прием и передачу
	UCSR0B = (1 << TXEN0)|(1 << RXEN0);
	// 8-битовая посылка
	UCSR0C = (1 << UCSZ00)|(1 << UCSZ01);
	#elif defined(__AVR_ATmega8__) || defined(__AVR_ATmega16A__) || defined(__AVR_ATmega16__)
	// baud rate
	UBRRH = HI(bauddivider);
	UBRRL = LO(bauddivider);

	UCSRA = 0x00;
	// разрешаем прием и передачу
	UCSRB = (1 << TXEN)|(1 << RXEN);
	// 8-битовая посылка
	UCSRC = (1 << UCSZ0)|(1 << UCSZ1);
	#else
	#error "Unsupported part:" __PART_NAME__
	#endif // part specific code
}

void send_UART(unsigned char c)
{
	#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega1284P__)
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
	#elif defined(__AVR_ATmega8__) || defined(__AVR_ATmega16A__) || defined(__AVR_ATmega16__)
	while (!(UCSRA & (1 << UDRE)));
	UDR = c;
	#else
	#error "Unsupported part:" __PART_NAME__
	#endif // part specific code
}

void send_UART_str(unsigned char *s)
{
	while (*s != 0x00)
		send_UART(*s++);
}

void send_UART_str_P(const unsigned char *s)
{
	while (pgm_read_byte(s) != 0x00)
		send_UART(pgm_read_byte(s++));
}
