#include <SDL3/SDL_stdinc.h>

#include "log.h"
#include "noise.h"

NoiseFloat noise_at(NoiseFloat x, NoiseFloat y) {
	static struct osn_context* ctx = NULL;
	if (!ctx)
		open_simplex_noise(0, &ctx); // TODO: randomize seed
	expect(ctx, "Somehow failed to create a noise generator");
	return 1.0 + 0.5 * open_simplex_noise2(ctx, x, y);
}
