#pragma once

typedef void (*drawFn)(), (*tickFn)();

struct modeAlist {
    const char* name;
    drawFn draw;
    tickFn tick;
};

enum {
    MODE_JUMBLED,
    MODE_MAX,
};

void setMode(const char*);
struct modeAlist* getMode();
void modeTick();
