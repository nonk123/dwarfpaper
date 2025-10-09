#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

#include <SDL3/SDL_video.h>

#include "clock.h"
#include "cmdline.h"
#include "colors.h"
#include "log.h"
#include "modes.h"
#include "vga9x16.h"
#include "window.h"

static Window* root = NULL;
static HWND worker_window = NULL;

static void resize(Window*, int, int);

static void spawn_window(HWND worker_window, SDL_DisplayID display) {
	expect(display != 0, "Tried spawning a window on top of an invalid display");

	Window* this = NULL;
	if (root == NULL) {
		root = this = SDL_malloc(sizeof(Window));
		expect(this, "Failed to allocate a window");
		SDL_memset(this, 0, sizeof(*this));
	} else {
		this = SDL_malloc(sizeof(Window));
		expect(this, "Failed to allocate a window");
		SDL_memset(this, 0, sizeof(*this));
		this->next = root;
		root = this;
	}

	this->display = display;
	this->last_render = elapsed() - CLOCK_SECOND, this->last_tick = this->last_render;
	SDL_strlcpy(this->mode, args.mode, sizeof(this->mode));

	expect(SDL_CreateWindowAndRenderer("dwarfpaper", 1, 1, 0, &this->sdl_window, &this->renderer),
		"Failed to create the SDL window+renderer!!! %s", SDL_GetError());

	SDL_Rect bounds = {0};
	SDL_GetDisplayBounds(display, &bounds);
	expect(SDL_SetWindowPosition(this->sdl_window, bounds.x, bounds.y), "Failed to set the SDL window position! %s",
		SDL_GetError());
	expect(SDL_SetWindowSize(this->sdl_window, bounds.w, bounds.h), "Failed to set the SDL window size! %s",
		SDL_GetError());

	const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(display);
	expect(mode, "Failed to fetch display refresh rate");
	this->hz = mode->refresh_rate;

	this->font = SDL_CreateTextureFromSurface(this->renderer, vga9x16());
	expect(this->font, "Failed to load font PNG into a texture");

	if (worker_window != NULL) {
		HWND w32_window = SDL_GetPointerProperty(
			SDL_GetWindowProperties(this->sdl_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
		SetParent(w32_window, worker_window);
		ShowWindow(worker_window, 1); // !!! won't do jackshit without this
	}

	set_window_mode(this, args.mode);
	resize(this, bounds.w, bounds.h);
}

__attribute__((stdcall)) static int find_worker(HWND top_handle, __attribute__((unused)) LPARAM top_param) {
	HWND p = FindWindowEx(top_handle, NULL, "SHELLDLL_DefView", NULL);
	if (p != NULL)
		worker_window = FindWindowEx(NULL, top_handle, "WorkerW", NULL);
	return 1;
}

void spawn_windows() {
	if (args.debug) {
		spawn_window(NULL, SDL_GetPrimaryDisplay());
		return;
	}

	HWND progman = FindWindow("Progman", NULL);
	expect(progman != NULL, "Failed to find the Progman window!");

	SendMessage(progman, 0x052C, 0, 0); // !!! undocumented thingy powering this whole ordeal
	EnumWindows(find_worker, 0);
	expect(worker_window != NULL, "Failed to find the Worker window!!!");

	for (SDL_DisplayID* display = SDL_GetDisplays(NULL); display && *display; display++)
		spawn_window(worker_window, *display);
}

static void teardown_window(Window* this) {
	if (this == NULL)
		return;
	teardown_window(this->next);

	SDL_DestroyTexture(this->font);
	SDL_DestroyTexture(this->canvas);
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->sdl_window);
}

void teardown_windows() {
	teardown_window(root);
}

Window* windows() {
	return root;
}

static void render_cell(Window* this, int x, int y) {
	const Cell* front = cell_at(x, y);
	Cell* back = cell_at_ex(this->back, x, y);

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
	SDL_SetRenderDrawColor(this->renderer, c.r, c.g, c.b, 255);
	SDL_RenderFillRect(this->renderer, &dest);

	c = colors[front->fg];
	SDL_SetTextureColorMod(this->font, c.r, c.g, c.b);
	SDL_RenderTexture(this->renderer, this->font, &src, &dest);
}

extern void set_active_window(Window*);
static void maybe_render(Window* this, Instant now) {
	if (now - this->last_render < (Instant)(CLOCK_SECOND / this->hz))
		return;
	this->last_render = now;

	SDL_Rect bounds = {0};
	expect(SDL_GetDisplayBounds(this->display, &bounds), "Failed to get display bounds");
	resize(this, bounds.w, bounds.h);

	expect(this->canvas, "`render` called before front-buffer texture was initialized");
	SDL_SetRenderTarget(this->renderer, this->canvas);

	for (int y = 0; y < screen_rows(); y++)
		for (int x = 0; x < screen_cols(); x++)
			render_cell(this, x, y);
	SDL_SetRenderTarget(this->renderer, NULL);

	SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
	SDL_RenderClear(this->renderer);

	const SDL_FRect rect = {0.f, 0.f, (float)screen_width(), (float)screen_height()};
	SDL_RenderTexture(this->renderer, this->canvas, &rect, &rect);

	SDL_RenderPresent(this->renderer);
}

ModeTable* window_mode(Window* this) {
	for (ModeTable* ptr = modes; ptr->name != NULL; ptr++)
		if (!SDL_strncmp(ptr->name, this->mode, sizeof(this->mode)))
			return ptr;
	fatal("Unknown wallpaper mode: %s", this->mode);
}

static void maybe_tick(Window* this, Instant now) {
	if (now - this->last_tick < (Instant)(CLOCK_SECOND / TICKRATE))
		return;
	this->last_tick = now;
	this->ticks++;

	const ModeTable* mode = window_mode(this);
	if (mode->update != NULL)
		mode->update(this->state);
	mode->draw(this->state);
}

static void force_redraw(Window* this) {
	set_active_window(this);
	window_mode(this)->draw(this->state);
}

void tick(Window* this) {
	const Instant now = elapsed();
	set_active_window(this);
	maybe_tick(this, now);
	maybe_render(this, now);
}

void set_window_mode(Window* this, const char* mode) {
	SDL_strlcpy(this->mode, mode, sizeof(this->mode));
	SDL_memset(this->state, 0, sizeof(this->state));
}

static void resize(Window* this, int new_w, int new_h) {
	if (this->width == new_w && this->height == new_h)
		return;
	this->width = new_w, this->height = new_h;

	this->canvas
		= SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, new_w, new_h);
	expect(this->canvas != NULL, "Failed to create the front-buffer texture!!! %s", SDL_GetError());

	for (int i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++) {
		Cell* cell = &this->back[i];
		cell->chr = 0, cell->fg = C_GRAY, cell->bg = C_BLACK;
	}

	force_redraw(this);
}
