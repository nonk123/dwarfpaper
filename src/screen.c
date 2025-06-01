#include "screen.h"

#define MAX_WIDTH (420)
#define MAX_HEIGHT (120)

static int rows, cols;
static struct chr buf[MAX_WIDTH * MAX_HEIGHT];

struct chr* chrAt(int x, int y) {
    static struct chr deflt;
    if (x < 0 || y < 0 || x > cols || y > rows)
        return &deflt;
    return &buf[y * cols + x];
}

void scrResize(int windW, int windH) {
    rows = windH / CHR_HEIGHT + 1;
    cols = windW / CHR_WIDTH + 1;
}

int scrRows() {
    return rows;
}

int scrCols() {
    return cols;
}
