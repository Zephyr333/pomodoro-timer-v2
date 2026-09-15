#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <knownfolders.h>
#include <direct.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <time.h>
#include <mmsystem.h>
#include <tchar.h>

#define ID_MENU_LANGUAGE 301
#define ID_MENU_LANG_EN 302
#define ID_MENU_LANG_HU 303
#define ID_MENU_LANG_DE 304
#define ID_MENU_LANG_IT 305
#define ID_MENU_LANG_ES 306
#define ID_MENU_LANG_FR 307
#define ID_MENU_LANG_RU 308
#define ID_MENU_RESET_COUNT 309
#define ID_MENU_SET_TIME 310
#define ID_MENU_SET_COUNT 311
#define ID_MENU_IDLE_POMODORO 312
#define ID_MENU_IDLE_SHORT_BREAK 313
#define ID_MENU_IDLE_CUSTOM 314
#define ID_MENU_START_CUSTOM 315
#define ID_MENU_REPORT 316
#define ID_MENU_PAUSE_RESUME 317
#define ID_MENU_PLUS_5_MIN 318
#define ID_MENU_MINUS_5_MIN 319
#define ID_MENU_STOP 320
#define ID_MENU_SET_POMODORO_DURATION 321
#define ID_MENU_SET_SHORT_BREAK_DURATION 322
#define ID_MENU_SET_LONG_BREAK_DURATION 323
#define ID_MENU_SET_CUSTOM_DURATION 324
#define ID_MENU_START_CURRENT 325
#define ID_MENU_SET_TOAST_COLLAPSE_SECONDS 326
#define ID_MENU_IMPORT_DATA 327
#define ID_MENU_EXPORT_DATA 328
#define ID_MENU_RESET_DEFAULTS 329
#define ID_MENU_DATA_LOC_DEFAULT 330
#define ID_MENU_DATA_LOC_ONEDRIVE 331
#define ID_MENU_DATA_LOC_CUSTOM 332
#define ID_MENU_IDLE_LONG_BREAK 334
#define ID_MENU_COMPLETION_SOUND 335
#define ID_MENU_SET_TIME_BLOCK 336
#define ID_MENU_START_SHORT_POMODORO 337
#define ID_MENU_SET_SHORT_POMODORO_DURATION 338
#define ID_MENU_DEFAULT_LONG_POMODORO 339
#define ID_MENU_DEFAULT_SHORT_POMODORO 340
#define ID_MENU_DEFAULT_LONG_BREAK 341
#define ID_MENU_DEFAULT_SHORT_BREAK 342
#define ID_MENU_IDLE_SHORT_POMODORO 343
#define ID_MENU_SET_LONG_POMODORO_COUNT 344
#define ID_MENU_START_COUNT_UP 345
#define ID_MENU_IDLE_COUNT_UP 346
#define ID_MENU_ENABLE_OVERTIME 347
#define TOAST_WINDOW_CLASS L"PomodoroToastClass"
#define HEATMAP_WINDOW_CLASS L"PomodoroHeatmapClass"
#define WM_TOAST_NOTIFY (WM_APP + 100)
#define WM_SETTINGS_SAVE_FAILED (WM_APP + 101)
#define ID_TOAST_ACTION 2001
#define ID_TOAST_CLOSE 2002
#define ID_TOAST_COLLAPSE 2003
#define ID_HEATMAP_PREV 3001
#define ID_HEATMAP_NEXT 3002
#define ID_HEATMAP_TODAY 3003
#define ID_MAIN_DAY_SYNC_TIMER 4001

// Structure for localized strings
typedef struct {
    WCHAR menu_start_pomodoro[64];
    WCHAR menu_start_break[64];
    WCHAR menu_start_long_break[64];
    WCHAR menu_clock_sound[64];
    WCHAR menu_autostart[64];
    WCHAR menu_show_dialog[64];
    WCHAR menu_settings[64];
    WCHAR menu_language[64];
    WCHAR menu_about[64];
    WCHAR menu_exit[64];
    WCHAR menu_reset_count[64];
    WCHAR tooltip_pomodoro[64];
    WCHAR tooltip_break[64];
    WCHAR settings_title[64];
    WCHAR settings_pomodoro[64];
    WCHAR settings_short_break[64];
    WCHAR settings_long_break[64];
    WCHAR settings_enable_sound[64];
    WCHAR settings_autostart[64];
    WCHAR error_invalid_time[128];
    WCHAR notify_pomodoro_complete[128];
    WCHAR notify_break_complete[128];
    WCHAR notify_long_break_complete[128];
} LANG;

// Language definitions
static LANG lang_en = {
    L"开始长番茄钟",
    L"开始休息",
    L"开始长休息",
    L"时钟音效",
    L"开机启动",
    L"显示完成弹窗",
    L"设置",
    L"语言",
    L"关于",
    L"退出",
    L"重置番茄钟计数",
    L"点击开始长番茄钟",
    L"点击开始休息",
    L"番茄钟设置",
    L"长番茄钟时长(分钟):",
    L"短休息时长(分钟):",
    L"长休息时长(分钟):",
    L"启用时钟音效",
    L"开机启动",
    L"时间输入无效! 请输入合理的正数。",
    L"番茄钟完成!",
    L"休息完成!",
    L"长休息完成!"
};

static LANG lang_hu = {
    L"Pomodoro indítása",
    L"Szünet indítása",
    L"Hosszú szünet indítása",
    L"Óra hang",
    L"Indítás rendszerindításkor",
    L"Befejezési ablak megjelenítése",
    L"Beállítások",
    L"Nyelv",
    L"Névjegy",
    L"Kilépés",
    L"Pomodoro körök törlése",
    L"Kattints egy pomodoro indításához",
    L"Kattints egy szünet indításához",
    L"Pomodoro beállítások",
    L"Pomodoro időtartam (perc):",
    L"Rövid szünet időtartama (perc):",
    L"Hosszú szünet időtartama (perc):",
    L"Óra hang engedélyezése",
    L"Indítás rendszerindításkor",
    L"Érvénytelen időértékek! Pozitívnak és ésszerűnek kell lenniük.",
    L"Pomodoro kész!",
    L"Szünet kész!",
    L"Hosszú szünet kész!"
};

static LANG lang_de = {
    L"Pomodoro starten",
    L"Pause starten",
    L"Lange Pause starten",
    L"Uhrenton",
    L"Beim Systemstart starten",
    L"Abschlussdialog anzeigen",
    L"Einstellungen",
    L"Sprache",
    L"Über",
    L"Beenden",
    L"Pomodoro-Sitzungen zurücksetzen",
    L"Klicken Sie, um einen Pomodoro zu starten",
    L"Klicken Sie, um eine Pause zu starten",
    L"Pomodoro-Einstellungen",
    L"Pomodoro-Dauer (Minuten):",
    L"Kurze Pausendauer (Minuten):",
    L"Lange Pausendauer (Minuten):",
    L"Uhrenton aktivieren",
    L"Beim Systemstart starten",
    L"Ungültige Zeitwerte! Müssen positiv und angemessen sein.",
    L"Pomodoro abgeschlossen!",
    L"Pause abgeschlossen!",
    L"Lange Pause abgeschlossen!"
};

static LANG lang_it = {
    L"Avvia Pomodoro",
    L"Avvia Pausa",
    L"Avvia Pausa Lunga",
    L"Suono Orologio",
    L"Avvia all'Avvio del Sistema",
    L"Mostra dialogo completamento",
    L"Impostazioni",
    L"Lingua",
    L"Info",
    L"Esci",
    L"Reimposta sessioni Pomodoro",
    L"Clicca per avviare un pomodoro",
    L"Clicca per avviare una pausa",
    L"Impostazioni Pomodoro",
    L"Durata pomodoro (minuti):",
    L"Durata pausa breve (minuti):",
    L"Durata pausa lunga (minuti):",
    L"Abilita suono orologio",
    L"Avvia all'avvio del sistema",
    L"Valori temporali non validi! Devono essere positivi e ragionevoli.",
    L"Pomodoro Completato!",
    L"Pausa Completata!",
    L"Pausa Lunga Completata!"
};

static LANG lang_es = {
    L"Iniciar Pomodoro",
    L"Iniciar Descanso",
    L"Iniciar Descanso Largo",
    L"Sonido del Reloj",
    L"Iniciar con el Sistema",
    L"Mostrar diálogo finalización",
    L"Configuración",
    L"Idioma",
    L"Acerca de",
    L"Salir",
    L"Reiniciar sesiones Pomodoro",
    L"Clic para iniciar un pomodoro",
    L"Clic para iniciar un descanso",
    L"Configuración de Pomodoro",
    L"Duración del pomodoro (minutos):",
    L"Duración del descanso corto (minutos):",
    L"Duración del descanso largo (minutos):",
    L"Habilitar sonido del reloj",
    L"Iniciar con el sistema",
    L"¡Valores de tiempo inválidos! Deben ser positivos y razonables.",
    L"¡Pomodoro completado!",
    L"¡Descanso completado!",
    L"¡Descanso largo completado!"
};

static LANG lang_fr = {
    L"Démarrer Pomodoro",
    L"Démarrer Pause",
    L"Démarrer Pause Longue",
    L"Son de l'Horloge",
    L"Démarrer avec le Système",
    L"Afficher dialogue de fin",
    L"Paramètres",
    L"Langue",
    L"À propos",
    L"Quitter",
    L"Réinitialiser sessions Pomodoro",
    L"Cliquez pour démarrer un pomodoro",
    L"Cliquez pour démarrer une pause",
    L"Paramètres Pomodoro",
    L"Durée pomodoro (minutes):",
    L"Durée pause courte (minutes):",
    L"Durée pause longue (minutes):",
    L"Activer le son de l'horloge",
    L"Démarrer avec le système",
    L"Valeurs de temps invalides! Doivent être positives et raisonnables.",
    L"Pomodoro Terminé!",
    L"Pause Terminée!",
    L"Pause Longue Terminée!"
};

static LANG lang_ru = {
    L"Запустить Помодоро",
    L"Запустить Перерыв",
    L"Запустить Длинный Перерыв",
    L"Звук Часов",
    L"Запускать при Старте Системы",
    L"Показывать диалог завершения",
    L"Настройки",
    L"Язык",
    L"О программе",
    L"Выход",
    L"Сбросить сессии Помодоро",
    L"Нажмите для запуска помодоро",
    L"Нажмите для запуска перерыва",
    L"Настройки Помодоро",
    L"Длительность помодоро (минуты):",
    L"Длительность короткого перерыва (минуты):",
    L"Длительность длинного перерыва (минуты):",
    L"Включить звук часов",
    L"Запускать при старте системы",
    L"Недопустимые значения времени! Должны быть положительными и разумными.",
    L"Помодоро завершено!",
    L"Перерыв завершен!",
    L"Длинный перерыв завершен!"
};

static LANG *g_lang = &lang_en;
static HMENU g_hLangMenu = NULL;
static HMENU g_hMenu = NULL;

// Timer settings structure
typedef struct {
    int long_pomodoro_duration;
    int long_pomodoro_count;
    int short_pomodoro_duration;
    int short_break_duration;
    int long_break_duration;
    int custom_duration;
    int adjust_block_minutes;
    int toast_auto_collapse_seconds;
    int enable_clock_sound;
    int enable_completion_sound;
    int show_completion_dialog;
    int default_pomodoro_is_long;
    int default_break_is_long;
    int enable_overtime_count_up;
} TimerSettings;

typedef enum {
    DATA_LOC_DEFAULT = 0,
    DATA_LOC_ONEDRIVE = 1,
    DATA_LOC_CUSTOM = 2
} DataLocationMode;

typedef enum {
    TIMER_NONE = 0,
    TIMER_LONG_POMODORO = 1,
    TIMER_SHORT_BREAK = 2,
    TIMER_LONG_BREAK = 3,
    TIMER_CUSTOM = 4,
    TIMER_SHORT_POMODORO = 5,
    TIMER_COUNT_UP = 6
} TimerMode;

typedef enum {
    IDLE_POMODORO = 0,
    IDLE_BREAK = 1,
    IDLE_CUSTOM = 2,
    IDLE_COUNT_UP = 3
} IdleMode;

typedef struct {
    char date[11];
    int count;
} DayCount;

// Global variables
TimerSettings settings = {90, 2, 45, 5, 15, 10, 5, 10, 0, 1, 1, 1, 0, 1};
int pomodoro_count = 0;
int is_running = 0;
int is_paused = 0;
int is_in_pomodoro = 0;
int remaining_seconds = 0;
int is_count_up_timer = 0;
int count_up_credited = 0;
int count_up_threshold_seconds = 0;
int is_overtime = 0;
int overtime_seconds = 0;
TimerMode overtime_source_mode = TIMER_NONE;
TimerMode current_timer_mode = TIMER_NONE;
IdleMode idle_mode = IDLE_POMODORO;
int idle_pomodoro_is_long = 1;
int idle_break_is_long = 0;
NOTIFYICONDATA nid = {0};
HANDLE timer_thread_handle = NULL;
int autostart_enabled = 0;
static HICON last_icon = NULL;
static wchar_t last_text[16] = {0};
static int last_dots = -1;
static int screenWidth = 0, screenHeight = 0;
static HWND g_hToastWnd = NULL;
static HWND g_main_hwnd = NULL; // main invisible window handle
static int toast_is_pomodoro = 0;
static int toast_is_long_break = 0;
static int toast_completed_mode = TIMER_NONE;
static HWND g_hToastButton = NULL;
static HWND g_hToastCloseButton = NULL;
static HWND g_hToastCollapseButton = NULL;
static int g_toast_collapsed = 0;
static RECT g_toast_expanded_rect = {0};
static HWND g_hHeatmapWnd = NULL;
static UINT g_taskbar_created_message = 0;
static DayCount g_day_counts[4096];
static int g_day_count = 0;
static int g_heatmap_weekTotal = 0;
static int g_heatmap_monthTotal = 0;
static int g_heatmap_yearTotal = 0;
static int g_heatmap_total = 0;
static int g_heatmap_display_year = 0;
static int g_heatmap_display_month = 0;
static int g_heatmap_selected_day = 0;
static wchar_t g_heatmap_info_text[160] = L"点击日历日期可查看当日完成数。";
static int g_clock_loop_playing = 0;
static RECT g_heatmap_day_rects[32];
static int g_heatmap_day_rect_valid[32];
static DataLocationMode g_data_location_mode = DATA_LOC_DEFAULT;
static char g_last_count_sync_date[11] = {0};
static CRITICAL_SECTION g_settings_file_lock;
static int g_settings_lock_ready = 0;
static wchar_t g_data_dir[MAX_PATH] = L"";
static wchar_t g_custom_data_dir[MAX_PATH] = L"";
static wchar_t g_settings_path[MAX_PATH] = L"";
static wchar_t g_settings_tmp_path[MAX_PATH] = L"";
static wchar_t g_settings_bak_path[MAX_PATH] = L"";
static wchar_t g_log_path[MAX_PATH] = L"";
static wchar_t g_adjustments_path[MAX_PATH] = L"";

static int get_visible_dots(int total_count) {
    int dots;
    if (total_count <= 0) return 0;
    dots = total_count % 4;
    return dots == 0 ? 4 : dots;
}

// Resource IDs
#define IDD_SETTINGS 100
#define IDD_ABOUT 101
#define IDC_POMODORO 101
#define IDC_SHORT_BREAK 102
#define IDC_LONG_BREAK 103
#define IDC_OK 104
#define IDC_CANCEL 105
#define IDC_CLOCK_SOUND 106
#define IDC_AUTOSTART 107
#define IDC_WEBSITE 108
#define IDC_COFFEE 109
#define IDD_INPUT 110
#define IDC_INPUT_LABEL 110
#define IDC_INPUT_EDIT 111
#define IDC_LABEL_CUSTOM 204
#define IDC_CUSTOM_TIMER 205
#define IDC_SHORT_POMODORO 220
#define IDC_LABEL_SHORT_POMODORO 221
#define IDC_DEFAULT_LONG_POMODORO 222
#define IDC_DEFAULT_SHORT_POMODORO 223
#define IDC_DEFAULT_SHORT_BREAK 224
#define IDC_DEFAULT_LONG_BREAK 225

// Function prototypes
void load_settings();
int save_settings(void);
void update_tray_icon(HWND hwnd, const wchar_t* text, int dots, int seconds);
void play_resource_sound(const char* resourceName);
int is_autostart_enabled();
void RefreshMenuText(void);
int set_autostart(int enable);
INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ShowSettingsDialog(HWND hwndParent);
void ShowAboutDialog(HWND hwndParent);
void ShowCompletionNotification(HWND hwnd, int completed_mode);
void start_timer(HWND hwnd, int duration_minutes, TimerMode mode);
DWORD WINAPI timer_thread(LPVOID lpParam);
int record_completed_pomodoros(int completedCount);
void generate_and_open_report(HWND hwnd);
void RegisterHeatmapWindowClass(void);
void ShowHeatmapWindow(HWND hwnd);
LRESULT CALLBACK HeatmapWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL PromptForInteger(HWND hwndParent, const wchar_t* title, const wchar_t* label, int currentValue, int minValue, int maxValue, int* outValue);
INT_PTR CALLBACK InputDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void load_heatmap_data(void);
static int get_day_count_value(int year, int month, int day);
static int sync_today_count_to_target(int targetCount);
static void build_data_file_paths(void);
static void load_data_location_from_registry(void);
static int save_data_location_to_registry(void);
static int switch_data_location(HWND hwnd, DataLocationMode mode, const wchar_t* customPath);
static int choose_folder_dialog(HWND owner, const wchar_t* title, wchar_t* outPath, size_t outChars);
static int export_data_to_folder(HWND hwnd);
static int import_data_from_folder(HWND hwnd);
static void repair_settings_recovery_chain(void);
static void reset_defaults_keep_data(void);
static void refresh_timer_icon_by_state(HWND hwnd);
static void maybe_archive_log_monthly(void);
static void load_archive_logs(void);
static int get_today_count_from_storage(void);
static void refresh_today_count_if_day_changed(HWND hwnd, int forceRefresh);
static void add_day_count(DayCount* days, int* dayCount, const char* date, int delta);
static int parse_pomodoro_log_entry(const char* line, char date[11], int* completedCount);
static int is_pomodoro_mode(TimerMode mode);
static void credit_count_up_thresholds(HWND hwnd);
static void clear_count_up_state(void);
static int get_long_pomodoro_duration(void);
static TimerMode get_default_pomodoro_mode(void);
static TimerMode get_default_break_mode(void);

static int clamp_int(int value, int minValue, int maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

static void join_path(wchar_t* out, size_t outChars, const wchar_t* dir, const wchar_t* name) {
    if (!out || outChars == 0) return;
    if (!dir || !name) {
        out[0] = L'\0';
        return;
    }
    swprintf(out, outChars, L"%ls\\%ls", dir, name);
}

static void get_module_directory(wchar_t* out, size_t outChars) {
    wchar_t modulePath[MAX_PATH] = L"";
    wchar_t* slash;
    if (!out || outChars == 0) return;
    GetModuleFileNameW(NULL, modulePath, MAX_PATH);
    slash = wcsrchr(modulePath, L'\\');
    if (slash) *slash = L'\0';
    wcsncpy(out, modulePath, outChars - 1);
    out[outChars - 1] = L'\0';
}

static int get_onedrive_directory(wchar_t* out, size_t outChars) {
    DWORD n;
    if (!out || outChars == 0) return 0;
    n = GetEnvironmentVariableW(L"OneDrive", out, (DWORD)outChars);
    if (n > 0 && n < outChars) return 1;

    {
        PWSTR knownPath = NULL;
        if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_OneDrive, 0, NULL, &knownPath)) && knownPath) {
            wcsncpy(out, knownPath, outChars - 1);
            out[outChars - 1] = L'\0';
            CoTaskMemFree(knownPath);
            return 1;
        }
    }

    return 0;
}

static void ensure_directory_exists(const wchar_t* dir) {
    if (!dir || !dir[0]) return;
    SHCreateDirectoryExW(NULL, dir, NULL);
}

static void build_data_file_paths(void) {
    join_path(g_settings_path, MAX_PATH, g_data_dir, L"pomodoro_settings.json");
    join_path(g_settings_tmp_path, MAX_PATH, g_data_dir, L"pomodoro_settings.json.tmp");
    join_path(g_settings_bak_path, MAX_PATH, g_data_dir, L"pomodoro_settings.json.bak");
    join_path(g_log_path, MAX_PATH, g_data_dir, L"pomodoro_log.csv");
    join_path(g_adjustments_path, MAX_PATH, g_data_dir, L"pomodoro_adjustments.csv");
}

static int resolve_data_dir_for_mode(DataLocationMode mode, const wchar_t* customPath, wchar_t* outDir, size_t outChars) {
    if (!outDir || outChars == 0) return 0;
    outDir[0] = L'\0';
    if (mode == DATA_LOC_ONEDRIVE) {
        wchar_t oneDriveDir[MAX_PATH] = L"";
        if (get_onedrive_directory(oneDriveDir, MAX_PATH)) {
            join_path(outDir, outChars, oneDriveDir, L"PomodoroTimer");
            return 1;
        }
        return 0;
    } else if (mode == DATA_LOC_CUSTOM && customPath && customPath[0]) {
        wcsncpy(outDir, customPath, outChars - 1);
        outDir[outChars - 1] = L'\0';
        return 1;
    }
    get_module_directory(outDir, outChars);
    return 1;
}

static int save_data_location_to_registry(void) {
    HKEY hKey;
    LONG modeResult;
    LONG pathResult;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\PomodoroTimer", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD mode = (DWORD)g_data_location_mode;
        modeResult = RegSetValueExW(hKey, L"DataLocationMode", 0, REG_DWORD, (const BYTE*)&mode, sizeof(DWORD));
        pathResult = RegSetValueExW(hKey, L"CustomDataPath", 0, REG_SZ, (const BYTE*)g_custom_data_dir, (DWORD)((wcslen(g_custom_data_dir) + 1) * sizeof(wchar_t)));
        RegCloseKey(hKey);
        return modeResult == ERROR_SUCCESS && pathResult == ERROR_SUCCESS;
    }
    return 0;
}

static void load_data_location_from_registry(void) {
    HKEY hKey;
    DWORD dataSize;
    DWORD mode;
    g_data_location_mode = DATA_LOC_DEFAULT;
    g_custom_data_dir[0] = L'\0';

    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\PomodoroTimer", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        dataSize = sizeof(DWORD);
        if (RegQueryValueExW(hKey, L"DataLocationMode", NULL, NULL, (LPBYTE)&mode, &dataSize) == ERROR_SUCCESS) {
            if (mode <= DATA_LOC_CUSTOM) g_data_location_mode = (DataLocationMode)mode;
        }

        dataSize = sizeof(g_custom_data_dir);
        RegQueryValueExW(hKey, L"CustomDataPath", NULL, NULL, (LPBYTE)g_custom_data_dir, &dataSize);
        RegCloseKey(hKey);
    }

    if (!resolve_data_dir_for_mode(g_data_location_mode, g_custom_data_dir, g_data_dir, MAX_PATH)) {
        // Keep selected mode from registry; only fallback runtime directory for this session.
        get_module_directory(g_data_dir, MAX_PATH);
    }
    ensure_directory_exists(g_data_dir);
    build_data_file_paths();
    repair_settings_recovery_chain();
}

static int copy_if_exists(const wchar_t* src, const wchar_t* dst) {
    DWORD attr = GetFileAttributesW(src);
    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) return 1;
    return CopyFileW(src, dst, FALSE) ? 1 : 0;
}

static void repair_settings_recovery_chain(void) {
    DWORD mainAttr;
    DWORD tmpAttr;
    DWORD bakAttr;

    if (g_settings_lock_ready) EnterCriticalSection(&g_settings_file_lock);

    mainAttr = GetFileAttributesW(g_settings_path);
    tmpAttr = GetFileAttributesW(g_settings_tmp_path);
    bakAttr = GetFileAttributesW(g_settings_bak_path);

    if (mainAttr == INVALID_FILE_ATTRIBUTES || (mainAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        if (tmpAttr != INVALID_FILE_ATTRIBUTES && !(tmpAttr & FILE_ATTRIBUTE_DIRECTORY)) {
            CopyFileW(g_settings_tmp_path, g_settings_path, FALSE);
        } else if (bakAttr != INVALID_FILE_ATTRIBUTES && !(bakAttr & FILE_ATTRIBUTE_DIRECTORY)) {
            CopyFileW(g_settings_bak_path, g_settings_path, FALSE);
        }
    }

    mainAttr = GetFileAttributesW(g_settings_path);
    bakAttr = GetFileAttributesW(g_settings_bak_path);
    if (mainAttr != INVALID_FILE_ATTRIBUTES && !(mainAttr & FILE_ATTRIBUTE_DIRECTORY) &&
        (bakAttr == INVALID_FILE_ATTRIBUTES || (bakAttr & FILE_ATTRIBUTE_DIRECTORY))) {
        CopyFileW(g_settings_path, g_settings_bak_path, TRUE);
    }

    if (g_settings_lock_ready) LeaveCriticalSection(&g_settings_file_lock);
}

static int migrate_data_files(const wchar_t* oldDir, const wchar_t* newDir, int* migratedCount, int* failedCount, int* expectedCount) {
    wchar_t src[MAX_PATH];
    wchar_t dst[MAX_PATH];
    int ok = 1;
    static const wchar_t* names[] = {L"pomodoro_settings.json", L"pomodoro_settings.json.tmp", L"pomodoro_settings.json.bak", L"pomodoro_log.csv", L"pomodoro_adjustments.csv"};
    int i;

    if (migratedCount) *migratedCount = 0;
    if (failedCount) *failedCount = 0;
    if (expectedCount) *expectedCount = 0;

    for (i = 0; i < (int)(sizeof(names) / sizeof(names[0])); ++i) {
        DWORD attr;
        join_path(src, MAX_PATH, oldDir, names[i]);
        join_path(dst, MAX_PATH, newDir, names[i]);
        attr = GetFileAttributesW(src);
        if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) continue;

        if (expectedCount) (*expectedCount)++;

        if (CopyFileW(src, dst, TRUE)) {
            if (migratedCount) (*migratedCount)++;
            continue;
        }

        ok = 0;
        if (failedCount) (*failedCount)++;
    }

    return ok;
}

static int migrate_archive_logs(const wchar_t* oldDir, const wchar_t* newDir, int* migratedCount, int* failedCount, int* expectedCount) {
    WIN32_FIND_DATAW ffd;
    HANDLE hFind;
    wchar_t pattern[MAX_PATH];
    wchar_t src[MAX_PATH];
    wchar_t dst[MAX_PATH];
    int ok = 1;

    swprintf(pattern, MAX_PATH, L"%ls\\pomodoro_log_*.csv", oldDir);
    hFind = FindFirstFileW(pattern, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) return 1;

    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (expectedCount) (*expectedCount)++;

        swprintf(src, MAX_PATH, L"%ls\\%ls", oldDir, ffd.cFileName);
        swprintf(dst, MAX_PATH, L"%ls\\%ls", newDir, ffd.cFileName);

        if (CopyFileW(src, dst, TRUE)) {
            if (migratedCount) (*migratedCount)++;
            continue;
        }

        ok = 0;
        if (failedCount) (*failedCount)++;
    } while (FindNextFileW(hFind, &ffd));

    FindClose(hFind);
    return ok;
}

static int directory_has_data_files(const wchar_t* dir) {
    WIN32_FIND_DATAW ffd;
    HANDLE hFind;
    wchar_t path[MAX_PATH];
    wchar_t pattern[MAX_PATH];
    static const wchar_t* names[] = {L"pomodoro_settings.json", L"pomodoro_settings.json.tmp", L"pomodoro_settings.json.bak", L"pomodoro_log.csv", L"pomodoro_adjustments.csv"};
    int i;

    for (i = 0; i < (int)(sizeof(names) / sizeof(names[0])); ++i) {
        DWORD attr;
        join_path(path, MAX_PATH, dir, names[i]);
        attr = GetFileAttributesW(path);
        if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) return 1;
    }

    swprintf(pattern, MAX_PATH, L"%ls\\pomodoro_log_*.csv", dir);
    hFind = FindFirstFileW(pattern, &ffd);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        return 1;
    }
    return 0;
}

static int delete_data_files_in_directory(const wchar_t* dir) {
    WIN32_FIND_DATAW ffd;
    HANDLE hFind;
    wchar_t path[MAX_PATH];
    wchar_t pattern[MAX_PATH];
    static const wchar_t* names[] = {L"pomodoro_settings.json", L"pomodoro_settings.json.tmp", L"pomodoro_settings.json.bak", L"pomodoro_log.csv", L"pomodoro_adjustments.csv"};
    int ok = 1;
    int i;

    for (i = 0; i < (int)(sizeof(names) / sizeof(names[0])); ++i) {
        DWORD attr;
        join_path(path, MAX_PATH, dir, names[i]);
        attr = GetFileAttributesW(path);
        if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY) && !DeleteFileW(path)) ok = 0;
    }

    swprintf(pattern, MAX_PATH, L"%ls\\pomodoro_log_*.csv", dir);
    hFind = FindFirstFileW(pattern, &ffd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            swprintf(path, MAX_PATH, L"%ls\\%ls", dir, ffd.cFileName);
            if (!DeleteFileW(path)) ok = 0;
        } while (FindNextFileW(hFind, &ffd));
        FindClose(hFind);
    }
    return ok;
}

static int choose_folder_dialog(HWND owner, const wchar_t* title, wchar_t* outPath, size_t outChars) {
    BROWSEINFOW bi = {0};
    LPITEMIDLIST pidl;
    int ok;
    if (!outPath || outChars == 0) return 0;
    outPath[0] = L'\0';
    bi.hwndOwner = owner;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    pidl = SHBrowseForFolderW(&bi);
    if (!pidl) return 0;
    ok = SHGetPathFromIDListW(pidl, outPath) ? 1 : 0;
    CoTaskMemFree(pidl);
    return ok;
}

static int switch_data_location(HWND hwnd, DataLocationMode mode, const wchar_t* customPath) {
    wchar_t targetDir[MAX_PATH];
    wchar_t prevDir[MAX_PATH];
    wchar_t prevCustomDir[MAX_PATH];
    DataLocationMode prevMode = g_data_location_mode;
    int migrated = 0;
    int failed = 0;
    int expected = 0;
    int copiedForSwitch = 0;

    if (is_running || is_paused) {
        MessageBoxW(hwnd, L"计时进行中，无法切换数据位置。", L"提示", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    if (!resolve_data_dir_for_mode(mode, customPath, targetDir, MAX_PATH)) {
        MessageBoxW(hwnd, L"目标数据位置不可用。", L"错误", MB_OK | MB_ICONERROR);
        return 0;
    }

    wcsncpy(prevDir, g_data_dir, MAX_PATH - 1);
    prevDir[MAX_PATH - 1] = L'\0';
    wcsncpy(prevCustomDir, g_custom_data_dir, MAX_PATH - 1);
    prevCustomDir[MAX_PATH - 1] = L'\0';
    ensure_directory_exists(targetDir);
    {
        DWORD targetAttr = GetFileAttributesW(targetDir);
        if (targetAttr == INVALID_FILE_ATTRIBUTES || !(targetAttr & FILE_ATTRIBUTE_DIRECTORY)) {
            MessageBoxW(hwnd, L"无法创建或访问目标数据目录。", L"错误", MB_OK | MB_ICONERROR);
            return 0;
        }
    }
    if (!save_settings()) return 0;

    if (prevDir[0] && _wcsicmp(prevDir, targetDir) != 0) {
        if (directory_has_data_files(prevDir) && directory_has_data_files(targetDir)) {
            MessageBoxW(hwnd,
                L"目标位置已经包含番茄钟数据。为避免覆盖或合并出错，本次切换已取消。\n请先导出目标数据，或选择一个空目录。",
                L"数据位置冲突", MB_OK | MB_ICONWARNING);
            return 0;
        }

        int okMain = migrate_data_files(prevDir, targetDir, &migrated, &failed, &expected);
        int okArchive = migrate_archive_logs(prevDir, targetDir, &migrated, &failed, &expected);
        if (!(okMain && okArchive)) {
            delete_data_files_in_directory(targetDir);
            MessageBoxW(hwnd,
                L"数据复制失败，已回滚目标目录；当前数据位置保持不变。",
                L"迁移失败", MB_OK | MB_ICONERROR);
            return 0;
        } else {
            copiedForSwitch = expected > 0;
        }
    }

    g_data_location_mode = mode;
    if (mode == DATA_LOC_CUSTOM && customPath) {
        wcsncpy(g_custom_data_dir, customPath, MAX_PATH - 1);
        g_custom_data_dir[MAX_PATH - 1] = L'\0';
    }
    wcsncpy(g_data_dir, targetDir, MAX_PATH - 1);
    g_data_dir[MAX_PATH - 1] = L'\0';
    if (!save_data_location_to_registry()) {
        g_data_location_mode = prevMode;
        wcsncpy(g_custom_data_dir, prevCustomDir, MAX_PATH - 1);
        g_custom_data_dir[MAX_PATH - 1] = L'\0';
        wcsncpy(g_data_dir, prevDir, MAX_PATH - 1);
        g_data_dir[MAX_PATH - 1] = L'\0';
        build_data_file_paths();
        if (copiedForSwitch) delete_data_files_in_directory(targetDir);
        MessageBoxW(hwnd, L"无法保存新的数据位置，切换已回滚。", L"切换失败", MB_OK | MB_ICONERROR);
        return 0;
    }
    build_data_file_paths();
    repair_settings_recovery_chain();
    load_settings();
    pomodoro_count = get_today_count_from_storage();
    refresh_today_count_if_day_changed(hwnd, 1);
    load_heatmap_data();
    if (g_hHeatmapWnd) InvalidateRect(g_hHeatmapWnd, NULL, TRUE);
    refresh_timer_icon_by_state(hwnd);
    if (_wcsicmp(prevDir, targetDir) != 0) {
        wchar_t okMsg[384];
        if (expected > 0) {
            swprintf(okMsg, 384, L"数据位置已安全切换到:\n%ls\n共迁移 %d/%d 个文件。", targetDir, migrated, expected);
        } else {
            swprintf(okMsg, 384, L"数据位置已切换到:\n%ls\n旧位置无可迁移文件。", targetDir);
        }
        MessageBoxW(hwnd, okMsg, L"提示", MB_OK | MB_ICONINFORMATION);
    }
    if (copiedForSwitch) {
        if (!delete_data_files_in_directory(prevDir)) {
            MessageBoxW(hwnd,
                L"新数据位置已经启用，但部分旧文件无法删除。新位置数据完整，旧文件可稍后手动清理。",
                L"旧文件未完全清理", MB_OK | MB_ICONWARNING);
        } else {
            RemoveDirectoryW(prevDir);
        }
    }
    return 1;
}

static int export_data_to_folder(HWND hwnd) {
    wchar_t outDir[MAX_PATH];
    wchar_t dst[MAX_PATH];
    wchar_t msg[384];
    WIN32_FIND_DATAW ffd;
    HANDLE hFind;
    wchar_t pattern[MAX_PATH];
    DWORD attr;
    wchar_t srcArchive[MAX_PATH];
    wchar_t dstArchive[MAX_PATH];
    int expected = 0;
    int copied = 0;
    int failed = 0;

    if (is_running || is_paused) {
        MessageBoxW(hwnd, L"计时进行中，无法导出数据。", L"提示", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    if (!choose_folder_dialog(hwnd, L"选择导出目录", outDir, MAX_PATH)) return 0;

    join_path(dst, MAX_PATH, outDir, L"pomodoro_settings.json");
    attr = GetFileAttributesW(g_settings_path);
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        expected++;
        if (copy_if_exists(g_settings_path, dst)) {
            copied++;
        } else {
            failed++;
        }
    }

    join_path(dst, MAX_PATH, outDir, L"pomodoro_settings.json.bak");
    attr = GetFileAttributesW(g_settings_bak_path);
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        expected++;
        if (copy_if_exists(g_settings_bak_path, dst)) {
            copied++;
        } else {
            failed++;
        }
    }

    join_path(dst, MAX_PATH, outDir, L"pomodoro_settings.json.tmp");
    attr = GetFileAttributesW(g_settings_tmp_path);
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        expected++;
        if (copy_if_exists(g_settings_tmp_path, dst)) {
            copied++;
        } else {
            failed++;
        }
    }

    attr = GetFileAttributesW(g_log_path);
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        expected++;
        join_path(dst, MAX_PATH, outDir, L"pomodoro_log.csv");
        if (copy_if_exists(g_log_path, dst)) {
            copied++;
        } else {
            failed++;
        }
    }

    attr = GetFileAttributesW(g_adjustments_path);
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        expected++;
        join_path(dst, MAX_PATH, outDir, L"pomodoro_adjustments.csv");
        if (copy_if_exists(g_adjustments_path, dst)) {
            copied++;
        } else {
            failed++;
        }
    }

    swprintf(pattern, MAX_PATH, L"%ls\\pomodoro_log_*.csv", g_data_dir);
    hFind = FindFirstFileW(pattern, &ffd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            swprintf(srcArchive, MAX_PATH, L"%ls\\%ls", g_data_dir, ffd.cFileName);
            swprintf(dstArchive, MAX_PATH, L"%ls\\%ls", outDir, ffd.cFileName);
            expected++;
            if (CopyFileW(srcArchive, dstArchive, FALSE)) {
                copied++;
            } else {
                failed++;
            }
        } while (FindNextFileW(hFind, &ffd));
        FindClose(hFind);
    }

    if (expected == 0) {
        MessageBoxW(hwnd, L"导出完成：未发现可导出的数据文件。", L"提示", MB_OK | MB_ICONINFORMATION);
    } else if (failed > 0) {
        swprintf(msg, 384, L"导出完成：成功 %d/%d，失败 %d。", copied, expected, failed);
        MessageBoxW(hwnd, msg, L"提示", MB_OK | MB_ICONWARNING);
    } else {
        swprintf(msg, 384, L"导出完成：成功 %d/%d。", copied, expected);
        MessageBoxW(hwnd, msg, L"提示", MB_OK | MB_ICONINFORMATION);
    }
    return 1;
}

static int import_data_from_folder(HWND hwnd) {
    wchar_t inDir[MAX_PATH];
    wchar_t backupDir[MAX_PATH];
    wchar_t msg[384];
    time_t now;
    struct tm* tmNow;
    int expected = 0;
    int copied = 0;
    int failed = 0;
    int backupExpected = 0;
    int backupCopied = 0;
    int backupFailed = 0;
    int restoreExpected = 0;
    int restoreCopied = 0;
    int restoreFailed = 0;
    int okMain;
    int okArchive;

    if (is_running || is_paused) {
        MessageBoxW(hwnd, L"计时进行中，无法导入数据。", L"提示", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    if (!choose_folder_dialog(hwnd, L"选择导入目录", inDir, MAX_PATH)) return 0;
    if (_wcsicmp(inDir, g_data_dir) == 0) {
        MessageBoxW(hwnd, L"不能从当前正在使用的数据目录导入。", L"提示", MB_OK | MB_ICONINFORMATION);
        return 0;
    }
    if (!directory_has_data_files(inDir)) {
        MessageBoxW(hwnd, L"所选目录中没有可导入的番茄钟数据。", L"提示", MB_OK | MB_ICONINFORMATION);
        return 0;
    }
    if (MessageBoxW(hwnd,
        L"导入会用所选备份替换当前数据。程序会先在当前数据目录创建完整备份；是否继续？",
        L"确认导入", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) {
        return 0;
    }

    ensure_directory_exists(g_data_dir);
    now = time(NULL);
    tmNow = localtime(&now);
    if (tmNow) {
        swprintf(backupDir, MAX_PATH, L"%ls\\import_backup_%04d%02d%02d_%02d%02d%02d_%lu",
            g_data_dir, tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday,
            tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec, GetCurrentProcessId());
    } else {
        swprintf(backupDir, MAX_PATH, L"%ls\\import_backup_%lu", g_data_dir, GetCurrentProcessId());
    }
    ensure_directory_exists(backupDir);
    if (GetFileAttributesW(backupDir) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(hwnd, L"无法创建导入前备份目录，导入已取消。", L"导入失败", MB_OK | MB_ICONERROR);
        return 0;
    }

    okMain = migrate_data_files(g_data_dir, backupDir, &backupCopied, &backupFailed, &backupExpected);
    okArchive = migrate_archive_logs(g_data_dir, backupDir, &backupCopied, &backupFailed, &backupExpected);
    if (!(okMain && okArchive)) {
        delete_data_files_in_directory(backupDir);
        RemoveDirectoryW(backupDir);
        MessageBoxW(hwnd, L"无法完整备份当前数据，导入已取消；当前数据未改动。", L"导入失败", MB_OK | MB_ICONERROR);
        return 0;
    }

    if (!delete_data_files_in_directory(g_data_dir)) {
        delete_data_files_in_directory(g_data_dir);
        migrate_data_files(backupDir, g_data_dir, &restoreCopied, &restoreFailed, &restoreExpected);
        migrate_archive_logs(backupDir, g_data_dir, &restoreCopied, &restoreFailed, &restoreExpected);
        MessageBoxW(hwnd, L"无法清理当前数据，已尝试恢复导入前状态；导入已取消。", L"导入失败", MB_OK | MB_ICONERROR);
        return 0;
    }

    okMain = migrate_data_files(inDir, g_data_dir, &copied, &failed, &expected);
    okArchive = migrate_archive_logs(inDir, g_data_dir, &copied, &failed, &expected);
    if (!(okMain && okArchive)) {
        delete_data_files_in_directory(g_data_dir);
        restoreCopied = restoreFailed = restoreExpected = 0;
        okMain = migrate_data_files(backupDir, g_data_dir, &restoreCopied, &restoreFailed, &restoreExpected);
        okArchive = migrate_archive_logs(backupDir, g_data_dir, &restoreCopied, &restoreFailed, &restoreExpected);
        if (okMain && okArchive) {
            MessageBoxW(hwnd, L"导入过程中发生错误，当前数据已从自动备份恢复。", L"导入失败", MB_OK | MB_ICONERROR);
        } else {
            MessageBoxW(hwnd, L"导入和自动恢复均未完整完成。请保留自动备份目录并手动恢复。", L"严重错误", MB_OK | MB_ICONERROR);
        }
        return 0;
    }

    repair_settings_recovery_chain();
    load_settings();
    pomodoro_count = get_today_count_from_storage();
    refresh_today_count_if_day_changed(hwnd, 1);
    load_heatmap_data();
    if (g_hHeatmapWnd) InvalidateRect(g_hHeatmapWnd, NULL, TRUE);
    refresh_timer_icon_by_state(hwnd);
    swprintf(msg, 384, L"导入完成：成功 %d/%d。\n导入前数据已备份到:\n%ls", copied, expected, backupDir);
    MessageBoxW(hwnd, msg, L"导入完成", MB_OK | MB_ICONINFORMATION);
    return 1;
}

static void maybe_archive_log_monthly(void) {
    WIN32_FILE_ATTRIBUTE_DATA fad;
    ULONGLONG size;
    FILE* in;
    FILE* keep;
    wchar_t keepPath[MAX_PATH];
    time_t now;
    struct tm* tmNow;
    char line[256];

    if (!GetFileAttributesExW(g_log_path, GetFileExInfoStandard, &fad)) return;
    size = ((ULONGLONG)fad.nFileSizeHigh << 32) | fad.nFileSizeLow;
    if (size < (ULONGLONG)(2 * 1024 * 1024)) return;

    in = _wfopen(g_log_path, L"r");
    if (!in) return;

    swprintf(keepPath, MAX_PATH, L"%ls.keep", g_log_path);
    keep = _wfopen(keepPath, L"w");
    if (!keep) {
        fclose(in);
        return;
    }

    now = time(NULL);
    tmNow = localtime(&now);

    while (fgets(line, sizeof(line), in)) {
        int y, m, d, hh, mm, ss;
        if (sscanf(line, "%d-%d-%d,%d:%d:%d", &y, &m, &d, &hh, &mm, &ss) == 6) {
            if (tmNow && y == (tmNow->tm_year + 1900) && m == (tmNow->tm_mon + 1)) {
                fputs(line, keep);
            } else {
                wchar_t monthPath[MAX_PATH];
                FILE* monthFp;
                swprintf(monthPath, MAX_PATH, L"%ls\\pomodoro_log_%04d-%02d.csv", g_data_dir, y, m);
                monthFp = _wfopen(monthPath, L"a");
                if (monthFp) {
                    fputs(line, monthFp);
                    fclose(monthFp);
                } else {
                    fputs(line, keep);
                }
            }
        } else {
            fputs(line, keep);
        }
    }

    fclose(in);
    fclose(keep);
    MoveFileExW(keepPath, g_log_path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

static void load_archive_logs(void) {
    WIN32_FIND_DATAW ffd;
    HANDLE hFind;
    wchar_t pattern[MAX_PATH];
    wchar_t filePath[MAX_PATH];
    FILE* fp;
    char line[128];

    swprintf(pattern, MAX_PATH, L"%ls\\pomodoro_log_*.csv", g_data_dir);
    hFind = FindFirstFileW(pattern, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        swprintf(filePath, MAX_PATH, L"%ls\\%ls", g_data_dir, ffd.cFileName);
        fp = _wfopen(filePath, L"r");
        if (!fp) continue;
        while (fgets(line, sizeof(line), fp)) {
            char date[11] = {0};
            int completedCount = 1;
            if (!parse_pomodoro_log_entry(line, date, &completedCount)) continue;
            add_day_count(g_day_counts, &g_day_count, date, completedCount);
        }
        fclose(fp);
    } while (FindNextFileW(hFind, &ffd));

    FindClose(hFind);
}

static void reset_defaults_keep_data(void) {
    settings.long_pomodoro_duration = 90;
    settings.long_pomodoro_count = 2;
    settings.short_pomodoro_duration = 45;
    settings.short_break_duration = 5;
    settings.long_break_duration = 15;
    settings.custom_duration = 10;
    settings.adjust_block_minutes = 5;
    settings.toast_auto_collapse_seconds = 10;
    settings.enable_clock_sound = 0;
    settings.enable_completion_sound = 1;
    settings.show_completion_dialog = 1;
    settings.default_pomodoro_is_long = 1;
    settings.default_break_is_long = 0;
    settings.enable_overtime_count_up = 1;

    if (!is_running && !is_paused) {
        if (idle_mode == IDLE_POMODORO) {
            idle_pomodoro_is_long = settings.default_pomodoro_is_long;
        } else if (idle_mode == IDLE_BREAK) {
            idle_break_is_long = settings.default_break_is_long;
        }
    }
}

// Initialize system metrics for dialog positioning
void init_system_metrics() {
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);
}

static void center_window_on_work_area(HWND hwnd) {
    RECT rect;
    RECT workArea;
    int width;
    int height;
    int x;
    int y;

    if (!hwnd) return;
    GetWindowRect(hwnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0)) {
        workArea.left = 0;
        workArea.top = 0;
        workArea.right = GetSystemMetrics(SM_CXSCREEN);
        workArea.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    x = workArea.left + ((workArea.right - workArea.left) - width) / 2;
    y = workArea.top + ((workArea.bottom - workArea.top) - height) / 2;
    if (x < workArea.left) x = workArea.left;
    if (y < workArea.top) y = workArea.top;

    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

static const wchar_t* g_input_title = L"输入";
static const wchar_t* g_input_label = L"请输入数字:";
static int g_input_default = 0;
static int g_input_min = 0;
static int g_input_max = 0;
static int g_input_result = 0;

INT_PTR CALLBACK InputDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    (void)lParam;
    switch (uMsg) {
        case WM_INITDIALOG: {
            wchar_t buf[32];
            SetWindowTextW(hwndDlg, g_input_title);
            SetDlgItemTextW(hwndDlg, IDC_INPUT_LABEL, g_input_label);
            SetDlgItemTextW(hwndDlg, IDOK, L"确定");
            SetDlgItemTextW(hwndDlg, IDCANCEL, L"取消");
            _itow(g_input_default, buf, 10);
            SetDlgItemTextW(hwndDlg, IDC_INPUT_EDIT, buf);
            center_window_on_work_area(hwndDlg);
            return TRUE;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                wchar_t buf[32];
                int value;
                GetDlgItemTextW(hwndDlg, IDC_INPUT_EDIT, buf, 31);
                value = _wtoi(buf);
                if (value < g_input_min || value > g_input_max) {
                    MessageBoxW(hwndDlg, L"输入超出允许范围。", L"错误", MB_OK | MB_ICONERROR);
                    return TRUE;
                }
                g_input_result = value;
                EndDialog(hwndDlg, IDOK);
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
        case WM_CLOSE:
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
    }
    return FALSE;
}

BOOL PromptForInteger(HWND hwndParent, const wchar_t* title, const wchar_t* label, int currentValue, int minValue, int maxValue, int* outValue) {
    INT_PTR result;
    g_input_title = title;
    g_input_label = label;
    g_input_default = currentValue;
    g_input_min = minValue;
    g_input_max = maxValue;
    result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_INPUT), hwndParent, InputDlgProc);
    if (result == IDOK) {
        *outValue = g_input_result;
        return TRUE;
    }
    return FALSE;
}

// Sets the application language and refreshes the UI
void SetLanguage(LANG *newLang) {
    g_lang = newLang;
    RefreshMenuText();
}

// Saves the selected language to the registry
void SaveLanguageSelectionToRegistry(void) {
    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\PomodoroTimer", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD langId = g_lang == &lang_en ? ID_MENU_LANG_EN :
                      g_lang == &lang_hu ? ID_MENU_LANG_HU :
                      g_lang == &lang_de ? ID_MENU_LANG_DE :
                      g_lang == &lang_it ? ID_MENU_LANG_IT :
                      g_lang == &lang_es ? ID_MENU_LANG_ES :
                      g_lang == &lang_fr ? ID_MENU_LANG_FR : ID_MENU_LANG_RU;
        RegSetValueExW(hKey, L"Language", 0, REG_DWORD, (const BYTE *)&langId, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

// Loads the selected language from the registry
BOOL LoadLanguageSelectionFromRegistry(void) {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\PomodoroTimer", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD langId, dataSize = sizeof(DWORD);
        if (RegQueryValueExW(hKey, L"Language", NULL, NULL, (LPBYTE)&langId, &dataSize) == ERROR_SUCCESS) {
            switch (langId) {
                case ID_MENU_LANG_EN: g_lang = &lang_en; break;
                case ID_MENU_LANG_HU: g_lang = &lang_hu; break;
                case ID_MENU_LANG_DE: g_lang = &lang_de; break;
                case ID_MENU_LANG_IT: g_lang = &lang_it; break;
                case ID_MENU_LANG_ES: g_lang = &lang_es; break;
                case ID_MENU_LANG_FR: g_lang = &lang_fr; break;
                case ID_MENU_LANG_RU: g_lang = &lang_ru; break;
                default: g_lang = &lang_en; break;
            }
            RegCloseKey(hKey);
            return TRUE;
        }
        RegCloseKey(hKey);
    }
    return FALSE;
}

// Refreshes menu text with current language
void RefreshMenuText(void) {
    if (!g_hMenu) return;
    
    ModifyMenu(g_hMenu, 1, MF_BYCOMMAND | MF_STRING, 1, g_lang->menu_start_pomodoro);
    ModifyMenu(g_hMenu, 2, MF_BYCOMMAND | MF_STRING, 2, g_lang->menu_start_break);
    ModifyMenu(g_hMenu, 3, MF_BYCOMMAND | MF_STRING, 3, g_lang->menu_start_long_break);
    ModifyMenu(g_hMenu, 4, MF_BYCOMMAND | MF_STRING | (settings.enable_clock_sound ? MF_CHECKED : 0), 4, g_lang->menu_clock_sound);
    ModifyMenu(g_hMenu, 5, MF_BYCOMMAND | MF_STRING | (autostart_enabled ? MF_CHECKED : 0), 5, g_lang->menu_autostart);
    ModifyMenu(g_hMenu, 9, MF_BYCOMMAND | MF_STRING | (settings.show_completion_dialog ? MF_CHECKED : 0), 9, g_lang->menu_show_dialog);
    ModifyMenu(g_hMenu, 6, MF_BYCOMMAND | MF_STRING, 6, g_lang->menu_settings);
    ModifyMenu(g_hMenu, 7, MF_BYCOMMAND | MF_STRING, 7, g_lang->menu_about);
    ModifyMenu(g_hMenu, ID_MENU_RESET_COUNT, MF_BYCOMMAND | MF_STRING, ID_MENU_RESET_COUNT, g_lang->menu_reset_count);
    ModifyMenu(g_hMenu, 8, MF_BYCOMMAND | MF_STRING, 8, g_lang->menu_exit);

    if (g_hLangMenu) {
        CheckMenuItem(g_hLangMenu, ID_MENU_LANG_EN, MF_BYCOMMAND | (g_lang == &lang_en ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(g_hLangMenu, ID_MENU_LANG_HU, MF_BYCOMMAND | (g_lang == &lang_hu ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(g_hLangMenu, ID_MENU_LANG_DE, MF_BYCOMMAND | (g_lang == &lang_de ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(g_hLangMenu, ID_MENU_LANG_IT, MF_BYCOMMAND | (g_lang == &lang_it ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(g_hLangMenu, ID_MENU_LANG_ES, MF_BYCOMMAND | (g_lang == &lang_es ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(g_hLangMenu, ID_MENU_LANG_FR, MF_BYCOMMAND | (g_lang == &lang_fr ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(g_hLangMenu, ID_MENU_LANG_RU, MF_BYCOMMAND | (g_lang == &lang_ru ? MF_CHECKED : MF_UNCHECKED));
    }
}

// Create dynamic tray icon with text and dots
HICON create_tray_icon(const wchar_t* text, int dots) {
    int visible_dots = get_visible_dots(dots);
    HDC hdc = NULL;
    HDC hdcMask = NULL;
    HBITMAP hBitmap = NULL;
    HBITMAP hMask = NULL;
    HGDIOBJ oldBitmap = NULL;
    HGDIOBJ oldMaskBitmap = NULL;
    HGDIOBJ oldFont = NULL;
    HGDIOBJ oldBrush = NULL;
    HFONT hFont = NULL;
    HBRUSH bgBrush = NULL;
    HBRUSH dotBrush = NULL;
    HICON hIcon = NULL;
    void* bits = NULL;
    size_t textLength = wcslen(text);
    int fontHeight = textLength >= 4 ? 15 : (textLength == 3 ? 20 : 27);

    // Return cached icon if nothing has changed
    if (last_icon && wcscmp(text, last_text) == 0 && visible_dots == last_dots) {
        return last_icon;
    }

    // Clean up previous icon
    if (last_icon) {
        DestroyIcon(last_icon);
        last_icon = NULL;
    }

    // Create device context and bitmap
    hdc = CreateCompatibleDC(NULL);
    if (!hdc) goto cleanup;

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = 32;
    bmi.bmiHeader.biHeight = -32;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!hBitmap || !bits) goto cleanup;
    oldBitmap = SelectObject(hdc, hBitmap);
    if (!oldBitmap || oldBitmap == HGDI_ERROR) {
        oldBitmap = NULL;
        goto cleanup;
    }

    // Draw background (dark red)
    bgBrush = CreateSolidBrush(RGB(139, 0, 0));
    if (!bgBrush) goto cleanup;
    RECT rect = {0, 0, 32, 32};
    FillRect(hdc, &rect, bgBrush);
    DeleteObject(bgBrush);
    bgBrush = NULL;

    // Set up text rendering
    hFont = CreateFontW(
        fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Arial"
    );
    if (!hFont) goto cleanup;
    oldFont = SelectObject(hdc, hFont);
    if (!oldFont || oldFont == HGDI_ERROR) {
        oldFont = NULL;
        goto cleanup;
    }
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);

    // Draw text centered
    SIZE textSize;
    GetTextExtentPoint32W(hdc, text, (int)wcslen(text), &textSize);
    int textX = (32 - textSize.cx) / 2;
    int textY = (32 - textSize.cy) / 2 - 2;
    
    // Slight adjustment for play button
    if (wcscmp(text, L"\u25BA") == 0) {
        textX += 2;
    }
    
    TextOutW(hdc, textX, textY, text, (int)wcslen(text));

    // Draw progress dots (light green)
    dotBrush = CreateSolidBrush(RGB(144, 238, 144));
    if (!dotBrush) goto cleanup;
    oldBrush = SelectObject(hdc, dotBrush);
    if (!oldBrush || oldBrush == HGDI_ERROR) {
        oldBrush = NULL;
        goto cleanup;
    }
    for (int i = 0; i < visible_dots; i++) {
        int dotX = 4 + i * 8;
        int dotY = 30;
        Ellipse(hdc, dotX - 3, dotY - 3, dotX + 3, dotY + 3);
    }
    SelectObject(hdc, oldBrush);
    oldBrush = NULL;
    DeleteObject(dotBrush);
    dotBrush = NULL;

    // Create icon mask
    hMask = CreateCompatibleBitmap(hdc, 32, 32);
    hdcMask = CreateCompatibleDC(hdc);
    if (!hMask || !hdcMask) goto cleanup;
    oldMaskBitmap = SelectObject(hdcMask, hMask);
    if (!oldMaskBitmap || oldMaskBitmap == HGDI_ERROR) {
        oldMaskBitmap = NULL;
        goto cleanup;
    }
    SetBkColor(hdc, RGB(139, 0, 0));
    BitBlt(hdcMask, 0, 0, 32, 32, hdc, 0, 0, SRCCOPY);

    // Create icon
    ICONINFO iconInfo = {0};
    iconInfo.fIcon = TRUE;
    iconInfo.hbmColor = hBitmap;
    iconInfo.hbmMask = hMask;
    hIcon = CreateIconIndirect(&iconInfo);

cleanup:
    if (oldBrush && hdc) SelectObject(hdc, oldBrush);
    if (oldFont && hdc) SelectObject(hdc, oldFont);
    if (oldMaskBitmap && hdcMask) SelectObject(hdcMask, oldMaskBitmap);
    if (oldBitmap && hdc) SelectObject(hdc, oldBitmap);
    if (dotBrush) DeleteObject(dotBrush);
    if (bgBrush) DeleteObject(bgBrush);
    if (hFont) DeleteObject(hFont);
    if (hdcMask) DeleteDC(hdcMask);
    if (hdc) DeleteDC(hdc);
    if (hBitmap) DeleteObject(hBitmap);
    if (hMask) DeleteObject(hMask);

    if (!hIcon) {
        hIcon = CopyIcon(LoadIconW(NULL, IDI_APPLICATION));
    }

    // Cache for later
    last_icon = hIcon;
    wcscpy(last_text, text);
    last_dots = visible_dots;

    return hIcon;
}

// Update system tray icon and tooltip
void update_tray_icon(HWND hwnd, const wchar_t* text, int dots, int seconds) {
    wchar_t tooltip[128];
    (void)hwnd;
    if ((is_running || is_paused) && (current_timer_mode != TIMER_NONE || is_overtime)) {
        int min = seconds / 60, sec = seconds % 60;
        const wchar_t* pause_suffix = is_paused ? L" (已暂停)" : L"";
        if (is_overtime) {
            swprintf(tooltip, sizeof(tooltip)/sizeof(tooltip[0]), L"超时正计时 +%02d:%02d%s", min, sec, pause_suffix);
        } else if (is_count_up_timer) {
            swprintf(tooltip, sizeof(tooltip)/sizeof(tooltip[0]), L"正计时 %02d:%02d%s", min, sec, pause_suffix);
        } else if (current_timer_mode == TIMER_SHORT_POMODORO) {
            swprintf(tooltip, sizeof(tooltip)/sizeof(tooltip[0]), L"短番茄钟 %02d:%02d%s", min, sec, pause_suffix);
        } else if (current_timer_mode == TIMER_LONG_POMODORO) {
            swprintf(tooltip, sizeof(tooltip)/sizeof(tooltip[0]), L"长番茄钟 %02d:%02d%s", min, sec, pause_suffix);
        } else if (current_timer_mode == TIMER_SHORT_BREAK) {
            swprintf(tooltip, sizeof(tooltip)/sizeof(tooltip[0]), L"短休息 %02d:%02d%s", min, sec, pause_suffix);
        } else if (current_timer_mode == TIMER_LONG_BREAK) {
            swprintf(tooltip, sizeof(tooltip)/sizeof(tooltip[0]), L"长休息 %02d:%02d%s", min, sec, pause_suffix);
        } else if (current_timer_mode == TIMER_CUSTOM) {
            swprintf(tooltip, sizeof(tooltip)/sizeof(tooltip[0]), L"自定义计时 %02d:%02d%s", min, sec, pause_suffix);
        } else {
            swprintf(tooltip, sizeof(tooltip)/sizeof(tooltip[0]), L"%02d:%02d%s", min, sec, pause_suffix);
        }
    } else {
        if (idle_mode == IDLE_BREAK) {
            wcsncpy(tooltip, idle_break_is_long ? L"点击开始长休息" : L"点击开始短休息", sizeof(tooltip)/sizeof(tooltip[0]) - 1);
            tooltip[sizeof(tooltip)/sizeof(tooltip[0]) - 1] = L'\0';
        } else if (idle_mode == IDLE_COUNT_UP) {
            wcsncpy(tooltip, L"点击开始正计时", sizeof(tooltip)/sizeof(tooltip[0]) - 1);
            tooltip[sizeof(tooltip)/sizeof(tooltip[0]) - 1] = L'\0';
        } else if (idle_mode == IDLE_CUSTOM) {
            wcsncpy(tooltip, L"点击开始自定义计时", sizeof(tooltip)/sizeof(tooltip[0]) - 1);
            tooltip[sizeof(tooltip)/sizeof(tooltip[0]) - 1] = L'\0';
        } else {
            wcsncpy(tooltip, idle_pomodoro_is_long ? L"点击开始长番茄钟" : L"点击开始短番茄钟", sizeof(tooltip)/sizeof(tooltip[0]) - 1);
            tooltip[sizeof(tooltip)/sizeof(tooltip[0]) - 1] = L'\0';
        }
    }
    wcsncpy(nid.szTip, tooltip, sizeof(nid.szTip)/sizeof(nid.szTip[0]) - 1);
    nid.szTip[sizeof(nid.szTip)/sizeof(nid.szTip[0]) - 1] = L'\0';

    nid.hIcon = create_tray_icon(text, dots);
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

static int play_resource_sound_ex(const char* resourceName, DWORD flags) {
    HRSRC hRes = FindResourceA(NULL, resourceName, "WAV");
    if (!hRes) return 0;

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return 0;

    LPVOID pData = LockResource(hData);
    if (!pData) return 0;

    return PlaySoundA((LPCSTR)pData, NULL, flags | SND_MEMORY) ? 1 : 0;
}

static void stop_clock_loop_sound(void) {
    if (g_clock_loop_playing) {
        PlaySoundA(NULL, NULL, 0);
        g_clock_loop_playing = 0;
    }
}

static void start_clock_loop_sound(void) {
    if (g_clock_loop_playing || !settings.enable_clock_sound) return;

    if (!play_resource_sound_ex("CLOCK_WAV", SND_ASYNC | SND_LOOP)) {
        if (PlaySoundW(L"clock.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP)) {
            g_clock_loop_playing = 1;
        }
    } else {
        g_clock_loop_playing = 1;
    }
}

static void stop_timer_thread_if_needed(void) {
    if (is_running) {
        is_running = 0;
    }
    if (timer_thread_handle != NULL) {
        WaitForSingleObject(timer_thread_handle, INFINITE);
        CloseHandle(timer_thread_handle);
        timer_thread_handle = NULL;
    }
}

static int launch_timer_thread(HWND hwnd) {
    timer_thread_handle = CreateThread(NULL, 0, timer_thread, hwnd, 0, NULL);
    if (timer_thread_handle) return 1;

    is_running = 0;
    is_paused = 0;
    remaining_seconds = 0;
    current_timer_mode = TIMER_NONE;
    clear_count_up_state();
    stop_clock_loop_sound();
    MessageBoxW(hwnd, L"无法创建计时线程，计时未启动。", L"启动失败", MB_OK | MB_ICONERROR);
    return 0;
}

static int is_pomodoro_mode(TimerMode mode) {
    return mode == TIMER_LONG_POMODORO || mode == TIMER_SHORT_POMODORO;
}

static int get_long_pomodoro_duration(void) {
    return settings.long_pomodoro_duration;
}

static TimerMode get_default_pomodoro_mode(void) {
    return settings.default_pomodoro_is_long ? TIMER_LONG_POMODORO : TIMER_SHORT_POMODORO;
}

static TimerMode get_default_break_mode(void) {
    return settings.default_break_is_long ? TIMER_LONG_BREAK : TIMER_SHORT_BREAK;
}

static void clear_count_up_state(void) {
    is_count_up_timer = 0;
    count_up_credited = 0;
    count_up_threshold_seconds = 0;
}

static void close_toast_notification_if_open(void) {
    if (g_hToastWnd) {
        DestroyWindow(g_hToastWnd);
        g_hToastWnd = NULL;
    }
}

static void clear_overtime_state(void) {
    if (is_overtime) {
        close_toast_notification_if_open();
    }
    is_overtime = 0;
    overtime_seconds = 0;
    overtime_source_mode = TIMER_NONE;
}

static void credit_count_up_thresholds(HWND hwnd) {
    int reached;
    int delta;
    int targetCount;

    if (!is_count_up_timer || current_timer_mode != TIMER_COUNT_UP || count_up_threshold_seconds <= 0) return;
    reached = remaining_seconds / count_up_threshold_seconds;
    if (reached <= count_up_credited) return;
    delta = reached - count_up_credited;

    if (record_completed_pomodoros(delta)) {
        pomodoro_count = get_today_count_from_storage();
    } else {
        targetCount = clamp_int(get_today_count_from_storage() + delta, 0, 9999);
        if (!sync_today_count_to_target(targetCount)) {
            OutputDebugStringA("Failed to persist count-up Pomodoro threshold.\n");
            return;
        }
        pomodoro_count = targetCount;
    }

    count_up_credited = reached;
    save_settings();
    if (settings.enable_completion_sound) Beep(880, 150);
    if (g_hHeatmapWnd) InvalidateRect(g_hHeatmapWnd, NULL, TRUE);
    refresh_timer_icon_by_state(hwnd);
}

static void set_idle_mode_after_manual_stop(void) {
    TimerMode effective_mode = is_overtime ? overtime_source_mode : current_timer_mode;
    if (is_pomodoro_mode(effective_mode)) {
        idle_mode = IDLE_BREAK;
        idle_break_is_long = settings.default_break_is_long;
    } else if (effective_mode == TIMER_SHORT_BREAK || effective_mode == TIMER_LONG_BREAK) {
        idle_mode = IDLE_POMODORO;
        idle_pomodoro_is_long = settings.default_pomodoro_is_long;
        idle_break_is_long = 0;
    } else if (effective_mode == TIMER_CUSTOM) {
        idle_mode = IDLE_CUSTOM;
    } else if (effective_mode == TIMER_COUNT_UP) {
        idle_mode = IDLE_BREAK;
        idle_break_is_long = settings.default_break_is_long;
    }
}

static void refresh_timer_icon_by_state(HWND hwnd) {
    if (is_running || is_paused) {
        wchar_t display_text[16];
        if (is_overtime) {
            if (overtime_seconds < 60) {
                swprintf(display_text, sizeof(display_text)/sizeof(display_text[0]), L"+%d", overtime_seconds);
            } else {
                swprintf(display_text, sizeof(display_text)/sizeof(display_text[0]), L"+%d", overtime_seconds / 60);
            }
            update_tray_icon(hwnd, is_paused ? L"II" : display_text, pomodoro_count, overtime_seconds);
        } else {
            if (remaining_seconds < 60) {
                _itow(remaining_seconds, display_text, 10);
            } else {
                _itow(remaining_seconds / 60, display_text, 10);
            }
            update_tray_icon(hwnd, is_paused ? L"II" : display_text, pomodoro_count, remaining_seconds);
        }
    } else {
        update_tray_icon(hwnd, L"\u25BA", pomodoro_count, 0);
    }
}

static void start_mode_from_menu(HWND hwnd, TimerMode mode) {
    if (is_running || is_paused) {
        stop_timer_thread_if_needed();
        is_paused = 0;
        current_timer_mode = TIMER_NONE;
        remaining_seconds = 0;
        clear_count_up_state();
        clear_overtime_state();
    }

    if (mode == TIMER_LONG_POMODORO) {
        idle_mode = IDLE_POMODORO;
        idle_pomodoro_is_long = 1;
        start_timer(hwnd, get_long_pomodoro_duration(), TIMER_LONG_POMODORO);
    } else if (mode == TIMER_SHORT_POMODORO) {
        idle_mode = IDLE_POMODORO;
        idle_pomodoro_is_long = 0;
        start_timer(hwnd, settings.short_pomodoro_duration, TIMER_SHORT_POMODORO);
    } else if (mode == TIMER_SHORT_BREAK) {
        idle_mode = IDLE_BREAK;
        idle_break_is_long = 0;
        start_timer(hwnd, settings.short_break_duration, TIMER_SHORT_BREAK);
    } else if (mode == TIMER_LONG_BREAK) {
        idle_mode = IDLE_BREAK;
        idle_break_is_long = 1;
        start_timer(hwnd, settings.long_break_duration, TIMER_LONG_BREAK);
    } else if (mode == TIMER_COUNT_UP) {
        idle_mode = IDLE_COUNT_UP;
        start_timer(hwnd, 0, TIMER_COUNT_UP);
    } else if (mode == TIMER_CUSTOM) {
        idle_mode = IDLE_CUSTOM;
        start_timer(hwnd, settings.custom_duration, TIMER_CUSTOM);
    }

    save_settings();
    refresh_timer_icon_by_state(hwnd);
}

static void start_current_idle_mode(HWND hwnd) {
    if (idle_mode == IDLE_BREAK) {
        if (idle_break_is_long) {
            start_mode_from_menu(hwnd, TIMER_LONG_BREAK);
        } else {
            start_mode_from_menu(hwnd, TIMER_SHORT_BREAK);
        }
    } else if (idle_mode == IDLE_COUNT_UP) {
        start_mode_from_menu(hwnd, TIMER_COUNT_UP);
    } else if (idle_mode == IDLE_CUSTOM) {
        start_mode_from_menu(hwnd, TIMER_CUSTOM);
    } else {
        start_mode_from_menu(hwnd, idle_pomodoro_is_long ? TIMER_LONG_POMODORO : TIMER_SHORT_POMODORO);
    }
}

// Play sound from resource
void play_resource_sound(const char* resourceName) {
    const wchar_t* fallbackPath = strcmp(resourceName, "CLOCK_WAV") == 0 ? L"clock.wav" : L"ding.wav";
    if (!play_resource_sound_ex(resourceName, SND_ASYNC)) {
        PlaySoundW(fallbackPath, NULL, SND_FILENAME | SND_ASYNC);
    }
}

// Timer thread function
DWORD WINAPI timer_thread(LPVOID lpParam) {
    start_clock_loop_sound();

    ULONGLONG last_tick = GetTickCount64();
    int last_shown_seconds = -1;
    HWND hwnd = (HWND)lpParam;

    while (is_running) {
        ULONGLONG now = GetTickCount64();
        ULONGLONG elapsed_ms = now - last_tick;

        if (elapsed_ms >= 1000) {
            int elapsed_sec = (int)(elapsed_ms / 1000);
            if (is_overtime) {
                if (overtime_seconds <= 2147483647 - elapsed_sec) {
                    overtime_seconds += elapsed_sec;
                }
            } else if (is_count_up_timer) {
                if (remaining_seconds <= 2147483647 - elapsed_sec) {
                    remaining_seconds += elapsed_sec;
                }
            } else {
                remaining_seconds -= elapsed_sec;
            }
            last_tick += (ULONGLONG)elapsed_sec * 1000;

            if (!is_count_up_timer && !is_overtime && remaining_seconds < 0) remaining_seconds = 0;

            if (!is_count_up_timer && !is_overtime && remaining_seconds > 0 && remaining_seconds <= 10 && settings.enable_clock_sound) {
                Beep(440, 100);
            }

            if (is_count_up_timer && !is_overtime) credit_count_up_thresholds(hwnd);

            if (is_overtime) {
                if (overtime_seconds != last_shown_seconds) {
                    wchar_t display_text[16];
                    if (overtime_seconds < 60) {
                        swprintf(display_text, sizeof(display_text)/sizeof(display_text[0]), L"+%d", overtime_seconds);
                    } else {
                        swprintf(display_text, sizeof(display_text)/sizeof(display_text[0]), L"+%d", overtime_seconds / 60);
                    }
                    update_tray_icon(hwnd, display_text, pomodoro_count, overtime_seconds);
                    last_shown_seconds = overtime_seconds;
                }
            } else {
                if (remaining_seconds != last_shown_seconds) {
                    wchar_t display_text[16];
                    if (remaining_seconds < 60) {
                        _itow(remaining_seconds, display_text, 10);
                    } else {
                        _itow(remaining_seconds / 60, display_text, 10);
                    }
                    update_tray_icon(hwnd, display_text, pomodoro_count, remaining_seconds);
                    last_shown_seconds = remaining_seconds;
                }
            }
        }

        if (!is_count_up_timer && !is_overtime && remaining_seconds <= 0) {
            stop_clock_loop_sound();

            int completed_mode = (int)current_timer_mode;

            if (is_pomodoro_mode(current_timer_mode)) {
                int completedCount = current_timer_mode == TIMER_LONG_POMODORO ? settings.long_pomodoro_count : 1;
                if (record_completed_pomodoros(completedCount)) {
                    pomodoro_count = get_today_count_from_storage();
                } else {
                    pomodoro_count = clamp_int(get_today_count_from_storage() + completedCount, 0, 9999);
                    if (!sync_today_count_to_target(pomodoro_count)) {
                        OutputDebugStringA("Failed to persist completion to both log and adjustment files.\n");
                    }
                }
            }

            if (is_pomodoro_mode((TimerMode)completed_mode)) {
                idle_mode = IDLE_BREAK;
                idle_break_is_long = settings.default_break_is_long;
            } else if (completed_mode == TIMER_SHORT_BREAK || completed_mode == TIMER_LONG_BREAK) {
                idle_mode = IDLE_POMODORO;
                idle_pomodoro_is_long = settings.default_pomodoro_is_long;
                idle_break_is_long = 0;
            } else if (completed_mode == TIMER_CUSTOM) {
                idle_mode = IDLE_CUSTOM;
            }

            if (settings.enable_overtime_count_up) {
                is_overtime = 1;
                overtime_seconds = 0;
                overtime_source_mode = (TimerMode)completed_mode;
                current_timer_mode = TIMER_NONE;
                clear_count_up_state();
                save_settings();

                if (settings.enable_completion_sound) {
                    play_resource_sound("DING_WAV");
                }

                if (settings.show_completion_dialog) {
                    PostMessage(hwnd, WM_TOAST_NOTIFY, (WPARAM)completed_mode, 0);
                }

                wchar_t display_text[16] = L"+0";
                update_tray_icon(hwnd, display_text, pomodoro_count, 0);
                last_shown_seconds = 0;
                last_tick = GetTickCount64();
                continue;
            }

            is_running = 0;
            is_paused = 0;

            current_timer_mode = TIMER_NONE;
            clear_count_up_state();
            clear_overtime_state();
            save_settings();

            update_tray_icon(hwnd, L"\u25BA", pomodoro_count, 0);
            if (settings.enable_completion_sound) {
                play_resource_sound("DING_WAV");
            }

            // Post a message to the main thread to show completion notification (create toast on GUI thread)
            if (settings.show_completion_dialog) {
                PostMessage(hwnd, WM_TOAST_NOTIFY, (WPARAM)completed_mode, 0);
            }

            return 0;
        }

        ULONGLONG next_boundary = last_tick + 1000;
        now = GetTickCount64();
        long sleep_ms = (long)(next_boundary - now);

        if (sleep_ms < 1) sleep_ms = 1;
        if (sleep_ms > 50) sleep_ms = 50;

        Sleep(sleep_ms);
    }

    stop_clock_loop_sound();

    return 0;
}

// Start timer with specified duration
void start_timer(HWND hwnd, int duration_minutes, TimerMode mode) {
    stop_timer_thread_if_needed();
    clear_overtime_state();

    is_count_up_timer = mode == TIMER_COUNT_UP;
    count_up_credited = 0;
    count_up_threshold_seconds = is_count_up_timer ? settings.short_pomodoro_duration * 60 : 0;
    remaining_seconds = is_count_up_timer ? 0 : duration_minutes * 60;
    is_running = 1;
    is_paused = 0;
    current_timer_mode = mode;
    launch_timer_thread(hwnd);
}

// Check if autostart is enabled in registry
int is_autostart_enabled() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD type;
        if (RegQueryValueExW(hKey, L"PomodoroTimer", NULL, &type, NULL, NULL) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return 1;
        }
        RegCloseKey(hKey);
    }
    return 0;
}

// Enable or disable autostart in registry
int set_autostart(int enable) {
    HKEY hKey;
    LONG result;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        if (enable) {
            wchar_t path[MAX_PATH];
            wchar_t command[MAX_PATH + 3];
            if (!GetModuleFileNameW(NULL, path, MAX_PATH)) {
                RegCloseKey(hKey);
                return 0;
            }
            swprintf(command, MAX_PATH + 3, L"\"%ls\"", path);
            result = RegSetValueExW(hKey, L"PomodoroTimer", 0, REG_SZ,
                (const BYTE*)command, (DWORD)((wcslen(command) + 1) * sizeof(wchar_t)));
        } else {
            result = RegDeleteValueW(hKey, L"PomodoroTimer");
            if (result == ERROR_FILE_NOT_FOUND) result = ERROR_SUCCESS;
        }
        RegCloseKey(hKey);
        return result == ERROR_SUCCESS;
    }
    return 0;
}

// Show settings dialog
void ShowSettingsDialog(HWND hwndParent) {
    INT_PTR result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), hwndParent, SettingsDlgProc);
    if (result == -1) {
        MessageBoxW(hwndParent, L"设置窗口加载失败，请检查程序资源是否正确打包。", L"错误", MB_OK | MB_ICONERROR);
    }
}

// Settings dialog procedure
INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    (void)lParam;
    switch (uMsg) {
        case WM_INITDIALOG: {
            // Set dialog title localized
            SetWindowTextW(hwndDlg, g_lang->settings_title);
            // Set label texts (assume label IDs: 201, 202, 203)
            SetDlgItemTextW(hwndDlg, 201, L"长番茄钟时长(分钟):");
            SetDlgItemTextW(hwndDlg, IDC_LABEL_SHORT_POMODORO, L"短番茄钟时长(分钟):");
            SetDlgItemTextW(hwndDlg, 202, g_lang->settings_short_break);
            SetDlgItemTextW(hwndDlg, 203, g_lang->settings_long_break);
            SetDlgItemTextW(hwndDlg, IDC_LABEL_CUSTOM, L"自定义计时时长(分钟):");
            SetDlgItemTextW(hwndDlg, IDC_CLOCK_SOUND, g_lang->settings_enable_sound);
            SetDlgItemTextW(hwndDlg, IDC_AUTOSTART, g_lang->settings_autostart);
            SetDlgItemTextW(hwndDlg, IDC_OK, L"确定");
            SetDlgItemTextW(hwndDlg, IDC_CANCEL, L"取消");

            // Set edit values
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", settings.long_pomodoro_duration);
            SetDlgItemTextA(hwndDlg, IDC_POMODORO, buf);
            snprintf(buf, sizeof(buf), "%d", settings.short_pomodoro_duration);
            SetDlgItemTextA(hwndDlg, IDC_SHORT_POMODORO, buf);
            snprintf(buf, sizeof(buf), "%d", settings.short_break_duration);
            SetDlgItemTextA(hwndDlg, IDC_SHORT_BREAK, buf);
            snprintf(buf, sizeof(buf), "%d", settings.long_break_duration);
            SetDlgItemTextA(hwndDlg, IDC_LONG_BREAK, buf);
            snprintf(buf, sizeof(buf), "%d", settings.custom_duration);
            SetDlgItemTextA(hwndDlg, IDC_CUSTOM_TIMER, buf);
            SendDlgItemMessageA(hwndDlg, IDC_CLOCK_SOUND, BM_SETCHECK, settings.enable_clock_sound ? BST_CHECKED : BST_UNCHECKED, 0);
            SendDlgItemMessageA(hwndDlg, IDC_AUTOSTART, BM_SETCHECK, autostart_enabled ? BST_CHECKED : BST_UNCHECKED, 0);
            CheckRadioButton(hwndDlg, IDC_DEFAULT_LONG_POMODORO, IDC_DEFAULT_SHORT_POMODORO,
                settings.default_pomodoro_is_long ? IDC_DEFAULT_LONG_POMODORO : IDC_DEFAULT_SHORT_POMODORO);
            CheckRadioButton(hwndDlg, IDC_DEFAULT_SHORT_BREAK, IDC_DEFAULT_LONG_BREAK,
                settings.default_break_is_long ? IDC_DEFAULT_LONG_BREAK : IDC_DEFAULT_SHORT_BREAK);

            center_window_on_work_area(hwndDlg);
            return TRUE;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_OK: {
                    // Save settings from dialog
                    char buf[16];
                    int long_pomodoro, short_pomodoro, short_break, long_break, custom_timer;
                    GetDlgItemTextA(hwndDlg, IDC_POMODORO, buf, sizeof(buf));
                    long_pomodoro = atoi(buf);
                    GetDlgItemTextA(hwndDlg, IDC_SHORT_POMODORO, buf, sizeof(buf));
                    short_pomodoro = atoi(buf);
                    GetDlgItemTextA(hwndDlg, IDC_SHORT_BREAK, buf, sizeof(buf));
                    short_break = atoi(buf);
                    GetDlgItemTextA(hwndDlg, IDC_LONG_BREAK, buf, sizeof(buf));
                    long_break = atoi(buf);
                    GetDlgItemTextA(hwndDlg, IDC_CUSTOM_TIMER, buf, sizeof(buf));
                    custom_timer = atoi(buf);
                    
                    // Validate values
                    if (long_pomodoro > 0 && long_pomodoro <= 720 &&
                        short_pomodoro > 0 && short_pomodoro <= 720 &&
                        short_break > 0 && short_break <= 60 && 
                        long_break > 0 && long_break <= 120 &&
                        custom_timer > 0 && custom_timer <= 720) {
                        
                        settings.long_pomodoro_duration = long_pomodoro;
                        settings.short_pomodoro_duration = short_pomodoro;
                        settings.short_break_duration = short_break;
                        settings.long_break_duration = long_break;
                        settings.custom_duration = custom_timer;
                        settings.enable_clock_sound = SendDlgItemMessageA(hwndDlg, IDC_CLOCK_SOUND, BM_GETCHECK, 0, 0) == BST_CHECKED;
                        settings.default_pomodoro_is_long = IsDlgButtonChecked(hwndDlg, IDC_DEFAULT_LONG_POMODORO) == BST_CHECKED;
                        settings.default_break_is_long = IsDlgButtonChecked(hwndDlg, IDC_DEFAULT_LONG_BREAK) == BST_CHECKED;
                        if (!is_running && !is_paused) {
                            if (idle_mode == IDLE_POMODORO) {
                                idle_pomodoro_is_long = settings.default_pomodoro_is_long;
                            } else if (idle_mode == IDLE_BREAK) {
                                idle_break_is_long = settings.default_break_is_long;
                            }
                        }
                        {
                            int requestedAutostart = SendDlgItemMessageA(hwndDlg, IDC_AUTOSTART, BM_GETCHECK, 0, 0) == BST_CHECKED;
                            if (set_autostart(requestedAutostart)) {
                                autostart_enabled = requestedAutostart;
                            } else {
                                MessageBoxW(hwndDlg, L"开机启动设置失败。", L"错误", MB_OK | MB_ICONERROR);
                            }
                        }
                        save_settings();
                        EndDialog(hwndDlg, IDOK);
                    } else {
                        MessageBoxW(hwndDlg, g_lang->error_invalid_time, L"错误", MB_ICONERROR);
                    }
                    return TRUE;
                }
                case IDC_CANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    return TRUE;
            }
            break;
        case WM_CLOSE:
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
    }
    return FALSE;
}

// Show about dialog
void ShowAboutDialog(HWND hwndParent) {
    INT_PTR result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hwndParent, AboutDlgProc);
    if (result == -1) {
        MessageBoxW(hwndParent, L"关于窗口加载失败，请检查程序资源是否正确打包。", L"错误", MB_OK | MB_ICONERROR);
    }
}

// About dialog procedure
INT_PTR CALLBACK AboutDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HFONT hLinkFont = NULL;
    static HBRUSH hBrush = NULL;

    switch (uMsg) {
        case WM_INITDIALOG: {
            SetWindowTextW(hwndDlg, L"关于番茄钟");
            SetDlgItemTextW(hwndDlg, 210, L"番茄钟计时器 v2.5.7");
            SetDlgItemTextW(hwndDlg, 211, L"一个简洁的效率工具");
            SetDlgItemTextW(hwndDlg, 212, L"作者: Ferenc Lutischan");
            SetDlgItemTextW(hwndDlg, IDC_WEBSITE, L"访问项目主页");
            SetDlgItemTextW(hwndDlg, IDC_COFFEE, L"支持作者（请喝咖啡）");

            // Create underlined font for link
            HFONT hFont = (HFONT)SendDlgItemMessageW(hwndDlg, IDC_WEBSITE, WM_GETFONT, 0, 0);
            LOGFONTW lf;
            GetObjectW(hFont, sizeof(LOGFONTW), &lf);
            lf.lfUnderline = TRUE;
            hLinkFont = CreateFontIndirectW(&lf);
            SendDlgItemMessageW(hwndDlg, IDC_WEBSITE, WM_SETFONT, (WPARAM)hLinkFont, TRUE);
            SendDlgItemMessageW(hwndDlg, IDC_COFFEE, WM_SETFONT, (WPARAM)hLinkFont, TRUE);

            // Create brush for WM_CTLCOLORSTATIC
            hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

            center_window_on_work_area(hwndDlg);
            return TRUE;
        }
        case WM_CTLCOLORSTATIC: {
            HWND hwndStatic = (HWND)lParam;
            if (GetDlgCtrlID(hwndStatic) == IDC_WEBSITE || GetDlgCtrlID(hwndStatic) == IDC_COFFEE) {
                HDC hdc = (HDC)wParam;
                SetTextColor(hdc, RGB(0, 0, 255));
                SetBkMode(hdc, TRANSPARENT);
                return (LRESULT)hBrush;
            }
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_WEBSITE && HIWORD(wParam) == STN_CLICKED) {
                ShellExecuteW(NULL, L"open", L"https://github.com/lutischan-ferenc/pomodoro-timer-v2", NULL, NULL, SW_SHOWNORMAL);
                return TRUE;
            }
            if (LOWORD(wParam) == IDC_COFFEE && HIWORD(wParam) == STN_CLICKED) {
                ShellExecuteW(NULL, L"open", L"https://coff.ee/lutischanf", NULL, NULL, SW_SHOWNORMAL);
                return TRUE;
            }
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                // Clean up resources and close dialog
                if (hLinkFont) {
                    DeleteObject(hLinkFont);
                    hLinkFont = NULL;
                }
                if (hBrush) {
                    DeleteObject(hBrush);
                    hBrush = NULL;
                }
                EndDialog(hwndDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
        case WM_DESTROY:
        case WM_CLOSE:
            // Clean up resources
            if (hLinkFont) {
                DeleteObject(hLinkFont);
                hLinkFont = NULL;
            }
            if (hBrush) {
                DeleteObject(hBrush);
                hBrush = NULL;
            }
            if (uMsg == WM_CLOSE)
                EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
    }
    return FALSE;
}

// Toast window procedure
LRESULT CALLBACK ToastWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Keep toast visible until explicit action (buttons) or collapse behavior.
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_TOAST_ACTION && HIWORD(wParam) == BN_CLICKED) {
                // Button clicked: start the appropriate timer on the main window
                if (is_pomodoro_mode((TimerMode)toast_completed_mode)) {
                    idle_mode = IDLE_BREAK;
                    idle_break_is_long = settings.default_break_is_long;
                    start_mode_from_menu(g_main_hwnd, get_default_break_mode());
                } else if (toast_completed_mode == TIMER_CUSTOM) {
                    idle_mode = IDLE_CUSTOM;
                    start_mode_from_menu(g_main_hwnd, TIMER_CUSTOM);
                } else {
                    idle_mode = IDLE_POMODORO;
                    idle_pomodoro_is_long = settings.default_pomodoro_is_long;
                    start_mode_from_menu(g_main_hwnd, get_default_pomodoro_mode());
                }
                 DestroyWindow(hwnd);
             } else if (LOWORD(wParam) == ID_TOAST_CLOSE && HIWORD(wParam) == BN_CLICKED) {
                 // Close button clicked
                 DestroyWindow(hwnd);
             } else if (LOWORD(wParam) == ID_TOAST_COLLAPSE && HIWORD(wParam) == BN_CLICKED) {
                 if (g_toast_collapsed) {
                     int expandedWidth = g_toast_expanded_rect.right - g_toast_expanded_rect.left;
                     int closeX;
                     g_toast_collapsed = 0;
                     SetWindowPos(hwnd, HWND_TOPMOST,
                         g_toast_expanded_rect.left,
                         g_toast_expanded_rect.top,
                         expandedWidth,
                         g_toast_expanded_rect.bottom - g_toast_expanded_rect.top,
                         SWP_SHOWWINDOW);
                     if (g_hToastButton) ShowWindow(g_hToastButton, SW_SHOW);
                     if (g_hToastCloseButton) ShowWindow(g_hToastCloseButton, SW_SHOW);
                     closeX = expandedWidth - 22 - 8;
                     if (g_hToastCloseButton) SetWindowPos(g_hToastCloseButton, NULL, closeX, 8, 22, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
                     if (g_hToastCollapseButton) SetWindowPos(g_hToastCollapseButton, NULL, closeX - 26, 8, 22, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
                     if (g_hToastCollapseButton) SetWindowTextW(g_hToastCollapseButton, L"_");
                 } else {
                     GetWindowRect(hwnd, &g_toast_expanded_rect);
                     g_toast_collapsed = 1;
                     if (g_hToastButton) ShowWindow(g_hToastButton, SW_HIDE);
                     if (g_hToastCloseButton) ShowWindow(g_hToastCloseButton, SW_HIDE);
                     SetWindowPos(hwnd, HWND_TOPMOST,
                         g_toast_expanded_rect.right - 40,
                         g_toast_expanded_rect.top,
                         40,
                         g_toast_expanded_rect.bottom - g_toast_expanded_rect.top,
                         SWP_SHOWWINDOW);
                     if (g_hToastCollapseButton) SetWindowPos(g_hToastCollapseButton, NULL, 9, 8, 22, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
                     if (g_hToastCollapseButton) SetWindowTextW(g_hToastCollapseButton, L">");
                 }
                 InvalidateRect(hwnd, NULL, TRUE);
             }
             return 0;
        case WM_TIMER:
            if (wParam == 1 && !g_toast_collapsed) {
                if (g_hToastCollapseButton) {
                    SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(ID_TOAST_COLLAPSE, BN_CLICKED), (LPARAM)g_hToastCollapseButton);
                }
                KillTimer(hwnd, 1);
            }
            return 0;
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            g_hToastWnd = NULL;
            g_hToastButton = NULL;
            g_hToastCloseButton = NULL;
            g_hToastCollapseButton = NULL;
            g_toast_collapsed = 0;
            return 0;
        case WM_LBUTTONDOWN:
            {
                POINT pt;
                HWND child;
                pt.x = (short)LOWORD(lParam);
                pt.y = (short)HIWORD(lParam);
                child = ChildWindowFromPointEx(hwnd, pt, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);
                if (!child || child == hwnd) {
                    // Drag toast when clicking background in either expanded or collapsed state.
                    ReleaseCapture();
                    SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                }
            }
            return 0;
        case WM_MOVE:
            {
                RECT rc;
                int expandedWidth;
                int expandedHeight;
                GetWindowRect(hwnd, &rc);
                if (!g_toast_collapsed) {
                    g_toast_expanded_rect = rc;
                } else {
                    expandedWidth = g_toast_expanded_rect.right - g_toast_expanded_rect.left;
                    expandedHeight = g_toast_expanded_rect.bottom - g_toast_expanded_rect.top;
                    if (expandedWidth <= 0) expandedWidth = 300;
                    if (expandedHeight <= 0) expandedHeight = 150;
                    g_toast_expanded_rect.left = rc.left + 40 - expandedWidth;
                    g_toast_expanded_rect.top = rc.top;
                    g_toast_expanded_rect.right = g_toast_expanded_rect.left + expandedWidth;
                    g_toast_expanded_rect.bottom = rc.top + expandedHeight;
                }
            }
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Draw reddish background (use same red as the tray icon)
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH hBrush = CreateSolidBrush(RGB(139, 0, 0));
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);

            // Draw subtle border (darker)
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(100, 0, 0));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, 0, 0, rect.right, rect.bottom);
            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);
            DeleteObject(hPen);

            if (g_toast_collapsed) {
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutW(hdc, 8, (rect.bottom - 24) / 2, L"提醒", 2);
            } else {
                // Draw progress dots (4) above the action button
                int totalDots = 4;
                int dotR = 6; // radius
                int spacing = 12;
                int dotDiameter = dotR * 2;
                int totalWidth = totalDots * dotDiameter + (totalDots - 1) * spacing;
                int startX = (rect.right - totalWidth) / 2;
                int dotsY = rect.bottom - 40 - 15 - 16; // above button area (btnH=40, gap=15, dots_radius_space=16)

                // Determine how many dots should be green: show up to 4 completed pomodoros
                int greenCount = get_visible_dots(pomodoro_count);

                for (int i = 0; i < totalDots; i++) {
                    int cx = startX + i * (dotDiameter + spacing);
                    int left = cx;
                    int top = dotsY - dotR;
                    int right = cx + dotDiameter;
                    int bottom = dotsY + dotR;

                    HBRUSH fill = CreateSolidBrush((i < greenCount) ? RGB(144, 238, 144) : RGB(0, 0, 0));
                    HBRUSH old = (HBRUSH)SelectObject(hdc, fill);
                    Ellipse(hdc, left, top, right, bottom);
                    SelectObject(hdc, old);
                    DeleteObject(fill);
                }

                // Draw message text in white
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(255, 255, 255));
                HFONT hFont = CreateFontW(20, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
                HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

                RECT textRect = {15, 15, rect.right - 15, rect.bottom - 80};
                const wchar_t* message = (const wchar_t*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
                if (message) {
                    DrawTextW(hdc, message, -1, &textRect, DT_LEFT | DT_WORDBREAK);
                }

                SelectObject(hdc, hOldFont);
                DeleteObject(hFont);
            }

            EndPaint(hwnd, &ps);
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

// Register toast window class
void RegisterToastWindowClass(void) {
    static BOOL registered = FALSE;
    if (registered) return;

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = ToastWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = TOAST_WINDOW_CLASS;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    registered = TRUE;
}

// Show completion notification as toast
void ShowCompletionNotification(HWND hwnd, int completed_mode) {
    const wchar_t* message;
    (void)hwnd;

    if (is_pomodoro_mode((TimerMode)completed_mode)) {
        message = g_lang->notify_pomodoro_complete;
    } else if (completed_mode == TIMER_LONG_BREAK) {
        message = g_lang->notify_long_break_complete;
    } else if (completed_mode == TIMER_CUSTOM) {
        message = L"自定义计时完成!";
    } else {
        message = g_lang->notify_break_complete;
    }

    // Close previous toast if exists
    if (g_hToastWnd) {
        DestroyWindow(g_hToastWnd);
        g_hToastWnd = NULL;
    }

    // Register toast window class if not already done
    RegisterToastWindowClass();

    // Get taskbar position
    RECT taskbarRect = {0};
    HWND taskbarWnd = FindWindowW(L"Shell_TrayWnd", NULL);
    if (taskbarWnd) {
        GetWindowRect(taskbarWnd, &taskbarRect);
    }

    // Calculate toast position (above taskbar, right side)
    int toastWidth = 300; // smaller width
    int toastHeight = 150; // smaller height to match button
    int toastScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int toastScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    int xPos = toastScreenWidth - toastWidth;  // stick to primary screen right edge
    int yPos = taskbarRect.top - toastHeight;  // flush to taskbar top

    // Fallback if taskbar position not found
    if (yPos < 0) {
        yPos = toastScreenHeight - toastHeight;
    }

    // Keep toast within visible screen on small displays / unusual taskbar layouts.
    if (xPos < 0) xPos = 0;
    if (yPos < 0) yPos = 0;
    if (xPos + toastWidth > toastScreenWidth) xPos = toastScreenWidth - toastWidth;
    if (yPos + toastHeight > toastScreenHeight) yPos = toastScreenHeight - toastHeight;
    if (xPos < 0) xPos = 0;
    if (yPos < 0) yPos = 0;

    // Create toast window with message in window title (for debugging)
    // Store message pointer for use in WM_PAINT
    static wchar_t toastMessage[256] = {0};
    swprintf(toastMessage, 256, L"%ls\n今日累计: %d", message, pomodoro_count);

    g_hToastWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
        TOAST_WINDOW_CLASS,
        L"番茄钟",
        WS_POPUP,
        xPos, yPos, toastWidth, toastHeight,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (g_hToastWnd) {
        g_toast_collapsed = 0;
        // store toast state
        toast_is_pomodoro = is_pomodoro_mode((TimerMode)completed_mode);
        toast_is_long_break = (completed_mode == TIMER_LONG_BREAK);
        toast_completed_mode = completed_mode;

        // Store message in window user data for access in WM_PAINT
        SetWindowLongPtrW(g_hToastWnd, GWLP_USERDATA, (LONG_PTR)toastMessage);

        // Create action button (centered at bottom)
        int btnW = 200, btnH = 40;
        int btnX = (toastWidth - btnW) / 2;
        int btnY = toastHeight - btnH - 15;
        const wchar_t* btnText;
        if (is_pomodoro_mode((TimerMode)completed_mode)) {
            btnText = settings.default_break_is_long ? L"开始长休息" : L"开始短休息";
        } else if (completed_mode == TIMER_CUSTOM) {
            btnText = L"继续自定义"
            ;
        } else {
            btnText = settings.default_pomodoro_is_long ? L"开始长番茄钟" : L"开始短番茄钟";
        }
        g_hToastButton = CreateWindowW(L"BUTTON", btnText,
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            btnX, btnY, btnW, btnH,
            g_hToastWnd, (HMENU)ID_TOAST_ACTION, GetModuleHandle(NULL), NULL);

        // Set button font
        static HFONT hBtnFont = NULL;
        if (!hBtnFont) {
            hBtnFont = CreateFontW(17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                     DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        }
        SendMessage(g_hToastButton, WM_SETFONT, (WPARAM)hBtnFont, TRUE);

        // Explicitly set button text (force correct label)
        SetWindowTextW(g_hToastButton, btnText);

        // Force repaint to update dots and button
        InvalidateRect(g_hToastWnd, NULL, TRUE);
        UpdateWindow(g_hToastWnd);

        // Create small close button in top-right corner
        int closeW = 22, closeH = 22;
        int closeX = toastWidth - closeW - 8;
        int closeY = 8;
        g_hToastCloseButton = CreateWindowW(L"BUTTON", L"✕",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER,
            closeX, closeY, closeW, closeH,
            g_hToastWnd, (HMENU)ID_TOAST_CLOSE, GetModuleHandle(NULL), NULL);
        SendMessageW(g_hToastCloseButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

        g_hToastCollapseButton = CreateWindowW(L"BUTTON", L"_",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER,
            closeX - 26, closeY, 22, 22,
            g_hToastWnd, (HMENU)ID_TOAST_COLLAPSE, GetModuleHandle(NULL), NULL);
        SendMessageW(g_hToastCollapseButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

        // Show window and bring to foreground
        ShowWindow(g_hToastWnd, SW_SHOW);
        UpdateWindow(g_hToastWnd);
        SetWindowPos(g_hToastWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        GetWindowRect(g_hToastWnd, &g_toast_expanded_rect);
        if (settings.toast_auto_collapse_seconds > 0) {
            SetTimer(g_hToastWnd, 1, (UINT)settings.toast_auto_collapse_seconds * 1000, NULL);
        }

        // Try to bring to foreground
        AllowSetForegroundWindow(GetCurrentProcessId());
        SetForegroundWindow(g_hToastWnd);
    } else {
        OutputDebugStringW(L"ShowCompletionNotification: failed to create toast window\n");
    }
}

static int find_day_count(DayCount* days, int dayCount, const char* date) {
    int i;
    for (i = 0; i < dayCount; ++i) {
        if (strcmp(days[i].date, date) == 0) return i;
    }
    return -1;
}

static void add_day_count(DayCount* days, int* dayCount, const char* date, int delta) {
    int idx;
    if (!days || !dayCount || !date || delta == 0) return;

    idx = find_day_count(days, *dayCount, date);
    if (idx >= 0) {
        days[idx].count += delta;
        return;
    }

    if (*dayCount < 4096) {
        strncpy(days[*dayCount].date, date, 10);
        days[*dayCount].date[10] = '\0';
        days[*dayCount].count = delta;
        (*dayCount)++;
    }
}

static int sync_today_count_to_target(int targetCount) {
    time_t now = time(NULL);
    struct tm* tmNow = localtime(&now);
    FILE* fp;
    int currentCount;
    int delta;
    char today[11];

    if (!tmNow) return 0;

    load_heatmap_data();
    sprintf(today, "%04d-%02d-%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday);
    currentCount = get_day_count_value(tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday);
    delta = targetCount - currentCount;
    if (delta == 0) return 1;

    fp = _wfopen(g_adjustments_path, L"a");
    if (!fp) return 0;
    fprintf(fp, "%s,%d\n", today, delta);
    fclose(fp);
    return 1;
}

static int get_day_count_value(int year, int month, int day) {
    char key[11];
    int idx;
    sprintf(key, "%04d-%02d-%02d", year, month, day);
    idx = find_day_count(g_day_counts, g_day_count, key);
    return idx >= 0 ? g_day_counts[idx].count : 0;
}

static int get_days_in_month(int year, int month) {
    static const int monthDays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int days = monthDays[month - 1];
    if (month == 2) {
        int leap = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
        if (leap) days = 29;
    }
    return days;
}

static int get_weekday_monday0(int year, int month, int day) {
    struct tm t = {0};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    mktime(&t);
    return (t.tm_wday + 6) % 7;
}

static int get_month_total(int year, int month) {
    int day;
    int total = 0;
    int days = get_days_in_month(year, month);
    for (day = 1; day <= days; ++day) {
        total += get_day_count_value(year, month, day);
    }
    return total;
}

static int get_year_total(int year) {
    int i;
    int total = 0;
    for (i = 0; i < g_day_count; ++i) {
        int y = 0, m = 0, d = 0;
        if (sscanf(g_day_counts[i].date, "%d-%d-%d", &y, &m, &d) == 3 && y == year) {
            total += g_day_counts[i].count;
        }
    }
    return total;
}

static int parse_pomodoro_log_entry(const char* line, char date[11], int* completedCount) {
    int count = 1;

    if (!line || !date || !completedCount) return 0;
    date[0] = '\0';
    if (sscanf(line, "%10[^,],%*[^,],%d", date, &count) == 2) {
        *completedCount = clamp_int(count, 1, 9999);
        return 1;
    }
    if (sscanf(line, "%10[^,]", date) == 1) {
        *completedCount = 1;
        return 1;
    }
    return 0;
}

int record_completed_pomodoros(int completedCount) {
    time_t now = time(NULL);
    struct tm* tmNow = localtime(&now);
    completedCount = clamp_int(completedCount, 1, 9999);
    maybe_archive_log_monthly();
    FILE* fp = _wfopen(g_log_path, L"a");
    if (!fp || !tmNow) {
        if (fp) fclose(fp);
        return 0;
    }
    fprintf(fp, "%04d-%02d-%02d,%02d:%02d:%02d,%d\n",
        tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday,
        tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec, completedCount);
    fclose(fp);

    if (g_hHeatmapWnd) InvalidateRect(g_hHeatmapWnd, NULL, TRUE);
    return 1;
}

static int get_today_count_from_storage(void) {
    time_t now = time(NULL);
    struct tm* tmNow = localtime(&now);
    char today[11];
    char line[128];
    int count = 0;
    FILE* in;
    FILE* adj;

    if (!tmNow) return clamp_int(pomodoro_count, 0, 9999);

    sprintf(today, "%04d-%02d-%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday);

    in = _wfopen(g_log_path, L"r");
    if (in) {
        while (fgets(line, sizeof(line), in)) {
            char date[11] = {0};
            int completedCount = 1;
            if (!parse_pomodoro_log_entry(line, date, &completedCount)) continue;
            if (strcmp(date, today) == 0) count += completedCount;
        }
        fclose(in);
    }

    adj = _wfopen(g_adjustments_path, L"r");
    if (adj) {
        while (fgets(line, sizeof(line), adj)) {
            char date[11] = {0};
            int delta = 0;
            if (sscanf(line, "%10[^,],%d", date, &delta) != 2) continue;
            if (strcmp(date, today) == 0) count += delta;
        }
        fclose(adj);
    }

    return clamp_int(count, 0, 9999);
}

static void refresh_today_count_if_day_changed(HWND hwnd, int forceRefresh) {
    time_t now = time(NULL);
    struct tm* tmNow = localtime(&now);
    char today[11];
    int newCount;

    if (!tmNow) return;
    sprintf(today, "%04d-%02d-%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday);

    if (!forceRefresh && strcmp(g_last_count_sync_date, today) == 0) return;

    strncpy(g_last_count_sync_date, today, 10);
    g_last_count_sync_date[10] = '\0';

    newCount = get_today_count_from_storage();
    if (newCount != pomodoro_count) {
        pomodoro_count = newCount;
        save_settings();
        if (g_hHeatmapWnd) InvalidateRect(g_hHeatmapWnd, NULL, TRUE);
        if (hwnd && nid.hWnd) {
            refresh_timer_icon_by_state(hwnd);
        }
    }
}

static void load_heatmap_data(void) {
    FILE* in = _wfopen(g_log_path, L"r");
    FILE* adj = _wfopen(g_adjustments_path, L"r");
    time_t now = time(NULL);
    struct tm* tmNow = localtime(&now);
    char line[128];
    int i;

    g_day_count = 0;

    g_heatmap_weekTotal = 0;
    g_heatmap_monthTotal = 0;
    g_heatmap_yearTotal = 0;
    g_heatmap_total = 0;

    if (!tmNow) {
        return;
    }

    if (g_heatmap_display_year == 0 || g_heatmap_display_month == 0) {
        g_heatmap_display_year = tmNow->tm_year + 1900;
        g_heatmap_display_month = tmNow->tm_mon + 1;
    }

    if (in) {
        while (fgets(line, sizeof(line), in)) {
            char date[11] = {0};
            int completedCount = 1;

            if (!parse_pomodoro_log_entry(line, date, &completedCount)) continue;
            add_day_count(g_day_counts, &g_day_count, date, completedCount);
        }
        fclose(in);
    }

    load_archive_logs();

    if (adj) {
        while (fgets(line, sizeof(line), adj)) {
            char date[11] = {0};
            int delta = 0;
            if (sscanf(line, "%10[^,],%d", date, &delta) != 2) continue;
            add_day_count(g_day_counts, &g_day_count, date, delta);
        }
        fclose(adj);
    }

    for (i = 0; i < g_day_count; ++i) {
        int y = 0, m = 0, d = 0;
        int c;
        struct tm tmEntry = {0};
        time_t entryTime;
        double deltaDays;

        if (sscanf(g_day_counts[i].date, "%d-%d-%d", &y, &m, &d) != 3) continue;
        c = g_day_counts[i].count;
        if (c < 0) c = 0;
        g_day_counts[i].count = c;

        tmEntry.tm_year = y - 1900;
        tmEntry.tm_mon = m - 1;
        tmEntry.tm_mday = d;
        tmEntry.tm_hour = 12;
        entryTime = mktime(&tmEntry);
        deltaDays = difftime(now, entryTime) / (60.0 * 60.0 * 24.0);

        g_heatmap_total += c;
        if (y == (tmNow->tm_year + 1900)) g_heatmap_yearTotal += c;
        if (y == (tmNow->tm_year + 1900) && m == (tmNow->tm_mon + 1)) g_heatmap_monthTotal += c;
        if (deltaDays >= 0.0 && deltaDays < 7.0) g_heatmap_weekTotal += c;
    }

    g_heatmap_total = clamp_int(g_heatmap_total, 0, 999999);
    g_heatmap_yearTotal = clamp_int(g_heatmap_yearTotal, 0, 999999);
    g_heatmap_monthTotal = clamp_int(g_heatmap_monthTotal, 0, 999999);
    g_heatmap_weekTotal = clamp_int(g_heatmap_weekTotal, 0, 999999);
}

LRESULT CALLBACK HeatmapWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            CreateWindowW(L"BUTTON", L"<", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                18, 10, 30, 24, hwnd, (HMENU)ID_HEATMAP_PREV, GetModuleHandle(NULL), NULL);
            CreateWindowW(L"BUTTON", L"本月", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                160, 10, 60, 24, hwnd, (HMENU)ID_HEATMAP_TODAY, GetModuleHandle(NULL), NULL);
            CreateWindowW(L"BUTTON", L">", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                336, 10, 30, 24, hwnd, (HMENU)ID_HEATMAP_NEXT, GetModuleHandle(NULL), NULL);
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_HEATMAP_PREV) {
                g_heatmap_display_month--;
                if (g_heatmap_display_month < 1) {
                    g_heatmap_display_month = 12;
                    g_heatmap_display_year--;
                }
                g_heatmap_selected_day = 0;
                wcscpy(g_heatmap_info_text, L"点击日历日期可查看当日完成数。");
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }
            if (LOWORD(wParam) == ID_HEATMAP_NEXT) {
                g_heatmap_display_month++;
                if (g_heatmap_display_month > 12) {
                    g_heatmap_display_month = 1;
                    g_heatmap_display_year++;
                }
                g_heatmap_selected_day = 0;
                wcscpy(g_heatmap_info_text, L"点击日历日期可查看当日完成数。");
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }
            if (LOWORD(wParam) == ID_HEATMAP_TODAY) {
                time_t now = time(NULL);
                struct tm* tmNow = localtime(&now);
                if (tmNow) {
                    g_heatmap_display_year = tmNow->tm_year + 1900;
                    g_heatmap_display_month = tmNow->tm_mon + 1;
                }
                g_heatmap_selected_day = 0;
                wcscpy(g_heatmap_info_text, L"已切换到当前月份。");
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }
            break;
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            POINT pt;
            int day;
            pt.x = x;
            pt.y = y;

            for (day = 1; day <= 31; ++day) {
                if (g_heatmap_day_rect_valid[day] && PtInRect(&g_heatmap_day_rects[day], pt)) {
                    int c = get_day_count_value(g_heatmap_display_year, g_heatmap_display_month, day);
                    g_heatmap_selected_day = day;
                    swprintf(g_heatmap_info_text, 160, L"%04d-%02d-%02d 完成番茄钟: %d", g_heatmap_display_year, g_heatmap_display_month, day, c);
                    InvalidateRect(hwnd, NULL, TRUE);
                    return 0;
                }
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            int i;
            int day;
            int firstWeekday;
            int days;
            int monthTotal;
            int selectedYearTotal;
            time_t now = time(NULL);
            struct tm* tmNow = localtime(&now);
            const wchar_t* weekNames[7] = {L"一", L"二", L"三", L"四", L"五", L"六", L"日"};
            int originX = 20;
            int originY = 126;
            int cellW = 48;
            int cellH = 34;

            // Keep the calendar live even if sessions complete while window remains open.
            load_heatmap_data();

            HFONT hTitle = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                DEFAULT_PITCH, L"Microsoft YaHei UI");
            HFONT hBody = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                DEFAULT_PITCH, L"Microsoft YaHei UI");
            HFONT hOld = (HFONT)SelectObject(hdc, hBody);

            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
            SetBkMode(hdc, TRANSPARENT);

            SelectObject(hdc, hTitle);
            TextOutW(hdc, 110, 42, L"番茄钟月历热力图", 8);

            SelectObject(hdc, hBody);
            {
                wchar_t summary[256];
                monthTotal = get_month_total(g_heatmap_display_year, g_heatmap_display_month);
                selectedYearTotal = get_year_total(g_heatmap_display_year);
                swprintf(summary, 256, L"%04d年%02d月   当月: %d   该年: %d   总计: %d",
                    g_heatmap_display_year, g_heatmap_display_month, monthTotal, selectedYearTotal, g_heatmap_total);
                TextOutW(hdc, 20, 76, summary, lstrlenW(summary));
            }

            for (i = 0; i < 7; ++i) {
                TextOutW(hdc, originX + i * cellW + 18, originY - 24, weekNames[i], 1);
            }

            firstWeekday = get_weekday_monday0(g_heatmap_display_year, g_heatmap_display_month, 1);
            days = get_days_in_month(g_heatmap_display_year, g_heatmap_display_month);

            if (g_heatmap_selected_day > days) {
                g_heatmap_selected_day = 0;
            }

            for (i = 0; i < 32; ++i) {
                g_heatmap_day_rect_valid[i] = 0;
            }

            for (day = 1; day <= days; ++day) {
                int slot = firstWeekday + day - 1;
                int row = slot / 7;
                int col = slot % 7;
                int x = originX + col * cellW;
                int y = originY + row * cellH;
                int c = get_day_count_value(g_heatmap_display_year, g_heatmap_display_month, day);
                COLORREF color = RGB(235, 237, 240);
                HBRUSH brush;
                RECT r = {x, y, x + cellW - 4, y + cellH - 4};
                wchar_t dayText[16];
                BOOL isToday = FALSE;

                if (c >= 13) color = RGB(25, 97, 39);
                else if (c >= 9) color = RGB(35, 154, 59);
                else if (c >= 5) color = RGB(123, 201, 111);
                else if (c >= 1) color = RGB(198, 228, 139);

                if (tmNow && g_heatmap_display_year == (tmNow->tm_year + 1900)
                    && g_heatmap_display_month == (tmNow->tm_mon + 1)
                    && day == tmNow->tm_mday) {
                    isToday = TRUE;
                }

                brush = CreateSolidBrush(color);
                FillRect(hdc, &r, brush);
                DeleteObject(brush);
                FrameRect(hdc, &r, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
                g_heatmap_day_rects[day] = r;
                g_heatmap_day_rect_valid[day] = 1;

                if (isToday || day == g_heatmap_selected_day) {
                    HPEN pen = CreatePen(PS_SOLID, 2, day == g_heatmap_selected_day ? RGB(220, 20, 60) : RGB(30, 144, 255));
                    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
                    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    Rectangle(hdc, r.left, r.top, r.right, r.bottom);
                    SelectObject(hdc, oldBrush);
                    SelectObject(hdc, oldPen);
                    DeleteObject(pen);
                }

                swprintf(dayText, 16, L"%d", day);
                TextOutW(hdc, x + 4, y + 2, dayText, lstrlenW(dayText));
                if (c > 0) {
                    swprintf(dayText, 16, L"%d", c);
                    TextOutW(hdc, x + cellW - 20, y + cellH - 18, dayText, lstrlenW(dayText));
                }
            }

            TextOutW(hdc, 20, 350, L"颜色越深，完成数量越多（16次及以上按最深色）。", 24);
            TextOutW(hdc, 20, 372, g_heatmap_info_text, lstrlenW(g_heatmap_info_text));

            SelectObject(hdc, hOld);
            DeleteObject(hTitle);
            DeleteObject(hBody);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            g_hHeatmapWnd = NULL;
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void RegisterHeatmapWindowClass(void) {
    static BOOL registered = FALSE;
    if (registered) return;

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = HeatmapWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = HEATMAP_WINDOW_CLASS;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);
    registered = TRUE;
}

void ShowHeatmapWindow(HWND hwnd) {
    (void)hwnd;
    RECT workArea = {0};
    int heatmapWidth = 390;
    int heatmapHeight = 430;
    int xPos;
    int yPos;

    load_heatmap_data();
    RegisterHeatmapWindowClass();

    if (g_hHeatmapWnd) {
        ShowWindow(g_hHeatmapWnd, SW_RESTORE);
        SetForegroundWindow(g_hHeatmapWnd);
        InvalidateRect(g_hHeatmapWnd, NULL, TRUE);
        return;
    }

    if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0)) {
        workArea.left = 0;
        workArea.top = 0;
        workArea.right = GetSystemMetrics(SM_CXSCREEN);
        workArea.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    xPos = workArea.left + ((workArea.right - workArea.left) - heatmapWidth) / 2;
    yPos = workArea.top + ((workArea.bottom - workArea.top) - heatmapHeight) / 2;
    if (xPos < workArea.left) xPos = workArea.left;
    if (yPos < workArea.top) yPos = workArea.top;

    g_hHeatmapWnd = CreateWindowW(
        HEATMAP_WINDOW_CLASS,
        L"番茄钟热力图",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        xPos, yPos, heatmapWidth, heatmapHeight,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (!g_hHeatmapWnd) {
        MessageBoxW(NULL, L"热力图窗口创建失败。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    ShowWindow(g_hHeatmapWnd, SW_SHOW);
    UpdateWindow(g_hHeatmapWnd);
}

void generate_and_open_report(HWND hwnd) {
    ShowHeatmapWindow(hwnd);
}

// Main window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_taskbar_created_message && msg == g_taskbar_created_message) {
        Shell_NotifyIconW(NIM_ADD, &nid);
        refresh_timer_icon_by_state(hwnd);
        return 0;
    }

    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, ID_MAIN_DAY_SYNC_TIMER, 30000, NULL);
            return 0;
        case WM_USER + 1: // Tray icon message
            if (LOWORD(lParam) == WM_LBUTTONUP) {
                // Left click: resume if paused, stop if running, otherwise start current idle mode.
                if (is_paused) {
                    if ((remaining_seconds > 0 || is_overtime) && (current_timer_mode != TIMER_NONE || is_overtime)) {
                        is_paused = 0;
                        is_running = 1;
                        launch_timer_thread(hwnd);
                    } else {
                        // Broken pause state fallback: clear pause and return to idle safely.
                        is_paused = 0;
                        is_running = 0;
                        current_timer_mode = TIMER_NONE;
                        remaining_seconds = 0;
                        clear_count_up_state();
                        clear_overtime_state();
                    }
                    refresh_timer_icon_by_state(hwnd);
                } else if (is_running) {
                    stop_timer_thread_if_needed();
                    is_paused = 0;
                    set_idle_mode_after_manual_stop();
                    current_timer_mode = TIMER_NONE;
                    remaining_seconds = 0;
                    clear_count_up_state();
                    clear_overtime_state();
                    save_settings();
                    refresh_timer_icon_by_state(hwnd);
                } else {
                    start_current_idle_mode(hwnd);
                }
            } else if (LOWORD(lParam) == WM_RBUTTONUP) {
                // Right click: show context menu
                HMENU hMenu = CreatePopupMenu();
                HMENU hStartMenu = CreatePopupMenu();
                HMENU hControlMenu = CreatePopupMenu();
                HMENU hAdjustMenu = CreatePopupMenu();
                HMENU hDurationMenu = CreatePopupMenu();
                HMENU hOptionMenu = CreatePopupMenu();
                HMENU hDataOpsMenu = CreatePopupMenu();
                HMENU hDataStoreMenu = CreatePopupMenu();
                HMENU hIdleMenu = CreatePopupMenu();
                UINT idleAvailability = (is_running || is_paused) ? MF_GRAYED : MF_ENABLED;
                g_hMenu = hMenu; // Store menu handle for language updates

                AppendMenu(hControlMenu, MF_STRING | ((is_running || is_paused) ? MF_GRAYED : MF_ENABLED), ID_MENU_START_CURRENT, L"开始");
                AppendMenu(hControlMenu, MF_STRING | ((is_running || is_paused) ? MF_ENABLED : MF_GRAYED), ID_MENU_PAUSE_RESUME,
                    is_running ? L"暂停" : L"继续");
                AppendMenu(hControlMenu, MF_STRING | ((is_running || is_paused) ? MF_ENABLED : MF_GRAYED), ID_MENU_STOP, L"结束");

                AppendMenu(hStartMenu, MF_STRING, 1, L"开始长番茄钟");
                AppendMenu(hStartMenu, MF_STRING, ID_MENU_START_SHORT_POMODORO, L"开始短番茄钟");
                AppendMenu(hStartMenu, MF_STRING, 2, L"开始短休息");
                AppendMenu(hStartMenu, MF_STRING, 3, L"开始长休息");
                AppendMenu(hStartMenu, MF_STRING, ID_MENU_START_COUNT_UP, L"开始正计时");
                AppendMenu(hStartMenu, MF_STRING, ID_MENU_START_CUSTOM, L"开始自定义计时");

                AppendMenu(hAdjustMenu, MF_STRING, ID_MENU_SET_TIME,
                    ((is_running || is_paused) && is_overtime) ? L"修改已超时时间(分钟)" :
                    (((is_running || is_paused) && is_count_up_timer) ? L"修改已计时时间(分钟)" : L"修改当前剩余时间(分钟)"));
                AppendMenu(hAdjustMenu, MF_STRING | ((is_running || is_paused) ? MF_ENABLED : MF_GRAYED), ID_MENU_PLUS_5_MIN, L"增加步进时间");
                AppendMenu(hAdjustMenu, MF_STRING | ((is_running || is_paused) ? MF_ENABLED : MF_GRAYED), ID_MENU_MINUS_5_MIN, L"减少步进时间");
                AppendMenu(hAdjustMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hAdjustMenu, MF_STRING | ((is_running || is_paused) ? MF_GRAYED : MF_ENABLED), ID_MENU_SET_COUNT, L"修改今日番茄数");
                AppendMenu(hAdjustMenu, MF_STRING | ((is_running || is_paused) ? MF_GRAYED : MF_ENABLED), ID_MENU_RESET_COUNT, L"番茄计数清零");
                AppendMenu(hAdjustMenu, MF_STRING | ((is_running || is_paused) ? MF_GRAYED : MF_ENABLED), ID_MENU_SET_LONG_POMODORO_COUNT, L"长番茄钟番茄钟数");

                AppendMenu(hDurationMenu, MF_STRING, ID_MENU_SET_POMODORO_DURATION, L"长番茄钟时长(分钟)");
                AppendMenu(hDurationMenu, MF_STRING, ID_MENU_SET_SHORT_POMODORO_DURATION, L"短番茄钟时长(分钟)");
                AppendMenu(hDurationMenu, MF_STRING, ID_MENU_SET_SHORT_BREAK_DURATION, L"短休息时长(分钟)");
                AppendMenu(hDurationMenu, MF_STRING, ID_MENU_SET_LONG_BREAK_DURATION, L"长休息时长(分钟)");
                AppendMenu(hDurationMenu, MF_STRING, ID_MENU_SET_CUSTOM_DURATION, L"自定义时长(分钟)");
                AppendMenu(hDurationMenu, MF_STRING, ID_MENU_SET_TIME_BLOCK, L"步进时间(分钟)");
                AppendMenu(hDurationMenu, MF_STRING, ID_MENU_SET_TOAST_COLLAPSE_SECONDS, L"弹窗自动折叠(秒)");

                AppendMenu(hOptionMenu, MF_STRING | (settings.enable_clock_sound ? MF_CHECKED : 0), 4, L"时钟音效");
                AppendMenu(hOptionMenu, MF_STRING | (settings.enable_completion_sound ? MF_CHECKED : 0), ID_MENU_COMPLETION_SOUND, L"完成声音");
                AppendMenu(hOptionMenu, MF_STRING | (settings.show_completion_dialog ? MF_CHECKED : 0), 9, L"完成后弹窗");
                AppendMenu(hOptionMenu, MF_STRING | (settings.enable_overtime_count_up ? MF_CHECKED : 0), ID_MENU_ENABLE_OVERTIME, L"超时正计时");
                AppendMenu(hOptionMenu, MF_STRING | (autostart_enabled ? MF_CHECKED : 0), 5, L"开机启动");
                AppendMenu(hOptionMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hOptionMenu, MF_STRING | (settings.default_pomodoro_is_long ? MF_CHECKED : 0), ID_MENU_DEFAULT_LONG_POMODORO, L"默认：长番茄钟");
                AppendMenu(hOptionMenu, MF_STRING | (!settings.default_pomodoro_is_long ? MF_CHECKED : 0), ID_MENU_DEFAULT_SHORT_POMODORO, L"默认：短番茄钟");
                AppendMenu(hOptionMenu, MF_STRING | (!settings.default_break_is_long ? MF_CHECKED : 0), ID_MENU_DEFAULT_SHORT_BREAK, L"默认：短休息");
                AppendMenu(hOptionMenu, MF_STRING | (settings.default_break_is_long ? MF_CHECKED : 0), ID_MENU_DEFAULT_LONG_BREAK, L"默认：长休息");

                AppendMenu(hDataOpsMenu, MF_STRING, ID_MENU_IMPORT_DATA, L"导入数据");
                AppendMenu(hDataOpsMenu, MF_STRING, ID_MENU_EXPORT_DATA, L"导出数据");
                AppendMenu(hDataOpsMenu, MF_STRING, ID_MENU_RESET_DEFAULTS, L"恢复默认");

                AppendMenu(hDataStoreMenu, MF_STRING | (g_data_location_mode == DATA_LOC_DEFAULT ? MF_CHECKED : 0), ID_MENU_DATA_LOC_DEFAULT, L"便携模式");
                AppendMenu(hDataStoreMenu, MF_STRING | (g_data_location_mode == DATA_LOC_ONEDRIVE ? MF_CHECKED : 0), ID_MENU_DATA_LOC_ONEDRIVE, L"OneDrive");
                AppendMenu(hDataStoreMenu, MF_STRING | (g_data_location_mode == DATA_LOC_CUSTOM ? MF_CHECKED : 0), ID_MENU_DATA_LOC_CUSTOM, L"自定义位置");

                    AppendMenu(hIdleMenu, MF_STRING | idleAvailability | ((!is_running && !is_paused && idle_mode == IDLE_POMODORO && idle_pomodoro_is_long) ? MF_CHECKED : 0), ID_MENU_IDLE_POMODORO, L"长番茄钟");
                    AppendMenu(hIdleMenu, MF_STRING | idleAvailability | ((!is_running && !is_paused && idle_mode == IDLE_POMODORO && !idle_pomodoro_is_long) ? MF_CHECKED : 0), ID_MENU_IDLE_SHORT_POMODORO, L"短番茄钟");
                    AppendMenu(hIdleMenu, MF_STRING | idleAvailability | ((!is_running && !is_paused && idle_mode == IDLE_BREAK && !idle_break_is_long) ? MF_CHECKED : 0), ID_MENU_IDLE_SHORT_BREAK, L"短休息");
                    AppendMenu(hIdleMenu, MF_STRING | idleAvailability | ((!is_running && !is_paused && idle_mode == IDLE_BREAK && idle_break_is_long) ? MF_CHECKED : 0), ID_MENU_IDLE_LONG_BREAK, L"长休息");
                    AppendMenu(hIdleMenu, MF_STRING | idleAvailability | ((!is_running && !is_paused && idle_mode == IDLE_COUNT_UP) ? MF_CHECKED : 0), ID_MENU_IDLE_COUNT_UP, L"正计时");
                    AppendMenu(hIdleMenu, MF_STRING | idleAvailability | ((!is_running && !is_paused && idle_mode == IDLE_CUSTOM) ? MF_CHECKED : 0), ID_MENU_IDLE_CUSTOM, L"自定义计时");

                AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hControlMenu, L"会话");
                AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hStartMenu, L"切换");
                AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hIdleMenu, L"待机");
                AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hAdjustMenu, L"调整");
                AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hDurationMenu, L"时长");
                AppendMenu(hMenu, MF_POPUP | ((is_running || is_paused) ? MF_GRAYED : 0), (UINT_PTR)hDataOpsMenu, L"操作");
                AppendMenu(hMenu, MF_POPUP | ((is_running || is_paused) ? MF_GRAYED : 0), (UINT_PTR)hDataStoreMenu, L"存储");
                AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hOptionMenu, L"偏好");
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, ID_MENU_REPORT, L"统计");
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, 8, L"退出");

                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hwnd);
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);

                // Handle menu commands
                switch (cmd) {
                    case ID_MENU_START_CURRENT:
                        if (is_overtime) {
                            stop_timer_thread_if_needed();
                            clear_overtime_state();
                            clear_count_up_state();
                        }
                        start_current_idle_mode(hwnd);
                        break;
                    case ID_MENU_PAUSE_RESUME:
                        if (is_running) {
                            stop_timer_thread_if_needed();
                            is_paused = 1;
                            refresh_timer_icon_by_state(hwnd);
                        } else if (is_paused && (remaining_seconds > 0 || is_overtime) && (current_timer_mode != TIMER_NONE || is_overtime)) {
                            is_paused = 0;
                            is_running = 1;
                            launch_timer_thread(hwnd);
                            refresh_timer_icon_by_state(hwnd);
                        } else {
                            MessageBoxW(hwnd, L"当前没有可暂停或继续的计时。", L"提示", MB_OK | MB_ICONINFORMATION);
                        }
                        break;
                    case ID_MENU_STOP:
                        if (is_running || is_paused) {
                            stop_timer_thread_if_needed();
                            is_paused = 0;
                            set_idle_mode_after_manual_stop();
                            current_timer_mode = TIMER_NONE;
                            remaining_seconds = 0;
                            clear_count_up_state();
                            clear_overtime_state();
                            save_settings();
                            refresh_timer_icon_by_state(hwnd);
                        }
                        break;
                    case 1: // Start long Pomodoro
                        start_mode_from_menu(hwnd, TIMER_LONG_POMODORO);
                        break;
                    case ID_MENU_START_SHORT_POMODORO:
                        start_mode_from_menu(hwnd, TIMER_SHORT_POMODORO);
                        break;
                    case 2: // Start Break
                        start_mode_from_menu(hwnd, TIMER_SHORT_BREAK);
                        break;
                    case 3: // Start Long Break
                        start_mode_from_menu(hwnd, TIMER_LONG_BREAK);
                        break;
                    case ID_MENU_START_COUNT_UP:
                        start_mode_from_menu(hwnd, TIMER_COUNT_UP);
                        break;
                    case ID_MENU_START_CUSTOM:
                        start_mode_from_menu(hwnd, TIMER_CUSTOM);
                        break;
                    case 4: // Toggle Clock Sound
                        settings.enable_clock_sound = !settings.enable_clock_sound;
                        if (!settings.enable_clock_sound) {
                            stop_clock_loop_sound();
                        } else if (is_running) {
                            start_clock_loop_sound();
                        }
                        save_settings();
                        break;
                    case ID_MENU_COMPLETION_SOUND:
                        settings.enable_completion_sound = !settings.enable_completion_sound;
                        save_settings();
                        break;
                    case 5: // Toggle Autostart
                        if (set_autostart(!autostart_enabled)) {
                            autostart_enabled = !autostart_enabled;
                        } else {
                            MessageBoxW(hwnd, L"开机启动设置失败。", L"错误", MB_OK | MB_ICONERROR);
                        }
                        break;
                    case 9: // Toggle Completion Dialog
                        settings.show_completion_dialog = !settings.show_completion_dialog;
                        save_settings();
                        break;
                    case ID_MENU_ENABLE_OVERTIME:
                        settings.enable_overtime_count_up = !settings.enable_overtime_count_up;
                        save_settings();
                        break;
                    case ID_MENU_DEFAULT_LONG_POMODORO:
                    case ID_MENU_DEFAULT_SHORT_POMODORO:
                        settings.default_pomodoro_is_long = (cmd == ID_MENU_DEFAULT_LONG_POMODORO);
                        if (!is_running && !is_paused && idle_mode == IDLE_POMODORO) {
                            idle_pomodoro_is_long = settings.default_pomodoro_is_long;
                            refresh_timer_icon_by_state(hwnd);
                        }
                        save_settings();
                        break;
                    case ID_MENU_DEFAULT_LONG_BREAK:
                    case ID_MENU_DEFAULT_SHORT_BREAK:
                        settings.default_break_is_long = (cmd == ID_MENU_DEFAULT_LONG_BREAK);
                        if (!is_running && !is_paused && idle_mode == IDLE_BREAK) {
                            idle_break_is_long = settings.default_break_is_long;
                            refresh_timer_icon_by_state(hwnd);
                        }
                        save_settings();
                        break;
                    case ID_MENU_SET_POMODORO_DURATION: {
                        int minutes = settings.long_pomodoro_duration;
                        if (PromptForInteger(hwnd, L"长番茄钟时长", L"请输入长番茄钟时长(1-720分钟):", minutes, 1, 720, &minutes)) {
                            settings.long_pomodoro_duration = minutes;
                            save_settings();
                        }
                        break;
                    }
                    case ID_MENU_SET_SHORT_POMODORO_DURATION: {
                        int minutes = settings.short_pomodoro_duration;
                        if (PromptForInteger(hwnd, L"短番茄钟时长", L"请输入短番茄钟时长(1-720分钟):", minutes, 1, 720, &minutes)) {
                            settings.short_pomodoro_duration = minutes;
                            save_settings();
                        }
                        break;
                    }
                    case ID_MENU_SET_LONG_POMODORO_COUNT: {
                        int count = settings.long_pomodoro_count;
                        if (PromptForInteger(hwnd, L"长番茄钟番茄钟数", L"请输入长番茄钟完成后计入的番茄钟数(1-12):", count, 1, 12, &count)) {
                            settings.long_pomodoro_count = count;
                            save_settings();
                        }
                        break;
                    }
                    case ID_MENU_SET_SHORT_BREAK_DURATION: {
                        int minutes = settings.short_break_duration;
                        if (PromptForInteger(hwnd, L"短休息时长", L"请输入短休息时长(1-60分钟):", minutes, 1, 60, &minutes)) {
                            settings.short_break_duration = minutes;
                            save_settings();
                        }
                        break;
                    }
                    case ID_MENU_SET_LONG_BREAK_DURATION: {
                        int minutes = settings.long_break_duration;
                        if (PromptForInteger(hwnd, L"长休息时长", L"请输入长休息时长(1-120分钟):", minutes, 1, 120, &minutes)) {
                            settings.long_break_duration = minutes;
                            save_settings();
                        }
                        break;
                    }
                    case ID_MENU_SET_CUSTOM_DURATION: {
                        int minutes = settings.custom_duration;
                        if (PromptForInteger(hwnd, L"自定义时长", L"请输入自定义时长(1-720分钟):", minutes, 1, 720, &minutes)) {
                            settings.custom_duration = minutes;
                            save_settings();
                        }
                        break;
                    }
                    case ID_MENU_SET_TOAST_COLLAPSE_SECONDS: {
                        int seconds = settings.toast_auto_collapse_seconds;
                        if (PromptForInteger(hwnd, L"弹窗自动折叠", L"请输入自动折叠秒数(0-600，0=不自动折叠):", seconds, 0, 600, &seconds)) {
                            settings.toast_auto_collapse_seconds = seconds;
                            save_settings();
                        }
                        break;
                    }
                    case ID_MENU_SET_TIME_BLOCK: {
                        int minutes = settings.adjust_block_minutes;
                        if (PromptForInteger(hwnd, L"步进分钟", L"请输入步进分钟数(1-120):", minutes, 1, 120, &minutes)) {
                            settings.adjust_block_minutes = minutes;
                            save_settings();
                        }
                        break;
                    }
                    case ID_MENU_IMPORT_DATA:
                        import_data_from_folder(hwnd);
                        break;
                    case ID_MENU_EXPORT_DATA:
                        export_data_to_folder(hwnd);
                        break;
                    case ID_MENU_RESET_DEFAULTS:
                        reset_defaults_keep_data();
                        if (save_settings()) {
                            refresh_timer_icon_by_state(hwnd);
                            MessageBoxW(hwnd, L"已恢复默认配置。", L"提示", MB_OK | MB_ICONINFORMATION);
                        }
                        break;
                    case ID_MENU_DATA_LOC_DEFAULT:
                        switch_data_location(hwnd, DATA_LOC_DEFAULT, NULL);
                        break;
                    case ID_MENU_DATA_LOC_ONEDRIVE:
                        if (!switch_data_location(hwnd, DATA_LOC_ONEDRIVE, NULL)) {
                            MessageBoxW(hwnd, L"未检测到 OneDrive 路径。", L"提示", MB_OK | MB_ICONINFORMATION);
                        }
                        break;
                    case ID_MENU_DATA_LOC_CUSTOM: {
                        wchar_t picked[MAX_PATH] = L"";
                        if (choose_folder_dialog(hwnd, L"选择自定义位置", picked, MAX_PATH)) {
                            switch_data_location(hwnd, DATA_LOC_CUSTOM, picked);
                        }
                        break;
                    }
                    case ID_MENU_RESET_COUNT:
                        if (is_running || is_paused) {
                            MessageBoxW(hwnd, L"计时进行中，无法修改今日计数。", L"提示", MB_OK | MB_ICONINFORMATION);
                            break;
                        }
                        if (!sync_today_count_to_target(0)) {
                            MessageBoxW(hwnd, L"今日统计同步失败，无法写入调整记录。", L"错误", MB_OK | MB_ICONERROR);
                            break;
                        }
                        pomodoro_count = 0;
                        save_settings();
                        if (g_hHeatmapWnd) InvalidateRect(g_hHeatmapWnd, NULL, TRUE);
                        refresh_timer_icon_by_state(hwnd);
                        break;
                    case ID_MENU_SET_TIME: {
                        if (!is_running && !is_paused) {
                            MessageBoxW(hwnd, L"当前没有活动计时。", L"提示", MB_OK | MB_ICONINFORMATION);
                            break;
                        }
                        if (is_overtime) {
                            int minutes = overtime_seconds / 60;
                            if (PromptForInteger(hwnd, L"修改已超时时间", L"请输入已超时分钟(0-720):", minutes, 0, 720, &minutes)) {
                                overtime_seconds = minutes * 60;
                                refresh_timer_icon_by_state(hwnd);
                            }
                            break;
                        }
                        int minutes = is_count_up_timer ? remaining_seconds / 60 :
                            (remaining_seconds > 0 ? (remaining_seconds + 59) / 60 :
                            (settings.default_pomodoro_is_long ? get_long_pomodoro_duration() : settings.short_pomodoro_duration));
                        int minMinutes = is_count_up_timer ?
                            (count_up_credited * count_up_threshold_seconds + 59) / 60 : 1;
                        int maxMinutes = is_count_up_timer ? 9999 : 720;
                        const wchar_t* title = is_count_up_timer ? L"修改已计时时间" : L"修改剩余时间";
                        const wchar_t* label = is_count_up_timer ? L"请输入已计时分钟(不能低于已计数阈值):" : L"请输入剩余分钟(1-720):";
                        if (PromptForInteger(hwnd, title, label, minutes, minMinutes, maxMinutes, &minutes)) {
                            remaining_seconds = minutes * 60;
                            if (is_count_up_timer) credit_count_up_thresholds(hwnd);
                            refresh_timer_icon_by_state(hwnd);
                        }
                        break;
                    }
                    case ID_MENU_PLUS_5_MIN:
                        if (is_running || is_paused) {
                            if (is_overtime) {
                                overtime_seconds = clamp_int(overtime_seconds + settings.adjust_block_minutes * 60, 0, 720 * 60);
                                refresh_timer_icon_by_state(hwnd);
                                break;
                            }
                            int maxSeconds = is_count_up_timer ? 9999 * 60 : 12 * 60 * 60;
                            remaining_seconds = clamp_int(remaining_seconds + settings.adjust_block_minutes * 60,
                                is_count_up_timer ? 0 : 60, maxSeconds);
                            if (is_count_up_timer) credit_count_up_thresholds(hwnd);
                            refresh_timer_icon_by_state(hwnd);
                        } else {
                            MessageBoxW(hwnd, L"当前没有活动计时。", L"提示", MB_OK | MB_ICONINFORMATION);
                        }
                        break;
                    case ID_MENU_MINUS_5_MIN:
                        if (is_running || is_paused) {
                            if (is_overtime) {
                                overtime_seconds = clamp_int(overtime_seconds - settings.adjust_block_minutes * 60, 0, 720 * 60);
                                refresh_timer_icon_by_state(hwnd);
                                break;
                            }
                            int minSeconds = is_count_up_timer ? count_up_credited * count_up_threshold_seconds : 1;
                            int maxSeconds = is_count_up_timer ? 9999 * 60 : 12 * 60 * 60;
                            remaining_seconds = clamp_int(remaining_seconds - settings.adjust_block_minutes * 60, minSeconds, maxSeconds);
                            refresh_timer_icon_by_state(hwnd);
                        } else {
                            MessageBoxW(hwnd, L"当前没有活动计时。", L"提示", MB_OK | MB_ICONINFORMATION);
                        }
                        break;
                    case ID_MENU_SET_COUNT: {
                        if (is_running || is_paused) {
                            MessageBoxW(hwnd, L"计时进行中，无法修改今日计数。", L"提示", MB_OK | MB_ICONINFORMATION);
                            break;
                        }
                        int count = pomodoro_count;
                        if (PromptForInteger(hwnd, L"修改今日番茄数", L"请输入今日已完成番茄钟数量(0-9999):", count, 0, 9999, &count)) {
                            if (!sync_today_count_to_target(count)) {
                                MessageBoxW(hwnd, L"今日统计同步失败，无法写入调整记录。", L"错误", MB_OK | MB_ICONERROR);
                                break;
                            }
                            pomodoro_count = count;
                            save_settings();
                            if (g_hHeatmapWnd) InvalidateRect(g_hHeatmapWnd, NULL, TRUE);
                            refresh_timer_icon_by_state(hwnd);
                        }
                        break;
                    }
                    case ID_MENU_IDLE_POMODORO:
                        if (is_running || is_paused) {
                            stop_timer_thread_if_needed();
                            is_paused = 0;
                            current_timer_mode = TIMER_NONE;
                            remaining_seconds = 0;
                            clear_count_up_state();
                            clear_overtime_state();
                        }
                        idle_mode = IDLE_POMODORO;
                        idle_pomodoro_is_long = 1;
                        save_settings();
                        refresh_timer_icon_by_state(hwnd);
                        break;
                    case ID_MENU_IDLE_SHORT_POMODORO:
                        if (is_running || is_paused) {
                            stop_timer_thread_if_needed();
                            is_paused = 0;
                            current_timer_mode = TIMER_NONE;
                            remaining_seconds = 0;
                            clear_count_up_state();
                            clear_overtime_state();
                        }
                        idle_mode = IDLE_POMODORO;
                        idle_pomodoro_is_long = 0;
                        save_settings();
                        refresh_timer_icon_by_state(hwnd);
                        break;
                    case ID_MENU_IDLE_SHORT_BREAK:
                        if (is_running || is_paused) {
                            stop_timer_thread_if_needed();
                            is_paused = 0;
                            current_timer_mode = TIMER_NONE;
                            remaining_seconds = 0;
                            clear_count_up_state();
                            clear_overtime_state();
                        }
                        idle_mode = IDLE_BREAK;
                        idle_break_is_long = 0;
                        save_settings();
                        refresh_timer_icon_by_state(hwnd);
                        break;
                    case ID_MENU_IDLE_LONG_BREAK:
                        if (is_running || is_paused) {
                            stop_timer_thread_if_needed();
                            is_paused = 0;
                            current_timer_mode = TIMER_NONE;
                            remaining_seconds = 0;
                            clear_count_up_state();
                            clear_overtime_state();
                        }
                        idle_mode = IDLE_BREAK;
                        idle_break_is_long = 1;
                        save_settings();
                        refresh_timer_icon_by_state(hwnd);
                        break;
                    case ID_MENU_IDLE_COUNT_UP:
                        if (is_running || is_paused) {
                            stop_timer_thread_if_needed();
                            is_paused = 0;
                            current_timer_mode = TIMER_NONE;
                            remaining_seconds = 0;
                            clear_count_up_state();
                            clear_overtime_state();
                        }
                        idle_mode = IDLE_COUNT_UP;
                        save_settings();
                        refresh_timer_icon_by_state(hwnd);
                        break;
                    case ID_MENU_IDLE_CUSTOM:
                        if (is_running || is_paused) {
                            stop_timer_thread_if_needed();
                            is_paused = 0;
                            current_timer_mode = TIMER_NONE;
                            remaining_seconds = 0;
                            clear_count_up_state();
                            clear_overtime_state();
                        }
                        idle_mode = IDLE_CUSTOM;
                        save_settings();
                        refresh_timer_icon_by_state(hwnd);
                        break;
                    case ID_MENU_REPORT:
                        generate_and_open_report(hwnd);
                        break;
                    case 8: // Exit
                        Shell_NotifyIcon(NIM_DELETE, &nid);
                        PostQuitMessage(0);
                        break;
                }
                DestroyMenu(hMenu);
            }
            break;
        case WM_TIMER:
            if (wParam == ID_MAIN_DAY_SYNC_TIMER) {
                refresh_today_count_if_day_changed(hwnd, 0);
                return 0;
            }
            break;
        case WM_DESTROY:
            // Clean up before exit
            KillTimer(hwnd, ID_MAIN_DAY_SYNC_TIMER);
            stop_timer_thread_if_needed();
            is_paused = 0;
            if (g_settings_lock_ready) {
                DeleteCriticalSection(&g_settings_file_lock);
                g_settings_lock_ready = 0;
            }
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        case WM_TOAST_NOTIFY:
            ShowCompletionNotification(hwnd, (int)wParam);
            return 0;
        case WM_SETTINGS_SAVE_FAILED:
            MessageBoxW(hwnd,
                L"配置保存失败。请检查当前数据目录是否可写、磁盘空间是否充足，或 OneDrive 是否可用。",
                L"保存失败", MB_OK | MB_ICONERROR);
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Load settings from file
static int extract_json_int(const char* buf, const char* key, int defaultValue) {
    const char* pos = strstr(buf, key);
    int value;
    if (!pos) return defaultValue;
    pos = strchr(pos, ':');
    if (!pos) return defaultValue;
    if (sscanf(pos + 1, "%d", &value) == 1) return value;
    return defaultValue;
}

static int read_settings_json_buffer(char* buf, size_t bufSize) {
    FILE* fp;
    size_t n;
    if (!buf || bufSize == 0) return 0;
    buf[0] = '\0';

    fp = _wfopen(g_settings_path, L"r");
    if (fp) {
        n = fread(buf, 1, bufSize - 1, fp);
        fclose(fp);
        if (n > 0) {
            buf[n] = '\0';
            if (strstr(buf, "\"pomodoro_duration\"") && strstr(buf, "\"pomodoro_count\"")) {
                return 1;
            }
        }
    }

    fp = _wfopen(g_settings_tmp_path, L"r");
    if (fp) {
        n = fread(buf, 1, bufSize - 1, fp);
        fclose(fp);
        if (n > 0) {
            buf[n] = '\0';
            if (strstr(buf, "\"pomodoro_duration\"") && strstr(buf, "\"pomodoro_count\"")) {
                MoveFileExW(g_settings_tmp_path, g_settings_path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
                return 1;
            }
        }
    }

    fp = _wfopen(g_settings_bak_path, L"r");
    if (fp) {
        n = fread(buf, 1, bufSize - 1, fp);
        fclose(fp);
        if (n > 0) {
            buf[n] = '\0';
            if (strstr(buf, "\"pomodoro_duration\"") && strstr(buf, "\"pomodoro_count\"")) {
                CopyFileW(g_settings_bak_path, g_settings_path, FALSE);
                return 1;
            }
        }
    }

    return 0;
}

void load_settings() {
    if (g_settings_lock_ready) EnterCriticalSection(&g_settings_file_lock);

    settings.long_pomodoro_duration = 90;
    settings.long_pomodoro_count = 2;
    settings.short_pomodoro_duration = 45;
    settings.short_break_duration = 5;
    settings.long_break_duration = 15;
    settings.custom_duration = 10;
    settings.adjust_block_minutes = 5;
    settings.toast_auto_collapse_seconds = 10;
    settings.enable_clock_sound = 0;
    settings.enable_completion_sound = 1;
    settings.show_completion_dialog = 1;
    settings.default_pomodoro_is_long = 1;
    settings.default_break_is_long = 0;
    settings.enable_overtime_count_up = 1;
    pomodoro_count = 0;
    idle_mode = IDLE_POMODORO;
    idle_pomodoro_is_long = 1;
    idle_break_is_long = 0;

    {
        char buf[1024] = {0};
        if (read_settings_json_buffer(buf, sizeof(buf))) {
            settings.long_pomodoro_duration = extract_json_int(buf, "\"long_pomodoro_duration\"",
                extract_json_int(buf, "\"pomodoro_duration\"", settings.long_pomodoro_duration));
            settings.short_pomodoro_duration = extract_json_int(buf, "\"short_pomodoro_duration\"", settings.short_pomodoro_duration);
            settings.long_pomodoro_count = extract_json_int(buf, "\"long_pomodoro_count\"", settings.long_pomodoro_count);
            settings.short_break_duration = extract_json_int(buf, "\"short_break_duration\"", settings.short_break_duration);
            settings.long_break_duration = extract_json_int(buf, "\"long_break_duration\"", settings.long_break_duration);
            settings.custom_duration = extract_json_int(buf, "\"custom_duration\"", settings.custom_duration);
            settings.adjust_block_minutes = extract_json_int(buf, "\"adjust_block_minutes\"", settings.adjust_block_minutes);
            settings.toast_auto_collapse_seconds = extract_json_int(buf, "\"toast_auto_collapse_seconds\"", settings.toast_auto_collapse_seconds);
            settings.enable_clock_sound = extract_json_int(buf, "\"enable_clock_sound\"", settings.enable_clock_sound);
            settings.enable_completion_sound = extract_json_int(buf, "\"enable_completion_sound\"", settings.enable_completion_sound);
            settings.show_completion_dialog = extract_json_int(buf, "\"show_completion_dialog\"", settings.show_completion_dialog);
            settings.default_pomodoro_is_long = extract_json_int(buf, "\"default_pomodoro_is_long\"", settings.default_pomodoro_is_long);
            settings.default_break_is_long = extract_json_int(buf, "\"default_break_is_long\"", settings.default_break_is_long);
            settings.enable_overtime_count_up = extract_json_int(buf, "\"enable_overtime_count_up\"", settings.enable_overtime_count_up);
            pomodoro_count = extract_json_int(buf, "\"pomodoro_count\"", pomodoro_count);
            idle_mode = (IdleMode)extract_json_int(buf, "\"idle_mode\"", (int)idle_mode);
            idle_pomodoro_is_long = extract_json_int(buf, "\"idle_pomodoro_is_long\"", idle_pomodoro_is_long);
            idle_break_is_long = extract_json_int(buf, "\"idle_break_is_long\"", idle_break_is_long);
        }
    }

    settings.long_pomodoro_duration = clamp_int(settings.long_pomodoro_duration, 1, 720);
    settings.short_pomodoro_duration = clamp_int(settings.short_pomodoro_duration, 1, 720);
    settings.long_pomodoro_count = clamp_int(settings.long_pomodoro_count, 1, 12);
    settings.short_break_duration = clamp_int(settings.short_break_duration, 1, 60);
    settings.long_break_duration = clamp_int(settings.long_break_duration, 1, 120);
    settings.custom_duration = clamp_int(settings.custom_duration, 1, 720);
    settings.adjust_block_minutes = clamp_int(settings.adjust_block_minutes, 1, 120);
    settings.toast_auto_collapse_seconds = clamp_int(settings.toast_auto_collapse_seconds, 0, 600);
    settings.enable_clock_sound = settings.enable_clock_sound ? 1 : 0;
    settings.enable_completion_sound = settings.enable_completion_sound ? 1 : 0;
    settings.show_completion_dialog = settings.show_completion_dialog ? 1 : 0;
    settings.default_pomodoro_is_long = settings.default_pomodoro_is_long ? 1 : 0;
    settings.default_break_is_long = settings.default_break_is_long ? 1 : 0;
    settings.enable_overtime_count_up = settings.enable_overtime_count_up ? 1 : 0;
    pomodoro_count = clamp_int(pomodoro_count, 0, 9999);
    if (idle_mode != IDLE_POMODORO && idle_mode != IDLE_BREAK && idle_mode != IDLE_CUSTOM && idle_mode != IDLE_COUNT_UP) {
        idle_mode = IDLE_POMODORO;
    }
    idle_pomodoro_is_long = idle_pomodoro_is_long ? 1 : 0;
    idle_break_is_long = idle_break_is_long ? 1 : 0;

    if (g_settings_lock_ready) LeaveCriticalSection(&g_settings_file_lock);
}

// Save settings to file
int save_settings(void) {
    int saved = 0;
    int writeOk = 0;
    int longDuration = settings.long_pomodoro_duration;

    if (g_settings_lock_ready) EnterCriticalSection(&g_settings_file_lock);

    if (GetFileAttributesW(g_settings_bak_path) == INVALID_FILE_ATTRIBUTES) {
        CopyFileW(g_settings_path, g_settings_bak_path, TRUE);
    }

    FILE* fp = _wfopen(g_settings_tmp_path, L"w");
    if (fp) {
        writeOk = fprintf(fp, "{\"pomodoro_duration\":%d,\"long_pomodoro_duration\":%d,\"long_pomodoro_count\":%d,\"short_pomodoro_duration\":%d,\"short_break_duration\":%d,\"long_break_duration\":%d,\"custom_duration\":%d,\"adjust_block_minutes\":%d,\"toast_auto_collapse_seconds\":%d,\"enable_clock_sound\":%d,\"enable_completion_sound\":%d,\"show_completion_dialog\":%d,\"default_pomodoro_is_long\":%d,\"default_break_is_long\":%d,\"enable_overtime_count_up\":%d,\"pomodoro_count\":%d,\"idle_mode\":%d,\"idle_pomodoro_is_long\":%d,\"idle_break_is_long\":%d}",
            longDuration, longDuration, settings.long_pomodoro_count, settings.short_pomodoro_duration,
            settings.short_break_duration, settings.long_break_duration, settings.custom_duration,
            settings.adjust_block_minutes, settings.toast_auto_collapse_seconds, settings.enable_clock_sound,
            settings.enable_completion_sound, settings.show_completion_dialog, settings.default_pomodoro_is_long,
            settings.default_break_is_long, settings.enable_overtime_count_up, pomodoro_count, (int)idle_mode, idle_pomodoro_is_long, idle_break_is_long) >= 0;
        if (writeOk && fflush(fp) != 0) writeOk = 0;
        if (fclose(fp) != 0) writeOk = 0;

        if (writeOk && MoveFileExW(g_settings_tmp_path, g_settings_path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            saved = 1;
        } else if (writeOk && CopyFileW(g_settings_tmp_path, g_settings_path, FALSE)) {
            DeleteFileW(g_settings_tmp_path);
            saved = 1;
        }

        if (saved) {
            CopyFileW(g_settings_path, g_settings_bak_path, FALSE);
        }
    }

    if (g_settings_lock_ready) LeaveCriticalSection(&g_settings_file_lock);

    if (!saved) {
        OutputDebugStringA("Failed to save pomodoro_settings.json\n");
        if (g_main_hwnd) {
            PostMessageW(g_main_hwnd, WM_SETTINGS_SAVE_FAILED, 0, 0);
        } else {
            MessageBoxW(NULL,
                L"配置保存失败。请检查数据目录写入权限、磁盘空间或 OneDrive 状态。",
                L"保存失败", MB_OK | MB_ICONERROR);
        }
    }
    return saved;
}

// Main entry point
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    HANDLE hEvent = CreateEventW(NULL, TRUE, FALSE, L"PomodoroTimerEvent");
    if (!hEvent) {
        MessageBoxW(NULL, L"无法创建程序单实例标识。", L"启动失败", MB_OK | MB_ICONERROR);
        return 1;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(NULL, L"程序已在运行，将退出本次启动。", L"提示", MB_OK | MB_ICONWARNING);
        CloseHandle(hEvent);
        return 0;
    }

    // Initialize
    init_system_metrics();
    InitializeCriticalSection(&g_settings_file_lock);
    g_settings_lock_ready = 1;
    load_data_location_from_registry();
    load_settings();
    pomodoro_count = get_today_count_from_storage();
    refresh_today_count_if_day_changed(NULL, 1);
    // Startup begins with the user's selected default Pomodoro mode.
    idle_mode = IDLE_POMODORO;
    idle_pomodoro_is_long = settings.default_pomodoro_is_long;
    idle_break_is_long = settings.default_break_is_long;
    autostart_enabled = is_autostart_enabled();

    // Force Simplified Chinese UI text pack.
    g_lang = &lang_en;

    // Create window class
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"Pomodoro";
    if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        MessageBoxW(NULL, L"程序窗口类注册失败。", L"启动失败", MB_OK | MB_ICONERROR);
        DeleteCriticalSection(&g_settings_file_lock);
        g_settings_lock_ready = 0;
        CloseHandle(hEvent);
        return 1;
    }

    // Create invisible window
    HWND hwnd = CreateWindowW(L"Pomodoro", L"Pomodoro", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        MessageBoxW(NULL, L"程序主窗口创建失败。", L"启动失败", MB_OK | MB_ICONERROR);
        DeleteCriticalSection(&g_settings_file_lock);
        g_settings_lock_ready = 0;
        CloseHandle(hEvent);
        return 1;
    }
    g_main_hwnd = hwnd;
    g_taskbar_created_message = RegisterWindowMessageW(L"TaskbarCreated");

    // Setup tray icon
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcsncpy(nid.szTip, g_lang->tooltip_pomodoro, sizeof(nid.szTip)/sizeof(nid.szTip[0]) - 1);
    nid.szTip[sizeof(nid.szTip)/sizeof(nid.szTip[0]) - 1] = L'\0';
    if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
        MessageBoxW(NULL, L"系统托盘图标创建失败。", L"启动失败", MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
        CloseHandle(hEvent);
        return 1;
    }

    // Show initial icon
    update_tray_icon(hwnd, L"\u25BA", pomodoro_count, 0);

    // Main message loop
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Clean up
    if (last_icon) {
        DestroyIcon(last_icon);
        last_icon = NULL;
    }
    CloseHandle(hEvent);

    return 0;
}

// Some toolchains (including portable LLVM-based ones) look for wWinMain
// when building Unicode Windows apps. Route it to the existing WinMain.
#ifndef _MSC_VER
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR lpCmdLine, _In_ int nCmdShow) {
    (void)lpCmdLine;
    return WinMain(hInstance, hPrevInstance, NULL, nCmdShow);
}
#endif
