#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include <stb_image.h>

#include "clock.h"
#include "fps.h"
#include "log.h"
#include "modes.h"
#include "screen.h"

static HWND worker_window = NULL;
static SDL_Window* sdl_window = NULL;
static SDL_Renderer* sdl_renderer = NULL;
static SDL_Texture* vga_texture = NULL;

static SDL_Color colors[16] = {0};

#define FULL (255)
#define MID (170)
#define LOW (85)
static void colors_init() {
	colors[C_BLACK] = (SDL_Color){0, 0, 0, 255};
	colors[C_GRAY] = (SDL_Color){LOW, LOW, LOW, 255};
	colors[C_WHITE] = (SDL_Color){MID, MID, MID, 255};
	colors[C_BRIGHT_WHITE] = (SDL_Color){FULL, FULL, FULL, 255};

	colors[C_RED] = (SDL_Color){MID, 0, 0, 255};
	colors[C_BRIGHT_RED] = (SDL_Color){FULL, LOW, LOW, 255};

	colors[C_GREEN] = (SDL_Color){0, MID, 0, 255};
	colors[C_BRIGHT_GREEN] = (SDL_Color){LOW, FULL, LOW, 255};

	colors[C_YELLOW] = (SDL_Color){FULL, MID, 0, 255};
	colors[C_BRIGHT_YELLOW] = (SDL_Color){FULL, FULL, LOW, 255};

	colors[C_BLUE] = (SDL_Color){0, 0, MID, 255};
	colors[C_BRIGHT_BLUE] = (SDL_Color){LOW, LOW, FULL, 255};

	colors[C_PURPLE] = (SDL_Color){MID, 0, MID, 255};
	colors[C_BRIGHT_PURPLE] = (SDL_Color){FULL, LOW, FULL, 255};

	colors[C_AQUA] = (SDL_Color){0, MID, MID, 255};
	colors[C_BRIGHT_AQUA] = (SDL_Color){LOW, FULL, FULL, 255};
}
#undef LOW
#undef MID
#undef FULL

static int find_worker(HWND top_handle, __attribute__((unused)) LPARAM top_param) {
	HWND p = FindWindowEx(top_handle, NULL, "SHELLDLL_DefView", NULL);
	if (p != NULL)
		worker_window = FindWindowEx(NULL, top_handle, "WorkerW", NULL);
	return 1;
}

static Cell back_buf[MAX_WIDTH * MAX_HEIGHT];
static SDL_Texture* double_buf = NULL;

void render() {
	assert(double_buf != NULL, "`render` called before front-buffer texture was initialized");
	SDL_SetRenderTarget(sdl_renderer, double_buf);

	for (int y = 0; y < screen_rows(); y++)
		for (int x = 0; x < screen_cols(); x++) {
			const Cell* front = cell_at(x, y);
			Cell* back = cell_at_ex(back_buf, x, y);

			if (!SDL_memcmp(front, back, sizeof(Cell)))
				continue;
			*back = *front;

			SDL_FRect dest;
			dest.x = (float)(x * CHR_WIDTH);
			dest.y = (float)(y * CHR_HEIGHT);
			dest.w = CHR_WIDTH;
			dest.h = CHR_HEIGHT;

			SDL_FRect src;
			src.x = (float)((int)(front->chr % 16) * CHR_WIDTH);
			src.y = (float)((int)(front->chr / 16) * CHR_HEIGHT);
			src.w = CHR_WIDTH;
			src.h = CHR_HEIGHT;

			SDL_Color c = colors[front->bg];
			SDL_SetRenderDrawColor(sdl_renderer, c.r, c.g, c.b, 255);
			SDL_RenderFillRect(sdl_renderer, &dest);

			c = colors[front->fg];
			SDL_SetTextureColorMod(vga_texture, c.r, c.g, c.b);
			SDL_RenderTexture(sdl_renderer, vga_texture, &src, &dest);
		}

	SDL_SetRenderTarget(sdl_renderer, NULL);

	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
	SDL_RenderClear(sdl_renderer);

	SDL_FRect rect = {0.f, 0.f, (float)screen_width(), (float)screen_height()};
	SDL_RenderTexture(sdl_renderer, double_buf, &rect, &rect);

	SDL_RenderPresent(sdl_renderer);
}

static uint64_t fps_counter = 60;
int get_fps() {
	return (int)fps_counter;
}

static int rows = 0, cols = 0;
int main(__attribute__((unused)) int argc, __attribute__((unused)) char* argv[]) {
	colors_init();
	set_mode("pipes");

	assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS), "SDL_Init failed! %s", SDL_GetError());

	HWND progman = FindWindow("Progman", NULL);
	assert(progman != NULL, "Failed to find the Progman window!");

	SendMessage(progman, 0x052C, 0, 0); // !!! undocumented thingy powering this whole ordeal
	EnumWindows(find_worker, 0);
	assert(worker_window != NULL, "Failed to find the Worker window!!!");

	RECT rect;
	GetWindowRect(worker_window, &rect);

	assert(SDL_CreateWindowAndRenderer("dwarfpaper", 1, 1, SDL_WINDOW_FULLSCREEN, &sdl_window, &sdl_renderer),
		"Failed to create the SDL window/renderer!!! %s", SDL_GetError());

	SDL_Rect bounds;
	SDL_DisplayID sdl_display = SDL_GetPrimaryDisplay();
	assert(SDL_GetDisplayBounds(sdl_display, &bounds), "Failed to get display bounds");
	rows = bounds.h / CHR_HEIGHT + 1;
	cols = bounds.w / CHR_WIDTH + 1;

	assert(SDL_SetWindowPosition(sdl_window, 0, 0), "Failed to set the SDL window position! %s", SDL_GetError());
	assert(SDL_SetWindowSize(sdl_window, screen_width(), screen_height()), "Failed to set the SDL window size! %s",
		SDL_GetError());

	HWND main_window
		= SDL_GetPointerProperty(SDL_GetWindowProperties(sdl_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	SetParent(main_window, worker_window);
	ShowWindow(worker_window, 1); // !!! won't do jackshit without this

	int d1, d2, n;
	uint8_t* vga_data = stbi_load("9x16.png", &d1, &d2, &n, 4);
	assert(vga_data != NULL, "Failed to load the VGA 9x16 font PNG");

	SDL_Surface* vga_surface = SDL_CreateSurfaceFrom(144, 256, SDL_PIXELFORMAT_RGBA8888, vga_data, 144 * 4);
	assert(vga_surface != NULL, "Failed to create the VGA 9x16 font surface!!! %s", SDL_GetError());

	vga_texture = SDL_CreateTextureFromSurface(sdl_renderer, vga_surface);
	assert(vga_texture != NULL, "Failed to load the VGA 9x16 font texture!!! %s", SDL_GetError());

	info("Running...");

	uint64_t second = 0, fps = 0;
	const float refresh_rate = SDL_GetDesktopDisplayMode(sdl_display)->refresh_rate;
	const uint64_t target_delta = (uint64_t)(((double)CLOCK_SECOND) / ((double)refresh_rate));

	for (;;) {
		const uint64_t frame_start = SDL_GetTicksNS();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT)
				goto cleanup;
			if (event.type != SDL_EVENT_WINDOW_RESIZED)
				continue;

			assert(SDL_GetDisplayBounds(sdl_display, &bounds), "Failed to get display bounds");
			rows = bounds.h / CHR_HEIGHT + 1;
			cols = bounds.w / CHR_WIDTH + 1;

			double_buf = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
				screen_width(), screen_height());
			assert(double_buf != NULL, "Failed to create the front-buffer texture!!! %s", SDL_GetError());

			for (int i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++) {
				back_buf[i].chr = 0;
				back_buf[i].fg = C_GRAY;
				back_buf[i].bg = C_BLACK;
			}

			mode_redraw();
		}

		mode_tick();

		const uint64_t now = SDL_GetTicksNS();
		uint64_t delta = now - frame_start;
		if (delta < target_delta) {
			SDL_DelayNS(target_delta - delta);
			delta = target_delta;
		}

		fps++;
		second += delta;
		if (second >= CLOCK_SECOND) {
			// Info("FPS: %d", get_fps());
			second -= CLOCK_SECOND;
			fps_counter = fps;
			fps = 0;
		}
	}

cleanup:
	info("Goodbye!");

	SDL_DestroyTexture(double_buf);
	SDL_DestroyTexture(vga_texture);
	SDL_DestroySurface(vga_surface);
	stbi_image_free(vga_data);

	SDL_DestroyWindow(sdl_window);
	SDL_Quit();

	return EXIT_SUCCESS;
}

int screen_cols() {
	return cols;
}

int screen_rows() {
	return rows;
}
