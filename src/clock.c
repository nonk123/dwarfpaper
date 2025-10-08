#include <SDL3/SDL_timer.h>

#include "clock.h"

Instant elapsed() {
	return SDL_GetTicksNS();
}
