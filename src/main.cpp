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


void t1(void) { while (TRUE) { toggle_led(LED1); end_cycle(); } }
void t2(void) { while (TRUE) { toggle_led(LED2); end_cycle(); } }
void t3(void) { while (TRUE) { toggle_led(LED3); end_cycle(); } }

void task_main(void) {
    disable_interrupts();

    solaire_log("Creating tasks...\n", LOG_FD_STDOUT);

    pin_md(LED1, OUTPUT);
    pin_md(LED2, OUTPUT);
    pin_md(LED3, OUTPUT);
    pin_md(LED4, OUTPUT);
    
    int16_t idx_t1 = create(TASK1_NAME, t1, TASK_TYPE_PERIODIC, TASK1_CRIT, TASK1_PERIOD, TASK1_WCET);
    int16_t idx_t2 = create(TASK2_NAME, t2, TASK_TYPE_PERIODIC, TASK2_CRIT, TASK2_PERIOD, TASK2_WCET);
    int16_t idx_t3 = create(TASK3_NAME, t3, TASK_TYPE_PERIODIC, TASK3_CRIT, TASK3_PERIOD, TASK3_WCET);

    Serial.print("idx_t1 = ");
    Serial.flush();

    Serial.println(idx_t1);
    Serial.flush();

    Serial.print("idx_t2 = ");
    Serial.flush();

    Serial.println(idx_t2);
    Serial.flush();

    Serial.print("idx_t3 = ");
    Serial.flush();

    Serial.println(idx_t3);
    Serial.flush();

    solaire_log("Activating tasks...\n", LOG_FD_STDOUT);

    activate(idx_t1);
    activate(idx_t2);
    activate(idx_t3);     
    
    solaire_log("All tasks have been activated successfully!", LOG_FD_STDOUT);

    enable_interrupts();

    /* busy waiting */
    while (TRUE) {
        asm("nop");
    }

    return;
}

uint16_t get_sp() {
    uint16_t sp;

    asm volatile (
        "in %A0, __SP_L__ \n\t"
        "in %B0, __SP_H__"
        : "=r" (sp)
    );

    return sp;
}

void fn() { 
    uint16_t a, b, c;

    a = 0;
    a++;
    b = 1;
    b = 2*a + 1;
    c = 2;
    c = b + 3;

    uint16_t sp = get_sp();

    Serial.print("&a: ");
    Serial.flush();
    Serial.println((uint16_t) &a, HEX);
    Serial.flush();

    Serial.print("&sp: ");
    Serial.flush();
    Serial.println((uint16_t) &sp, HEX);
    Serial.flush();

    Serial.print("&fn: ");
    Serial.flush();
    Serial.println((uint16_t) &fn, HEX);
    Serial.flush();

    Serial.print("&b: ");
    Serial.flush();
    Serial.println((uint16_t) &b, HEX);
    Serial.flush();

    sp = get_sp();

    Serial.print("&c: ");
    Serial.flush();
    Serial.println((uint16_t) &c, HEX);
    Serial.flush();

    return;
}

int main() {
    Serial.begin(115200);

    fn();

    // init_kernel(TICK_DURATION_MS, task_main);

    // solaire_log("shit\n", LOG_FD_STDERR);

    return EXIT_SUCCESS;
}
