#ifndef POMODORO_STATISTICS_SNAPSHOT_H
#define POMODORO_STATISTICS_SNAPSHOT_H
typedef enum { STATS_READ_OK,STATS_READ_MISSING,STATS_READ_FAILED } StatsReadStatus;
typedef struct { DayCount days[4096];int count,files;char date[11];wchar_t directory[MAX_PATH];int year,month,day;time_t now; } StatsSnapshot;
static int stats_read_warning;
#ifdef POMODORO_TEST_IO
static int test_stats_read_failure;
#endif
static char stats_snapshot_date[11];
static wchar_t stats_snapshot_dir[MAX_PATH];
static StatsReadStatus stats_last_status;
static int stats_has_read_error(void) {return stats_last_status==STATS_READ_FAILED;}
static void stats_suppress_read_warning(void) {if(stats_read_warning)stats_read_warning=2;}
static void stats_read_failed(void) {
    stats_last_status=STATS_READ_FAILED;stats_needs_refresh=1;
    if(!stats_read_warning){stats_read_warning=1;if(g_main_hwnd)PostMessageW(g_main_hwnd,WM_STATS_READ_FAILED,0,0);}
}
static int stats_snapshot_add(StatsSnapshot *s,const char *date,int delta) {
    if(!delta)return 1;int index=find_day_count(s->days,s->count,date);
    if(index>=0){LONGLONG n=(LONGLONG)s->days[index].count+delta;if(n<INT_MIN||n>INT_MAX)return 0;s->days[index].count=(int)n;return 1;}
    if(s->count>=4096)return 0;DayCount *item=&s->days[s->count++];strncpy(item->date,date,10);item->date[10]=0;item->count=delta;return 1;
}
static int stats_snapshot_file(StatsSnapshot *s,const wchar_t *path,int adjustments) {
    DWORD attr=GetFileAttributesW(path);if(attr==INVALID_FILE_ATTRIBUTES)return GetLastError()==ERROR_FILE_NOT_FOUND;
    if(attr&FILE_ATTRIBUTE_DIRECTORY)return 0;FILE *file=_wfopen(path,L"r");if(!file)return 0;++s->files;
    char line[128];int ok=1;
    while(fgets(line,sizeof(line),file)){size_t length=strlen(line);if(length==sizeof(line)-1&&line[length-1]!='\n'){ok=0;break;}
        char date[11]={0};int delta=1;
        if(adjustments){if(sscanf(line,"%10[^,],%d",date,&delta)!=2)continue;}
        else if(!parse_pomodoro_log_entry(line,date,&delta))continue;
        if(!stats_snapshot_add(s,date,delta)){ok=0;break;}}
    if(ferror(file))ok=0;if(fclose(file)!=0)ok=0;return ok;
}
static int stats_snapshot_read(StatsSnapshot *s) {
#ifdef POMODORO_TEST_IO
    if(test_stats_read_failure)return 0;
#endif
    s->now=time(NULL);struct tm *date=localtime(&s->now);if(!date)return 0;
    s->year=date->tm_year+1900;s->month=date->tm_mon+1;s->day=date->tm_mday;
    sprintf(s->date,"%04d-%02d-%02d",s->year,s->month,s->day);wcscpy(s->directory,g_data_dir);
    if(!stats_snapshot_file(s,g_log_path,0)||!stats_snapshot_file(s,g_adjustments_path,1))return 0;
    wchar_t pattern[MAX_PATH],path[MAX_PATH];if(swprintf(pattern,MAX_PATH,L"%ls\\pomodoro_log_*.csv",s->directory)<0)return 0;
    WIN32_FIND_DATAW item;HANDLE find=FindFirstFileW(pattern,&item);
    if(find==INVALID_HANDLE_VALUE){if(GetLastError()!=ERROR_FILE_NOT_FOUND)return 0;}
    else {int ok=1;do{if(item.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)continue;
            if(swprintf(path,MAX_PATH,L"%ls\\%ls",s->directory,item.cFileName)<0||!stats_snapshot_file(s,path,0)){ok=0;break;}
        }while(FindNextFileW(find,&item));if(ok&&GetLastError()!=ERROR_NO_MORE_FILES)ok=0;FindClose(find);if(!ok)return 0;}
    return !_wcsicmp(s->directory,g_data_dir);
}
static int stats_try_today(int *value) {
    StatsSnapshot *s=(StatsSnapshot*)calloc(1,sizeof(*s));if(!s){stats_read_failed();return 0;}
    if(!stats_snapshot_read(s)){free(s);stats_read_failed();return 0;}
    LONGLONG total=0,year=0,month=0,week=0;int today=0;
    for(int i=0;i<s->count;++i){int y,m,d;if(sscanf(s->days[i].date,"%d-%d-%d",&y,&m,&d)!=3)continue;
        int n=max(0,s->days[i].count);s->days[i].count=n;total+=n;if(y==s->year)year+=n;if(y==s->year&&m==s->month)month+=n;
        if(!strcmp(s->days[i].date,s->date))today=n;
        struct tm entry={0};entry.tm_year=y-1900;entry.tm_mon=m-1;entry.tm_mday=d;entry.tm_hour=12;
        double days=difftime(s->now,mktime(&entry))/86400.0;if(days>=0&&days<7)week+=n;}
    memcpy(g_day_counts,s->days,s->count*sizeof(DayCount));g_day_count=s->count;
    g_heatmap_total=(int)min(total,999999);g_heatmap_yearTotal=(int)min(year,999999);g_heatmap_monthTotal=(int)min(month,999999);g_heatmap_weekTotal=(int)min(week,999999);
    if(!g_heatmap_display_year||!g_heatmap_display_month){g_heatmap_display_year=s->year;g_heatmap_display_month=s->month;}
    wcscpy(stats_snapshot_dir,s->directory);strcpy(stats_snapshot_date,s->date);
    stats_last_status=s->files?STATS_READ_OK:STATS_READ_MISSING;stats_read_warning=0;*value=clamp_int(today,0,9999);free(s);return 1;
}
static int stats_fallback_credit(int count) {
    int value;if(!stats_try_today(&value))return 0;
    time_t now=time(NULL);struct tm *date=localtime(&now);if(!date)return 0;char today[11];sprintf(today,"%04d-%02d-%02d",date->tm_year+1900,date->tm_mon+1,date->tm_mday);
    if(strcmp(today,stats_snapshot_date)||_wcsicmp(g_data_dir,stats_snapshot_dir))return 0;
    int raw=get_day_count_value(date->tm_year+1900,date->tm_mon+1,date->tm_mday);if((LONGLONG)raw+count>INT_MAX)return 0;
    char line[96];sprintf(line,"%s,%d\n",today,count);return stats_append_line(g_adjustments_path,line);
}
#endif
