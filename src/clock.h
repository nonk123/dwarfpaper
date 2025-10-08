#pragma once

#include <SDL3/SDL_stdinc.h>

#define CLOCK_SECOND ((uint64_t)(1000000000))
typedef uint64_t Instant;
Instant elapsed();
