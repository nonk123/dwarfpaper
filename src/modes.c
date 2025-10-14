#include <SDL3/SDL_stdinc.h>

#include "modes.h"
#include "modes/cave.h"
#include "modes/forest.h"
#include "modes/pipes.h"

const ModeTable modes[] = {
	{"forest", update_forest, 20.f},
	{"cave",   update_cave,   20.f},
	{"pipes",  update_pipes,  30.f},
	{NULL,     NULL,          0.f },
};
