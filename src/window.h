#pragma once

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include "clock.h"
#include "screen.h"

typedef struct Window {
	char mode[128], state[4096];
	Cell front[CELL_COUNT], back[CELL_COUNT];
	struct Window* next;
	SDL_Window* sdl_window;
	SDL_Renderer* renderer;
	SDL_Texture *canvas, *font;
	SDL_DisplayID display;
	Instant last_render, last_tick;
	int width, height;
	float hz;
} Window;

void spawn_windows(), teardown_windows();
void set_mode(Window*, const char*), tick(Window*);
Window* windows();
