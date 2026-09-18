#include <windows.h>
#include <stdlib.h>
#include <math.h>

static BOOL running=TRUE, preview=FALSE;
static HWND windows[16];
static int window_count=0;
static POINT start_cursor={0,0};
static DWORD start_time=0;
static float x=.5f,y=.5f,vx=.0018f,vy=.0014f,t=0;

static void e(HDC d,int l,int q,int r,int b,COLORREF c){HBRUSH z=CreateSolidBrush(c),o=SelectObject(d,z);Ellipse(d,l,q,r,b);SelectObject(d,o);DeleteObject(z);}
static void p(HDC d,POINT*a,int n,COLORREF c){HBRUSH z=CreateSolidBrush(c),o=SelectObject(d,z);Polygon(d,a,n);SelectObject(d,o);DeleteObject(z);}

static void tux(HDC d,int cx,int cy,int s){
    COLORREF k=RGB(25,25,28),w=RGB(245,245,245),o=RGB(245,145,25);
    e(d,cx-s*42,cy+s*62,cx+s*42,cy+s*76,RGB(10,25,45));
    e(d,cx-s*54,cy-s*48,cx+s*54,cy+s*70,k);
    e(d,cx-s*48,cy-s*82,cx+s*48,cy+s*15,k);
    e(d,cx-s*34,cy-s*57,cx+s*34,cy-s*3,w);
    e(d,cx-s*38,cy-s*4,cx+s*38,cy+s*58,w);
    e(d,cx-s*25,cy-s*50,cx-s*4,cy-s*23,w);
    e(d,cx+s*4,cy-s*50,cx+s*25,cy-s*23,w);
    e(d,cx-s*17,cy-s*42,cx-s*9,cy-s*32,k);
    e(d,cx+s*9,cy-s*42,cx+s*17,cy-s*32,k);
    POINT b[]={{cx-s*18,cy-s*16},{cx,cy+s},{cx+s*18,cy-s*16},{cx,cy-s*29}};
    p(d,b,4,o);
    POINT a[]={{cx-s*38,cy-s*5},{cx-s*72,cy+s*35},{cx-s*48,cy+s*45},{cx-s*27,cy+s*17}};
    POINT c[]={{cx+s*38,cy-s*5},{cx+s*72,cy+s*35},{cx+s*48,cy+s*45},{cx+s*27,cy+s*17}};
    p(d,a,4,k);p(d,c,4,k);
    e(d,cx-s*43,cy+s*48,cx-s*3,cy+s*72,o);
    e(d,cx+s*3,cy+s*48,cx+s*43,cy+s*72,o);
}

static void bg(HDC d,RECT*r){
    int h=r->bottom,w=r->right;
    for(int y0=0;y0<h;y0++){
        float z=(float)y0/(h?h:1);
        HBRUSH b=CreateSolidBrush(RGB(5+(int)(8*z),15+(int)(25*z),35+(int)(55*z)));
        RECT q={0,y0,w,y0+1};FillRect(d,&q,b);DeleteObject(b);
    }
}

static void draw_scene(HDC d,RECT*r){
    bg(d,r);
    int minDim=r->right<r->bottom?r->right:r->bottom;
    int s=minDim/9;
    if(s<20)s=20;
    if(s>100)s=100;
    int cx=(int)(x*r->right);
    int cy=(int)(y*r->bottom+sinf(t*2)*s*.12f);
    tux(d,cx,cy,s);
    SetBkMode(d,TRANSPARENT);
    SetTextColor(d,RGB(225,240,255));
    HFONT f=CreateFontW(-(r->bottom>700?34:25),0,0,0,FW_LIGHT,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI Light");
    HFONT old=SelectObject(d,f);
    RECT q={0,r->bottom-80,r->right,r->bottom-25};
    DrawTextW(d,L"Tux - CarsonScreenSaver",-1,&q,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    SelectObject(d,old);DeleteObject(f);
}

static LRESULT CALLBACK W(HWND h,UINT m,WPARAM a,LPARAM b){
    switch(m){
        case WM_CREATE:
            GetCursorPos(&start_cursor);start_time=GetTickCount();SetTimer(h,1,16,0);return 0;
        case WM_TIMER:
            t+=.016f;x+=vx;y+=vy;
            if(x<.16f||x>.84f)vx=-vx;
            if(y<.22f||y>.72f)vy=-vy;
            InvalidateRect(h,0,FALSE);return 0;
        case WM_ERASEBKGND:return 1;
        case WM_PAINT:{
            PAINTSTRUCT ps;HDC d=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);
            HDC mem=CreateCompatibleDC(d);HBITMAP bm=CreateCompatibleBitmap(d,r.right,r.bottom);
            HBITMAP oldbm=SelectObject(mem,bm);
            draw_scene(mem,&r);
            BitBlt(d,0,0,r.right,r.bottom,mem,0,0,SRCCOPY);
            SelectObject(mem,oldbm);DeleteObject(bm);DeleteDC(mem);
            EndPaint(h,&ps);return 0;
        }
        case WM_MOUSEMOVE:
            if(!preview && GetTickCount()-start_time>1200){
                POINT pt;GetCursorPos(&pt);
                if(pt.x!=start_cursor.x || pt.y!=start_cursor.y){running=FALSE;for(int i=0;i<window_count;i++)if(windows[i]&&windows[i]!=h)DestroyWindow(windows[i]);DestroyWindow(h);PostQuitMessage(0);return 0;}
            }
            break;
        case WM_LBUTTONDOWN:case WM_RBUTTONDOWN:case WM_MBUTTONDOWN:
        case WM_KEYDOWN:case WM_SYSKEYDOWN:
            if(!preview){running=FALSE;for(int i=0;i<window_count;i++)if(windows[i]&&windows[i]!=h)DestroyWindow(windows[i]);DestroyWindow(h);PostQuitMessage(0);return 0;}break;
        case WM_DESTROY:KillTimer(h,1);running=FALSE;PostQuitMessage(0);return 0;
    }
    return DefWindowProcW(h,m,a,b);
}

static BOOL CALLBACK create_tux_monitor(HMONITOR monitor,HDC dc,LPRECT unused,LPARAM data){
    (void)dc;(void)unused;(void)data;
    if(window_count>=16)return FALSE;
    MONITORINFO mi={sizeof(mi)};GetMonitorInfoW(monitor,&mi);
    RECT r=mi.rcMonitor;HINSTANCE i=GetModuleHandleW(0);
    HWND h=CreateWindowExW(WS_EX_TOPMOST,L"CarsonTuxScreenSaverWindow",L"Carson Tux ScreenSaver",WS_POPUP,
      r.left,r.top,r.right-r.left,r.bottom-r.top,NULL,0,i,0);
    if(h){windows[window_count++]=h;ShowWindow(h,SW_SHOW);UpdateWindow(h);}
    return TRUE;
}
static int run(HWND parent){
    HINSTANCE i=GetModuleHandleW(0);
    WNDCLASSW c={0};c.hInstance=i;c.lpfnWndProc=W;c.lpszClassName=L"CarsonTuxScreenSaverWindow";
    c.hCursor=LoadCursorW(0,IDC_ARROW);c.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);RegisterClassW(&c);
    preview=parent!=0;running=TRUE;window_count=0;
    if(parent){
        RECT r;GetClientRect(parent,&r);
        windows[0]=CreateWindowExW(WS_EX_TOPMOST,c.lpszClassName,L"Carson Tux ScreenSaver",WS_CHILD,0,0,r.right,r.bottom,parent,0,i,0);
        if(!windows[0])return 1;
        window_count=1;ShowWindow(windows[0],SW_SHOW);UpdateWindow(windows[0]);
    }else{
        ShowCursor(FALSE);
        EnumDisplayMonitors(NULL,NULL,create_tux_monitor,0);
        if(window_count==0){ShowCursor(TRUE);return 1;}
    }
    MSG m;while(running&&GetMessageW(&m,0,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}
    if(!parent)ShowCursor(TRUE);return 0;
}

int WINAPI wWinMain(HINSTANCE i,HINSTANCE p,PWSTR cmd,int s){
    (void)i;(void)p;(void)s;
    while(*cmd==L' '||*cmd==L'\t')cmd++;
    if(*cmd==L'/'||*cmd==L'-'){
        wchar_t o=cmd[1];
        if(o==L'c'||o==L'C'){MessageBoxW(0,L"Carson Tux ScreenSaver\n\nAnimated Tux-themed screensaver.\n/s = run\n/p <HWND> = preview",L"Carson Tux ScreenSaver",MB_OK);return 0;}
        if(o==L'p'||o==L'P'){cmd+=2;while(*cmd==L' '||*cmd==L'\t')cmd++;HWND h=(HWND)(ULONG_PTR)wcstoull(cmd,0,10);return h?run(h):0;}
        if(o==L's'||o==L'S')return run(0);
    }
    return run(0);
}
