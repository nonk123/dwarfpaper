#include <stdint.h>
#include <time.h>

#include "clock.h"

static clock_t start;

void clockInit() {
    start = clock();
}

instant elapsed() {
    return (((instant)clock() - (instant)start) * CLOCK_RES) / CLOCKS_PER_SEC;
}

// TODO: make cross-platform??????
#include <windows.h>

void msSleep(int ms) {
    Sleep(ms);
}
