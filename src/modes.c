#include <SDL3/SDL_stdinc.h>

#include "modes.h"

#include "modes/cave.h"
#include "modes/pipes.h"

ModeTable modes[] = {
	{"pipes", draw_pipes, update_pipes},
	{"cave",  draw_cave,  update_cave },
	{NULL,    NULL,       NULL        },
};
