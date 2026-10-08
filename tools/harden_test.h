/* v3.0.9: only the five reviewed failure paths and their direct dependents. */
static wchar_t harden_root[MAX_PATH];
static int harden_inner_cancel,harden_outer_close;
static void harden_case(const wchar_t *name) {
    consistency_reset();swprintf(g_data_dir,MAX_PATH,L"%ls\\%ls",harden_root,name);CreateDirectoryW(g_data_dir,NULL);build_data_file_paths();
    load_settings();settings.reminder_mode=0;settings.enable_micro_break=settings.enable_completion_sound=settings.enable_clock_sound=0;settings.enable_overtime_count_up=1;
    pomodoro_count=0;stats_needs_refresh=0;stats_read_warning=0;config_warning=0;test_notify_stub=test_io_active=test_silent_sound=1;
    data_test_fail_phase=data_test_fail_restore=test_fail_stats_commit=test_stats_read_failure=0;wcscpy(test_io_root,harden_root);
}
static void harden_today_file(int count) {
    time_t now=time(NULL);struct tm *date=localtime(&now);char line[128];sprintf(line,"%04d-%02d-%02d,12:00:00,%d\n",date->tm_year+1900,date->tm_mon+1,date->tm_mday,count);
    write_test_json(g_log_path,line);pomodoro_count=count;refresh_today_count_if_day_changed(g_main_hwnd,1);
}
static HWND harden_input(const wchar_t *title) {
    for(HWND w=GetTopWindow(NULL);w;w=GetWindow(w,GW_HWNDNEXT)){DWORD pid=0;GetWindowThreadProcessId(w,&pid);if(pid!=GetCurrentProcessId())continue;
        wchar_t text[128];GetWindowTextW(w,text,128);if(GetDlgItem(w,IDC_INPUT_EDIT)&&!wcscmp(text,title))return w;}
    return NULL;
}
static VOID CALLBACK harden_inner(HWND w,UINT m,UINT_PTR id,DWORD t) {
    (void)w;(void)m;(void)t;HWND dialog=harden_input(L"修改今日番茄数");if(!dialog)return;KillTimer(NULL,id);
    SetDlgItemTextW(dialog,IDC_INPUT_EDIT,L"0");SendMessageW(dialog,WM_COMMAND,harden_inner_cancel?IDCANCEL:IDOK,0);
}
static VOID CALLBACK harden_outer(HWND w,UINT m,UINT_PTR id,DWORD t) {
    (void)w;(void)m;(void)t;HWND outer=harden_input(L"长番茄钟时长");if(!outer)return;KillTimer(NULL,id);
    SetTimer(NULL,0,30,harden_inner);choose_menu(ID_MENU_SET_COUNT);
    IntegerInputContext *input=(IntegerInputContext*)GetWindowLongPtrW(outer,DWLP_USER);
    CHECK(input&&input->minimum==1&&input->maximum==720,"outer dialog keeps its own original range after nested dialog");
    SetDlgItemTextW(outer,IDC_INPUT_EDIT,L"1000");SendMessageW(outer,WM_COMMAND,IDOK,0);
    CHECK(IsWindow(outer),"outer rejects 1000 while the inner allows 9999");
    if(harden_outer_close)SendMessageW(outer,WM_CLOSE,0,0);else {SetDlgItemTextW(outer,IDC_INPUT_EDIT,L"700");SendMessageW(outer,WM_COMMAND,IDOK,0);}
}
static void harden_inputs(void) {
    for(int i=0;i<3;++i){wchar_t name[32];swprintf(name,32,L"input-%d",i);harden_case(name);harden_inner_cancel=i==1;harden_outer_close=i==2;
        SetTimer(NULL,0,30,harden_outer);choose_menu(ID_MENU_SET_POMODORO_DURATION);
        CHECK(settings.long_pomodoro_duration==(i==2?90:700),"nested acceptance cancellation and outer close preserve each result");}
}
static void harden_statistics(void) {
    harden_case(L"stat-lock");harden_today_file(5);
    HANDLE lock=CreateFileW(g_log_path,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(lock!=INVALID_HANDLE_VALUE,"real read lock acquired");
    CHECK(!timer_correct_today_count(g_main_hwnd,3)&&pomodoro_count==5,"read failure refuses adjustment and retains valid count");
    CHECK(GetFileAttributesW(g_adjustments_path)==INVALID_FILE_ATTRIBUTES,"read failure cannot create adjustment file");CloseHandle(lock);
    CHECK(get_today_count_from_storage()==5,"unlock preserves original five");CHECK(timer_correct_today_count(g_main_hwnd,3)&&get_today_count_from_storage()==3,"retry after unlock applies exactly three");
    wchar_t month[MAX_PATH];swprintf(month,MAX_PATH,L"%ls\\pomodoro_log_2020-01.csv",g_data_dir);write_test_json(month,"2020-01-01,12:00:00,2\n");load_heatmap_data();
    lock=CreateFileW(month,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    CHECK(!timer_correct_today_count(g_main_hwnd,1)&&get_day_count_value(2020,1,1)==2,"partial archive failure cannot publish or compute a partial snapshot");CloseHandle(lock);
    CHECK(get_today_count_from_storage()==3,"partial archive failure leaves adjustment unchanged");
    harden_case(L"stat-readback");harden_today_file(5);start_timer(g_main_hwnd,0,TIMER_SHORT_POMODORO);consistency_clock();test_stats_read_failure=1;timer_advance_seconds(g_main_hwnd,0);
    CHECK(GetFileAttributesW(g_adjustments_path)==INVALID_FILE_ATTRIBUTES,"committed completion with failed readback never applies fallback");test_stats_read_failure=0;
    CHECK(get_today_count_from_storage()==6,"committed completion remains exactly once despite failed readback");timer_advance_seconds(g_main_hwnd,60);CHECK(get_today_count_from_storage()==6,"continued overtime never repeats committed completion");
    harden_case(L"stat-fallback");harden_today_file(5);lock=CreateFileW(g_log_path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    start_timer(g_main_hwnd,0,TIMER_SHORT_POMODORO);consistency_clock();timer_advance_seconds(g_main_hwnd,0);CloseHandle(lock);
    CHECK(get_today_count_from_storage()==6&&GetFileAttributesW(g_adjustments_path)!=INVALID_FILE_ATTRIBUTES,"readable baseline permits existing adjustment fallback exactly once");
    harden_case(L"stat-failed");harden_today_file(5);test_fail_stats_commit=1;start_timer(g_main_hwnd,0,TIMER_SHORT_POMODORO);consistency_clock();timer_advance_seconds(g_main_hwnd,0);test_fail_stats_commit=0;
    timer_advance_seconds(g_main_hwnd,100);CHECK(get_today_count_from_storage()==5,"failed primary and fallback never silently retry credit");
}
static void harden_config(void) {
    harden_case(L"config-recovery");write_test_json(g_settings_path,"broken");write_test_json(g_settings_tmp_path,"{\"pomodoro_duration\":42,\"pomodoro_count\":0,\"reminder_mode\":2}");repair_settings_recovery_chain();load_settings();
    char bytes[8192];CHECK(settings.long_pomodoro_duration==42&&config_read_valid(g_settings_bak_path,bytes,sizeof(bytes)),"temporary recovery creates a validated backup");
    write_test_json(g_settings_path,"broken again");repair_settings_recovery_chain();load_settings();CHECK(settings.long_pomodoro_duration==42&&settings.reminder_mode==2,"second corruption restores custom values");
    HANDLE lock=CreateFileW(g_settings_bak_path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    settings.long_pomodoro_duration=43;CHECK(save_settings(),"backup failure does not misreport committed main settings");
    CHECK(config_read_valid(g_settings_tmp_path,bytes,sizeof(bytes))&&extract_json_int(bytes,"\"long_pomodoro_duration\"",0)==43,"backup failure retains a complete pending recovery source");CloseHandle(lock);
    repair_settings_recovery_chain();CHECK(config_read_valid(g_settings_bak_path,bytes,sizeof(bytes))&&extract_json_int(bytes,"\"long_pomodoro_duration\"",0)==43,"unlocked backup completes recovery chain");
    lock=CreateFileW(g_settings_path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    settings.long_pomodoro_duration=44;CHECK(!save_settings(),"locked main cannot be overwritten through unsafe copy fallback");
    CHECK(config_read_valid(g_settings_path,bytes,sizeof(bytes))&&extract_json_int(bytes,"\"long_pomodoro_duration\"",0)==43,"failed save retains committed primary");CloseHandle(lock);
}
static void harden_import(void) {
    for(int mode=0;mode<5;++mode){wchar_t name[32],source[MAX_PATH],path[MAX_PATH];swprintf(name,32,L"import-%d",mode);harden_case(name);harden_today_file(5);save_settings();
        swprintf(source,MAX_PATH,L"%ls\\source-%d",harden_root,mode);CreateDirectoryW(source,NULL);
        swprintf(path,MAX_PATH,L"%ls\\pomodoro_log.csv",source);FILE *file=_wfopen(path,L"wb");time_t now=time(NULL);struct tm *date=localtime(&now);fprintf(file,"%04d-%02d-%02d,12:00:00,3\n",date->tm_year+1900,date->tm_mon+1,date->tm_mday);fclose(file);
        swprintf(path,MAX_PATH,L"%ls\\unrelated.txt",g_data_dir);write_test_json(path,"keep");wchar_t keep[MAX_PATH];swprintf(keep,MAX_PATH,L"%ls.keep",g_log_path);write_test_json(keep,"keep backup");
        wcscpy(test_io_folder,source);if(mode==1)data_test_fail_phase=1;if(mode==2)data_test_fail_phase=3;if(mode==3){data_test_fail_phase=3;data_test_fail_restore=1;}
        if(mode==4){swprintf(test_io_folder,MAX_PATH,L"%ls\\.",g_data_dir);}
        int result=import_data_from_folder(g_main_hwnd);
        if(mode==0)CHECK(result&&get_today_count_from_storage()==3,"normal staged import applies exact source count");
        else if(mode==3)CHECK(!result&&wcsstr(test_last_message,L"手动恢复"),"rollback failure is explicit and retains recovery evidence");
        else CHECK(!result&&get_today_count_from_storage()==5,"same directory preparation and commit failures preserve original data");
        CHECK(GetFileAttributesW(path)!=INVALID_FILE_ATTRIBUTES&&GetFileAttributesW(keep)!=INVALID_FILE_ATTRIBUTES,"import never deletes unrelated or keep files");
        data_test_fail_phase=data_test_fail_restore=0;
    }
    harden_case(L"identity-current");harden_today_file(5);swprintf(test_io_folder,MAX_PATH,L"%ls\\identity-alias",harden_root);
    CHECK(!import_data_from_folder(g_main_hwnd)&&get_today_count_from_storage()==5,"real directory junction to current dataset is rejected before mutation");
    harden_case(L"import-empty");harden_today_file(5);swprintf(test_io_folder,MAX_PATH,L"%ls\\empty",harden_root);CreateDirectoryW(test_io_folder,NULL);
    CHECK(!import_data_from_folder(g_main_hwnd)&&get_today_count_from_storage()==5,"empty source is never a successful replacement");
}
static void harden_taskbar(void) {
    harden_case(L"taskbar");wchar_t image[MAX_PATH],fixture[MAX_PATH],identity[MAX_PATH],event_name[80],command[1024];
    GetModuleFileNameW(NULL,image,MAX_PATH);wchar_t *slash=wcsrchr(image,L'\\');if(!slash){CHECK(0,"test exe directory available");return;}*slash=0;
    swprintf(fixture,MAX_PATH,L"%ls\\taskbar_fixture.exe",image);swprintf(identity,MAX_PATH,L"%ls\\fixture-identity.txt",g_data_dir);
    swprintf(event_name,80,L"Local\\PomodoroHarden-%lu",GetCurrentProcessId());HANDLE entered=CreateEventW(NULL,TRUE,FALSE,event_name);
    swprintf(command,1024,L"\"%ls\" \"%ls\" \"%ls\"",fixture,identity,event_name);STARTUPINFOW startup={sizeof(startup)};startup.dwFlags=STARTF_USESHOWWINDOW;startup.wShowWindow=SW_SHOWNOACTIVATE;wchar_t desktop[128];
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),NULL)){CHECK(0,"fixture desktop identity readable");CloseHandle(entered);return;}startup.lpDesktop=desktop;PROCESS_INFORMATION child={0};
    if(!CreateProcessW(fixture,command,NULL,NULL,FALSE,0,NULL,NULL,&startup,&child)){CHECK(0,"cross-process fixture starts");CloseHandle(entered);return;}
    HWND window=NULL;DWORD pid=0;ULONGLONG end=GetTickCount64()+2000;
    while(GetTickCount64()<end&&!window){FILE *f=_wfopen(identity,L"rb");if(f){unsigned long long value=0;unsigned long process=0;if(fscanf(f,"%llu %lu",&value,&process)==2){window=(HWND)(UINT_PTR)value;pid=process;}fclose(f);}if(!window)Sleep(10);}
    CHECK(window&&pid==child.dwProcessId,"fixture identity matches owned child");
    if(window){ResetEvent(entered);test_taskbar_trace=1;FsHiddenWindow target={0};target.window=window;target.process=pid;GetClassNameW(window,target.cls,256);
        AcquireSRWLockExclusive(&fs_taskbar_lock);fs_taskbar_targets[0]=target;fs_taskbar_target_count=1;++fs_taskbar_generation;ReleaseSRWLockExclusive(&fs_taskbar_lock);
        fs_start_taskbar_events();fs_taskbar_wake();CHECK(WaitForSingleObject(entered,1000)==WAIT_OBJECT_0,"other process enters delayed hide");
        ULONGLONG begin=GetTickCount64();fs_publish_taskbar_targets();fs_stop_taskbar_events();ULONGLONG delay=GetTickCount64()-begin;
        CHECK(delay<500&&!fs_taskbar_thread,"withdraw and stop do not wait for delayed external window");printf("HARDEN slow external window: main teardown=%llu ms\n",delay);
        /* Reenter before the old hide and restore requests have finished. */
        AcquireSRWLockExclusive(&fs_taskbar_lock);fs_taskbar_targets[0]=target;fs_taskbar_target_count=1;++fs_taskbar_generation;ReleaseSRWLockExclusive(&fs_taskbar_lock);
        fs_start_taskbar_events();fs_taskbar_wake();seal_pump_bounded(1800);CHECK(!IsWindowVisible(window),"quick reentry wins over previous delayed restore");
        fs_publish_taskbar_targets();fs_stop_taskbar_events();seal_pump_bounded(100);CHECK(IsWindowVisible(window),"last withdrawal restores after delayed requests");
        CHECK(!fs_taskbar_thread&&!fs_taskbar_target_count,"thread and active target cleanup complete");PostMessageW(window,WM_CLOSE,0,0);
    }
    if(WaitForSingleObject(child.hProcess,2000)!=WAIT_OBJECT_0){TerminateProcess(child.hProcess,2);CHECK(0,"owned fixture exited gracefully");}
    CloseHandle(child.hThread);CloseHandle(child.hProcess);CloseHandle(entered);test_taskbar_trace=0;
}
static void test_harden(void) {
    TimerSettings saved=settings;wcscpy(harden_root,g_data_dir);test_io_active=test_notify_stub=test_silent_sound=1;
    harden_inputs();harden_statistics();harden_config();harden_import();harden_taskbar();consistency_reset();
    settings=saved;wcscpy(g_data_dir,harden_root);build_data_file_paths();test_io_active=test_notify_stub=test_silent_sound=0;
    printf("HARDEN_TEST: %s (%d failures)\n",failures?"FAIL":"PASS",failures);
}
