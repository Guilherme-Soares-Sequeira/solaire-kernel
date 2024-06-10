#include "include/task.h"
#include "include/bit.h"
#include "include/register.h"

volatile uint8_t *init_task_stack(volatile uint8_t *stack_ptr, void (*addr)()) {
    uint16_t addr_ptr_holder = (uint16_t)addr;
    /*
    *stack_ptr = (uint8_t) (LSB_2B_MASK & addr_ptr_holder);
    
    addr_ptr_holder >>= 8;
    stack_ptr--;
    *stack_ptr = (uint8_t) (LSB_2B_MASK & addr_ptr_holder);
    */
    stack_ptr--;
    *((uint16_t*)stack_ptr) = addr_ptr_holder; 
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
    
    Serial.print("rcv call addr = ");
    Serial.flush();
    Serial.println((uint16_t) addr, HEX);
    Serial.flush();
    Serial.print("svd call addr = ");
    Serial.flush();
    Serial.println((uint16_t) *((uint16_t*)(stack_ptr + 33)), HEX);
    Serial.flush();

    return stack_ptr;
}
