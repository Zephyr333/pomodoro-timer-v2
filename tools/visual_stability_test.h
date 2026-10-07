/* Trace every native z-order operation, including intermediate states. */
static void test_visual_z_observe(void) {
    size_t i,j;HWND w;
    if(test_repair_z_tracking)test_repair_z_observe();
    if(!sr.active)return;
    for(i=0;i<fs_count;++i) for(j=0;j<sr.count;++j) {
        if(!IsWindowVisible(fs_windows[i]))continue;
        RECT a,b,overlap;GetWindowRect(fs_windows[i],&a);GetWindowRect(sr.items[j].window,&b);
        if(!IntersectRect(&overlap,&a,&b))continue;
        for(w=GetTopWindow(NULL);w;w=GetWindow(w,GW_HWNDNEXT)) {
            if(w==sr.items[j].window)break;
            if(w==fs_windows[i]){++test_visual_z_violations;break;}
        }
    }
}
static void visual_save_bitmap(HDC dc,HBITMAP bitmap,int width,int height,const wchar_t *name) {
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    size_t bytes=(size_t)width*height*4;void *pixels=malloc(bytes);BITMAPFILEHEADER header={0};
    if(pixels && GetDIBits(dc,bitmap,0,height,pixels,&info,DIB_RGB_COLORS)) {
        header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader);header.bfSize=(DWORD)(header.bfOffBits+bytes);
        wchar_t path[MAX_PATH];swprintf(path,MAX_PATH,L"%ls\\%ls",g_data_dir,name);FILE *file=_wfopen(path,L"wb");
        if(file){fwrite(&header,sizeof(header),1,file);fwrite(&info.bmiHeader,sizeof(info.bmiHeader),1,file);fwrite(pixels,bytes,1,file);fclose(file);}
    }
    free(pixels);
}
static void repair_icon_matrix(void);
static void visual_icons(void) { repair_icon_matrix(); }
static void visual_check_phase(void) {
    for(size_t i=0;i<sr.count;++i){InvalidateRect(sr.items[i].window,NULL,FALSE);UpdateWindow(sr.items[i].window);
        HDC dc=GetDC(sr.items[i].window);COLORREF color=GetPixel(dc,2,2);ReleaseDC(sr.items[i].window,dc);
        CHECK(color==(sr.last_white?RGB(255,255,255):RGB(0,0,0)),"all reminder windows paint the same committed phase");}
}
static int visual_live_desktop;
static void visual_sample_desktop(void) {
    if(!visual_live_desktop)return;
    unsigned white_samples=0,black_samples=0,unexpected=0;ULONGLONG end=GetTickCount64()+6000;
    while(GetTickCount64()<end) {
        pump(10);
        /* Give DWM a presentation interval at each intended phase boundary. */
        ULONGLONG position=(GetTickCount64()-sr.started)%1000;if(position<180||position>820)continue;
        HANDLE context=fs_enter_dpi();HDC screen=GetDC(NULL);
        for(size_t i=0;i<sr.count;++i) {
            RECT r=sr.items[i].monitor.info.rcMonitor;
            COLORREF pixel=GetPixel(screen,r.left+2,r.top+2);
            if(sr.last_white)++white_samples;else ++black_samples;
            if(pixel!=(sr.last_white?RGB(255,255,255):RGB(0,0,0)))++unexpected;
        }
        ReleaseDC(NULL,screen);fs_leave_dpi(context);
    }
    printf("Visible desktop samples: white %u black %u unexpected %u fs=%zu sr=%zu\n",white_samples,black_samples,unexpected,fs_count,sr.count);
    CHECK(white_samples>0&&black_samples>0&&unexpected==0,"visible desktop has no unintended background during settled phase");
}
static void test_visual_stability(void) {
    TimerSettings saved=settings;FullscreenMonitors online;size_t n;HWND original_work=GetForegroundWindow();
    if(visual_live_desktop){wchar_t station[80]={0},desktop[80]={0},input[80]={0};DWORD bytes,session;
        GetUserObjectInformationW(GetProcessWindowStation(),UOI_NAME,station,sizeof(station),&bytes);
        GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&bytes);
        HDESK active=OpenInputDesktop(0,FALSE,DESKTOP_READOBJECTS);
        if(active){GetUserObjectInformationW(active,UOI_NAME,input,sizeof(input),&bytes);CloseDesktop(active);}
        ProcessIdToSessionId(GetCurrentProcessId(),&session);
        printf("Live station=%ls desktop=%ls input=%ls session=%lu active-console=%lu\n",station,desktop,input,session,WTSGetActiveConsoleSessionId());
        CHECK(!_wcsicmp(station,L"WinSta0")&&!wcscmp(desktop,input)&&session==WTSGetActiveConsoleSessionId(),"live probe runs on active input desktop, not an invisible station");
    }
    strong_test_reset();test_silent_sound=1;settings.enable_completion_sound=0;
    CHECK(fs_get_monitors(&online)&&online.count>0,"screen inventory for visual tests");
    for(n=1;n<=online.count;++n) {
        strong_test_reset();settings.reminder_mode=2;settings.enable_completion_sound=0;
        fs_begin();for(size_t i=0;i<n;++i)fs_add_screen(&online.items[i]);
        start_timer(g_main_hwnd,0,TIMER_CUSTOM);timer_advance_seconds(g_main_hwnd,0);pump(30);
        CHECK(sr.active&&sr.count==n,"strong reminder keeps selected screen scope");
        test_visual_z_violations=0;test_visual_z_tracking=1;
        for(int i=0;i<15;++i){fs_enforce_topmost();sr_tick();}
        HWND work=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"test work",WS_POPUP|WS_VISIBLE,0,0,80,60,NULL,NULL,GetModuleHandleW(NULL),NULL);
        for(int i=0;i<8;++i){SetActiveWindow(work);SendMessageW(g_main_hwnd,WM_FS_MAINTAIN_LAYER,1,0);sr_tick();}
        DestroyWindow(work);
        fs_exit();fs_begin();for(size_t i=0;i<n;++i)fs_add_screen(&online.items[i]);
        fs_focus_window(fs_windows[0]);
        CHECK(sr.active&&sr.count==n,"recreating and focusing normal fullscreen preserves active reminder");
        KillTimer(g_main_hwnd,ID_STRONG_REMINDER_TIMER);
        for(int i=0;i<4;++i){sr.started=GetTickCount64()-(ULONGLONG)i*1000;sr_tick();visual_check_phase();}
        sr.last_white=1;sr.started=GetTickCount64()-1000;visual_check_phase(); /* Paint must not independently flip to black. */
        sr_tick();CHECK(sr.last_white==0,"next refresh commits black phase exactly once");visual_check_phase();
        SetTimer(g_main_hwnd,ID_STRONG_REMINDER_TIMER,100,NULL);if(visual_live_desktop)visual_sample_desktop();else pump(2200);
        test_visual_z_tracking=0;
        printf("Covered screens %zu: intermediate layer violations %d\n",n,test_visual_z_violations);
        CHECK(test_visual_z_violations==0,"normal fullscreen never rises above white reminder, including intermediate operations");
        sr_dismiss(0);fs_enforce_topmost();printf("After dismissal fs=%zu expected=%zu\n",fs_count,n);CHECK(fs_active&&fs_count==n,"normal fullscreen survives reminder dismissal");
    }
    strong_test_reset();visual_icons();settings=saved;test_silent_sound=0;
    if(visual_live_desktop&&IsWindow(original_work))SetForegroundWindow(original_work);
    printf("VISUAL_STABILITY_TEST: %s (%d failures)\n",failures?"FAIL":"PASS",failures);
}
