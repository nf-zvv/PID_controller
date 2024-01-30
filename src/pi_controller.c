#ifndef F_CPU
#define F_CPU 16000000UL 
#endif
 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <simplePID.h>
#include "lcd.h"
#include "adc.h"
#include "uart.h"

static PidController pi_controller;
static PidNewCoefficients pid_coeff;

const char title[] PROGMEM = "PID controller";
const unsigned char uart_start[] PROGMEM = "PID controller started\r\n";

// Прерывание таймера Т0
ISR(TIMER0_COMP_vect)
{
	// сбрасываем счетчик
	TCNT0 = 0;
	// Возводим флаг 

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
    char result[4];
    lcd_set_cursor(0,1);
    utoa(adc_read(0), result, 10);
    lcd_puts(result);
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

    UART_Init();

    send_UART_str_P(uart_start);
	
    //Make a PI controller:
    pid_coeff.new_kp = FLOAT_TO_Q8(1.0f);
    pid_coeff.new_ki = FLOAT_TO_Q8(0.1f);
    pid_coeff.new_kd = FLOAT_TO_Q8(1.0f);

    //FIRST: initialize your PID controller:
    PidControllerInit(&pi_controller,
                      &pid_coeff);

    while(1) {
        _delay_ms(2); //Simulate a plant running at 500Hz:

        //SECOND: Read sensor and compute next command:
        iq8_t command = PidControllerUpdate(&pi_controller, FLOAT_TO_Q8(0.5), StateEstimator());
        //THIRD: update actuator:
        Command(command);
    }
    return 0;
}