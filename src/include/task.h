#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#include "misc.h"
#include "kernel.h"

typedef enum { 
  TASK_TYPE_PERIODIC  = 0, 
  TASK_TYPE_SPORADIC  = 1, 
  TASK_TYPE_APERIODIC = 2 
} task_type;

typedef enum { 
  TASK_STATE_FREE   = 0, 
  TASK_STATE_READY  = 1, 
  TASK_STATE_EXE    = 2, 
  TASK_STATE_SLEEP  = 3, 
  TASK_STATE_IDLE   = 4, 
  TASK_STATE_WAIT   = 5, 
  TASK_STATE_ZOMBIE = 6 
} task_state;

typedef enum { 
  TASK_CRIT_HARD = 1, 
  TASK_CRIT_NRT = 2 
} task_crit;

typedef struct {
  char name[MAX_STR_LEN + 1]; /* task name                      */
  
  int32_t (*addr)();         /* first instruction address      */
  
  task_type type;             /* task type                      */
  task_state state;           /* task state                     */
  task_crit criticality;      /* task criticality               */
  
  long dline;                 /* task absolute deadline         */
  
  int32_t period;            /* task period                    */
  int32_t priority;          /* task priority                  */
  int32_t wcet;              /* task worst-case execution time */
  
  float utilf;                /* task utilization factor        */
  
  int32_t *ctx;               /* pointer to the task's context  */
  
  int32_t next;              /* pointer to the next TCB        */
  int32_t prev;              /* pointer to the previous TCB    */
} TCB;

void t0(void) { while(TRUE);; }
void t1(void) { while(TRUE);; }
void t2(void) { while(TRUE);; }
void t3(void) { while(TRUE);; }
void t4(void) { while(TRUE);; }
void t5(void) { while(TRUE);; }

#endif // TASK_H
