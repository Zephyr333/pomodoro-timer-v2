#ifndef POMODORO_STRONG_REMINDER_H
#define POMODORO_STRONG_REMINDER_H
/* Presentation-only overlay, independent of normal fullscreen and editor previews. */
static struct {
    int active;
    unsigned generation;
    ULONGLONG started;
    HWND foreground, focus;
    FullscreenView last_view;
    int last_white;
    struct { HWND window; FullscreenMonitor monitor; } items[FS_MAX_MONITORS];
    size_t count;
} sr;
static int sr_active(void) { return sr.active; }
static void sr_track_focus(HWND candidate) {
    if(!sr.active)return;
    UiFocusReturn context={sr.foreground,sr.focus};ui_focus_remember(&context,candidate,NULL);
    sr.foreground=context.foreground;sr.focus=context.focus;
}
static void sr_mouse_hint(wchar_t *out, size_t capacity) {
    swprintf(out,capacity,L"左键 / 中键 / Esc：关闭提醒　右键：%ls",timer_ui_primary_label());
}
static HWND sr_bottom_window(void) {
    HWND bottom=NULL,w;size_t i;
    if(!sr.active)return NULL;
    for(w=GetTopWindow(NULL);w;w=GetWindow(w,GW_HWNDNEXT))
        for(i=0;i<sr.count;++i)if(w==sr.items[i].window){bottom=w;break;}
    return bottom;
}
static void sr_maintain_layer(void) {
    size_t i;
    if (!sr.active) return;
    for(i=0;i<sr.count;++i) SetWindowPos(sr.items[i].window,HWND_TOPMOST,0,0,0,0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
}
static void sr_dismiss(int restore) {
    size_t i;
    HWND foreground=sr.foreground, focus=sr.focus;
    HWND current=GetForegroundWindow();
    if(!IsWindow(current)) current=GetActiveWindow();
    int owned=0;
    for(i=0;i<sr.count;++i) if(current==sr.items[i].window) owned=1;
    if (!sr.active && !sr.count) return;
    sr.active=0;
    KillTimer(g_main_hwnd,ID_STRONG_REMINDER_TIMER);
    for(i=0;i<sr.count;++i) DestroyWindow(sr.items[i].window);
    sr.count=0;
    sr.foreground=NULL;sr.focus=NULL;
    SetCursor(LoadCursor(NULL,IDC_ARROW));
    if(restore && (owned || current==g_main_hwnd || !IsWindow(current))) {
        UiFocusReturn context={foreground,focus};ui_focus_apply(context);
    }
}
static void sr_validate(void) {
    if(sr.active && (settings.reminder_mode!=2 || sr.generation!=timer_stage_generation)) sr_dismiss(1);
}
static int sr_white(void) { return ((GetTickCount64()-sr.started)/1000)%2==0; }
static void sr_paint(HWND window) {
    PAINTSTRUCT paint;RECT bounds;FullscreenView view;
    HDC target=BeginPaint(window,&paint), dc=CreateCompatibleDC(target);
    GetClientRect(window,&bounds);
    HBITMAP bitmap=CreateCompatibleBitmap(target,max(1,bounds.right),max(1,bounds.bottom));
    if(dc && bitmap) {
        HGDIOBJ previous=SelectObject(dc,bitmap);
        fs_read_timer_view(&view);
        fs_draw_view_ex(dc,bounds,&view,1,1,sr.last_white);
        BitBlt(target,0,0,bounds.right,bounds.bottom,dc,0,0,SRCCOPY);
        SelectObject(dc,previous);
    } else FillRect(target,&bounds,(HBRUSH)GetStockObject(sr.last_white?WHITE_BRUSH:BLACK_BRUSH));
    if(bitmap) DeleteObject(bitmap);if(dc) DeleteDC(dc);
    EndPaint(window,&paint);
}
static LRESULT CALLBACK StrongReminderWndProc(HWND window,UINT message,WPARAM wParam,LPARAM lParam) {
    switch(message) {
        case WM_PAINT:sr_paint(window);return 0;
        case WM_ERASEBKGND:return 1;
        case WM_MOUSEACTIVATE:sr_track_focus(ui_foreground());return MA_ACTIVATE;
        case WM_ACTIVATE:
            if(LOWORD(wParam)!=WA_INACTIVE)sr_track_focus((HWND)lParam);
            break;
        case WM_SETCURSOR:SetCursor(LoadCursor(NULL,IDC_ARROW));return TRUE;
        case WM_LBUTTONDOWN:case WM_MBUTTONDOWN:case WM_RBUTTONDOWN:case WM_XBUTTONDOWN:
        case WM_LBUTTONDBLCLK:case WM_MBUTTONDBLCLK:case WM_RBUTTONDBLCLK:return 0;
        case WM_LBUTTONUP:case WM_MBUTTONUP:case WM_XBUTTONUP:case WM_CLOSE:sr_dismiss(1);return 0;
        case WM_RBUTTONUP: {
            unsigned generation=sr.generation;
            timer_sync_clock(g_main_hwnd);
            sr_dismiss(1);
            if(generation==timer_stage_generation) timer_primary_action(g_main_hwnd);
            return 0;
        }
        case WM_KEYDOWN:if(wParam==VK_ESCAPE){sr_dismiss(1);return 0;}break;
        case WM_DISPLAYCHANGE:case 0x02E0:PostMessageW(g_main_hwnd,WM_FS_LAYOUT,0,0);return 0;
    }
    return DefWindowProcW(window,message,wParam,lParam);
}
static int sr_selected(const wchar_t *identity) {
    size_t i;
    if(fs_preview_active) {
        if(!fs_preview_selection_count) return 1;
        for(i=0;i<fs_preview_selection_count;++i) if(!wcscmp(identity,fs_preview_selection[i])) return 1;
        return 0;
    }
    if(!fs_active || !fs_count) return 1;
    return fs_window_index(identity)>=0;
}
static int sr_start(void) {
    FullscreenMonitors online;size_t i;HANDLE dpi;int failed=0;
    WNDCLASSW wc={0};
    if(sr.active && sr.generation==timer_stage_generation) return 1;
    sr_dismiss(0);
    if(!fs_get_monitors(&online)) return 0;
    wc.hInstance=GetModuleHandleW(NULL);wc.lpfnWndProc=StrongReminderWndProc;wc.lpszClassName=L"PomodoroStrongReminder";
    if(!RegisterClassW(&wc) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS) return 0;
    UiFocusReturn context=ui_dialog_return();sr.foreground=context.foreground;sr.focus=context.focus;memset(&sr.last_view,0,sizeof(sr.last_view));sr.last_white=1;
    sr.generation=timer_stage_generation;sr.started=GetTickCount64();sr.active=1;
    close_toast_notification_if_open();
    dpi=fs_enter_dpi();
    for(i=0;i<online.count;++i) {
        FullscreenMonitor *monitor=&online.items[i];RECT r=monitor->info.rcMonitor;
        if(!sr_selected(monitor->identity)) continue;
        HWND window=CreateWindowExW(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,wc.lpszClassName,L"到点提醒",WS_POPUP,
            r.left,r.top,r.right-r.left,r.bottom-r.top,NULL,NULL,wc.hInstance,NULL);
        if(!window) { failed=1;break; }
        sr.items[sr.count].window=window;sr.items[sr.count].monitor=*monitor;++sr.count;
        ShowWindow(window,SW_SHOWNOACTIVATE);UpdateWindow(window);
    }
    fs_leave_dpi(dpi);
    if(failed || !sr.count || !SetTimer(g_main_hwnd,ID_STRONG_REMINDER_TIMER,100,NULL)) { sr_dismiss(1);return 0; }
    sr_maintain_layer();SetForegroundWindow(sr.items[0].window);SetFocus(sr.items[0].window);
    return 1;
}
static void sr_reconcile_monitors(const FullscreenMonitors *list) {
    FullscreenMonitors online=*list;size_t i=0,j;HANDLE dpi;int lost_focus=0;
    if(!sr.active) return;
    dpi=fs_enter_dpi();
    while(i<sr.count) {
        for(j=0;j<online.count;++j) if(!wcscmp(sr.items[i].monitor.identity,online.items[j].identity)) break;
        if(j==online.count) {
            if(GetFocus()==sr.items[i].window || GetForegroundWindow()==sr.items[i].window) lost_focus=1;
            DestroyWindow(sr.items[i].window);
            memmove(sr.items+i,sr.items+i+1,(sr.count-i-1)*sizeof(sr.items[0]));--sr.count;continue;
        }
        RECT r=online.items[j].info.rcMonitor;
        sr.items[i].monitor=online.items[j];
        SetWindowPos(sr.items[i].window,NULL,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOACTIVATE|SWP_NOZORDER);
        ++i;
    }
    fs_leave_dpi(dpi);
    if(!sr.count) sr_dismiss(1);
    else {
        sr_maintain_layer();
        if(lost_focus){SetForegroundWindow(sr.items[0].window);SetFocus(sr.items[0].window);}
    }
}
static void sr_reconcile(void) {
    FullscreenMonitors list;if(sr.active && fs_get_monitors(&list)) sr_reconcile_monitors(&list);
}
static void sr_tick(void) {
    size_t i;FullscreenView view;sr_validate();if(!sr.active)return;
    sr_track_focus(ui_foreground());sr_maintain_layer();fs_read_timer_view(&view);
    int white=sr_white();
    if(white==sr.last_white && !memcmp(&view,&sr.last_view,sizeof(view))) return;
    sr.last_view=view;sr.last_white=white;
    for(i=0;i<sr.count;++i){InvalidateRect(sr.items[i].window,NULL,FALSE);UpdateWindow(sr.items[i].window);}
}
#endif
