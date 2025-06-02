#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "modes.h"
#include "screen.h"

#include "mode/pipes.h"

extern void paintSDL();

static void drawJumbled(const void* _state) {
    for (int x = 0; x < scrCols(); x++)
        for (int y = 0; y < scrRows(); y++) {
            chrAt(x, y)->idx = 1 + rand() % 255;
            chrAt(x, y)->fg = 1 + rand() % 15;
            chrAt(x, y)->bg = C_BLACK;
        }
}

static struct modeAlist modeAlist[] = {
    [MODE_JUMBLED] = {"jumbled", drawJumbled, NULL},
    [MODE_PIPES] = {"pipes", drawPipes, tickPipes},
};

#define CUR_MODE_MAX (256)
static char curMode[CUR_MODE_MAX] = {0};
static uint8_t modeState[MODE_STATE_SIZE] = {0};

void setMode(const char* name) {
    strncpy(curMode, name, CUR_MODE_MAX);
    memset(modeState, 0, MODE_STATE_SIZE);
}

static struct modeAlist* getMode() {
    struct modeAlist *mode = modeAlist, *const end = modeAlist + MODE_MAX;
    while (mode != end && strncmp(mode->name, curMode, CUR_MODE_MAX))
        mode++;
    Assert(mode != end, "Unknown wallpaper mode: %s", curMode);
    return mode;
}

void modeTick() {
    const struct modeAlist* mode = getMode();
    mode->draw(&modeState);
    if (mode->tick != NULL)
        mode->tick(&modeState);
    paintSDL();
}

void modeForceRedraw() {
    getMode()->draw(&modeState);
}
