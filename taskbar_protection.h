#ifndef POMODORO_TASKBAR_PROTECTION_H
#define POMODORO_TASKBAR_PROTECTION_H
#define WM_TASKBAR_RECONCILE (WM_APP+121)
typedef struct { FsHiddenWindow identity;int action,hide_pending,hidden_seen; } TaskbarLease;
/* Owned exclusively by a single worker; retained across a fully joined restart. */
static TaskbarLease fs_taskbar_leases[FS_TASKBAR_MAX];
static size_t fs_taskbar_lease_count;
#ifdef POMODORO_TEST_IO
static int test_taskbar_trace;
#endif
static int fs_taskbar_submit(HWND window,int command) {
    SetLastError(0);int accepted=ShowWindowAsync(window,command)!=0;
#ifdef POMODORO_TEST_IO
    if(test_taskbar_trace)printf("TASKBAR request cmd=%d accepted=%d visible=%d error=%lu\n",command,accepted,IsWindowVisible(window),GetLastError());
#endif
    return accepted;
}
static void fs_taskbar_wake(void) {if(fs_taskbar_thread)PostThreadMessageW(fs_taskbar_thread_id,WM_TASKBAR_RECONCILE,0,0);}
static void fs_publish_taskbar_targets(void) {
    FsHiddenWindow next[FS_TASKBAR_MAX],recovery[FS_TASKBAR_MAX];size_t count=0,recover_count;
    AcquireSRWLockShared(&fs_taskbar_lock);recover_count=fs_taskbar_recovery_count;memcpy(recovery,fs_taskbar_recovery,recover_count*sizeof(*recovery));ReleaseSRWLockShared(&fs_taskbar_lock);
    /* Queries deliberately stay outside the snapshot lock. Recovery leases
       inherit original visibility when a previous asynchronous restore is pending. */
    for(size_t pass=0;pass<2;++pass){FsHiddenWindow *items=pass?recovery:fs_hidden_windows;size_t size=pass?recover_count:fs_hidden_count;
        for(size_t i=0;i<size;++i)if(fs_hidden_valid(&items[i])&&fs_is_on_fullscreen_monitor_handle(items[i].window)){
            size_t j;for(j=0;j<count;++j)if(next[j].window==items[i].window)break;
            if(j==count&&count<FS_TASKBAR_MAX)next[count++]=items[i];}}
    AcquireSRWLockExclusive(&fs_taskbar_lock);memcpy(fs_taskbar_targets,next,count*sizeof(*next));fs_taskbar_target_count=count;++fs_taskbar_generation;ReleaseSRWLockExclusive(&fs_taskbar_lock);
    fs_taskbar_wake();
}
static int fs_taskbar_identity_equal(FsHiddenWindow *a,FsHiddenWindow *b) {return a->window==b->window&&a->process==b->process&&!wcscmp(a->cls,b->cls);}
static void fs_taskbar_reconcile(HWND event_window,DWORD event) {
    FsHiddenWindow targets[FS_TASKBAR_MAX],pending[FS_TASKBAR_MAX];size_t count,pending_count=0;
    AcquireSRWLockShared(&fs_taskbar_lock);count=fs_taskbar_target_count;memcpy(targets,fs_taskbar_targets,count*sizeof(*targets));ReleaseSRWLockShared(&fs_taskbar_lock);
    for(size_t j=0;j<count;++j){size_t i;for(i=0;i<fs_taskbar_lease_count;++i)if(fs_taskbar_identity_equal(&targets[j],&fs_taskbar_leases[i].identity))break;
        if(i==fs_taskbar_lease_count&&i<FS_TASKBAR_MAX){memset(&fs_taskbar_leases[i],0,sizeof(fs_taskbar_leases[i]));fs_taskbar_leases[i].identity=targets[j];++fs_taskbar_lease_count;}}
    size_t i=0;
    while(i<fs_taskbar_lease_count){TaskbarLease *lease=&fs_taskbar_leases[i];
        if(!fs_hidden_valid(&lease->identity)){memmove(lease,lease+1,(--fs_taskbar_lease_count-i)*sizeof(*lease));continue;}
        int desired=0;for(size_t j=0;j<count;++j)if(fs_taskbar_identity_equal(&targets[j],&lease->identity)){desired=1;break;}
        if(InterlockedCompareExchange(&fs_taskbar_stopping,0,0))desired=0;
        int visible=IsWindowVisible(lease->identity.window);
        if(!visible||(event_window==lease->identity.window&&event==EVENT_OBJECT_HIDE)){lease->hidden_seen=1;lease->hide_pending=0;}
        if(desired){
            if(lease->action==2||(!lease->hide_pending&&visible)){
                if(fs_taskbar_submit(lease->identity.window,SW_HIDE)){lease->action=1;lease->hide_pending=1;}
            }
        }else{
            /* Restore even if hide was merely accepted and still in the target queue. */
            if(lease->action==1){if(fs_taskbar_submit(lease->identity.window,SW_SHOWNOACTIVATE))lease->action=2;}
            if(!lease->action||(lease->action==2&&lease->hidden_seen&&visible)){
                memmove(lease,lease+1,(--fs_taskbar_lease_count-i)*sizeof(*lease));continue;}
        }
        if(lease->action&&pending_count<FS_TASKBAR_MAX)pending[pending_count++]=lease->identity;
        ++i;
    }
    AcquireSRWLockExclusive(&fs_taskbar_lock);memcpy(fs_taskbar_recovery,pending,pending_count*sizeof(*pending));fs_taskbar_recovery_count=pending_count;ReleaseSRWLockExclusive(&fs_taskbar_lock);
}
static void CALLBACK fs_taskbar_event(HWINEVENTHOOK hook,DWORD event,HWND window,LONG object,LONG child,DWORD thread,DWORD time) {
    (void)hook;(void)thread;(void)time;if(object!=OBJID_WINDOW||child!=CHILDID_SELF||!window)return;
    fs_taskbar_reconcile(window,event);
}
static DWORD WINAPI fs_taskbar_events(void *ready) {
    SetThreadDesktop(GetThreadDesktop(GetWindowThreadProcessId(g_main_hwnd,NULL)));MSG message;PeekMessageW(&message,NULL,WM_USER,WM_USER,PM_NOREMOVE);
    HWINEVENTHOOK show=SetWinEventHook(EVENT_OBJECT_SHOW,EVENT_OBJECT_SHOW,NULL,fs_taskbar_event,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);
    HWINEVENTHOOK reorder=SetWinEventHook(EVENT_OBJECT_REORDER,EVENT_OBJECT_REORDER,NULL,fs_taskbar_event,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);
    HWINEVENTHOOK hide=SetWinEventHook(EVENT_OBJECT_HIDE,EVENT_OBJECT_HIDE,NULL,fs_taskbar_event,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);
    InterlockedExchange(&fs_taskbar_available,show&&reorder&&hide);SetEvent((HANDLE)ready);CloseHandle((HANDLE)ready);
    if(fs_taskbar_available)while(GetMessageW(&message,NULL,0,0)>0){
        if(message.message==WM_TASKBAR_RECONCILE){while(PeekMessageW(&message,NULL,WM_TASKBAR_RECONCILE,WM_TASKBAR_RECONCILE,PM_REMOVE)){}fs_taskbar_reconcile(NULL,0);continue;}
        TranslateMessage(&message);DispatchMessageW(&message);
    }
    InterlockedExchange(&fs_taskbar_stopping,1);fs_taskbar_reconcile(NULL,0);InterlockedExchange(&fs_taskbar_available,0);
    if(show)UnhookWinEvent(show);if(reorder)UnhookWinEvent(reorder);if(hide)UnhookWinEvent(hide);return 0;
}
static void fs_start_taskbar_events(void) {
    if(fs_taskbar_thread){if(WaitForSingleObject(fs_taskbar_thread,0)!=WAIT_OBJECT_0)return;CloseHandle(fs_taskbar_thread);fs_taskbar_thread=NULL;}
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,NULL),worker_ready=NULL;if(!ready)return;
    if(!DuplicateHandle(GetCurrentProcess(),ready,GetCurrentProcess(),&worker_ready,0,FALSE,DUPLICATE_SAME_ACCESS)){CloseHandle(ready);return;}
    InterlockedExchange(&fs_taskbar_stopping,0);fs_taskbar_thread=CreateThread(NULL,0,fs_taskbar_events,worker_ready,0,&fs_taskbar_thread_id);
    if(fs_taskbar_thread)WaitForSingleObject(ready,1000);else CloseHandle(worker_ready);CloseHandle(ready);
}
static void fs_stop_taskbar_events(void) {
    InterlockedExchange(&fs_taskbar_stopping,1);
    AcquireSRWLockExclusive(&fs_taskbar_lock);fs_taskbar_target_count=0;++fs_taskbar_generation;ReleaseSRWLockExclusive(&fs_taskbar_lock);
    if(fs_taskbar_thread){PostThreadMessageW(fs_taskbar_thread_id,WM_QUIT,0,0);
        if(WaitForSingleObject(fs_taskbar_thread,2000)==WAIT_OBJECT_0){CloseHandle(fs_taskbar_thread);fs_taskbar_thread=NULL;fs_taskbar_thread_id=0;}}
}
#endif
