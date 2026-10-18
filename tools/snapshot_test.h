static void snapshot_test_restore(void) { choose_menu(ID_MENU_RESTORE_SNAPSHOT); }
static void test_snapshot(void) {
    TimerSettings saved=settings;
    test_io_active=test_notify_stub=test_silent_sound=1;
    settings.enable_clock_sound=settings.enable_completion_sound=settings.enable_micro_break=0;
    settings.reminder_mode=0;settings.enable_overtime_count_up=1;
    TimerMode modes[]={TIMER_LONG_POMODORO,TIMER_SHORT_POMODORO,TIMER_SHORT_BREAK,TIMER_LONG_BREAK,TIMER_CUSTOM,TIMER_COUNT_UP};
    for(int m=0;m<6;++m)for(int overtime=0;overtime<2;++overtime)for(int entry=0;entry<5;++entry) {
        consistency_reset();snapshot_clear();settings.reminder_mode=0;
        start_timer(g_main_hwnd,3,modes[m]);consistency_clock();remaining_seconds=87;timer_remainder_ms=375;
        if(overtime){is_overtime=1;overtime_source_mode=modes[m];current_timer_mode=TIMER_NONE;overtime_seconds=37;}
        int count=get_today_count_from_storage();unsigned generation=timer_stage_generation;
        if(entry==2)fs_show_all();
        if(entry==3){settings.reminder_mode=1;ShowCompletionNotification(g_main_hwnd,modes[m]);}
        if(entry==4){settings.reminder_mode=2;CHECK(sr_start(),"snapshot strong entry fixture starts");}
        resume_entry(entry);
        CHECK(snapshot_valid && timer_snapshot.mode==(overtime?TIMER_NONE:modes[m]) && timer_snapshot.seconds==87,"all action entries capture original context once");
        CHECK(overtime ? is_running&&!is_overtime : !is_running,"primary action performs its original transition");
        snapshot_test_restore();
        CHECK(is_running&&!is_paused && !!is_overtime==overtime && remaining_seconds==87 && overtime_seconds==(overtime?37:0),"restore resumes original ordinary or overtime clock");
        CHECK(current_timer_mode==(overtime?TIMER_NONE:modes[m]) && timer_remainder_ms==375 && session_rules.initial_seconds==(modes[m]==TIMER_COUNT_UP?0:180),"restore preserves mode fraction and session cap");
        CHECK(!snapshot_valid && !repeat_reminder.valid && !sr_active() && !g_hToastWnd,"restore without a saved completion event consumes undo and leaves reminders clear");
        CHECK(get_today_count_from_storage()==count && timer_stage_generation>generation,"restore does not credit and uses fresh generation");
        consistency_clock();
    }
    consistency_reset();snapshot_clear();settings.reminder_mode=0;settings.enable_overtime_count_up=0;
    idle_mode=IDLE_CUSTOM;settings.custom_duration=4;choose_menu(ID_MENU_START_CURRENT);
    CHECK(snapshot_valid && !timer_snapshot.running && timer_snapshot.idle==IDLE_CUSTOM,"starting idle stores the original ready target");
    settings.custom_duration=6;snapshot_test_restore();
    CHECK(timer_ui_is_ready() && timer_ui_resolve().mode==TIMER_CUSTOM && timer_ui_resolve().seconds==360,"restored ordinary ready state uses current defaults");
    CHECK(!snapshot_valid,"second restore is unavailable");snapshot_test_restore();
    start_timer(g_main_hwnd,5,TIMER_SHORT_POMODORO);consistency_clock();remaining_seconds=123;timer_remainder_ms=650;
    choose_menu(ID_MENU_IDLE_LONG_BREAK);CHECK(snapshot_valid && timer_ui_is_ready(),"ready selection captures active session before clearing");
    snapshot_test_restore();CHECK(is_running && current_timer_mode==TIMER_SHORT_POMODORO && remaining_seconds==123,"ready undo returns active focus");consistency_clock();
    choose_menu(ID_MENU_START_CUSTOM);CHECK(snapshot_valid && timer_snapshot.mode==TIMER_SHORT_POMODORO,"switch stores old focus rather than nested cleared state");
    choose_menu(ID_MENU_IDLE_COUNT_UP);CHECK(timer_snapshot.mode==TIMER_CUSTOM,"new action replaces previous undo");
    snapshot_test_restore();CHECK(current_timer_mode==TIMER_CUSTOM && is_running,"single undo restores only most recent switch");consistency_clock();
    timer_primary_action(g_main_hwnd);TimerSnapshot prior=timer_snapshot;
    settings.short_pomodoro_duration=46;
    CHECK(snapshot_valid && timer_snapshot.seconds==prior.seconds,"unrelated operations preserve undo");
    snapshot_test_restore();consistency_clock();timer_pause_toggle(g_main_hwnd);
    choose_menu(ID_MENU_START_COUNT_UP);CHECK(timer_snapshot.paused && !timer_snapshot.running,"switch from paused preserves paused origin");
    snapshot_test_restore();CHECK(is_paused&&!is_running && current_timer_mode==TIMER_CUSTOM,"paused origin stays paused on restore");
    timer_primary_action(g_main_hwnd);consistency_clock();timer_primary_action(g_main_hwnd);prior=timer_snapshot;
    choose_menu(ID_MENU_START_CUSTOM);timer_pause_toggle(g_main_hwnd);timer_primary_action(g_main_hwnd);
    CHECK(snapshot_valid && !timer_snapshot.running,"paused primary resume does not overwrite previous ready snapshot");
    consistency_clock();timer_pause_toggle(g_main_hwnd);timer_pause_toggle(g_main_hwnd);consistency_clock();
    CHECK(snapshot_valid && !timer_snapshot.running,"pause toggle leaves snapshot unchanged");
    /* Millisecond residue must come from the active monotonic clock, not stale cache. */
    consistency_reset();snapshot_clear();start_timer(g_main_hwnd,5,TIMER_CUSTOM);
    timer_remainder_ms=0;timer_last_tick=GetTickCount64()-640;timer_primary_action(g_main_hwnd);
    CHECK(timer_snapshot.remainder>=640 && timer_snapshot.remainder<900,"capture reads live sub-second residue");
    snapshot_test_restore();CHECK(timer_remainder_ms>=640 && timer_remainder_ms<900,"live residue survives undo");consistency_clock();
    /* All natural boundaries invalidate even when notifications/counting are disabled. */
    for(int overtime=0;overtime<2;++overtime)for(int m=0;m<5;++m){
        consistency_reset();snapshot_clear();settings.enable_overtime_count_up=overtime;settings.enable_pomodoro_count=0;
        choose_menu(ID_MENU_START_CUSTOM);consistency_clock();start_timer(g_main_hwnd,0,modes[m]);consistency_clock();timer_advance_seconds(g_main_hwnd,0);
        CHECK(!snapshot_valid,"every countdown completion invalidates undo regardless of overtime");
    }
    for(int overtime=0;overtime<2;++overtime){
        consistency_reset();snapshot_clear();settings.enable_micro_break=1;settings.enable_overtime_count_up=overtime;
        settings.micro_break_interval_minutes=15;settings.micro_break_duration_minutes=1;
        choose_menu(1);consistency_clock();timer_advance_seconds(g_main_hwnd,900);
        CHECK(!snapshot_valid && micro.phase==MICRO_WAIT_START,"natural micro due invalidates old undo");
        timer_primary_action(g_main_hwnd);consistency_clock();
        CHECK(snapshot_valid && timer_snapshot.micro.phase==MICRO_WAIT_START,"starting micro saves waiting origin");
        snapshot_test_restore();consistency_clock();
        CHECK(micro.phase==MICRO_WAIT_START && micro.frozen_seconds==4500,"undo micro start restores original frozen focus");
        timer_primary_action(g_main_hwnd);consistency_clock();timer_advance_seconds(g_main_hwnd,60);
        CHECK(!snapshot_valid && micro.phase==MICRO_WAIT_RESUME,"natural micro completion clears undo");
        timer_primary_action(g_main_hwnd);consistency_clock();snapshot_test_restore();consistency_clock();
        CHECK(micro.phase==MICRO_WAIT_RESUME && micro.frozen_seconds==4500,"undo focus resume restores micro completion wait");
        timer_primary_action(g_main_hwnd);consistency_clock();timer_primary_action(g_main_hwnd);snapshot_test_restore();consistency_clock();
        CHECK(current_timer_mode==TIMER_LONG_POMODORO && remaining_seconds==4500 && micro.source==TIMER_LONG_POMODORO,"undo focus end preserves micro rules and frozen context");
    }
    consistency_reset();snapshot_clear();settings.enable_micro_break=0;settings.enable_overtime_count_up=0;settings.enable_pomodoro_count=1;
    choose_menu(ID_MENU_START_SHORT_POMODORO);remaining_seconds=0;
    int count=get_today_count_from_storage();snapshot_test_restore();
    CHECK(!snapshot_valid && !is_running && get_today_count_from_storage()==count+1,"restore synchronizes natural completion and cannot resurrect settled session");
    snapshot_test_restore();CHECK(get_today_count_from_storage()==count+1,"stale restore never repeats completion credit");
    consistency_reset();snapshot_clear();start_timer(g_main_hwnd,3,TIMER_CUSTOM);consistency_clock();remaining_seconds=53;timer_primary_action(g_main_hwnd);
    test_resume_fail=1;snapshot_test_restore();
    CHECK(snapshot_valid && snapshot_retry && is_paused && !is_running && remaining_seconds==53,"failed restore retains target paused and undo for retry");
    test_resume_fail=0;timer_primary_action(g_main_hwnd);
    CHECK(!snapshot_valid && !snapshot_retry && is_running && remaining_seconds==53,"successful paused retry consumes undo without overwriting");consistency_clock();
    timer_primary_action(g_main_hwnd);unsigned old=timer_stage_generation;snapshot_test_restore();consistency_clock();
    settings.reminder_mode=2;notification_reminder_mode=2;SendMessageW(g_main_hwnd,WM_TOAST_NOTIFY,TIMER_CUSTOM,old);
    CHECK(!sr_active()&&!g_hToastWnd,"old notification generation is rejected after restore");
    consistency_reset();snapshot_clear();settings=saved;save_settings();test_io_active=test_notify_stub=test_silent_sound=0;
    printf("SNAPSHOT_TEST: %s (%d failures)\n",failures?"FAIL":"PASS",failures);
}
