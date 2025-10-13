#include <SDL3/SDL_stdinc.h>

#include "colors.h"
#include "noise.h"
#include "screen.h"

#include "modes/forest.h"

typedef struct {
	uint8_t initialized;
} State;

#define SKY (C_BLACK)
#define LOG (219)

#define SEED_PROB (8)
#define SEED_GROW_PROB (3)
static const uint8_t seed[] = {'.', ',', ':', '%', '$'}, seed_max = sizeof(seed);

static int flip(int y) {
	return screen_rows() - 1 - y;
}

static void clear_spot(int x, int y) {
	y = flip(y);
	cell_at(x, y)->bg = cell_at(x, y)->fg = SKY;
	cell_at(x, y)->chr = ' ';
}

static void place_rock(int x, int y) {
	y = flip(y);
	cell_at(x, y)->bg = cell_at(x, y)->fg = C_WHITE;
	cell_at(x, y)->chr = '#';
}

static void place_dirt(int x, int y) {
	y = flip(y);
	cell_at(x, y)->bg = cell_at(x, y)->fg = C_YELLOW;
	cell_at(x, y)->chr = '#';
}

static void place_grass(int x, int y) {
	y = flip(y);
	cell_at(x, y)->bg = cell_at(x, y)->fg = C_GREEN;
	cell_at(x, y)->chr = '#';
}

static void place_seed(int x, int y, int stage) {
	y = flip(y);
	cell_at(x, y)->bg = SKY;
	cell_at(x, y)->fg = C_YELLOW;
	cell_at(x, y)->chr = stage;
}

static void drop_seed(int x) {
	place_seed(x, screen_rows() - 1, seed[0]);
}

static void place_log(int x, int y) {
	y = flip(y);
	cell_at(x, y)->chr = LOG;
	cell_at(x, y)->fg = C_YELLOW;
	cell_at(x, y)->bg = SKY;
}

static int height_at(int ix) {
	const int sloping = 12, breadth = 64;
	const float x = ((float)ix) / ((float)breadth);
	return (int)(sloping * noise_at(x, 0.f));
}

static void generate(State* this) {
	this->initialized = 1;
	const int baseline = screen_rows() / 6, rockbed = 8;
	for (int ix = 0; ix < screen_cols(); ix++) {
		int iy = 0, height = height_at(ix), prev_height = height_at(ix - 1);
		for (; iy < baseline + height - rockbed; iy++)
			place_rock(ix, iy);
		for (; iy < baseline + height; iy++)
			place_dirt(ix, iy);
		if (prev_height < height)
			place_grass(ix, baseline + prev_height);
		else if (prev_height > height)
			place_grass(ix - 1, baseline + prev_height - 1);
		place_grass(ix, iy++);
		for (; iy < screen_rows(); iy++)
			clear_spot(ix, iy);
		if (SDL_rand(100) < SEED_PROB)
			drop_seed(ix);
	}
}

static int is_seed(int x, int y) {
	for (int i = 0; i < seed_max; i++) {
		if (cell_at(x, flip(y))->chr == seed[i])
			return 1;
	}
	return 0;
}

static int is_log(int x, int y) {
	return cell_at(x, flip(y))->chr == LOG;
}

static void move_seed(int x, int y) {
	if (cell_at(x, flip(y - 1))->chr != ' ')
		return;
	place_seed(x, y - 1, cell_at(x, flip(y))->chr);
	clear_spot(x, y);
}

static void grow_seed(int x, int y) {
	const int fy = flip(y);
	for (int i = 0; i < seed_max - 1; i++)
		if (seed[i] == cell_at(x, fy)->chr) {
			cell_at(x, fy)->chr = seed[i + 1];
			return;
		}
	place_log(x, y);
}

void update_forest(void* _this) {
	State* this = _this;
	if (!this->initialized) {
		generate(this);
		return;
	}
	for (int x = 0; x < screen_cols(); x++)
		for (int y = screen_rows() - 1; y >= 0; y--)
			if (is_log(x, y - 1) && SDL_rand(100) < SEED_GROW_PROB)
				place_log(x, y);
	for (int x = 0; x < screen_cols(); x++)
		for (int y = 0; y < screen_rows(); y++)
			if (is_seed(x, y))
				move_seed(x, y);
	for (int x = 0; x < screen_cols(); x++)
		for (int y = 0; y < screen_rows(); y++)
			if (is_seed(x, y) && SDL_rand(100) < SEED_GROW_PROB)
				grow_seed(x, y);
}
