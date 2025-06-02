#pragma once

typedef void (*drawFn)(const void*), (*tickFn)(void*);
#define MODE_STATE_SIZE (4096)

struct modeAlist {
    const char* name;
    drawFn draw;
    tickFn tick;
};

enum {
    MODE_JUMBLED,
    MODE_PIPES,
    MODE_MAX,
};

void setMode(const char*);
void modeTick(), modeForceRedraw();
