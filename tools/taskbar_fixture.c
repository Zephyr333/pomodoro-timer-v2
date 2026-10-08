#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
static HANDLE entered;
static int delay_once=1;
static LRESULT CALLBACK FixtureWndProc(HWND w,UINT m,WPARAM a,LPARAM b) {
    if(m==WM_SHOWWINDOW&&!a&&delay_once){delay_once=0;SetEvent(entered);Sleep(1500);}
    if(m==WM_DESTROY){PostQuitMessage(0);return 0;}return DefWindowProcW(w,m,a,b);
}
int wmain(int argc,wchar_t **argv) {
    if(argc<3)return 2;entered=OpenEventW(EVENT_MODIFY_STATE,FALSE,argv[2]);if(!entered)return 3;
    WNDCLASSW wc={0};wc.hInstance=GetModuleHandleW(NULL);wc.lpfnWndProc=FixtureWndProc;wc.lpszClassName=L"DFTaskbar:SlowCrossProcessFixture";RegisterClassW(&wc);
    HWND w=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"taskbar test fixture",WS_POPUP,10,10,32,24,NULL,NULL,wc.hInstance,NULL);
    if(!w)return 4;ShowWindow(w,SW_SHOWNOACTIVATE);ShowWindow(w,SW_SHOWNOACTIVATE);
    FILE *f=_wfopen(argv[1],L"wb");if(!f)return 5;fprintf(f,"%llu %lu\n",(unsigned long long)(UINT_PTR)w,GetCurrentProcessId());fclose(f);
    MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}CloseHandle(entered);return 0;
}
