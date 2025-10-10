#include <SDL3/SDL_stdinc.h>

#include "modes.h"
#include "modes/cave.h"
#include "modes/forest.h"
#include "modes/pipes.h"

const ModeTable modes[] = {
	{"forest", NULL, update_forest},
	{"cave",   NULL, update_cave  },
	{"pipes",  NULL, update_pipes },
	{NULL,     NULL, NULL         },
};
