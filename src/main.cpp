#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

#include "include/remaps.h"
#include "include/task.h"
#include "include/util.h"

#define BAUD_RATE 115200

TCB tcb_vec[MAX_TASKS];

int16_t idx_exe;     /* index to the executing process     */
int16_t idx_ready;   /* index to head of ready tasks queue */
int16_t idx_idle;    /* index to head of idle queue        */
int16_t idx_zombie;  /* index to head of zombie queue      */
int16_t idx_freetcb; /* index to head of free TCB queue    */

int32_t sys_clock; /* system's clock                      */

float time_unit; /* time unit used for timer ticks     */
float util_fact; /* cpu utilization factor             */

bool volatile tf = true;

void schedule(void);
void dispatch(void);

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

void init_hardware(void) {
    disable_interrupts();

    set_timer_registers();

    enable_interrupts();
}

void insert(int16_t idx_task, int16_t *queue) {
    int32_t deadline;
    int16_t prev_tcb;
    int16_t next_tcb;

    prev_tcb = NIL;
    next_tcb = *queue;
    deadline = tcb_vec[idx_task].dline;

    while ((next_tcb != NIL) && (deadline >= tcb_vec[next_tcb].dline)) {
        prev_tcb = next_tcb;
        next_tcb = tcb_vec[next_tcb].next;
    }

    if (prev_tcb != NIL) {
        tcb_vec[prev_tcb].next = idx_task;
    } else {
        *queue = idx_task;
    }

    if (next_tcb != NIL) {
        tcb_vec[next_tcb].prev = idx_task;
    }

    tcb_vec[idx_task].prev = prev_tcb;
    tcb_vec[idx_task].next = next_tcb;
}

int16_t extract(int16_t idx_task, int16_t *queue) {
    int16_t prev_tcb;
    int16_t next_tcb;

    prev_tcb = tcb_vec[idx_task].prev;
    next_tcb = tcb_vec[idx_task].next;

    if (prev_tcb == NIL) {
        *queue = next_tcb;
    } else {
        tcb_vec[prev_tcb].next = tcb_vec[idx_task].next;
    }

    if (next_tcb != NIL) {
        tcb_vec[next_tcb].prev = tcb_vec[idx_task].prev;
    }

    return idx_task;
}

int16_t getfirst(int16_t *queue) {
    int16_t head;

    head = *queue;

    if (head == NIL) {
        return NIL;
    }

    *queue = tcb_vec[head].next;
    tcb_vec[*queue].prev = NIL;

    return head;
}

int32_t firstdline(int16_t head) { return tcb_vec[head].dline; }

int16_t empty(int16_t head) {
    // TODO: this is probably very stupid, need to change later
    if (head == NIL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

kernel_state wake_up(void) {
    int16_t count = 0;
    int16_t idx_task;

    // TODO: call save_ctx() here
    sys_clock++;

    if (sys_clock >= LIFETIME) {
        return KERNEL_STATE_TIME_EXPIRED;
    }

    if (tcb_vec[idx_exe].criticality == TASK_CRIT_HARD) {
        if (sys_clock > tcb_vec[idx_exe].dline) {
            return KERNEL_STATE_TIME_OVERFLOW;
        }
    }

    while (!empty(idx_zombie) && firstdline(idx_zombie) <= sys_clock) {
        idx_task = getfirst(&idx_zombie);
        util_fact -= tcb_vec[idx_task].utilf;
        tcb_vec[idx_task].state = TASK_STATE_FREE;

        insert(idx_task, &idx_freetcb);
    }

    while (!empty(idx_idle) && (firstdline(idx_idle) <= sys_clock)) {
        idx_task = getfirst(&idx_idle);
        tcb_vec[idx_task].dline += (int32_t)tcb_vec[idx_task].period;
        tcb_vec[idx_task].state = TASK_STATE_READY;
        insert(idx_task, &idx_ready);
        count++;
    }

    if (count > 0) {
        schedule(); // TODO: this function needs to be renamed
    }

    // TODO: call load_ctx() here

    // TODO: check if this is correct
    return KERNEL_STATE_OK;
}

int16_t guarantee(int16_t idx_task) {
    util_fact += tcb_vec[idx_task].utilf;

    if (util_fact > 1.0) {
        util_fact -= tcb_vec[idx_task].utilf;

        return FALSE;
    }

    return TRUE;
}

void activate(int16_t idx_task) {
    // TODO: call save_ctx() here
    if (tcb_vec[idx_task].criticality == TASK_CRIT_HARD) {
        tcb_vec[idx_task].dline = sys_clock + (int32_t)tcb_vec[idx_task].period;
    }

    tcb_vec[idx_task].state = TASK_STATE_READY;

    insert(idx_task, &idx_ready);

    // TODO: call load_ctx() here
}

void sleep(void) {
    // TODO: call save_ctx() here
    tcb_vec[idx_exe].state = TASK_STATE_SLEEP;

    dispatch();

    // TODO: call load_ctx() here
}

void end_cycle(void) {
    int32_t deadline;

    // TODO: call save_ctx() here

    deadline = tcb_vec[idx_exe].dline;

    if (sys_clock < deadline) {
        tcb_vec[idx_exe].state = TASK_STATE_IDLE;

        insert(idx_exe, &idx_idle);
    } else {
        deadline += (int32_t)tcb_vec[idx_exe].period;

        tcb_vec[idx_exe].dline = deadline;
        tcb_vec[idx_exe].state = TASK_STATE_READY;

        insert(idx_exe, &idx_ready);
    }

    dispatch();

    // TODO: call load_ctx() here
}

void end_process(void) {
    // TODO: call disable_interrupts() here
    if (tcb_vec[idx_exe].criticality == TASK_CRIT_HARD) {
        insert(idx_exe, &idx_zombie);
    } else {
        tcb_vec[idx_exe].state = TASK_STATE_FREE;

        insert(idx_exe, &idx_freetcb);
    }

    dispatch();

    // TODO: call load_ctx() here
}

void kill(int16_t idx_task) {
    // TODO: call enable_interrupts() here
    if (idx_exe == idx_task) {
        end_process();

        return;
    }

    switch (tcb_vec[idx_exe].state) {
    case TASK_STATE_READY:
        extract(idx_task, &idx_ready);
        break;
    case TASK_STATE_IDLE:
        extract(idx_task, &idx_idle);
        break;
    default:
        break;
    }

    if (tcb_vec[idx_task].criticality == TASK_CRIT_HARD) {
        insert(idx_task, &idx_zombie);
    } else {
        tcb_vec[idx_task].state = TASK_STATE_FREE;

        insert(idx_task, &idx_freetcb);
    }
    // TODO: call disable_interrupts() here
}

int16_t create(const char name[MAX_STR_LEN + 1], int16_t (*addr)(),
               task_type type, float period, float wcet) {
    int16_t idx_task;

    // TODO: call enable_interrupts() here

    idx_task = getfirst(&idx_freetcb);
    if (idx_task == NIL) {
        return KERNEL_STATE_NO_TCB;
    }

    if (tcb_vec[idx_task].criticality == TASK_CRIT_HARD) {
        if (!guarantee(idx_task)) {
            return KERNEL_STATE_NO_GUARANTEE;
        }
    }

    // tcb_vec[idx_task].name = name;
    tcb_vec[idx_task].addr = addr;
    tcb_vec[idx_task].type = type;
    tcb_vec[idx_task].state = TASK_STATE_SLEEP;
    tcb_vec[idx_task].period = (int16_t)(period / time_unit);
    tcb_vec[idx_task].wcet = wcet;
    tcb_vec[idx_task].utilf = wcet / period;
    tcb_vec[idx_task].priority = (int16_t)period;
    tcb_vec[idx_task].dline =
        /* TODO: MAX_int32_t + */ (int32_t)(period - PRIORITY_LEVELS);

    // TODO: enable CPU interrupts

    return idx_task;
}

void dispatch(void) {
    idx_exe = getfirst(&idx_ready);

    if (idx_exe != NIL) {
        tcb_vec[idx_exe].state = TASK_STATE_EXE;
    } else {
        log("No task to dispatch!", LOG_FD_STDOUT);
    }
}

void schedule(void) {
    if (firstdline(idx_ready) < tcb_vec[idx_exe].dline) {
        tcb_vec[idx_exe].state = TASK_STATE_READY;

        insert(idx_exe, &idx_ready);

        dispatch();
    }
}

/**
 *
 * @param tick
 */
void init_system(float tick) {
    time_unit = tick;

    for (unsigned int task_index = 0; task_index < MAX_TASKS - 1;
         task_index++) {
        tcb_vec[task_index].next = task_index + 1;
    }

    tcb_vec[MAX_TASKS - 1].next = NIL;

    idx_ready = NIL;
    idx_idle = NIL;
    idx_zombie = NIL;
    idx_freetcb = 0;
    util_fact = 0.0f;
}

ISR(TIMER1_COMPA_vect) {

}

int main() {
    Serial.begin(BAUD_RATE);

    log("Initializing hardware...", LOG_FD_STDOUT);
    init_hardware();
    log("Success!\n", LOG_FD_STDERR);

    log("Initializing kernel", LOG_FD_STDOUT);  
    init_system(TICK_DURATION_MS);
    log("Success!\n", LOG_FD_STDOUT);

    return EXIT_SUCCESS;
}