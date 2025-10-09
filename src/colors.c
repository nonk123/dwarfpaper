#include "colors.h"

enum Color rand_bright() {
	return 9 + SDL_rand(7);
}

#define FULL (255)
#define MID (170)
#define LOW (85)

// clang-format off
const SDL_Color colors[C_MAX] = {
	[C_BLACK] = {0, 0, 0, 255},
	[C_GRAY] = {LOW, LOW, LOW, 255},
	[C_WHITE] = {MID, MID, MID, 255},
	[C_BRIGHT_WHITE] = {FULL, FULL, FULL, 255},
	[C_RED] = {MID, 0, 0, 255},
	[C_BRIGHT_RED] = {FULL, LOW, LOW, 255},
	[C_GREEN] = {0, MID, 0, 255},
	[C_BRIGHT_GREEN] = {LOW, FULL, LOW, 255},
	[C_YELLOW] = {FULL, MID, 0, 255},
	[C_BRIGHT_YELLOW] = {FULL, FULL, LOW, 255},
	[C_BLUE] = {0, 0, MID, 255},
	[C_BRIGHT_BLUE] = {LOW, LOW, FULL, 255},
	[C_PURPLE] = {MID,  0, MID, 255},
	[C_BRIGHT_PURPLE] = {FULL, LOW, FULL, 255},
	[C_AQUA] = {0, MID, MID, 255},
	[C_BRIGHT_AQUA] = {LOW, FULL, FULL, 255},
};
// clang-format on

#undef LOW
#undef MID
#undef FULL
