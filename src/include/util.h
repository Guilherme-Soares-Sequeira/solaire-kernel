#ifndef UTIL_H
#define UTIL_H

#define MAX_MSG_SIZE 512

#define LED1 13
#define LED2 12
#define LED3 11
#define LED4 10

#define LOG_ENABLED 1

#ifdef LOG_ENABLED
#define solaire_log(str, fd) __solaire_log_fn(F(str), fd);
#else
#define solaire_log(str, fd) ;
#endif

typedef enum {
    LOG_FD_STDOUT,
    LOG_FD_STDERR
} log_fd;

void __solaire_log_fn(const __FlashStringHelper *str, log_fd fd);

void solaire_dynamic_log(const char* str, log_fd fd);

void toggle_led(uint8_t led_pin);

#endif // UTIL_H
