/* Actual window message dispatch, native online displays, owned isolated desktop. */
static void click_check_teardown(void) {
    CHECK(!fs_active && !fs_count && !fs_windows,"last-screen exit fully clears overlay selection");
    CHECK(!fs_foreground_hook && !fs_show_hook && !fs_hidden_count && !fs_hud_visible,"last-screen exit clears guards hidden windows and mouse HUD");
}
static void test_fullscreen_clicks(void) {
    TimerSettings saved=settings;
    FullscreenMonitors online;
    wchar_t hint[192];
    int i, action, today;
    settings.enable_clock_sound=settings.enable_completion_sound=settings.show_completion_dialog=settings.enable_pomodoro_count=0;
    timer_test_reset();start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);choose_menu(ID_MENU_PAUSE_RESUME);
    int seconds=remaining_seconds;unsigned generation=timer_stage_generation;today=get_today_count_from_storage();
    CHECK(fs_get_monitors(&online) && online.count>0,"native display inventory exists");
    fs_show_all();size_t initial=fs_count;
    HWND clicked=fs_windows[initial-1];wchar_t removed_identity[128];
    wcscpy(removed_identity,((FullscreenMonitor *)GetWindowLongPtrW(clicked,GWLP_USERDATA))->identity);
    HWND kept=initial>1 ? fs_windows[0] : NULL;
    fs_mouse_hint(hint,192);
    CHECK(wcsstr(hint,L"左键：退出此屏") && wcsstr(hint,L"中键 / Esc：退出全部") && wcsstr(hint,L"右键：继续") && wcsstr(hint,L"中键点击托盘：全部全屏"),"ordinary mouse hint matches actual single-screen and all-screen actions");
    SendMessageW(clicked,WM_LBUTTONDOWN,0,0);
    CHECK(!IsWindow(clicked) && fs_window_index(removed_identity)<0,"left click removes only the clicked device");
    if(initial>1) CHECK(fs_active && fs_count==initial-1 && IsWindow(kept),"left click leaves other overlay windows intact");
    else click_check_teardown();
    SendMessageW(g_main_hwnd,WM_USER+1,0,WM_MBUTTONUP);
    CHECK(fs_count==online.count,"tray middle click still adds all online screens");
    SendMessageW(fs_windows[0],WM_MBUTTONDOWN,0,0);click_check_teardown();
    fs_show_all();SendMessageW(fs_windows[0],WM_KEYDOWN,VK_ESCAPE,0);click_check_teardown();
    /* An owned hidden window proves the existing restoration path is exercised. */
    HWND hidden=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"owned hidden fixture",WS_POPUP,0,0,40,40,NULL,NULL,GetModuleHandleW(NULL),NULL);
    fs_begin();fs_add_screen(&online.items[0]);fs_record_hidden_window(hidden);
    SendMessageW(fs_windows[0],WM_LBUTTONDOWN,0,0);click_check_teardown();
    CHECK(IsWindowVisible(hidden),"closing last screen restores recorded hidden window");DestroyWindow(hidden);
    CHECK(is_paused && !is_running && current_timer_mode==TIMER_SHORT_POMODORO && remaining_seconds==seconds && timer_stage_generation==generation && get_today_count_from_storage()==today,"ordinary screen close does not alter timer phase progress or statistics");
    for(i=0;i<2;++i) {
        FullscreenColorDraft color={0};FullscreenSignatureDraft signature={0};
        color.index=0;color.color=settings.fullscreen_colors[0];color.scale=100;wcscpy(color.font,L"Segoe UI");
        signature.color=settings.fullscreen_colors[5];signature.scale=100;wcscpy(signature.font,L"KaiTi");
        HWND editor=CreateDialogParamW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(i ? IDD_FULLSCREEN_SIGNATURE : IDD_FULLSCREEN_COLORS),g_main_hwnd,
            i ? FullscreenSignatureDlgProc : FullscreenColorsDlgProc,(LPARAM)(i ? (void *)&signature : (void *)&color));
        CHECK(editor!=NULL,"native preview editor created");ShowWindow(editor,SW_SHOW);
        for(action=0;action<3;++action) {
            fs_begin();fs_add_screen(&online.items[0]);
            FullscreenView view;fs_read_timer_view(&view);fs_start_preview(editor,i ? IDC_FS_SIGNATURE : IDC_FS_EDIT_FIRST,&view);
            CHECK(fs_preview_active && fs_count==online.count && !IsWindowVisible(editor),"preview initially covers every online screen");
            fs_mouse_hint(hint,192);CHECK(!wcscmp(hint,L"左键 / 中键 / Esc：返回编辑"),"preview hint states left middle and Esc all return to editor");
            if(action==2) SendMessageW(fs_windows[fs_count-1],WM_KEYDOWN,VK_ESCAPE,0);
            else SendMessageW(fs_windows[fs_count-1],action ? WM_MBUTTONDOWN : WM_LBUTTONDOWN,0,0);
            pump(350);
            CHECK(!fs_preview_active && fs_count==1 && fs_window_index(online.items[0].identity)>=0,"each preview exit action ends all preview and restores original device");
            CHECK(IsWindowVisible(editor) && GetFocus()==GetDlgItem(editor,i ? IDC_FS_SIGNATURE : IDC_FS_EDIT_FIRST) && surface_overlay_above(editor)==0,"preview exit restores visible editor input focus above fullscreen");
            CHECK(remaining_seconds==seconds && is_paused && timer_stage_generation==generation,"preview clicks leave timer session unchanged");
            fs_exit();
        }
        FullscreenView view;fs_read_timer_view(&view);fs_start_preview(editor,i ? IDC_FS_SIGNATURE : IDC_FS_EDIT_FIRST,&view);
        SendMessageW(fs_windows[0],WM_LBUTTONDOWN,0,0);
        CHECK(!fs_preview_active && IsWindowVisible(editor),"preview left click without prior fullscreen returns editor immediately");click_check_teardown();
        DestroyWindow(editor);
    }
    timer_test_reset();settings=saved;save_settings();
    printf("FULLSCREEN_CLICK_TEST: %s (%d failures), %zu native displays\n",failures ? "FAIL" : "PASS",failures,online.count);
}
