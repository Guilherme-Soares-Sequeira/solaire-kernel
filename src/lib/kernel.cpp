#include <stdint.h>

#include "include/context.h"
#include "include/kernel.h"
#include "include/remaps.h"
#include "include/timer.h"
#include "include/util.h"
#include "include/register.h"

#define BAUD_RATE 9600

#define STACK_SIZE 128 // stack size in bytes

TCB tcb_vec[MAX_TASKS];

uint8_t stack_vec[MAX_TASKS * STACK_SIZE];

int16_t idx_exe;     /* index to the executing process     */
int16_t idx_ready;   /* index to head of ready tasks queue */
int16_t idx_idle;    /* index to head of idle queue        */
int16_t idx_zombie;  /* index to head of zombie queue      */
int16_t idx_freetcb; /* index to head of free TCB queue    */
uint8_t ready_dirty = 0; /* dirty flag for ready queue        */

volatile uint8_t *volatile stack_exe = 0;

int16_t sys_clock = 0; /* system's clock number of ticks since system initialization */
float time_unit;   /* time unit used for timer ticks */
float util_fact;   /* cpu utilization factor         */

// ------------------------ Low Level Utils ------------------------

void insert(int16_t idx_task, int16_t *queue, task_state state) {
    int16_t deadline;
    int16_t idx_prev_tcb;
    int16_t idx_next_tcb;

    idx_prev_tcb = NIL;
    idx_next_tcb = *queue;
    deadline = tcb_vec[idx_task].dline;

    tcb_vec[idx_task].state = state;

    while ((idx_next_tcb != NIL) && (deadline >= tcb_vec[idx_next_tcb].dline)) {        
        idx_prev_tcb = idx_next_tcb;
        idx_next_tcb = tcb_vec[idx_next_tcb].next;
    }

    if (idx_prev_tcb != NIL) {
        tcb_vec[idx_prev_tcb].next = idx_task;
    } else {
        *queue = idx_task;
    }

    if (idx_next_tcb != NIL) {
        tcb_vec[idx_next_tcb].prev = idx_task;
    }

    tcb_vec[idx_task].prev = idx_prev_tcb;
    tcb_vec[idx_task].next = idx_next_tcb;
}

int16_t pop(int16_t *queue) {
    int16_t head;

    head = *queue;

    if (head == NIL) {
        return NIL;
    }

    *queue = tcb_vec[head].next;

    tcb_vec[*queue].prev = NIL;

    return head;
}

int16_t firstdline(int16_t head) { return tcb_vec[head].dline; }

int16_t empty(int16_t head) {
    if (head == NIL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int16_t guarantee(int16_t idx_task) {
    util_fact += tcb_vec[idx_task].utilf;

    if (util_fact > 1.0) {
        util_fact -= tcb_vec[idx_task].utilf;
        solaire_log("Could not guarantee feasibility of task!", LOG_FD_STDOUT);
        return FALSE;
    }
    return TRUE;
}

int16_t create(const char name[MAX_STR_LEN + 1], void (*addr)(), task_type type, task_crit criticality,
               float period, float wcet) {
    int16_t idx_task = 0;

    idx_task = pop(&idx_freetcb);

    if (idx_task == NIL) {
        solaire_log("There are no free TCBs!", LOG_FD_STDERR);
        exit(KERNEL_STATE_NO_TCB);
    }

    if (tcb_vec[idx_task].criticality == TASK_CRIT_HARD) {
        if (!guarantee(idx_task)) {
            return KERNEL_STATE_NO_GUARANTEE;
        }
    }

    strcpy(tcb_vec[idx_task].name, name);
    tcb_vec[idx_task].addr = addr;
    tcb_vec[idx_task].type = type;
    tcb_vec[idx_task].criticality = criticality; 
    tcb_vec[idx_task].state = TASK_STATE_SLEEP;
    tcb_vec[idx_task].period = (int16_t)(period / time_unit);
    tcb_vec[idx_task].wcet = (int16_t)(wcet / time_unit);
    tcb_vec[idx_task].utilf = wcet / period;
    tcb_vec[idx_task].priority = (int16_t)period;

    if (type == TASK_TYPE_PERIODIC) {
        tcb_vec[idx_task].dline = (int16_t)period;
    } else if (type == TASK_TYPE_MAIN) {
        tcb_vec[idx_task].dline = INT_FAST16_MAX;
    } else {
        dig_wr(LED1, LOW);
        dig_wr(LED2, HIGH);
        dig_wr(LED3, HIGH);
        dig_wr(LED4, LOW);
        abort();
    }

    tcb_vec[idx_task].stack_ptr = (uint8_t *) init_task_stack((uint8_t*) tcb_vec[idx_task].stack_ptr, tcb_vec[idx_task].addr);
    
    return idx_task;
}

void wake_up(void) {
    int16_t count = 0;
    int16_t idx_task;

    sys_clock++;

    if (sys_clock >= LIFETIME) {
        exit(KERNEL_STATE_TIME_EXPIRED);
    }

    if (tcb_vec[idx_exe].criticality == TASK_CRIT_HARD) {
        if (sys_clock > tcb_vec[idx_exe].dline) {
            exit(KERNEL_STATE_TIME_OVERFLOW);
        }
    }

    while (!empty(idx_zombie) && firstdline(idx_zombie) <= sys_clock) {
        idx_task = pop(&idx_zombie);
        util_fact -= tcb_vec[idx_task].utilf;

        insert(idx_task, &idx_freetcb, TASK_STATE_FREE);
    }

    while (!empty(idx_idle) && (firstdline(idx_idle) <= sys_clock)) {
        idx_task = pop(&idx_idle);
        tcb_vec[idx_task].dline += (int16_t)tcb_vec[idx_task].period;

        insert(idx_task, &idx_ready, TASK_STATE_READY);

        count++;
    }

    if (count > 0 || 1 == ready_dirty) {
        schedule();
    }

    return;
}

/* TODO
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
*/

/// ------------------- 3. Scheduling Primitives ------------------- ///

void dispatch(void) {
    idx_exe = pop(&idx_ready);

    if (idx_exe == NIL) {
        solaire_log("No task to dispatch!\n", LOG_FD_STDOUT);

        return;
    }

    tcb_vec[idx_exe].state = TASK_STATE_EXE;
    stack_exe = tcb_vec[idx_exe].stack_ptr;
}

void schedule(void) {
    ready_dirty = 0;
    if (firstdline(idx_ready) < tcb_vec[idx_exe].dline) {
        tcb_vec[idx_exe].stack_ptr = stack_exe;

        insert(idx_exe, &idx_ready, TASK_STATE_READY);

        dispatch();
    }
}

/// ----------------------- End of Section 3 ----------------------- ///

void init_kernel(float tick, void (*tsk_ptr)(void)) {
    disable_interrupts();

    // // Serial.begin(BAUD_RATE);

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);

    dig_wr(LED1, LOW);
    dig_wr(LED2, LOW);
    dig_wr(LED3, LOW);
    dig_wr(LED4, LOW);

    // Serial.begin(BAUD_RATE);    

    // Serial.println("In init_kernel");
    // Serial.flush();

    set_timer_registers();

    time_unit = tick;
    for (unsigned int task_index = 0; task_index < MAX_TASKS - 1;
         task_index++) {
        tcb_vec[task_index].next = task_index + 1;
        tcb_vec[task_index].stack_ptr = stack_vec + ((task_index+1) * STACK_SIZE) - 1;
    }

    tcb_vec[MAX_TASKS - 1].next = NIL;
    tcb_vec[MAX_TASKS - 1].stack_ptr = stack_vec + ((MAX_TASKS) * STACK_SIZE) - 1;

    idx_ready = NIL;
    idx_idle = NIL;
    idx_zombie = NIL;
    idx_freetcb = 0;
    util_fact = 0.0f;

    int16_t idx_main =
        create("M", tsk_ptr, TASK_TYPE_MAIN, TASK_CRIT_NRT, 10000.0, 10000.0);

    idx_exe = idx_main;
    
    tcb_vec[idx_exe].state = TASK_STATE_EXE;
    stack_exe = tcb_vec[idx_exe].stack_ptr;

    restore_ctx();
    
    asm volatile ("ret");
    
    solaire_log("shit but in init_kernel", LOG_FD_STDERR);
    
    abort();

}

ISR(TIMER1_COMPA_vect, ISR_NAKED) {
    disable_interrupts();
    save_ctx();

    toggle_led(LED1);
    //solaire_log("Calling wake_up", LOG_FD_STDOUT);
    wake_up();
    //solaire_log("Returned from wake_up", LOG_FD_STDERR);
    restore_ctx();
    asm volatile("reti");
}

/// ------------------------ System Calls ------------------------ ///

void end_cycle(void) {
    disable_interrupts();

    save_ctx();

    int16_t deadline;

    deadline = tcb_vec[idx_exe].dline;

    if (sys_clock < deadline) {
        insert(idx_exe, &idx_idle, TASK_STATE_IDLE);
    } else {
        deadline += (int16_t)tcb_vec[idx_exe].period;

        tcb_vec[idx_exe].dline = deadline;

        insert(idx_exe, &idx_ready, TASK_STATE_READY);
    }

    tcb_vec[idx_exe].stack_ptr = stack_exe;

    dispatch();

    restore_ctx();

    asm volatile("reti");
}

void activate(int16_t idx_task) {
    ready_dirty = 1;
    if (tcb_vec[idx_task].criticality == TASK_CRIT_HARD) {
        tcb_vec[idx_task].dline = sys_clock + (int16_t)tcb_vec[idx_task].period;
    }

    insert(idx_task, &idx_ready, TASK_STATE_READY);
}

/*
void sleep(void) {
    tcb_vec[idx_exe].state = TASK_STATE_SLEEP;

    dispatch();
}

void end_process(void) {
    disable_interrupts();

    if (tcb_vec[idx_exe].criticality == TASK_CRIT_HARD) {
        insert(idx_exe, &idx_zombie, TASK_STATE_ZOMBIE);
    } else {
        insert(idx_exe, &idx_freetcb, TASK_STATE_FREE);
    }

    dispatch();
}

void kill(int16_t idx_task) {
    disable_interrupts();

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
        insert(idx_task, &idx_zombie, TASK_STATE_ZOMBIE);
    } else {
        insert(idx_task, &idx_freetcb, TASK_STATE_FREE);
    }

    enable_interrupts();
}
*/
