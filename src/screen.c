#include "screen.h"
#include "colors.h"
#include "log.h"
#include "window.h"

static Cell buf[MAX_WIDTH * MAX_HEIGHT];

Cell* cell_at_ex(Cell* ptr, int x, int y) {
	static Cell deflt = {0, C_GRAY, C_BLACK};
	if (x < 0 || y < 0 || x > screen_cols() || y > screen_rows())
		return &deflt;
	return &ptr[y * screen_cols() + x];
}

Cell* cell_at(int x, int y) {
	return cell_at_ex(buf, x, y);
}

static Window* target_window = NULL;

void set_active_window(Window* window) {
	target_window = window;
}

int screen_rows() {
	return screen_height() / CHR_HEIGHT + 1;
}

int screen_cols() {
	return screen_width() / CHR_WIDTH + 1;
}

int screen_width() {
	expect(target_window, "No window is active");
	return target_window->width;
}

int screen_height() {
	expect(target_window, "No window is active");
	return target_window->height;
}
