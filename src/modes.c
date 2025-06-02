#include <stdlib.h>
#include <string.h>

#include "clock.h"
#include "log.h"
#include "modes.h"
#include "screen.h"

#define REDRAW_DELAY (5 * CLOCK_RES)
extern void paintSDL();

static void drawJumbled() {
    for (int x = 0; x < scrCols(); x++)
        for (int y = 0; y < scrRows(); y++) {
            chrAt(x, y)->idx = 1 + rand() % 255;
            chrAt(x, y)->fg = 1 + rand() % 15;
            chrAt(x, y)->bg = C_BLACK;
        }
}

static struct modeAlist modeAlist[] = {
    [MODE_JUMBLED] = {"jumbled", drawJumbled, NULL},
};

#define CUR_MODE_MAX (256)
static char curMode[CUR_MODE_MAX] = {0};

void setMode(const char* name) {
    strncpy(curMode, name, CUR_MODE_MAX);
}

struct modeAlist* getMode() {
    struct modeAlist *mode = modeAlist, *const end = modeAlist + MODE_MAX;
    while (mode != end && strncmp(mode->name, curMode, CUR_MODE_MAX))
        mode++;
    Assert(mode != end, "Unknown wallpaper mode: %s", curMode);
    return mode;
}

void modeTick() {
    const struct modeAlist* mode = getMode();
    if (mode->tick != NULL)
        mode->tick();

    static instant lastRedraw = -128;
    const instant now = elapsed();
    if (lastRedraw < 0 || (now - lastRedraw) >= REDRAW_DELAY) {
        mode->draw();
        lastRedraw = now;
        if (mode->tick == NULL)
            paintSDL();
    }

    if (mode->tick != NULL)
        paintSDL();
}
