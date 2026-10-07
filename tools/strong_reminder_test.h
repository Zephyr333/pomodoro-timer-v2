/* Strong reminder and merged radio preferences; isolated native windows and owned JSON. */
static void strong_test_reset(void) { sr_dismiss(0);fs_exit();timer_test_reset(); }
static void strong_test_event(TimerMode mode) {
    strong_test_reset();settings.reminder_mode=2;start_timer(g_main_hwnd,0,mode);timer_advance_seconds(g_main_hwnd,0);pump(30);
}
static void strong_test_render(int white) {
    FullscreenView view;RECT bounds={0,0,800,600};fs_read_timer_view(&view);
    HDC desktop=GetDC(NULL),dc=CreateCompatibleDC(desktop);HBITMAP bitmap=CreateCompatibleBitmap(desktop,800,600);
    HGDIOBJ previous=SelectObject(dc,bitmap);fs_draw_view_ex(dc,bounds,&view,1,1,white);
    CHECK(GetPixel(dc,0,0)==(white ? RGB(255,255,255) : RGB(0,0,0)),"strong renderer produces solid white/black background");
    int x,y,contrast=0;
    for(y=200;y<500;y+=5)for(x=150;x<650;x+=5)if(GetPixel(dc,x,y)==(white ? RGB(0,0,0) : RGB(255,255,255)))++contrast;
    CHECK(contrast>10,"time and mode remain readable in both strong reminder frames");
    SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);ReleaseDC(NULL,desktop);
}
static void test_strong_reminder(void) {
    TimerSettings saved=settings;FullscreenMonitors online;int i;
    strong_test_reset();test_silent_sound=1;test_one_shot_sound_count=0;
    CHECK(fs_get_monitors(&online) && online.count>0,"native monitor inventory exists");
    const char *json[]={"{\"show_completion_dialog\":0}","{\"show_completion_dialog\":1}","{\"reminder_mode\":2,\"show_completion_dialog\":0}","{\"reminder_mode\":1}","{\"reminder_mode\":0}","{\"reminder_mode\":99}"};
    int modes[]={0,1,2,1,0,1};
    for(i=0;i<6;++i){char fixture[256];sprintf(fixture,"{\"pomodoro_duration\":45,\"pomodoro_count\":0,%s",json[i]+1);write_test_json(g_settings_path,fixture);load_settings();CHECK(settings.reminder_mode==modes[i],"legacy popup migrates and explicit reminder mode takes precedence");}
    settings.enable_clock_sound=0;settings.enable_completion_sound=1;settings.enable_pomodoro_count=0;
    settings.enable_micro_break=1;settings.micro_break_interval_minutes=15;settings.micro_break_duration_minutes=1;
    settings.enable_overtime_count_up=1;
    choose_menu(ID_MENU_REMINDER_STRONG);CHECK(settings.reminder_mode==2 && timer_setting_locked(ID_MENU_SET_TOAST_COLLAPSE_SECONDS),"strong selection is exclusive and disables popup-only collapse input");
    load_settings();CHECK(settings.reminder_mode==2,"strong mode survives save and load");
    choose_menu(ID_MENU_REMINDER_OFF);CHECK(settings.reminder_mode==0,"off radio selection persists");
    choose_menu(ID_MENU_REMINDER_POPUP);CHECK(settings.reminder_mode==1 && !timer_setting_locked(ID_MENU_SET_TOAST_COLLAPSE_SECONDS),"popup radio enables collapse input");
    choose_menu(ID_MENU_REMINDER_OFF);start_timer(g_main_hwnd,0,TIMER_CUSTOM);timer_advance_seconds(g_main_hwnd,0);pump(30);
    CHECK(!sr.active && !g_hToastWnd,"off has no visual alert");
    int sound_before=test_one_shot_sound_count;strong_test_event(TIMER_CUSTOM);
    CHECK(sr.active && sr.count==online.count && !fs_active && !g_hToastWnd,"nonfullscreen strong alert covers every screen without selecting normal fullscreen or stacking toast");
    unsigned generation=timer_stage_generation;int seconds=overtime_seconds;
    strong_test_render(1);strong_test_render(0);
    CHECK(sr_white(),"strong reminder starts with white frame");sr.started=GetTickCount64()-1000;CHECK(!sr_white(),"black frame occurs after one second");
    sr_tick();sr_tick();CHECK(test_one_shot_sound_count==sound_before+1,"redrawing never repeats completion sound");
    SendMessageW(sr.items[0].window,WM_LBUTTONDOWN,0,0);CHECK(sr.active,"mouse down is consumed until release");
    SendMessageW(sr.items[0].window,WM_LBUTTONUP,0,0);
    CHECK(!sr.active && !fs_active && generation==timer_stage_generation && seconds==overtime_seconds,"left dismisses all temporary reminders without advancing timer");
    PostMessageW(g_main_hwnd,WM_TOAST_NOTIFY,TIMER_CUSTOM,generation);pump(30);CHECK(!sr.active && !g_hToastWnd,"dismissed same-generation alert cannot reappear");
    strong_test_reset();settings.reminder_mode=2;fs_begin();fs_add_screen(&online.items[0]);HWND original=fs_windows[0];
    start_timer(g_main_hwnd,0,TIMER_SHORT_BREAK);timer_advance_seconds(g_main_hwnd,0);pump(30);
    CHECK(sr.active && sr.count==1 && fs_count==1 && IsWindow(original),"existing partial fullscreen limits strong alert to original identity");
    SendMessageW(sr.items[0].window,WM_MBUTTONUP,0,0);
    CHECK(!sr.active && fs_count==1 && fs_windows[0]==original,"middle dismiss leaves original fullscreen intact");
    strong_test_reset();settings.reminder_mode=2;settings.enable_overtime_count_up=0;strong_test_event(TIMER_LONG_BREAK);
    CHECK(sr.active && !is_running,"alert refresh works even when completion stops the timer");
    sr_tick();SendMessageW(sr.items[0].window,WM_KEYDOWN,VK_ESCAPE,0);CHECK(!sr.active,"Esc dismisses stopped-timer alert");
    settings.enable_overtime_count_up=1;strong_test_reset();start_timer(g_main_hwnd,45,TIMER_SHORT_POMODORO);timer_advance_seconds(g_main_hwnd,900);pump(30);
    CHECK(sr.active && micro.phase==MICRO_WAIT_START && micro.frozen_seconds==1800,"micro due shows reminder while original focus stays frozen");
    SendMessageW(sr.items[0].window,WM_RBUTTONDOWN,0,0);SendMessageW(sr.items[0].window,WM_RBUTTONUP,0,0);
    CHECK(!sr.active && current_timer_mode==TIMER_MICRO_BREAK && remaining_seconds==60 && micro.frozen_seconds==1800,"right click advances once to micro break");
    timer_advance_seconds(g_main_hwnd,60);pump(30);CHECK(sr.active && micro.phase==MICRO_WAIT_RESUME,"micro completion creates its own strong reminder");
    SendMessageW(sr.items[0].window,WM_RBUTTONUP,0,0);CHECK(!sr.active && current_timer_mode==TIMER_SHORT_POMODORO && remaining_seconds==1800,"right click resumes frozen focus exactly once");
    strong_test_event(TIMER_CUSTOM);choose_menu(ID_MENU_REMINDER_POPUP);CHECK(!sr.active && !g_hToastWnd,"switching reminder choice ends current strong alert without supplementary toast");
    strong_test_event(TIMER_CUSTOM);start_mode_from_menu(g_main_hwnd,TIMER_COUNT_UP);CHECK(!sr.active,"new session synchronously removes stale strong alert");
    int count=(settings.short_pomodoro_duration*60);settings.enable_pomodoro_count=1;session_rules.count=1;settings.enable_completion_sound=0;timer_advance_seconds(g_main_hwnd,count);pump(30);CHECK(!sr.active,"count-up statistics threshold never shows strong reminder");
    strong_test_reset();settings.enable_pomodoro_count=0;settings.enable_completion_sound=1;settings.reminder_mode=2;
    fs_begin();fs_add_screen(&online.items[0]);original=fs_windows[0];
    FullscreenColorDraft draft={0};draft.index=0;draft.color=settings.fullscreen_colors[0];draft.scale=100;wcscpy(draft.font,L"Segoe UI");
    HWND editor=CreateDialogParamW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(IDD_FULLSCREEN_COLORS),g_main_hwnd,FullscreenColorsDlgProc,(LPARAM)&draft);ShowWindow(editor,SW_SHOW);SetActiveWindow(editor);SetFocus(GetDlgItem(editor,IDC_FS_EDIT_FIRST));
    FullscreenView preview;fs_read_timer_view(&preview);wcscpy(preview.time,L"99:59");fs_start_preview(editor,IDC_FS_EDIT_FIRST,&preview);
    start_timer(g_main_hwnd,0,TIMER_CUSTOM);timer_advance_seconds(g_main_hwnd,0);pump(30);
    CHECK(sr.active && sr.count==1 && fs_preview_active && fs_count==online.count,"preview is covered immediately based on pre-preview formal selection");
    FullscreenView actual;fs_read_timer_view(&actual);CHECK(wcscmp(actual.time,preview.time)!=0,"strong alert uses actual timer rather than draft preview");
    SendMessageW(sr.items[0].window,WM_LBUTTONUP,0,0);CHECK(!sr.active && fs_preview_active && fs_count==online.count,"dismissing reminder returns intact original preview");
    fs_end_preview();CHECK(IsWindowVisible(editor) && fs_count==1 && fs_windows[0]==original,"preview exits back to original editor and normal selection");DestroyWindow(editor);
    strong_test_event(TIMER_CUSTOM);choose_menu(ID_MENU_REMINDER_OFF);CHECK(!sr.active && !g_hToastWnd,"off stops strong overlay without advancing");
    strong_test_reset();settings.reminder_mode=1;start_timer(g_main_hwnd,0,TIMER_CUSTOM);timer_advance_seconds(g_main_hwnd,0);pump(30);
    CHECK(g_hToastWnd && !sr.active,"popup radio still uses existing native completion notification");
    strong_test_reset();settings.reminder_mode=2;settings.enable_completion_sound=0;
    TimerMode sources[]={TIMER_LONG_POMODORO,TIMER_SHORT_POMODORO,TIMER_SHORT_BREAK,TIMER_LONG_BREAK,TIMER_CUSTOM};
    for(i=0;i<5;++i){strong_test_event(sources[i]);CHECK(sr.active,"every countdown type triggers strong reminder");sr_dismiss(0);}
    strong_test_reset();settings.reminder_mode=2;fs_show_all();size_t normal_count=fs_count;
    start_timer(g_main_hwnd,0,TIMER_CUSTOM);timer_advance_seconds(g_main_hwnd,0);pump(30);
    CHECK(sr.active && sr.count==normal_count && fs_count==normal_count,"all-screen normal fullscreen is preserved by independent overlay");
    FullscreenMonitors reduced=online;if(reduced.count>1){memmove(reduced.items,reduced.items+1,(reduced.count-1)*sizeof(reduced.items[0]));--reduced.count;}
    sr_reconcile_monitors(&reduced);CHECK(sr.count==reduced.count && fs_count==normal_count,"simulated offline reminder screen is removed without mutating normal selection");
    sr_reconcile_monitors(&online);CHECK(sr.count==reduced.count,"reconnected screens are not added mid-reminder");sr_dismiss(0);
    strong_test_reset();settings.reminder_mode=2;test_fail_strong_window=1;
    start_timer(g_main_hwnd,0,TIMER_CUSTOM);timer_advance_seconds(g_main_hwnd,0);pump(30);
    CHECK(!sr.active && !sr.count && g_hToastWnd,"window creation failure tears down safely and falls back to ordinary popup");test_fail_strong_window=0;
    strong_test_reset();settings.reminder_mode=2;
    HWND owner=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"owned foreground",WS_POPUP|WS_VISIBLE,0,0,200,100,g_main_hwnd,NULL,GetModuleHandleW(NULL),NULL);
    HWND edit=CreateWindowW(L"EDIT",L"",WS_CHILD|WS_VISIBLE,0,0,100,25,owner,NULL,GetModuleHandleW(NULL),NULL);SetActiveWindow(owner);SetFocus(edit);
    start_timer(g_main_hwnd,0,TIMER_CUSTOM);timer_advance_seconds(g_main_hwnd,0);pump(30);
    CHECK(sr.active,"temporary overlay covers owned foreground without modifying it");SendMessageW(sr.items[0].window,WM_LBUTTONUP,0,0);
    CHECK(GetFocus()==edit && IsWindow(owner),"closing temporary reminder restores original input focus");DestroyWindow(owner);
    strong_test_reset();settings=saved;save_settings();test_silent_sound=0;
    printf("STRONG_REMINDER_TEST: %s (%d failures)\n",failures ? "FAIL" : "PASS",failures);
}
