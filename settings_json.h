#ifndef POMODORO_SETTINGS_JSON_H
#define POMODORO_SETTINGS_JSON_H
#include <errno.h>
#include <limits.h>

/* The settings format is a flat object of integers and strings. Scan tokens,
   not substrings: user text may itself contain any of the setting names. */
static const char *settings_json_space(const char *p) {
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ++p;
    return p;
}
static int settings_json_string(const char **cursor, wchar_t *out, size_t capacity) {
    const unsigned char *p = (const unsigned char *)*cursor;
    size_t used = 0;
    if (*p++ != '"') return 0;
    while (*p && *p != '"') {
        wchar_t chars[2];
        int count = 1, i;
        if (*p == '\\') {
            ++p;
            switch (*p++) {
                case '"': chars[0] = '"'; break;
                case '\\': chars[0] = '\\'; break;
                case '/': chars[0] = '/'; break;
                case 'b': chars[0] = '\b'; break;
                case 'f': chars[0] = '\f'; break;
                case 'n': chars[0] = '\n'; break;
                case 'r': chars[0] = '\r'; break;
                case 't': chars[0] = '\t'; break;
                case 'u': {
                    unsigned value = 0;
                    for (i = 0; i < 4; ++i) {
                        unsigned c = *p++, digit;
                        if (c >= '0' && c <= '9') digit = c - '0';
                        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
                        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
                        else return 0;
                        value = value * 16 + digit;
                    }
                    if (!value) return 0;
                    chars[0] = (wchar_t)value;
                    break;
                }
                default: return 0;
            }
        } else {
            int bytes = *p < 0x80 ? 1 : (*p >= 0xC2 && *p < 0xE0 ? 2 : (*p < 0xF0 && *p >= 0xE0 ? 3 : (*p >= 0xF0 && *p <= 0xF4 ? 4 : 0)));
            if (*p < 32 || !bytes) return 0;
            for (i = 1; i < bytes; ++i) if (!p[i]) return 0;
            count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (const char *)p, bytes, chars, 2);
            if (!count) return 0;
            p += bytes;
        }
        if (out && used + count >= capacity) return 0;
        for (i = 0; i < count; ++i) { if (out) out[used] = chars[i]; ++used; }
    }
    if (*p != '"') return 0;
    if (out) out[used] = 0;
    *cursor = (const char *)p + 1;
    return 1;
}
static int settings_json_number(const char **cursor, int *value) {
    const char *p = *cursor, *digits;
    char *end;
    long parsed;
    if (*p == '-') ++p;
    digits = p;
    while (*p >= '0' && *p <= '9') ++p;
    if (digits == p || (*digits == '0' && p - digits > 1)) return 0;
    errno = 0;
    parsed = strtol(*cursor, &end, 10);
    if (errno == ERANGE || end != p || parsed < INT_MIN || parsed > INT_MAX) return 0;
    if (value) *value = (int)parsed;
    *cursor = p;
    return 1;
}
static int settings_json_scan(const char *text, const char *key, const char **found) {
    const char *p;
    wchar_t name[128];
    if (strlen(text) >= 3 && (unsigned char)text[0] == 0xEF &&
        (unsigned char)text[1] == 0xBB && (unsigned char)text[2] == 0xBF) text += 3;
    p = settings_json_space(text);
    if (found) *found = NULL;
    if (*p++ != '{') return 0;
    p = settings_json_space(p);
    if (*p == '}') return !*settings_json_space(p + 1);
    for (;;) {
        const char *value;
        size_t i;
        int match = key != NULL;
        if (!settings_json_string(&p, name, 128)) return 0;
        p = settings_json_space(p);
        if (*p++ != ':') return 0;
        p = settings_json_space(p); value = p;
        if (*p == '"') { if (!settings_json_string(&p, NULL, 0)) return 0; }
        else if (!settings_json_number(&p, NULL)) return 0;
        if (key) {
            size_t key_length = strlen(key);
            if (key_length < 2 || wcslen(name) != key_length - 2) match = 0;
            else for (i = 0; name[i]; ++i) if (name[i] != (unsigned char)key[i + 1]) match = 0;
        }
        if (match && found) { if (*found) return 0; *found = value; }
        p = settings_json_space(p);
        if (*p == '}') return !*settings_json_space(p + 1);
        if (*p++ != ',') return 0;
        p = settings_json_space(p);
    }
}
static const char *settings_json_find(const char *text, const char *key) {
    const char *value;
    return settings_json_scan(text, key, &value) ? value : NULL;
}
static int settings_json_valid(const char *text) {
    return settings_json_find(text, "\"pomodoro_duration\"") != NULL &&
        settings_json_find(text, "\"pomodoro_count\"") != NULL;
}
static int settings_json_write_string(FILE *file, const wchar_t *value) {
    if (fputc('"', file) == EOF) return 0;
    /* Escaping each UTF-16 unit preserves Chinese, line breaks and surrogate pairs. */
    while (*value) if (fprintf(file, "\\u%04x", (unsigned)*value++) < 0) return 0;
    return fputc('"', file) != EOF;
}
#endif
