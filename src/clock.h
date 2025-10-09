#pragma once

#include <SDL3/SDL_stdinc.h>

typedef uint64_t Instant;
#define CLOCK_SECOND ((Instant)(1000000000))
Instant elapsed();

typedef uint32_t Ticks;
#define TICKRATE ((Ticks)(60))
Ticks ticks();
