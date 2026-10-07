/* v3.0.1: independent pre-micro waiting overtime, no parent extension. */
static void wait_check_view(int seconds, const wchar_t *time) {
    FullscreenView fullscreen;
    fs_read_timer_view(&fullscreen);
    TimerUiView ui=timer_ui_resolve();
    wchar_t notification[96];timer_ui_notification(notification,96,TIMER_MICRO_BREAK);
    CHECK(ui.overtime && ui.seconds==seconds && !wcscmp(fullscreen.time,time),"fullscreen displays only waiting overtime seconds");
    CHECK(wcsstr(nid.szTip,time) && wcsstr(nid.szTip,L"超时正计时"),"tray tooltip matches overtime mode and duration");
    CHECK(!wcscmp(notification,fullscreen.status),"notification and fullscreen resolve the same overtime status");
    if(g_hToastWnd) {
        const wchar_t *message=(const wchar_t *)GetWindowLongPtrW(g_hToastWnd,GWLP_USERDATA);
        CHECK(message && wcsstr(message,notification),"native completion popup matches current overtime status");
    }
}
static void test_micro_wait_overtime(void) {
    TimerSettings saved=settings;
    settings.enable_micro_break=1;settings.micro_break_interval_minutes=15;settings.micro_break_duration_minutes=1;
    settings.enable_overtime_count_up=1;settings.enable_pomodoro_count=0;
    settings.enable_clock_sound=settings.enable_completion_sound=settings.reminder_mode=0;
    timer_test_reset();start_timer(g_main_hwnd,45,TIMER_SHORT_POMODORO);
    timer_advance_seconds(g_main_hwnd,900);
    CHECK(is_overtime && micro.phase==MICRO_WAIT_START && remaining_seconds==1800 && micro.frozen_seconds==1800,"micro due freezes focus before user starts break");
    ShowCompletionNotification(g_main_hwnd,TIMER_MICRO_BREAK);wait_check_view(0,L"+00:00");
    timer_advance_seconds(g_main_hwnd,120);wait_check_view(120,L"+02:00");
    CHECK(remaining_seconds==1800 && micro.frozen_seconds==1800 && micro.base_seconds==2700,"waiting leaves frozen parent and schedule unchanged");
    choose_menu(ID_MENU_PAUSE_RESUME);wait_check_view(120,L"+02:00");
    timer_advance_seconds(g_main_hwnd,200);
    CHECK(is_paused && overtime_seconds==120 && micro.frozen_seconds==1800,"paused waiting overtime does not advance either clock");
    choose_menu(ID_MENU_PAUSE_RESUME);timer_advance_seconds(g_main_hwnd,1);wait_check_view(121,L"+02:01");
    /* Real tray callback advances directly to the micro break from waiting overtime. */
    SendMessageW(g_main_hwnd,WM_USER+1,0,WM_LBUTTONUP);
    CHECK(current_timer_mode==TIMER_MICRO_BREAK && remaining_seconds==60 && micro.frozen_seconds==1800 && !is_overtime,"tray click starts independent break without changing frozen focus");
    timer_advance_seconds(g_main_hwnd,62);
    CHECK(micro.phase==MICRO_WAIT_RESUME && overtime_seconds==2 && micro.frozen_seconds==1800,"micro completion overtime stays independent");
    SendMessageW(g_main_hwnd,WM_USER+1,0,WM_LBUTTONUP);
    CHECK(current_timer_mode==TIMER_SHORT_POMODORO && remaining_seconds==1800,"focus resumes original 30 minutes after both overtime stages");
    timer_advance_seconds(g_main_hwnd,900);
    CHECK(micro.phase==MICRO_WAIT_START && overtime_seconds==0 && micro.frozen_seconds==900 && (micro.frozen_seconds == micro.base_seconds - 2 * micro.interval_seconds),"next micro remains at original 15-minute tick");
    timer_advance_seconds(g_main_hwnd,2100);timer_stop_action(g_main_hwnd);
    CHECK(!is_overtime && !is_running && micro.phase==MICRO_WAIT_START && remaining_seconds==900 && micro.frozen_seconds==900,"ending waiting overtime retains original frozen focus for later start");
    timer_primary_action(g_main_hwnd);timer_stop_action(g_main_hwnd);timer_primary_action(g_main_hwnd);
    CHECK(remaining_seconds==900 && current_timer_mode==TIMER_SHORT_POMODORO,"early break completion returns exact original focus value");
    timer_advance_seconds(g_main_hwnd,900);
    CHECK(micro.source==TIMER_NONE && is_overtime && overtime_seconds==0,"original final tail completes without extra micro");
    timer_test_reset();start_timer(g_main_hwnd,45,TIMER_SHORT_POMODORO);timer_advance_seconds(g_main_hwnd,1020);
    CHECK(overtime_seconds==120 && remaining_seconds==1800 && micro.frozen_seconds==1800,"a single elapsed batch splits at boundary without swallowing waiting time");
    test_micro_case(40,0,2);test_micro_case(40,1200,2);test_micro_case(40,2100,2);
    test_micro_case(45,120,2);test_micro_case(50,2100,3);
    timer_test_reset();start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);
    remaining_seconds=1200;timer_advance_seconds(g_main_hwnd,600);
    CHECK(micro.phase==MICRO_WAIT_START && (micro.frozen_seconds == micro.base_seconds - 2 * micro.interval_seconds) && micro.frozen_seconds==600,"manual skip reaches original positive tail without catch-up");
    timer_test_reset();start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);remaining_seconds=600;timer_advance_seconds(g_main_hwnd,1);
    CHECK(micro.phase==MICRO_NONE,"manual setting exactly at tick does not trigger");
    timer_test_reset();settings.enable_overtime_count_up=0;start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);
    timer_advance_seconds(g_main_hwnd,900);timer_advance_seconds(g_main_hwnd,2100);
    CHECK(!is_running && !is_overtime && micro.phase==MICRO_WAIT_START && micro.frozen_seconds==1500,"disabled overtime retains stopped waiting stage");
    timer_primary_action(g_main_hwnd);timer_advance_seconds(g_main_hwnd,60);timer_advance_seconds(g_main_hwnd,2100);
    CHECK(!is_running && micro.phase==MICRO_WAIT_RESUME && micro.frozen_seconds==1500,"disabled overtime after micro never auto resumes focus");
    timer_primary_action(g_main_hwnd);CHECK(remaining_seconds==1500,"confirmation with overtime disabled returns original focus time");
    timer_test_reset();settings=saved;save_settings();
    printf("MICRO_WAIT_OVERTIME_TEST: %s (%d failures)\n",failures ? "FAIL" : "PASS",failures);
}
