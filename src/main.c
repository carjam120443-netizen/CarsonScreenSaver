#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdlib.h>
#include <string.h>

static HWND g_hwnd;
static BOOL g_running = TRUE;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 16, NULL);
        return 0;
    case WM_TIMER:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH bg = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rc, bg);
        DeleteObject(bg);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        const wchar_t *text = L"CarsonScreenSaver";
        DrawTextW(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        g_running = FALSE;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static int run_screensaver(HWND parent) {
    HINSTANCE instance = GetModuleHandleW(NULL);
    WNDCLASSW wc = {0};
    wc.hInstance = instance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = L"CarsonScreenSaverWindow";
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    DWORD style = parent ? WS_CHILD : WS_POPUP;
    RECT rc;
    if (parent) {
        GetClientRect(parent, &rc);
    } else {
        rc.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        rc.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
        rc.right = rc.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
        rc.bottom = rc.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }

    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST, wc.lpszClassName, L"CarsonScreenSaver", style,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        parent, NULL, instance, NULL
    );
    if (!g_hwnd) return 1;

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (g_running && GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

static void show_config(HWND parent) {
    MessageBoxW(
        parent,
        L"CarsonScreenSaver configuration will go here.\n\n"
        L"This starter project is ready for multiple screensaver modes.",
        L"CarsonScreenSaver",
        MB_OK | MB_ICONINFORMATION
    );
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR commandLine, int show) {
    (void)instance;
    (void)previous;
    (void)show;

    wchar_t *cmd = commandLine;
    while (*cmd == L' ' || *cmd == L'\t') cmd++;

    if (*cmd == L'/' || *cmd == L'-') {
        wchar_t option = cmd[1];
        if (option == L'c' || option == L'C') {
            show_config(NULL);
            return 0;
        }
        if (option == L'p' || option == L'P') {
            cmd += 2;
            while (*cmd == L' ' || *cmd == L'\t') cmd++;
            HWND preview = (HWND)(ULONG_PTR)wcstoull(cmd, NULL, 10);
            if (preview) return run_screensaver(preview);
            return 0;
        }
        if (option == L's' || option == L'S') {
            return run_screensaver(NULL);
        }
    }
    return run_screensaver(NULL);
}
