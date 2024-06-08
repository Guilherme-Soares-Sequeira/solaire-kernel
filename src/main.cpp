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

#define BAUD_RATE 115200

ISR(TIMER1_COMPA_vect) {
    disable_interrupts();
    
    wake_up();

    enable_interrupts(); 
}

int16_t t1(void) { while (TRUE) { solaire_log("AaaaaaaA", LOG_FD_STDOUT); end_cycle(); } return EXIT_SUCCESS; }
int16_t t2(void) { while (TRUE) { solaire_log("BbbbB", LOG_FD_STDOUT); end_cycle(); }    return EXIT_SUCCESS; }
int16_t t3(void) { while (TRUE) { solaire_log("CcC", LOG_FD_STDOUT); end_cycle(); }      return EXIT_SUCCESS; }

int16_t task_main(void) {
    if (create("Task 1", t1, TASK_TYPE_PERIODIC, /* period = */ 100.0, /* wcet = */ 50.0) != KERNEL_STATE_OK) {
        solaire_log("Error while creating Task 1!", LOG_FD_STDERR);

        return EXIT_FAILURE;
    }
    
    if (create("Task 2", t2, TASK_TYPE_PERIODIC, /* period = */ 100.0, /* wcet = */ 50.0) != KERNEL_STATE_OK) {
        solaire_log("Error while creating Task 2!", LOG_FD_STDERR);

        return EXIT_FAILURE;
    }
    
    if (create("Task 3", t3, TASK_TYPE_PERIODIC, /* period = */ 100.0, /* wcet = */ 50.0) != KERNEL_STATE_OK) {
        solaire_log("Error while creating Task 3!", LOG_FD_STDERR);

        return EXIT_FAILURE;
    }    
    
    while (TRUE) {
        end_cycle();        
    }

    return EXIT_SUCCESS;
}

int main() {
    Serial.begin(BAUD_RATE);

    solaire_log("Initializing hardware...", LOG_FD_STDOUT);
    set_timer_registers();
    solaire_log("Success!\n", LOG_FD_STDOUT);

    solaire_log("Initializing kernel", LOG_FD_STDOUT);
    
    if (init_kernel(TICK_DURATION_MS, task_main) != EXIT_SUCCESS) {
        solaire_log("Error while initializing the system!", LOG_FD_STDERR);

        return EXIT_FAILURE;
    }
    solaire_log("Success!\n", LOG_FD_STDOUT);

    return EXIT_SUCCESS;
}
