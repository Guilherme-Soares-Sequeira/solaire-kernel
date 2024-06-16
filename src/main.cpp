#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

#include "include/remaps.h"
#include "include/task.h"
#include "include/util.h"
#include "include/context.h"
#include "include/timer.h"
#include "include/kernel.h"

extern "C" uint16_t times[TIME_SIZE];
extern "C" uint8_t time_counter;

#define TASK1_NAME "T1"
#define TASK2_NAME "T2"
#define TASK3_NAME "T3"

#define TASK1_PERIOD 250.0
#define TASK2_PERIOD 500.0
#define TASK3_PERIOD 1000.0

#define TASK1_CRIT TASK_CRIT_HARD
#define TASK2_CRIT TASK_CRIT_HARD
#define TASK3_CRIT TASK_CRIT_HARD

#define TASK1_WCET 25.0
#define TASK2_WCET 25.0
#define TASK3_WCET 25.0

void t1(void) { while (TRUE) { toggle_led(LED2); end_cycle(); } }
void t2(void) { while (TRUE) { toggle_led(LED3); end_cycle(); } }
void t3(void) { while (TRUE) { toggle_led(LED4); end_cycle(); } }

void task_main(void) {
    disable_interrupts();

    pin_md(LED1, OUTPUT);
    pin_md(LED2, OUTPUT);
    pin_md(LED3, OUTPUT);
    pin_md(LED4, OUTPUT);

    dig_wr(LED1, LOW);
    dig_wr(LED2, LOW);
    dig_wr(LED3, LOW);
    dig_wr(LED4, LOW);

    int16_t idx_t1 = create(TASK1_NAME, t1, TASK_TYPE_PERIODIC, TASK1_CRIT, TASK1_PERIOD, TASK1_WCET);
    int16_t idx_t2 = create(TASK2_NAME, t2, TASK_TYPE_PERIODIC, TASK2_CRIT, TASK2_PERIOD, TASK2_WCET);
    int16_t idx_t3 = create(TASK3_NAME, t3, TASK_TYPE_PERIODIC, TASK3_CRIT, TASK3_PERIOD, TASK3_WCET);
    
    activate(idx_t1);
    activate(idx_t2);
    activate(idx_t3);     
    
    enable_interrupts();

    while (TRUE) {
        if (time_counter >= TIME_SIZE) {
            cli();
            for (uint8_t i = 0; i < TIME_SIZE; i ++) {
                Serial.println(times[i]);
                Serial.flush();
            }
            abort();
        }
        for (uint16_t i = 0; i < 60000; i++) {
            asm volatile("nop");
        }
    }

    return;
}

int main(void) {
    init(); // arduino stuff
    init_kernel(TICK_DURATION_MS, task_main);

    while (1) {
        asm("nop");
    }
    solaire_log("shit\n", LOG_FD_STDERR);
    return EXIT_SUCCESS;
}
