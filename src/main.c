#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

#include <time.h>

#include "stb_image.h"

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_main.h"
#include "SDL3/SDL_properties.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

#include "clock.h"
extern void clockInit();

#include "log.h"
#include "modes.h"
#include "screen.h"

static HWND progman = NULL, workerWindow = NULL, mainWindow = NULL;
static SDL_Window* sdlWindow = NULL;
static SDL_Renderer* sdlRenderer = NULL;

#define REFRESH_RATE (1)

static uint8_t* vgaData = NULL;
static HBRUSH brushes[16] = {0};

static void initBrushes() {
#define FULL (255)
#define MID (170)
#define LOW (85)
    brushes[C_BLACK] = CreateSolidBrush(RGB(0, 0, 0));
    brushes[C_GRAY] = CreateSolidBrush(RGB(LOW, LOW, LOW));
    brushes[C_WHITE] = CreateSolidBrush(RGB(MID, MID, MID));
    brushes[C_BRIGHT_WHITE] = CreateSolidBrush(RGB(FULL, FULL, FULL));

    brushes[C_RED] = CreateSolidBrush(RGB(MID, 0, 0));
    brushes[C_BRIGHT_RED] = CreateSolidBrush(RGB(FULL, LOW, LOW));

    brushes[C_GREEN] = CreateSolidBrush(RGB(0, MID, 0));
    brushes[C_BRIGHT_GREEN] = CreateSolidBrush(RGB(LOW, FULL, LOW));

    brushes[C_YELLOW] = CreateSolidBrush(RGB(FULL, MID, 0));
    brushes[C_BRIGHT_YELLOW] = CreateSolidBrush(RGB(FULL, FULL, LOW));

    brushes[C_BLUE] = CreateSolidBrush(RGB(0, 0, MID));
    brushes[C_BRIGHT_BLUE] = CreateSolidBrush(RGB(LOW, LOW, FULL));

    brushes[C_PURPLE] = CreateSolidBrush(RGB(MID, 0, MID));
    brushes[C_BRIGHT_PURPLE] = CreateSolidBrush(RGB(FULL, LOW, FULL));

    brushes[C_AQUA] = CreateSolidBrush(RGB(0, MID, MID));
    brushes[C_BRIGHT_AQUA] = CreateSolidBrush(RGB(LOW, FULL, FULL));
#undef LOW
#undef MID
#undef FULL
}

static void freeBrushes() {
    for (size_t i = 0; i < 16; i++)
        DeleteObject(brushes[i]);
}

static int findWorker(HWND topHandle, LPARAM topParamHandle) {
    HWND p = FindWindowEx(topHandle, NULL, "SHELLDLL_DefView", NULL);
    if (p != NULL)
        workerWindow = FindWindowEx(NULL, topHandle, "WorkerW", NULL);
    return 1;
}

static void paint() {
    const int scrW = scrCols() * CHR_WIDTH, scrH = scrRows() * CHR_HEIGHT;
}

static void tickDraw() {
    RECT rect;
    GetWindowRect(workerWindow, &rect);
    scrResize(rect.right - rect.left + 1, rect.bottom - rect.top + 1);

    drawRandom(); // TODO: customize the mode

    // RedrawWindow(mainWindow, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);

    SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderPresent(sdlRenderer);
}

static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (msg == WM_PAINT) {
        paint();
        return 0;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

// ungh....................
int main(int argc, char* argv[]) {
    srand(time(NULL));
    clockInit();

    if (!SDL_Init(SDL_INIT_VIDEO))
        Fatal("SDL_Init failed!");

    progman = FindWindow("Progman", NULL);
    if (progman == NULL)
        Fatal("Failed to find the Progman window!");

    SendMessage(progman, 0x052C, 0, 0);
    EnumWindows(findWorker, 0);

    if (workerWindow == NULL)
        Fatal("Failed to find the Worker window!!!");

    HMODULE hInstance = GetModuleHandle(NULL);

    RECT rect;
    GetWindowRect(workerWindow, &rect);

    if (!SDL_CreateWindowAndRenderer(
            "dwarfpaper", rect.right - rect.left + 1, rect.bottom - rect.top + 1,
            SDL_WINDOW_MAXIMIZED | SDL_WINDOW_BORDERLESS, &sdlWindow, &sdlRenderer
        ))
        Fatal("Failed to create the SDL window!!!");

    mainWindow = SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    SetParent(mainWindow, workerWindow);
    ShowWindow(workerWindow, 1);

    initBrushes();

    int d1, d2, n;
    vgaData = stbi_load("9x16.png", &d1, &d2, &n, 0);
    if (vgaData == NULL)
        Fatal("Failed to load the VGA 9x16 font PNG");

    Info("Starting...");

    instant lastDraw = elapsed();
    for (;;) {
        tickDraw();

        MSG msg;
        while (PeekMessageW(&msg, workerWindow, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        instant thisDraw = elapsed(), delta = thisDraw - lastDraw;

#define TARGET_DELTA (CLOCK_RES / REFRESH_RATE)
        if (delta < TARGET_DELTA) {
            msSleep((TARGET_DELTA - delta) / (CLOCK_RES / 1000));
            delta = TARGET_DELTA;
        }
#undef TARGET_DELTA

        lastDraw += delta;
    }

    Info("Goodbye!");

    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();

    stbi_image_free(vgaData);
    freeBrushes();

    return EXIT_SUCCESS;
}
