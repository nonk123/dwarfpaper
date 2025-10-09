#include <SDL3/SDL_stdinc.h>

#include "modes.h"

#include "modes/cave.h"
#include "modes/pipes.h"

ModeTable modes[] = {
	{"pipes", NULL, update_pipes},
	{"cave",  NULL, update_cave },
	{NULL,    NULL, NULL        },
};
