#include <time.h>

#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

#include "stb_image.h"

#include "SDL3/SDL_error.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_properties.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_video.h"

#include "clock.h"
#include "log.h"
#include "modes.h"
#include "screen.h"

static HWND workerWindow = NULL;
static SDL_Window* sdlWindow = NULL;
static SDL_Renderer* sdlRenderer = NULL;
static SDL_Texture* vgaTexture = NULL;

#define UPDATE_RATE (60)

static SDL_Color colors[16] = {0};

static void initColors() {
#define FULL (255)
#define MID (170)
#define LOW (85)
    colors[C_BLACK] = (SDL_Color){0, 0, 0, 255};
    colors[C_GRAY] = (SDL_Color){LOW, LOW, LOW, 255};
    colors[C_WHITE] = (SDL_Color){MID, MID, MID, 255};
    colors[C_BRIGHT_WHITE] = (SDL_Color){FULL, FULL, FULL, 255};

    colors[C_RED] = (SDL_Color){MID, 0, 0, 255};
    colors[C_BRIGHT_RED] = (SDL_Color){FULL, LOW, LOW, 255};

    colors[C_GREEN] = (SDL_Color){0, MID, 0, 255};
    colors[C_BRIGHT_GREEN] = (SDL_Color){LOW, FULL, LOW, 255};

    colors[C_YELLOW] = (SDL_Color){FULL, MID, 0, 255};
    colors[C_BRIGHT_YELLOW] = (SDL_Color){FULL, FULL, LOW, 255};

    colors[C_BLUE] = (SDL_Color){0, 0, MID, 255};
    colors[C_BRIGHT_BLUE] = (SDL_Color){LOW, LOW, FULL, 255};

    colors[C_PURPLE] = (SDL_Color){MID, 0, MID, 255};
    colors[C_BRIGHT_PURPLE] = (SDL_Color){FULL, LOW, FULL, 255};

    colors[C_AQUA] = (SDL_Color){0, MID, MID, 255};
    colors[C_BRIGHT_AQUA] = (SDL_Color){LOW, FULL, FULL, 255};
#undef LOW
#undef MID
#undef FULL
}

static int findWorker(HWND topHandle, LPARAM topParamHandle) {
    HWND p = FindWindowEx(topHandle, NULL, "SHELLDLL_DefView", NULL);
    if (p != NULL)
        workerWindow = FindWindowEx(NULL, topHandle, "WorkerW", NULL);
    return 1;
}

void paintSDL() {
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);

    for (int y = 0; y < scrRows(); y++)
        for (int x = 0; x < scrCols(); x++) {
            const struct chr* chr = chrAt(x, y);

            SDL_FRect rect;
            rect.x = x * CHR_WIDTH;
            rect.y = y * CHR_HEIGHT;
            rect.w = CHR_WIDTH;
            rect.h = CHR_HEIGHT;

            SDL_FRect src;
            src.x = (int)(chr->idx % 16) * CHR_WIDTH;
            src.y = (int)(chr->idx / 16) * CHR_HEIGHT;
            src.w = CHR_WIDTH;
            src.h = CHR_HEIGHT;

            SDL_Color c = colors[chr->bg];
            SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, 255);
            SDL_RenderFillRect(sdlRenderer, &rect);

            c = colors[chr->fg];
            SDL_SetTextureColorMod(vgaTexture, c.r, c.g, c.b);
            SDL_RenderTexture(sdlRenderer, vgaTexture, &src, &rect);
        }

    SDL_RenderPresent(sdlRenderer);
}

int main(int argc, char* argv[]) {
    initClock();
    initColors();
    srand(time(NULL));
    setMode("jumbled");

    Assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS), "SDL_Init failed! %s", SDL_GetError());

    HWND progman = FindWindow("Progman", NULL);
    Assert(progman != NULL, "Failed to find the Progman window!");

    SendMessage(progman, 0x052C, 0, 0);
    EnumWindows(findWorker, 0);
    Assert(workerWindow != NULL, "Failed to find the Worker window!!!");

    RECT rect;
    GetWindowRect(workerWindow, &rect);

    Assert(
        SDL_CreateWindowAndRenderer("dwarfpaper", 1, 1, SDL_WINDOW_FULLSCREEN, &sdlWindow, &sdlRenderer),
        "Failed to create the SDL window/renderer!!! %s", SDL_GetError()
    );

    syncScreenSize();
    Assert(SDL_SetWindowPosition(sdlWindow, 0, 0), "Failed to set the SDL window position! %s", SDL_GetError());
    Assert(
        SDL_SetWindowSize(sdlWindow, scrWidth(), scrHeight()), "Failed to set the SDL window size! %s", SDL_GetError()
    );
    Assert(SDL_SyncWindow(sdlWindow), "Failed to sync SDL window! %s", SDL_GetError());

    // Flush the whole buffer so at the next redraw we won't get a black screen....
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderPresent(sdlRenderer);

    HWND mainWindow =
        SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    SetParent(mainWindow, workerWindow);
    ShowWindow(workerWindow, 1); // !!! won't do jackshit without this

    int d1, d2, n;
    uint8_t* vgaData = stbi_load("9x16.png", &d1, &d2, &n, 4);
    Assert(vgaData != NULL, "Failed to load the VGA 9x16 font PNG");

    SDL_Surface* vgaSurface = SDL_CreateSurfaceFrom(144, 256, SDL_PIXELFORMAT_RGBA8888, vgaData, 144 * 4);
    Assert(vgaSurface != NULL, "Failed to create the VGA 9x16 font surface!!! %s", SDL_GetError());

    vgaTexture = SDL_CreateTextureFromSurface(sdlRenderer, vgaSurface);
    Assert(vgaTexture != NULL, "Failed to load the VGA 9x16 font texture!!! %s", SDL_GetError());

    Info("Starting...");

    instant lastUpdate = elapsed();

    for (;;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                goto cleanup;
        }

        syncScreenSize();
        modeTick();

        instant thisUpdate = elapsed(), delta = thisUpdate - lastUpdate;

#define TARGET_DELTA (CLOCK_RES / UPDATE_RATE)
        if (delta < TARGET_DELTA) {
            msSleep((TARGET_DELTA - delta) / (CLOCK_RES / 1000));
            delta = TARGET_DELTA;
        }
#undef TARGET_DELTA

        lastUpdate += delta;
    }

cleanup:
    Info("Goodbye!");

    SDL_DestroyTexture(vgaTexture);
    SDL_DestroySurface(vgaSurface);
    stbi_image_free(vgaData);

    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();

    return EXIT_SUCCESS;
}
