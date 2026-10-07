#ifndef POMODORO_TIMER_STATE_H
#define POMODORO_TIMER_STATE_H

/* Included after the existing mode-start helpers. No worker mutates these values. */
static void clear_micro_state(void) { memset(&micro, 0, sizeof(micro)); }

static int timer_session_active(void) {
    return is_running || is_paused || micro.phase != MICRO_NONE;
}

static TimerMode timer_unfinished_focus(void) {
    if (micro.source != TIMER_NONE) return micro.source;
    return (is_running || is_paused) && !is_overtime && is_pomodoro_mode(current_timer_mode) ? current_timer_mode : TIMER_NONE;
}

static int timer_can_edit_time(void) {
    return (is_running || is_paused) && !is_overtime &&
        micro.phase != MICRO_WAIT_START && micro.phase != MICRO_WAIT_RESUME;
}

static int timer_effective_count(void) { return session_rules.valid ? session_rules.count : settings.enable_pomodoro_count; }
static int timer_effective_overtime(void) { return session_rules.valid ? session_rules.overtime : settings.enable_overtime_count_up; }
static int timer_setting_locked(int command) {
    switch (command) {
        case ID_MENU_IMPORT_DATA: return is_running;
        case ID_MENU_DATA_LOC_DEFAULT:
        case ID_MENU_DATA_LOC_ONEDRIVE:
        case ID_MENU_DATA_LOC_CUSTOM: return is_running && !timer_ui_selection_locked(command);
        case ID_MENU_SET_TIME:
        case ID_MENU_PLUS_5_MIN:
        case ID_MENU_MINUS_5_MIN: return !timer_can_edit_time();
        case ID_MENU_SET_TOAST_COLLAPSE_SECONDS: return settings.reminder_mode != 1;
        default: return 0;
    }
}
static int timer_correct_today_count(HWND hwnd, int count) {
    timer_sync_clock(hwnd);
    refresh_today_count_if_day_changed(hwnd, 0);
    if (!sync_today_count_to_target(count)) {
        MessageBoxW(hwnd,L"今日统计同步失败。",L"错误",MB_OK|MB_ICONERROR); return 0;
    }
    pomodoro_count = count; save_settings();
    if (g_hHeatmapWnd) InvalidateRect(g_hHeatmapWnd,NULL,TRUE);
    refresh_timer_icon_by_state(hwnd); return 1;
}
static void timer_apply_live_preferences(void) {
    if (!settings.enable_clock_sound) stop_clock_loop_sound();
    else if (is_running && !is_overtime) start_clock_loop_sound();
    sr_validate();
    if (g_hToastWnd && settings.reminder_mode != toast_reminder_origin) close_toast_notification_if_open();
    toast_update_auto_collapse();
    timer_refresh_toast();
    if (fs_active) fs_refresh();
}
static void load_settings_preserving_runtime(void) {
    IdleMode selected = idle_mode; int focus_long = idle_pomodoro_is_long, break_long = idle_break_is_long;
    load_settings();
    idle_mode = selected; idle_pomodoro_is_long = focus_long; idle_break_is_long = break_long;
    timer_apply_live_preferences();
}

static void timer_notify(HWND hwnd, TimerMode mode) {
    if (settings.enable_completion_sound) play_resource_sound("DING_WAV");
    notification_reminder_mode = settings.reminder_mode;
    if (settings.reminder_mode != 0)
        PostMessageW(hwnd, WM_TOAST_NOTIFY, mode, (LPARAM)timer_stage_generation);
}

static TimerMode timer_next_mode(TimerMode source) {
    if (is_pomodoro_mode(source) || source == TIMER_COUNT_UP) return get_default_break_mode();
    if (source == TIMER_CUSTOM) return TIMER_CUSTOM;
    return get_default_pomodoro_mode();
}

static void timer_refresh_toast(void) {
    if (!g_hToastWnd || toast_stage_generation != timer_stage_generation) return;
    wchar_t status[96];
    wchar_t *message = (wchar_t *)GetWindowLongPtrW(g_hToastWnd, GWLP_USERDATA);
    timer_ui_status(status, 96, timer_ui_resolve());
    if (message) {
        if (timer_effective_count()) swprintf(message, 256, L"%ls\n今日累计: %d", status, pomodoro_count);
        else swprintf(message, 256, L"%ls", status);
    }
    SetWindowTextW(g_hToastButton, timer_ui_primary_label());
    InvalidateRect(g_hToastWnd, NULL, FALSE);
}

static void timer_begin_micro_wait(HWND hwnd) {
    micro.phase = MICRO_WAIT_START;
    micro.frozen_seconds = remaining_seconds;
    ++timer_stage_generation; sr_validate();
    close_toast_notification_if_open();
    stop_clock_loop_sound();
    if (timer_effective_overtime()) {
        is_overtime = 1;
        overtime_seconds = 0;
        overtime_source_mode = micro.source;
        current_timer_mode = TIMER_NONE;
    } else {
        stop_timer_clock();
        is_paused = 0;
    }
    timer_notify(hwnd, TIMER_MICRO_BREAK);
}

static void timer_finish_countdown(HWND hwnd) {
    TimerMode source = current_timer_mode;
    int use_overtime = timer_effective_overtime();
    ++timer_stage_generation; sr_validate();
    close_toast_notification_if_open();
    stop_clock_loop_sound();
    if (source == TIMER_MICRO_BREAK) {
        micro.phase = MICRO_WAIT_RESUME;
    } else {
        if (is_pomodoro_mode(source) && timer_effective_count()) {
            int count = source == TIMER_LONG_POMODORO ? (session_rules.valid ? session_rules.long_count : settings.long_pomodoro_count) : 1;
            if (record_completed_pomodoros(count)) pomodoro_count = get_today_count_from_storage();
            else {
                int target = clamp_int(get_today_count_from_storage() + count, 0, 9999);
                if (!sync_today_count_to_target(target)) PostMessageW(hwnd, WM_STATS_SAVE_FAILED, 0, 0);
                pomodoro_count = get_today_count_from_storage();
            }
        }
        set_idle_mode_after_manual_stop();
        clear_micro_state();
        completed_pending_mode = source;
    }
    clear_count_up_state();
    if (use_overtime) {
        is_overtime = 1;
        overtime_seconds = 0;
        overtime_source_mode = source;
        current_timer_mode = TIMER_NONE;
    } else {
        stop_timer_clock();
        is_paused = 0;
        current_timer_mode = micro.phase == MICRO_WAIT_RESUME ? TIMER_MICRO_BREAK : TIMER_NONE;
        if (micro.phase == MICRO_NONE) {
            session_rules.valid = 0;
            fs_completed_mode = source;
            if (fs_active) fs_refresh();
        }
    }
    save_settings();
    timer_notify(hwnd, source);
}

static int timer_saturating_add(int value, int delta) {
    return value > INT_MAX - delta ? INT_MAX : value + delta;
}

/* Consume elapsed time at the physical boundary, then in the resulting phase. */
static void timer_advance_seconds(HWND hwnd, int elapsed) {
    while (is_running && (elapsed > 0 || (!is_overtime && !is_count_up_timer && remaining_seconds <= 0))) {
        if (is_overtime) {
            overtime_seconds = timer_saturating_add(overtime_seconds, elapsed);
            break;
        }
        if (is_count_up_timer) {
            remaining_seconds = timer_saturating_add(remaining_seconds, elapsed);
            break;
        }
        if (remaining_seconds <= 0) {
            remaining_seconds = 0;
            timer_finish_countdown(hwnd);
            continue;
        }
        int ordinal = 0, boundary = 0;
        if (is_pomodoro_mode(current_timer_mode) && micro.source != TIMER_NONE && micro.interval_seconds > 0) {
            LONGLONG total = micro.base_seconds;
            LONGLONG candidate = total >= remaining_seconds ? (total - remaining_seconds) / micro.interval_seconds + 1 : 1;
            LONGLONG tick = total - candidate * micro.interval_seconds;
            /* Crossing is strict: resuming or manually setting exactly on a tick does not trigger. */
            if (tick > 0 && tick < remaining_seconds) { boundary = (int)tick; ordinal = 1; }
        }
        int distance = remaining_seconds - boundary;
        int step = elapsed < distance ? elapsed : distance;
        remaining_seconds -= step;
        elapsed -= step;
        if (remaining_seconds == boundary) {
            if (ordinal) timer_begin_micro_wait(hwnd);
            else timer_finish_countdown(hwnd);
        } else if (settings.enable_clock_sound && remaining_seconds <= 10) Beep(440, 100);
    }
    refresh_timer_icon_by_state(hwnd);
    if (fs_active) fs_refresh();
}

static void timer_sync_clock(HWND hwnd) {
    if (!is_running || !timer_clock_id) return;
    ULONGLONG now = GetTickCount64(), seconds = (now - timer_last_tick) / 1000;
    if (!seconds) {
        if (!is_overtime && !is_count_up_timer && remaining_seconds <= 0) timer_advance_seconds(hwnd, 0);
        return;
    }
    int elapsed = seconds > INT_MAX ? INT_MAX : (int)seconds;
    timer_last_tick += (ULONGLONG)elapsed * 1000;
    timer_advance_seconds(hwnd, elapsed);
}

static void timer_start_micro(HWND hwnd) {
    stop_timer_clock();
    clear_overtime_state();
    close_toast_notification_if_open();
    micro.phase = MICRO_ACTIVE;
    ++timer_stage_generation; sr_validate();
    current_timer_mode = TIMER_MICRO_BREAK;
    remaining_seconds = micro.duration_seconds;
    timer_remainder_ms=0;
    is_paused = 0;
    is_running = 1;
    launch_timer_clock(hwnd);
    refresh_timer_icon_by_state(hwnd);
}

static void timer_restore_focus(HWND hwnd) {
    stop_timer_clock();
    clear_overtime_state();
    close_toast_notification_if_open();
    current_timer_mode = micro.source;
    remaining_seconds = micro.frozen_seconds;
    timer_remainder_ms=0;
    micro.phase = MICRO_NONE;
    ++timer_stage_generation; sr_validate();
    fs_completed_mode = TIMER_NONE;
    is_paused = 0;
    is_running = 1;
    launch_timer_clock(hwnd);
    refresh_timer_icon_by_state(hwnd);
}

static void timer_select_idle(void) {
    session_rules.valid = 0;
    stop_timer_clock(); clear_overtime_state(); clear_micro_state(); clear_count_up_state();
    close_toast_notification_if_open();
    current_timer_mode = TIMER_NONE; remaining_seconds = 0; is_paused = 0;
    completed_pending_mode = TIMER_NONE; fs_completed_mode = TIMER_NONE;
    ++timer_stage_generation; sr_validate();
}
static void timer_end_overtime(HWND hwnd) {
    TimerMode source = overtime_source_mode;
    stop_timer_clock(); clear_overtime_state(); close_toast_notification_if_open();
    is_paused = 0; ++timer_stage_generation; sr_validate();
    if (micro.phase == MICRO_WAIT_START) {
        current_timer_mode = micro.source;
        remaining_seconds = micro.frozen_seconds;
    } else if (micro.phase == MICRO_WAIT_RESUME) {
        current_timer_mode = TIMER_MICRO_BREAK; remaining_seconds = 0;
    } else {
        /* Source is still available after clear_overtime_state() in this local variable. */
        current_timer_mode = source;
        set_idle_mode_after_manual_stop();
        session_rules.valid = 0;
        current_timer_mode = TIMER_NONE; remaining_seconds = 0;
        completed_pending_mode = source;
    }
    refresh_timer_icon_by_state(hwnd);
    if (fs_active) fs_refresh();
}
static void timer_stop_action(HWND hwnd) {
    if (!timer_ui_can_end()) return;
    if (is_overtime) { timer_end_overtime(hwnd); return; }
    if (micro.phase == MICRO_ACTIVE) {
        stop_timer_clock(); close_toast_notification_if_open();
        is_paused = 0; micro.phase = MICRO_WAIT_RESUME; remaining_seconds = 0;
        ++timer_stage_generation; sr_validate();
    } else {
        session_rules.valid = 0;
        stop_timer_clock(); set_idle_mode_after_manual_stop();
        is_paused = 0; current_timer_mode = TIMER_NONE; remaining_seconds = 0;
        clear_count_up_state(); clear_micro_state(); completed_pending_mode = TIMER_NONE;
        close_toast_notification_if_open(); ++timer_stage_generation; sr_validate();
    }
    save_settings(); refresh_timer_icon_by_state(hwnd);
    if (fs_active) fs_refresh();
}

static void timer_primary_action(HWND hwnd) {
    timer_sync_clock(hwnd);
    if (is_overtime) {
        if(micro.phase==MICRO_WAIT_START) timer_start_micro(hwnd);
        else if(micro.phase==MICRO_WAIT_RESUME) timer_restore_focus(hwnd);
        else {
            TimerMode target=timer_next_mode(overtime_source_mode);
            start_mode_from_menu(hwnd,target);
        }
        return;
    }
    if (is_running || is_paused) { timer_stop_action(hwnd); return; }
    if (micro.phase == MICRO_WAIT_START) timer_start_micro(hwnd);
    else if (micro.phase == MICRO_WAIT_RESUME) timer_restore_focus(hwnd);
    else start_current_idle_mode(hwnd);
}

#endif
