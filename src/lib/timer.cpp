#include <Arduino.h>

#include "include/timer.h"

void schedule(void);
void dispatch(void);
void end_cycle(void);

void reset_timer1_control_registers() {
    TCCR1A = 0;
    TCCR1B = 0;
}

void reset_timer1_counting_register() {
    TCNT1 = 0; // register that keeps count of the number of clock cycles
}

void set_timer1_ctc_mode() {
    TCCR1A &= ~(1 << WGM10);
    TCCR1A &= ~(1 << WGM11);
    TCCR1B |= (1 << WGM12);
}

// effective clock frequency = 16MHz / 256 = 62500Hz
void set_timer1_prescaler_256() {
    TCCR1B |= (1 << CS12);
    TCCR1B &= ~(1 << CS11);
    TCCR1B &= ~(1 << CS10);
}

void set_timer1_interrupt_on_compare_match_A() {
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 31;
}

// Assuming prescaler 256
void set_interrupt_period(float tick_delay) {
    OCR1A = (unsigned int)(tick_delay * 62.5);
}

void set_timer_registers() {
    reset_timer1_control_registers();

    reset_timer1_counting_register();

    set_timer1_ctc_mode();

    set_timer1_prescaler_256();

    set_timer1_interrupt_on_compare_match_A();

    set_interrupt_period(TICK_DURATION_MS);
}