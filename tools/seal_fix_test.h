/* v3.0.8: only the changed data, layer and focus contracts. */
static void seal_fixture(void) {
    FILE *f=_wfopen(g_log_path,L"wb");CHECK(f!=NULL,"large independent fixture opens");if(!f)return;
    fputs("2019-01-15,12:00:00,1\n",f);for(int i=0;i<160000;++i)fputs("# fixture padding\n",f);fclose(f);
}
static void seal_stats(void) {
    seal_fixture();wchar_t monthly[MAX_PATH];swprintf(monthly,MAX_PATH,L"%ls\\pomodoro_log_2019-01.csv",g_data_dir);
    FILE *f=_wfopen(monthly,L"wb");fputs("2019-01-15,12:00:01,2\n",f);fclose(f);load_heatmap_data();CHECK(get_day_count_value(2019,1,15)==3,"legacy main and month remain readable");
    HANDLE month=CreateFileW(monthly,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    CHECK(month!=INVALID_HANDLE_VALUE,"old archive locked against writes");CHECK(record_completed_pomodoros(1),"new record succeeds without writing old archive");
    load_heatmap_data();CHECK(get_day_count_value(2019,1,15)==3,"large log never duplicates or removes historical records");CloseHandle(month);
    HANDLE main=CreateFileW(g_log_path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    CHECK(!record_completed_pomodoros(1),"locked main replacement fails explicitly");load_heatmap_data();CHECK(get_day_count_value(2019,1,15)==3,"locked main cannot duplicate archive records");CloseHandle(main);
    test_fail_stats_commit=1;CHECK(!record_completed_pomodoros(1),"injected commit failure is not success");test_fail_stats_commit=0;
    load_heatmap_data();CHECK(get_day_count_value(2019,1,15)==3,"failed commit preserves original history");
    WIN32_FILE_ATTRIBUTE_DATA info;GetFileAttributesExW(monthly,GetFileExInfoStandard,&info);CHECK(info.nFileSizeLow==22,"old monthly bytes remain unchanged");
    {char bytes[32]={0};f=_wfopen(monthly,L"rb");CHECK(f!=NULL,"old archive remains readable");if(f){size_t size=fread(bytes,1,sizeof(bytes),f);fclose(f);CHECK(size==22&&!memcmp(bytes,"2019-01-15,12:00:01,2\n",22),"old archive exact bytes are unchanged");}}
    wchar_t keep[MAX_PATH];swprintf(keep,MAX_PATH,L"%ls.keep",g_log_path);CHECK(GetFileAttributesW(keep)==INVALID_FILE_ATTRIBUTES,"no old archive keep file created");
}
static void seal_editors(void) {
    FullscreenMonitors online;fs_get_monitors(&online);FullscreenColorDraft draft={0};draft.index=0;draft.color=settings.fullscreen_colors[0];draft.scale=100;wcscpy(draft.font,L"Segoe UI");
    HWND dialog=CreateDialogParamW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(IDD_FULLSCREEN_COLORS),g_main_hwnd,FullscreenColorsDlgProc,(LPARAM)&draft);ShowWindow(dialog,SW_SHOW);
    CHECK(!(GetWindowLongW(dialog,GWL_EXSTYLE)&WS_EX_TOPMOST),"color editor is normal without fullscreen");
    HANDLE dpi=fs_enter_dpi();RECT a=online.items[0].info.rcMonitor;SetWindowPos(dialog,NULL,a.left+100,a.top+100,300,240,SWP_NOZORDER|SWP_NOACTIVATE);fs_leave_dpi(dpi);
    fs_begin();fs_add_screen(&online.items[0]);fs_maintain_layer(1);CHECK(GetPropW(dialog,fs_aux_topmost_prop)!=NULL,"selected primary monitor temporarily protects editor");
    if(online.count>1){RECT b=online.items[1].info.rcMonitor;dpi=fs_enter_dpi();SetWindowPos(dialog,NULL,b.left+50,b.top+100,300,240,SWP_NOZORDER|SWP_NOACTIVATE);fs_leave_dpi(dpi);fs_maintain_layer(1);CHECK(!(GetWindowLongW(dialog,GWL_EXSTYLE)&WS_EX_TOPMOST),"unselected primary monitor releases editor");
        POINT edge={a.right-1,a.top+100};if(PtInRect(&b,(POINT){a.right+5,a.top+100})){dpi=fs_enter_dpi();SetWindowPos(dialog,NULL,edge.x,edge.y,300,240,SWP_NOZORDER|SWP_NOACTIVATE);fs_leave_dpi(dpi);fs_maintain_layer(1);CHECK(!GetPropW(dialog,fs_aux_topmost_prop),"one-pixel contact with selected monitor is insufficient");}}
    FullscreenView view;fs_read_timer_view(&view);fs_start_preview(dialog,IDC_FS_EDIT_FIRST,&view);fs_end_preview();CHECK(GetFocus()==GetDlgItem(dialog,IDC_FS_EDIT_FIRST),"preview returns input focus");
    fs_exit();CHECK(!(GetWindowLongW(dialog,GWL_EXSTYLE)&WS_EX_TOPMOST),"preview return and exit do not leave permanent topmost");
    fs_begin();fs_add_screen(&online.items[0]);fs_start_preview(dialog,IDC_FS_EDIT_FIRST,&view);fs_shutdown();CHECK(!fs_active&&!fs_count&&!fs_guard_timer_active&&!fs_taskbar_thread&&!fs_hidden_count,"application shutdown during preview releases every coverage lease");
    DestroyWindow(dialog);if(draft.preview_font)DeleteObject(draft.preview_font);fs_editor_dialog=NULL;
    FullscreenSignatureDraft sig={0};sig.color=settings.fullscreen_colors[5];sig.scale=100;wcscpy(sig.font,L"Segoe UI");
    dialog=CreateDialogParamW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(IDD_FULLSCREEN_SIGNATURE),g_main_hwnd,FullscreenSignatureDlgProc,(LPARAM)&sig);ShowWindow(dialog,SW_SHOW);CHECK(!(GetWindowLongW(dialog,GWL_EXSTYLE)&WS_EX_TOPMOST),"signature editor is normal without fullscreen");DestroyWindow(dialog);if(sig.preview_font)DeleteObject(sig.preview_font);fs_editor_dialog=NULL;
}
static HWND seal_bar(const FullscreenMonitor *monitor,const wchar_t *cls,int visible) {
    WNDCLASSW wc={0};wc.hInstance=GetModuleHandleW(NULL);wc.lpfnWndProc=DefWindowProcW;wc.lpszClassName=cls;RegisterClassW(&wc);
    RECT r=monitor->info.rcMonitor;HANDLE dpi=fs_enter_dpi();HWND w=CreateWindowExW(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,cls,L"fixture taskbar",WS_POPUP,r.left,r.bottom-32,r.right-r.left,32,NULL,NULL,wc.hInstance,NULL);fs_leave_dpi(dpi);
    if(visible){ShowWindow(w,SW_SHOWNOACTIVATE);ShowWindow(w,SW_SHOWNOACTIVATE);}return w;
}
static void seal_guard(void) {
    FullscreenMonitors online;fs_get_monitors(&online);HWND bars[FS_MAX_MONITORS];for(size_t i=0;i<online.count;++i)bars[i]=seal_bar(&online.items[i],L"DFTaskbar:Fixture",1);
    HWND preview=seal_bar(&online.items[0],L"DFTaskbarButtonPreviewModernStyle:Fixture",1),hidden=seal_bar(&online.items[0],L"DFTaskbar:HiddenFixture",0);
    settings.reminder_mode=2;start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(30);
    CHECK(sr.active&&!fs_active&&fs_guard_timer_active,"temporary strong-only coverage starts shared guard");
    for(size_t i=0;i<online.count;++i)CHECK(!IsWindowVisible(bars[i]),"temporary strong hides covered fixture root taskbar");CHECK(IsWindowVisible(preview),"taskbar thumbnail is excluded");
    HDC buffer=sr.items[0].dc;HBITMAP bitmap=sr.items[0].bitmap;int positions=test_position_calls;for(int i=0;i<5;++i){sr.started=GetTickCount64()-i*1000;sr_tick();}
    CHECK(test_position_calls==positions&&buffer==sr.items[0].dc&&bitmap==sr.items[0].bitmap,"color changes never reorder or reallocate unchanged buffer");
    positions=test_position_calls;fs_maintain_layer(0);fs_maintain_layer(0);CHECK(positions==test_position_calls,"stable watchdog performs no position changes");
    sr_dismiss(0);pump(30);CHECK(!fs_guard_timer_active&&!fs_foreground_hook&&!fs_show_hook&&!fs_reorder_hook,"last coverage removes hooks and timer");for(size_t i=0;i<online.count;++i)CHECK(IsWindowVisible(bars[i]),"taskbar restored when strong-only coverage ends");CHECK(!IsWindowVisible(hidden),"originally hidden taskbar remains hidden");
    fs_begin();fs_add_screen(&online.items[0]);start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(20);
    CHECK(sr.count==1&&!IsWindowVisible(bars[0]),"partial ordinary and reminder coverage preserved");if(online.count>1)CHECK(IsWindowVisible(bars[1]),"unselected taskbar untouched");
    if(online.count>1){fs_add_screen(&online.items[1]);SetWindowPos(bars[1],HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);fs_maintain_layer(1);pump(30);
        CHECK(!IsWindowVisible(bars[1])&&fs_window_above(fs_windows[1],bars[1]),"ordinary screen outside reminder repairs root taskbar order");
        fs_remove_window(1);pump(30);CHECK(IsWindowVisible(bars[1]),"released ordinary screen restores taskbar while reminder remains");}
    fs_exit();CHECK(sr.active&&fs_guard_timer_active&&!IsWindowVisible(bars[0]),"ordinary exit cannot stop active reminder protection");sr_dismiss(0);pump(30);
    for(size_t i=0;i<online.count;++i)DestroyWindow(bars[i]);DestroyWindow(hidden);DestroyWindow(preview);
}
static void test_seal_fixes(void) {
    TimerSettings saved=settings;test_notify_stub=test_silent_sound=1;settings.enable_clock_sound=settings.enable_completion_sound=0;settings.reminder_mode=0;
    consistency_reset();seal_stats();consistency_reset();seal_editors();consistency_reset();seal_guard();consistency_reset();settings=saved;test_notify_stub=test_silent_sound=0;
    printf("SEAL_FIX_TEST: %s (%d failures)\n",failures?"FAIL":"PASS",failures);
}
static HWND live_bars[32];static int live_visible[32];static size_t live_bar_count;
static BOOL CALLBACK live_collect_bar(HWND window,LPARAM unused) {
    (void)unused;if(!fs_is_suppressible_taskbar(window)||live_bar_count>=32)return TRUE;
    live_bars[live_bar_count]=window;live_visible[live_bar_count]=IsWindowVisible(window);++live_bar_count;return TRUE;
}
static void seal_pump_bounded(unsigned milliseconds) {
    ULONGLONG end=GetTickCount64()+milliseconds;MSG message;
    do {int count=0;while(count++<64&&PeekMessageW(&message,NULL,0,0,PM_REMOVE)){if(message.message!=WM_QUIT){TranslateMessage(&message);DispatchMessageW(&message);}}Sleep(1);}while(GetTickCount64()<end);
}
static COLORREF live_read_pixel(HDC screen, HDC mem, int x, int y) {
    if (mem && BitBlt(mem, 0, 0, 1, 1, screen, x, y, SRCCOPY)) return GetPixel(mem, 0, 0);
    return GetPixel(screen, x, y);
}
static void test_guard_live(int scenario) {
    TimerSettings saved=settings;FullscreenMonitors online;HWND original=GetForegroundWindow();fs_get_monitors(&online);
    test_notify_stub=test_silent_sound=1;settings.reminder_mode=2;settings.enable_clock_sound=settings.enable_completion_sound=0;
    settings.fullscreen_show_signature=1;wcscpy(settings.fullscreen_signature,L"测试中：请等待自动结束");
    consistency_reset();live_bar_count=0;EnumWindows(live_collect_bar,0);
    size_t selected=(scenario==3&&online.count>2)?2:(online.count>1?1:0);HWND editor=NULL;FullscreenColorDraft draft={0};
    if(scenario==1||scenario==3){fs_begin();fs_add_screen(&online.items[selected]);}
    if(scenario==2)fs_show_all();
    if(scenario==3){draft.index=0;draft.color=settings.fullscreen_colors[0];draft.scale=100;wcscpy(draft.font,L"Segoe UI");editor=CreateDialogParamW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(IDD_FULLSCREEN_COLORS),g_main_hwnd,FullscreenColorsDlgProc,(LPARAM)&draft);ShowWindow(editor,SW_SHOW);FullscreenView view;fs_read_timer_view(&view);fs_start_preview(editor,IDC_FS_EDIT_FIRST,&view);}
    start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(100);
    CHECK(sr.active,"live reminder created");unsigned unexpected=0,samples=0,visible_samples=0,above_samples=0,bar_pixels=0;ULONGLONG end=GetTickCount64()+20500;
    {wchar_t cls[256];GetClassNameW(GetForegroundWindow(),cls,256);printf("LIVE foreground=%ls reminder-owned=%d\n",cls,sr_owns_foreground(GetForegroundWindow()));}
    while(GetTickCount64()<end&&sr.active){seal_pump_bounded(10);HANDLE dpi=fs_enter_dpi();HDC screen=GetDC(NULL);
        HDC mem=CreateCompatibleDC(screen);HBITMAP bmp=CreateCompatibleBitmap(screen,1,1);HGDIOBJ old_bmp=mem?SelectObject(mem,bmp):NULL;
        ULONGLONG position=(GetTickCount64()-sr.started)%1000;
        for(size_t i=0;i<live_bar_count;++i)if(IsWindow(live_bars[i])&&fs_is_on_fullscreen_monitor_handle(live_bars[i])){++samples;if(IsWindowVisible(live_bars[i]))++visible_samples;
            HMONITOR monitor=MonitorFromWindow(live_bars[i],MONITOR_DEFAULTTONULL);HWND overlay=NULL;int white=0;
            for(size_t j=0;j<sr.count;++j)if(sr.items[j].monitor.handle==monitor){overlay=sr.items[j].window;white=sr.last_white;break;}
            if(!overlay)for(size_t j=0;j<fs_count;++j){FullscreenMonitor *item=(FullscreenMonitor*)GetWindowLongPtrW(fs_windows[j],GWLP_USERDATA);if(item&&item->handle==monitor){overlay=fs_windows[j];break;}}
            if(IsWindowVisible(live_bars[i])&&overlay&&fs_window_above(live_bars[i],overlay)){if(!above_samples)printf("LIVE first overlap elapsed=%llu bar=%p overlay=%p\n",GetTickCount64()-sr.started,live_bars[i],overlay);++above_samples;}
            if(position>180&&position<820&&overlay){RECT r;GetWindowRect(live_bars[i],&r);COLORREF c=live_read_pixel(screen,mem,r.left+2,r.top+2);++samples;if(c!=(white?RGB(255,255,255):RGB(0,0,0))){if(!bar_pixels)printf("LIVE first pixel elapsed=%llu point=%ld,%ld expected-white=%d actual=%08lx\n",GetTickCount64()-sr.started,r.left+2,r.top+2,white,(unsigned long)c);++bar_pixels;}}}
        if(position>180&&position<820)for(size_t i=0;i<sr.count;++i){RECT r=sr.items[i].monitor.info.rcMonitor;COLORREF c=live_read_pixel(screen,mem,r.left+2,r.top+2);++samples;if(c!=(sr.last_white?RGB(255,255,255):RGB(0,0,0)))++unexpected;}
        if(mem){SelectObject(mem,old_bmp);DeleteObject(bmp);DeleteDC(mem);}
        ReleaseDC(NULL,screen);fs_leave_dpi(dpi);
    }
    printf("LIVE scenario=%d bars=%zu samples=%u visible-covered=%u abnormal-background=%u above-overlay=%u bar-pixel-errors=%u\n",scenario,live_bar_count,samples,visible_samples,unexpected,above_samples,bar_pixels);
    CHECK(sr.active&&samples>0&&!above_samples&&!bar_pixels&&!unexpected,"ten actual black-white cycles with no sampled taskbar leak or wrong background");
    consistency_reset();if(editor){CHECK(!fs_preview_active&&fs_count==1&&IsWindowVisible(editor)&&GetFocus()==GetDlgItem(editor,IDC_FS_EDIT_FIRST),"live preview returns original screen and input focus");fs_exit();DestroyWindow(editor);if(draft.preview_font)DeleteObject(draft.preview_font);fs_editor_dialog=NULL;}
    seal_pump_bounded(150);
    for(size_t i=0;i<live_bar_count;++i)if(IsWindow(live_bars[i]))CHECK(IsWindowVisible(live_bars[i])==live_visible[i],"live taskbar visibility returns to original");
    CHECK(!fs_guard_timer_active&&!fs_hidden_count&&!fs_taskbar_thread&&!fs_taskbar_target_count,"live guard and event thread cleanup complete");settings=saved;test_notify_stub=test_silent_sound=0;if(IsWindow(original)&&!ui_work_window(GetForegroundWindow(),NULL))SetForegroundWindow(original);
    printf("GUARD_LIVE_TEST: %s (%d failures)\n",failures?"FAIL":"PASS",failures);
}
static DWORD WINAPI focus_worker(void *parameter) {
    HDESK desktop=(HDESK)parameter;SetThreadDesktop(desktop);WNDCLASSW wc={0};wc.hInstance=GetModuleHandleW(NULL);wc.lpfnWndProc=DefWindowProcW;wc.lpszClassName=L"SealFocusWorker";RegisterClassW(&wc);
    HWND work=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"test working window",WS_POPUP|WS_VISIBLE,100,100,100,70,NULL,NULL,wc.hInstance,NULL);
    ShowWindow(work,SW_SHOWNOACTIVATE);ShowWindow(work,SW_SHOWNOACTIVATE);
    MSG message;while(GetMessageW(&message,NULL,0,0)>0){if(message.message==WM_APP+123){SetForegroundWindow(work);SetFocus(work);continue;}TranslateMessage(&message);DispatchMessageW(&message);}DestroyWindow(work);return 0;
}
static void test_focus_live(void) {
    TimerSettings saved=settings;FullscreenMonitors online;fs_get_monitors(&online);HWND original=GetForegroundWindow();test_notify_stub=test_silent_sound=1;settings.reminder_mode=2;
    DWORD thread;HANDLE worker=CreateThread(NULL,0,focus_worker,GetThreadDesktop(GetCurrentThreadId()),0,&thread);pump(100);
    HWND work=FindWindowW(L"SealFocusWorker",NULL);CHECK(work!=NULL,"cross-thread working window exists");
    start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(30);PostThreadMessageW(thread,WM_APP+123,0,0);pump(100);
    CHECK(GetForegroundWindow()==work,"actual foreground belongs to different thread");FullscreenMonitors reduced=online;memmove(reduced.items,reduced.items+1,(reduced.count-1)*sizeof(reduced.items[0]));--reduced.count;
    sr_reconcile_monitors(&reduced);CHECK(GetForegroundWindow()==work,"simulated removal respects external-thread foreground");
    sr_reconcile_monitors(&(FullscreenMonitors){0});CHECK(!sr.active&&GetForegroundWindow()==work,"last reminder removal keeps new working foreground");
    consistency_reset();PostThreadMessageW(thread,WM_QUIT,0,0);WaitForSingleObject(worker,2000);CloseHandle(worker);settings=saved;test_notify_stub=test_silent_sound=0;if(IsWindow(original))SetForegroundWindow(original);
    printf("FOCUS_LIVE_TEST: %s (%d failures)\n",failures?"FAIL":"PASS",failures);
}
