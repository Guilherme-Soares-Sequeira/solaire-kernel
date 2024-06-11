#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <Arduino.h>

#include "misc.h"

#define MAX_STR_LEN 2

#define TASK1_NAME "T1"
#define TASK2_NAME "T2"
#define TASK3_NAME "T3"

#define TASK1_PERIOD  250.0
#define TASK2_PERIOD 500.0
#define TASK3_PERIOD 1000.0

#define TASK1_CRIT TASK_CRIT_HARD
#define TASK2_CRIT TASK_CRIT_HARD
#define TASK3_CRIT TASK_CRIT_HARD

#define TASK1_WCET 25.0
#define TASK2_WCET 25.0
#define TASK3_WCET 25.0

typedef enum {
    TASK_TYPE_PERIODIC = 0,
    TASK_TYPE_SPORADIC = 1,
    TASK_TYPE_APERIODIC = 2
} task_type;

typedef enum {
    TASK_STATE_FREE = 0,
    TASK_STATE_READY = 1,
    TASK_STATE_EXE = 2,
    TASK_STATE_SLEEP = 3,
    TASK_STATE_IDLE = 4,
    TASK_STATE_WAIT = 5,
    TASK_STATE_ZOMBIE = 6
} task_state;

typedef enum { TASK_CRIT_HARD = 1, TASK_CRIT_NRT = 2 } task_crit;

typedef struct {
    char name[MAX_STR_LEN + 1]; /* task name                      */

    void (*addr)(); /* first instruction address */

    task_type type;        /* task type                      */
    task_state state;      /* task state                     */
    task_crit criticality; /* task criticality */

    int32_t dline;    /* task absolute deadline */

    int16_t period;   /* task period                         */
    int16_t priority; /* task priority                       */
    int16_t wcet;     /* task worst-case execution time      */

    float utilf;      /* task utilization factor                  */

    volatile uint8_t* stack_ptr; /* pointer to the task's stack */

    int16_t next; /* pointer to the next TCB     */
    int16_t prev; /* pointer to the previous TCB */
} TCB;

uint8_t* init_task_stack(uint8_t* stack_ptr, void (*addr)());

#endif // TASK_H
