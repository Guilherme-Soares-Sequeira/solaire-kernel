#include "include/task.h"
#include "include/bit.h"
#include "include/register.h"

uint8_t* init_task_stack(uint8_t* stack_ptr, void (*addr)()) {
    uint16_t addr_ptr_holder = (uint16_t) addr;
    
    *(stack_ptr) = (uint8_t) (addr_ptr_holder & LSB_2B_MASK);

    addr_ptr_holder >>= REG_SIZE;

    *(stack_ptr) = (uint8_t) (addr_ptr_holder & LSB_2B_MASK);
    stack_ptr--;

    // Memory is initialized to 0, so skip all 31 general purpose registers + status register
    stack_ptr -= SAVED_REG_NUM;
    
    return stack_ptr;
}
