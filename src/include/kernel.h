#ifndef KERNEL_H
#define KERNEL_H

#include "include/task.h"

/* clang-format off */

#define MAX_STR_LEN      16
#define MAX_TASKS        8
#define MAX_DLINE        0x7FFFFFFF
#define PRIORITY_LEVELS  255
#define LIFETIME         (MAX_DLINE - PRIORITY_LEVELS)

typedef enum {
    KERNEL_STATE_OK            =  0,
    KERNEL_STATE_TIME_OVERFLOW = -1,
    KERNEL_STATE_TIME_EXPIRED  = -2,
    KERNEL_STATE_NO_GUARANTEE  = -3,
    KERNEL_STATE_NO_TCB        = -4,
    KERNEL_STATE_NO_SEM        = -5,
} kernel_state;

/* clang-format on */


void insert(int16_t idx_task, int16_t *queue);

int16_t extract(int16_t idx_task, int16_t *queue);

int16_t getfirst(int16_t *queue);

int32_t firstdline(int16_t head);

int16_t empty(int16_t head);

kernel_state wake_up(void);

int16_t guarantee(int16_t idx_task);

void activate(int16_t idx_task);

void sleep(void);

void end_cycle(void);

void end_process(void);

void kill(int16_t idx_task);

int16_t create(const char name[MAX_STR_LEN + 1], void (*addr)(),
               task_type type, float period, float wcet);

void dispatch(void);

void schedule(void);

int16_t init_kernel(float tick, void (*task_main)(void));

#endif // KERNEL_H