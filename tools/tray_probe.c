/* Read-only diagnostic using the production tray-region/source classifier. */
#define WinMain PomodoroOriginalWinMain
#include "../pomodoro-timer.c"
#undef WinMain
static BOOL CALLBACK inspect_item(HWND window, LPARAM unused) {
    wchar_t cls[256]; RECT rect; POINT point; HWND hit;
    (void)unused; td_class(window, cls);
    if (!td_prefix(cls, L"DFTaskbarItem:TrayIcon:")) return TRUE;
    GetWindowRect(window, &rect); point.x = (rect.left + rect.right) / 2; point.y = (rect.top + rect.bottom) / 2;
    hit = WindowFromPoint(point); td_class(hit, cls);
    wprintf(L"icon=%p hit=%p hitclass=%ls source-valid=%d\n", (void*)window, (void*)hit, cls, td_source_is_tray(hit));
    return TRUE;
}
static BOOL CALLBACK inspect_bar(HWND window, LPARAM unused) {
    wchar_t cls[256]; (void)unused; td_class(window, cls);
    if (td_prefix(cls, L"DFTaskbar:")) {
        wprintf(L"bar=%p class=%ls process-valid=%d\n", (void*)window, cls, td_df_process(window));
        EnumChildWindows(window, inspect_item, 0);
    }
    return TRUE;
}
int main(void) {
    TrayRegions regions; size_t i; HANDLE dpi = fs_enter_dpi();
    EnumWindows(inspect_bar, 0); td_get_regions(&regions);
    printf("regions=%zu overflow=%d\n", regions.count, regions.overflow);
    for (i=0; i<regions.count; ++i) printf("region=%ld,%ld,%ld,%ld\n", regions.rects[i].left, regions.rects[i].top, regions.rects[i].right, regions.rects[i].bottom);
    fs_leave_dpi(dpi); return 0;
}
