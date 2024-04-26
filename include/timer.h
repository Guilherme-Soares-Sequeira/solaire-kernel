#ifndef TIMER_H
#define TIMER_H

#define TIMER_CLEAR_COUNTER_ON_MATCH_PORT           ( 0x08 ) // when the timer reaches the defined tick value, a tick is generated and the timer is reset to 0
#define TIMER_PRESCALE_256_PORT                     ( 0x04 ) // account for 
#define TIMER_CLOCK_PRESCALER_PORT                  ( 256 ) 
#define TIMER_COMPARE_MATCH_A_INTERRUPT_ENABLE_PORT ( 0x10 )

#endif