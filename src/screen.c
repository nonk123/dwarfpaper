#include "screen.h"
#include "colors.h"
#include "log.h"
#include "window.h"

static Window* active_window = NULL;

void set_active_window(Window* window) {
	active_window = window;
}

static void expect_window() {
	expect(active_window, "No window is active");
}

Cell* cell_at_ex(Cell* ptr, int x, int y) {
	static Cell deflt = {0, C_GRAY, C_BLACK};
	if (x < 0 || y < 0 || x > screen_cols() || y > screen_rows())
		return &deflt;
	return &ptr[y * screen_cols() + x];
}

Cell* cell_at(int x, int y) {
	expect_window();
	return cell_at_ex(active_window->front, x, y);
}

int screen_rows() {
	return screen_height() / CHR_HEIGHT + 1;
}

int screen_cols() {
	return screen_width() / CHR_WIDTH + 1;
}

int screen_width() {
	expect_window();
	return active_window->width;
}

int screen_height() {
	expect_window();
	return active_window->height;
}

Ticks ticks() {
	expect_window();
	return active_window->ticks;
}
