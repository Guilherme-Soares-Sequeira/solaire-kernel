#include <Arduino.h>

#define MAX_MSG_SIZE 256

typedef enum {
    LOG_FD_STDOUT,
    LOG_FD_STDERR
} log_fd;

void log(const char *str, log_fd fd) {
    const char *prefix;

    switch (fd) {
        case LOG_FD_STDOUT: prefix = "[STDOUT] "; break;
        case LOG_FD_STDERR: prefix = "[STDERR] "; break;
        default:
            Serial.println("Invalid file descriptor provided.");
            Serial.flush();

            exit(EXIT_FAILURE);
    }

    char msg[MAX_MSG_SIZE];

    snprintf(msg, sizeof(msg), "%s%s", prefix, str);

    Serial.println(msg);
    Serial.flush();
}
