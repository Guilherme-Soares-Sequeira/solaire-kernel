#include "include/util.h"

#include <Arduino.h>

#include "include/remaps.h"

void toggle_led(uint8_t led_pin) {
    dig_wr(led_pin, !dig_rd(led_pin));
}

void __solaire_log_fn(const __FlashStringHelper *str, log_fd fd) {
    const __FlashStringHelper *prefix;

    switch (fd) {
        case LOG_FD_STDOUT: prefix = F("[STDOUT] "); break;
        case LOG_FD_STDERR: prefix = F("[STDERR] "); break;
        default:
            Serial.println(F("Invalid file descriptor provided."));
            Serial.flush();

            exit(EXIT_FAILURE);
    }

    Serial.print(prefix);
    Serial.println(str);
    
    Serial.flush();
}
