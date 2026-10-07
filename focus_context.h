#ifndef POMODORO_FOCUS_CONTEXT_H
#define POMODORO_FOCUS_CONTEXT_H
typedef struct { HWND foreground, focus; } UiFocusReturn;
static UiFocusReturn ui_menu_return;
static int ui_menu_active;
static HWND ui_foreground(void) {
    HWND window=GetForegroundWindow();return IsWindow(window)?window:GetActiveWindow();
}
static int ui_owned_by(HWND candidate,HWND temporary) {
    if(!temporary)return 0;
    if(candidate==temporary || IsChild(temporary,candidate))return 1;
    HWND owner=candidate;while((owner=GetWindow(owner,GW_OWNER))!=NULL)if(owner==temporary)return 1;
    return 0;
}
static int ui_work_window(HWND window,HWND temporary) {
    wchar_t cls[64];
    if(!IsWindow(window)||!IsWindowVisible(window)||window==g_main_hwnd||ui_owned_by(window,temporary))return 0;
    GetClassNameW(window,cls,64);
    return wcscmp(cls,L"PomodoroStrongReminder")!=0 && wcscmp(cls,TOAST_WINDOW_CLASS)!=0;
}
static void ui_focus_remember(UiFocusReturn *context,HWND candidate,HWND temporary) {
    if(!ui_work_window(candidate,temporary))return;
    context->foreground=candidate;
    HWND focus=GetFocus();context->focus=(focus==candidate||IsChild(candidate,focus))?focus:NULL;
}
static UiFocusReturn ui_focus_capture(void) {
    UiFocusReturn result={0};ui_focus_remember(&result,ui_foreground(),NULL);return result;
}
static UiFocusReturn ui_dialog_return(void) {
    UiFocusReturn result=ui_focus_capture();
    if(!result.foreground && ui_menu_active)result=ui_menu_return;
    return result;
}
static void ui_focus_apply(UiFocusReturn context) {
    if(!ui_work_window(context.foreground,NULL))return;
    SetForegroundWindow(context.foreground);
    if(GetWindowThreadProcessId(context.foreground,NULL)==GetCurrentThreadId()) {
        HWND focus=context.focus;
        if(!IsWindow(focus)||!(focus==context.foreground||IsChild(context.foreground,focus)))focus=context.foreground;
        SetFocus(focus);
    }
}
static void ui_focus_restore(UiFocusReturn context,HWND temporary) {
    HWND current=ui_foreground();
    if(ui_work_window(current,temporary) && current!=context.foreground)return;
    ui_focus_apply(context);
}
static void ui_menu_begin(void) { ui_menu_return=ui_focus_capture();ui_menu_active=1; }
static void ui_menu_end(void) { ui_focus_restore(ui_menu_return,NULL);ui_menu_active=0; }
#endif
