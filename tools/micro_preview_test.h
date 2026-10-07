/* Focused v2.5.39 regressions: actual timer engine and native overlay windows. */
static void test_micro_case(int minutes, int extension, int expected) {
    int k, total = minutes * 60 + extension;
    timer_test_reset();
    start_timer(g_main_hwnd, minutes, TIMER_SHORT_POMODORO);
    timer_advance_seconds(g_main_hwnd, 900);
    CHECK(micro.phase == MICRO_WAIT_START && timer_tick_handled(1), "original first tick triggers");
    timer_advance_seconds(g_main_hwnd, extension);
    CHECK(micro.extension_seconds == extension, "only waiting overtime extends parent");
    for (k = 1; k <= expected; ++k) {
        CHECK(micro.phase == MICRO_WAIT_START && micro.triggered_count == k && overtime_seconds == total-k*900,
            "eligible original or extension tick triggers exactly once at shifted value");
        timer_primary_action(g_main_hwnd);
        int frozen = micro.frozen_seconds;
        timer_advance_seconds(g_main_hwnd, 61);
        CHECK(micro.frozen_seconds == frozen && micro.extension_seconds == extension, "micro expiry overtime does not extend parent");
        timer_primary_action(g_main_hwnd);
        CHECK(remaining_seconds == frozen && current_timer_mode == TIMER_SHORT_POMODORO, "resume retains frozen parent time");
        if (k < expected) timer_advance_seconds(g_main_hwnd, 900);
    }
    timer_advance_seconds(g_main_hwnd, remaining_seconds);
    CHECK(micro.source == TIMER_NONE && is_overtime && overtime_seconds == 0, "no extra tail or zero tick before completion");
    timer_advance_seconds(g_main_hwnd, 1800);
    CHECK(micro.phase == MICRO_NONE, "completed focus overtime never creates micro ticks");
}
static void test_micro_preview(void) {
    TimerSettings saved = settings;
    FullscreenMonitors online, subset;
    FullscreenView view;
    settings.enable_micro_break = 1; settings.micro_break_interval_minutes = 15; settings.micro_break_duration_minutes = 1;
    settings.enable_overtime_count_up = 1; settings.enable_pomodoro_count = 0;
    settings.enable_clock_sound = settings.enable_completion_sound = settings.show_completion_dialog = 0;
    test_micro_case(40,0,2); test_micro_case(40,1199,2); test_micro_case(40,1200,3);
    test_micro_case(40,2099,3); test_micro_case(40,2100,4);
    test_micro_case(45,0,2); test_micro_case(50,0,3);
    timer_test_reset(); start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);
    timer_advance_seconds(g_main_hwnd,1500); /* first waiting point gains ten minutes */
    timer_primary_action(g_main_hwnd); timer_stop_action(g_main_hwnd); timer_primary_action(g_main_hwnd);
    timer_advance_seconds(g_main_hwnd,1500); /* second waiting point gains another ten */
    CHECK(micro.extension_seconds==1200 && overtime_seconds==1800 && micro.triggered_count==2,"extension accumulates across multiple waiting points");
    timer_primary_action(g_main_hwnd); timer_stop_action(g_main_hwnd); timer_primary_action(g_main_hwnd);
    timer_advance_seconds(g_main_hwnd,900);
    CHECK(micro.phase==MICRO_WAIT_START && micro.triggered_count==3 && overtime_seconds==900,"cumulative extension adds third tick without repeating earlier ordinals");
    timer_test_reset(); start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);
    remaining_seconds=1200; timer_advance_seconds(g_main_hwnd,600);
    CHECK(micro.phase==MICRO_WAIT_START && micro.triggered_count==1 && timer_tick_handled(2) && micro.extension_seconds==0,"manual skip retains original tail without catch-up or extension");
    timer_test_reset(); start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);
    remaining_seconds=600; timer_advance_seconds(g_main_hwnd,1);
    CHECK(micro.phase==MICRO_NONE,"manual setting exactly at tail tick does not trigger");
    timer_test_reset(); settings.enable_overtime_count_up = 0;
    start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);
    timer_advance_seconds(g_main_hwnd,900);
    timer_primary_action(g_main_hwnd); timer_stop_action(g_main_hwnd); timer_primary_action(g_main_hwnd);
    timer_advance_seconds(g_main_hwnd,900);
    CHECK(micro.phase == MICRO_WAIT_START && remaining_seconds == 600 && !is_running, "original short tail remains eligible with overtime disabled");
    timer_test_reset();
    CHECK(fs_get_monitors(&online) && online.count > 0,"native monitor inventory exists");
    fs_read_timer_view(&view);
    fs_start_preview(NULL,0,&view);
    CHECK(fs_preview_active && fs_count == online.count,"preview from no selection covers every online screen");
    fs_end_preview(); CHECK(!fs_active && !fs_count,"preview from no selection exits every screen");
    if (online.count) {
        size_t n, i;
        for (n=1; n<=online.count; ++n) {
            fs_begin();
            for(i=0;i<n;++i) fs_add_screen(&online.items[i]);
            fs_start_preview(NULL,0,&view);
            CHECK(fs_count == online.count,"preview expands existing screen selection to all screens");
            for(i=0;i<online.count;++i) CHECK(fs_window_index(online.items[i].identity)>=0,"all online screen identities included");
            subset=online;
            if(subset.count>1) {
                FullscreenMonitor swap=subset.items[0]; subset.items[0]=subset.items[subset.count-1]; subset.items[subset.count-1]=swap;
            }
            fs_reconcile_selection(&subset,1);
            CHECK(fs_count == online.count,"simulated enumeration reorder preserves preview coverage");
            if(online.count>1) {
                subset=online; subset.count=1; fs_reconcile_selection(&subset,1);
                CHECK(fs_count==1,"simulated offline screens removed during preview");
                fs_build_windows(); CHECK(fs_count==online.count,"preview restores newly online screens");
            }
            fs_end_preview();
            CHECK(!fs_preview_active && fs_count == n,"preview restores original selected screen count");
            for(i=0;i<online.count;++i) CHECK((fs_window_index(online.items[i].identity)>=0)==(i<n),"preview restores original identities rather than enumeration positions");
            fs_exit();
        }
    }
    CHECK(!is_running && micro.phase==MICRO_NONE && !session_rules.valid,"preview leaves timer and session unchanged");
    choose_menu(ID_MENU_FULLSCREEN_SHOW_TEXT); choose_menu(ID_MENU_FULLSCREEN_SHOW_TEXT);
    settings=saved; timer_test_reset(); save_settings();
    printf("MICRO_PREVIEW_TEST: %s (%d failures)\n",failures ? "FAIL" : "PASS",failures);
}
