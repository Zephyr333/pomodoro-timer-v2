#ifndef POMODORO_IMPORT_TRANSACTION_H
#define POMODORO_IMPORT_TRANSACTION_H
typedef struct { wchar_t (*names)[MAX_PATH];size_t count; } DataManifest;
typedef struct { DWORD volume,high,low; } DataDirectoryId;
#ifdef POMODORO_TEST_IO
static int data_test_fail_phase,data_test_fail_restore;
#endif
static int data_directory_id(const wchar_t *path,DataDirectoryId *id) {
    HANDLE h=CreateFileW(path,FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
    if(h==INVALID_HANDLE_VALUE)return 0;BY_HANDLE_FILE_INFORMATION info;int ok=GetFileInformationByHandle(h,&info)!=0;CloseHandle(h);
    if(!ok||!(info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)||(!info.nFileIndexHigh&&!info.nFileIndexLow))return 0;
    id->volume=info.dwVolumeSerialNumber;id->high=info.nFileIndexHigh;id->low=info.nFileIndexLow;return 1;
}
static int data_same_id(DataDirectoryId a,DataDirectoryId b) {return a.volume==b.volume&&a.high==b.high&&a.low==b.low;}
static int data_manifest_add(DataManifest *list,const wchar_t *name) {
    if(list->count>=4096)return 0;void *next=realloc(list->names,(list->count+1)*sizeof(*list->names));if(!next)return 0;
    list->names=next;wcscpy(list->names[list->count++],name);return 1;
}
static int data_manifest_read(const wchar_t *dir,DataManifest *list) {
    static const wchar_t *fixed[]={L"pomodoro_settings.json",L"pomodoro_settings.json.tmp",L"pomodoro_settings.json.bak",L"pomodoro_log.csv",L"pomodoro_adjustments.csv"};
    wchar_t path[MAX_PATH];for(size_t i=0;i<5;++i){if(swprintf(path,MAX_PATH,L"%ls\\%ls",dir,fixed[i])<0)return 0;
        DWORD attr=GetFileAttributesW(path);if(attr==INVALID_FILE_ATTRIBUTES){if(GetLastError()!=ERROR_FILE_NOT_FOUND)return 0;continue;}
        if(attr&FILE_ATTRIBUTE_DIRECTORY)return 0;if(!data_manifest_add(list,fixed[i]))return 0;}
    if(swprintf(path,MAX_PATH,L"%ls\\pomodoro_log_*.csv",dir)<0)return 0;
    WIN32_FIND_DATAW item;HANDLE find=FindFirstFileW(path,&item);if(find==INVALID_HANDLE_VALUE)return GetLastError()==ERROR_FILE_NOT_FOUND;
    int ok=1;do{if(item.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)continue;if(!data_manifest_add(list,item.cFileName)){ok=0;break;}}while(FindNextFileW(find,&item));
    if(ok&&GetLastError()!=ERROR_NO_MORE_FILES)ok=0;FindClose(find);return ok;
}
static int data_files_equal(const wchar_t *a,const wchar_t *b) {
    FILE *one=_wfopen(a,L"rb"),*two=_wfopen(b,L"rb");int ok=one&&two;char x[4096],y[4096];
    if(ok){for(;;){size_t nx=fread(x,1,sizeof(x),one),ny=fread(y,1,sizeof(y),two);if(nx!=ny||memcmp(x,y,nx)){ok=0;break;}if(!nx)break;}
        if(ferror(one)||ferror(two))ok=0;}
    if(one&&fclose(one)!=0)ok=0;if(two&&fclose(two)!=0)ok=0;return ok;
}
static int data_copy_verified(const wchar_t *source,const wchar_t *target,int phase,int replace) {
#ifdef POMODORO_TEST_IO
    if(data_test_fail_phase==phase||(phase==4&&data_test_fail_restore))return 0;
#else
    (void)phase;
#endif
    HANDLE hold=CreateFileW(source,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);if(hold==INVALID_HANDLE_VALUE)return 0;
    int ok=0;if(!replace)ok=CopyFileW(source,target,TRUE)!=0&&data_files_equal(source,target);
    else {wchar_t staged[MAX_PATH];if(GetTempFileNameW(g_data_dir,L"pmi",0,staged)){
        if(CopyFileW(source,staged,FALSE)&&data_files_equal(source,staged)&&MoveFileExW(staged,target,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))ok=data_files_equal(source,target);
        if(!ok)DeleteFileW(staged);}}
    CloseHandle(hold);return ok;
}
static int data_copy_manifest(const wchar_t *source,const wchar_t *target,DataManifest *list,int phase,int replace) {
    wchar_t a[MAX_PATH],b[MAX_PATH];for(size_t i=0;i<list->count;++i){
        if(swprintf(a,MAX_PATH,L"%ls\\%ls",source,list->names[i])<0||swprintf(b,MAX_PATH,L"%ls\\%ls",target,list->names[i])<0)return 0;
        if(replace&&data_files_equal(a,b))continue;if(!data_copy_verified(a,b,phase,replace))return 0;}
    return 1;
}
static int data_restore_manifest(const wchar_t *source,const wchar_t *target,DataManifest *list) {
    wchar_t a[MAX_PATH],b[MAX_PATH];int ok=1;
    for(size_t i=0;i<list->count;++i){if(swprintf(a,MAX_PATH,L"%ls\\%ls",source,list->names[i])<0||swprintf(b,MAX_PATH,L"%ls\\%ls",target,list->names[i])<0){ok=0;continue;}
        if(!data_files_equal(a,b)&&!data_copy_verified(a,b,4,1))ok=0;}
    return ok;
}
static int data_manifest_has(DataManifest *list,const wchar_t *name) {for(size_t i=0;i<list->count;++i)if(!_wcsicmp(list->names[i],name))return 1;return 0;}
static int data_delete_manifest(const wchar_t *dir,DataManifest *list,DataManifest *keep) {
    int ok=1;wchar_t path[MAX_PATH];for(size_t i=0;i<list->count;++i){if(keep&&data_manifest_has(keep,list->names[i]))continue;
        if(swprintf(path,MAX_PATH,L"%ls\\%ls",dir,list->names[i])<0){ok=0;continue;}
        if(!DeleteFileW(path)&&GetLastError()!=ERROR_FILE_NOT_FOUND)ok=0;}
    return ok;
}
static int data_private_directory(const wchar_t *dir,const wchar_t *prefix,wchar_t *out) {
    GUID guid;wchar_t text[64];if(FAILED(CoCreateGuid(&guid))||!StringFromGUID2(&guid,text,64))return 0;
    if(swprintf(out,MAX_PATH,L"%ls\\%ls_%ls",dir,prefix,text)<0)return 0;return CreateDirectoryW(out,NULL)!=0;
}
static int data_import_transaction(HWND hwnd) {
    wchar_t source[MAX_PATH],current[MAX_PATH],stage[MAX_PATH]={0},backup[MAX_PATH]={0},message[512];
    DataDirectoryId source_id,current_id,recheck;DataManifest incoming={0},original={0};int ok=0,stage_created=0,backup_created=0,keep_stage=0;
    if(is_running){MessageBoxW(hwnd,L"请暂停计时后导入数据。",L"提示",MB_OK|MB_ICONINFORMATION);return 0;}
    if(!choose_folder_dialog(hwnd,L"选择导入目录",source,MAX_PATH))return 0;wcscpy(current,g_data_dir);
    if(!data_directory_id(source,&source_id)||!data_directory_id(current,&current_id)){
        MessageBoxW(hwnd,L"无法可靠确认数据目录身份，导入已取消。",L"导入失败",MB_OK|MB_ICONERROR);return 0;}
    if(data_same_id(source_id,current_id)){MessageBoxW(hwnd,L"不能从当前正在使用的数据目录或其别名导入。",L"提示",MB_OK|MB_ICONINFORMATION);return 0;}
    if(MessageBoxW(hwnd,L"导入会用所选备份替换当前数据。程序会先暂存导入文件并创建完整备份；是否继续？",L"确认导入",MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2)!=IDYES)return 0;
    if(is_running||_wcsicmp(current,g_data_dir))goto cancelled;
    if(!data_manifest_read(source,&incoming)||!incoming.count)goto cancelled;
    if(!(stage_created=data_private_directory(current,L"import_stage",stage))||!data_copy_manifest(source,stage,&incoming,1,0))goto cancelled;
    /* Source may be a junction that was retargeted while the folder dialog was open. */
    if(!data_directory_id(source,&recheck)||!data_same_id(source_id,recheck))goto cancelled;
    if(!data_manifest_read(current,&original))goto cancelled;
    if(!(backup_created=data_private_directory(current,L"import_backup",backup))||!data_copy_manifest(current,backup,&original,2,0))goto cancelled;
    if(is_running||_wcsicmp(current,g_data_dir)||!data_directory_id(current,&recheck)||!data_same_id(current_id,recheck))goto cancelled;
    if(!data_delete_manifest(current,&original,NULL)||!data_copy_manifest(stage,current,&incoming,3,1)){
        int cleared=data_delete_manifest(current,&incoming,&original);int copied=data_restore_manifest(backup,current,&original);int restored=cleared&&copied;
        if(restored){swprintf(message,512,L"导入失败，已恢复原数据。备份保留在:\n%ls",backup);}
        else {keep_stage=1;swprintf(message,512,L"导入和恢复未完整完成，请保留以下目录并手动恢复。\n原数据备份:\n%ls\n导入暂存:\n%ls",backup,stage);}
        MessageBoxW(hwnd,message,L"导入失败",MB_OK|MB_ICONERROR);goto finish;
    }
    repair_settings_recovery_chain();load_settings_preserving_runtime();refresh_today_count_if_day_changed(hwnd,1);load_heatmap_data();
    if(g_hHeatmapWnd)InvalidateRect(g_hHeatmapWnd,NULL,TRUE);refresh_timer_icon_by_state(hwnd);
    swprintf(message,512,L"导入完成：成功 %zu/%zu。\n原数据备份:\n%ls",incoming.count,incoming.count,backup);MessageBoxW(hwnd,message,L"导入完成",MB_OK|MB_ICONINFORMATION);ok=1;goto finish;
cancelled:
    MessageBoxW(hwnd,L"导入准备未完成或数据状态已变化；当前数据未改动，导入已取消。",L"导入失败",MB_OK|MB_ICONERROR);
finish:
    if(stage_created&&!keep_stage){data_delete_manifest(stage,&incoming,NULL);RemoveDirectoryW(stage);}
    if(backup_created&&!ok&&original.count==0)RemoveDirectoryW(backup);
    free(incoming.names);free(original.names);return ok;
}
#endif
