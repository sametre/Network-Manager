#ifndef PCPOS_UTILS_H
#define PCPOS_UTILS_H

#include <windows.h>
#include <stddef.h>

BOOL util_get_app_directory(wchar_t *buffer, size_t buffer_count);
BOOL util_combine_path(wchar_t *buffer, size_t buffer_count,
                       const wchar_t *directory, const wchar_t *name);
BOOL util_ensure_directory(const wchar_t *path);
BOOL util_is_valid_ipv4(const wchar_t *text);
BOOL util_same_subnet(const wchar_t *ip_a, const wchar_t *ip_b, const wchar_t *mask);
BOOL util_is_safe_adapter_name(const wchar_t *name);
void util_append_log_file(const wchar_t *path, const wchar_t *message);
void util_format_system_error(DWORD error_code, wchar_t *buffer, size_t buffer_count);
void util_copy_text(wchar_t *destination, size_t destination_count, const wchar_t *source);
void util_append_text(wchar_t *destination, size_t destination_count, const wchar_t *source);

#endif
