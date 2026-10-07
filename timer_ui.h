#ifndef POMODORO_TIMER_UI_H
#define POMODORO_TIMER_UI_H

/* Presentation and action availability only; never advance a timer here. */
static const wchar_t *timer_mode_name(TimerMode mode) {
    switch (mode) {
        case TIMER_LONG_POMODORO: return L"长番茄钟";
        case TIMER_SHORT_POMODORO: return L"短番茄钟";
        case TIMER_SHORT_BREAK: return L"短休息";
        case TIMER_LONG_BREAK: return L"长休息";
        case TIMER_COUNT_UP: return L"正计时";
        case TIMER_CUSTOM: return L"自定义计时";
        case TIMER_MICRO_BREAK: return L"微休息";
        default: return L"番茄钟";
    }
}
typedef enum { TIMER_UI_READY, TIMER_UI_RUNNING, TIMER_UI_PAUSED } TimerUiState;
typedef struct { TimerMode mode; TimerUiState state; int seconds, overtime; } TimerUiView;
static int timer_ui_duration(TimerMode mode) {
    switch (mode) {
        case TIMER_LONG_POMODORO: return settings.long_pomodoro_duration * 60;
        case TIMER_SHORT_POMODORO: return settings.short_pomodoro_duration * 60;
        case TIMER_SHORT_BREAK: return settings.short_break_duration * 60;
        case TIMER_LONG_BREAK: return settings.long_break_duration * 60;
        case TIMER_CUSTOM: return settings.custom_duration * 60;
        case TIMER_MICRO_BREAK: return micro.duration_seconds;
        default: return 0;
    }
}
static TimerUiView timer_ui_resolve(void) {
    TimerUiView view = {current_timer_mode, TIMER_UI_READY, remaining_seconds, is_overtime};
    if (is_overtime) {
        view.mode = overtime_source_mode;
        view.seconds = overtime_seconds;
        view.state = is_paused ? TIMER_UI_PAUSED : TIMER_UI_RUNNING;
    } else if (micro.phase == MICRO_WAIT_START) {
        view.mode = TIMER_MICRO_BREAK; view.seconds = micro.duration_seconds;
    } else if (micro.phase == MICRO_WAIT_RESUME) {
        view.mode = micro.source; view.seconds = micro.frozen_seconds;
    } else if (is_running || is_paused) {
        view.state = is_paused ? TIMER_UI_PAUSED : TIMER_UI_RUNNING;
    } else {
        if (idle_mode == IDLE_BREAK) view.mode = idle_break_is_long ? TIMER_LONG_BREAK : TIMER_SHORT_BREAK;
        else if (idle_mode == IDLE_COUNT_UP) view.mode = TIMER_COUNT_UP;
        else if (idle_mode == IDLE_CUSTOM) view.mode = TIMER_CUSTOM;
        else view.mode = idle_pomodoro_is_long ? TIMER_LONG_POMODORO : TIMER_SHORT_POMODORO;
        view.seconds = timer_ui_duration(view.mode);
    }
    return view;
}
static int timer_ui_selection_locked(int command) {
    switch (command) {
        case ID_MENU_DEFAULT_LONG_POMODORO: return settings.default_pomodoro_is_long;
        case ID_MENU_DEFAULT_SHORT_POMODORO: return !settings.default_pomodoro_is_long;
        case ID_MENU_DEFAULT_LONG_BREAK: return settings.default_break_is_long;
        case ID_MENU_DEFAULT_SHORT_BREAK: return !settings.default_break_is_long;
        case ID_MENU_DATA_LOC_DEFAULT: return g_data_location_mode == DATA_LOC_DEFAULT;
        case ID_MENU_DATA_LOC_ONEDRIVE: return g_data_location_mode == DATA_LOC_ONEDRIVE;
        default: return 0;
    }

}
static int timer_ui_is_ready(void) { return timer_ui_resolve().state == TIMER_UI_READY; }
static int timer_ui_can_start(void) { return 1; }
static int timer_ui_can_pause(void) { return !timer_ui_is_ready(); }
static int timer_ui_can_end(void) { return !timer_ui_is_ready(); }
static const wchar_t *timer_ui_pause_label(void) { return is_paused ? L"继续" : L"暂停"; }
static const wchar_t *timer_ui_primary_label(void) {
    return is_overtime || timer_ui_is_ready() ? L"开始" : L"结束";
}
static void timer_ui_status(wchar_t *out, size_t capacity, TimerUiView view) {
    const wchar_t *suffix = view.state == TIMER_UI_READY ? L" · 未开始" : view.state == TIMER_UI_PAUSED ? L" · 已暂停" : L"";
    swprintf(out, capacity, L"%ls%ls", view.overtime ? L"超时正计时" : timer_mode_name(view.mode), suffix);
}
static void timer_ui_notification(wchar_t *out, size_t capacity, TimerMode mode) {
    (void)mode;
    timer_ui_status(out, capacity, timer_ui_resolve());
}
#endif
