#include "screen.h"

static struct chr buf[MAX_WIDTH * MAX_HEIGHT];

struct chr* chrAt(int x, int y) {
    static struct chr deflt = {0, C_GRAY, C_BLACK};
    if (x < 0 || y < 0 || x > scrCols() || y > scrRows())
        return &deflt;
    return &buf[y * scrCols() + x];
}

int scrWidth() {
    return scrCols() * CHR_WIDTH;
}

int scrHeight() {
    return scrRows() * CHR_HEIGHT;
}
