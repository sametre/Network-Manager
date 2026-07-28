#ifndef PCPOS_DEVICE_H
#define PCPOS_DEVICE_H

#include <winsock2.h>
#include <wchar.h>

#define MAX_IP_ADDR_LEN 64
#define MAX_MAC_ADDR_LEN 32
#define MAX_COMPUTER_NAME_LEN 256
#define MAX_USERNAME_LEN 256
#define MAX_WINDOWS_VERSION_LEN 256

// Cihaz bilgisi yapısı
typedef struct {
	wchar_t ip_address[MAX_IP_ADDR_LEN];
	wchar_t mac_address[MAX_MAC_ADDR_LEN];
	wchar_t computer_name[MAX_COMPUTER_NAME_LEN];
	wchar_t username[MAX_USERNAME_LEN];
	wchar_t windows_version[MAX_WINDOWS_VERSION_LEN];
	wchar_t install_date[64];  // YYYY-MM-DD HH:MM:SS
	wchar_t device_id[64];     // Unique identifier
} DeviceInfo;

// Cihaz bilgilerini topla
int device_collect_info(DeviceInfo *info);

// Cihaz bilgilerini JSON formatına çevir
int device_to_json(const DeviceInfo *info, wchar_t *json_buffer, size_t buffer_size);

// Cihaz bilgilerini metin formatına çevir (rapor)
int device_to_text(const DeviceInfo *info, wchar_t *text_buffer, size_t buffer_size);

#endif
