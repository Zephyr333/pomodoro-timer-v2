#ifndef POMODORO_TRAY_DRAG_H
#define POMODORO_TRAY_DRAG_H

/* The shell owns the actual icon drag. Observe input without capture or
   swallowing mouse events. Authorize only the initial, stationary press;
   shell callbacks carry no gesture ID and must never authorize a drag. */
#define WM_TRAY_DRAG_INPUT (WM_APP + 110)
#define TD_MAX_REGIONS 256

typedef struct {
    DWORD serial, started, released;
    POINT origin, point;
    HWND source;
    int pressed, moved, cancelled;
} TrayGesture;
typedef struct { RECT rects[TD_MAX_REGIONS]; size_t count; int overflow; } TrayRegions;
typedef struct {
    HWND owner;
    HANDLE thread, ready, stop;
    DWORD thread_id;
    CRITICAL_SECTION lock;
    HHOOK mouse_hook, key_hook;
    int initialized, available, dx, dy;
    TrayGesture gesture;
    DWORD assessed, confirmed, finished;
    TrayRegions initial_regions;
    UINT_PTR watchdog;
} TrayDrag;
static TrayDrag td;
static FILE *td_log_file;
static unsigned td_log_lines;

/* Opt-in diagnostics contain only tray gesture metadata, never key contents
   or other applications' titles. Bound each run to 1024 lines. */
static void td_log(const char *event, TrayGesture gesture) {
    wchar_t cls[256];
    if (!td_log_file || td_log_lines >= 1024) return;
    cls[0] = 0; GetClassNameW(gesture.source, cls, 256);
    fprintf(td_log_file, "%lu %s serial=%lu source=%ls origin=%ld,%ld point=%ld,%ld pressed=%d moved=%d cancelled=%d confirmed=%lu screens=%zu\n",
        GetTickCount(), event, gesture.serial, cls, gesture.origin.x, gesture.origin.y,
        gesture.point.x, gesture.point.y, gesture.pressed, gesture.moved, gesture.cancelled, td.confirmed, fs_count);
    fflush(td_log_file); ++td_log_lines;
}
static void td_open_log(void) {
    wchar_t path[MAX_PATH], *name;
    DWORD length = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (!length || length >= MAX_PATH) return;
    name = wcsrchr(path, L'\\');
    if (!name || (size_t)(name + 1 - path) + wcslen(L"tray-drag.log") >= MAX_PATH) return;
    wcscpy(name + 1, L"tray-drag.log");
    td_log_file = _wfopen(path, L"w"); td_log_lines = 0;
}

static int td_prefix(const wchar_t *s, const wchar_t *prefix) {
    return !wcsncmp(s, prefix, wcslen(prefix));
}
static void td_class(HWND window, wchar_t cls[256]) {
    cls[0] = 0; GetClassNameW(window, cls, 256);
}
static int td_df_process(HWND window) {
    DWORD pid; HANDLE process; wchar_t path[MAX_PATH], *base; DWORD size = MAX_PATH;
    int result = 0;
    GetWindowThreadProcessId(window, &pid);
    process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) return 0;
    if (QueryFullProcessImageNameW(process, 0, path, &size)) {
        base = wcsrchr(path, L'\\');
        result = !_wcsicmp(base ? base + 1 : path, L"DisplayFusion.exe");
    }
    CloseHandle(process); return result;
}
static int td_native_root(const wchar_t *cls) {
    return !wcscmp(cls, L"Shell_TrayWnd") || !wcscmp(cls, L"Shell_SecondaryTrayWnd");
}
static int td_overflow_root(const wchar_t *cls) {
    return !wcscmp(cls, L"NotifyIconOverflowWindow") ||
        !wcscmp(cls, L"TopLevelWindowForOverflowXamlIsland");
}
static int td_df_tray_part(const wchar_t *cls) {
    return td_prefix(cls, L"DFTaskbarItem:TrayIcon:") ||
        td_prefix(cls, L"DFTaskbarItem:TrayClock:") ||
        td_prefix(cls, L"DFTaskbarItem:NetworkVolumePower:") ||
        td_prefix(cls, L"DFTaskbarItem:Microphone:") ||
        td_prefix(cls, L"DFTaskbarItem:Location:") ||
        td_prefix(cls, L"DFTaskbarItem:ShowDesktop:");
}
static void td_region_add(TrayRegions *regions, const RECT *rect) {
    if (IsRectEmpty(rect)) return;
    if (regions->count == TD_MAX_REGIONS) { regions->overflow = 1; return; }
    regions->rects[regions->count++] = *rect;
}
typedef struct {
    TrayRegions *regions;
    RECT bar;
    int df;
} TrayBarScan;
static BOOL CALLBACK td_collect_child(HWND window, LPARAM value) {
    TrayBarScan *scan = (TrayBarScan *)value;
    RECT rect, clipped; wchar_t cls[256];
    if (!IsWindowVisible(window)) return TRUE;
    td_class(window, cls);
    if (!(scan->df ? td_df_tray_part(cls) : !wcscmp(cls, L"TrayNotifyWnd"))) return TRUE;
    if (GetWindowRect(window, &rect) && IntersectRect(&clipped, &rect, &scan->bar))
        td_region_add(scan->regions, &clipped);
    return TRUE;
}
/* Join neighboring tray items, including small padding gaps, but never span
   unrelated task buttons when the clock/tray were placed on opposite sides. */
static void td_join_regions(TrayRegions *regions, size_t first, const RECT *bar) {
    size_t i, j; int vertical = bar->bottom - bar->top > bar->right - bar->left;
    LONG thickness = vertical ? bar->right - bar->left : bar->bottom - bar->top;
    LONG gap = thickness / 8;
    for (i = first; i < regions->count; ++i) {
        for (j = i + 1; j < regions->count;) {
            RECT a = regions->rects[i], b = regions->rects[j];
            int adjacent = vertical ?
                (a.left < b.right && b.left < a.right && a.top <= b.bottom + gap && b.top <= a.bottom + gap) :
                (a.top < b.bottom && b.top < a.bottom && a.left <= b.right + gap && b.left <= a.right + gap);
            if (adjacent) {
                UnionRect(&regions->rects[i], &a, &b);
                memmove(&regions->rects[j], &regions->rects[j + 1],
                    (regions->count - j - 1) * sizeof(RECT));
                --regions->count; j = i + 1;
            } else ++j;
        }
    }
}
static BOOL CALLBACK td_collect_bar(HWND window, LPARAM value) {
    TrayRegions *regions = (TrayRegions *)value;
    TrayBarScan scan; wchar_t cls[256]; size_t first;
    if (!IsWindowVisible(window)) return TRUE;
    td_class(window, cls);
    scan.df = td_prefix(cls, L"DFTaskbar:") && td_df_process(window);
    if (!scan.df && !td_native_root(cls) && !td_overflow_root(cls)) return TRUE;
    if (!GetWindowRect(window, &scan.bar) || IsRectEmpty(&scan.bar) ||
        !MonitorFromRect(&scan.bar, MONITOR_DEFAULTTONULL)) return TRUE;
    if (td_overflow_root(cls)) { td_region_add(regions, &scan.bar); return TRUE; }
    first = regions->count; scan.regions = regions;
    EnumChildWindows(window, td_collect_child, (LPARAM)&scan);
    if (scan.df) td_join_regions(regions, first, &scan.bar);
    /* Unknown shell versions: fail closed inside that taskbar only. */
    if (regions->count == first) td_region_add(regions, &scan.bar);
    return TRUE;
}
static void td_get_regions(TrayRegions *regions) {
    HANDLE previous = fs_enter_dpi();
    memset(regions, 0, sizeof(*regions));
    EnumWindows(td_collect_bar, (LPARAM)regions);
    fs_leave_dpi(previous);
}
static int td_in_regions(const TrayRegions *regions, POINT point) {
    size_t i;
    if (regions->overflow) return 1; /* Incomplete exclusion map must not trigger. */
    for (i = 0; i < regions->count; ++i)
        if (PtInRect(&regions->rects[i], point)) return 1;
    return 0;
}
static int td_source_is_tray(HWND source) {
    HWND root = GetAncestor(source, GA_ROOT); wchar_t cls[256], root_cls[256];
    if (!root || !IsWindowVisible(root)) return 0;
    td_class(root, root_cls);
    if (td_native_root(root_cls) || td_overflow_root(root_cls)) return 1;
    td_class(source, cls);
    return td_prefix(root_cls, L"DFTaskbar:") && td_prefix(cls, L"DFTaskbarItem:TrayIcon:") &&
        td_df_process(root);
}
static int td_crossed_threshold(POINT origin, POINT point, int dx, int dy) {
    return llabs((long long)point.x - origin.x) >= dx || llabs((long long)point.y - origin.y) >= dy;
}
static TrayGesture td_snapshot(void) {
    TrayGesture gesture;
    EnterCriticalSection(&td.lock); gesture = td.gesture; LeaveCriticalSection(&td.lock);
    return gesture;
}
static void td_cancel(void) {
    if (!td.initialized) return;
    EnterCriticalSection(&td.lock); td.gesture.cancelled = 1; LeaveCriticalSection(&td.lock);
}
static LRESULT CALLBACK td_mouse_proc(int code, WPARAM message, LPARAM value) {
    if (code >= 0 && (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || message == WM_MOUSEMOVE)) {
        const MSLLHOOKSTRUCT *input = (const MSLLHOOKSTRUCT *)value;
        DWORD serial = 0; int notify = 0;
        EnterCriticalSection(&td.lock);
        if (message == WM_LBUTTONDOWN) {
            serial = td.gesture.serial + 1; if (!serial) serial = 1;
            memset(&td.gesture, 0, sizeof(td.gesture));
            td.gesture.serial = serial; td.gesture.started = GetTickCount();
            td.gesture.origin = td.gesture.point = input->pt;
            td.gesture.source = WindowFromPoint(input->pt);
            td.gesture.pressed = 1; notify = 1;
        } else if (td.gesture.pressed) {
            td.gesture.point = input->pt;
            if (td_crossed_threshold(td.gesture.origin, input->pt, td.dx, td.dy)) td.gesture.moved = 1;
            if (message == WM_LBUTTONUP) {
                td.gesture.pressed = 0; td.gesture.released = GetTickCount(); notify = 1;
            }
        }
        serial = td.gesture.serial;
        LeaveCriticalSection(&td.lock);
        if (message == WM_LBUTTONDOWN && !td.watchdog) td.watchdog = SetTimer(NULL, 0, 100, NULL);
        if (message == WM_LBUTTONUP && td.watchdog) { KillTimer(NULL, td.watchdog); td.watchdog = 0; }
        if (notify) PostMessageW(td.owner, WM_TRAY_DRAG_INPUT, serial, message);
    }
    return CallNextHookEx(NULL, code, message, value);
}
static LRESULT CALLBACK td_key_proc(int code, WPARAM message, LPARAM value) {
    if (code >= 0 && (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) &&
        ((const KBDLLHOOKSTRUCT *)value)->vkCode == VK_ESCAPE) td_cancel();
    return CallNextHookEx(NULL, code, message, value);
}
static DWORD WINAPI td_input_thread(void *unused) {
    MSG message; HANDLE previous = fs_enter_dpi();
    (void)unused;
    PeekMessageW(&message, NULL, 0, 0, PM_NOREMOVE);
    td.mouse_hook = SetWindowsHookExW(WH_MOUSE_LL, td_mouse_proc, GetModuleHandleW(NULL), 0);
    td.key_hook = SetWindowsHookExW(WH_KEYBOARD_LL, td_key_proc, GetModuleHandleW(NULL), 0);
    td.available = td.mouse_hook && td.key_hook;
    SetEvent(td.ready);
    while (td.available && MsgWaitForMultipleObjects(1, &td.stop, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0 + 1) {
      while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
        if (message.message == WM_TIMER) {
            TrayGesture gesture = td_snapshot();
            if (gesture.pressed && (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000) ||
                GetTickCount() - gesture.started > 120000)) {
                EnterCriticalSection(&td.lock);
                td.gesture.cancelled = 1; td.gesture.pressed = 0; td.gesture.released = GetTickCount();
                LeaveCriticalSection(&td.lock);
                PostMessageW(td.owner, WM_TRAY_DRAG_INPUT, gesture.serial, WM_LBUTTONUP);
                if (td.watchdog) { KillTimer(NULL, td.watchdog); td.watchdog = 0; }
            }
        }
        TranslateMessage(&message); DispatchMessageW(&message);
      }
    }
    if (td.watchdog) KillTimer(NULL, td.watchdog);
    if (td.mouse_hook) UnhookWindowsHookEx(td.mouse_hook);
    if (td.key_hook) UnhookWindowsHookEx(td.key_hook);
    fs_leave_dpi(previous); return 0;
}
static void td_stop(void) {
    if (!td.initialized) return;
    td_cancel();
    if (td.thread) {
        SetEvent(td.stop);
        WaitForSingleObject(td.thread, INFINITE); CloseHandle(td.thread);
    }
    if (td.ready) CloseHandle(td.ready);
    if (td.stop) CloseHandle(td.stop);
    DeleteCriticalSection(&td.lock); memset(&td, 0, sizeof(td));
    if (td_log_file) { fclose(td_log_file); td_log_file = NULL; }
}
static int td_start(HWND owner) {
    memset(&td, 0, sizeof(td)); td.owner = owner;
    td.dx = GetSystemMetrics(SM_CXDRAG); if (td.dx < 1) td.dx = 4;
    td.dy = GetSystemMetrics(SM_CYDRAG); if (td.dy < 1) td.dy = 4;
    InitializeCriticalSection(&td.lock); td.initialized = 1;
    td.ready = CreateEventW(NULL, TRUE, FALSE, NULL);
    td.stop = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (td.ready && td.stop) td.thread = CreateThread(NULL, 0, td_input_thread, NULL, 0, &td.thread_id);
    if (!td.thread || WaitForSingleObject(td.ready, 5000) != WAIT_OBJECT_0 || !td.available) {
        td_stop(); return 0;
    }
    return 1;
}
static int td_native_icon_at(TrayGesture gesture) {
    typedef HRESULT (WINAPI *RectFn)(const void *, RECT *);
    struct { DWORD cbSize; HWND hWnd; UINT uID; GUID guidItem; } identifier;
    RectFn get_rect = (RectFn)(void *)GetProcAddress(GetModuleHandleW(L"shell32.dll"), "Shell_NotifyIconGetRect");
    RECT rect; HWND root; wchar_t cls[256]; HANDLE previous; int found;
    if (!get_rect || !td_source_is_tray(gesture.source)) return 0;
    root = GetAncestor(gesture.source, GA_ROOT); td_class(root, cls);
    if (!td_native_root(cls) && !td_overflow_root(cls)) return 0;
    memset(&identifier, 0, sizeof(identifier)); identifier.cbSize = sizeof(identifier);
    identifier.hWnd = nid.hWnd; identifier.uID = nid.uID;
    previous = fs_enter_dpi();
    found = SUCCEEDED(get_rect(&identifier, &rect)) && PtInRect(&rect, gesture.origin);
    fs_leave_dpi(previous); return found;
}
static void td_confirm(TrayGesture gesture) {
    if (td.confirmed == gesture.serial) return;
    td.confirmed = gesture.serial; td_get_regions(&td.initial_regions);
    td_log("confirmed", gesture);
}
static void td_finish(TrayGesture gesture) {
    TrayRegions current;
    if (gesture.pressed || td.finished == gesture.serial || td.confirmed != gesture.serial) return;
    td.finished = gesture.serial;
    if (!gesture.moved || gesture.cancelled || !IsWindow(gesture.source) || fs_preview_active) {
        td_log("drop-skipped-click-or-cancel", gesture); return;
    }
    td_get_regions(&current);
    if (td_in_regions(&current, gesture.point) || td_in_regions(&td.initial_regions, gesture.point)) {
        td_log("drop-ignored-tray-region", gesture); return;
    }
    fs_show_at_point(gesture.point);
    td_log("drop-finished", gesture);
}
static void td_input(DWORD serial) {
    TrayGesture gesture;
    if (!td.available) return;
    gesture = td_snapshot();
    if (serial != gesture.serial) return;
    if (td_source_is_tray(gesture.source)) td_log("physical-input", gesture);
    if (td.assessed != serial) {
        /* A failed/missed press is final. Never reinterpret its origin using
           icon rectangles after the shell has rearranged the tray. */
        td.assessed = serial;
        if (gesture.pressed && !gesture.moved && !gesture.cancelled && td_native_icon_at(gesture)) {
            TrayGesture current = td_snapshot();
            /* The shell query can yield while input advances on the hook
               thread. Fail closed if movement/release/new press won the race. */
            if (current.serial == serial && current.pressed && !current.moved && !current.cancelled)
                td_confirm(current);
        }
    }
    td_finish(gesture);
}
/* Return true only for a physical drag/cancellation, never for an ordinary
   click. Late shell button-up callbacks still belong to the completed gesture. */
static int td_tray_message(UINT message) {
    TrayGesture gesture;
    if (!td.available || (message != WM_LBUTTONDOWN && message != WM_LBUTTONDBLCLK && message != WM_LBUTTONUP)) return 0;
    gesture = td_snapshot();
    if (!gesture.serial) return 0;
    td_log(message == WM_LBUTTONUP ? "callback-up" : "callback-down", gesture);
    if (message == WM_LBUTTONUP && gesture.pressed) return 1; /* stale callback */
    if (td.confirmed == gesture.serial) {
        td_finish(gesture);
        return gesture.moved || gesture.cancelled;
    }
    /* Even our own delayed callback may belong to an older gesture. It can
       suppress click behavior, but cannot grant ownership to the latest one. */
    return gesture.moved || gesture.cancelled;
}

#endif
