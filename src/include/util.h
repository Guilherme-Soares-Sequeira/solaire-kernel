#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>

#define MAX_MSG_SIZE 512

typedef enum {
    LOG_FD_STDOUT,
    LOG_FD_STDERR
} log_fd;

void solaire_log(const char *str, log_fd fd);

#endif // UTIL_H
