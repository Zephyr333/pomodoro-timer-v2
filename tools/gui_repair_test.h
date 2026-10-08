/* v3.0.7: owned UI layering, safe small-icon rasterization, strong fallback. */
static HWND repair_watch[2];static int repair_layer_violations;
static int repair_above(HWND a,HWND b) {
    for(HWND w=GetTopWindow(NULL);w;w=GetWindow(w,GW_HWNDNEXT)){if(w==a)return 1;if(w==b)return 0;}return -1;
}
static void test_repair_z_observe(void) {
    if(!fs_active || sr.active)return;
    for(int n=0;n<2;++n)if(IsWindow(repair_watch[n])&&IsWindowVisible(repair_watch[n])&&!IsIconic(repair_watch[n]))
        for(size_t i=0;i<fs_count;++i){if(!IsWindowVisible(fs_windows[i]))continue;RECT a,b,overlap;GetWindowRect(repair_watch[n],&a);GetWindowRect(fs_windows[i],&b);
            if(IntersectRect(&overlap,&a,&b) && repair_above(fs_windows[i],repair_watch[n])==1)++repair_layer_violations;}
}
static void repair_layering(void) {
    FullscreenMonitors online;CHECK(fs_get_monitors(&online)&&online.count>0,"monitor inventory for auxiliary windows");
    start_timer(g_main_hwnd,45,TIMER_SHORT_POMODORO);consistency_clock();fs_show_all();
    int seconds=remaining_seconds,counts=pomodoro_count;unsigned generation=timer_stage_generation;
    HWND input=CreateDialogW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(IDD_INPUT),g_main_hwnd,InputDlgProc);
    repair_watch[0]=input;repair_layer_violations=0;test_repair_z_tracking=1;ShowWindow(input,SW_SHOW);SetActiveWindow(input);SetFocus(GetDlgItem(input,IDC_INPUT_EDIT));
    ShowHeatmapWindow(g_main_hwnd);repair_watch[1]=g_hHeatmapWnd;
    for(int i=0;i<12;++i){fs_enforce_topmost();pump(10);}
    CHECK(GetPropW(input,fs_aux_topmost_prop)!=NULL&&GetPropW(g_hHeatmapWnd,fs_aux_topmost_prop)!=NULL,"only originally normal input and statistics windows receive temporary topmost marker");
    for(size_t i=0;i<fs_count;++i){CHECK(repair_above(input,fs_windows[i])==1,"input window is above ordinary fullscreen");CHECK(repair_above(g_hHeatmapWnd,fs_windows[i])==1,"statistics is above ordinary fullscreen");}
    CHECK(SendMessageW(fs_windows[0],WM_MOUSEACTIVATE,(WPARAM)fs_windows[0],MAKELPARAM(HTCLIENT,WM_LBUTTONDOWN))==MA_NOACTIVATE,"native mouse activation keeps operation windows above fullscreen while delivering clicks");
    fs_focus_window(fs_windows[0]);
    for(size_t i=0;i<fs_count;++i)CHECK(repair_above(input,fs_windows[i])==1&&repair_above(g_hHeatmapWnd,fs_windows[i])==1,"explicit fullscreen focus cannot cover the app's operation windows");
    fs_exit();fs_show_all();
    for(size_t i=0;i<fs_count;++i)CHECK(repair_above(input,fs_windows[i])==1&&repair_above(g_hHeatmapWnd,fs_windows[i])==1,"entering fullscreen with open operation windows protects their first visible frame");
    CHECK(repair_layer_violations==0,"maintenance never briefly covers protected UI");
    HWND work=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"new focus",WS_POPUP|WS_VISIBLE,0,0,100,70,NULL,NULL,GetModuleHandleW(NULL),NULL);
    SetActiveWindow(work);SetFocus(work);fs_enforce_topmost();CHECK(GetActiveWindow()==work&&GetFocus()==work,"layer maintenance does not steal new active window or input focus");
    ShowWindow(g_hHeatmapWnd,SW_MINIMIZE);fs_enforce_topmost();CHECK(IsIconic(g_hHeatmapWnd)&&!GetPropW(g_hHeatmapWnd,fs_aux_topmost_prop),"minimized statistics is left minimized and promotion removed");ShowWindow(g_hHeatmapWnd,SW_RESTORE);fs_enforce_topmost();
    if(online.count>1){HANDLE context=fs_enter_dpi();RECT r=online.items[0].info.rcMonitor;
        SetWindowPos(input,NULL,r.left+80,r.top+80,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        SetWindowPos(g_hHeatmapWnd,NULL,r.left+150,r.top+150,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);fs_leave_dpi(context);
        fs_remove_window(0);CHECK(!(GetWindowLongW(input,GWL_EXSTYLE)&WS_EX_TOPMOST)&&!(GetWindowLongW(g_hHeatmapWnd,GWL_EXSTYLE)&WS_EX_TOPMOST),"uncovered screen releases temporary UI topmost without closing windows");}
    fs_exit();CHECK(!(GetWindowLongW(input,GWL_EXSTYLE)&WS_EX_TOPMOST)&&!(GetWindowLongW(g_hHeatmapWnd,GWL_EXSTYLE)&WS_EX_TOPMOST),"last fullscreen exit restores original UI styles");
    CHECK(IsWindow(input)&&IsWindow(g_hHeatmapWnd)&&remaining_seconds==seconds&&pomodoro_count==counts&&timer_stage_generation==generation,"display-only layering leaves UI, timer stage, seconds and statistics intact");
    test_repair_z_tracking=0;repair_watch[0]=repair_watch[1]=NULL;DestroyWindow(input);DestroyWindow(g_hHeatmapWnd);DestroyWindow(work);
}
static void repair_editor_preview(void) {
    FullscreenMonitors online;fs_get_monitors(&online);fs_begin();fs_add_screen(&online.items[0]);
    FullscreenColorDraft draft={0};draft.index=0;draft.color=settings.fullscreen_colors[0];draft.scale=100;wcscpy(draft.font,L"Segoe UI");
    HWND editor=CreateDialogParamW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(IDD_FULLSCREEN_COLORS),g_main_hwnd,FullscreenColorsDlgProc,(LPARAM)&draft);
    ShowWindow(editor,SW_SHOW);SetActiveWindow(editor);SetFocus(GetDlgItem(editor,IDC_FS_EDIT_FIRST));fs_enforce_topmost();
    CHECK(repair_above(editor,fs_windows[0])==1&&GetPropW(editor,fs_aux_topmost_prop),"normal editor receives temporary fullscreen protection");
    FullscreenView view;fs_read_timer_view(&view);fs_start_preview(editor,IDC_FS_EDIT_FIRST,&view);
    CHECK(fs_preview_active&&fs_count==online.count&&!IsWindowVisible(editor),"preview still covers all online screens and hides editor");
    fs_end_preview();CHECK(!fs_preview_active&&fs_count==1&&IsWindowVisible(editor)&&GetFocus()==GetDlgItem(editor,IDC_FS_EDIT_FIRST),"preview returns to original selection and editor input focus");
    fs_exit();CHECK(!(GetWindowLongW(editor,GWL_EXSTYLE)&WS_EX_TOPMOST),"editor protection is removed by fullscreen cleanup");
    DestroyWindow(editor);if(draft.preview_font)DeleteObject(draft.preview_font);fs_editor_dialog=NULL;
}
static void repair_prefix_pixels(HDC dc,RECT bounds,int plus) {
    int vertical[64]={0},groups=0,previous=0,best=0,best_col=-1;
    for(int x=bounds.left;x<bounds.right;++x){int n=0;for(int y=bounds.top;y<bounds.bottom;++y)if(GetPixel(dc,x,y)==RGB(255,255,255))++n;
        vertical[x]=n;if(n>best){best=n;best_col=x;}int occupied=n>=3;if(occupied&&!previous)++groups;previous=occupied;}
    if(!plus)CHECK(groups==2,"scaled II has two complete vertical strokes with a visible gap");
    else {int complete=0;for(int y=bounds.top;y<bounds.bottom;++y){int start=-1;
        for(int x=bounds.left;x<=bounds.right;++x){int ink=x<bounds.right&&GetPixel(dc,x,y)==RGB(255,255,255);
            if(ink&&start<0)start=x;if(!ink&&start>=0){if(x-start>=3&&best>=3&&best_col>start&&best_col<x-1)complete=1;start=-1;}}}
        CHECK(complete,"scaled plus retains both arms and a complete vertical stroke");}
}
static void repair_icon_matrix(void) {
    const wchar_t *texts[]={L"25",L"Ⅱ25",L"+25",L"Ⅱ",L"Ⅱ720",L"+720",L"Ⅱ9",L"+9"};
    int sources[]={16,20,24,32,40,48},targets[]={16,20,24,32};
    HDC screen=GetDC(NULL),one=CreateCompatibleDC(screen),sheetdc=CreateCompatibleDC(screen);
    HBITMAP smallmap=CreateCompatibleBitmap(screen,64,64),sheet=CreateCompatibleBitmap(screen,1100,480);
    HGDIOBJ old=SelectObject(one,smallmap),old_sheet=SelectObject(sheetdc,sheet);RECT fill={0,0,1100,480},single={0,0,64,64};
    FillRect(sheetdc,&fill,(HBRUSH)GetStockObject(BLACK_BRUSH));SetBkMode(sheetdc,TRANSPARENT);SetTextColor(sheetdc,RGB(255,255,255));settings.enable_pomodoro_count=1;session_rules.count=1;
    for(int row=0;row<6;++row)for(int col=0;col<8;++col){test_tray_size_override=sources[row];HICON icon=create_tray_icon(texts[col],3);RECT prefix=test_tray_prefix_rect;
        ICONINFO info={0};BITMAP bitmap={0};GetIconInfo(icon,&info);GetObject(info.hbmColor,sizeof(bitmap),&bitmap);int canvas=bitmap.bmWidth;
        CHECK(canvas>=32&&canvas%16==0,"renderer uses an integer grid that can be safely reduced to 16 pixels");DeleteObject(info.hbmColor);DeleteObject(info.hbmMask);
        int x=col*128+64,y=row*78+23;wchar_t label[32];swprintf(label,32,L"%ls %d→%d",texts[col],sources[row],canvas);TextOutW(sheetdc,x,y-20,label,(int)wcslen(label));
        if(col==0){wchar_t rowname[16];swprintf(rowname,16,L"req %d",sources[row]);TextOutW(sheetdc,2,y,rowname,(int)wcslen(rowname));}
        int positions[]={0,23,50,81};
        for(int k=0;k<4;++k){int target=targets[k];FillRect(one,&single,(HBRUSH)GetStockObject(WHITE_BRUSH));DrawIconEx(one,0,0,icon,target,target,0,NULL,DI_NORMAL);
            CHECK(GetPixel(one,target-1,0)==RGB(139,0,0),"mask is opaque after every tested scale");
            if(col==1||col==2||col>=4){RECT bounds={prefix.left*target/canvas,prefix.top*target/canvas,(prefix.right*target+canvas-1)/canvas,(prefix.bottom*target+canvas-1)/canvas};
                repair_prefix_pixels(one,bounds,texts[col][0]==L'+');if(col==1||col==2)CHECK(MulDiv(test_tray_digit_height,target,canvas)>=MulDiv(10,target,16),"scaled two-digit body remains readable");}
            int green=0;for(int py=target*3/4;py<target;++py)for(int px=0;px<target;++px)if(GetPixel(one,px,py)==RGB(144,238,144))++green;
            CHECK(green>=3,"statistics dots remain visible at every tested size");BitBlt(sheetdc,x+positions[k],y,target,target,one,0,0,SRCCOPY);
        }
    }
    test_tray_size_override=16;HICON cached=create_tray_icon(L"Ⅱ25",3);CHECK(create_tray_icon(L"Ⅱ25",3)==cached,"same target and state reuse cache");
    test_tray_size_override=24;create_tray_icon(L"Ⅱ25",3);CHECK(last_tray_size==24,"requested DPI is part of cache despite a shared backing canvas");
    test_tray_size_override=40;HICON larger=create_tray_icon(L"Ⅱ25",3);ICONINFO info={0};BITMAP bitmap={0};GetIconInfo(larger,&info);GetObject(info.hbmColor,sizeof(bitmap),&bitmap);CHECK(bitmap.bmWidth==48,"higher DPI selects the next safe canvas size");DeleteObject(info.hbmColor);DeleteObject(info.hbmMask);
    SelectObject(sheetdc,old_sheet);visual_save_bitmap(sheetdc,sheet,1100,480,L"tray-prefix-matrix.bmp");SelectObject(one,old);
    DeleteObject(sheet);DeleteObject(smallmap);DeleteDC(one);DeleteDC(sheetdc);ReleaseDC(NULL,screen);test_tray_size_override=0;
}
static void repair_fallback(void) {
    consistency_reset();settings.reminder_mode=2;settings.enable_overtime_count_up=1;settings.enable_micro_break=1;settings.micro_break_interval_minutes=15;settings.micro_break_duration_minutes=1;settings.enable_toast_auto_collapse=1;
    fs_show_all();size_t scope=fs_count;test_fail_strong_window=1;
    start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(30);
    CHECK(!sr.active&&g_hToastWnd&&fs_count==scope,"strong failure on normal fullscreen creates fallback popup and preserves selection");
    HWND popup=g_hToastWnd;for(int i=0;i<12;++i){fs_enforce_topmost();pump(10);}for(size_t i=0;i<fs_count;++i)CHECK(repair_above(popup,fs_windows[i])==1,"fallback stays above ordinary fullscreen");
    unsigned generation=timer_stage_generation;timer_apply_live_preferences();CHECK(g_hToastWnd==popup,"unrelated preferences retain existing strong fallback");SendMessageW(popup,WM_TIMER,1,0);CHECK(!g_toast_collapsed,"strong fallback does not auto-collapse");
    SendMessageW(popup,WM_COMMAND,MAKEWPARAM(ID_TOAST_CLOSE,BN_CLICKED),0);CHECK(!g_hToastWnd&&is_overtime&&generation==timer_stage_generation,"fallback close changes display only");
    PostMessageW(g_main_hwnd,WM_TOAST_NOTIFY,TIMER_CUSTOM,generation);pump(30);CHECK(!g_hToastWnd,"dismissed fallback does not reappear for same event");
    test_fail_strong_window=1;consistency_reset();settings.reminder_mode=2;start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(30);
    popup=g_hToastWnd;CHECK(popup&&!fs_active,"strong failure without normal fullscreen creates a popup");fs_show_all();fs_focus_window(fs_windows[0]);
    CHECK(g_hToastWnd==popup,"entering normal fullscreen retains existing strong fallback");
    fs_preview_active=1;fs_enforce_topmost();for(size_t i=0;i<fs_count;++i)CHECK(repair_above(popup,fs_windows[i])==1,"fallback stays visible above preview layers as well");fs_preview_active=0;
    test_fail_strong_window=0;consistency_reset();settings.reminder_mode=2;fs_show_all();scope=fs_count;
    FullscreenMonitors online;fs_get_monitors(&online);if(online.count>1)test_fail_strong_after=2;else test_fail_strong_window=1;
    start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(30);
    CHECK(!sr.active&&!sr.count&&g_hToastWnd&&fs_count==scope,"partial creation failure cleans up all reminder windows before fallback");test_fail_strong_window=test_fail_strong_after=0;
    choose_menu(ID_MENU_REMINDER_OFF);CHECK(!g_hToastWnd&&!sr.active,"off closes fallback immediately without replacing it");
    consistency_reset();settings.reminder_mode=2;test_fail_strong_window=1;fs_show_all();int counts=pomodoro_count;
    start_timer(g_main_hwnd,45,TIMER_SHORT_POMODORO);consistency_clock();timer_advance_seconds(g_main_hwnd,900);pump(30);
    CHECK(g_hToastWnd&&micro.phase==MICRO_WAIT_START&&micro.frozen_seconds==1800&&pomodoro_count==counts,"pre-micro fallback preserves frozen parent and statistics");
    SendMessageW(g_hToastWnd,WM_COMMAND,MAKEWPARAM(ID_TOAST_ACTION,BN_CLICKED),0);pump(30);consistency_clock();CHECK(current_timer_mode==TIMER_MICRO_BREAK&&remaining_seconds==60&&micro.frozen_seconds==1800,"fallback start enters micro break exactly once");
    timer_advance_seconds(g_main_hwnd,60);pump(30);CHECK(g_hToastWnd&&micro.phase==MICRO_WAIT_RESUME,"post-micro fallback is a fresh event");
    SendMessageW(g_hToastWnd,WM_COMMAND,MAKEWPARAM(ID_TOAST_ACTION,BN_CLICKED),0);pump(30);consistency_clock();CHECK(current_timer_mode==TIMER_SHORT_POMODORO&&remaining_seconds==1800&&pomodoro_count==counts,"post-micro fallback restores original focus without changing frozen seconds or statistics");
    test_fail_strong_window=0;consistency_reset();settings.reminder_mode=1;fs_show_all();start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(30);CHECK(!g_hToastWnd,"ordinary popup selection still suppresses popups during normal fullscreen");
    consistency_reset();settings.reminder_mode=1;start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(30);CHECK(g_hToastWnd&&toast_auto_collapse_enabled(),"ordinary popup retains existing ten-second collapse option");SendMessageW(g_hToastWnd,WM_TIMER,1,0);CHECK(g_toast_collapsed,"ordinary popup still collapses on its timer");
}
static void test_gui_repairs(void) {
    TimerSettings saved=settings;int saved_stub=test_notify_stub;test_notify_stub=1;test_silent_sound=1;
    consistency_reset();settings.enable_completion_sound=settings.enable_clock_sound=0;settings.reminder_mode=0;
    repair_layering();consistency_reset();repair_editor_preview();consistency_reset();repair_icon_matrix();repair_fallback();consistency_reset();
    settings=saved;test_notify_stub=saved_stub;test_silent_sound=0;printf("GUI_REPAIR_TEST: %s (%d failures)\n",failures?"FAIL":"PASS",failures);
}
