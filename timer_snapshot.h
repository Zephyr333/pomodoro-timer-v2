#ifndef POMODORO_TIMER_SNAPSHOT_H
#define POMODORO_TIMER_SNAPSHOT_H

typedef struct {
    int running, paused, seconds, count_up, overtime, overtime_seconds, in_pomodoro;
    unsigned remainder;
    TimerMode mode, overtime_source, completed, fullscreen_completed;
    IdleMode idle;
    int idle_focus_long, idle_break_long;
    MicroState micro;
    SessionRules rules;
    RepeatReminderSnapshot reminder;
} TimerSnapshot;
static TimerSnapshot timer_snapshot, snapshot_candidate;
enum { SNAPSHOT_RETRY_CLOCK = 1, SNAPSHOT_RETRY_REMINDER = 2 };
static int snapshot_valid, snapshot_depth, snapshot_candidate_valid, snapshot_retry;

static void snapshot_clear(void) {
    snapshot_valid = snapshot_candidate_valid = snapshot_retry = 0;
}

static void snapshot_begin(HWND hwnd) {
    if (snapshot_depth++) return;
    timer_sync_clock(hwnd);
    snapshot_candidate.running = is_running;
    snapshot_candidate.paused = is_paused;
    snapshot_candidate.seconds = remaining_seconds;
    snapshot_candidate.count_up = is_count_up_timer;
    snapshot_candidate.overtime = is_overtime;
    snapshot_candidate.overtime_seconds = overtime_seconds;
    snapshot_candidate.in_pomodoro = is_in_pomodoro;
    snapshot_candidate.remainder = is_running && timer_clock_id ?
        (unsigned)((GetTickCount64() - timer_last_tick) % 1000) : timer_remainder_ms;
    snapshot_candidate.mode = current_timer_mode;
    snapshot_candidate.overtime_source = overtime_source_mode;
    snapshot_candidate.completed = completed_pending_mode;
    snapshot_candidate.fullscreen_completed = fs_completed_mode;
    snapshot_candidate.idle = idle_mode;
    snapshot_candidate.idle_focus_long = idle_pomodoro_is_long;
    snapshot_candidate.idle_break_long = idle_break_is_long;
    snapshot_candidate.micro = micro;
    snapshot_candidate.rules = session_rules;
    snapshot_candidate.reminder = repeat_reminder_capture(GetTickCount64());
    snapshot_candidate_valid = 1;
}

static void snapshot_end(void) {
    if (--snapshot_depth) return;
    if (snapshot_candidate_valid) {
        timer_snapshot = snapshot_candidate;
        snapshot_valid = 1;
        snapshot_retry = 0;
    }
    snapshot_candidate_valid = 0;
}

static void snapshot_resume_succeeded(void) {
    if (!(snapshot_retry & SNAPSHOT_RETRY_CLOCK)) return;
    snapshot_retry &= ~SNAPSHOT_RETRY_CLOCK;
    if (snapshot_retry & SNAPSHOT_RETRY_REMINDER) {
        if (repeat_reminder_restore(&timer_snapshot.reminder, GetTickCount64())) snapshot_retry &= ~SNAPSHOT_RETRY_REMINDER;
    }
    if (!snapshot_retry) snapshot_clear();
}

static void snapshot_restore(HWND hwnd) {
    timer_sync_clock(hwnd); /* A completion at this boundary invalidates undo. */
    if (!snapshot_valid) return;
    TimerSnapshot saved = timer_snapshot;
    stop_timer_clock();
    ++timer_stage_generation;
    InterlockedIncrement(&fs_session);
    repeat_reminder_cancel();
    sr_dismiss(1);
    close_toast_notification_if_open();
    remaining_seconds = saved.seconds;
    is_paused = saved.paused;
    is_running = saved.running;
    is_count_up_timer = saved.count_up;
    is_overtime = saved.overtime;
    overtime_seconds = saved.overtime_seconds;
    is_in_pomodoro = saved.in_pomodoro;
    timer_remainder_ms = saved.remainder;
    current_timer_mode = saved.mode;
    overtime_source_mode = saved.overtime_source;
    completed_pending_mode = saved.completed;
    fs_completed_mode = saved.fullscreen_completed;
    idle_mode = saved.idle;
    idle_pomodoro_is_long = saved.idle_focus_long;
    idle_break_is_long = saved.idle_break_long;
    micro = saved.micro;
    session_rules = saved.rules;
    /* Separate failures: resuming the clock must not discard a failed reminder retry. */
    snapshot_retry = saved.running && !launch_timer_clock(hwnd) ? SNAPSHOT_RETRY_CLOCK : 0;
    if (!repeat_reminder_restore(&saved.reminder, GetTickCount64())) snapshot_retry |= SNAPSHOT_RETRY_REMINDER;
    if (!snapshot_retry) snapshot_clear();
    refresh_timer_icon_by_state(hwnd);
    if (fs_active) fs_refresh();
}
#endif
