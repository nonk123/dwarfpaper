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
#include "cmdline.h"
#include "fps.h"
#include "log.h"
#include "modes.h"
#include "screen.h"
#include "vga9x16.h"

static HWND worker_window = NULL;
static SDL_Window* sdl_window = NULL;
static SDL_Renderer* sdl_renderer = NULL;
static SDL_Texture* vga_texture = NULL;

#define FULL (255)
#define MID (170)
#define LOW (85)

static const SDL_Color colors[C_MAX] = {
	[C_BLACK] = {0,    0,    0,    255},
	[C_GRAY] = {LOW,  LOW,  LOW,  255},
	[C_WHITE] = {MID,  MID,  MID,  255},
	[C_BRIGHT_WHITE] = {FULL, FULL, FULL, 255},
	[C_RED] = {MID,  0,    0,    255},
	[C_BRIGHT_RED] = {FULL, LOW,  LOW,  255},
	[C_GREEN] = {0,    MID,  0,    255},
	[C_BRIGHT_GREEN] = {LOW,  FULL, LOW,  255},
	[C_YELLOW] = {FULL, MID,  0,    255},
	[C_BRIGHT_YELLOW] = {FULL, FULL, LOW,  255},
	[C_BLUE] = {0,    0,    MID,  255},
	[C_BRIGHT_BLUE] = {LOW,  LOW,  FULL, 255},
	[C_PURPLE] = {MID,  0,    MID,  255},
	[C_BRIGHT_PURPLE] = {FULL, LOW,  FULL, 255},
	[C_AQUA] = {0,    MID,  MID,  255},
	[C_BRIGHT_AQUA] = {LOW,  FULL, FULL, 255},
};

#undef LOW
#undef MID
#undef FULL

__attribute__((stdcall)) static int find_worker(HWND top_handle, __attribute__((unused)) LPARAM top_param) {
	HWND p = FindWindowEx(top_handle, NULL, "SHELLDLL_DefView", NULL);
	if (p != NULL)
		worker_window = FindWindowEx(NULL, top_handle, "WorkerW", NULL);
	return 1;
}

static Cell back_buf[MAX_WIDTH * MAX_HEIGHT];
static SDL_Texture* canvas = NULL;

static void render_cell(int x, int y) {
	const Cell* front = cell_at(x, y);
	Cell* back = cell_at_ex(back_buf, x, y);

	if (!SDL_memcmp(front, back, sizeof(Cell)))
		return;
	*back = *front;

	SDL_FRect dest = {0};
	dest.x = (float)(x * CHR_WIDTH);
	dest.y = (float)(y * CHR_HEIGHT);
	dest.w = CHR_WIDTH, dest.h = CHR_HEIGHT;

	SDL_FRect src = {0};
	src.x = (float)((int)(front->chr % 16) * CHR_WIDTH);
	src.y = (float)((int)(front->chr / 16) * CHR_HEIGHT);
	src.w = CHR_WIDTH, src.h = CHR_HEIGHT;

	SDL_Color c = colors[front->bg];
	SDL_SetRenderDrawColor(sdl_renderer, c.r, c.g, c.b, 255);
	SDL_RenderFillRect(sdl_renderer, &dest);

	c = colors[front->fg];
	SDL_SetTextureColorMod(vga_texture, c.r, c.g, c.b);
	SDL_RenderTexture(sdl_renderer, vga_texture, &src, &dest);
}

void render() {
	expect(canvas != NULL, "`render` called before front-buffer texture was initialized");
	SDL_SetRenderTarget(sdl_renderer, canvas);

	for (int y = 0; y < screen_rows(); y++)
		for (int x = 0; x < screen_cols(); x++)
			render_cell(x, y);
	SDL_SetRenderTarget(sdl_renderer, NULL);

	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
	SDL_RenderClear(sdl_renderer);

	const SDL_FRect rect = {0.f, 0.f, (float)screen_width(), (float)screen_height()};
	SDL_RenderTexture(sdl_renderer, canvas, &rect, &rect);

	SDL_RenderPresent(sdl_renderer);
}

static uint64_t fps_counter = 60;
int get_fps() {
	return (int)fps_counter;
}

static int rows = 0, cols = 0;
int screen_rows() {
	return rows;
}
int screen_cols() {
	return cols;
}

static uint64_t vga9x16_size = 0;
static uint8_t vga9x16[8 * 1024] = {0};

static void load_9x16_png() {
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(VGA9X16), RT_RCDATA);
	expect(hRes != NULL, "Failed to find the embedded 9x16.png");

	vga9x16_size = SizeofResource(NULL, hRes);
	expect(vga9x16_size, "Failed to identify the size of the embedded 9x16.png");

	HGLOBAL hData = LoadResource(NULL, hRes);
	expect(hData != NULL, "Failed to load the embedded 9x16.png");

	const uint8_t* pData = (uint8_t*)(LockResource(hData));
	expect(pData != NULL, "Failed to lock the embedded 9x16.png data");

	SDL_memcpy(vga9x16, pData, vga9x16_size);
}

static int handle_events(SDL_DisplayID sdl_display) {
	SDL_Rect bounds = {0};
	SDL_Event event = {0};

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT)
			return 0;
		if (args.debug && event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE)
			return 0;
		if (event.type == SDL_EVENT_WINDOW_RESIZED)
			goto resized;
		continue;

	resized:
		expect(SDL_GetDisplayBounds(sdl_display, &bounds), "Failed to get display bounds");
		rows = bounds.h / CHR_HEIGHT + 1, cols = bounds.w / CHR_WIDTH + 1;

		canvas = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
			screen_width(), screen_height());
		expect(canvas != NULL, "Failed to create the front-buffer texture!!! %s", SDL_GetError());

		for (int i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++)
			back_buf[i].chr = 0, back_buf[i].fg = C_GRAY, back_buf[i].bg = C_BLACK;
		mode_redraw();
	}

	return 1;
}

int main(int argc, char* argv[]) {
	parse_cmdline(argc, argv);
	set_mode(args.mode);

	expect(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS), "SDL_Init failed! %s", SDL_GetError());

	const SDL_WindowFlags flags = SDL_WINDOW_FULLSCREEN * !args.debug;
	expect(SDL_CreateWindowAndRenderer("dwarfpaper", 1, 1, flags, &sdl_window, &sdl_renderer),
		"Failed to create the SDL window/renderer!!! %s", SDL_GetError());

	SDL_Rect bounds = {0};
	SDL_DisplayID sdl_display = SDL_GetPrimaryDisplay();
	expect(SDL_GetDisplayBounds(sdl_display, &bounds), "Failed to get display bounds");
	rows = bounds.h / CHR_HEIGHT + 1, cols = bounds.w / CHR_WIDTH + 1;

	expect(SDL_SetWindowPosition(sdl_window, 0, 0), "Failed to set the SDL window position! %s", SDL_GetError());
	expect(SDL_SetWindowSize(sdl_window, screen_width(), screen_height()), "Failed to set the SDL window size! %s",
		SDL_GetError());

	if (!args.debug) {
		HWND progman = FindWindow("Progman", NULL);
		expect(progman != NULL, "Failed to find the Progman window!");

		SendMessage(progman, 0x052C, 0, 0); // !!! undocumented thingy powering this whole ordeal
		EnumWindows(find_worker, 0);
		expect(worker_window != NULL, "Failed to find the Worker window!!!");

		HWND main_window = SDL_GetPointerProperty(
			SDL_GetWindowProperties(sdl_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
		SetParent(main_window, worker_window);
		ShowWindow(worker_window, 1); // !!! won't do jackshit without this
	}

	load_9x16_png();
	int d1, d2, n, ch = 4;
	uint8_t* vga_data = stbi_load_from_memory(vga9x16, (int)vga9x16_size, &d1, &d2, &n, ch);
	expect(vga_data != NULL, "Failed to load the VGA 9x16 font PNG");

	SDL_Surface* vga_surface = SDL_CreateSurfaceFrom(d1, d2, SDL_PIXELFORMAT_RGBA8888, vga_data, ch * d1);
	expect(vga_surface != NULL, "Failed to create the VGA 9x16 font surface!!! %s", SDL_GetError());

	vga_texture = SDL_CreateTextureFromSurface(sdl_renderer, vga_surface);
	expect(vga_texture != NULL, "Failed to load the VGA 9x16 font texture!!! %s", SDL_GetError());

	info("Running...");

	uint64_t second = 0, fps = 0;
	const float refresh_rate = SDL_GetDesktopDisplayMode(sdl_display)->refresh_rate;
	const uint64_t target_delta = (uint64_t)(((double)CLOCK_SECOND) / ((double)refresh_rate));

	for (;;) {
		const uint64_t frame_start = SDL_GetTicksNS();

		if (!handle_events(sdl_display))
			break;
		mode_tick();

		const uint64_t now = SDL_GetTicksNS();
		uint64_t delta = now - frame_start;
		if (delta < target_delta) {
			SDL_DelayNS(target_delta - delta);
			delta = target_delta;
		}

		fps++, second += delta;
		while (second >= CLOCK_SECOND) {
			fps_counter = fps, fps = 0;
			second -= CLOCK_SECOND;
		}
	}

	info("Goodbye!");

	SDL_DestroyTexture(canvas);
	SDL_DestroyTexture(vga_texture);
	SDL_DestroySurface(vga_surface);
	stbi_image_free(vga_data);

	SDL_DestroyWindow(sdl_window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
