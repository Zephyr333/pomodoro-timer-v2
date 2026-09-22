#ifndef POMODORO_FULLSCREEN_H
#define POMODORO_FULLSCREEN_H

#define ID_MENU_FULLSCREEN 348
#define ID_MENU_COLOR_FIRST 349
#define ID_MENU_COLOR_RESET 354
#define ID_MENU_FULLSCREEN_SHOW_TEXT 355
#define ID_MENU_FULLSCREEN_EXIT 356
#define ID_MENU_FULLSCREEN_SIGNATURE 357
#define ID_MENU_SIGNATURE_COLOR 358
#define ID_MENU_FULLSCREEN_SHOW_SIGNATURE 359
#define ID_MENU_FULLSCREEN_SHOW_MOUSE_TIPS 360
#define IDC_FS_COLOR_FULL 5132
#define ID_MENU_SCREEN_FIRST 6000
#define FS_MAX_MONITORS 64
#define IDD_FULLSCREEN_SIGNATURE 121
#define IDC_FS_SIGNATURE 5200
#define IDC_FS_SIGNATURE_SHOW 5201
#define IDC_FS_SIGNATURE_PREVIEW 5202
#define IDC_FS_SIGNATURE_FULL 5203
#define IDC_FS_SIGNATURE_ERROR 5204
#define IDC_FS_SIGNATURE_COLOR 5205
#define IDC_FS_SIGNATURE_PICK 5206
#define IDC_FS_SIGNATURE_FONT 5207
#define IDC_FS_SIGNATURE_SCALE 5208

#define IDD_FULLSCREEN_COLORS 120
#define IDC_FS_EDIT_FIRST 5100
#define IDC_FS_FONT_FIRST 5101
#define IDC_FS_SCALE_FIRST 5102
#define IDC_FS_PICK_FIRST 5110
#define IDC_FS_PREVIEW_FIRST 5120
#define IDC_FS_ERROR 5131
#define ID_FS_REFRESH 4002
#define ID_FS_LAYOUT 4003
#define ID_FS_HUD_TIMER 4004
#define WM_FS_COMPLETED (WM_APP + 102)
#define WM_FS_LAYOUT (WM_APP + 103)
#define WM_FS_MAINTAIN_LAYER (WM_APP + 104)

/* Persist RGB values, not Windows' byte-reversed COLORREF representation. */
static const int fs_default_colors[FS_COLOR_COUNT] = {0x7F8C98, 0x829889, 0x8C86A3, 0xA39182, 0xC79A52, 0xC4B8A8};
static const char *fs_color_keys[FS_COLOR_COUNT] = {
    "\"fullscreen_focus_color\"", "\"fullscreen_break_color\"",
    "\"fullscreen_count_up_color\"", "\"fullscreen_custom_color\"", "\"fullscreen_overtime_color\"", "\"fullscreen_signature_color\""
};

typedef struct {
    const wchar_t *display;
    const wchar_t *face;
} FullscreenFontOption;

static const FullscreenFontOption fs_mode_fonts[] = {
    {L"Segoe UI (现代简洁)", L"Segoe UI"},
    {L"Bahnschrift (数码几何)", L"Bahnschrift"},
    {L"Consolas (等宽代码)", L"Consolas"},
    {L"Arial (经典无衬线)", L"Arial"},
    {L"Georgia (复古衬线)", L"Georgia"},
    {L"楷体 (人文书法)", L"KaiTi"},
    {L"微软雅黑 (现代黑体)", L"Microsoft YaHei UI"},
};
#define FS_MODE_FONT_COUNT (sizeof(fs_mode_fonts) / sizeof(fs_mode_fonts[0]))

static const FullscreenFontOption fs_signature_fonts[] = {
    {L"楷体 (人文书法)", L"KaiTi"},
    {L"微软雅黑 (现代黑体)", L"Microsoft YaHei UI"},
    {L"宋体 (典雅报章)", L"SimSun"},
    {L"仿宋 (清秀工整)", L"FangSong"},
    {L"Segoe UI (西文现代)", L"Segoe UI"},
    {L"Georgia (西文衬线)", L"Georgia"},
    {L"Consolas (等宽代码)", L"Consolas"},
};
#define FS_SIGNATURE_FONT_COUNT (sizeof(fs_signature_fonts) / sizeof(fs_signature_fonts[0]))

typedef struct {
    const wchar_t *display;
    int scale;
} FullscreenScaleOption;

static const FullscreenScaleOption fs_scale_options[] = {
    {L"70% (紧凑)", 70},
    {L"85% (适中)", 85},
    {L"100% (标准)", 100},
    {L"120% (醒目)", 120},
    {L"140% (特大)", 140},
};
#define FS_SCALE_COUNT (sizeof(fs_scale_options) / sizeof(fs_scale_options[0]))

static const int fs_default_scales[FS_COLOR_COUNT] = {100, 100, 100, 100, 100, 100};
static const wchar_t *fs_default_fonts[FS_COLOR_COUNT] = {
    L"Segoe UI", L"Segoe UI", L"Segoe UI", L"Segoe UI", L"Segoe UI", L"KaiTi"
};
static const char *fs_scale_keys[FS_COLOR_COUNT] = {
    "\"fullscreen_focus_scale\"", "\"fullscreen_break_scale\"",
    "\"fullscreen_count_up_scale\"", "\"fullscreen_custom_scale\"", "\"fullscreen_overtime_scale\"", "\"fullscreen_signature_scale\""
};
static const char *fs_font_keys[FS_COLOR_COUNT] = {
    "\"fullscreen_focus_font\"", "\"fullscreen_break_font\"",
    "\"fullscreen_count_up_font\"", "\"fullscreen_custom_font\"", "\"fullscreen_overtime_font\"", "\"fullscreen_signature_font\""
};

typedef struct {
    wchar_t time[32];
    wchar_t status[96];
    int color;
    wchar_t font[32];
    int scale;
    int show_text;
    int signature_color;
    wchar_t signature_font[32];
    int signature_scale;
    wchar_t signature[FS_SIGNATURE_CAPACITY];
} FullscreenView;

typedef struct {
    wchar_t text[FS_SIGNATURE_CAPACITY];
    int color;
    wchar_t font[32];
    int scale;
    HFONT preview_font;
    HWND dialog;
} FullscreenSignatureDraft;
static FullscreenSignatureDraft *fs_signature_draft;
static int fs_preview_active, fs_preview_started;
static HWND fs_preview_dialog;
static int fs_preview_focus;
static FullscreenView fs_preview_view;
static void fs_end_preview(void);

/* Selection is session-only. Keep a device identity on each existing window. */
typedef struct {
    HMONITOR handle;
    MONITORINFOEXW info;
    wchar_t identity[128];
    wchar_t label[192];
} FullscreenMonitor;
typedef struct { FullscreenMonitor items[FS_MAX_MONITORS]; size_t count; } FullscreenMonitors;
static FullscreenMonitors fs_menu_monitors;

static HWND *fs_windows;
static size_t fs_count;
static int fs_active;
static int fs_building;
static TimerMode fs_completed_mode;
static volatile LONG fs_session;
static HWND fs_previous_foreground;
static FullscreenView fs_view;
static int fs_hud_visible;
static int fs_in_menu_loop;
static POINT fs_last_cursor;

#ifndef EVENT_SYSTEM_FOREGROUND
#define EVENT_SYSTEM_FOREGROUND 0x0003
#endif
#ifndef EVENT_OBJECT_SHOW
#define EVENT_OBJECT_SHOW 0x8002
#endif
#ifndef WINEVENT_OUTOFCONTEXT
#define WINEVENT_OUTOFCONTEXT 0x0000
#endif
#ifndef WINEVENT_SKIPOWNPROCESS
#define WINEVENT_SKIPOWNPROCESS 0x0002
#endif
#ifndef OBJID_WINDOW
#define OBJID_WINDOW 0x00000000
#endif
#ifndef CHILDID_SELF
#define CHILDID_SELF 0
#endif

static HWINEVENTHOOK fs_foreground_hook;
static HWINEVENTHOOK fs_show_hook;
static HWND *fs_hidden_windows;
static size_t fs_hidden_count;
static size_t fs_hidden_capacity;
static DWORD fs_last_suppress_tick;
static DWORD fs_last_topmost_tick;
static volatile LONG fs_maintain_scheduled;

static void fs_maintain_layer(int force_suppress);
static void fs_start_guard(void);
static void fs_stop_guard(void);
static void fs_restore_hidden_windows(void);
static HANDLE fs_enter_dpi(void);
static void fs_leave_dpi(HANDLE previous);


static COLORREF fs_colorref(int rgb) {
    return RGB((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255);
}

static int fs_parse_color(const wchar_t *text, int *rgb) {
    int value = 0, i;
    if (text[0] == L'#') ++text;
    if (wcslen(text) != 6) return 0;
    for (i = 0; i < 6; ++i) {
        int digit;
        if (text[i] >= L'0' && text[i] <= L'9') digit = text[i] - L'0';
        else if (text[i] >= L'a' && text[i] <= L'f') digit = text[i] - L'a' + 10;
        else if (text[i] >= L'A' && text[i] <= L'F') digit = text[i] - L'A' + 10;
        else return 0;
        value = value * 16 + digit;
    }
    *rgb = value;
    return 1;
}

static const wchar_t *fs_mode_name(TimerMode mode) {
    switch (mode) {
        case TIMER_LONG_POMODORO: return L"长番茄钟";
        case TIMER_SHORT_POMODORO: return L"短番茄钟";
        case TIMER_SHORT_BREAK: return L"短休息";
        case TIMER_LONG_BREAK: return L"长休息";
        case TIMER_COUNT_UP: return L"正计时";
        case TIMER_CUSTOM: return L"自定义计时";
        default: return L"番茄钟";
    }
}

static int fs_mode_color(TimerMode mode) {
    if (mode == TIMER_SHORT_BREAK || mode == TIMER_LONG_BREAK) return 1;
    if (mode == TIMER_COUNT_UP) return 2;
    if (mode == TIMER_CUSTOM) return 3;
    return 0;
}

static void fs_format_time(wchar_t *out, size_t capacity, int seconds, int overtime) {
    if (seconds < 0) seconds = 0;
    swprintf(out, capacity, overtime ? L"+%02d:%02d" : L"%02d:%02d", seconds / 60, seconds % 60);
}

/* Presentation only: never start, stop, credit, or advance a timer here. */
static void fs_read_view(FullscreenView *view) {
    TimerMode mode = current_timer_mode;
    int seconds = remaining_seconds;
    int active = is_running || is_paused;
    int overtime = active && is_overtime;
    const wchar_t *suffix = is_paused ? L" · 已暂停" : L"";
    if (fs_preview_active) { *view = fs_preview_view; return; }
    memset(view, 0, sizeof(*view));
    if (overtime) {
        mode = overtime_source_mode;
        seconds = overtime_seconds;
        swprintf(view->status, 96, L"%ls · 超时正计时%ls", fs_mode_name(mode), suffix);
    } else {
        if (!active && fs_completed_mode != TIMER_NONE) {
            mode = fs_completed_mode;
            seconds = 0;
            suffix = L" · 已完成";
        } else if (!active) {
            suffix = L" · 未开始";
            if (idle_mode == IDLE_BREAK) {
                mode = idle_break_is_long ? TIMER_LONG_BREAK : TIMER_SHORT_BREAK;
                seconds = (idle_break_is_long ? settings.long_break_duration : settings.short_break_duration) * 60;
            } else if (idle_mode == IDLE_COUNT_UP) {
                mode = TIMER_COUNT_UP; seconds = 0;
            } else if (idle_mode == IDLE_CUSTOM) {
                mode = TIMER_CUSTOM; seconds = settings.custom_duration * 60;
            } else {
                mode = idle_pomodoro_is_long ? TIMER_LONG_POMODORO : TIMER_SHORT_POMODORO;
                seconds = (idle_pomodoro_is_long ? settings.long_pomodoro_duration : settings.short_pomodoro_duration) * 60;
            }
        }
        swprintf(view->status, 96, L"%ls%ls", fs_mode_name(mode), suffix);
    }
    int color_index = overtime ? 4 : fs_mode_color(mode);
    fs_format_time(view->time, 32, seconds, overtime);
    view->color = settings.fullscreen_colors[color_index];
    wcscpy(view->font, settings.fullscreen_fonts[color_index][0] ? settings.fullscreen_fonts[color_index] : L"Segoe UI");
    view->scale = settings.fullscreen_scales[color_index] > 0 ? settings.fullscreen_scales[color_index] : 100;
    view->show_text = settings.fullscreen_show_text;
    view->signature_color = settings.fullscreen_colors[5];
    wcscpy(view->signature_font, settings.fullscreen_fonts[5][0] ? settings.fullscreen_fonts[5] : L"KaiTi");
    view->signature_scale = settings.fullscreen_scales[5] > 0 ? settings.fullscreen_scales[5] : 100;
    if (settings.fullscreen_show_signature) wcscpy(view->signature, settings.fullscreen_signature);
}

static void fs_refresh(void) {
    FullscreenView next;
    size_t i;
    if (!fs_active) return;
    fs_maintain_layer(0);
    if (is_running || is_paused) fs_completed_mode = TIMER_NONE;
    fs_read_view(&next);
    if (memcmp(&next, &fs_view, sizeof(next)) == 0) return;
    fs_view = next;
    for (i = 0; i < fs_count; ++i) {
        wchar_t title[160];
        swprintf(title, 160, L"%ls %ls — 番茄钟全屏", fs_view.time, fs_view.status);
        SetWindowTextW(fs_windows[i], title);
        InvalidateRect(fs_windows[i], NULL, FALSE);
        UpdateWindow(fs_windows[i]);
    }
}

static void fs_enforce_topmost(void) {
    size_t i;
    for (i = 0; i < fs_count; ++i) {
        if (fs_windows[i] && IsWindow(fs_windows[i])) {
            SetWindowPos(fs_windows[i], HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING);
        }
    }
}

static void fs_record_hidden_window(HWND hwnd) {
    size_t i;
    for (i = 0; i < fs_hidden_count; ++i) {
        if (fs_hidden_windows[i] == hwnd) return;
    }
    if (fs_hidden_count >= fs_hidden_capacity) {
        size_t new_cap = fs_hidden_capacity ? fs_hidden_capacity * 2 : 16;
        HWND *next = (HWND *)realloc(fs_hidden_windows, new_cap * sizeof(HWND));
        if (!next) return;
        fs_hidden_windows = next;
        fs_hidden_capacity = new_cap;
    }
    fs_hidden_windows[fs_hidden_count++] = hwnd;
}

static BOOL CALLBACK fs_suppress_enum_proc(HWND hwnd, LPARAM lParam) {
    DWORD process_id;
    LONG ex_style;
    RECT rect;
    POINT center;
    size_t i;
    (void)lParam;

    if (!hwnd || !IsWindowVisible(hwnd)) return TRUE;

    ex_style = GetWindowLongW(hwnd, GWL_EXSTYLE);
    if (!(ex_style & WS_EX_TOPMOST)) return TRUE;

    GetWindowThreadProcessId(hwnd, &process_id);
    if (process_id == GetCurrentProcessId()) return TRUE;

    if (!GetWindowRect(hwnd, &rect)) return TRUE;
    if ((rect.right - rect.left) < 30 || (rect.bottom - rect.top) < 20) return TRUE;

    center.x = (rect.left + rect.right) / 2;
    center.y = (rect.top + rect.bottom) / 2;

    for (i = 0; i < fs_count; ++i) {
        FullscreenMonitor *item = (FullscreenMonitor *)GetWindowLongPtrW(fs_windows[i], GWLP_USERDATA);
        if (item) {
            RECT mon_rect = item->info.rcMonitor;
            if (PtInRect(&mon_rect, center)) {
                ShowWindow(hwnd, SW_HIDE);
                fs_record_hidden_window(hwnd);
                break;
            }
        }
    }
    return TRUE;
}

static void fs_suppress_competing_windows(void) {
    HANDLE previous;
    if (!fs_count) return;
    previous = fs_enter_dpi();
    EnumWindows(fs_suppress_enum_proc, 0);
    fs_leave_dpi(previous);
}

static void fs_restore_hidden_windows(void) {
    size_t i;
    for (i = 0; i < fs_hidden_count; ++i) {
        if (IsWindow(fs_hidden_windows[i])) {
            ShowWindow(fs_hidden_windows[i], SW_SHOW);
        }
    }
    free(fs_hidden_windows);
    fs_hidden_windows = NULL;
    fs_hidden_count = 0;
    fs_hidden_capacity = 0;
}

static void fs_maintain_layer(int force_suppress) {
    DWORD now;
    if (!fs_active || !fs_count) return;
    now = GetTickCount();
    if (force_suppress || (now - fs_last_topmost_tick >= 250)) {
        fs_enforce_topmost();
        fs_last_topmost_tick = now;
    }
    if (force_suppress || (now - fs_last_suppress_tick >= 300)) {
        fs_suppress_competing_windows();
        fs_last_suppress_tick = now;
    }
}

static void CALLBACK fs_winevent_proc(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
    (void)hook; (void)event; (void)hwnd; (void)dwEventThread; (void)dwmsEventTime;
    if (idObject != (LONG)OBJID_WINDOW || idChild != CHILDID_SELF) return;
    if (!fs_active || !fs_count) return;
    if (InterlockedExchange(&fs_maintain_scheduled, 1) == 1) return;
    if (g_main_hwnd && IsWindow(g_main_hwnd)) {
        PostMessageW(g_main_hwnd, WM_FS_MAINTAIN_LAYER, 1, 0);
    } else {
        InterlockedExchange(&fs_maintain_scheduled, 0);
    }
}

static void fs_start_guard(void) {
    if (!fs_foreground_hook) {
        fs_foreground_hook = SetWinEventHook(
            EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,
            NULL, fs_winevent_proc, 0, 0,
            WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    }
    if (!fs_show_hook) {
        fs_show_hook = SetWinEventHook(
            EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW,
            NULL, fs_winevent_proc, 0, 0,
            WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    }
    fs_last_suppress_tick = 0;
    fs_last_topmost_tick = 0;
    fs_maintain_scheduled = 0;
    fs_maintain_layer(1);
}

static void fs_stop_guard(void) {
    if (fs_foreground_hook) {
        UnhookWinEvent(fs_foreground_hook);
        fs_foreground_hook = NULL;
    }
    if (fs_show_hook) {
        UnhookWinEvent(fs_show_hook);
        fs_show_hook = NULL;
    }
    fs_maintain_scheduled = 0;
    fs_restore_hidden_windows();
}

static void fs_destroy_windows(void) {
    size_t i;
    fs_stop_guard();
    if (fs_count > 0) KillTimer(fs_windows[0], ID_FS_HUD_TIMER);
    for (i = 0; i < fs_count; ++i) DestroyWindow(fs_windows[i]);
    free(fs_windows);
    fs_windows = NULL;
    fs_count = 0;
    fs_hud_visible = 0;
}

static void fs_exit(void) {
    size_t i;
    HWND foreground = GetForegroundWindow();
    int restore = 0;
    if (fs_preview_active) { fs_end_preview(); return; }
    if (!fs_active) return;
    for (i = 0; i < fs_count; ++i) if (foreground == fs_windows[i]) restore = 1;
    fs_active = 0;
    fs_hud_visible = 0;
    InterlockedIncrement(&fs_session);
    KillTimer(g_main_hwnd, ID_FS_REFRESH);
    KillTimer(g_main_hwnd, ID_FS_LAYOUT);
    fs_destroy_windows();
    fs_completed_mode = TIMER_NONE;
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    if (restore && IsWindow(fs_previous_foreground)) SetForegroundWindow(fs_previous_foreground);
}

/* Break long unspaced text too, without splitting a UTF-16 surrogate pair. */
static void fs_wrap_signature(HDC dc, int width, const wchar_t *text, wchar_t *out) {
    size_t i = 0, used = 0, start = 0;
    while (text[i]) {
        SIZE measured;
        int units = text[i] >= 0xD800 && text[i] <= 0xDBFF && text[i + 1] ? 2 : 1;
        if (text[i] == '\r' || text[i] == '\n') {
            if (text[i] == '\r' && text[i + 1] == '\n') ++i;
            out[used++] = '\n'; start = used; ++i; continue;
        }
        out[used] = text[i] == '\t' ? L' ' : text[i];
        if (units == 2) out[used + 1] = text[i + 1];
        GetTextExtentPoint32W(dc, out + start, (int)(used - start) + units, &measured);
        if (measured.cx > width && used > start) {
            memmove(out + used + 1, out + used, units * sizeof(wchar_t));
            out[used++] = '\n'; start = used;
        }
        used += units; i += units;
    }
    out[used] = 0;
}

static void fs_draw_signature_line(HDC dc, const wchar_t *text, int len, RECT rect) {
    int left_blank = 0, right_blank = 0, offset = 0;
    ABC abc;
    int first = 0, last = len - 1;
    while (first < len && (text[first] == L' ' || text[first] == L'\t' || text[first] == L'\r')) ++first;
    while (last >= first && (text[last] == L' ' || text[last] == L'\t' || text[last] == L'\r')) --last;
    if (first > last) return;

    if (!(text[first] >= 0xD800 && text[first] <= 0xDFFF)) {
        if (GetCharABCWidthsW(dc, (UINT)text[first], (UINT)text[first], &abc) && abc.abcA > 0)
            left_blank = abc.abcA;
    }
    if (!(text[last] >= 0xD800 && text[last] <= 0xDFFF)) {
        if (GetCharABCWidthsW(dc, (UINT)text[last], (UINT)text[last], &abc) && abc.abcC > 0)
            right_blank = abc.abcC;
    }
    offset = (right_blank - left_blank) / 2;
    rect.left += offset;
    rect.right += offset;
    DrawTextW(dc, text + first, last - first + 1, &rect, DT_CENTER | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
}

static void fs_draw_signature_text(HDC dc, const wchar_t *text, RECT rect) {
    const wchar_t *start;
    TEXTMETRICW tm;
    int line_height, y;

    if (!text || !*text) return;
    GetTextMetricsW(dc, &tm);
    line_height = tm.tmHeight;
    y = rect.top;

    for (start = text; *start; ) {
        const wchar_t *end = wcschr(start, L'\n');
        int len = end ? (int)(end - start) : (int)wcslen(start);
        RECT line_rect;
        line_rect.left = rect.left;
        line_rect.right = rect.right;
        line_rect.top = y;
        line_rect.bottom = y + line_height;

        fs_draw_signature_line(dc, start, len, line_rect);

        y += line_height;
        if (!end) break;
        start = end + 1;
    }
}

/* Shared by the real windows and the dialog preview, including line wrapping. */
static void fs_draw_view(HDC dc, RECT bounds, const FullscreenView *view, int hud) {
    int width = bounds.right - bounds.left, height = bounds.bottom - bounds.top;
    int is_portrait = height > width;
    int base_size = is_portrait ? max(8, min(height / 5, width * 10 / 35))
                                : max(8, min(height / 4, width / 6));
    int base_label_size = is_portrait ? max(7, min(height / 32, base_size / 8))
                                      : max(7, min(height / 32, width / 36));
    int base_sig_size = is_portrait ? max(base_label_size + 2, base_size * 20 / 100)
                                    : max(base_label_size + 2, MulDiv(base_label_size, 42, 25));

    /* The old 120% size is now the new 100% baseline for all text. */
    base_size = MulDiv(base_size, 120, 100);
    base_label_size = MulDiv(base_label_size, 120, 100);
    base_sig_size = MulDiv(base_sig_size, 120, 100);

    int scale = view->scale > 0 ? view->scale : 100;
    int size = max(8, MulDiv(base_size, scale, 100));
    int label_size = max(6, MulDiv(base_label_size, scale, 100));
    int sig_scale = view->signature_scale > 0 ? view->signature_scale : 100;
    int signature_size = max(6, MulDiv(base_sig_size, sig_scale, 100));
    const wchar_t *digit_font = view->font[0] ? view->font : L"Segoe UI";
    const wchar_t *sig_font = view->signature_font[0] ? view->signature_font : L"KaiTi";
    int signature_height = 0, status_height = 0, total, top, current_y;
    int gap, time_gap;
    HFONT digits, label, signature = NULL;
    HGDIOBJ previous;
    SIZE measured;
    TEXTMETRICW tm_digits, tm_label;
    RECT line, signature_rect;
    wchar_t wrapped[FS_SIGNATURE_CAPACITY * 2];
    int has_status;
    if (width <= 0 || height <= 0) return;
    FillRect(dc, &bounds, (HBRUSH)GetStockObject(BLACK_BRUSH));
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, fs_colorref(view->color));
    digits = CreateFontW(-size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, digit_font);
    previous = SelectObject(dc, digits);
    if (view->time[0] == L'+') {
        const wchar_t *digits_text = view->time + 1;
        SIZE size_p = {0}, size_d = {0};
        GetTextExtentPoint32W(dc, L"+", 1, &size_p);
        GetTextExtentPoint32W(dc, digits_text, (int)wcslen(digits_text), &size_d);
        int sym_w = size_d.cx + size_p.cx * 2;
        if (sym_w > width * 4 / 5) {
            size = max(6, MulDiv(size, width * 4 / 5, sym_w));
            SelectObject(dc, previous); DeleteObject(digits);
            digits = CreateFontW(-size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, digit_font);
            SelectObject(dc, digits);
        }
    } else {
        GetTextExtentPoint32W(dc, view->time, (int)wcslen(view->time), &measured);
        if (measured.cx > width * 4 / 5) {
            size = max(6, MulDiv(size, width * 4 / 5, measured.cx));
            SelectObject(dc, previous); DeleteObject(digits);
            digits = CreateFontW(-size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, digit_font);
            SelectObject(dc, digits);
        }
    }
    GetTextMetricsW(dc, &tm_digits);
    if (view->signature[0]) {
        /* Shrink long wrapped signatures to a bounded area above the HUD. */
        for (;;) {
            signature = CreateFontW(-signature_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, sig_font);
            SelectObject(dc, signature);
            signature_rect = bounds;
            signature_rect.left += width / 10; signature_rect.right -= width / 10;
            fs_wrap_signature(dc, signature_rect.right - signature_rect.left, view->signature, wrapped);
            DrawTextW(dc, wrapped, -1, &signature_rect, DT_CENTER | DT_NOPREFIX | DT_CALCRECT);
            signature_height = signature_rect.bottom - signature_rect.top;
            if (signature_height <= height / 3 || signature_size <= 1) break;
            SelectObject(dc, previous); DeleteObject(signature); --signature_size;
        }
        signature_height = min(signature_height, height / 3);
    }
    if (view->signature[0]) label_size = min(label_size, max(1, signature_size * 5 / 6));
    label = CreateFontW(-label_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, digit_font);
    SelectObject(dc, label);
    GetTextMetricsW(dc, &tm_label);

    has_status = (view->show_text && view->status[0]);
    status_height = has_status ? tm_label.tmHeight : 0;
    gap = max(8, is_portrait ? height / 35 : height / 30);
    time_gap = MulDiv(gap, 122, 100);

    total = tm_digits.tmAscent
        + (status_height ? (time_gap + status_height) : 0)
        + (signature_height ? (gap + signature_height) : 0);
    top = bounds.top + (height - total) / 2;

    line = bounds; line.top = top; line.bottom = top + tm_digits.tmHeight;
    SelectObject(dc, digits);
    if (view->time[0] == L'+') {
        const wchar_t *prefix = L"+";
        const wchar_t *digits_text = view->time + 1;
        SIZE size_p = {0}, size_d = {0};
        GetTextExtentPoint32W(dc, prefix, 1, &size_p);
        GetTextExtentPoint32W(dc, digits_text, (int)wcslen(digits_text), &size_d);

        int center_x = bounds.left + width / 2;
        int d_left = center_x - size_d.cx / 2;
        int p_left = d_left - size_p.cx;

        RECT r_prefix = line;
        r_prefix.left = p_left;
        r_prefix.right = d_left;

        RECT r_digits = line;
        r_digits.left = d_left;
        r_digits.right = d_left + size_d.cx;

        DrawTextW(dc, prefix, 1, &r_prefix, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE);
        DrawTextW(dc, digits_text, -1, &r_digits, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE);
    } else {
        DrawTextW(dc, view->time, -1, &line, DT_CENTER | DT_NOPREFIX | DT_SINGLELINE);
    }

    current_y = top + tm_digits.tmAscent;
    if (status_height) {
        current_y += time_gap;
        line = bounds; line.top = current_y; line.bottom = current_y + status_height;
        SelectObject(dc, label);
        DrawTextW(dc, view->status, -1, &line, DT_CENTER | DT_NOPREFIX | DT_SINGLELINE);
        current_y = line.bottom;
    }
    if (signature_height) {
        current_y += gap;
        line = bounds; line.top = current_y; line.bottom = current_y + signature_height;
        line.left = bounds.left + width / 10; line.right = bounds.right - width / 10;
        SelectObject(dc, signature);
        SetTextColor(dc, fs_colorref(view->signature_color));
        fs_draw_signature_text(dc, wrapped, line);
    }
    SelectObject(dc, previous);
    DeleteObject(digits); DeleteObject(label);
    if (signature) DeleteObject(signature);
    if (hud && (settings.fullscreen_show_mouse_tips || fs_preview_active)) {
        int hud_size = max(8, MulDiv(min(height / 60, width / 54), 120, 100));
        HFONT font = CreateFontW(-hud_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI");
        SelectObject(dc, font);
        line = bounds; line.left += width / 30; line.right -= width / 30;
        line.top = bounds.bottom - hud_size * 5; line.bottom = bounds.bottom - hud_size;
        SetTextColor(dc, RGB(150, 155, 160));
        DrawTextW(dc, fs_preview_active ? L"左键 / 中键 / Esc：返回编辑" :
            L"左键 / 中键 / Esc：退出全屏    右键：开始 / 停止\n中键点击托盘：全部全屏    拖动托盘图标：目标屏幕全屏", -1,
            &line, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX);
        SelectObject(dc, previous); DeleteObject(font);
    }
}

static void fs_paint(HWND hwnd) {
    PAINTSTRUCT ps;
    RECT bounds;
    HDC target = BeginPaint(hwnd, &ps), dc;
    HBITMAP bitmap, previous;
    GetClientRect(hwnd, &bounds);
    dc = CreateCompatibleDC(target);
    bitmap = CreateCompatibleBitmap(target, max(1, bounds.right), max(1, bounds.bottom));
    if (dc && bitmap) {
        previous = (HBITMAP)SelectObject(dc, bitmap);
        fs_draw_view(dc, bounds, &fs_view, fs_hud_visible);
        BitBlt(target, 0, 0, bounds.right, bounds.bottom, dc, 0, 0, SRCCOPY);
        SelectObject(dc, previous);
    } else FillRect(target, &bounds, (HBRUSH)GetStockObject(BLACK_BRUSH));
    if (bitmap) DeleteObject(bitmap);
    if (dc) DeleteDC(dc);
    EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK FullscreenWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: fs_paint(hwnd); return 0;
        case WM_DESTROY:
            free((void *)GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            return 0;
        case WM_ERASEBKGND: return 1;
        case WM_SETCURSOR:
            if (fs_in_menu_loop) {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return TRUE;
            }
            if (fs_hud_visible && (settings.fullscreen_show_mouse_tips || fs_preview_active)) {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return TRUE;
            }
            SetCursor(NULL);
            return TRUE;
        case WM_MOUSEMOVE: {
            POINT pt;
            GetCursorPos(&pt);
            if (pt.x != fs_last_cursor.x || pt.y != fs_last_cursor.y) {
                size_t i;
                fs_last_cursor = pt;
                if (settings.fullscreen_show_mouse_tips || fs_preview_active) {
                    if (!fs_hud_visible) {
                        fs_hud_visible = 1;
                        SetCursor(LoadCursor(NULL, IDC_ARROW));
                        for (i = 0; i < fs_count; ++i) InvalidateRect(fs_windows[i], NULL, FALSE);
                    }
                    if (fs_count > 0) SetTimer(fs_windows[0], ID_FS_HUD_TIMER, 1500, NULL);
                }
            }
            return 0;
        }
        case WM_TIMER:
            if (wParam == ID_FS_HUD_TIMER) {
                size_t i;
                POINT pt;
                int over_fs = 0;
                KillTimer(hwnd, ID_FS_HUD_TIMER);
                fs_hud_visible = 0;
                if (!fs_in_menu_loop) {
                    GetCursorPos(&pt);
                    for (i = 0; i < fs_count; ++i) {
                        if (fs_windows[i] && WindowFromPoint(pt) == fs_windows[i]) {
                            over_fs = 1;
                            break;
                        }
                    }
                    if (over_fs) {
                        SetCursor(NULL);
                    }
                }
                for (i = 0; i < fs_count; ++i) InvalidateRect(fs_windows[i], NULL, FALSE);
                return 0;
            }
            break;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) { fs_exit(); return 0; }
            break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
            return 0;
        case WM_RBUTTONUP:
            if (fs_preview_active) return 0;
            if (settings.fullscreen_show_mouse_tips) {
                if (!fs_hud_visible) {
                    fs_hud_visible = 1;
                }
                if (fs_count > 0) SetTimer(fs_windows[0], ID_FS_HUD_TIMER, 1500, NULL);
            }
            SendMessageW(g_main_hwnd, WM_USER + 1, 0, WM_LBUTTONUP);
            fs_refresh();
            return 0;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
        case WM_CLOSE:
            fs_exit();
            return 0;
        case WM_DISPLAYCHANGE:
        case 0x02E0: /* WM_DPICHANGED */
            if (!fs_building) PostMessageW(g_main_hwnd, WM_FS_LAYOUT, 0, 0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static HANDLE fs_enter_dpi(void) {
    typedef HANDLE (WINAPI *SetThreadDpiFn)(HANDLE);
    SetThreadDpiFn fn = (SetThreadDpiFn)(void *)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetThreadDpiAwarenessContext");
    HANDLE previous = fn ? fn((HANDLE)(INT_PTR)-4) : NULL;
    if (!previous && fn) previous = fn((HANDLE)(INT_PTR)-3);
    return previous;
}
static void fs_leave_dpi(HANDLE previous) {
    typedef HANDLE (WINAPI *SetThreadDpiFn)(HANDLE);
    SetThreadDpiFn fn = (SetThreadDpiFn)(void *)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetThreadDpiAwarenessContext");
    if (fn && previous) fn(previous);
}

static BOOL CALLBACK fs_collect_monitor(HMONITOR monitor, HDC dc, LPRECT rect, LPARAM value) {
    FullscreenMonitors *list = (FullscreenMonitors *)value;
    FullscreenMonitor *item;
    DISPLAY_DEVICEW device = {sizeof(device)};
    (void)dc; (void)rect;
    if (list->count == FS_MAX_MONITORS) return FALSE;
    item = &list->items[list->count];
    memset(item, 0, sizeof(*item));
    item->handle = monitor;
    item->info.cbSize = sizeof(item->info);
    if (!GetMonitorInfoW(monitor, (MONITORINFO *)&item->info)) return FALSE;
    if (EnumDisplayDevicesW(item->info.szDevice, 0, &device, EDD_GET_DEVICE_INTERFACE_NAME) && device.DeviceID[0])
        wcsncpy(item->identity, device.DeviceID, 127);
    else wcscpy(item->identity, item->info.szDevice);
    swprintf(item->label, 192, L"屏幕 %zu · %ld × %ld", (size_t)(list->count + 1),
        item->info.rcMonitor.right - item->info.rcMonitor.left,
        item->info.rcMonitor.bottom - item->info.rcMonitor.top);
    ++list->count;
    return TRUE;
}
static int fs_monitor_order(const void *a, const void *b) {
    const FullscreenMonitor *ma = (const FullscreenMonitor *)a;
    const FullscreenMonitor *mb = (const FullscreenMonitor *)b;
    LONG width_a = ma->info.rcMonitor.right - ma->info.rcMonitor.left;
    LONG width_b = mb->info.rcMonitor.right - mb->info.rcMonitor.left;
    LONG min_w = width_a < width_b ? width_a : width_b;
    LONG overlap_left = ma->info.rcMonitor.left > mb->info.rcMonitor.left ? ma->info.rcMonitor.left : mb->info.rcMonitor.left;
    LONG overlap_right = ma->info.rcMonitor.right < mb->info.rcMonitor.right ? ma->info.rcMonitor.right : mb->info.rcMonitor.right;
    LONG overlap_x = overlap_right - overlap_left;
    LONG cx_a = ma->info.rcMonitor.left + width_a / 2;
    LONG cx_b = mb->info.rcMonitor.left + width_b / 2;

    /* If two monitors overlap horizontally by more than 50% of the narrower screen,
       they are in the same vertical column (stacked vertically): sort top-to-bottom. */
    if (min_w > 0 && overlap_x > min_w / 2) {
        if (ma->info.rcMonitor.top != mb->info.rcMonitor.top)
            return (ma->info.rcMonitor.top < mb->info.rcMonitor.top) ? -1 : 1;
    }

    /* Otherwise, sort left-to-right primarily by horizontal center, then left coordinate. */
    if (cx_a != cx_b)
        return (cx_a < cx_b) ? -1 : 1;
    if (ma->info.rcMonitor.left != mb->info.rcMonitor.left)
        return (ma->info.rcMonitor.left < mb->info.rcMonitor.left) ? -1 : 1;
    if (ma->info.rcMonitor.top != mb->info.rcMonitor.top)
        return (ma->info.rcMonitor.top < mb->info.rcMonitor.top) ? -1 : 1;
    return wcscmp(ma->info.szDevice, mb->info.szDevice);
}
static int fs_get_monitors(FullscreenMonitors *list) {
    HANDLE previous = fs_enter_dpi();
    BOOL ok;
    size_t i;
    memset(list, 0, sizeof(*list));
    ok = EnumDisplayMonitors(NULL, NULL, fs_collect_monitor, (LPARAM)list);
    fs_leave_dpi(previous);
    qsort(list->items, list->count, sizeof(list->items[0]), fs_monitor_order);
    for (i = 0; i < list->count; ++i) {
        swprintf(list->items[i].label, 192, L"屏幕 %zu · %ld × %ld", i + 1,
            list->items[i].info.rcMonitor.right - list->items[i].info.rcMonitor.left,
            list->items[i].info.rcMonitor.bottom - list->items[i].info.rcMonitor.top);
    }
    return ok;
}
static int fs_window_index(const wchar_t *identity) {
    size_t i;
    for (i = 0; i < fs_count; ++i) {
        FullscreenMonitor *item = (FullscreenMonitor *)GetWindowLongPtrW(fs_windows[i], GWLP_USERDATA);
        if (item && !wcscmp(item->identity, identity)) return (int)i;
    }
    return -1;
}
static void fs_remove_window(size_t index) {
    HWND removed = fs_windows[index];
    int had_focus = GetForegroundWindow() == removed;
    KillTimer(removed, ID_FS_HUD_TIMER);
    memmove(fs_windows + index, fs_windows + index + 1, (fs_count - index - 1) * sizeof(HWND));
    --fs_count;
    DestroyWindow(removed);
    fs_restore_hidden_windows();
    if (fs_count) fs_maintain_layer(1);
    else fs_stop_guard();
    if (fs_count && fs_hud_visible) SetTimer(fs_windows[0], ID_FS_HUD_TIMER, 1500, NULL);
    if (had_focus && fs_count) { SetForegroundWindow(fs_windows[0]); SetFocus(fs_windows[0]); }
}
static int fs_begin(void) {
    WNDCLASSW wc = {0};
    if (fs_active) return 1;
    wc.style = CS_DBLCLKS;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpfnWndProc = FullscreenWndProc;
    wc.lpszClassName = L"PomodoroFullscreen";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return 0;
    fs_previous_foreground = GetForegroundWindow();
    fs_completed_mode = TIMER_NONE;
    InterlockedIncrement(&fs_session);
    fs_active = 1;
    fs_hud_visible = 0;
    GetCursorPos(&fs_last_cursor);
    close_toast_notification_if_open();
    fs_read_view(&fs_view);
    if (!SetTimer(g_main_hwnd, ID_FS_REFRESH, 100, NULL)) { fs_exit(); return 0; }
    fs_start_guard();
    return 1;
}
static int fs_add_screen(const FullscreenMonitor *item) {
    FullscreenMonitor *copy;
    HWND window, *next;
    HANDLE previous;
    RECT rect = item->info.rcMonitor;
    if (fs_window_index(item->identity) >= 0) return 1;
    copy = (FullscreenMonitor *)malloc(sizeof(*copy));
    if (!copy) return 0;
    *copy = *item;
    next = (HWND *)realloc(fs_windows, (fs_count + 1) * sizeof(HWND));
    if (!next) { free(copy); return 0; }
    fs_windows = next;
    previous = fs_enter_dpi();
    window = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"PomodoroFullscreen", L"番茄钟全屏",
        WS_POPUP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, GetModuleHandleW(NULL), NULL);
    fs_leave_dpi(previous);
    if (!window) { free(copy); return 0; }
    SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR)copy);
    fs_windows[fs_count++] = window;
    ShowWindow(window, SW_SHOWNOACTIVATE);
    InvalidateRect(window, NULL, FALSE);
    UpdateWindow(window);
    fs_maintain_layer(1);
    return 1;
}
/* Reconcile only selected, still-connected devices; never add newly connected screens. */
static void fs_reconcile_monitors(const FullscreenMonitors *list) {
    size_t i = 0, j;
    HANDLE previous = fs_enter_dpi();
    fs_building = 1;
    while (i < fs_count) {
        FullscreenMonitor *current = (FullscreenMonitor *)GetWindowLongPtrW(fs_windows[i], GWLP_USERDATA);
        for (j = 0; j < list->count; ++j)
            if (!wcscmp(current->identity, list->items[j].identity)) break;
        if (j == list->count) { fs_remove_window(i); continue; }
        {
            RECT actual, desired = list->items[j].info.rcMonitor;
            GetWindowRect(fs_windows[i], &actual);
            *current = list->items[j];
            if (!EqualRect(&actual, &desired))
                SetWindowPos(fs_windows[i], NULL, desired.left, desired.top, desired.right - desired.left,
                    desired.bottom - desired.top, SWP_NOACTIVATE | SWP_NOZORDER);
            InvalidateRect(fs_windows[i], NULL, FALSE);
        }
        ++i;
    }
    fs_building = 0;
    fs_leave_dpi(previous);
    if (!fs_count) fs_exit();
}
static void fs_build_windows(void) {
    FullscreenMonitors list;
    if (fs_active && fs_get_monitors(&list)) fs_reconcile_monitors(&list);
}
static void fs_show_all(void) {
    FullscreenMonitors list;
    size_t i;
    int ok = 1;
    if (!fs_get_monitors(&list)) return;
    if (fs_active && !fs_preview_active) fs_reconcile_monitors(&list);
    if (!list.count || !fs_begin()) return;
    fs_building = 1;
    for (i = 0; i < list.count; ++i) if (!fs_add_screen(&list.items[i])) ok = 0;
    fs_building = 0;
    if (!fs_count) fs_exit();
    else { SetForegroundWindow(fs_windows[0]); SetFocus(fs_windows[0]); }
    if (!ok) MessageBoxW(g_main_hwnd, L"部分屏幕无法进入全屏，请重试。计时不受影响。", L"全屏", MB_OK | MB_ICONERROR);
}
/* Idempotent drag target: add this screen, preserving every existing screen. */
static void fs_show_at_point(POINT point) {
    FullscreenMonitors monitors;
    size_t i;
    if (fs_preview_active || !fs_get_monitors(&monitors)) return;
    for (i = 0; i < monitors.count; ++i) {
        if (!PtInRect(&monitors.items[i].info.rcMonitor, point)) continue;
        if (!fs_begin()) return;
        if (!fs_add_screen(&monitors.items[i])) {
            if (!fs_count) fs_exit();
            MessageBoxW(g_main_hwnd, L"无法为此屏幕创建全屏窗口。计时不受影响。", L"全屏", MB_OK | MB_ICONERROR);
        } else {
            int index = fs_window_index(monitors.items[i].identity);
            if (index >= 0) { SetForegroundWindow(fs_windows[index]); SetFocus(fs_windows[index]); }
        }
        return;
    }
}
static void fs_toggle_screen(size_t index) {
    FullscreenMonitors current;
    size_t i;
    int existing;
    if (index >= fs_menu_monitors.count || !fs_get_monitors(&current)) return;
    /* Menu may have remained open while a screen was unplugged. Resolve identity again. */
    for (i = 0; i < current.count; ++i)
        if (!wcscmp(current.items[i].identity, fs_menu_monitors.items[index].identity)) break;
    if (i == current.count) { fs_build_windows(); return; }
    existing = fs_window_index(current.items[i].identity);
    if (existing >= 0) {
        if (fs_count == 1) fs_exit();
        else fs_remove_window((size_t)existing);
    } else if (fs_begin()) {
        if (!fs_add_screen(&current.items[i])) {
            if (!fs_count) fs_exit();
            MessageBoxW(g_main_hwnd, L"无法为此屏幕创建全屏窗口。计时不受影响。", L"全屏", MB_OK | MB_ICONERROR);
        } else {
            SetForegroundWindow(fs_windows[fs_count - 1]); SetFocus(fs_windows[fs_count - 1]);
        }
    }
}
static HMENU fs_create_menu(void) {
    HMENU menu = CreatePopupMenu();
    size_t i;
    int available = fs_get_monitors(&fs_menu_monitors) && fs_menu_monitors.count > 0;
    AppendMenuW(menu, MF_STRING | (available ? 0 : MF_GRAYED), ID_MENU_FULLSCREEN, L"全部屏幕");
    AppendMenuW(menu, MF_STRING | (fs_active ? 0 : MF_GRAYED), ID_MENU_FULLSCREEN_EXIT, L"退出全屏");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    for (i = 0; i < fs_menu_monitors.count; ++i)
        AppendMenuW(menu, MF_STRING | (fs_window_index(fs_menu_monitors.items[i].identity) >= 0 ? MF_CHECKED : 0),
            ID_MENU_SCREEN_FIRST + i, fs_menu_monitors.items[i].label);
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING | (settings.fullscreen_show_text ? MF_CHECKED : 0), ID_MENU_FULLSCREEN_SHOW_TEXT, L"显示状态");
    AppendMenuW(menu, MF_STRING | (settings.fullscreen_show_signature ? MF_CHECKED : 0), ID_MENU_FULLSCREEN_SHOW_SIGNATURE, L"显示签名");
    AppendMenuW(menu, MF_STRING | (settings.fullscreen_show_mouse_tips ? MF_CHECKED : 0), ID_MENU_FULLSCREEN_SHOW_MOUSE_TIPS, L"显示鼠标提示");
    return menu;
}

static int fs_signature_valid(const wchar_t *text) {
    size_t i, length = wcslen(text);
    int lines = 1;
    if (length >= FS_SIGNATURE_CAPACITY) return 0;
    for (i = 0; i < length; ++i) {
        unsigned ch = text[i];
        if (ch == '\r' || (ch == '\n' && (!i || text[i - 1] != '\r'))) ++lines;
        if (ch >= 0xD800 && ch <= 0xDBFF) {
            if (++i >= length || text[i] < 0xDC00 || text[i] > 0xDFFF) return 0;
        } else if (ch >= 0xDC00 && ch <= 0xDFFF) return 0;
        else if (ch < 32 && ch != '\r' && ch != '\n' && ch != '\t') return 0;
    }
    return lines <= 6;
}
static void fs_update_local_preview(HWND dialog, int id, const wchar_t *text) {
    HWND control = GetDlgItem(dialog, id);
    RECT bounds, format;
    HDC dc = GetDC(control);
    TEXTMETRICW metrics;
    UINT dpi = app_get_window_dpi(dialog);
    HGDIOBJ previous = SelectObject(dc, (HFONT)SendMessageW(control, WM_GETFONT, 0, 0));
    int padding = MulDiv(6, (int)dpi, 96), lines, content_height;
    GetTextMetricsW(dc, &metrics);
    SelectObject(dc, previous); ReleaseDC(control, dc);
    ShowScrollBar(control, SB_VERT, FALSE);
    GetClientRect(control, &bounds);
    format = bounds; InflateRect(&format, -padding, -padding);
    SendMessageW(control, EM_SETRECT, 0, (LPARAM)&format);
    SetWindowTextW(control, text);
    lines = (int)SendMessageW(control, EM_GETLINECOUNT, 0, 0);
    content_height = lines * metrics.tmHeight;
    if (content_height > format.bottom - format.top) {
        ShowScrollBar(control, SB_VERT, TRUE);
        GetClientRect(control, &format); InflateRect(&format, -padding, -padding);
    } else format.top = max(padding, (bounds.bottom - content_height) / 2);
    SendMessageW(control, EM_SETRECT, 0, (LPARAM)&format);
    SendMessageW(control, EM_LINESCROLL, 0, -32767);
    InvalidateRect(control, NULL, TRUE);
}
static INT_PTR fs_preview_brush(WPARAM wParam, int rgb) {
    SetBkColor((HDC)wParam, RGB(0, 0, 0));
    SetTextColor((HDC)wParam, fs_colorref(rgb));
    return (INT_PTR)GetStockObject(BLACK_BRUSH);
}

/* Both editors use one reversible preview session. No timer or saved setting is modified. */
static void fs_start_preview(HWND dialog, int focus, const FullscreenView *view) {
    fs_preview_dialog = dialog;
    fs_preview_focus = focus;
    fs_preview_view = *view;
    fs_preview_started = !fs_active;
    fs_preview_active = 1;
    ShowWindow(dialog, SW_HIDE);
    if (!fs_active) fs_show_all();
    else {
        fs_refresh();
        SetForegroundWindow(fs_windows[0]); SetFocus(fs_windows[0]);
    }
    if (!fs_count && fs_preview_active) fs_end_preview();
}
static void fs_end_preview(void) {
    int started = fs_preview_started;
    HWND dialog = fs_preview_dialog;
    fs_preview_active = 0;
    fs_preview_started = 0;
    fs_preview_dialog = NULL;
    if (started || !fs_count) fs_exit();
    else fs_refresh();
    if (IsWindow(dialog)) {
        ShowWindow(dialog, SW_SHOW);
        SetWindowPos(dialog, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetForegroundWindow(dialog);
        SetFocus(GetDlgItem(dialog, fs_preview_focus));
    }
}
static const wchar_t *fs_signature_sample(void) {
    return settings.fullscreen_signature[0] ? settings.fullscreen_signature : L"专注此刻";
}

static void fs_populate_font_combo(HWND combo, const FullscreenFontOption *options, size_t count, const wchar_t *current) {
    size_t i, selected = 0;
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);
    for (i = 0; i < count; ++i) {
        SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)options[i].display);
        if (!wcscmp(current, options[i].face)) selected = i;
    }
    SendMessageW(combo, CB_SETCURSEL, (WPARAM)selected, 0);
}

static void fs_populate_scale_combo(HWND combo, const FullscreenScaleOption *options, size_t count, int current) {
    size_t i, selected = 2;
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);
    for (i = 0; i < count; ++i) {
        SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)options[i].display);
        if (current == options[i].scale) selected = i;
    }
    SendMessageW(combo, CB_SETCURSEL, (WPARAM)selected, 0);
}

static void fs_read_font_combo(HWND combo, const FullscreenFontOption *options, size_t count, wchar_t *out, size_t capacity) {
    LRESULT sel = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (sel >= 0 && (size_t)sel < count) {
        wcsncpy(out, options[sel].face, capacity - 1);
        out[capacity - 1] = 0;
    }
}

static int fs_read_scale_combo(HWND combo, const FullscreenScaleOption *options, size_t count, int fallback) {
    LRESULT sel = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (sel >= 0 && (size_t)sel < count) return options[sel].scale;
    return fallback;
}

static void fs_update_signature_preview_font(HWND dialog, FullscreenSignatureDraft *draft) {
    HWND control = GetDlgItem(dialog, IDC_FS_SIGNATURE_PREVIEW);
    UINT dpi = app_get_window_dpi(dialog);
    int points = MulDiv(17, draft->scale > 0 ? draft->scale : 100, 100);
    if (draft->preview_font) DeleteObject(draft->preview_font);
    draft->preview_font = CreateFontW(-MulDiv(points, (int)dpi, 72), 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, draft->font[0] ? draft->font : L"KaiTi");
    SendMessageW(control, WM_SETFONT, (WPARAM)draft->preview_font, TRUE);
    GetDlgItemTextW(dialog, IDC_FS_SIGNATURE, draft->text, FS_SIGNATURE_CAPACITY);
    fs_update_local_preview(dialog, IDC_FS_SIGNATURE_PREVIEW, draft->text[0] ? draft->text : fs_signature_sample());
    InvalidateRect(control, NULL, TRUE);
}

static int fs_signature_update(HWND dialog, FullscreenSignatureDraft *draft) {
    int valid_text, valid_color, valid;
    wchar_t color_text[32];
    GetDlgItemTextW(dialog, IDC_FS_SIGNATURE, draft->text, FS_SIGNATURE_CAPACITY);
    valid_text = GetWindowTextLengthW(GetDlgItem(dialog, IDC_FS_SIGNATURE)) < FS_SIGNATURE_CAPACITY && fs_signature_valid(draft->text);
    GetDlgItemTextW(dialog, IDC_FS_SIGNATURE_COLOR, color_text, 32);
    valid_color = fs_parse_color(color_text, &draft->color);
    valid = valid_text && valid_color;
    EnableWindow(GetDlgItem(dialog, IDOK), valid);
    EnableWindow(GetDlgItem(dialog, IDC_FS_SIGNATURE_FULL), valid);
    if (!valid_text) SetDlgItemTextW(dialog, IDC_FS_SIGNATURE_ERROR, L"最多 500 字、6 行，请缩短内容。");
    else if (!valid_color) SetDlgItemTextW(dialog, IDC_FS_SIGNATURE_ERROR, L"请输入六位颜色代码，如 #C4B8A8。");
    else SetDlgItemTextW(dialog, IDC_FS_SIGNATURE_ERROR, L"");
    fs_update_local_preview(dialog, IDC_FS_SIGNATURE_PREVIEW, draft->text[0] ? draft->text : fs_signature_sample());
    InvalidateRect(GetDlgItem(dialog, IDC_FS_SIGNATURE_PREVIEW), NULL, TRUE);
    return valid;
}

static int fs_save_signature(const FullscreenSignatureDraft *draft) {
    wchar_t previous[FS_SIGNATURE_CAPACITY], previous_font[32];
    int previous_color, previous_scale, saved;
    if (!fs_signature_valid(draft->text)) return 0;
    if (g_settings_lock_ready) EnterCriticalSection(&g_settings_file_lock);
    wcscpy(previous, settings.fullscreen_signature);
    previous_color = settings.fullscreen_colors[5];
    previous_scale = settings.fullscreen_scales[5];
    wcscpy(previous_font, settings.fullscreen_fonts[5]);

    wcscpy(settings.fullscreen_signature, draft->text);
    settings.fullscreen_colors[5] = draft->color;
    settings.fullscreen_scales[5] = draft->scale;
    wcscpy(settings.fullscreen_fonts[5], draft->font);

    saved = save_settings();
    if (!saved) {
        wcscpy(settings.fullscreen_signature, previous);
        settings.fullscreen_colors[5] = previous_color;
        settings.fullscreen_scales[5] = previous_scale;
        wcscpy(settings.fullscreen_fonts[5], previous_font);
    }
    if (g_settings_lock_ready) LeaveCriticalSection(&g_settings_file_lock);
    if (saved) fs_refresh();
    return saved;
}

static INT_PTR CALLBACK FullscreenSignatureDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam) {
    FullscreenSignatureDraft *draft = (FullscreenSignatureDraft *)GetWindowLongPtrW(dialog, DWLP_USER);
    switch (msg) {
        case WM_INITDIALOG: {
            wchar_t color_text[8];
            draft = (FullscreenSignatureDraft *)lParam;
            draft->dialog = dialog;
            SetWindowLongPtrW(dialog, DWLP_USER, (LONG_PTR)draft);
            SendDlgItemMessageW(dialog, IDC_FS_SIGNATURE, EM_SETLIMITTEXT, 32767, 0);
            SendDlgItemMessageW(dialog, IDC_FS_SIGNATURE_COLOR, EM_SETLIMITTEXT, 7, 0);
            SetDlgItemTextW(dialog, IDC_FS_SIGNATURE, draft->text);
            swprintf(color_text, 8, L"#%06X", draft->color);
            SetDlgItemTextW(dialog, IDC_FS_SIGNATURE_COLOR, color_text);
            fs_populate_font_combo(GetDlgItem(dialog, IDC_FS_SIGNATURE_FONT), fs_signature_fonts, FS_SIGNATURE_FONT_COUNT, draft->font);
            fs_populate_scale_combo(GetDlgItem(dialog, IDC_FS_SIGNATURE_SCALE), fs_scale_options, FS_SCALE_COUNT, draft->scale);
            center_window_on_work_area(dialog);
            fs_update_signature_preview_font(dialog, draft);
            fs_signature_update(dialog, draft);
            SetWindowPos(dialog, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetFocus(GetDlgItem(dialog, IDC_FS_SIGNATURE));
            return FALSE;
        }
        case 0x02E0: /* WM_DPICHANGED */ {
            RECT *prc = (RECT *)lParam;
            if (prc) {
                SetWindowPos(dialog, NULL, prc->left, prc->top,
                    prc->right - prc->left, prc->bottom - prc->top,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            }
            if (draft) {
                fs_update_signature_preview_font(dialog, draft);
                fs_signature_update(dialog, draft);
            }
            return TRUE;
        }
        case WM_CTLCOLORSTATIC:
            if ((HWND)lParam == GetDlgItem(dialog, IDC_FS_SIGNATURE_PREVIEW))
                return fs_preview_brush(wParam, draft ? draft->color : settings.fullscreen_colors[5]);
            break;
        case WM_COMMAND:
            if (!draft) break;
            if ((LOWORD(wParam) == IDC_FS_SIGNATURE || LOWORD(wParam) == IDC_FS_SIGNATURE_COLOR) && HIWORD(wParam) == EN_CHANGE) {
                fs_signature_update(dialog, draft); return TRUE;
            }
            if (LOWORD(wParam) == IDC_FS_SIGNATURE_FONT && HIWORD(wParam) == CBN_SELCHANGE) {
                fs_read_font_combo(GetDlgItem(dialog, IDC_FS_SIGNATURE_FONT), fs_signature_fonts, FS_SIGNATURE_FONT_COUNT, draft->font, 32);
                fs_update_signature_preview_font(dialog, draft);
                return TRUE;
            }
            if (LOWORD(wParam) == IDC_FS_SIGNATURE_SCALE && HIWORD(wParam) == CBN_SELCHANGE) {
                draft->scale = fs_read_scale_combo(GetDlgItem(dialog, IDC_FS_SIGNATURE_SCALE), fs_scale_options, FS_SCALE_COUNT, 100);
                fs_update_signature_preview_font(dialog, draft);
                return TRUE;
            }
            if (LOWORD(wParam) == IDC_FS_SIGNATURE_PICK) {
                CHOOSECOLORW chooser = {0};
                static COLORREF custom[16];
                chooser.lStructSize = sizeof(chooser); chooser.hwndOwner = dialog;
                chooser.rgbResult = fs_colorref(draft->color); chooser.lpCustColors = custom;
                chooser.Flags = CC_FULLOPEN | CC_RGBINIT;
                if (ChooseColorW(&chooser)) {
                    wchar_t text[8];
                    int rgb = (GetRValue(chooser.rgbResult) << 16) | (GetGValue(chooser.rgbResult) << 8) | GetBValue(chooser.rgbResult);
                    swprintf(text, 8, L"#%06X", rgb);
                    SetDlgItemTextW(dialog, IDC_FS_SIGNATURE_COLOR, text);
                }
                return TRUE;
            }
            if (LOWORD(wParam) == IDC_FS_SIGNATURE_FULL && fs_signature_update(dialog, draft)) {
                FullscreenView view;
                fs_read_view(&view);
                view.signature_color = draft->color;
                view.signature_scale = draft->scale;
                wcscpy(view.signature_font, draft->font);
                wcscpy(view.signature, draft->text[0] ? draft->text : fs_signature_sample());
                fs_start_preview(dialog, IDC_FS_SIGNATURE, &view);
                return TRUE;
            }
            if (LOWORD(wParam) == IDOK && fs_signature_update(dialog, draft)) {
                if (!fs_save_signature(draft)) {
                    SetDlgItemTextW(dialog, IDC_FS_SIGNATURE_ERROR, L"保存失败，请重试。");
                    return TRUE;
                }
                EndDialog(dialog, IDOK); return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) { EndDialog(dialog, IDCANCEL); return TRUE; }
            break;
        case WM_CLOSE: EndDialog(dialog, IDCANCEL); return TRUE;
        case WM_DESTROY:
            if (draft && draft->preview_font) { DeleteObject(draft->preview_font); draft->preview_font = NULL; }
            break;
    }
    return FALSE;
}

static void fs_show_signature(HWND owner) {
    FullscreenSignatureDraft draft;
    memset(&draft, 0, sizeof(draft));
    wcscpy(draft.text, settings.fullscreen_signature);
    draft.color = settings.fullscreen_colors[5];
    draft.scale = settings.fullscreen_scales[5] > 0 ? settings.fullscreen_scales[5] : 100;
    wcscpy(draft.font, settings.fullscreen_fonts[5][0] ? settings.fullscreen_fonts[5] : L"KaiTi");
    fs_signature_draft = &draft;
    DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDD_FULLSCREEN_SIGNATURE), owner,
        FullscreenSignatureDlgProc, (LPARAM)&draft);
    if (fs_preview_active) fs_end_preview();
    fs_signature_draft = NULL;
    if (fs_active && fs_count) { SetForegroundWindow(fs_windows[0]); SetFocus(fs_windows[0]); }
}

static const wchar_t *fs_color_names[FS_COLOR_COUNT] = {L"番茄钟", L"休息", L"正计时", L"自定义", L"超时", L"签名"};
typedef struct {
    int index;
    int color;
    wchar_t font[32];
    int scale;
    HFONT preview_font;
} FullscreenColorDraft;

static UINT fs_color_command(int index) { return index == 5 ? ID_MENU_FULLSCREEN_SIGNATURE : ID_MENU_COLOR_FIRST + index; }
static HMENU fs_create_preview_menu(void) {
    HMENU menu = CreatePopupMenu();
    int i;
    AppendMenuW(menu, MF_STRING, fs_color_command(5), fs_color_names[5]);
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    for (i = 0; i < 5; ++i) AppendMenuW(menu, MF_STRING, fs_color_command(i), fs_color_names[i]);
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, ID_MENU_COLOR_RESET, L"恢复默认");
    return menu;
}
#define fs_create_color_menu fs_create_preview_menu

static int fs_save_color_mode(const FullscreenColorDraft *draft) {
    int previous_color, previous_scale, saved;
    wchar_t previous_font[32];
    if (draft->index < 0 || draft->index >= 5) return 0;
    if (g_settings_lock_ready) EnterCriticalSection(&g_settings_file_lock);
    previous_color = settings.fullscreen_colors[draft->index];
    previous_scale = settings.fullscreen_scales[draft->index];
    wcscpy(previous_font, settings.fullscreen_fonts[draft->index]);

    settings.fullscreen_colors[draft->index] = draft->color;
    settings.fullscreen_scales[draft->index] = draft->scale;
    wcscpy(settings.fullscreen_fonts[draft->index], draft->font);

    saved = save_settings();
    if (!saved) {
        settings.fullscreen_colors[draft->index] = previous_color;
        settings.fullscreen_scales[draft->index] = previous_scale;
        wcscpy(settings.fullscreen_fonts[draft->index], previous_font);
    }
    if (g_settings_lock_ready) LeaveCriticalSection(&g_settings_file_lock);
    if (saved) fs_refresh();
    return saved;
}

static int fs_save_palette(const int *colors) {
    int previous_colors[FS_COLOR_COUNT], previous_scales[FS_COLOR_COUNT], saved, i;
    wchar_t previous_fonts[FS_COLOR_COUNT][32];
    if (g_settings_lock_ready) EnterCriticalSection(&g_settings_file_lock);
    memcpy(previous_colors, settings.fullscreen_colors, sizeof(previous_colors));
    memcpy(previous_scales, settings.fullscreen_scales, sizeof(previous_scales));
    memcpy(previous_fonts, settings.fullscreen_fonts, sizeof(previous_fonts));

    memcpy(settings.fullscreen_colors, colors, sizeof(previous_colors));
    memcpy(settings.fullscreen_scales, fs_default_scales, sizeof(previous_scales));
    for (i = 0; i < FS_COLOR_COUNT; ++i) {
        wcscpy(settings.fullscreen_fonts[i], fs_default_fonts[i]);
    }

    saved = save_settings();
    if (!saved) {
        memcpy(settings.fullscreen_colors, previous_colors, sizeof(previous_colors));
        memcpy(settings.fullscreen_scales, previous_scales, sizeof(previous_scales));
        memcpy(settings.fullscreen_fonts, previous_fonts, sizeof(previous_fonts));
    }
    if (g_settings_lock_ready) LeaveCriticalSection(&g_settings_file_lock);
    if (saved) fs_refresh();
    return saved;
}

static const wchar_t *fs_mode_sample_time(int index) {
    switch (index) {
        case 1: return L"05:00";
        case 2: return L"03:12";
        case 3: return L"10:00";
        case 4: return L"+03:12";
        default: return L"25:00";
    }
}

static void fs_update_color_preview_font(HWND dialog, FullscreenColorDraft *draft) {
    HWND control = GetDlgItem(dialog, IDC_FS_PREVIEW_FIRST);
    UINT dpi = app_get_window_dpi(dialog);
    int points = MulDiv(31, draft->scale > 0 ? draft->scale : 100, 100);
    if (draft->preview_font) DeleteObject(draft->preview_font);
    draft->preview_font = CreateFontW(-MulDiv(points, (int)dpi, 72), 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, draft->font[0] ? draft->font : L"Segoe UI");
    SendMessageW(control, WM_SETFONT, (WPARAM)draft->preview_font, TRUE);
    fs_update_local_preview(dialog, IDC_FS_PREVIEW_FIRST, fs_mode_sample_time(draft->index));
    InvalidateRect(control, NULL, TRUE);
}

static int fs_validate_color(HWND dialog, FullscreenColorDraft *draft) {
    wchar_t text[32];
    int valid;
    GetDlgItemTextW(dialog, IDC_FS_EDIT_FIRST, text, 32);
    valid = fs_parse_color(text, &draft->color);
    EnableWindow(GetDlgItem(dialog, IDOK), valid);
    EnableWindow(GetDlgItem(dialog, IDC_FS_COLOR_FULL), valid);
    SetDlgItemTextW(dialog, IDC_FS_ERROR, valid ? L"" : L"请输入六位颜色代码，如 #7F8C98。");
    InvalidateRect(GetDlgItem(dialog, IDC_FS_PREVIEW_FIRST), NULL, TRUE);
    return valid;
}

static INT_PTR CALLBACK FullscreenColorsDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam) {
    FullscreenColorDraft *draft = (FullscreenColorDraft *)GetWindowLongPtrW(dialog, DWLP_USER);
    int id = LOWORD(wParam);
    switch (msg) {
        case WM_INITDIALOG: {
            wchar_t title[32], text[8];
            draft = (FullscreenColorDraft *)lParam;
            SetWindowLongPtrW(dialog, DWLP_USER, (LONG_PTR)draft);
            swprintf(title, 32, L"%ls", fs_color_names[draft->index]);
            SetWindowTextW(dialog, title);
            fs_populate_font_combo(GetDlgItem(dialog, IDC_FS_FONT_FIRST), fs_mode_fonts, FS_MODE_FONT_COUNT, draft->font);
            fs_populate_scale_combo(GetDlgItem(dialog, IDC_FS_SCALE_FIRST), fs_scale_options, FS_SCALE_COUNT, draft->scale);
            SendDlgItemMessageW(dialog, IDC_FS_EDIT_FIRST, EM_SETLIMITTEXT, 7, 0);
            swprintf(text, 8, L"#%06X", draft->color);
            SetDlgItemTextW(dialog, IDC_FS_EDIT_FIRST, text);
            fs_validate_color(dialog, draft);
            center_window_on_work_area(dialog);
            fs_update_color_preview_font(dialog, draft);
            SetWindowPos(dialog, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetFocus(GetDlgItem(dialog, IDC_FS_EDIT_FIRST));
            SendDlgItemMessageW(dialog, IDC_FS_EDIT_FIRST, EM_SETSEL, 0, -1);
            return FALSE;
        }
        case 0x02E0: /* WM_DPICHANGED */ {
            RECT *prc = (RECT *)lParam;
            if (prc) {
                SetWindowPos(dialog, NULL, prc->left, prc->top,
                    prc->right - prc->left, prc->bottom - prc->top,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            }
            if (draft) {
                fs_update_color_preview_font(dialog, draft);
            }
            return TRUE;
        }
        case WM_CTLCOLORSTATIC:
            if (draft && (HWND)lParam == GetDlgItem(dialog, IDC_FS_PREVIEW_FIRST))
                return fs_preview_brush(wParam, draft->color);
            break;
        case WM_COMMAND:
            if (!draft) break;
            if (id == IDC_FS_EDIT_FIRST && HIWORD(wParam) == EN_CHANGE) {
                fs_validate_color(dialog, draft); return TRUE;
            }
            if (LOWORD(wParam) == IDC_FS_FONT_FIRST && HIWORD(wParam) == CBN_SELCHANGE) {
                fs_read_font_combo(GetDlgItem(dialog, IDC_FS_FONT_FIRST), fs_mode_fonts, FS_MODE_FONT_COUNT, draft->font, 32);
                fs_update_color_preview_font(dialog, draft);
                return TRUE;
            }
            if (LOWORD(wParam) == IDC_FS_SCALE_FIRST && HIWORD(wParam) == CBN_SELCHANGE) {
                draft->scale = fs_read_scale_combo(GetDlgItem(dialog, IDC_FS_SCALE_FIRST), fs_scale_options, FS_SCALE_COUNT, 100);
                fs_update_color_preview_font(dialog, draft);
                return TRUE;
            }
            if (id == IDC_FS_COLOR_FULL && fs_validate_color(dialog, draft)) {
                FullscreenView view;
                fs_read_view(&view);
                view.color = draft->color;
                view.scale = draft->scale;
                wcscpy(view.font, draft->font);
                view.show_text = 1;
                wcscpy(view.status, fs_color_names[draft->index]);
                wcscpy(view.time, fs_mode_sample_time(draft->index));
                fs_start_preview(dialog, IDC_FS_EDIT_FIRST, &view);
                return TRUE;
            }
            if (id == IDC_FS_PICK_FIRST) {
                CHOOSECOLORW chooser = {0};
                static COLORREF custom[16];
                chooser.lStructSize = sizeof(chooser); chooser.hwndOwner = dialog;
                chooser.rgbResult = fs_colorref(draft->color); chooser.lpCustColors = custom;
                chooser.Flags = CC_FULLOPEN | CC_RGBINIT;
                if (ChooseColorW(&chooser)) {
                    wchar_t text[8];
                    int rgb = (GetRValue(chooser.rgbResult) << 16) | (GetGValue(chooser.rgbResult) << 8) | GetBValue(chooser.rgbResult);
                    swprintf(text, 8, L"#%06X", rgb);
                    SetDlgItemTextW(dialog, IDC_FS_EDIT_FIRST, text);
                }
                return TRUE;
            }
            if (id == IDOK && fs_validate_color(dialog, draft)) {
                if (!fs_save_color_mode(draft)) {
                    SetDlgItemTextW(dialog, IDC_FS_ERROR, L"保存失败，请重试。");
                    return TRUE;
                }
                EndDialog(dialog, IDOK); return TRUE;
            }
            if (id == IDCANCEL) { EndDialog(dialog, IDCANCEL); return TRUE; }
            break;
        case WM_CLOSE: EndDialog(dialog, IDCANCEL); return TRUE;
        case WM_DESTROY:
            if (draft && draft->preview_font) { DeleteObject(draft->preview_font); draft->preview_font = NULL; }
            break;
    }
    return FALSE;
}

static void fs_show_color(HWND owner, int index) {
    FullscreenColorDraft draft = {0};
    if (index < 0 || index >= FS_COLOR_COUNT) return;
    if (index == 5) { fs_show_signature(owner); return; }
    draft.index = index;
    draft.color = settings.fullscreen_colors[index];
    draft.scale = settings.fullscreen_scales[index] > 0 ? settings.fullscreen_scales[index] : 100;
    wcscpy(draft.font, settings.fullscreen_fonts[index][0] ? settings.fullscreen_fonts[index] : L"Segoe UI");
    DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDD_FULLSCREEN_COLORS), owner, FullscreenColorsDlgProc, (LPARAM)&draft);
    if (fs_preview_active) fs_end_preview();
    if (fs_active && fs_count) { SetForegroundWindow(fs_windows[0]); SetFocus(fs_windows[0]); }
}
#endif
