#include "utils.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#pragma warning(disable: 4996)  // wcsncpy is used safely with explicit null termination

void util_copy_text(wchar_t *destination, size_t destination_count, const wchar_t *source)
{
    if (destination == NULL || destination_count == 0) {
        return;
    }

    if (source == NULL) {
        destination[0] = L'\0';
        return;
    }

    wcsncpy(destination, source, destination_count - 1);
    destination[destination_count - 1] = L'\0';
}

void util_append_text(wchar_t *destination, size_t destination_count, const wchar_t *source)
{
    size_t used;

    if (destination == NULL || destination_count == 0 || source == NULL) {
        return;
    }

    used = wcslen(destination);
    if (used >= destination_count - 1) {
        return;
    }

    wcsncpy(destination + used, source, destination_count - used - 1);
    destination[destination_count - 1] = L'\0';
}

BOOL util_get_app_directory(wchar_t *buffer, size_t buffer_count)
{
    DWORD length;
    wchar_t *slash;

    if (buffer == NULL || buffer_count == 0) {
        return FALSE;
    }

    length = GetModuleFileNameW(NULL, buffer, (DWORD)buffer_count);
    if (length == 0 || length >= buffer_count) {
        return FALSE;
    }

    slash = wcsrchr(buffer, L'\\');
    if (slash == NULL) {
        return FALSE;
    }

    *slash = L'\0';
    return TRUE;
}

BOOL util_combine_path(wchar_t *buffer, size_t buffer_count,
                       const wchar_t *directory, const wchar_t *name)
{
    int written;

    if (buffer == NULL || buffer_count == 0 || directory == NULL || name == NULL) {
        return FALSE;
    }

    written = swprintf(buffer, buffer_count, L"%ls\\%ls", directory, name);
    return written > 0 && (size_t)written < buffer_count;
}

BOOL util_ensure_directory(const wchar_t *path)
{
    if (CreateDirectoryW(path, NULL)) {
        return TRUE;
    }

    return GetLastError() == ERROR_ALREADY_EXISTS;
}

BOOL util_is_valid_ipv4(const wchar_t *text)
{
    IN_ADDR address;

    if (text == NULL || text[0] == L'\0') {
        return FALSE;
    }

    return InetPtonW(AF_INET, text, &address) == 1;
}

BOOL util_same_subnet(const wchar_t *ip_a, const wchar_t *ip_b, const wchar_t *mask)
{
    IN_ADDR a;
    IN_ADDR b;
    IN_ADDR m;

    if (InetPtonW(AF_INET, ip_a, &a) != 1 ||
        InetPtonW(AF_INET, ip_b, &b) != 1 ||
        InetPtonW(AF_INET, mask, &m) != 1) {
        return FALSE;
    }

    return (a.S_un.S_addr & m.S_un.S_addr) == (b.S_un.S_addr & m.S_un.S_addr);
}

BOOL util_is_safe_adapter_name(const wchar_t *name)
{
    const wchar_t *cursor;

    if (name == NULL || name[0] == L'\0' || wcslen(name) >= 240) {
        return FALSE;
    }

    for (cursor = name; *cursor != L'\0'; ++cursor) {
        if (*cursor == L'"' || *cursor == L'\r' || *cursor == L'\n') {
            return FALSE;
        }
    }

    return TRUE;
}

void util_format_system_error(DWORD error_code, wchar_t *buffer, size_t buffer_count)
{
    DWORD length;

    if (buffer == NULL || buffer_count == 0) {
        return;
    }

    length = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buffer,
        (DWORD)buffer_count,
        NULL
    );

    if (length == 0) {
        swprintf(buffer, buffer_count, L"Windows hata kodu: %lu", (unsigned long)error_code);
    }
}

void util_append_log_file(const wchar_t *path, const wchar_t *message)
{
    HANDLE file;
    LARGE_INTEGER file_size;
    SYSTEMTIME time_value;
    wchar_t wide_line[4096];
    char *utf8_line;
    int utf8_length;
    DWORD bytes_written;
    const BYTE utf8_bom[] = {0xEF, 0xBB, 0xBF};

    if (path == NULL || message == NULL) {
        return;
    }

    GetLocalTime(&time_value);
    swprintf(wide_line, sizeof(wide_line) / sizeof(wide_line[0]),
             L"[%04u-%02u-%02u %02u:%02u:%02u] %ls\r\n",
             time_value.wYear, time_value.wMonth, time_value.wDay,
             time_value.wHour, time_value.wMinute, time_value.wSecond,
             message);

    utf8_length = WideCharToMultiByte(CP_UTF8, 0, wide_line, -1, NULL, 0, NULL, NULL);
    if (utf8_length <= 1) {
        return;
    }

    utf8_line = (char *)malloc((size_t)utf8_length);
    if (utf8_line == NULL) {
        return;
    }

    if (WideCharToMultiByte(CP_UTF8, 0, wide_line, -1,
                            utf8_line, utf8_length, NULL, NULL) <= 0) {
        free(utf8_line);
        return;
    }

    file = CreateFileW(path, FILE_APPEND_DATA,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        free(utf8_line);
        return;
    }

    if (GetFileSizeEx(file, &file_size) && file_size.QuadPart == 0) {
        WriteFile(file, utf8_bom, sizeof(utf8_bom), &bytes_written, NULL);
    }

    WriteFile(file, utf8_line, (DWORD)(utf8_length - 1), &bytes_written, NULL);
    CloseHandle(file);
    free(utf8_line);
}
