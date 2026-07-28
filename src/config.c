#include "config.h"

#include <wchar.h>

#pragma warning(disable: 4996)  // wcsncpy is used safely with explicit null termination

static void copy_text(wchar_t *destination, size_t destination_count, const wchar_t *source)
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

void config_set_defaults(AppConfig *config)
{
    if (config == NULL) {
        return;
    }

    ZeroMemory(config, sizeof(*config));
    copy_text(config->pc_ip, CONFIG_VALUE_LEN, L"192.168.1.10");
    copy_text(config->pos_ip, CONFIG_VALUE_LEN, L"192.168.1.11");
    copy_text(config->subnet_mask, CONFIG_VALUE_LEN, L"255.255.255.0");
    copy_text(config->gateway, CONFIG_VALUE_LEN, L"192.168.1.1");
    copy_text(config->dns_primary, CONFIG_VALUE_LEN, L"192.168.1.1");
    copy_text(config->dns_secondary, CONFIG_VALUE_LEN, L"1.1.1.1");
    config->auto_test = TRUE;
}

BOOL config_load(AppConfig *config, const wchar_t *path)
{
    wchar_t buffer[CONFIG_VALUE_LEN];

    if (config == NULL || path == NULL) {
        return FALSE;
    }

    config_set_defaults(config);

    GetPrivateProfileStringW(L"Network", L"Adapter", L"", config->adapter,
                             ADAPTER_NAME_LEN, path);
    GetPrivateProfileStringW(L"Network", L"PcIp", config->pc_ip, buffer,
                             CONFIG_VALUE_LEN, path);
    copy_text(config->pc_ip, CONFIG_VALUE_LEN, buffer);

    GetPrivateProfileStringW(L"Network", L"PosIp", config->pos_ip, buffer,
                             CONFIG_VALUE_LEN, path);
    copy_text(config->pos_ip, CONFIG_VALUE_LEN, buffer);

    GetPrivateProfileStringW(L"Network", L"SubnetMask", config->subnet_mask, buffer,
                             CONFIG_VALUE_LEN, path);
    copy_text(config->subnet_mask, CONFIG_VALUE_LEN, buffer);

    GetPrivateProfileStringW(L"Network", L"Gateway", config->gateway, buffer,
                             CONFIG_VALUE_LEN, path);
    copy_text(config->gateway, CONFIG_VALUE_LEN, buffer);

    GetPrivateProfileStringW(L"Network", L"DnsPrimary", config->dns_primary, buffer,
                             CONFIG_VALUE_LEN, path);
    copy_text(config->dns_primary, CONFIG_VALUE_LEN, buffer);

    GetPrivateProfileStringW(L"Network", L"DnsSecondary", config->dns_secondary, buffer,
                             CONFIG_VALUE_LEN, path);
    copy_text(config->dns_secondary, CONFIG_VALUE_LEN, buffer);

    config->auto_test = GetPrivateProfileIntW(L"Application", L"AutoTest", 1, path) != 0;
    return TRUE;
}

BOOL config_save(const AppConfig *config, const wchar_t *path)
{
    BOOL ok = TRUE;

    if (config == NULL || path == NULL) {
        return FALSE;
    }

    ok = WritePrivateProfileStringW(L"Network", L"Adapter", config->adapter, path) && ok;
    ok = WritePrivateProfileStringW(L"Network", L"PcIp", config->pc_ip, path) && ok;
    ok = WritePrivateProfileStringW(L"Network", L"PosIp", config->pos_ip, path) && ok;
    ok = WritePrivateProfileStringW(L"Network", L"SubnetMask", config->subnet_mask, path) && ok;
    ok = WritePrivateProfileStringW(L"Network", L"Gateway", config->gateway, path) && ok;
    ok = WritePrivateProfileStringW(L"Network", L"DnsPrimary", config->dns_primary, path) && ok;
    ok = WritePrivateProfileStringW(L"Network", L"DnsSecondary", config->dns_secondary, path) && ok;
    ok = WritePrivateProfileStringW(L"Application", L"AutoTest",
                                    config->auto_test ? L"1" : L"0", path) && ok;
    return ok;
}
