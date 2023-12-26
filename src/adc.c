#include <avr/io.h>
#include "adc.h"

void ADC_Init(void)
{
	// REFS0 = 1 - выбор ИОН AVcc с конденсатором на AREF
    // REFS0 = 3 - внутренний ИОН 2,56 В
	// ADLAR = 1 - выравнивание результата по левому краю,
	// это значит что ADCH содержит биты 9..2, а ADCL - биты 1, 0
    // ADLAR = 0 - выравнивание результата по правому краю,
	// это значит что ADCH содержит биты 9, 8, а ADCL - биты 7..0
	ADMUX = (1 << REFS0);

	// ADTS0 = 0 - свободный запуск
	//       = 4 - запуск по переполнению Таймера/Счетчика0
	//       = 5 - запуск по совпадению Таймера/Счетчика1 B
	//       = 6 - запуск по переполнению Таймера/Счетчика1
	//ADCSRB = (0<<ADTS0);

    // Не поддерживается в ATmega16?
	// ADC3D = 1 - отключить цифровые входы, соответствующие входам ADC0,1,2,3
	//DIDR0 = (1<<ADC0D)|(1<<ADC1D)|(1<<ADC2D)|(1<<ADC3D);

	// ADEN  = 1 - включение АЦП
	// ADIE  = 1 - разрешение прерывания от АЦП
	// ADSC  = 1 - запуск преобразования
	// ADATE = 1 - автозапуск включен (непрерывные последовательные преобразования, одно за другим)
	// ADPS0 = 7 - предделитель тактовой частоты (CLK/128). Итого частота АЦП 125 кГц 
	ADCSRA = (1<<ADEN)|(7<<ADPS0);
}

// read adc value
uint16_t adc_read(uint8_t ch)
{
    // select the corresponding channel 0~7
    // ANDing with '7' will always keep the value
    // of 'ch' between 0 and 7
    ch &= 0b00000111;  // AND operation with 7
    ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
 
    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1<<ADSC);
 
    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while(ADCSRA & (1<<ADSC));
 
    return (ADC);
}
