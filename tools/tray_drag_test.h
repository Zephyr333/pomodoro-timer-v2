/* Included by fullscreen_test.c: synthetic gesture regression on the isolated
   desktop. These tests do not claim to exercise Explorer/DisplayFusion input. */
static HWND td_test_window(const wchar_t *cls, HWND parent, RECT rect) {
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = DefWindowProcW; wc.hInstance = GetModuleHandleW(NULL); wc.lpszClassName = cls;
    RegisterClassW(&wc);
    return CreateWindowExW(WS_EX_TOOLWINDOW, cls, L"Tray test", (parent ? WS_CHILD : WS_POPUP) | WS_VISIBLE,
        rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, parent, NULL, wc.hInstance, NULL);
}
static void td_test_down(HWND source, POINT point) {
    EnterCriticalSection(&td.lock);
    { DWORD serial = td.gesture.serial + 1;
      td.previous = td.gesture;
      memset(&td.gesture, 0, sizeof(td.gesture)); td.gesture.serial = serial;
      td.gesture.source = source; td.gesture.origin = td.gesture.point = point;
      td.gesture.pressed = 1; td.gesture.started = GetTickCount(); }
    LeaveCriticalSection(&td.lock);
    td_input(td.gesture.serial);
    SendMessageW(g_main_hwnd, WM_USER + 1, nid.uID, WM_LBUTTONDOWN);
}
static void td_test_move_during_query(void) {
    EnterCriticalSection(&td.lock);
    td.gesture.moved = 1; td.gesture.point.x += 100;
    LeaveCriticalSection(&td.lock);
}
static void td_test_up(POINT point, int moved, int cancelled) {
    EnterCriticalSection(&td.lock);
    td.gesture.point = point; td.gesture.moved = moved; td.gesture.cancelled = cancelled;
    td.gesture.pressed = 0; td.gesture.released = GetTickCount();
    LeaveCriticalSection(&td.lock);
    td_input(td.gesture.serial);
    SendMessageW(g_main_hwnd, WM_USER + 1, nid.uID, WM_LBUTTONUP);
}
static void test_tray_drag(void) {
    FullscreenMonitors monitors; RECT bar, child, r; HWND shell, tray, df, item, placeholder, clock_window;
    TrayRegions regions; TrayBarScan scan; POINT origin, target, other;
    size_t i; DWORD serial; HANDLE dpi = fs_enter_dpi();
    fs_exit(); fs_get_monitors(&monitors);
    CHECK(monitors.count > 0, "drag has an online monitor");
    if (!monitors.count) { fs_leave_dpi(dpi); return; }
    bar = monitors.items[0].info.rcMonitor;
    bar.top = bar.bottom - 60;
    shell = td_test_window(L"Shell_TrayWnd", NULL, bar);
    child.left = bar.right - bar.left - 300; child.right = bar.right - bar.left;
    child.top = 0; child.bottom = 60;
    tray = td_test_window(L"TrayNotifyWnd", shell, child);
    origin.x = bar.right - 100; origin.y = bar.bottom - 30;
    target.x = monitors.items[0].info.rcMonitor.left + 200;
    target.y = monitors.items[0].info.rcMonitor.top + 200;
    other = origin;
    nid.hWnd = g_main_hwnd; nid.uID = 1;
    test_tray_rect_enabled = 1;
    SetRect(&test_tray_rect, origin.x - 10, origin.y - 10, origin.x + 10, origin.y + 10);
    CHECK(td_start(g_main_hwnd), "mouse observer starts on isolated desktop");
    CHECK(td_source_is_tray(tray), "native tray source recognized");
    td_get_regions(&regions);
    CHECK(td_in_regions(&regions, origin), "native notification area excluded");
    CHECK(!td_in_regions(&regions, target), "desktop is not notification area");
    CHECK(!td_crossed_threshold(origin, origin, 4, 4), "stationary press is a click");
    other.x += 3; CHECK(!td_crossed_threshold(origin, other, 4, 4), "small jitter is a click");
    other.x += 1; CHECK(td_crossed_threshold(origin, other, 4, 4), "system drag threshold is honored");

    is_running = 0; is_paused = 0; is_overtime = 0; current_timer_mode = TIMER_NONE;
    for (i = 0; i < monitors.count; ++i) {
        target.x = monitors.items[i].info.rcMonitor.left + 100;
        target.y = monitors.items[i].info.rcMonitor.top + 100;
        td_test_down(tray, origin); td_test_up(target, 1, 0);
        CHECK(fs_count == i + 1 && fs_window_index(monitors.items[i].identity) >= 0, "drag adds only destination monitor and preserves others");
        CHECK(!is_running && !is_paused && current_timer_mode == TIMER_NONE, "drag release does not change timer");
        serial = td.gesture.serial;
        td_input(serial); SendMessageW(g_main_hwnd, WM_USER + 1, nid.uID, WM_LBUTTONUP);
        CHECK(fs_count == i + 1 && !is_running, "late and duplicate callbacks do not toggle timer or fullscreen");
        td_test_down(tray, origin); td_test_up(target, 1, 0);
        CHECK(fs_count == i + 1, "repeated drag to same screen is idempotent");
    }
    fs_exit();
    td_test_down(tray, origin); td_test_up(origin, 1, 0);
    CHECK(!fs_active && !is_running, "drag back to tray does not start fullscreen or timer");
    td_test_down(tray, origin); td_test_up(target, 1, 1);
    CHECK(!fs_active && !is_running, "cancelled drag does not start fullscreen or timer");
    td_test_down(tray, origin);
    EnterCriticalSection(&td.lock); td.gesture.source = (HWND)(INT_PTR)-1; LeaveCriticalSection(&td.lock);
    td_test_up(target, 1, 0);
    CHECK(!fs_active && !is_running, "destroyed source does not turn drag into a click");
    EnterCriticalSection(&td.lock); td.confirmed = 0; LeaveCriticalSection(&td.lock);
    SendMessageW(g_main_hwnd, WM_USER + 1, nid.uID, WM_LBUTTONUP);
    CHECK(!is_running, "unrecognized drag source still cannot trigger timer click");
    td_test_down(tray, origin); td_cancel();
    CHECK(td_snapshot().cancelled, "escape or shell restart cancels active gesture");
    td_test_up(target, 1, 1);
    target.x = -100000; target.y = -100000;
    td_test_down(tray, origin); td_test_up(target, 1, 0);
    CHECK(!fs_active && !is_running, "off-monitor release does not fall back to nearest screen");
    td_test_down(tray, origin); td_test_up(origin, 0, 0);
    CHECK(is_running, "ordinary tray click still starts timer");
    td_test_down(tray, origin); td_test_up(origin, 0, 0);
    CHECK(!is_running, "next ordinary tray click still stops timer");

    /* Rapid consecutive clicks: next DOWN begins before previous UP callback arrives. */
    td_test_down(tray, origin);
    EnterCriticalSection(&td.lock);
    td.gesture.pressed = 0; td.gesture.released = GetTickCount();
    LeaveCriticalSection(&td.lock);
    td_test_down(tray, origin);
    SendMessageW(g_main_hwnd, WM_USER + 1, nid.uID, WM_LBUTTONUP);
    CHECK(is_running, "rapid click 1 toggles timer to running even if click 2 already pressed");
    EnterCriticalSection(&td.lock);
    td.gesture.pressed = 0; td.gesture.released = GetTickCount();
    LeaveCriticalSection(&td.lock);
    SendMessageW(g_main_hwnd, WM_USER + 1, nid.uID, WM_LBUTTONUP);
    CHECK(!is_running, "rapid click 2 toggles timer to stopped");
    td_test_down(tray, origin);
    EnterCriticalSection(&td.lock);
    td.gesture.pressed = 0; td.gesture.released = GetTickCount();
    LeaveCriticalSection(&td.lock);
    SendMessageW(g_main_hwnd, WM_USER + 1, nid.uID, WM_LBUTTONUP);
    CHECK(is_running, "rapid click 3 toggles timer back to running");
    td_test_down(tray, origin);
    EnterCriticalSection(&td.lock);
    td.gesture.pressed = 0; td.gesture.released = GetTickCount();
    LeaveCriticalSection(&td.lock);
    SendMessageW(g_main_hwnd, WM_USER + 1, nid.uID, WM_LBUTTONUP);
    CHECK(!is_running, "rapid click 4 toggles timer back to stopped");

    /* Seal regression: another icon initially occupies the pressed position.
       Reordering then moves our icon into it. Neither UP nor a delayed own
       DOWN/UP callback may retrospectively acquire this foreign gesture. */
    SetRect(&test_tray_rect, origin.x + 40, origin.y - 10, origin.x + 60, origin.y + 10);
    td_test_down(tray, origin);
    serial = td.gesture.serial;
    CHECK(td.confirmed != serial, "foreign icon press is not authorized");
    SetRect(&test_tray_rect, origin.x - 10, origin.y - 10, origin.x + 10, origin.y + 10);
    td_input(serial);
    SendMessageW(g_main_hwnd, WM_USER + 1, nid.uID, WM_LBUTTONDOWN);
    CHECK(td.confirmed != serial, "repeated input and delayed DOWN cannot reclassify a rejected press");
    target.x = monitors.items[0].info.rcMonitor.left + 100;
    target.y = monitors.items[0].info.rcMonitor.top + 100;
    td_test_up(target, 1, 0);
    CHECK(td.confirmed != serial && !fs_active && !is_running, "reordering and delayed UP never authorize another icon's drag");

    test_tray_rect_queries = 0;
    test_tray_rect_during_query = td_test_move_during_query;
    td_test_down(tray, origin);
    serial = td.gesture.serial;
    test_tray_rect_during_query = NULL;
    CHECK(test_tray_rect_queries == 1 && td.confirmed != serial, "input advancing during rectangle query fails closed");
    td_test_up(target, 1, 0);
    CHECK(!fs_active && !is_running, "missed press cannot trigger fullscreen on release");

    /* Queue delay: a complete gesture is first observed after release. */
    EnterCriticalSection(&td.lock);
    ++td.gesture.serial; td.gesture.pressed = 0; td.gesture.moved = 1;
    serial = td.gesture.serial;
    LeaveCriticalSection(&td.lock);
    test_tray_rect_queries = 0;
    td_input(serial);
    SendMessageW(g_main_hwnd, WM_USER + 1, nid.uID, WM_LBUTTONUP);
    CHECK(test_tray_rect_queries == 0 && td.confirmed != serial && !fs_active && !is_running,
        "late processing never queries release-time geometry to acquire a drag");

    /* DF geometry with dynamic class suffixes, reordered clock and placeholder. */
    r = bar; r.top -= 100; r.bottom -= 100;
    df = td_test_window(L"DFTaskbar:test", NULL, r);
    child.left = 400; child.right = 448; child.top = 0; child.bottom = 60;
    item = td_test_window(L"DFTaskbarItem:TrayIcon:test", df, child);
    child.left = 450; child.right = 498;
    td_test_window(L"DFTaskbarItem:TrayIcon:second", df, child);
    child.left = 10; child.right = 100;
    clock_window = td_test_window(L"DFTaskbarItem:TrayClock:test", df, child);
    child.left = -10000; child.right = -9952;
    placeholder = td_test_window(L"DFTaskbarItem:TrayIconPlaceholder:test", df, child);
    memset(&regions, 0, sizeof(regions)); scan.regions = &regions; scan.bar = r; scan.df = 1;
    EnumChildWindows(df, td_collect_child, (LPARAM)&scan); td_join_regions(&regions, 0, &r);
    CHECK(regions.count == 2, "DF adjacent icons merge but distant clock does not bridge task buttons");
    target.x = r.left + 449; target.y = r.top + 30;
    CHECK(td_in_regions(&regions, target), "DF inter-icon padding is excluded");
    target.x = r.left + 250;
    CHECK(!td_in_regions(&regions, target), "DF normal task area remains a valid drop target");
    target.x = r.left - 200;
    CHECK(!td_in_regions(&regions, target), "DF off-screen placeholder never expands exclusion area");
    CHECK(!td_source_is_tray(item), "DF class spoof from another executable is not accepted as source");
    ShowWindow(clock_window, SW_HIDE);
    memset(&regions, 0, sizeof(regions)); EnumChildWindows(df, td_collect_child, (LPARAM)&scan);
    td_join_regions(&regions, 0, &r);
    CHECK(regions.count == 1, "hidden tray parts are ignored");
    CHECK(IsWindow(placeholder), "placeholder remained available during filtering test");
    regions.overflow = 1;
    CHECK(td_in_regions(&regions, target), "incomplete region map fails closed");

    td_stop(); CHECK(!td.initialized && !td.thread, "observer and hooks stop cleanly");
    test_tray_rect_enabled = 0;
    DestroyWindow(df); DestroyWindow(shell); fs_exit();
    fs_leave_dpi(dpi);
    puts("Tray drag synthetic regression finished (live shell drag not simulated).");
}
