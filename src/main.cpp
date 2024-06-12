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


void t1(void) { 
    dig_wr(LED2, HIGH); // disable led 2 first time t1 is run
    while (TRUE) {
        toggle_led(LED3);
        end_cycle();
        }
    
}
void t2(void) { while (TRUE) { toggle_led(LED3); end_cycle(); } }
void t3(void) { while (TRUE) { toggle_led(LED4); end_cycle(); } }

void task_main(void) {
    disable_interrupts();

    Serial.println("in task_main");
    Serial.flush();

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

    Serial.println("Activating tasks...\n");
    Serial.flush();
    
    activate(idx_t1);
    //activate(idx_t2);
    //activate(idx_t3);     
    
    Serial.println("All tasks have been activated successfully!");
    Serial.flush();

    enable_interrupts();
    /* busy waiting */
    while (TRUE) {
        toggle_led(LED4);
        _delay_ms(50);
        asm volatile("nop");
    }

    return;
}

int main(void) {
    init_kernel(TICK_DURATION_MS, task_main);

    Serial.println("busy waiting in main function...");
    Serial.flush();

    while (1) {
        asm("nop");
    }
    solaire_log("shit\n", LOG_FD_STDERR);
    return EXIT_SUCCESS;
}
