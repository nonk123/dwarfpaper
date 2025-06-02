#include <stdint.h>
#include <stdlib.h>

#include "clock.h"
#include "screen.h"

#include "mode/pipes.h"

enum {
    DIR_NORTH,
    DIR_EAST,
    DIR_SOUTH,
    DIR_WEST,
};

struct pipe {
    int x, y;
    uint8_t active : 1;
    uint8_t dir : 2;
    uint8_t color : 4;
};

#define MAX_PIPES (64)
#define PIPE_ACTIVE_FREQ (6)
struct state {
    struct pipe pipes[MAX_PIPES];
    instant moveTimer, resetTimer;
    uint8_t init : 1;
};

static void clear() {
    for (int y = 0; y < scrRows(); y++)
        for (int x = 0; x < scrCols(); x++) {
            chrAt(x, y)->idx = 0;
            chrAt(x, y)->fg = C_BLACK;
            chrAt(x, y)->bg = C_BLACK;
        }
}

void drawPipes(const void* _state) {
    const struct state* state = _state;
    if (!state->init)
        clear();
}

#define MOVE_RATE (5)
#define TURN_FREQ (20)
#define RESET_SECS (30)

void tickPipes(void* _state) {
    struct state* state = _state;
    const instant now = elapsed();

    if (!state->init || (now - state->resetTimer >= CLOCK_RES * RESET_SECS)) {
        state->resetTimer = now;
        clear();

        for (size_t i = 0; i < MAX_PIPES; i++) {
            struct pipe* pipe = &state->pipes[i];
            pipe->x = rand() % scrCols();
            pipe->y = rand() % scrRows();
            pipe->active = !(rand() % PIPE_ACTIVE_FREQ);
            pipe->dir = rand() % 4;
            pipe->color = 9 + rand() % 7;
        }
    }
    if (!state->init) {
        state->moveTimer = now;
        state->init = 1;
    }

    if ((now - state->moveTimer) >= (CLOCK_RES / MOVE_RATE)) {
        state->moveTimer = now;
        return;
    }

    for (size_t i = 0; i < MAX_PIPES; i++) {
        struct pipe* pipe = &state->pipes[i];
        if (!pipe->active)
            continue;

        pipe->x += (pipe->dir == DIR_EAST) - (pipe->dir == DIR_WEST);
        pipe->y += (pipe->dir == DIR_SOUTH) - (pipe->dir == DIR_NORTH);
        if (pipe->x < 0)
            pipe->x = scrCols() - 1;
        if (pipe->x >= scrCols())
            pipe->x = 0;
        if (pipe->y < 0)
            pipe->y = scrRows() - 1;
        if (pipe->y >= scrRows())
            pipe->y = 0;

        struct chr* cur = chrAt(pipe->x, pipe->y);
        // cool effect: cur->bg = cur->fg;
        cur->fg = pipe->color;

        if (pipe->dir == DIR_NORTH || pipe->dir == DIR_SOUTH)
            cur->idx = (11 * 16) + 10;
        else
            cur->idx = (12 * 16) + 13;

        if (!(rand() % TURN_FREQ)) {
            int newDir = pipe->dir + 1 - 2 * (rand() % 2);
            while (newDir >= 3)
                newDir -= 4;
            while (newDir < 0)
                newDir += 4;
            if (pipe->dir == newDir)
                continue;

            if (pipe->dir == DIR_NORTH && newDir == DIR_WEST)
                cur->idx = (11 * 16) + 11;
            else if (pipe->dir == DIR_NORTH && newDir == DIR_EAST)
                cur->idx = (12 * 16) + 9;
            else if (pipe->dir == DIR_SOUTH && newDir == DIR_WEST)
                cur->idx = (11 * 16) + 12;
            else if (pipe->dir == DIR_SOUTH && newDir == DIR_EAST)
                cur->idx = (12 * 16) + 8;
            else if (pipe->dir == DIR_WEST && newDir == DIR_NORTH)
                cur->idx = (12 * 16) + 8;
            else if (pipe->dir == DIR_WEST && newDir == DIR_SOUTH)
                cur->idx = (12 * 16) + 9;
            else if (pipe->dir == DIR_EAST && newDir == DIR_NORTH)
                cur->idx = (11 * 16) + 12;
            else if (pipe->dir == DIR_EAST && newDir == DIR_SOUTH)
                cur->idx = (11 * 16) + 11;
            ;

            pipe->dir = newDir;
        }
    }
}
