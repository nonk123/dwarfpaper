#include <windows.h>

#include <stb_image.h>

#include "vga9x16.res.h"

#include "log.h"
#include "vga9x16.h"

static SDL_Surface* surface = NULL;
static uint8_t* image_data = NULL;

static void load() {
	static uint8_t buf[8 * 1024] = {0};

	HRSRC resource = FindResource(NULL, MAKEINTRESOURCE(VGA9X16), RT_RCDATA);
	expect(resource, "Failed to find the embedded 9x16.png");

	const uint64_t size = SizeofResource(NULL, resource);
	expect(size, "Failed to identify the size of the embedded 9x16.png");

	HGLOBAL res_data = LoadResource(NULL, resource);
	expect(res_data, "Failed to load the embedded 9x16.png");

	const uint8_t* real_data = LockResource(res_data);
	expect(real_data, "Failed to lock the embedded 9x16.png data");
	SDL_memcpy(buf, real_data, size);

	int d1, d2, n, ch = 4;
	image_data = stbi_load_from_memory(buf, (int)size, &d1, &d2, &n, ch);
	expect(image_data, "Failed to load the VGA 9x16 font PNG");

	surface = SDL_CreateSurfaceFrom(d1, d2, SDL_PIXELFORMAT_RGBA8888, image_data, ch * d1);
	expect(surface, "Failed to create the VGA 9x16 font surface!!! %s", SDL_GetError());
}

SDL_Surface* vga9x16() {
	if (!surface)
		load();
	return surface;
}

void vga9x16_cleanup() {
	if (image_data)
		stbi_image_free(image_data);
	SDL_DestroySurface(surface);
}
