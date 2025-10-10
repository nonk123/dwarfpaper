#include "cave.h"
#include "clock.h"
#include "screen.h"

#include "modes/cave.h"

typedef struct {
	Ticks last_reset;
	uint8_t guy_count;
} State;

typedef struct {
	int x, y;
	int8_t dir, dist;
	enum Color color;
} Guy;

#define STRIDE (2) // draw 2-wide tiles for a squarer look
#define RESET_SECS (10)

#define TILE_PROB (47)
#define ITERS (7)

#define MIN_GUYS (30)
#define MAX_GUYS (50)

#define MIN_DIST (1)
#define DIST_VARIANCE (4)

#define GUY_MOVE_PROB (8)
#define GUY (2)

static int visible_cols() {
	return screen_cols() / STRIDE;
}

static int is_wall(int x, int y) {
	x *= STRIDE;
	const int oob = x < 0 || y < 0 || x >= screen_cols() || y >= screen_rows();
	return oob || cell_at(x, y)->chr == '#';
}

static int is_floor(int x, int y) {
	return cell_at(x * STRIDE, y)->chr == '.';
}

static int is_guy(int x, int y) {
	return cell_at(x * STRIDE, y)->chr == GUY;
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

static void place_guy(int x, int y, enum Color color) {
	*cell_at(x * STRIDE, y) = (Cell){.chr = GUY, .fg = color, .bg = C_BLACK};
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
	this->last_reset = ticks();
	clear_screen(C_BLACK);

	for (int y = 0; y < screen_rows(); y++)
		for (int x = 0; x < visible_cols(); x++)
			if (SDL_rand(100) < TILE_PROB || !x || !y || x == visible_cols() - 1 || y == screen_rows() - 1)
				place_wall(x, y);
			else
				place_floor(x, y);
	for (int i = 0; i < ITERS; i++)
		iterate();

	Guy* guys = (Guy*)(this + 1);
	this->guy_count = MIN_GUYS + SDL_rand(MAX_GUYS - MIN_GUYS + 1);
	int remaining = this->guy_count;
	while (remaining > 0) {
		const int x = SDL_rand(visible_cols()), y = SDL_rand(screen_rows());
		if (!is_floor(x, y))
			continue;
		Guy* guy = &guys[--remaining];
		guy->x = x, guy->y = y, guy->color = rand_bright();
		guy->dir = guy->dist = 0;
		place_guy(x, y, guy->color);
	}
}

void reroll_direction(Guy* this) {
	this->dir = (int8_t)(this->dir + SDL_rand(3) - 1);
	while (this->dir < 0)
		this->dir += 8;
	while (this->dir > 7)
		this->dir -= 8;
}

static const int8_t dir_x[8] = {1, 1, 0, -1, -1, -1, 0, 1}, dir_y[8] = {0, -1, -1, -1, 0, 1, 1, 1};
static void move_guy(Guy* this) {
	for (int max_tries = 10; this->dist && max_tries > 0; max_tries--) {
		const int8_t dx = dir_x[this->dir], dy = dir_y[this->dir];
		if (!is_floor(this->x + dx, this->y + dy)) {
			reroll_direction(this);
			continue;
		}
		// Immediate placement gives an advantage to the guys who are processed first. Just saying.
		place_floor(this->x, this->y);
		this->x += dx, this->y += dy, this->dist--;
		place_guy(this->x, this->y, this->color);
		break;
	}
}

void update_cave(void* _this) {
	State* this = _this;
	if (!this->guy_count || (ticks() - this->last_reset) >= (TICKRATE * RESET_SECS)) {
		generate(this);
		return; // no guy movement for the first tick
	}
	Guy* guys = (Guy*)(this + 1);
	for (Guy* guy = guys; guy < guys + this->guy_count; guy++) {
		move_guy(guy);
		if (guy->dist || SDL_rand(100) >= GUY_MOVE_PROB)
			continue;
		guy->dist = MIN_DIST + SDL_rand(DIST_VARIANCE);
		reroll_direction(guy);
	}
}
