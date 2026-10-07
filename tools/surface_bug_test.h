/* Three confirmed surface bugs: native z-order, ready tooltip, and sound call state. */
static int surface_overlay_above(HWND dialog) {
    HWND window = GetTopWindow(NULL);
    while (window) {
        size_t i;
        if (window == dialog) return 0;
        for (i=0;i<fs_count;++i) if (window == fs_windows[i]) return 1;
        window = GetWindow(window,GW_HWNDNEXT);
    }
    return -1;
}
static void surface_check_overtime_sound(void) {
    CHECK(is_overtime && !g_clock_loop_playing,"entering overtime stops loop sound");
    choose_menu(ID_MENU_PAUSE_RESUME); choose_menu(ID_MENU_PAUSE_RESUME);
    CHECK(is_overtime && is_running && !g_clock_loop_playing,"overtime pause/resume stays silent");
    choose_menu(4); choose_menu(4);
    CHECK(settings.enable_clock_sound && !g_clock_loop_playing,"enabling sound during overtime stays silent");
    timer_apply_live_preferences();
    CHECK(!g_clock_loop_playing,"live preferences preserve overtime sound policy");
}
static void test_surface_bugs(void) {
    TimerSettings saved = settings;
    int i;
    int idle_commands[]={ID_MENU_IDLE_POMODORO,ID_MENU_IDLE_SHORT_POMODORO,ID_MENU_IDLE_SHORT_BREAK,ID_MENU_IDLE_LONG_BREAK,ID_MENU_IDLE_CUSTOM};
    int duration_commands[]={ID_MENU_SET_POMODORO_DURATION,ID_MENU_SET_SHORT_POMODORO_DURATION,ID_MENU_SET_SHORT_BREAK_DURATION,ID_MENU_SET_LONG_BREAK_DURATION,ID_MENU_SET_CUSTOM_DURATION};
    test_silent_sound=1;
    settings.enable_completion_sound=settings.show_completion_dialog=settings.enable_pomodoro_count=settings.enable_clock_sound=0;
    settings.enable_micro_break=1; settings.enable_overtime_count_up=1;
    settings.micro_break_interval_minutes=15;settings.micro_break_duration_minutes=1;
    timer_test_reset();
    for(i=0;i<5;++i) {
        choose_menu(idle_commands[i]);
        timer_input_case=1;SetTimer(NULL,0,50,timer_test_input_tick);choose_menu(duration_commands[i]);
        CHECK(timer_ui_resolve().seconds==29*60 && wcsstr(nid.szTip,L"29:00"),"ready duration edit immediately updates native tooltip for each countdown type");
        CHECK(!is_running && !is_paused,"default duration edit does not start timer");
    }
    settings.enable_clock_sound=1;
    start_timer(g_main_hwnd,1,TIMER_CUSTOM);
    CHECK(g_clock_loop_playing,"ordinary countdown retains enabled loop sound");
    timer_advance_seconds(g_main_hwnd,60);surface_check_overtime_sound();
    timer_test_reset();start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);
    timer_advance_seconds(g_main_hwnd,900);surface_check_overtime_sound();
    timer_primary_action(g_main_hwnd);
    CHECK(current_timer_mode==TIMER_MICRO_BREAK && g_clock_loop_playing,"micro countdown uses ordinary sound policy");
    timer_advance_seconds(g_main_hwnd,60);surface_check_overtime_sound();
    timer_primary_action(g_main_hwnd);
    CHECK(current_timer_mode==TIMER_SHORT_POMODORO && g_clock_loop_playing,"resumed focus restores countdown sound");
    choose_menu(ID_MENU_PAUSE_RESUME);
    timer_input_case=1;SetTimer(NULL,0,50,timer_test_input_tick);choose_menu(ID_MENU_SET_SHORT_POMODORO_DURATION);
    CHECK(is_paused && remaining_seconds==25*60 && wcsstr(nid.szTip,L"25:00"),"default duration refresh leaves frozen active progress unchanged");
    timer_test_reset();settings.enable_clock_sound=0;
    FullscreenMonitors online;CHECK(fs_get_monitors(&online) && online.count>0,"native screens available");
    for(i=0;i<2 && online.count;++i) {
        fs_begin();fs_add_screen(&online.items[0]);
        FullscreenColorDraft color={0};FullscreenSignatureDraft signature={0};HWND editor;
        color.index=0;color.color=settings.fullscreen_colors[0];color.scale=100;wcscpy(color.font,L"Segoe UI");
        signature.color=settings.fullscreen_colors[5];signature.scale=100;wcscpy(signature.font,L"KaiTi");
        editor=CreateDialogParamW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(i ? IDD_FULLSCREEN_SIGNATURE : IDD_FULLSCREEN_COLORS),g_main_hwnd,
            i ? FullscreenSignatureDlgProc : FullscreenColorsDlgProc,(LPARAM)(i ? (void *)&signature : (void *)&color));
        CHECK(editor!=NULL,"native appearance editor created");ShowWindow(editor,SW_SHOW);pump(600);
        CHECK(IsWindowVisible(editor) && surface_overlay_above(editor)==0,"periodic full screen guard keeps appearance editor accessible");
        FullscreenView view;fs_read_timer_view(&view);
        fs_start_preview(editor,i ? IDC_FS_SIGNATURE : IDC_FS_EDIT_FIRST,&view);pump(350);
        CHECK(!IsWindowVisible(editor) && fs_preview_active && fs_count==online.count,"preview hides editor and covers all online screens");
        fs_end_preview();pump(600);
        CHECK(IsWindowVisible(editor) && surface_overlay_above(editor)==0 && fs_count==1,"preview return keeps editor above restored original screen");
        CHECK(GetFocus()==GetDlgItem(editor,i ? IDC_FS_SIGNATURE : IDC_FS_EDIT_FIRST),"preview restores editor input focus");
        DestroyWindow(editor);
        CHECK(!fs_editor_dialog,"destroyed editor leaves no stale guard reference");
        fs_exit();
    }
    timer_test_reset();settings=saved;save_settings();test_silent_sound=0;
    printf("SURFACE_BUG_TEST: %s (%d failures)\n",failures ? "FAIL" : "PASS",failures);
}
