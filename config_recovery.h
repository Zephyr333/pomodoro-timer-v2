#ifndef POMODORO_CONFIG_RECOVERY_H
#define POMODORO_CONFIG_RECOVERY_H
static int config_warning;
static int config_read_valid(const wchar_t *path,char *out,size_t size) {
    FILE *file=_wfopen(path,L"rb");if(!file)return 0;
    size_t length=fread(out,1,size-1,file);int complete=fgetc(file)==EOF&&!ferror(file);
    if(fclose(file)!=0)complete=0;out[length]=0;
    return complete&&length&&settings_json_valid(out);
}
static int config_matches(const wchar_t *path,const char *bytes) {
    char read[8192];return config_read_valid(path,read,sizeof(read))&&!strcmp(bytes,read);
}
/* Success means primary replacement committed, separate from later readback. */
static int config_write_atomic(const wchar_t *path,const char *bytes) {
    wchar_t staged[MAX_PATH];if(!settings_json_valid(bytes)||!GetTempFileNameW(g_data_dir,L"pmc",0,staged))return 0;
    FILE *file=_wfopen(staged,L"wb");int ok=file!=NULL;
    if(file){size_t n=strlen(bytes);if(fwrite(bytes,1,n,file)!=n||fflush(file)!=0||_commit(_fileno(file))!=0)ok=0;if(fclose(file)!=0)ok=0;}
    if(ok)ok=config_matches(staged,bytes);
    if(ok)ok=MoveFileExW(staged,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
    if(!ok)DeleteFileW(staged);return ok;
}
static void config_warn(int committed) {
    if(config_warning)return;config_warning=1;
    if(g_main_hwnd)PostMessageW(g_main_hwnd,WM_SETTINGS_RECOVERY_FAILED,committed,0);
    else MessageBoxW(NULL,committed?L"设置已保存，备份更新失败。可用恢复文件已保留。":L"已读取可用配置，但文件恢复未完整完成。可用恢复文件已保留。",L"配置恢复提示",MB_OK|MB_ICONWARNING);
}
static int config_recover_read(char *bytes,size_t size) {
    int main=config_read_valid(g_settings_path,bytes,size);
    if(!main&&!config_read_valid(g_settings_tmp_path,bytes,size)&&!config_read_valid(g_settings_bak_path,bytes,size))return 0;
    int committed=main||config_write_atomic(g_settings_path,bytes);
    int main_verified=committed&&config_matches(g_settings_path,bytes);
    int backup=config_matches(g_settings_bak_path,bytes)||config_write_atomic(g_settings_bak_path,bytes);
    int complete=main_verified&&backup&&config_matches(g_settings_bak_path,bytes);
    if(complete){if(config_matches(g_settings_tmp_path,bytes))DeleteFileW(g_settings_tmp_path);config_warning=0;}
    else config_warn(0);
    return 1; /* The validated in-memory candidate remains usable on disk failure. */
}
#endif
