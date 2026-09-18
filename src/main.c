#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CONFIG_REG_KEY L"Software\\CarsonScreenSaver"
#define CONFIG_BUBBLES L"Bubbles"
#define CONFIG_SPEED L"Speed"
#define CONFIG_BRIGHTNESS L"Brightness"

static HWND g_windows[16];
static int g_window_count=0;
static BOOL g_running = TRUE;
static BOOL g_preview = FALSE;
static BOOL g_config_mode = FALSE;
static POINT g_start_cursor={0,0};
static DWORD g_start_time=0;
static float g_time = 0.0f;
static int g_bubble_count_setting = 5;
static int g_speed_setting = 100;
static int g_brightness_setting = 100;

typedef struct { float x,y,vx,vy,size,phase; } Bubble;
static Bubble g_bubbles[] = {
 {0.18f,0.25f,0.0007f,0.0004f,0.09f,0.0f},
 {0.42f,0.68f,-0.0005f,-0.0006f,0.065f,1.2f},
 {0.73f,0.30f,0.0004f,-0.0005f,0.075f,2.4f},
 {0.82f,0.72f,-0.0006f,0.0003f,0.055f,3.7f},
 {0.30f,0.52f,0.0003f,0.0007f,0.045f,4.8f},
 {0.60f,0.18f,-0.0004f,0.0005f,0.05f,5.4f},
 {0.12f,0.78f,0.0005f,-0.0004f,0.04f,6.1f},
 {0.90f,0.48f,-0.0003f,0.0006f,0.06f,7.0f}
};
static const int g_bubble_max = (int)(sizeof(g_bubbles)/sizeof(g_bubbles[0]));

static void load_settings(void) {
 HKEY key;
 if (RegOpenKeyExW(HKEY_CURRENT_USER, CONFIG_REG_KEY, 0, KEY_READ, &key)==ERROR_SUCCESS) {
  DWORD type,size=sizeof(DWORD),v;
  if(RegQueryValueExW(key,CONFIG_BUBBLES,0,&type,(BYTE*)&v,&size)==ERROR_SUCCESS) g_bubble_count_setting=(int)v;
  size=sizeof(DWORD); if(RegQueryValueExW(key,CONFIG_SPEED,0,&type,(BYTE*)&v,&size)==ERROR_SUCCESS) g_speed_setting=(int)v;
  size=sizeof(DWORD); if(RegQueryValueExW(key,CONFIG_BRIGHTNESS,0,&type,(BYTE*)&v,&size)==ERROR_SUCCESS) g_brightness_setting=(int)v;
  RegCloseKey(key);
 }
 if(g_bubble_count_setting<1) g_bubble_count_setting=1;
 if(g_bubble_count_setting>g_bubble_max) g_bubble_count_setting=g_bubble_max;
 if(g_speed_setting<25) g_speed_setting=25;
 if(g_speed_setting>250) g_speed_setting=250;
 if(g_brightness_setting<25) g_brightness_setting=25;
 if(g_brightness_setting>150) g_brightness_setting=150;
}
static void save_settings(void) {
 HKEY key; DWORD disp;
 if(RegCreateKeyExW(HKEY_CURRENT_USER,CONFIG_REG_KEY,0,NULL,0,KEY_WRITE,NULL,&key,&disp)==ERROR_SUCCESS) {
  DWORD v=(DWORD)g_bubble_count_setting; RegSetValueExW(key,CONFIG_BUBBLES,0,REG_DWORD,(BYTE*)&v,sizeof(v));
  v=(DWORD)g_speed_setting; RegSetValueExW(key,CONFIG_SPEED,0,REG_DWORD,(BYTE*)&v,sizeof(v));
  v=(DWORD)g_brightness_setting; RegSetValueExW(key,CONFIG_BRIGHTNESS,0,REG_DWORD,(BYTE*)&v,sizeof(v));
  RegCloseKey(key);
 }
}
static void fill_gradient(HDC hdc,const RECT *rc) {
 int h=rc->bottom-rc->top;
 for(int y=0;y<h;y++) {
  float t=h>1?(float)y/(float)(h-1):0;
  int k=g_brightness_setting;
  int r=(int)((7+24*t)*k/100), g=(int)((22+52*t)*k/100), b=(int)((56+105*t)*k/100);
  if(r>255)r=255;if(g>255)g=255;if(b>255)b=255;
  HBRUSH brush=CreateSolidBrush(RGB(r,g,b)); RECT line={rc->left,rc->top+y,rc->right,rc->top+y+1};
  FillRect(hdc,&line,brush); DeleteObject(brush);
 }
}
static void draw_bubble(HDC hdc,int cx,int cy,int radius,BYTE alpha) {
 HDC mem=CreateCompatibleDC(hdc); HBITMAP bm=CreateCompatibleBitmap(hdc,radius*2+8,radius*2+8); HGDIOBJ old=SelectObject(mem,bm);
 RECT r={0,0,radius*2+8,radius*2+8}; HBRUSH clear=CreateSolidBrush(RGB(12,40,85)); FillRect(mem,&r,clear); DeleteObject(clear);
 HPEN pen=CreatePen(PS_SOLID,2,RGB(135,205,255)); HGDIOBJ op=SelectObject(mem,pen); HGDIOBJ ob=SelectObject(mem,GetStockObject(NULL_BRUSH));
 Ellipse(mem,4,4,radius*2+4,radius*2+4); SelectObject(mem,ob); SelectObject(mem,op); DeleteObject(pen);
 BLENDFUNCTION blend={AC_SRC_OVER,0,alpha,0}; SIZE size={radius*2+8,radius*2+8};
 AlphaBlend(hdc,cx-radius-4,cy-radius-4,size.cx,size.cy,mem,0,0,size.cx,size.cy,blend);
 SelectObject(mem,old); DeleteObject(bm); DeleteDC(mem);
}
static LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
 switch(msg) {
 case WM_CREATE: GetCursorPos(&g_start_cursor); g_start_time=GetTickCount(); SetTimer(hwnd,1,16,NULL); return 0;
 case WM_TIMER:
  g_time+=0.016f*(float)g_speed_setting/100.0f;
  for(int i=0;i<g_bubble_max;i++){Bubble *b=&g_bubbles[i]; b->x+=b->vx*(float)g_speed_setting/100.0f; b->y+=b->vy*(float)g_speed_setting/100.0f;
   if(b->x<-.08f)b->x=1.08f;if(b->x>1.08f)b->x=-.08f;if(b->y<-.08f)b->y=1.08f;if(b->y>1.08f)b->y=-.08f;}
  InvalidateRect(hwnd,NULL,FALSE); return 0;
 case WM_PAINT:{
  PAINTSTRUCT ps; HDC hdc=BeginPaint(hwnd,&ps); RECT rc; GetClientRect(hwnd,&rc); fill_gradient(hdc,&rc);
  int w=rc.right,h=rc.bottom;
  for(int i=0;i<g_bubble_count_setting;i++){Bubble *b=&g_bubbles[i];int cx=(int)(b->x*w),cy=(int)(b->y*h+sinf(g_time*.7f+b->phase)*18);int rad=(int)(b->size*(float)(w<h?w:h));if(rad<18)rad=18;draw_bubble(hdc,cx,cy,rad,48);}
  SetBkMode(hdc,TRANSPARENT);SetTextColor(hdc,RGB(225,242,255));
  HFONT font=CreateFontW(-(h>700?42:30),0,0,0,FW_LIGHT,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI Light");
  HFONT old=(HFONT)SelectObject(hdc,font);RECT title=rc;title.top=h/2-35;title.bottom=h/2+15;DrawTextW(hdc,L"CarsonScreenSaver",-1,&title,DT_CENTER|DT_VCENTER|DT_SINGLELINE);SelectObject(hdc,old);DeleteObject(font);EndPaint(hwnd,&ps);return 0;
 }
 case WM_MOUSEMOVE:
  if(!g_preview&&!g_config_mode&&GetTickCount()-g_start_time>1200){POINT pt;GetCursorPos(&pt);if(pt.x!=g_start_cursor.x||pt.y!=g_start_cursor.y){g_running=FALSE;for(int i=0;i<g_window_count;i++)if(g_windows[i]&&g_windows[i]!=hwnd)DestroyWindow(g_windows[i]);DestroyWindow(hwnd);PostQuitMessage(0);return 0;}}
  break;
 case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN: case WM_KEYDOWN: case WM_SYSKEYDOWN:
  if(!g_preview&&!g_config_mode){g_running=FALSE;for(int i=0;i<g_window_count;i++)if(g_windows[i]&&g_windows[i]!=hwnd)DestroyWindow(g_windows[i]);DestroyWindow(hwnd);PostQuitMessage(0);return 0;} break;
 case WM_DESTROY: KillTimer(hwnd,1);g_running=FALSE;PostQuitMessage(0);return 0;
 }
 return DefWindowProcW(hwnd,msg,wParam,lParam);
}
static BOOL CALLBACK create_monitor_window(HMONITOR monitor,HDC dc,LPRECT unused,LPARAM data) {
 (void)dc;(void)unused;(void)data;
 if(g_window_count>=16)return FALSE;
 MONITORINFO mi={sizeof(mi)};GetMonitorInfoW(monitor,&mi);
 RECT r=mi.rcMonitor;HINSTANCE instance=GetModuleHandleW(NULL);
 HWND h=CreateWindowExW(WS_EX_TOPMOST,L"CarsonScreenSaverWindow",L"CarsonScreenSaver",WS_POPUP,
   r.left,r.top,r.right-r.left,r.bottom-r.top,NULL,NULL,instance,NULL);
 if(h){g_windows[g_window_count++]=h;ShowWindow(h,SW_SHOW);UpdateWindow(h);}
 return TRUE;
}
static int run_screensaver(HWND parent) {
 HINSTANCE instance=GetModuleHandleW(NULL); WNDCLASSW wc={0};wc.hInstance=instance;wc.lpfnWndProc=WndProc;wc.lpszClassName=L"CarsonScreenSaverWindow";wc.hCursor=LoadCursorW(NULL,IDC_ARROW);wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);RegisterClassW(&wc);
 g_preview=parent!=NULL;g_running=TRUE;g_window_count=0;
 if(parent){
  RECT rc;GetClientRect(parent,&rc);
  g_windows[0]=CreateWindowExW(WS_EX_TOPMOST,wc.lpszClassName,L"CarsonScreenSaver",WS_CHILD,0,0,rc.right,rc.bottom,parent,NULL,instance,NULL);
  if(!g_windows[0])return 1;
  g_window_count=1;ShowWindow(g_windows[0],SW_SHOW);UpdateWindow(g_windows[0]);
 }else{
  ShowCursor(FALSE);
  EnumDisplayMonitors(NULL,NULL,create_monitor_window,0);
  if(g_window_count==0){ShowCursor(TRUE);return 1;}
 }
 MSG msg;while(g_running&&GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
 if(!parent)ShowCursor(TRUE);return 0;
}
static void show_config(HWND parent) {
 load_settings();
 wchar_t text[1024];
 wsprintfW(text,L"CarsonScreenSaver - Windows 7 Aero inspired\n\nBubbles: %d of %d\nSpeed: %d%%\nBrightness: %d%%\n\n"
                 L"To change these settings, use the sliders below.",g_bubble_count_setting,g_bubble_max,g_speed_setting,g_brightness_setting);
 int r=MessageBoxW(parent,text,L"CarsonScreenSaver Configuration",MB_OKCANCEL|MB_ICONINFORMATION);
 if(r==IDCANCEL)return;
 // Simple configuration dialog implemented with standard Windows controls.
 WNDCLASSW wc={0}; HINSTANCE instance=GetModuleHandleW(NULL); wc.hInstance=instance; wc.lpfnWndProc=DefWindowProcW; wc.lpszClassName=L"CarsonScreenSaverConfig";
 HWND dlg=CreateWindowExW(WS_EX_DLGMODALFRAME|WS_EX_TOPMOST,L"CarsonScreenSaverConfig",L"CarsonScreenSaver Settings",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_VISIBLE,
   CW_USEDEFAULT,CW_USEDEFAULT,420,250,parent,NULL,instance,NULL);
 if(!dlg)return;
 CreateWindowW(L"STATIC",L"Bubble count (1-8)",WS_CHILD|WS_VISIBLE,25,25,160,22,dlg,NULL,instance,NULL);
 HWND bubbles=CreateWindowW(L"EDIT",L"5",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_NUMBER,200,22,80,25,dlg,NULL,instance,NULL);
 CreateWindowW(L"STATIC",L"Speed (25-250%)",WS_CHILD|WS_VISIBLE,25,70,160,22,dlg,NULL,instance,NULL);
 HWND speed=CreateWindowW(L"EDIT",L"100",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_NUMBER,200,67,80,25,dlg,NULL,instance,NULL);
 CreateWindowW(L"STATIC",L"Brightness (25-150%)",WS_CHILD|WS_VISIBLE,25,115,160,22,dlg,NULL,instance,NULL);
 HWND bright=CreateWindowW(L"EDIT",L"100",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_NUMBER,200,112,80,25,dlg,NULL,instance,NULL);
 HWND ok=CreateWindowW(L"BUTTON",L"Save",WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,185,165,90,30,dlg,(HMENU)IDOK,instance,NULL);
 HWND cancel=CreateWindowW(L"BUTTON",L"Cancel",WS_CHILD|WS_VISIBLE,285,165,90,30,dlg,(HMENU)IDCANCEL,instance,NULL);
 MSG msg;
 while(IsWindow(dlg)&&GetMessageW(&msg,NULL,0,0)>0){if(msg.message==WM_KEYDOWN&&msg.wParam==VK_ESCAPE){DestroyWindow(dlg);break;} if(msg.message==WM_COMMAND&&LOWORD(msg.wParam)==IDOK){wchar_t buf[32];GetWindowTextW(bubbles,buf,32);g_bubble_count_setting=_wtoi(buf);GetWindowTextW(speed,buf,32);g_speed_setting=_wtoi(buf);GetWindowTextW(bright,buf,32);g_brightness_setting=_wtoi(buf);load_settings();save_settings();DestroyWindow(dlg);break;} if(msg.message==WM_COMMAND&&LOWORD(msg.wParam)==IDCANCEL){DestroyWindow(dlg);break;} TranslateMessage(&msg);DispatchMessageW(&msg);}
 (void)ok;(void)cancel;
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,PWSTR commandLine,int show) {
 (void)instance;(void)previous;(void)show;load_settings();
 wchar_t *cmd=commandLine;while(*cmd==L' '||*cmd==L'\t')cmd++;
 if(*cmd==L'/'||*cmd==L'-'){wchar_t option=cmd[1];
  if(option==L'c'||option==L'C'){g_config_mode=TRUE;show_config(NULL);return 0;}
  if(option==L'p'||option==L'P'){cmd+=2;while(*cmd==L' '||*cmd==L'\t')cmd++;HWND preview=(HWND)(ULONG_PTR)wcstoull(cmd,NULL,10);if(preview)return run_screensaver(preview);return 0;}
  if(option==L's'||option==L'S')return run_screensaver(NULL);
 }
 return run_screensaver(NULL);
}
