#include "network_report.h"

#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#pragma comment(lib, "winhttp.lib")

#define REGISTRY_PATH L"Software\\ReSoft\\PcPosNetworkManager"
#define REG_REPORT_SENT L"ReportSent"

// Raporu HTTP POST ile gönder (WinHTTP kullanarak)
int report_send_device_info(const DeviceInfo *device_info)
{
	if (device_info == NULL) {
		return -1;
	}

	HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
	BOOL bResults = FALSE;
	int result = -1;

	// TXT formatında veri hazırla
	wchar_t report_data[2048] = {0};
	swprintf_s(report_data, sizeof(report_data) / sizeof(wchar_t),
			  L"IP_ADDRESS=%s\r\n"
			  L"MAC_ADDRESS=%s\r\n"
			  L"COMPUTER_NAME=%s\r\n"
			  L"USERNAME=%s\r\n"
			  L"WINDOWS_VERSION=%s\r\n"
			  L"DEVICE_ID=%s\r\n"
			  L"REPORT_DATE=%s\r\n",
			  device_info->ip_address,
			  device_info->mac_address,
			  device_info->computer_name,
			  device_info->username,
			  device_info->windows_version,
			  device_info->device_id,
			  device_info->install_date);

	// WinHTTP session oluştur
	hSession = WinHttpOpen(L"PcPosNetworkManager/1.0",
						  WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
						  WINHTTP_NO_PROXY_NAME,
						  WINHTTP_NO_PROXY_BYPASS, 0);

	if (!hSession) {
		return -1;
	}

	// HINTERNET hConnect oluştur
	// URL: http://temasre.shop/ip-address
	hConnect = WinHttpConnect(hSession, L"temasre.shop", INTERNET_DEFAULT_HTTP_PORT, 0);

	if (!hConnect) {
		WinHttpCloseHandle(hSession);
		return -1;
	}

	// HTTP POST isteği oluştur
	hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/ip-address",
								 NULL, WINHTTP_NO_REFERER,
								 WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

	if (!hRequest) {
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return -1;
	}

	// Content-Type header ekle
	WinHttpAddRequestHeaders(hRequest,
							L"Content-Type: text/plain\r\n",
							(ULONG)-1L,
							WINHTTP_ADDREQ_FLAG_ADD);

	// POST verilerini gönder
	DWORD dwDataSize = (wcslen(report_data) + 1) * sizeof(wchar_t);

	bResults = WinHttpSendRequest(hRequest,
								 WINHTTP_NO_ADDITIONAL_HEADERS, 0,
								 (LPVOID)report_data, dwDataSize,
								 dwDataSize, 0);

	if (bResults) {
		// Yanıtı al
		bResults = WinHttpReceiveResponse(hRequest, NULL);
		if (bResults) {
			result = 0;  // Başarılı
		}
	}

	// Resources'ı serbest bırak
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return result;
}

// Raporu dosyaya kaydet (backup/log)
int report_save_to_file(const DeviceInfo *device_info, const wchar_t *file_path)
{
	if (device_info == NULL || file_path == NULL) {
		return -1;
	}

	FILE *file = NULL;
	errno_t err = _wfopen_s(&file, file_path, L"w, ccs=UTF-8");

	if (err != 0 || file == NULL) {
		return -1;
	}

	fwprintf(file, L"=== Cihaz Raporu ===\n");
	fwprintf(file, L"IP Adresi: %s\n", device_info->ip_address);
	fwprintf(file, L"MAC Adresi: %s\n", device_info->mac_address);
	fwprintf(file, L"Bilgisayar Adı: %s\n", device_info->computer_name);
	fwprintf(file, L"Kullanıcı: %s\n", device_info->username);
	fwprintf(file, L"Windows Sürümü: %s\n", device_info->windows_version);
	fwprintf(file, L"Cihaz ID: %s\n", device_info->device_id);
	fwprintf(file, L"Raporlama Tarihi: %s\n", device_info->install_date);

	fclose(file);
	return 0;
}

// Rapor gönderildi olarak işaretle
int report_mark_as_sent(void)
{
	HKEY hKey;
	DWORD dwDisposition;
	DWORD dwSent = 1;

	if (RegCreateKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL,
					   REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {

		RegSetValueExW(hKey, REG_REPORT_SENT, 0, REG_DWORD,
					  (LPBYTE)&dwSent, sizeof(DWORD));

		RegCloseKey(hKey);
		return 0;
	}

	return -1;
}

// Rapor önceden gönderildi mi kontrol et
int report_has_been_sent(void)
{
	HKEY hKey;
	DWORD dwType, dwSize, dwSent = 0;

	if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		dwSize = sizeof(dwSent);
		RegQueryValueExW(hKey, REG_REPORT_SENT, NULL, &dwType,
						(LPBYTE)&dwSent, &dwSize);

		RegCloseKey(hKey);
		return dwSent ? 1 : 0;
	}

	return 0;
}
