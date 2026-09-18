/* In-process integration tests. All persistence is redirected to a caller-owned
   scratch directory; the installed app, its mutex and registry are untouched. */
#define UNICODE
#define _UNICODE
#include <windows.h>
static BOOL WINAPI test_track_popup_menu(HMENU, UINT, int, int, int, HWND, const RECT *);
#define TrackPopupMenu test_track_popup_menu
#define WinMain PomodoroOriginalWinMain
#include "../pomodoro-timer.c"
#undef WinMain

static int failures;
static int overlay_created;
static int overlay_destroyed;
#define CHECK(condition, name) do { if (!(condition)) { printf("FAIL: %s (line %d)\n", name, __LINE__); ++failures; } } while (0)

static int menu_request;
static BOOL WINAPI test_track_popup_menu(HMENU menu, UINT flags, int x, int y, int reserved, HWND owner, const RECT *rect) {
    const wchar_t *labels[] = {L"会话", L"切换", L"待机", L"调整", L"时长", L"操作", L"颜色", L"存储", L"偏好", L"", L"统计", L"", L"退出"};
    int i, j;
    wchar_t label[80];
    HMENU session = GetSubMenu(menu, 0), colors = GetSubMenu(menu, 6);
    (void)flags; (void)x; (void)y; (void)reserved; (void)owner; (void)rect;
    CHECK(GetMenuItemCount(menu) == 13, "root menu has no standalone fullscreen/settings entries");
    for (i = 0; i < 13; ++i) {
        HMENU child = GetSubMenu(menu, i);
        GetMenuStringW(menu, i, label, 80, MF_BYPOSITION);
        CHECK(!wcscmp(label, labels[i]), "root ordering includes operation / color / storage");
        if (child) for (j = 0; j < GetMenuItemCount(child); ++j)
            CHECK(GetSubMenu(child, j) == NULL, "menu depth does not exceed two levels");
    }
    CHECK(GetMenuItemID(session, 4) == ID_MENU_FULLSCREEN, "fullscreen placed after session separator");
    GetMenuStringW(session, 4, label, 80, MF_BYPOSITION);
    CHECK(!wcscmp(label, L"全屏"), "fullscreen menu label stays short");
    CHECK(!!(GetMenuState(session, 4, MF_BYPOSITION) & MF_CHECKED) == !!fs_active, "fullscreen menu reflects active state");
    CHECK(GetMenuItemCount(colors) == 7, "five colors plus separator and reset");
    for (i = 0; i < 5; ++i) {
        GetMenuStringW(colors, i, label, 80, MF_BYPOSITION);
        CHECK(!wcscmp(label, fs_color_names[i]), "color item has concise label");
        CHECK(GetMenuItemID(colors, i) == (UINT)(ID_MENU_COLOR_FIRST + i), "color command maps to intended mode");
    }
    CHECK(GetMenuItemID(colors, 6) == ID_MENU_COLOR_RESET, "color reset is directly accessible");
    return menu_request;
}

static void choose_menu(int command) {
    menu_request = command;
    SendMessageW(g_main_hwnd, WM_USER + 1, 0, WM_RBUTTONUP);
    menu_request = 0;
}

static LRESULT CALLBACK trace_window_messages(int code, WPARAM sent, LPARAM value) {
    if (code >= 0) {
        CWPSTRUCT *message = (CWPSTRUCT *)value;
        wchar_t name[64];
        GetClassNameW(message->hwnd, name, 64);
        if (!wcscmp(name, L"PomodoroFullscreen")) {
            if (message->message == WM_CREATE) ++overlay_created;
            if (message->message == WM_DESTROY) ++overlay_destroyed;
        }
        if (!wcscmp(name, L"PomodoroFullscreen") &&
            (message->message == WM_LBUTTONDOWN || message->message == WM_RBUTTONDOWN ||
             message->message == WM_MBUTTONDOWN || message->message == WM_XBUTTONDOWN ||
             message->message == WM_KEYDOWN || message->message == WM_CLOSE))
            printf("Overlay input: message=0x%04X wParam=%llu sent=%llu\n", message->message,
                (unsigned long long)message->wParam, (unsigned long long)sent);
    }
    return CallNextHookEx(NULL, code, sent, value);
}

static void pump(unsigned milliseconds) {
    ULONGLONG end = GetTickCount64() + milliseconds;
    MSG message;
    do {
        while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message != WM_QUIT) { TranslateMessage(&message); DispatchMessageW(&message); }
        }
        Sleep(10);
    } while (GetTickCount64() < end);
}

static int monitor_count;
static BOOL CALLBACK count_monitors(HMONITOR monitor, HDC dc, LPRECT rect, LPARAM value) {
    size_t i;
    int found = 0;
    MONITORINFO info = {sizeof(info)};
    (void)dc; (void)value;
    GetMonitorInfoW(monitor, &info);
    printf("Monitor: %ld,%ld %ldx%ld primary=%d\n", rect->left, rect->top,
        rect->right - rect->left, rect->bottom - rect->top, !!(info.dwFlags & MONITORINFOF_PRIMARY));
    for (i = 0; i < fs_count; ++i) {
        RECT actual;
        GetWindowRect(fs_windows[i], &actual);
        if (EqualRect(rect, &actual)) found = 1;
        CHECK((GetWindowLongW(fs_windows[i], GWL_EXSTYLE) & WS_EX_TOPMOST) != 0, "overlay topmost");
        CHECK((GetWindowLongW(fs_windows[i], GWL_EXSTYLE) & WS_EX_TOOLWINDOW) != 0, "overlay excluded from taskbar");
    }
    CHECK(found, "every monitor covered by exact full bounds");
    ++monitor_count;
    return TRUE;
}

static void capture_window(HWND window, const wchar_t *name) {
    typedef HANDLE (WINAPI *SetDpiFn)(HANDLE);
    typedef HANDLE (WINAPI *GetDpiFn)(HWND);
    SetDpiFn set_dpi = (SetDpiFn)(void *)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetThreadDpiAwarenessContext");
    GetDpiFn get_dpi = (GetDpiFn)(void *)GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetWindowDpiAwarenessContext");
    HANDLE old_dpi = set_dpi && get_dpi ? set_dpi(get_dpi(window)) : NULL;
    RECT rect;
    BITMAPINFO info = {0};
    BITMAPFILEHEADER header = {0};
    void *pixels;
    HDC screen = GetDC(window), memory = CreateCompatibleDC(screen);
    HBITMAP bitmap, old;
    FILE *file;
    wchar_t path[MAX_PATH];
    GetClientRect(window, &rect);
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = rect.right;
    info.bmiHeader.biHeight = -rect.bottom;
    info.bmiHeader.biPlanes = 1; info.bmiHeader.biBitCount = 32; info.bmiHeader.biCompression = BI_RGB;
    bitmap = CreateDIBSection(screen, &info, DIB_RGB_COLORS, &pixels, NULL, 0);
    CHECK(bitmap != NULL, "capture bitmap allocated");
    if (!bitmap) { DeleteDC(memory); ReleaseDC(window, screen); if (old_dpi) set_dpi(old_dpi); return; }
    old = (HBITMAP)SelectObject(memory, bitmap);
    ShowWindow(window, SW_SHOW);
    RedrawWindow(window, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    CHECK(PrintWindow(window, memory, PW_CLIENTONLY | 2), "window renders for capture");
    header.bfType = 0x4d42; header.bfOffBits = sizeof(header) + sizeof(BITMAPINFOHEADER);
    header.bfSize = header.bfOffBits + rect.right * rect.bottom * 4;
    join_path(path, MAX_PATH, g_data_dir, name);
    file = _wfopen(path, L"wb");
    if (file) {
        fwrite(&header, sizeof(header), 1, file); fwrite(&info.bmiHeader, sizeof(BITMAPINFOHEADER), 1, file);
        fwrite(pixels, 4, (size_t)rect.right * rect.bottom, file); fclose(file);
    }
    if (wcsncmp(name, L"fullscreen-", 11) == 0)
        CHECK(GetPixel(memory, 0, 0) == RGB(0, 0, 0), "fullscreen background is black");
    SelectObject(memory, old); DeleteObject(bitmap); DeleteDC(memory); ReleaseDC(window, screen);
    if (old_dpi) set_dpi(old_dpi);
}

static int dialog_case;
static int dialog_index;
static HWND picker_owner;
static int picker_seen;
static BOOL CALLBACK close_picker(HWND window, LPARAM unused) {
    wchar_t name[40];
    (void)unused;
    GetClassNameW(window, name, 40);
    if (GetWindow(window, GW_OWNER) == picker_owner && !wcscmp(name, L"#32770")) {
        picker_seen = 1;
        PostMessageW(window, WM_COMMAND, IDOK, 0);
    }
    return TRUE;
}
static VOID CALLBACK picker_timer(HWND window, UINT msg, UINT_PTR id, DWORD tick) {
    (void)window; (void)msg; (void)tick;
    EnumThreadWindows(GetCurrentThreadId(), close_picker, 0);
    if (picker_seen) KillTimer(NULL, id);
}
static BOOL CALLBACK exercise_dialog(HWND dialog, LPARAM unused) {
    wchar_t title[80];
    wchar_t expected_title[40];
    DWORD first = 0, last = 0;
    (void)unused;
    GetWindowTextW(dialog, title, 80);
    swprintf(expected_title, 40, L"%ls颜色", fs_color_names[dialog_index]);
    if (wcscmp(title, expected_title)) return TRUE;
    CHECK(GetDlgItem(dialog, IDC_FS_EDIT_FIRST + 1) == NULL, "dialog edits only one color");
    SendDlgItemMessageW(dialog, IDC_FS_EDIT_FIRST, EM_GETSEL, (WPARAM)&first, (LPARAM)&last);
    CHECK(first == 0 && last == 7, "current code is selected on entry");
    if (dialog_case == 1) {
        SetDlgItemTextW(dialog, IDC_FS_EDIT_FIRST, L"#12GG00");
        CHECK(!IsWindowEnabled(GetDlgItem(dialog, IDOK)), "invalid hex disables save");
        CHECK(settings.fullscreen_colors[0] == fs_default_colors[0], "draft does not alter live settings");
        SetDlgItemTextW(dialog, IDC_FS_EDIT_FIRST, L"abcdef");
        CHECK(IsWindowEnabled(GetDlgItem(dialog, IDOK)), "optional hash and lowercase accepted");
        capture_window(dialog, L"colors-dialog.bmp");
        SendMessageW(dialog, WM_COMMAND, IDCANCEL, 0);
    } else if (dialog_case == 2) {
        SetDlgItemTextW(dialog, IDC_FS_EDIT_FIRST, L"#123456");
        SendMessageW(dialog, WM_COMMAND, IDOK, 0);
    } else {
        picker_owner = dialog;
        SetTimer(NULL, 0, 100, picker_timer);
        SendMessageW(dialog, WM_COMMAND, IDC_FS_PICK_FIRST, 0);
        CHECK(picker_seen, "native color chooser opens and accepts color");
        SendMessageW(dialog, WM_COMMAND, IDCANCEL, 0);
    }
    return FALSE;
}

static VOID CALLBACK dialog_timer(HWND hwnd, UINT msg, UINT_PTR id, DWORD tick) {
    (void)hwnd; (void)msg; (void)tick;
    KillTimer(NULL, id);
    EnumThreadWindows(GetCurrentThreadId(), exercise_dialog, 0);
}

static void write_test_json(const wchar_t *path, const char *json) {
    FILE *file = _wfopen(path, L"w");
    CHECK(file != NULL, "open isolated settings fixture");
    if (file) { fputs(json, file); fclose(file); }
}

int wmain(int argc, wchar_t **argv) {
    WNDCLASSW wc = {0};
    FullscreenView view;
    int i, rgb;
    DWORD gdi_before;
    typedef HANDLE (WINAPI *DpiFn)(HANDLE);
    DpiFn dpi = (DpiFn)(void *)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetThreadDpiAwarenessContext");
    HANDLE old_dpi;
    HHOOK trace_hook;
    HDESK test_desktop = NULL;
    int interactive = 0, stress = 0;
    if (argc < 2 || wcsncmp(argv[1], L"tools\\.fullscreen-test\\", 23) != 0 || wcsstr(argv[1], L"..")) {
        /* Require the script's relative output path, not a personal data path. */
        fprintf(stderr, "Usage: fullscreen_test.exe tools\\.fullscreen-test\\run-id\n"); return 2;
    }
    for (i = 2; i < argc; ++i) {
        if (!wcscmp(argv[i], L"--interactive")) interactive = 1;
        if (!wcscmp(argv[i], L"--stress")) stress = 1;
    }
    if (!interactive) {
        wchar_t desktop_name[80];
        swprintf(desktop_name, 80, L"PomodoroFullscreenTest-%lu", GetCurrentProcessId());
        test_desktop = CreateDesktopW(desktop_name, NULL, NULL, 0,
            DESKTOP_CREATEWINDOW | DESKTOP_CREATEMENU | DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS | DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL, NULL);
        if (!test_desktop || !SetThreadDesktop(test_desktop)) {
            fprintf(stderr, "Could not create isolated test desktop: %lu\n", GetLastError());
            if (test_desktop) CloseDesktop(test_desktop);
            return 2;
        }
        puts("Using an isolated desktop; the visible desktop is never switched.");
    }
    {
        HMENU probe = CreatePopupMenu();
        if (!probe) { fprintf(stderr, "Test desktop cannot create menus: %lu\n", GetLastError()); return 2; }
        DestroyMenu(probe);
    }
    GetFullPathNameW(argv[1], MAX_PATH, g_data_dir, NULL);
    build_data_file_paths();
    InitializeCriticalSection(&g_settings_file_lock); g_settings_lock_ready = 1;
    load_settings();
    init_system_metrics();
    wc.hInstance = GetModuleHandleW(NULL); wc.lpfnWndProc = WndProc; wc.lpszClassName = L"PomodoroFullscreenTestMain";
    RegisterClassW(&wc);
    g_main_hwnd = CreateWindowW(wc.lpszClassName, L"Fullscreen test", 0, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
    CHECK(g_main_hwnd != NULL, "test main window created");
    KillTimer(g_main_hwnd, ID_MAIN_DAY_SYNC_TIMER);
    trace_hook = SetWindowsHookExW(WH_CALLWNDPROC, trace_window_messages, NULL, GetCurrentThreadId());
    CHECK(trace_hook != NULL, "window lifecycle observer installed");

    CHECK(fs_parse_color(L"#7F8C98", &rgb) && rgb == 0x7F8C98, "parse supplied focus color");
    CHECK(fs_parse_color(L"c79a52", &rgb) && rgb == 0xC79A52, "parse supplied overtime color");
    CHECK(!fs_parse_color(L"#12", &rgb) && !fs_parse_color(L"#1234567", &rgb) && !fs_parse_color(L"#GGFFFF", &rgb), "reject malformed colors");
    CHECK(fs_colorref(0x123456) == RGB(0x12, 0x34, 0x56), "RGB ordering");

    is_running = 1; current_timer_mode = TIMER_SHORT_POMODORO; remaining_seconds = 1500;
    fs_read_view(&view); CHECK(!wcscmp(view.time, L"25:00") && view.color == 0x7F8C98, "focus countdown presentation");
    remaining_seconds = 6000; fs_read_view(&view); CHECK(!wcscmp(view.time, L"100:00"), "three digit minutes");
    remaining_seconds = 2147483647; fs_read_view(&view); CHECK(!wcscmp(view.time, L"35791394:07"), "maximum timer value formats without truncation");
    current_timer_mode = TIMER_SHORT_BREAK; fs_read_view(&view); CHECK(view.color == 0x829889, "break palette");
    current_timer_mode = TIMER_COUNT_UP; remaining_seconds = 192; fs_read_view(&view); CHECK(!wcscmp(view.time, L"03:12") && view.color == 0x8C86A3, "count up has no plus");
    current_timer_mode = TIMER_CUSTOM; fs_read_view(&view); CHECK(view.color == 0xA39182, "custom palette");
    is_overtime = 1; overtime_seconds = 192; overtime_source_mode = TIMER_SHORT_BREAK;
    fs_read_view(&view); CHECK(!wcscmp(view.time, L"+03:12") && view.color == 0xC79A52, "overtime overrides source palette");
    is_running = 0; is_paused = 1; fs_read_view(&view); CHECK(wcsstr(view.status, L"已暂停") != NULL && !wcscmp(view.time, L"+03:12"), "paused overtime retains time and color");
    is_paused = 0; is_overtime = 0; idle_mode = IDLE_BREAK; idle_break_is_long = 0;
    fs_read_view(&view); CHECK(!wcscmp(view.time, L"05:00") && wcsstr(view.status, L"未开始"), "idle preview does not start timer");

    SendMessageW(g_main_hwnd, WM_USER + 1, 0, WM_MBUTTONUP); pump(100);
    CHECK(fs_active && fs_count > 0 && !is_running, "tray middle click enters without starting");
    old_dpi = dpi ? dpi((HANDLE)(INT_PTR)-4) : NULL;
    EnumDisplayMonitors(NULL, NULL, count_monitors, 0);
    if (old_dpi) dpi(old_dpi);
    CHECK(fs_count == (size_t)monitor_count, "one overlay per monitor");
    if (fs_count) capture_window(fs_windows[0], L"fullscreen-rest.bmp");
    CHECK(fs_active, "fullscreen remains open after rest capture");
    is_running = 1; is_overtime = 1; overtime_seconds = 192; overtime_source_mode = TIMER_SHORT_POMODORO;
    fs_refresh(); pump(50);
    if (fs_count) capture_window(fs_windows[0], L"fullscreen-overtime.bmp");
    if (fs_count > 1) capture_window(fs_windows[1], L"fullscreen-portrait.bmp");
    SendMessageW(g_main_hwnd, WM_DISPLAYCHANGE, 32, 0); pump(300);
    CHECK(fs_active && fs_count == (size_t)monitor_count, "display change rebuild keeps all windows");
    if (fs_count) SendMessageW(fs_windows[fs_count - 1], WM_KEYDOWN, VK_ESCAPE, 0);
    CHECK(!fs_active && !fs_count && is_running && is_overtime, "escape exits all overlays without stopping timer");
    is_running = 0; is_overtime = 0; current_timer_mode = TIMER_NONE;
    gdi_before = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    for (i = 0; i < (stress ? 8 : 1); ++i) {
        fs_toggle(); pump(20);
        if (fs_count) SendMessageW(fs_windows[0], WM_LBUTTONDOWN, MK_LBUTTON, 0);
    }
    CHECK(!fs_active && !fs_count, "click exits all overlays");
    CHECK(GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS) <= gdi_before + 2, "repeated entry and paint do not leak GDI objects");

    dialog_case = 1; dialog_index = 0;
    SetTimer(NULL, 0, 150, dialog_timer); choose_menu(ID_MENU_COLOR_FIRST);
    CHECK(settings.fullscreen_colors[0] == fs_default_colors[0], "cancel discards edits");
    dialog_case = 2;
    for (dialog_index = 0; dialog_index < 5; ++dialog_index) {
        SetTimer(NULL, 0, 150, dialog_timer); choose_menu(ID_MENU_COLOR_FIRST + dialog_index);
        memset(settings.fullscreen_colors, 0, sizeof(settings.fullscreen_colors)); load_settings();
        for (i = 0; i < 5; ++i)
            CHECK(settings.fullscreen_colors[i] == (i <= dialog_index ? 0x123456 : fs_default_colors[i]), "save changes only chosen color and survives reload");
    }
    dialog_case = 4; dialog_index = 0;
    SetTimer(NULL, 0, 150, dialog_timer); choose_menu(ID_MENU_COLOR_FIRST);
    CHECK(settings.fullscreen_colors[0] == 0x123456, "picker cancel leaves saved color unchanged");
    {
        int duration = settings.long_pomodoro_duration;
        choose_menu(ID_MENU_COLOR_RESET);
        load_settings();
        CHECK(settings.long_pomodoro_duration == duration, "palette reset preserves timer settings");
        for (i = 0; i < 5; ++i) CHECK(settings.fullscreen_colors[i] == fs_default_colors[i], "menu restores and persists all default colors");
    }
    choose_menu(ID_MENU_FULLSCREEN); CHECK(fs_active, "session menu enters fullscreen");
    choose_menu(ID_MENU_FULLSCREEN); CHECK(!fs_active, "session menu exits fullscreen");

    settings.enable_clock_sound = 0; settings.enable_completion_sound = 0; settings.show_completion_dialog = 1;
    settings.enable_overtime_count_up = 1;
    fs_toggle(); start_timer(g_main_hwnd, 0, TIMER_CUSTOM); pump(1250);
    CHECK(is_overtime && is_running && fs_active && fs_view.time[0] == L'+', "real timer completes into fullscreen overtime");
    CHECK(!g_hToastWnd, "completion toast suppressed in fullscreen");
    CHECK(pomodoro_count == 0, "custom completion and overtime do not credit tomatoes");
    {
        int created = overlay_created, destroyed = overlay_destroyed, seconds = overtime_seconds;
        CHECK(created > 0, "lifecycle observer has seen real window creation");
        pump(3500);
        CHECK(fs_active && overtime_seconds >= seconds + 3, "steady fullscreen keeps advancing time");
        CHECK(created == overlay_created && destroyed == overlay_destroyed, "steady refresh does not recreate fullscreen windows");
        puts("Steady fullscreen: 3.5 seconds, no overlay creation or destruction.");
    }
    stop_timer_thread_if_needed(); clear_overtime_state(); fs_exit();
    settings.enable_overtime_count_up = 0; settings.show_completion_dialog = 0;
    fs_toggle(); start_timer(g_main_hwnd, 0, TIMER_CUSTOM); pump(200);
    CHECK(!is_running && fs_active && !wcscmp(fs_view.time, L"00:00") && wcsstr(fs_view.status, L"已完成"), "completion remains visible even with toast disabled");
    stop_timer_thread_if_needed(); fs_exit();
    fs_toggle(); CHECK(wcsstr(fs_view.status, L"未开始") != NULL, "reentry clears completed presentation"); fs_exit();

    /* Verify compatibility and the existing recovery chain without registry writes. */
    write_test_json(g_settings_path, "{\"pomodoro_duration\":42,\"pomodoro_count\":3}");
    load_settings();
    CHECK(settings.long_pomodoro_duration == 42 && pomodoro_count == 3, "legacy settings preserve existing values");
    for (i = 0; i < 5; ++i) CHECK(settings.fullscreen_colors[i] == fs_default_colors[i], "legacy settings get new palette defaults");
    write_test_json(g_settings_path, "{\"pomodoro_duration\":42,\"pomodoro_count\":3,\"fullscreen_focus_color\":-1,\"fullscreen_break_color\":16777216,\"fullscreen_count_up_color\":\"bad\"}");
    load_settings();
    for (i = 0; i < 5; ++i) CHECK(settings.fullscreen_colors[i] == fs_default_colors[i], "invalid stored color falls back to default");
    settings.fullscreen_colors[0] = 0x654321; CHECK(save_settings(), "save settings recovery fixture");
    CopyFileW(g_settings_path, g_settings_tmp_path, FALSE);
    write_test_json(g_settings_path, "corrupt");
    load_settings(); CHECK(settings.fullscreen_colors[0] == 0x654321, "temporary settings recovery preserves colors");
    CopyFileW(g_settings_path, g_settings_bak_path, FALSE);
    write_test_json(g_settings_path, "corrupt");
    load_settings(); CHECK(settings.fullscreen_colors[0] == 0x654321, "backup settings recovery preserves colors");

    pump(6000);
    CHECK(IsWindow(g_main_hwnd), "main message loop remains responsive during smoke interval");
    if (trace_hook) UnhookWindowsHookEx(trace_hook);
    DestroyWindow(g_main_hwnd);
    /* Windows closes the isolated desktop when this process releases its final references. */
    if (test_desktop) CloseDesktop(test_desktop);
    printf("FULLSCREEN_TEST: %s (%d failures)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
