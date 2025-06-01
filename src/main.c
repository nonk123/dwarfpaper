#include <time.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

#include "clock.h"
extern void clockInit();

#include "log.h"
#include "modes.h"
#include "screen.h"

static HWND workerWindow;

#define REFRESH_RATE (1)

static int walkOverWindows(HWND topHandle, LPARAM topParamHandle) {
    HWND p = FindWindowEx(topHandle, NULL, "SHELLDLL_DefView", NULL);
    if (p != NULL)
        workerWindow = FindWindowEx(NULL, topHandle, "WorkerW", NULL);
    return 1;
}

static void tickDraw() {
    RECT rect;
    GetWindowRect(workerWindow, &rect);
    scrResize(rect.right - rect.left + 1, rect.top - rect.bottom + 1);

    drawRandom(); // TODO: customize the mode

    RedrawWindow(workerWindow, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);

    HDC hdc = GetDCEx(workerWindow, NULL, 0x403);
    if (hdc == NULL) {
        Warn("HDC is NULL!!!");
        goto cleanup;
    }

    PAINTSTRUCT ps;
    const int scrW = scrCols() * CHR_WIDTH, scrH = scrRows() * CHR_HEIGHT;

    RECT wholeScreen;
    wholeScreen.left = wholeScreen.top = 0;
    wholeScreen.right = scrW;
    wholeScreen.bottom = scrH;

    BeginPaint(workerWindow, &ps);

    HBRUSH solid = CreateSolidBrush(RGB(255, 0, 255));
    // for (int y = 0; y < scrRows(); y++)
    // for (int x = 0; x < scrCols(); x++)
    FillRect(hdc, &wholeScreen, solid);

    EndPaint(workerWindow, &ps);

cleanup:
    ReleaseDC(workerWindow, hdc);
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    clockInit();

    HWND progman = FindWindow("Progman", NULL);
    if (progman == NULL)
        Fatal("Failed to find the Progman window!");

    DWORD_PTR result;
    WPARAM wZero = 0;
    LPARAM lZero = 0;
    SendMessageTimeout(progman, 0x052C, wZero, lZero, SMTO_NORMAL, 1000, &result);

    EnumWindows(walkOverWindows, lZero);
    if (workerWindow == NULL)
        Fatal("Failed to find the Worker window!!!");
    Info("Starting...");

    instant lastDraw = elapsed();
    for (;;) {
        tickDraw();

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
