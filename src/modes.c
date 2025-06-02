#include <stdlib.h>

#include "modes.h"
#include "screen.h"

void drawRandom() {
    for (int x = 0; x < scrCols(); x++)
        for (int y = 0; y < scrRows(); y++) {
            const int r = rand() & 1;
            chrAt(x, y)->fg = r ? C_BLACK : C_RED;
            chrAt(x, y)->bg = r ? C_AQUA : C_BLACK;
        }
}
