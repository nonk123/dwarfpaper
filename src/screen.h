#pragma once

#include <stdint.h>

#define CHR_WIDTH (9)
#define CHR_HEIGHT (16)

#define MAX_WIDTH (420)
#define MAX_HEIGHT (120)

typedef struct {
	uint8_t chr, fg : 4, bg : 4;
} Cell;

int screen_cols(), screen_rows(), screen_width(), screen_height();
Cell *cell_at(int, int), *cell_at_ex(Cell*, int, int);

// Straight outta <https://github.com/nonk123/micraneft2>:

enum {
	C_BLACK,
	C_BLUE,
	C_GREEN,
	C_AQUA,
	C_RED,
	C_PURPLE,
	C_YELLOW,
	C_WHITE,
	C_GRAY,
	C_BRIGHT_BLUE,
	C_BRIGHT_GREEN,
	C_BRIGHT_AQUA,
	C_BRIGHT_RED,
	C_BRIGHT_PURPLE,
	C_BRIGHT_YELLOW,
	C_BRIGHT_WHITE,
};
