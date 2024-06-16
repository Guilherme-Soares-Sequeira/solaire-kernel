#ifndef TIMER_H
#define TIMER_H

#define TIMER_CLEAR_COUNTER_ON_MATCH_PORT           ( 0x08 ) // when the timer reaches the defined tick value, a tick is generated and the timer is reset to 0
#define TIMER_PRESCALE_256_PORT                     ( 0x04 ) // account for 
#define TIMER_CLOCK_PRESCALER_PORT                  ( 256 ) 
#define TIMER_COMPARE_MATCH_A_INTERRUPT_ENABLE_PORT ( 0x10 )
#define TICK_DURATION_MS                              100.0


/**
 * @brief 
 * 
 */
void reset_timer1_control_registers();

/**
 * @brief 
 * 
 */
void reset_timer1_counting_register();

/**
 * @brief Set the timer1 ctc mode object
 * 
 */
void set_timer1_ctc_mode();

/**
 * @brief Set the timer1 prescaler 256 object
 * 
 */
void set_timer1_prescaler_256();

/**
 * @brief Set the timer1 interrupt on compare match A object
 * 
 */
void set_timer1_interrupt_on_compare_match_A();

/**
 * @brief Set the interrupt period object
 * 
 * @param tick_delay 
 */
void set_interrupt_period(float tick_delay);

/**
 * @brief Set the timer registers object
 * 
 */
void set_timer_registers();

#endif
