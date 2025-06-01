#pragma once

#include <stdio.h>

enum {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
};

#define LOG_MAX (1024)

extern int gLogLevel;

#define _Log(level, linum, src_file, msg, ...)                                                                         \
    do {                                                                                                               \
        if ((level) < gLogLevel)                                                                                       \
            break;                                                                                                     \
        printf("%s: [%s:%d] -> " msg "\n", fmtLogLevel((level)), srcBasename((src_file)), (linum), ##__VA_ARGS__);     \
        fflush(stdout);                                                                                                \
    } while (0)
#define Log(level, ...) _Log(level, __LINE__, __FILE__, __VA_ARGS__)

#define Trace(...) Log(LOG_TRACE, __VA_ARGS__)
#define Debug(...) Log(LOG_DEBUG, __VA_ARGS__)
#define Info(...) Log(LOG_INFO, __VA_ARGS__)
#define Warn(...) Log(LOG_WARN, __VA_ARGS__)
#define Error(...) Log(LOG_ERROR, __VA_ARGS__)
#define Fatal(...)                                                                                                     \
    do {                                                                                                               \
        Log(LOG_FATAL, __VA_ARGS__);                                                                                   \
        commitSeppuku();                                                                                               \
    } while (0)

const char* fmtLogLevel(int);
const char* srcBasename(const char*);

void logInit();
void logCleanup();
void commitSeppuku();
