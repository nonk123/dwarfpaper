#include "cave.h"
#include "clock.h"
#include "screen.h"

typedef struct {
	Ticks last_reset;
	uint8_t initialized : 1;
} State;

#define ITERS (8)
#define TILE_PROB (50)
#define RESET_SECS (3)

static int is_wall(int x, int y) {
	const int oob = x < 0 || y < 0 || x >= screen_cols() || y >= screen_rows();
	return oob || cell_at(x, y)->chr == '#';
}

static int count_adjacent(int x, int y) {
	int count = 0;
	for (int dx = -1; dx <= 1; dx++)
		for (int dy = -1; dy <= 1; dy++)
			count += is_wall(x + dx, y + dy);
	return count;
}

static void place_wall(int x, int y) {
	cell_at(x, y)->chr = '#';
	cell_at(x, y)->bg = cell_at(x, y)->fg = C_GRAY;
}

static void place_floor(int x, int y) {
	cell_at(x, y)->chr = '.';
	cell_at(x, y)->fg = C_GRAY;
	cell_at(x, y)->bg = C_BLACK;
}

static void iterate() {
	uint8_t walls[CELL_COUNT] = {0};
	for (int y = 0; y < screen_rows(); y++)
		for (int x = 0; x < screen_cols(); x++)
			walls[y * screen_cols() + x] = count_adjacent(x, y) >= 5;
	for (int i = 0; i < CELL_COUNT; i++)
		if (walls[i])
			place_wall(i % screen_cols(), i / screen_cols());
		else
			place_floor(i % screen_cols(), i / screen_cols());
}

static void generate(State* this) {
	this->initialized = 1;
	this->last_reset = ticks();
	clear_screen(C_BLACK);

	for (int y = 0; y < screen_rows(); y++)
		for (int x = 0; x < screen_cols(); x++)
			if (SDL_rand(100) < TILE_PROB)
				place_wall(x, y);
			else
				place_floor(x, y);
	for (int i = 0; i < ITERS; i++)
		iterate();
}

void draw_cave(__attribute__((unused)) const void* _this) {}

void update_cave(void* _this) {
	State* this = _this;
	if (!this->initialized || (ticks() - this->last_reset) >= (TICKRATE * RESET_SECS))
		generate(this);
}
