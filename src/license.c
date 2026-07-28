#include "license.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define REGISTRY_PATH L"Software\\ReSoft\\PcPosNetworkManager"
#define REG_LICENSE_KEY L"LicenseKey"
#define REG_OWNER_NAME L"OwnerName"
#define REG_OWNER_EMAIL L"OwnerEmail"
#define REG_ISSUE_DATE L"IssueDate"
#define REG_EXPIRY_DATE L"ExpiryDate"
#define REG_FIRST_LAUNCH_TIME L"FirstLaunchTime"
#define REG_REPORT_SENT L"ReportSent"

// Lisans anahtarı oluştur (deterministik, cihaz ID'sine dayanabilir)
static void generate_license_key(wchar_t *license_key, size_t size)
{
	time_t now = time(NULL);
	swprintf_s(license_key, size, L"LIC-%lld-PCPOS", (long long)now);
}

// Bugünün tarihini YYYY-MM-DD formatında al
static void get_today_date(wchar_t *date_str, size_t size)
{
	time_t now = time(NULL);
	struct tm *timeinfo = localtime(&now);
	wcsftime(date_str, size, L"%Y-%m-%d", timeinfo);
}

// 30 gün sonrası tarihini hesapla
static void get_date_plus_days(wchar_t *date_str, size_t size, int days)
{
	time_t now = time(NULL);
	now += (days * 24 * 60 * 60);  // Saniye cinsinden ekle
	struct tm *timeinfo = localtime(&now);
	wcsftime(date_str, size, L"%Y-%m-%d", timeinfo);
}

// Tarih farkını gün cinsinde hesapla (YYYY-MM-DD formatında)
static int calculate_days_difference(const wchar_t *expiry_date_str)
{
	time_t now = time(NULL);

	// expiry_date_str'i parse et
	int year, month, day;
	swscanf_s(expiry_date_str, L"%d-%d-%d", &year, &month, &day);

	struct tm expiry_tm = {0};
	expiry_tm.tm_year = year - 1900;
	expiry_tm.tm_mon = month - 1;
	expiry_tm.tm_mday = day;
	expiry_tm.tm_hour = 0;
	expiry_tm.tm_min = 0;
	expiry_tm.tm_sec = 0;

	time_t expiry_time = mktime(&expiry_tm);

	double diff = difftime(expiry_time, now);
	int days_left = (int)(diff / (24.0 * 60.0 * 60.0));

	return (days_left > 0) ? days_left : 0;
}

// Registry'den lisans bilgilerini oku
int license_load_from_registry(LicenseInfo *license)
{
	if (license == NULL) {
		return -1;
	}

	HKEY hKey;
	DWORD dwType, dwSize;
	DWORD dwFirstLaunchTime = 0;

	memset(license, 0, sizeof(*license));

	if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		// License Key
		dwSize = sizeof(license->license_key);
		RegQueryValueExW(hKey, REG_LICENSE_KEY, NULL, &dwType,
						(LPBYTE)license->license_key, &dwSize);

		// Owner Name
		dwSize = sizeof(license->owner_name);
		RegQueryValueExW(hKey, REG_OWNER_NAME, NULL, &dwType,
						(LPBYTE)license->owner_name, &dwSize);

		// Owner Email
		dwSize = sizeof(license->owner_email);
		RegQueryValueExW(hKey, REG_OWNER_EMAIL, NULL, &dwType,
						(LPBYTE)license->owner_email, &dwSize);

		// Issue Date
		dwSize = sizeof(license->issue_date);
		RegQueryValueExW(hKey, REG_ISSUE_DATE, NULL, &dwType,
						(LPBYTE)license->issue_date, &dwSize);

		// Expiry Date
		dwSize = sizeof(license->expiry_date);
		RegQueryValueExW(hKey, REG_EXPIRY_DATE, NULL, &dwType,
						(LPBYTE)license->expiry_date, &dwSize);

		// First Launch Time
		dwSize = sizeof(dwFirstLaunchTime);
		RegQueryValueExW(hKey, REG_FIRST_LAUNCH_TIME, NULL, &dwType,
						(LPBYTE)&dwFirstLaunchTime, &dwSize);
		license->first_launch_timestamp = (int)dwFirstLaunchTime;

		RegCloseKey(hKey);
		return 0;
	}

	return -1;
}

// Registry'e lisans bilgilerini kaydet
int license_save_to_registry(const LicenseInfo *license)
{
	if (license == NULL) {
		return -1;
	}

	HKEY hKey;
	DWORD dwDisposition;

	if (RegCreateKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL,
					   REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {

		// License Key
		RegSetValueExW(hKey, REG_LICENSE_KEY, 0, REG_SZ,
					  (LPBYTE)license->license_key,
					  (wcslen(license->license_key) + 1) * sizeof(wchar_t));

		// Owner Name
		RegSetValueExW(hKey, REG_OWNER_NAME, 0, REG_SZ,
					  (LPBYTE)license->owner_name,
					  (wcslen(license->owner_name) + 1) * sizeof(wchar_t));

		// Owner Email
		RegSetValueExW(hKey, REG_OWNER_EMAIL, 0, REG_SZ,
					  (LPBYTE)license->owner_email,
					  (wcslen(license->owner_email) + 1) * sizeof(wchar_t));

		// Issue Date
		RegSetValueExW(hKey, REG_ISSUE_DATE, 0, REG_SZ,
					  (LPBYTE)license->issue_date,
					  (wcslen(license->issue_date) + 1) * sizeof(wchar_t));

		// Expiry Date
		RegSetValueExW(hKey, REG_EXPIRY_DATE, 0, REG_SZ,
					  (LPBYTE)license->expiry_date,
					  (wcslen(license->expiry_date) + 1) * sizeof(wchar_t));

		// First Launch Time
		DWORD dwFirstLaunchTime = (DWORD)license->first_launch_timestamp;
		RegSetValueExW(hKey, REG_FIRST_LAUNCH_TIME, 0, REG_DWORD,
					  (LPBYTE)&dwFirstLaunchTime, sizeof(DWORD));

		RegCloseKey(hKey);
		return 0;
	}

	return -1;
}

// Lisans başlatma (ilk kez çalışıyorsa lisans oluştur)
int license_init(LicenseInfo *license)
{
	if (license == NULL) {
		return -1;
	}

	memset(license, 0, sizeof(*license));

	// Registry'den yüklemesi dene
	if (license_load_from_registry(license) == 0 && 
		wcslen(license->license_key) > 0) {
		// Lisans zaten vardı
		license_check_status(license);
		return 0;
	}

	// Yeni lisans oluştur
	generate_license_key(license->license_key, MAX_LICENSE_KEY_LEN);
	wcscpy_s(license->owner_name, MAX_LICENSE_OWNER_LEN, L"Demo User");
	wcscpy_s(license->owner_email, MAX_LICENSE_EMAIL_LEN, L"user@example.com");

	get_today_date(license->issue_date, 64);
	get_date_plus_days(license->expiry_date, 64, 30);  // 30 gün trial

	license->first_launch_timestamp = (int)time(NULL);
	license->status = LICENSE_STATUS_ACTIVE;
	license->days_left = 30;

	// Registry'e kaydet
	license_save_to_registry(license);

	return 0;
}

// Lisans durumunu kontrol et
int license_check_status(LicenseInfo *license)
{
	if (license == NULL) {
		return -1;
	}

	license->days_left = calculate_days_difference(license->expiry_date);

	if (license->days_left <= 0) {
		license->status = LICENSE_STATUS_EXPIRED;
	} else {
		license->status = LICENSE_STATUS_ACTIVE;
	}

	return 0;
}

// Kalan gün sayısını al
int license_get_days_remaining(LicenseInfo *license)
{
	if (license == NULL) {
		return 0;
	}

	return license->days_left;
}

// Lisans bilgilerini metin formatında veri
int license_to_text(const LicenseInfo *license, wchar_t *text_buffer, size_t buffer_size)
{
	if (license == NULL || text_buffer == NULL) {
		return -1;
	}

	const wchar_t *status_str = (license->status == LICENSE_STATUS_ACTIVE) ? L"Aktif" : L"Süresi Dolmuş";

	swprintf_s(text_buffer, buffer_size,
			  L"=== Lisans Bilgileri ===\n"
			  L"Lisans Anahtarı: %s\n"
			  L"Kullanıcı: %s\n"
			  L"E-mail: %s\n"
			  L"Durumu: %s\n"
			  L"Kalan Gün: %d\n"
			  L"Başlama Tarihi: %s\n"
			  L"Bitiş Tarihi: %s\n",
			  license->license_key,
			  license->owner_name,
			  license->owner_email,
			  status_str,
			  license->days_left,
			  license->issue_date,
			  license->expiry_date);

	return 0;
}

// 30 gün geçti mi kontrol et (raporlama için)
int license_should_report_device(LicenseInfo *license)
{
	if (license == NULL) {
		return 0;
	}

	// Basit kontrol: ilk kurulumdan 30 gün geçtiyse true döndür
	time_t now = time(NULL);
	int elapsed_seconds = (int)(now - license->first_launch_timestamp);
	int elapsed_days = elapsed_seconds / (24 * 60 * 60);

	// Eğer 30 gün geçtiyse UI'da bildirim göster ve rapor gönder
	return (elapsed_days >= 30) ? 1 : 0;
}
