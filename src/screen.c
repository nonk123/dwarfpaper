#include "SDL3/SDL_video.h"

#include "log.h"
#include "modes.h"
#include "screen.h"

static int rows = 0, cols = 0;
static struct chr buf[MAX_WIDTH * MAX_HEIGHT];

struct chr* chrAt(int x, int y) {
    static struct chr deflt = {0, C_GRAY, C_BLACK};
    if (x < 0 || y < 0 || x > cols || y > rows)
        return &deflt;
    return &buf[y * cols + x];
}

void syncScreenSize() {
    SDL_Rect bounds;
    Assert(SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &bounds), "Failed to get display bounds");

    const int oldRows = rows, oldCols = cols;
    rows = bounds.h / CHR_HEIGHT + 1;
    cols = bounds.w / CHR_WIDTH + 1;

    if (rows != oldRows || cols != oldCols) {
        for (size_t i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++) {
            buf[i].idx = 0;
            buf[i].fg = C_GRAY;
            buf[i].bg = C_BLACK;
        }
        modeForceRedraw();
    }
}

int scrCols() {
    return cols;
}

int scrRows() {
    return rows;
}

int scrWidth() {
    return cols * CHR_WIDTH;
}

int scrHeight() {
    return rows * CHR_HEIGHT;
}
