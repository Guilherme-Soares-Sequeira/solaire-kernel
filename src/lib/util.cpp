#include "include/util.h"

void solaire_log(const char *str, log_fd fd) {
    const char *prefix;

    switch (fd) {
        case LOG_FD_STDOUT: prefix = "[STDOUT] "; break;
        case LOG_FD_STDERR: prefix = "[STDERR] "; break;
        default:
            Serial.println("Invalid file descriptor provided.");
            Serial.flush();

            exit(EXIT_FAILURE);
    }

    Serial.print(prefix);
    Serial.println(str);
    
    Serial.flush();
}
