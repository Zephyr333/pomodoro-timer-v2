#ifndef POMODORO_REPEAT_REMINDER_H
#define POMODORO_REPEAT_REMINDER_H

/* Main-thread only. A notification owns a stage, not a second completion. */
static struct {
    int valid, armed, interval_minutes;
    unsigned generation;
    TimerMode source;
    ULONGLONG due;
} repeat_reminder;

static void repeat_reminder_cancel(void) {
    KillTimer(g_main_hwnd, ID_REPEAT_REMINDER_TIMER);
    memset(&repeat_reminder, 0, sizeof(repeat_reminder));
}

static int repeat_reminder_valid(void) {
    return repeat_reminder.valid && repeat_reminder.generation == timer_stage_generation;
}

static void repeat_reminder_event(TimerMode source) {
    repeat_reminder_cancel();
    repeat_reminder.valid = 1;
    repeat_reminder.generation = timer_stage_generation;
    repeat_reminder.source = source;
}

static void repeat_reminder_arm(void) {
    if (!repeat_reminder_valid() || !settings.enable_repeat_reminder) return;
    repeat_reminder.due = GetTickCount64() + (ULONGLONG)settings.repeat_reminder_interval_minutes * 60000;
    repeat_reminder.interval_minutes = settings.repeat_reminder_interval_minutes;
    repeat_reminder.armed = SetTimer(g_main_hwnd, ID_REPEAT_REMINDER_TIMER, 1000, NULL) != 0;
    if (!repeat_reminder.armed)
        MessageBoxW(g_main_hwnd, L"无法安排重复提醒。", L"提醒失败", MB_OK | MB_ICONERROR);
}

static void repeat_reminder_user_closed(unsigned generation) {
    if (generation == timer_stage_generation && repeat_reminder_valid()) repeat_reminder_arm();
}

static void repeat_reminder_preferences_changed(void) {
    if (!repeat_reminder_valid()) { repeat_reminder_cancel(); return; }
    if (!settings.enable_repeat_reminder) {
        repeat_reminder.armed = 0;
        KillTimer(g_main_hwnd, ID_REPEAT_REMINDER_TIMER);
    } else if (repeat_reminder.armed && repeat_reminder.interval_minutes != settings.repeat_reminder_interval_minutes) repeat_reminder_arm();
}

/* 'now' is explicit so the real dispatch path can be tested without long sleeps. */
static void repeat_reminder_tick(ULONGLONG now) {
    if (!repeat_reminder_valid()) { repeat_reminder_cancel(); return; }
    if (!settings.enable_repeat_reminder) { repeat_reminder_preferences_changed(); return; }
    if (!repeat_reminder.armed || now < repeat_reminder.due) return;
    repeat_reminder.armed = 0;
    KillTimer(g_main_hwnd, ID_REPEAT_REMINDER_TIMER);
    if (g_hToastWnd || sr_active()) return; /* Wait for an explicit close. */
    if (settings.enable_completion_sound) play_resource_sound("DING_WAV");
    if (settings.reminder_mode == 2) {
        if (!sr_start()) ShowCompletionNotification(g_main_hwnd, repeat_reminder.source);
    } else if (settings.reminder_mode == 1 && !fs_active) {
        ShowCompletionNotification(g_main_hwnd, repeat_reminder.source);
    }
    if (!g_hToastWnd && !sr_active()) repeat_reminder_arm();
}
#endif
