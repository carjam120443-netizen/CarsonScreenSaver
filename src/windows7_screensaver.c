#include <windows.h>
#include <math.h>

static HWND windows[16];
static int window_count = 0;
static BOOL running = TRUE;
static BOOL preview = FALSE;
static POINT start_cursor = {0,0};
static DWORD start_time = 0;
static float logo_x = 0.5f, logo_y = 0.5f;
static float vx = 0.0022f, vy = 0.0016f;
static float phase = 0.0f;

static void fill_gradient(HDC dc, const RECT *r) {
    int h = r->bottom - r->top;
    for (int y = 0; y < h; ++y) {
        float t = h > 1 ? (float)y / (float)(h - 1) : 0.0f;
        HBRUSH b = CreateSolidBrush(RGB(4 + (int)(8*t), 16 + (int)(28*t), 48 + (int)(58*t)));
        RECT line = {0, y, r->right, y + 1};
        FillRect(dc, &line, b);
        DeleteObject(b);
    }
}

static void polygon(HDC dc, POINT *pts, int count, COLORREF color) {
    HBRUSH b = CreateSolidBrush(color);
    HGDIOBJ old = SelectObject(dc, b);
    Polygon(dc, pts, count);
    SelectObject(dc, old);
    DeleteObject(b);
}

static void draw_windows_logo(HDC dc, int cx, int cy, int s) {
    const int gap = s / 18;
    const int pane = (s - gap) / 2;
    const int tilt = s / 13;
    const COLORREF colors[4] = {
        RGB(242, 80, 34), RGB(127, 186, 0),
        RGB(0, 164, 239), RGB(255, 185, 0)
    };

    for (int i = 5; i >= 1; --i) {
        int rad = s / 2 + i * 9;
        HBRUSH glow = CreateSolidBrush(RGB(8, 35, 75));
        HGDIOBJ old = SelectObject(dc, glow);
        Ellipse(dc, cx-rad, cy-rad, cx+rad, cy+rad);
        SelectObject(dc, old);
        DeleteObject(glow);
    }

    int left = cx - s/2;
    int top = cy - s/2;
    POINT p[4];

    p[0]=(POINT){left, top + tilt}; p[1]=(POINT){left + pane, top};
    p[2]=(POINT){left + pane, top + pane}; p[3]=(POINT){left, top + pane + tilt};
    polygon(dc, p, 4, colors[0]);

    p[0]=(POINT){left + pane + gap, top}; p[1]=(POINT){left + s, top - tilt};
    p[2]=(POINT){left + s, top + pane - tilt}; p[3]=(POINT){left + pane + gap, top + pane};
    polygon(dc, p, 4, colors[1]);

    p[0]=(POINT){left, top + pane + gap + tilt}; p[1]=(POINT){left + pane, top + pane + gap};
    p[2]=(POINT){left + pane, top + s}; p[3]=(POINT){left, top + s - tilt};
    polygon(dc, p, 4, colors[2]);

    p[0]=(POINT){left + pane + gap, top + pane + gap}; p[1]=(POINT){left + s, top + pane - tilt};
    p[2]=(POINT){left + s, top + s - tilt}; p[3]=(POINT){left + pane + gap, top + s};
    polygon(dc, p, 4, colors[3]);
}

static void draw_scene(HDC dc, RECT *r) {
    fill_gradient(dc, r);

    int w = r->right, h = r->bottom;
    int min_dim = w < h ? w : h;
    int s = min_dim / 3;
    if (s < 140) s = 140;
    if (s > 430) s = 430;

    int cx = (int)(logo_x * w);
    int cy = (int)(logo_y * h + sinf(phase) * s * 0.035f);
    draw_windows_logo(dc, cx, cy, s);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(235, 245, 255));

    HFONT logo_font = CreateFontW(-(s / 7), 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH, L"Segoe UI Light");
    HGDIOBJ old = SelectObject(dc, logo_font);
    RECT title = {0, cy + s/2 + 22, w, cy + s/2 + 85};
    DrawTextW(dc, L"Windows 7", -1, &title, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old);
    DeleteObject(logo_font);

    HFONT small = CreateFontW(-(h > 700 ? 28 : 21), 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH, L"Segoe UI Light");
    old = (HFONT)SelectObject(dc, small);
    RECT footer = {0, h - 65, w, h - 20};
    DrawTextW(dc, L"CarsonScreenSaver", -1, &footer, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old);
    DeleteObject(small);
}

static void close_all(HWND except) {
    running = FALSE;
    for (int i = 0; i < window_count; ++i)
        if (windows[i] && windows[i] != except) DestroyWindow(windows[i]);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        GetCursorPos(&start_cursor);
        start_time = GetTickCount();
        SetTimer(hwnd, 1, 16, NULL);
        return 0;

    case WM_TIMER:
        phase += 0.035f;
        logo_x += vx; logo_y += vy;
        if (logo_x < 0.30f || logo_x > 0.70f) vx = -vx;
        if (logo_y < 0.34f || logo_y > 0.66f) vy = -vy;
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT r; GetClientRect(hwnd, &r);
        HDC mem = CreateCompatibleDC(dc);
        HBITMAP bitmap = CreateCompatibleBitmap(dc, r.right, r.bottom);
        HGDIOBJ old_bitmap = SelectObject(mem, bitmap);
        draw_scene(mem, &r);
        BitBlt(dc, 0, 0, r.right, r.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old_bitmap);
        DeleteObject(bitmap);
        DeleteDC(mem);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_MOUSEMOVE:
        if (!preview && GetTickCount() - start_time > 1200) {
            POINT pt; GetCursorPos(&pt);
            if (pt.x != start_cursor.x || pt.y != start_cursor.y) {
                close_all(hwnd); DestroyWindow(hwnd); PostQuitMessage(0); return 0;
            }
        }
        break;

    case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN:
    case WM_KEYDOWN: case WM_SYSKEYDOWN:
        if (!preview) {
            close_all(hwnd); DestroyWindow(hwnd); PostQuitMessage(0); return 0;
        }
        break;

    case WM_DESTROY:
        KillTimer(hwnd, 1); running = FALSE; PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static BOOL CALLBACK create_monitor(HMONITOR monitor, HDC dc, LPRECT unused, LPARAM data) {
    (void)dc; (void)unused; (void)data;
    if (window_count >= 16) return FALSE;
    MONITORINFO mi = {sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    RECT r = mi.rcMonitor;
    HINSTANCE instance = GetModuleHandleW(NULL);
    HWND hwnd = CreateWindowExW(WS_EX_TOPMOST, L"CarsonWindows7ScreenSaverWindow",
        L"Carson Windows 7 ScreenSaver", WS_POPUP,
        r.left, r.top, r.right-r.left, r.bottom-r.top, NULL, NULL, instance, NULL);
    if (hwnd) {
        windows[window_count++] = hwnd;
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }
    return TRUE;
}

static int run_screensaver(HWND parent) {
    HINSTANCE instance = GetModuleHandleW(NULL);
    WNDCLASSW wc = {0};
    wc.hInstance = instance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = L"CarsonWindows7ScreenSaverWindow";
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassW(&wc);

    preview = parent != NULL; running = TRUE; window_count = 0;

    if (parent) {
        RECT r; GetClientRect(parent, &r);
        windows[0] = CreateWindowExW(WS_EX_TOPMOST, wc.lpszClassName,
            L"Carson Windows 7 ScreenSaver", WS_CHILD, 0, 0, r.right, r.bottom,
            parent, NULL, instance, NULL);
        if (!windows[0]) return 1;
        window_count = 1; ShowWindow(windows[0], SW_SHOW); UpdateWindow(windows[0]);
    } else {
        ShowCursor(FALSE);
        EnumDisplayMonitors(NULL, NULL, create_monitor, 0);
        if (window_count == 0) { ShowCursor(TRUE); return 1; }
    }

    MSG msg;
    while (running && GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg); DispatchMessageW(&msg);
    }
    if (!parent) ShowCursor(TRUE);
    return 0;
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR commandLine, int show) {
    (void)instance; (void)previous; (void)show;
    while (*commandLine == L' ' || *commandLine == L'\t') commandLine++;

    if (*commandLine == L'/' || *commandLine == L'-') {
        wchar_t option = commandLine[1];
        if (option == L'c' || option == L'C') {
            MessageBoxW(NULL,
                L"Carson Windows 7 ScreenSaver\n\nAnimated Windows 7-style logo screensaver.\n/s = run\n/p <HWND> = preview",
                L"Carson Windows 7 ScreenSaver", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        if (option == L'p' || option == L'P') {
            commandLine += 2;
            while (*commandLine == L' ' || *commandLine == L'\t') commandLine++;
            HWND preview_window = (HWND)(ULONG_PTR)wcstoull(commandLine, NULL, 10);
            return preview_window ? run_screensaver(preview_window) : 0;
        }
        if (option == L's' || option == L'S') return run_screensaver(NULL);
    }
    return run_screensaver(NULL);
}
