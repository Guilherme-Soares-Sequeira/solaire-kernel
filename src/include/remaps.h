#ifndef REMAPS_H
#define REMAPS_H

#include <Arduino.h>

inline void dig_wr(uint8_t pin, uint8_t val) {
    digitalWrite(pin, val);
}

inline int dig_rd(uint8_t pin) {
    return digitalRead(pin);
}

inline void pin_md(uint8_t pin, uint8_t mode) {
    pinMode(pin, mode);       
}

inline void disable_interrupts() {
    noInterrupts();
}

inline void enable_interrupts() {
    interrupts();
}

#endif // REMAPS_H