#include <stdlib.h> // for `EXIT_SUCCESS`

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>

#include "clock.h"
#include "cmdline.h"
#include "log.h"
#include "vga9x16.h"
#include "window.h"

static void toggle_modes() {
	for (Window* window = windows(); window != NULL; window = window->next) {
		const ModeTable* mode = window_mode(window) + 1;
		if (mode->name == NULL)
			mode = modes;
		set_window_mode(window, mode->name);
	}
}

static void reset_modes() {
	for (Window* window = windows(); window != NULL; window = window->next)
		set_window_mode(window, window_mode(window)->name);
}

static int handle_events() {
	SDL_Event event = {0};
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT)
			return 0;
		if (!args.debug || event.type != SDL_EVENT_KEY_DOWN)
			continue;
		if (event.key.scancode == SDL_SCANCODE_ESCAPE)
			return 0;
		else if (event.key.scancode == SDL_SCANCODE_SPACE)
			toggle_modes();
		else if (event.key.scancode == SDL_SCANCODE_R)
			reset_modes();
	}
	return 1;
}

static uint64_t min_delta() {
	float hz = 0.f;
	for (const Window* window = windows(); window != NULL; window = window->next)
		hz = (window->hz > hz ? window->hz : hz);
	expect(hz != 0.f, "Seems like no windows were created...");
	return (uint64_t)(((float)CLOCK_SECOND) / hz);
}

int main_fr() {
	expect(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS), "SDL_Init failed! %s", SDL_GetError());
	spawn_windows();
	const uint64_t target_delta = min_delta();

	info("Running...");
	for (;;) {
		const uint64_t frame_start = SDL_GetTicksNS();
		if (!handle_events())
			break;
		for (Window* window = windows(); window != NULL; window = window->next)
			tick(window);

		const uint64_t now = SDL_GetTicksNS(), delta = now - frame_start;
		if (delta < target_delta)
			SDL_DelayNS(target_delta - delta);
	}
	info("Goodbye!");

	vga9x16_cleanup();
	teardown_windows();
	SDL_Quit();

	return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
	parse_cmdline(argc, argv);
	return main_fr(); // don't want the argc/argv in my scope :angry:
}
