#pragma once

#include "stdint.h"

#define CLOCK_RES (100000)
typedef int64_t instant;

void initClock();
instant elapsed();
void msSleep(int);
