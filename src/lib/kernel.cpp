#include <stdint.h>

#include "include/context.h"
#include "include/kernel.h"
#include "include/remaps.h"
#include "include/timer.h"
#include "include/util.h"

#define BAUD_RATE 115200

#define STACK_SIZE 128 // stack size in bytes

TCB tcb_vec[MAX_TASKS];

uint8_t stack_vec[MAX_TASKS * STACK_SIZE];

int16_t idx_exe;     /* index to the executing process     */
int16_t idx_ready;   /* index to head of ready tasks queue */
int16_t idx_idle;    /* index to head of idle queue        */
int16_t idx_zombie;  /* index to head of zombie queue      */
int16_t idx_freetcb; /* index to head of free TCB queue    */

volatile uint8_t *volatile stack_exe;

int32_t sys_clock; /* system's clock number of ticks since system initialization */
float time_unit;   /* time unit used for timer ticks */
float util_fact;   /* cpu utilization factor         */

// ------------------------ Low Level Utils ------------------------

void insert(int16_t idx_task, int16_t *queue, task_state state) {
    int32_t deadline;
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

int32_t firstdline(int16_t head) { return tcb_vec[head].dline; }

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

    Serial.print("received addr = ");
    Serial.flush();
    Serial.println((uint16_t) addr);
    Serial.flush();

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
    tcb_vec[idx_task].dline = INT_FAST32_MAX + (int32_t)(period - PRIORITY_LEVELS);
    tcb_vec[idx_task].stack_ptr = (uint8_t *)init_task_stack(tcb_vec[idx_task].stack_ptr, tcb_vec[idx_task].addr);
    
    return idx_task;
}

void wake_up(void) {
    int16_t count = 0;
    int16_t idx_task;

    sys_clock++;

    if (sys_clock >= LIFETIME) {
        solaire_log("The system time has expired!", LOG_FD_STDERR);

        exit(KERNEL_STATE_TIME_EXPIRED);
    }

    if (tcb_vec[idx_exe].criticality == TASK_CRIT_HARD) {
        if (sys_clock > tcb_vec[idx_exe].dline) {
            solaire_log("Timer overflow!", LOG_FD_STDERR);

            exit(KERNEL_STATE_TIME_OVERFLOW);
        }
    }

    while (!empty(idx_zombie) && firstdline(idx_zombie) <= sys_clock) {
        idx_task = pop(&idx_zombie);
        util_fact -= tcb_vec[idx_task].utilf;

        insert(idx_task, &idx_freetcb, TASK_STATE_FREE);
    }

    while (!empty(idx_idle) && (firstdline(idx_idle) <= sys_clock)) {
        solaire_log("The idle queue is not empty", LOG_FD_STDOUT);

        idx_task = pop(&idx_idle);
        tcb_vec[idx_task].dline += (int32_t)tcb_vec[idx_task].period;

        insert(idx_task, &idx_ready, TASK_STATE_READY);

        count++;
    }

    if (count > 0) {
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
    if (firstdline(idx_ready) < tcb_vec[idx_exe].dline) {
        tcb_vec[idx_exe].stack_ptr = stack_exe;

        insert(idx_exe, &idx_ready, TASK_STATE_READY);

        dispatch();
    }
}

/// ----------------------- End of Section 3 ----------------------- ///

uint16_t get_sp() {
    return ((uint16_t)SPH << 8) | SPL;
}

void init_kernel(float tick, void (*task_main)(void)) {
    Serial.begin(BAUD_RATE);
    
    solaire_log("Hello", LOG_FD_STDOUT);

    disable_interrupts();

    set_timer_registers();

    time_unit = tick;
    for (unsigned int task_index = 0; task_index < MAX_TASKS - 1;
         task_index++) {
        tcb_vec[task_index].next = task_index + 1;
        tcb_vec[task_index].stack_ptr = &(stack_vec[task_index]);
    }

    tcb_vec[MAX_TASKS - 1].next = NIL;
    tcb_vec[MAX_TASKS - 1].stack_ptr = &(stack_vec[MAX_TASKS - 1]);

    idx_ready = NIL;
    idx_idle = NIL;
    idx_zombie = NIL;
    idx_freetcb = 0;
    util_fact = 0.0f;

    int16_t idx_main =
        create("M", task_main, TASK_TYPE_APERIODIC, TASK_CRIT_NRT, 10000.0, 10000.0);
    
    if (idx_main != EXIT_SUCCESS) {
        solaire_log("Error while initializing main task!", LOG_FD_STDERR);

        abort();
    }

    idx_exe = idx_main;
    
    tcb_vec[idx_exe].state = TASK_STATE_EXE;
    stack_exe = tcb_vec[idx_exe].stack_ptr;

    Serial.print("stack exe = ");
    Serial.println((uint16_t) stack_exe, HEX);
    Serial.flush();

    uint16_t sp = get_sp();
    Serial.print("sp2 = ");
    Serial.println(sp, HEX);
    Serial.flush();
    
    //restore_ctx();
    
    //SPL = 0x34;
    //Serial.print("spl = ");
    //Serial.println(SPL, HEX);
    //Serial.flush();

    asm volatile("ldi r22,14                   \n\t"                              \
                 "out __SP_H__, r22            \n\t"                              \
    );

    Serial.print("sph = ");
    Serial.println(SPH, HEX);
    Serial.flush();

    abort();
    asm ("ret");

    solaire_log("shit but in init_kernel", LOG_FD_STDERR);
    
    abort();

}

ISR(TIMER1_COMPA_vect, ISR_NAKED) {
    disable_interrupts();

    save_ctx();

    solaire_log("Calling wake_up", LOG_FD_STDOUT);
    wake_up();
    solaire_log("Returned from wake_up", LOG_FD_STDOUT);

    restore_ctx();

    enable_interrupts();

    asm volatile("reti");
}

/// ------------------------ System Calls ------------------------ ///

void end_cycle(void) {
    disable_interrupts();

    save_ctx();

    int32_t deadline;

    deadline = tcb_vec[idx_exe].dline;

    if (sys_clock < deadline) {
        insert(idx_exe, &idx_idle, TASK_STATE_IDLE);
    } else {
        deadline += (int32_t)tcb_vec[idx_exe].period;

        tcb_vec[idx_exe].dline = deadline;

        insert(idx_exe, &idx_ready, TASK_STATE_READY);
    }

    tcb_vec[idx_exe].stack_ptr = stack_exe;

    dispatch();

    restore_ctx();

    asm volatile("reti");
}


void activate(int16_t idx_task) {
    if (tcb_vec[idx_task].criticality == TASK_CRIT_HARD) {
        tcb_vec[idx_task].dline = sys_clock + (int32_t)tcb_vec[idx_task].period;
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
