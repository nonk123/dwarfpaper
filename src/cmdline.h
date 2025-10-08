#pragma once

#include <SDL3/SDL_stdinc.h>

typedef struct {
	char mode[128];
	bool debug;
} Args;
extern Args args;

void parse_cmdline(int, char*[]);
