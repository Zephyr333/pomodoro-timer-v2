/* Focused UI checks; does not run the existing timer/statistics/drag suites. */
static void test_ui_copy(void) {
    TimerSettings saved = settings;
    FullscreenView view;
    wchar_t text[96];
    settings.enable_clock_sound = settings.enable_completion_sound = 0;
    settings.reminder_mode = 1;
    timer_test_reset();
    CHECK(!wcscmp(timer_ui_pause_label(), L"暂停") && !timer_ui_can_pause() && timer_ui_can_start() && !timer_ui_can_end(), "idle labels and availability");
    choose_menu(0);
    TimerMode modes[] = {TIMER_LONG_POMODORO, TIMER_SHORT_POMODORO, TIMER_SHORT_BREAK, TIMER_LONG_BREAK, TIMER_COUNT_UP, TIMER_CUSTOM, TIMER_MICRO_BREAK};
    int i;
    for (i = 0; i < 7; ++i) {
        current_timer_mode = modes[i]; is_running = 1; remaining_seconds = 123;
        fs_read_timer_view(&view);
        swprintf(text, 96, L"%ls", timer_mode_name(modes[i]));
        CHECK(!wcscmp(view.status, text), "running has the shared three-character suffix");
        CHECK(!wcscmp(timer_ui_primary_label(), L"结束") && timer_ui_can_start() && timer_ui_can_pause() && timer_ui_can_end(), "running action rules");
        update_tray_icon(g_main_hwnd, L"2", 0, 123);
        swprintf(text, 96, L"%ls %ls", view.status, view.time);
        CHECK(!wcscmp(nid.szTip, text), "tooltip shares mode time and status");
        is_running = 0; is_paused = 1;
        fs_read_timer_view(&view);
        swprintf(text, 96, L"%ls · 已暂停", timer_mode_name(modes[i]));
        CHECK(!wcscmp(view.status, text) && !wcscmp(timer_ui_pause_label(), L"继续") && !wcscmp(timer_ui_primary_label(), L"结束"), "paused uses shared status and continue");
        is_paused = 0; is_running = 1; is_overtime = 1; overtime_source_mode = modes[i]; overtime_seconds = 11;
        fs_read_timer_view(&view);
        CHECK(!wcscmp(view.status, L"超时正计时") && !wcscmp(view.time, L"+00:11"), "all overtime sources share one mode name");
        is_running = 0; is_paused = 1;
        fs_read_timer_view(&view);
        CHECK(!wcscmp(view.status, L"超时正计时 · 已暂停") && !wcscmp(timer_ui_primary_label(), L"开始"), "paused overtime begins next segment rather than showing continue in HUD");
        is_paused = 0; is_overtime = 0;
    }
    timer_test_reset();
    micro.source = TIMER_SHORT_POMODORO; current_timer_mode = TIMER_SHORT_POMODORO; micro.phase = MICRO_WAIT_START; micro.duration_seconds = 60; remaining_seconds = 1800;
    fs_read_timer_view(&view);
    CHECK(!wcscmp(view.status, L"微休息 · 未开始") && !wcscmp(view.time, L"01:00"), "non-overtime micro due retains source mode and real time");
    CHECK(timer_ui_can_start() && !timer_ui_can_pause() && !timer_ui_can_end(), "static pre-micro waiting action rules");
    choose_menu(0);
    timer_ui_notification(text, 96, TIMER_MICRO_BREAK);
    CHECK(!wcscmp(text, L"微休息 · 未开始"), "micro due notification is concise");
    micro.phase = MICRO_WAIT_RESUME; current_timer_mode = TIMER_MICRO_BREAK; remaining_seconds = 0; micro.frozen_seconds = 1800;
    fs_read_timer_view(&view);
    CHECK(!wcscmp(view.status, L"短番茄钟 · 未开始") && !wcscmp(view.time, L"30:00"), "completed micro uses common completion state");
    CHECK(timer_ui_can_start() && !timer_ui_can_pause() && !timer_ui_can_end(), "static post-micro waiting has one progression action");
    choose_menu(ID_MENU_STOP);
    CHECK(micro.phase == MICRO_WAIT_RESUME && micro.frozen_seconds == 1800, "disabled end command cannot bypass waiting restriction");
    MicroPhase phases[] = {MICRO_WAIT_START, MICRO_WAIT_RESUME};
    for (i = 0; i < 2; ++i) {
        micro.phase = phases[i]; is_overtime = 1; is_running = 1; overtime_seconds = 9;
        CHECK(timer_ui_can_pause(), "micro-derived overtime offers pause");
        choose_menu(ID_MENU_PAUSE_RESUME);
        CHECK(is_paused && !is_running && !wcscmp(timer_ui_pause_label(), L"继续"), "micro overtime actually pauses");
        CHECK(timer_ui_can_start() && timer_ui_can_end(), "paused overtime start and end availability");
        choose_menu(ID_MENU_PAUSE_RESUME);
        CHECK(is_running && !is_paused && overtime_seconds == 9 && micro.frozen_seconds == 1800, "continue retains overtime and frozen focus");
        stop_timer_clock();
    }
    timer_test_reset();
    completed_pending_mode = TIMER_SHORT_BREAK;
    ShowCompletionNotification(g_main_hwnd, TIMER_SHORT_BREAK);
    GetWindowTextW(g_hToastButton, text, 96);
    CHECK(!wcscmp(text, L"开始"), "initial popup action is always start");
    choose_menu(ID_MENU_DEFAULT_SHORT_POMODORO);
    GetWindowTextW(g_hToastButton, text, 96);
    CHECK(!wcscmp(text, L"开始"), "popup refresh retains the same action wording");
    timer_test_reset();
    settings = saved;
    printf("UI_COPY_TEST: %s (%d failures)\n", failures ? "FAIL" : "PASS", failures);
}
