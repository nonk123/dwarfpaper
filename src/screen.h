#pragma once

#include <stdint.h>

#define CHR_WIDTH (9)
#define CHR_HEIGHT (16)

struct chr {
    uint8_t idx;
    uint8_t fg : 4;
    uint8_t bg : 4;
};

// Resize the ASCII buffer to match the given window size in px.
void scrResize(int, int);
int scrRows(), scrCols();

struct chr* chrAt(int, int);

// Straight outta <https://github.com/nonk123/conge>:

enum {
    C_BLACK,
    C_BLUE,
    C_GREEN,
    C_AQUA,
    C_RED,
    C_PURPLE,
    C_YELLOW,
    C_WHITE,
    C_GRAY,
    C_BRIGHT_BLUE,
    C_BRIGHT_GREEN,
    C_BRIGHT_AQUA,
    C_BRIGHT_RED,
    C_BRIGHT_PURPLE,
    C_BRIGHT_YELLOW,
    C_BRIGHT_WHITE,
};
