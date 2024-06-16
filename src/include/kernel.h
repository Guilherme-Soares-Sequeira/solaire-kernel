#ifndef KERNEL_H
#define KERNEL_H

#include "include/task.h"

#define TIME_SIZE 80

/* clang-format off */

#define MAX_STR_LEN      2
#define MAX_TASKS        8
#define MAX_DLINE        0x7FFFFFFF
#define PRIORITY_LEVELS  255
#define LIFETIME         (MAX_DLINE - PRIORITY_LEVELS)

extern int16_t idx_exe;     /* index to the executing process     */
extern int16_t idx_ready;   /* index to head of ready tasks queue */
extern int16_t idx_idle;    /* index to head of idle queue        */
extern int16_t idx_zombie;  /* index to head of zombie queue      */
extern int16_t idx_freetcb; /* index to head of free TCB queue    */
extern uint8_t ready_dirty; /* dirty flag for ready queue        */

extern volatile uint8_t *volatile stack_exe;

extern int16_t sys_clock; /* system's clock number of ticks since system initialization */
extern float time_unit;   /* time unit used for timer ticks */
extern float util_fact;   /* cpu utilization factor         */

typedef enum {
    KERNEL_STATE_OK            =  0,
    KERNEL_STATE_TIME_OVERFLOW = -1,
    KERNEL_STATE_TIME_EXPIRED  = -2,
    KERNEL_STATE_NO_GUARANTEE  = -3,
    KERNEL_STATE_NO_TCB        = -4,
    KERNEL_STATE_NO_SEM        = -5,
} kernel_state;

/* clang-format on */

void insert(int16_t idx_task, int16_t *queue, task_state state);

int16_t extract(int16_t idx_task, int16_t *queue);

int16_t pop(int16_t *queue);

int16_t firstdline(int16_t head);

int16_t empty(int16_t head);

void wake_up(void);

int16_t guarantee(int16_t idx_task);

void activate(int16_t idx_task);

void sleep(void);

void end_cycle(void) __attribute__((naked));

void end_process(void);

void kill(int16_t idx_task);

int16_t create(const char name[MAX_STR_LEN + 1], void (*addr)(), task_type type,
               task_crit criticality, float period, float wcet);

void dispatch(void);

void schedule(void);

void init_kernel(float tick, void (*task_main)(void)) __attribute__((naked));

uint8_t *pxPortInitialiseStack( uint8_t* pxTopOfStack, void (*pxCode)(), void *pvParameters );

#endif // KERNEL_H
