#ifndef POMODORO_DARK_MODE_H
#define POMODORO_DARK_MODE_H

#include <windows.h>

typedef enum {
    APP_MODE_DEFAULT = 0,
    APP_MODE_ALLOW_DARK = 1,
    APP_MODE_FORCE_DARK = 2,
    APP_MODE_FORCE_LIGHT = 3,
    APP_MODE_MAX = 4
} PreferredAppMode;

typedef PreferredAppMode (WINAPI *pfnSetPreferredAppMode)(PreferredAppMode);
typedef BOOL (WINAPI *pfnAllowDarkModeForApp)(BOOL);
typedef void (WINAPI *pfnFlushMenuThemes)(void);
typedef BOOL (WINAPI *pfnAllowDarkModeForWindow)(HWND, BOOL);
typedef BOOL (WINAPI *pfnShouldAppsUseDarkMode)(void);

static pfnSetPreferredAppMode g_pfnSetPreferredAppMode = NULL;
static pfnAllowDarkModeForApp g_pfnAllowDarkModeForApp = NULL;
static pfnFlushMenuThemes g_pfnFlushMenuThemes = NULL;
static pfnAllowDarkModeForWindow g_pfnAllowDarkModeForWindow = NULL;
static pfnShouldAppsUseDarkMode g_pfnShouldAppsUseDarkMode = NULL;
static int g_darkModeSupported = 0;

typedef LONG (NTAPI *pfnRtlGetVersion)(PRTL_OSVERSIONINFOW);

static void init_dark_mode(void) {
    OSVERSIONINFOEXW osvi = { sizeof(osvi) };
    DWORD build = 0;

    HMODULE hNt = GetModuleHandleW(L"ntdll.dll");
    if (hNt) {
        pfnRtlGetVersion rtlGetVersion = (pfnRtlGetVersion)(void*)GetProcAddress(hNt, "RtlGetVersion");
        if (rtlGetVersion && rtlGetVersion((PRTL_OSVERSIONINFOW)&osvi) == 0) {
            build = osvi.dwBuildNumber;
        }
    }
    if (build < 17763) {
        return;
    }

    HMODULE hUx = LoadLibraryW(L"uxtheme.dll");
    if (!hUx) {
        return;
    }

    g_pfnFlushMenuThemes = (pfnFlushMenuThemes)(void*)GetProcAddress(hUx, MAKEINTRESOURCEA(136));
    g_pfnAllowDarkModeForWindow = (pfnAllowDarkModeForWindow)(void*)GetProcAddress(hUx, MAKEINTRESOURCEA(133));
    g_pfnShouldAppsUseDarkMode = (pfnShouldAppsUseDarkMode)(void*)GetProcAddress(hUx, MAKEINTRESOURCEA(132));

    if (build >= 18362) {
        g_pfnSetPreferredAppMode = (pfnSetPreferredAppMode)(void*)GetProcAddress(hUx, MAKEINTRESOURCEA(135));
        if (g_pfnSetPreferredAppMode) {
            g_pfnSetPreferredAppMode(APP_MODE_ALLOW_DARK);
            g_darkModeSupported = 1;
        }
    } else {
        g_pfnAllowDarkModeForApp = (pfnAllowDarkModeForApp)(void*)GetProcAddress(hUx, MAKEINTRESOURCEA(135));
        if (g_pfnAllowDarkModeForApp) {
            g_pfnAllowDarkModeForApp(TRUE);
            g_darkModeSupported = 1;
        }
    }

    if (g_pfnFlushMenuThemes) {
        g_pfnFlushMenuThemes();
    }
}

static void apply_dark_mode_to_window(HWND hwnd) {
    if (hwnd && g_pfnAllowDarkModeForWindow) {
        g_pfnAllowDarkModeForWindow(hwnd, TRUE);
    }
}

static void refresh_menu_theme(void) {
    if (g_pfnFlushMenuThemes) {
        g_pfnFlushMenuThemes();
    }
}

static int track_popup_menu_dark(HMENU hMenu, HWND hwnd, int x, int y) {
    BOOL menuDropAlign = FALSE;
    int cmd = 0;

    if (!hMenu || !hwnd) {
        return 0;
    }

    refresh_menu_theme();

    if (SystemParametersInfoW(SPI_GETMENUDROPALIGNMENT, 0, &menuDropAlign, 0) && menuDropAlign) {
        SystemParametersInfoW(SPI_SETMENUDROPALIGNMENT, 0, NULL, 0);
    } else {
        menuDropAlign = FALSE;
    }

    SetForegroundWindow(hwnd);
    cmd = TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, hwnd, NULL);

    if (menuDropAlign) {
        SystemParametersInfoW(SPI_SETMENUDROPALIGNMENT, 1, NULL, 0);
    }

    PostMessageW(hwnd, WM_NULL, 0, 0);
    return cmd;
}

#endif /* POMODORO_DARK_MODE_H */
