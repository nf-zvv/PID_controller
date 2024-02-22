#ifndef F_CPU
#define F_CPU 16000000UL 
#endif
 
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <ds18b20/ds18b20.h>
#include <simplePID.h>
#include "lcd.h"
#include "adc.h"

static PidController pi_controller;
static PidNewCoefficients pid_coeff;

const char title[] PROGMEM = "PID controller";

volatile struct 
{
    unsigned update: 1;
} flags;


// Прерывание таймера Т0
ISR(TIMER0_COMP_vect)
{
	// сбрасываем счетчик
	TCNT0 = 0;
	// Возводим флаг 
    flags.update = 1;
}

void Timer0_Init(void)
{
	// Период срабатывания прерывания таймера 2 мс (500 Гц)
	// 31.25 = 2 ms /(1000/(F_CPU/1024))
	OCR1B = 31;
	// Разрешаем прерывание таймера по совпадению
	TIMSK |= (1 << OCIE0);
	// Предделитель 1024
	TCCR0 = (5 << CS00);
}

iq8_t StateEstimator(void) {
    //TODO: fill here your sensors readings:
    //char result[4];
    //lcd_set_cursor(0,1);
    //utoa(adc_read(0), result, 10);
    //lcd_puts(result);
    return FLOAT_TO_Q8(0.5f);
}

void Command(iq8_t command) {
    //TODO: actuate on plant, eg.: PWM:
    (void) command;
}

int main(void) {
	// Инициализация ЖКИ 1602
	lcd_init();
	lcd_clear();
	lcd_return_home();
	
    lcd_puts_P(title);

    // Инициализация АЦП
    ADC_Init();
    // Инициализация таймера Т0
    // Срабатывание прерывания таймера каждые 2 мс (500 Гц)
    Timer0_Init();
    sei(); // глобальное разрешение прерываний
	
    //Make a PI controller:
    pid_coeff.new_kp = FLOAT_TO_Q8(1.0f);
    pid_coeff.new_ki = FLOAT_TO_Q8(0.1f);
    pid_coeff.new_kd = FLOAT_TO_Q8(1.0f);

    //FIRST: initialize your PID controller:
    PidControllerInit(&pi_controller,
                      &pid_coeff);

    int temp;
    char result[4];
    while(1) {

        //Start conversion (without ROM matching)
		ds18b20convert( &PORTB, &DDRB, &PINB, ( 1 << 0 ), NULL );

		//Delay (sensor needs time to perform conversion)
		_delay_ms(750);

		//Read temperature (without ROM matching)
		ds18b20read( &PORTB, &DDRB, &PINB, ( 1 << 0 ), NULL, &temp );

        lcd_set_cursor(0,1);
        itoa(temp, result, 10);
        lcd_puts(result);

        if (flags.update) {
            cli();
            flags.update = 0;
            sei();
            
        }
        _delay_ms(2); //Simulate a plant running at 500Hz:

        //SECOND: Read sensor and compute next command:
        iq8_t command = PidControllerUpdate(&pi_controller, FLOAT_TO_Q8(0.5), StateEstimator());
        //THIRD: update actuator:
        Command(command);
    }
    return 0;
}