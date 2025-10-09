#pragma once

#include <SDL3/SDL_video.h>

// Straight outta <https://github.com/nonk123/micraneft2>:
enum Color {
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
	C_MAX,
};

extern const SDL_Color colors[C_MAX];
