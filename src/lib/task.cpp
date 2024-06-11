#include "include/task.h"
#include "include/bit.h"
#include "include/register.h"

uint8_t* init_task_stack(uint8_t* pxTopOfStack, void (*ptr_to_fn)()) {
    uint16_t usAddress;
    
    usAddress = ( uint16_t ) ptr_to_fn;
    *pxTopOfStack = ( uint8_t ) ( usAddress & ( uint16_t ) 0x00ff );
    pxTopOfStack--;

    usAddress >>= 8;
    *pxTopOfStack = ( uint8_t ) ( usAddress & ( uint16_t ) 0x00ff );
    pxTopOfStack--;

    *pxTopOfStack = ( uint8_t ) 0x00;    /* R0 */
    pxTopOfStack--;

    *pxTopOfStack = ( (uint8_t) 0x00 );  /* R1 */
    pxTopOfStack --;

    pxTopOfStack -= 16;

    *pxTopOfStack = ( uint8_t ) 0x80;    /* SREG */
    pxTopOfStack --;

    pxTopOfStack -= 14;

    return pxTopOfStack;
}
