#ifndef F_CPU
#define F_CPU 16000000UL 
#endif
 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <simplePID.h>
#include "lcd.h"

static PidController pi_controller;
static PidNewCoefficients pid_coeff;

const char hello[] PROGMEM = "Hello world!";

iq8_t StateEstimator(void) {
    //TODO: fill here your sensors readings:
    return FLOAT_TO_Q8(0.5f);
}

void Command(iq8_t command) {
    //TODO: actuate on plant, eg.: PWM:
    (void) command;
}

int main(void) {
	// LCD 1602 initialize
	lcd_init();
	lcd_clear();
	lcd_return_home();
	
	lcd_puts("hello");
    lcd_set_cursor(0,1);
    lcd_puts_P(hello);
	
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