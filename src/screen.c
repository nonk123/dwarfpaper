#include "screen.h"
#include "modes.h"

static int rows = 0, cols = 0;
static struct chr buf[MAX_WIDTH * MAX_HEIGHT];

struct chr* chrAt(int x, int y) {
    static struct chr deflt;
    deflt.idx = 0;
    deflt.fg = C_GRAY;
    deflt.bg = C_BLACK;

    if (x < 0 || y < 0 || x > cols || y > rows)
        return &deflt;
    return &buf[y * cols + x];
}

void scrResize(int windW, int windH) {
    const int oldArea = rows * cols;
    rows = windH / CHR_HEIGHT + 1;
    cols = windW / CHR_WIDTH + 1;

    if (oldArea != rows * cols) {
        for (size_t i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++) {
            buf[i].idx = 1;
            buf[i].fg = C_GRAY;
            buf[i].bg = C_BLACK;
        }
        getMode()->draw();
    }
}

int scrRows() {
    return rows;
}

int scrCols() {
    return cols;
}
