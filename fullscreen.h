#ifndef POMODORO_FULLSCREEN_H
#define POMODORO_FULLSCREEN_H

#define ID_MENU_FULLSCREEN 348
#define ID_MENU_COLOR_FIRST 349
#define ID_MENU_COLOR_RESET 354
#define IDD_FULLSCREEN_COLORS 120
#define IDC_FS_EDIT_FIRST 5100
#define IDC_FS_PICK_FIRST 5110
#define IDC_FS_PREVIEW_FIRST 5120
#define IDC_FS_ERROR 5131
#define ID_FS_REFRESH 4002
#define ID_FS_LAYOUT 4003
#define WM_FS_COMPLETED (WM_APP + 102)
#define WM_FS_LAYOUT (WM_APP + 103)

/* Persist RGB values, not Windows' byte-reversed COLORREF representation. */
static const int fs_default_colors[5] = {0x7F8C98, 0x829889, 0x8C86A3, 0xA39182, 0xC79A52};
static const char *fs_color_keys[5] = {
    "\"fullscreen_focus_color\"", "\"fullscreen_break_color\"",
    "\"fullscreen_count_up_color\"", "\"fullscreen_custom_color\"", "\"fullscreen_overtime_color\""
};

typedef struct {
    wchar_t time[32];
    wchar_t status[96];
    int color;
} FullscreenView;

static HWND *fs_windows;
static size_t fs_count;
static int fs_active;
static int fs_building;
static TimerMode fs_completed_mode;
static volatile LONG fs_session;
static HWND fs_previous_foreground;
static FullscreenView fs_view;

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
    fs_format_time(view->time, 32, seconds, overtime);
    view->color = settings.fullscreen_colors[overtime ? 4 : fs_mode_color(mode)];
}

static void fs_refresh(void) {
    FullscreenView next;
    size_t i;
    if (!fs_active) return;
    if (is_running || is_paused) fs_completed_mode = TIMER_NONE;
    fs_read_view(&next);
    if (memcmp(&next, &fs_view, sizeof(next)) == 0) return;
    fs_view = next;
    for (i = 0; i < fs_count; ++i) {
        wchar_t title[160];
        swprintf(title, 160, L"%ls %ls — 番茄钟全屏", fs_view.time, fs_view.status);
        SetWindowTextW(fs_windows[i], title);
        InvalidateRect(fs_windows[i], NULL, FALSE);
    }
}

static void fs_destroy_windows(void) {
    size_t i;
    for (i = 0; i < fs_count; ++i) DestroyWindow(fs_windows[i]);
    free(fs_windows);
    fs_windows = NULL;
    fs_count = 0;
}

static void fs_exit(void) {
    size_t i;
    HWND foreground = GetForegroundWindow();
    int restore = 0;
    if (!fs_active) return;
    for (i = 0; i < fs_count; ++i) if (foreground == fs_windows[i]) restore = 1;
    fs_active = 0;
    InterlockedIncrement(&fs_session);
    KillTimer(g_main_hwnd, ID_FS_REFRESH);
    KillTimer(g_main_hwnd, ID_FS_LAYOUT);
    fs_destroy_windows();
    fs_completed_mode = TIMER_NONE;
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    if (restore && IsWindow(fs_previous_foreground)) SetForegroundWindow(fs_previous_foreground);
}

static void fs_paint(HWND hwnd) {
    PAINTSTRUCT ps;
    RECT bounds, line;
    HDC target = BeginPaint(hwnd, &ps), dc;
    HBITMAP bitmap, previous_bitmap;
    HFONT digits, label, previous_font;
    SIZE measured;
    int width, height, size, label_size;
    GetClientRect(hwnd, &bounds);
    width = bounds.right; height = bounds.bottom;
    if (width <= 0 || height <= 0) { EndPaint(hwnd, &ps); return; }
    dc = CreateCompatibleDC(target);
    bitmap = CreateCompatibleBitmap(target, width, height);
    if (!dc || !bitmap) {
        FillRect(target, &bounds, (HBRUSH)GetStockObject(BLACK_BRUSH));
        if (bitmap) DeleteObject(bitmap);
        if (dc) DeleteDC(dc);
        EndPaint(hwnd, &ps); return;
    }
    previous_bitmap = (HBITMAP)SelectObject(dc, bitmap);
    FillRect(dc, &bounds, (HBRUSH)GetStockObject(BLACK_BRUSH));
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, fs_colorref(fs_view.color));
    size = min(height / 4, width / 6);
    if (size < 12) size = 12;
    digits = CreateFontW(-size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FIXED_PITCH, L"Consolas");
    previous_font = (HFONT)SelectObject(dc, digits);
    GetTextExtentPoint32W(dc, fs_view.time, (int)wcslen(fs_view.time), &measured);
    if (measured.cx > width * 4 / 5) {
        size = max(8, MulDiv(size, width * 4 / 5, measured.cx));
        SelectObject(dc, previous_font); DeleteObject(digits);
        digits = CreateFontW(-size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FIXED_PITCH, L"Consolas");
        SelectObject(dc, digits);
    }
    line = bounds; line.top = (height - size) / 2 - size / 8; line.bottom = line.top + size * 3 / 2;
    DrawTextW(dc, fs_view.time, -1, &line, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
    label_size = max(14, min(height / 32, width / 36));
    label = CreateFontW(-label_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI");
    SelectObject(dc, label);
    line.top = (height + size) / 2 + size / 8; line.bottom = line.top + label_size * 2;
    DrawTextW(dc, fs_view.status, -1, &line, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
    BitBlt(target, 0, 0, width, height, dc, 0, 0, SRCCOPY);
    SelectObject(dc, previous_font); DeleteObject(digits); DeleteObject(label);
    SelectObject(dc, previous_bitmap); DeleteObject(bitmap); DeleteDC(dc);
    EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK FullscreenWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: fs_paint(hwnd); return 0;
        case WM_ERASEBKGND: return 1;
        case WM_SETCURSOR: SetCursor(NULL); return TRUE;
        case WM_KEYDOWN: if (wParam == VK_ESCAPE) { fs_exit(); return 0; } break;
        case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN: case WM_XBUTTONDOWN:
        case WM_CLOSE: fs_exit(); return 0;
        case WM_DISPLAYCHANGE: case 0x02E0: /* WM_DPICHANGED */
            if (!fs_building) PostMessageW(g_main_hwnd, WM_FS_LAYOUT, 0, 0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static BOOL CALLBACK fs_add_monitor(HMONITOR monitor, HDC dc, LPRECT rect, LPARAM data) {
    HWND window, *next;
    (void)monitor; (void)dc; (void)data;
    next = (HWND *)realloc(fs_windows, (fs_count + 1) * sizeof(HWND));
    if (!next) return FALSE;
    fs_windows = next;
    window = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"PomodoroFullscreen", L"番茄钟全屏",
        WS_POPUP, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top,
        NULL, NULL, GetModuleHandleW(NULL), NULL);
    if (!window) return FALSE;
    fs_windows[fs_count++] = window;
    return TRUE;
}

static void fs_build_windows(void) {
    typedef HANDLE (WINAPI *SetThreadDpiFn)(HANDLE);
    SetThreadDpiFn set_dpi = (SetThreadDpiFn)(void *)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetThreadDpiAwarenessContext");
    HANDLE old_dpi = NULL;
    BOOL success;
    size_t i;
    if (!fs_active) return;
    fs_building = 1;
    fs_destroy_windows();
    /* Scope DPI awareness to the new windows, preserving existing dialogs. */
    if (set_dpi) {
        old_dpi = set_dpi((HANDLE)(INT_PTR)-4);
        if (!old_dpi) old_dpi = set_dpi((HANDLE)(INT_PTR)-3);
    }
    success = EnumDisplayMonitors(NULL, NULL, fs_add_monitor, 0);
    if (old_dpi) set_dpi(old_dpi);
    if (!success || !fs_count) {
        fs_building = 0; fs_exit();
        MessageBoxW(g_main_hwnd, L"无法为所有显示器创建全屏窗口，已退出全屏。计时不受影响。", L"全屏模式", MB_OK | MB_ICONERROR);
        return;
    }
    memset(&fs_view, 0, sizeof(fs_view));
    fs_refresh();
    for (i = 0; i < fs_count; ++i) ShowWindow(fs_windows[i], SW_SHOWNOACTIVATE);
    SetForegroundWindow(fs_windows[0]);
    SetFocus(fs_windows[0]);
    fs_building = 0;
}

static void fs_toggle(void) {
    WNDCLASSW wc = {0};
    if (fs_active) { fs_exit(); return; }
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpfnWndProc = FullscreenWndProc;
    wc.lpszClassName = L"PomodoroFullscreen";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return;
    fs_previous_foreground = GetForegroundWindow();
    fs_completed_mode = TIMER_NONE;
    InterlockedIncrement(&fs_session);
    fs_active = 1;
    close_toast_notification_if_open();
    fs_build_windows();
    if (fs_active && !SetTimer(g_main_hwnd, ID_FS_REFRESH, 100, NULL)) fs_exit();
}

static const wchar_t *fs_color_names[5] = {L"番茄钟", L"休息", L"正计时", L"自定义", L"超时"};

typedef struct { int index; int color; } FullscreenColorDraft;

static HMENU fs_create_color_menu(void) {
    HMENU menu = CreatePopupMenu();
    int i;
    for (i = 0; i < 5; ++i) AppendMenuW(menu, MF_STRING, ID_MENU_COLOR_FIRST + i, fs_color_names[i]);
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, ID_MENU_COLOR_RESET, L"恢复默认");
    return menu;
}

static int fs_save_palette(const int *colors) {
    int previous[5], saved;
    if (g_settings_lock_ready) EnterCriticalSection(&g_settings_file_lock);
    memcpy(previous, settings.fullscreen_colors, sizeof(previous));
    memcpy(settings.fullscreen_colors, colors, sizeof(previous));
    saved = save_settings();
    if (!saved) memcpy(settings.fullscreen_colors, previous, sizeof(previous));
    if (g_settings_lock_ready) LeaveCriticalSection(&g_settings_file_lock);
    if (saved) fs_refresh();
    return saved;
}

static int fs_validate_color(HWND dialog, FullscreenColorDraft *draft) {
    wchar_t text[32];
    int valid;
    GetDlgItemTextW(dialog, IDC_FS_EDIT_FIRST, text, 32);
    valid = fs_parse_color(text, &draft->color);
    EnableWindow(GetDlgItem(dialog, IDOK), valid);
    SetDlgItemTextW(dialog, IDC_FS_ERROR, valid ? L"" : L"请输入六位颜色代码，如 #7F8C98。");
    InvalidateRect(GetDlgItem(dialog, IDC_FS_PREVIEW_FIRST), NULL, FALSE);
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
            swprintf(title, 32, L"%ls颜色", fs_color_names[draft->index]);
            SetWindowTextW(dialog, title);
            SendDlgItemMessageW(dialog, IDC_FS_EDIT_FIRST, EM_SETLIMITTEXT, 7, 0);
            swprintf(text, 8, L"#%06X", draft->color);
            SetDlgItemTextW(dialog, IDC_FS_EDIT_FIRST, text);
            fs_validate_color(dialog, draft);
            center_window_on_work_area(dialog);
            SetFocus(GetDlgItem(dialog, IDC_FS_EDIT_FIRST));
            SendDlgItemMessageW(dialog, IDC_FS_EDIT_FIRST, EM_SETSEL, 0, -1);
            return FALSE;
        }
        case WM_DRAWITEM: {
            DRAWITEMSTRUCT *draw = (DRAWITEMSTRUCT *)lParam;
            HGDIOBJ old_font;
            HFONT preview_font = NULL;
            int box_h, box_w, font_size;
            const wchar_t *text;
            SIZE measured;
            if (!draft || draw->CtlID != IDC_FS_PREVIEW_FIRST) break;
            FillRect(draw->hDC, &draw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            SetBkMode(draw->hDC, TRANSPARENT);
            SetTextColor(draw->hDC, fs_colorref(draft->color));
            box_h = draw->rcItem.bottom - draw->rcItem.top;
            box_w = draw->rcItem.right - draw->rcItem.left;
            font_size = box_h * 5 / 8;
            if (font_size < 12) font_size = 12;
            preview_font = CreateFontW(-font_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FIXED_PITCH, L"Consolas");
            old_font = SelectObject(draw->hDC, preview_font ? (HGDIOBJ)preview_font : (HGDIOBJ)SendMessageW(dialog, WM_GETFONT, 0, 0));
            text = (draft->index == 4) ? L"+03:12" : L"25:00";
            if (GetTextExtentPoint32W(draw->hDC, text, (int)wcslen(text), &measured) && measured.cx > box_w * 4 / 5) {
                font_size = max(8, MulDiv(font_size, box_w * 4 / 5, measured.cx));
                SelectObject(draw->hDC, old_font);
                if (preview_font) DeleteObject(preview_font);
                preview_font = CreateFontW(-font_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FIXED_PITCH, L"Consolas");
                SelectObject(draw->hDC, preview_font ? (HGDIOBJ)preview_font : old_font);
            }
            DrawTextW(draw->hDC, text, -1, &draw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(draw->hDC, old_font);
            if (preview_font) DeleteObject(preview_font);
            return TRUE;
        }
        case WM_COMMAND:
            if (!draft) break;
            if (id == IDC_FS_EDIT_FIRST && HIWORD(wParam) == EN_CHANGE) {
                fs_validate_color(dialog, draft); return TRUE;
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
                int colors[5];
                memcpy(colors, settings.fullscreen_colors, sizeof(colors));
                colors[draft->index] = draft->color;
                if (!fs_save_palette(colors)) {
                    SetDlgItemTextW(dialog, IDC_FS_ERROR, L"保存失败，请检查数据目录后重试。");
                    return TRUE;
                }
                EndDialog(dialog, IDOK); return TRUE;
            }
            if (id == IDCANCEL) { EndDialog(dialog, IDCANCEL); return TRUE; }
            break;
        case WM_CLOSE: EndDialog(dialog, IDCANCEL); return TRUE;
    }
    return FALSE;
}

static void fs_show_color(HWND owner, int index) {
    FullscreenColorDraft draft;
    if (index < 0 || index >= 5) return;
    draft.index = index;
    draft.color = settings.fullscreen_colors[index];
    DialogBoxParamW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDD_FULLSCREEN_COLORS), owner, FullscreenColorsDlgProc, (LPARAM)&draft);
}
#endif
