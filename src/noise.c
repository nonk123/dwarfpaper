#include <SDL3/SDL_stdinc.h>

#include "log.h"
#include "noise.h"

float noise_at(float x, float y) {
	static struct osn_context* ctx = NULL;
	if (!ctx)
		open_simplex_noise(0, &ctx); // TODO: randomize seed
	expect(ctx, "Somehow failed to create a noise generator");
	return 1.f + 0.5f * open_simplex_noise2(ctx, x, y);
}
