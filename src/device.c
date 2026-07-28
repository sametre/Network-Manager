#include "device.h"

#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

// Sistem sürümü bilgilerini al
static void get_windows_version(wchar_t *version, size_t size)
{
	HKEY hKey;
	DWORD dwType, dwSize;
	wchar_t buildStr[32] = L"";

	// Windows 10/11 build numarası
	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
					  L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
					  0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		dwSize = sizeof(buildStr);
		RegQueryValueExW(hKey, L"CurrentBuild", NULL, &dwType, 
						(LPBYTE)buildStr, &dwSize);
		RegCloseKey(hKey);
	}

	// Basit sürüm bilgisi (gerçek ortamda VERSIONINFO API kullanılabilir)
	swprintf_s(version, size, L"Windows (Build: %s)", buildStr[0] ? buildStr : L"Unknown");
}

// IP adresini al
static void get_ipv4_address(wchar_t *ip_addr, size_t size)
{
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		wcscpy_s(ip_addr, size, L"0.0.0.0");
		return;
	}

	// GetAdaptersInfo API kullan
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			wcscpy_s(ip_addr, size, L"0.0.0.0");
			return;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			// İlk aktif IPv4 adresini al
			if (pAdapter->IpAddressList.IpAddress.String[0]) {
				mbstowcs_s(NULL, ip_addr, size, 
						  pAdapter->IpAddressList.IpAddress.String, _TRUNCATE);
				if (wcscmp(ip_addr, L"0.0.0.0") != 0) {
					break;
				}
			}
			pAdapter = pAdapter->Next;
		}
	}

	if (pAdapterInfo)
		free(pAdapterInfo);
}

// MAC adresini al
static void get_mac_address(wchar_t *mac_addr, size_t size)
{
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		wcscpy_s(mac_addr, size, L"00:00:00:00:00:00");
		return;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			wcscpy_s(mac_addr, size, L"00:00:00:00:00:00");
			return;
		}
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			if (pAdapter->AddressLength > 0) {
				swprintf_s(mac_addr, size, 
						  L"%02X:%02X:%02X:%02X:%02X:%02X",
						  pAdapter->Address[0],
						  pAdapter->Address[1],
						  pAdapter->Address[2],
						  pAdapter->Address[3],
						  pAdapter->Address[4],
						  pAdapter->Address[5]);
				break;
			}
			pAdapter = pAdapter->Next;
		}
	}

	if (pAdapterInfo)
		free(pAdapterInfo);
}

// Bilgisayar adını al
static void get_computer_name(wchar_t *name, size_t size)
{
	DWORD dwSize = (DWORD)size;
	if (!GetComputerNameW(name, &dwSize)) {
		wcscpy_s(name, size, L"UNKNOWN");
	}
}

// Kullanıcı adını al
static void get_username(wchar_t *username, size_t size)
{
	DWORD dwSize = (DWORD)size;
	if (!GetUserNameW(username, &dwSize)) {
		wcscpy_s(username, size, L"UNKNOWN");
	}
}

// Geçerli zamanı format et
static void get_current_time_str(wchar_t *time_str, size_t size)
{
	time_t now = time(NULL);
	struct tm *timeinfo = localtime(&now);
	wcsftime(time_str, size, L"%Y-%m-%d %H:%M:%S", timeinfo);
}

// Benzersiz cihaz ID'si oluştur (MAC + Computer Name hash)
static void generate_device_id(const wchar_t *mac, const wchar_t *computer_name, 
							   wchar_t *device_id, size_t size)
{
	swprintf_s(device_id, size, L"%s_%s", computer_name, mac);
}

// Ana kollaşı
int device_collect_info(DeviceInfo *info)
{
	if (info == NULL) {
		return -1;
	}

	// Tüm bilgileri topla
	get_ipv4_address(info->ip_address, MAX_IP_ADDR_LEN);
	get_mac_address(info->mac_address, MAX_MAC_ADDR_LEN);
	get_computer_name(info->computer_name, MAX_COMPUTER_NAME_LEN);
	get_username(info->username, MAX_USERNAME_LEN);
	get_windows_version(info->windows_version, MAX_WINDOWS_VERSION_LEN);
	get_current_time_str(info->install_date, 64);
	generate_device_id(info->mac_address, info->computer_name, 
					  info->device_id, 64);

	return 0;
}

// JSON formatı
int device_to_json(const DeviceInfo *info, wchar_t *json_buffer, size_t buffer_size)
{
	if (info == NULL || json_buffer == NULL) {
		return -1;
	}

	swprintf_s(json_buffer, buffer_size,
			  L"{"
			  L"\"ip_address\":\"%s\","
			  L"\"mac_address\":\"%s\","
			  L"\"computer_name\":\"%s\","
			  L"\"username\":\"%s\","
			  L"\"windows_version\":\"%s\","
			  L"\"device_id\":\"%s\","
			  L"\"timestamp\":\"%s\""
			  L"}",
			  info->ip_address,
			  info->mac_address,
			  info->computer_name,
			  info->username,
			  info->windows_version,
			  info->device_id,
			  info->install_date);

	return 0;
}

// Metin formatı (rapor)
int device_to_text(const DeviceInfo *info, wchar_t *text_buffer, size_t buffer_size)
{
	if (info == NULL || text_buffer == NULL) {
		return -1;
	}

	swprintf_s(text_buffer, buffer_size,
			  L"=== Cihaz Bilgileri ===\n"
			  L"IP Adresi: %s\n"
			  L"MAC Adresi: %s\n"
			  L"Bilgisayar Adı: %s\n"
			  L"Kullanıcı: %s\n"
			  L"Windows Sürümü: %s\n"
			  L"Cihaz ID: %s\n"
			  L"Kurulum Tarihi: %s\n",
			  info->ip_address,
			  info->mac_address,
			  info->computer_name,
			  info->username,
			  info->windows_version,
			  info->device_id,
			  info->install_date);

	return 0;
}
