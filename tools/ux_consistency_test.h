static int ux_preview_seen;
static BOOL CALLBACK ux_preview_dialog(HWND dialog, LPARAM unused) {
    (void)unused;
    if (!GetDlgItem(dialog, IDC_FS_EDIT_FIRST)) return TRUE;
    wchar_t local_time[32];
    GetDlgItemTextW(dialog, IDC_FS_PREVIEW_FIRST, local_time, 32);
    CHECK(!wcscmp(local_time,L"32:00"), "native local preview uses the frozen current time");
    SendMessageW(dialog,WM_COMMAND,IDC_FS_COLOR_FULL,0);
    CHECK(fs_preview_active && !wcscmp(fs_view.status,L"短番茄钟 · 未开始") && !wcscmp(fs_view.time,local_time), "native fullscreen preview and local preview share one snapshot");
    if(fs_count) SendMessageW(fs_windows[0],WM_KEYDOWN,VK_ESCAPE,0);
    CHECK(!fs_preview_active && micro.frozen_seconds==1920, "preview exit does not alter retained focus");
    ux_preview_seen=1;
    SendMessageW(dialog,WM_COMMAND,IDCANCEL,0);
    return FALSE;
}
static VOID CALLBACK ux_preview_timer(HWND window,UINT message,UINT_PTR id,DWORD tick) {
    (void)window;(void)message;(void)tick;
    KillTimer(NULL,id);EnumThreadWindows(GetCurrentThreadId(),ux_preview_dialog,0);
}
/* New duplicate-choice protection and preview consistency only. */
static void test_ux_consistency(void) {
    TimerSettings saved = settings; DataLocationMode saved_location = g_data_location_mode;
    FullscreenView view; wchar_t expected[96];
    settings.enable_clock_sound = settings.enable_completion_sound = settings.reminder_mode = 0;
    timer_test_reset();
    micro.source = TIMER_SHORT_POMODORO; micro.phase = MICRO_WAIT_RESUME; micro.frozen_seconds = 1920;
    current_timer_mode = TIMER_MICRO_BREAK; settings.short_pomodoro_duration = 45;
    CHECK(timer_ui_selection_locked(ID_MENU_IDLE_SHORT_POMODORO), "current frozen ready mode is disabled");
    unsigned generation = timer_stage_generation;
    choose_menu(ID_MENU_IDLE_SHORT_POMODORO);
    CHECK(timer_stage_generation == generation && micro.frozen_seconds == 1920 && micro.source == TIMER_SHORT_POMODORO, "duplicate command cannot discard frozen focus");
    TimerUiView preview = fs_preview_timer(0);
    CHECK(preview.mode == TIMER_SHORT_POMODORO && preview.seconds == 1920 && preview.state == TIMER_UI_READY, "focus preview retains actual frozen mode time and state");
    fs_apply_mode_preview(&view, preview);
    CHECK(!wcscmp(view.status,L"短番茄钟 · 未开始") && !wcscmp(view.time,L"32:00"), "preview matches actual ready rendering");
    SetTimer(NULL,0,100,ux_preview_timer); fs_show_color(g_main_hwnd,0);
    CHECK(ux_preview_seen, "native color dialog and preview flow were exercised");
    preview = fs_preview_timer(4); fs_apply_mode_preview(&view,preview);
    CHECK(!wcscmp(view.status,L"超时正计时") && view.time[0] == L'+', "overtime preview never invents an idle overtime state");
    settings.default_pomodoro_is_long = 1; settings.default_break_is_long = 0;
    CHECK(timer_ui_selection_locked(ID_MENU_DEFAULT_LONG_POMODORO) && !timer_ui_selection_locked(ID_MENU_DEFAULT_SHORT_POMODORO), "default focus choices disable only selected value");
    CHECK(timer_ui_selection_locked(ID_MENU_DEFAULT_SHORT_BREAK) && !timer_ui_selection_locked(ID_MENU_DEFAULT_LONG_BREAK), "default break choices disable only selected value");
    choose_menu(ID_MENU_DEFAULT_LONG_POMODORO);
    CHECK(settings.default_pomodoro_is_long && micro.frozen_seconds == 1920, "duplicate default choice is harmless");
    g_data_location_mode = DATA_LOC_ONEDRIVE;
    CHECK(timer_ui_selection_locked(ID_MENU_DATA_LOC_ONEDRIVE) && !timer_ui_selection_locked(ID_MENU_DATA_LOC_CUSTOM), "current fixed location is disabled but custom target can change");
    timer_test_reset();
    TimerMode modes[] = {TIMER_LONG_POMODORO,TIMER_SHORT_BREAK,TIMER_COUNT_UP,TIMER_CUSTOM,TIMER_MICRO_BREAK};
    int indices[] = {0,1,2,3,6}; int i;
    for(i=0;i<5;++i) {
        current_timer_mode = modes[i]; remaining_seconds=123;is_running=1;is_paused=0;
        preview=fs_preview_timer(indices[i]);fs_apply_mode_preview(&view,preview);
        CHECK(!wcscmp(view.status,timer_mode_name(modes[i])) && !wcscmp(view.time,L"02:03"), "running preview uses full mode name without suffix");
        is_running=0;is_paused=1;preview=fs_preview_timer(indices[i]);fs_apply_mode_preview(&view,preview);
        swprintf(expected,96,L"%ls · 已暂停",timer_mode_name(modes[i]));
        CHECK(!wcscmp(view.status,expected), "paused preview uses common formatter");
        is_paused=0;
    }
    is_running=1;is_overtime=1;overtime_source_mode=TIMER_CUSTOM;overtime_seconds=192;
    preview=fs_preview_timer(4);fs_apply_mode_preview(&view,preview);
    CHECK(!wcscmp(view.status,L"超时正计时") && !wcscmp(view.time,L"+03:12"), "active overtime preview matches actual overtime");
    timer_test_reset();
    g_data_location_mode=saved_location;settings=saved;
    printf("UX_CONSISTENCY_TEST: %s (%d failures)\n",failures ? "FAIL" : "PASS",failures);
}
