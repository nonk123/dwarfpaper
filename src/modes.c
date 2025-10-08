#include <SDL3/SDL_stdinc.h>

#include "colors.h"
#include "modes.h"
#include "screen.h"

#include "modes/pipes.h"

static void draw_jumbled(__attribute__((unused)) const void* _state) {
	for (int x = 0; x < screen_cols(); x++)
		for (int y = 0; y < screen_rows(); y++) {
			cell_at(x, y)->chr = 1 + SDL_rand(256);
			cell_at(x, y)->fg = 1 + SDL_rand(15);
			cell_at(x, y)->bg = C_BLACK;
		}
}

ModeTable modes[] = {
	{"jumbled", draw_jumbled, NULL        },
	{"pipes",   draw_pipes,   update_pipes},
	{NULL,      NULL,         NULL        },
};
