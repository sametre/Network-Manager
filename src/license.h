#ifndef PCPOS_LICENSE_H
#define PCPOS_LICENSE_H

#include <wchar.h>

#define MAX_LICENSE_KEY_LEN 256
#define MAX_LICENSE_OWNER_LEN 256
#define MAX_LICENSE_EMAIL_LEN 256

typedef enum {
	LICENSE_STATUS_ACTIVE,
	LICENSE_STATUS_EXPIRED,
	LICENSE_STATUS_INVALID
} LicenseStatus;

typedef struct {
	wchar_t license_key[MAX_LICENSE_KEY_LEN];
	wchar_t owner_name[MAX_LICENSE_OWNER_LEN];
	wchar_t owner_email[MAX_LICENSE_EMAIL_LEN];
	wchar_t issue_date[64];      // YYYY-MM-DD
	wchar_t expiry_date[64];     // YYYY-MM-DD
	int days_left;
	LicenseStatus status;
	int first_launch_timestamp;  // Unix timestamp'i registry'de tutmak için
} LicenseInfo;

// Registry'den lisans bilgilerini yükle veya boş ise ilk kez ayarla
int license_init(LicenseInfo *license);

// Lisans durumunu kontrol et (30 gün geçti mi?)
int license_check_status(LicenseInfo *license);

// Kalan gün sayısını hesapla
int license_get_days_remaining(LicenseInfo *license);

// Lisans bilgilerini metin formatında al
int license_to_text(const LicenseInfo *license, wchar_t *text_buffer, size_t buffer_size);

// Registry'e lisans bilgilerini kaydet
int license_save_to_registry(const LicenseInfo *license);

// Registry'den lisans bilgilerini oku
int license_load_from_registry(LicenseInfo *license);

// 30 gün timer'ın tetiklenip tetiklenmediğini kontrol et
int license_should_report_device(LicenseInfo *license);

#endif
