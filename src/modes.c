#include <SDL3/SDL_stdinc.h>

#include "log.h"
#include "modes.h"
#include "screen.h"

#include "mode/pipes.h"

extern void render();

static void draw_jumbled(__attribute__((unused)) const void* _state) {
	for (int x = 0; x < screen_cols(); x++)
		for (int y = 0; y < screen_rows(); y++) {
			cell_at(x, y)->chr = 1 + SDL_rand(255);
			cell_at(x, y)->fg = 1 + SDL_rand(15);
			cell_at(x, y)->bg = C_BLACK;
		}
}

static mode_table modes[] = {
	{"jumbled", draw_jumbled, NULL        },
	{"pipes",   draw_pipes,   update_pipes},
};

static char cur_mode[256] = {0};
static uint8_t mode_state[MODE_STATE_SIZE] = {0};

void set_mode(const char* name) {
	SDL_strlcpy(cur_mode, name, sizeof(cur_mode));
	SDL_memset(mode_state, 0, sizeof(mode_state));
}

static mode_table* get_mode() {
	for (int i = 0; i < sizeof(modes) / sizeof(*modes); i++) {
		mode_table* mode = &modes[i];
		if (!SDL_strncmp(mode->name, cur_mode, sizeof(cur_mode)))
			return mode;
	}
	fatal("Unknown wallpaper mode: %s", cur_mode);
}

void mode_tick() {
	const mode_table* mode = get_mode();
	mode->draw(&mode_state);
	if (mode->update != NULL)
		mode->update(&mode_state);
	render();
}

void mode_redraw() {
	get_mode()->draw(&mode_state);
}
