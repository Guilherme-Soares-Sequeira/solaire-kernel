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
    
    save_ctx();

    wake_up();

    restore_ctx();

    enable_interrupts();

    asm volatile("reti"); 
}

void t1(void) { while (TRUE) { solaire_log("AaaaaaaA", LOG_FD_STDOUT); end_cycle(); } }
void t2(void) { while (TRUE) { solaire_log("BbbbB", LOG_FD_STDOUT); end_cycle(); }    }
void t3(void) { while (TRUE) { solaire_log("CcC", LOG_FD_STDOUT); end_cycle(); }      }

void task_main(void) {
    solaire_log("Creating the main task...\n", LOG_FD_STDOUT);

    if (create(TASK1_NAME, t1, TASK_TYPE_PERIODIC, TASK1_PERIOD, TASK1_WCET) != KERNEL_STATE_OK) {
        solaire_log("Error while creating Task 1!", LOG_FD_STDERR);
        return;
    }
    
    if (create(TASK2_NAME, t2, TASK_TYPE_PERIODIC, TASK2_PERIOD, TASK2_WCET) != KERNEL_STATE_OK) {
        solaire_log("Error while creating Task 2!", LOG_FD_STDERR);
        return;
    }
    
    if (create(TASK3_NAME, t3, TASK_TYPE_PERIODIC, TASK3_PERIOD, TASK3_WCET) != KERNEL_STATE_OK) {
        solaire_log("Error while creating Task 3!", LOG_FD_STDERR);
        return;
    }

    solaire_log("All tasks have been created successfully!", LOG_FD_STDOUT);

    while (TRUE);

    return;
}

int main() {
    disable_interrupts();
    Serial.begin(BAUD_RATE);
   
    solaire_log("Initializing hardware...", LOG_FD_STDOUT);
    set_timer_registers();
    solaire_log("Success!\n", LOG_FD_STDOUT);
        
    solaire_log("Initializing kernel...", LOG_FD_STDOUT);
    
    if (init_kernel(TICK_DURATION_MS, task_main) != EXIT_SUCCESS) {
        solaire_log("Error while initializing the system!", LOG_FD_STDERR);

        return EXIT_FAILURE;
    }

    enable_interrupts();

    task_main();

    solaire_log("Reached unreachable code!\n", LOG_FD_STDERR);

    return EXIT_FAILURE;
}
