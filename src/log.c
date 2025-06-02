#include <stdlib.h>
#include <string.h>

#include "log.h"

int gLogLevel = LOG_INFO;

const char* fmtLogLevel(int level) {
    switch (level) {
        case LOG_TRACE:
            return "TRACE";
        case LOG_DEBUG:
            return "DEBUG";
        case LOG_INFO:
            return "INFO";
        case LOG_WARN:
            return "WARN";
        case LOG_ERROR:
            return "ERROR";
        case LOG_FATAL:
            return "FATAL";
    }

    commitSeppuku();
    return NULL;
}

const char* srcBasename(const char* path) {
    const char* s = strrchr(path, '/');

    if (s == NULL)
        s = strrchr(path, '\\');

    return s == NULL ? path : s + 1;
}

void commitSeppuku() {
    exit(EXIT_FAILURE);
}
