#include <time.h>

#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

#include "stb_image.h"

#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_properties.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_video.h"

#include "clock.h"
#include "fps.h"
#include "log.h"
#include "modes.h"
#include "screen.h"

static HWND workerWindow = NULL;
static SDL_Window* sdlWindow = NULL;
static SDL_Renderer* sdlRenderer = NULL;
static SDL_Texture* vgaTexture = NULL;

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

static struct chr backBuf[MAX_WIDTH * MAX_HEIGHT];
static SDL_Texture* doubleBuf = NULL;

void paintSDL() {
    Assert(doubleBuf != NULL, "`paintSDL` called before front-buffer texture was initialized");
    SDL_SetRenderTarget(sdlRenderer, doubleBuf);

    for (int y = 0; y < scrRows(); y++)
        for (int x = 0; x < scrCols(); x++) {
            const struct chr* chr = chrAt(x, y);
            struct chr* back = &backBuf[y * scrCols() + x];

            if (chr->idx == back->idx && chr->fg == back->fg && chr->bg == back->bg)
                continue;
            *back = *chr;

            SDL_FRect dest;
            dest.x = x * CHR_WIDTH;
            dest.y = y * CHR_HEIGHT;
            dest.w = CHR_WIDTH;
            dest.h = CHR_HEIGHT;

            SDL_FRect src;
            src.x = (int)(chr->idx % 16) * CHR_WIDTH;
            src.y = (int)(chr->idx / 16) * CHR_HEIGHT;
            src.w = CHR_WIDTH;
            src.h = CHR_HEIGHT;

            SDL_Color c = colors[chr->bg];
            SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, 255);
            SDL_RenderFillRect(sdlRenderer, &dest);

            c = colors[chr->fg];
            SDL_SetTextureColorMod(vgaTexture, c.r, c.g, c.b);
            SDL_RenderTexture(sdlRenderer, vgaTexture, &src, &dest);
        }

    SDL_SetRenderTarget(sdlRenderer, NULL);

    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);

    SDL_FRect rect = {0, 0, scrWidth(), scrHeight()};
    SDL_RenderTexture(sdlRenderer, doubleBuf, &rect, &rect);

    SDL_RenderPresent(sdlRenderer);
}

static int fpsCounter = 60;
int curFPS() {
    return fpsCounter;
}

static int rows = 0, cols = 0;
int main(int argc, char* argv[]) {
    initClock();
    initColors();
    srand(time(NULL));
    setMode("pipes");

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

    SDL_Rect bounds;
    SDL_DisplayID sdlDisplay = SDL_GetPrimaryDisplay();
    Assert(SDL_GetDisplayBounds(sdlDisplay, &bounds), "Failed to get display bounds");
    rows = bounds.h / CHR_HEIGHT + 1;
    cols = bounds.w / CHR_WIDTH + 1;

    Assert(SDL_SetWindowPosition(sdlWindow, 0, 0), "Failed to set the SDL window position! %s", SDL_GetError());
    Assert(
        SDL_SetWindowSize(sdlWindow, scrWidth(), scrHeight()), "Failed to set the SDL window size! %s", SDL_GetError()
    );

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

    Info("Running...");

    uint64_t second = 0, fps = 0;
    int targetDelta = 1000000000.0 / SDL_GetDesktopDisplayMode(sdlDisplay)->refresh_rate;

    for (;;) {
        const uint64_t frameStart = SDL_GetTicksNS();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                goto cleanup;
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                Assert(SDL_GetDisplayBounds(sdlDisplay, &bounds), "Failed to get display bounds");
                rows = bounds.h / CHR_HEIGHT + 1;
                cols = bounds.w / CHR_WIDTH + 1;

                doubleBuf = SDL_CreateTexture(
                    sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, scrWidth(), scrHeight()
                );
                Assert(doubleBuf != NULL, "Failed to create the front-buffer texture!!! %s", SDL_GetError());

                for (size_t i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++) {
                    backBuf[i].idx = 0;
                    backBuf[i].fg = C_GRAY;
                    backBuf[i].bg = C_BLACK;
                }

                modeForceRedraw();
            }
        }

        modeTick();

        const uint64_t now = SDL_GetTicksNS();
        uint64_t delta = now - frameStart;
        if (delta < targetDelta) {
            SDL_DelayNS(targetDelta - delta);
            delta = targetDelta;
        }

        fps++;
        second += delta;
        if (second >= 1000000000) {
            // Info("FPS: %d", curFPS());
            second -= 1000000000;
            fpsCounter = fps;
            fps = 0;
        }
    }

cleanup:
    Info("Goodbye!");

    SDL_DestroyTexture(doubleBuf);
    SDL_DestroyTexture(vgaTexture);
    SDL_DestroySurface(vgaSurface);
    stbi_image_free(vgaData);

    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();

    return EXIT_SUCCESS;
}

int scrCols() {
    return cols;
}

int scrRows() {
    return rows;
}
