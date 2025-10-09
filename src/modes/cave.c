#include "cave.h"
#include "clock.h"
#include "screen.h"

#include "modes/cave.h"

typedef struct {
	Ticks last_reset;
	uint8_t initialized : 1;
} State;

#define ITERS (7)
#define TILE_PROB (47)
#define RESET_SECS (3)
#define STRIDE (2) // draw tiles 2-wide for a squarer look

static int visible_cols() {
	return screen_cols() / STRIDE;
}

static int is_wall(int x, int y) {
	x *= STRIDE;
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

static void place(int x, int y, Cell cell) {
	for (int dx = 0; dx < STRIDE; dx++)
		*cell_at(x * STRIDE + dx, y) = cell;
}

static void place_wall(int x, int y) {
	place(x, y, (Cell){.chr = '#', .fg = C_GRAY, .bg = C_GRAY});
}

static void place_floor(int x, int y) {
	place(x, y, (Cell){.chr = '.', .fg = C_GRAY, .bg = C_BLACK});
}

static void iterate() {
	uint8_t walls[CELL_COUNT] = {0};
	for (int y = 0; y < screen_rows(); y++)
		for (int x = 0; x < visible_cols(); x++)
			walls[y * visible_cols() + x] = count_adjacent(x, y) >= 5;
	for (int y = 0; y < screen_rows(); y++)
		for (int x = 0; x < visible_cols(); x++)
			if (walls[y * visible_cols() + x])
				place_wall(x, y);
			else
				place_floor(x, y);
}

static void generate(State* this) {
	this->initialized = 1;
	this->last_reset = ticks();
	clear_screen(C_BLACK);

	for (int y = 0; y < screen_rows(); y++)
		for (int x = 0; x < visible_cols(); x++)
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
