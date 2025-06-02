#include <stdint.h>
#include <time.h>

#include <windows.h>

#include "clock.h"

static clock_t start;

void initClock() {
    start = clock();
}

instant elapsed() {
    return (((instant)clock() - (instant)start) * CLOCK_RES) / CLOCKS_PER_SEC;
}

void msSleep(int ms) {
    Sleep(ms);
}
