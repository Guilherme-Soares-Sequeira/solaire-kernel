#ifndef KERNEL_H
#define KERNEL_H

/* clang-format off */

#define MAX_STR_LEN      16
#define MAX_TASKS        32
#define MAX_DLINE        0x7FFFFFFF
#define PRIORITY_LEVELS  255
#define TICK_DURATION_MS 5.0
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

#endif // KERNEL_H