#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static HWND g_hwnd;
static BOOL g_running = TRUE;
static float g_time = 0.0f;

typedef struct {
    float x, y, vx, vy, size;
    float phase;
} Bubble;

static Bubble g_bubbles[] = {
    {0.18f, 0.25f, 0.0007f, 0.0004f, 0.09f, 0.0f},
    {0.42f, 0.68f, -0.0005f, -0.0006f, 0.065f, 1.2f},
    {0.73f, 0.30f, 0.0004f, -0.0005f, 0.075f, 2.4f},
    {0.82f, 0.72f, -0.0006f, 0.0003f, 0.055f, 3.7f},
    {0.30f, 0.52f, 0.0003f, 0.0007f, 0.045f, 4.8f}
};
static const int g_bubble_count = (int)(sizeof(g_bubbles) / sizeof(g_bubbles[0]));

static void fill_gradient(HDC hdc, const RECT *rc) {
    int width = rc->right - rc->left;
    int height = rc->bottom - rc->top;
    for (int y = 0; y < height; ++y) {
        float t = height > 1 ? (float)y / (float)(height - 1) : 0.0f;
        int r = (int)(7 + 24 * t);
        int g = (int)(22 + 52 * t);
        int b = (int)(56 + 105 * t);
        HBRUSH brush = CreateSolidBrush(RGB(r, g, b));
        RECT line = {rc->left, rc->top + y, rc->right, rc->top + y + 1};
        FillRect(hdc, &line, brush);
        DeleteObject(brush);
    }
    (void)width;
}

static void draw_bubble(HDC hdc, int cx, int cy, int radius, BYTE alpha) {
    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP bitmap = CreateCompatibleBitmap(hdc, radius * 2 + 8, radius * 2 + 8);
    HGDIOBJ old = SelectObject(mem, bitmap);

    RECT r = {0, 0, radius * 2 + 8, radius * 2 + 8};
    HBRUSH clear = CreateSolidBrush(RGB(12, 40, 85));
    FillRect(mem, &r, clear);
    DeleteObject(clear);

    HPEN pen = CreatePen(PS_SOLID, 2, RGB(135, 205, 255));
    HGDIOBJ oldPen = SelectObject(mem, pen);
    HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HGDIOBJ oldBrush = SelectObject(mem, nullBrush);
    Ellipse(mem, 4, 4, radius * 2 + 4, radius * 2 + 4);
    SelectObject(mem, oldBrush);
    SelectObject(mem, oldPen);
    DeleteObject(pen);

    BLENDFUNCTION blend = {AC_SRC_OVER, 0, alpha, 0};
    SIZE size = {radius * 2 + 8, radius * 2 + 8};
    POINT dst = {cx - radius - 4, cy - radius - 4};
    POINT src = {0, 0};
    AlphaBlend(hdc, dst.x, dst.y, size.cx, size.cy, mem, src.x, src.y, size.cx, size.cy, blend);

    SelectObject(mem, old);
    DeleteObject(bitmap);
    DeleteDC(mem);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 16, NULL);
        return 0;

    case WM_TIMER:
        g_time += 0.016f;
        for (int i = 0; i < g_bubble_count; ++i) {
            Bubble *b = &g_bubbles[i];
            b->x += b->vx;
            b->y += b->vy;
            if (b->x < -0.08f) b->x = 1.08f;
            if (b->x > 1.08f) b->x = -0.08f;
            if (b->y < -0.08f) b->y = 1.08f;
            if (b->y > 1.08f) b->y = -0.08f;
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);

        fill_gradient(hdc, &rc);

        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;

        for (int i = 0; i < g_bubble_count; ++i) {
            const Bubble *b = &g_bubbles[i];
            int cx = (int)(b->x * w);
            int cy = (int)(b->y * h + sinf(g_time * 0.7f + b->phase) * 18.0f);
            int radius = (int)(b->size * (float)(w < h ? w : h));
            if (radius < 18) radius = 18;
            draw_bubble(hdc, cx, cy, radius, 48);
        }

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(225, 242, 255));
        HFONT font = CreateFontW(
            -(h > 700 ? 42 : 30), 0, 0, 0, FW_LIGHT,
            FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH,
            L"Segoe UI Light"
        );
        HFONT oldFont = (HFONT)SelectObject(hdc, font);

        RECT title = rc;
        title.top = h / 2 - 35;
        title.bottom = h / 2 + 15;
        DrawTextW(hdc, L"CarsonScreenSaver", -1, &title,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdc, oldFont);
        DeleteObject(font);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (GetWindowLongPtrW(hwnd, GWLP_USERDATA) == 1) {
            DestroyWindow(hwnd);
            return 0;
        }
        break;

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
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
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

    if (!parent) {
        ShowCursor(FALSE);
        SetWindowLongPtrW(g_hwnd, GWLP_USERDATA, 1);
    }

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (g_running && GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (!parent) ShowCursor(TRUE);
    return 0;
}

static void show_config(HWND parent) {
    MessageBoxW(
        parent,
        L"CarsonScreenSaver\n\n"
        L"Current mode: Aero / Windows 7 inspired\n"
        L"Floating glass bubbles drift across a blue gradient.",
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
