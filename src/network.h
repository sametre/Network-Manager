#ifndef PCPOS_NETWORK_H
#define PCPOS_NETWORK_H

#include <winsock2.h>
#include <windows.h>
#include <stddef.h>

#include "config.h"

#define MAX_ADAPTERS 64
#define ADAPTER_INFO_TEXT_LEN 512
#define OPERATION_DETAILS_LEN 16384

typedef struct AdapterInfo {
    wchar_t friendly_name[ADAPTER_NAME_LEN];
    wchar_t description[ADAPTER_NAME_LEN];
    wchar_t ipv4[CONFIG_VALUE_LEN];
    wchar_t mac[64];
    BOOL is_up;
    ULONG if_type;
} AdapterInfo;

typedef struct NetworkOperationResult {
    BOOL success;
    DWORD error_code;
    wchar_t summary[512];
    wchar_t details[OPERATION_DETAILS_LEN];
} NetworkOperationResult;

size_t network_enumerate_adapters(AdapterInfo *items, size_t capacity);
BOOL network_apply_static(const AppConfig *config, NetworkOperationResult *result);
BOOL network_enable_dhcp(const AppConfig *config, NetworkOperationResult *result);
BOOL network_ping_pos(const AppConfig *config, NetworkOperationResult *result);

#endif
