#include "include/task.h"
#include "include/bit.h"
#include "include/register.h"

volatile uint8_t *init_task_stack(volatile uint8_t *stack_ptr, void (*addr)()) {
    uint16_t addr_ptr_holder = (uint16_t)addr;

    // atmega328p is little-endian so MSB is in highest address
    *(stack_ptr) = (uint8_t)((addr_ptr_holder >> REG_SIZE) & LSB_2B_MASK);
    
    stack_ptr--;
    *(stack_ptr) = (uint8_t)(addr_ptr_holder & LSB_2B_MASK);

    /*
    // Memory is initialized to 0, so skip all 32 general purpose registers +
    status register stack_ptr -= SAVED_REG_NUM;
    */
    stack_ptr--;
    *(stack_ptr) = 0x00; // R0
    
    stack_ptr--;
    *(stack_ptr) = 0x00; // Status Register
    
    for (uint8_t i = 1; i < SAVED_REG_NUM - 1; i++) {
        stack_ptr--;
        *(stack_ptr) = (uint8_t)i;
    }

    return stack_ptr;
}
