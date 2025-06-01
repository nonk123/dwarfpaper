#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

#include <time.h>

#include "clock.h"
extern void clockInit();

#include "log.h"
#include "modes.h"
#include "screen.h"

static HWND progman = NULL, workerWindow = NULL, mainWindow = NULL;
#define REFRESH_RATE (1)

static int findWorker(HWND topHandle, LPARAM topParamHandle) {
    HWND p = FindWindowEx(topHandle, NULL, "SHELLDLL_DefView", NULL);
    if (p != NULL)
        workerWindow = FindWindowEx(NULL, topHandle, "WorkerW", NULL);
    return 1;
}

static void paint() {
    PAINTSTRUCT ps;
    const int scrW = scrCols() * CHR_WIDTH, scrH = scrRows() * CHR_HEIGHT;

    RECT wholeScreen;
    wholeScreen.left = wholeScreen.top = 0;
    wholeScreen.right = scrW;
    wholeScreen.bottom = scrH;

    HDC hdc = GetDCEx(mainWindow, NULL, 0x403);
    if (hdc == NULL) {
        Warn("HDC is NULL!!!");
        return;
    }

    HBRUSH solid = CreateSolidBrush(RGB(255, 0, 255));
    // for (int y = 0; y < scrRows(); y++)
    // for (int x = 0; x < scrCols(); x++)
    FillRect(hdc, &wholeScreen, solid);
    DeleteObject(solid);

    ReleaseDC(mainWindow, hdc);
}

static void tickDraw() {
    RECT rect;
    GetWindowRect(workerWindow, &rect);
    scrResize(rect.right - rect.left + 1, rect.bottom - rect.top + 1);

    drawRandom(); // TODO: customize the mode
    RedrawWindow(mainWindow, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
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

    progman = FindWindow("Progman", NULL);
    if (progman == NULL)
        Fatal("Failed to find the Progman window!");

    SendMessage(progman, 0x052C, 0, 0);
    EnumWindows(findWorker, 0);

    if (workerWindow == NULL)
        Fatal("Failed to find the Worker window!!!");

    HMODULE hInstance = GetModuleHandle(NULL);

#define CLASS_NAME "Window Thing"
    WNDCLASS winClass = {0};
    winClass.lpszClassName = CLASS_NAME;
    winClass.style = CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc = &windowProc;
    winClass.hInstance = hInstance;

    if (!RegisterClass(&winClass))
        Fatal("Failed to register WNDCLASS");

    RECT rect;
    GetWindowRect(workerWindow, &rect);
    mainWindow = CreateWindowEx(
        0, CLASS_NAME, "Window", WS_MAXIMIZE | WS_VISIBLE, 0, 0, rect.right - rect.left + 1, rect.bottom - rect.top + 1,
        0, 0, hInstance, 0
    );
    SetParent(mainWindow, workerWindow);
    ShowWindow(workerWindow, 1);
#undef CLASS_NAME

    Info("Starting...");

    instant lastDraw = elapsed();
    for (;;) {
        tickDraw();

        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
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

    return EXIT_SUCCESS;
}
