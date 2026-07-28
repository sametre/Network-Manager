#ifndef PCPOS_CONFIG_H
#define PCPOS_CONFIG_H

#include <windows.h>

#define CONFIG_VALUE_LEN 128
#define ADAPTER_NAME_LEN 256

typedef struct AppConfig {
    wchar_t adapter[ADAPTER_NAME_LEN];
    wchar_t pc_ip[CONFIG_VALUE_LEN];
    wchar_t pos_ip[CONFIG_VALUE_LEN];
    wchar_t subnet_mask[CONFIG_VALUE_LEN];
    wchar_t gateway[CONFIG_VALUE_LEN];
    wchar_t dns_primary[CONFIG_VALUE_LEN];
    wchar_t dns_secondary[CONFIG_VALUE_LEN];
    BOOL auto_test;
} AppConfig;

void config_set_defaults(AppConfig *config);
BOOL config_load(AppConfig *config, const wchar_t *path);
BOOL config_save(const AppConfig *config, const wchar_t *path);

#endif
