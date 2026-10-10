/* v3.0.5 focused contracts: independent timers, event notification, parser, clock and focus. */
static const wchar_t *reliability_input_text;
static int reliability_invalid_stayed,reliability_editor_case;
static HWND reliability_new_work;
static BOOL CALLBACK reliability_input_window(HWND dialog,LPARAM unused) {
    (void)unused;if(!GetDlgItem(dialog,IDC_INPUT_EDIT))return TRUE;
    SetDlgItemTextW(dialog,IDC_INPUT_EDIT,reliability_input_text);SendMessageW(dialog,WM_COMMAND,IDOK,0);
    reliability_invalid_stayed=IsWindow(dialog);
    if(IsWindow(dialog)){SetDlgItemTextW(dialog,IDC_INPUT_EDIT,L"0");SendMessageW(dialog,WM_COMMAND,IDOK,0);}
    return FALSE;
}
static VOID CALLBACK reliability_input_tick(HWND w,UINT m,UINT_PTR id,DWORD t) {
    (void)w;(void)m;(void)t;KillTimer(NULL,id);EnumThreadWindows(GetCurrentThreadId(),reliability_input_window,0);
}
static BOOL CALLBACK reliability_editor_window(HWND dialog,LPARAM unused) {
    (void)unused;if(!GetDlgItem(dialog,IDC_FS_EDIT_FIRST)&&!GetDlgItem(dialog,IDC_FS_SIGNATURE))return TRUE;
    if(reliability_editor_case==1){SetActiveWindow(reliability_new_work);SetFocus(reliability_new_work);}
    if(reliability_editor_case==2){
        SetActiveWindow(reliability_new_work);SetFocus(reliability_new_work);
        SendMessageW(dialog,WM_MOUSEACTIVATE,(WPARAM)dialog,MAKELPARAM(HTCLIENT,WM_LBUTTONDOWN));
        SetActiveWindow(dialog);SetFocus(dialog);
    }
    if(reliability_editor_case==3){FullscreenView view;fs_read_timer_view(&view);int field=GetDlgItem(dialog,IDC_FS_SIGNATURE)?IDC_FS_SIGNATURE:IDC_FS_EDIT_FIRST;fs_start_preview(dialog,field,&view);fs_end_preview();CHECK(GetFocus()==GetDlgItem(dialog,field),"explicit preview return still restores editor input focus");}
    SendMessageW(dialog,WM_COMMAND,IDCANCEL,0);return FALSE;
}
static VOID CALLBACK reliability_editor_tick(HWND w,UINT m,UINT_PTR id,DWORD t) {
    (void)w;(void)m;(void)t;KillTimer(NULL,id);EnumThreadWindows(GetCurrentThreadId(),reliability_editor_window,0);
}
static void reliability_event(TimerMode source,int kind) {
    consistency_reset();settings.reminder_mode=kind;start_timer(g_main_hwnd,0,source);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(20);
}
static void reliability_icon_sheet(void) {
    test_tray_size_override=32;
    HDC screen=GetDC(NULL),dc=CreateCompatibleDC(screen);HBITMAP bitmap=CreateCompatibleBitmap(screen,400,130);HGDIOBJ previous=SelectObject(dc,bitmap);
    RECT r={0,0,400,130};FillRect(dc,&r,(HBRUSH)GetStockObject(BLACK_BRUSH));SetTextColor(dc,RGB(255,255,255));SetBkMode(dc,TRANSPARENT);
    const wchar_t *samples[]={L"25",L"Ⅱ25",L"+25",L"Ⅱ",L"Ⅱ720",L"+720"};int i,base=0;
    settings.enable_pomodoro_count=1;
    for(i=0;i<6;++i){HICON icon=create_tray_icon(samples[i],3);if(i==0)base=test_tray_digit_height;
        if(i==1||i==2)CHECK(test_tray_digit_height>=MulDiv(base,3,4) && test_tray_prefix_height<test_tray_digit_height,"small separated prefix retains readable numeral body; complete pixels are checked by VisualOnly");
        if(i!=3)CHECK(test_tray_digit_height>=14,"single double and three-digit numerals retain readable size");
        TextOutW(dc,i*65+2,2,samples[i],(int)wcslen(samples[i]));DrawIconEx(dc,i*65+2,28,icon,32,32,0,NULL,DI_NORMAL);DrawIconEx(dc,i*65+2,75,icon,16,16,0,NULL,DI_NORMAL);DrawIconEx(dc,i*65+28,75,icon,24,24,0,NULL,DI_NORMAL);
    }
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=400;info.bmiHeader.biHeight=-130;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    size_t bytes=400*130*4;void *pixels=malloc(bytes);SelectObject(dc,previous);
    if(pixels && GetDIBits(dc,bitmap,0,130,pixels,&info,DIB_RGB_COLORS)){
        BITMAPFILEHEADER header={0};header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader);header.bfSize=(DWORD)(header.bfOffBits+bytes);
        wchar_t path[MAX_PATH];swprintf(path,MAX_PATH,L"%ls\\tray-prefix-samples.bmp",g_data_dir);FILE *file=_wfopen(path,L"wb");if(file){fwrite(&header,sizeof(header),1,file);fwrite(&info.bmiHeader,sizeof(info.bmiHeader),1,file);fwrite(pixels,bytes,1,file);fclose(file);}
    }
    free(pixels);DeleteObject(bitmap);DeleteDC(dc);ReleaseDC(NULL,screen);settings.enable_pomodoro_count=0;test_tray_size_override=0;
}
static void test_reliability(void) {
    TimerSettings saved=settings;consistency_reset();test_silent_sound=test_io_active=test_notify_stub=1;
    settings.enable_clock_sound=settings.enable_completion_sound=settings.enable_pomodoro_count=settings.enable_micro_break=0;settings.enable_overtime_count_up=1;settings.reminder_mode=0;
    test_timer_owner=g_main_hwnd;test_timer_tracking=1;test_main_timers=0;
    UINT_PTR ids[]={ID_MAIN_DAY_SYNC_TIMER,ID_TRAY_RETRY_TIMER,ID_FS_REFRESH,ID_FS_LAYOUT,ID_TIMER_CLOCK,ID_STRONG_REMINDER_TIMER};int i,j;
    for(i=0;i<6;++i)for(j=0;j<i;++j)CHECK(ids[i]!=ids[j],"all main-window timer IDs are independent");
    test_notify_failures=2;refresh_timer_icon_by_state(g_main_hwnd);CHECK(tray_retry_active && (test_main_timers&(1u<<(ID_TRAY_RETRY_TIMER-4001))),"failed tray publication schedules independent retry");
    fs_show_all();fs_exit();CHECK(tray_retry_active && (test_main_timers&(1u<<(ID_TRAY_RETRY_TIMER-4001))),"entering/exiting fullscreen cannot cancel tray retry");
    int calls=test_notify_calls;SendMessageW(g_main_hwnd,WM_TIMER,ID_TRAY_RETRY_TIMER,0);CHECK(test_notify_calls>calls&&!tray_retry_active,"retry branch publishes tray and stops after success");
    UINT taskbar_before=g_taskbar_created_message;g_taskbar_created_message=RegisterWindowMessageW(L"TaskbarCreated");test_notify_failures=2;SendMessageW(g_main_hwnd,g_taskbar_created_message,0,0);CHECK(tray_retry_active,"Explorer restart publication failure also retries");SendMessageW(g_main_hwnd,WM_TIMER,ID_TRAY_RETRY_TIMER,0);CHECK(!tray_retry_active,"Explorer retry recovers");g_taskbar_created_message=taskbar_before;
    TimerMode sources[]={TIMER_LONG_POMODORO,TIMER_SHORT_POMODORO,TIMER_SHORT_BREAK,TIMER_LONG_BREAK,TIMER_CUSTOM};
    for(i=0;i<5;++i)for(j=0;j<2;++j){reliability_event(sources[i],0);TimerMode next=timer_next_mode(sources[i]);if(j)choose_menu(ID_MENU_PAUSE_RESUME);CHECK(!wcscmp(timer_ui_primary_label(),L"开始"),"all running and paused overtime has start primary");if(j)choose_menu(ID_MENU_START_CURRENT);choose_menu(ID_MENU_START_CURRENT);consistency_clock();CHECK(is_running&&!is_overtime&&current_timer_mode==next,"one primary directly starts next ordinary mode from any overtime source");}
    reliability_event(TIMER_CUSTOM,0);choose_menu(ID_MENU_REMINDER_POPUP);CHECK(!g_hToastWnd,"off to popup never replays an old due event");
    reliability_event(TIMER_CUSTOM,1);CHECK(g_hToastWnd!=NULL,"fresh popup event appears");timer_primary_action(g_main_hwnd);consistency_clock();timer_apply_live_preferences();CHECK(!g_hToastWnd,"live preferences after overtime advance never create old popup");
    choose_menu(ID_MENU_PAUSE_RESUME);load_settings_preserving_runtime();CHECK(!g_hToastWnd,"loading preferences preserves notification lifecycle");
    consistency_reset();settings.reminder_mode=0;settings.enable_overtime_count_up=1;settings.enable_micro_break=1;settings.micro_break_interval_minutes=15;settings.micro_break_duration_minutes=1;
    start_timer(g_main_hwnd,40,TIMER_SHORT_POMODORO);consistency_clock();timer_advance_seconds(g_main_hwnd,900);choose_menu(ID_MENU_PAUSE_RESUME);timer_primary_action(g_main_hwnd);timer_primary_action(g_main_hwnd);consistency_clock();CHECK(current_timer_mode==TIMER_MICRO_BREAK&&remaining_seconds==60&&micro.frozen_seconds==1500,"paused pre-micro overtime starts micro directly while preserving parent");
    timer_advance_seconds(g_main_hwnd,60);timer_primary_action(g_main_hwnd);consistency_clock();CHECK(current_timer_mode==TIMER_SHORT_POMODORO&&remaining_seconds==1500,"post-micro overtime resumes focus directly");
    reliability_event(TIMER_CUSTOM,1);wchar_t word[32];GetWindowTextW(g_hToastButton,word,32);CHECK(!wcscmp(word,L"开始"),"overtime popup start wording matches shared action");SendMessageW(g_hToastWnd,WM_COMMAND,MAKEWPARAM(ID_TOAST_ACTION,BN_CLICKED),0);consistency_clock();CHECK(is_running&&!is_overtime&&current_timer_mode==TIMER_CUSTOM,"popup directly starts next mode");
    reliability_event(TIMER_CUSTOM,0);fs_show_all();size_t screens=fs_count;SendMessageW(fs_windows[0],WM_RBUTTONUP,0,0);consistency_clock();CHECK(is_running&&!is_overtime&&fs_count==screens,"normal fullscreen right action advances without leaving fullscreen");
    reliability_event(TIMER_CUSTOM,2);SendMessageW(sr.items[0].window,WM_RBUTTONUP,0,0);consistency_clock();CHECK(is_running&&!is_overtime&&!sr.active,"strong right action directly starts next mode");
    reliability_event(TIMER_CUSTOM,0);SendMessageW(g_main_hwnd,WM_USER+1,0,WM_LBUTTONUP);consistency_clock();CHECK(is_running&&!is_overtime,"tray action directly starts next mode");
    int result=777;const wchar_t *bad[]={L"",L"   ",L"12x",L"1.5",L"2147483648",L"999999999999999999999999999999"};
    for(i=0;i<6;++i){CHECK(!parse_input_integer(bad[i],0,INT_MAX,&result)&&result==777,"invalid empty mixed or overflowing integer cannot mutate result");reliability_input_text=bad[i];reliability_invalid_stayed=0;int out=5;SetTimer(NULL,0,30,reliability_input_tick);CHECK(PromptForInteger(g_main_hwnd,L"input test",L"value",5,0,INT_MAX,&out)&&out==0&&reliability_invalid_stayed,"real integer dialog rejects invalid text then accepts explicit zero");}
    CHECK(parse_input_integer(L" 25 ",1,INT_MAX,&result)&&result==25,"trimmed valid decimal input remains accepted");
    consistency_reset();settings.reminder_mode=0;settings.enable_micro_break=0;start_timer(g_main_hwnd,1,TIMER_CUSTOM);
    for(i=0;i<6;++i){pump(850);choose_menu(ID_MENU_PAUSE_RESUME);choose_menu(ID_MENU_PAUSE_RESUME);}
    CHECK(remaining_seconds<=55 && remaining_seconds>=53,"six real 850ms active slices accumulate rather than lose seconds on pause");
    choose_menu(ID_MENU_PAUSE_RESUME);int paused_value=remaining_seconds;unsigned fraction=timer_remainder_ms;pump(250);choose_menu(ID_MENU_PAUSE_RESUME);CHECK(remaining_seconds==paused_value && (GetTickCount64()-timer_last_tick)>=fraction,"resume excludes pause duration and retains fractional remainder");
    consistency_reset();start_timer(g_main_hwnd,0,TIMER_COUNT_UP);timer_last_tick=GetTickCount64()-650;choose_menu(ID_MENU_PAUSE_RESUME);CHECK(timer_remainder_ms>=650,"count-up retains fractional active time");choose_menu(ID_MENU_PAUSE_RESUME);timer_last_tick=GetTickCount64()-1100;timer_sync_clock(g_main_hwnd);CHECK(remaining_seconds>=1,"count-up accumulates integer seconds from fractional timeline");
    choose_menu(ID_MENU_PAUSE_RESUME);consistency_input(2);CHECK(!timer_remainder_ms&&remaining_seconds==120,"explicit timer correction resets fractional remainder");
    timer_remainder_ms=700;remaining_seconds=120;choose_menu(ID_MENU_PLUS_5_MIN);CHECK(timer_remainder_ms==700,"unclamped step preserves fractional progress");
    consistency_reset();start_timer(g_main_hwnd,1,TIMER_CUSTOM);consistency_clock();timer_remainder_ms=700;remaining_seconds=60;choose_menu(ID_MENU_PLUS_5_MIN);CHECK(!timer_remainder_ms&&remaining_seconds==60,"clamped step sets exact boundary without old remainder");
    timer_remainder_ms=700;start_timer(g_main_hwnd,2,TIMER_CUSTOM);CHECK(!timer_remainder_ms,"new timing segment does not inherit previous fractional time");
    consistency_reset();reliability_icon_sheet();CHECK(!wcscmp(fs_color_names[3],L"自定义计时")&&!wcscmp(fs_color_names[4],L"超时正计时"),"preview categories have full names");choose_menu(0);
    HWND first=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"old work",WS_POPUP|WS_VISIBLE,0,0,120,90,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HWND other=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"new work",WS_POPUP|WS_VISIBLE,160,0,120,90,NULL,NULL,GetModuleHandleW(NULL),NULL);reliability_new_work=other;
    SetActiveWindow(first);SetFocus(first);settings.reminder_mode=2;start_timer(g_main_hwnd,0,TIMER_CUSTOM);consistency_clock();timer_advance_seconds(g_main_hwnd,0);pump(20);
    HWND alert=sr.items[0].window;SetActiveWindow(other);SetFocus(other);SendMessageW(alert,WM_MOUSEACTIVATE,(WPARAM)alert,MAKELPARAM(HTCLIENT,WM_LBUTTONDOWN));SetActiveWindow(alert);SetFocus(alert);SendMessageW(alert,WM_LBUTTONDOWN,0,0);SendMessageW(alert,WM_LBUTTONUP,0,0);CHECK(GetFocus()==other,"mouse activation before dismiss restores latest work instead of stale original");
    consistency_reset();settings.reminder_mode=0;fs_show_all();
    for(i=0;i<2;++i)for(j=0;j<4;++j){SetActiveWindow(first);SetFocus(first);reliability_editor_case=j;SetTimer(NULL,0,50,reliability_editor_tick);if(i)fs_show_signature(g_main_hwnd);else fs_show_color(g_main_hwnd,0);CHECK(GetFocus()==(j==1||j==2 ? other : first),"editor close preserves original/latest work and preview return does not overwrite it with first fullscreen");}
    DestroyWindow(other);DestroyWindow(first);
    consistency_reset();settings=saved;save_settings();test_silent_sound=test_io_active=0;test_timer_tracking=0;test_notify_stub=0;
    printf("RELIABILITY_TEST: %s (%d failures)\n",failures ? "FAIL" : "PASS",failures);
}
