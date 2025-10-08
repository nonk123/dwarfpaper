#include "screen.h"

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

int screen_width() {
	return screen_cols() * CHR_WIDTH;
}

int screen_height() {
	return screen_rows() * CHR_HEIGHT;
}
