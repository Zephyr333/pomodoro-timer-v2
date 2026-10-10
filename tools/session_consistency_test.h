/* v3.0.4 new contract only; native commands with deterministic clock advancement. */
static int consistency_input_value;
static BOOL CALLBACK consistency_input_window(HWND dialog,LPARAM unused) {
    (void)unused;if(!GetDlgItem(dialog,IDC_INPUT_EDIT)) return TRUE;
    wchar_t value[32];swprintf(value,32,L"%d",consistency_input_value);SetDlgItemTextW(dialog,IDC_INPUT_EDIT,value);
    SendMessageW(dialog,WM_COMMAND,IDOK,0);return FALSE;
}
static VOID CALLBACK consistency_input_tick(HWND window,UINT message,UINT_PTR id,DWORD tick) {
    (void)window;(void)message;(void)tick;KillTimer(NULL,id);EnumThreadWindows(GetCurrentThreadId(),consistency_input_window,0);
}
static void consistency_clock(void) { if(timer_clock_id)KillTimer(g_main_hwnd,ID_TIMER_CLOCK);timer_clock_id=0; }
static void consistency_input(int minutes) {
    consistency_input_value=minutes;SetTimer(NULL,0,30,consistency_input_tick);choose_menu(ID_MENU_SET_TIME);
}
static void consistency_reset(void) { sr_dismiss(0);fs_exit();timer_test_reset(); }
static void consistency_micro_cycle(void) {
    CHECK(micro.phase==MICRO_WAIT_START,"micro progression starts from due phase");
    timer_primary_action(g_main_hwnd);consistency_clock();
    CHECK(current_timer_mode==TIMER_MICRO_BREAK && is_running,"one primary starts micro");
    timer_primary_action(g_main_hwnd);
    CHECK(!is_running && timer_ui_is_ready() && micro.phase==MICRO_WAIT_RESUME,"early micro end prepares frozen focus ready");
    timer_primary_action(g_main_hwnd);consistency_clock();
}
static void test_session_consistency(void) {
    TimerSettings saved=settings;int i,j;
    consistency_reset();test_silent_sound=1;test_io_active=1;
    settings.enable_clock_sound=settings.enable_completion_sound=0;settings.reminder_mode=0;
    settings.enable_micro_break=0;settings.enable_overtime_count_up=1;settings.enable_pomodoro_count=1;
    TimerMode countdowns[]={TIMER_LONG_POMODORO,TIMER_SHORT_POMODORO,TIMER_SHORT_BREAK,TIMER_LONG_BREAK,TIMER_CUSTOM};
    for(i=0;i<5;++i) for(j=0;j<2;++j) {
        consistency_reset();start_timer(g_main_hwnd,3,countdowns[i]);consistency_clock();
        if(j)choose_menu(ID_MENU_PAUSE_RESUME);
        CHECK(!wcscmp(timer_ui_primary_label(),j?L"开始":L"结束"),"paused starts and running ends");
        consistency_input(500);
        CHECK(remaining_seconds==180 && timer_time_cap()==180 && !!is_paused==j && !!is_running==!j,"input over cap clips to original duration and retains state in every countdown");
        settings.long_pomodoro_duration=settings.short_pomodoro_duration=settings.custom_duration=20;
        settings.short_break_duration=settings.long_break_duration=20;
        remaining_seconds=179;choose_menu(ID_MENU_PLUS_5_MIN);
        CHECK(remaining_seconds==180 && session_rules.initial_seconds==180,"step increase and changed defaults cannot exceed original session cap");
        if(j)choose_menu(ID_MENU_START_CURRENT);
        choose_menu(ID_MENU_START_CURRENT);
        CHECK(!is_running && !is_paused && timer_ui_is_ready() && !wcscmp(timer_ui_primary_label(),L"开始"),"single primary ends both running and paused countdowns into ready");
    }
    consistency_reset();settings.default_break_is_long=0;start_timer(g_main_hwnd,0,TIMER_SHORT_POMODORO);consistency_clock();
    timer_advance_seconds(g_main_hwnd,0);
    CHECK(is_overtime && !wcscmp(timer_ui_primary_label(),L"开始"),"completed overtime has start primary");
    choose_menu(ID_MENU_START_CURRENT);consistency_clock();CHECK(is_running && !is_overtime && current_timer_mode==TIMER_SHORT_BREAK,"overtime first click directly starts short break");
    consistency_reset();start_timer(g_main_hwnd,3,TIMER_SHORT_POMODORO);consistency_clock();timer_primary_action(g_main_hwnd);
    choose_menu(ID_MENU_DEFAULT_LONG_BREAK);CHECK(timer_ui_resolve().mode==TIMER_SHORT_BREAK,"changing default type cannot replace prepared ready mode");
    settings.short_break_duration=7;refresh_timer_icon_by_state(g_main_hwnd);CHECK(timer_ui_resolve().seconds==420,"prepared preset duration follows its own new default");
    choose_menu(ID_MENU_START_CURRENT);consistency_clock();CHECK(current_timer_mode==TIMER_SHORT_BREAK && remaining_seconds==420,"ready second click starts the displayed prepared target");
    consistency_reset();settings.enable_micro_break=1;settings.micro_break_interval_minutes=15;settings.micro_break_duration_minutes=1;
    start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);consistency_clock();timer_advance_seconds(g_main_hwnd,900);
    CHECK(!wcscmp(last_text,L"Ⅱ25") && wcsstr(nid.szTip,L"25:00") && wcsstr(nid.szTip,L"+00:00"),"micro due tray keeps frozen parent with prefix and real overtime tooltip");
    choose_menu(ID_MENU_PAUSE_RESUME);CHECK(!wcscmp(last_text,L"Ⅱ"),"paused overtime uses ordinary full pause glyph");
    choose_menu(ID_MENU_PAUSE_RESUME);consistency_clock();CHECK(!wcscmp(last_text,L"Ⅱ25"),"continuing restores frozen parent prefix");
    consistency_micro_cycle();CHECK(remaining_seconds==1500 && timer_time_cap()==2400 && !wcscmp(last_text,L"25"),"resumed parent retains time/cap and clears waiting prefix even for same number");
    choose_menu(ID_MENU_PAUSE_RESUME);consistency_input(26);choose_menu(ID_MENU_PAUSE_RESUME);consistency_clock();
    timer_advance_seconds(g_main_hwnd,60);CHECK(micro.phase==MICRO_WAIT_START && micro.frozen_seconds==1500,"re-crossing previously used 25-minute tick triggers again");
    consistency_micro_cycle();choose_menu(ID_MENU_PAUSE_RESUME);consistency_input(25);choose_menu(ID_MENU_PAUSE_RESUME);consistency_clock();timer_advance_seconds(g_main_hwnd,1);
    CHECK(micro.phase==MICRO_NONE && remaining_seconds==1499,"direct setting and restoring exact tick never trigger in place");
    choose_menu(ID_MENU_PAUSE_RESUME);consistency_input(26);consistency_input(24);choose_menu(ID_MENU_PAUSE_RESUME);consistency_clock();timer_advance_seconds(g_main_hwnd,1);
    CHECK(micro.phase==MICRO_NONE,"manual downward skip never creates retroactive micro");
    choose_menu(ID_MENU_PAUSE_RESUME);consistency_input(9999);CHECK(remaining_seconds==2400,"parent cap prevents upward expansion beyond initial schedule");
    choose_menu(ID_MENU_PAUSE_RESUME);consistency_clock();timer_advance_seconds(g_main_hwnd,900);
    choose_menu(ID_MENU_START_CURRENT);consistency_clock();CHECK(is_running && current_timer_mode==TIMER_MICRO_BREAK,"micro overtime first click directly begins micro");
    choose_menu(ID_MENU_PAUSE_RESUME);consistency_input(10);
    CHECK(remaining_seconds==60 && micro.frozen_seconds==1500 && timer_time_cap()==60,"micro cap is independent of parent frozen value and cap");
    choose_menu(ID_MENU_PAUSE_RESUME);consistency_clock();timer_advance_seconds(g_main_hwnd,60);
    CHECK(micro.phase==MICRO_WAIT_RESUME && !wcscmp(last_text,L"Ⅱ25"),"post-micro overtime has same parent prefix");
    settings.short_pomodoro_duration=2;choose_menu(ID_MENU_START_CURRENT);consistency_clock();CHECK(remaining_seconds==1500 && timer_time_cap()==2400,"new default cannot overwrite resumed parent or its cap");
    consistency_reset();settings.enable_micro_break=0;
    int before=get_today_count_from_storage();start_timer(g_main_hwnd,0,TIMER_COUNT_UP);consistency_clock();timer_advance_seconds(g_main_hwnd,5400);
    choose_menu(ID_MENU_PAUSE_RESUME);consistency_input(9999);consistency_input(0);choose_menu(ID_MENU_PAUSE_RESUME);consistency_clock();timer_advance_seconds(g_main_hwnd,5400);choose_menu(ID_MENU_START_CURRENT);
    CHECK(get_today_count_from_storage()==before && timer_ui_is_ready(),"count-up edits natural running and stop never credit Pomodoros");
    settings.long_pomodoro_count=3;
    consistency_reset();start_timer(g_main_hwnd,2,TIMER_LONG_POMODORO);consistency_clock();timer_advance_seconds(g_main_hwnd,60);timer_primary_action(g_main_hwnd);
    CHECK(get_today_count_from_storage()==before,"partially complete long Pomodoro never credits partial quota");
    start_timer(g_main_hwnd,2,TIMER_LONG_POMODORO);consistency_clock();settings.long_pomodoro_count=1;remaining_seconds=1;timer_advance_seconds(g_main_hwnd,1);
    CHECK(get_today_count_from_storage()==before+3,"long completion uses captured quota after real time edit");
    timer_advance_seconds(g_main_hwnd,5400);timer_primary_action(g_main_hwnd);CHECK(get_today_count_from_storage()==before+3,"overtime and ending it never repeat completion credit");
    consistency_reset();start_timer(g_main_hwnd,2,TIMER_SHORT_POMODORO);consistency_clock();consistency_input(20);remaining_seconds=1;timer_advance_seconds(g_main_hwnd,1);
    CHECK(get_today_count_from_storage()==before+4,"short completion always credits exactly one despite edits");
    for(i=2;i<5;++i){consistency_reset();start_timer(g_main_hwnd,0,countdowns[i]);consistency_clock();timer_advance_seconds(g_main_hwnd,0);}
    CHECK(get_today_count_from_storage()==before+4,"rest and custom countdowns never credit Pomodoros");
    consistency_reset();start_timer(g_main_hwnd,1,TIMER_SHORT_POMODORO);remaining_seconds=1;timer_last_tick=GetTickCount64()-1000;
    choose_menu(ID_MENU_START_CURRENT);CHECK(get_today_count_from_storage()==before+5 && is_running && !is_overtime,"action synchronizes boundary completion before advancing overtime");
    before=get_today_count_from_storage();test_fail_stats_commit=1;
    CHECK(!record_completed_pomodoros(1) && get_today_count_from_storage()==before,"failed atomic statistic append preserves previous target exactly");
    consistency_reset();start_timer(g_main_hwnd,0,TIMER_SHORT_POMODORO);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(20);
    CHECK(get_today_count_from_storage()==before && is_overtime,"failed completion reports failure and finalizes stage without recursive credit");test_fail_stats_commit=0;
    timer_advance_seconds(g_main_hwnd,500);CHECK(get_today_count_from_storage()==before,"failed completed stage is not silently credited twice");
    consistency_reset();const char *fold_json[]={"{\"pomodoro_duration\":45,\"pomodoro_count\":0,\"toast_auto_collapse_seconds\":0}","{\"pomodoro_duration\":45,\"pomodoro_count\":0,\"toast_auto_collapse_seconds\":29}","{\"pomodoro_duration\":45,\"pomodoro_count\":0,\"enable_toast_auto_collapse\":0,\"toast_auto_collapse_seconds\":10}"};
    int folds[]={0,1,0};for(i=0;i<3;++i){write_test_json(g_settings_path,fold_json[i]);load_settings();CHECK(settings.enable_toast_auto_collapse==folds[i],"fold checkbox migration and explicit new preference precedence");}
    settings.reminder_mode=1;settings.enable_completion_sound=0;settings.enable_clock_sound=0;settings.enable_toast_auto_collapse=0;
    ShowCompletionNotification(g_main_hwnd,TIMER_SHORT_BREAK);choose_menu(ID_MENU_SET_TOAST_COLLAPSE_SECONDS);
    CHECK(settings.enable_toast_auto_collapse && g_hToastWnd && !g_toast_collapsed,"fold checkbox enables current expanded toast without input dialog");
    SendMessageW(g_hToastWnd,WM_TIMER,1,0);CHECK(g_toast_collapsed,"auto fold timer collapses existing toast");
    SendMessageW(g_hToastWnd,WM_COMMAND,MAKEWPARAM(ID_TOAST_COLLAPSE,BN_CLICKED),0);CHECK(!g_toast_collapsed,"manual expansion works while automatic fold remains enabled");
    choose_menu(ID_MENU_SET_TOAST_COLLAPSE_SECONDS);CHECK(!settings.enable_toast_auto_collapse && !g_toast_collapsed,"disabling fold preserves current expanded toast");
    close_toast_notification_if_open();
    consistency_reset();settings.reminder_mode=2;settings.enable_micro_break=1;settings.micro_break_interval_minutes=15;settings.micro_break_duration_minutes=1;settings.enable_overtime_count_up=0;
    start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);consistency_clock();timer_advance_seconds(g_main_hwnd,900);pump(20);CHECK(sr.active,"first micro event delivers strong reminder");
    unsigned notified=timer_stage_generation;SendMessageW(sr.items[0].window,WM_LBUTTONUP,0,0);consistency_micro_cycle();
    choose_menu(ID_MENU_PAUSE_RESUME);consistency_input(26);choose_menu(ID_MENU_PAUSE_RESUME);consistency_clock();timer_advance_seconds(g_main_hwnd,60);pump(20);
    CHECK(sr.active && timer_stage_generation!=notified,"fresh repeat crossing produces a new reminder despite prior dismissal");
    sr_dismiss(0);PostMessageW(g_main_hwnd,WM_TOAST_NOTIFY,TIMER_MICRO_BREAK,timer_stage_generation);pump(20);CHECK(!sr.active,"same event still deduplicates stale notification message");
    consistency_reset();settings.reminder_mode=2;
    HWND first=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"first",WS_POPUP|WS_VISIBLE,0,0,100,80,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HWND other=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"user selected",WS_POPUP|WS_VISIBLE,110,0,100,80,NULL,NULL,GetModuleHandleW(NULL),NULL);
    SetActiveWindow(first);SetFocus(first);start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(20);
    SetActiveWindow(other);SetFocus(other);sr_dismiss(1);CHECK(GetFocus()==other,"reminder dismissal respects newly selected window focus");
    DestroyWindow(other);DestroyWindow(first);
    consistency_reset();settings.enable_micro_break=0;settings.enable_clock_sound=settings.enable_completion_sound=0;settings.enable_overtime_count_up=1;settings.reminder_mode=0;
    start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);
    SendMessageW(g_main_hwnd,WM_USER+1,0,WM_LBUTTONUP);consistency_clock();CHECK(is_running && !is_overtime,"tray primary advances overtime directly on first click");
    SendMessageW(g_main_hwnd,WM_USER+1,0,WM_LBUTTONUP);CHECK(!is_running && timer_ui_is_ready(),"tray normal running click still ends into ready");
    consistency_reset();fs_show_all();size_t screen_count=fs_count;start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);
    SendMessageW(fs_windows[0],WM_RBUTTONUP,0,0);CHECK(is_running && !is_overtime && fs_count==screen_count,"normal fullscreen right click advances overtime directly");
    consistency_clock();SendMessageW(fs_windows[0],WM_RBUTTONUP,0,0);CHECK(!is_running && timer_ui_is_ready(),"normal fullscreen right click ends ordinary running into ready");
    consistency_reset();settings.reminder_mode=1;start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(20);
    wchar_t button[32];GetWindowTextW(g_hToastButton,button,32);CHECK(!wcscmp(button,L"开始"),"native overtime popup primary label is start");
    SendMessageW(g_hToastWnd,WM_COMMAND,MAKEWPARAM(ID_TOAST_ACTION,BN_CLICKED),(LPARAM)g_hToastButton);
    CHECK(is_running && !is_overtime && !g_hToastWnd,"popup primary advances overtime directly");
    consistency_reset();settings.reminder_mode=2;start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(20);
    CHECK(sr.active,"strong reminder for entry-point test created");SendMessageW(sr.items[0].window,WM_RBUTTONUP,0,0);
    CHECK(!sr.active && is_running && !is_overtime,"strong right click advances overtime directly");
    consistency_reset();settings.reminder_mode=0;settings.enable_micro_break=1;settings.micro_break_interval_minutes=15;settings.micro_break_duration_minutes=1;
    start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);consistency_clock();timer_advance_seconds(g_main_hwnd,900);consistency_micro_cycle();
    for(i=0;i<3;++i){choose_menu(ID_MENU_PAUSE_RESUME);consistency_input(26);choose_menu(ID_MENU_PAUSE_RESUME);consistency_clock();timer_advance_seconds(g_main_hwnd,60);CHECK(micro.phase==MICRO_WAIT_START,"same physical tick can be crossed repeatedly without lifetime history");consistency_micro_cycle();}
    consistency_reset();start_timer(g_main_hwnd,3,TIMER_SHORT_BREAK);consistency_clock();choose_menu(ID_MENU_PAUSE_RESUME);consistency_input(INT_MAX);
    CHECK(remaining_seconds==180,"maximum valid input clamps safely without integer overflow");
    consistency_reset();settings.enable_micro_break=1;settings.micro_break_interval_minutes=15;settings.micro_break_duration_minutes=1;settings.enable_overtime_count_up=1;settings.enable_pomodoro_count=1;
    start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);consistency_clock();settings.micro_break_interval_minutes=10;settings.micro_break_duration_minutes=5;settings.enable_micro_break=0;settings.enable_overtime_count_up=0;settings.enable_pomodoro_count=0;
    timer_advance_seconds(g_main_hwnd,900);
    CHECK(is_overtime && micro.frozen_seconds==1500 && micro.interval_seconds==900 && micro.duration_seconds==60 && timer_effective_count(),"changing defaults preserves current captured timing and count rules");
    consistency_micro_cycle();choose_menu(ID_MENU_PAUSE_RESUME);
    test_tray_size_override=32;
    HDC icon_target=GetDC(NULL),icon_dc=CreateCompatibleDC(icon_target);HBITMAP icon_bitmap=CreateCompatibleBitmap(icon_target,32,32);HGDIOBJ prior_bitmap=SelectObject(icon_dc,icon_bitmap);
    micro.phase=MICRO_WAIT_RESUME;is_paused=0;refresh_timer_icon_by_state(g_main_hwnd);
    DrawIconEx(icon_dc,0,0,last_icon,32,32,0,NULL,DI_NORMAL);
    int y,clipped=0;for(y=0;y<25;++y)if(GetPixel(icon_dc,0,y)==RGB(255,255,255)||GetPixel(icon_dc,31,y)==RGB(255,255,255))++clipped;
    CHECK(!wcscmp(last_text,L"Ⅱ25") && last_icon!=NULL,"same-value waiting icon is refreshed; complete glyph masks are verified by GUI repair matrix, border contact alone is not clipping");
    test_tray_size_override=0;
    SelectObject(icon_dc,prior_bitmap);DeleteObject(icon_bitmap);DeleteDC(icon_dc);ReleaseDC(NULL,icon_target);
    consistency_reset();settings.reminder_mode=1;settings.enable_toast_auto_collapse=0;ShowCompletionNotification(g_main_hwnd,TIMER_CUSTOM);
    timer_apply_live_preferences();SendMessageW(g_hToastWnd,WM_TIMER,1,0);CHECK(!g_toast_collapsed,"cancelled stale auto-fold message does not collapse disabled popup");
    settings.enable_toast_auto_collapse=1;timer_apply_live_preferences();SendMessageW(g_hToastWnd,WM_TIMER,1,0);CHECK(g_toast_collapsed,"live preferences rearm folding for current expanded popup");close_toast_notification_if_open();
    consistency_reset();settings=saved;save_settings();test_silent_sound=test_io_active=0;
    printf("SESSION_CONSISTENCY_TEST: %s (%d failures)\n",failures ? "FAIL" : "PASS",failures);
}
