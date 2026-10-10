static void resume_entry(int entry) {
    switch(entry){case 0:choose_menu(ID_MENU_START_CURRENT);break;case 1:SendMessageW(g_main_hwnd,WM_USER+1,0,WM_LBUTTONUP);break;
    case 2:SendMessageW(fs_windows[0],WM_RBUTTONUP,0,0);break;case 3:SendMessageW(g_hToastWnd,WM_COMMAND,MAKEWPARAM(ID_TOAST_ACTION,BN_CLICKED),0);break;
    case 4:SendMessageW(sr.items[0].window,WM_RBUTTONUP,0,0);break;case 5:choose_menu(ID_MENU_PAUSE_RESUME);break;}
}
static void test_resume(void) {
    TimerSettings saved=settings;test_io_active=test_notify_stub=test_silent_sound=1;
    settings.enable_clock_sound=settings.enable_completion_sound=settings.enable_micro_break=0;settings.enable_overtime_count_up=1;settings.reminder_mode=0;
    TimerMode modes[]={TIMER_LONG_POMODORO,TIMER_SHORT_POMODORO,TIMER_SHORT_BREAK,TIMER_LONG_BREAK,TIMER_CUSTOM,TIMER_COUNT_UP};
    for(int m=0;m<6;++m)for(int overtime=0;overtime<2;++overtime)for(int entry=0;entry<6;++entry){
        consistency_reset();settings.reminder_mode=0;start_timer(g_main_hwnd,2,modes[m]);
        if(overtime){is_overtime=1;overtime_source_mode=modes[m];current_timer_mode=TIMER_NONE;overtime_seconds=37;}
        remaining_seconds=37;
        timer_pause_toggle(g_main_hwnd);timer_remainder_ms=650;
        unsigned generation=timer_stage_generation;int count=get_today_count_from_storage();int cap=session_rules.initial_seconds;
        CHECK(!wcscmp(timer_ui_primary_label(),L"开始")&&!wcscmp(timer_ui_pause_label(),L"暂停"),"paused commands are start and checked pause, never separate continue");
        if(entry==2)fs_show_all();if(entry==3){settings.reminder_mode=1;ShowCompletionNotification(g_main_hwnd,modes[m]);}
        if(entry==4){settings.reminder_mode=2;CHECK(sr_start(),"paused strong reminder fixture starts");}
        resume_entry(entry);
        CHECK(is_running&&!is_paused&&!!is_overtime==overtime&&remaining_seconds==37,"one action resumes rather than ending or advancing");
        CHECK(current_timer_mode==(overtime?TIMER_NONE:modes[m])&&overtime_seconds==(overtime?37:0),"resume preserves current mode and overtime value");
        CHECK(timer_stage_generation==generation&&session_rules.initial_seconds==cap&&timer_remainder_ms==650&&get_today_count_from_storage()==count,"resume preserves generation cap fraction and statistics");
        timer_primary_action(g_main_hwnd);CHECK(overtime?!is_overtime:!is_running,"next action follows running semantics");
    }
    for(int phase=0;phase<3;++phase){consistency_reset();settings.reminder_mode=0;settings.enable_micro_break=1;settings.micro_break_interval_minutes=15;settings.micro_break_duration_minutes=1;
        start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);consistency_clock();timer_advance_seconds(g_main_hwnd,900);
        if(phase){timer_primary_action(g_main_hwnd);if(phase==2)timer_advance_seconds(g_main_hwnd,60);}
        is_running=1;launch_timer_clock(g_main_hwnd);timer_pause_toggle(g_main_hwnd);MicroPhase before=micro.phase;unsigned generation=timer_stage_generation;
        timer_primary_action(g_main_hwnd);CHECK(is_running&&!is_paused&&micro.phase==before&&micro.frozen_seconds==1500&&generation==timer_stage_generation,"each micro phase resumes without changing frozen parent or advancing");
    }
    consistency_reset();settings.enable_micro_break=0;settings.reminder_mode=0;start_timer(g_main_hwnd,1,TIMER_CUSTOM);timer_pause_toggle(g_main_hwnd);timer_remainder_ms=750;
    test_resume_fail=1;timer_primary_action(g_main_hwnd);CHECK(is_paused&&!is_running&&remaining_seconds==60&&timer_remainder_ms==750,"failed resume retains paused state and real time");
    test_resume_fail=0;timer_primary_action(g_main_hwnd);CHECK(is_running&&!is_paused,"start retries failed resume");
    consistency_reset();settings=saved;test_io_active=test_notify_stub=test_silent_sound=0;printf("RESUME_TEST: %s (%d failures)\n",failures?"FAIL":"PASS",failures);
}
