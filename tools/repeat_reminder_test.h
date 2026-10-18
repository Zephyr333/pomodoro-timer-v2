static void repeat_test_close_popup(void) {
    CHECK(g_hToastWnd != NULL, "notification popup exists");
    SendMessageW(g_hToastWnd, WM_COMMAND, MAKEWPARAM(ID_TOAST_CLOSE, BN_CLICKED), 0);
}

static void test_repeat_reminder(void) {
    TimerSettings saved = settings;
    test_io_active = test_notify_stub = test_silent_sound = 1;
    settings.enable_clock_sound = settings.enable_micro_break = 0;
    settings.enable_completion_sound = settings.enable_pomodoro_count = 1;
    settings.enable_repeat_reminder = 1;
    settings.repeat_reminder_interval_minutes = 5;
    TimerMode modes[] = {TIMER_LONG_POMODORO,TIMER_SHORT_POMODORO,TIMER_SHORT_BREAK,TIMER_LONG_BREAK,TIMER_CUSTOM};
    for (int m=0; m<5; ++m) for (int overtime=0; overtime<2; ++overtime) {
        consistency_reset(); settings.reminder_mode=1; settings.enable_overtime_count_up=overtime;
        start_timer(g_main_hwnd,0,modes[m]); consistency_clock(); timer_advance_seconds(g_main_hwnd,0); pump(10);
        CHECK(g_hToastWnd && !repeat_reminder.armed,"first popup does not start repeat countdown");
        int count=get_today_count_from_storage(), sound=test_one_shot_sound_count;
        unsigned generation=timer_stage_generation;
        SendMessageW(g_hToastWnd,WM_COMMAND,MAKEWPARAM(ID_TOAST_COLLAPSE,BN_CLICKED),0);
        CHECK(g_toast_collapsed && !repeat_reminder.armed,"collapse is not dismissal");
        ULONGLONG before=GetTickCount64(); repeat_test_close_popup();
        CHECK(repeat_reminder.armed && repeat_reminder.due>=before+300000,"manual close starts full five-minute delay");
        repeat_reminder_tick(repeat_reminder.due-1);
        CHECK(!g_hToastWnd && test_one_shot_sound_count==sound,"nothing fires before deadline");
        repeat_reminder_tick(repeat_reminder.due);
        CHECK(g_hToastWnd && !repeat_reminder.armed && test_one_shot_sound_count==sound+1,"deadline redisplays once and waits for dismissal");
        repeat_reminder_tick(GetTickCount64()+3600000);
        CHECK(test_one_shot_sound_count==sound+1,"visible reminder never repeats or stacks");
        CHECK(get_today_count_from_storage()==count && timer_stage_generation==generation && !!is_overtime==overtime,"repeat preserves statistics stage and overtime");
        repeat_test_close_popup();
        CHECK(repeat_reminder.armed,"second close schedules a new interval");
        timer_primary_action(g_main_hwnd);
        CHECK(!repeat_reminder.valid && !repeat_reminder.armed,"starting next segment cancels old reminder");
    }
    for(int overtime=0;overtime<2;++overtime) {
        consistency_reset();settings.enable_micro_break=1;settings.micro_break_interval_minutes=15;
        settings.micro_break_duration_minutes=1;settings.enable_overtime_count_up=overtime;settings.reminder_mode=1;
        start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);consistency_clock();timer_advance_seconds(g_main_hwnd,900);pump(10);
        for(int phase=0;phase<2;++phase) {
            CHECK(micro.phase==(phase?MICRO_WAIT_RESUME:MICRO_WAIT_START),"micro boundary retains expected waiting phase");
            repeat_test_close_popup();repeat_reminder_tick(repeat_reminder.due);
            CHECK(g_hToastWnd && micro.frozen_seconds==1500,"micro repeat preserves frozen focus");
            repeat_test_close_popup();timer_primary_action(g_main_hwnd);consistency_clock();
            CHECK(!repeat_reminder.valid,"micro primary action cancels previous wait");
            if(!phase){timer_advance_seconds(g_main_hwnd,60);pump(10);}
        }
        CHECK(current_timer_mode==TIMER_SHORT_POMODORO && remaining_seconds==1500,"micro repeat chain returns to original focus");
    }
    settings.enable_micro_break=0;settings.enable_overtime_count_up=1;
    for(int input=0;input<4;++input) {
        consistency_reset();settings.reminder_mode=2;
        start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(20);
        CHECK(sr_active() && !repeat_reminder.armed,"strong reminder initially waits for dismissal");
        if(input==0)SendMessageW(sr.items[0].window,WM_LBUTTONUP,0,0);
        if(input==1)SendMessageW(sr.items[0].window,WM_MBUTTONUP,0,0);
        if(input==2)SendMessageW(sr.items[0].window,WM_KEYDOWN,VK_ESCAPE,0);
        if(input==3)SendMessageW(sr.items[0].window,WM_CLOSE,0,0);
        CHECK(!sr_active() && repeat_reminder.armed,"strong manual close schedules repeat");
        timer_pause_toggle(g_main_hwnd);
        CHECK(is_paused && repeat_reminder.armed,"overtime pause preserves repeat wait");
        timer_primary_action(g_main_hwnd);consistency_clock();
        CHECK(!is_paused && repeat_reminder.armed,"overtime resume preserves repeat wait");
        repeat_reminder_tick(repeat_reminder.due);
        CHECK(sr_active() && !repeat_reminder.armed,"strong reminder can redisplay in same stage");
        SendMessageW(sr.items[0].window,WM_RBUTTONUP,0,0);
        CHECK(!repeat_reminder.valid && is_running && !is_overtime,"strong action advances instead of rescheduling");
    }
    consistency_reset();settings.reminder_mode=1;
    start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(10);
    close_toast_notification_if_open();CHECK(!repeat_reminder.armed,"internal popup destruction never arms repeat");
    ShowCompletionNotification(g_main_hwnd,TIMER_CUSTOM);SendMessageW(g_hToastWnd,WM_CLOSE,0,0);
    CHECK(repeat_reminder.armed,"system popup close also arms repeat");
    ULONGLONG deadline=repeat_reminder.due;
    timer_apply_live_preferences();CHECK(repeat_reminder.due==deadline,"unrelated preference refresh does not delay reminder");
    HANDLE locked=CreateFileW(g_settings_path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    CHECK(locked!=INVALID_HANDLE_VALUE,"settings lock fixture created");
    if(locked!=INVALID_HANDLE_VALUE){choose_menu(ID_MENU_REPEAT_REMINDER);CloseHandle(locked);}
    CHECK(settings.enable_repeat_reminder && repeat_reminder.armed && repeat_reminder.due==deadline,"failed switch save preserves preference and active wait");
    choose_menu(ID_MENU_REPEAT_REMINDER);CHECK(!settings.enable_repeat_reminder && !repeat_reminder.armed,"off switch cancels waiting timer");
    choose_menu(ID_MENU_REPEAT_REMINDER);CHECK(settings.enable_repeat_reminder && !repeat_reminder.armed,"on switch does not resurrect old wait");
    ShowCompletionNotification(g_main_hwnd,TIMER_CUSTOM);repeat_test_close_popup();
    consistency_input_value=3;SetTimer(NULL,0,30,consistency_input_tick);choose_menu(ID_MENU_REPEAT_INTERVAL);
    CHECK(settings.repeat_reminder_interval_minutes==3,"duration menu edits repeat interval through real input dialog");
    CHECK(repeat_reminder.armed && repeat_reminder.due>=GetTickCount64()+179900 && repeat_reminder.due<=GetTickCount64()+180000,"changing interval restarts pending wait");
    settings.reminder_mode=0;settings.enable_completion_sound=0;
    repeat_reminder_tick(repeat_reminder.due);CHECK(!g_hToastWnd && !sr_active() && repeat_reminder.armed,"disabled visual and sound settings remain quiet");
    settings.reminder_mode=1;settings.enable_completion_sound=1;fs_show_all();
    int sound=test_one_shot_sound_count;repeat_reminder_tick(repeat_reminder.due);
    CHECK(!g_hToastWnd && repeat_reminder.armed && test_one_shot_sound_count==sound+1,"fullscreen suppresses popup but retains bounded retry");
    settings.reminder_mode=2;test_fail_strong_window=1;repeat_reminder_tick(repeat_reminder.due);test_fail_strong_window=0;
    CHECK(g_hToastWnd && !repeat_reminder.armed && !sr_active(),"failed strong repeat falls back to visible popup");
    repeat_test_close_popup();choose_menu(ID_MENU_IDLE_CUSTOM);
    CHECK(!repeat_reminder.valid,"explicit ready selection cancels old repeat");
    consistency_reset();start_timer(g_main_hwnd,0,TIMER_COUNT_UP);consistency_clock();timer_advance_seconds(g_main_hwnd,60);
    CHECK(!repeat_reminder.valid,"independent count-up has no completion reminder");
    consistency_reset();settings.reminder_mode=1;
    start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(10);repeat_test_close_popup();
    repeat_reminder.due=GetTickCount64();SendMessageW(g_main_hwnd,WM_TIMER,ID_REPEAT_REMINDER_TIMER,0);
    CHECK(g_hToastWnd && !repeat_reminder.armed,"native timer dispatch delivers repeat notification");
    consistency_reset();reset_defaults_keep_data();
    CHECK(settings.enable_repeat_reminder==1 && settings.repeat_reminder_interval_minutes==5,"default switch and interval are enabled and five minutes");
    settings.enable_repeat_reminder=0;settings.repeat_reminder_interval_minutes=7;CHECK(save_settings(),"repeat preferences save");
    settings.enable_repeat_reminder=1;settings.repeat_reminder_interval_minutes=99;load_settings();
    CHECK(!settings.enable_repeat_reminder && settings.repeat_reminder_interval_minutes==7,"repeat preferences round-trip");
    CHECK(config_write_atomic(g_settings_path,"{\"pomodoro_duration\":90,\"pomodoro_count\":0}"),"legacy settings fixture written");load_settings();
    CHECK(settings.enable_repeat_reminder==1 && settings.repeat_reminder_interval_minutes==5,"legacy config receives defaults");
    CHECK(config_write_atomic(g_settings_path,"{\"pomodoro_duration\":90,\"pomodoro_count\":0,\"repeat_reminder_interval_minutes\":0}"),"invalid interval fixture written");load_settings();
    CHECK(settings.repeat_reminder_interval_minutes==1,"invalid persisted interval clamps to positive range");
    consistency_reset();settings=saved;save_settings();test_io_active=test_notify_stub=test_silent_sound=0;
    printf("REPEAT_REMINDER_TEST: %s (%d failures)\n",failures?"FAIL":"PASS",failures);
}
