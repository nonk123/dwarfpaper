#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

#include <SDL3/SDL_video.h>

#include "clock.h"
#include "cmdline.h"
#include "colors.h"
#include "log.h"
#include "vga9x16.h"
#include "window.h"

#include "modes.h"

static Window* root = NULL;
static HWND worker_window = NULL;

static void maybe_resize(Window*, int, int);
extern void set_active_window(Window*);

static void spawn_window(HWND worker_window, SDL_DisplayID display) {
	expect(display != 0, "Tried spawning a window on top of an invalid display");

	Window* this = NULL;
	if (root) {
		this = SDL_malloc(sizeof(Window));
		expect(this, "Failed to allocate a window");
		SDL_memset(this, 0, sizeof(*this));
		this->next = root;
		root = this;
	} else {

		root = this = SDL_malloc(sizeof(Window));
		expect(this, "Failed to allocate a window");
		SDL_memset(this, 0, sizeof(*this));
	}

	this->display = display, this->last_rendered = elapsed() - CLOCK_SECOND;
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

	if (worker_window) {
		SDL_PropertiesID props = SDL_GetWindowProperties(this->sdl_window);
		HWND w32_window = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
		SetParent(w32_window, worker_window);
		ShowWindow(worker_window, 1); // !!! won't do jackshit without this
	}

	set_window_mode(this, args.mode);
	maybe_resize(this, bounds.w, bounds.h); // the display size needs to be known before the first update
}

__attribute__((stdcall)) static int find_worker(HWND top_handle, __attribute__((unused)) LPARAM top_param) {
	HWND p = FindWindowEx(top_handle, NULL, "SHELLDLL_DefView", NULL);
	if (p)
		worker_window = FindWindowEx(NULL, top_handle, "WorkerW", NULL);
	return 1;
}

void spawn_windows() {
	if (args.debug) {
		spawn_window(NULL, SDL_GetPrimaryDisplay());
		return;
	}

	HWND progman = FindWindow("Progman", NULL);
	expect(progman, "Failed to find the Progman window!");

	SendMessage(progman, 0x052C, 0, 0); // !!! undocumented thingy powering this whole ordeal
	EnumWindows(find_worker, 0);
	expect(worker_window, "Failed to find the Worker window!!!");

	for (SDL_DisplayID* display = SDL_GetDisplays(NULL); display && *display; display++)
		spawn_window(worker_window, *display);
}

static void teardown_window(Window* this) {
	if (!this)
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

	SDL_FRect src = {0}, dest = {0};
	dest.x = (float)(x * CHR_WIDTH), dest.y = (float)(y * CHR_HEIGHT);
	src.x = (float)((int)(front->chr % 16) * CHR_WIDTH);
	src.y = (float)((int)(front->chr / 16) * CHR_HEIGHT);
	src.w = dest.w = CHR_WIDTH, src.h = dest.h = CHR_HEIGHT;

	SDL_Color c = colors[front->bg];
	SDL_SetRenderDrawColor(this->renderer, c.r, c.g, c.b, 255);
	SDL_RenderFillRect(this->renderer, &dest);

	c = colors[front->fg];
	SDL_SetTextureColorMod(this->font, c.r, c.g, c.b);
	SDL_RenderTexture(this->renderer, this->font, &src, &dest);
}

static void render(Window* this) {
	this->last_rendered = elapsed();
	set_active_window(this);

	SDL_Rect bounds = {0};
	expect(SDL_GetDisplayBounds(this->display, &bounds), "Failed to get display bounds");
	maybe_resize(this, bounds.w, bounds.h);

	expect(this->canvas, "`render` called before front-buffer texture was initialized");
	SDL_SetRenderTarget(this->renderer, this->canvas);

	for (int y = 0; y < screen_rows(); y++)
		for (int x = 0; x < screen_cols(); x++)
			render_cell(this, x, y);

	SDL_SetRenderTarget(this->renderer, NULL);
	SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
	SDL_RenderClear(this->renderer);
	SDL_RenderTexture(this->renderer, this->canvas, NULL, NULL);
	SDL_RenderPresent(this->renderer);
}

const ModeTable* window_mode(Window* this) {
	for (const ModeTable* ptr = modes; ptr->name; ptr++)
		if (!SDL_strncmp(ptr->name, this->mode, sizeof(this->mode)))
			return ptr;
	fatal("Unknown wallpaper mode: %s", this->mode);
}

static void force_update(Window* this) {
	set_active_window(this), this->ticks++;
	const ModeTable* mode = window_mode(this);
	if (mode && mode->update)
		mode->update(this->state);
	if (mode && mode->draw)
		mode->draw(this->state);
}

static void maybe_update(Window* this) {
	const uint64_t targetTicks = ((elapsed() - this->last_reset) * TICKRATE) / CLOCK_SECOND;
	while (this->ticks < targetTicks)
		force_update(this);
}

static void force_redraw(Window* this) {
	set_active_window(this);
	for (int i = 0; i < CELL_COUNT; i++) {
		Cell* cell = &this->back[i];
		cell->chr = 0, cell->fg = C_GRAY, cell->bg = C_BLACK;
	}
	force_update(this);
}

static void reset_window_mode(Window* this) {
	SDL_memset(this->state, 0, sizeof(this->state));
	this->last_reset = elapsed(), this->ticks = 0;
}

void tick(Window* this) {
	const Instant reset_interval = CLOCK_SECOND * window_mode(this)->reset_secs,
		      render_interval = CLOCK_SECOND / this->hz;
	if (elapsed() - this->last_reset >= reset_interval) {
		reset_window_mode(this);
		force_redraw(this);
	} else
		maybe_update(this);
	if (elapsed() - this->last_rendered >= render_interval)
		render(this);
}

void set_window_mode(Window* this, const char* mode) {
	SDL_strlcpy(this->mode, mode, sizeof(this->mode));
	reset_window_mode(this);
}

static void maybe_resize(Window* this, int new_w, int new_h) {
	if (this->width == new_w && this->height == new_h)
		return;
	this->width = new_w, this->height = new_h;

	this->canvas
		= SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, new_w, new_h);
	expect(this->canvas, "Failed to create the front-buffer texture!!! %s", SDL_GetError());

	force_redraw(this);
}
