#include <SDL3/SDL_stdinc.h>

#include "clock.h"
#include "screen.h"

#include "modes/pipes.h"

enum {
	DIR_NORTH,
	DIR_EAST,
	DIR_SOUTH,
	DIR_WEST,
};

typedef struct {
	int x, y;
	uint8_t dir : 2, color : 4;
} Pipe;

typedef struct {
	Ticks last_reset;
	uint8_t pipe_count;
} State;

#define TURN_FREQ (20)
#define RESET_SECS (30)

#define MAX_PIPES (13)
#define MIN_PIPES (6)

void draw_pipes(__attribute__((unused)) const void* _this) {}

void update_pipes(void* _this) {
	State* this = _this;
	Pipe* pipes = (Pipe*)(this + 1);

	if ((ticks() - this->last_reset) >= (TICKRATE * RESET_SECS))
		this->pipe_count = 0;
	if (!this->pipe_count) {
		this->last_reset = ticks();
		clear_screen(C_BLACK);

		this->pipe_count = MIN_PIPES + SDL_rand(MAX_PIPES - MIN_PIPES + 1);
		for (int i = 0; i < this->pipe_count; i++) {
			pipes[i].x = SDL_rand(screen_cols());
			pipes[i].y = SDL_rand(screen_rows());
			pipes[i].dir = SDL_rand(4) % 4;
			pipes[i].color = rand_bright();
		}
	}

	for (int i = 0; i < this->pipe_count; i++) {
		Pipe* pipe = &pipes[i];

		pipe->x += (pipe->dir == DIR_EAST) - (pipe->dir == DIR_WEST);
		pipe->y += (pipe->dir == DIR_SOUTH) - (pipe->dir == DIR_NORTH);
		if (pipe->x < 0)
			pipe->x = screen_cols() - 1;
		if (pipe->x >= screen_cols())
			pipe->x = 0;
		if (pipe->y < 0)
			pipe->y = screen_rows() - 1;
		if (pipe->y >= screen_rows())
			pipe->y = 0;

		Cell* cur = cell_at(pipe->x, pipe->y);
		// cool effect: cur->bg = cur->fg;
		cur->fg = pipe->color;

		if (pipe->dir == DIR_NORTH || pipe->dir == DIR_SOUTH)
			cur->chr = (11 * 16) + 10;
		else
			cur->chr = (12 * 16) + 13;

		if (SDL_rand(TURN_FREQ))
			continue;

		int newDir = pipe->dir + 1 - 2 * SDL_rand(2);
		while (newDir >= 3)
			newDir -= 4;
		while (newDir < 0)
			newDir += 4;
		if (pipe->dir == newDir)
			continue;

		if ((pipe->dir == DIR_NORTH && newDir == DIR_WEST) || (pipe->dir == DIR_EAST && newDir == DIR_SOUTH))
			cur->chr = (11 * 16) + 11;
		else if ((pipe->dir == DIR_NORTH && newDir == DIR_EAST)
			 || (pipe->dir == DIR_WEST && newDir == DIR_SOUTH))
			cur->chr = (12 * 16) + 9;
		else if ((pipe->dir == DIR_SOUTH && newDir == DIR_WEST)
			 || (pipe->dir == DIR_EAST && newDir == DIR_NORTH))
			cur->chr = (11 * 16) + 12;
		else if ((pipe->dir == DIR_SOUTH && newDir == DIR_EAST)
			 || (pipe->dir == DIR_WEST && newDir == DIR_NORTH))
			cur->chr = (12 * 16) + 8;
		pipe->dir = newDir;
	}
}
