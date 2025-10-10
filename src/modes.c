#include <SDL3/SDL_stdinc.h>

#include "modes.h"
#include "modes/cave.h"
#include "modes/forest.h"
#include "modes/pipes.h"

const ModeTable modes[] = {
	{"forest", NULL, update_forest, 10.f},
	{"cave",   NULL, update_cave,   10.f},
	{"pipes",  NULL, update_pipes,  30.f},
	{NULL,     NULL, NULL,          0.f },
};
