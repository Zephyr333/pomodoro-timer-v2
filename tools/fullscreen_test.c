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
    const wchar_t *labels[] = {L"会话", L"切换", L"待机", L"", L"全屏", L"预览", L"", L"调整", L"时长", L"偏好", L"", L"存储", L"备份", L"", L"统计", L"", L"退出"};
    int i, j;
    wchar_t label[80];
    HMENU session = GetSubMenu(menu, 0), fullscreen = GetSubMenu(menu, 4), preview = GetSubMenu(menu, 5);
    (void)flags; (void)x; (void)y; (void)reserved; (void)owner; (void)rect;
    CHECK(GetMenuItemCount(menu) == 17, "root menu includes grouped items with separators");
    for (i = 0; i < 17; ++i) {
        HMENU child = GetSubMenu(menu, i);
        GetMenuStringW(menu, i, label, 80, MF_BYPOSITION);
        CHECK(!wcscmp(label, labels[i]), "root ordering matches grouped layout");
        if (child) for (j = 0; j < GetMenuItemCount(child); ++j)
            CHECK(GetSubMenu(child, j) == NULL, "menu depth does not exceed two levels");
    }
    CHECK(GetMenuItemCount(session) == 3, "fullscreen moved out of session menu");
    CHECK(GetMenuItemID(fullscreen, 0) == ID_MENU_FULLSCREEN, "all screens directly accessible");
    CHECK(GetMenuItemID(fullscreen, 1) == ID_MENU_FULLSCREEN_EXIT, "exit directly accessible");
    CHECK(!!(GetMenuState(fullscreen, ID_MENU_FULLSCREEN_EXIT, MF_BYCOMMAND) & MF_GRAYED) == !fs_active, "exit enabled only while fullscreen");
    GetMenuStringW(fullscreen, ID_MENU_FULLSCREEN_SHOW_TEXT, label, 80, MF_BYCOMMAND);
    CHECK(!wcscmp(label, L"显示状态"), "state visibility has concise unambiguous label");
    GetMenuStringW(fullscreen, ID_MENU_FULLSCREEN_SHOW_MOUSE_TIPS, label, 80, MF_BYCOMMAND);
    CHECK(!wcscmp(label, L"显示鼠标提示"), "mouse tips visibility has concise unambiguous label");
    CHECK(GetMenuState(GetSubMenu(menu, 9), ID_MENU_FULLSCREEN_SHOW_TEXT, MF_BYCOMMAND) == (UINT)-1, "fullscreen state removed from preferences");
    for (i = 0; i < (int)fs_menu_monitors.count; ++i)
        CHECK(!!(GetMenuState(fullscreen, ID_MENU_SCREEN_FIRST + i, MF_BYCOMMAND) & MF_CHECKED) ==
            (fs_window_index(fs_menu_monitors.items[i].identity) >= 0), "screen checkmark represents real active window");
    CHECK(GetMenuItemCount(preview) == 9, "signature, separator, five preview modes, separator and reset");
    GetMenuStringW(preview, 0, label, 80, MF_BYPOSITION);
    CHECK(!wcscmp(label, L"签名"), "signature is first preview item");
    CHECK(GetMenuItemID(preview, 0) == ID_MENU_FULLSCREEN_SIGNATURE, "signature command maps to signature dialog");
    CHECK(GetMenuItemID(preview, 1) == 0, "separator follows signature");
    for (i = 0; i < 5; ++i) {
        GetMenuStringW(preview, 2 + i, label, 80, MF_BYPOSITION);
        CHECK(!wcscmp(label, fs_color_names[i]), "preview mode item has concise label");
        CHECK(GetMenuItemID(preview, 2 + i) == fs_color_command(i), "preview command maps to intended mode");
    }
    CHECK(GetMenuItemID(preview, 7) == 0, "separator precedes reset");
    CHECK(GetMenuItemID(preview, 8) == ID_MENU_COLOR_RESET, "color reset is directly accessible");
    CHECK(GetMenuState(fullscreen, ID_MENU_FULLSCREEN_SIGNATURE, MF_BYCOMMAND) == (UINT)-1, "signature content moved out of fullscreen menu");
    CHECK(!!(GetMenuState(fullscreen, ID_MENU_FULLSCREEN_SHOW_SIGNATURE, MF_BYCOMMAND) & MF_CHECKED) == !!settings.fullscreen_show_signature,
        "signature visibility checkmark matches setting");
    for (i = 0; i < (int)fs_menu_monitors.count; ++i)
        CHECK(!wcsstr(fs_menu_monitors.items[i].label, L"主屏"), "monitor labels omit primary annotation");
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
    {
        wchar_t class_name[32];
        GetClassNameW(window, class_name, 32);
        /* Offscreen DWM captures can retain stale child pixels. Request native
           control printing for dialogs so screenshots contain the live draft. */
        if (!wcscmp(class_name, L"#32770")) {
            RECT outer;
            POINT client = {0, 0}, previous_origin;
            GetWindowRect(window, &outer);
            ClientToScreen(window, &client);
            SetViewportOrgEx(memory, outer.left - client.x, outer.top - client.y, &previous_origin);
            SendMessageW(window, WM_PRINT, (WPARAM)memory, PRF_CLIENT | PRF_CHILDREN | PRF_ERASEBKGND);
            SetViewportOrgEx(memory, previous_origin.x, previous_origin.y, NULL);
        }
    }
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
    swprintf(expected_title, 40, L"%ls", fs_color_names[dialog_index]);
    if (wcscmp(title, expected_title)) return TRUE;
    CHECK(GetDlgItem(dialog, IDC_FS_EDIT_FIRST + 3) == NULL, "dialog edits only one color");
    CHECK(GetDlgItem(dialog, IDC_FS_FONT_FIRST) != NULL, "mode dialog includes font combo");
    CHECK(GetDlgItem(dialog, IDC_FS_SCALE_FIRST) != NULL, "mode dialog includes scale combo");
    CHECK(GetNextDlgTabItem(dialog, GetDlgItem(dialog, IDC_FS_FONT_FIRST), FALSE) == GetDlgItem(dialog, IDC_FS_SCALE_FIRST), "dialog tab reaches scale combo");
    CHECK(GetNextDlgTabItem(dialog, GetDlgItem(dialog, IDC_FS_SCALE_FIRST), FALSE) == GetDlgItem(dialog, IDC_FS_EDIT_FIRST), "dialog tab reaches color edit");
    CHECK(GetNextDlgTabItem(dialog, GetDlgItem(dialog, IDC_FS_EDIT_FIRST), FALSE) == GetDlgItem(dialog, IDC_FS_PICK_FIRST), "dialog tab reaches pick button");
    CHECK(GetNextDlgTabItem(dialog, GetDlgItem(dialog, IDC_FS_PICK_FIRST), FALSE) == GetDlgItem(dialog, IDC_FS_PREVIEW_FIRST), "dialog tab reaches preview");
    CHECK(GetNextDlgTabItem(dialog, GetDlgItem(dialog, IDC_FS_PREVIEW_FIRST), FALSE) == GetDlgItem(dialog, IDC_FS_COLOR_FULL), "dialog tab reaches full preview button");
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
        FullscreenView original;
        int running = is_running, paused = is_paused, seconds = remaining_seconds;
        TimerMode mode = current_timer_mode;
        size_t count = fs_count;
        fs_read_view(&original);
        SetDlgItemTextW(dialog, IDC_FS_EDIT_FIRST, L"#123456");
        SendMessageW(GetDlgItem(dialog, IDC_FS_FONT_FIRST), CB_SETCURSEL, 1, 0);
        SendMessageW(dialog, WM_COMMAND, MAKEWPARAM(IDC_FS_FONT_FIRST, CBN_SELCHANGE), (LPARAM)GetDlgItem(dialog, IDC_FS_FONT_FIRST));
        SendMessageW(GetDlgItem(dialog, IDC_FS_SCALE_FIRST), CB_SETCURSEL, 3, 0);
        SendMessageW(dialog, WM_COMMAND, MAKEWPARAM(IDC_FS_SCALE_FIRST, CBN_SELCHANGE), (LPARAM)GetDlgItem(dialog, IDC_FS_SCALE_FIRST));
        {
            RECT preview_rect = {0};
            SendMessageW(GetDlgItem(dialog, IDC_FS_PREVIEW_FIRST), EM_GETRECT, 0, (LPARAM)&preview_rect);
            CHECK(preview_rect.top >= 10, "color mode preview remains vertically centered after changing font and scale");
        }
        SendMessageW(dialog, WM_COMMAND, IDC_FS_COLOR_FULL, 0);
        CHECK(fs_preview_active && !IsWindowVisible(dialog), "color editor uses shared fullscreen preview");
        CHECK(fs_count >= count, "color preview preserves coverage");
        if (dialog_index == 5) {
            CHECK(fs_view.signature_color == 0x123456 && fs_view.signature[0], "signature color preview shows visible sample even with empty content");
            CHECK(fs_view.color == original.color, "signature color preview does not recolor timer");
        } else {
            CHECK(fs_view.color == 0x123456 && !wcscmp(fs_view.status, fs_color_names[dialog_index]), "color preview shows selected state rather than active timer state");
            CHECK(!wcscmp(fs_view.font, L"Bahnschrift"), "color preview uses draft font");
            CHECK(fs_view.scale == 120, "color preview uses draft scale");
            CHECK(fs_view.signature_color == original.signature_color, "state color preview preserves independent signature color");
        }
        if (fs_count) SendMessageW(fs_windows[0], WM_KEYDOWN, VK_ESCAPE, 0);
        CHECK(!fs_preview_active && IsWindowVisible(dialog) && fs_count == count, "color preview returns to original editor and coverage");
        {
            RECT preview_rect = {0};
            SendMessageW(GetDlgItem(dialog, IDC_FS_PREVIEW_FIRST), EM_GETRECT, 0, (LPARAM)&preview_rect);
            CHECK(preview_rect.top >= 10, "color mode preview remains vertically centered after returning from fullscreen preview");
        }
        CHECK(is_running == running && is_paused == paused && remaining_seconds == seconds && current_timer_mode == mode, "color preview does not mutate timer state");
        CHECK(settings.fullscreen_colors[dialog_index] == fs_default_colors[dialog_index], "color preview does not save draft");
        if (dialog_index == 5) capture_window(dialog, L"signature-color-dialog.bmp");
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

static void signature_input(HWND dialog, const wchar_t *text) {
    SendDlgItemMessageW(dialog, IDC_FS_SIGNATURE, EM_SETSEL, 0, -1);
    SendDlgItemMessageW(dialog, IDC_FS_SIGNATURE, EM_REPLACESEL, TRUE, (LPARAM)text);
}
static int signature_case;
static const wchar_t *signature_sample = L"把今天做好。\r\n\r\n一步一步，慢慢来。";
static BOOL CALLBACK exercise_signature(HWND dialog, LPARAM unused) {
    wchar_t title[80], text[FS_SIGNATURE_CAPACITY + 10];
    int before_running = is_running, before_seconds = remaining_seconds;
    size_t before_count = fs_count;
    HWND before_window = fs_count ? fs_windows[0] : NULL;
    (void)unused;
    GetWindowTextW(dialog, title, 80);
    if (wcscmp(title, L"签名")) return TRUE;
    CHECK(GetDlgItem(dialog, IDC_FS_SIGNATURE_SHOW) == NULL, "visibility switch belongs in menu, not content editor");
    CHECK(GetDlgItem(dialog, IDC_FS_SIGNATURE_COLOR) != NULL, "merged signature dialog includes color input");
    CHECK(GetDlgItem(dialog, IDC_FS_SIGNATURE_PICK) != NULL, "merged signature dialog includes pick button");
    GetDlgItemTextW(dialog, IDC_FS_SIGNATURE, text, FS_SIGNATURE_CAPACITY);
    CHECK(!wcscmp(text, settings.fullscreen_signature), "editor starts with saved signature");
    CHECK(GetWindowLongW(GetDlgItem(dialog, IDC_FS_SIGNATURE), GWL_STYLE) & ES_WANTRETURN, "Enter inserts newline in editor");
    CHECK(GetWindowLongW(GetDlgItem(dialog, IDC_FS_SIGNATURE), GWL_STYLE) & ES_MULTILINE, "editor supports multiple lines");
    {
        RECT client, button;
        GetClientRect(dialog, &client);
        GetWindowRect(GetDlgItem(dialog, IDCANCEL), &button);
        MapWindowPoints(NULL, dialog, (POINT *)&button, 2);
        CHECK(button.left >= 0 && button.top >= 0 && button.right <= client.right && button.bottom <= client.bottom,
            "dialog action buttons fit inside the client area");
    }
    signature_input(dialog, L"1\r\n2\r\n3\r\n4\r\n5\r\n6\r\n7");
    CHECK(!IsWindowEnabled(GetDlgItem(dialog, IDOK)), "seven lines rejected visibly");
    {
        int i;
        for (i = 0; i < FS_SIGNATURE_CAPACITY; ++i) text[i] = L'字';
        text[FS_SIGNATURE_CAPACITY] = 0;
        signature_input(dialog, text);
        CHECK(!IsWindowEnabled(GetDlgItem(dialog, IDOK)), "overlength paste cannot save truncated draft");
        {
            HWND preview = GetDlgItem(dialog, IDC_FS_SIGNATURE_PREVIEW);
            LOGFONTW font;
            GetObjectW((HFONT)SendMessageW(preview, WM_GETFONT, 0, 0), sizeof(font), &font);
            CHECK(abs(font.lfHeight) >= 18, "long local preview never shrinks text");
            CHECK(GetWindowLongW(preview, GWL_STYLE) & WS_VSCROLL, "long local preview offers scrolling");
            SendMessageW(preview, EM_LINESCROLL, 0, 10);
            CHECK(SendMessageW(preview, EM_GETFIRSTVISIBLELINE, 0, 0) > 0, "local preview can scroll to later lines");
        }
    }
    signature_input(dialog, signature_sample);
    CHECK(IsWindowEnabled(GetDlgItem(dialog, IDOK)), "multiline Chinese accepted");
    if (signature_case == 2) {
        SetDlgItemTextW(dialog, IDC_FS_SIGNATURE_COLOR, L"#123456");
        SendMessageW(dialog, WM_COMMAND, MAKEWPARAM(IDC_FS_SIGNATURE_COLOR, EN_CHANGE), (LPARAM)GetDlgItem(dialog, IDC_FS_SIGNATURE_COLOR));
        SendMessageW(GetDlgItem(dialog, IDC_FS_SIGNATURE_FONT), CB_SETCURSEL, 1, 0);
        SendMessageW(dialog, WM_COMMAND, MAKEWPARAM(IDC_FS_SIGNATURE_FONT, CBN_SELCHANGE), (LPARAM)GetDlgItem(dialog, IDC_FS_SIGNATURE_FONT));
        SendMessageW(GetDlgItem(dialog, IDC_FS_SIGNATURE_SCALE), CB_SETCURSEL, 3, 0);
        SendMessageW(dialog, WM_COMMAND, MAKEWPARAM(IDC_FS_SIGNATURE_SCALE, CBN_SELCHANGE), (LPARAM)GetDlgItem(dialog, IDC_FS_SIGNATURE_SCALE));
        {
            RECT preview_rect = {0};
            SendMessageW(GetDlgItem(dialog, IDC_FS_SIGNATURE_PREVIEW), EM_GETRECT, 0, (LPARAM)&preview_rect);
            CHECK(preview_rect.top >= 6, "signature preview remains vertically centered or padded after changing font and scale");
        }
    }
    CHECK(fs_signature_draft && !wcscmp(fs_signature_draft->text, signature_sample), "edit notification updates live preview draft");
    {
        LOGFONTW font;
        HWND preview = GetDlgItem(dialog, IDC_FS_SIGNATURE_PREVIEW);
        GetWindowTextW(preview, text, FS_SIGNATURE_CAPACITY);
        CHECK(!wcscmp(text, signature_sample), "local preview contains only the signature, not timer thumbnail");
        GetObjectW((HFONT)SendMessageW(preview, WM_GETFONT, 0, 0), sizeof(font), &font);
        CHECK(abs(font.lfHeight) >= 18, "local preview uses legible fixed-size text");
    }
    pump(100);
    CHECK(!settings.fullscreen_signature[0], "typing does not change saved settings");
    RedrawWindow(dialog, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    capture_window(dialog, L"signature-dialog.bmp");
    SendMessageW(dialog, WM_COMMAND, IDC_FS_SIGNATURE_FULL, 0);
    CHECK(fs_preview_active && fs_count, "fullscreen preview opens");
    CHECK(!IsWindowVisible(dialog), "editor hides during preview");
    CHECK(!wcscmp(fs_view.signature, signature_sample), "fullscreen uses draft signature");
    if (signature_case == 2) {
        CHECK(fs_view.signature_color == 0x123456, "fullscreen preview uses draft signature color");
        CHECK(!wcscmp(fs_view.signature_font, L"Microsoft YaHei UI"), "fullscreen preview uses draft signature font");
        CHECK(fs_view.signature_scale == 120, "fullscreen preview uses draft signature scale");
    }
    if (before_count) CHECK(fs_count == before_count && fs_windows[0] == before_window, "preview preserves selected screens and windows");
    if (fs_count) {
        capture_window(fs_windows[0], L"signature-fullscreen.bmp");
        {
            size_t screen;
            for (screen = 0; screen < fs_count; ++screen) {
                RECT bounds;
                GetWindowRect(fs_windows[screen], &bounds);
                if (bounds.bottom - bounds.top > bounds.right - bounds.left)
                    capture_window(fs_windows[screen], L"signature-portrait.bmp");
            }
        }
        SendMessageW(fs_windows[0], WM_RBUTTONUP, 0, 0);
        CHECK(is_running == before_running && remaining_seconds == before_seconds, "preview right click does not change timer");
        SendMessageW(fs_windows[0], WM_KEYDOWN, VK_ESCAPE, 0);
    }
    CHECK(!fs_preview_active && IsWindowVisible(dialog), "Escape returns to editor");
    {
        RECT preview_rect = {0};
        SendMessageW(GetDlgItem(dialog, IDC_FS_SIGNATURE_PREVIEW), EM_GETRECT, 0, (LPARAM)&preview_rect);
        CHECK(preview_rect.top >= 6, "signature preview remains vertically centered or padded after returning from fullscreen preview");
    }
    CHECK(fs_count == before_count, "preview restores original coverage");
    CHECK(!settings.fullscreen_signature[0], "preview never saves the draft");
    SendMessageW(dialog, WM_COMMAND, IDC_FS_SIGNATURE_FULL, 0);
    CHECK(GetNextDlgTabItem(dialog, GetDlgItem(dialog, IDC_FS_SIGNATURE), FALSE) == GetDlgItem(dialog, IDC_FS_SIGNATURE_FONT), "keyboard tab reaches font combo");
    CHECK(GetNextDlgTabItem(dialog, GetDlgItem(dialog, IDC_FS_SIGNATURE_FONT), FALSE) == GetDlgItem(dialog, IDC_FS_SIGNATURE_SCALE), "keyboard tab reaches scale combo");
    CHECK(GetNextDlgTabItem(dialog, GetDlgItem(dialog, IDC_FS_SIGNATURE_SCALE), FALSE) == GetDlgItem(dialog, IDC_FS_SIGNATURE_COLOR), "keyboard tab reaches color edit");
    CHECK(GetNextDlgTabItem(dialog, GetDlgItem(dialog, IDC_FS_SIGNATURE_COLOR), FALSE) == GetDlgItem(dialog, IDC_FS_SIGNATURE_PICK), "keyboard tab reaches pick button");
    CHECK(GetNextDlgTabItem(dialog, GetDlgItem(dialog, IDC_FS_SIGNATURE_PICK), FALSE) == GetDlgItem(dialog, IDC_FS_SIGNATURE_PREVIEW), "keyboard tab reaches scrollable preview");
    CHECK(GetNextDlgTabItem(dialog, GetDlgItem(dialog, IDC_FS_SIGNATURE_PREVIEW), FALSE) == GetDlgItem(dialog, IDC_FS_SIGNATURE_FULL), "keyboard tab reaches fullscreen preview button");
    SendMessageW(dialog, WM_COMMAND, signature_case == 2 ? IDOK : IDCANCEL, 0);
    return FALSE;
}
static VOID CALLBACK signature_timer(HWND hwnd, UINT msg, UINT_PTR id, DWORD tick) {
    (void)hwnd; (void)msg; (void)tick;
    KillTimer(NULL, id);
    EnumThreadWindows(GetCurrentThreadId(), exercise_signature, 0);
}
static void test_screen_selection(void) {
    HWND first;
    int created, destroyed;
    FullscreenMonitors snapshot;
    fs_exit();
    choose_menu(ID_MENU_SCREEN_FIRST);
    CHECK(fs_count == 1 && fs_active, "first menu screen enters alone");
    if (!fs_count) return;
    first = fs_windows[0]; created = overlay_created; destroyed = overlay_destroyed;
    if (fs_menu_monitors.count > 1) {
        choose_menu(ID_MENU_SCREEN_FIRST + 1);
        CHECK(fs_count == 2 && fs_windows[0] == first, "second screen adds to first without replacing it");
        CHECK(overlay_created == created + 1 && overlay_destroyed == destroyed, "adding second screen has no old-window destruction");
        choose_menu(ID_MENU_SCREEN_FIRST + 1);
        CHECK(fs_count == 1 && fs_windows[0] == first, "unchecking screen removes only that screen");
    }
    SendMessageW(g_main_hwnd, WM_DISPLAYCHANGE, 32, 0); pump(300);
    CHECK(fs_count == 1 && fs_windows[0] == first, "layout message preserves selection and existing window");
    SendMessageW(g_main_hwnd, WM_USER + 1, 0, WM_MBUTTONUP);
    CHECK(fs_count == (size_t)monitor_count && fs_windows[0] == first, "tray middle click fills all screens preserving first window");
    created = overlay_created; destroyed = overlay_destroyed;
    SendMessageW(g_main_hwnd, WM_USER + 1, 0, WM_MBUTTONUP);
    CHECK(fs_count == (size_t)monitor_count && overlay_created == created && overlay_destroyed == destroyed, "repeated tray middle click is idempotent");
    fs_get_monitors(&snapshot);
    if (snapshot.count > 1) {
        --snapshot.count;
        fs_reconcile_monitors(&snapshot);
        CHECK(fs_count == snapshot.count, "simulated unplug removes only disconnected screen");
        fs_build_windows();
        CHECK(fs_count == snapshot.count, "reconnected screen does not automatically join");
    }
    memset(&snapshot, 0, sizeof(snapshot));
    fs_reconcile_monitors(&snapshot);
    CHECK(!fs_active && !fs_count, "loss of last selected screen exits fullscreen");
    choose_menu(ID_MENU_SCREEN_FIRST); choose_menu(ID_MENU_SCREEN_FIRST);
    CHECK(!fs_active && !fs_count, "unchecking last screen exits fullscreen");
    choose_menu(ID_MENU_SCREEN_FIRST);
    if (fs_count) SendMessageW(fs_windows[0], WM_MBUTTONDOWN, 0, 0);
    CHECK(!fs_active && !fs_count, "middle click on overlay exits all screens");
}
static void test_monitor_ordering(void) {
    FullscreenMonitor monitors[4];

    /* Test 1: Horizontal side-by-side monitors out of order (DISPLAY1 at X=0, DISPLAY2 at X=2720, DISPLAY3 at X=1920) */
    memset(monitors, 0, sizeof(monitors));
    wcscpy(monitors[0].info.szDevice, L"\\\\.\\DISPLAY1");
    SetRect(&monitors[0].info.rcMonitor, 0, 0, 1440, 900);

    wcscpy(monitors[1].info.szDevice, L"\\\\.\\DISPLAY2");
    SetRect(&monitors[1].info.rcMonitor, 2720, -120, 4914, 1114);

    wcscpy(monitors[2].info.szDevice, L"\\\\.\\DISPLAY3");
    SetRect(&monitors[2].info.rcMonitor, 1920, -40, 2720, 1240);

    qsort(monitors, 3, sizeof(monitors[0]), fs_monitor_order);
    CHECK(!wcscmp(monitors[0].info.szDevice, L"\\\\.\\DISPLAY1"), "spatial order: leftmost screen is first");
    CHECK(!wcscmp(monitors[1].info.szDevice, L"\\\\.\\DISPLAY3"), "spatial order: middle screen is second");
    CHECK(!wcscmp(monitors[2].info.szDevice, L"\\\\.\\DISPLAY2"), "spatial order: rightmost screen is third");

    /* Test 2: Vertically stacked monitors (stacked in same column) */
    memset(monitors, 0, sizeof(monitors));
    wcscpy(monitors[0].info.szDevice, L"\\\\.\\DISPLAY_BOT");
    SetRect(&monitors[0].info.rcMonitor, 0, 0, 1920, 1080);

    wcscpy(monitors[1].info.szDevice, L"\\\\.\\DISPLAY_TOP");
    SetRect(&monitors[1].info.rcMonitor, 0, -1080, 1920, 0);

    qsort(monitors, 2, sizeof(monitors[0]), fs_monitor_order);
    CHECK(!wcscmp(monitors[0].info.szDevice, L"\\\\.\\DISPLAY_TOP"), "spatial order: top stacked screen is first");
    CHECK(!wcscmp(monitors[1].info.szDevice, L"\\\\.\\DISPLAY_BOT"), "spatial order: bottom stacked screen is second");

    /* Test 3: Mixed arrangement (left single screen, right stacked screens) */
    memset(monitors, 0, sizeof(monitors));
    wcscpy(monitors[0].info.szDevice, L"\\\\.\\DISPLAY_RIGHT_BOT");
    SetRect(&monitors[0].info.rcMonitor, 1920, 0, 3840, 1080);

    wcscpy(monitors[1].info.szDevice, L"\\\\.\\DISPLAY_LEFT");
    SetRect(&monitors[1].info.rcMonitor, 0, 0, 1920, 1080);

    wcscpy(monitors[2].info.szDevice, L"\\\\.\\DISPLAY_RIGHT_TOP");
    SetRect(&monitors[2].info.rcMonitor, 1920, -1080, 3840, 0);

    qsort(monitors, 3, sizeof(monitors[0]), fs_monitor_order);
    CHECK(!wcscmp(monitors[0].info.szDevice, L"\\\\.\\DISPLAY_LEFT"), "spatial order: left screen is first");
    CHECK(!wcscmp(monitors[1].info.szDevice, L"\\\\.\\DISPLAY_RIGHT_TOP"), "spatial order: top-right screen is second");
    CHECK(!wcscmp(monitors[2].info.szDevice, L"\\\\.\\DISPLAY_RIGHT_BOT"), "spatial order: bottom-right screen is third");
}
static void test_signature(void) {
    FullscreenView view;
    FullscreenSignatureDraft draft = {0};
    wchar_t previous_path[MAX_PATH], unicode[FS_SIGNATURE_CAPACITY];
    MSG message;
    const char *sample;
    int i;
    fs_exit(); settings.fullscreen_signature[0] = 0; settings.fullscreen_show_signature = 1;
    signature_case = 1;
    SetTimer(NULL, 0, 100, signature_timer); choose_menu(ID_MENU_FULLSCREEN_SIGNATURE);
    CHECK(!settings.fullscreen_signature[0], "cancel discards signature draft");
    choose_menu(ID_MENU_SCREEN_FIRST);
    SetTimer(NULL, 0, 100, signature_timer); choose_menu(ID_MENU_FULLSCREEN_SIGNATURE);
    CHECK(fs_count == 1 && !settings.fullscreen_signature[0], "cancel preserves existing selected screen");
    fs_exit();
    choose_menu(ID_MENU_FULLSCREEN_SHOW_SIGNATURE);
    CHECK(!settings.fullscreen_show_signature, "signature display toggles directly in menu");
    signature_case = 2;
    SetTimer(NULL, 0, 100, signature_timer); choose_menu(ID_MENU_FULLSCREEN_SIGNATURE);
    load_settings();
    CHECK(!wcscmp(settings.fullscreen_signature, signature_sample), "signature save survives reload with empty lines");
    CHECK(settings.fullscreen_colors[5] == 0x123456, "signature color save survives reload");
    CHECK(!wcscmp(settings.fullscreen_fonts[5], L"Microsoft YaHei UI"), "signature font save survives reload");
    CHECK(settings.fullscreen_scales[5] == 120, "signature scale save survives reload");
    CHECK(!settings.fullscreen_show_signature, "editing hidden signature preserves display switch");
    choose_menu(ID_MENU_FULLSCREEN_SHOW_SIGNATURE);
    settings.fullscreen_show_text = 0;
    fs_read_view(&view);
    CHECK(!view.show_text && !wcscmp(view.signature, signature_sample), "signature is independent of state visibility");
    fs_show_all();
    if (fs_count) capture_window(fs_windows[0], L"signature-no-status.bmp");
    settings.fullscreen_show_signature = 0; fs_refresh();
    CHECK(!fs_view.signature[0] && !fs_view.show_text, "both texts can be hidden independently");
    if (fs_count) capture_window(fs_windows[0], L"time-only.bmp");
    fs_exit();
    settings.fullscreen_show_signature = 1;
    wcscpy(draft.text, L"中文\r\n\"fullscreen_show_text\":0 \\ & \xD83D\xDE80");
    draft.color = 0x8899AA;
    CHECK(fs_save_signature(&draft), "save escaped Unicode signature");
    load_settings();
    CHECK(!wcscmp(settings.fullscreen_signature, draft.text), "quotes backslash ampersand emoji roundtrip exactly");
    CHECK(settings.fullscreen_colors[5] == 0x8899AA, "signature color roundtrip exactly");
    settings.fullscreen_colors[5] = fs_default_colors[5];
    for (i = 0; i < 500; ++i) draft.text[i] = L'字'; draft.text[500] = 0;
    CHECK(fs_save_signature(&draft), "500 characters save successfully");
    load_settings(); CHECK(wcslen(settings.fullscreen_signature) == 500, "configuration larger than old 2KB buffer survives reload");
    settings.fullscreen_show_text = 1;
    fs_show_all();
    if (fs_count) {
        size_t screen;
        for (screen = 0; screen < fs_count; ++screen) {
            RECT bounds;
            GetWindowRect(fs_windows[screen], &bounds);
            if (bounds.bottom - bounds.top > bounds.right - bounds.left)
                capture_window(fs_windows[screen], L"signature-long-portrait.bmp");
        }
    }
    fs_exit();

    wcscpy(previous_path, g_settings_tmp_path);
    wcscpy(g_settings_tmp_path, g_data_dir); /* A directory cannot be opened as a file. */
    wcscpy(draft.text, L"不可保存的草稿");
    CHECK(!fs_save_signature(&draft), "write failure reported");
    CHECK(wcslen(settings.fullscreen_signature) == 500 && settings.fullscreen_show_signature, "failed signature save restores live settings");
    wcscpy(g_settings_tmp_path, previous_path);
    while (PeekMessageW(&message, g_main_hwnd, WM_SETTINGS_SAVE_FAILED, WM_SETTINGS_SAVE_FAILED, PM_REMOVE)) { }
    sample = "\"中文\\n\\\"quoted\\\"\\\\\"";
    CHECK(settings_json_string(&sample, unicode, FS_SIGNATURE_CAPACITY) && !wcscmp(unicode, L"中文\n\"quoted\"\\"), "raw UTF8 signature decodes");
    CHECK(extract_json_int("{\"fullscreen_signature\":\"\\\"fullscreen_show_text\\\":0\",\"fullscreen_show_text\":1}", "\"fullscreen_show_text\"", -1) == 1, "embedded setting name cannot override real key");
    CHECK(!settings_json_valid("{\"pomodoro_duration\":42,\"pomodoro_count\":3,\"fullscreen_signature\":\"truncated"), "truncated settings rejected for recovery");
    CHECK(!settings_json_valid("{\"pomodoro_duration\":42,\"pomodoro_count\":3}garbage"), "trailing corruption rejected");
    CHECK(settings_json_valid("\xEF\xBB\xBF{\"pomodoro_duration\":42,\"pomodoro_count\":3}"), "UTF8 BOM settings remain compatible");
    settings.fullscreen_signature[0] = 0; settings.fullscreen_show_text = 1; settings.fullscreen_show_signature = 1; settings.fullscreen_show_mouse_tips = 1;
    save_settings();
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
    refresh_timer_icon_by_state(g_main_hwnd); /* Include the persistent tray icon in both GDI snapshots. */
    gdi_before = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    for (i = 0; i < (stress ? 8 : 1); ++i) {
        fs_show_all(); pump(20);
        if (fs_count) SendMessageW(fs_windows[0], WM_LBUTTONDOWN, MK_LBUTTON, 0);
    }
    CHECK(!fs_active && !fs_count, "click exits all overlays");

    test_screen_selection();
    test_monitor_ordering();
    test_signature();

    /* Test right click toggle and mouse hover HUD prompt in fullscreen */
    fs_show_all(); pump(50);
    CHECK(fs_active && !is_running, "re-entered fullscreen in idle state");
    if (fs_count) {
        --fs_last_cursor.x;
        SendMessageW(fs_windows[0], WM_MOUSEMOVE, 0, MAKELPARAM(100, 100));
        pump(20);
        CHECK(fs_hud_visible, "mouse move in fullscreen activates HUD prompt");
        capture_window(fs_windows[0], L"fullscreen-hud.bmp");
        SendMessageW(fs_windows[0], WM_RBUTTONUP, 0, MAKELPARAM(100, 100));
        pump(100);
        CHECK(fs_active && is_running, "right click in fullscreen starts timer without exiting");
        SendMessageW(fs_windows[0], WM_RBUTTONUP, 0, MAKELPARAM(100, 100));
        pump(100);
        CHECK(fs_active && !is_running, "right click again stops timer and stays fullscreen");

        /* Test rapid right-click double-click toggles without delay */
        SendMessageW(fs_windows[0], WM_RBUTTONDOWN, 0, 0);
        SendMessageW(fs_windows[0], WM_RBUTTONUP, 0, 0);
        CHECK(fs_active && is_running, "rapid click 1 starts timer");
        SendMessageW(fs_windows[0], WM_RBUTTONDBLCLK, 0, 0);
        SendMessageW(fs_windows[0], WM_RBUTTONUP, 0, 0);
        CHECK(fs_active && !is_running, "rapid click 2 stops timer");
        SendMessageW(fs_windows[0], WM_RBUTTONDOWN, 0, 0);
        SendMessageW(fs_windows[0], WM_RBUTTONUP, 0, 0);
        CHECK(fs_active && is_running, "rapid click 3 starts timer again");
        SendMessageW(fs_windows[0], WM_RBUTTONDBLCLK, 0, 0);
        SendMessageW(fs_windows[0], WM_RBUTTONUP, 0, 0);
        CHECK(fs_active && !is_running, "rapid click 4 stops timer again");

        SendMessageW(fs_windows[0], WM_KEYDOWN, VK_ESCAPE, 0); pump(50);
    }
    CHECK(!fs_active, "escape exits fullscreen after right click test");
    CHECK(GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS) <= gdi_before + 2, "repeated entry and paint do not leak GDI objects");

    dialog_case = 1; dialog_index = 0;
    SetTimer(NULL, 0, 150, dialog_timer); choose_menu(ID_MENU_COLOR_FIRST);
    CHECK(settings.fullscreen_colors[0] == fs_default_colors[0], "cancel discards edits");
    dialog_case = 2;
    for (dialog_index = 0; dialog_index < 5; ++dialog_index) {
        SetTimer(NULL, 0, 150, dialog_timer); choose_menu(fs_color_command(dialog_index));
        memset(settings.fullscreen_colors, 0, sizeof(settings.fullscreen_colors)); load_settings();
        for (i = 0; i < 5; ++i) {
            CHECK(settings.fullscreen_colors[i] == (i <= dialog_index ? 0x123456 : fs_default_colors[i]), "save changes only chosen color and survives reload");
            CHECK(!wcscmp(settings.fullscreen_fonts[i], i <= dialog_index ? L"Bahnschrift" : fs_default_fonts[i]), "save changes font and survives reload");
            CHECK(settings.fullscreen_scales[i] == (i <= dialog_index ? 120 : fs_default_scales[i]), "save changes scale and survives reload");
        }
    }
    dialog_case = 4; dialog_index = 0;
    SetTimer(NULL, 0, 150, dialog_timer); choose_menu(ID_MENU_COLOR_FIRST);
    CHECK(settings.fullscreen_colors[0] == 0x123456, "picker cancel leaves saved color unchanged");
    {
        int duration = settings.long_pomodoro_duration;
        choose_menu(ID_MENU_COLOR_RESET);
        load_settings();
        CHECK(settings.long_pomodoro_duration == duration, "palette reset preserves timer settings");
        for (i = 0; i < FS_COLOR_COUNT; ++i) {
            CHECK(settings.fullscreen_colors[i] == fs_default_colors[i], "menu restores and persists all default colors");
            CHECK(!wcscmp(settings.fullscreen_fonts[i], fs_default_fonts[i]), "menu restores and persists all default fonts");
            CHECK(settings.fullscreen_scales[i] == fs_default_scales[i], "menu restores and persists all default scales");
        }
    }
    choose_menu(ID_MENU_FULLSCREEN); CHECK(fs_active, "all screens menu enters fullscreen");
    choose_menu(ID_MENU_FULLSCREEN); CHECK(fs_active, "all screens menu is idempotent");
    choose_menu(ID_MENU_FULLSCREEN_EXIT); CHECK(!fs_active, "exit fullscreen menu exits all windows");

    settings.enable_clock_sound = 0; settings.enable_completion_sound = 0; settings.show_completion_dialog = 1;
    settings.enable_overtime_count_up = 1;
    fs_show_all(); start_timer(g_main_hwnd, 0, TIMER_CUSTOM); pump(1250);
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
    fs_show_all(); start_timer(g_main_hwnd, 0, TIMER_CUSTOM); pump(200);
    CHECK(!is_running && fs_active && !wcscmp(fs_view.time, L"00:00") && wcsstr(fs_view.status, L"已完成"), "completion remains visible even with toast disabled");
    stop_timer_thread_if_needed(); fs_exit();
    fs_show_all(); CHECK(wcsstr(fs_view.status, L"未开始") != NULL, "reentry clears completed presentation"); fs_exit();

    /* Verify compatibility and the existing recovery chain without registry writes. */
    write_test_json(g_settings_path, "{\"pomodoro_duration\":42,\"pomodoro_count\":3}");
    load_settings();
    CHECK(settings.long_pomodoro_duration == 42 && pomodoro_count == 3, "legacy settings preserve existing values");
    CHECK(!settings.fullscreen_signature[0] && settings.fullscreen_show_signature, "legacy signature defaults to empty");
    for (i = 0; i < FS_COLOR_COUNT; ++i) CHECK(settings.fullscreen_colors[i] == fs_default_colors[i], "legacy settings get new palette defaults");
    write_test_json(g_settings_path, "{\"pomodoro_duration\":42,\"pomodoro_count\":3,\"fullscreen_focus_color\":-1,\"fullscreen_break_color\":16777216,\"fullscreen_count_up_color\":\"bad\"}");
    load_settings();
    for (i = 0; i < FS_COLOR_COUNT; ++i) CHECK(settings.fullscreen_colors[i] == fs_default_colors[i], "invalid stored color falls back to default");
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
