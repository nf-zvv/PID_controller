/* ADC
 *
 *
 */

#ifndef __ADC_H__
#define __ADC_H__

#include <avr/io.h>

void ADC_Init(void);

uint16_t adc_read(uint8_t ch);

#endif