#pragma once

#include <SDL3/SDL_stdinc.h>

#define CHR_WIDTH (9)
#define CHR_HEIGHT (16)

#define MAX_WIDTH (420)
#define MAX_HEIGHT (120)

typedef struct {
	uint8_t chr, fg : 4, bg : 4;
} Cell;

int screen_cols(), screen_rows(), screen_width(), screen_height();
Cell *cell_at(int, int), *cell_at_ex(Cell*, int, int);
