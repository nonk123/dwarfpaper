#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>

#include <stb_image.h>

#include "clock.h"
#include "cmdline.h"
#include "log.h"
#include "vga9x16.h"
#include "window.h"

static int handle_events() {
	SDL_Event event = {0};
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT)
			return 0;
		if (args.debug && event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE)
			return 0;
	}
	return 1;
}

int main(int argc, char* argv[]) {
	parse_cmdline(argc, argv);

	expect(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS), "SDL_Init failed! %s", SDL_GetError());
	spawn_windows();

	info("Running...");

	uint64_t second = 0, fps = 0;
	float max_hz = 0;
	for (const Window* window = windows(); window != NULL; window = window->next)
		if (window->hz > max_hz)
			max_hz = window->hz;
	expect(max_hz, "Seems like all windows failed to create...");

	const uint64_t target_delta = (uint64_t)(CLOCK_SECOND / max_hz);
	for (;;) {
		const uint64_t frame_start = SDL_GetTicksNS();

		if (!handle_events())
			break;
		for (Window* window = windows(); window != NULL; window = window->next)
			tick(window);

		const uint64_t now = SDL_GetTicksNS();
		uint64_t delta = now - frame_start;
		if (delta < target_delta)
			SDL_DelayNS(target_delta - delta);
	}

	info("Goodbye!");

	vga9x16_cleanup();
	teardown_windows();
	SDL_Quit();

	return EXIT_SUCCESS;
}
